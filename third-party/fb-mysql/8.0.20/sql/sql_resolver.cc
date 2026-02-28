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
  Implementation of name resolution stage


  @defgroup Query_Resolver  Query Resolver
  @{
*/

#include "sql/sql_resolver.h"

#include <stddef.h>
#include <sys/types.h>
#include <algorithm>
#include <functional>
#include <utility>

#include "field_types.h"
#include "lex_string.h"
#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "sql/aggregate_check.h"  // Group_check
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_single_table_access
#include "sql/derror.h"            // ER_THD
#include "sql/enum_query_type.h"
#include "sql/error_handler.h"  // View_error_handler
#include "sql/field.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_func.h"
#include "sql/item_row.h"
#include "sql/item_subselect.h"
#include "sql/item_sum.h"  // Item_sum
#include "sql/mem_root_array.h"
#include "sql/nested_join.h"
#include "sql/opt_hints.h"
#include "sql/opt_range.h"  // prune_partitions
#include "sql/opt_trace.h"  // Opt_trace_object
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_node_base.h"
#include "sql/parser_yystype.h"
#include "sql/query_options.h"
#include "sql/query_result.h"  // Query_result
#include "sql/sql_base.h"      // setup_fields
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_optimizer.h"  // Prepare_error_tracker
#include "sql/sql_select.h"
#include "sql/sql_test.h"   // print_where
#include "sql/sql_union.h"  // Query_result_union
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/window.h"
#include "template_utils.h"

static bool simplify_const_condition(THD *thd, Item **cond,
                                     bool remove_cond = true,
                                     bool *ret_cond_value = nullptr);
static TABLE_LIST *locate_derived(mem_root_deque<TABLE_LIST *> &tables,
                                  SELECT_LEX_UNIT *query_expression = nullptr);

/**
  Prepare query block for optimization.

  Resolve table and column information.
  Resolve all expressions (item trees), ie WHERE clause, join conditions,
  GROUP BY clause, HAVING clause, ORDER BY clause, LIMIT clause.
  Prepare all subqueries recursively as part of resolving the expressions.
  Apply permanent transformations to the abstract syntax tree, such as
  semi-join transformation, derived table transformation, elimination of
  constant values and redundant clauses (e.g ORDER BY, GROUP BY).

  @param thd    thread handler

  @returns false if success, true if error

  @note on privilege checking for SELECT query that possibly contains view
        or derived table references:

   - When this function is called, it is assumed that the precheck() function
     has been called. precheck() ensures that the user has some SELECT
     privileges to the tables involved in the query. When resolving views
     it has also been established that the user has some privileges for them.
     To prepare a view for privilege checking, it is also needed to call
     check_view_privileges() after views have been merged into the query.
     This is not necessary for unnamed derived tables since it has already
     been established that we have SELECT privileges for the underlying tables
     by the precheck functions. (precheck() checks a query without resolved
     views, ie. before tables are opened, so underlying tables of views
     are not yet available).

   - When a query block is resolved, always ensure that the user has SELECT
     privileges to the columns referenced in the WHERE clause, the join
     conditions, the GROUP BY clause, the HAVING clause and the ORDER BY clause.

   - When resolving the outer-most query block, ensure that the user also has
     SELECT privileges to the columns in the selected expressions.

   - When setting up a derived table or view for materialization, ensure that
     the user has SELECT privileges to the columns in the selected expressions

   - Column privileges are normally checked by Item_field::fix_fields().
     Exceptions are select list of derived tables/views which are checked
     in TABLE_LIST::setup_materialized_derived(), and natural/using join
     conditions that are checked in mark_common_columns().

   - As far as INSERT, UPDATE and DELETE statements have the same expressions
     as a SELECT statement, this note applies to those statements as well.
*/
bool SELECT_LEX::prepare(THD *thd) {
  DBUG_TRACE;

  // We may do subquery transformation, or Item substitution:
  Prepare_error_tracker tracker(thd);

  DBUG_ASSERT(this == thd->lex->current_select());
  DBUG_ASSERT(join == nullptr);
  DBUG_ASSERT(!thd->is_error());

  // If this query block is a table value constructor, a lot of the preparation
  // done in SELECT_LEX::prepare becomes irrelevant. Thus we call our own
  // SELECT_LEX::prepare_values in this case.
  if (is_table_value_constructor) return prepare_values(thd);

  SELECT_LEX_UNIT *const unit = master_unit();

  if (!top_join_list.empty()) propagate_nullability(&top_join_list, false);

  /*
    Determine whether it is suggested to merge immediate derived tables, based
    on the placement of the query block:
      - DTs belonging to outermost query block: always
      - DTs belonging to first level subqueries: Yes if inside SELECT statement,
        no otherwise (including UPDATE and DELETE).
        This is required to support a workaround for allowing subqueries
        containing the same table as is target for delete or update,
        by forcing a materialization of the subquery.
      - All other cases inherit status of parent query block.
  */
  allow_merge_derived = outer_select() == nullptr ||
                        master_unit()->item == nullptr ||
                        (outer_select()->outer_select() == nullptr
                             ? parent_lex->sql_command == SQLCOM_SELECT
                             : outer_select()->allow_merge_derived);

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper_prepare(trace);
  Opt_trace_object trace_prepare(trace, "join_preparation");
  trace_prepare.add_select_number(select_number);
  Opt_trace_array trace_steps(trace, "steps");

  /*
    Setup the expressions in the SELECT list. Wait with privilege checking
    until all derived tables are resolved, except do privilege checking for
    subqueries inside a derived table.
    Need to be done here in order for table function's arguments to be fixed
    properly.
  */
  const bool check_privs =
      !thd->derived_tables_processing || master_unit()->item != nullptr;
  thd->mark_used_columns = check_privs ? MARK_COLUMNS_READ : MARK_COLUMNS_NONE;
  ulonglong want_privilege_saved = thd->want_privilege;
  thd->want_privilege = check_privs ? SELECT_ACL : 0;

  /*
    Expressions in lateral join can't refer to item list, thus item list lookup
    shouldn't be allowed during table/table function setup.
  */
  is_item_list_lookup = false;

  /* Check that all tables, fields, conds and order are ok */

  if (!(active_options() & OPTION_SETUP_TABLES_DONE)) {
    if (setup_tables(thd, get_table_list(), false)) return true;

    if ((derived_table_count || table_func_count) &&
        resolve_placeholder_tables(thd, true))
      return true;

    // Wait with privilege checking until all derived tables are resolved.
    if (derived_table_count && !thd->derived_tables_processing &&
        check_view_privileges(thd, SELECT_ACL, SELECT_ACL))
      return true;
  }

  is_item_list_lookup = true;

  // Precompute and store the row types of NATURAL/USING joins.
  if (leaf_table_count >= 2 &&
      setup_natural_join_row_types(thd, join_list, &context))
    return true;

  Mem_root_array<Item_exists_subselect *> sj_candidates_local(thd->mem_root);
  set_sj_candidates(&sj_candidates_local);

  /*
    Item and Item_field CTORs will both increment some counters
    in current_select(), based on the current parsing context.
    We are not parsing anymore: any new Items created now are due to
    query rewriting, so stop incrementing counters.
   */
  DBUG_ASSERT(parsing_place == CTX_NONE);
  parsing_place = CTX_NONE;

  resolve_place = RESOLVE_SELECT_LIST;

  if (with_wild && setup_wild(thd)) return true;
  if (setup_base_ref_items(thd)) return true; /* purecov: inspected */

  // Initially, "all_fields" is the select list (after expansion of *).
  all_fields = fields_list;

  if (setup_fields(thd, base_ref_items, fields_list, thd->want_privilege,
                   &all_fields, true, false))
    return true;

  resolve_place = RESOLVE_NONE;

  const nesting_map save_allow_sum_func = thd->lex->allow_sum_func;
  const nesting_map save_deny_window_func = thd->lex->m_deny_window_func;

  // Do not allow local set functions for join conditions, WHERE and GROUP BY
  thd->lex->allow_sum_func &= ~((nesting_map)1 << nest_level);

  thd->mark_used_columns = MARK_COLUMNS_READ;
  thd->want_privilege = SELECT_ACL;

  // Set up join conditions and WHERE clause
  if (setup_conds(thd)) return true;

  // Set up the GROUP BY clause
  int all_fields_count = all_fields.elements;
  if (group_list.elements && setup_group(thd)) return true;
  hidden_group_field_count = all_fields.elements - all_fields_count;

  // Allow local set functions in HAVING and ORDER BY
  thd->lex->allow_sum_func |= (nesting_map)1 << nest_level;

  // Windowing is not allowed with HAVING
  thd->lex->m_deny_window_func |= (nesting_map)1 << nest_level;

  // Setup the HAVING clause
  if (m_having_cond) {
    DBUG_ASSERT(m_having_cond->is_bool_func());
    thd->where = "having clause";
    having_fix_field = true;
    resolve_place = RESOLVE_HAVING;
    if (!m_having_cond->fixed &&
        (m_having_cond->fix_fields(thd, &m_having_cond) ||
         m_having_cond->check_cols(1)))
      return true;

    // Simplify the having condition if it is a const item
    if (m_having_cond->const_item() && !thd->lex->is_view_context_analysis() &&
        !m_having_cond->walk(&Item::is_non_const_over_literals,
                             enum_walk::POSTFIX, nullptr) &&
        simplify_const_condition(thd, &m_having_cond))
      return true;

    having_fix_field = false;
    resolve_place = RESOLVE_NONE;
  }

  thd->lex->m_deny_window_func = save_deny_window_func;

  if (m_having_cond && olap == ROLLUP_TYPE &&
      resolve_rollup_item(thd, m_having_cond))
    return true;

  // Set up the ORDER BY clause
  all_fields_count = all_fields.elements;
  if (order_list.elements) {
    if (setup_order(thd, base_ref_items, get_table_list(), fields_list,
                    all_fields, order_list.first))
      return true;
  }

  hidden_order_field_count = all_fields.elements - all_fields_count;

  /*
    Query block is completely resolved, except for windows (see below) which
    handles its own, so restore set function allowance.
  */
  thd->lex->allow_sum_func = save_allow_sum_func;

  /*
    Permanently remove redundant parts from the query if
      1) This is a subquery
      2) This is the first time this query is prepared (since the
         transformation is permanent)
      3) Not normalizing a view. Removal should take place when a
         query involving a view is optimized, not when the view
         is created
  */
  if (unit->item &&                           // 1)
      first_execution &&                      // 2)
      !thd->lex->is_view_context_analysis())  // 3)
  {
    remove_redundant_subquery_clauses(thd, hidden_group_field_count);
  }

  /*
    Set up windows after setup_order() (as the query's ORDER BY may contain
    window functions), and before setup_order_final() (as such function needs
    to know about implicit grouping which may be induced by an aggregate
    function in the window's PARTITION or ORDER clause).
  */
  if (m_windows.elements != 0 &&
      Window::setup_windows(thd, this, base_ref_items, get_table_list(),
                            fields_list, all_fields, m_windows))
    return true;

  if (order_list.elements && setup_order_final(thd))
    return true; /* purecov: inspected */

  thd->want_privilege = want_privilege_saved;

  if (is_distinct() && is_grouped() && hidden_group_field_count == 0 &&
      olap == UNSPECIFIED_OLAP_TYPE) {
    /*
      All GROUP expressions are in SELECT list, so resulting rows are distinct.
      ROLLUP is not specified, so adds no row. So all rows in the result set
      are distinct, DISTINCT is useless.
      @todo could remove DISTINCT if ROLLUP were specified and all GROUP
      expressions were non-nullable, because ROLLUP adds only NULL values.
      Currently, ROLLUP+DISTINCT is rejected because executor cannot handle
      it in all cases.
    */
    remove_base_options(SELECT_DISTINCT);
  }

  /*
    Printing the expanded query should happen here and not elsewhere, because
    when a view is merged (when the view is opened in open_tables()), the
    parent query's select_lex does not yet contain a correct WHERE clause (it
    misses the view's merged WHERE clause). This is corrected only just above,
    in TABLE_LIST::prep_where(), called by
    setup_without_group()->setup_conds().
    We also have to wait for fix_fields() on HAVING, above.
    At this stage, we also have properly set up Item_ref-s.
  */
  {
    Opt_trace_object trace_wrapper(trace);
    opt_trace_print_expanded_query(thd, this, &trace_wrapper);
  }

  /*
    When normalizing a view (like when writing a view's body to the FRM),
    subquery transformations don't apply (if they did, IN->EXISTS could not be
    undone in favour of materialization, when optimizing a later statement
    using the view)
  */
  if (unit->item &&                     // This is a subquery
      this != unit->fake_select_lex &&  // A real query block
                                        // Not normalizing a view
      !thd->lex->is_view_context_analysis()) {
    // Query block represents a subquery within an IN/ANY/ALL/EXISTS predicate
    if (resolve_subquery(thd)) return true;
  }

  // Transform eligible scalar subqueries to derived tables.
  //
  // Don't transform if analyzing a view: the resulting query may not be
  // compilable from sqldump, (due to group by check/visibility in HAVING).
  //
  // Don't transform if the switch scalar_subquery_to_derived is false.
  //
  // We do not perform the transformation during EXECUTE [1], but keep the plan
  // from the PREPARE, even if the switch has been changed. We do however,
  // refresh [2] the transform during EXECUTE if it was performed during
  // PREPARE. Thus can be simplified after WL#6570.
  //
  // Note that the transformation must precede m_having_cond->split_sum_func2
  // below since substitutions may be made in the HAVING clause which would not
  // otherwise get done.

  if (!(thd->lex->context_analysis_only & CONTEXT_ANALYSIS_ONLY_VIEW) &&
      ((((parent_lex->m_sql_cmd != nullptr &&
          thd->secondary_engine_optimization() ==
              Secondary_engine_optimization::SECONDARY)) &&
        thd->stmt_arena->get_state() != Query_arena::STMT_EXECUTED /* [1] */) ||
       locate_derived(*join_list) != nullptr /* [2] */) &&
      transform_scalar_subqueries_to_derived(thd))
    return true; /* purecov: inspected */

  /*
    If GROUPING function is present in having condition -
    1. Set that the evaluation of this condition depends on rollup
    result.
    2. Add a reference to the condition so that result is stored
    after evalution.
  */
  if (m_having_cond && (m_having_cond->has_aggregation() ||
                        m_having_cond->has_grouping_func())) {
    m_having_cond->split_sum_func2(thd, base_ref_items, all_fields,
                                   &m_having_cond, true);
  }
  if (inner_sum_func_list) {
    Item_sum *end = inner_sum_func_list;
    Item_sum *item_sum = end;
    do {
      item_sum = item_sum->next_sum;
      item_sum->split_sum_func2(thd, base_ref_items, all_fields, nullptr,
                                false);
    } while (item_sum != end);
  }

  if (group_list.elements) {
    /*
      Because HEAP tables can't index BIT fields we need to use an
      additional hidden field for grouping because later it will be
      converted to a LONG field. Original field will remain of the
      BIT type and will be returned to a client.
    */
    for (ORDER *ord = group_list.first; ord; ord = ord->next) {
      if ((*ord->item)->type() == Item::FIELD_ITEM &&
          (*ord->item)->data_type() == MYSQL_TYPE_BIT) {
        Item_field *field = new Item_field(thd, *(Item_field **)ord->item);
        ord->item = add_hidden_item(field);
      }
    }
  }

  // Setup full-text functions after resolving HAVING
  if (has_ft_funcs() && setup_ftfuncs(thd, this)) return true;

  if (query_result() && query_result()->prepare(thd, fields_list, unit))
    return true;

  if (has_sj_candidates() && flatten_subqueries(thd)) return true;

  set_sj_candidates(nullptr);

  /*
    When reaching the top-most query block, or the next-to-top query block for
    the SQL command SET and for SP instructions (indicated with SQLCOM_END),
    apply local transformations to this query block and all underlying query
    blocks.
  */
  if ((outer_select() == nullptr ||
       ((parent_lex->sql_command == SQLCOM_SET_OPTION ||
         parent_lex->sql_command == SQLCOM_END) &&
        outer_select()->outer_select() == nullptr)) &&
      !skip_local_transforms) {
    /*
      This code is invoked in the following cases:
      - if this is an outer-most query block of a SELECT or multi-table
        UPDATE/DELETE statement. Notice that for a UNION, this applies to
        all query blocks. It also applies to a fake_select_lex object.
      - if this is one of highest-level subqueries, if the statement is
        something else; like subq-i in:
          UPDATE t1 SET col1=(subq-1), col2=(subq-2);
      - If this is a subquery in a SET command
        @todo: Refactor SET so that this is not needed.
      - INSERT may in some cases alter the sequence of preparation calls, by
        setting the skip_local_transforms flag before calling prepare().

      Local transforms are applied after query block merging.
      This means that we avoid unnecessary invocations, as local transforms
      would otherwise have been performed first before query block merging and
      then another time after query block merging.
      Thus, apply_local_transforms() may run only after the top query
      is finished with query block merging. That's why
      apply_local_transforms() is initiated only by the top query, and then
      recurses into subqueries.
    */
    if (apply_local_transforms(thd, true)) return true;
  }

  /*
    If the query directly contains windowing, remove any unused explicit window
    definitions.
  */
  if (m_windows.elements != 0) Window::remove_unused_windows(thd, m_windows);

  DBUG_ASSERT(!thd->is_error());
  return false;
}

/**
  Prepare a table value constructor query block for optimization.

  In the case of a table value constructor SELECT_LEX, we return the result of
  this function from SELECT_LEX::prepare, instead of doing the standard prepare
  routine.

  For a table value constructor block, most preparation of a standard SELECT_LEX
  becomes irrelevant (in particular INTO, FROM, WHERE, GROUP, HAVING and
  WINDOW). We therefore substitute the standard resolving routine with this one,
  which is simply responsible for resolving the expressions contained in VALUES,
  as well as the query result.

  @param thd    thread handler

  @returns false if success, true if error
 */

bool SELECT_LEX::prepare_values(THD *thd) {
  SELECT_LEX_UNIT *const unit = master_unit();

  if (resolve_table_value_constructor_values(thd)) return true;

  // Setup the HAVING clause, duplicating code from SELECT_LEX::prepare. This is
  // strictly necessary in the case of PREPARE statements, where
  // resolve_subquery may rewrite its SELECT_LEX to use m_having_cond.
  //
  // For example, a query like `SELECT * FROM t WHERE (a, b) IN (VALUES ROW(1,
  // 10))` may be rewritten such that the SELECT_LEX within the IN subquery has
  // a HAVING clause with an Item_cond_and. This must be taken into account
  // during the second preparation that is done when the prepared statement is
  // _executed_; we now have to resolve m_having_cond properly.
  //
  // Note that this duplicated code should be removed in the future. TODO: for
  // wl#9384, which refactors DML statement preparation to be done only once.
  if (m_having_cond) {
    DBUG_ASSERT(m_having_cond->is_bool_func());
    thd->where = "having clause";
    having_fix_field = true;
    resolve_place = RESOLVE_HAVING;
    if (!m_having_cond->fixed &&
        (m_having_cond->fix_fields(thd, &m_having_cond) ||
         m_having_cond->check_cols(1)))
      return true; /* purecov: inspected */

    DBUG_ASSERT(!m_having_cond->const_item());

    having_fix_field = false;
    resolve_place = RESOLVE_NONE;
  }

  // Again, duplicating checks that are also done in SELECT_LEX::prepare for
  // resolving subqueries. This should, like the resolving of m_having_clause
  // above, be refactored such that there is less duplication of code from
  // SELECT_LEX::prepare.
  if (unit->item &&                     // This is a subquery
      this != unit->fake_select_lex &&  // A real query block
                                        // Not normalizing a view
      !thd->lex->is_view_context_analysis()) {
    // Query block represents a subquery within an IN/ANY/ALL/EXISTS predicate
    if (resolve_subquery(thd)) return true;
  }

  if (query_result() && query_result()->prepare(thd, item_list, unit))
    return true; /* purecov: inspected */

  return false;
}

/**
  Check whether the given table function or lateral derived table depends on a
  table which it's RIGHT JOINed to. An error is thrown if such dependency is
  found. For example:
  T RIGHT JOIN [lateral derived depending on T].
  Note that there is no need to look for the symmetric situation:
  [lateral derived depending on T] LEFT JOIN T
  because name resolution in the lateral derived table's body stops before T.

  @param table_ref  Table representing the table function or lateral derived
                    table
  @param map        Tables on which table_ref depends.

  @returns
    false no dependency is found
    true  otherwise
*/

bool check_right_lateral_join(TABLE_LIST *table_ref, table_map map) {
  TABLE_LIST *orig_table = table_ref;

  for (; table_ref->embedding && map; table_ref = table_ref->embedding) {
    for (TABLE_LIST *table : table_ref->embedding->nested_join->join_list) {
      table_map cur_table_map =
          table->nested_join ? table->nested_join->used_tables : table->map();
      if (cur_table_map & map) {
        if (table->outer_join == JOIN_TYPE_RIGHT) {
          my_error(ER_TF_FORBIDDEN_JOIN_TYPE, MYF(0), orig_table->alias);
          return true;
        }
        map &= (~cur_table_map);
        if (!map) return false;
      }
    }
  }
  return false;
}

/**
  Apply local transformations, such as query block merging.
  Also perform partition pruning, which is most effective after transformations
  have been done.

  @param thd      thread handler
  @param prune    if true, then prune partitions based on const conditions

  @returns false if success, true if error

  Since this is called after flattening of query blocks, call this function
  while traversing the query block hierarchy top-down.
*/

bool SELECT_LEX::apply_local_transforms(THD *thd, bool prune) {
  DBUG_TRACE;

  // No transformations required when creating a view only
  if (thd->lex->context_analysis_only & CONTEXT_ANALYSIS_ONLY_VIEW)
    return false;

  /*
    If query block contains one or more merged derived tables/views,
    walk through lists of columns in select lists and remove unused columns.
  */
  if (derived_table_count && first_execution &&
      !(thd->lex->is_view_context_analysis()))
    delete_unused_merged_columns(&top_join_list);

  for (SELECT_LEX_UNIT *unit = first_inner_unit(); unit;
       unit = unit->next_unit()) {
    for (SELECT_LEX *sl = unit->first_select(); sl; sl = sl->next_select()) {
      // Prune all subqueries, regardless of passed argument
      if (sl->apply_local_transforms(thd, true)) return true;
    }
    if (unit->fake_select_lex &&
        unit->fake_select_lex->apply_local_transforms(thd, false))
      return true;
  }

  if (first_execution && !thd->lex->is_view_context_analysis()) {
    /*
      The following code will allocate the new items in a permanent
      MEMROOT for prepared statements and stored procedures.
    */
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    // Convert all outer joins to inner joins if possible
    if (simplify_joins(thd, &top_join_list, true, false, &m_where_cond))
      return true;
    if (record_join_nest_info(&top_join_list)) return true;
    build_bitmap_for_nested_joins(&top_join_list, 0);

    /*
      Here are the reasons why we do the following check here (i.e. late).
      * setup_fields () may have done split_sum_func () on aggregate items of
      the SELECT list, so for reliable comparison of the ORDER BY list with
      the SELECT list, we need to wait until split_sum_func() is done with
      the ORDER BY list.
      * we get resolved expressions "most of the time", which is always a good
      thing. Some outer references may not be resolved, though.
      * we need nested_join::used_tables, and this member is set in
      simplify_joins()
      * simplify_joins() does outer-join-to-inner conversion, which increases
      opportunities for functional dependencies (weak-to-strong, which is
      unusable, becomes strong-to-strong).
      * check_only_full_group_by() is dependent on processing done by
      simplify_joins() (for example it uses the value of
      SELECT_LEX::outer_join).

      The drawback is that the checks are after resolve_subquery(), so can
      meet strange "internally added" items.

      Note that when we are creating a view, simplify_joins() doesn't run so
      check_only_full_group_by() cannot run, any error will be raised only
      when the view is later used (SELECTed...)
    */
    if ((is_distinct() || is_grouped()) &&
        (thd->variables.sql_mode & MODE_ONLY_FULL_GROUP_BY) &&
        check_only_full_group_by(thd))
      return true;
  }

  fix_prepare_information(thd);

  /*
    Prune partitions for all query blocks after query block merging, if
    pruning is wanted.
  */
  if (partitioned_table_count && prune) {
    for (TABLE_LIST *tbl = leaf_tables; tbl; tbl = tbl->next_leaf) {
      /*
        This will only prune constant conditions, which will be used for
        lock pruning.
      */
      if (prune_partitions(thd, tbl->table,
                           tbl->join_cond() ? tbl->join_cond() : m_where_cond))
        return true; /* purecov: inspected */

      if (tbl->table->all_partitions_pruned_away &&
          !tbl->is_inner_table_of_outer_join())
        set_empty_query();
    }
  }

  return false;
}

/**
  Try to replace a const condition with a simple constant.
  A true condition is replaced with an empty item pointer if remove_cond
  is true. Else it is replaced witha a constant TRUE.
  A false condition is replaced with the constant FALSE.

  @param thd            Thread handler
  @param[in,out]  cond  Address of condition, may be substituted with a literal
  @param remove_cond    If true removes a "true" condition. Else replaces
                        it with a constant TRUE.
  @param ret_cond_value Store the result of the evaluated const condition

  @returns false if success, true if error
*/

static bool simplify_const_condition(THD *thd, Item **cond, bool remove_cond,
                                     bool *ret_cond_value) {
  DBUG_ASSERT((*cond)->const_item());

  bool cond_value;

  /* Push ignore / strict error handler */
  Ignore_error_handler ignore_handler;
  Strict_error_handler strict_handler;
  if (thd->lex->is_ignore())
    thd->push_internal_handler(&ignore_handler);
  else if (thd->install_strict_handler())
    thd->push_internal_handler(&strict_handler);

  bool err = eval_const_cond(thd, *cond, &cond_value);
  /* Pop ignore / strict error handler */
  if (thd->lex->is_ignore() || thd->install_strict_handler())
    thd->pop_internal_handler();

  if (err) return true;

  DBUG_EXECUTE("where",
               print_where(thd, *cond, "simplify_const_cond", QT_ORDINARY););
  if (cond_value) {
    if (remove_cond)
      *cond = nullptr;
    else {
      Prepared_stmt_arena_holder ps_arena_holder(thd);
      *cond = new (thd->mem_root) Item_func_true();
      if (*cond == nullptr) return true;
    }
  } else if ((*cond)->type() != Item::INT_ITEM) {
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    *cond = new (thd->mem_root) Item_func_false();
    if (*cond == nullptr) return true;
  }
  if (ret_cond_value) *ret_cond_value = cond_value;
  return false;
}

/**
  Check if the subquery predicate can be executed via materialization.

  @param thd       THD
  @param select_lex SELECT_LEX of the subquery
  @param outer      Parent SELECT_LEX (outer to subquery)

  @return true if subquery allows materialization, false otherwise.
*/

bool Item_in_subselect::subquery_allows_materialization(
    THD *thd, SELECT_LEX *select_lex, const SELECT_LEX *outer) {
  const uint elements = unit->first_select()->item_list.elements;
  DBUG_TRACE;
  DBUG_ASSERT(elements >= 1);
  DBUG_ASSERT(left_expr->cols() == elements);

  OPT_TRACE_TRANSFORM(&thd->opt_trace, trace_wrapper, trace_mat,
                      select_lex->select_number, "IN (SELECT)",
                      "materialization");

  const char *cause = nullptr;
  if (substype() != Item_subselect::IN_SUBS) {
    // Subq-mat cannot handle 'outer_expr > {ANY|ALL}(subq)'...
    cause = "not an IN predicate";
  } else if (select_lex->is_part_of_union()) {
    // Subquery must be a single query specification clause (not a UNION)
    cause = "in UNION";
  } else if (!select_lex->master_unit()->first_select()->leaf_tables) {
    // Subquery has no tables, hence no point in materializing.
    cause = "no inner tables";
  } else if (!outer->join) {
    /*
      Maybe this is a subquery of a single table UPDATE/DELETE (TODO:
      handle this by switching to multi-table UPDATE/DELETE).
    */
    cause = "parent query has no JOIN";
  } else if (!outer->leaf_tables) {
    // The upper query is SELECT ... FROM DUAL. No gain in materializing.
    cause = "no tables in outer query";
  } else if (dependent_before_in2exists()) {
    /*
      Subquery should not be correlated; the correlation due to predicates
      injected by IN->EXISTS does not count as we will remove them if we
      choose materialization.

      TODO:
      This is an overly restrictive condition. It can be extended to:
         (Subquery is non-correlated ||
          Subquery is correlated to any query outer to IN predicate ||
          (Subquery is correlated to the immediate outer query &&
           Subquery !contains {GROUP BY, ORDER BY [LIMIT],
           aggregate functions}) && subquery predicate is not under "NOT IN"))
    */
    cause = "correlated";
  } else {
    /*
      Check that involved expression types allow materialization.
      This is a temporary fix for BUG#36752; see bug report for
      description of restrictions we need to put on the compared expressions.
    */
    DBUG_ASSERT(left_expr->fixed);
    // @see comment in Item_subselect::element_index()
    bool has_nullables = left_expr->maybe_null;

    List_iterator<Item> it(unit->first_select()->item_list);
    for (uint i = 0; i < elements; i++) {
      Item *const inner_item = it++;
      Item *const outer_item = left_expr->element_index(i);
      if (!types_allow_materialization(outer_item, inner_item)) {
        cause = "type mismatch";
        break;
      }
      if (inner_item->is_blob_field())  // 6
      {
        cause = "inner blob";
        break;
      }
      has_nullables |= inner_item->maybe_null;
    }

    if (!cause) {
      trace_mat.add("has_nullable_expressions", has_nullables);
      /*
        Subquery materialization cannot handle NULLs partial matching
        properly, yet. If the outer or inner values are NULL, the
        subselect_hash_sj_engine may reply FALSE when it should reply UNKNOWN.
        So, we must limit it to those three cases:
        - when FALSE and UNKNOWN are equivalent answers. I.e. this is a a
        top-level predicate (this implies it is not negated).
        - when outer and inner values cannot be NULL.
        - when there is a single inner column (because for this we have a
        limited implementation of NULLs partial matching).
      */
      trace_mat.add("treat_UNKNOWN_as_FALSE", abort_on_null);

      if (!abort_on_null && has_nullables && (elements > 1))
        cause = "cannot_handle_partial_matches";
      else {
        trace_mat.add("possible", true);
        return true;
      }
    }
  }
  DBUG_ASSERT(cause != nullptr);
  trace_mat.add("possible", false).add_alnum("cause", cause);
  return false;
}

/**
  Make list of leaf tables of join table tree

  @param list    pointer to pointer on list first element
                 Must be set to NULL before first (recursive) call
  @param tables  table list

  @returns pointer on pointer to next_leaf of last element
*/

static TABLE_LIST **make_leaf_tables(TABLE_LIST **list, TABLE_LIST *tables) {
  for (TABLE_LIST *table = tables; table; table = table->next_local) {
    // A mergable view is not allowed to have a table pointer.
    DBUG_ASSERT(!(table->is_view() && table->is_merged() && table->table));
    if (table->merge_underlying_list) {
      DBUG_ASSERT(table->is_merged());

      list = make_leaf_tables(list, table->merge_underlying_list);
    } else {
      *list = table;
      list = &table->next_leaf;
    }
  }
  return list;
}

/**
  Check privileges for the view tables merged into a query block.

  @param thd                   Thread context.
  @param want_privilege_first  Privileges requested for the first leaf.
  @param want_privilege_next   Privileges requested for the remaining leaves.

  @note Beware that it can't properly check privileges in cases when
        table being changed is not the first table in the list of leaf
        tables (for example, for multi-UPDATE).

  @note The inner loop is slightly inefficient. A view will have its privileges
        checked once for every base table that it refers to.

  @returns false if success, true if error.
*/

bool SELECT_LEX::check_view_privileges(THD *thd, ulong want_privilege_first,
                                       ulong want_privilege_next) {
  ulong want_privilege = want_privilege_first;
  Internal_error_handler_holder<View_error_handler, TABLE_LIST> view_handler(
      thd, true, leaf_tables);

  for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
    for (TABLE_LIST *ref = tl; ref->referencing_view;
         ref = ref->referencing_view) {
      if (check_single_table_access(thd, want_privilege, ref, false))
        return true;
    }
    want_privilege = want_privilege_next;
  }
  return false;
}

