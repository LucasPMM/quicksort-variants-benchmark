#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "benchmark.h"
#include "cli.h"

typedef struct {
    const char *name;
    const char *code;
} VariantName;

static const VariantName variants[] = {
    {"middle-pivot", "QC"}, {"median-of-three", "QM3"}, {"first-pivot", "QPE"}, {"hybrid-1", "QI1"},
    {"hybrid-5", "QI5"},    {"hybrid-10", "QI10"},      {"iterative", "QNR"},
};

static void print_usage(FILE *stream, const char *program) {
    fprintf(stream, "Usage: %s <variant> <input_order> <size> [-p|--print] [--seed N]\n", program);
    fputs("Variants: middle-pivot, median-of-three, first-pivot, hybrid-1, "
          "hybrid-5, hybrid-10, iterative\n",
          stream);
    fputs("Input orders: ascending, descending, random\n", stream);
    fprintf(stream, "Size: 1 to %d elements (the historical study used 50,000-step sizes).\n",
            BENCHMARK_MAX_LENGTH);
    fputs("The last result field is median execution time in microseconds.\n", stream);
    fputs("Legacy variant codes and OrdC, OrdD, Ale input codes are also accepted.\n", stream);
}

static const char *variant_code(const char *name) {
    for (size_t i = 0; i < sizeof variants / sizeof variants[0]; ++i) {
        if (strcmp(name, variants[i].name) == 0 || strcmp(name, variants[i].code) == 0) {
            return variants[i].code;
        }
    }
    return NULL;
}

static int parse_order(const char *name, InputOrder *order) {
    if (strcmp(name, "ascending") == 0 || strcmp(name, "OrdC") == 0) {
        *order = INPUT_ASCENDING;
    } else if (strcmp(name, "descending") == 0 || strcmp(name, "OrdD") == 0) {
        *order = INPUT_DESCENDING;
    } else if (strcmp(name, "random") == 0 || strcmp(name, "Ale") == 0) {
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
    if (errno != 0 || end == text || *end != '\0' || value < 1 || value > BENCHMARK_MAX_LENGTH) {
        return 0;
    }
    *length = (int)value;
    return 1;
}

static int parse_seed(const char *text, unsigned int *seed) {
    if (*text == '\0') {
        return 0;
    }
    for (const char *digit = text; *digit != '\0'; ++digit) {
        if (*digit < '0' || *digit > '9') {
            return 0;
        }
    }
    char *end;
    errno = 0;
    unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || *end != '\0' || value > UINT_MAX) {
        return 0;
    }
    *seed = (unsigned int)value;
    return 1;
}

static int parse_flags(int argc, char *argv[], CliOptions *options) {
    int seed_seen = 0;
    for (int i = 4; i < argc; ++i) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--print") == 0) {
            if (options->print_originals) {
                return 0;
            }
            options->print_originals = 1;
        } else if (strcmp(argv[i], "--seed") == 0) {
            if (seed_seen || ++i >= argc || !parse_seed(argv[i], &options->seed)) {
                return 0;
            }
            seed_seen = 1;
        } else {
            return 0;
        }
    }
    return 1;
}

int parse_cli(int argc, char *argv[], CliOptions *options) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        print_usage(stdout, argv[0]);
        if (fflush(stdout) == EOF) {
            fputs("Could not write help.\n", stderr);
            return -1;
        }
        return 0;
    }
    if (argc < 4) {
        print_usage(stderr, argv[0]);
        return -1;
    }

    *options = (CliOptions){.seed = (unsigned int)time(NULL)};
    options->variant_name = argv[1];
    options->order_name = argv[2];
    options->variant_code = variant_code(argv[1]);
    if (options->variant_code == NULL || !parse_order(argv[2], &options->order) ||
        !parse_length(argv[3], &options->length) || !parse_flags(argc, argv, options)) {
        fputs("Invalid command-line arguments.\n", stderr);
        print_usage(stderr, argv[0]);
        return -1;
    }
    return 1;
}
