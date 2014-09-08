#ifndef __lcd_lib__
#define __lcd_lib__

// -------------- init & sleep & display ----------------

/**
 * 初始化硬件，应该第一个调用
 * @return  0或错误号
 */
extern int lcd_init(void);
extern void lcd_exit(void);

extern int lcd_soft_reset(void);
extern int lcd_hard_reset(void);

/**
 * 以常用的配置初始化
 * @return  0或错误号
 */
extern int lcd_normal_config(void);

extern int lcd_sleep_in(int delay);
extern int lcd_sleep_out(void);

extern int lcd_display_on(void);
extern int lcd_display_off(void);

// ---------------- pixel ------------------

#define PIXEL_FORMAT_16 0x55
#define PIXEL_FORMAT_18 0x66
extern int lcd_pixel_format_set(int mode);

// ------------ address access ------------------

extern int lcd_column_address_set(int x1, int x2);
extern int lcd_page_address_set(int y1, int y2);

#define lcd_address_set(x1, y1, x2, y2)	do \
										{	\
											lcd_column_address_set(x1,x2); \
											lcd_page_address_set(y1,y2); \
										}while(0)

// 初始化光标位置到 area 的开头
extern int lcd_cursor_reset(void);

// -------------- memory access ----------------

#define MEMORY_ACCESS_MY 		(1<<7)
#define MEMORY_ACCESS_MX 		(1<<6)
#define MEMORY_ACCESS_MV 		(1<<5)
#define MEMORY_ACCESS_ML 		(1<<4)
#define MEMORY_ACCESS_BGR		(1<<3)
#define MEMORY_ACCESS_MH 		(1<<2)
#define MEMORY_ACCESS_NORMAL	(MEMORY_ACCESS_MY | MEMORY_ACCESS_MX | MEMORY_ACCESS_MV | MEMORY_ACCESS_ML | MEMORY_ACCESS_BGR)

extern int lcd_memory_access_control(int mode);

/**
 * 先调用 lcd_address_set() 明确 area，否则结果未知。
 * @param  _continue 从上次结束的地址开始写
 * @return           实际接收的字节数或错误号
 */
extern int lcd_memory_area_write(const unsigned char *buf, int size, int _continue);

/**
 * 默认 area 0,0,320,240
 */
extern int lcd_memory_write(const unsigned char *buf, int size, int _continue);
extern int lcd_memory_write_from(int x, int y, const unsigned char *buf, int size);

/**
 * 先调用 lcd_address_set() 明确 area，否则结果未知。
 * @param  _continue 从上次结束的地址开始写
 * @return           实际接收的字节数或错误号
 */
extern int lcd_memory_area_read(unsigned char *buf, int size, int _continue);

/**
 * 默认 area 0,0,320,240
 */
extern int lcd_memory_read(unsigned char *buf, int size, int _continue);

// ------------------ Power control -----------------

extern int lcd_power_contral_a(int reg_vd, int vbc);
extern int lcd_power_contral_b(int pc, int dc_ena);

#endif
