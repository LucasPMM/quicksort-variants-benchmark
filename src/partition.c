#include "partition.h"

int select_pivot(const int *values, int left, int right, PivotStrategy strategy) {
    int first = values[left];
    int middle = values[(left + right) / 2];
    int last = values[right];

    if (strategy == PIVOT_MIDDLE) {
        return middle;
    }
    if (strategy == PIVOT_FIRST) {
        return first;
    }

    /* Sorting the three sampled values yields their median without moving array data. */
    if (first > middle) {
        int temporary = first;
        first = middle;
        middle = temporary;
    }
    if (middle > last) {
        int temporary = middle;
        middle = last;
        last = temporary;
    }
    if (first > middle) {
        int temporary = first;
        first = middle;
        middle = temporary;
    }
    return middle;
}

int partition_range(int left, int right, int *i, int *j, int *values, PivotStrategy strategy,
                    int64_t *movements) {
    int comparisons = 0;
    int pivot = select_pivot(values, left, right, strategy);
    *i = left;
    *j = right;

    /* Walk inward until both cursors identify values on the wrong pivot side. */
    do {
        while (pivot > values[*i]) {
            ++comparisons;
            ++*i;
        }
        while (pivot < values[*j]) {
            ++comparisons;
            --*j;
        }
        /* The two terminating element comparisons are part of the archival metric. */
        comparisons += 2;

        if (*i <= *j) {
            int temporary = values[*i];
            values[*i] = values[*j];
            values[*j] = temporary;
            ++*i;
            --*j;
            *movements += 3;
        }
    } while (*i <= *j);

    return comparisons;
}
