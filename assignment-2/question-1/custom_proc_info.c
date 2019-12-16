#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/highmem.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <linux/kallsyms.h>
#include <asm/special_insns.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/sched.h>

#define SYS_ENTRY 304
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

void **sys_call_table;
void *sys_call_empty_entry;

struct procinfo 
{
	pid_t pid; /* Process ID */
	pid_t ppid; /* Parent process ID */
	struct timespec start_time; /* Process start time */
	int num_sib; /* Number of siblings */
}; 


int getprocinfo(pid_t pid,struct procinfo *inf)
{

	if(inf == NULL)
	{
		return -EINVAL;
	}

	struct procinfo proc;
	struct task_struct *current_task = current;
	struct task_struct *task;
	struct task_struct *parent_task = current->parent;
	struct list_head *list;
	int sibling_count = 0;
	int process_found = 0;

	printk("PID value passed to getprocinfo:%d\n",pid);
	//pid checks 
	if(pid == 0)
	{
		proc.pid = current_task->pid;
		proc.ppid = parent_task->pid;
//		proc.start_time = task->start_time;
		list_for_each(list, &parent_task->children)
		{
  			task = list_entry(list, struct task_struct, sibling);
    			sibling_count++;
		}
		proc.num_sib = sibling_count-1;
	}
	else if(pid > 0)
	{
		for_each_process( task )
		{
			if(task->pid == pid)
			{
				parent_task = task->parent;	
				proc.pid = task->pid;
				proc.ppid = parent_task->pid;
				process_found = 1;
			}	
		}

		if(!process_found)
		{
			printk("process with pid:%d does not exist!\n",pid);
			return -EINVAL;
		}		
	}
	else
	{
		struct task_struct *grand_parent_task = parent_task->parent;
		proc.pid = parent_task->pid;
		proc.ppid = grand_parent_task->pid;
		list_for_each(list, &grand_parent_task->children)
		{
  			task = list_entry(list, struct task_struct, sibling);
    			sibling_count++;
		}
		proc.num_sib = sibling_count-1;
	}
	
		printk("parent proc.pid:%d\n",proc.pid);
		printk("parent proc.ppid:%d\n",proc.ppid);
		printk("proc.num_sib:%d\n",proc.num_sib);

	copy_to_user(inf,&proc,sizeof(struct procinfo));
	return 0;
	//error checks - negative pid?
}

static int __init begin_module(void)
{
        printk(KERN_INFO "************************************NEW SYSCALL MODULE LOAD*************************************");
	printk(KERN_INFO "function pointer address of getprocinfo(syscall number 304):%p",getprocinfo);	
        sys_call_table = (void**)0xffffffff81a001c0;
        DISABLE;
	sys_call_empty_entry = sys_call_table[SYS_ENTRY]; 
        sys_call_table[SYS_ENTRY] = (void*)getprocinfo;
        ENABLE;
        return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
        DISABLE;
        sys_call_table[SYS_ENTRY] = sys_call_empty_entry;
        ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

