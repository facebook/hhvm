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
  Sum functions (COUNT, MIN...)
*/

#include "sql/item_sum.h"

#include <string.h>
#include <algorithm>
#include <bitset>
#include <functional>
#include <string>
#include <utility>  // std::forward

#include "decimal.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_byteorder.h"
#include "my_compare.h"
#include "my_dbug.h"
#include "my_double2ulonglong.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/aggregate_check.h"  // Distinct_check
#include "sql/create_field.h"
#include "sql/current_thd.h"  // current_thd
#include "sql/derror.h"       // ER_THD
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/item_json_func.h"
#include "sql/item_subselect.h"
#include "sql/json_dom.h"
#include "sql/key_spec.h"
#include "sql/mysqld.h"
#include "sql/parse_tree_helpers.h"  // PT_item_list
#include "sql/parse_tree_nodes.h"    // PT_order_list
#include "sql/parser_yystype.h"
#include "sql/sql_array.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_exception_handler.h"  // handle_std_exception
#include "sql/sql_executor.h"           // copy_fields
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_resolver.h"  // setup_order
#include "sql/sql_select.h"
#include "sql/sql_tmp_table.h"  // create_tmp_table
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/temp_table_param.h"  // Temp_table_param
#include "sql/thr_malloc.h"
#include "sql/uniques.h"  // Unique
#include "sql/window.h"

using std::max;
using std::min;

bool Item_sum::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;

  if (m_window) {
    if (m_window->contextualize(pc)) return true; /* purecov: inspected */
    if (!m_window->is_reference()) {
      pc->select->m_windows.push_back(m_window);
      m_window->set_def_pos(pc->select->m_windows.elements);
    }
    m_is_window_function = true;
    pc->select->n_sum_items++;
    set_wf();
  } else {
    mark_as_sum_func(pc->select);
    pc->select->in_sum_expr++;
  }

  for (uint i = 0; i < arg_count; i++) {
    if (args[i]->itemize(pc, &args[i])) return true;
  }

  if (!m_window) pc->select->in_sum_expr--;

  return false;
}

/**
  Calculate the affordable RAM limit for structures like TREE or Unique
  used in Item_sum_*
*/

ulonglong Item_sum::ram_limitation(THD *thd) {
  ulonglong limitation =
      min(thd->variables.tmp_table_size, thd->variables.max_heap_table_size);

  DBUG_EXECUTE_IF("simulate_low_itemsum_ram_limitation", limitation = 32;);

  return limitation;
}

/**
  Prepare an aggregate function for checking of context.

    The function initializes the members of the Item_sum object.
    It also checks the general validity of the set function:
    If none of the currently active query blocks allow evaluation of
    set functions, an error is reported.

  @note
    This function must be called for all set functions when expressions are
    resolved. It must be invoked in prefix order, ie at the descent of this
    traversal. @see corresponding Item_sum::check_sum_func(), which should
    be called on ascent.

  @param thd      reference to the thread context info

  @returns false if success, true if error
*/

bool Item_sum::init_sum_func_check(THD *thd) {
  if (m_is_window_function) {
    /*
      Are either no aggregates of any kind allowed at this level, or
      specifically not window functions?
    */
    LEX *const lex = thd->lex;
    if (((~lex->allow_sum_func | lex->m_deny_window_func) >>
         lex->current_select()->nest_level) &
        0x1) {
      my_error(ER_WINDOW_INVALID_WINDOW_FUNC_USE, MYF(0), func_name());
      return true;
    }
    in_sum_func = nullptr;
  } else {
    if (!thd->lex->allow_sum_func) {
      my_error(ER_INVALID_GROUP_FUNC_USE, MYF(0));
      return true;
    }
    // Set a reference to the containing set function if there is one
    in_sum_func = thd->lex->in_sum_func;
    /*
      Set this object as the current containing set function, used when
      checking arguments of this set function.
    */
    thd->lex->in_sum_func = this;
  }
  save_deny_window_func = thd->lex->m_deny_window_func;
  thd->lex->m_deny_window_func |= (nesting_map)1
                                  << thd->lex->current_select()->nest_level;
  // @todo: When resolving once, move following code to constructor
  base_select = thd->lex->current_select();
  aggr_select = nullptr;  // Aggregation query block is undetermined yet
  referenced_by[0] = nullptr;
  /*
    Leave referenced_by[1] unchanged as in execution of PS, in-to-exists is not
    re-done, so referenced_by[1] isn't set again. So keep it as it was in
    preparation.
  */
  if (thd->lex->current_select()->first_execution) referenced_by[1] = nullptr;
  max_aggr_level = -1;
  max_sum_func_level = -1;
  used_tables_cache = 0;
  return false;
}

/**
  Validate the semantic requirements of a set function.

    Check whether the context of the set function allows it to be aggregated
    and, when it is an argument of another set function, directly or indirectly,
    the function makes sure that these two set functions are aggregated in
    different query blocks.
    If the context conditions are not met, an error is reported.
    If the set function is aggregated in some outer query block, it is
    added to the chain of items inner_sum_func_list attached to the
    aggregating query block.

    A number of designated members of the object are used to check the
    conditions. They are specified in the comment before the Item_sum
    class declaration.
    Additionally a bitmap variable called allow_sum_func is employed.
    It is included into the LEX structure.
    The bitmap contains 1 at n-th position if the query block at level "n"
    allows a set function reference (i.e the current resolver context for
    the query block is either in the SELECT list or in the HAVING or
    ORDER BY clause).

    Consider the query:
    @code
       SELECT SUM(t1.b) FROM t1 GROUP BY t1.a
         HAVING t1.a IN (SELECT t2.c FROM t2 WHERE AVG(t1.b) > 20) AND
                t1.a > (SELECT MIN(t2.d) FROM t2);
    @endcode
    when the set functions are resolved, allow_sum_func will contain:
    - for SUM(t1.b) - 1 at position 0 (SUM is in SELECT list)
    - for AVG(t1.b) - 1 at position 0 (subquery is in HAVING clause)
                      0 at position 1 (AVG is in WHERE clause)
    - for MIN(t2.d) - 1 at position 0 (subquery is in HAVING clause)
                      1 at position 1 (MIN is in SELECT list)

  @note
    This function must be called for all set functions when expressions are
    resolved. It must be invoked in postfix order, ie at the ascent of this
    traversal.

  @param thd  reference to the thread context info
  @param ref  location of the pointer to this item in the containing expression

  @returns false if success, true if error
*/

bool Item_sum::check_sum_func(THD *thd, Item **ref) {
  DBUG_TRACE;

  if (m_is_window_function) {
    update_used_tables();
    thd->lex->m_deny_window_func = save_deny_window_func;
    return false;
  }

  const nesting_map allow_sum_func = thd->lex->allow_sum_func;
  const nesting_map nest_level_map = (nesting_map)1 << base_select->nest_level;

  DBUG_ASSERT(thd->lex->current_select() == base_select);
  DBUG_ASSERT(aggr_select == nullptr);

  /*
    max_aggr_level is the level of the innermost qualifying query block of
    the column references of this set function. If the set function contains
    no column references, max_aggr_level is -1.
    max_aggr_level cannot be greater than nest level of the current query block.
  */
  DBUG_ASSERT(max_aggr_level <= base_select->nest_level);

  if (base_select->nest_level == max_aggr_level) {
    /*
      The function must be aggregated in the current query block,
      and it must be referred within a clause where it is valid
      (ie. HAVING clause, ORDER BY clause or SELECT list)
    */
    if ((allow_sum_func & nest_level_map) != 0) aggr_select = base_select;
  } else if (max_aggr_level >= 0 || !(allow_sum_func & nest_level_map)) {
    /*
      Look for an outer query block where the set function should be
      aggregated. If it finds such a query block, then aggr_select is set
      to this query block
    */
    for (SELECT_LEX *sl = base_select->outer_select();
         sl && sl->nest_level >= max_aggr_level; sl = sl->outer_select()) {
      if (allow_sum_func & ((nesting_map)1 << sl->nest_level)) aggr_select = sl;
    }
  } else  // max_aggr_level < 0
  {
    /*
      Set function without column reference is aggregated in innermost query,
      without any validation.
    */
    aggr_select = base_select;
  }

  if (aggr_select == nullptr && (allow_sum_func & nest_level_map) != 0 &&
      !(thd->variables.sql_mode & MODE_ANSI))
    aggr_select = base_select;

  /*
    At this place a query block where the set function is to be aggregated
    has been found and is assigned to aggr_select, or aggr_select is NULL to
    indicate an invalid set function.

    Additionally, check whether possible nested set functions are acceptable
    here: their aggregation level must be greater than this set function's
    aggregation level.
  */
  if (aggr_select == nullptr || aggr_select->nest_level <= max_sum_func_level) {
    my_error(ER_INVALID_GROUP_FUNC_USE, MYF(0));
    return true;
  }

  if (aggr_select != base_select) {
    referenced_by[0] = ref;
    /*
      Add the set function to the list inner_sum_func_list for the
      aggregating query block.

      @note
        Now we 'register' only set functions that are aggregated in outer
        query blocks. Actually it makes sense to link all set functions for
        a query block in one chain. It would simplify the process of 'splitting'
        for set functions.
    */
    if (!aggr_select->inner_sum_func_list)
      next_sum = this;
    else {
      next_sum = aggr_select->inner_sum_func_list->next_sum;
      aggr_select->inner_sum_func_list->next_sum = this;
    }
    aggr_select->inner_sum_func_list = this;
    aggr_select->with_sum_func = true;

    /*
      Mark subqueries as containing set function all the way up to the
      set function's aggregation query block.
      Note that we must not mark the Item of calculation context itself
      because with_sum_func on the aggregation query block is already set above.

      has_aggregation() being set for an Item means that this Item refers
      (somewhere in it, e.g. one of its arguments if it's a function) directly
      or indirectly to a set function that is calculated in a
      context "outside" of the Item (e.g. in the current or outer query block).

      with_sum_func being set for a query block means that this query block
      has set functions directly referenced (i.e. not through a subquery).

      If, going up, we meet a derived table, we do nothing special for it:
      it doesn't need this information.
    */
    for (SELECT_LEX *sl = base_select; sl && sl != aggr_select;
         sl = sl->outer_select()) {
      if (sl->master_unit()->item) sl->master_unit()->item->set_aggregation();
    }

    base_select->mark_as_dependent(aggr_select, true);
  }

  if (in_sum_func) {
    /*
      If the set function is nested adjust the value of
      max_sum_func_level for the containing set function.
      We take into account only set functions that are to be aggregated on
      the same level or outer compared to the nest level of the containing
      set function.
      But we must always pass up the max_sum_func_level because it is
      the maximum nest level of all directly and indirectly contained
      set functions. We must do that even for set functions that are
      aggregated inside of their containing set function's nest level
      because the containing function may contain another containing
      function that is to be aggregated outside or on the same level
      as its parent's nest level.
    */
    if (in_sum_func->base_select->nest_level >= aggr_select->nest_level)
      in_sum_func->max_sum_func_level =
          max(in_sum_func->max_sum_func_level, int8(aggr_select->nest_level));
    in_sum_func->max_sum_func_level =
        max(in_sum_func->max_sum_func_level, max_sum_func_level);
  }

  aggr_select->set_agg_func_used(true);
  if (sum_func() == JSON_AGG_FUNC) aggr_select->set_json_agg_func_used(true);
  update_used_tables();
  thd->lex->in_sum_func = in_sum_func;
  thd->lex->m_deny_window_func = save_deny_window_func;

  return false;
}

bool Item_sum::check_wf_semantics(THD *thd MY_ATTRIBUTE((unused)),
                                  SELECT_LEX *select MY_ATTRIBUTE((unused)),
                                  Window_evaluation_requirements *r) {
  const PT_frame *frame = m_window->frame();

  /*
    If we have ROWS BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW, we can just
    accumulate as we see rows, never need to invert old rows or to look at
    future rows, so don't need a frame buffer.
  */
  r->needs_buffer = !(frame->m_unit == WFU_ROWS &&
                      frame->m_from->m_border_type == WBT_UNBOUNDED_PRECEDING &&
                      frame->m_to->m_border_type == WBT_CURRENT_ROW);

  if (with_distinct) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "<window function>(DISTINCT ..)");
    return true;
  }
  return false;
}

Item_sum::Item_sum(const POS &pos, PT_item_list *opt_list, PT_window *w)
    : super(pos),
      m_window(w),
      m_window_resolved(false),
      next_sum(nullptr),
      arg_count(opt_list == nullptr ? 0 : opt_list->elements()),
      args(nullptr),
      used_tables_cache(0),
      forced_const(false) {
  if (arg_count > 0) {
    args = (Item **)(*THR_MALLOC)->Alloc(sizeof(Item *) * arg_count);
    if (args == nullptr) {
      return;  // OOM
    }
    uint i = 0;
    List_iterator_fast<Item> li(opt_list->value);
    Item *item;

    while ((item = li++)) args[i++] = item;
  }
  init_aggregator();
}

/**
  Constructor used in processing select with temporary tebles.
*/

Item_sum::Item_sum(THD *thd, const Item_sum *item)
    : Item_result_field(thd, item),
      m_window(item->m_window),
      m_window_resolved(false),
      next_sum(nullptr),
      base_select(item->base_select),
      aggr_select(item->aggr_select),
      allow_group_via_temp_table(item->allow_group_via_temp_table),
      arg_count(item->arg_count),
      used_tables_cache(item->used_tables_cache),
      forced_const(item->forced_const) {
  if (arg_count <= 2)
    args = tmp_args;
  else if (!(args = (Item **)thd->alloc(sizeof(Item *) * arg_count)))
    return;
  memcpy(args, item->args, sizeof(Item *) * arg_count);
  init_aggregator();
  with_distinct = item->with_distinct;
  if (item->aggr) set_aggregator(item->aggr->Aggrtype());
  DBUG_ASSERT(!m_is_window_function);  // WF items are never copied
}

void Item_sum::mark_as_sum_func() {
  mark_as_sum_func(current_thd->lex->current_select());
}

void Item_sum::mark_as_sum_func(SELECT_LEX *cur_select) {
  cur_select->n_sum_items++;
  cur_select->with_sum_func = true;
  set_aggregation();
}

void Item_sum::print(const THD *thd, String *str,
                     enum_query_type query_type) const {
  str->append(func_name());
  str->append('(');
  if (has_with_distinct()) str->append("distinct ");

  for (uint i = 0; i < arg_count; i++) {
    if (i) str->append(',');
    args[i]->print(thd, str, query_type);
  }
  str->append(')');

  if (m_window) {
    str->append(" OVER ");
    m_window->print(thd, str, query_type, false);
  }
}

void Item_sum::fix_num_length_and_dec() {
  decimals = 0;
  for (uint i = 0; i < arg_count; i++)
    decimals = max(decimals, args[i]->decimals);
  max_length = float_length(decimals);
}

bool Item_sum::resolve_type(THD *) {
  maybe_null = true;
  null_value = true;

  const Sumfunctype t = sum_func();

  // None except these 3 types are allowed for geometry arguments.
  if (!(t == COUNT_FUNC || t == COUNT_DISTINCT_FUNC || t == SUM_BIT_FUNC))
    return reject_geometry_args(arg_count, args, this);
  return false;
}

bool Item_sum::walk(Item_processor processor, enum_walk walk, uchar *argument) {
  if ((walk & enum_walk::PREFIX) && (this->*processor)(argument)) return true;

  Item **arg, **arg_end;
  for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
    if ((*arg)->walk(processor, walk, argument)) return true;
  }

  return (walk & enum_walk::POSTFIX) && (this->*processor)(argument);
}

/**
 Transform an Item_func object with a transformer callback function.

 The function recursively applies the transform method to each
 argument of the Item_func node.
 If the call of the method for an argument item returns a new item
 the old item is substituted for a new one.
 After this the transformer is applied to the root node
 of the Item_func object.
 */

Item *Item_sum::transform(Item_transformer transformer, uchar *argument) {
  if (arg_count) {
    Item **arg, **arg_end;
    for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
      Item *new_item = (*arg)->transform(transformer, argument);
      if (new_item == nullptr) return nullptr;
      if (*arg != new_item) current_thd->change_item_tree(arg, new_item);
    }
  }
  return (this->*transformer)(argument);
}

/**
  Remove the item from the list of inner aggregation functions in the
  SELECT_LEX it was moved to by Item_sum::check_sum_func().

  This is done to undo some of the effects of Item_sum::check_sum_func() so
  that the item may be removed from the query.

  @note This doesn't completely undo Item_sum::check_sum_func(), as
  aggregation information is left untouched. This means that if this
  item is removed, aggr_select and all subquery items between aggr_select
  and this item may be left with has_aggregation() set to true, even if
  there are no aggregation functions. To our knowledge, this has no
  impact on the query result.

  @see Item_sum::check_sum_func()
  @see remove_redundant_subquery_clauses()

  If this is a window function, remove the reference from the window.
  This is needed when constant predicates are being removed.

  @see Item_cond::fix_fields()
  @see Item_cond::remove_const_cond()
 */
bool Item_sum::clean_up_after_removal(uchar *arg) {
  /*
    Don't do anything if
    1) this is an unresolved item (This may happen if an
       expression occurs twice in the same query. In that case, the
       whole item tree for the second occurence is replaced by the
       item tree for the first occurence, without calling fix_fields()
       on the second tree. Therefore there's nothing to clean up.), or
    If it is a grouped aggregate,
    2) there is no inner_sum_func_list, or
    3) the item is not an element in the inner_sum_func_list.
  */
  if (!fixed ||  // 1
      (m_window == nullptr && (aggr_select == nullptr ||
                               aggr_select->inner_sum_func_list == nullptr  // 2
                               || next_sum == nullptr)))                    // 3
    return false;

  if (m_window) {
    // Cleanup the reference for this window function from m_functions
    auto *ctx = pointer_cast<Cleanup_after_removal_context *>(arg);
    if (ctx != nullptr) {
      List_iterator<Item_sum> li(m_window->functions());
      Item *item = nullptr;
      while ((item = li++)) {
        if (item == this) {
          li.remove();
          break;
        }
      }
    }
  } else {
    if (next_sum == this)
      aggr_select->inner_sum_func_list = nullptr;
    else {
      Item_sum *prev;
      for (prev = this; prev->next_sum != this; prev = prev->next_sum)
        ;
      prev->next_sum = next_sum;
      next_sum = nullptr;

      if (aggr_select->inner_sum_func_list == this)
        aggr_select->inner_sum_func_list = prev;
    }
  }

  return false;
}

/// @note Please keep in sync with Item_func::eq().
bool Item_sum::eq(const Item *item, bool binary_cmp) const {
  /* Assume we don't have rtti */
  if (this == item) return true;
  if (item->type() != type() ||
      item->m_is_window_function != m_is_window_function)
    return false;
  const Item_sum *const item_sum = static_cast<const Item_sum *>(item);
  const enum Sumfunctype my_sum_func = sum_func();
  if (item_sum->sum_func() != my_sum_func || item_sum->m_window != m_window)
    return false;
  if (arg_count != item_sum->arg_count ||
      (my_sum_func != Item_sum::UDF_SUM_FUNC &&
       strcmp(func_name(), item_sum->func_name()) != 0) ||
      (my_sum_func == Item_sum::UDF_SUM_FUNC &&
       my_strcasecmp(system_charset_info, func_name(), item_sum->func_name())))
    return false;
  for (uint i = 0; i < arg_count; i++) {
    if (!args[i]->eq(item_sum->args[i], binary_cmp)) return false;
  }
  return true;
}

bool Item_sum::aggregate_check_distinct(uchar *arg) {
  DBUG_ASSERT(fixed);
  Distinct_check *dc = reinterpret_cast<Distinct_check *>(arg);

  if (dc->is_stopped(this)) return false;

  /*
    In the Standard, ORDER BY cannot contain an aggregate function;
    we are less strict, we allow it.
    However, if the aggregate in ORDER BY is not in the SELECT list, it
    might not be functionally dependent on all selected expressions, and thus
    might produce random order in combination with DISTINCT; then we reject
    it.

    One case where the aggregate is surely functionally dependent on the
    selected expressions, is if all GROUP BY expressions are in the SELECT
    list. But in that case DISTINCT is redundant and we have removed it in
    SELECT_LEX::prepare().
  */
  if (aggr_select == dc->select) return true;

  return false;
}

bool Item_sum::aggregate_check_group(uchar *arg) {
  DBUG_ASSERT(fixed);

  Group_check *gc = reinterpret_cast<Group_check *>(arg);

  if (gc->is_stopped(this)) return false;

  if (aggr_select != gc->select) {
    /*
      If aggr_select is inner to gc's select_lex, this aggregate function might
      reference some columns of gc, so we need to analyze its arguments.
      If it is outer, analyzing its arguments should not cause a problem, we
      will meet outer references which we will ignore.
    */
    return false;
  }

  if (gc->is_fd_on_source(this)) {
    gc->stop_at(this);
    return false;
  }

  return true;
}

bool Item_sum::has_aggregate_ref_in_group_by(uchar *) {
  /*
    We reject references to aggregates in the GROUP BY clause of the
    query block where the aggregation happens.
  */
  return aggr_select != nullptr && aggr_select->group_fix_field;
}

Field *Item_sum::create_tmp_field(bool, TABLE *table) {
  DBUG_TRACE;
  Field *field;
  switch (result_type()) {
    case REAL_RESULT:
      field = new (*THR_MALLOC) Field_double(
          max_length, maybe_null, item_name.ptr(), decimals, false, true);
      break;
    case INT_RESULT:
      field = new (*THR_MALLOC) Field_longlong(max_length, maybe_null,
                                               item_name.ptr(), unsigned_flag);
      break;
    case STRING_RESULT:
      return make_string_field(table);
    case DECIMAL_RESULT:
      field = Field_new_decimal::create_from_item(this);
      break;
    case ROW_RESULT:
    default:
      // This case should never be choosen
      DBUG_ASSERT(0);
      return nullptr;
  }
  if (field) field->init(table);
  return field;
}

bool Item_sum::collect_grouped_aggregates(uchar *arg) {
  auto *info = pointer_cast<Collect_grouped_aggregate_info *>(arg);

  if (m_is_window_function || info->m_break_off) return false;

  if (info->m_select == aggr_select && (used_tables() & OUTER_REF_TABLE_BIT)) {
    // This aggregate function aggregates in the transformed query block, but is
    // located inside a subquery. Currently, transform cannot get to this since
    // it doesn't descend into subqueries. This means we cannot substitute a
    // field for this aggregates, so break off. TODO.
    info->m_break_off = true;
    return false;
  }

  if (info->m_select != aggr_select) {
    // Aggregated either inside a subquery of the transformed query block or
    // outside of it. In either case, ignore it.
    return false;
  }

  for (auto e : info->list) {  // eliminate duplicates
    if (e == this) {
      return false;
    }
  }

  info->list.emplace_back(this);
  return false;
}

Item *Item_sum::replace_aggregate(uchar *arg) {
  auto *info = pointer_cast<Item::Aggregate_replacement *>(arg);
  if (info->m_target == this)
    return info->m_replacement;
  else
    return this;
}

