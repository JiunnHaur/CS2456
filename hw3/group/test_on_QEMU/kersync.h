#ifndef _KERSYNC_H
#define _KERSYNC_H 
#include <linux/spinlock.h>
#include <linux/sys.h>
#include <linux/sched.h>
typedef struct event
{
    spinlock_t e_lock;
    int invalid;
    int count;
    wait_queue *Q;
}event_t;

//global lock
//this will be initialized by sys_doevent_init()
//function during the system boot up process. 
spinlock_t list_lock;
#endif
