/* Graphical interface */
#ifndef _GUI_H_
#define _GUI_H_

/* Initialisation and finalization */
int gui_init(int width, int height);
int gui_fin(void);

/* Get a descriptor to select/poll on */
int gui_fd(void);

/* Event type */
typedef enum {
  /* no events available */
  GUI_EVENT_NONE = 0,
  /* movements (observer frame) */
  GUI_EVENT_FWD,  GUI_EVENT_BACK,
  GUI_EVENT_LEFT, GUI_EVENT_RIGHT,
  GUI_EVENT_UP,   GUI_EVENT_DOWN,
  /* rotations (observer frame, RPY as for aircrafts) */
  GUI_EVENT_ROLL_CCW,   GUI_EVENT_ROLL_CW,
  GUI_EVENT_PITCH_DOWN, GUI_EVENT_PITCH_UP,
  GUI_EVENT_YAW_LEFT,   GUI_EVENT_YAW_RIGHT,
  /* error condition */
  GUI_EVENT_INVALID = -1
} gui_event_t;

/* Get next available event for controller */
gui_event_t gui_poll(void);

#endif/*_GUI_H_*/
