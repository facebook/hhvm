/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MOCK_FIELD_VARSTRING_INCLUDED
#define MOCK_FIELD_VARSTRING_INCLUDED

#include <vector>

#include "sql/field.h"

/** Helper to simplify creating fields. */
class Mock_field_varstring : public Field_varstring {
 public:
  /**
    Creates a column.
    @param share TABLE_SHARE this field belongs to.
    @param name The column name.
    @param char_len Length in chars
    @param is_nullable Whether it's nullable.
  */
  Mock_field_varstring(TABLE_SHARE *share, const char *name, uint char_len,
                       bool is_nullable)
      : Field_varstring(nullptr,                          // ptr_arg
                        calc_len_arg(char_len),           // len_arg
                        calc_length_bytes_arg(char_len),  // length_bytes_arg
                        is_nullable ? buffer : nullptr,   // null_ptr_arg
                        is_nullable ? 1 : 0,              // null_bit_arg
                        Field::NONE,                      // auto_flags_arg
                        name,                             // field_name_arg
                        share,                            // share
                        &my_charset_utf8mb4_general_ci)   // cs
  {
    ptr = buffer + 1;
    std::memset(buffer, 0, MAX_FIELD_VARCHARLENGTH + 1);

    static const char *table_name_buf = "table_name";
    table_name = &table_name_buf;
  }

 private:
  uchar buffer[MAX_FIELD_VARCHARLENGTH + 1];

  static uint calc_byte_len(uint char_len) {
    return char_len * my_charset_utf8mb4_general_ci.mbmaxlen;
  }

  static uint32 calc_len_arg(uint char_len) {
    return calc_byte_len(char_len) + calc_length_bytes_arg(char_len);
  }

  static uint calc_length_bytes_arg(uint char_len) {
    return (calc_byte_len(char_len) < 256) ? 1 : 2;
  }
};

#endif  // MOCK_FIELD_VARSTRING_INCLUDED
