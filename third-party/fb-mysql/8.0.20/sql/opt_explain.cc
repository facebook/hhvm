/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/opt_explain.cc
  "EXPLAIN <command>" implementation.
*/

#include "sql/opt_explain.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <atomic>
#include <limits>
#include <string>
#include <vector>

#include "ft_global.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "scope_guard.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/derror.h"      // ER_THD
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_func.h"
#include "sql/item_subselect.h"
#include "sql/key.h"
#include "sql/mysqld.h"              // stage_explaining
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/opt_costmodel.h"
#include "sql/opt_explain_format.h"
#include "sql/opt_range.h"  // QUICK_SELECT_I
#include "sql/opt_trace.h"  // Opt_trace_*
#include "sql/protocol.h"
#include "sql/row_iterator.h"
#include "sql/sql_base.h"  // lock_tables
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_cmd.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_executor.h"
#include "sql/sql_join_buffer.h"  // JOIN_CACHE
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_parse.h"      // is_explainable_query
#include "sql/sql_partition.h"  // for make_used_partitions_str()
#include "sql/sql_select.h"
#include "sql/table.h"
#include "sql/table_function.h"  // Table_function
#include "sql/timing_iterator.h"
#include "sql_string.h"
#include "template_utils.h"

class Opt_trace_context;

using std::function;
using std::string;
using std::vector;

typedef qep_row::extra extra;

static bool mysql_explain_unit(THD *explain_thd, const THD *query_thd,
                               SELECT_LEX_UNIT *unit);

const char *join_type_str[] = {
    "UNKNOWN", "system", "const",    "eq_ref",      "ref",        "ALL",
    "range",   "index",  "fulltext", "ref_or_null", "index_merge"};

static const enum_query_type cond_print_flags =
    enum_query_type(QT_ORDINARY | QT_SHOW_SELECT_NUMBER);

/// First string: for regular EXPLAIN; second: for EXPLAIN CONNECTION
static const char *plan_not_ready[] = {"Not optimized, outer query is empty",
                                       "Plan isn't ready yet"};

static bool ExplainIterator(THD *ethd, const THD *query_thd,
                            SELECT_LEX_UNIT *unit);

/**
  A base for all Explain_* classes

  Explain_* classes collect and output EXPLAIN data.

  This class hierarchy is a successor of the old select_describe() function
  of 5.5.
*/

class Explain {
 protected:
  THD *const explain_thd;        ///< cached THD which runs the EXPLAIN command
  const THD *query_thd;          ///< THD which runs the query to be explained
  const CHARSET_INFO *const cs;  ///< cached pointer to system_charset_info
  /**
     Cached SELECT_LEX of the explained query. Used for all explained stmts,
     including single-table UPDATE (provides way to access ORDER BY of
     UPDATE).
  */
  SELECT_LEX *const select_lex;

  Explain_format *const fmt;          ///< shortcut for thd->lex->explain_format
  enum_parsing_context context_type;  ///< associated value for struct. explain

  bool order_list;  ///< if query block has ORDER BY

  const bool explain_other;  ///< if we explain other thread than us

 protected:
  class Lazy_condition : public Lazy {
    Item *const condition;

   public:
    Lazy_condition(Item *condition_arg) : condition(condition_arg) {}
    virtual bool eval(String *ret) {
      ret->length(0);
      if (condition) condition->print(current_thd, ret, cond_print_flags);
      return false;
    }
  };

  explicit Explain(enum_parsing_context context_type_arg, THD *explain_thd_arg,
                   const THD *query_thd_arg, SELECT_LEX *select_lex_arg)
      : explain_thd(explain_thd_arg),
        query_thd(query_thd_arg),
        cs(system_charset_info),
        select_lex(select_lex_arg),
        fmt(explain_thd->lex->explain_format),
        context_type(context_type_arg),
        order_list(false),
        explain_other(explain_thd != query_thd) {
    if (explain_other) query_thd->query_plan.assert_plan_is_locked_if_other();
  }

 public:
  virtual ~Explain() {}

  bool send();

  /**
     Tells if it is allowed to print the WHERE / GROUP BY / etc
     clauses.
  */
  bool can_print_clauses() const {
    /*
      Certain implementations of Item::print() modify the item, so cannot be
      called by another thread which does not own the item. Moreover, the
      owning thread may be modifying the item at this moment (example:
      Item_in_subselect::finalize_materialization_transform() is done
      at first execution of the subquery, which happens after the parent query
      has a plan, and affects how the parent query would be printed).
    */
    return !explain_other;
  }

 protected:
  /**
    Explain everything but subqueries
  */
  virtual bool shallow_explain();
  /**
    Explain the rest of things after the @c shallow_explain() call
  */
  bool explain_subqueries();
  bool mark_subqueries(Item *item, qep_row *destination);
  bool prepare_columns();

  /**
    Push a part of the "extra" column into formatter

    Traditional formatter outputs traditional_extra_tags[tag] as is.
    Hierarchical formatter outputs a property with the json_extra_tags[tag] name
    and a boolean value of true.

    @param      tag     type of the "extra" part

    @retval     false   Ok
    @retval     true    Error (OOM)
  */
  bool push_extra(Extra_tag tag) {
    extra *e = new (explain_thd->mem_root) extra(tag);
    return e == nullptr || fmt->entry()->col_extra.push_back(e);
  }

  /**
    Push a part of the "extra" column into formatter

    @param      tag     type of the "extra" part
    @param      arg     for traditional formatter: rest of the part text,
                        for hierarchical format: string value of the property

    @retval     false   Ok
    @retval     true    Error (OOM)
  */
  bool push_extra(Extra_tag tag, const String &arg) {
    if (arg.is_empty()) return push_extra(tag);
    extra *e =
        new (explain_thd->mem_root) extra(tag, arg.dup(explain_thd->mem_root));
    return !e || !e->data || fmt->entry()->col_extra.push_back(e);
  }

  /**
    Push a part of the "extra" column into formatter

    @param      tag     type of the "extra" part
    @param      arg     for traditional formatter: rest of the part text,
                        for hierarchical format: string value of the property

    NOTE: arg must be a long-living string constant.

    @retval     false   Ok
    @retval     true    Error (OOM)
  */
  bool push_extra(Extra_tag tag, const char *arg) {
    extra *e = new (explain_thd->mem_root) extra(tag, arg);
    return !e || fmt->entry()->col_extra.push_back(e);
  }

  /*
    Rest of the functions are overloadable functions, those calculate and fill
    "col_*" fields with Items for further sending as EXPLAIN columns.

    "explain_*" functions return false on success and true on error (usually
    OOM).
  */
  virtual bool explain_id();
  virtual bool explain_select_type();
  virtual bool explain_table_name() { return false; }
  virtual bool explain_partitions() { return false; }
  virtual bool explain_join_type() { return false; }
  virtual bool explain_possible_keys() { return false; }
  /** fill col_key and and col_key_len fields together */
  virtual bool explain_key_and_len() { return false; }
  virtual bool explain_ref() { return false; }
  /** fill col_rows and col_filtered fields together */
  virtual bool explain_rows_and_filtered() { return false; }
  virtual bool explain_extra() { return false; }
  virtual bool explain_modify_flags() { return false; }

 protected:
  /**
     Returns true if the WHERE, ORDER BY, GROUP BY, etc clauses can safely be
     traversed: it means that we can iterate through them (no element is
     added/removed/replaced); the internal details of an element can change
     though (in particular if that element is an Item_subselect).

     By default, if we are explaining another connection, this is not safe.
  */
  virtual bool can_walk_clauses() { return !explain_other; }
  virtual enum_parsing_context get_subquery_context(
      SELECT_LEX_UNIT *unit) const;
};

enum_parsing_context Explain::get_subquery_context(
    SELECT_LEX_UNIT *unit) const {
  return unit->get_explain_marker(query_thd);
}

/**
  Explain_no_table class outputs a trivial EXPLAIN row with "extra" column

  This class is intended for simple cases to produce EXPLAIN output
  with "No tables used", "No matching records" etc.
  Optionally it can output number of estimated rows in the "row"
  column.

  @note This class also produces EXPLAIN rows for inner units (if any).
*/

class Explain_no_table : public Explain {
 private:
  const char *message;  ///< cached "message" argument
  const ha_rows rows;   ///< HA_POS_ERROR or cached "rows" argument

 public:
  Explain_no_table(THD *explain_thd_arg, const THD *query_thd_arg,
                   SELECT_LEX *select_lex_arg, const char *message_arg,
                   enum_parsing_context context_type_arg = CTX_JOIN,
                   ha_rows rows_arg = HA_POS_ERROR)
      : Explain(context_type_arg, explain_thd_arg, query_thd_arg,
                select_lex_arg),
        message(message_arg),
        rows(rows_arg) {
    if (can_walk_clauses())
      order_list = (select_lex_arg->order_list.elements != 0);
  }

 protected:
  virtual bool shallow_explain();

  virtual bool explain_rows_and_filtered();
  virtual bool explain_extra();
  virtual bool explain_modify_flags();

 private:
  enum_parsing_context get_subquery_context(SELECT_LEX_UNIT *unit) const;
};

/**
  Explain_union_result class outputs EXPLAIN row for UNION
*/

class Explain_union_result : public Explain {
 public:
  Explain_union_result(THD *explain_thd_arg, const THD *query_thd_arg,
                       SELECT_LEX *select_lex_arg)
      : Explain(CTX_UNION_RESULT, explain_thd_arg, query_thd_arg,
                select_lex_arg) {
    /* it's a UNION: */
    DBUG_ASSERT(select_lex_arg ==
                select_lex_arg->master_unit()->fake_select_lex);
    // Use optimized values from fake_select_lex's join
    order_list = (select_lex_arg->join->order != nullptr);
    // A plan exists so the reads above are safe:
    DBUG_ASSERT(select_lex_arg->join->get_plan_state() != JOIN::NO_PLAN);
  }

 protected:
  virtual bool explain_id();
  virtual bool explain_table_name();
  virtual bool explain_join_type();
  virtual bool explain_extra();
  /* purecov: begin deadcode */
  virtual bool can_walk_clauses() {
    DBUG_ASSERT(0);  // UNION result can't have conditions
    return true;     // Because we know that we have a plan
  }
  /* purecov: end */
};

/**
  Common base class for Explain_join and Explain_table
*/

class Explain_table_base : public Explain {
 protected:
  const TABLE *table;
  /**
     The QEP_TAB which we are currently explaining. It is NULL for the
     inserted table in INSERT/REPLACE SELECT.
     @note that you should never read quick() or condition(), they may change
     under your feet without holding the mutex ; read quick_optim() and
     condition_optim() instead.
  */
  QEP_TAB *tab;
  Key_map usable_keys;

  Explain_table_base(enum_parsing_context context_type_arg,
                     THD *const explain_thd_arg, const THD *query_thd_arg,
                     SELECT_LEX *select_lex_arg = nullptr,
                     TABLE *const table_arg = nullptr)
      : Explain(context_type_arg, explain_thd_arg, query_thd_arg,
                select_lex_arg),
        table(table_arg),
        tab(nullptr) {}

  virtual bool explain_partitions();
  virtual bool explain_possible_keys();

  bool explain_key_parts(int key, uint key_parts);
  bool explain_key_and_len_quick(QUICK_SELECT_I *quick);
  bool explain_key_and_len_index(int key);
  bool explain_key_and_len_index(int key, uint key_length, uint key_parts);
  bool explain_extra_common(int quick_type, uint keyno);
  bool explain_tmptable_and_filesort(bool need_tmp_table_arg,
                                     bool need_sort_arg);
};

/**
  Explain_join class produces EXPLAIN output for JOINs
*/

