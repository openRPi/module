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
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/spi/spi.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#include "qtft_spi.h"
#include "qtft_gpio.h"
#include "lcd_lib.h"

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

extern struct fb_var_screeninfo qtft_fb_var_default;
extern struct fb_fix_screeninfo qtft_fb_fix_default;

ssize_t ops_read(struct fb_info *info, char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned char *fb_buf=NULL;
	unsigned long total_size;
	int err=0;
	int x1,x2,y1,y2;

	if (info->state != FBINFO_STATE_RUNNING)
		return -EPERM;

	total_size = info->screen_size;

	if (total_size == 0)
		total_size = info->fix.smem_len;

	if (p > total_size)
	{
		err = -EFBIG;
		goto out;
	}

	if (count > total_size) 
		count = total_size;

	if (count + p > total_size) 
		count = total_size - p;

	fb_buf = kmalloc(count,GFP_KERNEL);
	if(!fb_buf)
	{
		err = -EBUSY;
		goto out;
	}

	x1 = p%(info->var.xres);
	y1 = p/(info->var.yres);
	x2 = (p+count)%(info->var.xres);
	y2 = (p+count)/(info->var.yres);

	err = lcd_memory_area_read(x1,y1,x2,y2,fb_buf,count);
	if(err)
		goto err0;

	if(copy_to_user(buf, fb_buf, count))
	{
		err = -EFAULT;
		goto err0;
	}

	*ppos += count;
	
	err = count;
	goto err0;

err0:
	kfree(fb_buf);
out:
	return err;
}

ssize_t ops_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned char *fb_buf=NULL;
	unsigned long total_size;
	int err=0;
	int x1,x2,y1,y2;

	if (info->state != FBINFO_STATE_RUNNING)
		return -EPERM;

	total_size = info->screen_size;

	if (total_size == 0)
		total_size = info->fix.smem_len;

	if (p > total_size)
	{
		err = -EFBIG;
		goto out;
	}

	if (count > total_size) 
		count = total_size;

	if (count + p > total_size) 
		count = total_size - p;

	fb_buf = kmalloc(count,GFP_KERNEL);
	if(!fb_buf)
	{
		err = -EBUSY;
		goto out;
	}

	if(copy_from_user(fb_buf, buf, count))
	{
		err = -EFAULT;
		goto err0;
	}

	x1 = p%(info->var.xres*2);
	y1 = p/(info->var.yres*2);
	x2 = (p+count)%(info->var.xres*2);
	y2 = (p+count)/(info->var.yres*2);

	printk(KERN_INFO "count=%d, pos=%d, x1=%d, y1=%d, x2=%d, y2=%d\n",count,p,x1,y1,x2,y2);

	err = lcd_memory_area_write(x1,y1,x2,y2,fb_buf,count);
	if(err)
		goto err0;

	*ppos += count;

	err = count;
	goto err0;

err0:
	kfree(fb_buf);
out:
	return err;
}

int ops_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	func_in();
	*var = qtft_fb_var_default;
	func_out();
	return 0;
}

int ops_set_par(struct fb_info *info)
{
	func_in();
	func_out();
	return 0;
}

int ops_blank(int blank, struct fb_info *info)
{
	func_in();
	func_out();
	return 0;
}

int ops_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info)
{
	int err=0;
	u32 v;
	func_in();

	if (regno >= 16)
	{
		err = 1;
		goto out;
	}

	v = (red 	<< info->var.red.offset) 	|
	    (green 	<< info->var.green.offset) 	|
	    (blue 	<< info->var.blue.offset)	|
	    (transp << info->var.transp.offset)	;

	((u32 *) (info->pseudo_palette))[regno] = v;

	goto out;

out:
	func_out();
	return err;
}

/**
 * 等待位块传送
 */
int ops_sync(struct fb_info *info)
{
	func_in();
	func_out();
	return 0;
}

struct fb_ops qtft_fb_ops = 
{
	.owner = THIS_MODULE,
	
	.fb_read      = ops_read,
	.fb_write     = ops_write,

	/* checks var and eventually tweaks it to something supported,
     * DO NOT MODIFY PAR */
	// .fb_check_var = ops_check_var,

	/* set the video mode according to info->var */
	// .fb_set_par   = ops_set_par,

	/* set color register */
	.fb_setcolreg = ops_setcolreg,
	// .fb_blank     = ops_blank,
	// .fb_sync      = ops_sync,
};
