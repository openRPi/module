/*
 *	注册I2C设备到内核
 *	设备名chr_iic_dev，设备地址0x10.
 *	只完成加载和卸载的工作。
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：http://www.openrpi.org/blogs/?p=281
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/i2c.h>

/**
 * i2c-stub 虚拟的I2C总线号为2
 */
#define BUS_NUM 2

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

struct i2c_adapter * adap=NULL;
struct i2c_client * client=NULL;

static struct i2c_board_info chr_iic_dev_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("chr_iic_dev",0x10),
	}
};

static int __init chr_iic_dev_init(void)
{
	func_in();

	/**
	 * 获取 I2C适配器
	 */
	adap = i2c_get_adapter(BUS_NUM);
	if (!adap)
		return -ENODEV;
	printk(KERN_INFO "get adap (bus %d)\n",BUS_NUM);

	/**
	 * 注册设备
	 * 若存在设备名匹配，I2C驱动程序的probe函数将被自动调用
	 */
	client = i2c_new_device(adap,chr_iic_dev_board_info);
	if(client)
		printk(KERN_INFO "add new device. name=%s, addr=%x \n",client->name, client->addr);
	else
		return -ENODEV;

	func_out();
	return 0;
}

static void __exit chr_iic_dev_exit(void)
{
	func_in();

	if(client)
	{
		i2c_unregister_device(client);
		printk(KERN_INFO "release device.\n");
	}

	if(adap)
	{
		i2c_put_adapter(adap);
		printk(KERN_INFO "release adap (bus %d)\n",BUS_NUM);
	}
	func_out();
}

module_init(chr_iic_dev_init);
module_exit(chr_iic_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_iic_dev");
