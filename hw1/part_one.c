#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#define MAX_SIZE 128

typedef struct {
	int n_sector_rw;
	int n_contex_s ;
	int n_process_c;
	int interval;
	int interval2;
}rate;
void printcpu();
void printmem();
void printuptime();
void *getinfo(void *data);
void *printinfo(void *data);
int main(int argc, char *argv[])
{
	if(argc == 1)
	{
		printcpu();
		printmem();
		printuptime();
		exit(1);
	}
	else
	{
		if(argc != 3) 
			printf("error!\n");
		int read_rate, printout_rate;
		read_rate = atoi(argv[1]);
		printout_rate = atoi(argv[2]);

		rate myrate;
		myrate.n_sector_rw = -1;
		myrate.n_contex_s = -1;
		myrate.n_process_c = -1;
		myrate.interval = read_rate*1000000;
		myrate.interval2 = printout_rate*1000000;
		pthread_t thread;
		pthread_t thread2;
		if(pthread_create(&thread,NULL,getinfo,&myrate) != 0){
			printf("create_thread error\n");
		}
		if(pthread_create(&thread2,NULL,printinfo,&myrate) != 0){
			printf("create_thread error\n");
		}
		while(1);
	}
	return 0;
}



void *getinfo(void *data)
{
	rate *prate=(rate*)data;
	//now read value in from /proc
	//first part is the # of read and write sections
	while(1){
		char line[MAX_SIZE];
		FILE *fp;
		int useless;	
		int n_read_sectors;
		int n_write_sectors;	
		char disk_name[20];

		fp = fopen("/proc/diskstats","r");
		if(fp == NULL)
			exit(0);
		while(fgets(line,MAX_SIZE,fp)){
			sscanf(line,"%*d %*d %s %*d %*d %d %*d %*d %*d %d %*d %*d %*d %*d",
				disk_name, &n_read_sectors, &n_write_sectors );

			if(strncmp(disk_name,"sda",strlen("sda")) == 0){
				//printf("found sda\n");
				break;
			}
			else{
				continue;
			}
		}
		fclose(fp);
		//printf("name: %s read: %d write: %d\n", disk_name,
		//n_read_sectors,
		//n_write_sectors);
		//update fields in the passed in prate structure
		if(prate->n_sector_rw == -1)
			prate->n_sector_rw = n_read_sectors + n_write_sectors;
		else 
			prate->n_sector_rw = n_read_sectors + n_write_sectors - prate->n_sector_rw;
		
		
		//---------------------------------------------------------//
		fp = fopen("/proc/stat","r");
		char str_name[20];
		int n_number;
		if(fp == NULL)
			exit(0);
		while(fgets(line,MAX_SIZE,fp)){
			sscanf(line,"%s %d",str_name,&n_number);

			if(strncmp(str_name,"ctxt",strlen("ctxt")) == 0){

				if(prate->n_contex_s == -1)
					prate->n_contex_s = n_number;
				else
					prate->n_contex_s = n_number - prate->n_contex_s;
			} 
			else if(strncmp(str_name,"processes",strlen("processes")) == 0 ){
				if(prate->n_process_c == -1)
					prate->n_process_c = n_number;
				else
					prate->n_process_c = n_number - prate->n_process_c;
			}else{
				continue;
			}
		}
		fclose(fp);

		//---------------------------------------------------------//

		#ifdef DEBUG
			/*printf("n_sector_rw == %d\n",prate->n_sector_rw);
			printf("n_contex_s == %d\n",prate->n_contex_s);
			printf("n_process_c == %d\n",prate->n_process_c);
			printf("interval == %d\n",prate->interval);*/
		#endif
	usleep(prate->interval);
	}
}

