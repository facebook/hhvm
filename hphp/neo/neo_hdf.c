/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#include "cs_config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include <windows.h>
#include <direct.h>
#include <io.h>
#define PATH_MAX MAX_PATH
#else
#include <unistd.h>
#endif

#include "neo_misc.h"
#include "neo_err.h"
#include "neo_rand.h"
#include "neo_hdf.h"
#include "neo_str.h"
#include "neo_files.h"
#include "ulist.h"

static NEOERR* hdf_read_file_internal (HDF *hdf, const char *path,
                                       int include_handle);


static char *_read_file(FILE *f) {
  int size = 1024;
  char *ret = malloc(size + 1);
  char *newret;

  size_t nread;
  char *buf = ret;
  int len = size;
  while ((nread = fread(buf, 1, len, f))) {
    buf += nread;
    len -= nread;
    if (len == 0) {
      len = size;
      size <<= 1;

      newret = realloc(ret, size + 1);
      buf = buf - ret + newret;
      ret = newret;
    }
  }
  *buf = '\0';
  return ret;
}

/* Ok, in order to use the hash, we have to support n-len strings
 * instead of null terminated strings (since in set_value and walk_hdf
 * we are merely using part of the HDF name for lookup, and that might
 * be a const, and we don't want the overhead of allocating/copying
 * that data out...)
 *
 * Since HASH doesn't maintain any data placed in it, merely pointers to
 * it, we use the HDF node itself as the key, and have specific
 * comp/hash functions which just use the name/name_len as the key.
 */

static int hash_hdf_comp(const void *a, const void *b)
{
  HDF *ha = (HDF *)a;
  HDF *hb = (HDF *)b;

  return (ha->name_len == hb->name_len) && !strncmp(ha->name, hb->name, ha->name_len);
}

static UINT32 hash_hdf_hash(const void *a)
{
  HDF *ha = (HDF *)a;
  return ne_crc((UINT8 *)(ha->name), ha->name_len);
}

static NEOERR *_alloc_hdf (HDF **hdf, const char *name, size_t nlen,
                           const char *value, int dupl, int wf, HDF *top, int wc)
{
  *hdf = calloc (1, sizeof (HDF));
  if (*hdf == NULL)
  {
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory for hdf element");
  }

  (*hdf)->top = top;
  (*hdf)->is_wildcard = wc;

  if (name != NULL)
  {
    (*hdf)->name_len = nlen;
    (*hdf)->name = (char *) malloc (nlen + 1);
    if ((*hdf)->name == NULL)
    {
      free((*hdf));
      (*hdf) = NULL;
      return nerr_raise (NERR_NOMEM,
         "Unable to allocate memory for hdf element: %s", name);
    }
    strncpy((*hdf)->name, name, nlen);
    (*hdf)->name[nlen] = '\0';
  }
  if (value != NULL)
  {
    if (dupl)
    {
      (*hdf)->alloc_value = 1;
      (*hdf)->value = strdup(value);
      if ((*hdf)->value == NULL)
      {
        free((*hdf)->name);
        free((*hdf));
        (*hdf) = NULL;
        return nerr_raise (NERR_NOMEM,
            "Unable to allocate memory for hdf element %s", name);
      }
    }
    else
    {
      (*hdf)->alloc_value = wf;
      /* We're overriding the const of value here for the set_buf case
       * where we overrode the char * to const char * earlier, since
       * alloc_value actually keeps track of the const-ness for us */
      (*hdf)->value = (char *)value;
    }
  }
  return STATUS_OK;
}

static void _dealloc_hdf (HDF **hdf)
{
  HDF *myhdf = *hdf;
  HDF *next = NULL;

  if (myhdf == NULL) return;
  if (myhdf->child != NULL)
    _dealloc_hdf(&(myhdf->child));

  /* This was easier recursively, but dangerous on long lists, so we
   * walk it ourselves */
  next = myhdf->next;
  while (next != NULL)
  {
    myhdf->next = next->next;
    next->next = NULL;
    _dealloc_hdf(&next);
    next = myhdf->next;
  }
  if (myhdf->name != NULL)
  {
    free (myhdf->name);
    myhdf->name = NULL;
  }
  if (myhdf->value != NULL)
  {
    if (myhdf->alloc_value)
      free (myhdf->value);
    myhdf->value = NULL;
  }
  if (myhdf->hash != NULL)
  {
    ne_hash_destroy(&myhdf->hash);
  }
  free(myhdf);
  *hdf = NULL;
}

