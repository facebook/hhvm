/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

// Handle UPDATE queries (both single- and multi-table).

#include "sql/sql_update.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <utility>

#include "field_types.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_bit.h"  // my_count_bits
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "prealloced_array.h"  // Prealloced_array
#include "scope_guard.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_grant, check_access
#include "sql/basic_row_iterators.h"
#include "sql/binlog.h"  // mysql_bin_log
#include "sql/composite_iterators.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/derror.h"      // ER_THD
#include "sql/field.h"       // Field
#include "sql/filesort.h"    // Filesort
#include "sql/handler.h"
#include "sql/item.h"            // Item
#include "sql/item_json_func.h"  // Item_json_func
#include "sql/key.h"             // is_key_used
#include "sql/key_spec.h"
#include "sql/locked_tables_list.h"
#include "sql/mem_root_array.h"
#include "sql/mysqld.h"       // stage_... mysql_tmpdir
#include "sql/opt_explain.h"  // Modification_plan
#include "sql/opt_explain_format.h"
#include "sql/opt_range.h"  // QUICK_SELECT_I
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_node_base.h"
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/records.h"  // unique_ptr_destroy_only<RowIterator>
#include "sql/row_iterator.h"
#include "sql/select_lex_visitor.h"
#include "sql/sorting_iterator.h"
#include "sql/sql_array.h"
#include "sql/sql_base.h"  // check_record, fill_record
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_data_change.h"
#include "sql/sql_error.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"  // build_equal_items, substitute_gc
#include "sql/sql_partition.h"  // partition_key_modified
#include "sql/sql_resolver.h"   // setup_order
#include "sql/sql_select.h"
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/sql_view.h"       // check_key_in_view
#include "sql/system_variables.h"
#include "sql/table.h"                     // TABLE
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/temp_table_param.h"
#include "sql/thd_raii.h"
#include "sql/timing_iterator.h"
#include "sql/transaction_info.h"
#include "sql/trigger_def.h"
#include "template_utils.h"
#include "thr_lock.h"

class COND_EQUAL;
class Item_exists_subselect;

bool Sql_cmd_update::precheck(THD *thd) {
  DBUG_TRACE;

  if (!multitable) {
    if (check_one_table_access(thd, UPDATE_ACL, lex->query_tables)) return true;
  } else {
    /*
      Ensure that we have UPDATE or SELECT privilege for each table
      The exact privilege is checked in mysql_multi_update()
    */
    for (TABLE_LIST *tr = lex->query_tables; tr; tr = tr->next_global) {
      /*
        "uses_materialization()" covers the case where a prepared statement is
        executed and a view is decided to be materialized during preparation.
        @todo: Check whether this properly handles the case when privileges
        for a view is revoked during execution of a prepared statement.
      */
      if (tr->is_derived() || tr->uses_materialization())
        tr->grant.privilege = SELECT_ACL;
      else if ((check_access(thd, UPDATE_ACL, tr->db, &tr->grant.privilege,
                             &tr->grant.m_internal, false, true) ||
                check_grant(thd, UPDATE_ACL, tr, false, 1, true)) &&
               (check_access(thd, SELECT_ACL, tr->db, &tr->grant.privilege,
                             &tr->grant.m_internal, false, false) ||
                check_grant(thd, SELECT_ACL, tr, false, 1, false)))
        return true;
    }
  }
  return false;
}

/**
   True if the table's input and output record buffers are comparable using
   compare_records(TABLE*).
 */
bool records_are_comparable(const TABLE *table) {
  return ((table->file->ha_table_flags() & HA_PARTIAL_COLUMN_READ) == 0) ||
         bitmap_is_subset(table->write_set, table->read_set);
}

/**
   Compares the input and outbut record buffers of the table to see if a row
   has changed. The algorithm iterates over updated columns and if they are
   nullable compares NULL bits in the buffer before comparing actual
   data. Special care must be taken to compare only the relevant NULL bits and
   mask out all others as they may be undefined. The storage engine will not
   and should not touch them.

   @param table The table to evaluate.

   @return true if row has changed.
   @return false otherwise.
*/
bool compare_records(const TABLE *table) {
  DBUG_ASSERT(records_are_comparable(table));

  if ((table->file->ha_table_flags() & HA_PARTIAL_COLUMN_READ) != 0) {
    /*
      Storage engine may not have read all columns of the record.  Fields
      (including NULL bits) not in the write_set may not have been read and
      can therefore not be compared.
    */
    for (Field **ptr = table->field; *ptr != nullptr; ptr++) {
      Field *field = *ptr;
      if (bitmap_is_set(table->write_set, field->field_index)) {
        if (field->is_nullable()) {
          uchar null_byte_index = field->null_offset();

          if (((table->record[0][null_byte_index]) & field->null_bit) !=
              ((table->record[1][null_byte_index]) & field->null_bit))
            return true;
        }
        if (field->cmp_binary_offset(table->s->rec_buff_length)) return true;
      }
    }
    return false;
  }

  /*
     The storage engine has read all columns, so it's safe to compare all bits
     including those not in the write_set. This is cheaper than the
     field-by-field comparison done above.
  */
  if (table->s->blob_fields + table->s->varchar_fields == 0)
    // Fixed-size record: do bitwise comparison of the records
    return cmp_record(table, record[1]);
  /* Compare null bits */
  if (memcmp(table->null_flags, table->null_flags + table->s->rec_buff_length,
             table->s->null_bytes))
    return true;  // Diff in NULL value
  /* Compare updated fields */
  for (Field **ptr = table->field; *ptr; ptr++) {
    if (bitmap_is_set(table->write_set, (*ptr)->field_index) &&
        (*ptr)->cmp_binary_offset(table->s->rec_buff_length))
      return true;
  }
  return false;
}

/**
  Check that all fields are base table columns.
  Replace columns from views with base table columns.

  @param      thd              thread handler
  @param      items            Items for check

  @return false if success, true if error (Items not updatable columns or OOM)
*/

static bool check_fields(THD *thd, List<Item> &items) {
  List_iterator<Item> it(items);
  Item *item;

  while ((item = it++)) {
    /*
      we make temporary copy of Item_field, to avoid influence of changing
      result_field on Item_ref which refer on this field
    */
    Item_field *const base_table_field = item->field_for_view_update();
    DBUG_ASSERT(base_table_field != nullptr);

    Item_field *const cloned_field = new Item_field(thd, base_table_field);
    if (!cloned_field) return true; /* purecov: inspected */

    thd->change_item_tree(it.ref(), cloned_field);
  }
  return false;
}

/**
  Check if all expressions in list are constant expressions

  @param[in] values List of expressions

  @retval true Only constant expressions
  @retval false At least one non-constant expression
*/

static bool check_constant_expressions(List<Item> *values) {
  Item *value;
  List_iterator_fast<Item> v(*values);
  DBUG_TRACE;

  while ((value = v++)) {
    if (!value->const_item()) {
      DBUG_PRINT("exit", ("expression is not constant"));
      return false;
    }
  }
  DBUG_PRINT("exit", ("expression is constant"));
  return true;
}

/**
  Perform an update to a set of rows in a single table.

  @param thd     Thread handler

  @returns false if success, true if error
*/

