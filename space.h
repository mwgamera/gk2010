/* Linear transformations module: Public interface */
#ifndef _SPACE_H_
#define _SPACE_H_
#include "space-dep.h"

/* A point to be transformed, 3D vector */
typedef struct __point point;
/* Transformation matrix */
typedef struct __tmatrix tmatrix;

/* Component access macros, might be suboptimal. */
#define POINT_GET(p,i) __POINT_GET(p,i)
#define POINT_SET(p,i,v) __POINT_SET(p,i,v)
#define TMATRIX_GET(m,i,j) __TMATRIX_GET(m,i,j)
#define TMATRIX_SET(m,i,j,v) __TMATRIX_SET(m,i,j,v)

/* Static initialisation */
#define POINT(x,y,z) __POINT(x,y,z)
#define TMATRIX(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) \
  __TMATRIX(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)

#define POINT_ZERO    POINT(0,0,0)
#define TMATRIX_ONE   TMATRIX(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1)
#define TMATRIX_ZERO  TMATRIX(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)

/* Composition of transformations */
tmatrix tcompose(tmatrix, tmatrix);

/* Application of transformation */
point transform(tmatrix, point);

/* Versor of a vector */
point direction(point);

#endif/*_SPACE_H_*/
