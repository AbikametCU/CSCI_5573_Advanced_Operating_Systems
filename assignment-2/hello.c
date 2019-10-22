#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

int init_module(void)
{
printk(KERN_ALERT "Hello world: This is Abikamet speaking from the Kernel \n");
return 0;
}

void cleanup_module(void)
{
	printk(KERN_ALERT "Goodbye from Abikamet,I am exiting the Kernel \n");
}

