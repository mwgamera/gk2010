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
