#include <concore/spawn.hpp>

#include "../common/utils.hpp"

void print_message_task(const char* msg) {
    CONCORE_PROFILING_SCOPE();
    CONCORE_PROFILING_SET_TEXT(msg);

    printf(" %s", msg);

    sleep_for(100ms);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Create a task group, so that we keep track of the running tasks
    auto grp = concore::task_group::create();

    // Create 9 tasks to be run concurrently
    concore::spawn([=] { print_message_task("How"); }, grp);
    concore::spawn([=] { print_message_task("did"); }, grp);
    concore::spawn([=] { print_message_task("the"); }, grp);
    concore::spawn([=] { print_message_task("multi-threaded"); }, grp);
    concore::spawn([=] { print_message_task("chicken"); }, grp);
    concore::spawn([=] { print_message_task("cross"); }, grp);
    concore::spawn([=] { print_message_task("the"); }, grp);
    concore::spawn([=] { print_message_task("road"); }, grp);
    concore::spawn([=] { print_message_task("?"); }, grp);

    // Ensure that all the tasks are completed
    // This performs a BUSY WAIT -- it takes tasks and executes them
    concore::wait(grp);

    printf("\n");
    return 0;
}
