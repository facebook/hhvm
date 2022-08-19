/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "unittest/gunit/skip_trailing.h"

#include "my_config.h"

#include "my_byteorder.h"
#include "my_dbug.h"

namespace skip_trailing_space_unittest {

/* SPACE_INT is a word that contains only spaces */
#if SIZEOF_INT == 4
static const unsigned SPACE_INT = 0x20202020U;
#elif SIZEOF_INT == 8
static const unsigned SPACE_INT = 0x2020202020202020ULL;
#else
#error define the appropriate constant for a word full of spaces
#endif

// A copy of the original version of skip_trailing_space
const uchar *skip_trailing_orig(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;

  if (len > 20) {
    const uchar *end_words = (const uchar *)(intptr)(((ulonglong)(intptr)end) /
                                                     SIZEOF_INT * SIZEOF_INT);
    const uchar *start_words = (const uchar *)(intptr)(
        (((ulonglong)(intptr)ptr) + SIZEOF_INT - 1) / SIZEOF_INT * SIZEOF_INT);

    DBUG_ASSERT(((ulonglong)(intptr)ptr) >= SIZEOF_INT);
    if (end_words > ptr) {
      while (end > end_words && end[-1] == 0x20) end--;
      if (end[-1] == 0x20 && start_words < end_words)
        while (end > start_words &&
               (pointer_cast<const unsigned *>(end))[-1] == SPACE_INT)
          end -= SIZEOF_INT;
    }
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}

// Read 8 bytes at a time, ignoring alignment.
// We use uint8korr which is fast on all platforms, except sparc.
const uchar *skip_trailing_unalgn(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;
  while (end - ptr >= 8) {
    if (uint8korr(end - 8) != 0x2020202020202020ULL) break;
    end -= 8;
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}

// Same as the original, except we skip a test which is always true.
const uchar *skip_trailing_4byte(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;

  if (len > 20) {
    const uchar *end_words = (const uchar *)(intptr)(((ulonglong)(intptr)end) /
                                                     SIZEOF_INT * SIZEOF_INT);
    const uchar *start_words = (const uchar *)(intptr)(
        (((ulonglong)(intptr)ptr) + SIZEOF_INT - 1) / SIZEOF_INT * SIZEOF_INT);

    DBUG_ASSERT(end_words > ptr);
    {
      while (end > end_words && end[-1] == 0x20) end--;
      if (end[-1] == 0x20 && start_words < end_words)
        while (end > start_words &&
               (pointer_cast<const unsigned *>(end))[-1] == SPACE_INT)
          end -= SIZEOF_INT;
    }
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}

// Same as skip_trailing_4byte, except we read 8 bytes at a time (aligned).
const uchar *skip_trailing_8byte(const uchar *ptr, size_t len) {
  const uchar *end = ptr + len;

  if (len > 20) {
    const uchar *end_words =
        (const uchar *)(longlong)(((ulonglong)(longlong)end) / 8 * 8);
    const uchar *start_words =
        (const uchar *)(longlong)((((ulonglong)(longlong)ptr) + 8 - 1) / 8 * 8);

    DBUG_ASSERT(end_words > ptr);
    {
      while (end > end_words && end[-1] == 0x20) end--;
      if (end[-1] == 0x20 && start_words < end_words)
        while (end > start_words && (pointer_cast<const ulonglong *>(
                                        end))[-1] == 0x2020202020202020ULL)
          end -= 8;
    }
  }
  while (end > ptr && end[-1] == 0x20) end--;
  return (end);
}
}  // namespace skip_trailing_space_unittest