bool Sql_cmd_update::update_single_table(THD *thd) {
  DBUG_TRACE;

  myf error_flags = MYF(0); /**< Flag for fatal errors */
  /*
    Most recent handler error
    =  1: Some non-handler error
    =  0: Success
    = -1: No more rows to process, or reached limit
  */
  int error = 0;

  SELECT_LEX *const select_lex = lex->select_lex;
  SELECT_LEX_UNIT *const unit = lex->unit;
  TABLE_LIST *const table_list = select_lex->get_table_list();
  TABLE_LIST *const update_table_ref = table_list->updatable_base_table();
  TABLE *const table = update_table_ref->table;

  DBUG_ASSERT(table->pos_in_table_list == update_table_ref);

  const bool transactional_table = table->file->has_transactions();

  const bool has_update_triggers =
      table->triggers && table->triggers->has_update_triggers();

  const bool has_after_triggers =
      has_update_triggers &&
      table->triggers->has_triggers(TRG_EVENT_UPDATE, TRG_ACTION_AFTER);

  List<Item> *update_field_list = &select_lex->item_list;

  if (unit->set_limit(thd, unit->global_parameters()))
    return true; /* purecov: inspected */

  ha_rows limit = unit->select_limit_cnt;
  const bool using_limit = limit != HA_POS_ERROR;

  // Used to track whether there are no rows that need to be read
  bool no_rows = limit == 0;

  THD::killed_state killed_status = THD::NOT_KILLED;
  COPY_INFO update(COPY_INFO::UPDATE_OPERATION, update_field_list,
                   update_value_list);
  if (update.add_function_default_columns(table, table->write_set)) return true;

  const bool safe_update = thd->variables.option_bits & OPTION_SAFE_UPDATES;

  QEP_TAB_standalone qep_tab_st;
  QEP_TAB &qep_tab = qep_tab_st.as_QEP_TAB();

  if (table->all_partitions_pruned_away || m_empty_query) {
    /*
      All partitions were pruned away during preparation. Shortcut further
      processing by "no rows". If explaining, report the plan and bail out.
    */
    no_rows = true;

    if (lex->is_explain()) {
      Modification_plan plan(thd, MT_UPDATE, table,
                             "No matching rows after partition pruning", true,
                             0);
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }
  }
  Item *conds = nullptr;
  ORDER *order = select_lex->order_list.first;
  if (!no_rows && select_lex->get_optimizable_conditions(thd, &conds, nullptr))
    return true; /* purecov: inspected */

  /*
    Reset the field list to remove any hidden fields added by substitute_gc() in
    the previous execution.
  */
  select_lex->all_fields = select_lex->fields_list;

  /*
    See if we can substitute expressions with equivalent generated
    columns in the WHERE and ORDER BY clauses of the UPDATE statement.
    It is unclear if this is best to do before or after the other
    substitutions performed by substitute_for_best_equal_field(). Do
    it here for now, to keep it consistent with how multi-table
    updates are optimized in JOIN::optimize().
  */
  if (conds || order)
    static_cast<void>(substitute_gc(thd, select_lex, conds, nullptr, order));

  if (conds != nullptr) {
    COND_EQUAL *cond_equal = nullptr;
    Item::cond_result result;
    if (table_list->check_option) {
      /*
        If this UPDATE is on a view with CHECK OPTION, field references in
        'conds' must not be replaced by constants. The reason is that when
        'conds' is optimized, 'check_option' is also optimized (it is
        part of 'conds'). Const replacement is fine for 'conds'
        because it is evaluated on a read row, but 'check_option' is
        evaluated on a row with updated fields and needs those updated
        values to be correct.

        Example:
        CREATE VIEW v1 ... WHERE fld < 2 WITH CHECK_OPTION
        UPDATE v1 SET fld=4 WHERE fld=1

        check_option is  "(fld < 2)"
        conds is         "(fld < 2) and (fld = 1)"

        optimize_cond() would propagate fld=1 to the first argument of
        the AND to create "(1 < 2) AND (fld = 1)". After this,
        check_option would be "(1 < 2)". But for check_option to work
        it must be evaluated with the *updated* value of fld: 4.
        Otherwise it will evaluate to true even when it should be
        false, which is the case for the UPDATE statement above.

        Thus, if there is a check_option, we do only the "safe" parts
        of optimize_cond(): Item_row -> Item_func_eq conversion (to
        enable range access) and removal of always true/always false
        predicates.

        An alternative to restricting this optimization of 'conds' in
        the presense of check_option: the Item-tree of 'check_option'
        could be cloned before optimizing 'conds' and thereby avoid
        const replacement. However, at the moment there is no such
        thing as Item::clone().
      */
      if (build_equal_items(thd, conds, &conds, nullptr, false,
                            select_lex->join_list, &cond_equal))
        return true;
      if (remove_eq_conds(thd, conds, &conds, &result))
        return true; /* purecov: inspected */
    } else {
      if (optimize_cond(thd, &conds, &cond_equal, select_lex->join_list,
                        &result))
        return true;
    }

    if (result == Item::COND_FALSE) {
      no_rows = true;  // Impossible WHERE
      if (thd->lex->is_explain()) {
        Modification_plan plan(thd, MT_UPDATE, table, "Impossible WHERE", true,
                               0);
        bool err =
            explain_single_table_modification(thd, thd, &plan, select_lex);
        return err;
      }
    }
    if (conds != nullptr) {
      conds = substitute_for_best_equal_field(thd, conds, cond_equal, nullptr);
      if (conds == nullptr) return true;

      conds->update_used_tables();
    }
  }

  /*
    Also try a second time after locking, to prune when subqueries and
    stored programs can be evaluated.
  */
  if (table->part_info) {
    if (prune_partitions(thd, table, conds))
      return true; /* purecov: inspected */
    if (table->all_partitions_pruned_away) {
      no_rows = true;

      if (thd->lex->is_explain()) {
        Modification_plan plan(thd, MT_UPDATE, table,
                               "No matching rows after partition pruning", true,
                               0);
        bool err =
            explain_single_table_modification(thd, thd, &plan, select_lex);
        return err;
      }
      my_ok(thd);
      return false;
    }
  }
  // Initialize the cost model that will be used for this table
  table->init_cost_model(thd->cost_model());

  /* Update the table->file->stats.records number */
  table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);

  table->mark_columns_needed_for_update(thd,
                                        false /*mark_binlog_columns=false*/);

  qep_tab.set_table(table);
  qep_tab.set_condition(conds);

  if (conds &&
      thd->optimizer_switch_flag(OPTIMIZER_SWITCH_ENGINE_CONDITION_PUSHDOWN)) {
    table->file->cond_push(conds, false);
  }

  {  // Enter scope for optimizer trace wrapper
    Opt_trace_object wrapper(&thd->opt_trace);
    wrapper.add_utf8_table(update_table_ref);

    if (!no_rows && conds != nullptr) {
      Key_map keys_to_use(Key_map::ALL_BITS), needed_reg_dummy;
      QUICK_SELECT_I *qck;
      no_rows = test_quick_select(thd, keys_to_use, 0, limit, safe_update,
                                  ORDER_NOT_RELEVANT, &qep_tab, conds,
                                  &needed_reg_dummy, &qck,
                                  qep_tab.table()->force_index) < 0;
      qep_tab.set_quick(qck);
      if (thd->is_error()) return true;
    }
    if (no_rows) {
      if (thd->lex->is_explain()) {
        Modification_plan plan(thd, MT_UPDATE, table, "Impossible WHERE", true,
                               0);
        bool err =
            explain_single_table_modification(thd, thd, &plan, select_lex);
        return err;
      }

      char buff[MYSQL_ERRMSG_SIZE];
      snprintf(buff, sizeof(buff), ER_THD(thd, ER_UPDATE_INFO), 0L, 0L,
               (long)thd->get_stmt_da()->current_statement_cond_count());
      my_ok(thd, 0, 0, buff);

      DBUG_PRINT("info", ("0 records updated"));
      return false;
    }
  }  // Ends scope for optimizer trace wrapper

  /* If running in safe sql mode, don't allow updates without keys */
  if (table->quick_keys.is_clear_all()) {
    thd->server_status |= SERVER_QUERY_NO_INDEX_USED;

    /*
      No safe update error will be returned if:
      1) Statement is an EXPLAIN OR
      2) LIMIT is present.

      Append the first warning (if any) to the error message. Allows the user
      to understand why index access couldn't be chosen.
    */
    if (!lex->is_explain() && safe_update && !using_limit) {
      my_error(ER_UPDATE_WITHOUT_KEY_IN_SAFE_MODE, MYF(0),
               thd->get_stmt_da()->get_first_condition_message());
      return true;
    }
  }
  if (select_lex->has_ft_funcs() && init_ftfuncs(thd, select_lex))
    return true; /* purecov: inspected */

  if (table->update_const_key_parts(conds)) return true;

  order = simple_remove_const(order, conds);
  bool need_sort;
  bool reverse = false;
  bool used_key_is_modified = false;
  uint used_index;
  {
    ORDER_with_src order_src(order, ESC_ORDER_BY);
    used_index =
        get_index_for_order(&order_src, &qep_tab, limit, &need_sort, &reverse);
  }
  if (need_sort) {  // Assign table scan index to check below for modified key
                    // fields:
    used_index = table->file->key_used_on_scan;
  }
  if (used_index != MAX_KEY) {  // Check if we are modifying a key that we are
                                // used to search with:
    used_key_is_modified = is_key_used(table, used_index, table->write_set);
  } else if (qep_tab.quick()) {
    /*
      select->quick != NULL and used_index == MAX_KEY happens for index
      merge and should be handled in a different way.
    */
    used_key_is_modified = (!qep_tab.quick()->unique_key_range() &&
                            qep_tab.quick()->is_keys_used(table->write_set));
  }

  if (table->part_info)
    used_key_is_modified |= partition_key_modified(table, table->write_set);

  const bool using_filesort = order && need_sort;

  table->mark_columns_per_binlog_row_image(thd);

  if (table->setup_partial_update()) return true; /* purecov: inspected */

  ha_rows updated_rows = 0;
  ha_rows found_rows = 0;

  unique_ptr_destroy_only<Filesort> fsort;
  unique_ptr_destroy_only<RowIterator> iterator;

  {  // Start of scope for Modification_plan
    ha_rows rows;
    if (qep_tab.quick())
      rows = qep_tab.quick()->records;
    else if (!conds && !need_sort && limit != HA_POS_ERROR)
      rows = limit;
    else {
      update_table_ref->fetch_number_of_rows();
      rows = table->file->stats.records;
    }
    qep_tab.set_quick_optim();
    qep_tab.set_condition_optim();
    DEBUG_SYNC(thd, "before_single_update");
    Modification_plan plan(thd, MT_UPDATE, &qep_tab, used_index, limit,
                           (!using_filesort && (used_key_is_modified || order)),
                           using_filesort, used_key_is_modified, rows);
    DEBUG_SYNC(thd, "planned_single_update");
    if (thd->lex->is_explain()) {
      bool err = explain_single_table_modification(thd, thd, &plan, select_lex);
      return err;
    }

    if (thd->lex->is_ignore()) table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);

    if (used_key_is_modified || order) {
      /*
        We can't update table directly;  We must first search after all
        matching rows before updating the table!
      */

      /* note: We avoid sorting if we sort on the used index */
      if (using_filesort) {
        /*
          Doing an ORDER BY;  Let filesort find and sort the rows we are going
          to update
          NOTE: filesort will call table->prepare_for_position()
        */
        ha_rows examined_rows = 0;
        iterator =
            create_table_iterator(thd, nullptr, &qep_tab, false,
                                  /*ignore_not_found_rows=*/false,
                                  &examined_rows, /*using_table_scan=*/nullptr);

        if (qep_tab.condition() != nullptr) {
          iterator = NewIterator<FilterIterator>(thd, move(iterator),
                                                 qep_tab.condition());
        }

        // Force filesort to sort by position.
        fsort.reset(new (thd->mem_root) Filesort(
            thd, qep_tab.table(), /*keep_buffers=*/false, order, limit,
            /*force_stable_sort=*/false,
            /*remove_duplicates=*/false,
            /*force_sort_positions=*/true));
        iterator = NewIterator<SortingIterator>(thd, &qep_tab, fsort.get(),
                                                move(iterator),
                                                /*examined_rows=*/nullptr);
        if (iterator->Init()) return true;
        thd->inc_examined_row_count(examined_rows);

        /*
          Filesort has already found and selected the rows we want to update,
          so we don't need the where clause
        */
        qep_tab.set_quick(nullptr);
        qep_tab.set_condition(nullptr);
      } else {
        /*
          We are doing a search on a key that is updated. In this case
          we go trough the matching rows, save a pointer to them and
          update these in a separate loop based on the pointer. In the end,
          we get a result file that looks exactly like what filesort uses
          internally, which allows us to read from it
          using SortFileIndirectIterator.

          TODO: Find something less ugly.
         */
        Key_map covering_keys_for_cond;  // @todo - move this
        if (used_index < MAX_KEY && covering_keys_for_cond.is_set(used_index))
          table->set_keyread(true);

        table->prepare_for_position();

        /* If quick select is used, initialize it before retrieving rows. */
        if (qep_tab.quick() && (error = qep_tab.quick()->reset())) {
          if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

          table->file->print_error(error, error_flags);
          return true;
        }
        table->file->try_semi_consistent_read(true);
        auto end_semi_consistent_read = create_scope_guard(
            [table] { table->file->try_semi_consistent_read(false); });

        /*
          When we get here, we have one of the following options:
          A. used_index == MAX_KEY
          This means we should use full table scan, and start it with
          init_read_record call
          B. used_index != MAX_KEY
          B.1 quick select is used, start the scan with init_read_record
          B.2 quick select is not used, this is full index scan (with LIMIT)
          Full index scan must be started with init_read_record_idx
        */

        if (used_index == MAX_KEY || qep_tab.quick()) {
          iterator = create_table_iterator(thd, nullptr, &qep_tab, false,
                                           /*ignore_not_found_rows=*/false,
                                           /*examined_rows=*/nullptr,
                                           /*using_table_scan=*/nullptr);
        } else {
          iterator = create_table_iterator_idx(thd, table, used_index, reverse,
                                               &qep_tab);
        }

        if (iterator->Init()) {
          return true;
        }

        THD_STAGE_INFO(thd, stage_searching_rows_for_update);
        ha_rows tmp_limit = limit;

        IO_CACHE *tempfile =
            (IO_CACHE *)my_malloc(key_memory_TABLE_sort_io_cache,
                                  sizeof(IO_CACHE), MYF(MY_FAE | MY_ZEROFILL));

        if (open_cached_file(tempfile, mysql_tmpdir, TEMP_PREFIX,
                             DISK_BUFFER_SIZE, MYF(MY_WME))) {
          my_free(tempfile);
          return true;
        }

        while (!(error = iterator->Read()) && !thd->killed) {
          DBUG_ASSERT(!thd->is_error());
          thd->inc_examined_row_count(1);

          if (qep_tab.condition() != nullptr) {
            const bool skip_record = qep_tab.condition()->val_int() == 0;
            if (thd->is_error()) {
              error = 1;
              /*
                Don't try unlocking the row if skip_record reported an error
                since in this case the transaction might have been rolled back
                already.
              */
              break;
            }
            if (skip_record) {
              table->file->unlock_row();
              continue;
            }
          }
          if (table->file->was_semi_consistent_read())
            continue; /* repeat the read of the same row if it still exists */

          table->file->position(table->record[0]);
          if (my_b_write(tempfile, table->file->ref, table->file->ref_length)) {
            error = 1; /* purecov: inspected */
            break;     /* purecov: inspected */
          }
          if (!--limit && using_limit) {
            error = -1;
            break;
          }
        }

        if (thd->killed && !error)  // Aborted
          error = 1;                /* purecov: inspected */
        limit = tmp_limit;
        end_semi_consistent_read.rollback();
        if (used_index < MAX_KEY && covering_keys_for_cond.is_set(used_index))
          table->set_keyread(false);
        table->file->ha_index_or_rnd_end();
        iterator.reset();

        // Change reader to use tempfile
        if (reinit_io_cache(tempfile, READ_CACHE, 0L, false, false))
          error = 1; /* purecov: inspected */

        if (error >= 0) {
          close_cached_file(tempfile);
          my_free(tempfile);
          return error > 0;
        }

        iterator = NewIterator<SortFileIndirectIterator>(
            thd, table, tempfile,
            /*request_cache=*/false,
            /*ignore_not_found_rows=*/false,
            /*examined_rows=*/nullptr);
        if (iterator->Init()) return true;

        qep_tab.set_quick(nullptr);
        qep_tab.set_condition(nullptr);
      }
    } else {
      // No ORDER BY or updated key underway, so we can use a regular read.
      iterator = init_table_iterator(thd, nullptr, &qep_tab, false,
                                     /*ignore_not_found_rows=*/false);
      if (iterator == nullptr) return true; /* purecov: inspected */
    }

    table->file->try_semi_consistent_read(true);
    auto end_semi_consistent_read = create_scope_guard(
        [table] { table->file->try_semi_consistent_read(false); });

    /*
      Generate an error (in TRADITIONAL mode) or warning
      when trying to set a NOT NULL field to NULL.
    */
    thd->check_for_truncated_fields = CHECK_FIELD_WARN;
    thd->num_truncated_fields = 0L;
    THD_STAGE_INFO(thd, stage_updating);

    bool will_batch;
    /// read_removal is only used by NDB storage engine
    bool read_removal = false;

    if (table->prepare_triggers_for_update_stmt_or_event()) {
      will_batch = false;
    } else {
      // No after update triggers, attempt to start bulk update
      will_batch = !table->file->start_bulk_update();
    }
    if ((table->file->ha_table_flags() & HA_READ_BEFORE_WRITE_REMOVAL) &&
        !thd->lex->is_ignore() && !using_limit && !has_update_triggers &&
        qep_tab.quick() && qep_tab.quick()->index != MAX_KEY &&
        check_constant_expressions(update_value_list))
      read_removal = table->check_read_removal(qep_tab.quick()->index);

    // If the update is batched, we cannot do partial update, so turn it off.
    if (will_batch) table->cleanup_partial_update(); /* purecov: inspected */

    uint dup_key_found;

    while (true) {
      error = iterator->Read();
      if (error || thd->killed) break;
      thd->inc_examined_row_count(1);
      if (qep_tab.condition() != nullptr) {
        const bool skip_record = qep_tab.condition()->val_int() == 0;
        if (thd->is_error()) {
          error = 1;
          break;
        }
        if (skip_record) {
          table->file
              ->unlock_row();  // Row failed condition check, release lock
          thd->get_stmt_da()->inc_current_row_for_condition();
          continue;
        }
      }
      DBUG_ASSERT(!thd->is_error());

      if (table->file->was_semi_consistent_read())
        continue; /* repeat the read of the same row if it still exists */

      table->clear_partial_update_diffs();

      store_record(table, record[1]);
      bool is_row_changed = false;
      if (fill_record_n_invoke_before_triggers(
              thd, &update, *update_field_list, *update_value_list, table,
              TRG_EVENT_UPDATE, 0, false, &is_row_changed)) {
        error = 1;
        break;
      }
      found_rows++;

      if (is_row_changed) {
        /*
          Default function and default expression values are filled before
          evaluating the view check option. Check option on view using table(s)
          with default function and default expression breaks otherwise.

          It is safe to not invoke CHECK OPTION for VIEW if records are same.
          In this case the row is coming from the view and thus should satisfy
          the CHECK OPTION.
        */
        int check_result = table_list->view_check_option(thd);
        if (check_result != VIEW_CHECK_OK) {
          if (check_result == VIEW_CHECK_SKIP)
            continue;
          else if (check_result == VIEW_CHECK_ERROR) {
            error = 1;
            break;
          }
        }

        /*
          Existing rows in table should normally satisfy CHECK constraints. So
          it should be safe to check constraints only for rows that has really
          changed (i.e. after compare_records()).

          In future, once addition/enabling of CHECK constraints without their
          validation is supported, we might encounter old rows which do not
          satisfy CHECK constraints currently enabled. However, rejecting no-op
          updates to such invalid pre-existing rows won't make them valid and is
          probably going to be confusing for users. So it makes sense to stick
          to current behavior.
        */
        if (invoke_table_check_constraints(thd, table)) {
          if (thd->is_error()) {
            error = 1;
            break;
          }
          // continue when IGNORE clause is used.
          continue;
        }

        if (will_batch) {
          /*
            Typically a batched handler can execute the batched jobs when:
            1) When specifically told to do so
            2) When it is not a good idea to batch anymore
            3) When it is necessary to send batch for other reasons
            (One such reason is when READ's must be performed)

            1) is covered by exec_bulk_update calls.
            2) and 3) is handled by the bulk_update_row method.

            bulk_update_row can execute the updates including the one
            defined in the bulk_update_row or not including the row
            in the call. This is up to the handler implementation and can
            vary from call to call.

            The dup_key_found reports the number of duplicate keys found
            in those updates actually executed. It only reports those if
            the extra call with HA_EXTRA_IGNORE_DUP_KEY have been issued.
            If this hasn't been issued it returns an error code and can
            ignore this number. Thus any handler that implements batching
            for UPDATE IGNORE must also handle this extra call properly.

            If a duplicate key is found on the record included in this
            call then it should be included in the count of dup_key_found
            and error should be set to 0 (only if these errors are ignored).
          */
          error = table->file->ha_bulk_update_row(
              table->record[1], table->record[0], &dup_key_found);
          limit += dup_key_found;
          updated_rows -= dup_key_found;
        } else {
          /* Non-batched update */
          error =
              table->file->ha_update_row(table->record[1], table->record[0]);
        }
        if (error == 0)
          updated_rows++;
        else if (error == HA_ERR_RECORD_IS_THE_SAME)
          error = 0;
        else {
          if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

          table->file->print_error(error, error_flags);

          // The error can have been downgraded to warning by IGNORE.
          if (thd->is_error()) break;
        }
      }

      if (!error && has_after_triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_UPDATE,
                                            TRG_ACTION_AFTER, true)) {
        error = 1;
        break;
      }

      if (!--limit && using_limit) {
        /*
          We have reached end-of-file in most common situations where no
          batching has occurred and if batching was supposed to occur but
          no updates were made and finally when the batch execution was
          performed without error and without finding any duplicate keys.
          If the batched updates were performed with errors we need to
          check and if no error but duplicate key's found we need to
          continue since those are not counted for in limit.
        */
        if (will_batch &&
            ((error = table->file->exec_bulk_update(&dup_key_found)) ||
             dup_key_found)) {
          if (error) {
            /* purecov: begin inspected */
            DBUG_ASSERT(false);
            /*
              The handler should not report error of duplicate keys if they
              are ignored. This is a requirement on batching handlers.
            */
            if (table->file->is_fatal_error(error))
              error_flags |= ME_FATALERROR;

            table->file->print_error(error, error_flags);
            error = 1;
            break;
            /* purecov: end */
          }
          /*
            Either an error was found and we are ignoring errors or there
            were duplicate keys found. In both cases we need to correct
            the counters and continue the loop.
          */
          limit = dup_key_found;  // limit is 0 when we get here so need to +
          updated_rows -= dup_key_found;
        } else {
          error = -1;  // Simulate end of file
          break;
        }
      }

      thd->get_stmt_da()->inc_current_row_for_condition();
      DBUG_ASSERT(!thd->is_error());
      if (thd->is_error()) {
        error = 1;
        break;
      }
    }
    end_semi_consistent_read.rollback();

    dup_key_found = 0;
    /*
      Caching the killed status to pass as the arg to query event constuctor;
      The cached value can not change whereas the killed status can
      (externally) since this point and change of the latter won't affect
      binlogging.
      It's assumed that if an error was set in combination with an effective
      killed status then the error is due to killing.
    */
    killed_status = thd->killed;  // get the status of the atomic
    // simulated killing after the loop must be ineffective for binlogging
    DBUG_EXECUTE_IF("simulate_kill_bug27571",
                    { thd->killed = THD::KILL_QUERY; };);
    if (killed_status != THD::NOT_KILLED) error = 1;

    int loc_error;
    if (error && will_batch &&
        (loc_error = table->file->exec_bulk_update(&dup_key_found)))
    /*
      An error has occurred when a batched update was performed and returned
      an error indication. It cannot be an allowed duplicate key error since
      we require the batching handler to treat this as a normal behavior.

      Otherwise we simply remove the number of duplicate keys records found
      in the batched update.
    */
    {
      /* purecov: begin inspected */
      error_flags = MYF(0);
      if (table->file->is_fatal_error(loc_error)) error_flags |= ME_FATALERROR;

      table->file->print_error(loc_error, error_flags);
      error = 1;
      /* purecov: end */
    } else
      updated_rows -= dup_key_found;
    if (will_batch) table->file->end_bulk_update();

    if (read_removal) {
      /* Only handler knows how many records really was written */
      updated_rows = table->file->end_read_removal();
      if (!records_are_comparable(table)) found_rows = updated_rows;
    }

  }  // End of scope for Modification_plan

  if (!transactional_table && updated_rows > 0)
    thd->get_transaction()->mark_modified_non_trans_table(
        Transaction_ctx::STMT);

  iterator.reset();

  /*
    error < 0 means really no error at all: we processed all rows until the
    last one without error. error > 0 means an error (e.g. unique key
    violation and no IGNORE or REPLACE). error == 0 is also an error (if
    preparing the record or invoking before triggers fails). See
    ha_autocommit_or_rollback(error>=0) and return error>=0 below.
    Sometimes we want to binlog even if we updated no rows, in case user used
    it to be sure master and slave are in same state.
  */
  if ((error < 0) ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    if (mysql_bin_log.is_open()) {
      int errcode = 0;
      if (error < 0)
        thd->clear_error();
      else
        errcode = query_error_code(thd, killed_status == THD::NOT_KILLED);

      if (thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                            thd->query().length, transactional_table, false,
                            false, errcode)) {
        error = 1;  // Rollback update
      }
    }
  }
  DBUG_ASSERT(
      transactional_table || updated_rows == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));

  // If LAST_INSERT_ID(X) was used, report X
  const ulonglong id = thd->arg_of_last_insert_id_function
                           ? thd->first_successful_insert_id_in_prev_stmt
                           : 0;

  if (error < 0) {
    char buff[MYSQL_ERRMSG_SIZE];
    snprintf(buff, sizeof(buff), ER_THD(thd, ER_UPDATE_INFO), (long)found_rows,
             (long)updated_rows,
             (long)thd->get_stmt_da()->current_statement_cond_count());
    my_ok(thd,
          thd->get_protocol()->has_client_capability(CLIENT_FOUND_ROWS)
              ? found_rows
              : updated_rows,
          id, buff);
    DBUG_PRINT("info", ("%ld records updated", (long)updated_rows));
  }
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;
  thd->current_found_rows = found_rows;
  // Following test is disabled, as we get RQG errors that are hard to debug
  // DBUG_ASSERT((error >= 0) == thd->is_error());
  return error >= 0 || thd->is_error();
}

