#ifndef LITTLE_ENDIAN_INCLUDED
#define LITTLE_ENDIAN_INCLUDED
/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/little_endian.h
  Data in little-endian format.
*/

// IWYU pragma: private, include "my_byteorder.h"

#ifndef MY_BYTEORDER_INCLUDED
#error This file should never be #included directly; use my_byteorder.h.
#endif

#include <string.h>

#include "my_inttypes.h"

/*
  Since the pointers may be misaligned, we cannot do a straight read out of
  them. (It usually works-by-accident on x86 and on modern ARM, but not always
  when the compiler chooses unusual instruction for the read, e.g. LDM on ARM
  or most SIMD instructions on x86.) memcpy is safe and gets optimized to a
  single operation, since the size is small and constant.
*/

static inline int16 sint2korr(const uchar *A) {
  int16 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline int32 sint4korr(const uchar *A) {
  int32 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline uint16 uint2korr(const uchar *A) {
  uint16 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline uint32 uint4korr(const uchar *A) {
  uint32 ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline ulonglong uint8korr(const uchar *A) {
  ulonglong ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline longlong sint8korr(const uchar *A) {
  longlong ret;
  memcpy(&ret, A, sizeof(ret));
  return ret;
}

static inline void int2store(uchar *T, uint16 A) { memcpy(T, &A, sizeof(A)); }

static inline void int4store(uchar *T, uint32 A) { memcpy(T, &A, sizeof(A)); }

static inline void int7store(uchar *T, ulonglong A) { memcpy(T, &A, 7); }

static inline void int8store(uchar *T, ulonglong A) {
  memcpy(T, &A, sizeof(A));
}

static inline float float4get(const uchar *M) {
  float V;
  memcpy(&V, (M), sizeof(float));
  return V;
}

static inline void float4store(uchar *V, float M) {
  memcpy(V, (&M), sizeof(float));
}

static inline double float8get(const uchar *M) {
  double V;
  memcpy(&V, M, sizeof(double));
  return V;
}

static inline void float8store(uchar *V, double M) {
  memcpy(V, &M, sizeof(double));
}

#endif /* LITTLE_ENDIAN_INCLUDED */
