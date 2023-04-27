/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/sql_opt_exec_shared.h
  Common types of the Optimizer, used by optimization and execution.
*/

#ifndef SQL_OPT_EXEC_SHARED_INCLUDED
#define SQL_OPT_EXEC_SHARED_INCLUDED

#include "my_base.h"
#include "my_dbug.h"
#include "sql/item.h"

class JOIN;
class Item_func_match;
class store_key;
struct POSITION;
class QUICK_SELECT_I;

/**
   This represents the index of a JOIN_TAB/QEP_TAB in an array. "plan_idx":
   "Plan Table Index". It is signed, because:
   - firstmatch_return may be PRE_FIRST_PLAN_IDX (it can happen that the first
   table of the plan uses FirstMatch: SELECT ... WHERE literal IN (SELECT
   ...)).
   - it must hold the invalid value NO_PLAN_IDX (which means "no
   JOIN_TAB/QEP_TAB", equivalent of NULL pointer); this invalid value must
   itself be different from PRE_FIRST_PLAN_IDX, to distinguish "FirstMatch to
   before-first-table" (firstmatch_return==PRE_FIRST_PLAN_IDX) from "No
   FirstMatch" (firstmatch_return==NO_PLAN_IDX).
*/
typedef int8 plan_idx;
#define NO_PLAN_IDX (-2)  ///< undefined index
#define PRE_FIRST_PLAN_IDX \
  (-1)  ///< right before the first (first's index is 0)

struct TABLE_REF {
  bool key_err;
  uint key_parts;    ///< num of ...
  uint key_length;   ///< length of key_buff
  int key;           ///< key no
  uchar *key_buff;   ///< value to look for with key
  uchar *key_buff2;  ///< key_buff+key_length
  /**
     Used to store the value from each keypart field. These values are
     used for ref access. If key_copy[key_part] == NULL it means that
     the value is constant and does not need to be reevaluated
  */
  store_key **key_copy;
  Item **items;  ///< val()'s for each keypart
  /*
    Array of pointers to trigger variables. Some/all of the pointers may be
    NULL.  The ref access can be used iff

      for each used key part i, (!cond_guards[i] || *cond_guards[i])

    This array is used by subquery code. The subquery code may inject
    triggered conditions, i.e. conditions that can be 'switched off'. A ref
    access created from such condition is not valid when at least one of the
    underlying conditions is switched off (see subquery code for more details).
    If a table in a subquery has this it means that the table access
    will switch from ref access to table scan when the outer query
    produces a NULL value to be checked for in the subquery. This will
    be used by NOT IN subqueries and IN subqueries which need to distinguish
    NULL and FALSE, where ignore_unknown() is false.
  */
  bool **cond_guards;
  /**
    @code (null_rejecting & (1<<i)) @endcode means the condition is '=' and no
    matching rows will be produced if items[i] IS NULL (see
    add_not_null_conds())
  */
  key_part_map null_rejecting;
  table_map depend_map;  ///< Table depends on these tables.
  /*
    NULL byte position in the key_buf (if set, the key is taken to be NULL);
    normally points to the first byte in the buffer. Used for REF_OR_NULL
    lookups.
   */
  uchar *null_ref_key;
  /*
    The number of times the record associated with this key was used
    in the join.
  */
  ha_rows use_count;

  /*
    true <=> disable the "cache" as doing lookup with the same key value may
    produce different results (because of Index Condition Pushdown)
  */
  bool disable_cache;

  /*
    If non-nullptr, all the fields are hashed together through functions
    in store_key (with the result being put into this field), as opposed to
    being matched against individual fields in the associated KEY's key parts.
   */
  ulonglong *keypart_hash = nullptr;

  TABLE_REF()
      : key_err(true),
        key_parts(0),
        key_length(0),
        key(-1),
        key_buff(nullptr),
        key_buff2(nullptr),
        key_copy(nullptr),
        items(nullptr),
        cond_guards(nullptr),
        null_rejecting(0),
        depend_map(0),
        null_ref_key(nullptr),
        use_count(0),
        disable_cache(false) {}

