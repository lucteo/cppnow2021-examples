#include <concore/execution.hpp>
#include <concore/thread_pool.hpp>
#include <concore/as_receiver.hpp>

#include "../common/utils.hpp"

// Receiver of void, and sender of int
template <typename S>
struct value_sender {
    int val_;
    S base_sender_;

    value_sender(int val, S&& base_sender)
        : val_(val)
        , base_sender_((S &&) base_sender) {}

    template <template <typename...> class Tuple, template <typename...> class Variant>
    using value_types = Variant<Tuple<int>>;
    template <template <typename...> class Variant>
    using error_types = Variant<std::exception_ptr>;
    static constexpr bool sends_done = true;

    template <typename R>
    struct op_state {
        struct void_receiver {
            int val_;
            R final_receiver_;

            void_receiver(int val, R&& final_receiver)
                : val_(val)
                , final_receiver_((R &&) final_receiver) {}

            void set_value() noexcept { ((R &&) final_receiver_).set_value(val_); }
            void set_done() noexcept { final_receiver_.set_done(); }
            void set_error(std::exception_ptr e) noexcept { final_receiver_.set_error(e); }
        };

        typename concore::connect_result_t<S, void_receiver> kickoff_op_;

        op_state(int val, R&& recv, S base_sender)
            : kickoff_op_(concore::connect((S &&) base_sender, void_receiver{val, (R &&) recv})) {}

        void start() noexcept { concore::start(kickoff_op_); }
    };

    template <typename R>
    op_state<R> connect(R&& recv) {
        return {val_, (R &&) recv, (S &&) base_sender_};
    }
};

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

    concore::static_thread_pool pool{3};
    auto sched = pool.scheduler();

    // transforms a sender into another sender
    auto val_sender = value_sender{3, sched.schedule()};
    concore::submit(val_sender, my_int_receiver{});

    printf("work was submitted; receiver should get the signal concurrently\n");

    pool.wait();

    return 0;
}
