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

/**
  @file

  @brief
  Query execution


  @defgroup Query_Executor  Query Executor
  @{
*/

#include "sql/sql_executor.h"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "field_types.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "mem_root_allocator.h"
#include "mem_root_deque.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/basic_row_iterators.h"
#include "sql/bka_iterator.h"
#include "sql/composite_iterators.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/filesort.h"  // Filesort
#include "sql/handler.h"
#include "sql/hash_join_iterator.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/item_sum.h"  // Item_sum
#include "sql/json_dom.h"  // Json_wrapper
#include "sql/key.h"       // key_cmp
#include "sql/key_spec.h"
#include "sql/mem_root_array.h"
#include "sql/mysqld.h"  // stage_executing
#include "sql/nested_join.h"
#include "sql/opt_costmodel.h"
#include "sql/opt_explain_format.h"
#include "sql/opt_range.h"  // QUICK_SELECT_I
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_nodes.h"  // PT_frame
#include "sql/query_options.h"
#include "sql/record_buffer.h"  // Record_buffer
#include "sql/records.h"
#include "sql/ref_row_iterators.h"
#include "sql/row_iterator.h"
#include "sql/sort_param.h"
#include "sql/sorting_iterator.h"
#include "sql/sql_base.h"  // fill_record
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_join_buffer.h"
#include "sql/sql_list.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_select.h"
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/temp_table_param.h"  // Mem_root_vector
#include "sql/timing_iterator.h"
#include "sql/window.h"
#include "sql/window_lex.h"
#include "sql_string.h"
#include "tables_contained_in.h"
#include "template_utils.h"
#include "thr_lock.h"

using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

static void join_setup_iterator(QEP_TAB *tab);
static int read_system(TABLE *table);
static int read_const(TABLE *table, TABLE_REF *ref);
static bool alloc_group_fields(JOIN *join, ORDER *group);
static void SetCostOnTableIterator(const Cost_model_server &cost_model,
                                   const POSITION *pos, bool is_after_filter,
                                   RowIterator *iterator);
static inline pair<uchar *, key_part_map> FindKeyBufferAndMap(
    const TABLE_REF *ref);

/// Maximum amount of space (in bytes) to allocate for a Record_buffer.
static constexpr size_t MAX_RECORD_BUFFER_SIZE = 128 * 1024;  // 128KB

string RefToString(const TABLE_REF &ref, const KEY *key, bool include_nulls) {
  string ret;

  if (ref.keypart_hash != nullptr) {
    DBUG_ASSERT(!include_nulls);
    ret = key->key_part[0].field->field_name;
    ret += "=hash(";
    for (unsigned key_part_idx = 0; key_part_idx < ref.key_parts;
         ++key_part_idx) {
      if (key_part_idx != 0) {
        ret += ", ";
      }
      ret += ItemToString(ref.items[key_part_idx]);
    }
    ret += ")";
    return ret;
  }

  const uchar *key_buff = ref.key_buff;

  for (unsigned key_part_idx = 0; key_part_idx < ref.key_parts;
       ++key_part_idx) {
    if (key_part_idx != 0) {
      ret += ", ";
    }
    const Field *field = key->key_part[key_part_idx].field;
    if (field->is_field_for_functional_index()) {
      // Do not print out the column name if the column represents a functional
      // index. Instead, print out the indexed expression.
      ret += ItemToString(field->gcol_info->expr_item);
    } else {
      DBUG_ASSERT(!field->is_hidden_from_user());
      ret += field->field_name;
    }
    ret += "=";
    ret += ItemToString(ref.items[key_part_idx]);

    // If we have ref_or_null access, find out if this keypart is the one that
    // is -or-NULL (there's always only a single one).
    if (include_nulls && key_buff == ref.null_ref_key) {
      ret += " or NULL";
    }
    key_buff += key->key_part[key_part_idx].store_length;
  }
  return ret;
}

bool JOIN::create_intermediate_table(QEP_TAB *const tab,
                                     List<Item> *tmp_table_fields,
                                     ORDER_with_src &tmp_table_group,
                                     bool save_sum_fields) {
  DBUG_TRACE;
  THD_STAGE_INFO(thd, stage_creating_tmp_table);
  const bool windowing = m_windows.elements > 0;
  /*
    Pushing LIMIT to the temporary table creation is not applicable
    when there is ORDER BY or GROUP BY or aggregate/window functions, because
    in all these cases we need all result rows.
  */
  ha_rows tmp_rows_limit =
      ((order == nullptr || skip_sort_order) && !tmp_table_group &&
       !windowing && !select_lex->with_sum_func)
          ? m_select_limit
          : HA_POS_ERROR;

  tab->tmp_table_param = new (thd->mem_root) Temp_table_param(tmp_table_param);
  tab->tmp_table_param->skip_create_table = true;

  bool distinct_arg =
      select_distinct &&
      // GROUP BY is absent or has been done in a previous step
      !group_list &&
      // We can only do DISTINCT in last window's tmp table step
      (!windowing || (tab->tmp_table_param->m_window &&
                      tab->tmp_table_param->m_window->is_last()));

  TABLE *table =
      create_tmp_table(thd, tab->tmp_table_param, *tmp_table_fields,
                       tmp_table_group, distinct_arg, save_sum_fields,
                       select_lex->active_options(), tmp_rows_limit, "");
  if (!table) return true;
  tmp_table_param.using_outer_summary_function =
      tab->tmp_table_param->using_outer_summary_function;

  DBUG_ASSERT(tab->idx() > 0);
  tab->set_table(table);
  tab->set_temporary_table_deduplicates(distinct_arg ||
                                        tmp_table_group != nullptr);

  /**
    If this is a window's OUT table, any final DISTINCT, ORDER BY will lead to
    windows showing use of tmp table in the final windowing step, so no
    need to signal use of tmp table unless we are here for another tmp table.
  */
  if (!tab->tmp_table_param->m_window) {
    if (table->group)
      explain_flags.set(tmp_table_group.src, ESP_USING_TMPTABLE);
    else if (table->is_distinct || select_distinct)
      explain_flags.set(ESC_DISTINCT, ESP_USING_TMPTABLE);
    else {
      /*
        Try to find a reason for this table, to show in EXPLAIN.
        If there's no GROUP BY, no ORDER BY, no DISTINCT, it must be just a
        result buffer. If there's ORDER BY but there is also windowing
        then ORDER BY happens after windowing, and here we are before
        windowing, so the table is not for ORDER BY either.
      */
      if ((!group_list && (!order || windowing) && !select_distinct) ||
          (select_lex->active_options() &
           (SELECT_BIG_RESULT | OPTION_BUFFER_RESULT)))
        explain_flags.set(ESC_BUFFER_RESULT, ESP_USING_TMPTABLE);
    }
  }
  /* if group or order on first table, sort first */
  if (group_list && simple_group) {
    DBUG_PRINT("info", ("Sorting for group"));

    if (m_ordered_index_usage != ORDERED_INDEX_GROUP_BY &&
        add_sorting_to_table(const_tables, &group_list))
      goto err;

    if (alloc_group_fields(this, group_list)) goto err;
    if (make_sum_func_list(all_fields, fields_list, true)) goto err;
    const bool need_distinct =
        !(tab->quick() && tab->quick()->is_agg_loose_index_scan());
    if (prepare_sum_aggregators(sum_funcs, need_distinct)) goto err;
    if (setup_sum_funcs(thd, sum_funcs)) goto err;
    group_list = nullptr;
  } else {
    if (make_sum_func_list(all_fields, fields_list, false)) goto err;
    const bool need_distinct =
        !(tab->quick() && tab->quick()->is_agg_loose_index_scan());
    if (prepare_sum_aggregators(sum_funcs, need_distinct)) goto err;
    if (setup_sum_funcs(thd, sum_funcs)) goto err;

    if (!group_list && !table->is_distinct && order && simple_order &&
        !m_windows_sort) {
      DBUG_PRINT("info", ("Sorting for order"));

      if (m_ordered_index_usage != ORDERED_INDEX_ORDER_BY &&
          add_sorting_to_table(const_tables, &order))
        goto err;
      order = nullptr;
    }
  }
  return false;

err:
  if (table != nullptr) {
    free_tmp_table(thd, table);
    tab->set_table(nullptr);
  }
  return true;
}

/**
  Checks if an item has a ROLLUP NULL which needs to be written to
  temp table.

  @param item         Item for which we need to detect if ROLLUP
                      NULL has to be written.

  @returns false if ROLLUP NULL need not be written for this item.
           true if it has to be written.
*/

bool has_rollup_result(Item *item) {
  if (item->type() == Item::NULL_RESULT_ITEM) return true;

  if (item->type() == Item::FUNC_ITEM) {
    for (uint i = 0; i < ((Item_func *)item)->arg_count; i++) {
      Item *real_item = ((Item_func *)item)->arguments()[i];
      while (real_item->type() == Item::REF_ITEM)
        real_item = *((down_cast<Item_ref *>(real_item))->ref);

      if (real_item->type() == Item::NULL_RESULT_ITEM)
        return true;
      else if (real_item->type() == Item::FUNC_ITEM &&
               has_rollup_result(real_item))
        return true;
    }
  }
  return false;
}

void JOIN::optimize_distinct() {
  for (int i = primary_tables - 1; i >= 0; --i) {
    QEP_TAB *last_tab = qep_tab + i;
    if (select_lex->select_list_tables & last_tab->table_ref->map()) break;
    last_tab->not_used_in_distinct = true;
  }

  /* Optimize "select distinct b from t1 order by key_part_1 limit #" */
  if (order && skip_sort_order) {
    /* Should already have been optimized away */
    DBUG_ASSERT(m_ordered_index_usage == ORDERED_INDEX_ORDER_BY);
    if (m_ordered_index_usage == ORDERED_INDEX_ORDER_BY) {
      order = nullptr;
    }
  }
}

bool prepare_sum_aggregators(Item_sum **func_ptr, bool need_distinct) {
  Item_sum *func;
  DBUG_TRACE;
  while ((func = *(func_ptr++))) {
    if (func->set_aggregator(need_distinct && func->has_with_distinct()
                                 ? Aggregator::DISTINCT_AGGREGATOR
                                 : Aggregator::SIMPLE_AGGREGATOR))
      return true;
  }
  return false;
}

/******************************************************************************
  Code for calculating functions
******************************************************************************/

/**
  Call @c setup() for all sum functions.

  @param thd           thread handler
  @param func_ptr      sum function list

  @retval
    false  ok
  @retval
    true   error
*/

bool setup_sum_funcs(THD *thd, Item_sum **func_ptr) {
  Item_sum *func;
  DBUG_TRACE;
  while ((func = *(func_ptr++))) {
    if (func->aggregator_setup(thd)) return true;
  }
  return false;
}

void init_tmptable_sum_functions(Item_sum **func_ptr) {
  DBUG_TRACE;
  Item_sum *func;
  while ((func = *(func_ptr++))) func->reset_field();
}

/** Update record 0 in tmp_table from record 1. */

void update_tmptable_sum_func(Item_sum **func_ptr,
                              TABLE *tmp_table MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  Item_sum *func;
  while ((func = *(func_ptr++))) func->update_field();
}

/** Copy result of sum functions to record in tmp_table. */

void copy_sum_funcs(Item_sum **func_ptr, Item_sum **end_ptr) {
  DBUG_TRACE;
  for (; func_ptr != end_ptr; func_ptr++) {
    if ((*func_ptr)->get_result_field() != nullptr) {
      (*func_ptr)->save_in_field((*func_ptr)->get_result_field(), true);
    }
  }
}

bool init_sum_functions(Item_sum **func_ptr, Item_sum **end_ptr) {
  for (; func_ptr != end_ptr; func_ptr++) {
    if ((*func_ptr)->reset_and_add()) return true;
  }
  /* If rollup, calculate the upper sum levels */
  for (; *func_ptr; func_ptr++) {
    if ((*func_ptr)->aggregator_add()) return true;
  }
  return false;
}

bool update_sum_func(Item_sum **func_ptr) {
  DBUG_TRACE;
  Item_sum *func;
  for (; (func = *func_ptr); func_ptr++)
    if (func->aggregator_add()) return true;
  return false;
}

/**
  Copy result of functions to record in tmp_table.

  Uses the thread pointer to check for errors in
  some of the val_xxx() methods called by the
  save_in_result_field() function.
  TODO: make the Item::val_xxx() return error code

  @param param     Copy functions of tmp table specified by param
  @param thd       pointer to the current thread for error checking
  @param type      type of function Items that need to be copied (used
                   w.r.t windowing functions).
  @retval
    false if OK
  @retval
    true on error
*/
bool copy_funcs(Temp_table_param *param, const THD *thd, Copy_func_type type) {
  DBUG_TRACE;
  if (!param->items_to_copy->size()) return false;

  Func_ptr_array *func_ptr = param->items_to_copy;
  uint end = func_ptr->size();
  for (uint i = 0; i < end; i++) {
    Func_ptr &func = func_ptr->at(i);
    Item *item = func.func();
    bool do_copy = false;
    switch (type) {
      case CFT_ALL:
        do_copy = true;
        break;
      case CFT_WF_FRAMING:
        do_copy = (item->m_is_window_function &&
                   down_cast<Item_sum *>(item)->framing());
        break;
      case CFT_WF_NON_FRAMING:
        do_copy = (item->m_is_window_function &&
                   !down_cast<Item_sum *>(item)->framing() &&
                   !down_cast<Item_sum *>(item)->needs_card());
        break;
      case CFT_WF_NEEDS_CARD:
        do_copy = (item->m_is_window_function &&
                   down_cast<Item_sum *>(item)->needs_card());
        break;
      case CFT_WF_USES_ONLY_ONE_ROW:
        do_copy = (item->m_is_window_function &&
                   down_cast<Item_sum *>(item)->uses_only_one_row());
        break;
      case CFT_HAS_NO_WF:
        do_copy = !item->m_is_window_function && !item->has_wf();
        break;
      case CFT_HAS_WF:
        do_copy = !item->m_is_window_function && item->has_wf();
        break;
      case CFT_WF:
        do_copy = item->m_is_window_function;
        break;
      case CFT_DEPENDING_ON_AGGREGATE:
        do_copy =
            item->has_aggregation() && item->type() != Item::SUM_FUNC_ITEM;
        break;
    }

    if (do_copy) {
      if (func.override_result_field() == nullptr) {
        item->save_in_field(item->get_result_field(),
                            /*no_conversions=*/true);
      } else {
        item->save_in_field(func.override_result_field(),
                            /*no_conversions=*/true);
      }
      /*
        Need to check the THD error state because Item::val_xxx() don't
        return error code, but can generate errors
        TODO: change it for a real status check when Item::val_xxx()
        are extended to return status code.
      */
      if (thd->is_error()) return true;
    }
  }
  return false;
}

/**
  Check appearance of new constant items in multiple equalities
  of a condition after reading a constant table.

    The function retrieves the cond condition and for each encountered
    multiple equality checks whether new constants have appeared after
    reading the constant (single row) table tab. If so it adjusts
    the multiple equality appropriately.

  @param thd        thread handler
  @param cond       condition whose multiple equalities are to be checked
  @param tab        constant table that has been read
*/

static bool update_const_equal_items(THD *thd, Item *cond, JOIN_TAB *tab) {
  if (!(cond->used_tables() & tab->table_ref->map())) return false;

  if (cond->type() == Item::COND_ITEM) {
    for (Item &item : *(down_cast<Item_cond *>(cond))->argument_list()) {
      if (update_const_equal_items(thd, &item, tab)) return true;
    }
  } else if (cond->type() == Item::FUNC_ITEM &&
             down_cast<Item_func *>(cond)->functype() ==
                 Item_func::MULT_EQUAL_FUNC) {
    Item_equal *item_equal = (Item_equal *)cond;
    bool contained_const = item_equal->get_const() != nullptr;
    if (item_equal->update_const(thd)) return true;
    if (!contained_const && item_equal->get_const()) {
      /* Update keys for range analysis */
      Item_equal_iterator it(*item_equal);
      Item_field *item_field;
      while ((item_field = it++)) {
        const Field *field = item_field->field;
        JOIN_TAB *stat = field->table->reginfo.join_tab;
        Key_map possible_keys = field->key_start;
        possible_keys.intersect(field->table->keys_in_use_for_query);
        stat[0].const_keys.merge(possible_keys);
        stat[0].keys().merge(possible_keys);

        /*
          For each field in the multiple equality (for which we know that it
          is a constant) we have to find its corresponding key part, and set
          that key part in const_key_parts.
        */
        if (!possible_keys.is_clear_all()) {
          TABLE *const table = field->table;
          for (Key_use *use = stat->keyuse();
               use && use->table_ref == item_field->table_ref; use++) {
            if (possible_keys.is_set(use->key) &&
                table->key_info[use->key].key_part[use->keypart].field == field)
              table->const_key_parts[use->key] |= use->keypart_map;
          }
        }
      }
    }
  }
  return false;
}

/**
  @brief Setup write_func of QEP_tmp_table object

  @param tab QEP_TAB of a tmp table
  @param trace Opt_trace_object to add to
  @details
  Function sets up write_func according to how QEP_tmp_table object that
  is attached to the given join_tab will be used in the query.
*/

void setup_tmptable_write_func(QEP_TAB *tab, Opt_trace_object *trace) {
  DBUG_TRACE;
  JOIN *join = tab->join();
  TABLE *table = tab->table();
  Temp_table_param *const tmp_tbl = tab->tmp_table_param;
  uint phase = tab->ref_item_slice;
  const char *description = nullptr;
  DBUG_ASSERT(table);

  if (table->group && tmp_tbl->sum_func_count &&
      !tmp_tbl->precomputed_group_by) {
    /*
      Note for MyISAM tmp tables: if uniques is true keys won't be
      created.
    */
    DBUG_ASSERT(phase < REF_SLICE_WIN_1);
    if (table->s->keys) {
      description = "continuously_update_group_row";
      tab->op_type = QEP_TAB::OT_AGGREGATE_INTO_TMP_TABLE;
    }
  } else if (join->streaming_aggregation && !tmp_tbl->precomputed_group_by) {
    DBUG_ASSERT(phase < REF_SLICE_WIN_1);
    description = "write_group_row_when_complete";
    DBUG_PRINT("info", ("Using end_write_group"));
    tab->op_type = QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE;
  } else {
    description = "write_all_rows";
    tab->op_type = (phase >= REF_SLICE_WIN_1 ? QEP_TAB::OT_WINDOWING_FUNCTION
                                             : QEP_TAB::OT_MATERIALIZE);
    if (tmp_tbl->precomputed_group_by) {
      Item_sum **func_ptr = join->sum_funcs;
      Item_sum *func;
      while ((func = *(func_ptr++))) {
        tmp_tbl->items_to_copy->push_back(Func_ptr(func));
      }
    }
  }
  if (description) trace->add_alnum("write_method", description);
}

/**
  @details
  Rows produced by a join sweep may end up in a temporary table or be sent
  to a client. Setup the function of the nested loop join algorithm which
  handles final fully constructed and matched records.

  @return
    end_select function to use. This function can't fail.
*/
QEP_TAB::enum_op_type JOIN::get_end_select_func() {
  DBUG_TRACE;
  /*
     Choose method for presenting result to user. Use end_send_group
     if the query requires grouping (has a GROUP BY clause and/or one or
     more aggregate functions). Use end_send if the query should not
     be grouped.
   */
  if (streaming_aggregation && !tmp_table_param.precomputed_group_by) {
    DBUG_PRINT("info", ("Using end_send_group"));
    return QEP_TAB::OT_AGGREGATE;
  }
  DBUG_PRINT("info", ("Using end_send"));
  return QEP_TAB::OT_NONE;
}

/**
  Find out how many bytes it takes to store the smallest prefix which
  covers all the columns that will be read from a table.

  @param qep_tab the table to read
  @return the size of the smallest prefix that covers all records to be
          read from the table
*/
static size_t record_prefix_size(const QEP_TAB *qep_tab) {
  const TABLE *table = qep_tab->table();

  /*
    Find the end of the last column that is read, or the beginning of
    the record if no column is read.

    We want the column that is physically last in table->record[0],
    which is not necessarily the column that is last in table->field.
    For example, virtual columns come at the end of the record, even
    if they are not at the end of table->field. This means we need to
    inspect all the columns in the read set and take the one with the
    highest end pointer.
  */
  uchar *prefix_end = table->record[0];  // beginning of record
  for (auto f = table->field, end = table->field + table->s->fields; f < end;
       ++f) {
    if (bitmap_is_set(table->read_set, (*f)->field_index))
      prefix_end = std::max(prefix_end, (*f)->ptr + (*f)->pack_length());
  }

  /*
    If this is an index merge, the primary key columns may be required
    for positioning in a later stage, even though they are not in the
    read_set here. Allocate space for them in case they are needed.
    Also allocate space for them for dynamic ranges, because they can
    switch to index merge for a subsequent scan.
  */
  if ((qep_tab->type() == JT_INDEX_MERGE || qep_tab->dynamic_range()) &&
      !table->s->is_missing_primary_key() &&
      (table->file->ha_table_flags() & HA_PRIMARY_KEY_REQUIRED_FOR_POSITION)) {
    const KEY &key = table->key_info[table->s->primary_key];
    for (auto kp = key.key_part, end = kp + key.user_defined_key_parts;
         kp < end; ++kp) {
      const Field *f = table->field[kp->fieldnr - 1];
      /*
        If a key column comes after all the columns in the read set,
        extend the prefix to include the key column.
      */
      prefix_end = std::max(prefix_end, f->ptr + f->pack_length());
    }
  }

  return prefix_end - table->record[0];
}

/**
  Allocate a data buffer that the storage engine can use for fetching
  batches of records.

  A buffer is only allocated if ha_is_record_buffer_wanted() returns true
  for the handler, and the scan in question is of a kind that could be
  expected to benefit from fetching records in batches.

  @param tab the table to read
  @retval true if an error occurred when allocating the buffer
  @retval false if a buffer was successfully allocated, or if a buffer
  was not attempted allocated
*/
bool set_record_buffer(const QEP_TAB *tab) {
  if (tab == nullptr) return false;

  TABLE *const table = tab->table();

  DBUG_ASSERT(table->file->inited);
  DBUG_ASSERT(table->file->ha_get_record_buffer() == nullptr);

  // Skip temporary tables.
  if (tab->position() == nullptr) return false;

  // Don't allocate a buffer for loose index scan.
  if (tab->quick_optim() && tab->quick_optim()->is_loose_index_scan())
    return false;

  // Only create a buffer if the storage engine wants it.
  ha_rows max_rows = 0;
  if (!table->file->ha_is_record_buffer_wanted(&max_rows) || max_rows == 0)
    return false;

  // If we already have a buffer, reuse it.
  if (table->m_record_buffer.max_records() > 0) {
    /*
      Assume that the existing buffer has the shape we want. That is, the
      record size shouldn't change for a table during execution.
    */
    DBUG_ASSERT(table->m_record_buffer.record_size() ==
                record_prefix_size(tab));
    table->m_record_buffer.reset();
    table->file->ha_set_record_buffer(&table->m_record_buffer);
    return false;
  }

  // How many rows do we expect to fetch?
  double rows_to_fetch = tab->position()->rows_fetched;

  /*
    If this is the outer table of a join and there is a limit defined
    on the query block, adjust the buffer size accordingly.
  */
  const JOIN *const join = tab->join();
  if (tab->idx() == 0 && join->m_select_limit != HA_POS_ERROR) {
    /*
      Estimated number of rows returned by the join per qualifying row
      in the outer table.
    */
    double fanout = 1.0;
    for (uint i = 1; i < join->primary_tables; i++) {
      const auto p = join->qep_tab[i].position();
      fanout *= p->rows_fetched * p->filter_effect;
    }

    /*
      The number of qualifying rows to read from the outer table in
      order to reach the limit is limit / fanout. Divide by
      filter_effect to get the total number of qualifying and
      non-qualifying rows to fetch to reach the limit.
    */
    rows_to_fetch = std::min(rows_to_fetch, join->m_select_limit / fanout /
                                                tab->position()->filter_effect);
  }

  ha_rows rows_in_buffer = static_cast<ha_rows>(std::ceil(rows_to_fetch));

  // No need for a multi-row buffer if we don't expect multiple rows.
  if (rows_in_buffer <= 1) return false;

  /*
    How much space do we need to allocate for each record? Enough to
    hold all columns from the beginning and up to the last one in the
    read set. We don't need to allocate space for unread columns at
    the end of the record.
  */
  const size_t record_size = record_prefix_size(tab);

  // Do not allocate a buffer whose total size exceeds MAX_RECORD_BUFFER_SIZE.
  if (record_size > 0)
    rows_in_buffer =
        std::min<ha_rows>(MAX_RECORD_BUFFER_SIZE / record_size, rows_in_buffer);

  // Do not allocate space for more rows than the handler asked for.
  rows_in_buffer = std::min(rows_in_buffer, max_rows);

  const auto bufsize = Record_buffer::buffer_size(rows_in_buffer, record_size);
  const auto ptr = static_cast<uchar *>(table->in_use->alloc(bufsize));
  if (ptr == nullptr) return true; /* purecov: inspected */

  table->m_record_buffer = Record_buffer{rows_in_buffer, record_size, ptr};
  table->file->ha_set_record_buffer(&table->m_record_buffer);
  return false;
}

/**
  Split AND conditions into their constituent parts, recursively.
  Conditions that are not AND conditions are appended unchanged onto
  condition_parts. E.g. if you have ((a AND b) AND c), condition_parts
  will contain [a, b, c], plus whatever it contained before the call.
 */
static void ExtractConditions(Item *condition,
                              vector<Item *> *condition_parts) {
  if (condition == nullptr) {
    return;
  }
  if (condition->type() != Item::COND_ITEM ||
      down_cast<Item_cond *>(condition)->functype() !=
          Item_bool_func2::COND_AND_FUNC) {
    condition_parts->push_back(condition);
    return;
  }

  Item_cond_and *and_condition = down_cast<Item_cond_and *>(condition);
  for (Item &item : *and_condition->argument_list()) {
    ExtractConditions(&item, condition_parts);
  }
}

/**
  Return a new iterator that wraps "iterator" and that tests all of the given
  conditions (if any), ANDed together. If there are no conditions, just return
  the given iterator back.
 */
unique_ptr_destroy_only<RowIterator> PossiblyAttachFilterIterator(
    unique_ptr_destroy_only<RowIterator> iterator,
    const vector<Item *> &conditions, THD *thd,
    table_map *conditions_depend_on_outer_tables) {
  // See if any of the sub-conditions are known to be always false,
  // and filter out any conditions that are known to be always true.
  List<Item> items;
  for (Item *cond : conditions) {
    if (cond->const_item()) {
      if (cond->val_int() == 0) {
        unique_ptr_destroy_only<RowIterator> zero_iterator =
            NewIterator<ZeroRowsIterator>(thd, "Impossible filter",
                                          move(iterator));
        zero_iterator->set_expected_rows(0.0);
        zero_iterator->set_estimated_cost(0.0);
        return zero_iterator;
      } else {
        // Known to be always true, so skip it.
      }
    } else {
      items.push_back(cond);
    }
  }

  Item *condition = nullptr;
  if (items.size() == 0) {
    return iterator;
  } else if (items.size() == 1) {
    condition = items.head();
  } else {
    condition = new Item_cond_and(items);
    condition->quick_fix_field();
    condition->update_used_tables();
    condition->apply_is_true();
  }
  *conditions_depend_on_outer_tables |= condition->used_tables();

  RowIterator *child_iterator = iterator.get();
  unique_ptr_destroy_only<RowIterator> filter_iterator =
      NewIterator<FilterIterator>(thd, move(iterator), condition);

  // Copy costs (we don't care about filter_effect here, even though we
  // should).
  filter_iterator->set_expected_rows(child_iterator->expected_rows());
  filter_iterator->set_estimated_cost(child_iterator->estimated_cost());

  return filter_iterator;
}

unique_ptr_destroy_only<RowIterator> CreateNestedLoopIterator(
    THD *thd, unique_ptr_destroy_only<RowIterator> left_iterator,
    unique_ptr_destroy_only<RowIterator> right_iterator, JoinType join_type,
    bool pfs_batch_mode) {
  if (join_type == JoinType::ANTI || join_type == JoinType::SEMI) {
    // This does not make sense as an optimization for anti- or semijoins.
    pfs_batch_mode = false;
  }

  return NewIterator<NestedLoopIterator>(thd, move(left_iterator),
                                         move(right_iterator), join_type,
                                         pfs_batch_mode);
}

static unique_ptr_destroy_only<RowIterator> CreateInvalidatorIterator(
    THD *thd, QEP_TAB *qep_tab, unique_ptr_destroy_only<RowIterator> iterator) {
  RowIterator *child_iterator = iterator.get();

  unique_ptr_destroy_only<RowIterator> invalidator =
      NewIterator<CacheInvalidatorIterator>(thd, move(iterator),
                                            qep_tab->table()->alias);

  // Copy costs.
  invalidator->set_expected_rows(child_iterator->expected_rows());
  invalidator->set_estimated_cost(child_iterator->estimated_cost());

  table_map deps = qep_tab->lateral_derived_tables_depend_on_me;
  for (QEP_TAB **tab2 = qep_tab->join()->map2qep_tab; deps;
       tab2++, deps >>= 1) {
    if (!(deps & 1)) continue;
    if ((*tab2)->invalidators == nullptr) {
      (*tab2)->invalidators = new (thd->mem_root)
          Mem_root_array<const CacheInvalidatorIterator *>(thd->mem_root);
    }
    (*tab2)->invalidators->push_back(
        down_cast<CacheInvalidatorIterator *>(invalidator->real_iterator()));
  }
  return invalidator;
}

static table_map ConvertQepTabMapToTableMap(JOIN *join, qep_tab_map tables) {
  table_map map = 0;
  for (QEP_TAB *tab : TablesContainedIn(join, tables)) {
    map |= tab->table_ref->map();
  }
  return map;
}

