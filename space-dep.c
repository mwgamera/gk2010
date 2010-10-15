#include "space-dep.h"

tmatrix tcompose(tmatrix a, tmatrix b) {
  tmatrix c = TMATRIX_ZERO;
  /* FIXME */
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

static point transform_asm(tmatrix *a, point *b, point *c) {
  __asm__ __volatile__(
      "movaps   (%1), %%xmm0\n\t"
      "movaps 16(%1), %%xmm1\n\t"
      "movaps 32(%1), %%xmm2\n\t"
      "movaps 48(%1), %%xmm3\n\t"

      "movlps   (%2), %%xmm4\n\t"
      "movlps  8(%2), %%xmm6\n\t"
      "unpcklps %%xmm4, %%xmm4\n\t"
      "unpcklps %%xmm6, %%xmm6\n\t"
      "movhlps %%xmm4, %%xmm5\n\t"
      "movhlps %%xmm6, %%xmm7\n\t"

      "movlhps %%xmm4, %%xmm4\n\t"
      "mulps   %%xmm0, %%xmm4\n\t"

      "movlhps %%xmm5, %%xmm5\n\t"
      "mulps   %%xmm1, %%xmm5\n\t"

      "movlhps %%xmm6, %%xmm6\n\t"
      "mulps   %%xmm2, %%xmm6\n\t"

      "addps   %%xmm5, %%xmm4\n\t"

      "movlhps %%xmm7, %%xmm7\n\t"
      "mulps   %%xmm3, %%xmm7\n\t"

      "addps   %%xmm7, %%xmm6\n\t"
      "addps   %%xmm6, %%xmm4\n\t"

      "movaps %%xmm4, (%0)\n\t"
      : "=r"(c) : "r"(a), "r"(b)
      : "%xmm0", "%xmm1", "%xmm2",
      "%xmm3", "%xmm4", "%xmm5",
      "%xmm6", "%xmm7", "memory");
  return *c;
}

point transform(tmatrix a, point b) {
  point c = POINT_ZERO;
  return transform_asm(&a, &b, &c);
}
