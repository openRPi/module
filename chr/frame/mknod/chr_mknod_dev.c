/*
 *	最简单的字符设备驱动。
 *	只完成加载和卸载的工作，并用mknod命令静态创建用户空间接口文件。
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：http://www.openrpi.org/blogs/?p=153
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

static struct cdev chr_dev;
static dev_t ndev;

struct file_operations chr_mknod_dev_fops =
{
	.owner = THIS_MODULE,
};

/**
 * 设备的加载函数。
 * 主要完成
 * 	1. 动态分配设备号
 * 	2. 初始化cdev结构体
 * 	3. 关联cdev结构体和设备号
 * 	
 * @return  0或负的错误号
 */
static int __init chr_mknod_dev_init(void)
{
	int ret;

	ret = alloc_chrdev_region(&ndev,0,1,"mknod_dev");
	if(ret<0)
		return ret;

	cdev_init(&chr_dev,&chr_mknod_dev_fops);
	ret = cdev_add(&chr_dev,ndev,1);
	if(ret<0)
		return ret;

	printk(KERN_INFO "chr_mknod_dev init, major=%d, minor=%d\n",MAJOR(ndev),MINOR(ndev));
	return 0;
}

/**
 * 设备的卸载函数。
 * 主要完成：
 * 	1. 移除cdev结构体
 * 	2. 释放占用的设备号
 */
static void __exit chr_mknod_dev_exit(void)
{
	cdev_del(&chr_dev);
	unregister_chrdev_region(ndev,1);
	printk(KERN_INFO "chr_mknod_dev exit\n");
}

module_init(chr_mknod_dev_init);
module_exit(chr_mknod_dev_exit);

/**
 * 模块信息
 */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_mknod_dev");
