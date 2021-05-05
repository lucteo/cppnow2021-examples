#include <concore/spawn.hpp>
#include <concore/any_executor.hpp>
#include <concore/inline_executor.hpp>

#include "../common/utils.hpp"

// Just the prototype of a a data stream -- not actually used
// Very, very simplistic
// It can be connected to a 'receiver' and it's called when a value arrives
template <typename T>
struct data_stream_prototype {
    using value_type = T;

    template <typename Recv>
    void connect(Recv recv);

    void on_value(T&& val) const;
};

// A source stream; just pushes the given values to the receiver
template <typename T>
struct stream_source {
    using value_type = T;

    template <typename Recv>
    void connect(const Recv& recv) {
        recv_fun_ = [&recv](T&& val) mutable { recv.on_value((T &&) val); };
    }
    void on_value(T&& val) const { push_value((T &&) val); }

    void push_value(T val) {
        if (recv_fun_)
            recv_fun_((T &&) val);
    }

private:
    using recv_fun_type = std::function<void(T)>;
    recv_fun_type recv_fun_;
};

// Map stream: applies a transformation function over the values received
template <typename T, typename T2>
struct map_stream {
    using value_type = T;
    using map_fun_type = std::function<T2(T)>;

    explicit map_stream(map_fun_type f, concore::task_group grp = {},
            concore::any_executor ex = concore::spawn_executor{})
        : fun_(std::move(f))
        , grp_(grp)
        , ex_(ex) {}

    template <typename Recv>
    void connect(const Recv& recv) {
        recv_fun_ = [&recv](T2 val) mutable { recv.on_value((T2 &&) val); };
    }
    void on_value(T&& val) const {
        auto task_fun = [val = std::move(val), this] {
            auto v2 = fun_((T &&) val);
            if (recv_fun_)
                recv_fun_(std::move(v2));
        };
        concore::execute(ex_, concore::task{std::move(task_fun), grp_});
    }

private:
    map_fun_type fun_;
    concore::task_group grp_;
    concore::any_executor ex_;
    using recv_fun_type = std::function<void(T2)>;
    recv_fun_type recv_fun_;
};

// Filter stream: only pushes values if they satisfy the given predicate
template <typename T>
struct filter_stream {
    using value_type = T;
    using filter_fun_task = std::function<bool(T)>;

    explicit filter_stream(filter_fun_task f, concore::task_group grp = {},
            concore::any_executor ex = concore::inline_executor{})
        : fun_(std::move(f))
        , grp_(grp)
        , ex_(ex) {}

    template <typename Recv>
    void connect(const Recv& recv) {
        recv_fun_ = [&recv](T&& val) mutable { recv.on_value((T &&) val); };
    }
    void on_value(T&& val) const {
        auto task_fun = [val = std::move(val), this] {
            bool should_allow = fun_((const T&)val);
            if (should_allow && recv_fun_)
                recv_fun_((T &&) val);
        };
        concore::execute(ex_, concore::task{std::move(task_fun), grp_});
    }

private:
    filter_fun_task fun_;
    concore::task_group grp_;
    concore::any_executor ex_;
    using recv_fun_type = std::function<void(T)>;
    recv_fun_type recv_fun_;
};

// Split stream: splits the stream into however many streams needed by the connected receivers
template <typename T>
struct split_stream {
    using value_type = T;

    explicit split_stream(
            concore::task_group grp = {}, concore::any_executor ex = concore::spawn_executor{})
        : grp_(grp)
        , ex_(ex) {}

    template <typename Recv>
    void connect(const Recv& recv) {
        recv_funs_.emplace_back([&recv](T val) mutable { recv.on_value((T &&) val); });
    }
    void on_value(T&& val) const {
        for (const auto& f : recv_funs_) {
            if (f) {
                auto task_fun = [val, f] { f(val); };
                concore::execute(ex_, concore::task{task_fun, grp_});
            }
        }
    }

private:
    concore::task_group grp_;
    concore::any_executor ex_;
    using recv_fun_type = std::function<void(T)>;
    std::vector<recv_fun_type> recv_funs_;
};


int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    /*
                                      /-> filter -> transform
        source -> transform -> split *
                                      \-> filter -> transform
    */

    auto tr = [](int x) {
        CONCORE_PROFILING_SCOPE_N("transform");
        sleep_for(10ms);
        return sin(x * 3.141592 / 180.0);
    };
    auto flt_left = [](double x) {
        CONCORE_PROFILING_SCOPE_N("flt_left");
        sleep_for(3ms);
        return x < 0;
    };
    auto flt_right = [](double x) {
        CONCORE_PROFILING_SCOPE_N("flt_right");
        sleep_for(3ms);
        return x >= 0;
    };
    auto print_l = [](double x) {
        CONCORE_PROFILING_SCOPE_N("print_l");
        int indent = 40 - int(40 * (-x));
        int i = 0;
        for (; i < indent; i++)
            printf(" ");
        for (; i < 40; i++)
            printf("L");
        printf("\n");
        fflush(stdout);
        sleep_for(10ms);
        return true;
    };
    auto print_r = [](double x) {
        CONCORE_PROFILING_SCOPE_N("print_r");
        int indent = int(40 * x);
        printf("                                        ");
        for (int i = 0; i < indent; i++)
            printf("R");
        printf("\n");
        fflush(stdout);
        sleep_for(10ms);
        return true;
    };

    auto grp = concore::task_group::create();

    stream_source<int> src;
    map_stream<int, double> map_s{tr, grp};
    split_stream<double> split_s{grp};
    filter_stream<double> flt_left_s{flt_left};
    filter_stream<double> flt_right_s{flt_right};
    map_stream<double, bool> print_left_s{print_l, grp};
    map_stream<double, bool> print_right_s{print_r, grp};

    src.connect(map_s);
    map_s.connect(split_s);
    split_s.connect(flt_left_s);
    split_s.connect(flt_right_s);
    flt_left_s.connect(print_left_s);
    flt_right_s.connect(print_right_s);

    static constexpr int num_values = 2 * 360;
    for (int i = 0; i < 2 * 360; i++) {
        src.push_value(i);
        sleep_for(10ms);
    }

    // Wait for all tasks to complete
    concore::wait(grp);

    return 0;
}
