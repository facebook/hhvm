/*
   Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/my_bitmap.cc
  Handling of uchar arrays as large bitmaps.

  API limitations (or, rather asserted safety assumptions,
  to encourage correct programming)

    * the internal size is a set of 32 bit words
    * the number of bits specified in creation can be any number > 0
      a bitmap with zero bits can be created and initialized, but not used.

  TODO:
  Make assembler THREAD safe versions of these using test-and-set instructions

  Original version created by Sergei Golubchik 2001 - 2004.
  New version written and test program added and some changes to the interface
  was made by Mikael Ronström 2005, with assistance of Tomas Ulin and Mats
  Kindahl.
*/

#include <string.h>
#include <sys/types.h>
#include <algorithm>

#include "my_bit.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys/mysys_priv.h"

void create_last_word_mask(MY_BITMAP *map) {
  /* Get the number of used bits (1..8) in the last byte */
  unsigned int const used = 1U + ((map->n_bits - 1U) & 0x7U);

  /*
    Create a mask with the upper 'unused' bits set and the lower 'used'
    bits clear. The bits within each byte is stored in big-endian order.
   */
  unsigned char const mask = (~((1 << used) - 1)) & 255;

  /*
    The first bytes are to be set to zero since they represent real  bits
    in the bitvector. The last bytes are set to 0xFF since they  represent
    bytes not used by the bitvector. Finally the last byte contains  bits
    as set by the mask above.
  */
  unsigned char *ptr = (unsigned char *)&map->last_word_mask;

  /* Avoid out-of-bounds read/write if we have zero bits. */
  map->last_word_ptr =
      map->n_bits == 0 ? map->bitmap : map->bitmap + no_words_in_map(map) - 1;

  switch (no_bytes_in_map(map) & 3) {
    case 1:
      map->last_word_mask = ~0U;
      ptr[0] = mask;
      return;
    case 2:
      map->last_word_mask = ~0U;
      ptr[0] = 0;
      ptr[1] = mask;
      return;
    case 3:
      map->last_word_mask = 0U;
      ptr[2] = mask;
      ptr[3] = 0xFFU;
      return;
    case 0:
      map->last_word_mask = 0U;
      ptr[3] = mask;
      return;
  }
}

static inline uint get_first_set(uint32 value, uint word_pos) {
  uchar *byte_ptr = (uchar *)&value;
  uchar byte_value;
  uint byte_pos, bit_pos;

  for (byte_pos = 0; byte_pos < 4; byte_pos++, byte_ptr++) {
    byte_value = *byte_ptr;
    if (byte_value) {
      for (bit_pos = 0;; bit_pos++)
        if (byte_value & (1 << bit_pos))
          return (word_pos * 32) + (byte_pos * 8) + bit_pos;
    }
  }
  return MY_BIT_NONE;
}

static inline uint get_first_not_set(uint32 value, uint word_pos) {
  uchar *byte_ptr = (uchar *)&value;
  uchar byte_value;
  uint byte_pos, bit_pos;

  for (byte_pos = 0; byte_pos < 4; byte_pos++, byte_ptr++) {
    byte_value = *byte_ptr;
    if (byte_value != 0xFF) {
      for (bit_pos = 0;; bit_pos++)
        if (!(byte_value & (1 << bit_pos)))
          return (word_pos * 32) + (byte_pos * 8) + bit_pos;
    }
  }
  return MY_BIT_NONE;
}

bool bitmap_init(MY_BITMAP *map, my_bitmap_map *buf, uint n_bits) {
  DBUG_TRACE;
  if (!buf) {
    uint size_in_bytes = bitmap_buffer_size(n_bits);
    uint extra = 0;

    if (!(buf = (my_bitmap_map *)my_malloc(key_memory_MY_BITMAP_bitmap,
                                           size_in_bytes + extra, MYF(MY_WME))))
      return true;
  }

  map->bitmap = buf;
  map->n_bits = n_bits;
  create_last_word_mask(map);
  bitmap_clear_all(map);
  return false;
}

void bitmap_free(MY_BITMAP *map) {
  DBUG_TRACE;
  if (map->bitmap) {
    my_free(map->bitmap);
    map->bitmap = nullptr;
  }
}

