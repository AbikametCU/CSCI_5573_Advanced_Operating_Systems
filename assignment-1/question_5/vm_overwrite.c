#include<stdio.h>
#include<stdlib.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<linux/module.h>
#include<string.h>


int main(){
	char *s;
	int i;
	s = strdup("This is my initial string!");
	printf("%s:%p\n",s,&s);
	printf("PID:%d\n",getpid());

		while (s)
     {
          printf("[%lu] %s (%p)\n", i, s, (void *)s);
          sleep(5);
          i++;
     }
	

	return 0;
}