/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

// Handle DELETE queries (both single- and multi-table).

#include "sql/sql_delete.h"

#include <limits.h>
#include <atomic>
#include <memory>
#include <utility>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_table_access
#include "sql/binlog.h"            // mysql_bin_log
#include "sql/composite_iterators.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/filesort.h"    // Filesort
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/key_spec.h"
#include "sql/mem_root_array.h"
#include "sql/mysqld.h"       // stage_...
#include "sql/opt_explain.h"  // Modification_plan
#include "sql/opt_explain_format.h"
#include "sql/opt_range.h"  // prune_partitions
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/query_options.h"
#include "sql/records.h"  // unique_ptr_destroy_only<RowIterator>
#include "sql/row_iterator.h"
#include "sql/sorting_iterator.h"
#include "sql/sql_base.h"  // update_non_unique_table_error
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_optimizer.h"  // optimize_cond, substitute_gc
#include "sql/sql_resolver.h"   // setup_order
#include "sql/sql_select.h"
#include "sql/sql_view.h"  // check_key_in_view
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/timing_iterator.h"
#include "sql/transaction_info.h"
#include "sql/trigger_def.h"
#include "sql/uniques.h"  // Unique

class COND_EQUAL;
class Item_exists_subselect;
class Opt_trace_context;
class Select_lex_visitor;

bool Sql_cmd_delete::precheck(THD *thd) {
  DBUG_TRACE;

  TABLE_LIST *tables = lex->query_tables;

  if (!multitable) {
    if (check_one_table_access(thd, DELETE_ACL, tables)) return true;
  } else {
    TABLE_LIST *aux_tables = delete_tables->first;
    TABLE_LIST **save_query_tables_own_last = lex->query_tables_own_last;

    if (check_table_access(thd, SELECT_ACL, tables, false, UINT_MAX, false))
      return true;

    /*
      Since aux_tables list is not part of LEX::query_tables list we
      have to juggle with LEX::query_tables_own_last value to be able
      call check_table_access() safely.
    */
    lex->query_tables_own_last = nullptr;
    if (check_table_access(thd, DELETE_ACL, aux_tables, false, UINT_MAX,
                           false)) {
      lex->query_tables_own_last = save_query_tables_own_last;
      return true;
    }
    lex->query_tables_own_last = save_query_tables_own_last;
  }
  return false;
}

/**
  Delete a set of rows from a single table.

  @param thd    Thread handler

  @returns false on success, true on error

  @note Like implementations of other DDL/DML in MySQL, this function
  relies on the caller to close the thread tables. This is done in the
  end of dispatch_command().
*/

