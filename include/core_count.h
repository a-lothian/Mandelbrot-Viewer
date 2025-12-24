#ifndef CORE_COUNT
#define CORE_COUNT

#include <stdio.h>

// returns number of cores on the user's machine
long get_num_logical_cores();

#if defined(_WIN32) || defined(_WIN64)
// Windows
#include <windows.h>

#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
// POSIX: macOS, linux, etc
#include <unistd.h>

#else
// unknown systems
#warning "Unknown OS detected. Defaulting core count to 1."
#endif
#endif