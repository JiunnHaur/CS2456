CS 2456 Homework 05 - Qiyuan Qiu and Jake Brock
-----------------------------------------------

Files Changed:

  main.c
    Included "memstats.h" so that initialize_global() is available.

  Makefile
    core-y has cs2456 folder in it now.

  memstats.c
    Assignment Part 1:

      initialize_global(void) // For Parts 1.5 - 1.6
        Initializes num_active_to_inactive and num_inactive_to_free
        both to 0.

      sys_memstats(void) // Solves Parts 1.1 - 1.4
        Uses for_each_zone(zone) and then list_for_each() (for
        zone->active_list, then zone->inactive_list) to iterate over
        every page, counting the number in each list, and the number
        whose reference bits are set.  Uses spin_lock to protect
        access to the zone.

    Assignment Part 2:
      Creates a timer that periodically scans the pages and adds
      their reference bit to their counter.  If the counter would
      overflow (sum is 0), then bit is not added (that is, it is kept
      at the largest value).  Currently once per second - but can be
      adjusted in line 109).

  memstats.h
    Prototype for initialize_global().
    declare num_active_to_inactive and num_inactive to free.

  mm_types.h
    Augmented struct page to include a counter for the number of times
    the page has been referenced "ref_cnt".

  syscall_table_32.S
    Included the memstats system call.

  unistd_32.h
    Included the memstats system call.

  vmscan.c
    Assignment Part 1
      When shrink_active_list is run, it calculates pgdeactivated.  We
      accumulate this number to our counter "num_active_to_inactive"
      at each run.

      When shink_inactive_list is run, it calculates (and returns)
      nr_reclaimed.  We accumulate this number to our counter
      "num_inactive_to_free" at each run.

    Assignment Part 2
      The policy for eviction is held inside of shrink_active_list().
      It iterates over l_hold (a local copy of part of the
      active_list) and at each page, decides whether to move it to the
      l_active list or l_inactive list.

      Where the original version tests if the page is referenced to
      decide, we "add the reference bit value to the page's reference
      counter [ref_cnt] (and clear the reference bit at the same
      time). Then we check the reference counter. If the counter is
      0, you evict the page. Otherwise, you decrement the counter by
      1 and move the frame to the back of list (as the original
      second-chance LRU approximation normally does)." (Quoted from
      assignment).
