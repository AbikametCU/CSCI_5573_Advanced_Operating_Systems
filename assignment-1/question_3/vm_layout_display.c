#include<stdio.h>
#include<stdlib.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<linux/module.h>
#include<sys/mman.h>

char file_path[30];

typedef struct{
	int a;
	char b;
	long c;
	double d;
}dummy_struct;


void print_exec_code_region(){
		char str[100];
		char delim[3] = " ";
 		char *token;
		char bash_query[100];
		sprintf(bash_query,"cat %s | grep 'vm_layout_display' ",file_path);
		system(bash_query);
		return;
}

void print_heap_address(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'heap' >> heap_growth.txt ",file_path);
	system(bash_query);
	return;
}

void print_stack_address(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'stack' >> stack_growth.txt ",file_path);
	system(bash_query);
	return;
}

void print_heap_address_on_console(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'heap' ",file_path);
	system(bash_query);
	return;
}

int stack_builder(dummy_struct dummy){
	int dummy_char[500] = {0,0,0,0,0,0,0};
	int i;
	for (i=0;i<500;i++){
		dummy_char[i] = 1;
	}
	if(dummy.a ==1 || dummy.a == 0){
		return 1;
	}
	dummy.a --;
	print_stack_address();
	return stack_builder(dummy);	 
}

void heap_builder(int no_of_times,int size_of_each_allocation){
	int i;

	for(i=0;i<no_of_times;i++){
		malloc(size_of_each_allocation);
		print_heap_address();
	}

	return;
}

int main(int argc, char **argv){

	int i;
	//remove stack_growth.txt and heap_growth.txt old contents
	system("rm stack_growth.txt");
	system("rm heap_growth.txt");

	sprintf(file_path,"/proc/%d/maps",getpid());
	printf("\nPID:%d\n",getpid());


	//argc and argv locations
	printf("argc:%p\n",&argc);	
	printf("argv:%p\n",(void *)argv);

	//address of command line arguments
	for(i=0;i<argc;i++){
		printf("[%s]:%p\n",argv[i],&argv[i]);
	}

	//Executable regions 
	print_exec_code_region();

	//program break
	printf("program break:%p\n",sbrk(0));

	//Address of first malloc variable
	char *p1 = malloc(sizeof(char)*200);
	printf("\nAddress of dynamically allocated memory: %p\n",p1);
	print_heap_address_on_console();
	
	
	char *p2 = malloc(sizeof(char)*200);
	printf("Address of second malloc variable:%p\n",p2);
	print_heap_address_on_console();
	printf("\n");

	double diff = (void *)p2 - (void*)p1;
	printf("malloc difference:%p\n",diff);
	print_heap_address_on_console();
	printf("\n");

	char *p3 = malloc(sizeof(char)*200);
	printf("Address of third malloc variable:%p\n",p3);
	print_heap_address_on_console();
	printf("\n");

    diff = (void *)p3 - (void*)p2;
	printf("malloc difference :%p\n",diff);
	printf("\n");
	//Heap growth
	heap_builder(20,20000);

	printf("program break:%p\n",sbrk(0));
	print_heap_address_on_console();

	//stack growth
	dummy_struct dummy = {100,'b',5454545,5454.505050};
	int x = stack_builder(dummy);

	return 0;
}


/*

gap between program break and heap
94794223054848
94523640115200
94124208156672

getting different gap values but not sure of it 


0x6fb2000
0x6ff6000

0x58afd000
0x532f0000
0x34162000

*/