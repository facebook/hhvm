#ifndef MYISAMPACK_INCLUDED
#define MYISAMPACK_INCLUDED

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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/myisampack.h
  Storing of values in high byte first order.

  Integer keys and file pointers are stored with high byte first to get
  better compression.
*/

#include "my_config.h"

#include <sys/types.h>

#include "my_inttypes.h"

/* these two are for uniformity */

static inline int8 mi_sint1korr(const uchar *A) { return *A; }

static inline uint8 mi_uint1korr(const uchar *A) { return *A; }

static inline int16 mi_sint2korr(const uchar *A) {
  return (int16)((uint32)(A[1]) + ((uint32)(A[0]) << 8));
}

static inline int32 mi_sint3korr(const uchar *A) {
  return (int32)((A[0] & 128) ? ((255U << 24) | ((uint32)(A[0]) << 16) |
                                 ((uint32)(A[1]) << 8) | ((uint32)A[2]))
                              : (((uint32)(A[0]) << 16) |
                                 ((uint32)(A[1]) << 8) | ((uint32)(A[2]))));
}

static inline int32 mi_sint4korr(const uchar *A) {
  return (int32)((uint32)(A[3]) + ((uint32)(A[2]) << 8) +
                 ((uint32)(A[1]) << 16) + ((uint32)(A[0]) << 24));
}

static inline uint16 mi_uint2korr(const uchar *A) {
  return (uint16)((uint16)A[1]) + ((uint16)A[0] << 8);
}

static inline uint32 mi_uint3korr(const uchar *A) {
  return (uint32)((uint32)A[2] + ((uint32)A[1] << 8) + ((uint32)A[0] << 16));
}

static inline uint32 mi_uint4korr(const uchar *A) {
  return (uint32)((uint32)A[3] + ((uint32)A[2] << 8) + ((uint32)A[1] << 16) +
                  ((uint32)A[0] << 24));
}

static inline ulonglong mi_uint5korr(const uchar *A) {
  return (ulonglong)((uint32)A[4] + ((uint32)A[3] << 8) + ((uint32)A[2] << 16) +
                     ((uint32)A[1] << 24)) +
         ((ulonglong)A[0] << 32);
}

static inline ulonglong mi_uint6korr(const uchar *A) {
  return (ulonglong)((uint32)A[5] + ((uint32)A[4] << 8) + ((uint32)A[3] << 16) +
                     ((uint32)A[2] << 24)) +
         (((ulonglong)((uint32)A[1] + ((uint32)A[0] << 8))) << 32);
}

static inline ulonglong mi_uint7korr(const uchar *A) {
  return (ulonglong)((uint32)A[6] + ((uint32)A[5] << 8) + ((uint32)A[4] << 16) +
                     ((uint32)A[3] << 24)) +
         (((ulonglong)((uint32)A[2] + ((uint32)A[1] << 8) +
                       ((uint32)A[0] << 16)))
          << 32);
}

static inline ulonglong mi_uint8korr(const uchar *A) {
  return (ulonglong)((uint32)A[7] + ((uint32)A[6] << 8) + ((uint32)A[5] << 16) +
                     ((uint32)A[4] << 24)) +
         (((ulonglong)((uint32)A[3] + ((uint32)A[2] << 8) +
                       ((uint32)A[1] << 16) + ((uint32)A[0] << 24)))
          << 32);
}

static inline longlong mi_sint8korr(const uchar *A) {
  return (longlong)mi_uint8korr(A);
}

/* This one is for uniformity */
#define mi_int1store(T, A) *((uchar *)(T)) = (uchar)(A)

#define mi_int2store(T, A)                      \
  {                                             \
    uint def_temp = (uint)(A);                  \
    ((uchar *)(T))[1] = (uchar)(def_temp);      \
    ((uchar *)(T))[0] = (uchar)(def_temp >> 8); \
  }
#define mi_int3store(T, A)                       \
  { /*lint -save -e734 */                        \
    ulong def_temp = (ulong)(A);                 \
    ((uchar *)(T))[2] = (uchar)(def_temp);       \
    ((uchar *)(T))[1] = (uchar)(def_temp >> 8);  \
    ((uchar *)(T))[0] = (uchar)(def_temp >> 16); \
                              /*lint -restore */}
