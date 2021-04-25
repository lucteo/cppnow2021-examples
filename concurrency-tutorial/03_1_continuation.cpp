#include <concore/spawn.hpp>

#include "../common/utils.hpp"

#include <string>

struct person {
    std::string name_;

    explicit person(const char* name, bool busy)
        : name_(name) {}

    void sms(const char* from, const char* message, std::function<void()> callback) {
        CONCORE_PROFILING_FUNCTION();
        printf("%s is being texted by %s (message: %s)\n", name_.c_str(), from, message);
        printf("    continuing current work...\n");

        // Continue working, and then call back
        concore::spawn([=] {
            CONCORE_PROFILING_SCOPE_N("continue working");
            // Finish current work first
            sleep_for(100ms);

            printf("%s is now ready to respond to %s\n", name_.c_str(), from);
            // Now call back
            concore::spawn(callback);
        });
    }
};

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    person p1("Alice", true);

    // Try contacting
    auto callback = []() {
        CONCORE_PROFILING_SCOPE_N("callback");
        printf("\nCalling back\n");
        sleep_for(10ms);
    };
    p1.sms("Bob", "Hey, are you free?", callback);

    // Do something else while waiting for the message
    {
        CONCORE_PROFILING_SCOPE_N("doing something else");
        for (int i = 0; i < 10; i++) {
            printf(".");
            sleep_for(15ms);
        }
    }

    printf("\n");
    return 0;
}
