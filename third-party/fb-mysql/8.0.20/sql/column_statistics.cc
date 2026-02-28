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

#include "sql/column_statistics.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "sql/item_subselect.h"
#include "sql/nested_join.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_info.h"
#include "sql/sql_lex.h"
#include "storage/perfschema/table_column_statistics.h"

/*
  column_statistics
  Associates a SQL Id with a set of columns that were used in the query.
  Captures column identification information such as database, table name etc.
  and information about how the column was used, for eg. operation, operator
  etc.
*/

// Mapping from SQL_ID to column usage information.
std::unordered_map<digest_key, std::set<ColumnUsageInfo>> col_statistics_map;

// Operator definition for strict weak ordering. Should be a function of all
// constituents of the struct. The ordering is done for ColumnUsageInfo structs
// in lexicographic fashion on the following elements.
// TABLE_SCHEMA, TABLE_NAME, TABLE_INSTANCE, COLUMN_NAME, SQL_OPERATION,
// OPERATOR_TYPE
// For eg. (xdb_mzait, tab1, instance1, col1, FILTER, GREATER_THAN) comes
// before (xdb_ritwik, tab1, instance2, col1, FILTER, GREATER_THAN)
// because the table schema `xdb_mzait` is lexicographically ordered before
// `xdb_ritwik`.
bool ColumnUsageInfo::operator<(const ColumnUsageInfo &other) const {
  if (table_schema.compare(other.table_schema) < 0) {
    return true;
  } else if (table_schema.compare(other.table_schema) == 0) {
    if (table_name.compare(other.table_name) < 0) {
      return true;
    } else if (table_name.compare(other.table_name) == 0) {
      if (table_instance.compare(other.table_instance) < 0) {
        return true;
      } else if (table_instance.compare(other.table_instance) == 0) {
        if (column_name.compare(other.column_name) < 0) {
          return true;
        } else if (column_name.compare(other.column_name) == 0) {
          if (sql_op < other.sql_op) {
            return true;
          } else if (sql_op == other.sql_op) {
            return op_type < other.op_type;
          }
        }
      }
    }
  }
  return false;
}

std::string sql_operation_string(const sql_operation &sql_op) {
  switch (sql_op) {
    case sql_operation::FILTER:
      return "FILTER";
    case sql_operation::TABLE_JOIN:
      return "TABLE_JOIN";
    case sql_operation::ORDER_BY:
      return "ORDER_BY";
    case sql_operation::GROUP_BY:
      return "GROUP_BY";
    default:
      // Asserting in debug mode since this code path implies that we've missed
      // some SQL operation.
      DBUG_ASSERT(true);
      return "";
  }
}

std::string operator_type_string(const operator_type &op_type) {
  switch (op_type) {
    case operator_type::BETWEEN:
      return "BETWEEN";
    case operator_type::EQUAL:
      return "EQUAL";
    case operator_type::NULLSAFE_EQUAL:
      return "NULLSAFE_EQUAL";
    case operator_type::LESS_THAN:
      return "LESS_THAN";
    case operator_type::LESS_THAN_EQUAL:
      return "LESS_THAN_EQUAL";
    case operator_type::NULL_CHECK:
      return "NULL_CHECK";
    case operator_type::GREATER_THAN:
      return "GREATER_THAN";
    case operator_type::GREATER_THAN_EQUAL:
      return "GREATER_THAN_EQUAL";
    case operator_type::NOT_EQUAL:
      return "NOT_EQUAL";
    case operator_type::SET_MEMBERSHIP:
      return "SET_MEMBERSHIP";
    case operator_type::PATTERN_MATCH:
      return "PATTERN_MATCH";
    case operator_type::SORT_ASCENDING:
      return "SORT_ASCENDING";
    case operator_type::SORT_DESCENDING:
      return "SORT_DESCENDING";
    case operator_type::UNKNOWN_OPERATOR:
    default:
      // Asserting in debug mode since this code path implies that we've missed
      // some operator.
      DBUG_ASSERT(true);
      return "UNKNOWN_OPERATOR";
  }
}