NEOERR* hdf_init (HDF **hdf)
{
  NEOERR *err;
  HDF *my_hdf;

  *hdf = NULL;

  err = nerr_init();
  if (err != STATUS_OK)
    return nerr_pass (err);

  err = _alloc_hdf (&my_hdf, NULL, 0, NULL, 0, 0, NULL, 0);
  if (err != STATUS_OK)
    return nerr_pass (err);

  my_hdf->top = my_hdf;

  *hdf = my_hdf;

  return STATUS_OK;
}

void hdf_destroy (HDF **hdf)
{
  if (*hdf == NULL) return;
  if ((*hdf)->top == (*hdf))
  {
    _dealloc_hdf(hdf);
  }
}

#define WALK_MAX_DEPTH 1000

static int _walk_hdf (HDF *hdf, const char *name, HDF **node, NEOERR** err)
{
  HDF *parent = NULL;
  HDF *hp;
  HDF hash_key;
  int x = 0;
  const char *s, *n;

  *node = NULL;

  if (hdf == NULL) return -1;
  if (name == NULL || name[0] == '\0')
  {
    *node = hdf;
    return 0;
  }

  parent = hdf;
  hp = hdf->child;
  if (hp == NULL)
  {
    return -1;
  }

  n = name;
  s = strchr (n, '.');
  x = (s == NULL) ? strlen(n) : s - n;

  while (1)
  {
    if (parent && parent->hash)
    {
      hash_key.name = (char *)n;
      hash_key.name_len = x;
      hp = ne_hash_lookup(parent->hash, &hash_key);
    }
    else
    {
      while (hp != NULL)
      {
        if (hp->name && (x == hp->name_len) && !strncmp(hp->name, n, x))
        {
          break;
        }
        else
        {
          hp = hp->next;
        }
      }
    }
    if (hp == NULL)
    {
      return -1;
    }
    if (s == NULL) break;

    parent = hp;
    hp = hp->child;
    n = s + 1;
    s = strchr (n, '.');
    x = (s == NULL) ? strlen(n) : s - n;
  }

  *node = hp;
  return 0;
}

HDF* hdf_get_obj (HDF *hdf, const char *name, NEOERR** err)
{
  HDF *obj;

  _walk_hdf(hdf, name, &obj, err);
  return obj;
}

HDF* hdf_get_child (HDF *hdf, const char *name, NEOERR** err)
{
  HDF *obj;
  _walk_hdf(hdf, name, &obj, err);
  if (obj != NULL) return obj->child;
  return obj;
}

void hdf_set_visited (HDF *hdf, int visited) {
  if (hdf) hdf->visited = visited;
}

int hdf_is_visited (HDF *hdf) {
  return hdf ? hdf->visited : 0;
}

int hdf_is_wildcard (HDF *hdf) {
  return hdf ? hdf->is_wildcard : 0;
}

HDF* hdf_obj_child (HDF *hdf, NEOERR** err)
{
  if (hdf == NULL) return NULL;
  return hdf->child;
}

HDF* hdf_obj_next (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->next;
}

char* hdf_obj_name (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->name;
}

char* hdf_obj_value (HDF *hdf, NEOERR** err)
{
  if (hdf == NULL) return NULL;
  return hdf->value;
}

NEOERR* _hdf_hash_level(HDF *hdf)
{
  NEOERR *err;
  HDF *child;

  err = ne_hash_init(&(hdf->hash), hash_hdf_hash, hash_hdf_comp);
  if (err) return nerr_pass(err);

  child = hdf->child;
  while (child)
  {
    err = ne_hash_insert(hdf->hash, child, child);
    if (err) return nerr_pass(err);
    child = child->next;
  }
  return STATUS_OK;
}