bool Sql_cmd_delete::delete_from_single_table(THD *thd) {
  DBUG_TRACE;

  myf error_flags = MYF(0); /**< Flag for fatal errors */
  bool will_batch;
  /*
    Most recent handler error
    =  1: Some non-handler error
    =  0: Success
    = -1: No more rows to process, or reached limit
  */
  int error = 0;
  ha_rows deleted_rows = 0;
  bool reverse = false;
  /// read_removal is only used by NDB storage engine
  bool read_removal = false;
  bool need_sort = false;

  uint usable_index = MAX_KEY;
  SELECT_LEX *const select_lex = lex->select_lex;
  SELECT_LEX_UNIT *const unit = select_lex->master_unit();
  ORDER *order = select_lex->order_list.first;
  TABLE_LIST *const table_list = select_lex->get_table_list();
  THD::killed_state killed_status = THD::NOT_KILLED;
  THD::enum_binlog_query_type query_type = THD::ROW_QUERY_TYPE;

  const bool safe_update = thd->variables.option_bits & OPTION_SAFE_UPDATES;

  TABLE_LIST *const delete_table_ref = table_list->updatable_base_table();
  TABLE *const table = delete_table_ref->table;

  const bool transactional_table = table->file->has_transactions();

  const bool has_delete_triggers =
      table->triggers && table->triggers->has_delete_triggers();

  const bool has_before_triggers =
      has_delete_triggers &&
      table->triggers->has_triggers(TRG_EVENT_DELETE, TRG_ACTION_BEFORE);
  const bool has_after_triggers =
      has_delete_triggers &&
      table->triggers->has_triggers(TRG_EVENT_DELETE, TRG_ACTION_AFTER);
  unit->set_limit(thd, select_lex);

  ha_rows limit = unit->select_limit_cnt;
  const bool using_limit = limit != HA_POS_ERROR;

  // Used to track whether there are no rows that need to be read
  bool no_rows = limit == 0;

  QEP_TAB_standalone qep_tab_st;
  QEP_TAB &qep_tab = qep_tab_st.as_QEP_TAB();

  if (table->all_partitions_pruned_away || m_empty_query) {
    /*
      All partitions were pruned away during preparation. Shortcut further
      processing by "no rows". If explaining, report the plan and bail out.
    */
    no_rows = true;

    if (lex->is_explain()) {
      Modification_plan plan(thd, MT_DELETE, table,
                             "No matching rows after partition pruning", true,
                             0);
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }
  }

  Item *conds = nullptr;
  if (!no_rows && select_lex->get_optimizable_conditions(thd, &conds, nullptr))
    return true; /* purecov: inspected */

  /*
    Reset the field list to remove any hidden fields added by substitute_gc() in
    the previous execution.
  */
  select_lex->all_fields = select_lex->fields_list;

  /*
    See if we can substitute expressions with equivalent generated
    columns in the WHERE and ORDER BY clauses of the DELETE statement.
    It is unclear if this is best to do before or after the other
    substitutions performed by substitute_for_best_equal_field(). Do
    it here for now, to keep it consistent with how multi-table
    deletes are optimized in JOIN::optimize().
  */
  if (conds || order)
    static_cast<void>(substitute_gc(thd, select_lex, conds, nullptr, order));

  const bool const_cond = conds == nullptr || conds->const_item();
  const bool const_cond_result = const_cond && (!conds || conds->val_int());
  if (thd->is_error())  // Error during val_int()
    return true;        /* purecov: inspected */
  /*
    We are passing HA_EXTRA_IGNORE_DUP_KEY flag here to recreate query with
    IGNORE keyword within federated storage engine. If federated engine is
    removed in the future, use of HA_EXTRA_IGNORE_DUP_KEY and
    HA_EXTRA_NO_IGNORE_DUP_KEY flag should be removed from
    delete_from_single_table(), Query_result_delete::optimize() and
  */
  if (lex->is_ignore()) table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);

  /*
    Test if the user wants to delete all rows and deletion doesn't have
    any side-effects (because of triggers), so we can use optimized
    handler::delete_all_rows() method.

    We can use delete_all_rows() if and only if:
    - There is no limit clause
    - The condition is constant
    - The row set is not empty
    - We allow new functions (not using option --skip-new)
    - If there is a condition, then it it produces a non-zero value
    - If the current command is DELETE FROM with no where clause, then:
      - We will not be binlogging this statement in row-based, and
      - there should be no delete triggers associated with the table.
  */
  if (!using_limit && const_cond_result && !no_rows &&
      !(specialflag & SPECIAL_NO_NEW_FUNC) &&
      ((!thd->is_current_stmt_binlog_format_row() ||  // not ROW binlog-format
        thd->is_current_stmt_binlog_disabled()) &&    // no binlog for this
                                                      // command
       !has_delete_triggers)) {
    /* Update the table->file->stats.records number */
    table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
    ha_rows const maybe_deleted = table->file->stats.records;

    Modification_plan plan(thd, MT_DELETE, table, "Deleting all rows", false,
                           maybe_deleted);
    if (lex->is_explain()) {
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }

    /* Do not allow deletion of all records if safe_update is set. */
    if (safe_update) {
      my_error(ER_UPDATE_WITHOUT_KEY_IN_SAFE_MODE, MYF(0),
               thd->get_stmt_da()->get_first_condition_message());
      return true;
    }

    DBUG_PRINT("debug", ("Trying to use delete_all_rows()"));
    if (!(error = table->file->ha_delete_all_rows())) {
      /*
        As delete_all_rows() was used, we have to log it in statement format.
      */
      query_type = THD::STMT_QUERY_TYPE;
      error = -1;
      deleted_rows = maybe_deleted;
      goto cleanup;
    }
    if (error != HA_ERR_WRONG_COMMAND) {
      if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

      table->file->print_error(error, error_flags);
      goto cleanup;
    }
    /* Handler didn't support fast delete; Delete rows one by one */
  }

  if (conds != nullptr) {
    COND_EQUAL *cond_equal = nullptr;
    Item::cond_result result;

    if (optimize_cond(thd, &conds, &cond_equal, select_lex->join_list, &result))
      return true;
    if (result == Item::COND_FALSE)  // Impossible where
    {
      no_rows = true;

      if (lex->is_explain()) {
        Modification_plan plan(thd, MT_DELETE, table, "Impossible WHERE", true,
                               0);
        bool err =
            explain_single_table_modification(thd, thd, &plan, select_lex);
        return err;
      }
    }
    if (conds) {
      conds = substitute_for_best_equal_field(thd, conds, cond_equal, nullptr);
      if (conds == nullptr) return true;

      conds->update_used_tables();
    }
  }

  // Initialize the cost model that will be used for this table
  table->init_cost_model(thd->cost_model());

  /* Update the table->file->stats.records number */
  table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);

  // These have been cleared when binding the TABLE object.
  DBUG_ASSERT(table->quick_keys.is_clear_all() &&
              table->possible_quick_keys.is_clear_all());

  table->covering_keys.clear_all();

  /* Prune a second time to be able to prune on subqueries in WHERE clause. */
  if (prune_partitions(thd, table, conds)) return true;
  if (table->all_partitions_pruned_away) {
    /* No matching records */
    if (lex->is_explain()) {
      Modification_plan plan(thd, MT_DELETE, table,
                             "No matching rows after partition pruning", true,
                             0);
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }
    my_ok(thd, 0);
    return false;
  }

  qep_tab.set_table(table);
  qep_tab.set_condition(conds);

  if (conds &&
      thd->optimizer_switch_flag(OPTIMIZER_SWITCH_ENGINE_CONDITION_PUSHDOWN)) {
    table->file->cond_push(conds, false);
  }

  {  // Enter scope for optimizer trace wrapper
    Opt_trace_object wrapper(&thd->opt_trace);
    wrapper.add_utf8_table(delete_table_ref);

    if (!no_rows && conds != nullptr) {
      Key_map keys_to_use(Key_map::ALL_BITS), needed_reg_dummy;
      QUICK_SELECT_I *qck;
      no_rows = test_quick_select(thd, keys_to_use, 0, limit, safe_update,
                                  ORDER_NOT_RELEVANT, &qep_tab, conds,
                                  &needed_reg_dummy, &qck,
                                  qep_tab.table()->force_index) < 0;
      qep_tab.set_quick(qck);
    }
    if (thd->is_error())  // test_quick_select() has improper error propagation
      return true;

    if (no_rows) {
      if (lex->is_explain()) {
        Modification_plan plan(thd, MT_DELETE, table, "Impossible WHERE", true,
                               0);
        bool err =
            explain_single_table_modification(thd, thd, &plan, select_lex);
        return err;
      }

      my_ok(thd, 0);
      return false;  // Nothing to delete
    }
  }  // Ends scope for optimizer trace wrapper

  /* If running in safe sql mode, don't allow updates without keys */
  if (table->quick_keys.is_clear_all()) {
    thd->server_status |= SERVER_QUERY_NO_INDEX_USED;

    /*
      Safe update error isn't returned if:
      1) It is  an EXPLAIN statement OR
      2) LIMIT is present.

      Append the first warning (if any) to the error message. This allows the
      user to understand why index access couldn't be chosen.
    */
    if (!thd->lex->is_explain() && safe_update && !using_limit) {
      my_error(ER_UPDATE_WITHOUT_KEY_IN_SAFE_MODE, MYF(0),
               thd->get_stmt_da()->get_first_condition_message());
      return true;
    }
  }

  if (order) {
    if (table->update_const_key_parts(conds)) return true;
    order = simple_remove_const(order, conds);
    ORDER_with_src order_src(order, ESC_ORDER_BY);
    usable_index =
        get_index_for_order(&order_src, &qep_tab, limit, &need_sort, &reverse);
  }

  // Reaching here only when table must be accessed
  DBUG_ASSERT(!no_rows);

  {
    ha_rows rows;
    if (qep_tab.quick())
      rows = qep_tab.quick()->records;
    else if (!conds && !need_sort && limit != HA_POS_ERROR)
      rows = limit;
    else {
      delete_table_ref->fetch_number_of_rows();
      rows = table->file->stats.records;
    }
    qep_tab.set_quick_optim();
    qep_tab.set_condition_optim();
    Modification_plan plan(thd, MT_DELETE, &qep_tab, usable_index, limit, false,
                           need_sort, false, rows);
    DEBUG_SYNC(thd, "planned_single_delete");

    if (lex->is_explain()) {
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }

    if (select_lex->active_options() & OPTION_QUICK)
      (void)table->file->ha_extra(HA_EXTRA_QUICK);

    unique_ptr_destroy_only<Filesort> fsort;
    unique_ptr_destroy_only<RowIterator> iterator;
    ha_rows examined_rows = 0;
    if (usable_index == MAX_KEY || qep_tab.quick())
      iterator =
          create_table_iterator(thd, nullptr, &qep_tab, false,
                                /*ignore_not_found_rows=*/false, &examined_rows,
                                /*using_table_scan=*/nullptr);
    else
      iterator = create_table_iterator_idx(thd, table, usable_index, reverse,
                                           &qep_tab);

    if (need_sort) {
      DBUG_ASSERT(usable_index == MAX_KEY);

      if (qep_tab.condition() != nullptr) {
        iterator = NewIterator<FilterIterator>(thd, move(iterator),
                                               qep_tab.condition());
      }

      fsort.reset(new (thd->mem_root) Filesort(
          thd, table, /*keep_buffers=*/false, order, HA_POS_ERROR,
          /*force_stable_sort=*/false,
          /*remove_duplicates=*/false,
          /*force_sort_positions=*/true));
      unique_ptr_destroy_only<RowIterator> sort = NewIterator<SortingIterator>(
          thd, &qep_tab, fsort.get(), move(iterator),
          /*rows_examined=*/nullptr);
      if (sort->Init()) return true;
      iterator = move(sort);
      thd->inc_examined_row_count(examined_rows);

      /*
        Filesort has already found and selected the rows we want to delete,
        so we don't need the where clause
      */
      qep_tab.set_condition(nullptr);
    } else {
      if (iterator->Init()) return true;
    }

    if (select_lex->has_ft_funcs() && init_ftfuncs(thd, select_lex))
      return true; /* purecov: inspected */

    THD_STAGE_INFO(thd, stage_updating);

    if (table->prepare_triggers_for_delete_stmt_or_event()) {
      will_batch = false;
    } else {
      // No after delete triggers, attempt to start bulk delete
      will_batch = !table->file->start_bulk_delete();
    }
    table->mark_columns_needed_for_delete(thd);
    if (thd->is_error()) return true;

    if ((table->file->ha_table_flags() & HA_READ_BEFORE_WRITE_REMOVAL) &&
        !using_limit && !has_delete_triggers && qep_tab.quick() &&
        qep_tab.quick()->index != MAX_KEY)
      read_removal = table->check_read_removal(qep_tab.quick()->index);

    DBUG_ASSERT(limit > 0);

    // The loop that reads rows and delete those that qualify

    while (!(error = iterator->Read()) && !thd->killed) {
      DBUG_ASSERT(!thd->is_error());
      thd->inc_examined_row_count(1);

      if (qep_tab.condition() != nullptr) {
        const bool skip_record = qep_tab.condition()->val_int() == 0;
        if (thd->is_error()) {
          error = 1;
          break;
        }
        if (skip_record) {
          // Row failed condition check, release lock
          table->file->unlock_row();
          continue;
        }
      }

      DBUG_ASSERT(!thd->is_error());
      if (has_before_triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                            TRG_ACTION_BEFORE, false)) {
        error = 1;
        break;
      }

      if ((error = table->file->ha_delete_row(table->record[0]))) {
        if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

        table->file->print_error(error, error_flags);
        /*
          In < 4.0.14 we set the error number to 0 here, but that
          was not sensible, because then MySQL would not roll back the
          failed DELETE, and also wrote it to the binlog. For MyISAM
          tables a DELETE probably never should fail (?), but for
          InnoDB it can fail in a FOREIGN KEY error or an
          out-of-tablespace error.
        */
        if (thd->is_error())  // Could be downgraded to warning by IGNORE
        {
          error = 1;
          break;
        }
      }

      deleted_rows++;
      if (has_after_triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                            TRG_ACTION_AFTER, false)) {
        error = 1;
        break;
      }
      if (!--limit && using_limit) {
        error = -1;
        break;
      }
    }

    killed_status = thd->killed;
    if (killed_status != THD::NOT_KILLED || thd->is_error())
      error = 1;  // Aborted
    int loc_error;
    if (will_batch && (loc_error = table->file->end_bulk_delete())) {
      /* purecov: begin inspected */
      if (error != 1) {
        if (table->file->is_fatal_error(loc_error))
          error_flags |= ME_FATALERROR;

        table->file->print_error(loc_error, error_flags);
      }
      error = 1;
      /* purecov: end */
    }
    if (read_removal) {
      /* Only handler knows how many records were really written */
      deleted_rows = table->file->end_read_removal();
    }
    if (select_lex->active_options() & OPTION_QUICK)
      (void)table->file->ha_extra(HA_EXTRA_NORMAL);
  }  // End of scope for Modification_plan