/*
  test if bit already set and set it if it was not

  SYNOPSIS
    bitmap_test_and_set()
    MAP   bit map struct
    BIT   bit number

  RETURN
    0    bit was not set
    !=0  bit was set
*/

bool bitmap_test_and_set(MY_BITMAP *map, uint bitmap_bit) {
  uchar *value = ((uchar *)map->bitmap) + (bitmap_bit / 8);
  uchar bit = 1 << ((bitmap_bit)&7);
  uchar res = (*value) & bit;
  *value |= bit;
  return res;
}

uint bitmap_set_next(MY_BITMAP *map) {
  uint bit_found;
  DBUG_ASSERT(map->bitmap);
  if ((bit_found = bitmap_get_first(map)) != MY_BIT_NONE)
    bitmap_set_bit(map, bit_found);
  return bit_found;
}

/**
  Set the specified number of bits in the bitmap buffer.

  @param map                Bitmap
  @param prefix_size        Number of bits to be set
*/
void bitmap_set_prefix(MY_BITMAP *map, uint prefix_size) {
  uint prefix_bytes, prefix_bits, d;
  uchar *m = (uchar *)map->bitmap;

  DBUG_ASSERT(map->bitmap &&
              (prefix_size <= map->n_bits || prefix_size == (uint)~0));
  prefix_size = std::min(prefix_size, map->n_bits);
  if ((prefix_bytes = prefix_size / 8)) memset(m, 0xff, prefix_bytes);
  m += prefix_bytes;
  if ((prefix_bits = prefix_size & 7)) {
    *(m++) = (1 << prefix_bits) - 1;
    // As the prefix bits are set, lets count this byte too as a prefix byte.
    prefix_bytes++;
  }
  if ((d = no_bytes_in_map(map) - prefix_bytes)) memset(m, 0, d);
}

bool bitmap_is_prefix(const MY_BITMAP *map, uint prefix_size) {
  uint prefix_bits = prefix_size % 32;
  my_bitmap_map *word_ptr = map->bitmap, last_word;
  my_bitmap_map *end_prefix = word_ptr + prefix_size / 32;
  DBUG_ASSERT(word_ptr && prefix_size <= map->n_bits);

  /* 1: Words that should be filled with 1 */
  for (; word_ptr < end_prefix; word_ptr++)
    if (*word_ptr != 0xFFFFFFFF) return false;

  DBUG_ASSERT(map->n_bits > 0);
  last_word = *map->last_word_ptr & ~map->last_word_mask;

  /* 2: Word which contains the end of the prefix (if any) */
  if (prefix_bits) {
    if (word_ptr == map->last_word_ptr)
      return uint4korr((uchar *)&last_word) ==
             (uint32)((1 << prefix_bits) - 1U);
    else if (uint4korr((uchar *)word_ptr) != (uint32)((1 << prefix_bits) - 1U))
      return false;
    word_ptr++;
  }

  /* 3: Words that should be filled with 0 */
  for (; word_ptr < map->last_word_ptr; word_ptr++)
    if (*word_ptr != 0) return false;

  /*
    We can end up here in two situations:
    1) We went through the whole bitmap in step 1. This will happen if the
       whole bitmap is filled with 1 and prefix_size is a multiple of 32
       (i.e. the prefix does not end in the middle of a word).
       In this case word_ptr will be larger than map->last_word_ptr.
    2) We have gone through steps 1-3 and just need to check that also
       the last word is 0.
  */
  return word_ptr > map->last_word_ptr || last_word == 0;
}

bool bitmap_is_set_all(const MY_BITMAP *map) {
  my_bitmap_map *data_ptr = map->bitmap;
  my_bitmap_map *end = map->last_word_ptr;

  DBUG_ASSERT(map->n_bits > 0);
  for (; data_ptr < end; data_ptr++)
    if (*data_ptr != 0xFFFFFFFF) return false;
  if ((*map->last_word_ptr | map->last_word_mask) != 0xFFFFFFFF) return false;
  return true;
}

bool bitmap_is_clear_all(const MY_BITMAP *map) {
  my_bitmap_map *data_ptr = map->bitmap;
  my_bitmap_map *end = map->last_word_ptr;

  DBUG_ASSERT(map->n_bits > 0);
  for (; data_ptr < end; data_ptr++)
    if (*data_ptr) return false;
  if (*map->last_word_ptr & ~map->last_word_mask) return false;
  return true;
}

