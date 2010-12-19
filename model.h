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

/* Apply transformation to a model */
void model_transform(tmatrix, model*);

/* Commit transformed model to a world */
void model_commit(model*);

/* Free scene data */
void scene_free(scene*);

/* Build a new scene from given number of models */
scene *scene_build(model**, int);

/* Traverse faces from the furthest to nearest
 * given reference point in world coordinates */
void scene_traverse(scene*, point, void (*)(face*));

#endif/*_MODEL_H_*/
