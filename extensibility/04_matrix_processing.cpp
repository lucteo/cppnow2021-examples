#include <concore/serializer.hpp>
#include <concore/finish_task.hpp>
#include <concore/spawn.hpp>

#include "../common/utils.hpp"

/*
    d d d d . .
    d d X . . .
    . . . . . .
    . . . . . .
    . . . . . .

*/

struct atomic_int : std::atomic<int> {
    atomic_int(int val = 0)
        : atomic(val) {}
    atomic_int(const atomic_int& other)
        : atomic(other.load()) {}
};

struct process_matrix {
    using cell_fun_t = std::function<void(int x, int y)>;

    void start(int w, int h, cell_fun_t cf, concore::task&& donet) {
        width_ = w;
        height_ = h;
        cell_fun_ = cf;
        done_task_ = std::move(donet);
        ref_counts_.resize(h * w);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++)
                ref_counts_[y * w + x].store(x == 0 || y == 0 || x == w - 1 ? 1 : 2);
        }

        // Start with the first cell
        concore::spawn(create_cell_task(0, 0));
    }

private:
    int width_{0};
    int height_{0};
    cell_fun_t cell_fun_;
    concore::task done_task_;
    std::vector<atomic_int> ref_counts_;

    concore::task create_cell_task(int x, int y) {
        auto f = [this, x, y] { cell_fun_(x, y); };
        auto cont = [this, x, y](std::exception_ptr) {
            // Spawn bottom task
            if (y < height_ - 1 && x > 0)
                unblock_cell(x - 1, y + 1);
            // Spawn right task
            if (x < width_ - 1)
                unblock_cell(x + 1, y, false);
            // Finish?
            if (y == height_ - 1 && x == width_ - 1)
                concore::spawn(std::move(done_task_), false);
        };
        return concore::task{f, {}, cont};
    }
    void unblock_cell(int x, int y, bool wake_workers = true) {
        int idx = y * width_ + x;
        if (ref_counts_[idx]-- == 1)
            concore::spawn(create_cell_task(x, y), wake_workers);
    }
};

struct my_matrix {
    int width_{0};
    int height_{0};
    std::vector<int> done_blocks_;

    my_matrix(int w, int h)
        : width_(w)
        , height_(h) {
        done_blocks_.resize(w * h, 0);
    }

    void set_done(int x, int y) { done_blocks_[y * width_ + x] = 1; }

    void print() {
        for (int y = 0; y < height_; y++) {
            for (int x = 0; x < width_; x++) {
                if (done_blocks_[y * width_ + x])
                    printf("* ");
                else
                    printf(". ");
            }
            printf("\n");
        }
        printf("\n");
        fflush(stdout);
    }
};

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();
    concore::task done_task{[] {}, grp};

    my_matrix mat{10, 10};
    auto cell_fun = [&mat](int x, int y) {
        CONCORE_PROFILING_SCOPE_N("cell");
        mat.set_done(x, y);
        sleep_for(100ms);
    };

    // Start processing the matrix
    process_matrix proc;
    proc.start(10, 10, std::move(cell_fun), std::move(done_task));

    // Periodically print the matrix
    for (int i = 0; i < 30; i++) {
        mat.print();
        sleep_for(100ms);
    }

    // Wait for all tasks to complete
    concore::wait(grp);

    return 0;
}