class Explain_join : public Explain_table_base {
 private:
  bool need_tmp_table;  ///< add "Using temporary" to "extra" if true
  bool need_order;      ///< add "Using filesort"" to "extra" if true
  const bool distinct;  ///< add "Distinct" string to "extra" column if true

  JOIN *join;      ///< current JOIN
  int quick_type;  ///< current quick type, see anon. enum at QUICK_SELECT_I

 public:
  Explain_join(THD *explain_thd_arg, const THD *query_thd_arg,
               SELECT_LEX *select_lex_arg, bool need_tmp_table_arg,
               bool need_order_arg, bool distinct_arg)
      : Explain_table_base(CTX_JOIN, explain_thd_arg, query_thd_arg,
                           select_lex_arg),
        need_tmp_table(need_tmp_table_arg),
        need_order(need_order_arg),
        distinct(distinct_arg),
        join(select_lex_arg->join) {
    DBUG_ASSERT(join->get_plan_state() == JOIN::PLAN_READY);
    /* it is not UNION: */
    DBUG_ASSERT(join->select_lex != join->unit->fake_select_lex);
    order_list = (join->order != nullptr);
  }

 private:
  // Next 4 functions begin and end context for GROUP BY, ORDER BY and DISTINC
  bool begin_sort_context(Explain_sort_clause clause, enum_parsing_context ctx);
  bool end_sort_context(Explain_sort_clause clause, enum_parsing_context ctx);
  bool begin_simple_sort_context(Explain_sort_clause clause,
                                 enum_parsing_context ctx);
  bool end_simple_sort_context(Explain_sort_clause clause,
                               enum_parsing_context ctx);
  bool explain_qep_tab(size_t tab_num);

 protected:
  virtual bool shallow_explain();

  virtual bool explain_table_name();
  virtual bool explain_join_type();
  virtual bool explain_key_and_len();
  virtual bool explain_ref();
  virtual bool explain_rows_and_filtered();
  virtual bool explain_extra();
  virtual bool explain_select_type();
  virtual bool explain_id();
  virtual bool explain_modify_flags();
  virtual bool can_walk_clauses() {
    return true;  // Because we know that we have a plan
  }
};

/**
  Explain_table class produce EXPLAIN output for queries without top-level JOIN

  This class is a simplified version of the Explain_join class. It works in the
  context of queries which implementation lacks top-level JOIN object (EXPLAIN
  single-table UPDATE and DELETE).
*/

class Explain_table : public Explain_table_base {
 private:
  const uint key;                   ///< cached "key" number argument
  const ha_rows limit;              ///< HA_POS_ERROR or cached "limit" argument
  const bool need_tmp_table;        ///< cached need_tmp_table argument
  const bool need_sort;             ///< cached need_sort argument
  const enum_mod_type mod_type;     ///< Table modification type
  const bool used_key_is_modified;  ///< UPDATE command updates used key
  const char *message;              ///< cached "message" argument

 public:
  Explain_table(THD *const explain_thd_arg, const THD *query_thd_arg,
                SELECT_LEX *select_lex_arg, TABLE *const table_arg,
                QEP_TAB *tab_arg, uint key_arg, ha_rows limit_arg,
                bool need_tmp_table_arg, bool need_sort_arg,
                enum_mod_type mod_type_arg, bool used_key_is_modified_arg,
                const char *msg)
      : Explain_table_base(CTX_JOIN, explain_thd_arg, query_thd_arg,
                           select_lex_arg, table_arg),
        key(key_arg),
        limit(limit_arg),
        need_tmp_table(need_tmp_table_arg),
        need_sort(need_sort_arg),
        mod_type(mod_type_arg),
        used_key_is_modified(used_key_is_modified_arg),
        message(msg) {
    tab = tab_arg;
    usable_keys = table->possible_quick_keys;
    if (can_walk_clauses())
      order_list = (select_lex_arg->order_list.elements != 0);
  }

  virtual bool explain_modify_flags();

 private:
  virtual bool explain_tmptable_and_filesort(bool need_tmp_table_arg,
                                             bool need_sort_arg);
  virtual bool shallow_explain();

  virtual bool explain_ref();
  virtual bool explain_table_name();
  virtual bool explain_join_type();
  virtual bool explain_key_and_len();
  virtual bool explain_rows_and_filtered();
  virtual bool explain_extra();

  virtual bool can_walk_clauses() {
    return true;  // Because we know that we have a plan
  }
};

/* Explain class functions ****************************************************/

bool Explain::shallow_explain() {
  return prepare_columns() || fmt->flush_entry();
}

/**
  Qualify subqueries with WHERE/HAVING/ORDER BY/GROUP BY clause type marker

  @param item           Item tree to find subqueries
  @param destination    For WHERE clauses

  @note WHERE clause belongs to TABLE or QEP_TAB. The @c destination parameter
        provides a pointer to QEP data for such a table to associate a future
        subquery EXPLAIN output with table QEP provided.

  @retval false         OK
  @retval true          Error
*/

bool Explain::mark_subqueries(Item *item, qep_row *destination) {
  if (item == nullptr || !fmt->is_hierarchical()) return false;

  item->compile(&Item::explain_subquery_checker,
                reinterpret_cast<uchar **>(&destination),
                &Item::explain_subquery_propagator, nullptr);
  return false;
}

static bool explain_ref_key(Explain_format *fmt, uint key_parts,
                            store_key *key_copy[]) {
  if (key_parts == 0) return false;

  for (uint part_no = 0; part_no < key_parts; part_no++) {
    const store_key *const s_key = key_copy[part_no];
    if (s_key == nullptr) {
      // Const keys don't need to be copied
      if (fmt->entry()->col_ref.push_back(STORE_KEY_CONST_NAME))
        return true; /* purecov: inspected */
    } else if (fmt->entry()->col_ref.push_back(s_key->name()))
      return true; /* purecov: inspected */
  }
  return false;
}

enum_parsing_context Explain_no_table::get_subquery_context(
    SELECT_LEX_UNIT *unit) const {
  const enum_parsing_context context = Explain::get_subquery_context(unit);
  if (context == CTX_OPTIMIZED_AWAY_SUBQUERY) return context;
  if (context == CTX_DERIVED)
    return context;
  else if (message != plan_not_ready[explain_other])
    /*
      When zero result is given all subqueries are considered as optimized
      away.
    */
    return CTX_OPTIMIZED_AWAY_SUBQUERY;
  return context;
}

/**
  Traverses SQL clauses of this query specification to identify children
  subqueries, marks each of them with the clause they belong to.
  Then goes though all children subqueries and produces their EXPLAIN
  output, attached to the proper clause's context.

  @retval       false   Ok
  @retval       true    Error (OOM)
*/
bool Explain::explain_subqueries() {
  /*
    Subqueries in empty queries are neither optimized nor executed. They are
    therefore not to be included in the explain output.
  */
  if (select_lex->is_empty_query()) return false;

  for (SELECT_LEX_UNIT *unit = select_lex->first_inner_unit(); unit;
       unit = unit->next_unit()) {
    SELECT_LEX *sl = unit->first_select();
    enum_parsing_context context = get_subquery_context(unit);
    if (context == CTX_NONE) context = CTX_OPTIMIZED_AWAY_SUBQUERY;

    uint derived_clone_id = 0;
    bool is_derived_clone = false;
    if (context == CTX_DERIVED) {
      TABLE_LIST *tl = unit->derived_table;
      derived_clone_id = tl->query_block_id_for_explain();
      DBUG_ASSERT(derived_clone_id);
      is_derived_clone = derived_clone_id != tl->query_block_id();
      if (is_derived_clone && !fmt->is_hierarchical()) {
        // Don't show underlying tables of derived table clone
        continue;
      }
    }

    if (fmt->begin_context(context, unit)) return true;

    if (is_derived_clone) fmt->entry()->derived_clone_id = derived_clone_id;

    if (mysql_explain_unit(explain_thd, query_thd, unit)) return true;

    /*
      This must be after mysql_explain_unit() so that JOIN::optimize() has run
      and had a chance to choose materialization.
    */
    if (fmt->is_hierarchical() &&
        (context == CTX_WHERE || context == CTX_HAVING ||
         context == CTX_SELECT_LIST || context == CTX_GROUP_BY_SQ ||
         context == CTX_ORDER_BY_SQ) &&
        (!explain_other ||
         (sl->join && sl->join->get_plan_state() != JOIN::NO_PLAN)) &&
        // Check below requires complete plan
        unit->item &&
        (unit->item->engine_type() == Item_subselect::HASH_SJ_ENGINE)) {
      fmt->entry()->is_materialized_from_subquery = true;
      fmt->entry()->col_table_name.set_const("<materialized_subquery>");
      fmt->entry()->using_temporary = true;

      const QEP_TAB *const tmp_tab = unit->item->get_qep_tab();

      fmt->entry()->col_join_type.set_const(join_type_str[tmp_tab->type()]);
      fmt->entry()->col_key.set_const("<auto_key>");

      char buff_key_len[24];
      fmt->entry()->col_key_len.set(
          buff_key_len,
          longlong10_to_str(tmp_tab->table()->key_info[0].key_length,
                            buff_key_len, 10) -
              buff_key_len);

      if (explain_ref_key(fmt, tmp_tab->ref().key_parts,
                          tmp_tab->ref().key_copy))
        return true;

      fmt->entry()->col_rows.set(1);
      /*
       The value to look up depends on the outer value, so the materialized
       subquery is dependent and not cacheable:
      */
      fmt->entry()->is_dependent = true;
      fmt->entry()->is_cacheable = false;
    }

    if (fmt->end_context(context)) return true;
  }
  return false;
}

/**
  Pre-calculate table property values for further EXPLAIN output
*/
bool Explain::prepare_columns() {
  return explain_id() || explain_select_type() || explain_table_name() ||
         explain_partitions() || explain_join_type() ||
         explain_possible_keys() || explain_key_and_len() || explain_ref() ||
         explain_modify_flags() || explain_rows_and_filtered() ||
         explain_extra();
}

/**
  Explain class main function

  This function:
    a) allocates a Query_result_send object (if no one pre-allocated available),
    b) calculates and sends whole EXPLAIN data.

  @return false if success, true if error
*/

bool Explain::send() {
  DBUG_TRACE;

  if (fmt->begin_context(context_type, nullptr)) return true;

  /* Don't log this into the slow query log */
  explain_thd->server_status &=
      ~(SERVER_QUERY_NO_INDEX_USED | SERVER_QUERY_NO_GOOD_INDEX_USED);

  bool ret = shallow_explain() || explain_subqueries();

  if (!ret) ret = fmt->end_context(context_type);

  return ret;
}

bool Explain::explain_id() {
  if (select_lex->select_number < INT_MAX)
    fmt->entry()->col_id.set(select_lex->select_number);
  return false;
}

bool Explain::explain_select_type() {
  // ignore top-level SELECT_LEXes
  // Elaborate only when plan is ready
  if (select_lex->master_unit()->outer_select() && select_lex->join &&
      select_lex->join->get_plan_state() != JOIN::NO_PLAN) {
    fmt->entry()->is_dependent = select_lex->is_dependent();
    fmt->entry()->is_cacheable = select_lex->is_cacheable();
  }
  fmt->entry()->col_select_type.set(select_lex->type());
  return false;
}

/* Explain_no_table class functions *******************************************/

bool Explain_no_table::shallow_explain() {
  return (fmt->begin_context(CTX_MESSAGE) || Explain::shallow_explain() ||
          (can_walk_clauses() &&
           mark_subqueries(select_lex->where_cond(), fmt->entry())) ||
          fmt->end_context(CTX_MESSAGE));
}

