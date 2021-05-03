#include <concore/spawn.hpp>
#include <concore/execution.hpp>
#include <concore/data/concurrent_queue.hpp>

#include "../common/utils.hpp"

class prio_serializer {
public:
    explicit prio_serializer(int num_prios);
    ~prio_serializer();

    void add_task(int prio, concore::task&& t);

private:
    using task = concore::task;
    //! Type representing a queue of tasks, all having the same priority
    using task_queue =
            concore::concurrent_queue<concore::task, concore::queue_type::multi_prod_single_cons>;
    //! The number of priorities
    int num_prios_{0};
    //! The tasks arranged by priority
    task_queue* tasks_per_prio_{nullptr};
    //! Number of tasks that we have in our queues
    std::atomic<int> count_{0};

    void set_continuation(task& t);
    void on_cont(std::exception_ptr);

    void start_next_task();
};

prio_serializer::prio_serializer(int num_prios)
    : num_prios_(num_prios) {
    tasks_per_prio_ = new task_queue[num_prios];
}
prio_serializer::~prio_serializer() { delete[] tasks_per_prio_; }

void prio_serializer::add_task(int prio, task&& t) {
    CONCORE_PROFILING_FUNCTION();
    // Set the right continuation to the given task
    set_continuation(t);

    // Add the task to the queue, with the right continuation
    tasks_per_prio_[prio].push(std::forward<task>(t));

    // If there were no other tasks, enqueue a task in the base executor
    if (count_++ == 0)
        start_next_task();
}

void prio_serializer::set_continuation(task& t) {
    auto inner_cont = t.get_continuation();
    concore::task_continuation_function cont;
    if (inner_cont) {
        // We need to call two continuation functions:
        //  a) one that was present in the task
        //  b) one that makes our serializer work
        cont = [this, inner_cont](std::exception_ptr ex) {
            inner_cont(ex);
            this->on_cont(std::move(ex));
        };
    } else {
        cont = [this](std::exception_ptr ex) { this->on_cont(std::move(ex)); };
    }
    t.set_continuation(std::move(cont));
}

void prio_serializer::on_cont(std::exception_ptr) {
    CONCORE_PROFILING_FUNCTION();
    // task exceptions are not reported through except_fun_
    if (count_-- > 1)
        start_next_task();
}

void prio_serializer::start_next_task() {
    CONCORE_PROFILING_FUNCTION();
    // We know we have at least one task (counter was greater than 0)
    // Iterate until we find one task to execute
    while (true) {
        for (int i = 0; i < num_prios_; i++) {
            task to_execute;
            if (tasks_per_prio_[i].try_pop(to_execute)) {
                // Found and extracted the task -- spawn it
                concore::spawn(std::move(to_execute), false);
                return;
            }
        }
    }
}

//! Something that implements the 'executor' concept
struct prio_serializer_executor {
    prio_serializer& ser_;
    int prio_;

    template <typename F>
    void execute(F&& f) const {
        ser_.add_task(prio_, concore::task{std::forward<F>(f)});
    }
    void execute(concore::task t) const { ser_.add_task(prio_, std::move(t)); }
};

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    static_assert(concore::executor<prio_serializer_executor>, "Invalid executor type");

    prio_serializer ser{5};

    auto grp = concore::task_group::create();

    // Spawn tasks at different priorities
    for (int i = 0; i < 100; i++) {
        int prio = (i + 1) % 5;

        // The task to be run
        auto f = [i, prio] {
            CONCORE_PROFILING_SCOPE();
            switch (prio) {
            case 0:
                CONCORE_PROFILING_SET_DYNNAME("P0 task");
                break;
            case 1:
                CONCORE_PROFILING_SET_DYNNAME("P1 task");
                break;
            case 2:
                CONCORE_PROFILING_SET_DYNNAME("P2 task");
                break;
            case 3:
                CONCORE_PROFILING_SET_DYNNAME("P3 task");
                break;
            case 4:
                CONCORE_PROFILING_SET_DYNNAME("P4 task");
                break;
            }
            CONCORE_PROFILING_SET_TEXT_FMT(32, "idx: %d", i);

            sleep_for(10ms);
        };
        concore::task t{std::move(f), grp};

        // Spawn the task using our executor
        prio_serializer_executor ex{ser, prio};
        concore::execute(ex, std::move(t));
    }

    // Wait for all tasks to complete
    concore::wait(grp);

    return 0;
}
