/* Copyright (c) 2001, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  Process query expressions that are composed of

  1. UNION of query blocks, and/or

  2. have ORDER BY / LIMIT clauses in more than one level.

  An example of 2) is:

    (SELECT * FROM t1 ORDER BY a LIMIT 10) ORDER BY b LIMIT 5
*/

#include "sql/sql_union.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "memory_debugging.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/udf_registration_types.h"
#include "mysqld_error.h"
#include "scope_guard.h"
#include "sql/auth/auth_acls.h"
#include "sql/basic_row_iterators.h"
#include "sql/composite_iterators.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"     // DEBUG_SYNC
#include "sql/error_handler.h"  // Strict_error_handler
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_subselect.h"
#include "sql/mem_root_array.h"
#include "sql/mysqld.h"
#include "sql/opt_explain.h"  // explain_no_table
#include "sql/opt_explain_format.h"
#include "sql/opt_trace.h"
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_node_base.h"
#include "sql/parse_tree_nodes.h"  // PT_with_clause
#include "sql/pfs_batch_mode.h"
#include "sql/protocol.h"
#include "sql/query_options.h"
#include "sql/row_iterator.h"
#include "sql/sql_base.h"  // fill_record
#include "sql/sql_class.h"
#include "sql/sql_cmd.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_select.h"
#include "sql/sql_tmp_table.h"  // tmp tables
#include "sql/system_variables.h"
#include "sql/table_function.h"  // Table_function
#include "sql/thd_raii.h"
#include "sql/timing_iterator.h"
#include "sql/window.h"  // Window
#include "template_utils.h"

using std::move;
using std::vector;

bool Query_result_union::prepare(THD *, List<Item> &, SELECT_LEX_UNIT *u) {
  unit = u;
  return false;
}

bool Query_result_union::send_data(THD *thd, List<Item> &values) {
  if (fill_record(thd, table, table->visible_field_ptr(), values, nullptr,
                  nullptr, false))
    return true; /* purecov: inspected */

  if (!check_unique_constraint(table)) return false;

  const int error = table->file->ha_write_row(table->record[0]);
  if (!error) {
    m_rows_in_table++;
    return false;
  }
  // create_ondisk_from_heap will generate error if needed
  if (!table->file->is_ignorable_error(error)) {
    bool is_duplicate;
    if (create_ondisk_from_heap(thd, table, error, true, &is_duplicate))
      return true; /* purecov: inspected */
    // Table's engine changed, index is not initialized anymore
    if (table->hash_field) table->file->ha_index_init(0, false);
    if (!is_duplicate) m_rows_in_table++;
  }
  return false;
}

bool Query_result_union::send_eof(THD *) { return false; }

bool Query_result_union::flush() { return false; }

/**
  Create a temporary table to store the result of a query expression
  (used, among others, when materializing a UNION DISTINCT).

  @param thd_arg            thread handle
  @param column_types       a list of items used to define columns of the
                            temporary table
  @param is_union_distinct  if set, the temporary table will eliminate
                            duplicates on insert
  @param options            create options
  @param table_alias        name of the temporary table
  @param bit_fields_as_long convert bit fields to ulonglong
  @param create_table If false, a table handler will not be created when
                      creating the result table.

  @details
    Create a temporary table that is used to store the result of a UNION,
    derived table, or a materialized cursor.

  @returns false if table created, true if error
*/

bool Query_result_union::create_result_table(
    THD *thd_arg, List<Item> *column_types, bool is_union_distinct,
    ulonglong options, const char *table_alias, bool bit_fields_as_long,
    bool create_table) {
  DBUG_ASSERT(table == nullptr);
  tmp_table_param = Temp_table_param();
  count_field_types(thd_arg->lex->current_select(), &tmp_table_param,
                    *column_types, false, true);
  tmp_table_param.skip_create_table = !create_table;
  tmp_table_param.bit_fields_as_long = bit_fields_as_long;
  if (unit != nullptr) {
    if (unit->is_recursive()) {
      /*
        If the UNIQUE key specified for UNION DISTINCT were a primary key in
        InnoDB, rows would be returned by the scan in an order depending on
        their columns' values, not in insertion order.
      */
      tmp_table_param.can_use_pk_for_unique = false;
    }
    if (unit->mixed_union_operators()) {
      // If we have mixed UNION DISTINCT / UNION ALL, we can't use an unique
      // index to deduplicate, as we need to be able to turn off deduplication
      // checking when we get to the UNION ALL part. The handler supports
      // turning off indexes (and the pre-iterator executor used this to
      // implement mixed DISTINCT/ALL), but not selectively, and we might very
      // well need the other indexes when querying against the table.
      // (Also, it would be nice to be able to remove this functionality
      // altogether from the handler.) Thus, we do it manually instead.
      tmp_table_param.force_hash_field_for_unique = true;
    }
  }

  if (!(table = create_tmp_table(thd_arg, &tmp_table_param, *column_types,
                                 nullptr, is_union_distinct, true, options,
                                 HA_POS_ERROR, table_alias)))
    return true;
  if (create_table) {
    table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
    if (table->hash_field) table->file->ha_index_init(0, false);
  }
  return false;
}

/**
  Reset and empty the temporary table that stores the materialized query result.

  @note The cleanup performed here is exactly the same as for the two temp
  tables of JOIN - exec_tmp_table_[1 | 2].
*/

bool Query_result_union::reset() {
  m_rows_in_table = 0;
  return table ? table->empty_result_table() : false;
}

/**
  This class is effectively dead. It was used for non-DISTINCT UNIONs
  in the pre-iterator executor. Now it exists only as a shell for certain
  setup tasks, and should be removed.
*/
class Query_result_union_direct final : public Query_result_union {
 private:
  /// Result object that receives all rows
  Query_result *result;

  /// Wrapped result is optimized
  bool optimized;
  /// Wrapped result has started execution
  bool execution_started;

