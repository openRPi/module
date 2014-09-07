// ILI9341 芯片
// 

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/delay.h>

#include "qtft_spi.h"
#include "qtft_gpio.h"

#include "lcd_lib.h"

enum flag_t {
	flag_data,
	flag_cmd,
};

/**
 * 延时毫秒
 * 可重写
 * 
 * @param ms 毫秒
 */
// void delay_ms(int ms)
// {
// 	;
// }
#define delay_ms(ms) msleep(ms)

/**
 * 底层接口初始化
 * 依赖于硬件，可重写
 * 
 * @return  0或错误号
 */
int iface_init(void)
{
	int err=0;

	qtft_gpio_set_reset(1);
	delay_ms(5);
	qtft_gpio_set_reset(0);
	delay_ms(20);
	qtft_gpio_set_reset(1);
	delay_ms(120);

	goto out;

out:
	return err;
}

/**
 * 释放底层接口。
 * 但这里不需要干什么
 */
void iface_exit(void)
{
	;
}

/**
 * 底层接口的数据传输函数
 * 依赖于硬件，可重写
 * 
 * @param  tbuf 发送数组
 * @param  tn   发送字节数
 * @param  rbuf 接收数组
 * @param  rn   接收字节数
 * @param  flag 标示符（自定义的其他操作）
 * @return      0或错误号
 */
int iface_write_then_read(const void *tbuf, int tn, void *rbuf, int rn, enum flag_t flag)
{
	int err=0;

	switch(flag)
	{
		case flag_cmd: 
			qtft_gpio_set_dc(0);
			break;

		case flag_data:
			qtft_gpio_set_dc(1);
			break;

		default:
			err = -EPERM;
			goto out;
	}

	if(tn)
	{
		// bcm2835_spi_writenb((char *)tbuf,tn);
		err = qtft_spi_write(tbuf,tn);
		if(err)
			goto out;
	}

	if(rn)
	{
		// bcm2835_spi_transfern((char *)rbuf,rn);
		err = qtft_spi_read(rbuf,tn);
		if(err)
			goto out;
	}

	goto out;

out:
	return err;
}

int w8(unsigned char value, enum flag_t flag)
{
	return iface_write_then_read(&value, 1, NULL, 0, flag);
}

int r8(unsigned char *value, enum flag_t flag)
{
	return iface_write_then_read(NULL, 0, value, 1, flag);
}

int wc8_then_wd8(unsigned char cmd, unsigned char data)
{
	int err=0;
	err = w8(cmd, flag_cmd);
	if(err)
		return err;
	err = w8(data, flag_data);
	if(err)
		return err;
	return 0;
}

int wc8_then_wdbuf(unsigned char cmd, const unsigned char *buf, int size)
{
	int err=0;
	err = w8(cmd, flag_cmd);
	if(err)
		return err;
	err = iface_write_then_read(buf, size, NULL, 0, flag_data);
	if(err)
		return err;
	return 0;
}

int wc8_then_rdbuf(unsigned char cmd, unsigned char *buf, int size)
{
	int err=0;
	err = w8(cmd, flag_cmd);
	if(err)
		return err;
	err = iface_write_then_read(NULL, 0, buf, size, flag_data);
	if(err)
		return err;
	return 0;
}

int lcd_sleep_in(int delay)
{
	int err=0;
	err = w8(0x10,flag_cmd);
	if(err || !delay)
		return err;
	delay_ms(delay);
	return 0;
}

int lcd_sleep_out(void)
{
	return w8(0x11, flag_cmd);
}

int lcd_memory_access_control(int mode)
{
	return wc8_then_wd8(0x36, mode);
}

int lcd_pixel_format_set(int mode)
{
	return wc8_then_wd8(0x3a, mode);
}

int lcd_display_off(void)
{
	return w8(0x28,flag_cmd);
}

int lcd_display_on(void)
{
	return w8(0x29, flag_cmd);
}

int lcd_memory_area_write(int x1, int y1, int x2, int y2, const unsigned char *buf, int size)
{
	lcd_address_set(x1, y1, x2, y2);
	return wc8_then_wdbuf(0x2c, buf, size);
}

int lcd_column_address_set(int x1, int x2)
{
	unsigned char tbuf[4];
	tbuf[0] = (x1 >> 8) & 0xff;
	tbuf[1] = x1 & 0xff;
	tbuf[2] = (x2 >> 8) & 0xff;
	tbuf[3] = x2 & 0xff;
	return wc8_then_wdbuf(0x2a, tbuf, 4);
}

int lcd_page_address_set(int y1, int y2)
{
	unsigned char tbuf[4];
	tbuf[0] = (y1 >> 8) & 0xff;
	tbuf[1] = y1 & 0xff;
	tbuf[2] = (y2 >> 8) & 0xff;
	tbuf[3] = y2 & 0xff;
	return wc8_then_wdbuf(0x2b, tbuf, 4);
}

int lcd_memory_area_read(int x1, int y1, int x2, int y2, unsigned char *buf, int size)
{
	int err=0;
	int area_size=(x2-x1+1)*(y2-y1+1);
	int min = size<area_size ? size : area_size;

	lcd_address_set(x1,y1,x2,y2);
	err = wc8_then_rdbuf(0x2e, buf, min);
	return err? err : min;
}