/***************************************************************************
  Update multiple tables from join
***************************************************************************/

/*
  Get table map for list of Item_field
*/

static table_map get_table_map(List<Item> *items) {
  List_iterator_fast<Item> item_it(*items);
  Item_field *item;
  table_map map = 0;

  while ((item = (Item_field *)item_it++)) map |= item->used_tables();
  DBUG_PRINT("info", ("table_map: 0x%08lx", (long)map));
  return map;
}

/**
  If one row is updated through two different aliases and the first
  update physically moves the row, the second update will error
  because the row is no longer located where expected. This function
  checks if the multiple-table update is about to do that and if so
  returns with an error.

  The following update operations physically moves rows:
    1) Update of a column in a clustered primary key
    2) Update of a column used to calculate which partition the row belongs to

  This function returns with an error if both of the following are
  true:

    a) A table in the multiple-table update statement is updated
       through multiple aliases (including views)
    b) At least one of the updates on the table from a) may physically
       moves the row. Note: Updating a column used to calculate which
       partition a row belongs to does not necessarily mean that the
       row is moved. The new value may or may not belong to the same
       partition.

  @param leaves               First leaf table
  @param tables_for_update    Map of tables that are updated

  @return
    true   if the update is unsafe, in which case an error message is also set,
    false  otherwise.
*/
static bool unsafe_key_update(TABLE_LIST *leaves, table_map tables_for_update) {
  TABLE_LIST *tl = leaves;

  for (tl = leaves; tl; tl = tl->next_leaf) {
    if (tl->map() & tables_for_update) {
      TABLE *table1 = tl->table;
      bool primkey_clustered = (table1->file->primary_key_is_clustered() &&
                                table1->s->primary_key != MAX_KEY);

      bool table_partitioned = (table1->part_info != nullptr);

      if (!table_partitioned && !primkey_clustered) continue;

      for (TABLE_LIST *tl2 = tl->next_leaf; tl2; tl2 = tl2->next_leaf) {
        /*
          Look at "next" tables only since all previous tables have
          already been checked
        */
        TABLE *table2 = tl2->table;
        if (tl2->map() & tables_for_update && table1->s == table2->s) {
          // A table is updated through two aliases
          if (table_partitioned &&
              (partition_key_modified(table1, table1->write_set) ||
               partition_key_modified(table2, table2->write_set))) {
            // Partitioned key is updated
            my_error(
                ER_MULTI_UPDATE_KEY_CONFLICT, MYF(0),
                tl->belong_to_view ? tl->belong_to_view->alias : tl->alias,
                tl2->belong_to_view ? tl2->belong_to_view->alias : tl2->alias);
            return true;
          }

          if (primkey_clustered) {
            // The primary key can cover multiple columns
            KEY key_info = table1->key_info[table1->s->primary_key];
            KEY_PART_INFO *key_part = key_info.key_part;
            KEY_PART_INFO *key_part_end =
                key_part + key_info.user_defined_key_parts;

            for (; key_part != key_part_end; ++key_part) {
              if (bitmap_is_set(table1->write_set, key_part->fieldnr - 1) ||
                  bitmap_is_set(table2->write_set, key_part->fieldnr - 1)) {
                // Clustered primary key is updated
                my_error(
                    ER_MULTI_UPDATE_KEY_CONFLICT, MYF(0),
                    tl->belong_to_view ? tl->belong_to_view->alias : tl->alias,
                    tl2->belong_to_view ? tl2->belong_to_view->alias
                                        : tl2->alias);
                return true;
              }
            }
          }
        }
      }
    }
  }
  return false;
}