 public:
  Query_result_union_direct(Query_result *result, SELECT_LEX *last_select_lex)
      : Query_result_union(),
        result(result),
        optimized(false),
        execution_started(false) {
    unit = last_select_lex->master_unit();
  }
  bool change_query_result(THD *thd, Query_result *new_result) override;
  uint field_count(List<Item> &) const override {
    // Only called for top-level Query_results, usually Query_result_send
    DBUG_ASSERT(false); /* purecov: inspected */
    return 0;           /* purecov: inspected */
  }
  bool postponed_prepare(THD *thd, List<Item> &types) override;
  bool send_result_set_metadata(THD *, List<Item> &, uint) override {
    // Should never be called.
    abort();
  }
  bool send_data(THD *, List<Item> &) override {
    // Should never be called.
    abort();
  }
  bool optimize() override {
    if (optimized) return false;
    optimized = true;

    return result->optimize();
  }
  bool start_execution(THD *thd) override {
    if (execution_started) return false;
    execution_started = true;
    return result->start_execution(thd);
  }
  void send_error(THD *thd, uint errcode, const char *err) override {
    result->send_error(thd, errcode, err); /* purecov: inspected */
  }
  bool send_eof(THD *) override {
    // Should never be called.
    abort();
  }
  bool flush() override { return false; }
  bool check_simple_select() const override {
    // Only called for top-level Query_results, usually Query_result_send
    DBUG_ASSERT(false); /* purecov: inspected */
    return false;       /* purecov: inspected */
  }
  void abort_result_set(THD *thd) override {
    result->abort_result_set(thd); /* purecov: inspected */
  }
  void cleanup(THD *) override {}
};

/**
  Replace the current query result with new_result and prepare it.

  @param thd        Thread handle
  @param new_result New query result

  @returns false if success, true if error
*/
bool Query_result_union_direct::change_query_result(THD *thd,
                                                    Query_result *new_result) {
  result = new_result;
  return result->prepare(thd, unit->types, unit);
}

bool Query_result_union_direct::postponed_prepare(THD *thd, List<Item> &types) {
  if (result == nullptr) return false;

  return result->prepare(thd, types, unit);
}

/// RAII class to automate saving/restoring of current_select()
class Change_current_select {
 public:
  Change_current_select(THD *thd_arg)
      : thd(thd_arg), saved_select(thd->lex->current_select()) {}
  void restore() { thd->lex->set_current_select(saved_select); }
  ~Change_current_select() { restore(); }

 private:
  THD *thd;
  SELECT_LEX *saved_select;
};

/**
  Prepare the fake_select_lex query block

  @param thd_arg Thread handler

  @returns false if success, true if error
*/

bool SELECT_LEX_UNIT::prepare_fake_select_lex(THD *thd_arg) {
  DBUG_TRACE;

  DBUG_ASSERT(thd_arg->lex->current_select() == fake_select_lex);

  // The UNION result table is input table for this query block
  fake_select_lex->table_list.link_in_list(&result_table_list,
                                           &result_table_list.next_local);

  result_table_list.select_lex = fake_select_lex;

  // Set up the result table for name resolution
  fake_select_lex->context.table_list =
      fake_select_lex->context.first_name_resolution_table =
          fake_select_lex->get_table_list();
  if (!fake_select_lex->first_execution) {
    for (ORDER *order = fake_select_lex->order_list.first; order;
         order = order->next)
      order->item = &order->item_ptr;
  }
  for (ORDER *order = fake_select_lex->order_list.first; order;
       order = order->next) {
    Item_ident::Change_context ctx(&fake_select_lex->context);
    (*order->item)
        ->walk(&Item::change_context_processor, enum_walk::POSTFIX,
               (uchar *)&ctx);
  }
  fake_select_lex->set_query_result(query_result());

  fake_select_lex->fields_list = item_list;

  /*
    We need to add up n_sum_items in order to make the correct
    allocation in setup_ref_array().
    Don't add more sum_items if we have already done SELECT_LEX::prepare
    for this (with a different join object)
  */
  if (fake_select_lex->base_ref_items.is_null())
    fake_select_lex->n_child_sum_items += fake_select_lex->n_sum_items;

  DBUG_ASSERT(fake_select_lex->with_wild == 0 &&
              fake_select_lex->master_unit() == this &&
              !fake_select_lex->group_list.elements &&
              fake_select_lex->where_cond() == nullptr &&
              fake_select_lex->having_cond() == nullptr);

  if (fake_select_lex->prepare(thd_arg)) return true;

  return false;
}

bool SELECT_LEX_UNIT::can_materialize_directly_into_result() const {
  // There's no point in doing this if we're not already trying to materialize.
  if (!is_union()) {
    return false;
  }

  // We can't materialize directly into the result if we have sorting.
  // Otherwise, we're fine.
  return global_parameters()->order_list.elements == 0;
}

/**
  Prepares all query blocks of a query expression, including
  fake_select_lex. If a recursive query expression, this also creates the
  materialized temporary table.

  @param thd           Thread handler
  @param sel_result    Result object where the unit's output should go.
  @param added_options These options will be added to the query blocks.
  @param removed_options Options that cannot be used for this query

  @returns false if success, true if error
 */