operator_type match_op(Item_func::Functype fitem_type) {
  switch (fitem_type) {
    case Item_func::BETWEEN:
      return operator_type::BETWEEN;
    case Item_func::EQ_FUNC:
      return operator_type::EQUAL;
    case Item_func::EQUAL_FUNC:
      return operator_type::NULLSAFE_EQUAL;
    case Item_func::LIKE_FUNC:
      return operator_type::PATTERN_MATCH;
    case Item_func::LT_FUNC:
      return operator_type::LESS_THAN;
    case Item_func::LE_FUNC:
      return operator_type::LESS_THAN_EQUAL;
    case Item_func::ISNULL_FUNC:
    case Item_func::ISNOTNULL_FUNC:
      return operator_type::NULL_CHECK;
    case Item_func::GT_FUNC:
      return operator_type::GREATER_THAN;
    case Item_func::GE_FUNC:
      return operator_type::GREATER_THAN_EQUAL;
    case Item_func::NE_FUNC:
      return operator_type::NOT_EQUAL;
    default:
      // Asserting in debug mode since this code path implies that we've missed
      // some operator.
      DBUG_ASSERT(true);
      break;
  }
  return operator_type::UNKNOWN_OPERATOR;
}

operator_type match_op(enum_order direction) {
  switch (direction) {
    case enum_order::ORDER_ASC:
      return operator_type::SORT_ASCENDING;
    case enum_order::ORDER_DESC:
      return operator_type::SORT_DESCENDING;
    case enum_order::ORDER_NOT_RELEVANT:
    default:
      return operator_type::UNKNOWN_OPERATOR;
  }
}

void fetch_table_info(Item_field *field_arg, ColumnUsageInfo *cui) {
  DBUG_ENTER("fetch_table_info");
  DBUG_ASSERT(field_arg);
  DBUG_ASSERT(cui);

  // `orig_table` and associated parameters maybe absent for
  // derived / temp tables.
  if (field_arg->field && field_arg->field->orig_table &&
      field_arg->field->orig_table->pos_in_table_list) {
    TABLE_LIST *tl = field_arg->field->orig_table->pos_in_table_list;
    cui->table_name = tl->get_table_name();

    // Table instance in column statistics is just a string representation
    // of the TABLE_LIST object corresponding to the table instance. We could
    // have used an integer to represent it but the order is immaterial.
    // The only property that matters is distinctness of the following tuple.
    // sql_id, table_schema, table_name, table_instance
    std::stringstream tl_addr;
    tl_addr << (void const *)tl;
    cui->table_instance = tl_addr.str();
  }

  DBUG_VOID_RETURN;
}

void populate_field_info(const sql_operation &op, const operator_type &op_type,
                         Item_field *field_arg,
                         std::set<ColumnUsageInfo> &out_cus) {
  // This is a helper to parse column usage information for one Item_field.
  // It relies on the prepare stage to resolve column names to their respective
  // tables and database. Derived and temporary tables do NOT have db_name.

  DBUG_ENTER("populate_field_info");
  DBUG_ASSERT(field_arg);

  // We do not populate column usage information if we weren't able to resolve
  // the table_schema (database). This is particularly true for derived tables
  // where indexing cannot help.
  std::string db_name = (field_arg->db_name) ? field_arg->db_name : "";
  if (db_name == "") {
    DBUG_VOID_RETURN;
  }

  ColumnUsageInfo cui;
  cui.sql_op = op;
  cui.op_type = op_type;
  cui.table_schema = db_name;
  fetch_table_info(field_arg, &cui);
  cui.column_name = (field_arg->field_name) ? field_arg->field_name : "";

  // This condition should never trigger because for non-base tables, db_name
  // would be empty as well. However, adding this check to be absolutely
  // certain.
  if (cui.table_name == "") {
    DBUG_ASSERT(false);
    DBUG_VOID_RETURN;
  }

  out_cus.insert(cui);

  DBUG_VOID_RETURN;
}

