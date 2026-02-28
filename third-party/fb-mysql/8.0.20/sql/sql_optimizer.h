#ifndef SQL_OPTIMIZER_INCLUDED
#define SQL_OPTIMIZER_INCLUDED

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

/**
  @file sql/sql_optimizer.h
  Classes used for query optimizations.
*/

/*
   This structure is used to collect info on potentially sargable
   predicates in order to check whether they become sargable after
   reading const tables.
   We form a bitmap of indexes that can be used for sargable predicates.
   Only such indexes are involved in range analysis.
*/

#include <string.h>
#include <sys/types.h>
#include <memory>
#include <utility>

#include "field_types.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_table_map.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/mem_root_array.h"
#include "sql/opt_explain_format.h"  // Explain_sort_clause
#include "sql/row_iterator.h"
#include "sql/sql_array.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_select.h"  // Key_use
#include "sql/table.h"
#include "sql/temp_table_param.h"

class COND_EQUAL;
class Item_subselect;
class Item_sum;
class Opt_trace_context;
class THD;
class Window;
struct MYSQL_LOCK;

typedef Bounds_checked_array<Item_null_result *> Item_null_array;

// Key_use has a trivial destructor, no need to run it from Mem_root_array.
typedef Mem_root_array<Key_use> Key_use_array;

class Cost_model_server;

struct SARGABLE_PARAM {
  Field *field;     /* field against which to check sargability */
  Item **arg_value; /* values of potential keys for lookups     */
  uint num_values;  /* number of values in the above array      */
};

struct ROLLUP {
  enum State { STATE_NONE, STATE_INITED, STATE_READY };
  State state;
  Item_null_array null_items;
  Ref_item_array *ref_item_arrays;
  List<Item> *fields_list;  ///< SELECT list
  List<Item> *all_fields;   ///< Including hidden fields
};

/**
  Wrapper for ORDER* pointer to trace origins of ORDER list

  As far as ORDER is just a head object of ORDER expression
  chain, we need some wrapper object to associate flags with
  the whole ORDER list.
*/
class ORDER_with_src {
  /**
    Private empty class to implement type-safe NULL assignment

    This private utility class allows us to implement a constructor
    from NULL and only NULL (or 0 -- this is the same thing) and
    an assignment operator from NULL.
    Assignments from other pointers still prohibited since other
    pointer types are incompatible with the "null" type, and the
    casting is impossible outside of ORDER_with_src class, since
    the "null" type is private.
  */
  struct null {};

 public:
  ORDER *order;  ///< ORDER expression that we are wrapping with this class
  Explain_sort_clause src;  ///< origin of order list

 private:
  int flags;  ///< bitmap of Explain_sort_property

 public:
  ORDER_with_src() { clean(); }

  ORDER_with_src(ORDER *order_arg, Explain_sort_clause src_arg)
      : order(order_arg),
        src(src_arg),
        flags(order_arg ? ESP_EXISTS : ESP_none) {}

  /**
    Type-safe NULL assignment

    See a commentary for the "null" type above.
  */
  ORDER_with_src &operator=(null *) {
    clean();
    return *this;
  }

  /**
    Type-safe constructor from NULL

    See a commentary for the "null" type above.
  */
  ORDER_with_src(null *) { clean(); }

  /**
    Transparent access to the wrapped order list

    These operators are safe, since we don't do any conversion of
    ORDER_with_src value, but just an access to the wrapped
    ORDER pointer value.
    We can use ORDER_with_src objects instead ORDER pointers in
    a transparent way without accessor functions.

    @note     This operator also implements safe "operator bool()"
              functionality.
  */
  operator ORDER *() { return order; }
  operator const ORDER *() const { return order; }

  ORDER *operator->() const { return order; }

  void clean() {
    order = nullptr;
    src = ESC_none;
    flags = ESP_none;
  }

  int get_flags() const {
    DBUG_ASSERT(order);
    return flags;
  }
};

class JOIN {
 public:
  JOIN(THD *thd_arg, SELECT_LEX *select);
  JOIN(const JOIN &rhs) = delete;
  JOIN &operator=(const JOIN &rhs) = delete;

  /// Query block that is optimized and executed using this JOIN
  SELECT_LEX *const select_lex;
  /// Query expression referring this query block
  SELECT_LEX_UNIT *const unit;
  /// Thread handler
  THD *const thd;

  /**
    Optimal query execution plan. Initialized with a tentative plan in
    JOIN::make_join_plan() and later replaced with the optimal plan in
    get_best_combination().
  */
  JOIN_TAB *join_tab{nullptr};
  /// Array of QEP_TABs
  QEP_TAB *qep_tab{nullptr};

