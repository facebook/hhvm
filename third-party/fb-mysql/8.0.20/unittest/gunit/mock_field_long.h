#ifndef MOCK_FIELD_LONG_INCLUDED
#define MOCK_FIELD_LONG_INCLUDED
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

#include "sql/field.h"

/**
  Base class for creating mock Field objects.

  To do: Make all other tests #include this file instead of using
  their own copy-pasted variants.
*/
class Mock_field_long : public Field_long {
 public:
  /// Creates a nullable column with the default name.
  Mock_field_long() : Mock_field_long("field_name", true) {}

  /**
    Creates a nullable column.
    @param name The column name.
  */
  Mock_field_long(const char *name) : Mock_field_long(name, true) {}

  /**
    Creates a nullable column.
    @param name The column name.
  */
  Mock_field_long(const std::string &&name, bool is_nullable)
      : Mock_field_long(name.c_str(), is_nullable) {}

  /**
    Creates a column.
    @param name The column name.
    @param is_nullable Whether it's nullable.
  */
  Mock_field_long(const char *name, bool is_nullable)
      : Field_long(
            nullptr,                                            // ptr_arg
            8,                                                  // len_arg
            is_nullable ? &Field::dummy_null_buffer : nullptr,  // null_ptr_arg
            is_nullable ? 1 : 0,                                // null_bit_arg
            Field::NONE,  // auto_flags_arg
            name,         // field_name_arg
            false,        // zero_arg
            false)        // unsigned_arg
  {
    initialize(name);
  }

  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
  void make_readable() { bitmap_set_bit(table->read_set, field_index); }

 private:
  char m_name[1024];

  void initialize(const char *name) {
    static const char *table_name_buf = "table_name";
    table_name = &table_name_buf;
    if (name) {
      snprintf(m_name, sizeof(m_name), "%.1023s", name);
      field_name = m_name;
    }
  }
};

#endif  // MOCK_FIELD_LONG_INCLUDED
