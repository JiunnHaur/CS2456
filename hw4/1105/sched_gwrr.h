#include <linux/types.h>
#define GWRR_DWEIGHT 10
#define GWRR_MIN_WEIGHT 1
#define GWRR_MAX_WEIGHT 100

struct group_struct
{
    int weight;
	int curr_weight;
    gid_t gid;

    struct list_head group_hanger;
};

