
#include <linux/fb.h>

struct fb_var_screeninfo qtft_fb_var_default = {
	.xres           = 320,
	.yres           = 240,
	.xres_virtual   = 320,
	.yres_virtual   = 240,
	.bits_per_pixel = 16,

	.red   = { 5, 11, 0 },
	.green = { 6, 5, 0 },
	.blue  = { 5, 0, 0 },

	.activate =	FB_ACTIVATE_NOW,
	.height   = -1,
	.width    = -1,
	.pixclock = 20000,

	.left_margin  = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len    = 0,
	.vsync_len    = 0,

	.vmode = FB_VMODE_NONINTERLACED,
};

struct fb_fix_screeninfo qtft_fb_fix_default = {
	.id        = "SPI QVGA TFT LED",
	// Packed Pixels
	.type      = FB_TYPE_PACKED_PIXELS,
	// True color
	.visual    = FB_VISUAL_TRUECOLOR,
	.xpanstep  = 1,
	.ypanstep  = 1,
	.ywrapstep = 1,
	.accel     = FB_ACCEL_NONE,
};