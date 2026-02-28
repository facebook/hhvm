/* Copyright (c) 2003, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  Implementation of a bitmap type.
  The idea with this is to be able to handle any constant number of bits but
  also be able to use 32 or 64 bits bitmaps very efficiently
*/

#ifndef SQL_BITMAP_INCLUDED
#define SQL_BITMAP_INCLUDED

#include "m_string.h"      // longlong2str
#include "my_bitmap.h"     // MY_BITMAP
#include "my_byteorder.h"  // int8store
#include "my_dbug.h"
#include "template_utils.h"

template <uint default_width>
class Bitmap {
  MY_BITMAP map;
  uint32 buffer[(default_width + 31) / 32];

 public:
  enum { ALL_BITS = default_width };
  Bitmap() { init(); }
  Bitmap(const Bitmap &from) { *this = from; }
  explicit Bitmap(uint prefix_to_set) { init(prefix_to_set); }
  void init() { bitmap_init(&map, buffer, default_width); }
  void init(uint prefix_to_set) {
    init();
    set_prefix(prefix_to_set);
  }
  uint length() const { return default_width; }
  Bitmap &operator=(const Bitmap &map2) {
    init();
    memcpy(buffer, map2.buffer, sizeof(buffer));
    return *this;
  }
  void set_bit(uint n) { bitmap_set_bit(&map, n); }
  void clear_bit(uint n) { bitmap_clear_bit(&map, n); }
  void set_prefix(uint n) { bitmap_set_prefix(&map, n); }
  void set_all() { bitmap_set_all(&map); }
  void clear_all() { bitmap_clear_all(&map); }
  void intersect(const Bitmap &map2) { bitmap_intersect(&map, &map2.map); }
  void intersect(ulonglong map2buff) {
    // Use a spearate temporary buffer, as bitmap_init() clears all the bits.
    ulonglong buf2;
    MY_BITMAP map2;

    bitmap_init(&map2, (uint32 *)&buf2, sizeof(ulonglong) * 8);

    // Store the original bits.
    if (sizeof(ulonglong) >= 8) {
      int8store(
          const_cast<uchar *>(static_cast<uchar *>(static_cast<void *>(&buf2))),
          map2buff);
    } else {
      DBUG_ASSERT(sizeof(buffer) >= 4);
      int4store(
          const_cast<uchar *>(static_cast<uchar *>(static_cast<void *>(&buf2))),
          static_cast<uint32>(map2buff));
    }

    bitmap_intersect(&map, &map2);
  }
  /* Use highest bit for all bits above sizeof(ulonglong)*8. */
  void intersect_extended(ulonglong map2buff) {
    intersect(map2buff);
    if (map.n_bits > sizeof(ulonglong) * 8)
      bitmap_set_above(&map, sizeof(ulonglong),
                       (map2buff & (1LL << (sizeof(ulonglong) * 8 - 1))));
  }
  void subtract(const Bitmap &map2) { bitmap_subtract(&map, &map2.map); }
  void merge(const Bitmap &map2) { bitmap_union(&map, &map2.map); }
  bool is_set(uint n) const { return bitmap_is_set(&map, n); }
  bool is_prefix(uint n) const { return bitmap_is_prefix(&map, n); }
  bool is_clear_all() const { return bitmap_is_clear_all(&map); }
  bool is_set_all() const { return bitmap_is_set_all(&map); }
  bool is_subset(const Bitmap &map2) const {
    return bitmap_is_subset(&map, &map2.map);
  }
  bool is_overlapping(const Bitmap &map2) const {
    return bitmap_is_overlapping(&map, &map2.map);
  }
  bool operator==(const Bitmap &map2) const {
    return bitmap_cmp(&map, &map2.map);
  }
  bool operator!=(const Bitmap &map2) const { return !(*this == map2); }
  char *print(char *buf) const {
    char *s = buf;
    const uchar *e = pointer_cast<const uchar *>(&buffer[0]),
                *b = e + sizeof(buffer) - 1;
    while (!*b && b > e) b--;
    if ((*s = _dig_vec_upper[*b >> 4]) != '0') s++;
    *s++ = _dig_vec_upper[*b & 15];
    while (--b >= e) {
      *s++ = _dig_vec_upper[*b >> 4];
      *s++ = _dig_vec_upper[*b & 15];
    }
    *s = 0;
    return buf;
  }
  ulonglong to_ulonglong() const {
    if (sizeof(buffer) >= 8)
      return uint8korr(
          static_cast<const uchar *>(static_cast<const void *>(buffer)));
    DBUG_ASSERT(sizeof(buffer) >= 4);
    return (ulonglong)uint4korr(
        static_cast<const uchar *>(static_cast<const void *>(buffer)));
  }
  uint bits_set() const { return bitmap_bits_set(&map); }
  uint get_first_set() { return bitmap_get_first_set(&map); }
};

