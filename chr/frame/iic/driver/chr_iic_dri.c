/*
 *	注册I2C设备驱动到内核
 *	驱动名chr_iic_dri，匹配I2C设备chr_iic_dev
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

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

static struct i2c_device_id chr_iic_dri_idtable[] = {
	{ "chr_iic_dev", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c,chr_iic_dri_idtable);

/**
 * 设备名匹配时的回调函数
 * @param  client   设备client结构体指针
 * @param  id_table 设备id_table指针
 * @return          0或错误号
 */
int chr_iic_dri_probe(struct i2c_client * client, const struct i2c_device_id * id_table)
{
	func_in();

	printk(KERN_INFO "probe device. name=%s, addr=%x, id=%d\n",client->name, client->addr,(int)id_table->driver_data);
	i2c_smbus_write_byte(client,0xab);
	printk(KERN_INFO "write 0xab\n");

	func_out();
	return 0;
}

int chr_iic_dri_remove(struct i2c_client * client)
{
	func_in();

	printk(KERN_INFO "remove device. name=%s, addr=%x \n",client->name, client->addr);

	func_out();
	return 0;
}

static struct i2c_driver chr_iic_dri_driver = {
	.driver = {
		.name	= "chr_iic_dri",
	},

	.id_table	= chr_iic_dri_idtable,
	.probe		= chr_iic_dri_probe,
	.remove		= chr_iic_dri_remove,
};

static int __init chr_iic_dri_init(void)
{
	int err;
	func_in();
	err = i2c_add_driver(&chr_iic_dri_driver);
	func_out();
	return err;
}

static void __exit chr_iic_dri_exit(void)
{
	func_in();
	i2c_del_driver(&chr_iic_dri_driver);
	func_out();
}

module_init(chr_iic_dri_init);
module_exit(chr_iic_dri_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_iic_dri");
