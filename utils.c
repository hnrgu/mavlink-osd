#include "utils.h"

#include <unistd.h>
#include <time.h>

uint64_t get_monotonic_time() {
    struct timespec ts = {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}