/// Check if a list of Items contains an Item whose type is JSON.
static bool has_json_columns(List<Item> *items) {
  List_iterator_fast<Item> it(*items);
  for (Item *item = it++; item != nullptr; item = it++)
    if (item->data_type() == MYSQL_TYPE_JSON) return true;
  return false;
}

/**
  Mark the columns that can possibly be updated in-place using partial update.

  Only JSON columns can be updated in-place, and only if all the updates of the
  column are on the form

      json_col = JSON_SET(json_col, ...)

      json_col = JSON_REPLACE(json_col, ...)

      json_col = JSON_REMOVE(json_col, ...)

  Even though a column is marked for partial update, it is not necessarily
  updated as a partial update during execution. It depends on the actual data
  in the column if it is possible to do it as a partial update. Also, for
  multi-table updates, it is only possible to perform partial updates in the
  first table of the join operation, and it is not determined until later (in
  Query_result_update::optimize()) which table it is.

  @param trace   the optimizer trace context
  @param fields  the fields that are updated by the update statement
  @param values  the values they are updated to
  @return false on success, true on error
*/
static bool prepare_partial_update(Opt_trace_context *trace, List<Item> *fields,
                                   List<Item> *values) {
  /*
    First check if we have any JSON columns. The only reason we do this, is to
    prevent writing an empty optimizer trace about partial update if there are
    no JSON columns.
  */
  if (!has_json_columns(fields)) return false;

  Opt_trace_object trace_partial_update(trace, "json_partial_update");
  Opt_trace_array trace_rejected(trace, "rejected_columns");

  using Field_array = Prealloced_array<const Field *, 8>;
  Field_array partial_update_fields(PSI_NOT_INSTRUMENTED);
  Field_array rejected_fields(PSI_NOT_INSTRUMENTED);
  List_iterator_fast<Item> field_it(*fields);
  List_iterator_fast<Item> value_it(*values);
  for (Item *field_item = field_it++, *value_item = value_it++;
       field_item != nullptr && value_item != nullptr;
       field_item = field_it++, value_item = value_it++) {
    // Only consider JSON fields for partial update for now.
    if (field_item->data_type() != MYSQL_TYPE_JSON) continue;

    const Field_json *field =
        down_cast<Field_json *>(down_cast<Item_field *>(field_item)->field);

    if (rejected_fields.count_unique(field) != 0) continue;

    /*
      Function object that adds the current column to the list of rejected
      columns, and possibly traces the rejection if optimizer tracing is
      enabled.
    */
    const auto reject_column = [&](const char *cause) {
      Opt_trace_object trace_obj(trace);
      trace_obj.add_utf8_table(field->table->pos_in_table_list);
      trace_obj.add_utf8("column", field->field_name);
      trace_obj.add_utf8("cause", cause);
      rejected_fields.insert_unique(field);
    };

    if ((field->table->file->ha_table_flags() & HA_BLOB_PARTIAL_UPDATE) == 0) {
      reject_column("Storage engine does not support partial update");
      continue;
    }

    if (!value_item->supports_partial_update(field)) {
      reject_column(
          "Updated using a function that does not support partial "
          "update, or source and target column differ");
      partial_update_fields.erase_unique(field);
      continue;
    }

    partial_update_fields.insert_unique(field);
  }

  if (partial_update_fields.empty()) return false;

  for (const Field *fld : partial_update_fields)
    if (fld->table->mark_column_for_partial_update(fld))
      return true; /* purecov: inspected */

  field_it.rewind();
  value_it.rewind();
  for (Item *field_item = field_it++, *value_item = value_it++;
       field_item != nullptr && value_item != nullptr;
       field_item = field_it++, value_item = value_it++) {
    const Field *field = down_cast<Item_field *>(field_item)->field;
    if (field->table->is_marked_for_partial_update(field)) {
      auto json_field = down_cast<const Field_json *>(field);
      auto json_func = down_cast<Item_json_func *>(value_item);
      json_func->mark_for_partial_update(json_field);
    }
  }

  return false;
}

