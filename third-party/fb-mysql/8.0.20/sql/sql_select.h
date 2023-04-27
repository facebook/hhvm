#ifndef SQL_SELECT_INCLUDED
#define SQL_SELECT_INCLUDED

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
  @file sql/sql_select.h
*/

#include <limits.h>
#include <stddef.h>
#include <sys/types.h>

#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_table_map.h"
#include "sql/item_cmpfunc.h"  // Item_cond_and
#include "sql/opt_costmodel.h"
#include "sql/sql_bitmap.h"
#include "sql/sql_cmd_dml.h"  // Sql_cmd_dml
#include "sql/sql_const.h"
#include "sql/sql_opt_exec_shared.h"  // join_type

class Field;
class Item;
class Item_func;
class JOIN_TAB;
class KEY;
class QEP_TAB;
class Query_result;
class SELECT_LEX;
class Select_lex_visitor;
class SJ_TMP_TABLE;
class Temp_table_param;
class THD;
template <class T>
class List;
struct LEX;
struct MYSQL_LEX_CSTRING;
struct ORDER;
struct SJ_TMP_TABLE_TAB;
struct TABLE;
struct TABLE_LIST;

typedef ulonglong nested_join_map;

class Sql_cmd_select : public Sql_cmd_dml {
 public:
  explicit Sql_cmd_select(Query_result *result_arg) : Sql_cmd_dml() {
    result = result_arg;
  }

  enum_sql_command sql_command_code() const override { return SQLCOM_SELECT; }

  bool is_data_change_stmt() const override { return false; }

  bool accept(THD *thd, Select_lex_visitor *visitor) override;

  const MYSQL_LEX_CSTRING *eligible_secondary_storage_engine() const override;

 protected:
  bool precheck(THD *thd) override;

  bool prepare_inner(THD *thd) override;
};

/**
   Returns a constant of type 'type' with the 'A' lowest-weight bits set.
   Example: LOWER_BITS(uint, 3) == 7.
   Requirement: A < sizeof(type) * 8.
*/
#define LOWER_BITS(type, A) ((type)(((type)1 << (A)) - 1))

/* Values in optimize */
#define KEY_OPTIMIZE_EXISTS 1
#define KEY_OPTIMIZE_REF_OR_NULL 2
#define FT_KEYPART (MAX_REF_PARTS + 10)

/**
  A Key_use represents an equality predicate of the form (table.column = val),
  where the column is indexed by @c keypart in @c key and @c val is either a
  constant, a column from other table, or an expression over column(s) from
  other table(s). If @c val is not a constant, then the Key_use specifies an
  equi-join predicate, and @c table must be put in the join plan after all
  tables in @c used_tables.

  At an abstract level a Key_use instance can be viewed as a directed arc
  of an equi-join graph, where the arc originates from the table(s)
  containing the column(s) that produce the values used for index lookup
  into @c table, and the arc points into @c table.

  For instance, assuming there is only an index t3(c), the query

  @code
    SELECT * FROM t1, t2, t3
    WHERE t1.a = t3.c AND
          t2.b = t3.c;
  @endcode

  would generate two arcs (instances of Key_use)

  @code
     t1-- a ->- c --.
                    |
                    V
                    t3
                    ^
                    |
     t2-- b ->- c --'
  @endcode

  If there were indexes t1(a), and t2(b), then the equi-join graph
  would have two additional arcs "c->a" and "c->b" recording the fact
  that it is possible to perform lookup in either direction.

  @code
    t1-- a ->- c --.    ,-- c -<- b --- t2
     ^             |    |               ^
     |             |    |               |
     `-- a -<- c - v    v-- c ->- b ----'
                     t3
  @endcode

  The query

  @code
    SELECT * FROM t1, t2, t3 WHERE t1.a + t2.b = t3.c;
  @endcode

  can be viewed as a graph with one "multi-source" arc:

  @code
    t1-- a ---
              |
               >-- c --> t3
              |
    t2-- b ---
  @endcode

  The graph of all equi-join conditions usable for index lookup is
  stored as an ordered sequence of Key_use elements in
  JOIN::keyuse_array. See sort_keyuse() for details on the
  ordering. Each JOIN_TAB::keyuse points to the first array element
  with the same table.
*/
class Key_use {
 public:
  // We need the default constructor for unit testing.
  Key_use()
      : table_ref(nullptr),
        val(nullptr),
        used_tables(0),
        key(0),
        keypart(0),
        optimize(0),
        keypart_map(0),
        ref_table_rows(0),
        null_rejecting(false),
        cond_guard(nullptr),
        sj_pred_no(UINT_MAX),
        bound_keyparts(0),
        fanout(0.0),
        read_cost(0.0) {}

