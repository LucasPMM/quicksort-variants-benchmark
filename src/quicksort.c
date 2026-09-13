#include <string.h>

#include "partition.h"
#include "quicksort.h"
#include "stack.h"

static int64_t insertion_sort(int *values, int left, int right, int64_t *movements) {
    int64_t comparisons = 0;
    for (int i = left; i <= right; ++i) {
        int key = values[i];
        int j = i - 1;
        while (j >= left) {
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

/* Recurse on the smaller partition to bound call-stack depth even for QPE. */
static int64_t sort_recursive(int left, int right, int *values, PivotStrategy strategy,
                              int64_t *movements, int insertion_cutoff) {
    int64_t comparisons = 0;
    while (left < right) {
        if (insertion_cutoff > 0 && right - left + 1 <= insertion_cutoff) {
            comparisons += insertion_sort(values, left, right, movements);
            break;
        }

        int i;
        int j;
        comparisons += partition_range(left, right, &i, &j, values, strategy, movements);
        int left_length = j >= left ? j - left + 1 : 0;
        int right_length = i <= right ? right - i + 1 : 0;

        if (left_length < right_length) {
            if (left_length > 1) {
                comparisons +=
                    sort_recursive(left, j, values, strategy, movements, insertion_cutoff);
            }
            left = i;
        } else {
            if (right_length > 1) {
                comparisons +=
                    sort_recursive(i, right, values, strategy, movements, insertion_cutoff);
            }
            right = j;
        }
    }
    return comparisons;
}

/* Push the larger side and work on the smaller side before returning to the stack. */
static int64_t sort_iterative(int *values, int length, int64_t *movements) {
    int left = 0;
    int right = length - 1;
    int64_t comparisons = 0;
    RangeStack stack;

    if (!stack_init(&stack)) {
        return -1;
    }

    for (;;) {
        while (left < right) {
            int i;
            int j;
            comparisons += partition_range(left, right, &i, &j, values, PIVOT_MIDDLE, movements);
            int left_length = j >= left ? j - left + 1 : 0;
            int right_length = i <= right ? right - i + 1 : 0;
            SortRange deferred;

            if (left_length < right_length) {
                deferred = (SortRange){.left = i, .right = right};
                if (right_length > 1 && !stack_push(&stack, deferred)) {
                    stack_destroy(&stack);
                    return -1;
                }
                right = j;
            } else {
                deferred = (SortRange){.left = left, .right = j};
                if (left_length > 1 && !stack_push(&stack, deferred)) {
                    stack_destroy(&stack);
                    return -1;
                }
                left = i;
            }
        }
        if (stack_is_empty(&stack)) {
            break;
        }
        SortRange next;
        stack_pop(&stack, &next);
        left = next.left;
        right = next.right;
    }

    stack_destroy(&stack);
    return comparisons;
}

int64_t sort_variant(int *values, int length, const char *variant, int64_t *movements) {
    if (length < 0 || variant == NULL || movements == NULL || (length > 0 && values == NULL)) {
        return -1;
    }
    if (length == 0) {
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
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements, length / 100);
    }
    if (strcmp(variant, "QI5") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements, length / 20);
    }
    if (strcmp(variant, "QI10") == 0) {
        return sort_recursive(0, last, values, PIVOT_MEDIAN_OF_THREE, movements, length / 10);
    }
    if (strcmp(variant, "QNR") == 0) {
        return sort_iterative(values, length, movements);
    }
    return -1;
}
