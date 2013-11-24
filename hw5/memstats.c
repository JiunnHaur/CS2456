//headers from timer.c
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/pid_namespace.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/tick.h>
#include <linux/kallsyms.h>
#include <linux/mmzone.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>

#include <linux/kernel.h>
#include <linux/page-flags.h>
#include "memstats.h"

#include <linux/spinlock.h>
#include <linux/timer.h>

long num_active_to_inactive;
long num_inactive_to_free;

/* Timer Help From Online: http://dinomycle.blogspot.com/2009/07/using-timers-in-linux-kernel-module.html */

struct timer_list timer1;

int i;

void timer1_routine(unsigned long data)
{
  int zone_num = 0;

  struct zone * zone;
  struct page * page;

  struct list_head * pos1;
  struct list_head * pos2;

  int ref_bit;
  unsigned long num_active_modified;
  unsigned long num_inactive_modified;

  for_each_zone(zone){
    zone_num++;
    num_active_modified = 0;
    num_inactive_modified = 0;

    spin_lock_irq(&zone->lru_lock);

    list_for_each(pos1, &(zone->active_list)){
      page = list_entry(pos1, struct page, lru);

      if( !(test_bit(PG_lru, &page->flags)) ){
        printk("This page has PG_lru = 0\n");
        continue;
      }

      ref_bit = TestClearPageReferenced(page);

      if(page->ref_cnt + ref_bit > 0) {
        num_active_modified++;
        page->ref_cnt += ref_bit;
      }
    }
    spin_unlock_irq(&zone->lru_lock);

    spin_lock_irq(&zone->lru_lock);

    list_for_each(pos2, &(zone->inactive_list)){
      page = list_entry(pos1, struct page, lru);

      if( !(test_bit(PG_lru, &page->flags)) ){
        printk("This page has PG_lru = 0\n");
        continue;
      }

      ref_bit = TestClearPageReferenced(page);

      if(page->ref_cnt + ref_bit > 0) {
        num_inactive_modified++;
        page->ref_cnt += ref_bit;
      }
    }

    spin_unlock_irq(&zone->lru_lock);
  
    printk("Zone %i:\n", zone_num);
    printk("num_active_modified: %lu\n", num_active_modified);
    printk("num_inactive_modified: %lu\n\n",  num_inactive_modified);
  }

  printk(KERN_ALERT"Inside Timer Routine count-> %d data passed %ld\n",i++,data);

  mod_timer(&timer1, jiffies + HZ); /* restarting timer */
}

static int timer_module_init(void)
{
  init_timer(&timer1);

  timer1.function = timer1_routine;
  timer1.data = 1;
  timer1.expires = jiffies + HZ; /* 1 second */
  add_timer(&timer1); /* Starting the timer */

  printk(KERN_ALERT"Timer Module loaded\n");
  return 0;
}

static void timer_module_exit(void)
{
  del_timer_sync(&timer1); /* Deleting the timer */

  printk(KERN_ALERT "Timer module unloaded \n");
}

module_init(timer_module_init);
module_exit(timer_module_exit);

/* End From Online */

void initialize_global(void){
  num_active_to_inactive = 0;
  num_inactive_to_free = 0;
  //  kthread_run(ref_cnt_maintain, NULL, "ref_cnt_maintain");

  printk("\n\nInitialized Global\n\n");
}

asmlinkage int sys_memstats(void){
  struct zone * zone;
  struct page * page;

  long active_count = 0;
  long inactive_count = 0;
  long active_ref_bit_count = 0;
  long inactive_ref_bit_count = 0;

  struct list_head * pos1;
  struct list_head * pos2;

  for_each_zone(zone){
    spin_lock_irq(&zone->lru_lock);
    list_for_each(pos1, &(zone->active_list)){
      active_count++;
      page = list_entry(pos1, struct page, lru);
      active_ref_bit_count += test_bit(PG_referenced, &page->flags);
    }
    list_for_each(pos2, &(zone->inactive_list)){
      inactive_count++;  // see zone->nr_inactive
      page = list_entry(pos2, struct page, lru);
      inactive_ref_bit_count += test_bit(PG_referenced, &page->flags);
    }
    spin_unlock_irq(&zone->lru_lock);
  }

  printk("1. active_count:           %lu\n", active_count);
  printk("2. inactive_count:         %lu\n", inactive_count);
  printk("3. active_ref_bit_count:   %lu\n", active_ref_bit_count);
  printk("4. inactive_ref_bit_count: %lu\n", inactive_ref_bit_count);
  printk("5. num_active_to_inactive: %lu\n", num_active_to_inactive);
  printk("6. num_inactive_to_free:   %lu\n", num_inactive_to_free);

  return 0;
}