  Key_use(TABLE_LIST *table_ref_arg, Item *val_arg, table_map used_tables_arg,
          uint key_arg, uint keypart_arg, uint optimize_arg,
          key_part_map keypart_map_arg, ha_rows ref_table_rows_arg,
          bool null_rejecting_arg, bool *cond_guard_arg, uint sj_pred_no_arg)
      : table_ref(table_ref_arg),
        val(val_arg),
        used_tables(used_tables_arg),
        key(key_arg),
        keypart(keypart_arg),
        optimize(optimize_arg),
        keypart_map(keypart_map_arg),
        ref_table_rows(ref_table_rows_arg),
        null_rejecting(null_rejecting_arg),
        cond_guard(cond_guard_arg),
        sj_pred_no(sj_pred_no_arg),
        bound_keyparts(0),
        fanout(0.0),
        read_cost(0.0) {}

  TABLE_LIST *table_ref;  ///< table owning the index

  /**
    Value used for lookup into @c key. It may be an Item_field, a
    constant or any other expression. If @c val contains a field from
    another table, then we have a join condition, and the table(s) of
    the field(s) in @c val should be before @c table in the join plan.
  */
  Item *val;

  /**
    All tables used in @c val, that is all tables that provide bindings
    for the expression @c val. These tables must be in the plan before
    executing the equi-join described by a Key_use.
  */
  table_map used_tables;
  uint key;                  ///< number of index
  uint keypart;              ///< used part of the index
  uint optimize;             ///< 0, or KEY_OPTIMIZE_*
  key_part_map keypart_map;  ///< like keypart, but as a bitmap
  ha_rows ref_table_rows;    ///< Estimate of how many rows for a key value
  /**
    If true, the comparison this value was created from will not be
    satisfied if val has NULL 'value'.
    Not used if the index is fulltext (such index cannot be used for
    equalities).
  */
  bool null_rejecting;
  /**
    !NULL - This Key_use was created from an equality that was wrapped into
            an Item_func_trig_cond. This means the equality (and validity of
            this Key_use element) can be turned on and off. The on/off state
            is indicted by the pointed value:
              *cond_guard == true @<=@> equality condition is on
              *cond_guard == false @<=@> equality condition is off

    NULL  - Otherwise (the source equality can't be turned off)

    Not used if the index is fulltext (such index cannot be used for
    equalities).
  */
  bool *cond_guard;
  /**
     0..63    @<=@> This was created from semi-join IN-equality # sj_pred_no.
     UINT_MAX  Otherwise

     Not used if the index is fulltext (such index cannot be used for
     semijoin).

     @see get_semi_join_select_list_index()
  */
  uint sj_pred_no;

  /*
    The three members below are different from the rest of Key_use: they are
    set only by Optimize_table_order, and they change with the currently
    considered join prefix.
  */

  /**
     The key columns which are equal to expressions depending only of earlier
     tables of the current join prefix.
     This information is stored only in the first Key_use of the index.
  */
  key_part_map bound_keyparts;

  /**
     Fanout of the ref access path for this index, in the current join
     prefix.
     This information is stored only in the first Key_use of the index.
  */
  double fanout;

