/* Handle colors with simple cache */
#ifndef _COLORS_H_
#define _COLORS_H_
#include <xcb/xcb.h>

#define CACHE_SIZE  1021

void colors_init(xcb_connection_t*, xcb_colormap_t);
uint32_t get_color_pixel(uint16_t R, uint16_t G, uint16_t B);

#endif/*_COLORS_H_*/