/**
  Set up table leaves in the query block based on list of tables.

  @param thd           Thread handler
  @param tables        List of tables to handle
  @param select_insert It is SELECT ... INSERT command

  @note
    Check also that the 'used keys' and 'ignored keys' exists and set up the
    table structure accordingly.
    Create a list of leaf tables.

    This function has to be called for all tables that are used by items,
    as otherwise table->map is not set and all Item_field will be regarded
    as const items.

  @returns False on success, true on error
*/

bool SELECT_LEX::setup_tables(THD *thd, TABLE_LIST *tables,
                              bool select_insert) {
  DBUG_TRACE;

  DBUG_ASSERT((select_insert && !tables->next_name_resolution_table) ||
              !tables ||
              (context.table_list && context.first_name_resolution_table));

  leaf_tables = nullptr;
  (void)make_leaf_tables(&leaf_tables, tables);

  TABLE_LIST *first_select_table = nullptr;
  if (select_insert) {
    // "insert_table" is needed for remap_tables().
    thd->lex->insert_table = leaf_tables->top_table();

    // Get first table in SELECT part
    first_select_table = thd->lex->insert_table->next_local;

    // Then, find the first leaf table
    if (first_select_table)
      first_select_table = first_select_table->first_leaf_table();
  }
  uint tableno = 0;
  leaf_table_count = 0;
  partitioned_table_count = 0;

  for (TABLE_LIST *tr = leaf_tables; tr; tr = tr->next_leaf, tableno++) {
    TABLE *const table = tr->table;
    if (tr == first_select_table) {
      /*
        For INSERT ... SELECT command, restart numbering from zero for first
        leaf table from SELECT part of query.
      */
      first_select_table = nullptr;
      tableno = 0;
    }
    if (tableno >= MAX_TABLES) {
      my_error(ER_TOO_MANY_TABLES, MYF(0), static_cast<int>(MAX_TABLES));
      return true;
    }
    tr->set_tableno(tableno);
    leaf_table_count++;  // Count the input tables of the query

    /*
      Only set hints on first execution.  Otherwise, hints will refer to
      wrong query block after semijoin transformation
    */
    if (first_execution && opt_hints_qb &&  // QB hints initialized
        !tr->opt_hints_table)               // Table hints are not adjusted yet
    {
      tr->opt_hints_table = opt_hints_qb->adjust_table_hints(tr);
    }

    if (table == nullptr) continue;
    table->set_pos_in_table_list(tr);

    if (!tr->opt_hints_table ||
        // Ignore old index hint processing if new style hints are specified.
        !tr->opt_hints_table->update_index_hint_maps(thd, tr->table)) {
      if (tr->process_index_hints(thd, table)) return true;
    }

    if (table->part_info)  // Count number of partitioned tables
      partitioned_table_count++;
  }

  /*
    @todo - consider calling this from SELECT::prepare() instead.
    It might save the test on select_insert to prevent check_unresolved()
    from being called twice for INSERT ... SELECT.
  */
  if (opt_hints_qb && !select_insert) opt_hints_qb->check_unresolved(thd);

  return false;
}

/**
  Re-map table numbers for all tables in a query block.

  @param thd           Thread handler

  @note
    This function needs to be called after setup_tables() has been called,
    and after a query block for a subquery has been merged into a parent
    quary block.
*/

void SELECT_LEX::remap_tables(THD *thd) {
  LEX *const lex = thd->lex;
  TABLE_LIST *first_select_table = nullptr;
  if (lex->insert_table && lex->insert_table == leaf_tables->top_table()) {
    /*
      For INSERT ... SELECT command, restart numbering from zero for first
      leaf table from SELECT part of query.
    */
    // Get first table in SELECT part
    first_select_table = lex->insert_table->next_local;

    // Then, recurse down to get first leaf table
    if (first_select_table)
      first_select_table = first_select_table->first_leaf_table();
  }

  uint tableno = 0;
  for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
    // Reset table number after having reached first table after insert table
    if (first_select_table == tl) tableno = 0;
    tl->set_tableno(tableno++);
  }
}

/**
  @brief Resolve derived table, view or table function references in query block

  @param thd            Pointer to THD.
  @param apply_semijoin if true, apply semi-join transform when possible

  @return false if success, true if error
*/

bool SELECT_LEX::resolve_placeholder_tables(THD *thd, bool apply_semijoin) {
  DBUG_TRACE;

  DBUG_ASSERT(derived_table_count > 0 || table_func_count > 0);

  /*
    A table function TF may depend on a previous derived table DT in the same
    FROM clause.
    Thus DT must be resolved before TF.
    It is the case, as the loops below progress from left to right in FROM.
    Alas this progress misses formerly-nested derived tables which are handled
    last, by the special branch 'if (!first_execution)'.
    So, assume a prepared statement:
    - SELECT FROM (SELECT FROM (SELECT ...) AS DT) AS DT1, JSON_TABLE(DT1.col);
    - DT1 is resolved, which materializes DT
    - JSON_TABLE is resolved
    - DT1 is merged
    Now we execute the statement:
    - in the first loop in the function,
        DT1 is not resolved again, as it was merged,
        and DT is not reached
    - we must thus defer resolution of JSON_TABLE
    - until DT is reached and resolved by the special branch 'if
    (!first_execution)' for formerly-nested derived tables
    - and then we can resolve JSON_TABLE.
    So, we run the code in this function twice: a first time for all
    non-JSON_TABLE tables, a second time for JSON_TABLE.
    @todo remove this in WL#6570.
  */
  bool do_tf_lateral = false;

loop:

  /*
    Prepare derived tables and views that belong to this query block.

    [1] A scalar subquery that have been transformed into materialized derived
        table in a PREPARE is skipped. Handled in
        transform_scalar_subqueries_to_derived.
  */
  for (TABLE_LIST *tl = get_table_list(); tl; tl = tl->next_local) {
    if ((!tl->is_view_or_derived() && !tl->is_table_function()) ||
        tl->is_merged() | tl->m_was_scalar_subquery /* [1] */)
      continue;
    if ((tl->is_table_function() ||
         (tl->is_derived() && tl->derived_unit()->m_lateral_deps)) ^
        do_tf_lateral)
      continue;
    if (tl->resolve_derived(thd, apply_semijoin)) return true;
    /*
      Merge the derived tables that do not require materialization into
      the current query block, if possible.
      Merging is only done once and must not be repeated for prepared execs.
    */
    if (!thd->lex->is_view_context_analysis() && first_execution) {
      if (tl->is_mergeable() && merge_derived(thd, tl))
        return true; /* purecov: inspected */
    }
    if (tl->is_merged()) continue;
    // Prepare remaining derived tables for materialization
    // Ensure that any derived table is merged or materialized after prepare.
    // Exception: if this is a derived table synthesized as part of a scalar to
    // derived table transformation, see [1] below. It may happen that this
    // transformation only happens at optimize time (if we only transform for
    // a sedondary engine), in which case none of the other predicates hold.
    DBUG_ASSERT(first_execution || !tl->is_view_or_derived() ||
                tl->is_merged() || tl->uses_materialization() ||
                tl->is_table_function() || tl->m_was_grouped2derived /* [1] */);
    /*
      If tl->resolve_derived() created the tmp table, don't create it again.
      @todo in WL#6570, eliminate tests of tl->table in this function.
    */
    if (tl->is_table_function()) {
      if (tl->setup_table_function(thd)) return true;
      continue;
    }
    if (tl->table == nullptr && tl->setup_materialized_derived(thd))
      return true;
    materialized_derived_table_count++;
  }

  /*
    The loops above will not reach derived tables that are contained within
    other derived tables that have been merged into the enclosing query block.
    To reach them, traverse the list of leaf tables and resolve and
    setup for materialization those derived tables that have no TABLE
    object (they have not been set up yet).

    [1] A scalar subquery that have been transformed into materialized derived
    table in a PREPARE is skipped. Handled in
    transform_scalar_subqueries_to_derived.
  */
  if (!first_execution) {
    for (TABLE_LIST *tl = leaf_tables; tl; tl = tl->next_leaf) {
      if (!(tl->is_view_or_derived() || tl->is_table_function()) ||
          tl->table != nullptr || tl->m_was_scalar_subquery /* [1] */)
        continue;
      if ((tl->is_table_function() ||
           (tl->is_derived() && tl->derived_unit()->m_lateral_deps)) ^
          do_tf_lateral)
        continue;
      DBUG_ASSERT(!tl->is_merged());
      if (tl->is_table_function()) {
        if (tl->setup_table_function(thd)) return true;
      }
      if (tl->resolve_derived(thd, apply_semijoin))
        return true; /* purecov: inspected */
      if (tl->table == nullptr && tl->setup_materialized_derived(thd))
        return true; /* purecov: inspected */
      // materialized_derived_table_count was incremented during preparation,
      // so do not do it once more.
    }
  }

  if (!do_tf_lateral) {
    do_tf_lateral = true;
    goto loop;
  }

  return false;
}

/**
  @brief Resolve predicate involving subquery

  @param thd     Pointer to THD.

  @retval false  Success.
  @retval true   Error.

  @details
  Perform early unconditional subquery transformations:
   - Convert subquery predicate into semi-join, or
   - Mark the subquery for execution using materialization, or
   - Perform IN->EXISTS transformation, or
   - Perform more/less ALL/ANY -> MIN/MAX rewrite
   - Substitute trivial scalar-context subquery with its value

  @todo for PS, make the whole block execute only on the first execution

*/

bool SELECT_LEX::resolve_subquery(THD *thd) {
  DBUG_TRACE;

  bool chose_semijoin = false;
  bool deterministic = true;
  SELECT_LEX *const outer = outer_select();

  /*
    @todo for PS, make the whole block execute only on the first execution.
    resolve_subquery() is only invoked in the first execution for subqueries
    that are transformed to semijoin, but for other subqueries, this function
    is called for every execution. One solution is perhaps to define
    exec_method in class Item_subselect and exit immediately if unequal to
    SubqueryExecMethod::EXEC_UNSPECIFIED.
  */
  Item_subselect *subq_predicate = master_unit()->item;
  DBUG_ASSERT(subq_predicate != nullptr);
  /**
    @note
    In this case: IN (SELECT ... UNION SELECT ...), SELECT_LEX::prepare() is
    called for each of the two UNION members, and in those two calls,
    subq_predicate is the same, not sure this is desired (double work?).
  */

  // Predicate for possible semi-join candidates (IN and EXISTS)
  Item_exists_subselect *const predicate =
      subq_predicate->substype() == Item_subselect::EXISTS_SUBS ||
              subq_predicate->substype() == Item_subselect::IN_SUBS
          ? down_cast<Item_exists_subselect *>(subq_predicate)
          : nullptr;

  // Predicate for IN subquery predicate
  Item_in_subselect *const in_predicate =
      subq_predicate->substype() == Item_subselect::IN_SUBS
          ? down_cast<Item_in_subselect *>(subq_predicate)
          : nullptr;

  if (in_predicate != nullptr) {
    thd->lex->set_current_select(outer);
    char const *save_where = thd->where;
    thd->where = "IN/ALL/ANY subquery";
    Disable_semijoin_flattening DSF(outer, true);

    bool result =
        !in_predicate->left_expr->fixed &&
        in_predicate->left_expr->fix_fields(thd, &in_predicate->left_expr);
    thd->lex->set_current_select(this);
    thd->where = save_where;
    if (result) return true;

    /*
      Check if the left and right expressions have the same # of
      columns, i.e. we don't have a case like
        (oe1, oe2) IN (SELECT ie1, ie2, ie3 ...)

      TODO why do we have this duplicated in IN->EXISTS transformers?
      psergey-todo: fix these: grep for duplicated_subselect_card_check
    */
    if (item_list.elements != in_predicate->left_expr->cols()) {
      my_error(ER_OPERAND_COLUMNS, MYF(0), in_predicate->left_expr->cols());
      return true;
    }
    if (in_predicate->left_expr->used_tables() & RAND_TABLE_BIT)
      deterministic = false;
  }

  DBUG_PRINT("info", ("Checking if subq can be converted to semi-join"));
  /*
    Check if we're in subquery that is a candidate for flattening into a
    semi-join (which is done in flatten_subqueries()). The requirements are:
      0. Semi-join is enabled
      1. Subquery predicate is an IN/=ANY or EXISTS predicate
      2. Subquery is a single query block (not a UNION)
      3. Subquery is not grouped (explicitly or implicitly)
         3x: outer aggregated expression are not accepted
      4. Subquery does not use HAVING
      5. Subquery does not use windowing functions
      6. Subquery predicate is (a) in an ON/WHERE clause, and (b) at
      the AND-top-level of that clause.
      7. Parent query block accepts semijoins (i.e we are not in a subquery of
      a single table UPDATE/DELETE (TODO: We should handle this at some
      point by switching to multi-table UPDATE/DELETE)
      8. We're not in a confluent table-less subquery, like "SELECT 1".
      9. No execution method was already chosen (by a prepared statement)
      10. Parent query block is not a confluent table-less query block.
      11. Neither parent nor child query block has straight join.
      12. Parent query block does not prohibit semi-join.
      13. LHS of IN predicate is deterministic
      14. The surrounding truth test, and the nullability of expressions,
      are compatible with the conversion.
  */
  if (semijoin_enabled(thd) &&                                    // 0
      predicate != nullptr &&                                     // 1
      !is_part_of_union() &&                                      // 2
      !is_grouped() &&                                            // 3
      !with_sum_func &&                                           // 3x
      having_cond() == nullptr &&                                 // 4
      !has_windows() &&                                           // 5
      (outer->resolve_place == SELECT_LEX::RESOLVE_CONDITION ||   // 6a
       outer->resolve_place == SELECT_LEX::RESOLVE_JOIN_NEST) &&  // 6a
      !outer->semijoin_disallowed &&                              // 6b
      outer->sj_candidates &&                                     // 7
      leaf_table_count > 0 &&                                     // 8
      predicate->exec_method ==                                   // 9
          SubqueryExecMethod::EXEC_UNSPECIFIED &&                 // 9
      outer->leaf_table_count > 0 &&                              // 10
      !((active_options() | outer->active_options()) &            // 11
        SELECT_STRAIGHT_JOIN) &&                                  // 11
      !(outer->active_options() & SELECT_NO_SEMI_JOIN) &&         // 12
      deterministic &&                                            // 13
      predicate->choose_semijoin_or_antijoin())                   // 14
  {
    DBUG_PRINT("info", ("Subquery is semi-join conversion candidate"));

    /* Notify in the subquery predicate where it belongs in the query graph */
    predicate->embedding_join_nest = outer->resolve_nest;

    /* Register the subquery for further processing in flatten_subqueries() */
    outer->sj_candidates->push_back(predicate);
    chose_semijoin = true;
  }

  if (predicate != nullptr) {
    Opt_trace_context *const trace = &thd->opt_trace;
    OPT_TRACE_TRANSFORM(
        trace, oto0, oto1, select_number,
        in_predicate != nullptr ? "IN (SELECT)" : "EXISTS (SELECT)",
        "semijoin");
    oto1.add("chosen", chose_semijoin);
  }

  if (!chose_semijoin && subq_predicate->select_transformer(thd, this) ==
                             Item_subselect::RES_ERROR)
    return true;

  return false;
}

/**
  Expand all '*' in list of expressions with the matching column references

  Function should not be called with no wild cards in select list

  @param  thd     thread handler

  @returns false if OK, true if error
*/

bool SELECT_LEX::setup_wild(THD *thd) {
  DBUG_TRACE;

  DBUG_ASSERT(with_wild);

  // PS/SP uses arena so that changes are made permanently.
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  Item *item;
  List_iterator<Item> it(fields_list);

  while (with_wild && (item = it++)) {
    Item_field *item_field;
    if (item->type() == Item::FIELD_ITEM && (item_field = (Item_field *)item) &&
        item_field->field_name && item_field->field_name[0] == '*' &&
        !item_field->field) {
      const uint elem = fields_list.elements;
      const bool any_privileges = item_field->any_privileges;
      Item_subselect *subsel = master_unit()->item;

      /*
        In case of EXISTS(SELECT * ... HAVING ...), don't use this
        transformation. The columns in HAVING will need to resolve to the
        select list. Replacing * with 1 effectively eliminates this
        possibility.
      */
      if (subsel && subsel->substype() == Item_subselect::EXISTS_SUBS &&
          !having_cond()) {
        /*
          It is EXISTS(SELECT * ...) and we can replace * by any constant.

          Item_int do not need fix_fields() because it is basic constant.
        */
        it.replace(new Item_int(NAME_STRING("Not_used"), (longlong)1,
                                MY_INT64_NUM_DECIMAL_DIGITS));
      } else {
        if (insert_fields(thd, item_field->context, item_field->db_name,
                          item_field->table_name, &it, any_privileges))
          return true;
      }
      /*
        all_fields is a list that has the fields list as a tail.
        Because of this we have to update the element count also for this
        list after expanding the '*' entry.
      */
      all_fields.elements += fields_list.elements - elem;

      with_wild--;
    }
  }

  return false;
}

/**
  Resolve WHERE condition and join conditions

  @param  thd     thread handler

  @returns false if success, true if error
*/

bool SELECT_LEX::setup_conds(THD *thd) {
  DBUG_TRACE;

  /*
    it_is_update set to true when tables of primary SELECT_LEX (SELECT_LEX
    which belong to LEX, i.e. most up SELECT) will be updated by
    INSERT/UPDATE/LOAD
    NOTE: using this condition helps to prevent call of prepare_check_option()
    from subquery of VIEW, because tables of subquery belongs to VIEW
    (see condition before prepare_check_option() call)
  */
  const bool it_is_update = (this == thd->lex->select_lex) &&
                            thd->lex->which_check_option_applicable();
  const bool save_is_item_list_lookup = is_item_list_lookup;
  is_item_list_lookup = false;

  DBUG_PRINT("info", ("thd->mark_used_columns: %d", thd->mark_used_columns));

  if (m_where_cond) {
    DBUG_ASSERT(m_where_cond->is_bool_func());
    resolve_place = SELECT_LEX::RESOLVE_CONDITION;
    thd->where = "where clause";
    if ((!m_where_cond->fixed &&
         m_where_cond->fix_fields(thd, &m_where_cond)) ||
        m_where_cond->check_cols(1))
      return true;

    // Simplify the where condition if it's a const item
    if (m_where_cond->const_item() && !thd->lex->is_view_context_analysis() &&
        !m_where_cond->walk(&Item::is_non_const_over_literals,
                            enum_walk::POSTFIX, nullptr) &&
        simplify_const_condition(thd, &m_where_cond))
      return true;

    resolve_place = SELECT_LEX::RESOLVE_NONE;
  }

  // Resolve all join condition clauses
  if (!top_join_list.empty() &&
      setup_join_cond(thd, &top_join_list, it_is_update))
    return true;

  is_item_list_lookup = save_is_item_list_lookup;

  DBUG_ASSERT(thd->lex->current_select() == this);
  DBUG_ASSERT(!thd->is_error());
  return false;
}

/**
  Resolve join conditions for a join nest

  @param thd    thread handler
  @param tables List of tables with join conditions
  @param in_update True if used in update command that may have CHECK OPTION

  @returns false if success, true if error
*/

bool SELECT_LEX::setup_join_cond(THD *thd, mem_root_deque<TABLE_LIST *> *tables,
                                 bool in_update) {
  DBUG_TRACE;

  for (TABLE_LIST *tr : *tables) {
    // Traverse join conditions recursively
    if (tr->nested_join != nullptr &&
        setup_join_cond(thd, &tr->nested_join->join_list, in_update))
      return true;

    Item **ref = tr->join_cond_ref();
    Item *join_cond = tr->join_cond();
    bool remove_cond = false;
    if (join_cond) {
      DBUG_ASSERT(join_cond->is_bool_func());
      resolve_place = SELECT_LEX::RESOLVE_JOIN_NEST;
      resolve_nest = tr;
      thd->where = "on clause";
      if ((!join_cond->fixed && join_cond->fix_fields(thd, ref)) ||
          join_cond->check_cols(1))
        return true;
      cond_count++;

      if ((*ref)->const_item() && !thd->lex->is_view_context_analysis() &&
          !(*ref)->walk(&Item::is_non_const_over_literals, enum_walk::POSTFIX,
                        nullptr) &&
          simplify_const_condition(thd, ref, remove_cond))
        return true;

      resolve_place = SELECT_LEX::RESOLVE_NONE;
      resolve_nest = nullptr;
    }
    if (in_update) {
      // Process CHECK OPTION
      TABLE_LIST *view = tr->top_table();
      if (view->is_view() && view->is_merged()) {
        if (view->prepare_check_option(thd))
          return true; /* purecov: inspected */
        tr->check_option = view->check_option;
      }
    }
  }

  return false;
}

/**
  Set NESTED_JOIN::counter=0 in all nested joins in passed list.

  @param join_list  Pass NULL. Non-NULL is reserved for recursive inner calls,
  then it is a list of nested joins to process, and may also contain base
  tables which will be ignored.
*/

void SELECT_LEX::reset_nj_counters(mem_root_deque<TABLE_LIST *> *join_list) {
  if (join_list == nullptr) join_list = &top_join_list;
  DBUG_TRACE;
  for (TABLE_LIST *table : *join_list) {
    NESTED_JOIN *nested_join;
    if ((nested_join = table->nested_join)) {
      nested_join->nj_counter = 0;
      reset_nj_counters(&nested_join->join_list);
    }
  }
}