bool SELECT_LEX_UNIT::prepare(THD *thd, Query_result *sel_result,
                              ulonglong added_options,
                              ulonglong removed_options) {
  DBUG_TRACE;

  DBUG_ASSERT(!is_prepared());
  Change_current_select save_select(thd);

  Query_result *tmp_result;
  bool instantiate_tmp_table = false;

  SELECT_LEX *last_select = first_select();
  while (last_select->next_select()) last_select = last_select->next_select();

  set_query_result(sel_result);

  thd->lex->set_current_select(first_select());

  // Save fake_select_lex in case we don't need it for anything but
  // global parameters.
  if (saved_fake_select_lex ==
          nullptr &&  // Don't overwrite on PS second prepare
      fake_select_lex != nullptr)
    saved_fake_select_lex = fake_select_lex;

  const bool simple_query_expression = is_simple();

  // Create query result object for use by underlying query blocks
  if (!simple_query_expression) {
    if (is_union() && !union_needs_tmp_table(thd->lex)) {
      if (!(tmp_result = union_result = new (thd->mem_root)
                Query_result_union_direct(sel_result, last_select)))
        goto err; /* purecov: inspected */
      if (fake_select_lex != nullptr) fake_select_lex = nullptr;
      instantiate_tmp_table = false;
    } else {
      if (!(tmp_result = union_result =
                new (thd->mem_root) Query_result_union()))
        goto err; /* purecov: inspected */
      instantiate_tmp_table = true;
    }

    if (fake_select_lex != nullptr) {
      /*
        There exists a query block that consolidates the UNION result.
        Prepare the active options for this query block. If these options
        contain OPTION_BUFFER_RESULT, the query block will perform a buffering
        operation, which means that an underlying query block does not need to
        buffer its result, and the buffer option for the underlying query blocks
        can be cleared.
        For subqueries in form "a IN (SELECT .. UNION SELECT ..):
        when optimizing the fake_select_lex that reads the results of the union
        from a temporary table, do not mark the temp. table as constant because
        the contents in it may vary from one subquery execution to another, by
        adding OPTION_NO_CONST_TABLES.
      */
      fake_select_lex->make_active_options(
          (added_options & (OPTION_FOUND_ROWS | OPTION_BUFFER_RESULT)) |
              OPTION_NO_CONST_TABLES | SELECT_NO_UNLOCK,
          0);
      added_options &= ~OPTION_BUFFER_RESULT;
    }
  } else {
    // Only one query block, and no "fake" object: No extra result needed:
    tmp_result = sel_result;
  }

  first_select()->context.resolve_in_select_list = true;

  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    // All query blocks get their options in this phase
    sl->set_query_result(tmp_result);
    sl->make_active_options(added_options | SELECT_NO_UNLOCK, removed_options);
    sl->fields_list = sl->item_list;
    /*
      setup_tables_done_option should be set only for very first SELECT,
      because it protect from second setup_tables call for select-like non
      select commands (DELETE/INSERT/...) and they use only very first
      SELECT (for union it can be only INSERT ... SELECT).
    */
    added_options &= ~OPTION_SETUP_TABLES_DONE;

    thd->lex->set_current_select(sl);

    if (sl == first_recursive) {
      // create_result_table() depends on current_select()
      save_select.restore();
      /*
        All next query blocks will read the temporary table, which we must
        thus create now:
      */
      if (derived_table->setup_materialized_derived_tmp_table(thd))
        goto err; /* purecov: inspected */
      thd->lex->set_current_select(sl);
    }

    if (sl->recursive_reference)  // Make tmp table known to query block:
      derived_table->common_table_expr()->substitute_recursive_reference(thd,
                                                                         sl);

    if (sl->prepare(thd)) goto err;

    /*
      Use items list of underlaid select for derived tables to preserve
      information about fields lengths and exact types
    */
    if (!is_union())
      types = first_select()->item_list;
    else if (sl == first_select()) {
      types.empty();
      List_iterator_fast<Item> it(sl->item_list);
      Item *item_tmp;
      while ((item_tmp = it++)) {
        /*
          If the outer query has a GROUP BY clause, an outer reference to this
          query block may have been wrapped in a Item_outer_ref, which has not
          been fixed yet. An Item_type_holder must be created based on a fixed
          Item, so use the inner Item instead.
        */
        DBUG_ASSERT(item_tmp->fixed ||
                    (item_tmp->type() == Item::REF_ITEM &&
                     down_cast<Item_ref *>(item_tmp)->ref_type() ==
                         Item_ref::OUTER_REF));
        if (!item_tmp->fixed) item_tmp = item_tmp->real_item();

        auto holder = new Item_type_holder(thd, item_tmp);
        if (!holder) goto err; /* purecov: inspected */
        if (is_recursive()) {
          holder->maybe_null = true;  // Always nullable, per SQL standard.
          /*
            The UNION code relies on join_types() to change some
            transitional types like MYSQL_TYPE_DATETIME2 into other types; in
            case this is the only nonrecursive query block join_types() won't
            be called so we need an explicit call:
          */
          holder->join_types(thd, item_tmp);
        }
        types.push_back(holder);
      }
    } else {
      if (types.elements != sl->item_list.elements) {
        my_error(ER_WRONG_NUMBER_OF_COLUMNS_IN_SELECT, MYF(0));
        goto err;
      }
      if (sl->recursive_reference) {
        /*
          Recursive query blocks don't determine output types of the result.
          The only thing to check could be if the recursive query block has a
          type which can't be cast to the output type of the result.
          But in MySQL, all types can be cast to each other (at least during
          resolution; an error may reported when trying to actually insert, for
          example an INT into a POINT). So no further compatibility check is
          needed here.
        */
      } else {
        List_iterator_fast<Item> it(sl->item_list);
        List_iterator_fast<Item> tp(types);
        Item *type, *item_tmp;
        while ((type = tp++, item_tmp = it++)) {
          if (((Item_type_holder *)type)->join_types(thd, item_tmp)) goto err;
        }
      }
    }

    if (sl->recursive_reference &&
        (sl->is_grouped() || sl->m_windows.elements > 0)) {
      // Per SQL2011.
      my_error(ER_CTE_RECURSIVE_FORBIDS_AGGREGATION, MYF(0),
               derived_table->alias);
      goto err;
    }
  }

  if (is_recursive()) {
    // This had to wait until all query blocks are prepared:
    if (check_materialized_derived_query_blocks(thd))
      goto err; /* purecov: inspected */
  }

  /*
    If the query is using Query_result_union_direct, we have postponed
    preparation of the underlying Query_result until column types are known.
  */
  if (union_result != nullptr && union_result->postponed_prepare(thd, types))
    goto err;

  if (!simple_query_expression) {
    /*
      Check that it was possible to aggregate all collations together for UNION.
      We need this in case of UNION DISTINCT, to filter out duplicates using
      the proper collation.

      TODO: consider removing this test in case of UNION ALL.
    */
    List_iterator_fast<Item> tp(types);
    Item *type;

    while ((type = tp++)) {
      if (type->result_type() == STRING_RESULT &&
          type->collation.derivation == DERIVATION_NONE) {
        my_error(ER_CANT_AGGREGATE_NCOLLATIONS, MYF(0), "UNION");
        goto err;
      }
    }
    ulonglong create_options =
        first_select()->active_options() | TMP_TABLE_ALL_COLUMNS;

    if (union_result->create_result_table(
            thd, &types, union_distinct != nullptr, create_options, "", false,
            instantiate_tmp_table))
      goto err;
    result_table_list = TABLE_LIST();
    result_table_list.db = "";
    result_table_list.table_name = result_table_list.alias = "union";
    result_table_list.table = table = union_result->table;
    table->set_pos_in_table_list(&result_table_list);
    result_table_list.select_lex =
        fake_select_lex ? fake_select_lex : saved_fake_select_lex;
    result_table_list.set_tableno(0);

    result_table_list.set_privileges(SELECT_ACL);

    if (!item_list.elements) {
      Prepared_stmt_arena_holder ps_arena_holder(thd);
      if (table->fill_item_list(&item_list)) goto err; /* purecov: inspected */
    } else {
      /*
        We're in execution of a prepared statement or stored procedure:
        reset field items to point at fields from the created temporary table.
      */
      table->reset_item_list(&item_list);
    }
    if (fake_select_lex != nullptr) {
      thd->lex->set_current_select(fake_select_lex);

      if (prepare_fake_select_lex(thd)) goto err;
    }
  }

  // Query blocks are prepared, update the state
  set_prepared();

  return false;

