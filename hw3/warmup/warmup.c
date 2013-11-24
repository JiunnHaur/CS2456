#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include "hrtimer_x86.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#define MAX_ITERATION 1000
#define READ_END 0
#define WRITE_END 1
double moment[4];
int pipe_one[2],pipe_two[2];
char msg[] = "M";
void *thread_read();
double c_time;

int main()
{
    //This part is to force a singe CPU execution
    //With this configuration, this program will force a context switch 
    //when executing the thread function
    cpu_set_t mask;
    CPU_ZERO(&mask);
    int a=sched_setaffinity(0,sizeof(mask),&mask);
    pthread_t child_thread;
    int i;
    double total_interval = 0;
    char readbuf[2];
    double start, end;
    
    memset(readbuf,0,sizeof(readbuf));
    memset(moment,0,sizeof(moment));
    pipe(pipe_one);
    pipe(pipe_two);

    pthread_create(&child_thread, NULL, thread_read, NULL);

    //This is a iteration for read/write procedure
    //In each iteration, the main process write to pipe_one
    //even the thread has been created at this time it will 
    //wait for the parent to finish writing because it is 
    //blocked by the read();
    //
    //The main process will be blocked by its read() after 
    //it finishes the first write()
    //
    //Therefore there should be a iteration in the thread function 
    //that has exactly the same number of iterations
    start = gethrtime_x86();
    for(i=0;i<MAX_ITERATION;i++)
    {
        if(write(pipe_one[WRITE_END],msg,2) == -1)
            printf("parent write fail\n");
        #ifdef DEBUG
            printf("parent sent msg: %s\n",msg);
        #endif

        if(read(pipe_two[READ_END],readbuf,2) == -1)
            printf("parent read fail\n");
        
        #ifdef DEBUG
            printf("parent read msg: %s\n", readbuf);
        #endif
    }
    end = gethrtime_x86();
    //Because I did enought times of iteration, the erroe in 
    //calling the hrtimer and other constant time cost will be
    //reduced by the number of iterations times.
    total_interval = ((end - start)-(c_time))/MAX_ITERATION; 
    if(pthread_join(child_thread,NULL) != 0)
            printf("pthread_join fail\n");
    printf("Context switch time is %e second(s)\n", total_interval/2);
    return 0;
}

void *thread_read()
{
    //child reads from pipe_one
    //child writes to pipe_two
    char buffer[2];
    memset(buffer,0,sizeof(buffer));
    double c_read_done, c_read_start;
    int i=0;
    c_time = 0;
    for(i=0;i<MAX_ITERATION;i++)
    {
        c_read_start = gethrtime_x86(); 
        if(read(pipe_one[READ_END],buffer,2) == -1)
            printf("child read fail\n");
        c_read_done = gethrtime_x86(); 
        #ifdef DEBUG
            printf("child receive msg %s\n",buffer);
        #endif
        if(write(pipe_two[WRITE_END],buffer,2) == -1 )
            printf("child write fail\n");
        #ifdef DEBUG
            printf("child sent msg %s\n", buffer);
        #endif
        //This c_time here is used to calculate the 
        //average read time of child thread, 
        //which will be deduced from the total time
        //of the entire read and write process.
        c_time += c_read_done - c_read_start;
    }
}
