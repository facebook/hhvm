/*****************************************************************************

Copyright (c) 2016, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/
#ifndef _mach0data_h_
#define _mach0data_h_

#include "lot0types.h"
#include "mtr0types.h"
#include "ut0byte.h"
#include "ut0dbg.h"

inline uint8_t mach_read_from_1(const byte *b) {
  ut_ad(b != nullptr);
  return ((uint8_t)(b[0]));
}

inline uint16_t mach_read_from_2(const byte *b) {
  return (((ulint)(b[0]) << 8) | (ulint)(b[1]));
}

inline void mach_write_to_1(byte *b, ulint n) { b[0] = (byte)n; }

inline void mach_write_to_2(byte *b, ulint n) {
  b[0] = (byte)(n >> 8);
  b[1] = (byte)(n);
}

inline void mach_write_to_4(byte *b, ulint n) {
  b[0] = (byte)(n >> 24);
  b[1] = (byte)(n >> 16);
  b[2] = (byte)(n >> 8);
  b[3] = (byte)n;
}

/** The following function is used to fetch data from 4 consecutive
bytes. The most significant byte is at the lowest address.
@param[in] b pointer to four bytes.
@return ulint integer */
inline ulint mach_read_from_4(const byte *b) {
  return (((ulint)(b[0]) << 24) | ((ulint)(b[1]) << 16) | ((ulint)(b[2]) << 8) |
          (ulint)(b[3]));
}

inline uint32_t mach_read_ulint(const byte *ptr, mlog_id_t type) {
  switch (type) {
    case MLOG_1BYTE:
      return (mach_read_from_1(ptr));
    case MLOG_2BYTES:
      return (mach_read_from_2(ptr));
    case MLOG_4BYTES:
      return (mach_read_from_4(ptr));
    default:
      break;
  }

  ut_error;
  return (0);
}

/** The following function is used to fetch data from 6 consecutive
bytes. The most significant byte is at the lowest address.
@param[in]      b       pointer to 6 bytes to read
@return 48-bit integer */
inline ib_uint64_t mach_read_from_6(const byte *b) {
  ut_ad(b != nullptr);

  return (ut_ull_create(mach_read_from_2(b), mach_read_from_4(b + 2)));
}

/** The following function is used to store data in 6 consecutive
bytes. We store the most significant byte to the lowest address.
@param[in]      b       pointer to 6 bytes where to store
@param[in]      n       48-bit integer to write */
inline void mach_write_to_6(byte *b, ib_uint64_t n) {
  ut_ad(b != nullptr);

  mach_write_to_2(b, (ulint)(n >> 32));
  mach_write_to_4(b + 2, (ulint)n);
}

#endif  // _mach0data_h_