bool Item_sum::collect_scalar_subqueries(uchar *arg) {
  if (!m_is_window_function) {
    auto *info = pointer_cast<Collect_scalar_subquery_info *>(arg);
    /// Don't walk below grouped aggregate functions
    if (info->is_stopped(this)) return false;
    info->stop_at(this);
  }
  return false;
}

bool Item_sum::collect_item_field_or_view_ref_processor(uchar *arg) {
  if (!m_is_window_function) {
    auto *info = pointer_cast<Collect_item_fields_or_view_refs *>(arg);
    /// Don't walk below grouped aggregate functions
    if (info->is_stopped(this)) return false;
    info->stop_at(this);
  }
  return false;
}

void Item_sum::update_used_tables() {
  if (forced_const) return;

  used_tables_cache = 0;
  // Re-accumulate all properties except three
  m_accum_properties &=
      (PROP_AGGREGATION | PROP_WINDOW_FUNCTION | PROP_ROLLUP_EXPR);

  for (uint i = 0; i < arg_count; i++) {
    args[i]->update_used_tables();
    used_tables_cache |= args[i]->used_tables();
    add_accum_properties(args[i]);
  }
  add_used_tables_for_aggr_func();
}

void Item_sum::fix_after_pullout(SELECT_LEX *parent_select,
                                 SELECT_LEX *removed_select) {
  // Cannot aggregate into a context that is merged up.
  DBUG_ASSERT(aggr_select != removed_select);

  // We may merge up a query block, if it is not the aggregating query context
  if (base_select == removed_select) base_select = parent_select;

  // Perform pullout of arguments to aggregate function
  used_tables_cache = 0;

  Item **arg, **arg_end;
  for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
    Item *const item = *arg;
    item->fix_after_pullout(parent_select, removed_select);
    used_tables_cache |= item->used_tables();
  }
  // Complete used_tables information by looking at aggregate function
  add_used_tables_for_aggr_func();
}

/**
  Add used_tables information for aggregate function, based on its aggregated
  query block.

  If the function is aggregated into its local context, it can
  be calculated only after evaluating the full join, thus it
  depends on all tables of this join. Otherwise, it depends on
  outer tables, even if its arguments args[] do not explicitly
  reference an outer table, like COUNT (*) or COUNT(123).

  Window functions are always evaluated in the local scope
  and depend on all tables involved in the join since they cannot
  be evaluated until after the join is completed.
*/

void Item_sum::add_used_tables_for_aggr_func() {
  used_tables_cache |= aggr_select == base_select || m_is_window_function
                           ? base_select->all_tables_map()
                           : OUTER_REF_TABLE_BIT;
  /*
    Aggregate functions are not allowed to be const, but they may
    be const-for-execution.
  */
  if (used_tables_cache == 0) used_tables_cache = INNER_TABLE_BIT;
}

Item *Item_sum::set_arg(uint i, THD *thd, Item *new_val) {
  thd->change_item_tree(args + i, new_val);
  return new_val;
}

int Item_sum::set_aggregator(Aggregator::Aggregator_type aggregator) {
  /*
    Dependent subselects may be executed multiple times, making
    set_aggregator to be called multiple times. The aggregator type
    will be the same, but it needs to be reset so that it is
    reevaluated with the new dependent data.
    This function may also be called multiple times during query optimization.
    In this case, the type may change, so we delete the old aggregator,
    and create a new one.
  */
  if (aggr && aggregator == aggr->Aggrtype()) {
    aggr->clear();
    return false;
  }

  destroy(aggr);
  switch (aggregator) {
    case Aggregator::DISTINCT_AGGREGATOR:
      aggr = new (*THR_MALLOC) Aggregator_distinct(this);
      break;
    case Aggregator::SIMPLE_AGGREGATOR:
      aggr = new (*THR_MALLOC) Aggregator_simple(this);
      break;
  };
  return aggr ? false : true;
}

void Item_sum::cleanup() {
  if (aggr) {
    destroy(aggr);
    aggr = nullptr;
  }
  Item_result_field::cleanup();
  forced_const = false;
}

bool Item_sum::fix_fields(THD *thd, Item **ref MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(fixed == 0);
  if (m_window != nullptr) {
    if (m_window_resolved) return false;

    if (Window::resolve_reference(thd, this, &m_window)) return true;

    m_window_resolved = true;
  }
  return false;
}

void Item_sum::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                              List<Item> &fields) {
  if (m_is_window_function) {
    for (auto &it : Bounds_checked_array<Item *>(args, arg_count))
      it->split_sum_func2(thd, ref_item_array, fields, &it, true);
  }
}

bool Item_sum::reset_wf_state(uchar *arg) {
  if (!m_is_window_function) return false;
  DBUG_TRACE;
  bool *do_framing = (bool *)arg;

  if (*do_framing) {
    if (framing()) clear();
  } else {
    if (!framing()) clear();
  }
  return false;
}

bool Item_sum::wf_common_init() {
  if (m_window->do_copy_null()) {
    DBUG_ASSERT(m_window->needs_buffering());
    null_value = maybe_null;
    return true;
  }
  if (m_window->at_partition_border() && !m_window->needs_buffering()) {
    clear();
  }
  return false;
}

/**
  Compare keys consisting of single field that cannot be compared as binary.

  Used by the Unique class to compare keys. Will do correct comparisons
  for all field types.

  @param    arg     Pointer to the relevant Field class instance
  @param    a       left key image
  @param    b       right key image
  @return   comparison result
    @retval < 0       if key1 < key2
    @retval = 0       if key1 = key2
    @retval > 0       if key1 > key2
*/

static int simple_str_key_cmp(const void *arg, const void *a, const void *b) {
  const Field *f = pointer_cast<const Field *>(arg);
  const uchar *key1 = pointer_cast<const uchar *>(a);
  const uchar *key2 = pointer_cast<const uchar *>(b);
  return f->cmp(key1, key2);
}

/**
  Correctly compare composite keys.

  Used by the Unique class to compare keys. Will do correct comparisons
  for composite keys with various field types.

  @param arg     Pointer to the relevant Aggregator_distinct instance
  @param a       left key image
  @param b       right key image
  @return        comparison result
    @retval <0       if key1 < key2
    @retval =0       if key1 = key2
    @retval >0       if key1 > key2
*/

int Aggregator_distinct::composite_key_cmp(const void *arg, const void *a,
                                           const void *b) {
  const Aggregator_distinct *aggr =
      static_cast<const Aggregator_distinct *>(arg);
  const uchar *key1 = pointer_cast<const uchar *>(a);
  const uchar *key2 = pointer_cast<const uchar *>(b);
  Field **field = aggr->table->field;
  Field **field_end = field + aggr->table->s->fields;
  uint32 *lengths = aggr->field_lengths;
  for (; field < field_end; ++field) {
    Field *f = *field;
    int len = *lengths++;
    int res = f->cmp(key1, key2);
    if (res) return res;
    key1 += len;
    key2 += len;
  }
  return 0;
}

static enum enum_field_types calc_tmp_field_type(
    enum enum_field_types table_field_type, Item_result result_type) {
  /* Adjust tmp table type according to the chosen aggregation type */
  switch (result_type) {
    case STRING_RESULT:
    case REAL_RESULT:
      if (table_field_type != MYSQL_TYPE_FLOAT)
        table_field_type = MYSQL_TYPE_DOUBLE;
      break;
    case INT_RESULT:
      table_field_type = MYSQL_TYPE_LONGLONG;
      /* fallthrough */
    case DECIMAL_RESULT:
      if (table_field_type != MYSQL_TYPE_LONGLONG)
        table_field_type = MYSQL_TYPE_NEWDECIMAL;
      break;
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  }
  return table_field_type;
}

/***************************************************************************/

/* Declarations for auxilary C-callbacks */

static int simple_raw_key_cmp(const void *arg, const void *key1,
                              const void *key2) {
  return memcmp(key1, key2, *(const uint *)arg);
}

static int item_sum_distinct_walk(void *element, element_count, void *item) {
  return ((Aggregator_distinct *)(item))->unique_walk_function(element);
}

/***************************************************************************/
/**
  Called before feeding the first row. Used to allocate/setup
  the internal structures used for aggregation.

  @param thd Thread descriptor
  @return status
    @retval false success
    @retval true  faliure

    Prepares Aggregator_distinct to process the incoming stream.
    Creates the temporary table and the Unique class if needed.
    Called by Item_sum::aggregator_setup()
*/

bool Aggregator_distinct::setup(THD *thd) {
  endup_done = false;
  /*
    Setup can be called twice for ROLLUP items. This is a bug.
    Please add DBUG_ASSERT(tree == 0) here when it's fixed.
  */
  if (tree || table || tmp_table_param) return false;

  DBUG_ASSERT(thd->lex->current_select() == item_sum->aggr_select);

  if (item_sum->setup(thd)) return true;
  if (item_sum->sum_func() == Item_sum::COUNT_FUNC ||
      item_sum->sum_func() == Item_sum::COUNT_DISTINCT_FUNC) {
    List<Item> list;
    SELECT_LEX *select_lex = item_sum->aggr_select;

    if (!(tmp_table_param = new (thd->mem_root) Temp_table_param)) return true;

    /**
      Create a table with an unique key over all parameters.
      If the list contains only const values, const_distinct
      is set to CONST_NOT_NULL to avoid creation of temp table
      and thereby counting as count(distinct of const values)
      will always be 1. If any of these const values is null,
      const_distinct is set to CONST_NULL to ensure aggregation
      does not happen.
     */
    uint const_items = 0;
    uint num_args = item_sum->get_arg_count();
    DBUG_ASSERT(num_args);
    for (uint i = 0; i < num_args; i++) {
      Item *item = item_sum->get_arg(i);
      if (list.push_back(item)) return true;  // out of memory
      if (item->const_item()) {
        const bool is_null = item->is_null();
        if (thd->is_error()) return true;  // is_null can fail
        if (is_null) {
          const_distinct = CONST_NULL;
          return false;
        } else
          const_items++;
      }
    }
    if (num_args == const_items) {
      const_distinct = CONST_NOT_NULL;
      return false;
    }
    count_field_types(select_lex, tmp_table_param, list, false, false);
    tmp_table_param->force_copy_fields = item_sum->has_force_copy_fields();
    DBUG_ASSERT(table == nullptr);
    /*
      Make create_tmp_table() convert BIT columns to BIGINT.
      This is needed because BIT fields store parts of their data in table's
      null bits, and we don't have methods to compare two table records, which
      is needed by Unique which is used when HEAP table is used.
    */
    {
      List_iterator_fast<Item> li(list);
      Item *item;
      while ((item = li++)) {
        if (item->type() == Item::FIELD_ITEM &&
            ((Item_field *)item)->field->type() == FIELD_TYPE_BIT)
          item->marker = Item::MARKER_BIT;
      }
    }
    if (!(table =
              create_tmp_table(thd, tmp_table_param, list, nullptr, true, false,
                               select_lex->active_options(), HA_POS_ERROR, "")))
      return true;
    table->file->ha_extra(HA_EXTRA_NO_ROWS);  // Don't update rows
    table->no_rows = true;
    if (table->hash_field) table->file->ha_index_init(0, false);

    if ((table->s->db_type() == temptable_hton ||
         table->s->db_type() == heap_hton) &&
        (table->s->blob_fields == 0)) {
      /*
        No blobs:
        set up a compare function and its arguments to use with Unique.
      */
      qsort2_cmp compare_key;
      void *cmp_arg;
      Field **field = table->field;
      Field **field_end = field + table->s->fields;
      bool all_binary = true;

      for (tree_key_length = 0; field < field_end; ++field) {
        Field *f = *field;
        enum enum_field_types type = f->type();
        tree_key_length += f->pack_length();
        if ((type == MYSQL_TYPE_VARCHAR) ||
            (!f->binary() &&
             (type == MYSQL_TYPE_STRING || type == MYSQL_TYPE_VAR_STRING))) {
          all_binary = false;
          break;
        }
      }
      if (all_binary) {
        cmp_arg = (void *)&tree_key_length;
        compare_key = simple_raw_key_cmp;
      } else {
        if (table->s->fields == 1) {
          /*
            If we have only one field, which is the most common use of
            count(distinct), it is much faster to use a simpler key
            compare method that can take advantage of not having to worry
            about other fields.
          */
          compare_key = simple_str_key_cmp;
          cmp_arg = (void *)table->field[0];
          /* tree_key_length has been set already */
        } else {
          uint32 *length;
          compare_key = composite_key_cmp;
          cmp_arg = (void *)this;
          field_lengths =
              (uint32 *)thd->alloc(table->s->fields * sizeof(uint32));
          for (tree_key_length = 0, length = field_lengths,
              field = table->field;
               field < field_end; ++field, ++length) {
            *length = (*field)->pack_length();
            tree_key_length += *length;
          }
        }
      }
      DBUG_ASSERT(tree == nullptr);
      tree = new (thd->mem_root) Unique(compare_key, cmp_arg, tree_key_length,
                                        item_sum->ram_limitation(thd));
      /*
        The only time tree_key_length could be 0 is if someone does
        count(distinct) on a char(0) field - stupid thing to do,
        but this has to be handled - otherwise someone can crash
        the server with a DoS attack
      */
      if (!tree) return true;
    }
    return false;
  } else {
    List<Create_field> field_list;
    Create_field field_def; /* field definition */
    Item *arg;
    DBUG_TRACE;
    /* It's legal to call setup() more than once when in a subquery */
    if (tree) return false;

    /*
      Virtual table and the tree are created anew on each re-execution of
      PS/SP. Hence all further allocations are performed in the runtime
      mem_root.
    */
    if (field_list.push_back(&field_def)) return true;

    item_sum->null_value = item_sum->maybe_null = true;
    item_sum->allow_group_via_temp_table = false;

    DBUG_ASSERT(item_sum->get_arg(0)->fixed);

    arg = item_sum->get_arg(0);
    if (arg->const_item()) {
      if (arg->update_null_value()) return true;
      if (arg->null_value) {
        const_distinct = CONST_NULL;
        return false;
      }
    }

    enum enum_field_types field_type =
        calc_tmp_field_type(arg->data_type(), arg->result_type());

    field_def.init_for_tmp_table(
        field_type, arg->max_length,
        field_type == MYSQL_TYPE_NEWDECIMAL
            ? min<unsigned int>(arg->decimals, DECIMAL_MAX_SCALE)
            : arg->decimals,
        arg->maybe_null, arg->unsigned_flag, 0);

    if (!(table = create_tmp_table_from_fields(thd, field_list))) return true;

    /* XXX: check that the case of CHAR(0) works OK */
    tree_key_length = table->s->reclength - table->s->null_bytes;

    /*
      Unique handles all unique elements in a tree until they can't fit
      in.  Then the tree is dumped to the temporary file. We can use
      simple_raw_key_cmp because the table contains numbers only; decimals
      are converted to binary representation as well.
    */
    tree = new (thd->mem_root)
        Unique(simple_raw_key_cmp, &tree_key_length, tree_key_length,
               item_sum->ram_limitation(thd));

    return tree == nullptr;
  }
}

/**
  Invalidate calculated value and clear the distinct rows.

  Frees space used by the internal data structures.
  Removes the accumulated distinct rows. Invalidates the calculated result.
*/

void Aggregator_distinct::clear() {
  endup_done = false;
  item_sum->clear();
  if (tree) tree->reset();
  /* tree and table can be both null only if const_distinct is enabled*/
  if (item_sum->sum_func() == Item_sum::COUNT_FUNC ||
      item_sum->sum_func() == Item_sum::COUNT_DISTINCT_FUNC) {
    if (!tree && table) {
      (void)table->empty_result_table();
      if (table->hash_field) table->file->ha_index_init(0, false);
    }
  } else {
    item_sum->null_value = true;
  }
}

/**
  Process incoming row.

  Add it to Unique/temp hash table if it's unique. Skip the row if
  not unique.
  Prepare Aggregator_distinct to process the incoming stream.
  Create the temporary table and the Unique class if needed.
  Called by Item_sum::aggregator_add().
  To actually get the result value in item_sum's buffers
  Aggregator_distinct::endup() must be called.

  @return status
    @retval false     success
    @retval true      failure
*/

bool Aggregator_distinct::add() {
  if (const_distinct == CONST_NULL) return false;

  if (item_sum->sum_func() == Item_sum::COUNT_FUNC ||
      item_sum->sum_func() == Item_sum::COUNT_DISTINCT_FUNC) {
    int error;

    if (const_distinct == CONST_NOT_NULL) {
      DBUG_ASSERT(item_sum->fixed == 1);
      Item_sum_count *sum = (Item_sum_count *)item_sum;
      sum->count = 1;
      return false;
    }
    if (copy_fields(tmp_table_param, table->in_use)) return true;
    if (copy_funcs(tmp_table_param, table->in_use)) return true;

    for (Field **field = table->field; *field; field++)
      if ((*field)->is_real_null()) return false;  // Don't count NULL

    if (tree) {
      /*
        The first few bytes of record (at least one) are just markers
        for deleted and NULLs. We want to skip them since they will
        bloat the tree without providing any valuable info. Besides,
        key_length used to initialize the tree didn't include space for them.
      */
      return tree->unique_add(table->record[0] + table->s->null_bytes);
    }

    if (!check_unique_constraint(table)) return false;
    if ((error = table->file->ha_write_row(table->record[0])) &&
        !table->file->is_ignorable_error(error))
      return true;
    return false;
  } else {
    item_sum->get_arg(0)->save_in_field(table->field[0], false);
    if (table->field[0]->is_null()) return false;
    DBUG_ASSERT(tree);
    item_sum->null_value = false;
    /*
      '0' values are also stored in the tree. This doesn't matter
      for SUM(DISTINCT), but is important for AVG(DISTINCT)
    */
    return tree->unique_add(table->field[0]->ptr);
  }
}

/**
  Calculate the aggregate function value.

  Since Distinct_aggregator::add() just collects the distinct rows,
  we must go over the distinct rows and feed them to the aggregation
  function before returning its value.
  This is what endup () does. It also sets the result validity flag
  endup_done to true so it will not recalculate the aggregate value
  again if the Item_sum hasn't been reset.
*/

void Aggregator_distinct::endup() {
  DBUG_TRACE;
  /* prevent consecutive recalculations */
  if (endup_done) return;

  if (const_distinct == CONST_NOT_NULL) {
    endup_done = true;
    return;
  }

  /* we are going to calculate the aggregate value afresh */
  item_sum->clear();

  /* The result will definitely be null : no more calculations needed */
  if (const_distinct == CONST_NULL) return;

  if (item_sum->sum_func() == Item_sum::COUNT_FUNC ||
      item_sum->sum_func() == Item_sum::COUNT_DISTINCT_FUNC) {
    DBUG_ASSERT(item_sum->fixed == 1);
    Item_sum_count *sum = (Item_sum_count *)item_sum;

    if (tree && tree->is_in_memory()) {
      /* everything fits in memory */
      sum->count = (longlong)tree->elements_in_tree();
      endup_done = true;
    }
    if (!tree) {
      /* there were blobs */
      table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
      if (table->file->ha_table_flags() & HA_STATS_RECORDS_IS_EXACT)
        sum->count = table->file->stats.records;
      else {
        // index must be closed before ha_records() is called
        if (table->file->inited) table->file->ha_index_or_rnd_end();
        ha_rows num_rows = 0;
        table->file->ha_records(&num_rows);
        // We have to initialize hash_index because update_sum_func needs it
        if (table->hash_field) table->file->ha_index_init(0, false);
        sum->count = static_cast<longlong>(num_rows);
      }
      endup_done = true;
    }
  }

  /*
    We don't have a tree only if 'setup()' hasn't been called;
    this is the case of sql_executor.cc:return_zero_rows.
  */
  if (tree && !endup_done) {
    /*
      All tree's values are not NULL.
      Note that value of field is changed as we walk the tree, in
      Aggregator_distinct::unique_walk_function, but it's always not NULL.
    */
    table->field[0]->set_notnull();
    /* go over the tree of distinct keys and calculate the aggregate value */
    use_distinct_values = true;
    tree->walk(item_sum_distinct_walk, (void *)this);
    use_distinct_values = false;
  }
  /* prevent consecutive recalculations */
  endup_done = true;
}

String *Item_sum_num::val_str(String *str) { return val_string_from_real(str); }

my_decimal *Item_sum_num::val_decimal(my_decimal *decimal_value) {
  return val_decimal_from_real(decimal_value);
}

String *Item_sum_int::val_str(String *str) { return val_string_from_int(str); }

my_decimal *Item_sum_int::val_decimal(my_decimal *decimal_value) {
  return val_decimal_from_int(decimal_value);
}

bool Item_sum_num::fix_fields(THD *thd, Item **ref) {
  if (super::fix_fields(thd, ref)) return true; /* purecov: inspected */

  if (init_sum_func_check(thd)) return true;

  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  maybe_null = false;

  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
    maybe_null |= args[i]->maybe_null;
  }

  // Set this value before calling resolve_type()
  null_value = true;

  if (resolve_type(thd)) return true;

  if (check_sum_func(thd, ref)) return true;

  fixed = true;
  return false;
}

bool Item_sum_bit::fix_fields(THD *thd, Item **ref) {
  DBUG_ASSERT(!fixed);

  if (super::fix_fields(thd, ref)) return true; /* purecov: inspected */

  if (init_sum_func_check(thd)) return true;

  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
  }

  if (resolve_type(thd)) return true;

  if (thd->is_error()) return true;

  if (check_sum_func(thd, ref)) return true;

  fixed = true;
  return false;
}

bool Item_sum_bit::resolve_type(THD *thd) {
  max_length = 0;
  if (bit_func_returns_binary(args[0], nullptr)) {
    hybrid_type = STRING_RESULT;
    for (uint i = 0; i < arg_count; i++)
      max_length = max(max_length, args[i]->max_length);
    if (max_length > (CONVERT_IF_BIGGER_TO_BLOB - 1)) {
      /*
        Implementation of Item_sum_bit_field expects that "result_field" is
        Field_varstring, not Field_blob, so that the buffer's content is easily
        modifiable.
        The above check guarantees that the tmp table code will choose a
        Field_varstring over a Field_blob, and an assertion is present in the
        constructor of Item_sum_bit_field to verify the Field.
      */
      my_error(ER_INVALID_BITWISE_AGGREGATE_OPERANDS_SIZE, MYF(0), func_name());
      return true;
    }
    m_digit_cnt_card = max_length * 8;
    /*
      One extra byte needed to store a per-group boolean flag
      if Item_sum_bit_field is used.
    */
    max_length++;
    set_data_type(MYSQL_TYPE_VARCHAR);
  } else {
    m_digit_cnt_card = DIGIT_CNT_CARD;
    hybrid_type = INT_RESULT;
    max_length = MAX_BIGINT_WIDTH + 1;
    set_data_type(MYSQL_TYPE_LONGLONG);
  }

  if (m_window != nullptr && !m_is_xor) {
    m_digit_cnt = new (thd->mem_root) ulonglong[m_digit_cnt_card];
    if (m_digit_cnt == nullptr) return true;
    std::memset(m_digit_cnt, 0, m_digit_cnt_card * sizeof(ulonglong));
  }

  maybe_null = false;
  null_value = false;
  result_field = nullptr;
  decimals = 0;
  unsigned_flag = true;

  return reject_geometry_args(arg_count, args, this);
}