unique_ptr_destroy_only<RowIterator> CreateBKAIterator(
    THD *thd, JOIN *join, unique_ptr_destroy_only<RowIterator> iterator,
    qep_tab_map left_tables,
    unique_ptr_destroy_only<RowIterator> subtree_iterator,
    qep_tab_map right_tables, TABLE *table, TABLE_LIST *table_list,
    TABLE_REF *ref, MultiRangeRowIterator *mrr_iterator, JoinType join_type) {
  table_map left_table_map = ConvertQepTabMapToTableMap(join, left_tables);
  table_map right_table_map = ConvertQepTabMapToTableMap(join, right_tables);

  // If the BKA join condition (the “ref”) references fields that are outside
  // what we have available for this join, it is because they were
  // substituted by multi-equalities earlier (which assumes the
  // pre-iterator executor, which goes outside-in and not inside-out),
  // so find those multi-equalities and rewrite the fields back.
  for (uint part_no = 0; part_no < ref->key_parts; ++part_no) {
    Item *item = ref->items[part_no];
    if (item->type() == Item::FUNC_ITEM || item->type() == Item::COND_ITEM) {
      Item_func *func_item = down_cast<Item_func *>(item);
      if (func_item->functype() == Item_func::EQ_FUNC) {
        down_cast<Item_func_eq *>(func_item)
            ->ensure_multi_equality_fields_are_available(left_table_map,
                                                         right_table_map);
      }
    } else if (item->type() == Item::FIELD_ITEM) {
      if (ref->key_copy[part_no] == nullptr) {
        // A constant, so no need to propagate.
        continue;
      }

      bool dummy;
      Item_equal *item_eq = find_item_equal(
          table_list->cond_equal, down_cast<Item_field *>(item), &dummy);
      if (item_eq == nullptr) {
        // Didn't come from a multi-equality.
        continue;
      }

      item->walk(&Item::ensure_multi_equality_fields_are_available_walker,
                 enum_walk::POSTFIX, pointer_cast<uchar *>(&left_table_map));
      down_cast<store_key_field *>(ref->key_copy[part_no])
          ->replace_from_field(down_cast<Item_field *>(item)->field);
    }
  }

  const float rec_per_key =
      table->key_info[ref->key].records_per_key(ref->key_parts - 1);
  return NewIterator<BKAIterator>(
      thd, join, move(iterator), left_tables, move(subtree_iterator),
      thd->variables.join_buff_size, table->file->stats.mrr_length_per_rec,
      rec_per_key, mrr_iterator, join_type);
}

static unique_ptr_destroy_only<RowIterator> PossiblyAttachFilterIterator(
    unique_ptr_destroy_only<RowIterator> iterator,
    const vector<PendingCondition> &conditions, THD *thd,
    table_map *conditions_depend_on_outer_tables) {
  vector<Item *> stripped_conditions;
  for (const PendingCondition &cond : conditions) {
    stripped_conditions.push_back(cond.cond);
  }
  return PossiblyAttachFilterIterator(move(iterator), stripped_conditions, thd,
                                      conditions_depend_on_outer_tables);
}

static Item_func_trig_cond *GetTriggerCondOrNull(Item *item) {
  if (item->type() == Item::FUNC_ITEM &&
      down_cast<Item_func *>(item)->functype() ==
          Item_bool_func2::TRIG_COND_FUNC) {
    return down_cast<Item_func_trig_cond *>(item);
  } else {
    return nullptr;
  }
}

enum CallingContext {
  TOP_LEVEL,
  DIRECTLY_UNDER_SEMIJOIN,
  DIRECTLY_UNDER_OUTER_JOIN,
  DIRECTLY_UNDER_WEEDOUT
};

/**
  For historical reasons, derived table materialization and temporary
  table materialization didn't specify the fields to materialize in the
  same way. Temporary table materialization used copy_fields() and
  copy_funcs() (also reused for aggregation; see the comments on
  AggregateIterator for the relation between aggregations and temporary
  tables) to get the data into the Field pointers of the temporary table
  to be written, storing the lists in copy_fields and items_to_copy.

  However, derived table materialization used JOIN::fields (which is a
  set of Item, not Field!) for the same purpose, calling fill_record()
  (which originally was meant for INSERT and UPDATE) instead. Thus, we
  have to rewrite one to the other, so that we can have only one
  MaterializeIterator. We choose to rewrite JOIN::fields to
  copy_fields/items_to_copy.

  TODO: The optimizer should output just one kind of structure directly.
 */
void ConvertItemsToCopy(List<Item> *items, Field **fields,
                        Temp_table_param *param, JOIN *join) {
  DBUG_ASSERT(param->items_to_copy == nullptr);

  const bool replaced_items_for_rollup =
      (join != nullptr && join->replaced_items_for_rollup);

  // All fields are to be copied.
  Func_ptr_array *copy_func =
      new (current_thd->mem_root) Func_ptr_array(current_thd->mem_root);
  Field **field_ptr = fields;
  for (Item &item : *items) {
    Item *real_item = item.real_item();
    if (real_item->type() == Item::FIELD_ITEM) {
      Field *from_field = (pointer_cast<Item_field *>(real_item))->field;
      Field *to_field = *field_ptr;
      param->copy_fields.emplace_back(to_field, from_field, /*save=*/true);

      // If any of the Item_null_result items are set to save in this field,
      // forward them to the new field instead. See below for the result fields
      // for the other items.
      if (replaced_items_for_rollup) {
        for (size_t rollup_level = 0; rollup_level < join->send_group_parts;
             ++rollup_level) {
          for (Item &item_r : join->rollup.fields_list[rollup_level]) {
            if (item_r.type() == Item::NULL_RESULT_ITEM &&
                item_r.get_result_field() == from_field) {
              item_r.set_result_field(to_field);
            }
          }
        }
      }
    } else if (item.real_item()->is_result_field()) {
      Field *from_field = item.real_item()->get_result_field();
      Field *to_field = *field_ptr;
      item.set_result_field(to_field);
      copy_func->push_back(Func_ptr(&item));

      // Similarly to above, set the right result field for any aggregates
      // that we might output as part of rollup.
      if (replaced_items_for_rollup && &item != real_item) {
        for (Item_sum **func_ptr = join->sum_funcs;
             func_ptr != join->sum_funcs_end[join->send_group_parts];
             ++func_ptr) {
          if ((*func_ptr)->get_result_field() == from_field) {
            (*func_ptr)->set_result_field(to_field);
          }
        }
      }
    } else {
      Func_ptr ptr(&item);
      ptr.set_override_result_field(*field_ptr);
      copy_func->push_back(ptr);
    }
    ++field_ptr;
  }
  param->items_to_copy = copy_func;

  if (replaced_items_for_rollup) {
    // Patch up the rollup items so that they save in the same field as
    // the ref would. This is required because we call save_in_result_field()
    // directly on each field in the rollup field list
    // (in AggregateIterator::Read), not on the Item_ref in join->fields.
    for (size_t rollup_level = 0; rollup_level < join->send_group_parts;
         ++rollup_level) {
      List_STL_Iterator<Item> item_it = join->fields->begin();
      for (Item &item : join->rollup.fields_list[rollup_level]) {
        // For cases where we need an Item_null_result, the field in
        // join->fields often does not have the right result field set.
        // However, the Item_null_result field does after we patched it
        // up earlier in the function.
        if (item.type() != Item::NULL_RESULT_ITEM) {
          item.set_result_field(item_it->get_result_field());
        }
        ++item_it;
      }
    }
  }
}

/** Similar to PendingCondition, but for cache invalidator iterators. */
struct PendingInvalidator {
  /**
    The table whose every (post-join) row invalidates one or more derived
    lateral tables.
   */
  QEP_TAB *qep_tab;
  int table_index_to_attach_to;  // -1 means “on the last possible outer join”.
};

/// @param item The item we want to see if is a join condition.
/// @param qep_tab The table we are joining in.
/// @returns true if 'item' is a join condition for a join involving the given
///   table (both equi-join and non-equi-join condition).
static bool IsJoinCondition(const Item *item, const QEP_TAB *qep_tab) {
  table_map used_tables = item->used_tables();
  if ((~qep_tab->table_ref->map() & used_tables) != 0) {
    // This is a join condition (either equi-join or non-equi-join).
    return true;
  }

  return false;
}

/// @returns the innermost condition of a nested trigger condition. If the item
///   is not a trigger condition, the item itself is returned.
static Item *GetInnermostCondition(Item *item) {
  Item_func_trig_cond *trig_cond = GetTriggerCondOrNull(item);
  while (trig_cond != nullptr) {
    item = trig_cond->arguments()[0];
    trig_cond = GetTriggerCondOrNull(item);
  }

  return item;
}

/*
  There are three kinds of conditions stored into a table's QEP_TAB object:

  1. Join conditions (where not optimized into EQ_REF accesses or similar).
     These are attached as a condition on the rightmost table of the join;
     if it's an outer join, they are wrapped in a “not_null_compl”
     condition, to mark that they should not be applied to the NULL values
     synthesized when no row is found. These can be kept on the table, and
     we don't really need the not_null_compl wrapper as long as we don't
     move the condition up above the join (which we don't).

  2. WHERE predicates referring to the table, and possibly also one or more
     earlier tables in the join. These should normally be kept on the table,
     so we can discard rows as early as possible (but see next point).
     We should test these after the join conditions, though, as they may
     have side effects. Also note that these may be pushed below sort
     operations for efficiency -- in fact, they already have, so we should
     not try to re-apply them.

  3. Predicates like in #2 that are on the inner (right) side of a
     left join. These conditions must be moved _above_ the join, as they
     should also be tested for NULL-complemented rows the join may generate.
     E.g., for t1 LEFT JOIN t2 WHERE t1.x + t2.x > 3, the condition will be
     attached to t2's QEP_TAB, but needs to be attached above the join, or
     it would erroneously keep rows wherever t2 did not produce a
     (real) row. Such conditions are marked with a “found” trigger (in the
     old execution engine, which tested qep_tab->condition() both before and
     after the join, it would need to be exempt from the first test).

  4. Predicates that are #1 _and_ #3. These can happen with more complicated
     outer joins; e.g., with t1 LEFT JOIN ( t2 LEFT JOIN t3 ON <x> ) ON <y>,
     the <x> join condition (posted on t3) should be above one join but
     below the other.

  TODO: The optimizer should distinguish between before-join and
  after-join conditions to begin with, instead of us having to untangle
  it here.
 */
void SplitConditions(Item *condition, QEP_TAB *current_table,
                     vector<Item *> *predicates_below_join,
                     vector<PendingCondition> *predicates_above_join,
                     vector<PendingCondition> *join_conditions) {
  vector<Item *> condition_parts;
  ExtractConditions(condition, &condition_parts);
  for (Item *item : condition_parts) {
    Item_func_trig_cond *trig_cond = GetTriggerCondOrNull(item);
    if (trig_cond != nullptr) {
      Item *inner_cond = trig_cond->arguments()[0];
      if (trig_cond->get_trig_type() == Item_func_trig_cond::FOUND_MATCH) {
        // A WHERE predicate on the table that needs to be pushed up above the
        // join (case #3 above). Push it up to above the last outer join.
        predicates_above_join->push_back(PendingCondition{inner_cond, -1});
      } else if (trig_cond->get_trig_type() ==
                 Item_func_trig_cond::IS_NOT_NULL_COMPL) {
        // It's a join condition, so it should nominally go directly onto the
        // table. If it _also_ has a FOUND_MATCH predicate, we are dealing
        // with case #4 above, and need to push it up to exactly the right
        // spot.
        //
        // There is a special exception here for antijoins; see the code under
        // qep_tab->table()->reginfo.not_exists_optimize in ConnectJoins().
        Item_func_trig_cond *inner_trig_cond = GetTriggerCondOrNull(inner_cond);
        if (inner_trig_cond != nullptr) {
          // Note that we can have a condition inside multiple levels of a
          // trigger condition. We want the innermost condition, as we really do
          // not care about trigger conditions after this point.
          Item *inner_inner_cond = GetInnermostCondition(inner_trig_cond);
          if (join_conditions != nullptr) {
            // If join_conditions is set, it indicates that we are on the right
            // side of an outer join that will be executed using hash join. The
            // condition must be moved to the point where the hash join iterator
            // is created, so the condition can be attached to the iterator.
            join_conditions->push_back(
                PendingCondition{inner_inner_cond, trig_cond->idx()});
          } else {
            predicates_above_join->push_back(
                PendingCondition{inner_inner_cond, inner_trig_cond->idx()});
          }
        } else {
          if (join_conditions != nullptr) {
            // Similar to the left join above: If join_conditions is set,
            // it indicates that we are on the inner side of an antijoin (we are
            // dealing with the NOT IN side in the below example), and the
            // antijoin will be executed using hash join:
            //
            //   SELECT * FROM t1 WHERE t1.col1 NOT IN (SELECT t2.col1 FROM t2);
            //
            // In this case, the condition must be moved up to the outer side
            // where the hash join iterator is created, so it can be attached
            // to the iterator.
            join_conditions->push_back(
                PendingCondition{inner_cond, trig_cond->idx()});
          } else {
            predicates_below_join->push_back(inner_cond);
          }
        }
      } else {
        predicates_below_join->push_back(item);
      }
    } else {
      if (current_table->match_tab != NO_PLAN_IDX &&
          join_conditions != nullptr && IsJoinCondition(item, current_table)) {
        // We are on the inner side of a semijoin, and the item we are looking
        // at is a join condition. In addition, the join will be executed using
        // hash join. Move the join condition up to the table we are semijoining
        // against (where the join iterator is created), so that it can be
        // attached to the hash join iterator.
        join_conditions->push_back(
            PendingCondition{item, current_table->match_tab});
      } else {
        predicates_below_join->push_back(item);
      }
    }
  }
}

/**
  For a given duplicate weedout operation, figure out which tables are supposed
  to be deduplicated by it, and add those to unhandled_duplicates. (SJ_TMP_TABLE
  contains the deduplication key, which is exactly the complement of the tables
  to be deduplicated.)
 */
static void MarkUnhandledDuplicates(SJ_TMP_TABLE *weedout,
                                    plan_idx weedout_start,
                                    plan_idx weedout_end,
                                    qep_tab_map *unhandled_duplicates) {
  DBUG_ASSERT(weedout_start >= 0);
  DBUG_ASSERT(weedout_end >= 0);

  qep_tab_map weedout_range = TablesBetween(weedout_start, weedout_end);
  if (weedout->is_confluent) {
    // Confluent weedout doesn't have tabs or tabs_end set; it just implicitly
    // says none of the tables are allowed to produce duplicates.
  } else {
    // Remove all tables that are part of the key.
    for (SJ_TMP_TABLE_TAB *tab = weedout->tabs; tab != weedout->tabs_end;
         ++tab) {
      weedout_range &= ~tab->qep_tab->idx_map();
    }
  }
  *unhandled_duplicates |= weedout_range;
}

static unique_ptr_destroy_only<RowIterator> CreateWeedoutIterator(
    THD *thd, unique_ptr_destroy_only<RowIterator> iterator,
    SJ_TMP_TABLE *weedout_table) {
  if (weedout_table->is_confluent) {
    // A “confluent” weedout is one that deduplicates on all the
    // fields. If so, we can drop the complexity of the WeedoutIterator
    // and simply insert a LIMIT 1.
    return NewIterator<LimitOffsetIterator>(
        thd, move(iterator), /*limit=*/1, /*offset=*/0,
        /*count_all_rows=*/false, /*skipped_rows=*/nullptr);
  } else {
    return NewIterator<WeedoutIterator>(thd, move(iterator), weedout_table);
  }
}

static unique_ptr_destroy_only<RowIterator> CreateWeedoutIteratorForTables(
    THD *thd, const qep_tab_map tables_to_deduplicate, QEP_TAB *qep_tabs,
    uint primary_tables, unique_ptr_destroy_only<RowIterator> iterator) {
  Prealloced_array<SJ_TMP_TABLE_TAB, MAX_TABLES> sj_tabs(PSI_NOT_INSTRUMENTED);
  for (uint i = 0; i < primary_tables; ++i) {
    if (!ContainsTable(tables_to_deduplicate, i)) {
      SJ_TMP_TABLE_TAB sj_tab;
      sj_tab.qep_tab = &qep_tabs[i];
      sj_tabs.push_back(sj_tab);

      // See JOIN::add_sorting_to_table() for rationale.
      Filesort *filesort = qep_tabs[i].filesort;
      if (filesort != nullptr) {
        DBUG_ASSERT(filesort->m_sort_param.m_addon_fields_status ==
                    Addon_fields_status::unknown_status);
        filesort->m_force_sort_positions = true;
      }
    }
  }

  JOIN *join = qep_tabs[0].join();
  SJ_TMP_TABLE *sjtbl =
      create_sj_tmp_table(thd, join, &sj_tabs[0], &sj_tabs[0] + sj_tabs.size());
  return CreateWeedoutIterator(thd, move(iterator), sjtbl);
}

enum class Substructure { NONE, OUTER_JOIN, SEMIJOIN, WEEDOUT };

/**
  Given a range of tables (where we assume that we've already handled
  first_idx..(this_idx-1) as inner joins), figure out whether this is a
  semijoin, an outer join or a weedout. In general, the outermost structure
  wins; if we are in one of the rare cases where there are e.g. coincident
  (first match) semijoins and weedouts, we do various forms of conflict
  resolution:

   - Unhandled weedouts will add elements to unhandled_duplicates
     (to be handled at the top level of the query).
   - Unhandled semijoins will either:
     * Set add_limit_1 to true, which means a LIMIT 1 iterator should
       be added, or
     * Add elements to unhandled_duplicates in situations that cannot
       be solved by a simple one-table, one-row LIMIT.

  If not returning NONE, substructure_end will also be filled with where this
  sub-join ends (exclusive).
 */
static Substructure FindSubstructure(
    QEP_TAB *qep_tabs, const plan_idx first_idx, const plan_idx this_idx,
    const plan_idx last_idx, CallingContext calling_context, bool *add_limit_1,
    plan_idx *substructure_end, qep_tab_map *unhandled_duplicates) {
  QEP_TAB *qep_tab = &qep_tabs[this_idx];
  bool is_outer_join =
      qep_tab->last_inner() != NO_PLAN_IDX && qep_tab->last_inner() < last_idx;
  plan_idx outer_join_end =
      qep_tab->last_inner() + 1;  // Only valid if is_outer_join.

  // See if this table marks the end of the left side of a semijoin.
  bool is_semijoin = false;
  plan_idx semijoin_end = NO_PLAN_IDX;
  for (plan_idx j = this_idx; j < last_idx; ++j) {
    if (qep_tabs[j].firstmatch_return == this_idx - 1) {
      is_semijoin = true;
      semijoin_end = j + 1;
      break;
    }
  }

  // Outer joins (or semijoins) wrapping a weedout is tricky,
  // especially in edge cases. If we have an outer join wrapping
  // a weedout, the outer join needs to be processed first.
  // But the weedout wins if it's strictly larger than the outer join.
  // However, a problem occurs if the weedout wraps two consecutive
  // outer joins (which can happen if the join optimizer interleaves
  // tables from different weedouts and needs to combine them into
  // one larger weedout). E.g., consider a join order such as
  //
  //   a LEFT JOIN (b,c) LEFT JOIN (d,e)
  //
  // where there is _also_ a weedout wrapping all four tables [b,e].
  // (Presumably, there were originally two weedouts b+e and c+d,
  // but due to reordering, they were combined into one.)
  // In this case, we have a non-hierarchical situation since the
  // (a,(b,c)) join only partially overlaps with the [b,e] weedout.
  //
  // We solve these non-hierarchical cases by punting them upwards;
  // we signal that they are simply not done by adding them to
  // unhandled_duplicates, and then drop the weedout. The top level
  // will then add a final weedout after all joins. In some cases,
  // it is possible to push the weedout further down than this,
  // but these cases are so marginal that it's not worth it.

  // See if this table starts a weedout operation.
  bool is_weedout = false;
  plan_idx weedout_end = NO_PLAN_IDX;
  if (qep_tab->starts_weedout() &&
      !(calling_context == DIRECTLY_UNDER_WEEDOUT && this_idx == first_idx)) {
    for (plan_idx j = this_idx; j < last_idx; ++j) {
      if (qep_tabs[j].check_weed_out_table == qep_tab->flush_weedout_table) {
        weedout_end = j + 1;
        break;
      }
    }
    if (weedout_end != NO_PLAN_IDX) {
      is_weedout = true;
    }
  }

  if (weedout_end > last_idx) {
    // See comment above.
    MarkUnhandledDuplicates(qep_tab->flush_weedout_table, this_idx, weedout_end,
                            unhandled_duplicates);
    is_weedout = false;
  }

  *add_limit_1 = false;
  if (is_outer_join && is_weedout) {
    if (outer_join_end > weedout_end) {
      // Weedout will be handled at a lower recursion level.
      is_weedout = false;
    } else {
      if (qep_tab->flush_weedout_table->is_confluent) {
        // We have the case where the right side of an outer join is a confluent
        // weedout. The weedout will return at most one row, so replace the
        // weedout with LIMIT 1.
        *add_limit_1 = true;
      } else {
        // See comment above.
        MarkUnhandledDuplicates(qep_tab->flush_weedout_table, this_idx,
                                weedout_end, unhandled_duplicates);
      }
      is_weedout = false;
    }
  }
  if (is_semijoin && is_weedout) {
    if (semijoin_end > weedout_end) {
      // Weedout will be handled at a lower recursion level.
      is_weedout = false;
    } else {
      // See comment above.
      MarkUnhandledDuplicates(qep_tab->flush_weedout_table, this_idx,
                              weedout_end, unhandled_duplicates);
      is_weedout = false;
    }
  }

  // Occasionally, a subslice may be designated as the right side of both a
  // semijoin _and_ an outer join. This is a fairly odd construction,
  // as it means exactly one row is generated no matter what (negating the
  // point of a semijoin in the first place), and typically happens as the
  // result of the join optimizer reordering tables that have no real bearing
  // on the query, such as ... WHERE t1 IN ( t2.i FROM t2 LEFT JOIN t3 )
  // with the ordering t2, t1, t3 (t3 will now be in such a situation).
  //
  // Nominally, these tables should be optimized away, but this is not the
  // right place for that, so we solve it by adding a LIMIT 1 and then
  // treating the slice as a normal outer join.
  if (is_semijoin && is_outer_join) {
    if (semijoin_end == outer_join_end) {
      *add_limit_1 = true;
      is_semijoin = false;
    } else if (semijoin_end > outer_join_end) {
      // A special case of the special case; there might be more than one
      // outer join contained in this semijoin, e.g. A LEFT JOIN B LEFT JOIN C
      // where the combination B-C is _also_ the right side of a semijoin.
      // The join optimizer should not produce this.
      DBUG_ASSERT(false);
    }
  }

  // Yet another special case like the above; this is when we have a semijoin
  // and then a partially overlapping outer join that ends outside the semijoin.
  // E.g., A JOIN B JOIN C LEFT JOIN D, where A..C denotes a semijoin
  // (C has first match back to A). Verify that it cannot happen.
  if (is_semijoin) {
    for (plan_idx i = this_idx; i < semijoin_end; ++i) {
      DBUG_ASSERT(qep_tabs[i].last_inner() < semijoin_end);
    }
  }

  // We may have detected both a semijoin and an outer join starting at
  // this table. Decide which one is the outermost that is not already
  // processed, so that we recurse in the right order.
  if (calling_context == DIRECTLY_UNDER_SEMIJOIN && this_idx == first_idx &&
      semijoin_end == last_idx) {
    is_semijoin = false;
  } else if (calling_context == DIRECTLY_UNDER_OUTER_JOIN &&
             this_idx == first_idx && outer_join_end == last_idx) {
    is_outer_join = false;
  }
  if (is_semijoin && is_outer_join) {
    DBUG_ASSERT(outer_join_end > semijoin_end);
    is_semijoin = false;
  }

  // If we found any unhandled duplicates, mark in the QEP_TABs that a row ID is
  // needed. This will notify iterators (e.g., HashJoinIterator) that they need
  // to store and restore the row ID.
  if (*unhandled_duplicates != 0) {
    qep_tab_map table_range =
        TablesBetween(first_idx, last_idx) & ~*unhandled_duplicates;
    for (QEP_TAB *tab : TablesContainedIn(qep_tab->join(), table_range)) {
      if (tab->rowid_status == NO_ROWID_NEEDED) {
        tab->rowid_status = NEED_TO_CALL_POSITION_FOR_ROWID;
      }
    }
  }

  DBUG_ASSERT(is_semijoin + is_outer_join + is_weedout <= 1);

  if (is_semijoin) {
    *substructure_end = semijoin_end;
    return Substructure::SEMIJOIN;
  } else if (is_outer_join) {
    *substructure_end = outer_join_end;
    return Substructure::OUTER_JOIN;
  } else if (is_weedout) {
    *substructure_end = weedout_end;
    return Substructure::WEEDOUT;
  } else {
    *substructure_end = NO_PLAN_IDX;  // Not used.
    return Substructure::NONE;
  }
}

/// @cond Doxygen_is_confused
static unique_ptr_destroy_only<RowIterator> ConnectJoins(
    plan_idx upper_first_idx, plan_idx first_idx, plan_idx last_idx,
    QEP_TAB *qep_tabs, THD *thd, CallingContext calling_context,
    vector<PendingCondition> *pending_conditions,
    vector<PendingInvalidator> *pending_invalidators,
    vector<PendingCondition> *pending_join_conditions,
    qep_tab_map *unhandled_duplicates,
    table_map *conditions_depend_on_outer_tables);
/// @endcond

unique_ptr_destroy_only<RowIterator> GetIteratorForDerivedTable(
    THD *thd, QEP_TAB *qep_tab) {
  SELECT_LEX_UNIT *unit = qep_tab->table_ref->derived_unit();
  JOIN *subjoin = nullptr;
  Temp_table_param *tmp_table_param;
  int select_number;

  // If we have a single query block at the end of the QEP_TAB array,
  // it may contain aggregation that have already set up fields and items
  // to copy, and we need to pass those to MaterializeIterator, so reuse its
  // tmp_table_param. If not, make a new object, so that we don't
  // disturb the materialization going on inside our own query block.
  if (unit->is_simple()) {
    subjoin = unit->first_select()->join;
    tmp_table_param = &unit->first_select()->join->tmp_table_param;
    select_number = subjoin->select_lex->select_number;
  } else if (unit->fake_select_lex != nullptr) {
    // NOTE: subjoin here is never used, as ConvertItemsToCopy only uses it
    // for ROLLUP, and fake_select_lex can't have ROLLUP.
    subjoin = unit->fake_select_lex->join;
    tmp_table_param = &unit->fake_select_lex->join->tmp_table_param;
    select_number = unit->fake_select_lex->select_number;
  } else {
    tmp_table_param = new (thd->mem_root) Temp_table_param;
    select_number = unit->first_select()->select_number;
  }
  ConvertItemsToCopy(unit->get_field_list(),
                     qep_tab->table()->visible_field_ptr(), tmp_table_param,
                     subjoin);
  bool copy_fields_and_items_in_materialize = true;
  if (unit->is_simple()) {
    // See if AggregateIterator already does this for us.
    JOIN *join = unit->first_select()->join;
    copy_fields_and_items_in_materialize =
        !join->streaming_aggregation ||
        join->tmp_table_param.precomputed_group_by;
  }

  MaterializeIterator *materialize = nullptr;
  unique_ptr_destroy_only<RowIterator> iterator;

  if (unit->unfinished_materialization()) {
    // The unit is a UNION capable of materializing directly into our result
    // table. This saves us from doing double materialization (first into
    // a UNION result table, then from there into our own).
    //
    // We will already have set up a unique index on the table if
    // required; see TABLE_LIST::setup_materialized_derived_tmp_table().
    iterator = NewIterator<MaterializeIterator>(
        thd, unit->release_query_blocks_to_materialize(), qep_tab->table(),
        move(qep_tab->iterator), qep_tab->table_ref->common_table_expr(), unit,
        /*subjoin=*/nullptr,
        /*ref_slice=*/-1, qep_tab->rematerialize, unit->select_limit_cnt);
    materialize = down_cast<MaterializeIterator *>(iterator->real_iterator());
    if (unit->offset_limit_cnt != 0) {
      // LIMIT is handled inside MaterializeIterator, but OFFSET is not.
      // SQL_CALC_FOUND_ROWS cannot occur in a derived table's definition.
      iterator = NewIterator<LimitOffsetIterator>(
          thd, move(iterator), unit->select_limit_cnt, unit->offset_limit_cnt,
          /*count_all_rows=*/false,
          /*skipped_rows=*/nullptr);
    }
  } else if (qep_tab->table_ref->common_table_expr() == nullptr &&
             qep_tab->rematerialize && qep_tab->using_table_scan()) {
    // We don't actually need the materialization for anything (we would
    // just reading the rows straight out from the table, never to be used
    // again), so we can just stream records directly over to the next
    // iterator. This saves both CPU time and memory (for the temporary
    // table).
    //
    // NOTE: Currently, qep_tab->rematerialize is true only for JSON_TABLE.
    // We could extend this to other situations, such as the leftmost
    // table of the join (assuming nested loop only). The test for CTEs is
    // also conservative; if the CTEs is defined within this join and used
    // only once, we could still stream without losing performance.
    iterator = NewIterator<StreamingIterator>(
        thd, unit->release_root_iterator(), &subjoin->tmp_table_param,
        qep_tab->table(), copy_fields_and_items_in_materialize);
  } else {
    iterator = NewIterator<MaterializeIterator>(
        thd, unit->release_root_iterator(), tmp_table_param, qep_tab->table(),
        move(qep_tab->iterator), qep_tab->table_ref->common_table_expr(),
        select_number, unit, /*subjoin=*/nullptr,
        /*ref_slice=*/-1, copy_fields_and_items_in_materialize,
        qep_tab->rematerialize, tmp_table_param->end_write_records);
    materialize = down_cast<MaterializeIterator *>(iterator->real_iterator());
  }

  if (!qep_tab->rematerialize) {
    if (qep_tab->invalidators != nullptr) {
      for (const CacheInvalidatorIterator *invalidator :
           *qep_tab->invalidators) {
        materialize->AddInvalidator(invalidator);
      }
    }
  }

  return iterator;
}

/**
  Get the RowIterator used for scanning the given table, with any required
  materialization operations done first.
 */
