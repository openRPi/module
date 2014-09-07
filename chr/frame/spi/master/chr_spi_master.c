/*
 *	注册SPI Master到内核
 *	设备名chr_spi_master，总线号固定为99
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
#include <linux/spi/spi.h>
#include <linux/platform_device.h>

#define BUS_NUM 		99
#define NUM_CHIPSELECT	3

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

static void print_spi_device(struct spi_device * spi)
{
	printk(KERN_INFO "@ spi_device: %s\n",spi->modalias);
	printk(KERN_INFO "@ chip_select = %d\n",spi->chip_select);
	printk(KERN_INFO "@ ---\n");
}

static void print_spi_master(struct spi_master *master)
{
	printk(KERN_INFO "@ spi_master: %d\n",master->bus_num);
	printk(KERN_INFO "@ ---\n");
}

static int chr_spi_master_setup(struct spi_device * spi)
{
	func_in();
	print_spi_device(spi);
	func_out();
	return 0;
}

static int chr_spi_master_cleanup(struct spi_device * spi)
{
	func_in();
	print_spi_device(spi);
	func_out();
	return 0;
}

static int chr_spi_master_pth(struct spi_master *master)
{
	func_in();
	print_spi_master(master);
	func_out();
	return 0;
}

static int chr_spi_master_upth(struct spi_master *master)
{
	func_in();
	print_spi_master(master);
	func_out();
	return 0;
}

static int chr_spi_master_tom(struct spi_master *master, struct spi_message *mesg)
{
	func_in();
	print_spi_master(master);
	print_spi_device(mesg->spi);
	func_out();
	return 0;
}

static int chr_spi_master_probe(struct platform_device *pdev)
{
	int err;
	struct spi_master *master;
	printk(KERN_INFO "++ chr_spi_master_probe ++\n");

	master = spi_alloc_master(&pdev->dev, 0);
	if(!master)
	{
		printk(KERN_ERR "spi_alloc_master ERR!\n");
		return -ENODEV;
	}
	platform_set_drvdata(pdev, master);

	master->bus_num                     = BUS_NUM;
	master->num_chipselect              = NUM_CHIPSELECT;
	master->setup                       = chr_spi_master_setup;
	master->cleanup                     = chr_spi_master_cleanup;
	master->prepare_transfer_hardware   = chr_spi_master_pth;
	master->transfer_one_message        = chr_spi_master_tom;
	master->unprepare_transfer_hardware = chr_spi_master_upth;

	err = spi_register_master(master);
	if(err)
	{
		dev_err(&pdev->dev, "Could not register SPI master: %d\n", err);
		goto out_master_put;
	}

out_master_put:
	spi_master_put(master);

	printk(KERN_INFO "-- chr_spi_master_probe --\n");
	return 0;
}

static int chr_spi_master_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	printk(KERN_INFO "++ chr_spi_master_remove ++\n");

	master = spi_master_get(platform_get_drvdata(pdev));
	spi_unregister_master(master);
	spi_master_put(master);

	printk(KERN_INFO "-- chr_spi_master_remove --\n");
	return 0;
}

static struct platform_driver chr_spi_master_driver = {
	.driver		= {
		.name	= "chr_spi_master",
		.owner	= THIS_MODULE,
	},
	.probe		= chr_spi_master_probe,
	.remove		= chr_spi_master_remove,
};

static int __init chr_spi_master_init(void)
{
	int err;
	printk(KERN_INFO "++ chr_spi_master_init ++\n");
	err = platform_driver_register(&chr_spi_master_driver);
	printk(KERN_INFO "-- chr_spi_master_init --\n");
	return err;
}

static void __exit chr_spi_master_exit(void)
{
	printk(KERN_INFO "++ chr_spi_master_exit ++\n");
	platform_driver_unregister(&chr_spi_master_driver);
	printk(KERN_INFO "-- chr_spi_master_exit --\n");
}

module_init(chr_spi_master_init);
module_exit(chr_spi_master_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_spi_master");
