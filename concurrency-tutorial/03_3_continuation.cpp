#include <concore/spawn.hpp>

#include "../common/utils.hpp"

#include <vector>

using task_function = std::function<void()>;

template <typename T>
struct async_result {
    std::shared_ptr<T> val_;
    std::vector<task_function> work_items_{};

    template <typename T2, typename F>
    async_result<T2> then(F f) {
        // Steal the work items we had so far
        std::vector<task_function> new_work_items{std::move(work_items_)};

        // Build a new work item that executes the given function
        auto prev_val = val_;
        auto new_val_ptr = std::make_shared<T2>();
        new_work_items.emplace_back([=] { *new_val_ptr = f(*prev_val); });
        // concore::task new_task{};

        return async_result<T2>{new_val_ptr, std::move(new_work_items)};
    }

    void start() {
        task_function last_work;
        // concore::task t;
        bool last_elem = true;
        for (auto it = rbegin(work_items_); it != rend(work_items_); it++) {
            if (last_work) {
                auto new_work = [w = std::move(*it), next_work = std::move(last_work)] {
                    // Run the current work item
                    try {
                        w();
                    } catch (...) {
                        std::terminate();
                    }
                    // Then spawn a task for the next work
                    concore::spawn(std::move(next_work));
                };
                last_work = std::move(new_work);
            } else
                last_work = std::move(*it);
        }
        concore::spawn(last_work);
    }
};

template <typename T>
async_result<T> start_async(T val) {
    return {std::make_shared<T>(val), {}};
}

void do_something_else() {
    CONCORE_PROFILING_FUNCTION();
    for (int i = 0; i < 10; i++) {
        printf(".");
        sleep_for(45ms);
    }
    printf("\n");
}

int sqr(int x) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(100ms);

    return x * x;
}

double timesPi(int x) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(100ms);

    constexpr double pi = 3.141592653589793238462643383279502884L;
    return pi * x;
}

bool print(double x) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(100ms);

    printf("%g", x);
    return true;
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // clang-format off
    auto chain = start_async(3)
        .then<int>(sqr)
        .then<double>(timesPi)
        .then<bool>(print)
        ;
    // clang-format on

    chain.start();

    // Do some work while the chain is processing in the background
    concore::spawn_and_wait(do_something_else);

    printf("\n");
    return 0;
}
