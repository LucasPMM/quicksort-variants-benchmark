#include <inttypes.h>
#include <stdio.h>

#include "report.h"

int print_result(const char *variant, const char *order, int length,
                 const BenchmarkResult *result) {
    return printf("%s %s %d %.0f %" PRId64 " %" PRIu64 "\n", variant, order, length,
                  result->mean_comparisons, result->mean_movements,
                  result->median_time_us) >= 0;
}

int print_array(const int *values, int length) {
    for (int i = 0; i < length; ++i) {
        if (printf("%d ", values[i]) < 0) {
            return 0;
        }
    }
    return putchar('\n') != EOF;
}
