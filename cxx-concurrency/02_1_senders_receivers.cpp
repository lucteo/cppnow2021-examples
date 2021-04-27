#include <concore/execution.hpp>
#include <concore/thread_pool.hpp>

#include "../common/utils.hpp"

/*
P0443: A Unified Executors Proposal for C++ 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- receiver: something hat receives values (or cues from a sender)
- sender: something that sends values/cues to a receiver

- operation_state = sender + receiver
    - result of connect(sender, receiver)
    - can be started: concore::start(op_state)
    - similar to a task

- scheduler: something that can create senders
    - something in between executors and senders

KEY IDEA: express computations as a pair of sender + receiver
*/

// A receiver is something that receives values (or cues) from a sender
struct my_receiver {
    void set_value() noexcept {
        printf("Receiver: I received a cue that I need to do something\n");
        sleep_for(10ms);
    }
    void set_done() noexcept {
        printf("Receiver: Action was cancelled\n");
        std::terminate();
    }
    void set_error(std::exception_ptr) noexcept {
        printf("Receiver: An error occurred somewhere before this\n");
        std::terminate();
    }
};

int main() {
    profiling_sleep profiling_helper;

    concore::static_thread_pool pool{3};

    // A scheduler can create senders
    auto sched = pool.scheduler();
    auto sndr = concore::schedule(sched);

    // Senders can be connected to receivers
    auto op_state = concore::connect(sndr, my_receiver{});
    concore::start(op_state); // can be started a bit later

    // Connect and start can be simplified with submit
    concore::submit(sndr, my_receiver{});

    pool.wait();

    return 0;
}
