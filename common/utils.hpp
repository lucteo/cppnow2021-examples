#pragma once

#include <concore/profiling.hpp>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using std::this_thread::sleep_for;


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
