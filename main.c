#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include "camera.h"
#include "model.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

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

    /* cyclic occlusion demo */
    "triangle.mesh",
    "triangle.mesh",
    "triangle.mesh",
    "triangle.mesh"
  };
  /* world transformation */
  static tmatrix tns[] = {
    TMATRIX(100,0,0, 100, 0,100, 0, 500, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0,-200, 0,100, 0, 500, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0,-200, 0,100, 0, 800, 0, 0,100, -30, 0,0,0,1),
    TMATRIX(100,0,0, 100, 0,100, 0, 800, 0, 0,100, -30, 0,0,0,1),
    TMATRIX( 50,0,0, 150, 0, 50, 0, 550, 0, 0, 50, -80, 0,0,0,1),

    TMATRIX( 40,  0,  0,  0,  0, 40,  0,1300,  0,  0, 40,-80, 0,0,0,1),
    TMATRIX(  0,  0,-40, 80,  0, 40,  0,1300, 40,  0,  0,  0, 0,0,0,1),
    TMATRIX(-40,  0,  0,  0,  0, 40,  0,1300,  0,  0,-40, 80, 0,0,0,1),
    TMATRIX(  0,  0, 40,-80,  0, 40,  0,1300,-40,  0,  0,  0, 0,0,0,1)
  };
  /* colors (uv) */
  static float clrs[][2] = {
    {-42.116,  22.187},
    {-21.701, -59.268},
    { 82.556,  49.946},
    { 36.871,  30.384},
    {137.094,  34.108},

    {-35.319, 56.595},
    {-39.325, 12.866},
    {  2.768,-84.294},
    { 83.483, 36.735}
  };
  model *objects[sizeof fns / sizeof *fns];
  int i, j, n = (sizeof fns / sizeof *fns);
  FILE *fp;
  assert(sizeof tns / sizeof*tns == n);
  assert(sizeof clrs/ sizeof*clrs== n);
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
        model_userdata(objects[j], clrs + i);
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
  scalar z = POINT_GET(vx->camera, 2);
  union { scalar w; short c; } code;
  /* normalization */
  if (POINT_GET(vx->camera, 2) > 0)
    vx->camera = normalize(vx->camera);
  /* frustum clipping */
  code.c = sutherland(vx->camera);
  POINT_SET(vx->camera, 3, code.w);
  POINT_SET(vx->camera, 2, z);
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

/* Square of distance to barycenter of face */
float face_distsq(face *s) {
  int i;
  float d = 0.f;
  for (i = 0; i < 3; i++)
    d += POINT_GET(s->v[i]->camera,2) / 3.0;
  return d * d;
}

/* Color heuristic */
#define REF_U 0.197839824821408
#define REF_V 0.468336302932410
void close_color_poly(face *fx) {
  float u, v, l;
  float x, y, z, r, g, b;
  int R, G, B;
  point normal;
  u = ((float*)fx->userdata)[0];
  v = ((float*)fx->userdata)[1];
  /* heuristic */
  normal = direction(pointplane(
        fx->v[0]->camera,
        fx->v[1]->camera,
        fx->v[2]->camera));
  r = POINT_GET(normal,2) / 2;
  r = sqrt(r < 0 ? -r : r);
  u *= r; v *= r;
  l = 1249997500 / (face_distsq(fx) + 24999750);
  l *= r;
  if (l < 10.f) l = 10.f;
  /* conversion uv->rgb */
  y = (l+16) / 116;
  if (y*y*y > 0.008856) y = y*y*y;
  else y = (y-16/116) / 7.787;
  u = u / (13*l) + REF_U;
  v = v / (13*l) + REF_V;
  y *= 100.f;
  x = -(9*y*u) / ((u-4)*v - u*v);
  z = (9*y-(15*v*y) - (v*x)) / (3*v);
  x /= 100.f; y /= 100.f; z /= 100.f;
  r = x *  3.2406 + y * -1.5372 + z * -0.4986;
  g = x * -0.9689 + y *  1.8758 + z *  0.0415;
  b = x *  0.0557 + y * -0.2040 + z *  1.0570;
  if ( r > 0.0031308 ) r = 1.055 * exp(log(r)/2.4) - 0.055;
  else r = 12.92 * r;
  if ( g > 0.0031308 ) g = 1.055 * exp(log(g)/2.4) - 0.055;
  else g = 12.92 * g;
  if ( b > 0.0031308 ) b = 1.055 * exp(log(b)/2.4) - 0.055;
  else b = 12.92 * b;
  R = 0xFF * r; if (R>0xFF) R = 0xFF; if (R < 0) R = 0;
  G = 0xFF * g; if (G>0xFF) G = 0xFF; if (G < 0) G = 0;
  B = 0xFF * b; if (B>0xFF) B = 0xFF; if (B < 0) B = 0;
  /* draw */
  gui_draw_polygon_color(poly, R, G, B);
}

/* Draw a single face if applicable */
void draw_face(face *s) {
  short i, d0, d1;
  float cx, cy;
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
  cx = cy = 0.f;
  for (i = 0; i < 3; i++) {
    cx += POINT_GET(s->v[i]->camera, 0) / 3.f;
    cy += POINT_GET(s->v[i]->camera, 1) / 3.f;
  }
  for (i = 0; i < 3; i++) {
    float x, y;
    x = POINT_GET(s->v[i]->camera, 0);
    y = POINT_GET(s->v[i]->camera, 1);
    gui_polygon_add(poly,
        (int)(x + (x < cx ? 0 : +1.f)),
        (int)(y + (y < cy ? 0 : +1.f)));
  }
  /* determine color and draw */
  close_color_poly(s);
}

/* Render scene */
void draw_scene(void) {
  scene_transform(camera_transform(), data);
  scene_vertices(data, &proc_vertex);
  gui_clear();
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