bool Explain_no_table::explain_rows_and_filtered() {
  /* Don't print estimated # of rows in table for INSERT/REPLACE. */
  if (rows == HA_POS_ERROR || fmt->entry()->mod_type == MT_INSERT ||
      fmt->entry()->mod_type == MT_REPLACE)
    return false;
  fmt->entry()->col_rows.set(rows);
  return false;
}

bool Explain_no_table::explain_extra() {
  return fmt->entry()->col_message.set(message);
}

bool Explain_no_table::explain_modify_flags() {
  switch (query_thd->query_plan.get_command()) {
    case SQLCOM_UPDATE_MULTI:
    case SQLCOM_UPDATE:
      fmt->entry()->mod_type = MT_UPDATE;
      break;
    case SQLCOM_DELETE_MULTI:
    case SQLCOM_DELETE:
      fmt->entry()->mod_type = MT_DELETE;
      break;
    case SQLCOM_INSERT_SELECT:
    case SQLCOM_INSERT:
      fmt->entry()->mod_type = MT_INSERT;
      break;
    case SQLCOM_REPLACE_SELECT:
    case SQLCOM_REPLACE:
      fmt->entry()->mod_type = MT_REPLACE;
      break;
    default:;
  }
  return false;
}

/* Explain_union_result class functions
 * ****************************************/

bool Explain_union_result::explain_id() { return false; }

bool Explain_union_result::explain_table_name() {
  // Get the last of UNION's selects
  SELECT_LEX *last_select =
      select_lex->master_unit()->first_select()->last_select();
  // # characters needed to print select_number of last select
  int last_length = (int)log10((double)last_select->select_number) + 1;

  SELECT_LEX *sl = select_lex->master_unit()->first_select();
  size_t len = 6, lastop = 0;
  char table_name_buffer[NAME_LEN];
  memcpy(table_name_buffer, STRING_WITH_LEN("<union"));
  /*
    - len + lastop: current position in table_name_buffer
    - 6 + last_length: the number of characters needed to print
      '...,'<last_select->select_number>'>\0'
  */
  for (; sl && len + lastop + 6 + last_length < NAME_CHAR_LEN;
       sl = sl->next_select()) {
    len += lastop;
    lastop = snprintf(table_name_buffer + len, NAME_CHAR_LEN - len, "%u,",
                      sl->select_number);
  }
  if (sl || len + lastop >= NAME_CHAR_LEN) {
    memcpy(table_name_buffer + len, STRING_WITH_LEN("...,"));
    len += 4;
    lastop = snprintf(table_name_buffer + len, NAME_CHAR_LEN - len, "%u,",
                      last_select->select_number);
  }
  len += lastop;
  table_name_buffer[len - 1] = '>';  // change ',' to '>'

  return fmt->entry()->col_table_name.set(table_name_buffer, len);
}

bool Explain_union_result::explain_join_type() {
  fmt->entry()->col_join_type.set_const(join_type_str[JT_ALL]);
  return false;
}

bool Explain_union_result::explain_extra() {
  if (!fmt->is_hierarchical()) {
    /*
     Currently we always use temporary table for UNION result
    */
    if (push_extra(ET_USING_TEMPORARY)) return true;
    /*
      here we assume that the query will return at least two rows, so we
      show "filesort" in EXPLAIN. Of course, sometimes we'll be wrong
      and no filesort will be actually done, but executing all selects in
      the UNION to provide precise EXPLAIN information will hardly be
      appreciated :)
    */
    if (order_list) {
      return push_extra(ET_USING_FILESORT);
    }
  }
  return Explain::explain_extra();
}

/* Explain_table_base class functions *****************************************/

bool Explain_table_base::explain_partitions() {
  if (table->part_info)
    return make_used_partitions_str(table->part_info,
                                    &fmt->entry()->col_partitions);
  return false;
}

bool Explain_table_base::explain_possible_keys() {
  if (usable_keys.is_clear_all()) return false;

  if ((table->file->ha_table_flags() & HA_NO_INDEX_ACCESS) != 0) return false;

  for (uint j = 0; j < table->s->keys; j++) {
    if (usable_keys.is_set(j) &&
        fmt->entry()->col_possible_keys.push_back(table->key_info[j].name))
      return true;
  }
  return false;
}

bool Explain_table_base::explain_key_parts(int key, uint key_parts) {
  KEY_PART_INFO *kp = table->key_info[key].key_part;
  for (uint i = 0; i < key_parts; i++, kp++)
    if (fmt->entry()->col_key_parts.push_back(
            get_field_name_or_expression(explain_thd, kp->field)))
      return true;
  return false;
}

bool Explain_table_base::explain_key_and_len_quick(QUICK_SELECT_I *quick) {
  bool ret = false;
  StringBuffer<512> str_key(cs);
  StringBuffer<512> str_key_len(cs);

  if (quick->index != MAX_KEY)
    ret = explain_key_parts(quick->index, quick->used_key_parts);
  quick->add_keys_and_lengths(&str_key, &str_key_len);
  return (ret || fmt->entry()->col_key.set(str_key) ||
          fmt->entry()->col_key_len.set(str_key_len));
}

bool Explain_table_base::explain_key_and_len_index(int key) {
  DBUG_ASSERT(key != MAX_KEY);
  return explain_key_and_len_index(key, table->key_info[key].key_length,
                                   table->key_info[key].user_defined_key_parts);
}

bool Explain_table_base::explain_key_and_len_index(int key, uint key_length,
                                                   uint key_parts) {
  DBUG_ASSERT(key != MAX_KEY);

  char buff_key_len[24];
  const KEY *key_info = table->key_info + key;
  const size_t length =
      longlong10_to_str(key_length, buff_key_len, 10) - buff_key_len;
  const bool ret = explain_key_parts(key, key_parts);
  return (ret || fmt->entry()->col_key.set(key_info->name) ||
          fmt->entry()->col_key_len.set(buff_key_len, length));
}

bool Explain_table_base::explain_extra_common(int quick_type, uint keyno) {
  if (((keyno != MAX_KEY && keyno == table->file->pushed_idx_cond_keyno &&
        table->file->pushed_idx_cond) ||
       (tab && tab->cache_idx_cond))) {
    StringBuffer<160> buff(cs);
    if (fmt->is_hierarchical() && can_print_clauses()) {
      if (table->file->pushed_idx_cond)
        table->file->pushed_idx_cond->print(explain_thd, &buff,
                                            cond_print_flags);
      else
        tab->cache_idx_cond->print(explain_thd, &buff, cond_print_flags);
    }
    if (push_extra(ET_USING_INDEX_CONDITION, buff))
      return true; /* purecov: inspected */
  }

  const TABLE *pushed_root = table->file->member_of_pushed_join();
  if (pushed_root && select_lex->join &&
      select_lex->join->get_plan_state() == JOIN::PLAN_READY) {
    char buf[128];
    size_t len;
    int pushed_id = 0;
    for (QEP_TAB *prev = select_lex->join->qep_tab; prev <= tab; prev++) {
      if (prev->table() == nullptr) continue;

      const TABLE *prev_root = prev->table()->file->member_of_pushed_join();
      if (prev_root == prev->table()) {
        pushed_id++;
        if (prev_root == pushed_root) break;
      }
    }
    if (pushed_root == table) {
      uint pushed_count = tab->table()->file->number_of_pushed_joins();
      len = snprintf(buf, sizeof(buf) - 1, "Parent of %d pushed join@%d",
                     pushed_count, pushed_id);
    } else {
      len = snprintf(buf, sizeof(buf) - 1, "Child of '%s' in pushed join@%d",
                     tab->table()->file->parent_of_pushed_join()->alias,
                     pushed_id);
    }

    {
      StringBuffer<128> buff(cs);
      buff.append(buf, len);
      if (push_extra(ET_PUSHED_JOIN, buff)) return true;
    }
  }

  switch (quick_type) {
    case QUICK_SELECT_I::QS_TYPE_ROR_UNION:
    case QUICK_SELECT_I::QS_TYPE_ROR_INTERSECT:
    case QUICK_SELECT_I::QS_TYPE_INDEX_MERGE: {
      StringBuffer<32> buff(cs);
      tab->quick_optim()->add_info_string(&buff);
      if (fmt->is_hierarchical()) {
        /*
          We are replacing existing col_key value with a quickselect info,
          but not the reverse:
        */
        DBUG_ASSERT(fmt->entry()->col_key.length);
        if (fmt->entry()->col_key.set(buff))  // keep col_key_len intact
          return true;
      } else {
        if (push_extra(ET_USING, buff)) return true;
      }
    } break;
    default:;
  }

  if (tab) {
    if (tab->table_ref && tab->table_ref->table_function) {
      StringBuffer<64> str(cs);
      str.append(tab->table_ref->table_function->func_name());

      if (push_extra(ET_TABLE_FUNCTION, str) || push_extra(ET_USING_TEMPORARY))
        return true;
    }
    if (tab->dynamic_range()) {
      StringBuffer<64> str(STRING_WITH_LEN("index map: 0x"), cs);
      /* 4 bits per 1 hex digit + terminating '\0' */
      char buf[MAX_KEY / 4 + 1];
      str.append(tab->keys().print(buf));
      if (push_extra(ET_RANGE_CHECKED_FOR_EACH_RECORD, str)) return true;
    } else if (tab->condition_optim()) {
      if (fmt->is_hierarchical() && can_print_clauses()) {
        Lazy_condition *c =
            new (explain_thd->mem_root) Lazy_condition(tab->condition_optim());
        if (c == nullptr) return true;
        fmt->entry()->col_attached_condition.set(c);
      } else if (push_extra(ET_USING_WHERE))
        return true;
    }

    const Item *pushed_cond = table->file->pushed_cond;
    if (pushed_cond) {
      StringBuffer<64> buff(cs);
      if (can_print_clauses())
        pushed_cond->print(explain_thd, &buff, cond_print_flags);
      if (push_extra(ET_USING_PUSHED_CONDITION, buff)) return true;
    }
    if (((quick_type >= 0 && tab->quick_optim()->reverse_sorted()) ||
         tab->reversed_access()) &&
        push_extra(ET_BACKWARD_SCAN))
      return true;
  }
  if (table->reginfo.not_exists_optimize && push_extra(ET_NOT_EXISTS))
    return true;

  if (quick_type == QUICK_SELECT_I::QS_TYPE_RANGE) {
    uint mrr_flags = ((QUICK_RANGE_SELECT *)(tab->quick_optim()))->mrr_flags;

    /*
      During normal execution of a query, multi_range_read_init() is
      called to initialize MRR. If HA_MRR_SORTED is set at this point,
      multi_range_read_init() for any native MRR implementation will
      revert to default MRR if not HA_MRR_SUPPORT_SORTED.
      Calling multi_range_read_init() can potentially be costly, so it
      is not done when executing an EXPLAIN. We therefore simulate
      its effect here:
    */
    if (mrr_flags & HA_MRR_SORTED && !(mrr_flags & HA_MRR_SUPPORT_SORTED))
      mrr_flags |= HA_MRR_USE_DEFAULT_IMPL;

    if (!(mrr_flags & HA_MRR_USE_DEFAULT_IMPL) && push_extra(ET_USING_MRR))
      return true;
  }

  if (tab && tab->type() == JT_FT &&
      (table->file->ha_table_flags() & HA_CAN_FULLTEXT_HINTS)) {
    /*
      Print info about FT hints.
    */
    StringBuffer<64> buff(cs);
    Ft_hints *ft_hints = tab->ft_func()->get_hints();
    bool not_first = false;
    if (ft_hints->get_flags() & FT_SORTED) {
      buff.append("sorted");
      not_first = true;
    } else if (ft_hints->get_flags() & FT_NO_RANKING) {
      buff.append("no_ranking");
      not_first = true;
    }
    if (ft_hints->get_op_type() != FT_OP_UNDEFINED &&
        ft_hints->get_op_type() != FT_OP_NO) {
      char buf[64];
      size_t len = 0;

      if (not_first) buff.append(", ");
      switch (ft_hints->get_op_type()) {
        case FT_OP_GT:
          len = snprintf(buf, sizeof(buf) - 1, "rank > %.0g",
                         ft_hints->get_op_value());
          break;
        case FT_OP_GE:
          len = snprintf(buf, sizeof(buf) - 1, "rank >= %.0g",
                         ft_hints->get_op_value());
          break;
        default:
          DBUG_ASSERT(0);
      }

      buff.append(buf, len, cs);
      not_first = true;
    }

    if (ft_hints->get_limit() != HA_POS_ERROR) {
      char buf[64];
      size_t len = 0;

      if (not_first) buff.append(", ");

      len =
          snprintf(buf, sizeof(buf) - 1, "limit = %llu", ft_hints->get_limit());
      buff.append(buf, len, cs);
      not_first = true;
    }
    if (not_first) push_extra(ET_FT_HINTS, buff);
  }

  /*
    EXPLAIN FORMAT=JSON FOR CONNECTION will mention clearly that index dive has
    been skipped.
  */
  if (explain_thd->lex->sql_command == SQLCOM_EXPLAIN_OTHER && tab &&
      fmt->is_hierarchical() && tab->skip_records_in_range())
    push_extra(ET_SKIP_RECORDS_IN_RANGE);

  return false;
}

