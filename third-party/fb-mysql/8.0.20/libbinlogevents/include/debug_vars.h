/* Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file debug_vars.h

  @brief This header file contains the status of variables used by MySQL tests
  for debug operations. The variables are set to true by the MySQL server if
  the test pertaining to the variable is active. The variables are initialized
   with false (in binlog_event.cpp).
*/
#ifndef DEBUG_VARS_INCLUDED
#define DEBUG_VARS_INCLUDED

namespace binary_log_debug {
extern bool debug_checksum_test;
extern bool debug_query_mts_corrupt_db_names;
extern bool debug_simulate_invalid_address;
extern bool debug_pretend_version_50034_in_binlog;
// TODO(WL#7546):Add variables here as we move methods into libbinlogevent
// from the server while implementing the WL#7546(Moving binlog event
// encoding into a separate package)
}  // namespace binary_log_debug
#endif