void Item_sum_bit::remove_bits(const String *s1, ulonglong b1) {
  if (m_is_xor) {
    // XOR satisfies ((A OP B) OP B) == A, so inverting is easy:
    (void)add_bits(s1, b1);  // add_bits() cannot fail here.
    return;
  }

  const uchar *s1_c_p;
  uchar *value_bits;
  size_t buff_length;

  if (hybrid_type == STRING_RESULT) {
    s1_c_p = pointer_cast<const uchar *>(s1->ptr());
    value_bits = pointer_cast<uchar *>(value_buff.ptr());
    buff_length = value_buff.length() - 1;
  } else {
    s1_c_p = pointer_cast<const uchar *>(&b1);
    value_bits = pointer_cast<uchar *>(&bits);
    buff_length = sizeof(b1);
  }

  /*
    Execute the bitwise inverse operation. We could have executed this
    with a combination of std::bitset<sizeeof(ulonglong) * 8> and
    std::bitset<8>, as does add_bits(), but longer bits shifting
    to get bits in place might not be beneficial, so use just bytes.
    Microbenchmarking showed little difference.
  */
  for (size_t i = 0; i < buff_length; i++) {
    std::bitset<8> s1_bits(s1_c_p[i]);
    if (is_and()) {
      for (uint bit = 0; bit < 8; bit++) {
        m_digit_cnt[(i * 8) + bit] -= !s1_bits[bit];  // one less 0 in frame
        // Temporarily save updated bit in s1_bits:
        s1_bits.set(bit, m_digit_cnt[(i * 8) + bit] == 0);
      }
    } else  // OR
    {
      for (uint bit = 0; bit < 8; bit++) {
        m_digit_cnt[(i * 8) + bit] -= s1_bits[bit];  // one less 1 in frame
        s1_bits.set(bit, m_digit_cnt[(i * 8) + bit] > 0);
      }
    }

    value_bits[i] = s1_bits.to_ulong();
  }
}

/**
  Helper for Item_sum_bit::add_bits().

  Does value_bits = s1_c_p bit_op value_bits.

  @tparam Char_op  class offering a bit operation for a uchar: AND, OR
                   or XOR
  @tparam Int_op   class offering a bit operation for a ulonglong: ditto
  @param  buff_length  length of s1_c_p
  @param  s1_c_p             first argument of bit op
  @param[in,out] value_bits  second argument of bit op, and result
*/
template <class Char_op, class Int_op>
static inline void apply_bit_op(size_t buff_length, const uchar *s1_c_p,
                                uchar *value_bits) {
  auto int_op = Int_op();
  auto char_op = Char_op();
  size_t i = 0;
  // Execute the bitwise operation.
  while (i + sizeof(longlong) <= buff_length) {
    int8store(&value_bits[i],
              int_op(uint8korr(&s1_c_p[i]), uint8korr(&value_bits[i])));
    i += sizeof(longlong);
  }
  while (i < buff_length) {
    value_bits[i] = char_op(s1_c_p[i], value_bits[i]);
    i++;
  }
}

bool Item_sum_bit::add_bits(const String *s1, ulonglong b1) {
  DBUG_ASSERT(!args[0]->null_value);

  const uchar *s1_c_p;
  size_t buff_length;

  if (hybrid_type == STRING_RESULT) {
    DBUG_ASSERT(s1 != nullptr);
    s1_c_p = pointer_cast<const uchar *>(s1->ptr());
    buff_length = s1->length();
    DBUG_ASSERT(value_buff.length() > 0);
    // See if there has been a non-NULL value in this group/frame:
    const bool non_nulls = value_buff[value_buff.length() - 1];
    if (!non_nulls) {
      // Allocate length of argument + one extra byte for non_nulls
      if (value_buff.alloc(buff_length + 1)) {
        null_value = true;
        return true;
      }
      value_buff.length(buff_length + 1);
      // This is the first non-NULL value of the group, accumulate it.
      std::memcpy(&value_buff[0], s1->ptr(), buff_length);
      // Store that a non-NULL value has been seen.
      value_buff[buff_length] = 1;
    } else {
      /*
        If current value's length is different from the length of the
        accumulated value for this group, return error.
      */
      if ((value_buff.length() - 1) != buff_length) {
        my_error(ER_INVALID_BITWISE_OPERANDS_SIZE, MYF(0), func_name());
        return true;
      }

      // At this point the values should be not-null and have the same size.
      uchar *value_bits = pointer_cast<uchar *>(value_buff.ptr());
      if (m_is_xor)
        apply_bit_op<std::bit_xor<char>, std::bit_xor<ulonglong>>(
            buff_length, s1_c_p, value_bits);
      else if (is_and())
        apply_bit_op<std::bit_and<char>, std::bit_and<ulonglong>>(
            buff_length, s1_c_p, value_bits);
      else
        apply_bit_op<std::bit_or<char>, std::bit_or<ulonglong>>(
            buff_length, s1_c_p, value_bits);
    }
  } else {
    bits = m_is_xor ? (bits ^ b1) : (is_and() ? (bits & b1) : (bits | b1));
    // Consider the integer's bytes as a string for the rest of this function
    s1_c_p = pointer_cast<const uchar *>(&b1);
    buff_length = sizeof(b1);
  }

  /*
    For each bit in s1's bytes, update the bit's counter (m_digit_cnt) for
    that bit as follows: for BIT_AND, increment the counter if we see a zero in
    that bit; for BIT_OR increment the counter if we see a 1 in that bit.
    BIT_XOR doesn't need special treatment. And set functions don't use
    inversion so don't need the counter.
  */

  if (!m_is_window_function || m_is_xor) return false;

  for (size_t i = 0; i < buff_length; i++) {
    std::bitset<8> s1_bits(s1_c_p[i]);
    for (uint bit = 0; bit < 8; bit++) {
      DBUG_ASSERT((i * 8) + bit < m_digit_cnt_card);
      m_digit_cnt[(i * 8) + bit] += s1_bits[bit] ^ is_and();
    }
  }

  return false;
}

/**
  Executes the requested bitwise operation, using args[0] as first argument.
  If the result type is 'binary string':
   - takes value_buff as second argument and stores the result in value_buff.
   - sets the last character of value_buff to be a 'char' equal to
     1 if at least one non-NULL value has been seen for this group, to 0
     otherwise.
  If the result type is integer:
   - takes 'bits' as second argument and stores the result in 'bits'.
*/
bool Item_sum_bit::add() {
  char buff[CONVERT_IF_BIGGER_TO_BLOB - 1];

  const String *argval_s = nullptr;
  ulonglong argval_i = 0;

  String tmp_str(buff, sizeof(buff), &my_charset_bin);
  if (hybrid_type == STRING_RESULT) {
    argval_s = args[0]->val_str(&tmp_str);
  } else
    argval_i = (ulonglong)args[0]->val_int();

  /*
    Handle grouped aggregates first
  */
  if (!m_is_window_function) {
    if (args[0]->null_value)
      return false;  // NULLs are ignorable for the set function
    return add_bits(argval_s, argval_i);
  }

  /*
    The next section follows the normal pattern for optimized window function
    aggregates.
  */
  if (!args[0]->null_value) {
    if (m_window->do_inverse()) {
      DBUG_ASSERT(m_count > 0 && m_count > m_frame_null_count);
      remove_bits(argval_s, argval_i);
      m_count--;
    } else {
      if (add_bits(argval_s, argval_i))
        return true;  // error, typically different length
      m_count++;
    }
  } else {
    if (m_window->do_inverse()) {
      DBUG_ASSERT(m_count >= m_frame_null_count && m_frame_null_count > 0);
      m_count--;
      m_frame_null_count--;
    } else {
      m_count++;
      m_frame_null_count++;
    }
  }

  if (m_count == m_frame_null_count) {
    if (hybrid_type == STRING_RESULT) {
      // Mark that there are only NULLs; val_str() will set default value
      const size_t buff_length = value_buff.length() - 1;
      value_buff[buff_length] = 0;
    } else
      bits = reset_bits;
  }

  return false;
}

bool Item_sum_hybrid::fix_fields(THD *thd, Item **ref) {
  if (super::fix_fields(thd, ref)) return true; /* purecov: inspected */

  Item *item = args[0];

  if (init_sum_func_check(thd)) return true;

  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  // 'item' can be changed during fix_fields
  if ((!item->fixed && item->fix_fields(thd, args)) ||
      (item = args[0])->check_cols(1))
    return true;
  decimals = item->decimals;

  switch (hybrid_type = item->result_type()) {
    case INT_RESULT:
    case DECIMAL_RESULT:
    case STRING_RESULT:
      max_length = item->max_length;
      break;
    case REAL_RESULT:
      max_length = float_length(decimals);
      break;
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  };
  if (setup_hybrid(args[0], nullptr)) return true;
  /* MIN/MAX can return NULL for empty set indepedent of the used column */
  maybe_null = true;
  unsigned_flag = item->unsigned_flag;
  result_field = nullptr;
  null_value = true;
  if (resolve_type(thd)) return true;
  item = item->real_item();
  if (item->type() == Item::FIELD_ITEM)
    set_data_type(item->data_type());
  else if (item->data_type() == MYSQL_TYPE_JSON)
    set_data_type_json();
  else
    set_data_type_from_result(hybrid_type, max_length);

  if (check_sum_func(thd, ref)) return true;

  fixed = true;
  return false;
}

bool Item_sum_hybrid::setup_hybrid(Item *item, Item *value_arg) {
  value = Item_cache::get_cache(item);
  value->setup(item);
  value->store(value_arg);
  arg_cache = Item_cache::get_cache(item);
  if (arg_cache == nullptr) return true;
  arg_cache->setup(item);
  cmp = new (*THR_MALLOC) Arg_comparator();
  if (cmp == nullptr) return true;
  if (cmp->set_cmp_func(this, pointer_cast<Item **>(&arg_cache),
                        pointer_cast<Item **>(&value), false))
    return true;
  collation.set(item->collation);

  return false;
}

Field *Item_sum_hybrid::create_tmp_field(bool group, TABLE *table) {
  DBUG_TRACE;
  Field *field;
  if (args[0]->type() == Item::FIELD_ITEM) {
    field = ((Item_field *)args[0])->field;

    if ((field = create_tmp_field_from_field(current_thd, field,
                                             item_name.ptr(), table, nullptr)))
      field->flags &= ~NOT_NULL_FLAG;
    return field;
  }
  /*
    DATE/TIME fields have STRING_RESULT result types.
    In order to preserve field type, it's needed to handle DATE/TIME
    fields creations separately.
  */
  switch (args[0]->data_type()) {
    case MYSQL_TYPE_DATE:
      field = new (*THR_MALLOC) Field_newdate(maybe_null, item_name.ptr());
      break;
    case MYSQL_TYPE_TIME:
      field =
          new (*THR_MALLOC) Field_timef(maybe_null, item_name.ptr(), decimals);
      break;
    case MYSQL_TYPE_TIMESTAMP:
      field = new (*THR_MALLOC)
          Field_timestampf(maybe_null, item_name.ptr(), decimals);
      break;
    case MYSQL_TYPE_DATETIME:
      field = new (*THR_MALLOC)
          Field_datetimef(maybe_null, item_name.ptr(), decimals);
      break;
    default:
      return Item_sum::create_tmp_field(group, table);
  }
  if (field) field->init(table);
  return field;
}

/***********************************************************************
** reset and add of sum_func
***********************************************************************/

/**
  @todo
  check if the following assignments are really needed
*/
Item_sum_sum::Item_sum_sum(THD *thd, Item_sum_sum *item)
    : Item_sum_num(thd, item),
      hybrid_type(item->hybrid_type),
      curr_dec_buff(item->curr_dec_buff),
      m_count(item->m_count),
      m_frame_null_count(item->m_frame_null_count) {
  /* TODO: check if the following assignments are really needed */
  if (hybrid_type == DECIMAL_RESULT) {
    my_decimal2decimal(item->dec_buffs, dec_buffs);
    my_decimal2decimal(item->dec_buffs + 1, dec_buffs + 1);
  } else
    sum = item->sum;
}

Item *Item_sum_sum::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_sum(thd, this);
  return result;
}

void Item_sum_sum::clear() {
  null_value = true;
  if (hybrid_type == DECIMAL_RESULT) {
    curr_dec_buff = 0;
    my_decimal_set_zero(&dec_buffs[0]);
    my_decimal_set_zero(&dec_buffs[1]);
  } else
    sum = 0.0;
  m_count = 0;
  m_frame_null_count = 0;
}

bool Item_sum_sum::resolve_type(THD *) {
  DBUG_TRACE;
  maybe_null = true;
  null_value = true;
  decimals = args[0]->decimals;
  max_length = float_length(decimals);

  switch (args[0]->numeric_context_result_type()) {
    case REAL_RESULT:
      hybrid_type = REAL_RESULT;
      sum = 0.0;
      break;
    case INT_RESULT:
    case DECIMAL_RESULT: {
      /* SUM result can't be longer than length(arg) + length(MAX_ROWS) */
      int precision = args[0]->decimal_precision() + DECIMAL_LONGLONG_DIGITS;
      max_length = my_decimal_precision_to_length_no_truncation(
          precision, decimals, unsigned_flag);
      curr_dec_buff = 0;
      hybrid_type = DECIMAL_RESULT;
      my_decimal_set_zero(dec_buffs);
      break;
    }
    case STRING_RESULT:
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  }

  if (reject_geometry_args(arg_count, args, this)) return true;

  set_data_type_from_result(hybrid_type, max_length);

  DBUG_PRINT("info",
             ("Type: %s (%d, %d)",
              (hybrid_type == REAL_RESULT
                   ? "REAL_RESULT"
                   : hybrid_type == DECIMAL_RESULT
                         ? "DECIMAL_RESULT"
                         : hybrid_type == INT_RESULT ? "INT_RESULT"
                                                     : "--ILLEGAL!!!--"),
              max_length, (int)decimals));
  return false;
}

bool Item_sum_sum::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                      Window_evaluation_requirements *r) {
  bool result = Item_sum::check_wf_semantics(thd, select, r);
  if (hybrid_type == REAL_RESULT) {
    /*
      If the frame's start moves we will consider inversion, to remove the
      start rows. But, as we're using REAL_RESULT, and floating point
      arithmetic isn't mathematically exact, inversion may give different
      results from that of the non-optimized path. So, we use it only if the
      user allowed it:
    */
    const PT_frame *f = m_window->frame();
    if (f->m_from->m_border_type == WBT_VALUE_PRECEDING ||
        f->m_from->m_border_type == WBT_VALUE_FOLLOWING ||
        f->m_from->m_border_type == WBT_CURRENT_ROW) {
      r->row_optimizable &= !thd->variables.windowing_use_high_precision;
      r->range_optimizable &= !thd->variables.windowing_use_high_precision;
    }
  }
  return result;
}

bool Item_sum_sum::add() {
  DBUG_TRACE;
  DBUG_ASSERT(!m_is_window_function);
  if (hybrid_type == DECIMAL_RESULT) {
    my_decimal value;
    const my_decimal *val = aggr->arg_val_decimal(&value);
    if (!aggr->arg_is_null(true)) {
      my_decimal_add(E_DEC_FATAL_ERROR, dec_buffs + (curr_dec_buff ^ 1), val,
                     dec_buffs + curr_dec_buff);
      curr_dec_buff ^= 1;
      null_value = false;
    }
  } else {
    sum += aggr->arg_val_real();
    if (!aggr->arg_is_null(true)) null_value = false;
  }
  return false;
}

longlong Item_sum_sum::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (m_window != nullptr) {
    if (hybrid_type == REAL_RESULT) {
      return llrint_with_overflow_check(val_real());
    }
    longlong result = 0;
    my_decimal tmp;
    my_decimal *r = Item_sum_sum::val_decimal(&tmp);
    if (r != nullptr && !null_value)
      my_decimal2int(E_DEC_FATAL_ERROR, r, unsigned_flag, &result);
    return result;
  }

  if (aggr) aggr->endup();
  if (hybrid_type == DECIMAL_RESULT) {
    longlong result;
    my_decimal2int(E_DEC_FATAL_ERROR, dec_buffs + curr_dec_buff, unsigned_flag,
                   &result);
    return result;
  }
  return llrint_with_overflow_check(val_real());
}

double Item_sum_sum::val_real() {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return 0.0;

    if (hybrid_type == DECIMAL_RESULT) {
      my_decimal tmp;
      my_decimal *r = Item_sum_sum::val_decimal(&tmp);
      if (r != nullptr && !null_value)
        my_decimal2double(E_DEC_FATAL_ERROR, r, &sum);
    } else {
      double d = args[0]->val_real();

      if (!args[0]->null_value) {
        if (m_window->do_inverse()) {
          DBUG_ASSERT(m_count > 0 && m_count > m_frame_null_count);
          sum -= d;
          m_count--;
        } else {
          sum += d;
          m_count++;
        }
      } else {
        if (m_window->do_inverse()) {
          DBUG_ASSERT(m_count >= m_frame_null_count && m_frame_null_count > 0);
          m_count--;
          m_frame_null_count--;
        } else {
          m_count++;
          m_frame_null_count++;
        }
      }
      null_value = (m_count == m_frame_null_count);
    }
    return sum;
  } else {
    if (aggr) aggr->endup();
    if (hybrid_type == DECIMAL_RESULT)
      my_decimal2double(E_DEC_FATAL_ERROR, dec_buffs + curr_dec_buff, &sum);
    return sum;
  }
}

String *Item_sum_sum::val_str(String *str) {
  if (aggr) aggr->endup();
  if (hybrid_type == DECIMAL_RESULT) return val_string_from_decimal(str);
  return val_string_from_real(str);
}

my_decimal *Item_sum_sum::val_decimal(my_decimal *val) {
  if (m_is_window_function) {
    if (hybrid_type != DECIMAL_RESULT) return val_decimal_from_real(val);

    if (wf_common_init()) {
      my_decimal_set_zero(val);
      return null_value ? nullptr : val;
    }

    my_decimal *const argd = args[0]->val_decimal(&dec_buffs[0]);

    if (!args[0]->null_value) {
      my_decimal tmp;
      if (m_window->do_inverse()) {
        DBUG_ASSERT(m_count > 0 && m_count > m_frame_null_count);
        my_decimal_sub(E_DEC_FATAL_ERROR, &tmp, &dec_buffs[1], argd);
        tmp.swap(dec_buffs[1]);
        m_count--;
      } else {
        my_decimal_add(E_DEC_FATAL_ERROR, &tmp, &dec_buffs[1], argd);
        tmp.swap(dec_buffs[1]);
        m_count++;
      }
    } else {
      if (m_window->do_inverse()) {
        DBUG_ASSERT(m_count >= m_frame_null_count && m_frame_null_count > 0);
        m_count--;
        m_frame_null_count--;
      } else {
        m_count++;
        m_frame_null_count++;
      }
    }

    null_value = (m_count == m_frame_null_count);

    return &dec_buffs[1];
  }

  if (aggr) aggr->endup();
  if (hybrid_type == DECIMAL_RESULT) return (dec_buffs + curr_dec_buff);
  return val_decimal_from_real(val);
}

/**
  Aggregate a distinct row from the distinct hash table.

  Called for each row into the hash table 'Aggregator_distinct::table'.
  Includes the current distinct row into the calculation of the
  aggregate value. Uses the Field classes to get the value from the row.
  This function is used for AVG/SUM(DISTINCT). For COUNT(DISTINCT)
  it's called only when there are no blob arguments and the data don't
  fit into memory (so Unique makes persisted trees on disk).

  @param element     pointer to the row data.

  @return status
    @retval false     success
    @retval true      failure
*/

bool Aggregator_distinct::unique_walk_function(void *element) {
  DBUG_TRACE;
  memcpy(table->field[0]->ptr, element, tree_key_length);
  item_sum->add();
  return false;
}

Aggregator_distinct::~Aggregator_distinct() {
  if (tree) {
    destroy(tree);
    tree = nullptr;
  }
  if (table) {
    if (table->file) table->file->ha_index_or_rnd_end();
    free_tmp_table(table->in_use, table);
    table = nullptr;
  }
  if (tmp_table_param) {
    destroy(tmp_table_param);
    tmp_table_param = nullptr;
  }
}

my_decimal *Aggregator_simple::arg_val_decimal(my_decimal *value) {
  return item_sum->args[0]->val_decimal(value);
}

double Aggregator_simple::arg_val_real() {
  return item_sum->args[0]->val_real();
}

bool Aggregator_simple::arg_is_null(bool use_null_value) {
  Item **item = item_sum->args;
  const uint item_count = item_sum->arg_count;
  if (use_null_value) {
    for (uint i = 0; i < item_count; i++) {
      if (item[i]->null_value) return true;
    }
  } else {
    for (uint i = 0; i < item_count; i++) {
      if (item[i]->maybe_null && item[i]->is_null()) return true;
    }
  }
  return false;
}

my_decimal *Aggregator_distinct::arg_val_decimal(my_decimal *value) {
  return use_distinct_values ? table->field[0]->val_decimal(value)
                             : item_sum->args[0]->val_decimal(value);
}

double Aggregator_distinct::arg_val_real() {
  return use_distinct_values ? table->field[0]->val_real()
                             : item_sum->args[0]->val_real();
}

bool Aggregator_distinct::arg_is_null(bool use_null_value) {
  if (use_distinct_values) {
    const bool rc = table->field[0]->is_null();
    DBUG_ASSERT(!rc);  // NULLs are never stored in 'tree'
    return rc;
  }
  return use_null_value
             ? item_sum->args[0]->null_value
             : (item_sum->args[0]->maybe_null && item_sum->args[0]->is_null());
}

Item *Item_sum_count::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result = m_is_window_function ? this
                                      : new (thd->mem_root)
                                            Item_sum_count(thd, this);
  return result;
}

void Item_sum_count::clear() { count = 0; }

bool Item_sum_count::add() {
  DBUG_ASSERT(!m_is_window_function);
  if (aggr->arg_is_null(false)) return false;
  count++;
  return false;
}

longlong Item_sum_count::val_int() {
  DBUG_TRACE;
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return 0;

    DBUG_EXECUTE_IF(("enter"), {
      DBUG_PRINT("enter", ("Item_sum_count::val_int arg0 %p", args[0]));
      if (dynamic_cast<Item_field *>(args[0])) {
        Item_field *f = down_cast<Item_field *>(args[0]);
        DBUG_PRINT(("enter"), ("Item_sum_count::val_int field: %p ptr: %p",
                               f->field, f->field->ptr));
      }
    });

    if (args[0]->is_null()) {
      return count;
    }
    if (m_window->do_inverse()) {
      if (count > 0) count--;
    } else {
      count++;
    }
    null_value = false;

    return count;
  } else {
    if (aggr) aggr->endup();
    return count;
  }
}

void Item_sum_count::cleanup() {
  DBUG_TRACE;
  count = 0;
  Item_sum_int::cleanup();
}

