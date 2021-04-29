#include <concore/spawn.hpp>
#include <concore/init.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

void work() {
    CONCORE_PROFILING_FUNCTION();
    cpu_busy_work_large_unit();
}

static constexpr int num_tasks = 96;

void run_tasks_serially() {
    CONCORE_PROFILING_FUNCTION();
    for (int i = 0; i < num_tasks; i++)
        work();
}

void set_num_workers(int count) {
    CONCORE_PROFILING_FUNCTION();
    concore::shutdown();
    concore::init_data config;
    config.num_workers_ = count;
    concore::init(config);
}

double time_run_tasks() {
    CONCORE_PROFILING_FUNCTION();

    using clock = std::chrono::high_resolution_clock;
    auto start = clock::now();

    auto grp = concore::task_group::create();
    for (int i = 0; i < num_tasks; i++)
        concore::spawn(concore::task{work, grp});
    concore::wait(grp);

    auto end = clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    return elapsed.count();
}

double run_test(int num_workers) {
    set_num_workers(num_workers);

    CONCORE_PROFILING_FUNCTION();

    double dur_ms;
    std::atomic<bool> done{false};

    concore::spawn([&] {
        dur_ms = time_run_tasks();
        done = true;
    });

    // Just sleep until the other thread is done
    while (!done.load())
        sleep_for(500ms);

    return dur_ms;
}

void report_n_threads(int count, double t1) {
    double t_n = run_test(count);
    printf("Time %d threads: %g; speedup=%.2f\n", count, t_n, t1/t_n);
    fflush(stdout);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    double t1 = run_test(1);
    printf("Time 1 thread: %g\n", t1);
    fflush(stdout);

    report_n_threads(2, t1);
    report_n_threads(4, t1);
    // report_n_threads(6, t1);
    // report_n_threads(8, t1);
    // report_n_threads(12, t1);
    // report_n_threads(24, t1);

    // Things to notice:
    // - speedup for different number of threads
    // - speedup growth after 6 threads
    // - speedup for 24 threads vs 12 threads
    // - indirect contention

    // Possible run results:
    //      Time 1 thread: 21744.1
    //      Time 2 threads: 10938.9; speedup=1.99
    //      Time 4 threads: 5473.57; speedup=3.97
    //      Time 6 threads: 3760.39; speedup=5.78
    //      Time 8 threads: 3354.62; speedup=6.48
    //      Time 12 threads: 3032.51; speedup=7.17
    //      Time 24 threads: 3052.63; speedup=7.12    

    return 0;
}
