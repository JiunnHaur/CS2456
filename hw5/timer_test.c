#include <stdio.h>
#include <linux/timer.h>

void myfunction(void)
{
    printf("My timer wokrs \n");
}

int main()
{
    struct timer_list mytimer;
    init_timer(&mytimer);
    TIMER_INITIALIZER(myfunction, 5,0);
    add_timer(&mytimer);
    return 0;
}