/**
  Simplify joins replacing outer joins by inner joins whenever it's
  possible.

    The function, during a retrieval of join_list,  eliminates those
    outer joins that can be converted into inner join, possibly nested.
    It also moves the join conditions for the converted outer joins
    and from inner joins to conds.
    The function also calculates some attributes for nested joins:

    -# used_tables
    -# not_null_tables
    -# dep_tables.
    -# join_cond_dep_tables

    The first two attributes are used to test whether an outer join can
    be substituted by an inner join. The third attribute represents the
    relation 'to be dependent on' for tables. If table t2 is dependent
    on table t1, then in any evaluated execution plan table access to
    table t2 must precede access to table t2. This relation is used also
    to check whether the query contains  invalid cross-references.
    The fourth attribute is an auxiliary one and is used to calculate
    dep_tables.
    As the attribute dep_tables qualifies possibles orders of tables in the
    execution plan, the dependencies required by the straight join
    modifiers are reflected in this attribute as well.
    The function also removes all parentheses that can be removed from the join
    expression without changing its meaning.

  @note
    An outer join can be replaced by an inner join if the where condition
    or the join condition for an embedding nested join contains a conjunctive
    predicate rejecting null values for some attribute of the inner tables.

    E.g. in the query:
    @code
      SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a WHERE t2.b < 5
    @endcode
    the predicate t2.b < 5 rejects nulls.
    The query is converted first to:
    @code
      SELECT * FROM t1 INNER JOIN t2 ON t2.a=t1.a WHERE t2.b < 5
    @endcode
    then to the equivalent form:
    @code
      SELECT * FROM t1, t2 ON t2.a=t1.a WHERE t2.b < 5 AND t2.a=t1.a
    @endcode


    Similarly the following query:
    @code
      SELECT * from t1 LEFT JOIN (t2, t3) ON t2.a=t1.a t3.b=t1.b
        WHERE t2.c < 5
    @endcode
    is converted to:
    @code
      SELECT * FROM t1, (t2, t3) WHERE t2.c < 5 AND t2.a=t1.a t3.b=t1.b

    @endcode

    One conversion might trigger another:
    @code
      SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a
                       LEFT JOIN t3 ON t3.b=t2.b
        WHERE t3 IS NOT NULL =>
      SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t1.a, t3
        WHERE t3 IS NOT NULL AND t3.b=t2.b =>
      SELECT * FROM t1, t2, t3
        WHERE t3 IS NOT NULL AND t3.b=t2.b AND t2.a=t1.a
  @endcode

    The function removes all unnecessary parentheses from the expression
    produced by the conversions.
    E.g.
    @code
      SELECT * FROM t1, (t2, t3) WHERE t2.c < 5 AND t2.a=t1.a AND t3.b=t1.b
    @endcode
    finally is converted to:
    @code
      SELECT * FROM t1, t2, t3 WHERE t2.c < 5 AND t2.a=t1.a AND t3.b=t1.b

    @endcode


    It also will remove parentheses from the following queries:
    @code
      SELECT * from (t1 LEFT JOIN t2 ON t2.a=t1.a) LEFT JOIN t3 ON t3.b=t2.b
      SELECT * from (t1, (t2,t3)) WHERE t1.a=t2.a AND t2.b=t3.b.
    @endcode

    The benefit of this simplification procedure is that it might return
    a query for which the optimizer can evaluate execution plans with more
    join orders. With a left join operation the optimizer does not
    consider any plan where one of the inner tables is before some of outer
    tables.

  IMPLEMENTATION
    The function is implemented by a recursive procedure.  On the recursive
    ascent all attributes are calculated, all outer joins that can be
    converted are replaced and then all unnecessary parentheses are removed.
    As join list contains join tables in the reverse order sequential
    elimination of outer joins does not require extra recursive calls.

  SEMI-JOIN NOTES
    Remove all semi-joins that have are within another semi-join (i.e. have
    an "ancestor" semi-join nest)

  EXAMPLES
    Here is an example of a join query with invalid cross references:
    @code
      SELECT * FROM t1 LEFT JOIN t2 ON t2.a=t3.a LEFT JOIN t3 ON t3.b=t1.b
    @endcode

  @param thd         thread handler
  @param join_list   list representation of the join to be converted
  @param top         true <=> cond is the where condition
  @param in_sj       true <=> processing semi-join nest's children
  @param[in,out] cond In: condition to which the join condition for converted
                          outer joins is to be added;
                      Out: new condition
  @param changelog   Don't specify this parameter, it is reserved for
                     recursive calls inside this function

  @returns true for error, false for success
*/
bool SELECT_LEX::simplify_joins(THD *thd,
                                mem_root_deque<TABLE_LIST *> *join_list,
                                bool top, bool in_sj, Item **cond,
                                uint *changelog) {
  /*
    Each type of change done by this function, or its recursive calls, is
    tracked in a bitmap:
  */
  enum change {
    NONE = 0,
    OUTER_JOIN_TO_INNER = 1 << 0,
    JOIN_COND_TO_WHERE = 1 << 1,
    PAREN_REMOVAL = 1 << 2,
    SEMIJOIN = 1 << 3
  };
  uint changes = 0;          // To keep track of changes.
  if (changelog == nullptr)  // This is the top call.
    changelog = &changes;

  NESTED_JOIN *nested_join;
  TABLE_LIST *prev_table = nullptr;
  const bool straight_join = active_options() & SELECT_STRAIGHT_JOIN;
  DBUG_TRACE;

  /*
    Try to simplify join operations from join_list.
    The most outer join operation is checked for conversion first.
    join_list is a join nest, and 'cond' is a condition which acts as a filter
    applied to the nest's operation (post-filter).
    Thus, considering this example:
    (A LEFT JOIN B ON JC) WHERE W ,
    we'll "confront W with A LEFT JOIN B": this will, recursively,
    - confront W with B,
    - confront W with A.
    Because W is external to the nest, if W would be false when B is
    NULL-complemented we know we can change LEFT JOIN to JOIN.
    We will not confront JC with B or A, it wouldn't make sense, as JC isn't a
    post-filter for their join operation.
    Another example:
    (A LEFT JOIN (B LEFT JOIN C ON JC2) ON JC1) WHERE W ,
    while confronting W with (B LEFT JOIN C), we will also, as first step,
    confront JC1 with (B LEFT JOIN C), and thus recursively confront JC1
    with C and then with B.
    Another example:
    (A LEFT JOIN (B SEMI JOIN C ON JC2) ON JC1) WHERE W ,
    while confronting W with (B SEMI JOIN C), if W is known false we will

  */
  for (TABLE_LIST *table : *join_list) {
    table_map used_tables;
    table_map not_null_tables = (table_map)0;

    if ((nested_join = table->nested_join)) {
      /*
         If the element of join_list is a nested join apply
         the procedure to its nested join list first.
         This confronts the join nest's condition with each member of the
         nest.
      */
      if (table->join_cond()) {
        Item *join_cond = table->join_cond();
        /*
           If a join condition JC is attached to the table,
           check all null rejected predicates in this condition.
           If such a predicate over an attribute belonging to
           an inner table of an embedded outer join is found,
           the outer join is converted to an inner join and
           the corresponding join condition is added to JC.
        */
        if (simplify_joins(
                thd, &nested_join->join_list,
                false,  // not 'top' as it's not WHERE.
                // SJ nests can dissolve into upper SJ or anti SJ nests:
                in_sj || table->is_sj_or_aj_nest(), &join_cond, changelog))
          return true;

        if (join_cond != table->join_cond()) {
          DBUG_ASSERT(join_cond);
          table->set_join_cond(join_cond);
        }
      }
      nested_join->used_tables = (table_map)0;
      nested_join->not_null_tables = (table_map)0;
      // This recursively confronts "cond" with each member of the nest
      if (simplify_joins(thd, &nested_join->join_list,
                         top,  // if it was WHERE it still is
                         in_sj || table->is_sj_or_aj_nest(), cond, changelog))
        return true;
      used_tables = nested_join->used_tables;
      not_null_tables = nested_join->not_null_tables;
    } else {
      used_tables = table->map();
      if (*cond) not_null_tables = (*cond)->not_null_tables();
    }

    if (table->embedding) {
      table->embedding->nested_join->used_tables |= used_tables;
      table->embedding->nested_join->not_null_tables |= not_null_tables;
    }

    if (!table->outer_join || (used_tables & not_null_tables)) {
      /*
        For some of the inner tables there are conjunctive predicates
        that reject nulls => the outer join can be replaced by an inner join.
      */
      if (table->outer_join) {
        *changelog |= OUTER_JOIN_TO_INNER;
        table->outer_join = JOIN_TYPE_INNER;
      }
      if (table->join_cond()) {
        *changelog |= JOIN_COND_TO_WHERE;
        /* Add join condition to the WHERE or upper-level join condition. */
        if (*cond) {
          Item *i1 = *cond, *i2 = table->join_cond();
          /*
            User supplied stored procedures in the query can violate row-level
            filter enforced by a view. So make sure view's filter conditions
            precede any other conditions.
          */
          if (table->is_view() && i1->has_stored_program()) {
            std::swap(i1, i2);
          }

          Item_cond_and *new_cond =
              down_cast<Item_cond_and *>(and_conds(i1, i2));
          if (!new_cond) return true;
          new_cond->apply_is_true();
          /*
            It is always a new item as both the upper-level condition and a
            join condition existed
          */
          DBUG_ASSERT(!new_cond->fixed);
          Item *cond_after_fix = new_cond;
          if (new_cond->fix_fields(thd, &cond_after_fix)) return true;

          if (new_cond == cond_after_fix) {
            /* If join condition has a pending rollback in THD::change_list */
            List_iterator<Item> lit(*new_cond->argument_list());
            Item *arg;
            while ((arg = lit++)) {
              /*
                Check whether the arguments to AND need substitution
                of rollback location.
              */
              thd->replace_rollback_place(lit.ref());
            }
          }
          *cond = cond_after_fix;
        } else {
          *cond = table->join_cond();
          /* If join condition has a pending rollback in THD::change_list */
          thd->replace_rollback_place(cond);
        }
        table->set_join_cond(nullptr);
      }
    }

    // A table is traversed when 'cond' is WHERE, and when 'cond' is the join
    // condition of any nest containing the table. Some bitmaps can be set
    // only after all traversals of this table i.e. when 'cond' is WHERE.
    if (!top) continue;

    /*
      Only inner tables of non-convertible outer joins remain with
      the join condition.
    */
    if (table->join_cond()) {
      table->dep_tables |= table->join_cond()->used_tables();
      // At this point the joined tables always have an embedding join nest:
      DBUG_ASSERT(table->embedding);
      table->dep_tables &= ~table->embedding->nested_join->used_tables;

      // Embedding table depends on tables used in embedded join conditions.
      table->embedding->join_cond_dep_tables |=
          table->join_cond()->used_tables();
    }

    if (prev_table) {
      /* The order of tables is reverse: prev_table follows table */
      if (prev_table->straight || straight_join)
        prev_table->dep_tables |= used_tables;
      if (prev_table->join_cond()) {
        prev_table->dep_tables |= table->join_cond_dep_tables;
        table_map prev_used_tables = prev_table->nested_join
                                         ? prev_table->nested_join->used_tables
                                         : prev_table->map();
        /*
          If join condition contains no reference to outer tables
          we still make the inner tables dependent on the outer tables,
          as the outer must go before the inner since the executor requires
          that at least one outer table is before the inner tables.
          It would be enough to set dependency only on one outer table
          for them. Yet this is really a rare case.
          Note:
          PSEUDO_TABLE_BITS mask should not be counted as it
          prevents update of inner table dependencies.
          For example it might happen if RAND()/COUNT(*) function
          is used in JOIN ON clause.
        */
        if ((((prev_table->join_cond()->used_tables() & ~PSEUDO_TABLE_BITS) &
              ~prev_used_tables) &
             used_tables) == 0) {
          prev_table->dep_tables |= used_tables;
        }
      }
    }
    prev_table = table;
  }

  /*
    Flatten nested joins that can be flattened.
    no join condition and not a semi-join => can be flattened.
  */
  for (auto li = join_list->begin(); li != join_list->end();) {
    TABLE_LIST *table = *li;
    nested_join = table->nested_join;
    if (table->is_sj_or_aj_nest()) {
      // See other uses of clear_sj_expressions().
      // 'cond' is a post-filter after the semi/antijoin, so if it's
      // always false the semi/antijoin can be partially simplified
      // (note that the semi/antijoin nest will still be created and used in
      // optimization and execution).
      if (*cond && (*cond)->const_item() &&
          !(*cond)->walk(&Item::is_non_const_over_literals, enum_walk::POSTFIX,
                         nullptr)) {
        bool cond_value = true;
        if (simplify_const_condition(thd, cond, false, &cond_value))
          return true;
        if (!cond_value) clear_sj_expressions(nested_join);
      }
    }
    if (table->is_sj_nest() && !in_sj) {
      /*
        If this is a semi-join that is not contained within another semi-join,
        leave it intact.
        Otherwise it is flattened, for example
        A SJ (B SJ (C)) becomes the equivalent A SJ (B JOIN C),
        A AJ (B SJ (C)) becomes the equivalent A AJ (B JOIN C),
        While dissolving a SJ nest into an AJ nest is ok (for the AJ
        this may lead to duplicates but AJ only cares for "at least
        one match"), dissolving an AJ nest into a SJ is not ok:
        A SJ (B AJ (C)) is not equivalent to A SJ (B JOIN C);
        that is why the next if() block is guarded by !join_cond() which takes
        care of that.
        Note that when dissolving the SJ nest, its condition isn't lost as it
        has previously been added to WHERE or outer nest's condition in
        convert_subquery_to_semijoin().
      */
      *changelog |= SEMIJOIN;
    } else if (nested_join && !table->join_cond()) {
      *changelog |= PAREN_REMOVAL;
      for (TABLE_LIST *tbl : nested_join->join_list) {
        tbl->embedding = table->embedding;
        tbl->join_list = table->join_list;
        tbl->dep_tables |= table->dep_tables;
      }
      li = join_list->erase(li);
      li = join_list->insert(li, nested_join->join_list.begin(),
                             nested_join->join_list.end());

      // Don't advance li; we want to process the newly added tables.
      continue;
    }
    ++li;
  }

  if (changes) {
    Opt_trace_context *trace = &thd->opt_trace;
    if (unlikely(trace->is_started())) {
      Opt_trace_object trace_wrapper(trace);
      Opt_trace_object trace_object(trace, "transformations_to_nested_joins");
      {
        Opt_trace_array trace_changes(trace, "transformations");
        if (changes & SEMIJOIN) trace_changes.add_alnum("semijoin");
        if (changes & OUTER_JOIN_TO_INNER)
          trace_changes.add_alnum("outer_join_to_inner_join");
        if (changes & JOIN_COND_TO_WHERE)
          trace_changes.add_alnum("JOIN_condition_to_WHERE");
        if (changes & PAREN_REMOVAL)
          trace_changes.add_alnum("parenthesis_removal");
      }
      // the newly transformed query is worth printing
      opt_trace_print_expanded_query(thd, this, &trace_object);
    }
  }
  return false;
}

/**
  Record join nest info in the select block.

  After simplification of inner join, outer join and semi-join structures:
   - record the remaining semi-join structures in the enclosing query block.
   - record transformed join conditions in TABLE_LIST objects.

  This function is called recursively for each join nest and/or table
  in the query block.

  @param tables List of tables and join nests

  @return False if successful, True if failure
*/
bool SELECT_LEX::record_join_nest_info(mem_root_deque<TABLE_LIST *> *tables) {
  for (TABLE_LIST *table : *tables) {
    if (table->nested_join == nullptr) {
      if (table->join_cond()) outer_join |= table->map();
      continue;
    }

    if (record_join_nest_info(&table->nested_join->join_list)) return true;
    /*
      sj_inner_tables is set properly later in pull_out_semijoin_tables().
      This assignment is required in case pull_out_semijoin_tables()
      is not called.
    */
    if (table->is_sj_or_aj_nest())
      table->sj_inner_tables = table->nested_join->used_tables;

    if (table->is_sj_or_aj_nest()) {
      sj_nests.push_back(table);
    }

    if (table->join_cond()) outer_join |= table->nested_join->used_tables;
  }
  return false;
}

/**
  Update table reference information for conditions and expressions due to
  query blocks having been merged in from derived tables/views and due to
  semi-join transformation.

  This is needed for two reasons:

  1. Since table numbers are changed, we need to update used_tables
     information for all conditions and expressions that are possibly touched.

  2. For semi-join, some column references are changed from outer references
     to local references.

  The function needs to recursively walk down into join nests,
  in order to cover all conditions and expressions.

  For a semi-join, tables from the subquery are added last in the query block.
  This means that conditions and expressions from the outer query block
  are unaffected. But all conditions inside the semi-join nest, including
  join conditions, must have their table numbers changed.

  For a derived table/view, tables from the subquery are merged into the
  outer query, and this function is called for every derived table that is
  merged in. This algorithm only works when derived tables are merged in
  the order of their original table numbers.

  A hypothetical example with a triple self-join over a mergeable view:

    CREATE VIEW v AS SELECT t1.a, t2.b FROM t1 JOIN t2 USING (a);
    SELECT v1.a, v1.b, v2.b, v3.b
    FROM v AS v1 JOIN v AS v2 ON ... JOIN v AS v3 ON ...;

  The analysis starts with three tables v1, v2 and v3 having numbers 0, 1, 2.
  First we merge in v1, so we get (t1, t2, v2, v3). v2 and v3 are shifted up.
  Tables from v1 need to have their table numbers altered (actually they do not
  since both old and new numbers are 0 and 1, but this is a special case).
  v2 and v3 are not merged in yet, so we delay pullout on them until they
  are merged. Conditions and expressions from the outer query are not resolved
  yet, so regular resolving will take of them later.
  Then we merge in v2, so we get (t1, t2, t1, t2, v3). The tables from this
  view gets numbers 2 and 3, and v3 gets number 4.
  Because v2 had a higher number than the tables from v1, the join nest
  representing v1 is unaffected. And v3 is still not merged, so the only
  join nest we need to consider is v2.
  Finally we merge in v3, and then we have tables (t1, t2, t1, t2, t1, t2),
  with numbers 0 through 5.
  Again, since v3 has higher number than any of the already merged in views,
  only this join nest needs the pullout.

  @param parent_select  Query block being merged into
  @param removed_select Query block that is removed (subquery)
  @param tr             Table object this pullout is applied to
  @param table_adjust   Number of positions that a derived table nest is
                        adjusted, used to fix up semi-join related fields.
                        Tables are adjusted from position N to N+table_adjust
*/

static void fix_tables_after_pullout(SELECT_LEX *parent_select,
                                     SELECT_LEX *removed_select, TABLE_LIST *tr,
                                     uint table_adjust) {
  if (tr->is_merged()) {
    // Update select list of merged derived tables:
    for (Field_translator *transl = tr->field_translation;
         transl < tr->field_translation_end; transl++) {
      DBUG_ASSERT(transl->item->fixed);
      transl->item->fix_after_pullout(parent_select, removed_select);
    }
    // Update used table info for the WHERE clause of the derived table
    DBUG_ASSERT(!tr->derived_where_cond || tr->derived_where_cond->fixed);
    if (tr->derived_where_cond)
      tr->derived_where_cond->fix_after_pullout(parent_select, removed_select);
  }

  /*
    If join_cond() is fixed, it contains a join condition from a subquery
    that has already been resolved. Call fix_after_pullout() to update
    used table information since table numbers may have changed.
    If join_cond() is not fixed, it contains a condition that was generated
    in the derived table merge operation, which will be fixed later.
    This condition may also contain a fixed part, but this is saved as
    derived_where_cond and is pulled out explicitly.
  */
  if (tr->join_cond() && tr->join_cond()->fixed)
    tr->join_cond()->fix_after_pullout(parent_select, removed_select);

  if (tr->nested_join) {
    // In case a derived table is merged-in, these fields need adjustment:
    tr->nested_join->sj_corr_tables <<= table_adjust;
    tr->nested_join->sj_depends_on <<= table_adjust;

    for (TABLE_LIST *child : tr->nested_join->join_list) {
      fix_tables_after_pullout(parent_select, removed_select, child,
                               table_adjust);
    }
  }
  if (tr->is_derived() && tr->table &&
      tr->derived_unit()->uncacheable & UNCACHEABLE_DEPENDENT) {
    /*
      It's a materialized derived table which is being pulled up.
      If it has an outer reference, and this ref belongs to parent_select,
      then the derived table will need re-materialization as if it were
      LATERAL, not just once per execution of parent_select.
      We thus compute its used_tables in the new context, to decide.
    */
    SELECT_LEX_UNIT *unit = tr->derived_unit();
    unit->m_lateral_deps = OUTER_REF_TABLE_BIT;
    unit->fix_after_pullout(parent_select, removed_select);
    unit->m_lateral_deps &= ~PSEUDO_TABLE_BITS;
    tr->dep_tables |= unit->m_lateral_deps;
    /*
      If m_lateral_deps!=0, some outer ref is now a neighbour in FROM: we have
      made 'tr' LATERAL.
      @todo after WL#6570 when we don't re-resolve, remove this comment.
      Note that this above gives 'tr' enough "right to look left", but alas
      also too much of it; e.g.
      select * from t1, lateral (select * from dt1, dt2) dt3
      becomes
      select * from t1, lateral dt1, lateral dt2 :
      dt2 needs the right to look into t1 and gets it, but also dt2 gets the
      right to look into dt1, which is too much. But this is only a problem in
      execution of PS (which does name resolution on the merged query), and
      cached_table saves the day (tested in derived_correlated.test, search
      for "prepared stmt"). So there's no problem.
    */
  }
}

/**
  Fix used tables information for a subquery after query transformations.
  This is for transformations where the subquery remains a subquery - it is
  not merged, it merely moves up by effect of a transformation on a containing
  query block.
  Most actions here involve re-resolving information for conditions
  and items belonging to the subquery.
  If the subquery contains an outer reference into removed_select or
  parent_select, the relevant information is updated by
  Item_ident::fix_after_pullout().
*/
void SELECT_LEX_UNIT::fix_after_pullout(SELECT_LEX *parent_select,
                                        SELECT_LEX *removed_select)

{
  // Go through all query specification objects of the subquery and re-resolve
  // all relevant expressions belonging to them.
  for (SELECT_LEX *sel = first_select(); sel; sel = sel->next_select()) {
    sel->fix_after_pullout(parent_select, removed_select);
  }
  // @todo figure out if we need to do it for fake_select_lex too.
}

/// @see SELECT_LEX_UNIT::fix_after_pullout
void SELECT_LEX::fix_after_pullout(SELECT_LEX *parent_select,
                                   SELECT_LEX *removed_select) {
  if (where_cond())
    where_cond()->fix_after_pullout(parent_select, removed_select);

  /*
    Join conditions can contain an outer reference; and
    derived table merging changes WHERE to a join condition, which thus can
    have an outer reference. So we have to call fix_after_pullout() on join
    conditions. The reference may also be located in a derived table used by
    this subquery. fix_tables_after_pullout() will handle the two cases.
    table_adjust is 0 because we're not merging these tables up.
  */
  for (TABLE_LIST *tr : top_join_list) {
    fix_tables_after_pullout(parent_select, removed_select, tr,
                             /*table_adjust=*/0);
  }

  if (having_cond())
    having_cond()->fix_after_pullout(parent_select, removed_select);

  List_iterator<Item> li(item_list);
  Item *item;
  while ((item = li++)) item->fix_after_pullout(parent_select, removed_select);

  /* Re-resolve ORDER BY and GROUP BY fields */

  for (ORDER *order = order_list.first; order; order = order->next)
    (*order->item)->fix_after_pullout(parent_select, removed_select);

  for (ORDER *group = group_list.first; group; group = group->next)
    (*group->item)->fix_after_pullout(parent_select, removed_select);
}

/**
 Remove SJ outer/inner expressions.

 @param nested_join         join nest
*/

void SELECT_LEX::clear_sj_expressions(NESTED_JOIN *nested_join) {
  nested_join->sj_outer_exprs.empty();
  nested_join->sj_inner_exprs.empty();
  DBUG_ASSERT(sj_nests.empty());
}

static bool add_possible_transformation_rollbacks(THD *thd,
                                                  Item_func_eq *item_eq);

/**
  Build equality conditions using outer expressions and inner
  expressions. If the equality condition is not constant, add
  it to the semi-join condition. Otherwise, evaluate it and
  remove the constant expressions from the
  outer/inner expressions list if the result is true. If the
  result is false, remove all the expressions in outer/inner
  expression list and attach an always false condition
  to semijoin condition.

  @param thd            Thread context
  @param nested_join    Join nest
  @param subq_select    Query block for the subquery
  @param outer_tables_map Map of tables from original outer query block
  @param[out] sj_cond   Semi-join condition to be constructed

  @return false if success, true if error
*/
bool SELECT_LEX::build_sj_cond(THD *thd, NESTED_JOIN *nested_join,
                               SELECT_LEX *subq_select,
                               table_map outer_tables_map, Item **sj_cond) {
  if (nested_join->sj_inner_exprs.elements == 0) {
    // Semi-join materialization requires a key, push a constant integer item
    Item *const_item = new Item_int(1);
    if (const_item == nullptr) return true;
    if (nested_join->sj_inner_exprs.push_back(const_item)) return true;
    if (nested_join->sj_outer_exprs.push_back(const_item)) return true;
  }
  List_iterator<Item> ii(nested_join->sj_inner_exprs);
  List_iterator<Item> oi(nested_join->sj_outer_exprs);
  Item *inner, *outer;
  while (outer = oi++, inner = ii++) {
    /*
      Ensure that all involved expressions are pulled out after transformation.
      (If they are already out, this is a no-op).
    */
    outer->fix_after_pullout(this, subq_select);
    inner->fix_after_pullout(this, subq_select);

    Item_func_eq *item_eq = new Item_func_eq(outer, inner);
    if (item_eq == nullptr) return true; /* purecov: inspected */
    if (add_possible_transformation_rollbacks(thd, item_eq)) return true;
    Item *predicate = item_eq;
    if (!item_eq->fixed && item_eq->fix_fields(thd, &predicate)) return true;

    // Evaluate if the condition is on const expressions
    if (predicate->const_item() &&
        !(predicate)->walk(&Item::is_non_const_over_literals,
                           enum_walk::POSTFIX, nullptr)) {
      bool cond_value = true;

      /* Push ignore / strict error handler */
      Ignore_error_handler ignore_handler;
      Strict_error_handler strict_handler;
      if (thd->lex->is_ignore())
        thd->push_internal_handler(&ignore_handler);
      else if (thd->install_strict_handler())
        thd->push_internal_handler(&strict_handler);

      bool err = eval_const_cond(thd, predicate, &cond_value);
      /* Pop ignore / strict error handler */
      if (thd->lex->is_ignore() || thd->install_strict_handler())
        thd->pop_internal_handler();

      if (err) return true;

      if (cond_value) {
        /*
          Remove the expression from inner/outer expression list if the
          const condition evalutes to true as Item_cond::fix_fields will
          remove the condition later.
          Do the above if this is not the last expression in the list.
          Semijoin processing expects atleast one inner/outer expression
          in the list if there is a sj_nest present.
        */
        if (!(nested_join->sj_inner_exprs.elements == 1)) {
          oi.remove();
          ii.remove();
        }
      } else {
        /*
          Remove all the expressions in inner/outer expression list if
          one of condition evaluates to always false. Add an always false
          condition to semi-join condition.
        */
        nested_join->sj_inner_exprs.empty();
        nested_join->sj_outer_exprs.empty();
        Item *new_item = new Item_func_false();
        if (new_item == nullptr) return true;
        (*sj_cond) = new_item;
        break;
      }
    }
    /*
      li [left_expr->element_index(i)] can be a transient Item_outer_ref,
      whose usage has already been marked for rollback, but we need to roll
      back this location (inside Item_func_eq) in stead, since this is the
      place that matters after this semijoin transformation. arguments()
      gets the address of li as stored in item_eq ("place").
      */
    thd->replace_rollback_place(item_eq->arguments());
    (*sj_cond) = and_items(*sj_cond, predicate);
    if (*sj_cond == nullptr) return true; /* purecov: inspected */
    /*
      If the selected expression has an outer reference, add it as a
      non-trivially correlated reference (to avoid materialization).
    */
    nested_join->sj_corr_tables |= inner->used_tables() & outer_tables_map;
  }
  return false;
}

/**
  Try to decorrelate an equality node. The node can be decorrelated if one
  argument contains only outer references and the other argument contains
  references only to local tables.
  Both arguments should be deterministic.
  cons-for-execution values are accepted in both arguments.

  @param sj_nest The semi-join nest that the decorrelated expressions are
                 added to.
  @param func    The query function node
  @param[out] was_correlated = true if comparison is correlated and the
                 the expressions are added to sj_nest.

  @returns false if success, true if error
*/

static bool decorrelate_equality(TABLE_LIST *sj_nest, Item_func *func,
                                 bool *was_correlated) {
  DBUG_ASSERT(func->functype() == Item_func::EQ_FUNC);
  *was_correlated = false;
  Item *const left = func->arguments()[0];
  Item *const right = func->arguments()[1];
  Item *inner = nullptr;
  Item *outer = nullptr;
  table_map left_used_tables = left->used_tables() & ~INNER_TABLE_BIT;
  table_map right_used_tables = right->used_tables() & ~INNER_TABLE_BIT;

  /*
    Predicates that have non-deterministic elements are not decorrelated,
    see explanation for SELECT_LEX::decorrelate_condition().
  */
  if ((left_used_tables & RAND_TABLE_BIT) ||
      (right_used_tables & RAND_TABLE_BIT))
    return false;

  if (left_used_tables == OUTER_REF_TABLE_BIT) {
    outer = left;
  } else if (left_used_tables && !(left_used_tables & OUTER_REF_TABLE_BIT)) {
    inner = left;
  }
  if (right_used_tables == OUTER_REF_TABLE_BIT) {
    outer = right;
  } else if (right_used_tables && !(right_used_tables & OUTER_REF_TABLE_BIT)) {
    inner = right;
  }
  if (inner == nullptr || outer == nullptr) return false;

  // Equalities over row items cannot be decorrelated
  if (outer->type() == Item::ROW_ITEM) return false;

  if (sj_nest->nested_join->sj_outer_exprs.push_back(outer)) return true;
  if (sj_nest->nested_join->sj_inner_exprs.push_back(inner)) return true;

  *was_correlated = true;

  return false;
}

/**
  Decorrelate the WHERE clause or a join condition of a subquery used in
  an IN or EXISTS predicate.
  Correlated predicates are removed from the condition and added to the
  supplied semi-join nest.
  The predicate must be either a simple equality, or an AND condition that
  contains one or more simple equalities, in order for decorrelation to be
  possible.

  @param sj_nest   The semi-join nest of the outer query block, the correlated
                   expressions are added to sj_inner_exprs and sj_outer_exprs.
  @param join_nest Nest containing join condition to be decorrelated
                   =NULL: decorrelate the WHERE condition

  @returns false if success, true if error

  Decorrelation for subqueries containing non-deterministic components:
  --------------------------------------------------------------------

  There are two types of IN and EXISTS queries with non-deterministic
  functions that may be meaningful (the EXISTS queries below are correlated
  equivalents of the respective IN queries):

  1. Non-deterministic function as substitute for expression from outer
     query block:

  A SELECT * FROM t1
    WHERE RAND() IN (SELECT t2.x FROM t2)

  B SELECT * FROM t1
    WHERE EXISTS (SELECT * FROM t2 WHERE RAND() = t2.x);

  Pick a set of random rows that matches against a fixed set (the subquery).

  The intuitive interpretation of the IN subquery is that the random function
  is evaluated per row of the outer query block, whereas in the EXISTS subquery,
  it should be evaluated per row of the inner query block, and the subquery
  is evaluated once per row of the outer query block.

  2. Non-deterministic function as substitute for expression from inner
     query block:

  A SELECT * FROM t1
    WHERE t1.x IN (SELECT RAND() FROM t2)

  B SELECT * FROM t1
    WHERE EXISTS (SELECT * FROM t2 WHERE RAND() = t1.x);

  This is another way of picking a random row, but now the non-determinism
  occurs in the inner query block.

  The user will expect that only query 1A has the evaluation of
  non-deterministic functions being performed in the outer query block.
  Using decorrelation for query 1B would change the apparent semantics of
  the query.

  The purpose of decorrelation is to be able to use more execution strategies.
  Without decorrelation, EXISTS is limited to FirstMatch and DupsWeedout
  strategies. Decorrelation enables LooseScan and Materialization.
  We can rule out LooseScan for case 2B, since it requires an indexed column
  from the subquery, and for case 1B, since it requires that the outer table
  is partitioned according to the distinct values of the index, and random
  values do not fulfill that partitioning requirement.

  The only strategy left is Materialization. With decorrelation, 1B would be
  evaluated like 1A, which is not the intuitive way. 2B would also be
  implemented like 2A, meaning that evaluation of non-deterministic functions
  would move to the materialization function.

  Thus, the intuitive interpretation is to avoid materialization for subqueries
  with non-deterministic components in the inner query block, and hence
  such predicates will not be decorrelated.
*/