  /**
    @returns whether the reference contains NULL values which could never give
    a match.
  */
  bool impossible_null_ref() const {
    if (null_rejecting != 0) {
      for (uint i = 0; i < key_parts; i++) {
        if ((null_rejecting & 1 << i) && items[i]->is_null()) return true;
      }
    }
    return false;
  }

  /**
    Check if there are triggered/guarded conditions that might be
    'switched off' by the subquery code when executing 'Full scan on
    NULL key' subqueries.

    @return true if there are guarded conditions, false otherwise
  */

  bool has_guarded_conds() const {
    DBUG_ASSERT(key_parts == 0 || cond_guards != nullptr);

    for (uint i = 0; i < key_parts; i++) {
      if (cond_guards[i]) return true;
    }
    return false;
  }
};

struct CACHE_FIELD;
class QEP_operation;
class Filesort;
class Semijoin_mat_exec;

/*
  The structs which holds the join connections and join states
*/
enum join_type {
  /* Initial state. Access type has not yet been decided for the table */
  JT_UNKNOWN,
  /* Table has exactly one row */
  JT_SYSTEM,
  /*
    Table has at most one matching row. Values read
    from this row can be treated as constants. Example:
    "WHERE table.pk = 3"
   */
  JT_CONST,
  /*
    '=' operator is used on unique index. At most one
    row is read for each combination of rows from
    preceding tables
  */
  JT_EQ_REF,
  /*
    '=' operator is used on non-unique index
  */
  JT_REF,
  /*
    Full table scan.
  */
  JT_ALL,
  /*
    Range scan.
  */
  JT_RANGE,
  /*
    Like table scan, but scans index leaves instead of
    the table
  */
  JT_INDEX_SCAN,
  /* Fulltext index is used */
  JT_FT,
  /*
    Like ref, but with extra search for NULL values.
    E.g. used for "WHERE col = ... OR col IS NULL"
   */
  JT_REF_OR_NULL,
  /*
    Do multiple range scans over one table and combine
    the results into one. The merge can be used to
    produce unions and intersections
  */
  JT_INDEX_MERGE
};

/// Holds members common to JOIN_TAB and QEP_TAB.
class QEP_shared {
 public:
  QEP_shared()
      : m_join(nullptr),
        m_idx(NO_PLAN_IDX),
        m_table(nullptr),
        m_position(nullptr),
        m_sj_mat_exec(nullptr),
        m_first_sj_inner(NO_PLAN_IDX),
        m_last_sj_inner(NO_PLAN_IDX),
        m_first_inner(NO_PLAN_IDX),
        m_last_inner(NO_PLAN_IDX),
        m_first_upper(NO_PLAN_IDX),
        m_ref(),
        m_index(0),
        m_type(JT_UNKNOWN),
        m_condition(nullptr),
        m_keys(),
        m_records(0),
        m_quick(nullptr),
        prefix_tables_map(0),
        added_tables_map(0),
        m_ft_func(nullptr),
        m_skip_records_in_range(false) {}

  /*
    Simple getters and setters. They are public. However, this object is
    protected in QEP_shared_owner, so only that class and its children
    (JOIN_TAB, QEP_TAB) can access the getters and setters.
  */

