#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include "camera.h"
#include "model.h"
#include <stdio.h>
#include <assert.h>

#define WIDTH  640
#define HEIGHT 480

/* Scene data */
scene *data;

/* Current polygon to draw */
gui_polygon *poly;

/* Load objects and build scene */
void make_scene(void) {
  /* models to load */
  static char *fns[] = {
    "cube.mesh",
    "cube.mesh",
    "cube.mesh",
    "cube.mesh",
    "sphere.mesh",
    "cyclic.mesh"
  };
  /* world transformation */
  static tmatrix tns[] = {
    TMATRIX(100,0,0, 100, 0,100, 0, 500, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0,-200, 0,100, 0, 500, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0,-200, 0,100, 0, 800, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0, 100, 0,100, 0, 800, 0, 0,100, -30, 0,0,0,1),
    TMATRIX( 50,0,0, 150, 0, 50, 0, 550, 0, 0, 50, -80, 0,0,0,1),
    TMATRIX( 40,0,0,-400, 0,  0,40, 540, 0,40,  0,-160, 0,0,0,1)
  };
  model *objects[sizeof fns / sizeof *fns];
  int i, j, n = (sizeof fns / sizeof *fns);
  FILE *fp;
  assert(sizeof tns / sizeof*tns == n);
  /* read models of objects */
  for (i = j = 0; i < n; i++) {
    if ((fp = fopen(fns[i],"rb")) != NULL) {
      int error = 0;
      objects[j] = model_read_mesh(fp, &error);
      fclose(fp);
      if (error)
        fprintf(stderr, "model_read_mesh error: %s: %d\n", fns[i], error);
      else {
        model_transform(tns[i], objects[j]);
        model_commit(objects[j]);
        j++;
      }
    }
    else
      fprintf(stderr, "fopen error: %s\n", fns[i]);
  }
  /* build scene */
  assert(j <= i);
  assert(j > 0);
  data = scene_build(objects, j);
  /* release models memory */
  while (j--)
    model_free(objects[j]);
}

/* Clean up scene */
void clean_scene(void) {
  scene_free(data);
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

/* Postprocessing of vertices */
void proc_vertex(vertex *vx) {
  union { scalar w; short c; } code;
  /* normalization */
  if (POINT_GET(vx->camera, 2) > 0)
    vx->camera = normalize(vx->camera);
  /* frustum clipping */
  code.c = sutherland(vx->camera);
  POINT_SET(vx->camera, 3, code.w);
}

/* Get orientation of a face */
float surforient(face *s) {
  point a, b, c;
  float x1, x2, x3, y1, y2, y3;
  a = s->v[0]->camera;
  b = s->v[1]->camera;
  c = s->v[2]->camera;
  x1 = POINT_GET(a,0);
  y1 = POINT_GET(a,1);
  x2 = POINT_GET(b,0);
  y2 = POINT_GET(b,1);
  x3 = POINT_GET(c,0);
  y3 = POINT_GET(c,1);
  return x2*y3-x1*y3-x3*y2+x1*y2+x3*y1-x2*y1;
}

static int Q;
/* Draw a single face if applicable */
void draw_face(face *s) {
  short i, d0, d1;
  Q += 6;
  /* ignore back-faces */
  if (surforient(s) <= 0.f) return;
  d0 = ~0;
  d1 =  0;
  /* Cohen-Sutherland-like visibility */
  for (i = 0; i < 3; i++) {
    union { scalar w; short c; } code;
    code.w = POINT_GET(s->v[i]->camera, 3);
    d0 &= code.c;
    d1 |= code.c;
  }
  /* ignore faces outside viewing frustum */
  if (d0 || d1 & 0x10) return;
  /* draw polygon */
  gui_polygon_clear(poly);
  for (i = 0; i < 3; i++)
    gui_polygon_add(poly,
        POINT_GET(s->v[i]->camera, 0),
        POINT_GET(s->v[i]->camera, 1));
  gui_draw_polygon_color(poly,
      0x00, Q&0xFF, 0xFF); /* FIXME */
}

/* Render scene */
void draw_scene(void) {
  scene_transform(camera_transform(), data);
  scene_vertices(data, &proc_vertex);
  gui_clear();
  Q = -6;
  scene_traverse(data, camera_position(), &draw_face);
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
