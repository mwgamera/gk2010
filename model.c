#include "model.h"
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Model structure */
struct _model {
  vertex *v;  /* array of vertices */
  face *s;    /* array of faces */
  int nv, ns; /* lengths */
};


/* Free model structure */
void model_free(model *m) {
  assert(m != NULL);
  free(m->v);
  free(m->s);
  free(m);
}


/* Two triangular faces from vertices of convex planar quadrilateral */
static void split_quadrilateral(
    face *f1, face *f2,
    vertex *a, vertex *b, vertex *c, vertex *d) {
  float d1, d2, q;
  q = POINT_GET(a->world,0) - POINT_GET(c->world,0); d1  = q*q;
  q = POINT_GET(a->world,1) - POINT_GET(c->world,1); d1 += q*q;
  q = POINT_GET(a->world,2) - POINT_GET(c->world,2); d1 += q*q;
  q = POINT_GET(b->world,0) - POINT_GET(d->world,0); d2  = q*q;
  q = POINT_GET(b->world,1) - POINT_GET(d->world,1); d2 += q*q;
  q = POINT_GET(b->world,2) - POINT_GET(d->world,2); d2 += q*q;
  if (d1 < d2) {
    f1->v[0] = a; f1->v[1] = b; f1->v[2] = c;
    f2->v[0] = c; f2->v[1] = d; f2->v[2] = a;
  }
  else {
    f1->v[0] = d; f1->v[1] = a; f1->v[2] = b;
    f2->v[0] = b; f2->v[1] = c; f2->v[2] = d;
  }
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
/* Internal error codes:
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
        ap = realloc(m->s, 2 * (m->ns + i) * sizeof *m->s);
        if (!ap) READ_ERRRET(-3);
        m->s = ap;
        while (i--) {
          vertex *a, *b, *c, *d;
          a = m->v + atoi(buf_token(&buf)) - 1;
          b = m->v + atoi(buf_token(&buf)) - 1;
          c = m->v + atoi(buf_token(&buf)) - 1;
          d = m->v + atoi(buf_token(&buf)) - 1;
          split_quadrilateral(m->s+m->ns, m->s+m->ns+1, a, b, c, d);
          (void) buf_token(&buf); /* ref no */
          m->ns += 2;
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


/* Transform model */
void model_transform(tmatrix a, model *m) {
  int i = m->nv;
  while (i--)
    m->v[i].camera = transform(a, m->v[i].world);
}

/* Permanently apply transformation of model */
void model_commit(model *m) {
  int i = m->nv;
  while (i--)
    m->v[i].world = normalize(m->v[i].camera);
}



/* Scene structure */
typedef struct _scene_node scene_node;
struct _scene_node {
  scene_node *p, *n; /* positive and negative half-scenes */
  face *s; /* face laying on the splitting plane */
  point d; /* coefficients of splitting plane */
};

struct _scene {
  vertex *vx; /* array of vertices */
  scene_node *root;  /* root of the scene */
  int nvx; /* lengths */
};


/* Free scene structure */
static void scene_free_r(scene_node *s) {
  if (s == NULL) return;
  scene_free_r(s->p);
  scene_free_r(s->n);
  free(s);
}
void scene_free(scene *s) {
  scene_free_r(s->root);
  free(s->vx);
  free(s);
}


/* Space partitioning */

/* Copy faces with vertices out of a number of models */
static int scene_build_copy(
    vertex **vx, int *nvx, face **fx, int *nfx, /* out */
    model **models, int nmodels) {
  int i, va, fa;
  *nvx = *nfx = 0;
  for (i = 0; i < nmodels; i++) {
    *nvx += models[i]->nv;
    *nfx += models[i]->ns;
  }
  *vx = malloc(*nvx * sizeof**vx);
  if (!*vx) return -1;
  *fx = malloc(*nfx * sizeof**fx);
  if (!*fx) {
    free(*vx);
    return -1;
  }
  va = fa = 0;
  for (i = 0; i < nmodels; i++) {
    int k;
    for (k = 0; k < models[i]->nv; k++)
      (*vx)[va+k].world = models[i]->v[k].world;
    for (k = 0; k < models[i]->ns; k++) {
      (*fx)[fa+k].v[0] = models[i]->s[k].v[0] - models[i]->v + *vx + va;
      (*fx)[fa+k].v[1] = models[i]->s[k].v[1] - models[i]->v + *vx + va;
      (*fx)[fa+k].v[2] = models[i]->s[k].v[2] - models[i]->v + *vx + va;
    }
    fa += models[i]->ns;
    va += models[i]->nv;
  }
  return 0;
}
/* Partition sub-scene */
static scene_node *scene_build_node() {
  /* TODO */
  return NULL;
}
scene *scene_build(model **models, int nmodels) {
  /* TODO */
  return NULL;
}


/* Transform scene */
void scene_transform(tmatrix a, scene *s) {
  int i = s->nvx;
  while (i--)
    s->vx[i].camera = transform(a, s->vx[i].world);
}


/* Traverse vertices */
void scene_vertices(scene *s, void(*fun)(vertex*)) {
  int i = s->nvx;
  while (i--)
    (*fun)(&(s->vx[i]));
}


/* Traverse BSP-tree in-order */
static void scene_traverse_r(scene_node *s,
    point p, void (*fun)(face*)) {
  if (s->p == s->n) {
    assert(s->p == NULL);
    (*fun)(s->s);
  }
  else {
    scalar d = pdotmul(s->d, p);
    if (d < 0.f) {
      if (s->p != NULL)
        scene_traverse_r(s->p, p, fun);
      (*fun)(s->s);
      if (s->n != NULL)
        scene_traverse_r(s->n, p, fun);
    }
    else {
      if (s->n != NULL)
        scene_traverse_r(s->n, p, fun);
      (*fun)(s->s);
      if (s->p != NULL)
        scene_traverse_r(s->p, p, fun);
    }
  }
}
void scene_traverse(scene *s,
    point p, void (*fun)(face*)) {
  scene_traverse_r(s->root, p, fun);
}

