#ifndef __MEMSTATS_H
#define __MEMSTATS_H

#include <linux/list.h>
#include <linux/wait.h>
#include <linux/spinlock.h>

void initialize_global(void);

extern long num_active_to_inactive;
extern long num_inactive_to_free;

#endif
