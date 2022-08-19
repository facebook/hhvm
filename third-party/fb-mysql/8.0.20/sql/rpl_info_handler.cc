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

#include "sql/rpl_info_handler.h"

#include "my_dbug.h"
#include "sql/rpl_info_values.h"  // Rpl_info_values

bool operator!(Rpl_info_handler::enum_field_get_status status) {
  return status ==
         Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL;
}
#include <iostream>
Rpl_info_handler::Rpl_info_handler(const int nparam,
                                   MY_BITMAP const *nullable_bitmap)
    : field_values(nullptr),
      ninfo(nparam),
      cursor((my_off_t)0),
      prv_error(false),
      prv_get_error(
          Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL),
      sync_counter(0),
      sync_period(0) {
  field_values = new Rpl_info_values(ninfo);
  /*
    Configures fields to temporary hold information. If the configuration
    fails due to memory allocation problems, the object is deleted.
  */
  if (field_values && field_values->init()) {
    delete field_values;
    field_values = nullptr;
  }
  bitmap_init(&nullable_fields, nullptr, nparam);
  bitmap_clear_all(&nullable_fields);
  if (nullable_bitmap != nullptr)
    bitmap_copy(&nullable_fields, nullable_bitmap);
}

Rpl_info_handler::~Rpl_info_handler() {
  bitmap_free(&nullable_fields);
  delete field_values;
}

void Rpl_info_handler::set_sync_period(uint period) { sync_period = period; }

const char *Rpl_info_handler::get_rpl_info_type_str() {
  switch (do_get_rpl_info_type()) {
    case INFO_REPOSITORY_DUMMY:
      return "DUMMY";
    case INFO_REPOSITORY_FILE:
      return "FILE";
    case INFO_REPOSITORY_TABLE:
      return "TABLE";
  }

  DBUG_ASSERT(0);
  return "";
}

bool Rpl_info_handler::is_field_nullable(int pos) {
  return bitmap_is_set(&nullable_fields, pos);
}
