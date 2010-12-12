#include "model.h"
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#define TOKEN_LENGTH 128
#define BUFFER_SIZE 1024

/* Free model structure */ 
void model_free(model *m) {
  assert(m != NULL);
  free(m->halfedges);
  free(m->surfaces);
  free(m->vertices);
  free(m);
}

/* Parser */

struct file_buffer {
  FILE *fp;
  char buf[BUFFER_SIZE];
  int consumed, filled;
};

/* Get next character */
static int fbnext(struct file_buffer *ff) {
  if (ff->filled <= ff->consumed) {
    ff->filled = fread(ff->buf, 1, BUFFER_SIZE, ff->fp);
    ff->consumed = 0;
    if (!ff->filled)
      return -1;
  }
  return ff->buf[ff->consumed++];
}

/* Get next token or NULL after EOF */
static char *token(struct file_buffer *ff) {
  /* (?:[:space:]*#[^\n\r]*)*([:graph:]{1,TOKEN_LENGTH}) */
  static char buf[TOKEN_LENGTH];
  int c, i, x = 0;
  for (;;) {
    c = fbnext(ff);
    switch (x) {
      case 0:
        if (c == '#') { x = 1; break; }
        if (c == EOF) return NULL;
        if (!isspace(c)) {
          x = 2; i = 0;
          buf[i++] = c;
        }
        break;
      case 1:
        if (c == '\n' || c == '\r') { x = 0; break; }
        if (c == EOF) return NULL;
        break;
      case 2:
        if (c == EOF || !isgraph(c) || i == TOKEN_LENGTH-1) {
          buf[i++] = 0;
          return buf;
        }
        buf[i++] = c;
        break;
    }
  }
  assert(0);
  return NULL;
}

/* Read model from file */
model *model_read(FILE *f) {
  struct file_buffer buf = {NULL, "", 0, 0};
  tmatrix world = TMATRIX_ONE;
  model *m;
  char *s;
  int i, j;
  assert(f != NULL);
  buf.fp = f;
  if ((m = malloc(sizeof*m)) == NULL)
    return NULL;
  m->nvertices = 0;
  m->nsurfaces = 0;
  m->halfedges = NULL;
  while ((s = token(&buf)) != NULL) {
    switch (s[0]) {
      case 'v': /* vertices */
        m->nvertices = atoi(s+1);
        assert(m->nvertices > 0);
        m->vertices = malloc(m->nvertices * sizeof *m->vertices);
        for (i = 0; i < m->nvertices; i++) {
          POINT_SET(m->vertices[i].world, 0, atof(token(&buf)));
          POINT_SET(m->vertices[i].world, 1, atof(token(&buf)));
          POINT_SET(m->vertices[i].world, 2, atof(token(&buf)));
          POINT_SET(m->vertices[i].world, 3, 1.f);
        }
        break;
      case 'e': /* edges -> half-edges */
        assert(m->nvertices > 0);
        j = atoi(s+1); j <<= 1;
        assert(j > 0);
        m->halfedges = malloc(j * sizeof *m->halfedges);
        for (i = 0; i < j; i += 2) {
          m->halfedges[i+0].next = NULL;
          m->halfedges[i+1].next = NULL;
          m->halfedges[i+0].pair = m->halfedges + i+1;
          m->halfedges[i+1].pair = m->halfedges + i+0;
          m->halfedges[i+0].vertex = m->vertices + atoi(token(&buf));
          m->halfedges[i+1].vertex = m->vertices + atoi(token(&buf));
          m->halfedges[i+0].vertex->edge = m->halfedges + i+1;
          m->halfedges[i+1].vertex->edge = m->halfedges + i+0;
        }
        break;
      case 's': /* surfaces */
        assert(m->halfedges != NULL);
        m->nsurfaces = atoi(s+1);
        assert(m->nsurfaces > 0);
        m->surfaces = malloc(m->nsurfaces * sizeof *m->surfaces);
        for (i = 0; i < m->nsurfaces; i++) {
          halfedge *a, *b;
          int k;
          j = atoi(token(&buf)); /* number of edges surrounding surface */
          assert(j >= 3);
          a = m->halfedges + (atoi(token(&buf)) << 1); --j;
          b = m->halfedges + (atoi(token(&buf)) << 1); --j;
          for (k = 0; k < 4; k++) {
            int g = (k&1), h = (k>>1);
            if (a[g].vertex == b[h].pair->vertex) {
              a[g].next = b+h;
              m->surfaces[i].edge = a+g;
              break;
            }
          }
          assert(m->surfaces[i].edge != NULL);
          a = m->surfaces[i].edge->next;
          while (j--) {
            b = m->halfedges + (atoi(token(&buf)) << 1);
            for (k = 0; k < 2; k++) {
              if (b[k].pair->vertex == a->vertex) {
                a->next = b+k;
                a = a->next;
                break;
              }
            }
          }
          assert(a->vertex == m->surfaces[i].edge->pair->vertex);
          a->next = m->surfaces[i].edge;
          a = m->surfaces[i].edge;
        }
        break;
      case 'm': /* matrix */
        for (i = 0; i < 4; i++)
          for (j = 0; j < 4; j++)
            TMATRIX_SET(world, i, j, atof(token(&buf)));
        break;
      default: /* syntax error */
        if (m->nsurfaces) free(m->surfaces);
        if (m->nvertices) free(m->vertices);
        if (m->halfedges != NULL)
          free(m->halfedges);
        free(m);
        assert(0);
        return NULL;
    }
  }
  for (i = 0; i < m->nvertices; i++)
    m->vertices[i].world = normalize(transform(world, m->vertices[i].world));
  return m;
}

void model_transform(tmatrix a, model *m) {
  int i = m->nvertices;
  vertex *vs = m->vertices;
  point p;
  assert(i > 0);
  assert(vs != NULL);
  while (i--) {
    p = transform(a, vs->world);
    if (POINT_GET(p, 2) > 0)
      p = normalize(p);
    vs->camera = p;
    vs++;
  }
}
