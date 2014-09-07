/*
 *	qtft 视频驱动的 SPI 子系统
 *	SPI 注册成功后，再进一步注册 framebuffer
 *
 *  依赖：spi_bcm2708 (SPI controller driver)
 *
 *  注意：不可先加载 spidev (User mode SPI device interface) 模块，
 *  因为它会占用 chip_select 0~1，导致本模块无法加载。
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：?
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#include "qtft_gpio.h"
#include "qtft_fb.h"

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

#define SPI_BUS_NUM 0

static struct spi_device *current_device=NULL;

static struct spi_board_info qtft_spi_dev_board_info[] = {
	{
		.modalias    = "qtft_spi_dev",
		.bus_num     = SPI_BUS_NUM,
		.chip_select = 0,
		.mode        = SPI_MODE_0,
		.max_speed_hz = 10e6,
	}
};

static struct spi_device_id qtft_spi_dri_idtable[] = {
	{ "qtft_spi_dev", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi,qtft_spi_dri_idtable);

#define lcd_context_reg()	qtft_gpio_set_dc(0)
#define lcd_context_data()	qtft_gpio_set_dc(1)

int qtft_spi_dri_probe(struct spi_device *dev)
{
	int err = 0;

	current_device = dev;
	goto out;

out:
	return err;
}

int qtft_spi_dri_remove(struct spi_device *dev)
{
	current_device = NULL;
	return 0;
}

static struct spi_driver qtft_spi_dri_driver = {
	.driver = {
		.name	= "qtft_spi_dri",
		.owner = THIS_MODULE,
	},

	.id_table	= qtft_spi_dri_idtable,
	.probe		= qtft_spi_dri_probe,
	.remove		= qtft_spi_dri_remove,
};

static void qtft_spi_device_unregister(struct spi_device *device)
{
	spi_unregister_device(device);
}

int qtft_spi_init(void)
{
	int err=0;
	struct spi_master *master=NULL;
	struct spi_device *new_device=NULL;
	func_in();

	err = spi_register_driver(&qtft_spi_dri_driver);
	if(err)
	{
		printk(KERN_ERR "Can't register spi driver \n");
		goto out;
	}

	master = spi_busnum_to_master(SPI_BUS_NUM);
	if(!master)
	{
		printk(KERN_ERR "Can't get SPI bus %d\n",SPI_BUS_NUM);
		err = -ENODEV;
		goto err0;
	}

	new_device = spi_new_device(master, qtft_spi_dev_board_info);
	if(!new_device)
	{
		printk(KERN_ERR "Can't register spi device \n");
		err = -ENODEV;
		goto err0;
	}

	goto out;

err0:
	spi_unregister_driver(&qtft_spi_dri_driver);
out:
	func_out();
	return err;
}

void qtft_spi_exit(void)
{
	func_in();

	qtft_spi_device_unregister(current_device);
	spi_unregister_driver(&qtft_spi_dri_driver);

	func_out();
}

int qtft_spi_write_then_read(const void *tbuf, size_t tn, void *rbuf, size_t rn)
{
	if(current_device)
		return spi_write_then_read(current_device, tbuf, tn, rbuf, rn);
	else
		return -ENODEV;
}

int qtft_spi_write(const void *buf, size_t len)
{
	if(current_device)
		return spi_write(current_device, buf, len);
	else
		return -ENODEV;
}

int qtft_spi_read(void *buf, size_t len)
{
	if(current_device)
		return spi_read(current_device, buf, len);
	else
		return -ENODEV;
}