bool SELECT_LEX::decorrelate_condition(TABLE_LIST *const sj_nest,
                                       TABLE_LIST *join_nest) {
  Item *base_cond =
      join_nest == nullptr ? where_cond() : join_nest->join_cond();
  Item_cond *cond;
  Item_func *func;

  DBUG_ASSERT(base_cond != nullptr);

  if (base_cond->type() == Item::FUNC_ITEM &&
      (func = down_cast<Item_func *>(base_cond)) &&
      func->functype() == Item_func::EQ_FUNC) {
    bool was_correlated;
    if (decorrelate_equality(sj_nest, func, &was_correlated)) return true;
    if (was_correlated) {  // The simple equality has been decorrelated
      if (join_nest == nullptr)
        set_where_cond(nullptr);
      else  // Join conditions cannot be empty so install a TRUE value
        join_nest->set_join_cond(new Item_func_true());
    }
  } else if (base_cond->type() == Item::COND_ITEM &&
             (cond = down_cast<Item_cond *>(base_cond)) &&
             cond->functype() == Item_func::COND_AND_FUNC) {
    List<Item> *args = cond->argument_list();
    List_iterator<Item> li(*args);
    Item *item;
    while ((item = li++)) {
      if (item->type() == Item::FUNC_ITEM &&
          (func = down_cast<Item_func *>(item)) &&
          func->functype() == Item_func::EQ_FUNC) {
        bool was_correlated;
        if (decorrelate_equality(sj_nest, func, &was_correlated)) return true;
        if (was_correlated) li.remove();
      }
    }
    if (args->is_empty()) {  // All predicates have been decorrelated
      if (join_nest == nullptr)
        set_where_cond(nullptr);
      else  // Join conditions cannot be empty so install a TRUE value
        join_nest->set_join_cond(new Item_func_true());
    }
  }
  return false;
}

bool walk_join_list(mem_root_deque<TABLE_LIST *> &list,
                    std::function<bool(TABLE_LIST *)> action) {
  for (TABLE_LIST *tl : list) {
    if (action(tl)) return true;
    if (tl->nested_join != nullptr &&
        walk_join_list(tl->nested_join->join_list, action))
      return true;
  }
  return false;
}

static bool maybe_mark_item_for_rollback(THD *thd, Item **i);

/**
  Convert a subquery predicate of this query block into a TABLE_LIST semi-join
  nest.

  @param thd         Thread handle
  @param subq_pred   Subquery predicate to be converted.
                     This is either an IN, =ANY or EXISTS predicate, possibly
                     negated.

  @returns false if success, true if error

  The following transformations are performed:

  1. IN/=ANY predicates on the form:

  @code
  SELECT ...
  FROM ot1 ... otN
  WHERE (oe1, ... oeM) IN (SELECT ie1, ..., ieM
                           FROM it1 ... itK
                          [WHERE inner-cond])
   [AND outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  are transformed into:

  @code
  SELECT ...
  FROM (ot1 ... otN) SJ (it1 ... itK)
                     ON (oe1, ... oeM) = (ie1, ..., ieM)
                        [AND inner-cond]
  [WHERE outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  Notice that the inner-cond may contain correlated and non-correlated
  expressions. Further transformations will analyze and break up such
  expressions.

  2. EXISTS predicates on the form:

  @code
  SELECT ...
  FROM ot1 ... otN
  WHERE EXISTS (SELECT expressions
                FROM it1 ... itK
                [WHERE inner-cond])
   [AND outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  are transformed into:

  @code
  SELECT ...
  FROM (ot1 ... otN) SJ (it1 ... itK)
                     [ON inner-cond]
  [WHERE outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  3. Negated EXISTS predicates on the form:

  @code
  SELECT ...
  FROM ot1 ... otN
  WHERE NOT EXISTS (SELECT expressions
                FROM it1 ... itK
                [WHERE inner-cond])
   [AND outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  are transformed into:

  @code
  SELECT ...
  FROM (ot1 ... otN) AJ (it1 ... itK)
                     [ON inner-cond]
  [WHERE outer-cond AND is-null-cond(it1)]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  where AJ means "antijoin" and is like a LEFT JOIN; and is-null-cond is
  false if the row of it1 is "found" and "not_null_compl" (i.e. matches
  inner-cond).

  4. Negated IN predicates on the form:

  @code
  SELECT ...
  FROM ot1 ... otN
  WHERE (oe1, ... oeM) NOT IN (SELECT ie1, ..., ieM
                               FROM it1 ... itK
                               [WHERE inner-cond])
   [AND outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  are transformed into:

  @code
  SELECT ...
  FROM (ot1 ... otN) AJ (it1 ... itK)
                     ON (oe1, ... oeM) = (ie1, ..., ieM)
                        [AND inner-cond]
  [WHERE outer-cond]
  [GROUP BY ...] [HAVING ...] [ORDER BY ...]
  @endcode

  5. The cases 1/2 (respectively 3/4) above also apply when the predicate is
  decorated with IS TRUE or IS NOT FALSE (respectively IS NOT TRUE or IS
  FALSE).
*/
bool SELECT_LEX::convert_subquery_to_semijoin(
    THD *thd, Item_exists_subselect *subq_pred) {
  TABLE_LIST *emb_tbl_nest = nullptr;
  mem_root_deque<TABLE_LIST *> *emb_join_list = &top_join_list;
  DBUG_TRACE;

  DBUG_ASSERT(subq_pred->substype() == Item_subselect::IN_SUBS ||
              subq_pred->substype() == Item_subselect::EXISTS_SUBS);

  Opt_trace_context *trace = &thd->opt_trace;
  Opt_trace_object trace_object(trace, "transformation_to_semi_join");
  if (unlikely(trace->is_started())) {
    trace_object.add("subquery_predicate", subq_pred);
  }

  bool outer_join = false;  // True if predicate is inner to an outer join

  // Save the set of tables in the outer query block:
  table_map outer_tables_map = all_tables_map();
  const bool do_aj = subq_pred->can_do_aj;

  /*
    Find out where to insert the semi-join nest and the generated condition.

    For t1 LEFT JOIN t2, embedding_join_nest will be t2.
    Note that t2 may be a simple table or may itself be a join nest
    (e.g. in the case t1 LEFT JOIN (t2 JOIN t3))
  */
  if (subq_pred->embedding_join_nest != nullptr) {
    // Is this on inner side of an outer join?
    outer_join = subq_pred->embedding_join_nest->is_inner_table_of_outer_join();

    if (subq_pred->embedding_join_nest->nested_join) {
      /*
        We're dealing with

          ... [LEFT] JOIN  ( ... ) ON (subquery AND condition) ...

        The sj-nest will be inserted into the brackets nest.
      */
      emb_tbl_nest = subq_pred->embedding_join_nest;
      emb_join_list = &emb_tbl_nest->nested_join->join_list;
    } else if (!subq_pred->embedding_join_nest->outer_join) {
      /*
        We're dealing with

          ... INNER JOIN tblX ON (subquery AND condition) ...

        The sj-nest will be tblX's "sibling", i.e. another child of its
        parent. This is ok because tblX is joined as an inner join.
      */
      emb_tbl_nest = subq_pred->embedding_join_nest->embedding;
      if (emb_tbl_nest) emb_join_list = &emb_tbl_nest->nested_join->join_list;
    } else {
      TABLE_LIST *outer_tbl = subq_pred->embedding_join_nest;
      /*
        We're dealing with

          ... LEFT JOIN tbl ON (on_expr AND subq_pred) ...

        tbl will be replaced with:

          ( tbl SJ (subq_tables) )
          |                      |
          |<----- wrap_nest ---->|

        giving:
          ... LEFT JOIN ( tbl SJ (subq_tables) ) ON (on_expr AND subq_pred) ...

        Q:  other subqueries may be pointing to this element. What to do?
        A1: simple solution: copy *subq_pred->embedding_join_nest= *parent_nest.
            But we'll need to fix other pointers.
        A2: Another way: have TABLE_LIST::next_ptr so the following
            subqueries know the table has been nested.
        A3: changes in the TABLE_LIST::outer_join will make everything work
            automatically.
      */
      TABLE_LIST *const wrap_nest = TABLE_LIST::new_nested_join(
          thd->mem_root, "(sj-wrap)", outer_tbl->embedding,
          outer_tbl->join_list, this);
      if (wrap_nest == nullptr) return true;

      wrap_nest->nested_join->join_list.push_back(outer_tbl);

      outer_tbl->embedding = wrap_nest;
      outer_tbl->join_list = &wrap_nest->nested_join->join_list;

      /*
        An important note, if this 'PREPARE stmt'.
        The FROM clause of the outer query now looks like
        CONCAT(original FROM clause of outer query, sj-nest).
        Given that the original FROM clause is reversed, this list is
        interpreted as "sj-nest is first".
        Thus, at a next execution, setup_natural_join_types() will decide that
        the name resolution context of the FROM clause should start at the
        first inner table in sj-nest.
        However, note that in the present function we do not change
        first_name_resolution_table (and friends) of sj-inner tables.
        So, at the next execution, name resolution for columns of
        outer-table columns is bound to fail (the first inner table does
        not have outer tables in its chain of resolution).
        Fortunately, Item_field::cached_table, which is set during resolution
        of 'PREPARE stmt', gives us the answer and avoids a failing search.
      */

      /*
        wrap_nest will take place of outer_tbl, so move the outer join flag
        and join condition.
      */
      wrap_nest->outer_join = outer_tbl->outer_join;
      outer_tbl->outer_join = JOIN_TYPE_INNER;

      // There are item-rollback problems in this function: see bug#16926177
      wrap_nest->set_join_cond(outer_tbl->join_cond()->real_item());
      outer_tbl->set_join_cond(nullptr);

      for (auto li = wrap_nest->join_list->begin();
           li != wrap_nest->join_list->end(); ++li) {
        TABLE_LIST *tbl = *li;
        if (tbl == outer_tbl) {
          *li = wrap_nest;
          break;
        }
      }

      /*
        outer_tbl is replaced by wrap_nest.
        For subselects, update embedding_join_nest to point to wrap_nest
        instead of outer_tbl.
      */
      for (Item_exists_subselect *subquery : (*sj_candidates)) {
        if (subquery->embedding_join_nest == outer_tbl)
          subquery->embedding_join_nest = wrap_nest;
      }

      /*
        Ok now wrap_nest 'contains' outer_tbl and we're ready to add the
        semi-join nest into it
      */
      emb_join_list = &wrap_nest->nested_join->join_list;
      emb_tbl_nest = wrap_nest;
    }
  }
  // else subquery is in WHERE.

  if (do_aj) {
    /*
      A negated IN/EXISTS like:
      NOT EXISTS(... FROM subq_tables WHERE subq_cond)
      The above code has ensured that we have one of these 3 situations:

      (a) FROM ... WHERE (subquery AND condition)
      (emb_tbl_nest == nullptr, emb_join_list == FROM clause)

      which has to be changed to
          FROM (...)            LEFT JOIN (subq_tables) ON subq_cond
               ^ aj-left-nest             ^aj-nest
          WHERE x IS NULL AND condition

      or:
      (b) ... [LEFT] JOIN ( ...          ) ON (subquery AND condition) ...
                          ^ emb_tbl_nest, emb_join_list

      which has to be changed to
          ... [LEFT] JOIN ( (...)          LEFT JOIN (subq_tables) ON subq_cond)
                            ^aj-left-nest            ^aj-nest
                          ^ emb_tbl_nest, emb_join_list
              ON x IS NULL AND condition ...

      or:
      (c) ... INNER JOIN tblX ON (subquery AND condition) ...
          ^ emb_tbl_nest, emb_join_list
            (if no '()' above this INNER JOIN up to the root, emb_tbl_nest ==
             nullptr and emb_join_list == FROM clause)

      which has to be changed to
       ( ... INNER JOIN tblX ON condition) LEFT JOIN (subq_tables) ON subq_cond
       ^aj-left-nest                                 ^aj-nest

      so:
      - move all tables of emb_join_list into a new aj-left-nest
      - emb_join_list is now empty
      - put subq_tables in a new aj-nest
      - add the subq's subq_cond to aj-nest's ON
      - add a LEFT JOIN operator between the aj-left-nest and aj-nest, with
      ON condition subq_cond.
      - insert aj-nest and aj-left-nest into emb_join_list
      - for some reason, a LEFT JOIN must always be wrapped into a nest (call
      nest_last_join() then)
      - do not yet add 'x IS NULL to WHERE' (add it in optimization phase when
      we have the QEP_TABs so we can set up the 'found'/'not_null_compl'
      pointers in trig conds).
    */
    TABLE_LIST *const wrap_nest = TABLE_LIST::new_nested_join(
        thd->mem_root, "(aj-left-nest)", emb_tbl_nest, emb_join_list, this);
    if (wrap_nest == nullptr) return true;

    // Go through tables of emb_join_list, insert them in wrap_nest
    for (TABLE_LIST *outer_tbl : *emb_join_list) {
      wrap_nest->nested_join->join_list.push_back(outer_tbl);
      outer_tbl->embedding = wrap_nest;
      outer_tbl->join_list = &wrap_nest->nested_join->join_list;

      /*
        outer_tbl is replaced by wrap_nest.
        For subselects, update embedding_join_nest to point to wrap_nest
        instead of outer_tbl.
      */
      for (Item_exists_subselect *subquery : (*sj_candidates)) {
        if (subquery->embedding_join_nest == outer_tbl)
          subquery->embedding_join_nest = wrap_nest;
      }
    }
    // FROM clause is now only the new left nest
    emb_join_list->clear();
    emb_join_list->push_back(wrap_nest);
    outer_join = true;
  }

  if (unlikely(trace->is_started()))
    trace_object.add_alnum("embedded in", emb_tbl_nest ? "JOIN" : "WHERE");

  TABLE_LIST *const sj_nest = TABLE_LIST::new_nested_join(
      thd->mem_root, do_aj ? "(aj-nest)" : "(sj-nest)", emb_tbl_nest,
      emb_join_list, this);
  if (sj_nest == nullptr) return true; /* purecov: inspected */

  NESTED_JOIN *const nested_join = sj_nest->nested_join;

  /* Nests do not participate in those 'chains', so: */
  /* sj_nest->next_leaf= sj_nest->next_local= sj_nest->next_global == NULL*/
  /*
    Using push_front, as sj_nest may be right arg of LEFT JOIN if
    antijoin, and right args of LEFT JOIN go before left arg.
  */
  emb_join_list->push_front(sj_nest);

  /*
    Natural joins inside a semi-join nest were already processed when the
    subquery went through initial preparation.
  */
  sj_nest->nested_join->natural_join_processed = true;
  /*
    nested_join->used_tables and nested_join->not_null_tables are
    initialized in simplify_joins().
  */

  SELECT_LEX *const subq_select = subq_pred->unit->first_select();

  nested_join->query_block_id = subq_select->select_number;

  // Merge tables from underlying query block into this join nest
  if (sj_nest->merge_underlying_tables(subq_select))
    return true; /* purecov: inspected */

  /*
    Add tables from subquery at end of leaf table chain.
    (This also means that table map for parent query block tables are unchanged)
  */
  TABLE_LIST *tl;
  for (tl = leaf_tables; tl->next_leaf; tl = tl->next_leaf) {
  }
  tl->next_leaf = subq_select->leaf_tables;

  // Add tables from subquery at end of next_local chain.
  table_list.push_back(&subq_select->table_list);

  // Note that subquery's tables are already in the next_global chain

  // Remove the original subquery predicate from the WHERE/ON
  // The subqueries were replaced with TRUE value earlier
  // @todo also reset the 'with_subselect' there.

  // Walk through child's tables and adjust table map
  uint table_no = leaf_table_count;
  for (tl = subq_select->leaf_tables; tl; tl = tl->next_leaf, table_no++) {
    tl->dep_tables <<= leaf_table_count;
    tl->set_tableno(table_no);
  }

  /*
    If we leave this function in an error path before subq_select is unlinked,
    make sure tables are not duplicated, or cleanup code could be confused:
  */
  subq_select->table_list.empty();
  subq_select->leaf_tables = nullptr;

  // Adjust table and expression counts in parent query block:
  derived_table_count += subq_select->derived_table_count;
  materialized_derived_table_count +=
      subq_select->materialized_derived_table_count;
  table_func_count += subq_select->table_func_count;
  has_sj_nests |= subq_select->has_sj_nests;
  has_aj_nests |= subq_select->has_aj_nests;
  partitioned_table_count += subq_select->partitioned_table_count;
  leaf_table_count += subq_select->leaf_table_count;
  cond_count += subq_select->cond_count;
  between_count += subq_select->between_count;

  if (subq_select->active_options() & OPTION_SCHEMA_TABLE)
    add_base_options(OPTION_SCHEMA_TABLE);

  if (outer_join) propagate_nullability(&sj_nest->nested_join->join_list, true);

  nested_join->sj_outer_exprs.empty();
  nested_join->sj_inner_exprs.empty();

  subq_pred->exec_method = SubqueryExecMethod::EXEC_SEMI_JOIN;

  if (subq_pred->substype() == Item_subselect::IN_SUBS) {
    Item_in_subselect *in_subq_pred = (Item_in_subselect *)subq_pred;

    DBUG_ASSERT(in_subq_pred->left_expr->fixed);

    /*
    Create the IN-equalities and inject them into semi-join's ON condition.
    Additionally, for LooseScan strategy
     - Record the number of IN-equalities.
     - Create list of pointers to (oe1, ..., ieN). We'll need the list to
       see which of the expressions are bound and which are not (for those
       we'll produce a distinct stream of (ie_i1,...ie_ik).

       (TODO: can we just create a list of pointers and hope the expressions
       will not substitute themselves on fix_fields()? or we need to wrap
       them into Item_view_refs and store pointers to those. The
       pointers to Item_view_refs are guaranteed to be stable as
       Item_view_refs doesn't substitute itself with anything in
       Item_view_ref::fix_fields.

    We have a special case for IN predicates with a scalar subquery or a
    row subquery in the predicand (left operand), such as this:
       (SELECT 1,2 FROM t1) IN (SELECT x,y FROM t2)
    We cannot make the join condition 1=x AND 2=y, since that might evaluate
    to true even if t1 is empty. Instead make the join condition
    (SELECT 1,2 FROM t1) = (x,y) in this case.
    */
    Item_subselect *left_subquery =
        (in_subq_pred->left_expr->type() == Item::SUBSELECT_ITEM)
            ? static_cast<Item_subselect *>(in_subq_pred->left_expr)
            : nullptr;

    if (left_subquery &&
        (left_subquery->substype() == Item_subselect::SINGLEROW_SUBS)) {
      List<Item> ref_list;
      Item *header = subq_select->base_ref_items[0];
      for (uint i = 1; i < in_subq_pred->left_expr->cols(); i++) {
        ref_list.push_back(subq_select->base_ref_items[i]);
      }

      Item_row *right_expr = new Item_row(header, ref_list);
      if (!right_expr) return true; /* purecov: inspected */

      nested_join->sj_outer_exprs.push_back(in_subq_pred->left_expr);
      nested_join->sj_inner_exprs.push_back(right_expr);
    } else {
      for (uint i = 0; i < in_subq_pred->left_expr->cols(); i++) {
        Item *const li = in_subq_pred->left_expr->element_index(i);
        nested_join->sj_outer_exprs.push_back(li);
        nested_join->sj_inner_exprs.push_back(subq_select->base_ref_items[i]);

        Item **lr_outer = nested_join->sj_outer_exprs.tail_ref();
        Item **lr_inner = nested_join->sj_inner_exprs.tail_ref();
        DBUG_ASSERT(*lr_inner == subq_select->base_ref_items[i]);
        DBUG_ASSERT(*lr_outer == li);
        if (maybe_mark_item_for_rollback(thd, lr_outer)) return true;
        if (maybe_mark_item_for_rollback(thd, lr_inner)) return true;
        DBUG_ASSERT(*lr_inner == subq_select->base_ref_items[i]);
        DBUG_ASSERT(*lr_outer == li);
      }
    }
  } else {  // this is EXISTS
    // Expressions from the SELECT list will not be used; unlike in the case of
    // IN, they are not part of sj_inner_exprs.
    // @todo in WL#6570, move this to resolve_subquery().
    Item::Cleanup_after_removal_context ctx(this);
    List_iterator<Item> li(subq_select->fields_list);
    Item *item;
    while ((item = li++)) {
      item->walk(&Item::clean_up_after_removal, enum_walk::SUBQUERY_POSTFIX,
                 pointer_cast<uchar *>(&ctx));
    }
  }

  /*
    The WHERE clause and the join conditions may contain equalities that may
    be leveraged by semi-join strategies (e.g to set up key lookups in
    semi-join materialization), decorrelate them (ie. add respective fields
    and expressions to sj_inner_exprs and sj_outer_exprs).
  */
  if (subq_select->where_cond() &&
      subq_select->decorrelate_condition(sj_nest, nullptr))
    return true;

  if (walk_join_list(subq_select->top_join_list, [&](TABLE_LIST *tr) -> bool {
        return !tr->is_inner_table_of_outer_join() && tr->join_cond() &&
               subq_select->decorrelate_condition(sj_nest, tr);
      }))
    return true;

  // Unlink the subquery's query expression:
  subq_select->master_unit()->exclude_level();

  // Merge subquery's name resolution contexts into parent's
  merge_contexts(subq_select);

  repoint_contexts_of_join_nests(subq_select->top_join_list);

  // Update table map for semi-join nest's WHERE condition and join conditions
  fix_tables_after_pullout(this, subq_select, sj_nest, 0);

  Item *sj_cond = subq_select->where_cond();
  if (sj_cond != nullptr) sj_cond->fix_after_pullout(this, subq_select);

  // Assign the set of non-trivially tables after decorrelation
  nested_join->sj_corr_tables =
      (sj_cond != nullptr ? sj_cond->used_tables() & outer_tables_map : 0);

  walk_join_list(subq_select->top_join_list, [&](TABLE_LIST *tr) -> bool {
    if (tr->join_cond())
      nested_join->sj_corr_tables |=
          tr->join_cond()->used_tables() & outer_tables_map;
    if (tr->is_derived() && tr->uses_materialization())
      nested_join->sj_corr_tables |= tr->derived_unit()->m_lateral_deps;
    return false;
  });

  // Build semijoin condition using the inner/outer expression list
  if (build_sj_cond(thd, nested_join, subq_select, outer_tables_map, &sj_cond))
    return true;

  // Processing requires a non-empty semi-join condition:
  DBUG_ASSERT(sj_cond != nullptr);

  // Fix the created equality and AND
  if (!sj_cond->fixed) {
    Opt_trace_array sj_on_trace(&thd->opt_trace,
                                "evaluating_constant_semijoin_conditions");
    sj_cond->apply_is_true();
    if (sj_cond->fix_fields(thd, &sj_cond))
      return true; /* purecov: inspected */
  }

  sj_nest->set_sj_or_aj_nest();
  DBUG_ASSERT(sj_nest->join_cond() == nullptr);

  if (do_aj) {
    sj_nest->outer_join = JOIN_TYPE_LEFT;
    sj_nest->set_join_cond(sj_cond);
    this->outer_join |= sj_nest->nested_join->used_tables;
    if (emb_tbl_nest == nullptr)
      nest_last_join(thd);  // as is done for a true LEFT JOIN
  }

  if (unlikely(trace->is_started())) {
    trace_object.add("semi-join condition", sj_cond);
    Opt_trace_array trace_dep(trace, "decorrelated_predicates");
    List_iterator<Item> ii(nested_join->sj_inner_exprs);
    List_iterator<Item> oi(nested_join->sj_outer_exprs);
    Item *inner, *outer;
    while (outer = oi++, inner = ii++) {
      Opt_trace_object trace_predicate(trace);
      trace_predicate.add("outer", outer);
      trace_predicate.add("inner", inner);
    }
  }

  /*
    sj_depends_on contains the set of outer tables referred in the
    subquery's WHERE clause as well as tables referred in the IN predicate's
    left-hand side, and lateral dependencies from materialized derived tables
    contained in the original subquery.
  */
  nested_join->sj_depends_on =
      nested_join->sj_corr_tables | (sj_cond->used_tables() & outer_tables_map);

  // TODO fix QT_
  DBUG_EXECUTE("where", print_where(thd, sj_cond, "SJ-COND", QT_ORDINARY););

  if (do_aj) { /* Condition remains attached to inner table, as for LEFT JOIN
                */
  } else if (emb_tbl_nest) {
    // Inject semi-join condition into parent's join condition
    emb_tbl_nest->set_join_cond(and_items(emb_tbl_nest->join_cond(), sj_cond));
    if (emb_tbl_nest->join_cond() == nullptr) return true;
    emb_tbl_nest->join_cond()->apply_is_true();
    if (!emb_tbl_nest->join_cond()->fixed &&
        emb_tbl_nest->join_cond()->fix_fields(thd,
                                              emb_tbl_nest->join_cond_ref()))
      return true;
  } else {
    // Inject semi-join condition into parent's WHERE condition
    m_where_cond = and_items(m_where_cond, sj_cond);
    if (m_where_cond == nullptr) return true;
    m_where_cond->apply_is_true();
    if (m_where_cond->fix_fields(thd, &m_where_cond)) return true;
  }

  Item *cond = emb_tbl_nest ? emb_tbl_nest->join_cond() : m_where_cond;
  if (cond && cond->const_item() &&
      !cond->walk(&Item::is_non_const_over_literals, enum_walk::POSTFIX,
                  nullptr)) {
    bool cond_value = true;
    if (simplify_const_condition(thd, &cond, false, &cond_value)) return true;
    if (!cond_value) {
      /*
        Parent's condition is always FALSE. Thus:
        (a) the value of the anti/semi-join condition has no influence on the
        result
        (b) we don't need to set up lookups (for loosescan or materialization)
        (c) for a semi-join, the semi-join condition is already lost (it was
        in parent's condition, which has been replaced with FALSE); the
        outer/inner sj expressions are Items which point into the SJ
        condition, so at 2nd execution they won't be fixed => clearing them
        prevents a bug.
        (d) for an anti-join, the join condition remains in
        sj_nest->join_cond() and will possibly be evaluated. (c) doesn't hold,
        but (a) and (b) do.
      */
      clear_sj_expressions(nested_join);
    }
  }

  if (subq_select->ftfunc_list->elements &&
      add_ftfunc_list(subq_select->ftfunc_list))
    return true; /* purecov: inspected */

  if (do_aj)
    has_aj_nests = true;
  else
    has_sj_nests = true;  // This query block has semi-join nests

  return false;
}

/**
  Merge a derived table or view into a query block.
  If some constraint prevents the derived table from being merged then do
  nothing, which means the table will be prepared for materialization later.

  After this call, check is_merged() to see if the table was really merged.

  @param thd           Thread handler
  @param derived_table Derived table which is to be merged.

  @return false if successful, true if error
*/

