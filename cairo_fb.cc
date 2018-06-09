#include "cairo_fb.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

void cairo_linuxfb_surface_destroy(void *device) {
  cairo_linuxfb_device_t *dev = (cairo_linuxfb_device_t *)device;

  if (dev) {
    fprintf(stderr, "Surface for fb at %p destroyed\n", dev->fb_data) ;
    munmap(dev->fb_data, dev->fb_screensize);
    close(dev->fb_fd) ;
    free(dev) ;
  }
}

cairo_surface_t *cairo_linuxfb_surface_create(const char *fb_name) {
  cairo_linuxfb_device_t *device ;
  cairo_surface_t *surface ;

  if (fb_name == NULL) {
    fb_name = "/dev/fb0" ;
  }

  surface = NULL ;

  device = (cairo_linuxfb_device_t*)malloc(sizeof(*device)) ;
  if (device) {
    if ((device->fb_fd = open(fb_name, O_RDWR)) >= 0) {
      if (ioctl(device->fb_fd, FBIOGET_VSCREENINFO, &device->fb_vinfo) >= 0) {
	device->fb_screensize = device->fb_vinfo.xres * device->fb_vinfo.yres
	  * device->fb_vinfo.bits_per_pixel / 8;

	device->fb_data = (uint8_t *)mmap(0, device->fb_screensize,
					  PROT_READ | PROT_WRITE, MAP_SHARED,
					  device->fb_fd, 0);
	if (device->fb_data) {
	  cairo_format_t pixel_format ;

	  // TODO: prendre en compte les fb 32 bits
	  pixel_format = CAIRO_FORMAT_RGB16_565 ;
	  
	  if (ioctl(device->fb_fd, FBIOGET_FSCREENINFO, &device->fb_finfo) >= 0) {
	    surface = cairo_image_surface_create_for_data(device->fb_data,
							  pixel_format,
							  device->fb_vinfo.xres,
							  device->fb_vinfo.yres,
							  cairo_format_stride_for_width(pixel_format, device->fb_vinfo.xres)) ;
	  
	    cairo_surface_set_user_data(surface, NULL, device,
					&cairo_linuxfb_surface_destroy);
	  } else {
	    perror("FBIOGET_FSCREENINFO") ;
	    munmap(device->fb_data, device->fb_screensize) ;
	  close_device:
	    close(device->fb_fd) ;
	  }
	} else {
	  perror("mmap") ;
	  goto close_device ;
	}
      } else {
	perror("FBIOGET_VSCREENINFO") ;
	goto close_device ;
      }
    } else {
      perror(fb_name) ;
    }
  }

  return surface ;
}
