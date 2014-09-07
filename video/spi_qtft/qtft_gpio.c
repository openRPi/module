
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/gpio.h>

#define GPIO_RESET 17
#define GPIO_DC 18

struct gpio gpio_list[] = 
{
	{GPIO_RESET, GPIOF_OUT_INIT_HIGH, "qtft gpio RESET"},
	{GPIO_DC   , GPIOF_OUT_INIT_LOW , "qtft gpio DC"},
};

/**
 * qtft gpio 初始化
 * 在 spi probe() 中被调用
 * 
 * @return  0或错误号
 */
int qtft_gpio_init(void)
{
	int err=0;

	err = gpio_request_array(gpio_list,ARRAY_SIZE(gpio_list));
	if(err)
	{
		printk(KERN_ERR "Can't request gpio\n");
		goto out;
	}

	goto out;

out:
	return err;
}

/**
 * qtft gpio 释放
 * 在 spi remove 中被调用
 *
 * 注意：若 qtft_gpio_init() 失败，则此函数不会被调用
 */
void qtft_gpio_exit(void)
{
	gpio_free_array(gpio_list,ARRAY_SIZE(gpio_list));
}

void qtft_gpio_set_reset(int x)
{
	gpio_set_value(GPIO_RESET, x);
}

void qtft_gpio_set_dc(int x)
{
	gpio_set_value(GPIO_DC, x);
}