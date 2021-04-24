#include <concore/conc_scan.hpp>
#include <concore/finish_task.hpp>

#include "../common/utils.hpp"

#include <memory>
#include <vector>
#include <string>

struct data_stream {};
struct parsed_body {};
struct request_handling_resources {};
struct response_data {};

struct request_data {
    // Data stream from which we read the request
    data_stream data_stream_;

    // Data resulting from reading the request
    std::string uri_;
    std::vector<std::pair<std::string, std::string>> headers_;
    std::string body_;

    // Parsed body data
    std::shared_ptr<parsed_body> parsed_body_;

    // Result of the authentication check
    bool invalid_auth_{false};

    // The resources needed for handling the request
    std::shared_ptr<request_handling_resources> resources_;

    // The response that need to be send back
    std::shared_ptr<response_data> response_;
};

using request_ptr = std::shared_ptr<request_data>;

request_ptr create_request_data() {
    return std::make_shared<request_data>();
}

void read_http_request(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}
void parse_body(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(30, 50);
}
void authenticate(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(5, 10);
}
void log_start_event(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}
void alloc_resources(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}
void compute_result(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(40, 80);
}
void log_end_event(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}
void update_stats(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}
void send_response(request_ptr req) {
    CONCORE_PROFILING_FUNCTION();
    sleep_in_between_ms(20, 40);
}


int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    init_randomness();

    concore::finish_wait done;

    request_ptr data = std::make_shared<request_data>();
    // create the tasks
    concore::chained_task t1{[data] { read_http_request(data); }};
    concore::chained_task t2{[data] { parse_body(data); }};
    concore::chained_task t3{[data] { authenticate(data); }};
    concore::chained_task t4{[data] { log_start_event(data); }};
    concore::chained_task t5{[data] { alloc_resources(data); }};
    concore::chained_task t6{[data] { compute_result(data); }};
    concore::chained_task t7{[data] { log_end_event(data); }};
    concore::chained_task t8{[data] { update_stats(data); }};
    concore::chained_task t9{[data] { send_response(data); }};
    concore::chained_task t_done{concore::task{[]{}, {}, done.get_continuation()}};

    // set up dependencies
    concore::add_dependencies(t1, {t2, t3});
    concore::add_dependency(t2, t4);
    concore::add_dependencies(t3, {t4, t5});
    concore::add_dependency(t4, t7);
    concore::add_dependencies({t3, t5}, t6);
    concore::add_dependencies(t6, {t7, t8, t9});
    concore::add_dependencies({t7, t8, t9}, {t_done});
    // start the graph
    concore::spawn(t1);

    // Wait until the graph is done executing
    done.wait();

    return 0;
}
