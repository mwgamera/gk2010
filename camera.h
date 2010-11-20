/* Camera transform */
#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "space.h"

/* Focal length, everything with z < camera_flength should be invisible */
extern float camera_flength;

/* Reset state. Must be caled to initialize. */
void camera_reset(int screen_width, int screen_height,
    float pixels_per_unit);

/* Get projection matrix */
tmatrix camera_projection(void);

#endif/*_CAMERA_H_*/
