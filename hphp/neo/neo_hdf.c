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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/stat.h>
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
                           const char *value, int dupl, int wf, HDF *top)
{
  *hdf = calloc (1, sizeof (HDF));
  if (*hdf == NULL)
  {
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory for hdf element");
  }

  (*hdf)->top = top;

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

static void _dealloc_hdf_attr(HDF_ATTR **attr)
{
  HDF_ATTR *next;

  while ((*attr) != NULL)
  {
    next = (*attr)->next;
    if ((*attr)->key) free((*attr)->key);
    if ((*attr)->value) free((*attr)->value);
    free(*attr);
    *attr = next;
  }
  *attr = NULL;
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
  if (myhdf->attr != NULL)
  {
    _dealloc_hdf_attr(&(myhdf->attr));
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

  err = _alloc_hdf (&my_hdf, NULL, 0, NULL, 0, 0, NULL);
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

static int _walk_hdf (HDF *hdf, const char *name, HDF **node)
{
  HDF *parent = NULL;
  HDF *hp = hdf;
  HDF hash_key;
  int x = 0;
  const char *s, *n;
  int r;

  *node = NULL;

  if (hdf == NULL) return -1;
  if (name == NULL || name[0] == '\0')
  {
    *node = hdf;
    return 0;
  }

  if (hdf->link)
  {
    r = _walk_hdf (hdf->top, hdf->value, &hp);
    if (r) return r;
    if (hp)
    {
      parent = hp;
      hp = hp->child;
    }
  }
  else
  {
    parent = hdf;
    hp = hdf->child;
  }
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

    if (hp->link)
    {
      r = _walk_hdf (hp->top, hp->value, &hp);
      if (r) {
	return r;
      }
      parent = hp;
      hp = hp->child;
    }
    else
    {
      parent = hp;
      hp = hp->child;
    }
    n = s + 1;
    s = strchr (n, '.');
    x = (s == NULL) ? strlen(n) : s - n;
  }
  if (hp->link)
  {
    return _walk_hdf (hp->top, hp->value, node);
  }

  *node = hp;
  return 0;
}

int hdf_get_int_value (HDF *hdf, const char *name, int defval)
{
  HDF *node;
  int v;
  char *n;

  if ((_walk_hdf(hdf, name, &node) == 0) && (node->value != NULL))
  {
    v = strtol (node->value, &n, 10);
    if (node->value == n) v = defval;
    return v;
  }
  return defval;
}

/* This should return a const char *, but changing this would have big
 * repurcussions for any C code using this function, so no change for now */
char* hdf_get_value (HDF *hdf, const char *name, const char *defval)
{
  HDF *node;

  if ((_walk_hdf(hdf, name, &node) == 0) && (node->value != NULL))
  {
    return node->value;
  }
  return (char *)defval;
}

char* hdf_get_valuevf (HDF *hdf, const char *namefmt, va_list ap)
{
  HDF *node;
  char *name;

  name = vsprintf_alloc(namefmt, ap);
  if (name == NULL) return NULL;
  if ((_walk_hdf(hdf, name, &node) == 0) && (node->value != NULL))
  {
    free(name);
    return node->value;
  }
  free(name);
  return NULL;
}

char* hdf_get_valuef (HDF *hdf, const char *namefmt, ...)
{
  char *val;
  va_list ap;

  va_start(ap, namefmt);
  val = hdf_get_valuevf(hdf, namefmt, ap);
  va_end(ap);
  return val;
}

NEOERR* hdf_get_copy (HDF *hdf, const char *name, char **value,
                      const char *defval)
{
  HDF *node;

  if ((_walk_hdf(hdf, name, &node) == 0) && (node->value != NULL))
  {
    *value = strdup(node->value);
    if (*value == NULL)
    {
      return nerr_raise (NERR_NOMEM, "Unable to allocate copy of %s", name);
    }
  }
  else
  {
    if (defval == NULL)
      *value = NULL;
    else
    {
      *value = strdup(defval);
      if (*value == NULL)
      {
	return nerr_raise (NERR_NOMEM, "Unable to allocate copy of %s", name);
      }
    }
  }
  return STATUS_OK;
}

HDF* hdf_get_obj (HDF *hdf, const char *name)
{
  HDF *obj;

  _walk_hdf(hdf, name, &obj);
  return obj;
}

HDF* hdf_get_child (HDF *hdf, const char *name)
{
  HDF *obj;
  _walk_hdf(hdf, name, &obj);
  if (obj != NULL) return obj->child;
  return obj;
}

HDF_ATTR* hdf_get_attr (HDF *hdf, const char *name)
{
  HDF *obj;
  _walk_hdf(hdf, name, &obj);
  if (obj != NULL) return obj->attr;
  return NULL;
}

NEOERR* hdf_set_attr (HDF *hdf, const char *name, const char *key,
                      const char *value)
{
  HDF *obj;
  HDF_ATTR *attr, *last;

  _walk_hdf(hdf, name, &obj);
  if (obj == NULL)
    return nerr_raise(NERR_ASSERT, "Unable to set attribute on none existant node");

  if (obj->attr != NULL)
  {
    attr = obj->attr;
    last = attr;
    while (attr != NULL)
    {
      if (!strcmp(attr->key, key))
      {
	if (attr->value) free(attr->value);
	/* a set of NULL deletes the attr */
	if (value == NULL)
	{
	  if (attr == obj->attr)
	    obj->attr = attr->next;
	  else
	    last->next = attr->next;
	  free(attr->key);
	  free(attr);
	  return STATUS_OK;
	}
	attr->value = strdup(value);
	if (attr->value == NULL)
	  return nerr_raise(NERR_NOMEM, "Unable to set attr %s to %s", key, value);
	return STATUS_OK;
      }
      last = attr;
      attr = attr->next;
    }
    last->next = (HDF_ATTR *) calloc(1, sizeof(HDF_ATTR));
    if (last->next == NULL)
      return nerr_raise(NERR_NOMEM, "Unable to set attr %s to %s", key, value);
    attr = last->next;
  }
  else
  {
    if (value == NULL) return STATUS_OK;
    obj->attr = (HDF_ATTR *) calloc(1, sizeof(HDF_ATTR));
    if (obj->attr == NULL)
      return nerr_raise(NERR_NOMEM, "Unable to set attr %s to %s", key, value);
    attr = obj->attr;
  }
  attr->key = strdup(key);
  attr->value = strdup(value);
  if (attr->key == NULL || attr->value == NULL)
    return nerr_raise(NERR_NOMEM, "Unable to set attr %s to %s", key, value);

  return STATUS_OK;
}

void hdf_set_visited (HDF *hdf, int visited) {
  if (hdf) hdf->visited = visited;
}

int hdf_is_visited (HDF *hdf) {
  return hdf ? hdf->visited : 0;
}

HDF* hdf_obj_child (HDF *hdf)
{
  HDF *obj;
  if (hdf == NULL) return NULL;
  if (hdf->link)
  {
    if (_walk_hdf(hdf->top, hdf->value, &obj))
      return NULL;
    return obj->child;
  }
  return hdf->child;
}

HDF* hdf_obj_next (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->next;
}

HDF* hdf_obj_top (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->top;
}

HDF_ATTR* hdf_obj_attr (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->attr;
}

char* hdf_obj_name (HDF *hdf)
{
  if (hdf == NULL) return NULL;
  return hdf->name;
}

char* hdf_obj_value (HDF *hdf)
{
  int count = 0;

  if (hdf == NULL) return NULL;
  while (hdf->link && count < 100)
  {
    if (_walk_hdf (hdf->top, hdf->value, &hdf))
      return NULL;
    count++;
  }
  return hdf->value;
}

void _merge_attr (HDF_ATTR *dest, HDF_ATTR *src)
{
  HDF_ATTR *da, *ld;
  HDF_ATTR *sa, *ls;
  char found;

  sa = src;
  ls = src;
  while (sa != NULL)
  {
    da = dest;
    ld = da;
    found = 0;
    while (da != NULL)
    {
      if (!strcmp(da->key, sa->key))
      {
	if (da->value) free(da->value);
	da->value = sa->value;
	sa->value = NULL;
	found = 1;
	break;
      }
      ld = da;
      da = da->next;
    }
    if (!found)
    {
      ld->next = sa;
      ls->next = sa->next;
      if (src == sa) src = sa->next;
      ld->next->next = NULL;
      sa = ls->next;
    }
    else
    {
      ls = sa;
      sa = sa->next;
    }
  }
  _dealloc_hdf_attr(&src);
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
                           int dupl, int wf, int lnk, HDF_ATTR *attr,
                           HDF **set_node)
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
    /* handle setting attr first */
    if (hdf->attr == NULL)
    {
      hdf->attr = attr;
    }
    else
    {
      _merge_attr(hdf->attr, attr);
    }
    /* set link flag */
    if (lnk) hdf->link = 1;
    else hdf->link = 0;
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
	    value, name);
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

  if (hdf->link)
  {
    char *new_name = (char *) malloc(strlen(hdf->value) + 1 + strlen(name) + 1);
    if (new_name == NULL)
    {
      return nerr_raise(NERR_NOMEM, "Unable to allocate memory");
    }
    strcpy(new_name, hdf->value);
    strcat(new_name, ".");
    strcat(new_name, name);
    err = _set_value (hdf->top, new_name, value, dupl, wf, lnk, attr, set_node);
    free(new_name);
    return nerr_pass(err);
  }
  else
  {
    hn = hdf;
  }

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
	err = _alloc_hdf (&hp, n, x, NULL, 0, 0, hdf->top);
      }
      else
      {
	err = _alloc_hdf (&hp, n, x, value, dupl, wf, hdf->top);
	if (lnk) hp->link = 1;
	else hp->link = 0;
	hp->attr = attr;
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
      /* handle setting attr first */
      if (hp->attr == NULL)
      {
	hp->attr = attr;
      }
      else
      {
	_merge_attr(hp->attr, attr);
      }
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
      if (lnk) hp->link = 1;
      else hp->link = 0;
    }
    else if (hp->link)
    {
      char *new_name = (char *) malloc(strlen(hp->value) + strlen(s) + 1);
      if (new_name == NULL)
      {
        return nerr_raise(NERR_NOMEM, "Unable to allocate memory");
      }
      strcpy(new_name, hp->value);
      strcat(new_name, s);
      err = _set_value (hdf->top, new_name, value, dupl, wf, lnk, attr, set_node);
      free(new_name);
      return nerr_pass(err);
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
  return nerr_pass(_set_value (hdf, name, value, 1, 1, 0, NULL, NULL));
}