  /**
    Array of plan operators representing the current (partial) best
    plan. The array is allocated in JOIN::make_join_plan() and is valid only
    inside this function. Initially (*best_ref[i]) == join_tab[i].
    The optimizer reorders best_ref.
  */
  JOIN_TAB **best_ref{nullptr};
  /// mapping between table indexes and JOIN_TABs
  JOIN_TAB **map2table{nullptr};
  ///< mapping between table indexes and QEB_TABs
  QEP_TAB **map2qep_tab{nullptr};
  /*
    The table which has an index that allows to produce the requried ordering.
    A special value of 0x1 means that the ordering will be produced by
    passing 1st non-const table to filesort(). NULL means no such table exists.
  */
  TABLE *sort_by_table{nullptr};
  /**
    Before plan has been created, "tables" denote number of input tables in the
    query block and "primary_tables" is equal to "tables".
    After plan has been created (after JOIN::get_best_combination()),
    the JOIN_TAB objects are enumerated as follows:
    - "tables" gives the total number of allocated JOIN_TAB objects
    - "primary_tables" gives the number of input tables, including
      materialized temporary tables from semi-join operation.
    - "const_tables" are those tables among primary_tables that are detected
      to be constant.
    - "tmp_tables" is 0, 1 or 2 (more if windows) and counts the maximum
      possible number of intermediate tables in post-processing (ie sorting and
      duplicate removal).
      Later, tmp_tables will be adjusted to the correct number of
      intermediate tables, @see JOIN::make_tmp_tables_info.
    - The remaining tables (ie. tables - primary_tables - tmp_tables) are
      input tables to materialized semi-join operations.
    The tables are ordered as follows in the join_tab array:
     1. const primary table
     2. non-const primary tables
     3. intermediate sort/group tables
     4. possible holes in array
     5. semi-joined tables used with materialization strategy
  */
  uint tables{0};          ///< Total number of tables in query block
  uint primary_tables{0};  ///< Number of primary input tables in query block
  uint const_tables{0};    ///< Number of primary tables deemed constant
  uint tmp_tables{0};      ///< Number of temporary tables used by query
  uint send_group_parts{0};
  /**
    Indicates that the data will be aggregated (typically GROUP BY),
    _and_ that it is already processed in an order that is compatible with
    the grouping in use (e.g. because we are scanning along an index,
    or because an earlier step sorted the data in a group-compatible order).

    Note that this flag changes value at multiple points during optimization;
    if it's set when a temporary table is created, this means we aggregate
    into said temporary table (end_write_group is chosen instead of end_write),
    but if it's set later, it means that we can aggregate as we go,
    just before sending the data to the client (end_send_group is chosen
    instead of end_send).

    @see make_group_fields, alloc_group_fields, JOIN::exec
  */
  bool streaming_aggregation{false};
  /// If query contains GROUP BY clause
  bool grouped;
  /// If true, send produced rows using query_result
  bool do_send_rows{true};
  /// Set of tables contained in query
  table_map all_table_map{0};
  table_map const_table_map;  ///< Set of tables found to be const
  /**
     Const tables which are either:
     - not empty
     - empty but inner to a LEFT JOIN, thus "considered" not empty for the
     rest of execution (a NULL-complemented row will be used).
  */
  table_map found_const_table_map;
  /**
     Used in some loops which scan the JOIN's tables: it is the bitmap of all
     tables which are dependencies of lateral derived tables which the loop
     has not yet processed.
  */
  table_map deps_of_remaining_lateral_derived_tables{0};

  /* Number of records produced after join + group operation */
  ha_rows send_records{0};
  ha_rows found_records{0};
  ha_rows examined_rows{0};
  ha_rows row_limit{0};
  // m_select_limit is used to decide if we are likely to scan the whole table.
  ha_rows m_select_limit{0};
  /**
    Used to fetch no more than given amount of rows per one
    fetch operation of server side cursor.
    The value is checked in end_send and end_send_group in fashion, similar
    to offset_limit_cnt:
      - fetch_limit= HA_POS_ERROR if there is no cursor.
      - when we open a cursor, we set fetch_limit to 0,
      - on each fetch iteration we add num_rows to fetch to fetch_limit
  */
  ha_rows fetch_limit{HA_POS_ERROR};

  /**
    This is the result of join optimization.

    @note This is a scratch array, not used after get_best_combination().
  */
  POSITION *best_positions{nullptr};

  /******* Join optimization state members start *******/

  /* Current join optimization state */
  POSITION *positions{nullptr};

  /* We also maintain a stack of join optimization states in * join->positions[]
   */
  /******* Join optimization state members end *******/

  /// A hook that secondary storage engines can use to override the executor
  /// completely.
  using Override_executor_func = bool (*)(JOIN *);
  Override_executor_func override_executor_func = nullptr;

  /**
    The cost of best complete join plan found so far during optimization,
    after optimization phase - cost of picked join order (not taking into
    account the changes made by test_if_skip_sort_order()).
  */
  double best_read{0.0};
  /**
    The estimated row count of the plan with best read time (see above).
  */
  ha_rows best_rowcount{0};
  /// Expected cost of filesort.
  double sort_cost{0.0};
  /// Expected cost of windowing;
  double windowing_cost{0.0};
  List<Item> *fields;
  List<Cached_item> group_fields{};
  List<Cached_item> group_fields_cache{};
  Item_sum **sum_funcs{nullptr};
  Item_sum ***sum_funcs_end{nullptr};
  /**
     Describes a temporary table.
     Each tmp table has its own tmp_table_param.
     The one here has two roles:
     - is transiently used as a model by create_intermediate_table(), to build
     the tmp table's own tmp_table_param.
     - is also used as description of the pseudo-tmp-table of grouping
     (REF_SLICE_ORDERED_GROUP_BY) (e.g. in end_send_group()).
  */
  Temp_table_param tmp_table_param;
  MYSQL_LOCK *lock;