/* Return TRUE if map1 is a subset of map2 */

bool bitmap_is_subset(const MY_BITMAP *map1, const MY_BITMAP *map2) {
  my_bitmap_map *m1 = map1->bitmap, *m2 = map2->bitmap, *end;

  DBUG_ASSERT(map1->bitmap && map2->bitmap && map1->n_bits == map2->n_bits);

  end = map1->last_word_ptr;
  for (; m1 < end; m1++, m2++)
    if (*m1 & ~(*m2)) return false;

  DBUG_ASSERT(map1->n_bits > 0);
  DBUG_ASSERT(map2->n_bits > 0);

  if ((*map1->last_word_ptr & ~map1->last_word_mask) &
      ~(*map2->last_word_ptr & ~map2->last_word_mask))
    return false;
  return true;
}

/* True if bitmaps has any common bits */

bool bitmap_is_overlapping(const MY_BITMAP *map1, const MY_BITMAP *map2) {
  my_bitmap_map *m1 = map1->bitmap, *m2 = map2->bitmap, *end;

  DBUG_ASSERT(map1->bitmap && map2->bitmap && map1->n_bits == map2->n_bits);

  DBUG_ASSERT(map1->n_bits > 0);
  DBUG_ASSERT(map2->n_bits > 0);

  end = map1->last_word_ptr;
  for (; m1 < end; m1++, m2++)
    if (*m1 & *m2) return true;

  if ((*map1->last_word_ptr & ~map1->last_word_mask) &
      (*map2->last_word_ptr & ~map2->last_word_mask))
    return true;
  return false;
}

void bitmap_intersect(MY_BITMAP *to, const MY_BITMAP *from) {
  DBUG_ASSERT(to->bitmap && from->bitmap);

  uint to_length = no_words_in_map(to);
  uint from_length = no_words_in_map(from);
  uint min_length = std::min(to_length, from_length);

  // Clear bits in 'to' not set in 'from'
  for (uint i = 0; i < min_length; i++) to->bitmap[i] &= from->bitmap[i];

  if (to_length >= from_length)
    to->bitmap[from_length - 1] &= ~from->last_word_mask;

  // Clear bits in 'to' where no corresponding bits exist in 'from'
  for (uint i = min_length; i < to_length; i++) to->bitmap[i] = 0;
}

/*
  Set/clear all bits above a bit.

  SYNOPSIS
    bitmap_set_above()
    map                  RETURN The bitmap to change.
    from_byte                   The bitmap buffer byte offset to start with.
    use_bit                     The bit value (1/0) to use for all upper bits.

  NOTE
    You can only set/clear full bytes.
    The function is meant for the situation that you copy a smaller bitmap
    to a bigger bitmap. Bitmap lengths are always multiple of eigth (the
    size of a byte). Using 'from_byte' saves multiplication and division
    by eight during parameter passing.

  RETURN
    void
*/

void bitmap_set_above(MY_BITMAP *map, uint from_byte, bool use_bit) {
  uchar use_byte = use_bit ? 0xff : 0;
  uchar *to = (uchar *)map->bitmap + from_byte;
  uchar *end = (uchar *)map->bitmap + (map->n_bits + 7) / 8;

  for (; to < end; to++) *to = use_byte;
}

void bitmap_subtract(MY_BITMAP *map, const MY_BITMAP *map2) {
  my_bitmap_map *to = map->bitmap, *from = map2->bitmap, *end;
  DBUG_ASSERT(map->bitmap && map2->bitmap && map->n_bits == map2->n_bits);
  DBUG_ASSERT(map->n_bits > 0);
  end = map->last_word_ptr;

  for (; to <= end; to++, from++) *to &= ~(*from);
}

void bitmap_union(MY_BITMAP *map, const MY_BITMAP *map2) {
  my_bitmap_map *to = map->bitmap, *from = map2->bitmap, *end;
  DBUG_ASSERT(map->bitmap && map2->bitmap && map->n_bits == map2->n_bits);
  DBUG_ASSERT(map->n_bits > 0);
  end = map->last_word_ptr;

  for (; to <= end; to++, from++) *to |= *from;
}

