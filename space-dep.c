#include "space-dep.h"

tmatrix tcompose(tmatrix a, tmatrix b) {
  tmatrix c = TMATRIX_ZERO;
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
  point c = POINT_ZERO;
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      c.d[j] += a.d[i][j] * b.d[i];
    }
  }
  return c;
}
