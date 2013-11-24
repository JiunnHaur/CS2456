#include <stdio.h>
#include "hrtimer_x86.h"
#define NUM_ITERATION 1000
void foo();
int main()
{
    hrtime_t start_cyc, end_cyc;
    int i=0;
    double interval_from_cyc=0;
    start_cyc = gethrcycle_x86();
    for(i=0;i<NUM_ITERATION;i++)
    {
        foo();
    }
    end_cyc = gethrcycle_x86();

    interval_from_cyc = (end_cyc - start_cyc);    
    double cyc_per_usec = getMHZ_x86();
    interval_from_cyc = interval_from_cyc / cyc_per_usec / 1000; 
    
    printf("Minimun function cost is: %f mili second(s).\n",interval_from_cyc/NUM_ITERATION);
    return 0;
}

void foo()
{
    return;
}
