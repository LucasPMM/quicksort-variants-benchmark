#ifndef PARTITION_H
#define PARTITION_H

#include <stdint.h>

typedef enum { PIVOT_MIDDLE, PIVOT_FIRST, PIVOT_MEDIAN_OF_THREE } PivotStrategy;

int select_pivot(const int *values, int left, int right, PivotStrategy strategy);
int partition_range(int left, int right, int *i, int *j, int *values, PivotStrategy strategy,
                    int64_t *movements);

#endif
