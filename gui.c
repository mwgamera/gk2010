#include "gui.h"
#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <assert.h>

static xcb_connection_t *xc = NULL;
static xcb_screen_t *xs = NULL;
static xcb_window_t win;
static xcb_keysym_t keymap[256];
static xcb_pixmap_t buffer;
static xcb_gcontext_t blit_gc, draw_gc;
static int width, height;

/* Cerate main window with proper attributes */
static xcb_window_t _main_window(xcb_connection_t *xc, xcb_screen_t *xs, int w, int h) {
  xcb_window_t window;
  uint32_t values[2];
  xcb_cw_t mask;

  window = xcb_generate_id(xc);

  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = xs->black_pixel;
  values[1] = XCB_EVENT_MASK_KEY_PRESS
    | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
    | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE;

  xcb_create_window(xc, xs->root_depth, window, xs->root, 0, 0, w, h, 0, 
      XCB_WINDOW_CLASS_INPUT_OUTPUT, xs->root_visual, mask, values);
  return window;
}

/* Initialise mapping of keysyms to their basic keycodes (ignoring modifiers) */
static xcb_keysym_t *_init_keymap(xcb_connection_t *xc) {
  xcb_get_keyboard_mapping_reply_t *reply;
  xcb_keysym_t *syms;
  int i, j, l;
  reply = xcb_get_keyboard_mapping_reply(xc, xcb_get_keyboard_mapping(xc, 8, 248), NULL);
  syms = xcb_get_keyboard_mapping_keysyms(reply);
  l = xcb_get_keyboard_mapping_keysyms_length(reply);
  for (i = 8, j = 0; i < 255 && j < l; j += reply->keysyms_per_keycode, i++)
    keymap[i] = syms[j];
  free(reply);
  return keymap;
}

/* Initialise pixmap buffer */
static void _init_buf(xcb_connection_t *xc, xcb_screen_t *xs, int w, int h) {
  buffer  = xcb_generate_id(xc);
  blit_gc = xcb_generate_id(xc);
  draw_gc = xcb_generate_id(xc);
  xcb_create_pixmap(xc, xs->root_depth, buffer, xs->root, w, h);
  xcb_create_gc(xc, blit_gc, xs->root, 0, NULL);
  xcb_create_gc(xc, draw_gc, buffer, 0, NULL);
  gui_clear();
}

/* Initialise all */
int gui_init(int w, int h) {
  assert(!xc);
  width = w;
  height = h;
  xc = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(xc)) return -1;
  xs = xcb_setup_roots_iterator(xcb_get_setup(xc)).data;
  win = _main_window(xc, xs, w, h);
  _init_buf(xc, xs, w, h);
  _init_keymap(xc);
  xcb_map_window(xc, win);
  xcb_flush(xc);
  return 0;
}

/* Close and clean up */
int gui_fin() {
  assert(xc);
  if (buffer) xcb_free_pixmap(xc, buffer);
  if (blit_gc) xcb_free_gc(xc, blit_gc);
  if (draw_gc) xcb_free_gc(xc, draw_gc);
  xcb_flush(xc);
  xcb_disconnect(xc);
  xc = NULL;
  return 0;
}

/* Return a descriptor to poll on */
int gui_fd() {
  assert(xc);
  return xcb_get_file_descriptor(xc);
}

/* Flush data to server */
void gui_update() {
  assert(xc);
  xcb_copy_area(xc, buffer, win, blit_gc,
      0, 0, 0, 0, width, height);
  xcb_flush(xc);
}

/* Handle expose event - redraw screen */
static void _event_expose(xcb_expose_event_t *e) {
  xcb_copy_area(xc, buffer, win, blit_gc,
      e->x, e->y, e->x, e->y,
      e->width, e->height);
  xcb_flush(xc);
}

#define DIVSQRT2(x) (((x)*46341)>>16) /* ~= x / sqrt(2) */

/* Handle keyboard events */
static gui_event_t _event_kbd(xcb_keycode_t code) {
  gui_event_t r = { GUI_EVENT_NONE, {0, 0, 0} };
  assert(code >= 8);
  switch (keymap[code]) {
    /* horizontal movement (WSAD/arrows) */
    case XK_w: case XK_Up:
      r.type = GUI_EVENT_FORWARD;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    case XK_s: case XK_Down:
      r.type = GUI_EVENT_FORWARD;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_a: case XK_Left:
      r.type = GUI_EVENT_RIGHT;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_d: case XK_Right:
      r.type = GUI_EVENT_RIGHT;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    /* pitch/yaw rotations (HJKL+YUBN) */
    case XK_k:
      r.type = GUI_EVENT_PITCH;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    case XK_j:
      r.type = GUI_EVENT_PITCH;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_h:
      r.type = GUI_EVENT_YAW;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_l:
      r.type = GUI_EVENT_YAW;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    case XK_y:
      r.type = GUI_EVENT_PITCH | GUI_EVENT_YAW;
      r.scale[0] = +DIVSQRT2(GUI_SCALE_KBD);
      r.scale[1] = -DIVSQRT2(GUI_SCALE_KBD);
      break;
    case XK_u:
      r.type = GUI_EVENT_PITCH | GUI_EVENT_YAW;
      r.scale[0] = +DIVSQRT2(GUI_SCALE_KBD);
      r.scale[1] = +DIVSQRT2(GUI_SCALE_KBD);
      break;
    case XK_b:
      r.type = GUI_EVENT_PITCH | GUI_EVENT_YAW;
      r.scale[0] = -DIVSQRT2(GUI_SCALE_KBD);
      r.scale[1] = -DIVSQRT2(GUI_SCALE_KBD);
      break;
    case XK_n:
      r.type = GUI_EVENT_PITCH | GUI_EVENT_YAW;
      r.scale[0] = -DIVSQRT2(GUI_SCALE_KBD);
      r.scale[1] = +DIVSQRT2(GUI_SCALE_KBD);
      break;
    /* vertical movement (TG) */
    case XK_t:
      r.type = GUI_EVENT_DOWN;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_g:
      r.type = GUI_EVENT_DOWN;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    /* rolls (<>) */
    case XK_comma:
      r.type = GUI_EVENT_ROLL;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    case XK_period:
      r.type = GUI_EVENT_ROLL;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    /* focus */
    case XK_plus: case XK_equal: case XK_KP_Add:
      r.type = GUI_EVENT_FOCUS;
      r.scale[0] = +GUI_SCALE_KBD;
      break;
    case XK_minus: case XK_KP_Subtract:
      r.type = GUI_EVENT_FOCUS;
      r.scale[0] = -GUI_SCALE_KBD;
      break;
    /* quit */
    case XK_Escape:
      r.type = GUI_EVENT_QUIT;
      break;
    default:
      break;
  }
  return r;
}

