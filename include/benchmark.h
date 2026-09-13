#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

#define BENCHMARK_TRIALS 20
#define BENCHMARK_MAX_LENGTH 500000

typedef struct {
    int success;
    double mean_comparisons;
    int64_t mean_movements;
    uint64_t median_time_us;
} BenchmarkResult;

int benchmark_median_us(const uint64_t *samples, int count, uint64_t *median);
BenchmarkResult measure_sort(int **arrays, int length, const char *variant);

#endif
