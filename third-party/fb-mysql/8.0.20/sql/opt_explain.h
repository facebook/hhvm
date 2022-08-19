/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef OPT_EXPLAIN_INCLUDED
#define OPT_EXPLAIN_INCLUDED

/**
  @file sql/opt_explain.h
  EXPLAIN @<command@>.

Single table UPDATE/DELETE commands are explained by the
explain_single_table_modification() function.

A query expression (complete SELECT query possibly including
subqueries and unions), INSERT...SELECT and multitable UPDATE/DELETE
commands are explained like this:

(1) explain_query_expression()

Is the entry point. Forwards the job to explain_unit().

(2) explain_unit()

Is for a SELECT_LEX_UNIT, prepares, optimizes, explains one JOIN for
each "top-level" SELECT_LEXs of the unit (like: all SELECTs of a
UNION; but not subqueries), and one JOIN for the fake SELECT_LEX of
UNION); each JOIN explain (JOIN::exec()) calls explain_query_specification()

(3) explain_query_specification()

Is for a single SELECT_LEX (fake or not). It needs a prepared and
optimized JOIN, for which it builds the EXPLAIN rows. But it also
launches the EXPLAIN process for "inner units" (==subqueries of this
SELECT_LEX), by calling explain_unit() for each of them.
*/

#include <string>
#include <vector>

#include "my_base.h"
#include "my_sqlcommand.h"
#include "my_thread_local.h"
#include "sql/opt_explain_format.h"
#include "sql/parse_tree_node_base.h"
#include "sql/query_result.h"  // Query_result_send
#include "sql/row_iterator.h"
#include "sql/sql_cmd.h"  // Sql_cmd
#include "sql/sql_lex.h"
#include "sys/types.h"

#include <functional>
#include <string>

class Item;
class JOIN;
class QEP_TAB;
class SELECT_LEX;
class SELECT_LEX_UNIT;
class THD;
struct TABLE;
template <class T>
class List;

extern const char *join_type_str[];

/** Table modification plan for JOIN-less statements (update/delete) */
class Modification_plan {
 public:
  THD *const thd;  ///< Owning thread
  const enum_mod_type
      mod_type;  ///< Modification type - MT_INSERT/MT_UPDATE/etc
  TABLE *table;  ///< Table to modify

  QEP_TAB *tab;               ///< QUICK access method + WHERE clause
  uint key;                   ///< Key to use
  ha_rows limit;              ///< Limit
  bool need_tmp_table;        ///< Whether tmp table needs to be used
  bool need_sort;             ///< Whether to use filesort
  bool used_key_is_modified;  ///< Whether the key used to scan is modified
  const char *message;        ///< Arbitrary message
  bool zero_result;           ///< true <=> plan will not be executed
  ha_rows examined_rows;  ///< # of rows expected to be examined in the table

  Modification_plan(THD *thd_arg, enum_mod_type mt, QEP_TAB *qep_tab,
                    uint key_arg, ha_rows limit_arg, bool need_tmp_table_arg,
                    bool need_sort_arg, bool used_key_is_modified_arg,
                    ha_rows rows);

  Modification_plan(THD *thd_arg, enum_mod_type mt, TABLE *table_arg,
                    const char *message_arg, bool zero_result_arg,
                    ha_rows rows);

  ~Modification_plan();

 private:
  void register_in_thd();
};

/**
  EXPLAIN functionality for Query_result_insert, Query_result_update and
  Query_result_delete.

  This class objects substitute Query_result_insert, Query_result_update and
  Query_result_delete data interceptor objects to implement EXPLAIN for INSERT,
  REPLACE and multi-table UPDATE and DELETE queries.
  Query_result_explain class object initializes tables like Query_result_insert,
  Query_result_update or Query_result_delete data interceptor do, but it
  suppresses table data modification by the underlying interceptor object.
  Thus, we can use Query_result_explain object in the context of EXPLAIN INSERT/
  REPLACE/UPDATE/DELETE query like we use Query_result_send in the context of
  EXPLAIN SELECT command:
  1) in presence of lex->describe flag, pass Query_result_explain object to
     execution function,
  2) it calls prepare(), optimize() and start_execution() functions
     to mark modified tables etc.
*/

class Query_result_explain final : public Query_result_send {
 protected:
  /**
    Pointer to underlying Query_result_insert, Query_result_update or
    Query_result_delete object.
  */
  Query_result *interceptor;

 public:
  Query_result_explain(SELECT_LEX_UNIT *unit_arg, Query_result *interceptor_arg)
      : Query_result_send(), interceptor(interceptor_arg) {
    unit = unit_arg;
  }

 protected:
  bool prepare(THD *thd, List<Item> &list, SELECT_LEX_UNIT *u) override {
    return Query_result_send::prepare(thd, list, u) ||
           interceptor->prepare(thd, list, u);
  }

  bool start_execution(THD *thd) override {
    return Query_result_send::start_execution(thd) ||
           interceptor->start_execution(thd);
  }

  bool optimize() override {
    return Query_result_send::optimize() || interceptor->optimize();
  }

  void cleanup(THD *thd) override {
    Query_result_send::cleanup(thd);
    interceptor->cleanup(thd);
  }
};

bool explain_no_table(THD *explain_thd, const THD *query_thd,
                      SELECT_LEX *select_lex, const char *message,
                      enum_parsing_context ctx);
bool explain_single_table_modification(THD *explain_thd, const THD *query_thd,
                                       const Modification_plan *plan,
                                       SELECT_LEX *select);
bool explain_query(THD *explain_thd, const THD *query_thd,
                   SELECT_LEX_UNIT *unit);
bool explain_query_specification(THD *explain_thd, const THD *query_thd,
                                 SELECT_LEX *select_lex,
                                 enum_parsing_context ctx);

class Sql_cmd_explain_other_thread final : public Sql_cmd {
 public:
  explicit Sql_cmd_explain_other_thread(my_thread_id thread_id)
      : m_thread_id(thread_id) {}

  enum_sql_command sql_command_code() const override {
    return SQLCOM_EXPLAIN_OTHER;
  }

  bool execute(THD *thd) override;

 private:
  /// connection_id in EXPLAIN FOR CONNECTION \<connection_id\>
  my_thread_id m_thread_id;
};

// Print out an iterator and all of its children (if any) in a tree.
// "level" is the current indenting level, as this is called recursively.
std::string PrintQueryPlan(int level, RowIterator *iterator);

// For each subselect within the given item, call the given functor
// with its SELECT number, dependent/cacheable status and an iterator.
void ForEachSubselect(
    Item *parent_item,
    const std::function<void(int select_number, bool is_dependent,
                             bool is_cacheable, RowIterator *iterator)>
        &callback);

// For the given join, return a list of pseudo-children corresponding to
// subselects in the SELECT list (if any).
std::vector<RowIterator::Child> GetIteratorsFromSelectList(JOIN *join);

#endif /* OPT_EXPLAIN_INCLUDED */
