#ifndef __CAIRO_FB_H__
#define __CAIRO_FB_H__

#include <cairo.h>
#include <stdint.h>

#include <linux/fb.h>
#include <sys/ioctl.h>

typedef struct _cairo_linuxfb_device {
  int fb_fd;
  uint8_t *fb_data;
  long fb_screensize;
  struct fb_var_screeninfo fb_vinfo;
  struct fb_fix_screeninfo fb_finfo;
} cairo_linuxfb_device_t;

extern void cairo_linuxfb_surface_destroy(void *device) ;
extern cairo_surface_t *cairo_linuxfb_surface_create(const char *fb_name) ;


#endif /* __CAIRO_FB_H__ */