bool Item_sum_avg::resolve_type(THD *thd) {
  if (Item_sum_sum::resolve_type(thd)) return true;

  maybe_null = true;
  null_value = true;
  prec_increment = thd->variables.div_precincrement;
  if (hybrid_type == DECIMAL_RESULT) {
    int precision = args[0]->decimal_precision() + prec_increment;
    decimals = min<uint>(args[0]->decimals + prec_increment, DECIMAL_MAX_SCALE);
    max_length = my_decimal_precision_to_length_no_truncation(
        precision, decimals, unsigned_flag);
    f_precision =
        min(precision + DECIMAL_LONGLONG_DIGITS, DECIMAL_MAX_PRECISION);
    f_scale = args[0]->decimals;
    dec_bin_size = my_decimal_get_binary_size(f_precision, f_scale);
  } else {
    decimals =
        min<uint>(args[0]->decimals + prec_increment, DECIMAL_NOT_SPECIFIED);
    max_length = args[0]->max_length + prec_increment;
  }
  set_data_type_from_result(hybrid_type, max_length);
  return false;
}

Item *Item_sum_avg::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_avg(thd, this);
  return result;
}

Field *Item_sum_avg::create_tmp_field(bool group, TABLE *table) {
  DBUG_TRACE;
  Field *field;
  if (group) {
    /*
      We must store both value and counter in the temporary table in one field.
      The easiest way is to do this is to store both value in a string
      and unpack on access.
    */
    field = new (*THR_MALLOC) Field_string(
        ((hybrid_type == DECIMAL_RESULT) ? dec_bin_size : sizeof(double)) +
            sizeof(longlong),
        false, item_name.ptr(), &my_charset_bin);
  } else if (hybrid_type == DECIMAL_RESULT)
    field = Field_new_decimal::create_from_item(this);
  else
    field = new (*THR_MALLOC) Field_double(
        max_length, maybe_null, item_name.ptr(), decimals, false, true);
  if (field) field->init(table);
  return field;
}

void Item_sum_avg::clear() { Item_sum_sum::clear(); }

bool Item_sum_avg::add() {
  DBUG_ASSERT(!m_is_window_function);
  if (Item_sum_sum::add()) return true;
  if (!aggr->arg_is_null(true)) m_count++;
  return false;
}

double Item_sum_avg::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return 0.0;

    double sum = Item_sum_sum::val_real();

    if (m_window->is_last_row_in_frame()) {
      int64 divisor = (m_window->needs_buffering()
                           ? m_window->rowno_in_frame() - m_frame_null_count
                           : m_count - m_frame_null_count);
      if (divisor > 0) sum = sum / ulonglong2double(divisor);
    }
    m_avg = sum;  // save
    return sum;
  } else {
    if (aggr) aggr->endup();
    if (!m_count) {
      null_value = true;
      return 0.0;
    }
    return Item_sum_sum::val_real() / ulonglong2double(m_count);
  }
}

my_decimal *Item_sum_avg::val_decimal(my_decimal *val) {
  DBUG_TRACE;
  my_decimal sum_buff, cnt;
  const my_decimal *sum_dec;
  DBUG_ASSERT(fixed == 1);

  if (m_is_window_function) {
    if (hybrid_type != DECIMAL_RESULT) {
      my_decimal *result = val_decimal_from_real(val);
      return result;
    }

    if (wf_common_init()) {
      my_decimal_set_zero(val);
      return null_value ? nullptr : val;
    }

    /*
      dec_buff[0]:   the current value
      dec_buff[1]:   holds sum so far
    */
    my_decimal *argd = args[0]->val_decimal(&dec_buffs[0]);

    if (!args[0]->null_value) {
      my_decimal tmp;
      if (m_window->do_inverse()) {
        DBUG_ASSERT(m_count > 0 && m_count > m_frame_null_count);
        my_decimal_sub(E_DEC_FATAL_ERROR, &tmp, &dec_buffs[1], argd);
        tmp.swap(dec_buffs[1]);
        m_count--;
      } else {
        my_decimal_add(E_DEC_FATAL_ERROR, &tmp, &dec_buffs[1], argd);
        tmp.swap(dec_buffs[1]);
        m_count++;
      }
    } else {
      if (m_window->do_inverse()) {
        DBUG_ASSERT(m_count >= m_frame_null_count && m_frame_null_count > 0);
        m_frame_null_count--;
        m_count--;
        // else no need to inverse if we only saw nulls
      } else {
        m_frame_null_count++;
        m_count++;
      }
    }

    int64 divisor = (m_window->needs_buffering()
                         ? m_window->rowno_in_frame() - m_frame_null_count
                         : m_count - m_frame_null_count);

    if (m_window->is_last_row_in_frame() && divisor > 0) {
      int2my_decimal(E_DEC_FATAL_ERROR, divisor, false, &cnt);
      my_decimal_div(E_DEC_FATAL_ERROR, &dec_buffs[0], &dec_buffs[1], &cnt,
                     prec_increment);
      val->swap(dec_buffs[0]);
    } else
      my_decimal2decimal(&dec_buffs[1], val);

    null_value = (m_count == m_frame_null_count);
    my_decimal tmp(*val);
    m_avg_dec.swap(tmp);  // save result
    return val;
  } else {
    if (aggr) aggr->endup();
    if (!m_count) {
      null_value = true;
      return nullptr;
    }

    /*
     For non-DECIMAL hybrid_type the division will be done in
     Item_sum_avg::val_real().
     */
    if (hybrid_type != DECIMAL_RESULT) {
      my_decimal *result = val_decimal_from_real(val);
      return result;
    }

    sum_dec = dec_buffs + curr_dec_buff;
    int2my_decimal(E_DEC_FATAL_ERROR, m_count, false, &cnt);
    my_decimal_div(E_DEC_FATAL_ERROR, val, sum_dec, &cnt, prec_increment);
    return val;
  }
}

String *Item_sum_avg::val_str(String *str) {
  if (aggr) aggr->endup();
  if (hybrid_type == DECIMAL_RESULT) return val_string_from_decimal(str);
  return val_string_from_real(str);
}

/*
  Standard deviation
*/

double Item_sum_std::val_real() {
  DBUG_ASSERT(fixed == 1);
  double nr = Item_sum_variance::val_real();

  DBUG_ASSERT(nr >= 0.0);

  return sqrt(nr);
}

Item *Item_sum_std::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_std(thd, this);
  return result;
}

/*
  Variance function has two implementations:
  The first implementation (Algorthm I - see Item_sum_variance) is based
  on Knuth's _TAoCP_, 3rd ed, volume 2, pg232. This alters the value at
  m, s, and increments count.
  The second implementation (Algorithm II - See Item_sum_variance)
  initializes 'm' to the first sample and uses a different formula to
  get s, s^2. This implementation allows incremental computation which
  is used in optimizing windowing functions with frames.
  By default, group aggregates and windowing functions use algorithm I.
  Algorithm II is used when user explicitly requests optimized way of
  calculating variance if frames are present.

  variance_fp_recurrence_next calculates the recurrence values m,s used in
  algorithm I.
  add_sample/remove_sample calculates the recurrence values m,s,s2 used in
  algorthm II.
*/

/**
  Calculates the next recurrence value s,s2 using the current sample
  as input. m is initialized to the first sample. Its not changed for the
  later calls.

  @param[in,out] m     recurrence value
  @param[in,out] s     recurrence value
  @param[in,out] s2    Square of the recurrence value s
  @param[in,out] count Number of rows for which m,s,s2 is calculated
  @param[in]     nr    Current sample
*/
static void add_sample(double *m, double *s, double *s2, ulonglong *count,
                       double nr) {
  *count += 1;
  if (*count == 1) {
    *m = nr;
    *s = 0;
    *s2 = 0;
  } else {
    *s += nr - *m;
    *s2 += (nr - *m) * (nr - *m);
  }
}

/**
  Removes the earlier calculated recurrence value s,s2 for current
  sample from the current s,s2 values. Called when do_inverse()
  is true.

  @param[in]     m     recurrence value
  @param[in,out] s     recurrence value
  @param[in,out] s2    Square of the recurrence value s
  @param[in,out] count Number of rows for which s,s2 is calculated
  @param[in]     nr    Current sample
*/
static void remove_sample(double *m, double *s, double *s2, ulonglong *count,
                          double nr) {
  *count -= 1;
  *s -= (nr - *m);
  *s2 -= (nr - *m) * (nr - *m);
}

/**
  Calculates the next recurrence value for current sample.

  @param[in,out] m     recurrence value
  @param[in,out] s     recurrence value
  @param[in,out] s2    Square of the recurrence value s
  @param[in,out] count Number of rows for which m,s,s2 is calculated
  @param[in] nr        Current sample
  @param[in] optimize  If set to true is Algorithm II is used to calculate
                       m,s and s2. Else Algorithm I is used to calculate
                       m,s.
  @param[in] inverse   If set to true, we use formulas from Algorithm II
                       to remove value calculated for s,s2 for sample "nr"
                       from the the current value of (s,s2).

  Note:
  variance_fp_recurrence_next and variance_fp_recurrence_result are used by
  Item_sum_variance and Item_variance_field classes, which are unrelated,
  and each need to calculate variance. The difference between the two
  classes is that the first is used for a mundane SELECT and when used with
  windowing functions, while the latter is used in a GROUPing SELECT.
*/
static void variance_fp_recurrence_next(double *m, double *s, double *s2,
                                        ulonglong *count, double nr,
                                        bool optimize, bool inverse) {
  if (optimize) {
    return inverse ? remove_sample(m, s, s2, count, nr)
                   : add_sample(m, s, s2, count, nr);
  } else {
    *count += 1;

    if (*count == 1) {
      *m = nr;
      *s = 0;
    } else {
      double m_kminusone = *m;
      *m = m_kminusone + (nr - m_kminusone) / (double)*count;
      *s = *s + (nr - m_kminusone) * (nr - *m);
    }
  }
}

/**
  Calculates variance using one of the two algorithms
  (See Item_sum_variance) as specified.

  @param[in] s                  recurrence value
  @param[in] s2                 Square of the recurrence value. Used
                                only by Algorithm II
  @param[in] count              Number of rows for which variance needs
                                to be calculated.
  @param[in] is_sample_variance true if calculating sample variance and
                                false if population variance.
  @param[in] optimize           true if algorthm II is used to calculate
                                variance.

  @retval                       returns calculated variance value

*/
static double variance_fp_recurrence_result(double s, double s2,
                                            ulonglong count,
                                            bool is_sample_variance,
                                            bool optimize) {
  if (count == 1) return 0.0;

  if (optimize) {
    double variance = is_sample_variance
                          ? ((s2 - (s * s) / count) / (count - 1))
                          : ((s2 - (s * s) / count) / count);

    /*
      In optimized code path, we might see a rounding error while
      calculating recurrence_s2 in remove_sample leading to negative
      variance (happens rarely). Fix this.
    */
    if (variance < 0.0) return 0.0;

    return variance;
  }

  return is_sample_variance ? (s / (count - 1)) : (s / count);
}

Item_sum_variance::Item_sum_variance(THD *thd, Item_sum_variance *item)
    : Item_sum_num(thd, item),
      hybrid_type(item->hybrid_type),
      count(item->count),
      sample(item->sample),
      prec_increment(item->prec_increment),
      optimize(item->optimize) {
  recurrence_m = item->recurrence_m;
  recurrence_s = item->recurrence_s;
  recurrence_s2 = item->recurrence_s2;
}

bool Item_sum_variance::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                           Window_evaluation_requirements *r) {
  bool result = Item_sum::check_wf_semantics(thd, select, r);
  const PT_frame *f = m_window->frame();
  if (f->m_from->m_border_type == WBT_VALUE_PRECEDING ||
      f->m_from->m_border_type == WBT_VALUE_FOLLOWING ||
      f->m_from->m_border_type == WBT_CURRENT_ROW) {
    optimize = !thd->variables.windowing_use_high_precision;
    r->row_optimizable &= optimize;
    r->range_optimizable &= optimize;
  } else
    r->row_optimizable = r->range_optimizable = optimize = false;

  return result;
}

bool Item_sum_variance::resolve_type(THD *) {
  DBUG_TRACE;
  maybe_null = true;
  null_value = true;

  /*
    According to the SQL2003 standard (Part 2, Foundations; sec 10.9,
    aggregate function; paragraph 7h of Syntax Rules), "the declared
    type of the result is an implementation-defined aproximate numeric
    type.
  */
  set_data_type_double();
  hybrid_type = REAL_RESULT;

  if (reject_geometry_args(arg_count, args, this)) return true;
  DBUG_PRINT("info", ("Type: REAL_RESULT (%d, %d)", max_length, (int)decimals));
  return false;
}

Item *Item_sum_variance::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result = m_is_window_function ? this
                                      : new (thd->mem_root)
                                            Item_sum_variance(thd, this);
  return result;
}

/**
  Create a new field to match the type of value we're expected to yield.
  If we're grouping, then we need some space to serialize variables into, to
  pass around.
*/
Field *Item_sum_variance::create_tmp_field(bool group, TABLE *table) {
  DBUG_TRACE;
  Field *field;
  if (group) {
    /*
      We must store both value and counter in the temporary table in one field.
      The easiest way is to do this is to store both value in a string
      and unpack on access.
    */
    field =
        new (*THR_MALLOC) Field_string(sizeof(double) * 2 + sizeof(longlong),
                                       false, item_name.ptr(), &my_charset_bin);
  } else
    field = new (*THR_MALLOC) Field_double(
        max_length, maybe_null, item_name.ptr(), decimals, false, true);

  if (field != nullptr) field->init(table);

  return field;
}

void Item_sum_variance::clear() { count = 0; }

bool Item_sum_variance::add() {
  /*
    Why use a temporary variable?  We don't know if it is null until we
    evaluate it, which has the side-effect of setting null_value .
  */
  double nr = args[0]->val_real();

  if (!args[0]->null_value)
    variance_fp_recurrence_next(
        &recurrence_m, &recurrence_s, &recurrence_s2, &count, nr, optimize,
        m_is_window_function ? m_window->do_inverse() : false);
  return false;
}

double Item_sum_variance::val_real() {
  DBUG_ASSERT(fixed == 1);

  /*
    'sample' is a 1/0 boolean value.  If it is 1/true, id est this is a sample
    variance call, then we should set nullness when the count of the items
    is one or zero.  If it's zero, i.e. a population variance, then we only
    set nullness when the count is zero.

    Another way to read it is that 'sample' is the numerical threshhold, at and
    below which a 'count' number of items is called NULL.
  */
  DBUG_ASSERT((sample == 0) || (sample == 1));
  if (m_is_window_function) {
    if (wf_common_init()) return 0.0;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for a window function, which does not use Aggregator, it has to be called
      here.
    */
    add();
  }
  if (count <= sample) {
    null_value = true;
    return 0.0;
  }

  null_value = false;
  return variance_fp_recurrence_result(recurrence_s, recurrence_s2, count,
                                       sample, optimize);
}

my_decimal *Item_sum_variance::val_decimal(my_decimal *dec_buf) {
  DBUG_ASSERT(fixed == 1);
  return val_decimal_from_real(dec_buf);
}

void Item_sum_variance::reset_field() {
  double nr;
  uchar *res = result_field->ptr;

  nr = args[0]->val_real(); /* sets null_value as side-effect */

  if (args[0]->null_value)
    memset(res, 0, sizeof(double) * 2 + sizeof(longlong));
  else {
    /* Serialize format is (double)m, (double)s, (longlong)count */
    ulonglong tmp_count;
    double tmp_s;
    float8store(res, nr); /* recurrence variable m */
    tmp_s = 0.0;
    float8store(res + sizeof(double), tmp_s);
    tmp_count = 1;
    int8store(res + sizeof(double) * 2, tmp_count);
  }
}

void Item_sum_variance::update_field() {
  ulonglong field_count;
  uchar *res = result_field->ptr;

  double nr = args[0]->val_real(); /* sets null_value as side-effect */

  if (args[0]->null_value) return;

  /* Serialize format is (double)m, (double)s, (longlong)count */
  double field_recurrence_m = float8get(res);
  double field_recurrence_s = float8get(res + sizeof(double));
  field_count = sint8korr(res + sizeof(double) * 2);

  variance_fp_recurrence_next(&field_recurrence_m, &field_recurrence_s, nullptr,
                              &field_count, nr, false, false);

  float8store(res, field_recurrence_m);
  float8store(res + sizeof(double), field_recurrence_s);
  res += sizeof(double) * 2;
  int8store(res, field_count);
}

/* min & max */

void Item_sum_hybrid::clear() {
  value->clear();
  value->store(args[0]);
  arg_cache->clear();
  null_value = true;
  m_cnt = 0;
  m_saved_last_value_at = 0;
}

bool Item_sum_hybrid::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                         Window_evaluation_requirements *r) {
  bool result = Item_sum::check_wf_semantics(thd, select, r);

  const PT_order_list *order = m_window->effective_order_by();
  if (order != nullptr) {
    ORDER *o = order->value.first;
    // The logic below (see class's doc) makes sense only for MIN and MAX
    DBUG_ASSERT(sum_func() == MIN_FUNC || sum_func() == MAX_FUNC);
    if (o->item_ptr->real_item()->eq(args[0]->real_item(), false)) {
      if (r->row_optimizable || r->range_optimizable) {
        m_optimize = true;
        value->setup(args[0]);  // no comparisons needed
        if (o->direction == ORDER_ASC) {
          r->opt_first_row = m_is_min ? true : r->opt_first_row;
          r->opt_last_row = !m_is_min ? true : r->opt_last_row;
          m_want_first = m_is_min;
          m_nulls_first = true;
        } else {
          r->opt_last_row = m_is_min ? true : r->opt_last_row;
          r->opt_first_row = !m_is_min ? true : r->opt_first_row;
          m_want_first = !m_is_min;
          m_nulls_first = false;
        }
      }
    }
  }
  if (!m_optimize) {
    r->row_optimizable = false;
    r->range_optimizable = false;
  }
  return result;
}

bool Item_sum_hybrid::compute() {
  m_cnt++;

  if (m_window->do_inverse()) {
    null_value = true;
    return true;
  }

  /*
    We have four cases:
               m_want_first  m_nulls_first
          (1)       F            F
          (2)       F            T
          (3)       T            F
          (4)       T            T

    Since we want non-null values if present, special handling is needed for
    (1) and (4), i.e. those cases where we have to potentially[1] ignore nulls
    before (4) or after (1) a non-null value in a frame.

    [1] If we have a frame stretching back or forward to a non-null.
  */
  if (m_want_first != m_nulls_first) {
    // Cases (2) and (3): same structure as Item_first_last_value::compute
    if ((m_window->needs_buffering() &&
         (((m_window->rowno_in_frame() == 1 && m_want_first) ||
           (m_window->is_last_row_in_frame() && !m_want_first)) ||
          m_window->rowno_being_visited() == 0 /* No FROM; one const row */)) ||
        (!m_window->needs_buffering() &&
         ((m_want_first && m_cnt == 1) || !m_want_first))) {
      value->cache_value();
      null_value = value->null_value;
    }
  } else if (m_want_first) {
    /*
      Case (4) Handle potential nulls before non-null. If we don't find a
      non-NULL value on the first row of the frame, try on succeeding rows.
      If the first row in the frame never is a non-NULL, the value is still set
      when evaluating the last row (which will cover all rows in the frame at
      one time or another); in the priming (non-optimized) loop or in the
      optimized loop; see more below.
    */
    if ((m_window->needs_buffering() &&
         ((m_window->rowno_in_frame() == 1) ||
          (null_value && m_window->rowno_in_frame() > 1) ||
          m_window->rowno_being_visited() == 0 /* No FROM; one const row */)) ||
        (!m_window->needs_buffering() && m_cnt == 1)) {
      DBUG_ASSERT(m_nulls_first);
      value->store_and_cache(args[0]);
      null_value = value->null_value;

      if (!null_value) {
        /*
          In optimized mode with a moving frame, the visit pattern[1] is:
             invert N-1, read N (new first).. read M (new last).

          [1] in process_buffered_windowing_record

          The first time we find a non-null value can actually be[2] when we,
          in optimized mode, have discovered that we have a now last row,
          cf. the branch in [1]:

             if (new_last_row) ..

          Since this will be first non-null row in this case, it will be
          the MIN (or MAX is descending sort) until it goes out of frame.

          When we next read the new first in a moving frame (N+1), if the value
          if NULL, we already have the value cached, and use it, see "else if".

          [2] if the frame for the first row in the partition didn't see a non-
          NULL row under priming (non-optimized loop in [1]).
        */
        arg_cache->store_and_cache(value);
      } else if (!arg_cache->null_value) {
        value->store_and_cache(arg_cache);
        null_value = value->null_value;
      }
    }
  } else {
    /*
      Case (1) Handle potential nulls after non-null. If we see a NULL, reuse
      any earlier seen non-NULL value as long as that value is still in
      frame.
    */
    if ((m_window->needs_buffering() &&
         ((m_window->is_last_row_in_frame()) ||
          m_window->rowno_being_visited() == 0 /* No FROM; one const row */)) ||
        (!m_window->needs_buffering())) {
      value->store_and_cache(args[0]);
      null_value = value->null_value;
      const int64 frame_start =
          (m_window->rowno_being_visited() - m_window->rowno_in_frame() + 1);

      if (!value->null_value &&
          m_window->rowno_being_visited() > m_saved_last_value_at) {
        arg_cache->store_and_cache(value);
        m_saved_last_value_at = m_window->rowno_being_visited();
      } else if (m_saved_last_value_at >= frame_start) {
        DBUG_ASSERT(!m_nulls_first);
        value->store_and_cache(arg_cache);
        null_value = value->null_value;
      }
    }
  }
  return null_value || current_thd->is_error();
}

double Item_sum_hybrid::val_real() {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return 0.0;
    bool ret = false;
    m_optimize ? ret = compute() : add();
    if (ret) return error_real();
  }
  if (null_value) return 0.0;
  double retval = value->val_real();
  if ((null_value = value->null_value)) DBUG_ASSERT(retval == 0.0);
  return retval;
}

longlong Item_sum_hybrid::val_int() {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return 0;
    bool ret = false;
    m_optimize ? ret = compute() : add();
    if (ret) return error_int();
  }
  if (null_value) return 0;
  longlong retval = value->val_int();
  if ((null_value = value->null_value)) DBUG_ASSERT(retval == 0);
  return retval;
}

longlong Item_sum_hybrid::val_time_temporal() {
  DBUG_ASSERT(fixed == 1);
  if (null_value) return 0;
  longlong retval = value->val_time_temporal();
  if ((null_value = value->null_value)) DBUG_ASSERT(retval == 0);
  return retval;
}

longlong Item_sum_hybrid::val_date_temporal() {
  DBUG_ASSERT(fixed == 1);
  if (null_value) return 0;
  longlong retval = value->val_date_temporal();
  if ((null_value = value->null_value)) DBUG_ASSERT(retval == 0);
  return retval;
}

my_decimal *Item_sum_hybrid::val_decimal(my_decimal *val) {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) {
      my_decimal_set_zero(val);
      return null_value ? nullptr : val;
    }
    bool ret = false;
    m_optimize ? ret = compute() : add();
    if (ret) return nullptr;
  }
  if (null_value) return nullptr;
  my_decimal *retval = value->val_decimal(val);
  if ((null_value = value->null_value))
    DBUG_ASSERT(retval == nullptr || my_decimal_is_zero(retval));
  return retval;
}

bool Item_sum_hybrid::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  DBUG_ASSERT(fixed == 1);
  if (null_value) return true;
  return (null_value = value->get_date(ltime, fuzzydate));
}

