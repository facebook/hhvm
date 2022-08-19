/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_compare_h
#define _my_compare_h

/**
  @file include/my_compare.h
*/

#include <sys/types.h>

#include "m_ctype.h" /* CHARSET_INFO */
#include "my_inttypes.h"
#include "myisampack.h"

/*
  There is a hard limit for the maximum number of keys as there are only
  8 bits in the index file header for the number of keys in a table.
  This means that 0..255 keys can exist for a table. The idea of
  HA_MAX_POSSIBLE_KEY is to ensure that one can use myisamchk & tools on
  a MyISAM table for which one has more keys than MyISAM is normally
  compiled for. If you don't have this, you will get a core dump when
  running myisamchk compiled for 128 keys on a table with 255 keys.
*/

#define HA_MAX_POSSIBLE_KEY 255 /* For myisamchk */
/*
  The following defines can be increased if necessary.
  But beware the dependency of MI_MAX_POSSIBLE_KEY_BUFF and HA_MAX_KEY_LENGTH.
*/

#define HA_MAX_KEY_LENGTH 1000 /* Max length in bytes */
#define HA_MAX_KEY_SEG 16      /* Max segments for key */

#define HA_MAX_POSSIBLE_KEY_BUFF (HA_MAX_KEY_LENGTH + 24 + 6 + 6)
#define HA_MAX_KEY_BUFF (HA_MAX_KEY_LENGTH + HA_MAX_KEY_SEG * 6 + 8 + 8)

struct HA_KEYSEG /* Key-portion */
{
  const CHARSET_INFO *charset;
  uint32 start;    /* Start of key in record */
  uint32 null_pos; /* position to NULL indicator */
  uint16 bit_pos;  /* Position to bit part */
  uint16 flag;
  uint16 length; /* Keylength */
  uint16 language;
  uint8 type;               /* Type of key (for sort) */
  uint8 null_bit;           /* bitmask to test for NULL */
  uint8 bit_start, bit_end; /* if bit field */
  uint8 bit_length;         /* Length of bit part */
};

static inline uint get_key_length(const uchar **key) {
  if (**key != 255) return *(*key)++;
  uint length = mi_uint2korr((*key) + 1);
  (*key) += 3;
  return length;
}

static inline uint get_key_pack_length(const uchar **key, uint *length_pack) {
  *length_pack = (**key != 255) ? 1 : 3;
  return get_key_length(key);
}

#define store_key_length_inc(key, length) \
  {                                       \
    if ((length) < 255) {                 \
      *(key)++ = (length);                \
    } else {                              \
      *(key) = 255;                       \
      mi_int2store((key) + 1, (length));  \
      (key) += 3;                         \
    }                                     \
  }

#define size_to_store_key_length(length) ((length) < 255 ? 1 : 3)

#define get_rec_bits(bit_ptr, bit_ofs, bit_len)                          \
  (((((uint16)(bit_ptr)[1] << 8) | (uint16)(bit_ptr)[0]) >> (bit_ofs)) & \
   ((1 << (bit_len)) - 1))

#define set_rec_bits(bits, bit_ptr, bit_ofs, bit_len)                        \
  {                                                                          \
    (bit_ptr)[0] = ((bit_ptr)[0] & ~(((1 << (bit_len)) - 1) << (bit_ofs))) | \
                   ((bits) << (bit_ofs));                                    \
    if ((bit_ofs) + (bit_len) > 8)                                           \
      (bit_ptr)[1] =                                                         \
          ((bit_ptr)[1] & ~((1 << ((bit_len)-8 + (bit_ofs))) - 1)) |         \
          ((bits) >> (8 - (bit_ofs)));                                       \
  }

#define clr_rec_bits(bit_ptr, bit_ofs, bit_len) \
  set_rec_bits(0, bit_ptr, bit_ofs, bit_len)

extern int ha_compare_text(const CHARSET_INFO *, const uchar *, uint,
                           const uchar *, uint, bool);
extern int ha_key_cmp(const HA_KEYSEG *keyseg, const uchar *a, const uchar *b,
                      uint key_length, uint nextflag, uint *diff_pos);

/**
  Compare two numbers of the same type.
  @param val1 the first number
  @param val2 the second number
  @retval -1 if val1 is less than val2,
  @retval 0 if val1 is equal to val2,
  @retval 1 if val1 is greater than val2
*/
template <class T>
int compare_numbers(T val1, T val2) {
  return val1 < val2 ? -1 : (val1 == val2 ? 0 : 1);
}

#endif /* _my_compare_h */
