#ifndef __CAIRO_X_H__
#define __CAIRO_X_H__

#include <cairo.h>

/*! Simple Cairo/Xlib example.
 * @author Bernhard R. Fischer, 2048R/5C5FFD47 <bf@abenteuerland.at>.
 * @version 2014110801
 * Compile with
 * gcc -Wall $(pkg-config --libs --cflags cairo x11) -o cairo_xlib_simple cairo_xlib_simple.c
 */

/*! Check for Xlib Mouse/Keypress events. All other events are discarded. 
 * @param sfc Pointer to Xlib surface.
 * @param block If block is set to 0, this function always returns immediately
 * and does not block. if set to a non-zero value, the function will block
 * until the next event is received.
 * @return The function returns 0 if no event occured (and block is set). A
 * positive value indicates that a key was pressed and the X11 key symbol as
 * defined in <X11/keysymdef.h> is returned. A negative value indicates a mouse
 * button event. -1 is button 1 (left button), -2 is the middle button, and -3
 * the right button.
 */
extern int cairo_check_event(cairo_surface_t *sfc, int block) ;

/*! Open an X11 window and create a cairo surface base on that window.
 * @param x Width of window.
 * @param y Height of window.
 * @return Returns a pointer to a valid Xlib cairo surface. The function does
 * not return on error (exit(3)).
 */
extern cairo_surface_t *cairo_create_x11_surface(int x, int y) ;

/*! Destroy cairo Xlib surface and close X connection.
 */
extern void cairo_close_x11_surface(cairo_surface_t *sfc) ;

extern void cairo_flush_x11_surface(cairo_surface_t* sfc) ;


#endif /* __CAIRO_H__H */