bool Item_sum_hybrid::get_time(MYSQL_TIME *ltime) {
  DBUG_ASSERT(fixed == 1);
  if (null_value) return true;
  return (null_value = value->get_time(ltime));
}

String *Item_sum_hybrid::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return nullptr;
    bool ret = false;
    m_optimize ? ret = compute() : add();
    if (ret) return nullptr;
  }
  if (null_value) return nullptr;

  String *retval = value->val_str(str);
  if ((null_value = value->null_value)) DBUG_ASSERT(retval == nullptr);
  return retval;
}

bool Item_sum_hybrid::val_json(Json_wrapper *wr) {
  DBUG_ASSERT(fixed);
  if (null_value) return false;
  bool ok = value->val_json(wr);
  null_value = value->null_value;
  return ok;
}

void Item_sum_hybrid::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                                     List<Item> &fields) {
  super::split_sum_func(thd, ref_item_array, fields);
  /*
    Grouped aggregate functions used as arguments to windowing functions get
    replaced with aggregate ref's in split_sum_func. So need to redo the cache
    setup.
  */
  arg_cache->setup(args[0]);
}

void Item_sum_hybrid::cleanup() {
  DBUG_TRACE;
  Item_sum::cleanup();
  forced_const = false;
  destroy(cmp);
  cmp = nullptr;
  /*
    by default it is true to avoid true reporting by
    Item_func_not_all/Item_func_nop_all if this item was never called.

    no_rows_in_result() set it to false if was not results found.
    If some results found it will be left unchanged.
  */
  was_values = true;
}

void Item_sum_hybrid::no_rows_in_result() {
  was_values = false;
  clear();
}

Item *Item_sum_hybrid::copy_or_same(THD *thd) {
  if (m_is_window_function) return this;
  Item_sum_hybrid *item = clone_hybrid(thd);
  if (item == nullptr || item->setup_hybrid(args[0], value)) return nullptr;
  return item;
}

Item_sum_min *Item_sum_min::clone_hybrid(THD *thd) const {
  return new (thd->mem_root) Item_sum_min(thd, this);
}

Item_sum_max *Item_sum_max::clone_hybrid(THD *thd) const {
  return new (thd->mem_root) Item_sum_max(thd, this);
}

/**
  Checks if a value should replace the minimum or maximum value seen so far in
  the MIN and MAX aggregate functions.

  @param comparison_result  the result of comparing the current value with the
                            min/max value seen so far (negative if it's
                            smaller, 0 if it's equal, positive if it's greater)
  @param is_min  true if called by MIN, false if called by MAX

  @return true if the current value should replace the min/max value seen so far
*/
static bool min_max_best_so_far(int comparison_result, bool is_min) {
  return is_min ? comparison_result < 0 : comparison_result > 0;
}

bool Item_sum_hybrid::add() {
  arg_cache->cache_value();
  if (!arg_cache->null_value &&
      (null_value || min_max_best_so_far(cmp->compare(), m_is_min))) {
    value->store(arg_cache);
    value->cache_value();
    null_value = false;
  }
  return false;
}

String *Item_sum_bit::val_str(String *str) {
  if (m_is_window_function) {
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for a window function, which does not use Aggregator, it has to be called
      here.
    */
    if (!wf_common_init()) {
      if (add()) return str;
    }
  }

  if (hybrid_type == INT_RESULT) return val_string_from_int(str);

  DBUG_ASSERT(value_buff.length() > 0);
  const bool non_nulls = value_buff[value_buff.length() - 1];
  // If the group has no non-NULLs repeat the default value max_length times.
  if (!non_nulls) {
    str->length(0);
    if (str->fill(max_length - 1, static_cast<char>(reset_bits)))
      return error_str();
    str->set_charset(&my_charset_bin);
  } else {
    // Prepare the result (skip the flag at the end)
    if (str->copy(value_buff.ptr(), value_buff.length() - 1, &my_charset_bin))
      return error_str();
  }

  return str;
}

bool Item_sum_bit::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  if (hybrid_type == INT_RESULT)
    return get_date_from_int(ltime, fuzzydate);
  else
    return get_date_from_string(ltime, fuzzydate);
}

bool Item_sum_bit::get_time(MYSQL_TIME *ltime) {
  if (hybrid_type == INT_RESULT)
    return get_time_from_int(ltime);
  else
    return get_time_from_string(ltime);
}

my_decimal *Item_sum_bit::val_decimal(my_decimal *dec_buf) {
  if (m_is_window_function) {
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for a window function, which does not use Aggregator, it has be called
      here.
    */
    if (!wf_common_init()) add();
  }

  if (hybrid_type == INT_RESULT)
    return val_decimal_from_int(dec_buf);
  else
    return val_decimal_from_string(dec_buf);
}

double Item_sum_bit::val_real() {
  DBUG_ASSERT(fixed);

  if (m_is_window_function) {
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for a window function, which does not use Aggregator, it has be called
      here.
    */
    if (!wf_common_init()) add();
  }

  if (hybrid_type == INT_RESULT) return bits;
  String *res;
  if (!(res = val_str(&str_value))) return 0.0;

  int ovf_error;
  const char *from = res->ptr();
  size_t len = res->length();
  const char *end = from + len;
  return my_strtod(from, &end, &ovf_error);
}
/* bit_or and bit_and */

longlong Item_sum_bit::val_int() {
  DBUG_ASSERT(fixed);
  if (m_is_window_function) {
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for a window function, which does not use Aggregator, it has be called
      here.
    */
    if (!wf_common_init()) add();
  }

  if (hybrid_type == INT_RESULT) return (longlong)bits;

  String *res;
  if (!(res = val_str(&str_value))) return 0;

  int ovf_error;
  const char *from = res->ptr();
  size_t len = res->length();
  const char *end = from + len;
  return my_strtoll10(from, &end, &ovf_error);
}

void Item_sum_bit::clear() {
  if (hybrid_type == INT_RESULT)
    bits = reset_bits;
  else {
    // Prepare value_buff for a new group: no non-NULLs seen.
    value_buff[value_buff.length() - 1] = 0;
  }
  m_count = 0;
  m_frame_null_count = 0;
  if (m_digit_cnt != nullptr) {
    std::memset(m_digit_cnt, 0, m_digit_cnt_card * sizeof(ulonglong));
  }
}

Item *Item_sum_or::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_or(thd, this);
  return result;
}

Item *Item_sum_xor::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_xor(thd, this);
  return result;
}

Item *Item_sum_and::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result =
      m_is_window_function ? this : new (thd->mem_root) Item_sum_and(thd, this);
  return result;
}

/************************************************************************
** reset result of a Item_sum with is saved in a tmp_table
*************************************************************************/

void Item_sum_num::reset_field() {
  double nr = args[0]->val_real();
  uchar *res = result_field->ptr;

  if (maybe_null) {
    if (args[0]->null_value) {
      nr = 0.0;
      result_field->set_null();
    } else
      result_field->set_notnull();
  }
  float8store(res, nr);
}

void Item_sum_hybrid::reset_field() {
  switch (hybrid_type) {
    case STRING_RESULT: {
      if (args[0]->is_temporal()) {
        longlong nr = args[0]->val_temporal_by_field_type();
        if (maybe_null) {
          if (args[0]->null_value) {
            nr = 0;
            result_field->set_null();
          } else
            result_field->set_notnull();
        }
        result_field->store_packed(nr);
        break;
      }

      char buff[MAX_FIELD_WIDTH];
      String tmp(buff, sizeof(buff), result_field->charset()), *res;

      res = args[0]->val_str(&tmp);
      if (args[0]->null_value) {
        result_field->set_null();
        result_field->reset();
      } else {
        result_field->set_notnull();
        result_field->store(res->ptr(), res->length(), tmp.charset());
      }
      break;
    }
    case INT_RESULT: {
      longlong nr = args[0]->val_int();

      if (maybe_null) {
        if (args[0]->null_value) {
          nr = 0;
          result_field->set_null();
        } else
          result_field->set_notnull();
      }
      result_field->store(nr, unsigned_flag);
      break;
    }
    case REAL_RESULT: {
      double nr = args[0]->val_real();

      if (maybe_null) {
        if (args[0]->null_value) {
          nr = 0.0;
          result_field->set_null();
        } else
          result_field->set_notnull();
      }
      result_field->store(nr);
      break;
    }
    case DECIMAL_RESULT: {
      my_decimal value_buff, *arg_dec = args[0]->val_decimal(&value_buff);

      if (maybe_null) {
        if (args[0]->null_value)
          result_field->set_null();
        else
          result_field->set_notnull();
      }
      /*
        We must store zero in the field as we will use the field value in
        add()
      */
      if (!arg_dec)  // Null
        arg_dec = &decimal_zero;
      result_field->store_decimal(arg_dec);
      break;
    }
    case ROW_RESULT:
    default:
      DBUG_ASSERT(0);
  }
}

void Item_sum_sum::reset_field() {
  DBUG_ASSERT(aggr->Aggrtype() != Aggregator::DISTINCT_AGGREGATOR);
  if (hybrid_type == DECIMAL_RESULT) {
    my_decimal value, *arg_val = args[0]->val_decimal(&value);
    if (!arg_val)  // Null
      arg_val = &decimal_zero;
    result_field->store_decimal(arg_val);
  } else {
    DBUG_ASSERT(hybrid_type == REAL_RESULT);
    double nr = args[0]->val_real();  // Nulls also return 0
    float8store(result_field->ptr, nr);
  }
  if (args[0]->null_value)
    result_field->set_null();
  else
    result_field->set_notnull();
}

void Item_sum_count::reset_field() {
  uchar *res = result_field->ptr;
  longlong nr = 0;
  DBUG_ASSERT(aggr->Aggrtype() != Aggregator::DISTINCT_AGGREGATOR);

  if (!args[0]->maybe_null || !args[0]->is_null()) nr = 1;
  int8store(res, nr);
}

void Item_sum_avg::reset_field() {
  uchar *res = result_field->ptr;
  DBUG_ASSERT(aggr->Aggrtype() != Aggregator::DISTINCT_AGGREGATOR);
  if (hybrid_type == DECIMAL_RESULT) {
    longlong tmp;
    my_decimal value, *arg_dec = args[0]->val_decimal(&value);
    if (args[0]->null_value) {
      arg_dec = &decimal_zero;
      tmp = 0;
    } else
      tmp = 1;
    my_decimal2binary(E_DEC_FATAL_ERROR, arg_dec, res, f_precision, f_scale);
    res += dec_bin_size;
    int8store(res, tmp);
  } else {
    double nr = args[0]->val_real();

    if (args[0]->null_value)
      memset(res, 0, sizeof(double) + sizeof(longlong));
    else {
      longlong tmp = 1;
      float8store(res, nr);
      res += sizeof(double);
      int8store(res, tmp);
    }
  }
}

void Item_sum_bit::reset_field() {
  reset_and_add();
  if (hybrid_type == INT_RESULT)
    // Store the result in result_field
    result_field->store(bits, unsigned_flag);
  else
    result_field->store(value_buff.ptr(), value_buff.length(),
                        value_buff.charset());
}

void Item_sum_bit::update_field() {
  if (hybrid_type == INT_RESULT) {
    // Restore previous value to bits
    bits = result_field->val_int();
    // Add the current value to the group determined value.
    add();
    // Store the value in the result_field
    result_field->store(bits, unsigned_flag);
  } else  // hybrid_type == STRING_RESULT
  {
    // Restore previous value to result_field
    result_field->val_str(&value_buff);
    // Add the current value to the previously determined one
    add();
    // Store the value in the result_field
    result_field->store(value_buff.ptr(), value_buff.length(),
                        default_charset());
  }
}

/**
  calc next value and merge it with field_value.
*/

void Item_sum_sum::update_field() {
  DBUG_TRACE;
  DBUG_ASSERT(aggr->Aggrtype() != Aggregator::DISTINCT_AGGREGATOR);
  if (hybrid_type == DECIMAL_RESULT) {
    my_decimal value, *arg_val = args[0]->val_decimal(&value);
    if (!args[0]->null_value) {
      if (!result_field->is_null()) {
        my_decimal field_value,
            *field_val = result_field->val_decimal(&field_value);
        my_decimal_add(E_DEC_FATAL_ERROR, dec_buffs, arg_val, field_val);
        result_field->store_decimal(dec_buffs);
      } else {
        result_field->store_decimal(arg_val);
        result_field->set_notnull();
      }
    }
  } else {
    uchar *res = result_field->ptr;

    double old_nr = float8get(res);
    double nr = args[0]->val_real();
    if (!args[0]->null_value) {
      old_nr += nr;
      result_field->set_notnull();
    }
    float8store(res, old_nr);
  }
}

void Item_sum_count::update_field() {
  longlong nr;
  uchar *res = result_field->ptr;

  nr = sint8korr(res);
  if (!args[0]->maybe_null || !args[0]->is_null()) nr++;
  int8store(res, nr);
}

void Item_sum_avg::update_field() {
  DBUG_TRACE;
  longlong field_count;
  uchar *res = result_field->ptr;

  DBUG_ASSERT(aggr->Aggrtype() != Aggregator::DISTINCT_AGGREGATOR);

  if (hybrid_type == DECIMAL_RESULT) {
    my_decimal value, *arg_val = args[0]->val_decimal(&value);
    if (!args[0]->null_value) {
      binary2my_decimal(E_DEC_FATAL_ERROR, res, dec_buffs + 1, f_precision,
                        f_scale);
      field_count = sint8korr(res + dec_bin_size);
      my_decimal_add(E_DEC_FATAL_ERROR, dec_buffs, arg_val, dec_buffs + 1);
      my_decimal2binary(E_DEC_FATAL_ERROR, dec_buffs, res, f_precision,
                        f_scale);
      res += dec_bin_size;
      field_count++;
      int8store(res, field_count);
    }
  } else {
    double nr;

    nr = args[0]->val_real();
    if (!args[0]->null_value) {
      double old_nr = float8get(res);
      field_count = sint8korr(res + sizeof(double));
      old_nr += nr;
      float8store(res, old_nr);
      res += sizeof(double);
      field_count++;
      int8store(res, field_count);
    }
  }
}

void Item_sum_hybrid::update_field() {
  switch (hybrid_type) {
    case STRING_RESULT:
      if (args[0]->is_temporal())
        min_max_update_temporal_field();
      else if (data_type() == MYSQL_TYPE_JSON)
        min_max_update_json_field();
      else
        min_max_update_str_field();
      break;
    case INT_RESULT:
      min_max_update_int_field();
      break;
    case DECIMAL_RESULT:
      min_max_update_decimal_field();
      break;
    default:
      min_max_update_real_field();
  }
}

void Item_sum_hybrid::min_max_update_temporal_field() {
  const longlong nr = args[0]->val_temporal_by_field_type();
  if (args[0]->null_value) return;

  if (result_field->is_null()) {
    result_field->set_notnull();
  } else {
    const longlong old_nr = result_field->val_temporal_by_field_type();
    if (!min_max_best_so_far(
            unsigned_flag ? compare_numbers(ulonglong(nr), ulonglong(old_nr))
                          : compare_numbers(nr, old_nr),
            m_is_min))
      return;
  }

  result_field->store_packed(nr);
}

void Item_sum_hybrid::min_max_update_json_field() {
  Json_wrapper json1;
  if (args[0]->val_json(&json1)) return;
  if (args[0]->null_value) return;

  Field_json *const json_field = down_cast<Field_json *>(result_field);
  if (json_field->is_null()) {
    json_field->set_notnull();
  } else {
    Json_wrapper json2;
    if (json_field->val_json(&json2) ||
        !min_max_best_so_far(json1.compare(json2), m_is_min))
      return;
  }

  json_field->store_json(&json1);
}

void Item_sum_hybrid::min_max_update_str_field() {
  DBUG_ASSERT(cmp);
  const String *const res_str = args[0]->val_str(&cmp->value1);
  if (args[0]->null_value) return;

  if (result_field->is_null())
    result_field->set_notnull();
  else if (!min_max_best_so_far(
               sortcmp(res_str, result_field->val_str(&cmp->value2),
                       collation.collation),
               m_is_min))
    return;

  result_field->store(res_str->ptr(), res_str->length(), res_str->charset());
}

void Item_sum_hybrid::min_max_update_real_field() {
  const double nr = args[0]->val_real();
  if (args[0]->null_value) return;

  if (result_field->is_null())
    result_field->set_notnull();
  else if (!min_max_best_so_far(compare_numbers(nr, result_field->val_real()),
                                m_is_min))
    return;

  result_field->store(nr);
}

void Item_sum_hybrid::min_max_update_int_field() {
  const longlong nr = args[0]->val_int();
  if (args[0]->null_value) return;

  if (result_field->is_null()) {
    result_field->set_notnull();
  } else {
    const longlong old_nr = result_field->val_int();
    if (!min_max_best_so_far(
            unsigned_flag ? compare_numbers(ulonglong(nr), ulonglong(old_nr))
                          : compare_numbers(nr, old_nr),
            m_is_min))
      return;
  }

  result_field->store(nr, unsigned_flag);
}

void Item_sum_hybrid::min_max_update_decimal_field() {
  my_decimal nr_val;
  const my_decimal *const nr = args[0]->val_decimal(&nr_val);
  if (args[0]->null_value) return;

  if (result_field->is_null()) {
    result_field->set_notnull();
  } else {
    my_decimal old_val;
    const my_decimal *const old_nr = result_field->val_decimal(&old_val);
    if (!min_max_best_so_far(my_decimal_cmp(nr, old_nr), m_is_min)) return;
  }

  result_field->store_decimal(nr);
}

Item_avg_field::Item_avg_field(Item_result res_type, Item_sum_avg *item) {
  DBUG_ASSERT(!item->m_is_window_function);
  item_name = item->item_name;
  decimals = item->decimals;
  max_length = item->max_length;
  unsigned_flag = item->unsigned_flag;
  field = item->get_result_field();
  maybe_null = true;
  hybrid_type = res_type;
  set_data_type(hybrid_type == DECIMAL_RESULT ? MYSQL_TYPE_NEWDECIMAL
                                              : MYSQL_TYPE_DOUBLE);
  prec_increment = item->prec_increment;
  if (hybrid_type == DECIMAL_RESULT) {
    f_scale = item->f_scale;
    f_precision = item->f_precision;
    dec_bin_size = item->dec_bin_size;
  }
}

double Item_avg_field::val_real() {
  // fix_fields() never calls for this Item
  longlong count;
  uchar *res;

  if (hybrid_type == DECIMAL_RESULT) return val_real_from_decimal();

  double nr = float8get(field->ptr);
  res = (field->ptr + sizeof(double));
  count = sint8korr(res);

  if ((null_value = !count)) return 0.0;
  return nr / (double)count;
}

my_decimal *Item_avg_field::val_decimal(my_decimal *dec_buf) {
  // fix_fields() never calls for this Item
  if (hybrid_type == REAL_RESULT) return val_decimal_from_real(dec_buf);
  longlong count = sint8korr(field->ptr + dec_bin_size);
  if ((null_value = !count)) return nullptr;

  my_decimal dec_count, dec_field;
  binary2my_decimal(E_DEC_FATAL_ERROR, field->ptr, &dec_field, f_precision,
                    f_scale);
  int2my_decimal(E_DEC_FATAL_ERROR, count, false, &dec_count);
  my_decimal_div(E_DEC_FATAL_ERROR, dec_buf, &dec_field, &dec_count,
                 prec_increment);
  return dec_buf;
}

String *Item_avg_field::val_str(String *str) {
  // fix_fields() never calls for this Item
  if (hybrid_type == DECIMAL_RESULT) return val_string_from_decimal(str);
  return val_string_from_real(str);
}

Item_sum_bit_field::Item_sum_bit_field(Item_result res_type, Item_sum_bit *item,
                                       ulonglong neutral_element) {
  DBUG_ASSERT(!item->m_is_window_function);
  reset_bits = neutral_element;
  item_name = item->item_name;
  decimals = item->decimals;
  max_length = item->max_length;
  unsigned_flag = item->unsigned_flag;
  field = item->get_result_field();
  maybe_null = false;
  hybrid_type = res_type;
  DBUG_ASSERT(hybrid_type == INT_RESULT || hybrid_type == STRING_RESULT);
  if (hybrid_type == INT_RESULT)
    set_data_type(MYSQL_TYPE_LONGLONG);
  else if (hybrid_type == STRING_RESULT)
    set_data_type(MYSQL_TYPE_VARCHAR);
  // Implementation requires a non-Blob for string results.
  DBUG_ASSERT(hybrid_type != STRING_RESULT ||
              field->type() == MYSQL_TYPE_VARCHAR);
}

longlong Item_sum_bit_field::val_int() {
  if (hybrid_type == INT_RESULT)
    return uint8korr(field->ptr);
  else {
    String *res;
    if (!(res = val_str(&str_value))) return 0;

    int ovf_error;
    const char *from = res->ptr();
    size_t len = res->length();
    const char *end = from + len;
    return my_strtoll10(from, &end, &ovf_error);
  }
}

double Item_sum_bit_field::val_real() {
  if (hybrid_type == INT_RESULT) {
    ulonglong result = uint8korr(field->ptr);
    return result;
  } else {
    String *res;
    if (!(res = val_str(&str_value))) return 0.0;

    int ovf_error;
    const char *from = res->ptr();
    size_t len = res->length();
    const char *end = from + len;

    return my_strtod(from, &end, &ovf_error);
  }
}

my_decimal *Item_sum_bit_field::val_decimal(my_decimal *dec_buf) {
  if (hybrid_type == INT_RESULT)
    return val_decimal_from_int(dec_buf);
  else
    return val_decimal_from_string(dec_buf);
}

/// @see Item_sum_bit::val_str()
String *Item_sum_bit_field::val_str(String *str) {
  if (hybrid_type == INT_RESULT)
    return val_string_from_int(str);
  else {
    String *res_str = field->val_str(str);
    const bool non_nulls = res_str->ptr()[res_str->length() - 1];
    if (!non_nulls) {
      DBUG_EXECUTE_IF("simulate_sum_out_of_memory", { return nullptr; });
      if (res_str->alloc(max_length - 1)) return nullptr;
      std::memset(res_str->ptr(), static_cast<int>(reset_bits), max_length - 1);
      res_str->length(max_length - 1);
      res_str->set_charset(&my_charset_bin);
    } else
      res_str->length(res_str->length() - 1);
    return res_str;
  }
}

bool Item_sum_bit_field::get_date(MYSQL_TIME *ltime,
                                  my_time_flags_t fuzzydate) {
  if (hybrid_type == INT_RESULT)
    return get_date_from_decimal(ltime, fuzzydate);
  else
    return get_date_from_string(ltime, fuzzydate);
}
bool Item_sum_bit_field::get_time(MYSQL_TIME *ltime) {
  if (hybrid_type == INT_RESULT)
    return get_time_from_numeric(ltime);
  else
    return get_time_from_string(ltime);
}

Item_std_field::Item_std_field(Item_sum_std *item)
    : Item_variance_field(item) {}

double Item_std_field::val_real() {
  double nr;
  // fix_fields() never calls for this Item
  nr = Item_variance_field::val_real();
  DBUG_ASSERT(nr >= 0.0);
  return sqrt(nr);
}