int lcd_power_contral_a(int reg_vd, int vbc)
{
	unsigned char buf[5] = {0x39,0x2c,0x00,
		0x34|(reg_vd&0x07),0x02|(vbc&0x07)};

	return wc8_then_wdbuf(0xcb, buf, 5);
}

int lcd_power_contral_b(int pc, int dc_ena)
{
	unsigned char buf[5] = {0x39,0x2c,0x00,
		0x81|((pc<<3)&0x18), dc_ena? 0x40:0x30};

	return wc8_then_wdbuf(0xcf, buf, 5);
}

int lcd_soft_reset(void)
{
	return w8(0x01, flag_cmd);
}

int lcd_init(void)
{
	int err=0;

	err = iface_init();
	if(err)
		return err;

	return 0;
}

int lcd_init_normal(void)
{
	#define return_err(func) do{int err=func; if(err){return err;}}while(0)

	return_err( lcd_init() );
	return_err( lcd_memory_access_control(MEMORY_ACCESS_NORMAL) );
	return_err( lcd_pixel_format_set(PIXEL_FORMAT_16) );
	return_err( lcd_power_contral_a(0,0) );
	return_err( lcd_power_contral_b(0,0) );

	#define LCD_wr_reg(value) w8(value,flag_cmd)
	#define LCD_wr_data8(value) w8(value,flag_data)

    LCD_wr_reg(0xCF);  
    LCD_wr_data8(0x00); 
    LCD_wr_data8(0XC1); 
    LCD_wr_data8(0X30); 

    LCD_wr_reg(0xE8);  
    LCD_wr_data8(0x85); 
    LCD_wr_data8(0x00); 
    LCD_wr_data8(0x78); 

    LCD_wr_reg(0xEA);  
    LCD_wr_data8(0x00); 
    LCD_wr_data8(0x00); 

    LCD_wr_reg(0xED);  
    LCD_wr_data8(0x64); 
    LCD_wr_data8(0x03); 
    LCD_wr_data8(0X12); 
    LCD_wr_data8(0X81); 

    LCD_wr_reg(0xF7);  
    LCD_wr_data8(0x20); 

    LCD_wr_reg(0xC0);    //Power control 
    LCD_wr_data8(0x23);   //VRH[5:0] 

    LCD_wr_reg(0xC1);    //Power control 
    LCD_wr_data8(0x10);   //SAP[2:0];BT[3:0] 

    LCD_wr_reg(0xC5);    //VCM control 
    LCD_wr_data8(0x3e); //对比度调节
    LCD_wr_data8(0x28); 

    LCD_wr_reg(0xC7);    //VCM control2 
    LCD_wr_data8(0x86);  //--

    LCD_wr_reg(0xB1);    
    LCD_wr_data8(0x00);  
    LCD_wr_data8(0x18); 

    LCD_wr_reg(0xB6);    // Display Function Control 
    LCD_wr_data8(0x08); 
    LCD_wr_data8(0x82);
    LCD_wr_data8(0x27);  

    LCD_wr_reg(0xF2);    // 3Gamma Function Disable 
    LCD_wr_data8(0x00); 

    LCD_wr_reg(0x26);    //Gamma curve selected 
    LCD_wr_data8(0x01); 

    LCD_wr_reg(0xE0);    //Set Gamma 
    LCD_wr_data8(0x0F); 
    LCD_wr_data8(0x31); 
    LCD_wr_data8(0x2B); 
    LCD_wr_data8(0x0C); 
    LCD_wr_data8(0x0E); 
    LCD_wr_data8(0x08); 
    LCD_wr_data8(0x4E); 
    LCD_wr_data8(0xF1); 
    LCD_wr_data8(0x37); 
    LCD_wr_data8(0x07); 
    LCD_wr_data8(0x10); 
    LCD_wr_data8(0x03); 
    LCD_wr_data8(0x0E); 
    LCD_wr_data8(0x09); 
    LCD_wr_data8(0x00); 

    LCD_wr_reg(0XE1);    //Set Gamma 
    LCD_wr_data8(0x00); 
    LCD_wr_data8(0x0E); 
    LCD_wr_data8(0x14); 
    LCD_wr_data8(0x03); 
    LCD_wr_data8(0x11); 
    LCD_wr_data8(0x07); 
    LCD_wr_data8(0x31); 
    LCD_wr_data8(0xC1); 
    LCD_wr_data8(0x48); 
    LCD_wr_data8(0x08); 
    LCD_wr_data8(0x0F); 
    LCD_wr_data8(0x0C); 
    LCD_wr_data8(0x31); 
    LCD_wr_data8(0x36); 
    LCD_wr_data8(0x0F); 

    #undef LCD_wr_reg 
	#undef LCD_wr_data8 

	return_err( lcd_sleep_out() );
	delay_ms(100);

	return_err( lcd_display_on() );

	#undef return_err

	return 0;
}

void lcd_exit(void)
{
	iface_exit();
}