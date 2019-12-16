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
long sys_call_table_current[SYSCALL_TABLE_SIZE];

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

void file_close(struct file *file)
{
    filp_close(file, NULL);
}

int file_write(struct file *file, unsigned long long offset,long data[])
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
	printk("file_write: data: %p\n",data[i]);
        //fprintf(file,"%llu",data[i]);
        ret += vfs_write(file,data[i],17, &offset);
        //memset(str, 0, 20);
    }
    set_fs(oldfs);
    return ret;
}


static int __init begin_module(void)
{
  	printk(KERN_INFO "************************************NEW SYSCALL MODULE LOAD*************************************");
	sys_call_table = (void**)0xffffffff81a001c0;
	int i;

	for(i=0;i<SYSCALL_TABLE_SIZE-1;i++)
        {
                sys_call_table_current[i] =(long)sys_call_table[i];	
                printk("syscall table long array entry value %lld\n",sys_call_table_current[i]);
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
                printk(KERN_INFO "Pending");
        }
	
	return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
        DISABLE;
       // sys_call_table[SYS_ENTRY] =(void *)SYSCALL_TABLE_EMPTY_ADDRESS;
        ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

