#include<stdio.h>
#include<stdlib.h>
#include<linux/kernel.h>
#include<sys/syscall.h>
#include<linux/module.h>
#include<string.h>


#define PROC_STAT_FILE_PATH /proc/stat


typedef struct {
	double total_mem;
	double available_mem;
}memory_info;



typedef struct {
	double disk_sector_read;
	double disk_sector_write;
}disk_rw_sector_info;



typedef struct {
	double user_space_utilization;
	double kernel_space_utilization;
	double idle_time_percentage;
}processor_util_info;


void print_usage(){
	printf("usage: proc_parse print_interval(seconds) average_interval(seconds)");
}

void print_basic_version(){
	system("bash basic_kernel_info.sh");
}


disk_rw_sector_info get_disk_read_write_info(){
	disk_rw_sector_info disk_rw_info;
	char str[100];
	char delim[3] = " ";
 	char *token;
	char cmd[50];
	sprintf(cmd,"cat /proc/diskstats | grep 'sda '");
	FILE *file_ptr = popen(cmd,"r");
	fgets(str,100,file_ptr);
	printf("contents: %s\n",str);
	fclose(file_ptr);
	token = strtok(str,"       ");
	int i = 0;
	while(token !=NULL){
		printf("token:%s\n",token);
		token = strtok(NULL,delim);
		switch(i){
			case 4:
				disk_rw_info.disk_sector_read  = atof(token);
				break;
			case 8:
				disk_rw_info.disk_sector_write = atof(token);
				break;	
		}	
				
				i++;				
	}
	printf("disk_sector_read:%lf\n",disk_rw_info.disk_sector_read);
	printf("disk_sector_write:%lf\n",disk_rw_info.disk_sector_write);
	return disk_rw_info;

}


processor_util_info get_processor_util_stats(){
			processor_util_info proc_info;
			char str[100];
			char delim[3] = " ";
 			char *token;
			char cmd[50];
			double total_ticks,user_ticks,kernel_ticks,user_niced_ticks,idle_ticks,total_cpu_run_ticks;
			sprintf(cmd,"cat /proc/stat | grep 'cpu '");
			FILE *file_ptr = popen(cmd,"r");
			fgets(str,100,file_ptr);
			printf("contents: %s\n",str);
			fclose(file_ptr);
			token = strtok(str,delim);
			int i = 0;
			while(token !=NULL){
				printf("token:%s\n",token);
				total_ticks += atoi(token);
				token = strtok(NULL,delim);
				switch(i){
					case 0:
						user_ticks = atof(token);
						break;
					case 1:
						user_niced_ticks = atof(token);
						break;
					case 2:
						kernel_ticks = atof(token);
						break;
					case 3:
						idle_ticks = atof(token);
						break;
						
				}	
				
				i++;				
			}
			total_cpu_run_ticks = (user_ticks + user_niced_ticks+kernel_ticks+idle_ticks);
		 
			double user_space_utilization = ((user_ticks+user_niced_ticks)/total_cpu_run_ticks)*100;
			double kernel_space_utilization = ((kernel_ticks/total_cpu_run_ticks))*100;
			double idle_time_percentage = ((idle_ticks/total_cpu_run_ticks))*100;

			proc_info.user_space_utilization = user_space_utilization;
			proc_info.kernel_space_utilization = kernel_space_utilization;
			proc_info.idle_time_percentage = idle_time_percentage;
			/*
			printf("user space ticks:%lf \n",user_ticks);
			printf("kernel space ticks:%lf \n",kernel_ticks);
			printf("idle ticks:%lf\n",idle_ticks);
			printf("userspace utilization: %lf%\n",user_space_utilization);
			printf("kernel space utilization: %lf%\n",kernel_space_utilization);
			printf("idle time : %lf%\n",idle_time_percentage);	
			printf("total:%lf\n",total_ticks);
			*/
	return proc_info;
}


double get_memory_util_stats(){
			double available_mem_percentage;
			double total_mem, available_mem;
			char str[100];
			char delim[20] = "        ";
 			char *token;
			char cmd[50];
			// get total memory from file
			sprintf(cmd,"cat /proc/meminfo | grep 'MemTotal'");			
			FILE *file_ptr = popen(cmd,"r");
			fgets(str,100,file_ptr);
			printf("contents: %s\n",str);
			fclose(file_ptr);
			token = strtok(str,delim);
			token = strtok(NULL,delim);
			total_mem = atof(token);
			//printf("token: %s\n",token);

			// get available memory from file
			sprintf(cmd,"cat /proc/meminfo | grep 'MemAvailable'");			
			FILE *file_ptr_2 = popen(cmd,"r");
			fgets(str,100,file_ptr_2);
			printf("contents: %s\n",str);
			fclose(file_ptr_2);
			token = strtok(str,delim);
			token = strtok(NULL,delim);
			available_mem = atof(token);
			//printf("token: %s\n",token);



			available_mem_percentage = (available_mem/total_mem)*100;	
			return available_mem_percentage;
			
}



long get_no_of_context_switches(){
			char str[100];
			char delim[3] = " ";
 			char *token;
			char cmd[50];
			// get total memory from file
			sprintf(cmd,"cat /proc/stat | grep 'ctxt'");			
			FILE *file_ptr = popen(cmd,"r");
			fgets(str,100,file_ptr);
			printf("contents: %s\n",str);
			fclose(file_ptr);
			token = strtok(str,delim);
			token = strtok(NULL,delim);
			return atof(token);
}

