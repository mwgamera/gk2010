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

/* Face */
struct _face {
  vertex *v[3]; /* pointers to vertices */
};

/* Free model data */
void model_free(model*);

/* Load model from a .mesh file */
model *model_read_mesh(FILE*, int*);

/* Transform points of a model */
void model_transform(tmatrix, model*);

/* Commit transformed model to a world */
void model_commit(model*);

/* Free scene data */
void scene_free(scene*);

/* Build a new scene from given number of models */
scene *scene_build(model**, int);

/* Transform points of a scene */
void scene_transform(tmatrix, scene*);

/* Traverse vertices in undefined order */
void scene_vertices(scene*, void(*)(vertex*));

/* Traverse faces from the furthest to nearest
 * given reference point in world coordinates */
void scene_traverse(scene*, point, void (*)(face*));

#endif/*_MODEL_H_*/
