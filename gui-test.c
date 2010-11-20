#include "gui.h"
#include "scheduler.h"
#include "space.h"
#include <stdio.h>
#include <math.h>

point cube[][4] = {
  { POINT( 6, 6, 6), POINT(60, 6, 6), POINT(60,60, 6), POINT( 6,60, 6) },
  { POINT( 6, 6,60), POINT(60, 6,60), POINT(60,60,60), POINT( 6,60,60) }
};
point cub2[][4] = {
  { POINT(180, 6, 0), POINT(240, 6, 0), POINT(240,60, 0), POINT(180,60, 0) },
  { POINT(180, 6,60), POINT(240, 6,60), POINT(240,60,60), POINT(180,60,60) }
};
/* camera orientation */
point c_x = POINT(1,0,0);
point c_y = POINT(0,-1,0);
point c_z = POINT(0,0,1);
/* camera position */
point cc = POINT(0,0,-300);
/* focus */
float f = 300.f;

tmatrix view;

void printmatrix(tmatrix m) {
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++)
      printf("%8.4f ", TMATRIX_GET(m, i, j));
    printf("\n");
  }
}

void draw_line(point a, point b) {
  int x0, y0, x1, y1;
  tmatrix x = TMATRIX(1,0,0, 640/2 ,0,1,0, 480/2 ,0,0,1, 0 ,0,0,0,1); /* center screen */
  tmatrix p = TMATRIX_ONE; /* perspective */
  /**/ TMATRIX_SET(p,3,2,1./f); /**/
  /**/ TMATRIX_SET(p,3,3,0); /**/
  /** /
  TMATRIX_SET(p,3,2,1);
  TMATRIX_SET(p,3,3,0);
  TMATRIX_SET(p,2,2,1./(1.-f));
  TMATRIX_SET(p,2,3,-f/(1.-f));
  /**/
  /** / printmatrix(p); /**/
  /**/
  x = tcompose(x, tcompose(p, view)); /**/
  /** / x = tcompose(x, view); /**/
  /*
  printf("[%8.4f %8.4f %8.4f]%8.4f -> [%8.4f %8.4f %8.4f]%8.4f\n",
      POINT_GET(a, 0), POINT_GET(a, 1), POINT_GET(a, 2), POINT_GET(a, 3),
      POINT_GET(b, 0), POINT_GET(b, 1), POINT_GET(b, 2), POINT_GET(b, 3));
  a = transform(view, a);
  b = transform(view, b);
  printf("[%8.4f %8.4f %8.4f]%8.4f -> [%8.4f %8.4f %8.4f]%8.4f <R\n",
      POINT_GET(a, 0), POINT_GET(a, 1), POINT_GET(a, 2), POINT_GET(a, 3),
      POINT_GET(b, 0), POINT_GET(b, 1), POINT_GET(b, 2), POINT_GET(b, 3));
  a = transform(p, a);
  b = transform(p, b);
  printf("[%8.4f %8.4f %8.4f]%8.4f -> [%8.4f %8.4f %8.4f]%8.4f <P\n",
      POINT_GET(a, 0), POINT_GET(a, 1), POINT_GET(a, 2), POINT_GET(a, 3),
      POINT_GET(b, 0), POINT_GET(b, 1), POINT_GET(b, 2), POINT_GET(b, 3));
      */
  a = transform(x, a);
  b = transform(x, b);
  /**/ if (POINT_GET(a,2) < f || POINT_GET(b,2) < f) return; /* it's behind pplane */
  a = normalize(a);
  b = normalize(b);
  x0 = POINT_GET(a, 0);
  y0 = POINT_GET(a, 1);
  x1 = POINT_GET(b, 0);
  y1 = POINT_GET(b, 1);
  if (POINT_GET(a,2) < 0.f || POINT_GET(b,2) < 0.f)
  {
    double dx = x1-x0, dy = y1-y0;
    double l = 15;
    int i;
    for (i=0; i < l; i++) {
      gui_draw_line(
          x0+(dx*(double)i)/l,
          y0+(dy*(double)i)/l,
          x0+(dx*(double)i+dx/2.)/l,
          y0+(dy*(double)i+dy/2.)/l);
    }
  }
  else
  gui_draw_line(x0,y0,x1,y1);
}

void draw_line_xyz(float x1,float y1,float z1,float x2,float y2,float z2) {
  point p1 = POINT(x1,y1,z1);
  point p2 = POINT(x2,y2,z2);
  draw_line(p1, p2);
}