  /**
    Cost of the ref access path for the current join prefix, i.e. the
    cost of using ref access once multiplied by estimated number of
    partial rows from tables earlier in the join sequence.
    read_cost does NOT include cost of processing rows on the
    server side (row_evaluate_cost).

    Example: If the cost of ref access on this index is 5, and the
    estimated number of partial rows from earlier tables is 10,
    read_cost=50.

    This information is stored only in the first Key_use of the index.
  */
  double read_cost;
};

/// @returns join type according to quick select type used
join_type calc_join_type(int quick_type);

class JOIN;

#define SJ_OPT_NONE 0
#define SJ_OPT_DUPS_WEEDOUT 1
#define SJ_OPT_LOOSE_SCAN 2
#define SJ_OPT_FIRST_MATCH 3
#define SJ_OPT_MATERIALIZE_LOOKUP 4
#define SJ_OPT_MATERIALIZE_SCAN 5

inline bool sj_is_materialize_strategy(uint strategy) {
  return strategy >= SJ_OPT_MATERIALIZE_LOOKUP;
}

/**
    Bits describing quick select type
*/
enum quick_type { QS_NONE, QS_RANGE, QS_DYNAMIC_RANGE };

/**
  A position of table within a join order. This structure is primarily used
  as a part of @c join->positions and @c join->best_positions arrays.

  One POSITION element contains information about:
   - Which table is accessed
   - Which access method was chosen
      = Its cost and \#of output records
   - Semi-join strategy choice. Note that there are two different
     representation formats:
      1. The one used during join optimization
      2. The one used at plan refinement/code generation stage.
      We call fix_semijoin_strategies_for_picked_join_order() to switch
      between #1 and #2. See that function's comment for more details.

   - Semi-join optimization state. When we're running join optimization,
     we main a state for every semi-join strategy which are various
     variables that tell us if/at which point we could consider applying the
     strategy.
     The variables are really a function of join prefix but they are too
     expensive to re-caclulate for every join prefix we consider, so we
     maintain current state in join->positions[\#tables_in_prefix]. See
     advance_sj_state() for details.

  This class has to stay a POD, because it is memcpy'd in many places.
*/

struct POSITION {
  /**
    The number of rows that will be fetched by the chosen access
    method per each row combination of previous tables. That is:

      rows_fetched = selectivity(access_condition) * cardinality(table)

    where 'access_condition' is whatever condition was chosen for
    index access, depending on the access method ('ref', 'range',
    etc.)

    @note For index/table scans, rows_fetched may be less than
    the number of rows in the table because the cost of evaluating
    constant conditions is included in the scan cost, and the number
    of rows produced by these scans is the estimated number of rows
    that pass the constant conditions. @see
    Optimize_table_order::calculate_scan_cost() . But this is only during
    planning; make_join_readinfo() simplifies it for EXPLAIN.
  */
  double rows_fetched;

  /**
    Cost of accessing the table in course of the entire complete join
    execution, i.e. cost of one access method use (e.g. 'range' or
    'ref' scan ) multiplied by estimated number of rows from tables
    earlier in the join sequence.

    read_cost does NOT include cost of processing rows within the
    executor (row_evaluate_cost).
  */
  double read_cost;

  /**
    The fraction of the 'rows_fetched' rows that will pass the table
    conditions that were NOT used by the access method. If, e.g.,

      "SELECT ... WHERE t1.colx = 4 and t1.coly @> 5"

    is resolved by ref access on t1.colx, filter_effect will be the
    fraction of rows that will pass the "t1.coly @> 5" predicate. The
    valid range is 0..1, where 0.0 means that no rows will pass the
    table conditions and 1.0 means that all rows will pass.

    It is used to calculate how many row combinations will be joined
    with the next table, @see prefix_rowcount below.

    @note With condition filtering enabled, it is possible to get
    a fanout = rows_fetched * filter_effect that is less than 1.0.
    Consider, e.g., a join between t1 and t2:

       "SELECT ... WHERE t1.col1=t2.colx and t2.coly OP @<something@>"

    where t1 is a prefix table and the optimizer currently calculates
    the cost of adding t2 to the join. Assume that the chosen access
    method on t2 is a 'ref' access on 'colx' that is estimated to
    produce 2 rows per row from t1 (i.e., rows_fetched = 2). It will
    in this case be perfectly fine to calculate a filtering effect
    @<0.5 (resulting in "rows_fetched * filter_effect @< 1.0") from the
    predicate "t2.coly OP @<something@>". If so, the number of row
    combinations from (t1,t2) is lower than the prefix_rowcount of t1.

    The above is just an example of how the fanout of a table can
    become less than one. It can happen for any access method.
  */
  float filter_effect;