bool Explain_table_base::explain_tmptable_and_filesort(bool need_tmp_table_arg,
                                                       bool need_sort_arg) {
  /*
    For hierarchical EXPLAIN we output "Using temporary" and
    "Using filesort" with related ORDER BY, GROUP BY or DISTINCT
  */
  if (fmt->is_hierarchical()) return false;

  if (need_tmp_table_arg && push_extra(ET_USING_TEMPORARY)) return true;
  if (need_sort_arg && push_extra(ET_USING_FILESORT)) return true;
  return false;
}

bool Explain_join::explain_modify_flags() {
  THD::Query_plan const *query_plan = &query_thd->query_plan;
  /*
    Because we are PLAN_READY, the following data structures are not changing
    and thus are safe to read.
  */
  switch (query_plan->get_command()) {
    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
      if (table->pos_in_table_list->updating &&
          table->s->table_category != TABLE_CATEGORY_TEMPORARY)
        fmt->entry()->mod_type = MT_UPDATE;
      break;
    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
      if (table->pos_in_table_list->updating &&
          table->s->table_category != TABLE_CATEGORY_TEMPORARY)
        fmt->entry()->mod_type = MT_DELETE;
      break;
    case SQLCOM_INSERT_SELECT:
      if (table == query_plan->get_lex()->insert_table_leaf->table)
        fmt->entry()->mod_type = MT_INSERT;
      break;
    case SQLCOM_REPLACE_SELECT:
      if (table == query_plan->get_lex()->insert_table_leaf->table)
        fmt->entry()->mod_type = MT_REPLACE;
      break;
    default:;
  };
  return false;
}

/* Explain_join class functions ***********************************************/

bool Explain_join::begin_sort_context(Explain_sort_clause clause,
                                      enum_parsing_context ctx) {
  const Explain_format_flags *flags = &join->explain_flags;
  return (flags->get(clause, ESP_EXISTS) &&
          !flags->get(clause, ESP_IS_SIMPLE) &&
          fmt->begin_context(ctx, nullptr, flags));
}

bool Explain_join::end_sort_context(Explain_sort_clause clause,
                                    enum_parsing_context ctx) {
  const Explain_format_flags *flags = &join->explain_flags;
  return (flags->get(clause, ESP_EXISTS) &&
          !flags->get(clause, ESP_IS_SIMPLE) && fmt->end_context(ctx));
}

bool Explain_join::begin_simple_sort_context(Explain_sort_clause clause,
                                             enum_parsing_context ctx) {
  const Explain_format_flags *flags = &join->explain_flags;
  return (flags->get(clause, ESP_IS_SIMPLE) &&
          fmt->begin_context(ctx, nullptr, flags));
}

bool Explain_join::end_simple_sort_context(Explain_sort_clause clause,
                                           enum_parsing_context ctx) {
  const Explain_format_flags *flags = &join->explain_flags;
  return (flags->get(clause, ESP_IS_SIMPLE) && fmt->end_context(ctx));
}

bool Explain_join::shallow_explain() {
  qep_row *join_entry = fmt->entry();

  join_entry->col_read_cost.set(join->best_read);

  if (select_lex->is_recursive()) {
    /*
      This will add the "recursive" word to:
      - the block of the JOIN, in JSON format
      - the first table of the JOIN, in TRADITIONAL format.
    */
    if (push_extra(ET_RECURSIVE)) return true; /* purecov: inspected */
  }

  LEX const *query_lex = join->thd->query_plan.get_lex();
  if (query_lex->insert_table_leaf &&
      query_lex->insert_table_leaf->select_lex == join->select_lex) {
    table = query_lex->insert_table_leaf->table;
    /*
      The target table for INSERT/REPLACE doesn't actually belong to join,
      thus tab is set to NULL. But in order to print it we add it to the
      list of plan rows. Explain printing code (traditional/json) will deal with
      it.
    */
    tab = nullptr;
    if (fmt->begin_context(CTX_QEP_TAB) || prepare_columns() ||
        fmt->flush_entry() || fmt->end_context(CTX_QEP_TAB))
      return true; /* purecov: inspected */
  }

  if (begin_sort_context(ESC_ORDER_BY, CTX_ORDER_BY))
    return true; /* purecov: inspected */
  if (begin_sort_context(ESC_DISTINCT, CTX_DISTINCT))
    return true; /* purecov: inspected */

  qep_row *order_by_distinct = fmt->entry();
  qep_row *windowing = nullptr;

  if (join->m_windowing_steps) {
    if (begin_sort_context(ESC_WINDOWING, CTX_WINDOW))
      return true; /* purecov: inspected */

    windowing = fmt->entry();
    if (!fmt->is_hierarchical()) {
      /*
        TRADITIONAL prints nothing for window functions, except the use of a
        temporary table and a filesort.
      */
      push_warning(explain_thd, Sql_condition::SL_NOTE, ER_WINDOW_EXPLAIN_JSON,
                   ER_THD(explain_thd, ER_WINDOW_EXPLAIN_JSON));
    }
    windowing->m_windows = &select_lex->m_windows;
    if (join->windowing_cost > 0)
      windowing->col_read_cost.set(join->windowing_cost);
  }

  if (begin_sort_context(ESC_GROUP_BY, CTX_GROUP_BY))
    return true; /* purecov: inspected */

  qep_row *order_by_distinct_or_grouping = fmt->entry();

  if (join->sort_cost > 0.0) {
    /*
      This sort is for GROUP BY, ORDER BY, DISTINCT so we attach its cost to
      them, by checking which is in use. When there is no windowing, we ascribe
      this cost always to the GROUP BY, if there is one, since ORDER
      BY/DISTINCT sorts in those cases are elided, else to ORDER BY, or
      DISTINCT.  With windowing, both GROUP BY and ORDER BY/DISTINCT may carry
      sorting costs.
    */
    if (join->m_windowing_steps) {
      int atrs = 0;  // attribute sorting costs to pre-window and/or post-window
      if (order_by_distinct_or_grouping != windowing &&
          join->explain_flags.get(ESC_GROUP_BY, ESP_USING_FILESORT)) {
        // We have a group by: assign it cost iff is used sorting
        order_by_distinct_or_grouping->col_read_cost.set(join->sort_cost);
        atrs++;
      }
      if (order_by_distinct != join_entry &&
          (join->explain_flags.get(ESC_ORDER_BY, ESP_USING_FILESORT) ||
           join->explain_flags.get(ESC_DISTINCT, ESP_USING_FILESORT))) {
        order_by_distinct->col_read_cost.set(join->sort_cost);
        atrs++;
      }

      if (atrs == 2) {
        /*
          We do sorting twice because of intervening windowing sorts, so
          increase total correspondingly. It has already been added to
          best_read once in the optimizer.
        */
        join_entry->col_read_cost.set(join->best_read + join->sort_cost);
      }
    } else {
      /*
        Due to begin_sort_context() calls above, fmt->entry() returns another
        context than stored in join_entry.
      */
      DBUG_ASSERT(order_by_distinct_or_grouping != join_entry ||
                  !fmt->is_hierarchical());
      order_by_distinct_or_grouping->col_read_cost.set(join->sort_cost);
    }
  }

  if (begin_sort_context(ESC_BUFFER_RESULT, CTX_BUFFER_RESULT))
    return true; /* purecov: inspected */

  for (size_t t = 0, cnt = fmt->is_hierarchical() ? join->primary_tables
                                                  : join->tables;
       t < cnt; t++) {
    if (explain_qep_tab(t)) return true;
  }

  if (end_sort_context(ESC_BUFFER_RESULT, CTX_BUFFER_RESULT)) return true;
  if (end_sort_context(ESC_GROUP_BY, CTX_GROUP_BY)) return true;
  if (join->m_windowing_steps) {
    if (end_sort_context(ESC_WINDOWING, CTX_WINDOW))
      return true; /* purecov: inspected */
  }
  if (end_sort_context(ESC_DISTINCT, CTX_DISTINCT)) return true;
  if (end_sort_context(ESC_ORDER_BY, CTX_ORDER_BY)) return true;

  return false;
}

bool Explain_join::explain_qep_tab(size_t tabnum) {
  tab = join->qep_tab + tabnum;
  if (!tab->position()) return false;
  table = tab->table();
  usable_keys = tab->keys();
  usable_keys.merge(table->possible_quick_keys);
  quick_type = -1;

  if (tab->type() == JT_RANGE || tab->type() == JT_INDEX_MERGE) {
    DBUG_ASSERT(tab->quick_optim());
    quick_type = tab->quick_optim()->get_type();
  }

  if (tab->starts_weedout()) fmt->begin_context(CTX_DUPLICATES_WEEDOUT);

  const bool first_non_const = tabnum == join->const_tables;

  if (first_non_const) {
    if (begin_simple_sort_context(ESC_ORDER_BY, CTX_SIMPLE_ORDER_BY))
      return true;
    if (begin_simple_sort_context(ESC_DISTINCT, CTX_SIMPLE_DISTINCT))
      return true;
    if (begin_simple_sort_context(ESC_GROUP_BY, CTX_SIMPLE_GROUP_BY))
      return true;
  }

  Semijoin_mat_exec *const sjm = tab->sj_mat_exec();
  const enum_parsing_context c = sjm ? CTX_MATERIALIZATION : CTX_QEP_TAB;

  if (fmt->begin_context(c) || prepare_columns()) return true;

  fmt->entry()->query_block_id = table->pos_in_table_list->query_block_id();

  if (sjm) {
    if (sjm->is_scan) {
      fmt->entry()->col_rows.cleanup();  // TODO: set(something reasonable)
    } else {
      fmt->entry()->col_rows.set(1);
    }
  }

  if (fmt->flush_entry() ||
      (can_walk_clauses() &&
       mark_subqueries(tab->condition_optim(), fmt->entry())))
    return true;

  if (sjm && fmt->is_hierarchical()) {
    for (size_t sjt = sjm->inner_table_index, end = sjt + sjm->table_count;
         sjt < end; sjt++) {
      if (explain_qep_tab(sjt)) return true;
    }
  }

  if (fmt->end_context(c)) return true;

  if (first_non_const) {
    if (end_simple_sort_context(ESC_GROUP_BY, CTX_SIMPLE_GROUP_BY)) return true;
    if (end_simple_sort_context(ESC_DISTINCT, CTX_SIMPLE_DISTINCT)) return true;
    if (end_simple_sort_context(ESC_ORDER_BY, CTX_SIMPLE_ORDER_BY)) return true;
  }

  if (tab->finishes_weedout() && fmt->end_context(CTX_DUPLICATES_WEEDOUT))
    return true;

  return false;
}

