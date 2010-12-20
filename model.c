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
  int c, i = 0, x = 0;
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
  face s; /* face laying on the splitting plane */
  point d; /* coefficients of splitting plane */
};

struct _scene {
  scene_node *root;  /* root of the scene */
  vertex *vx; /* array of original vertices */
  vertex **vsx; /* arrays of vertices from splits */
  int nvx, nvsx; /* lengths */
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
  while (s->nvsx--)
    free(s->vsx[s->nvsx]);
  free(s->vsx);
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
/* Fast PRNG, Marsaglia 2003 */
static unsigned long xrand() {
  static unsigned long y = 2463534242;
  y ^= y << 13; y ^= y >> 17; y ^= y << 5;
  return y;
}
/* Qualify face */
static int face_infront(point plane, face *fx) {
  scalar d[3];
  int i;
  for (i = 0; i < 3; i++)
    d[i] = pdotmul(plane, fx->v[i]->world);
  if (d[0] >= 0.f && d[1] >= 0.f && d[2] >= 0.f)
    return 1; /* in front */
  if (d[0] <= 0.f && d[1] <= 0.f && d[2] <= 0.f)
    return -1; /* behind */
  return 0; /* intersected */
}
/* Swap two elements of face array */
static void faceswap(face *fx, int i, int j) {
  face ff = fx[i];
  fx[i] = fx[j];
  fx[j] = ff;
}
/* Choose splitting face */
#define SPLIT_SAMPLE 20 /* number of candidates to split planes */
#define SPLIT_DEPTH 2 /* magic tunable performance parameter */
static void splitplane(scene_node *n, face *faces, int length) {
  int i, j, slen, sidx[SPLIT_SAMPLE];
  int bidx = 0, best = -1;
  /* draw candidates */
  if (length < SPLIT_SAMPLE)
    for (slen = 0; slen < length; slen++)
      sidx[slen] = slen;
  else
    for (slen = 0; slen < SPLIT_SAMPLE; slen++)
      sidx[slen] = xrand() % length;
  /* qualify candidates */
  for (i = 0; i < slen; i++) {
    int dd, d[3] = { 0, 0, 0 };
    point plane = pointplane(
        faces[sidx[i]].v[0]->world,
        faces[sidx[i]].v[1]->world,
        faces[sidx[i]].v[2]->world);
    for (j = 0; j < sidx[i]; j++)
      d[1+face_infront(plane, faces + j)]++;
    for (j = sidx[i]+1; j < length; j++)
      d[1+face_infront(plane, faces + j)]++;
    dd = (d[0]>d[2] ? d[0]-d[2] : d[2]-d[0]) + SPLIT_DEPTH*d[1];
    if (dd < best || best < 0) {
      best = dd;
      bidx = sidx[i];
      n->s = faces[sidx[i]];
      n->d = plane;
    }
  }
  /* move the chosen one to zero */
  if (bidx) faceswap(faces, 0, bidx);
}
/* Split face into sides of a plane */
static int facesplit(
    face *pos, face *neg, int *pi, int *ni, /* out */
    point plane, face fx, scene *sx) {
  vertex *front[4], *back[4];
  vertex **vsx, *vvsx;
  point a, b;
  scalar sa, sb;
  int i, vc = 0, fc = 0, bc = 0;

  /* prepare space */
  vvsx = malloc(2 * sizeof*vvsx);
  if (!vvsx) return -1;
  vsx = realloc(sx->vsx, (++sx->nvsx) * sizeof*sx->vsx);
  if (!vsx) {
    free(vvsx);
    return -2;
  }
  sx->vsx = vsx;
  sx->vsx[sx->nvsx-1] = vvsx;

  a = fx.v[2]->world;
  sa = pdotmul(plane, a);

  /* determine vertices of side polygons */
  for (i = 0; i < 3; i++) {
    b = fx.v[i]->world;
    sb = pdotmul(plane, b);
    if (sb < 0.f) {
      if (sa > 0.f) {
        vvsx[vc++].world = planeintrs(plane, a, b);
        back[bc++] = front[fc++] = vvsx + vc - 1;
      }
      back[bc++] = fx.v[i];
    }
    else
    if (sb > 0.f) {
      if (sa < 0.f) {
        vvsx[vc++].world = planeintrs(plane, a, b);
        front[fc++] = back[bc++] = vvsx + vc - 1;
      }
      front[fc++] = fx.v[i];
    }
    else
      front[fc++] = back[bc++] = fx.v[i];
    a = b;
    sa = sb;
  }

  /* return front */
  if (fc == 4) {
    split_quadrilateral(pos+*pi, pos+*pi+1,
        front[0], front[1], front[2], front[3]);
    *pi += 2;
  }
  else {
    assert(fc == 3);
    for (i = 0; i < 3; i++)
      pos[*pi].v[i] = front[i];
    *pi += 1;
  }

  /* return back */
  if (bc == 4) {
    split_quadrilateral(neg+*ni, neg+*ni+1,
        back[0], back[1], back[2], back[3]);
    *ni += 2;
  }
  else {
    assert(bc == 3);
    for (i = 0; i < 3; i++)
      neg[*ni].v[i] = back[i];
    *ni += 1;
  }
  return 0;
}
/* Partition sub-scene */
static scene_node *scene_build_node(face *faces, int length, scene *sx) {
  scene_node *s;
  face *pos, *neg;
  int i, d[3] = { 0, 0, 0 };
  int pi, ni;
  if (length < 1) {
    free(faces);
    return NULL;
  }
  s = malloc(sizeof*s);
  if (!s) return NULL;
  s->p = s->n = NULL;
  if (length < 2) {
    s->s = *faces;
    free(faces);
    return s;
  }
  splitplane(s, faces, length);
  for (i = 1; i < length; i++)
    d[1+face_infront(s->d, faces + i)]++;
  pos = malloc((d[2]+2*d[1]) * sizeof*pos);
  neg = malloc((d[0]+2*d[1]) * sizeof*neg);
  if (!pos || !neg) {
    free(pos);
    free(neg);
    free(s);
    free(faces);
    return NULL;
  }
  pi = 0; ni = 0;
  for (i = 1; i < length; i++) {
    switch (face_infront(s->d, faces + i)) {
      case -1:
        neg[ni++] = faces[i];
        break;
      case +1:
        pos[pi++] = faces[i];
        break;
      case 0:
        if (facesplit(pos, neg, &pi, &ni, s->d, faces[i], sx)) {
          free(pos);
          free(neg);
          free(s);
          free(faces);
          return NULL;
        }
        break;
    }
  }
  for (i = 0; i < ni; i++) {
    assert(neg[i].v);
  }
  for (i = 0; i < pi; i++) {
    assert(pos[i].v);
  }
  free(faces);
  s->n = scene_build_node(neg, ni, sx);
  s->p = scene_build_node(pos, pi, sx);
  return s;
}
scene *scene_build(model **models, int nmodels) {
  face *fxs = NULL;
  int nfx;
  scene *s = malloc(sizeof*s);
  if (!s) return NULL;
  scene_build_copy(&s->vx, &s->nvx, &fxs, &nfx, models, nmodels);
  s->nvsx = 0;
  s->vsx = NULL;
  s->root = scene_build_node(fxs, nfx, s);
  return s;
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
    (*fun)(&s->s);
  }
  else {
    scalar d = pdotmul(s->d, p);
    if (d < 0.f) {
      if (s->p != NULL)
        scene_traverse_r(s->p, p, fun);
      (*fun)(&s->s);
      if (s->n != NULL)
        scene_traverse_r(s->n, p, fun);
    }
    else {
      if (s->n != NULL)
        scene_traverse_r(s->n, p, fun);
      (*fun)(&s->s);
      if (s->p != NULL)
        scene_traverse_r(s->p, p, fun);
    }
  }
}
void scene_traverse(scene *s,
    point p, void (*fun)(face*)) {
  scene_traverse_r(s->root, p, fun);
}

