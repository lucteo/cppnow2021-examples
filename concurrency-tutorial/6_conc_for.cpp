#include <concore/conc_for.hpp>

#include "../common/utils.hpp"

void work(int idx) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
    printf("I'm a proud worker, working on part %d\n", idx);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    init_randomness();

    concore::conc_for(0, 20, work);

    // std::for_each(std::execution::par, int_iter{0}, int_iter{20}, work);

    return 0;
}
