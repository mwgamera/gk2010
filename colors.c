#include "colors.h"
#include <string.h>
#include <stdlib.h>

xcb_connection_t *xc;
xcb_colormap_t cmap;

static struct {
  uint16_t r;
  uint16_t g;
  uint16_t b;
  uint32_t pixel;
} cache[CACHE_SIZE];

void colors_init(xcb_connection_t *conn, xcb_colormap_t colormap) {
  xc = conn; cmap = colormap;
  memset(cache, 0, CACHE_SIZE * sizeof*cache);
}

static int cache_hash(uint16_t r, uint16_t g, uint16_t b) {
  uint32_t h = 2463534242;
  h ^= (uint32_t)0xFFFF + b - g;
  h ^= h << 13; h ^= h >> 17; h ^= h << 5;
  h ^= (uint32_t)0xFFFF + r - g;
  h ^= h << 13; h ^= h >> 17; h ^= h << 5;
  h ^= g + (((uint32_t)r + b) >> 1);
  return (4091 * h) % CACHE_SIZE;
}

uint32_t get_color_pixel(uint16_t r, uint16_t g, uint16_t b) {
  int p = cache_hash(r,g,b);
  if (cache[p].r == r && cache[p].g == g && cache[p].b == b)
    return cache[p].pixel;
  else {
    xcb_alloc_color_cookie_t cookie;
    xcb_alloc_color_reply_t *reply;
    cookie = xcb_alloc_color(xc, cmap, r, g, b);
    reply = xcb_alloc_color_reply(xc, cookie, NULL);
    if (reply != NULL) {
      cache[p].r = r;
      cache[p].g = g;
      cache[p].b = b;
      cache[p].pixel = reply->pixel;
      free(reply);
      return cache[p].pixel;
    }
  }
  return 0;
}