bool Sql_cmd_update::prepare_inner(THD *thd) {
  DBUG_TRACE;

  Prepare_error_tracker tracker(thd);

  SELECT_LEX *const select = lex->select_lex;
  TABLE_LIST *const table_list = select->get_table_list();

  TABLE_LIST *single_table_updated = nullptr;

  List<Item> *update_fields = &select->item_list;
  table_map tables_for_update;
  const bool using_lock_tables = thd->locked_tables_mode != LTM_NONE;

  DBUG_ASSERT(update_fields->elements == update_value_list->elements);

  bool apply_semijoin;

  Mem_root_array<Item_exists_subselect *> sj_candidates_local(thd->mem_root);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_prepare(trace, "update_preparation");
  trace_prepare.add_select_number(select->select_number);

  if (multitable) {
    /*
      A view's CHECK OPTION is incompatible with semi-join.
      @note We could let non-updated views do semi-join, and we could let
            updated views without CHECK OPTION do semi-join.
            But since we resolve derived tables before we know this context,
            we cannot use semi-join in any case currently.
            The problem is that the CHECK OPTION condition serves as
            part of the semi-join condition, and a standalone condition
            to be evaluated as part of the UPDATE, and those two uses are
            incompatible.
    */
    apply_semijoin = false;
    select->set_sj_candidates(&sj_candidates_local);
  } else {
    apply_semijoin = false;
  }

  if (!select->top_join_list.empty())
    propagate_nullability(&select->top_join_list, false);

  if (select->setup_tables(thd, table_list, false))
    return true; /* purecov: inspected */

  thd->want_privilege = SELECT_ACL;
  enum enum_mark_columns mark_used_columns_saved = thd->mark_used_columns;
  thd->mark_used_columns = MARK_COLUMNS_READ;
  if (select->derived_table_count || select->table_func_count) {
    if (select->resolve_placeholder_tables(thd, apply_semijoin)) return true;
    /*
      @todo - This check is a bit primitive and ad-hoc. We have not yet analyzed
      the list of tables that are updated. Perhaps we should wait until that
      list is ready. In that case, we should check for UPDATE and SELECT
      privileges for tables that are updated and SELECT privileges for tables
      that are selected from. However, check_view_privileges() lacks
      functionality for detailed privilege checking.
    */
    if (select->check_view_privileges(thd, UPDATE_ACL, SELECT_ACL)) return true;
  }

  /*
    Updatability test is spread across several places:
    - Target table or view must be updatable (checked below)
    - A view has special requirements with respect to columns being updated
                                          (checked in check_key_in_view)
    - All updated columns must be from an updatable component of a view
                                          (checked in setup_fields)
    - Target table must not be same as one selected from
                                          (checked in unique_table)
  */

  if (!multitable) {
    // Single-table UPDATE, the table must be updatable:
    if (!table_list->is_updatable()) {
      my_error(ER_NON_UPDATABLE_TABLE, MYF(0), table_list->alias, "UPDATE");
      return true;
    }
    // Perform multi-table operation if table to be updated is multi-table view
    if (table_list->is_multiple_tables()) multitable = true;
  }

  if (select->leaf_table_count >= 2 &&
      setup_natural_join_row_types(thd, select->join_list, &select->context))
    return true;

  if (!multitable) {
    select->make_active_options(SELECT_NO_JOIN_CACHE, 0);

    // Identify the single table to be updated
    single_table_updated = table_list->updatable_base_table();
  } else {
    // At this point the update is known to be a multi-table operation.
    select->make_active_options(SELECT_NO_JOIN_CACHE | SELECT_NO_UNLOCK,
                                OPTION_BUFFER_RESULT);

    Prepared_stmt_arena_holder ps_holder(thd);
    result = new (thd->mem_root)
        Query_result_update(update_fields, update_value_list);
    if (result == nullptr) return true; /* purecov: inspected */

    // The former is for the pre-iterator executor; the latter is for the
    // iterator executor.
    // TODO(sgunders): Get rid of this when we remove Query_result.
    select->set_query_result(result);
    select->master_unit()->set_query_result(result);
  }

  lex->allow_sum_func = 0;  // Query block cannot be aggregated

  if (select->setup_conds(thd)) return true;

  if (select->setup_base_ref_items(thd)) return true; /* purecov: inspected */

  if (setup_fields(thd, Ref_item_array(), *update_fields, UPDATE_ACL, nullptr,
                   false, true))
    return true;

  if (check_fields(thd, *update_fields)) return true; /* purecov: inspected */

  /*
    Calculate map of tables that are updated based on resolved columns
    in the update field list.
  */
  thd->table_map_for_update = tables_for_update = get_table_map(update_fields);

  uint update_table_count_local = my_count_bits(tables_for_update);

  DBUG_ASSERT(update_table_count_local > 0);

  if (setup_fields(thd, Ref_item_array(), *update_value_list, SELECT_ACL,
                   nullptr, false, false))
    return true; /* purecov: inspected */

  thd->mark_used_columns = mark_used_columns_saved;

  if (select->master_unit()->prepare_limit(thd, select)) return true;

  if (prepare_partial_update(trace, update_fields, update_value_list))
    return true; /* purecov: inspected */

  if (!multitable) {
    // Add default values provided by a function, required for part. pruning
    // @todo consolidate with corresponding function in update_single_table()
    COPY_INFO update(COPY_INFO::UPDATE_OPERATION, update_fields,
                     update_value_list);
    TABLE *table = single_table_updated->table;
    if (update.add_function_default_columns(table, table->write_set))
      return true; /* purecov: inspected */

    if ((table->file->ha_table_flags() & HA_PARTIAL_COLUMN_READ) != 0 &&
        update.function_defaults_apply(table))
      /*
        A column is to be set to its ON UPDATE function default only if other
        columns of the row are changing. To know this, we must be able to
        compare the "before" and "after" value of those columns
        (i.e. records_are_comparable() must be true below). Thus, we must read
        those columns:
      */
      // @todo - consolidate with Query_result_update::prepare()
      bitmap_union(table->read_set, table->write_set);

    // UPDATE operations requires full row from base table, disable covering key
    // @todo - Consolidate this with multi-table ops
    table->covering_keys.clear_all();

    /*
      This must be done before partition pruning, since prune_partitions()
      uses table->write_set to determine if locks can be pruned.
    */
    if (table->triggers && table->triggers->mark_fields(TRG_EVENT_UPDATE))
      return true;
  }

  for (TABLE_LIST *tl = select->leaf_tables; tl; tl = tl->next_leaf) {
    tl->updating = tl->map() & tables_for_update;
    if (tl->updating) {
      // Cannot update a table if the storage engine does not support update.
      if (tl->table->file->ha_table_flags() & HA_UPDATE_NOT_SUPPORTED) {
        my_error(ER_ILLEGAL_HA, MYF(0), tl->table_name);
        return true;
      }

      if ((tl->table->vfield || tl->table->gen_def_fields_ptr != nullptr) &&
          validate_gc_assignment(update_fields, update_value_list, tl->table))
        return true; /* purecov: inspected */

      // Mark all containing view references as updating
      for (TABLE_LIST *ref = tl; ref != nullptr; ref = ref->referencing_view)
        ref->updating = true;

      // Check that table is unique, updatability has already been checked.
      if (select->first_execution && check_key_in_view(thd, tl, tl)) {
        my_error(ER_NON_UPDATABLE_TABLE, MYF(0), tl->top_table()->alias,
                 "UPDATE");
        return true;
      }

      DBUG_PRINT("info", ("setting table `%s` for update", tl->alias));
    } else {
      DBUG_PRINT("info", ("setting table `%s` for read-only", tl->alias));
      /*
        If we are using the binary log, we need TL_READ_NO_INSERT to get
        correct order of statements. Otherwise, we use a TL_READ lock to
        improve performance.
        We don't downgrade metadata lock from SW to SR in this case as
        there is no guarantee that the same ticket is not used by
        another table instance used by this statement which is going to
        be write-locked (for example, trigger to be invoked might try
        to update this table).
        Last argument routine_modifies_data for read_lock_type_for_table()
        is ignored, as prelocking placeholder will never be set here.
      */
      DBUG_ASSERT(tl->prelocking_placeholder == false);
      tl->set_lock({read_lock_type_for_table(thd, lex, tl, true), THR_DEFAULT});
      /* Update TABLE::lock_type accordingly. */
      if (!tl->is_placeholder() && !using_lock_tables)
        tl->table->reginfo.lock_type = tl->lock_descriptor().type;
    }
  }

  if (update_table_count_local > 1 &&
      unsafe_key_update(select->leaf_tables, tables_for_update))
    return true;

  /*
    Check that tables being updated are not used in a subquery, but
    skip all tables of the UPDATE query block itself
  */
  select->exclude_from_table_unique_test = true;

  for (TABLE_LIST *tr = select->leaf_tables; tr; tr = tr->next_leaf) {
    if (tr->updating) {
      TABLE_LIST *duplicate = unique_table(tr, select->leaf_tables, false);
      if (duplicate != nullptr) {
        update_non_unique_table_error(select->leaf_tables, "UPDATE", duplicate);
        return true;
      }
    }
  }

  /*
    Set exclude_from_table_unique_test value back to false. It is needed for
    further check whether to use record cache.
  */
  select->exclude_from_table_unique_test = false;

  /* check single table update for view compound from several tables */
  for (TABLE_LIST *tl = table_list; tl; tl = tl->next_local) {
    if (tl->is_merged()) {
      DBUG_ASSERT(tl->is_view_or_derived());
      TABLE_LIST *for_update = nullptr;
      if (tl->check_single_table(&for_update, tables_for_update)) {
        my_error(ER_VIEW_MULTIUPDATE, MYF(0), tl->view_db.str,
                 tl->view_name.str);
        return true;
      }
    }
  }

  /* @todo: downgrade the metadata locks here. */

  /*
    Syntax rule for multi-table update prevents these constructs.
    But they are possible for single-table UPDATE against multi-table view.
  */
  if (multitable && select->order_list.elements) {
    my_error(ER_WRONG_USAGE, MYF(0), "UPDATE", "ORDER BY");
    return true;
  }
  if (multitable && select->select_limit) {
    my_error(ER_WRONG_USAGE, MYF(0), "UPDATE", "LIMIT");
    return true;
  }
  if (select->order_list.first) {
    List<Item> all_fields;  // @todo check this
    if (setup_order(thd, select->base_ref_items, table_list, all_fields,
                    all_fields, select->order_list.first))
      return true;
  }

  DBUG_ASSERT(select->having_cond() == nullptr &&
              select->group_list.elements == 0);

  if (select->has_ft_funcs() && setup_ftfuncs(thd, select))
    return true; /* purecov: inspected */

  if (select->query_result() &&
      select->query_result()->prepare(thd, select->fields_list, lex->unit))
    return true; /* purecov: inspected */

  Opt_trace_array trace_steps(trace, "steps");
  opt_trace_print_expanded_query(thd, select, &trace_wrapper);

  if (select->has_sj_candidates() && select->flatten_subqueries(thd))
    return true; /* purecov: inspected */

  select->set_sj_candidates(nullptr);

  if (select->apply_local_transforms(thd, true))
    return true; /* purecov: inspected */

  if (!multitable && select->is_empty_query()) set_empty_query();

  return false;
}

bool Sql_cmd_update::execute_inner(THD *thd) {
  return multitable ? Sql_cmd_dml::execute_inner(thd)
                    : update_single_table(thd);
}

/*
  Connect fields with tables and create list of tables that are updated
*/

