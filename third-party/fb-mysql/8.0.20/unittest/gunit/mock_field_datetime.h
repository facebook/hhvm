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

#ifndef MOCK_FIELD_DATETIME_H
#define MOCK_FIELD_DATETIME_H

#include "sql/field.h"
#include "unittest/gunit/fake_table.h"

class Mock_field_datetime : public Field_datetime {
  void initialize() {
    table = new Fake_TABLE(this);
    ptr = table->record[0];
    // Make it possible to write into this field
    bitmap_set_bit(table->write_set, 0);
  }

 public:
  Mock_field_datetime() : Field_datetime("") { initialize(); }
  ~Mock_field_datetime() override { delete static_cast<Fake_TABLE *>(table); }
};

#endif  // MOCK_FIELD_DATETIME_H