  /**
    prefix_rowcount and prefix_cost form a stack of partial join
    order costs and output sizes

    prefix_rowcount: The number of row combinations that will be
    joined to the next table in the join sequence.

    For a joined table it is calculated as
      prefix_rowcount =
          last_table.prefix_rowcount * rows_fetched * filter_effect

    @see filter_effect

    For a semijoined table it may be less than this formula due to
    duplicate elimination.
  */
  double prefix_rowcount;
  double prefix_cost;

  JOIN_TAB *table;

  /**
    NULL  -  'index' or 'range' or 'index_merge' or 'ALL' access is used.
    Other - [eq_]ref[_or_null] access is used. Pointer to {t.keypart1 = expr}
  */
  Key_use *key;

  /** If ref-based access is used: bitmap of tables this table depends on  */
  table_map ref_depend_map;
  bool use_join_buffer;

  /**
    Current optimization state: Semi-join strategy to be used for this
    and preceding join tables.

    Join optimizer sets this for the *last* join_tab in the
    duplicate-generating range. That is, in order to interpret this field,
    one needs to traverse join->[best_]positions array from right to left.
    When you see a join table with sj_strategy!= SJ_OPT_NONE, some other
    field (depending on the strategy) tells how many preceding positions
    this applies to. The values of covered_preceding_positions->sj_strategy
    must be ignored.
  */
  uint sj_strategy;
  /**
    Valid only after fix_semijoin_strategies_for_picked_join_order() call:
    if sj_strategy!=SJ_OPT_NONE, this is the number of subsequent tables that
    are covered by the specified semi-join strategy
  */
  uint n_sj_tables;

  /**
    Bitmap of semi-join inner tables that are in the join prefix and for
    which there's no provision yet for how to eliminate semi-join duplicates
    which they produce.
  */
  table_map dups_producing_tables;

  /* LooseScan strategy members */

  /* The first (i.e. driving) table we're doing loose scan for */
  uint first_loosescan_table;
  /*
     Tables that need to be in the prefix before we can calculate the cost
     of using LooseScan strategy.
  */
  table_map loosescan_need_tables;

  /*
    keyno  -  Planning to do LooseScan on this key. If keyuse is NULL then
              this is a full index scan, otherwise this is a ref+loosescan
              scan (and keyno matches the KEUSE's)
    MAX_KEY - Not doing a LooseScan
  */
  uint loosescan_key;   // final (one for strategy instance )
  uint loosescan_parts; /* Number of keyparts to be kept distinct */

  /* FirstMatch strategy */
  /*
    Index of the first inner table that we intend to handle with this
    strategy
  */
  uint first_firstmatch_table;
  /**
     Value of Optimize_table_order::cur_embedding_map after this table has
     been added to the plan. Used to constrain FirstMatch table orders.
  */
  nested_join_map cur_embedding_map;
  /*
    Tables that were not in the join prefix when we've started considering
    FirstMatch strategy.
  */
  table_map first_firstmatch_rtbl;
  /*
    Tables that need to be in the prefix before we can calculate the cost
    of using FirstMatch strategy.
   */
  table_map firstmatch_need_tables;

