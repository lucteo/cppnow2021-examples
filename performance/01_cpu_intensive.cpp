#include <concore/spawn.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

void work_5s() {
    CONCORE_PROFILING_SCOPE_N("5 seconds, 1 CPU busy");
    do_work_for(5s);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Run a simple test to inspect the size of a work unit
    concore::spawn_and_wait([]{
        CONCORE_PROFILING_SCOPE_N("1000 work units");
        for ( int i=0; i<1000; i++ )
            cpu_busy_work_unit();
    });

    // Run a simple test to inspect the size of a large work unit
    concore::spawn_and_wait([]{
        CONCORE_PROFILING_SCOPE_N("large work unit");
        cpu_busy_work_large_unit();
    });

    // Keep one CPU busy for a few seconds
    concore::spawn_and_wait(work_5s);

    // Keep 4 CPUs busy for a few seconds
    concore::spawn_and_wait({work_5s, work_5s, work_5s, work_5s});

    return 0;
}