unique_ptr_destroy_only<RowIterator> GetTableIterator(THD *thd,
                                                      QEP_TAB *qep_tab,
                                                      QEP_TAB *qep_tabs) {
  unique_ptr_destroy_only<RowIterator> table_iterator;
  if (qep_tab->materialize_table == QEP_TAB::MATERIALIZE_DERIVED) {
    table_iterator = GetIteratorForDerivedTable(thd, qep_tab);
  } else if (qep_tab->materialize_table ==
             QEP_TAB::MATERIALIZE_TABLE_FUNCTION) {
    table_iterator = NewIterator<MaterializedTableFunctionIterator>(
        thd, qep_tab->table_ref->table_function, qep_tab->table(),
        move(qep_tab->iterator));
  } else if (qep_tab->materialize_table == QEP_TAB::MATERIALIZE_SEMIJOIN) {
    Semijoin_mat_exec *sjm = qep_tab->sj_mat_exec();

    // create_tmp_table() has already filled sjm->table_param.items_to_copy.
    // However, the structures there are not used by
    // join_materialize_semijoin, and don't have e.g. result fields set up
    // correctly, so we just clear it and create our own.
    sjm->table_param.items_to_copy = nullptr;
    ConvertItemsToCopy(&sjm->sj_nest->nested_join->sj_inner_exprs,
                       qep_tab->table()->visible_field_ptr(), &sjm->table_param,
                       qep_tab->join());

    int join_start = sjm->inner_table_index;
    int join_end = join_start + sjm->table_count;

    // Handle this subquery as a we would a completely separate join,
    // even though the tables are part of the same JOIN object
    // (so in effect, a “virtual join”).
    qep_tab_map unhandled_duplicates = 0;
    table_map conditions_depend_on_outer_tables = 0;
    unique_ptr_destroy_only<RowIterator> subtree_iterator = ConnectJoins(
        /*upper_first_idx=*/NO_PLAN_IDX, join_start, join_end, qep_tabs, thd,
        TOP_LEVEL,
        /*pending_conditions=*/nullptr,
        /*pending_invalidators=*/nullptr,
        /*pending_join_conditions=*/nullptr, &unhandled_duplicates,
        &conditions_depend_on_outer_tables);

    // If there were any weedouts that we had to drop during ConnectJoins()
    // (ie., the join left some tables that were supposed to be deduplicated
    // but were not), handle them now at the end of the virtual join.
    if (unhandled_duplicates != 0) {
      subtree_iterator = CreateWeedoutIteratorForTables(
          thd, unhandled_duplicates, qep_tab, qep_tab->join()->primary_tables,
          move(subtree_iterator));
    }

    // Since materialized semijoins are based on ref access against the table,
    // and ref access has NULL = NULL (while IN expressions should not),
    // remove rows with NULLs in them here. This is only an optimization for IN
    // (since equality propagation will filter away NULLs on the other side),
    // but is required for NOT IN correctness.
    //
    // TODO: It could be possible to join this with an existing condition,
    // and possibly also in some cases when scanning each table.
    vector<Item *> not_null_conditions;
    for (Item &item : sjm->sj_nest->nested_join->sj_inner_exprs) {
      if (item.maybe_null) {
        Item *condition = new Item_func_isnotnull(&item);
        condition->quick_fix_field();
        condition->update_used_tables();
        condition->apply_is_true();
        not_null_conditions.push_back(condition);
      }
    }
    subtree_iterator = PossiblyAttachFilterIterator(
        move(subtree_iterator), not_null_conditions, thd,
        &conditions_depend_on_outer_tables);

    bool copy_fields_and_items_in_materialize =
        true;  // We never have aggregation within semijoins.
    table_iterator = NewIterator<MaterializeIterator>(
        thd, move(subtree_iterator), &sjm->table_param, qep_tab->table(),
        move(qep_tab->iterator), /*cte=*/nullptr,
        qep_tab->join()->select_lex->select_number, /*unit=*/nullptr,
        qep_tab->join(),
        /*ref_slice=*/-1, copy_fields_and_items_in_materialize,
        qep_tab->rematerialize, sjm->table_param.end_write_records);

#ifndef DBUG_OFF
    // Make sure we clear this table out when the join is reset,
    // since its contents may depend on outer expressions.
    bool found = false;
    for (TABLE &sj_tmp_tab : qep_tab->join()->sj_tmp_tables) {
      if (&sj_tmp_tab == qep_tab->table()) {
        found = true;
        break;
      }
    }
    DBUG_ASSERT(found);
#endif
  } else {
    table_iterator = move(qep_tab->iterator);

    POSITION *pos = qep_tab->position();
    if (pos != nullptr) {
      SetCostOnTableIterator(*thd->cost_model(), pos, /*is_after_filter=*/false,
                             table_iterator.get());
    }

    // See if this is an information schema table that must be filled in before
    // we scan.
    if (qep_tab->table_ref->schema_table &&
        qep_tab->table_ref->schema_table->fill_table) {
      table_iterator.reset(new (thd->mem_root)
                               MaterializeInformationSchemaTableIterator(
                                   thd, qep_tab, move(table_iterator)));
    }
  }
  return table_iterator;
}

void SetCostOnTableIterator(const Cost_model_server &cost_model,
                            const POSITION *pos, bool is_after_filter,
                            RowIterator *iterator) {
  double num_rows_after_filtering = pos->rows_fetched * pos->filter_effect;
  if (is_after_filter) {
    iterator->set_expected_rows(num_rows_after_filtering);
  } else {
    iterator->set_expected_rows(pos->rows_fetched);
  }

  // Note that we don't try to adjust for the filtering here;
  // we estimate the same cost as the table itself.
  double cost =
      pos->read_cost + cost_model.row_evaluate_cost(num_rows_after_filtering);
  if (pos->prefix_rowcount <= 0.0) {
    iterator->set_estimated_cost(cost);
  } else {
    // Scale the estimated cost to being for one loop only, to match the
    // measured costs.
    iterator->set_estimated_cost(cost * num_rows_after_filtering /
                                 pos->prefix_rowcount);
  }
}

void SetCostOnNestedLoopIterator(const Cost_model_server &cost_model,
                                 const POSITION *pos_right,
                                 RowIterator *iterator) {
  if (pos_right == nullptr) {
    // No cost information.
    return;
  }

  DBUG_ASSERT(iterator->children().size() == 2);
  RowIterator *left = iterator->children()[0].iterator;
  RowIterator *right = iterator->children()[1].iterator;

  if (left->expected_rows() == -1.0 || right->expected_rows() == -1.0) {
    // Missing cost information on at least one child.
    return;
  }

  // Mirrors set_prefix_join_cost(), even though the cost calculation doesn't
  // make a lot of sense.
  double right_expected_rows_before_filter =
      pos_right->filter_effect > 0.0
          ? (right->expected_rows() / pos_right->filter_effect)
          : 0.0;
  double joined_rows =
      left->expected_rows() * right_expected_rows_before_filter;
  iterator->set_expected_rows(joined_rows * pos_right->filter_effect);
  iterator->set_estimated_cost(left->estimated_cost() + pos_right->read_cost +
                               cost_model.row_evaluate_cost(joined_rows));
}

void SetCostOnHashJoinIterator(const Cost_model_server &cost_model,
                               const POSITION *pos_right,
                               RowIterator *iterator) {
  if (pos_right == nullptr) {
    // No cost information.
    return;
  }

  DBUG_ASSERT(iterator->children().size() == 2);
  RowIterator *left = iterator->children()[1].iterator;
  RowIterator *right = iterator->children()[0].iterator;

  if (left->expected_rows() == -1.0 || right->expected_rows() == -1.0) {
    // Missing cost information on at least one child.
    return;
  }

  // Mirrors set_prefix_join_cost(), even though the cost calculation doesn't
  // make a lot of sense.
  double joined_rows = left->expected_rows() * right->expected_rows();
  iterator->set_expected_rows(joined_rows * pos_right->filter_effect);
  iterator->set_estimated_cost(left->estimated_cost() + pos_right->read_cost +
                               cost_model.row_evaluate_cost(joined_rows));
}

static bool ConditionIsAlwaysTrue(Item *item) {
  return item->const_item() && item->val_bool();
}

// Returns true if the item refers to only one side of the join. This is used to
// determine whether an equi-join conditions need to be attached as an "extra"
// condition (pure join conditions must refer to both sides of the join).
static bool ItemRefersToOneSideOnly(Item *item, table_map left_side,
                                    table_map right_side) {
  item->update_used_tables();
  const table_map item_used_tables = item->used_tables();

  if ((left_side & item_used_tables) == 0 ||
      (right_side & item_used_tables) == 0) {
    return true;
  }
  return false;
}

// Create a hash join iterator with the given build and probe input. We will
// move conditions from the argument "join_conditions" into two separate lists;
// one list for equi-join conditions that will be used as normal join conditions
// in hash join, and one list for non-equi-join conditions that will be attached
// as "extra" conditions in hash join. The "extra" conditions are conditions
// that must be evaluated after the hash table lookup, but _before_ returning a
// row. Conditions that are not moved will be attached as filters after the
// join. Note that we only attach conditions as "extra" conditions if the join
// type is not inner join. This gives us more fine-grained output from EXPLAIN
// ANALYZE, where we can see whether the condition was expensive.
// This information is lost when we attach conditions as extra conditions inside
// hash join.
//
// The function will also determine whether hash join is allowed to spill to
// disk. In general, we reject spill to disk if the query has a LIMIT and no
// aggregation or grouping. See comments inside the function for justification.
static unique_ptr_destroy_only<RowIterator> CreateHashJoinIterator(
    THD *thd, QEP_TAB *qep_tab,
    unique_ptr_destroy_only<RowIterator> build_iterator,
    qep_tab_map build_tables,
    unique_ptr_destroy_only<RowIterator> probe_iterator,
    qep_tab_map probe_tables, JoinType join_type,
    vector<Item *> *join_conditions,
    table_map *conditions_depend_on_outer_tables) {
  table_map left_table_map =
      ConvertQepTabMapToTableMap(qep_tab->join(), probe_tables);
  table_map right_table_map =
      ConvertQepTabMapToTableMap(qep_tab->join(), build_tables);

  // Move out equi-join conditions and non-equi-join conditions, so we can
  // attach them as join condition and extra conditions in hash join.
  vector<HashJoinCondition> hash_join_conditions;
  vector<Item *> hash_join_extra_conditions;

  for (Item *outer_item : *join_conditions) {
    // We can encounter conditions that are AND'ed together (i.e. a condition
    // that originally was Item_cond_and inside a Item_trig_cond).
    vector<Item *> condition_parts;
    ExtractConditions(outer_item, &condition_parts);
    for (Item *inner_item : condition_parts) {
      if (ConditionIsAlwaysTrue(inner_item)) {
        // The optimizer may leave conditions that are always 'true'. These have
        // no effect on the query, so we ignore them. Ideally, the optimizer
        // should not attach these conditions in the first place.
        continue;
      }

      // See if this is an equi-join condition.
      if (inner_item->type() == Item::FUNC_ITEM ||
          inner_item->type() == Item::COND_ITEM) {
        Item_func *func_item = down_cast<Item_func *>(inner_item);

        if (func_item->functype() == Item_func::EQ_FUNC) {
          down_cast<Item_func_eq *>(func_item)
              ->ensure_multi_equality_fields_are_available(left_table_map,
                                                           right_table_map);
        }

        if (func_item->contains_only_equi_join_condition() &&
            !ItemRefersToOneSideOnly(func_item, left_table_map,
                                     right_table_map)) {
          // Make a hash join condition for this equality comparison.
          // This may entail allocating type cast nodes; see the comments
          // on HashJoinCondition for more details.
          hash_join_conditions.emplace_back(
              down_cast<Item_func_eq *>(func_item), thd->mem_root);
          continue;
        }
      }
      // It was not.
      hash_join_extra_conditions.push_back(inner_item);
    }
  }

  // For any conditions for which HashJoinCondition decided only to store the
  // hash in the key, we need to re-check.
  for (const HashJoinCondition &cond : hash_join_conditions) {
    if (!cond.store_full_sort_key()) {
      hash_join_extra_conditions.push_back(cond.join_condition());
    }
  }

  if (join_type == JoinType::INNER) {
    // For inner join, attach the extra conditions as filters after the join.
    // This gives us more detailed output in EXPLAIN ANALYZE since we get an
    // instrumented FilterIterator on top of the join.
    *join_conditions = move(hash_join_extra_conditions);
  } else {
    join_conditions->clear();

    // The join condition could contain conditions that can be pushed down into
    // the right side, e.g. “t1 LEFT JOIN t2 ON t2.x > 3” (or simply
    // “ON FALSE”). For inner joins, the optimizer will have pushed these down
    // to the right tables, but it is not capable of doing so for outer joins.
    // As a band-aid, we identify these and push them down onto the build
    // iterator. This isn't ideal (they will not e.g. give rise to index
    // lookups, and if there are multiple tables, we don't push the condition
    // as far down as we should), but it should give reasonable speedups for
    // many common cases.
    vector<Item *> build_conditions;
    for (auto cond_it = hash_join_extra_conditions.begin();
         cond_it != hash_join_extra_conditions.end();) {
      Item *cond = *cond_it;
      if ((cond->used_tables() & (left_table_map | RAND_TABLE_BIT)) == 0) {
        build_conditions.push_back(cond);
        cond_it = hash_join_extra_conditions.erase(cond_it);
      } else {
        *conditions_depend_on_outer_tables |= cond->used_tables();
        ++cond_it;
      }
    }
    build_iterator =
        PossiblyAttachFilterIterator(move(build_iterator), build_conditions,
                                     thd, conditions_depend_on_outer_tables);
  }

  const JOIN *join = qep_tab->join();
  const bool has_grouping = join->implicit_grouping || join->grouped;

  const bool has_limit = join->m_select_limit != HA_POS_ERROR;

  const bool has_order_by = join->order.order != nullptr;

  // If we have a limit in the query, do not allow hash join to spill to
  // disk. The effect of this is that hash join will start producing
  // result rows a lot earlier, and thus hit the LIMIT a lot sooner.
  // Ideally, this should be decided during optimization.
  // There are however two situations where we always allow spill to disk,
  // and that is if we either have grouping or sorting in the query. In
  // those cases, the iterator above us will most likely consume the
  // entire result set anyways.
  bool allow_spill_to_disk = !has_limit || has_grouping || has_order_by;

  // If this table is part of a pushed join query, rows from the dependant child
  // table(s) has to be read while we are positioned on the rows from the pushed
  // ancestors which the child depends on. Thus, we can not allow rows from a
  // 'pushed join' to 'spill_to_disk'.
  if (qep_tab->table()->file->member_of_pushed_join()) {
    allow_spill_to_disk = false;
  }

  auto iterator = NewIterator<HashJoinIterator>(
      thd, move(build_iterator), build_tables, move(probe_iterator),
      probe_tables, thd->variables.join_buff_size, hash_join_conditions,
      allow_spill_to_disk, join_type, join, hash_join_extra_conditions);
  SetCostOnHashJoinIterator(*thd->cost_model(), qep_tab->position(),
                            iterator.get());

  return iterator;
}

// Move all the join conditions from the vector "predicates" over to the
// vector "join_conditions", while filters are untouched. This is done so that
// we can attach the join conditions directly to the hash join iterator. Further
// separation into equi-join and non-equi-join conditions will be done inside
// CreateHashJoinIterator().
static void ExtractJoinConditions(const QEP_TAB *current_table,
                                  vector<Item *> *predicates,
                                  vector<Item *> *join_conditions) {
  vector<Item *> real_predicates;
  for (Item *item : *predicates) {
    if (IsJoinCondition(item, current_table)) {
      join_conditions->emplace_back(item);
    } else {
      real_predicates.emplace_back(item);
    }
  }

  *predicates = move(real_predicates);
}

// See if a given subtree contains a pushed join that are self-contained within
// the subtree. Consider the following execution tree:
//
//       +--join 1--+
//       |          |
//  +--join 2--+    t3
//  |          |
//  t1         t2
//
// If there is a pushed join between t2 and t3, this function will return
// 'false' for both sides of 'join 1' as the pushed join is a part of multiple
// subtrees.
static bool SubtreeHasIncompletePushedJoin(JOIN *join, qep_tab_map subtree) {
  const table_map subtree_table_map = ConvertQepTabMapToTableMap(join, subtree);
  for (QEP_TAB *qep_tab : TablesContainedIn(join, subtree)) {
    handler *handler = qep_tab->table()->file;
    table_map tables_in_pushed_join = handler->tables_in_pushed_join();

    // See if any of the tables in the pushed join does not belong to the given
    // subtree.
    if (tables_in_pushed_join & ~subtree_table_map) {
      return true;
    }
  }

  return false;
}

// Given a pushed join between t1 and t2 where t1 is the root of the pushed
// join, reading a row from t1 causes NDB to do a join against t2 so that next
// read from t2 will give back the matching row(s). This means that one read
// from t1 must be followed by read from t2 until EOF. In other words, joins
// must be executed using nested loop for pushed joins to work correctly. With
// hash join, this pattern is broken; both inputs may be written out to disk,
// causing multiple reads from one subtree before doing any reads from the other
// subtree. This means that if one side of the hash join contains a pushed join
// with tables outside of said side, hash join cannot be used.
//
// Note that if we force _inner_ hash joins to not spill to disk, the right side
// (the probe input) of the hash join will not be materialized, causing it to
// resemble a block nested loop. So if the join is a inner join, hash join can
// be used as long as we do not spill to disk _and_ the left side (the build
// input) does not contain an incomplete pushed join. This is not true for
// semi/anti/outer hash join, as the right side is the _build_ input for these
// join types.
static bool PushedJoinRejectsHashJoin(JOIN *join, qep_tab_map left_subtree,
                                      qep_tab_map right_subtree,
                                      JoinType join_type) {
  if (join_type == JoinType::INNER) {
    // Inner hash join works fine with pushed joins as long as we ensure that we
    // do not spill to disk, _and_ the left subtree (the build input) does not
    // have an incomplete pushed join.
    return SubtreeHasIncompletePushedJoin(join, left_subtree);
  }

  return SubtreeHasIncompletePushedJoin(join, left_subtree) ||
         SubtreeHasIncompletePushedJoin(join, right_subtree);
}

static bool UseHashJoin(QEP_TAB *qep_tab) {
  return qep_tab->op_type == QEP_TAB::OT_BNL;
}

static bool UseBKA(QEP_TAB *qep_tab) {
  if (qep_tab->op_type != QEP_TAB::OT_BKA) {
    // Not BKA.
    return false;
  }

  // Similar to QueryMixesOuterBKAAndBNL(), if we have an outer join BKA
  // that contains multiple tables on the right side, we will not have a
  // left-deep tree, which we cannot handle at this point.
  if (qep_tab->last_inner() != NO_PLAN_IDX &&
      qep_tab->last_inner() != qep_tab->idx()) {
    // More than one table on the right side of an outer join, so not
    // left-deep.
    return false;
  }
  return true;
}

// Having a non-BKA join on the right side of an outer BKA join causes problems
// for the matched-row signaling from MultiRangeRowIterator to BKAIterator;
// rows could be found just fine, but not go through the join filter (and thus
// not be marked as matched in BKAIterator), creating extra NULLs.
//
// The only way this can happen is when we get a hash join on the inside of an
// outer BKA join (otherwise, the join tree will be left-deep). If this
// happens, we simply turn off both BKA and hash join handling for the query;
// it is a very rare situation, and the slowdown should be acceptable.
// (Only turning off BKA helps somewhat, but MultiRangeRowIterator also cannot
// be on the inside of a hash join, so we need to turn off BNL as well.)
static bool QueryMixesOuterBKAAndBNL(JOIN *join) {
  bool has_outer_bka = false;
  bool has_bnl = false;
  for (uint i = join->const_tables; i < join->primary_tables; ++i) {
    QEP_TAB *qep_tab = &join->qep_tab[i];
    if (UseHashJoin(qep_tab)) {
      has_bnl = true;
    } else if (qep_tab->op_type == QEP_TAB::OT_BKA &&
               qep_tab->last_inner() != NO_PLAN_IDX) {
      has_outer_bka = true;
    }
  }
  return has_bnl && has_outer_bka;
}

static bool InsideOuterOrAntiJoin(QEP_TAB *qep_tab) {
  return qep_tab->last_inner() != NO_PLAN_IDX;
}

template <class T>
void PickOutConditionsForTableIndex(int table_idx, vector<T> *from,
                                    vector<T> *to) {
  for (auto it = from->begin(); it != from->end();) {
    if (it->table_index_to_attach_to == table_idx) {
      to->push_back(*it);
      it = from->erase(it);
    } else {
      ++it;
    }
  }
}

void PickOutConditionsForTableIndex(int table_idx,
                                    vector<PendingCondition> *from,
                                    vector<Item *> *to) {
  for (auto it = from->begin(); it != from->end();) {
    if (it->table_index_to_attach_to == table_idx) {
      to->push_back(it->cond);
      it = from->erase(it);
    } else {
      ++it;
    }
  }
}

unique_ptr_destroy_only<RowIterator> FinishPendingOperations(
    THD *thd, unique_ptr_destroy_only<RowIterator> iterator,
    QEP_TAB *remove_duplicates_loose_scan_qep_tab,
    const vector<PendingCondition> &pending_conditions,
    const vector<PendingInvalidator> &pending_invalidators,
    table_map *conditions_depend_on_outer_tables) {
  iterator =
      PossiblyAttachFilterIterator(move(iterator), pending_conditions, thd,
                                   conditions_depend_on_outer_tables);

  if (remove_duplicates_loose_scan_qep_tab != nullptr) {
    QEP_TAB *const qep_tab =
        remove_duplicates_loose_scan_qep_tab;  // For short.
    KEY *key = qep_tab->table()->key_info + qep_tab->index();
    iterator = NewIterator<RemoveDuplicatesIterator>(
        thd, move(iterator), qep_tab->table(), key, qep_tab->loosescan_key_len);
  }

  // It's highly unlikely that we have more than one pending QEP_TAB here
  // (the most common case will be zero), so don't bother combining them
  // into one invalidator.
  for (const PendingInvalidator &invalidator : pending_invalidators) {
    iterator =
        CreateInvalidatorIterator(thd, invalidator.qep_tab, move(iterator));
  }

  return iterator;
}

/**
  For a given slice of the table list, build up the iterator tree corresponding
  to the tables in that slice. It handles inner and outer joins, as well as
  semijoins (“first match”).

  The join tree in MySQL is generally a left-deep tree of inner joins,
  so we can start at the left, make an inner join against the next table,
  join the result of that against the next table, etc.. However, a given
  sub-slice of the table list can be designated as an outer join, by setting
  first_inner() and last_inner() on the first table of said slice. (It is also
  set in some, but not all, of the other tables in the slice.) If so, we call
  ourselves recursively with that slice, put it as the right (inner) arm of
  an outer join, and then continue with our inner join.

  Similarly, if a table N has set “first match” to table M (ie., jump back to
  table M whenever we see a non-filtered record in table N), then there is a
  subslice from [M+1,N] that we need to process recursively before putting it
  as the right side of a semijoin. Every semijoin can be implemented with a
  LIMIT 1, but for clarity and performance, we prefer to use a NestedLoopJoin
  with a special SEMI join type whenever possible. Sometimes, we have no choice,
  though (see the comments below). Note that we cannot use first_sj_inner() for
  detecting semijoins, as it is not updated when tables are reordered by the
  join optimizer. Outer joins and semijoins can nest, so we need to take some
  care to make sure that we pick the outermost structure to recurse on.

  Conditions are a bit tricky. Conceptually, SQL evaluates conditions only
  after all tables have been joined; however, for efficiency reasons, we want
  to evaluate them as early as possible. As long as we are only dealing with
  inner joins, this is as soon as we've read all tables participating in the
  condition, but for outer joins, we need to wait until the join has happened.
  See pending_conditions below.

  @param upper_first_idx gives us the first table index of the other side of the
    join. Only valid if we are inside a substructure (outer join, semijoin or
    antijoin). I.e., if we are processing the right side of the query
    't1 LEFT JOIN t2', upper_first_idx gives us the table index of 't1'. Used by
    hash join to determine the table map for each side of the join.
  @param first_idx index of the first table in the slice we are creating a
    tree for (inclusive)
  @param last_idx index of the last table in the slice we are creating a
    tree for (exclusive)
  @param qep_tabs the full list of tables we are joining
  @param thd the THD to allocate the iterators on
  @param calling_context what situation we have immediately around is in the
    tree (ie., whether we are called to resolve the inner part of an outer
    join, a semijoin, etc.); mostly used to avoid infinite recursion where we
    would process e.g. the same semijoin over and over again
  @param pending_conditions if nullptr, we are not at the right (inner) side of
    any outer join and can evaluate conditions immediately. If not, we need to
    push any WHERE predicates to that vector and evaluate them only after joins.
  @param pending_invalidators similar to pending_conditions, but for tables
    that should have a CacheInvalidatorIterator synthesized for them;
    NULL-complemented rows must also invalidate materialized lateral derived
    tables.
  @param pending_join_conditions if not nullptr, we are at the inner side of
    semijoin/antijoin. The join iterator is created at the outer side, so any
    join conditions at the inner side needs to be pushed to this vector so that
    they can be attached to the join iterator. Note that this is currently only
    used by hash join.
  @param[out] unhandled_duplicates list of tables we should have deduplicated
    using duplicate weedout, but could not; append-only.
  @param[out] conditions_depend_on_outer_tables For each condition we have
    applied on the inside of these iterators, their dependent tables are
    appended to this set. Thus, if conditions_depend_on_outer_tables contain
    something from outside the tables covered by [first_idx,last_idx)
    (ie., after translation from QEP_TAB indexes to table indexes), we cannot
    use a hash join, since the returned iterator depends on seeing outer rows
    when evaluating its conditions.
 */
