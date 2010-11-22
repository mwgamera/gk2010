/* Virtual objects to look at */
#ifndef _MODEL_H_
#define _MODEL_H_
#include "space.h"
#include <stdio.h>

/* Simple B-rep model */
typedef struct {
  /* vertices */
  point *vertex;
  point *pvertex;
  int nvertices;
  /* edges */
  int *edge;
  int nedges;
} model;

model *model_read(FILE*);
void model_free(model*);

void model_transform(tmatrix,model*);

#endif/*_MODEL_H_*/