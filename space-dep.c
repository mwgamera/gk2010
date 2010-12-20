#include "space.h"
#include "space-dep.h"
#include <math.h>

tmatrix tcompose(tmatrix a, tmatrix b) {
  tmatrix c = __TMATRIX(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  scalar r; int j, k;
  for (j = 0; j < 4; j++) {
    for (k = 0; k < 4; k++) {
      r = b.d[j][k];
      c.d[j][0] += r * a.d[k][0];
      c.d[j][1] += r * a.d[k][1];
      c.d[j][2] += r * a.d[k][2];
      c.d[j][3] += r * a.d[k][3];
    }
  }
  return c;
}

point transform(tmatrix a, point b) {
  point c = {{ 0, 0, 0, 0 }};
  int i, j;
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      c.d[j] += a.d[i][j] * b.d[i];
  return c;
}

point normalize(point p) {
  int i;
  for (i = 0; i < 4; i++)
    p.d[i] /= p.d[3];
  return p;
}

point direction(point v) {
  double r = 0; int j;
  for (j = 0; j < 3; j++)
    r += v.d[j] * v.d[j];
  r = sqrt(r);
  for (j = 0; j < 3; j++)
    v.d[j] /= r;
  v.d[3] = 1;
  return v;
}

point sdotmul(scalar a, point b) {
  int i;
  for (i = 0; i < 4; i++)
    b.d[i] *= a;
  return b;
}

scalar pdotmul(point a, point b) {
  int i;
  scalar s = 0.f;
  for (i = 0; i < 4; i++)
    s += a.d[i] * b.d[i];
  return s;
}

point pointplane(point a, point b, point c) {
  point plane;
  scalar ax = a.d[0], ay = a.d[1], az = a.d[2];
  scalar bx = b.d[0], by = b.d[1], bz = b.d[2];
  scalar cx = c.d[0], cy = c.d[1], cz = c.d[2];
  plane.d[0] = -ay*(cz-bz)+by*cz+az*(cy-by)-bz*cy;
  plane.d[1] = ax*(cz-bz)-bx*cz+bz*cx+az*(bx-cx);
  plane.d[2] = bx*cy+ax*(by-cy)-by*cx-ay*(bx-cx);
  plane.d[3] = -ax*(by*cz-bz*cy)+ay*(bx*cz-bz*cx)-az*(bx*cy-by*cx);
  return plane;
}

point planeintrs(point plane, point a, point b) {
  point v = {{ 0, 0, 0, 0 }};
  scalar u;
  int i;
  for (i = 0; i < 3; i++)
    v.d[i] = b.d[i]-a.d[i];
  u = pdotmul(plane, b) / pdotmul(plane, v);
  a.d[0] += u * v.d[0];
  a.d[1] += u * v.d[1];
  a.d[2] += u * v.d[2];
  return a;
}
