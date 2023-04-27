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

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "include/my_md5.h"
#include "sql/column_statistics_dt.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/key_spec.h"

class column_statistics_row;

/*
  sql_operation_string
    Stringifies sql_operation enum.
  Input:
    sql_op                in: sql_operation
  Output:
    sql_operation_name    std::string
*/
std::string sql_operation_string(const sql_operation &sql_op);

/*
  operator_type_string
    Stringifies operator_type enum.
  Input:
    op_type         in: operator_type
  Output:
    op_type_name    std::string
*/
std::string operator_type_string(const operator_type &op_type);

/*
  match_op
    Matches an item function type to operators that feature in column usage
    stats.
  Input:
    fitem_type     in: Item_func::Functype
  Output:
    operator_type
*/
operator_type match_op(Item_func::Functype fitem_type);

/*
  match_op
    Overloads match_op to match ORDER::enum_order to operators that feature
    in column usage stats.
  Input:
    direction     in: ORDER::enum_order
  Output:
    operator_type
*/
operator_type match_op(enum_order direction);

/*
  fetch_table_info
    Helper to fetch information of the base table to which the field belongs.
    This includes the name and the table instance in a query.
  Input:
    field_arg     in:  Item_field
                       The field argument to parse the column usage info
                       struct from.
    cui           out: ColumnUsageInfo
                       Base table information is populated in this structure.
*/
void fetch_table_info(Item_field *field_arg, ColumnUsageInfo *cui);

/*
  populate_field_info
    Helper to parse column usage information corresponding to a single
    function item.
  Input:
    op            in: sql_operation
                      The SQL operation FILTER, TABLE_JOIN etc.
                      in which the field was used.
    op_type       in: operator_type
    field_arg     in: Item_field
                      The field argument to parse the column usage info
                      struct from.
    out_cus       out: std::set<ColumnUsageInfo>
                       Column usage information parsed from field_arg.
*/
void populate_field_info(const sql_operation &op, const operator_type &op_type,
                         Item_field *field_arg,
                         std::set<ColumnUsageInfo> &out_cus);

/*
  parse_column_from_func_item
    Helper to parse column usage information corresponding to a single
    function item.
  Input:
    op            in: sql_operation
                      The SQL operation FILTER, TABLE_JOIN etc.
                      corresponding to the functional item.
    fitem         in: Item_func
                      The functional item to be parsed.
    out_cus       out: std::set<ColumnUsageInfo>
                       Column usage information parsed from fitem.
*/
int parse_column_from_func_item(sql_operation op, Item_func *fitem,
                                std::set<ColumnUsageInfo> &out_cus);

/*
  parse_column_from_cond_item
    Helper to parse column usage information corresponding to a single
    conditional item.
  Input:
    op                 in: sql_operation
                           The SQL operation FILTER, TABLE_JOIN etc.
                           corresponding to the conditional item.
    citem              in: Item_cond
                           The conditional item to be parsed.
    out_cus            out: std::set<ColumnUsageInfo>
                            Column usage information parsed from fitem.
    recursion_depth    in: int
                           Book-keeping variable to prevent infinite recursion.
                           To be removed later.
*/
int parse_column_from_cond_item(sql_operation op, Item_cond *citem,
                                std::set<ColumnUsageInfo> &out_cus,
                                int recursion_depth);

/*
  parse_column_from_item
    Helper to parse column usage information corresponding to a single item.
  Input:
    op                 in: sql_operation
                           The SQL operation FILTER, TABLE_JOIN etc.
                           corresponding to the generic item being parsed.
    item               in: Item
                           The item to be parsed.
    out_cus            out: std::set<ColumnUsageInfo>
                            Column usage information parsed from fitem.
    recursion_depth    in: int
                           Book-keeping variable to prevent infinite recursion.
                           To be removed later.
*/
int parse_column_from_item(sql_operation op, Item *item,
                           std::set<ColumnUsageInfo> &out_cus,
                           int recursion_depth);

/*
  parse_columns_from_order_list
    Helper to parse column usage information corresponding to an ordered list
    of columns. This is used for ORDER BY and GROUP BY clauses.
  Input:
    op                 in: sql_operation
                           The operation GROUP BY or ORDER_BY which corresponds
                           to the list being processed.
    first_col          in: ORDER*
                           Pointer to the first column being parsed.
    out_cus            out: std::set<ColumnUsageInfo>
                            Column usage information parsed from fitem.
*/
int parse_columns_from_order_list(sql_operation op, ORDER *first_col,
                                  std::set<ColumnUsageInfo> &out_cus);

/*
  parse_column_usage_info
    Parses column usage information from the parse tree before execution of the
    query.
  Input:
    thd        in: THD
    out_cus    out: std::set<ColumnUsageInfo>
                    Column usage info derived from the parse tree.
*/
extern int parse_column_usage_info(THD *thd);

/*
  exists_column_usage_info
    Returns TRUE if we already collected column usage statistics for the SQL
    statement
  Input:
    thd        in: THD
*/
extern bool exists_column_usage_info(THD *thd);

/*
  populate_column_usage_info
    Populates column usage information into the temporary table data structures.
    This information was derived in `parse_column_usage_info`.
  Input:
    thd        in: THD
    cus        in: std::set<ColumnUsageInfo>
                   A set of column usage info structs to populate into the
                   temporary table (column_statistics) data structure.
                   NOTE: This parameter is acquired by the callee and cannot
                   be used any further by the caller.
*/
extern void populate_column_usage_info(THD *thd);

/*
  get_all_column_statistics
    Returns all the rows in column_statistics from the internal data
    structures.
*/
extern std::vector<column_statistics_row> get_all_column_statistics();

/*
  free_column_stats
    Evicts column stats from col_statistics_map.
*/
extern void free_column_stats();

/* Global variable to control collecting column statistics */
extern ulong column_stats_control;

// Read-write lock to make the unordered map storing column usage, threadsafe.
extern mysql_rwlock_t LOCK_column_statistics;

// Mapping from SQL_ID to all the column usage information.
extern std::unordered_map<digest_key, std::set<ColumnUsageInfo>>
    col_statistics_map;
