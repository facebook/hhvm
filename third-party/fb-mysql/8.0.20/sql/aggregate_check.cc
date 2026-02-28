/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Checks for some semantic constraints on queries using GROUP
   BY, or aggregate functions, or DISTINCT. Enforced if
   sql_mode contains 'only_full_group_by'.
*/

#include "sql/aggregate_check.h"

#include "my_config.h"

#include <stdio.h>
#include <utility>

#include "my_base.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/item_func.h"
#include "sql/item_row.h"
#include "sql/key.h"
#include "sql/nested_join.h"
#include "sql/opt_trace.h"
#include "sql/opt_trace_context.h"
#include "sql/parse_tree_nodes.h"
#include "sql/sql_base.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/table.h"
#include "sql/window.h"
#include "template_utils.h"

/**
  @addtogroup AGGREGATE_CHECKS

  @section USED_TABLES Implementation note: used_tables_for_level() vs
  used_tables()

  - When we are looking for items to validate, we must enter scalar/row
  subqueries; if we find an item of our SELECT_LEX inside such subquery, for
  example an Item_field with depended_from equal to our SELECT_LEX, we
  must use used_tables_for_level(). Example: when validating t1.a in
  select (select t1.a from t1 as t2 limit 1) from t1 group by t1.pk;
  we need t1.a's map in the grouped query; used_tables() would return
  OUTER_REF_TABLE_BIT.
  - When we are searching for FDs in PKs, or join conditions, or the GROUP BY
  clause, we do not enter scalar/row subqueries, so we use
  used_tables(). Example:
  select ... from t1 where t1.a=(subq) from t1 group by ...
  the subq is not used to discover FDs in the grouped query.
  Or:
  select (select t2.a from t1 as t2 where t2.a=t1.a group by t2.b) from t1
  when validating the subq, t1.a is an outer reference, kind of a constant, so
  tells us that t2.a is FD on {} ; using used_tables_for_level() on t1.a would
  be nonsense - we are validating the subquery.

  @{
*/

/**
  We need to search for items inside subqueries, in case subqueries contain
  outer references to tables of a query block having DISTINCT or GROUP BY.
  We also need to sometimes skip parts of item trees, so the walk processor
  must be called prefix (to enable skipping) and postfix (to disable
  skipping).
*/
static const enum_walk walk_options =
    enum_walk::SUBQUERY_PREFIX | enum_walk::POSTFIX;

/**
   Rejects the query if it has a combination of DISTINCT and ORDER BY which
   could lead to randomly ordered results. More precisely: if, in a query
   block 'sl', an ORDER BY expression
   - is not the same expression as one in the SELECT list of 'sl' (1)
   - and contains a column which:
   -- is of a table whose qualifying query block is 'sl'          (2)
   -- and is not in the SELECT list of 'sl'                       (3)
   then 'sl' should not have DISTINCT.

   @returns true if rejected (my_error() is called)
*/
bool Distinct_check::check_query(THD *thd) {
  uint number_in_list = 1;
  for (ORDER *order = select->order_list.first; order;
       ++number_in_list, order = order->next) {
    if (order->in_field_list)  // is in SELECT list
      continue;
    DBUG_ASSERT((*order->item)->fixed);
    uint counter;
    enum_resolution_type resolution;
    /*
      Search if this expression is equal to one in the SELECT
      list. setup_order()/find_order_in_list() has already done so, but not
      perfectly, indeed at that time the expression was not fixed, which
      prevents recognition of certain equalities. Example:
        create view v select x*2 as b from ...;
        select distinct sin(b) as z from v order by sin(b);
      This query is valid because the expression in ORDER BY is the same as
      the one in SELECT list. But in setup_order():
      'b' in ORDER BY (not yet fixed) is still a 'generic' Item_field,
      'b' in SELECT (already fixed) is Item_view_ref referencing 'x*2'
      (so type()==REF_ITEM).
      So Item_field::eq() says the 'b's are different, so 'sin(b)' of
      ORDER BY is not found equal to 'sin(b)' of SELECT.

      On the other hand, the search below will match, because everything is
      now fixed.

      There is a limitation with subqueries: in this query
      SELECT (subquery) ... ORDER BY (subquery)
      we may not be able to recognize that both subqueries are the same (and
      so we may reject it even though the order is deterministic). It is
      because Item_subselect::eq() is Item::eq() which is too coarse and
      misses equalities: it compares names of both items; depending on the
      position of the subquery in the query, MySQL gives it a name or not; and
      this name is the raw text of the subquery (so if subqueries' texts
      differ due to white space....).
      Subqueries in ORDER BY are non-standard anyway.
    */
    Item **const res =
        find_item_in_list(thd, *order->item, select->item_list, &counter,
                          REPORT_EXCEPT_NOT_FOUND, &resolution);
    if (res == nullptr)  // Other error than "not found", my_error() was called
      return true;       /* purecov: inspected */
    if (res != not_found_item)  // is in SELECT list
      continue;
    /*
      [numbers refer to the function's comment]
      (1) is true. Check (2) and (3) inside walk().
    */
    if ((*order->item)
            ->walk(&Item::aggregate_check_distinct, walk_options,
                   (uchar *)this)) {
      if (failed_ident)
        my_error(ER_FIELD_IN_ORDER_NOT_SELECT, MYF(0), number_in_list,
                 failed_ident->full_name(), "DISTINCT");
      else
        my_error(ER_AGGREGATE_IN_ORDER_NOT_SELECT, MYF(0), number_in_list,
                 "DISTINCT");
      return true;
    }
  }
  return false;
}