  JOIN *join() const { return m_join; }
  void set_join(JOIN *j) { m_join = j; }
  plan_idx idx() const {
    DBUG_ASSERT(m_idx >= 0);  // Index must be valid
    return m_idx;
  }
  void set_idx(plan_idx i) {
    DBUG_ASSERT(m_idx == NO_PLAN_IDX);  // Index should not change in lifetime
    m_idx = i;
  }
  TABLE *table() const { return m_table; }
  void set_table(TABLE *t) { m_table = t; }
  POSITION *position() const { return m_position; }
  void set_position(POSITION *p) { m_position = p; }
  Semijoin_mat_exec *sj_mat_exec() const { return m_sj_mat_exec; }
  void set_sj_mat_exec(Semijoin_mat_exec *s) { m_sj_mat_exec = s; }
  plan_idx first_sj_inner() { return m_first_sj_inner; }
  plan_idx last_sj_inner() { return m_last_sj_inner; }
  plan_idx first_inner() { return m_first_inner; }
  void set_first_inner(plan_idx i) { m_first_inner = i; }
  void set_last_inner(plan_idx i) { m_last_inner = i; }
  void set_first_sj_inner(plan_idx i) { m_first_sj_inner = i; }
  void set_last_sj_inner(plan_idx i) { m_last_sj_inner = i; }
  void set_first_upper(plan_idx i) { m_first_upper = i; }
  plan_idx last_inner() { return m_last_inner; }
  plan_idx first_upper() { return m_first_upper; }
  TABLE_REF &ref() { return m_ref; }
  uint index() const { return m_index; }
  void set_index(uint i) { m_index = i; }
  enum join_type type() const { return m_type; }
  void set_type(enum join_type t) { m_type = t; }
  Item *condition() const { return m_condition; }
  void set_condition(Item *c) { m_condition = c; }
  bool condition_is_pushed_to_sort() const {
    return m_condition_is_pushed_to_sort;
  }
  void mark_condition_as_pushed_to_sort() {
    m_condition_is_pushed_to_sort = true;
  }
  Key_map &keys() { return m_keys; }
  ha_rows records() const { return m_records; }
  void set_records(ha_rows r) { m_records = r; }
  QUICK_SELECT_I *quick() const { return m_quick; }
  void set_quick(QUICK_SELECT_I *q) { m_quick = q; }
  table_map prefix_tables() const { return prefix_tables_map; }
  table_map added_tables() const { return added_tables_map; }
  Item_func_match *ft_func() const { return m_ft_func; }
  void set_ft_func(Item_func_match *f) { m_ft_func = f; }

  // More elaborate functions:

  /**
    Set available tables for a table in a join plan.

    @param prefix_tables_arg Set of tables available for this plan
    @param prev_tables_arg   Set of tables available for previous table, used to
                             calculate set of tables added for this table.
  */
  void set_prefix_tables(table_map prefix_tables_arg,
                         table_map prev_tables_arg) {
    prefix_tables_map = prefix_tables_arg;
    added_tables_map = prefix_tables_arg & ~prev_tables_arg;
  }

  /**
    Add an available set of tables for a table in a join plan.

    @param tables Set of tables added for this table in plan.
  */
  void add_prefix_tables(table_map tables) {
    prefix_tables_map |= tables;
    added_tables_map |= tables;
  }

  bool is_first_inner_for_outer_join() const { return m_first_inner == m_idx; }

  bool is_inner_table_of_outer_join() const {
    return m_first_inner != NO_PLAN_IDX;
  }
  bool is_single_inner_of_semi_join() const {
    return m_first_sj_inner == m_idx && m_last_sj_inner == m_idx;
  }
  bool is_single_inner_of_outer_join() const {
    return m_first_inner == m_idx && m_last_inner == m_idx;
  }

  void set_skip_records_in_range(bool skip_records_in_range) {
    m_skip_records_in_range = skip_records_in_range;
  }

  bool skip_records_in_range() const { return m_skip_records_in_range; }

 private:
  JOIN *m_join;

  /**
     Index of structure in array:
     - NO_PLAN_IDX if before get_best_combination()
     - index of pointer to this JOIN_TAB, in JOIN::best_ref array
     - index of this QEP_TAB, in JOIN::qep array.
  */
  plan_idx m_idx;

  /// Corresponding table. Might be an internal temporary one.
  TABLE *m_table;

  /// Points into best_positions array. Includes cost info.
  POSITION *m_position;

  /*
    semijoin-related members.
  */

  /**
    Struct needed for materialization of semi-join. Set for a materialized
    temporary table, and NULL for all other join_tabs (except when
    materialization is in progress, @see join_materialize_semijoin()).
  */
  Semijoin_mat_exec *m_sj_mat_exec;