int parse_column_from_func_item(sql_operation op, Item_func *fitem,
                                std::set<ColumnUsageInfo> &out_cus) {
  DBUG_ENTER("parse_column_from_func_item");
  DBUG_ASSERT(fitem);

  const auto type = fitem->functype();

  switch (type) {
    case Item_func::IN_FUNC: {
      const auto args = fitem->arguments();

      // If a field item is present in the item function, it has to be the
      // first arg in the IN operator.
      if (args[0]->type() == Item::FIELD_ITEM) {
        Item_field *field_arg = static_cast<Item_field *>(args[0]);

        // Populating ColumnUsageInfo struct
        populate_field_info(op, operator_type::SET_MEMBERSHIP, field_arg,
                            out_cus);
      }
      break;
    }
    case Item_func::BETWEEN:
    case Item_func::EQ_FUNC:
    case Item_func::EQUAL_FUNC:
    case Item_func::ISNULL_FUNC:
    case Item_func::ISNOTNULL_FUNC:
    case Item_func::LT_FUNC:
    case Item_func::LE_FUNC:
    case Item_func::GE_FUNC:
    case Item_func::GT_FUNC:
    case Item_func::NE_FUNC: {
      const auto args = fitem->arguments();

      // Populating ColumnUsageInfo structs
      for (uint i = 0; i < fitem->argument_count(); i++) {
        if (args[i]->type() == Item::FIELD_ITEM) {
          Item_field *field_arg = static_cast<Item_field *>(args[i]);
          populate_field_info(op, match_op(type), field_arg, out_cus);
        }
      }

      break;
    }
    case Item_func::LIKE_FUNC: {
      DBUG_ASSERT(fitem->argument_count() == 2);
      const auto args = fitem->arguments();

      // Populating ColumnUsageInfo structs
      // Clause col1 LIKE 'pr%' uses an index but 'prn' LIKE col1 doesn't.
      if (fitem->argument_count() == 2 && args[0]->type() == Item::FIELD_ITEM &&
          args[1]->type() == Item::STRING_ITEM) {
        Item_field *field_arg = static_cast<Item_field *>(args[0]);
        Item_string *string_arg = static_cast<Item_string *>(args[1]);

        // The passed in argument isn't really used but following the
        // convention seen elsewhere. The lifetime semantics of String objects
        // seem dangerous for anything non-trivial.
        // More details in sql_string.h
        String tmp1, *tmp2;
        tmp2 = string_arg->val_str(&tmp1);
        if (!tmp2->is_empty() && tmp2->ptr()[0] != '%' &&
            tmp2->ptr()[0] != '_') {
          populate_field_info(op, match_op(type), field_arg, out_cus);
        }
      }

      break;
    }
    default: {
      // Asserting in debug mode in case we reach this codepath which signifies
      // that we've missed some function type.
      DBUG_ASSERT(true);
      break;
    }
  }
  DBUG_RETURN(0);
}

int parse_column_from_cond_item(sql_operation op, Item_cond *citem,
                                std::set<ColumnUsageInfo> &out_cus,
                                int recursion_depth) {
  DBUG_ENTER("parse_column_from_cond_item");
  DBUG_ASSERT(citem);

  switch (citem->functype()) {
    case Item_func::COND_AND_FUNC:
    case Item_func::COND_OR_FUNC: {
      List<Item> *arg_list = citem->argument_list();
      Item *item;
      List_iterator_fast<Item> li(*arg_list);
      while ((item = li++)) {
        parse_column_from_item(op, item, out_cus, recursion_depth + 1);
      }
      break;
    }
    default: {
      // Asserting in debug mode in case we reach this codepath which signifies
      // that we've missed some conditional type.
      DBUG_ASSERT(true);
      break;
    }
  }
  DBUG_RETURN(0);
}