static NEOERR* _set_value (HDF *hdf, const char *name, const char *value,
                           int dupl, int wf, HDF **set_node, int wc)
{
  NEOERR *err;
  HDF *hn, *hp, *hs;
  HDF hash_key;
  int x = 0;
  const char *s = name;
  const char *n = name;
  int count = 0;

  if (set_node != NULL) *set_node = NULL;
  if (hdf == NULL)
  {
    return nerr_raise(NERR_ASSERT, "Unable to set %s on NULL hdf", name);
  }

  /* HACK: allow setting of this node by passing an empty name */
  if (name == NULL || name[0] == '\0')
  {
    /* if we're setting ourselves to ourselves... */
    if (hdf->value == value)
    {
      if (set_node != NULL) *set_node = hdf;
      return STATUS_OK;
    }
    if (hdf->alloc_value)
    {
      free(hdf->value);
      hdf->value = NULL;
    }
    if (value == NULL)
    {
      hdf->alloc_value = 0;
      hdf->value = NULL;
    }
    else if (dupl)
    {
      hdf->alloc_value = 1;
      hdf->value = strdup(value);
      if (hdf->value == NULL)
        return nerr_raise (NERR_NOMEM, "Unable to duplicate value %s for %s",
            value, hdf->name);
    }
    else
    {
      hdf->alloc_value = wf;
      hdf->value = (char *)value;
    }
    if (set_node != NULL) *set_node = hdf;
    return STATUS_OK;
  }

  n = name;
  s = strchr (n, '.');
  x = (s != NULL) ? s - n : strlen(n);
  if (x == 0)
  {
    return nerr_raise(NERR_ASSERT, "Unable to set Empty component %s", name);
  }

  hn = hdf;

  while (1)
  {
    /* examine cache to see if we have a match */
    count = 0;
    hp = hn->last_hp;
    hs = hn->last_hs;

    if ((hs == NULL && hp == hn->child) || (hs && hs->next == hp))
    {
      if (hp && hp->name && (x == hp->name_len) && !strncmp (hp->name, n, x))
      {
        goto skip_search;
      }
    }

    hp = hn->child;
    hs = NULL;

    /* Look for a matching node at this level */
    if (hn->hash != NULL)
    {
      hash_key.name = (char *)n;
      hash_key.name_len = x;
      hp = ne_hash_lookup(hn->hash, &hash_key);
      hs = hn->last_child;
    }
    else
    {
      while (hp != NULL)
      {
        if (hp->name && (x == hp->name_len) && !strncmp(hp->name, n, x))
        {
          break;
        }
        hs = hp;
        hp = hp->next;
        count++;
      }
    }

    /* save in cache any value we found */
    if (hp) {
      hn->last_hp = hp;
      hn->last_hs = hs;
    }

skip_search:

    if (hp == NULL)
    {
      /* If there was no matching node at this level, we need to
       * allocate an intersitial node (or the actual node if we're
       * at the last part of the HDF name) */
      if (s != NULL)
      {
        /* intersitial */
        err = _alloc_hdf (&hp, n, x, NULL, 0, 0, hdf->top, 0);
      }
      else
      {
        err = _alloc_hdf (&hp, n, x, value, dupl, wf, hdf->top, wc);
      }
      if (err != STATUS_OK)
        return nerr_pass (err);
      if (hn->child == NULL)
        hn->child = hp;
      else
        hs->next = hp;
      hn->last_child = hp;

      /* This is the point at which we convert to a hash table
       * at this level, if we're over the count */
      if (count > FORCE_HASH_AT && hn->hash == NULL)
      {
        err = _hdf_hash_level(hn);
        if (err) return nerr_pass(err);
      }
      else if (hn->hash != NULL)
      {
        err = ne_hash_insert(hn->hash, hp, hp);
        if (err) return nerr_pass(err);
      }
    }
    else if (s == NULL)
    {
      /* If there is a matching node and we're at the end of the HDF
       * name, then we update the value of the node */
      if (hp->value != value)
      {
        if (hp->alloc_value)
        {
          free(hp->value);
          hp->value = NULL;
        }
        if (value == NULL)
        {
          hp->alloc_value = 0;
          hp->value = NULL;
        }
        else if (dupl)
        {
          hp->alloc_value = 1;
          hp->value = strdup(value);
          if (hp->value == NULL)
            return nerr_raise (NERR_NOMEM, "Unable to duplicate value %s for %s",
                value, name);
        }
        else
        {
          hp->alloc_value = wf;
          hp->value = (char *)value;
        }
      }
    }
    /* At this point, we're done if there is not more HDF name space to
     * traverse */
    if (s == NULL)
      break;
    /* Otherwise, we need to find the next part of the namespace */
    n = s + 1;
    s = strchr (n, '.');
    x = (s != NULL) ? s - n : strlen(n);
    if (x == 0)
    {
      return nerr_raise(NERR_ASSERT, "Unable to set Empty component %s", name);
    }
    hn = hp;
  }
  if (set_node != NULL) *set_node = hp;
  return STATUS_OK;
}

