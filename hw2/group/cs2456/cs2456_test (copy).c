#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/prinfo.h>
#include <linux/types.h>
#include <linux/sched.h>
//return 0 on success
//return -22 if prinfo is null
asmlinkage long sys_cs2456_test(struct prinfo *info)
{
    struct task_struct *ptr_task;
    int i;
    int my_pid, cnt_pid, youngest_of_olders,oldest_of_youngers, youngest_pid;
    long cutime,cstime;
    struct task_struct *ptr_cnt_child_task, *ptr_cnt_sibling_task;
    struct list_head *list,*list1;
    if(info == NULL) return -22;
    
    #ifdef DEBUG
    printk("--syscall input pid is  %d\n",info->pid);
    printk("The state of pid %d is %ld\n",info->pid, ptr_task->state);
    #endif    

    ptr_task = find_task_by_pid(info->pid); 
    //Find state
    info->state = ptr_task->state;
    
    //Get the nice value
    info->nice = task_nice(ptr_task);

    //Get the parent pid
    info->parent_pid = ptr_task->parent->pid;
    
    //Get start time
    info->start_time = ptr_task->real_start_time.tv_sec;

    //Get user_time
    info->user_time = ptr_task->utime;

    //Get sys_time
    info->sys_time = ptr_task->stime;

    //Get uid
    info->uid = ptr_task->uid;

    //Get comm
    i=0;
    do
    {
        info->comm[i] = ptr_task->comm[i];
    }while(info->comm[i++]);
    
    //Iterate through children list for current process with given pid.
    //Find the value for time
    //TODO implement one more iteration to find the "right" valueinitial value
    //for youngest_pid
    cutime = cstime = 0; 
    youngest_pid = 400000;
    list_for_each(list,&ptr_task->children){
        ptr_cnt_child_task = list_entry(list, struct task_struct, sibling);
        printk("cnt child pid %d\n",ptr_cnt_child_task->pid);
        cnt_pid = ptr_cnt_child_task->pid;
        if(cnt_pid < youngest_pid)
            youngest_pid = cnt_pid;
        cutime += ptr_cnt_child_task->utime;
        cstime += ptr_cnt_child_task->stime;
    }
    info->cutime = cutime;
    info->cstime = cstime;
    info->youngest_child_pid = youngest_pid;

    //Iterate through sibling list for current process with given pid
    //Find the values for youngest_of_olders and oldest_of_youngers
    //TODO implement one more iteration to find the "right" initial value
    //for youngest_of_olders and oldest_of_youngers
    youngest_of_olders = 30000,  oldest_of_youngers = 0;
    list_for_each(list1,&ptr_task->sibling){
        ptr_cnt_sibling_task = list_entry(list1, struct task_struct, sibling);
        printk("cnt sibling pid %d\n",ptr_cnt_sibling_task->pid);
        my_pid = ptr_task->pid;
        cnt_pid = ptr_cnt_sibling_task->pid;

        if(cnt_pid > my_pid && cnt_pid < youngest_of_olders )
        {
            #ifdef DEBUG
            printk("cnt_pid %d my_pid %d younge_of_older_pid %d\n",cnt_pid,my_pid, youngest_of_olders);
            #endif
            youngest_of_olders = cnt_pid;
        }
        if(cnt_pid <  my_pid && cnt_pid > oldest_of_youngers )
        {
            #ifdef DEBUG
            printk("cnt_pid %d my_pid &d oldest_of_youngers %d\n",cnt_pid,my_pid, oldest_of_youngers);
            #endif
            oldest_of_youngers = cnt_pid;
        }
    }
    info->younger_sibling_pid = youngest_of_olders;
    info->older_sibling_pid = oldest_of_youngers;

    return 0; 
}
