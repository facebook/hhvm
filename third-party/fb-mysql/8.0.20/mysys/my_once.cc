/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_once.cc
*/

/* Not MT-SAFE */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysys/my_static.h"
#include "mysys_err.h"

/*
  Alloc for things we don't nend to free run-time (that only
  should be free'd on exit)

  SYNOPSIS
    my_once_alloc()
      Size
      MyFlags

  NOTES
    No DBUG_TRACE... here to get smaller dbug-startup
*/

void *my_once_alloc(size_t Size, myf MyFlags) {
  size_t get_size, max_left;
  uchar *point;
  USED_MEM *next;
  USED_MEM **prev;

  Size = ALIGN_SIZE(Size);
  prev = &my_once_root_block;
  max_left = 0;
  for (next = my_once_root_block; next && next->left < Size;
       next = next->next) {
    if (next->left > max_left) max_left = next->left;
    prev = &next->next;
  }
  if (!next) { /* Time to alloc new block */
    get_size = Size + ALIGN_SIZE(sizeof(USED_MEM));
    if (max_left * 4 < my_once_extra && get_size < my_once_extra)
      get_size = my_once_extra; /* Normal alloc */

    if ((next = (USED_MEM *)malloc(get_size)) == nullptr) {
      set_my_errno(errno);
      if (MyFlags & (MY_FAE + MY_WME))
        my_error(EE_OUTOFMEMORY, MYF(ME_FATALERROR), get_size);
      return ((uchar *)nullptr);
    }
    DBUG_PRINT("test", ("my_once_malloc %lu byte malloced", (ulong)get_size));
    next->next = nullptr;
    next->size = (uint)get_size;
    next->left = (uint)(get_size - ALIGN_SIZE(sizeof(USED_MEM)));
    *prev = next;
  }
  point = (uchar *)((char *)next + (next->size - next->left));
  next->left -= (uint)Size;

  if (MyFlags & MY_ZEROFILL) memset(point, 0, Size);
  return ((void *)point);
} /* my_once_alloc */

char *my_once_strdup(const char *src, myf myflags) {
  size_t len = strlen(src) + 1;
  uchar *dst = static_cast<uchar *>(my_once_alloc(len, myflags));
  if (dst) memcpy(dst, src, len);
  return (char *)dst;
}

void *my_once_memdup(const void *src, size_t len, myf myflags) {
  uchar *dst = static_cast<uchar *>(my_once_alloc(len, myflags));
  if (dst) memcpy(dst, src, len);
  return dst;
}

/*
  Deallocate everything that was allocated with my_once_alloc

  SYNOPSIS
    my_once_free()
*/

void my_once_free(void) {
  USED_MEM *next, *old;
  DBUG_TRACE;

  for (next = my_once_root_block; next;) {
    old = next;
    next = next->next;
    free((uchar *)old);
  }
  my_once_root_block = nullptr;
} /* my_once_free */