/**
   Rejects the query if it does aggregation or grouping, and expressions in
   its SELECT list, ORDER BY clause, HAVING condition, or window functions
   may vary inside a group (are not "group-invariant").
*/
bool Group_check::check_query(THD *thd) {
  ORDER *order = select->order_list.first;

  // Validate SELECT list
  uint number_in_list = 1;
  const char *place = "SELECT list";

  for (Item &sel_expr : select->item_list) {
    if (check_expression(thd, &sel_expr, true)) goto err;
    ++number_in_list;
  }

  // Aggregate without GROUP BY has no ORDER BY at this stage
  DBUG_ASSERT(!(select->is_implicitly_grouped() && select->is_ordered()));
  // Validate ORDER BY list
  if (order) {
    number_in_list = 1;
    place = "ORDER BY clause";
    for (; order; order = order->next) {
      // If it is in SELECT list it is already checked.
      if (!order->in_field_list && check_expression(thd, *order->item, false))
        goto err;
      ++number_in_list;
    }
  }

  // Validate HAVING condition
  if (select->having_cond()) {
    number_in_list = 1;
    place = "HAVING clause";
    if (check_expression(thd, select->having_cond(), false)) goto err;
  }

  // Validate windows' ORDER BY and PARTITION BY clauses.
  char buff[STRING_BUFFER_USUAL_SIZE];
  {
    List_iterator<Window> li(select->m_windows);
    for (Window *w = li++; w != nullptr; w = li++) {
      for (auto it : {w->first_partition_by(), w->first_order_by()}) {
        if (it != nullptr) {
          number_in_list = 1;
          for (ORDER *o = it; o != nullptr; o = o->next) {
            Item *expr = *(o->item);
            if (check_expression(thd, expr, false)) {
              snprintf(buff, sizeof(buff),
                       "PARTITION BY or ORDER BY clause of window '%s'",
                       w->printable_name());
              place = buff;
              goto err;
            }
            ++number_in_list;
          }
        }
      }
    }
  }

  return false;

err:
  uint code;
  const char *text;
  /*
    Starting from MySQL 5.7 we want give a better messages than before,
    to provide more information. But we can't change texts of existing error
    codes for backward-compatibility reasons, so we introduce new texts;
    however we want to keep sending the old error codes, for pre-5.7
    applications used to it.
  */
  if (select->is_explicitly_grouped()) {
    code = ER_WRONG_FIELD_WITH_GROUP;                  // old code
    text = ER_THD(thd, ER_WRONG_FIELD_WITH_GROUP_V2);  // new text
  } else {
    code = ER_MIX_OF_GROUP_FUNC_AND_FIELDS;                  // old code
    text = ER_THD(thd, ER_MIX_OF_GROUP_FUNC_AND_FIELDS_V2);  // new text
  }
  my_printf_error(code, text, MYF(0), number_in_list, place,
                  failed_ident->full_name());
  return true;
}

/**
   Validates one expression (this forms one step of check_query()).
   @param  thd   current thread
   @param  expr  expression
   @param  in_select_list  whether this expression is coming from the SELECT
   list.
*/
bool Group_check::check_expression(THD *thd, Item *expr, bool in_select_list) {
  DBUG_ASSERT(!is_child());
  if (!in_select_list) {
    uint counter;
    enum_resolution_type resolution;
    // Search if this expression is equal to one in the SELECT list.
    Item **const res = find_item_in_list(thd, expr, select->item_list, &counter,
                                         REPORT_EXCEPT_NOT_FOUND, &resolution);
    if (res == nullptr)  // Other error than "not found", my_error() was called
      return true;       /* purecov: inspected */
    if (res != not_found_item) {
      // is in SELECT list, which has already been validated.
      return false;
    }
  }

  for (ORDER *grp = select->group_list.first; grp; grp = grp->next) {
    if ((*grp->item)->eq(expr, false))
      return false;  // Expression is in GROUP BY so is ok
  }

  // Analyze columns/aggregates referenced by the expression
  return expr->walk(&Item::aggregate_check_group, walk_options, (uchar *)this);
}