  ROLLUP rollup{};         ///< Used with rollup
  bool implicit_grouping;  ///< True if aggregated but no GROUP BY

  /**
    At construction time, set if SELECT DISTINCT. May be reset to false
    later, when we set up a temporary table operation that deduplicates for us.
   */
  bool select_distinct;

  /**
    If we have the GROUP BY statement in the query,
    but the group_list was emptied by optimizer, this
    flag is true.
    It happens when fields in the GROUP BY are from
    constant table
  */
  bool group_optimized_away{false};

  /*
    simple_xxxxx is set if ORDER/GROUP BY doesn't include any references
    to other tables than the first non-constant table in the JOIN.
    It's also set if ORDER/GROUP BY is empty.
    Used for deciding for or against using a temporary table to compute
    GROUP/ORDER BY.
  */
  bool simple_order{false};
  bool simple_group{false};

  /*
    m_ordered_index_usage is set if an ordered index access
    should be used instead of a filesort when computing
    ORDER/GROUP BY.
  */
  enum {
    ORDERED_INDEX_VOID,      // No ordered index avail.
    ORDERED_INDEX_GROUP_BY,  // Use index for GROUP BY
    ORDERED_INDEX_ORDER_BY   // Use index for ORDER BY
  } m_ordered_index_usage{ORDERED_INDEX_VOID};

  /**
    Is set if we have a GROUP BY and we have ORDER BY on a constant or when
    sorting isn't required.
  */
  bool skip_sort_order{false};

  /**
    If true we need a temporary table on the result set before any
    windowing steps, e.g. for DISTINCT or we have a query ORDER BY.
    See details in JOIN::optimize
  */
  bool need_tmp_before_win{false};

  /// If JOIN has lateral derived tables (is set at start of planning)
  bool has_lateral{false};

  /// Used and updated by JOIN::make_join_plan() and optimize_keyuse()
  Key_use_array keyuse_array;

  /// List storing all expressions used in query block
  List<Item> &all_fields;

  /// List storing all expressions of select list
  List<Item> &fields_list;

  /**
     This is similar to tmp_fields_list, but it also contains necessary
     extras: expressions added for ORDER BY, GROUP BY, window clauses,
     underlying items of split items.
  */
  List<Item> *tmp_all_fields{nullptr};

  /**
    Array of pointers to lists of expressions.
    Each list represents the SELECT list at a certain stage of execution.
    This array is only used when the query makes use of tmp tables: after
    writing to tmp table (e.g. for GROUP BY), if this write also does a
    function's calculation (e.g. of SUM), after the write the function's value
    is in a column of the tmp table. If a SELECT list expression is the SUM,
    and we now want to read that materialized SUM and send it forward, a new
    expression (Item_field type instead of Item_sum), is needed. The new
    expressions are listed in JOIN::tmp_fields_list[x]; 'x' is a number
    (REF_SLICE_).
    Same is applicable to tmp_all_fields.
    @see JOIN::make_tmp_tables_info()
  */
  List<Item> *tmp_fields_list{nullptr};

  int error{0};  ///< set in optimize(), exec(), prepare_result()

  /**
    ORDER BY and GROUP BY lists, to transform with prepare,optimize and exec
  */
  ORDER_with_src order, group_list;

  /**
    Any window definitions
  */
  List<Window> m_windows;

  /**
    True if a window requires a certain order of rows, which implies that any
    order of rows coming out of the pre-window join will be disturbed.
  */
  bool m_windows_sort{false};

  /// If we have set up tmp tables for windowing, @see make_tmp_tables_info
  bool m_windowing_steps{false};

  /**
    Buffer to gather GROUP BY, ORDER BY and DISTINCT QEP details for EXPLAIN
  */
  Explain_format_flags explain_flags{};

  /**
    JOIN::having_cond is initially equal to select_lex->having_cond, but may
    later be changed by optimizations performed by JOIN.
    The relationship between the JOIN::having_cond condition and the
    associated variable select_lex->having_value is so that
    having_value can be:
     - COND_UNDEF if a having clause was not specified in the query or
       if it has not been optimized yet
     - COND_TRUE if the having clause is always true, in which case
       JOIN::having_cond is set to NULL.
     - COND_FALSE if the having clause is impossible, in which case
       JOIN::having_cond is set to NULL
     - COND_OK otherwise, meaning that the having clause needs to be
       further evaluated
    All of the above also applies to the where_cond/select_lex->cond_value
    pair.
  */
  /**
    Optimized WHERE clause item tree (valid for one single execution).
    Used in JOIN execution if no tables. Otherwise, attached in pieces to
    JOIN_TABs and then not used in JOIN execution.
    Printed by EXPLAIN EXTENDED.
    Initialized by SELECT_LEX::get_optimizable_conditions().
  */
  Item *where_cond;
  /**
    Optimized HAVING clause item tree (valid for one single execution).
    Used in JOIN execution, as last "row filtering" step. With one exception:
    may be pushed to the JOIN_TABs of temporary tables used in DISTINCT /
    GROUP BY (see JOIN::make_tmp_tables_info()); in that case having_cond is
    set to NULL, but is first saved to having_for_explain so that EXPLAIN
    EXTENDED can still print it.
    Initialized by SELECT_LEX::get_optimizable_conditions().
  */
  Item *having_cond;
  Item *having_for_explain;  ///< Saved optimized HAVING for EXPLAIN
  /**
    Pointer set to select_lex->get_table_list() at the start of
    optimization. May be changed (to NULL) only if optimize_aggregated_query()
    optimizes tables away.
  */
  TABLE_LIST *tables_list;
  COND_EQUAL *cond_equal{nullptr};
  /*
    Join tab to return to. Points to an element of join->join_tab array, or to
    join->join_tab[-1].
    This is used at execution stage to shortcut join enumeration. Currently
    shortcutting is done to handle outer joins or handle semi-joins with
    FirstMatch strategy.
  */
  plan_idx return_tab{0};

