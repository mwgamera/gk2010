#include "gui.h"
#include <stdint.h>
#include <xcb/xcb.h>
#include <X11/keysymdef.h>
#include <stdlib.h>
#include <assert.h>

static xcb_connection_t *xc = NULL;
static xcb_screen_t *xs = NULL;
static xcb_window_t win;
static xcb_keysym_t keymap[256]; 

/* Cerate main window with proper attributes */
static xcb_window_t _main_window(xcb_connection_t *xc, int w, int h) {
  xcb_window_t window;
  uint32_t values[2];
  xcb_cw_t mask;
  window = xcb_generate_id(xc);
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = xs->black_pixel;
  values[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_BUTTON_PRESS
    | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
    | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_EXPOSURE;
  xcb_create_window(xc, xs->root_depth, window, xs->root, 0, 0, w, h, 0, 
      XCB_WINDOW_CLASS_INPUT_OUTPUT, xs->root_visual, mask, values);
  return window;
}

/* Initialise mapping of keysyms to their basic keycodes (ignoring modifiers) */
static xcb_keysym_t *_init_keymap(xcb_connection_t *xc) {
  xcb_get_keyboard_mapping_reply_t *reply;
  xcb_keysym_t *syms;
  int i;
  reply = xcb_get_keyboard_mapping_reply(xc, xcb_get_keyboard_mapping(xc, 8, 248), NULL);
  syms = xcb_get_keyboard_mapping_keysyms(reply);
  i = xcb_get_keyboard_mapping_keysyms_length(reply) >> 3;
  while (i--)
    keymap[i+8] = syms[i<<3];
  free(reply);
  return keymap;
}

/* Initialise all */
int gui_init(int w, int h) {
  assert(!xc);
  xc = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(xc)) return -1;
  xs = xcb_setup_roots_iterator(xcb_get_setup(xc)).data;
  win = _main_window(xc, w, h);
  xcb_map_window(xc, win);
  _init_keymap(xc);
  return 0;
}

#include <stdio.h> /*FIXME printf */
/* Close and clean up */
int gui_fin() {
  assert(xc);
  fprintf(stderr,"gui_fin\n");
  xcb_disconnect(xc);
  xc = NULL;
  return 0;
}

/* Return a descriptor to poll on */
int gui_fd() {
  assert(xc);
  return xcb_get_file_descriptor(xc);
}

#include <unistd.h> /* FIXME sleep */
/* Process events, return request for higher layer */
gui_event_t gui_poll() {
  xcb_generic_event_t *e;
  assert(xc);
  while ((e = xcb_poll_for_event(xc)) != NULL) {
    switch (e->response_type & ~0x80) {
      case XCB_KEY_PRESS:
        {
        xcb_key_press_event_t *kp = (xcb_key_press_event_t*) e;
        printf("Key %d -> %d '%c'\n", kp->detail, keymap[kp->detail], keymap[kp->detail]);
        break;
        }
      case XCB_EXPOSE:
        printf("Expose\n");
        break;
      default:
        break;
    }
    free(e);
  }
  if (xcb_connection_has_error(xc))
    return GUI_EVENT_INVALID;
  xcb_flush(xc);
  /* TODO Somehow returning something other than INVALID yields leaks --investigate */
  return GUI_EVENT_NONE;
}

