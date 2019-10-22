#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
	syscall(60);
	return 0;
}