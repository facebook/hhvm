/* Copyright (c) 2006, 2018, Oracle and/or its affiliates. All rights reserved.

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

#pragma once

#include <array>
#include <string>

/*
 * `sql_operation` defines operations performed by a query where an index could
 * have been useful.
 */
enum sql_operation : int { FILTER, TABLE_JOIN, ORDER_BY, GROUP_BY };

/*
 * `operator_type` defines operators which let specify filtration predicates
 * across a variety of queries - update, select, delete etc.
 */
enum operator_type : int {
  BETWEEN,
  EQUAL,
  NULLSAFE_EQUAL,
  LESS_THAN,
  LESS_THAN_EQUAL,
  GREATER_THAN,
  GREATER_THAN_EQUAL,
  NOT_EQUAL,
  NULL_CHECK,
  SET_MEMBERSHIP,
  PATTERN_MATCH,
  SORT_ASCENDING,
  SORT_DESCENDING,
  UNKNOWN_OPERATOR
};

/*
 * ColumnUsageInfo struct contains information about the usage of a specific
 * column in a table. A single query may use several different columns across
 * multiple tables.
 */
struct ColumnUsageInfo {
  std::string table_schema;
  std::string table_name;
  std::string table_instance;
  std::string column_name;
  sql_operation sql_op;
  operator_type op_type;

  // Comparator required for defining a strict weak ordering of
  // `ColumnUsageInfo` structs.
  bool operator<(const ColumnUsageInfo &) const;
};
