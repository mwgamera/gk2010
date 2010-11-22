#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include "camera.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int nobjects = 0;
model **object = NULL;

/* Load scene */
void make_scene(void) {
  static char *fns[] = { "cube1.model", "cube2.model" };
  FILE *fp;
  int i, n = (sizeof fns / sizeof *fns);
  assert(nobjects == 0);
  object = malloc(n * sizeof*object);
  if (object == NULL) {
    fprintf(stderr, "malloc error\n");
    return;
  }
  for (i = 0; i < n; i++) {
    if ((fp = fopen(fns[i],"rb")) != NULL) {
      object[nobjects++] = model_read(fp);
      fclose(fp);
    }
    else
      fprintf(stderr, "fopen error : %s\n", fns[nobjects]);
  }
  assert(nobjects <= i);
  assert(nobjects > 0);
}

/* Clean up scene */
void clean_scene(void) {
  assert(nobjects > 0);
  while (nobjects--)
    model_free(object[nobjects]);
  free(object);
}

/* Draw wire-frame without occlusions */
void draw_scene(void) {
  int i, k;
  tmatrix p = camera_transform();
  for (i = 0; i < nobjects; i++)
    model_transform(p, object[i]);
  gui_clear();
  for (i = 0; i < nobjects; i++) {
    /* wire-frame specific code, relies on structure of model */
    for (k = 0; k < object[i]->nedges; k++) {
      point a = object[i]->pvertex[object[i]->edge[(k<<1)+0]];
      point b = object[i]->pvertex[object[i]->edge[(k<<1)+1]];
      if (POINT_GET(a,2) >= 0 && POINT_GET(b,2) >= 0)
        gui_draw_line(
            POINT_GET(a,0),
            POINT_GET(a,1),
            POINT_GET(b,0),
            POINT_GET(b,1));
    }
  }
  gui_update();
}

/* Event loop */
void loop(void *x) {
  int k = 0;
  gui_event_t ev = gui_poll();
  x = NULL;
  if (!ev.type) return;

  if (ev.type & GUI_EVENT_FORWARD) CAMERA_FORWARD(ev.scale[k++]);
  if (ev.type & GUI_EVENT_RIGHT) CAMERA_RIGHT(ev.scale[k++]);
  if (ev.type & GUI_EVENT_DOWN) CAMERA_DOWN(ev.scale[k++]);

  if (ev.type & GUI_EVENT_ROLL) CAMERA_ROLL(ev.scale[k++] / 180.0);
  if (ev.type & GUI_EVENT_PITCH) CAMERA_PITCH(ev.scale[k++] / 180.0);
  if (ev.type & GUI_EVENT_YAW) CAMERA_YAW(ev.scale[k++] / 180.0);

  if (ev.type & GUI_EVENT_FOCUS) camera_focus_add(ev.scale[k++]);

  if (ev.type == GUI_EVENT_QUIT) scheduler_stop();
  if (k) draw_scene();
}

/* Main */
int main() {
  gui_init(640, 480);
  make_scene();
  camera_reset(640, 480, 1);
  draw_scene();
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  scheduler_main();
  gui_fin();
  clean_scene();
  return 0;
}