my_decimal *Item_std_field::val_decimal(my_decimal *dec_buf) {
  /*
    We can't call val_decimal_from_real() for DECIMAL_RESULT as
    Item_variance_field::val_real() would cause an infinite loop
  */
  my_decimal tmp_dec, *dec;
  double nr;
  if (hybrid_type == REAL_RESULT) return val_decimal_from_real(dec_buf);

  dec = Item_variance_field::val_decimal(dec_buf);
  if (!dec) return nullptr;
  my_decimal2double(E_DEC_FATAL_ERROR, dec, &nr);
  DBUG_ASSERT(nr >= 0.0);
  nr = sqrt(nr);
  double2my_decimal(E_DEC_FATAL_ERROR, nr, &tmp_dec);
  my_decimal_round(E_DEC_FATAL_ERROR, &tmp_dec, decimals, false, dec_buf);
  return dec_buf;
}

Item_variance_field::Item_variance_field(Item_sum_variance *item) {
  DBUG_ASSERT(!item->m_is_window_function);
  item_name = item->item_name;
  decimals = item->decimals;
  max_length = item->max_length;
  unsigned_flag = item->unsigned_flag;
  field = item->get_result_field();
  maybe_null = true;
  sample = item->sample;
  hybrid_type = item->hybrid_type;
  DBUG_ASSERT(hybrid_type == REAL_RESULT);
  set_data_type(MYSQL_TYPE_DOUBLE);
}

double Item_variance_field::val_real() {
  // fix_fields() never calls for this Item
  if (hybrid_type == DECIMAL_RESULT) return val_real_from_decimal();

  double recurrence_s = float8get(field->ptr + sizeof(double));
  ulonglong count = sint8korr(field->ptr + sizeof(double) * 2);

  if ((null_value = (count <= sample))) return 0.0;
  return variance_fp_recurrence_result(recurrence_s, 0.0, count, sample, false);
}

/****************************************************************************
** Functions to handle dynamic loadable aggregates
****************************************************************************/

bool Item_udf_sum::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  pc->thd->lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_UDF);
  pc->thd->lex->safe_to_cache_query = false;
  return false;
}

void Item_udf_sum::clear() {
  DBUG_TRACE;
  udf.clear();
}

bool Item_udf_sum::add() {
  DBUG_TRACE;
  udf.add(&null_value);
  return false;
}

void Item_udf_sum::cleanup() {
  /*
    udf_handler::cleanup() nicely handles case when we have not
    original item but one created by copy_or_same() method.
  */
  udf.cleanup();
  Item_sum::cleanup();
}

void Item_udf_sum::print(const THD *thd, String *str,
                         enum_query_type query_type) const {
  str->append(func_name());
  str->append('(');
  for (uint i = 0; i < arg_count; i++) {
    if (i) str->append(',');
    args[i]->print(thd, str, query_type);
  }
  str->append(')');
}

Item *Item_sum_udf_float::copy_or_same(THD *thd) {
  return new (thd->mem_root) Item_sum_udf_float(thd, this);
}

double Item_sum_udf_float::val_real() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  DBUG_PRINT("info", ("result_type: %d  arg_count: %d", args[0]->result_type(),
                      arg_count));
  return udf.val_real(&null_value);
}

String *Item_sum_udf_float::val_str(String *str) {
  return val_string_from_real(str);
}

my_decimal *Item_sum_udf_float::val_decimal(my_decimal *dec) {
  return val_decimal_from_real(dec);
}

String *Item_sum_udf_decimal::val_str(String *str) {
  return val_string_from_decimal(str);
}

double Item_sum_udf_decimal::val_real() { return val_real_from_decimal(); }

longlong Item_sum_udf_decimal::val_int() { return val_int_from_decimal(); }

my_decimal *Item_sum_udf_decimal::val_decimal(my_decimal *dec_buf) {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  DBUG_PRINT("info", ("result_type: %d  arg_count: %d", args[0]->result_type(),
                      arg_count));

  return udf.val_decimal(&null_value, dec_buf);
}

Item *Item_sum_udf_decimal::copy_or_same(THD *thd) {
  return new (thd->mem_root) Item_sum_udf_decimal(thd, this);
}

Item *Item_sum_udf_int::copy_or_same(THD *thd) {
  return new (thd->mem_root) Item_sum_udf_int(thd, this);
}

longlong Item_sum_udf_int::val_int() {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  DBUG_PRINT("info", ("result_type: %d  arg_count: %d", args[0]->result_type(),
                      arg_count));
  return udf.val_int(&null_value);
}

String *Item_sum_udf_int::val_str(String *str) {
  return val_string_from_int(str);
}

my_decimal *Item_sum_udf_int::val_decimal(my_decimal *dec) {
  return val_decimal_from_int(dec);
}

/** Default max_length is max argument length. */

bool Item_sum_udf_str::resolve_type(THD *) {
  set_data_type(MYSQL_TYPE_VARCHAR);
  max_length = 0;
  for (uint i = 0; i < arg_count; i++)
    max_length = max(max_length, args[i]->max_length);
  return false;
}

Item *Item_sum_udf_str::copy_or_same(THD *thd) {
  return new (thd->mem_root) Item_sum_udf_str(thd, this);
}

my_decimal *Item_sum_udf_str::val_decimal(my_decimal *dec) {
  return val_decimal_from_string(dec);
}

String *Item_sum_udf_str::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  DBUG_TRACE;
  String *res = udf.val_str(str, &str_value);
  null_value = !res;
  return res;
}

/*****************************************************************************
 GROUP_CONCAT function

 SQL SYNTAX:
  GROUP_CONCAT([DISTINCT] expr,... [ORDER BY col [ASC|DESC],...]
    [SEPARATOR str_const])

 concat of values from "group by" operation

 BUGS
   Blobs doesn't work with DISTINCT or ORDER BY
*****************************************************************************/

/**
  Compares the values for fields in expr list of GROUP_CONCAT.

  @code
     GROUP_CONCAT([DISTINCT] expr [,expr ...]
              [ORDER BY {unsigned_integer | col_name | expr}
                  [ASC | DESC] [,col_name ...]]
              [SEPARATOR str_val])
  @endcode

  @retval -1 : key1 < key2
  @retval  0 : key1 = key2
  @retval  1 : key1 > key2
*/

int group_concat_key_cmp_with_distinct(const void *arg, const void *key1,
                                       const void *key2) {
  DBUG_TRACE;
  const Item_func_group_concat *item_func =
      static_cast<const Item_func_group_concat *>(arg);
  TABLE *table = item_func->table;

  for (uint i = 0; i < item_func->arg_count_field; i++) {
    Item *item = item_func->args[i];
    /*
      If item is a const item then either get_tmp_table_field returns 0
      or it is an item over a const table.
    */
    if (item->const_item()) continue;
    /*
      We have to use get_tmp_table_field() instead of
      real_item()->get_tmp_table_field() because we want the field in
      the temporary table, not the original field
    */
    Field *field = item->get_tmp_table_field();

    if (!field) continue;

    uint offset = field->offset(field->table->record[0]) - table->s->null_bytes;
    int res = field->cmp(pointer_cast<const uchar *>(key1) + offset,
                         pointer_cast<const uchar *>(key2) + offset);
    if (res) return res;
  }
  return 0;
}

/**
  function of sort for syntax: GROUP_CONCAT(expr,... ORDER BY col,... )
*/

int group_concat_key_cmp_with_order(const void *arg, const void *key1,
                                    const void *key2) {
  DBUG_TRACE;
  const Item_func_group_concat *grp_item =
      static_cast<const Item_func_group_concat *>(arg);
  const ORDER *order_item, *end;
  TABLE *table = grp_item->table;

  for (order_item = grp_item->order_array.begin(),
      end = grp_item->order_array.end();
       order_item < end; order_item++) {
    Item *item = *(order_item)->item;
    /*
      If item is a const item then either get_tmp_table_field returns 0
      or it is an item over a const table.
    */
    if (item->const_item()) continue;
    /*
      We have to use get_tmp_table_field() instead of
      real_item()->get_tmp_table_field() because we want the field in
      the temporary table, not the original field
     */
    Field *field = item->get_tmp_table_field();
    if (!field) continue;

    uint offset =
        (field->offset(field->table->record[0]) - table->s->null_bytes);
    int res = field->cmp(pointer_cast<const uchar *>(key1) + offset,
                         pointer_cast<const uchar *>(key2) + offset);
    if (res) return ((order_item)->direction == ORDER_ASC) ? res : -res;
  }
  /*
    We can't return 0 because in that case the tree class would remove this
    item as double value. This would cause problems for case-changes and
    if the returned values are not the same we do the sort on.
  */
  return 1;
}

/**
  Append data from current leaf to item->result.
*/

int dump_leaf_key(void *key_arg, element_count count MY_ATTRIBUTE((unused)),
                  void *item_arg) {
  DBUG_TRACE;
  Item_func_group_concat *item = (Item_func_group_concat *)item_arg;
  TABLE *table = item->table;
  String tmp((char *)table->record[1], table->s->reclength,
             default_charset_info);
  String tmp2;
  uchar *key = (uchar *)key_arg;
  String *result = &item->result;
  Item **arg = item->args, **arg_end = item->args + item->arg_count_field;
  size_t old_length = result->length();

  if (!item->m_result_finalized)
    item->m_result_finalized = true;
  else
    result->append(*item->separator);

  tmp.length(0);

  for (; arg < arg_end; arg++) {
    String *res;
    /*
      We have to use get_tmp_table_field() instead of
      real_item()->get_tmp_table_field() because we want the field in
      the temporary table, not the original field
      We also can't use table->field array to access the fields
      because it contains both order and arg list fields.
     */
    if ((*arg)->const_item())
      res = (*arg)->val_str(&tmp);
    else {
      Field *field = (*arg)->get_tmp_table_field();
      if (field) {
        uint offset =
            (field->offset(field->table->record[0]) - table->s->null_bytes);
        DBUG_ASSERT(offset < table->s->reclength);
        res = field->val_str(&tmp, key + offset);
      } else
        res = (*arg)->val_str(&tmp);
    }
    if (res) result->append(*res);
  }

  item->row_count++;

  /*
     Stop if the size of group_concat value, in bytes, is longer than
     the maximum size.
  */
  if (result->length() > item->group_concat_max_len) {
    int well_formed_error;
    const CHARSET_INFO *cs = item->collation.collation;
    const char *ptr = result->ptr();
    size_t add_length;
    /*
      It's ok to use item->result.length() as the fourth argument
      as this is never used to limit the length of the data.
      Cut is done with the third argument.
    */
    add_length = cs->cset->well_formed_len(
        cs, ptr + old_length, ptr + item->group_concat_max_len,
        result->length(), &well_formed_error);
    result->length(old_length + add_length);
    item->warning_for_row = true;
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_CUT_VALUE_GROUP_CONCAT,
        ER_THD(current_thd, ER_CUT_VALUE_GROUP_CONCAT), item->row_count);

    /**
       To avoid duplicated warnings in Item_func_group_concat::val_str()
    */
    if (table && table->blob_storage)
      table->blob_storage->set_truncated_value(false);
    return 1;
  }
  return 0;
}

/**
  Constructor of Item_func_group_concat.

  @param pos The token's position.
  @param distinct_arg   distinct
  @param select_list    list of expression for show values
  @param opt_order_list list of sort columns
  @param separator_arg  string value of separator.
  @param w              window, iff we have a windowing use of GROUP_CONCAT
*/

Item_func_group_concat::Item_func_group_concat(
    const POS &pos, bool distinct_arg, PT_item_list *select_list,
    PT_order_list *opt_order_list, String *separator_arg, PT_window *w)
    : super(pos, w),
      tmp_table_param(nullptr),
      separator(separator_arg),
      tree(nullptr),
      unique_filter(nullptr),
      table(nullptr),
      order_array(*THR_MALLOC),
      arg_count_order(opt_order_list ? opt_order_list->value.elements : 0),
      arg_count_field(select_list->elements()),
      row_count(0),
      group_concat_max_len(0),
      distinct(distinct_arg),
      warning_for_row(false),
      always_null(false),
      force_copy_fields(false),
      original(nullptr) {
  Item *item_select;
  Item **arg_ptr;

  allow_group_via_temp_table = false;
  arg_count = arg_count_field + arg_count_order;

  if (!(args = (Item **)(*THR_MALLOC)->Alloc(sizeof(Item *) * arg_count)))
    return;

  if (order_array.reserve(arg_count_order)) return;

  /* fill args items of show and sort */
  List_iterator_fast<Item> li(select_list->value);

  for (arg_ptr = args; (item_select = li++); arg_ptr++) *arg_ptr = item_select;

  if (arg_count_order) {
    for (ORDER *order_item = opt_order_list->value.first; order_item != nullptr;
         order_item = order_item->next) {
      order_array.push_back(*order_item);
      *arg_ptr = *order_item->item;
      order_array.back().item = arg_ptr++;
    }
    for (ORDER *ord = order_array.begin(); ord < order_array.end(); ++ord)
      ord->next = ord != &order_array.back() ? ord + 1 : nullptr;
  }
}

bool Item_func_group_concat::itemize(Parse_context *pc, Item **res) {
  if (skip_itemize(res)) return false;
  if (super::itemize(pc, res)) return true;
  context = pc->thd->lex->current_context();
  return false;
}

Item_func_group_concat::Item_func_group_concat(THD *thd,
                                               Item_func_group_concat *item)
    : Item_sum(thd, item),
      tmp_table_param(item->tmp_table_param),
      separator(item->separator),
      tree(item->tree),
      unique_filter(item->unique_filter),
      table(item->table),
      order_array(thd->mem_root),
      context(item->context),
      arg_count_order(item->arg_count_order),
      arg_count_field(item->arg_count_field),
      row_count(item->row_count),
      group_concat_max_len(item->group_concat_max_len),
      distinct(item->distinct),
      warning_for_row(item->warning_for_row),
      always_null(item->always_null),
      force_copy_fields(item->force_copy_fields),
      original(item) {
  allow_group_via_temp_table = item->allow_group_via_temp_table;
  result.set_charset(collation.collation);

  /*
    Since the ORDER structures pointed to by the elements of the 'order' array
    may be modified in find_order_in_list() called from
    Item_func_group_concat::setup(), create a copy of those structures so that
    such modifications done in this object would not have any effect on the
    object being copied.
  */
  if (order_array.reserve(arg_count_order)) return;

  for (uint i = 0; i < arg_count_order; i++) {
    /*
      Compiler generated copy constructor is used to
      to copy all the members of ORDER struct.
      It's also necessary to update ORDER::next pointer
      so that it points to new ORDER element.
    */
    order_array.push_back(item->order_array[i]);
  }
  if (arg_count_order) {
    for (ORDER *ord = order_array.begin(); ord < order_array.end(); ++ord)
      ord->next = ord != &order_array.back() ? ord + 1 : nullptr;
  }
}

void Item_func_group_concat::cleanup() {
  DBUG_TRACE;
  Item_sum::cleanup();

  /*
    Free table and tree if they belong to this item (if item have not pointer
    to original item from which was made copy => it own its objects )
  */
  if (!original) {
    destroy(tmp_table_param);
    tmp_table_param = nullptr;
    if (table) {
      THD *thd = table->in_use;
      if (table->blob_storage) destroy(table->blob_storage);
      free_tmp_table(thd, table);
      table = nullptr;
      if (tree) {
        delete_tree(tree);
        tree = nullptr;
      }
      if (unique_filter) {
        destroy(unique_filter);
        unique_filter = nullptr;
      }
    }
    DBUG_ASSERT(tree == nullptr);
  }
  /*
   As the ORDER structures pointed to by the elements of the
   'order' array may be modified in find_order_in_list() called
   from Item_func_group_concat::setup() to point to runtime
   created objects, we need to reset them back to the original
   arguments of the function.
   */
  for (uint i = 0; i < arg_count_order; i++) {
    if (order_array[i].is_position)
      args[arg_count_field + i] = order_array[i].item_ptr;
  }
}

Field *Item_func_group_concat::make_string_field(TABLE *table_arg) const {
  Field *field;
  DBUG_ASSERT(collation.collation);
  /*
    Use mbminlen to determine maximum number of characters.
    Compared to using mbmaxlen, this provides ability to
    accommodate more characters in case of charsets that
    support variable length characters.
    If the actual data has characters with length less than
    mbmaxlen, with this approach more characters can be stored.
  */

  const uint32 max_characters =
      group_concat_max_len / collation.collation->mbminlen;
  if (max_characters > CONVERT_IF_BIGGER_TO_BLOB)
    field = new (*THR_MALLOC)
        Field_blob(max_characters * collation.collation->mbmaxlen, maybe_null,
                   item_name.ptr(), collation.collation, true);
  else
    field = new (*THR_MALLOC) Field_varstring(
        max_characters * collation.collation->mbmaxlen, maybe_null,
        item_name.ptr(), table_arg->s, collation.collation);

  if (field) field->init(table_arg);
  return field;
}

Item *Item_func_group_concat::copy_or_same(THD *thd) {
  DBUG_TRACE;
  Item *result = m_is_window_function ? this
                                      : new (thd->mem_root)
                                            Item_func_group_concat(thd, this);
  return result;
}

void Item_func_group_concat::clear() {
  result.length(0);
  result.copy();
  null_value = true;
  warning_for_row = false;
  m_result_finalized = false;
  if (tree) reset_tree(tree);
  if (unique_filter) unique_filter->reset();
  if (table && table->blob_storage) table->blob_storage->reset();
  /* No need to reset the table as we never call write_row */
}

bool Item_func_group_concat::add() {
  if (always_null) return false;
  if (copy_fields(tmp_table_param, table->in_use)) return true;
  if (copy_funcs(tmp_table_param, table->in_use)) return true;

  for (uint i = 0; i < arg_count_field; i++) {
    Item *show_item = args[i];
    if (show_item->const_item()) continue;

    Field *field = show_item->get_tmp_table_field();
    if (field && field->is_null_in_record((const uchar *)table->record[0]))
      return false;  // Skip row if it contains null
  }

  null_value = false;
  bool row_eligible = true;

  if (distinct) {
    /* Filter out duplicate rows. */
    uint count = unique_filter->elements_in_tree();
    unique_filter->unique_add(table->record[0] + table->s->null_bytes);
    if (count == unique_filter->elements_in_tree()) row_eligible = false;
  }

  TREE_ELEMENT *el = nullptr;  // Only for safety
  if (row_eligible && tree) {
    DBUG_EXECUTE_IF("trigger_OOM_in_gconcat_add",
                    DBUG_SET("+d,simulate_persistent_out_of_memory"););
    el = tree_insert(tree, table->record[0] + table->s->null_bytes, 0,
                     tree->custom_arg);
    DBUG_EXECUTE_IF("trigger_OOM_in_gconcat_add",
                    DBUG_SET("-d,simulate_persistent_out_of_memory"););
    /* check if there was enough memory to insert the row */
    if (!el) return true;
  }
  /*
    In case of GROUP_CONCAT with DISTINCT or ORDER BY (or both) don't dump the
    row to the output buffer here. That will be done in val_str.
  */
  if (row_eligible && !warning_for_row && tree == nullptr && !distinct)
    dump_leaf_key(table->record[0] + table->s->null_bytes, 1, this);

  return false;
}

bool Item_func_group_concat::fix_fields(THD *thd, Item **ref) {
  if (super::fix_fields(thd, ref)) return true;

  if (init_sum_func_check(thd)) return true;

  maybe_null = true;

  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  /*
    Fix fields for select list and ORDER clause
  */

  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
  }

  /* skip charset aggregation for order columns */
  if (agg_item_charsets_for_string_result(collation, func_name(), args,
                                          arg_count - arg_count_order))
    return true;

  result.set_charset(collation.collation);
  result_field = nullptr;
  null_value = true;
  group_concat_max_len = thd->variables.group_concat_max_len;
  uint32 max_chars = group_concat_max_len / collation.collation->mbminlen;
  uint max_byte_length = max_chars * collation.collation->mbmaxlen;
  max_chars > CONVERT_IF_BIGGER_TO_BLOB ? set_data_type_blob(max_byte_length)
                                        : set_data_type_string(max_chars);

  size_t offset;
  if (separator->needs_conversion(separator->length(), separator->charset(),
                                  collation.collation, &offset)) {
    size_t buflen = collation.collation->mbmaxlen * separator->length();
    uint errors;
    size_t conv_length;
    char *buf;
    String *new_separator;

    if (!(buf = (char *)thd->stmt_arena->alloc(buflen)) ||
        !(new_separator = new (thd->stmt_arena->mem_root)
              String(buf, buflen, collation.collation)))
      return true;

    conv_length =
        copy_and_convert(buf, buflen, collation.collation, separator->ptr(),
                         separator->length(), separator->charset(), &errors);
    new_separator->length(conv_length);
    separator = new_separator;
  }

  if (check_sum_func(thd, ref)) return true;

  fixed = true;
  return false;
}

bool Item_func_group_concat::setup(THD *thd) {
  DBUG_TRACE;

  List<Item> list;
  DBUG_ASSERT(thd->lex->current_select() == aggr_select);

  const bool order_or_distinct = (arg_count_order > 0 || distinct);

  /*
    Currently setup() can be called twice. Please add
    assertion here when this is fixed.
  */
  if (table || tree) return false;

  if (!(tmp_table_param = new (thd->mem_root) Temp_table_param)) return true;

  /* Push all not constant fields to the list and create a temp table */
  always_null = false;
  for (uint i = 0; i < arg_count_field; i++) {
    Item *item = args[i];
    if (list.push_back(item)) return true;
    if (item->const_item()) {
      if (item->is_null()) {
        always_null = true;
        return false;
      }
    }
  }

  List<Item> all_fields(list);
  /*
    Try to find every ORDER expression in the list of GROUP_CONCAT
    arguments. If an expression is not found, prepend it to
    "all_fields". The resulting field list is used as input to create
    tmp table columns.
  */
  if (arg_count_order &&
      setup_order(thd, Ref_item_array(args, arg_count), context->table_list,
                  list, all_fields, order_array.begin()))
    return true;

  count_field_types(aggr_select, tmp_table_param, all_fields, false, true);
  tmp_table_param->force_copy_fields = force_copy_fields;
  DBUG_ASSERT(table == nullptr);
  if (order_or_distinct) {
    /*
      Force the create_tmp_table() to convert BIT columns to INT
      as we cannot compare two table records containg BIT fields
      stored in the the tree used for distinct/order by.
      Moreover we don't even save in the tree record null bits
      where BIT fields store parts of their data.
    */
    List_iterator_fast<Item> li(all_fields);
    Item *item;
    while ((item = li++)) {
      if (item->type() == Item::FIELD_ITEM &&
          ((Item_field *)item)->field->type() == FIELD_TYPE_BIT)
        item->marker = Item::MARKER_BIT;
    }
  }

  /*
    We have to create a temporary table to get descriptions of fields
    (types, sizes and so on).

    Note that in the table, we first have the ORDER BY fields, then the
    field list.
  */
  if (!(table = create_tmp_table(thd, tmp_table_param, all_fields, nullptr,
                                 false, true, aggr_select->active_options(),
                                 HA_POS_ERROR, "")))
    return true;
  table->file->ha_extra(HA_EXTRA_NO_ROWS);
  table->no_rows = true;

  /**
    Initialize blob_storage if GROUP_CONCAT is used
    with ORDER BY | DISTINCT and BLOB field count > 0.
  */
  if (order_or_distinct && table->s->blob_fields)
    table->blob_storage = new (thd->mem_root) Blob_mem_storage();

  /*
     Need sorting or uniqueness: init tree and choose a function to sort.
     Don't reserve space for NULLs: if any of gconcat arguments is NULL,
     the row is not added to the result.
  */
  uint tree_key_length = table->s->reclength - table->s->null_bytes;

  if (arg_count_order) {
    tree = &tree_base;
    /*
      Create a tree for sorting. The tree is used to sort (according to the
      syntax of this function). If there is no ORDER BY clause, we don't
      create this tree.
    */
    init_tree(tree, 0, tree_key_length, group_concat_key_cmp_with_order, false,
              nullptr, this);
  }

  if (distinct)
    unique_filter = new (thd->mem_root)
        Unique(group_concat_key_cmp_with_distinct, (void *)this,
               tree_key_length, ram_limitation(thd));

  return false;
}

