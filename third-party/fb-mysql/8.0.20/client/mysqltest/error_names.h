#ifndef ERROR_NAMES_INCLUDED
#define ERROR_NAMES_INCLUDED

// Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.
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

#include <string>

struct st_error {
  const char *name;
  int error_code;
  const char *description;
  const char *odbc_state;
  const char *jdbc_state;
  unsigned int error_index;
};

/// Get an error code from an error name string.
///
/// @param error_name Error name string
///
/// @retval -1 if error name is unknown, error code otherwise.
int get_errcode_from_name(std::string error_name);

/// Get an error name from an error code.
///
/// @param error_code Error code
///
/// @retval "<Unknown>" keyword if error code is unknown, error name
///          otherwise.
const char *get_errname_from_code(int error_code);

#endif  // ERROR_NAMES_INCLUDED