bool SELECT_LEX::merge_derived(THD *thd, TABLE_LIST *derived_table) {
  DBUG_TRACE;

  if (!derived_table->is_view_or_derived() || derived_table->is_merged())
    return false;

  SELECT_LEX_UNIT *const derived_unit = derived_table->derived_unit();

  // A derived table must be prepared before we can merge it
  DBUG_ASSERT(derived_unit->is_prepared());

  LEX *const lex = parent_lex;

  // Check whether the outer query allows merged views
  if ((master_unit() == lex->unit && !lex->can_use_merged()) ||
      lex->can_not_use_merged())
    return false;

  /*
    @todo: The implementation of LEX::can_use_merged() currently avoids
           merging of views that are contained in other views if
           can_use_merged() returns false.
  */
  /*
    Check whether derived table is mergeable, and directives allow merging;
    priority order is:
    - ALGORITHM says MERGE or TEMPTABLE
    - hint specifies MERGE or NO_MERGE (=materialization)
    - optimizer_switch's derived_merge is ON and heuristic suggests merge
  */
  if (derived_table->algorithm == VIEW_ALGORITHM_TEMPTABLE ||
      !derived_unit->is_mergeable())
    return false;

  if (derived_table->algorithm == VIEW_ALGORITHM_UNDEFINED) {
    const bool merge_heuristic =
        (derived_table->is_view() || allow_merge_derived) &&
        derived_unit->merge_heuristic(thd->lex);
    if (!hint_table_state(thd, derived_table, DERIVED_MERGE_HINT_ENUM,
                          merge_heuristic ? OPTIMIZER_SWITCH_DERIVED_MERGE : 0))
      return false;
  }

  SELECT_LEX *const derived_select = derived_unit->first_select();
  /*
    If STRAIGHT_JOIN is specified, it is not valid to merge in a query block
    that contains semi-join nests
  */
  if ((active_options() & SELECT_STRAIGHT_JOIN) &&
      (derived_select->has_sj_nests || derived_select->has_aj_nests))
    return false;

  // Check that we have room for the merged tables in the table map:
  if (leaf_table_count + derived_select->leaf_table_count - 1 > MAX_TABLES)
    return false;

  derived_table->set_merged();

  DBUG_PRINT("info", ("algorithm: MERGE"));

  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  Opt_trace_object trace_derived(trace,
                                 derived_table->is_view() ? "view" : "derived");
  trace_derived.add_utf8_table(derived_table)
      .add("select#", derived_select->select_number)
      .add("merged", true);

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  // Save offset for table number adjustment
  uint table_adjust = derived_table->tableno();

  // Set up permanent list of underlying tables of a merged view
  derived_table->merge_underlying_list = derived_select->get_table_list();

  /**
    A view is updatable if any underlying table is updatable.
    A view is insertable-into if all underlying tables are insertable.
    A view is not updatable nor insertable if it contains an outer join
    @see mysql_register_view()
  */
  if (derived_table->is_view()) {
    bool updatable = false;
    bool insertable = true;
    bool outer_joined = false;
    for (TABLE_LIST *tr = derived_table->merge_underlying_list; tr;
         tr = tr->next_local) {
      updatable |= tr->is_updatable();
      insertable &= tr->is_insertable();
      outer_joined |= tr->is_inner_table_of_outer_join();
    }
    updatable &= !outer_joined;
    insertable &= !outer_joined;
    if (updatable) derived_table->set_updatable();
    if (insertable) derived_table->set_insertable();

    TABLE_LIST *tl;
    List_iterator<TABLE_LIST> tli(derived_select->m_scalar_to_derived);
    while ((tl = tli++)) m_scalar_to_derived.push_back(tl);
  }
  // Add a nested join object to the derived table object
  if (!(derived_table->nested_join = new (thd->mem_root) NESTED_JOIN))
    return true;

  // Merge tables from underlying query block into this join nest
  if (derived_table->merge_underlying_tables(derived_select))
    return true; /* purecov: inspected */

  // Replace derived table in leaf table list with underlying tables:
  for (TABLE_LIST **tl = &leaf_tables; *tl; tl = &(*tl)->next_leaf) {
    if (*tl == derived_table) {
      for (TABLE_LIST *leaf = derived_select->leaf_tables; leaf;
           leaf = leaf->next_leaf) {
        leaf->dep_tables <<= table_adjust;
        if (leaf->next_leaf == nullptr) {
          leaf->next_leaf = (*tl)->next_leaf;
          break;
        }
      }
      *tl = derived_select->leaf_tables;
      break;
    }
  }

  leaf_table_count += (derived_select->leaf_table_count - 1);
  derived_table_count += derived_select->derived_table_count;
  table_func_count += derived_select->table_func_count;
  materialized_derived_table_count +=
      derived_select->materialized_derived_table_count;
  has_sj_nests |= derived_select->has_sj_nests;
  has_aj_nests |= derived_select->has_aj_nests;
  partitioned_table_count += derived_select->partitioned_table_count;
  cond_count += derived_select->cond_count;
  between_count += derived_select->between_count;

  // Propagate schema table indication:
  // @todo: Add to BASE options instead
  if (derived_select->active_options() & OPTION_SCHEMA_TABLE)
    add_base_options(OPTION_SCHEMA_TABLE);

  // Propagate nullability for derived tables within outer joins:
  if (derived_table->is_inner_table_of_outer_join())
    propagate_nullability(&derived_table->nested_join->join_list, true);

  select_n_having_items += derived_select->select_n_having_items;

  // Merge the WHERE clause into the outer query block
  if (derived_table->merge_where(thd)) return true; /* purecov: inspected */

  if (derived_table->create_field_translation(thd))
    return true; /* purecov: inspected */

  // Exclude the derived table query expression from query graph.
  derived_unit->exclude_level();

  // Don't try to access it:
  derived_table->set_derived_unit((SELECT_LEX_UNIT *)1);

  // Merge subquery's name resolution contexts into parent's
  merge_contexts(derived_select);

  repoint_contexts_of_join_nests(derived_select->top_join_list);

  // Leaf tables have been shuffled, so update table numbers for them
  remap_tables(thd);

  // Update table info of referenced expressions after query block is merged
  fix_tables_after_pullout(this, derived_select, derived_table, table_adjust);

  if (derived_select->is_ordered()) {
    /*
      An ORDER BY clause is moved to an outer query block
      - if the outer query block allows ordering, and
      - that refers to this view/derived table only, and
      - is not part of a UNION, and
      - may have a WHERE clause but is not grouped or aggregated and is not
        itself ordered.
     Otherwise the ORDER BY clause is ignored.

     Only SELECT statements and single-table UPDATE and DELETE statements
     allow ordering.

     Up to version 5.6 included, ORDER BY was unconditionally merged.
     Currently we only merge in the simple case above, which ensures
     backward compatibility for most reasonable use cases.

     Note that table numbers in order_list do not need updating, since
     the outer query contains only one table reference.
    */
    // LIMIT currently blocks derived table merge
    DBUG_ASSERT(!derived_select->has_limit());

    if ((lex->sql_command == SQLCOM_SELECT ||
         lex->sql_command == SQLCOM_UPDATE ||
         lex->sql_command == SQLCOM_DELETE) &&
        !(master_unit()->is_union() || is_grouped() || is_distinct() ||
          is_ordered() || get_table_list()->next_local != nullptr)) {
      order_list.push_back(&derived_select->order_list);
      /*
        If at outer-most level (not within another derived table), ensure
        the ordering columns are marked in read_set, since columns selected
        from derived tables are not marked in initial resolving.
      */
      if (!thd->derived_tables_processing) {
        Mark_field mf(thd->mark_used_columns);
        for (ORDER *o = derived_select->order_list.first; o != nullptr;
             o = o->next)
          o->item[0]->walk(&Item::mark_field_in_map, enum_walk::POSTFIX,
                           pointer_cast<uchar *>(&mf));
      }
    } else {
      derived_select->empty_order_list(this);
      trace_derived.add_alnum("transformations_to_derived_table",
                              "removed_ordering");
    }
  }

  // Add any full-text functions from derived table into outer query
  if (derived_select->ftfunc_list->elements &&
      add_ftfunc_list(derived_select->ftfunc_list))
    return true; /* purecov: inspected */

  /*
    The "laterality" of this nest is not interesting anymore; it was
    transferred to underlying tables.
  */
  derived_unit->m_lateral_deps = 0;

  return false;
}

/**
   Destructively replaces a sub-condition inside a condition tree. The
   parse tree is also altered.

   @note Because of current requirements for semijoin flattening, we do not
   need to recurse here, hence this function will only examine the top-level
   AND conditions. (see SELECT_LEX::prepare, comment starting with "Check if
   the subquery predicate can be executed via materialization".)

   @param thd  thread handler

   @param tree Must be the handle to the top level condition. This is needed
   when the top-level condition changes.

   @param old_cond The condition to be replaced.

   @param new_cond The condition to be substituted.

   @param do_fix_fields If true, Item::fix_fields(THD*, Item**) is called for
   the new condition.

   @return error status

   @retval true If there was an error.
   @retval false If successful.
*/

static bool replace_subcondition(THD *thd, Item **tree, Item *old_cond,
                                 Item *new_cond, bool do_fix_fields) {
  if (*tree == old_cond) {
    *tree = new_cond;
    if (do_fix_fields && new_cond->fix_fields(thd, tree)) return true;
    return false;
  } else if ((*tree)->type() == Item::COND_ITEM) {
    List_iterator<Item> li(*((Item_cond *)(*tree))->argument_list());
    Item *item;
    while ((item = li++)) {
      if (item == old_cond) {
        li.replace(new_cond);
        if (do_fix_fields && new_cond->fix_fields(thd, li.ref())) return true;
        return false;
      }
    }
  } else
    // If we came here it means there were an error during prerequisites check.
    DBUG_ASSERT(false);

  return true;
}

/**
  Convert semi-join subquery predicates into semi-join join nests.

  Convert candidate subquery predicates into semi-join join nests. This
  transformation is performed once in query lifetime and is irreversible.

  Conversion of one subquery predicate
  ------------------------------------

  We start with a query block that has a semi-join subquery predicate:

  @code
  SELECT ...
  FROM ot, ...
  WHERE oe IN (SELECT ie FROM it1 ... itN WHERE subq_where) AND outer_where
  @endcode

  and convert the predicate and subquery into a semi-join nest:

  @code
  SELECT ...
  FROM ot SEMI JOIN (it1 ... itN), ...
  WHERE outer_where AND subq_where AND oe=ie
  @endcode

  that is, in order to do the conversion, we need to

   * Create the "SEMI JOIN (it1 .. itN)" part and add it into the parent
     query block's FROM structure.
   * Add "AND subq_where AND oe=ie" into parent query block's WHERE (or ON if
     the subquery predicate was in an ON condition)
   * Remove the subquery predicate from the parent query block's WHERE

  Considerations when converting many predicates
  ----------------------------------------------

  A join may have at most MAX_TABLES tables. This may prevent us from
  flattening all subqueries when the total number of tables in parent and
  child selects exceeds MAX_TABLES. In addition, one slot is reserved per
  semi-join nest, in case the subquery needs to be materialized in a
  temporary table.
  We deal with this problem by flattening children's subqueries first and
  then using a heuristic rule to determine each subquery predicate's
  priority, which is calculated in this order:

  1. Prefer dependent subqueries over non-dependent ones
  2. Prefer subqueries with many tables over those with fewer tables
  3. Prefer early subqueries over later ones (to make sort deterministic)

  @returns false if success, true if error
*/
bool SELECT_LEX::flatten_subqueries(THD *thd) {
  DBUG_TRACE;

  DBUG_ASSERT(has_sj_candidates());

  Item_exists_subselect **subq, **subq_begin = sj_candidates->begin(),
                                **subq_end = sj_candidates->end();

  Opt_trace_context *const trace = &thd->opt_trace;

  /*
    Semijoin flattening is bottom-up. Indeed, we have this execution flow,
    for SELECT#1 WHERE X IN (SELECT #2 WHERE Y IN (SELECT#3)) :

    SELECT_LEX::prepare() (select#1)
       -> fix_fields() on IN condition
           -> SELECT_LEX::prepare() on subquery (select#2)
               -> fix_fields() on IN condition
                    -> SELECT_LEX::prepare() on subquery (select#3)
                    <- SELECT_LEX::prepare()
               <- fix_fields()
               -> flatten_subqueries: merge #3 in #2
               <- flatten_subqueries
           <- SELECT_LEX::prepare()
       <- fix_fields()
       -> flatten_subqueries: merge #2 in #1

    Note that flattening of #(N) is done by its parent JOIN#(N-1), because
    there are cases where flattening is not possible and only the parent can
    know.
   */
  uint subq_no;
  for (subq = subq_begin, subq_no = 0; subq < subq_end; subq++, subq_no++) {
    auto subq_item = *subq;
    // Transformation of IN and EXISTS subqueries is supported
    DBUG_ASSERT(subq_item->substype() == Item_subselect::IN_SUBS ||
                subq_item->substype() == Item_subselect::EXISTS_SUBS);

    SELECT_LEX *child_select = subq_item->unit->first_select();

    // Check that we proceeded bottom-up
    DBUG_ASSERT(child_select->sj_candidates == nullptr);

    bool dependent = subq_item->unit->uncacheable & UNCACHEABLE_DEPENDENT;
    subq_item->sj_convert_priority =
        (((dependent * MAX_TABLES_FOR_SIZE) +  // dependent subqueries first
          child_select->leaf_table_count) *
         65536) +           // then with many tables
        (65536 - subq_no);  // then based on position

    /*
      We may actually allocate more than 64k subqueries in a query block,
      but this is so unlikely that we ignore the impact it may have on sorting.
     */
  }

  /*
    Pick which subqueries to convert:
      sort the subquery array
      - prefer correlated subqueries over uncorrelated;
      - prefer subqueries that have greater number of outer tables;
  */
  std::sort(subq_begin, subq_begin + sj_candidates->size(),
            [](Item_exists_subselect *el1, Item_exists_subselect *el2) {
              return el1->sj_convert_priority > el2->sj_convert_priority;
            });

  // A permanent transformation is going to start, so:
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  /*
    Replace all subqueries to be flattened with a truth predicate.
    Generally, this predicate is TRUE, but if the subquery has a WHERE condition
    that is always false, replace with a FALSE predicate. In the latter case,
    also avoid converting the subquery to a semi-join.
  */

  uint table_count = leaf_table_count;
  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    // Add the tables in the subquery nest plus one in case of materialization:
    const uint tables_added =
        subq_item->unit->first_select()->leaf_table_count + 1;

    // (1) Not too many tables in total.
    // (2) This subquery contains no antijoin nest (anti/semijoin nest cannot
    // include antijoin nest for implementation reasons, see
    // advance_sj_state()).
    if (table_count + tables_added <= MAX_TABLES &&      // (1)
        !subq_item->unit->first_select()->has_aj_nests)  // (2)
      subq_item->sj_selection = Item_exists_subselect::SJ_SELECTED;

    Item *subq_where = subq_item->unit->first_select()->where_cond();
    /*
      A predicate can be evaluated to ALWAYS TRUE or ALWAYS FALSE when it
      has only const items. If found to be ALWAYS FALSE, do not include
      the subquery in transformations.
    */
    bool cond_value = true;
    if (subq_where && subq_where->const_item() &&
        !subq_where->walk(&Item::is_non_const_over_literals, enum_walk::POSTFIX,
                          nullptr) &&
        simplify_const_condition(thd, &subq_where, false, &cond_value))
      return true;

    if (!cond_value) {
      subq_item->sj_selection = Item_exists_subselect::SJ_ALWAYS_FALSE;
      // Unlink this subquery's query expression
      Item::Cleanup_after_removal_context ctx(this);
      subq_item->walk(&Item::clean_up_after_removal,
                      enum_walk::SUBQUERY_POSTFIX, pointer_cast<uchar *>(&ctx));
      // The cleaning up has called remove_semijoin_candidate() which has
      // changed the sj_candidates array: now *subq is the _next_ subquery.
      subq--;  // So that the next iteration will handle the next subquery.
      DBUG_ASSERT(subq_begin == sj_candidates->begin());
      subq_end = sj_candidates->end();  // array's end moved.
    }

    if (subq_item->sj_selection == Item_exists_subselect::SJ_SELECTED)
      table_count += tables_added;

    if (subq_item->sj_selection == Item_exists_subselect::SJ_NOT_SELECTED)
      continue;
    /*
      In WHERE/ON of parent query, replace IN (subq) with truth value:
      - When subquery is converted to anti/semi-join: truth value true.
      - When subquery WHERE cond is false: IN returns FALSE, so truth value
      false if a semijoin (IN) and truth value true if an antijoin (NOT IN).
    */
    Item *truth_item =
        (cond_value || subq_item->can_do_aj)
            ? down_cast<Item *>(new (thd->mem_root) Item_func_true())
            : down_cast<Item *>(new (thd->mem_root) Item_func_false());
    if (truth_item == nullptr) return true;
    Item **tree = (subq_item->embedding_join_nest == nullptr)
                      ? &m_where_cond
                      : subq_item->embedding_join_nest->join_cond_ref();
    if (replace_subcondition(thd, tree, subq_item, truth_item, false))
      return true; /* purecov: inspected */
  }

  /* Transform the selected subqueries into semi-join */

  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->sj_selection != Item_exists_subselect::SJ_SELECTED) continue;

    OPT_TRACE_TRANSFORM(
        trace, oto0, oto1, subq_item->unit->first_select()->select_number,
        "IN (SELECT)", subq_item->can_do_aj ? "antijoin" : "semijoin");
    oto1.add("chosen", true);
    if (convert_subquery_to_semijoin(thd, *subq)) return true;
  }
  /*
    Finalize the subqueries that we did not convert,
    ie. perform IN->EXISTS rewrite.
  */
  for (subq = subq_begin; subq < subq_end; subq++) {
    auto subq_item = *subq;
    if (subq_item->sj_selection != Item_exists_subselect::SJ_NOT_SELECTED)
      continue;
    {
      OPT_TRACE_TRANSFORM(trace, oto0, oto1,
                          subq_item->unit->first_select()->select_number,
                          "IN (SELECT)", "semijoin");
      oto1.add("chosen", false);
    }
    Item_subselect::trans_res res;
    subq_item->changed = false;
    subq_item->fixed = false;

    SELECT_LEX *save_select_lex = thd->lex->current_select();
    thd->lex->set_current_select(subq_item->unit->first_select());

    // This is the only part of the function which uses a JOIN.
    res = subq_item->select_transformer(thd, subq_item->unit->first_select());

    thd->lex->set_current_select(save_select_lex);

    if (res == Item_subselect::RES_ERROR) return true;

    subq_item->changed = true;
    subq_item->fixed = true;

    /*
      If the Item has been substituted with another Item (e.g an
      Item_in_optimizer), resolve it and add it to proper WHERE or ON clause.
      If no substitute exists (e.g for EXISTS predicate), no action is required.
    */
    Item *substitute = subq_item->substitution;
    if (substitute == nullptr) continue;
    const bool do_fix_fields = !substitute->fixed;
    const bool subquery_in_join_clause =
        subq_item->embedding_join_nest != nullptr;

    Item **tree = subquery_in_join_clause
                      ? (subq_item->embedding_join_nest->join_cond_ref())
                      : &m_where_cond;
    if (replace_subcondition(thd, tree, *subq, substitute, do_fix_fields))
      return true;
    subq_item->substitution = nullptr;
  }

  sj_candidates->clear();
  return false;
}

bool SELECT_LEX::is_in_select_list(Item *cand) {
  List_iterator<Item> li(fields_list);
  Item *item;
  while ((item = li++)) {
    // Use a walker to detect if cand is present in this select item

    if (item->walk(&Item::find_item_processor, enum_walk::SUBQUERY_POSTFIX,
                   pointer_cast<uchar *>(cand)))
      return true;
  }
  return false;
}

/**
  Propagate nullability into inner tables of outer join operation

  @param tables  List of tables and join nests, start at top_join_list
  @param nullable  true: Set all underlying tables as nullable
*/
void propagate_nullability(mem_root_deque<TABLE_LIST *> *tables,
                           bool nullable) {
  for (TABLE_LIST *tr : *tables) {
    if (tr->table && !tr->table->is_nullable() && (nullable || tr->outer_join))
      tr->table->set_nullable();
    if (tr->nested_join == nullptr) continue;
    propagate_nullability(&tr->nested_join->join_list,
                          nullable || tr->outer_join);
  }
}

/**
  Propagate exclusion from unique table check into all subqueries belonging
  to this query block.

  This function can be applied to all subqueries of a materialized derived
  table or view.
*/

void SELECT_LEX::propagate_unique_test_exclusion() {
  for (SELECT_LEX_UNIT *unit = first_inner_unit(); unit;
       unit = unit->next_unit())
    for (SELECT_LEX *sl = unit->first_select(); sl; sl = sl->next_select())
      sl->propagate_unique_test_exclusion();

  exclude_from_table_unique_test = true;
}

/**
  Add a list of full-text function elements into a query block.

  @param ftfuncs   List of full-text function elements to add.

  @returns false if success, true if error
*/

bool SELECT_LEX::add_ftfunc_list(List<Item_func_match> *ftfuncs) {
  Item_func_match *ifm;
  List_iterator_fast<Item_func_match> li(*ftfuncs);
  while ((ifm = li++)) {
    if (ftfunc_list->push_back(ifm)) return true; /* purecov: inspected */
  }
  return false;
}

/**
   Go through a list of tables and join nests, recursively, and repoint
   its select_lex pointer.

   @param  join_list  List of tables and join nests
*/
void SELECT_LEX::repoint_contexts_of_join_nests(
    mem_root_deque<TABLE_LIST *> join_list) {
  for (TABLE_LIST *tbl : join_list) {
    tbl->select_lex = this;
    if (tbl->nested_join)
      repoint_contexts_of_join_nests(tbl->nested_join->join_list);
  }
}

/**
  Merge name resolution context objects belonging to an inner subquery
  to parent query block.
  Update all context objects to have this base query block.
  Used when a subquery's query block is merged into its parent.

  @param inner  Subquery for which context objects are to be merged.
*/
void SELECT_LEX::merge_contexts(SELECT_LEX *inner) {
  for (Name_resolution_context *ctx = inner->first_context; ctx != nullptr;
       ctx = ctx->next_context) {
    ctx->select_lex = this;
    if (ctx->next_context == nullptr) {
      ctx->next_context = first_context;
      first_context = inner->first_context;
      inner->first_context = nullptr;
      break;
    }
  }
}

/**
   For a table subquery predicate (IN/ANY/ALL/EXISTS/etc):
   since it does not support LIMIT the following clauses are redundant:

   ORDER BY
   DISTINCT
   GROUP BY   if there are no aggregate functions and no HAVING clause

   For a scalar subquery without LIMIT:
   ORDER BY is redundant, as the number of rows to order must be 1.

   This removal is permanent. Thus, it only makes sense to call this function
   for regular queries and on first execution of SP/PS

   @param thd               thread handler
   @param hidden_group_field_count Number of hidden group fields added
                            by setup_group().
*/

void SELECT_LEX::remove_redundant_subquery_clauses(
    THD *thd, int hidden_group_field_count) {
  Item_subselect *subq_predicate = master_unit()->item;
  enum change {
    REMOVE_NONE = 0,
    REMOVE_ORDER = 1 << 0,
    REMOVE_DISTINCT = 1 << 1,
    REMOVE_GROUP = 1 << 2
  };
  uint possible_changes;

  if (subq_predicate->substype() == Item_subselect::SINGLEROW_SUBS) {
    if (explicit_limit) return;
    possible_changes = REMOVE_ORDER;
  } else {
    DBUG_ASSERT(subq_predicate->substype() == Item_subselect::EXISTS_SUBS ||
                subq_predicate->substype() == Item_subselect::IN_SUBS ||
                subq_predicate->substype() == Item_subselect::ALL_SUBS ||
                subq_predicate->substype() == Item_subselect::ANY_SUBS);
    possible_changes = REMOVE_ORDER | REMOVE_DISTINCT | REMOVE_GROUP;
  }

  uint changelog = 0;

  if ((possible_changes & REMOVE_ORDER) && order_list.elements) {
    changelog |= REMOVE_ORDER;
    empty_order_list(this);
  }

  if ((possible_changes & REMOVE_DISTINCT) && is_distinct()) {
    changelog |= REMOVE_DISTINCT;
    remove_base_options(SELECT_DISTINCT);
  }

  /*
    Remove GROUP BY if there are no aggregate functions, no HAVING clause,
    no ROLLUP and no windowing functions.
  */

  if ((possible_changes & REMOVE_GROUP) && group_list.elements &&
      !agg_func_used() && !having_cond() && olap == UNSPECIFIED_OLAP_TYPE &&
      m_windows.elements == 0) {
    changelog |= REMOVE_GROUP;
    for (ORDER *g = group_list.first; g != nullptr; g = g->next) {
      if (*g->item == g->item_ptr) {
        Item::Cleanup_after_removal_context ctx(this);
        (*g->item)->walk(&Item::clean_up_after_removal,
                         enum_walk::SUBQUERY_POSTFIX,
                         pointer_cast<uchar *>(&ctx));
      }
    }
    group_list.empty();
    while (hidden_group_field_count-- > 0) {
      all_fields.pop();
      base_ref_items[all_fields.elements] = NULL;
    }
  }

  if (changelog) {
    Opt_trace_context *trace = &thd->opt_trace;
    if (unlikely(trace->is_started())) {
      Opt_trace_object trace_wrapper(trace);
      Opt_trace_array trace_changes(trace, "transformations_to_subquery");
      if (changelog & REMOVE_ORDER) trace_changes.add_alnum("removed_ordering");
      if (changelog & REMOVE_DISTINCT)
        trace_changes.add_alnum("removed_distinct");
      if (changelog & REMOVE_GROUP) trace_changes.add_alnum("removed_grouping");
    }
  }
}

/**
  Empty the ORDER list.
  Delete corresponding elements from all_fields and base_ref_items too.
  If ORDER list contain any subqueries, delete them from the query block list.

  @param sl  Query block that possible subquery blocks in the ORDER BY clause
             are attached to (may be different from "this" when query block has
             been merged into an outer query block).
*/

void SELECT_LEX::empty_order_list(SELECT_LEX *sl) {
  if (m_windows.elements != 0) {
    /*
      The next lines doing cleanup of ORDER elements expect the
      query block's ORDER BY items to be the last part of all_fields and
      base_ref_items, as they just chop the lists' end. But if there is a
      window, that end is actually the PARTITION BY and ORDER BY clause of the
      window, so do not chop then: leave the items in place.
    */
    order_list.empty();
    return;
  }
  for (ORDER *o = order_list.first; o != nullptr; o = o->next) {
    if (*o->item == o->item_ptr) {
      Item::Cleanup_after_removal_context ctx(sl);
      (*o->item)->walk(&Item::clean_up_after_removal,
                       enum_walk::SUBQUERY_POSTFIX,
                       pointer_cast<uchar *>(&ctx));
    }
  }
  order_list.empty();
  while (hidden_order_field_count-- > 0) {
    all_fields.pop();
    base_ref_items[all_fields.elements] = NULL;
  }
}

/*****************************************************************************
  Group and order functions
*****************************************************************************/

/**
  Resolve an ORDER BY or GROUP BY column reference.

  Given a column reference (represented by 'order') from a GROUP BY or ORDER
  BY clause, find the actual column it represents. If the column being
  resolved is from the GROUP BY clause, the procedure searches the SELECT
  list 'fields' and the columns in the FROM list 'tables'. If 'order' is from
  the ORDER BY clause, only the SELECT list is being searched.

  If 'order' is resolved to an Item, then order->item is set to the found
  Item. If there is no item for the found column (that is, it was resolved
  into a table field), order->item is 'fixed' and is added to all_fields and
  ref_item_array.

  ref_item_array and all_fields are updated.

  @param[in] thd                    Pointer to current thread structure
  @param[in,out] ref_item_array     All select, group and order by fields
  @param[in] tables                 List of tables to search in (usually
    FROM clause)
  @param[in] order                  Column reference to be resolved
  @param[in] fields                 List of fields to search in (usually
    SELECT list)
  @param[in,out] all_fields         All select, group and order by fields
  @param[in] is_group_field         True if order is a GROUP field, false if
    ORDER by field
  @param[in] is_window_order        True if order is a Window function's
    PARTITION BY or ORDER BY field

  @retval
    false if OK
  @retval
    true  if error occurred
*/

