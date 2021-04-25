#include <concore/n_serializer.hpp>
#include <concore/spawn.hpp>
#include <concore/data/concurrent_queue.hpp>

#include "../common/utils.hpp"

struct backup_engine {
    int idx_{0};

    void save() {
        CONCORE_PROFILING_FUNCTION();
        CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", idx_);
        printf("%d: backing up\n", idx_);
        sleep_in_between_ms(10, 20);
        printf("%d: finishing backing up\n", idx_);
    }
};

using conc_backup_engine_list = concore::concurrent_queue<backup_engine>;

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Construct multiple backup engine objects
    static constexpr int num_engines = 3;
    conc_backup_engine_list engines;
    for (int i = 0; i < num_engines; i++)
        engines.push(backup_engine{i});

    auto grp = concore::task_group::create();
    concore::n_serializer ser{num_engines};

    auto running_process = [&] {
        CONCORE_PROFILING_SCOPE_N("process");

        for (int i = 0; i < 100; i++) {
            // Wait for a bit
            sleep_in_between_ms(1, 5);

            // Trigger a backup
            auto f = [&] {
                CONCORE_PROFILING_SCOPE_N("backup wrapper");

                // Acquire a free backup engine
                backup_engine cur_engine;
                bool res = engines.try_pop(cur_engine);
                assert(res);

                // Actually perform the backup
                cur_engine.save();

                // Release the backup engine
                engines.push(std::move(cur_engine));
            };
            ser.execute(concore::task{std::move(f), grp});
        }
    };

    concore::spawn_and_wait(running_process);

    concore::wait(grp); // ensure the serializer finishes

    return 0;
}