/**
   Tells if 'item' is functionally dependent ("FD") on source columns.
   Source columns are:
   - if !is_child(), the GROUP BY columns
   - if is_child(), columns of the result of the query expression under
   'table' which are themselves part of 'fd' of the parent Group_check.

   We recognize most FDs imposed by SQL2011 (optional feature T301)

   We build sets, named En, by induction.
   A "column" is defined as base table / view / derived table's column.

   E1 = {source columns} (=group columns, if this is a master Group_check;
   empty set otherwise).

   En is a set of columns of the result of the WHERE clause of 'select' which
   are functionally dependent on E1.
   If is_child(), En columns might rather be of the result of the GROUP BY
   clause (if there is one), but that's an unimportant detail, ignored further
   down.

   Given En, build En+1:
    - let En+1= En
    - for each {pk/unique index of some table T} found in En, add T.* to En+1
    (actually, we rather add T's map bit to the table_map whole_tables_fd).

   Then build En+2, by adding y, for each x=y in AND parts of WHERE/ON where
   x is in En+1 or is constant, and y is a column not in En+1.

   When we meet columns of views or derived tables, we additionally search
   for FDs in their query expression, which can then give FDs in our
   query.
   If En+2==En, this is the end of the induction. Otherwise we loop.

   As we build En, we check if 'item' is in it.

   @param  item  Item to consider; either a column local to 'select', or a set
   function whose aggregation query is 'select'

   @returns true if 'item' is functionally dependent on source columns.
*/
bool Group_check::is_fd_on_source(Item *item) {
  if (is_in_fd(item)) return true;

  if (!is_child()) {
    /*
      If it were a child Group_check, its list of source columns
      would start empty, it would gradually be filled by the master
      Group_check when it fills its own list.
      Here it is the master Group_check, so GROUP expressions are considered
      to be known, from which we build E1.
    */
    if (fd.empty()) {
      /*
        We do a first attempt: is the column part of group columns? This
        test should be sufficient to accept any query accepted by
        only_full_group_by in 5.6, and avoids filling the "fd" list with
        add_to_fd() (and potentially add_to_source_of_mat_table()).
        It's just an optimization.
      */
      for (ORDER *grp = select->group_list.first; grp; grp = grp->next) {
        if ((*grp->item)->eq(item, false)) return true;
      }
      // It didn't suffice. Let's start the search for FDs: build E1.
      for (ORDER *grp = select->group_list.first; grp; grp = grp->next) {
        Item *const grp_it = *grp->item;
        add_to_fd(grp_it, local_column(grp_it));
      }
    }
  }

  if (select->olap != UNSPECIFIED_OLAP_TYPE) {
    /*
      - the syntactical transformation of ROLLUP is to make a union of
      queries, and in each such query, some group column references are
      replaced with a NULL literal.
      - functional dependencies should be recognized only after that
      transformation. But there cannot be a key-based or equality-based
      functional dependency on a NULL literal.
      Moreover, there are no FDs in a UNION.
      So if the query has ROLLUP, we can stop here. Above, we have tested the
      column against the group columns and that's enough.
      See test group_by_fd_no_prot for examples of queries with ROLLUP which
      are accepted or not.
    */
    return false;
  }

  // no need to search for keys in those tables:
  table_map tested_map_for_keys = whole_tables_fd;
  recheck_nullable_keys = 0;
  while (true) {
    // build En+1
    const table_map last_whole_tables_fd = whole_tables_fd;
    for (uint j = 0; j < fd.size(); j++) {
      Item *const item2 = fd.at(j)->real_item();  // Go down view field
      if (item2->type() != Item::FIELD_ITEM) continue;

      Item_field *const item_field = down_cast<Item_field *>(item2);
      /**
        @todo make table_ref non-NULL for gcols, then use it for 'tl'.
        Do the same in Item_field::used_tables_for_level().
      */
      TABLE_LIST *const tl = item_field->field->table->pos_in_table_list;

      if (tested_map_for_keys & tl->map()) continue;
      tested_map_for_keys |= tl->map();
      for (uint keyno = 0; keyno < tl->table->s->keys; keyno++) {
        KEY *const key_info = &tl->table->key_info[keyno];
        if (!(key_info->flags & HA_NOSAME)) continue;
        uint k;
        for (k = 0; k < key_info->user_defined_key_parts; k++) {
          const Field *const key_field = key_info->key_part[k].field;
          bool key_field_in_fd = false;
          for (uint l = 0; l < fd.size(); l++) {
            Item *const item3 = fd.at(l)->real_item();  // Go down view field
            if (item3->type() != Item::FIELD_ITEM) continue;
            if (static_cast<Item_field *>(item3)->field == key_field &&
                // Not a nullable column, or can be treated as not nullable
                (!key_field->is_nullable() ||
                 item3->marker == Item::MARKER_FUNC_DEP_NOT_NULL)) {
              key_field_in_fd = true;
              break;
            }
          }
          if (!key_field_in_fd) break;
        }
        if (k == key_info->user_defined_key_parts) {
          /*
            We just found that intersect(En,table.*) contains all columns of
            the key, so intersect(En,table.*) -> table.* in 'table'.
            This is key-based so is an NFFD, so it propagates to the result of
            the WHERE clause. Thus, intersect(En,table.*) -> table.* in this
            result, so En -> table.* in this result.
            We knew that E1 -> En in this result.
            So, E1 -> table.* there too. So we can add table.* to En+1:
          */
          add_to_fd(tl->map());
          break;
        }
      }
    }
    if (last_whole_tables_fd !=
            whole_tables_fd &&  // something new, check again
        is_in_fd(item))
      return true;

    // Build En+2
    uint last_fd = fd.size();

    find_fd_in_joined_table(select->join_list);  // [OUTER] JOIN ON

    if (select->where_cond())  // WHERE
      find_fd_in_cond(select->where_cond(), 0, false);

    table_map map_of_new_fds = recheck_nullable_keys;
    recheck_nullable_keys = 0;

    for (; last_fd < fd.size(); ++last_fd)
      map_of_new_fds |= fd.at(last_fd)->used_tables();

    /*
      When we built map_of_new_fds we may have used columns from merged views.
      Such column is properly local to 'select', and in general it
      wraps an expression of columns of merged tables (local tables) and of
      other objects (e.g. outer references, if this view is rather a derived
      table containing an outer reference). Only local tables are of interest:
    */
    map_of_new_fds &= ~PSEUDO_TABLE_BITS;
    if (map_of_new_fds != 0)  // something new, check again
    {
      if (is_in_fd(item)) return true;
      // Recheck keys only in tables with something new:
      tested_map_for_keys &= ~map_of_new_fds;
    } else {
      // If already searched in expressions underlying identifiers.
      if (search_in_underlying) return false;

      // Otherwise, iterate once more and dig deeper.
      search_in_underlying = true;

      if (is_in_fd(item)) return true;
    }
  }  // while(true)
}

