/* Virtual objects to look at */
#ifndef _MODEL_H_
#define _MODEL_H_
#include "space.h"
#include <stdio.h>

/* Types */
typedef struct _model model;
typedef struct _surface surface;
typedef struct _halfedge halfedge;
typedef struct _vertex vertex;

struct _model {
  vertex *vertices;
  surface *surfaces;
  halfedge *halfedges;
  int nvertices;
  int nsurfaces;
};

struct _surface {
  halfedge *edge;
};

struct _vertex {
  point world;
  point camera;
  halfedge *edge;
};

struct _halfedge {
  vertex *vertex;
  halfedge *pair;
  halfedge *next;
};

model *model_read(FILE*);
void model_free(model*);

void model_transform(tmatrix, model*);

#endif/*_MODEL_H_*/
