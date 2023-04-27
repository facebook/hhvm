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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

#include "client/mysqltest/utils.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

/// Run a query and return one field in the result set from the first
/// row and column.
///
/// @param mysql  mysql handle
/// @param query  Query string
/// @param column Column value
/// @param str    String object to store the rvalue
///
/// @retval True if fails to get the value, true otherwise.
bool query_get_string(MYSQL *mysql, const char *query, int column,
                      std::string *str) {
  MYSQL_RES *res = nullptr;
  MYSQL_ROW row;

  if (mysql_query(mysql, query)) {
    mysql_free_result(res);
    return true;
  }

  if (((res = mysql_store_result(mysql)) == nullptr) ||
      ((row = mysql_fetch_row(res)) == nullptr)) {
    mysql_free_result(res);
    str = nullptr;
    return true;
  }

  str->assign(row[column] ? row[column] : "NULL");
  mysql_free_result(res);

  return false;
}

/// Use stoi function to get the integer value from a string.
///
/// @param str String which may contain an integer or an alphanumeric
///            string.
///
/// @retval Integer value corresponding to the contents of the string,
///         if conversion is successful, or -1 if integer is out of
///         range, or if the conversion fails.
int get_int_val(const char *str) {
  int value;
  std::size_t size;

  try {
    value = std::stoi(str, &size, 10);
    if (size != std::strlen(str)) value = -1;
  } catch (const std::out_of_range &) {
    std::fprintf(stderr, "Integer value '%s' is out of range. ", str);
    value = -1;
  } catch (const std::invalid_argument &) {
    value = -1;
  }

  return value;
}