NEOERR* hdf_set_value_attr (HDF *hdf, const char *name, const char *value,
                            HDF_ATTR *attr)
{
  return nerr_pass(_set_value (hdf, name, value, 1, 1, 0, attr, NULL));
}

NEOERR* hdf_set_symlink (HDF *hdf, const char *src, const char *dest)
{
  return nerr_pass(_set_value (hdf, src, dest, 1, 1, 1, NULL, NULL));
}

NEOERR* hdf_set_int_value (HDF *hdf, const char *name, int value)
{
  char buf[256];

  snprintf (buf, sizeof(buf), "%d", value);
  return nerr_pass(_set_value (hdf, name, buf, 1, 1, 0, NULL, NULL));
}

NEOERR* hdf_set_buf (HDF *hdf, const char *name, char *value)
{
  return nerr_pass(_set_value (hdf, name, value, 0, 1, 0, NULL, NULL));
}

NEOERR* hdf_set_copy (HDF *hdf, const char *dest, const char *src)
{
  HDF *node;
  if ((_walk_hdf(hdf, src, &node) == 0) && (node->value != NULL))
  {
    return nerr_pass(_set_value (hdf, dest, node->value, 0, 0, 0, NULL, NULL));
  }
  return nerr_raise (NERR_NOT_FOUND, "Unable to find %s", src);
}

