/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/lf_dynarray.cc
  Analog of DYNAMIC_ARRAY that never reallocs
  (so no pointer into the array may ever become invalid).

  Memory is allocated in non-contiguous chunks.
  This data structure is not space efficient for sparse arrays.

  Every element is aligned to sizeof(element) boundary
  (to avoid false sharing if element is big enough).

  LF_DYNARRAY is a recursive structure. On the zero level
  LF_DYNARRAY::level[0] it's an array of LF_DYNARRAY_LEVEL_LENGTH elements,
  on the first level it's an array of LF_DYNARRAY_LEVEL_LENGTH pointers
  to arrays of elements, on the second level it's an array of pointers
  to arrays of pointers to arrays of elements. And so on.

  With four levels the number of elements is limited to 4311810304
  (but as in all functions index is uint, the real limit is 2^32-1)

  Actually, it's wait-free, not lock-free ;-)
*/

#include <string.h>
#include <sys/types.h>

#include <algorithm>

#include "lf.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"

void lf_dynarray_init(LF_DYNARRAY *array, uint element_size) {
  std::fill(begin(array->level), end(array->level), nullptr);
  array->size_of_element = element_size;
}

static void recursive_free(std::atomic<void *> *alloc, int level) {
  if (!alloc) {
    return;
  }

  if (level) {
    int i;
    for (i = 0; i < LF_DYNARRAY_LEVEL_LENGTH; i++)
      recursive_free(static_cast<std::atomic<void *> *>(alloc[i].load()),
                     level - 1);
    my_free(alloc);
  } else {
    my_free(alloc[-1]);
  }
}

void lf_dynarray_destroy(LF_DYNARRAY *array) {
  int i;
  for (i = 0; i < LF_DYNARRAY_LEVELS; i++)
    recursive_free(static_cast<std::atomic<void *> *>(array->level[i].load()),
                   i);
}

static const ulong dynarray_idxes_in_prev_levels[LF_DYNARRAY_LEVELS] = {
    0, /* +1 here to to avoid -1's below */
    LF_DYNARRAY_LEVEL_LENGTH,
    LF_DYNARRAY_LEVEL_LENGTH *LF_DYNARRAY_LEVEL_LENGTH +
        LF_DYNARRAY_LEVEL_LENGTH,
    LF_DYNARRAY_LEVEL_LENGTH *LF_DYNARRAY_LEVEL_LENGTH
            *LF_DYNARRAY_LEVEL_LENGTH +
        LF_DYNARRAY_LEVEL_LENGTH *LF_DYNARRAY_LEVEL_LENGTH +
        LF_DYNARRAY_LEVEL_LENGTH};

static const ulong dynarray_idxes_in_prev_level[LF_DYNARRAY_LEVELS] = {
    0, /* +1 here to to avoid -1's below */
    LF_DYNARRAY_LEVEL_LENGTH,
    LF_DYNARRAY_LEVEL_LENGTH *LF_DYNARRAY_LEVEL_LENGTH,
    LF_DYNARRAY_LEVEL_LENGTH *LF_DYNARRAY_LEVEL_LENGTH
        *LF_DYNARRAY_LEVEL_LENGTH,
};

