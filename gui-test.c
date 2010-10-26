#include "gui.h"
#include "scheduler.h"
#include <stdio.h>

void loop(void *x) {
  static const char *strings[] = {
    "GUI_EVENT_FORWARD",  "GUI_EVENT_RIGHT",  "GUI_EVENT_DOWN", "(reserved)",
    "GUI_EVENT_ROLL",     "GUI_EVENT_PITCH",  "GUI_EVENT_YAW",  "(reserved)",
    "GUI_EVENT_FOCUS",    "(reserved)",       "(reserved)",     "(reserved)",
    "(reserved)",         "(reserved)",       "(reserved)",     "GUI_EVENT_QUIT"
  };
  gui_event_t ev;
  int i, j = 0, k = 0;
  x = NULL;
  ev = gui_poll();
  /* printf("gui_poll -> %04X [%d %d %d]\n", ev.type, ev.scale[0], ev.scale[1], ev.scale[2]); */
  for (i = 1; i; i <<= 1, j++)
    if (ev.type & i)
      printf("(%04X)%s<%d>\n", ev.type&i, strings[j], k<4 ? ev.scale[k++] : 0);
  if (ev.type == GUI_EVENT_QUIT) scheduler_stop();
}

int main() {
  gui_init(640, 480);
  printf("fd = %d\n", gui_fd());
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  scheduler_main();
  gui_fin();
  return 0;
}
