#ifndef BENCHMARK_H
#define BENCHMARK_H

#define BENCHMARK_TRIALS 20

typedef struct {
    int success;
    double mean_comparisons;
    long mean_movements;
    int median_time_us;
} BenchmarkResult;

BenchmarkResult measure_sort(int **arrays, int length, const char *variant);

#endif
