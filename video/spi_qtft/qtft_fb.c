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

#include "lcd_lib.h"

#define VIDEOMEMSIZE	(320*240*2) 

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

static void *videomemory;
static u_long videomemorysize = VIDEOMEMSIZE;

static struct platform_device *qtft_fb_device;

// 定义在 screen_info.c
extern struct fb_var_screeninfo qtft_fb_var_default;
extern struct fb_fix_screeninfo qtft_fb_fix_default;

// 定义在 ops.c
extern struct fb_ops qtft_fb_ops;

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

static int qtft_fb_probe(struct platform_device * dev)
{
	struct fb_info *info;
	int retval = -ENOMEM;

	func_in();

	// 分配显存
	if (!(videomemory = rvmalloc(videomemorysize)))
		return retval;
	memset(videomemory, 0, videomemorysize);

	// 动态分配 fb_info
	info = framebuffer_alloc((sizeof(u32)*16), &dev->dev);
	if (!info)
		goto err0;

	// 虚拟地址
	info->screen_base = (char __iomem *)videomemory;
	info->screen_size = videomemorysize;
	
	// 文件操作符 qtft_fb_ops 在 ops.c 中定义
	info->fbops = &qtft_fb_ops;

	info->var = qtft_fb_var_default;

	qtft_fb_fix_default.smem_start = (unsigned long) videomemory;
	qtft_fb_fix_default.smem_len = videomemorysize;
	info->fix = qtft_fb_fix_default;
	
	// 16色伪调色板指针指向了 info->par 的私有空间
	info->pseudo_palette = info->par;
	info->par = NULL;
	info->flags = FBINFO_DEFAULT;

	// 为16色伪调色板分配内存
	retval = fb_alloc_cmap(&info->cmap, 16, 0);
	if (retval < 0)
		goto err1;

	retval = register_framebuffer(info);
	if (retval < 0)
		goto err2;

	// 初始化 LCD 模块
	retval = lcd_init_normal();
	if (retval < 0)
		goto err3;
	
	// 将 info 指针存入平台设备私有数据
	platform_set_drvdata(dev, info);

	printk(KERN_INFO "SPI QVGA TFT LCD driver: fb%d, %ldK video memory\n", info->node, videomemorysize >> 10);
	goto out;

err3:
	unregister_framebuffer(info);
err2:
	fb_dealloc_cmap(&info->cmap);
err1:
	framebuffer_release(info);
err0:
	rvfree(videomemory, videomemorysize);
out:
	func_out();
	return retval;
}

static int qtft_fb_remove(struct platform_device *dev)
{
	struct fb_info *info = platform_get_drvdata(dev);

	func_in();
	if (info)
	{
		lcd_exit();
		unregister_framebuffer(info);
		fb_dealloc_cmap(&info->cmap);
		framebuffer_release(info);
		rvfree(videomemory, videomemorysize);
	}
	func_out();
	return 0;
}

static struct platform_device_id qtft_fb_idtable[] = 
{
	{"qtft_fb_device", 0},
	{ },
};

static struct platform_driver qtft_fb_driver = 
{
	.driver = 
	{
		.name = "qtft_fb_driver",
	},
	.probe    = qtft_fb_probe,
	.remove   = qtft_fb_remove,
	.id_table = qtft_fb_idtable,
};

int qtft_fb_init(void)
{
	int err = 0;
	func_in();

	err = platform_driver_register(&qtft_fb_driver);
	if(err<0)
		goto out;

	qtft_fb_device = platform_device_alloc("qtft_fb_device", 0);
	if (!qtft_fb_device)
	{
		err = -ENOMEM;
		goto unregister;
	}

	err = platform_device_add(qtft_fb_device);
	if (err<0)
		goto device_put;

	goto out;

device_put:
	platform_device_put(qtft_fb_device);
unregister:
	platform_driver_unregister(&qtft_fb_driver);
out:
	func_out();
	return err;
}

void qtft_fb_exit(void)
{
	func_in();

	platform_device_unregister(qtft_fb_device);
	platform_driver_unregister(&qtft_fb_driver);

	func_out();
}
