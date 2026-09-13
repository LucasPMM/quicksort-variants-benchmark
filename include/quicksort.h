#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <stdint.h>

/* The length is an element count; every variant sorts values[0..length-1]. */
int64_t sort_variant(int *values, int length, const char *variant, int64_t *movements);

#endif
