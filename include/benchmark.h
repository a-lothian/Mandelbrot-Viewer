#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <stdbool.h>

 struct BenchmarkOpts {
     int threads;
     bool smooth;
 };

 void run_benchmark(struct BenchmarkOpts opts);
 #endif