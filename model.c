#include "model.h"
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define TOKEN_LENGTH 128

/* Free model */
void model_free(model *m) {
  free(m->edge);
  free(m->vertex);
  free(m->pvertex);
  free(m);
}

/* Read next token from file, not supposed to be fast */
static char *token(FILE *f) {
  static char buf[TOKEN_LENGTH];
  int c, i = 0;
  do c = getc(f); while (isspace(c));
  if (c == '#') {
    do c = getc(f);
    while (c != '\n' && c != EOF);
    c = getc(f);
  }
  while (!isspace(c) && c != EOF && i < TOKEN_LENGTH-1) {
    buf[i++] = c;
    c = getc(f);
  }
  buf[i] = '\0';
  return buf;
}

/* Read model from file */
model *model_read(FILE *f) {
  tmatrix world = TMATRIX_ONE;
  model *m = malloc(sizeof*m);
  char *s;
  int i, j;
  if (m == NULL) return NULL;
  m->nvertices = 0;
  m->nedges = 0;
  while ((s = token(f))[0] != '\0')
    switch(s[0]) {
      case 'v': /* vertices */
        m->nvertices = atoi(s+1);
        m->vertex  = malloc(m->nvertices * sizeof*m->vertex);
        m->pvertex = malloc(m->nvertices * sizeof*m->vertex);
        for (i = 0; i < m->nvertices; i++) {
          POINT_SET(m->vertex[i], 0, atoi(token(f)));
          POINT_SET(m->vertex[i], 1, atoi(token(f)));
          POINT_SET(m->vertex[i], 2, atoi(token(f)));
          POINT_SET(m->vertex[i], 3, 1.f);
        }
        break;
      case 'e': /* edges */
        m->nedges = atoi(s+1);
        m->edge = malloc(2 * m->nedges * sizeof*m->edge);
        for (i = 0; i < (m->nedges << 1); i++)
          m->edge[i] = atoi(token(f));
        break;
      case 's': /* surfaces -- eat and ignore (for now) */
        i = atoi(s+1);
        while (i--) {
          j = atoi(token(f));
          while (j--) token(f);
        }
        break;
      case 'm': /* matrix */
        for (i = 0; i < 4; i++)
          for (j = 0; j < 4; j++)
            TMATRIX_SET(world, i, j, atof(token(f)));
        break;
      default: /* syntax error */
        if (m->nedges) free(m->edge);
        if (m->nvertices) {
          free(m->vertex);
          free(m->pvertex);
        }
        free(m);
        return NULL;
    }
  for (i = 0; i < m->nvertices; i++)
    m->vertex[i] = normalize(transform(world, m->vertex[i]));
  return m;
}

void model_transform(tmatrix a, model *m) {
  int i;
  for (i = 0; i < m->nvertices; i++)
    m->pvertex[i] = transform(a, m->vertex[i]);
  for (i = 0; i < m->nvertices; i++) {
    if (POINT_GET(m->pvertex[i], 2) >= 0)
        m->pvertex[i] = normalize(m->pvertex[i]);
  }
}