  /**
    ref_items is an array of 5 slices, each containing an array of Item
    pointers. ref_items is used in different phases of query execution.
    - slice 0 is initially the same as SELECT_LEX::base_ref_items, ie it is
      the set of items referencing fields from base tables. During optimization
      and execution it may be temporarily overwritten by slice 1-3.
    - slice 1 is a representation of the used items when being read from
      the first temporary table.
    - slice 2 is a representation of the used items when being read from
      the second temporary table.
    - slice 3 is a representation of the used items when used in
      aggregation but no actual temporary table is needed.
    - slice 4 is a copy of the original slice 0. It is created if
      slice overwriting is necessary, and it is used to restore
      original values in slice 0 after having been overwritten.
    - slices 5 -> N are used by windowing:
      first are all the window's out tmp tables,
      the next indexes are reserved for the windows' frame buffers (in the same
      order), if any, e.g.

      One window:      5: window 1's out table
                       6: window 1's FB

      Two windows:     5: window 1's out table
                       6: window 2's out table
                       7: window 1's FB
                       8: window 2's FB
      and so on.

    Slice 0 is allocated for the lifetime of a statement, whereas slices 1-4
    are associated with a single optimization. The size of slice 0 determines
    the slice size used when allocating the other slices.
   */
  Ref_item_array *ref_items{
      nullptr};  // cardinality: REF_SLICE_SAVED_BASE + 1 + #windows*2

  /**
     If slice REF_SLICE_ORDERED_GROUP_BY has been created, this is the QEP_TAB
     which is right before calculation of items in this slice.
  */
  QEP_TAB *ref_slice_immediately_before_group_by{nullptr};

  /**
    The slice currently stored in ref_items[0].
    Used to restore the base ref_items slice from the "save" slice after it
    has been overwritten by another slice (1-3).
  */
  uint current_ref_item_slice;

  /**
    Used only if this query block is recursive. Contains count of
    all executions of this recursive query block, since the last
    this->reset().
  */
  uint recursive_iteration_count{0};

  /**
    <> NULL if optimization has determined that execution will produce an
    empty result before aggregation, contains a textual explanation on why
    result is empty. Implicitly grouped queries may still produce an
    aggregation row.
    @todo - suggest to set to "Preparation determined that query is empty"
            when SELECT_LEX::is_empty_query() is true.
  */
  const char *zero_result_cause{nullptr};

  /**
     True if, at this stage of processing, subquery materialization is allowed
     for children subqueries of this JOIN (those in the SELECT list, in WHERE,
     etc). If false, and we have to evaluate a subquery at this stage, then we
     must choose EXISTS.
  */
  bool child_subquery_can_materialize{false};
  /**
     True if plan search is allowed to use references to expressions outer to
     this JOIN (for example may set up a 'ref' access looking up an outer
     expression in the index, etc).
  */
  bool allow_outer_refs{false};

  /* Temporary tables used to weed-out semi-join duplicates */
  List<TABLE> sj_tmp_tables{};
  List<Semijoin_mat_exec> sjm_exec_list{};
  /* end of allocation caching storage */

  /** Exec time only: true <=> current group has been sent */
  bool group_sent{false};
  /// If true, calculate found rows for this query block
  bool calc_found_rows{false};

  /**
    This will force tmp table to NOT use index + update for group
    operation as it'll cause [de]serialization for each json aggregated
    value and is very ineffective (times worse).
    Server should use filesort, or tmp table + filesort to resolve GROUP BY
    with JSON aggregate functions.
  */
  bool with_json_agg;

  /**
    If set, "fields" has been replaced with a set of Item_refs for rollup
    processing; see the AggregateIterator constructor for more details.
    This is used when constructing iterators only; it is not used during
    execution.
   */
  bool replaced_items_for_rollup = false;

  /// True if plan is const, ie it will return zero or one rows.
  bool plan_is_const() const { return const_tables == primary_tables; }

  /**
    True if plan contains one non-const primary table (ie not including
    tables taking part in semi-join materialization).
  */
  bool plan_is_single_table() { return primary_tables - const_tables == 1; }

  bool optimize();
  void reset();
  bool prepare_result();
  bool destroy();
  bool alloc_func_list();
  bool make_sum_func_list(List<Item> &all_fields, List<Item> &send_fields,
                          bool before_group_by, bool recompute = false);

