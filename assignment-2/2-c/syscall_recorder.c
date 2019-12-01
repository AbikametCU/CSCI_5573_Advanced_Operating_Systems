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

#define SYS_ENTRY 308
#define SYS_ENTRY_VERIFY 307
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))
#define SYSCALL_TABLE_SIZE 546

#define FILE_NAME "/home/abikamet/syscall_entries.txt"

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

void **sys_call_table;
void *sys_call_table_snapshot[SYSCALL_TABLE_SIZE];
void *sys_call_table_current[SYSCALL_TABLE_SIZE];
int sys_call_already_recorded = 0;

struct file *file_open(const char *path, int flags, int rights) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) 
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}  


int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    printk("Entered file write\n");
    printk("write arguments: %s\n",data);
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
} 

void verify_syscalls(void)
{	
	int fd;
	struct file *filp;
//	fd = open(FILE_NAME,O_RDONLY|O_WRONLY|O_CREAT,0640);
	filp  = file_open(FILE_NAME,O_RDONLY|O_WRONLY|O_CREAT,0640);
	int i;
	void *function_pointer;
	char *function_address;
	for(i=0;i<SYSCALL_TABLE_SIZE;i++)
	{	
		//function_pointer = sys_call_table[i];
		//sys_call_table_snapshot[i] = function_pointer;	
		//printk("syscall table[%d] function pointer address:%p\n",i,sys_call_table_snapshot[i]);
		//function_address = (char *)function_pointer;		
		file_read(filp,0,sys_call_table_snapshot[i],17);
		//sys_call_table_current[i] = sys_call_table_snapshot[i];
		if(strcmp(sys_call_table_snapshot[i],sys_call_table_current[i]) != 0)
		{
			printk("syscall table has been intercepted\n");
			printk("verify_syscalls :sys_call_table_snapshot[i] :%p\n",i,sys_call_table_snapshot[i]);
			printk("verify_syscalls :sys_call_table_current[i] :%p\n",i,sys_call_table_current[i]);
			
		}
		printk("verify_syscalls : function pointer value:%p\n",i,sys_call_table_current[i]);
		printk("verify_syscalls : crossed file_read\n");	
	}
//	close(fd);
	file_close(filp);
	printk("verify_syscalls : closed file\n");
//	sys_close(fd);
}


void record_syscalls(void)
{	
	int fd;
	struct file *filp;
//	fd = open(FILE_NAME,O_RDONLY|O_WRONLY|O_CREAT,0640);
	filp  = file_open(FILE_NAME,O_RDONLY|O_WRONLY|O_CREAT,0640);
	int i;
	void *function_pointer;
	char *function_address;
	for(i=0;i<SYSCALL_TABLE_SIZE-1;i++)
	{	
		function_pointer = sys_call_table[i];
		sys_call_table_current[i] = function_pointer;	
		printk("record_syscalls : syscall table[%d] function pointer address:%p\n",i,sys_call_table_current[i]);
		function_address = (char *)function_pointer;
		if(sys_call_table_current[i] == NULL)
		{
			printk("record_syscalls Entered Null snapshot entry condition\n");
			file_write(filp,0,"NULL",strlen("NULL"));		
		}
		else
		{
			
			file_write(filp,0,function_address,17);	
		}
		
		printk("record_syscalls string function pointer value:%p\n",i,sys_call_table_current[i]);
		printk("record_syscalls crossed typecasting\n");	
		printk("record_syscalls crossed file_write\n");	
	}
//	close(fd);
	file_close(filp);
	printk("closed file\n");
//	sys_close(fd);
}

static int __init begin_module(void)
{
        printk(KERN_INFO "************************************NEW SYSCALL MODULE LOAD*************************************");
        sys_call_table = (void**)0xffffffff81a001c0;
        DISABLE;
        sys_call_table[SYS_ENTRY] = (void*)record_syscalls;
        sys_call_table[SYS_ENTRY_VERIFY] = (void*)verify_syscalls;
        ENABLE;
        return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
        DISABLE;
        sys_call_table[SYS_ENTRY] = NULL;
        ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