/*
  Returns a valid lvalue pointer to the element number 'idx'.
  Allocates memory if necessary.
*/
void *lf_dynarray_lvalue(LF_DYNARRAY *array, uint idx) {
  void *ptr;
  int i;

  for (i = LF_DYNARRAY_LEVELS - 1; idx < dynarray_idxes_in_prev_levels[i]; i--)
    /* no-op */;
  std::atomic<void *> *ptr_ptr = &array->level[i];
  idx -= dynarray_idxes_in_prev_levels[i];
  for (; i > 0; i--) {
    if (!(ptr = *ptr_ptr)) {
      void *alloc = my_malloc(key_memory_lf_dynarray,
                              LF_DYNARRAY_LEVEL_LENGTH * sizeof(void *),
                              MYF(MY_WME | MY_ZEROFILL));
      if (unlikely(!alloc)) {
        return (nullptr);
      }
      if (atomic_compare_exchange_strong(ptr_ptr, &ptr, alloc)) {
        ptr = alloc;
      } else {
        my_free(alloc);
      }
    }
    ptr_ptr =
        ((std::atomic<void *> *)ptr) + idx / dynarray_idxes_in_prev_level[i];
    idx %= dynarray_idxes_in_prev_level[i];
  }
  if (!(ptr = *ptr_ptr)) {
    uchar *alloc, *data;
    alloc = static_cast<uchar *>(
        my_malloc(key_memory_lf_dynarray,
                  LF_DYNARRAY_LEVEL_LENGTH * array->size_of_element +
                      std::max<uint>(array->size_of_element, sizeof(void *)),
                  MYF(MY_WME | MY_ZEROFILL)));
    if (unlikely(!alloc)) {
      return (nullptr);
    }
    /* reserve the space for free() address */
    data = alloc + sizeof(void *);
    {
      /* alignment */
      intptr mod = ((intptr)data) % array->size_of_element;
      if (mod) {
        data += array->size_of_element - mod;
      }
    }
    ((void **)data)[-1] = alloc; /* free() will need the original pointer */
    if (atomic_compare_exchange_strong(ptr_ptr, &ptr,
                                       static_cast<void *>(data))) {
      ptr = data;
    } else {
      my_free(alloc);
    }
  }
  return ((uchar *)ptr) + array->size_of_element * idx;
}

/*
  Returns a pointer to the element number 'idx'
  or NULL if an element does not exists
*/
void *lf_dynarray_value(LF_DYNARRAY *array, uint idx) {
  void *ptr;
  int i;

  for (i = LF_DYNARRAY_LEVELS - 1; idx < dynarray_idxes_in_prev_levels[i]; i--)
    /* no-op */;
  std::atomic<void *> *ptr_ptr = &array->level[i];
  idx -= dynarray_idxes_in_prev_levels[i];
  for (; i > 0; i--) {
    if (!(ptr = *ptr_ptr)) {
      return (nullptr);
    }
    ptr_ptr =
        ((std::atomic<void *> *)ptr) + idx / dynarray_idxes_in_prev_level[i];
    idx %= dynarray_idxes_in_prev_level[i];
  }
  if (!(ptr = *ptr_ptr)) {
    return (nullptr);
  }
  return ((uchar *)ptr) + array->size_of_element * idx;
}

static int recursive_iterate(LF_DYNARRAY *array, void *ptr, int level,
                             lf_dynarray_func func, void *arg) {
  int res, i;
  if (!ptr) {
    return 0;
  }
  if (!level) {
    return func(ptr, arg);
  }
  for (i = 0; i < LF_DYNARRAY_LEVEL_LENGTH; i++)
    if ((res = recursive_iterate(array, ((void **)ptr)[i], level - 1, func,
                                 arg))) {
      return res;
    }
  return 0;
}

/*
  Calls func(array, arg) on every array of LF_DYNARRAY_LEVEL_LENGTH elements
  in lf_dynarray.

  DESCRIPTION
    lf_dynarray consists of a set of arrays, LF_DYNARRAY_LEVEL_LENGTH elements
    each. lf_dynarray_iterate() calls user-supplied function on every array
    from the set. It is the fastest way to scan the array, faster than
      for (i=0; i < N; i++) { func(lf_dynarray_value(dynarray, i)); }

  NOTE
    if func() returns non-zero, the scan is aborted
*/
int lf_dynarray_iterate(LF_DYNARRAY *array, lf_dynarray_func func, void *arg) {
  int i, res;
  for (i = 0; i < LF_DYNARRAY_LEVELS; i++)
    if ((res = recursive_iterate(array, array->level[i], i, func, arg))) {
      return res;
    }
  return 0;
}
