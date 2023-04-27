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
  @file include/big_endian.h

  Endianness-independent definitions (little_endian.h contains optimized
  versions if you know you are on a little-endian platform).
*/

// IWYU pragma: private, include "my_byteorder.h"

#ifndef MY_BYTEORDER_INCLUDED
#error This file should never be #included directly; use my_byteorder.h.
#endif

#include <string.h>

#include "my_inttypes.h"

static inline int16 sint2korr(const uchar *A) {
  return (int16)(((int16)(A[0])) + ((int16)(A[1]) << 8));
}

static inline int32 sint4korr(const uchar *A) {
  return (int32)(((int32)(A[0])) + (((int32)(A[1]) << 8)) +
                 (((int32)(A[2]) << 16)) + (((int32)(A[3]) << 24)));
}

static inline uint16 uint2korr(const uchar *A) {
  return (uint16)(((uint16)(A[0])) + ((uint16)(A[1]) << 8));
}

static inline uint32 uint4korr(const uchar *A) {
  return (uint32)(((uint32)(A[0])) + (((uint32)(A[1])) << 8) +
                  (((uint32)(A[2])) << 16) + (((uint32)(A[3])) << 24));
}

static inline ulonglong uint8korr(const uchar *A) {
  return ((ulonglong)(((uint32)(A[0])) + (((uint32)(A[1])) << 8) +
                      (((uint32)(A[2])) << 16) + (((uint32)(A[3])) << 24)) +
          (((ulonglong)(((uint32)(A[4])) + (((uint32)(A[5])) << 8) +
                        (((uint32)(A[6])) << 16) + (((uint32)(A[7])) << 24)))
           << 32));
}

static inline longlong sint8korr(const uchar *A) {
  return (longlong)uint8korr(A);
}

static inline void int2store(uchar *T, uint16 A) {
  uint def_temp = A;
  *(T) = (uchar)(def_temp);
  *(T + 1) = (uchar)(def_temp >> 8);
}

static inline void int4store(uchar *T, uint32 A) {
  *(T) = (uchar)(A);
  *(T + 1) = (uchar)(A >> 8);
  *(T + 2) = (uchar)(A >> 16);
  *(T + 3) = (uchar)(A >> 24);
}

static inline void int7store(uchar *T, ulonglong A) {
  *(T) = (uchar)(A);
  *(T + 1) = (uchar)(A >> 8);
  *(T + 2) = (uchar)(A >> 16);
  *(T + 3) = (uchar)(A >> 24);
  *(T + 4) = (uchar)(A >> 32);
  *(T + 5) = (uchar)(A >> 40);
  *(T + 6) = (uchar)(A >> 48);
}

static inline void int8store(uchar *T, ulonglong A) {
  uint def_temp = (uint)A, def_temp2 = (uint)(A >> 32);
  int4store(T, def_temp);
  int4store(T + 4, def_temp2);
}

/*
  Data in big-endian format.
*/
static inline void float4store(uchar *T, float A) {
  *(T) = ((uchar *)&A)[3];
  *((T) + 1) = (char)((uchar *)&A)[2];
  *((T) + 2) = (char)((uchar *)&A)[1];
  *((T) + 3) = (char)((uchar *)&A)[0];
}

static inline float float4get(const uchar *M) {
  float def_temp;
  ((uchar *)&def_temp)[0] = (M)[3];
  ((uchar *)&def_temp)[1] = (M)[2];
  ((uchar *)&def_temp)[2] = (M)[1];
  ((uchar *)&def_temp)[3] = (M)[0];
  return def_temp;
}

static inline void float8store(uchar *T, double V) {
  *(T) = ((uchar *)&V)[7];
  *((T) + 1) = (char)((uchar *)&V)[6];
  *((T) + 2) = (char)((uchar *)&V)[5];
  *((T) + 3) = (char)((uchar *)&V)[4];
  *((T) + 4) = (char)((uchar *)&V)[3];
  *((T) + 5) = (char)((uchar *)&V)[2];
  *((T) + 6) = (char)((uchar *)&V)[1];
  *((T) + 7) = (char)((uchar *)&V)[0];
}

static inline double float8get(const uchar *M) {
  double def_temp;
  ((uchar *)&def_temp)[0] = (M)[7];
  ((uchar *)&def_temp)[1] = (M)[6];
  ((uchar *)&def_temp)[2] = (M)[5];
  ((uchar *)&def_temp)[3] = (M)[4];
  ((uchar *)&def_temp)[4] = (M)[3];
  ((uchar *)&def_temp)[5] = (M)[2];
  ((uchar *)&def_temp)[6] = (M)[1];
  ((uchar *)&def_temp)[7] = (M)[0];
  return def_temp;
}