void bitmap_xor(MY_BITMAP *map, const MY_BITMAP *map2) {
  my_bitmap_map *to = map->bitmap, *from = map2->bitmap, *end;
  DBUG_ASSERT(map->bitmap && map2->bitmap && map->n_bits == map2->n_bits);
  DBUG_ASSERT(map->n_bits > 0);
  end = map->last_word_ptr;

  for (; to <= end; to++, from++) *to ^= *from;
}

void bitmap_invert(MY_BITMAP *map) {
  my_bitmap_map *to = map->bitmap, *end;
  DBUG_ASSERT(map->bitmap);
  DBUG_ASSERT(map->n_bits > 0);
  end = map->last_word_ptr;

  for (; to <= end; to++) *to ^= 0xFFFFFFFF;
}

uint bitmap_bits_set(const MY_BITMAP *map) {
  my_bitmap_map *data_ptr = map->bitmap;
  my_bitmap_map *end = map->last_word_ptr;
  uint res = 0;
  DBUG_ASSERT(map->bitmap);
  DBUG_ASSERT(map->n_bits > 0);

  for (; data_ptr < end; data_ptr++) res += my_count_bits_uint32(*data_ptr);

  /*Reset last bits to zero*/
  res += my_count_bits_uint32(*map->last_word_ptr & ~map->last_word_mask);
  return res;
}

void bitmap_copy(MY_BITMAP *map, const MY_BITMAP *map2) {
  my_bitmap_map *to = map->bitmap, *from = map2->bitmap, *end;
  DBUG_ASSERT(map->bitmap && map2->bitmap && map->n_bits == map2->n_bits);
  DBUG_ASSERT(map->n_bits > 0);
  end = map->last_word_ptr;

  for (; to <= end; to++, from++) *to = *from;
}

uint bitmap_get_first_set(const MY_BITMAP *map) {
  uint word_pos;
  my_bitmap_map *data_ptr, *end = map->last_word_ptr;

  DBUG_ASSERT(map->bitmap);
  DBUG_ASSERT(map->n_bits > 0);
  data_ptr = map->bitmap;

  for (word_pos = 0; data_ptr < end; data_ptr++, word_pos++)
    if (*data_ptr) return get_first_set(*data_ptr, word_pos);

  return get_first_set(*map->last_word_ptr & ~map->last_word_mask, word_pos);
}

/**
  Get the next set bit.

  @param  map         Bitmap
  @param  bitmap_bit  Bit to start search from

  @return Index to first bit set after bitmap_bit
*/

uint bitmap_get_next_set(const MY_BITMAP *map, uint bitmap_bit) {
  uint word_pos, byte_to_mask, i;
  my_bitmap_map first_word;
  unsigned char *ptr = (unsigned char *)&first_word;
  my_bitmap_map *data_ptr, *end = map->last_word_ptr;

  DBUG_ASSERT(map->bitmap);
  DBUG_ASSERT(map->n_bits > 0);

  /* Look for the next bit */
  bitmap_bit++;
  if (bitmap_bit >= map->n_bits) return MY_BIT_NONE;
  word_pos = bitmap_bit / 32;
  data_ptr = map->bitmap + word_pos;
  first_word = *data_ptr;

  /* Mask out previous bits */
  byte_to_mask = (bitmap_bit % 32) / 8;
  for (i = 0; i < byte_to_mask; i++) ptr[i] = 0;
  ptr[byte_to_mask] &= 0xFFU << (bitmap_bit & 7);

  if (data_ptr == end)
    return get_first_set(first_word & ~map->last_word_mask, word_pos);

  if (first_word) return get_first_set(first_word, word_pos);

  for (data_ptr++, word_pos++; data_ptr < end; data_ptr++, word_pos++)
    if (*data_ptr) return get_first_set(*data_ptr, word_pos);

  return get_first_set(*end & ~map->last_word_mask, word_pos);
}

uint bitmap_get_first(const MY_BITMAP *map) {
  uint word_pos;
  my_bitmap_map *data_ptr, *end = map->last_word_ptr;

  DBUG_ASSERT(map->bitmap);
  DBUG_ASSERT(map->n_bits > 0);
  data_ptr = map->bitmap;

  for (word_pos = 0; data_ptr < end; data_ptr++, word_pos++)
    if (*data_ptr != 0xFFFFFFFF) return get_first_not_set(*data_ptr, word_pos);

  return get_first_not_set(*map->last_word_ptr | map->last_word_mask, word_pos);
}
