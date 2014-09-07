/*
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
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include "ops.h"

#define SPI_BUS_NUM 0

#define VIDEOMEMSIZE	(320*240*16) 

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

static void *videomemory;
static u_long videomemorysize = VIDEOMEMSIZE;

static struct platform_device *spi_qtft_device;

extern struct fb_var_screeninfo spi_qtft_var_default;
extern struct fb_fix_screeninfo spi_qtft_fix_default;

// 要注册的 SPI 设备信息
static struct spi_board_info qtft_spi_board_info = 
{
	.modalias    = "qtft_spi",
	.bus_num     = SPI_BUS_NUM,
	.chip_select = 0,
};

// 将被存入 fb_info.par
struct qtft_par
{
	// 调色板store
	u32 palette[16];
	// SPI 设备
	struct spi_device *spi;
};

#define qtft_par_size() 	( sizeof(struct qtft_par) )
#define to_qtft_par(par)	( (struct qtft_par *)par )

/**
 * 分配显存
 * @param  size 显存大小
 * @return      指针
 */
static void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); /* Clear the ram out, no junk to the user */
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	return mem;
}

/**
 * 释放显存
 * @param mem  指针
 * @param size 大小
 */
static void rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}

/**
 * 注册SPI设备。
 * 设备信息定义在qtft_spi_board_info。
 * @param  par struct qtft_par 指针
 * @return     0或错误号
 */
static int qtft_spi_device_register(struct qtft_par *par)
{
	struct spi_device * spi;
	struct spi_master * master = NULL;
	int err=0;

	master = spi_busnum_to_master(SPI_BUS_NUM);
	if (!master)
	{
		// printk(KERN_INFO "Can't get SPI master (bus %d)\n",SPI_BUS_NUM);
		err = -ENODEV;
		goto out;
	}

	spi = spi_new_device(master, &qtft_spi_board_info);
	if (!spi)
	{
		// printk(KERN_INFO "Can't register SPI device: %s\n",qtft_spi_board_info.modalias);
		err = -ENODEV;
		goto out;
	}

	par->spi = spi;
	goto out;

out:
	return err;
}

static void qtft_spi_device_unregister(struct qtft_par *par)
{
	struct spi_device *spi = par->spi;
	spi_unregister_device(spi);
}

static int spi_qtft_probe(struct platform_device * dev)
{
	struct fb_info *info;
	int retval = -ENOMEM;

	func_in();

	// 分配显存
	if (!(videomemory = rvmalloc(videomemorysize)))
		return retval;
	memset(videomemory, 0, videomemorysize);

	// 动态分配 fb_info
	info = framebuffer_alloc(qtft_par_size(), &dev->dev);
	if (!info)
		goto err0;

	// 虚拟地址
	info->screen_base = (char __iomem *)videomemory;
	info->screen_size = videomemorysize;
	
	// 文件操作符 spi_qtft_ops 在 ops.c 中定义
	info->fbops = &spi_qtft_ops;

	info->var = spi_qtft_var_default;

	spi_qtft_fix_default.smem_start = (unsigned long) videomemory;
	spi_qtft_fix_default.smem_len = videomemorysize;
	info->fix = spi_qtft_fix_default;
	
	// 16色伪调色板指针指向了 info->par->palette
	info->pseudo_palette = (void *) (to_qtft_par(info->par)->palette);
	info->flags = FBINFO_DEFAULT;

	// 注册SPI设备
	retval = qtft_spi_device_register(to_qtft_par(info->par));
	if(retval<0)
	{
		dev_err(&dev->dev, "Can't register SPI device: %s\n",qtft_spi_board_info.modalias);
		goto err1;
	}

	// 为16色伪调色板分配内存
	retval = fb_alloc_cmap(&info->cmap, 16, 0);
	if (retval < 0)
		goto err2;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err3;

	// 将 info 指针存入平台设备私有数据
	platform_set_drvdata(dev, info);

	printk(KERN_INFO "SPI QVGA TFT LCD driver: fb%d, %ldK video memory\n", info->node, videomemorysize >> 10);
	goto out;

err3:
	fb_dealloc_cmap(&info->cmap);
err2:
	qtft_spi_device_unregister(to_qtft_par(info->par));
err1:
	framebuffer_release(info);
err0:
	rvfree(videomemory, videomemorysize);
out:
	func_out();
	return retval;
}

static int spi_qtft_remove(struct platform_device *dev)
{
	struct fb_info *info = platform_get_drvdata(dev);

	func_in();
	if (info)
	{
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		qtft_spi_device_unregister(info->par);
		framebuffer_release(info);
		rvfree(videomemory, videomemorysize);
	}
	func_out();
	return 0;
}

struct platform_device_id spi_qtft_idtable[] = 
{
	{"spi_qtft", 0},
	{ },
};

static struct platform_driver spi_qtft_driver = 
{
	.driver = 
	{
		.name = "spi_qtft_driver",
	},
	.probe    = spi_qtft_probe,
	.remove   = spi_qtft_remove,
	.id_table = spi_qtft_idtable,
};

static int  __init spi_qtft_init(void)
{
	int err = 0;
	func_in();

	err = platform_driver_register(&spi_qtft_driver);
	if(err<0)
		goto out;

	spi_qtft_device = platform_device_alloc("spi_qtft", 0);
	if (!spi_qtft_device)
	{
		err = -ENOMEM;
		goto unregister;
	}

	err = platform_device_add(spi_qtft_device);
	if (err<0)
		goto device_put;

	goto out;

device_put:
	platform_device_put(spi_qtft_device);
unregister:
	platform_driver_unregister(&spi_qtft_driver);
out:
	func_out();
	return err;
}

static void __exit spi_qtft_exit(void)
{
	func_in();

	platform_device_unregister(spi_qtft_device);
	platform_driver_unregister(&spi_qtft_driver);

	func_out();
}

module_init(spi_qtft_init);
module_exit(spi_qtft_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("SPI QVGA TFT LCD driver");