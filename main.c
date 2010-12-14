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

/* Scene */
int nobjects = 0;
model **object = NULL;

/* Polygon do draw */
gui_polygon *poly;

/* Load scene */
void make_scene(void) {
  static char *fns[] = {
    "cube1.model",
    "cube2.model",
    "cube3.model",
    "cube4.model",
    "sphere.model"
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
      object[nobjects] = model_read(fp);
      assert(object[nobjects]);
      fclose(fp);
      nobjects++;
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

/* Get orientation of surface */
float surforient(surface *s) {
  point a, b, c;
  float x1, x2, x3, y1, y2, y3;
  a = s->edge->vertex->camera;
  b = s->edge->next->vertex->camera;
  c = s->edge->next->next->vertex->camera;
  x1 = POINT_GET(a,0);
  y1 = POINT_GET(a,1);
  x2 = POINT_GET(b,0);
  y2 = POINT_GET(b,1);
  x3 = POINT_GET(c,0);
  y3 = POINT_GET(c,1);
  return x2*y3-x1*y3-x3*y2+x1*y2+x3*y1-x2*y1;
}

/* Draw wire-frame without occlusions */
/* this code relies on structure of model */
void draw_scene(void) {
  int i, k;
  tmatrix p = camera_transform();

  /* transform all points in all objects */
  for (i = 0; i < nobjects; i++) {
    model_transform(p, object[i]);
    /* calculate and store code of each point */
    for (k = 0; k < object[i]->nvertices; k++) {
      union { scalar w; short c; } code;
      code.c = sutherland(object[i]->vertices[k].camera);
      POINT_SET(object[i]->vertices[k].camera, 3, code.w);
    }
  }
  gui_clear();
  for (i = 0; i < nobjects; i++) {
    for (k = 0; k < object[i]->nsurfaces; k++) {
      if (surforient(object[i]->surfaces+k) < 0) {
        halfedge *e = object[i]->surfaces[k].edge;
        halfedge *x = e;
        short d0 = ~0, d1 = 0; /* Cohen-Sutherland visibility */
        gui_polygon_clear(poly);
        do {
          union { scalar w; short c; } code;
          point vx = x->vertex->camera;
          code.w = POINT_GET(vx, 3);
          d0 &= code.c;
          d1 |= code.c;
          gui_polygon_add(poly, POINT_GET(vx,0), POINT_GET(vx,1));
          x = x->next;
        } while (x != e);
        if (!d0 && !(d1 & 0x10))
          gui_draw_polygon_color(poly,
              /* FIXME: make light-like coloring */
              0x7F,
              255*(object[i]->nsurfaces-k)/object[i]->nsurfaces,
              255*i/nobjects);
      }
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
  poly = gui_polygon_alloc(3);
  draw_scene();
  fprintf(stderr, "Ready.\n");
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  scheduler_main();
  gui_fin();
  clean_scene();
  gui_polygon_free(poly);
  return 0;
}
