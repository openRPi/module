#ifndef __qtft_gpio__
#define __qtft_gpio__

extern int qtft_gpio_init(void);
extern void qtft_gpio_exit(void);

extern void qtft_gpio_set_reset(int x);
extern void qtft_gpio_set_dc(int x);

#endif 