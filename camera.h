/* Virtual camera */
#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "space.h"

/* Focal length */
extern float camera_flength;

/* Reset state. Must be caled to initialize. */
void camera_reset(int screen_width, int screen_height,
    float pixels_per_unit);

/* Movement */
void camera_move(int, float);
#define CAMERA_RIGHT(s) (camera_move(0,s))
#define CAMERA_DOWN(s) (camera_move(1,s))
#define CAMERA_FORWARD(s) (camera_move(2,s))

/* Rotation */
void camera_rotate(int, float);
#define CAMERA_PITCH(s) (camera_rotate(0,s))
#define CAMERA_YAW(s) (camera_rotate(1,s))
#define CAMERA_ROLL(s) (camera_rotate(2,s))

/* Get projection matrix */
tmatrix camera_projection(void);

#endif/*_CAMERA_H_*/
