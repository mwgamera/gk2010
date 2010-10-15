/* Implementation specific details of linear transformations' unit
 * that should be never relied upon outside this module.  */
#ifndef _SPACE_DEP_H_
#define _SPACE_DEP_H_
#include "space.h"

typedef float scalar;
struct __point { scalar d[4]; };
struct __tmatrix { scalar d[4][4]; };

#define __POINT_GET(p,i)   ((p).d[i]) 
#define __POINT_SET(p,i,v) ((p).d[i] = (v))
#define __TMATRIX_GET(m,i,j)   ((m).d[j][i]) 
#define __TMATRIX_SET(m,i,j,v) ((m).d[j][i] = (v))

#define __POINT(x,y,z) {{ x, y, z, 1 }}
#define __TMATRIX(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) {{ \
    { a, b, c, d }, \
    { e, f, g, h }, \
    { i, j, k, l }, \
    { m, n, o, p }  \
}}

#endif/*_SPACE_DEP_H_*/
