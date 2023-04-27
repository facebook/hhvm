// Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 2.0,
// as published by the Free Software Foundation.
//
// This program is also distributed with certain software (including
// but not limited to OpenSSL) that is licensed under separate terms,
// as designated in a particular file or component or in included license
// documentation.  The authors of MySQL hereby grant you an additional
// permission to link the program and your derivative works with the
// separately licensed software that they have included with MySQL.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License, version 2.0, for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.

#include "client/mysqltest/error_names.h"

static st_error global_error_names[] = {
    {"<No error>", static_cast<int>(-1), "", nullptr, nullptr, 0},
#ifndef IN_DOXYGEN
#include "mysqlclient_ername.h"
#include "mysqld_ername.h"
#endif /* IN_DOXYGEN */
    {nullptr, 0, nullptr, nullptr, nullptr, 0}};

int get_errcode_from_name(std::string error_name) {
  for (st_error *error = global_error_names; error->name; error++) {
    if (error_name.compare(error->name) == 0) return error->error_code;
  }

  // Unknown SQL error name, return -1
  return -1;
}

const char *get_errname_from_code(int error_code) {
  // Return an empty string if error code is 0.
  if (!error_code) return "";

  for (st_error *error = global_error_names; error->error_code; error++) {
    if (error->error_code == error_code) return error->name;
  }

  // Unknown SQL error code, return "<Unknown>" keyword.
  return "<Unknown>";
}
