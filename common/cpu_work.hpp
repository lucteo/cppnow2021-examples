#pragma once

#include "utils.hpp"

inline void cpu_busy_work_unit() {
    uint64_t a = 0;
    uint64_t b = 1;
    while (b < 0x8000000000000000ULL) {
        auto sum = a + b;
        a = b;
        b = sum;
    }
}

inline void cpu_busy_work_large_unit() {
    for (int i=0; i<1'000'000; i++)
        cpu_busy_work_unit();
}

template <typename Rep, typename Period>
inline void do_work_for(const std::chrono::duration<Rep, Period>& dur) {
    using clock = std::chrono::high_resolution_clock;
    auto stopTime = clock::now() + dur;
    do {
        cpu_busy_work_unit();
    } while (clock::now() < stopTime);
}