tmatrix rot(point u, double a) {
  tmatrix r = TMATRIX_ONE;
  double s = sin(a);
  double c = cos(a);
  double ic= 1.f - c;
  double x = POINT_GET(u,0);
  double y = POINT_GET(u,1);
  double z = POINT_GET(u,2);
  double xx = x * x;
  double yy = y * y;
  double zz = z * z;
  double xy = x * y;
  double xz = x * z;
  double yz = y * z;
  u = direction(u);
  TMATRIX_SET(r,0,0, c+xx*ic);
  TMATRIX_SET(r,0,1, xy*ic-z*s);
  TMATRIX_SET(r,0,2, xz*ic+y*s);
  TMATRIX_SET(r,1,0, xy*ic+z*s);
  TMATRIX_SET(r,1,1, c+yy*ic);
  TMATRIX_SET(r,1,2, yz*ic-x*s);
  TMATRIX_SET(r,2,0, xz*ic-y*s);
  TMATRIX_SET(r,2,1, yz*ic+x*s);
  TMATRIX_SET(r,2,2, c+zz*ic);
  return r;
}

void draw_scene() {
  int i, j;
  TMATRIX_SET(view,0,0,POINT_GET(c_x,0));
  TMATRIX_SET(view,0,1,POINT_GET(c_x,1));
  TMATRIX_SET(view,0,2,POINT_GET(c_x,2));
  TMATRIX_SET(view,0,3,0.f);
  TMATRIX_SET(view,1,0,POINT_GET(c_y,0));
  TMATRIX_SET(view,1,1,POINT_GET(c_y,1));
  TMATRIX_SET(view,1,2,POINT_GET(c_y,2));
  TMATRIX_SET(view,1,3,0.f);
  TMATRIX_SET(view,2,0,POINT_GET(c_z,0));
  TMATRIX_SET(view,2,1,POINT_GET(c_z,1));
  TMATRIX_SET(view,2,2,POINT_GET(c_z,2));
  TMATRIX_SET(view,2,3,0.f);
  TMATRIX_SET(view,3,0,0.f);
  TMATRIX_SET(view,3,1,0.f);
  TMATRIX_SET(view,3,2,0.f);
  TMATRIX_SET(view,3,3,1.f);
  { /*shift*/
    tmatrix s = TMATRIX_ONE;
    TMATRIX_SET(s,0,3,-POINT_GET(cc,0));
    TMATRIX_SET(s,1,3,-POINT_GET(cc,1));
    TMATRIX_SET(s,2,3,-POINT_GET(cc,2));
    view = tcompose(view, s);
  }
  printf("CAMERA AT [%8.4f %8.4f %8.4f]%8.4f\n",
      POINT_GET(cc, 0), POINT_GET(cc, 1), POINT_GET(cc, 2), POINT_GET(cc, 3));
  { /* debug */
    point z = POINT_ZERO;
    point o = POINT(1,1,1);
    z = transform(view, z);
    o = transform(view, o);
    printf("[0 0 0] -> [%8.4f %8.4f %8.4f] %8.4f\n",
        POINT_GET(z,0),
        POINT_GET(z,1),
        POINT_GET(z,2),
        POINT_GET(z,3)); 
    printf("[1 1 1] -> [%8.4f %8.4f %8.4f] %8.4f\n",
        POINT_GET(o,0),
        POINT_GET(o,1),
        POINT_GET(o,2),
        POINT_GET(o,3)); 
  }
  printmatrix(view); printf("--- \n");
  gui_clear();
  for (i = 0; i < 4; i++) draw_line(cube[0][i], cube[0][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cube[1][i], cube[1][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cube[0][i], cube[1][i]);
  draw_line_xyz(6,6,6,60,60,60);

  for (i = 0; i < 4; i++) draw_line(cub2[0][i], cub2[0][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cub2[1][i], cub2[1][(i+1)%4]);
  for (i = 0; i < 4; i++) draw_line(cub2[0][i], cub2[1][i]);

  draw_line_xyz(0,0,0, 10,0,0);
  draw_line_xyz(0,0,0, 0,10,0); draw_line_xyz(0,15,0, 0,20,0);
  draw_line_xyz(0,0,0, 0,0,10); draw_line_xyz(0,0,15, 0,0,20); draw_line_xyz(0,0,25, 0,0,30);

  /* grid */ /**/
  for (i = -10; i < 10; i++)
    for (j = -10; j < 10; j++) {
      draw_line_xyz(i*30, 0, j*30, (i+1)*30, 0, j*30);
      draw_line_xyz(i*30, 0, j*30, i*30, 0, (j+1)*30);
      draw_line_xyz((i+1)*30, 0, j*30, (i+1)*30, 0, (j+1)*30);
      draw_line_xyz(i*30, 0, (j+1)*30, (i+1)*30, 0, (j+1)*30);
    }
  /**/
  gui_update();
}

void loop(void *x) {
  static const char *strings[] = {
    "GUI_EVENT_FORWARD",  "GUI_EVENT_RIGHT",  "GUI_EVENT_DOWN", "(reserved#4)",
    "GUI_EVENT_ROLL",     "GUI_EVENT_PITCH",  "GUI_EVENT_YAW",  "(reserved#8)",
    "GUI_EVENT_FOCUS",    "(reserved#10)",    "(reserved#11)",  "(reserved#12)",
    "(reserved#13)",      "(reserved#14)",    "(reserved#15)",  "GUI_EVENT_QUIT"
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
          /* TMATRIX_SET(view, 1, 3, TMATRIX_GET(view,1,3)-ev.scale[k]); */
          /* POINT_SET(cc,2,POINT_GET(cc,2)-ev.scale[k]); */
          POINT_SET(cc,0,POINT_GET(cc,0) + ev.scale[k]*POINT_GET(c_z,0));
          POINT_SET(cc,1,POINT_GET(cc,1) + ev.scale[k]*POINT_GET(c_z,1));
          POINT_SET(cc,2,POINT_GET(cc,2) + ev.scale[k]*POINT_GET(c_z,2));
          break;
        case GUI_EVENT_RIGHT:
          /* POINT_SET(cc,0,POINT_GET(cc,0)-ev.scale[k]); */
          POINT_SET(cc,0,POINT_GET(cc,0) + ev.scale[k]*POINT_GET(c_x,0));
          POINT_SET(cc,1,POINT_GET(cc,1) + ev.scale[k]*POINT_GET(c_x,1));
          POINT_SET(cc,2,POINT_GET(cc,2) + ev.scale[k]*POINT_GET(c_x,2));
          break;
        case GUI_EVENT_DOWN:
          /* POINT_SET(cc,1,POINT_GET(cc,1)-ev.scale[k]); */
          POINT_SET(cc,0,POINT_GET(cc,0) + ev.scale[k]*POINT_GET(c_y,0));
          POINT_SET(cc,1,POINT_GET(cc,1) + ev.scale[k]*POINT_GET(c_y,1));
          POINT_SET(cc,2,POINT_GET(cc,2) + ev.scale[k]*POINT_GET(c_y,2));
          break;
        case GUI_EVENT_FOCUS:
          f += ev.scale[k] / 20;
          printf("f = %g\n", f);
          break;
        case GUI_EVENT_ROLL:
          {
            double a = ev.scale[k] / -180.0;
            /*
            double c = cos(a), s = sin(a);
            tmatrix r = TMATRIX(c,-s,0,0, s,c,0,0, 0,0,1,0, 0,0,0,1);
            */
            tmatrix r = rot(c_z, a);
            c_x = direction(transform(r, c_x));
            c_y = direction(transform(r, c_y));
            c_z = direction(transform(r, c_z));
          }
          break;
        case GUI_EVENT_YAW:
          {
            double a = ev.scale[k] / -180.0;
            /*
            double c = cos(a), s = sin(a);
            tmatrix r = TMATRIX(c,0,s,0, 0,1,0,0, -s,0,c,0, 0,0,0,1);
            */
            tmatrix r = rot(c_y, a);
            c_x = direction(transform(r, c_x));
            c_y = direction(transform(r, c_y));
            c_z = direction(transform(r, c_z));
          }
          break;
        case GUI_EVENT_PITCH:
          {
            double a = ev.scale[k] / -180.0;
            /*
            double c = cos(a), s = sin(a);
            tmatrix r = TMATRIX(1,0,0,0, 0,c,-s,0, 0,s,c,0, 0,0,0,1);
            */
            tmatrix r = rot(c_x, a);
            c_x = direction(transform(r, c_x));
            c_y = direction(transform(r, c_y));
            c_z = direction(transform(r, c_z));
          }
          break;
        default: break;
      }
      k++;
    }
  if (ev.type == GUI_EVENT_QUIT) scheduler_stop();
  if (k) draw_scene();
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
