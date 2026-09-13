#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <time.h>

#include "benchmark.h"
#include "quicksort.h"

static int elapsed_microseconds(const struct timespec *start,
                                const struct timespec *end) {
    return (int)(1000000.0 * (double)(end->tv_sec - start->tv_sec) +
                 0.001 * (double)(end->tv_nsec - start->tv_nsec));
}

BenchmarkResult measure_sort(int **arrays, int length, const char *variant) {
    BenchmarkResult result = {0};
    int elapsed[BENCHMARK_TRIALS];
    double comparisons = 0.0;
    double movements = 0.0;

    for (int i = 0; i < BENCHMARK_TRIALS; ++i) {
        struct timespec start;
        struct timespec end;
        long trial_movements = 0;
        if (clock_gettime(CLOCK_REALTIME, &start) != 0) {
            return result;
        }
        long trial_comparisons = sort_variant(arrays[i], length, variant,
                                              &trial_movements);
        if (trial_comparisons < 0 || clock_gettime(CLOCK_REALTIME, &end) != 0) {
            return result;
        }

        comparisons += (double)trial_comparisons / BENCHMARK_TRIALS;
        movements += (double)trial_movements / BENCHMARK_TRIALS;
        elapsed[i] = elapsed_microseconds(&start, &end);
    }

    /* The timing sort is outside the measured interval and has its own counter. */
    long ignored_movements = 0;
    if (sort_variant(elapsed, BENCHMARK_TRIALS, "QC", &ignored_movements) < 0) {
        return result;
    }

    result.success = 1;
    result.mean_comparisons = comparisons;
    result.mean_movements = (long)movements;
    result.median_time_us = (elapsed[BENCHMARK_TRIALS / 2 - 1] +
                             elapsed[BENCHMARK_TRIALS / 2]) / 2;
    return result;
}
