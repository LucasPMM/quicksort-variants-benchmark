#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "benchmark.h"
#include "input.h"
#include "quicksort.h"

/* Linker wrappers fail one allocation at a time without changing production code. */
void *__real_malloc(size_t size);
void *__real_calloc(size_t count, size_t size);
int __real_clock_gettime(clockid_t clock, struct timespec *result);

static int malloc_calls;
static int calloc_calls;
static int malloc_failure_at;
static int calloc_failure_at;
static int clock_calls;
static int clock_failure_at;
static int fake_clock;
static int backwards_clock;
static int wide_clock;
static int wrong_clock;

void *__wrap_malloc(size_t size) {
    ++malloc_calls;
    if (malloc_calls == malloc_failure_at) {
        return NULL;
    }
    return __real_malloc(size);
}

void *__wrap_calloc(size_t count, size_t size) {
    ++calloc_calls;
    if (calloc_calls == calloc_failure_at) {
        return NULL;
    }
    return __real_calloc(count, size);
}

int __wrap_clock_gettime(clockid_t clock, struct timespec *result) {
    ++clock_calls;
    if (clock != CLOCK_MONOTONIC) {
        wrong_clock = 1;
    }
    if (clock_calls == clock_failure_at) {
        return -1;
    }
    if (!fake_clock) {
        return __real_clock_gettime(clock, result);
    }
    if (wide_clock) {
        *result = (struct timespec){.tv_sec = clock_calls % 2 == 0 ? 3000 : 0, .tv_nsec = 0};
    } else if (clock_calls % 2 != 0) {
        *result = (struct timespec){.tv_sec = 10, .tv_nsec = 999999000};
    } else if (backwards_clock) {
        *result = (struct timespec){.tv_sec = 9, .tv_nsec = 0};
    } else {
        *result = (struct timespec){.tv_sec = 11, .tv_nsec = 1000};
    }
    return 0;
}

static int check_input_allocation_failures(void) {
    for (int failure = 1; failure <= 3; ++failure) {
        calloc_calls = 0;
        calloc_failure_at = failure;
        int **arrays = create_input_arrays(INPUT_ASCENDING, 3, 2);
        if (arrays != NULL || calloc_calls != failure) {
            free_input_arrays(arrays, 2);
            return 0;
        }
    }
    calloc_failure_at = 0;
    int **source = create_input_arrays(INPUT_DESCENDING, 3, 2);
    if (source == NULL) {
        return 0;
    }
    for (int failure = 1; failure <= 3; ++failure) {
        calloc_calls = 0;
        calloc_failure_at = failure;
        int **copy = copy_input_arrays(source, 3, 2);
        if (copy != NULL || calloc_calls != failure) {
            free_input_arrays(copy, 2);
            free_input_arrays(source, 2);
            return 0;
        }
    }
    calloc_failure_at = 0;
    free_input_arrays(source, 2);
    return 1;
}

static int check_stack_allocation_failures(void) {
    int64_t movements = 0;
    int values[] = {8, 7, 6, 5, 4, 3, 2, 1};
    malloc_calls = 0;
    malloc_failure_at = 1;
    if (sort_variant(values, 8, "QNR", &movements) != -1 || malloc_calls != 1) {
        return 0;
    }
    malloc_calls = 0;
    malloc_failure_at = 2;
    if (sort_variant(values, 8, "QNR", &movements) != -1 || malloc_calls != 2) {
        return 0;
    }
    malloc_failure_at = 0;
    return 1;
}

static int check_clock_failures(void) {
    int **arrays = create_input_arrays(INPUT_ASCENDING, 2, BENCHMARK_TRIALS);
    if (arrays == NULL) {
        return 0;
    }
    fake_clock = 1;
    clock_calls = 0;
    BenchmarkResult result = measure_sort(arrays, 2, "QC");
    int passed = result.success && result.median_time_us == 2 &&
                 clock_calls == 2 * BENCHMARK_TRIALS && !wrong_clock;

    clock_calls = 0;
    wide_clock = 1;
    result = measure_sort(arrays, 2, "QC");
    passed &= result.success && result.median_time_us == UINT64_C(3000000000) &&
              clock_calls == 2 * BENCHMARK_TRIALS;
    wide_clock = 0;

    clock_calls = 0;
    clock_failure_at = 1;
    passed &= !measure_sort(arrays, 2, "QC").success && clock_calls == 1;
    clock_calls = 0;
    clock_failure_at = 2 * BENCHMARK_TRIALS;
    passed &= !measure_sort(arrays, 2, "QC").success && clock_calls == 2 * BENCHMARK_TRIALS;

    clock_calls = 0;
    clock_failure_at = 0;
    backwards_clock = 1;
    passed &= !measure_sort(arrays, 2, "QC").success && clock_calls == 2;
    fake_clock = 0;
    backwards_clock = 0;
    free_input_arrays(arrays, BENCHMARK_TRIALS);
    return passed;
}

int main(void) {
    if (!check_input_allocation_failures() || !check_stack_allocation_failures() ||
        !check_clock_failures()) {
        fputs("Fault-injection tests failed\n", stderr);
        return 1;
    }
    puts("Fault-injection tests passed");
    return 0;
}
