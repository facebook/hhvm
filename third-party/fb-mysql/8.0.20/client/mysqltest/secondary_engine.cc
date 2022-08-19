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

#include "client/mysqltest/secondary_engine.h"
#include "client/mysqltest/error_names.h"
#include "client/mysqltest/utils.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>

#include "my_dbug.h"

static int offload_count_after = 0;
static int offload_count_before = 0;

/// Get secondary engine execution count value.
//
/// @param mysql mysql handle
/// @param mode  Mode value (either "after" or "before")
///
/// @retval True if the query fails, false otherwise.
bool Secondary_engine::offload_count(MYSQL *mysql, const char *mode) {
  std::string offload_count;

  const char *query =
      "SHOW GLOBAL STATUS LIKE 'Secondary_engine_execution_count'";

  if (query_get_string(mysql, query, 1, &offload_count)) {
    int error = mysql_errno(mysql);
    if (error == 0 || error == 1104 || error == 2006) return false;
    std::cerr << "mysqltest: Query '" << query << "' failed, ERROR " << error
              << " (" << mysql_sqlstate(mysql) << "): " << mysql_error(mysql)
              << std::endl;
    return true;
  }

  if (!std::strcmp(mode, "before")) {
    offload_count_before = get_int_val(offload_count.c_str());
  } else if (!std::strcmp(mode, "after")) {
    if (!offload_count_after) {
      offload_count_after = get_int_val(offload_count.c_str());
    } else {
      offload_count_after =
          offload_count_after + get_int_val(offload_count.c_str());
    }
  }

  return false;
}

/// Report secondary engine execution count value.
///
/// @param filename File to store the count value
void Secondary_engine::report_offload_count(const char *filename) {
  if (!offload_count_after && offload_count_after < offload_count_before)
    offload_count_after = offload_count_before;

  int count_val = offload_count_after - offload_count_before;
  DBUG_ASSERT(count_val >= 0);

  std::ofstream report_file(filename, std::ios::out);

  if (report_file.is_open()) {
    std::string count = std::to_string(count_val);
    report_file << count << std::endl;
  }

  report_file.close();
}