int parse_column_from_item(sql_operation op, Item *item,
                           std::set<ColumnUsageInfo> &out_cus,
                           int recursion_depth) {
  DBUG_ENTER("parse_column_from_item");
  DBUG_ASSERT(item);

  // This is just a sanity check to detect infinite recursion. We will remove it
  // once the code matures. There is no scientific reason behind choosing 15
  // as a limit; it should be high enough for programmatically generated
  // queries.
  if (recursion_depth >= 15) {
    // Killing the server only when debugger is attached. In production, we will
    // just return.
    DBUG_EXECUTE_IF("crash_excessive_recursion_column_stats", DBUG_SUICIDE(););
    DBUG_RETURN(-1);
  }

  switch (item->type()) {
    case Item::COND_ITEM: {
      Item_cond *citem = static_cast<Item_cond *>(item);
      parse_column_from_cond_item(op, citem, out_cus, recursion_depth);
      break;
    }
    case Item::FUNC_ITEM: {
      Item_func *fitem = static_cast<Item_func *>(item);
      parse_column_from_func_item(op, fitem, out_cus);
      break;
    }
    case Item::SUBSELECT_ITEM: {
      /**
       * The case of SUBSELECT_ITEM is special. Simple usage of the
       * IN operator results in a FUNC_ITEM which is an instatiation of
       * IN_FUNC. Example of simple usage is as follows.
       *   SELECT * FROM tbl1 WHERE col1 IN (2, 5, 7)
       * However, for cases where a subselect is involved like,
       *   SELECT * FROM tbl1 WHERE col1 IN (SELECT col2 FROM tbl2)
       * the parser treats it slightly differently and it results in an item
       * of type SUBSELECT_ITEM. Since we are only interested in parsing
       * columns from the IN operator, we only consider SUBSELECT_ITEM of
       * subtype IN_SUBS. Only the left expression for it is parsed, provided
       * it's a field item.
       * MySQL treats this differently for non-select statements
       * (like UPDATE, DELETE etc.) wherein the prepare stage replaces
       * Item_subselect by an UNKNOWN_FUNC item. More details:
       * 1. bug#17766653
       * 2. https://fburl.com/8qnjbage
       */
      Item_subselect *ssitem = static_cast<Item_subselect *>(item);
      if (ssitem->substype() == Item_subselect::IN_SUBS) {
        // Only parse subselects of type IN_SUBS which denotes usage of IN
        // operator.
        Item_in_subselect *in_predicate =
            static_cast<Item_in_subselect *>(ssitem);
        // The left expression if only parse if it is a single field and not a
        // complex expression.
        if (in_predicate->left_expr &&
            in_predicate->left_expr->type() == Item::FIELD_ITEM) {
          Item_field *field_arg =
              static_cast<Item_field *>(in_predicate->left_expr);
          populate_field_info(op, operator_type::SET_MEMBERSHIP, field_arg,
                              out_cus);
        }
      }
      break;
    }
    default:
      break;
  }
  DBUG_RETURN(0);
}

int parse_columns_from_order_list(sql_operation op, ORDER *first_col,
                                  std::set<ColumnUsageInfo> &out_cus) {
  DBUG_ENTER("parse_columns_from_order_list");

  // Return early if the column is null.
  if (!first_col) {
    DBUG_RETURN(0);
  }

  // Iterate over the linked list of columns.
  for (ORDER *order_obj = first_col; order_obj && order_obj->item;
       order_obj = order_obj->next) {
    // Skip if there is no item corresponding to the column or if the
    // item type is not FIELD_ITEM (representing a FIELD).
    if (*(order_obj->item) == nullptr ||
        (*(order_obj->item))->type() != Item::FIELD_ITEM) {
      continue;
    }

    Item_field *field_arg = static_cast<Item_field *>(*(order_obj->item));

    // Populating ColumnUsageInfo struct
    populate_field_info(op, match_op(order_obj->direction), field_arg, out_cus);
  }
  DBUG_RETURN(0);
}

int parse_column_usage_info(THD *thd) {
  DBUG_ENTER("parse_column_usage_info");
  DBUG_ASSERT(thd);

  // Return early without doing anything if
  // 1. COLUMN_STATS switch is not ON
  // 2. The SQL_ID was already processed
  if (column_stats_control != SQL_INFO_CONTROL_ON ||
      exists_column_usage_info(thd)) {
    DBUG_RETURN(0);
  }

  thd->column_usage_info.clear();

  LEX *lex = thd->lex;

  // List of supported commands for the collection of column_statistics.
  static const std::set<enum_sql_command> supported_commands = {
      SQLCOM_SELECT, SQLCOM_UPDATE, SQLCOM_DELETE, SQLCOM_INSERT_SELECT,
      SQLCOM_REPLACE_SELECT};

  // Check statement type - only commands featuring SIMPLE SELECTs.
  if (supported_commands.find(lex->sql_command) == supported_commands.end()) {
    DBUG_RETURN(0);
  }

  // There is a SELECT_LEX for each SELECT statement in a query. We iterate
  // over all of them and process them sequentially, extracting column usage
  // information from each such select / subselect.
  for (SELECT_LEX *select_lex = lex->all_selects_list; select_lex;
       select_lex = select_lex->next_select_in_list()) {
    if (select_lex->where_cond() != nullptr) {
      // Parsing column statistics from the WHERE clause.
      parse_column_from_item(sql_operation::FILTER, select_lex->where_cond(),
                             thd->column_usage_info, 0 /* recursion_depth */);
    }

    // Parsing column statistics from the ON clause in joins.
    for (TABLE_LIST *table = select_lex->leaf_tables; table;
         table = table->next_leaf) {
      TABLE_LIST *embedded; /* The table at the current level of nesting. */
      TABLE_LIST *embedding = table; /* The parent nested table reference. */
      do {
        embedded = embedding;
        if (embedded->join_cond()) {
          parse_column_from_item(sql_operation::TABLE_JOIN,
                                 embedded->join_cond(), thd->column_usage_info,
                                 0 /* recursion_depth */);
        }
        embedding = embedded->embedding;
      } while (embedding &&
               embedding->nested_join->join_list.front() == embedded);
    }

    // Parsing column statistics from the GROUP BY clause.
    parse_columns_from_order_list(sql_operation::GROUP_BY,
                                  select_lex->group_list.first,
                                  thd->column_usage_info);

    // Parsing column statistics from the ORDER BY clause.
    parse_columns_from_order_list(sql_operation::ORDER_BY,
                                  select_lex->order_list.first,
                                  thd->column_usage_info);
  }
  DBUG_RETURN(0);
}