  /**
    Boundaries of semijoin inner tables around this table. Valid only once
    final QEP has been chosen. Depending on the strategy, they may define an
    interval (all tables inside are inner of a semijoin) or
    not. last_sj_inner is not set for Duplicates Weedout.
  */
  plan_idx m_first_sj_inner, m_last_sj_inner;

  /*
    outer-join-related members.
  */
  plan_idx m_first_inner;  ///< first inner table for including outer join
  plan_idx m_last_inner;   ///< last table table for embedding outer join
  plan_idx m_first_upper;  ///< first inner table for embedding outer join

  /**
     Used to do index-based look up based on a key value.
     Used when we read constant tables, in misc optimization (like
     remove_const()), and in execution.
  */
  TABLE_REF m_ref;

  /// ID of index used for index scan or semijoin LooseScan
  uint m_index;

  /// Type of chosen access method (scan, etc).
  enum join_type m_type;

  /**
    Table condition, ie condition to be evaluated for a row from this table.
    Notice that the condition may refer to rows from previous tables in the
    join prefix, as well as outer tables.
  */
  Item *m_condition;

  /**
    Whether the condition in m_condition is evaluated in front of a sort,
    so that it does not need to be evaluated again (unless it is outer to
    an inner join; see the relevant comments in SortingIterator::Init().

    Note that m_condition remains non-nullptr in this case, for purposes
    of the (non-tree) EXPLAIN and for filesort to build up its read maps.
  */
  bool m_condition_is_pushed_to_sort = false;

  /**
     All keys with can be used.
     Used by add_key_field() (optimization time) and execution of dynamic
     range (DynamicRangeIterator), and EXPLAIN.
  */
  Key_map m_keys;

  /**
     Either number of rows in the table or 1 for const table.
     Used in optimization, and also in execution for FOUND_ROWS().
  */
  ha_rows m_records;

  /**
     Non-NULL if quick-select used.
     Filled in optimization, used in execution to find rows, and in EXPLAIN.
  */
  QUICK_SELECT_I *m_quick;

  /*
    Maps below are shared because of dynamic range: in execution, it needs to
    know the prefix tables, to find the possible QUICK methods.
  */

  /**
    The set of all tables available in the join prefix for this table,
    including the table handled by this JOIN_TAB.
  */
  table_map prefix_tables_map;
  /**
    The set of tables added for this table, compared to the previous table
    in the join prefix.
  */
  table_map added_tables_map;

  /** FT function */
  Item_func_match *m_ft_func;

  /**
    Set if index dive can be skipped for this query.
    See comments for check_skip_records_in_range_qualification.
  */
  bool m_skip_records_in_range;
};

/// Owner of a QEP_shared; parent of JOIN_TAB and QEP_TAB.
class QEP_shared_owner {
 public:
  QEP_shared_owner() : m_qs(nullptr) {}

  /// Instructs to share the QEP_shared with another owner
  void share_qs(QEP_shared_owner *other) { other->set_qs(m_qs); }
  void set_qs(QEP_shared *q) {
    DBUG_ASSERT(!m_qs);
    m_qs = q;
  }

  // Getters/setters forwarding to QEP_shared:

  JOIN *join() const { return m_qs ? m_qs->join() : nullptr; }
  void set_join(JOIN *j) { return m_qs->set_join(j); }

  // NOTE: This index (and the associated map) is not the same as
  // table_ref's index, which is the index in the original FROM list
  // (before optimization).
  plan_idx idx() const { return m_qs->idx(); }
  void set_idx(plan_idx i) { return m_qs->set_idx(i); }
  qep_tab_map idx_map() const { return qep_tab_map{1} << m_qs->idx(); }