/**
  Generates either usual table name or <derived#N>, and passes it to
  any given function for showing to the user.
  @param tr   Table reference
  @param fmt  EXPLAIN's format
  @param func Function receiving the name
  @returns true if error.
*/
static bool store_table_name(
    TABLE_LIST *tr, Explain_format *fmt,
    std::function<bool(const char *name, size_t len)> func) {
  char namebuf[NAME_LEN];
  size_t len = sizeof(namebuf);
  if (tr->query_block_id() && tr->is_view_or_derived() &&
      !fmt->is_hierarchical()) {
    /* Derived table name generation */
    len = snprintf(namebuf, len - 1, "<derived%u>",
                   tr->query_block_id_for_explain());
    return func(namebuf, len);
  } else {
    return func(tr->alias, strlen(tr->alias));
  }
}

bool Explain_join::explain_table_name() {
  return store_table_name(table->pos_in_table_list, fmt,
                          [&](const char *name, size_t len) {
                            return fmt->entry()->col_table_name.set(name, len);
                          });
}

bool Explain_join::explain_select_type() {
  if (tab && sj_is_materialize_strategy(tab->get_sj_strategy()))
    fmt->entry()->col_select_type.set(enum_explain_type::EXPLAIN_MATERIALIZED);
  else
    return Explain::explain_select_type();
  return false;
}

bool Explain_join::explain_id() {
  if (tab && sj_is_materialize_strategy(tab->get_sj_strategy()))
    fmt->entry()->col_id.set(tab->sjm_query_block_id());
  else
    return Explain::explain_id();
  return false;
}

bool Explain_join::explain_join_type() {
  const join_type j_t = tab ? tab->type() : JT_ALL;
  const char *str = join_type_str[j_t];
  if ((j_t == JT_EQ_REF || j_t == JT_REF || j_t == JT_REF_OR_NULL) &&
      join->unit->item) {
    /*
      For backward-compatibility, we have special presentation of "index
      lookup used for in(subquery)": we do not show "ref/etc", but
      "index_subquery/unique_subquery".
    */
    if (join->unit->item->engine_type() == Item_subselect::INDEXSUBQUERY_ENGINE)
      str = (j_t == JT_EQ_REF) ? "unique_subquery" : "index_subquery";
  }

  fmt->entry()->col_join_type.set_const(str);
  return false;
}

bool Explain_join::explain_key_and_len() {
  if (!tab) return false;
  if (tab->ref().key_parts)
    return explain_key_and_len_index(tab->ref().key, tab->ref().key_length,
                                     tab->ref().key_parts);
  else if (tab->type() == JT_INDEX_SCAN || tab->type() == JT_FT)
    return explain_key_and_len_index(tab->index());
  else if (tab->type() == JT_RANGE || tab->type() == JT_INDEX_MERGE ||
           ((tab->type() == JT_REF || tab->type() == JT_REF_OR_NULL) &&
            tab->quick_optim()))
    return explain_key_and_len_quick(tab->quick_optim());
  return false;
}

bool Explain_join::explain_ref() {
  if (!tab) return false;
  return explain_ref_key(fmt, tab->ref().key_parts, tab->ref().key_copy);
}

bool Explain_join::explain_rows_and_filtered() {
  if (!tab || tab->table_ref->schema_table) return false;

  POSITION *const pos = tab->position();

  if (explain_thd->lex->sql_command == SQLCOM_EXPLAIN_OTHER &&
      tab->skip_records_in_range()) {
    // Skipping col_rows, col_filtered, col_prefix_rows will set them to NULL.
    fmt->entry()->col_cond_cost.set(0);
    fmt->entry()->col_read_cost.set(0.0);
    fmt->entry()->col_prefix_cost.set(0);
    fmt->entry()->col_data_size_query.set("0");
  } else {
    fmt->entry()->col_rows.set(static_cast<ulonglong>(pos->rows_fetched));
    fmt->entry()->col_filtered.set(
        pos->rows_fetched
            ? static_cast<float>(100.0 * tab->position()->filter_effect)
            : 0.0f);

    // Print cost-related info
    double prefix_rows = pos->prefix_rowcount;
    ulonglong prefix_rows_ull =
        prefix_rows >=
                static_cast<double>(std::numeric_limits<ulonglong>::max())
            ? std::numeric_limits<ulonglong>::max()
            : static_cast<ulonglong>(prefix_rows);
    fmt->entry()->col_prefix_rows.set(prefix_rows_ull);
    double const cond_cost = join->cost_model()->row_evaluate_cost(prefix_rows);
    fmt->entry()->col_cond_cost.set(cond_cost < 0 ? 0 : cond_cost);
    fmt->entry()->col_read_cost.set(pos->read_cost < 0.0 ? 0.0
                                                         : pos->read_cost);
    fmt->entry()->col_prefix_cost.set(pos->prefix_cost);
    // Calculate amount of data from this table per query
    char data_size_str[32];
    double data_size = prefix_rows * tab->table()->s->rec_buff_length;
    human_readable_num_bytes(data_size_str, sizeof(data_size_str), data_size);
    fmt->entry()->col_data_size_query.set(data_size_str);
  }

  return false;
}

bool Explain_join::explain_extra() {
  if (!tab) return false;
  if (tab->type() == JT_SYSTEM && tab->position()->rows_fetched == 0.0) {
    if (push_extra(ET_CONST_ROW_NOT_FOUND))
      return true; /* purecov: inspected */
  } else if (tab->type() == JT_CONST && tab->position()->rows_fetched == 0.0) {
    if (push_extra(ET_UNIQUE_ROW_NOT_FOUND))
      return true; /* purecov: inspected */
  } else if (tab->type() == JT_CONST && tab->position()->rows_fetched == 1.0 &&
             tab->table()->has_null_row()) {
    if (push_extra(ET_IMPOSSIBLE_ON_CONDITION))
      return true; /* purecov: inspected */
  } else {
    uint keyno = MAX_KEY;
    if (tab->ref().key_parts)
      keyno = tab->ref().key;
    else if (tab->type() == JT_RANGE || tab->type() == JT_INDEX_MERGE)
      keyno = tab->quick_optim()->index;

    if (explain_extra_common(quick_type, keyno)) return true;

    if (((tab->type() == JT_INDEX_SCAN || tab->type() == JT_CONST) &&
         table->covering_keys.is_set(tab->index())) ||
        (quick_type == QUICK_SELECT_I::QS_TYPE_ROR_INTERSECT &&
         !((QUICK_ROR_INTERSECT_SELECT *)tab->quick_optim())
              ->need_to_fetch_row) ||
        /*
          Notice that table->key_read can change on the fly (grep
          for set_keyread); so EXPLAIN CONNECTION reads a changing variable,
          fortunately it's a bool and not a pointer and the consequences
          cannot be severe (at worst, wrong EXPLAIN).
        */
        table->key_read || tab->keyread_optim()) {
      if (quick_type == QUICK_SELECT_I::QS_TYPE_GROUP_MIN_MAX) {
        QUICK_GROUP_MIN_MAX_SELECT *qgs =
            (QUICK_GROUP_MIN_MAX_SELECT *)tab->quick_optim();
        StringBuffer<64> buff(cs);
        qgs->append_loose_scan_type(&buff);
        if (push_extra(ET_USING_INDEX_FOR_GROUP_BY, buff)) return true;
      } else if (quick_type == QUICK_SELECT_I::QS_TYPE_SKIP_SCAN) {
        if (push_extra(ET_USING_INDEX_FOR_SKIP_SCAN)) return true;
      } else {
        if (push_extra(ET_USING_INDEX)) return true;
      }
    }

    if (explain_tmptable_and_filesort(need_tmp_table, need_order)) return true;
    need_tmp_table = need_order = false;

    if (distinct && tab->not_used_in_distinct && push_extra(ET_DISTINCT))
      return true;

    if (tab->do_loosescan() && push_extra(ET_LOOSESCAN)) return true;

    if (tab->starts_weedout()) {
      if (!fmt->is_hierarchical() && push_extra(ET_START_TEMPORARY))
        return true;
    }
    if (tab->finishes_weedout()) {
      if (!fmt->is_hierarchical() && push_extra(ET_END_TEMPORARY)) return true;
    } else if (tab->do_firstmatch()) {
      if (tab->firstmatch_return == PRE_FIRST_PLAN_IDX) {
        if (push_extra(ET_FIRST_MATCH)) return true;
      } else {
        StringBuffer<64> buff(cs);
        if (store_table_name(join->qep_tab[tab->firstmatch_return].table_ref,
                             fmt,
                             [&](const char *name, size_t len) {
                               return buff.append(name, len);
                             }) ||
            push_extra(ET_FIRST_MATCH, buff))
          return true;
      }
    }

    if (tab->lateral_derived_tables_depend_on_me) {
      auto deps = tab->lateral_derived_tables_depend_on_me;
      StringBuffer<64> buff(cs);
      bool first = true;
      for (QEP_TAB **tab2 = join->map2qep_tab; deps; tab2++, deps >>= 1) {
        if (deps & 1) {
          if (!first) buff.append(",");
          first = false;
          if (store_table_name((*tab2)->table_ref, fmt,
                               [&](const char *name, size_t len) {
                                 return buff.append(name, len);
                               }))
            return true;
        }
      }
      if (push_extra(ET_REMATERIALIZE, buff)) return true;
    }

    if (tab->has_guarded_conds() && push_extra(ET_FULL_SCAN_ON_NULL_KEY))
      return true;

    if (tab->op_type == QEP_TAB::OT_BNL || tab->op_type == QEP_TAB::OT_BKA) {
      StringBuffer<64> buff(cs);
      if (tab->op_type == QEP_TAB::OT_BNL) {
        // BNL does not exist in the iterator executor, but is nearly
        // always rewritten to hash join, so use that in traditional EXPLAIN.
        buff.append("hash join");
      } else if (tab->op_type == QEP_TAB::OT_BKA)
        buff.append("Batched Key Access");
      else
        DBUG_ASSERT(0); /* purecov: inspected */
      if (push_extra(ET_USING_JOIN_BUFFER, buff)) return true;
    }
  }
  if (fmt->is_hierarchical() && (!bitmap_is_clear_all(table->read_set) ||
                                 !bitmap_is_clear_all(table->write_set))) {
    Field **fld;
    for (fld = table->field; *fld; fld++) {
      if (!bitmap_is_set(table->read_set, (*fld)->field_index) &&
          !bitmap_is_set(table->write_set, (*fld)->field_index))
        continue;

      const char *field_description =
          get_field_name_or_expression(explain_thd, *fld);
      fmt->entry()->col_used_columns.push_back(field_description);
      if (table->is_binary_diff_enabled(*fld))
        fmt->entry()->col_partial_update_columns.push_back(field_description);
    }
  }

  if (table->s->is_secondary_engine() &&
      push_extra(ET_USING_SECONDARY_ENGINE, table->file->table_type()))
    return true;

  return false;
}

/* Explain_table class functions **********************************************/

bool Explain_table::explain_modify_flags() {
  fmt->entry()->mod_type = mod_type;
  return false;
}

bool Explain_table::explain_tmptable_and_filesort(bool need_tmp_table_arg,
                                                  bool need_sort_arg) {
  if (fmt->is_hierarchical()) {
    /*
      For hierarchical EXPLAIN we output "using_temporary_table" and
      "using_filesort" with related ORDER BY, GROUP BY or DISTINCT
      (excluding the single-table UPDATE command that updates used key --
      in this case we output "using_temporary_table: for update"
      at the "table" node)
    */
    if (need_tmp_table_arg) {
      DBUG_ASSERT(used_key_is_modified || order_list);
      if (used_key_is_modified && push_extra(ET_USING_TEMPORARY, "for update"))
        return true;
    }
  } else {
    if (need_tmp_table_arg && push_extra(ET_USING_TEMPORARY)) return true;

    if (need_sort_arg && push_extra(ET_USING_FILESORT)) return true;
  }

  return false;
}

