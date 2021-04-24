#pragma once

#include <concore/profiling.hpp>

#include <chrono>
#include <thread>
#include <random>
#include <stdio.h>
#include <time.h>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;

using std::chrono::milliseconds;
using std::chrono::microseconds;

//! Helper class that sleeps a bit on construction and destruction.
//! Allows Tracy to properly send all the messages for short applications.
struct profiling_sleep {
    profiling_sleep() {
#if TRACY_ENABLE
        sleep_for(300ms);
#endif
    }
    ~profiling_sleep() {
#if TRACY_ENABLE
        sleep_for(300ms);
#endif
    }
};

inline std::mt19937& get_random_object() {
    static std::mt19937 randomness;
    return randomness;
}

inline void init_randomness() {
    get_random_object().seed(time(nullptr));
}

inline void sleep_in_between_ms(int low, int high) {
    int val = low + get_random_object()() % (high-low);
    sleep_for(milliseconds(val));
}

