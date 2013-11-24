#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include "prinfo.h"
#include <linux/types.h>
#define _CS2456_TEST_ 327

void list_field(struct prinfo *p_info);
void list_field2(struct prinfo *p_info);
//TODO
//Add in funcition that reminds user
//the useage of this program
int main(int argc, char *argv[])
{
        if(argc<=1)
        {
                printf("usage: ./mysyscall <pid>\n");
                return 0;
        }
        char abc[220];
        struct prinfo info;
        info.pid = atoi(argv[1]);
        if(kill(info.pid,0)!=0)
        {
                printf("PID Doesn't exist\n");
                return -1;
        }
        printf("\nDiving to kernel level\n\n");
        syscall(_CS2456_TEST_ , &info);
        printf("\nRising to user level\n\n");
        list_field2(&info);
        return 0;
}

void list_field(struct prinfo *p_info)
{
        printf("current state of process %*ld\n",20,p_info->state);
        printf("process nice value: %*ld\n",20,p_info->nice);
        printf("process id (input): %*d\n",20,p_info->pid);
        printf("process id of parent: %*d\n",20,p_info->parent_pid);
        printf("process id of youngest child: %*d\n",20,p_info->youngest_child_pid);
        printf("pid of the youngest among older siblings: %*d\n",20,p_info->older_sibling_pid);
        printf("pid of the oldest among younger siblings: %*d\n",20,p_info->younger_sibling_pid);
        printf("process start time %*lu\n",20,p_info->start_time);
        printf("CPU time spent in user mode %*d\n",20,p_info->user_time);
        printf("CPU time spent in system mode %*d\n",20,p_info->sys_time);
        printf("total user time of children %*ld\n",20,p_info->cutime);
        printf("total system time of children %*ld\n",20,p_info->cstime);
        printf("user id of process owner %*ld\n",20,p_info->uid);
        printf("name of program executed %*s\n",20,p_info->comm);
}
void list_field2(struct prinfo *pri_info)
{
        char sentences[15][100];
        int i,j;
        char aa;
        int len_al=60;
        strcpy (sentences[0],"Current State of Process is ");
        strcpy (sentences[1],"Nice Value of Process is ");
        strcpy (sentences[2],"Process Identifier ");
        strcpy (sentences[3],"Process ID of parent ");
        strcpy (sentences[4],"Process ID of youngest child ");
        strcpy (sentences[5],"PID of the oldest among younger siblings ");
        strcpy (sentences[6],"PID of the youngest among older siblings ");
        strcpy (sentences[7],"Process start time ");
        strcpy (sentences[8],"CPU time spent in user mode ");
        strcpy (sentences[9],"CPU time spent in system mode ");
        strcpy (sentences[10],"Total user time of children ");
        strcpy (sentences[11],"Total system time of children ");
        strcpy (sentences[12],"User id of process owner ");
        strcpy (sentences[13],"Name of program executed ");

        /*
        printf("%s %d",sentences[1],strlen(sentences[2]));
        scanf("%c",&aa);
        */
        len_al=strlen(sentences[0]);
        for (i=0;i<14;i++)
        {
                if (strlen(sentences[i])>len_al)
                        len_al=strlen(sentences[i]);
        }
        len_al+=1;
        //printf("\n %d %d",len_al,strlen(sentences[6]));
        for (i=0;i<14;i++)
        {
                printf("\n");
                for (j=0;j<len_al-strlen(sentences[i]);j++)
                {
                        printf(" ");
                        //printf("\n %d %d",j,len_al-strlen(sentences[i]));
                        //scanf("%c",&aa);
                }
                if (i==0)
                        printf("%s %ld",sentences[i],pri_info->state);
                if (i==1)
                        printf("%s %ld",sentences[i],pri_info->nice);
                if (i==2)
                        printf("%s %d",sentences[i],pri_info->pid);
                if (i==3)
                        printf("%s %d",sentences[i],pri_info->parent_pid);
                if (i==4)
                        printf("%s %d",sentences[i],pri_info->youngest_child_pid);
                if (i==5)
                        printf("%s %d",sentences[i],pri_info->younger_sibling_pid);
                if (i==6)
                        printf("%s %d",sentences[i],pri_info->older_sibling_pid);
                if (i==7)
                        printf("%s %lu",sentences[i],pri_info->start_time);
                if (i==8)
                        printf("%s %ld",sentences[i],pri_info->user_time);
                if (i==9)
                        printf("%s %ld",sentences[i],pri_info->sys_time);
                if (i==10)
                        printf("%s %ld",sentences[i],pri_info->cutime);
                if (i==11)
                        printf("%s %ld",sentences[i],pri_info->cstime);
                if (i==12)
                        printf("%s %ld",sentences[i],pri_info->uid);
                if (i==13)
                        printf("%s %s",sentences[i],pri_info->comm);


        }
        printf("\n");
        return;
}


