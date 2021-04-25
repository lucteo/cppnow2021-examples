#include <concore/rw_serializer.hpp>
#include <concore/serializer.hpp>
#include <concore/spawn.hpp>

#include "../common/utils.hpp"

#include <vector>
#include <cmath>
#include <algorithm>

class running_window {
    int window_size_{1};
    int op_idx_{0};
    std::vector<double> values_;

public:
    explicit running_window(int size)
        : window_size_(size) {}

    // Cannot call this in parallel
    void add(double val) {
        CONCORE_PROFILING_FUNCTION();
        int old_idx = op_idx_;

        // Ensure we don't have too many elements in our vector
        if (int(values_.size()) > window_size_ + 1)
            values_.erase(values_.begin());
        // Add the new element in the vector
        values_.push_back(val);
        // We have a new operation
        op_idx_++;

        // assume this is slightly more complex
        sleep_in_between_ms(2, 5);

        // sanity checking code
        if (op_idx_ != old_idx + 1) {
            printf("ERROR: concurrency bugs in the software: %d != %d\n", op_idx_, old_idx + 1);
            std::terminate();
        }
    }

    // Also cannot be called in parallel
    double get_average() const {
        CONCORE_PROFILING_FUNCTION();
        int old_idx = op_idx_;

        static double nan = std::nan("");
        if (values_.empty())
            return nan;

        // assume this is slightly more complex
        sleep_in_between_ms(2, 5);

        double sum = std::accumulate(values_.begin(), values_.end(), 0.0);

        // sanity checking code
        if (op_idx_ != old_idx) {
            printf("ERROR: concurrency bugs in the software (calling get_average: %d != %d)\n",
                    op_idx_, old_idx);
            std::terminate();
        }

        return sum / double(values_.size());
    }
};

void safety_with_serializer() {
    CONCORE_PROFILING_FUNCTION();
    running_window my_window(10); // cannot be shared

    auto grp = concore::task_group::create();
    concore::serializer ser;

    auto producer_process = [&]() {
        CONCORE_PROFILING_SCOPE_N("producer");
        std::random_device providence;
        std::mt19937 crystal_ball{providence()};
        std::uniform_real_distribution<> fortune_teller(1.0, 100.0);

        for (int i = 0; i < 100; i++) {
            CONCORE_PROFILING_SCOPE_N("iter");
            CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", i);
            // Wait for a bit
            sleep_in_between_ms(7, 10);
            // Produce one value
            double val = fortune_teller(crystal_ball);

            // Safely push the value into our average window object
            concore::task t([&, val] { my_window.add(val); }, grp);
            concore::execute(ser, std::move(t));
        }
    };

    auto average_consumer_process = [&]() {
        CONCORE_PROFILING_SCOPE_N("consumer");

        for (int i = 0; i < 100; i++) {
            // Wait for a bit
            sleep_in_between_ms(1, 5);

            // Safely get a value and report it
            concore::task t(
                    [&] {
                        printf("%5.2f\n", my_window.get_average());
                        fflush(stdout);
                    },
                    grp);
            concore::execute(ser, std::move(t));
        }
    };

    concore::spawn_and_wait({
            producer_process,
            average_consumer_process,
            average_consumer_process,
            average_consumer_process,
    });

    concore::wait(grp); // ensure the serializer finishes
}

void more_performance_with_rw_serializer() {
    CONCORE_PROFILING_FUNCTION();
    running_window my_window(10); // cannot be shared

    auto grp = concore::task_group::create();
    concore::rw_serializer ser;

    auto producer_process = [&]() {
        CONCORE_PROFILING_SCOPE_N("producer");
        std::random_device providence;
        std::mt19937 crystal_ball{providence()};
        std::uniform_real_distribution<> fortune_teller(1.0, 100.0);

        for (int i = 0; i < 100; i++) {
            CONCORE_PROFILING_SCOPE_N("iter");
            CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", i);
            // Wait for a bit
            sleep_in_between_ms(7, 10);
            // Produce one value
            double val = fortune_teller(crystal_ball);

            // Safely push the value into our average window object
            concore::task t([&, val] { my_window.add(val); }, grp);
            concore::execute(ser.writer(), std::move(t));
        }
    };

    auto average_consumer_process = [&]() {
        CONCORE_PROFILING_SCOPE_N("consumer");

        for (int i = 0; i < 100; i++) {
            // Wait for a bit
            sleep_in_between_ms(1, 5);

            // Safely get a value and report it
            concore::task t(
                    [&] {
                        printf("%5.2f\n", my_window.get_average());
                        fflush(stdout);
                    },
                    grp);
            concore::execute(ser.reader(), std::move(t));
        }
    };

    concore::spawn_and_wait({
            producer_process,
            average_consumer_process,
            average_consumer_process,
            average_consumer_process,
    });

    concore::wait(grp); // ensure the serializer finishes
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    safety_with_serializer();
    printf("---\n");
    more_performance_with_rw_serializer();

    return 0;
}
