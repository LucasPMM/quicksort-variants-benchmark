#ifndef REPORT_H
#define REPORT_H

#include "benchmark.h"

void print_result(const char *variant, const char *order, int length,
                  const BenchmarkResult *result);
void print_array(const int *values, int length);

#endif
