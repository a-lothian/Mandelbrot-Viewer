#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <stdbool.h>

struct BenchmarkOpts {
    int threads;
    bool smooth;
    bool scalar;
    bool sweep;
    bool no_optimisations;
};

void run_benchmark(struct BenchmarkOpts opts);
void run_sweep(struct BenchmarkOpts opts);
#endif