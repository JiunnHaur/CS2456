#ifndef _KERSYNC_H
#define _KERSYNC_H 
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/wait.h>
typedef struct 
{
    spinlock_t e_lock;
    long event_id;
    int invalid;
    int count;
    wait_queue_head_t Q;
    //Q local to each event
    struct list_head list;
    //List for all events
    //cannot be a pointer because 
    //this list will embeded into 
    //the structure of the container
}event_t;
void doevent_init(void);
#endif
