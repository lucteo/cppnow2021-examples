#include <concore/spawn.hpp>
#include <concore/init.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Limit to 4 global working threads
    concore::init_data config;
    config.num_workers_ = 4;
    concore::init(config);

    auto grp = concore::task_group::create();

    {
        CONCORE_PROFILING_SCOPE_N("starting tasks")

        // Create 100 tasks
        auto task_fun = [] {
            CONCORE_PROFILING_SCOPE_N("task");
            do_work_for(100ms);
        };
        for (int i = 0; i < 100; i++)
            concore::spawn(concore::task{task_fun, grp});

        // Wait for all the tasks to finish
        concore::wait(grp);
    }

    // Things to notice:
    // - number of threads that execute work: 4+1
    // - gap between tasks

    return 0;
}
