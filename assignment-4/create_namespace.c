#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<sched.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
//#include<linux/module.h>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/wait.h>

static char child_stack[1048576];

static int child_fn()
{

	printf("PID:%ld\n",(long)getpid());
	printf("Parent PID:%ld\n",(long)getppid());
	system("ip link");

	system("cgexec -g blkio,memory,cpu:myapp chroot jailed bash");
	return 0;
}

int main()
{
	int flag;
	pid_t child_pid = clone(child_fn,child_stack+1048576,CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWNS | SIGCHLD,NULL);
	if(child_pid<0)
		perror("clone");
	printf("clone()=%ld\n",(long)child_pid);
	waitpid(child_pid,NULL,0);
	return 0;
}