bool Explain_table::shallow_explain() {
  Explain_format_flags flags;
  if (order_list) {
    flags.set(ESC_ORDER_BY, ESP_EXISTS);
    if (need_sort) flags.set(ESC_ORDER_BY, ESP_USING_FILESORT);
    if (!used_key_is_modified && need_tmp_table)
      flags.set(ESC_ORDER_BY, ESP_USING_TMPTABLE);
  }

  if (order_list && fmt->begin_context(CTX_ORDER_BY, nullptr, &flags))
    return true;

  if (fmt->begin_context(CTX_QEP_TAB)) return true;

  if (Explain::shallow_explain() ||
      (can_walk_clauses() &&
       mark_subqueries(select_lex->where_cond(), fmt->entry())))
    return true;

  if (fmt->end_context(CTX_QEP_TAB)) return true;

  if (order_list && fmt->end_context(CTX_ORDER_BY)) return true;

  return false;
}

bool Explain_table::explain_table_name() {
  return fmt->entry()->col_table_name.set(table->alias);
}

bool Explain_table::explain_join_type() {
  join_type jt;
  if (tab && tab->quick_optim())
    jt = calc_join_type(tab->quick_optim()->get_type());
  else if (key != MAX_KEY)
    jt = JT_INDEX_SCAN;
  else
    jt = JT_ALL;

  fmt->entry()->col_join_type.set_const(join_type_str[jt]);
  return false;
}

bool Explain_table::explain_ref() {
  if (tab && tab->quick_optim()) {
    int key_parts = tab->quick_optim()->used_key_parts;
    while (key_parts--) {
      fmt->entry()->col_ref.push_back("const");
    }
  }
  return false;
}

bool Explain_table::explain_key_and_len() {
  if (tab && tab->quick_optim())
    return explain_key_and_len_quick(tab->quick_optim());
  else if (key != MAX_KEY)
    return explain_key_and_len_index(key);
  return false;
}

bool Explain_table::explain_rows_and_filtered() {
  /* Don't print estimated # of rows in table for INSERT/REPLACE. */
  if (fmt->entry()->mod_type == MT_INSERT ||
      fmt->entry()->mod_type == MT_REPLACE)
    return false;

  ha_rows examined_rows =
      query_thd->query_plan.get_modification_plan()->examined_rows;
  fmt->entry()->col_rows.set(static_cast<long long>(examined_rows));

  fmt->entry()->col_filtered.set(100.0);

  return false;
}

bool Explain_table::explain_extra() {
  if (message) return fmt->entry()->col_message.set(message);

  for (Field **fld = table->field; *fld != nullptr; ++fld)
    if (table->is_binary_diff_enabled(*fld))
      fmt->entry()->col_partial_update_columns.push_back((*fld)->field_name);

  uint keyno;
  int quick_type;
  if (tab && tab->quick_optim()) {
    keyno = tab->quick_optim()->index;
    quick_type = tab->quick_optim()->get_type();
  } else {
    keyno = key;
    quick_type = -1;
  }

  return (explain_extra_common(quick_type, keyno) ||
          explain_tmptable_and_filesort(need_tmp_table, need_sort));
}

/******************************************************************************
  External function implementations
******************************************************************************/

/**
  Send a message as an "extra" column value

  This function forms the 1st row of the QEP output with a simple text message.
  This is useful to explain such trivial cases as "No tables used" etc.

  @note Also this function explains the rest of QEP (subqueries or joined
        tables if any).

  @param explain_thd thread handle for the connection doing explain
  @param query_thd   thread handle for the connection being explained
  @param select_lex  select_lex to explain
  @param message     text message for the "extra" column.
  @param ctx         current query context, CTX_JOIN in most cases.

  @return false if success, true if error
*/

bool explain_no_table(THD *explain_thd, const THD *query_thd,
                      SELECT_LEX *select_lex, const char *message,
                      enum_parsing_context ctx) {
  DBUG_TRACE;
  const bool ret = Explain_no_table(explain_thd, query_thd, select_lex, message,
                                    ctx, HA_POS_ERROR)
                       .send();
  return ret;
}

/**
  Check that we are allowed to explain all views in list.
  Because this function is called only when we have a complete plan, we know
  that:
  - views contained in merge-able views have been merged and brought up in
  the top list of tables, so we only need to scan this list
  - table_list is not changing while we are reading it.
  If we don't have a complete plan, EXPLAIN output does not contain table
  names, so we don't need to check views.

  @param table_list table to start with, usually lex->query_tables

  @returns
    true   Caller can't EXPLAIN query due to lack of rights on a view in the
           query
    false  Caller can EXPLAIN query
*/

static bool check_acl_for_explain(const TABLE_LIST *table_list) {
  for (const TABLE_LIST *tbl = table_list; tbl; tbl = tbl->next_global) {
    if (tbl->is_view() && tbl->view_no_explain) {
      my_error(ER_VIEW_NO_EXPLAIN, MYF(0));
      return true;
    }
  }
  return false;
}

/**
  EXPLAIN handling for single-table UPDATE and DELETE queries

  Send to the client a QEP data set for single-table EXPLAIN UPDATE/DELETE
  queries. As far as single-table UPDATE/DELETE are implemented without
  the regular JOIN tree, we can't reuse explain_unit() directly,
  thus we deal with this single table in a special way and then call
  explain_unit() for subqueries (if any).

  @param explain_thd    thread handle for the connection doing explain
  @param query_thd      thread handle for the connection being explained
  @param plan           table modification plan
  @param select         Query's select lex

  @return false if success, true if error
*/

bool explain_single_table_modification(THD *explain_thd, const THD *query_thd,
                                       const Modification_plan *plan,
                                       SELECT_LEX *select) {
  DBUG_TRACE;
  Query_result_send result;
  const bool other = (query_thd != explain_thd);
  bool ret;

  if (explain_thd->lex->explain_format->is_tree()) {
    // These kinds of queries don't have a JOIN with an iterator tree.
    return ExplainIterator(explain_thd, query_thd, nullptr);
  }

  /**
    Prepare the self-allocated result object

    For queries with top-level JOIN the caller provides pre-allocated
    Query_result_send object. Then that JOIN object prepares the
    Query_result_send object calling result->prepare() in SELECT_LEX::prepare(),
    result->optimize() in JOIN::optimize() and result->start_execution()
    in JOIN::exec().
    However without the presence of the top-level JOIN we have to
    prepare/initialize Query_result_send object manually.
  */
  List<Item> dummy;
  if (result.prepare(explain_thd, dummy, explain_thd->lex->unit))
    return true; /* purecov: inspected */

  explain_thd->lex->explain_format->send_headers(&result);

  /*
    Optimize currently non-optimized subqueries when needed, but
    - do not optimize subqueries for other connections, and
    - there is no need to optimize subqueries that will not be explained
      because they are attached to a query block that do not return any rows.
  */
  if (!other && !select->is_empty_query()) {
    for (SELECT_LEX_UNIT *unit = select->first_inner_unit(); unit;
         unit = unit->next_unit()) {
      // Derived tables and const subqueries are already optimized
      if (!unit->is_optimized() &&
          unit->optimize(explain_thd, /*materialize_destination=*/nullptr))
        return true; /* purecov: inspected */
    }
  }

  if (!plan || plan->zero_result) {
    ret = Explain_no_table(explain_thd, query_thd, select,
                           plan ? plan->message : plan_not_ready[other],
                           CTX_JOIN, HA_POS_ERROR)
              .send();
  } else {
    // Check access rights for views
    if (other &&
        check_acl_for_explain(query_thd->query_plan.get_lex()->query_tables))
      ret = true;
    else
      ret = Explain_table(explain_thd, query_thd, select, plan->table,
                          plan->tab, plan->key, plan->limit,
                          plan->need_tmp_table, plan->need_sort, plan->mod_type,
                          plan->used_key_is_modified, plan->message)
                .send() ||
            explain_thd->is_error();
  }
  if (ret)
    result.abort_result_set(explain_thd);
  else {
    if (!other) {
      StringBuffer<1024> str;
      query_thd->lex->unit->print(
          explain_thd, &str,
          enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                          QT_NO_DATA_EXPANSION));
      str.append('\0');
      push_warning(explain_thd, Sql_condition::SL_NOTE, ER_YES, str.ptr());
    }

    result.send_eof(explain_thd);
  }
  return ret;
}

/**
  Explain select_lex's join.

  @param explain_thd thread handle for the connection doing explain
  @param query_thd   thread handle for the connection being explained
  @param select_lex  explain join attached to given select_lex
  @param ctx         current explain context
*/

bool explain_query_specification(THD *explain_thd, const THD *query_thd,
                                 SELECT_LEX *select_lex,
                                 enum_parsing_context ctx) {
  Opt_trace_context *const trace = &explain_thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_exec(trace, "join_explain");
  trace_exec.add_select_number(select_lex->select_number);
  Opt_trace_array trace_steps(trace, "steps");
  JOIN *join = select_lex->join;
  const bool other = (query_thd != explain_thd);

  if (!join || join->get_plan_state() == JOIN::NO_PLAN)
    return explain_no_table(explain_thd, query_thd, select_lex,
                            plan_not_ready[other], ctx);

  THD::Query_plan const *query_plan = &join->thd->query_plan;

  // Check access rights for views
  if (other && check_acl_for_explain(query_plan->get_lex()->query_tables))
    return true;

  THD_STAGE_INFO(explain_thd, stage_explaining);

  bool ret;

  switch (join->get_plan_state()) {
    case JOIN::ZERO_RESULT: {
      ret = explain_no_table(explain_thd, query_thd, select_lex,
                             join->zero_result_cause, ctx);
      break;
    }
    case JOIN::NO_TABLES: {
      if (query_plan->get_lex()->insert_table_leaf &&
          query_plan->get_lex()->insert_table_leaf->select_lex == select_lex) {
        // INSERT/REPLACE SELECT ... FROM dual
        ret = Explain_table(
                  explain_thd, query_thd, select_lex,
                  query_plan->get_lex()->insert_table_leaf->table, nullptr,
                  MAX_KEY, HA_POS_ERROR, false, false,
                  (query_plan->get_lex()->sql_command == SQLCOM_INSERT_SELECT
                       ? MT_INSERT
                       : MT_REPLACE),
                  false, nullptr)
                  .send() ||
              explain_thd->is_error();
      } else
        ret = explain_no_table(explain_thd, query_thd, select_lex,
                               "No tables used", CTX_JOIN);

      break;
    }
    case JOIN::PLAN_READY: {
      /*
        (1) If this connection is explaining its own query
        (2) and it hasn't already prepared the JOIN's result,
        then we need to prepare it (for example, to materialize I_S tables).
      */
      if (!other && !join->is_executed() && join->prepare_result())
        return true; /* purecov: inspected */

      const Explain_format_flags *flags = &join->explain_flags;
      const bool need_tmp_table = flags->any(ESP_USING_TMPTABLE);
      const bool need_order = flags->any(ESP_USING_FILESORT);
      const bool distinct = flags->get(ESC_DISTINCT, ESP_EXISTS);

      if (select_lex == select_lex->master_unit()->fake_select_lex)
        ret = Explain_union_result(explain_thd, query_thd, select_lex).send();
      else
        ret = Explain_join(explain_thd, query_thd, select_lex, need_tmp_table,
                           need_order, distinct)
                  .send();
      break;
    }
    default:
      DBUG_ASSERT(0); /* purecov: inspected */
      ret = true;
  }
  DBUG_ASSERT(ret || !explain_thd->is_error());
  ret |= explain_thd->is_error();
  return ret;
}

