#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "partition.h"
#include "quicksort.h"
#include "stack.h"

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

    int64_t movements = 0;
    int64_t comparisons = sort_variant(actual, length, variant, &movements);
    return comparisons >= 0 && movements >= 0 &&
           memcmp(actual, expected, (size_t)length * sizeof *actual) == 0;
}

static int check_large_case(const char *variant, int length, int pattern) {
    int *actual = malloc((size_t)length * sizeof *actual);
    int *expected = malloc((size_t)length * sizeof *expected);
    if (actual == NULL || expected == NULL) {
        free(actual);
        free(expected);
        return 0;
    }
    unsigned int state = 0x12345678U;
    for (int i = 0; i < length; ++i) {
        if (pattern == 0) {
            actual[i] = i;
        } else if (pattern == 1) {
            actual[i] = length - i;
        } else if (pattern == 2) {
            actual[i] = (i * 37) % 101 - 50;
        } else {
            state = state * 1664525U + 1013904223U;
            actual[i] = (int)(state % 2001U) - 1000;
        }
        expected[i] = actual[i];
    }
    qsort(expected, (size_t)length, sizeof *expected, compare_ints);
    int64_t movements = 0;
    int64_t comparisons = sort_variant(actual, length, variant, &movements);
    int passed = comparisons >= 0 && movements >= 0 &&
                 memcmp(actual, expected, (size_t)length * sizeof *actual) == 0;
    free(actual);
    free(expected);
    return passed;
}

static int check_pivot_selection(void) {
    const int permutations[6][3] = {
        {1, 2, 3}, {1, 3, 2}, {2, 1, 3}, {2, 3, 1}, {3, 1, 2}, {3, 2, 1},
    };
    for (int i = 0; i < 6; ++i) {
        if (select_pivot(permutations[i], 0, 2, PIVOT_MEDIAN_OF_THREE) != 2) {
            return 0;
        }
    }
    int values[] = {99, 9, 1, 5, -99};
    return select_pivot(values, 1, 3, PIVOT_FIRST) == 9 &&
           select_pivot(values, 1, 3, PIVOT_MIDDLE) == 1 &&
           select_pivot(values, 1, 3, PIVOT_MEDIAN_OF_THREE) == 5;
}

static int check_partition_boundaries(void) {
    int values[] = {INT_MIN, 9, 1, 5, 3, 7, INT_MAX};
    int left_cursor = 0;
    int right_cursor = 0;
    int64_t movements = 0;
    int comparisons =
        partition_range(1, 5, &left_cursor, &right_cursor, values, PIVOT_MIDDLE, &movements);
    if (comparisons <= 0 || movements < 0 || movements % 3 != 0 || values[0] != INT_MIN ||
        values[6] != INT_MAX || left_cursor <= right_cursor) {
        return 0;
    }
    for (int i = 1; i <= right_cursor; ++i) {
        if (values[i] > 5) {
            return 0;
        }
    }
    for (int i = left_cursor; i <= 5; ++i) {
        if (values[i] < 5) {
            return 0;
        }
    }

    int equal[] = {7, 7, 7};
    movements = 0;
    comparisons =
        partition_range(0, 2, &left_cursor, &right_cursor, equal, PIVOT_MIDDLE, &movements);
    return comparisons == 4 && movements == 6 && left_cursor == 2 && right_cursor == 0;
}

static int check_stack(void) {
    RangeStack stack;
    if (!stack_init(&stack)) {
        return 0;
    }
    SortRange first = {.left = 1, .right = 5};
    SortRange second = {.left = 10, .right = 20};
    int passed = stack_is_empty(&stack) && stack_size(&stack) == 0 && stack_push(&stack, first) &&
                 stack_push(&stack, second) && !stack_is_empty(&stack) && stack_size(&stack) == 2;
    SortRange actual = {.left = -1, .right = -1};
    if (passed) {
        stack_pop(&stack, &actual);
        passed =
            actual.left == second.left && actual.right == second.right && stack_size(&stack) == 1;
        stack_pop(&stack, &actual);
        passed &= actual.left == first.left && actual.right == first.right &&
                  stack_is_empty(&stack) && stack_size(&stack) == 0;
        stack_pop(&stack, &actual);
        passed &= actual.left == first.left && actual.right == first.right;
    }
    stack_destroy(&stack);
    return passed && stack.top == NULL && stack.bottom == NULL && stack.size == 0;
}

int main(void) {
    const char *const variants[] = {"QC", "QM3", "QPE", "QI1", "QI5", "QI10", "QNR"};
    const int extremes[] = {INT_MAX, 0, INT_MIN, 0, -1, INT_MAX, INT_MIN, 1};
    const int cutoff_lengths[] = {9,   10,  11,  19,  20,  21,   99,  100,
                                  101, 199, 200, 201, 999, 1000, 1001};

    if (!check_pivot_selection() || !check_partition_boundaries() || !check_stack()) {
        fputs("Pivot, partition, or stack test failed\n", stderr);
        return 1;
    }

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
                    fprintf(stderr, "%s failed length %d, case %d\n", variants[v], length, mask);
                    return 1;
                }
            }
        }

        for (size_t n = 0; n < sizeof cutoff_lengths / sizeof cutoff_lengths[0]; ++n) {
            for (int pattern = 0; pattern < 4; ++pattern) {
                if (!check_large_case(variants[v], cutoff_lengths[n], pattern)) {
                    fprintf(stderr, "%s failed length %d, pattern %d\n", variants[v],
                            cutoff_lengths[n], pattern);
                    return 1;
                }
            }
        }
    }
    if (!check_large_case("QPE", 10000, 0) || !check_large_case("QPE", 10000, 1)) {
        fputs("First-pivot ordered-input stress failed\n", stderr);
        return 1;
    }
    int64_t movements = 0;
    if (sort_variant(NULL, 1, "QC", &movements) >= 0 ||
        sort_variant(NULL, -1, "QC", &movements) >= 0 ||
        sort_variant(NULL, 0, "unknown", &movements) != 0 ||
        sort_variant(NULL, 1, "unknown", &movements) >= 0) {
        fputs("Invalid sorting arguments were accepted\n", stderr);
        return 1;
    }
    puts("Exhaustive and boundary sorting tests passed");
    return 0;
}