  /* Duplicate Weedout strategy */
  /* The first table that the strategy will need to handle */
  uint first_dupsweedout_table;
  /*
    Tables that we will need to have in the prefix to do the weedout step
    (all inner and all outer that the involved semi-joins are correlated with)
  */
  table_map dupsweedout_tables;

  /* SJ-Materialization-Scan strategy */
  /* The last inner table (valid once we're after it) */
  uint sjm_scan_last_inner;
  /*
    Tables that we need to have in the prefix to calculate the correct cost.
    Basically, we need all inner tables and outer tables mentioned in the
    semi-join's ON expression so we can correctly account for fanout.
  */
  table_map sjm_scan_need_tables;

  /**
     Even if the query has no semijoin, two sj-related members are read and
     must thus have been set, by this function.
  */
  void no_semijoin() {
    sj_strategy = SJ_OPT_NONE;
    dups_producing_tables = 0;
  }
  /**
    Set complete estimated cost and produced rowcount for the prefix of tables
    up to and including this table, in the join plan.

    @param cost     Estimated cost
    @param rowcount Estimated row count
  */
  void set_prefix_cost(double cost, double rowcount) {
    prefix_cost = cost;
    prefix_rowcount = rowcount;
  }
  /**
    Set complete estimated cost and produced rowcount for the prefix of tables
    up to and including this table, calculated from the cost of the previous
    stage, the fanout of the current stage and the cost to process a row at
    the current stage.

    @param idx      Index of position object within array, if zero there is no
                    "previous" stage that can be added.
    @param cm       Cost model that provides the actual calculation
  */
  void set_prefix_join_cost(uint idx, const Cost_model_server *cm) {
    if (idx == 0) {
      prefix_rowcount = rows_fetched;
      prefix_cost = read_cost + cm->row_evaluate_cost(prefix_rowcount);
    } else {
      prefix_rowcount = (this - 1)->prefix_rowcount * rows_fetched;
      prefix_cost = (this - 1)->prefix_cost + read_cost +
                    cm->row_evaluate_cost(prefix_rowcount);
    }
    prefix_rowcount *= filter_effect;
  }
};

/**
  Query optimization plan node.

  Specifies:

  - a table access operation on the table specified by this node, and

  - a join between the result of the set of previous plan nodes and
    this plan node.
*/
class JOIN_TAB : public QEP_shared_owner {
 public:
  JOIN_TAB();

  void set_table(TABLE *t);

  /// Sets the pointer to the join condition of TABLE_LIST
  void init_join_cond_ref(TABLE_LIST *tl);

  /// @returns join condition
  Item *join_cond() const { return *m_join_cond_ref; }

  /**
     Sets join condition
     @note this also changes TABLE_LIST::m_join_cond.
  */
  void set_join_cond(Item *cond) { *m_join_cond_ref = cond; }

  /// Set the combined condition for a table (may be performed several times)
  void set_condition(Item *to) {
    if (condition() != to) {
      m_qs->set_condition(to);
      // Condition changes, so some indexes may become usable or not:
      quick_order_tested.clear_all();
    }
  }

  uint use_join_cache() const { return m_use_join_cache; }
  void set_use_join_cache(uint u) { m_use_join_cache = u; }
  Key_use *keyuse() const { return m_keyuse; }
  void set_keyuse(Key_use *k) { m_keyuse = k; }

  TABLE_LIST *table_ref; /**< points to table reference               */

 private:
  Key_use *m_keyuse; /**< pointer to first used key               */

  /**
    Pointer to the associated join condition:

    - if this is a table with position==NULL (e.g. internal sort/group
      temporary table), pointer is NULL

    - otherwise, pointer is the address of some TABLE_LIST::m_join_cond.
      Thus, the pointee is the same as TABLE_LIST::m_join_cond (changing one
      changes the other; thus, optimizations made on the second are reflected
      in SELECT_LEX::print_table_array() which uses the first one).
  */
  Item **m_join_cond_ref;

 public:
  COND_EQUAL *cond_equal; /**< multiple equalities for the on expression*/

