#include <string.h>

#include "partition.h"
#include "quicksort.h"
#include "stack.h"

static long insertion_sort(int *values, int left, int right, long *movements) {
    long comparisons = 0;
    for (int i = left; i <= right; ++i) {
        int key = values[i];
        int j = i - 1;
        while (j >= 0) {
            ++comparisons;
            if (key >= values[j]) {
                break;
            }
            values[j + 1] = values[j];
            ++*movements;
            --j;
        }
        values[j + 1] = key;
        ++*movements;
    }
    return comparisons;
}

/* Hybrid variants switch each remaining partition to insertion sort at the cutoff. */
static long sort_recursive(int left, int right, int *values, PivotStrategy strategy,
                           long *movements, int insertion_cutoff) {
    long comparisons = 0;
    int i;
    int j;

    if (insertion_cutoff == 0) {
        if (strategy == PIVOT_MEDIAN_OF_THREE && left - right <= 2) {
            strategy = PIVOT_MIDDLE;
        }
        comparisons = partition_range(left, right, &i, &j, values, strategy,
                                      movements);
        if (left < j) {
            comparisons += sort_recursive(left, j, values, strategy, movements,
                                          insertion_cutoff);
        }
        if (i < right) {
            comparisons += sort_recursive(i, right, values, strategy, movements,
                                          insertion_cutoff);
        }
    } else if (right - left > insertion_cutoff) {
        if (strategy == PIVOT_MEDIAN_OF_THREE && right - left <= 2) {
            strategy = PIVOT_MIDDLE;
        }
        comparisons = partition_range(left, right, &i, &j, values, strategy,
                                      movements);
        comparisons += sort_recursive(left, j, values, strategy, movements,
                                      insertion_cutoff);
        comparisons += sort_recursive(i, right, values, strategy, movements,
                                      insertion_cutoff);
    } else {
        comparisons = insertion_sort(values, left, right, movements);
    }
    return comparisons;
}

/* Push the larger side and work on the smaller side before returning to the stack. */
static long sort_iterative(int *values, int length, long *movements) {
    int left = 0;
    int right = length - 1;
    int i;
    int j;
    long comparisons = 0;
    RangeStack stack;
    SortRange range = {.left = left, .right = right};

    if (!stack_init(&stack)) {
        return -1;
    }
    if (!stack_push(&stack, range)) {
        stack_destroy(&stack);
        return -1;
    }
    do {
        if (right > left) {
            comparisons += partition_range(left, right, &i, &j, values,
                                           PIVOT_MIDDLE, movements);
            if (j - left > right - i) {
                range.left = left;
                range.right = j;
                if (!stack_push(&stack, range)) {
                    stack_destroy(&stack);
                    return -1;
                }
                left = i;
            } else {
                range.left = i;
                range.right = right;
                if (!stack_push(&stack, range)) {
                    stack_destroy(&stack);
                    return -1;
                }
                right = j;
            }
        } else {
            stack_pop(&stack, &range);
            left = range.left;
            right = range.right;
        }
    } while (!stack_is_empty(&stack));

    stack_destroy(&stack);
    return comparisons;
}

long sort_variant(int *values, int length, const char *variant, long *movements) {
    if (length <= 0) {
        return 0;
    }
    int last = length - 1;
    if (strcmp(variant, "QC") == 0) {
        return sort_recursive(0, last, values, PIVOT_MIDDLE, movements, 0);
    }
    if (strcmp(variant, "QM3") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements, 0);
    }
    if (strcmp(variant, "QPE") == 0) {
        return sort_recursive(0, last, values, PIVOT_FIRST, movements, 0);
    }
    if (strcmp(variant, "QI1") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements,
                              last / 100);
    }
    if (strcmp(variant, "QI5") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements,
                              last / 20);
    }
    if (strcmp(variant, "QI10") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements,
                              last / 10);
    }
    if (strcmp(variant, "QNR") == 0) {
        return sort_iterative(values, length, movements);
    }
    return 0;
}