  /**
     Overwrites one slice of ref_items with the contents of another slice.
     In the normal case, dst and src have the same size().
     However: the rollup slices may have smaller size than slice_sz.
   */
  void copy_ref_item_slice(uint dst_slice, uint src_slice) {
    copy_ref_item_slice(ref_items[dst_slice], ref_items[src_slice]);
  }
  void copy_ref_item_slice(Ref_item_array dst_arr, Ref_item_array src_arr) {
    DBUG_ASSERT(dst_arr.size() >= src_arr.size());
    void *dest = dst_arr.array();
    const void *src = src_arr.array();
    if (!src_arr.is_null())
      memcpy(dest, src, src_arr.size() * src_arr.element_size());
  }

  /**
    Allocate a ref_item slice, assume that slice size is in ref_items[0]

    @param thd_arg  thread handler
    @param sliceno  The slice number to allocate in JOIN::ref_items

    @returns false if success, true if error
  */
  bool alloc_ref_item_slice(THD *thd_arg, int sliceno);

  /**
    Overwrite the base slice of ref_items with the slice supplied as argument.

    @param sliceno number to overwrite the base slice with, must be 1-4 or
           4 + windowno.
  */
  void set_ref_item_slice(uint sliceno) {
    DBUG_ASSERT((int)sliceno >= 1);
    if (current_ref_item_slice != sliceno) {
      copy_ref_item_slice(REF_SLICE_ACTIVE, sliceno);
      DBUG_PRINT("info",
                 ("ref slice %u -> %u", current_ref_item_slice, sliceno));
      current_ref_item_slice = sliceno;
    }
  }

  /// @note do also consider Switch_ref_item_slice
  uint get_ref_item_slice() const { return current_ref_item_slice; }

  /**
     Returns the clone of fields_list which is appropriate for evaluating
     expressions at the current stage of execution; which stage is denoted by
     the value of current_ref_item_slice.
  */
  List<Item> *get_current_fields();

  bool optimize_rollup();
  bool rollup_process_const_fields();
  bool rollup_make_fields(List<Item> &all_fields, List<Item> &fields,
                          Item_sum ***func);
  bool switch_slice_for_rollup_fields(List<Item> &all_fields,
                                      List<Item> &fields);
  bool finalize_table_conditions();
  /**
    Release memory and, if possible, the open tables held by this execution
    plan (and nested plans). It's used to release some tables before
    the end of execution in order to increase concurrency and reduce
    memory consumption.
  */
  void join_free();
  /** Cleanup this JOIN. Not a full cleanup. reusable? */
  void cleanup();

  bool clear_fields(table_map *save_nullinfo);
  void restore_fields(table_map save_nullinfo);

  /**
    Return whether the caller should send a row even if the join
    produced no rows if:
     - there is an aggregate function (sum_func_count!=0), and
     - the query is not grouped, and
     - a possible HAVING clause evaluates to TRUE.

    @note: if there is a having clause, it must be evaluated before
    returning the row.
  */
  bool send_row_on_empty_set() const {
    return (do_send_rows && tmp_table_param.sum_func_count != 0 &&
            group_list == nullptr && !group_optimized_away &&
            select_lex->having_value != Item::COND_FALSE);
  }

  bool generate_derived_keys();
  void finalize_derived_keys();
  bool get_best_combination();
  bool attach_join_conditions(plan_idx last_tab);

 private:
  bool attach_join_condition_to_nest(plan_idx first_inner, plan_idx last_tab,
                                     Item *join_cond, bool is_sj_mat_cond);

 public:
  bool update_equalities_for_sjm();
  bool add_sorting_to_table(uint idx, ORDER_with_src *order,
                            bool force_stable_sort = false);
  bool decide_subquery_strategy();
  void refine_best_rowcount();
  void recalculate_deps_of_remaining_lateral_derived_tables(
      table_map plan_tables, uint idx);
  bool clear_sj_tmp_tables();
  bool clear_corr_derived_tmp_tables();

  void mark_const_table(JOIN_TAB *table, Key_use *key);
  /// State of execution plan. Currently used only for EXPLAIN
  enum enum_plan_state {
    NO_PLAN,      ///< No plan is ready yet
    ZERO_RESULT,  ///< Zero result cause is set
    NO_TABLES,    ///< Plan has no tables
    PLAN_READY    ///< Plan is ready
  };
  /// See enum_plan_state
  enum_plan_state get_plan_state() const { return plan_state; }
  bool is_optimized() const { return optimized; }
  void set_optimized() { optimized = true; }
  bool is_executed() const { return executed; }
  void set_executed() { executed = true; }

  /**
    Retrieve the cost model object to be used for this join.

    @return Cost model object for the join
  */

  const Cost_model_server *cost_model() const;

  /**
    Check if FTS index only access is possible
  */
  bool fts_index_access(JOIN_TAB *tab);

  QEP_TAB::enum_op_type get_end_select_func();
  /**
     Propagate dependencies between tables due to outer join relations.

     @returns false if success, true if error
  */
  bool propagate_dependencies();

  /**
    Handle offloading of query parts to the underlying engines, when
    such is supported by their implementation.

    @returns false if success, true if error
  */
  bool push_to_engines();

  RowIterator *root_iterator() const { return m_root_iterator.get(); }
  unique_ptr_destroy_only<RowIterator> release_root_iterator() {
    return move(m_root_iterator);
  }
  void set_root_iterator(unique_ptr_destroy_only<RowIterator> iterator) {
    m_root_iterator = move(iterator);
  }

