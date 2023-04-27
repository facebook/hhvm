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
  @file mysys/mulalloc.cc
*/

#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"  // IWYU pragma: keep
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"

/*
  Malloc many pointers at the same time
  Only ptr1 can be free'd, and doing this will free all
  the memory allocated. ptr2, etc all point inside big allocated
  memory area.

  SYNOPSIS
    my_multi_malloc()
      myFlags              Flags
        ptr1, length1      Multiple arguments terminated by null ptr
        ptr2, length2      ...
        ...
        NULL
*/

void *my_multi_malloc(PSI_memory_key key, myf myFlags, ...) {
  va_list args;
  char **ptr, *start, *res;
  size_t tot_length, length;
  DBUG_TRACE;

  va_start(args, myFlags);
  tot_length = 0;
  while ((ptr = va_arg(args, char **))) {
    length = va_arg(args, uint);
    tot_length += ALIGN_SIZE(length);
  }
  va_end(args);

  if (!(start = (char *)my_malloc(key, tot_length, myFlags)))
    return nullptr; /* purecov: inspected */

  va_start(args, myFlags);
  res = start;
  while ((ptr = va_arg(args, char **))) {
    *ptr = res;
    length = va_arg(args, uint);
    res += ALIGN_SIZE(length);
  }
  va_end(args);
  return (void *)start;
}
