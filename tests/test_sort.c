#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quicksort.h"

static int compare_ints(const void *left, const void *right) {
    int a = *(const int *)left;
    int b = *(const int *)right;
    return (a > b) - (a < b);
}

static int check_case(const char *variant, const int *input, int length) {
    int actual[8] = {0};
    int expected[8] = {0};
    memcpy(actual, input, (size_t)length * sizeof *actual);
    memcpy(expected, input, (size_t)length * sizeof *expected);
    qsort(expected, (size_t)length, sizeof *expected, compare_ints);

    long movements = 0;
    long comparisons = sort_variant(actual, length, variant, &movements);
    return comparisons >= 0 && movements >= 0 &&
           memcmp(actual, expected, (size_t)length * sizeof *actual) == 0;
}

int main(void) {
    const char *const variants[] = {"QC", "QM3", "QPE", "QI1", "QI5", "QI10", "QNR"};
    const int extremes[] = {INT_MAX, 0, INT_MIN, 0, -1, INT_MAX, INT_MIN, 1};

    for (size_t v = 0; v < sizeof variants / sizeof variants[0]; ++v) {
        if (!check_case(variants[v], extremes, 8)) {
            fprintf(stderr, "%s failed the integer-boundary case\n", variants[v]);
            return 1;
        }

        /* Exhaust every length up to seven over {-1, 0, 1}, including duplicates. */
        for (int length = 0; length <= 7; ++length) {
            int combinations = 1;
            for (int i = 0; i < length; ++i) {
                combinations *= 3;
            }
            for (int mask = 0; mask < combinations; ++mask) {
                int input[8] = {0};
                int digits = mask;
                for (int i = 0; i < length; ++i) {
                    input[i] = digits % 3 - 1;
                    digits /= 3;
                }
                if (!check_case(variants[v], input, length)) {
                    fprintf(stderr, "%s failed length %d, case %d\n", variants[v],
                            length, mask);
                    return 1;
                }
            }
        }
    }
    puts("Short-array exhaustive sorting tests passed");
    return 0;
}
