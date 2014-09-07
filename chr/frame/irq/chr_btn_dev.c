/*
 *	GPIO中断测试
 *	注册按键到中断服务函数，并打印中断信息
 *	
 *	
 *	Copyright (C) 2014 concefly <h.wenjian@openrpi.org>
 *	Copyright (C) 2014 openRPi
 *	
 *		代码遵循GNU协议
 *	
 *	文档：http://www.openrpi.org/blogs/?p=296
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define debug 1

/**
 * 描述按键的结构体
 * 调用 init_btn_irq 填充 .irq成员
 */
struct btn_t
{
	int gpio_num;
	int irq;
	char *name;
};

static struct btn_t btn_list[] = {
	{4 ,0,"P1-7"},
	{17,0,"P1-11"},
	{18,0,"P1-12"},
	{27,0,"P1-13"},
	{22,0,"P1-15"},
	{23,0,"P1-16"},
	{24,0,"P1-18"},
	{25,0,"P1-22"},
};

/**
 * 打印按键信息
 * @param btn   头指针
 * @param count 要打印的个数
 */
void print_btn_info(const struct btn_t * btn, int count)
{
	int i=0;
	printk(KERN_INFO "@+ \n");

	for(i=0;i<count;i++,btn++)
	{
		printk(KERN_INFO "@ %d, %d, %s\n",
			btn->gpio_num,
			btn->irq,
			btn->name);
	}
	printk(KERN_INFO "@- \n");
}

/**
 * 按键中断服务函数
 * @param  irq 中断号
 * @param  dev 按键描述结构体指针
 * @return     处理状态，定义在 #include <linux/interrupt.h>
 */
static irqreturn_t btn_interrput(int irq, void *dev)
{
	struct btn_t * btn = (struct btn_t *)dev;

	printk(KERN_INFO "++ btn_interrput %d ++\n",irq);
	print_btn_info(btn,1);
	printk(KERN_INFO "-- btn_interrput --\n");
	return IRQ_HANDLED;
}

/**
 * 填充中断号
 * @param  btn   头指针
 * @param  count 个数
 * @return       0或错误号
 */
int init_btn_irq(struct btn_t * btn, int count)
{
	int i=0, err;
	for(i=0;i<count;i++,btn++)
	{
		err = gpio_to_irq(btn->gpio_num);
		if(err<0)
		{
			#if debug
			print_btn_info(btn,1);
			#endif
			return err;
		}
		btn->irq = err;
	}
	return 0;
}

/**
 * 根据按键描述结构体申请中断服务
 * @param  btn   头指针
 * @param  count 个数
 * @return       0或错误号
 */
int request_btn_irq(struct btn_t * btn, int count)
{
	int i=0, err;

	for(i=0;i<count;i++,btn++)
	{
		if(request_irq(	btn->irq,
						btn_interrput,
						// 上升沿触发
						IRQF_TRIGGER_RISING,
						btn->name,
						// 按键描述结构体指针。会传入给中断服务程序。
						btn))
		{
			#if debug
			print_btn_info(btn,1);
			#endif
			err = -EIO;
			goto err_free;
		}
	}
	return 0;

err_free:
	// 出错，则释放申请成功的中断号
	while(i--)
		free_irq(btn->irq,btn);
	return err;
}

/**
 * 根据按键描述结构体释放中断号
 * @param btn   头指针
 * @param count 个数
 */
void free_btn_irq(struct btn_t * btn, int count)
{
	int i;
	for(i=0;i<count;i++,btn++)
		free_irq(btn->irq,btn);
}

static int __init chr_btn_dev_init(void)
{
	int err;

	printk(KERN_INFO "++ chr_btn_dev init ++\n");

	// 填充中断号
	err = init_btn_irq(btn_list,ARRAY_SIZE(btn_list));
	if(err<0)
	{
		printk(KERN_ERR "Init IQR ERR\n");
		return err;
	}

	if((err=request_btn_irq(btn_list,ARRAY_SIZE(btn_list)))<0)
	{
		printk(KERN_ERR "Regsiter IQR ERR\n");
		return err;
	}

	printk(KERN_INFO "-- chr_btn_dev init --\n");
	return 0;
}

static void __exit chr_btn_dev_exit(void)
{
	printk(KERN_INFO "++ chr_btn_dev exit ++\n");

	free_btn_irq(btn_list,ARRAY_SIZE(btn_list));
	printk(KERN_INFO "Release IRQ \n");

	printk(KERN_INFO "-- chr_btn_dev exit --\n");
}

module_init(chr_btn_dev_init);
module_exit(chr_btn_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("h.wenjian@openrpi.org");
MODULE_DESCRIPTION("A chr_btn_dev");
