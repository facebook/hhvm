/* Copyright (c) 2007, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_RECORD_H
#define RPL_RECORD_H

#include <stddef.h>
#include <sys/types.h>
#include <string>

#include "my_inttypes.h"

class Relay_log_info;
struct TABLE;

struct MY_BITMAP;

enum class enum_row_image_type { WRITE_AI, UPDATE_BI, UPDATE_AI, DELETE_BI };

#if defined(MYSQL_SERVER)
size_t pack_row(TABLE *table, MY_BITMAP const *cols, uchar *row_data,
                const uchar *data, enum_row_image_type row_image_type,
                ulonglong value_options = 0);

bool unpack_row(Relay_log_info const *rli, TABLE *table,
                uint const master_column_count, uchar const *const row_data,
                MY_BITMAP const *column_image,
                uchar const **const row_image_end_p,
                uchar const *const event_end,
                enum_row_image_type row_image_type,
                bool event_has_value_options, bool only_seek,
                std::string *row_query = nullptr);

// Fill table's record[0] with default values.
int prepare_record(TABLE *const table, const MY_BITMAP *cols, const bool check);
#endif

/**
  Template base class of Bit_reader / Bit_writer.
*/
template <typename T, typename UT>
class Bit_stream_base {
 protected:
  /// Pointer to beginning of buffer where bits are read or written.
  T *m_ptr;
  /// Current position in buffer.
  uint m_current_bit;

 public:
  /**
    Construct a new Bit_stream (either reader or writer).
    @param ptr Pointer where bits will be read or written.
  */
  Bit_stream_base(T *ptr) : m_ptr(ptr), m_current_bit(0) {}

  /**
    Set the buffer pointer.
    @param ptr Pointer where bits will be read or written.
  */
  void set_ptr(T *ptr) { m_ptr = ptr; }
  /**
    Set the buffer pointer, using an unsigned datatype.
    @param ptr Pointer where bits will be read or written.
  */
  void set_ptr(UT *ptr) { m_ptr = (T *)ptr; }

  /// @return the current position.
  uint tell() const { return m_current_bit; }

  /**
    Print all the bits before the current position to the debug trace.
    @param str Descriptive text that will be prefixed before the bit string.
  */
  void dbug_print(const char *str) const;
};

/**
  Auxiliary class to write a stream of bits to a memory location.

  Call set() to write a bit and move the position one bit foward.
*/
class Bit_writer : public Bit_stream_base<char, uchar> {
 public:
  Bit_writer(char *ptr = nullptr) : Bit_stream_base<char, uchar>(ptr) {}
  Bit_writer(uchar *ptr) : Bit_writer((char *)ptr) {}

  /**
    Write the next bit and move the write position one bit forward.
    @param set_to_on If true, set the bit to 1, otherwise set it to 0.
  */
  void set(bool set_to_on) {
    uint byte = m_current_bit / 8;
    uint bit_within_byte = m_current_bit % 8;
    m_current_bit++;
    if (bit_within_byte == 0)
      m_ptr[byte] = set_to_on ? 1 : 0;
    else if (set_to_on)
      m_ptr[byte] |= 1 << bit_within_byte;
  }
};

/**
  Auxiliary class to read or write a stream of bits to a memory location.

  Call get() to read a bit and move the position one bit foward.
*/
class Bit_reader : public Bit_stream_base<const char, const uchar> {
 public:
  Bit_reader(const char *ptr = nullptr)
      : Bit_stream_base<const char, const uchar>(ptr) {}
  Bit_reader(const uchar *ptr) : Bit_reader((const char *)ptr) {}

  /**
    Read the next bit and move the read position one bit forward.
    @return true if the bit was 1, false if the bit was 0.
  */
  bool get() {
    uint byte = m_current_bit / 8;
    uint bit_within_byte = m_current_bit % 8;
    m_current_bit++;
    return (m_ptr[byte] & (1 << bit_within_byte)) != 0;
  }
};

#endif  // ifdef RPL_RECORD_H