/*
  Record that an expression is uniquely determined by source columns.

  @param  item  Expression to add.
  @param  local_column  True: it is a column, should be added to 'fd'. False:
  it cannot be added to 'fd' but we can still derive some useful knowledge -
  see the function's body.
  @param  add_to_mat_table True: we should check if it's a column of a mat
  table and if so we should pass it to the child Group_check.
*/
void Group_check::add_to_fd(Item *item, bool local_column,
                            bool add_to_mat_table) {
  /*
    Because the "fd" list is limited to columns and because MySQL allows
    non-column expressions in GROUP BY (unlike the standard), we need this
    block _even_if_ this is not a column. For example:
      select d.s from
        (select b*3 as c, sum(a) as s from t1 group by b*3) as d
      group by d.c;
    Say that we are validating the outer query. d.c is a group column,
    containing the value of t1.b*3; say that we are presently telling the
    (child) Group_check of the subquery that t1.b*3 is in its source (in
    other words, the value of t1.b*3 can be considered as determined).
    t1.b*3 is not a column, so it cannot be put into "fd", however it's the
    group expression, so it determines sum(a), and so d.* is determined.
    In this corner case, a "source" can be a source _expression_, not
    column.
  */
  find_group_in_fd(item);

  if (!local_column) return;

  /*
    A column reference can later give more FDs, record it.

    You may wonder why, if this is a merged view item (Item_*view_ref), we add
    it to "fd" instead of adding the underlying item. Here is why:
    create view v1 as select t1.i*2 as z from t1;
    select v1.z*5 from v1 group by v1.z;
    When validating z*5, we need to find z in "fd". So:
    - either we have added z to "fd" here (chosen solution), then we search
    for "z" and match.
    - or we have added the real_item of "z", t1.i*2, to "fd" here, then we
    search for the real_item of "z", t1.i*2 and match. However, matching
    (using Item::eq()) of certain items is not working well (if the item
    contains a subquery), which would lead to some incorrect rejections of
    queries. Moreover, it is good to stick to the definition of what a
    functionally dependent column can be: it can be a view's column.
  */

  fd.push_back(down_cast<Item_ident *>(item));

  if (!add_to_mat_table) return;

  item = item->real_item();  // for merged view containing mat table
  if (item->type() == Item::FIELD_ITEM) {
    Item_field *const item_field = (Item_field *)item;
    TABLE_LIST *const tl = item_field->field->table->pos_in_table_list;
    if (tl->uses_materialization() &&  // materialized table
        !tl->is_table_function())      // there's no underlying query expr
      add_to_source_of_mat_table(item_field, tl);
  }
}

/**
   This function must be called every time we discover an item which is FD on
   source columns, or add a bit to whole_tables_fd; it maintains group_in_fd.

   @param item  item which is FD; if NULL, means that we instead added a bit
   to whole_tables_fd.
*/
void Group_check::find_group_in_fd(Item *item) {
  if (group_in_fd == ~0ULL) return;  // nothing to do
  if (select->is_grouped()) {
    /*
      See if we now have all of query expression's GROUP BY list; an
      implicitely grouped query has an empty group list.
    */
    bool missing = false;
    int j = 0;
    for (ORDER *grp = select->group_list.first; grp; ++j, grp = grp->next) {
      if (!(group_in_fd & (1ULL << j))) {
        Item *grp_item = *grp->item;
        if ((local_column(grp_item) &&
             (grp_item->used_tables() & ~whole_tables_fd) == 0) ||
            (item && grp_item->eq(item, false)))
          group_in_fd |= (1ULL << j);
        else
          missing = true;
      }
    }
    if (!missing) {
      /*
        All GROUP BY exprs are FD on the source. Turn all bits on, for easy
        testing.
      */
      group_in_fd = ~0ULL;
    }
  }
}

/**
   @returns the idx-th expression in the SELECT list of our query block.
*/
Item *Group_check::select_expression(uint idx) {
  List_iterator<Item> it_select_list_of_subq(*select->get_item_list());
  Item *expr_under = nullptr;
  for (uint k = 0; k <= idx; k++) expr_under = it_select_list_of_subq++;
  DBUG_ASSERT(expr_under);
  return expr_under;
}

/**
   If we just added a column of a materialized table to 'fd', we record this
   fact in a new Group_check (mat_gc) for the query expression underlying that
   table. This can later help us derive new functional dependencies in our
   Group_check. For example:
     select d.a, d.b from (select t.a*2 as a, t.a as b from t) group by d.b;
   When we add d.b to 'fd', in this function we create mat_gc, see that d.b is
   built from a column of t (t.b), we can say that "t.b is determined", so we
   add t.b to mat_gc.fd. Later, when we wonder if d.a is functionally
   dependent on d.b, we process d.a in is_in_fd_of_underlying():
   we analyze 2*t.a in the context of mat_gc: 2*t.a is FD on t.a, we
   conclude that d.a is indeed FD on d.b.

   @param  item_field  column of 'tl', just added to 'fd'
   @param  tl          mat table
*/
void Group_check::add_to_source_of_mat_table(Item_field *item_field,
                                             TABLE_LIST *tl) {
  SELECT_LEX_UNIT *const mat_unit = tl->derived_unit();
  // Query expression underlying 'tl':
  SELECT_LEX *const mat_select = mat_unit->first_select();
  if (mat_unit->is_union() || mat_select->olap != UNSPECIFIED_OLAP_TYPE)
    return;  // If UNION or ROLLUP, no FD
  // Grab Group_check for this subquery.
  Group_check *mat_gc = nullptr;
  uint j;
  for (j = 0; j < mat_tables.size(); j++) {
    mat_gc = mat_tables.at(j);
    if (mat_gc->select == mat_select) break;
  }
  if (j == mat_tables.size())  // not found, create it
  {
    mat_gc = new (m_root) Group_check(mat_select, m_root, tl);
    mat_tables.push_back(mat_gc);
  }
  // Find underlying expression of item_field, in SELECT list of mat_select
  Item *const expr_under =
      mat_gc->select_expression(item_field->field->field_index);

  // non-nullability of tl's column in tl, is equal to that of expr_under.
  if (expr_under && !expr_under->maybe_null) mat_gc->non_null_in_source = true;

  mat_gc->add_to_fd(expr_under, mat_gc->local_column(expr_under));

  if (mat_gc->group_in_fd == ~0ULL &&                   // (1)
      (!(mat_gc->table->map() & select->outer_join) ||  // (2)
       mat_gc->non_null_in_source))                     // (3)
  {
    /*
      (1): In mat_gc, all GROUP BY expressions of mat_select are dependent on
      source columns. Thus, all SELECT list expressions are, too (otherwise,
      the validation of mat_select has or will fail). So, in our Group_check,
      intersect(En, tl.*) -> tl.* .
      This FD needs to propagate in our Group_check all the way up to the
      result of the WHERE clause. It does, if:
      - either there is no weak side above this table (2) (so NFFD is not
      needed).
      - or intersect(En, tl.*) contains a non-nullable column (3) (then
      the FD is NFFD).

      VE does not need to be deterministic: there is only one row per values
      of group columns; if those values are known, then any VE, even rand(),
      is uniquely determined.
    */
    add_to_fd(tl->map());
  }
}

