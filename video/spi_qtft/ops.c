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

ssize_t ops_read(struct fb_info *info, char __user *buf, size_t count,
		    loff_t *ppos)
{
	unsigned long p = *ppos;
	void *src;
	int err = 0;
	unsigned long total_size;

	if (info->state != FBINFO_STATE_RUNNING)
		return -EPERM;

	total_size = info->screen_size;

	if (total_size == 0)
		total_size = info->fix.smem_len;

	if (p >= total_size)
		return 0;

	if (count >= total_size)
		count = total_size;

	if (count + p > total_size)
		count = total_size - p;

	src = (void __force *)(info->screen_base + p);

	if (info->fbops->fb_sync)
		info->fbops->fb_sync(info);

	if (copy_to_user(buf, src, count))
		err = -EFAULT;

	if  (!err)
		*ppos += count;

	return (err) ? err : count;
}

ssize_t ops_write(struct fb_info *info, const char __user *buf, size_t count, loff_t *ppos)
{
	unsigned long p = *ppos;

	void *fb_buf_dst=NULL;
	unsigned long total_size;

	unsigned long new_p=p;
	size_t new_count=count;

	int err=0;
	int x=0, y=0;

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

	fb_buf_dst = (void __force *)(info->screen_base + p);

	if(copy_from_user(fb_buf_dst, buf, count))
	{
		err = -EFAULT;
		goto out;
	}

	// 计算合适的起始位置和写入字节数
	if(p%2)
		new_p = p - 1;
	if((p+count)%2)
		new_count = count + 1 + (p-new_p);
	if(new_p + new_count > total_size)
		new_count = total_size - new_p;

	x = new_p/2 % 320;
	y = new_p/2 / 320;

	// printk(KERN_INFO "@p         = %ld\n",p);
	// printk(KERN_INFO "@new_p     = %ld\n",new_p);
	// printk(KERN_INFO "@count     = %d\n",count);
	// printk(KERN_INFO "@new_count = %d\n",new_count);
	// printk(KERN_INFO "@x         = %d\n",x);
	// printk(KERN_INFO "@y         = %d\n",y);

	err = lcd_memory_write_from(x,y, (void __force *)(info->screen_base + new_p), new_count);
	if(err)
		goto out;

	*ppos += count;

	err = count;
	goto out;

out:
	return err;
}

int ops_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	*var = qtft_fb_var_default;
	return 0;
}

int ops_set_par(struct fb_info *info)
{
	return 0;
}

int ops_blank(int blank, struct fb_info *info)
{
	return 0;
}

int ops_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info)
{
	int err=0;
	u32 v;

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
	return err;
}

struct fb_ops qtft_fb_ops = 
{
	.owner = THIS_MODULE,
	
	.fb_read      = ops_read,
	.fb_write     = ops_write,

	/* checks var and eventually tweaks it to something supported,
     * DO NOT MODIFY PAR */
	.fb_check_var = ops_check_var,

	/* set the video mode according to info->var */
	// .fb_set_par   = ops_set_par,

	/* set color register */
	.fb_setcolreg = ops_setcolreg,
	// .fb_blank     = ops_blank,
};