cleanup:
  DBUG_ASSERT(!lex->is_explain());

  if (!transactional_table && deleted_rows > 0)
    thd->get_transaction()->mark_modified_non_trans_table(
        Transaction_ctx::STMT);

  /* See similar binlogging code in sql_update.cc, for comments */
  if ((error < 0) ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    if (mysql_bin_log.is_open()) {
      int errcode = 0;
      if (error < 0)
        thd->clear_error();
      else
        errcode = query_error_code(thd, killed_status == THD::NOT_KILLED);

      /*
        [binlog]: As we don't allow the use of 'handler:delete_all_rows()' when
        binlog_format == ROW, if 'handler::delete_all_rows()' was called
        we replicate statement-based; otherwise, 'ha_delete_row()' was used to
        delete specific rows which we might log row-based.
      */
      int log_result =
          thd->binlog_query(query_type, thd->query().str, thd->query().length,
                            transactional_table, false, false, errcode);

      if (log_result) {
        error = 1;
      }
    }
  }
  DBUG_ASSERT(
      transactional_table || deleted_rows == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
  if (error < 0) {
    my_ok(thd, deleted_rows);
    DBUG_PRINT("info", ("%ld records deleted", (long)deleted_rows));
  }
  return error > 0;
}

/**
  Prepare a DELETE statement
*/

