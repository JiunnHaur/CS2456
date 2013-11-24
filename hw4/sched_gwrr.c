#include <linux/sched_gwrr.h>
static struct list_head fifo_holder = LIST_HEAD_INIT(fifo_holder);

static struct list_head groups_holder = LIST_HEAD_INIT(groups_holder);
static spinlock_t group_lock = SPIN_LOCK_UNLOCKED;

struct task_struct *last = NULL;

void print_structure()
{
	group_member *tmp = NULL;
	struct task_struct *t_tmp = NULL;
	struct list_head *pos = NULL;
	struct list_head *t_pos = NULL;
	spin_lock(&group_lock);
	list_for_each(pos, &groups_holder)
	{
		tmp = list_entry(pos, group_member, group_hanger);
		printk("gid=%d\n\t", tmp->gid);
		list_for_each(t_pos, &tmp->tasks_holder)
			{
				t_tmp = list_entry(t_pos, struct task_struct, task_hanger);
				printk("%d, ",t_tmp->pid);
			}
		printk("\n");
	}
	spin_unlock(&group_lock);
}

static void enqueue_task_gwrr(struct rq *rq, struct task_struct *p, int wakeup, bool head)
{
	printk("enqueue pid=%d\n", p->pid);
	pid_t gid = p->gid;
	group_member *tmp = NULL;
	struct list_head *pos = NULL;

	spin_lock(&group_lock);

	list_for_each(pos, &groups_holder)
	{
		tmp = list_entry(pos, group_member, group_hanger);
		if(tmp->gid == gid)
		{
			printk("found gid=%d\n", gid);
			break;
		}
	}

	if(tmp == NULL)
	{
		printk("not found gid=%d\n", gid);
		tmp = kmalloc(sizeof(group_member),GFP_KERNEL);
		tmp->gid = gid;
		tmp->weight = GWRR_DWEIGHT;
		tmp->curr_weight = tmp->weight;
		INIT_LIST_HEAD(&tmp->tasks_holder);
		list_add(&tmp->group_hanger, &groups_holder);
	}

	list_add(&p->task_hanger, &tmp->tasks_holder);
	spin_unlock(&group_lock);
	print_structure();

	last = p;
}

static void dequeue_task_gwrr(struct rq *rq, struct task_struct *p, int sleep)
{
	printk("dequeue pid=%d\n", p->pid);
	pid_t gid = p->gid;
	group_member *tmp = NULL;
	struct task_struct *t_tmp = NULL;
	struct list_head *pos = NULL, *next = NULL;
	struct list_head *t_pos = NULL, *t_next = NULL;

	spin_lock(&group_lock);
	list_for_each_safe(pos, next, &groups_holder)
	{
		tmp = list_entry(pos, group_member, group_hanger);
		printk("tmp->gid=%d p->gid=%d\n", tmp->gid, p->gid);
		if(tmp->gid == gid)
		{
			printk("found gid=%d\n", gid);
			list_for_each_safe(t_pos, t_next, &tmp->tasks_holder)
			{
				t_tmp = list_entry(t_pos, struct task_struct, task_hanger);
				if(t_tmp->pid == p->pid)
				{
					printk("found pid=%d\n", p->pid);
					list_del(t_pos);
					kfree(t_tmp);
					break;
				}
			}

			if(list_empty(&tmp->tasks_holder))
			{
				printk("group gid=%d is empty, deleteing...\n", gid);
				list_del(pos);
				kfree(tmp);
			}
			break;
		}
	}
	spin_unlock(&group_lock);
	print_structure();

	if(last == p)
		last = NULL;
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
	group_member *tmp = NULL, *max_tmp = NULL;
	struct list_head *pos = NULL, *next = NULL;
	int max = 0;

	//printk("in pick next...\n");
	print_structure();
	spin_lock(&group_lock);

	list_for_each(pos, &groups_holder)
	{
		tmp = list_entry(pos, group_member, group_hanger);
		if(tmp->curr_weight > max)
		{
			max = tmp->curr_weight;
			max_tmp = tmp;
		}
	}

	if(tmp == NULL)
	{
		spin_unlock(&group_lock);
		//printk("picking NULL\n");
		return NULL;
	}

	if(max == 0)
	{
		list_for_each(pos, &groups_holder)
		{
			tmp = list_entry(pos, group_member, group_hanger);
			tmp->curr_weight = tmp->weight;
		}

		list_for_each(pos, &groups_holder)
		{
			tmp = list_entry(pos, group_member, group_hanger);
			if(tmp->curr_weight > max)
			{
				max = tmp->curr_weight;
				max_tmp = tmp;
			}
		}
	}

	max_tmp->curr_weight--;

	struct task_struct *p = NULL;
	list_for_each_safe(pos, next, &max_tmp->tasks_holder)
	{
		p = list_entry(pos, struct task_struct, task_hanger);
		break;
	}

	if(p == NULL)
	{
		spin_unlock(&group_lock);
		//printk("picking NULL\n");
		return NULL;
	}

	list_move_tail(&p->task_hanger, &max_tmp->tasks_holder);

	spin_unlock(&group_lock);

	//print_structure();
	//printk("picking %d\n", p->pid);

	return p;
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