#define mi_int4store(T, A)                       \
  {                                              \
    ulong def_temp = (ulong)(A);                 \
    ((uchar *)(T))[3] = (uchar)(def_temp);       \
    ((uchar *)(T))[2] = (uchar)(def_temp >> 8);  \
    ((uchar *)(T))[1] = (uchar)(def_temp >> 16); \
    ((uchar *)(T))[0] = (uchar)(def_temp >> 24); \
  }
#define mi_int5store(T, A)                                       \
  {                                                              \
    ulong def_temp = (ulong)(A), def_temp2 = (ulong)((A) >> 32); \
    ((uchar *)(T))[4] = (uchar)(def_temp);                       \
    ((uchar *)(T))[3] = (uchar)(def_temp >> 8);                  \
    ((uchar *)(T))[2] = (uchar)(def_temp >> 16);                 \
    ((uchar *)(T))[1] = (uchar)(def_temp >> 24);                 \
    ((uchar *)(T))[0] = (uchar)(def_temp2);                      \
  }
#define mi_int6store(T, A)                                       \
  {                                                              \
    ulong def_temp = (ulong)(A), def_temp2 = (ulong)((A) >> 32); \
    ((uchar *)(T))[5] = (uchar)(def_temp);                       \
    ((uchar *)(T))[4] = (uchar)(def_temp >> 8);                  \
    ((uchar *)(T))[3] = (uchar)(def_temp >> 16);                 \
    ((uchar *)(T))[2] = (uchar)(def_temp >> 24);                 \
    ((uchar *)(T))[1] = (uchar)(def_temp2);                      \
    ((uchar *)(T))[0] = (uchar)(def_temp2 >> 8);                 \
  }
#define mi_int7store(T, A)                                       \
  {                                                              \
    ulong def_temp = (ulong)(A), def_temp2 = (ulong)((A) >> 32); \
    ((uchar *)(T))[6] = (uchar)(def_temp);                       \
    ((uchar *)(T))[5] = (uchar)(def_temp >> 8);                  \
    ((uchar *)(T))[4] = (uchar)(def_temp >> 16);                 \
    ((uchar *)(T))[3] = (uchar)(def_temp >> 24);                 \
    ((uchar *)(T))[2] = (uchar)(def_temp2);                      \
    ((uchar *)(T))[1] = (uchar)(def_temp2 >> 8);                 \
    ((uchar *)(T))[0] = (uchar)(def_temp2 >> 16);                \
  }
#define mi_int8store(T, A)                                        \
  {                                                               \
    ulong def_temp3 = (ulong)(A), def_temp4 = (ulong)((A) >> 32); \
    mi_int4store((uchar *)(T) + 0, def_temp4);                    \
    mi_int4store((uchar *)(T) + 4, def_temp3);                    \
  }

#ifdef WORDS_BIGENDIAN

#define mi_float4store(T, A)              \
  {                                       \
    ((uchar *)(T))[0] = ((uchar *)&A)[0]; \
    ((uchar *)(T))[1] = ((uchar *)&A)[1]; \
    ((uchar *)(T))[2] = ((uchar *)&A)[2]; \
    ((uchar *)(T))[3] = ((uchar *)&A)[3]; \
  }

static inline float mi_float4get(const uchar *M) {
  float def_temp;
  ((uchar *)&def_temp)[0] = M[0];
  ((uchar *)&def_temp)[1] = M[1];
  ((uchar *)&def_temp)[2] = M[2];
  ((uchar *)&def_temp)[3] = M[3];
  return def_temp;
}

#define mi_float8store(T, V)              \
  {                                       \
    ((uchar *)(T))[0] = ((uchar *)&V)[0]; \
    ((uchar *)(T))[1] = ((uchar *)&V)[1]; \
    ((uchar *)(T))[2] = ((uchar *)&V)[2]; \
    ((uchar *)(T))[3] = ((uchar *)&V)[3]; \
    ((uchar *)(T))[4] = ((uchar *)&V)[4]; \
    ((uchar *)(T))[5] = ((uchar *)&V)[5]; \
    ((uchar *)(T))[6] = ((uchar *)&V)[6]; \
    ((uchar *)(T))[7] = ((uchar *)&V)[7]; \
  }

