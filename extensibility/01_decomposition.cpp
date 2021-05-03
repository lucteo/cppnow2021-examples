#include <concore/pipeline.hpp>
#include <concore/finish_task.hpp>
#include <concore/spawn.hpp>

#include "../common/utils.hpp"

#include <memory>
#include <vector>
#include <string>

struct frame_data {
    int frame_idx_{0};
    int stage_{0};
    //...

    explicit frame_data(int idx)
        : frame_idx_(idx) {}
};

using request_ptr = std::shared_ptr<frame_data>;

void parse_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_for(10ms);
}

void preprocess_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_for(10ms);
}

void decode_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    // Complex process
    sleep_for(50ms);
    sleep_for(50ms);
    sleep_for(50ms);
    sleep_for(50ms);
}
void postprocess_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_for(10ms);
}
void write_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_for(10ms);
}

static constexpr int max_concurrency = 12;

void test_pipeline() {
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    // Construct the pipeline
    auto my_pipeline =                                                  //
            concore::pipeline_builder<frame_data>(max_concurrency, grp) //
            | concore::stage_ordering::in_order                         //
            | parse_frame                                               //
            | concore::stage_ordering::concurrent                       //
            | preprocess_frame                                          //
            | decode_frame                                              //
            | concore::stage_ordering::out_of_order                     //
            | postprocess_frame                                         //
            | concore::stage_ordering::in_order                         //
            | write_frame                                               //
            | concore::pipeline_end;

    // Push items through the pipeline
    for (int i = 0; i < 40; i++)
        my_pipeline.push(frame_data{i});

    // Wait until we've finished everything
    concore::wait(grp);
}

void decode_frame2(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);

    // Create a join task, and exchange continuation
    auto final_cont = concore::exchange_cur_continuation();
    concore::finish_task done_task(concore::task{[]{}, {}, final_cont});

    // Break the complex process into multiple tasks
    auto f1 = []{ CONCORE_PROFILING_SCOPE_N("task 1"); sleep_for(50ms); };
    auto f2 = []{ CONCORE_PROFILING_SCOPE_N("task 2"); sleep_for(50ms); };
    auto f3 = []{ CONCORE_PROFILING_SCOPE_N("task 3"); sleep_for(50ms); };
    auto f4 = []{ CONCORE_PROFILING_SCOPE_N("task 4"); sleep_for(50ms); };

    // Spawn these tasks
    auto cont = done_task.get_continuation();
    concore::spawn(concore::task{f1, {}, cont});
    concore::spawn(concore::task{f2, {}, cont});
    concore::spawn(concore::task{f3, {}, cont});
    concore::spawn(concore::task{f4, {}, cont});

    // When all the tasks are done, 'done_task' will be called
    // This will call the continuation of the current task
}


// Continuation basics:
//  - a task has 2 functors associated with it:
//      - work functor: the functor that does the work of the task
//      - continuation functor: called to advance he concurrency abstraction
// - in a regular task: a continuation functor is always called after the work
// - typically, user work is placed in the work functor
// - continuations are utilized by the abstractions to make progress
// - as long as continuations are called at some point, the concurrent abstractions work

void test_pipeline2() {
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    // Construct the pipeline
    auto my_pipeline =                                                  //
            concore::pipeline_builder<frame_data>(max_concurrency, grp) //
            | concore::stage_ordering::in_order                         //
            | parse_frame                                               //
            | concore::stage_ordering::concurrent                       //
            | preprocess_frame                                          //
            | decode_frame2                                             //
            | concore::stage_ordering::out_of_order                     //
            | postprocess_frame                                         //
            | concore::stage_ordering::in_order                         //
            | write_frame                                               //
            | concore::pipeline_end;

    // Push items through the pipeline
    for (int i = 0; i < 40; i++)
        my_pipeline.push(frame_data{i});

    // Wait until we've finished everything
    concore::wait(grp);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    test_pipeline();
    test_pipeline2();

    return 0;
}
