#include<stdio.h>
#include<stdlib.h>

struct procinfo
{
        pid_t pid; /* Process ID */
        pid_t ppid; /* Parent process ID */
        struct timespec start_time; /* Process start time */
        int num_sib; /* Number of siblings */
};

int main(int argc,char **argv)
{
	struct procinfo *dummy;
	int pid;
	int retval;
	dummy = (struct procinfo *)malloc(sizeof(struct procinfo));
	if(argc<2)
	{
		pid = 0;
	}
	else
	{
		pid = atoi(argv[1]);
	}
	printf("PID:%d\n",getpid());
	printf("PID passed to system call:%d\n",pid);	
	retval = syscall(304,pid,dummy);
	if(retval != 0)
	{
		printf("error retval:%d\n",retval);
		exit(1);
	}
	printf("proc.pid:%d\n",dummy->pid);
	printf("proc.ppid:%d\n",dummy->ppid);
	printf("proc.num_sib:%d\n",dummy->num_sib);
	printf("retval:%d\n",retval);
	return 0;
}
