#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include "kersync.h"
#include <linux/ioctl.h>
#define MAX_EVENT_ID 5000
#define DEBUG

//global lock
//this will be initialized by sys_doevent_init()
//function during the system boot up process. 
static spinlock_t list_lock;
static LIST_HEAD(event_list);
//event_list is not a pointer
static int next_event_id;

event_t * find_event_by_id(long event_id)
{
    struct list_head *p;
    event_t *f;

    list_for_each(p, &event_list){
       f = list_entry(p,event_t, list); 
       if(f->event_id == event_id)
           return f;
    }

    return NULL;
}

void event_init(event_t *cnt_ent)
{
    cnt_ent->event_id = next_event_id++;
    cnt_ent->invalid = 0;
    cnt_ent->count = 0;
    init_waitqueue_head(&cnt_ent->Q);
    spin_lock_init(&cnt_ent->e_lock);
    INIT_LIST_HEAD(&cnt_ent->list);
}

asmlinkage long sys_doevent_open(void)
{
    event_t * new_ent;
    printk(" Event Open !\n");
    spin_lock(&list_lock);
    if(next_event_id > (MAX_EVENT_ID))
        return -1;
    new_ent = kmalloc(sizeof(* new_ent), GFP_KERNEL);
    event_init(new_ent);
    list_add(&new_ent->list, &event_list);
    spin_unlock(&list_lock);

    return 0;
}

asmlinkage long sys_doevent_wait(long event_id)
{
#ifdef DEBUG
    printk("inside wait\n");
#endif
	/*lock global lock*/
    spin_lock(&list_lock);

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event==NULL)
	{
		/*unlock global lock*/
        spin_unlock(&list_lock);
#ifdef DEBUG
        printk("event is NULL\n");
#endif
		return -1;
	}
	event->count++;										/*increase the num of process using the event*/

	/*unlock global lock*/
    spin_unlock(&list_lock);

	/*lock event lock*/
    spin_lock(&event->e_lock);
	if(event->invalid)									/*if already close*/
	{
		event->count--;										/*after use of the event*/
        printk("Q no longer exists\n");
		/*unlock event lock*/
        spin_unlock(&event->e_lock);
#ifdef DEBUG
        printk("invalid set");
#endif
		return -1;
	}
	/*unlock event lock*/
    spin_unlock(&event->e_lock);
#ifdef DEBUG
    printk("Going to be put into Q\n");
#endif
	sleep_on(&event->Q);								/*sleep on event*/

    spin_lock(&event->e_lock);
	event->count--;										/*after use of the event*/
    spin_unlock(&event->e_lock);
#ifdef DEBUG
    printk("Wake up from kernel sleep\n");
#endif
	return 0;
}

asmlinkage long sys_doevent_sig(long event_id)
{
	/*lock global lock*/
    spin_lock(&list_lock);

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event==NULL)
	{
		/*unlock global lock*/
        spin_unlock(&list_lock);
		return -1;
	}
	event->count++;										/*increase the num of process using the event*/

	/*unlock global lock*/
    spin_unlock(&list_lock);

	/*lock event lock*/
    spin_lock(&event->e_lock);
	if(event->invalid)									/*if already close*/
	{
		event->count--;										/*after use of the event*/
        printk("Q no longer exists\n");
        /*unlock event lock*/
        spin_unlock(&event->e_lock);
		return -1;
	}
	wake_up(&event->Q);								
	event->count--;										/*after use of the event*/
	/*unlock event lock*/
    spin_unlock(&event->e_lock);
	return 0;
}


asmlinkage long sys_doevent_close(long event_id)
{
	/*lock global lock*/
    spin_lock(&list_lock);

	event_t * event = find_event_by_id(event_id);		/*find event by event id*/
	if(event == NULL)
	{
		/*unlock global lock*/
        spin_unlock(&list_lock);
		return -1;
	}
	list_del(&event->list);

	/*unlock global lock*/
    spin_unlock(&list_lock);
	
	/*lock event lock*/
    spin_lock(&event->e_lock);
		wake_up(&event->Q);								/*signal the queue*/
		event->invalid = 1;
		if(event->count==0)
        {
			kfree(event);
        }
        else
        {
            printk("Should join all thread and close\n");
            spin_unlock(&event->e_lock);
            return -1;
        }
		
	/*unlock event lock*/
    spin_unlock(&event->e_lock);
	return 0;
}

void doevent_init(void)
{
    next_event_id = 0; 
    spin_lock_init(&list_lock);
    printk("list_lock initialized\n");
}