  TABLE *table() const { return m_qs->table(); }
  POSITION *position() const { return m_qs->position(); }
  void set_position(POSITION *p) { return m_qs->set_position(p); }
  Semijoin_mat_exec *sj_mat_exec() const { return m_qs->sj_mat_exec(); }
  void set_sj_mat_exec(Semijoin_mat_exec *s) {
    return m_qs->set_sj_mat_exec(s);
  }
  plan_idx first_sj_inner() const { return m_qs->first_sj_inner(); }
  plan_idx last_sj_inner() const { return m_qs->last_sj_inner(); }
  plan_idx first_inner() const { return m_qs->first_inner(); }
  plan_idx last_inner() const { return m_qs->last_inner(); }
  plan_idx first_upper() const { return m_qs->first_upper(); }
  void set_first_inner(plan_idx i) { return m_qs->set_first_inner(i); }
  void set_last_inner(plan_idx i) { return m_qs->set_last_inner(i); }
  void set_first_sj_inner(plan_idx i) { return m_qs->set_first_sj_inner(i); }
  void set_last_sj_inner(plan_idx i) { return m_qs->set_last_sj_inner(i); }
  void set_first_upper(plan_idx i) { return m_qs->set_first_upper(i); }
  TABLE_REF &ref() const { return m_qs->ref(); }
  uint index() const { return m_qs->index(); }
  void set_index(uint i) { return m_qs->set_index(i); }
  enum join_type type() const { return m_qs->type(); }
  void set_type(enum join_type t) { return m_qs->set_type(t); }
  Item *condition() const { return m_qs->condition(); }
  void set_condition(Item *to) { return m_qs->set_condition(to); }
  bool condition_is_pushed_to_sort() const {
    return m_qs->condition_is_pushed_to_sort();
  }
  void mark_condition_as_pushed_to_sort() {
    m_qs->mark_condition_as_pushed_to_sort();
  }
  Key_map &keys() const { return m_qs->keys(); }
  ha_rows records() const { return m_qs->records(); }
  void set_records(ha_rows r) { return m_qs->set_records(r); }
  QUICK_SELECT_I *quick() const { return m_qs->quick(); }
  void set_quick(QUICK_SELECT_I *q) { return m_qs->set_quick(q); }
  table_map prefix_tables() const { return m_qs->prefix_tables(); }
  table_map added_tables() const { return m_qs->added_tables(); }
  Item_func_match *ft_func() const { return m_qs->ft_func(); }
  void set_ft_func(Item_func_match *f) { return m_qs->set_ft_func(f); }
  void set_prefix_tables(table_map prefix_tables, table_map prev_tables) {
    return m_qs->set_prefix_tables(prefix_tables, prev_tables);
  }
  void add_prefix_tables(table_map tables) {
    return m_qs->add_prefix_tables(tables);
  }
  bool is_single_inner_of_semi_join() const {
    return m_qs->is_single_inner_of_semi_join();
  }
  bool is_inner_table_of_outer_join() const {
    return m_qs->is_inner_table_of_outer_join();
  }
  bool is_first_inner_for_outer_join() const {
    return m_qs->is_first_inner_for_outer_join();
  }
  bool is_single_inner_for_outer_join() const {
    return m_qs->is_single_inner_of_outer_join();
  }

  bool has_guarded_conds() const { return ref().has_guarded_conds(); }
  bool and_with_condition(Item *tmp_cond);

  void set_skip_records_in_range(bool skip_records_in_range) {
    m_qs->set_skip_records_in_range(skip_records_in_range);
  }

  bool skip_records_in_range() const { return m_qs->skip_records_in_range(); }

  void qs_cleanup();

 protected:
  QEP_shared *m_qs;  // qs stands for Qep_Shared
};

