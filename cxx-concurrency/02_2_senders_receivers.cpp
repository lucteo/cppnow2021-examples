#include <concore/execution.hpp>
#include <concore/thread_pool.hpp>
#include <concore/as_receiver.hpp>
#include <concore/as_invocable.hpp>
#include <concore/as_sender.hpp>
#include <concore/as_operation.hpp>

#include "../common/utils.hpp"

void test_as_receiver(concore::static_thread_pool::scheduler_type& sched) {
    auto receiver_fun = []() {
        printf("The receiver functor was called\n");
        sleep_for(10ms);
    };

    auto recv = concore::as_receiver<decltype(receiver_fun)>(std::move(receiver_fun));
    concore::submit(sched.schedule(), std::move(recv));
}

struct my_receiver {
    void set_value() noexcept {
        printf("I did receive a call!\n");
        sleep_for(10ms);
    }
    void set_done() noexcept {}
    void set_error(std::exception_ptr) noexcept {}
};

void test_as_invocable() {
    my_receiver recv;
    auto task_fun = concore::as_invocable<my_receiver>(recv);

    // Invoke it as a simple function
    task_fun();
}

void test_as_sender(concore::static_thread_pool& pool) {
    const auto& exec = pool.executor();

    // executor -> scheduler -> sender

    auto sndr = concore::as_sender<decltype(exec)>(exec);

    // concore::submit(sndr, my_receiver{});
    // TODO
}

void test_as_operation(concore::static_thread_pool& pool) {
    const auto& exec = pool.executor();

    // executor -> scheduler -> sender
    // + receiver
    // => operation_state

    my_receiver recv;
    auto op_state = concore::as_operation<decltype(exec), my_receiver>(exec, recv);
    // concore::start(std::move(op_state));
    // TODO
}



int main() {
    profiling_sleep profiling_helper;

    concore::static_thread_pool pool{3};
    auto sched = pool.scheduler();

    test_as_receiver(sched);
    pool.wait();

    test_as_invocable();

    test_as_sender(pool);
    pool.wait();

    test_as_operation(pool);
    pool.wait();

    return 0;
}
