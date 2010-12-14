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

/* Polygon to draw */
gui_polygon *poly;

/* List of visible surfaces to sort */
surface **vorder;

/* Load scene */
void make_scene(void) {
  /* list of models to load */
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
  /* allocate list of objects */
  object = malloc(n * sizeof*object);
  if (object == NULL) {
    fprintf(stderr, "malloc error\n");
    return;
  }
  /* read models of objects */
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
  /* count surfaces and allocate sort list */
  for (n = i = 0; i < nobjects; i++)
    n += object[i]->nsurfaces;
  vorder = malloc(n * sizeof*vorder);
  assert(vorder);
}

/* Clean up scene */
void clean_scene(void) {
  assert(nobjects > 0);
  while (nobjects--)
    model_free(object[nobjects]);
  free(object);
  free(vorder);
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

/* Compare two surfaces in drawing order */
int surfcmp(const void *a, const void *b) {
  halfedge *e;
  scalar az1, az2, az3;
  scalar bz1, bz2, bz3;

  e = (*(surface**)a)->edge;
  az1 = POINT_GET(e->vertex->camera,2);
  e = e->next;
  az2 = POINT_GET(e->vertex->camera,2);
  e = e->next;
  az3 = POINT_GET(e->vertex->camera,2);

  e = (*(surface**)b)->edge;
  bz1 = POINT_GET(e->vertex->camera,2);
  e = e->next;
  bz2 = POINT_GET(e->vertex->camera,2);
  e = e->next;
  bz3 = POINT_GET(e->vertex->camera,2);

  /* barycenter depth difference */
  return (az1+az2+az3 -bz1-bz2-bz3);
}

/* Render scene */
void draw_scene(void) {
  int i, j, k;
  union { scalar w; short c; } code;
  tmatrix p = camera_transform();

  /* transform all points in all objects */
  for (i = 0; i < nobjects; i++) {
    model_transform(p, object[i]);
    /* calculate and store code of each point */
    for (k = 0; k < object[i]->nvertices; k++) {
      scalar z = POINT_GET(object[i]->vertices[k].camera, 2);
      if (z > 0)
        object[i]->vertices[k].camera = normalize(object[i]->vertices[k].camera);
      code.c = sutherland(object[i]->vertices[k].camera);
      POINT_SET(object[i]->vertices[k].camera, 3, code.w);
      POINT_SET(object[i]->vertices[k].camera, 2, z);
    }
  }
  /* prepare list of possibly visible surfaces */
  j = 0;
  for (i = 0; i < nobjects; i++) {
    for (k = 0; k < object[i]->nsurfaces; k++) {
      /* orientation based visibility */
      if (surforient(object[i]->surfaces+k) < 0) {
        halfedge *e = object[i]->surfaces[k].edge;
        halfedge *x = e;
        short d0 = ~0, d1 = 0; 
        do {
          point vx = x->vertex->camera;
          code.w = POINT_GET(vx, 3);
          d0 &= code.c;
          d1 |= code.c;
          x = x->next;
        } while (x != e);
        /* Cohen-Sutherland-like visibility */
        if (!d0 && !(d1 & 0x10))
          vorder[j++] = object[i]->surfaces+k;
      }
    }
  }
  /* sort surfaces into drawing order */
  qsort(vorder, j, sizeof*vorder, &surfcmp);
  /* draw surfaces on screen in order */
  gui_clear();
  while (j--) {
    halfedge *e = vorder[j]->edge;
    halfedge *x = e;
    gui_polygon_clear(poly);
    do {
      point vx = x->vertex->camera;
      gui_polygon_add(poly, POINT_GET(vx,0), POINT_GET(vx,1));
      x = x->next;
    } while (x != e);
    gui_draw_polygon_color(poly,
        0x00, (255*j/nobjects)&0xFF, 0xFF); /* FIXME */
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
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  fprintf(stderr, "Ready.\n");
  scheduler_main();
  gui_fin();
  clean_scene();
  gui_polygon_free(poly);
  return 0;
}
