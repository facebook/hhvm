/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_info_values.h"

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql_string.h"  // String

Rpl_info_values::Rpl_info_values(int param_ninfo)
    : value(nullptr), ninfo(param_ninfo) {}

/**
  Initializes a sequence of values to be read from or stored into a repository.
  The number of values created and initialized are determined by the property
  @c ninfo which is set while calling the constructor. Each value is created
  with the default size of @c FN_REFLEN.

  @retval false No error
  @retval true Failure
*/
bool Rpl_info_values::init() {
  DBUG_TRACE;

  if (!value && !(value = new String[ninfo])) return true;
  if (bitmap_init(&is_null, nullptr, ninfo)) {
    delete[] value;
    return true;
  }
  bitmap_clear_all(&is_null);
  return false;
}

Rpl_info_values::~Rpl_info_values() {
  delete[] value;
  bitmap_free(&is_null);
}
