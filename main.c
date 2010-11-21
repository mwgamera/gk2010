#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include "camera.h"
#include <stdio.h>

/* Projective line drawing */
void draw_line(tmatrix p, point a, point b) {
  int x0, y0, x1, y1;
  a = transform(p, a);
  b = transform(p, b);
  if (POINT_GET(a,2) < 0 || POINT_GET(b,2) < 0) return;
  a = normalize(a);
  b = normalize(b);
  x0 = POINT_GET(a, 0); y0 = POINT_GET(a, 1);
  x1 = POINT_GET(b, 0); y1 = POINT_GET(b, 1);
  gui_draw_line(x0,y0,x1,y1);
}

/* Redraw */
void draw_scene() {
  tmatrix p = camera_transform();
  gui_clear();
  { /* FIXME: Example cube */
    point cube[][4] = {
      { POINT( 0, 0, 0), POINT(60, 0, 0), POINT(60,60, 0), POINT( 0,60, 0) },
      { POINT( 0, 0,60), POINT(60, 0,60), POINT(60,60,60), POINT( 0,60,60) }
    };
    tmatrix w = TMATRIX(1,0,0,0, 0,1,0,330, 0,0,1,170, 0,0,0,1);
    int i;
    w = tcompose(p, w);
    for (i = 0; i < 4; i++) draw_line(w, cube[0][i], cube[0][(i+1)%4]);
    for (i = 0; i < 4; i++) draw_line(w, cube[1][i], cube[1][(i+1)%4]);
    for (i = 0; i < 4; i++) draw_line(w, cube[0][i], cube[1][i]);
  }
  { /* FIXME: Floor grid */
    int i, j;
    tmatrix w = TMATRIX(1,0,0,0, 0,1,0,360, 0,0,1,260, 0,0,0,1);
    w = tcompose(p, w);
    for (i = -10; i < 10; i++)
      for (j = -10; j < 10; j++) {
        point a = POINT((i+0)*50, (j+0)*50, 0);
        point b = POINT((i+0)*50, (j+1)*50, 0);
        point c = POINT((i+1)*50, (j+1)*50, 0);
        draw_line(w, a, b);
        draw_line(w, b, c);
      }
  }
  gui_update();
}

/* Event loop */
void loop(void *x) {
  int i, k = 0;
  gui_event_t ev = gui_poll();
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
  camera_reset(640, 480, 1);
  draw_scene();
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  scheduler_main();
  gui_fin();
  return 0;
}
