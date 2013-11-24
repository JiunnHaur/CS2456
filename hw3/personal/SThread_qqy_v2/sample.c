#define _REENTRANT
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "sthread.h"

sthread_mutex_t lala;
int
threadmain(void *arg)
{
    int threadno = (int)arg;
    for (;;) {
        sthread_mutex_lock(&lala);
        printf("thread %d: I'm going to sleep\n", threadno);
        //sthread_suspend();
        sleep(1);
        printf("thread %d: I woke up!\n", threadno);
        sthread_mutex_unlock(&lala);
    }
    return 0;
}

int
main(int argc, char *argv[])
{
    sthread_t thr1, thr2, thr3;

    sthread_mutex_init(&lala);

    int i;
    if (sthread_init() == -1)
    fprintf(stderr, "%s: sthread_init: %s\n", argv[0], strerror(errno));

    if (sthread_create(&thr1, threadmain, (void *)1) == -1)
    fprintf(stderr, "%s: sthread_create: %s\n", argv[0], strerror(errno));

    if (sthread_create(&thr2, threadmain, (void *)2) == -1)
    fprintf(stderr, "%s: sthread_create: %s\n", argv[0], strerror(errno));

    if (sthread_create(&thr3, threadmain, (void *)3) == -1)
    fprintf(stderr, "%s: sthread_create: %s\n", argv[0], strerror(errno));
    //sleep(1);
    //sthread_wake(thr1);
    //sleep(1);
    //sthread_wake(thr2);
    //sleep(1);
    //sthread_wake(thr1);
    //sthread_wake(thr2);
    //sleep(1);
    sleep(10);

    //sthread_mutex_destroy(&lala);
    return 0;
}
