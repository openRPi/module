/*
 *	自由操作一段内核缓存。注册为混杂设备。
 *	
 *	所有混杂设备的主设备号都为10
 *	混杂类目录 /sys/class/misc
 *	设备目录 /dev/chr_chr_buf_dev
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：？
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>

#define BUF_SIZE 50

#define func_in()	printk(KERN_INFO "++ %s:%s (%d) ++\n", __FILE__, __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s:%s (%d) --\n", __FILE__, __func__, __LINE__)

static int chr_buf_dev_open(struct inode * inode, struct file * filp)
{
	char * buf_array=NULL;
	func_in();

	buf_array = kmalloc(BUF_SIZE,GFP_KERNEL);
	if(!buf_array)
		return -EINVAL;
	// 缓存指针存入 filp 的私有数据。
	filp->private_data = buf_array;

	func_out();
	return 0;
}

static int chr_buf_dev_release(struct inode * inode, struct file * filp)
{
	func_in();
	kfree(filp->private_data);
	func_out();
	return 0;
}

static ssize_t chr_buf_dev_read(struct file * filp, char __user * up, size_t size, loff_t * off)
{
	char * buf_array=NULL;
	int min=0;

	func_in();
	printk(KERN_INFO "@size=%d\n@off=%d\n",size,(int)*off);
	
	buf_array = filp->private_data;
	// 确定实际读取的字节数。
	min = BUF_SIZE - *off < size? BUF_SIZE - *off:size;

	if(copy_to_user(up, &(buf_array[*off]), min))
		return -EINVAL;
	// 根据实际读取的字节数移动位置指针
	*off += min;
	
	printk(KERN_INFO "real_read_size=%d\n",min);
	
	func_out();
	// 返回实际读取的字节数!
	return min;
}

static ssize_t chr_buf_dev_write(struct file * filp, const char __user * up, size_t size, loff_t * off)
{
	char * buf_array=NULL;
	int min=0;
	
	func_in();
	printk(KERN_INFO "@size=%d\n@off=%d\n",size,(int)*off);

	buf_array = filp->private_data;
	// 确定实际写入的字节数。
	min = BUF_SIZE - *off < size? BUF_SIZE - *off:size;

	if(copy_from_user(&(buf_array[*off]), up, min))
		return -EINVAL;
	// 根据实际写入的字节数移动位置指针
	*off += min;

	printk(KERN_INFO "real_write_size=%d\n",min);
	func_out();
	// 返回实际写入的字节数!
	return min;
}

static loff_t chr_buf_dev_llseek(struct file * filp, loff_t off, int where)
{
	loff_t newpos;
	func_in();

	switch(where)
	{
		case 0: /* SEEK_SET */
			newpos = off;
			break;

		case 1: /* SEEK_CUR */
			newpos = filp->f_pos + off;
			break;

		case 2: /* SEEK_END */
			newpos = BUF_SIZE + off;
			break;

		default :
			return -EINVAL;
	}
	// 检查位置指针是否移出缓存边界
	if (newpos < 0 || newpos > BUF_SIZE-1)
		return -EINVAL;

	printk(KERN_INFO "lastpos=%d\n",(int)(filp->f_pos));
	printk(KERN_INFO "off=%d\n",(int)off);
	printk(KERN_INFO "newpos=%d\n",(int)newpos);

	// 修改 filp 的位置指针成员
	filp->f_pos = newpos;

	func_out();
	// 返回新的位置指针！
	return newpos;
}

static struct file_operations chr_buf_dev_fops =
{
	.owner = THIS_MODULE,
	.open = chr_buf_dev_open,
	.release = chr_buf_dev_release,
	.write = chr_buf_dev_write,
	.read = chr_buf_dev_read,
	.llseek = chr_buf_dev_llseek,
};

static struct miscdevice chr_buf_dev =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "chr_buf_dev",
	.fops = &chr_buf_dev_fops,
};

static int __init chr_buf_dev_init(void)
{
	func_in();
	misc_register(&chr_buf_dev);
	func_out();
	return 0;
}

static void __exit chr_buf_dev_exit(void)
{
	func_in();
	misc_deregister(&chr_buf_dev);
	func_out();
}

module_init(chr_buf_dev_init);
module_exit(chr_buf_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_buf_dev");