long get_no_of_process_creations(){
			char str[100];
			char delim[3] = " ";
 			char *token;
			char cmd[50];
			// get total memory from file
			sprintf(cmd,"cat /proc/stat | grep 'processes'");			
			FILE *file_ptr = popen(cmd,"r");
			fgets(str,100,file_ptr);
			printf("contents: %s\n",str);
			fclose(file_ptr);
			token = strtok(str,delim);
			token = strtok(NULL,delim);
			return atof(token);
}


int main(int argc, char **argv){
	
	if(argc == 1){
		print_basic_version();
	}else{
		if(argc!=3){
			print_usage();
		}else{
			int print_interval = atoi(argv[1]);
			int average_interval = atoi(argv[2]);
			int samples = average_interval/print_interval;
			int i = 0;

			processor_util_info proc_info = {0,0,0};
			disk_rw_sector_info disk_rw_info;
			double context_switch_rate, process_creation_rate,disk_sector_read_rate,disk_sector_write_rate; 
			long prev_no_context_switches,curr_no_context_switches;
			long prev_no_process_creations,curr_no_process_creations;
			long prev_no_disk_reads, curr_no_disk_reads;
			long prev_no_disk_writes, curr_no_disk_writes;
			long available_mem_percentage;

			//for average statistics
			long total_context_switch_rate, total_process_creation_rate;
			double total_user_space_utilization, total_kernel_space_utilization;
			double total_idle_time_percentage;
			double total_available_mem_percentage;
			double total_disk_read_rate,total_disk_write_rate;

			prev_no_context_switches = get_no_of_context_switches();
			prev_no_process_creations = get_no_of_process_creations();
			disk_rw_info = get_disk_read_write_info();
			prev_no_disk_writes = disk_rw_info.disk_sector_write;
			prev_no_disk_reads  = disk_rw_info.disk_sector_read;

			while(1){

				usleep(print_interval*1000000);
				proc_info = get_processor_util_stats();
				disk_rw_info = get_disk_read_write_info();
				available_mem_percentage = get_memory_util_stats();
				curr_no_context_switches = get_no_of_context_switches();
				curr_no_process_creations = get_no_of_process_creations();
				curr_no_disk_reads = disk_rw_info.disk_sector_read;
				curr_no_disk_writes = disk_rw_info.disk_sector_write;
				context_switch_rate = curr_no_context_switches - prev_no_context_switches;
				process_creation_rate = curr_no_process_creations - prev_no_process_creations;
				disk_sector_read_rate = curr_no_disk_reads - prev_no_disk_reads;
				disk_sector_write_rate = curr_no_disk_writes - prev_no_disk_writes;
				
				total_user_space_utilization += proc_info.user_space_utilization;
				total_kernel_space_utilization += proc_info.kernel_space_utilization;
				total_idle_time_percentage += proc_info.idle_time_percentage;
				total_available_mem_percentage += available_mem_percentage;
				total_context_switch_rate += context_switch_rate;
				total_process_creation_rate += process_creation_rate;
				total_disk_read_rate += disk_sector_read_rate;
				total_disk_write_rate +=disk_sector_write_rate;

				//Print kernel structures
				printf("userspace utilization: %lf%\n",proc_info.user_space_utilization);
				printf("kernel space utilization: %lf%\n",proc_info.kernel_space_utilization);
				printf("idle time : %lf%\n",proc_info.idle_time_percentage);
				printf("available memory :%lf\n",available_mem_percentage);
				printf("process_creation_rate:%lf/second\n",process_creation_rate);
				printf("context_switch_rate:%lf/second\n",context_switch_rate);
				printf("process_creation_rate:%lf/second\n",process_creation_rate);
				printf("disk_sector_read_rate:%lf/second\n",disk_sector_read_rate);
				printf("disk_sector_write_rate:%lf/second\n",disk_sector_write_rate);
				

				prev_no_context_switches = curr_no_context_switches;
				prev_no_process_creations = curr_no_process_creations;
				prev_no_disk_reads = curr_no_disk_reads;
				prev_no_disk_writes = curr_no_disk_writes;
				i++;
				if(i%samples == 0){
					//print statistics of the collected samples
					printf("************************Average statistics!********************* \n");
					printf("average_userspace_utilization:%lf%\n"
						,total_user_space_utilization/samples);
					printf("average_kernelspace_utilization:%lf%\n"
						,total_kernel_space_utilization/samples);
					printf("average_idle_time percentage:%lf%\n"
						,total_idle_time_percentage/samples);
					printf("average_available_memory_space:%lf%\n"
						,total_available_mem_percentage/samples);
					printf("average_context_switch rate:%lf/second\n"
						,total_context_switch_rate/samples);
					printf("average_process_creation rate:%lf/second\n"
						,total_process_creation_rate/samples);
					printf("average_disk_read rate:%lf/second\n"
						,total_disk_read_rate/samples);
					printf("average_disk_write rate:%lf/second\n"
						,total_disk_write_rate/samples);

					//reset proc values
					total_user_space_utilization = 0;
					total_kernel_space_utilization = 0;
					total_idle_time_percentage = 0;
					total_available_mem_percentage = 0;
					total_context_switch_rate = 0;
					total_process_creation_rate = 0;
					total_disk_read_rate =  0;
					total_disk_write_rate = 0;
				}
				
			}			
		}		
	}

	return 0;
}


/*
	
	...............................
	disk_sector_read:2454554.000000
	disk_sector_write:6652040.000000

*/