bool Query_result_update::prepare(THD *thd, List<Item> &, SELECT_LEX_UNIT *u) {
  SQL_I_List<TABLE_LIST> update_list;
  List_iterator_fast<Item> field_it(*fields);
  List_iterator_fast<Item> value_it(*values);
  DBUG_TRACE;

  unit = u;

  SELECT_LEX *const select = unit->first_select();
  TABLE_LIST *const leaves = select->leaf_tables;

  thd->check_for_truncated_fields = CHECK_FIELD_WARN;
  thd->num_truncated_fields = 0L;
  THD_STAGE_INFO(thd, stage_updating_main_table);

  const table_map tables_to_update = get_table_map(fields);

  /*
    We gather the set of columns read during evaluation of SET expression in
    TABLE::tmp_set by pointing TABLE::read_set to it and then restore it after
    setup_fields().
  */
  for (TABLE_LIST *tr = leaves; tr; tr = tr->next_leaf) {
    DBUG_ASSERT(tr->updating == ((tables_to_update & tr->map()) != 0));
    if (tables_to_update & tr->map()) {
      TABLE *const table = tr->table;
      DBUG_ASSERT(table->read_set == &table->def_read_set);
      table->read_set = &table->tmp_set;
      bitmap_clear_all(table->read_set);
    }
    // Resolving may be needed for subsequent executions
    if (tr->check_option && !tr->check_option->fixed &&
        tr->check_option->fix_fields(thd, nullptr))
      return true; /* purecov: inspected */
  }

  for (TABLE_LIST *tr = leaves; tr; tr = tr->next_leaf) {
    if (tables_to_update & tr->map()) {
      TABLE *const table = tr->table;
      table->read_set = &table->def_read_set;
      bitmap_union(table->read_set, &table->tmp_set);
      bitmap_clear_all(&table->tmp_set);
    }
  }

  /*
    Save tables beeing updated in update_tables
    update_table->shared is position for table
    Don't use key read on tables that are updated
  */

  update_list.empty();
  for (TABLE_LIST *tr = leaves; tr; tr = tr->next_leaf) {
    /* TODO: add support of view of join support */
    if (tables_to_update & tr->map()) {
      auto dup = new (thd->mem_root) TABLE_LIST(*tr);
      if (dup == nullptr) return true;

      TABLE *const table = tr->table;

      update_list.link_in_list(dup, &dup->next_local);
      tr->shared = dup->shared = update_table_count++;
      table->no_keyread = true;
      table->covering_keys.clear_all();
      table->set_pos_in_table_list(dup);
      table->prepare_triggers_for_update_stmt_or_event();
    }
  }

  update_table_count = update_list.elements;
  update_tables = update_list.first;

  tmp_tables = (TABLE **)thd->mem_calloc(sizeof(TABLE *) * update_table_count);
  if (tmp_tables == nullptr) return true;
  tmp_table_param = new (thd->mem_root) Temp_table_param[update_table_count];
  if (tmp_table_param == nullptr) return true;
  fields_for_table =
      (List_item **)thd->alloc(sizeof(List_item *) * update_table_count);
  if (fields_for_table == nullptr) return true;
  values_for_table =
      (List_item **)thd->alloc(sizeof(List_item *) * update_table_count);
  if (values_for_table == nullptr) return true;

  DBUG_ASSERT(update_operations == nullptr);
  update_operations =
      (COPY_INFO **)thd->mem_calloc(sizeof(COPY_INFO *) * update_table_count);

  if (update_operations == nullptr) return true;
  for (uint i = 0; i < update_table_count; i++) {
    fields_for_table[i] = new (thd->mem_root) List_item;
    values_for_table[i] = new (thd->mem_root) List_item;
  }
  if (thd->is_error()) return true;

  /* Split fields into fields_for_table[] and values_by_table[] */

  Item *item;
  while ((item = field_it++)) {
    Item_field *const field = down_cast<Item_field *>(item);
    Item *const value = value_it++;
    uint offset = field->table_ref->shared;
    fields_for_table[offset]->push_back(field);
    values_for_table[offset]->push_back(value);
  }
  if (thd->is_error()) return true;

  /* Allocate copy fields */
  max_fields = 0;
  for (uint i = 0; i < update_table_count; i++)
    max_fields = std::max(max_fields, size_t(fields_for_table[i]->elements +
                                             select->leaf_table_count));
  copy_field = new (thd->mem_root) Copy_field[max_fields];

  for (TABLE_LIST *ref = leaves; ref != nullptr; ref = ref->next_leaf) {
    if (tables_to_update & ref->map()) {
      const uint position = ref->shared;
      List<Item> *cols = fields_for_table[position];
      List<Item> *vals = values_for_table[position];
      TABLE *const table = ref->table;

      COPY_INFO *update = new (thd->mem_root)
          COPY_INFO(COPY_INFO::UPDATE_OPERATION, cols, vals);
      if (update == nullptr ||
          update->add_function_default_columns(table, table->write_set))
        return true;

      update_operations[position] = update;

      if ((table->file->ha_table_flags() & HA_PARTIAL_COLUMN_READ) != 0 &&
          update->function_defaults_apply(table)) {
        /*
          A column is to be set to its ON UPDATE function default only if
          other columns of the row are changing. To know this, we must be able
          to compare the "before" and "after" value of those columns. Thus, we
          must read those columns:
        */
        bitmap_union(table->read_set, table->write_set);
      }
      /* All needed columns must be marked before prune_partitions(). */
      if (table->triggers && table->triggers->mark_fields(TRG_EVENT_UPDATE))
        return true;
    }
  }

  DBUG_ASSERT(!thd->is_error());
  return false;
}

/*
  Check if table is safe to update on fly

  SYNOPSIS
    safe_update_on_fly()
    join_tab            How table is used in join
    all_tables          List of tables

  NOTES
    We can update the first table in join on the fly if we know that
    a row in this table will never be read twice. This is true under
    the following conditions:

    - No column is both written to and read in SET expressions.

    - We are doing a table scan and the data is in a separate file (MyISAM) or
      if we don't update a clustered key.

    - We are doing a range scan and we don't update the scan key or
      the primary key for a clustered table handler.

    - Table is not joined to itself.

    This function gets information about fields to be updated from
    the TABLE::write_set bitmap.

  WARNING
    This code is a bit dependent of how make_join_readinfo() works.

    The field table->tmp_set is used for keeping track of which fields are
    read during evaluation of the SET expression.
    See Query_result_update::prepare.

  RETURN
    0		Not safe to update
    1		Safe to update
*/

static bool safe_update_on_fly(JOIN_TAB *join_tab, TABLE_LIST *table_ref,
                               TABLE_LIST *all_tables) {
  TABLE *table = join_tab->table();
  if (unique_table(table_ref, all_tables, false)) return false;
  switch (join_tab->type()) {
    case JT_SYSTEM:
    case JT_CONST:
    case JT_EQ_REF:
      return true;  // At most one matching row
    case JT_REF:
    case JT_REF_OR_NULL:
      return !is_key_used(table, join_tab->ref().key, table->write_set);
    case JT_ALL:
      if (bitmap_is_overlapping(&table->tmp_set, table->write_set))
        return false;
      /* If range search on index */
      if (join_tab->quick())
        return !join_tab->quick()->is_keys_used(table->write_set);
      /* If scanning in clustered key */
      if ((table->file->ha_table_flags() & HA_PRIMARY_KEY_IN_READ_INDEX) &&
          table->s->primary_key < MAX_KEY)
        return !is_key_used(table, table->s->primary_key, table->write_set);
      return true;
    default:
      break;  // Avoid compler warning
  }
  return false;
}

/**
  Set up data structures for multi-table UPDATE

  IMPLEMENTATION
    - Update first table in join on the fly, if possible
    - Create temporary tables to store changed values for all other tables
      that are updated (and main_table if the above doesn't hold).
*/

bool Query_result_update::optimize() {
  TABLE_LIST *table_ref;
  DBUG_TRACE;

  SELECT_LEX *const select = unit->first_select();
  JOIN *const join = select->join;
  THD *thd = join->thd;

  ASSERT_BEST_REF_IN_JOIN_ORDER(join);

  TABLE_LIST *leaves = select->leaf_tables;

  if ((thd->variables.option_bits & OPTION_SAFE_UPDATES) &&
      error_if_full_join(join))
    return true;
  main_table = join->best_ref[0]->table();
  table_to_update = nullptr;

  /* Any update has at least one pair (field, value) */
  DBUG_ASSERT(fields->elements);
  /*
   Only one table may be modified by UPDATE of an updatable view.
   For an updatable view first_table_for_update indicates this
   table.
   For a regular multi-update it refers to some updated table.
  */
  TABLE_LIST *first_table_for_update =
      ((Item_field *)fields->head())->table_ref;

  /* Create a temporary table for keys to all tables, except main table */
  for (table_ref = update_tables; table_ref;
       table_ref = table_ref->next_local) {
    TABLE *table = table_ref->table;
    uint cnt = table_ref->shared;
    List<Item> temp_fields;
    ORDER group;
    Temp_table_param *tmp_param;

    if (thd->lex->is_ignore()) table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
    if (table == main_table)  // First table in join
    {
      /*
        If there are at least two tables to update, t1 and t2, t1 being
        before t2 in the plan, we need to collect all fields of t1 which
        influence the selection of rows from t2. If those fields are also
        updated, it will not be possible to update t1 on-the-fly.
        Due to how the nested loop join algorithm works, when collecting
        we can ignore the condition attached to t1 - a row of t1 is read
        only one time.
      */
      if (update_tables->next_local) {
        for (uint i = 1; i < join->tables; ++i) {
          JOIN_TAB *tab = join->best_ref[i];
          if (tab->condition())
            tab->condition()->walk(&Item::add_field_to_set_processor,
                                   enum_walk::SUBQUERY_POSTFIX,
                                   reinterpret_cast<uchar *>(main_table));
          /*
            On top of checking conditions, we need to check conditions
            referenced by index lookup on the following tables. They implement
            conditions too, but their corresponding search conditions might
            have been optimized away. The second table is an exception: even if
            rows are read from it using index lookup which references a column
            of main_table, the implementation of ref access will see the
            before-update value;
            consider this flow of a nested loop join:
            read a row from main_table and:
            - init ref access (construct_lookup_ref() in RefIterator):
              copy referenced value from main_table into 2nd table's ref buffer
            - look up a first row in 2nd table (RefIterator::Read())
              - if it joins, update row of main_table on the fly
            - look up a second row in 2nd table (again RefIterator::Read()).
            Because construct_lookup_ref() is not called again, the
            before-update value of the row of main_table is still in the 2nd
            table's ref buffer. So the lookup is not influenced by the just-done
            update of main_table.
          */
          if (tab > join->join_tab + 1) {
            for (uint key_part_idx = 0; key_part_idx < tab->ref().key_parts;
                 key_part_idx++) {
              Item *ref_item = tab->ref().items[key_part_idx];
              if ((table_ref->map() & ref_item->used_tables()) != 0)
                ref_item->walk(&Item::add_field_to_set_processor,
                               enum_walk::SUBQUERY_POSTFIX,
                               reinterpret_cast<uchar *>(main_table));
            }
          }
        }
      }
      if (safe_update_on_fly(join->best_ref[0], table_ref,
                             select->get_table_list())) {
        table->mark_columns_needed_for_update(
            thd, true /*mark_binlog_columns=true*/);
        if (table->setup_partial_update()) return true; /* purecov: inspected */
        table_to_update = table;  // Update table on the fly
        continue;
      }
    }
    table->mark_columns_needed_for_update(thd,
                                          true /*mark_binlog_columns=true*/);

    if (table != table_to_update &&
        table->has_columns_marked_for_partial_update()) {
      Opt_trace_context *trace = &thd->opt_trace;
      if (trace->is_started()) {
        Opt_trace_object trace_wrapper(trace);
        Opt_trace_object trace_partial_update(trace, "json_partial_update");
        Opt_trace_object trace_rejected(trace, "rejected_table");
        trace_rejected.add_utf8_table(table->pos_in_table_list);
        trace_rejected.add_utf8("cause", "Table cannot be updated on the fly");
      }
    }

    /*
      enable uncacheable flag if we update a view with check option
      and check option has a subselect, otherwise, the check option
      can be evaluated after the subselect was freed as independent
      (See full_local in JOIN::join_free()).
    */
    if (table_ref->check_option && !select->uncacheable) {
      SELECT_LEX_UNIT *tmp_unit;
      SELECT_LEX *sl;
      for (tmp_unit = select->first_inner_unit(); tmp_unit;
           tmp_unit = tmp_unit->next_unit()) {
        for (sl = tmp_unit->first_select(); sl; sl = sl->next_select()) {
          if (sl->master_unit()->item) {
            // Prevent early freeing in JOIN::join_free()
            select->uncacheable |= UNCACHEABLE_CHECKOPTION;
            goto loop_end;
          }
        }
      }
    }
  loop_end:

    if (table_ref->table == first_table_for_update->table &&
        table_ref->check_option) {
      table_map unupdated_tables = table_ref->check_option->used_tables() &
                                   ~first_table_for_update->map();
      for (TABLE_LIST *tbl_ref = leaves; unupdated_tables && tbl_ref;
           tbl_ref = tbl_ref->next_leaf) {
        if (unupdated_tables & tbl_ref->map())
          unupdated_tables &= ~tbl_ref->map();
        else
          continue;
        if (unupdated_check_opt_tables.push_back(tbl_ref->table))
          return true; /* purecov: inspected */
      }
    }

    tmp_param = tmp_table_param + cnt;

    /*
      Create a temporary table to store all fields that are changed for this
      table. The first field in the temporary table is a pointer to the
      original row so that we can find and update it. For the updatable
      VIEW a few following fields are rowids of tables used in the CHECK
      OPTION condition.
    */

    List_iterator_fast<TABLE> tbl_it(unupdated_check_opt_tables);
    TABLE *tbl = table;
    do {
      /*
        Signal each table (including tables referenced by WITH CHECK OPTION
        clause) for which we will store row position in the temporary table
        that we need a position to be read first.
      */
      tbl->prepare_for_position();

      Field_string *field = new (thd->mem_root) Field_string(
          tbl->file->ref_length, false, tbl->alias, &my_charset_bin);
      if (!field) return true;
      field->init(tbl);
      Item_field *ifield = new (thd->mem_root) Item_field(field);
      if (!ifield) return true;
      ifield->maybe_null = false;
      if (temp_fields.push_back(ifield)) return true;
    } while ((tbl = tbl_it++));

    temp_fields.concat(fields_for_table[cnt]);

    /* Make an unique key over the first field to avoid duplicated updates */
    memset(&group, 0, sizeof(group));
    group.direction = ORDER_ASC;
    group.item = temp_fields.head_ref();

    tmp_param->allow_group_via_temp_table = true;
    tmp_param->field_count = temp_fields.elements;
    tmp_param->group_parts = 1;
    tmp_param->group_length = table->file->ref_length;
    tmp_tables[cnt] =
        create_tmp_table(thd, tmp_param, temp_fields, &group, false, false,
                         TMP_TABLE_ALL_COLUMNS, HA_POS_ERROR, "");
    if (!tmp_tables[cnt]) return true;

    /*
      Pass a table triggers pointer (Table_trigger_dispatcher *) from
      the original table to the new temporary table. This pointer will be used
      inside the method Query_result_update::send_data() to determine temporary
      nullability flag for the temporary table's fields. It will be done before
      calling fill_record() to assign values to the temporary table's fields.
    */
    tmp_tables[cnt]->triggers = table->triggers;
  }
  return false;
}