bool find_order_in_list(THD *thd, Ref_item_array ref_item_array,
                        TABLE_LIST *tables, ORDER *order, List<Item> &fields,
                        List<Item> &all_fields, bool is_group_field,
                        bool is_window_order) {
  Item *order_item = *order->item; /* The item from the GROUP/ORDER clause. */
  Item::Type order_item_type;
  Item **select_item; /* The corresponding item from the SELECT clause. */
  Field *from_field;  /* The corresponding field from the FROM clause. */
  uint counter;
  enum_resolution_type resolution;

  /*
    Local SP variables may be int but are expressions, not positions.
    (And they can't be used before fix_fields is called for them).
  */
  if (order_item->type() == Item::INT_ITEM &&
      order_item->basic_const_item()) { /* Order by position */
    uint count = (uint)order_item->val_int();
    if (!count || count > fields.elements) {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), order_item->full_name(), thd->where);
      return true;
    }
    order->item = &ref_item_array[count - 1];
    order->in_field_list = true;
    order->is_position = true;
    return false;
  }
  /* Lookup the current GROUP/ORDER field in the SELECT clause. */
  select_item = find_item_in_list(thd, order_item, fields, &counter,
                                  REPORT_EXCEPT_NOT_FOUND, &resolution);
  if (!select_item)
    return true; /* The item is not unique, or some other error occurred. */

  /* Check whether the resolved field is not ambiguos. */
  if (select_item != not_found_item) {
    Item *view_ref = nullptr;
    /*
      If we have found field not by its alias in select list but by its
      original field name, we should additionally check if we have conflict
      for this name (in case if we would perform lookup in all tables).
    */
    if (resolution == RESOLVED_BEHIND_ALIAS && !order_item->fixed &&
        order_item->fix_fields(thd, order->item))
      return true;

    /*
      Lookup the current GROUP or WINDOW partition by or order by field in the
      FROM clause.
    */
    order_item_type = order_item->type();
    from_field = not_found_field;
    if (((is_group_field || is_window_order) &&
         order_item_type == Item::FIELD_ITEM) ||
        order_item_type == Item::REF_ITEM) {
      from_field =
          find_field_in_tables(thd, (Item_ident *)order_item, tables, nullptr,
                               &view_ref, IGNORE_ERRORS, true, false);
      if (thd->is_error()) return true;

      if (!from_field) from_field = not_found_field;
    }

    if (from_field == not_found_field ||
        (from_field != view_ref_found
             ?
             /* it is field of base table => check that fields are same */
             ((*select_item)->type() == Item::FIELD_ITEM &&
              ((Item_field *)(*select_item))->field->eq(from_field))
             :
             /*
               in is field of view table => check that references on translation
               table are same
             */
             ((*select_item)->type() == Item::REF_ITEM &&
              view_ref->type() == Item::REF_ITEM &&
              ((Item_ref *)(*select_item))->ref ==
                  ((Item_ref *)view_ref)->ref))) {
      /*
        If there is no such field in the FROM clause, or it is the same field
        as the one found in the SELECT clause, then use the Item created for
        the SELECT field. As a result if there was a derived field that
        'shadowed' a table field with the same name, the table field will be
        chosen over the derived field.

        If we replace *order->item with one from the select list or
        from a table in the FROM list, we should clean up after
        removing the old *order->item from the query. The item has not
        been fixed (so there are no aggregation functions that need
        cleaning up), but it may contain subqueries that should be
        unlinked.
      */
      if ((*order->item)->real_item() != *select_item)
        (*order->item)
            ->walk(&Item::clean_up_after_removal, enum_walk::SUBQUERY_POSTFIX,
                   nullptr);
      order->item = &ref_item_array[counter];
      order->in_field_list = true;
      if (resolution == RESOLVED_AGAINST_ALIAS && from_field == not_found_field)
        order->used_alias = true;
      return false;
    } else {
      /*
        There is a field with the same name in the FROM clause. This
        is the field that will be chosen. In this case we issue a
        warning so the user knows that the field from the FROM clause
        overshadows the column reference from the SELECT list.
        For window functions we do not need to issue this warning
        (field should resolve to a unique column in the FROM derived
        table expression, cf. SQL 2016 section 7.15 SR 4)
      */
      if (!is_window_order) {
        push_warning_printf(thd, Sql_condition::SL_WARNING, ER_NON_UNIQ_ERROR,
                            ER_THD(thd, ER_NON_UNIQ_ERROR),
                            ((Item_ident *)order_item)->field_name, thd->where);
      }
    }
  }

  order->in_field_list = false;
  /*
    The call to order_item->fix_fields() means that here we resolve
    'order_item' to a column from a table in the list 'tables', or to
    a column in some outer query. Exactly because of the second case
    we come to this point even if (select_item == not_found_item),
    inspite of that fix_fields() calls find_item_in_list() one more
    time.

    We check order_item->fixed because Item_func_group_concat can put
    arguments for which fix_fields already was called.

    group_fix_field= true is to resolve aliases from the SELECT list
    without creating of Item_ref-s: JOIN::exec() wraps aliased items
    in SELECT list with Item_copy items. To re-evaluate such a tree
    that includes Item_copy items we have to refresh Item_copy caches,
    but:
      - filesort() never refresh Item_copy items,
      - end_send_group() checks every record for group boundary by the
        test_if_group_changed function that obtain data from these
        Item_copy items, but the copy_fields function that
        refreshes Item copy items is called after group boundaries only -
        that is a vicious circle.
    So we prevent inclusion of Item_copy items.
  */
  bool save_group_fix_field = thd->lex->current_select()->group_fix_field;
  if (is_group_field) thd->lex->current_select()->group_fix_field = true;
  bool ret =
      (!order_item->fixed && (order_item->fix_fields(thd, order->item) ||
                              (order_item = *order->item)->check_cols(1)));
  thd->lex->current_select()->group_fix_field = save_group_fix_field;
  if (ret) return true; /* Wrong field. */

  uint el = all_fields.elements;
  all_fields.push_front(order_item); /* Add new field to field list. */
  ref_item_array[el] = order_item;
  /*
    If the order_item is a SUM_FUNC_ITEM, when fix_fields is called
    referenced_by is set to order->item which is the address of order_item.
    But this needs to be address of order_item in the all_fields list.
    As a result, when it gets replaced with Item_aggregate_ref
    object in Item::split_sum_func2, we will be able to retrieve the
    newly created object.
  */
  if (order_item->type() == Item::SUM_FUNC_ITEM)
    ((Item_sum *)order_item)->referenced_by[0] = all_fields.head_ref();

  /*
    Currently, we assume that this assertion holds. If it turns out
    that it fails for some query, order->item has changed and the old
    item is removed from the query. In that case, we must call walk()
    with clean_up_after_removal() on the old order->item.
  */
  DBUG_ASSERT(order_item == *order->item);
  order->item = &ref_item_array[el];
  return false;
}

/**
  Resolve and setup list of expressions in ORDER BY clause.

  Change order to point at item in select list.
  If item isn't a number and doesn't exists in the select list, add it to the
  the field list.

  @param thd            Current session.
  @param ref_item_array The Ref_item_array for this query block.
  @param tables         From clause of the query.
  @param fields         Selected columns.
  @param all_fields     All columns, including hidden ones.
  @param order          The query block's order clause.

  @returns false if success, true if error.
*/

bool setup_order(THD *thd, Ref_item_array ref_item_array, TABLE_LIST *tables,
                 List<Item> &fields, List<Item> &all_fields, ORDER *order) {
  DBUG_TRACE;

  DBUG_ASSERT(order);

  SELECT_LEX *const select = thd->lex->current_select();

  thd->where = "order clause";

  const bool for_union = select->master_unit()->is_union() &&
                         select == select->master_unit()->fake_select_lex;
  const bool is_aggregated = select->is_grouped();

  for (uint number = 1; order; order = order->next, number++) {
    if (find_order_in_list(thd, ref_item_array, tables, order, fields,
                           all_fields, false, false))
      return true;
    if ((*order->item)->has_aggregation()) {
      /*
        Aggregated expressions in ORDER BY are not supported by SQL standard,
        but MySQL has some limited support for them. The limitations are
        checked below:

        1. A UNION query is not aggregated, so ordering by a set function
           is always wrong.
      */
      if (for_union) {
        my_error(ER_AGGREGATE_ORDER_FOR_UNION, MYF(0), number);
        return true;
      }

      /*
        2. A non-aggregated query combined with a set function in ORDER BY
           that does not contain an outer reference is illegal, because it
           would cause the query to become aggregated.
           (Since is_aggregated is false, this expression would cause
            agg_func_used() to become true).
      */
      if (!is_aggregated && select->agg_func_used()) {
        my_error(ER_AGGREGATE_ORDER_NON_AGG_QUERY, MYF(0), number);
        return true;
      }
    }
    if (for_union && (*order->item)->has_wf()) {
      // Window function in ORDER BY of UNION not supported, SQL2014 4.16.3
      my_error(ER_AGGREGATE_ORDER_FOR_UNION, MYF(0), number);
      return true;
    }
  }
  return false;
}

/**
   Runs checks mandated by ONLY_FULL_GROUP_BY

   @param  thd                     THD pointer

   @returns true if ONLY_FULL_GROUP_BY is violated.
*/

bool SELECT_LEX::check_only_full_group_by(THD *thd) {
  bool rc = false;

  if (is_grouped()) {
    MEM_ROOT root;
    /*
      "root" has very short lifetime, and should not consume much
      => not instrumented.
    */
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &root, MEM_ROOT_BLOCK_SIZE, 0);
    {
      Group_check gc(this, &root);
      rc = gc.check_query(thd);
      gc.to_opt_trace(thd);
    }  // scope, to let any destructor run before free_root().
    free_root(&root, MYF(0));
  }

  if (!rc && is_distinct()) {
    Distinct_check dc(this);
    rc = dc.check_query(thd);
  }

  return rc;
}

/**
  Do final setup of ORDER BY clause, after the query block is fully resolved.

  Check that ORDER BY clause is not redundant.
  Split any aggregate functions.

  @param thd                      Thread handler

  @returns false if success, true if error
*/
bool SELECT_LEX::setup_order_final(THD *thd) {
  DBUG_TRACE;
  if (is_implicitly_grouped()) {
    // Result will contain zero or one row - ordering is redundant
    empty_order_list(this);
    return false;
  }

  if ((master_unit()->is_union() || master_unit()->fake_select_lex) &&
      this != master_unit()->fake_select_lex && !explicit_limit) {
    // Part of UNION which requires global ordering may skip local order
    empty_order_list(this);
    return false;
  }

  for (ORDER *ord = order_list.first; ord; ord = ord->next) {
    Item *const item = *ord->item;

    const bool is_grouped_aggregate =
        (item->type() == Item::SUM_FUNC_ITEM && !item->m_is_window_function);
    if (is_grouped_aggregate) continue;

    if (item->has_aggregation() ||
        (!item->m_is_window_function && item->has_wf())) {
      item->split_sum_func(thd, base_ref_items, all_fields);
      if (thd->is_error()) return true; /* purecov: inspected */
    }
  }
  return false;
}

/**
  Resolve and set up the GROUP BY list.

  @param thd			Thread handler

  @todo
    change ER_WRONG_FIELD_WITH_GROUP to more detailed
    ER_NON_GROUPING_FIELD_USED

  @returns false if success, true if error
*/

bool SELECT_LEX::setup_group(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(group_list.elements);

  thd->where = "group statement";
  for (ORDER *group = group_list.first; group; group = group->next) {
    if (find_order_in_list(thd, base_ref_items, get_table_list(), group,
                           fields_list, all_fields, true, false))
      return true;

    Item *item = *group->item;
    if (item->has_aggregation() || item->has_wf()) {
      my_error(ER_WRONG_GROUP_FIELD, MYF(0), (*group->item)->full_name());
      return true;
    }

    else if (item->has_grouping_func()) {
      my_error(ER_WRONG_GROUP_FIELD, MYF(0), "GROUPING function");
      return true;
    }
  }

  if (olap == ROLLUP_TYPE && resolve_rollup(thd))
    return true; /* purecov: inspected */

  return false;
}

/****************************************************************************
 ROLLUP handling
 ****************************************************************************/

/**
  Minion of change_group_ref_for_func and change_group_ref_for_cond. Does the
  brunt of the work: checks whether a function or condition contains a
  reference to a grouped expression, and if so, creates an Item_ref to it and
  replaces the reference to the condition with that reference. Marks the
  expression tree as containing a rolled up expression.
*/
static bool find_and_change_grouped_expr(
    THD *thd, SELECT_LEX *select, uint i, Item *func_or_cond, Item *item,
    bool wf, bool *arg_changed, std::function<void(Item *)> update_functor) {
  const bool is_grouping_func =
      (wf ? false
          : (func_or_cond->type() == Item::FUNC_ITEM &&
             down_cast<Item_func *>(func_or_cond)->functype() ==
                 Item_func::GROUPING_FUNC));

  bool found_match = false;
  for (ORDER *group = select->group_list.first; group; group = group->next) {
    Item *real_item = item->real_item();
    if (real_item->eq((*group->item)->real_item(), false)) {
      // If to-be-replaced Item is alias, make replacing Item an alias.
      bool alias_of_expr = (item->type() == Item::FIELD_ITEM ||
                            item->type() == Item::REF_ITEM) &&
                           down_cast<Item_ident *>(item)->is_alias_of_expr();
      Item_ref *new_item;
      if (!(new_item = new Item_ref(&select->context, group->item, nullptr,
                                    item->item_name.ptr(), alias_of_expr)))
        return true; /* purecov: inspected */

      update_functor(new_item);
      new_item->set_rollup_expr();
      found_match = true;
      break;
    }
  }

  if (is_grouping_func && !found_match) {
    my_error(ER_FIELD_IN_GROUPING_NOT_GROUP_BY, MYF(0), (i + 1));
    return true;
  }

  if (found_match) {
    *arg_changed = true;
  } else {
    Item *real_item = item->real_item();
    if (real_item->type() == Item::FUNC_ITEM ||
        (real_item->type() == Item::SUM_FUNC_ITEM &&
         real_item->m_is_window_function)) {
      if (select->change_group_ref_for_func(thd, real_item, arg_changed))
        return true;
    } else if (real_item->type() == Item::COND_ITEM) {
      if (select->change_group_ref_for_cond(
              thd, down_cast<Item_cond *>(real_item), arg_changed))
        return true;
    }
  }
  return false;
}

/**
  Replace occurrences of group by fields in a functions's arguments by ref
  items.

  The method replaces such occurrences of group by expressions by ref objects
  for these expressions unless they are under aggregate functions.  The
  function also corrects the value of the maybe_null attribute for the items of
  all subexpressions containing group by expressions.

  Similarly, replace occurrences of group by expressions in arguments of a
  windowing function with ref items.

  It also checks if expressions in the GROUPING function are present in GROUP
  BY list. This cannot be pushed to Item_func_grouping::fix_fields as GROUP BY
  expressions get resolved at the end. And it cannot be checked later in
  Item_func_grouping::aggregate_check_group as we replace all occurrences of
  GROUP BY expressions with ref items.  As a result, we cannot compare the
  objects for equality.

  @b EXAMPLES
    @code
      SELECT a+1 FROM t1 GROUP BY a WITH ROLLUP
      SELECT SUM(a)+a FROM t1 GROUP BY a WITH ROLLUP
      SELECT a+1, GROUPING(a) FROM t1 GROUP BY a WITH ROLLUP;
  @endcode

  @b IMPLEMENTATION

    The function recursively traverses the tree of function's arguments, looks
    for occurrences of the group by expression that are not under aggregate
    functions and replaces them for the corresponding ref items.  It works
    recursively in conjunction with the companion method
    change_group_ref_for_cond which handles operands of conditions (as opposed
    to function arguments).

  @note
    This substitution is needed GROUP BY queries with ROLLUP if
    SELECT list contains expressions over group by attributes.

  @b EXAMPLE
    @code
      SELECT LAG(f1+3/2,1,1) OVER (ORDER BY f1) FROM t GROUP BY f1
      WITH ROLLUP
  @endcode

  @param thd                  reference to the context
  @param func                 function to make replacement
  @param [out] changed  returns true if item contains a replaced field item

  @todo
    Some functions are not null-preserving. For those functions
    updating of the maybe_null attribute is an overkill.

  @returns false if success, true if error

*/
bool SELECT_LEX::change_group_ref_for_func(THD *thd, Item *func,
                                           bool *changed) {
  bool arg_changed = false;
  bool wf = func->m_is_window_function;
  Item_sum *window_func = wf ? down_cast<Item_sum *>(func) : nullptr;
  Item_func *func_item = !wf ? down_cast<Item_func *>(func) : nullptr;
  uint argcnt = wf ? window_func->get_arg_count() : func_item->arg_count;
  Item **args = (wf ? window_func->get_arg_ptr(0)
                    : (argcnt > 0 ? func_item->arguments() : nullptr));

  for (uint i = 0; i < argcnt; i++) {
    Item *const item = args[i];

    if (wf) {
      if (find_and_change_grouped_expr(thd, this, i, func, item, wf,
                                       &arg_changed,
                                       [=](Item *new_item) -> void {
                                         window_func->set_arg(i, thd, new_item);
                                       }))
        return true;
    } else {
      if (find_and_change_grouped_expr(
              thd, this, i, func, item, wf, &arg_changed,
              [=](Item *new_item) -> void {
                func_item->replace_argument(thd, args + i, new_item);
              }))
        return true;
    }
  }
  if (arg_changed) {
    func->maybe_null = true;
    *changed = true;
  }
  return false;
}

/**
  Similar to change_group_ref_for_func, except we are looking into an AND or OR
  conditions instead of functions' arguments.  It works recursively in
  conjunction with change_group_ref_for_func.

  @b EXAMPLE
    @code
      SELECT FROM t1 GROUP BY a WITH ROLLUP HAVING foo(a) OR bar(a)
  @endcode

  @param thd            session context
  @param cond_item      function to make replacement in
  @param [out] changed  set to true if we replaced a group item with a
                        reference, otherwise not touched, so needs
                        initialization
  @return true on error
*/
bool SELECT_LEX::change_group_ref_for_cond(THD *thd, Item_cond *cond_item,
                                           bool *changed) {
  bool arg_changed = false;
  List_iterator<Item> li(*cond_item->argument_list());
  for (Item *item = li++; item != nullptr; item = li++) {
    if (find_and_change_grouped_expr(
            thd, this, 0, cond_item, item, false, &arg_changed,
            [thd, &li](Item *new_item) -> void {
              thd->change_item_tree(li.ref(), new_item);
            }))
      return true;
  }
  if (arg_changed) {
    cond_item->maybe_null = true;
    *changed = true;
  }
  return false;
}

/**
  Resolve an item (and its tree) for rollup processing by replacing fields with
  references and updating properties (maybe_null, PROP_ROLLUP_FIELD).
  Also check any GROUPING function for incorrect column.

  @param   thd      session context
  @param   item     the item to be processed
  @returns true on error
*/
bool SELECT_LEX::resolve_rollup_item(THD *thd, Item *item) {
  bool found_in_group = false;

  for (ORDER *group = group_list.first; group; group = group->next) {
    /*
      If this item is present in GROUP BY clause, set maybe_null
      to true as ROLLUP will generate NULL's for this column.
    */
    if (*group->item == item || item->eq(*group->item, false)) {
      item->maybe_null = true;
      /*
        If this is a reference, e.g a view column, we need the column to be
        marked as nullable also, since this will form the basis of temporary
        table fields.  Copy_field's from_null_ptr, to_null_ptr will be
        missing if the Item_field isn't marked correctly, which will cause
        problems if we have buffered windowing.
      */
      item->real_item()->maybe_null = true;
      found_in_group = true;
      break;
    }
  }

  if (!found_in_group) {
    bool changed = false;
    if (item->type() == Item::FUNC_ITEM) {
      if (change_group_ref_for_func(thd, item, &changed))
        return true; /* purecov: inspected */
    } else if (item->type() == Item::COND_ITEM) {
      if (change_group_ref_for_cond(thd, down_cast<Item_cond *>(item),
                                    &changed))
        return true; /* purecov: inspected */
    }
    if (changed) item->update_used_tables();
  }

  return false;
}

/**
  Resolve items in SELECT list and ORDER BY list for rollup processing

  @param   thd   session context

  @returns false if success, true if error
*/

bool SELECT_LEX::resolve_rollup(THD *thd) {
  DBUG_TRACE;
  for (Item &item : all_fields) {
    if (resolve_rollup_item(thd, &item)) return true;
  }

  /*
    ORDER BY items haven't been induced into select list yet, so need to
    process these items too
  */

  // Allow local set functions in ORDER BY
  const bool saved_allow = thd->lex->allow_sum_func;
  thd->lex->allow_sum_func |= (nesting_map)1 << nest_level;
  thd->where = "order clause";

  for (ORDER *order = order_list.first; order; order = order->next) {
    Item *order_item = *order->item;

    order->in_field_list = false;
    bool ret =
        (!order_item->fixed && (order_item->fix_fields(thd, order->item) ||
                                (order_item = *order->item)->check_cols(1)));
    if (ret) return true; /* Wrong field. */

    if (resolve_rollup_item(thd, order_item)) return true;
  }

  thd->lex->allow_sum_func = saved_allow;
  return false;
}

/**
  Replace group by field references inside window functions with references
  in the the presence of ROLLUP.

  @param   thd   session context
  @returns false if success, true if error
*/

bool SELECT_LEX::resolve_rollup_wfs(THD *thd) {
  DBUG_TRACE;
  for (Item &item : all_fields) {
    if (resolve_rollup_item(thd, &item)) return true;
    if (item.type() == Item::SUM_FUNC_ITEM && item.m_is_window_function) {
      bool changed = false;
      if (change_group_ref_for_func(thd, &item, &changed))
        return true; /* purecov: inspected */
      if (changed) item.update_used_tables();
    }
  }
  /*
    When this method is called from setup_windows, all ORDER BY items not
    already present in the SELECT list have been added to the select list as
    hidden items, so we do not need to traverse order_list to see all
    items. The companion method, resolve_rollup, needs to traverse order_list
    list, because at the the time that method is called, the ORDER BY
    itms haven't been added yet. Cf second loop in resolve_rollup.
  */

  return false;
}
/**
  @brief  validate_gc_assignment
  Check whether the other values except DEFAULT are assigned
  for generated columns.

  @param fields                     Item_fields list to be filled
  @param values                     values to fill with
  @param table                      table to be checked
  @return Operation status
    @retval false   OK
    @retval true    Error occurred

  @note  This function must be called after table->write_set has been
         filled.
*/
bool validate_gc_assignment(List<Item> *fields, List<Item> *values,
                            TABLE *table) {
  Field **fld = nullptr;
  MY_BITMAP *bitmap = table->write_set;
  bool use_table_field = false;
  DBUG_TRACE;

  if (!values || (values->elements == 0)) return false;

  // If fields has no elements, we use all table fields
  if (fields->elements == 0) {
    use_table_field = true;
    fld = table->field;
  }
  List_iterator_fast<Item> f(*fields), v(*values);
  Item *value;
  while ((value = v++)) {
    const Field *rfield;

    if (!use_table_field)
      rfield = (down_cast<Item_field *>((f++)->real_item()))->field;
    else
      rfield = *(fld++);
    if (rfield->table != table) continue;

    // Skip fields that are hidden from the user.
    if (rfield->is_hidden_from_user()) continue;

    // If any of the explicit values is DEFAULT
    if (rfield->m_default_val_expr &&
        value->type() == Item::DEFAULT_VALUE_ITEM) {
      // Restore the statement safety flag to current lex
      table->in_use->lex->set_stmt_unsafe_flags(
          rfield->m_default_val_expr->get_stmt_unsafe_flags());
      // Mark the columns that this expression reads to rthe ead_set
      for (uint j = 0; j < table->s->fields; j++) {
        if (bitmap_is_set(&rfield->m_default_val_expr->base_columns_map, j)) {
          bitmap_set_bit(table->read_set, j);
        }
      }
    }

    /* skip non marked fields */
    if (!bitmap_is_set(bitmap, rfield->field_index)) continue;
    if (rfield->gcol_info && value->type() != Item::DEFAULT_VALUE_ITEM) {
      my_error(ER_NON_DEFAULT_VALUE_FOR_GENERATED_COLUMN, MYF(0),
               rfield->field_name, rfield->table->s->table_name.str);
      return true;
    }
  }
  return false;
}

/**
  Delete unused columns from merged tables.

  This function is called recursively for each join nest and/or table
  in the query block. For each merged table that it finds, each column
  that contains a subquery and is not marked as used is removed and
  the translation item is set to NULL.

  @param tables List of tables and join nests
*/

void SELECT_LEX::delete_unused_merged_columns(
    mem_root_deque<TABLE_LIST *> *tables) {
  DBUG_TRACE;

  for (TABLE_LIST *tl : *tables) {
    if (tl->nested_join == nullptr) continue;
    if (tl->is_merged()) {
      for (Field_translator *transl = tl->field_translation;
           transl < tl->field_translation_end; transl++) {
        Item *const item = transl->item;

        DBUG_ASSERT(item->fixed);
        if (!item->has_subquery()) continue;

        /*
          All used columns selected from derived tables are already marked
          as such. But unmarked columns may still refer to other columns
          from underlying derived tables, and in that case we cannot
          delete these columns as they share the same items.
          Thus, dive into the expression and mark such columns as "used".
          (This is a bit incorrect, as only a part of its underlying expression
          is "used", but that has no practical meaning.)
        */
        if (!item->is_derived_used() &&
            item->walk(&Item::propagate_derived_used, enum_walk::POSTFIX,
                       nullptr))
          item->walk(&Item::propagate_set_derived_used,
                     enum_walk::SUBQUERY_POSTFIX, nullptr);

        if (!item->is_derived_used()) {
          Item::Cleanup_after_removal_context ctx(this);
          item->walk(&Item::clean_up_after_removal, enum_walk::SUBQUERY_POSTFIX,
                     pointer_cast<uchar *>(&ctx));
          transl->item = nullptr;
        }
      }
    }
    delete_unused_merged_columns(&tl->nested_join->join_list);
  }
}

/**
  Add item to the hidden part of select list.

  @param item  item to add

  @return Pointer to reference to the added item
*/

Item **SELECT_LEX::add_hidden_item(Item *item) {
  const uint el = all_fields.elements;
  base_ref_items[el] = item;
  all_fields.push_front(item);
  return &base_ref_items[el];
}

/**
  Resolve the rows of a table value constructor and aggregate the type of each
  column across rows.

  @param thd    thread handler

  @returns false if success, true if error
*/

bool SELECT_LEX::resolve_table_value_constructor_values(THD *thd) {
  // Item_values_column objects may be allocated; they should be persistent for
  // PREPARE statements.
  Prepared_stmt_arena_holder ps_arena_holder(thd);

  size_t num_rows = row_value_list->size();
  size_t row_degree = row_value_list->head()->size();

  // All table row value expressions shall be of the same degree. Note that
  // non-scalar subqueries are not allowed; we can simply count the number of
  // elements.
  if (row_degree > MAX_FIELDS) {
    my_error(ER_TOO_MANY_FIELDS, MYF(0));
    return true;
  }

  size_t row_index = 0;
  for (List<Item> &values_row : *row_value_list) {
    if (values_row.size() != row_degree) {
      my_error(ER_WRONG_VALUE_COUNT_ON_ROW, MYF(0), row_index + 1);
      return true;
    } else if (values_row.size() == 0) {
      // A table value constructor with empty row objects is a syntax error,
      // except when used as the source for an INSERT statement.
      my_error(ER_TABLE_VALUE_CONSTRUCTOR_MUST_HAVE_COLUMNS, MYF(0));
      return true;
    }

    size_t item_index = 0;
    Item *item;
    List_iterator<Item> it(values_row);
    while ((item = it++)) {
      if ((!item->fixed && item->fix_fields(thd, it.ref())) ||
          (item = *(it.ref()))->check_cols(1))
        return true; /* purecov: inspected */

      if (item->type() == Item::DEFAULT_VALUE_ITEM) {
        my_error(ER_TABLE_VALUE_CONSTRUCTOR_CANNOT_HAVE_DEFAULT, MYF(0));
        return true;
      }

      if (row_index == 0) {
        // If single row, we skip setting up indirections.
        if (num_rows != 1 && first_execution) {
          Item_values_column *column = new Item_values_column(thd, item);
          if (column == nullptr) return true;
          column->add_used_tables(item);
          item = column;
        }
        // Make sure to also replace the reference in item_list. In the case
        // where fix_fields transforms an item, it.ref() will only update the
        // reference of values_row.
        if (first_execution) item_list.replace(item_index, item);
      } else {
        Item_values_column *column =
            down_cast<Item_values_column *>(item_list[item_index]);
        if (column->join_types(thd, item)) return true;
        column->add_used_tables(item);
        column->fixed = true;  // Does not have regular fix_fields()
      }

      ++item_index;
    }

    ++row_index;
  }

  // base_ref_items is used during row_value_in_to_exists_transformer to set up
  // equality checks when transforming IN subquery predicates.
  if (setup_base_ref_items(thd)) return true;

  size_t name_len;
  char buff[NAME_LEN + 1];
  if (check_stack_overrun(thd, STACK_MIN_SIZE, pointer_cast<uchar *>(buff)))
    return true; /* purecov: inspected */

  size_t item_index = 0;
  for (Item &column : item_list) {
    base_ref_items[item_index] = &column;

    // Name the columns column_0, column_1, ...
    name_len = snprintf(buff, NAME_LEN, "column_%zu", item_index);
    column.item_name.copy(buff, name_len);

    ++item_index;
  }

  return false;
}

