/*
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
#include <linux/uaccess.h>
#include <linux/spi/spi.h>

#define func_in()	printk(KERN_INFO "++ %s (%d) ++\n", __func__, __LINE__)
#define func_out()	printk(KERN_INFO "-- %s (%d) --\n", __func__, __LINE__)

// ------------ 设备与驱动描述 -------------

#define SPI_BUS_NUM 0

static struct spi_device *current_device=NULL;
static struct spi_device *new_device=NULL;

static struct spi_board_info spi_tool_dev_board_info[] __initdata = {
	{
		.modalias    = "spi_tool_dev",
		.bus_num     = SPI_BUS_NUM,
		.max_speed_hz = 8e6,
		.chip_select = 0,
	}
};

static struct spi_device_id spi_tool_idtable[] = {
	{ "spi_tool_dev", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi,spi_tool_idtable);

// ------------- misc 描述 -----------------

#define BUF_SIZE 50

// static int spi_tool_open(struct inode * inode, struct file * filp)
// {
// 	char * buf_array=NULL;
// 	func_in();

// 	buf_array = kmalloc(BUF_SIZE,GFP_KERNEL);
// 	if(!buf_array)
// 		return -EINVAL;

// 	// 缓存指针存入 filp 的私有数据。
// 	filp->private_data = buf_array;

// 	func_out();
// 	return 0;
// }

// static int spi_tool_release(struct inode * inode, struct file * filp)
// {
// 	func_in();
// 	kfree(filp->private_data);
// 	func_out();
// 	return 0;
// }

// static ssize_t spi_tool_read(struct file * filp, char __user * up, size_t size, loff_t * off)
// {
// 	char rbuf[1]={0};
// 	func_in();
// 	spi_read(current_device,*rbuf,1);
// 	func_out();
// }

static ssize_t spi_tool_write(struct file * filp, const char __user * up, size_t size, loff_t * off)
{
	char *tbuf=NULL;
	int err=0;
	func_in();

	tbuf = kmalloc(size,GFP_KERNEL);
	if(!tbuf)
	{
		err = -EINVAL;
		goto out;
	}

	if(copy_from_user(tbuf,up,size))
	{
		err = -EINVAL;
		goto err0;
	}

	err = spi_write(current_device, tbuf, size);
	if(err)
	{
		dev_err(&current_device->dev, "Can't write SPI\n");
		err = -EINVAL;
		goto err0;
	}

	printk(KERN_INFO "write %d byte(s) \n",size);
	err = size;
	goto out;

err0:
	kfree(tbuf);
out:
	func_out();
	return err;
}

struct file_operations spi_tool_fops =
{
	.owner = THIS_MODULE,
	.write = spi_tool_write,
};

static struct miscdevice spi_tool_misc =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "spi_tool",
	.fops = &spi_tool_fops,
};

/**
 * 设备名匹配时的回调函数
 * @param  spi      设备spi结构体指针
 * @return          0或错误号
 */
int spi_tool_probe(struct spi_device *dev)
{
	func_in();

	current_device = dev;
	misc_register(&spi_tool_misc);

	func_out();
	return 0;
}

int spi_tool_remove(struct spi_device *dev)
{
	func_in();

	current_device = NULL;
	misc_deregister(&spi_tool_misc);

	func_out();
	return 0;
}

static struct spi_driver spi_tool_driver = {
	.driver = {
		.name	= "spi_tool_dri",
		.owner = THIS_MODULE,
	},

	.id_table	= spi_tool_idtable,
	.probe		= spi_tool_probe,
	.remove		= spi_tool_remove,
};

static int __init spi_tool_init(void)
{
	int err=0;
	struct spi_master *master = NULL;
	func_in();

	err = spi_register_driver(&spi_tool_driver);
	if(err)
	{
		printk(KERN_ERR "Can't get SPI driver \n");
		goto out;
	}

	master = spi_busnum_to_master(SPI_BUS_NUM);
	if(!master)
	{
		printk(KERN_ERR "Can't get SPI bus %d\n",SPI_BUS_NUM);
		err = -ENODEV;
		goto err0;
	}

	new_device = spi_new_device(master,spi_tool_dev_board_info);
	if(!new_device)
	{
		printk(KERN_ERR "Can't add SPI device \n");
		goto err0;
	}

	goto out;

err0:
	spi_unregister_driver(&spi_tool_driver);
out:
	func_out();
	return err;
}

static void __exit spi_tool_exit(void)
{
	func_in();
	
	if(new_device)
	{
		spi_unregister_device(new_device);
	}
	spi_unregister_driver(&spi_tool_driver);

	func_out();
}

module_init(spi_tool_init);
module_exit(spi_tool_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A spi_tool");
