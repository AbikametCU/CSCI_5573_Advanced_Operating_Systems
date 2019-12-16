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
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define SYS_ENTRY 308
#define SYS_ENTRY_VERIFY 307
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))
#define SYSCALL_TABLE_SIZE 400
#define SYSCALL_TABLE_EMPTY_ADDRESS 0xffffffff810a1170

#define FILE_NAME "syscall_entries.txt"

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

void **sys_call_table;
void *sys_call_table_empty_entry;
void *sys_call_table_snapshot[SYSCALL_TABLE_SIZE];
unsigned long long sys_call_table_current[SYSCALL_TABLE_SIZE];
int sys_call_already_recorded = 0;

struct file *file_open(int flags) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(FILE_NAME, flags, 640);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) {
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, long sys_call_snapshot[]) 
{
    printk("read 1");
    mm_segment_t oldfs;
    printk("read 2");
    int ret;
    int size = 17;
    char str[17];
    char str2[17];

    oldfs = get_fs();
    set_fs(get_ds());
    int i;
    printk("passed in data[i]:%lld",sys_call_snapshot[i]);
    for (i = 0; i < SYSCALL_TABLE_SIZE; i++){
    	printk("read 3 for loop %d \n",i);

	
	//ret += file->f_op->read(file, data[i], size,&file->f_pos);
	ret += vfs_read(file, str, size,&offset);
	printk("ret value");
	printk("str value:%s",str);
	//memcpy(&data[i],str,strlen(data[i]));
//	printk("address %d read from file %s \n",i,data[i]);
   	// return ret;
	//Verify syscalls
//	sprintf(str2, "%llx\n", (char *)sys_call_table_current[i]);
//	if(strncmp(&str,data[i],17) != 0)
//	{
//		printk("Interception at syscall entry %d",i);
//	}
	memset(str, 0, 20);
    }

    
    return ret;

    set_fs(oldfs);
    printk("read 4");
    return ret;
}  

int file_write(struct file *file, unsigned long long offset, unsigned long long data[]) 
{
    printk("Entered file write\n");
    mm_segment_t oldfs;
    int ret = 0, i; 
    char str[20];
    char str_2[20];
    memset(str, 0, 20);
    int size = 17;

    oldfs = get_fs();
    set_fs(get_ds());
    for (i = 0; i < SYSCALL_TABLE_SIZE; i++){
	sprintf(str, "%ld\n", sys_call_table[i]);
    	ret += vfs_write(file,data[i],size, &offset);
	memset(str, 0, 20);
    }
    set_fs(oldfs);
    return ret;
} 

void verify_syscalls(void)
{	
	int fd;
	int i;
	for(i=0 ;i<SYSCALL_TABLE_SIZE;i++)
	{	
		//function_pointer = sys_call_table[i];
		//sys_call_table_snapshot[i] = function_pointer;	
		//printk("syscall table[%d] function pointer address:%p\n",i,sys_call_table_snapshot[i]);
		//function_address = (char *)function_pointer;		
		//sys_call_table_current[i] = sys_call_table_snapshot[i];
		if(memcmp(sys_call_table_snapshot[i], sys_call_table_current[i],20) != 0)
		{
			printk("syscall table has been intercepted\n");
			printk("verify_syscalls :sys_call_table_snapshot[i] :%p\n",i,sys_call_table_snapshot[i]);
			printk("verify_syscalls :sys_call_table_current[i] :%p\n",i,sys_call_table_current[i]);
			
		}
		printk("verify_syscalls : function pointer value:%p\n",i,sys_call_table_current[i]);
		printk("verify_syscalls : crossed file_read\n");	
	}
	printk("verify_syscalls : closed file\n");
//	sys_close(fd);
}

static int __init begin_module(void)
{
        printk(KERN_INFO "************************************NEW SYSCALL MODULE LOAD*************************************");
        sys_call_table = (void**)0xffffffff81a001c0;
	int i;
	//Recording current syscall_entries
	for(i=0;i<SYSCALL_TABLE_SIZE-1;i++)
	{	
		sys_call_table_current[i] = sys_call_table[i];
		printk("syscall table long array entry value %ld\n",sys_call_table[i]);
	}

	struct file *filp;
	filp = file_open(O_RDWR);

	if (filp == NULL)
	{
		printk("File open failed \n");

	}
	else
	{
		file_write(filp, 0, sys_call_table_current);	
		
		//	printk(KERN_INFO "Pending");
	}
        DISABLE;
        //sys_call_table[SYS_ENTRY_VERIFY] = (void*)
    //    file_read(filp,0,sys_call_table_snapshot);
        //sys_call_table[SYS_ENTRY_VERIFY] = (void*)verify_syscalls;
        ENABLE;
	file_close(filp);
        return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
        DISABLE;
        sys_call_table[SYS_ENTRY] =(void *)SYSCALL_TABLE_EMPTY_ADDRESS;
        ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