/**
  Create a new TABLE_LIST object for this query block, for either:
  1) a derived table which will replace the scalar subquery, or
  2) an extra derived table for handling grouping, if necessary,
     cf. transform_grouped_to_derived.

  The derived table is added to the list of used tables for the query block
  ("outer").

  @param     thd        the session context
  @param     unit       the query expression for subquery (case 1), or a new
                        query expression for (case 2)
  @param     join_cond  != nullptr: we are  synthesizing a derived table for a
                        subquery within this join condition
                        = nullptr: synthesizing a derived table for a subquery
                        where the subquery is not contained in a join condition
  @param     i          counter used to name the new derived table uniquely
                        Combining the SELECT number and this number hopefully
                        makes the name of the derived table unique.
                        We use the form "derived_\<select-#\>_\<i\>.
                        The reason we need the select number as well is because
                        subquery flattening (e.g. semijoin) can collapse two
                        query block levels, making only it non-unique in the
                        scope.
  @param     left_outer true for case (1), false for (2)
  @returns the derived table object, or nullptr on error.
*/
TABLE_LIST *SELECT_LEX::synthesize_derived(THD *thd, SELECT_LEX_UNIT *unit,
                                           Item *join_cond, uint i,
                                           bool left_outer) {
  char name[STRING_BUFFER_USUAL_SIZE];
  std::snprintf(name, sizeof(name), "derived_%d_%d", select_number, i);
  char *namep = thd->mem_strdup(name);
  if (namep == nullptr) return nullptr;

  auto *const ti = new (thd->mem_root) Table_ident(unit);
  if (ti == nullptr) return nullptr;

  TABLE_LIST *derived_table =
      add_table_to_list(thd, ti, namep, 0, TL_READ, MDL_SHARED_READ);
  if (derived_table == nullptr) return nullptr;

  if (left_outer) {
    derived_table->outer_join = JOIN_TYPE_LEFT;
    derived_table->m_was_scalar_subquery = true;
  } else {
    derived_table->m_was_grouped2derived = true;
  }

  if (left_outer) {
    if (join_cond != nullptr) {
      // The derived table is for a subquery in a join condition

      // peel off any semi-joins and save
      mem_root_deque<TABLE_LIST *> sj(thd->mem_root);
      uint siz = join_list->size();
      for (uint j = 0; j < siz - 1; j++) {
        DBUG_ASSERT(join_list->front()->is_sj_nest());
        sj.push_back(join_list->front());
        join_list->pop_front();
      }

      TABLE_LIST *old_nest = join_list->front();
      DBUG_ASSERT(!old_nest->is_sj_or_aj_nest());
      join_list->pop_front();
      DBUG_ASSERT(join_list->size() == 0);
      TABLE_LIST *new_nest = nest_derived(
          thd, join_cond, &old_nest->nested_join->join_list, derived_table);
      if (new_nest == nullptr) return nullptr;

      // add back any semi-joins
      while (sj.size() > 0) {
        if (add_joined_table(sj.back())) return nullptr;
        sj.pop_back();
      }
    } else {
      // The derived table is not for a subquery in a join condition
      if (add_joined_table(derived_table)) return nullptr;
      if (nest_last_join(thd) == nullptr) return nullptr;
    }

    auto *const join_cond_true = new (thd->mem_root) Item_func_true();
    if (join_cond_true == nullptr) return nullptr;
    derived_table->set_join_cond(join_cond_true);
  }

  unit->derived_table = derived_table;
  return derived_table;
}

/**
  Remove a derived table we added previously as part of
  transform_scalar_subqueries_to_derived. This can happen when
  the transformed scalar subquery is part of a view that
  is not used in a query block referencing the view, e.g. if the view
  has a scalar subquery in the select list and this field is not referenced
  by the query invoking the view.

  @param thd      Session state
  @param tl       The derived table that should be removed
*/
void SELECT_LEX::remove_derived(THD *thd, TABLE_LIST *tl) {
  // Remove from leaf_tables
  materialized_derived_table_count--;
  derived_table_count--;

  TABLE_LIST **leafp = &leaf_tables;
  while (*leafp != nullptr) {
    if (*leafp == tl) {
      *leafp = (*leafp)->next_leaf;
      break;
    }
    leafp = &(*leafp)->next_leaf;
  }
  // Remove query expression from this block's set of query expressions
  SELECT_LEX_UNIT **unitp = &slave;
  while (*unitp != nullptr) {
    if (*unitp == tl->derived_unit()) {
      *unitp = (*unitp)->next;
      if (*unitp != nullptr) {
        (*unitp)->prev = unitp;
      }
      break;
    }
    unitp = &(*unitp)->next;
  }
  // Remove derived table's query block from global list
  SELECT_LEX **qbp = &thd->lex->all_selects_list;
  while (*qbp != nullptr) {
    if (*qbp == tl->derived_unit()->first_select()) {
      *qbp = (*qbp)->link_next;
      if (*qbp != nullptr) {
        (*qbp)->link_prev = qbp;
      }
      break;
    }
    qbp = &(*qbp)->link_next;
  }
}

/// Auxiliary method for SELECT_LEX::field_of_table_present
static bool find_in_join_conditions(mem_root_deque<TABLE_LIST *> &list,
                                    std::function<bool(Item **expr_p)> action) {
  for (TABLE_LIST *tl : list) {
    if (tl->join_cond() != nullptr) {
      if (action(tl->join_cond_ref())) return true;
    }
    if (tl->nested_join != nullptr &&
        find_in_join_conditions(tl->nested_join->join_list, action))
      return true; /* purecov: inspected */
  }
  return false;
}

/**
  Find if the current block contains any reference to a field in
  table tl.

  @param tl          the table inquestion
  @param[out] found  set to true if a field of tl is found to be referenced

  @returns true on error
*/
bool SELECT_LEX::field_of_table_present(TABLE_LIST *tl, bool *found) {
  Item *i;
  List_iterator<Item> li(all_fields);
  while ((i = li++)) {
    Item *ri = i->real_item();
    if (ri->type() == Item::FIELD_ITEM &&
        down_cast<Item_field *>(ri)->table_ref == tl) {
      *found = true;
      return false;
    }
  }

  List<Item> fields;
  if (m_where_cond != nullptr &&
      m_where_cond->walk(&Item::collect_item_field_processor,
                         enum_walk::POSTFIX, pointer_cast<uchar *>(&fields)))
    return true; /* purecov: inspected */

  if (having_cond() != nullptr) {
    if (having_cond()->walk(&Item::collect_item_field_processor,
                            enum_walk::POSTFIX, pointer_cast<uchar *>(&fields)))
      return true; /* purecov: inspected */
  }

  if (find_in_join_conditions(
          top_join_list, [&fields](Item **expr_p) mutable -> bool {
            if ((*expr_p)->walk(&Item::collect_item_field_processor,
                                enum_walk::POSTFIX,
                                pointer_cast<uchar *>(&fields)))
              return true; /* purecov: inspected */
            return false;
          }))
    return true; /* purecov: inspected */

  List_iterator<Window> liw(m_windows);
  for (Window *w = liw++; w != nullptr; w = liw++) {
    for (ORDER *it : {w->first_order_by(), w->first_partition_by()}) {
      if (it != nullptr) {
        for (auto ord = it; ord != nullptr; ord = ord->next) {
          if ((*ord->item)
                  ->walk(&Item::collect_item_field_processor,
                         enum_walk::POSTFIX, pointer_cast<uchar *>(&fields)))
            return true; /* purecov: inspected */
        }
      }
    }
  }

  li.rewind();
  li.init(fields);
  while ((i = li++)) {
    if (down_cast<Item_field *>(i)->table_ref == tl) {
      *found = true;
      return false;
    }
  }

  return false;
}

/**
  A minion of transform_grouped_to_derived.

  Replace occurrences of the aggregate function identified in info.m_target with
  the the field info.m_replacement in the expressions contained in list.
  Note that since this is part of a permanent transformation, we use the extra
  m_permanent_transform flag in the THD

  @param info  a tuple containing {aggregate, replacement field}
  @param list  the list of expressions

  @returns true on error (can not happen currently unless replacement field is
                          empty)
*/
static bool replace_aggregate_in_list(Item::Aggregate_replacement &info,
                                      List<Item> &list) {
  List_iterator<Item> lii(list);
  Item *select_expr;
  while ((select_expr = lii++)) {
    THD::Permanent_transform tr(current_thd);  // RAII
    Item *const new_item = select_expr->transform(&Item::replace_aggregate,
                                                  pointer_cast<uchar *>(&info));
    if (new_item == nullptr) return true;
    new_item->update_used_tables();
    if (new_item != select_expr) *lii.ref() = new_item;
  }
  return false;
}

/**
  A minion of transform_grouped_to_derived.

  "Remove" any non-window aggregate functions from all_fields unconditionally.
  If such an aggregate is found, the query block should have a HAVING clause.
  This is asserted in debug mode. We "remove" them by replacing them with
  an Item_int, which should have no adverse effects. This avoids creating
  trouble for SELECT_LEX::add_hidden_item which would otherwise need to keep
  track of removed items.

  @param thd      session context
  @param select   the query block whose aggregates are being moved into a
                  derived table
  @returns true on error, else false
*/
bool SELECT_LEX::remove_aggregates(THD *thd,
                                   SELECT_LEX MY_ATTRIBUTE((unused)) * select) {
  Item *select_expr;
  List_iterator<Item> lii(all_fields);
  while ((select_expr = lii++)) {
    if (!select_expr->m_is_window_function &&
        select_expr->type() == Item::SUM_FUNC_ITEM) {
      // must be an aggregate induced from a HAVING clause, remove from
      // transformed query block since it is not needed on that
      // level any more
      DBUG_ASSERT(select->having_cond() != nullptr);
      Item *int_item = new (thd->mem_root) Item_int(0);
      if (int_item == nullptr) return true;
      *lii.ref() = int_item;
    }
  }
  return false;
}

/**
  A minion of transform_grouped_to_derived.

  This updates the name resolution contexts in expr to that of new_derived
  permanently. If some of the items in expr have been replaced by a reference
  to a derived table, we update the context of the original item as well, cf.
  Item_view_ref::change_context_processor.

  @param  expr        the expression to be updated
  @param  new_derived the query block of the new derived table which now holds
                      the expression after it has been moved down.

  @returns true on error
*/
static bool update_context_to_derived(Item *expr, SELECT_LEX *new_derived) {
  Item_ident::Change_context ctx(&new_derived->context, true /* permanent */);
  if (expr != nullptr && expr->walk(&Item::change_context_processor,
                                    enum_walk::POSTFIX, (uchar *)&ctx))
    return true; /* purecov: inspected */
  return false;
}

/**
  A minion of transform_grouped_to_derived.

  Collect a unique list of aggregate functions used in the transformed query
  block, which will need to be replaced with fields from the derived table
  containing the grouping during transform_grouped_to_derived.

  @param[in]       select     the query block
  @param[in, out]  aggregates the accumulator which wll contain the aggregates
  @return true on error
*/
static bool collect_aggregates(
    SELECT_LEX *select, Item_sum::Collect_grouped_aggregate_info *aggregates) {
  List_iterator<Item> lii(select->fields_list);
  Item *select_expr;
  while ((select_expr = lii++)) {
    if (select_expr->walk(&Item::collect_grouped_aggregates,
                          enum_walk::SUBQUERY_PREFIX,
                          pointer_cast<uchar *>(aggregates)))
      return true; /* purecov: inspected */
  }

  if (select->having_cond() != nullptr) {
    if (select->having_cond()->walk(&Item::collect_grouped_aggregates,
                                    enum_walk::SUBQUERY_PREFIX,
                                    pointer_cast<uchar *>(aggregates)))
      return true; /* purecov: inspected */
  }
  // We move the aggregate functions from an implicitly grouped query block to
  // a new derived table, effectively making the existing query block
  // non-grouped. When the grouping is implicit, the ORDER BY is eliminated
  // since the result set has only one row, so skip processing of the
  // order_list.
  DBUG_ASSERT(select->order_list.elements == 0);

  List_iterator<Window> li(select->m_windows);
  for (Window *w = li++; w != nullptr; w = li++) {
    for (ORDER *it : {w->first_order_by(), w->first_partition_by()}) {
      if (it != nullptr) {
        for (auto ord = it; ord != nullptr; ord = ord->next) {
          if ((*ord->item)
                  ->walk(&Item::collect_grouped_aggregates, enum_walk::PREFIX,
                         pointer_cast<uchar *>(aggregates)))
            return true; /* purecov: inspected */
        }
      }
    }
  }
  return false;
}

/**
  A minion of transform_grouped_to_derived.

  Fields from the query block containing the scalar subquery are moved
  to the new derived table which will perform the grouping. We give them
  synthetic unique names here. The name given is "d_<field_number>".

  @param thd      current session context
  @param item     the item we want to name
  @param field_no the field number
  @returns true on error
*/
static bool baptize_item(THD *thd, Item *item, int *field_no) {
  char buff[100];
  std::snprintf(buff, sizeof(buff), "d_%d", (*field_no)++);
  char *namep = thd->mem_strdup(buff);
  if (namep == nullptr) return true;
  item->orig_name.set(item->item_name.ptr());
  item->item_name.set(namep);
  return false;
}

/**
  Minion of transform_scalar_subqueries_to_derived. Moves implicit
  grouping down into a derived table to prepare for
  transform_scalar_subqueries_to_derived.

  Example:

  @verbatim

    SELECT (SELECT COUNT(*)
            FROM t1) AS tot,
           IFNULL(MAX(t2.b), 0) + 6 AS mx
    FROM t2
    WHERE expr-2;

  is transformed to ->

    SELECT derived_1_1.`COUNT(*)` AS tot,
           (IFNULL(derived_1_0.`MAX(t2.b)`,0) + 6) AS mx
    FROM (SELECT MAX(t2.b) AS `MAX(t2.b)`
          FROM t2
          WHERE expr-2) derived_1_0
         LEFT JOIN
         (SELECT COUNT(0) AS `COUNT(*)`
          FROM t1) derived_1_1
         ON TRUE;

  @endverbatim

  Create a new query expression object and query block object to represent the
  contents of a derived table ("new_derived" in the code below, "derived1" in
  the example above), with a select list which only contains the aggregate
  functions lifted out of the transformed query block ("MAX(b) AS mx" above) and
  any fields referenced ("i" above).

  The transformed query block retains the original select list except aggregates
  and fields are replaced by fields ("derived1.mx", "derived1.i" above) from the
  new subquery, but it loses its FROM list, replaced by the new derived table
  ("derived1" above) and its WHERE and HAVING clauses which all go to
  the derived table's query block.

  Any DISTINCT, WINDOW clauses and LIMITs stay in place at the transformed
  query block.

  @param      thd        session context
  @param[out] break_off  set to true of transformation could not be performed
  @returns               true on error
*/
bool SELECT_LEX::transform_grouped_to_derived(THD *thd, bool *break_off) {
  // Collect all aggregates, and add them to our new select list
  Item_sum::Collect_grouped_aggregate_info aggregates(this);

  // Remember implicit grouping in case this query is also a scalar subquery
  // so we can still identify it after this transform.
  m_was_implicitly_grouped = is_implicitly_grouped();

  if (collect_aggregates(this, &aggregates)) return true;
  if (aggregates.m_break_off) {
    *break_off = true;  // some aggregates functions aggregate in an outer query
    return false;
  }

  TABLE_LIST *tl = nullptr;
  SELECT_LEX *new_derived = nullptr;
  List<Item> item_fields_or_view_refs;
  std::unordered_map<Field *, Item_field *> unique_fields;
  std::vector<Item_view_ref *> unique_view_refs;
  /*
    In addition to adding the aggregates to the derived table's SELECT list,
    we need to add all referenced fields that will be needed in this query
    block.
    They fall into three categories:

    1) fields referenced directly in the select list
    2) fields referenced by window functions as arguments, or in
       in a window definition's ORDER BY or PARTITION BY clauses
    3) fields referenced by the transformed query block's ORDER BY clause

    All of these can reference items from tables that are now moved inside the
    derived table.

    This query block will get its fields replaced by the corresponding ones in
    the derived table shortly, after we have resolved the derived table.  We
    need to give them unique names in the derived table, else we could have
    issues with resolution. Can probably be removed after WL#6570.

    Method: collect all unique fields referenced in categories 1-3 above.
    Add them with unique names to the SELECT list of the derived table,
    after the aggregates (e.g. inside the derived table one may see t1.i and
    t2.i, but at this level both fields are part of the same derived table,
    so they cannot both be known as i in this query block).

    When the fields in the derived table are known (after the call to
    resolve_placeholder_tables below, we can go back and modify the references
    at this level.
  */
  std::vector<Item **> contrib_exprs;

  // We want permanent changes
  {
    Prepared_stmt_arena_holder ps_arena_holder(thd);

    SELECT_LEX_UNIT *const old_slave = slave;
    slave = nullptr;
    // The new derived table takes over WHERE and HAVING from this query block
    SELECT_LEX_UNIT *new_slu = parent_lex->create_query_expr_and_block(
        thd, this, m_where_cond, m_having_cond, CTX_DERIVED);
    if (new_slu == nullptr) return true;
    new_derived = new_slu->first_select();

    m_where_cond = nullptr;
    m_having_cond = nullptr;
    new_derived->linkage = DERIVED_TABLE_TYPE;

    // inherit item counts for safe allocation of base_ref_items array
    new_derived->select_n_having_items = select_n_having_items;
    new_derived->select_n_where_fields = select_n_where_fields;
    new_derived->n_sum_items = n_sum_items;
    new_derived->n_child_sum_items = n_child_sum_items;

    with_sum_func = false;

    // Any moved Item_ident needs new name resolution context
    Item *conds[2] = {new_derived->m_where_cond, new_derived->m_having_cond};
    for (auto cond : conds) {
      if (update_context_to_derived(cond, new_derived)) return true;
    }

    DBUG_ASSERT(join == nullptr);

    // Move FROM tables under the new derived table with fix ups
    new_derived->table_list = table_list;
    table_list.empty();
    for (TABLE_LIST *tables = new_derived->table_list.first; tables != nullptr;
         tables = tables->next_local) {
      tables->select_lex = new_derived;  // update query block context
      if (update_context_to_derived(tables->join_cond(), new_derived))
        return true; /* purecov: inspected */
    }

    new_derived->derived_table_count = this->derived_table_count;
    derived_table_count = 0;  // will soon become 1.

    DBUG_ASSERT(is_implicitly_grouped());  // only implicit grouping moved
    DBUG_ASSERT(group_list.elements == 0);
    DBUG_ASSERT(olap == UNSPECIFIED_OLAP_TYPE);

    // Let new derived take over grouping flags
    new_derived->m_agg_func_used = m_agg_func_used;
    m_agg_func_used = false;
    new_derived->m_json_agg_func_used = m_json_agg_func_used;
    m_json_agg_func_used = false;

    // Let new derived take over any semijoin candidates
    new_derived->sj_candidates = sj_candidates;
    sj_candidates = nullptr;

    DBUG_ASSERT(join_list == &top_join_list);
    new_derived->top_join_list = std::move(top_join_list);
    top_join_list.clear();
    new_derived->join_list = &new_derived->top_join_list;
    new_derived->leaf_tables = leaf_tables;
    new_derived->leaf_table_count = leaf_table_count;
    leaf_tables = nullptr;
    leaf_table_count = 0;
    // Add the derived table to this query block's FROM list
    tl = synthesize_derived(thd, new_slu, nullptr, 0, false);
    if (tl == nullptr) return true;

    if (!(tl->derived_result = new (thd->mem_root) Query_result_union()))
      return true; /* purecov: inspected */
    new_slu->set_query_result(tl->derived_result);

    top_join_list.push_back(tl);

    // Update this query block's and the derived table's query block's name
    // resolution contexts
    context.table_list = tl;
    context.first_name_resolution_table = tl;
    DBUG_ASSERT(context.last_name_resolution_table == nullptr);
    new_derived->context.init();
    new_derived->context.table_list = table_list.first;
    new_derived->context.select_lex = new_derived;
    new_derived->context.outer_context = &context;
    new_derived->context.first_name_resolution_table = table_list.first;

    /*
      Retain only subqueries from SELECT list in this block [2]; all other
      query expressions go to the new derived table [1]:
    */
    Item_subselect::Collect_subq_info subqueries(this);
    List_iterator<Item> li(all_fields);
    Item *item;
    while ((item = li++)) {
      if (item->walk(&Item::collect_subqueries, enum_walk::PREFIX,
                     pointer_cast<uchar *>(&subqueries)))
        return true; /* purecov: inspected */
    }

    DBUG_ASSERT(slave != nullptr);
    DBUG_ASSERT(new_derived->slave == nullptr);

    // Collect all query expressions in a container first, since we cannot rely
    // on old_slave's ::next pointer chain once we start inserting them.
    std::vector<SELECT_LEX_UNIT *> old_slaves;
    for (SELECT_LEX_UNIT *cand = old_slave; cand != nullptr;
         cand = cand->next) {
      old_slaves.push_back(cand);
    }

    for (auto cand : old_slaves) {
      if (cand == new_slu) continue;  // already in place
      if (subqueries.contains(cand))
        cand->include_down(parent_lex, this);  // [2]
      else {
        cand->include_down(parent_lex, new_derived);  // [1]
        // These subqueries are now moving into a new query block, so we need
        // to update any outer references inside such subqueries from this block
        // to that of the new derived table.
        Item_ident::Depended_change info{this, new_derived};
        if (cand->walk(&Item::update_depended_from, enum_walk::SUBQUERY_PREFIX,
                       pointer_cast<uchar *>(&info)))
          return true; /* purecov: inspected */
      }
    }

    // Insert the aggregates in the derived table's query block
    int i = 0;
    for (Item_sum *agg : aggregates.list) {
      DBUG_ASSERT(agg->aggr_select == agg->base_select);
      agg->aggr_select = new_derived;
      agg->base_select = new_derived;
      if (new_derived->add_item_to_list(agg)) return true;
      if (agg->item_name.length() == 0) {
        // Generate a name (required)
        char buff[100];
        std::snprintf(buff, sizeof(buff), "tmp_aggr_%d", ++i);
        agg->item_name.copy(buff);
        if (agg->item_name.length() == 0) return true;  // allocation error.
      }
    }

    // We will find all fields mentioned above by checking all_fields, which
    // has any hidden fields induced by ORDER BY or window specifications, in
    // addition to fields from the select expressions.
    li.init(all_fields);
    while ((item = li++)) {
      contrib_exprs.push_back(li.ref());
    }

    // Collect fields in expr, but not from inside grouped aggregates.
    Item::Collect_item_fields_or_view_refs info{&item_fields_or_view_refs,
                                                this};
    for (auto expr : contrib_exprs) {
      if ((*expr)->walk(&Item::collect_item_field_or_view_ref_processor,
                        enum_walk::SUBQUERY_PREFIX | enum_walk::POSTFIX,
                        pointer_cast<uchar *>(&info)))
        return true; /* purecov: inspected */
    }

    List_iterator<Item> lfi(item_fields_or_view_refs);
    Item *lf;

    // Remove irrelevant field references, i.e. those fields that are not local
    // to new_derived
    while ((lf = lfi++)) {
      if (lf->type() == Item::FIELD_ITEM) {
        Item_field *f = down_cast<Item_field *>(lf);
        if (!(f->context->select_lex == this || f->depended_from == this))
          lfi.remove();
      }
    }
    // We now have all fields and view refefences; now find only unique ones.
    lfi.init(item_fields_or_view_refs);
    while ((lf = lfi++)) {
      if (lf->type() == Item::FIELD_ITEM) {
        Item_field *f = down_cast<Item_field *>(lf);
        if (unique_fields.find(f->field) == unique_fields.end()) {
          unique_fields.emplace(std::pair<Field *, Item_field *>(f->field, f));
        }
      } else {
        Item_view_ref *vr = down_cast<Item_view_ref *>(lf);
        for (auto curr : unique_view_refs) {
          if (curr->eq(vr, true)) goto continue_outer;
        }
        unique_view_refs.push_back(vr);
      }
    continue_outer:;
    }

    int field_no = 1;

    for (auto vr : unique_view_refs) {
      if (baptize_item(thd, vr, &field_no)) return true;
      if (new_derived->add_item_to_list(vr)) return true;
      // We have moved the view reference to new derived, now make sure it
      // will be rolled back (if at all) to resolve in new_derived too by
      // changing its rollback record, and giving it the correct alias and
      // context
      Item **place = new_derived->item_list.tail_ref();
      Item *old_value = thd->replace_rollback_place(place);
      if (old_value != nullptr) {
        old_value->item_name.set(vr->item_name.ptr());
        down_cast<Item_field *>(old_value)->context = &new_derived->context;
      }
    }

    for (auto pair : unique_fields) {
      if (new_derived->add_item_to_list(pair.second)) return true;
      if (baptize_item(thd, pair.second, &field_no)) return true;
      if (update_context_to_derived(pair.second, new_derived)) return true;
      pair.second->depended_from = nullptr;
    }

    if (new_derived->has_sj_candidates() &&
        new_derived->flatten_subqueries(thd))
      return true;

    if (setup_tables(thd, get_table_list(), false)) return true;
  }  // Prepared_stmt_arena_holder scope

  // Resolving the new derived table needs normal arena
  if (resolve_placeholder_tables(thd, true)) return true;

  {
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    DBUG_ASSERT(tl->table != nullptr);

    /*
      We pushed the HAVING clause into new_derived above, but it is resolved to
      this query block, meaning it may have Item_aggregate_refs pointing into
      this->base_ref_items. We need to update such references to point into
      new_derived->base_ref_items instead, since this is where the aggregates
      are now also. We do this by adding them as hidden items and setting
      the Item_aggregate_refs::ref accordingly.
    */
    if (new_derived->m_having_cond != nullptr) {
      Item_sum::Collect_grouped_aggregate_info having_aggs(this);
      if (new_derived->m_having_cond->walk(&Item::collect_grouped_aggregates,
                                           enum_walk::PREFIX,
                                           pointer_cast<uchar *>(&having_aggs)))
        return true; /* purecov: inspected */

      for (auto agg : having_aggs.list) {
        Item::Aggregate_ref_update info(agg, new_derived);
        bool MY_ATTRIBUTE((unused)) error = new_derived->m_having_cond->walk(
            &Item::update_aggr_refs, enum_walk::PREFIX,
            pointer_cast<uchar *>(&info));
        DBUG_ASSERT(!error);
        agg->aggr_select = new_derived;
      }
    }

    /*
      Permanently replace the aggregates in this select list and windowing
      clauses with fields from the derived table.
    */
    Field **field_ptr = tl->table->field;
    for (auto agg : aggregates.list) {
      Item_field *replaces_agg = new (thd->mem_root) Item_field(*field_ptr);
      if (replaces_agg == nullptr) return true;

      // So we can re-bind this field in EXECUTE phase of prepared statement
      // Remove after WL#6570.
      replaces_agg->set_orig_names();

      /*
        The WHERE condition cannot contain group function from this level, so
        ignore. Only replace aggregates from the SELECT lists with fields from
        the derived table, then remove aggregates from top select lists.

        (1) fields_list
        (2) all_fields
      */
      Item::Aggregate_replacement info(agg, replaces_agg);
      if (replace_aggregate_in_list(info, all_fields)) return true;

      // We only transform implicit grouping to a derived table: in such a case,
      // the order by is eliminated since the result set has only one row, so
      // skip processing of order_list.
      DBUG_ASSERT(group_list.elements == 0);
      DBUG_ASSERT(order_list.elements == 0);

      List_iterator<Window> wli(m_windows);
      for (Window *w = wli++; w != nullptr; w = wli++) {
        for (ORDER *it : {w->first_order_by(), w->first_partition_by()}) {
          if (it != nullptr) {
            for (auto ord = it; ord != nullptr; ord = ord->next) {
              Item *new_item;
              THD::Permanent_transform tr(thd);  // RAII
              if (!(new_item = (*ord->item)
                                   ->transform(&Item::replace_aggregate,
                                               pointer_cast<uchar *>(&info))))
                return true; /* purecov: inspected */
              new_item->update_used_tables();
              if (new_item != *ord->item) {
                ord->item_ptr = new_item;
                *ord->item = new_item;
              }
            }
          }
        }
        // Physical sorting order should not have been set up since we are
        // implicitly grouped, so no need to attempt substitution in it.
        DBUG_ASSERT(w->sorting_order(nullptr, false) == nullptr);
      }

      // Aggregate argument may contain identifiers that need correct
      // context. View references will have been replaced Item_fields,
      // so we have to be careful: these will be rolled back and to make
      // our transformation permanent we need to update the context of the
      // original Item_fields, not the Item_view_refs.
      if (update_context_to_derived(agg, new_derived)) return true;

      ++field_ptr;
    }

    /*
      Remove any moved aggregates from top query block that did not get
      replaced above.
    */
    if (remove_aggregates(thd, new_derived)) return true;

    // field_ptr now points to the first of the view references added to the
    // select list of the derived table's query block. We now create new fields
    // for this block which will point to the corresponding item in the derived
    // table and then we substitute the new fields for the view refs. They must
    // survive name resolution in an EXECUTE step, so we use
    // THD::m_permanent_transform.
    for (auto vr : unique_view_refs) {
      for (auto expr : contrib_exprs) {
        Item::Item_view_ref_replacement info(vr->real_item(), *field_ptr, this);
        THD::Permanent_transform tr(thd);  // RAII
        Item *new_item = (*expr)->transform(&Item::replace_item_view_ref,
                                            pointer_cast<uchar *>(&info));
        if (new_item == nullptr) return true;
        if (new_item != *expr) *expr = new_item;
      }
      ++field_ptr;
    }

    // We have replaced the view references in this block with Item_fields
    // permanently, so we do not want any rollback at those locations in
    // the item_list. This only applies to duplicates of view references,
    // since the rollback record for the one in unique_view_refs has been
    // repurposed for rollback of the location in new_derived's item_list.
    List_iterator<Item> li(item_fields_or_view_refs);
    for (Item *i = li++; i != nullptr; i = li++) {
      if (std::find(unique_view_refs.begin(), unique_view_refs.end(), i) ==
          unique_view_refs.end()) {
        if (i->type() == Item::REF_ITEM) thd->cancel_rollback(i);
      }
    }

    // field_ptr now points to the first of the fields added to the select list
    // of the derived table's query block. We now create new fields for this
    // block which will point to the corresponding fields moved to the derived
    // table and then we substitute the new fields for the old ones. They must
    // survive name resolution in an EXECUTE step, so we use
    // THD::m_permanent_transform.
    for (auto pair : unique_fields) {
      auto replaces_field = new (thd->mem_root) Item_field(*field_ptr);
      if (replaces_field == nullptr) return true;
      replaces_field->set_orig_names();
      // Get back our original item name at this level
      replaces_field->item_name.set(pair.second->orig_name.ptr());
      // don't want synthetic name rolled back
      pair.second->orig_name.set(nullptr, 0);
      // We can update context of the field moved into the derived table
      // now that replaces_field has inherited the upper context
      pair.second->context = &new_derived->context;

      for (auto expr : contrib_exprs) {
        Item::Item_field_replacement info(pair.first, replaces_field, this,
                                          (*expr)->type() == Item::FIELD_ITEM);
        THD::Permanent_transform tr(thd);  // RAII
        Item *new_item = (*expr)->transform(&Item::replace_item_field,
                                            pointer_cast<uchar *>(&info));
        if (new_item == nullptr) return true;
        if (new_item != *expr) *expr = new_item;
      }

      ++field_ptr;
    }

    // Any join conditions in the from list now belong to new_derived, not
    // this block. Only needed for re-resolving in EXECUTE. May be removed after
    // WL#6570.
    update_join_cond_context(new_derived, new_derived->join_list);
    // Any fields from the original block have been moved to the new block,
    // so update fields in the rollback list also.
    thd->update_ident_context(this, new_derived);

    Opt_trace_context *trace = &thd->opt_trace;
    if (unlikely(trace->is_started())) {
      Opt_trace_object trace_wrapper(trace);
      Opt_trace_object trace_object(trace, "transform_grouped_to_derived");
      // the newly transformed query is worth printing
      opt_trace_print_expanded_query(thd, this, &trace_object);
    }
  }  // Prepared_stmt_arena_holder scope
  return false;
}

