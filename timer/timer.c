
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/timer.h>

struct timer_list tlist;
int count=0;

static void timer_dev_func(unsigned long data)
{
	printk(KERN_INFO "touch %d times \n", (*((int *)data))++);

	// 重新启动定时器
	tlist.expires = jiffies + HZ;
	add_timer(&tlist);
}

static int __init timer_dev_init(void)
{
	init_timer(&tlist);
	tlist.expires  = jiffies + HZ;
	tlist.data     = &count;
	tlist.function = &timer_dev_func;
	add_timer(&tlist);

	printk(KERN_INFO "timer added: 1HZ\n");

	return 0;
}

static void __exit timer_dev_exit(void)
{
	del_timer_sync(&tlist);
	printk(KERN_INFO "timer deleted\n");
}

module_init(timer_dev_init);
module_exit(timer_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A timer_dev demo");