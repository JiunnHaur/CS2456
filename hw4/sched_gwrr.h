#include <linux/types.h>
#define GWRR_DWEIGHT 10

typedef struct
{
    int weight;
	int curr_weight;
    gid_t gid;
    struct list_head group_hanger;
    struct list_head tasks_holder;
}
group_member;