/**
   is_in_fd() is low-level compared to is_fd_on_source(). The former only
   searches through built FD information; the latter builds this information
   and calls the former to search in it.

   @param  item  Item to consider; either a column local to 'select', or a set
   function whose aggregation query is 'select'

   @returns true if the expression is FD on the source.
 */
bool Group_check::is_in_fd(Item *item) {
  if ((item->type() == Item::SUM_FUNC_ITEM && !item->m_is_window_function) ||
      (item->type() == Item_func::FUNC_ITEM &&
       (((Item_func *)item)->functype() == Item_func::GROUPING_FUNC))) {
    /*
      If all group expressions are FD on the source, this set function also is
      (one single value per group).
    */
    return group_in_fd == ~0ULL;
  }

  DBUG_ASSERT(local_column(item));
  Used_tables ut(select);
  (void)item->walk(&Item::used_tables_for_level, enum_walk::POSTFIX,
                   pointer_cast<uchar *>(&ut));
  if ((ut.used_tables & ~whole_tables_fd) == 0) {
    /*
      The item is a column from a table whose all columns are FD.
      If the table is a view, the item wraps an expression, which
      uses columns of underlying tables which are all FD; we don't even have
      to walk the underlying expression.
      An expression-based FD in a view is not necessarily an NFFD, but here it
      is, as the bits in whole_tables_fd are on only if the determinant
      columns are non-NULLable or there is no weak side upwards (see calls to
      add_to_fd(table_map)).
    */
    return true;
  }
  for (uint j = 0; j < fd.size(); j++) {
    Item *const item2 = fd.at(j);
    if (item2->eq(item, false)) return true;
    /*
      Say that we have view:
      create view v1 as select i, 2*i as z from t1; and we do:
      select z from v1 group by i;
      v1 is merged.
      v1.i is Item_*view_ref to t1.i;
      v1.z is Item_*view_ref to Item_func_mul which has two arguments:
      Item_int (2) and Item_field (t1.i).
      We added the grouping column v1.i to "fd". Now we are walking v1.z: we
      meet Item_field (t1.i). For us to find this t1.i in "fd" we have to
      reach to real_item() of v1.i.
    */
    Item *const real_it2 = item2->real_item();
    if (real_it2 != item2 && real_it2->eq(item, false)) return true;
  }
  if (!search_in_underlying) return false;
  return is_in_fd_of_underlying(down_cast<Item_ident *>(item));
}

/**
   See if we can derive a FD from a column which has an underlying expression.

   For a generated column, see if we can derive a FD from its expression.
   For a column of a view or derived table, see if we can derive a FD from the
   underlying query block.

   @param  item  column
   @returns true  if this column is FD on source
*/
bool Group_check::is_in_fd_of_underlying(Item_ident *item) {
  if (item->type() == Item::REF_ITEM) {
    /*
      It's a merged view's item.
      Consider
        create view v1 as select as as a, a*2 as b from t1;
        select v1.b from v1 group by v1.a;
      we have this->fd={v1.a}, and we search if v1.b is FD on v1.a. We'll look
      if t1.a*2 is FD on t1.a.
    */
    DBUG_ASSERT(static_cast<const Item_ref *>(item)->ref_type() ==
                Item_ref::VIEW_REF);
    /*
      Refuse RAND_TABLE_BIT because:
      - FDs in a view are those of the underlying query expression.
      - For FDs in a query expression, expressions in the SELECT list must be
      deterministic.
      Same is true for materialized tables further down.
    */
    if (item->used_tables() & RAND_TABLE_BIT) return false;

    Item *const real_it = item->real_item();
    Used_tables ut(select);
    (void)item->walk(&Item::used_tables_for_level, enum_walk::POSTFIX,
                     pointer_cast<uchar *>(&ut));
    /*
      todo When we eliminate all uses of cached_table, we can probably add a
      derived_table_ref field to Item_view_ref objects and use it here.
    */
    TABLE_LIST *const tl = item->cached_table;
    DBUG_ASSERT(tl->is_view_or_derived());
    /*
      We might find expression-based FDs in the result of the view's query
      expression; but if this view is on the weak side of an outer join,
      the FD won't propagate to that outer join's result.
    */
    const bool weak_side_upwards = tl->is_inner_table_of_outer_join();

    /*
      (3) real_it is a deterministic expression of columns which are all FD on
      the source. This gives a FD in the view, maybe not NFFD. It propagates
      to our query expression if:
      (1) Either there is no weak side upwards (NFFD not needed)
      (2) Or NULLness of columns implies NULLness of expression (so it's
      NFFD).
    */
    if ((!weak_side_upwards ||                              // (1)
         (ut.used_tables & real_it->not_null_tables())) &&  // (2)
        !real_it->walk(&Item::is_column_not_in_fd, walk_options,
                       pointer_cast<uchar *>(this)))  // (3)
    {
      add_to_fd(item, true);
      return true;
    }
  }

  else if (item->type() == Item::FIELD_ITEM) {
    Item_field *const item_field = down_cast<Item_field *>(item);
    /**
      @todo make table_ref non-NULL for gcols, then use it for 'tl'.
      Do the same in Item_field::used_tables_for_level().
    */
    TABLE_LIST *const tl = item_field->field->table->pos_in_table_list;
    if (item_field->field->is_gcol())  // Generated column
    {
      DBUG_ASSERT(!tl->uses_materialization());
      Item *const expr = item_field->field->gcol_info->expr_item;
      DBUG_ASSERT(expr->fixed);
      Used_tables ut(select);
      item_field->used_tables_for_level(pointer_cast<uchar *>(&ut));
      const bool weak_side_upwards = tl->is_inner_table_of_outer_join();
      if ((!weak_side_upwards || (ut.used_tables & expr->not_null_tables())) &&
          !expr->walk(&Item::is_column_not_in_fd, walk_options,
                      pointer_cast<uchar *>(this))) {
        add_to_fd(item, true);
        return true;
      }
    } else if (tl->uses_materialization() &&  // Materialized derived table
               !tl->is_table_function()) {
      SELECT_LEX *const mat_select = tl->derived_unit()->first_select();
      uint j;
      for (j = 0; j < mat_tables.size(); j++) {
        if (mat_tables.at(j)->select == mat_select) break;
      }
      if (j < mat_tables.size())  // if false, we know nothing about this table
      {
        Group_check *const mat_gc = mat_tables.at(j);
        /*
          'item' belongs to a materialized table, and certain fields of the
          subquery are in this->fd.
          Search if the expression inside 'item' is FD on them.
        */
        Item *const expr_under =
            mat_gc->select_expression(item_field->field->field_index);
        /*
          expr_under is the expression underlying 'item'.
          (1) and (4) it is a deterministic expression of mat_gc source
          columns, so is FD on them. This gives a FD in the mat table, maybe
          not NFFD: intersect(En, tl.*) -> item .
          This FD needs to propagate in our Group_check all the way up to the
          result of the WHERE clause. It does, if:
          - either there is no weak side above this table (2) (so NFFD is not
          needed).
          - or intersect(En, tl.*) contains a non-nullable column (3) (then
          the FD is NFFD).
        */
        if (!(expr_under->used_tables() & RAND_TABLE_BIT) &&  // (1)
            (!(mat_gc->table->map() & select->outer_join) ||  // (2)
             mat_gc->non_null_in_source) &&                   // (3)
            !expr_under->walk(&Item::aggregate_check_group,   // (4)
                              walk_options, pointer_cast<uchar *>(mat_gc))) {
          /*
            We pass add_to_mat_table==false otherwise add_to_fd() may add
            expr_under (if it's a field) to mat_gc->fd, uselessly (it is
            already in mat_gc->fd, as walk() succeeded above). This is just to
            not make the 'fd' list longer than needed.
          */
          add_to_fd(item_field, true, false);
          return true;
        }
      }
    }
  }

  return false;
}

