#include "space-dep.h"

static tmatrix tcompose_asm(tmatrix *a, tmatrix *b, tmatrix *c) {
  __asm__ __volatile__ (
      /* left matrix */
      "movaps   (%1), %%xmm0\n\t"
      "movaps 16(%1), %%xmm1\n\t"
      "movaps 32(%1), %%xmm2\n\t"
      "movaps 48(%1), %%xmm3\n\t"

      /* first column */
      "movlps  (%2), %%xmm4\n\t"
      "movlps 8(%2), %%xmm6\n\t"
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

      /* second column */
      "movlps 16(%2), %%xmm4\n\t"
      "movlps 24(%2), %%xmm6\n\t"
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

      "movaps %%xmm4, 16(%0)\n\t"
 
      /* third column */
      "movlps 32(%2), %%xmm4\n\t"
      "movlps 40(%2), %%xmm6\n\t"
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

      "movaps %%xmm4, 32(%0)\n\t"
 
      /* fourth column */
      "movlps 48(%2), %%xmm4\n\t"
      "movlps 56(%2), %%xmm6\n\t"
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

      "movaps %%xmm4, 48(%0)\n\t"

      : "=r"(c) : "r"(a), "r"(b)
      : "%xmm0", "%xmm1", "%xmm2",
      "%xmm3", "%xmm4", "%xmm5",
      "%xmm6", "%xmm7", "memory");
  return *c;
}

tmatrix tcompose(tmatrix a, tmatrix b) {
  tmatrix c = TMATRIX_ZERO;
  return tcompose_asm(&a, &b, &c);
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
