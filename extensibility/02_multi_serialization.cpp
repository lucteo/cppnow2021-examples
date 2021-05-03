#include <concore/serializer.hpp>
#include <concore/finish_task.hpp>
#include <concore/spawn.hpp>

#include "../common/utils.hpp"

int counter_a{0};
int counter_b{0};

concore::serializer s_a;
concore::serializer s_b;

void task_1() { // needs protected access to counter_a
    CONCORE_PROFILING_FUNCTION();
    int old = counter_a;

    sleep_in_between_ms(10, 20);

    if (counter_a != old) {
        printf("Correctness issue\n");
        std::terminate();
    }
    counter_a++;
}

void task_2() { // needs protected access to counter_b
    CONCORE_PROFILING_FUNCTION();
    int old = counter_b;

    sleep_in_between_ms(10, 20);

    if (counter_b != old) {
        printf("Correctness issue\n");
        std::terminate();
    }
    counter_b++;
}

void task_3() { // needs protected access to both counter_a and counter_b
    CONCORE_PROFILING_FUNCTION();
    int old_a = counter_a;
    int old_b = counter_b;

    sleep_in_between_ms(10, 20);

    if (counter_a != old_a || counter_b != old_b) {
        printf("Correctness issue\n");
        std::terminate();
    }
    counter_a++;
    counter_b++;
}

void spawn_t1(const concore::task_group& grp) { concore::execute(s_a, concore::task{task_1, grp}); }

void spawn_t2(const concore::task_group& grp) { concore::execute(s_b, concore::task{task_2, grp}); }

void spawn_t3(const concore::task_group& grp) {
    auto wrapper_a = [grp] {
        auto cont = concore::exchange_cur_continuation();
        concore::execute(s_b, concore::task{task_3, grp, cont});
    };
    concore::execute(s_a, concore::task{wrapper_a, grp});
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    auto grp = concore::task_group::create();

    // Spawn tasks of different types
    for (int i = 0; i < 30; i++) {
        switch (i % 3) {
        case 0:
            spawn_t1(grp);
            break;
        case 1:
            spawn_t2(grp);
            break;
        case 2:
            spawn_t3(grp);
            break;
        }
        sleep_for(15ms);
    }

    // Wait for all tasks to complete
    concore::wait(grp);

    // Print the results
    printf("Counter a: %d\n", counter_a);
    printf("Counter b: %d\n", counter_b);

    return 0;
}
