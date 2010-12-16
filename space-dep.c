#include "space.h"
#include "space-dep.h"

static tmatrix tcompose_asm(tmatrix *a, tmatrix *b) {
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

      : "=r"(b) : "r"(a), "0"(b), "m"(*a), "m"(*b)
      : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
        "%xmm4", "%xmm5", "%xmm6", "%xmm7");
  return *b;
}

tmatrix tcompose(tmatrix a, tmatrix b) {
  return tcompose_asm(&a, &b);
}

static point transform_asm(tmatrix *a, point *b) {
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

      "movaps %%xmm4, (%0)\n\t"

      : "=r"(b) : "r"(a), "0"(b), "m"(*a), "m"(*b)
      : "%xmm0", "%xmm1", "%xmm2", "%xmm3",
        "%xmm4", "%xmm5", "%xmm6", "%xmm7");
  return *b;
}

point transform(tmatrix a, point b) {
  return transform_asm(&a, &b);
}

static point normalize_asm(point *p) {
  __asm__ (
      "movaps (%1), %%xmm0\n\t"
      "movaps %%xmm0, %%xmm1\n\t"
      "shufps $0xFF, %%xmm1, %%xmm1\n\t"
      "divps %%xmm1, %%xmm0\n\t"
      "movaps %%xmm0, (%0)\n\t"
      : "=r"(p) : "0"(p), "m"(*p)
      : "%xmm0", "%xmm1");
  return *p;
}

point normalize(point p) {
  return normalize_asm(&p);
}

static point direction_asm(point *v) {
  __asm__ (
      "movaps (%1), %%xmm0\n\t"
      "movaps (%1), %%xmm2\n\t"

      "mulps %%xmm0, %%xmm0\n\t"
      "movhlps %%xmm0, %%xmm1\n\t"
      "haddps %%xmm0, %%xmm0\n\t"
      "addss %%xmm1, %%xmm0\n\t"
      "rsqrtss %%xmm0, %%xmm0\n\t"
      "shufps $0x00, %%xmm0, %%xmm0\n\t"
      "mulps %%xmm0, %%xmm2\n\t"

      "movaps %%xmm2, (%0)\n\t"
      "mov %3, 12(%0)"
      : "=r"(v) : "0"(v), "m"(*v), "r"(1.0f)
      : "%eax", "%xmm0", "%xmm1", "%xmm2");
  return *v;
}

point direction(point v) {
  return direction_asm(&v);
}

static point sdotmul_asm(point *a, scalar *b) {
  __asm__ (
      "movss (%2), %%xmm0\n\t"
      "unpcklps %%xmm0, %%xmm0\n\t"
      "movlhps %%xmm0, %%xmm0\n\t"
      "mulps (%1), %%xmm0\n\t"
      "movaps %%xmm0, (%0)\n\t"
      : "=r"(a) : "0"(a), "r"(b), "m"(*a), "m"(*b)
      : "%xmm0");
  return *a;
}

point sdotmul(scalar b, point a) {
  return sdotmul_asm(&a, &b);
}

static point pdotmul_asm(point *a, point *b) {
  __asm__ (
      "movaps (%1), %%xmm0\n\t"
      "mulps (%2), %%xmm0\n\t"
      "movaps %%xmm0, (%0)\n\t"
      : "=r"(a) : "0"(a), "r"(b), "m"(*a), "m"(*b)
      : "%xmm0");
  return *a;
}

point pdotmul(point a, point b) {
  return pdotmul_asm(&a, &b);
}

