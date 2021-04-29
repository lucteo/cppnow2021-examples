#include <concore/spawn.hpp>
#include <concore/conc_for.hpp>
#include <concore/conc_reduce.hpp>
#include <concore/init.hpp>

#include "../common/utils.hpp"
#include "../common/cpu_work.hpp"

#include <vector>

double transform_work_1(int idx) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(500ms);
    return double(idx);
}

double transform_work_2(int idx) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(500ms);
    return double(idx);
}

double reduce_work_1(double l, double r) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(200ms);
    return l + r;
}

double reduce_work_2(double l, double r) {
    CONCORE_PROFILING_FUNCTION();
    sleep_for(200ms);
    return l + r;
}

double test_sequence(const std::vector<int>& vals) {
    CONCORE_PROFILING_FUNCTION();

    // Step 1: transform the values
    std::vector<double> vals2;
    vals2.resize(vals.size());
    auto tr_fun = [&](int i) { vals2[i] = transform_work_1(i); };
    concore::conc_for(0, int(vals.size()), std::move(tr_fun));

    // Step 2: reduce the data
    return concore::conc_reduce(vals2.begin(), vals2.end(), 0.0, reduce_work_1, reduce_work_1);
}

double test_fusion(const std::vector<int>& vals) {
    CONCORE_PROFILING_FUNCTION();

    // Fuse the two operations
    auto fuse_op = [](double lhs, int rhs) -> double {
        return reduce_work_2(lhs, transform_work_2(rhs));
    };
    return concore::conc_reduce(vals.begin(), vals.end(), 0.0, fuse_op, reduce_work_2);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Create the initial vector of elements
    std::vector<int> v;
    for (int i = 0; i < 40; i++)
        v.push_back(i);

    double r1 = test_sequence(v);
    double r2 = test_fusion(v);
    printf("First method: %g\n", r1);
    printf("Second method: %g\n", r2);

    // Start other tasks while we have our algorithm running
    auto grp = concore::task_group::create();
    auto f = [] {
        CONCORE_PROFILING_SCOPE_N("parallel task")
        sleep_for(3200ms);
        // Spawn some more tasks
        for (int i = 0; i < 10; i++)
            concore::spawn([] {
                CONCORE_PROFILING_SCOPE_N("other task");
                sleep_for(1500ms);
            });
    };
    concore::spawn(std::move(f), grp);
    test_fusion(v);

    concore::wait(grp);

    // Things to notice:
    // - algorithms composabiliy is not great
    // - better to fuse algorithms
    // - latency problems in the presence of other work

    return 0;
}