template <>
class Bitmap<64> {
  ulonglong map;

 public:
  Bitmap<64>() { init(); }
  enum { ALL_BITS = 64 };

  explicit Bitmap<64>(uint prefix_to_set) { set_prefix(prefix_to_set); }
  void init() { clear_all(); }
  void init(uint prefix_to_set) { set_prefix(prefix_to_set); }
  uint length() const { return 64; }
  void set_bit(uint n) {
    DBUG_ASSERT(n < 64);
    map |= ((ulonglong)1) << n;
  }
  void clear_bit(uint n) {
    DBUG_ASSERT(n < 64);
    map &= ~(((ulonglong)1) << n);
  }
  void set_prefix(uint n) {
    if (n >= length())
      set_all();
    else
      map = (((ulonglong)1) << n) - 1;
  }
  void set_all() { map = ~(ulonglong)0; }
  void clear_all() { map = (ulonglong)0; }
  void intersect(const Bitmap<64> &map2) { map &= map2.map; }
  void intersect(ulonglong map2) { map &= map2; }
  void intersect_extended(ulonglong map2) { map &= map2; }
  void subtract(const Bitmap<64> &map2) { map &= ~map2.map; }
  void merge(const Bitmap<64> &map2) { map |= map2.map; }
  bool is_set(uint n) const {
    DBUG_ASSERT(n < 64);
    return (map & (((ulonglong)1) << n));
  }
  bool is_prefix(uint n) const {
    DBUG_ASSERT(n <= 64);
    if (n < 64)
      return map == (((ulonglong)1) << n) - 1;
    else
      return map == ~(ulonglong)1;
  }
  bool is_clear_all() const { return map == (ulonglong)0; }
  bool is_set_all() const { return map == ~(ulonglong)0; }
  bool is_subset(const Bitmap<64> &map2) const { return !(map & ~map2.map); }
  bool is_overlapping(const Bitmap<64> &map2) const {
    return (map & map2.map) != 0;
  }
  bool operator==(const Bitmap<64> &map2) const { return map == map2.map; }
  bool operator!=(const Bitmap<64> &map2) const { return !(*this == map2); }
  char *print(char *buf) const {
    longlong2str(map, buf, 16);
    return buf;
  }
  ulonglong to_ulonglong() const { return map; }
  uint bits_set() const {
    uint res = 0;
    for (uint i = 0; i < ALL_BITS; i++)
      if (is_set(i)) res++;
    return res;
  }
  uint get_first_set() const {
    for (uint i = 0; i < ALL_BITS; i++)
      if (map & (1ULL << i)) return i;
    return MY_BIT_NONE;
  }
};

/* An iterator to quickly walk over bits in unlonglong bitmap. */
class Table_map_iterator {
  ulonglong bmp;
  uint no;

 public:
  Table_map_iterator(ulonglong t) : bmp(t), no(0) {}
  int next_bit() {
    static const char last_bit[16] = {32, 0, 1, 0, 2, 0, 1, 0,
                                      3,  0, 1, 0, 2, 0, 1, 0};
    uint bit;
    while ((bit = last_bit[bmp & 0xF]) == 32) {
      no += 4;
      bmp = bmp >> 4;
      if (!bmp) return BITMAP_END;
    }
    bmp &= ~(1LL << bit);
    return no + bit;
  }
  enum { BITMAP_END = 64 };
};

#if MAX_INDEXES <= 64
typedef Bitmap<64> Key_map; /* Used for finding keys */
#elif MAX_INDEXES > 255
#error "MAX_INDEXES values greater than 255 is not supported."
#else
typedef Bitmap<((MAX_INDEXES + 7) / 8 * 8)> Key_map; /* Used for finding keys */
#endif

#endif /* SQL_BITMAP_INCLUDED */