/**
  @returns an element of 'fd' array equal to 'item', or nullptr if not found.
  @param item Item to search for.
*/
Item *Group_check::get_fd_equal(Item *item) {
  for (uint j = 0; j < fd.size(); j++) {
    Item *const item2 = fd.at(j);
    if (item2->eq(item, false)) return item2;
    Item *const real_it2 = item2->real_item();
    if (real_it2 != item2 && real_it2->eq(item, false)) return item2;
  }
  return nullptr;
}

/**
   Searches for equality-based functional dependences in an AND-ed part of a
   condition (a conjunct).
   Search for columns which are known-not-nullable due to the conjunct.

   @param  cond        complete condition
   @param  conjunct    one AND-ed part of 'cond'
   @param  weak_tables  If condition is WHERE, it's 0. Otherwise it is the map
   of weak tables in the join nest which owns the condition.
   @param  weak_side_upwards If condition is WHERE it's false. Otherwise it is
   true if the join nest owning this condition is embedded in the weak side
   of some parent outer join (no matter how far up the parent is).
*/
void Group_check::analyze_conjunct(Item *cond, Item *conjunct,
                                   table_map weak_tables,
                                   bool weak_side_upwards) {
  if (conjunct->type() != Item::FUNC_ITEM) return;
  const Item_func *cnj = static_cast<const Item_func *>(conjunct);
  if (cnj->functype() == Item_func::EQ_FUNC) {
    Item *left_item = cnj->arguments()[0];
    Item *right_item = cnj->arguments()[1];
    if (left_item->type() == Item::ROW_ITEM &&
        right_item->type() == Item::ROW_ITEM) {
      /*
        (a,b)=(c,d) is equivalent to 'a=c and b=d', let's iterate on pairs.
        Note that it's not recursive: we don't handle (a,(b,c))=(d,(e,f)), the
        Standard does not seem to require it.
      */
      Item_row *left_row = down_cast<Item_row *>(left_item);
      Item_row *right_row = down_cast<Item_row *>(right_item);
      int elem = left_row->cols();
      while (--elem >= 0)
        analyze_scalar_eq(cond, left_row->element_index(elem),
                          right_row->element_index(elem), weak_tables,
                          weak_side_upwards);
    } else
      analyze_scalar_eq(cond, left_item, right_item, weak_tables,
                        weak_side_upwards);
  }
  /*
    'cnj' can be a non-equality and still reject NULLs for a certain column,
    which can help us discover known-not-null columns. @see OUTEREQ in
    aggregate_check.h for an explanation.
    For example: = < > <> >= <= and IS NOT NULL.
  */
  const table_map not_null_tables = cnj->not_null_tables();
  if (!not_null_tables) return;
  auto functype = cnj->functype();
  if (functype == Item_func::NOT_FUNC ||    // to handle e.g. col NOT LIKE
      functype == Item_func::ISTRUTH_FUNC)  // and e.g. (col>3) IS TRUE
  {
    conjunct = cnj->arguments()[0];
    if (conjunct->type() != Item::FUNC_ITEM) return;
    cnj = static_cast<const Item_func *>(conjunct);  // Dive in NOT's argument.
    /*
      We intentionally keep not_null_tables of the NOT, as we're interested in
      what makes the NOT not true, not what makes NOT's argument not true.
    */
  }
  for (Item **parg = cnj->arguments(),
            **parg_end = parg + cnj->argument_count();
       parg != parg_end; parg++) {
    Item *const arg = *parg;
    const table_map used_tables = arg->used_tables();
    Item *arg_in_fd;
    /*
      Check if:
      (1) it's a local column,
      (2) a NULL value for this column makes the function return FALSE or
      UNKNOWN; note that we do not dive into each argument, so will not get
      into complicated cases like: coalesce(t1.a,2)+t1.b=3 ('cnj' is '='
      here), and can thus be sure that not_null_tables() is a good enough
      test; if we dove into the '+' argument, we may end up finding 'a' and
      thinking a NULL value of 'a' makes "=" UNKNOWN (while it's a NULL
      value of 'b' which does and explains the presence of t1 in
      not_null_tables()). There is only one case where we dive because it's
      easy, it's NOT and IS TRUE/FALSE.
      (3) the condition is in WHERE, or is an outer join condition and the
      column's table is on weak side.
      (4) the column is represented by some Item_ident in the FD list.
      If so, we mark the said Item as "not nullable in its base table", and
      we ask for a re-check of unique indexes of this table.
    */
    if (!(used_tables & not_null_tables) ||               // (2)
        (weak_tables && !(used_tables & weak_tables)) ||  // (3)
        !local_column(arg) ||                             // (1)
        !(arg_in_fd = get_fd_equal(arg)))                 // (4)
      continue;
    /*
      Mark the item as "can be treated as not nullable in its
      table".
      We are not going to mark 'arg'; consider:
       SELECT ... WHERE a IS NOT NULL GROUP BY a;
      the Item 'a' in the IS NOT NULL predicate is not part of 'fd', it is
      invisible to the rest of the FD-detection logic (e.g. the logic which
      looks at unique keys); what matters is marking one copy
      present in 'fd': 'arg_in_fd' (here the Item 'a' of GROUP BY).
    */
    if (arg_in_fd->marker != Item::MARKER_FUNC_DEP_NOT_NULL) {
      // If a merged view's column, mark the underlying expression too
      arg_in_fd->marker = arg_in_fd->real_item()->marker =
          Item::MARKER_FUNC_DEP_NOT_NULL;
      recheck_nullable_keys |= used_tables;
    }
  }
}