NEOERR* hdf_set_valuevf (HDF *hdf, const char *fmt, va_list ap)
{
  NEOERR *err;
  char *k;
  char *v;

  k = vsprintf_alloc(fmt, ap);
  if (k == NULL)
  {
    return nerr_raise(NERR_NOMEM, "Unable to allocate memory for format string");
  }
  v = strchr(k, '=');
  if (v == NULL)
  {
    err = nerr_raise(NERR_ASSERT, "No equals found: %s", k);
    free(k);
    return err;
  }
  *v++ = '\0';
  err = hdf_set_value(hdf, k, v);
  free(k);
  return nerr_pass(err);
}

NEOERR* hdf_set_valuef (HDF *hdf, const char *fmt, ...)
{
  NEOERR *err;
  va_list ap;

  va_start(ap, fmt);
  err = hdf_set_valuevf(hdf, fmt, ap);
  va_end(ap);
  return nerr_pass(err);
}

NEOERR* hdf_get_node (HDF *hdf, const char *name, HDF **ret)
{
  _walk_hdf(hdf, name, ret);
  if (*ret == NULL)
  {
    return nerr_pass(_set_value (hdf, name, NULL, 0, 1, 0, NULL, ret));
  }
  return STATUS_OK;
}

/* Ok, this version avoids the bubble sort by walking the level once to
 * load them all into a ULIST, qsort'ing the list, and then dumping them
 * back out... */
