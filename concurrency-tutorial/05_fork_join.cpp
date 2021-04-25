#include <concore/spawn.hpp>

#include "../common/utils.hpp"

template <typename F>
void conc_apply(int start, int end, int granularity, F f) {
    if (end - start <= granularity)
        for (int i = start; i < end; i++)
            f(i);
    else {
        int mid = start + (end - start) / 2;
        concore::spawn_and_wait({
                // first half
                [=] { conc_apply(start, mid, granularity, f); },
                // second half
                [=] { conc_apply(mid, end, granularity, f); } //
        });
    }
}

template <typename F>
void conc_apply_variant(int start, int end, int granularity, F f) {
    if (end - start <= granularity)
        for (int i = start; i < end; i++)
            f(i);
    else {
        int mid = start + (end - start) / 2;

        auto grp = concore::task_group::create();
        concore::spawn([=] { conc_apply(start, mid, granularity, f); }, grp);
        concore::spawn([=] { conc_apply(mid, end, granularity, f); }, grp);
        concore::wait(grp);
    }
}

void work(int idx) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
    printf("I'm a proud worker, working on part %d\n", idx);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    init_randomness();

    concore::spawn_and_wait([] { printf("Nothing new. We've already seen this...\n"); });
    printf("---\n");

    conc_apply(0, 20, 1, work);
    printf("---\n");
    conc_apply_variant(0, 20, 1, work);

    return 0;
}
