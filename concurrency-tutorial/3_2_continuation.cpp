#include <concore/spawn.hpp>

#include "../common/utils.hpp"

#include <string>
#include <string_view>

void print_message_task(const char* msg) {
    CONCORE_PROFILING_SCOPE();
    CONCORE_PROFILING_SET_TEXT(msg);

    printf(" %s", msg);

    sleep_for(100ms);
}

struct HttpResponse {
    int status_code_;
    std::string status_line_{};
    std::string body_{};
};

using response_callback = std::function<void(HttpResponse)>;

void invoke_callback(response_callback cb, HttpResponse resp) {
    concore::spawn([cb = std::move(cb), resp = std::move(resp)]() mutable {
        CONCORE_PROFILING_SCOPE_N("response callback");
        std::move(cb)(std::move(resp));
    });
}

void make_http_request(std::string url, response_callback cb) {
    CONCORE_PROFILING_FUNCTION();

    concore::spawn([=] {
        CONCORE_PROFILING_SCOPE_N("processing request");

        if (url.find("mutex") != std::string::npos) {
            // Minimal processing
            sleep_for(10ms);
            // Respond with failure
            auto resp = HttpResponse{404, "Not found", "Mutexes are not found in user code."};
            invoke_callback(std::move(cb), std::move(resp));
        } else {
            // Do some processing
            sleep_for(100ms);
            // Respond with success
            auto resp = HttpResponse{200, "OK", "There is a better way do do concurrency."};
            invoke_callback(std::move(cb), std::move(resp));
        }
    });
}

void my_callback(HttpResponse&& resp) {
    CONCORE_PROFILING_FUNCTION();
    printf("Response received: %d %s => %s\n", resp.status_code_, resp.status_line_.c_str(),
            resp.body_.c_str());
}

void do_something_else() {
    CONCORE_PROFILING_FUNCTION();
    for (int i = 0; i < 10; i++) {
        printf(".");
        sleep_for(100us);
    }
    printf("\n");
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    // Make a first request
    auto f1 = [] {
        CONCORE_PROFILING_SCOPE_N("req 1");

        const char* uri = "google.com?q=mutex";
        printf("Making request: %s\n", uri);

        make_http_request(uri, my_callback);
    };
    concore::spawn_and_wait({f1, do_something_else});

    printf("---\n");

    auto f2 = [] {
        CONCORE_PROFILING_SCOPE_N("req 2");

        const char* uri = "google.com?q=concurrency";
        printf("Making request: %s\n", uri);

        make_http_request(uri, my_callback);
    };
    concore::spawn_and_wait({f2, do_something_else});

    printf("\n");
    return 0;
}
