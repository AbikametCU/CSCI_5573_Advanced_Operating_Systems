#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/highmem.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
//#include <asm/semaphore.h>
#include <asm/cacheflush.h>

//#include <sys/syscall.h>

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

#define SYS_EXIT_GROUP_ENTRY 60
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))
//extern void *sys_call_table[];
//SYSCALL INTERCEPT:
int level;

void **sys_call_table; 
asmlinkage long (*original_sys_exit)(int);
/*
int make_rw(unsigned long add)
{
	printk("SYSCALL INTERCEPT:KERN_ALERT before set_memory_rw in make_rw");
	//set_memory_rw ((long unsigned int)sys_call_table,1);
	pte_t *pte = lookup_address(add, &level);
	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
	return 0;
}

int make_ro(unsigned long address)
{
	printk("SYSCALL INTERCEPT:KERN_ALERT before set_memory_ro in make_ro");
	//set_memory_ro ((long unsigned int)sys_call_table,1);
	 
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	return 0;
}
*/
asmlinkage long our_fake_exit_function(int error_code)
{
	/*print message on console every time we
	*are called*/
	printk("SYSCALL INTERCEPT:HEY! this is Abikametkamets custom exit call sys_exit called with error_code=%d\n",
	error_code);
	/*call the original sys_exit*/
	return original_sys_exit(error_code);
}

// int init_module()
// {
// 		make_rw(sys_call_table);

// 	/*store reference to the original sys_exit*/
// 	original_sys_exit=sys_call_table[__NR_exit];
// 	manipulate sys_call_table to call our
// 	fake exit function instead of sys_exit
// 	sys_call_table[__NR_exit]=our_fake_exit_function;
// 	return 0;
// }


static int __init begin_module(void)
{
	//printk("SYSCALL INTERCEPT:Before make_rw:\n");
	sys_call_table = (void**)0xffffffff81e00240;
	DISABLE;
	//make_rw((void*)sys_call_table);
	/*store reference to the original sys_exit*/
	//printk("SYSCALL INTERCEPT:Before original_sys_exit:\n");

	original_sys_exit=((void**)sys_call_table)[SYS_EXIT_GROUP_ENTRY];
	//printk("original sys exit address:%p",original_sys_exit);
	//printk("SYSCALL INTERCEPT:Before sys_call_table:\n");

	sys_call_table[SYS_EXIT_GROUP_ENTRY] = (void*)&our_fake_exit_function;
	//printk("our fake sys exit address:%p", our_fake_exit_function);
	//printk("syscall exit call entry value:%p",*(sys_call_table + SYS_EXIT_GROUP_ENTRY));
	//printk("SYSCALL INTERCEPT:Before make_ro:\n");
	ENABLE;
	//make_ro((void*)sys_call_table);
	return 0;
}

static void exit_module(void)
{
	DISABLE;
	//make_rw((unsigned long)sys_call_table);
	sys_call_table[SYS_EXIT_GROUP_ENTRY] =(void*) original_sys_exit;
	//make_ro((unsigned long)sys_call_table);
	ENABLE;
}


module_init(begin_module);
module_exit(exit_module);

/*int set_page_rw(long unsigned int _addr)
{
    return set_memory_rw(_addr, 1);
}

int set_page_ro(long unsigned int _addr)
{
    return set_memory_ro(_addr, 1);
}*/

// void cleanup_module()
// {
// 	make __NR_exit point to the original
// 	sys_exit when our module is unloaded
// 	sys_call_table[__NR_exit]=original_sys_exit;
// 	return;
// }