err:
  (void)cleanup(thd, false);
  return true;
}

bool SELECT_LEX_UNIT::optimize(THD *thd, TABLE *materialize_destination) {
  DBUG_TRACE;

  DBUG_ASSERT(is_prepared() && !is_optimized());

  Change_current_select save_select(thd);

  ha_rows estimated_rowcount = 0;
  double estimated_cost = 0.0;

  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    thd->lex->set_current_select(sl);

    // LIMIT is required for optimization
    if (set_limit(thd, sl)) return true; /* purecov: inspected */

    if (sl->optimize(thd)) return true;

    /*
      Accumulate estimated number of rows.
      1. Implicitly grouped query has one row (with HAVING it has zero or one
         rows).
      2. If GROUP BY clause is optimized away because it was a constant then
         query produces at most one row.
     */
    estimated_rowcount +=
        sl->is_implicitly_grouped() || sl->join->group_optimized_away
            ? 1
            : sl->join->best_rowcount;
    estimated_cost += sl->join->best_read;

    // TABLE_LIST::fetch_number_of_rows() expects to get the number of rows
    // from all earlier query blocks from the query result, so we need to update
    // it as we go. In particular, this is used when optimizing a recursive
    // SELECT in a CTE, so that it knows how many rows the non-recursive query
    // blocks will produce.
    //
    // TODO(sgunders): Communicate this in a different way when the query result
    // goes away.
    if (query_result() != nullptr) {
      query_result()->estimated_rowcount = estimated_rowcount;
      query_result()->estimated_cost = estimated_cost;
    }
  }

  if ((uncacheable & UNCACHEABLE_DEPENDENT) && estimated_rowcount <= 1) {
    /*
      This depends on outer references, so optimization cannot assume that all
      executions will always produce the same row. So, increase the counter to
      prevent that this table is replaced with a constant.
      Not testing all bits of "uncacheable", as if derived table sets user
      vars (UNCACHEABLE_SIDEEFFECT) the logic above doesn't apply.
    */
    estimated_rowcount = PLACEHOLDER_TABLE_ROW_ESTIMATE;
  }

  if (fake_select_lex) {
    thd->lex->set_current_select(fake_select_lex);

    if (set_limit(thd, fake_select_lex)) return true; /* purecov: inspected */

    /*
      In EXPLAIN command, constant subqueries that do not use any
      tables are executed two times:
       - 1st time is a real evaluation to get the subquery value
       - 2nd time is to produce EXPLAIN output rows.
      1st execution sets certain members (e.g. Query_result) to perform
      subquery execution rather than EXPLAIN line production. In order
      to reset them back, we re-do all of the actions (yes it is ugly).
    */
    DBUG_ASSERT(fake_select_lex->with_wild == 0 &&
                fake_select_lex->master_unit() == this &&
                !fake_select_lex->group_list.elements &&
                fake_select_lex->get_table_list() == &result_table_list &&
                fake_select_lex->where_cond() == nullptr &&
                fake_select_lex->having_cond() == nullptr);

    if (fake_select_lex->optimize(thd)) return true;
  } else if (saved_fake_select_lex != nullptr) {
    // When GetTableIterator() sets up direct materialization, it looks for
    // the value of global_parameters()'s LIMIT in unit->select_limit_cnt;
    // so set unit->select_limit_cnt accordingly here. This is also done in
    // the other branch above when there is a fake_select_lex.
    if (set_limit(thd, saved_fake_select_lex))
      return true; /* purecov: inspected */
  }

  query_result()->estimated_rowcount = estimated_rowcount;
  query_result()->estimated_cost = estimated_cost;

  // If the caller has asked for materialization directly into a table of its
  // own, and we can do so, do an unfinished materialization (see the comment
  // on this function for more details).
  if (thd->lex->m_sql_cmd != nullptr &&
      thd->lex->m_sql_cmd->using_secondary_storage_engine()) {
    // Not supported when using secondary storage engine.
    create_iterators(thd);
  } else if (estimated_rowcount <= 1) {
    // Don't do it for const tables, as for those, optimize_derived() wants to
    // run the query during optimization, and thus needs an iterator.
    //
    // Do note that JOIN::extract_func_dependent_tables() can want to read from
    // the derived table during the optimization phase even if it has
    // estimated_rowcount larger than one (e.g., because it understands it can
    // get only one row due to a unique index), but will detect that the table
    // has not been created, and treat the the lookup as non-const.
    create_iterators(thd);
  } else if (materialize_destination != nullptr &&
             can_materialize_directly_into_result()) {
    m_query_blocks_to_materialize = setup_materialization(
        thd, materialize_destination, /*union_distinct_only=*/false);
  } else {
    // Recursive CTEs expect to see the rows in the result table immediately
    // after writing them.
    DBUG_ASSERT(!is_recursive());
    create_iterators(thd);
  }

  if (false) {
    // This can be useful during debugging.
    fprintf(stderr, "Query plan:\n%s\n",
            PrintQueryPlan(0, m_root_iterator.get()).c_str());
  }
  set_optimized();  // All query blocks optimized, update the state

  if (item != nullptr) {
    // If we're part of an IN subquery, the containing engine may want to
    // add its own iterators on top, e.g. to materialize us.
    //
    // TODO(sgunders): See if we can do away with the engine concept
    // altogether, now that there's much less execution logic in them.
    DBUG_ASSERT(!unfinished_materialization());
    item->create_iterators(thd);
  }

  return false;
}