bool Sql_cmd_delete::prepare_inner(THD *thd) {
  DBUG_TRACE;

  Prepare_error_tracker tracker(thd);

  SELECT_LEX *const select = lex->select_lex;
  TABLE_LIST *const table_list = select->get_table_list();

  bool apply_semijoin;

  Mem_root_array<Item_exists_subselect *> sj_candidates_local(thd->mem_root);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_prepare(trace, "delete_preparation");
  trace_prepare.add_select_number(select->select_number);
  Opt_trace_array trace_steps(trace, "steps");

  if (multitable) {
    if (!select->top_join_list.empty())
      propagate_nullability(&select->top_join_list, false);

    Prepared_stmt_arena_holder ps_holder(thd);
    result = new (thd->mem_root) Query_result_delete();
    if (result == nullptr) return true; /* purecov: inspected */

    // The former is for the pre-iterator executor; the latter is for the
    // iterator executor.
    // TODO(sgunders): Get rid of this when we remove Query_result.
    select->set_query_result(result);
    select->master_unit()->set_query_result(result);

    select->make_active_options(SELECT_NO_JOIN_CACHE | SELECT_NO_UNLOCK,
                                OPTION_BUFFER_RESULT);
    apply_semijoin = true;
    select->set_sj_candidates(&sj_candidates_local);
  } else {
    table_list->updating = true;
    select->make_active_options(0, 0);
    apply_semijoin = false;
  }

  if (select->setup_tables(thd, table_list, false))
    return true; /* purecov: inspected */

  ulong want_privilege_saved = thd->want_privilege;
  thd->want_privilege = SELECT_ACL;
  enum enum_mark_columns mark_used_columns_saved = thd->mark_used_columns;
  thd->mark_used_columns = MARK_COLUMNS_READ;

  if (select->derived_table_count || select->table_func_count) {
    if (select->resolve_placeholder_tables(thd, apply_semijoin)) return true;

    if (select->check_view_privileges(thd, DELETE_ACL, SELECT_ACL)) return true;
  }

  /*
    Deletability test is spread across several places:
    - Target table or view must be updatable (checked below)
    - A view has special requirements with respect to keys
                                          (checked in check_key_in_view)
    - Target table must not be same as one selected from
                                          (checked in unique_table)
  */

  // Check the list of tables to be deleted from
  for (TABLE_LIST *table_ref = table_list; table_ref;
       table_ref = table_ref->next_local) {
    // Skip tables that are only selected from
    if (!table_ref->updating) continue;

    // Cannot delete from a non-updatable view or derived table.
    if (!table_ref->is_updatable()) {
      my_error(ER_NON_UPDATABLE_TABLE, MYF(0), table_ref->alias, "DELETE");
      return true;
    }

    // DELETE does not allow deleting from multi-table views
    if (table_ref->is_multiple_tables()) {
      my_error(ER_VIEW_DELETE_MERGE_VIEW, MYF(0), table_ref->view_db.str,
               table_ref->view_name.str);
      return true;
    }

    if (check_key_in_view(thd, table_ref, table_ref->updatable_base_table())) {
      my_error(ER_NON_UPDATABLE_TABLE, MYF(0), table_ref->alias, "DELETE");
      return true;
    }

    // A view must be merged, and thus cannot have a TABLE
    DBUG_ASSERT(!table_ref->is_view() || table_ref->table == nullptr);

    // Cannot delete from a storage engine that does not support delete.
    TABLE_LIST *base_table = table_ref->updatable_base_table();
    if (base_table->table->file->ha_table_flags() & HA_DELETE_NOT_SUPPORTED) {
      my_error(ER_ILLEGAL_HA, MYF(0), base_table->table_name);
      return true;
    }

    for (TABLE_LIST *tr = base_table; tr != nullptr;
         tr = tr->referencing_view) {
      tr->updating = true;
    }
  }

  // Precompute and store the row types of NATURAL/USING joins.
  if (select->leaf_table_count >= 2 &&
      setup_natural_join_row_types(thd, select->join_list, &select->context))
    return true;

  // Enable the following code if allowing LIMIT with multi-table DELETE
  DBUG_ASSERT(sql_command_code() == SQLCOM_DELETE ||
              select->select_limit == nullptr);

  lex->allow_sum_func = 0;

  if (select->setup_conds(thd)) return true;

  DBUG_ASSERT(select->having_cond() == nullptr &&
              select->group_list.elements == 0 &&
              select->offset_limit == nullptr);

  if (select->master_unit()->prepare_limit(thd, select))
    return true; /* purecov: inspected */

  // check ORDER BY even if it can be ignored
  if (select->order_list.first) {
    TABLE_LIST tables;
    List<Item> fields;
    List<Item> all_fields;

    tables.table = table_list->table;
    tables.alias = table_list->alias;

    DBUG_ASSERT(!select->group_list.elements);
    if (select->setup_base_ref_items(thd)) return true; /* purecov: inspected */
    if (setup_order(thd, select->base_ref_items, &tables, fields, all_fields,
                    select->order_list.first))
      return true;
  }

  thd->want_privilege = want_privilege_saved;
  thd->mark_used_columns = mark_used_columns_saved;

  if (select->has_ft_funcs() && setup_ftfuncs(thd, select))
    return true; /* purecov: inspected */

  /*
    Check tables to be deleted from for duplicate entries -
    must be done after conditions have been prepared.
  */
  select->exclude_from_table_unique_test = true;

  for (TABLE_LIST *table_ref = table_list; table_ref;
       table_ref = table_ref->next_local) {
    if (!table_ref->updating) continue;
    /*
      Check that table from which we delete is not used somewhere
      inside subqueries/view.
    */
    TABLE_LIST *duplicate = unique_table(table_ref->updatable_base_table(),
                                         lex->query_tables, false);
    if (duplicate) {
      update_non_unique_table_error(table_ref, "DELETE", duplicate);
      return true;
    }
  }

  select->exclude_from_table_unique_test = false;

  if (select->query_result() &&
      select->query_result()->prepare(thd, select->fields_list, lex->unit))
    return true; /* purecov: inspected */

  opt_trace_print_expanded_query(thd, select, &trace_wrapper);

  if (select->has_sj_candidates() && select->flatten_subqueries(thd))
    return true;

  select->set_sj_candidates(nullptr);

  if (select->apply_local_transforms(thd, true))
    return true; /* purecov: inspected */

  if (!multitable && select->is_empty_query()) set_empty_query();

  return false;
}

