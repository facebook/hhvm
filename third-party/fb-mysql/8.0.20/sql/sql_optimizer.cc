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

  @brief Optimize query expressions: Make optimal table join order, select
         optimal access methods per table, apply grouping, sorting and
         limit processing.

  @defgroup Query_Optimizer  Query Optimizer
  @{
*/

#include "sql/sql_optimizer.h"

#include <limits.h>
#include <algorithm>
#include <atomic>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "field_types.h"  // enum_field_types
#include "ft_global.h"
#include "m_ctype.h"
#include "memory_debugging.h"
#include "my_bit.h"  // my_count_bits
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "scope_guard.h"
#include "sql/abstract_query_plan.h"  // Join_plan
#include "sql/basic_row_iterators.h"
#include "sql/check_stack.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/derror.h"      // ER_THD
#include "sql/enum_query_type.h"
#include "sql/error_handler.h"  // Functional_index_error_handler
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/item_row.h"
#include "sql/item_subselect.h"
#include "sql/item_sum.h"  // Item_sum
#include "sql/key.h"
#include "sql/key_spec.h"
#include "sql/lock.h"    // mysql_unlock_some_tables
#include "sql/mysqld.h"  // stage_optimizing
#include "sql/nested_join.h"
#include "sql/opt_costmodel.h"
#include "sql/opt_explain.h"  // join_type_str
#include "sql/opt_hints.h"    // hint_table_state
#include "sql/opt_range.h"    // QUICK_SELECT_I
#include "sql/opt_trace.h"    // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_node_base.h"
#include "sql/parser_yystype.h"
#include "sql/query_options.h"
#include "sql/query_result.h"
#include "sql/sql_base.h"  // init_ftfuncs
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_const_folding.h"
#include "sql/sql_error.h"
#include "sql/sql_join_buffer.h"  // JOIN_CACHE
#include "sql/sql_planner.h"      // calculate_condition_filter
#include "sql/sql_test.h"         // print_where
#include "sql/sql_tmp_table.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/timing_iterator.h"
#include "sql/window.h"
#include "sql_string.h"
#include "template_utils.h"

using std::max;
using std::min;

const char *antijoin_null_cond = "<ANTIJOIN-NULL>";

static bool optimize_semijoin_nests_for_materialization(JOIN *join);
static void calculate_materialization_costs(JOIN *join, TABLE_LIST *sj_nest,
                                            uint n_tables,
                                            Semijoin_mat_optimize *sjm);
static bool make_join_select(JOIN *join, Item *item);
static bool list_contains_unique_index(JOIN_TAB *tab,
                                       bool (*find_func)(Field *, void *),
                                       void *data);
static bool find_field_in_item_list(Field *field, void *data);
static bool find_field_in_order_list(Field *field, void *data);
static TABLE *get_sort_by_table(ORDER *a, ORDER *b, TABLE_LIST *tables);
static void trace_table_dependencies(Opt_trace_context *trace,
                                     JOIN_TAB *join_tabs, uint table_count);
static bool update_ref_and_keys(THD *thd, Key_use_array *keyuse,
                                JOIN_TAB *join_tab, uint tables, Item *cond,
                                table_map normal_tables, SELECT_LEX *select_lex,
                                SARGABLE_PARAM **sargables);
static bool pull_out_semijoin_tables(JOIN *join);
static void add_loose_index_scan_and_skip_scan_keys(JOIN *join,
                                                    JOIN_TAB *join_tab);
static ha_rows get_quick_record_count(THD *thd, JOIN_TAB *tab, ha_rows limit);
static bool only_eq_ref_tables(JOIN *join, ORDER *order, table_map tables,
                               table_map *cached_eq_ref_tables,
                               table_map *eq_ref_tables);
static bool setup_join_buffering(JOIN_TAB *tab, JOIN *join, uint no_jbuf_after);

static bool test_if_skip_sort_order(JOIN_TAB *tab, ORDER_with_src &order,
                                    ha_rows select_limit, const bool no_changes,
                                    const Key_map *map, int *best_idx);

static Item_func_match *test_if_ft_index_order(ORDER *order);

static uint32 get_key_length_tmp_table(Item *item);
static const char *can_switch_from_ref_to_range(THD *thd, JOIN_TAB *tab,
                                                enum_order ordering,
                                                bool recheck_range);

static bool has_not_null_predicate(Item *cond, Item_field *not_null_item);
static void measure_compilation_cpu(THD *thd, int cpu_res,
                                    timespec *beginning_id);

JOIN::JOIN(THD *thd_arg, SELECT_LEX *select)
    : select_lex(select),
      unit(select->master_unit()),
      thd(thd_arg),
      // @todo Can this be substituted with select->is_explicitly_grouped()?
      grouped(select->is_explicitly_grouped()),
      // Inner tables may always be considered to be constant:
      const_table_map(INNER_TABLE_BIT),
      found_const_table_map(INNER_TABLE_BIT),
      // Needed in case optimizer short-cuts, set properly in
      // make_tmp_tables_info()
      fields(&select->item_list),
      tmp_table_param(thd_arg->mem_root),
      lock(thd->lock),
      // @todo Can this be substituted with select->is_implicitly_grouped()?
      implicit_grouping(select->is_implicitly_grouped()),
      select_distinct(select->is_distinct()),
      keyuse_array(thd->mem_root),
      all_fields(select->all_fields),
      fields_list(select->fields_list),
      order(select->order_list.first, ESC_ORDER_BY),
      group_list(select->group_list.first, ESC_GROUP_BY),
      m_windows(select->m_windows),
      /*
        Those four members are meaningless before JOIN::optimize(), so force a
        crash if they are used before that.
      */
      where_cond(reinterpret_cast<Item *>(1)),
      having_cond(reinterpret_cast<Item *>(1)),
      having_for_explain(reinterpret_cast<Item *>(1)),
      tables_list(reinterpret_cast<TABLE_LIST *>(1)),
      current_ref_item_slice(REF_SLICE_SAVED_BASE),
      with_json_agg(select->json_agg_func_used()) {
  rollup.state = ROLLUP::STATE_NONE;
  if (select->order_list.first) explain_flags.set(ESC_ORDER_BY, ESP_EXISTS);
  if (select->group_list.first) explain_flags.set(ESC_GROUP_BY, ESP_EXISTS);
  if (select->is_distinct()) explain_flags.set(ESC_DISTINCT, ESP_EXISTS);
  if (m_windows.elements > 0) explain_flags.set(ESC_WINDOWING, ESP_EXISTS);
  // Calculate the number of groups
  for (ORDER *group = group_list; group; group = group->next)
    send_group_parts++;
}

bool JOIN::alloc_ref_item_slice(THD *thd_arg, int sliceno) {
  DBUG_ASSERT(sliceno > 0);
  DBUG_ASSERT(ref_items[sliceno].is_null());
  size_t count = ref_items[0].size();
  Item **slice = thd_arg->mem_root->ArrayAlloc<Item *>(count);
  if (slice == nullptr) return true;
  ref_items[sliceno] = Ref_item_array(slice, count);
  return false;
}

bool JOIN::alloc_indirection_slices() {
  const uint card = REF_SLICE_WIN_1 + m_windows.elements * 2;

  DBUG_ASSERT(ref_items == nullptr);
  ref_items =
      (Ref_item_array *)(*THR_MALLOC)->Alloc(sizeof(Ref_item_array) * card);
  if (ref_items == nullptr) return true;

  tmp_all_fields =
      (List<Item> *)(*THR_MALLOC)->Alloc(sizeof(List<Item>) * card);
  if (tmp_all_fields == nullptr) return true;

  tmp_fields_list =
      (List<Item> *)(*THR_MALLOC)->Alloc(sizeof(List<Item>) * card);
  if (tmp_fields_list == nullptr) return true;

  for (uint i = 0; i < card; i++) {
    ref_items[i].reset();
    tmp_all_fields[i].empty();
    tmp_fields_list[i].empty();
  }

  return false;
}

/**
  Optimizes one query block into a query execution plan (QEP.)

  This is the entry point to the query optimization phase. This phase
  applies both logical (equivalent) query rewrites, cost-based join
  optimization, and rule-based access path selection. Once an optimal
  plan is found, the member function creates/initializes all
  structures needed for query execution. The main optimization phases
  are outlined below:

    -# Logical transformations:
      - Outer to inner joins transformation.
      - Equality/constant propagation.
      - Partition pruning.
      - COUNT(*), MIN(), MAX() constant substitution in case of
        implicit grouping.
      - ORDER BY optimization.
    -# Perform cost-based optimization of table order and access path
       selection. See JOIN::make_join_plan()
    -# Post-join order optimization:
       - Create optimal table conditions from the where clause and the
         join conditions.
       - Inject outer-join guarding conditions.
       - Adjust data access methods after determining table condition
         (several times.)
       - Optimize ORDER BY/DISTINCT.
    -# Code generation
       - Set data access functions.
       - Try to optimize away sorting/distinct.
       - Setup temporary table usage for grouping and/or sorting.

  @retval false Success.
  @retval true Error, error code saved in member JOIN::error.
*/
bool JOIN::optimize() {
  DBUG_TRACE;

  uint no_jbuf_after = UINT_MAX;
  const bool has_windows = m_windows.elements != 0;

  DBUG_ASSERT(select_lex->leaf_table_count == 0 ||
              thd->lex->is_query_tables_locked() ||
              select_lex == unit->fake_select_lex);
  DBUG_ASSERT(tables == 0 && primary_tables == 0 &&
              tables_list == (TABLE_LIST *)1);

  // to prevent double initialization on EXPLAIN
  if (optimized) return false;

  /* measure the compilation CPU
     TODO: Currently compilation CPU is not correctly for two
           kinds of queries:
     - delete from <TBL> where <predicate(s) on columns>
     - update from <TBL> where <predicate(s) on columns>
     NOTE: If the where clause contains sub-queries then
           the compilation CPU is tracked correctly.
   */
  timespec time_beg;
  int cpu_res = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_beg);

  auto compilation_cpu_guard = create_scope_guard(
      [&]() { measure_compilation_cpu(thd, cpu_res, &time_beg); });

  Prepare_error_tracker tracker(thd);

  DEBUG_SYNC(thd, "before_join_optimize");

  THD_STAGE_INFO(thd, stage_optimizing);

  if (select_lex->first_execution) {
    /**
      @todo
      This query block didn't transform itself in SELECT_LEX::prepare(), so
      belongs to a parent query block. That parent, or its parents, had to
      transform us - it has not; maybe it is itself in prepare() and
      evaluating the present query block as an Item_subselect. Such evaluation
      in prepare() is expected to be a rare case to be eliminated in the
      future ("SET x=(subq)" is one such case; because it locks tables before
      prepare()).
    */
    if (select_lex->apply_local_transforms(thd, false)) return (error = 1);
  }

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_optimize(trace, "join_optimization");
  trace_optimize.add_select_number(select_lex->select_number);
  Opt_trace_array trace_steps(trace, "steps");

  count_field_types(select_lex, &tmp_table_param, all_fields, false, false);

  DBUG_ASSERT(tmp_table_param.sum_func_count == 0 || group_list ||
              implicit_grouping);

  if (select_lex->olap == ROLLUP_TYPE && optimize_rollup())
    return true; /* purecov: inspected */

  if (alloc_func_list()) return true; /* purecov: inspected */

  if (select_lex->get_optimizable_conditions(thd, &where_cond, &having_cond))
    return true;

  set_optimized();

  tables_list = select_lex->leaf_tables;

  if (alloc_indirection_slices()) return true;

  // The base ref items from query block are assigned as JOIN's ref items
  ref_items[REF_SLICE_ACTIVE] = select_lex->base_ref_items;

  /* dump_TABLE_LIST_graph(select_lex, select_lex->leaf_tables); */
  /*
    Run optimize phase for all derived tables/views used in this SELECT,
    including those in semi-joins.
  */
  if (select_lex->materialized_derived_table_count) {
    for (TABLE_LIST *tl = select_lex->leaf_tables; tl; tl = tl->next_leaf) {
      if (tl->is_view_or_derived() && tl->optimize_derived(thd)) return true;
    }
  }

  has_lateral = false;

  /* dump_TABLE_LIST_graph(select_lex, select_lex->leaf_tables); */

  if (thd->optimizer_switch_flag(OPTIMIZER_SWITCH_GROUP_BY_LIMIT)) {
    row_limit = unit->select_limit_cnt;
  } else {
    row_limit =
        ((select_distinct || order || group_list) ? HA_POS_ERROR
                                                  : unit->select_limit_cnt);
  }

  // m_select_limit is used to decide if we are likely to scan the whole table.
  m_select_limit = unit->select_limit_cnt;

  if (unit->first_select()->active_options() & OPTION_FOUND_ROWS) {
    /*
      Calculate found rows (ie., keep counting rows even after we hit LIMIT) if
      - LIMIT is set, and
      - This is the outermost query block (for a UNION query, this is the
        fake query block that contains the limit applied on the final UNION
        evaluation).
     */
    calc_found_rows =
        m_select_limit != HA_POS_ERROR &&
        (!unit->is_union() || select_lex == unit->fake_select_lex);
  }
  if (having_cond || calc_found_rows) m_select_limit = HA_POS_ERROR;

  if (unit->select_limit_cnt == 0 && !calc_found_rows) {
    zero_result_cause = "Zero limit";
    best_rowcount = 0;
    create_iterators_for_zero_rows();
    goto setup_subq_exit;
  }

  if (where_cond || select_lex->outer_join) {
    if (optimize_cond(thd, &where_cond, &cond_equal, &select_lex->top_join_list,
                      &select_lex->cond_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_cond"));
      return true;
    }
    if (select_lex->cond_value == Item::COND_FALSE) {
      zero_result_cause = "Impossible WHERE";
      best_rowcount = 0;
      create_iterators_for_zero_rows();
      goto setup_subq_exit;
    }
  }
  if (having_cond) {
    if (optimize_cond(thd, &having_cond, &cond_equal, nullptr,
                      &select_lex->having_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_cond"));
      return true;
    }
    if (select_lex->having_value == Item::COND_FALSE) {
      zero_result_cause = "Impossible HAVING";
      best_rowcount = 0;
      create_iterators_for_zero_rows();
      goto setup_subq_exit;
    }
  }

  if (select_lex->partitioned_table_count && prune_table_partitions()) {
    error = 1;
    DBUG_PRINT("error", ("Error from prune_partitions"));
    return true;
  }

  /*
     Try to optimize count(*), min() and max() to const fields if
     there is implicit grouping (aggregate functions but no
     group_list). In this case, the result set shall only contain one
     row.
  */
  if (tables_list && implicit_grouping &&
      !(select_lex->active_options() & OPTION_NO_CONST_TABLES)) {
    aggregate_evaluated outcome;
    if (optimize_aggregated_query(thd, select_lex, all_fields, where_cond,
                                  &outcome)) {
      error = 1;
      DBUG_PRINT("error", ("Error from optimize_aggregated_query"));
      return true;
    }
    switch (outcome) {
      case AGGR_REGULAR:
        // Query was not (fully) evaluated. Revert to regular optimization.
        break;
      case AGGR_DELAYED:
        // Query was not (fully) evaluated. Revert to regular optimization,
        // but indicate that storage engine supports HA_COUNT_ROWS_INSTANT.
        select_count = true;
        break;
      case AGGR_COMPLETE:
        // All SELECT expressions are fully evaluated
        DBUG_PRINT("info", ("Select tables optimized away"));
        zero_result_cause = "Select tables optimized away";
        tables_list = nullptr;  // All tables resolved
        best_rowcount = 1;
        const_tables = tables = primary_tables = select_lex->leaf_table_count;
        m_root_iterator =
            NewIterator<FakeSingleRowIterator>(thd, &examined_rows);
        m_root_iterator =
            attach_iterators_for_having_and_limit(move(m_root_iterator));
        /*
          There are no relevant conditions left from the WHERE;
          optimize_aggregated_query() will not return AGGR_COMPLETE if there are
          any table-independent conditions, and all other conditions have been
          optimized away by it. Thus, remove the condition, unless we have
          EXPLAIN (in which case we will keep it for printing).
        */
        if (!thd->lex->is_explain()) {
#ifndef DBUG_OFF
          // Verify, to be sure.
          if (where_cond != nullptr) {
            Item *table_independent_conds = make_cond_for_table(
                thd, where_cond, PSEUDO_TABLE_BITS, table_map(0), false);
            DBUG_ASSERT(table_independent_conds == nullptr);
          }
#endif
          where_cond = nullptr;
        }
        goto setup_subq_exit;
      case AGGR_EMPTY:
        // It was detected that the result tables are empty
        DBUG_PRINT("info", ("No matching min/max row"));
        zero_result_cause = "No matching min/max row";
        create_iterators_for_zero_rows();
        goto setup_subq_exit;
    }
  }
  if (tables_list == nullptr) {
    DBUG_PRINT("info", ("No tables"));
    best_rowcount = 1;
    error = 0;
    if (make_tmp_tables_info()) return true;
    count_field_types(select_lex, &tmp_table_param, all_fields, false, false);
    // Make plan visible for EXPLAIN
    set_plan_state(NO_TABLES);
    create_iterators();
    return false;
  }
  error = -1;  // Error is sent to client

  {
    m_windowing_steps = false;  // initialization
    m_windows_sort = false;
    List_iterator<Window> li(m_windows);
    Window *w;
    while ((w = li++))
      if (w->needs_sorting()) {
        m_windows_sort = true;
        break;
      }
  }

  sort_by_table = get_sort_by_table(order, group_list, select_lex->leaf_tables);

  if ((where_cond || group_list || order) &&
      substitute_gc(thd, select_lex, where_cond, group_list, order)) {
    // We added hidden fields to the all_fields list, count them.
    count_field_types(select_lex, &tmp_table_param, select_lex->all_fields,
                      false, false);
  }
  // Ensure there are no errors prior making query plan
  if (thd->is_error()) return true;

  // Set up join order and initial access paths
  THD_STAGE_INFO(thd, stage_statistics);
  if (make_join_plan()) {
    if (thd->killed) thd->send_kill_message();
    DBUG_PRINT("error", ("Error: JOIN::make_join_plan() failed"));
    return true;
  }

  // At this stage, join_tab==NULL, JOIN_TABs are listed in order by best_ref.
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  if (zero_result_cause != nullptr) {  // Can be set by make_join_plan().
    create_iterators_for_zero_rows();
    goto setup_subq_exit;
  }

  if (rollup.state == ROLLUP::STATE_NONE) {
    /* Remove distinct if only const tables */
    select_distinct &= !plan_is_const();
  }

  if (const_tables && !thd->locked_tables_mode &&
      !(select_lex->active_options() & SELECT_NO_UNLOCK)) {
    TABLE *ct[MAX_TABLES];
    for (uint i = 0; i < const_tables; i++) ct[i] = best_ref[i]->table();
    mysql_unlock_some_tables(thd, ct, const_tables);
  }
  if (!where_cond && select_lex->outer_join) {
    /* Handle the case where we have an OUTER JOIN without a WHERE */
    where_cond = new Item_func_true();  // Always true
  }

  error = 0;
  /*
    Among the equal fields belonging to the same multiple equality
    choose the one that is to be retrieved first and substitute
    all references to these in where condition for a reference for
    the selected field.
  */
  if (where_cond) {
    where_cond =
        substitute_for_best_equal_field(thd, where_cond, cond_equal, map2table);
    if (thd->is_error()) {
      error = 1;
      DBUG_PRINT("error", ("Error from substitute_for_best_equal"));
      return true;
    }
    where_cond->update_used_tables();
    DBUG_EXECUTE("where",
                 print_where(thd, where_cond, "after substitute_best_equal",
                             QT_ORDINARY););
  }

  /*
    Perform the same optimization on field evaluation for all join conditions.
  */
  for (uint i = const_tables; i < tables; ++i) {
    JOIN_TAB *const tab = best_ref[i];
    if (tab->position() && tab->join_cond()) {
      tab->set_join_cond(substitute_for_best_equal_field(
          thd, tab->join_cond(), tab->cond_equal, map2table));
      if (thd->is_error()) {
        error = 1;
        DBUG_PRINT("error", ("Error from substitute_for_best_equal"));
        return true;
      }
      tab->join_cond()->update_used_tables();
      if (tab->join_cond())
        tab->join_cond()->walk(&Item::cast_incompatible_args,
                               enum_walk::POSTFIX, nullptr);
    }
  }

  if (init_ref_access()) {
    error = 1;
    DBUG_PRINT("error", ("Error from init_ref_access"));
    return true;
  }

  // Update table dependencies after assigning ref access fields
  update_depend_map();

  THD_STAGE_INFO(thd, stage_preparing);

  if (make_join_select(this, where_cond)) {
    if (thd->is_error()) return true;

    zero_result_cause = "Impossible WHERE noticed after reading const tables";
    create_iterators_for_zero_rows();
    goto setup_subq_exit;
  }

  if (select_lex->query_result()->optimize()) {
    DBUG_PRINT("error", ("Error: Query_result::optimize() failed"));
    return true;  // error == -1
  }

  // Inject cast nodes into the WHERE conditions
  if (where_cond)
    where_cond->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX,
                     nullptr);

  error = -1; /* if goto err */

  if (optimize_distinct_group_order()) return true;

  if ((select_lex->active_options() & SELECT_NO_JOIN_CACHE) ||
      select_lex->ftfunc_list->elements)
    no_jbuf_after = 0;

  /* Perform FULLTEXT search before all regular searches */
  if (select_lex->has_ft_funcs() && optimize_fts_query()) return true;

  /*
    By setting child_subquery_can_materialize so late we gain the following:
    JOIN::compare_costs_of_subquery_strategies() can test this variable to
    know if we are have finished evaluating constant conditions, which itself
    helps determining fanouts.
  */
  child_subquery_can_materialize = true;

  /*
    It's necessary to check const part of HAVING cond as
    there is a chance that some cond parts may become
    const items after make_join_plan() (for example
    when Item is a reference to const table field from
    outer join).
    This check is performed only for those conditions
    which do not use aggregate functions. In such case
    temporary table may not be used and const condition
    elements may be lost during further having
    condition transformation in JOIN::exec.
  */
  if (having_cond && !having_cond->has_aggregation() && (const_tables > 0)) {
    having_cond->update_used_tables();
    if (remove_eq_conds(thd, having_cond, &having_cond,
                        &select_lex->having_value)) {
      error = 1;
      DBUG_PRINT("error", ("Error from remove_eq_conds"));
      return true;
    }
    if (select_lex->having_value == Item::COND_FALSE) {
      having_cond = new Item_func_false();
      zero_result_cause =
          "Impossible HAVING noticed after reading const tables";
      create_iterators_for_zero_rows();
      goto setup_subq_exit;
    }
  }

  // Inject cast nodes into the HAVING conditions
  if (having_cond)
    having_cond->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX,
                      nullptr);

  // Traverse the expressions and inject cast nodes to compatible data types,
  // if needed.
  {
    List_iterator<Item> select_expression_it(all_fields);
    Item *item;
    while ((item = select_expression_it++))
      item->walk(&Item::cast_incompatible_args, enum_walk::POSTFIX, nullptr);
  }

  if (rollup.state != ROLLUP::STATE_NONE) {
    if (rollup_process_const_fields()) {
      DBUG_PRINT("error", ("Error: rollup_process_fields() failed"));
      return true;
    }
    /*
      Fields may have been replaced by Item_func_rollup_const, so
      recalculate the number of fields and functions for this query block.
    */

    // JOIN::optimize_rollup() may set allow_group_via_temp_table = false,
    // and we must not undo that.
    const bool save_allow_group_via_temp_table =
        tmp_table_param.allow_group_via_temp_table;

    count_field_types(select_lex, &tmp_table_param, all_fields, false, false);
    tmp_table_param.allow_group_via_temp_table =
        save_allow_group_via_temp_table;
  }

  // See if this subquery can be evaluated with subselect_indexsubquery_engine
  if (const int ret = replace_index_subquery()) {
    create_iterators_for_index_subquery();
    set_plan_state(PLAN_READY);
    /*
      We leave optimize() because the rest of it is only about order/group
      which those subqueries don't have and about setting up plan which
      we're not going to use due to different execution method.
    */
    return ret < 0;
  }

  {
    /*
      If the hint FORCE INDEX FOR ORDER BY/GROUP BY is used for the first
      table (it does not make sense for other tables) then we cannot do join
      buffering.
    */
    if (!plan_is_const()) {
      const TABLE *const first = best_ref[const_tables]->table();
      if ((first->force_index_order && order) ||
          (first->force_index_group && group_list))
        no_jbuf_after = 0;
    }

    bool simple_sort = true;
    Deps_of_remaining_lateral_derived_tables deps_lateral(this, all_table_map);
    // Check whether join cache could be used
    for (uint i = const_tables; i < tables; i++) {
      JOIN_TAB *const tab = best_ref[i];
      if (!tab->position()) continue;
      if (setup_join_buffering(tab, this, no_jbuf_after)) return true;
      if (tab->use_join_cache() != JOIN_CACHE::ALG_NONE) simple_sort = false;
      DBUG_ASSERT(tab->type() != JT_FT ||
                  tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
      deps_lateral.recalculate(tab, i + 1);
    }
    if (!simple_sort) {
      /*
        A join buffer is used for this table. We here inform the optimizer
        that it should not rely on rows of the first non-const table being in
        order thanks to an index scan; indeed join buffering of the present
        table subsequently changes the order of rows.
      */
      simple_order = simple_group = false;
    }
  }

  if (!plan_is_const() && order) {
    /*
      Force using of tmp table if sorting by a SP or UDF function due to
      their expensive and probably non-deterministic nature.
    */
    for (ORDER *tmp_order = order; tmp_order; tmp_order = tmp_order->next) {
      Item *item = *tmp_order->item;
      if (item->is_expensive()) {
        /* Force tmp table without sort */
        simple_order = simple_group = false;
        break;
      }
    }
  }

  /*
    Check if we need to create a temporary table prior to any windowing.

    (1) If there is ROLLUP, which happens before DISTINCT, windowing and ORDER
    BY, any of those clauses needs the result of ROLLUP in a tmp table.

    Rows which ROLLUP adds to the result are visible only to DISTINCT,
    windowing and ORDER BY which we handled above. So for the rest of
    conditions ((2), etc), we can do as if there were no ROLLUP.

    (2) If all tables are constant, the query's result is guaranteed to have 0
    or 1 row only, so all SQL clauses discussed below (DISTINCT, ORDER BY,
    GROUP BY, windowing, SQL_BUFFER_RESULT) are useless and need no tmp
    table.

    (3) If there is GROUP BY which isn't resolved by using an index or sorting
    the first table, we need a tmp table to compute the grouped rows.
    GROUP BY happens before windowing; so it is a pre-windowing tmp
    table.

    (4) (5) If there is DISTINCT, or ORDER BY which isn't resolved by using an
    index or sorting the first table, those clauses need an input tmp table.
    If we have windowing, as those clauses are used after windowing, they can
    use the last window's tmp table.

    (6) If there are different ORDER BY and GROUP BY orders, ORDER BY needs an
    input tmp table, so it's like (5).

    (7) If the user wants us to buffer the result, we need a tmp table. But
    windowing creates one anyway, and so does the materialization of a derived
    table.

    See also the computation of Temp_table_param::m_window_short_circuit,
    where we make sure to create a tmp table if the clauses above want one.

    (8) If the first windowing step needs sorting, filesort() will be used; it
    can sort one table but not a join of tables, so we need a tmp table
    then. If GROUP BY was optimized away, the pre-windowing result is 0 or 1
    row so doesn't need sorting.
  */

  if (rollup.state != ROLLUP::STATE_NONE &&  // (1)
      (select_distinct || has_windows || order))
    need_tmp_before_win = true;

  if (!plan_is_const())  // (2)
  {
    if ((group_list && !simple_group) ||               // (3)
        (!has_windows && (select_distinct ||           // (4)
                          (order && !simple_order) ||  // (5)
                          (group_list && order))) ||   // (6)
        ((select_lex->active_options() & OPTION_BUFFER_RESULT) &&
         !has_windows &&
         !(unit->derived_table &&
           unit->derived_table->uses_materialization())) ||     // (7)
        (has_windows && (primary_tables - const_tables) > 1 &&  // (8)
         m_windows[0]->needs_sorting() && !group_optimized_away))
      need_tmp_before_win = true;
  }

  DBUG_EXECUTE("info", TEST_join(this););

  if (!plan_is_const()) {
    // Test if we can use an index instead of sorting
    test_skip_sort();

    if (finalize_table_conditions()) return true;
  }

  if (alloc_qep(tables)) return (error = 1); /* purecov: inspected */

  if (make_join_readinfo(this, no_jbuf_after))
    return true; /* purecov: inspected */

  if (make_tmp_tables_info()) return true;

  /*
    At this stage, we have fully set QEP_TABs; JOIN_TABs are unaccessible,
    Query parts being offloaded to the engines(below) may still change the
    'plan', affecting which type of Iterator we should create. Thus no
    Iterators should be set up until after push_to_engine() has completed.
  */
  if (push_to_engines()) return true;

  /*
    If we decided to not sort after all, update the cost of the JOIN.
    Windowing sorts are handled elsewhere
  */
  if (sort_cost > 0.0 &&
      !explain_flags.any(ESP_USING_FILESORT, ESC_WINDOWING)) {
    best_read -= sort_cost;
    sort_cost = 0.0;
  }

  count_field_types(select_lex, &tmp_table_param, all_fields, false, false);

  // Create the basic table Iterators, and composite Iterators where supported.
  create_iterators();

  // Make plan visible for EXPLAIN
  set_plan_state(PLAN_READY);

  DEBUG_SYNC(thd, "after_join_optimize");

  error = 0;
  return false;

setup_subq_exit:

  DBUG_ASSERT(zero_result_cause != nullptr);
  DBUG_ASSERT(m_root_iterator != nullptr);
  /*
    Even with zero matching rows, subqueries in the HAVING clause may
    need to be evaluated if there are aggregate functions in the
    query. If this JOIN is part of an outer query, subqueries in HAVING may
    be evaluated several times in total; so subquery materialization makes
    sense.
  */
  child_subquery_can_materialize = true;
  trace_steps.end();  // because all steps are done
  Opt_trace_object(trace, "empty_result").add_alnum("cause", zero_result_cause);

  having_for_explain = having_cond;
  error = 0;

  if (!qep_tab && best_ref) {
    /*
      After creation of JOIN_TABs in make_join_plan(), we have shortcut due to
      some zero_result_cause. For simplification, if we have JOIN_TABs we
      want QEP_TABs too.
    */
    if (alloc_qep(tables)) return true; /* purecov: inspected */
    unplug_join_tabs();
  }

  set_plan_state(ZERO_RESULT);
  return false;
}

void JOIN::create_iterators_for_zero_rows() {
  if (send_row_on_empty_set()) {
    // Aggregate no rows into an aggregate row.
    m_root_iterator = NewIterator<ZeroRowsAggregatedIterator>(
        thd, zero_result_cause, select_lex->join, &examined_rows);
    m_root_iterator =
        attach_iterators_for_having_and_limit(move(m_root_iterator));
  } else {
    // Send no row at all (so also no need to check HAVING or LIMIT).
    m_root_iterator = NewIterator<ZeroRowsIterator>(thd, zero_result_cause,
                                                    /*child_iterator=*/nullptr);
  }
}

/**
  Push (parts of) the query execution down to the storage engines if they
  can provide faster execution of the query, or part of it.

  The handler will inspect the QEP through the
  AQP (Abstract Query Plan) and extract from it whatever
  it might implement of pushed execution.

  It is the responsibility of the handler to store
  any information it need for the later execution of
  pushed queries and conditions.

  @retval false Success.
  @retval true Error, error code saved in member JOIN::error.
*/
bool JOIN::push_to_engines() {
  DBUG_TRACE;

  if (!plan_is_const()) {
    const AQP::Join_plan plan(this);

    for (uint i = const_tables; i < plan.get_access_count(); i++) {
      TABLE *const table = qep_tab[i].table();
      if (likely(table != nullptr)) {
        if (unlikely(table->file->engine_push(plan.get_table_access(i)))) {
          return true;
        }
      }
    }
  }
  return false;
}

/**
  Substitute all expressions in the WHERE condition and ORDER/GROUP lists
  that match generated columns (GC) expressions with GC fields, if any.

  @details This function does 3 things:
  1) Creates list of all GC fields that are a part of a key and the GC
    expression is a function. All query tables are scanned. If there's no
    such fields, function exits.
  2) By means of Item::compile() WHERE clause is transformed.
    @see Item_func::gc_subst_transformer() for details.
  3) If there's ORDER/GROUP BY clauses, this function tries to substitute
    expressions in these lists with GC too. It removes from the list of
    indexed GC all elements which index blocked by hints. This is done to
    reduce amount of further work. Next it goes through ORDER/GROUP BY list
    and matches the expression in it against GC expressions in indexed GC
    list. When a match is found, the expression is replaced with a new
    Item_field for the matched GC field. Also, this new field is added to
    the hidden part of all_fields list.

  @param thd         thread handle
  @param select_lex  the current select
  @param where_cond  the WHERE condition, possibly NULL
  @param group_list  the GROUP BY clause, possibly NULL
  @param order       the ORDER BY clause, possibly NULL

  @return true if the GROUP BY clause or the ORDER BY clause was
          changed, false otherwise
*/

bool substitute_gc(THD *thd, SELECT_LEX *select_lex, Item *where_cond,
                   ORDER *group_list, ORDER *order) {
  List<Field> indexed_gc;
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object subst_gc(trace, "substitute_generated_columns");

  // Collect all GCs that are a part of a key
  for (TABLE_LIST *tl = select_lex->leaf_tables; tl; tl = tl->next_leaf) {
    if (tl->table->s->keys == 0) continue;
    for (uint i = 0; i < tl->table->s->fields; i++) {
      Field *fld = tl->table->field[i];
      if (fld->is_gcol() &&
          !(fld->part_of_key.is_clear_all() &&
            fld->part_of_prefixkey.is_clear_all()) &&
          fld->gcol_info->expr_item->can_be_substituted_for_gc()) {
        // Don't check allowed keys here as conditions/group/order use
        // different keymaps for that.
        indexed_gc.push_back(fld);
      }
    }
  }
  // No GC in the tables used in the query
  if (indexed_gc.elements == 0) return false;

  if (where_cond) {
    // Item_func::compile will dereference this pointer, provide valid value.
    uchar i, *dummy = &i;
    where_cond->compile(&Item::gc_subst_analyzer, &dummy,
                        &Item::gc_subst_transformer, (uchar *)&indexed_gc);
    subst_gc.add("resulting_condition", where_cond);
  }

  // An error occur during substitution. Let caller handle it.
  if (thd->is_error()) return false;

  if (!(group_list || order)) return false;
  // Filter out GCs that do not have index usable for GROUP/ORDER
  Field *gc;
  List_iterator<Field> li(indexed_gc);

  while ((gc = li++)) {
    Key_map tkm = gc->part_of_key;
    tkm.intersect(group_list ? gc->table->keys_in_use_for_group_by
                             : gc->table->keys_in_use_for_order_by);
    if (tkm.is_clear_all()) li.remove();
  }
  if (!indexed_gc.elements) return false;

  // Index could be used for ORDER only if there is no GROUP
  ORDER *list = group_list ? group_list : order;
  bool changed = false;
  for (ORDER *ord = list; ord; ord = ord->next) {
    li.rewind();
    if (!(*ord->item)->can_be_substituted_for_gc()) continue;
    while ((gc = li++)) {
      Item_func *tmp = pointer_cast<Item_func *>(*ord->item);
      Item_field *field;
      if ((field = get_gc_for_expr(&tmp, gc, gc->result_type()))) {
        changed = true;
        /* Add new field to field list. */
        ord->item = select_lex->add_hidden_item(field);
        break;
      }
    }
  }
  // An error occur during substitution. Let caller handle it.
  if (thd->is_error()) return false;

  if (changed && trace->is_started()) {
    String str;
    SELECT_LEX::print_order(
        thd, &str, list,
        enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                        QT_NO_DEFAULT_DB));
    subst_gc.add_utf8(group_list ? "resulting_GROUP_BY" : "resulting_ORDER_BY",
                      str.ptr(), str.length());
  }
  return changed;
}

/**
   Sets the plan's state of the JOIN. This is always the final step of
   optimization; starting from this call, we expose the plan to other
   connections (via EXPLAIN CONNECTION) so the plan has to be final.
   QEP_TAB's quick_optim, condition_optim and keyread_optim are set here.
*/
void JOIN::set_plan_state(enum_plan_state plan_state_arg) {
  // A plan should not change to another plan:
  DBUG_ASSERT(plan_state_arg == NO_PLAN || plan_state == NO_PLAN);
  if (plan_state == NO_PLAN && plan_state_arg != NO_PLAN) {
    if (qep_tab != nullptr) {
      /*
        We want to cover primary tables, tmp tables (they may have a sort, so
        their "quick" and "condition" may change when execution runs the
        sort), and sj-mat inner tables. Note that make_tmp_tables_info() may
        have added a sort to the first non-const primary table, so it's
        important to do those assignments after make_tmp_tables_info().
      */
      for (uint i = const_tables; i < tables; ++i) {
        qep_tab[i].set_quick_optim();
        qep_tab[i].set_condition_optim();
        qep_tab[i].set_keyread_optim();
      }
    }
  }

  DEBUG_SYNC(thd, "before_set_plan");

  // If SQLCOM_END, no thread is explaining our statement anymore.
  const bool need_lock = thd->query_plan.get_command() != SQLCOM_END;

  if (need_lock) thd->lock_query_plan();
  plan_state = plan_state_arg;
  if (need_lock) thd->unlock_query_plan();
}

bool JOIN::alloc_qep(uint n) {
  static_assert(MAX_TABLES <= INT_MAX8, "plan_idx needs to be wide enough.");

  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  qep_tab = new (thd->mem_root)
      QEP_TAB[n + 1];        // The last one holds only the final op_type.
  if (!qep_tab) return true; /* purecov: inspected */
  for (uint i = 0; i < n; ++i) qep_tab[i].init(best_ref[i]);
  return false;
}

void QEP_TAB::init(JOIN_TAB *jt) {
  jt->share_qs(this);
  set_table(table());  // to update table()->reginfo.qep_tab
  table_ref = jt->table_ref;
}

/// @returns semijoin strategy for this table.
uint QEP_TAB::get_sj_strategy() const {
  if (first_sj_inner() == NO_PLAN_IDX) return SJ_OPT_NONE;
  const uint s = join()->qep_tab[first_sj_inner()].position()->sj_strategy;
  DBUG_ASSERT(s != SJ_OPT_NONE);
  return s;
}

/**
  Return the index used for a table in a QEP

  The various access methods have different places where the index/key
  number is stored, so this function is needed to return the correct value.

  @returns index number, or MAX_KEY if not applicable.

  JT_SYSTEM and JT_ALL does not use an index, and will always return MAX_KEY.

  JT_INDEX_MERGE supports more than one index. Hence MAX_KEY is returned and
  a further inspection is needed.
*/
uint QEP_TAB::effective_index() const {
  switch (type()) {
    case JT_SYSTEM:
      DBUG_ASSERT(ref().key == -1);
      return MAX_KEY;

    case JT_CONST:
    case JT_EQ_REF:
    case JT_REF_OR_NULL:
    case JT_REF:
      DBUG_ASSERT(ref().key != -1);
      return uint(ref().key);

    case JT_INDEX_SCAN:
    case JT_FT:
      return index();

    case JT_INDEX_MERGE:
      DBUG_ASSERT(quick()->index == MAX_KEY);
      return MAX_KEY;

    case JT_RANGE:
      return quick()->index;

    case JT_ALL:
    default:
      // @todo Check why JT_UNKNOWN is a valid value here.
      DBUG_ASSERT(type() == JT_ALL || type() == JT_UNKNOWN);
      return MAX_KEY;
  }
}

uint JOIN_TAB::get_sj_strategy() const {
  if (first_sj_inner() == NO_PLAN_IDX) return SJ_OPT_NONE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(join());
  JOIN_TAB *tab = join()->best_ref[first_sj_inner()];
  uint s = tab->position()->sj_strategy;
  DBUG_ASSERT(s != SJ_OPT_NONE);
  return s;
}

int JOIN::replace_index_subquery() {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  if (group_list ||
      !(unit->item && unit->item->substype() == Item_subselect::IN_SUBS) ||
      primary_tables != 1 || !where_cond || unit->is_union())
    return 0;

  // Guaranteed by remove_redundant_subquery_clauses():
  DBUG_ASSERT(order == nullptr && !select_distinct);

  Item_in_subselect *const in_subs =
      static_cast<Item_in_subselect *>(unit->item);
  bool found_engine = false;

  JOIN_TAB *const first_join_tab = best_ref[0];

  if (in_subs->exec_method == SubqueryExecMethod::EXEC_MATERIALIZATION) {
    // We cannot have two engines at the same time
  } else if (first_join_tab->table_ref->is_view_or_derived() &&
             first_join_tab->table_ref->derived_unit()->is_recursive()) {
    // The index subquery engine, which runs the derived table machinery
    // from the old executor, is not capable of materializing a WITH RECURSIVE
    // query from the iterator executor. Thus, be conservative here, so that the
    // case never happens.
  } else if (having_cond == nullptr) {
    const join_type type = first_join_tab->type();
    if ((type == JT_EQ_REF || type == JT_REF) &&
        first_join_tab->ref().items[0]->item_name.ptr() == in_left_expr_name) {
      found_engine = true;
    }
  } else if (first_join_tab->type() == JT_REF_OR_NULL &&
             first_join_tab->ref().items[0]->item_name.ptr() ==
                 in_left_expr_name &&
             having_cond->created_by_in2exists()) {
    found_engine = true;
  }

  if (!found_engine) return 0;

  /* Remove redundant predicates and cache constant expressions  */
  if (finalize_table_conditions()) return -1;

  if (alloc_qep(tables)) return -1; /* purecov: inspected */
  unplug_join_tabs();

  error = 0;
  QEP_TAB *const first_qep_tab = &qep_tab[0];

  if (first_qep_tab->table()->covering_keys.is_set(first_qep_tab->ref().key)) {
    DBUG_ASSERT(!first_qep_tab->table()->no_keyread);
    first_qep_tab->table()->set_keyread(true);
  }

  subselect_indexsubquery_engine *engine = new (thd->mem_root)
      subselect_indexsubquery_engine(first_qep_tab,
                                     down_cast<Item_in_subselect *>(unit->item),
                                     first_qep_tab->condition(), having_cond);
  unit->item->set_indexsubquery_engine(engine);
  return 1;
}

bool JOIN::optimize_distinct_group_order() {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  const bool windowing = m_windows.elements > 0;
  const bool may_trace = select_distinct || group_list || order || windowing ||
                         tmp_table_param.sum_func_count;
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_disable_I_S trace_disabled(trace, !may_trace);
  Opt_trace_object wrapper(trace);
  Opt_trace_object trace_opt(trace, "optimizing_distinct_group_by_order_by");
  /* Optimize distinct away if possible */
  {
    ORDER *org_order = order;
    order = ORDER_with_src(
        remove_const(order, where_cond, rollup.state == ROLLUP::STATE_NONE,
                     &simple_order, false),
        order.src);
    if (thd->is_error()) {
      error = 1;
      DBUG_PRINT("error", ("Error from remove_const"));
      return true;
    }

    /*
      If we are using ORDER BY NULL or ORDER BY const_expression,
      return result in any order (even if we are using a GROUP BY)
    */
    if (!order && org_order) skip_sort_order = true;
  }
  /*
     Check if we can optimize away GROUP BY/DISTINCT.
     We can do that if there are no aggregate functions, the
     fields in DISTINCT clause (if present) and/or columns in GROUP BY
     (if present) contain direct references to all key parts of
     an unique index (in whatever order) and if the key parts of the
     unique index cannot contain NULLs.
     Note that the unique keys for DISTINCT and GROUP BY should not
     be the same (as long as they are unique).

     The FROM clause must contain a single non-constant table.

     @todo Apart from the LIS test, every condition depends only on facts
     which can be known in SELECT_LEX::prepare(), possibly this block should
     move there.
  */

  JOIN_TAB *const tab = best_ref[const_tables];

  if (plan_is_single_table() && (group_list || select_distinct) &&
      !tmp_table_param.sum_func_count &&
      (!tab->quick() ||
       tab->quick()->get_type() != QUICK_SELECT_I::QS_TYPE_GROUP_MIN_MAX)) {
    if (group_list && rollup.state == ROLLUP::STATE_NONE &&
        list_contains_unique_index(tab, find_field_in_order_list,
                                   (void *)group_list)) {
      /*
        We have found that grouping can be removed since groups correspond to
        only one row anyway.
      */
      group_list = nullptr;
      grouped = false;
    }
    if (select_distinct &&
        list_contains_unique_index(tab, find_field_in_item_list,
                                   (void *)&fields_list)) {
      select_distinct = false;
      trace_opt.add("distinct_is_on_unique", true)
          .add("removed_distinct", true);
    }
  }
  if (!(group_list || tmp_table_param.sum_func_count || windowing) &&
      select_distinct && plan_is_single_table() &&
      rollup.state == ROLLUP::STATE_NONE) {
    int order_idx = -1, group_idx = -1;
    /*
      We are only using one table. In this case we change DISTINCT to a
      GROUP BY query if:
      - The GROUP BY can be done through indexes (no sort) and the ORDER
        BY only uses selected fields.
        (In this case we can later optimize away GROUP BY and ORDER BY)
      - We are scanning the whole table without LIMIT
        This can happen if:
        - We are using CALC_FOUND_ROWS
        - We are using an ORDER BY that can't be optimized away.
      - Selected expressions are not set functions (those cannot be put
      into GROUP BY).

      We don't want to use this optimization when we are using LIMIT
      because in this case we can just create a temporary table that
      holds LIMIT rows and stop when this table is full.
    */
    if (order) {
      skip_sort_order = test_if_skip_sort_order(
          tab, order, m_select_limit,
          true,  // no_changes
          &tab->table()->keys_in_use_for_order_by, &order_idx);
      count_field_types(select_lex, &tmp_table_param, all_fields, false, false);
    }
    ORDER *o;
    bool all_order_fields_used;
    if ((o = create_order_from_distinct(thd, ref_items[REF_SLICE_ACTIVE], order,
                                        fields_list, /*skip_aggregates=*/true,
                                        /*convert_bit_fields_to_long=*/true,
                                        &all_order_fields_used))) {
      group_list = ORDER_with_src(o, ESC_DISTINCT);
      const bool skip_group =
          skip_sort_order &&
          test_if_skip_sort_order(tab, group_list, m_select_limit,
                                  true,  // no_changes
                                  &tab->table()->keys_in_use_for_group_by,
                                  &group_idx);
      count_field_types(select_lex, &tmp_table_param, all_fields, false, false);
      // ORDER BY and GROUP BY are using different indexes, can't skip sorting
      if (group_idx >= 0 && order_idx >= 0 && group_idx != order_idx)
        skip_sort_order = false;
      if ((skip_group && all_order_fields_used) ||
          m_select_limit == HA_POS_ERROR || (order && !skip_sort_order)) {
        /*  Change DISTINCT to GROUP BY */
        select_distinct = false;
        /*
          group_list was created with ORDER BY clause as prefix and
          replaces it. So it must respect ordering. If there is no
          ORDER BY, GROUP BY need not have to provide order.
        */
        if (!order) {
          for (ORDER *group = group_list; group; group = group->next)
            group->direction = ORDER_NOT_RELEVANT;
        }
        if (all_order_fields_used && skip_sort_order && order) {
          /*
            Force MySQL to read the table in sorted order to get result in
            ORDER BY order.
          */
          tmp_table_param.allow_group_via_temp_table = false;
        }
        grouped = true;  // For end_write_group
        trace_opt.add("changed_distinct_to_group_by", true);
      } else
        group_list = nullptr;
    } else if (thd->is_fatal_error())  // End of memory
      return true;
  }
  simple_group = false;

  ORDER *old_group_list = group_list;
  group_list = ORDER_with_src(
      remove_const(group_list, where_cond, rollup.state == ROLLUP::STATE_NONE,
                   &simple_group, true),
      group_list.src);

  if (thd->is_error()) {
    error = 1;
    DBUG_PRINT("error", ("Error from remove_const"));
    return true;
  }
  if (old_group_list && !group_list) select_distinct = false;

  if (!group_list && grouped) {
    order = nullptr;  // The output has only one row
    simple_order = true;
    select_distinct = false;  // No need in distinct for 1 row
    group_optimized_away = true;
  }

  calc_group_buffer(this, group_list);
  send_group_parts = tmp_table_param.group_parts; /* Save org parts */

  /*
     If ORDER BY is a prefix of GROUP BY and if windowing or ROLLUP
     doesn't change this order, ORDER BY can be removed and we can
     enforce GROUP BY to provide order.
     Also true if the result is one row.
  */
  if ((test_if_subpart(group_list, order) && !m_windows_sort &&
       select_lex->olap != ROLLUP_TYPE) ||
      (!group_list && tmp_table_param.sum_func_count)) {
    if (order) {
      order = nullptr;
      trace_opt.add("removed_order_by", true);
    }
    if (is_indexed_agg_distinct(this, nullptr)) streaming_aggregation = false;
  }

  return false;
}

void JOIN::test_skip_sort() {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  JOIN_TAB *const tab = best_ref[const_tables];

  DBUG_ASSERT(m_ordered_index_usage == ORDERED_INDEX_VOID);

  if (group_list)  // GROUP BY honoured first
                   // (DISTINCT was rewritten to GROUP BY if skippable)
  {
    /*
      When there is SQL_BIG_RESULT or a JSON aggregation function,
      do not sort using index for GROUP BY, and thus force sorting on disk
      unless a group min-max optimization is going to be used as it is applied
      now only for one table queries with covering indexes.
    */
    if (!(select_lex->active_options() & SELECT_BIG_RESULT || with_json_agg) ||
        (tab->quick() &&
         tab->quick()->get_type() == QUICK_SELECT_I::QS_TYPE_GROUP_MIN_MAX)) {
      if (simple_group &&    // GROUP BY is possibly skippable
          !select_distinct)  // .. if not preceded by a DISTINCT
      {
        /*
          Calculate a possible 'limit' of table rows for 'GROUP BY':
          A specified 'LIMIT' is relative to the final resultset.
          'need_tmp' implies that there will be more postprocessing
          so the specified 'limit' should not be enforced yet.
         */
        const ha_rows limit =
            (need_tmp_before_win ? HA_POS_ERROR : m_select_limit);
        int dummy;

        if (test_if_skip_sort_order(tab, group_list, limit, false,
                                    &tab->table()->keys_in_use_for_group_by,
                                    &dummy)) {
          m_ordered_index_usage = ORDERED_INDEX_GROUP_BY;
        }
      }

      /*
        If we are going to use semi-join LooseScan, it will depend
        on the selected index scan to be used.  If index is not used
        for the GROUP BY, we risk that sorting is put on the LooseScan
        table.  In order to avoid this, force use of temporary table.
        TODO: Explain the allow_group_via_temp_table part of the test below.
       */
      if ((m_ordered_index_usage != ORDERED_INDEX_GROUP_BY) &&
          (tmp_table_param.allow_group_via_temp_table ||
           (tab->emb_sj_nest &&
            tab->position()->sj_strategy == SJ_OPT_LOOSE_SCAN))) {
        need_tmp_before_win = true;
        simple_order = simple_group = false;  // Force tmp table without sort
      }
    }
  } else if (order &&  // ORDER BY wo/ preceding GROUP BY
             (simple_order ||
              skip_sort_order) &&  // which is possibly skippable,
             !m_windows_sort)      // and WFs will not shuffle rows
  {
    int dummy;
    if ((skip_sort_order = test_if_skip_sort_order(
             tab, order, m_select_limit, false,
             &tab->table()->keys_in_use_for_order_by, &dummy))) {
      m_ordered_index_usage = ORDERED_INDEX_ORDER_BY;
    }
  }
}

/**
  Test if ORDER BY is a single MATCH function(ORDER BY MATCH)
  and sort order is descending.

  @param order                 pointer to ORDER struct.

  @retval
    Pointer to MATCH function if order is 'ORDER BY MATCH() DESC'
  @retval
    NULL otherwise
*/

static Item_func_match *test_if_ft_index_order(ORDER *order) {
  if (order && order->next == nullptr && order->direction == ORDER_DESC &&
      (*order->item)->type() == Item::FUNC_ITEM &&
      down_cast<Item_func *>(*order->item)->functype() == Item_func::FT_FUNC)
    return down_cast<Item_func_match *>(*order->item)->get_master();

  return nullptr;
}

/**
  Test if one can use the key to resolve ordering.

  @param order_src           Sort order
  @param table               Table to sort
  @param idx                 Index to check
  @param[out] used_key_parts NULL by default, otherwise return value for
                             used key parts.
  @param[out] skip_quick     Whether found index can be used for backward range
                             scans

  @note
    used_key_parts is set to correct key parts used if return value != 0
    (On other cases, used_key_part may be changed)
    Note that the value may actually be greater than the number of index
    key parts. This can happen for storage engines that have the primary
    key parts as a suffix for every secondary key.

  @retval
    1   key is ok.
  @retval
    0   Key can't be used
  @retval
    -1   Reverse key can be used
*/

int test_if_order_by_key(ORDER_with_src *order_src, TABLE *table, uint idx,
                         uint *used_key_parts, bool *skip_quick) {
  DBUG_TRACE;
  KEY_PART_INFO *key_part, *key_part_end;
  key_part = table->key_info[idx].key_part;
  key_part_end = key_part + table->key_info[idx].user_defined_key_parts;
  key_part_map const_key_parts = table->const_key_parts[idx];
  int reverse = 0;
  uint key_parts;
  bool on_pk_suffix = false;
  // Whether [extented] key has key parts with mixed ASC/DESC order
  bool mixed_order = false;
  // Order direction of the first key part
  bool reverse_sorted = (bool)(key_part->key_part_flag & HA_REVERSE_SORT);
  ORDER *order = *order_src;
  *skip_quick = false;

  for (; order; order = order->next, const_key_parts >>= 1) {
    /*
      Since only fields can be indexed, ORDER BY <something> that is
      not a field cannot be resolved by using an index.
    */
    Item *real_itm = (*order->item)->real_item();
    if (real_itm->type() != Item::FIELD_ITEM) return 0;

    const Field *field = down_cast<const Item_field *>(real_itm)->field;

    /*
      Skip key parts that are constants in the WHERE clause.
      These are already skipped in the ORDER BY by const_expression_in_where()
    */
    for (; const_key_parts & 1 && key_part < key_part_end;
         const_key_parts >>= 1)
      key_part++;

    if (key_part == key_part_end) {
      /*
        We are at the end of the key. Check if the engine has the primary
        key as a suffix to the secondary keys. If it has continue to check
        the primary key as a suffix.
      */
      if (!on_pk_suffix &&
          (table->file->ha_table_flags() & HA_PRIMARY_KEY_IN_READ_INDEX) &&
          table->s->primary_key != MAX_KEY && table->s->primary_key != idx) {
        on_pk_suffix = true;
        key_part = table->key_info[table->s->primary_key].key_part;
        key_part_end =
            key_part +
            table->key_info[table->s->primary_key].user_defined_key_parts;
        const_key_parts = table->const_key_parts[table->s->primary_key];

        for (; const_key_parts & 1 && key_part < key_part_end;
             const_key_parts >>= 1)
          key_part++;
        /*
         The primary and secondary key parts were all const (i.e. there's
         one row).  The sorting doesn't matter.
        */
        if (key_part == key_part_end && reverse == 0) {
          key_parts = 0;
          reverse = 1;
          goto ok;
        }
      } else
        return 0;
    }

    if (key_part->field != field || !field->part_of_sortkey.is_set(idx))
      return 0;
    if (order->direction != ORDER_NOT_RELEVANT) {
      const enum_order keypart_order =
          (key_part->key_part_flag & HA_REVERSE_SORT) ? ORDER_DESC : ORDER_ASC;
      /* set flag to 1 if we can use read-next on key, else to -1 */
      int cur_scan_dir = (order->direction == keypart_order) ? 1 : -1;
      if (reverse && cur_scan_dir != reverse) return 0;
      reverse = cur_scan_dir;  // Remember if reverse
    }
    mixed_order |=
        (reverse_sorted != (bool)((key_part)->key_part_flag & HA_REVERSE_SORT));

    key_part++;
  }
  /*
   The index picked here might be used for range scans with multiple ranges.
   This will require tricky reordering in case of ranges would have to be
   scanned backward and index consists of mixed ASC/DESC key parts. Due to that
   backward scans on such indexes are disabled.
  */
  if (mixed_order && reverse < 0) *skip_quick = true;

  if (!reverse) {
    /*
      We get here when the key is suitable and we don't care about it's
      order, i.e. GROUP BY/DISTINCT. Use forward scan.
    */
    reverse = 1;
  }
  if (on_pk_suffix) {
    uint used_key_parts_secondary = table->key_info[idx].user_defined_key_parts;
    uint used_key_parts_pk =
        (uint)(key_part - table->key_info[table->s->primary_key].key_part);
    key_parts = used_key_parts_pk + used_key_parts_secondary;

    if (reverse == -1 &&
        (!(table->file->index_flags(idx, used_key_parts_secondary - 1, true) &
           HA_READ_PREV) ||
         !(table->file->index_flags(table->s->primary_key,
                                    used_key_parts_pk - 1, true) &
           HA_READ_PREV)))
      reverse = 0;  // Index can't be used
  } else {
    key_parts = (uint)(key_part - table->key_info[idx].key_part);
    if (reverse == -1 &&
        !(table->file->index_flags(idx, key_parts - 1, true) & HA_READ_PREV))
      reverse = 0;  // Index can't be used
  }
ok:
  if (used_key_parts != nullptr) *used_key_parts = key_parts;
  return reverse;
}

/**
  Find shortest key suitable for full table scan.

  @param table                 Table to scan
  @param usable_keys           Allowed keys

  @note
     As far as
     1) clustered primary key entry data set is a set of all record
        fields (key fields and not key fields) and
     2) secondary index entry data is a union of its key fields and
        primary key fields (at least InnoDB and its derivatives don't
        duplicate primary key fields there, even if the primary and
        the secondary keys have a common subset of key fields),
     then secondary index entry data is always a subset of primary key entry.
     Unfortunately, key_info[nr].key_length doesn't show the length
     of key/pointer pair but a sum of key field lengths only, thus
     we can't estimate index IO volume comparing only this key_length
     value of secondary keys and clustered PK.
     So, try secondary keys first, and choose PK only if there are no
     usable secondary covering keys or found best secondary key include
     all table fields (i.e. same as PK):

  @return
    MAX_KEY     no suitable key found
    key index   otherwise
*/

uint find_shortest_key(TABLE *table, const Key_map *usable_keys) {
  uint best = MAX_KEY;
  uint usable_clustered_pk = (table->file->primary_key_is_clustered() &&
                              table->s->primary_key != MAX_KEY &&
                              usable_keys->is_set(table->s->primary_key))
                                 ? table->s->primary_key
                                 : MAX_KEY;
  if (!usable_keys->is_clear_all()) {
    uint min_length = (uint)~0;
    for (uint nr = 0; nr < table->s->keys; nr++) {
      if (nr == usable_clustered_pk) continue;
      if (usable_keys->is_set(nr)) {
        /*
          Can not do full index scan on rtree index because it is not
          supported by Innodb, probably not supported by others either.
          A multi-valued key requires unique filter, and won't be the most
          fast option even if it will be the shortest one.
         */
        const KEY &key_ref = table->key_info[nr];
        DBUG_ASSERT(!(key_ref.flags & HA_MULTI_VALUED_KEY));
        if (key_ref.key_length < min_length && !(key_ref.flags & HA_SPATIAL)) {
          min_length = key_ref.key_length;
          best = nr;
        }
      }
    }
  }
  if (usable_clustered_pk != MAX_KEY) {
    /*
     If the primary key is clustered and found shorter key covers all table
     fields then primary key scan normally would be faster because amount of
     data to scan is the same but PK is clustered.
     It's safe to compare key parts with table fields since duplicate key
     parts aren't allowed.
     */
    if (best == MAX_KEY ||
        table->key_info[best].user_defined_key_parts >= table->s->fields)
      best = usable_clustered_pk;
  }
  return best;
}

/**
  Test if a second key is the subkey of the first one.

  @param key_part              First key parts
  @param ref_key_part          Second key parts
  @param ref_key_part_end      Last+1 part of the second key

  @note
    Second key MUST be shorter than the first one.

  @retval
    1	is a subkey
  @retval
    0	no sub key
*/

inline bool is_subkey(KEY_PART_INFO *key_part, KEY_PART_INFO *ref_key_part,
                      KEY_PART_INFO *ref_key_part_end) {
  for (; ref_key_part < ref_key_part_end; key_part++, ref_key_part++)
    if (!key_part->field->eq(ref_key_part->field)) return false;
  return true;
}

/**
  Test if REF_OR_NULL optimization will be used if the specified
  ref_key is used for REF-access to 'tab'

  @retval
    true	JT_REF_OR_NULL will be used
  @retval
    false	no JT_REF_OR_NULL access
*/

static bool is_ref_or_null_optimized(const JOIN_TAB *tab, uint ref_key) {
  if (tab->keyuse()) {
    const Key_use *keyuse = tab->keyuse();
    while (keyuse->key != ref_key && keyuse->table_ref == tab->table_ref)
      keyuse++;

    const table_map const_tables = tab->join()->const_table_map;
    while (keyuse->key == ref_key && keyuse->table_ref == tab->table_ref) {
      if (!(keyuse->used_tables & ~const_tables)) {
        if (keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL) return true;
      }
      keyuse++;
    }
  }
  return false;
}

/**
  Test if we can use one of the 'usable_keys' instead of 'ref' key
  for sorting.

  @param order The query block's order clause.
  @param tab   Current JOIN_TAB.
  @param ref			Number of key, used for WHERE clause
  @param ref_key_parts Index columns used for ref lookup.
  @param usable_keys		Keys for testing

  @return
    - MAX_KEY			If we can't use other key
    - the number of found key	Otherwise
*/

static uint test_if_subkey(ORDER_with_src *order, JOIN_TAB *tab, uint ref,
                           uint ref_key_parts, const Key_map *usable_keys) {
  uint nr;
  uint min_length = (uint)~0;
  uint best = MAX_KEY;
  TABLE *table = tab->table();
  KEY_PART_INFO *ref_key_part = table->key_info[ref].key_part;
  KEY_PART_INFO *ref_key_part_end = ref_key_part + ref_key_parts;

  for (nr = 0; nr < table->s->keys; nr++) {
    bool skip_quick;
    if (usable_keys->is_set(nr) &&
        table->key_info[nr].key_length < min_length &&
        table->key_info[nr].user_defined_key_parts >= ref_key_parts &&
        is_subkey(table->key_info[nr].key_part, ref_key_part,
                  ref_key_part_end) &&
        !is_ref_or_null_optimized(tab, nr) &&
        test_if_order_by_key(order, table, nr, nullptr, &skip_quick) &&
        !skip_quick) {
      min_length = table->key_info[nr].key_length;
      best = nr;
    }
  }
  return best;
}

/**
  It is not obvious to see that test_if_skip_sort_order() never changes the
  plan if no_changes is true. So we double-check: creating an instance of this
  class saves some important access-path-related information of the current
  table; when the instance is destroyed, the latest access-path information is
  compared with saved data.
*/

class Plan_change_watchdog {
#ifndef DBUG_OFF
 public:
  /**
    @param tab_arg     table whose access path is being determined
    @param no_changes_arg whether a change to the access path is allowed
  */
  Plan_change_watchdog(const JOIN_TAB *tab_arg, const bool no_changes_arg) {
    if (no_changes_arg) {
      tab = tab_arg;
      type = tab->type();
      if ((quick = tab->quick())) quick_index = quick->index;
      use_quick = tab->use_quick;
      ref_key = tab->ref().key;
      ref_key_parts = tab->ref().key_parts;
      index = tab->index();
    } else {
      tab = nullptr;
      type = JT_UNKNOWN;
      quick = nullptr;
      ref_key = ref_key_parts = index = 0;
      use_quick = QS_NONE;
    }
  }
  ~Plan_change_watchdog() {
    if (tab == nullptr) return;
    // changes are not allowed, we verify:
    DBUG_ASSERT(tab->type() == type);
    DBUG_ASSERT(tab->quick() == quick);
    DBUG_ASSERT((quick == nullptr) || tab->quick()->index == quick_index);
    DBUG_ASSERT(tab->use_quick == use_quick);
    DBUG_ASSERT(tab->ref().key == ref_key);
    DBUG_ASSERT(tab->ref().key_parts == ref_key_parts);
    DBUG_ASSERT(tab->index() == index);
  }

 private:
  const JOIN_TAB *tab;  ///< table, or NULL if changes are allowed
  enum join_type type;  ///< copy of tab->type()
  // "Range / index merge" info
  const QUICK_SELECT_I *quick{nullptr};  ///< copy of tab->select->quick
  uint quick_index{0};                   ///< copy of tab->select->quick->index
  enum quick_type use_quick;             ///< copy of tab->use_quick
  // "ref access" info
  int ref_key;         ///< copy of tab->ref().key
  uint ref_key_parts;  /// copy of tab->ref().key_parts
  // Other index-related info
  uint index;  ///< copy of tab->index
#else          // in non-debug build, empty class
 public:
  Plan_change_watchdog(const JOIN_TAB *, const bool) {}
#endif
};

/**
  Test if we can skip ordering by using an index.

  If the current plan is to use an index that provides ordering, the
  plan will not be changed. Otherwise, if an index can be used, the
  JOIN_TAB / tab->select struct is changed to use the index.

  The index must cover all fields in @<order@>, or it will not be considered.

  @param tab           NULL or JOIN_TAB of the accessed table
  @param order         Linked list of ORDER BY arguments
  @param select_limit  LIMIT value, or HA_POS_ERROR if no limit
  @param no_changes    No changes will be made to the query plan.
  @param map           Key_map of applicable indexes.
  @param [out] order_idx Number of index selected, -1 if no applicable index
                       found

  @todo
    - sergeyp: Results of all index merge selects actually are ordered
    by clustered PK values.

  @note
  This function may change tmp_table_param.precomputed_group_by. This
  affects how create_tmp_table() treats aggregation functions, so
  count_field_types() must be called again to make sure this is taken
  into consideration.

  @retval
    0    We have to use filesort to do the sorting
  @retval
    1    We can use an index.
*/

static bool test_if_skip_sort_order(JOIN_TAB *tab, ORDER_with_src &order,
                                    ha_rows select_limit, const bool no_changes,
                                    const Key_map *map, int *order_idx) {
  DBUG_TRACE;
  int ref_key;
  uint ref_key_parts = 0;
  int order_direction = 0;
  uint used_key_parts = 0;
  TABLE *const table = tab->table();
  JOIN *const join = tab->join();
  THD *const thd = join->thd;
  QUICK_SELECT_I *const save_quick = tab->quick();
  int best_key = -1;
  bool set_up_ref_access_to_key = false;
  bool can_skip_sorting = false;  // used as return value
  int changed_key = -1;

  /* Check that we are always called with first non-const table */
  DBUG_ASSERT((uint)tab->idx() == join->const_tables);

  Plan_change_watchdog watchdog(tab, no_changes);
  *order_idx = -1;
  /* Sorting a single row can always be skipped */
  if (tab->type() == JT_EQ_REF || tab->type() == JT_CONST ||
      tab->type() == JT_SYSTEM) {
    return true;
  }

  /*
    Check if FT index can be used to retrieve result in the required order.
    It is possible if ordering is on the first non-constant table.
  */
  if (join->order && join->simple_order) {
    /*
      Check if ORDER is DESC, ORDER BY is a single MATCH function.
    */
    Item_func_match *ft_func = test_if_ft_index_order(order);
    /*
      Two possible cases when we can skip sort order:
      1. FT_SORTED must be set(Natural mode, no ORDER BY).
      2. If FT_SORTED flag is not set then
      the engine should support deferred sorting. Deferred sorting means
      that sorting is postponed utill the start of index reading(InnoDB).
      In this case we set FT_SORTED flag here to let the engine know that
      internal sorting is needed.
    */
    if (ft_func && ft_func->ft_handler && ft_func->ordered_result()) {
      /*
        FT index scan is used, so the only additional requirement is
        that ORDER BY MATCH function is the same as the function that
        is used for FT index.
      */
      if (tab->type() == JT_FT &&
          ft_func->eq(tab->position()->key->val, true)) {
        ft_func->set_hints(join, FT_SORTED, select_limit, false);
        return true;
      }
      /*
        No index is used, it's possible to use FT index for ORDER BY if
        LIMIT is present and does not exceed count of the records in FT index
        and there is no WHERE condition since a condition may potentially
        require more rows to be fetch from FT index.
      */
      else if (!tab->condition() && select_limit != HA_POS_ERROR &&
               select_limit <= ft_func->get_count()) {
        /* test_if_ft_index_order() always returns master MATCH function. */
        DBUG_ASSERT(!ft_func->master);
        /* ref is not set since there is no WHERE condition */
        DBUG_ASSERT(tab->ref().key == -1);

        /*Make EXPLAIN happy */
        tab->set_type(JT_FT);
        tab->ref().key = ft_func->key;
        tab->ref().key_parts = 0;
        tab->set_index(ft_func->key);
        tab->set_ft_func(ft_func);

        /* Setup FT handler */
        ft_func->set_hints(join, FT_SORTED, select_limit, true);
        ft_func->join_key = true;
        table->file->ft_handler = ft_func->ft_handler;
        return true;
      }
    }
  }

  /*
    Keys disabled by ALTER TABLE ... DISABLE KEYS should have already
    been taken into account.
  */
  Key_map usable_keys = *map;

  for (ORDER *tmp_order = order; tmp_order; tmp_order = tmp_order->next) {
    const Item *item = (*tmp_order->item)->real_item();
    if (item->type() != Item::FIELD_ITEM) {
      usable_keys.clear_all();
      return false;
    }
    usable_keys.intersect(
        down_cast<const Item_field *>(item)->field->part_of_sortkey);
    if (usable_keys.is_clear_all()) return false;  // No usable keys
  }
  if (tab->type() == JT_REF_OR_NULL || tab->type() == JT_FT) return false;

  ref_key = -1;
  /* Test if constant range in WHERE */
  if (tab->type() == JT_REF) {
    DBUG_ASSERT(tab->ref().key >= 0 && tab->ref().key_parts);
    ref_key = tab->ref().key;
    ref_key_parts = tab->ref().key_parts;
  } else if (tab->type() == JT_RANGE || tab->type() == JT_INDEX_MERGE) {
    // Range found by opt_range
    int quick_type = tab->quick()->get_type();
    /*
      assume results are not ordered when index merge is used
      TODO: sergeyp: Results of all index merge selects actually are ordered
      by clustered PK values.
    */

    if (quick_type == QUICK_SELECT_I::QS_TYPE_INDEX_MERGE ||
        quick_type == QUICK_SELECT_I::QS_TYPE_ROR_UNION ||
        quick_type == QUICK_SELECT_I::QS_TYPE_ROR_INTERSECT)
      return false;
    ref_key = tab->quick()->index;
    ref_key_parts = tab->quick()->used_key_parts;
  } else if (tab->type() == JT_INDEX_SCAN) {
    // The optimizer has decided to use an index scan.
    ref_key = tab->index();
    ref_key_parts = actual_key_parts(&table->key_info[tab->index()]);
  }

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper_1(trace);
  Opt_trace_object trace_skip_sort_order(
      trace, "reconsidering_access_paths_for_index_ordering");
  trace_skip_sort_order.add_alnum(
      "clause", (order.src == ESC_ORDER_BY ? "ORDER BY" : "GROUP BY"));
  Opt_trace_array trace_steps(trace, "steps");

  if (ref_key >= 0) {
    /*
      We come here when ref/index scan/range scan access has been set
      up for this table. Do not change access method if ordering is
      provided already.
    */
    if (!usable_keys.is_set(ref_key)) {
      /*
        We come here when ref_key is not among usable_keys, try to find a
        usable prefix key of that key.
      */
      uint new_ref_key;
      /*
        If using index only read, only consider other possible index only
        keys
      */
      if (table->covering_keys.is_set(ref_key))
        usable_keys.intersect(table->covering_keys);

      if ((new_ref_key = test_if_subkey(&order, tab, ref_key, ref_key_parts,
                                        &usable_keys)) < MAX_KEY) {
        /* Found key that can be used to retrieve data in sorted order */
        if (tab->ref().key >= 0) {
          /*
            We'll use ref access method on key new_ref_key. The actual change
            is done further down in this function where we update the plan.
          */
          set_up_ref_access_to_key = true;
        } else if (!no_changes) {
          /*
            The range optimizer constructed QUICK_RANGE for ref_key, and
            we want to use instead new_ref_key as the index. We can't
            just change the index of the quick select, because this may
            result in an incosistent QUICK_SELECT object. Below we
            create a new QUICK_SELECT from scratch so that all its
            parameres are set correctly by the range optimizer.

            Note that the range optimizer is NOT called if
            no_changes==true. This reason is that the range optimizer
            cannot find a QUICK that can return ordered result unless
            index access (ref or index scan) is also able to do so
            (which test_if_order_by_key () will tell).
            Admittedly, range access may be much more efficient than
            e.g. index scan, but the only thing that matters when
            no_change==true is the answer to the question: "Is it
            possible to avoid sorting if an index is used to access
            this table?". The answer does not depend on the outcome of
            the range optimizer.
          */
          Key_map new_ref_key_map;  // Force the creation of quick select
          new_ref_key_map.set_bit(new_ref_key);  // only for new_ref_key.

          Opt_trace_object trace_wrapper_2(trace);
          Opt_trace_object trace_recest(trace, "rows_estimation");
          trace_recest.add_utf8_table(tab->table_ref)
              .add_utf8("index", table->key_info[new_ref_key].name);
          QUICK_SELECT_I *qck;
          const bool no_quick =
              test_quick_select(thd, new_ref_key_map,
                                0,  // empty table_map
                                join->calc_found_rows
                                    ? HA_POS_ERROR
                                    : join->unit->select_limit_cnt,
                                false,  // don't force quick range
                                order->direction, tab,
                                // we are after make_join_select():
                                tab->condition(), &tab->needed_reg, &qck,
                                tab->table()->force_index) <= 0;
          DBUG_ASSERT(tab->quick() == save_quick);
          tab->set_quick(qck);
          if (no_quick) {
            can_skip_sorting = false;
            goto fix_ICP;
          }
        }
        ref_key = new_ref_key;
        changed_key = new_ref_key;
      }
    }
    bool dummy;
    /* Check if we get the rows in requested sorted order by using the key */
    if (usable_keys.is_set(ref_key))
      // Last parameter can be ignored as it'll be checked later, if needed
      order_direction =
          test_if_order_by_key(&order, table, ref_key, &used_key_parts, &dummy);
  }
  if (ref_key < 0 || order_direction <= 0) {
    /*
      There is no ref/index scan/range scan access set up for this
      table, or it does not provide the requested ordering, or it uses
      backward scan. Do a cost-based search on all keys.
    */
    uint best_key_parts = 0;
    uint saved_best_key_parts = 0;
    int best_key_direction = 0;
    ha_rows table_records = table->file->stats.records;

    /*
      If an index scan that cannot provide ordering has been selected
      then do not use the index scan key as starting hint to
      test_if_cheaper_ordering()
    */
    const int ref_key_hint =
        (order_direction == 0 && tab->type() == JT_INDEX_SCAN) ? -1 : ref_key;

    // Does the query have a "FORCE INDEX [FOR GROUP BY] (idx)" (if clause is
    // group by) or a "FORCE INDEX [FOR ORDER BY] (idx)" (if clause is order
    // by)?
    const bool is_group_by =
        join && join->grouped && order.order == join->group_list.order;
    const bool is_force_index =
        table->force_index ||
        (is_group_by ? table->force_index_group : table->force_index_order);

    // Find an ordering index alternative over the chosen plan iff
    // prefer_ordering_index switch is on. This switch is overridden only when
    // force index for order/group is specified.
    if (thd->optimizer_switch_flag(OPTIMIZER_SWITCH_PREFER_ORDERING_INDEX) ||
        is_force_index)
      test_if_cheaper_ordering(tab, &order, table, usable_keys, ref_key_hint,
                               select_limit, &best_key, &best_key_direction,
                               &select_limit, &best_key_parts,
                               &saved_best_key_parts);

    // Try backward scan for previously found key
    if (best_key < 0 && order_direction < 0) goto check_reverse_order;

    if (best_key < 0) {
      // No usable key has been found
      can_skip_sorting = false;
      goto fix_ICP;
    }
    /*
      If found index will use backward index scan, ref_key uses backward
      range/ref, pick the latter as it has better selectivity.
    */
    if (order_direction < 0 && order_direction == best_key_direction) {
      best_key = -1;  // reset found best key
      goto check_reverse_order;
    }

    /*
      filesort() and join cache are usually faster than reading in
      index order and not using join cache. Don't use index scan
      unless:
       - the user specified FORCE INDEX [FOR {GROUP|ORDER} BY] (have to assume
         the user knows what's best)
       - the chosen index is clustered primary key (table scan is not cheaper)
    */
    if (!is_force_index && (select_limit >= table_records) &&
        (tab->type() == JT_ALL &&
         join->primary_tables > join->const_tables + 1) &&
        ((unsigned)best_key != table->s->primary_key ||
         !table->file->primary_key_is_clustered())) {
      can_skip_sorting = false;
      goto fix_ICP;
    }

    if (table->quick_keys.is_set(best_key) &&
        !tab->quick_order_tested.is_set(best_key) && best_key != ref_key) {
      tab->quick_order_tested.set_bit(best_key);
      Opt_trace_object trace_wrapper_3(trace);
      Opt_trace_object trace_recest(trace, "rows_estimation");
      trace_recest.add_utf8_table(tab->table_ref)
          .add_utf8("index", table->key_info[best_key].name);

      Key_map keys_to_use;            // Force the creation of quick select
      keys_to_use.set_bit(best_key);  // only best_key.
      QUICK_SELECT_I *qck;
      test_quick_select(
          thd, keys_to_use,
          0,  // empty table_map
          join->calc_found_rows ? HA_POS_ERROR : join->unit->select_limit_cnt,
          true,  // force quick range
          order->direction, tab, tab->condition(), &tab->needed_reg, &qck,
          tab->table()->force_index);
      if (order_direction < 0 && tab->quick() != nullptr &&
          tab->quick() != save_quick) {
        /*
          We came here in case when 3 indexes are available for quick
          select:
            1 - found by join order optimizer (greedy search) and saved in
                save_quick
            2 - constructed far above, as better suited for order by, but it was
                found that it requires backward scan.
            3 - constructed right above
          In this case we drop quick #2 as #3 is expected to be better.
        */
        delete tab->quick();
        tab->set_quick(nullptr);
      }
      /*
        If tab->quick() pointed to another quick than save_quick, we would
        lose access to it and leak memory.
      */
      DBUG_ASSERT(tab->quick() == save_quick || tab->quick() == nullptr);
      tab->set_quick(qck);
    }
    order_direction = best_key_direction;
    /*
      saved_best_key_parts is actual number of used keyparts found by the
      test_if_order_by_key function. It could differ from keyinfo->key_parts,
      thus we have to restore it in case of desc order as it affects
      QUICK_SELECT_DESC behaviour.
    */
    used_key_parts =
        (order_direction == -1) ? saved_best_key_parts : best_key_parts;
    changed_key = best_key;
    // We will use index scan or range scan:
    set_up_ref_access_to_key = false;
  }

check_reverse_order:
  DBUG_ASSERT(order_direction != 0);

  if (order_direction == -1)  // If ORDER BY ... DESC
  {
    if (tab->quick()) {
      /*
        Don't reverse the sort order, if it's already done.
        (In some cases test_if_order_by_key() can be called multiple times
      */
      if (tab->quick()->reverse_sorted()) {
        can_skip_sorting = true;
        goto fix_ICP;
      }
      // test_if_cheaper_ordering() might disable range scan on current index
      if (tab->table()->quick_keys.is_set(tab->quick()->index) &&
          tab->quick()->reverse_sort_possible())
        can_skip_sorting = true;
      else {
        can_skip_sorting = false;
        goto fix_ICP;
      }
    } else {
      // Other index access (ref or scan) poses no problem
      can_skip_sorting = true;
    }
  } else {
    // ORDER BY ASC poses no problem
    can_skip_sorting = true;
  }

  DBUG_ASSERT(can_skip_sorting);

  /*
    Update query plan with access pattern for doing
    ordered access according to what we have decided
    above.
  */
  if (!no_changes)  // We are allowed to update QEP
  {
    if (set_up_ref_access_to_key) {
      /*
        We'll use ref access method on key changed_key. In general case
        the index search tuple for changed_ref_key will be different (e.g.
        when one index is defined as (part1, part2, ...) and another as
        (part1, part2(N), ...) and the WHERE clause contains
        "part1 = const1 AND part2=const2".
        So we build tab->ref() from scratch here.
      */
      Key_use *keyuse = tab->keyuse();
      while (keyuse->key != (uint)changed_key &&
             keyuse->table_ref == tab->table_ref)
        keyuse++;

      if (create_ref_for_key(join, tab, keyuse, tab->prefix_tables())) {
        can_skip_sorting = false;
        goto fix_ICP;
      }

      DBUG_ASSERT(tab->type() != JT_REF_OR_NULL && tab->type() != JT_FT);

      // Changing the key makes filter_effect obsolete
      tab->position()->filter_effect = COND_FILTER_STALE;

      /*
        Check if it is possible to shift from ref to range. The index chosen
        for 'ref' has changed since the last time this function was called.
      */
      if (can_switch_from_ref_to_range(thd, tab, order->direction, true)) {
        // Allow the code to fall-through to the next if condition.
        set_up_ref_access_to_key = false;
        best_key = changed_key;
      }
    }
    if (!set_up_ref_access_to_key && best_key >= 0) {
      // Cancel any ref-access previously set up
      tab->ref().key = -1;
      tab->ref().key_parts = 0;

      /*
        If ref_key used index tree reading only ('Using index' in EXPLAIN),
        and best_key doesn't, then revert the decision.
      */
      if (!table->covering_keys.is_set(best_key)) table->set_keyread(false);
      if (!tab->quick() || tab->quick() == save_quick)  // created no QUICK
      {
        // Avoid memory leak:
        DBUG_ASSERT(tab->quick() == save_quick || tab->quick() == nullptr);
        tab->set_quick(nullptr);
        tab->set_index(best_key);
        tab->set_type(JT_INDEX_SCAN);  // Read with index_first(), index_next()
        /*
          There is a bug. When we change here, e.g. from group_min_max to
          index scan: loose index scan expected to read a small number of rows
          (jumping through the index), this small number was in
          position()->rows_fetched; index scan will read much more, so
          rows_fetched should be updated. So should the filtering effect.
          It is visible in main.distinct in trunk:
          explain SELECT distinct a from t3 order by a desc limit 2;
          id	select_type	table	partitions	type
          possible_keys	key	key_len	ref	rows	filtered	Extra 1
          SIMPLE	t3	NULL	index	a	a	5	NULL
          40	25.00	Using index "rows=40" should be ~200 i.e. # of records
          in table. Filter should be 100.00 (no WHERE).
        */
        table->file->ha_index_or_rnd_end();
        tab->position()->filter_effect = COND_FILTER_STALE;
      } else if (tab->type() != JT_ALL) {
        /*
          We're about to use a quick access to the table.
          We need to change the access method so as the quick access
          method is actually used.
        */
        DBUG_ASSERT(tab->quick());
        DBUG_ASSERT(tab->quick()->index == (uint)best_key);
        tab->set_type(calc_join_type(tab->quick()->get_type()));
        tab->use_quick = QS_RANGE;
        if (tab->quick()->is_loose_index_scan())
          join->tmp_table_param.precomputed_group_by = true;
        tab->position()->filter_effect = COND_FILTER_STALE;
      }
    }  // best_key >= 0

    if (order_direction == -1)  // If ORDER BY ... DESC
    {
      if (tab->quick()) {
        /* ORDER BY range_key DESC */
        QUICK_SELECT_I *tmp = tab->quick()->make_reverse(used_key_parts);
        if (!tmp) {
          /* purecov: begin inspected */
          can_skip_sorting = false;  // Reverse sort failed -> filesort
          goto fix_ICP;
          /* purecov: end */
        }
        if (tab->quick() != tmp && tab->quick() != save_quick)
          delete tab->quick();
        tab->set_quick(tmp);
        tab->set_type(calc_join_type(tmp->get_type()));
        tab->position()->filter_effect = COND_FILTER_STALE;
      } else if (tab->type() == JT_REF &&
                 tab->ref().key_parts <= used_key_parts) {
        /*
          SELECT * FROM t1 WHERE a=1 ORDER BY a DESC,b DESC

          Use a traversal function that starts by reading the last row
          with key part (A) and then traverse the index backwards.
        */
        tab->reversed_access = true;

        /*
          The current implementation of the reverse RefIterator does not
          work well in combination with ICP and can lead to increased
          execution time. Setting changed_key to the current key
          (based on that we change the access order for the key) will
          ensure that a pushed index condition will be cancelled.
        */
        changed_key = tab->ref().key;
      } else if (tab->type() == JT_INDEX_SCAN)
        tab->reversed_access = true;
    } else if (tab->quick())
      tab->quick()->need_sorted_output();

  }  // QEP has been modified

fix_ICP:
  /*
    Cleanup:
    We may have both a 'tab->quick()' and 'save_quick' (original)
    at this point. Delete the one that we won't use.
  */
  if (can_skip_sorting && !no_changes) {
    if (tab->type() == JT_INDEX_SCAN &&
        select_limit < table->file->stats.records) {
      DBUG_ASSERT(select_limit > 0);
      tab->position()->rows_fetched = select_limit;
      tab->position()->filter_effect = COND_FILTER_STALE_NO_CONST;
    }

    // Keep current (ordered) tab->quick()
    if (save_quick != tab->quick()) delete save_quick;
  } else {
    // Restore original save_quick
    if (tab->quick() != save_quick) {
      delete tab->quick();
      tab->set_quick(save_quick);
    }
  }

  trace_steps.end();
  Opt_trace_object trace_change_index(trace, "index_order_summary");
  trace_change_index.add_utf8_table(tab->table_ref)
      .add("index_provides_order", can_skip_sorting)
      .add_alnum("order_direction",
                 order_direction == 1
                     ? "asc"
                     : ((order_direction == -1) ? "desc" : "undefined"));

  if (changed_key >= 0) {
    // switching to another index
    // Should be no pushed index conditions at this point
    DBUG_ASSERT(!table->file->pushed_idx_cond);
    if (unlikely(trace->is_started())) {
      trace_change_index.add_utf8("index", table->key_info[changed_key].name);
      trace_change_index.add("plan_changed", !no_changes);
      if (!no_changes)
        trace_change_index.add_alnum("access_type", join_type_str[tab->type()]);
    }
  } else if (unlikely(trace->is_started())) {
    trace_change_index.add_utf8(
        "index", ref_key >= 0 ? table->key_info[ref_key].name : "unknown");
    trace_change_index.add("plan_changed", false);
  }
  *order_idx = best_key < 0 ? ref_key : best_key;
  return can_skip_sorting;
}

/**
  Prune partitions for all tables of a join (query block).

  Requires that tables have been locked.

  @returns false if success, true if error
*/

bool JOIN::prune_table_partitions() {
  DBUG_ASSERT(select_lex->partitioned_table_count);

  for (TABLE_LIST *tbl = select_lex->leaf_tables; tbl; tbl = tbl->next_leaf) {
    /*
      If tbl->embedding!=NULL that means that this table is in the inner
      part of the nested outer join, and we can't do partition pruning
      (TODO: check if this limitation can be lifted.
             This also excludes semi-joins.  Is that intentional?)
      This will try to prune non-static conditions, which can
      be used after the tables are locked.
    */
    if (!tbl->embedding) {
      Item *prune_cond =
          tbl->join_cond_optim() ? tbl->join_cond_optim() : where_cond;
      if (prune_partitions(thd, tbl->table, prune_cond)) return true;
    }
  }

  return false;
}

/**
  A helper function to check whether it's better to use range than ref.

  @details
  Heuristic: Switch from 'ref' to 'range' access if 'range'
  access can utilize more keyparts than 'ref' access. Conditions
  for doing switching:

  1) Range access is possible
  2) 'ref' access and 'range' access uses the same index
  3) Used parts of key shouldn't have nullable parts & ref_or_null isn't used.
  4) 'ref' access depends on a constant, not a value read from a
     table earlier in the join sequence.

     Rationale: if 'ref' depends on a value from another table,
     the join condition is not used to limit the rows read by
     'range' access (that would require dynamic range - 'Range
     checked for each record'). In other words, if 'ref' depends
     on a value from another table, we have a query with
     conditions of the form

      this_table.idx_col1 = other_table.col AND   <<- used by 'ref'
      this_table.idx_col1 OP @<const@> AND        <<- used by 'range'
      this_table.idx_col2 OP @<const@> AND ...    <<- used by 'range'

     and an index on (idx_col1,idx_col2,...). But the fact that
     'range' access uses more keyparts does not mean that it is
     more selective than 'ref' access because these access types
     utilize different parts of the query condition. We
     therefore trust the cost based choice made by
     best_access_path() instead of forcing a heuristic choice
     here.
  5) 'range' access uses more keyparts than 'ref' access
  6) ORDER BY might make range better than table scan:
     Check possibility of range scan even if it was previously deemed unviable
     (for example when table scan was estimated to be cheaper). If yes,
     range-access should be chosen only for larger key length.

  @param thd           To re-run range optimizer.
  @param tab           JOIN_TAB to check
  @param ordering      Used as a parameter to call test_quick_select.
  @param recheck_range Check possibility of range scan even if it is currently
                       unviable.

  @return Pointer  Range is better than ref. The pointer points to a string
                   describing why (the string is intended to be used in the
                   optimizer trace)
  @return nullptr  Ref is better or switch isn't possible

  @todo: This decision should rather be made in best_access_path()
*/

static const char *can_switch_from_ref_to_range(THD *thd, JOIN_TAB *tab,
                                                enum_order ordering,
                                                bool recheck_range) {
  if ((tab->quick() &&                                       // 1)
       tab->position()->key->key == tab->quick()->index) ||  // 2)
      recheck_range) {
    uint keyparts = 0, length = 0;
    table_map dep_map = 0;
    bool maybe_null = false;

    calc_length_and_keyparts(tab->position()->key, tab,
                             tab->position()->key->key, tab->prefix_tables(),
                             nullptr, &length, &keyparts, &dep_map,
                             &maybe_null);
    if (!maybe_null &&  // 3)
        !dep_map)       // 4)
    {
      if (recheck_range)  // 6)
      {
        Key_map new_ref_key_map;
        new_ref_key_map.set_bit(tab->ref().key);

        Opt_trace_context *const trace = &thd->opt_trace;
        Opt_trace_object trace_wrapper(trace);
        Opt_trace_object can_switch(
            trace, "check_if_range_uses_more_keyparts_than_ref");
        Opt_trace_object trace_cond(
            trace, "rerunning_range_optimizer_for_single_index");

        QUICK_SELECT_I *qck;
        if (test_quick_select(
                thd, new_ref_key_map, 0,  // empty table_map
                tab->join()->row_limit, false, ordering, tab,
                tab->join_cond() ? tab->join_cond() : tab->join()->where_cond,
                &tab->needed_reg, &qck, recheck_range) > 0) {
          if (length < qck->max_used_key_length) {
            delete tab->quick();
            tab->set_quick(qck);
            return "uses_more_keyparts";
          } else {
            Opt_trace_object(trace, "access_type_unchanged")
                .add("ref_key_length", length)
                .add("range_key_length", qck->max_used_key_length);
            delete qck;
          }
        }
      } else if (length == tab->quick()->max_used_key_length &&
                 tab->quick()->get_type() == QUICK_SELECT_I::QS_TYPE_RANGE) {
        /*
          If there was a quick select that
           - was scanning the same range
           - used MRR
           - MRR implementation indicated that it's preferred to use MRR over
             ref(const).
          then switch to range access as it will read rows using the MRR
          interface
        */
        auto *qr = static_cast<QUICK_RANGE_SELECT *>(tab->quick());
        if ((qr->mrr_flags & HA_MRR_USE_DEFAULT_IMPL) == 0 &&
            (qr->mrr_flags & HA_MRR_CONVERT_REF_TO_RANGE) != 0) {
          return "uses_mrr";
        }
      } else {
        return length < tab->quick()->max_used_key_length ? "uses_more_keyparts"
                                                          : nullptr;  // 5)
      }
    }
  }
  return nullptr;
}

/**
 An utility function - apply heuristics and optimize access methods to tables.
 Currently this function can change REF to RANGE and ALL to INDEX scan if
 latter is considered to be better (not cost-based) than the former.
 @note Side effect - this function could set 'Impossible WHERE' zero
 result.
*/

void JOIN::adjust_access_methods() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *const tab = best_ref[i];
    TABLE_LIST *const tl = tab->table_ref;

    if (tab->type() == JT_ALL) {
      /*
       It's possible to speedup query by switching from full table scan to
       the scan of covering index, due to less data being read.
       Prerequisites for this are:
       1) Keyread (i.e index only scan) is allowed (table isn't updated/deleted
         from)
       2) Covering indexes are available
       3) This isn't a derived table/materialized view
      */
      if (!tab->table()->no_keyread &&                    //  1
          !tab->table()->covering_keys.is_clear_all() &&  //  2
          !tl->uses_materialization())                    //  3
      {
        /*
        It has turned out that the change commented out below, while speeding
        things up for disk-bound loads, slows them down for cases when the data
        is in disk cache (see BUG#35850):
        //  See bug #26447: "Using the clustered index for a table scan
        //  is always faster than using a secondary index".
        if (table->s->primary_key != MAX_KEY &&
            table->file->primary_key_is_clustered())
          tab->index= table->s->primary_key;
        else
          tab->index=find_shortest_key(table, & table->covering_keys);
        */
        if (tab->position()->sj_strategy != SJ_OPT_LOOSE_SCAN)
          tab->set_index(
              find_shortest_key(tab->table(), &tab->table()->covering_keys));
        tab->set_type(JT_INDEX_SCAN);  // Read with index_first / index_next
        // From table scan to index scan, thus filter effect needs no recalc.
      }
    } else if (tab->type() == JT_REF) {
      const char *switch_reason =
          can_switch_from_ref_to_range(thd, tab, ORDER_NOT_RELEVANT, false);
      if (switch_reason != nullptr) {
        tab->set_type(JT_RANGE);

        Opt_trace_context *const trace = &thd->opt_trace;
        Opt_trace_object wrapper(trace);
        Opt_trace_object(trace, "access_type_changed")
            .add_utf8_table(tl)
            .add_utf8("index",
                      tab->table()->key_info[tab->position()->key->key].name)
            .add_alnum("old_type", "ref")
            .add_alnum("new_type", join_type_str[tab->type()])
            .add_alnum("cause", switch_reason);

        tab->use_quick = QS_RANGE;
        tab->position()->filter_effect = COND_FILTER_STALE;
      } else {
        // Cleanup quick, REF/REF_OR_NULL/EQ_REF, will be clarified later
        delete tab->quick();
        tab->set_quick(nullptr);
      }
    }
    // Ensure AM consistency
    DBUG_ASSERT(
        !(tab->quick() && (tab->type() == JT_REF || tab->type() == JT_ALL)));
    DBUG_ASSERT((tab->type() != JT_RANGE && tab->type() != JT_INDEX_MERGE) ||
                tab->quick());
    if (!tab->const_keys.is_clear_all() &&
        tab->table()->reginfo.impossible_range &&
        ((i == const_tables && tab->type() == JT_REF) ||
         ((tab->type() == JT_ALL || tab->type() == JT_RANGE ||
           tab->type() == JT_INDEX_MERGE || tab->type() == JT_INDEX_SCAN) &&
          tab->use_quick != QS_RANGE)) &&
        !tab->table_ref->is_inner_table_of_outer_join())
      zero_result_cause = "Impossible WHERE noticed after reading const tables";
  }
}

static JOIN_TAB *alloc_jtab_array(THD *thd, uint table_count) {
  JOIN_TAB *t = new (thd->mem_root) JOIN_TAB[table_count];
  if (!t) return nullptr; /* purecov: inspected */

  QEP_shared *qs = new (thd->mem_root) QEP_shared[table_count];
  if (!qs) return nullptr; /* purecov: inspected */

  for (uint i = 0; i < table_count; ++i) t[i].set_qs(qs++);

  return t;
}

/**
  Set up JOIN_TAB structs according to the picked join order in best_positions.
  This allocates execution structures so may be called only after we have the
  very final plan. It must be called after
  Optimize_table_order::fix_semijoin_strategies().

  @return False if success, True if error

  @details
    - create join->join_tab array and copy from existing JOIN_TABs in join order
    - create helper structs for materialized semi-join handling
    - finalize semi-join strategy choices
    - Number of intermediate tables "tmp_tables" is calculated.
    - "tables" and "primary_tables" are recalculated.
    - for full and index scans info of estimated # of records is updated.
    - in a helper function:
      - all heuristics are applied and the final access method type is picked
        for each join_tab (only test_if_skip_sortorder() could override it)
      - AM consistency is ensured (e.g only range and index merge are allowed
        to have quick select set).
      - if "Impossible WHERE" is detected - appropriate zero_result_cause is
        set.

   Notice that intermediate tables will not have a POSITION reference; and they
   will not have a TABLE reference before the final stages of code generation.

   @todo the block which sets tab->type should move to adjust_access_methods
   for unification.
*/

bool JOIN::get_best_combination() {
  DBUG_TRACE;

  // At this point "tables" and "primary"tables" represent the same:
  DBUG_ASSERT(tables == primary_tables);

  /*
    Allocate additional space for tmp tables.
    Number of plan nodes:
      # of regular input tables (including semi-joined ones) +
      # of semi-join nests for materialization +
      1? + // For GROUP BY (or implicit grouping when we have windowing)
      1? + // For DISTINCT
      1? + // For aggregation functions aggregated in outer query
           // when used with distinct
      1? + // For ORDER BY
      1?   // buffer result

    Up to 2 tmp tables + N window output tmp are allocated (NOTE: windows also
    have frame buffer tmp tables, but those are not relevant here).
  */
  uint num_tmp_tables =
      (group_list || (implicit_grouping && m_windows.elements) > 0 ? 1 : 0) +
      (select_distinct ? (tmp_table_param.outer_sum_func_count ? 2 : 1) : 0) +
      (order ? 1 : 0) +
      (select_lex->active_options() & (SELECT_BIG_RESULT | OPTION_BUFFER_RESULT)
           ? 1
           : 0) +
      m_windows.elements + 1; /* the presence of windows may increase need for
                                 grouping tmp tables, cf. de-optimization
                                 in make_tmp_tables_info
                               */
  if (num_tmp_tables > (2 + m_windows.elements))
    num_tmp_tables = 2 + m_windows.elements;

  /*
    Rearrange queries with materialized semi-join nests so that the semi-join
    nest is replaced with a reference to a materialized temporary table and all
    materialized subquery tables are placed after the intermediate tables.
    After the following loop, "inner_target" is the position of the first
    subquery table (if any). "outer_target" is the position of first outer
    table, and will later be used to track the position of any materialized
    temporary tables.
  */
  const bool has_semijoin = !select_lex->sj_nests.empty();
  uint outer_target = 0;
  uint inner_target = primary_tables + num_tmp_tables;
  uint sjm_nests = 0;

  if (has_semijoin) {
    for (uint tableno = 0; tableno < primary_tables;) {
      if (sj_is_materialize_strategy(best_positions[tableno].sj_strategy)) {
        sjm_nests++;
        inner_target -= (best_positions[tableno].n_sj_tables - 1);
        tableno += best_positions[tableno].n_sj_tables;
      } else
        tableno++;
    }
  }

  JOIN_TAB *tmp_join_tabs = nullptr;
  if (sjm_nests + num_tmp_tables) {
    // join_tab array only has "primary_tables" tables. We need those more:
    if (!(tmp_join_tabs = alloc_jtab_array(thd, sjm_nests + num_tmp_tables)))
      return true; /* purecov: inspected */
  }

  // To check that we fill the array correctly: fill it with zeros first
  memset(best_ref, 0,
         sizeof(JOIN_TAB *) * (primary_tables + sjm_nests + num_tmp_tables));

  int sjm_index = tables;  // Number assigned to materialized temporary table
  int remaining_sjm_inner = 0;
  bool err = false;
  for (uint tableno = 0; tableno < tables; tableno++) {
    POSITION *const pos = best_positions + tableno;
    if (has_semijoin && sj_is_materialize_strategy(pos->sj_strategy)) {
      DBUG_ASSERT(outer_target < inner_target);

      TABLE_LIST *const sj_nest = pos->table->emb_sj_nest;

      // Handle this many inner tables of materialized semi-join
      remaining_sjm_inner = pos->n_sj_tables;

      /*
        If we fail in some allocation below, we cannot bail out immediately;
        that would put us in a difficult situation to clean up; imagine we
        have planned this layout:
          outer1 - sj_mat_tmp1 - outer2 - sj_mat_tmp2 - outer3
        We have successfully filled a JOIN_TAB for sj_mat_tmp1, and are
        failing to fill a JOIN_TAB for sj_mat_tmp2 (OOM). So we want to quit
        this function, which will lead to cleanup functions.
        But sj_mat_tmp1 is in this->best_ref only, outer3 is in this->join_tab
        only: what is the array to traverse for cleaning up? What is the
        number of tables to loop over?
        So: if we fail in the present loop, we record the error but continue
        filling best_ref; when it's fully filled, bail out, because then
        best_ref can be used as reliable array for cleaning up.
      */
      JOIN_TAB *const tab = tmp_join_tabs++;
      best_ref[outer_target] = tab;
      tab->set_join(this);
      tab->set_idx(outer_target);

      /*
        Up to this point there cannot be a failure. JOIN_TAB has been filled
        enough to be clean-able.
      */

      Semijoin_mat_exec *const sjm_exec = new (thd->mem_root) Semijoin_mat_exec(
          sj_nest, (pos->sj_strategy == SJ_OPT_MATERIALIZE_SCAN),
          remaining_sjm_inner, outer_target, inner_target);

      tab->set_sj_mat_exec(sjm_exec);

      if (!sjm_exec || setup_semijoin_materialized_table(
                           tab, sjm_index, pos, best_positions + sjm_index))
        err = true; /* purecov: inspected */

      outer_target++;
      sjm_index++;
    }
    /*
      Locate join_tab target for the table we are considering.
      (remaining_sjm_inner becomes negative for non-SJM tables, this can be
       safely ignored).
    */
    const uint target =
        (remaining_sjm_inner--) > 0 ? inner_target++ : outer_target++;
    JOIN_TAB *const tab = pos->table;

    best_ref[target] = tab;
    tab->set_idx(target);
    tab->set_position(pos);
    TABLE *const table = tab->table();
    if (tab->type() != JT_CONST && tab->type() != JT_SYSTEM) {
      if (pos->sj_strategy == SJ_OPT_LOOSE_SCAN && tab->quick() &&
          tab->quick()->index != pos->loosescan_key) {
        /*
          We must use the duplicate-eliminating index, so this QUICK is not
          an option.
        */
        delete tab->quick();
        tab->set_quick(nullptr);
      }
      if (!pos->key) {
        if (tab->quick())
          tab->set_type(calc_join_type(tab->quick()->get_type()));
        else
          tab->set_type(JT_ALL);
      } else
        // REF or RANGE, clarify later when prefix tables are set for JOIN_TABs
        tab->set_type(JT_REF);
    }
    DBUG_ASSERT(tab->type() != JT_UNKNOWN);

    DBUG_ASSERT(table->reginfo.join_tab == tab);
    if (!tab->join_cond())
      table->reginfo.not_exists_optimize = false;  // Only with LEFT JOIN
    map2table[tab->table_ref->tableno()] = tab;
  }

  // Count the materialized semi-join tables as regular input tables
  tables += sjm_nests + num_tmp_tables;
  // Set the number of non-materialized tables:
  primary_tables = outer_target;

  /*
    Between the last outer table or sj-mat tmp table, and the first sj-mat
    inner table, there may be 2 slots for sort/group/etc tmp tables:
  */
  for (uint i = 0; i < num_tmp_tables; ++i) {
    const uint idx = outer_target + i;
    tmp_join_tabs->set_join(this);
    tmp_join_tabs->set_idx(idx);
    DBUG_ASSERT(best_ref[idx] == nullptr);  // verify that not overwriting
    best_ref[idx] = tmp_join_tabs++;
    /*
      note that set_table() cannot be called yet. We may not even use this
      JOIN_TAB in the end, it's dummy at the moment. Which can be tested with
      "position()!=NULL".
    */
  }

  // make array unreachable: should walk JOIN_TABs by best_ref now
  join_tab = nullptr;

  if (err) return true; /* purecov: inspected */

  if (has_semijoin) {
    set_semijoin_info();

    // Update equalities and keyuses after having added SJ materialization
    if (update_equalities_for_sjm()) return true;
  }
  if (!plan_is_const()) {
    // Assign map of "available" tables to all tables belonging to query block
    set_prefix_tables();
    adjust_access_methods();
  }
  // Calculate outer join info
  if (select_lex->outer_join) make_outerjoin_info();

  // sjm is no longer needed, trash it. To reuse it, reset its members!
  for (TABLE_LIST *sj_nest : select_lex->sj_nests) {
    TRASH(&sj_nest->nested_join->sjm, sizeof(sj_nest->nested_join->sjm));
  }

  return false;
}

/**
   Updates JOIN::deps_of_remaining_lateral_derived_tables

   @param plan_tables  map of all tables that the planner is processing
                       (tables already in plan and tables to be added to plan)
   @param idx          index of the table which the planner is currently
                       considering
*/
void JOIN::recalculate_deps_of_remaining_lateral_derived_tables(
    table_map plan_tables, uint idx) {
  DBUG_ASSERT(has_lateral);
  deps_of_remaining_lateral_derived_tables = 0;
  auto last = best_ref + tables;
  for (auto **pos = best_ref + idx; pos < last; pos++) {
    if ((*pos)->table_ref && (*pos)->table_ref->is_derived() &&
        ((*pos)->table_ref->map() & plan_tables))
      deps_of_remaining_lateral_derived_tables |=
          (*pos)->table_ref->derived_unit()->m_lateral_deps;
  }
}

/*
  Revise usage of join buffer for the specified table and the whole nest

  SYNOPSIS
    revise_cache_usage()
      tab    join table for which join buffer usage is to be revised

  DESCRIPTION
    The function revise the decision to use a join buffer for the table 'tab'.
    If this table happened to be among the inner tables of a nested outer join/
    semi-join the functions denies usage of join buffers for all of them

  RETURN
    none
*/

static void revise_cache_usage(JOIN_TAB *join_tab) {
  plan_idx first_inner = join_tab->first_inner();
  JOIN *const join = join_tab->join();
  if (first_inner != NO_PLAN_IDX) {
    plan_idx end_tab = join_tab->idx();
    for (first_inner = join_tab->first_inner(); first_inner != NO_PLAN_IDX;
         first_inner = join->best_ref[first_inner]->first_upper()) {
      for (plan_idx i = end_tab - 1; i >= first_inner; --i)
        join->best_ref[i]->set_use_join_cache(JOIN_CACHE::ALG_NONE);
      end_tab = first_inner;
    }
  } else if (join_tab->get_sj_strategy() == SJ_OPT_FIRST_MATCH) {
    plan_idx first_sj_inner = join_tab->first_sj_inner();
    for (plan_idx i = join_tab->idx() - 1; i >= first_sj_inner; --i) {
      JOIN_TAB *tab = join->best_ref[i];
      if (tab->first_sj_inner() == first_sj_inner)
        tab->set_use_join_cache(JOIN_CACHE::ALG_NONE);
    }
  } else
    join_tab->set_use_join_cache(JOIN_CACHE::ALG_NONE);
  DBUG_ASSERT(join->qep_tab == nullptr);
}

/**
  Set up join buffering for a specified table, if possible.

  @param tab             joined table to check join buffer usage for
  @param join            join for which the check is performed
  @param no_jbuf_after   don't use join buffering after table with this number

  @return false if successful, true if error.
          Currently, allocation errors for join cache objects are ignored,
          and regular execution is chosen silently.

  @details
    The function finds out whether the table 'tab' can be joined using a join
    buffer. This check is performed after the best execution plan for 'join'
    has been chosen. If the function decides that a join buffer can be employed
    then it selects the most appropriate join cache type, which later will
    be instantiated by init_join_cache().
    If it has already been decided to not use join buffering for this table,
    no action is taken.

    Often it is already decided that join buffering will be used earlier in
    the optimization process, and this will also ensure that the most correct
    cost for the operation is calculated, and hence the probability of
    choosing an optimal join plan is higher. However, some join buffering
    decisions cannot currently be taken before this stage, hence we need this
    function to decide the most accurate join buffering strategy.

    @todo Long-term it is the goal that join buffering strategy is decided
    when the plan is selected.

    The result of the check and the type of the join buffer to be used
    depend on:
      - the access method to access rows of the joined table
      - whether the join table is an inner table of an outer join or semi-join
      - the optimizer_switch settings for join buffering
      - the join 'options'.
    In any case join buffer is not used if the number of the joined table is
    greater than 'no_jbuf_after'.

    If block_nested_loop is turned on, and if all other criteria for using
    join buffering is fulfilled (see below), then join buffer is used
    for any join operation (inner join, outer join, semi-join) with 'JT_ALL'
    access method.  In that case, a JOIN_CACHE_BNL type is always employed.

    If an index is used to access rows of the joined table and
  batched_key_access is on, then a JOIN_CACHE_BKA type is employed.

    If the function decides that a join buffer can be used to join the table
    'tab' then it sets @c tab->use_join_cache to reflect the chosen algorithm.

  @note
    For a nested outer join/semi-join, currently, we either use join buffers for
    all inner tables or for none of them.

  @todo
    Support BKA inside SJ-Materialization nests. When doing this, we'll need
    to only store sj-inner tables in the join buffer.
    @verbatim
        JOIN_TAB *first_tab= join->join_tab+join->const_tables;
        uint n_tables= i-join->const_tables;
        / *
          We normally put all preceding tables into the join buffer, except
          for the constant tables.
          If we're inside a semi-join materialization nest, e.g.

             outer_tbl1  outer_tbl2  ( inner_tbl1, inner_tbl2 ) ...
                                                       ^-- we're here

          then we need to put into the join buffer only the tables from
          within the nest.
        * /
        if (i >= first_sjm_table && i < last_sjm_table)
        {
          n_tables= i - first_sjm_table; // will be >0 if we got here
          first_tab= join->join_tab + first_sjm_table;
        }
    @endverbatim
*/

static bool setup_join_buffering(JOIN_TAB *tab, JOIN *join,
                                 uint no_jbuf_after) {
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  Cost_estimate cost;
  ha_rows rows;
  uint bufsz = 4096;
  uint join_cache_flags = HA_MRR_NO_NULL_ENDPOINTS;
  const bool bnl_on = hint_table_state(join->thd, tab->table_ref, BNL_HINT_ENUM,
                                       OPTIMIZER_SWITCH_BNL);
  const bool bka_on = hint_table_state(join->thd, tab->table_ref, BKA_HINT_ENUM,
                                       OPTIMIZER_SWITCH_BKA);

  const uint tableno = tab->idx();
  const uint tab_sj_strategy = tab->get_sj_strategy();

  // Set preliminary join cache setting based on decision from greedy search
  if (!join->select_count)
    tab->set_use_join_cache(tab->position()->use_join_buffer
                                ? JOIN_CACHE::ALG_BNL
                                : JOIN_CACHE::ALG_NONE);

  if (tableno == join->const_tables) {
    DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
    return false;
  }

  if (!(bnl_on || bka_on)) goto no_join_cache;

  /*
    psergey-todo: why the below when execution code seems to handle the
    "range checked for each record" case?
  */
  if (tab->use_quick == QS_DYNAMIC_RANGE) goto no_join_cache;

  /* No join buffering if prevented by no_jbuf_after */
  if (tableno > no_jbuf_after) goto no_join_cache;

  /*
    An inner table of an outer join nest must not use join buffering if
    the first inner table of that outer join nest does not use join buffering.
    This condition is not handled by earlier optimizer stages.
  */
  if (tab->first_inner() != NO_PLAN_IDX && tab->first_inner() != tab->idx() &&
      !join->best_ref[tab->first_inner()]->use_join_cache())
    goto no_join_cache;
  /*
    The first inner table of an outer join nest must not use join buffering
    if the tables in the embedding outer join nest do not use join buffering.
    This condition is not handled by earlier optimizer stages.
  */
  if (tab->first_upper() != NO_PLAN_IDX &&
      !join->best_ref[tab->first_upper()]->use_join_cache())
    goto no_join_cache;

  if (tab->table()->pos_in_table_list->is_table_function() && tab->dependent)
    goto no_join_cache;

  switch (tab_sj_strategy) {
    case SJ_OPT_FIRST_MATCH:
      /*
        Use join cache with FirstMatch semi-join strategy only when semi-join
        contains only one table.
      */
      if (!tab->is_single_inner_of_semi_join()) {
        DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
        goto no_join_cache;
      }
      break;

    case SJ_OPT_LOOSE_SCAN:
      /* No join buffering if this semijoin nest is handled by loosescan */
      DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
      goto no_join_cache;

    case SJ_OPT_MATERIALIZE_LOOKUP:
    case SJ_OPT_MATERIALIZE_SCAN:
      /*
        The Materialize strategies reuse the join_tab belonging to the
        first table that was materialized. Neither table can use join buffering:
        - The first table in a join never uses join buffering.
        - The join_tab used for looking up a row in the materialized table, or
          scanning the rows of a materialized table, cannot use join buffering.
        We allow join buffering for the remaining tables of the materialized
        semi-join nest.
      */
      if (tab->first_sj_inner() == tab->idx()) {
        DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
        goto no_join_cache;
      }
      break;

    case SJ_OPT_DUPS_WEEDOUT:
    // This strategy allows the same join buffering as a regular join would.
    case SJ_OPT_NONE:
      break;
  }

  /*
    The following code prevents use of join buffering when there is an
    outer join operation and first match semi-join strategy is used, because:

    Outer join needs a "match flag" to track that a row should be
    NULL-complemented, such flag being attached to first inner table's cache
    (tracks whether the cached row from outer table got a match, in which case
    no NULL-complemented row is needed).

    FirstMatch also needs a "match flag", such flag is attached to sj inner
    table's cache (tracks whether the cached row from outer table already got
    a first match in the sj-inner table, in which case we don't need to join
    this cached row again)
     - but a row in a cache has only one "match flag"
     - so if "sj inner table"=="first inner", there is a problem.
  */
  if (tab_sj_strategy == SJ_OPT_FIRST_MATCH &&
      tab->is_inner_table_of_outer_join())
    goto no_join_cache;

  if (join->deps_of_remaining_lateral_derived_tables &
      (tab->prefix_tables() & ~tab->added_tables())) {
    /*
      Even though the planner said "no jbuf please", the switch below may
      force it.
      If first-dependency-of-lateral-table < table-we-plan-for <=
      lateral-table, disable join buffering.
      Reason for this rule:
      consider a plan t1-t2-dt where dt is LATERAL and depends only on t1, and
      imagine t2 could do join buffering: then we buffer many rows of t1, then
      read one row of t2, fetch row#1 of t1 from cache, then materialize "dt"
      (as it depends on t1) and send row to client; then fetch row#2 of t1
      from cache, rematerialize "dt": it's very inefficient. So we forbid join
      buffering on t2; this way, the signal "row of t1 changed" is emitted at
      the level of t1's operator, i.e. much less often, as one row of t1 may
      serve N rows of t2 before changing.
      On the other hand, t1 can do join buffering.
      A nice side-effect is to disable join buffering for "dt" itself. If
      "dt" would do join buffering: "dt" buffers many rows from t1/t2, then in a
      second phase we read one row from "dt" and join it with the many rows
      from t1/t2; but we cannot read a row from "dt" without first choosing a
      row of t1/t2 as "dt" depends on t1.
      See similar code in best_access_path().
    */
    goto no_join_cache;
  }

  switch (tab->type()) {
    case JT_ALL:
    case JT_INDEX_SCAN:
    case JT_RANGE:
    case JT_INDEX_MERGE:
      if (!bnl_on) {
        DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
        goto no_join_cache;
      }

      if (!join->select_count) tab->set_use_join_cache(JOIN_CACHE::ALG_BNL);
      return false;
    case JT_SYSTEM:
    case JT_CONST:
    case JT_REF:
    case JT_EQ_REF:
      if (!bka_on) {
        DBUG_ASSERT(tab->use_join_cache() == JOIN_CACHE::ALG_NONE);
        goto no_join_cache;
      }

      /*
        Disable BKA for materializable derived tables/views as they aren't
        instantiated yet.
      */
      if (tab->table_ref->uses_materialization()) goto no_join_cache;

      /*
        Can't use BKA for subquery if dealing with a subquery that can
        turn a ref access into a "full scan on NULL key" table scan.

        @see Item_in_optimizer::val_int()
        @see subselect_iterator_engine::exec()
        @see TABLE_REF::cond_guards
        @see push_index_cond()

        @todo: This choice to not use BKA should be done before making
        cost estimates, e.g. in set_join_buffer_properties(). That
        happens before cond guards are set up, so instead of doing the
        check below, BKA should be disabled if
         - We are in an IN subquery, and
         - The IN predicate is not a top_level_item, and
         - The left_expr of the IN predicate may contain NULL values
           (left_expr->maybe_null)
      */
      if (tab->has_guarded_conds()) goto no_join_cache;

      if (tab->table()->covering_keys.is_set(tab->ref().key))
        join_cache_flags |= HA_MRR_INDEX_ONLY;

      if (tab->table()->key_info[tab->ref().key].actual_key_parts ==
          tab->ref().key_parts)
        join_cache_flags |= HA_MRR_FULL_EXTENDED_KEYS;

      rows = tab->table()->file->multi_range_read_info(
          tab->ref().key, 10, 20, &bufsz, &join_cache_flags, &cost);
      /*
        Cannot use BKA if
        1. MRR scan cannot be performed, or
        2. MRR default implementation is used, or
        3. HA_MRR_NO_ASSOCIATION flag is set
      */
      if ((rows == HA_POS_ERROR) ||                        // 1
          (join_cache_flags & HA_MRR_USE_DEFAULT_IMPL) ||  // 2
          (join_cache_flags & HA_MRR_NO_ASSOCIATION))      // 3
        goto no_join_cache;

      tab->set_use_join_cache(JOIN_CACHE::ALG_BKA);

      tab->join_cache_flags = join_cache_flags;
      return false;
    default:;
  }

no_join_cache:
  revise_cache_usage(tab);
  tab->set_use_join_cache(JOIN_CACHE::ALG_NONE);
  return false;
}

/*****************************************************************************
  Make some simple condition optimization:
  If there is a test 'field = const' change all refs to 'field' to 'const'
  Remove all dummy tests 'item = item', 'const op const'.
  Remove all 'item is NULL', when item can never be null!
  Return in cond_value false if condition is impossible (1 = 2)
*****************************************************************************/

class COND_CMP : public ilink<COND_CMP> {
 public:
  static void *operator new(size_t size) { return (*THR_MALLOC)->Alloc(size); }
  static void operator delete(void *ptr MY_ATTRIBUTE((unused)),
                              size_t size MY_ATTRIBUTE((unused))) {
    TRASH(ptr, size);
  }

  Item *and_level;
  Item_func *cmp_func;
  COND_CMP(Item *a, Item_func *b) : and_level(a), cmp_func(b) {}
};

Item_equal *find_item_equal(COND_EQUAL *cond_equal,
                            const Item_field *item_field, bool *inherited_fl) {
  Item_equal *item = nullptr;
  bool in_upper_level = false;
  while (cond_equal) {
    List_iterator_fast<Item_equal> li(cond_equal->current_level);
    while ((item = li++)) {
      if (item->contains(item_field->field)) goto finish;
    }
    in_upper_level = true;
    cond_equal = cond_equal->upper_levels;
  }
  in_upper_level = false;
finish:
  *inherited_fl = in_upper_level;
  return item;
}

/**
  Get the best field substitution for a given field.

  If the field is member of a multiple equality, look up that equality
  and return the most appropriate field. Usually this is the equivalenced
  field belonging to the outer-most table in the join order, but
  @see Item_field::get_subst_item() for details.
  Otherwise, return the same field.

  @param item_field The field that we are seeking a substitution for.
  @param cond_equal multiple equalities to search in

  @return The substituted field.
*/

Item_field *get_best_field(Item_field *item_field, COND_EQUAL *cond_equal) {
  bool dummy;
  Item_equal *item_eq = find_item_equal(cond_equal, item_field, &dummy);
  if (!item_eq) return item_field;

  return item_eq->get_subst_item(item_field);
}

/**
  Check whether an equality can be used to build multiple equalities.

    This function first checks whether the equality (left_item=right_item)
    is a simple equality i.e. one that equates a field with another field
    or a constant (field=field_item or field=const_item).
    If this is the case the function looks for a multiple equality
    in the lists referenced directly or indirectly by cond_equal inferring
    the given simple equality. If it doesn't find any, it builds a multiple
    equality that covers the predicate, i.e. the predicate can be inferred
    from this multiple equality.
    The built multiple equality could be obtained in such a way:
    create a binary  multiple equality equivalent to the predicate, then
    merge it, if possible, with one of old multiple equalities.
    This guarantees that the set of multiple equalities covering equality
    predicates will be minimal.

  EXAMPLE:
    For the where condition
    @code
      WHERE a=b AND b=c AND
            (b=2 OR f=e)
    @endcode
    the check_equality will be called for the following equality
    predicates a=b, b=c, b=2 and f=e.
    - For a=b it will be called with *cond_equal=(0,[]) and will transform
      *cond_equal into (0,[Item_equal(a,b)]).
    - For b=c it will be called with *cond_equal=(0,[Item_equal(a,b)])
      and will transform *cond_equal into CE=(0,[Item_equal(a,b,c)]).
    - For b=2 it will be called with *cond_equal=(ptr(CE),[])
      and will transform *cond_equal into (ptr(CE),[Item_equal(2,a,b,c)]).
    - For f=e it will be called with *cond_equal=(ptr(CE), [])
      and will transform *cond_equal into (ptr(CE),[Item_equal(f,e)]).

  @note
    Now only fields that have the same type definitions (verified by
    the Field::eq_def method) are placed to the same multiple equalities.
    Because of this some equality predicates are not eliminated and
    can be used in the constant propagation procedure.
    We could weaken the equality test as soon as at least one of the
    equal fields is to be equal to a constant. It would require a
    more complicated implementation: we would have to store, in
    general case, its own constant for each fields from the multiple
    equality. But at the same time it would allow us to get rid
    of constant propagation completely: it would be done by the call
    to build_equal_items_for_cond.

    The implementation does not follow exactly the above rules to
    build a new multiple equality for the equality predicate.
    If it processes the equality of the form field1=field2, it
    looks for multiple equalities me1 containing field1 and me2 containing
    field2. If only one of them is found the function expands it with
    the lacking field. If multiple equalities for both fields are
    found they are merged. If both searches fail a new multiple equality
    containing just field1 and field2 is added to the existing
    multiple equalities.
    If the function processes the predicate of the form field1=const,
    it looks for a multiple equality containing field1. If found, the
    function checks the constant of the multiple equality. If the value
    is unknown, it is setup to const. Otherwise the value is compared with
    const and the evaluation of the equality predicate is performed.
    When expanding/merging equality predicates from the upper levels
    the function first copies them for the current level. It looks
    acceptable, as this happens rarely. The implementation without
    copying would be much more complicated.

  @param thd         Thread handler
  @param left_item   left term of the equality to be checked
  @param right_item  right term of the equality to be checked
  @param item        equality item if the equality originates from a condition
                     predicate, 0 if the equality is the result of row
                     elimination
  @param cond_equal  multiple equalities that must hold together with the
                     equality
  @param[out] simple_equality
                     true  if the predicate is a simple equality predicate
                           to be used for building multiple equalities
                     false otherwise

  @returns false if success, true if error
*/

static bool check_simple_equality(THD *thd, Item *left_item, Item *right_item,
                                  Item *item, COND_EQUAL *cond_equal,
                                  bool *simple_equality) {
  *simple_equality = false;

  if (left_item->type() == Item::REF_ITEM &&
      down_cast<Item_ref *>(left_item)->ref_type() == Item_ref::VIEW_REF) {
    if (down_cast<Item_ref *>(left_item)->depended_from) return false;
    left_item = left_item->real_item();
  }
  if (right_item->type() == Item::REF_ITEM &&
      down_cast<Item_ref *>(right_item)->ref_type() == Item_ref::VIEW_REF) {
    if (down_cast<Item_ref *>(right_item)->depended_from) return false;
    right_item = right_item->real_item();
  }
  const Item_field *left_item_field, *right_item_field;

  if (left_item->type() == Item::FIELD_ITEM &&
      right_item->type() == Item::FIELD_ITEM &&
      (left_item_field = down_cast<const Item_field *>(left_item)) &&
      (right_item_field = down_cast<const Item_field *>(right_item)) &&
      !left_item_field->depended_from && !right_item_field->depended_from) {
    /* The predicate the form field1=field2 is processed */

    const Field *const left_field = left_item_field->field;
    const Field *const right_field = right_item_field->field;

    if (!left_field->eq_def(right_field)) return false;

    /* Search for multiple equalities containing field1 and/or field2 */
    bool left_copyfl, right_copyfl;
    Item_equal *left_item_equal =
        find_item_equal(cond_equal, left_item_field, &left_copyfl);
    Item_equal *right_item_equal =
        find_item_equal(cond_equal, right_item_field, &right_copyfl);

    /* As (NULL=NULL) != TRUE we can't just remove the predicate f=f */
    if (left_field->eq(right_field)) /* f = f */
    {
      *simple_equality =
          !((left_field->is_nullable() || left_field->table->is_nullable()) &&
            !left_item_equal);
      return false;
    }

    if (left_item_equal && left_item_equal == right_item_equal) {
      /*
        The equality predicate is inference of one of the existing
        multiple equalities, i.e the condition is already covered
        by upper level equalities
      */
      *simple_equality = true;
      return false;
    }

    /* Copy the found multiple equalities at the current level if needed */
    if (left_copyfl) {
      /* left_item_equal of an upper level contains left_item */
      left_item_equal = new Item_equal(left_item_equal);
      if (left_item_equal == nullptr) return true;
      cond_equal->current_level.push_back(left_item_equal);
    }
    if (right_copyfl) {
      /* right_item_equal of an upper level contains right_item */
      right_item_equal = new Item_equal(right_item_equal);
      if (right_item_equal == nullptr) return true;
      cond_equal->current_level.push_back(right_item_equal);
    }

    if (left_item_equal) {
      /* left item was found in the current or one of the upper levels */
      if (!right_item_equal)
        left_item_equal->add(down_cast<Item_field *>(right_item));
      else {
        /* Merge two multiple equalities forming a new one */
        if (left_item_equal->merge(thd, right_item_equal)) return true;
        /* Remove the merged multiple equality from the list */
        List_iterator<Item_equal> li(cond_equal->current_level);
        while ((li++) != right_item_equal)
          ;
        li.remove();
      }
    } else {
      /* left item was not found neither the current nor in upper levels  */
      if (right_item_equal) {
        right_item_equal->add(down_cast<Item_field *>(left_item));
      } else {
        /* None of the fields was found in multiple equalities */
        Item_equal *item_equal =
            new Item_equal(down_cast<Item_field *>(left_item),
                           down_cast<Item_field *>(right_item));
        if (item_equal == nullptr) return true;
        cond_equal->current_level.push_back(item_equal);
      }
    }
    *simple_equality = true;
    return false;
  }

  {
    /* The predicate of the form field=const/const=field is processed */
    Item *const_item = nullptr;
    Item_field *field_item = nullptr;
    if (left_item->type() == Item::FIELD_ITEM &&
        (field_item = down_cast<Item_field *>(left_item)) &&
        field_item->depended_from == nullptr &&
        right_item->const_for_execution()) {
      const_item = right_item;
    } else if (right_item->type() == Item::FIELD_ITEM &&
               (field_item = down_cast<Item_field *>(right_item)) &&
               field_item->depended_from == nullptr &&
               left_item->const_for_execution()) {
      const_item = left_item;
    }

    // Don't evaluate subqueries if they are disabled during optimization.
    if (const_item != nullptr &&
        !evaluate_during_optimization(const_item, thd->lex->current_select()))
      return false;

    /*
      If the constant expression contains a reference to the field
      (for example, a = (a IS NULL)), we don't want to replace the
      field with the constant expression as it makes the predicates
      more complex and may introduce cycles in the Item tree.
    */
    if (const_item != nullptr &&
        const_item->walk(&Item::find_field_processor, enum_walk::POSTFIX,
                         pointer_cast<uchar *>(field_item->field)))
      return false;

    if (const_item && field_item->result_type() == const_item->result_type()) {
      if (field_item->result_type() == STRING_RESULT) {
        const CHARSET_INFO *cs = field_item->field->charset();
        if (!item) {
          Item_func_eq *const eq_item = new Item_func_eq(left_item, right_item);
          if (eq_item == nullptr || eq_item->set_cmp_func()) return true;
          eq_item->quick_fix_field();
          item = eq_item;
        }
        if ((cs != down_cast<Item_func *>(item)->compare_collation()) ||
            !cs->coll->propagate(cs, nullptr, 0))
          return false;
      }

      bool copyfl;
      Item_equal *item_equal = find_item_equal(cond_equal, field_item, &copyfl);
      if (copyfl) {
        item_equal = new Item_equal(item_equal);
        if (item_equal == nullptr) return true;
        cond_equal->current_level.push_back(item_equal);
      }
      if (item_equal) {
        /*
          The flag cond_false will be set to 1 after this, if item_equal
          already contains a constant and its value is  not equal to
          the value of const_item.
        */
        if (item_equal->add(thd, const_item, field_item)) return true;
      } else {
        item_equal = new Item_equal(const_item, field_item);
        if (item_equal == nullptr) return true;
        cond_equal->current_level.push_back(item_equal);
      }
      *simple_equality = true;
      return false;
    }
  }
  return false;
}

/**
  Convert row equalities into a conjunction of regular equalities.

    The function converts a row equality of the form (E1,...,En)=(E'1,...,E'n)
    into a list of equalities E1=E'1,...,En=E'n. For each of these equalities
    Ei=E'i the function checks whether it is a simple equality or a row
    equality. If it is a simple equality it is used to expand multiple
    equalities of cond_equal. If it is a row equality it converted to a
    sequence of equalities between row elements. If Ei=E'i is neither a
    simple equality nor a row equality the item for this predicate is added
    to eq_list.

  @param thd        thread handle
  @param left_row   left term of the row equality to be processed
  @param right_row  right term of the row equality to be processed
  @param cond_equal multiple equalities that must hold together with the
                    predicate
  @param eq_list    results of conversions of row equalities that are not
                    simple enough to form multiple equalities
  @param[out] simple_equality
                    true if the row equality is composed of only
                    simple equalities.

  @returns false if conversion succeeded, true if any error.
*/

static bool check_row_equality(THD *thd, Item *left_row, Item_row *right_row,
                               COND_EQUAL *cond_equal, List<Item> *eq_list,
                               bool *simple_equality) {
  *simple_equality = false;
  uint n = left_row->cols();
  for (uint i = 0; i < n; i++) {
    bool is_converted;
    Item *left_item = left_row->element_index(i);
    Item *right_item = right_row->element_index(i);
    if (left_item->type() == Item::ROW_ITEM &&
        right_item->type() == Item::ROW_ITEM) {
      if (check_row_equality(thd, down_cast<Item_row *>(left_item),
                             down_cast<Item_row *>(right_item), cond_equal,
                             eq_list, &is_converted))
        return true;
      if (!is_converted) thd->lex->current_select()->cond_count++;
    } else {
      if (check_simple_equality(thd, left_item, right_item, nullptr, cond_equal,
                                &is_converted))
        return true;
      thd->lex->current_select()->cond_count++;
    }

    if (!is_converted) {
      Item_func_eq *const eq_item = new Item_func_eq(left_item, right_item);
      if (eq_item == nullptr) return true;
      if (eq_item->set_cmp_func()) {
        // Failed to create cmp func -> not only simple equalitities
        return true;
      }
      eq_item->quick_fix_field();
      eq_list->push_back(eq_item);
    }
  }
  *simple_equality = true;
  return false;
}

/**
  Eliminate row equalities and form multiple equalities predicates.

    This function checks whether the item is a simple equality
    i.e. the one that equates a field with another field or a constant
    (field=field_item or field=constant_item), or, a row equality.
    For a simple equality the function looks for a multiple equality
    in the lists referenced directly or indirectly by cond_equal inferring
    the given simple equality. If it doesn't find any, it builds/expands
    multiple equality that covers the predicate.
    Row equalities are eliminated substituted for conjunctive regular
    equalities which are treated in the same way as original equality
    predicates.

  @param thd        thread handle
  @param item       predicate to process
  @param cond_equal multiple equalities that must hold together with the
                    predicate
  @param eq_list    results of conversions of row equalities that are not
                    simple enough to form multiple equalities
  @param[out] equality
                    true if re-writing rules have been applied
                    false otherwise, i.e.
                      if the predicate is not an equality, or
                      if the equality is neither a simple nor a row equality

  @returns false if success, true if error

  @note If the equality was created by IN->EXISTS, it may be removed later by
  subquery materialization. So we don't mix this possibly temporary equality
  with others; if we let it go into a multiple-equality (Item_equal), then we
  could not remove it later. There is however an exception: if the outer
  expression is a constant, it is safe to leave the equality even in
  materialization; all it can do is preventing NULL/FALSE distinction but if
  such distinction mattered the equality would be in a triggered condition so
  we would not come to this function. And injecting constants is good because
  it makes the materialized table smaller.
*/

static bool check_equality(THD *thd, Item *item, COND_EQUAL *cond_equal,
                           List<Item> *eq_list, bool *equality) {
  *equality = false;
  DBUG_ASSERT(item->is_bool_func());
  Item_func *item_func;
  if (item->type() == Item::FUNC_ITEM &&
      (item_func = down_cast<Item_func *>(item))->functype() ==
          Item_func::EQ_FUNC) {
    Item *left_item = item_func->arguments()[0];
    Item *right_item = item_func->arguments()[1];

    if (item->created_by_in2exists() && !left_item->const_item())
      return false;  // See note above

    if (left_item->type() == Item::ROW_ITEM &&
        right_item->type() == Item::ROW_ITEM) {
      thd->lex->current_select()->cond_count--;
      return check_row_equality(thd, down_cast<Item_row *>(left_item),
                                down_cast<Item_row *>(right_item), cond_equal,
                                eq_list, equality);
    } else
      return check_simple_equality(thd, left_item, right_item, item, cond_equal,
                                   equality);
  }

  return false;
}

/**
  Replace all equality predicates in a condition by multiple equality items.

    At each 'and' level the function detects items for equality predicates
    and replaces them by a set of multiple equality items of class Item_equal,
    taking into account inherited equalities from upper levels.
    If an equality predicate is used not in a conjunction it's just
    replaced by a multiple equality predicate.
    For each 'and' level the function set a pointer to the inherited
    multiple equalities in the cond_equal field of the associated
    object of the type Item_cond_and.
    The function also traverses the cond tree and for each field reference
    sets a pointer to the multiple equality item containing the field, if there
    is any. If this multiple equality equates fields to a constant the
    function replaces the field reference by the constant in the cases
    when the field is not of a string type or when the field reference is
    just an argument of a comparison predicate.
    The function also determines the maximum number of members in
    equality lists of each Item_cond_and object assigning it to
    thd->lex->current_select()->max_equal_elems.

  @note
    Multiple equality predicate =(f1,..fn) is equivalent to the conjuction of
    f1=f2, .., fn-1=fn. It substitutes any inference from these
    equality predicates that is equivalent to the conjunction.
    Thus, =(a1,a2,a3) can substitute for ((a1=a3) AND (a2=a3) AND (a2=a1)) as
    it is equivalent to ((a1=a2) AND (a2=a3)).
    The function always makes a substitution of all equality predicates occurred
    in a conjunction for a minimal set of multiple equality predicates.
    This set can be considered as a canonical representation of the
    sub-conjunction of the equality predicates.
    E.g. (t1.a=t2.b AND t2.b>5 AND t1.a=t3.c) is replaced by
    (=(t1.a,t2.b,t3.c) AND t2.b>5), not by
    (=(t1.a,t2.b) AND =(t1.a,t3.c) AND t2.b>5);
    while (t1.a=t2.b AND t2.b>5 AND t3.c=t4.d) is replaced by
    (=(t1.a,t2.b) AND =(t3.c=t4.d) AND t2.b>5),
    but if additionally =(t4.d,t2.b) is inherited, it
    will be replaced by (=(t1.a,t2.b,t3.c,t4.d) AND t2.b>5)

    The function performs the substitution in a recursive descent of
    the condition tree, passing to the next AND level a chain of multiple
    equality predicates which have been built at the upper levels.
    The Item_equal items built at the level are attached to other
    non-equality conjuncts as a sublist. The pointer to the inherited
    multiple equalities is saved in the and condition object (Item_cond_and).
    This chain allows us for any field reference occurence to easily find a
    multiple equality that must be held for this occurence.
    For each AND level we do the following:
    - scan it for all equality predicate (=) items
    - join them into disjoint Item_equal() groups
    - process the included OR conditions recursively to do the same for
      lower AND levels.

    We need to do things in this order as lower AND levels need to know about
    all possible Item_equal objects in upper levels.

  @param thd          thread handle
  @param cond         condition(expression) where to make replacement
  @param[out] retcond returned condition
  @param inherited    path to all inherited multiple equality items
  @param do_inherit   whether or not to inherit equalities from other parts
                      of the condition

  @returns false if success, true if error
*/

static bool build_equal_items_for_cond(THD *thd, Item *cond, Item **retcond,
                                       COND_EQUAL *inherited, bool do_inherit) {
  Item_equal *item_equal;
  COND_EQUAL cond_equal;
  cond_equal.upper_levels = inherited;
  DBUG_ASSERT(cond->is_bool_func());
  if (check_stack_overrun(thd, STACK_MIN_SIZE, nullptr))
    return true;  // Fatal error flag is set!

  const enum Item::Type cond_type = cond->type();
  if (cond_type == Item::COND_ITEM) {
    List<Item> eq_list;
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    const bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List<Item> *args = item_cond->argument_list();

    List_iterator<Item> li(*args);
    Item *item;

    if (and_level) {
      /*
         Retrieve all conjuncts of this level detecting the equality
         that are subject to substitution by multiple equality items and
         removing each such predicate from the conjunction after having
         found/created a multiple equality whose inference the predicate is.
       */
      while ((item = li++)) {
        /*
          PS/SP note: we can safely remove a node from AND-OR
          structure here because it's restored before each
          re-execution of any prepared statement/stored procedure.
        */
        bool equality;
        if (check_equality(thd, item, &cond_equal, &eq_list, &equality))
          return true;
        if (equality) li.remove();
      }

      /*
        Check if we eliminated all the predicates of the level, e.g.
        (a=a AND b=b AND a=a).
      */
      if (!args->elements && !cond_equal.current_level.elements &&
          !eq_list.elements) {
        *retcond = new Item_func_true();
        return *retcond == nullptr;
      }

      List_iterator_fast<Item_equal> it(cond_equal.current_level);
      while ((item_equal = it++)) {
        if (item_equal->resolve_type(thd)) return true;
        item_equal->update_used_tables();
        thd->lex->current_select()->max_equal_elems = std::max(
            thd->lex->current_select()->max_equal_elems, item_equal->members());
      }

      Item_cond_and *const item_cond_and = down_cast<Item_cond_and *>(cond);
      item_cond_and->cond_equal = cond_equal;
      inherited = &item_cond_and->cond_equal;
    }
    /*
       Make replacement of equality predicates for lower levels
       of the condition expression.
    */
    li.rewind();
    while ((item = li++)) {
      Item *new_item;
      if (build_equal_items_for_cond(thd, item, &new_item, inherited,
                                     do_inherit))
        return true;
      if (new_item != item) {
        /* This replacement happens only for standalone equalities */
        /*
          This is ok with PS/SP as the replacement is done for
          arguments of an AND/OR item, which are restored for each
          execution of PS/SP.
        */
        li.replace(new_item);
      }
    }
    if (and_level) {
      args->concat(&eq_list);
      args->concat((List<Item> *)&cond_equal.current_level);
    }
  } else if (cond->type() == Item::FUNC_ITEM) {
    List<Item> eq_list;
    /*
      If an equality predicate forms the whole and level,
      we call it standalone equality and it's processed here.
      E.g. in the following where condition
      WHERE a=5 AND (b=5 or a=c)
      (b=5) and (a=c) are standalone equalities.
      In general we can't leave alone standalone eqalities:
      for WHERE a=b AND c=d AND (b=c OR d=5)
      b=c is replaced by =(a,b,c,d).
     */
    bool equality;
    if (check_equality(thd, cond, &cond_equal, &eq_list, &equality))
      return true;
    if (equality) {
      int n = cond_equal.current_level.elements + eq_list.elements;
      if (n == 0) {
        *retcond = new Item_func_true();
        return *retcond == nullptr;
      } else if (n == 1) {
        if ((item_equal = cond_equal.current_level.pop())) {
          if (item_equal->resolve_type(thd)) return true;
          item_equal->update_used_tables();
          thd->lex->current_select()->max_equal_elems =
              std::max(thd->lex->current_select()->max_equal_elems,
                       item_equal->members());
          *retcond = item_equal;
          return false;
        }

        *retcond = eq_list.pop();
        return false;
      } else {
        /*
          Here a new AND level must be created. It can happen only
          when a row equality is processed as a standalone predicate.
        */
        Item_cond_and *and_cond = new Item_cond_and(eq_list);
        if (and_cond == nullptr) return true;

        and_cond->quick_fix_field();
        List<Item> *args = and_cond->argument_list();
        List_iterator_fast<Item_equal> it(cond_equal.current_level);
        while ((item_equal = it++)) {
          if (item_equal->resolve_type(thd)) return true;
          item_equal->update_used_tables();
          thd->lex->current_select()->max_equal_elems =
              std::max(thd->lex->current_select()->max_equal_elems,
                       item_equal->members());
        }
        and_cond->cond_equal = cond_equal;
        args->concat((List<Item> *)&cond_equal.current_level);

        *retcond = and_cond;
        return false;
      }
    }

    if (do_inherit) {
      /*
        For each field reference in cond, not from equal item predicates,
        set a pointer to the multiple equality it belongs to (if there is any)
        as soon the field is not of a string type or the field reference is
        an argument of a comparison predicate.
      */
      uchar *is_subst_valid = (uchar *)1;
      cond = cond->compile(&Item::subst_argument_checker, &is_subst_valid,
                           &Item::equal_fields_propagator, (uchar *)inherited);
      if (cond == nullptr) return true;
    }
    cond->update_used_tables();
  }
  *retcond = cond;
  return false;
}

/**
  Build multiple equalities for a WHERE condition and all join conditions that
  inherit these multiple equalities.

    The function first applies the build_equal_items_for_cond function
    to build all multiple equalities for condition cond utilizing equalities
    referred through the parameter inherited. The extended set of
    equalities is returned in the structure referred by the cond_equal_ref
    parameter. After this the function calls itself recursively for
    all join conditions whose direct references can be found in join_list
    and who inherit directly the multiple equalities just having built.

  @note
    The join condition used in an outer join operation inherits all equalities
    from the join condition of the embedding join, if there is any, or
    otherwise - from the where condition.
    This fact is not obvious, but presumably can be proved.
    Consider the following query:
    @code
      SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t1.a=t3.a AND t2.a=t4.a
        WHERE t1.a=t2.a;
    @endcode
    If the join condition in the query inherits =(t1.a,t2.a), then we
    can build the multiple equality =(t1.a,t2.a,t3.a,t4.a) that infers
    the equality t3.a=t4.a. Although the join condition
    t1.a=t3.a AND t2.a=t4.a AND t3.a=t4.a is not equivalent to the one
    in the query the latter can be replaced by the former: the new query
    will return the same result set as the original one.

    Interesting that multiple equality =(t1.a,t2.a,t3.a,t4.a) allows us
    to use t1.a=t3.a AND t3.a=t4.a under the join condition:
    @code
      SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t1.a=t3.a AND t3.a=t4.a
        WHERE t1.a=t2.a
    @endcode
    This query equivalent to:
    @code
      SELECT * FROM (t1 LEFT JOIN (t3,t4) ON t1.a=t3.a AND t3.a=t4.a),t2
        WHERE t1.a=t2.a
    @endcode
    Similarly the original query can be rewritten to the query:
    @code
      SELECT * FROM (t1,t2) LEFT JOIN (t3,t4) ON t2.a=t4.a AND t3.a=t4.a
        WHERE t1.a=t2.a
    @endcode
    that is equivalent to:
    @code
      SELECT * FROM (t2 LEFT JOIN (t3,t4)ON t2.a=t4.a AND t3.a=t4.a), t1
        WHERE t1.a=t2.a
    @endcode
    Thus, applying equalities from the where condition we basically
    can get more freedom in performing join operations.
    Although we don't use this property now, it probably makes sense to use
    it in the future.

  @param thd		     Thread handler
  @param cond                condition to build the multiple equalities for
  @param[out] retcond        Returned condition
  @param inherited           path to all inherited multiple equality items
  @param do_inherit          whether or not to inherit equalities from other
                             parts of the condition
  @param join_list           list of join tables that the condition refers to
  @param[out] cond_equal_ref pointer to the structure to place built
                             equalities in

  @returns false if success, true if error
*/

bool build_equal_items(THD *thd, Item *cond, Item **retcond,
                       COND_EQUAL *inherited, bool do_inherit,
                       mem_root_deque<TABLE_LIST *> *join_list,
                       COND_EQUAL **cond_equal_ref) {
  COND_EQUAL *cond_equal = nullptr;

  if (cond) {
    if (build_equal_items_for_cond(thd, cond, &cond, inherited, do_inherit))
      return true;
    cond->update_used_tables();
    // update_used_tables() returns void but can stil fail.
    if (thd->is_error()) return true;

    const enum Item::Type cond_type = cond->type();
    if (cond_type == Item::COND_ITEM &&
        down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC)
      cond_equal = &down_cast<Item_cond_and *>(cond)->cond_equal;
    else if (cond_type == Item::FUNC_ITEM &&
             down_cast<Item_func *>(cond)->functype() ==
                 Item_func::MULT_EQUAL_FUNC) {
      cond_equal = new (thd->mem_root) COND_EQUAL;
      if (cond_equal == nullptr) return true;
      cond_equal->current_level.push_back(down_cast<Item_equal *>(cond));
    }
  }
  if (cond_equal) {
    cond_equal->upper_levels = inherited;
    inherited = cond_equal;
  }
  *cond_equal_ref = cond_equal;

  if (join_list) {
    for (TABLE_LIST *table : *join_list) {
      if (table->join_cond_optim()) {
        mem_root_deque<TABLE_LIST *> *nested_join_list =
            table->nested_join ? &table->nested_join->join_list : nullptr;
        Item *join_cond;
        if (build_equal_items(thd, table->join_cond_optim(), &join_cond,
                              inherited, do_inherit, nested_join_list,
                              &table->cond_equal))
          return true;
        table->set_join_cond_optim(join_cond);
      }
    }
  }

  *retcond = cond;
  return false;
}

/**
  Compare field items by table order in the execution plan.

    field1 considered as better than field2 if the table containing
    field1 is accessed earlier than the table containing field2.
    The function finds out what of two fields is better according
    this criteria.

  @param field1          first field item to compare
  @param field2          second field item to compare
  @param table_join_idx  index to tables determining table order

  @retval
   -1  if field1 is better than field2
  @retval
    1  if field2 is better than field1
  @retval
    0  otherwise
*/

static int compare_fields_by_table_order(Item_field *field1, Item_field *field2,
                                         JOIN_TAB **table_join_idx) {
  int cmp = 0;
  bool outer_ref = false;
  if (field1->used_tables() & OUTER_REF_TABLE_BIT) {
    outer_ref = true;
    cmp = -1;
  }
  if (field2->used_tables() & OUTER_REF_TABLE_BIT) {
    outer_ref = true;
    cmp++;
  }
  if (outer_ref) return cmp;

  /*
    table_join_idx is NULL if this function was not called from JOIN::optimize()
    but from e.g. mysql_delete() or mysql_update(). In these cases
    there is only one table and both fields belong to it. Example
    condition where this is the case: t1.fld1=t1.fld2
  */
  if (!table_join_idx) return 0;

  // Locate JOIN_TABs thanks to table_join_idx, then compare their index.
  cmp = table_join_idx[field1->table_ref->tableno()]->idx() -
        table_join_idx[field2->table_ref->tableno()]->idx();
  return cmp < 0 ? -1 : (cmp ? 1 : 0);
}

/**
  Generate minimal set of simple equalities equivalent to a multiple equality.

    The function retrieves the fields of the multiple equality item
    item_equal and  for each field f:
    - if item_equal contains const it generates the equality f=const_item;
    - otherwise, if f is not the first field, generates the equality
      f=item_equal->get_first().
    All generated equality are added to the cond conjunction.

  @param thd             the session context
  @param cond            condition to add the generated equality to
  @param upper_levels    structure to access multiple equality of upper levels
  @param item_equal      multiple equality to generate simple equality from

  @note
    Before generating an equality function checks that it has not
    been generated for multiple equalities of the upper levels.
    E.g. for the following where condition
    WHERE a=5 AND ((a=b AND b=c) OR  c>4)
    the upper level AND condition will contain =(5,a),
    while the lower level AND condition will contain =(5,a,b,c).
    When splitting =(5,a,b,c) into a separate equality predicates
    we should omit 5=a, as we have it already in the upper level.
    The following where condition gives us a more complicated case:
    WHERE t1.a=t2.b AND t3.c=t4.d AND (t2.b=t3.c OR t4.e>5 ...) AND ...
    Given the tables are accessed in the order t1->t2->t3->t4 for
    the selected query execution plan the lower level multiple
    equality =(t1.a,t2.b,t3.c,t4.d) formally  should be converted to
    t1.a=t2.b AND t1.a=t3.c AND t1.a=t4.d. But t1.a=t2.a will be
    generated for the upper level. Also t3.c=t4.d will be generated there.
    So only t1.a=t3.c should be left in the lower level.
    If cond is equal to 0, then not more then one equality is generated
    and a pointer to it is returned as the result of the function.

  @return
    - The condition with generated simple equalities or
    a pointer to the simple generated equality, if success.
    - 0, otherwise.
*/

static Item *eliminate_item_equal(THD *thd, Item *cond,
                                  COND_EQUAL *upper_levels,
                                  Item_equal *item_equal) {
  List<Item> eq_list;
  Item *eq_item = nullptr;
  if (((Item *)item_equal)->const_item() && !item_equal->val_int())
    return new Item_func_false();
  Item *const item_const = item_equal->get_const();
  Item_equal_iterator it(*item_equal);
  if (!item_const) {
    /*
      If there is a const item, match all field items with the const item,
      otherwise match the second and subsequent field items with the first one:
    */
    it++;
  }
  Item_field *item_field;  // Field to generate equality for.
  while ((item_field = it++)) {
    /*
      Generate an equality of the form:
      item_field = some previous field in item_equal's list.

      First see if we really need to generate it:
    */
    Item_equal *const upper = item_field->find_item_equal(upper_levels);
    if (upper)  // item_field is in this upper equality
    {
      if (item_const && upper->get_const())
        continue;  // Const at both levels, no need to generate at current level
      /*
        If the upper-level multiple equality contains this item, there is no
        need to generate the equality, unless item_field belongs to a
        semi-join nest that is used for Materialization, and refers to tables
        that are outside of the materialized semi-join nest,
        As noted in Item_equal::get_subst_item(), subquery materialization
        does not have this problem.
      */
      JOIN_TAB *const tab = item_field->field->table->reginfo.join_tab;

      if (!(tab && sj_is_materialize_strategy(tab->get_sj_strategy()))) {
        Item_field *item_match;
        Item_equal_iterator li(*item_equal);
        while ((item_match = li++) != item_field) {
          if (item_match->find_item_equal(upper_levels) == upper)
            break;  // (item_match, item_field) is also in upper level equality
        }
        if (item_match != item_field) continue;
      }
    }  // ... if (upper).

    /*
      item_field should be compared with the head of the multiple equality
      list.
      item_field may refer to a table that is within a semijoin materialization
      nest. In that case, the order of the join_tab entries may look like:

        ot1 ot2 <subquery> ot5 SJM(it3 it4)

      If we have a multiple equality

        (ot1.c1, ot2.c2, <subquery>.c it3.c3, it4.c4, ot5.c5),

      we should generate the following equalities:
        1. ot1.c1 = ot2.c2
        2. ot1.c1 = <subquery>.c
        3. it3.c3 = it4.c4
        4. ot1.c1 = ot5.c5

      Equalities 1) and 4) are regular equalities between two outer tables.
      Equality 2) is an equality that matches the outer query with a
      materialized temporary table. It is either performed as a lookup
      into the materialized table (SJM-lookup), or as a condition on the
      outer table (SJM-scan).
      Equality 3) is evaluated during semijoin materialization.

      If there is a const item, match against this one.
      Otherwise, match against the first field item in the multiple equality,
      unless the item is within a materialized semijoin nest, in case it will
      be matched against the first item within the SJM nest.
      @see JOIN::set_prefix_tables()
      @see Item_equal::get_subst_item()
    */

    Item *const head =
        item_const ? item_const : item_equal->get_subst_item(item_field);
    if (head == item_field) continue;

    // we have a pair, can generate 'item_field=head'
    if (eq_item) eq_list.push_back(eq_item);

    if (head->type() == Item::FIELD_ITEM) {
      // Store away all fields that were considered equal, so that we are able
      // to undo this operation later if we have to. See
      // Item_func::ensure_multi_equality_fields_are_available for more details.
      Item_field *head_field = down_cast<Item_field *>(head);
      head_field->set_item_equal_all_join_nests(item_equal);
    }
    eq_item = new Item_func_eq(item_field, head);

    if (!eq_item || down_cast<Item_func_eq *>(eq_item)->set_cmp_func())
      return nullptr;

    eq_item->quick_fix_field();
    if (item_const != nullptr) {
      eq_item->apply_is_true();
      Item::cond_result res;
      if (fold_condition(thd, eq_item, &eq_item, &res)) return nullptr;
      if (res == Item::COND_FALSE) {
        eq_item = new (thd->mem_root) Item_func_false();
        if (eq_item == nullptr) return nullptr;
        return eq_item;  // entire AND is false
      } else if (res == Item::COND_TRUE) {
        eq_item = new (thd->mem_root) Item_func_true();
        if (eq_item == nullptr) return nullptr;
      }
    }
  }  // ... while ((item_field= it++))

  if (!cond && !eq_list.head()) {
    if (!eq_item) return new Item_func_true();
    return eq_item;
  }

  if (eq_item) eq_list.push_back(eq_item);
  if (!cond)
    cond = new Item_cond_and(eq_list);
  else {
    DBUG_ASSERT(cond->type() == Item::COND_ITEM);
    if (eq_list.elements) ((Item_cond *)cond)->add_at_head(&eq_list);
  }

  cond->quick_fix_field();
  cond->update_used_tables();

  return cond;
}

/**
  Substitute every field reference in a condition by the best equal field
  and eliminate all multiple equality predicates.

    The function retrieves the cond condition and for each encountered
    multiple equality predicate it sorts the field references in it
    according to the order of tables specified by the table_join_idx
    parameter. Then it eliminates the multiple equality predicate by
    replacing it with the conjunction of simple equality predicates
    equating every field from the multiple equality to the first
    field in it, or to the constant, if there is any.
    After this, the function retrieves all other conjuncted
    predicates and substitutes every field reference by the field reference
    to the first equal field or equal constant if there are any.

  @param thd             the session context
  @param cond            condition to process
  @param cond_equal      multiple equalities to take into consideration
  @param table_join_idx  index to tables determining field preference

  @note
    At the first glance, a full sort of fields in multiple equality
    seems to be an overkill. Yet it's not the case due to possible
    new fields in multiple equality item of lower levels. We want
    the order in them to comply with the order of upper levels.

  @return
    The transformed condition, or NULL in case of error
*/

Item *substitute_for_best_equal_field(THD *thd, Item *cond,
                                      COND_EQUAL *cond_equal,
                                      JOIN_TAB **table_join_idx) {
  DBUG_ASSERT(cond->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    List<Item> *cond_list = ((Item_cond *)cond)->argument_list();

    bool and_level =
        ((Item_cond *)cond)->functype() == Item_func::COND_AND_FUNC;
    if (and_level) {
      cond_equal = &((Item_cond_and *)cond)->cond_equal;
      cond_list->disjoin((List<Item> *)&cond_equal->current_level);

      List_iterator_fast<Item_equal> it(cond_equal->current_level);
      auto cmp = [table_join_idx](Item_field *f1, Item_field *f2) {
        return compare_fields_by_table_order(f1, f2, table_join_idx);
      };
      Item_equal *item_equal;
      while ((item_equal = it++)) {
        item_equal->sort(cmp);
      }
    }

    List_iterator<Item> li(*cond_list);
    Item *item;
    while ((item = li++)) {
      Item *new_item = substitute_for_best_equal_field(thd, item, cond_equal,
                                                       table_join_idx);
      if (new_item == nullptr) return nullptr;
      /*
        This works OK with PS/SP re-execution as changes are made to
        the arguments of AND/OR items only
      */
      if (new_item != item) li.replace(new_item);
    }

    if (and_level) {
      List_iterator_fast<Item_equal> it(cond_equal->current_level);
      Item_equal *item_equal;
      while ((item_equal = it++)) {
        cond = eliminate_item_equal(thd, cond, cond_equal->upper_levels,
                                    item_equal);
        if (cond == nullptr) return nullptr;
        // This occurs when eliminate_item_equal() founds that cond is
        // always false and substitutes it with a false value.
        // Due to this, value of item_equal will be 0, so just return it.
        if (cond->type() != Item::COND_ITEM) break;
      }
    }
    if (cond->type() == Item::COND_ITEM &&
        !((Item_cond *)cond)->argument_list()->elements)
      cond = cond->val_bool() ? down_cast<Item *>(new Item_func_true())
                              : down_cast<Item *>(new Item_func_false());
  } else if (cond->type() == Item::FUNC_ITEM &&
             (down_cast<Item_func *>(cond))->functype() ==
                 Item_func::MULT_EQUAL_FUNC) {
    Item_equal *item_equal = down_cast<Item_equal *>(cond);
    item_equal->sort([table_join_idx](Item_field *f1, Item_field *f2) {
      return compare_fields_by_table_order(f1, f2, table_join_idx);
    });
    if (cond_equal && cond_equal->current_level.head() == item_equal)
      cond_equal = cond_equal->upper_levels;
    return eliminate_item_equal(thd, nullptr, cond_equal, item_equal);
  } else {
    cond->transform(&Item::replace_equal_field, nullptr);
  }
  return cond;
}

/**
  change field = field to field = const for each found field = const in the
  and_level

  @param thd      Thread handler
  @param save_list
  @param and_father
  @param cond       Condition where fields are replaced with constant values
  @param field      The field that will be substituted
  @param value      The substitution value

  @returns false if success, true if error
*/

static bool change_cond_ref_to_const(THD *thd, I_List<COND_CMP> *save_list,
                                     Item *and_father, Item *cond, Item *field,
                                     Item *value) {
  DBUG_ASSERT(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator<Item> li(*item_cond->argument_list());
    Item *item;
    while ((item = li++)) {
      if (change_cond_ref_to_const(thd, save_list, and_level ? cond : item,
                                   item, field, value))
        return true;
    }
    return false;
  }
  if (cond->eq_cmp_result() == Item::COND_OK)
    return false;  // Not a boolean function

  Item_bool_func2 *func = down_cast<Item_bool_func2 *>(cond);
  Item **args = func->arguments();
  Item *left_item = args[0];
  Item *right_item = args[1];
  Item_func::Functype functype = func->functype();

  if (right_item->eq(field, false) && left_item != value &&
      right_item->cmp_context == field->cmp_context &&
      (left_item->result_type() != STRING_RESULT ||
       value->result_type() != STRING_RESULT ||
       left_item->collation.collation == value->collation.collation)) {
    Item *const clone = value->clone_item();
    if (thd->is_error()) return true;

    if (clone == nullptr) return false;

    clone->collation.set(right_item->collation);
    thd->change_item_tree(args + 1, clone);
    func->update_used_tables();
    if ((functype == Item_func::EQ_FUNC || functype == Item_func::EQUAL_FUNC) &&
        and_father != cond && !left_item->const_item()) {
      cond->marker = Item::MARKER_CONST_PROPAG;
      COND_CMP *const cond_cmp = new COND_CMP(and_father, func);
      if (cond_cmp == nullptr) return true;

      save_list->push_back(cond_cmp);
    }
    if (func->set_cmp_func()) return true;
  } else if (left_item->eq(field, false) && right_item != value &&
             left_item->cmp_context == field->cmp_context &&
             (right_item->result_type() != STRING_RESULT ||
              value->result_type() != STRING_RESULT ||
              right_item->collation.collation == value->collation.collation)) {
    Item *const clone = value->clone_item();
    if (thd->is_error()) return true;

    if (clone == nullptr) return false;

    clone->collation.set(left_item->collation);
    thd->change_item_tree(args, clone);
    value = clone;
    func->update_used_tables();
    if ((functype == Item_func::EQ_FUNC || functype == Item_func::EQUAL_FUNC) &&
        and_father != cond && !right_item->const_item()) {
      args[0] = args[1];  // For easy check
      thd->change_item_tree(args + 1, value);
      cond->marker = Item::MARKER_CONST_PROPAG;
      COND_CMP *const cond_cmp = new COND_CMP(and_father, func);
      if (cond_cmp == nullptr) return true;

      save_list->push_back(cond_cmp);
    }
    if (func->set_cmp_func()) return true;
  }
  return false;
}

/**
  Propagate constant values in a condition

  @param thd        Thread handler
  @param save_list
  @param and_father
  @param cond       Condition for which constant values are propagated

  @returns false if success, true if error
*/
static bool propagate_cond_constants(THD *thd, I_List<COND_CMP> *save_list,
                                     Item *and_father, Item *cond) {
  DBUG_ASSERT(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator_fast<Item> li(*item_cond->argument_list());
    Item *item;
    I_List<COND_CMP> save;
    while ((item = li++)) {
      if (propagate_cond_constants(thd, &save, and_level ? cond : item, item))
        return true;
    }
    if (and_level) {  // Handle other found items
      I_List_iterator<COND_CMP> cond_itr(save);
      COND_CMP *cond_cmp;
      while ((cond_cmp = cond_itr++)) {
        Item **args = cond_cmp->cmp_func->arguments();
        if (!args[0]->const_item() &&
            change_cond_ref_to_const(thd, &save, cond_cmp->and_level,
                                     cond_cmp->and_level, args[0], args[1]))
          return true;
      }
    }
  } else if (and_father != cond &&
             cond->marker != Item::MARKER_CONST_PROPAG)  // In a AND group
  {
    Item_func *func;
    if (cond->type() == Item::FUNC_ITEM &&
        (func = down_cast<Item_func *>(cond)) &&
        (func->functype() == Item_func::EQ_FUNC ||
         func->functype() == Item_func::EQUAL_FUNC)) {
      Item **args = func->arguments();
      bool left_const = args[0]->const_item();
      bool right_const = args[1]->const_item();
      if (!(left_const && right_const) &&
          args[0]->result_type() == args[1]->result_type()) {
        if (right_const) {
          if (resolve_const_item(thd, &args[1], args[0])) return true;
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[0], args[1]))
            return true;
        } else if (left_const) {
          if (resolve_const_item(thd, &args[0], args[1])) return true;
          func->update_used_tables();
          if (change_cond_ref_to_const(thd, save_list, and_father, and_father,
                                       args[1], args[0]))
            return true;
        }
      }
    }
  }

  return false;
}

/**
  Assign each nested join structure a bit in nested_join_map.

  @param join_list     List of tables
  @param first_unused  Number of first unused bit in nested_join_map before the
                       call

  @note
    This function is called after simplify_joins(), when there are no
    redundant nested joins.
    We cannot have more nested joins in a query block than there are tables,
    so as long as the number of bits in nested_join_map is not less than the
    maximum number of tables in a query block, nested_join_map can never
    overflow.

  @return
    First unused bit in nested_join_map after the call.
*/

uint build_bitmap_for_nested_joins(mem_root_deque<TABLE_LIST *> *join_list,
                                   uint first_unused) {
  DBUG_TRACE;
  for (TABLE_LIST *table : *join_list) {
    NESTED_JOIN *nested_join;
    if ((nested_join = table->nested_join)) {
      // We should have a join condition or a semi-join condition or both
      DBUG_ASSERT((table->join_cond() != nullptr) || table->is_sj_nest());

      nested_join->nj_map = 0;
      nested_join->nj_total = 0;
      /*
        We only record nested join information for outer join nests.
        Tables belonging in semi-join nests are recorded in the
        embedding outer join nest, if one exists.
      */
      if (table->join_cond()) {
        DBUG_ASSERT(first_unused < sizeof(nested_join_map) * 8);
        nested_join->nj_map = (nested_join_map)1 << first_unused++;
        nested_join->nj_total = nested_join->join_list.size();
      } else if (table->is_sj_nest()) {
        NESTED_JOIN *const outer_nest =
            table->embedding ? table->embedding->nested_join : nullptr;
        /*
          The semi-join nest has already been counted into the table count
          for the outer join nest as one table, so subtract 1 from the
          table count.
        */
        if (outer_nest)
          outer_nest->nj_total += (nested_join->join_list.size() - 1);
      } else
        DBUG_ASSERT(false);

      first_unused =
          build_bitmap_for_nested_joins(&nested_join->join_list, first_unused);
    }
  }
  return first_unused;
}

/** Update the dependency map for the tables. */

void JOIN::update_depend_map() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  for (uint tableno = 0; tableno < tables; tableno++) {
    JOIN_TAB *const tab = best_ref[tableno];
    TABLE_REF *const ref = &tab->ref();
    table_map depend_map = 0;
    Item **item = ref->items;
    for (uint i = 0; i < ref->key_parts; i++, item++)
      depend_map |= (*item)->used_tables();
    depend_map &= ~PSEUDO_TABLE_BITS;
    ref->depend_map = depend_map;
    for (JOIN_TAB **tab2 = map2table; depend_map; tab2++, depend_map >>= 1) {
      if (depend_map & 1) ref->depend_map |= (*tab2)->ref().depend_map;
    }
  }
}

/** Update the dependency map for the sort order. */

void JOIN::update_depend_map(ORDER *order) {
  DBUG_TRACE;
  for (; order; order = order->next) {
    table_map depend_map;
    order->item[0]->update_used_tables();
    order->depend_map = depend_map =
        order->item[0]->used_tables() & ~INNER_TABLE_BIT;
    order->used = 0;
    // Not item_sum(), RAND() and no reference to table outside of sub select
    if (!(order->depend_map & (OUTER_REF_TABLE_BIT | RAND_TABLE_BIT)) &&
        !order->item[0]->has_aggregation()) {
      for (JOIN_TAB **tab = map2table; depend_map; tab++, depend_map >>= 1) {
        if (depend_map & 1) order->depend_map |= (*tab)->ref().depend_map;
      }
    }
  }
}

/**
  Update equalities and keyuse references after semi-join materialization
  strategy is chosen.

  @details
    For each multiple equality that contains a field that is selected
    from a subquery, and that subquery is executed using a semi-join
    materialization strategy, add the corresponding column in the materialized
    temporary table to the equality.
    For each injected semi-join equality that is not converted to
    multiple equality, replace the reference to the expression selected
    from the subquery with the corresponding column in the temporary table.

    This is needed to properly reflect the equalities that involve injected
    semi-join equalities when materialization strategy is chosen.
    @see eliminate_item_equal() for how these equalities are used to generate
    correct equality predicates.

    The MaterializeScan semi-join strategy requires some additional processing:
    All primary tables after the materialized temporary table must be inspected
    for keyuse objects that point to expressions from the subquery tables.
    These references must be replaced with references to corresponding columns
    in the materialized temporary table instead. Those primary tables using
    ref access will thus be made to depend on the materialized temporary table
    instead of the subquery tables.

    Only the injected semi-join equalities need this treatment, other predicates
    will be handled correctly by the regular item substitution process.

  @return False if success, true if error
*/

bool JOIN::update_equalities_for_sjm() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  List_iterator<Semijoin_mat_exec> sj_it(sjm_exec_list);
  Semijoin_mat_exec *sjm_exec;
  while ((sjm_exec = sj_it++)) {
    TABLE_LIST *const sj_nest = sjm_exec->sj_nest;

    Item *cond;
    /*
      Conditions involving SJ-inner tables are only to be found in the closest
      nest's condition, which may be an AJ nest, a LEFT JOIN nest, or the
      WHERE clause.
    */
    if (sj_nest->is_aj_nest())
      cond = sj_nest->join_cond_optim();
    else if (sj_nest->outer_join_nest())
      cond = sj_nest->outer_join_nest()->join_cond_optim();
    else
      cond = where_cond;
    if (!cond) continue;

    uchar *dummy = nullptr;
    cond = cond->compile(&Item::equality_substitution_analyzer, &dummy,
                         &Item::equality_substitution_transformer,
                         (uchar *)sj_nest);
    if (cond == nullptr) return true;

    cond->update_used_tables();

    // Loop over all primary tables that follow the materialized table
    for (uint j = sjm_exec->mat_table_index + 1; j < primary_tables; j++) {
      JOIN_TAB *const tab = best_ref[j];
      for (Key_use *keyuse = tab->position()->key;
           keyuse && keyuse->table_ref == tab->table_ref &&
           keyuse->key == tab->position()->key->key;
           keyuse++) {
        List_iterator<Item> it(sj_nest->nested_join->sj_inner_exprs);
        Item *old;
        uint fieldno = 0;
        while ((old = it++)) {
          if (old->real_item()->eq(keyuse->val->real_item(), false)) {
            /*
              Replace the expression selected from the subquery with the
              corresponding column of the materialized temporary table.
            */
            keyuse->val = sj_nest->nested_join->sjm.mat_fields[fieldno];
            keyuse->used_tables = keyuse->val->used_tables();
            break;
          }
          fieldno++;
        }
      }
    }
  }

  return false;
}

/**
  Assign set of available (prefix) tables to all tables in query block.
  Also set added tables, ie the tables added in each JOIN_TAB compared to the
  previous JOIN_TAB.
  This function must be called for every query block after the table order
  has been determined.
*/

void JOIN::set_prefix_tables() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);
  DBUG_ASSERT(!plan_is_const());
  /*
    The const tables are available together with the first non-const table in
    the join order.
  */
  table_map const initial_tables_map =
      const_table_map | (allow_outer_refs ? OUTER_REF_TABLE_BIT : 0);

  table_map current_tables_map = initial_tables_map;
  table_map prev_tables_map = (table_map)0;
  table_map saved_tables_map = (table_map)0;

  JOIN_TAB *last_non_sjm_tab = nullptr;  // Track the last non-sjm table

  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *const tab = best_ref[i];
    if (!tab->table()) continue;
    /*
      Tables that are within SJ-Materialization nests cannot have their
      conditions referring to preceding non-const tables.
       - If we're looking at the first SJM table, reset current_tables_map
         to refer to only allowed tables
      @see Item_equal::get_subst_item()
      @see eliminate_item_equal()
    */
    if (sj_is_materialize_strategy(tab->get_sj_strategy())) {
      const table_map sjm_inner_tables = tab->emb_sj_nest->sj_inner_tables;
      if (!(sjm_inner_tables & current_tables_map)) {
        saved_tables_map = current_tables_map;
        current_tables_map = initial_tables_map;
        prev_tables_map = (table_map)0;
      }

      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;

      if (!(sjm_inner_tables & ~current_tables_map)) {
        /*
          At the end of a semi-join materialization nest,
          add non-deterministic expressions to the last table of the nest:
        */
        tab->add_prefix_tables(RAND_TABLE_BIT);

        // Restore the previous map:
        current_tables_map = saved_tables_map;
        prev_tables_map =
            last_non_sjm_tab ? last_non_sjm_tab->prefix_tables() : (table_map)0;
      }
    } else {
      last_non_sjm_tab = tab;
      current_tables_map |= tab->table_ref->map();
      tab->set_prefix_tables(current_tables_map, prev_tables_map);
      prev_tables_map = current_tables_map;
    }
  }
  /*
    Non-deterministic expressions must be added to the last table's condition.
    It solves problem with queries like SELECT * FROM t1 WHERE rand() > 0.5
  */
  if (last_non_sjm_tab != nullptr)
    last_non_sjm_tab->add_prefix_tables(RAND_TABLE_BIT);
}

/**
  Calculate best possible join order and initialize the join structure.

  @return true if success, false if error.

  The JOIN object is populated with statistics about the query,
  and a plan with table order and access method selection is made.

  The list of tables to be optimized is taken from select_lex->leaf_tables.
  JOIN::where_cond is also used in the optimization.
  As a side-effect, JOIN::keyuse_array is populated with key_use information.

  Here is an overview of the logic of this function:

  - Initialize JOIN data structures and setup basic dependencies between tables.

  - Update dependencies based on join information.

  - Make key descriptions (update_ref_and_keys()).

  - Pull out semi-join tables based on table dependencies.

  - Extract tables with zero or one rows as const tables.

  - Read contents of const tables, substitute columns from these tables with
    actual data. Also keep track of empty tables vs. one-row tables.

  - After const table extraction based on row count, more tables may
    have become functionally dependent. Extract these as const tables.

  - Add new sargable predicates based on retrieved const values.

  - Calculate number of rows to be retrieved from each table.

  - Calculate cost of potential semi-join materializations.

  - Calculate best possible join order based on available statistics.

  - Fill in remaining information for the generated join order.
*/

bool JOIN::make_join_plan() {
  DBUG_TRACE;

  SARGABLE_PARAM *sargables = nullptr;

  Opt_trace_context *const trace = &thd->opt_trace;

  if (init_planner_arrays())  // Create and initialize the arrays
    return true;

  // Outer join dependencies were initialized above, now complete the analysis.
  if (select_lex->outer_join || select_lex->is_recursive()) {
    if (propagate_dependencies()) {
      /*
        Catch illegal join order.
        SQL2011 forbids:
        WITH RECURSIVE rec AS (
        ... UNION ALL SELECT ... FROM tbl LEFT JOIN rec ON...)c...
        MySQL also forbids the same query with STRAIGHT_JOIN instead of LEFT
        JOIN, because the algorithm of with-recursive imposes that "rec" be
        first in plan, i.e. "tbl" depends on "rec", but STRAIGHT_JOIN imposes
        the opposite dependency.
      */
      DBUG_ASSERT(select_lex->is_recursive());
      my_error(ER_CTE_RECURSIVE_FORBIDDEN_JOIN_ORDER, MYF(0),
               select_lex->recursive_reference->alias);
      return true;
    }
    init_key_dependencies();
  }

  if (unlikely(trace->is_started()))
    trace_table_dependencies(trace, join_tab, primary_tables);

  // Build the key access information, which is the basis for ref access.
  if (where_cond || select_lex->outer_join) {
    if (update_ref_and_keys(thd, &keyuse_array, join_tab, tables, where_cond,
                            ~select_lex->outer_join, select_lex, &sargables))
      return true;
  }

  /*
    Pull out semi-join tables based on dependencies. Dependencies are valid
    throughout the lifetime of a query, so this operation can be performed
    on the first optimization only.
  */
  if (!select_lex->sj_pullout_done && !select_lex->sj_nests.empty() &&
      pull_out_semijoin_tables(this))
    return true;

  select_lex->sj_pullout_done = true;
  const uint sj_nests = select_lex->sj_nests.size();  // Changed by pull-out

  if (!(select_lex->active_options() & OPTION_NO_CONST_TABLES)) {
    // Detect tables that are const (0 or 1 row) and read their contents.
    if (extract_const_tables()) return true;

    // Detect tables that are functionally dependent on const values.
    if (extract_func_dependent_tables()) return true;
  }
  // Possibly able to create more sargable predicates from const rows.
  if (const_tables && sargables) update_sargable_from_const(sargables);

  // Make a first estimate of the fanout for each table in the query block.
  if (estimate_rowcount()) return true;

  /*
    Apply join order hints, with the exception of
    JOIN_FIXED_ORDER and STRAIGHT_JOIN.
  */
  if (select_lex->opt_hints_qb &&
      !(select_lex->active_options() & SELECT_STRAIGHT_JOIN))
    select_lex->opt_hints_qb->apply_join_order_hints(this);

  if (sj_nests) {
    set_semijoin_embedding();
    select_lex->update_semijoin_strategies(thd);
  }

  if (!plan_is_const()) optimize_keyuse();

  allow_outer_refs = true;

  if (sj_nests && optimize_semijoin_nests_for_materialization(this))
    return true;

  // Choose the table order based on analysis done so far.
  if (Optimize_table_order(thd, this, nullptr).choose_table_order())
    return true;

  DBUG_EXECUTE_IF("bug13820776_1", thd->killed = THD::KILL_QUERY;);
  if (thd->killed || thd->is_error()) return true;

  // If this is a subquery, decide between In-to-exists and materialization
  if (unit->item && decide_subquery_strategy()) return true;

  refine_best_rowcount();

  if (!(thd->variables.option_bits & OPTION_BIG_SELECTS) &&
      best_read > (double)thd->variables.max_join_size &&
      !thd->lex->is_explain()) { /* purecov: inspected */
    my_error(ER_TOO_BIG_SELECT, MYF(0));
    error = -1;
    return true;
  }

  positions = nullptr;  // But keep best_positions for get_best_combination

  // Generate an execution plan from the found optimal join order.
  if (get_best_combination()) return true;

  // Cleanup after update_ref_and_keys has added keys for derived tables.
  if (select_lex->materialized_derived_table_count ||
      select_lex->table_func_count)
    finalize_derived_keys();

  // No need for this struct after new JOIN_TAB array is set up.
  best_positions = nullptr;

  // Some called function may still set error status unnoticed
  if (thd->is_error()) return true;

  // There is at least one empty const table
  if (const_table_map != found_const_table_map)
    zero_result_cause = "no matching row in const table";

  return false;
}

/**
  Initialize scratch arrays for the join order optimization

  @returns false if success, true if error

  @note If something fails during initialization, JOIN::cleanup()
        will free anything that has been partially allocated and set up.
        Arrays are created in the execution mem_root, so they will be
        deleted automatically when the mem_root is re-initialized.
*/

bool JOIN::init_planner_arrays() {
  // Up to one extra slot per semi-join nest is needed (if materialized)
  const uint sj_nests = select_lex->sj_nests.size();
  const uint table_count = select_lex->leaf_table_count;

  DBUG_ASSERT(primary_tables == 0 && tables == 0);

  if (!(join_tab = alloc_jtab_array(thd, table_count))) return true;

  /*
    We add 2 cells:
    - because planning stage uses 0-termination so needs +1
    - because after get_best_combination, we don't use 0-termination but
    need +2, to host at most 2 tmp sort/group/distinct tables.
  */
  if (!(best_ref = (JOIN_TAB **)thd->alloc(
            sizeof(JOIN_TAB *) *
            (table_count + sj_nests + 2 + m_windows.elements))))
    return true;

  // sort/group tmp tables have no map
  if (!(map2table = (JOIN_TAB **)thd->alloc(sizeof(JOIN_TAB *) *
                                            (table_count + sj_nests))))
    return true;

  if (!(positions = new (thd->mem_root) POSITION[table_count])) return true;

  if (!(best_positions = new (thd->mem_root) POSITION[table_count + sj_nests]))
    return true;

  /*
    Initialize data structures for tables to be joined.
    Initialize dependencies between tables.
  */
  JOIN_TAB **best_ref_p = best_ref;
  TABLE_LIST *tl = select_lex->leaf_tables;

  for (JOIN_TAB *tab = join_tab; tl; tab++, tl = tl->next_leaf, best_ref_p++) {
    *best_ref_p = tab;
    TABLE *const table = tl->table;
    tab->table_ref = tl;
    tab->set_table(table);
    const int err = tl->fetch_number_of_rows();

    // Initialize the cost model for the table
    table->init_cost_model(cost_model());

    DBUG_EXECUTE_IF("bug11747970_raise_error", {
      if (!err) {
        my_error(ER_UNKNOWN_ERROR, MYF(0));
        return true;
      }
    });

    if (err) {
      table->file->print_error(err, MYF(0));
      return true;
    }
    all_table_map |= tl->map();
    tab->set_join(this);

    tab->dependent = tl->dep_tables;  // Initialize table dependencies
    if (select_lex->is_recursive()) {
      if (select_lex->recursive_reference != tl)
        // Recursive reference must go first
        tab->dependent |= select_lex->recursive_reference->map();
      else {
        // Recursive reference mustn't use any index
        table->covering_keys.clear_all();
        table->keys_in_use_for_group_by.clear_all();
        table->keys_in_use_for_order_by.clear_all();
      }
    }
    if (tl->schema_table) table->file->stats.records = 2;
    table->quick_condition_rows = table->file->stats.records;

    tab->init_join_cond_ref(tl);

    if (tl->outer_join_nest()) {
      // tab belongs to a nested join, maybe to several embedding joins
      tab->embedding_map = 0;
      for (TABLE_LIST *embedding = tl->embedding; embedding;
           embedding = embedding->embedding) {
        NESTED_JOIN *const nested_join = embedding->nested_join;
        tab->embedding_map |= nested_join->nj_map;
        tab->dependent |= embedding->dep_tables;
      }
    } else if (tab->join_cond()) {
      // tab is the only inner table of an outer join
      tab->embedding_map = 0;
      for (TABLE_LIST *embedding = tl->embedding; embedding;
           embedding = embedding->embedding)
        tab->embedding_map |= embedding->nested_join->nj_map;
    }

    if (tl->is_derived() && tl->derived_unit()->m_lateral_deps)
      has_lateral = true;

    tables++;  // Count number of initialized tables
  }

  primary_tables = tables;
  *best_ref_p = nullptr;  // Last element of array must be NULL

  return false;
}

/**
  Propagate dependencies between tables due to outer join relations.

  @returns false if success, true if error

  Build transitive closure for relation 'to be dependent on'.
  This will speed up the plan search for many cases with outer joins,
  as well as allow us to catch illegal cross references.
  Warshall's algorithm is used to build the transitive closure.
  As we may restart the outer loop upto 'table_count' times, the
  complexity of the algorithm is O((number of tables)^3).
  However, most of the iterations will be shortcircuited when
  there are no dependencies to propagate.
*/

bool JOIN::propagate_dependencies() {
  for (uint i = 0; i < tables; i++) {
    if (!join_tab[i].dependent) continue;

    // Add my dependencies to other tables depending on me
    uint j;
    JOIN_TAB *tab;
    for (j = 0, tab = join_tab; j < tables; j++, tab++) {
      if (tab->dependent & join_tab[i].table_ref->map()) {
        const table_map was_dependent = tab->dependent;
        tab->dependent |= join_tab[i].dependent;
        /*
          If we change dependencies for a table we already have
          processed: Redo dependency propagation from this table.
        */
        if (i > j && tab->dependent != was_dependent) {
          i = j - 1;
          break;
        }
      }
    }
  }

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    if ((tab->dependent & tab->table_ref->map())) return true;
  }

  return false;
}

/**
  Extract const tables based on row counts.

  @returns false if success, true if error

  This extraction must be done for each execution.
  Tables containing exactly zero or one rows are marked as const, but
  notice the additional constraints checked below.
  Tables that are extracted have their rows read before actual execution
  starts and are placed in the beginning of the join_tab array.
  Thus, they do not take part in join order optimization process,
  which can significantly reduce the optimization time.
  The data read from these tables can also be regarded as "constant"
  throughout query execution, hence the column values can be used for
  additional constant propagation and extraction of const tables based
  on eq-ref properties.

  The tables are given the type JT_SYSTEM.
*/

bool JOIN::extract_const_tables() {
  enum enum_const_table_extraction {
    extract_no_table = 0,
    extract_empty_table = 1,
    extract_const_table = 2
  };

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    TABLE *const table = tab->table();
    TABLE_LIST *const tl = tab->table_ref;
    enum enum_const_table_extraction extract_method = extract_const_table;

    const bool all_partitions_pruned_away = table->all_partitions_pruned_away;

    if (tl->outer_join_nest()) {
      /*
        Table belongs to a nested join, no candidate for const table extraction.
      */
      extract_method = extract_no_table;
    } else if (tl->embedding && tl->embedding->is_sj_or_aj_nest()) {
      /*
        Table belongs to a semi-join.
        We do not currently pull out const tables from semi-join nests.
      */
      extract_method = extract_no_table;
    } else if (tab->join_cond()) {
      // tab is the only inner table of an outer join, extract empty tables
      extract_method = extract_empty_table;
    }
    switch (extract_method) {
      case extract_no_table:
        break;

      case extract_empty_table:
        // Extract tables with zero rows, but only if statistics are exact
        if ((table->file->stats.records == 0 || all_partitions_pruned_away) &&
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT))
          mark_const_table(tab, nullptr);
        break;

      case extract_const_table:
        /*
          Extract tables with zero or one rows, but do not extract tables that
           1. are dependent upon other tables, or
           2. have no exact statistics, or
           3. are full-text searched
        */
        if ((table->s->system || table->file->stats.records <= 1 ||
             all_partitions_pruned_away) &&
            !tab->dependent &&                                              // 1
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT) &&  // 2
            !table->fulltext_searched)                                      // 3
          mark_const_table(tab, nullptr);
        break;
    }
  }

  // Read const tables (tables matching no more than 1 rows)
  if (!const_tables) return false;

  for (POSITION *p_pos = positions, *p_end = p_pos + const_tables;
       p_pos < p_end; p_pos++) {
    JOIN_TAB *const tab = p_pos->table;
    const int status = join_read_const_table(tab, p_pos);
    if (status > 0)
      return true;
    else if (status == 0) {
      found_const_table_map |= tab->table_ref->map();
      tab->table_ref->optimized_away = true;
    }
  }

  return false;
}

/**
  Extract const tables based on functional dependencies.

  @returns false if success, true if error

  This extraction must be done for each execution.

  Mark as const the tables that
   - are functionally dependent on constant values, or
   - are inner tables of an outer join and contain exactly zero or one rows

  Tables that are extracted have their rows read before actual execution
  starts and are placed in the beginning of the join_tab array, just as
  described for JOIN::extract_const_tables().

  The tables are given the type JT_CONST.
*/

bool JOIN::extract_func_dependent_tables() {
  // loop until no more const tables are found
  bool ref_changed;
  // Tables referenced by others; if they're const the others may be too.
  table_map found_ref;
  do {
  more_const_tables_found:
    ref_changed = false;
    found_ref = 0;

    // Loop over all tables that are not already determined to be const
    for (JOIN_TAB **pos = best_ref + const_tables; *pos; pos++) {
      JOIN_TAB *const tab = *pos;
      TABLE *const table = tab->table();
      TABLE_LIST *const tl = tab->table_ref;
      /*
        If equi-join condition by a key is null rejecting and after a
        substitution of a const table the key value happens to be null
        then we can state that there are no matches for this equi-join.
      */
      Key_use *keyuse = tab->keyuse();
      if (keyuse && tab->join_cond() && !tab->embedding_map) {
        /*
          When performing an outer join operation if there are no matching rows
          for the single row of the outer table all the inner tables are to be
          null complemented and thus considered as constant tables.
          Here we apply this consideration to the case of outer join operations
          with a single inner table only because the case with nested tables
          would require a more thorough analysis.
          TODO. Apply single row substitution to null complemented inner tables
          for nested outer join operations.
        */
        while (keyuse->table_ref == tl) {
          if (!(keyuse->val->used_tables() & ~const_table_map) &&
              keyuse->val->is_null() && keyuse->null_rejecting) {
            table->set_null_row();
            table->const_table = true;
            found_const_table_map |= tl->map();
            mark_const_table(tab, keyuse);
            goto more_const_tables_found;
          }
          keyuse++;
        }
      }

      if (tab->dependent)  // If dependent on some table
      {
        // All dependent tables must be const
        if (tab->dependent & ~const_table_map) {
          found_ref |= tab->dependent;
          continue;
        }
        /*
          Mark a dependent table as constant if
           1. it has exactly zero or one rows (it is a system table), and
           2. it is not within a nested outer join, and
           3. it does not have an expensive outer join condition.
              This is because we have to determine whether an outer-joined table
              has a real row or a null-extended row in the optimizer phase.
              We have no possibility to evaluate its join condition at
              execution time, when it is marked as a system table.
        */
        if (table->file->stats.records <= 1L &&                             // 1
            (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT) &&  // 1
            !tl->outer_join_nest() &&                                       // 2
            !(tab->join_cond() && tab->join_cond()->is_expensive()))        // 3
        {  // system table
          mark_const_table(tab, nullptr);
          const int status =
              join_read_const_table(tab, positions + const_tables - 1);
          if (status > 0)
            return true;
          else if (status == 0)
            found_const_table_map |= tl->map();
          continue;
        }
      }

      // Check if table can be read by key or table only uses const refs

      if ((keyuse = tab->keyuse())) {
        while (keyuse->table_ref == tl) {
          Key_use *const start_keyuse = keyuse;
          const uint key = keyuse->key;
          tab->keys().set_bit(key);  // QQ: remove this ?

          table_map refs = 0;
          Key_map const_ref, eq_part;
          do {
            if (keyuse->val->type() != Item::NULL_ITEM && !keyuse->optimize) {
              if (!((~found_const_table_map) & keyuse->used_tables))
                const_ref.set_bit(keyuse->keypart);
              else
                refs |= keyuse->used_tables;
              eq_part.set_bit(keyuse->keypart);
            }
            keyuse++;
          } while (keyuse->table_ref == tl && keyuse->key == key);

          /*
            Extract const tables with proper key dependencies.
            Exclude tables that
             1. are full-text searched, or
             2. are part of nested outer join, or
             3. are part of semi-join, or
             4. have an expensive outer join condition.
             5. are blocked by handler for const table optimize.
             6. are not going to be used, typically because they are streamed
                instead of materialized
                (see SELECT_LEX_UNIT::can_materialize_directly_into_result()).
          */
          if (eq_part.is_prefix(table->key_info[key].user_defined_key_parts) &&
              !table->fulltext_searched &&                                // 1
              !tl->outer_join_nest() &&                                   // 2
              !(tl->embedding && tl->embedding->is_sj_or_aj_nest()) &&    // 3
              !(tab->join_cond() && tab->join_cond()->is_expensive()) &&  // 4
              !(table->file->ha_table_flags() & HA_BLOCK_CONST_TABLE) &&  // 5
              table->is_created()) {                                      // 6
            if (table->key_info[key].flags & HA_NOSAME) {
              if (const_ref == eq_part) {  // Found everything for ref.
                ref_changed = true;
                mark_const_table(tab, start_keyuse);
                if (create_ref_for_key(this, tab, start_keyuse,
                                       found_const_table_map))
                  return true;
                const int status =
                    join_read_const_table(tab, positions + const_tables - 1);
                if (status > 0)
                  return true;
                else if (status == 0)
                  found_const_table_map |= tl->map();
                break;
              } else
                found_ref |= refs;  // Table is const if all refs are const
            } else if (const_ref == eq_part)
              tab->const_keys.set_bit(key);
          }
        }
      }
    }
  } while
      /*
        A new const table appeared, that is referenced by others, so re-check
        others:
      */
      ((const_table_map & found_ref) && ref_changed);

  return false;
}

/**
  Update info on indexes that can be used for search lookups as
  reading const tables may has added new sargable predicates.
*/

void JOIN::update_sargable_from_const(SARGABLE_PARAM *sargables) {
  for (; sargables->field; sargables++) {
    Field *const field = sargables->field;
    JOIN_TAB *const tab = field->table->reginfo.join_tab;
    Key_map possible_keys = field->key_start;
    possible_keys.intersect(field->table->keys_in_use_for_query);
    bool is_const = true;
    for (uint j = 0; j < sargables->num_values; j++)
      is_const &= sargables->arg_value[j]->const_item();
    if (is_const) {
      tab->const_keys.merge(possible_keys);
      tab->keys().merge(possible_keys);
    }
  }
}

/**
  Estimate the number of matched rows for each joined table.
  Set up range scan for tables that have proper predicates.

  @returns false if success, true if error
*/

bool JOIN::estimate_rowcount() {
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_records(trace, "rows_estimation");

  JOIN_TAB *const tab_end = join_tab + tables;
  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    const Cost_model_table *const cost_model = tab->table()->cost_model();
    Opt_trace_object trace_table(trace);
    trace_table.add_utf8_table(tab->table_ref);
    if (tab->type() == JT_SYSTEM || tab->type() == JT_CONST) {
      trace_table.add("rows", 1)
          .add("cost", 1)
          .add_alnum("table_type",
                     (tab->type() == JT_SYSTEM) ? "system" : "const")
          .add("empty", tab->table()->has_null_row());

      // Only one matching row and one block to read
      tab->set_records(tab->found_records = 1);
      tab->worst_seeks = cost_model->page_read_cost(1.0);
      tab->read_time = tab->worst_seeks;
      continue;
    }
    // Approximate number of found rows and cost to read them
    tab->set_records(tab->found_records = tab->table()->file->stats.records);
    const Cost_estimate table_scan_time = tab->table()->file->table_scan_cost();
    tab->read_time = table_scan_time.total_cost();

    /*
      Set a max value for the cost of seek operations we can expect
      when using key lookup. This can't be too high as otherwise we
      are likely to use table scan.
    */
    tab->worst_seeks =
        min(cost_model->page_read_cost((double)tab->found_records / 10),
            tab->read_time * 3);
    const double min_worst_seek = cost_model->page_read_cost(2.0);
    if (tab->worst_seeks < min_worst_seek)  // Fix for small tables
      tab->worst_seeks = min_worst_seek;

    /*
      Add to tab->const_keys the indexes for which all group fields or
      all select distinct fields participate in one index.
      Add to tab->skip_scan_keys indexes which can be used for skip
      scan access if no aggregates are present.
    */
    add_loose_index_scan_and_skip_scan_keys(this, tab);

    /*
      Perform range analysis if there are keys it could use (1).
      Don't do range analysis if on the inner side of an outer join (2).
      Do range analysis if on the inner side of a semi-join (3).
    */
    TABLE_LIST *const tl = tab->table_ref;
    if ((!tab->const_keys.is_clear_all() ||
         !tab->skip_scan_keys.is_clear_all()) &&                 // (1)
        (!tl->embedding ||                                       // (2)
         (tl->embedding && tl->embedding->is_sj_or_aj_nest())))  // (3)
    {
      /*
        This call fills tab->quick() with the best QUICK access method
        possible for this table, and only if it's better than table scan.
        It also fills tab->needed_reg.
      */
      ha_rows records = get_quick_record_count(thd, tab, row_limit);

      if (records == 0 && thd->is_error()) return true;

      /*
        Check for "impossible range", but make sure that we do not attempt
        to mark semi-joined tables as "const" (only semi-joined tables that
        are functionally dependent can be marked "const", and subsequently
        pulled out of their semi-join nests).
      */
      if (records == 0 && tab->table()->reginfo.impossible_range &&
          (!(tl->embedding && tl->embedding->is_sj_or_aj_nest()))) {
        /*
          Impossible WHERE condition or join condition
          In case of join cond, mark that one empty NULL row is matched.
          In case of WHERE, don't set found_const_table_map to get the
          caller to abort with a zero row result.
        */
        mark_const_table(tab, nullptr);
        tab->set_type(JT_CONST);  // Override setting made in mark_const_table()
        if (tab->join_cond()) {
          // Generate an empty row
          trace_table.add("returning_empty_null_row", true)
              .add_alnum("cause", "impossible_on_condition");
          found_const_table_map |= tl->map();
          tab->table()->set_null_row();  // All fields are NULL
        } else {
          trace_table.add("rows", 0).add_alnum("cause",
                                               "impossible_where_condition");
        }
      }
      if (records != HA_POS_ERROR) {
        tab->found_records = records;
        tab->read_time =
            tab->quick() ? tab->quick()->cost_est.total_cost() : 0.0;
      }
    } else {
      Opt_trace_object(trace, "table_scan")
          .add("rows", tab->found_records)
          .add("cost", tab->read_time);
    }
  }

  return false;
}

/**
  Set semi-join embedding join nest pointers.

  Set pointer to embedding semi-join nest for all semi-joined tables.
  This is the closest semi-join or anti-join nest.
  Note that this must be done for every table inside all semi-join nests,
  even for tables within outer join nests embedded in semi-join nests.
  A table can never be part of multiple semi-join nests, hence no
  ambiguities can ever occur.
  Note also that the pointer is not set for TABLE_LIST objects that
  are outer join nests within semi-join nests.
*/

void JOIN::set_semijoin_embedding() {
  DBUG_ASSERT(!select_lex->sj_nests.empty());

  JOIN_TAB *const tab_end = join_tab + primary_tables;

  for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++) {
    tab->emb_sj_nest = nullptr;
    for (TABLE_LIST *tl = tab->table_ref; tl->embedding; tl = tl->embedding) {
      if (tl->embedding->is_sj_or_aj_nest()) {
        DBUG_ASSERT(!tab->emb_sj_nest);
        tab->emb_sj_nest = tl->embedding;
        // Let the up-walk continue, to assert there's no AJ/SJ nest above.
      }
    }
  }
}

/**
  @brief Check if semijoin's compared types allow materialization.

  @param[in,out] sj_nest Semi-join nest containing information about correlated
         expressions. Set nested_join->sjm.scan_allowed to true if
         MaterializeScan strategy allowed. Set nested_join->sjm.lookup_allowed
         to true if MaterializeLookup strategy allowed

  @details
    This is a temporary fix for BUG#36752.

    There are two subquery materialization strategies for semijoin:

    1. Materialize and do index lookups in the materialized table. See
       BUG#36752 for description of restrictions we need to put on the
       compared expressions.

       In addition, since indexes are not supported for BLOB columns,
       this strategy can not be used if any of the columns in the
       materialized table will be BLOB/GEOMETRY columns.  (Note that
       also columns for non-BLOB values that may be greater in size
       than CONVERT_IF_BIGGER_TO_BLOB, will be represented as BLOB
       columns.)

    2. Materialize and then do a full scan of the materialized table.
       The same criteria as for MaterializeLookup are applied, except that
       BLOB/GEOMETRY columns are allowed.
*/

static void semijoin_types_allow_materialization(TABLE_LIST *sj_nest) {
  DBUG_TRACE;

  DBUG_ASSERT(sj_nest->nested_join->sj_outer_exprs.elements ==
              sj_nest->nested_join->sj_inner_exprs.elements);

  if (sj_nest->nested_join->sj_outer_exprs.elements > MAX_REF_PARTS ||
      sj_nest->nested_join->sj_outer_exprs.elements == 0) {
    // building an index is impossible
    sj_nest->nested_join->sjm.scan_allowed = false;
    sj_nest->nested_join->sjm.lookup_allowed = false;
    return;
  }

  List_iterator<Item> it1(sj_nest->nested_join->sj_outer_exprs);
  List_iterator<Item> it2(sj_nest->nested_join->sj_inner_exprs);

  sj_nest->nested_join->sjm.scan_allowed = true;
  sj_nest->nested_join->sjm.lookup_allowed = true;

  bool blobs_involved = false;
  Item *outer, *inner;
  uint total_lookup_index_length = 0;
  uint max_key_length, max_key_part_length, max_key_parts;
  /*
    Maximum lengths for keys and key parts that are supported by
    the temporary table storage engine(s).
  */
  get_max_key_and_part_length(&max_key_length, &max_key_part_length,
                              &max_key_parts);
  while (outer = it1++, inner = it2++) {
    DBUG_ASSERT(outer->real_item() && inner->real_item());
    if (!types_allow_materialization(outer, inner)) {
      sj_nest->nested_join->sjm.scan_allowed = false;
      sj_nest->nested_join->sjm.lookup_allowed = false;
      return;
    }
    blobs_involved |= inner->is_blob_field();

    // Calculate the index length of materialized table
    const uint lookup_index_length = get_key_length_tmp_table(inner);
    if (lookup_index_length > max_key_part_length)
      sj_nest->nested_join->sjm.lookup_allowed = false;
    total_lookup_index_length += lookup_index_length;
  }
  if (total_lookup_index_length > max_key_length)
    sj_nest->nested_join->sjm.lookup_allowed = false;

  if (blobs_involved) sj_nest->nested_join->sjm.lookup_allowed = false;

  DBUG_PRINT("info", ("semijoin_types_allow_materialization: ok, allowed"));
}

/**
  Index dive can be skipped if the following conditions are satisfied:
  F1) For a single table query:
     a) FORCE INDEX applies to a single index.
     b) No subquery is present.
     c) Fulltext Index is not involved.
     d) No GROUP-BY or DISTINCT clause.
     e) No ORDER-BY clause.

  F2) Not applicable to multi-table query.

  F3) This optimization is not applicable to EXPLAIN queries.

  @param tab   JOIN_TAB object.
  @param thd   THD object.
*/
static bool check_skip_records_in_range_qualification(JOIN_TAB *tab, THD *thd) {
  SELECT_LEX *select = thd->lex->current_select();
  TABLE *table = tab->table();
  return ((table->force_index &&
           table->keys_in_use_for_query.bits_set() == 1) &&     // F1.a
          select->parent_lex->is_single_level_stmt() &&         // F1.b
          !select->has_ft_funcs() &&                            // F1.c
          (!select->is_grouped() && !select->is_distinct()) &&  // F1.d
          !select->is_ordered() &&                              // F1.e
          select->join_list->size() == 1 &&                     // F2
          !thd->lex->is_explain());                             // F3
}

/*****************************************************************************
  Create JOIN_TABS, make a guess about the table types,
  Approximate how many records will be used in each table
*****************************************************************************/

/**
  Returns estimated number of rows that could be fetched by given
  access method.

  The function calls the range optimizer to estimate the cost of the
  cheapest QUICK_* index access method to scan one or several of the
  'keys' using the conditions 'select->cond'. The range optimizer
  compares several different types of 'quick select' methods (range
  scan, index merge, loose index scan) and selects the cheapest one.

  If the best index access method is cheaper than a table- and an index
  scan, then the range optimizer also constructs the corresponding
  QUICK_* object and assigns it to select->quick. In most cases this
  is the QUICK_* object used at later (optimization and execution)
  phases.

  @param thd    Session that runs the query.
  @param tab    JOIN_TAB of source table.
  @param limit  maximum number of rows to select.

  @note
    In case of valid range, a QUICK_SELECT_I object will be constructed and
    saved in select->quick.

  @return Estimated number of result rows selected from 'tab'.

  @retval HA_POS_ERROR For derived tables/views or if an error occur.
  @retval 0            If impossible query (i.e. certainly no rows will be
                       selected.)
*/
static ha_rows get_quick_record_count(THD *thd, JOIN_TAB *tab, ha_rows limit) {
  DBUG_TRACE;
  uchar buff[STACK_BUFF_ALLOC];
  if (check_stack_overrun(thd, STACK_MIN_SIZE, buff))
    return 0;  // Fatal error flag is set
  TABLE_LIST *const tl = tab->table_ref;
  tab->set_skip_records_in_range(
      check_skip_records_in_range_qualification(tab, thd));

  // Derived tables aren't filled yet, so no stats are available.
  if (!tl->uses_materialization()) {
    QUICK_SELECT_I *qck;
    Key_map keys_to_use = tab->const_keys;
    keys_to_use.merge(tab->skip_scan_keys);
    int error = test_quick_select(
        thd, keys_to_use,
        0,  // empty table_map
        limit,
        false,  // don't force quick range
        ORDER_NOT_RELEVANT, tab,
        tab->join_cond() ? tab->join_cond() : tab->join()->where_cond,
        &tab->needed_reg, &qck, tab->table()->force_index);
    tab->set_quick(qck);

    if (error == 1) return qck->records;
    if (error == -1) {
      tl->table->reginfo.impossible_range = true;
      return 0;
    }
    DBUG_PRINT("warning", ("Couldn't use record count on const keypart"));
  } else if (tl->is_table_function() || tl->materializable_is_const()) {
    tl->fetch_number_of_rows();
    return tl->table->file->stats.records;
  }
  return HA_POS_ERROR;
}

/*
  Get estimated record length for semi-join materialization temptable

  SYNOPSIS
    get_tmp_table_rec_length()
      items  IN subquery's select list.

  DESCRIPTION
    Calculate estimated record length for semi-join materialization
    temptable. It's an estimate because we don't follow every bit of
    create_tmp_table()'s logic. This isn't necessary as the return value of
    this function is used only for cost calculations.

  RETURN
    Length of the temptable record, in bytes
*/

static uint get_tmp_table_rec_length(List<Item> &items) {
  uint len = 0;
  Item *item;
  List_iterator<Item> it(items);
  while ((item = it++)) {
    switch (item->result_type()) {
      case REAL_RESULT:
        len += sizeof(double);
        break;
      case INT_RESULT:
        if (item->max_length >= (MY_INT32_NUM_DECIMAL_DIGITS - 1))
          len += 8;
        else
          len += 4;
        break;
      case STRING_RESULT:
        /* DATE/TIME and GEOMETRY fields have STRING_RESULT result type.  */
        if (item->is_temporal() || item->data_type() == MYSQL_TYPE_GEOMETRY)
          len += 8;
        else
          len += item->max_length;
        break;
      case DECIMAL_RESULT:
        len += 10;
        break;
      case ROW_RESULT:
      default:
        DBUG_ASSERT(0); /* purecov: deadcode */
        break;
    }
  }
  return len;
}

/**
   Writes to the optimizer trace information about dependencies between
   tables.
   @param trace  optimizer trace
   @param join_tabs  all JOIN_TABs of the join
   @param table_count how many JOIN_TABs in the 'join_tabs' array
*/
static void trace_table_dependencies(Opt_trace_context *trace,
                                     JOIN_TAB *join_tabs, uint table_count) {
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_dep(trace, "table_dependencies");
  for (uint i = 0; i < table_count; i++) {
    TABLE_LIST *table_ref = join_tabs[i].table_ref;
    Opt_trace_object trace_one_table(trace);
    trace_one_table.add_utf8_table(table_ref).add(
        "row_may_be_null", table_ref->table->is_nullable());
    const table_map map = table_ref->map();
    DBUG_ASSERT(map < (1ULL << table_count));
    for (uint j = 0; j < table_count; j++) {
      if (map & (1ULL << j)) {
        trace_one_table.add("map_bit", j);
        break;
      }
    }
    Opt_trace_array depends_on(trace, "depends_on_map_bits");
    static_assert(sizeof(table_ref->map()) <= 64,
                  "RAND_TABLE_BIT may be in join_tabs[i].dependent, so we test "
                  "all 64 bits.");
    for (uint j = 0; j < 64; j++) {
      if (join_tabs[i].dependent & (1ULL << j)) depends_on.add(j);
    }
  }
}

/**
  Add to join_tab[i]->condition() "table.field IS NOT NULL" conditions
  we've inferred from ref/eq_ref access performed.

    This function is a part of "Early NULL-values filtering for ref access"
    optimization.

    Example of this optimization:
    For query SELECT * FROM t1,t2 WHERE t2.key=t1.field @n
    and plan " any-access(t1), ref(t2.key=t1.field) " @n
    add "t1.field IS NOT NULL" to t1's table condition. @n

    Description of the optimization:

      We look through equalities choosen to perform ref/eq_ref access,
      pick equalities that have form "tbl.part_of_key = othertbl.field"
      (where othertbl is a non-const table and othertbl.field may be NULL)
      and add them to conditions on correspoding tables (othertbl in this
      example).

      Exception from that is the case when referred_tab->join != join.
      I.e. don't add NOT NULL constraints from any embedded subquery.
      Consider this query:
      @code
      SELECT A.f2 FROM t1 LEFT JOIN t2 A ON A.f2 = f1
      WHERE A.f3=(SELECT MIN(f3) FROM  t2 C WHERE A.f4 = C.f4) OR A.f3 IS NULL;
      @endcode
      Here condition A.f3 IS NOT NULL is going to be added to the WHERE
      condition of the embedding query.
      Another example:
      SELECT * FROM t10, t11 WHERE (t10.a < 10 OR t10.a IS NULL)
      AND t11.b <=> t10.b AND (t11.a = (SELECT MAX(a) FROM t12
      WHERE t12.b = t10.a ));
      Here condition t10.a IS NOT NULL is going to be added.
      In both cases addition of NOT NULL condition will erroneously reject
      some rows of the result set.
      referred_tab->join != join constraint would disallow such additions.

      This optimization doesn't affect the choices that ref, range, or join
      optimizer make. This was intentional because this was added after 4.1
      was GA.

    Implementation overview
      1. update_ref_and_keys() accumulates info about null-rejecting
         predicates in in Key_field::null_rejecting
      1.1 add_key_part saves these to Key_use.
      2. create_ref_for_key copies them to TABLE_REF.
      3. add_not_null_conds adds "x IS NOT NULL" to join_tab->m_condition of
         appropiate JOIN_TAB members.
*/

static void add_not_null_conds(JOIN *join) {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  for (uint i = join->const_tables; i < join->tables; i++) {
    JOIN_TAB *const tab = join->best_ref[i];
    if ((tab->type() == JT_REF || tab->type() == JT_EQ_REF ||
         tab->type() == JT_REF_OR_NULL) &&
        !tab->table()->is_nullable()) {
      for (uint keypart = 0; keypart < tab->ref().key_parts; keypart++) {
        if (tab->ref().null_rejecting & ((key_part_map)1 << keypart)) {
          Item *item = tab->ref().items[keypart];
          Item *notnull;
          Item *real = item->real_item();
          DBUG_ASSERT(real->type() == Item::FIELD_ITEM);
          Item_field *not_null_item = (Item_field *)real;
          JOIN_TAB *referred_tab =
              not_null_item->field->table->reginfo.join_tab;
          /*
            For UPDATE queries such as:
            UPDATE t1 SET t1.f2=(SELECT MAX(t2.f4) FROM t2 WHERE t2.f3=t1.f1);
            not_null_item is the t1.f1, but it's referred_tab is 0.
          */
          if (!referred_tab || referred_tab->join() != join) continue;
          /* Skip if we already have a 'not null' predicate for 'item' */
          if (has_not_null_predicate(referred_tab->condition(), not_null_item))
            continue;
          if (!(notnull = new Item_func_isnotnull(not_null_item))) return;
          /*
            We need to do full fix_fields() call here in order to have correct
            notnull->const_item(). This is needed e.g. by test_quick_select
            when it is called from make_join_select after this function is
            called.
          */
          if (notnull->fix_fields(join->thd, &notnull)) return;
          DBUG_EXECUTE("where",
                       print_where(join->thd, notnull,
                                   referred_tab->table()->alias, QT_ORDINARY););
          referred_tab->and_with_condition(notnull);
        }
      }
    }
  }
}

/**
  Check all existing AND'ed predicates in 'cond' for an existing
  'is not null 'not_null_item''-predicate.

  A condition consisting of multiple AND'ed terms is recursively
  decomposed in the search for the specified not null predicate.

  @param  cond           Condition to be checked.
  @param  not_null_item  The item in: 'is not null 'item'' to search for

  @return true if 'is not null 'not_null_item'' is a predicate
          in the specified 'cond'.
*/
static bool has_not_null_predicate(Item *cond, Item_field *not_null_item) {
  if (cond == nullptr) return false;
  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *item_func = down_cast<Item_func *>(cond);
    const Item_func::Functype func_type = item_func->functype();
    return (func_type == Item_func::ISNOTNULL_FUNC &&
            item_func->key_item() == not_null_item);
  } else if (cond->type() == Item::COND_ITEM) {
    Item_cond *item_cond = down_cast<Item_cond *>(cond);
    if (item_cond->functype() == Item_func::COND_AND_FUNC) {
      List_iterator<Item> li(*item_cond->argument_list());
      Item *item;
      while ((item = li++)) {
        if (has_not_null_predicate(item, not_null_item)) return true;
      }
    }
  }
  return false;
}

/**
  Check if given expression only uses fields covered by index @a keyno in the
  table tbl. The expression can use any fields in any other tables.

  The expression is guaranteed not to be AND or OR - those constructs are
  handled outside of this function.

  Restrict some function types from being pushed down to storage engine:
  a) Don't push down the triggered conditions. Nested outer joins execution
     code may need to evaluate a condition several times (both triggered and
     untriggered).
     TODO: Consider cloning the triggered condition and using the copies for:
        1. push the first copy down, to have most restrictive index condition
           possible.
        2. Put the second copy into tab->m_condition.
  b) Stored functions contain a statement that might start new operations (like
     DML statements) from within the storage engine. This does not work against
     all SEs.
  c) Subqueries might contain nested subqueries and involve more tables.
     TODO: ROY: CHECK THIS
  d) Do not push down internal functions of type DD_INTERNAL_FUNC. When ICP is
     enabled, pushing internal functions to storage engine for evaluation will
     open data-dictionary tables. In InnoDB storage engine this will result in
     situation like recursive latching of same page by the same thread. To avoid
     such situation, internal functions of type DD_INTERNAL_FUNC are not pushed
  to storage engine for evaluation.

  @param  item           Expression to check
  @param  tbl            The table having the index
  @param  keyno          The index number
  @param  other_tbls_ok  true <=> Fields of other non-const tables are allowed

  @return false if No, true if Yes
*/

bool uses_index_fields_only(Item *item, TABLE *tbl, uint keyno,
                            bool other_tbls_ok) {
  // Restrictions b and c.
  if (item->has_stored_program() || item->has_subquery()) return false;

  // No table fields in const items
  if (item->const_item()) return true;

  const Item::Type item_type = item->type();

  switch (item_type) {
    case Item::FUNC_ITEM: {
      Item_func *item_func = (Item_func *)item;
      const Item_func::Functype func_type = item_func->functype();

      if (func_type == Item_func::TRIG_COND_FUNC ||  // Restriction a.
          func_type == Item_func::DD_INTERNAL_FUNC)  // Restriction d.
        return false;

      /* This is a function, apply condition recursively to arguments */
      if (item_func->argument_count() > 0) {
        Item **item_end =
            (item_func->arguments()) + item_func->argument_count();
        for (Item **child = item_func->arguments(); child != item_end;
             child++) {
          if (!uses_index_fields_only(*child, tbl, keyno, other_tbls_ok))
            return false;
        }
      }
      return true;
    }
    case Item::COND_ITEM: {
      /*
        This is a AND/OR condition. Regular AND/OR clauses are handled by
        make_cond_for_index() which will chop off the part that can be
        checked with index. This code is for handling non-top-level AND/ORs,
        e.g. func(x AND y).
      */
      List_iterator<Item> li(*((Item_cond *)item)->argument_list());
      Item *cond_item;
      while ((cond_item = li++)) {
        if (!uses_index_fields_only(cond_item, tbl, keyno, other_tbls_ok))
          return false;
      }
      return true;
    }
    case Item::FIELD_ITEM: {
      const Item_field *item_field = down_cast<const Item_field *>(item);
      if (item_field->field->table != tbl) return other_tbls_ok;
      /*
        The below is probably a repetition - the first part checks the
        other two, but let's play it safe:
      */
      return item_field->field->part_of_key.is_set(keyno) &&
             item_field->field->type() != MYSQL_TYPE_GEOMETRY &&
             item_field->field->type() != MYSQL_TYPE_BLOB;
    }
    case Item::REF_ITEM:
      return uses_index_fields_only(item->real_item(), tbl, keyno,
                                    other_tbls_ok);
    default:
      return false; /* Play it safe, don't push unknown non-const items */
  }
}

/**
  Optimize semi-join nests that could be run with sj-materialization

  @param join           The join to optimize semi-join nests for

  @details
    Optimize each of the semi-join nests that can be run with
    materialization. For each of the nests, we
     - Generate the best join order for this "sub-join" and remember it;
     - Remember the sub-join execution cost (it's part of materialization
       cost);
     - Calculate other costs that will be incurred if we decide
       to use materialization strategy for this semi-join nest.

    All obtained information is saved and will be used by the main join
    optimization pass.

  @return false if successful, true if error
*/

static bool optimize_semijoin_nests_for_materialization(JOIN *join) {
  DBUG_TRACE;
  Opt_trace_context *const trace = &join->thd->opt_trace;

  for (TABLE_LIST *sj_nest : join->select_lex->sj_nests) {
    /* As a precaution, reset pointers that were used in prior execution */
    sj_nest->nested_join->sjm.positions = nullptr;

    /* Calculate the cost of materialization if materialization is allowed. */
    if (sj_nest->nested_join->sj_enabled_strategies &
        OPTIMIZER_SWITCH_MATERIALIZATION) {
      /* A semi-join nest should not contain tables marked as const */
      DBUG_ASSERT(!(sj_nest->sj_inner_tables & join->const_table_map));

      Opt_trace_object trace_wrapper(trace);
      Opt_trace_object trace_sjmat(
          trace, "execution_plan_for_potential_materialization");
      Opt_trace_array trace_sjmat_steps(trace, "steps");
      /*
        Try semijoin materialization if the semijoin is classified as
        non-trivially-correlated.
      */
      if (sj_nest->nested_join->sj_corr_tables) continue;
      /*
        Check whether data types allow execution with materialization.
      */
      semijoin_types_allow_materialization(sj_nest);

      if (!sj_nest->nested_join->sjm.scan_allowed &&
          !sj_nest->nested_join->sjm.lookup_allowed)
        continue;

      if (Optimize_table_order(join->thd, join, sj_nest).choose_table_order())
        return true;
      const uint n_tables = my_count_bits(sj_nest->sj_inner_tables);
      calculate_materialization_costs(join, sj_nest, n_tables,
                                      &sj_nest->nested_join->sjm);
      /*
        Cost data is in sj_nest->nested_join->sjm. We also need to save the
        plan:
      */
      if (!(sj_nest->nested_join->sjm.positions =
                (POSITION *)join->thd->alloc(sizeof(POSITION) * n_tables)))
        return true;
      memcpy(sj_nest->nested_join->sjm.positions,
             join->best_positions + join->const_tables,
             sizeof(POSITION) * n_tables);
    }
  }
  return false;
}

/*
  Check if table's Key_use elements have an eq_ref(outer_tables) candidate

  SYNOPSIS
    find_eq_ref_candidate()
      tl                Table to be checked
      sj_inner_tables   Bitmap of inner tables. eq_ref(inner_table) doesn't
                        count.

  DESCRIPTION
    Check if table's Key_use elements have an eq_ref(outer_tables) candidate

  TODO
    Check again if it is feasible to factor common parts with constant table
    search

  RETURN
    true  - There exists an eq_ref(outer-tables) candidate
    false - Otherwise
*/

static bool find_eq_ref_candidate(TABLE_LIST *tl, table_map sj_inner_tables) {
  Key_use *keyuse = tl->table->reginfo.join_tab->keyuse();

  if (keyuse) {
    while (true) /* For each key */
    {
      const uint key = keyuse->key;
      KEY *const keyinfo = tl->table->key_info + key;
      key_part_map bound_parts = 0;
      if ((keyinfo->flags & (HA_NOSAME)) == HA_NOSAME) {
        do /* For all equalities on all key parts */
        {
          /* Check if this is "t.keypart = expr(outer_tables) */
          if (!(keyuse->used_tables & sj_inner_tables) &&
              !(keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL)) {
            /*
              Consider only if the resulting condition does not pass a NULL
              value through. Especially needed for a UNIQUE index on NULLable
              columns where a duplicate row is possible with NULL values.
            */
            if (keyuse->null_rejecting || !keyuse->val->maybe_null ||
                !keyinfo->key_part[keyuse->keypart].field->is_nullable())
              bound_parts |= (key_part_map)1 << keyuse->keypart;
          }
          keyuse++;
        } while (keyuse->key == key && keyuse->table_ref == tl);

        if (bound_parts == LOWER_BITS(uint, keyinfo->user_defined_key_parts))
          return true;
        if (keyuse->table_ref != tl) return false;
      } else {
        do {
          keyuse++;
          if (keyuse->table_ref != tl) return false;
        } while (keyuse->key == key);
      }
    }
  }
  return false;
}

/**
  Pull tables out of semi-join nests based on functional dependencies

  @param join  The join where to do the semi-join table pullout

  @return False if successful, true if error (Out of memory)

  @details
    Pull tables out of semi-join nests based on functional dependencies,
    ie. if a table is accessed via eq_ref(outer_tables).
    The function may be called several times, the caller is responsible
    for setting up proper key information that this function acts upon.

    PRECONDITIONS
    When this function is called, the join may have several semi-join nests
    but it is guaranteed that one semi-join nest does not contain another.
    For functionally dependent tables to be pulled out, key information must
    have been calculated (see update_ref_and_keys()).

    POSTCONDITIONS
     * Tables that were pulled out are removed from the semi-join nest they
       belonged to and added to the parent join nest.
     * For these tables, the used_tables and not_null_tables fields of
       the semi-join nest they belonged to will be adjusted.
       The semi-join nest is also marked as correlated, and
       sj_corr_tables and sj_depends_on are adjusted if necessary.
     * Semi-join nests' sj_inner_tables is set equal to used_tables

    NOTE
    Table pullout may make uncorrelated subquery correlated. Consider this
    example:

     ... WHERE oe IN (SELECT it1.primary_key WHERE p(it1, it2) ... )

    here table it1 can be pulled out (we have it1.primary_key=oe which gives
    us functional dependency). Once it1 is pulled out, all references to it1
    from p(it1, it2) become references to outside of the subquery and thus
    make the subquery (i.e. its semi-join nest) correlated.
    Making the subquery (i.e. its semi-join nest) correlated prevents us from
    using Materialization or LooseScan to execute it.
*/

static bool pull_out_semijoin_tables(JOIN *join) {
  DBUG_TRACE;

  DBUG_ASSERT(!join->select_lex->sj_nests.empty());

  Opt_trace_context *const trace = &join->thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_pullout(trace, "pulled_out_semijoin_tables");

  /* Try pulling out tables from each semi-join nest */
  for (auto sj_list_it = join->select_lex->sj_nests.begin();
       sj_list_it != join->select_lex->sj_nests.end();) {
    TABLE_LIST *sj_nest = *sj_list_it;
    if (sj_nest->is_aj_nest()) {
      ++sj_list_it;
      continue;
    }
    table_map pulled_tables = 0;
    /*
      Calculate set of tables within this semi-join nest that have
      other dependent tables. They cannot be pulled out. For example, with
      t1 SEMIJOIN (t2 LEFT JOIN t3 ON ...) ON t1.a=t2.pk,
      t2 cannot be pulled out because t3 depends on it.
    */
    table_map dep_tables = 0;
    for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
      if (tbl->dep_tables & sj_nest->nested_join->used_tables)
        dep_tables |= tbl->dep_tables;
    }
    /*
      Find which tables we can pull out based on key dependency data.
      Note that pulling one table out can allow us to pull out some
      other tables too.
    */
    bool pulled_a_table;
    do {
      pulled_a_table = false;
      for (TABLE_LIST *tbl : sj_nest->nested_join->join_list) {
        if (tbl->table && !(pulled_tables & tbl->map()) &&
            !(dep_tables & tbl->map())) {
          if (find_eq_ref_candidate(
                  tbl, sj_nest->nested_join->used_tables & ~pulled_tables)) {
            pulled_a_table = true;
            pulled_tables |= tbl->map();
            Opt_trace_object(trace).add_utf8_table(tbl).add(
                "functionally_dependent", true);
            /*
              Pulling a table out of uncorrelated subquery in general makes
              it correlated. See the NOTE to this function.
            */
            sj_nest->nested_join->sj_corr_tables |= tbl->map();
            sj_nest->nested_join->sj_depends_on |= tbl->map();
          }
        }
      }
    } while (pulled_a_table);

    /*
      Move the pulled out TABLE_LIST elements to the parents.
    */
    sj_nest->nested_join->used_tables &= ~pulled_tables;
    sj_nest->nested_join->not_null_tables &= ~pulled_tables;

    /* sj_inner_tables is a copy of nested_join->used_tables */
    sj_nest->sj_inner_tables = sj_nest->nested_join->used_tables;

    bool remove = false;
    if (pulled_tables) {
      mem_root_deque<TABLE_LIST *> *upper_join_list =
          (sj_nest->embedding != nullptr)
              ? &sj_nest->embedding->nested_join->join_list
              : &join->select_lex->top_join_list;

      Prepared_stmt_arena_holder ps_arena_holder(join->thd);

      for (auto child_li = sj_nest->nested_join->join_list.begin();
           child_li != sj_nest->nested_join->join_list.end();) {
        TABLE_LIST *tbl = *child_li;
        if (tbl->table && !(sj_nest->nested_join->used_tables & tbl->map())) {
          /*
            Pull the table up in the same way as simplify_joins() does:
            update join_list and embedding pointers but keep next[_local]
            pointers.
          */
          child_li = sj_nest->nested_join->join_list.erase(child_li);

          upper_join_list->push_back(tbl);

          tbl->join_list = upper_join_list;
          tbl->embedding = sj_nest->embedding;
        } else {
          ++child_li;
        }
      }

      /* Remove the sj-nest itself if we've removed everything from it */
      if (!sj_nest->nested_join->used_tables) {
        upper_join_list->erase(std::find(upper_join_list->begin(),
                                         upper_join_list->end(), sj_nest));
        /* Also remove it from the list of SJ-nests: */
        remove = true;
      }
    }

    if (remove) {
      sj_list_it = join->select_lex->sj_nests.erase(sj_list_it);
    } else {
      ++sj_list_it;
    }
  }
  return false;
}

/**
  @defgroup RefOptimizerModule Ref Optimizer

  @{

  This module analyzes all equality predicates to determine the best
  independent ref/eq_ref/ref_or_null index access methods.

  The 'ref' optimizer determines the columns (and expressions over them) that
  reference columns in other tables via an equality, and analyzes which keys
  and key parts can be used for index lookup based on these references. The
  main outcomes of the 'ref' optimizer are:

  - A bi-directional graph of all equi-join conditions represented as an
    array of Key_use elements. This array is stored in JOIN::keyuse_array in
    table, key, keypart order. Each JOIN_TAB::keyuse points to the
    first Key_use element with the same table as JOIN_TAB::table.

  - The table dependencies needed by the optimizer to determine what
    tables must be before certain table so that they provide the
    necessary column bindings for the equality predicates.

  - Computed properties of the equality predicates such as null_rejecting
    and the result size of each separate condition.

  Updates in JOIN_TAB:
  - JOIN_TAB::keys       Bitmap of all used keys.
  - JOIN_TAB::const_keys Bitmap of all keys that may be used with quick_select.
  - JOIN_TAB::keyuse     Pointer to possible keys.
*/

/**
  A Key_field is a descriptor of a predicate of the form (column @<op@> val).
  Currently 'op' is one of {'=', '<=>', 'IS [NOT] NULL', 'arg1 IN arg2'},
  and 'val' can be either another column or an expression (including constants).

  Key_field's are used to analyze columns that may potentially serve as
  parts of keys for index lookup. If 'field' is part of an index, then
  add_key_part() creates a corresponding Key_use object and inserts it
  into the JOIN::keyuse_array which is passed by update_ref_and_keys().

  The structure is used only during analysis of the candidate columns for
  index 'ref' access.
*/
struct Key_field {
  Key_field(Item_field *item_field, Item *val, uint level, uint optimize,
            bool eq_func, bool null_rejecting, bool *cond_guard,
            uint sj_pred_no)
      : item_field(item_field),
        val(val),
        level(level),
        optimize(optimize),
        eq_func(eq_func),
        null_rejecting(null_rejecting),
        cond_guard(cond_guard),
        sj_pred_no(sj_pred_no) {}
  Item_field *item_field;  ///< Item representing the column
  Item *val;               ///< May be empty if diff constant
  uint level;
  uint optimize;  ///< KEY_OPTIMIZE_*
  bool eq_func;
  /**
    If true, the condition this struct represents will not be satisfied
    when val IS NULL.
    @sa Key_use::null_rejecting .
  */
  bool null_rejecting;
  bool *cond_guard;  ///< @sa Key_use::cond_guard
  uint sj_pred_no;   ///< @sa Key_use::sj_pred_no
};

/* Values in optimize */
#define KEY_OPTIMIZE_EXISTS 1
#define KEY_OPTIMIZE_REF_OR_NULL 2

/**
  Merge new key definitions to old ones, remove those not used in both.

  This is called for OR between different levels.

  To be able to do 'ref_or_null' we merge a comparison of a column
  and 'column IS NULL' to one test.  This is useful for sub select queries
  that are internally transformed to something like:.

  @code
  SELECT * FROM t1 WHERE t1.key=outer_ref_field or t1.key IS NULL
  @endcode

  Key_field::null_rejecting is processed as follows: @n
  result has null_rejecting=true if it is set for both ORed references.
  for example:
  -   (t2.key = t1.field OR t2.key  =  t1.field) -> null_rejecting=true
  -   (t2.key = t1.field OR t2.key <=> t1.field) -> null_rejecting=false

  @todo
    The result of this is that we're missing some 'ref' accesses.
    OptimizerTeam: Fix this
*/

static Key_field *merge_key_fields(Key_field *start, Key_field *new_fields,
                                   Key_field *end, uint and_level) {
  if (start == new_fields) return start;  // Impossible or
  if (new_fields == end) return start;    // No new fields, skip all

  Key_field *first_free = new_fields;

  /* Mark all found fields in old array */
  for (; new_fields != end; new_fields++) {
    const Field *const new_field = new_fields->item_field->field;

    for (Key_field *old = start; old != first_free; old++) {
      const Field *const old_field = old->item_field->field;

      /*
        Check that the Field objects are the same, as we may have several
        Item_field objects pointing to the same Field:
      */
      if (old_field == new_field) {
        /*
          NOTE: below const_item() call really works as "!used_tables()", i.e.
          it can return false where it is feasible to make it return true.

          The cause is as follows: Some of the tables are already known to be
          const tables (the detection code is in JOIN::make_join_plan(),
          above the update_ref_and_keys() call), but we didn't propagate
          information about this: TABLE::const_table is not set to true, and
          Item::update_used_tables() hasn't been called for each item.
          The result of this is that we're missing some 'ref' accesses.
          TODO: OptimizerTeam: Fix this
        */
        if (!new_fields->val->const_item()) {
          /*
            If the value matches, we can use the key reference.
            If not, we keep it until we have examined all new values
          */
          if (old->val->eq(new_fields->val, old_field->binary())) {
            old->level = and_level;
            old->optimize =
                ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
                 ((old->optimize | new_fields->optimize) &
                  KEY_OPTIMIZE_REF_OR_NULL));
            old->null_rejecting =
                (old->null_rejecting && new_fields->null_rejecting);
          }
        } else if (old->eq_func && new_fields->eq_func &&
                   old->val->eq_by_collation(new_fields->val,
                                             old_field->binary(),
                                             old_field->charset())) {
          old->level = and_level;
          old->optimize =
              ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
               ((old->optimize | new_fields->optimize) &
                KEY_OPTIMIZE_REF_OR_NULL));
          old->null_rejecting =
              (old->null_rejecting && new_fields->null_rejecting);
        } else if (old->eq_func && new_fields->eq_func &&
                   ((old->val->const_item() && old->val->is_null()) ||
                    new_fields->val->is_null())) {
          /* field = expression OR field IS NULL */
          old->level = and_level;
          old->optimize = KEY_OPTIMIZE_REF_OR_NULL;
          /*
            Remember the NOT NULL value unless the value does not depend
            on other tables.
          */
          if (!old->val->used_tables() && old->val->is_null())
            old->val = new_fields->val;
          /* The referred expression can be NULL: */
          old->null_rejecting = false;
        } else {
          /*
            We are comparing two different const.  In this case we can't
            use a key-lookup on this so it's better to remove the value
            and let the range optimizer handle it
          */
          if (old == --first_free)  // If last item
            break;
          *old = *first_free;  // Remove old value
          old--;               // Retry this value
        }
      }
    }
  }
  /* Remove all not used items */
  for (Key_field *old = start; old != first_free;) {
    if (old->level != and_level) {  // Not used in all levels
      if (old == --first_free) break;
      *old = *first_free;  // Remove old value
      continue;
    }
    old++;
  }
  return first_free;
}

/**
  Given a field, return its index in semi-join's select list, or UINT_MAX

  @param item_field Field to be looked up in select list

  @retval =UINT_MAX Field is not from a semijoin-transformed subquery
  @retval <UINT_MAX Index in select list of subquery

  @details
  Given a field, find its table; then see if the table is within a
  semi-join nest and if the field was in select list of the subquery
  (if subquery was part of a quantified comparison predicate), or
  the field was a result of subquery decorrelation.
  If it was, then return the field's index in the select list.
  The value is used by LooseScan strategy.
*/

static uint get_semi_join_select_list_index(Item_field *item_field) {
  TABLE_LIST *emb_sj_nest = item_field->table_ref->embedding;
  if (emb_sj_nest && emb_sj_nest->is_sj_or_aj_nest()) {
    List<Item> &items = emb_sj_nest->nested_join->sj_inner_exprs;
    List_iterator<Item> it(items);
    for (uint i = 0; i < items.elements; i++) {
      const Item *sel_item = it++;
      if (sel_item->type() == Item::FIELD_ITEM &&
          down_cast<const Item_field *>(sel_item)->field->eq(item_field->field))
        return i;
    }
  }
  return UINT_MAX;
}

/**
   @brief
   If EXPLAIN or if the --safe-updates option is enabled, add a warning that an
   index cannot be used for ref access.

   @details
   If EXPLAIN or if the --safe-updates option is enabled, add a warning for each
   index that cannot be used for ref access due to either type conversion or
   different collations on the field used for comparison

   Example type conversion (char compared to int):

   CREATE TABLE t1 (url char(1) PRIMARY KEY);
   SELECT * FROM t1 WHERE url=1;

   Example different collations (danish vs german2):

   CREATE TABLE t1 (url char(1) PRIMARY KEY) collate latin1_danish_ci;
   SELECT * FROM t1 WHERE url='1' collate latin1_german2_ci;

   @param thd                Thread for the connection that submitted the query
   @param field              Field used in comparison
   @param cant_use_index     Indexes that cannot be used for lookup
 */
static void warn_index_not_applicable(THD *thd, const Field *field,
                                      const Key_map cant_use_index) {
  Functional_index_error_handler functional_index_error_handler(field, thd);

  if (thd->lex->is_explain() ||
      thd->variables.option_bits & OPTION_SAFE_UPDATES)
    for (uint j = 0; j < field->table->s->keys; j++)
      if (cant_use_index.is_set(j))
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_WARN_INDEX_NOT_APPLICABLE,
                            ER_THD(thd, ER_WARN_INDEX_NOT_APPLICABLE), "ref",
                            field->table->key_info[j].name, field->field_name);
}

/**
  Add a possible key to array of possible keys if it's usable as a key

  @param [in,out] key_fields Used as an input parameter in the sense that it is
  a pointer to a pointer to a memory area where an array of Key_field objects
  will stored. It is used as an out parameter in the sense that the pointer will
  be updated to point beyond the last Key_field written.

  @param thd                session context
  @param and_level          And level, to be stored in Key_field
  @param cond               Condition predicate
  @param item_field         Field used in comparison
  @param eq_func            True if we used =, <=> or IS NULL
  @param value              Array of values used for comparison with field
  @param num_values         Number of elements in the array of values
  @param usable_tables      Tables which can be used for key optimization
  @param sargables          IN/OUT Array of found sargable candidates.
                            Will be ignored in case eq_func is true.

  @note
    If we are doing a NOT NULL comparison on a NOT NULL field in a outer join
    table, we store this to be able to do not exists optimization later.


  @returns false if success, true if error
*/

static bool add_key_field(THD *thd, Key_field **key_fields, uint and_level,
                          Item_func *cond, Item_field *item_field, bool eq_func,
                          Item **value, uint num_values,
                          table_map usable_tables, SARGABLE_PARAM **sargables) {
  DBUG_ASSERT(cond->is_bool_func());
  DBUG_ASSERT(eq_func || sargables);

  Field *const field = item_field->field;
  TABLE_LIST *const tl = item_field->table_ref;

  if (tl->table->reginfo.join_tab == nullptr) {
    /*
       Due to a bug in IN-to-EXISTS (grep for real_item() in item_subselect.cc
       for more info), an index over a field from an outer query might be
       considered here, which is incorrect. Their query has been fully
       optimized already so their reginfo.join_tab is NULL and we reject them.
    */
    return false;
  }

  DBUG_PRINT("info", ("add_key_field for field %s", field->field_name));
  uint exists_optimize = 0;
  if (!tl->derived_keys_ready && tl->uses_materialization() &&
      !tl->table->is_created()) {
    bool allocated;
    if (tl->update_derived_keys(thd, field, value, num_values, &allocated))
      return true;
    if (!allocated) return false;
  }
  if (!(field->flags & PART_KEY_FLAG)) {
    // Don't remove column IS NULL on a LEFT JOIN table
    if (!eq_func || (*value)->type() != Item::NULL_ITEM ||
        !tl->table->is_nullable() || field->is_nullable())
      return false;  // Not a key. Skip it
    exists_optimize = KEY_OPTIMIZE_EXISTS;
    DBUG_ASSERT(num_values == 1);
  } else {
    table_map used_tables = 0;
    bool optimizable = false;
    for (uint i = 0; i < num_values; i++) {
      used_tables |= (value[i])->used_tables();
      if (!((value[i])->used_tables() & (tl->map() | RAND_TABLE_BIT)))
        optimizable = true;
    }
    if (!optimizable) return false;
    if (!(usable_tables & tl->map())) {
      if (!eq_func || (*value)->type() != Item::NULL_ITEM ||
          !tl->table->is_nullable() || field->is_nullable())
        return false;  // Can't use left join optimize
      exists_optimize = KEY_OPTIMIZE_EXISTS;
    } else {
      JOIN_TAB *stat = tl->table->reginfo.join_tab;
      Key_map possible_keys = field->key_start;
      possible_keys.intersect(tl->table->keys_in_use_for_query);
      stat[0].keys().merge(possible_keys);  // Add possible keys

      /*
        Save the following cases:
        Field op constant
        Field LIKE constant where constant doesn't start with a wildcard
        Field = field2 where field2 is in a different table
        Field op formula
        Field IS NULL
        Field IS NOT NULL
        Field BETWEEN ...
        Field IN ...
      */
      stat[0].key_dependent |= used_tables;

      bool is_const = true;
      for (uint i = 0; i < num_values; i++) {
        if (!(is_const &= value[i]->const_for_execution())) break;
      }
      if (is_const)
        stat[0].const_keys.merge(possible_keys);
      else if (!eq_func) {
        /*
          Save info to be able check whether this predicate can be
          considered as sargable for range analysis after reading const tables.
          We do not save info about equalities as update_const_equal_items
          will take care of updating info on keys from sargable equalities.
        */
        DBUG_ASSERT(sargables);
        (*sargables)--;
        /*
          The sargables and key_fields arrays share the same memory
          buffer, and grow from opposite directions, so make sure they
          don't cross.
        */
        DBUG_ASSERT(*sargables >
                    reinterpret_cast<SARGABLE_PARAM *>(*key_fields));
        (*sargables)->field = field;
        (*sargables)->arg_value = value;
        (*sargables)->num_values = num_values;
      }
      /*
        We can't always use indexes when comparing a string index to a
        number. cmp_type() is checked to allow compare of dates to numbers.
        eq_func is NEVER true when num_values > 1
       */
      if (!eq_func) return false;

      /*
        Check if the field and value are comparable in the index.
        @todo: This code is almost identical to comparable_in_index()
        in opt_range.cc. Consider replacing the checks below with a
        function call to comparable_in_index()
      */
      if (field->result_type() == STRING_RESULT) {
        if ((*value)->result_type() != STRING_RESULT) {
          if (field->cmp_type() != (*value)->result_type()) {
            warn_index_not_applicable(stat->join()->thd, field, possible_keys);
            return false;
          }
        } else {
          /*
            Can't optimize datetime_column=indexed_varchar_column,
            also can't use indexes if the effective collation
            of the operation differ from the field collation.
            IndexedTimeComparedToDate: can't optimize
            'indexed_time = temporal_expr_with_date_part' because:
            - without index, a TIME column with value '48:00:00' is equal to a
              DATETIME column with value 'CURDATE() + 2 days'
            - with ref access into the TIME column, CURDATE() + 2 days becomes
              "00:00:00" (Field_timef::store_internal() simply extracts the time
              part from the datetime) which is a lookup key which does not match
              "48:00:00"; so ref access is not be able to give the same result
              as without index, so is disabled.
            On the other hand, we can optimize indexed_datetime = time
            because Field_temporal_with_date::store_time() will convert
            48:00:00 to CURDATE() + 2 days which is the correct lookup key.
          */
          if ((!is_temporal_type(field->type()) && value[0]->is_temporal()) ||
              (field->cmp_type() == STRING_RESULT &&
               field->match_collation_to_optimize_range() &&
               field->charset() != cond->compare_collation()) ||
              field_time_cmp_date(field, value[0])) {
            warn_index_not_applicable(stat->join()->thd, field, possible_keys);
            return false;
          }
        }
      }

      /*
        We can't use indexes when comparing to a JSON value. For example,
        the string '{}' should compare equal to the JSON string "{}". If
        we use a string index to compare the two strings, we will be
        comparing '{}' and '"{}"', which don't compare equal.
        The only exception is Item_json, which is a basic const item and is
        used to contain value coerced to index's type.
      */
      if (value[0]->result_type() == STRING_RESULT &&
          value[0]->data_type() == MYSQL_TYPE_JSON &&
          !value[0]->basic_const_item()) {
        warn_index_not_applicable(stat->join()->thd, field, possible_keys);
        return false;
      }
    }
  }
  /*
    For the moment eq_func is always true. This slot is reserved for future
    extensions where we want to remembers other things than just eq comparisons
  */
  DBUG_ASSERT(eq_func);
  /*
    If the condition has form "tbl.keypart = othertbl.field" and
    othertbl.field can be NULL, there will be no matches if othertbl.field
    has NULL value.
    We use null_rejecting in add_not_null_conds() to add
    'othertbl.field IS NOT NULL' to tab->m_condition, if this is not an outer
    join. We also use it to shortcut reading "tbl" when othertbl.field is
    found to be a NULL value (in RefIterator and BKA).
  */
  Item *const real = (*value)->real_item();
  const bool null_rejecting =
      ((cond->functype() == Item_func::EQ_FUNC) ||
       (cond->functype() == Item_func::MULT_EQUAL_FUNC)) &&
      (real->type() == Item::FIELD_ITEM) &&
      (down_cast<Item_field *>(real)->field->is_nullable() ||
       down_cast<Item_field *>(real)->field->table->is_nullable());

  /* Store possible eq field */
  new (*key_fields) Key_field(item_field, *value, and_level, exists_optimize,
                              eq_func, null_rejecting, nullptr,
                              get_semi_join_select_list_index(item_field));
  (*key_fields)++;
  /*
    The sargables and key_fields arrays share the same memory buffer,
    and grow from opposite directions, so make sure they don't
    cross. But if sargables was NULL, eq_func had to be true and we
    don't write any sargables.
  */
  DBUG_ASSERT(sargables == nullptr ||
              *key_fields < reinterpret_cast<Key_field *>(*sargables));

  return false;
}

/**
  Add possible keys to array of possible keys originated from a simple
  predicate.

    @param  thd            session context
    @param[in,out] key_fields Pointer to add key, if usable
                           is incremented if key was stored in the array
    @param  and_level      And level, to be stored in Key_field
    @param  cond           Condition predicate
    @param  field_item     Field used in comparision
    @param  eq_func        True if we used =, <=> or IS NULL
    @param  val            Value used for comparison with field
                           Is NULL for BETWEEN and IN
    @param  num_values
    @param  usable_tables  Tables which can be used for key optimization
    @param  sargables      IN/OUT Array of found sargable candidates

  @note
    If field items f1 and f2 belong to the same multiple equality and
    a key is added for f1, the the same key is added for f2.

  @returns false if success, true if error
*/

static bool add_key_equal_fields(THD *thd, Key_field **key_fields,
                                 uint and_level, Item_func *cond,
                                 Item_field *field_item, bool eq_func,
                                 Item **val, uint num_values,
                                 table_map usable_tables,
                                 SARGABLE_PARAM **sargables) {
  DBUG_ASSERT(cond->is_bool_func());

  if (add_key_field(thd, key_fields, and_level, cond, field_item, eq_func, val,
                    num_values, usable_tables, sargables))
    return true;
  Item_equal *item_equal = field_item->item_equal;
  if (item_equal == nullptr) return false;
  /*
    Add to the set of possible key values every substitution of
    the field for an equal field included into item_equal
  */
  Item_equal_iterator it(*item_equal);
  Item_field *item;
  while ((item = it++)) {
    if (!field_item->field->eq(item->field)) {
      if (add_key_field(thd, key_fields, and_level, cond, item, eq_func, val,
                        num_values, usable_tables, sargables))
        return true;
    }
  }
  return false;
}

/**
  Check if an expression is a non-outer field.

  Checks if an expression is a field and belongs to the current select.

  @param   field  Item expression to check

  @return boolean
     @retval true   the expression is a local field
     @retval false  it's something else
*/

static bool is_local_field(Item *field) {
  return field->real_item()->type() == Item::FIELD_ITEM &&
         !(field->used_tables() & OUTER_REF_TABLE_BIT) &&
         !down_cast<Item_ident *>(field)->depended_from &&
         !down_cast<Item_ident *>(field->real_item())->depended_from;
}

/**
  Check if a row constructor expression is over columns in the same query block.

  @param item_row Row expression to check.

  @return boolean
  @retval true  The expression is a local column reference.
  @retval false It's something else.
*/
static bool is_row_of_local_columns(Item_row *item_row) {
  for (uint i = 0; i < item_row->cols(); ++i)
    if (!is_local_field(item_row->element_index(i))) return false;
  return true;
}

/**
   The guts of the ref optimizer. This function, along with the other
   add_key_* functions, make up a recursive procedure that analyzes a
   condition expression (a tree of AND and OR predicates) and does
   many things.

   @param thd      session context
   @param join     The query block involving the condition.
   @param [in,out] key_fields Start of memory buffer, see below.
   @param [in,out] and_level Current 'and level', see below.
   @param cond The conditional expression to analyze.
   @param usable_tables Tables not in this bitmap will not be examined.
   @param [in,out] sargables End of memory buffer, see below.

   @returns false if success, true if error

   This documentation is the result of reverse engineering and may
   therefore not capture the full gist of the procedure, but it is
   known to do the following:

   - Populate a raw memory buffer from two directions at the same time. An
     'array' of Key_field objects fill the buffer from low to high addresses
     whilst an 'array' of SARGABLE_PARAM's fills the buffer from high to low
     addresses. At the first call to this function, it is assumed that
     key_fields points to the beginning of the buffer and sargables point to the
     end (except for a poor-mans 'null element' at the very end).

   - Update a number of properties in the JOIN_TAB's that can be used
     to find search keys (sargables).

     - JOIN_TAB::keys
     - JOIN_TAB::key_dependent
     - JOIN_TAB::const_keys (dictates if the range optimizer will be run
       later.)

   The Key_field objects are marked with something called an 'and_level', which
   does @b not correspond to their nesting depth within the expression tree. It
   is rather a tag to group conjunctions together. For instance, in the
   conditional expression

   @code
     a = 0 AND b = 0
   @endcode

   two Key_field's are produced, both having an and_level of 0.

   In an expression such as

   @code
     a = 0 AND b = 0 OR a = 1
   @endcode

   three Key_field's are produced, the first two corresponding to 'a = 0' and
   'b = 0', respectively, both with and_level 0. The third one corresponds to
   'a = 1' and has an and_level of 1.

   A separate function, merge_key_fields() performs ref access validation on
   the Key_field array on the recursice ascent. If some Key_field's cannot be
   used for ref access, the key_fields pointer is rolled back. All other
   modifications to the query plan remain.
*/
static bool add_key_fields(THD *thd, JOIN *join, Key_field **key_fields,
                           uint *and_level, Item *cond, table_map usable_tables,
                           SARGABLE_PARAM **sargables) {
  DBUG_ASSERT(cond->is_bool_func());

  if (cond->type() == Item_func::COND_ITEM) {
    List_iterator_fast<Item> li(*((Item_cond *)cond)->argument_list());
    Key_field *org_key_fields = *key_fields;

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++)) {
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
      }
      for (; org_key_fields != *key_fields; org_key_fields++)
        org_key_fields->level = *and_level;
    } else {
      (*and_level)++;
      if (add_key_fields(thd, join, key_fields, and_level, li++, usable_tables,
                         sargables))
        return true;
      Item *item;
      while ((item = li++)) {
        Key_field *start_key_fields = *key_fields;
        (*and_level)++;
        if (add_key_fields(thd, join, key_fields, and_level, item,
                           usable_tables, sargables))
          return true;
        *key_fields = merge_key_fields(org_key_fields, start_key_fields,
                                       *key_fields, ++(*and_level));
      }
    }
    return false;
  }

  /*
    Subquery optimization: Conditions that are pushed down into subqueries
    are wrapped into Item_func_trig_cond. We process the wrapped condition
    but need to set cond_guard for Key_use elements generated from it.
  */
  if (cond->type() == Item::FUNC_ITEM &&
      down_cast<Item_func *>(cond)->functype() == Item_func::TRIG_COND_FUNC) {
    Item *const cond_arg = down_cast<Item_func *>(cond)->arguments()[0];
    if (!join->group_list && !join->order && join->unit->item &&
        join->unit->item->substype() == Item_subselect::IN_SUBS &&
        !join->unit->is_union()) {
      Key_field *save = *key_fields;
      if (add_key_fields(thd, join, key_fields, and_level, cond_arg,
                         usable_tables, sargables))
        return true;
      // Indicate that this ref access candidate is for subquery lookup:
      for (; save != *key_fields; save++)
        save->cond_guard = ((Item_func_trig_cond *)cond)->get_trig_var();
    }
    return false;
  }

  /* If item is of type 'field op field/constant' add it to key_fields */
  if (cond->type() != Item::FUNC_ITEM) return false;
  Item_func *const cond_func = down_cast<Item_func *>(cond);
  auto optimize = cond_func->select_optimize(thd);
  // Catch errors that might be thrown during select_optimize()
  if (thd->is_error()) return true;
  switch (optimize) {
    case Item_func::OPTIMIZE_NONE:
      break;
    case Item_func::OPTIMIZE_KEY: {
      Item **values;
      /*
        Build list of possible keys for 'a BETWEEN low AND high'.
        It is handled similar to the equivalent condition
        'a >= low AND a <= high':
      */
      if (cond_func->functype() == Item_func::BETWEEN) {
        Item_field *field_item;
        bool equal_func = false;
        uint num_values = 2;
        values = cond_func->arguments();

        bool binary_cmp =
            (values[0]->real_item()->type() == Item::FIELD_ITEM)
                ? ((Item_field *)values[0]->real_item())->field->binary()
                : true;

        /*
          Additional optimization: If 'low = high':
          Handle as if the condition was "t.key = low".
        */
        if (!((Item_func_between *)cond_func)->negated &&
            values[1]->eq(values[2], binary_cmp)) {
          equal_func = true;
          num_values = 1;
        }

        /*
          Append keys for 'field <cmp> value[]' if the
          condition is of the form::
          '<field> BETWEEN value[1] AND value[2]'
        */
        if (is_local_field(values[0])) {
          field_item = (Item_field *)(values[0]->real_item());
          if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                   field_item, equal_func, &values[1],
                                   num_values, usable_tables, sargables))
            return true;
        }
        /*
          Append keys for 'value[0] <cmp> field' if the
          condition is of the form:
          'value[0] BETWEEN field1 AND field2'
        */
        for (uint i = 1; i <= num_values; i++) {
          if (is_local_field(values[i])) {
            field_item = (Item_field *)(values[i]->real_item());
            if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                     field_item, equal_func, values, 1,
                                     usable_tables, sargables))
              return true;
          }
        }
      }  // if ( ... Item_func::BETWEEN)
      else if (cond_func->functype() == Item_func::MEMBER_OF_FUNC &&
               is_local_field(cond_func->key_item())) {
        // The predicate is <val> IN (<typed array>)
        add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                             (Item_field *)(cond_func->key_item()->real_item()),
                             true, cond_func->arguments(), 1, usable_tables,
                             sargables);
      } else if (cond_func->functype() == Item_func::JSON_CONTAINS ||
                 cond_func->functype() == Item_func::JSON_OVERLAPS) {
        /*
          Applicability analysis was done during substitute_gc().
          Check here that a typed array field is used and there's a key over
          it.
          1) func has a key item
          2) key item is a local field
          3) key item is a typed array field
          If so, mark appropriate index as available for range optimizer
        */
        if (!cond_func->key_item() ||                  // 1
            !is_local_field(cond_func->key_item()) ||  // 2
            !cond_func->key_item()->returns_array())   // 3
          break;
        const Field *field =
            (down_cast<const Item_field *>(cond_func->key_item()))->field;
        JOIN_TAB *tab = field->table->reginfo.join_tab;
        Key_map possible_keys = field->key_start;

        possible_keys.intersect(field->table->keys_in_use_for_query);
        tab->keys().merge(possible_keys);      // Add possible keys
        tab->const_keys.merge(possible_keys);  // Add possible keys
      }                                        // if (... Item_func::CONTAINS)
      // The predicate is IN or <>
      else if (is_local_field(cond_func->key_item()) &&
               !(cond_func->used_tables() & OUTER_REF_TABLE_BIT)) {
        values = cond_func->arguments() + 1;
        if (cond_func->functype() == Item_func::NE_FUNC &&
            is_local_field(cond_func->arguments()[1]))
          values--;
        DBUG_ASSERT(cond_func->functype() != Item_func::IN_FUNC ||
                    cond_func->argument_count() != 2);
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->key_item()->real_item()), false,
                values, cond_func->argument_count() - 1, usable_tables,
                sargables))
          return true;
      } else if (cond_func->functype() == Item_func::IN_FUNC &&
                 cond_func->key_item()->type() == Item::ROW_ITEM) {
        /*
          The condition is (column1, column2, ... ) IN ((const1_1, const1_2),
          ...) and there is an index on (column1, column2, ...)

          The code below makes sure that the row constructor on the lhs indeed
          contains only column references before calling add_key_field on them.

          We can't do a ref access on IN, yet here we are. Why? We need
          to run add_key_field() only because it verifies that there are
          only constant expressions in the rows on the IN's rhs, see
          comment above the call to add_key_field() below.

          Actually, We could in theory do a ref access if the IN rhs
          contained just a single row, but there is a hack in the parser
          causing such IN predicates be parsed as row equalities.
        */
        Item_row *lhs_row = static_cast<Item_row *>(cond_func->key_item());
        if (is_row_of_local_columns(lhs_row)) {
          for (uint i = 0; i < lhs_row->cols(); ++i) {
            Item *const lhs_item = lhs_row->element_index(i)->real_item();
            DBUG_ASSERT(lhs_item->type() == Item::FIELD_ITEM);
            Item_field *const lhs_column = static_cast<Item_field *>(lhs_item);
            // j goes from 1 since arguments()[0] is the lhs of IN.
            for (uint j = 1; j < cond_func->argument_count(); ++j) {
              // Here we pick out the i:th column in the j:th row.
              Item *rhs_item = cond_func->arguments()[j];
              DBUG_ASSERT(rhs_item->type() == Item::ROW_ITEM);
              Item_row *rhs_row = static_cast<Item_row *>(rhs_item);
              DBUG_ASSERT(rhs_row->cols() == lhs_row->cols());
              Item **rhs_expr_ptr = rhs_row->addr(i);
              /*
                add_key_field() will write a Key_field on each call
                here, but we don't care, it will never be used. We only
                call it for the side effect: update JOIN_TAB::const_keys
                so the range optimizer can be invoked. We pass a
                scrap buffer and pointer here.
              */
              Key_field scrap_key_field = **key_fields;
              Key_field *scrap_key_field_ptr = &scrap_key_field;
              if (add_key_field(thd, &scrap_key_field_ptr, *and_level,
                                cond_func, lhs_column,
                                true,  // eq_func
                                rhs_expr_ptr,
                                1,  // Number of expressions: one
                                usable_tables,
                                nullptr))  // sargables
                return true;
              // The pointer is not supposed to increase by more than one.
              DBUG_ASSERT(scrap_key_field_ptr <= &scrap_key_field + 1);
            }
          }
        }
      }
      break;
    }
    case Item_func::OPTIMIZE_OP: {
      bool equal_func = (cond_func->functype() == Item_func::EQ_FUNC ||
                         cond_func->functype() == Item_func::EQUAL_FUNC);

      if (is_local_field(cond_func->arguments()[0])) {
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[0])->real_item(),
                equal_func, cond_func->arguments() + 1, 1, usable_tables,
                sargables))
          return true;
      } else {
        Item *real_item = cond_func->arguments()[0]->real_item();
        if (real_item->type() == Item::FUNC_ITEM) {
          Item_func *func_item = down_cast<Item_func *>(real_item);
          if (func_item->functype() == Item_func::COLLATE_FUNC) {
            Item *key_item = func_item->key_item();
            if (key_item->type() == Item::FIELD_ITEM) {
              if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                       down_cast<Item_field *>(key_item),
                                       equal_func, cond_func->arguments() + 1,
                                       1, usable_tables, sargables))
                return true;
            }
          }
        }
      }
      if (is_local_field(cond_func->arguments()[1]) &&
          cond_func->functype() != Item_func::LIKE_FUNC) {
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[1])->real_item(),
                equal_func, cond_func->arguments(), 1, usable_tables,
                sargables))
          return true;
      } else {
        Item *real_item = cond_func->arguments()[1]->real_item();
        if (real_item->type() == Item::FUNC_ITEM) {
          Item_func *func_item = down_cast<Item_func *>(real_item);
          if (func_item->functype() == Item_func::COLLATE_FUNC) {
            Item *key_item = func_item->key_item();
            if (key_item->type() == Item::FIELD_ITEM) {
              if (add_key_equal_fields(thd, key_fields, *and_level, cond_func,
                                       down_cast<Item_field *>(key_item),
                                       equal_func, cond_func->arguments(), 1,
                                       usable_tables, sargables))
                return true;
            }
          }
        }
      }

      break;
    }
    case Item_func::OPTIMIZE_NULL:
      /* column_name IS [NOT] NULL */
      if (is_local_field(cond_func->arguments()[0]) &&
          !(cond_func->used_tables() & OUTER_REF_TABLE_BIT)) {
        Item *tmp = new Item_null;
        if (tmp == nullptr) return true;
        if (add_key_equal_fields(
                thd, key_fields, *and_level, cond_func,
                (Item_field *)(cond_func->arguments()[0])->real_item(),
                cond_func->functype() == Item_func::ISNULL_FUNC, &tmp, 1,
                usable_tables, sargables))
          return true;
      }
      break;
    case Item_func::OPTIMIZE_EQUAL:
      Item_equal *item_equal = (Item_equal *)cond;
      Item *const_item = item_equal->get_const();
      if (const_item) {
        /*
          For each field field1 from item_equal consider the equality
          field1=const_item as a condition allowing an index access of the table
          with field1 by the keys value of field1.
        */
        Item_equal_iterator it(*item_equal);
        Item_field *item;
        while ((item = it++)) {
          if (add_key_field(thd, key_fields, *and_level, cond_func, item, true,
                            &const_item, 1, usable_tables, sargables))
            return true;
        }
      } else {
        /*
          Consider all pairs of different fields included into item_equal.
          For each of them (field1, field1) consider the equality
          field1=field2 as a condition allowing an index access of the table
          with field1 by the keys value of field2.
        */
        Item_equal_iterator outer_it(*item_equal);
        Item_equal_iterator inner_it(*item_equal);
        Item_field *outer;
        while ((outer = outer_it++)) {
          Item_field *inner;
          while ((inner = inner_it++)) {
            if (!outer->field->eq(inner->field)) {
              if (add_key_field(thd, key_fields, *and_level, cond_func, outer,
                                true, (Item **)&inner, 1, usable_tables,
                                sargables))
                return true;
            }
          }
          inner_it.rewind();
        }
      }
      break;
  }
  return false;
}

/*
  Add all keys with uses 'field' for some keypart
  If field->and_level != and_level then only mark key_part as const_part

  RETURN
   0 - OK
   1 - Out of memory.
*/

static bool add_key_part(Key_use_array *keyuse_array, Key_field *key_field) {
  if (key_field->eq_func && !(key_field->optimize & KEY_OPTIMIZE_EXISTS)) {
    const Field *const field = key_field->item_field->field;
    TABLE_LIST *const tl = key_field->item_field->table_ref;
    TABLE *const table = tl->table;

    for (uint key = 0; key < table->s->keys; key++) {
      if (!(table->keys_in_use_for_query.is_set(key))) continue;
      if (table->key_info[key].flags & (HA_FULLTEXT | HA_SPATIAL))
        continue;  // ToDo: ft-keys in non-ft queries.   SerG

      uint key_parts = actual_key_parts(&table->key_info[key]);
      for (uint part = 0; part < key_parts; part++) {
        if (field->eq(table->key_info[key].key_part[part].field)) {
          const Key_use keyuse(tl, key_field->val,
                               key_field->val->used_tables(), key, part,
                               key_field->optimize & KEY_OPTIMIZE_REF_OR_NULL,
                               (key_part_map)1 << part,
                               ~(ha_rows)0,  // will be set in optimize_keyuse
                               key_field->null_rejecting, key_field->cond_guard,
                               key_field->sj_pred_no);
          if (keyuse_array->push_back(keyuse))
            return true; /* purecov: inspected */
        }
      }
    }
  }
  return false;
}

/**
   Function parses WHERE condition and add key_use for FT index
   into key_use array if suitable MATCH function is found.
   Condition should be a set of AND expression, OR is not supported.
   MATCH function should be a part of simple expression.
   Simple expression is MATCH only function or MATCH is a part of
   comparison expression ('>=' or '>' operations are supported).
   It also sets FT_HINTS values(op_type, op_value).

   @param keyuse_array      Key_use array
   @param stat              JOIN_TAB structure
   @param cond              WHERE condition
   @param usable_tables     usable tables
   @param simple_match_expr true if this is the first call false otherwise.
                            if MATCH function is found at first call it means
                            that MATCH is simple expression, otherwise, in case
                            of AND/OR condition this parameter will be false.

   @retval
   true if FT key was added to Key_use array
   @retval
   false if no key was added to Key_use array

*/

static bool add_ft_keys(Key_use_array *keyuse_array, JOIN_TAB *stat, Item *cond,
                        table_map usable_tables, bool simple_match_expr) {
  Item_func_match *cond_func = nullptr;

  if (!cond) return false;

  DBUG_ASSERT(cond->is_bool_func());

  if (cond->type() == Item::FUNC_ITEM) {
    Item_func *func = down_cast<Item_func *>(cond);
    Item_func::Functype functype = func->functype();
    if (functype == Item_func::MATCH_FUNC) {
      func = down_cast<Item_func *>(func->arguments()[0]);
      functype = func->functype();
    }
    enum ft_operation op_type = FT_OP_NO;
    double op_value = 0.0;
    if (functype == Item_func::FT_FUNC) {
      cond_func = down_cast<Item_func_match *>(func)->get_master();
      cond_func->set_hints_op(op_type, op_value);
    } else if (func->arg_count == 2) {
      Item *arg0 = func->arguments()[0];
      Item *arg1 = func->arguments()[1];
      if (arg1->const_item() && arg0->type() == Item::FUNC_ITEM &&
          down_cast<Item_func *>(arg0)->functype() == Item_func::FT_FUNC &&
          ((functype == Item_func::GE_FUNC &&
            (op_value = arg1->val_real()) > 0) ||
           (functype == Item_func::GT_FUNC &&
            (op_value = arg1->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg0)->get_master();
        if (functype == Item_func::GE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::GT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      } else if (arg0->const_item() && arg1->type() == Item::FUNC_ITEM &&
                 down_cast<Item_func *>(arg1)->functype() ==
                     Item_func::FT_FUNC &&
                 ((functype == Item_func::LE_FUNC &&
                   (op_value = arg0->val_real()) > 0) ||
                  (functype == Item_func::LT_FUNC &&
                   (op_value = arg0->val_real()) >= 0))) {
        cond_func = down_cast<Item_func_match *>(arg1)->get_master();
        if (functype == Item_func::LE_FUNC)
          op_type = FT_OP_GE;
        else if (functype == Item_func::LT_FUNC)
          op_type = FT_OP_GT;
        cond_func->set_hints_op(op_type, op_value);
      }
    }
  } else if (cond->type() == Item::COND_ITEM) {
    List_iterator_fast<Item> li(*down_cast<Item_cond *>(cond)->argument_list());

    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++)) {
        if (add_ft_keys(keyuse_array, stat, item, usable_tables, false))
          return true;
      }
    }
  }

  if (!cond_func || cond_func->key == NO_SUCH_KEY ||
      !(usable_tables & cond_func->table_ref->map()))
    return false;

  cond_func->set_simple_expression(simple_match_expr);

  const Key_use keyuse(cond_func->table_ref, cond_func,
                       cond_func->key_item()->used_tables(), cond_func->key,
                       FT_KEYPART,
                       0,            // optimize
                       0,            // keypart_map
                       ~(ha_rows)0,  // ref_table_rows
                       false,        // null_rejecting
                       nullptr,      // cond_guard
                       UINT_MAX);    // sj_pred_no
  return keyuse_array->push_back(keyuse);
}

/**
  Compares two keyuse elements.

  @param a first Key_use element
  @param b second Key_use element

  Compare Key_use elements so that they are sorted as follows:
    -# By table.
    -# By key for each table.
    -# By keypart for each key.
    -# Const values.
    -# Ref_or_null.

  @retval true If a < b.
  @retval false If a >= b.
*/
static bool sort_keyuse(const Key_use &a, const Key_use &b) {
  if (a.table_ref->tableno() != b.table_ref->tableno())
    return a.table_ref->tableno() < b.table_ref->tableno();
  if (a.key != b.key) return a.key < b.key;
  if (a.keypart != b.keypart) return a.keypart < b.keypart;
  // Place const values before other ones
  bool a_const = a.used_tables & ~OUTER_REF_TABLE_BIT;
  bool b_const = b.used_tables & ~OUTER_REF_TABLE_BIT;
  if (a_const != b_const) return b_const;
  /* Place rows that are not 'OPTIMIZE_REF_OR_NULL' first */
  return (a.optimize & KEY_OPTIMIZE_REF_OR_NULL) <
         (b.optimize & KEY_OPTIMIZE_REF_OR_NULL);
}

/*
  Add to Key_field array all 'ref' access candidates within nested join.

    This function populates Key_field array with entries generated from the
    ON condition of the given nested join, and does the same for nested joins
    contained within this nested join.

  @param          thd                 session context
  @param[in]      nested_join_table   Nested join pseudo-table to process
  @param[in,out]  end                 End of the key field array
  @param[in,out]  and_level           And-level
  @param[in,out]  sargables           Array of found sargable candidates

  @returns false if success, true if error

  @note
    We can add accesses to the tables that are direct children of this nested
    join (1), and are not inner tables w.r.t their neighbours (2).

    Example for #1 (outer brackets pair denotes nested join this function is
    invoked for):
    @code
     ... LEFT JOIN (t1 LEFT JOIN (t2 ... ) ) ON cond
    @endcode
    Example for #2:
    @code
     ... LEFT JOIN (t1 LEFT JOIN t2 ) ON cond
    @endcode
    In examples 1-2 for condition cond, we can add 'ref' access candidates to
    t1 only.
    Example #3:
    @code
     ... LEFT JOIN (t1, t2 LEFT JOIN t3 ON inner_cond) ON cond
    @endcode
    Here we can add 'ref' access candidates for t1 and t2, but not for t3.
*/

static bool add_key_fields_for_nj(THD *thd, JOIN *join,
                                  TABLE_LIST *nested_join_table,
                                  Key_field **end, uint *and_level,
                                  SARGABLE_PARAM **sargables) {
  mem_root_deque<TABLE_LIST *> &join_list =
      nested_join_table->nested_join->join_list;
  auto li = join_list.begin();
  auto li_end = join_list.end();
  auto li2 = join_list.begin();
  auto li2_end = join_list.end();
  bool have_another = false;
  table_map tables = 0;
  TABLE_LIST *table;

  while ((table = (li != li_end) ? *li++ : nullptr) ||
         (have_another && li2 != join_list.end() &&
          (li = li2, li_end = li2_end, have_another = false,
           (li != li_end) && (table = *li++)))) {
    if (table->nested_join) {
      if (!table->join_cond_optim()) {
        /* It's a semi-join nest. Walk into it as if it wasn't a nest */
        have_another = true;
        li2 = li;
        li2_end = li_end;
        li = table->nested_join->join_list.begin();
        li_end = table->nested_join->join_list.end();
      } else {
        if (add_key_fields_for_nj(thd, join, table, end, and_level, sargables))
          return true;
      }
    } else if (!table->join_cond_optim())
      tables |= table->map();
  }
  if (nested_join_table->join_cond_optim()) {
    if (add_key_fields(thd, join, end, and_level,
                       nested_join_table->join_cond_optim(), tables, sargables))
      return true;
  }
  return false;
}

///  @} (end of group RefOptimizerModule)

/**
  Check for the presence of AGGFN(DISTINCT a) queries that may be subject
  to loose index scan.


  Check if the query is a subject to AGGFN(DISTINCT) using loose index scan
  (QUICK_GROUP_MIN_MAX_SELECT).
  Optionally (if out_args is supplied) will push the arguments of
  AGGFN(DISTINCT) to the list

  Check for every COUNT(DISTINCT), AVG(DISTINCT) or
  SUM(DISTINCT). These can be resolved by Loose Index Scan as long
  as all the aggregate distinct functions refer to the same
  fields. Thus:

  SELECT AGGFN(DISTINCT a, b), AGGFN(DISTINCT b, a)... => can use LIS
  SELECT AGGFN(DISTINCT a),    AGGFN(DISTINCT a)   ... => can use LIS
  SELECT AGGFN(DISTINCT a, b), AGGFN(DISTINCT a)   ... => cannot use LIS
  SELECT AGGFN(DISTINCT a),    AGGFN(DISTINCT b)   ... => cannot use LIS
  etc.

  @param      join       the join to check
  @param[out] out_args   Collect the arguments of the aggregate functions
                         to a list. We don't worry about duplicates as
                         these will be sorted out later in
                         get_best_group_min_max.

  @return                does the query qualify for indexed AGGFN(DISTINCT)
    @retval   true       it does
    @retval   false      AGGFN(DISTINCT) must apply distinct in it.
*/

bool is_indexed_agg_distinct(JOIN *join, List<Item_field> *out_args) {
  Item_sum **sum_item_ptr;
  bool result = false;
  Field_map first_aggdistinct_fields;

  if (join->primary_tables > 1 ||            /* reference more than 1 table */
      join->select_distinct ||               /* or a DISTINCT */
      join->select_lex->olap == ROLLUP_TYPE) /* Check (B3) for ROLLUP */
    return false;

  if (join->make_sum_func_list(join->all_fields, join->fields_list, true))
    return false;

  for (sum_item_ptr = join->sum_funcs; *sum_item_ptr; sum_item_ptr++) {
    Item_sum *sum_item = *sum_item_ptr;
    Field_map cur_aggdistinct_fields;
    Item *expr;
    /* aggregate is not AGGFN(DISTINCT) or more than 1 argument to it */
    switch (sum_item->sum_func()) {
      case Item_sum::MIN_FUNC:
      case Item_sum::MAX_FUNC:
        continue;
      case Item_sum::COUNT_DISTINCT_FUNC:
        break;
      case Item_sum::AVG_DISTINCT_FUNC:
      case Item_sum::SUM_DISTINCT_FUNC:
        if (sum_item->get_arg_count() == 1) break;
      /* fall through */
      default:
        return false;
    }

    for (uint i = 0; i < sum_item->get_arg_count(); i++) {
      expr = sum_item->get_arg(i);
      /* The AGGFN(DISTINCT) arg is not an attribute? */
      if (expr->real_item()->type() != Item::FIELD_ITEM) return false;

      Item_field *item = static_cast<Item_field *>(expr->real_item());
      if (out_args) out_args->push_back(item);

      cur_aggdistinct_fields.set_bit(item->field->field_index);
      result = true;
    }
    /*
      If there are multiple aggregate functions, make sure that they all
      refer to exactly the same set of columns.
    */
    if (first_aggdistinct_fields.is_clear_all())
      first_aggdistinct_fields.merge(cur_aggdistinct_fields);
    else if (first_aggdistinct_fields != cur_aggdistinct_fields)
      return false;
  }

  return result;
}

/**
  Print keys that were appended to join_tab->const_keys because they
  can be used for GROUP BY or DISTINCT to the optimizer trace.

  @param trace     The optimizer trace context we're adding info to
  @param join_tab  The table the indexes cover
  @param new_keys  The keys that are considered useful because they can
                   be used for GROUP BY or DISTINCT
  @param cause     Zero-terminated string with reason for adding indexes
                   to const_keys

  @see add_group_and_distinct_keys()
 */
static void trace_indexes_added_group_distinct(Opt_trace_context *trace,
                                               const JOIN_TAB *join_tab,
                                               const Key_map new_keys,
                                               const char *cause) {
  if (likely(!trace->is_started())) return;

  KEY *key_info = join_tab->table()->key_info;
  Key_map existing_keys = join_tab->const_keys;
  uint nbrkeys = join_tab->table()->s->keys;

  Opt_trace_object trace_summary(trace, "const_keys_added");
  {
    Opt_trace_array trace_key(trace, "keys");
    for (uint j = 0; j < nbrkeys; j++)
      if (new_keys.is_set(j) && !existing_keys.is_set(j))
        trace_key.add_utf8(key_info[j].name);
  }
  trace_summary.add_alnum("cause", cause);
}

/**
  Discover the indexes that might be used for GROUP BY or DISTINCT queries or
  indexes that might be used for SKIP SCAN.

  If the query has a GROUP BY clause, find all indexes that contain
  all GROUP BY fields, and add those indexes to join_tab->const_keys
  and join_tab->keys.

  If the query has a DISTINCT clause, find all indexes that contain
  all SELECT fields, and add those indexes to join_tab->const_keys and
  join_tab->keys. This allows later on such queries to be processed by
  a QUICK_GROUP_MIN_MAX_SELECT.

  If the query does not have GROUP BY clause or any aggregate function
  the function collects possible keys to use for skip scan access.

  Note that indexes that are not usable for resolving GROUP
  BY/DISTINCT may also be added in some corner cases. For example, an
  index covering 'a' and 'b' is not usable for the following query but
  is still added: "SELECT DISTINCT a+b FROM t1". This is not a big
  issue because a) although the optimizer will consider using the
  index, it will not chose it (so minor calculation cost added but not
  wrong result) and b) it applies only to corner cases.

  @param join
  @param join_tab
*/

static void add_loose_index_scan_and_skip_scan_keys(JOIN *join,
                                                    JOIN_TAB *join_tab) {
  DBUG_ASSERT(join_tab->const_keys.is_subset(join_tab->keys()));

  List<Item_field> indexed_fields;
  List_iterator<Item_field> indexed_fields_it(indexed_fields);
  ORDER *cur_group;
  Item_field *cur_item;
  const char *cause;

  /* Find the indexes that might be used for skip scan queries. */
  if (hint_table_state(join->thd, join_tab->table_ref, SKIP_SCAN_HINT_ENUM,
                       OPTIMIZER_SKIP_SCAN) &&
      join->where_cond && join->primary_tables == 1 && !join->group_list &&
      !is_indexed_agg_distinct(join, &indexed_fields) &&
      !join->select_distinct) {
    Item::Collect_item_fields_with_item_func_in info(&indexed_fields);
    join->where_cond->walk(&Item::collect_item_field_with_item_func_in,
                           enum_walk::POSTFIX, (uchar *)&info);

    /*
      Skip scan considers all keys that containing any fields found in the WHERE
      clause, meaning that range analysis will examine a lot more keys due to
      skip scan. This can cause it to run out memory, and fall back to full
      table scan. To help mitigate this, do not consider skip scan keys if we
      find large IN lists.
    */
    if (join->thd->variables.optimizer_skip_scan_in_list_limit != 0 &&
        info.m_item_func_in_max_args >
            join->thd->variables.optimizer_skip_scan_in_list_limit) {
      push_warning_printf(
          join->thd, Sql_condition::SL_WARNING,
          ER_WARN_SKIP_SCAN_KEYS_NOT_CONSIDERED,
          ER_THD(join->thd, ER_WARN_SKIP_SCAN_KEYS_NOT_CONSIDERED),
          info.m_item_func_in_max_args);
      return;
    }

    Key_map possible_keys;
    possible_keys.set_all();
    join_tab->skip_scan_keys.clear_all();
    while ((cur_item = indexed_fields_it++)) {
      if (cur_item->used_tables() != join_tab->table_ref->map()) return;
      possible_keys.intersect(cur_item->field->part_of_key);
    }
    join_tab->skip_scan_keys.merge(possible_keys);

    return;
  }

  if (join->group_list) {
    /* Collect all query fields referenced in the GROUP clause. */
    for (cur_group = join->group_list; cur_group; cur_group = cur_group->next)
      (*cur_group->item)
          ->walk(&Item::collect_item_field_processor, enum_walk::POSTFIX,
                 (uchar *)&indexed_fields);
    cause = "group_by";
  } else if (join->select_distinct) {
    /* Collect all query fields referenced in the SELECT clause. */
    List<Item> &select_items = join->fields_list;
    List_iterator<Item> select_items_it(select_items);
    Item *item;
    while ((item = select_items_it++))
      item->walk(&Item::collect_item_field_processor, enum_walk::POSTFIX,
                 (uchar *)&indexed_fields);
    cause = "distinct";
  } else if (join->tmp_table_param.sum_func_count &&
             is_indexed_agg_distinct(join, &indexed_fields)) {
    /*
      SELECT list with AGGFN(distinct col). The query qualifies for
      loose index scan, and is_indexed_agg_distinct() has already
      collected all referenced fields into indexed_fields.
    */
    join->streaming_aggregation = true;
    cause = "indexed_distinct_aggregate";
  } else
    return;

  if (indexed_fields.elements == 0) return;

  Key_map possible_keys = join_tab->table()->keys_in_use_for_query;
  possible_keys.merge(join_tab->table()->keys_in_use_for_group_by);

  /* Intersect the keys of all group fields. */
  while ((cur_item = indexed_fields_it++)) {
    if (cur_item->used_tables() != join_tab->table_ref->map()) {
      /*
        Doing GROUP BY or DISTINCT on a field in another table so no
        index in this table is usable
      */
      return;
    } else
      possible_keys.intersect(cur_item->field->part_of_key);
  }

  /*
    At this point, possible_keys has key bits set only for usable
    indexes because indexed_fields is non-empty and if any of the
    fields belong to a different table the function would exit in the
    loop above.
  */

  if (!possible_keys.is_clear_all() &&
      !possible_keys.is_subset(join_tab->const_keys)) {
    trace_indexes_added_group_distinct(&join->thd->opt_trace, join_tab,
                                       possible_keys, cause);
    join_tab->const_keys.merge(possible_keys);
    join_tab->keys().merge(possible_keys);
  }

  DBUG_ASSERT(join_tab->const_keys.is_subset(join_tab->keys()));
}

/**
  Update keyuse array with all possible keys we can use to fetch rows.

  @param       thd            session context
  @param[out]  keyuse         Put here ordered array of Key_use structures
  @param       join_tab       Array in table number order
  @param       tables         Number of tables in join
  @param       cond           WHERE condition (note that the function analyzes
                              join_tab[i]->join_cond() too)
  @param       normal_tables  Tables not inner w.r.t some outer join (ones
                              for which we can make ref access based the WHERE
                              clause)
  @param       select_lex     current SELECT
  @param[out]  sargables      Array of found sargable candidates

  @returns false if success, true if error
*/

static bool update_ref_and_keys(THD *thd, Key_use_array *keyuse,
                                JOIN_TAB *join_tab, uint tables, Item *cond,
                                table_map normal_tables, SELECT_LEX *select_lex,
                                SARGABLE_PARAM **sargables) {
  DBUG_ASSERT(cond == nullptr || cond->is_bool_func());
  uint and_level, i, found_eq_constant;
  Key_field *key_fields, *end, *field;
  size_t sz;
  uint m = max(select_lex->max_equal_elems, 1U);
  JOIN *const join = select_lex->join;
  /*
    We use the same piece of memory to store both  Key_field
    and SARGABLE_PARAM structure.
    Key_field values are placed at the beginning this memory
    while  SARGABLE_PARAM values are put at the end.
    All predicates that are used to fill arrays of Key_field
    and SARGABLE_PARAM structures have at most 2 arguments
    except BETWEEN predicates that have 3 arguments and
    IN predicates.
    This any predicate if it's not BETWEEN/IN can be used
    directly to fill at most 2 array elements, either of Key_field
    or SARGABLE_PARAM type. For a BETWEEN predicate 3 elements
    can be filled as this predicate is considered as
    saragable with respect to each of its argument.
    An IN predicate can require at most 1 element as currently
    it is considered as sargable only for its first argument.
    Multiple equality can add  elements that are filled after
    substitution of field arguments by equal fields. There
    can be not more than select_lex->max_equal_elems such
    substitutions.
  */
  sz = max(sizeof(Key_field), sizeof(SARGABLE_PARAM)) *
       (((select_lex->cond_count + 1) * 2 + select_lex->between_count) * m + 1);
  if (!(key_fields = (Key_field *)thd->alloc(sz)))
    return true; /* purecov: inspected */
  and_level = 0;
  field = end = key_fields;
  *sargables = (SARGABLE_PARAM *)key_fields +
               (sz - sizeof((*sargables)[0].field)) / sizeof(SARGABLE_PARAM);
  /* set a barrier for the array of SARGABLE_PARAM */
  (*sargables)[0].field = nullptr;

  if (cond) {
    if (add_key_fields(thd, join, &end, &and_level, cond, normal_tables,
                       sargables))
      return true;
    for (Key_field *fld = field; fld != end; fld++) {
      /* Mark that we can optimize LEFT JOIN */
      if (fld->val->type() == Item::NULL_ITEM &&
          !fld->item_field->field->is_nullable()) {
        /*
          Example:
          SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.a WHERE t2.a IS NULL;
          this just wants rows of t1 where t1.a does not exist in t2.
        */
        fld->item_field->field->table->reginfo.not_exists_optimize = true;
      }
    }
  }

  for (i = 0; i < tables; i++) {
    /*
      Block the creation of keys for inner tables of outer joins.
      Here only the outer joins that can not be converted to
      inner joins are left and all nests that can be eliminated
      are flattened.
      In the future when we introduce conditional accesses
      for inner tables in outer joins these keys will be taken
      into account as well.
    */
    if (join_tab[i].join_cond()) {
      if (add_key_fields(thd, join, &end, &and_level, join_tab[i].join_cond(),
                         join_tab[i].table_ref->map(), sargables))
        return true;
    }
  }

  /* Process ON conditions for the nested joins */
  for (TABLE_LIST *tl : select_lex->top_join_list) {
    if (tl->nested_join &&
        add_key_fields_for_nj(thd, join, tl, &end, &and_level, sargables))
      return true;
  }

  /* Generate keys descriptions for derived tables */
  if (select_lex->materialized_derived_table_count ||
      select_lex->table_func_count) {
    if (join->generate_derived_keys()) return true;
  }
  /* fill keyuse with found key parts */
  for (; field != end; field++) {
    if (add_key_part(keyuse, field)) return true;
  }

  if (select_lex->ftfunc_list->elements) {
    if (add_ft_keys(keyuse, join_tab, cond, normal_tables, true)) return true;
  }

  /*
    Sort the array of possible keys and remove the following key parts:
    - ref if there is a keypart which is a ref and a const.
      (e.g. if there is a key(a,b) and the clause is a=3 and b=7 and b=t2.d,
      then we skip the key part corresponding to b=t2.d)
    - keyparts without previous keyparts
      (e.g. if there is a key(a,b,c) but only b < 5 (or a=2 and c < 3) is
      used in the query, we drop the partial key parts from consideration).
    Special treatment for ft-keys.
  */
  if (!keyuse->empty()) {
    Key_use *save_pos, *use;

    std::sort(keyuse->begin(), keyuse->begin() + keyuse->size(), sort_keyuse);

    const Key_use key_end(nullptr, nullptr, 0, 0, 0, 0, 0, 0, false, nullptr,
                          0);
    if (keyuse->push_back(key_end))  // added for easy testing
      return true;

    use = save_pos = keyuse->begin();
    const Key_use *prev = &key_end;
    found_eq_constant = 0;
    for (i = 0; i < keyuse->size() - 1; i++, use++) {
      TABLE *const table = use->table_ref->table;
      if (!use->used_tables && use->optimize != KEY_OPTIMIZE_REF_OR_NULL)
        table->const_key_parts[use->key] |= use->keypart_map;
      if (use->keypart != FT_KEYPART) {
        if (use->key == prev->key && use->table_ref == prev->table_ref) {
          if (prev->keypart + 1 < use->keypart ||
              (prev->keypart == use->keypart && found_eq_constant))
            continue;                  /* remove */
        } else if (use->keypart != 0)  // First found must be 0
          continue;
      }

      /*
        Protect against self assignment.
        The compiler *may* generate a call to memcpy() to do the assignment,
        and that is undefined behaviour (memory overlap).
       */
      if (save_pos != use) *save_pos = *use;
      prev = use;
      found_eq_constant = !use->used_tables;
      /* Save ptr to first use */
      if (!table->reginfo.join_tab->keyuse())
        table->reginfo.join_tab->set_keyuse(save_pos);
      table->reginfo.join_tab->checked_keys.set_bit(use->key);
      save_pos++;
    }
    i = (uint)(save_pos - keyuse->begin());
    keyuse->at(i) = key_end;
    keyuse->chop(i);
  }
  print_keyuse_array(&thd->opt_trace, keyuse);
  /*
    Number of functions here call val_x() methods, which might throw an error.
    Catch those errors here.
  */
  return thd->is_error();
}

/**
  Create a keyuse array for a table with a primary key.
  To be used when creating a materialized temporary table.

  @param thd         THD pointer, for memory allocation
  @param keyparts    Number of key parts in the primary key
  @param fields
  @param outer_exprs List of items used for key lookup

  @return Pointer to created keyuse array, or NULL if error
*/
Key_use_array *create_keyuse_for_table(THD *thd, uint keyparts,
                                       Item_field **fields,
                                       List<Item> outer_exprs) {
  void *mem = thd->alloc(sizeof(Key_use_array));
  if (!mem) return nullptr;
  Key_use_array *keyuses = new (mem) Key_use_array(thd->mem_root);

  List_iterator<Item> outer_expr(outer_exprs);

  for (uint keypartno = 0; keypartno < keyparts; keypartno++) {
    Item *const item = outer_expr++;
    Key_field key_field(fields[keypartno], item, 0, 0, true,
                        // null_rejecting must be true for field items only,
                        // add_not_null_conds() is incapable of handling
                        // other item types.
                        (item->type() == Item::FIELD_ITEM), nullptr, UINT_MAX);
    if (add_key_part(keyuses, &key_field)) return nullptr;
  }
  const Key_use key_end(nullptr, nullptr, 0, 0, 0, 0, 0, 0, false, nullptr, 0);
  if (keyuses->push_back(key_end))  // added for easy testing
    return nullptr;

  return keyuses;
}

/**
  Move const tables first in the position array.

  Increment the number of const tables and set same basic properties for the
  const table.
  A const table looked up by a key has type JT_CONST.
  A const table with a single row has type JT_SYSTEM.

  @param tab    Table that is designated as a const table
  @param key    The key definition to use for this table (NULL if table scan)
*/

void JOIN::mark_const_table(JOIN_TAB *tab, Key_use *key) {
  POSITION *const position = positions + const_tables;
  position->table = tab;
  position->key = key;
  position->rows_fetched = 1.0;  // This is a const table
  position->filter_effect = 1.0;
  position->prefix_rowcount = 1.0;
  position->read_cost = 0.0;
  position->ref_depend_map = 0;
  position->loosescan_key = MAX_KEY;  // Not a LooseScan
  position->sj_strategy = SJ_OPT_NONE;
  positions->use_join_buffer = false;

  // Move the const table as far down as possible in best_ref
  JOIN_TAB **pos = best_ref + const_tables + 1;
  for (JOIN_TAB *next = best_ref[const_tables]; next != tab; pos++) {
    JOIN_TAB *const tmp = pos[0];
    pos[0] = next;
    next = tmp;
  }
  best_ref[const_tables] = tab;

  tab->set_type(key ? JT_CONST : JT_SYSTEM);

  const_table_map |= tab->table_ref->map();

  const_tables++;
}

void JOIN::make_outerjoin_info() {
  DBUG_TRACE;

  DBUG_ASSERT(select_lex->outer_join);
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  select_lex->reset_nj_counters();

  for (uint i = const_tables; i < tables; ++i) {
    JOIN_TAB *const tab = best_ref[i];
    TABLE *const table = tab->table();
    if (!table) continue;

    TABLE_LIST *const tbl = tab->table_ref;
    /*
      If 'tbl' is inside a SJ/AJ nest served by materialization, we must
      limit setting first_inner, last_inner and first_upper for join nests
      inside the materialized table. Indeed it is the SJ-tmp table, and not
      'tbl', which interacts with the nests outer to the SJ/AJ nest.
    */
    const bool sj_mat_inner =
        sj_is_materialize_strategy(tab->get_sj_strategy());

    if (tbl->outer_join) {
      /*
        Table tab is the only one inner table for outer join.
        (Like table t4 for the table reference t3 LEFT JOIN t4 ON t3.a=t4.a
        is in the query above.)
      */
      tab->set_last_inner(i);
      tab->set_first_inner(i);
      tab->init_join_cond_ref(tbl);
      tab->cond_equal = tbl->cond_equal;
      /*
        If this outer join nest is embedded in another join nest,
        link the join-tabs:
      */
      TABLE_LIST *const outer_join_nest = tbl->outer_join_nest();
      if (outer_join_nest) {
        DBUG_ASSERT(outer_join_nest->nested_join->first_nested != NO_PLAN_IDX);
        if (!sj_mat_inner ||
            (tab->emb_sj_nest->sj_inner_tables &
             best_ref[outer_join_nest->nested_join->first_nested]
                 ->table_ref->map()))
          tab->set_first_upper(outer_join_nest->nested_join->first_nested);
      }
    }
    for (TABLE_LIST *embedding = tbl->embedding; embedding;
         embedding = embedding->embedding) {
      // When reaching the outer tables of the materialized temporary table,
      // the decoration for this table is complete.
      if (sj_mat_inner && embedding == tab->emb_sj_nest) break;
      // Ignore join nests that are not outer join nests:
      if (!embedding->join_cond_optim()) continue;
      NESTED_JOIN *const nested_join = embedding->nested_join;
      if (!nested_join->nj_counter) {
        /*
          Table tab is the first inner table for nested_join.
          Save reference to it in the nested join structure.
        */
        nested_join->first_nested = i;
        // The table's condition is set to point to the join nest's condition
        tab->init_join_cond_ref(embedding);
        tab->cond_equal = tbl->cond_equal;

        TABLE_LIST *const outer_join_nest = embedding->outer_join_nest();
        if (outer_join_nest) {
          DBUG_ASSERT(outer_join_nest->nested_join->first_nested !=
                      NO_PLAN_IDX);
          if (!sj_mat_inner ||
              (tab->emb_sj_nest->sj_inner_tables &
               best_ref[outer_join_nest->nested_join->first_nested]
                   ->table_ref->map()))
            tab->set_first_upper(outer_join_nest->nested_join->first_nested);
        }
      }
      if (tab->first_inner() == NO_PLAN_IDX)
        tab->set_first_inner(nested_join->first_nested);
      /*
        If including the sj-mat tmp table, this also implicitely
        includes the inner tables of the sj-nest.
      */
      nested_join->nj_counter +=
          tab->sj_mat_exec() ? tab->sj_mat_exec()->table_count : 1;
      if (nested_join->nj_counter < nested_join->nj_total) break;
      // Table tab is the last inner table for nested join.
      best_ref[nested_join->first_nested]->set_last_inner(i);
    }
  }
}

/**
  Build a condition guarded by match variables for embedded outer joins.
  When generating a condition for a table as part of an outer join condition
  or the WHERE condition, the table in question may also be part of an
  embedded outer join. In such cases, the condition must be guarded by
  the match variable for this embedded outer join. Such embedded outer joins
  may also be recursively embedded in other joins.

  The function recursively adds guards for a condition ascending from tab
  to root_tab, which is the first inner table of an outer join,
  or NULL if the condition being handled is the WHERE clause.

  @param join
  @param idx       index of the first inner table for the inner-most outer join
  @param cond      the predicate to be guarded (must be set)
  @param root_idx  index of the inner table to stop at
                   (is NO_PLAN_IDX if this is the WHERE clause)

  @return
    -  pointer to the guarded predicate, if success
    -  NULL if error
*/

static Item *add_found_match_trig_cond(JOIN *join, plan_idx idx, Item *cond,
                                       plan_idx root_idx) {
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);
  DBUG_ASSERT(cond->is_bool_func());

  for (; idx != root_idx; idx = join->best_ref[idx]->first_upper()) {
    if (!(cond = new Item_func_trig_cond(cond, nullptr, join, idx,
                                         Item_func_trig_cond::FOUND_MATCH)))
      return nullptr;

    cond->quick_fix_field();
    cond->update_used_tables();
  }

  return cond;
}

/**
   Helper for JOIN::attach_join_conditions().
   Attaches bits of 'join_cond' to each table in the range [first_inner,
   last_tab], with proper guards.
   If 'sj_mat_cond' is true, we do not see first_inner (and tables on the same
   level of it) as inner to anything, as they're at the top from the POV of
   the materialization of the tmp table. So, if the SJ-mat nest is A LJ B,
   A will get a part of condition without any guard; B will get another part
   with a guard on A->found_match. It's like pushing a WHERE.
*/
bool JOIN::attach_join_condition_to_nest(plan_idx first_inner,
                                         plan_idx last_tab, Item *join_cond,
                                         bool is_sj_mat_cond) {
  /*
    Add the constant part of the join condition to the first inner table
    of the outer join.
  */
  Item *cond =
      make_cond_for_table(thd, join_cond, const_table_map, table_map(0), false);
  if (cond) {
    if (!is_sj_mat_cond) {
      cond = new Item_func_trig_cond(cond, nullptr, this, first_inner,
                                     Item_func_trig_cond::IS_NOT_NULL_COMPL);
      if (!cond) return true;
      if (cond->fix_fields(thd, nullptr)) return true;
    }
    if (best_ref[first_inner]->and_with_condition(cond)) return true;
  }
  /*
    Split the non-constant part of the join condition into parts that
    can be attached to the inner tables of the outer join.
  */
  for (plan_idx i = first_inner; i <= last_tab; ++i) {
    table_map prefix_tables = best_ref[i]->prefix_tables();
    table_map added_tables = best_ref[i]->added_tables();

    /*
      When handling the first inner table of an outer join, we may also
      reference all tables ahead of this table:
    */
    if (i == first_inner) added_tables = prefix_tables;
    /*
      We need RAND_TABLE_BIT on the last inner table, in case there is a
      non-deterministic function in the join condition.
      (RAND_TABLE_BIT is set for the last table of the join plan,
      but this is not sufficient for join conditions, which may have a
      last inner table that is ahead of the last table of the join plan).
    */
    if (i == last_tab) {
      prefix_tables |= RAND_TABLE_BIT;
      added_tables |= RAND_TABLE_BIT;
    }
    cond =
        make_cond_for_table(thd, join_cond, prefix_tables, added_tables, false);
    if (cond == nullptr) continue;
    /*
      If the table is part of an outer join that is embedded in the
      outer join currently being processed, wrap the condition in
      triggered conditions for match variables of such embedded outer joins.
    */
    if (!(cond = add_found_match_trig_cond(
              this, best_ref[i]->first_inner(), cond,
              is_sj_mat_cond ? NO_PLAN_IDX : first_inner)))
      return true;

    if (!is_sj_mat_cond) {
      // Add the guard turning the predicate off for the null-complemented row.
      cond = new Item_func_trig_cond(cond, nullptr, this, first_inner,
                                     Item_func_trig_cond::IS_NOT_NULL_COMPL);
      if (!cond) return true;
      if (cond->fix_fields(thd, nullptr)) return true;
    }
    // Add the generated condition to the existing table condition
    if (best_ref[i]->and_with_condition(cond)) return true;
  }
  return false;
}

/**
  Attach outer join conditions to generated table conditions in an optimal way.

  @param last_tab - Last table that has been added to the current plan.
                    Pre-condition: If this is the last inner table of an outer
                    join operation, a join condition is attached to the first
                    inner table of that outer join operation.

  @return false if success, true if error.

  Outer join conditions are attached to individual tables, but we can analyze
  those conditions only when reaching the last inner table of an outer join
  operation. Notice also that a table can be last within several outer join
  nests, hence the outer for() loop of this function.

  Example:
    SELECT * FROM t1 LEFT JOIN (t2 LEFT JOIN t3 ON t2.a=t3.a) ON t1.a=t2.a

    Table t3 is last both in the join nest (t2 - t3) and in (t1 - (t2 - t3))
    Thus, join conditions for both join nests will be evaluated when reaching
    this table.

  For each outer join operation processed, the join condition is split
  optimally over the inner tables of the outer join. The split-out conditions
  are later referred to as table conditions (but note that several table
  conditions stemming from different join operations may be combined into
  a composite table condition).

  Example:
    Consider the above query once more.
    The predicate t1.a=t2.a can be evaluated when rows from t1 and t2 are ready,
    ie at table t2. The predicate t2.a=t3.a can be evaluated at table t3.

  Each non-constant split-out table condition is guarded by a match variable
  that enables it only when a matching row is found for all the embedded
  outer join operations.

  Each split-out table condition is guarded by a variable that turns the
  condition off just before a null-complemented row for the outer join
  operation is formed. Thus, the join condition will not be checked for
  the null-complemented row.
*/

bool JOIN::attach_join_conditions(plan_idx last_tab) {
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  JOIN_TAB *lt = best_ref[last_tab];

  for (plan_idx first_inner = lt->first_inner();
       first_inner != NO_PLAN_IDX &&
       best_ref[first_inner]->last_inner() == last_tab;
       first_inner = best_ref[first_inner]->first_upper()) {
    /*
      Table last_tab is the last inner table of an outer join, locate
      the corresponding join condition from the first inner table of the
      same outer join:
    */
    Item *const join_cond = best_ref[first_inner]->join_cond();
    DBUG_ASSERT(join_cond);
    if (attach_join_condition_to_nest(first_inner, last_tab, join_cond, false))
      return true;
  }
  if (sj_is_materialize_strategy(lt->get_sj_strategy())) {
    plan_idx mat_tbl = NO_PLAN_IDX;
    /*
      The SJ nest's condition contains both the SJ equality condition and the
       WHERE of the replaced subquery. This WHERE must be pushed to SJ-inner
       tables for evaluation during materialization!
    */
    Semijoin_mat_exec *sjm = nullptr;
    for (plan_idx j = last_tab;; j--) {
      sjm = best_ref[j]->sj_mat_exec();
      if (sjm && sjm->sj_nest == lt->emb_sj_nest) {
        // 'j' is the sj-mat tmp table
        mat_tbl = j;
        break;
      }
    }
    DBUG_ASSERT(sjm);
    if (sjm->inner_table_index + sjm->table_count - 1 == (uint)last_tab) {
      // we're at last table of sjmat nest
      auto join_cond = best_ref[mat_tbl]->join_cond();
      if (join_cond && attach_join_condition_to_nest(sjm->inner_table_index,
                                                     last_tab, join_cond, true))
        return true;
    }
  }

  /*
    See if 'last_tab' is the first inner of an antijoin nest,
    then add a IS NULL condition on it.
    By attaching the condition to the first inner table, we know that if
    it is not satisfied we can just jump back to the table right before
    it.
  */
  if (lt->table_ref->embedding && lt->table_ref->embedding->is_aj_nest() &&
      last_tab == lt->first_inner() &&
      /*
        Exception: in A AJ (B LJ C) where C is a single table: there is no
        join nest for C as it's single; C->embedding is thus the AJ nest; but
        C->first_inner() is C (as it's the first inner of the LJ operation).
        In that case it's not the first inner table of the AJ.
        Catch this case:
      */
      !lt->table_ref->join_cond()) {
    Item *cond = new Item_func_false();
    if (!cond) return true;
    // This is a signal for JOIN::create_iterators
    cond->item_name.set(antijoin_null_cond);
    /*
      For A AJ B ON COND, we need an IS NULL condition which
      is tested on the result rows of A LEFT JOIN B ON COND.
      It must be tested only after the "match status" of a row of B has been
      decided, so is wrapped in a condition triggered by B->found_match.
      To have it test IS NULL, it's wrapped in a triggered condition which is
      false if B is not NULL-complemented.
      We needn't wrap this condition with triggers from upper nests, hence the
      last argument of the call below.
    */
    cond = add_found_match_trig_cond(this, last_tab, cond, lt->first_upper());
    if (!cond) return true;
    cond = new Item_func_trig_cond(cond, nullptr, this, last_tab,
                                   Item_func_trig_cond::IS_NOT_NULL_COMPL);
    if (!cond) return true;
    if (cond->fix_fields(thd, nullptr)) return true;
    if (lt->and_with_condition(cond)) return true;
    lt->table()->reginfo.not_exists_optimize = true;
  }

  return false;
}

/*****************************************************************************
  Remove calculation with tables that aren't yet read. Remove also tests
  against fields that are read through key where the table is not a
  outer join table.
  We can't remove tests that are made against columns which are stored
  in sorted order.
*****************************************************************************/

static Item *part_of_refkey(TABLE *table, TABLE_REF *ref, const Field *field) {
  uint ref_parts = ref->key_parts;
  if (ref_parts) {
    if (ref->has_guarded_conds()) return nullptr;

    const KEY_PART_INFO *key_part = table->key_info[ref->key].key_part;

    for (uint part = 0; part < ref_parts; part++, key_part++)
      if (field->eq(key_part->field) &&
          !(key_part->key_part_flag & HA_PART_KEY_SEG))
        return ref->items[part];
  }
  return nullptr;
}

/**
  @brief
  Identify redundant predicates.

  @details
  Test if the equality predicate 'left_item = right_item' is redundant
  due to a REF-access already being set up on the table, where 'left_item' is
  part of the REF-key being used, and 'right_item' is equal to the key value
  specified for that field in the key.
  In such cases the predicate is known to be 'true' for any rows retrieved
  from that table. Thus it is redundant.

  @param left_item   The Item_field possibly being part of A ref-KEY.
  @param right_item  The equality value specified for 'left_item'.

  @return  'true' if the predicate is redundant.

  @note See comments in reduce_cond_for_table() about careful
  usage/modifications of test_if_ref().
*/

static bool test_if_ref(Item_field *left_item, Item *right_item) {
  if (left_item->depended_from)
    return false;  // don't even read join_tab of inner subquery!
  Field *field = left_item->field;
  JOIN_TAB *join_tab = field->table->reginfo.join_tab;
  if (join_tab == nullptr) return false;

  ASSERT_BEST_REF_IN_JOIN_ORDER(join_tab->join());

  // No need to change const test
  if (!field->table->const_table &&
      /* "ref_or_null" implements "x=y or x is null", not "x=y" */
      (join_tab->type() != JT_REF_OR_NULL)) {
    Item *ref_item = part_of_refkey(field->table, &join_tab->ref(), field);
    if (ref_item && ref_item->eq(right_item, true)) {
      right_item = right_item->real_item();
      if (right_item->type() == Item::FIELD_ITEM)
        return (field->eq_def(down_cast<Item_field *>(right_item)->field));
      /* remove equalities injected by IN->EXISTS transformation */
      else if (right_item->type() == Item::CACHE_ITEM)
        return down_cast<Item_cache *>(right_item)->eq_def(field);
      if (right_item->const_for_execution() && !(right_item->is_null())) {
        /*
          We can remove all fields except:
          1. String data types:
           - For BINARY/VARBINARY fields with equality against a
             string: Ref access can return more rows than match the
             string. The reason seems to be that the string constant
             is not "padded" to the full length of the field when
             setting up ref access. @todo Change how ref access for
             BINARY/VARBINARY fields are done so that only qualifying
             rows are returned from the storage engine.
          2. Float data type: Comparison of float can differ
           - When we search "WHERE field=value" using an index,
             the "value" side is converted from double to float by
             Field_float::store(), then two floats are compared.
           - When we search "WHERE field=value" without indexes,
             the "field" side is converted from float to double by
             Field_float::val_real(), then two doubles are compared.
        */
        if (field->type() == MYSQL_TYPE_STRING &&
            field->charset()->pad_attribute == NO_PAD) {
          /*
            For "NO PAD" collations on CHAR columns, this function must return
            false, because removal of trailing space in CHAR columns makes the
            table value and the index value compare differently. As the column
            strips trailing spaces, it can return false candidates. Further
            comparison of the actual table values is required.
           */
          return false;
        }
        if (!((field->type() == MYSQL_TYPE_STRING ||  // 1
               field->type() == MYSQL_TYPE_VARCHAR) &&
              field->binary()) &&
            !(field->type() == MYSQL_TYPE_FLOAT && field->decimals() > 0))  // 2
        {
          return !right_item->save_in_field_no_warnings(field, true);
        }
      }
    }
  }
  return false;  // keep predicate
}

/**
  @brief
  Remove redundant predicates from condition, return the reduced condition.

  @details
  A predicate of the form 'field = value' may be redundant if the
  (ref-) access choosen for the table use an index containing 'field',
  where 'value' is specified as (part of) its ref-key. This method remove
  such redundant predicates, thus reducing the condition, possibly
  eliminating it entirely.

  If comparing 'values' against outer-joined tables, these are possibly
  'null-extended'. Thus the usage of these values in the ref-key, is not
  sufficient anymore to guarantee that 'field = value' is 'TRUE'.
  The 'null_extended' argument hold the table_map of any such possibly
  null-extended tables which are excluded from the above 'reduce' logic.

  Any tables referred in Item_func_trig_cond(FOUND_MATCH) conditions are
  aggregated into this null_extended table_map.

  @param cond           The condition to be 'reduced'.
  @param null_extended  table_map of possibly null-extended outer-tables.

  @return               The condition with redundant predicates removed,
                        possibly nullptr.
*/
static Item *reduce_cond_for_table(Item *cond, table_map null_extended) {
  DBUG_TRACE;
  DBUG_EXECUTE("where",
               print_where(current_thd, cond, "cond term", QT_ORDINARY););

  if (cond->type() == Item::COND_ITEM) {
    List<Item> *arguments = down_cast<Item_cond *>(cond)->argument_list();
    List_iterator<Item> li(*arguments);
    if (down_cast<Item_cond *>(cond)->functype() == Item_func::COND_AND_FUNC) {
      Item *item;
      while ((item = li++)) {
        Item *upd_item = reduce_cond_for_table(item, null_extended);
        if (upd_item == nullptr) {
          li.remove();
        } else if (upd_item != item) {
          li.replace(upd_item);
        }
      }
      switch (arguments->elements) {
        case 0:
          return nullptr;  // All 'true' -> And-cond true
        case 1:
          return arguments->head();
      }
    } else {  // Or list
      Item *item;
      while ((item = li++)) {
        Item *upd_item = reduce_cond_for_table(item, null_extended);
        if (upd_item == nullptr) {
          return nullptr;  // Term 'true' -> entire Or-cond true
        } else if (upd_item != item) {
          li.replace(upd_item);
        }
      }
    }
  } else if (cond->type() == Item::FUNC_ITEM) {
    Item_func *func = down_cast<Item_func *>(cond);
    if (func->functype() == Item_func::TRIG_COND_FUNC) {
      Item_func_trig_cond *func_trig = down_cast<Item_func_trig_cond *>(func);
      if (func_trig->get_trig_type() == Item_func_trig_cond::FOUND_MATCH) {
        /*
          All inner-tables are possible null-extended when evaluating
          the 'FOUND_MATCH'. Thus, predicates embedded in this trig_cond,
          refering these tables, should not be eliminated.
          -> Add to null_extended map.
        */
        null_extended |= func_trig->get_inner_tables();
      }

      Item *cond_arg = func->arguments()[0];
      Item *upd_arg = reduce_cond_for_table(cond_arg, null_extended);
      if (upd_arg == nullptr) {
        return nullptr;
      }
      func->arguments()[0] = upd_arg;
    }

    else if (func->functype() == Item_func::EQ_FUNC) {
      /*
        Remove equalities that are guaranteed to be true by use of 'ref' access
        method.
        Note that ref access implements "table1.field1 <=>
        table2.indexed_field2", i.e. if it passed a NULL field1, it will return
        NULL indexed_field2 if there are.
        Thus the equality "table1.field1 = table2.indexed_field2",
        is equivalent to "ref access AND table1.field1 IS NOT NULL"
        i.e. "ref access and proper setting/testing of ref->null_rejecting".
        Thus, we must be careful, that when we remove equalities below we also
        set ref->null_rejecting, and test it at execution; otherwise wrong NULL
        matches appear.
        So:
        - for the optimization phase, the code which is below, and the code in
        test_if_ref(), and in add_key_field(), must be kept in sync: if the
        applicability conditions in one place are relaxed, they should also be
        relaxed elsewhere.
        - for the execution phase, all possible execution methods must test
        ref->null_rejecting.
      */
      Item *left_item = func->arguments()[0]->real_item();
      Item *right_item = func->arguments()[1]->real_item();
      if ((left_item->type() == Item::FIELD_ITEM &&
           !(left_item->used_tables() & null_extended) &&
           test_if_ref(down_cast<Item_field *>(left_item), right_item)) ||
          (right_item->type() == Item::FIELD_ITEM &&
           !(right_item->used_tables() & null_extended) &&
           test_if_ref(down_cast<Item_field *>(right_item), left_item))) {
        return nullptr;
      }
    }
  }
  return cond;
}

/**
  @brief
  Remove redundant predicates and cache constant expressions.

  @details
  Do a final round on pushed down table conditions and HAVING
  clause. Optimize them for faster execution by removing
  predicates being obsolete due to the access path selected
  for the table. Constant expressions are also cached
  to avoid evaluating them for each row being compared.

  @return False if success, True if error

  @note This function is run after conditions have been pushed down to
        individual tables, so transformation is applied to JOIN_TAB::condition
        and not to the WHERE condition.
*/
bool JOIN::finalize_table_conditions() {
  /*
    Unnecessary to reduce conditions for const tables as they are only
    evaluated once.
  */
  DBUG_ASSERT(!plan_is_const());
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_array trace_tables(trace, "finalizing_table_conditions");

  for (uint i = const_tables; i < tables; i++) {
    Item *condition = best_ref[i]->condition();
    if (condition == nullptr) continue;

    /*
      Table predicates known to be true by the selected
      (ref-)access method may be removed from the condition
    */
    Opt_trace_object trace_cond(trace);
    trace_cond.add_utf8_table(best_ref[i]->table_ref);
    trace_cond.add("original_table_condition", condition);

    /*
      Calculate the set of possibly NULL extended tables when 'condition'
      is evaluated. As it is evaluated on a found row from table, that
      table is subtracted from the nullable tables. Note that a FOUND_MATCH
      trigger is a special case, handled in reduce_cond_for_table().
    */
    const table_map null_extended =
        select_lex->outer_join & ~best_ref[i]->table_ref->map();
    condition = reduce_cond_for_table(condition, null_extended);
    if (condition != nullptr) condition->update_used_tables();

    /*
      Cache constant expressions in table conditions.
      (Moved down from WHERE- and ON-clauses)
    */
    if (condition != nullptr) {
      cache_const_expr_arg cache_arg;
      cache_const_expr_arg *analyzer_arg = &cache_arg;
      condition = condition->compile(
          &Item::cache_const_expr_analyzer, (uchar **)&analyzer_arg,
          &Item::cache_const_expr_transformer, (uchar *)&cache_arg);
      if (condition == nullptr) return true;
    }

    trace_cond.add("final_table_condition   ", condition);
    best_ref[i]->set_condition(condition);
  }

  /* Cache constant expressions in HAVING-clauses. */
  if (having_cond != nullptr) {
    cache_const_expr_arg cache_arg;
    cache_const_expr_arg *analyzer_arg = &cache_arg;
    having_cond = having_cond->compile(
        &Item::cache_const_expr_analyzer, (uchar **)&analyzer_arg,
        &Item::cache_const_expr_transformer, (uchar *)&cache_arg);
    if (having_cond == nullptr) return true;
  }
  return false;
}

/**
  @brief
  Add keys to derived tables'/views' result tables in a list

  @details
  This function generates keys for all derived tables/views of the select_lex
  to which this join corresponds to with help of the TABLE_LIST:generate_keys
  function.

  @return false all keys were successfully added.
  @return true OOM error
*/

bool JOIN::generate_derived_keys() {
  DBUG_ASSERT(select_lex->materialized_derived_table_count ||
              select_lex->table_func_count);

  for (TABLE_LIST *table = select_lex->leaf_tables; table;
       table = table->next_leaf) {
    table->derived_keys_ready = true;
    /* Process tables that aren't materialized yet. */
    if (table->uses_materialization() && !table->table->is_created() &&
        table->generate_keys())
      return true;
  }
  return false;
}

/**
  For each materialized derived table/view, informs every TABLE of the key it
  will (not) use, segregates used keys from unused keys in TABLE::key_info,
  and eliminates unused keys.
*/

void JOIN::finalize_derived_keys() {
  DBUG_ASSERT(select_lex->materialized_derived_table_count ||
              select_lex->table_func_count);
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  bool adjust_key_count = false;
  table_map processed_tables = 0;

  for (uint i = 0; i < tables; i++) {
    JOIN_TAB *tab = best_ref[i];
    TABLE *table = tab->table();
    TABLE_LIST *table_ref = tab->table_ref;
    /*
     Save chosen key description if:
     1) it's a materialized derived table
     2) it's not yet instantiated
     3) some keys are defined for it
    */
    if (table && table_ref->uses_materialization() &&  // (1)
        !table->is_created() &&                        // (2)
        table->s->keys > 0)                            // (3)
    {
      /*
        If there are two local references to the same CTE, and they use
        the same key, the iteration for the second reference is unnecessary.
      */
      if (processed_tables & table_ref->map()) continue;

      adjust_key_count = true;

      Key_map used_keys;

      // Mark all unique indexes as in use, since they have an effect
      // (deduplication) whether any expression refers to them or not.
      // In particular, they are used if we want to materialize a UNION DISTINCT
      // directly into the derived table.
      for (uint key_idx = 0; key_idx < table->s->keys; ++key_idx) {
        if (table->key_info[key_idx].flags & HA_NOSAME) {
          used_keys.set_bit(key_idx);
        }
      }

      // Same for the hash key used for manual deduplication, if any. (It always
      // has index 0 if it exists.)
      if (table->hash_field) {
        used_keys.set_bit(0);
      }

      Key_use *const keyuse = tab->position()->key;
      if (keyuse == nullptr && used_keys.is_clear_all()) {
        // Nothing uses any keys.
        tab->keys().clear_all();
        tab->const_keys.clear_all();
        continue;
      }

      Derived_refs_iterator it(table_ref);
      while (TABLE *t = it.get_next()) {
        /*
          Eliminate possible keys created by this JOIN and which it
          doesn't use.
          Collect all keys of this table which are used by any reference in
          this query block. Any other query block doesn't matter as:
          - either it was optimized before, so it's not using a key we may
          want to drop.
          - or it was optimized in this same window, so:
            * either we own the window, then any key we may want to
            drop is not visible to it.
            * or it owns the window, then we are using only existing
            keys.
          - or it will be optimized after, so it's not using any key yet.

          used_keys is a mix of possible used keys and existing used keys.
        */
        if (t->pos_in_table_list->select_lex == select_lex) {
          JOIN_TAB *jtab = t->reginfo.join_tab;
          Key_use *keyuse_1 = jtab->position()->key;
          if (keyuse_1) used_keys.set_bit(keyuse_1->key);
        }
      }

      uint new_idx = table->s->find_first_unused_tmp_key(
          used_keys);  // Also updates table->s->first_unused_tmp_key.
      if (keyuse == nullptr) {
        continue;
      }

      const uint old_idx = keyuse->key;
      DBUG_ASSERT(old_idx != new_idx);

      if (old_idx > new_idx) {
        DBUG_ASSERT(table->s->owner_of_possible_tmp_keys == select_lex);
        it.rewind();
        while (TABLE *t = it.get_next()) {
          /*
            Unlike the collection of used_keys, references from other query
            blocks must be considered here, as they need a key_info array
            consistent with the to-be-changed table->s->keys.
          */
          t->copy_tmp_key(old_idx, it.is_first());
        }
      } else
        new_idx = old_idx;  // Index stays at same slot

      /*
        If the key was created by earlier-optimized query blocks, and is
        already used by nonlocal references, those don't need any further
        update: they are already setup to use it and we're not moving the
        key.
        If the key was created by this query block, nonlocal references cannot
        possibly be referencing it.
        In both cases, only local references need to update their Key_use.
      */
      it.rewind();
      while (TABLE *t = it.get_next()) {
        if (t->pos_in_table_list->select_lex != select_lex) continue;
        JOIN_TAB *jtab = t->reginfo.join_tab;
        Key_use *keyuse_1 = jtab->position()->key;
        if (keyuse_1 && keyuse_1->key == old_idx) {
          processed_tables |= t->pos_in_table_list->map();
          const bool key_is_const = jtab->const_keys.is_set(old_idx);
          // tab->keys() was never set, so must be set
          jtab->keys().clear_all();
          jtab->keys().set_bit(new_idx);
          jtab->const_keys.clear_all();
          if (key_is_const) tab->const_keys.set_bit(new_idx);
          for (Key_use *kit = keyuse_1;
               kit->table_ref == jtab->table_ref && kit->key == old_idx; kit++)
            kit->key = new_idx;
        }
      }
    }
  }

  if (!adjust_key_count) return;

  // Finally we know how many keys remain in the table.
  for (uint i = 0; i < tables; i++) {
    JOIN_TAB *tab = best_ref[i];
    TABLE *table = tab->table();
    TABLE_LIST *table_ref = tab->table_ref;
    if (table && table_ref->uses_materialization() && !table->is_created() &&
        table->s->keys > 0) {
      if (table->s->owner_of_possible_tmp_keys != select_lex) continue;
      /*
        Release lock. As a bonus, avoid double work when this loop
        later processes another local reference to the same table (similar to
        the processed_tables map in the first loop).
      */
      table->s->owner_of_possible_tmp_keys = nullptr;
      Derived_refs_iterator it(table_ref);
      while (TABLE *t = it.get_next()) t->drop_unused_tmp_keys(it.is_first());
    }
  }
}

/**
  @brief
  Extract a condition that can be checked after reading given table

  @param thd        Current session.
  @param cond       Condition to analyze
  @param tables     Tables for which "current field values" are available
  @param used_table Table(s) that we are extracting the condition for (may
                    also include PSEUDO_TABLE_BITS, and may be zero)
  @param exclude_expensive_cond  Do not push expensive conditions

  @retval <>NULL Generated condition
  @retval = NULL Already checked, OR error

  @details
    Extract the condition that can be checked after reading the table(s)
    specified in @c used_table, given that current-field values for tables
    specified in @c tables bitmap are available.
    If @c used_table is 0, extract conditions for all tables in @c tables.

    This function can be used to extract conditions relevant for a table
    in a join order. Together with its caller, it will ensure that all
    conditions are attached to the first table in the join order where all
    necessary fields are available, and it will also ensure that a given
    condition is attached to only one table.
    To accomplish this, first initialize @c tables to the empty
    set. Then, loop over all tables in the join order, set @c used_table to
    the bit representing the current table, accumulate @c used_table into the
    @c tables set, and call this function. To ensure correct handling of
    const expressions and outer references, add the const table map and
    OUTER_REF_TABLE_BIT to @c used_table for the first table. To ensure
    that random expressions are evaluated for the final table, add
    RAND_TABLE_BIT to @c used_table for the final table.

    The function assumes that constant, inexpensive parts of the condition
    have already been checked. Constant, expensive parts will be attached
    to the first table in the join order, provided that the above call
    sequence is followed.

    The call order will ensure that conditions covering tables in @c tables
    minus those in @c used_table, have already been checked.

    The function takes into account that some parts of the condition are
    guaranteed to be true by employed 'ref' access methods (the code that
    does this is located at the end, search down for "EQ_FUNC").

  @note
    make_cond_for_info_schema() uses an algorithm similar to
    make_cond_for_table().
*/

Item *make_cond_for_table(THD *thd, Item *cond, table_map tables,
                          table_map used_table, bool exclude_expensive_cond) {
  /*
    May encounter an Item_cache_int as "condition" here, so cannot
    assert that it satisfies is_bool_func().
  */
  /*
    Ignore this condition if
     1. We are extracting conditions for a specific table, and
     2. that table is not referenced by the condition, but not if
     3. this is a constant condition not checked at optimization time and
        this is the first table we are extracting conditions for.
       (Assuming that used_table == tables for the first table.)
  */
  if (used_table &&                                     // 1
      !(cond->used_tables() & used_table) &&            // 2
      !(cond->is_expensive() && used_table == tables))  // 3
    return nullptr;

  if (cond->type() == Item::COND_ITEM) {
    if (((Item_cond *)cond)->functype() == Item_func::COND_AND_FUNC) {
      /* Create new top level AND item */
      Item_cond_and *new_cond = new Item_cond_and;
      if (!new_cond) return nullptr;
      List_iterator<Item> li(*((Item_cond *)cond)->argument_list());
      Item *item;
      while ((item = li++)) {
        Item *fix = make_cond_for_table(thd, item, tables, used_table,
                                        exclude_expensive_cond);
        if (fix) new_cond->argument_list()->push_back(fix);
      }
      switch (new_cond->argument_list()->elements) {
        case 0:
          return nullptr;  // Always true
        case 1:
          return new_cond->argument_list()->head();
        default:
          if (new_cond->fix_fields(thd, nullptr)) return nullptr;
          return new_cond;
      }
    } else {  // Or list
      Item_cond_or *new_cond = new Item_cond_or;
      if (!new_cond) return nullptr;
      List_iterator<Item> li(*((Item_cond *)cond)->argument_list());
      Item *item;
      while ((item = li++)) {
        Item *fix = make_cond_for_table(thd, item, tables, table_map(0),
                                        exclude_expensive_cond);
        if (!fix) return nullptr;  // Always true
        new_cond->argument_list()->push_back(fix);
      }
      if (new_cond->fix_fields(thd, nullptr)) return nullptr;
      return new_cond;
    }
  }

  /*
    Omit this condition if
     1. Some tables referred by the condition are not available, or
     2. We are extracting conditions for all tables, the condition is
        considered 'expensive', and we want to delay evaluation of such
        conditions to the execution phase.
  */
  if ((cond->used_tables() & ~tables) ||                                // 1
      (!used_table && exclude_expensive_cond && cond->is_expensive()))  // 2
    return nullptr;

  return cond;
}

/**
  Separates the predicates in a join condition and pushes them to the
  join step where all involved tables are available in the join prefix.
  ON clauses from JOIN expressions are also pushed to the most appropriate step.

  @param join Join object where predicates are pushed.

  @param cond Pointer to condition which may contain an arbitrary number of
              predicates, combined using AND, OR and XOR items.
              If NULL, equivalent to a predicate that returns true for all
              row combinations.


  @retval true  Found impossible WHERE clause, or out-of-memory
  @retval false Other
*/

static bool make_join_select(JOIN *join, Item *cond) {
  DBUG_ASSERT(cond == nullptr || cond->is_bool_func());
  THD *thd = join->thd;
  Opt_trace_context *const trace = &thd->opt_trace;
  DBUG_TRACE;
  ASSERT_BEST_REF_IN_JOIN_ORDER(join);

  // Add IS NOT NULL conditions to table conditions:
  add_not_null_conds(join);

  /*
    Extract constant conditions that are part of the WHERE clause.
    Constant parts of join conditions from outer joins are attached to
    the appropriate table condition in JOIN::attach_join_conditions().
  */
  if (cond) /* Because of QUICK_GROUP_MIN_MAX_SELECT */
  {         /* there may be a select without a cond. */
    if (join->primary_tables > 1)
      cond->update_used_tables();  // Table number may have changed
    if (join->plan_is_const() &&
        join->select_lex->master_unit() ==
            thd->lex->unit)  // The outer-most query block
      join->const_table_map |= RAND_TABLE_BIT;
  }
  /*
    Extract conditions that depend on constant tables.
    The const part of the query's WHERE clause can be checked immediately
    and if it is not satisfied then the join has empty result
  */
  Item *const_cond = nullptr;
  if (cond)
    const_cond = make_cond_for_table(thd, cond, join->const_table_map,
                                     table_map(0), true);

  // Add conditions added by add_not_null_conds()
  for (uint i = 0; i < join->const_tables; i++) {
    if (and_conditions(&const_cond, join->best_ref[i]->condition()))
      return true;
  }
  DBUG_EXECUTE("where",
               print_where(thd, const_cond, "constants", QT_ORDINARY););
  if (const_cond != nullptr &&
      evaluate_during_optimization(const_cond, join->select_lex)) {
    const bool const_cond_result = const_cond->val_int() != 0;
    if (thd->is_error()) return true;

    Opt_trace_object trace_const_cond(trace);
    trace_const_cond.add("condition_on_constant_tables", const_cond)
        .add("condition_value", const_cond_result);
    if (const_cond_result) {
      /*
        If all the tables referred by the condition are const tables and
        if the condition is not expensive, we can remove the where condition
        as it will always evaluate to "true".
      */
      if (join->plan_is_const() &&
          !(cond->used_tables() & ~join->const_table_map) &&
          !cond->is_expensive()) {
        DBUG_PRINT("info", ("Found always true WHERE condition"));
        join->where_cond = nullptr;
      }
    } else {
      DBUG_PRINT("info", ("Found impossible WHERE condition"));
      return true;
    }
  }

  /*
    Extract remaining conditions from WHERE clause and join conditions,
    and attach them to the most appropriate table condition. This means that
    a condition will be evaluated as soon as all fields it depends on are
    available. For outer join conditions, the additional criterion is that
    we must have determined whether outer-joined rows are available, or
    have been NULL-extended, see JOIN::attach_join_conditions() for details.
  */
  {
    Opt_trace_object trace_wrapper(trace);
    Opt_trace_object trace_conditions(trace, "attaching_conditions_to_tables");
    trace_conditions.add("original_condition", cond);
    Opt_trace_array trace_attached_comp(trace,
                                        "attached_conditions_computation");

    for (uint i = join->const_tables; i < join->tables; i++) {
      JOIN_TAB *const tab = join->best_ref[i];

      if (!tab->position()) continue;
      /*
        first_inner is the X in queries like:
        SELECT * FROM t1 LEFT OUTER JOIN (t2 JOIN t3) ON X
      */
      const plan_idx first_inner = tab->first_inner();
      const table_map used_tables = tab->prefix_tables();
      const table_map current_map = tab->added_tables();
      Item *tmp = nullptr;

      if (cond)
        tmp = make_cond_for_table(thd, cond, used_tables, current_map, false);
      /* Add conditions added by add_not_null_conds(). */
      if (tab->condition() && and_conditions(&tmp, tab->condition()))
        return true;

      if (cond && !tmp && tab->quick()) {  // Outer join
        DBUG_ASSERT(tab->type() == JT_RANGE || tab->type() == JT_INDEX_MERGE);
        /*
          Hack to handle the case where we only refer to a table
          in the ON part of an OUTER JOIN. In this case we want the code
          below to check if we should use 'quick' instead.
        */
        DBUG_PRINT("info", ("Item_func_true"));
        tmp = new Item_func_true();  // Always true
      }
      if (tmp || !cond || tab->type() == JT_REF ||
          tab->type() == JT_REF_OR_NULL || tab->type() == JT_EQ_REF ||
          first_inner != NO_PLAN_IDX) {
        DBUG_EXECUTE("where",
                     print_where(thd, tmp, tab->table()->alias, QT_ORDINARY););
        /*
          If tab is an inner table of an outer join operation,
          add a match guard to the pushed down predicate.
          The guard will turn the predicate on only after
          the first match for outer tables is encountered.
        */
        if (cond && tmp) {
          /*
            Because of QUICK_GROUP_MIN_MAX_SELECT there may be a select without
            a cond, so neutralize the hack above.
          */
          if (!(tmp = add_found_match_trig_cond(join, first_inner, tmp,
                                                NO_PLAN_IDX)))
            return true;
          tab->set_condition(tmp);
        } else {
          tab->set_condition(nullptr);
        }

        DBUG_EXECUTE("where",
                     print_where(thd, tmp, tab->table()->alias, QT_ORDINARY););

        if (tab->quick()) {
          if (tab->needed_reg.is_clear_all() && tab->type() != JT_CONST) {
            /*
              We keep (for now) the QUICK AM calculated in
              get_quick_record_count().
            */
            DBUG_ASSERT(tab->quick()->is_valid());
          } else {
            delete tab->quick();
            tab->set_quick(nullptr);
          }
        }

        if ((tab->type() == JT_ALL || tab->type() == JT_RANGE ||
             tab->type() == JT_INDEX_MERGE || tab->type() == JT_INDEX_SCAN) &&
            tab->use_quick != QS_RANGE) {
          /*
            We plan to scan (table/index/range scan).
            Check again if we should use an index. We can use an index if:

            1a) There is a condition that range optimizer can work on, and
            1b) There are non-constant conditions on one or more keys, and
            1c) Some of the non-constant fields may have been read
                already. This may be the case if this is not the first
                table in the join OR this is a subselect with
                non-constant conditions referring to an outer table
                (dependent subquery)
                or,
            2a) There are conditions only relying on constants
            2b) This is the first non-constant table
            2c) There is a limit of rows to read that is lower than
                the fanout for this table, predicate filters included
                (i.e., the estimated number of rows that will be
                produced for this table per row combination of
                previous tables)
            2d) The query is NOT run with FOUND_ROWS() (because in that
                case we have to scan through all rows to count them anyway)
          */
          enum {
            DONT_RECHECK,
            NOT_FIRST_TABLE,
            LOW_LIMIT
          } recheck_reason = DONT_RECHECK;

          DBUG_ASSERT(tab->const_keys.is_subset(tab->keys()));

          const join_type orig_join_type = tab->type();
          const QUICK_SELECT_I *const orig_quick = tab->quick();

          if (cond &&                              // 1a
              (tab->keys() != tab->const_keys) &&  // 1b
              (i > 0 ||                            // 1c
               (join->select_lex->master_unit()->item &&
                cond->used_tables() & OUTER_REF_TABLE_BIT)))
            recheck_reason = NOT_FIRST_TABLE;
          else if (!tab->const_keys.is_clear_all() &&  // 2a
                   i == join->const_tables &&          // 2b
                   (join->unit->select_limit_cnt <
                    (tab->position()->rows_fetched *
                     tab->position()->filter_effect)) &&  // 2c
                   !join->calc_found_rows)                // 2d
            recheck_reason = LOW_LIMIT;

          // Don't recheck if the storage engine does not support index access.
          if ((tab->table()->file->ha_table_flags() & HA_NO_INDEX_ACCESS) != 0)
            recheck_reason = DONT_RECHECK;

          if (tab->position()->sj_strategy == SJ_OPT_LOOSE_SCAN) {
            /*
              Semijoin loose scan has settled for a certain index-based access
              method with suitable characteristics, don't substitute it.
            */
            recheck_reason = DONT_RECHECK;
          }

          if (recheck_reason != DONT_RECHECK) {
            Opt_trace_object trace_one_table(trace);
            trace_one_table.add_utf8_table(tab->table_ref);
            Opt_trace_object trace_table(trace, "rechecking_index_usage");
            if (recheck_reason == NOT_FIRST_TABLE)
              trace_table.add_alnum("recheck_reason", "not_first_table");
            else
              trace_table.add_alnum("recheck_reason", "low_limit")
                  .add("limit", join->unit->select_limit_cnt)
                  .add("row_estimate", tab->position()->rows_fetched *
                                           tab->position()->filter_effect);

            /* Join with outer join condition */
            Item *orig_cond = tab->condition();
            tab->and_with_condition(tab->join_cond());

            /*
              We can't call sel->cond->fix_fields,
              as it will break tab->join_cond() if it's AND condition
              (fix_fields currently removes extra AND/OR levels).
              Yet attributes of the just built condition are not needed.
              Thus we call sel->cond->quick_fix_field for safety.
            */
            if (tab->condition() && !tab->condition()->fixed)
              tab->condition()->quick_fix_field();

            Key_map usable_keys = tab->keys();
            enum_order interesting_order = ORDER_NOT_RELEVANT;

            if (recheck_reason == LOW_LIMIT) {
              int read_direction = 0;

              /*
                If the current plan is to use range, then check if the
                already selected index provides the order dictated by the
                ORDER BY clause.
              */
              if (tab->quick() && tab->quick()->index != MAX_KEY) {
                const uint ref_key = tab->quick()->index;
                bool skip_quick;
                read_direction = test_if_order_by_key(
                    &join->order, tab->table(), ref_key, nullptr, &skip_quick);
                if (skip_quick) read_direction = 0;
                /*
                  If the index provides order there is no need to recheck
                  index usage; we already know from the former call to
                  test_quick_select() that a range scan on the chosen
                  index is cheapest. Note that previous calls to
                  test_quick_select() did not take order direction
                  (ASC/DESC) into account, so in case of DESC ordering
                  we still need to recheck.
                */
                if ((read_direction == 1) ||
                    (read_direction == -1 && tab->quick()->reverse_sorted())) {
                  recheck_reason = DONT_RECHECK;
                }
              }
              // We do a cost based search for an ordering index here. Do this
              // only if prefer_ordering_index switch is on or an index is
              // forced for order by
              if (recheck_reason != DONT_RECHECK &&
                  (tab->table()->force_index_order ||
                   thd->optimizer_switch_flag(
                       OPTIMIZER_SWITCH_PREFER_ORDERING_INDEX))) {
                int best_key = -1;
                ha_rows select_limit = join->unit->select_limit_cnt;

                /* Use index specified in FORCE INDEX FOR ORDER BY, if any. */
                if (tab->table()->force_index_order)
                  usable_keys.intersect(tab->table()->keys_in_use_for_order_by);

                // Do a cost based search on the indexes that give sort order.
                test_if_cheaper_ordering(
                    tab, &join->order, tab->table(), usable_keys, -1,
                    select_limit, &best_key, &read_direction, &select_limit);
                if (best_key < 0)
                  recheck_reason = DONT_RECHECK;  // No usable keys
                else {
                  // Only usable_key is the best_key chosen
                  usable_keys.clear_all();
                  usable_keys.set_bit(best_key);
                  interesting_order =
                      (read_direction == -1 ? ORDER_DESC : ORDER_ASC);
                }
              }
            }

            bool search_if_impossible = recheck_reason != DONT_RECHECK;
            if (search_if_impossible) {
              if (tab->quick()) {
                delete tab->quick();
                tab->set_type(JT_ALL);
              }
              QUICK_SELECT_I *qck;
              search_if_impossible =
                  test_quick_select(
                      thd, usable_keys, used_tables & ~tab->table_ref->map(),
                      join->calc_found_rows ? HA_POS_ERROR
                                            : join->unit->select_limit_cnt,
                      false,  // don't force quick range
                      interesting_order, tab, tab->condition(),
                      &tab->needed_reg, &qck, tab->table()->force_index) < 0;
              tab->set_quick(qck);
            }
            tab->set_condition(orig_cond);
            if (search_if_impossible) {
              /*
                Before reporting "Impossible WHERE" for the whole query
                we have to check isn't it only "impossible ON" instead
              */
              if (!tab->join_cond())
                return true;  // No ON, so it's really "impossible WHERE"
              Opt_trace_object trace_without_on(trace, "without_ON_clause");
              if (tab->quick()) {
                delete tab->quick();
                tab->set_type(JT_ALL);
              }
              QUICK_SELECT_I *qck;
              const bool impossible_where =
                  test_quick_select(
                      thd, tab->keys(), used_tables & ~tab->table_ref->map(),
                      join->calc_found_rows ? HA_POS_ERROR
                                            : join->unit->select_limit_cnt,
                      false,  // don't force quick range
                      ORDER_NOT_RELEVANT, tab, tab->condition(),
                      &tab->needed_reg, &qck, tab->table()->force_index) < 0;
              tab->set_quick(qck);
              if (impossible_where) return true;  // Impossible WHERE
            }

            /*
              Access method changed. This is after deciding join order
              and access method for all other tables so the info
              updated below will not have any effect on the execution
              plan.
            */
            if (tab->quick())
              tab->set_type(calc_join_type(tab->quick()->get_type()));

          }  // end of "if (recheck_reason != DONT_RECHECK)"

          if (!tab->table()->quick_keys.is_subset(tab->checked_keys) ||
              !tab->needed_reg.is_subset(tab->checked_keys)) {
            tab->keys().merge(tab->table()->quick_keys);
            tab->keys().merge(tab->needed_reg);

            /*
              The logic below for assigning tab->use_quick is strange.
              It bases the decision of which access method to use
              (dynamic range, range, scan) based on seemingly
              unrelated information like the presense of another index
              with too bad selectivity to be used.

              Consider the following scenario:

              The join optimizer has decided to use join order
              (t1,t2), and 'tab' is currently t2. Further, assume that
              there is a join condition between t1 and t2 using some
              range operator (e.g. "t1.x < t2.y").

              It has been decided that a table scan is best for t2.
              make_join_select() then reran the range optimizer a few
              lines up because there is an index 't2.good_idx'
              covering the t2.y column. If 'good_idx' is the only
              index in t2, the decision below will be to use dynamic
              range. However, if t2 also has another index 't2.other'
              which the range access method can be used on but
              selectivity is bad (#rows estimate is high), then table
              scan is chosen instead.

              Thus, the choice of DYNAMIC RANGE vs SCAN depends on the
              presense of an index that has so bad selectivity that it
              will not be used anyway.
            */
            if (!tab->needed_reg.is_clear_all() &&
                (tab->table()->quick_keys.is_clear_all() ||
                 (tab->quick() && (tab->quick()->records >= 100L)))) {
              tab->use_quick = QS_DYNAMIC_RANGE;
              tab->set_type(JT_ALL);
            } else
              tab->use_quick = QS_RANGE;
          }

          if (tab->type() != orig_join_type ||
              tab->quick() != orig_quick)  // Access method changed
            tab->position()->filter_effect = COND_FILTER_STALE;
        }
      }

      if (join->attach_join_conditions(i)) return true;
    }
    trace_attached_comp.end();

    /*
      In outer joins the loop above, in iteration for table #i, may push
      conditions to a table before #i. Thus, the processing below has to be in
      a separate loop:
    */
    Opt_trace_array trace_attached_summary(trace,
                                           "attached_conditions_summary");
    for (uint i = join->const_tables; i < join->tables; i++) {
      JOIN_TAB *const tab = join->best_ref[i];
      if (!tab->table()) continue;
      Item *const tab_cond = tab->condition();
      Opt_trace_object trace_one_table(trace);
      trace_one_table.add_utf8_table(tab->table_ref).add("attached", tab_cond);
      if (tab_cond && tab_cond->has_subquery())  // traverse only if needed
      {
        /*
          Why we pass walk_subquery=false: imagine
          WHERE t1.col IN (SELECT * FROM t2
                             WHERE t2.col IN (SELECT * FROM t3)
          and tab==t1. The grandchild subquery (SELECT * FROM t3) should not
          be marked as "in condition of t1" but as "in condition of t2", for
          correct calculation of the number of its executions.
        */
        std::pair<SELECT_LEX *, int> pair_object(join->select_lex, i);
        tab_cond->walk(&Item::inform_item_in_cond_of_tab, enum_walk::POSTFIX,
                       pointer_cast<uchar *>(&pair_object));
      }
    }
  }
  return false;
}

/**
  Remove the following expressions from ORDER BY and GROUP BY:
  Constant expressions @n
  Expression that only uses tables that are of type EQ_REF and the reference
  is in the ORDER list or if all refereed tables are of the above type.

  In the following, the X field can be removed:
  @code
  SELECT * FROM t1,t2 WHERE t1.a=t2.a ORDER BY t1.a,t2.X
  SELECT * FROM t1,t2,t3 WHERE t1.a=t2.a AND t2.b=t3.b ORDER BY t1.a,t3.X
  @endcode

  These can't be optimized:
  @code
  SELECT * FROM t1,t2 WHERE t1.a=t2.a ORDER BY t2.X,t1.a
  SELECT * FROM t1,t2 WHERE t1.a=t2.a AND t1.b=t2.b ORDER BY t1.a,t2.c
  SELECT * FROM t1,t2 WHERE t1.a=t2.a ORDER BY t2.b,t1.a
  @endcode

  @param  join         join object
  @param  start_order  clause being analyzed (ORDER BY, GROUP BY...)
  @param  tab          table
  @param  cached_eq_ref_tables  bitmap: bit Z is set if the table of map Z
  was already the subject of an eq_ref_table() call for the same clause; then
  the return value of this previous call can be found at bit Z of
  'eq_ref_tables'
  @param  eq_ref_tables see above.
*/

static bool eq_ref_table(JOIN *join, ORDER *start_order, JOIN_TAB *tab,
                         table_map *cached_eq_ref_tables,
                         table_map *eq_ref_tables) {
  /* We can skip const tables only if not an outer table */
  if (tab->type() == JT_CONST && tab->first_inner() == NO_PLAN_IDX) return true;
  if (tab->type() != JT_EQ_REF || tab->table()->is_nullable()) return false;

  const table_map map = tab->table_ref->map();
  uint found = 0;

  for (Item **ref_item = tab->ref().items,
            **end = ref_item + tab->ref().key_parts;
       ref_item != end; ref_item++) {
    if (!(*ref_item)->const_item()) {  // Not a const ref
      ORDER *order;
      for (order = start_order; order; order = order->next) {
        if ((*ref_item)->eq(order->item[0], false)) break;
      }
      if (order) {
        if (!(order->used & map)) {
          found++;
          order->used |= map;
        }
        continue;  // Used in ORDER BY
      }
      if (!only_eq_ref_tables(join, start_order, (*ref_item)->used_tables(),
                              cached_eq_ref_tables, eq_ref_tables))
        return false;
    }
  }
  /* Check that there was no reference to table before sort order */
  for (; found && start_order; start_order = start_order->next) {
    if (start_order->used & map) {
      found--;
      continue;
    }
    if (start_order->depend_map & map) return false;
  }
  return true;
}

/// @see eq_ref_table()
static bool only_eq_ref_tables(JOIN *join, ORDER *order, table_map tables,
                               table_map *cached_eq_ref_tables,
                               table_map *eq_ref_tables) {
  tables &= ~PSEUDO_TABLE_BITS;
  for (JOIN_TAB **tab = join->map2table; tables; tab++, tables >>= 1) {
    if (tables & 1) {
      const table_map map = (*tab)->table_ref->map();
      bool is_eq_ref;
      if (*cached_eq_ref_tables & map)  // then there exists a cached bit
        is_eq_ref = *eq_ref_tables & map;
      else {
        is_eq_ref = eq_ref_table(join, order, *tab, cached_eq_ref_tables,
                                 eq_ref_tables);
        if (is_eq_ref)
          *eq_ref_tables |= map;
        else
          *eq_ref_tables &= ~map;
        *cached_eq_ref_tables |= map;  // now there exists a cached bit
      }
      if (!is_eq_ref) return false;
    }
  }
  return true;
}

/**
  Check if an expression in ORDER BY or GROUP BY is a duplicate of a
  preceding expression.

  @param  first_order   the first expression in the ORDER BY or
                        GROUP BY clause
  @param  possible_dup  the expression that might be a duplicate of
                        another expression preceding it the ORDER BY
                        or GROUP BY clause

  @returns true if possible_dup is a duplicate, false otherwise
*/
static bool duplicate_order(const ORDER *first_order,
                            const ORDER *possible_dup) {
  const ORDER *order;
  for (order = first_order; order; order = order->next) {
    if (order == possible_dup) {
      // all expressions preceding possible_dup have been checked.
      return false;
    } else {
      const Item *it1 = order->item[0]->real_item();
      const Item *it2 = possible_dup->item[0]->real_item();

      if (it1->eq(it2, false)) return true;
    }
  }
  return false;
}

/**
  Remove all constants and check if ORDER only contains simple
  expressions.

  simple_order is set to 1 if sort_order only uses fields from head table
  and the head table is not a LEFT JOIN table.

  @param first_order            List of SORT or GROUP order
  @param cond                   WHERE statement
  @param change_list            Set to 1 if we should remove things from list.
                                If this is not set, then only simple_order is
                                calculated.
  @param simple_order           Set to 1 if we are only using simple expressions
  @param group_by               True if order represents a grouping operation

  @return
    Returns new sort order
*/

ORDER *JOIN::remove_const(ORDER *first_order, Item *cond, bool change_list,
                          bool *simple_order, bool group_by) {
  DBUG_TRACE;

  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  if (plan_is_const())
    return change_list ? nullptr : first_order;  // No need to sort

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_disable_I_S trace_disabled(trace, first_order == nullptr);
  Opt_trace_object trace_simpl(
      trace, group_by ? "simplifying_group_by" : "simplifying_order_by");
  if (trace->is_started()) {
    String str;
    SELECT_LEX::print_order(
        thd, &str, first_order,
        enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                        QT_NO_DEFAULT_DB));
    trace_simpl.add_utf8("original_clause", str.ptr(), str.length());
  }
  Opt_trace_array trace_each_item(trace, "items");

  ORDER *order, **prev_ptr;
  JOIN_TAB *const first_tab = best_ref[const_tables];
  table_map first_table = first_tab->table_ref->map();
  table_map not_const_tables = ~const_table_map;
  table_map ref;
  // Caches to avoid repeating eq_ref_table() calls, @see eq_ref_table()
  table_map eq_ref_tables = 0, cached_eq_ref_tables = 0;

  prev_ptr = &first_order;
  *simple_order = !first_tab->join_cond();

  // De-optimization in conjunction with window functions
  if (group_by && m_windows.elements > 0) *simple_order = false;

  update_depend_map(first_order);

  for (order = first_order; order; order = order->next) {
    Opt_trace_object trace_one_item(trace);
    trace_one_item.add("item", order->item[0]);
    table_map order_tables = order->item[0]->used_tables();

    if (order->item[0]->has_aggregation() || order->item[0]->has_wf() ||
        /*
          If the outer table of an outer join is const (either by itself or
          after applying WHERE condition), grouping on a field from such a
          table will be optimized away and filesort without temporary table
          will be used unless we prevent that now. Filesort is not fit to
          handle joins and the join condition is not applied. We can't detect
          the case without an expensive test, however, so we force temporary
          table for all queries containing more than one table, ROLLUP, and an
          outer join.
         */
        (primary_tables > 1 && rollup.state == ROLLUP::STATE_INITED &&
         select_lex->outer_join))
      *simple_order = false;  // Must do a temp table to sort
    else if ((order_tables & not_const_tables) == 0 &&
             evaluate_during_optimization(order->item[0], select_lex)) {
      if (order->item[0]->has_subquery()) {
        if (!thd->lex->is_explain()) {
          Opt_trace_array trace_subselect(trace, "subselect_evaluation");
          String str;
          order->item[0]->val_str(&str);
        }
        order->item[0]->mark_subqueries_optimized_away();
      }
      trace_one_item.add("uses_only_constant_tables", true);
      continue;  // skip const item
    } else if (duplicate_order(first_order, order)) {
      /*
        If 'order' is a duplicate of an expression earlier in the
        ORDER/GROUP BY sequence, it can be removed from the ORDER BY
        or GROUP BY clause.
      */
      trace_one_item.add("duplicate_item", true);
      continue;
    } else if (order->in_field_list && order->item[0]->has_subquery())
      /*
        If the order item is a subquery that is also in the field
        list, a temp table should be used to avoid evaluating the
        subquery for each row both when a) creating a sort index and
        b) getting the value.
          Example: "SELECT (SELECT ... ) as a ... GROUP BY a;"
       */
      *simple_order = false;
    else {
      if (order_tables & (RAND_TABLE_BIT | OUTER_REF_TABLE_BIT))
        *simple_order = false;
      else {
        if (cond && const_expression_in_where(cond, order->item[0])) {
          trace_one_item.add("equals_constant_in_where", true);
          continue;
        }
        if ((ref = order_tables & (not_const_tables ^ first_table))) {
          if (!(order_tables & first_table) &&
              only_eq_ref_tables(this, first_order, ref, &cached_eq_ref_tables,
                                 &eq_ref_tables)) {
            trace_one_item.add("eq_ref_to_preceding_items", true);
            continue;
          }
          *simple_order = false;  // Must do a temp table to sort
        }
      }
    }
    if (change_list) *prev_ptr = order;  // use this entry
    prev_ptr = &order->next;
  }
  if (change_list) *prev_ptr = nullptr;
  if (prev_ptr == &first_order)  // Nothing to sort/group
    *simple_order = true;
  DBUG_PRINT("exit", ("simple_order: %d", (int)*simple_order));

  trace_each_item.end();
  trace_simpl.add("resulting_clause_is_simple", *simple_order);
  if (trace->is_started() && change_list) {
    String str;
    SELECT_LEX::print_order(
        thd, &str, first_order,
        enum_query_type(QT_TO_SYSTEM_CHARSET | QT_SHOW_SELECT_NUMBER |
                        QT_NO_DEFAULT_DB));
    trace_simpl.add_utf8("resulting_clause", str.ptr(), str.length());
  }

  return first_order;
}

/**
  Optimize conditions by

     a) applying transitivity to build multiple equality predicates
        (MEP): if x=y and y=z the MEP x=y=z is built.
     b) apply constants where possible. If the value of x is known to be
        42, x is replaced with a constant of value 42. By transitivity, this
        also applies to MEPs, so the MEP in a) will become 42=x=y=z.
     c) remove conditions that are always false or always true

  @param thd                Thread handler
  @param[in,out] cond       WHERE or HAVING condition to optimize
  @param[out] cond_equal    The built multiple equalities
  @param join_list          list of join operations with join conditions
                            = NULL: Called for HAVING condition
  @param[out] cond_value    Not changed if cond was empty
                              COND_TRUE if cond is always true
                              COND_FALSE if cond is impossible
                              COND_OK otherwise


  @returns false if success, true if error
*/

bool optimize_cond(THD *thd, Item **cond, COND_EQUAL **cond_equal,
                   mem_root_deque<TABLE_LIST *> *join_list,
                   Item::cond_result *cond_value) {
  DBUG_TRACE;
  Opt_trace_context *const trace = &thd->opt_trace;

  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_cond(trace, "condition_processing");
  trace_cond.add_alnum("condition", join_list ? "WHERE" : "HAVING");
  trace_cond.add("original_condition", *cond);
  Opt_trace_array trace_steps(trace, "steps");

  /*
    Enter this function
    a) For a WHERE condition or a query having outer join.
    b) For a HAVING condition.
  */
  DBUG_ASSERT(*cond || join_list);

  /*
    Build all multiple equality predicates and eliminate equality
    predicates that can be inferred from these multiple equalities.
    For each reference of a field included into a multiple equality
    that occurs in a function set a pointer to the multiple equality
    predicate. Substitute a constant instead of this field if the
    multiple equality contains a constant.
    This is performed for the WHERE condition and any join conditions, but
    not for the HAVING condition.
  */
  if (join_list) {
    Opt_trace_object step_wrapper(trace);
    step_wrapper.add_alnum("transformation", "equality_propagation");
    {
      Opt_trace_disable_I_S disable_trace_wrapper(
          trace, !(*cond && (*cond)->has_subquery()));
      Opt_trace_array trace_subselect(trace, "subselect_evaluation");
      if (build_equal_items(thd, *cond, cond, nullptr, true, join_list,
                            cond_equal))
        return true;
    }
    step_wrapper.add("resulting_condition", *cond);
  }
  /* change field = field to field = const for each found field = const */
  if (*cond) {
    Opt_trace_object step_wrapper(trace);
    step_wrapper.add_alnum("transformation", "constant_propagation");
    {
      Opt_trace_disable_I_S disable_trace_wrapper(trace,
                                                  !(*cond)->has_subquery());
      Opt_trace_array trace_subselect(trace, "subselect_evaluation");
      if (propagate_cond_constants(thd, nullptr, *cond, *cond)) return true;
    }
    step_wrapper.add("resulting_condition", *cond);
  }

  /*
    Remove all instances of item == item
    Remove all and-levels where CONST item != CONST item
  */
  DBUG_EXECUTE("where",
               print_where(thd, *cond, "after const change", QT_ORDINARY););
  if (*cond) {
    Opt_trace_object step_wrapper(trace);
    step_wrapper.add_alnum("transformation", "trivial_condition_removal");
    {
      Opt_trace_disable_I_S disable_trace_wrapper(trace,
                                                  !(*cond)->has_subquery());
      Opt_trace_array trace_subselect(trace, "subselect_evaluation");
      if (remove_eq_conds(thd, *cond, cond, cond_value)) return true;
    }
    step_wrapper.add("resulting_condition", *cond);
  }
  if (thd->is_error()) return true;
  return false;
}

/**
  Checks if a condition can be evaluated during constant folding. It can be
  evaluated if it is constant during execution and not expensive to evaluate. If
  it contains a subquery, it should not be evaluated if the option
  OPTION_NO_SUBQUERY_DURING_OPTIMIZATION is active.
*/
static bool can_evaluate_condition(THD *thd, Item *condition) {
  return condition->const_for_execution() && !condition->is_expensive() &&
         evaluate_during_optimization(condition, thd->lex->current_select());
}

/**
  Calls fold_condition. If that made the condition constant for execution,
  simplify and fold again. @see fold_condition() for arguments.
*/
static bool fold_condition_exec(THD *thd, Item *cond, Item **retcond,
                                Item::cond_result *cond_value) {
  if (fold_condition(thd, cond, retcond, cond_value)) return true;
  if (*retcond != nullptr &&
      can_evaluate_condition(thd, *retcond))  // simplify further maybe
    return remove_eq_conds(thd, *retcond, retcond, cond_value);
  return false;
}

/**
  Removes const and eq items. Returns the new item, or nullptr if no condition.

  @param      thd        thread handler
  @param      cond       the condition to handle
  @param[out] retcond    condition after const removal
  @param[out] cond_value resulting value of the condition
              =COND_OK    condition must be evaluated (e.g. field = constant)
              =COND_TRUE  always true                 (e.g. 1 = 1)
              =COND_FALSE always false                (e.g. 1 = 2)

  @returns false if success, true if error
*/
bool remove_eq_conds(THD *thd, Item *cond, Item **retcond,
                     Item::cond_result *cond_value) {
  DBUG_ASSERT(cond->real_item()->is_bool_func());
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *const item_cond = down_cast<Item_cond *>(cond);
    const bool and_level = item_cond->functype() == Item_func::COND_AND_FUNC;
    List_iterator<Item> li(*item_cond->argument_list());
    bool should_fix_fields = false;
    *cond_value = Item::COND_UNDEF;
    Item *item;
    while ((item = li++)) {
      Item *new_item;
      Item::cond_result tmp_cond_value;
      if (remove_eq_conds(thd, item, &new_item, &tmp_cond_value)) return true;

      if (new_item == nullptr)
        li.remove();
      else if (item != new_item) {
        (void)li.replace(new_item);
        should_fix_fields = true;
      }
      if (*cond_value == Item::COND_UNDEF) *cond_value = tmp_cond_value;
      switch (tmp_cond_value) {
        case Item::COND_OK:  // Not true or false
          if (and_level || *cond_value == Item::COND_FALSE)
            *cond_value = tmp_cond_value;
          break;
        case Item::COND_FALSE:
          if (and_level)  // Always false
          {
            *cond_value = tmp_cond_value;
            *retcond = nullptr;
            return false;
          }
          break;
        case Item::COND_TRUE:
          if (!and_level)  // Always true
          {
            *cond_value = tmp_cond_value;
            *retcond = nullptr;
            return false;
          }
          break;
        case Item::COND_UNDEF:  // Impossible
          DBUG_ASSERT(false);   /* purecov: deadcode */
      }
    }
    if (should_fix_fields) item_cond->update_used_tables();

    if (item_cond->argument_list()->elements == 0 ||
        *cond_value != Item::COND_OK) {
      *retcond = nullptr;
      return false;
    }
    if (item_cond->argument_list()->elements == 1) {
      /*
        BUG#11765699:
        We're dealing with an AND or OR item that has only one
        argument. However, it is not an option to empty the list
        because:

         - this function is called for either JOIN::conds or
           JOIN::having, but these point to the same condition as
           SELECT_LEX::where and SELECT_LEX::having do.

         - The return value of remove_eq_conds() is assigned to
           JOIN::conds and JOIN::having, so emptying the list and
           returning the only remaining item "replaces" the AND or OR
           with item for the variables in JOIN. However, the return
           value is not assigned to the SELECT_LEX counterparts. Thus,
           if argument_list is emptied, SELECT_LEX forgets the item in
           argument_list()->head().

        item is therefore returned, but argument_list is not emptied.
      */
      item = item_cond->argument_list()->head();
      /*
        Consider reenabling the line below when the optimizer has been
        split into properly separated phases.

        item_cond->argument_list()->empty();
      */
      *retcond = item;
      return false;
    }
  } else if (can_evaluate_condition(thd, cond)) {
    bool value;
    if (eval_const_cond(thd, cond, &value)) return true;
    *cond_value = value ? Item::COND_TRUE : Item::COND_FALSE;
    *retcond = nullptr;
    return false;
  } else {  // Boolean compare function
    *cond_value = cond->eq_cmp_result();
    if (*cond_value == Item::COND_OK) {
      return fold_condition_exec(thd, cond, retcond, cond_value);
    }
    Item *left_item = down_cast<Item_func *>(cond)->arguments()[0];
    Item *right_item = down_cast<Item_func *>(cond)->arguments()[1];
    if (left_item->eq(right_item, true) &&
        (cond->used_tables() & RAND_TABLE_BIT) == 0) {
      /*
       Two identical items are being compared:
       1) If the items are not nullable, return result from eq_cmp_result(),
          that is, we can short circuit because result is statically always
          known to be true or false, depending on which operator we are
          dealing with. If the operator allows equality, *cond_value is
          Item::COND_TRUE (a non-null value is always equal to itself), else
          Item::COND_FALSE (a non-null value is never unequal to itself).
       2) If the items are nullable and the result from eq_cmp_result() is
          false, result is always false, that is, the operator doesn't
          allow for equality, the result is always false: Any non-null
          value cannot obviously be unequal to itself, and any NULL value
          would yield an undefined result (e.g. NULL < NULL
          is undefined), and hence Item::COND_FALSE in this context is the
          effective result.
          (Call order ensures test is not applied to conditions with explicit
          truth value test)
       3) If the <=> operator is used, result is always true because
          NULL = NULL is true for this operator
      */
      if (!left_item->maybe_null || *cond_value == Item::COND_FALSE ||
          down_cast<Item_func *>(cond)->functype() == Item_func::EQUAL_FUNC) {
        *retcond = nullptr;
        return false;
      }
    }
  }
  return fold_condition_exec(thd, cond, retcond, cond_value);
}

/**
  Check if GROUP BY/DISTINCT can be optimized away because the set is
  already known to be distinct.

  Used in removing the GROUP BY/DISTINCT of the following types of
  statements:
  @code
    SELECT [DISTINCT] <unique_key_cols>... FROM <single_table_ref>
      [GROUP BY <unique_key_cols>,...]
  @endcode

    If (a,b,c is distinct)
    then <any combination of a,b,c>,{whatever} is also distinct

    This function checks if all the key parts of any of the unique keys
    of the table are referenced by a list : either the select list
    through find_field_in_item_list or GROUP BY list through
    find_field_in_order_list.
    If the above holds and the key parts cannot contain NULLs then we
    can safely remove the GROUP BY/DISTINCT,
    as no result set can be more distinct than an unique key.

  @param tab                  The join table to operate on.
  @param find_func            function to iterate over the list and search
                              for a field
  @param data

  @retval
    1                    found
  @retval
    0                    not found.

  @note
    The function assumes that make_outerjoin_info() has been called in
    order for the check for outer tables to work.
*/

static bool list_contains_unique_index(JOIN_TAB *tab,
                                       bool (*find_func)(Field *, void *),
                                       void *data) {
  TABLE *table = tab->table();

  if (tab->is_inner_table_of_outer_join()) return false;
  for (uint keynr = 0; keynr < table->s->keys; keynr++) {
    if (keynr == table->s->primary_key ||
        (table->key_info[keynr].flags & HA_NOSAME)) {
      KEY *keyinfo = table->key_info + keynr;
      KEY_PART_INFO *key_part, *key_part_end;

      for (key_part = keyinfo->key_part,
          key_part_end = key_part + keyinfo->user_defined_key_parts;
           key_part < key_part_end; key_part++) {
        if (key_part->field->is_nullable() || !find_func(key_part->field, data))
          break;
      }
      if (key_part == key_part_end) return true;
    }
  }
  return false;
}

/**
  Helper function for list_contains_unique_index.
  Find a field reference in a list of ORDER structures.
  Finds a direct reference of the Field in the list.

  @param field                The field to search for.
  @param data                 ORDER *.The list to search in

  @retval
    1                    found
  @retval
    0                    not found.
*/

static bool find_field_in_order_list(Field *field, void *data) {
  ORDER *group = (ORDER *)data;
  bool part_found = false;
  for (ORDER *tmp_group = group; tmp_group; tmp_group = tmp_group->next) {
    const Item *item = (*tmp_group->item)->real_item();
    if (item->type() == Item::FIELD_ITEM &&
        down_cast<const Item_field *>(item)->field->eq(field)) {
      part_found = true;
      break;
    }
  }
  return part_found;
}

/**
  Helper function for list_contains_unique_index.
  Find a field reference in a dynamic list of Items.
  Finds a direct reference of the Field in the list.

  @param[in] field             The field to search for.
  @param[in] data              List<Item> *.The list to search in

  @retval
    1                    found
  @retval
    0                    not found.
*/

static bool find_field_in_item_list(Field *field, void *data) {
  List<Item> *fields = (List<Item> *)data;
  bool part_found = false;
  List_iterator<Item> li(*fields);
  const Item *item;

  while ((item = li++)) {
    if (item->type() == Item::FIELD_ITEM &&
        down_cast<const Item_field *>(item)->field->eq(field)) {
      part_found = true;
      break;
    }
  }
  return part_found;
}

ORDER *create_order_from_distinct(THD *thd, Ref_item_array ref_item_array,
                                  ORDER *order_list, List<Item> &fields,
                                  bool skip_aggregates,
                                  bool convert_bit_fields_to_long,
                                  bool *all_order_by_fields_used) {
  List_iterator<Item> li(fields);
  Item *item;
  ORDER *order, *group, **prev;

  *all_order_by_fields_used = true;

  prev = &group;
  group = nullptr;
  for (order = order_list; order; order = order->next) {
    if (order->in_field_list) {
      ORDER *ord = (ORDER *)thd->memdup((char *)order, sizeof(ORDER));
      if (!ord) return nullptr;
      *prev = ord;
      prev = &ord->next;
      (*ord->item)->marker = Item::MARKER_DISTINCT_GROUP;
    } else
      *all_order_by_fields_used = false;
  }

  li.rewind();
  while ((item = li++)) {
    if (!item->const_item() && (!skip_aggregates || !item->has_aggregation()) &&
        item->marker != Item::MARKER_DISTINCT_GROUP) {
      /*
        Don't put duplicate columns from the SELECT list into the
        GROUP BY list.
      */
      ORDER *ord_iter;
      for (ord_iter = group; ord_iter; ord_iter = ord_iter->next)
        if ((*ord_iter->item)->eq(item, true)) goto next_item;

      ORDER *ord = (ORDER *)thd->mem_calloc(sizeof(ORDER));
      if (!ord) return nullptr;

      if (item->type() == Item::FIELD_ITEM &&
          item->data_type() == MYSQL_TYPE_BIT && convert_bit_fields_to_long) {
        /*
          Because HEAP tables can't index BIT fields we need to use an
          additional hidden field for grouping because later it will be
          converted to a LONG field. Original field will remain of the
          BIT type and will be returned to a client.
          @note setup_ref_array() needs to account for the extra space.
        */
        Item_field *new_item = new Item_field(thd, (Item_field *)item);
        ord->item = thd->lex->current_select()->add_hidden_item(new_item);
      } else {
        /*
          We have here only field_list (not all_field_list), so we can use
          simple indexing of ref_item_array (order in the array and in the
          list are same)
        */
        ord->item = &ref_item_array[0];
      }
      ord->direction = ORDER_ASC;
      *prev = ord;
      prev = &ord->next;
    }
  next_item:
    ref_item_array.pop_front();
  }
  *prev = nullptr;
  return group;
}

/**
  Return table number if there is only one table in sort order
  and group and order is compatible, else return 0.
*/

static TABLE *get_sort_by_table(ORDER *a, ORDER *b, TABLE_LIST *tables) {
  DBUG_TRACE;
  table_map map = (table_map)0;

  if (!a)
    a = b;  // Only one need to be given
  else if (!b)
    b = a;

  for (; a && b; a = a->next, b = b->next) {
    if (!(*a->item)->eq(*b->item, true)) return nullptr;
    map |= a->item[0]->used_tables();
  }
  map &= ~INNER_TABLE_BIT;
  if (!map || (map & (RAND_TABLE_BIT | OUTER_REF_TABLE_BIT))) return nullptr;

  for (; !(map & tables->map()); tables = tables->next_leaf)
    ;
  if (map != tables->map()) return nullptr;  // More than one table
  DBUG_PRINT("exit", ("sort by table: %d", tables->tableno()));
  return tables->table;
}

/**
  Update some values in keyuse for faster choose_table_order() loop.

  @todo Check if this is the real meaning of ref_table_rows.
*/

void JOIN::optimize_keyuse() {
  for (size_t ix = 0; ix < keyuse_array.size(); ++ix) {
    Key_use *keyuse = &keyuse_array.at(ix);
    table_map map;
    /*
      If we find a ref, assume this table matches a proportional
      part of this table.
      For example 100 records matching a table with 5000 records
      gives 5000/100 = 50 records per key
      Constant tables are ignored.
      To avoid bad matches, we don't make ref_table_rows less than 100.
    */
    keyuse->ref_table_rows = ~(ha_rows)0;  // If no ref
    if (keyuse->used_tables &
        (map = (keyuse->used_tables &
                ~(const_table_map | OUTER_REF_TABLE_BIT)))) {
      uint tableno;
      for (tableno = 0; !(map & 1); map >>= 1, tableno++) {
      }
      if (map == 1)  // Only one table
      {
        TABLE *tmp_table = join_tab[tableno].table();

        keyuse->ref_table_rows =
            max<ha_rows>(tmp_table->file->stats.records, 100);
      }
    }
    /*
      Outer reference (external field) is constant for single executing
      of subquery
    */
    if (keyuse->used_tables == OUTER_REF_TABLE_BIT) keyuse->ref_table_rows = 1;
  }
}

/**
  Function sets FT hints, initializes FT handlers
  and checks if FT index can be used as covered.
*/

bool JOIN::optimize_fts_query() {
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  DBUG_ASSERT(select_lex->has_ft_funcs());

  for (uint i = const_tables; i < tables; i++) {
    JOIN_TAB *tab = best_ref[i];
    if (tab->type() != JT_FT) continue;

    Item_func_match *ifm;
    Item_func_match *ft_func =
        down_cast<Item_func_match *>(tab->position()->key->val);
    List_iterator<Item_func_match> li(*(select_lex->ftfunc_list));

    while ((ifm = li++)) {
      if (!(ifm->used_tables() & tab->table_ref->map()) || ifm->master)
        continue;

      if (ifm != ft_func) {
        if (ifm->can_skip_ranking())
          ifm->set_hints(this, FT_NO_RANKING, HA_POS_ERROR, false);
      }
    }

    /*
      Check if internal sorting is needed. FT_SORTED flag is set
      if no ORDER BY clause or ORDER BY MATCH function is the same
      as the function that is used for FT index and FT table is
      the first non-constant table in the JOIN.
    */
    if (i == const_tables && !(ft_func->get_hints()->get_flags() & FT_BOOL) &&
        (!order || ft_func == test_if_ft_index_order(order)))
      ft_func->set_hints(this, FT_SORTED, m_select_limit, false);

    /*
      Check if ranking is not needed. FT_NO_RANKING flag is set if
      MATCH function is used only in WHERE condition and  MATCH
      function is not part of an expression.
    */
    if (ft_func->can_skip_ranking())
      ft_func->set_hints(this, FT_NO_RANKING,
                         !order ? m_select_limit : HA_POS_ERROR, false);
  }

  return init_ftfuncs(thd, select_lex);
}

/**
  Check if FTS index only access is possible.

  @param tab  pointer to JOIN_TAB structure.

  @return  true if index only access is possible,
           false otherwise.
*/

bool JOIN::fts_index_access(JOIN_TAB *tab) {
  DBUG_ASSERT(tab->type() == JT_FT);
  TABLE *table = tab->table();

  if ((table->file->ha_table_flags() & HA_CAN_FULLTEXT_EXT) == 0)
    return false;  // Optimizations requires extended FTS support by table
                   // engine

  /*
    This optimization does not work with filesort nor GROUP BY
  */
  if (grouped || (order && m_ordered_index_usage != ORDERED_INDEX_ORDER_BY))
    return false;

  /*
    Check whether the FTS result is covering.  If only document id
    and rank is needed, there is no need to access table rows.
  */
  for (uint i = bitmap_get_first_set(table->read_set); i < table->s->fields;
       i = bitmap_get_next_set(table->read_set, i)) {
    if (table->field[i] != table->fts_doc_id_field ||
        !tab->ft_func()->docid_in_result())
      return false;
  }

  return true;
}

/**
   For {semijoin,subquery} materialization: calculates various cost
   information, based on a plan in join->best_positions covering the
   to-be-materialized query block and only this.

   @param join     JOIN where plan can be found
   @param sj_nest  sj materialization nest (NULL if subquery materialization)
   @param n_tables number of to-be-materialized tables
   @param[out] sjm where computed costs will be stored

   @note that this function modifies join->map2table, which has to be filled
   correctly later.
*/
static void calculate_materialization_costs(JOIN *join, TABLE_LIST *sj_nest,
                                            uint n_tables,
                                            Semijoin_mat_optimize *sjm) {
  double mat_cost;           // Estimated cost of materialization
  double mat_rowcount;       // Estimated row count before duplicate removal
  double distinct_rowcount;  // Estimated rowcount after duplicate removal
  List<Item> *inner_expr_list;

  if (sj_nest) {
    /*
      get_partial_join_cost() assumes a regular join, which is correct when
      we optimize a sj-materialization nest (always executed as regular
      join).
    */
    get_partial_join_cost(join, n_tables, &mat_cost, &mat_rowcount);
    n_tables += join->const_tables;
    inner_expr_list = &sj_nest->nested_join->sj_inner_exprs;
  } else {
    mat_cost = join->best_read;
    mat_rowcount = static_cast<double>(join->best_rowcount);
    inner_expr_list = &join->select_lex->item_list;
  }

  /*
    Adjust output cardinality estimates. If the subquery has form

    ... oe IN (SELECT t1.colX, t2.colY, func(X,Y,Z) )

    then the number of distinct output record combinations has an
    upper bound of product of number of records matching the tables
    that are used by the SELECT clause.
    TODO:
    We can get a more precise estimate if we
     - use rec_per_key cardinality estimates. For simple cases like
     "oe IN (SELECT t.key ...)" it is trivial.
     - Functional dependencies between the tables in the semi-join
     nest (the payoff is probably less here?)
  */
  {
    for (uint i = 0; i < n_tables; i++) {
      JOIN_TAB *const tab = join->best_positions[i].table;
      join->map2table[tab->table_ref->tableno()] = tab;
    }
    List_iterator<Item> it(*inner_expr_list);
    Item *item;
    table_map map = 0;
    while ((item = it++)) map |= item->used_tables();
    map &= ~PSEUDO_TABLE_BITS;
    Table_map_iterator tm_it(map);
    int tableno;
    double rows = 1.0;
    while ((tableno = tm_it.next_bit()) != Table_map_iterator::BITMAP_END)
      rows *= join->map2table[tableno]->table()->quick_condition_rows;
    distinct_rowcount = min(mat_rowcount, rows);
  }
  /*
    Calculate temporary table parameters and usage costs
  */
  const uint rowlen = get_tmp_table_rec_length(*inner_expr_list);

  const Cost_model_server *cost_model = join->cost_model();

  Cost_model_server::enum_tmptable_type tmp_table_type;
  if (rowlen * distinct_rowcount < join->thd->variables.max_heap_table_size)
    tmp_table_type = Cost_model_server::MEMORY_TMPTABLE;
  else
    tmp_table_type = Cost_model_server::DISK_TMPTABLE;

  /*
    Let materialization cost include the cost to create the temporary
    table and write the rows into it:
  */
  mat_cost += cost_model->tmptable_create_cost(tmp_table_type);
  mat_cost +=
      cost_model->tmptable_readwrite_cost(tmp_table_type, mat_rowcount, 0.0);

  sjm->materialization_cost.reset();
  sjm->materialization_cost.add_io(mat_cost);

  sjm->expected_rowcount = distinct_rowcount;

  /*
    Set the cost to do a full scan of the temptable (will need this to
    consider doing sjm-scan):
  */
  sjm->scan_cost.reset();
  if (distinct_rowcount > 0.0) {
    const double scan_cost = cost_model->tmptable_readwrite_cost(
        tmp_table_type, 0.0, distinct_rowcount);
    sjm->scan_cost.add_io(scan_cost);
  }

  // The cost to lookup a row in temp. table
  const double row_cost =
      cost_model->tmptable_readwrite_cost(tmp_table_type, 0.0, 1.0);
  sjm->lookup_cost.reset();
  sjm->lookup_cost.add_io(row_cost);
}

/**
   Decides between EXISTS and materialization; performs last steps to set up
   the chosen strategy.
   @returns 'false' if no error

   @note If UNION this is called on each contained JOIN.

 */
bool JOIN::decide_subquery_strategy() {
  DBUG_ASSERT(unit->item);

  switch (unit->item->substype()) {
    case Item_subselect::IN_SUBS:
    case Item_subselect::ALL_SUBS:
    case Item_subselect::ANY_SUBS:
      // All of those are children of Item_in_subselect and may use EXISTS
      break;
    default:
      return false;
  }

  Item_in_subselect *const in_pred =
      static_cast<Item_in_subselect *>(unit->item);

  SubqueryExecMethod chosen_method = in_pred->exec_method;
  // Materialization does not allow UNION so this can't happen:
  DBUG_ASSERT(chosen_method != SubqueryExecMethod::EXEC_MATERIALIZATION);

  if ((chosen_method == SubqueryExecMethod::EXEC_EXISTS_OR_MAT) &&
      compare_costs_of_subquery_strategies(&chosen_method))
    return true;

  switch (chosen_method) {
    case SubqueryExecMethod::EXEC_EXISTS:
      if (select_lex->m_windows.elements > 0)  // grep for WL#10431
      {
        my_error(ER_NOT_SUPPORTED_YET, MYF(0),
                 "the combination of this ALL/ANY/SOME/IN subquery with this"
                 " comparison operator and with contained window functions");
        return true;
      }
      return in_pred->finalize_exists_transform(thd, select_lex);
    case SubqueryExecMethod::EXEC_MATERIALIZATION:
      return in_pred->finalize_materialization_transform(thd, this);
    default:
      DBUG_ASSERT(false);
      return true;
  }
}

/**
   Tells what is the cheapest between IN->EXISTS and subquery materialization,
   in terms of cost, for the subquery's JOIN.
   Input:
   - join->{best_positions,best_read,best_rowcount} must contain the
   execution plan of EXISTS (where 'join' is the subquery's JOIN)
   - join2->{best_positions,best_read,best_rowcount} must be correctly set
   (where 'join2' is the parent join, the grandparent join, etc).
   Output:
   join->{best_positions,best_read,best_rowcount} contain the cheapest
   execution plan (where 'join' is the subquery's JOIN).

   This plan choice has to happen before calling functions which set up
   execution structures, like JOIN::get_best_combination().

   @param[out] method  chosen method (EXISTS or materialization) will be put
                       here.
   @returns false if success
*/
bool JOIN::compare_costs_of_subquery_strategies(SubqueryExecMethod *method) {
  *method = SubqueryExecMethod::EXEC_EXISTS;

  SubqueryExecMethod allowed_strategies = select_lex->subquery_strategy(thd);

  /*
    A non-deterministic subquery should not use materialization, unless forced.
    For a detailed explanation, see SELECT_LEX::decorrelate_where_cond().
    Here, the same logic is applied also for subqueries that are not converted
    to semi-join.
  */
  if (allowed_strategies == SubqueryExecMethod::EXEC_EXISTS_OR_MAT &&
      (unit->uncacheable & UNCACHEABLE_RAND))
    allowed_strategies = SubqueryExecMethod::EXEC_EXISTS;

  if (allowed_strategies == SubqueryExecMethod::EXEC_EXISTS) return false;

  DBUG_ASSERT(allowed_strategies == SubqueryExecMethod::EXEC_EXISTS_OR_MAT ||
              allowed_strategies == SubqueryExecMethod::EXEC_MATERIALIZATION);

  const JOIN *parent_join = unit->outer_select()->join;
  if (!parent_join || !parent_join->child_subquery_can_materialize)
    return false;

  Item_in_subselect *const in_pred =
      static_cast<Item_in_subselect *>(unit->item);

  /*
    Testing subquery_allows_etc() at each optimization is necessary as each
    execution of a prepared statement may use a different type of parameter.
  */
  if (!in_pred->subquery_allows_materialization(thd, select_lex,
                                                select_lex->outer_select()))
    return false;

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_subqmat(
      trace, "execution_plan_for_potential_materialization");
  const double saved_best_read = best_read;
  const ha_rows saved_best_rowcount = best_rowcount;
  POSITION *const saved_best_pos = best_positions;

  if (in_pred->in2exists_added_to_where()) {
    Opt_trace_array trace_subqmat_steps(trace, "steps");

    // Up to one extra slot per semi-join nest is needed (if materialized)
    const uint sj_nests = select_lex->sj_nests.size();

    if (!(best_positions = new (thd->mem_root) POSITION[tables + sj_nests]))
      return true;

    // Compute plans which do not use outer references

    DBUG_ASSERT(allow_outer_refs);
    allow_outer_refs = false;

    if (optimize_semijoin_nests_for_materialization(this)) return true;

    if (Optimize_table_order(thd, this, nullptr).choose_table_order())
      return true;
  } else {
    /*
      If IN->EXISTS didn't add any condition to WHERE (only to HAVING, which
      can happen if subquery has aggregates) then the plan for materialization
      will be the same as for EXISTS - don't compute it again.
    */
    trace_subqmat.add("surely_same_plan_as_EXISTS", true)
        .add_alnum("cause", "EXISTS_did_not_change_WHERE");
  }

  Semijoin_mat_optimize sjm;
  calculate_materialization_costs(this, nullptr, primary_tables, &sjm);

  /*
    The number of evaluations of the subquery influences costs, we need to
    compute it.
  */
  Opt_trace_object trace_subq_mat_decision(trace, "subq_mat_decision");
  const double subq_executions = calculate_subquery_executions(in_pred, trace);
  const double cost_exists = subq_executions * saved_best_read;
  const double cost_mat_table = sjm.materialization_cost.total_cost();
  const double cost_mat =
      cost_mat_table + subq_executions * sjm.lookup_cost.total_cost();
  const bool mat_chosen =
      (allowed_strategies == SubqueryExecMethod::EXEC_EXISTS_OR_MAT)
          ? (cost_mat < cost_exists)
          : true;
  trace_subq_mat_decision
      .add("cost_to_create_and_fill_materialized_table", cost_mat_table)
      .add("cost_of_one_EXISTS", saved_best_read)
      .add("number_of_subquery_evaluations", subq_executions)
      .add("cost_of_materialization", cost_mat)
      .add("cost_of_EXISTS", cost_exists)
      .add("chosen", mat_chosen);
  if (mat_chosen) {
    *method = SubqueryExecMethod::EXEC_MATERIALIZATION;
  } else {
    best_read = saved_best_read;
    best_rowcount = saved_best_rowcount;
    best_positions = saved_best_pos;
    /*
      Don't restore JOIN::positions or best_ref, they're not used
      afterwards. best_positions is (like: by get_sj_strategy()).
    */
  }
  return false;
}

double calculate_subquery_executions(const Item_subselect *subquery,
                                     Opt_trace_context *trace) {
  Opt_trace_array trace_parents(trace, "parent_fanouts");
  double subquery_executions = 1.0;
  for (;;) {
    const SELECT_LEX *const parent_select = subquery->unit->outer_select();
    const JOIN *const parent_join = parent_select->join;
    if (parent_join == nullptr) {
      /*
        May be single-table UPDATE/DELETE, has no join.
        @todo  we should find how many rows it plans to UPDATE/DELETE, taking
        inspiration in Explain_table::explain_rows_and_filtered().
        This is not a priority as it applies only to
        UPDATE - child(non-mat-subq) - grandchild(may-be-mat-subq).
        And it will autosolve the day UPDATE gets a JOIN.
      */
      break;
    }

    Opt_trace_object trace_parent(trace);
    trace_parent.add_select_number(parent_select->select_number);
    double parent_fanout;
    if (  // safety, not sure needed
        parent_join->plan_is_const() ||
        // if subq is in condition on constant table:
        !parent_join->child_subquery_can_materialize) {
      parent_fanout = 1.0;
      trace_parent.add("subq_attached_to_const_table", true);
    } else {
      if (subquery->in_cond_of_tab != NO_PLAN_IDX) {
        /*
          Subquery is attached to a certain 'pos', pos[-1].prefix_rowcount
          is the number of times we'll start a loop accessing 'pos'; each such
          loop will read pos->rows_fetched rows of 'pos', so subquery will
          be evaluated pos[-1].prefix_rowcount * pos->rows_fetched times.
          Exceptions:
          - if 'pos' is first, use 1.0 instead of pos[-1].prefix_rowcount
          - if 'pos' is first of a sj-materialization nest, same.

          If in a sj-materialization nest, pos->rows_fetched and
          pos[-1].prefix_rowcount are of the "nest materialization" plan
          (copied back in fix_semijoin_strategies()), which is
          appropriate as it corresponds to evaluations of our subquery.

          pos->prefix_rowcount is not suitable because if we have:
          select ... from ot1 where ot1.col in
            (select it1.col1 from it1 where it1.col2 not in (subq));
          and subq does subq-mat, and plan is ot1 - it1+firstmatch(ot1),
          then:
          - t1.prefix_rowcount==1 (due to firstmatch)
          - subq is attached to it1, and is evaluated for each row read from
            t1, potentially way more than 1.
         */
        const uint idx = subquery->in_cond_of_tab;
        DBUG_ASSERT((int)idx >= 0 && idx < parent_join->tables);
        trace_parent.add("subq_attached_to_table", true);
        QEP_TAB *const parent_tab = &parent_join->qep_tab[idx];
        trace_parent.add_utf8_table(parent_tab->table_ref);
        parent_fanout = parent_tab->position()->rows_fetched;
        if ((idx > parent_join->const_tables) &&
            !sj_is_materialize_strategy(parent_tab->position()->sj_strategy))
          parent_fanout *= parent_tab[-1].position()->prefix_rowcount;
      } else {
        /*
          Subquery is SELECT list, GROUP BY, ORDER BY, HAVING: it is evaluated
          at the end of the parent join's execution.
          It can be evaluated once per row-before-grouping:
          SELECT SUM(t1.col IN (subq)) FROM t1 GROUP BY expr;
          or once per row-after-grouping:
          SELECT SUM(t1.col) AS s FROM t1 GROUP BY expr HAVING s IN (subq),
          SELECT SUM(t1.col) IN (subq) FROM t1 GROUP BY expr
          It's hard to tell. We simply assume 'once per
          row-before-grouping'.

          Another approximation:
          SELECT ... HAVING x IN (subq) LIMIT 1
          best_rowcount=1 due to LIMIT, though HAVING (and thus the subquery)
          may be evaluated many times before HAVING becomes true and the limit
          is reached.
        */
        trace_parent.add("subq_attached_to_join_result", true);
        parent_fanout = static_cast<double>(parent_join->best_rowcount);
      }
    }
    subquery_executions *= parent_fanout;
    trace_parent.add("fanout", parent_fanout);
    const bool cacheable = parent_select->is_cacheable();
    trace_parent.add("cacheable", cacheable);
    if (cacheable) {
      // Parent executed only once
      break;
    }
    /*
      Parent query is executed once per outer row => go up to find number of
      outer rows. Example:
      SELECT ... IN(subq-with-in2exists WHERE ... IN (subq-with-mat))
    */
    subquery = parent_join->unit->item;
    if (subquery == nullptr) {
      // derived table, materialized only once
      break;
    }
  }  // for(;;)
  return subquery_executions;
}

/**
  Optimize rollup specification.

  Allocate objects needed for rollup processing.

  @returns false if success, true if error.
*/

bool JOIN::optimize_rollup() {
  tmp_table_param.allow_group_via_temp_table = false;
  rollup.state = ROLLUP::STATE_INITED;

  /*
    Create pointers to the different sum function groups
    These are updated by rollup_make_fields()
  */
  tmp_table_param.group_parts = send_group_parts;
  /*
    substitute_gc() might substitute an expression in the GROUP BY list with
    a generated column. In such case the GC is added to the all_fields as a
    hidden field. In total, all_fields list could be grown by up to
    send_group_parts columns. Reserve space for them here.
  */
  const uint ref_array_size = all_fields.elements + send_group_parts;

  Item_null_result **null_items = static_cast<Item_null_result **>(
      thd->alloc(sizeof(Item *) * send_group_parts));

  rollup.null_items = Item_null_array(null_items, send_group_parts);
  rollup.ref_item_arrays = static_cast<Ref_item_array *>(
      thd->alloc((sizeof(Ref_item_array) + ref_array_size * sizeof(Item *)) *
                 send_group_parts));
  rollup.all_fields = static_cast<List<Item> *>(
      thd->alloc(sizeof(List<Item>) * send_group_parts));
  rollup.fields_list = static_cast<List<Item> *>(
      thd->alloc(sizeof(List<Item>) * send_group_parts));

  if (!null_items || !rollup.ref_item_arrays || !rollup.all_fields ||
      !rollup.fields_list)
    return true;

  Item **ref_array = (Item **)(rollup.ref_item_arrays + send_group_parts);

  /*
    Prepare space for field list for the different levels
    These will be filled up in rollup_make_fields()
  */
  ORDER *group = group_list;
  for (uint i = 0; i < send_group_parts; i++, group = group->next) {
    rollup.null_items[i] = new (thd->mem_root) Item_null_result(
        (*group->item)->data_type(), (*group->item)->result_type());
    if (rollup.null_items[i] == NULL) return true; /* purecov: inspected */
    rollup.fields_list[i].empty();
    rollup.all_fields[i].empty();
    rollup.ref_item_arrays[i] = Ref_item_array(ref_array, ref_array_size);
    ref_array += ref_array_size;
  }
  for (uint i = 0; i < send_group_parts; i++) {
    for (uint j = 0; j < fields_list.elements; j++)
      rollup.fields_list[i].push_back(rollup.null_items[i]);
    for (uint j = 0; j < all_fields.elements; j++)
      rollup.all_fields[i].push_back(rollup.null_items[i]);
  }
  return false;
}

/**
  Refine the best_rowcount estimation based on what happens after tables
  have been joined: LIMIT and type of result sink.
 */
void JOIN::refine_best_rowcount() {
  // If plan is const, 0 or 1 rows should be returned
  DBUG_ASSERT(!plan_is_const() || best_rowcount <= 1);

  if (plan_is_const()) return;

  /*
    If a derived table, or a member of a UNION which itself forms a derived
    table:
    setting estimate to 0 or 1 row would mark the derived table as const.
    The row count is bumped to the nearest higher value, so that the
    query block will not be evaluated during optimization.
  */
  if (best_rowcount <= 1 &&
      select_lex->master_unit()->first_select()->linkage == DERIVED_TABLE_TYPE)
    best_rowcount = PLACEHOLDER_TABLE_ROW_ESTIMATE;

  /*
    There will be no more rows than defined in the LIMIT clause. Use it
    as an estimate. If LIMIT 1 is specified, the query block will be
    considered "const", with actual row count 0 or 1.
  */
  best_rowcount = std::min(best_rowcount, unit->select_limit_cnt);
}

List<Item> *JOIN::get_current_fields() {
  DBUG_ASSERT((int)current_ref_item_slice >= 0);
  if (current_ref_item_slice == REF_SLICE_SAVED_BASE) return fields;
  return &tmp_fields_list[current_ref_item_slice];
}

const Cost_model_server *JOIN::cost_model() const {
  DBUG_ASSERT(thd != nullptr);
  return thd->cost_model();
}

/**
  Measure Compilation CPU and store it statement metrics tables

    The function computes the compilation CPU time and stores it in thread.
    This function is called from JOIN::optimize().

  @param thd          thread handle
  @param cpu_res      CPU resource usage return code
  @param beginning_id timespec of the beginning of the clock
*/

static void measure_compilation_cpu(THD *thd, int cpu_res,
                                    timespec *beginning_id) {
  timespec time_end;
  if (cpu_res == 0 &&
      (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_end) == 0)) {
    ulonglong diff = diff_timespec(&time_end, beginning_id);
    diff *= 1000; /* convert to picoseconds */
    MYSQL_INC_STATEMENT_COMPILATION_CPU(thd->m_statement_psi, (ulonglong)diff);
  }
}

/**
  @} (end of group Query_Optimizer)
*/

/**
  This function is used to get the key length of Item object on
  which one tmp field will be created during create_tmp_table.
  This function references KEY_PART_INFO::init_from_field().

  @param item  A inner item of outer join

  @return  The length of a item to be as a key of a temp table
*/

static uint32 get_key_length_tmp_table(Item *item) {
  uint32 len = 0;

  item = item->real_item();
  if (item->type() == Item::FIELD_ITEM)
    len = ((Item_field *)item)->field->key_length();
  else
    len = item->max_length;

  if (item->maybe_null) len += HA_KEY_NULL_LENGTH;

  // references KEY_PART_INFO::init_from_field()
  enum_field_types type = item->data_type();
  if (type == MYSQL_TYPE_BLOB || type == MYSQL_TYPE_VARCHAR ||
      type == MYSQL_TYPE_GEOMETRY)
    len += HA_KEY_BLOB_LENGTH;

  return len;
}

bool evaluate_during_optimization(const Item *item, const SELECT_LEX *select) {
  /*
    Should only be called on items that are const_for_execution(), as those
    items are the only ones that are allowed to be evaluated during optimization
    in the first place.

    Additionally, allow items that only access tables in JOIN::const_table_map.
    This should not be necessary, but the const_for_execution() property is not
    always updated correctly by update_used_tables() for certain subqueries.
  */
  DBUG_ASSERT(item->const_for_execution() ||
              (item->used_tables() & ~select->join->const_table_map) == 0);

  // If the Item does not access any tables, it can always be evaluated.
  if (item->const_item()) return true;

  return !item->has_subquery() || (select->active_options() &
                                   OPTION_NO_SUBQUERY_DURING_OPTIMIZATION) == 0;
}

Prepare_error_tracker::~Prepare_error_tracker() {
  if (m_thd->is_error()) m_thd->lex->mark_broken();
}