/**
   Helper function @see analyze_conjunct().
*/
void Group_check::analyze_scalar_eq(Item *cond, Item *left_item,
                                    Item *right_item, table_map weak_tables,
                                    bool weak_side_upwards) {
  table_map left_tables = left_item->used_tables();
  table_map right_tables = right_item->used_tables();
  bool left_is_column = local_column(left_item);
  bool right_is_column = local_column(right_item);

  /*
    We look for something=col_not_FD.
    If there are weak tables, this column must be weak (something=strong gives
    us nothing, in an outer join condition).
  */
  if (right_is_column && (!weak_tables || (weak_tables & right_tables)) &&
      !is_in_fd(right_item)) {
  } else if (left_is_column && (!weak_tables || (weak_tables & left_tables)) &&
             !is_in_fd(left_item)) {
    // col_not_FD=something: change to something=col_not_FD
    std::swap(left_item, right_item);
    std::swap(left_tables, right_tables);
    std::swap(left_is_column, right_is_column);
  } else
    return;  // this equality brings nothing

  // right_item is a column not in fd, see if we can add it.

  if (left_is_column && (weak_tables & left_tables) && is_in_fd(left_item)) {
    // weak=weak: left->right, and this is NFFD
    add_to_fd(right_item, true);
    return;
  }

  const table_map strong_tables = (~weak_tables) & ~PSEUDO_TABLE_BITS;

  if ((left_is_column && (strong_tables & left_tables) &&
       is_in_fd(left_item)) ||
      left_item->const_item() || (OUTER_REF_TABLE_BIT & left_tables)) {
    // strong_or_literal_or_outer_ref= right_item

    if (!weak_tables) {
      /*
        It cannot be an inner join, due to transformations done in
        simplify_joins(). So it is WHERE, so right_item is strong.
        This may be constant=right_item and thus not be an NFFD, but WHERE is
        exterior to join nests so propagation is not needed.
      */
      DBUG_ASSERT(!weak_side_upwards);  // cannot be inner join
      add_to_fd(right_item, true);
    } else {
      // Outer join. So condition must be deterministic.
      if (cond->used_tables() & RAND_TABLE_BIT) return;

      /*
        FD will have DJS as source columns, where DJS is the set of strong
        columns referenced by "cond". FD has to propagate. It does if:
        - either there is no weak side upwards
        - or NULLness of DJS columns implies that "cond" is not true.
      */
      if (weak_side_upwards && !(strong_tables & cond->not_null_tables()))
        return;

      std::pair<Group_check *, table_map> p(this, strong_tables);
      if (!cond->walk(&Item::is_strong_side_column_not_in_fd, walk_options,
                      (uchar *)&p)) {
        /*
          "cond" is deterministic.
          right_item is weak.
          strong_or_literal_or_outer= weak.
          So DJS->right_item holds in the result of the join, and it
          propagates.
          As DJS is FD on E1 (the walk() succeeded), E1->right_item in the
          result of WHERE.
        */
        add_to_fd(right_item, true);
      }
    }
  }
}

/**
   Searches for equality-based functional dependencies in a condition.

   @param  cond  condition: a WHERE condition or JOIN condition.
   @param  weak_tables  If condition is WHERE, it's 0. Otherwise it is the map
   of weak tables in the join nest which owns the condition.
   @param  weak_side_upwards If condition is WHERE it's false. Otherwise it is
   true if the join nest owning this condition is embedded in the right side
   of some parent left join.
*/
void Group_check::find_fd_in_cond(Item *cond, table_map weak_tables,
                                  bool weak_side_upwards) {
  if (cond->type() == Item::COND_ITEM) {
    Item_cond *cnd = static_cast<Item_cond *>(cond);
    /*
      All ANDs already flattened, see:
      "(X1 AND X2) AND (Y1 AND Y2) ==> AND (X1, X2, Y1, Y2)"
      in sql_yacc, and also Item_cond::fix_fields().
    */
    if (cnd->functype() != Item_func::COND_AND_FUNC) return;
    List_iterator<Item> li(*(cnd->argument_list()));
    Item *item;
    while ((item = li++))
      analyze_conjunct(cond, item, weak_tables, weak_side_upwards);
  } else  // only one conjunct
    analyze_conjunct(cond, cond, weak_tables, weak_side_upwards);
}

