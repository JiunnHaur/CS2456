//Mary
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/sched.h>
#include <sched.h>
#include <stdio.h>
#include <sys/syscall.h>
#define SCHED_GWRR 4
#define _GET_GROUPWEIGHT_ 327
#define _SET_GROUPWEIGHT_ 328
int main()
{
	pid_t pid = getpid(); 
	gid_t gid = getgid(); // get the real group id of calling process
	gid_t egid = getegid(); 
	int g_weight;

	printf("Mary with pid %d gid %d egid %d\n",pid,gid,egid);
	if(setgid(2234) != 0)
	{
		printf("ERROR: %s\n",strerror(errno));
	}

	struct sched_param param;
	param.sched_priority = 0;
	if(sched_setscheduler(getpid(), SCHED_GWRR, &param) == -1)
		sprintf(stderr,"set to GRWW error: %s\n");

        g_weight = syscall(_GET_GROUPWEIGHT_, 2234);
        printf("group weight is %d\n",g_weight);
        g_weight = syscall(_SET_GROUPWEIGHT_, 2234,30);
        printf("set group weight to %d\n",g_weight);
        g_weight = syscall(_GET_GROUPWEIGHT_, 2234);
        printf("updated group weight is %d\n",g_weight);


	pid = getpid(); 
	gid = getgid(); // get the real group id of calling process
	egid = getegid(); 
	printf("Mary with pid %d gid %d egid %d\n",pid,gid,egid);

	while(1){}
	return 0;
}
