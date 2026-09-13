#include <stdio.h>
#include <stdlib.h>

#include "application.h"
#include "benchmark.h"
#include "cli.h"
#include "input.h"
#include "report.h"

static int print_original_arrays(int *const *arrays, int length) {
    for (int i = 0; i < BENCHMARK_TRIALS; ++i) {
        if (!print_array(arrays[i], length)) {
            return 0;
        }
    }
    return 1;
}

int run_application(int argc, char *argv[]) {
    CliOptions options;
    int parsed = parse_cli(argc, argv, &options);
    if (parsed <= 0) {
        return parsed == 0 ? 0 : 1;
    }
    srand(options.seed);

    int **arrays = create_input_arrays(options.order, options.length, BENCHMARK_TRIALS);
    if (arrays == NULL) {
        fputs("Could not allocate input arrays.\n", stderr);
        return 1;
    }

    int **originals = NULL;
    if (options.print_originals) {
        originals = copy_input_arrays(arrays, options.length, BENCHMARK_TRIALS);
        if (originals == NULL) {
            fputs("Could not copy input arrays.\n", stderr);
            free_input_arrays(arrays, BENCHMARK_TRIALS);
            return 1;
        }
    }

    BenchmarkResult result = measure_sort(arrays, options.length, options.variant_code);
    if (!result.success) {
        fputs("Could not complete the benchmark.\n", stderr);
        free_input_arrays(originals, BENCHMARK_TRIALS);
        free_input_arrays(arrays, BENCHMARK_TRIALS);
        return 1;
    }
    int output_ok = print_result(options.variant_name, options.order_name, options.length, &result);
    if (output_ok && originals != NULL) {
        output_ok = print_original_arrays(originals, options.length);
    }
    if (fflush(stdout) == EOF) {
        output_ok = 0;
    }

    free_input_arrays(originals, BENCHMARK_TRIALS);
    free_input_arrays(arrays, BENCHMARK_TRIALS);
    if (!output_ok) {
        fputs("Could not write output.\n", stderr);
        return 1;
    }
    return 0;
}
