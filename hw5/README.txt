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
        This is a system call. It does the following before printing
        results:

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

  test.c

    Note - nice figure that shows reference bit setting scheme in
    Understanding the Linux Kernel (3rd Ed.) p. 693.

    This program is run in the VM.  It makes the system call to
    memstats.

    Example Output (with the LRU policy):

      [  108.358621] 1. active_count:           2425
      [  108.362113] 2. inactive_count:         1346
      [  108.363194] 3. active_ref_bit_count:   1327
      [  108.364166] 4. inactive_ref_bit_count: 493
      [  108.365092] 5. num_active_to_inactive: 0
      [  108.366094] 6. num_inactive_to_free:   0

      Then download 50MB file...

      [  223.283098] 1. active_count:           2569
      [  223.287367] 2. inactive_count:         10661
      [  223.288505] 3. active_ref_bit_count:   1277
      [  223.291341] 4. inactive_ref_bit_count: 403
      [  223.292423] 5. num_active_to_inactive: 475
      [  223.293528] 6. num_inactive_to_free:   3870


    Example Output (with the cs2456 policy):

      [   66.597606] 1. active_count:           2448
      [   66.602026] 2. inactive_count:         1336
      [   66.605983] 3. active_ref_bit_count:   117
      [   66.606840] 4. inactive_ref_bit_count: 490
      [   66.607274] 5. num_active_to_inactive: 0
      [   66.607274] 6. num_inactive_to_free:   0

      Then download 50MB file...

      [  928.959517] 1. active_count:           2644
      [  928.961600] 2. inactive_count:         431
      [  928.962663] 3. active_ref_bit_count:   119
      [  928.965576] 4. inactive_ref_bit_count: 37
      [  928.966479] 5. num_active_to_inactive: 477
      [  928.967120] 6. num_inactive_to_free:   3870

  test2.c
    Iterates over an array of 1048576 longs (each is 8 bytes).
    Changes size of active_list.  Demonstrate like so:

    $ ./a.out
    $ ./test2.out &
    $ ./a.out
    $ kill <test2.out>
    $ ./a.out

  change.sh
    This script downloads a 50MB file (happens to be a linux kernel).
    The idea is that writing a large amount to memory will cause new
    pages to be allocated, and placed in the active_list, but since
    there are so many (only have 64MB of memory), try_to_free_pages is
    called, and shrink_active_list (increments num_active_to_inactive)
    and shrink_inactive_list (increments num_inactive_to_free) are
    both called [reasons for 5 and 6].

    In our policy, inactive_ref_bit 