#define MODIFIER_BUTTON1 0x100
#define MODIFIER_BUTTON2 0x200
#define MODIFIER_BUTTON3 0x400

/* Handle mouse events */
static gui_event_t _event_ptr(xcb_motion_notify_event_t *e) {
  static int16_t last_x, last_y;
  gui_event_t r = { GUI_EVENT_NONE, { 0, 0, 0 } };
  int k, i = 0;
  if (e->state & MODIFIER_BUTTON3) { /* right button - planar movement */
    k = (last_y - e->event_y) * GUI_SCALE_PRT;
    if (k) { r.type |= GUI_EVENT_FORWARD; r.scale[i++] = k; }
    k = (e->event_x - last_x) * GUI_SCALE_PRT;
    if (k) { r.type |= GUI_EVENT_RIGHT; r.scale[i++] = k; }
  } else
  if (e->state & MODIFIER_BUTTON1) { /* left button - phi,theta rotation */
    k = (last_y - e->event_y) * GUI_SCALE_PRT;
    if (k) { r.type |= GUI_EVENT_PITCH; r.scale[i++] = k; }
    k = (e->event_x - last_x) * GUI_SCALE_PRT;
    if (k) { r.type |= GUI_EVENT_YAW; r.scale[i++] = k; }
  }
  if (e->state & MODIFIER_BUTTON2) { /* middle button - focal length */
    r.type |= GUI_EVENT_FOCUS;
    r.scale[i++] = (e->event_y - last_y) * GUI_SCALE_PRT;
  }
  assert(i <= 4);
  last_x = e->event_x;
  last_y = e->event_y;
  return r;
}

/* Process events, return requests for higher layer */
gui_event_t gui_poll() {
  xcb_generic_event_t *e = NULL;
  gui_event_t r = { GUI_EVENT_NONE, { 0, 0, 0 } };
  assert(xc);
  if (xcb_connection_has_error(xc)) {
    r.type = GUI_EVENT_QUIT;
    return r;
  }
  while (!r.type && (e = xcb_poll_for_event(xc)) != NULL) {
    switch (e->response_type & ~0x80) {
      /* navigation with pointing device */
      case XCB_MOTION_NOTIFY:
        r = _event_ptr((xcb_motion_notify_event_t*)e);
        break;
      case XCB_BUTTON_PRESS:
        _event_ptr((xcb_motion_notify_event_t*)e);
        xcb_flush(xc);
        break;
      /* keyboard navigation */
      case XCB_KEY_PRESS:
        r = _event_kbd(((xcb_key_press_event_t*)e)->detail);
        break;
      /* events handled internally */
      case XCB_EXPOSE:
        _event_expose((xcb_expose_event_t*)e);
        break;
      default:
        break;
    }
    free(e);
  }
  if (!r.type && xcb_connection_has_error(xc))
    r.type = GUI_EVENT_QUIT;
  return r;
}

/* Clear screen */
xcb_gcontext_t gui_gc_clear = 0;
void gui_clear() {
  xcb_rectangle_t rect;
  uint32_t mask = XCB_GC_FOREGROUND;
  uint32_t value;
  assert(xc);
  xcb_change_gc(xc, draw_gc, mask, &value);
  rect.x = 0;
  rect.y = 0;
  rect.width = width;
  rect.height = height;
  xcb_poly_fill_rectangle(xc, buffer, draw_gc, 1, &rect);
}

/* Draw line */
void gui_draw_line(int x0, int y0, int x1, int y1) {
  xcb_point_t p[2];
  uint32_t mask = XCB_GC_FOREGROUND;
  uint32_t value;
  assert(xc);
  value = xs->white_pixel;
  xcb_change_gc(xc, draw_gc, mask, &value);
  p[0].x = x0; p[0].y = y0;
  p[1].x = x1; p[1].y = y1;
  xcb_poly_line(xc, XCB_COORD_MODE_ORIGIN, buffer, draw_gc, 2, p);
}

