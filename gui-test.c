#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include <stdio.h>
#include <math.h>

point cube[][4] = {
  { POINT( 2, 2, 2), POINT(20, 2, 2), POINT(20,20, 2), POINT( 2,20, 2) },
  { POINT( 2, 2,20), POINT(20, 2,20), POINT(20,20,20), POINT( 2,20,20) }
};
tmatrix view = TMATRIX_ONE;

void draw_line(point a, point b) {
  int x0, y0, x1, y1;
  a = transform(view, a);
  b = transform(view, b);
  x0 = POINT_GET(a, 0);
  y0 = POINT_GET(a, 1);
  x1 = POINT_GET(b, 0);
  y1 = POINT_GET(b, 1);
  gui_draw_line(x0,y0,x1,y1);
}

void printmatrix(tmatrix m) {
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++)
      printf("%8.4f ", TMATRIX_GET(m, i, j));
    printf("\n");
  }
}


void draw_scene() {
  int i;
  printmatrix(view); printf("--- \n");
  gui_clear();
  for (i = 0; i < 4; i++) draw_line(cube[0][i], cube[0][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cube[1][i], cube[1][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cube[0][i], cube[1][i]);
  gui_update();
}

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
  if (!ev.type) return; /* !! */

  for (i = 1; i; i <<= 1, j++)
    if (ev.type & i) {
      printf("(%04X)%s<%d>\n", ev.type&i, strings[j], k<4 ? ev.scale[k] : 0);
      switch (i) {
        /* FIXME: Obviously this isn't a correct use of controls */
        case GUI_EVENT_FORWARD:
          TMATRIX_SET(view, 1, 3, TMATRIX_GET(view,1,3)-ev.scale[k]);
          break;
        case GUI_EVENT_RIGHT:
          TMATRIX_SET(view, 0, 3, TMATRIX_GET(view,0,3)-ev.scale[k]);
          break;
        case GUI_EVENT_ROLL:
          {
            double a = ev.scale[k] / 180.0;
            tmatrix rz = TMATRIX(cos(a),-sin(a),0,0,sin(a),cos(a),0,0,0,0,1,0,0,0,0,1);
            view = tcompose(rz, view);
            break;
          }
        case GUI_EVENT_YAW:
          {
            double a = ev.scale[k] / 180.0;
            tmatrix ry = TMATRIX(cos(a),0,sin(a),0,0,1,0,0,-sin(a),0,cos(a),0,0,0,0,1);
            view = tcompose(ry, view);
            break;
          }
        case GUI_EVENT_PITCH:
          {
            double a = ev.scale[k] / 180.0;
            tmatrix rx = TMATRIX(1,0,0,0,0,cos(a),-sin(a),0,0,sin(a),cos(a),0,0,0,0,1);
            view = tcompose(rx, view);
            break;
          }
        default: break;
      }
      k++;
    }
  if (ev.type == GUI_EVENT_QUIT) scheduler_stop();
  draw_scene();
}

int main() {
  gui_init(640, 480);
  printf("fd = %d\n", gui_fd());
  scheduler_register(gui_fd(), SCHEDULER_FD_READ, &loop, NULL);
  draw_scene();
  scheduler_main();
  gui_fin();
  return 0;
}
