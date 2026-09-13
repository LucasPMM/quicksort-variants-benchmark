#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "application.h"
#include "benchmark.h"
#include "input.h"
#include "report.h"

static int is_known_variant(const char *variant) {
    const char *const names[] = {"QC", "QM3", "QPE", "QI1", "QI5", "QI10", "QNR"};
    for (size_t i = 0; i < sizeof names / sizeof names[0]; ++i) {
        if (strcmp(variant, names[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int parse_input_order(const char *name, InputOrder *order) {
    if (strcmp(name, "OrdC") == 0) {
        *order = INPUT_ASCENDING;
    } else if (strcmp(name, "OrdD") == 0) {
        *order = INPUT_DESCENDING;
    } else if (strcmp(name, "Ale") == 0) {
        *order = INPUT_RANDOM;
    } else {
        return 0;
    }
    return 1;
}

static int parse_length(const char *text, int *length) {
    char *end;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value <= 0 || value > INT_MAX) {
        return 0;
    }
    *length = (int)value;
    return 1;
}

static void print_original_arrays(int *const *arrays, int length) {
    for (int i = 0; i < BENCHMARK_TRIALS; ++i) {
        print_array(arrays[i], length);
    }
}

int run_application(int argc, char *argv[]) {
    if ((argc != 4 && argc != 5) || (argc == 5 && strcmp(argv[4], "-p") != 0)) {
        fprintf(stderr, "Usage: %s <variant> <input_order> <size> [-p]\n", argv[0]);
        return 1;
    }

    const char *variant = argv[1];
    const char *order_name = argv[2];
    InputOrder order;
    int length;
    if (!is_known_variant(variant) || !parse_input_order(order_name, &order) ||
        !parse_length(argv[3], &length)) {
        fputs("Invalid variant, input order, or size.\n", stderr);
        return 1;
    }

    int **arrays = create_input_arrays(order, length, BENCHMARK_TRIALS);
    if (arrays == NULL) {
        fputs("Could not allocate input arrays.\n", stderr);
        return 1;
    }

    int **originals = NULL;
    if (argc == 5) {
        originals = copy_input_arrays(arrays, length, BENCHMARK_TRIALS);
        if (originals == NULL) {
            fputs("Could not copy input arrays.\n", stderr);
            free_input_arrays(arrays, BENCHMARK_TRIALS);
            return 1;
        }
    }

    BenchmarkResult result = measure_sort(arrays, length, variant);
    if (!result.success) {
        fputs("Could not complete the benchmark.\n", stderr);
        free_input_arrays(originals, BENCHMARK_TRIALS);
        free_input_arrays(arrays, BENCHMARK_TRIALS);
        return 1;
    }
    print_result(variant, order_name, length, &result);
    if (originals != NULL) {
        print_original_arrays(originals, length);
    }

    free_input_arrays(originals, BENCHMARK_TRIALS);
    free_input_arrays(arrays, BENCHMARK_TRIALS);
    return 0;
}
