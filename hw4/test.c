#include <linux/sched.h>
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#define SCHED_GWRR 4
int main()
{
	struct sched_param param;
	param.sched_priority = 0;
	if(sched_setscheduler(getpid(), SCHED_GWRR, &param) == -1)
		sprintf(stderr,"set to GRWW error: %s\n");

	if(sched_setscheduler(getpid(), SCHED_NORMAL, &param) == -1)
		sprintf(stderr,"set to NORMAL error: %s\n");

	return 0;
}
