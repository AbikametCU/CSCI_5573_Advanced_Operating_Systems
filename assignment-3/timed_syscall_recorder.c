#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/dirent.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/fcntl.h>
#include <asm/errno.h>
#include <linux/kernel.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/init.h>
#include <linux/errno.h> 
#include <linux/types.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>  
#include <asm/page.h>  
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/mutex.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>

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

int g_time_interval = 5000;
struct timer_list syscall_timer;

struct tty_struct *tty;

static struct kprobe kp;

static void printString(void);

void time_handler(unsigned long data)
{
	printk("invoked from time handler");
	printString();
	mod_timer(&syscall_timer,jiffies+msecs_to_jiffies(g_time_interval));	
}

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
    for (i = 0; i < SYSCALL_TABLE_SIZE; i++)
    {
	printk("file_write: data: %p\n",data[i]);
        //fprintf(file,"%llu",data[i]);
        ret += vfs_write(file,data[i],17, &offset);
        //memset(str, 0, 20);
    }
    set_fs(oldfs);
    return ret;
}

static void print_dump_state(struct pt_regs *regs){
	print_symbol(KERN_INFO "IP: %s\n", regs->ip);
	printk(KERN_INFO "rip: %016lx\nrax: %016lx\nrbx: %016lx\nrcx: %016lx\nrdx: %016lx\n", regs->ip, regs->ax, regs->bx, regs->cx, regs->dx);
	printk(KERN_INFO "rsi: %016lx\nrdi: %016lx\nrbp: %016lx\nrsp: %016lx\n", regs->si, regs->di, regs->bp, regs->sp);
	//printk(KERN_INFO "ds: %04x es: %04x\n", regs->xds & 0xffff, regs->xes & 0xffff);
	printk(KERN_INFO "Process %s\npid: %d\nthreadinfo= %p\ntask= %p\n", current->comm, current->pid, current_thread_info(), current);
}



static void printString(void)
{


	char *module_print_message = "printed to terminal from kernel module\n";
	if(tty != NULL)
	{
		(tty->driver->ops->write)(tty,module_print_message,strlen(module_print_message));
	}
	else
	{
		printk("tty was NULL");
	}
}

int pre_kdump(struct kprobe *p, struct pt_regs *regs){ 
	printk("dump state before executing instructions!\n");
	print_dump_state(regs);
	return 0; 
} 
 
int  post_kdump(struct kprobe *p, struct pt_regs *regs, unsigned long flags){ 
	printk("dump state after execution instruction!\n");
	print_dump_state(regs);
	return 0;
} 

static int __init begin_module(void)
{
	init_timer(&syscall_timer);
	syscall_timer.expires = jiffies + HZ;
	syscall_timer.function = time_handler;
	syscall_timer.data = NULL;
	add_timer(&syscall_timer);
	mod_timer(&syscall_timer,jiffies+msecs_to_jiffies(g_time_interval));


	kp.pre_handler = pre_kdump; 
	kp.post_handler = post_kdump; 
	kp.addr = (kprobe_opcode_t *)printString; 
	register_kprobe(&kp); 

	
	tty = get_current_tty();
 	printString();	
	return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
	del_timer(&syscall_timer);
        DISABLE;
       // sys_call_table[SYS_ENTRY] =(void *)SYSCALL_TABLE_EMPTY_ADDRESS;
        ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