static inline double mi_float8get(const uchar *M) {
  double def_temp;
  ((uchar *)&def_temp)[0] = M[0];
  ((uchar *)&def_temp)[1] = M[1];
  ((uchar *)&def_temp)[2] = M[2];
  ((uchar *)&def_temp)[3] = M[3];
  ((uchar *)&def_temp)[4] = M[4];
  ((uchar *)&def_temp)[5] = M[5];
  ((uchar *)&def_temp)[6] = M[6];
  ((uchar *)&def_temp)[7] = M[7];
  return def_temp;
}
#else

#define mi_float4store(T, A)              \
  {                                       \
    ((uchar *)(T))[0] = ((uchar *)&A)[3]; \
    ((uchar *)(T))[1] = ((uchar *)&A)[2]; \
    ((uchar *)(T))[2] = ((uchar *)&A)[1]; \
    ((uchar *)(T))[3] = ((uchar *)&A)[0]; \
  }

static inline float mi_float4get(const uchar *M) {
  float def_temp;
  ((uchar *)&def_temp)[0] = M[3];
  ((uchar *)&def_temp)[1] = M[2];
  ((uchar *)&def_temp)[2] = M[1];
  ((uchar *)&def_temp)[3] = M[0];
  return def_temp;
}

#if defined(__FLOAT_WORD_ORDER) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN)
#define mi_float8store(T, V)              \
  {                                       \
    ((uchar *)(T))[0] = ((uchar *)&V)[3]; \
    ((uchar *)(T))[1] = ((uchar *)&V)[2]; \
    ((uchar *)(T))[2] = ((uchar *)&V)[1]; \
    ((uchar *)(T))[3] = ((uchar *)&V)[0]; \
    ((uchar *)(T))[4] = ((uchar *)&V)[7]; \
    ((uchar *)(T))[5] = ((uchar *)&V)[6]; \
    ((uchar *)(T))[6] = ((uchar *)&V)[5]; \
    ((uchar *)(T))[7] = ((uchar *)&V)[4]; \
  }

static inline double mi_float8get(const uchar *M) {
  double def_temp;
  ((uchar *)&def_temp)[0] = M[3];
  ((uchar *)&def_temp)[1] = M[2];
  ((uchar *)&def_temp)[2] = M[1];
  ((uchar *)&def_temp)[3] = M[0];
  ((uchar *)&def_temp)[4] = M[7];
  ((uchar *)&def_temp)[5] = M[6];
  ((uchar *)&def_temp)[6] = M[5];
  ((uchar *)&def_temp)[7] = M[4];
  return def_temp;
}

#else
#define mi_float8store(T, V)              \
  {                                       \
    ((uchar *)(T))[0] = ((uchar *)&V)[7]; \
    ((uchar *)(T))[1] = ((uchar *)&V)[6]; \
    ((uchar *)(T))[2] = ((uchar *)&V)[5]; \
    ((uchar *)(T))[3] = ((uchar *)&V)[4]; \
    ((uchar *)(T))[4] = ((uchar *)&V)[3]; \
    ((uchar *)(T))[5] = ((uchar *)&V)[2]; \
    ((uchar *)(T))[6] = ((uchar *)&V)[1]; \
    ((uchar *)(T))[7] = ((uchar *)&V)[0]; \
  }

static inline double mi_float8get(const uchar *M) {
  double def_temp;
  ((uchar *)&def_temp)[0] = M[7];
  ((uchar *)&def_temp)[1] = M[6];
  ((uchar *)&def_temp)[2] = M[5];
  ((uchar *)&def_temp)[3] = M[4];
  ((uchar *)&def_temp)[4] = M[3];
  ((uchar *)&def_temp)[5] = M[2];
  ((uchar *)&def_temp)[6] = M[1];
  ((uchar *)&def_temp)[7] = M[0];
  return def_temp;
}

#endif /* __FLOAT_WORD_ORDER */
#endif /* WORDS_BIGENDIAN */

#define mi_rowstore(T, A) mi_int8store(T, A)
#define mi_rowkorr(T) mi_uint8korr(T)

#define mi_sizestore(T, A) mi_int8store(T, A)
#define mi_sizekorr(T) mi_uint8korr(T)

#endif /* MYISAMPACK_INCLUDED */