void populate_column_usage_info(THD *thd) {
  DBUG_ENTER("populate_column_usage_info");
  DBUG_ASSERT(thd);

  // If the transaction wasn't successful, return.
  // Column usage statistics are not updated in this case.
  if (thd->is_error() ||
      (thd->variables.option_bits & OPTION_MASTER_SQL_ERROR)) {
    DBUG_VOID_RETURN;
  }

  /* Return early if any of the following true
    - the column usage information is empty
    - SQL_ID is not set
  */
  if (thd->column_usage_info.empty() || !thd->mt_key_is_set(THD::SQL_ID)) {
    DBUG_VOID_RETURN;
  }

  mysql_rwlock_wrlock(&LOCK_column_statistics);
  auto iter = col_statistics_map.find(thd->mt_key_value(THD::SQL_ID));
  if (iter == col_statistics_map.end()) {
    col_statistics_map.insert(
        std::make_pair(thd->mt_key_value(THD::SQL_ID), thd->column_usage_info));
  }
  mysql_rwlock_unlock(&LOCK_column_statistics);
  DBUG_VOID_RETURN;
}

bool exists_column_usage_info(THD *thd) {
  DBUG_ENTER("exists_column_usage_info");
  DBUG_ASSERT(thd);

  // return now if the SQL ID was not set
  if (!thd->mt_key_is_set(THD::SQL_ID)) {
    DBUG_RETURN(true);
  }

  mysql_rwlock_rdlock(&LOCK_column_statistics);
  bool exists = (col_statistics_map.find(thd->mt_key_value(THD::SQL_ID)) ==
                 col_statistics_map.end())
                    ? false
                    : true;
  mysql_rwlock_unlock(&LOCK_column_statistics);

  DBUG_RETURN(exists);
}

std::vector<column_statistics_row> get_all_column_statistics() {
  std::vector<column_statistics_row> column_statistics;

  mysql_rwlock_rdlock(&LOCK_column_statistics);
  for (auto iter = col_statistics_map.cbegin();
       iter != col_statistics_map.cend(); ++iter) {
    /* Generate the DIGEST string from the digest */
    char sql_id_string[DIGEST_HASH_TO_STRING_LENGTH + 1];
    DIGEST_HASH_TO_STRING(iter->first.data(), sql_id_string);
    sql_id_string[DIGEST_HASH_TO_STRING_LENGTH] = '\0';

    for (const ColumnUsageInfo &cui : iter->second) {
      column_statistics.emplace_back(
          sql_id_string,                       // SQL_ID
          cui.table_schema,                    // TABLE_SCHEMA
          cui.table_name,                      // TABLE_NAME
          cui.table_instance,                  // TABLE_INSTANCE
          cui.column_name,                     // COLUMN_NAME
          sql_operation_string(cui.sql_op),    // SQL_OPERATION
          operator_type_string(cui.op_type));  // OPERATOR_TYPE
    }
  }

  mysql_rwlock_unlock(&LOCK_column_statistics);
  return column_statistics;
}

void free_column_stats() {
  mysql_rwlock_wrlock(&LOCK_column_statistics);
  col_statistics_map.clear();
  mysql_rwlock_unlock(&LOCK_column_statistics);
}