/**
  Execute a DELETE statement.
*/
bool Sql_cmd_delete::execute_inner(THD *thd) {
  return multitable ? Sql_cmd_dml::execute_inner(thd)
                    : delete_from_single_table(thd);
}

/***************************************************************************
  Delete multiple tables from join
***************************************************************************/

extern "C" int refpos_order_cmp(const void *arg, const void *a, const void *b) {
  const handler *file = static_cast<const handler *>(arg);
  return file->cmp_ref(static_cast<const uchar *>(a),
                       static_cast<const uchar *>(b));
}

bool Query_result_delete::prepare(THD *thd, List<Item> &, SELECT_LEX_UNIT *u) {
  DBUG_TRACE;
  unit = u;

  for (TABLE_LIST *tr = u->first_select()->leaf_tables; tr;
       tr = tr->next_leaf) {
    if (tr->updating) {
      // Count number of tables deleted from
      delete_table_count++;

      // Don't use KEYREAD optimization on this table
      tr->table->no_keyread = true;
    }
  }

  THD_STAGE_INFO(thd, stage_deleting_from_main_table);
  return false;
}

/**
  Optimize for deletion from one or more tables in a multi-table DELETE

  Function is called when the join order has been determined.
  Calculate which tables can be deleted from immediately and which tables
  must be delayed. Create objects for handling of delayed deletes.
*/

