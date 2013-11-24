// Tests for Hwk 05

#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>

#define _memstats_ 327

int main(){
  printf("Page size is: %i\n", getpagesize());
  syscall(_memstats_);
  return 0;
}

// 1. the current number of pages in the active list (over all memory zones);
//   * Verify with cat /proc/meminfo | grep ctive

// 2. the current number of pages in the inactive list (over all memory zones);
// 3. the current number of pages in the active list whose reference bits are
//    set (over all memory zones);
// 4. the current number of pages in the inactive list whose reference bits are
//    set (over all memory zones);
// 5. the cumulative number of pages moved from the active list to the inactive
//    list (since the last machine boot);
// 6. the cumulative number of pages evicted from the inactive list (since the
//    last machine boot);
