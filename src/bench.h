#ifndef BENCH_H
#define BENCH_H


#include <stdio.h>
#include <time.h>


#define BENCHMARK_START(param_name, param_n) \
{ \
    char *BM_name = param_name; \
    printf("%s...", BM_name); \
    fflush(stdout); \
    unsigned long int BM_n = param_n; \
    clock_t BM_clock_start = clock(); \
    for (unsigned long int i = 0; i < BM_n; i++) {


#define BENCHMARK_END \
    } \
    clock_t BM_clock_elapsed = clock() - BM_clock_start; \
    float BM_clock_secs = (float)BM_clock_elapsed / CLOCKS_PER_SEC / BM_n; \
    float BM_clock_usecs = 1000 * 1000  * BM_clock_secs; \
    printf("\r%s: %fus\n", BM_name, BM_clock_usecs); \
}


#endif