  /**
    The maximum value for the cost of seek operations for key lookup
    during ref access. The cost model for ref access assumes every key
    lookup will cause reading a block from disk. With many key lookups
    into the same table, most of the blocks will soon be in a memory
    buffer. As a consequence, there will in most cases be an upper
    limit on the number of actual disk accesses the ref access will
    cause. This variable is used for storing a maximum cost estimate
    for the disk accesses for ref access. It is used for limiting the
    cost estimate for ref access to a more realistic value than
    assuming every key lookup causes a random disk access. Without
    having this upper limit for the cost of ref access, table scan
    would be more likely to be chosen for cases where ref access
    performs better.
  */
  double worst_seeks;
  /** Keys with constant part. Subset of keys. */
  Key_map const_keys;
  Key_map checked_keys; /**< Keys checked */
  /**
    Keys which can be used for skip scan access. We store it
    separately from tab->const_keys & join_tab->keys() to avoid
    unnecessary printing of the prossible keys in EXPLAIN output
    as some of these keys can be marked not usable for skip scan later.
    More strict check for prossible keys is performed in
    get_best_skip_scan() function.
  */
  Key_map skip_scan_keys;
  Key_map needed_reg;

  /**
    Used to avoid repeated range analysis for the same key in
    test_if_skip_sort_order(). This would otherwise happen if the best
    range access plan found for a key is turned down.
    quick_order_tested is cleared every time the select condition for
    this JOIN_TAB changes since a new condition may give another plan
    and cost from range analysis.
   */
  Key_map quick_order_tested;

  /*
    Number of records that will be scanned (yes scanned, not returned) by the
    best 'independent' access method, i.e. table scan or QUICK_*_SELECT)
  */
  ha_rows found_records;
  /*
    Cost of accessing the table using "ALL" or range/index_merge access
    method (but not 'index' for some reason), i.e. this matches method which
    E(#records) is in found_records.
  */
  double read_time;
  /**
    The set of tables that this table depends on. Used for outer join and
    straight join dependencies.
  */
  table_map dependent;
  /**
    The set of tables that are referenced by key from this table.
  */
  table_map key_dependent;

 public:
  uint used_fieldlength;
  enum quick_type use_quick;

  /**
    Join buffering strategy.
    After optimization it contains chosen join buffering strategy (if any).
  */
  uint m_use_join_cache;

  /* SemiJoinDuplicateElimination variables: */
  /*
    Embedding SJ-nest (may be not the direct parent), or NULL if none.
    It is the closest semijoin or antijoin nest.
    This variable holds the result of table pullout.
  */
  TABLE_LIST *emb_sj_nest;

  /* NestedOuterJoins: Bitmap of nested joins this table is part of */
  nested_join_map embedding_map;

  /** Flags from SE's MRR implementation, to be used by JOIN_CACHE */
  uint join_cache_flags;

  /** true <=> AM will scan backward */
  bool reversed_access;

  /** Clean up associated table after query execution, including resources */
  void cleanup();

  /// @returns semijoin strategy for this table.
  uint get_sj_strategy() const;

 private:
  JOIN_TAB(const JOIN_TAB &);             // not defined
  JOIN_TAB &operator=(const JOIN_TAB &);  // not defined
};

inline JOIN_TAB::JOIN_TAB()
    : QEP_shared_owner(),
      table_ref(nullptr),
      m_keyuse(nullptr),
      m_join_cond_ref(nullptr),
      cond_equal(nullptr),
      worst_seeks(0.0),
      const_keys(),
      checked_keys(),
      skip_scan_keys(),
      needed_reg(),
      quick_order_tested(),
      found_records(0),
      read_time(0),
      dependent(0),
      key_dependent(0),
      used_fieldlength(0),
      use_quick(QS_NONE),
      m_use_join_cache(0),
      emb_sj_nest(nullptr),
      embedding_map(0),
      join_cache_flags(0),
      reversed_access(false) {}