Mem_root_array<MaterializeIterator::QueryBlock>
SELECT_LEX_UNIT::setup_materialization(THD *thd, TABLE *dst_table,
                                       bool union_distinct_only) {
  Mem_root_array<MaterializeIterator::QueryBlock> query_blocks(thd->mem_root);

  bool activate_deduplication = (union_distinct != nullptr);
  for (SELECT_LEX *select = first_select(); select != nullptr;
       select =
           select->next_select()) {  // Termination condition at end of loop.
    JOIN *join = select->join;
    MaterializeIterator::QueryBlock query_block;
    DBUG_ASSERT(join && join->is_optimized());
    DBUG_ASSERT(join->root_iterator() != nullptr);
    ConvertItemsToCopy(join->fields, dst_table->visible_field_ptr(),
                       &join->tmp_table_param, join);

    query_block.subquery_iterator = join->release_root_iterator();
    query_block.select_number = select->select_number;
    query_block.join = join;
    if (mixed_union_operators() && !activate_deduplication) {
      query_block.disable_deduplication_by_hash_field = true;
    }
    // See the class comment on AggregateIterator.
    query_block.copy_fields_and_items =
        !join->streaming_aggregation ||
        join->tmp_table_param.precomputed_group_by;
    query_block.temp_table_param = &join->tmp_table_param;
    query_block.is_recursive_reference = select->recursive_reference;

    if (query_block.is_recursive_reference) {
      // Find the recursive reference to ourselves; there should be exactly one,
      // as per the standard.
      for (unsigned table_idx = 0; table_idx < join->tables; ++table_idx) {
        QEP_TAB *qep_tab = &join->qep_tab[table_idx];
        if (qep_tab->recursive_iterator != nullptr) {
          DBUG_ASSERT(query_block.recursive_reader == nullptr);
          query_block.recursive_reader = qep_tab->recursive_iterator;
#ifndef DBUG_OFF
          break;
#endif
        }
      }
      if (query_block.recursive_reader == nullptr) {
        // The recursive reference was optimized away, e.g. due to an impossible
        // WHERE condition, so we're not a recursive reference after all.
        query_block.is_recursive_reference = false;
      }
    }

    query_blocks.push_back(move(query_block));

    if (select == union_distinct) {
      // Last query block that is part of a UNION DISTINCT.
      activate_deduplication = false;
      if (union_distinct_only) {
        // The rest will be done by appending.
        break;
      }
    }
  }
  return query_blocks;
}

void SELECT_LEX_UNIT::create_iterators(THD *thd) {
  if (is_simple()) {
    JOIN *join = first_select()->join;
    DBUG_ASSERT(join && join->is_optimized());
    m_root_iterator = join->release_root_iterator();
    return;
  }

  // Decide whether we can stream rows, ie., never actually put them into the
  // temporary table. If we can, we materialize the UNION DISTINCT blocks first,
  // and then stream the remaining UNION ALL blocks (if any) by means of
  // AppendIterator.
  //
  // If we cannot stream (ie., everything has to go into the temporary table),
  // our strategy for mixed UNION ALL/DISTINCT becomes a bit different;
  // see MaterializeIterator for details.
  bool streaming_allowed = true;
  if (global_parameters()->order_list.size() != 0) {
    // If we're sorting, we currently put it in a real table no matter what.
    // This is a legacy decision, because we used to not know whether filesort
    // would want to refer to rows in the table after the sort (sort by row ID).
    // We could probably be more intelligent here now.
    streaming_allowed = false;
  } else if ((thd->lex->sql_command == SQLCOM_INSERT_SELECT ||
              thd->lex->sql_command == SQLCOM_REPLACE_SELECT) &&
             thd->lex->unit == this) {
    // If we're doing an INSERT or REPLACE, and we're not outputting to
    // a temporary table already (ie., we are the topmost unit), then we
    // don't want to insert any records before we're done scanning. Otherwise,
    // we would risk incorrect results and/or infinite loops, as we'd be seeing
    // our own records as they get inserted.
    //
    // @todo Figure out if we can check for OPTION_BUFFER_RESULT instead;
    //       see bug #23022426.
    streaming_allowed = false;
  }

  TABLE *tmp_table = union_result->table;
  tmp_table->alias = "<union temporary>";

  ha_rows offset = global_parameters()->get_offset(thd);
  ha_rows limit = global_parameters()->get_limit(thd);
  if (limit + offset >= limit)
    limit += offset;
  else
    limit = HA_POS_ERROR; /* purecov: inspected */
  const bool calc_found_rows =
      (first_select()->active_options() & OPTION_FOUND_ROWS);

  vector<unique_ptr_destroy_only<RowIterator>> union_all_sub_iterators;

  // If streaming is allowed, we can do all the parts that are UNION ALL by
  // streaming; the rest have to go to the table.
  //
  // Handle the query blocks that we need to materialize. This may be
  // UNION DISTINCT query blocks only, or all blocks.
  if (union_distinct != nullptr || !streaming_allowed) {
    Mem_root_array<MaterializeIterator::QueryBlock> query_blocks =
        setup_materialization(thd, tmp_table, streaming_allowed);

    unique_ptr_destroy_only<RowIterator> table_iterator;
    if (fake_select_lex != nullptr) {
      table_iterator = fake_select_lex->join->release_root_iterator();
    } else {
      table_iterator =
          NewIterator<TableScanIterator>(thd, tmp_table, nullptr, nullptr);
    }
    bool push_limit_down =
        global_parameters()->order_list.size() == 0 && !calc_found_rows;
    union_all_sub_iterators.emplace_back(NewIterator<MaterializeIterator>(
        thd, move(query_blocks), tmp_table, move(table_iterator),
        /*cte=*/nullptr, /*unit=*/nullptr, /*join=*/nullptr,
        /*ref_slice=*/-1,
        /*rematerialize=*/true, push_limit_down ? limit : HA_POS_ERROR));
  }

  if (streaming_allowed) {
    SELECT_LEX *first_union_all = (union_distinct == nullptr)
                                      ? first_select()
                                      : union_distinct->next_select();
    for (SELECT_LEX *select = first_union_all; select != nullptr;
         select = select->next_select()) {
      JOIN *join = select->join;
      DBUG_ASSERT(join && join->is_optimized());
      ConvertItemsToCopy(join->fields, tmp_table->visible_field_ptr(),
                         &join->tmp_table_param, join);
      bool copy_fields_and_items = !join->streaming_aggregation ||
                                   join->tmp_table_param.precomputed_group_by;
      union_all_sub_iterators.emplace_back(NewIterator<StreamingIterator>(
          thd, join->release_root_iterator(), &join->tmp_table_param, tmp_table,
          copy_fields_and_items));
    }
  }

  DBUG_ASSERT(!union_all_sub_iterators.empty());
  if (union_all_sub_iterators.size() == 1) {
    m_root_iterator = move(union_all_sub_iterators[0]);
  } else {
    // Just append all the UNION ALL sub-blocks.
    DBUG_ASSERT(streaming_allowed);
    m_root_iterator =
        NewIterator<AppendIterator>(thd, move(union_all_sub_iterators));
  }

  // NOTE: If there's a fake_select_lex, its JOIN's iterator already handles
  // LIMIT/OFFSET, so we don't do it again here.
  if ((limit != HA_POS_ERROR || offset != 0) && fake_select_lex == nullptr) {
    m_root_iterator = NewIterator<LimitOffsetIterator>(
        thd, move(m_root_iterator), limit, offset, calc_found_rows,
        &send_records);
  }
}

