#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "benchmark.h"
#include "quicksort.h"

static int elapsed_microseconds(const struct timespec *start, const struct timespec *end,
                                uint64_t *elapsed) {
    if (end->tv_sec < start->tv_sec ||
        (end->tv_sec == start->tv_sec && end->tv_nsec < start->tv_nsec)) {
        return 0;
    }

    time_t seconds = end->tv_sec - start->tv_sec;
    long nanoseconds = end->tv_nsec - start->tv_nsec;
    if (nanoseconds < 0) {
        --seconds;
        nanoseconds += 1000000000L;
    }
    if (seconds < 0 || (uint64_t)seconds > UINT64_MAX / 1000000U) {
        return 0;
    }
    *elapsed = (uint64_t)seconds * 1000000U + (uint64_t)nanoseconds / 1000U;
    return 1;
}

static int compare_durations(const void *left, const void *right) {
    uint64_t a = *(const uint64_t *)left;
    uint64_t b = *(const uint64_t *)right;
    return (a > b) - (a < b);
}

int benchmark_median_us(const uint64_t *samples, int count, uint64_t *median) {
    if (samples == NULL || median == NULL || count < 1 || count > BENCHMARK_TRIALS) {
        return 0;
    }
    uint64_t sorted[BENCHMARK_TRIALS];
    memcpy(sorted, samples, (size_t)count * sizeof *sorted);
    qsort(sorted, (size_t)count, sizeof *sorted, compare_durations);

    if (count % 2 != 0) {
        *median = sorted[count / 2];
    } else {
        uint64_t lower = sorted[count / 2 - 1];
        uint64_t upper = sorted[count / 2];
        *median = lower + (upper - lower) / 2U;
    }
    return 1;
}

BenchmarkResult measure_sort(int **arrays, int length, const char *variant) {
    BenchmarkResult result = {0};
    if (arrays == NULL || length < 1 || length > BENCHMARK_MAX_LENGTH || variant == NULL) {
        return result;
    }
    uint64_t elapsed[BENCHMARK_TRIALS];
    int64_t total_comparisons = 0;
    int64_t total_movements = 0;

    for (int i = 0; i < BENCHMARK_TRIALS; ++i) {
        if (arrays[i] == NULL) {
            return result;
        }
        struct timespec start;
        struct timespec end;
        int64_t trial_movements = 0;
        if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
            return result;
        }
        int64_t trial_comparisons = sort_variant(arrays[i], length, variant, &trial_movements);
        if (trial_comparisons < 0 || clock_gettime(CLOCK_MONOTONIC, &end) != 0 ||
            !elapsed_microseconds(&start, &end, &elapsed[i])) {
            return result;
        }

        total_comparisons += trial_comparisons;
        total_movements += trial_movements;
    }

    if (!benchmark_median_us(elapsed, BENCHMARK_TRIALS, &result.median_time_us)) {
        return result;
    }
    result.success = 1;
    result.mean_comparisons = (double)total_comparisons / BENCHMARK_TRIALS;
    result.mean_movements = total_movements / BENCHMARK_TRIALS;
    return result;
}