/**
   Searches for equality-based functional dependencies in the condition of a
   join nest, and recursively in all contained join nests.

   @param  join_list  members of the join nest
*/
void Group_check::find_fd_in_joined_table(
    mem_root_deque<TABLE_LIST *> *join_list) {
  for (const TABLE_LIST *table : *join_list) {
    if (table->is_sj_or_aj_nest()) {
      /*
        We can ignore this nest as:
        - the subquery's WHERE was copied to the semi-join condition
        - so where the IN equalities
        - sj_cond() is also present in the parent nest's join condition or in
        the query block's WHERE condition, which we check somewhere else.
        Note that columns from sj-inner tables can help only as "intermediate
        link" in the graph of functional dependencies, as they are neither in
        GROUP BY (the source) nor in the SELECT list / HAVING / ORDER BY (the
        target).
        If this is an antijoin nest, it's a NOT, which doesn't bring any
        functional dependency.
      */
      continue;
    }
    table_map used_tables;
    NESTED_JOIN *nested_join = table->nested_join;
    if (nested_join) {
      find_fd_in_joined_table(&nested_join->join_list);
      used_tables = nested_join->used_tables;
    } else
      used_tables = table->map();
    if (table->join_cond()) {
      /*
        simplify_joins() moves the ON condition of an inner join to the closest
        outer join nest or to WHERE. So this assertion should hold.
        Thus, used_tables are weak tables.
      */
      DBUG_ASSERT(table->outer_join);
      /*
        We might find equality-based FDs in the result of this outer join; but
        if this outer join is itself on the weak side of a parent outer join,
        the FD won't propagate to that parent outer join's result.
      */
      const bool weak_side_upwards =
          table->embedding && table->embedding->is_inner_table_of_outer_join();
      find_fd_in_cond(table->join_cond(), used_tables, weak_side_upwards);
    }
  }
}

/// Writes "check information" to the optimizer trace
void Group_check::to_opt_trace(THD *thd) {
  if (fd.empty() && !whole_tables_fd) return;
  Opt_trace_context *ctx = &thd->opt_trace;
  if (!ctx->is_started()) return;
  Opt_trace_object trace_wrapper(ctx);
  Opt_trace_object trace_fds(ctx, "functional_dependencies_of_GROUP_columns");
  to_opt_trace2(ctx, &trace_fds);
}

/**
   Utility function for to_opt_trace(), as we need recursion in children
   Group_checks. to_opt_trace() only writes a one-time header.
*/
void Group_check::to_opt_trace2(Opt_trace_context *ctx,
                                Opt_trace_object *parent) {
  if (table) parent->add_utf8_table(table);
  if (whole_tables_fd) {
    Opt_trace_array array(ctx, "all_columns_of_table_map_bits");
    for (uint j = 0; j < MAX_TABLES; j++)
      if (whole_tables_fd & (1ULL << j)) array.add(j);
  }
  if (!fd.empty()) {
    Opt_trace_array array(ctx, "columns");
    for (uint j = 0; j < fd.size(); j++) array.add_utf8(fd.at(j)->full_name());
  }
  if (is_child()) {
    if (group_in_fd == ~0ULL && select->is_explicitly_grouped())
      parent->add("all_group_expressions", true);
  }
  if (!mat_tables.empty()) {
    Opt_trace_array array(ctx, "searched_in_materialized_tables");
    for (uint j = 0; j < mat_tables.size(); j++) {
      Opt_trace_object trace_wrapper(ctx);
      mat_tables.at(j)->to_opt_trace2(ctx, &trace_wrapper);
    }
  }
}

/**
   Does one low-level check on one Item_ident. Called by Item_ident walk
   processors.
   @param i     item to check
   @param tm    map of strong tables, if type==CHECK_STRONG_SIDE_COLUMN
   @param type  check to do:
      - CHECK_GROUP for Item_ident::aggregate_check_group()
      - CHECK_STRONG_SIDE_COLUMN for
      Item_ident::is_strong_side_column_not_in_fd()
      - CHECK_COLUMN for Item_ident::is_column_not_in_fd()
   @returns false if success
*/
bool Group_check::do_ident_check(Item_ident *i, table_map tm,
                                 enum enum_ident_check type) {
  if (is_stopped(i)) return false;

  const Bool3 local = i->local_column(select);
  if (local.is_false()) goto ignore_children;

  if (local.is_unknown()) return false;  // dive in child item

  switch (type) {
    case CHECK_GROUP:
      if (i->type() == Item::FIELD_ITEM &&
          down_cast<Item_field *>(i)->table_ref->m_was_scalar_subquery)
        return false;
      if (!is_fd_on_source(i)) {
        // It is not FD on source columns:
        if (!is_child()) failed_ident = i;
        return true;
      }
      goto ignore_children;
    case CHECK_STRONG_SIDE_COLUMN: {
      Used_tables ut(select);
      (void)i->walk(&Item::used_tables_for_level, enum_walk::POSTFIX,
                    pointer_cast<uchar *>(&ut));
      if ((ut.used_tables & tm) && !is_in_fd(i))
        return true;  // It is a strong-side column and not FD
      goto ignore_children;
    }
    case CHECK_COLUMN:
      if (!is_in_fd(i)) return true;  // It is a column and not FD
      goto ignore_children;
  }

ignore_children:
  stop_at(i);
  return false;
}

/// @} (end of group AGGREGATE_CHECKS ONLY_FULL_GROUP_BY)