/* This is used by rollup to create a separate usable copy of the function */

void Item_func_group_concat::make_unique() {
  tmp_table_param = nullptr;
  table = nullptr;
  original = nullptr;
  force_copy_fields = true;
  tree = nullptr;
}

String *Item_func_group_concat::val_str(String *) {
  DBUG_ASSERT(fixed == 1);
  if (null_value) return nullptr;

  if (!m_result_finalized)  // Result yet to be written.
  {
    if (tree != nullptr)  // order by
      tree_walk(tree, &dump_leaf_key, this, left_root_right);
    else if (distinct)  // distinct (and no order by).
      unique_filter->walk(&dump_leaf_key, this);
    else
      DBUG_ASSERT(false);  // Can't happen
  }

  if (table && table->blob_storage &&
      table->blob_storage->is_truncated_value()) {
    warning_for_row = true;
    push_warning_printf(
        current_thd, Sql_condition::SL_WARNING, ER_CUT_VALUE_GROUP_CONCAT,
        ER_THD(current_thd, ER_CUT_VALUE_GROUP_CONCAT), row_count);
  }

  return &result;
}

void Item_func_group_concat::print(const THD *thd, String *str,
                                   enum_query_type query_type) const {
  str->append(STRING_WITH_LEN("group_concat("));
  if (distinct) str->append(STRING_WITH_LEN("distinct "));
  for (uint i = 0; i < arg_count_field; i++) {
    if (i) str->append(',');
    args[i]->print(thd, str, query_type);
  }
  if (arg_count_order) {
    str->append(STRING_WITH_LEN(" order by "));
    for (uint i = 0; i < arg_count_order; i++) {
      if (i) str->append(',');
      args[i + arg_count_field]->print(thd, str, query_type);
      if (order_array[i].direction == ORDER_ASC)
        str->append(STRING_WITH_LEN(" ASC"));
      else
        str->append(STRING_WITH_LEN(" DESC"));
    }
  }
  str->append(STRING_WITH_LEN(" separator \'"));

  if (query_type & QT_TO_SYSTEM_CHARSET) {
    // Convert to system charset.
    convert_and_print(separator, str, system_charset_info);
  } else if (query_type & QT_TO_ARGUMENT_CHARSET) {
    /*
      Convert the string literals to str->charset(),
      which is typically equal to charset_set_client.
    */
    convert_and_print(separator, str, str->charset());
  } else {
    separator->print(str);
  }
  str->append(STRING_WITH_LEN("\')"));
}

Item_func_group_concat::~Item_func_group_concat() {
  if (!original && unique_filter) destroy(unique_filter);
}

bool Item_non_framing_wf::fix_fields(THD *thd, Item **items) {
  if (super::fix_fields(thd, items)) return true;

  if (init_sum_func_check(thd)) return true;

  /*
    Although group aggregate functions must use Disable_semijoin_flattening
    here, WFs need not. Indeed, WFs can never be used in a WHERE or JOIN ON
    condition, so semijoin is never attempted on any subquery argument of
    theirs.
  */
  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
  }

  if (resolve_type(thd)) return true;

  if (check_sum_func(thd, items)) return true;

  fixed = true;
  return false;
}

longlong Item_row_number::val_int() {
  DBUG_TRACE;

  if (m_window->at_partition_border() && !m_window->needs_buffering()) {
    clear();
  }

  m_ctr++;

  DBUG_PRINT("enter", ("Item_row_number::val_int  at border: %d ctr: %llu",
                       m_window->at_partition_border(), m_ctr));
  return m_ctr;
}

double Item_row_number::val_real() {
  DBUG_ASSERT(unsigned_flag);
  return (ulonglong)val_int();
}

String *Item_row_number::val_str(String *buff) {
  return val_string_from_int(buff);
}

my_decimal *Item_row_number::val_decimal(my_decimal *buffer) {
  (void)int2my_decimal(E_DEC_FATAL_ERROR, val_int(), false, buffer);
  return buffer;
}

void Item_row_number::clear() { m_ctr = 0; }

bool Item_rank::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                   Window_evaluation_requirements *r
                                       MY_ATTRIBUTE((unused))) {
  const PT_order_list *order = m_window->effective_order_by();
  // SQL2015 6.10 <window function> SR 6.a: require ORDER BY; we don't.
  if (!order) return false;  // all rows in partition are peers
  for (ORDER *o = order->value.first; o != nullptr; o = o->next) {
    /*
      We need to access the value of the ORDER expression when evaluating
      RANK to determine equality or not, so we need a handle.
    */
    Item_ref *ir = new Item_ref(&select->context, o->item, "<no matter>",
                                "<partition order>");
    if (ir == nullptr) return true;

    m_previous.push_back(new_Cached_item(thd, ir));
  }
  return false;
}

longlong Item_rank::val_int() {
  DBUG_TRACE;
  if (m_window->at_partition_border() && !m_window->needs_buffering()) {
    clear();
  }

  bool change = false;
  if (m_window->has_windowing_steps()) {
    List_iterator<Cached_item> li(m_previous);
    Cached_item *item;

    /*
      Check if any of the ORDER BY expressions have changed. If so, we
      need to update the rank, considering any duplicates.
    */
    while ((item = li++)) {
      change |= item->cmp();
    }
  }
  // if no windowing steps, no comparison needed.

  if (change) {
    m_rank_ctr += 1 + (m_dense ? 0 : m_duplicates);
    m_duplicates = 0;
  } else {
    m_duplicates++;
  }

  return m_rank_ctr;
}

double Item_rank::val_real() {
  DBUG_ASSERT(unsigned_flag);
  return (ulonglong)val_int();
}

String *Item_rank::val_str(String *buff) { return val_string_from_int(buff); }

my_decimal *Item_rank::val_decimal(my_decimal *buffer) {
  (void)int2my_decimal(E_DEC_FATAL_ERROR, val_int(), false, buffer);
  return buffer;
}

void Item_rank::clear() {
  /*
    Cf. also ::reset_cmp which can't be called until we have the partition's
    first row ready (after copy_fields).
  */
  m_rank_ctr = 1;
  m_duplicates = -1;

  // Reset comparator
  if (m_window->has_windowing_steps()) {
    List_iterator<Cached_item> li(m_previous);
    Cached_item *item;
    while ((item = li++)) {
      item->cmp();  // set baseline
    }
  }  // if no windowing steps, no comparison needed.
}

void Item_rank::cleanup() {
  super::cleanup();
  List_iterator<Cached_item> li(m_previous);
  Cached_item *ci;
  while ((ci = li++)) {
    ci->~Cached_item();
  }
  m_previous.empty();
}

bool Item_cume_dist::check_wf_semantics(THD *, SELECT_LEX *,
                                        Window_evaluation_requirements *r) {
  // we need to know partition cardinality, so two passes
  r->needs_buffer = true;
  // Before we can compute for the current row we need the count of its peers
  r->needs_peerset = true;
  // SQL2015 6.10 <window function> SR 6.h: don't require ORDER BY.
  return false;
}

double Item_cume_dist::val_real() {
  DBUG_TRACE;

  if (!m_window->has_windowing_steps())
    return 1.0;  // degenerate case, no real windowing

  double cume_dist = (double)m_window->last_rowno_in_peerset() /
                     m_window->last_rowno_in_cache();

  return cume_dist;
}

longlong Item_cume_dist::val_int() {
  DBUG_TRACE;

  longlong result = (longlong)rint(val_real());

  return result;
}

String *Item_cume_dist::val_str(String *buff) {
  return val_string_from_real(buff);
}

my_decimal *Item_cume_dist::val_decimal(my_decimal *buffer) {
  (void)double2my_decimal(E_DEC_FATAL_ERROR, val_real(), buffer);
  return buffer;
}

bool Item_percent_rank::check_wf_semantics(THD *, SELECT_LEX *,
                                           Window_evaluation_requirements *r) {
  // we need to know partition cardinality, so two passes
  r->needs_buffer = true;
  /*
    The family of RANK functions doesn't need the peer set: even though they
    give the same value to peers, that value can be computed for the first row
    of the peer set without knowing how many peers it has. However, this family
    needs detection of when the current row leaves the current peer set (to
    increase the rank counter):
    - RANK and DENSE_RANK do so internally with row comparison;
    - but PERCENT_RANK, as it needs partition cardinality, requires buffering,
    so it can simply pretend it needs_peerset() and then the buffering code will
    detect the peer set's end and provide it in last_rowno_in_peerset().
  */
  r->needs_peerset = true;

  const PT_order_list *order = m_window->effective_order_by();
  // SQL2015 6.10 <window function> SR 6.g+6.a: require ORDER BY; we don't.
  if (!order) return false;  // all rows in partition are peers

  return false;
}

double Item_percent_rank::val_real() {
  DBUG_TRACE;

  if (!m_window->has_windowing_steps())
    return 0.0;  // degenerate case, no real windowing

  if (m_window->rowno_being_visited() == m_window->rowno_in_partition()) {
    if (m_last_peer_visited) {
      m_rank_ctr += m_peers;
      m_peers = 0;
      m_last_peer_visited = false;
    }

    m_peers++;

    if (m_window->rowno_being_visited() == m_window->last_rowno_in_peerset())
      m_last_peer_visited = true;

    if (m_rank_ctr == 1) return 0;
  }

  double percent_rank =
      (double)(m_rank_ctr - 1) / (m_window->last_rowno_in_cache() - 1);
  return percent_rank;
}

longlong Item_percent_rank::val_int() {
  DBUG_TRACE;

  longlong result = (longlong)rint(val_real());

  return result;
}

String *Item_percent_rank::val_str(String *buff) {
  return val_string_from_real(buff);
}

my_decimal *Item_percent_rank::val_decimal(my_decimal *buffer) {
  (void)double2my_decimal(E_DEC_FATAL_ERROR, val_real(), buffer);
  return buffer;
}

void Item_percent_rank::clear() {
  m_rank_ctr = 1;
  m_peers = 0;
  m_last_peer_visited = false;
}

void Item_percent_rank::cleanup() { super::cleanup(); }

bool Item_ntile::fix_fields(THD *thd, Item **items) {
  if (super::fix_fields(thd, items)) return true;

  Item *arg = args[0];
  /*
    Semantic check of the argument. Should be a positive constant
    integer larger than zero, cf. SQL 2011 section 6.10 GR 1,a,ii,1-2)
    NULL is allowed. Dynamic parameter is allowed.
  */
  if (arg->type() == Item::PARAM_ITEM) {
    // we are in a PREPARE phase, so can't check yet
  } else if (!arg->const_item() ||
             (!arg->is_null() &&
              ((arg->result_type() != INT_RESULT || arg->val_int() <= 0)))) {
    my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
    return true;
  }

  maybe_null = true;
  return false;
}

longlong Item_ntile::val_int() {
  if (m_window->rowno_being_visited() == m_window->rowno_in_partition()) {
    if (args[0]->is_null()) {
      null_value = true;
      return 0;
    }

    longlong buckets = args[0]->val_int();

    /*
      Should not be evaluated until we have read all rows in partition
      notwithstanding any frames, so last_rowno_in_cache should be cardinality
      of partition.
    */

    int64 full_rounds = m_window->last_rowno_in_cache() / buckets;
    int64 modulus = m_window->last_rowno_in_cache() % buckets;
    int64 r;

    /*
      Rows might not distribute evenly, if modulus!=0. In that case, add
      extras at the beginning as per SQL 2011 section 6.10 <window function>
      GR 1a, ii, 3): the first 'modulus' buckets contain 'full_rounds + 1'
      rows, the other buckets contain 'full_rounds' rows.
     */
    if (modulus == 0 && full_rounds == 0) {
      r = 1;  // degenerate case; no real windowing
    } else {
      // Using convention "row 0 is first row" for those two variables:
      int64 rowno = m_window->rowno_in_partition() - 1,
            // the first rowno of smaller buckets
          first_of_small = modulus * (full_rounds + 1);
      if (rowno >= first_of_small)  // row goes into small buckets
      {
        r = (rowno - first_of_small) / full_rounds + 1 + modulus;
      } else  // row goes into big buckets
      {
        r = rowno / (full_rounds + 1) + 1;
      }
    }
    m_value = r;
  }

  return m_value;
}

double Item_ntile::val_real() {
  DBUG_ASSERT(unsigned_flag);
  return (ulonglong)val_int();
}

String *Item_ntile::val_str(String *buff) { return val_string_from_int(buff); }

my_decimal *Item_ntile::val_decimal(my_decimal *buffer) {
  (void)int2my_decimal(E_DEC_FATAL_ERROR, val_int(), false, buffer);
  return buffer;
}

bool Item_ntile::check_wf_semantics(THD *thd MY_ATTRIBUTE((unused)),
                                    SELECT_LEX *select MY_ATTRIBUTE((unused)),
                                    Window_evaluation_requirements *r) {
  r->needs_buffer =
      true;  // we need to know partition cardinality, so two passes
  // SQL2015 6.10 <window function> SR 6.a: require ORDER BY; we don't.
  return false;
}

bool Item_first_last_value::check_wf_semantics(
    THD *thd, SELECT_LEX *select, Window_evaluation_requirements *r) {
  if (super::check_wf_semantics(thd, select, r)) return true;

  r->opt_first_row = m_is_first;
  r->opt_last_row = !m_is_first;

  if (m_null_treatment == NT_IGNORE_NULLS) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "IGNORE NULLS");
    return true;
  }
  return false;
}

bool Item_first_last_value::resolve_type(THD *thd MY_ATTRIBUTE((unused))) {
  aggregate_type(make_array(args, 1));
  m_hybrid_type = Field::result_merge_type(data_type());
  maybe_null = true;  // if empty frame, notwithstanding nullability of arg

  if (m_hybrid_type == STRING_RESULT) {
    if (aggregate_string_properties(func_name(), args, 1)) return true;
  } else {
    collation.set_numeric();  // Number
    aggregate_num_type(m_hybrid_type, args, 1);
  }

  return false;
}

bool Item_first_last_value::fix_fields(THD *thd, Item **items) {
  if (super::fix_fields(thd, items)) return true;

  if (init_sum_func_check(thd)) return true;

  if ((!args[0]->fixed && args[0]->fix_fields(thd, args)) ||
      args[0]->check_cols(1))
    return true;

  if (setup_first_last()) return true;

  result_field = nullptr;

  if (resolve_type(thd)) return true;

  if (check_sum_func(thd, items)) return true;

  fixed = true;
  return false;
}

void Item_first_last_value::split_sum_func(THD *thd,
                                           Ref_item_array ref_item_array,
                                           List<Item> &fields) {
  super::split_sum_func(thd, ref_item_array, fields);
  // Need to redo this now:
  m_value->setup(args[0]);
}

bool Item_first_last_value::setup_first_last() {
  m_value = Item_cache::get_cache(args[0]);
  if (m_value == nullptr) return true;
  /*
    After any split_sum_func, we will need to update the m_value::example,
    cf. Item_first_last_value::split_sum_func
  */
  m_value->setup(args[0]);
  return false;
}

void Item_first_last_value::clear() {
  m_value->clear();
  null_value = true;
  cnt = 0;
}

bool Item_first_last_value::compute() {
  cnt++;

  if (m_window->do_inverse()) {
    null_value = true;
  } else if ((m_window->needs_buffering() &&
              (((m_window->rowno_in_frame() == 1 && m_is_first) ||
                (m_window->is_last_row_in_frame() && !m_is_first)) ||
               m_window->rowno_being_visited() ==
                   0 /* No FROM; one const row */)) ||
             (!m_window->needs_buffering() &&
              ((m_is_first && cnt == 1) || !m_is_first))) {
    // if() above says we are positioned at the proper first/last row of frame
    m_value->cache_value();
    null_value = m_value->null_value;
  }
  return null_value || current_thd->is_error();
}

longlong Item_first_last_value::val_int() {
  if (wf_common_init()) return 0;

  if (compute()) return error_int();

  return m_value->val_int();
}

double Item_first_last_value::val_real() {
  if (wf_common_init()) return 0.0;

  if (compute()) return error_real();

  return m_value->val_real();
}

bool Item_first_last_value::get_date(MYSQL_TIME *ltime,
                                     my_time_flags_t fuzzydate) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_value->get_date(ltime, fuzzydate);
}

bool Item_first_last_value::get_time(MYSQL_TIME *ltime) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_value->get_time(ltime);
}

bool Item_first_last_value::val_json(Json_wrapper *jw) {
  if (wf_common_init()) return true;

  if (compute()) return null_value ? false : true;

  return m_value->val_json(jw);
}

my_decimal *Item_first_last_value::val_decimal(my_decimal *decimal_buffer) {
  if (wf_common_init()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  if (compute()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  return m_value->val_decimal(decimal_buffer);
}

String *Item_first_last_value::val_str(String *str) {
  if (wf_common_init()) return str;

  if (compute()) return error_str();

  return m_value->val_str(str);
}

bool Item_nth_value::resolve_type(THD *thd MY_ATTRIBUTE((unused))) {
  aggregate_type(make_array(args, 1));
  m_hybrid_type = Field::result_merge_type(data_type());
  maybe_null = true;

  if (m_hybrid_type == STRING_RESULT) {
    if (aggregate_string_properties(func_name(), args, 1)) return true;
  } else {
    collation.set_numeric();  // Number
    aggregate_num_type(m_hybrid_type, args, 1);
  }

  return false;
}

bool Item_nth_value::fix_fields(THD *thd, Item **items) {
  if (super::fix_fields(thd, items)) return true;

  if (init_sum_func_check(thd)) return true;

  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
  }

  /*
    Semantic check of the row argument. Should be a positive constant
    integer larger than zero, cf. SQL 2011 section 6.10 GR 1,d,ii,1-2)
    NULL is allowed. Dynamic parameter is allowed.
  */
  if (args[1]->type() == Item::PARAM_ITEM) {
    // we are in a PREPARE phase, so can't check yet
  } else {
    if (!args[1]->const_item() ||
        (!args[1]->is_null() &&
         (args[1]->result_type() != INT_RESULT || args[1]->val_int() <= 0))) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
      return true;
    }
    m_n = args[1]->val_int();
  }

  result_field = nullptr;

  if (resolve_type(thd)) return true;

  if (setup_nth()) return true;

  if (check_sum_func(thd, items)) return true;

  fixed = true;
  return false;
}

void Item_nth_value::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                                    List<Item> &fields) {
  super::split_sum_func(thd, ref_item_array, fields);
  // If function was set up, need to redo this now:
  m_value->setup(args[0]);
}

bool Item_nth_value::setup_nth() {
  /*
    After any split_sum_func, we will need to update the m_value::example,
    cf. Item_nth_value::split_sum_func
  */
  m_value = Item_cache::get_cache(args[0]);
  if (m_value == nullptr) return true;
  m_value->setup(args[0]);
  return false;
}

void Item_nth_value::clear() {
  m_value->clear();
  null_value = true;
  m_cnt = 0;
}

bool Item_nth_value::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                        Window_evaluation_requirements *r) {
  if (super::check_wf_semantics(thd, select, r)) return true;

  r->opt_nth_row.m_rowno = m_n;
  r->opt_nth_row.m_from_last = m_from_last;

  if (m_null_treatment == NT_IGNORE_NULLS) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "IGNORE NULLS");
    return true;
  }

  if (m_from_last) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "FROM LAST");
    return true;
  }

  return false;
}

bool Item_nth_value::compute() {
  m_cnt++;

  if (m_window->do_inverse())
    null_value = true;
  else if (!m_window->needs_buffering()) {
    if (m_cnt == m_n) {
      m_value->cache_value();
      null_value = m_value->null_value;
    }
  } else if (m_window->rowno_being_visited() == 0) {
    // empty FROM, single constant row
    if (m_n == 1) {
      m_value->cache_value();
      null_value = m_value->null_value;
    }
  } else if (!m_from_last) {
    if (m_window->rowno_in_frame() == m_n) {
      m_value->cache_value();
      null_value = m_value->null_value;
    }
  } else if (m_from_last) {
    DBUG_ASSERT(false);  // Not yet supported
    //    if (m_window->frame_cardinality() - m_window->rowno_in_frame() + 1
    //        == m_n)
    //    {
    //      m_value->cache_value();
    //      null_value= m_value->null_value;
    //    }
  }
  return null_value || current_thd->is_error();
}

longlong Item_nth_value::val_int() {
  if (wf_common_init()) return 0;

  if (compute()) return error_int();

  return m_value->val_int();
}

double Item_nth_value::val_real() {
  if (wf_common_init()) return 0;

  if (compute()) return error_real();

  return m_value->val_real();
}

my_decimal *Item_nth_value::val_decimal(my_decimal *decimal_buffer) {
  if (wf_common_init()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  if (compute()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  return m_value->val_decimal(decimal_buffer);
}

String *Item_nth_value::val_str(String *str) {
  if (wf_common_init()) return str;

  if (compute()) return error_str();

  return m_value->val_str(str);
}

bool Item_nth_value::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_value->get_date(ltime, fuzzydate);
}

bool Item_nth_value::get_time(MYSQL_TIME *ltime) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_value->get_time(ltime);
}

bool Item_nth_value::val_json(Json_wrapper *jw) {
  if (wf_common_init()) return true;

  if (compute()) return null_value ? false : true;

  return m_value->val_json(jw);
}

bool Item_lead_lag::resolve_type(THD *thd) {
  /*
    If we have default, check type compatibility of default_value to the main
    expression. Modeled on IFNULL, i.e. what's done for
    Item_func_ifnull::resolve_type.
  */

  /*
    As we have to aggregate types of args[0] and args[2], and for that we use
    functions which take arrays, let's temporarily copy args[2] to args[1].
  */
  Item *save_arg1 = nullptr;
  uint orig_arg_count = arg_count;
  if (arg_count == 3) {
    save_arg1 = args[1];
    args[1] = args[2];
    arg_count--;
  } else if (arg_count == 2) {
    arg_count--;
  }

  aggregate_type(make_array(args, arg_count));
  m_hybrid_type = Field::result_merge_type(data_type());

  if (arg_count == 2)
    maybe_null = args[1]->maybe_null || args[0]->maybe_null;
  else
    maybe_null = true;  // No default value provided, so we get NULLs

  if (m_hybrid_type == STRING_RESULT) {
    if (aggregate_string_properties(func_name(), args, arg_count)) return true;
  } else {
    aggregate_num_type(m_hybrid_type, args, arg_count);
  }

  if (orig_arg_count == 3)  // restore args array
  {
    // agg_item_charsets can have changed args[1]:
    args[2] = args[1];
    // and can even have stored its address:
    thd->replace_rollback_place(&args[2]);
    args[1] = save_arg1;
  }
  arg_count = orig_arg_count;
  return false;
}