 private:
  bool optimized{false};  ///< flag to avoid double optimization in EXPLAIN

  /**
    Set by exec(), reset by reset(). Note that this needs to be set
    _during_ the query (not only when it's done executing), or the
    dynamic range optimizer will not understand which tables have been
    read.
   */
  bool executed{false};

  /// Final execution plan state. Currently used only for EXPLAIN
  enum_plan_state plan_state{NO_PLAN};

 public:
  /*
    When join->select_count is set, tables will not be optimized away.
    The call to records() will be delayed until the execution phase and
    the counting will be done on an index of Optimizer's choice.
    The index will be decided in find_shortest_key(), called from
    optimize_aggregated_query().
  */
  bool select_count{false};

 private:
  /**
    Create a temporary table to be used for processing DISTINCT/ORDER
    BY/GROUP BY.

    @note Will modify JOIN object wrt sort/group attributes

    @param tab              the JOIN_TAB object to attach created table to
    @param tmp_table_fields List of items that will be used to define
                            column types of the table.
    @param tmp_table_group  Group key to use for temporary table, NULL if none.
    @param save_sum_fields  If true, do not replace Item_sum items in
                            @c tmp_fields list with Item_field items referring
                            to fields in temporary table.

    @returns false on success, true on failure
  */
  bool create_intermediate_table(QEP_TAB *tab, List<Item> *tmp_table_fields,
                                 ORDER_with_src &tmp_table_group,
                                 bool save_sum_fields);

  /**
    Optimize distinct when used on a subset of the tables.

    E.g.,: SELECT DISTINCT t1.a FROM t1,t2 WHERE t1.b=t2.b
    In this case we can stop scanning t2 when we have found one t1.a
  */
  void optimize_distinct();

  /**
    Function sets FT hints, initializes FT handlers and
    checks if FT index can be used as covered.
  */
  bool optimize_fts_query();

  bool prune_table_partitions();
  /**
    Initialize key dependencies for join tables.

    TODO figure out necessity of this method. Current test
         suite passed without this intialization.
  */
  void init_key_dependencies() {
    JOIN_TAB *const tab_end = join_tab + tables;
    for (JOIN_TAB *tab = join_tab; tab < tab_end; tab++)
      tab->key_dependent = tab->dependent;
  }

 private:
  void set_prefix_tables();
  void cleanup_item_list(List<Item> &items) const;
  void set_semijoin_embedding();
  bool make_join_plan();
  bool init_planner_arrays();
  bool extract_const_tables();
  bool extract_func_dependent_tables();
  void update_sargable_from_const(SARGABLE_PARAM *sargables);
  bool estimate_rowcount();
  void optimize_keyuse();
  void set_semijoin_info();
  /**
   An utility function - apply heuristics and optimize access methods to tables.
   @note Side effect - this function could set 'Impossible WHERE' zero
   result.
  */
  void adjust_access_methods();
  void update_depend_map();
  void update_depend_map(ORDER *order);
  /**
    Fill in outer join related info for the execution plan structure.

      For each outer join operation left after simplification of the
      original query the function set up the following pointers in the linear
      structure join->join_tab representing the selected execution plan.
      The first inner table t0 for the operation is set to refer to the last
      inner table tk through the field t0->last_inner.
      Any inner table ti for the operation are set to refer to the first
      inner table ti->first_inner.
      The first inner table t0 for the operation is set to refer to the
      first inner table of the embedding outer join operation, if there is any,
      through the field t0->first_upper.
      The on expression for the outer join operation is attached to the
      corresponding first inner table through the field t0->on_expr_ref.
      Here ti are structures of the JOIN_TAB type.

    EXAMPLE. For the query:
    @code
          SELECT * FROM t1
                        LEFT JOIN
                        (t2, t3 LEFT JOIN t4 ON t3.a=t4.a)
                        ON (t1.a=t2.a AND t1.b=t3.b)
            WHERE t1.c > 5,
    @endcode

      given the execution plan with the table order t1,t2,t3,t4
      is selected, the following references will be set;
      t4->last_inner=[t4], t4->first_inner=[t4], t4->first_upper=[t2]
      t2->last_inner=[t4], t2->first_inner=t3->first_inner=[t2],
      on expression (t1.a=t2.a AND t1.b=t3.b) will be attached to
      *t2->on_expr_ref, while t3.a=t4.a will be attached to *t4->on_expr_ref.

    @note
      The function assumes that the simplification procedure has been
      already applied to the join query (see simplify_joins).
      This function can be called only after the execution plan
      has been chosen.
  */
  void make_outerjoin_info();

  /**
    Initialize ref access for all tables that use it.

    @return False if success, True if error

    @note We cannot setup fields used for ref access before we have sorted
          the items within multiple equalities according to the final order of
          the tables involved in the join operation. Currently, this occurs in
          @see substitute_for_best_equal_field().
  */
  bool init_ref_access();
  bool alloc_qep(uint n);
  void unplug_join_tabs();
  bool setup_semijoin_materialized_table(JOIN_TAB *tab, uint tableno,
                                         POSITION *inner_pos,
                                         POSITION *sjm_pos);

