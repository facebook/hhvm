/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BASE_MOCK_FIELD_INCLUDED
#define BASE_MOCK_FIELD_INCLUDED

// First include (the generated) my_config.h, to get correct platform defines.
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "my_config.h"

#include "sql/field.h"
#include "sql/json_dom.h"
#include "sql/table.h"

/**
  Base mocks for Field_*. Create subclasses mocking additional virtual
  functions depending on what you want to test.
*/

class Base_mock_field_longlong : public Field_longlong {
 public:
  Base_mock_field_longlong()
      : Field_longlong(nullptr,                    // ptr_arg
                       8,                          // len_arg
                       &Field::dummy_null_buffer,  // null_ptr_arg
                       1,                          // null_bit_arg
                       Field::NONE,                // auto_flags_arg
                       "field_name",               // field_name_arg
                       false,                      // zero_arg
                       false)                      // unsigned_arg
  {}

  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
  void make_readable() { bitmap_set_bit(table->read_set, field_index); }
};

class Base_mock_field_varstring : public Field_varstring {
 public:
  Base_mock_field_varstring(uint32 length, TABLE_SHARE *share)
      : Field_varstring(length,              // len_arg
                        false,               // maybe_null_arg
                        "field_NAME",        // field_name_arg
                        share,               // share
                        &my_charset_latin1)  // char set
  {}
};

class Base_mock_field_json : public Field_json {
 public:
  Base_mock_field_json() : Field_json(MAX_BLOB_WIDTH, true, "json_field") {}
  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
};

#endif  // BASE_MOCK_FIELD_INCLUDED
