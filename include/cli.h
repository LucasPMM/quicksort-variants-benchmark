#ifndef CLI_H
#define CLI_H

#include "input.h"

typedef struct {
    const char *variant_name;
    const char *variant_code;
    const char *order_name;
    InputOrder order;
    int length;
    int print_originals;
    unsigned int seed;
} CliOptions;

/* Return 1 for valid options, 0 for help, and -1 for invalid input. */
int parse_cli(int argc, char *argv[], CliOptions *options);

#endif
