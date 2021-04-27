#include <concore/execution.hpp>
#include <concore/thread_pool.hpp>
#include <concore/sender_algo/just.hpp>
#include <concore/sender_algo/just_on.hpp>
#include <concore/sender_algo/on.hpp>
#include <concore/sender_algo/sync_wait.hpp>
#include <concore/sender_algo/transform.hpp>
#include <concore/sender_algo/when_all.hpp>
#include <concore/sender_algo/let_value.hpp>

#include "../common/utils.hpp"

/*
P1897: Towards C++23 executors: A proposal for an initial set of algorithms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sender algorithms:

- just(v...)
- just_on(sch, v...)
- on(s, sch)
- sync_wait(s)
- transform(s, f)
- when_all(s...)

- let_value(s, f)
- let_error(s, f)
- ensure_started(s)
*/

struct my_int_receiver {
    void set_value(int val) {
        sleep_for(50ms);
        printf("Received int: %d\n", val);
    }
    void set_done() noexcept {}
    void set_error(std::exception_ptr) noexcept {}
};

int main() {
    profiling_sleep profiling_helper;

    // 'just' creates a sender of the specified value
    {
        printf("just\n");
        auto sndr = concore::just(17);
        concore::submit(std::move(sndr), my_int_receiver{});
        printf("Receiver should be called by now\n");
    }
    printf("---\n");

    // 'on' moves work on the given scheduler
    {
        concore::static_thread_pool pool{3};
        auto sched = pool.scheduler();

        printf("just + on\n");
        auto sndr = concore::just(19) | concore::on(sched);

        concore::submit(std::move(sndr), my_int_receiver{});
        printf("receiver is called on the thread pool\n");
    }
    printf("---\n");

    // 'just_on' == 'just' + 'on'
    {
        concore::static_thread_pool pool{3};
        auto sched = pool.scheduler();

        printf("just_on\n");
        auto sndr = concore::just_on(sched, 23);

        concore::submit(std::move(sndr), my_int_receiver{});
        printf("receiver is called on the thread pool\n");
    }
    printf("---\n");

    // 'sync_wait' waits for the value -- DO NOT ABUSE -- typically an anti-pattern
    {
        concore::static_thread_pool pool{3};
        auto sched = pool.scheduler();

        printf("sync_wait\n");
        auto sndr = concore::just(29) | concore::on(sched);

        auto val = concore::sync_wait(sndr);
        printf("Value obtained: %d\n", val);
    }
    printf("---\n");

    // 'transform' will transform the values passed, with the given functor
    {
        concore::static_thread_pool pool{3};
        auto sched = pool.scheduler();

        printf("transform\n");
        auto f = [](int x) { return 3.141592 * x; };
        auto sndr = concore::just(2) | concore::on(sched) | concore::transform(f);

        auto val = concore::sync_wait(sndr);
        printf("Value obtained: %g\n", val);
    }
    printf("---\n");

    // 'when_all' joins multiple senders
    {
        concore::static_thread_pool pool{3};
        auto sched = pool.scheduler();

        printf("when_all\n");
        auto sndr1 = concore::just(299792458L);
        auto sndr2 = concore::just(2.71828);
        auto sndr3 = concore::just(std::string{"E=mc^2"});

        auto sndr_all = concore::when_all(sndr1, sndr2, sndr3);

        auto f = [](long l, double d, std::string s) {
            printf("Received values: %ld, %g, %s\n", l, d, s.c_str());
            return true;
        };
        bool res = std::move(sndr_all) | concore::transform(f) | concore::sync_wait();
        printf("A true value: %s\n", res ? "true" : "false");
    }
    printf("---\n");

    return 0;
}
