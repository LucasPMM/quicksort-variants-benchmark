#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "input.h"

static int check_median(void) {
    uint64_t odd[] = {9, 1, 5, 3, 7};
    uint64_t even[] = {4, 1, 2, 3};
    uint64_t extremes[] = {UINT64_MAX, UINT64_MAX - 1};
    uint64_t zeros[BENCHMARK_TRIALS] = {0};
    uint64_t copy[5];
    uint64_t result = 0;
    memcpy(copy, odd, sizeof odd);
    if (!benchmark_median_us(odd, 5, &result) || result != 5 ||
        memcmp(copy, odd, sizeof odd) != 0) {
        return 0;
    }
    if (!benchmark_median_us(even, 4, &result) || result != 2 ||
        !benchmark_median_us(extremes, 2, &result) ||
        result != UINT64_MAX - 1 ||
        !benchmark_median_us(zeros, BENCHMARK_TRIALS, &result) || result != 0 ||
        benchmark_median_us(odd, 0, &result) ||
        benchmark_median_us(odd, BENCHMARK_TRIALS + 1, &result) ||
        benchmark_median_us(NULL, 1, &result) ||
        benchmark_median_us(odd, 1, NULL)) {
        return 0;
    }
    return 1;
}

static int check_inputs(void) {
    int **ascending = create_input_arrays(INPUT_ASCENDING, 10, 2);
    int **descending = create_input_arrays(INPUT_DESCENDING, 10, 2);
    int **copy = copy_input_arrays(ascending, 10, 2);
    int passed = ascending != NULL && descending != NULL && copy != NULL;
    if (passed) {
        for (int row = 0; row < 2; ++row) {
            for (int i = 0; i < 10; ++i) {
                passed &= ascending[row][i] == i + 1;
                passed &= descending[row][i] == 10 - i;
            }
        }
        ascending[0][0] = 99;
        passed &= copy[0][0] == 1;
    }
    free_input_arrays(ascending, 2);
    free_input_arrays(descending, 2);
    free_input_arrays(copy, 2);

    srand(42);
    int **first = create_input_arrays(INPUT_RANDOM, 20, 2);
    srand(42);
    int **second = create_input_arrays(INPUT_RANDOM, 20, 2);
    passed &= first != NULL && second != NULL;
    if (first != NULL && second != NULL) {
        for (int row = 0; row < 2; ++row) {
            for (int i = 0; i < 20; ++i) {
                int value = first[row][i];
                passed &= value >= 1 && value <= 191 && (value - 1) % 10 == 0;
                passed &= value == second[row][i];
            }
        }
    }
    free_input_arrays(first, 2);
    free_input_arrays(second, 2);
    passed &= create_input_arrays(INPUT_ASCENDING, 0, 1) == NULL;
    passed &= create_input_arrays(INPUT_ASCENDING, BENCHMARK_MAX_LENGTH + 1, 1) == NULL;
    passed &= create_input_arrays(INPUT_ASCENDING, 10, 0) == NULL;
    passed &= create_input_arrays(INPUT_ASCENDING, 10, BENCHMARK_TRIALS + 1) == NULL;
    passed &= create_input_arrays((InputOrder)99, 10, 1) == NULL;
    passed &= copy_input_arrays(NULL, 10, 1) == NULL;
    free_input_arrays(NULL, 0);
    return passed;
}

static int check_measurement(void) {
    int **arrays = create_input_arrays(INPUT_ASCENDING, 2, BENCHMARK_TRIALS);
    if (arrays == NULL) {
        return 0;
    }
    for (int trial = BENCHMARK_TRIALS / 2; trial < BENCHMARK_TRIALS; ++trial) {
        arrays[trial][0] = 2;
        arrays[trial][1] = 1;
    }
    BenchmarkResult result = measure_sort(arrays, 2, "QC");
    int passed = result.success && result.mean_comparisons == 2.5 &&
                 result.mean_movements == 3;
    for (int trial = 0; trial < BENCHMARK_TRIALS; ++trial) {
        for (int i = 0; i < 2; ++i) {
            passed &= arrays[trial][i] == i + 1;
        }
    }
    passed &= !measure_sort(arrays, 0, "QC").success;
    passed &= !measure_sort(arrays, BENCHMARK_MAX_LENGTH + 1, "QC").success;
    passed &= !measure_sort(arrays, 2, NULL).success;
    passed &= !measure_sort(arrays, 2, "unknown").success;
    int *saved = arrays[0];
    arrays[0] = NULL;
    passed &= !measure_sort(arrays, 2, "QC").success;
    arrays[0] = saved;
    free_input_arrays(arrays, BENCHMARK_TRIALS);
    passed &= !measure_sort(NULL, 10, "QC").success;
    return passed;
}

int main(void) {
    if (!check_median() || !check_inputs() || !check_measurement()) {
        fputs("Benchmark/input tests failed\n", stderr);
        return 1;
    }
    puts("Benchmark and input tests passed");
    return 0;
}
