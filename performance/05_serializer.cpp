#include <concore/spawn.hpp>
#include <concore/serializer.hpp>
#include <concore/init.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

#include <mutex>

void serialized_work() {
    CONCORE_PROFILING_FUNCTION();
    cpu_busy_work_large_unit();
}

void other_work() {
    CONCORE_PROFILING_FUNCTION();
    cpu_busy_work_large_unit();
}

static constexpr int num_ser_tasks = 30;
static constexpr int num_other_tasks = 100;

void test_serializer() {
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    concore::serializer ser;

    // Spawn tasks to be serialized
    for ( int i=0; i<num_ser_tasks; i++ )
        ser.execute(concore::task{serialized_work, grp});

    // Spawn other tasks
    for ( int i=0; i<num_other_tasks; i++ )
        concore::spawn(concore::task{other_work, grp});

    concore::wait(grp);
}

void test_mutex() {
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    concore::serializer ser;

    // Spawn tasks to be serialized
    std::mutex bottleneck;          // Mutexes are bottlenecks
    for ( int i=0; i<num_ser_tasks; i++ ) {
        auto f = [&] {
            CONCORE_PROFILING_SCOPE_N("wrapper task");
            std::scoped_lock<std::mutex> lock{bottleneck};
            serialized_work();
        };
        concore::spawn(concore::task{std::move(f), grp});
    }

    // Spawn other tasks
    for ( int i=0; i<num_other_tasks; i++ )
        concore::spawn(concore::task{other_work, grp});

    concore::wait(grp);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Don't kill the CPU while presenting this live
    concore::init_data config;
    config.num_workers_ = 3;
    concore::init(config);

    test_serializer();
    test_mutex();

    // Things to notice:
    // - using serializers can achieve maximum throughput
    // - locks are bottlenecks

    return 0;
}
