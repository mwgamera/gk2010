#include "model.h"
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Private structures */
struct _model {
  vertex *v;  /* array of vertices */
  face *s; /* array of faces */
  int nv, ns; /* lengths */
};

struct _scene {
  face *s; /* face laying on splitting plane */
  scene *p, *n; /* positive and negative half-scenes */
  scalar a, b, c, d; /* coefficients of splitting plane */
};


/* Free model structure */
void model_free(model *m) {
  int i;
  assert(m != NULL);
  for (i = 0; i < m->ns; i++)
    free(m->s[i].v);
  free(m->v);
  free(m->s);
  free(m);
}


/* Parser */
#define BUFFER_SIZE 1024
#define MAX_TOKEN_LENGTH 128 

struct file_buffer {
  FILE *fp;
  char buf[BUFFER_SIZE];
  int consumed, read;
};

static int buf_next(struct file_buffer *ff) {
  if (ff->read <= ff->consumed) {
    ff->read = fread(ff->buf, 1, BUFFER_SIZE, ff->fp);
    ff->consumed = 0;
    if (!ff->read)
      return -1;
  }
  return ff->buf[ff->consumed++];
}

static char *buf_token(struct file_buffer *ff) {
  /* (?:[:space:]*#[^\n\r]*)*([:graph:]{1,MAX_TOKEN_LENGTH}) */
  static char buf[MAX_TOKEN_LENGTH];
  int c, i, x = 0;
  for (;;) {
    c = buf_next(ff);
    switch (x) {
      case 0:
        if (c == '#') { x = 1; break; }
        if (c == EOF) {
          buf[0] = '\0';
          return buf;
        }
        if (!isspace(c)) {
          x = 2; i = 0;
          buf[i++] = c;
        }
        break;
      case 1:
        if (c == '\n' || c == '\r') { x = 0; break; }
        if (c == EOF) {
          buf[0] = '\0';
          return buf;
        }
        break;
      case 2:
        if (c == EOF || !isgraph(c) || i == MAX_TOKEN_LENGTH-1) {
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

#define READ_ERRRET(code) { \
  if (m) { \
    for (i = 0; i < m->ns; i++) \
      free(m->s[i].v); \
    free(m->v); \
    free(m->s); \
    free(m); \
  } \
  *error = (code); \
  return NULL; \
}
/* Error codes:
 *  0 success
 * -1 invalid format
 * -2 invalid version of format
 * -3 memory allocation failure
 * -4 unrecognized keyword
 * -5 premature end of file
 * -6 unsupported feature */
model *model_read_mesh(FILE *fp, int *error) {
  struct file_buffer buf = {NULL, "", 0, 0};
  model *m = NULL;
  char *s;
  void *ap;
  int i, dim = 3;
  assert(fp);
  buf.fp = fp;
  if (!error)
    error = &i;
  else
    *error = 0;
  /* magic number and version */
  if (strcmp(buf_token(&buf),"MeshVersionFormatted"))
    READ_ERRRET(-1);
  if (atoi(buf_token(&buf)) > 1) /* 0 or 1 */
    READ_ERRRET(-2);
  /* allocate */
  m = malloc(sizeof *m);
  if (!m) READ_ERRRET(-3);
  m->v = NULL;
  m->s = NULL;
  m->nv = m->ns = 0;
  /* read data */
  while ((s = buf_token(&buf)) != NULL)
    switch (s[0]) {
      case 0:
        if (!m || !m->nv || !m->ns || !m->v || !m->s)
          READ_ERRRET(-5);
        return m;
      case 'A':
        if (strcmp(s, "AngleOfCornerBound"))
          READ_ERRRET(-4);
        (void) buf_token(&buf); 
        break;
      case 'B':
        if (strcmp(s, "BoundingBox"))
          READ_ERRRET(-4);
        for (i = 0; i < dim; i++) {
          (void) buf_token(&buf); 
          (void) buf_token(&buf); 
        }
        break;
      case 'C':
        switch (s[1]) {
          case 'o':
            if (strcmp(s, "Corners"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--)
              (void) buf_token(&buf);
            break;
          case 'r':
            if (strcmp(s, "CrackedEdges"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--) {
              (void) buf_token(&buf);
              (void) buf_token(&buf);
            }
            break;
          default:
            READ_ERRRET(-4);
            break;
        }
        break;
      case 'D':
        if (strcmp(s, "Dimension"))
          READ_ERRRET(-4);
        dim = atoi(buf_token(&buf));
        if (dim != 2 && dim != 3)
          READ_ERRRET(-6);
        break;
      case 'E':
        switch (s[1]) {
          case 'd':
            if (strcmp(s, "Edges"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--) {
              (void) buf_token(&buf);
              (void) buf_token(&buf);
            }
            break;
          case 'n':
            if (strcmp(s, "End"))
              READ_ERRRET(-4);
            if (!m || !m->nv || !m->ns || !m->v || !m->s)
              READ_ERRRET(-5);
            return m;
          case 'q':
            if (strcmp(s, "EquivalenceEdges"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--) {
              (void) buf_token(&buf);
              (void) buf_token(&buf);
            }
            break;
          default:
            READ_ERRRET(-4);
            break;
        }
        break;
      case 'G':
        if (strcmp(s, "Geometry"))
          READ_ERRRET(-4);
        READ_ERRRET(-6);
        break;
      case 'I':
        if (strcmp(s, "IncludeFile"))
          READ_ERRRET(-4);
        READ_ERRRET(-6);
        break;
      case 'M':
        if (strcmp(s, "MeshSupportOfVertices"))
          READ_ERRRET(-4);
        READ_ERRRET(-6);
        break;
      case 'P':
        if (strcmp(s, "PhysicsReference"))
          READ_ERRRET(-4);
        READ_ERRRET(-6);
        break;
      case 'Q':
        if (strcmp(s, "Quadrilaterals"))
          READ_ERRRET(-4);
        i = atoi(buf_token(&buf));
        ap = realloc(m->s, (m->ns + i) * sizeof *m->s);
        if (!ap) READ_ERRRET(-3);
        m->s = ap;
        while (i--) {
          m->s[m->ns].n = 4;
          m->s[m->ns].v = malloc(m->s[m->ns].n * sizeof*m->s->v);
          m->s[m->ns].v[0] = m->v + atoi(buf_token(&buf)) - 1;
          m->s[m->ns].v[1] = m->v + atoi(buf_token(&buf)) - 1;
          m->s[m->ns].v[2] = m->v + atoi(buf_token(&buf)) - 1;
          m->s[m->ns].v[3] = m->v + atoi(buf_token(&buf)) - 1;
          (void) buf_token(&buf); /* ref no */
          m->ns++;
        }
        break;
      case 'R':
        for (i = 1; i < 8; i++)
          if (!s[i])
            READ_ERRRET(-4);
        switch (s[8]) {
          case 'E':
            if (strcmp(s, "RequiredEdges"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--)
              (void) buf_token(&buf);
            break;
          case 'V':
            if (strcmp(s, "RequiredVertices"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--)
              (void) buf_token(&buf);
            break;
          default:
            READ_ERRRET(-4);
            break;
        }
        break;
      case 'S':
        for (i = 1; i < 9; i++)
          if (!s[i])
            READ_ERRRET(-4);
        switch (s[9]) {
          case 0:
            if (strcmp(s, "SubDomain"))
              READ_ERRRET(-4);
            break;
          case 'F':
            for (i = 10; i < 13; i++)
              if (!s[i])
                READ_ERRRET(-4);
            switch (s[13]) {
              case 'G':
                if (strcmp(s, "SubDomainFromGeom"))
                  READ_ERRRET(-4);
                break;
              case 'M':
                if (strcmp(s, "SubDomainFromMesh"))
                  READ_ERRRET(-4);
                break;
              default:
                READ_ERRRET(-4);
                break;
            }
            break;
          default:
            READ_ERRRET(-4);
            break;
        }
        i = atoi(buf_token(&buf));
        while (i--) {
          (void) buf_token(&buf);
          (void) buf_token(&buf);
          (void) buf_token(&buf);
          (void) buf_token(&buf);
        }
        break;
      case 'T':
        switch (s[1]) {
          case 'a':
            if (strcmp(s, "TangentAtEdges"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            while (i--) {
              int j;
              (void) buf_token(&buf);
              (void) buf_token(&buf);
              for (j = 0; j < dim; j++)
                (void) buf_token(&buf);
            }
            break;
          case 'r':
            if (strcmp(s, "Triangles"))
              READ_ERRRET(-4);
            i = atoi(buf_token(&buf));
            ap = realloc(m->s, (m->ns + i) * sizeof *m->s);
            if (!ap) READ_ERRRET(-3);
            m->s = ap;
            while (i--) {
              m->s[m->ns].n = 3;
              m->s[m->ns].v = malloc(m->s[m->ns].n * sizeof*m->s->v);
              m->s[m->ns].v[0] = m->v + atoi(buf_token(&buf)) - 1;
              m->s[m->ns].v[1] = m->v + atoi(buf_token(&buf)) - 1;
              m->s[m->ns].v[2] = m->v + atoi(buf_token(&buf)) - 1;
              (void) buf_token(&buf); /* ref no */
              m->ns++;
            }
            break;
          default:
            READ_ERRRET(-4);
            break;
        }
        break;
      case 'V':
        if (strcmp(s, "Vertices"))
          READ_ERRRET(-4);
        i = atoi(buf_token(&buf));
        ap = realloc(m->v, (m->nv + i) * sizeof *m->v);
        if (!ap) READ_ERRRET(-3);
        m->v = ap;
        while (i--) {
          POINT_SET(m->v[m->nv].world, 0, atof(buf_token(&buf)));
          POINT_SET(m->v[m->nv].world, 1, atof(buf_token(&buf)));
          if (dim == 3)
            POINT_SET(m->v[m->nv].world, 2, atof(buf_token(&buf)));
          else
            POINT_SET(m->v[m->nv].world, 2, 0.f);
          POINT_SET(m->v[m->nv].world, 3, 1.f);
          (void) buf_token(&buf); /* ref no */
          m->nv++;
        }
        break;
      default:
        READ_ERRRET(-4);
        break;
    }
  if (!m || !m->nv || !m->ns || !m->v || !m->s)
    READ_ERRRET(-5);
  return m;
}

/* TODO: building scene from models */
