#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include "camera.h"
#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define WIDTH  640
#define HEIGHT 480

int nobjects = 0;
model **object = NULL;

/* Load scene */
void make_scene(void) {
  static char *fns[] = {
    "cube1.model",
    "cube2.model",
    "cube3.model",
    "cube4.model"
  };
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

/* Get 2.5D Cohen-Sutherland clipping code */
short sutherland(point p) {
  short c = 0;
  if (POINT_GET(p,0) < 0)      c |= 0x1;
  if (POINT_GET(p,0) > WIDTH)  c |= 0x2;
  if (POINT_GET(p,1) < 0)      c |= 0x4;
  if (POINT_GET(p,1) > HEIGHT) c |= 0x8;
  if (POINT_GET(p,2) < 0.0001) c |= 0x10; /* behind */
  return c;
}

/* Draw wire-frame without occlusions */
void draw_scene(void) {
  int i, k;
  tmatrix p = camera_transform();

  /* transform all points in all objects */
  for (i = 0; i < nobjects; i++) {
    model_transform(p, object[i]);
    /* calculate and store code of each point */
    for (k = 0; k < object[i]->nvertices; k++) {
      union { scalar w; short c; } code;
      code.c = sutherland(object[i]->pvertex[k]);
      POINT_SET(object[i]->pvertex[k], 3, code.w);
    }
  }
  gui_clear();
  for (i = 0; i < nobjects; i++) {
    /* wire-frame specific code, relies on structure of model */
    for (k = 0; k < object[i]->nedges; k++) {
      union { scalar w; short c; } ac, bc;
      point a = object[i]->pvertex[object[i]->edge[(k<<1)+0]];
      point b = object[i]->pvertex[object[i]->edge[(k<<1)+1]];
      ac.w = POINT_GET(a,3);
      bc.w = POINT_GET(b,3);
      if (!(ac.c & bc.c) && !((ac.c | bc.c) & 0x10))
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

  if (ev.type & GUI_EVENT_ROLL) CAMERA_ROLL(ev.scale[k++] / 314.0);
  if (ev.type & GUI_EVENT_PITCH) CAMERA_PITCH(ev.scale[k++] / 314.0);
  if (ev.type & GUI_EVENT_YAW) CAMERA_YAW(ev.scale[k++] / 314.0);

  if (ev.type & GUI_EVENT_FOCUS) camera_focus_add(ev.scale[k++]);

  if (ev.type == GUI_EVENT_QUIT) scheduler_stop();
  if (k) draw_scene();
}

/* Main */
int main() {
  gui_init(WIDTH, HEIGHT);
  make_scene();
  camera_reset(WIDTH, HEIGHT, 1);
  draw_scene();
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  scheduler_main();
  gui_fin();
  clean_scene();
  return 0;
}
