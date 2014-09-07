#ifndef __qtft_spi__
#define __qtft_spi__

extern int qtft_spi_init(void);
extern void qtft_spi_exit(void);

extern int qtft_spi_write(const void *buf, size_t len);
extern int qtft_spi_read(void *buf, size_t len);
extern int qtft_spi_write_then_read(const void *tbuf, size_t tn, void *rbuf, size_t rn);

#endif 