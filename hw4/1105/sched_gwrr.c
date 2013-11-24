#include <linux/sched_gwrr.h>
static struct list_head groups_holder = LIST_HEAD_INIT(groups_holder);
static struct list_head tasks_holder = LIST_HEAD_INIT(tasks_holder);
static spinlock_t lock = SPIN_LOCK_UNLOCKED;

static void enqueue_task_gwrr(struct rq *rq, struct task_struct *p, int wakeup, bool head)
{
	printk("enqueue pid=%d\n", p->pid);

	struct list_head *pos = NULL;
	struct group_struct *grp = NULL;

	spin_lock(&lock);
	list_for_each(pos, &groups_holder)
	{
		grp = list_entry(pos, struct group_struct, group_hanger);
		if(grp->gid == p->gid)
		{
			break;
		}
	}

	if(grp == NULL || grp->gid != p->gid)
	{
		grp = kmalloc(sizeof(struct group_struct), GFP_KERNEL);
		grp->gid = p->gid;
		grp->curr_weight = grp->weight = GWRR_DWEIGHT;

		list_add_tail(&grp->group_hanger, &groups_holder);
	}

	list_add_tail(&p->task_hanger, &tasks_holder);
	spin_unlock(&lock);
}

static void dequeue_task_gwrr(struct rq *rq, struct task_struct *p, int sleep)
{
	printk("dequeue pid=%d\n", p->pid);
	struct task_struct *tsk = NULL;
	struct list_head *pos = NULL, *next = NULL;
	int grp_tsk_count = 0;
	gid_t gid = p->gid;

	spin_lock(&lock);
	list_for_each_safe(pos, next, &tasks_holder)
	{
		tsk = list_entry(pos, struct task_struct, task_hanger);
		if(tsk->pid == p->pid)
		{
			list_del(pos);
		}
		else if(tsk->gid == gid)
		{
			grp_tsk_count++;
		}
	}

	if(grp_tsk_count == 0)
	{
		struct task_struct *grp = NULL;
		list_for_each_safe(pos, next, &groups_holder)
		{
			grp = list_entry(pos, struct group_struct, group_hanger);
			if(grp->gid == gid)
			{
				list_del(pos);
				break;
			}
		}
	}
	spin_unlock(&lock);
}

static void yield_task_gwrr(struct rq *rq)
{
	//printk("yield_task_gwrr\n");
}

static void check_preempt_curr_gwrr(struct rq *rq, struct task_struct *p, int flags)
{
	//printk("check_preempt_curr_gwrr\n");
}

static struct task_struct *pick_next_task_gwrr(struct rq *rq)
{
	struct list_head *pos = NULL;
	
	struct group_struct *grp = NULL, *max_grp = NULL;
	int max_weight = 0;

	spin_lock(&lock);
	if(list_empty(&groups_holder) || list_empty(&tasks_holder))
	{
		spin_unlock(&lock);
		return NULL;
	}

retry:	
	list_for_each(pos, &groups_holder)
	{
		grp = list_entry(pos, struct group_struct, group_hanger);
		if(grp->curr_weight > max_weight)
		{
			max_weight = grp->curr_weight;
			max_grp = grp;
		}
	}

	if(max_weight == 0)
	{
		list_for_each(pos, &groups_holder)
		{
			grp = list_entry(pos, struct group_struct, group_hanger);
			grp->curr_weight = grp->weight;
		}

		goto retry;
	}

	max_grp->curr_weight--;

	struct task_struct *tsk = NULL;
	list_for_each(pos, &tasks_holder)
	{
		tsk = list_entry(pos, struct task_struct, task_hanger);
		if(tsk->gid == max_grp->gid)
			break;
	}

	if(tsk == NULL)
	{
		spin_unlock(&lock);
		return NULL;
	}

	list_move_tail(&tsk->task_hanger, &tasks_holder);
	spin_unlock(&lock);
	printk("picking %d...\n", tsk->pid);
	return tsk;
}

static void put_prev_task_gwrr(struct rq *rq, struct task_struct *p)
{
	//printk("put_prev_task_gwrr %d\n", p->pid);
}

static void set_curr_task_gwrr(struct rq *rq)
{
	//printk("set_curr_task_gwrr\n");
}

static void task_tick_gwrr(struct rq *rq, struct task_struct *p, int queued)
{
	//printk("task_tick_gwrr %d\n", p->pid);
	set_tsk_need_resched(p);
}

static void prio_changed_gwrr(struct rq *rq, struct task_struct *p, int oldprio, int running)
{
	//printk("prio_changed_gwrr\n");
}

static void switched_to_gwrr(struct rq *this_rq, struct task_struct *p, int running)
{
	//printk("switched_to_gwrr %d\n", p->pid);
}

static const struct sched_class gwrr_sched_class = {
	.next = &idle_sched_class,
	.enqueue_task = enqueue_task_gwrr,
	.dequeue_task = dequeue_task_gwrr,

	.yield_task = yield_task_gwrr,
	.check_preempt_curr = check_preempt_curr_gwrr,

	.pick_next_task = pick_next_task_gwrr,
	.put_prev_task = put_prev_task_gwrr,

	.set_curr_task = set_curr_task_gwrr,
	.task_tick = task_tick_gwrr,
	.prio_changed = prio_changed_gwrr,
	.switched_to = switched_to_gwrr,
};