/**
  Replace the name resolution context in fields contained in join clauses
  in join_list to that of new_context, if they are currently resolved to
  this block.

  @param new_context    The now block the fields now belong to
  @param join_list      The join_list of new_context or one of its nested
                        join_lists (the latter in recursive calls)
*/
void SELECT_LEX::update_join_cond_context(
    SELECT_LEX *new_context, mem_root_deque<TABLE_LIST *> *join_list) {
  for (auto tl : *join_list) {
    if (tl->join_cond() != nullptr) {
      Item_field::Context_info info{this, new_context};
      (void)tl->join_cond()->walk(&Item::update_context, enum_walk::PREFIX,
                                  pointer_cast<uchar *>(&info));
    }
    if (tl->nested_join != nullptr) {
      update_join_cond_context(new_context, &tl->nested_join->join_list);
    }
  }
}

/**
  A minion of transform_scalar_subqueries_to_derived.

  A transform creates a field representing the value of the derived table and
  adds it as a hidden field to the select list.  Next, it replaces the subquery
  in the item tree with this field.  If we replace in a HAVING condition, we
  build an Item_ref, cf. PTI_simple_ident_ident::itemize which also creates a
  Item_ref for a field reference in HAVING, because we may need to access the
  field in a tmp table.

  @param      thd       The session context
  @param      subquery  The scalar subquery
  @param      expr      The expression we are replacing (in)
*/
bool SELECT_LEX::replace_subquery_in_expr(THD *thd,
                                          Item_singlerow_subselect *subquery,
                                          Item **expr) {
  if (!(*expr)->has_subquery()) return false;

  Item_singlerow_subselect::Scalar_subquery_replacement info(
      subquery, *subquery->m_derived_replacement->table->field, this);

  Item *new_item = (*expr)->transform(&Item::replace_scalar_subquery,
                                      pointer_cast<uchar *>(&info));
  if (new_item == nullptr) return true;

  // If we replaced an item contained in the transformed query block, save it
  // for rollback and retain its name so the metadata column name remains
  // correct.
  if (*expr != new_item) {
    new_item->item_name.set((*expr)->item_name.ptr());
    thd->change_item_tree(expr, new_item);
  }

  new_item->update_used_tables();

  // If this expression has aggregation and we have replaced a subquery
  // with a field, we need to recompute split_sum_func
  if ((new_item->has_aggregation() &&
       !(new_item->type() == Item::SUM_FUNC_ITEM &&
         !new_item->m_is_window_function)) ||  //(1)
      new_item->has_wf())                      // (2)
    new_item->split_sum_func(thd, base_ref_items, all_fields);
  if (thd->is_error()) return true;
  return false;
}

/**
  A minion of transform_scalar_subqueries_to_derived.

  With PREPARE + EXECUTE, the derived table substituted for the scalar
  subquery is not rolled back after prepare. So, when we do EXECUTE,
  we need to find the derived table we created. This function does that
  by checking a flag present only in such derived tables and that the derived
  table belongs to unit.

  @param    tables    The list of tables to search through
  @param    unit      The query expression for the subquery
                      If nullptr, return the first found, if any

  @returns the derived table or nullptr if we haven't created one yet
*/
static TABLE_LIST *locate_derived(mem_root_deque<TABLE_LIST *> &tables,
                                  SELECT_LEX_UNIT *unit) {
  for (auto cand : tables) {
    if (cand->is_derived() &&
        (unit == nullptr || cand->derived_unit() == unit) &&
        cand->m_was_scalar_subquery)
      return cand;
    if (cand->nested_join != nullptr) {
      auto *result = locate_derived(cand->nested_join->join_list, unit);
      if (result != nullptr) return result;
    }
  }
  return nullptr;
}

/**
  A minion of transform_scalar_subqueries_to_derived.

  Determine if the query expression is directly contained in the
  query block, i.e. it is a subquery.

  @param select  the query block
  @param slu     the query expression

  @returns true if slu is directly contained in select, else false
*/
static bool query_block_contains_subquery(SELECT_LEX *select,
                                          SELECT_LEX_UNIT *slu) {
  for (SELECT_LEX_UNIT *cand = select->first_inner_unit(); cand != nullptr;
       cand = cand->next_unit()) {
    if (cand == slu) return true;
  }
  return false;
}

static bool walk_join_conditions(
    mem_root_deque<TABLE_LIST *> &list,
    std::function<bool(Item **expr_p)> action,
    Item_singlerow_subselect::Collect_scalar_subquery_info *info) {
  for (TABLE_LIST *tl : list) {
    if (tl->join_cond() != nullptr) {
      info->m_join_condition_context = tl->join_cond();
      if (action(tl->join_cond_ref())) return true;
    }
    if (tl->nested_join != nullptr &&
        walk_join_conditions(tl->nested_join->join_list, action, info))
      return true; /* purecov: inspected */
  }
  info->m_join_condition_context = nullptr;
  return false;
}

/**
 Remember if this transform was performed. It it was done by a secondary
 engine, it may need to be rolled back before falling back on primary engine
 execution.
 */
static void remember_transform(THD *, SELECT_LEX *select) {
  // Transform was enabled not by switch, but by secondary enginee
  select->parent_lex->m_sql_cmd->set_optional_transform_prepared(true);
}

/**
  Push the generated derived table to the correct location inside a join nest.
  It will be nested in a new nest along with the outer table to the join
  which owns the search condition in which we found the scalar subquery.
  For example:

      select t1.i,
             t2.i
      from t1
           left outer join
           t2 on
           (t1.i < (select max(t2.i) from t2));

      in transformed to

      select t1.i,
             t2.i
      from t1
           left join
           (select max(t2.i) AS `max(t2.i)` from t2) derived_1_0   [*]
           on(true)
           left join
           t2
           on((t1.i < derived_1_0.`max(t2.i)`))

  [*]: the derived table is nested in here, just ahead of the inner table
       t2 to which the join condition is attached.

  In the original join nest before transformation may look like this
  (the join order list is reversed relative to the logical order):

   (nest_last_join)
      t2  LEFT OUTER        ON .. = ..       (inner table)
      t1                                     (outer table)

   After the transformation we have this nest structure:

   (nest_last_join)
      t2 LEFT OUTER         ON  .. = ..
      (nest_last_join)
         derived_1_0 LEFT OUTER ON true
         t1

  The method will recursively inspect and rebuild join nests as needed since
  the join with the condition may be deeply nested. Special treatment
  for semi-join and anti-join nests is present.

  @param   thd           the session context
  @param   join_cond     the join condition which identified the join we want to
                         nest into
  @param   nested_join_list
                         the join list at the current nesting level
  @param   derived_table the table we want to nest

  @returns the new resulting nest, or nullptr on error
*/
TABLE_LIST *SELECT_LEX::nest_derived(
    THD *thd, Item *join_cond, mem_root_deque<TABLE_LIST *> *nested_join_list,
    TABLE_LIST *derived_table) {
  DBUG_ASSERT(nested_join_list->size() == 2);
  TABLE_LIST *old_inner = nested_join_list->front();
  TABLE_LIST *old_outer = nested_join_list->back();
  nested_join_list->clear();

  if (join_cond == old_inner->join_cond() ||
      join_cond == old_outer->join_cond()) {
    // We found the correct join, insert the new derived table
    if (add_joined_table(old_outer)) return nullptr;
    if (add_joined_table(derived_table)) return nullptr;
    if (nest_last_join(thd) == nullptr) return nullptr;
    if (add_joined_table(old_inner)) return nullptr;

    if (join_cond == old_outer->join_cond()) {
      // A join condition on the outer table: can happen after another
      // transform, e.g. merging a single-table view moves view's WHERE to
      // table's ON. Ok with us, should be safe to put derived table after it.
      // Just move the join condition up to the resulting (outer) nest.
      old_outer->set_join_cond(nullptr);
      join_list->back()->set_join_cond(join_cond);
    }
  } else {
    // We need to look deeper.
    enum Kind { OUTER = 0, INNER = 1 };
    TABLE_LIST *new_joinees[2] = {old_outer, old_inner};
    if (old_inner->is_aj_nest()) {
      DBUG_ASSERT(std::strcmp(old_outer->alias, "(aj-left-nest)") == 0);

      // peel off any semi-joins and save
      mem_root_deque<TABLE_LIST *> sj(thd->mem_root);
      uint siz = old_outer->nested_join->join_list.size();
      for (uint i = 0; i < siz - 1; i++) {
        DBUG_ASSERT(old_outer->nested_join->join_list.front()->is_sj_nest());
        sj.push_back(old_outer->nested_join->join_list.front());
        old_outer->nested_join->join_list.pop_front();
      }
      TABLE_LIST *nest = old_outer->nested_join->join_list.front();
      old_outer->nested_join->join_list.pop_front();

      TABLE_LIST *new_nest = nest_derived(
          thd, join_cond, &nest->nested_join->join_list, derived_table);
      if (new_nest == nullptr) return nullptr;
      join_list->pop_front();  // undo this, too early

      new_nest->embedding = old_outer;
      old_outer->nested_join->join_list.push_front(new_nest);

      // add back any semi-joins
      while (sj.size() > 0) {
        old_outer->nested_join->join_list.push_front(sj.back());
        sj.pop_back();
      }
    } else {
      for (auto &tl : new_joinees) {
        if (tl->nested_join != nullptr &&
            tl->nested_join->join_list.size() != 1) {
          TABLE_LIST *new_nest = nest_derived(
              thd, join_cond, &tl->nested_join->join_list, derived_table);
          if (new_nest == nullptr) return nullptr;
          if (tl != new_nest) {
            // update any references to the old nest to the new one
            for (auto sj : *sj_candidates)
              if (sj->embedding_join_nest == tl)
                sj->embedding_join_nest = new_nest;
          }
          tl = new_nest;
          // eliminate unwanted side effect: we are not ready to set join_list
          DBUG_ASSERT(join_list->size() == 1);
          join_list->pop_front();
        }
      }

      if (new_joinees[INNER] != old_inner) {
        // The inner table carries the join information we need to copy over
        new_joinees[INNER]->set_join_cond(old_inner->join_cond());
        new_joinees[INNER]->outer_join = old_inner->outer_join;
        new_joinees[INNER]->straight = old_inner->straight;
      }
    }

    // We have (hopefully ;-) pushed the derived table into a deeper nest, now
    // rebuild the nest at this level
    for (auto tl : new_joinees)
      if (add_joined_table(tl)) return nullptr;
  }

  return nest_last_join(thd);
}

bool SELECT_LEX::transform_scalar_subqueries_to_derived(THD *thd) {
  // Need at least one FROM table. Also, we do not want to perform this
  // transformation if we have an assignment of a user variable in the query.
  if (leaf_table_count == 0 || thd->lex->set_var_list.elements > 0)
    return false;

  uint derived_tab_no = 0;

  /*
    Collect list of eligible scalar subqueries used in JOIN conds, WHERE conds,
    SELECT list expressions and HAVING cond. NOTE: Join conditions need to be
    collected/transformed first since they have the be nested after the outer
    join table (i.e. before the inner). So, if we have scalar subqueries in
    other locations that the JOIN conditions, those need to be added after the
    JOIN conditions have been put in place.
  */

  Item_singlerow_subselect::Collect_scalar_subquery_info subqueries;

  // Collect from join conditions
  if (walk_join_conditions(
          top_join_list,
          [&](Item **expr_p) mutable -> bool {
            subqueries.m_location = Item_singlerow_subselect::
                Collect_scalar_subquery_info::L_JOIN_COND;
            if ((*expr_p)->has_subquery() &&
                (*expr_p)->walk(&Item::collect_scalar_subqueries,
                                enum_walk::PREFIX | enum_walk::POSTFIX,
                                pointer_cast<uchar *>(&subqueries)))
              return true; /* purecov: inspected */
            return false;
          },
          &subqueries))
    return true; /* purecov: inspected */

  subqueries.m_location =
      Item_singlerow_subselect::Collect_scalar_subquery_info::L_WHERE;

  Item **where_expr_p = &m_where_cond;
  if (*where_expr_p != nullptr && (*where_expr_p)->has_subquery()) {
    if ((*where_expr_p)
            ->walk(&Item::collect_scalar_subqueries,
                   enum_walk::PREFIX | enum_walk::POSTFIX,
                   pointer_cast<uchar *>(&subqueries)))
      return true; /* purecov: inspected */
  }

  subqueries.m_location =
      Item_singlerow_subselect::Collect_scalar_subquery_info::L_SELECT;
  List_iterator<Item> lii(fields_list);
  Item *select_expr;
  while ((select_expr = lii++)) {
    if (select_expr->has_subquery() &&
        (select_expr)
            ->walk(&Item::collect_scalar_subqueries,
                   enum_walk::PREFIX | enum_walk::POSTFIX,
                   pointer_cast<uchar *>(&subqueries)))
      return true; /* purecov: inspected */
  }

  subqueries.m_location =
      Item_singlerow_subselect::Collect_scalar_subquery_info::L_HAVING;
  Item **having_expr_p = &m_having_cond;
  if (*having_expr_p != nullptr && (*having_expr_p)->has_subquery()) {
    if ((*having_expr_p)
            ->walk(&Item::collect_scalar_subqueries,
                   enum_walk::PREFIX | enum_walk::POSTFIX,
                   pointer_cast<uchar *>(&subqueries)))
      return true; /* purecov: inspected */
  }

  /*
    Loop through eligible subqueries and see if we need the extra transform of
    implicit grouping into a separate derived table before we can
    transform the scalar subqueries to more derived tables.  But we
    cannot do this if we have a HAVING expression which references or contains
    a subquery.
    In that case, we throw in the towel and don't do any transformations. E.g.

    1. SELECT SUM(a), (SELECT SUM(b) FROM t3) scalar
       FROM t1
       HAVING SUM(a) > scalar;

    2. SELECT MAX(a)
       FROM t1
       WHERE FALSE
       HAVING (SELECT MIN(a) FROM t1) > 0;

   TODO: we could solve this by not moving the HAVING condition into the derived
   table, but instead letting it remain in the transformed block as a WHERE
   predicate, e.g. in the case of example 1:

     SELECT derived0.summ, derived1.scalar
     FROM (SELECT SUM(a) AS summ FROM t1) AS derived0
           LEFT JOIN
           (SELECT SUM(b) AS scalar FROM t3) AS derived1
           ON TRUE
     WHERE derived0.sum > derived1.scalar;

   but this is not yet done.
  */
  if (is_implicitly_grouped()) {
    bool need_new_outer = false;
    for (auto subquery : subqueries.list) {
      auto *subq = subquery.item;
      if (!query_block_contains_subquery(this, subq->unit)) continue;

      // Possibly contradicting requirements
      // (1) Subquery is in SELECT list: new_outer
      // (2) No new outer possible if HAVING contains subquery
      if (subquery.m_location &
          Item_singlerow_subselect::Collect_scalar_subquery_info::L_SELECT) {
        need_new_outer = true;
      }
      if (subquery.m_location &
          Item_singlerow_subselect::Collect_scalar_subquery_info::L_HAVING)
        return false;
    }

    if (need_new_outer) {
      /*
        In this case, the default transform with a single new derived table and
        a LEFT OUTER JOIN isn't always correct - we need to first move the
        aggregated query to a new derived subquery before we can transform the
        scalar subqueries to other derived tables.
      */
      bool break_off = false;
      if (transform_grouped_to_derived(thd, &break_off)) return true;
      derived_tab_no++;
      if (break_off) return false;  // skip transformation
    }
  }

  std::vector<TABLE_LIST *> seen_derived;
  /*
    Loop through eligible subqueries and transform them to derived tables
    and replace occurrences in expression trees with a field of the relevant
    derived table.
  */
  for (auto subquery : subqueries.list) {
    Item_singlerow_subselect *const subq = subquery.item;
    SELECT_LEX_UNIT *const subs_unit = subq->unit;

    /*
      A reference to a scalar subquery from another query expression can happen.
      We can't transform it here, but it may be replaced from another query
      block.
    */
    if (!query_block_contains_subquery(this, subs_unit)) continue;

    TABLE_LIST *tl = locate_derived(*join_list, subs_unit);
    const bool first_prepare = (tl == nullptr);

    if (first_prepare) {
      // We did not do the transformation yet
      remember_transform(thd, this);

      // We want the TABLE_LIST, Table_ident and m_join_cond to be permanent
      Prepared_stmt_arena_holder ps_arena_holder(thd);

      // Create a derived table for the subquery and nest it. If we found the
      // subquery outside of a join condition, we simply nest it at the end
      // with a LEFT OUTER .. ON TRUE, e.g.
      //
      // SELECT (SELECT COUNT(a) FROM t2) + a FROM t1;
      // ->
      // SELECT derived.cnt + t1.a FROM
      //   t1 LEFT OUTER JOIN
      //   (select COUNT(a) AS cnt FROM t2) AS derived
      // ON TRUE;
      //
      // If we have a subquery inside a join condition we nest it after the
      // outer table:
      //
      // SELECT * FROM t1 LEFT JOIN
      //               t2
      //             ON (SELECT COUNT(a) AS cnt FROM t2) = t1.a;
      // ->
      // SELECT * FROM t1 LEFT JOIN
      //               (SELECT COUNT(t2.a) AS cnt
      //                FROM t2) derived_1_0
      //             ON(TRUE) LEFT JOIN
      //               t2
      //             ON derived_1_0.cnt = t1.a
      //
      tl = synthesize_derived(thd, subs_unit, subquery.m_join_condition,
                              derived_tab_no++, true);
      if (tl == nullptr) return true;
      m_scalar_to_derived.push_back(tl);
      subq->m_derived_replacement = tl;

      // Append to end of leaf tables list
      TABLE_LIST *leaf;
      for (leaf = leaf_tables; leaf->next_leaf != nullptr;
           leaf = leaf->next_leaf) {
      }
      leaf->next_leaf = tl;

      // Adjust table no and map
      tl->set_tableno(leaf_table_count);

      tl->embedding->nested_join->query_block_id =
          subq->unit->first_select()->select_number;
      leaf_table_count += 1;

      if (!(tl->derived_result = new (thd->mem_root) Query_result_union()))
        return true; /* purecov: inspected */
      subs_unit->set_explain_marker(thd, CTX_DERIVED);
      subs_unit->first_select()->linkage = DERIVED_TABLE_TYPE;

      // Break connection to the subquery expression:
      subs_unit->m_place_before_transform = subs_unit->item->place();
      subs_unit->item = nullptr;
    } else {
      DBUG_ASSERT(subq->m_derived_replacement == tl);
      /*
        EXECUTE time: the derived table and outer join wrapping was inserted at
        PREPARE time, so transformation has been done, except we have lost the
        the temporary table and the item representing the scalar query
        result. We just found the derived table. We will need to reinitialize
        tmp table and Item_field. Replacement of the subquery item must also be
        redone.
      */
      tl->derived_result->table = nullptr;
    }
    seen_derived.push_back(tl);

    subs_unit->set_query_result(tl->derived_result);
    subs_unit->first_select()->set_query_result(tl->derived_result);

    if (first_prepare) {
      materialized_derived_table_count++;
      derived_table_count++;
    }

    if (tl->table == nullptr && tl->setup_materialized_derived(thd))
      return true; /* purecov: inspected */

    /*
      Replace the subquery with a field in the materialized tmp table
      in WHERE, JOIN conditions, HAVING clause or SELECT expressions (could be
      optimized by keeping track in which expression the subquery was found)
    */

    // Replace in WHERE clause?
    if (subquery.m_location &
        Item_singlerow_subselect::Collect_scalar_subquery_info::L_WHERE) {
      if (*where_expr_p != nullptr &&
          replace_subquery_in_expr(thd, subq, where_expr_p))
        return true; /* purecov: inspected */
    }

    // Replace in join conditions?
    if (subquery.m_location &
        Item_singlerow_subselect::Collect_scalar_subquery_info::L_JOIN_COND) {
      if (walk_join_conditions(
              top_join_list,
              [&](Item **expr_p) mutable -> bool {
                subqueries.m_location = Item_singlerow_subselect::
                    Collect_scalar_subquery_info::L_JOIN_COND;
                if (*expr_p != nullptr &&
                    replace_subquery_in_expr(thd, subq, expr_p))
                  return true; /* purecov: inspected */
                return false;
              },
              &subqueries))
        return true; /* purecov: inspected */
    }

    List_iterator<Item> lia(all_fields);
    while ((select_expr = lia++)) {
      if (replace_subquery_in_expr(thd, subq, lia.ref())) return true;
      if (*lia.ref() != select_expr) {
        for (uint i = 0; i < all_fields.elements; i++) {
          if (base_ref_items[i] == select_expr) base_ref_items[i] = *lia.ref();
        }
      }
    }

    // Replace in HAVING clause?
    if (subquery.m_location &
        (Item_singlerow_subselect::Collect_scalar_subquery_info::L_HAVING)) {
      if (*having_expr_p != nullptr &&
          replace_subquery_in_expr(thd, subq, having_expr_p))
        return true; /* purecov: inspected */
    }

    // A subquery in the SELECT list can be present in the GROUP BY clause
    // so we potentially need to replace there too.
    for (ORDER *ord = group_list.first; ord != nullptr; ord = ord->next) {
      if (replace_subquery_in_expr(thd, subq, ord->item)) return true;
    }

    // Replace again any sj expressions which were made permanent during PREPARE
    // semijoin flattening happens after this transform, and can make aliases
    // which we need to handle specially during PS execute. Can be removed after
    // WL#6570.
    if (!first_prepare) {
      TABLE_LIST *tbl = table_list.first;
      while (tbl != nullptr) {
        NESTED_JOIN *nj;
        if (tbl->embedding != nullptr &&
            ((nj = tbl->embedding->nested_join), nj != nullptr)) {
          List_iterator<Item> li(nj->sj_inner_exprs);
          Item *i;
          while ((i = li++)) {
            if (replace_subquery_in_expr(thd, subq, li.ref())) return true;
          }
          li.rewind();
          li.init(nj->sj_outer_exprs);
          while ((i = li++)) {
            if (replace_subquery_in_expr(thd, subq, li.ref())) return true;
          }
        }
        tbl = tbl->next_local;
      }
    }

    Opt_trace_context *trace = &thd->opt_trace;
    if (unlikely(trace->is_started())) {
      Opt_trace_object trace_wrapper(trace);
      Opt_trace_object trace_object(trace,
                                    "transform_scalar_subquery_to_derived");
      // the newly transformed query is worth printing
      opt_trace_print_expanded_query(thd, this, &trace_object);
    }
  }

  TABLE_LIST *tl;
  List_iterator<TABLE_LIST> tli(m_scalar_to_derived);
  while ((tl = tli++)) {
    if (std::find(seen_derived.begin(), seen_derived.end(), tl) ==
        seen_derived.end()) {
      // We didn't see this derived table during this transform so it must
      // come from another.
      // m_scalar_to_derived also contains any synthesized derived tables from
      // underlying views that were merged into this block.
      // Delete if we have no reference to them on this block level.
      // If we don't do this, we get into trouble during EXECUTE.
      bool found = false;
      if (field_of_table_present(tl, &found)) return true;
      if (!found) remove_derived(thd, tl);
    }
  }

  return false;
}

/**
  Normally, the end of a prepared statement rolls back the replacement of
  scalar subqueries with a field (the derived table stays) in the expressions
  of the query (select list where, clause, order by etc). With semijoin
  flattening, an alias of the substituted field representing the scalar
  subquery is made during the call to build_sj_cond. (Since the semijoin
  flattening is done after the scalar subquery to derived table
  transformations).  A priori, these locations will not be rolled back after
  the prepare ends, since they are not registered when performing
  replace_subquery_in_expr. This leaves the semijoin condition with stale
  arguments that are not resolvable in an subsequent EXECUTE's prepare. This
  method takes care of this by marking such locations for rollback after
  prepare as well. During executing, the scalar subqueried will be seen again
  (this time inside the semijoin conditions) and will be replaced by the correct
  Item_fields. Also applies to NESTED_JOIN::sj_{inner,outer}_exprs.
  Should all should be removed after WL#6570.

  @param  thd  current session context
  @param  i    the item pointer that may need rollback
  @return true on error
*/
static bool maybe_mark_item_for_rollback(THD *thd, Item **i) {
  I_List_iterator<Item_change_record> it(thd->change_list);
  Item_change_record *change;
  while ((change = it++)) {
    if (*i == change->new_value && !change->m_cancel &&
        change->old_value->type() == Item::SUBSELECT_ITEM &&
        down_cast<Item_subselect *>(change->old_value)->substype() ==
            Item_subselect::SINGLEROW_SUBS &&
        down_cast<Item_singlerow_subselect *>(change->old_value)
                ->m_derived_replacement != nullptr) {
      *i = change->old_value;
      thd->nocheck_register_item_tree_change(i, change->new_value);
      if (thd->is_fatal_error()) return true;
      *i = change->new_value;
    }
  }

  return false;
}

/**
  Applies mark_item_for_rollback to the arguments of item_eq.

  @param thd     current session context
  @param item_eq the target function whose arguments may need roolback after
                 PREPARE. See also explanation given in
                 maybe_mark_item_for_rollback.
*/
static bool add_possible_transformation_rollbacks(THD *thd,
                                                  Item_func_eq *item_eq) {
  Item **args = item_eq->arguments();
  for (int i = 0; i < 2; i++) {
    if (args[i]->type() == Item::FIELD_ITEM) {
      if (maybe_mark_item_for_rollback(thd, &args[i])) return true;
    }
  }
  return false;
}

/**
  @} (end of group Query_Resolver)
*/
