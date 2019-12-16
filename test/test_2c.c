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
	printf("PID:%d\n",getpid());	
	syscall(308);
//	syscall(307);
	return 0;
}
