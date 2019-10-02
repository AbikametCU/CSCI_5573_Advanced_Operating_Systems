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
		sprintf(bash_query,"cat %s | grep 'vm_layout_child' ",file_path);
		system(bash_query);
		return;
}

void print_heap_address(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'heap' >> heap_growth.txt ",file_path);
	system(bash_query);
	return;
}

void print_heap_address_on_console(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'heap' ",file_path);
	system(bash_query);
	return;
}

void print_stack_address(){
	char bash_query[60];
	sprintf(bash_query,"cat %s | grep 'stack' >> stack_growth.txt ",file_path);
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


void print_vm_layout(int argc, char **argv){

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
	char *p = malloc(sizeof(char)*200);
	printf("Address of dynamically allocated memory: %p\n",p);
	print_heap_address_on_console();

	//Heap growth
	heap_builder(20,20000);
	printf("program break:%p\n",sbrk(0));
	print_heap_address_on_console();

	//stack growth
	dummy_struct dummy = {100,'b',5454545,5454.505050};
	int x = stack_builder(dummy);

	return;
}


void foo(int argc, char **argv){
	int pid = fork();
	if(pid == 0 ){
		printf("**************************************Child process VM Layout**********************************************\n");
		print_vm_layout(argc,argv);
	}
}


int main(int argc, char **argv){
	printf("******************************************Parent process VM layout *************************************\n");
	print_vm_layout(argc,argv);
	foo(argc,argv);
	return 0;

}

/*


    561241d13000-561241d15000

	561241d13000-561241d15000

	56096b1e7000-56096b24c000

	56096b1e7000-56096b208000

*/