Part 2
------
(1) Keep reference counter (ref_cnt) as new member in page struct.

(2) In shrink_active_list(), assuming "evict" means move to inactive
    list, we do the following to each page.

      if(page->ref_cnt + page->PG_referenced >= page->ref_cnt)
        page->ref_cnt += page->PG_referenced

      page->PG_referenced = 0

      if page->ref_cnt == 0
        list_move(page, l_inactive)
      else
        page->ref_cnt--
        list_move(page, l_active)

(3) Find the timer interrupt - inside it add

      for_each_zone(zone)
        list_for_each(zone, active_list)
          if(page->ref_cnt + page->PG_referenced >= page->ref_cnt)
            page->ref_cnt += page->PG_referenced

          page->PG_referenced = 0

        // Do we need to do?
        list_for_each(zone, inactive_list)
          if(page->ref_cnt + page->PG_referenced >= page->ref_cnt)
            page->ref_cnt += page->PG_referenced

          page->PG_referenced = 0
