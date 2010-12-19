/* Virtual objects to look at */
#ifndef _MODEL_H_
#define _MODEL_H_
#include "space.h"
#include <stdio.h>

/* Types */
typedef struct _scene scene;
typedef struct _model model;
typedef struct _face face;
typedef struct _vertex vertex;

/* Publicly accessible structures */

/* Vertex */
struct _vertex {
  point world; /* point in world coordinates */
  point camera; /* point in transformed coordinates */
};

/* Convex polygon determined by ordered set of its vertices */
struct _face {
  vertex *v[3]; /* pointers to vertices */
};

/* Free model data */
void model_free(model*);

/* Load model from a .mesh file */
model *model_read_mesh(FILE*, int*);

/* TODO */

#endif/*_MODEL_H_*/