void *printinfo(void *data)
{
	rate *prate = (rate*)data;
	int count = 0;
	while(1){
		//find percentage of time
		usleep(prate->interval2);
		FILE* fp;
		char line[MAX_SIZE];
		float t_user,t_sys,t_idl;
		float sum,time_percent;
		char name[20];
		float number;
		float mem_total, mem_free;
		fp = fopen("/proc/stat","r");
		while( fgets(line,MAX_SIZE,fp) ){
			sscanf(line,"%s %f %f %f",name,&t_user,&t_sys,&t_idl);		
			if( strncmp(name,"cpu",strlen("cpu")) == 0 ){
				break;
			}
		}
		fclose(fp);
		sum = t_user + t_sys + t_idl;
		printf("User mode: %f\n",t_user/sum);
		printf("Kernel mode: %f\n",t_sys/sum);
		printf("Idle mode: %f\n",t_idl/sum);

		//fine percentage of memory
		fp = fopen("/proc/meminfo","r");
		while(fgets(line,MAX_SIZE,fp)){
			sscanf(line,"%s %f",name,&number);
			if( strncmp(name,"MemTotal:", strlen("MemTotal:"))== 0){
				mem_total = number;
			}	
			if( strncmp(name,"MemFree:", strlen("MemFree:"))== 0){
				mem_free = number;	
				break;
			}
		}
		
		printf("Percentage of free memory: %f\n", mem_free/mem_total);
		fclose(fp);
		printf("Read and write rate:%d\nContext switch rate:%d\nProcess creation rate:%d\n\n",prate->n_sector_rw,prate->n_contex_s,prate->n_process_c);
		
	}
}


void printcpu(){
	printf("CPU info:\n");
	FILE *fp;
	char *line=NULL;
	size_t len=0;
	ssize_t read;

	fp = fopen("/proc/cpuinfo","r");
	if(fp == NULL)
		exit(0);
	int j=0;
	while((read = getline(&line,&len,fp)) != -1 )
	{	
		const char target[]="model name	:";
		char part_one[MAX_SIZE];
		memset(part_one,0,sizeof(part_one));
		int pos = 0,i=0;
		for(i=0;i<strlen(line);i++)
		{
			char c = line[i];
			if(c == ':')
			{
				pos = i;
				break;
			}
		}
	//	printf("pos == %d\n",pos);
		strncpy(part_one,line,pos+1);
		part_one[pos+1] = '\0';
	
		//printf("'%s'\n",part_one);
		//printf("'%s'\n",target);
	//	printf("len of part_one: %d\n",strlen(part_one));
	//	printf("len of target: %d\n",strlen(target));

		if(strncmp(part_one,target,strlen(target))==0)
		{
		//	printf("match\n");
			printf("CPU%d: %s",j++,line);
		}
		else
		{
			continue;
		}
	}
	if(line)
		line = NULL;
	fclose(fp);
}

void printversion()
{
	FILE *fp;
	char *line=NULL;
	size_t len=0;
	ssize_t read;
	fp = fopen("/proc/version","r");
	if(fp == NULL)
		exit(0);
	printf("\nKernel Version:\n");	
	while((read = getline(&line,&len,fp)) != -1 )
	{
		printf("%s",line);
	}	

	fclose(fp);
}

void printmem()
{
	FILE *fp;
	char *line=NULL;
	size_t len=0;
	ssize_t read;
	fp = fopen("/proc/meminfo","r");	
	if(fp == NULL)
		exit(0);
	printf("\nTotal Amount of Memory is:\n");
	getline(&line,&len,fp);
	printf("%s",line);	
	fclose(fp);
}

void printuptime()
{
	FILE *fp;
	char *line=NULL;
	size_t len=0;
	ssize_t read;
	fp = fopen("/proc/uptime","r");
	if(fp == NULL)
		exit(0);
	printf("\nSystem uptime is:\n");
	getline(&line,&len,fp);
	char part_one[MAX_SIZE];
	int i=0,pos=0;
	for(i=0;i<len;i++)
	{
		char c = line[i];
		if(c == ' ')
		{
			pos = i;
			break;
		}
	}
	strncpy(part_one,line,pos);
	part_one[pos]= '\0';
	printf("%s\n",part_one);
	fclose(fp);
}
