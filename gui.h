/* Graphical interface */
#ifndef _GUI_H_
#define _GUI_H_

/* Initialisation and finalization */
int gui_init(int width, int height);
int gui_fin(void);

/* Update GUI, should be called after doing any changes */
void gui_update(void);

/* Get a descriptor to select/poll on */
int gui_fd(void);

/* Event type */
typedef enum {
  /* nothing happens */
  GUI_EVENT_NONE = 0,
  /* movements (observer frame) */
  GUI_EVENT_FORWARD = 0x001,
  GUI_EVENT_RIGHT   = 0x002,
  GUI_EVENT_DOWN    = 0x004,
  /* rotations (observer frame, RPY as for aircrafts) */
  GUI_EVENT_ROLL  = 0x010, /* clockwise */
  GUI_EVENT_PITCH = 0x020, /* look up */
  GUI_EVENT_YAW   = 0x040, /* look right */
  /* zoom (changes in focal length) */
  GUI_EVENT_FOCUS = 0x100,
  /* quit requested by user or caused by error condition */
  GUI_EVENT_QUIT = 0x8000
} gui_event_type_t;

/* Event structure */
/* From controller's point of view, events are user's request to change some
 * properties of camera.  Multiple changes may be requested in single event but
 * GUI should be built in a way that makes composed events always defined (for
 * example simultaneous pitch and yaw are okay) */
typedef struct {
  gui_event_type_t type; /* directions of changes (bit mask) */
  int scale[3]; /* scale for each of change types (one for each bit, max 3) */
} gui_event_t;

#define GUI_SCALE_KBD 20
#define GUI_SCALE_PRT 1

/* Get next available event for controller */
gui_event_t gui_poll(void);


/* Darwing functions */
void gui_clear(void);
void gui_draw_line(int,int,int,int); 
void gui_draw_line_color(int,int,int,int,
    unsigned char, unsigned char, unsigned char);

/* Polygon structure */
/* Proxy to hide structure of XCB data types and yet copy vertices once.
 * A function call might be optimized out by compiler when doing full
 * program optimization yielding good performance.  */
typedef struct _gui_polygon gui_polygon;

gui_polygon *gui_polygon_alloc(int);
void gui_polygon_free(gui_polygon*);
void gui_polygon_clear(gui_polygon*);
int gui_polygon_add(gui_polygon*, int, int);
void gui_draw_polygon_color(gui_polygon*,
    unsigned char, unsigned char, unsigned char);

#endif/*_GUI_H_*/