/**
  Explain query starting from this unit.

  @param explain_thd thread handle for the connection doing explain
  @param query_thd   thread handle for the connection being explained

  @return false if success, true if error
*/

bool SELECT_LEX_UNIT::explain(THD *explain_thd, const THD *query_thd) {
  DBUG_TRACE;

#ifndef DBUG_OFF
  SELECT_LEX *lex_select_save = query_thd->lex->current_select();
#endif
  Explain_format *fmt = explain_thd->lex->explain_format;
  const bool other = (query_thd != explain_thd);
  bool ret = false;

  DBUG_ASSERT(other || is_optimized() || outer_select()->is_empty_query() ||
              // @todo why is this necessary?
              outer_select()->join == nullptr ||
              outer_select()->join->zero_result_cause);

  if (fmt->begin_context(CTX_UNION)) return true;

  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    if (fmt->begin_context(CTX_QUERY_SPEC)) return true;
    if (explain_query_specification(explain_thd, query_thd, sl, CTX_JOIN) ||
        fmt->end_context(CTX_QUERY_SPEC))
      return true;
  }

  if (fake_select_lex != nullptr) {
    // Don't save result as it's needed only for consequent exec.
    ret = explain_query_specification(explain_thd, query_thd, fake_select_lex,
                                      CTX_UNION_RESULT);
  }
  if (!other)
    DBUG_ASSERT(current_thd->lex->current_select() == lex_select_save);

  if (ret) return true;
  fmt->end_context(CTX_UNION);

  return false;
}

/**
  Empties all correlated query blocks defined within the query expression;
  that is, correlated CTEs defined in the expression's WITH clause, and
  correlated derived tables.
 */
bool SELECT_LEX_UNIT::clear_correlated_query_blocks() {
  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    sl->join->clear_corr_derived_tmp_tables();
    sl->join->clear_sj_tmp_tables();
  }
  if (!m_with_clause) return false;
  for (auto el : m_with_clause->m_list->elements()) {
    Common_table_expr &cte = el->m_postparse;
    bool reset_tables = false;
    for (auto tl : cte.references) {
      if (tl->table &&
          tl->derived_unit()->uncacheable & UNCACHEABLE_DEPENDENT) {
        reset_tables = true;
        if (tl->derived_unit()->query_result()->reset()) return true;
      }
      /*
        This loop has found all non-recursive clones; one writer and N
        readers.
      */
    }
    if (!reset_tables) continue;
    for (auto tl : cte.tmp_tables) {
      if (tl->is_derived()) continue;  // handled above
      if (tl->table->empty_result_table()) return true;
      // This loop has found all recursive clones (only readers).
    }
    /*
      Doing delete_all_rows on all clones is good as it makes every
      'file' up to date. Setting materialized=false on all is also important
      or the writer would skip materialization, see loop at start of
      TABLE_LIST::materialize_derived()).
      There is one "recursive table" which we don't find here: it's the
      UNION DISTINCT tmp table. It's reset in unit::execute() of the unit
      which is the body of the CTE.
    */
  }
  return false;
}

bool SELECT_LEX_UNIT::ClearForExecution(THD *thd) {
  if (is_executed()) {
    if (clear_correlated_query_blocks()) return true;

    // TODO(sgunders): Most of JOIN::reset() should be done in iterators.
    for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
      if (sl->join->is_executed()) {
        thd->lex->set_current_select(sl);
        sl->join->reset();
      }
      if (fake_select_lex != nullptr) {
        thd->lex->set_current_select(fake_select_lex);
        fake_select_lex->join->reset();
      }
    }
  }

  for (SELECT_LEX *select_lex = first_select(); select_lex;
       select_lex = select_lex->next_select()) {
    JOIN *join = select_lex->join;
    select_lex->join->examined_rows = 0;
    select_lex->join
        ->set_executed();  // The dynamic range optimizer expects this.

    // TODO(sgunders): Consider doing this in some iterator instead.
    if (join->m_windows.elements > 0 && !join->m_windowing_steps) {
      // Initialize state of window functions as end_write_wf() will be shortcut
      for (Window &w : select_lex->join->m_windows) {
        w.reset_all_wf_state();
      }
    }
  }
  return false;
}

