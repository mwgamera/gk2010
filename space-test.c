#include "space.h"
#include <stdio.h>

void printpoint(point x) {
  printf("[%g %g %g] %g\n",
      POINT_GET(x,0),
      POINT_GET(x,1),
      POINT_GET(x,2),
      POINT_GET(x,3) /* FIXME */
      );
}
void printmatrix(tmatrix m) {
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++)
      printf("%8.4f ", TMATRIX_GET(m, i, j));
    printf("\n");
  }
}

int main() {
  tmatrix a = TMATRIX(1,2,4,5, 8,5,3,2, 3,7,3,1, 1,1,1,1);
  tmatrix b = TMATRIX(
      1,0,0,0, 0,0.540302305868139717, -.84147098480789650665, 0,
      0,.8414709848078965066, 0.540302305868139717,0, 0,0,0,1); 
  point q = POINT(3,4,7);
  int i;
  /*
  printpoint(q); printf("---\n");
  printmatrix(a); printf("---\n");
  printmatrix(b); printf("---\n");
  printmatrix(tcompose(a, b)); printf("---\n");
  printpoint(transform(a, q)); printf("===\n");
  printmatrix(a); printf("---\n");
  printpoint(q); printf("---\n");
  */
  /* stress test */
  for (i=0; i<0xFFFFF; i++) {
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
    a = tcompose(b,a);
  }
  printmatrix(a);
  return 0;
}