  bool add_having_as_tmp_table_cond(uint curr_tmp_table);
  bool make_tmp_tables_info();
  void set_plan_state(enum_plan_state plan_state_arg);
  bool compare_costs_of_subquery_strategies(SubqueryExecMethod *method);
  ORDER *remove_const(ORDER *first_order, Item *cond, bool change_list,
                      bool *simple_order, bool group_by);

  /**
    Check whether this is a subquery that can be evaluated by index look-ups.
    If so, change subquery engine to subselect_indexsubquery_engine.

    @retval  1   engine was changed
    @retval  0   engine wasn't changed
    @retval -1   OOM
  */
  int replace_index_subquery();

  /**
    Optimize DISTINCT, GROUP BY, ORDER BY clauses

    @retval false ok
    @retval true  an error occurred
  */
  bool optimize_distinct_group_order();

  /**
    Test if an index could be used to replace filesort for ORDER BY/GROUP BY

    @details
      Investigate whether we may use an ordered index as part of either
      DISTINCT, GROUP BY or ORDER BY execution. An ordered index may be
      used for only the first of any of these terms to be executed. This
      is reflected in the order which we check for test_if_skip_sort_order()
      below. However we do not check for DISTINCT here, as it would have
      been transformed to a GROUP BY at this stage if it is a candidate for
      ordered index optimization.
      If a decision was made to use an ordered index, the availability
      if such an access path is stored in 'm_ordered_index_usage' for later
      use by 'execute' or 'explain'
  */
  void test_skip_sort();

  bool alloc_indirection_slices();

  /**
    If possible, convert the executor structures to a set of row iterators,
    storing the result in m_root_iterator. If not, m_root_iterator will remain
    nullptr.
   */
  void create_iterators();

  /**
    Create iterators with the knowledge that there are going to be zero rows
    coming from tables (before aggregation); typically because we know that
    all of them would be filtered away by WHERE (e.g. SELECT * FROM t1
    WHERE 1=2). This will normally yield no output rows, but if we have implicit
    aggregation, it might yield a single one.
   */
  void create_iterators_for_zero_rows();

  void create_iterators_for_index_subquery();

  /** @{ Helpers for create_iterators. */
  void create_table_iterators();
  unique_ptr_destroy_only<RowIterator> create_root_iterator_for_join();
  unique_ptr_destroy_only<RowIterator> attach_iterators_for_having_and_limit(
      unique_ptr_destroy_only<RowIterator> iterator);
  /** @} */

  /**
    An iterator you can read from to get all records for this query.

    May be nullptr even after create_iterators() if the current query
    is not supported by the iterator executor.
   */
  unique_ptr_destroy_only<RowIterator> m_root_iterator;
};

/**
  Use this in a function which depends on best_ref listing tables in the
  final join order. If 'tables==0', one is not expected to consult best_ref
  cells, and best_ref may not even have been allocated.
*/
#define ASSERT_BEST_REF_IN_JOIN_ORDER(join)               \
  do {                                                    \
    DBUG_ASSERT((join)->tables == 0 ||                    \
                ((join)->best_ref && !(join)->join_tab)); \
  } while (0)

/**
  RAII class to ease the temporary switching to a different slice of
  the ref item array.
*/
class Switch_ref_item_slice {
  JOIN *join;
  uint saved;

 public:
  Switch_ref_item_slice(JOIN *join_arg, uint new_v)
      : join(join_arg), saved(join->get_ref_item_slice()) {
    if (!join->ref_items[new_v].is_null()) join->set_ref_item_slice(new_v);
  }
  ~Switch_ref_item_slice() { join->set_ref_item_slice(saved); }
};

/**
  RAII class to ease the call of LEX::mark_broken() if error.
  Used during preparation and optimization of DML queries.
*/
class Prepare_error_tracker {
 public:
  explicit Prepare_error_tracker(THD *thd) : m_thd(thd) {}
  ~Prepare_error_tracker();

 private:
  THD *const m_thd;
};

bool uses_index_fields_only(Item *item, TABLE *tbl, uint keyno,
                            bool other_tbls_ok);
bool remove_eq_conds(THD *thd, Item *cond, Item **retcond,
                     Item::cond_result *cond_value);
bool optimize_cond(THD *thd, Item **conds, COND_EQUAL **cond_equal,
                   mem_root_deque<TABLE_LIST *> *join_list,
                   Item::cond_result *cond_value);
Item *substitute_for_best_equal_field(THD *thd, Item *cond,
                                      COND_EQUAL *cond_equal,
                                      JOIN_TAB **table_join_idx);
bool build_equal_items(THD *thd, Item *cond, Item **retcond,
                       COND_EQUAL *inherited, bool do_inherit,
                       mem_root_deque<TABLE_LIST *> *join_list,
                       COND_EQUAL **cond_equal_ref);
bool is_indexed_agg_distinct(JOIN *join, List<Item_field> *out_args);
Key_use_array *create_keyuse_for_table(THD *thd, uint keyparts,
                                       Item_field **fields,
                                       List<Item> outer_exprs);
Item_field *get_best_field(Item_field *item_field, COND_EQUAL *cond_equal);
Item *make_cond_for_table(THD *thd, Item *cond, table_map tables,
                          table_map used_table, bool exclude_expensive_cond);
