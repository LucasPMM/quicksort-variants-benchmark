#ifndef REPORT_H
#define REPORT_H

#include "benchmark.h"

int print_result(const char *variant, const char *order, int length,
                 const BenchmarkResult *result);
int print_array(const int *values, int length);

#endif