bool Item_lead_lag::fix_fields(THD *thd, Item **items) {
  if (super::fix_fields(thd, items)) return true;

  /*
    Semantic check of the offset argument. Should be a integral constant
  */
  if (arg_count >= 2) {
    if (args[1]->type() == Item::PARAM_ITEM) {
      // PREPARE time, can't check offset yet
    } else {
      if (!args[1]->const_item() || args[1]->is_null() ||
          (args[1]->result_type() != INT_RESULT)) {
        my_error(ER_WRONG_ARGUMENTS, MYF(0), func_name());
        return true;
      }
      m_n = args[1]->val_int();
    }
  } else {
    m_n = 1;
  }

  /*
    Canonicalize LEAD to negative LAG so we can order all sequentially around
    current row: positive value are LAG, i.e. addresses a row earlier than
    the current row in the result set.
  */
  if (m_is_lead) {
    m_n = -m_n;
  }

  if (setup_lead_lag()) return true;

  fixed = true;
  return false;
}

void Item_lead_lag::split_sum_func(THD *thd, Ref_item_array ref_item_array,
                                   List<Item> &fields) {
  super::split_sum_func(thd, ref_item_array, fields);
  // If function was set up, need to redo these now:
  m_value->setup(args[0]);
  if (m_default != nullptr) m_default->setup(args[2]);
}

bool Item_lead_lag::setup_lead_lag() {
  /*
    After any split_sum_func, we will need to update the m_value::example
    and any m_default::example cf. Item_lead_lag_value::split_sum_func
  */
  m_value = Item_cache::get_cache(args[0]);
  if (m_value == nullptr) return true;
  m_value->setup(args[0]);
  if (arg_count == 3) {
    m_default = Item_cache::get_cache(args[2]);
    if (m_default == nullptr) return true;
    m_default->setup(args[2]);
  }
  return false;
}

bool Item_lead_lag::check_wf_semantics(
    THD *thd MY_ATTRIBUTE((unused)), SELECT_LEX *select MY_ATTRIBUTE((unused)),
    Window_evaluation_requirements *r) {
  if (m_null_treatment == NT_IGNORE_NULLS) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "IGNORE NULLS");
    return true;
  }
  r->needs_buffer = true;
  r->opt_ll_row.m_rowno = m_n;
  // SQL2015 6.10 <window function> SR 6.a: require ORDER BY; we don't.
  return false;
}

void Item_lead_lag::clear() {
  m_value->clear();
  null_value = true;
  m_has_value = false;
  m_use_default = false;
}

longlong Item_lead_lag::val_int() {
  if (wf_common_init()) return 0;

  if (compute()) return error_int();

  return m_use_default ? m_default->val_int() : m_value->val_int();
}

double Item_lead_lag::val_real() {
  if (wf_common_init()) return 0;

  if (compute()) return error_real();

  return m_use_default ? m_default->val_real() : m_value->val_real();
}

my_decimal *Item_lead_lag::val_decimal(my_decimal *decimal_buffer) {
  if (wf_common_init()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  if (compute()) {
    my_decimal_set_zero(decimal_buffer);
    return null_value ? nullptr : decimal_buffer;
  }

  return m_use_default ? m_default->val_decimal(decimal_buffer)
                       : m_value->val_decimal(decimal_buffer);
}

String *Item_lead_lag::val_str(String *str) {
  if (wf_common_init()) return str;

  if (compute()) return error_str();

  return m_use_default ? m_default->val_str(str) : m_value->val_str(str);
}

bool Item_lead_lag::get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_use_default ? m_default->get_date(ltime, fuzzydate)
                       : m_value->get_date(ltime, fuzzydate);
}

bool Item_lead_lag::get_time(MYSQL_TIME *ltime) {
  if (wf_common_init()) return true;

  if (compute()) return true;

  return m_use_default ? m_default->get_time(ltime) : m_value->get_time(ltime);
}

bool Item_lead_lag::val_json(Json_wrapper *jw) {
  if (wf_common_init()) return true;

  if (compute()) return null_value ? false : true;

  return (m_has_value ? (m_use_default ? m_default->val_json(jw)
                                       : m_value->val_json(jw))
                      : false);
}

bool Item_lead_lag::compute() {
  if (m_window->do_inverse()) {
    // nothing, not relevant for LEAD/LAG
  } else {
    if (m_window->rowno_being_visited() == m_window->rowno_in_partition()) {
      /*
        Setup default value if present: it needs to be evaluated on the
        current row, not at the lead/lag row, cf. GR 1.b.i, SQL 2011
      */
      if (arg_count == 3) m_default->cache_value();
      null_value = true;  // a priori for current row
    }

    if (!m_window->has_windowing_steps()) {
      // empty FROM: we have exactly one constant row
      if (m_n == 0) {
        m_value->cache_value();
        null_value = m_value->null_value;
        m_has_value = true;
      } else if (arg_count == 3) {
        null_value = m_default->null_value;
        m_use_default = true;
        m_has_value = true;
      } else {
        null_value = true;
      }

      return null_value || current_thd->is_error();
    }

    bool our_offset = (m_window->rowno_being_visited() ==
                       m_window->rowno_in_partition() - m_n);

    if (our_offset) {
      if ((m_window->rowno_being_visited()) < 1 ||
          (m_window->rowno_being_visited() > m_window->last_rowno_in_cache())) {
        /*
          The row is outside the partition set; use default value if any
          provided else use NULL
        */
        if (arg_count == 3) {
          null_value = m_default->null_value;
          m_use_default = true;
        }
      } else {
        m_value->cache_value();
        null_value = m_value->null_value;
      }
      m_has_value = true;
    } else {
      // Visiting another function; return NULL or result we have.
      if (!m_has_value) null_value = true;
    }
  }
  return null_value || current_thd->is_error();
}

template <typename... Args>
Item_sum_json::Item_sum_json(unique_ptr_destroy_only<Json_wrapper> wrapper,
                             Args &&... parent_args)
    : Item_sum(std::forward<Args>(parent_args)...),
      m_wrapper(std::move(wrapper)) {
  set_data_type_json();
}

Item_sum_json::~Item_sum_json() = default;

bool Item_sum_json::check_wf_semantics(THD *thd, SELECT_LEX *select,
                                       Window_evaluation_requirements *reqs) {
  return Item_sum::check_wf_semantics(thd, select, reqs);
}

bool Item_sum_json::fix_fields(THD *thd, Item **ref) {
  DBUG_ASSERT(!fixed);
  result_field = nullptr;

  if (super::fix_fields(thd, ref)) return true; /* purecov: inspected */

  if (init_sum_func_check(thd)) return true;

  Disable_semijoin_flattening DSF(thd->lex->current_select(), true);

  for (uint i = 0; i < arg_count; i++) {
    if ((!args[i]->fixed && args[i]->fix_fields(thd, args + i)) ||
        args[i]->check_cols(1))
      return true;
  }

  if (resolve_type(thd)) return true;

  if (check_sum_func(thd, ref)) return true;

  maybe_null = true;
  null_value = true;
  fixed = true;
  return false;
}

String *Item_sum_json::val_str(String *str) {
  DBUG_ASSERT(fixed == 1);
  if (m_is_window_function) {
    if (wf_common_init()) return str;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for window functions, which does not use Aggregator, it has to be called
      here.
    */
    if (add()) return str;
  }
  if (null_value || m_wrapper->empty()) return nullptr;
  str->length(0);
  if (m_wrapper->to_string(str, true, func_name())) return error_str();

  return str;
}

bool Item_sum_json::val_json(Json_wrapper *wr) {
  if (m_is_window_function) {
    if (wf_common_init()) return true;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for window functions, which does not use Aggregator, it has to be called
      here.
    */
    add();
  }
  if (null_value || m_wrapper->empty()) return true;

  /*
    val_* functions are called more than once in aggregates and
    by passing the dom some function will destroy it so a clone is needed.
  */
  *wr = Json_wrapper(m_wrapper->clone_dom(current_thd));
  return false;
}

double Item_sum_json::val_real() {
  if (m_is_window_function) {
    if (wf_common_init()) return 0.0;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for window functions, which does not use Aggregator, it has to be called
      here.
    */
    add();
  }
  if (null_value || m_wrapper->empty()) return 0.0;

  return m_wrapper->coerce_real(func_name());
}

longlong Item_sum_json::val_int() {
  if (m_is_window_function) {
    if (wf_common_init()) return 0;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for window functions, which does not use Aggregator, it has to be called
      here.
    */
    add();
  }
  if (null_value || m_wrapper->empty()) return 0;

  return m_wrapper->coerce_int(func_name());
}

my_decimal *Item_sum_json::val_decimal(my_decimal *decimal_value) {
  if (m_is_window_function) {
    if (wf_common_init()) return nullptr;
    /*
      For a group aggregate function, add() is called by Aggregator* classes;
      for window functions, which does not use Aggregator, it has to be called
      here.
    */
    add();
  }
  if (null_value || m_wrapper->empty()) {
    my_decimal_set_zero(decimal_value);
    return decimal_value;
  }

  return m_wrapper->coerce_decimal(decimal_value, func_name());
}

bool Item_sum_json::get_date(MYSQL_TIME *ltime, my_time_flags_t) {
  if (null_value || m_wrapper->empty()) return true;

  return m_wrapper->coerce_date(ltime, func_name());
}

bool Item_sum_json::get_time(MYSQL_TIME *ltime) {
  if (null_value || m_wrapper->empty()) return true;

  return m_wrapper->coerce_time(ltime, func_name());
}

void Item_sum_json::reset_field() {
  /* purecov: begin inspected */
  DBUG_ASSERT(0);  // Check JOIN::with_json_agg for more details.
  // Create the container
  clear();
  // Append element to the container.
  add();

  /*
    field_type is MYSQL_TYPE_JSON so Item::make_string_field will always
    create a Field_json(in Item_sum::create_tmp_field).
    The cast is need since Field does not expose store_json function.
  */
  Field_json *json_result_field = down_cast<Field_json *>(result_field);
  json_result_field->set_notnull();
  // Store the container inside the field.
  json_result_field->store_json(m_wrapper.get());
  /* purecov: end */
}

void Item_sum_json::update_field() {
  /* purecov: begin inspected */
  DBUG_ASSERT(0);  // Check JOIN::with_json_agg for more details.
  /*
    field_type is MYSQL_TYPE_JSON so Item::make_string_field will always
    create a Field_json(in Item_sum::create_tmp_field).
    The cast is need since Field does not expose store_json function.
  */
  Field_json *json_result_field = down_cast<Field_json *>(result_field);
  // Restore the container(m_wrapper) from the field
  json_result_field->val_json(m_wrapper.get());

  // Append elements to the container.
  add();
  // Store the container inside the field.
  json_result_field->store_json(m_wrapper.get());
  json_result_field->set_notnull();
  /* purecov: end */
}

Item_sum_json_array::Item_sum_json_array(
    THD *thd, Item_sum *item, unique_ptr_destroy_only<Json_wrapper> wrapper,
    unique_ptr_destroy_only<Json_array> array)
    : Item_sum_json(std::move(wrapper), thd, item),
      m_json_array(std::move(array)) {}

Item_sum_json_array::Item_sum_json_array(
    const POS &pos, Item *a, PT_window *w,
    unique_ptr_destroy_only<Json_wrapper> wrapper,
    unique_ptr_destroy_only<Json_array> array)
    : Item_sum_json(std::move(wrapper), pos, a, w),
      m_json_array(std::move(array)) {}

Item_sum_json_array::~Item_sum_json_array() = default;

void Item_sum_json_array::clear() {
  null_value = true;
  m_json_array->clear();

  // Set the array to the m_wrapper, but let Item_sum_json_array keep the
  // ownership.
  *m_wrapper = Json_wrapper(m_json_array.get(), true);
}

Item_sum_json_object::Item_sum_json_object(
    THD *thd, Item_sum *item, unique_ptr_destroy_only<Json_wrapper> wrapper,
    unique_ptr_destroy_only<Json_object> object)
    : Item_sum_json(std::move(wrapper), thd, item),
      m_json_object(std::move(object)) {}

Item_sum_json_object::Item_sum_json_object(
    const POS &pos, Item *a, Item *b, PT_window *w,
    unique_ptr_destroy_only<Json_wrapper> wrapper,
    unique_ptr_destroy_only<Json_object> object)
    : Item_sum_json(std::move(wrapper), pos, a, b, w),
      m_json_object(std::move(object)) {}

Item_sum_json_object::~Item_sum_json_object() = default;

void Item_sum_json_object::clear() {
  null_value = true;
  m_json_object->clear();

  // Set the object to the m_wrapper, but let Item_sum_json_object keep the
  // ownership.
  *m_wrapper = Json_wrapper(m_json_object.get(), true);
  m_key_map.clear();
}

bool Item_sum_json_object::check_wf_semantics(
    THD *thd, SELECT_LEX *select, Window_evaluation_requirements *r) {
  Item_sum_json::check_wf_semantics(thd, select, r);
  /*
    As Json_object always stores only the last value for a key,
    optimization/inversion for windowing function is not possible
    unless row of the stored key/value pair is known. In case of
    an ordered result, if its known that a row is the last peer
    in a window frame for a key, then that key/value pair can be
    removed from the Json_object. So we let
    process_buffered_windowing_record() know by setting
    needs_last_peer_in_frame to true.
  */
  const PT_order_list *order = m_window->effective_order_by();
  if (order != nullptr) {
    ORDER *o = order->value.first;
    if (o->item_ptr->real_item()->eq(args[0]->real_item(), false)) {
      r->needs_last_peer_in_frame = true;
      m_optimize = true;
    }
  }
  return false;
}

bool Item_sum_json_array::add() {
  DBUG_ASSERT(fixed == 1);
  DBUG_ASSERT(arg_count == 1);

  const THD *thd = base_select->parent_lex->thd;
  /*
     Checking if an error happened inside one of the functions that have no
     way of returning an error status. (reset_field(), update_field() or
     clear())
   */
  if (thd->is_error()) return error_json();

  try {
    if (m_is_window_function) {
      if (m_window->do_inverse()) {
        auto arr = down_cast<Json_array *>(m_wrapper->to_dom(thd));
        arr->remove(0);  // Remove the first element from the array
        arr->size() == 0 ? null_value = true : null_value = false;
        return false;
      }
    }
    Json_wrapper value_wrapper;
    // Get the value.
    if (get_atom_null_as_null(args, 0, func_name(), &m_value,
                              &m_conversion_buffer, &value_wrapper))
      return error_json();

    Json_dom_ptr value_dom(value_wrapper.to_dom(thd));
    value_wrapper.set_alias();  // release the DOM

    /*
      The m_wrapper always points to m_json_array or the result of
      deserializing the result_field in reset/update_field.
    */
    const auto arr = down_cast<Json_array *>(m_wrapper->to_dom(thd));
    if (arr->append_alias(std::move(value_dom)))
      return error_json(); /* purecov: inspected */

    null_value = false;
  } catch (...) {
    /* purecov: begin inspected */
    handle_std_exception(func_name());
    return error_json();
    /* purecov: end */
  }

  return false;
}

Item *Item_sum_json_array::copy_or_same(THD *thd) {
  if (m_is_window_function) return this;

  auto wrapper = make_unique_destroy_only<Json_wrapper>(thd->mem_root);
  if (wrapper == nullptr) return nullptr;

  unique_ptr_destroy_only<Json_array> array{::new (thd->mem_root) Json_array};
  if (array == nullptr) return nullptr;

  return new (thd->mem_root)
      Item_sum_json_array(thd, this, std::move(wrapper), std::move(array));
}

bool Item_sum_json_object::add() {
  DBUG_ASSERT(fixed == 1);
  DBUG_ASSERT(arg_count == 2);

  const THD *thd = base_select->parent_lex->thd;
  /*
     Checking if an error happened inside one of the functions that have no
     way of returning an error status. (reset_field(), update_field() or
     clear())
   */
  if (thd->is_error()) return error_json();

  try {
    // key
    Item *key_item = args[0];
    const char *safep;   // contents of key_item, possibly converted
    size_t safe_length;  // length of safep

    if (get_json_string(key_item, &m_tmp_key_value, &m_conversion_buffer,
                        &safep, &safe_length)) {
      my_error(ER_JSON_DOCUMENT_NULL_KEY, MYF(0));
      return error_json();
    }

    std::string key(safep, safe_length);

    if (m_is_window_function) {
      /*
        When a row is leaving a frame, we have two options:
        1. If rows are ordered according to the "key", then remove
        the key/value pair from Json_object if this row is the
        last row in peerset for that key.
        2. If unordered, reduce the count in the key map for this key.
        If the count is 0, remove the key/value pair from the Json_object.
      */
      if (m_window->do_inverse()) {
        auto object = down_cast<Json_object *>(m_wrapper->to_dom(thd));
        if (m_optimize)  // Option 1
        {
          if (m_window->is_last_row_in_peerset_within_frame())
            object->remove(key);
        } else  // Option 2
        {
          auto it = m_key_map.find(key);
          if (it != m_key_map.end()) {
            int count = it->second - 1;
            if (count > 0) {
              it->second = count;
            } else {
              m_key_map.erase(it);
              object->remove(key);
            }
          }
        }
        object->cardinality() == 0 ? null_value = true : null_value = false;
        return false;
      }
    }
    // value
    Json_wrapper value_wrapper;
    if (get_atom_null_as_null(args, 1, func_name(), &m_value,
                              &m_conversion_buffer, &value_wrapper))
      return error_json();

    /*
      The m_wrapper always points to m_json_object or the result of
      deserializing the result_field in reset/update_field.
    */
    Json_object *object = down_cast<Json_object *>(m_wrapper->to_dom(thd));
    if (object->add_alias(key, value_wrapper.to_dom(thd)))
      return error_json(); /* purecov: inspected */
    /*
      If rows in the window are not ordered based on "key", add this key
      to the key map.
    */
    if (m_is_window_function && !m_optimize) {
      int count = 1;
      auto it = m_key_map.find(key);
      if (it != m_key_map.end()) {
        count = count + it->second;
        it->second = count;
      } else
        m_key_map.emplace(std::make_pair(key, count));
    }

    null_value = false;
    // object will take ownership of the value
    value_wrapper.set_alias();
  } catch (...) {
    /* purecov: begin inspected */
    handle_std_exception(func_name());
    return error_json();
    /* purecov: end */
  }

  return false;
}

Item *Item_sum_json_object::copy_or_same(THD *thd) {
  if (m_is_window_function) return this;

  auto wrapper = make_unique_destroy_only<Json_wrapper>(thd->mem_root);
  if (wrapper == nullptr) return nullptr;

  unique_ptr_destroy_only<Json_object> object{::new (thd->mem_root)
                                                  Json_object};
  if (object == nullptr) return nullptr;

  return new (thd->mem_root)
      Item_sum_json_object(thd, this, std::move(wrapper), std::move(object));
}

/**
  Resolve the fields in the GROUPING function.
  The GROUPING function can only appear in SELECT list or
  in HAVING clause and requires WITH ROLLUP. Check that this holds.
  We also need to check if all the arguments of the function
  are present in GROUP BY clause. As GROUP BY columns are not
  resolved at this time, we do it in SELECT_LEX::resolve_rollup().
  However, if the GROUPING function is found in HAVING clause,
  we can check here. Also, resolve_rollup() does not
  check for items present in HAVING clause.

  @param[in]     thd        current thread
  @param[in,out] ref        reference to place where item is
                            stored
  @retval
    true  if error
  @retval
    false on success

*/
bool Item_func_grouping::fix_fields(THD *thd, Item **ref) {
  /*
    We do not allow GROUPING by position. However GROUP BY allows
    it for now.
  */
  Item **arg, **arg_end;
  for (arg = args, arg_end = args + arg_count; arg != arg_end; arg++) {
    if ((*arg)->type() == Item::INT_ITEM && (*arg)->basic_const_item()) {
      my_error(ER_WRONG_ARGUMENTS, MYF(0), "GROUPING function");
      return true;
    }
  }

  if (Item_func::fix_fields(thd, ref)) return true;

  // Make GROUPING function dependent upon all tables (prevents const-ness)
  used_tables_cache |= thd->lex->current_select()->all_tables_map();

  /*
    More than 64 args cannot be supported as the bitmask which is
    used to represent the result cannot accomodate.
  */
  if (arg_count > 64) {
    my_error(ER_INVALID_NO_OF_ARGS, MYF(0), "GROUPING", arg_count, "64");
    return true;
  }

  /*
    GROUPING() is not allowed in a WHERE condition or a JOIN condition and
    cannot be used without rollup.
  */
  SELECT_LEX *select = thd->lex->current_select();

  if (select->olap == UNSPECIFIED_OLAP_TYPE ||
      select->resolve_place == SELECT_LEX::RESOLVE_JOIN_NEST ||
      select->resolve_place == SELECT_LEX::RESOLVE_CONDITION) {
    my_error(ER_INVALID_GROUP_FUNC_USE, MYF(0));
    return true;
  }

  return false;
}

/**
  Evaluation of the GROUPING function.
  We check the type of the item for all the arguments of
  GROUPING function. If it's a NULL_RESULT_ITEM, set the bit for
  the field in the result. The result of the GROUPING function
  would be the integer bit mask having 1's for the arguments
  of type NULL_RESULT_ITEM.

  @return
  integer bit mask having 1's for the arguments which have a
  NULL in their result becuase of ROLLUP operation.
*/
longlong Item_func_grouping::val_int() {
  longlong result = 0;
  for (uint i = 0; i < arg_count; i++) {
    Item *real_item = args[i];
    while (real_item->type() == REF_ITEM)
      real_item = *((down_cast<Item_ref *>(real_item))->ref);
    /*
      Note: if the current input argument is an 'Item_null_result',
      then we know it is generated by rollup handler to fill the
      subtotal rows.
    */
    if (real_item->type() == NULL_RESULT_ITEM)
      result += 1 << (arg_count - (i + 1));
  }
  return result;
}

/**
  This function is expected to check if GROUPING function with
  its arguments is "group-invariant".
  However, GROUPING function produces only one value per
  group similar to the other set functions and the arguments
  to the GROUPING function are always present in GROUP BY (this
  is checked in resolve_rollup() which is called much earlier to
  aggregate_check_group). As a result, aggregate_check_group does
  not have to determine if the result of this function is
  "group-invariant".

  @retval
    true  if error
  @retval
    false on success
*/
bool Item_func_grouping::aggregate_check_group(uchar *arg) {
  Group_check *gc = reinterpret_cast<Group_check *>(arg);

  if (gc->is_stopped(this)) return false;

  if (gc->is_fd_on_source(this)) {
    gc->stop_at(this);
    return false;
  }
  return true;
}

void Item_func_grouping::update_used_tables() {
  Item_int_func::update_used_tables();
  set_grouping_func();
  set_rollup_expr();
  /*
    GROUPING function can never be a constant item. It's
    result always depends on ROLLUP result.
  */
  used_tables_cache |= current_thd->lex->current_select()->all_tables_map();
}
