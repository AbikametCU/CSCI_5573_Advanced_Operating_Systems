#include<stdio.h>
#include<stdlib.h>


#define FILE_NAME /home/abikamet/syscall_snapshot.txt


void *syscallsnapshot[700];

int main()
{
	system("sudo insmod ../2-c/syscall_recorder.ko");	
	int fd;
	fd = fopen(FILE_NAME,"w+");
	int i;
	for(i=0;i<700;i++)
	{

		fread(fd);
	}

	fclose(fd);
	system("sudo rmmod ../2-c/syscall_recorder.ko");	
	return 0; 
}
