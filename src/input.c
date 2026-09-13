#include <stdlib.h>
#include <string.h>

#include "benchmark.h"
#include "input.h"

static int valid_shape(int length, int count) {
    return length > 0 && length <= BENCHMARK_MAX_LENGTH &&
           count > 0 && count <= BENCHMARK_TRIALS;
}

void free_input_arrays(int **arrays, int count) {
    if (arrays == NULL) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        free(arrays[i]);
    }
    free(arrays);
}

int **create_input_arrays(InputOrder order, int length, int count) {
    if (!valid_shape(length, count) || order < INPUT_ASCENDING || order > INPUT_RANDOM) {
        return NULL;
    }
    int **arrays = calloc((size_t)count, sizeof *arrays);
    if (arrays == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; ++i) {
        arrays[i] = calloc((size_t)length, sizeof *arrays[i]);
        if (arrays[i] == NULL) {
            free_input_arrays(arrays, count);
            return NULL;
        }
        for (int j = 0; j < length; ++j) {
            if (order == INPUT_ASCENDING) {
                arrays[i][j] = j + 1;
            } else if (order == INPUT_DESCENDING) {
                arrays[i][j] = length - j;
            } else {
                /* Keep the archival distribution until the measurement policy is revised. */
                arrays[i][j] = (rand() % length * 10) + 1;
            }
        }
    }
    return arrays;
}

int **copy_input_arrays(int *const *arrays, int length, int count) {
    if (arrays == NULL || !valid_shape(length, count)) {
        return NULL;
    }
    int **copy = calloc((size_t)count, sizeof *copy);
    if (copy == NULL) {
        return NULL;
    }
    for (int i = 0; i < count; ++i) {
        if (arrays[i] == NULL) {
            free_input_arrays(copy, count);
            return NULL;
        }
        copy[i] = calloc((size_t)length, sizeof *copy[i]);
        if (copy[i] == NULL) {
            free_input_arrays(copy, count);
            return NULL;
        }
        memcpy(copy[i], arrays[i], (size_t)length * sizeof *copy[i]);
    }
    return copy;
}
