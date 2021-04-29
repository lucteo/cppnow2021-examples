#include <concore/spawn.hpp>
#include <concore/init.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

void work() {
    CONCORE_PROFILING_FUNCTION();
    cpu_busy_work_large_unit();
}

static constexpr int num_tasks = 50;

void run_tasks_serially() {
    CONCORE_PROFILING_FUNCTION();
    for (int i = 0; i < num_tasks; i++)
        work();
}

void run_tasks_with_spawn() {
    CONCORE_PROFILING_FUNCTION();
    auto grp = concore::task_group::create();
    for (int i = 0; i < num_tasks; i++)
        concore::spawn(concore::task{work, grp});
    concore::wait(grp);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Limit to 1 global working thread
    concore::init_data config;
    config.num_workers_ = 1;
    concore::init(config);

    // First, run the test serially
    run_tasks_serially();

    // Now, run the test that spawns the task.
    // Make sure this thread doesn't get any work
    std::atomic<bool> done{false};
    concore::spawn([&] {
        run_tasks_with_spawn();
        done = true;
    });

    // Just sleep until the other thread is done
    while (!done.load())
        sleep_for(500ms);

    // Things to notice:
    // - the two methods execute in roughly the same amount of time
    // - gap between tasks (serial execution vs spawn-based execution)

    return 0;
}