NEOERR* hdf_set_value (HDF *hdf, const char *name, const char *value)
{
  return nerr_pass(_set_value (hdf, name, value, 1, 1, NULL, 0));
}

NEOERR* hdf_get_node (HDF *hdf, const char *name, HDF **ret)
{
  NEOERR* err = STATUS_OK;
  _walk_hdf(hdf, name, ret, &err);
  if (*ret == NULL)
  {
    if (err != STATUS_OK) return err;
    return nerr_pass(_set_value (hdf, name, NULL, 0, 1, ret, 0));
  }
  return STATUS_OK;
}

NEOERR* hdf_remove_tree (HDF *hdf, const char *name)
{
  HDF *hp = hdf;
  HDF *lp = NULL, *ln = NULL; /* last parent, last node */
  int x = 0;
  const char *s = name;
  const char *n = name;

  if (hdf == NULL) return STATUS_OK;

  hp = hdf->child;
  if (hp == NULL)
  {
    return STATUS_OK;
  }

  lp = hdf;
  ln = NULL;

  n = name;
  s = strchr (n, '.');
  x = (s == NULL) ? strlen(n) : s - n;

  while (1)
  {
    while (hp != NULL)
    {
      if (hp->name && (x == hp->name_len) && !strncmp(hp->name, n, x))
      {
      break;
      }
      else
      {
        ln = hp;
        hp = hp->next;
      }
    }
    if (hp == NULL)
    {
      return STATUS_OK;
    }
    if (s == NULL) break;

    lp = hp;
    ln = NULL;
    hp = hp->child;
    n = s + 1;
    s = strchr (n, '.');
    x = (s == NULL) ? strlen(n) : s - n;
  }

  if (lp->hash != NULL)
  {
    ne_hash_remove(lp->hash, hp);
  }
  if (ln)
  {
    ln->next = hp->next;
    /* check to see if we are the last parent's last_child, if so
     * repoint so hash table inserts will go to the right place */
    if (hp == lp->last_child)
      lp->last_child = ln;
    hp->next = NULL;
  }
  else
  {
    lp->child = hp->next;
    hp->next = NULL;
  }
  _dealloc_hdf (&hp);

  return STATUS_OK;
}

static NEOERR * _copy_nodes (HDF *dest, HDF *src)
{
  NEOERR *err = STATUS_OK;
  HDF *dt, *st;

  st = src->child;
  while (st != NULL)
  {
    if (err) return nerr_pass(err);
    err = _set_value(dest, st->name, st->value, 1, 1, &dt, st->is_wildcard);
    if (err) {
      return nerr_pass(err);
    }
    if (src->child)
    {
      err = _copy_nodes (dt, st);
      if (err) return nerr_pass(err);
    }
    st = st->next;
  }
  return STATUS_OK;
}