void Query_result_update::cleanup(THD *thd) {
  TABLE_LIST *table;
  for (table = update_tables; table; table = table->next_local) {
    table->table->no_cache = false;
  }

  if (tmp_tables) {
    for (uint cnt = 0; cnt < update_table_count; cnt++) {
      if (tmp_tables[cnt]) {
        free_tmp_table(thd, tmp_tables[cnt]);
        tmp_table_param[cnt].cleanup();
      }
    }
  }
  destroy_array(copy_field, max_fields);
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;  // Restore this setting
  DBUG_ASSERT(
      trans_safe || updated_rows == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));

  if (update_operations != nullptr)
    for (uint i = 0; i < update_table_count; i++) destroy(update_operations[i]);
}

bool Query_result_update::send_data(THD *thd, List<Item> &) {
  TABLE_LIST *cur_table;
  DBUG_TRACE;

  for (cur_table = update_tables; cur_table;
       cur_table = cur_table->next_local) {
    TABLE *table = cur_table->table;
    uint offset = cur_table->shared;
    /*
      Check if we are using outer join and we didn't find the row
      or if we have already updated this row in the previous call to this
      function.

      The same row may be presented here several times in a join of type
      UPDATE t1 FROM t1,t2 SET t1.a=t2.a

      In this case we will do the update for the first found row combination.
      The join algorithm guarantees that we will not find the a row in
      t1 several times.
    */
    if (table->has_null_row() || table->has_updated_row()) continue;

    if (table == table_to_update) {
      table->clear_partial_update_diffs();
      table->set_updated_row();
      store_record(table, record[1]);
      bool is_row_changed = false;
      if (fill_record_n_invoke_before_triggers(
              thd, update_operations[offset], *fields_for_table[offset],
              *values_for_table[offset], table, TRG_EVENT_UPDATE, 0, false,
              &is_row_changed))
        return true;

      found_rows++;
      int error = 0;
      if (is_row_changed) {
        if ((error = cur_table->view_check_option(thd)) != VIEW_CHECK_OK) {
          found_rows--;
          if (error == VIEW_CHECK_SKIP)
            continue;
          else if (error == VIEW_CHECK_ERROR)
            return true;
        }

        /*
          Existing rows in table should normally satisfy CHECK constraints. So
          it should be safe to check constraints only for rows that has really
          changed (i.e. after compare_records()).

          In future, once addition/enabling of CHECK constraints without their
          validation is supported, we might encounter old rows which do not
          satisfy CHECK constraints currently enabled. However, rejecting
          no-op updates to such invalid pre-existing rows won't make them
          valid and is probably going to be confusing for users. So it makes
          sense to stick to current behavior.
        */
        if (invoke_table_check_constraints(thd, table)) {
          if (thd->is_error()) return true;
          // continue when IGNORE clause is used.
          continue;
        }

        if (!updated_rows++) {
          /*
            Inform the main table that we are going to update the table even
            while we may be scanning it.  This will flush the read cache
            if it's used.
          */
          main_table->file->ha_extra(HA_EXTRA_PREPARE_FOR_UPDATE);
        }
        if ((error = table->file->ha_update_row(table->record[1],
                                                table->record[0])) &&
            error != HA_ERR_RECORD_IS_THE_SAME) {
          updated_rows--;
          myf error_flags = MYF(0);
          if (table->file->is_fatal_error(error)) error_flags |= ME_FATALERROR;

          table->file->print_error(error, error_flags);

          /* Errors could be downgraded to warning by IGNORE */
          if (thd->is_error()) return true;
        } else {
          if (error == HA_ERR_RECORD_IS_THE_SAME) {
            error = 0;
            updated_rows--;
          }
          /* non-transactional or transactional table got modified   */
          /* either Query_result_update class' flag is raised in its branch */
          if (table->file->has_transactions())
            transactional_tables = true;
          else {
            trans_safe = false;
            thd->get_transaction()->mark_modified_non_trans_table(
                Transaction_ctx::STMT);
          }
        }
      }
      if (!error && table->triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_UPDATE,
                                            TRG_ACTION_AFTER, true))
        return true;
    } else {
      int error;
      TABLE *tmp_table = tmp_tables[offset];
      /*
       For updatable VIEW store rowid of the updated table and
       rowids of tables used in the CHECK OPTION condition.
      */
      uint field_num = 0;
      List_iterator_fast<TABLE> tbl_it(unupdated_check_opt_tables);
      TABLE *tbl = table;
      do {
        tbl->file->position(tbl->record[0]);
        memcpy((char *)tmp_table->visible_field_ptr()[field_num]->ptr,
               (char *)tbl->file->ref, tbl->file->ref_length);
        /*
         For outer joins a rowid field may have no NOT_NULL_FLAG,
         so we have to reset NULL bit for this field.
         (set_notnull() resets NULL bit only if available).
        */
        tmp_table->visible_field_ptr()[field_num]->set_notnull();
        field_num++;
      } while ((tbl = tbl_it++));

      /*
        If there are triggers in an original table the temporary table based on
        then enable temporary nullability for temporary table's fields.
      */
      if (tmp_table->triggers) {
        for (Field **modified_fields = tmp_table->visible_field_ptr() + 1 +
                                       unupdated_check_opt_tables.elements;
             *modified_fields; ++modified_fields) {
          (*modified_fields)->set_tmp_nullable();
        }
      }

      /* Store regular updated fields in the row. */
      fill_record(thd, tmp_table,
                  tmp_table->visible_field_ptr() + 1 +
                      unupdated_check_opt_tables.elements,
                  *values_for_table[offset], nullptr, nullptr, false);

      /* Write row, ignoring duplicated updates to a row */
      error = tmp_table->file->ha_write_row(tmp_table->record[0]);
      if (error != HA_ERR_FOUND_DUPP_KEY && error != HA_ERR_FOUND_DUPP_UNIQUE) {
        if (error &&
            create_ondisk_from_heap(thd, tmp_table, error, true, nullptr)) {
          update_completed = true;
          return true;  // Not a table_is_full error
        }
        found_rows++;
      }
    }
  }
  return false;
}

void Query_result_update::send_error(THD *, uint errcode, const char *err) {
  /* First send error what ever it is ... */
  my_error(errcode, MYF(0), err);
}

void Query_result_update::abort_result_set(THD *thd) {
  /* the error was handled or nothing deleted and no side effects return */
  if (error_handled ||
      (!thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT) &&
       updated_rows == 0))
    return;

  /*
    If all tables that has been updated are trans safe then just do rollback.
    If not attempt to do remaining updates.
  */

  if (!trans_safe) {
    DBUG_ASSERT(
        thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
    if (!update_completed && update_table_count > 1) {
      /* @todo: Add warning here */
      (void)do_updates(thd);
    }
  }
  if (thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    /*
      The query has to binlog because there's a modified non-transactional table
      either from the query's list or via a stored routine: bug#13270,23333
    */
    if (mysql_bin_log.is_open()) {
      /*
        THD::killed status might not have been set ON at time of an error
        got caught and if happens later the killed error is written
        into repl event.
      */
      int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
      /* the error of binary logging is ignored */
      (void)thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                              thd->query().length, transactional_tables, false,
                              false, errcode);
    }
  }
  DBUG_ASSERT(
      trans_safe || updated_rows == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
}