vector<string> FullDebugString(const THD *thd, const RowIterator &iterator) {
  vector<string> ret = iterator.DebugString();
  if (iterator.expected_rows() >= 0.0) {
    // NOTE: We cannot use %f, since MSVC and GCC round 0.5 in different
    // directions, so tests would not be reproducible between platforms.
    // Format/round using my_gcvt() and llrint() instead.
    char cost_as_string[FLOATING_POINT_BUFFER];
    my_fcvt(iterator.estimated_cost(), 2, cost_as_string, /*error=*/nullptr);
    char str[512];
    snprintf(str, sizeof(str), "  (cost=%s rows=%lld)", cost_as_string,
             llrint(iterator.expected_rows()));
    ret.back() += str;
  }
  if (thd->lex->is_explain_analyze) {
    if (iterator.expected_rows() < 0.0) {
      // We always want a double space between the iterator name and the costs.
      ret.back().push_back(' ');
    }
    ret.back().push_back(' ');
    ret.back() += iterator.TimingString();
  }
  return ret;
}

std::string PrintQueryPlan(int level, RowIterator *iterator) {
  string ret;

  if (iterator == nullptr) {
    ret.assign(level * 4, ' ');
    return ret + "<not executable by iterator executor>\n";
  }

  int top_level = level;

  for (const string &str : FullDebugString(current_thd, *iterator)) {
    ret.append(level * 4, ' ');
    ret += "-> ";
    ret += str;
    ret += "\n";
    ++level;
  }

  for (const RowIterator::Child &child : iterator->children()) {
    if (!child.description.empty()) {
      ret.append(level * 4, ' ');
      ret.append("-> ");
      ret.append(child.description);
      ret.append("\n");
      ret += PrintQueryPlan(level + 1, child.iterator);
    } else {
      ret += PrintQueryPlan(level, child.iterator);
    }
  }
  if (iterator->join_for_explain() != nullptr) {
    for (const auto &child :
         GetIteratorsFromSelectList(iterator->join_for_explain())) {
      ret.append(top_level * 4, ' ');
      ret.append("-> ");
      ret.append(child.description);
      ret.append("\n");
      ret += PrintQueryPlan(top_level + 1, child.iterator);
    }
  }
  return ret;
}

// Return a comma-separated list of all tables that are touched by UPDATE or
// DELETE.
static string FindUpdatedTables(JOIN *join) {
  string ret;
  for (size_t idx = 0; idx < join->tables; ++idx) {
    TABLE *table = join->qep_tab[idx].table();
    if (table != nullptr && table->pos_in_table_list->updating &&
        table->s->table_category != TABLE_CATEGORY_TEMPORARY) {
      if (!ret.empty()) {
        ret += ", ";
      }
      ret += table->alias;
    }
  }
  return ret;
}

static bool ExplainIterator(THD *ethd, const THD *query_thd,
                            SELECT_LEX_UNIT *unit) {
  Query_result_send result;
  {
    List<Item> field_list;
    Item *item = new Item_empty_string("EXPLAIN", 78, system_charset_info);
    if (field_list.push_back(item)) return true;
    if (result.send_result_set_metadata(
            ethd, field_list, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
      return true;
    }
  }

  {
    std::string explain;
    if (unit != nullptr) {
      int base_level = 0;
      JOIN *join = unit->first_select()->join;
      const THD::Query_plan *query_plan = &query_thd->query_plan;
      switch (query_plan->get_command()) {
        case SQLCOM_UPDATE_MULTI:
        case SQLCOM_UPDATE:
          explain = "-> Update " + FindUpdatedTables(join) + "\n";
          base_level = 1;
          break;
        case SQLCOM_DELETE_MULTI:
        case SQLCOM_DELETE:
          explain = "-> Delete from " + FindUpdatedTables(join) + "\n";
          base_level = 1;
          break;
        case SQLCOM_INSERT_SELECT:
        case SQLCOM_INSERT:
          explain = string("-> Insert into ") +
                    query_plan->get_lex()->insert_table_leaf->table->alias +
                    "\n";
          base_level = 1;
          break;
        case SQLCOM_REPLACE_SELECT:
        case SQLCOM_REPLACE:
          explain = string("-> Replace into ") +
                    query_plan->get_lex()->insert_table_leaf->table->alias +
                    "\n";
          base_level = 1;
          break;
        default:
          break;
      }
      explain += PrintQueryPlan(base_level, unit->root_iterator());
    } else {
      explain += PrintQueryPlan(0, nullptr);
    }
    List<Item> field_list;
    Item *item =
        new Item_string(explain.data(), explain.size(), system_charset_info);
    if (field_list.push_back(item)) return true;

    if (query_thd->killed) {
      ethd->raise_warning(ER_QUERY_INTERRUPTED);
    }

    if (result.send_data(ethd, field_list)) {
      return true;
    }
  }
  return result.send_eof(ethd);
}

/**
  A query result handler that outputs nothing. It is used during EXPLAIN
  ANALYZE, to ignore the output of the query when it's being run.
 */
class Query_result_null : public Query_result_interceptor {
 public:
  Query_result_null() : Query_result_interceptor() {}
  uint field_count(List<Item> &) const override { return 0; }
  bool send_result_set_metadata(THD *, List<Item> &, uint) override {
    return false;
  }
  bool send_data(THD *thd, List<Item> &items) override {
    // Evaluate all the items, to make sure that any subqueries in SELECT lists
    // are evaluated. We don't get their timings added to any parents, but at
    // least we will have real row counts and times printed out.
    for (Item &item : items) {
      item.val_str(&m_str);
      if (thd->is_error()) return true;
    }
    return false;
  }
  bool send_eof(THD *) override { return false; }

 private:
  String m_str;
};

/**
  EXPLAIN handling for SELECT, INSERT/REPLACE SELECT, and multi-table
  UPDATE/DELETE queries

  Send to the client a QEP data set for any DML statement that has a QEP
  represented completely by JOIN object(s).

  This function uses a specific Query_result object for sending explain
  output to the client.

  When explaining own query, the existing Query_result object (found
  in outermost SELECT_LEX_UNIT or SELECT_LEX) is used. However, if the
  Query_result is unsuitable for explanation (need_explain_interceptor()
  returns true), wrap the Query_result inside an Query_result_explain object.

  When explaining other query, create a Query_result_send object and prepare it
  as if it was a regular SELECT query.

  @note see explain_single_table_modification() for single-table
        UPDATE/DELETE EXPLAIN handling.

  @note Unlike handle_query(), explain_query() calls abort_result_set()
        itself in the case of failure (OOM etc.) since it may use
        an internally created Query_result object that has to be deleted
        before exiting the function.

  @param explain_thd thread handle for the connection doing explain
  @param query_thd   thread handle for the connection being explained
  @param unit    query tree to explain

  @return false if success, true if error
*/

bool explain_query(THD *explain_thd, const THD *query_thd,
                   SELECT_LEX_UNIT *unit) {
  DBUG_TRACE;

  const bool other = (explain_thd != query_thd);

  LEX *lex = explain_thd->lex;
  if (lex->explain_format->is_tree()) {
    if (lex->is_explain_analyze) {
      if (explain_thd->lex->m_sql_cmd != nullptr &&
          explain_thd->lex->m_sql_cmd->using_secondary_storage_engine()) {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "EXPLAIN ANALYZE with secondary engine");
        unit->set_executed();
        return true;
      }
      if (unit->root_iterator() == nullptr) {
        // TODO(sgunders): Remove when the iterator executor supports
        // all queries.
        my_error(ER_NOT_SUPPORTED_YET, MYF(0), "EXPLAIN ANALYZE on this query");
        unit->set_executed();
        return true;
      }

      // Run the query, but with the result suppressed.
      Query_result_null null_result;
      unit->set_query_result(&null_result);
      explain_thd->running_explain_analyze = true;
      unit->execute(explain_thd);
      explain_thd->running_explain_analyze = false;
      unit->set_executed();
      if (query_thd->is_error()) return true;
    }

    return ExplainIterator(explain_thd, query_thd, unit);
  }

  Query_result *explain_result = nullptr;

  if (!other)
    explain_result = unit->query_result()
                         ? unit->query_result()
                         : unit->first_select()->query_result();

  Query_result_explain explain_wrapper(unit, explain_result);

  if (other) {
    if (!((explain_result = new (explain_thd->mem_root) Query_result_send())))
      return true; /* purecov: inspected */
    List<Item> dummy;
    if (explain_result->prepare(explain_thd, dummy, explain_thd->lex->unit))
      return true; /* purecov: inspected */
  } else {
    DBUG_ASSERT(unit->is_optimized());
    if (explain_result->need_explain_interceptor())
      explain_result = &explain_wrapper;
  }

  explain_thd->lex->explain_format->send_headers(explain_result);

  // Reset OFFSET/LIMIT for EXPLAIN output
  explain_thd->lex->unit->offset_limit_cnt = 0;
  explain_thd->lex->unit->select_limit_cnt = 0;

  const bool res = mysql_explain_unit(explain_thd, query_thd, unit);
  /*
    1) The code which prints the extended description is not robust
       against malformed queries, so skip it if we have an error.
    2) The code also isn't thread-safe, skip if explaining other thread
    (see Explain::can_print_clauses())
    3) Allow only SELECT, INSERT/REPLACE ... SELECT, Multi-DELETE and
       Multi-UPDATE. Also Update of VIEW (so techincally it is a single table
       UPDATE), but if the VIEW refers to multiple tables it will be handled in
       this function.
  */
  if (!res &&    // (1)
      !other &&  // (2)
      (query_thd->query_plan.get_command() == SQLCOM_SELECT ||
       query_thd->query_plan.get_command() == SQLCOM_INSERT_SELECT ||
       query_thd->query_plan.get_command() == SQLCOM_REPLACE_SELECT ||
       query_thd->query_plan.get_command() == SQLCOM_DELETE_MULTI ||
       query_thd->query_plan.get_command() == SQLCOM_UPDATE ||
       query_thd->query_plan.get_command() == SQLCOM_UPDATE_MULTI))  // (3)
  {
    StringBuffer<1024> str;
    /*
      The warnings system requires input in utf8, see mysqld_show_warnings().
    */

    enum_query_type eqt =
        enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER);

    /**
      For DML statements use QT_NO_DATA_EXPANSION to avoid over-simplification.
    */
    if (query_thd->query_plan.get_command() != SQLCOM_SELECT)
      eqt = enum_query_type(eqt | QT_NO_DATA_EXPANSION);

    unit->print(explain_thd, &str, eqt);
    str.append('\0');
    push_warning(explain_thd, Sql_condition::SL_NOTE, ER_YES, str.ptr());
  }

  if (res)
    explain_result->abort_result_set(explain_thd);
  else
    explain_result->send_eof(explain_thd);

  if (other) destroy(explain_result);

  return res;
}

/**
  Explain UNION or subqueries of the unit

  If the unit is a UNION, explain it as a UNION. Otherwise explain nested
  subselects.

  @param explain_thd    thread handle for the connection doing explain
  @param query_thd      thread handle for the connection being explained
  @param unit           unit object, might not belong to ethd

  @return false if success, true if error
*/

bool mysql_explain_unit(THD *explain_thd, const THD *query_thd,
                        SELECT_LEX_UNIT *unit) {
  DBUG_TRACE;
  bool res = false;
  if (unit->is_union())
    res = unit->explain(explain_thd, query_thd);
  else
    res = explain_query_specification(explain_thd, query_thd,
                                      unit->first_select(), CTX_JOIN);
  DBUG_ASSERT(res || !explain_thd->is_error());
  res |= explain_thd->is_error();
  return res;
}

