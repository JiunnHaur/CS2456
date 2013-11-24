#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h> 
#include <linux/spinlock.h>
#include <stdlib.h>

#define _DOEVENT_OPEN_ 327
	//int doevent_open()
#define _DOEVENT_CLOSE_ 328
	//int doevent_close(int eventID)
#define _DOEVENT_WAIT_ 329
	//int doevent_wait(int eventID)
#define _DOEVENT_SIG_ 330 
	//int doevent_sig(int eventID)

int main(int argc, char *argv[])
{
	syscall(_DOEVENT_OPEN_);
	syscall(_DOEVENT_OPEN_);
	int i=0;
	for(i=0;i<5;i++)
	{
		if(fork() == 0)
		{
			//child process
			if (i<3)
			{
				printf("%d is going to sleep\n",i);
				syscall(_DOEVENT_WAIT_,0);
				printf("%d is awake\n",i);
			}
			else
			{
				printf("%d is going to sleep\n",i);
				syscall(_DOEVENT_WAIT_,1);
				printf("%d is awake\n",i);
			}
			printf("child %d exit\n",i);
			return 0;
		}
	}
	sleep(2);
	for(i=0;i<6;i++)
	{
		syscall(_DOEVENT_CLOSE_,i);
	}
	return 0;
}
