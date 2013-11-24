#include <stdio.h>
#include "hrtimer_x86.h"
#include <unistd.h>
#define MAX_ITERATION 1000
int main()
{
    long long interval_cyc;
    double cyc_per_us;
    long long start_cyc,end_cyc;
    int i=0;
    interval_cyc = 0;
    start_cyc = gethrcycle_x86();
    for(i=0;i<MAX_ITERATION;i++)
    {
       getpid();
    }
    end_cyc = gethrcycle_x86();
    interval_cyc = (end_cyc - start_cyc);
    cyc_per_us = getMHZ_x86();
    double total_time_secs;
    total_time_secs = (double) interval_cyc / cyc_per_us / 1000;
    printf("The minimum cost for system call is %f mili second(s)\n",
            total_time_secs/MAX_ITERATION);
    return 0;
}
