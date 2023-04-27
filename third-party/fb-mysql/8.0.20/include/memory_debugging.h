/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MEMORY_DEBUGGING_INCLUDED
#define MEMORY_DEBUGGING_INCLUDED

/**
  @file memory_debugging.h

  Various macros useful for communicating with memory debuggers,
  such as Valgrind.
*/

#include <string.h>

#ifdef HAVE_VALGRIND
#include <valgrind/valgrind.h>

#define MEM_MALLOCLIKE_BLOCK(p1, p2, p3, p4) \
  VALGRIND_MALLOCLIKE_BLOCK(p1, p2, p3, p4)
#define MEM_FREELIKE_BLOCK(p1, p2) VALGRIND_FREELIKE_BLOCK(p1, p2)
#include <valgrind/memcheck.h>

#define MEM_UNDEFINED(a, len) VALGRIND_MAKE_MEM_UNDEFINED(a, len)
#define MEM_DEFINED_IF_ADDRESSABLE(a, len) \
  VALGRIND_MAKE_MEM_DEFINED_IF_ADDRESSABLE(a, len)
#define MEM_NOACCESS(a, len) VALGRIND_MAKE_MEM_NOACCESS(a, len)
#define MEM_CHECK_ADDRESSABLE(a, len) VALGRIND_CHECK_MEM_IS_ADDRESSABLE(a, len)

#else /* HAVE_VALGRIND */

#define MEM_MALLOCLIKE_BLOCK(p1, p2, p3, p4) \
  do {                                       \
  } while (0)
#define MEM_FREELIKE_BLOCK(p1, p2) \
  do {                             \
  } while (0)
#define MEM_UNDEFINED(a, len) ((void)0)
#define MEM_DEFINED_IF_ADDRESSABLE(a, len) ((void)0)
#define MEM_NOACCESS(a, len) ((void)0)
#define MEM_CHECK_ADDRESSABLE(a, len) ((void)0)

#endif

#if !defined(DBUG_OFF) || defined(HAVE_VALGRIND)

/**
  Put bad content in memory to be sure it will segfault if dereferenced.
  With Valgrind, verify that memory is addressable, and mark it undefined.
*/
inline void TRASH(void *ptr, size_t length) {
  MEM_CHECK_ADDRESSABLE(ptr, length);
  memset(ptr, 0x8F, length);
  MEM_UNDEFINED(ptr, length);
}

#else

inline void TRASH(void *, size_t) {}

#endif

#endif  // MEMORY_DEBUGGING_INCLUDED