static unique_ptr_destroy_only<RowIterator> ConnectJoins(
    plan_idx upper_first_idx, plan_idx first_idx, plan_idx last_idx,
    QEP_TAB *qep_tabs, THD *thd, CallingContext calling_context,
    vector<PendingCondition> *pending_conditions,
    vector<PendingInvalidator> *pending_invalidators,
    vector<PendingCondition> *pending_join_conditions,
    qep_tab_map *unhandled_duplicates,
    table_map *conditions_depend_on_outer_tables) {
  DBUG_ASSERT(last_idx > first_idx);
  DBUG_ASSERT((pending_conditions == nullptr) ==
              (pending_invalidators == nullptr));
  unique_ptr_destroy_only<RowIterator> iterator = nullptr;

  // A special case: If we are at the top but the first table is an outer
  // join, we implicitly have one or more const tables to the left side
  // of said join.
  bool is_top_level_outer_join =
      calling_context == TOP_LEVEL &&
      qep_tabs[first_idx].last_inner() != NO_PLAN_IDX;

  vector<PendingCondition> top_level_pending_conditions;
  vector<PendingInvalidator> top_level_pending_invalidators;
  vector<PendingCondition> top_level_pending_join_conditions;
  if (is_top_level_outer_join) {
    iterator =
        NewIterator<FakeSingleRowIterator>(thd, /*examined_rows=*/nullptr);
    pending_conditions = &top_level_pending_conditions;
    pending_invalidators = &top_level_pending_invalidators;
    pending_join_conditions = &top_level_pending_join_conditions;
  }

  // NOTE: i is advanced in one of two ways:
  //
  //  - If we have an inner join, it will be incremented near the bottom of the
  //    loop, as we can process inner join tables one by one.
  //  - If not (ie., we have an outer join or semijoin), we will process
  //    the sub-join recursively, and thus move it past the end of said
  //    sub-join.
  for (plan_idx i = first_idx; i < last_idx;) {
    if (is_top_level_outer_join && i == qep_tabs[first_idx].last_inner() + 1) {
      // Finished the top level outer join.
      iterator = FinishPendingOperations(
          thd, move(iterator), /*remove_duplicates_loose_scan_qep_tab=*/nullptr,
          top_level_pending_conditions, top_level_pending_invalidators,
          conditions_depend_on_outer_tables);

      is_top_level_outer_join = false;
      pending_conditions = nullptr;
      pending_invalidators = nullptr;
      pending_join_conditions = nullptr;
    }

    bool add_limit_1;
    plan_idx substructure_end;
    Substructure substructure =
        FindSubstructure(qep_tabs, first_idx, i, last_idx, calling_context,
                         &add_limit_1, &substructure_end, unhandled_duplicates);

    QEP_TAB *qep_tab = &qep_tabs[i];
    if (substructure == Substructure::OUTER_JOIN ||
        substructure == Substructure::SEMIJOIN) {
      qep_tab_map left_tables = TablesBetween(first_idx, i);
      qep_tab_map right_tables = TablesBetween(i, substructure_end);

      // Outer or semijoin, consisting of a subtree (possibly of only one
      // table), so we send the entire subtree down to a recursive invocation
      // and then join the returned root into our existing tree.
      unique_ptr_destroy_only<RowIterator> subtree_iterator;
      vector<PendingCondition> subtree_pending_conditions;
      vector<PendingInvalidator> subtree_pending_invalidators;
      vector<PendingCondition> subtree_pending_join_conditions;
      table_map conditions_depend_on_outer_tables_subtree = 0;
      if (substructure == Substructure::SEMIJOIN) {
        // Semijoins don't have special handling of WHERE, so simply recurse.
        if (UseHashJoin(qep_tab) &&
            !PushedJoinRejectsHashJoin(qep_tab->join(), left_tables,
                                       right_tables, JoinType::SEMI) &&
            !QueryMixesOuterBKAAndBNL(qep_tab->join())) {
          // We must move any join conditions inside the subtructure up to this
          // level so that they can be attached to the hash join iterator.
          subtree_iterator = ConnectJoins(
              first_idx, i, substructure_end, qep_tabs, thd,
              DIRECTLY_UNDER_SEMIJOIN, &subtree_pending_conditions,
              &subtree_pending_invalidators, &subtree_pending_join_conditions,
              unhandled_duplicates, &conditions_depend_on_outer_tables_subtree);
        } else {
          // Send in "subtree_pending_join_conditions", so that any semijoin
          // conditions are moved up to this level, where they will be attached
          // as conditions to the hash join iterator.
          subtree_iterator = ConnectJoins(
              first_idx, i, substructure_end, qep_tabs, thd,
              DIRECTLY_UNDER_SEMIJOIN, pending_conditions, pending_invalidators,
              &subtree_pending_join_conditions, unhandled_duplicates,
              &conditions_depend_on_outer_tables_subtree);
        }
      } else if (pending_conditions != nullptr) {
        // We are already on the right (inner) side of an outer join,
        // so we need to keep deferring WHERE predicates.
        subtree_iterator = ConnectJoins(
            first_idx, i, substructure_end, qep_tabs, thd,
            DIRECTLY_UNDER_OUTER_JOIN, pending_conditions, pending_invalidators,
            pending_join_conditions, unhandled_duplicates,
            &conditions_depend_on_outer_tables_subtree);

        // Pick out any conditions that should be directly above this join
        // (ie., the ON conditions for this specific join).
        PickOutConditionsForTableIndex(i, pending_conditions,
                                       &subtree_pending_conditions);

        // Similarly, for invalidators.
        PickOutConditionsForTableIndex(i, pending_invalidators,
                                       &subtree_pending_invalidators);

        // Similarly, for join conditions.
        if (pending_join_conditions != nullptr) {
          PickOutConditionsForTableIndex(i, pending_join_conditions,
                                         &subtree_pending_join_conditions);
        }
      } else {
        // We can check the WHERE predicates on this table right away
        // after the join (and similarly, set up invalidators).
        subtree_iterator = ConnectJoins(
            first_idx, i, substructure_end, qep_tabs, thd,
            DIRECTLY_UNDER_OUTER_JOIN, &subtree_pending_conditions,
            &subtree_pending_invalidators, &subtree_pending_join_conditions,
            unhandled_duplicates, &conditions_depend_on_outer_tables_subtree);
      }
      *conditions_depend_on_outer_tables |=
          conditions_depend_on_outer_tables_subtree;

      JoinType join_type;
      if (qep_tab->table()->reginfo.not_exists_optimize) {
        // Similar to the comment on SplitConditions (see case #3), we can only
        // enable antijoin optimizations if we are not already on the right
        // (inner) side of another outer join. Otherwise, we would cause the
        // higher-up outer join to create NULL rows where there should be none.
        DBUG_ASSERT(substructure != Substructure::SEMIJOIN);
        join_type =
            (pending_conditions == nullptr) ? JoinType::ANTI : JoinType::OUTER;

        // Normally, a ”found” trigger means that the condition should be moved
        // up above some outer join (ie., it's a WHERE, not an ON condition).
        // However, there is one specific case where the optimizer sets up such
        // a trigger with the condition being _the same table as it's posted
        // on_, namely antijoins used for NOT IN; here, a FALSE condition is
        // being used to specify that inner rows should pass by the join, but
        // they should inhibit the null-complemented row. (So in this case,
        // the antijoin is no longer just an optimization that can be ignored
        // as we rewrite into an outer join.) In this case, there's a condition
        // wrapped in “not_null_compl” and ”found”, with the trigger for both
        // being the same table as the condition is posted on.
        //
        // So, as a special exception, detect this case, removing these
        // conditions (as they would otherwise kill all of our output rows) and
        // use them to mark the join as _really_ antijoin, even when it's
        // within an outer join.
        for (auto it = subtree_pending_conditions.begin();
             it != subtree_pending_conditions.end();) {
          if (it->table_index_to_attach_to == int(i) &&
              it->cond->item_name.ptr() == antijoin_null_cond) {
            DBUG_ASSERT(nullptr != dynamic_cast<Item_func_false *>(it->cond));
            join_type = JoinType::ANTI;
            it = subtree_pending_conditions.erase(it);
          } else {
            ++it;
          }
        }

        // Do the same for antijoin-marking conditions.
        for (auto it = subtree_pending_join_conditions.begin();
             it != subtree_pending_join_conditions.end();) {
          if (it->table_index_to_attach_to == int(i) &&
              it->cond->item_name.ptr() == antijoin_null_cond) {
            DBUG_ASSERT(nullptr != dynamic_cast<Item_func_false *>(it->cond));
            join_type = JoinType::ANTI;
            it = subtree_pending_join_conditions.erase(it);
          } else {
            ++it;
          }
        }
      } else {
        join_type = substructure == Substructure::SEMIJOIN ? JoinType::SEMI
                                                           : JoinType::OUTER;
      }

      // If the entire slice is a semijoin (e.g. because we are semijoined
      // against all the const tables, or because we're a semijoin within an
      // outer join), solve it by using LIMIT 1.
      //
      // If the entire slice is an outer join, we've solved that in a more
      // roundabout way; see is_top_level_outer_join above.
      if (iterator == nullptr) {
        DBUG_ASSERT(substructure == Substructure::SEMIJOIN);
        add_limit_1 = true;
      }

      if (add_limit_1) {
        subtree_iterator = NewIterator<LimitOffsetIterator>(
            thd, move(subtree_iterator), /*limit=*/1, /*offset=*/0,
            /*count_all_rows=*/false, /*skipped_rows=*/nullptr);
      }

      const bool pfs_batch_mode = qep_tab->pfs_batch_update(qep_tab->join()) &&
                                  join_type != JoinType::ANTI &&
                                  join_type != JoinType::SEMI;

      // See documentation for conditions_depend_on_outer_tables in
      // the function comment. Note that this cannot happen for inner joins
      // (join conditions can always be pulled up for them), so we do not
      // replicate this check for inner joins below.
      const bool right_side_depends_on_outer =
          Overlaps(conditions_depend_on_outer_tables_subtree,
                   ConvertQepTabMapToTableMap(qep_tab->join(), left_tables));

      bool remove_duplicates_loose_scan = false;
      if (i != first_idx && qep_tabs[i - 1].do_loosescan() &&
          qep_tabs[i - 1].match_tab != i - 1) {
        QEP_TAB *prev_qep_tab = &qep_tabs[i - 1];
        DBUG_ASSERT(iterator != nullptr);

        KEY *key = prev_qep_tab->table()->key_info + prev_qep_tab->index();
        if (substructure == Substructure::SEMIJOIN) {
          iterator =
              NewIterator<NestedLoopSemiJoinWithDuplicateRemovalIterator>(
                  thd, move(iterator), move(subtree_iterator),
                  prev_qep_tab->table(), key, prev_qep_tab->loosescan_key_len);
          SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                      iterator.get());
        } else {
          // We were originally in a semijoin, even if it didn't win in
          // FindSubstructure (LooseScan against multiple tables always puts
          // the non-first tables in FirstMatch), it was just overridden by
          // the outer join. In this case, we put duplicate removal after the
          // join (and any associated filtering), which is the safe option --
          // and in this case, it's no slower, since we'll be having a LIMIT 1
          // inserted anyway.
          DBUG_ASSERT(substructure == Substructure::OUTER_JOIN);
          remove_duplicates_loose_scan = true;

          iterator = NewIterator<NestedLoopIterator>(thd, move(iterator),
                                                     move(subtree_iterator),
                                                     join_type, pfs_batch_mode);
          SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                      iterator.get());
        }
      } else if (iterator == nullptr) {
        DBUG_ASSERT(substructure == Substructure::SEMIJOIN);
        iterator = move(subtree_iterator);
      } else if (((UseHashJoin(qep_tab) &&
                   !PushedJoinRejectsHashJoin(qep_tab->join(), left_tables,
                                              right_tables, join_type) &&
                   !right_side_depends_on_outer) ||
                  UseBKA(qep_tab)) &&
                 !QueryMixesOuterBKAAndBNL(qep_tab->join())) {
        // Join conditions that were inside the substructure are placed in the
        // vector 'subtree_pending_join_conditions'. Find out which of these
        // conditions that should be attached to this table, and attach them
        // to the hash join iterator.
        vector<Item *> join_conditions;
        PickOutConditionsForTableIndex(i, &subtree_pending_join_conditions,
                                       &join_conditions);

        if (UseBKA(qep_tab)) {
          iterator = CreateBKAIterator(thd, qep_tab->join(), move(iterator),
                                       left_tables, move(subtree_iterator),
                                       right_tables, qep_tab->table(),
                                       qep_tab->table_ref, &qep_tab->ref(),
                                       qep_tab->mrr_iterator, join_type);
        } else {
          iterator = CreateHashJoinIterator(
              thd, qep_tab, move(subtree_iterator), right_tables,
              move(iterator), left_tables, join_type, &join_conditions,
              conditions_depend_on_outer_tables);
        }

        iterator =
            PossiblyAttachFilterIterator(move(iterator), join_conditions, thd,
                                         conditions_depend_on_outer_tables);
      } else {
        // Normally, subtree_pending_join_conditions should be empty when we
        // create a nested loop iterator. However, in the case where we thought
        // we would be making a hash join but changed our minds (due to
        // right_side_depends_on_outer), there may be conditions there.
        // Similar to hash join above, pick out those conditions and add them
        // here.
        vector<Item *> join_conditions;
        PickOutConditionsForTableIndex(i, &subtree_pending_join_conditions,
                                       &join_conditions);
        subtree_iterator = PossiblyAttachFilterIterator(
            move(subtree_iterator), join_conditions, thd,
            conditions_depend_on_outer_tables);

        iterator = NewIterator<NestedLoopIterator>(thd, move(iterator),
                                                   move(subtree_iterator),
                                                   join_type, pfs_batch_mode);
        SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                    iterator.get());
      }

      QEP_TAB *remove_duplicates_loose_scan_qep_tab =
          remove_duplicates_loose_scan ? &qep_tabs[i - 1] : nullptr;
      iterator = FinishPendingOperations(
          thd, move(iterator), remove_duplicates_loose_scan_qep_tab,
          subtree_pending_conditions, subtree_pending_invalidators,
          conditions_depend_on_outer_tables);

      i = substructure_end;
      continue;
    } else if (substructure == Substructure::WEEDOUT) {
      unique_ptr_destroy_only<RowIterator> subtree_iterator = ConnectJoins(
          first_idx, i, substructure_end, qep_tabs, thd, DIRECTLY_UNDER_WEEDOUT,
          pending_conditions, pending_invalidators, pending_join_conditions,
          unhandled_duplicates, conditions_depend_on_outer_tables);
      RowIterator *child_iterator = subtree_iterator.get();
      subtree_iterator = CreateWeedoutIterator(thd, move(subtree_iterator),
                                               qep_tab->flush_weedout_table);

      // Copy costs (even though it makes no sense for the LIMIT 1 case).
      subtree_iterator->set_expected_rows(child_iterator->expected_rows());
      subtree_iterator->set_estimated_cost(child_iterator->estimated_cost());

      if (iterator == nullptr) {
        iterator = move(subtree_iterator);
      } else {
        iterator = NewIterator<NestedLoopIterator>(
            thd, move(iterator), move(subtree_iterator), JoinType::INNER,
            /*pfs_batch_mode=*/false);
        SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                    iterator.get());
      }

      i = substructure_end;
      continue;
    } else if (qep_tab->do_loosescan() && qep_tab->match_tab != i &&
               iterator != nullptr) {
      // Multi-table loose scan is generally handled by other parts of the code
      // (FindSubstructure() returns SEMIJOIN on the next table, since they will
      // have first match set), but we need to make sure there is only one table
      // on NestedLoopSemiJoinWithDuplicateRemovalIterator's left (outer) side.
      // Since we're not at the first table, we would be collecting a join
      // in “iterator” if we just kept on going, so we need to create a separate
      // tree by recursing here.
      unique_ptr_destroy_only<RowIterator> subtree_iterator = ConnectJoins(
          first_idx, i, qep_tab->match_tab + 1, qep_tabs, thd, TOP_LEVEL,
          pending_conditions, pending_invalidators, pending_join_conditions,
          unhandled_duplicates, conditions_depend_on_outer_tables);

      iterator = NewIterator<NestedLoopIterator>(
          thd, move(iterator), move(subtree_iterator), JoinType::INNER,
          /*pfs_batch_mode=*/false);
      SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                  iterator.get());
      i = qep_tab->match_tab + 1;
      continue;
    }

    unique_ptr_destroy_only<RowIterator> table_iterator =
        GetTableIterator(thd, qep_tab, qep_tabs);

    qep_tab_map right_tables = qep_tab->idx_map();
    qep_tab_map left_tables = 0;

    // Get the left side tables of this join.
    if (calling_context == DIRECTLY_UNDER_SEMIJOIN ||
        InsideOuterOrAntiJoin(qep_tab)) {
      // Join buffering (hash join, BKA) supports semijoin with only one inner
      // table (see setup_join_buffering), so the calling context for a
      // semijoin with join buffering will always be DIRECTLY_UNDER_SEMIJOIN.
      left_tables |= TablesBetween(upper_first_idx, first_idx);
    } else {
      left_tables |= TablesBetween(first_idx, i);
    }

    // If this is a BNL, we should replace it with hash join. We did decide
    // during create_iterators that we actually can replace the BNL with a hash
    // join, so we don't bother checking any further that we actually can
    // replace the BNL with a hash join.
    const bool replace_with_hash_join =
        UseHashJoin(qep_tab) && !QueryMixesOuterBKAAndBNL(qep_tab->join()) &&
        !PushedJoinRejectsHashJoin(qep_tab->join(), left_tables, right_tables,
                                   JoinType::INNER);

    vector<Item *> predicates_below_join;
    vector<Item *> join_conditions;
    vector<PendingCondition> predicates_above_join;

    // If we are on the inner side of a semi-/antijoin, pending_join_conditions
    // will be set. If the join should be executed using hash join,
    // SplitConditions() will put all join conditions in
    // pending_join_conditions. These conditions will later be attached to the
    // hash join iterator when we are done handling the inner side.
    SplitConditions(qep_tab->condition(), qep_tab, &predicates_below_join,
                    &predicates_above_join,
                    replace_with_hash_join ? pending_join_conditions : nullptr);

    // We can always do BKA. The setup is very similar to hash join.
    const bool is_bka =
        UseBKA(qep_tab) && !QueryMixesOuterBKAAndBNL(qep_tab->join());

    if (is_bka) {
      TABLE_REF &ref = qep_tab->ref();

      table_iterator = NewIterator<MultiRangeRowIterator>(
          thd, qep_tab->cache_idx_cond, qep_tab->table(),
          qep_tab->copy_current_rowid, &ref,
          qep_tab->position()->table->join_cache_flags);
      qep_tab->mrr_iterator =
          down_cast<MultiRangeRowIterator *>(table_iterator->real_iterator());

      if (qep_tab->cache_idx_cond != nullptr) {
        *conditions_depend_on_outer_tables |=
            qep_tab->cache_idx_cond->used_tables();
      }
      for (unsigned key_part_idx = 0; key_part_idx < ref.key_parts;
           ++key_part_idx) {
        *conditions_depend_on_outer_tables |=
            ref.items[key_part_idx]->used_tables();
      }
    } else if (replace_with_hash_join) {
      // We will now take all the join conditions (both equi- and
      // non-equi-join conditions) and move them to a separate vector so we
      // can attach them to the hash join iterator later. Conditions that
      // should be attached after the join remain in "predicates_below_join"
      // (i.e. filters).
      ExtractJoinConditions(qep_tab, &predicates_below_join, &join_conditions);
    }

    if (!qep_tab->condition_is_pushed_to_sort()) {  // See the comment on #2.
      double expected_rows = table_iterator->expected_rows();
      table_iterator = PossiblyAttachFilterIterator(
          move(table_iterator), predicates_below_join, thd,
          conditions_depend_on_outer_tables);
      POSITION *pos = qep_tab->position();
      if (expected_rows >= 0.0 && !predicates_below_join.empty() &&
          pos != nullptr) {
        SetCostOnTableIterator(*thd->cost_model(), pos,
                               /*is_after_filter=*/true, table_iterator.get());
      }
    } else {
      *conditions_depend_on_outer_tables |= qep_tab->condition()->used_tables();
    }

    // Handle LooseScan that hits this specific table only.
    // Multi-table LooseScans will be handled by
    // NestedLoopSemiJoinWithDuplicateRemovalIterator
    // (which is essentially a semijoin NestedLoopIterator and
    // RemoveDuplicatesIterator in one).
    if (qep_tab->do_loosescan() && qep_tab->match_tab == i) {
      KEY *key = qep_tab->table()->key_info + qep_tab->index();
      table_iterator = NewIterator<RemoveDuplicatesIterator>(
          thd, move(table_iterator), qep_tab->table(), key,
          qep_tab->loosescan_key_len);
    }

    if (qep_tab->lateral_derived_tables_depend_on_me) {
      if (pending_invalidators != nullptr) {
        pending_invalidators->push_back(
            PendingInvalidator{qep_tab, /*table_index_to_attach_to=*/i});
      } else {
        table_iterator =
            CreateInvalidatorIterator(thd, qep_tab, move(table_iterator));
      }
    }

    if (iterator == nullptr) {
      // We are the first table in this join.
      iterator = move(table_iterator);
    } else {
      // We can only enable DISTINCT optimizations if we are not in the right
      // (inner) side of an outer join; since the filter is deferred, the limit
      // would have to be, too. Similarly, we the old executor can do these
      // optimizations for multiple tables, but it requires poking into global
      // state to see if later tables produced rows or not; we restrict
      // ourselves to the rightmost table, instead of trying to make iterators
      // look at nonlocal state.
      //
      // We don't lose correctness by not applying the limit, only performance
      // on some fairly rare queries (for for former: DISTINCT queries where we
      // outer-join in a table that we don't use in the select list, but filter
      // on one of the columns; for the latter: queries with multiple unused
      // tables).
      //
      // Note that if we are to attach a hash join iterator, we cannot add this
      // optimization, as it would limit the probe input to only one row before
      // the join condition is even applied. Same with BKA; we need to buffer
      // the entire input, since we don't know if there's a match until the join
      // has actually happened.
      //
      // TODO: Consider pushing this limit up the tree together with the filter.
      // Note that this would require some trickery to reset the filter for
      // each new row on the left side of the join, so it's probably not worth
      // it.
      if (qep_tab->not_used_in_distinct && pending_conditions == nullptr &&
          i == static_cast<plan_idx>(qep_tab->join()->primary_tables - 1) &&
          !add_limit_1 && !replace_with_hash_join && !is_bka) {
        table_iterator = NewIterator<LimitOffsetIterator>(
            thd, move(table_iterator), /*limit=*/1, /*offset=*/0,
            /*count_all_rows=*/false, /*skipped_rows=*/nullptr);
      }

      // Inner join this table to the existing tree.
      // Inner joins are always left-deep, so we can just attach the tables as
      // we find them.
      DBUG_ASSERT(qep_tab->last_inner() == NO_PLAN_IDX);

      if (is_bka) {
        iterator = CreateBKAIterator(thd, qep_tab->join(), move(iterator),
                                     left_tables, move(table_iterator),
                                     right_tables, qep_tab->table(),
                                     qep_tab->table_ref, &qep_tab->ref(),
                                     qep_tab->mrr_iterator, JoinType::INNER);
      } else if (replace_with_hash_join) {
        // The numerically lower QEP_TAB is often (if not always) the smaller
        // input, so use that as the build input.
        iterator = CreateHashJoinIterator(
            thd, qep_tab, move(iterator), left_tables, move(table_iterator),
            right_tables, JoinType::INNER, &join_conditions,
            conditions_depend_on_outer_tables);

        // Attach any remaining non-equi-join conditions as a filter after the
        // join.
        iterator =
            PossiblyAttachFilterIterator(move(iterator), join_conditions, thd,
                                         conditions_depend_on_outer_tables);
      } else {
        iterator = CreateNestedLoopIterator(
            thd, move(iterator), move(table_iterator), JoinType::INNER,
            qep_tab->pfs_batch_update(qep_tab->join()));
        SetCostOnNestedLoopIterator(*thd->cost_model(), qep_tab->position(),
                                    iterator.get());
      }
    }
    ++i;

    // If we have any predicates that should be above an outer join,
    // send them upwards.
    for (PendingCondition &cond : predicates_above_join) {
      DBUG_ASSERT(pending_conditions != nullptr);
      pending_conditions->push_back(cond);
    }
  }
  if (is_top_level_outer_join) {
    // We can't have any invalidators here, because there's no later table
    // to invalidate.
    DBUG_ASSERT(top_level_pending_invalidators.empty());

    DBUG_ASSERT(last_idx == qep_tabs[first_idx].last_inner() + 1);
    iterator = FinishPendingOperations(
        thd, move(iterator), /*remove_duplicates_loose_scan_qep_tab=*/nullptr,
        top_level_pending_conditions, top_level_pending_invalidators,
        conditions_depend_on_outer_tables);
  }
  return iterator;
}

void JOIN::create_iterators() {
  DBUG_ASSERT(m_root_iterator == nullptr);

  // 1) Set up the basic RowIterators for accessing each specific table.
  create_table_iterators();

  // 2) Create the composite iterators combining the row from each table.
  unique_ptr_destroy_only<RowIterator> iterator =
      create_root_iterator_for_join();
  assert(iterator != nullptr);

  iterator = attach_iterators_for_having_and_limit(move(iterator));
  iterator->set_join_for_explain(this);
  m_root_iterator = move(iterator);
}

void JOIN::create_table_iterators() {
  for (unsigned table_idx = const_tables; table_idx < tables; ++table_idx) {
    QEP_TAB *qep_tab = &this->qep_tab[table_idx];
    if (qep_tab->position() == nullptr) {
      continue;
    }

    /*
      Create the specific RowIterators, including any specific
      RowIterator for the pushed queries.
    */
    qep_tab->pick_table_access_method();

    if (qep_tab->filesort) {
      unique_ptr_destroy_only<RowIterator> iterator = move(qep_tab->iterator);

      // Evaluate any conditions before sorting entire row set.
      if (qep_tab->condition()) {
        vector<Item *> predicates_below_join;
        vector<PendingCondition> predicates_above_join;
        SplitConditions(qep_tab->condition(), qep_tab, &predicates_below_join,
                        &predicates_above_join,
                        /*join_conditions=*/nullptr);

        table_map conditions_depend_on_outer_tables = 0;
        iterator = PossiblyAttachFilterIterator(
            move(iterator), predicates_below_join, thd,
            &conditions_depend_on_outer_tables);
        qep_tab->mark_condition_as_pushed_to_sort();
      }

      // Wrap the chosen RowIterator in a SortingIterator, so that we get
      // sorted results out.
      qep_tab->iterator = NewIterator<SortingIterator>(
          qep_tab->join()->thd, qep_tab, qep_tab->filesort, move(iterator),
          &qep_tab->join()->examined_rows);
      qep_tab->table()->sorting_iterator =
          down_cast<SortingIterator *>(qep_tab->iterator->real_iterator());
    }
  }
}

unique_ptr_destroy_only<RowIterator> JOIN::create_root_iterator_for_join() {
  if (select_count) {
    return unique_ptr_destroy_only<RowIterator>(
        new (thd->mem_root) UnqualifiedCountIterator(thd, this));
  }

  // OK, so we're good. Go through the tables and make the join iterators.
  unique_ptr_destroy_only<RowIterator> iterator;
  if (select_lex->is_table_value_constructor) {
    best_rowcount = select_lex->row_value_list->size();
    iterator = NewIterator<TableValueConstructorIterator>(
        thd, &examined_rows, *select_lex->row_value_list, fields);
  } else if (const_tables == primary_tables) {
    // Only const tables, so add a fake single row to join in all
    // the const tables (only inner-joined tables are promoted to
    // const tables in the optimizer).
    iterator = NewIterator<FakeSingleRowIterator>(thd, &examined_rows);
    qep_tab_map conditions_depend_on_outer_tables = 0;
    if (where_cond != nullptr) {
      iterator = PossiblyAttachFilterIterator(
          move(iterator), vector<Item *>{where_cond}, thd,
          &conditions_depend_on_outer_tables);
    }

    // Surprisingly enough, we can specify that the const tables are
    // to be dumped immediately to a temporary table. If we don't do this,
    // we risk that there are fields that are not copied correctly
    // (tmp_table_param contains copy_funcs we'd otherwise miss).
    if (const_tables > 0) {
      QEP_TAB *qep_tab = &this->qep_tab[const_tables];
      if (qep_tab->op_type == QEP_TAB::OT_MATERIALIZE) {
        qep_tab->iterator.reset();
        join_setup_iterator(qep_tab);
        qep_tab->table()->alias = "<temporary>";
        iterator = NewIterator<MaterializeIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, qep_tab->table(),
            move(qep_tab->iterator), /*cte=*/nullptr, select_lex->select_number,
            unit, this, qep_tab->ref_item_slice,
            /*copy_fields_and_items=*/true,
            /*rematerialize=*/true,
            qep_tab->tmp_table_param->end_write_records);
      }
    }
  } else {
    qep_tab_map unhandled_duplicates = 0;
    qep_tab_map conditions_depend_on_outer_tables = 0;
    iterator = ConnectJoins(
        /*upper_first_idx=*/NO_PLAN_IDX, const_tables, primary_tables, qep_tab,
        thd, TOP_LEVEL, nullptr, nullptr,
        /*pending_join_conditions=*/nullptr, &unhandled_duplicates,
        &conditions_depend_on_outer_tables);

    // If there were any weedouts that we had to drop during ConnectJoins()
    // (ie., the join left some tables that were supposed to be deduplicated
    // but were not), handle them now at the very end.
    if (unhandled_duplicates != 0) {
      iterator = CreateWeedoutIteratorForTables(
          thd, unhandled_duplicates, qep_tab, primary_tables, move(iterator));
    }
  }

  // Deal with any materialization happening at the end (typically for
  // sorting, grouping or distinct).
  for (unsigned table_idx = const_tables + 1; table_idx <= tables;
       ++table_idx) {
    QEP_TAB *qep_tab = &this->qep_tab[table_idx];
    if (qep_tab->op_type != QEP_TAB::OT_MATERIALIZE &&
        qep_tab->op_type != QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE &&
        qep_tab->op_type != QEP_TAB::OT_AGGREGATE_INTO_TMP_TABLE &&
        qep_tab->op_type != QEP_TAB::OT_WINDOWING_FUNCTION) {
      continue;
    }
    if (qep_tab->op_type == QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE) {
      // Aggregate as we go, with output into a temporary table.
      // (We can also aggregate as we go after the materialization step;
      // see below. We won't be aggregating twice, though.)
      if (qep_tab->tmp_table_param->precomputed_group_by) {
        DBUG_ASSERT(rollup.state == ROLLUP::STATE_NONE);
        iterator = NewIterator<PrecomputedAggregateIterator>(
            thd, move(iterator), this, qep_tab->tmp_table_param,
            qep_tab->ref_item_slice);
      } else {
        iterator = NewIterator<AggregateIterator>(
            thd, move(iterator), this, qep_tab->tmp_table_param,
            qep_tab->ref_item_slice, rollup.state != ROLLUP::STATE_NONE);
      }
    }

    // Attach HAVING if needed (it's put on the QEP_TAB and not on the JOIN if
    // we have a temporary table) and we've done all aggregation.
    //
    // FIXME: If the HAVING condition is an alias (a MySQL-specific extension),
    // it could be evaluated twice; once for the condition, and again for the
    // copying into the table. This was originally partially fixed by moving
    // the HAVING into qep_tab->condition() instead, although this makes the
    // temporary table larger than it needs to be, and is not a legal case in
    // the presence of SELECT DISTINCT. (The main.having test has a few tests
    // for this.) Later, it was completely fixed for the old executor,
    // by evaluating the filter against the temporary table row (switching
    // slices), although the conditional move into qep_tab->condition(),
    // which was obsolete for the old executor after said fix, was never
    // removed. See if we can get this fixed in the new executor as well,
    // and then remove the code that moves HAVING onto qep_tab->condition().
    if (qep_tab->having != nullptr &&
        qep_tab->op_type != QEP_TAB::OT_AGGREGATE_INTO_TMP_TABLE) {
      iterator =
          NewIterator<FilterIterator>(thd, move(iterator), qep_tab->having);
    }

    // Sorting comes after the materialization (which we're about to add),
    // and should be shown as such.
    Filesort *filesort = qep_tab->filesort;

    Filesort *dup_filesort = nullptr;
    bool limit_1_for_dup_filesort = false;

    // The pre-iterator executor did duplicate removal by going into the
    // temporary table and actually deleting records, using a hash table for
    // smaller tables and an O(n²) algorithm for large tables. This kind of
    // deletion is not cleanly representable in the iterator model, so we do it
    // using a duplicate-removing filesort instead, which has a straight-up
    // O(n log n) cost.
    if (qep_tab->needs_duplicate_removal) {
      bool all_order_fields_used;

      // If there's an ORDER BY on the query, it needs to be heeded in the
      // re-sort for DISTINCT. Note that the global ORDER BY could be pushed
      // to the first table, so we need to check there, too.
      ORDER *desired_order = this->order;
      if (desired_order == nullptr &&
          this->qep_tab[0].filesort_pushed_order != nullptr) {
        desired_order = this->qep_tab[0].filesort_pushed_order;
      }

      ORDER *order = create_order_from_distinct(
          thd, ref_items[qep_tab->ref_item_slice], desired_order, fields_list,
          /*skip_aggregates=*/false, /*convert_bit_fields_to_long=*/false,
          &all_order_fields_used);
      if (order == nullptr) {
        // Only const fields.
        limit_1_for_dup_filesort = true;
      } else {
        bool force_sort_positions = false;
        if (all_order_fields_used) {
          // The ordering for DISTINCT already gave us the right sort order,
          // so no need to sort again.
          filesort = nullptr;
        } else if (filesort != nullptr && !filesort->using_addon_fields()) {
          // We have the rather unusual situation here that we have two sorts
          // directly after each other, with no temporary table in-between,
          // and filesort expects to be able to refer to rows by their position.
          // Usually, the sort for DISTINCT would be a superset of the sort for
          // ORDER BY, but not always (e.g. when sorting by some expression),
          // so we could end up in a situation where the first sort is by addon
          // fields and the second one is by positions.
          //
          // Thus, in this case, we force the first sort to be by positions,
          // so that the result comes from SortFileIndirectIterator or
          // SortBufferIndirectIterator. These will both position the cursor
          // on the underlying temporary table correctly before returning it,
          // so that the successive filesort will save the right position
          // for the row.
          force_sort_positions = true;
        }

        // Switch to the right slice if applicable, so that we fetch out the
        // correct items from order_arg.
        Switch_ref_item_slice slice_switch(this, qep_tab->ref_item_slice);
        dup_filesort = new (thd->mem_root)
            Filesort(thd, qep_tab->table(), /*keep_buffers=*/false, order,
                     HA_POS_ERROR, /*force_stable_sort=*/false,
                     /*remove_duplicates=*/true, force_sort_positions);
      }
    }

    qep_tab->iterator.reset();
    join_setup_iterator(qep_tab);

    qep_tab->table()->alias = "<temporary>";

    if (qep_tab->op_type == QEP_TAB::OT_WINDOWING_FUNCTION) {
      if (qep_tab->tmp_table_param->m_window->needs_buffering()) {
        iterator = NewIterator<BufferingWindowingIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, this,
            qep_tab->ref_item_slice);
      } else {
        iterator = NewIterator<WindowingIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, this,
            qep_tab->ref_item_slice);
      }
      if (!qep_tab->tmp_table_param->m_window_short_circuit) {
        iterator = NewIterator<MaterializeIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, qep_tab->table(),
            move(qep_tab->iterator), /*cte=*/nullptr, select_lex->select_number,
            unit, this,
            /*ref_slice=*/-1, /*copy_fields_and_items_in_materialize=*/false,
            /*rematerialize=*/true, tmp_table_param.end_write_records);
      }
    } else if (qep_tab->op_type == QEP_TAB::OT_AGGREGATE_INTO_TMP_TABLE) {
      iterator = NewIterator<TemptableAggregateIterator>(
          thd, move(iterator), qep_tab->tmp_table_param, qep_tab->table(),
          move(qep_tab->iterator), select_lex, this, qep_tab->ref_item_slice);
      if (qep_tab->having != nullptr) {
        iterator =
            NewIterator<FilterIterator>(thd, move(iterator), qep_tab->having);
      }
    } else {
      DBUG_ASSERT(qep_tab->op_type == QEP_TAB::OT_MATERIALIZE ||
                  qep_tab->op_type == QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE);
      bool copy_fields_and_items =
          (qep_tab->op_type != QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE);

      // If we don't need the row IDs, and don't have some sort of deduplication
      // (e.g. for GROUP BY) on the table, filesort can take in the data
      // directly, without going through a temporary table.
      //
      // If there are two sorts, we need row IDs if either one of them needs it.
      // Above, we've set up so that the innermost sort (for DISTINCT) always
      // needs row IDs if the outermost (for ORDER BY) does. The other way is
      // fine, though; if the innermost needs row IDs but the outermost doesn't,
      // then we can use row IDs here (ie., no streaming) but drop them in the
      // outer sort. Thus, we check the using_addon_fields() flag on the
      // innermost.
      //
      // TODO: If the sort order is suitable (or extendable), we could take over
      // the deduplicating responsibilities of the temporary table and activate
      // this mode even if qep_tab->temporary_table_deduplicates() is set.
      Filesort *first_sort = dup_filesort != nullptr ? dup_filesort : filesort;
      if (first_sort != nullptr && first_sort->using_addon_fields() &&
          !qep_tab->temporary_table_deduplicates()) {
        iterator = NewIterator<StreamingIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, qep_tab->table(),
            copy_fields_and_items);
      } else {
        iterator = NewIterator<MaterializeIterator>(
            thd, move(iterator), qep_tab->tmp_table_param, qep_tab->table(),
            move(qep_tab->iterator), /*cte=*/nullptr, select_lex->select_number,
            unit, this, qep_tab->ref_item_slice, copy_fields_and_items,
            /*rematerialize=*/true,
            qep_tab->tmp_table_param->end_write_records);
      }
    }

    if (qep_tab->condition() != nullptr) {
      iterator = NewIterator<FilterIterator>(thd, move(iterator),
                                             qep_tab->condition());
      qep_tab->mark_condition_as_pushed_to_sort();
    }

    if (limit_1_for_dup_filesort) {
      iterator = NewIterator<LimitOffsetIterator>(
          thd, move(iterator), /*select_limit_cnt=*/1, /*offset_limit_cnt=*/0,
          /*count_all_rows=*/false, /*skipped_rows=*/nullptr);
    } else if (dup_filesort != nullptr) {
      iterator = NewIterator<SortingIterator>(thd, qep_tab, dup_filesort,
                                              move(iterator), &examined_rows);
      qep_tab->table()->duplicate_removal_iterator =
          down_cast<SortingIterator *>(iterator->real_iterator());
    }
    if (filesort != nullptr) {
      iterator = NewIterator<SortingIterator>(thd, qep_tab, filesort,
                                              move(iterator), &examined_rows);
      qep_tab->table()->sorting_iterator =
          down_cast<SortingIterator *>(iterator->real_iterator());
    }
  }

  // See if we need to aggregate data in the final step. Note that we can
  // _not_ rely on streaming_aggregation, as it can be changed from false
  // to true during optimization, and depending on when it was set, it could
  // either mean to aggregate into a temporary table or aggregate on final
  // send.
  bool do_aggregate;
  if (primary_tables == 0 && tmp_tables == 0) {
    // We can't check qep_tab since there's no table, but in this specific case,
    // it is safe to call get_end_select_func() at this point.
    do_aggregate = (get_end_select_func() == QEP_TAB::OT_AGGREGATE);
  } else {
    // Note that tmp_table_param.precomputed_group_by can be set even if we
    // don't actually have any grouping (e.g., make_tmp_tables_info() does this
    // even if there are no temporary tables made).
    do_aggregate = (qep_tab[primary_tables + tmp_tables].op_type ==
                    QEP_TAB::OT_AGGREGATE) ||
                   ((grouped || group_optimized_away) &&
                    tmp_table_param.precomputed_group_by);
  }
  if (do_aggregate) {
    // Aggregate as we go, with output into a special slice of the same table.
    DBUG_ASSERT(streaming_aggregation || tmp_table_param.precomputed_group_by);
#ifndef DBUG_OFF
    for (unsigned table_idx = const_tables; table_idx < tables; ++table_idx) {
      DBUG_ASSERT(qep_tab->op_type != QEP_TAB::OT_AGGREGATE_THEN_MATERIALIZE);
    }
#endif
    if (tmp_table_param.precomputed_group_by) {
      iterator = NewIterator<PrecomputedAggregateIterator>(
          thd, move(iterator), this, &tmp_table_param,
          REF_SLICE_ORDERED_GROUP_BY);
      DBUG_ASSERT(rollup.state == ROLLUP::STATE_NONE);
    } else {
      iterator = NewIterator<AggregateIterator>(
          thd, move(iterator), this, &tmp_table_param,
          REF_SLICE_ORDERED_GROUP_BY, rollup.state != ROLLUP::STATE_NONE);
    }
  }

  return iterator;
}

