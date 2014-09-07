/*
 *	最简单的字符设备驱动
 *	只完成加载和卸载的工作，并用udev机制自动创建用户空间接口文件。
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

static struct cdev chr_dev;
static struct class * chr_dev_class;
static dev_t ndev;

struct file_operations chr_udev_dev_fops =
{
	.owner = THIS_MODULE,
};

/**
 * 设备的加载函数
 * 主要完成
 * 	1. 动态分配设备号
 * 	2. 初始化cdev结构体
 * 	3. 关联cdev结构体和设备号
 * 	4. 创建字符设备类
 * 	5. 创建设备
 * @return  0或负的错误号
 */
static int __init chr_udev_dev_init(void)
{
	int ret;
	
	ret = alloc_chrdev_region(&ndev,0,1,"mknod_dev");
	if(ret<0)
		return ret;
	
	cdev_init(&chr_dev,&chr_udev_dev_fops);
	ret = cdev_add(&chr_dev,ndev,1);
	if(ret<0)
		return ret;

	chr_dev_class = class_create(THIS_MODULE,"chr_udev_dev");
	device_create(chr_dev_class,NULL,ndev,NULL,"chr_udev_dev");

	printk(KERN_INFO "chr_udev_dev init, major=%d, minor=%d\n",MAJOR(ndev),MINOR(ndev));
	return 0;
}

static void __exit chr_udev_dev_exit(void)
{
	cdev_del(&chr_dev);
	unregister_chrdev_region(ndev,1);
	device_destroy(chr_dev_class,ndev);
	class_destroy(chr_dev_class);
	printk(KERN_INFO "chr_udev_dev exit\n");
}

module_init(chr_udev_dev_init);
module_exit(chr_udev_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_udev_dev");