/**
  Callback function used by Sql_cmd_explain_other_thread::execute() to find thd
  based on the thread id.

  @note It acquires LOCK_thd_data mutex and LOCK_query_plan mutex,
  when it finds matching thd.
  It is the responsibility of the caller to release LOCK_thd_data.
  We release LOCK_query_plan in the DTOR.
*/
class Find_thd_query_lock : public Find_THD_Impl {
 public:
  explicit Find_thd_query_lock(my_thread_id value)
      : m_id(value), m_thd(nullptr) {}
  ~Find_thd_query_lock() {
    if (m_thd) m_thd->unlock_query_plan();
  }
  virtual bool operator()(THD *thd) {
    if (thd->thread_id() == m_id) {
      mysql_mutex_lock(&thd->LOCK_thd_data);
      thd->lock_query_plan();
      m_thd = thd;
      return true;
    }
    return false;
  }

 private:
  const my_thread_id m_id;  ///< The thread id we are looking for.
  THD *m_thd;               ///< THD we found, having this ID.
};

/**
   Entry point for EXPLAIN CONNECTION: locates the connection by its ID, takes
   proper locks, explains its current statement, releases locks.
   @param  thd THD executing this function (== the explainer)
*/
bool Sql_cmd_explain_other_thread::execute(THD *thd) {
  bool res = false;
  THD *query_thd = nullptr;
  bool send_ok = false;
  const char *user;
  bool unlock_thd_data = false;
  const std::string &db_name = thd->db().str ? thd->db().str : "";
  THD::Query_plan *qp;
  DEBUG_SYNC(thd, "before_explain_other");
  /*
    Check for a super user, if:
    1) connected user don't have enough rights, or
    2) has switched to another user
    then it's not super user.
  */
  if (!(thd->m_main_security_ctx.check_access(GLOBAL_ACLS & ~GRANT_ACL,
                                              db_name)) ||    // (1)
      (0 != strcmp(thd->m_main_security_ctx.priv_user().str,  // (2)
                   thd->security_context()->priv_user().str) ||
       0 != my_strcasecmp(system_charset_info,
                          thd->m_main_security_ctx.priv_host().str,
                          thd->security_context()->priv_host().str))) {
    // Can see only connections of this user
    user = thd->security_context()->priv_user().str;
  } else {
    // Can see all connections
    user = nullptr;
  }

  // Pick thread
  Find_thd_query_lock find_thd_query_lock(m_thread_id);
  if (!thd->killed) {
    query_thd =
        Global_THD_manager::get_instance()->find_thd(&find_thd_query_lock);
    if (query_thd) unlock_thd_data = true;
  }

  if (!query_thd) {
    my_error(ER_NO_SUCH_THREAD, MYF(0), m_thread_id);
    goto err;
  }

  qp = &query_thd->query_plan;

  if (query_thd->get_protocol()->connection_alive() &&
      !query_thd->system_thread && qp->get_command() != SQLCOM_END) {
    /*
      Don't explain:
      1) Prepared statements
      2) EXPLAIN to avoid clash in EXPLAIN code
      3) statements of stored routine
      4) Resolver has not finished (then data structures are changing too much
        and are not safely readable).
        m_sql_cmd is set during parsing and cleared in LEX::reset(), without
        mutex. If we are here, the explained connection has set its qp to
        something else than SQLCOM_END with set_query_plan(), so is in a phase
        after parsing and before LEX::reset(). Thus we can read m_sql_cmd.
        m_sql_cmd::m_prepared is set at end of resolution and cleared at end
        of execution (before setting qp to SQLCOM_END), without mutex.
        So if we see it false while it just changed to true, we'll bail out
        which is ok; if we see it true while it just changed to false, we can
        indeed explain as the plan is still valid and will remain so as we
        hold the mutex.
    */
    if (!qp->is_ps_query() &&  // (1)
        is_explainable_query(qp->get_command()) &&
        !qp->get_lex()->is_explain() &&      // (2)
        qp->get_lex()->sphead == nullptr &&  // (3)
        (!qp->get_lex()->m_sql_cmd ||
         qp->get_lex()->m_sql_cmd->is_prepared()))  // (4)
    {
      Security_context *tmp_sctx = query_thd->security_context();
      DBUG_ASSERT(tmp_sctx->user().str);
      if (user && strcmp(tmp_sctx->user().str, user)) {
        my_error(ER_ACCESS_DENIED_ERROR, MYF(0),
                 thd->security_context()->priv_user().str,
                 thd->security_context()->priv_host().str,
                 (thd->password ? ER_THD(thd, ER_YES) : ER_THD(thd, ER_NO)));
        goto err;
      }
      mysql_mutex_unlock(&query_thd->LOCK_thd_data);
      unlock_thd_data = false;
    } else {
      /*
        Note that we send "not supported" for a supported stmt (e.g. SELECT)
        which is in-parsing or in-preparation, which is a bit confusing, but
        ok as the user is unlikely to try EXPLAIN in these short phases.
      */
      my_error(ER_EXPLAIN_NOT_SUPPORTED, MYF(0));
      goto err;
    }
  } else {
    send_ok = true;
    goto err;
  }
  DEBUG_SYNC(thd, "explain_other_got_thd");

  if (qp->is_single_table_plan())
    res = explain_single_table_modification(
        thd, query_thd, qp->get_modification_plan(),
        qp->get_lex()->unit->first_select());
  else
    res = explain_query(thd, query_thd, qp->get_lex()->unit);

err:
  if (unlock_thd_data) mysql_mutex_unlock(&query_thd->LOCK_thd_data);

  DEBUG_SYNC(thd, "after_explain_other");
  if (!res && send_ok) my_ok(thd, 0);

  return false;  // Always return "success".
}

void Modification_plan::register_in_thd() {
  thd->lock_query_plan();
  DBUG_ASSERT(thd->query_plan.get_modification_plan() == nullptr);
  thd->query_plan.set_modification_plan(this);
  thd->unlock_query_plan();
}

/**
  Modification_plan's constructor, to represent that we will use an access
  method on the table.

  @details
  Create single table modification plan. The plan is registered in the
  given thd unless the modification is done in a sub-statement
  (function/trigger).

  @param thd_arg        owning thread
  @param mt             modification type - MT_INSERT/MT_UPDATE/etc
  @param tab_arg        Table to modify
  @param key_arg        MAX_KEY or and index number of the key that was chosen
                        to access table data.
  @param limit_arg      HA_POS_ERROR or LIMIT value.
  @param need_tmp_table_arg true if it requires temporary table --
                        "Using temporary"
                        string in the "extra" column.
  @param need_sort_arg  true if it requires filesort() -- "Using filesort"
                        string in the "extra" column.
  @param used_key_is_modified_arg UPDATE updates used key column
  @param rows           How many rows we plan to modify in the table.
*/

Modification_plan::Modification_plan(THD *thd_arg, enum_mod_type mt,
                                     QEP_TAB *tab_arg, uint key_arg,
                                     ha_rows limit_arg, bool need_tmp_table_arg,
                                     bool need_sort_arg,
                                     bool used_key_is_modified_arg,
                                     ha_rows rows)
    : thd(thd_arg),
      mod_type(mt),
      table(tab_arg->table()),
      tab(tab_arg),
      key(key_arg),
      limit(limit_arg),
      need_tmp_table(need_tmp_table_arg),
      need_sort(need_sort_arg),
      used_key_is_modified(used_key_is_modified_arg),
      message(nullptr),
      zero_result(false),
      examined_rows(rows) {
  DBUG_ASSERT(current_thd == thd);
  if (!thd->in_sub_stmt) register_in_thd();
}

/**
  Modification_plan's constructor, to convey a message in the "extra" column
  of EXPLAIN. This is for the case where this message is the main information
  (there is no access path to the table).

  @details
  Create minimal single table modification plan. The plan is registered in the
  given thd unless the modification is done in a sub-statement
  (function/trigger).

  @param thd_arg    Owning thread
  @param mt         Modification type - MT_INSERT/MT_UPDATE/etc
  @param table_arg  Table to modify
  @param message_arg Message
  @param zero_result_arg If we shortcut execution
  @param rows       How many rows we plan to modify in the table.
*/

Modification_plan::Modification_plan(THD *thd_arg, enum_mod_type mt,
                                     TABLE *table_arg, const char *message_arg,
                                     bool zero_result_arg, ha_rows rows)
    : thd(thd_arg),
      mod_type(mt),
      table(table_arg),
      tab(nullptr),
      key(MAX_KEY),
      limit(HA_POS_ERROR),
      need_tmp_table(false),
      need_sort(false),
      used_key_is_modified(false),
      message(message_arg),
      zero_result(zero_result_arg),
      examined_rows(rows) {
  DBUG_ASSERT(current_thd == thd);
  if (!thd->in_sub_stmt) register_in_thd();
}

Modification_plan::~Modification_plan() {
  if (!thd->in_sub_stmt) {
    thd->lock_query_plan();
    DBUG_ASSERT(current_thd == thd &&
                thd->query_plan.get_modification_plan() == this);
    thd->query_plan.set_modification_plan(nullptr);
    thd->unlock_query_plan();
  }
}

void ForEachSubselect(
    Item *parent_item,
    const function<void(int select_number, bool is_dependent, bool is_cacheable,
                        RowIterator *)> &callback) {
  WalkItem(parent_item, enum_walk::POSTFIX, [&callback](Item *item) {
    if (item->type() == Item::SUBSELECT_ITEM) {
      Item_subselect *subselect = down_cast<Item_subselect *>(item);
      SELECT_LEX *select_lex = subselect->unit->first_select();
      int select_number = select_lex->select_number;
      bool is_dependent = select_lex->is_dependent();
      bool is_cacheable = select_lex->is_cacheable();
      if (subselect->unit->root_iterator() != nullptr) {
        callback(select_number, is_dependent, is_cacheable,
                 subselect->unit->root_iterator());
      } else {
        callback(select_number, is_dependent, is_cacheable,
                 subselect->unit->item->root_iterator());
      }
    }
    return false;
  });
}

namespace {

void GetIteratorsFromItem(Item *item, vector<RowIterator::Child> *children) {
  ForEachSubselect(item, [children](int select_number, bool is_dependent,
                                    bool is_cacheable, RowIterator *iterator) {
    char description[256];
    if (is_dependent) {
      snprintf(description, sizeof(description),
               "Select #%d (subquery in projection; dependent)", select_number);
    } else if (!is_cacheable) {
      snprintf(description, sizeof(description),
               "Select #%d (subquery in projection; uncacheable)",
               select_number);
    } else {
      snprintf(description, sizeof(description),
               "Select #%d (subquery in projection; run only once)",
               select_number);
    }
    children->push_back(RowIterator::Child{iterator, description});
  });
}

}  // namespace

vector<RowIterator::Child> GetIteratorsFromSelectList(JOIN *join) {
  vector<RowIterator::Child> ret;
  if (join == nullptr) {
    return ret;
  }

  // Look for any Items in the projection list itself.
  for (Item &item : *join->get_current_fields()) {
    GetIteratorsFromItem(&item, &ret);
  }

  // Look for any Items that were materialized into fields during execution.
  for (unsigned table_idx = join->primary_tables; table_idx < join->tables;
       ++table_idx) {
    QEP_TAB *qep_tab = &join->qep_tab[table_idx];
    if (qep_tab != nullptr && qep_tab->tmp_table_param != nullptr) {
      for (Func_ptr &func : *qep_tab->tmp_table_param->items_to_copy) {
        GetIteratorsFromItem(func.func(), &ret);
      }
    }
  }
  return ret;
}
