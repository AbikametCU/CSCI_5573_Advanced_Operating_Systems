#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/highmem.h>
#include <linux/semaphore.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>

#define GETDENTS_SYSCALL_ENTRY 78

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

static unsigned long *sys_call_table = (unsigned long *)(0xffffffff81e00240);
asmlinkage unsigned long (*original_sys_getdents)(int);


int make_rw(unsigned long address)
{
	printk("SYSCALL INTERCEPT:KERN_ALERT before set_memory_rw in make_rw");
	//set_memory_rw ((long unsigned int)sys_call_table,1);

	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~ _PAGE_RW)
	pte->pte |= _PAGE_RW;
	return 0;
}

int make_ro(unsigned long address)
{
	printk("SYSCALL INTERCEPT:KERN_ALERT before set_memory_ro in make_ro");
	//set_memory_ro ((long unsigned int)sys_call_table,1);
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	return 0;
}

asmlinkage int our_fake_getdents_function(unsigned int fd, struct linux_dirent *dirp,unsigned int count)
{
	/*print message on console every time we
	*are called*/

	
	printk("SYSCALL INTERCEPT:HEY! this is Abikamets custom exit call sys_exit called with error_code=%d\n",
	error_code);
	/*call the original sys_exit*/
	return original_sys_getdents(error_code);
}

int init_module()
{
	printk("SYSCALL INTERCEPT:Before make_rw:\n");
	make_rw((unsigned long)sys_call_table);
	/*store reference to the original sys_exit*/
	printk("SYSCALL INTERCEPT:Before original_sys_exit:\n");
	original_sys_getdents=(void*)*(sys_call_table + GETDENTS_SYSCALL_ENTRY);
	printk("SYSCALL INTERCEPT:Before sys_call_table:\n");
	*(sys_call_table + GETDENTS_SYSCALL_ENTRY) = (unsigned long)our_fake_getdents_function;
	printk("SYSCALL INTERCEPT:Before make_ro:\n");
	make_ro((unsigned long)sys_call_table);	
}

void cleanup_module()
{
	make_rw((unsigned long)sys_call_table);
	*(sys_call_table + GETDENTS_SYSCALL_ENTRY) =
	(unsigned long)original_sys_getdents;
	make_ro((unsigned long)sys_call_table);	
}