bool SELECT_LEX_UNIT::ExecuteIteratorQuery(THD *thd) {
  THD_STAGE_INFO(thd, stage_executing);
  DEBUG_SYNC(thd, "before_join_exec");

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_exec(trace, "join_execution");
  if (is_simple()) {
    trace_exec.add_select_number(first_select()->select_number);
  }
  Opt_trace_array trace_steps(trace, "steps");

  if (ClearForExecution(thd)) {
    return true;
  }

  List<Item> *fields = get_field_list();
  Query_result *query_result = this->query_result();
  DBUG_ASSERT(query_result != nullptr);

  if (query_result->start_execution(thd)) return true;

  if (query_result->send_result_set_metadata(
          thd, *fields, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    return true;
  }

  set_executed();

  // Hand over the query to the secondary engine if needed.
  if (first_select()->join->override_executor_func != nullptr) {
    thd->current_found_rows = 0;
    for (SELECT_LEX *select = first_select(); select != nullptr;
         select = select->next_select()) {
      if (select->join->override_executor_func(select->join)) {
        return true;
      }
      thd->current_found_rows += select->join->send_records;
    }
    const bool calc_found_rows =
        (first_select()->active_options() & OPTION_FOUND_ROWS);
    if (!calc_found_rows) {
      // This is for backwards compatibility reasons only;
      // we have documented that without SQL_CALC_FOUND_ROWS,
      // we return the actual number of rows returned.
      thd->current_found_rows =
          std::min(thd->current_found_rows, select_limit_cnt);
    }
    return query_result->send_eof(thd);
  }

  if (item) {
    item->reset_value_registration();

    if (item->assigned()) {
      item->assigned(false);  // Prepare for re-execution of this unit
      item->reset();
    }
  }

  // We need to accumulate in the first join's send_records as long as
  // we support SQL_CALC_FOUND_ROWS, since LimitOffsetIterator will use it
  // for reporting rows skipped by OFFSET or LIMIT. When we get rid of
  // SQL_CALC_FOUND_ROWS, we can use a local variable here instead.
  ha_rows *send_records_ptr;
  if (fake_select_lex != nullptr) {
    // UNION with LIMIT: found_rows() applies to the outermost block.
    // LimitOffsetIterator will write skipped OFFSET rows into the
    // fake_select_lex's send_records, so use that.
    send_records_ptr = &fake_select_lex->join->send_records;
  } else if (is_simple()) {
    // Not an UNION: found_rows() applies to the join.
    // LimitOffsetIterator will write skipped OFFSET rows into the JOIN's
    // send_records, so use that.
    send_records_ptr = &first_select()->join->send_records;
  } else {
    // UNION, but without a fake_select_lex (may or may not have a
    // LIMIT): found_rows() applies to the outermost block. See
    // SELECT_LEX_UNIT::send_records for more information.
    send_records_ptr = &send_records;
  }
  *send_records_ptr = 0;

  thd->get_stmt_da()->reset_current_row_for_condition();
  if (m_root_iterator->Init()) {
    return true;
  }

  {
    PFSBatchMode pfs_batch_mode(m_root_iterator.get());
    auto join_cleanup = create_scope_guard([this, thd] {
      for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
        JOIN *join = sl->join;
        join->join_free();
        thd->inc_examined_row_count(join->examined_rows);
      }
      if (fake_select_lex != nullptr) {
        thd->inc_examined_row_count(fake_select_lex->join->examined_rows);
      }
    });

    for (;;) {
      int error = m_root_iterator->Read();
      DBUG_EXECUTE_IF("bug13822652_1", thd->killed = THD::KILL_QUERY;);

      if (error > 0 || thd->is_error())  // Fatal error
        return true;
      else if (error < 0)
        break;
      else if (thd->killed)  // Aborted by user
      {
        thd->send_kill_message();
        return true;
      }

      ++*send_records_ptr;
      if (query_result->send_data(thd, *fields)) {
        return true;
      }
      thd->get_stmt_da()->inc_current_row_for_condition();
    }

    // NOTE: join_cleanup must be done before we send EOF, so that we get the
    // row counts right.
  }

  thd->current_found_rows = *send_records_ptr;

  return query_result->send_eof(thd);
}

/**
  Execute a query expression that may be a UNION and/or have an ordered result.

  @param thd          thread handle

  @returns false if success, true if error
*/

bool SELECT_LEX_UNIT::execute(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(is_optimized());

  if (is_executed() && !uncacheable) return false;

  DBUG_ASSERT(!unfinished_materialization());

  /*
    Even if we return "true" the statement might continue
    (e.g. ER_SUBQUERY_1_ROW in stmt with IGNORE), so we want to restore
    current_select():
  */
  Change_current_select save_select(thd);

  return ExecuteIteratorQuery(thd);
}

/**
  Cleanup this query expression object after preparation or one round
  of execution. After the cleanup, the object can be reused for a
  new round of execution, but a new optimization will be needed before
  the execution.

  @return false if previous execution was successful, and true otherwise
*/

bool SELECT_LEX_UNIT::cleanup(THD *thd, bool full) {
  DBUG_TRACE;

  DBUG_ASSERT(thd == current_thd);

  if (cleaned >= (full ? UC_CLEAN : UC_PART_CLEAN)) return false;

  cleaned = (full ? UC_CLEAN : UC_PART_CLEAN);

  if (full) {
    m_root_iterator.reset();
  }

  m_query_blocks_to_materialize.clear();

  bool error = false;
  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select())
    error |= sl->cleanup(thd, full);

  if (fake_select_lex) {
    if (full) {
      fake_select_lex->table_list.empty();
      fake_select_lex->recursive_reference = nullptr;
    }
    error |= fake_select_lex->cleanup(thd, full);
  }

  // subselect_hash_sj_engine may hold iterators that need to be cleaned up
  // before the MEM_ROOT goes away.
  if (item != nullptr) {
    item->cleanup();
  }

  // fake_select_lex's table depends on Temp_table_param inside union_result
  if (full && union_result) {
    union_result->cleanup(thd);
    destroy(union_result);
    union_result = nullptr;  // Safety
    if (table) free_tmp_table(thd, table);
    table = nullptr;  // Safety
  }

  /*
    explain_marker is (mostly) a property determined at prepare time and must
    thus be preserved for the next execution, if this is a prepared statement.
  */

  return error;
}

