#include <concore/pipeline.hpp>
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
    sleep_in_between_ms(20, 40);
    printf("%d: parse\n", frm.frame_idx_);
    assert(frm.stage_ == 0);
    frm.stage_++;
}

void preprocess_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_in_between_ms(20, 40);
    printf("    %d: preprocess\n", frm.frame_idx_);
    assert(frm.stage_ == 1);
    frm.stage_++;
}

void decode_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_in_between_ms(200, 300);
    printf("    %d: decode\n", frm.frame_idx_);
    assert(frm.stage_ == 2);
    frm.stage_++;
}
void postprocess_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_in_between_ms(5, 15);
    printf("    %d: postprocess\n", frm.frame_idx_);
    assert(frm.stage_ == 3);
    frm.stage_++;
}
void write_frame(frame_data& frm) {
    CONCORE_PROFILING_FUNCTION();
    CONCORE_PROFILING_SET_TEXT_FMT(32, "%d", frm.frame_idx_);
    sleep_in_between_ms(20, 40);
    printf("%d: write\n", frm.frame_idx_);
    assert(frm.stage_ == 4);
    frm.stage_++;
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    static constexpr int max_concurrency = 20;

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

    return 0;
}