bool Query_result_delete::optimize() {
  DBUG_TRACE;

  SELECT_LEX *const select = unit->first_select();

  JOIN *const join = select->join;
  THD *thd = join->thd;

  ASSERT_BEST_REF_IN_JOIN_ORDER(join);

  if ((thd->variables.option_bits & OPTION_SAFE_UPDATES) &&
      error_if_full_join(join))
    return true;

  if (!(tempfiles =
            (Unique **)sql_calloc(sizeof(Unique *) * delete_table_count)))
    return true; /* purecov: inspected */

  if (!(tables = (TABLE **)sql_calloc(sizeof(TABLE *) * delete_table_count)))
    return true; /* purecov: inspected */

  bool delete_while_scanning = true;
  for (TABLE_LIST *tr = select->leaf_tables; tr; tr = tr->next_leaf) {
    if (!tr->updating) continue;
    delete_table_map |= tr->map();
    if (delete_while_scanning && unique_table(tr, join->tables_list, false)) {
      /*
        If the table being deleted from is also referenced in the query,
        defer delete so that the delete doesn't interfer with reading of this
        table.
      */
      delete_while_scanning = false;
    }
  }

  for (uint i = 0; i < join->primary_tables; i++) {
    TABLE *const table = join->best_ref[i]->table();
    const table_map map = join->best_ref[i]->table_ref->map();
    if (!(map & delete_table_map)) continue;

    // We are going to delete from this table
    // Don't use record cache
    table->no_cache = true;
    table->covering_keys.clear_all();
    if (table->file->has_transactions())
      transactional_table_map |= map;
    else
      non_transactional_table_map |= map;
    table->prepare_triggers_for_delete_stmt_or_event();
    if (thd->lex->is_ignore()) table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
    table->prepare_for_position();
    table->mark_columns_needed_for_delete(thd);
    if (thd->is_error()) return true;
  }
  /*
    In some cases, rows may be deleted from the first table(s) in the join order
    while performing the join operation when "delete_while_scanning" is true and
      1. deleting from one of the const tables, or
      2. deleting from the first non-const table
  */
  table_map possible_tables = join->const_table_map;  // 1
  if (join->primary_tables > join->const_tables)
    possible_tables |=
        join->best_ref[join->const_tables]->table_ref->map();  // 2
  if (delete_while_scanning)
    delete_immediate = delete_table_map & possible_tables;

  // Set up a Unique object for each table whose delete operation is deferred:

  Unique **tempfile = tempfiles;
  TABLE **table_ptr = tables;
  for (uint i = 0; i < join->primary_tables; i++) {
    const table_map map = join->best_ref[i]->table_ref->map();

    if (!(map & delete_table_map & ~delete_immediate)) continue;

    TABLE *const table = join->best_ref[i]->table();
    if (!(*tempfile++ = new (thd->mem_root)
              Unique(refpos_order_cmp, (void *)table->file,
                     table->file->ref_length, thd->variables.sortbuff_size)))
      return true; /* purecov: inspected */
    *(table_ptr++) = table;
  }

  if (select->has_ft_funcs() && init_ftfuncs(thd, select)) return true;

  return thd->is_fatal_error();
}