uint build_bitmap_for_nested_joins(mem_root_deque<TABLE_LIST *> *join_list,
                                   uint first_unused);

/**
  Create an order list that consists of all non-const fields and items.
  This is usable for e.g. converting DISTINCT into GROUP or ORDER BY.

  Try to put the items in "order_list" first, to allow one to optimize away
  a later ORDER BY.
 */
ORDER *create_order_from_distinct(THD *thd, Ref_item_array ref_item_array,
                                  ORDER *order_list, List<Item> &fields,
                                  bool skip_aggregates,
                                  bool convert_bit_fields_to_long,
                                  bool *all_order_by_fields_used);

/**
   Returns true if arguments are a temporal Field having no date,
   part and a temporal expression having a date part.
   @param  f  Field
   @param  v  Expression
 */
inline bool field_time_cmp_date(const Field *f, const Item *v) {
  const enum_field_types ft = f->type();
  return is_temporal_type(ft) && !is_temporal_type_with_date(ft) &&
         v->is_temporal_with_date();
}

bool substitute_gc(THD *thd, SELECT_LEX *select_lex, Item *where_cond,
                   ORDER *group_list, ORDER *order);

/// RAII class to manage JOIN::deps_of_remaining_lateral_derived_tables
class Deps_of_remaining_lateral_derived_tables {
  JOIN *join;
  table_map saved;
  /// All lateral tables not part of this map should be ignored
  table_map plan_tables;

 public:
  /**
     Constructor.
     @param j                the JOIN
     @param plan_tables_arg  @see
                             JOIN::deps_of_remaining_lateral_derived_tables
  */
  Deps_of_remaining_lateral_derived_tables(JOIN *j, table_map plan_tables_arg)
      : join(j),
        saved(join->deps_of_remaining_lateral_derived_tables),
        plan_tables(plan_tables_arg) {}
  ~Deps_of_remaining_lateral_derived_tables() { restore(); }
  void restore() { join->deps_of_remaining_lateral_derived_tables = saved; }
  void assert_unchanged() {
    DBUG_ASSERT(join->deps_of_remaining_lateral_derived_tables == saved);
  }
  void recalculate(uint next_idx) {
    if (join->has_lateral)
      /*
        No cur_tab given, so assume we start from a place in the plan which
        may be backward or forward compared to where we were before:
        recalculate.
      */
      join->recalculate_deps_of_remaining_lateral_derived_tables(plan_tables,
                                                                 next_idx);
  }

  void recalculate(JOIN_TAB *cur_tab, uint next_idx) {
    /*
      We have just added cur_tab to the plan; if it's not lateral, the map
      doesn't change, no need to recalculate it.
    */
    if (join->has_lateral && cur_tab->table_ref->is_derived() &&
        cur_tab->table_ref->derived_unit()->m_lateral_deps)
      recalculate(next_idx);
  }
  void init() {
    // Normally done once in a run of JOIN::optimize().
    if (join->has_lateral) {
      recalculate(join->const_tables);
      // Forget stale value:
      saved = join->deps_of_remaining_lateral_derived_tables;
    }
  }
};

/**
  Estimates how many times a subquery will be executed as part of a
  query execution. If it is a cacheable subquery, the estimate tells
  how many times the subquery will be executed if it is not cached.

  @param[in]     subquery  the Item that represents the subquery
  @param[in,out] trace     optimizer trace context

  @return the number of times the subquery is expected to be executed
*/
double calculate_subquery_executions(const Item_subselect *subquery,
                                     Opt_trace_context *trace);

/**
  Class which presents a view of the current candidate table order for a JOIN.
*/
class Candidate_table_order {
 public:
  Candidate_table_order(const JOIN *join) : m_join(join) {}

  /// Returns the number of tables in the candidate plan.
  size_t size() const { return m_join->tables; }

  /// Returns the table reference at the given position in the candidate plan.
  const TABLE_LIST *table_ref(size_t position) const {
    return m_join->positions[position].table->table_ref;
  }

 private:
  const JOIN *const m_join;
};

extern const char *antijoin_null_cond;

/**
  Checks if an Item, which is constant for execution, can be evaluated during
  optimization. It cannot be evaluated if it contains a subquery and the
  OPTION_NO_SUBQUERY_DURING_OPTIMIZATION query option is active.

  @param item    the Item to check
  @param select  the query block that contains the Item
  @return false if this Item contains a subquery and subqueries cannot be
  evaluated during optimization, or true otherwise
*/
bool evaluate_during_optimization(const Item *item, const SELECT_LEX *select);

/**
  Find the multiple equality predicate containing a field.

  The function retrieves the multiple equalities accessed through
  the cond_equal structure from current level and up looking for
  an equality containing a field. It stops retrieval as soon as the equality
  is found and set up inherited_fl to true if it's found on upper levels.

  @param cond_equal          multiple equalities to search in
  @param item_field          field to look for
  @param[out] inherited_fl   set up to true if multiple equality is found
                             on upper levels (not on current level of
                             cond_equal)

  @return
    - Item_equal for the found multiple equality predicate if a success;
    - nullptr otherwise.
*/
Item_equal *find_item_equal(COND_EQUAL *cond_equal,
                            const Item_field *item_field, bool *inherited_fl);

#endif /* SQL_OPTIMIZER_INCLUDED */
