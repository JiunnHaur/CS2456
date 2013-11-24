#include <stdio.h>
#include "hrtimer_x86.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#define MAX_ITERATION 10
#define READ_END 0
#define WRITE_END 1
struct region {
    long long moment[4];
};
struct region *rptr;

int main()
{
    int fd;
    fd = shm_open("/myregion", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(fd == -1)
        perror("Open region error: %s\n");

    if(ftruncate(fd,sizeof(struct region)) == -1)
        perror("Arrange space error: %s\n");
    //map address into parent process
    rptr = mmap(NULL, sizeof(struct region),
                    PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0); 
    double total_interval;
    int i;
    {
        int pipe1[2],pipe2[2];
        pipe(pipe1);
        pipe(pipe2);
        pid_t child_id;
        child_id = fork();
        if(child_id)
        {
            //parent process
            //parent writes to pipe1
            //paretn reads from pipe2
            close(pipe1[READ_END]);
            close(pipe2[WRITE_END]);
            char msg[] = "M";
            char readbuf[4];
            write(pipe1[WRITE_END],msg,strlen(msg));
            #ifdef DEBUG
                printf("parent sent msg: %s\n",msg);
            #endif
            long long  p_write_done = gethrcycle_x86();
            read(pipe2[READ_END],readbuf,sizeof(readbuf));
            long long p_read_done = gethrcycle_x86();
            #ifdef DEBUG
                printf("parent read from pipe result: %s\n", readbuf);
            #endif
            rptr->moment[0]=p_read_done;
            rptr->moment[1]=p_write_done;
        }
        else if(child_id == 0) 
        {
            //child process
            //child reads from pipe1
            //child writes to pipe2
            close(pipe1[WRITE_END]);
            close(pipe2[READ_END]);
            char buffer[4];
            read(pipe1[READ_END],buffer,sizeof(buffer));
            long long c_read_done = gethrcycle_x86(); 
            #ifdef DEBUG
                printf("child receive msg %s\n",buffer);
            #endif
            write(pipe2[WRITE_END],buffer,strlen(buffer));
            long long c_write_done = gethrcycle_x86(); 
            #ifdef DEBUG
                printf("child sent msg %s\n", buffer);
            #endif
            rptr = mmap(NULL, sizeof(struct region),
                    PROT_READ |PROT_WRITE, MAP_SHARED,fd, 0); 
            rptr->moment[2]=c_read_done;
            rptr->moment[3]=c_write_done;
            exit(0);
        }else
        {
            perror("fork error");
            exit(1);
        }
    }
    long long pr, pw, cr, cw;
    double cyc_per_usec = getMHZ_x86();
    pr = rptr->moment[0];
    pw = rptr->moment[1];
    cr = rptr->moment[2];
    cw = rptr->moment[3];
    total_interval = ((cr - pw) + (pr - cw)) / cyc_per_usec / 1000; // ms
    printf("Context switch time is %f mili second(s)\n", total_interval/2);
    return 0;
}
