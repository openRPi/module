#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/errno.h>

#include "qtft_gpio.h"
#include "qtft_fb.h"
#include "qtft_spi.h"

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

static int __init qtft_base_init(void)
{
	int err=0;
	func_in();

	// 注册gpio
	err = qtft_gpio_init();
	if(err)
	{
		printk(KERN_ERR "Can't init qtft gpio\n");
		goto out;
	}

	// 注册 spi
	err = qtft_spi_init();
	if(err)
	{
		printk(KERN_ERR "Can't init qtft spi\n");
		goto err0;
	}

	// 注册 framebuffer
	err = qtft_fb_init();
	if(err)
	{
		printk(KERN_ERR "Can't init qtft framebuffer\n");
		goto err1;
	}

	goto out;

err1:
	qtft_spi_exit();
err0:
	qtft_gpio_exit();
out:
	func_out();
	return err;
}

static void __exit qtft_base_exit(void)
{
	func_in();

	qtft_gpio_exit();
	qtft_spi_exit();
	qtft_fb_exit();

	func_out();
}

module_init(qtft_base_init);
module_exit(qtft_base_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("qtft");