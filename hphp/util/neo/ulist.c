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
#include <errno.h>

#include "neo_misc.h"
#include "neo_err.h"
#include "ulist.h"

#define ULIST_DEFAULT_SIZE 10

static NEOERR *check_resize (ULIST *ul, int size)
{
  if (size > ul->max)
  {
    void **new_items;
    int new_size = 0;

    new_size = ul->max*2;
    if (size > new_size)
    {
      new_size = size + ul->max;
    }

    new_items = (void **) realloc ((void *)(ul->items), new_size * sizeof(void *));
    if (new_items == NULL)
    {
      return nerr_raise(NERR_NOMEM, 
	  "Unable to resize ULIST to %d: Out of memory", new_size);
    }
    ul->items = new_items;
    ul->max = new_size;
  }

  return STATUS_OK;
}


NEOERR *uListInit(ULIST **ul, int size, int flags)
{
  ULIST *r_ul;

  *ul = NULL;
  if (size == 0)
  {
    size = ULIST_DEFAULT_SIZE;
  }

  r_ul = (ULIST *) calloc (1, sizeof (ULIST));
  if (r_ul == NULL)
  {
    return nerr_raise(NERR_NOMEM, "Unable to create ULIST: Out of memory");
  }
  r_ul->items = (void **) calloc (size, sizeof(void *));
  if (r_ul->items == NULL)
  {
    free (r_ul);
    return nerr_raise(NERR_NOMEM, "Unable to create ULIST: Out of memory");
  }

  r_ul->num = 0;
  r_ul->max = size;
  r_ul->flags = flags;
  *ul = r_ul;

  return STATUS_OK;
}

NEOERR *uListvInit(ULIST **ul, ...)
{
  NEOERR *err;
  va_list ap;
  void *it;

  err = uListInit (ul, 0, 0);
  if (err) return nerr_pass (err);

  va_start (ap, ul);

  it = va_arg (ap, void *);

  while (it)
  {
    err = uListAppend (*ul, it);
    if (err) 
    {
      uListDestroy(ul, 0);
      return nerr_pass (err);
    }
    it = va_arg (ap, void *);
  }
  return STATUS_OK;
}

NEOERR *uListAppend (ULIST *ul, void *data)
{
  NEOERR *r;

  r = check_resize (ul, ul->num + 1);
  if (r != STATUS_OK)
    return r;

  ul->items[ul->num] = data;
  ul->num++;

  return STATUS_OK;
}

NEOERR *uListPop (ULIST *ul, void **data)
{
  if (ul->num == 0)
    return nerr_raise(NERR_OUTOFRANGE, "uListPop: empty list");

  *data = ul->items[ul->num - 1];
  ul->num--;

  return STATUS_OK;
}

NEOERR *uListInsert (ULIST *ul, int x, void *data)
{
  void **start;
  NEOERR *r;

  if (x < 0)
    x = ul->num + x;

  if (x >= ul->num)
    return nerr_raise(NERR_OUTOFRANGE, "uListInsert: past end (%d > %d)", 
	x, ul->num);

  r = check_resize (ul, ul->num + 1);
  if (r != STATUS_OK)
    return r;

  start = &(ul->items[x]);
  memmove (start + 1, start, (ul->num - x) * sizeof(void *));
  ul->items[x] = data;
  ++ul->num;

  return STATUS_OK;
}

NEOERR *uListDelete (ULIST *ul, int x, void **data)
{
  void **start;

  if (x < 0)
    x = ul->num + x;

  if (x >= ul->num)
    return nerr_raise(NERR_OUTOFRANGE, "uListDelete: past end (%d > %d)", 
	x, ul->num);

  if (data != NULL)
    *data = ul->items[x];

  start = &(ul->items[x]);
  memmove (start, start+1, (ul->num - x - 1) * sizeof(void *));
  --ul->num;

  return STATUS_OK;
}

NEOERR *uListGet (ULIST *ul, int x, void **data)
{
  if (x < 0)
    x = ul->num + x;

  if (x >= ul->num)
    return nerr_raise(NERR_OUTOFRANGE, "uListGet: past end (%d > %d)", 
	x, ul->num);

  if (x < 0)
    return nerr_raise(NERR_OUTOFRANGE, "uListGet: past beginning (%d < 0)", x);

  *data = ul->items[x];

  return STATUS_OK;
}

NEOERR *uListSet (ULIST *ul, int x, void *data)
{
  if (x >= ul->num)
    return nerr_raise(NERR_OUTOFRANGE, "uListSet: past end (%d > %d)", 
	x, ul->num);

  ul->items[x] = data;

  return STATUS_OK;
}

NEOERR *uListReverse (ULIST *ul)
{
  int i;

  for (i = 0; i < ul->num/2; ++i) {
    void *tmp = ul->items[i];
    ul->items[i] = ul->items[ul->num-1-i];
    ul->items[ul->num-1-i] = tmp;
  }

  return STATUS_OK;
}

NEOERR *uListSort (ULIST *ul, int (*compareFunc)(const void *, const void*)) {
  qsort(ul->items, ul->num, sizeof(void *), compareFunc);
  return STATUS_OK;
}

void *uListSearch (ULIST *ul, const void *key, int
    (*compareFunc)(const void *, const void*)) {
  return bsearch(key, ul->items, ul->num, sizeof(void *), compareFunc);
}

void *uListIn (ULIST *ul, const void *key, int (*compareFunc)(const void *, const void*)) {
  int i;

  for (i = 0; i < ul->num; ++i) {
    if (!compareFunc(key, &ul->items[i])) {
      return &ul->items[i];
    }
  }
  return NULL;
}

int uListIndex (ULIST *ul, const void *key, int (*compareFunc)(const void *, const void*)) {
  void **p = uListIn(ul, key, compareFunc);
  return p ? (p - ul->items) : -1;
}



NEOERR *uListDestroy (ULIST **ul, int flags)
{
  if (flags & ULIST_FREE)
  {
    return uListDestroyFunc(ul, free);
  }
  else
  {
    return uListDestroyFunc(ul, NULL);
  }
}

NEOERR *uListDestroyFunc (ULIST **ul, void (*destroyFunc)(void *))
{
  ULIST *r_ul;

  r_ul = *ul;

  if (r_ul == NULL)
    return STATUS_OK;

  if (destroyFunc != NULL)
  {
    int x;
    for (x = 0; x < r_ul->num; x++)
    {
      (*destroyFunc)(r_ul->items[x]);
    }
  }
  free (r_ul->items);
  free (r_ul);
  *ul = NULL;

  return STATUS_OK;
}

int uListLength (ULIST *ul)
{
  if (ul == NULL) return 0;
  return ul->num;
}