unique_ptr_destroy_only<RowIterator>
JOIN::attach_iterators_for_having_and_limit(
    unique_ptr_destroy_only<RowIterator> iterator) {
  // Attach HAVING and LIMIT if needed.
  // NOTE: We can have HAVING even without GROUP BY, although it's not very
  // useful.
  if (having_cond != nullptr) {
    iterator = NewIterator<FilterIterator>(thd, move(iterator), having_cond);
  }

  // Note: For select_count, LIMIT 0 is handled in JOIN::optimize() for the
  // common case, but not for CALC_FOUND_ROWS. OFFSET also isn't handled there.
  if (unit->select_limit_cnt != HA_POS_ERROR || unit->offset_limit_cnt != 0) {
    iterator = NewIterator<LimitOffsetIterator>(
        thd, move(iterator), unit->select_limit_cnt, unit->offset_limit_cnt,
        calc_found_rows, &send_records);
  }

  return iterator;
}

void JOIN::create_iterators_for_index_subquery() {
  create_table_iterators();

  QEP_TAB *first_qep_tab = &qep_tab[0];
  if (first_qep_tab->condition() != nullptr) {
    first_qep_tab->iterator = NewIterator<FilterIterator>(
        thd, move(first_qep_tab->iterator), first_qep_tab->condition());
  }

  TABLE_LIST *const tl = qep_tab->table_ref;
  if (tl && tl->uses_materialization()) {
    if (tl->is_table_function()) {
      m_root_iterator = NewIterator<MaterializedTableFunctionIterator>(
          thd, tl->table_function, first_qep_tab->table(),
          move(first_qep_tab->iterator));
    } else {
      m_root_iterator = GetIteratorForDerivedTable(thd, first_qep_tab);
    }
  } else {
    m_root_iterator = move(first_qep_tab->iterator);
  }

  m_root_iterator =
      attach_iterators_for_having_and_limit(move(m_root_iterator));
}

/**
  SemiJoinDuplicateElimination: Weed out duplicate row combinations

  SYNPOSIS
    do_sj_dups_weedout()
      thd    Thread handle
      sjtbl  Duplicate weedout table

  DESCRIPTION
    Try storing current record combination of outer tables (i.e. their
    rowids) in the temporary table. This records the fact that we've seen
    this record combination and also tells us if we've seen it before.

  RETURN
    -1  Error
    1   The row combination is a duplicate (discard it)
    0   The row combination is not a duplicate (continue)
*/

int do_sj_dups_weedout(THD *thd, SJ_TMP_TABLE *sjtbl) {
  int error;
  SJ_TMP_TABLE_TAB *tab = sjtbl->tabs;
  SJ_TMP_TABLE_TAB *tab_end = sjtbl->tabs_end;

  DBUG_TRACE;

  if (sjtbl->is_confluent) {
    if (sjtbl->have_confluent_row)
      return 1;
    else {
      sjtbl->have_confluent_row = true;
      return 0;
    }
  }

  uchar *ptr = sjtbl->tmp_table->visible_field_ptr()[0]->ptr;
  // Put the rowids tuple into table->record[0]:
  // 1. Store the length
  if (((Field_varstring *)(sjtbl->tmp_table->visible_field_ptr()[0]))
          ->length_bytes == 1) {
    *ptr = (uchar)(sjtbl->rowid_len + sjtbl->null_bytes);
    ptr++;
  } else {
    int2store(ptr, sjtbl->rowid_len + sjtbl->null_bytes);
    ptr += 2;
  }

  // 2. Zero the null bytes
  uchar *const nulls_ptr = ptr;
  if (sjtbl->null_bytes) {
    memset(ptr, 0, sjtbl->null_bytes);
    ptr += sjtbl->null_bytes;
  }

  // 3. Put the rowids
  for (uint i = 0; tab != tab_end; tab++, i++) {
    handler *h = tab->qep_tab->table()->file;
    if (tab->qep_tab->table()->is_nullable() &&
        tab->qep_tab->table()->has_null_row()) {
      /* It's a NULL-complemented row */
      *(nulls_ptr + tab->null_byte) |= tab->null_bit;
      memset(ptr + tab->rowid_offset, 0, h->ref_length);
    } else {
      /* Copy the rowid value */
      memcpy(ptr + tab->rowid_offset, h->ref, h->ref_length);
    }
  }

  if (!check_unique_constraint(sjtbl->tmp_table)) return 1;
  error = sjtbl->tmp_table->file->ha_write_row(sjtbl->tmp_table->record[0]);
  if (error) {
    /* If this is a duplicate error, return immediately */
    if (sjtbl->tmp_table->file->is_ignorable_error(error)) return 1;
    /*
      Other error than duplicate error: Attempt to create a temporary table.
    */
    bool is_duplicate;
    if (create_ondisk_from_heap(thd, sjtbl->tmp_table, error, true,
                                &is_duplicate))
      return -1;
    return is_duplicate ? 1 : 0;
  }
  return 0;
}

/*****************************************************************************
  The different ways to read a record
  Returns -1 if row was not found, 0 if row was found and 1 on errors
*****************************************************************************/

/** Help function when we get some an error from the table handler. */

int report_handler_error(TABLE *table, int error) {
  if (error == HA_ERR_END_OF_FILE || error == HA_ERR_KEY_NOT_FOUND) {
    table->set_no_row();
    return -1;  // key not found; ok
  }
  /*
    Do not spam the error log with these temporary errors:
       LOCK_DEADLOCK LOCK_WAIT_TIMEOUT TABLE_DEF_CHANGED
    Also skip printing to error log if the current thread has been killed.
  */
  String dummy;
  if (error != HA_ERR_LOCK_DEADLOCK && error != HA_ERR_LOCK_WAIT_TIMEOUT &&
      error != HA_ERR_TABLE_DEF_CHANGED && !table->in_use->killed &&
      !table->file->get_error_message(error, &dummy))
    LogErr(ERROR_LEVEL, ER_READING_TABLE_FAILED, error, table->s->path.str);
  table->file->print_error(error, MYF(0));
  return 1;
}

/**
  Initialize an index scan and the record buffer to use in the scan.

  @param qep_tab the table to read
  @param file    the handler to initialize
  @param idx     the index to use
  @param sorted  use the sorted order of the index
  @retval true   if an error occurred
  @retval false  on success
*/
static bool init_index_and_record_buffer(const QEP_TAB *qep_tab, handler *file,
                                         uint idx, bool sorted) {
  if (file->inited) return false;  // OK, already initialized

  int error = file->ha_index_init(idx, sorted);
  if (error != 0) {
    (void)report_handler_error(qep_tab->table(), error);
    return true;
  }

  return set_record_buffer(qep_tab);
}

int safe_index_read(QEP_TAB *tab) {
  int error;
  TABLE *table = tab->table();
  if ((error = table->file->ha_index_read_map(
           table->record[0], tab->ref().key_buff,
           make_prev_keypart_map(tab->ref().key_parts), HA_READ_KEY_EXACT)))
    return report_handler_error(table, error);
  return 0;
}

/**
   Reads content of constant table
   @param tab  table
   @param pos  position of table in query plan
   @retval 0   ok, one row was found or one NULL-complemented row was created
   @retval -1  ok, no row was found and no NULL-complemented row was created
   @retval 1   error
*/

int join_read_const_table(JOIN_TAB *tab, POSITION *pos) {
  int error;
  DBUG_TRACE;
  TABLE *table = tab->table();
  THD *const thd = tab->join()->thd;
  table->const_table = true;
  DBUG_ASSERT(!thd->is_error());

  if (table->reginfo.lock_type >= TL_WRITE_ALLOW_WRITE) {
    const enum_sql_command sql_command = tab->join()->thd->lex->sql_command;
    if (sql_command == SQLCOM_UPDATE_MULTI ||
        sql_command == SQLCOM_DELETE_MULTI) {
      /*
        In a multi-UPDATE, if we represent "depends on" with "->", we have:
        "what columns to read (read_set)" ->
        "whether table will be updated on-the-fly or with tmp table" ->
        "whether to-be-updated columns are used by access path"
        "access path to table (range, ref, scan...)" ->
        "query execution plan" ->
        "what tables are const" ->
        "reading const tables" ->
        "what columns to read (read_set)".
        To break this loop, we always read all columns of a constant table if
        it is going to be updated.
        Another case is in multi-UPDATE and multi-DELETE, when the table has a
        trigger: bits of columns needed by the trigger are turned on in
        result->optimize(), which has not yet been called when we do
        the reading now, so we must read all columns.
      */
      bitmap_set_all(table->read_set);
      /* Virtual generated columns must be writable */
      for (Field **vfield_ptr = table->vfield; vfield_ptr && *vfield_ptr;
           vfield_ptr++)
        bitmap_set_bit(table->write_set, (*vfield_ptr)->field_index);
      table->file->column_bitmaps_signal();
    }
  }

  if (tab->type() == JT_SYSTEM)
    error = read_system(table);
  else {
    if (!table->key_read && table->covering_keys.is_set(tab->ref().key) &&
        !table->no_keyread &&
        (int)table->reginfo.lock_type <= (int)TL_READ_HIGH_PRIORITY) {
      table->set_keyread(true);
      tab->set_index(tab->ref().key);
    }
    error = read_const(table, &tab->ref());
    table->set_keyread(false);
  }

  if (error) {
    // Promote error to fatal if an actual error was reported
    if (thd->is_error()) error = 1;
    /* Mark for EXPLAIN that the row was not found */
    pos->filter_effect = 1.0;
    pos->rows_fetched = 0.0;
    pos->prefix_rowcount = 0.0;
    pos->ref_depend_map = 0;
    if (!tab->table_ref->outer_join || error > 0) return error;
  }

  if (tab->join_cond() && !table->has_null_row()) {
    // We cannot handle outer-joined tables with expensive join conditions here:
    DBUG_ASSERT(!tab->join_cond()->is_expensive());
    if (tab->join_cond()->val_int() == 0) table->set_null_row();
  }

  /* Check appearance of new constant items in Item_equal objects */
  JOIN *const join = tab->join();
  if (join->where_cond && update_const_equal_items(thd, join->where_cond, tab))
    return 1;
  TABLE_LIST *tbl;
  for (tbl = join->select_lex->leaf_tables; tbl; tbl = tbl->next_leaf) {
    TABLE_LIST *embedded;
    TABLE_LIST *embedding = tbl;
    do {
      embedded = embedding;
      if (embedded->join_cond_optim() &&
          update_const_equal_items(thd, embedded->join_cond_optim(), tab))
        return 1;
      embedding = embedded->embedding;
    } while (embedding &&
             embedding->nested_join->join_list.front() == embedded);
  }

  return 0;
}

/**
  Read a constant table when there is at most one matching row, using a table
  scan.

  @param table			Table to read

  @retval  0  Row was found
  @retval  -1 Row was not found
  @retval  1  Got an error (other than row not found) during read
*/
static int read_system(TABLE *table) {
  int error;
  if (!table->is_started())  // If first read
  {
    if ((error = table->file->ha_read_first_row(table->record[0],
                                                table->s->primary_key))) {
      if (error != HA_ERR_END_OF_FILE)
        return report_handler_error(table, error);
      table->set_null_row();
      empty_record(table);  // Make empty record
      return -1;
    }
    store_record(table, record[1]);
  } else if (table->has_row() && table->is_nullable()) {
    /*
      Row buffer contains a row, but it may have been partially overwritten
      by a null-extended row. Restore the row from the saved copy.
      @note this branch is currently unused.
    */
    DBUG_ASSERT(false);
    table->set_found_row();
    restore_record(table, record[1]);
  }

  return table->has_row() ? 0 : -1;
}

