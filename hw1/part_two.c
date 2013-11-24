#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#define MAXLINE 1024    // Maximum length of a command
#define MAX_CMD_NUM 8   // Max number of commands in a pipeline
#define MAX_ARG_NUM 64  // Max number of arguments for each command
#define MAX_ARG_LENGTH 256  //Max lenth of each argument
#define WRITE_END 1
#define READ_END 0

#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

//Global Varialbe
char prompt[] = "CSC2/456Shell$ ";    // command line prompt (DO NOT CHANGE) 
char cmds[MAX_CMD_NUM][MAXLINE];    //Array of commands 
int num_cmds;
int child_pids[MAX_CMD_NUM];
int child_status[MAX_CMD_NUM];
int nextjid = 1;            /* next job ID to allocate */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
    int pid_q[MAX_CMD_NUM];
    int pid_q_length;
};
struct job_t jobs[MAXJOBS]; /* The job list */

//Declare Functions
void eval_cmd();

int parse_user_input(char user_input[MAXLINE]);

int parse_cmds(char *cmd, char *name, char *arg_v[MAX_ARG_NUM]);

void delete_bg_character(char *instr);

int is_pipe_bg(char *userinput);

void child_handler();

void waitfg(struct job_t *pjob);
//Helper routines
//From  /u/cs456/src/sh-skeleton.c
void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void sigint_handler(int signum)
{//stop current fg program
	
}
void sigtstp_handler(int signum)
{//suspend current fg

}
int main()
{
	signal(SIGINT,  sigint_handler);   /* ctrl-c */
	signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
	signal(SIGCHLD, child_handler);

while (1) {
        char cmdline[MAXLINE];
        /* Read command line */
        fflush(stdout);
        printf("CSC2/456Shell$ ");

        if ((fgets(cmdline, MAXLINE, stdin) == NULL))
            printf("error\n");

        int cnt_pipe_bg = 0;
        cnt_pipe_bg = is_pipe_bg(cmdline);
    
        if(cnt_pipe_bg){
            delete_bg_character(cmdline);
        }
        /* Parse user input */
        num_cmds = parse_user_input(cmdline);   
       
       
        /* Evaluate the command line */
        /* Spawn child process to execute command*/
        if(strncmp(cmds[0],"exit",strlen("exit")) == 0){
            //exit shell
            printf("Exit shell!\n");
            exit(0);
        }else if(strncmp(cmds[0],"cd",strlen("cd")) == 0){
            //change current working directory
            char name[MAX_ARG_LENGTH];
            char *arg_v[MAX_ARG_NUM];
            parse_cmds(cmds[0],name,arg_v);
            chdir(arg_v[1]);
            continue;
        }else if(strncmp(cmds[0],"fg",strlen("fg")) == 0){
            char name[MAX_ARG_LENGTH];
            char *arg_v[MAX_ARG_NUM];
            parse_cmds(cmds[0],name,arg_v);
            //TODO 
            //use jobid to find thread id and bring up process to foreground;
            struct job_t *pcnt_job;
            int jid = atoi(arg_v[1]);
            pcnt_job = getjobjid(jobs,jid);
            #ifdef DEBUG
                printf("find job %d, with pid:%d\n",jid,pcnt_job->pid);
            #endif
            waitfg(pcnt_job);
            continue;
        }else if(strncmp(cmds[0],"bg",strlen("bg")) == 0){
            continue;
        }else if(strncmp(cmds[0],"jobs",strlen("jobs")) == 0){
            //show jobs status
            listjobs(jobs);
	    continue;
        }
        else{
            eval_cmd();
	    //set up groupid int the above eval_cmd() function
        }
        
        if(cnt_pipe_bg){
            //TODO
            //wait for all jobs that is in background???
            //addthing to list 
            //use WNOHANG to get statues of all child process
            //and then kill all bg process that are terminated
            //already
            addjob(jobs,child_pids[0],BG,cmdline);
            #ifdef DEBUG
                listjobs(jobs);
            #endif
            continue;
        }
        else{
		addjob(jobs,child_pids[0],FG,cmdline);
		int i;
		for(i=0;i<num_cmds;i++){
			waitpid(child_pids[i],&child_status[i],0);
		}
        }
    } 
    return 0;
}
void delete_bg_character(char *instr){
    int i=0;
    for(i=strlen(instr)-1;i>=0;i--){
        char c = instr[i];
        if(c == '&'){
            instr[i] = ' ';
            break;
        }
    }
}
int is_pipe_bg(char *userinput){
    int i=0;
    for(i=strlen(userinput)-1;i>=0;i--){
        char c = userinput[i];
        if(c == '&'){
            return 1;
            break;
        }
    }
    return 0;
}

// parse user input 
// format the command
// so that the final string is only seperated by ' '
// store the result in golbal cmds arrary
int parse_user_input(char user_input[MAXLINE])
{
    char localcp_input[MAXLINE];
    strcpy(localcp_input, user_input);

    //take '|' away
    char *token;
    char *rest;
    char *ptr = localcp_input;
    int count = 0;
    while(token = strtok_r(ptr,"\n|",&rest) ){
        ptr = rest;
        strcpy(cmds[count++],token);
    }
    return count;
}

