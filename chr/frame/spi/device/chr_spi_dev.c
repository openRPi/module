/*
 *	注册SPI设备到内核
 *	设备名chr_spi_dev，挂载到总线号0.
 *	只完成加载和卸载的工作。
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：http://www.openrpi.org/blogs/?p=305
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>

/**
 * 挂载到总线号0
 */
#define BUS_NUM 0

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

struct spi_master * master = NULL;
struct spi_device * device = NULL;

static struct spi_board_info chr_spi_dev_board_info[] __initdata = {
	{
		.modalias    = "chr_spi_dev",
		.bus_num     = BUS_NUM,
		.chip_select = 2,
	}
};

static int __init chr_spi_dev_init(void)
{
	func_in();

	master = spi_busnum_to_master(BUS_NUM);
	if (!master)
		return -ENODEV;
	printk(KERN_INFO "get master (bus %d)\n",BUS_NUM);

	/**
	 * 注册设备
	 * 若存在设备名匹配，SPI驱动程序的probe函数将被自动调用
	 */
	device = spi_new_device(master,chr_spi_dev_board_info);
	if(device)
		printk(KERN_INFO "add new device. name=%s \n",device->modalias);
	else
		return -ENODEV;

	func_out();
	return 0;
}

static void __exit chr_spi_dev_exit(void)
{
	func_in();

	if(device)
	{
		spi_unregister_device(device);
		printk(KERN_INFO "release device.\n");
	}

	func_out();
}

module_init(chr_spi_dev_init);
module_exit(chr_spi_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_spi_dev");