ConstIterator::ConstIterator(THD *thd, TABLE *table, TABLE_REF *table_ref,
                             ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_ref(table_ref),
      m_examined_rows(examined_rows) {}

bool ConstIterator::Init() {
  m_first_record_since_init = true;
  return false;
}

/**
  Read a constant table when there is at most one matching row, using an
  index lookup.

  @retval 0  Row was found
  @retval -1 Row was not found
  @retval 1  Got an error (other than row not found) during read
*/

int ConstIterator::Read() {
  if (!m_first_record_since_init) {
    return -1;
  }
  m_first_record_since_init = false;
  int err = read_const(table(), m_ref);
  if (err == 0 && m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  table()->const_table = true;
  return err;
}

vector<string> ConstIterator::DebugString() const {
  DBUG_ASSERT(table()->file->pushed_idx_cond == nullptr);
  DBUG_ASSERT(table()->file->pushed_cond == nullptr);
  return {string("Constant row from ") + table()->alias};
}

static int read_const(TABLE *table, TABLE_REF *ref) {
  int error;
  DBUG_TRACE;

  if (!table->is_started())  // If first read
  {
    /* Perform "Late NULLs Filtering" (see internals manual for explanations) */
    if (ref->impossible_null_ref() ||
        construct_lookup_ref(table->in_use, table, ref))
      error = HA_ERR_KEY_NOT_FOUND;
    else {
      // Increment rows_requested for the corresponding index if `table` is
      // associated with an index.
      if (table->in_use) {
        ulonglong *ius_requested_rows = get_or_add_index_stats_ptr(
            &(table->in_use->thd_ius), table, ref->key);
        if (ius_requested_rows != nullptr) {
          ++*ius_requested_rows;
        }
      }
      error = table->file->ha_index_read_idx_map(
          table->record[0], ref->key, ref->key_buff,
          make_prev_keypart_map(ref->key_parts), HA_READ_KEY_EXACT);
    }
    if (error) {
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
        const int ret = report_handler_error(table, error);
        return ret;
      }
      table->set_no_row();
      table->set_null_row();
      empty_record(table);
      return -1;
    }
    /*
      read_const() may be called several times inside a nested loop join.
      Save record in case it is needed when table is in "started" state.
    */
    store_record(table, record[1]);
  } else if (table->has_row() && table->is_nullable()) {
    /*
      Row buffer contains a row, but it may have been partially overwritten
      by a null-extended row. Restore the row from the saved copy.
    */
    table->set_found_row();
    restore_record(table, record[1]);
  }
  return table->has_row() ? 0 : -1;
}

EQRefIterator::EQRefIterator(THD *thd, TABLE *table, TABLE_REF *ref,
                             bool use_order, ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_ref(ref),
      m_use_order(use_order),
      m_examined_rows(examined_rows) {}

/**
  Read row using unique key: eq_ref access method implementation

  @details
    This is the "read_first" function for the eq_ref access method.
    The difference from ref access function is that it has a one-element
    lookup cache, maintained in record[0]. Since the eq_ref access method
    will always return the same row, it is not necessary to read the row
    more than once, regardless of how many times it is needed in execution.
    This cache element is used when a row is needed after it has been read once,
    unless a key conversion error has occurred, or the cache has been disabled.

  @retval  0 - Ok
  @retval -1 - Row not found
  @retval  1 - Error
*/

bool EQRefIterator::Init() {
  if (!table()->file->inited) {
    DBUG_ASSERT(!m_use_order);  // Don't expect sort req. for single row.
    int error = table()->file->ha_index_init(m_ref->key, m_use_order);
    if (error) {
      PrintError(error);
      return true;
    }

    // Insert a record in the book-keeping THD data structure that tracks
    // rows_requested for each index.
    ius_requested_rows =
        get_or_add_index_stats_ptr(&(thd()->thd_ius), table(), m_ref->key);
  }

  m_first_record_since_init = true;

  return false;
}

/**
  Read row using unique key: eq_ref access method implementation

  @details
    The difference from RefIterator is that it has a one-element
    lookup cache, maintained in record[0]. Since the eq_ref access method
    will always return the same row, it is not necessary to read the row
    more than once, regardless of how many times it is needed in execution.
    This cache element is used when a row is needed after it has been read once,
    unless a key conversion error has occurred, or the cache has been disabled.

  @retval  0 - Ok
  @retval -1 - Row not found
  @retval  1 - Error
*/

int EQRefIterator::Read() {
  if (!m_first_record_since_init) {
    return -1;
  }
  m_first_record_since_init = false;

  /*
    Calculate if needed to read row. Always needed if
    - no rows read yet, or
    - table has a pushed condition, or
    - cache is disabled, or
    - previous lookup caused error when calculating key.
  */
  bool read_row = !table()->is_started() || table()->file->pushed_cond ||
                  m_ref->disable_cache || m_ref->key_err;
  if (!read_row)
    // Last lookup found a row, copy its key to secondary buffer
    memcpy(m_ref->key_buff2, m_ref->key_buff, m_ref->key_length);

  // Create new key for lookup
  m_ref->key_err = construct_lookup_ref(table()->in_use, table(), m_ref);
  if (m_ref->key_err) {
    table()->set_no_row();
    return -1;
  }

  // Re-use current row if keys are equal
  if (!read_row &&
      memcmp(m_ref->key_buff2, m_ref->key_buff, m_ref->key_length) != 0)
    read_row = true;

  if (read_row) {
    /*
       Moving away from the current record. Unlock the row
       in the handler if it did not match the partial WHERE.
     */
    if (table()->has_row() && m_ref->use_count == 0)
      table()->file->unlock_row();

    /*
      Perform "Late NULLs Filtering" (see internals manual for explanations)

      As EQRefIterator effectively implements a one row cache of last
      fetched row, the NULLs filtering cant be done until after the cache
      key has been checked and updated, and row locks maintained.
    */
    if (m_ref->impossible_null_ref()) {
      DBUG_PRINT("info", ("EQRefIterator null_rejected"));
      table()->set_no_row();
      return -1;
    }

    // Increment rows_requested counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }

    pair<uchar *, key_part_map> key_buff_and_map = FindKeyBufferAndMap(m_ref);
    int error = table()->file->ha_index_read_map(
        table()->record[0], key_buff_and_map.first, key_buff_and_map.second,
        HA_READ_KEY_EXACT);
    if (error) {
      return HandleError(error);
    }

    m_ref->use_count = 1;
    table()->save_null_flags();
  } else if (table()->has_row()) {
    DBUG_ASSERT(!table()->has_null_row());
    table()->restore_null_flags();
    m_ref->use_count++;
  }

  if (table()->has_row() && m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return table()->has_row() ? 0 : -1;
}

/**
  Since EQRefIterator may buffer a record, do not unlock
  it if it was not used in this invocation of EQRefIterator::Read().
  Only count locks, thus remembering if the record was left unused,
  and unlock already when pruning the current value of
  TABLE_REF buffer.
  @sa EQRefIterator::Read()
*/

void EQRefIterator::UnlockRow() {
  DBUG_ASSERT(m_ref->use_count);
  if (m_ref->use_count) m_ref->use_count--;
}

vector<string> EQRefIterator::DebugString() const {
  const KEY *key = &table()->key_info[m_ref->key];
  string str = string("Single-row index lookup on ") + table()->alias +
               " using " + key->name + " (" +
               RefToString(*m_ref, key, /*include_nulls=*/false) + ")";
  if (table()->file->pushed_idx_cond != nullptr) {
    str += ", with index condition: " +
           ItemToString(table()->file->pushed_idx_cond);
  }
  str += table()->file->explain_extra();
  return {str};
}

PushedJoinRefIterator::PushedJoinRefIterator(THD *thd, TABLE *table,
                                             TABLE_REF *ref, bool use_order,
                                             ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_ref(ref),
      m_use_order(use_order),
      m_examined_rows(examined_rows) {}

bool PushedJoinRefIterator::Init() {
  DBUG_ASSERT(!m_use_order);  // Pushed child can't be sorted

  if (!table()->file->inited) {
    // Insert a record in the book-keeping THD data structure that tracks
    // rows_requested for each index.
    ius_requested_rows =
        get_or_add_index_stats_ptr(&(thd()->thd_ius), table(), m_ref->key);
    int error = table()->file->ha_index_init(m_ref->key, m_use_order);
    if (error) {
      PrintError(error);
      return true;
    }
  }

  m_first_record_since_init = true;
  return false;
}

int PushedJoinRefIterator::Read() {
  if (m_first_record_since_init) {
    m_first_record_since_init = false;

    /* Perform "Late NULLs Filtering" (see internals manual for explanations) */
    if (m_ref->impossible_null_ref()) {
      table()->set_no_row();
      DBUG_PRINT("info", ("PushedJoinRefIterator::Read() null_rejected"));
      return -1;
    }

    if (construct_lookup_ref(thd(), table(), m_ref)) {
      table()->set_no_row();
      return -1;
    }

    // Bump requested_rows counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }

    // 'read' itself is a NOOP:
    //  handler::ha_index_read_pushed() only unpack the prefetched row and
    //  set 'status'
    int error = table()->file->ha_index_read_pushed(
        table()->record[0], m_ref->key_buff,
        make_prev_keypart_map(m_ref->key_parts));
    if (error) {
      return HandleError(error);
    }
  } else {
    // Bump requested_rows counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }
    int error = table()->file->ha_index_next_pushed(table()->record[0]);
    if (error) {
      return HandleError(error);
    }
  }
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

vector<string> PushedJoinRefIterator::DebugString() const {
  DBUG_ASSERT(table()->file->pushed_idx_cond == nullptr);
  const KEY *key = &table()->key_info[m_ref->key];
  return {string("Index lookup on ") + table()->alias + " using " + key->name +
          " (" + RefToString(*m_ref, key, /*include_nulls=*/false) + ")" +
          table()->file->explain_extra()};
}

template <bool Reverse>
bool RefIterator<Reverse>::Init() {
  m_first_record_since_init = true;

  // Insert a record in the book-keeping THD data structure that tracks
  // rows_requested for each index.
  ius_requested_rows =
      get_or_add_index_stats_ptr(&(thd()->thd_ius), table(), m_ref->key);

  return init_index_and_record_buffer(m_qep_tab, m_qep_tab->table()->file,
                                      m_ref->key, m_use_order);
}

template <bool Reverse>
vector<string> RefIterator<Reverse>::DebugString() const {
  const KEY *key = &table()->key_info[m_ref->key];
  string str = string("Index lookup on ") + table()->alias + " using " +
               key->name + " (" +
               RefToString(*m_ref, key, /*include_nulls=*/false);
  if (Reverse) {
    str += "; iterate backwards";
  }
  str += ")";
  if (table()->file->pushed_idx_cond != nullptr) {
    str += ", with index condition: " +
           ItemToString(table()->file->pushed_idx_cond);
  }
  str += table()->file->explain_extra();
  return {str};
}

// Doxygen gets confused by the explicit specializations.

//! @cond
template <>
int RefIterator<false>::Read() {  // Forward read.
  if (m_first_record_since_init) {
    m_first_record_since_init = false;

    /*
      a = b can never return true if a or b is NULL, so if we're asked
      to do such a lookup, we can say there won't be a match without even
      checking the index. This is “late NULLs filtering” (as opposed to
      “early NULLs filtering”, which propagates the IS NOT NULL constraint
      further back to the other table so we don't even get the request).
      See the internals manual for more details.
     */
    if (m_ref->impossible_null_ref()) {
      DBUG_PRINT("info", ("RefIterator null_rejected"));
      table()->set_no_row();
      return -1;
    }
    if (construct_lookup_ref(thd(), table(), m_ref)) {
      table()->set_no_row();
      return -1;
    }

    // Bump rows_requested counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }

    pair<uchar *, key_part_map> key_buff_and_map = FindKeyBufferAndMap(m_ref);
    int error = table()->file->ha_index_read_map(
        table()->record[0], key_buff_and_map.first, key_buff_and_map.second,
        HA_READ_KEY_EXACT);
    if (error) {
      return HandleError(error);
    }
  } else {
    // Bump rows_requested counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }
    int error = table()->file->ha_index_next_same(
        table()->record[0], m_ref->key_buff, m_ref->key_length);
    if (error) {
      return HandleError(error);
    }
  }
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

/**
  This function is used when optimizing away ORDER BY in
  SELECT * FROM t1 WHERE a=1 ORDER BY a DESC,b DESC.
*/
template <>
int RefIterator<true>::Read() {  // Reverse read.
  assert(m_ref->keypart_hash == nullptr);

  if (m_first_record_since_init) {
    m_first_record_since_init = false;

    /*
      a = b can never return true if a or b is NULL, so if we're asked
      to do such a lookup, we can say there won't be a match without even
      checking the index. This is “late NULLs filtering” (as opposed to
      “early NULLs filtering”, which propagates the IS NOT NULL constraint
      further back to the other table so we don't even get the request).
      See the internals manual for more details.
     */
    if (m_ref->impossible_null_ref()) {
      DBUG_PRINT("info", ("RefIterator null_rejected"));
      table()->set_no_row();
      return -1;
    }
    if (construct_lookup_ref(thd(), table(), m_ref)) {
      table()->set_no_row();
      return -1;
    }

    // Bump rows_requested counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }

    int error = table()->file->ha_index_read_last_map(
        table()->record[0], m_ref->key_buff,
        make_prev_keypart_map(m_ref->key_parts));
    if (error) {
      return HandleError(error);
    }
  } else {
    /*
      Using ha_index_prev() for reading records from the table can cause
      performance issues if used in combination with ICP. The ICP code
      in the storage engine does not know when to stop reading from the
      index and a call to ha_index_prev() might cause the storage engine
      to read to the beginning of the index if no qualifying record is
      found.
     */
    DBUG_ASSERT(table()->file->pushed_idx_cond == nullptr);

    // Bump rows_requested counter for the index.
    if (ius_requested_rows != nullptr) {
      ++*ius_requested_rows;
    }

    int error = table()->file->ha_index_prev(table()->record[0]);
    if (error) {
      return HandleError(error);
    }
    if (key_cmp_if_same(table(), m_ref->key_buff, m_ref->key,
                        m_ref->key_length)) {
      table()->set_no_row();
      return -1;
    }
  }
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}
//! @endcond

DynamicRangeIterator::DynamicRangeIterator(THD *thd, TABLE *table,
                                           QEP_TAB *qep_tab,
                                           ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_qep_tab(qep_tab),
      m_examined_rows(examined_rows),
      m_original_read_set(table->read_set) {
  add_virtual_gcol_base_cols(table, thd->mem_root, &m_table_scan_read_set);
}

bool DynamicRangeIterator::Init() {
  // The range optimizer generally expects this to be set.
  thd()->lex->set_current_select(m_qep_tab->join()->select_lex);

  Opt_trace_context *const trace = &thd()->opt_trace;
  const bool disable_trace =
      m_quick_traced_before &&
      !trace->feature_enabled(Opt_trace_context::DYNAMIC_RANGE);
  Opt_trace_disable_I_S disable_trace_wrapper(trace, disable_trace);

  m_quick_traced_before = true;

  Opt_trace_object wrapper(trace);
  Opt_trace_object trace_table(trace, "rows_estimation_per_outer_row");
  trace_table.add_utf8_table(m_qep_tab->table_ref);

  Key_map needed_reg_dummy;
  QUICK_SELECT_I *old_qck = m_qep_tab->quick();
  QUICK_SELECT_I *qck;
  DEBUG_SYNC(thd(), "quick_not_created");
  const int rc = test_quick_select(thd(), m_qep_tab->keys(),
                                   0,  // empty table map
                                   HA_POS_ERROR,
                                   false,  // don't force quick range
                                   ORDER_NOT_RELEVANT, m_qep_tab,
                                   m_qep_tab->condition(), &needed_reg_dummy,
                                   &qck, m_qep_tab->table()->force_index);
  if (thd()->is_error())  // @todo consolidate error reporting of
                          // test_quick_select
    return true;
  DBUG_ASSERT(old_qck == nullptr || old_qck != qck);
  m_qep_tab->set_quick(qck);

  /*
    EXPLAIN CONNECTION is used to understand why a query is currently taking
    so much time. So it makes sense to show what the execution is doing now:
    is it a table scan or a range scan? A range scan on which index.
    So: below we want to change the type and quick visible in EXPLAIN, and for
    that, we need to take mutex and change type and quick_optim.
  */

  DEBUG_SYNC(thd(), "quick_created_before_mutex");

  thd()->lock_query_plan();
  m_qep_tab->set_type(qck ? calc_join_type(qck->get_type()) : JT_ALL);
  m_qep_tab->set_quick_optim();
  thd()->unlock_query_plan();

  delete old_qck;
  DEBUG_SYNC(thd(), "quick_droped_after_mutex");

  // Clear out and destroy any old iterators before we start constructing
  // new ones, since they may share the same memory in the union.
  m_iterator.reset();

  if (rc == -1) {
    return false;
  }

  if (qck) {
    m_iterator = NewIterator<IndexRangeScanIterator>(
        thd(), table(), qck, m_qep_tab, m_examined_rows);
    table()->read_set = m_original_read_set;
  } else {
    m_iterator = NewIterator<TableScanIterator>(thd(), table(), m_qep_tab,
                                                m_examined_rows);
    table()->read_set = &m_table_scan_read_set;
  }
  return m_iterator->Init();
}

int DynamicRangeIterator::Read() {
  if (m_iterator == nullptr) {
    return -1;
  } else {
    return m_iterator->Read();
  }
}

vector<string> DynamicRangeIterator::DebugString() const {
  // TODO: Convert QUICK_SELECT_I to RowIterator so that we can get
  // better outputs here (similar to dbug_dump()), although it might
  // get tricky when there are many alternatives.
  string str = string("Index range scan on ") + table()->alias +
               " (re-planned for each iteration)";
  if (table()->file->pushed_idx_cond != nullptr) {
    str += ", with index condition: " +
           ItemToString(table()->file->pushed_idx_cond);
  }
  str += table()->file->explain_extra();
  return {str};
}

static void join_setup_iterator(QEP_TAB *tab) {
  bool using_table_scan;
  tab->iterator =
      create_table_iterator(tab->join()->thd, nullptr, tab, false,
                            /*ignore_not_found_rows=*/false,
                            /*examined_rows=*/nullptr, &using_table_scan);
  tab->set_using_table_scan(using_table_scan);
}

/**
  Check if access to this JOIN_TAB has to retrieve rows
  in sorted order as defined by the ordered index
  used to access this table.
*/
bool QEP_TAB::use_order() const {
  /*
    No need to require sorted access for single row reads
    being performed by const- or EQ_REF-accessed tables.
  */
  if (type() == JT_EQ_REF || type() == JT_CONST || type() == JT_SYSTEM)
    return false;

  /*
    First non-const table requires sorted results
    if ORDER or GROUP BY use ordered index.
  */
  if ((uint)idx() == join()->const_tables &&
      join()->m_ordered_index_usage != JOIN::ORDERED_INDEX_VOID)
    return true;

  /*
    LooseScan strategy for semijoin requires sorted
    results even if final result is not to be sorted.
  */
  if (position()->sj_strategy == SJ_OPT_LOOSE_SCAN) return true;

  /* Fall through: Results don't have to be sorted */
  return false;
}

FullTextSearchIterator::FullTextSearchIterator(THD *thd, TABLE *table,
                                               TABLE_REF *ref, bool use_order,
                                               ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_ref(ref),
      m_use_order(use_order),
      m_examined_rows(examined_rows) {}

FullTextSearchIterator::~FullTextSearchIterator() {
  table()->file->ha_index_or_rnd_end();
}

bool FullTextSearchIterator::Init() {
  if (!table()->file->inited) {
    // Insert a record in the book-keeping THD data structure that tracks
    // rows_requested for each index.
    ius_requested_rows =
        get_or_add_index_stats_ptr(&(thd()->thd_ius), table(), m_ref->key);

    int error = table()->file->ha_index_init(m_ref->key, m_use_order);
    if (error) {
      PrintError(error);
      return true;
    }
  }
  table()->file->ft_init();
  return false;
}

int FullTextSearchIterator::Read() {
  // Increment rows_requested counter for the index.
  if (ius_requested_rows != nullptr) {
    ++*ius_requested_rows;
  }

  int error = table()->file->ha_ft_read(table()->record[0]);
  if (error) {
    return HandleError(error);
  }
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

vector<string> FullTextSearchIterator::DebugString() const {
  DBUG_ASSERT(table()->file->pushed_idx_cond == nullptr);
  const KEY *key = &table()->key_info[m_ref->key];
  return {string("Indexed full text search on ") + table()->alias + " using " +
          key->name + " (" + RefToString(*m_ref, key, /*include_nulls=*/false) +
          ")" + table()->file->explain_extra()};
}

/**
  Reading of key with key reference and one part that may be NULL.
*/

RefOrNullIterator::RefOrNullIterator(THD *thd, TABLE *table, TABLE_REF *ref,
                                     bool use_order, QEP_TAB *qep_tab,
                                     ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_ref(ref),
      m_use_order(use_order),
      m_qep_tab(qep_tab),
      m_examined_rows(examined_rows) {}

bool RefOrNullIterator::Init() {
  m_reading_first_row = true;
  *m_ref->null_ref_key = false;

  // Insert a record in the book-keeping THD data structure that tracks
  // rows_requested for each index.
  ius_requested_rows =
      get_or_add_index_stats_ptr(&(thd()->thd_ius), table(), m_ref->key);
  return init_index_and_record_buffer(m_qep_tab, m_qep_tab->table()->file,
                                      m_ref->key, m_use_order);
}

int RefOrNullIterator::Read() {
  if (m_reading_first_row && !*m_ref->null_ref_key) {
    /* Perform "Late NULLs Filtering" (see internals manual for explanations)
     */
    if (m_ref->impossible_null_ref() ||
        construct_lookup_ref(thd(), table(), m_ref)) {
      // Skip searching for non-NULL rows; go straight to NULL rows.
      *m_ref->null_ref_key = true;
    }
  }

  pair<uchar *, key_part_map> key_buff_and_map = FindKeyBufferAndMap(m_ref);

  // Increment rows_requested counter for the index.
  if (ius_requested_rows != nullptr) {
    ++*ius_requested_rows;
  }

  int error;
  if (m_reading_first_row) {
    m_reading_first_row = false;
    error = table()->file->ha_index_read_map(
        table()->record[0], key_buff_and_map.first, key_buff_and_map.second,
        HA_READ_KEY_EXACT);
  } else {
    error = table()->file->ha_index_next_same(
        table()->record[0], key_buff_and_map.first, m_ref->key_length);
  }

  if (error == 0) {
    if (m_examined_rows != nullptr) {
      ++*m_examined_rows;
    }
    return 0;
  } else if (error == HA_ERR_END_OF_FILE || error == HA_ERR_KEY_NOT_FOUND) {
    if (!*m_ref->null_ref_key) {
      // No more non-NULL rows; try again with NULL rows.
      *m_ref->null_ref_key = true;
      m_reading_first_row = true;
      return Read();
    } else {
      // Real EOF.
      table()->set_no_row();
      return -1;
    }
  } else {
    return HandleError(error);
  }
}

vector<string> RefOrNullIterator::DebugString() const {
  const KEY *key = &table()->key_info[m_ref->key];
  string str = string("Index lookup on ") + table()->alias + " using " +
               key->name + " (" +
               RefToString(*m_ref, key, /*include_nulls=*/true) + ")";
  if (table()->file->pushed_idx_cond != nullptr) {
    str += ", with index condition: " +
           ItemToString(table()->file->pushed_idx_cond);
  }
  str += table()->file->explain_extra();
  return {str};
}

AlternativeIterator::AlternativeIterator(
    THD *thd, TABLE *table, QEP_TAB *qep_tab, ha_rows *examined_rows,
    unique_ptr_destroy_only<RowIterator> source, TABLE_REF *ref)
    : RowIterator(thd),
      m_ref(ref),
      m_source_iterator(std::move(source)),
      m_table_scan_iterator(
          NewIterator<TableScanIterator>(thd, table, qep_tab, examined_rows)),
      m_table(table),
      m_original_read_set(table->read_set) {
  for (unsigned key_part_idx = 0; key_part_idx < ref->key_parts;
       ++key_part_idx) {
    bool *cond_guard = ref->cond_guards[key_part_idx];
    if (cond_guard != nullptr) {
      m_applicable_cond_guards.push_back(cond_guard);
    }
  }
  DBUG_ASSERT(!m_applicable_cond_guards.empty());

  add_virtual_gcol_base_cols(table, thd->mem_root, &m_table_scan_read_set);
}

bool AlternativeIterator::Init() {
  m_iterator = m_source_iterator.get();
  m_table->read_set = m_original_read_set;
  for (bool *cond_guard : m_applicable_cond_guards) {
    if (!*cond_guard) {
      m_iterator = m_table_scan_iterator.get();
      m_table->read_set = &m_table_scan_read_set;
      break;
    }
  }

  if (m_iterator != m_last_iterator_inited) {
    m_table->file->ha_index_or_rnd_end();
    m_last_iterator_inited = m_iterator;
  }

  return m_iterator->Init();
}

vector<string> AlternativeIterator::DebugString() const {
  const TABLE *table =
      down_cast<TableScanIterator *>(m_table_scan_iterator->real_iterator())
          ->table();
  const KEY *key = &table->key_info[m_ref->key];
  string ret = "Alternative plans for IN subquery: Index lookup unless ";
  if (m_applicable_cond_guards.size() > 1) {
    ret += " any of (";
  }
  bool first = true;
  for (unsigned key_part_idx = 0; key_part_idx < m_ref->key_parts;
       ++key_part_idx) {
    if (m_ref->cond_guards[key_part_idx] == nullptr) {
      continue;
    }
    if (!first) {
      ret += ", ";
    }
    first = false;
    ret += key->key_part[key_part_idx].field->field_name;
  }
  if (m_applicable_cond_guards.size() > 1) {
    ret += ")";
  }
  ret += " IS NULL";
  return {ret};
}

/**
  Pick the appropriate access method functions

  Sets the functions for the selected table access method
*/

void QEP_TAB::pick_table_access_method() {
  DBUG_ASSERT(table());
  // Only some access methods support reversed access:
  DBUG_ASSERT(!m_reversed_access || type() == JT_REF ||
              type() == JT_INDEX_SCAN);
  TABLE_REF *used_ref = nullptr;

  const TABLE *pushed_root = table()->file->member_of_pushed_join();
  const bool is_pushed_child = (pushed_root && pushed_root != table());
  // A 'pushed_child' has to be a REF type
  DBUG_ASSERT(!is_pushed_child || type() == JT_REF || type() == JT_EQ_REF);

  switch (type()) {
    case JT_REF:
      if (is_pushed_child) {
        DBUG_ASSERT(!m_reversed_access);
        iterator = NewIterator<PushedJoinRefIterator>(
            join()->thd, table(), &ref(), use_order(), &join()->examined_rows);
      } else if (m_reversed_access) {
        iterator = NewIterator<RefIterator<true>>(join()->thd, table(), &ref(),
                                                  use_order(), this,
                                                  &join()->examined_rows);
      } else {
        iterator = NewIterator<RefIterator<false>>(join()->thd, table(), &ref(),
                                                   use_order(), this,
                                                   &join()->examined_rows);
      }
      used_ref = &ref();
      break;

    case JT_REF_OR_NULL:
      iterator = NewIterator<RefOrNullIterator>(join()->thd, table(), &ref(),
                                                use_order(), this,
                                                &join()->examined_rows);
      used_ref = &ref();
      break;

    case JT_CONST:
      iterator = NewIterator<ConstIterator>(join()->thd, table(), &ref(),
                                            &join()->examined_rows);
      break;

    case JT_EQ_REF:
      if (is_pushed_child) {
        iterator = NewIterator<PushedJoinRefIterator>(
            join()->thd, table(), &ref(), use_order(), &join()->examined_rows);
      } else {
        iterator = NewIterator<EQRefIterator>(
            join()->thd, table(), &ref(), use_order(), &join()->examined_rows);
      }
      used_ref = &ref();
      break;

    case JT_FT:
      iterator = NewIterator<FullTextSearchIterator>(
          join()->thd, table(), &ref(), use_order(), &join()->examined_rows);
      used_ref = &ref();
      break;

    case JT_INDEX_SCAN:
      if (m_reversed_access) {
        iterator = NewIterator<IndexScanIterator<true>>(
            join()->thd, table(), index(), use_order(), this,
            &join()->examined_rows);
      } else {
        iterator = NewIterator<IndexScanIterator<false>>(
            join()->thd, table(), index(), use_order(), this,
            &join()->examined_rows);
      }
      break;
    case JT_ALL:
    case JT_RANGE:
    case JT_INDEX_MERGE:
      if (using_dynamic_range) {
        iterator = NewIterator<DynamicRangeIterator>(join()->thd, table(), this,
                                                     &join()->examined_rows);
      } else {
        iterator =
            create_table_iterator(join()->thd, nullptr, this, false,
                                  /*ignore_not_found_rows=*/false,
                                  &join()->examined_rows, &m_using_table_scan);
      }
      break;
    default:
      DBUG_ASSERT(0);
      break;
  }

  /*
    If we have an item like <expr> IN ( SELECT f2 FROM t2 ), and we were not
    able to rewrite it into a semijoin, the optimizer may rewrite it into
    EXISTS ( SELECT 1 FROM t2 WHERE f2=<expr> LIMIT 1 ) (ie., pushing down the
    value into the subquery), using a REF or REF_OR_NULL scan on t2 if possible.
    This happens in Item_in_subselect::select_in_like_transformer() and the
    functions it calls.

    However, if <expr> evaluates to NULL, this transformation is incorrect,
    and the transformation used should instead be to

      EXISTS ( SELECT 1 FROM t2 LIMIT 1 ) ? NULL : FALSE.

    Thus, in the case of nullable <expr>, the rewriter inserts so-called
    “condition guards” (pointers to bool saying whether <expr> was NULL or not,
    for each part of <expr> if it contains multiple columns). These condition
    guards do two things:

      1. They disable the pushed-down WHERE clauses.
      2. They change the REF/REF_OR_NULL accesses to table scans.

    We don't need to worry about #1 here, but #2 needs to be dealt with,
    as it changes the plan. We solve it by inserting an AlternativeIterator
    that chooses between two sub-iterators at execution time, based on the
    condition guard in question.

    Note that ideally, we'd plan a completely separate plan for the NULL case,
    as there might be e.g. a different index we could scan on, or even a
    different optimal join order. (Note, however, that for the case of multiple
    columns in the expression, we could get 2^N different plans.) However, given
    that most cases are now handled by semijoins and not in2exists at all,
    we don't need to jump through every possible hoop to optimize these cases.
   */
  if (used_ref != nullptr) {
    for (unsigned key_part_idx = 0; key_part_idx < used_ref->key_parts;
         ++key_part_idx) {
      if (used_ref->cond_guards[key_part_idx] != nullptr) {
        DBUG_ASSERT(!is_pushed_child);
        // At least one condition guard is relevant, so we need to use
        // the AlternativeIterator.
        iterator = NewIterator<AlternativeIterator>(join()->thd, table(), this,
                                                    &join()->examined_rows,
                                                    move(iterator), used_ref);
        break;
      }
    }
  }
}

/**
  Get exact count of rows in all tables. When this is called, at least one
  table's SE doesn't include HA_COUNT_ROWS_INSTANT.

    @param qep_tab      List of qep_tab in this JOIN.
    @param table_count  Count of qep_tab in the JOIN.
    @param error [out]  Return any possible error. Else return 0

    @returns
      Cartesian product of count of the rows in all tables if success
      0 if error.

  @note The "error" parameter is required for the sake of testcases like the
        one in innodb-wl6742.test:272. Earlier if an error was raised by
        ha_records, it wasn't handled by get_exact_record_count. Instead it was
        just allowed to go to the execution phase, where end_send_group would
        see the same error and raise it.

        But with the new function 'end_send_count' in the execution phase,
        such an error should be properly returned so that it can be raised.
*/
ulonglong get_exact_record_count(QEP_TAB *qep_tab, uint table_count,
                                 int *error) {
  ulonglong count = 1;
  QEP_TAB *qt;

  for (uint i = 0; i < table_count; i++) {
    ha_rows tmp = 0;
    qt = qep_tab + i;

    if (qt->type() == JT_ALL || (qt->index() == qt->table()->s->primary_key &&
                                 qt->table()->file->primary_key_is_clustered()))
      *error = qt->table()->file->ha_records(&tmp);
    else
      *error = qt->table()->file->ha_records(&tmp, qt->index());
    if (*error != 0) {
      (void)report_handler_error(qt->table(), *error);
      return 0;
    }
    count *= tmp;
  }
  *error = 0;
  return count;
}

static bool cmp_field_value(Field *field, ptrdiff_t diff) {
  DBUG_ASSERT(field);
  /*
    Records are different when:
    1) NULL flags aren't the same
    2) length isn't the same
    3) data isn't the same
  */
  const bool value1_isnull = field->is_real_null();
  const bool value2_isnull = field->is_real_null(diff);

  if (value1_isnull != value2_isnull)  // 1
    return true;
  if (value1_isnull) return false;  // Both values are null, no need to proceed.

  const size_t value1_length = field->data_length();
  const size_t value2_length = field->data_length(diff);

  if (field->type() == MYSQL_TYPE_JSON) {
    Field_json *json_field = down_cast<Field_json *>(field);

    // Fetch the JSON value on the left side of the comparison.
    Json_wrapper left_wrapper;
    if (json_field->val_json(&left_wrapper))
      return true; /* purecov: inspected */

    // Fetch the JSON value on the right side of the comparison.
    Json_wrapper right_wrapper;
    json_field->ptr += diff;
    bool err = json_field->val_json(&right_wrapper);
    json_field->ptr -= diff;
    if (err) return true; /* purecov: inspected */

    return (left_wrapper.compare(right_wrapper) != 0);
  }

  // Trailing space can't be skipped and length is different
  if (!field->is_text_key_type() && value1_length != value2_length)  // 2
    return true;

  if (field->cmp_max(field->ptr, field->ptr + diff,  // 3
                     std::max(value1_length, value2_length)))
    return true;

  return false;
}

/**
  Compare GROUP BY in from tmp table's record[0] and record[1]

  @returns
    true  records are different
    false records are the same
*/

static bool group_rec_cmp(ORDER *group, uchar *rec0, uchar *rec1) {
  DBUG_TRACE;
  ptrdiff_t diff = rec1 - rec0;

  for (ORDER *grp = group; grp; grp = grp->next) {
    Field *field = grp->field_in_tmp_table;
    if (cmp_field_value(field, diff)) return true;
  }
  return false;
}

/**
  Compare GROUP BY in from tmp table's record[0] and record[1]

  @returns
    true  records are different
    false records are the same
*/

static bool table_rec_cmp(TABLE *table) {
  DBUG_TRACE;
  ptrdiff_t diff = table->record[1] - table->record[0];
  Field **fields = table->visible_field_ptr();

  for (uint i = 0; i < table->visible_field_count(); i++) {
    Field *field = fields[i];
    if (cmp_field_value(field, diff)) return true;
  }
  return false;
}

/**
  Generate hash for a field

  @returns generated hash
*/

ulonglong unique_hash(const Field *field, ulonglong *hash_val) {
  const uchar *pos, *end;
  uint64 seed1 = 0, seed2 = 4;
  ulonglong crc = *hash_val;

  if (field->is_null()) {
    /*
      Change crc in a way different from an empty string or 0.
      (This is an optimisation;  The code will work even if
      this isn't done)
    */
    crc = ((crc << 8) + 511 + (crc >> (8 * sizeof(ha_checksum) - 8)));
    goto finish;
  }

  pos = field->get_ptr();
  end = pos + field->data_length();

  if (field->type() == MYSQL_TYPE_JSON) {
    const Field_json *json_field = down_cast<const Field_json *>(field);

    crc = json_field->make_hash_key(*hash_val);
  } else if (field->key_type() == HA_KEYTYPE_TEXT ||
             field->key_type() == HA_KEYTYPE_VARTEXT1 ||
             field->key_type() == HA_KEYTYPE_VARTEXT2) {
    field->charset()->coll->hash_sort(field->charset(), (const uchar *)pos,
                                      field->data_length(), &seed1, &seed2);
    crc ^= seed1;
  } else
    while (pos != end)
      crc = ((crc << 8) + (*pos++)) + (crc >> (8 * sizeof(ha_checksum) - 8));
finish:
  *hash_val = crc;
  return crc;
}

/**
  Generate hash for unique constraint according to group-by list.

  This reads the values of the GROUP BY expressions from fields so assumes
  those expressions have been computed and stored into fields of a temporary
  table; in practice this means that copy_fields() and copy_funcs() must have
  been called.
*/

static ulonglong unique_hash_group(ORDER *group) {
  DBUG_TRACE;
  ulonglong crc = 0;

  for (ORDER *ord = group; ord; ord = ord->next) {
    Field *field = ord->field_in_tmp_table;
    DBUG_ASSERT(field);
    unique_hash(field, &crc);
  }

  return crc;
}

/* Generate hash for unique_constraint for all visible fields of a table */

static ulonglong unique_hash_fields(TABLE *table) {
  ulonglong crc = 0;
  Field **fields = table->visible_field_ptr();

  for (uint i = 0; i < table->visible_field_count(); i++)
    unique_hash(fields[i], &crc);

  return crc;
}

/**
  Check unique_constraint.

  @details Calculates record's hash and checks whether the record given in
  table->record[0] is already present in the tmp table.

  @param table JOIN_TAB of tmp table to check

  @note This function assumes record[0] is already filled by the caller.
  Depending on presence of table->group, it's or full list of table's fields
  are used to calculate hash.

  @returns
    false same record was found
    true  record wasn't found
*/

bool check_unique_constraint(TABLE *table) {
  ulonglong hash;

  if (!table->hash_field) return true;

  if (table->no_keyread) return true;

  if (table->group)
    hash = unique_hash_group(table->group);
  else
    hash = unique_hash_fields(table);
  table->hash_field->store(hash, true);
  int res =
      table->file->ha_index_read_map(table->record[1], table->hash_field->ptr,
                                     HA_WHOLE_KEY, HA_READ_KEY_EXACT);
  while (!res) {
    // Check whether records are the same.
    if (!(table->group
              ? group_rec_cmp(table->group, table->record[0], table->record[1])
              : table_rec_cmp(table)))
      return false;  // skip it
    res = table->file->ha_index_next_same(table->record[1],
                                          table->hash_field->ptr, sizeof(hash));
  }
  return true;
}

/**
  Minion for reset_framing_wf_states and reset_non_framing_wf_state, q.v.

  @param func_ptr     the set of functions
  @param framing      true if we want to reset for framing window functions
*/
static inline void reset_wf_states(Func_ptr_array *func_ptr, bool framing) {
  for (auto it : *func_ptr) {
    (void)it.func()->walk(&Item::reset_wf_state, enum_walk::POSTFIX,
                          (uchar *)&framing);
  }
}
/**
  Walk the function calls and reset any framing window function's window state.

  @param func_ptr   an array of function call items which might represent
                    or contain window function calls
*/
static inline void reset_framing_wf_states(Func_ptr_array *func_ptr) {
  reset_wf_states(func_ptr, true);
}

/**
  Walk the function calls and reset any non-framing window function's window
  state.

  @param func_ptr   an array of function call items which might represent
                    or contain window function calls
 */
static inline void reset_non_framing_wf_state(Func_ptr_array *func_ptr) {
  reset_wf_states(func_ptr, false);
}

/**
  Save a window frame buffer to frame buffer temporary table.

  @param thd      The current thread
  @param w        The current window
  @param rowno    The rowno in the current partition (1-based)
*/
static bool buffer_record_somewhere(THD *thd, Window *w, int64 rowno) {
  DBUG_TRACE;
  TABLE *const t = w->frame_buffer();
  uchar *record = t->record[0];

  DBUG_ASSERT(rowno != Window::FBC_FIRST_IN_NEXT_PARTITION);
  DBUG_ASSERT(t->is_created());

  if (!t->file->inited) {
    /*
      On the frame buffer table, t->file, we do several things in the
      windowing code:
      - read a row by position,
      - read rows after that row,
      - write a row,
      - find the position of a just-written row, if it's first in partition.
      To prepare for reads, we initialize a scan once for all with
      ha_rnd_init(), with argument=true as we'll use ha_rnd_next().
      To read a row, we use ha_rnd_pos() or ha_rnd_next().
      To write, we use ha_write_row().
      To find the position of a just-written row, we are in the following
      conditions:
      - the written row is first of its partition
      - before writing it, we have processed the previous partition, and that
      process ended with a read of the previous partition's last row
      - so, before the write, the read cursor is already positioned on that
      last row.
      Then we do the write; the new row goes after the last row; then
      ha_rnd_next() reads the row after the last row, i.e. reads the written
      row. Then position() gives the position of the written row.
    */
    int rc = t->file->ha_rnd_init(true);
    if (rc != 0) {
      t->file->print_error(rc, MYF(0));
      return true;
    }
  }

  int error = t->file->ha_write_row(record);
  w->set_frame_buffer_total_rows(w->frame_buffer_total_rows() + 1);

  constexpr size_t first_in_partition = static_cast<size_t>(
      Window_retrieve_cached_row_reason::FIRST_IN_PARTITION);

  if (error) {
    /* If this is a duplicate error, return immediately */
    if (t->file->is_ignorable_error(error)) return true;

    /* Other error than duplicate error: Attempt to create a temporary table. */
    bool is_duplicate;
    if (create_ondisk_from_heap(thd, t, error, true, &is_duplicate)) return -1;

    DBUG_ASSERT(t->s->db_type() == innodb_hton);
    if (t->file->ha_rnd_init(true)) return true; /* purecov: inspected */

    /*
      Reset all hints since they all pertain to the in-memory file, not the
      new on-disk one.
    */
    for (size_t i = first_in_partition;
         i < Window::FRAME_BUFFER_POSITIONS_CARD +
                 w->opt_nth_row().m_offsets.size() +
                 w->opt_lead_lag().m_offsets.size();
         i++) {
      void *r = (*THR_MALLOC)->Alloc(t->file->ref_length);
      if (r == nullptr) return true;
      w->m_frame_buffer_positions[i].m_position = static_cast<uchar *>(r);
      w->m_frame_buffer_positions[i].m_rowno = -1;
    }

    if ((w->m_tmp_pos.m_position =
             (uchar *)(*THR_MALLOC)->Alloc(t->file->ref_length)) == nullptr)
      return true;

    w->m_frame_buffer_positions[first_in_partition].m_rowno = 1;
    /*
      The auto-generated primary key of the first row is 1. Our offset is
      also one-based, so we can use w->frame_buffer_partition_offset() "as is"
      to construct the position.
    */
    encode_innodb_position(
        w->m_frame_buffer_positions[first_in_partition].m_position,
        t->file->ref_length, w->frame_buffer_partition_offset());

    return is_duplicate ? true : false;
  }

  /* Save position in frame buffer file of first row in a partition */
  if (rowno == 1) {
    if (w->m_frame_buffer_positions.empty()) {
      w->m_frame_buffer_positions.init(thd->mem_root);
      /* lazy initialization of positions remembered */
      for (uint i = 0; i < Window::FRAME_BUFFER_POSITIONS_CARD +
                               w->opt_nth_row().m_offsets.size() +
                               w->opt_lead_lag().m_offsets.size();
           i++) {
        void *r = (*THR_MALLOC)->Alloc(t->file->ref_length);
        if (r == nullptr) return true;
        Window::Frame_buffer_position p(static_cast<uchar *>(r), -1);
        w->m_frame_buffer_positions.push_back(p);
      }

      if ((w->m_tmp_pos.m_position =
               (uchar *)(*THR_MALLOC)->Alloc(t->file->ref_length)) == nullptr)
        return true;
    }

    // Do a read to establish scan position, then get it
    error = t->file->ha_rnd_next(record);
    t->file->position(record);
    std::memcpy(w->m_frame_buffer_positions[first_in_partition].m_position,
                t->file->ref, t->file->ref_length);
    w->m_frame_buffer_positions[first_in_partition].m_rowno = 1;
    w->set_frame_buffer_partition_offset(w->frame_buffer_total_rows());
  }

  return false;
}

/**
  If we cannot evaluate all window functions for a window on the fly, buffer the
  current row for later processing by process_buffered_windowing_record.

  @param thd                Current thread
  @param param              The temporary table parameter

  @param[in,out] new_partition If input is not nullptr:
                            sets the bool pointed to to true if a new partition
                            was found and there was a previous partition; if
                            so the buffering of the first row in new
                            partition isn't done and must be repeated
                            later: we save away the row as rowno
                            FBC_FIRST_IN_NEXT_PARTITION, then fetch it back
                            later, cf. end_write_wf.
                            If input is nullptr, this is the "later" call to
                            buffer the first row of the new partition:
                            buffer the row.
  @return true if error.
*/
bool buffer_windowing_record(THD *thd, Temp_table_param *param,
                             bool *new_partition) {
  DBUG_TRACE;
  Window *w = param->m_window;

  if (copy_fields(w->frame_buffer_param(), thd)) return true;

  if (new_partition != nullptr) {
    const bool first_partition = w->partition_rowno() == 0;
    w->check_partition_boundary();

    if (!first_partition && w->partition_rowno() == 1) {
      *new_partition = true;
      w->save_special_record(Window::FBC_FIRST_IN_NEXT_PARTITION,
                             w->frame_buffer());
      return false;
    }
  }

  /*
    The record is now ready in TABLE and can be saved. The window
    function(s) on the window have not yet been evaluated, but
    will be evaluated when we read frame rows back, before the end wf result
    (usually ready in the last read when the last frame row has been read back)
    can be produced. E.g. SUM(i): we save away all rows in partition.
    We read back rows in current row's frame, producing the total SUM in the
    last read back row. That value for SUM will then be used for the current row
    output.
  */

  if (w->needs_restore_input_row()) {
    w->save_special_record(Window::FBC_LAST_BUFFERED_ROW, w->frame_buffer());
  }

  if (buffer_record_somewhere(thd, w, w->partition_rowno())) return true;

  w->set_last_rowno_in_cache(w->partition_rowno());

  return false;
}

/**
  Read row rowno from frame buffer tmp file using cached row positions to
  minimize positioning work.
*/
static bool read_frame_buffer_row(int64 rowno, Window *w,
#ifndef DBUG_OFF
                                  bool for_nth_value)
#else
                                  bool for_nth_value MY_ATTRIBUTE((unused)))
#endif
{
  int use_idx = 0;  // closest prior position found, a priori 0 (row 1)
  int diff = w->last_rowno_in_cache();  // maximum a priori
  TABLE *t = w->frame_buffer();

  // Find the saved position closest to where we want to go
  for (int i = w->m_frame_buffer_positions.size() - 1; i >= 0; i--) {
    auto cand = w->m_frame_buffer_positions[i];
    if (cand.m_rowno == -1 || cand.m_rowno > rowno) continue;

    if (rowno - cand.m_rowno < diff) {
      /* closest so far */
      diff = rowno - cand.m_rowno;
      use_idx = i;
    }
  }

  auto cand = &w->m_frame_buffer_positions[use_idx];

  int error =
      t->file->ha_rnd_pos(w->frame_buffer()->record[0], cand->m_position);
  if (error) {
    t->file->print_error(error, MYF(0));
    return true;
  }

  if (rowno > cand->m_rowno) {
    /*
      The saved position didn't correspond exactly to where we want to go, but
      is located one or more rows further out on the file, so read next to move
      forward to desired row.
    */
    const int64 cnt = rowno - cand->m_rowno;

    /*
      We should have enough location hints to normally need only one extra read.
      If we have just switched to INNODB due to MEM overflow, a rescan is
      required, so skip assert if we have INNODB.
    */
    DBUG_ASSERT(w->frame_buffer()->s->db_type()->db_type == DB_TYPE_INNODB ||
                cnt <= 1 ||
                // unless we have a frame beyond the current row, 1. time
                // in which case we need to do some scanning...
                (w->last_row_output() == 0 &&
                 w->frame()->m_from->m_border_type == WBT_VALUE_FOLLOWING) ||
                // or unless we are search for NTH_VALUE, which can be in the
                // middle of a frame, and with RANGE frames it can jump many
                // positions from one frame to the next with optimized eval
                // strategy
                for_nth_value);

    for (int i = 0; i < cnt; i++) {
      error = t->file->ha_rnd_next(t->record[0]);
      if (error) {
        t->file->print_error(error, MYF(0));
        return true;
      }
    }
  }

  return false;
}

#if !defined(DBUG_OFF)
inline static void dbug_allow_write_all_columns(
    Temp_table_param *param, std::map<TABLE *, my_bitmap_map *> &map) {
  for (auto &copy_field : param->copy_fields) {
    TABLE *const t = copy_field.from_field()->table;
    if (t != nullptr) {
      auto it = map.find(t);
      if (it == map.end())
        map.insert(it, std::pair<TABLE *, my_bitmap_map *>(
                           t, dbug_tmp_use_all_columns(t, t->write_set)));
    }
  }
}

inline static void dbug_restore_all_columns(
    std::map<TABLE *, my_bitmap_map *> &map) {
  auto func = [](std::pair<TABLE *const, my_bitmap_map *> &e) {
    dbug_tmp_restore_column_map(e.first->write_set, e.second);
  };

  std::for_each(map.begin(), map.end(), func);
}
#endif

/**
  Bring back buffered data to the record of qep_tab-1 [1], and optionally
  execute copy_fields() to the OUT table.

  [1] This is not always the case. For the first window, if we have no
  PARTITION BY or ORDER BY in the window, and there is more than one table
  in the join, the logical input can consist of more than one table
  (qep_tab-1 .. qep_tab-n), so the record accordingly.

  This method works by temporarily reversing the "normal" direction of the field
  copying.

  Also make a note of the position of the record we retrieved in the window's
  m_frame_buffer_positions to be able to optimize succeeding retrievals.

  @param thd       The current thread
  @param w         The current window
  @param out_param OUT table; if not nullptr, does copy_fields() to OUT
  @param rowno     The row number (in the partition) to set up
  @param reason    What kind of row to retrieve
  @param fno       Used with NTH_VALUE and LEAD/LAG to specify which
                   window function's position cache to use, i.e. what index
                   of m_frame_buffer_positions to update. For the second
                   LEAD/LAG window function in a query, the index would be
                   REA_MISC_POSITIONS (reason) + \<no of NTH functions\> + 2.

  @return true on error
*/
bool bring_back_frame_row(THD *thd, Window *w, Temp_table_param *out_param,
                          int64 rowno, Window_retrieve_cached_row_reason reason,
                          int fno) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("rowno: %" PRId64 " reason: %d fno: %d", rowno,
                       static_cast<int>(reason), fno));
  DBUG_ASSERT(reason == Window_retrieve_cached_row_reason::MISC_POSITIONS ||
              fno == 0);

  uchar *fb_rec = w->frame_buffer()->record[0];

  DBUG_ASSERT(rowno != 0);

  /*
    If requested row is the last we fetched from FB and copied to OUT, we
    don't need to fetch and copy again.
    Because "reason", "fno" may differ from the last call which fetched the
    row, we still do the updates of w.m_frame_buffer_positions even if
    do_fetch=false.
  */
  bool do_fetch;

  if (rowno == Window::FBC_FIRST_IN_NEXT_PARTITION) {
    do_fetch = true;
    w->restore_special_record(rowno, fb_rec);
  } else if (rowno == Window::FBC_LAST_BUFFERED_ROW) {
    do_fetch = w->row_has_fields_in_out_table() != w->last_rowno_in_cache();
    if (do_fetch) w->restore_special_record(rowno, fb_rec);
  } else {
    DBUG_ASSERT(reason != Window_retrieve_cached_row_reason::WONT_UPDATE_HINT);
    do_fetch = w->row_has_fields_in_out_table() != rowno;

    if (do_fetch &&
        read_frame_buffer_row(
            rowno, w,
            reason == Window_retrieve_cached_row_reason::MISC_POSITIONS))
      return true;

    /* Got row rowno in record[0], remember position */
    const TABLE *const t = w->frame_buffer();
    t->file->position(fb_rec);
    std::memcpy(
        w->m_frame_buffer_positions[static_cast<int>(reason) + fno].m_position,
        t->file->ref, t->file->ref_length);
    w->m_frame_buffer_positions[static_cast<int>(reason) + fno].m_rowno = rowno;
  }

  if (!do_fetch) return false;

  Temp_table_param *const fb_info = w->frame_buffer_param();

#if !defined(DBUG_OFF)
  /*
    Since we are copying back a row from the frame buffer to the input table's
    buffer, we will be copying into fields that are not necessarily marked as
    writeable. To eliminate problems with ASSERT_COLUMN_MARKED_FOR_WRITE, we
    set all fields writeable. This is only
    applicable in debug builds, since ASSERT_COLUMN_MARKED_FOR_WRITE is debug
    only.
  */
  std::map<TABLE *, my_bitmap_map *> saved_map;
  dbug_allow_write_all_columns(fb_info, saved_map);
#endif

  /*
    Do the inverse of copy_fields to get the row's fields back to the input
    table from the frame buffer.
  */
  bool rc = copy_fields(fb_info, thd, true);

#if !defined(DBUG_OFF)
  dbug_restore_all_columns(saved_map);
#endif

  if (!rc) {
    if (out_param) {
      if (copy_fields(out_param, thd)) return true;
      // fields are in IN and in OUT
      if (rowno >= 1) w->set_row_has_fields_in_out_table(rowno);
    } else
      // we only wrote IN record, so OUT and IN are inconsistent
      w->set_row_has_fields_in_out_table(0);
  }

  return rc;
}

/**
  Save row special_rowno in table t->record[0] to an in-memory copy for later
  restoration.
*/
void Window::save_special_record(uint64 special_rowno, TABLE *t) {
  DBUG_PRINT("info", ("save_special_record: %" PRIu64, special_rowno));
  size_t l = t->s->reclength;
  DBUG_ASSERT(m_special_rows_cache_max_length >= l);  // check room.
  // From negative enum, get proper array index:
  int idx = FBC_FIRST_KEY - special_rowno;
  m_special_rows_cache_length[idx] = l;
  std::memcpy(m_special_rows_cache + idx * m_special_rows_cache_max_length,
              t->record[0], l);
}

/**
  Restore row special_rowno into record from in-memory copy. Any fields not
  the result of window functions are not used, but they do tag along here
  (unnecessary copying..). BLOBs: have storage in result_field of Item
  for the window function although the pointer is copied here. The
  result field storage is stable across reads from the frame buffer, so safe.
*/
void Window::restore_special_record(uint64 special_rowno, uchar *record) {
  DBUG_PRINT("info", ("restore_special_record: %" PRIu64, special_rowno));
  int idx = FBC_FIRST_KEY - special_rowno;
  size_t l = m_special_rows_cache_length[idx];
  std::memcpy(record,
              m_special_rows_cache + idx * m_special_rows_cache_max_length, l);
  // Sometimes, "record" points to IN record
  set_row_has_fields_in_out_table(0);
}

/**
  Process window functions that need partition cardinality
*/
static bool process_wfs_needing_card(
    THD *thd, Temp_table_param *param, const Window::st_nth &have_nth_value,
    const Window::st_lead_lag &have_lead_lag, const int64 current_row,
    Window *w, Window_retrieve_cached_row_reason current_row_reason) {
  w->set_rowno_being_visited(current_row);

  // Reset state for LEAD/LAG functions
  if (!have_lead_lag.m_offsets.empty()) w->reset_lead_lag();

  // This also handles LEAD(.., 0)
  if (copy_funcs(param, thd, CFT_WF_NEEDS_CARD)) return true;

  if (!have_lead_lag.m_offsets.empty()) {
    int fno = 0;
    const int nths = have_nth_value.m_offsets.size();

    for (auto &ll : have_lead_lag.m_offsets) {
      const int64 rowno_to_visit = current_row - ll.m_rowno;

      if (rowno_to_visit == current_row)
        continue;  // Already processed above above

      /*
        Note that this value can be outside partition, even negative: if so,
        the default will applied, if any is provided.
      */
      w->set_rowno_being_visited(rowno_to_visit);

      if (rowno_to_visit >= 1 && rowno_to_visit <= w->last_rowno_in_cache()) {
        if (bring_back_frame_row(
                thd, w, param, rowno_to_visit,
                Window_retrieve_cached_row_reason::MISC_POSITIONS,
                nths + fno++))
          return true;
      }

      if (copy_funcs(param, thd, CFT_WF_NEEDS_CARD)) return true;
    }
    /* Bring back the fields for the output row */
    if (bring_back_frame_row(thd, w, param, current_row, current_row_reason))
      return true;
  }

  return false;
}

/**
  While there are more unprocessed rows ready to process given the current
  partition/frame state, process such buffered rows by evaluating/aggregating
  the window functions defined over this window on the current frame, moving
  the frame if required.

  This method contains the main execution time logic of the evaluation
  window functions if we need buffering for one or more of the window functions
  defined on the window.

  Moving (sliding) frames can be executed using a naive or optimized strategy
  for aggregate window functions, like SUM or AVG (but not MAX, or MIN).
  In the naive approach, for each row considered for processing from the buffer,
  we visit all the rows defined in the frame for that row, essentially leading
  to N*M complexity, where N is the number of rows in the result set, and M is
  the number for rows in the frame. This can be slow for large frames,
  obviously, so we can choose an optimized evaluation strategy using inversion.
  This means that when rows leave the frame as we move it forward, we re-use
  the previous aggregate state, but compute the *inverse* function to eliminate
  the contribution to the aggregate by the row(s) leaving the frame, and then
  use the normal aggregate function to add the contribution of the rows moving
  into the frame. The present method contains code paths for both strategies.

  For integral data types, this is safe in the sense that the result will be the
  same if no overflow occurs during normal evaluation. For floating numbers,
  optimizing in this way may lead to different results, so it is not done by
  default, cf the session variable "windowing_use_high_precision".

  Since the evaluation strategy is chosen based on the "most difficult" window
  function defined on the window, we must also be able to evaluate
  non-aggregates like ROW_NUMBER, NTILE, FIRST_VALUE in the code path of the
  optimized aggregates, so there is redundant code for those in the naive and
  optimized code paths. Note that NTILE forms a class of its own of the
  non-aggregates: it needs two passes over the partition's rows since the
  cardinality is needed to compute it. Furthermore, FIRST_VALUE and LAST_VALUE
  heed the frames, but they are not aggregates.

  The is a special optimized code path for *static aggregates*: when the window
  frame is the default, e.g. the entire partition and there is no ORDER BY
  specified, the value of the framing window functions, i.e. SUM, AVG,
  FIRST_VALUE, LAST_VALUE can be evaluated once and for all and saved when
  we visit and evaluate the first row of the partition. For later rows we
  restore the aggregate values and just fill in the other fields and evaluate
  non-framing window functions for the row.

  The code paths both for naive execution and optimized execution differ
  depending on whether we have ROW or RANGE boundaries in a explicit frame.

  A word on BLOBs. Below we make copies of rows into the frame buffer.
  This is a temporary table, so BLOBs get copied in the normal way.

  Sometimes we save records containing already computed framing window
  functions away into memory only: is the lifetime of the referenced BLOBs long
  enough? We have two cases:

  BLOB results from wfs: Any BLOB results will reside in the copies in result
  fields of the Items ready for the out file, so they no longer need any BLOB
  memory read from the frame buffer tmp file.

  BLOB fields not evaluated by wfs: Any other BLOB field will be copied as
  well, and would not have life-time past the next read from the frame buffer,
  but they are never used since we fill in the fields from the current row
  after evaluation of the window functions, so we don't need to make special
  copies of such BLOBs. This can be (and was) tested by shredding any BLOBs
  deallocated by InnoDB at the next read.

  We also save away in memory the next record of the next partition while
  processing the current partition. Any blob there will have its storage from
  the read of the input file, but we won't be touching that for reading again
  until after we start processing the next partition and save the saved away
  next partition row to the frame buffer.

  Note that the logic of this function is centered around the window, not
  around the window function. It is about putting rows in a partition,
  in a frame, in a set of peers, and passing this information to all window
  functions attached to this window; each function looks at the partition,
  frame, or peer set in its own particular way (for example RANK looks at the
  partition, SUM looks at the frame).

  @param thd                    Current thread
  @param param                  Current temporary table
  @param new_partition_or_eof   True if (we are about to start a new partition
                                and there was a previous partition) or eof
  @param[out] output_row_ready  True if there is a row record ready to write
                                to the out table

  @return true if error
*/
bool process_buffered_windowing_record(THD *thd, Temp_table_param *param,
                                       const bool new_partition_or_eof,
                                       bool *output_row_ready) {
  DBUG_TRACE;
  /**
    The current window
  */
  Window &w = *param->m_window;

  /**
    The frame
  */
  const PT_frame *f = w.frame();

  *output_row_ready = false;

  /**
    This is the row we are currently considering for processing and getting
    ready for output, cf. output_row_ready.
  */
  const int64 current_row = w.last_row_output() + 1;

  /**
    This is the row number of the last row we have buffered so far.
  */
  const int64 last_rowno_in_cache = w.last_rowno_in_cache();

  if (current_row > last_rowno_in_cache)  // already sent all buffered rows
    return false;

  /**
    If true, use code path for static aggregates
  */
  const bool static_aggregate = w.static_aggregates();

  /**
    If true, use code path for ROW bounds with optimized strategy
  */
  const bool row_optimizable = w.optimizable_row_aggregates();

  /**
    If true, use code path for RANGE bounds with optimized strategy
  */
  const bool range_optimizable = w.optimizable_range_aggregates();

  // These three strategies are mutually exclusive:
  DBUG_ASSERT((static_aggregate + row_optimizable + range_optimizable) <= 1);

  /**
    We need to evaluate FIRST_VALUE, or optimized MIN/MAX
  */
  const bool have_first_value = w.opt_first_row();

  /**
    We need to evaluate LAST_VALUE, or optimized MIN/MAX
  */
  const bool have_last_value = w.opt_last_row();

  /**
    We need to evaluate NTH_VALUE
  */
  const Window::st_nth &have_nth_value = w.opt_nth_row();

  /**
    We need to evaluate LEAD/LAG rows
  */

  const Window::st_lead_lag &have_lead_lag = w.opt_lead_lag();

  /**
    True if an inversion optimization strategy is used. For common
    code paths.
  */
  const bool optimizable = (row_optimizable || range_optimizable);

  /**
    RANGE was specified as the bounds unit for the frame
  */
  const bool range_frame = f->m_unit == WFU_RANGE;

  const bool range_to_current_row =
      range_frame && f->m_to->m_border_type == WBT_CURRENT_ROW;

  const bool range_from_first_to_current_row =
      range_to_current_row &&
      f->m_from->m_border_type == WBT_UNBOUNDED_PRECEDING;
  /**
    UNBOUNDED FOLLOWING was specified for the frame
  */
  bool unbounded_following = false;

  /**
    Row_number of the first row in the frame. Invariant: lower_limit >= 1
    after initialization.
  */
  int64 lower_limit = 1;

  /**
    Row_number of the logically last row to be computed in the frame, may be
    higher than the number of rows in the partition. The actual highest row
    number is computed later, see upper below.
  */
  int64 upper_limit = 0;

  /**
    needs peerset of current row to evaluate a wf for the current row.
  */
  bool needs_peerset = w.needs_peerset();

  /**
    needs the last peer of the current row within a frame.
  */
  const bool needs_last_peer_in_frame = w.needs_last_peer_in_frame();

  DBUG_PRINT("enter", ("current_row: %" PRId64 ", new_partition_or_eof: %d",
                       current_row, new_partition_or_eof));

  /* Compute lower_limit, upper_limit and possibly unbounded_following */
  if (f->m_unit == WFU_RANGE) {
    lower_limit = w.first_rowno_in_range_frame();
    /*
      For RANGE frame, we first buffer all the rows in the partition due to the
      need to find last peer before first can be processed. This can be
      optimized,
      FIXME.
    */
    upper_limit = INT64_MAX;
  } else {
    DBUG_ASSERT(f->m_unit == WFU_ROWS);
    bool lower_within_limits = true;
    /* Determine lower border */
    int64 border =
        f->m_from->border() != nullptr ? f->m_from->border()->val_int() : 0;
    switch (f->m_from->m_border_type) {
      case WBT_CURRENT_ROW:
        lower_limit = current_row;
        break;
      case WBT_VALUE_PRECEDING:
        /*
          Example: 1 PRECEDING and current row== 2 => 1
                                   current row== 1 => 1
                                   current row== 3 => 2
        */
        lower_limit = std::max<int64>(current_row - border, 1);
        break;
      case WBT_VALUE_FOLLOWING:
        /*
          Example: 1 FOLLOWING and current row== 2 => 3
                                   current row== 1 => 2
                                   current row== 3 => 4
        */
        if (border <= (std::numeric_limits<int64>::max() - current_row))
          lower_limit = current_row + border;
        else {
          lower_within_limits = false;
          lower_limit = INT64_MAX;
        }
        break;
      case WBT_UNBOUNDED_PRECEDING:
        lower_limit = 1;
        break;
      case WBT_UNBOUNDED_FOLLOWING:
        DBUG_ASSERT(false);
        break;
    }

    /* Determine upper border */
    border = f->m_to->border() != nullptr ? f->m_to->border()->val_int() : 0;
    {
      switch (f->m_to->m_border_type) {
        case WBT_CURRENT_ROW:
          // we always have enough cache
          upper_limit = current_row;
          break;
        case WBT_VALUE_PRECEDING:
          upper_limit = current_row - border;
          break;
        case WBT_VALUE_FOLLOWING:
          if (border <= (std::numeric_limits<longlong>::max() - current_row))
            upper_limit = current_row + border;
          else {
            upper_limit = INT64_MAX;
            /*
              If both the border specifications are beyond numeric limits,
              the window frame is empty.
            */
            if (f->m_from->m_border_type == WBT_VALUE_FOLLOWING &&
                !lower_within_limits) {
              lower_limit = INT64_MAX;
              upper_limit = INT64_MAX - 1;
            }
          }
          break;
        case WBT_UNBOUNDED_FOLLOWING:
          unbounded_following = true;
          upper_limit = INT64_MAX;  // need whole partition
          break;
        case WBT_UNBOUNDED_PRECEDING:
          DBUG_ASSERT(false);
          break;
      }
    }
  }

  /*
    Determine if, given our current read and buffering state, we have enough
    buffered rows to compute an output row.

    Example: ROWS BETWEEN 1 PRECEDING and 3 FOLLOWING

    State:
    +---+-------------------------------+
    |   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
    +---+-------------------------------+
    ^    1?         ^
    lower      last_rowno_in_cache
    (0)             (4)

    This state means:

    We have read 4 rows, cf. value of last_rowno_in_cache.
    We can now process row 1 since both lower (1-1=0) and upper (1+3=4) are less
    than or equal to 4, the last row in the cache so far.

    We can not process row 2 since: !(4 >= 2 + 3) and we haven't seen the last
    row in partition which means that the frame may not be full yet.

    If we have a window function that needs to know the partition cardinality,
    we also must buffer all records of the partition before processing.
  */

  if (!((lower_limit <= last_rowno_in_cache &&
         upper_limit <= last_rowno_in_cache &&
         !w.needs_card()) || /* we have cached enough rows */
        new_partition_or_eof /* we have cached all rows */))
    return false;  // We haven't read enough rows yet, so return

  w.set_rowno_in_partition(current_row);

  /*
    By default, we must:
    - if we are the first row of a partition, reset values for both
    non-framing and framing WFs
    - reset values for framing WFs (new current row = new frame = new
    values for WFs).

    Both resettings require restoring the row from the FB. And, as we have
    restored this row, we use this opportunity to compute non-framing
    does-not-need-card functions.

    The meaning of if statements below is that in some cases, we can avoid
    this default behaviour.

    For example, if we have static framing WFs, and this is not the
    partition's first row: the previous row's framing-WF values should be
    reused without change, so all the above resetting must be skipped;
    so row restoration isn't immediately needed; that and the computation of
    non-framing functions is then done in another later block of code.
    Likewise, if we have framing WFs with inversion, and it's not the
    first row of the partition, we must skip the resetting of framing WFs.
  */
  if (!static_aggregate || current_row == 1) {
    /*
      We need to reset functions. As part of it, their comparators need to
      update themselves to use the new row as base line. So, restore it:
    */
    if (bring_back_frame_row(thd, &w, param, current_row,
                             Window_retrieve_cached_row_reason::CURRENT))
      return true;

    if (current_row == 1)  // new partition
      reset_non_framing_wf_state(param->items_to_copy);
    if (!optimizable || current_row == 1)  // new frame
    {
      reset_framing_wf_states(param->items_to_copy);
    }  // else we remember state and update it for row 2..N

    /* E.g. ROW_NUMBER, RANK, DENSE_RANK */
    if (copy_funcs(param, thd, CFT_WF_NON_FRAMING)) return true;
    if (!optimizable || current_row == 1) {
      /*
        So far frame is empty; set up a flag which makes framing WFs set
        themselves to NULL in OUT.
      */
      w.set_do_copy_null(true);
      if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;
      w.set_do_copy_null(false);
    }  // else aggregates keep value of previous row, and we'll do inversion
  }

  if (range_frame) {
    /* establish current row as base-line for RANGE computation */
    w.reset_order_by_peer_set();
  }

  bool first_row_in_range_frame_seen = false;

  /**
    For optimized strategy we want to save away the previous aggregate result
    and reuse in later round by inversion. This keeps track of whether we
    managed to compute results for this current row (result are "primed"), so we
    can use inversion in later rows. Cf Window::m_aggregates_primed.
  */
  bool optimizable_primed = false;

  /**
    Possible adjustment of the logical upper_limit: no rows exist beyond
    last_rowno_in_cache.
  */
  const int64 upper = min(upper_limit, last_rowno_in_cache);

  /*
    Optimization: we evaluate the peer set of the current row potentially
    several times. Window functions like CUME_DIST sets needs_peerset and is
    evaluated last, so if any other wf evaluation led to finding the peer set
    of the current row, make a note of it, so we can skip doing it twice.
  */
  bool have_peers_current_row = false;

  if ((static_aggregate && current_row == 1) ||   // skip for row > 1
      (optimizable && !w.aggregates_primed()) ||  // skip for 2..N in frame
      (!static_aggregate && !optimizable))        // normal: no skip
  {
    // Compute and output current_row.
    int64 rowno;        ///< iterates over rows in a frame
    int64 skipped = 0;  ///< RANGE: # of visited rows seen before the frame

    for (rowno = lower_limit; rowno <= upper; rowno++) {
      if (optimizable) optimizable_primed = true;

      /*
        Set window frame state before computing framing window function.
        'n' is the number of row #rowno relative to the beginning of the
        frame, 1-based.
      */
      const int64 n = rowno - lower_limit + 1 - skipped;

      w.set_rowno_in_frame(n);
      w.set_rowno_being_visited(rowno);

      const Window_retrieve_cached_row_reason reason =
          (n == 1 ? Window_retrieve_cached_row_reason::FIRST_IN_FRAME
                  : Window_retrieve_cached_row_reason::LAST_IN_FRAME);
      /*
        Hint maintenance: we will normally read past last row in frame, so
        prepare to resurrect that hint once we do.
      */
      w.save_pos(reason);

      /* Set up the non-wf fields for aggregating to the output row. */
      if (bring_back_frame_row(thd, &w, param, rowno, reason)) return true;

      if (range_frame) {
        if (w.before_frame()) {
          skipped++;
          continue;
        }
        if (w.after_frame()) {
          w.set_last_rowno_in_range_frame(rowno - 1);

          if (!first_row_in_range_frame_seen)
            // empty frame, optimize starting point for next row
            w.set_first_rowno_in_range_frame(rowno);
          w.restore_pos(reason);
          break;
        }  // else: row is within range, process

        if (!first_row_in_range_frame_seen) {
          /*
            Optimize starting point for next row: monotonic increase in frame
            bounds
          */
          first_row_in_range_frame_seen = true;
          w.set_first_rowno_in_range_frame(rowno);
        }
      }

      /*
        Compute framing WFs. For ROWS frame, "upper" is exactly the frame's
        last row; but for the case of RANGE
        we can't be sure that this is indeed the last row, but we must make a
        pessimistic assumption. If it is not the last, the final row
        calculation, if any, as for AVG, will be repeated for the next peer
        row(s).
        For optimized MIN/MAX [1], we do this to make sure we have a non-NULL
        last value (if one exists) for the initial frame.
      */
      const bool setstate =
          (rowno == upper || range_frame || have_last_value /* [1] */);
      if (setstate)
        w.set_is_last_row_in_frame(true);  // temporary state for next call

      // Accumulate frame's row into WF's value for current_row:
      if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;

      if (setstate) w.set_is_last_row_in_frame(false);  // undo temporary state
    }

    if (range_frame || rowno > upper)  // no more rows in partition
    {
      if (range_frame) {
        if (!first_row_in_range_frame_seen) {
          /*
            Empty frame: optimize starting point for next row: monotonic
            increase in frame bounds
          */
          w.set_first_rowno_in_range_frame(rowno);
        }
      }
      w.set_last_rowno_in_range_frame(rowno - 1);
      if (range_to_current_row) {
        w.set_last_rowno_in_peerset(w.last_rowno_in_range_frame());
        have_peers_current_row = true;
      }
    }  // else: we already set it before breaking out of loop
  }

  /*
    While the block above was for the default execution method, below we have
    alternative blocks for optimized methods: static framing WFs and
    inversion, when current_row isn't first; i.e. we can use the previous
    row's value of framing WFs as a base.
    In the row buffer of OUT, after the previous row was emitted, these values
    of framing WFs are still present, as no copy_funcs(CFT_WF_FRAMING) was run
    for our new row yet.
  */
  if (static_aggregate && current_row != 1) {
    /* Set up the correct non-wf fields for copying to the output row */
    if (bring_back_frame_row(thd, &w, param, current_row,
                             Window_retrieve_cached_row_reason::CURRENT))
      return true;

    /* E.g. ROW_NUMBER, RANK, DENSE_RANK */
    if (copy_funcs(param, thd, CFT_WF_NON_FRAMING)) return true;
  } else if (row_optimizable && w.aggregates_primed()) {
    /*
      Rows 2..N in partition: we still have state from previous current row's
      frame computation, now adjust by subtracting row 1 in frame (lower_limit)
      and adding new, if any, final frame row
    */
    const bool remove_previous_first_row =
        (lower_limit > 1 && lower_limit - 1 <= last_rowno_in_cache);
    const bool new_last_row =
        (upper_limit <= upper &&
         !unbounded_following /* all added when primed */);
    const int64 rn_in_frame = upper - lower_limit + 1;

    /* possibly subtract: early in partition there may not be any */
    if (remove_previous_first_row) {
      /*
        Check if the row leaving the frame is the last row in the peerset
        within a frame. If true, set is_last_row_in_peerset_within_frame
        to true.
        Used by JSON_OBJECTAGG to remove the key/value pair only
        when it is the last row having that key value.
      */
      if (needs_last_peer_in_frame) {
        int64 rowno = lower_limit - 1;
        bool is_last_row_in_peerset = true;
        if (rowno < upper) {
          if (bring_back_frame_row(
                  thd, &w, param, rowno,
                  Window_retrieve_cached_row_reason::LAST_IN_PEERSET))
            return true;
          // Establish current row as base-line for peer set.
          w.reset_order_by_peer_set();
          /*
            Check if the next row is a peer to this row. If not
            set current row as the last row in peerset within
            frame.
          */
          rowno++;
          if (rowno < upper) {
            if (bring_back_frame_row(
                    thd, &w, param, rowno,
                    Window_retrieve_cached_row_reason::LAST_IN_PEERSET))
              return true;
            // Compare only the first order by item.
            if (!w.in_new_order_by_peer_set(false))
              is_last_row_in_peerset = false;
          }
        }
        if (is_last_row_in_peerset)
          w.set_is_last_row_in_peerset_within_frame(true);
      }

      if (bring_back_frame_row(
              thd, &w, param, lower_limit - 1,
              Window_retrieve_cached_row_reason::FIRST_IN_FRAME))
        return true;

      w.set_inverse(true);
      if (!new_last_row) {
        w.set_rowno_in_frame(rn_in_frame);
        if (rn_in_frame > 0)
          w.set_is_last_row_in_frame(true);  // do final comp., e.g. div in AVG

        if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;

        w.set_is_last_row_in_frame(false);  // undo temporary states
      } else {
        if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;
      }

      w.set_is_last_row_in_peerset_within_frame(false);
      w.set_inverse(false);
    }

    if (have_first_value && (lower_limit <= last_rowno_in_cache)) {
      // We have seen first row of frame, FIRST_VALUE can be computed:
      if (bring_back_frame_row(
              thd, &w, param, lower_limit,
              Window_retrieve_cached_row_reason::FIRST_IN_FRAME))
        return true;

      w.set_rowno_in_frame(1);

      /*
        Framing WFs which accumulate (SUM, COUNT, AVG) shouldn't accumulate
        this row again as they have done so already. Evaluate only
        X_VALUE/MIN/MAX.
      */
      if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;
    }

    if (have_last_value && !new_last_row) {
      // We have seen last row of frame, LAST_VALUE can be computed:
      if (bring_back_frame_row(
              thd, &w, param, upper,
              Window_retrieve_cached_row_reason::LAST_IN_FRAME))
        return true;

      w.set_rowno_in_frame(rn_in_frame);

      if (rn_in_frame > 0) w.set_is_last_row_in_frame(true);

      if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;

      w.set_is_last_row_in_frame(false);
    }

    if (!have_nth_value.m_offsets.empty()) {
      int fno = 0;
      for (auto nth : have_nth_value.m_offsets) {
        if (lower_limit + nth.m_rowno - 1 <= upper) {
          if (bring_back_frame_row(
                  thd, &w, param, lower_limit + nth.m_rowno - 1,
                  Window_retrieve_cached_row_reason::MISC_POSITIONS, fno++))
            return true;

          w.set_rowno_in_frame(nth.m_rowno);

          if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;
        }
      }
    }

    if (new_last_row)  // Add new last row to framing WF's value
    {
      if (bring_back_frame_row(
              thd, &w, param, upper,
              Window_retrieve_cached_row_reason::LAST_IN_FRAME))
        return true;

      w.set_rowno_in_frame(upper - lower_limit + 1)
          .set_is_last_row_in_frame(true);  // temporary states for next copy
      w.set_rowno_being_visited(upper);

      if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;

      w.set_is_last_row_in_frame(false);  // undo temporary states
    }
  } else if (range_optimizable && w.aggregates_primed()) {
    /*
      Peer sets 2..N in partition: we still have state from previous current
      row's frame computation, now adjust by possibly subtracting rows no
      longer in frame and possibly adding new rows now within range.
    */
    const int64 prev_last_rowno_in_frame = w.last_rowno_in_range_frame();
    const int64 prev_first_rowno_in_frame = w.first_rowno_in_range_frame();

    /*
      As an optimization, if:
      - RANGE frame specification ends at CURRENT ROW and
      - current_row belongs to frame of previous row,
      then both rows are peers, so have the same frame: nothing changes.
    */
    if (range_to_current_row && current_row >= prev_first_rowno_in_frame &&
        current_row <= prev_last_rowno_in_frame) {
      // Peer set should already have been determined:
      DBUG_ASSERT(w.last_rowno_in_peerset() >= current_row);
      have_peers_current_row = true;
    } else {
      /**
         Whether we know the start of the frame yet. The a priori setting is
         inherited from the previous current row.
      */
      bool found_first =
          (prev_first_rowno_in_frame <= prev_last_rowno_in_frame);
      int64 new_first_rowno_in_frame = prev_first_rowno_in_frame;  // a priori

      int64 inverted = 0;  // Number of rows inverted when moving frame
      int64 rowno;         // Partition relative, loop counter

      if (range_from_first_to_current_row) {
        /*
          No need to locate frame's start, it's first row of partition. No
          need to recompute FIRST_VALUE, it's same as for previous row.
          So we just have to accumulate new rows.
        */
        DBUG_ASSERT(current_row > prev_last_rowno_in_frame &&
                    lower_limit == 1 && prev_first_rowno_in_frame == 1 &&
                    found_first);
      } else {
        for (rowno = lower_limit;
             (rowno <= upper &&
              prev_first_rowno_in_frame <= prev_last_rowno_in_frame);
             rowno++) {
          /* Set up the non-wf fields for aggregating to the output row. */
          if (bring_back_frame_row(
                  thd, &w, param, rowno,
                  Window_retrieve_cached_row_reason::FIRST_IN_FRAME))
            return true;

          if (w.before_frame()) {
            w.set_inverse(true)
                .
                /*
                  The next setting sets the logical last row number in the frame
                  after inversion, so that final actions can do the right thing,
                  e.g.  AVG needs to know the updated cardinality. The
                  aggregates consults m_rowno_in_frame for that, so set it
                  accordingly.
                */
                set_rowno_in_frame(prev_last_rowno_in_frame -
                                   prev_first_rowno_in_frame + 1 - ++inverted)
                .set_is_last_row_in_frame(true);  // pessimistic assumption

            // Set the current row as the last row in the peerset.
            w.set_is_last_row_in_peerset_within_frame(true);

            /*
              It may be that rowno is not in previous frame; for example if
              column id contains 1, 3, 4 and 5 and frame is RANGE BETWEEN 2
              FOLLOWING AND 2 FOLLOWING: we process id=1, frame of id=1 is
              id=3; then we process id=3: id=3 is before frame (and was in
              previous frame), id=4 is before frame too (and was not in
              previous frame); so id=3 only should be inverted:
            */
            if (rowno >= prev_first_rowno_in_frame &&
                rowno <= prev_last_rowno_in_frame) {
              if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;
            }

            w.set_inverse(false).set_is_last_row_in_frame(false);
            w.set_is_last_row_in_peerset_within_frame(false);
            found_first = false;
          } else {
            if (w.after_frame()) {
              found_first = false;
            } else {
              w.set_first_rowno_in_range_frame(rowno);
              found_first = true;
              new_first_rowno_in_frame = rowno;
              w.set_rowno_in_frame(1);
            }

            break;
          }
        }

        if ((have_first_value || have_last_value) &&
            (rowno <= last_rowno_in_cache) && found_first) {
          /*
             We have FIRST_VALUE or LAST_VALUE and have a new first row; make it
             last also until we find something better.
          */
          w.set_is_last_row_in_frame(true);
          w.set_rowno_being_visited(rowno);

          if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;
          w.set_is_last_row_in_frame(false);

          if (have_last_value && w.last_rowno_in_range_frame() > rowno) {
            /* Set up the non-wf fields for aggregating to the output row. */
            if (bring_back_frame_row(
                    thd, &w, param, w.last_rowno_in_range_frame(),
                    Window_retrieve_cached_row_reason::LAST_IN_FRAME))
              return true;

            w.set_rowno_in_frame(w.last_rowno_in_range_frame() -
                                 w.first_rowno_in_range_frame() + 1)
                .set_is_last_row_in_frame(true);
            w.set_rowno_being_visited(w.last_rowno_in_range_frame());
            if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;
            w.set_is_last_row_in_frame(false);
          }
        }
      }

      /*
        We last evaluated last_rowno_in_range_frame for the previous current
        row. Now evaluate over any new rows within range of the current row.
      */
      const int64 first = w.last_rowno_in_range_frame() + 1;
      bool row_added = false;

      for (rowno = first; rowno <= upper; rowno++) {
        w.save_pos(Window_retrieve_cached_row_reason::LAST_IN_FRAME);
        if (bring_back_frame_row(
                thd, &w, param, rowno,
                Window_retrieve_cached_row_reason::LAST_IN_FRAME))
          return true;

        if (w.before_frame()) {
          if (!found_first) new_first_rowno_in_frame++;
          continue;
        } else if (w.after_frame()) {
          w.set_last_rowno_in_range_frame(rowno - 1);
          if (!found_first) w.set_first_rowno_in_range_frame(rowno);
          /*
            We read one row too far, so reinstate previous hint for last in
            frame. We will likely be reading the last row in frame
            again in for next current row, and then we will need the hint.
          */
          w.restore_pos(Window_retrieve_cached_row_reason::LAST_IN_FRAME);
          break;
        }  // else: row is within range, process

        const int64 rowno_in_frame = rowno - new_first_rowno_in_frame + 1;

        if (rowno_in_frame == 1 && !found_first) {
          found_first = true;
          w.set_first_rowno_in_range_frame(rowno);
          // Found the first row in this range frame. Make a note in the hint.
          w.copy_pos(Window_retrieve_cached_row_reason::LAST_IN_FRAME,
                     Window_retrieve_cached_row_reason::FIRST_IN_FRAME);
        }
        w.set_rowno_in_frame(rowno_in_frame)
            .set_is_last_row_in_frame(true);  // pessimistic assumption
        w.set_rowno_being_visited(rowno);

        if (copy_funcs(param, thd, CFT_WF_FRAMING)) return true;

        w.set_is_last_row_in_frame(false);  // undo temporary states
        row_added = true;
      }

      if (rowno > upper && row_added)
        w.set_last_rowno_in_range_frame(rowno - 1);

      if (range_to_current_row) {
        w.set_last_rowno_in_peerset(w.last_rowno_in_range_frame());
        have_peers_current_row = true;
      }

      if (found_first && !have_nth_value.m_offsets.empty()) {
        // frame is non-empty, so we might find NTH_VALUE
        DBUG_ASSERT(w.first_rowno_in_range_frame() <=
                    w.last_rowno_in_range_frame());
        int fno = 0;
        for (auto nth : have_nth_value.m_offsets) {
          const int64 row_to_get =
              w.first_rowno_in_range_frame() + nth.m_rowno - 1;
          if (row_to_get <= w.last_rowno_in_range_frame()) {
            if (bring_back_frame_row(
                    thd, &w, param, row_to_get,
                    Window_retrieve_cached_row_reason::MISC_POSITIONS, fno++))
              return true;

            w.set_rowno_in_frame(nth.m_rowno);

            if (copy_funcs(param, thd, CFT_WF_USES_ONLY_ONE_ROW)) return true;
          }
        }
      }

      // We have empty frame, maintain invariant
      if (!found_first) {
        DBUG_ASSERT(!row_added);
        w.set_first_rowno_in_range_frame(w.last_rowno_in_range_frame() + 1);
      }
    }
  }

  /* We need the peer of the current row to evaluate the row. */
  if (needs_peerset && !have_peers_current_row) {
    int64 first = current_row;

    if (current_row != 1) first = w.last_rowno_in_peerset() + 1;

    if (current_row >= first) {
      int64 rowno;
      for (rowno = current_row; rowno <= last_rowno_in_cache; rowno++) {
        if (bring_back_frame_row(
                thd, &w, param, rowno,
                Window_retrieve_cached_row_reason::LAST_IN_PEERSET))
          return true;

        if (rowno == current_row) {
          /* establish current row as base-line for peer set */
          w.reset_order_by_peer_set();
          w.set_last_rowno_in_peerset(current_row);
        } else if (w.in_new_order_by_peer_set()) {
          w.set_last_rowno_in_peerset(rowno - 1);
          break;  // we have accumulated all rows in the peer set
        }
      }
      if (rowno > last_rowno_in_cache)
        w.set_last_rowno_in_peerset(last_rowno_in_cache);
    }
  }

  if (optimizable && optimizable_primed) w.set_aggregates_primed(true);

  if (bring_back_frame_row(thd, &w, param, current_row,
                           Window_retrieve_cached_row_reason::CURRENT))
    return true;

  /* NTILE and other non-framing wfs */
  if (w.needs_card()) {
    /* Set up the non-wf fields for aggregating to the output row. */
    if (process_wfs_needing_card(thd, param, have_nth_value, have_lead_lag,
                                 current_row, &w,
                                 Window_retrieve_cached_row_reason::CURRENT))
      return true;
  }

  if (w.is_last() && copy_funcs(param, thd, CFT_HAS_WF)) return true;
  *output_row_ready = true;
  w.set_last_row_output(current_row);
  DBUG_PRINT("info", ("sent row: %" PRId64, current_row));

  return false;
}

bool construct_lookup_ref(THD *thd, TABLE *table, TABLE_REF *ref) {
  enum enum_check_fields save_check_for_truncated_fields =
      thd->check_for_truncated_fields;
  thd->check_for_truncated_fields = CHECK_FIELD_IGNORE;
  my_bitmap_map *old_map = dbug_tmp_use_all_columns(table, table->write_set);
  bool result = false;

  for (uint part_no = 0; part_no < ref->key_parts; part_no++) {
    store_key *s_key = ref->key_copy[part_no];
    if (!s_key) continue;

    /*
      copy() can return STORE_KEY_OK even when there are errors so need to
      check thd->is_error().
      @todo This is due to missing handling of error return value from
      Field::store().
    */
    if (s_key->copy() != store_key::STORE_KEY_OK || thd->is_error()) {
      result = true;
      break;
    }
  }
  thd->check_for_truncated_fields = save_check_for_truncated_fields;
  dbug_tmp_restore_column_map(table->write_set, old_map);
  return result;
}

/**
  allocate group fields or take prepared (cached).

  @param main_join   join of current select
  @param curr_join   current join (join of current select or temporary copy
                     of it)

  @retval
    0   ok
  @retval
    1   failed
*/

bool make_group_fields(JOIN *main_join, JOIN *curr_join) {
  DBUG_TRACE;
  if (main_join->group_fields_cache.elements) {
    curr_join->group_fields = main_join->group_fields_cache;
    curr_join->streaming_aggregation = true;
  } else {
    if (alloc_group_fields(curr_join, curr_join->group_list)) return true;
    main_join->group_fields_cache = curr_join->group_fields;
  }
  return false;
}

/**
  Get a list of buffers for saveing last group.

  Groups are saved in reverse order for easyer check loop.
*/

static bool alloc_group_fields(JOIN *join, ORDER *group) {
  if (group) {
    for (; group; group = group->next) {
      Cached_item *tmp = new_Cached_item(join->thd, *group->item);
      if (!tmp || join->group_fields.push_front(tmp)) return true;
    }
  }
  join->streaming_aggregation = true; /* Mark for do_select */
  return false;
}

/*
  Test if a single-row cache of items changed, and update the cache.

  @details Test if a list of items that typically represents a result
  row has changed. If the value of some item changed, update the cached
  value for this item.

  @param list list of <item, cached_value> pairs stored as Cached_item.

  @return -1 if no item changed
  @return index of the first item that changed
*/

int update_item_cache_if_changed(List<Cached_item> &list) {
  DBUG_TRACE;
  List_iterator<Cached_item> li(list);
  int idx = -1, i;
  Cached_item *buff;

  for (i = (int)list.elements - 1; (buff = li++); i--) {
    if (buff->cmp()) idx = i;
  }
  DBUG_PRINT("info", ("idx: %d", idx));
  return idx;
}

/**
  Sets up caches for holding the values of non-aggregated expressions. The
  values are saved at the start of every new group.

  This code path is used in the cases when aggregation can be performed
  without a temporary table. Why it still uses a Temp_table_param is a
  mystery.

  Only FIELD_ITEM:s and FUNC_ITEM:s needs to be saved between groups.
  Change old item_field to use a new field with points at saved fieldvalue
  This function is only called before use of send_result_set_metadata.

  @param all_fields                  all fields list; should really be const,
                                       but Item does not always respect
                                       constness
  @param num_select_elements         number of elements in select item list
  @param thd                         THD pointer
  @param [in,out] param              temporary table parameters
  @param [out] ref_item_array        array of pointers to top elements of field
                                       list
  @param [out] res_selected_fields   new list of items of select item list
  @param [out] res_all_fields        new list of all items

  @todo
    In most cases this result will be sent to the user.
    This should be changed to use copy_int or copy_real depending
    on how the value is to be used: In some cases this may be an
    argument in a group function, like: IF(ISNULL(col),0,COUNT(*))

  @returns false if success, true if error
*/

bool setup_copy_fields(List<Item> &all_fields, size_t num_select_elements,
                       THD *thd, Temp_table_param *param,
                       Ref_item_array ref_item_array,
                       List<Item> *res_selected_fields,
                       List<Item> *res_all_fields) {
  DBUG_TRACE;

  res_selected_fields->empty();
  res_all_fields->empty();
  size_t border = all_fields.size() - num_select_elements;
  Mem_root_vector<Item_copy *> extra_funcs(
      Mem_root_allocator<Item_copy *>(thd->mem_root));

  param->grouped_expressions.clear();
  DBUG_ASSERT(param->copy_fields.empty());

  try {
    param->grouped_expressions.reserve(all_fields.size());
    param->copy_fields.reserve(param->field_count);
    extra_funcs.reserve(border);
  } catch (std::bad_alloc &) {
    return true;
  }

  List_iterator_fast<Item> li(all_fields);
  Item *pos;
  for (size_t i = 0; (pos = li++); i++) {
    Item *real_pos = pos->real_item();
    if (real_pos->type() == Item::FIELD_ITEM) {
      Item_field *item = new Item_field(thd, ((Item_field *)real_pos));
      if (item == nullptr) return true;
      if (pos->type() == Item::REF_ITEM) {
        /* preserve the names of the ref when dereferncing */
        Item_ref *ref = (Item_ref *)pos;
        item->db_name = ref->db_name;
        item->table_name = ref->table_name;
        item->item_name = ref->item_name;
      }
      pos = item;
      if (item->field->flags & BLOB_FLAG) {
        Item_copy *item_copy = Item_copy::create(pos);
        if (item_copy == nullptr) return true;
        pos = item_copy;
        /*
          Item_copy_string::copy for function can call
          Item_copy_string::val_int for blob via Item_ref.
          But if Item_copy_string::copy for blob isn't called before,
          it's value will be wrong
          so let's insert Item_copy_string for blobs in the beginning of
          copy_funcs
          (to see full test case look at having.test, BUG #4358)
        */
        param->grouped_expressions.push_back(item_copy);
      } else {
        DBUG_ASSERT(param->field_count > param->copy_fields.size());
        param->copy_fields.emplace_back(thd->mem_root, item);

        /*
          Even though the field doesn't point into field->table->record[0], we
          must still link it to 'table' through field->table because that's an
          existing way to access some type info (e.g. nullability from
          table->nullable).
        */
      }
    } else if (((real_pos->type() == Item::FUNC_ITEM ||
                 real_pos->type() == Item::SUBSELECT_ITEM ||
                 real_pos->type() == Item::CACHE_ITEM ||
                 real_pos->type() == Item::COND_ITEM) &&
                !real_pos->has_aggregation() &&
                !real_pos->has_rollup_expr())) {  // Save for send fields
      pos = real_pos;
      /* TODO:
         In most cases this result will be sent to the user.
         This should be changed to use copy_int or copy_real depending
         on how the value is to be used: In some cases this may be an
         argument in a group function, like: IF(ISNULL(col),0,COUNT(*))
      */
      Item_copy *item_copy = Item_copy::create(pos);
      if (item_copy == nullptr) return true;
      pos = item_copy;
      if (i < border)  // HAVING, ORDER and GROUP BY
        extra_funcs.push_back(item_copy);
      else
        param->grouped_expressions.push_back(item_copy);
    }
    res_all_fields->push_back(pos);
    ref_item_array[((i < border) ? all_fields.size() - i - 1 : i - border)] =
        pos;
  }

  List_iterator_fast<Item> itr(*res_all_fields);
  for (size_t i = 0; i < border; i++) itr++;
  itr.sublist(*res_selected_fields, num_select_elements);
  /*
    Put elements from HAVING, ORDER BY and GROUP BY last to ensure that any
    reference used in these will resolve to a item that is already calculated
  */
  param->grouped_expressions.insert(param->grouped_expressions.end(),
                                    extra_funcs.begin(), extra_funcs.end());
  return false;
}

/**
  Make a copy of all simple SELECT'ed fields.

  This is done at the start of a new group so that we can retrieve
  these later when the group changes. It is also used in materialization,
  to copy the values into the temporary table's fields.

  @param param     Represents the current temporary file being produced
  @param thd       The current thread
  @param reverse_copy   If true, copies fields *back* from the frame buffer
                        tmp table to the input table's buffer,
                        cf. #bring_back_frame_row.

  @returns false if OK, true on error.
*/

bool copy_fields(Temp_table_param *param, const THD *thd, bool reverse_copy) {
  DBUG_TRACE;

  DBUG_PRINT("enter", ("for param %p", param));
  for (Copy_field &ptr : param->copy_fields) ptr.invoke_do_copy(reverse_copy);

  if (thd->is_error()) return true;

  for (Item_copy *item : param->grouped_expressions) {
    if (item->copy(thd)) return true;
  }
  return false;
}

bool copy_fields_and_funcs(Temp_table_param *param, const THD *thd,
                           Copy_func_type type) {
  if (copy_fields(param, thd)) return true;
  if (param->items_to_copy != nullptr) {
    if (copy_funcs(param, thd, type)) return true;
  }
  return false;
}

/**
  Change all funcs and sum_funcs to fields in tmp table, and create
  new list of all items.

  @param all_fields                  all fields list; should really be const,
                                       but Item does not always respect
                                       constness
  @param num_select_elements         number of elements in select item list
  @param thd                         THD pointer
  @param [out] ref_item_array        array of pointers to top elements of filed
  list
  @param [out] res_selected_fields   new list of items of select item list
  @param [out] res_all_fields        new list of all items

  @returns false if success, true if error
*/

bool change_to_use_tmp_fields(List<Item> &all_fields,
                              size_t num_select_elements, THD *thd,
                              Ref_item_array ref_item_array,
                              List<Item> *res_selected_fields,
                              List<Item> *res_all_fields) {
  DBUG_TRACE;

  res_selected_fields->empty();
  res_all_fields->empty();

  List_iterator_fast<Item> li(all_fields);
  size_t border = all_fields.size() - num_select_elements;
  Item *item;
  for (size_t i = 0; (item = li++); i++) {
    Item *item_field;
    Field *field;
    if (item->has_aggregation() && item->type() != Item::SUM_FUNC_ITEM)
      item_field = item;
    else if (item->type() == Item::FIELD_ITEM)
      item_field = item->get_tmp_table_item(thd);
    else if (item->type() == Item::FUNC_ITEM &&
             ((Item_func *)item)->functype() == Item_func::SUSERVAR_FUNC) {
      field = item->get_tmp_table_field();
      if (field != nullptr) {
        /*
          Replace "@:=<expression>" with "@:=<tmp table column>". Otherwise, we
          would re-evaluate <expression>, and if expression were a subquery,
          this would access already-unlocked tables.
        */
        Item_func_set_user_var *suv =
            new Item_func_set_user_var(thd, (Item_func_set_user_var *)item);
        Item_field *new_field = new Item_field(field);
        if (!suv || !new_field) return true;  // Fatal error
        List<Item> list;
        list.push_back(new_field);
        suv->set_arguments(list, true);
        item_field = suv;
      } else
        item_field = item;
    } else if ((field = item->get_tmp_table_field())) {
      if (item->type() == Item::SUM_FUNC_ITEM && field->table->group) {
        item_field = down_cast<Item_sum *>(item)->result_item(field);
        DBUG_ASSERT(item_field != nullptr);
      } else {
        item_field = new (thd->mem_root) Item_field(field);
        if (item_field == nullptr) return true;
      }
      if (item->real_item()->type() != Item::FIELD_ITEM)
        field->orig_table = nullptr;
      item_field->item_name = item->item_name;
      if (item->type() == Item::REF_ITEM) {
        Item_field *ifield = (Item_field *)item_field;
        Item_ref *iref = (Item_ref *)item;
        ifield->table_name = iref->table_name;
        ifield->db_name = iref->db_name;
      }
#ifndef DBUG_OFF
      if (!item_field->item_name.is_set()) {
        char buff[256];
        String str(buff, sizeof(buff), &my_charset_bin);
        str.length(0);
        item->print(thd, &str, QT_ORDINARY);
        item_field->item_name.copy(str.ptr(), str.length());
      }
#endif
    } else
      item_field = item;

    res_all_fields->push_back(item_field);
    /*
      Cf. comment explaining the reordering going on below in
      similar section of change_refs_to_tmp_fields
    */
    ref_item_array[((i < border) ? all_fields.size() - i - 1 : i - border)] =
        item_field;
    item_field->set_orig_field(item->get_orig_field());
  }

  List_iterator_fast<Item> itr(*res_all_fields);
  for (size_t i = 0; i < border; i++) itr++;
  itr.sublist(*res_selected_fields, num_select_elements);
  return false;
}

/**
  Change all sum_func refs to fields to point at fields in tmp table.
  Change all funcs to be fields in tmp table.

  @param all_fields                  all fields list; should really be const,
                                       but Item does not always respect
                                       constness
  @param num_select_elements         number of elements in select item list
  @param thd                         THD pointer
  @param [out] ref_item_array        array of pointers to top elements of filed
  list
  @param [out] res_selected_fields   new list of items of select item list
  @param [out] res_all_fields        new list of all items

  @returns false if success, true if error
*/

bool change_refs_to_tmp_fields(List<Item> &all_fields,
                               size_t num_select_elements, THD *thd,
                               Ref_item_array ref_item_array,
                               List<Item> *res_selected_fields,
                               List<Item> *res_all_fields) {
  DBUG_TRACE;
  res_selected_fields->empty();
  res_all_fields->empty();

  List_iterator_fast<Item> li(all_fields);
  size_t border = all_fields.size() - num_select_elements;
  Item *item;
  for (size_t i = 0; (item = li++); i++) {
    /*
      Below we create "new_item" using get_tmp_table_item
      based on all_fields[i] and assign them to res_all_fields[i].

      The new items are also put into ref_item_array, but in another order,
      cf the diagram below.

      Example of the population of ref_item_array, ref_all_fields and
      res_selected_fields based on all_fields:

      res_all_fields             res_selected_fields
         |                          |
         V                          V
       +--+   +--+   +--+   +--+   +--+   +--+          +--+
       |0 |-->|  |-->|  |-->|3 |-->|4 |-->|  |--> .. -->|9 |
       +--+   +--+   +--+   +--+   +--+   +--+          +--+
                              |     |
        ,------------->--------\----/
        |                       |
      +-^-+---+---+---+---+---#-^-+---+---+---+
      |   |   |   |   |   |   #   |   |   |   | ref_item_array
      +---+---+---+---+---+---#---+---+---+---+
        4   5   6   7   8   9   3   2   1   0   position in all_fields list
                                                similar to ref_all_fields pos
      all_fields.elements == 10      border == 4
      (visible) elements == 6

      i==0   ->   afe-0-1 == 9     i==4 -> 4-4 == 0
      i==1   ->   afe-1-1 == 8      :
      i==2   ->   afe-2-1 == 7
      i==3   ->   afe-3-1 == 6     i==9 -> 9-4 == 5
    */
    Item *new_item = item->get_tmp_table_item(thd);
    res_all_fields->push_back(new_item);
    ref_item_array[((i < border) ? all_fields.size() - i - 1 : i - border)] =
        new_item;
  }

  List_iterator_fast<Item> itr(*res_all_fields);
  for (size_t i = 0; i < border; i++) itr++;
  itr.sublist(*res_selected_fields, num_select_elements);

  return thd->is_fatal_error();
}

/**
  Clear all result fields. Non-aggregated fields are set to NULL,
  aggregated fields are set to their special "clear" value.

  Result fields can be fields from input tables, field values generated
  by sum functions and literal values.

  This is used when no rows are found during grouping: for FROM clause, a
  result row of all NULL values will be output; then SELECT list expressions
  get evaluated. E.g. SUM() will be NULL (the special "clear" value) and thus
  SUM() IS NULL will be true.

  @note Setting field values for input tables is a destructive operation,
        since it overwrite the NULL value flags with 1 bits. Rows from
        const tables are never re-read, hence their NULL value flags must
        be saved by this function and later restored by JOIN::restore_fields().
        This is generally not necessary for non-const tables, since field
        values are overwritten when new rows are read.

  @param[out] save_nullinfo Map of tables whose fields were set to NULL,
                            and for which NULL values must be restored.
                            Should be set to all zeroes on entry to function.

  @returns false if success, true if error
*/

bool JOIN::clear_fields(table_map *save_nullinfo) {
  // Set all column values from all input tables to NULL.
  for (uint tableno = 0; tableno < primary_tables; tableno++) {
    QEP_TAB *const tab = qep_tab + tableno;
    TABLE *const table = tab->table_ref->table;
    if (!table->has_null_row()) {
      *save_nullinfo |= tab->table_ref->map();
      if (table->const_table) table->save_null_flags();
      table->set_null_row();  // All fields are NULL
    }
  }
  if (copy_fields(&tmp_table_param, thd)) return true;

  if (sum_funcs) {
    Item_sum *func, **func_ptr = sum_funcs;
    while ((func = *(func_ptr++))) func->clear();
  }
  return false;
}

/**
  Restore all result fields for all tables specified in save_nullinfo.

  @param save_nullinfo Set of tables for which restore is necessary.

  @note Const tables must have their NULL value flags restored,
        @see JOIN::clear_fields().
*/
void JOIN::restore_fields(table_map save_nullinfo) {
  DBUG_ASSERT(save_nullinfo);

  for (uint tableno = 0; tableno < primary_tables; tableno++) {
    QEP_TAB *const tab = qep_tab + tableno;
    if (save_nullinfo & tab->table_ref->map()) {
      TABLE *const table = tab->table_ref->table;
      if (table->const_table) table->restore_null_flags();
      table->reset_null_row();
    }
  }
}

/******************************************************************************
  Code for pfs_batch_update
******************************************************************************/

bool QEP_TAB::pfs_batch_update(const JOIN *join) const {
  /*
    Use PFS batch mode unless
     1. tab is not an inner-most table, or
     2. a table has eq_ref or const access type, or
     3. this tab contains a subquery that accesses one or more tables
  */

  return !((join->qep_tab + join->primary_tables - 1) != this ||  // 1
           this->type() == JT_EQ_REF ||                           // 2
           this->type() == JT_CONST || this->type() == JT_SYSTEM ||
           (condition() && condition()->has_subquery()));  // 3
}

/**
  @} (end of group Query_Executor)
*/

vector<string> UnqualifiedCountIterator::DebugString() const {
  return {"Count rows in " + string(m_join->qep_tab->table()->alias)};
}

int UnqualifiedCountIterator::Read() {
  if (!m_has_row) {
    return -1;
  }

  for (Item &item : m_join->all_fields) {
    if (item.type() == Item::SUM_FUNC_ITEM &&
        down_cast<Item_sum &>(item).sum_func() == Item_sum::COUNT_FUNC) {
      int error;
      ulonglong count = get_exact_record_count(m_join->qep_tab,
                                               m_join->primary_tables, &error);
      if (error) return 1;

      down_cast<Item_sum_count &>(item).make_const(
          static_cast<longlong>(count));
    }
  }

  // If we are outputting to a temporary table, we need to copy the results
  // into it here. It is also used for nonaggregated items, even when there are
  // no temporary tables involved.
  if (copy_fields_and_funcs(&m_join->tmp_table_param, m_join->thd)) {
    return 1;
  }

  m_has_row = false;
  return 0;
}

int ZeroRowsAggregatedIterator::Read() {
  if (!m_has_row) {
    return -1;
  }

  // Mark tables as containing only NULL values
  for (TABLE_LIST *table = m_join->select_lex->leaf_tables; table;
       table = table->next_leaf) {
    table->table->set_null_row();
  }

  // Calculate aggregate functions for no rows

  /*
    Must notify all fields that there are no rows (not only those
    that will be returned) because join->having may refer to
    fields that are not part of the result columns.
   */
  for (Item &item : m_join->all_fields) {
    item.no_rows_in_result();
  }

  m_has_row = false;
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

TableValueConstructorIterator::TableValueConstructorIterator(
    THD *thd, ha_rows *examined_rows, const List<List<Item>> &row_value_list,
    List<Item> *join_fields)
    : RowIterator(thd),
      m_examined_rows(examined_rows),
      m_row_value_list(row_value_list),
      m_output_refs(join_fields) {}

bool TableValueConstructorIterator::Init() {
  m_row_it = m_row_value_list.begin();
  return false;
}

int TableValueConstructorIterator::Read() {
  if (*m_examined_rows == m_row_value_list.size()) return -1;

  // If the TVC has a single row, we don't create Item_values_column reference
  // objects during resolving. We will instead use the single row directly from
  // SELECT_LEX::item_list, such that we don't have to change references here.
  if (m_row_value_list.size() != 1) {
    List_STL_Iterator<Item> output_refs_it = m_output_refs->begin();
    for (const Item &value : *m_row_it) {
      Item_values_column &ref =
          down_cast<Item_values_column &>(*output_refs_it);
      ++output_refs_it;

      // Ideally we would not be casting away constness here. However, as the
      // evaluation of Item objects during execution is not const (i.e. none of
      // the val methods are const), the reference contained in a
      // Item_values_column object cannot be const.
      ref.set_value(const_cast<Item *>(&value));
    }
    ++m_row_it;
  }

  ++*m_examined_rows;
  return 0;
}

static inline pair<uchar *, key_part_map> FindKeyBufferAndMap(
    const TABLE_REF *ref) {
  if (ref->keypart_hash != nullptr) {
    return make_pair(pointer_cast<uchar *>(ref->keypart_hash), key_part_map{1});
  } else {
    return make_pair(ref->key_buff, make_prev_keypart_map(ref->key_parts));
  }
}