bool Query_result_update::do_updates(THD *thd) {
  TABLE_LIST *cur_table;
  int local_error = 0;
  ha_rows org_updated;
  TABLE *table, *tmp_table;
  List_iterator_fast<TABLE> check_opt_it(unupdated_check_opt_tables);
  myf error_flags = MYF(0); /**< Flag for fatal errors */

  DBUG_TRACE;

  update_completed = true;  // Don't retry this function

  if (found_rows == 0) {
    /*
      If the binary log is on, we still need to check
      if there are transactional tables involved. If
      there are mark the transactional_tables flag correctly.

      This flag determines whether the writes go into the
      transactional or non transactional cache, even if they
      do not change any table, they are still written into
      the binary log when the format is STMT or MIXED.
    */
    if (mysql_bin_log.is_open()) {
      for (cur_table = update_tables; cur_table;
           cur_table = cur_table->next_local) {
        table = cur_table->table;
        transactional_tables |= table->file->has_transactions();
      }
    }
    return false;
  }

  // If we're updating based on an outer join, the executor may have left some
  // rows in NULL row state. Reset them before we start looking at rows,
  // so that generated fields don't inadvertedly get NULL inputs.
  for (cur_table = update_tables; cur_table;
       cur_table = cur_table->next_local) {
    cur_table->table->reset_null_row();
  }

  for (cur_table = update_tables; cur_table;
       cur_table = cur_table->next_local) {
    uint offset = cur_table->shared;

    table = cur_table->table;

    /*
      Always update the flag if - even if not updating the table,
      when the binary log is ON. This will allow the right binlog
      cache - stmt or trx cache - to be selected when logging
      innefective statementst to the binary log (in STMT or MIXED
      mode logging).
     */
    if (mysql_bin_log.is_open())
      transactional_tables |= table->file->has_transactions();

    if (table == table_to_update) continue;  // Already updated
    org_updated = updated_rows;
    tmp_table = tmp_tables[cur_table->shared];
    if ((local_error = table->file->ha_rnd_init(false))) {
      if (table->file->is_fatal_error(local_error))
        error_flags |= ME_FATALERROR;

      table->file->print_error(local_error, error_flags);
      goto err;
    }

    check_opt_it.rewind();
    while (TABLE *tbl = check_opt_it++) {
      if (tbl->file->ha_rnd_init(true))
        // No known handler error code present, print_error makes no sense
        goto err;
    }

    /*
      Setup copy functions to copy fields from temporary table
    */
    List_iterator_fast<Item> field_it(*fields_for_table[offset]);
    Field **field = tmp_table->visible_field_ptr() + 1 +
                    unupdated_check_opt_tables.elements;  // Skip row pointers
    Copy_field *copy_field_ptr = copy_field, *copy_field_end;
    for (; *field; field++) {
      Item_field *item = (Item_field *)field_it++;
      (copy_field_ptr++)->set(item->field, *field, false);
    }
    copy_field_end = copy_field_ptr;

    if ((local_error = tmp_table->file->ha_rnd_init(true))) {
      if (table->file->is_fatal_error(local_error))
        error_flags |= ME_FATALERROR;

      table->file->print_error(local_error, error_flags);
      goto err;
    }

    for (;;) {
      if (thd->killed && trans_safe)
        // No known handler error code present, print_error makes no sense
        goto err;
      if ((local_error = tmp_table->file->ha_rnd_next(tmp_table->record[0]))) {
        if (local_error == HA_ERR_END_OF_FILE) break;
        if (local_error == HA_ERR_RECORD_DELETED)
          continue;  // May happen on dup key
        if (table->file->is_fatal_error(local_error))
          error_flags |= ME_FATALERROR;

        table->file->print_error(local_error, error_flags);
        goto err;
      }

      /* call ha_rnd_pos() using rowids from temporary table */
      check_opt_it.rewind();
      TABLE *tbl = table;
      uint field_num = 0;
      do {
        if ((local_error = tbl->file->ha_rnd_pos(
                 tbl->record[0],
                 (uchar *)tmp_table->visible_field_ptr()[field_num]->ptr))) {
          if (table->file->is_fatal_error(local_error))
            error_flags |= ME_FATALERROR;

          table->file->print_error(local_error, error_flags);
          goto err;
        }
        field_num++;
      } while ((tbl = check_opt_it++));

      table->set_updated_row();
      store_record(table, record[1]);

      /* Copy data from temporary table to current table */
      for (copy_field_ptr = copy_field; copy_field_ptr != copy_field_end;
           copy_field_ptr++)
        copy_field_ptr->invoke_do_copy();

      if (thd->is_error()) goto err;

      // The above didn't update generated columns
      if (table->vfield &&
          update_generated_write_fields(table->write_set, table))
        goto err;

      if (table->triggers) {
        bool rc = table->triggers->process_triggers(thd, TRG_EVENT_UPDATE,
                                                    TRG_ACTION_BEFORE, true);

        // Trigger might have changed dependencies of generated columns
        if (!rc && table->vfield &&
            update_generated_write_fields(table->write_set, table))
          goto err;

        table->triggers->disable_fields_temporary_nullability();

        if (rc || check_record(thd, table->field)) goto err;
      }

      if (!records_are_comparable(table) || compare_records(table)) {
        /*
          This function does not call the fill_record_n_invoke_before_triggers
          which sets function defaults automagically. Hence calling
          set_function_defaults here explicitly to set the function defaults.
        */
        update_operations[offset]->set_function_defaults(table);

        /*
          It is safe to not invoke CHECK OPTION for VIEW if records are same.
          In this case the row is coming from the view and thus should satisfy
          the CHECK OPTION.
        */
        int error;
        if ((error = cur_table->view_check_option(thd)) != VIEW_CHECK_OK) {
          if (error == VIEW_CHECK_SKIP)
            continue;
          else if (error == VIEW_CHECK_ERROR)
            // No known handler error code present, print_error makes no sense
            goto err;
        }

        /*
          Existing rows in table should normally satisfy CHECK constraints. So
          it should be safe to check constraints only for rows that has really
          changed (i.e. after compare_records()).

          In future, once addition/enabling of CHECK constraints without their
          validation is supported, we might encounter old rows which do not
          satisfy CHECK constraints currently enabled. However, rejecting no-op
          updates to such invalid pre-existing rows won't make them valid and is
          probably going to be confusing for users. So it makes sense to stick
          to current behavior.
        */
        if (invoke_table_check_constraints(thd, table)) {
          if (thd->is_error()) goto err;
          // continue when IGNORE clause is used.
          continue;
        }

        local_error =
            table->file->ha_update_row(table->record[1], table->record[0]);
        if (!local_error)
          updated_rows++;
        else if (local_error == HA_ERR_RECORD_IS_THE_SAME)
          local_error = 0;
        else {
          if (table->file->is_fatal_error(local_error))
            error_flags |= ME_FATALERROR;

          table->file->print_error(local_error, error_flags);
          /* Errors could be downgraded to warning by IGNORE */
          if (thd->is_error()) goto err;
        }
      }

      if (!local_error && table->triggers &&
          table->triggers->process_triggers(thd, TRG_EVENT_UPDATE,
                                            TRG_ACTION_AFTER, true))
        goto err;
    }

    if (updated_rows != org_updated) {
      if (!table->file->has_transactions()) {
        trans_safe = false;  // Can't do safe rollback
        thd->get_transaction()->mark_modified_non_trans_table(
            Transaction_ctx::STMT);
      }
    }
    (void)table->file->ha_rnd_end();
    (void)tmp_table->file->ha_rnd_end();
    check_opt_it.rewind();
    while (TABLE *tbl = check_opt_it++) tbl->file->ha_rnd_end();
  }
  return false;

err:
  if (table->file->inited) (void)table->file->ha_rnd_end();
  if (tmp_table->file->inited) (void)tmp_table->file->ha_rnd_end();
  check_opt_it.rewind();
  while (TABLE *tbl = check_opt_it++) {
    if (tbl->file->inited) (void)tbl->file->ha_rnd_end();
  }

  if (updated_rows != org_updated) {
    if (table->file->has_transactions())
      transactional_tables = true;
    else {
      trans_safe = false;
      thd->get_transaction()->mark_modified_non_trans_table(
          Transaction_ctx::STMT);
    }
  }
  return true;
}

bool Query_result_update::send_eof(THD *thd) {
  char buff[STRING_BUFFER_USUAL_SIZE];
  ulonglong id;
  THD::killed_state killed_status = THD::NOT_KILLED;
  DBUG_TRACE;
  THD_STAGE_INFO(thd, stage_updating_reference_tables);

  /*
     Does updates for the last n - 1 tables, returns 0 if ok;
     error takes into account killed status gained in do_updates()
  */
  int local_error = thd->is_error();
  if (!local_error) local_error = (update_table_count) ? do_updates(thd) : 0;
  /*
    if local_error is not set ON until after do_updates() then
    later carried out killing should not affect binlogging.
  */
  killed_status = (local_error == 0) ? THD::NOT_KILLED : thd->killed.load();

  /*
    Write the SQL statement to the binlog if we updated
    rows and we succeeded or if we updated some non
    transactional tables.

    The query has to binlog because there's a modified non-transactional table
    either from the query's list or via a stored routine: bug#13270,23333
  */

  if (local_error == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT)) {
    if (mysql_bin_log.is_open()) {
      int errcode = 0;
      if (local_error == 0)
        thd->clear_error();
      else
        errcode = query_error_code(thd, killed_status == THD::NOT_KILLED);
      if (thd->binlog_query(THD::ROW_QUERY_TYPE, thd->query().str,
                            thd->query().length, transactional_tables, false,
                            false, errcode)) {
        local_error = 1;  // Rollback update
      }
    }
  }
  DBUG_ASSERT(
      trans_safe || updated_rows == 0 ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));

  if (local_error != 0)
    error_handled = true;  // to force early leave from ::send_error()

  if (local_error > 0)  // if the above log write did not fail ...
  {
    /* Safety: If we haven't got an error before (can happen in do_updates) */
    my_message(ER_UNKNOWN_ERROR, "An error occurred in multi-table update",
               MYF(0));
    return true;
  }

  id = thd->arg_of_last_insert_id_function
           ? thd->first_successful_insert_id_in_prev_stmt
           : 0;

  snprintf(buff, sizeof(buff), ER_THD(thd, ER_UPDATE_INFO), (long)found_rows,
           (long)updated_rows,
           (long)thd->get_stmt_da()->current_statement_cond_count());
  ::my_ok(thd,
          thd->get_protocol()->has_client_capability(CLIENT_FOUND_ROWS)
              ? found_rows
              : updated_rows,
          id, buff);
  return false;
}

bool Sql_cmd_update::accept(THD *thd, Select_lex_visitor *visitor) {
  SELECT_LEX *const select = thd->lex->select_lex;
  // Update tables
  if (select->table_list.elements != 0 &&
      accept_for_join(select->join_list, visitor))
    return true;

  // Update list
  List_iterator<Item> it_value(*update_value_list),
      it_column(select->item_list);
  Item *column, *value;
  while ((column = it_column++) && (value = it_value++))
    if (walk_item(column, visitor) || walk_item(value, visitor)) return true;

  // Where clause
  if (select->where_cond() != nullptr &&
      walk_item(select->where_cond(), visitor))
    return true;

  // Order clause
  if (accept_for_order(select->order_list, visitor)) return true;

  // Limit clause
  if (select->explicit_limit)
    if (walk_item(select->select_limit, visitor)) return true;

  return visitor->visit(select);
}
