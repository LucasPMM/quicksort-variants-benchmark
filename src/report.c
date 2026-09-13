#include <stdio.h>

#include "report.h"

void print_result(const char *variant, const char *order, int length,
                  const BenchmarkResult *result) {
    printf("%s %s %d %.0f %ld %d\n", variant, order, length,
           result->mean_comparisons, result->mean_movements, result->median_time_us);
}

void print_array(const int *values, int length) {
    for (int i = 0; i < length; ++i) {
        printf("%d ", values[i]);
    }
    putchar('\n');
}
