#include <concore/spawn.hpp>
#include <concore/finish_task.hpp>

#include "../common/utils.hpp"

void do_work_1() {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(80, 120);
    printf("We have some work\n");
}

void do_work_2() {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(80, 120);
    printf("Competition is also doing work\n");
}

void do_work_3() {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(80, 120);
    printf("And there are some other things to be done\n");
}

void done_work() {
    CONCORE_PROFILING_FUNCTION();
    printf("-\n");
    printf("All the work is done. Have a good night!\n");
    sleep_for(100ms);
}

void example_1() {
    CONCORE_PROFILING_FUNCTION();

    concore::finish_wait done;

    // Spawn 3 tasks
    concore::spawn(concore::task{do_work_1, {}, done.get_continuation()});
    concore::spawn(concore::task{do_work_2, {}, done.get_continuation()});
    concore::spawn(concore::task{do_work_3, {}, done.get_continuation()});

    // This is a BUSY wait: it will attempt to execute work
    done.wait();
}

void example_2() {
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    concore::finish_task done_task(concore::task{done_work, grp});

    // Spawn 3 tasks
    concore::spawn(concore::task{do_work_1, grp, done_task.get_continuation()});
    concore::spawn(concore::task{do_work_2, grp, done_task.get_continuation()});
    concore::spawn(concore::task{do_work_3, grp, done_task.get_continuation()});

    // When all the tasks are executed, the done-task will be called
    // This will call done_work()
    concore::wait(grp);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    init_randomness();

    printf("---\n");
    example_1();

    printf("---\n");
    example_2();

    printf("---\n");
    return 0;
}