NEOERR *hdf_sort_obj (HDF *h, int (*compareFunc)(const void *, const void *))
{
  NEOERR *err = STATUS_OK;
  ULIST *level = NULL;
  HDF *p, *c;
  int x;

  if (h == NULL) return STATUS_OK;
  c = h->child;
  if (c == NULL) return STATUS_OK;

  do {
    err = uListInit(&level, 40, 0);
    if (err) return nerr_pass(err);
    for (p = c; p; p = p->next) {
      err = uListAppend(level, p);
      if (err) break;
    }
    err = uListSort(level, compareFunc);
    if (err) break;
    uListGet(level, 0, (void *)&c);
    h->child = c;
    for (x = 1; x < uListLength(level); x++)
    {
      uListGet(level, x, (void *)&p);
      c->next = p;
      p->next = NULL;
      c = p;
    }
    h->last_child = c;
  } while (0);
  uListDestroy(&level, 0);
  return nerr_pass(err);
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

static NEOERR * _copy_attr (HDF_ATTR **dest, HDF_ATTR *src)
{
  HDF_ATTR *copy, *last = NULL;

  *dest = NULL;
  while (src != NULL)
  {
    copy = (HDF_ATTR *)malloc(sizeof(HDF_ATTR));
    if (copy == NULL)
    {
      _dealloc_hdf_attr(dest);
      return nerr_raise(NERR_NOMEM, "Unable to allocate copy of HDF_ATTR");
    }
    copy->key = strdup(src->key);
    copy->value = strdup(src->value);
    copy->next = NULL;
    if ((copy->key == NULL) || (copy->value == NULL))
    {
      _dealloc_hdf_attr(dest);
      return nerr_raise(NERR_NOMEM, "Unable to allocate copy of HDF_ATTR");
    }
    if (last) {
      last->next = copy;
    }
    else
    {
      *dest = copy;
    }
    last = copy;
    src = src->next;
  }
  return STATUS_OK;
}

static NEOERR * _copy_nodes (HDF *dest, HDF *src)
{
  NEOERR *err = STATUS_OK;
  HDF *dt, *st;
  HDF_ATTR *attr_copy;

  st = src->child;
  while (st != NULL)
  {
    err = _copy_attr(&attr_copy, st->attr);
    if (err) return nerr_pass(err);
    err = _set_value(dest, st->name, st->value, 1, 1, st->link, attr_copy, &dt);
    if (err) {
      _dealloc_hdf_attr(&attr_copy);
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
  HDF_ATTR *attr_copy;

  if (_walk_hdf(dest, name, &node) == -1)
  {
    err = _copy_attr(&attr_copy, src->attr);
    if (err) return nerr_pass(err);
    err = _set_value (dest, name, src->value, 1, 1, src->link, attr_copy,
                      &node);
    if (err) {
      _dealloc_hdf_attr(&attr_copy);
      return nerr_pass(err);
    }
  }
  return nerr_pass (_copy_nodes (node, src));
}

/* BUG: currently, this only prints something if there is a value...
 * but we now allow attributes on nodes with no value... */

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
      if (hdf->link) op = ':';
      if (prefix && (dtype == DUMP_TYPE_DOTTED))
      {
	err = dump_cbf(rock, "%s.%s", prefix, hdf->name);
      }
      else
      {
	err = dump_cbf(rock, "%s%s", whsp, hdf->name);
      }
      if (err) return nerr_pass (err);
      if (hdf->attr)
      {
	HDF_ATTR *attr = hdf->attr;
	char *v = NULL;

	err = dump_cbf(rock, " [");
	if (err) return nerr_pass(err);
	while (attr != NULL)
	{
	  if (attr->value == NULL || !strcmp(attr->value, "1"))
	    err = dump_cbf(rock, "%s", attr->key);
	  else
	  {
	    v = repr_string_alloc(attr->value);

	    if (v == NULL)
	      return nerr_raise(NERR_NOMEM, "Unable to repr attr %s value %s", attr->key, attr->value);
	    err = dump_cbf(rock, "%s=%s", attr->key, v);
	    free(v);
	  }
	  if (err) return nerr_pass(err);
	  if (attr->next)
	  {
	    err = dump_cbf(rock, ", ");
	    if (err) return nerr_pass(err);
	  }
	  attr = attr->next;
	}
	err = dump_cbf(rock, "] ");
	if (err) return nerr_pass(err);
      }
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
	p = (char *) malloc (strlen(hdf->name) + strlen(prefix) + 2);
	sprintf (p, "%s.%s", prefix, hdf->name);
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

NEOERR* hdf_dump(HDF *hdf, const char *prefix)
{
  return nerr_pass(hdf_dump_cb(hdf, prefix, DUMP_TYPE_DOTTED, 0, stdout, _fp_dump_cb));
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

NEOERR *hdf_write_file_atomic (HDF *hdf, const char *path)
{
  NEOERR *err;
  FILE *fp;
  char tpath[PATH_BUF_SIZE];
  static int count = 0;

  snprintf(tpath, sizeof(tpath), "%s.%5.5f.%d", path, ne_timef(), count++);

  fp = fopen(tpath, "w");
  if (fp == NULL)
    return nerr_raise_errno (NERR_IO, "Unable to open %s for writing", tpath);

  err = hdf_dump_format (hdf, 0, fp);

  fclose (fp);

  if (err)
  {
    unlink(tpath);
    return nerr_pass(err);
  }
  if (rename(tpath, path) == -1)
  {
    unlink (tpath);
    return nerr_raise_errno (NERR_IO, "Unable to rename file %s to %s",
	tpath, path);
  }

  return STATUS_OK;
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

/* attributes are of the form [key1, key2, key3=value, key4="repr"] */
static NEOERR* parse_attr(char **str, HDF_ATTR **attr)
{
  NEOERR *err = STATUS_OK;
  char *s = *str;
  char *k, *v;
  int k_l, v_l;
  NEOSTRING buf;
  char c;
  HDF_ATTR *ha, *hal = NULL;

  *attr = NULL;

  string_init(&buf);
  while (*s && *s != ']')
  {
    k = s;
    k_l = 0;
    v = NULL;
    v_l = 0;
    while (*s && isalnum(*s)) s++;
    k_l = s-k;
    if (*s == '\0' || k_l == 0)
    {
      _dealloc_hdf_attr(attr);
      return nerr_raise(NERR_PARSE, "Misformed attribute specification: %s", *str);
    }
    SKIPWS(s);
    if (*s == '=')
    {
      s++;
      SKIPWS(s);
      if (*s == '"')
      {
	s++;
	while (*s && *s != '"')
	{
	  if (*s == '\\')
	  {
	    if (isdigit(*(s+1)))
	    {
	      s++;
	      c = *s - '0';
	      if (isdigit(*(s+1)))
	      {
		s++;
		c = (c * 8) + (*s - '0');
		if (isdigit(*(s+1)))
		{
		  s++;
		  c = (c * 8) + (*s - '0');
		}
	      }
	    }
	    else
	    {
	      s++;
	      if (*s == 'n') c = '\n';
	      else if (*s == 't') c = '\t';
	      else if (*s == 'r') c = '\r';
	      else c = *s;
	    }
	    err = string_append_char(&buf, c);
	  }
	  else
	  {
	    err = string_append_char(&buf, *s);
	  }
	  if (err)
	  {
	    string_clear(&buf);
	    _dealloc_hdf_attr(attr);
	    return nerr_pass(err);
	  }
	  s++;
	}
	if (*s == '\0')
	{
	  _dealloc_hdf_attr(attr);
	  string_clear(&buf);
	  return nerr_raise(NERR_PARSE, "Misformed attribute specification: %s", *str);
	}
	s++;
	v = buf.buf;
        v_l = buf.len;
      }
      else
      {
	v = s;
	while (*s && *s != ' ' && *s != ',' && *s != ']') s++;
	if (*s == '\0')
	{
	  _dealloc_hdf_attr(attr);
	  return nerr_raise(NERR_PARSE, "Misformed attribute specification: %s", *str);
	}
        v_l = s-v;
      }
    }
    else
    {
      v = "1";
    }
    ha = (HDF_ATTR*) calloc (1, sizeof(HDF_ATTR));
    if (ha == NULL)
    {
      _dealloc_hdf_attr(attr);
      string_clear(&buf);
      return nerr_raise(NERR_NOMEM, "Unable to load attributes: %s", s);
    }
    if (*attr == NULL) *attr = ha;
    ha->key = _strndup(k, k_l);
    if (v)
      ha->value = _strndup(v, v_l);
    else
      ha->value = strdup("");
    if (ha->key == NULL || ha->value == NULL)
    {
      _dealloc_hdf_attr(attr);
      string_clear(&buf);
      return nerr_raise(NERR_NOMEM, "Unable to load attributes: %s", s);
    }
    if (hal != NULL) hal->next = ha;
    hal = ha;
    string_clear(&buf);
    SKIPWS(s);
    if (*s == ',')
    {
      s++;
      SKIPWS(s);
    }
  }
  if (*s == '\0')
  {
    _dealloc_hdf_attr(attr);
    return nerr_raise(NERR_PARSE, "Misformed attribute specification: %s", *str);
  }
  *str = s+1;
  return STATUS_OK;
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
  HDF_ATTR *attr = NULL;

  while (**str != '\0')
  {
    /* Reset string length, but don't free the reserved buffer */
    line->len = 0;
    err = _copy_line_advance(str, line);
    if (err) return nerr_pass(err);
    attr = NULL;
    (*lineno)++;
    s = line->buf;
    SKIPWS(s);
    if ((!strncmp(s, "#include ", 9) || !strncmp(s, "-include ", 9)) && include_handle != INCLUDE_IGNORE)
    {
      int required = !strncmp(s, "#include ", 9);
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
        char fullpath[PATH_MAX];
        if (name[0] != '/') {
          memset(fullpath, 0, PATH_MAX);

          char *p = strrchr(path, '/');
          if (p == NULL) {
            char pwd[PATH_MAX];
            memset(pwd, 0, PATH_MAX);
            getcwd(pwd, PATH_MAX);
            snprintf(fullpath, PATH_MAX, "%s/%s", pwd, name);
          } else {
            int dir_len = p - path + 1;
            snprintf(fullpath, PATH_MAX, "%s", path);
            snprintf(fullpath + dir_len, PATH_MAX - dir_len, "%s", name);
          }
          name = fullpath;
        }
        err = hdf_read_file_internal(hdf, name, include_handle + 1);
        if (err != STATUS_OK && required)
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
      if (splice) s++;
      name = s;
      while (*s && (isalnum(*s) || *s == '_' || *s == '.' || *s == '*' || *s == '\\')) s++;
      SKIPWS(s);

      char num[256];
      static int counter = 0;
      char *p;
      int i = 0;
      for (p = name; p < s && i < 200; p++) {
        if (*p != '*') {
          num[i++] = *p;
        } else {
          i += snprintf(num + i, 256 - i, "%d", counter++);
          name = num;
        }
      }
      num[i] = '\0';

      if (s[0] == '[') /* attributes */
      {
	*s = '\0';
	name = neos_strip(name);
	s++;
	err = parse_attr(&s, &attr);
	if (err)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
	SKIPWS(s);
      }
      if (splice) {
        name = neos_strip(name);
        HDF *h = hdf_get_obj(hdf->top, name);
        if (h) {
          HDF *c = hdf_obj_child(h);
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
	err = _set_value (hdf, name, value, 1, 1, 0, attr, NULL);
	if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == ':' && s[1] == '=') /* copy */
      {
	*s = '\0';
	name = neos_strip(name);
	s+=2;
	value = neos_strip(s);
        HDF *h = hdf_get_obj(hdf->top, value);
        if (!h)
        {
	  err = nerr_raise(NERR_PARSE,
                           "[%s:%d] Failed to copy a node that is not loaded "
                           "yet: %s", path, *lineno, value);
          return err;
        }
        err = hdf_copy(hdf, name, h);
	if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == '!' && s[1] == '=') /* exec */
      {
	*s = '\0';
	name = neos_strip(name);
	s+=2;
	value = neos_strip(s);

        FILE *f = popen(value, "r");
	if (f == NULL)
        {
	  err = nerr_raise(NERR_PARSE,
                           "[%s:%d] Failed to exec specified command: %s",
                           path, *lineno, line->buf);
          return err;
        }
        char *content = _read_file(f);
        fclose(f);
        int len = strlen(content);
        if (len > 0 && content[len - 1] == '\n') {
          content[len - 1] = '\0'; // remove \n artifact
        }
	err = _set_value (hdf, name, content, 1, 1, 0, attr, NULL);
        free(content);

	if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == ':') /* link */
      {
	*s = '\0';
	name = neos_strip(name);
	s++;
	value = neos_strip(s);
	err = _set_value (hdf, name, value, 1, 1, 1, attr, NULL);
	if (err != STATUS_OK)
        {
          return nerr_pass_ctx(err, "In file %s:%d", path, *lineno);
        }
      }
      else if (s[0] == '{') /* deeper */
      {
	*s = '\0';
	name = neos_strip(name);
	lower = hdf_get_obj (hdf, name);
	if (lower == NULL)
	{
	  err = _set_value (hdf, name, NULL, 1, 1, 0, attr, &lower);
	}
	else
	{
	  err = _set_value (lower, NULL, lower->value, 1, 1, 0, attr, NULL);
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
	err = _set_value (hdf, name, m, 0, 1, 0, attr, NULL);
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

NEOERR * hdf_read_string_ignore (HDF *hdf, const char *str, int ignore)
{
  NEOERR *err;
  int lineno = 0;
  NEOSTRING line;
  string_init(&line);
  err = _hdf_read_string(hdf, &str, &line, "<string>", &lineno,
                         (ignore ? INCLUDE_IGNORE : INCLUDE_ERROR), 0);
  string_clear(&line);
  return nerr_pass(err);
}

/* The search path is part of the HDF by convention */
NEOERR* hdf_search_path (HDF *hdf, const char *path, char *full, int full_len)
{
  HDF *paths;
  struct stat s;

  for (paths = hdf_get_child (hdf, "hdf.loadpaths");
      paths;
      paths = hdf_obj_next (paths))
  {
    snprintf (full, full_len, "%s/%s", hdf_obj_value(paths), path);
    errno = 0;
    if (stat (full, &s) == -1)
    {
      if (errno != ENOENT)
	return nerr_raise_errno (NERR_SYSTEM, "Stat of %s failed", full);
    }
    else
    {
      return STATUS_OK;
    }
  }

  strncpy (full, path, full_len);
  if (stat (full, &s) == -1)
  {
    if (errno != ENOENT)
      return nerr_raise_errno (NERR_SYSTEM, "Stat of %s failed", full);
  }
  else return STATUS_OK;

  return nerr_raise (NERR_NOT_FOUND, "Path %s not found", path);
}

static NEOERR* hdf_read_file_internal (HDF *hdf, const char *path,
                                       int include_handle)
{
  NEOERR *err;
  int lineno = 0;
  char fpath[PATH_BUF_SIZE];
  char *ibuf = NULL;
  const char *ptr = NULL;
  HDF *top = hdf->top;
  NEOSTRING line;

  string_init(&line);

  if (path == NULL)
    return nerr_raise(NERR_ASSERT, "Can't read NULL file");

  if (top->fileload)
  {
    err = top->fileload(top->fileload_ctx, hdf, path, &ibuf);
  }
  else
  {
    if (path[0] != '/')
    {
      err = hdf_search_path (hdf, path, fpath, PATH_BUF_SIZE);
      if (err != STATUS_OK) return nerr_pass(err);
      path = fpath;
    }

    err = ne_load_file (path, &ibuf);
  }
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

void hdf_register_fileload(HDF *hdf, void *ctx, HDFFILELOAD fileload)
{
  if (hdf == NULL) return;
  if (hdf->top != NULL) hdf = hdf->top;
  hdf->fileload_ctx = ctx;
  hdf->fileload = fileload;
}

