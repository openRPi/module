/*
 *	注册SPI设备驱动到内核
 *	驱动名chr_spi_dri，匹配I2C设备chr_spi_dev
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

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

static struct spi_device_id chr_spi_dri_idtable[] = {
	{ "chr_spi_dev", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi,chr_spi_dri_idtable);

/**
 * 设备名匹配时的回调函数
 * @param  spi      设备spi结构体指针
 * @return          0或错误号
 */
int chr_spi_dri_probe(struct spi_device *spi)
{
	func_in();

	printk(KERN_INFO "probe device. name=%s\n",spi->modalias);

	func_out();
	return 0;
}

int chr_spi_dri_remove(struct spi_device *spi)
{
	func_in();

	printk(KERN_INFO "remove device. name=%s\n",spi->modalias);

	func_out();
	return 0;
}

static struct spi_driver chr_spi_dri_driver = {
	.driver = {
		.name	= "chr_spi_dri",
		.owner = THIS_MODULE,
	},

	.id_table	= chr_spi_dri_idtable,
	.probe		= chr_spi_dri_probe,
	.remove		= chr_spi_dri_remove,
};

static int __init chr_spi_dri_init(void)
{
	int re;
	func_in();
	re = spi_register_driver(&chr_spi_dri_driver);
	func_out();
	return re;
}

static void __exit chr_spi_dri_exit(void)
{
	func_in();
	spi_unregister_driver(&chr_spi_dri_driver);
	func_out();
}

module_init(chr_spi_dri_init);
module_exit(chr_spi_dri_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_spi_dri");