NEOERR* hdf_copy (HDF *dest, const char *name, HDF *src)
{
  NEOERR *err;
  HDF *node;

  err = STATUS_OK;
  if (_walk_hdf(dest, name, &node, &err) == -1)
  {
    if (err) return err;
    if (err) return nerr_pass(err);
    err = _set_value (dest, name, src->value, 1, 1, &node, src->is_wildcard);
    if (err) {
      return nerr_pass(err);
    }
  }
  return nerr_pass (_copy_nodes (node, src));
}

static void gen_ml_break(char *ml, size_t len)
{
  int nlen;
  int x = 0;

  ml[x++] = '\n';
  nlen = 2 + neo_rand(len-5);
  if (nlen == 0)
  {
    nlen = len / 2;
  }
  while (nlen)
  {
    ml[x++] = ('A' + neo_rand(26));
    nlen--;
  }
  ml[x++] = '\n';
  ml[x] = '\0';
}

typedef NEOERR *(*DUMPF_CB)(void *rock, const char *fmt, ...);

static NEOERR *_fp_dump_cb (void *rock, const char *fmt, ...)
{
  FILE *fp = (FILE *)rock;
  va_list ap;

  va_start (ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  return STATUS_OK;
}

static NEOERR *_string_dump_cb (void *rock, const char *fmt, ...)
{
  NEOERR *err;
  NEOSTRING *str = (NEOSTRING *)rock;
  va_list ap;

  va_start (ap, fmt);
  err = string_appendvf(str, fmt, ap);
  va_end(ap);
  return nerr_pass(err);
}

#define DUMP_TYPE_DOTTED 0
#define DUMP_TYPE_COMPACT 1
#define DUMP_TYPE_PRETTY 2

static NEOERR* hdf_dump_cb(HDF *hdf, const char *prefix, int dtype, int lvl,
                           void *rock, DUMPF_CB dump_cbf)
{
  NEOERR *err;
  char *p, op;
  char ml[10] = "\nEOM\n";
  int ml_len = strlen(ml);
  char whsp[256] = "";

  if (dtype == DUMP_TYPE_PRETTY)
  {
    memset(whsp, ' ', 256);
    if (lvl > 127)
      lvl = 127;
    whsp[lvl*2] = '\0';
  }

  if (hdf != NULL) hdf = hdf->child;

  while (hdf != NULL)
  {
    op = '=';
    if (hdf->value)
    {
      if (prefix && (dtype == DUMP_TYPE_DOTTED))
      {
        err = dump_cbf(rock, "%s.%s", prefix, hdf->name);
      }
      else
      {
        err = dump_cbf(rock, "%s%s", whsp, hdf->name);
      }
      if (err) return nerr_pass (err);
      if (strchr (hdf->value, '\n'))
      {
        int vlen = strlen(hdf->value);

        while (strstr(hdf->value, ml) || ((vlen > ml_len) && !strncmp(hdf->value + vlen - ml_len + 1, ml, strlen(ml) - 1)))
        {
          gen_ml_break(ml, sizeof(ml));
          ml_len = strlen(ml);
        }
        if (hdf->value[strlen(hdf->value)-1] != '\n')
          err = dump_cbf(rock, " << %s%s%s", ml+1, hdf->value, ml);
        else
          err = dump_cbf(rock, " << %s%s%s", ml+1, hdf->value, ml+1);
      }
      else
      {
        err = dump_cbf(rock, " %c %s\n", op, hdf->value);
      }
      if (err) return nerr_pass (err);
    }
    if (hdf->child)
    {
      if (prefix && (dtype == DUMP_TYPE_DOTTED))
      {
        size_t p_len = strlen(hdf->name) + strlen(prefix) + 2;
        p = (char *) malloc (p_len);
        snprintf (p, p_len, "%s.%s", prefix, hdf->name);

        err = hdf_dump_cb (hdf, p, dtype, lvl+1, rock, dump_cbf);
        free(p);
      }
      else
      {
        if (hdf->name && (dtype != DUMP_TYPE_DOTTED))
        {
          err = dump_cbf(rock, "%s%s {\n", whsp, hdf->name);
          if (err) return nerr_pass (err);
          err = hdf_dump_cb (hdf, hdf->name, dtype, lvl+1, rock, dump_cbf);
          if (err) return nerr_pass (err);
          err = dump_cbf(rock, "%s}\n", whsp);
        }
        else
        {
          err = hdf_dump_cb (hdf, hdf->name, dtype, lvl+1, rock, dump_cbf);
        }
      }
      if (err) return nerr_pass (err);
    }
    hdf = hdf->next;
  }
  return STATUS_OK;
}

NEOERR* hdf_dump_str (HDF *hdf, const char *prefix, int dtype, NEOSTRING *str)
{
  return nerr_pass(hdf_dump_cb(hdf, prefix, dtype, 0, str, _string_dump_cb));
}

NEOERR* hdf_dump_format (HDF *hdf, int lvl, FILE *fp)
{
  return nerr_pass(hdf_dump_cb(hdf, "", DUMP_TYPE_PRETTY, 0, fp, _fp_dump_cb));
}

NEOERR *hdf_write_file (HDF *hdf, const char *path)
{
  NEOERR *err;
  FILE *fp;

  fp = fopen(path, "w");
  if (fp == NULL)
    return nerr_raise_errno (NERR_IO, "Unable to open %s for writing", path);

  err = hdf_dump_format (hdf, 0, fp);

  fclose (fp);
  if (err)
  {
    unlink(path);
  }
  return nerr_pass(err);
}

NEOERR *hdf_write_string (HDF *hdf, char **s)
{
  NEOSTRING str;
  NEOERR *err;

  *s = NULL;

  string_init (&str);

  err = hdf_dump_str (hdf, NULL, 2, &str);
  if (err)
  {
    string_clear (&str);
    return nerr_pass(err);
  }
  if (str.buf == NULL)
  {
    *s = strdup("");
    if (*s == NULL) return nerr_raise(NERR_NOMEM, "Unable to allocate empty string");
  }
  else
  {
    *s = str.buf;
  }

  return STATUS_OK;
}


#define SKIPWS(s) while (*s && isspace(*s)) s++;

static int _copy_line (const char **s, char *buf, size_t buf_len)
{
  int x = 0;
  const char *st = *s;

  while (*st && x < buf_len-1)
  {
    buf[x++] = *st;
    if (*st++ == '\n') break;
  }
  buf[x] = '\0';
  *s = st;

  return x;
}

/* Copy the characters in the file (up to the next newline) into line
 * and advance s to the next line */
static NEOERR *_copy_line_advance(const char **s, NEOSTRING *line)
{
  NEOERR *err;
  int x = 0;
  const char *st = *s;
  const char *nl;

  nl = strchr(st, '\n');
  if (nl == NULL)
  {
    x = strlen(st);
    err = string_appendn(line, st, x);
    if (err) return nerr_pass(err);
    *s = st + x;
  }
  else
  {
    x = nl - st;
    err = string_appendn(line, st, x);
    if (err) return nerr_pass(err);
    *s = nl + 1;
  }

  return STATUS_OK;
}

char *_strndup(const char *s, int len) {
  int x;
  char *dupl;
  if (s == NULL) return NULL;
  dupl = (char *) malloc(len+1);
  if (dupl == NULL) return NULL;
  for (x = 0; x < len && s[x]; x++)
  {
    dupl[x] = s[x];
  }
  dupl[x] = '\0';
  dupl[len] = '\0';
  return dupl;
}

#define INCLUDE_ERROR -1
#define INCLUDE_IGNORE -2
#define INCLUDE_FILE 0
#define INCLUDE_MAX_DEPTH 50

static NEOERR* _hdf_read_string (HDF *hdf, const char **str, NEOSTRING *line,
                                 const char *path, int *lineno,
                                 int include_handle, int expect_end_brace) {
  NEOERR *err;
  HDF *lower;
  char *s;
  char *name, *value;
  int is_wildcard;

  while (**str != '\0')
  {
    /* Reset string length, but don't free the reserved buffer */
    line->len = 0;
    err = _copy_line_advance(str, line);
    if (err) return nerr_pass(err);
    (*lineno)++;
    s = line->buf;
    is_wildcard = 0;
    SKIPWS(s);
    if (!strncmp(s, "#include ", 9) && include_handle != INCLUDE_IGNORE)
    {
      if (include_handle == INCLUDE_ERROR)
      {
        return nerr_raise (NERR_PARSE,
                           "[%d]: #include not supported in string parse",
                           *lineno);
      }
      else if (include_handle < INCLUDE_MAX_DEPTH)
      {
        int l;
        s += 9;
        name = neos_strip(s);
        l = strlen(name);
        if (name[0] == '"' && name[l-1] == '"')
        {
          name[l-1] = '\0';
          name++;
        }
        char fullpath[PATH_MAX + 2];
        if (name[0] != '/') {
          memset(fullpath, 0, PATH_MAX);

          char *p = strrchr(path, '/');
          if (p == NULL) {
            char pwd[PATH_MAX];
            memset(pwd, 0, PATH_MAX);
            getcwd(pwd, PATH_MAX);
            snprintf(fullpath, sizeof(fullpath), "%s/%s", pwd, name);
          } else {
            int dir_len = p - path + 1;
            snprintf(fullpath, sizeof(fullpath), "%s", path);
            snprintf(fullpath + dir_len, sizeof(fullpath) - dir_len, "%s", name);
          }
          name = fullpath;
        }
        err = hdf_read_file_internal(hdf, name, include_handle + 1);
        if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else {
        return nerr_raise (NERR_MAX_RECURSION,
                           "[%d]: Too many recursion levels.",
                           *lineno
                           );
      }
    }
    else if (s[0] == '#')
    {
      /* comment: pass */
    }
    else if (s[0] == '}') /* up */
    {
      s = neos_strip(s);
      if (strcmp(s, "}"))
      {
        err = nerr_raise(NERR_PARSE,
            "[%s:%d] Trailing garbage on line following }: %s", path, *lineno,
            line->buf);
        return err;
      }
      return STATUS_OK;
    }
    else if (s[0])
    {
      /* Valid hdf name is [0-9a-zA-Z_.*\]+ */
      int splice = *s == '@';
      char num[256];
      static int counter = 0;
      if (splice) s++;
      name = s;
      if (*s == '*') {
        if (*s++ && (isalnum(*s) || *s == '_' || *s == '.' || *s == '\\')) {
          return nerr_raise(NERR_PARSE, "Illegal name containing '*'");
        }
        snprintf(num, 256, "%d", counter++);
        name = num;
        is_wildcard = 1;
      } else {
        int saw_star = 0;
        while (*s && (isalnum(*s) || *s == '_' || *s == '.' || *s == '\\')) {
          if (saw_star || (*s++ != '.' && *s == '*')) {
            return nerr_raise(NERR_PARSE, "Illegal name containing '*'");
          } else if (*s == '*') {
            if (splice) {
              return nerr_raise(NERR_PARSE,
                                "Illegal splice name containing '*'");
            }
            *s++ = '\0';
            snprintf(num, 256, "%s%i", name, counter++);
            name = num;
            is_wildcard = 1;
            saw_star = 1;
          }
        }
      }
      SKIPWS(s);

      if (s[0] == '[') /* attributes */
      {
        return nerr_raise(NERR_PARSE, "Illegal attribute");
      }
      if (splice) {
        name = neos_strip(name);
        HDF *h = hdf_get_obj(hdf->top, name, &err);
        if (err != STATUS_OK) {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
        if (h) {
          HDF *c = hdf_obj_child(h, &err);
          if (err != STATUS_OK) {
            return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
          }
          while (c) {
            err = hdf_copy (hdf, hdf_obj_name(c), c);
            if (err != STATUS_OK) break;
            c = hdf_obj_next(c);
          }
        }
        if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      } else if (s[0] == '=') /* assignment */
      {
        *s = '\0';
        name = neos_strip(name);
        s++;
        value = neos_strip(s);
        err = _set_value (hdf, name, value, 1, 1, NULL, is_wildcard);
        if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == ':' && s[1] == '=') /* copy */
      {
        return nerr_raise(NERR_PARSE, "Illegal copy syntax: ':='");
      }
      else if (s[0] == '!' && s[1] == '=') /* exec */
      {
        return nerr_raise(NERR_PARSE, "Illegal exec syntax: '!='");
      }
      else if (s[0] == ':') /* link */
      {
        return nerr_raise(NERR_PARSE, "Illegal link syntax: ':'");
      }
      else if (s[0] == '{') /* deeper */
      {
        *s = '\0';
        name = neos_strip(name);
        lower = hdf_get_obj (hdf, name, &err);
        if (lower == NULL)
        {
          err = _set_value (hdf, name, NULL, 1, 1, &lower, is_wildcard);
        }
        else
        {
          err = _set_value (lower, NULL, lower->value, 1, 1, NULL, is_wildcard);
        }
        if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
        err = _hdf_read_string (lower, str, line, path, lineno, include_handle,
                                1);
        if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == '<' && s[1] == '<') /* multi-line assignment */
      {
        char *m;
        int msize = 0;
        int mmax = 128;
        int l;

        *s = '\0';
        name = neos_strip(name);
        s+=2;
        value = neos_strip(s);
        l = strlen(value);
        if (l == 0)
        {
          err = nerr_raise(NERR_PARSE,
            "[%s:%d] No multi-assignment terminator given: %s", path, *lineno,
            line->buf);
          return err;
        }
        m = (char *) malloc (mmax * sizeof(char));
        if (m == NULL)
        {
          return nerr_raise(NERR_NOMEM,
            "[%s:%d] Unable to allocate memory for multi-line assignment to %s",
            path, *lineno, name);
        }
        while (_copy_line (str, m+msize, mmax-msize) != 0)
        {
          (*lineno)++;
          if (!strncmp(value, m+msize, l) && isspace(m[msize+l]))
          {
            m[msize] = '\0';
            break;
          }
          msize += strlen(m+msize);
          if (msize + l + 10 > mmax)
          {
            void *new_ptr;
            mmax += 128;
            new_ptr = realloc (m, mmax * sizeof(char));
            if (new_ptr == NULL)
            {
              free(m);
              return nerr_raise(NERR_NOMEM,
                "[%s:%d] Unable to allocate memory for multi-line assignment to %s: size=%d",
                path, *lineno, name, mmax);
            }
            m = (char *) new_ptr;
          }
        }
        err = _set_value (hdf, name, m, 0, 1, NULL, is_wildcard);
        if (err != STATUS_OK)
        {
          free (m);
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }

      }
      else
      {
        err = nerr_raise(NERR_PARSE, "[%s:%d] Unable to parse line %s", path,
            *lineno, line->buf);
        return err;
      }
    }
  }
  if (expect_end_brace) {
    err = nerr_raise(NERR_PARSE, "[%s:%d] Missing matching }", path, *lineno);
    return err;
  }
  return STATUS_OK;
}

NEOERR * hdf_read_string (HDF *hdf, const char *str)
{
  NEOERR *err;
  int lineno = 0;
  NEOSTRING line;
  string_init(&line);
  err = _hdf_read_string(hdf, &str, &line, "<string>", &lineno, INCLUDE_ERROR,
                         0);
  string_clear(&line);
  return nerr_pass(err);
}

static NEOERR* hdf_read_file_internal (HDF *hdf, const char *path,
                                       int include_handle)
{
  NEOERR *err;
  int lineno = 0;
  char *ibuf = NULL;
  const char *ptr = NULL;
  NEOSTRING line;

  string_init(&line);

  if (path == NULL) return nerr_raise(NERR_ASSERT, "Can't read NULL file");

  err = ne_load_file (path, &ibuf);
  if (err) return nerr_pass(err);

  ptr = ibuf;
  err = _hdf_read_string(hdf, &ptr, &line, path, &lineno, include_handle, 0);
  free(ibuf);
  string_clear(&line);
  return nerr_pass(err);
}

NEOERR* hdf_read_file (HDF *hdf, const char *path)
{
  NEOERR *err;
  err = hdf_read_file_internal (hdf, path, INCLUDE_FILE);
  return nerr_pass(err);
}
