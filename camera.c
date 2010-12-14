#include "camera.h"
#include <math.h>

/* Focal length */
#define DEFAULT_FLENGTH 600.f
static float fo;

/* Tree of transformation matrices with validity flag */
#define TMM_LENGTH 7
static tmatrix tmm_matrix[TMM_LENGTH];
static unsigned long tmm_valid;
#define TMM_PARENT(i) ((((i)+1)>>1)-1)
#define TMM_LEFT(i) ((((i)+1)<<1)-1)
#define TMM_RIGHT(i) (((i)+1)<<1)
/* access */
#define TMM_MATRIX(i) (tmm_matrix[(i)])
#define TMM_ISVALID(i) (tmm_valid & (1<<(i)))
#define TMM_SETVALID(i) (tmm_valid |= (1<<(i)))
#define TMM_INVALIDATE(i) (tmm_valid &= ~(1<<(i)))
/* leaf indices */
#define TMM_LOCATION 6
#define TMM_ORIENTATION 5
#define TMM_PROJECTION 4
#define TMM_SCREEN 3

/* Invalidate parents of i-th matrix */
static void tmm_modified(int i) {
  i = TMM_PARENT(i); 
  while (i >= 0) {
    TMM_INVALIDATE(i);
    i = TMM_PARENT(i);
  }
}

/* Get i-th matrix, recalculate if required */
static tmatrix tmm_combine(int i) {
  /* assert leaves valid (should be invariant after camera_reset) */
  if (TMM_ISVALID(i))
    return TMM_MATRIX(i);
  TMM_MATRIX(i) = tcompose(tmm_combine(TMM_LEFT(i)), tmm_combine(TMM_RIGHT(i)));
  TMM_SETVALID(i);
  return TMM_MATRIX(i);
}

/* Retrieve camera matrix */
tmatrix camera_transform() {
  return tmm_combine(0);
}

/* Reset camera state to default (correct) values */
void camera_reset(int w, int h, float s) {
  tmatrix I = TMATRIX_ONE;
  /* screen geometry matrix */
  TMM_MATRIX(TMM_SCREEN) = I;
  TMATRIX_SET(TMM_MATRIX(TMM_SCREEN), 0, 3, w >> 1);
  TMATRIX_SET(TMM_MATRIX(TMM_SCREEN), 1, 3, h >> 1);
  TMATRIX_SET(TMM_MATRIX(TMM_SCREEN), 0, 0, s);
  TMATRIX_SET(TMM_MATRIX(TMM_SCREEN), 1, 1, s);
  TMM_SETVALID(TMM_SCREEN);
  tmm_modified(TMM_SCREEN);
  /* projection matrix */
  fo = DEFAULT_FLENGTH;
  TMM_MATRIX(TMM_PROJECTION) = I;
  camera_focus_set(fo);
  TMM_SETVALID(TMM_PROJECTION);
  tmm_modified(TMM_PROJECTION);
  /* orientation matrix (rotation) */
  TMM_MATRIX(TMM_ORIENTATION) = I;
  TMATRIX_SET(TMM_MATRIX(TMM_ORIENTATION), 1, 1, 0);
  TMATRIX_SET(TMM_MATRIX(TMM_ORIENTATION), 2, 2, 0);
  TMATRIX_SET(TMM_MATRIX(TMM_ORIENTATION), 1, 2, 1);
  TMATRIX_SET(TMM_MATRIX(TMM_ORIENTATION), 2, 1, 1);
  TMM_SETVALID(TMM_ORIENTATION);
  tmm_modified(TMM_ORIENTATION);
  /* location matrix, reset to camera at (0,0,0) */
  TMM_MATRIX(TMM_LOCATION) = I;
  TMM_SETVALID(TMM_LOCATION);
  tmm_modified(TMM_LOCATION);
}

/* Move along axis (intrinsic) by scale units */
void camera_move(int axis, float scale) {
  TMATRIX_SET(TMM_MATRIX(TMM_LOCATION),0,3,
      TMATRIX_GET(TMM_MATRIX(TMM_LOCATION),0,3) -
      TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,0) * scale);
  TMATRIX_SET(TMM_MATRIX(TMM_LOCATION),1,3,
      TMATRIX_GET(TMM_MATRIX(TMM_LOCATION),1,3) -
      TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,1) * scale);
  TMATRIX_SET(TMM_MATRIX(TMM_LOCATION),2,3,
      TMATRIX_GET(TMM_MATRIX(TMM_LOCATION),2,3) -
      TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,2) * scale);
  tmm_modified(TMM_LOCATION);
}

static tmatrix mkrtt(scalar,scalar,scalar,double);

/* Move around axis (intrinsic) by angle */
void camera_rotate(int axis, float angle) {
  /* (R*A')' = A*R' */
  TMM_MATRIX(TMM_ORIENTATION) = tcompose(TMM_MATRIX(TMM_ORIENTATION), mkrtt(
        TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,0),
        TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,1),
        TMATRIX_GET(TMM_MATRIX(TMM_ORIENTATION),axis,2),
        -angle));
  /* FIXME: it will loose it's orthogonality with time */
  tmm_modified(TMM_ORIENTATION);
}

/* Change focal length */
float camera_focus_set(float f) {
  float old = fo; fo = f > 0 ? f : fo;
  TMATRIX_SET(TMM_MATRIX(TMM_PROJECTION), 3, 2, 1.0 / fo);
  /* TMATRIX_SET(TMM_MATRIX(TMM_PROJECTION), 2, 3, -fo); */
  TMATRIX_SET(TMM_MATRIX(TMM_PROJECTION), 3, 3, 0.0);

  tmm_modified(TMM_PROJECTION);
  return old;
}

/* Add to focal length */
float camera_focus_add(float f) {
  return camera_focus_set(fo+f);
}

/* Transposed rotation around given axis (0,0,0)->(x,y,z) */
static tmatrix mkrtt(scalar x, scalar y, scalar z, double a) {
  /* TODO: This should be moved to space-dep since it can be optimised for SSE. */
  tmatrix r = TMATRIX_ONE;
  double s = sin(a);
  double c = cos(a);
  double ic = 1.f - c;
  double xy = x * y;
  double xz = x * z;
  double yz = y * z;
  TMATRIX_SET(r,0,0, c+x*x*ic);
  TMATRIX_SET(r,1,0, xy*ic-z*s);
  TMATRIX_SET(r,2,0, xz*ic+y*s);
  TMATRIX_SET(r,0,1, xy*ic+z*s);
  TMATRIX_SET(r,1,1, c+y*y*ic);
  TMATRIX_SET(r,2,1, yz*ic-x*s);
  TMATRIX_SET(r,0,2, xz*ic-y*s);
  TMATRIX_SET(r,1,2, yz*ic+x*s);
  TMATRIX_SET(r,2,2, c+z*z*ic);
  return r;
}
