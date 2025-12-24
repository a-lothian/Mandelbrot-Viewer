#include "core_count.h"

long get_num_logical_cores() {
    long num_cores = 1;

#if defined(_WIN32) || defined(_WIN64)
    // Windows
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    num_cores = sysinfo.dwNumberOfProcessors;

#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
    // POSIX: macOS, linux, etc
    num_cores = sysconf(_SC_NPROCESSORS_ONLN);

#endif
    // handle -1 fail by defaulting to 1 core
    return (num_cores > 0) ? num_cores : 1;
}