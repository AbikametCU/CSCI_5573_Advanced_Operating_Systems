#include<stdio.h>
#include<stdlib.h>
#include <time.h>
struct procinfo
{
        pid_t pid; /* Process ID */
        pid_t ppid; /* Parent process ID */
        struct timespec start_time; /* Process start time */
        int num_sib; /* Number of siblings */
};

double  start_time;
double  end_time;
double  time_taken;

double getTimeMsec(void)
{
  struct timespec event_ts = {0, 0};
  clock_gettime(CLOCK_REALTIME, &event_ts);
  return ((event_ts.tv_sec)*1000.0) + ((event_ts.tv_nsec)/1000000.0);
}

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
	start_time = getTimeMsec();		
	retval = syscall(304,pid,dummy);
	end_time = getTimeMsec();
	time_taken = end_time-start_time;
	if(retval != 0)
	{
		printf("error retval:%d\n",retval);
		exit(1);
	}
	printf("proc.pid:%d\n",dummy->pid);
	printf("proc.ppid:%d\n",dummy->ppid);
	printf("proc.num_sib:%d\n",dummy->num_sib);
	printf("time taken by system call: %lf",time_taken);
	printf("retval:%d\n",retval);
	return 0;
}
