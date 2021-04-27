#include <concore/execution.hpp>
#include <concore/spawn.hpp>
#include <concore/global_executor.hpp>
#include <concore/any_executor.hpp>
#include <concore/delegating_executor.hpp>
#include <concore/inline_executor.hpp>
#include <concore/thread_pool.hpp>

#include <concore/serializer.hpp>
#include <concore/rw_serializer.hpp>
#include <concore/n_serializer.hpp>

#include "../common/utils.hpp"

template <typename T>
void ensure_executor() {
    static_assert(concore::executor<T>, "Given type is not an executor");
}

void a_series_of_executors() {
    // main
    ensure_executor<concore::spawn_executor>();
    ensure_executor<concore::global_executor>();

    // generic
    ensure_executor<concore::any_executor>();

    // other
    ensure_executor<concore::delegating_executor>();
    ensure_executor<concore::inline_executor>();
    ensure_executor<concore::static_thread_pool::executor_type>();

    // serializers
    ensure_executor<concore::serializer>();
    ensure_executor<concore::n_serializer>();
    ensure_executor<concore::rw_serializer::reader_type>();
    ensure_executor<concore::rw_serializer::writer_type>();
}

template <typename E>
int test_max_parallelism(const E& executor) {
    CONCORE_PROFILING_FUNCTION();

    std::atomic<int> num_parallel{0};
    std::atomic<int> max_parallel{0};
    std::atomic<int> count{0};

    auto task_fun = [&] {
        CONCORE_PROFILING_SCOPE_N("task");
        // Increase parallelism count
        int cur_par = ++num_parallel;
        // Check max parallelism
        int old_max = max_parallel.load();
        while (old_max < cur_par && !max_parallel.compare_exchange_weak(old_max, cur_par))
            ; // empty

        sleep_in_between_ms(5, 10);

        // Decrease parallelism count
        num_parallel--;

        // Finished another task
        count++;
    };

    // Spawn some tasks
    static constexpr int num_tasks = 50;
    for (int i = 0; i < 50; i++)
        concore::execute(executor, task_fun);

    // Poor-man sync
    while (count.load() < num_tasks)
        sleep_for(1ms);

    return max_parallel.load();
}

void test_different_executors() {
    CONCORE_PROFILING_FUNCTION();

    // Static thread pool
    {
        concore::static_thread_pool pool{3};

        int par = test_max_parallelism(pool.executor());
        printf("Max observed parallelism for static_thread_pool(3) is %d\n", par);

        pool.wait();
    }

    // Global executor
    {
        int par = test_max_parallelism(concore::global_executor{});
        printf("Max observed parallelism for global_executor is %d\n", par);
    }

    // Spawn executor
    {
        int par = test_max_parallelism(concore::spawn_executor{});
        printf("Max observed parallelism for spawn_executor is %d\n", par);
    }

    // Serializer executor
    {
        int par = test_max_parallelism(concore::serializer{});
        printf("Max observed parallelism for serializer is %d\n", par);
    }

    // n_serializer executor
    {
        int par = test_max_parallelism(concore::n_serializer{3});
        printf("Max observed parallelism for n_serializer(3) is %d\n", par);
    }
}

struct custom_executor {
    template <typename F>
    void execute(F&& f) const {
        printf("custom_executor: about to execute something...\n");
        f();
        printf("custom_executor: done executing\n");
    }

    friend inline bool operator==(custom_executor, custom_executor) { return true; }
    friend inline bool operator!=(custom_executor, custom_executor) { return false; }
};

void test_custom_executor() {
    ensure_executor<custom_executor>();

    auto work = [] {
        printf("Performing some work...\n");
        sleep_for(10ms);
    };

    concore::execute(custom_executor{}, std::move(work));
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    a_series_of_executors();

    test_different_executors();

    printf("---\n");
    test_custom_executor();

    return 0;
}
