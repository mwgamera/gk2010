#include "space.h"
#include "space-dep.h"

tmatrix tcompose(tmatrix a, tmatrix b) {
  __asm__ (
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

      "movaps %%xmm4, (%2)\n\t"

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

      "movaps %%xmm4, 16(%2)\n\t"
 
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

      "movaps %%xmm4, 32(%2)\n\t"
 
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

      "movaps %%xmm4, 48(%2)\n\t"

      : "=o"(b) : "r"(&a), "r"(&b), "o"(a), "o"(b)
      : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
        "%xmm4", "%xmm5", "%xmm6", "%xmm7");
  return b;
}

point transform(tmatrix a, point b) {
  __asm__ (
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

      "movaps %%xmm4, %0\n\t"

      : "=X"(b) : "r"(&a), "r"(&b), "o"(a), "o"(b)
      : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
        "%xmm4", "%xmm5", "%xmm6", "%xmm7");
  return b;
}

point normalize(point p) {
  __asm__ (
      "movaps     %1, %%xmm0\n\t"
      "movaps %%xmm0, %%xmm1\n\t"
      "shufps  $0xFF, %%xmm1, %%xmm1\n\t"
      "divps  %%xmm1, %%xmm0\n\t"
      "movaps %%xmm0, %0\n\t"
      : "=X"(p) : "X"(p) : "%xmm0");
  return p;
}

point direction(point v) {
  __asm__ (
      "movaps      %1, %%xmm0\n\t"
      "movaps      %1, %%xmm2\n\t"
      "mulps   %%xmm0, %%xmm0\n\t"
      "movhlps %%xmm0, %%xmm1\n\t"
      "haddps  %%xmm0, %%xmm0\n\t"
      "addss   %%xmm1, %%xmm0\n\t"
      "rsqrtss %%xmm0, %%xmm0\n\t"
      "shufps  $0, %%xmm0, %%xmm0\n\t"
      "mulps   %%xmm2, %%xmm0\n\t"
      "movaps  %%xmm0, %0\n\t"
      : "=X"(v) : "X"(v)
      : "%xmm0", "%xmm1", "%xmm2");
  v.d[3] = 1.f;
  return v;
}

point sdotmul(scalar b, point a) {
  __asm__ (
      "unpcklps %2, %2\n\t"
      "movlhps  %2, %2\n\t"
      "mulps    %1, %2\n\t"
      "movaps   %2, %0\n\t"
      : "=X"(a) : "X"(a), "x"(b));
  return a;
}

scalar pdotmul(point a, point b) {
  scalar c;
  __asm__ (
      "movaps %1, %0\n\t"
      "mulps  %2, %0\n\t"
      "haddps %0, %0\n\t"
      "haddps %0, %0\n\t"
      : "=x"(c) : "X"(a), "X"(b));
  return c;
}

static void mtr3(point *a, point *b, point *c) {
  /* transpose 3x3 matrix */
  __asm__ (
      "movaps (%3), %%xmm0\n\t"
      "movaps (%4), %%xmm1\n\t"
      "movaps (%5), %%xmm2\n\t"

      "movaps   %%xmm0, %%xmm3\n\t"
      "unpcklps %%xmm2, %%xmm0\n\t"
      "unpckhps %%xmm2, %%xmm3\n\t"

      "movaps   %%xmm1, %%xmm2\n\t"
      "unpcklps %%xmm1, %%xmm1\n\t"
      "unpckhps %%xmm2, %%xmm2\n\t"

      "unpcklps %%xmm2, %%xmm3\n\t"
      "movaps   %%xmm0, %%xmm2\n\t"
      "unpcklps %%xmm1, %%xmm0\n\t"
      "unpckhps %%xmm1, %%xmm2\n\t"

      "movaps %%xmm0, (%3)\n\t"
      "movaps %%xmm2, (%4)\n\t"
      "movaps %%xmm3, (%5)\n\t"

      : "=m"(*a), "=m"(*b), "=m"(*c)
      :  "r" (a),  "r" (b),  "r" (c),
         "m"(*a),  "m"(*b),  "m"(*c)
      : "%xmm0", "%xmm1", "%xmm2", "%xmm3");
}

static scalar det3(point a, point b, point c) {
  /* determinant of a 3x3 matrix given its three vectors */
  scalar d;
  __asm__ (
      "movaps     %2, %0\n\t"
      "movaps     %3, %%xmm2\n\t"
      "movaps     %0, %%xmm1\n\t"
      "movaps %%xmm2, %%xmm3\n\t"

      "shufps  $0x09, %0,     %0\n\t"
      "shufps  $0x12, %%xmm2, %%xmm2\n\t"
      "mulps  %%xmm2, %0\n\t"

      "shufps  $0x12, %%xmm1, %%xmm1\n\t"
      "shufps  $0x09, %%xmm3, %%xmm3\n\t"
      "mulps  %%xmm3, %%xmm1\n\t"

      "subps  %%xmm1, %0\n\t"
      "mulps      %1, %0\n\t"

      "movhlps    %0, %%xmm1\n\t"
      "haddps     %0, %0\n\t"
      "addss  %%xmm1, %0\n\t"

      : "=x"(d)
      : "X"(a), "X"(b), "X"(c)
      : "%xmm1", "%xmm2", "%xmm3");
  return d;
}

point pointplane(point a, point b, point c) {
  point plane, I = {{ 1, 1, 1 }};
  mtr3(&a, &b, &c);
  plane.d[0] = det3(I, b, c);
  plane.d[1] = det3(a, I, c);
  plane.d[2] = det3(a, b, I);
  plane.d[3] = det3(a, c, b);
  return plane;
}
