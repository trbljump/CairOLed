#include <cairo.h>
#include <qrencode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <iostream>
#include <vector>
#include <map>

#ifdef USE_X11
#include "cairo_x.h"
#else
#include "cairo_fb.h"
#endif /* USE_X11 */


/**
 * Crèe la surface Cairo correspondant à l'écran sur
 * /dev/fb1
 */
static cairo_t* createCairo() {
  cairo_surface_t* surface;
  cairo_t* cr ;
  double x1, y1, x2, y2 ;

#ifdef USE_X11
  surface = cairo_create_x11_surface(720, 480);
#else
  surface = cairo_linuxfb_surface_create("/dev/fb1");
#endif
  
  cr = cairo_create(surface) ;
  cairo_clip_extents(cr, &x1, &y1, &x2, &y2) ;

  if (y2 == 132 && x2 == 132) {
    cairo_matrix_t translator ;

    // Ceci est un patch pour reconnaître le cas de l'affichage
    // Waveshare 1.4" dont les deux premières lignes et la
    // première colonne sont invisibles. Le "Device Tree Overlay"
    // que nous utilisons déclare un écran de 132x132 pixels
    // et c'est suffisamment rare pour détecter ceci. #yeahright

    cairo_matrix_init_translate(&translator, 1, 2) ;
    cairo_set_matrix(cr, &translator) ;
    
    cairo_rectangle(cr, 0, 0, 128, 128) ;
    cairo_clip(cr) ;    
  }

  return cr ;
}

static bool drawQRCode(cairo_t* cr,
		       const char* message,
		       bool whiteFill=true,
		       int margin=4) {
  QRcode* code ;
  double x1, y1, x2, y2 ;
  int qrWidth ;
  int qrEffectiveWidth ;
  double scale ;
  double ox, oy ;
  uint32_t* qrImage ;
  cairo_surface_t* qrSurface ;
  cairo_matrix_t transform ;
  cairo_pattern_t* qrPattern ;
  bool result ;

  result = false ;

  if ((code = QRcode_encodeString(message, 0, QR_ECLEVEL_L, QR_MODE_8, 1))) {
    qrWidth = code->width ;
    qrEffectiveWidth = qrWidth + 2*margin ;

    std::cerr << "len=" << strlen(message)
	      << " version=" << code->version
	      << " size=" << qrWidth
	      << std::endl ;
  
    cairo_clip_extents(cr, &x1, &y1, &x2, &y2) ;

    if ((x2-x1) > (y2-y1)) {
      scale = floor((y2-y1)/(qrEffectiveWidth)) ;
    } else {
      scale = floor((x2-x1)/(qrEffectiveWidth)) ;
    }

    if (scale > 0) {
      cairo_save(cr) ;

      ox = x1 + ((x2-x1) - scale*qrWidth)/2.0 ;
      oy = y1 + ((y2-y1) - scale*qrWidth)/2.0 ;

      cairo_matrix_init_translate(&transform, ox, oy) ;  
      cairo_matrix_scale(&transform, scale, scale) ;
      cairo_set_matrix(cr, &transform) ;

      if (whiteFill) {
	cairo_set_source_rgba(cr, 1, 1, 1, 1) ;
	cairo_rectangle(cr, -margin, -margin,
			qrWidth+2*margin, qrWidth+2*margin) ;
	cairo_fill(cr) ;
      }

      qrImage = new uint32_t[qrWidth*qrWidth] ;
    
      uint32_t* p ;
      uint8_t* q ;
      int i ;
      
      for (p=qrImage,
	     q=code->data,
	     i=0 ; i < qrWidth*qrWidth; i++) {
	*p++ = (*q++ & 1) ? 0xff000000 : 0x00000000 ;
      }
      
      qrSurface= cairo_image_surface_create_for_data((unsigned char*)qrImage,
						     CAIRO_FORMAT_ARGB32,
						     qrWidth,
						     qrWidth,
						     qrWidth*sizeof(*qrImage)) ;

      qrPattern = cairo_pattern_create_for_surface(qrSurface) ;
      cairo_pattern_set_filter(qrPattern, CAIRO_FILTER_NEAREST) ;
      
      cairo_set_source(cr, qrPattern) ;
      cairo_paint(cr) ;
      
      delete qrImage ;
      cairo_pattern_destroy(qrPattern) ;
      cairo_surface_destroy(qrSurface) ;
      
      cairo_restore(cr) ;

      result = true ;
    } 
    QRcode_free(code) ;
  }

  return result ;
}

static void backDrop(cairo_t* cr, const char* filename) {
  cairo_surface_t* wifi ;
  cairo_pattern_t* wifiPattern ;
  int w, h ;
  double x1, y1, x2, y2 ;
  double xScale, yScale ;
  double ox, oy ;
  cairo_matrix_t transform ;
  
  wifi = cairo_image_surface_create_from_png(filename) ;
  w = cairo_image_surface_get_width(wifi) ;
  h = cairo_image_surface_get_height(wifi) ;

  cairo_clip_extents(cr, &x1, &y1, &x2, &y2) ;
  xScale = (x2-x1) / w ;
  yScale = (y2-y1) / h ;
  if (yScale < xScale) {
    xScale = yScale ;
  } else {
    yScale = xScale ;
  }
  
  cairo_save(cr) ;

  ox = x1 + ((x2-x1) - xScale*w)/2.0 ;
  oy = y1 + ((y2-y1) - yScale*h)/2.0 ;

  cairo_matrix_init_translate(&transform, ox, oy) ;  
  cairo_matrix_scale(&transform, xScale, yScale) ;
  cairo_set_matrix(cr, &transform) ;
  
  wifiPattern = cairo_pattern_create_for_surface(wifi) ;
  cairo_set_source(cr, wifiPattern) ;
  cairo_paint(cr) ;

  cairo_restore(cr) ;  
}

int main(int argc, char **argv) {
  cairo_t* cr ;

  cr = createCairo() ;

  cairo_set_source_rgb (cr, 1,1,1) ;
  cairo_paint(cr);

  backDrop(cr, "wifi.png") ;

  drawQRCode(cr,
	     // Cette chaîne de 32 bytes (SSID de 5 bytes
	     // et PSK de 10 bytes) donne un QR code de version 2
	     // (25x25 modules) qui "tient" sur un OLED 96x64.
	     //             01234   0123456789
	     "WIFI:T:WPA2;S:Xy2FA;P:U!(a)W*HG/",
	     false, 3) ;
  
  cairo_surface_flush(cairo_get_target(cr)) ;
#ifdef USE_X11
  cairo_flush_x11_surface(cairo_get_target(cr)) ;
#endif
  
  cairo_surface_destroy(cairo_get_target(cr));
  cairo_destroy(cr);

  return 0;
}

