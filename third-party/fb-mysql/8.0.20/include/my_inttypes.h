/*
   Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MY_INTTYPES_INCLUDED
#define MY_INTTYPES_INCLUDED

/**
  @file include/my_inttypes.h
  Some integer typedefs for easier portability.

  @deprecated Use <stdint.h> instead. Prefer int to sized types.
*/

#include "my_config.h"

#ifndef MYSQL_ABI_CHECK
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#endif

#ifdef _WIN32
#include <BaseTsd.h>
typedef unsigned int uint;
typedef unsigned short ushort;
#endif
#if !defined(HAVE_ULONG)
typedef unsigned long ulong; /* Short for unsigned long */
#endif

typedef unsigned char uchar; /* Short for unsigned char */

// Don't use these in new code; use [u]int64_t.
typedef long long int longlong;
#if !defined(HAVE_ULONGLONG)
typedef unsigned long long int ulonglong;
#endif

// Legacy typedefs. Prefer the standard intXX_t (or std::intXX_t) to these.
// Note that the Google C++ style guide says you should generally not use
// unsigned types unless you need defined wraparound semantics or store
// things like bitfields. Your default choice of type should be simply int.
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
#if !defined(HAVE_INT64)
typedef int64_t int64;
#endif
#if !defined(HAVE_UINT64)
typedef uint64_t uint64;
#endif
typedef intptr_t intptr;

typedef ulonglong my_off_t;
#define MY_FILEPOS_ERROR (~(my_off_t)0)

#define INT_MIN64 (~0x7FFFFFFFFFFFFFFFLL)
#define INT_MAX64 0x7FFFFFFFFFFFFFFFLL
#define INT_MIN32 (~0x7FFFFFFFL)
#define INT_MAX32 0x7FFFFFFFL
#define UINT_MAX32 0xFFFFFFFFL
#define INT_MIN24 (~0x007FFFFF)
#define INT_MAX24 0x007FFFFF
#define UINT_MAX24 0x00FFFFFF
#define INT_MIN16 (~0x7FFF)
#define INT_MAX16 0x7FFF
#define UINT_MAX16 0xFFFF
#define INT_MIN8 (~0x7F)
#define INT_MAX8 0x7F
#define UINT_MAX8 0xFF

#ifndef SIZE_T_MAX
#define SIZE_T_MAX (~((size_t)0))
#endif

typedef int myf; /* Type of MyFlags in my_funcs */

/* Macros for converting *constants* to the right type */
#define MYF(v) (myf)(v)

/* Length of decimal number represented by INT32. */
#define MY_INT32_NUM_DECIMAL_DIGITS 11U

/* Length of decimal number represented by INT64. */
#define MY_INT64_NUM_DECIMAL_DIGITS 21U

#ifdef _WIN32
typedef SSIZE_T ssize_t;
#endif

/*
  This doesn't really belong here, but it was the only reasonable place
  at the time.
*/
#ifdef _WIN32
typedef int sigset_t;
#endif

#endif  // MY_INTTYPES_INCLUDED
