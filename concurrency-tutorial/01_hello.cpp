#include <concore/spawn.hpp>

int main() {
    // Create a task and executes it
    // The task can run in the same thread, or a different thread
    concore::spawn_and_wait([] {
        printf("Hello, concurrent world!\n");
    });

    return 0;
}