void eval_cmd()
{
    int i=0;
    int groupid = 0;
    for(i=0;i<num_cmds;i++){
   //n_cmd here intends to be the total number of all the user cmds in one pipeline list    
        int oldpipe[2],newpipe[2];
        pid_t pid;
        pipe(newpipe);
        pid = fork();
        child_pids[i] = pid;
        if(i == 0)
		groupid = pid; //record the first child's pid
	if(pid > 0){//parent process
	    close(newpipe[WRITE_END]);
            oldpipe[READ_END] = newpipe[READ_END];
            oldpipe[WRITE_END] = newpipe[WRITE_END];
        }else if(pid == 0){ //child process
	    if(setgid(groupid) == -1){
	    //	printf("setgid Error: %s\n",strerror(errno));
	    } //set groupid to the recorded number;
            //child precesses read from oldpipe and write newpipe;
            close(newpipe[READ_END]);
            if(i != 0 ){
                //if this is not the first cmd
                //redirect child stdin to oldpipe read.
                dup2(oldpipe[READ_END],0);
                close(oldpipe[READ_END]); 
            }
            if(i != num_cmds - 1 ){
                //if this is not the last cmd
                //redirect child stdout to newpipe write
                dup2(newpipe[WRITE_END],1);
                close(newpipe[WRITE_END]);
            }
            char name[MAX_ARG_LENGTH];
            char *arg_v[MAX_ARG_NUM];
            int n_arg;
            n_arg = parse_cmds(cmds[i],name,arg_v);       
            execvp(name,arg_v);
        }else{
            perror("Create child process fail.");
        }
    }//Finish executing cmds[];
}
//parse cmds[] entry into command name and arrary of arguments
//for this command entry
//store cmds into a privided argument array
int parse_cmds(char *cmd, char *name, char *arg_v[MAX_ARG_NUM])
{
    char localcpy[MAXLINE];
    strcpy(localcpy,cmd);
    
    char *ptr = localcpy;
    char *rest;
    char *token;
    
    int count = 0;
    while(token = strtok_r(ptr,"\t' ",&rest) ){
         arg_v[count] = (char *)malloc(strlen(token)*sizeof(char));
         strcpy(arg_v[count++],token);
         ptr = rest;
    }
    strcpy(name,arg_v[0]);
    arg_v[count] = NULL;
    return count;
}
void child_handler(){
    int i;
    for(i=0;i < MAXJOBS; i++){
        if(jobs[i].pid != 0){
		int j;
		for(j=0;j<jobs[i].pid_q_length;j++){
			int child_status;
			waitpid(jobs[i].pid_q[j],&child_status,WNOHANG);
			if(WIFEXITED(child_status) == 1){ 
			//child terminate
				kill(jobs[i].pid,SIGKILL);
				deletejob(jobs,jobs[i].pid);
			} 
		}
		 
        }
    } 
    
}
/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';

	int i;
	for(i=0;i<job->pid_q_length;i++){
		job->pid_q[i] = 0;
	}

    job->pid_q_length = 0;
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].state = state;
            jobs[i].jid = nextjid++;
	    jobs[i].pid_q_length = num_cmds;
            if (nextjid > MAXJOBS)
            nextjid = 1;
            strcpy(jobs[i].cmdline, cmdline);
	    //record all pids for each pipleline
	    int j=0;
	    for(j=0; j<jobs[i].pid_q_length;j++){
	    	jobs[i].pid_q[j] = child_pids[j];
	    }
            #ifdef DEBUG
                printf("Added job [%d] %d %s", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
		for(j=0;j<jobs[i].pid_q_length;j++){
			printf("Pid of %d cmd in this pipe is %d\n",j,jobs[i].pid_q[j]);
		}
            #endif 
                return 1;
        }
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            clearjob(&jobs[i]);
            nextjid = maxjid(jobs)+1;
            return 1;
        }
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid != 0) {
            printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
            switch (jobs[i].state) {
            case BG: 
                printf("Running ");
                break;
            case FG: 
                printf("Foreground ");
                break;
            case ST: 
                printf("Stopped ");
                break;
            default:
                printf("listjobs: Internal error: job[%d].state=%d ", 
                   i, jobs[i].state);
            }
            printf("%s", jobs[i].cmdline);
            printf("\n");
        }
    }
}
/******************************
 * end job list helper routines
 ******************************/

void waitfg(struct job_t *pjob){
    int i;
    int n = pjob->pid_q_length;
    for(i=0;i<n;i++){
        int child_status;
        int cnt_pid = pjob->pid_q[i];
        waitpid(cnt_pid,&child_status,0);
        if(WIFEXITED(child_status) == 1){ 
            kill(cnt_pid,SIGKILL);
            deletejob(jobs,cnt_pid);
        } 
    }
}
