#ifndef SECONDARY_ENGINE_INCLUDED
#define SECONDARY_ENGINE_INCLUDED

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

#include <string>
#include <vector>

#include "mysql.h"

class Secondary_engine {
 public:
  Secondary_engine() {}
  ~Secondary_engine() {}

  /// Get the secondary engine execution count value.
  ///
  /// @param mysql mysql handle
  /// @param mode  Mode value (either "after" or "before")
  ///
  /// @retval True if the query fails, false otherwise.
  bool offload_count(MYSQL *mysql, const char *mode);

  /// Report secondary engine execution count value.
  ///
  /// @param filename File to store the count value
  void report_offload_count(const char *filename);
};

#endif  // SECONDARY_ENGINE_INCLUDED