/**
  Symbolic slice numbers into JOIN's arrays ref_items, tmp_fields and
  tmp_all_fields

  See also the comments on JOIN::ref_items.
*/
enum {
  /**
     The slice which is used during evaluation of expressions; Item_ref::ref
     points there. This is the only slice that is not allocated on the heap;
     it always points to select_lex->base_ref_items.

     If we have a simple query (no temporary tables or GROUP BY needed),
     this slice always contains the base slice, i.e., the actual Items used
     in the original query.

     However, if we have temporary tables, there are cases where we need to
     swap out those Items, because they refer to Fields that are no longer in
     use. As a simple case, consider

       SELECT REVERSE(t1), COUNT(*) FROM t1 GROUP BY REVERSE(t1);

     Assuming no index on t1, this will require creating a temporary table
     consisting only of REVERSE(t1), and then sorting it before grouping.
     During execution of the query creating the temporary table, we will
     have an Item_func_reverse pointing to a Field for t1, and the result of
     this will be stored in the temporary table "tmp". However, when reading
     from "tmp", it would be wrong to use that Item_func_reverse, as the Field
     no longer exists. Thus, we create a slice (in REF_SLICE_TMP1) with new Item
     pointers, where Item_func_reverse is replaced by an Item_field that reads
     from the right field in the temporary table. Similar logic applies for
     windowing functions etc.; see below.

     In such cases, the pointers in this slice are _overwritten_ (using memcpy)
     by e.g. REF_SLICE_TMP1 for as long as we read from the temporary table.
     Switch_ref_item_slice provides an encapsulation of the overwriting,
     and the optimizer stores a copy of the original Item pointers in the
     REF_SLICE_SAVED_BASE slice so that it is possible to copy them back
     when we are done.

     @todo It would probably be better to store the active slice index in
     current_thd and do the indirection in Item_ref_* instead of copying the
     slices around.
  */
  REF_SLICE_ACTIVE = 0,
  /**
     The slice with pointers to columns of 1st group-order-distinct tmp
     table
  */
  REF_SLICE_TMP1,
  /**
     The slice with pointers to columns of 2nd group-order-distinct tmp
     table
  */
  REF_SLICE_TMP2,
  /**
     Stores the unfinished aggregated row when doing GROUP BY on an
     ordered table.

     For certain queries with GROUP BY (e.g., when using an index),
     rows arrive already sorted in the right order for grouping.
     In that case, we do not need nor use a temporary table, but can
     just group values as we go. However, we do not necessarily know when
     a group ends -- a group implicitly ends when we see that the
     group index values have changed, and by that time, it's too late to
     output them in the aggregated row (the Fields already point to the
     new row, so the data is lost).

     Thus, we need to store the values for the current group somewhere.
     We use a set of Items, which together represent a one-row
     pseudo-tmp-table holding the current group. These items are created
     by setup_copy_fields().

     When we have finished reading a row from the last pre-grouping table, we
     process it either with end_send_group() or end_send():

       * end_send_group(): Compare the new row with the current group.
         If it belongs to the current group, we update the aggregation functions
         and move on. If not, we output the aggregated row and overwrite the
         contents of this slice with the new group.

       * end_send(): Used when we know there's exactly one row for each group
         (e.g., during a loose index scan). In this case, we can skip the
         comparison and just output the group directly; however, we still need
         the temporary table to avoid evaluating Items more than once (see the
         next paragraph).

     Both functions build the group by copying values of items from the previous
     stages into a pseudo-table, e.g.

       SELECT a, RAND() AS r FROM t GROUP BY a HAVING r=1;

     copies "a" from "t" and stores it into the pseudo-table (this slice),
     evaluates rand() and stores it, then finally evaluates "r=1" based on the
     stored value (so that "r" in the SELECT list and "r" in "r=1" match).

     Groups from this slice are always directly sent to the query's result,
     and never buffered to any further temporary table.
  */
  REF_SLICE_ORDERED_GROUP_BY,
  /**
     The slice with pointers to columns of table(s), ie., the actual Items.
     Only used for queries involving temporary tables or the likes; for simple
     queries, they always live in REF_SLICE_ACTIVE, so we don't need a copy
     here. See REF_SLICE_ACTIVE for more discussion.
  */
  REF_SLICE_SAVED_BASE,
  /**
     The slice with pointers to columns of 1st tmp table of windowing
  */
  REF_SLICE_WIN_1
};

#endif  // SQL_OPT_EXEC_SHARED_INCLUDED