/* Extern functions in sql_select.cc */
void count_field_types(SELECT_LEX *select_lex, Temp_table_param *param,
                       List<Item> &fields, bool reset_with_sum_func,
                       bool save_sum_fields);
uint find_shortest_key(TABLE *table, const Key_map *usable_keys);

/* functions from opt_sum.cc */
bool simple_pred(Item_func *func_item, Item **args, bool *inv_order);

enum aggregate_evaluated {
  AGGR_COMPLETE,  // All aggregates were evaluated
  AGGR_REGULAR,   // Aggregates not fully evaluated, regular execution required
  AGGR_DELAYED,   // Aggregates not fully evaluated, execute with ha_records()
  AGGR_EMPTY      // Source tables empty, aggregates are NULL or 0 (for COUNT)
};

bool optimize_aggregated_query(THD *thd, SELECT_LEX *select,
                               List<Item> &all_fields, Item *conds,
                               aggregate_evaluated *decision);

/* from sql_delete.cc, used by opt_range.cc */
extern "C" int refpos_order_cmp(const void *arg, const void *a, const void *b);

/// The name of store_key instances that represent constant items.
constexpr const char *STORE_KEY_CONST_NAME = "const";

/** class to copying an field/item to a key struct */

class store_key {
 public:
  bool null_key{false}; /* true <=> the value of the key has a null part */
  enum store_key_result { STORE_KEY_OK, STORE_KEY_FATAL, STORE_KEY_CONV };
  store_key(THD *thd, Field *field_arg, uchar *ptr, uchar *null, uint length);
  virtual ~store_key() = default;
  virtual const char *name() const = 0;

  /**
    @brief sets ignore truncation warnings mode and calls the real copy method

    @details this function makes sure truncation warnings when preparing the
    key buffers don't end up as errors (because of an enclosing INSERT/UPDATE).
  */
  store_key_result copy();

 protected:
  Field *to_field;  // Store data here

  virtual enum store_key_result copy_inner() = 0;
};

class store_key_field final : public store_key {
  Copy_field m_copy_field;
  const char *m_field_name;

 public:
  store_key_field(THD *thd, Field *to_field_arg, uchar *ptr,
                  uchar *null_ptr_arg, uint length, Field *from_field,
                  const char *name_arg);

  const char *name() const override { return m_field_name; }

  // Change the source field to be another field. Used only by
  // CreateBKAIterator, when rewriting multi-equalities used in ref access.
  void replace_from_field(Field *from_field);

 protected:
  enum store_key_result copy_inner() override;
};
class store_key_item : public store_key {
 protected:
  Item *item;

 public:
  store_key_item(THD *thd, Field *to_field_arg, uchar *ptr, uchar *null_ptr_arg,
                 uint length, Item *item_arg);
  const char *name() const override { return "func"; }

 protected:
  enum store_key_result copy_inner() override;
};

/*
  Class used for unique constraint implementation by subselect_hash_sj_engine.
  It uses store_key_item implementation to do actual copying, but after
  that, copy_inner calculates hash of each key part for unique constraint.
*/

class store_key_hash_item final : public store_key_item {
  ulonglong *hash;

 public:
  store_key_hash_item(THD *thd, Field *to_field_arg, uchar *ptr,
                      uchar *null_ptr_arg, uint length, Item *item_arg,
                      ulonglong *hash_arg)
      : store_key_item(thd, to_field_arg, ptr, null_ptr_arg, length, item_arg),
        hash(hash_arg) {}
  const char *name() const override { return "func"; }

 protected:
  enum store_key_result copy_inner() override;
};

bool error_if_full_join(JOIN *join);
bool handle_query(THD *thd, LEX *lex, Query_result *result,
                  ulonglong added_options, ulonglong removed_options);

// Statement timeout function(s)
bool set_statement_timer(THD *thd);
void reset_statement_timer(THD *thd);

void free_underlaid_joins(THD *thd, SELECT_LEX *select);