#ifndef DBUG_OFF
void SELECT_LEX_UNIT::assert_not_fully_clean() {
  DBUG_ASSERT(cleaned < UC_CLEAN);
  SELECT_LEX *sl = first_select();
  for (;;) {
    if (!sl) {
      sl = fake_select_lex;
      if (!sl) break;
    }
    for (SELECT_LEX_UNIT *lex_unit = sl->first_inner_unit(); lex_unit;
         lex_unit = lex_unit->next_unit())
      lex_unit->assert_not_fully_clean();
    if (sl == fake_select_lex)
      break;
    else
      sl = sl->next_select();
  }
}
#endif

void SELECT_LEX_UNIT::reinit_exec_mechanism() {
  prepared = optimized = executed = false;
  m_root_iterator.reset();
#ifndef DBUG_OFF
  if (is_union()) {
    List_iterator_fast<Item> it(item_list);
    Item *field;
    while ((field = it++)) {
      /*
        we can't cleanup here, because it broke link to temporary table field,
        but have to drop fixed flag to allow next fix_field of this field
        during re-executing
      */
      field->fixed = false;
    }
  }
#endif
}

/**
  Change the query result object used to return the final result of
  the unit, replacing occurences of old_result with new_result.

  @param thd        Thread handle
  @param new_result New query result object
  @param old_result Old query result object

  @retval false Success
  @retval true  Error
*/

bool SELECT_LEX_UNIT::change_query_result(
    THD *thd, Query_result_interceptor *new_result,
    Query_result_interceptor *old_result) {
  for (SELECT_LEX *sl = first_select(); sl; sl = sl->next_select()) {
    if (sl->query_result() &&
        sl->change_query_result(thd, new_result, old_result))
      return true; /* purecov: inspected */
  }
  set_query_result(new_result);
  return false;
}

/**
  Get column type information for this query expression.

  For a single query block the column types are taken from the list
  of selected items of this block.

  For a union this function assumes that SELECT_LEX_UNIT::prepare()
  has been called and returns the type holders that were created for unioned
  column types of all query blocks.

  @note
    The implementation of this function should be in sync with
    SELECT_LEX_UNIT::prepare()

  @returns List of items as specified in function description
*/

List<Item> *SELECT_LEX_UNIT::get_unit_column_types() {
  DBUG_ASSERT(is_prepared());

  return is_union() ? &types : &first_select()->item_list;
}

/**
  Get field list for this query expression.

  For a UNION of query blocks, return the field list generated during prepare.
  For a single query block, return the field list after all possible
  intermediate query processing steps are done (optimization is complete).

  @returns List containing fields of the query expression.
*/

List<Item> *SELECT_LEX_UNIT::get_field_list() {
  DBUG_ASSERT(is_optimized());

  if (fake_select_lex != nullptr) {
    return fake_select_lex->join->fields;
  } else if (is_union()) {
    return &item_list;
  } else {
    return first_select()->join->fields;
  }
}

bool SELECT_LEX_UNIT::mixed_union_operators() const {
  return union_distinct && union_distinct->next_select();
}

bool SELECT_LEX_UNIT::walk(Item_processor processor, enum_walk walk,
                           uchar *arg) {
  for (auto select = first_select(); select != nullptr;
       select = select->next_select()) {
    if (select->walk(processor, walk, arg)) return true;
  }
  if (fake_select_lex && fake_select_lex->walk(processor, walk, arg))
    return true;
  return false;
}

/**
   Closes (and, if last reference, drops) temporary tables created to
   materialize derived tables, schema tables and CTEs.

   @param thd  Thread handler
   @param list List of tables to search in
*/
static void destroy_materialized(THD *thd, TABLE_LIST *list) {
  for (auto tl = list; tl; tl = tl->next_local) {
    if (tl->merge_underlying_list) {
      // Find a materialized view inside another view.
      destroy_materialized(thd, tl->merge_underlying_list);
    } else if (tl->is_table_function()) {
      tl->table_function->cleanup();
    }
    if (tl->table == nullptr) continue;  // Not materialized
    if (tl->is_view_or_derived()) {
      tl->reset_name_temporary();
      if (tl->common_table_expr()) tl->common_table_expr()->tmp_tables.clear();
    } else if (!tl->is_recursive_reference() && !tl->schema_table &&
               !tl->is_table_function())
      continue;
    free_tmp_table(thd, tl->table);
    tl->table = nullptr;
  }
}

/**
  Cleanup after preparation or one round of execution.

  @return false if previous execution was successful, and true otherwise
*/

bool SELECT_LEX::cleanup(THD *thd, bool full) {
  DBUG_TRACE;

  bool error = false;
  if (join) {
    if (full) {
      DBUG_ASSERT(join->select_lex == this);
      error = join->destroy();
      destroy(join);
      join = nullptr;
    } else
      join->cleanup();
  }

  if (full) destroy_materialized(thd, get_table_list());

  for (SELECT_LEX_UNIT *lex_unit = first_inner_unit(); lex_unit;
       lex_unit = lex_unit->next_unit()) {
    error |= lex_unit->cleanup(thd, full);
  }

  if (full && m_windows.elements > 0) {
    List_iterator<Window> li(m_windows);
    Window *w;
    while ((w = li++)) w->cleanup(thd);
  }

  return error;
}

void SELECT_LEX::cleanup_all_joins() {
  if (join) join->cleanup();

  for (SELECT_LEX_UNIT *unit = first_inner_unit(); unit;
       unit = unit->next_unit()) {
    for (SELECT_LEX *sl = unit->first_select(); sl; sl = sl->next_select())
      sl->cleanup_all_joins();
  }
}
