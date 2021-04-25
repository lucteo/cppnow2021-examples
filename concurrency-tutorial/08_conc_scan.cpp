#include <concore/conc_scan.hpp>

#include "../common/utils.hpp"

std::vector<int> prefix_sum(const std::vector<int>& vals) {
    CONCORE_PROFILING_FUNCTION();
    std::vector<int> res;
    res.resize(vals.size());

    auto op = [](int id, int i) -> int {
        CONCORE_PROFILING_SCOPE_N("op");
        CONCORE_PROFILING_SET_TEXT_FMT(32, "%d+%d", id, i);
        sleep_for(50ms); // pretend this is a heavy operation
        return id + i;
    };
    concore::conc_scan(vals.begin(), vals.end(), res.begin(), 0, op);

    return res;
}

std::vector<int> create_consecutive_seq(int count, int start = 1, int step = 1) {
    std::vector<int> res;
    res.reserve(count);
    for (int i = 0; i < count; i++)
        res.push_back(start + i*step);
    return res;
}

void print_vec(const std::vector<int>& v) {
    CONCORE_PROFILING_FUNCTION();
    for (int i = 0; i < int(v.size()); i++) {
        if (i > 0)
            printf(", ");
        printf("%d", v[i]);
    }
    printf("\n");
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    print_vec(prefix_sum(create_consecutive_seq(100)));
    printf("---\n");
    print_vec(prefix_sum(create_consecutive_seq(100, 2, 2)));

    return 0;
}