void Query_result_delete::cleanup(THD *) {
  // Cleanup only needed if result object has been prepared
  if (delete_table_count == 0) return;

  // Remove optimize structs for this operation.
  for (uint counter = 0; counter < delete_table_count; counter++) {
    if (tempfiles && tempfiles[counter]) destroy(tempfiles[counter]);
  }
  tempfiles = nullptr;
  tables = nullptr;
}

bool Query_result_delete::send_data(THD *thd, List<Item> &) {
  DBUG_TRACE;

  JOIN *const join = unit->first_select()->join;

  int unique_counter = 0;

  for (uint i = 0; i < join->primary_tables; i++) {
    const table_map map = join->qep_tab[i].table_ref->map();

    // Check whether this table is being deleted from
    if (!(map & delete_table_map)) continue;

    const bool immediate = map & delete_immediate;

    TABLE *const table = join->qep_tab[i].table();

    DBUG_ASSERT(immediate || table == tables[unique_counter]);

    /*
      If not doing immediate deletion, increment unique_counter and assign
      "tempfile" here, so that it is available when and if it is needed.
    */
    Unique *const tempfile = immediate ? nullptr : tempfiles[unique_counter++];

    // Check if using outer join and no row found, or row is already deleted
    if (table->has_null_row() || table->has_deleted_row()) continue;

    table->file->position(table->record[0]);
    found_rows++;

    if (immediate) {
      // Rows from this table can be deleted immediately
      if (table->triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                            TRG_ACTION_BEFORE, false))
        return true;
      table->set_deleted_row();
      if (map & non_transactional_table_map) non_transactional_deleted = true;
      if (!(error = table->file->ha_delete_row(table->record[0]))) {
        deleted_rows++;
        if (!table->file->has_transactions())
          thd->get_transaction()->mark_modified_non_trans_table(
              Transaction_ctx::STMT);
        if (table->triggers &&
            table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                              TRG_ACTION_AFTER, false))
          return true;
      } else {
        myf error_flags = MYF(0);
        if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;
        table->file->print_error(error, error_flags);

        /*
          If IGNORE option is used errors caused by ha_delete_row will
          be downgraded to warnings and don't have to stop the iteration.
        */
        if (thd->is_error()) return true;

        /*
          If IGNORE keyword is used, then 'error' variable will have the error
          number which is ignored. Reset the 'error' variable if IGNORE is used.
          This is necessary to call my_ok().
        */
        error = 0;
      }
    } else {
      // Save deletes in a Unique object, to be carried out later.
      error = tempfile->unique_add((char *)table->file->ref);
      if (error) {
        /* purecov: begin inspected */
        error = 1;
        return true;
        /* purecov: end */
      }
    }
  }
  return false;
}

void Query_result_delete::send_error(THD *, uint errcode, const char *err) {
  DBUG_TRACE;

  /* First send error what ever it is ... */
  my_message(errcode, err, MYF(0));
}

void Query_result_delete::abort_result_set(THD *thd) {
  DBUG_TRACE;

  /* the error was handled or nothing deleted and no side effects return */
  if (error_handled ||
      (!thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT) &&
       deleted_rows == 0))
    return;

  /*
    If rows from the first table only has been deleted and it is
    transactional, just do rollback.
    The same if all tables are transactional, regardless of where we are.
    In all other cases do attempt deletes ...
  */
  if (!delete_completed && non_transactional_deleted) {
    /*
      We have to execute the recorded do_deletes() and write info into the
      error log
    */
    error = 1;
    send_eof(thd);
    DBUG_ASSERT(error_handled);
    return;
  }

  if (thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    /*
       there is only side effects; to binlog with the error
    */
    if (mysql_bin_log.is_open()) {
      int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
      /* possible error of writing binary log is ignored deliberately */
      (void)thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                              thd->query().length, transactional_table_map != 0,
                              false, false, errcode);
    }
  }
}