void calc_used_field_length(TABLE *table, bool needs_rowid, uint *p_used_fields,
                            uint *p_used_fieldlength, uint *p_used_blobs,
                            bool *p_used_null_fields,
                            bool *p_used_uneven_bit_fields);

ORDER *simple_remove_const(ORDER *order, Item *where);
bool const_expression_in_where(Item *cond, Item *comp_item,
                               const Field *comp_field = nullptr,
                               Item **const_item = nullptr);
bool test_if_subpart(ORDER *a, ORDER *b);
void calc_group_buffer(JOIN *join, ORDER *group);
bool make_join_readinfo(JOIN *join, uint no_jbuf_after);
bool create_ref_for_key(JOIN *join, JOIN_TAB *j, Key_use *org_keyuse,
                        table_map used_tables);
bool types_allow_materialization(Item *outer, Item *inner);
bool and_conditions(Item **e1, Item *e2);

/**
  Create a AND item of two existing items.
  A new Item_cond_and item is created with the two supplied items as
  arguments.

  @note About handling of null pointers as arguments: if the first
  argument is a null pointer, then the item given as second argument is
  returned (no new Item_cond_and item is created). The second argument
  must not be a null pointer.

  @param cond  the first argument to the new AND condition
  @param item  the second argument to the new AND condtion

  @return the new AND item
*/
static inline Item *and_items(Item *cond, Item *item) {
  DBUG_ASSERT(item != nullptr);
  return (cond ? (new Item_cond_and(cond, item)) : item);
}

/// A variant of the above, guaranteed to return Item_bool_func.
static inline Item_bool_func *and_items(Item *cond, Item_bool_func *item) {
  DBUG_ASSERT(item != nullptr);
  return (cond ? (new Item_cond_and(cond, item)) : item);
}

uint actual_key_parts(const KEY *key_info);

class ORDER_with_src;

uint get_index_for_order(ORDER_with_src *order, QEP_TAB *tab, ha_rows limit,
                         bool *need_sort, bool *reverse);
int test_if_order_by_key(ORDER_with_src *order, TABLE *table, uint idx,
                         uint *used_key_parts, bool *skip_quick);
bool test_if_cheaper_ordering(const JOIN_TAB *tab, ORDER_with_src *order,
                              TABLE *table, Key_map usable_keys, int key,
                              ha_rows select_limit, int *new_key,
                              int *new_key_direction, ha_rows *new_select_limit,
                              uint *new_used_key_parts = nullptr,
                              uint *saved_best_key_parts = nullptr);
/**
  Calculate properties of ref key: key length, number of used key parts,
  dependency map, possibility of null.

  @param keyuse               Array of keys to consider
  @param tab                  join_tab to calculate ref parameters for
  @param key                  number of the key to use
  @param used_tables          tables read prior to this table
  @param [out] chosen_keyuses when given, this function will fill array with
                              chosen keyuses
  @param [out] length_out     calculated length of the ref
  @param [out] keyparts_out   calculated number of used keyparts
  @param [out] dep_map        when given, calculated dependency map
  @param [out] maybe_null     when given, calculated maybe_null property
*/

void calc_length_and_keyparts(Key_use *keyuse, JOIN_TAB *tab, const uint key,
                              table_map used_tables, Key_use **chosen_keyuses,
                              uint *length_out, uint *keyparts_out,
                              table_map *dep_map, bool *maybe_null);

/**
  Set up the support structures (NULL bits, row offsets, etc.) for a semijoin
  duplicate weedout table. The object is allocated on the given THD's MEM_ROOT.

  @param thd the THD to allocate the object on
  @param join the JOIN that will own the temporary table (ie., has the
    responsibility to destroy it after use)
  @param first_tab first table in row key (inclusive)
  @param last_tab last table in row key (exclusive)
 */
SJ_TMP_TABLE *create_sj_tmp_table(THD *thd, JOIN *join,
                                  SJ_TMP_TABLE_TAB *first_tab,
                                  SJ_TMP_TABLE_TAB *last_tab);

#endif /* SQL_SELECT_INCLUDED */