/**
  Do delete from other tables.

  @retval 0 ok
  @retval 1 error
*/

int Query_result_delete::do_deletes(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(!delete_completed);

  delete_completed = true;  // Mark operation as complete
  if (found_rows == 0) return 0;

  for (uint counter = 0; counter < delete_table_count; counter++) {
    TABLE *const table = tables[counter];
    if (table == nullptr) break;

    if (tempfiles[counter]->get(table)) return 1;

    int local_error = do_table_deletes(thd, table);

    if (thd->killed && !local_error) return 1;

    if (local_error == -1)  // End of file
      local_error = 0;

    if (local_error) return local_error;
  }
  return 0;
}

/**
   Implements the inner loop of nested-loops join within multi-DELETE
   execution.

   @param thd   Thread handle.
   @param table The table from which to delete.

   @return Status code

   @retval  0 All ok.
   @retval  1 Triggers or handler reported error.
   @retval -1 End of file from handler.
*/
int Query_result_delete::do_table_deletes(THD *thd, TABLE *table) {
  myf error_flags = MYF(0); /**< Flag for fatal errors */
  int local_error = 0;
  ha_rows last_deleted = deleted_rows;
  DBUG_TRACE;
  /*
    Ignore any rows not found in reference tables as they may already have
    been deleted by foreign key handling
  */
  unique_ptr_destroy_only<RowIterator> iterator = init_table_iterator(
      thd, table, nullptr, false, /*ignore_not_found_rows=*/true);
  if (iterator == nullptr) return 1;
  bool will_batch = !table->file->start_bulk_delete();
  while (!(local_error = iterator->Read()) && !thd->killed) {
    if (table->triggers &&
        table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                          TRG_ACTION_BEFORE, false)) {
      local_error = 1;
      break;
    }

    local_error = table->file->ha_delete_row(table->record[0]);
    if (local_error) {
      if (table->file->is_fatal_error(local_error))
        error_flags |= ME_FATALERROR;

      table->file->print_error(local_error, error_flags);
      /*
        If IGNORE option is used errors caused by ha_delete_row will
        be downgraded to warnings and don't have to stop the iteration.
      */
      if (thd->is_error()) break;
    }

    /*
      Increase the reported number of deleted rows only if no error occurred
      during ha_delete_row.
      Also, don't execute the AFTER trigger if the row operation failed.
    */
    if (!local_error) {
      deleted_rows++;
      if (table->pos_in_table_list->map() & non_transactional_table_map)
        non_transactional_deleted = true;

      if (table->triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_DELETE,
                                            TRG_ACTION_AFTER, false)) {
        local_error = 1;
        break;
      }
    }
  }
  if (will_batch) {
    int tmp_error = table->file->end_bulk_delete();
    if (tmp_error && !local_error) {
      local_error = tmp_error;
      if (table->file->is_fatal_error(local_error))
        error_flags |= ME_FATALERROR;

      table->file->print_error(local_error, error_flags);
    }
  }
  if (last_deleted != deleted_rows && !table->file->has_transactions())
    thd->get_transaction()->mark_modified_non_trans_table(
        Transaction_ctx::STMT);

  return local_error;
}

/**
  Send ok to the client

  The function has to perform all deferred deletes that have been queued up.

  @return false if success, true if error
*/

bool Query_result_delete::send_eof(THD *thd) {
  THD::killed_state killed_status = THD::NOT_KILLED;
  THD_STAGE_INFO(thd, stage_deleting_from_reference_tables);

  /* Does deletes for the last n - 1 tables, returns 0 if ok */
  int local_error = do_deletes(thd);  // returns 0 if success

  /* compute a total error to know if something failed */
  local_error = local_error || error;
  killed_status = (local_error == 0) ? THD::NOT_KILLED : thd->killed.load();
  /* reset used flags */

  if ((local_error == 0) ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    if (mysql_bin_log.is_open()) {
      int errcode = 0;
      if (local_error == 0)
        thd->clear_error();
      else
        errcode = query_error_code(thd, killed_status == THD::NOT_KILLED);
      thd->thread_specific_used = true;
      if (thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                            thd->query().length, transactional_table_map != 0,
                            false, false, errcode) &&
          !non_transactional_table_map) {
        local_error = 1;  // Log write failed: roll back the SQL statement
      }
    }
  }
  if (local_error != 0)
    error_handled = true;  // to force early leave from ::send_error()

  if (!local_error && !thd->is_error()) {
    ::my_ok(thd, deleted_rows);
  }
  return thd->is_error();
}

bool Sql_cmd_delete::accept(THD *thd, Select_lex_visitor *visitor) {
  return thd->lex->unit->accept(visitor);
}
