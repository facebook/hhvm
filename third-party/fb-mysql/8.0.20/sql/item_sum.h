#ifndef ITEM_SUM_INCLUDED
#define ITEM_SUM_INCLUDED

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

/* classes for sum functions */

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <map>
#include <memory>
#include <string>

#include "field_types.h"  // enum_field_types
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "my_table_map.h"
#include "my_time.h"
#include "my_tree.h"  // TREE
#include "mysql/udf_registration_types.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "sql/enum_query_type.h"
#include "sql/item.h"       // Item_result_field
#include "sql/item_func.h"  // Item_int_func
#include "sql/mem_root_array.h"
#include "sql/my_decimal.h"
#include "sql/parse_tree_node_base.h"
#include "sql/parse_tree_nodes.h"  // PT_window
#include "sql/sql_base.h"
#include "sql/sql_const.h"
#include "sql/sql_list.h"
#include "sql/sql_udf.h"  // udf_handler
#include "sql/window_lex.h"
#include "sql_string.h"
#include "template_utils.h"

class Field;
class Item_sum;
class Json_array;
class Json_object;
class Json_wrapper;
class PT_item_list;
class SELECT_LEX;
class THD;
class Temp_table_param;
class Window;
struct ORDER;
struct TABLE;
struct Window_evaluation_requirements;

/**
  The abstract base class for the Aggregator_* classes.
  It implements the data collection functions (setup/add/clear)
  as either pass-through to the real functionality or
  as collectors into an Unique (for distinct) structure.

  Note that update_field/reset_field are not in that
  class, because they're simply not called when
  GROUP BY/DISTINCT can be handled with help of index on grouped
  fields (allow_group_via_temp_table is false);
*/

class Aggregator {
  friend class Item_sum;
  friend class Item_sum_sum;
  friend class Item_sum_count;
  friend class Item_sum_avg;

  /*
    All members are protected as this class is not usable outside of an
    Item_sum descendant.
  */
 protected:
  /* the aggregate function class to act on */
  Item_sum *item_sum;

 public:
  Aggregator(Item_sum *arg) : item_sum(arg) {}
  virtual ~Aggregator() {}

  enum Aggregator_type { SIMPLE_AGGREGATOR, DISTINCT_AGGREGATOR };
  virtual Aggregator_type Aggrtype() = 0;

  /**
    Called before adding the first row.
    Allocates and sets up the internal aggregation structures used,
    e.g. the Unique instance used to calculate distinct.
  */
  virtual bool setup(THD *) = 0;

  /**
    Called when we need to wipe out all the data from the aggregator :
    all the values acumulated and all the state.
    Cleans up the internal structures and resets them to their initial state.
  */
  virtual void clear() = 0;

  /**
    Called when there's a new value to be aggregated.
    Updates the internal state of the aggregator to reflect the new value.
  */
  virtual bool add() = 0;

  /**
    Called when there are no more data and the final value is to be retrieved.
    Finalises the state of the aggregator, so the final result can be retrieved.
  */
  virtual void endup() = 0;

  /** Decimal value of being-aggregated argument */
  virtual my_decimal *arg_val_decimal(my_decimal *value) = 0;
  /** Floating point value of being-aggregated argument */
  virtual double arg_val_real() = 0;
  /**
    NULLness of being-aggregated argument.

    @param use_null_value Optimization: to determine if the argument is NULL
    we must, in the general case, call is_null() on it, which itself might
    call val_*() on it, which might be costly. If you just have called
    arg_val*(), you can pass use_null_value=true; this way, arg_is_null()
    might avoid is_null() and instead do a cheap read of the Item's null_value
    (updated by arg_val*()).
  */
  virtual bool arg_is_null(bool use_null_value) = 0;
};

/**
  Class Item_sum is the base class used for special expressions that SQL calls
  'set functions'. These expressions are formed with the help of aggregate
  functions such as SUM, MAX, GROUP_CONCAT etc.
  Class Item_sum is also the base class for Window functions; the text below
  first documents set functions, then window functions.

 GENERAL NOTES

  A set function cannot be used in all positions where expressions are accepted.
  There are some quite explicable restrictions for the use of set functions.

  In the query:

    SELECT AVG(b) FROM t1 WHERE SUM(b) > 20 GROUP by a

  the set function AVG(b) is valid, while the usage of SUM(b) is invalid.
  A WHERE condition must contain expressions that can be evaluated for each row
  of the table. Yet the expression SUM(b) can be evaluated only for each group
  of rows with the same value of column a.
  In the query:

    SELECT AVG(b) FROM t1 WHERE c > 30 GROUP BY a HAVING SUM(b) > 20

  both set function expressions AVG(b) and SUM(b) are valid.

  We can say that in a query without nested selects an occurrence of a
  set function in an expression of the SELECT list or/and in the HAVING
  clause is valid, while in the WHERE clause, FROM clause or GROUP BY clause
  it is invalid.

  The general rule to detect whether a set function is valid in a query with
  nested subqueries is much more complicated.

  Consider the following query:

    SELECT t1.a FROM t1 GROUP BY t1.a
      HAVING t1.a > ALL (SELECT t2.c FROM t2 WHERE SUM(t1.b) < t2.c).

  The set function SUM(b) is used here in the WHERE clause of the subquery.
  Nevertheless it is valid since it is contained in the HAVING clause of the
  outer query. The expression SUM(t1.b) is evaluated for each group defined
 in the main query, not for groups of the subquery.

  The problem of finding the query where to aggregate a particular
  set function is not so simple as it seems to be.

  In the query:
    SELECT t1.a FROM t1 GROUP BY t1.a
     HAVING t1.a > ALL(SELECT t2.c FROM t2 GROUP BY t2.c
                         HAVING SUM(t1.a) < t2.c)

  the set function can be evaluated in both the outer and the inner query block.
  If we evaluate SUM(t1.a) for the outer query then we get the value of t1.a
  multiplied by the cardinality of a group in table t1. In this case,
  SUM(t1.a) is used as a constant value in each correlated subquery.
  But SUM(t1.a) can also be evaluated for the inner query.
  In this case t1.a will be a constant value for each correlated subquery and
  summation is performed for each group of table t2.
  (Here it makes sense to remind that the query

    SELECT c FROM t GROUP BY a HAVING SUM(1) < a

  is quite valid in our SQL).

  So depending on what query block we assign the set function to we
  can get different results.

  The general rule to detect the query block Q where a set function will be
  aggregated (evaluated) can be formulated as follows.

  Reference: SQL2011 @<set function specification@> syntax rules 6 and 7.

  Consider a set function S(E) where E is an expression which contains
  column references C1, ..., Cn. Resolve all column references Ci against
  the query block Qi containing the set function S(E). Let Q be the innermost
  query block of all query blocks Qi. (It should be noted here that S(E)
  in no way can be aggregated in the query block containing the subquery Q,
  otherwise S(E) would refer to at least one unbound column reference).
  If S(E) is used in a construct of Q where set functions are allowed then
  we aggregate S(E) in Q.
  Otherwise:
  - if ANSI SQL mode is enabled (MODE_ANSI), then report an error.
  - otherwise, look for the innermost query block containing S(E) of those
    where usage of S(E) is allowed. The place of aggregation depends on which
    clause the subquery is contained within; It will be different when
    contained in a WHERE clause versus in the select list or in HAVING clause.

  Let's demonstrate how this rule is applied to the following queries.

  1. SELECT t1.a FROM t1 GROUP BY t1.a
       HAVING t1.a > ALL(SELECT t2.b FROM t2 GROUP BY t2.b
                           HAVING t2.b > ALL(SELECT t3.c FROM t3 GROUP BY t3.c
                                                HAVING SUM(t1.a+t2.b) < t3.c))
  For this query the set function SUM(t1.a+t2.b) contains t1.a and t2.b
  with t1.a defined in the outermost query, and t2.b defined for its
  subquery. The set function is contained in the HAVING clause of the subquery
  and can be evaluated in this subquery.

  2. SELECT t1.a FROM t1 GROUP BY t1.a
       HAVING t1.a > ALL(SELECT t2.b FROM t2
                           WHERE t2.b > ALL (SELECT t3.c FROM t3 GROUP BY t3.c
                                               HAVING SUM(t1.a+t2.b) < t3.c))
  The set function SUM(t1.a+t2.b) is contained in the WHERE clause of the second
  query block - the outermost query block where t1.a and t2.b are defined.
  If we evaluate the function in this subquery we violate the context rules.
  So we evaluate the function in the third query block (over table t3) where it
  is used under the HAVING clause; if in ANSI SQL mode, an error is thrown.

  3. SELECT t1.a FROM t1 GROUP BY t1.a
       HAVING t1.a > ALL(SELECT t2.b FROM t2
                           WHERE t2.b > ALL (SELECT t3.c FROM t3
                                               WHERE SUM(t1.a+t2.b) < t3.c))
  In this query, evaluation of SUM(t1.a+t2.b) is not valid neither in the second
  nor in the third query block.

  Set functions can generally not be nested. In the query

    SELECT t1.a from t1 GROUP BY t1.a HAVING AVG(SUM(t1.b)) > 20

  the expression SUM(b) is not valid, even though it is contained inside
  a HAVING clause.
  However, it is acceptable in the query:

    SELECT t.1 FROM t1 GROUP BY t1.a HAVING SUM(t1.b) > 20.

  An argument of a set function does not have to be a simple column reference
  as seen in examples above. This can be a more complex expression

    SELECT t1.a FROM t1 GROUP BY t1.a HAVING SUM(t1.b+1) > 20.

  The expression SUM(t1.b+1) has clear semantics in this context:
  we sum up the values of t1.b+1 where t1.b varies for all values within a
  group of rows that contain the same t1.a value.

  A set function for an outer query yields a constant value within a subquery.
  So the semantics of the query

    SELECT t1.a FROM t1 GROUP BY t1.a
      HAVING t1.a IN (SELECT t2.c FROM t2 GROUP BY t2.c
                        HAVING AVG(t2.c+SUM(t1.b)) > 20)

  is still clear. For a group of rows with the same value for t1.a, calculate
  the value of SUM(t1.b) as 's'. This value is substituted in the subquery:

    SELECT t2.c FROM t2 GROUP BY t2.c HAVING AVG(t2.c+s)

  By the same reason the following query with a subquery

    SELECT t1.a FROM t1 GROUP BY t1.a
      HAVING t1.a IN (SELECT t2.c FROM t2 GROUP BY t2.c
                        HAVING AVG(SUM(t1.b)) > 20)
  is also valid.

 IMPLEMENTATION NOTES

  The member base_select contains a reference to the query block that the
  set function is contained within.

  The member aggr_select contains a reference to the query block where the
  set function is aggregated.

  The field max_aggr_level holds the maximum of the nest levels of the
  unbound column references contained in the set function. A column reference
  is unbound within a set function if it is not bound by any subquery
  used as a subexpression in this function. A column reference is bound by
  a subquery if it is a reference to the column by which the aggregation
  of some set function that is used in the subquery is calculated.
  For the set function used in the query

    SELECT t1.a FROM t1 GROUP BY t1.a
      HAVING t1.a > ALL(SELECT t2.b FROM t2 GROUP BY t2.b
                          HAVING t2.b > ALL(SELECT t3.c FROM t3 GROUP BY t3.c
                                              HAVING SUM(t1.a+t2.b) < t3.c))

  the value of max_aggr_level is equal to 1 since t1.a is bound in the main
  query, and t2.b is bound by the first subquery whose nest level is 1.
  Obviously a set function cannot be aggregated in a subquery whose
  nest level is less than max_aggr_level. (Yet it can be aggregated in the
  subqueries whose nest level is greater than max_aggr_level.)
  In the query
    SELECT t1.a FROM t1 HAVING AVG(t1.a+(SELECT MIN(t2.c) FROM t2))

  the value of the max_aggr_level for the AVG set function is 0 since
  the reference t2.c is bound in the subquery.

  If a set function contains no column references (like COUNT(*)),
  max_aggr_level is -1.

  The field 'max_sum_func_level' is to contain the maximum of the
  nest levels of the set functions that are used as subexpressions of
  the arguments of the given set function, but not aggregated in any
  subquery within this set function. A nested set function s1 can be
  used within set function s0 only if s1.max_sum_func_level <
  s0.max_sum_func_level. Set function s1 is considered as nested
  for set function s0 if s1 is not calculated in any subquery
  within s0.

  A set function that is used as a subexpression in an argument of another
  set function refers to the latter via the field 'in_sum_func'.

  The condition imposed on the usage of set functions are checked when
  we traverse query subexpressions with the help of the recursive method
  fix_fields. When we apply this method to an object of the class
  Item_sum, first, on the descent, we call the method init_sum_func_check
  that initialize members used at checking. Then, on the ascent, we
  call the method check_sum_func that validates the set function usage
  and reports an error if it is invalid.
  The method check_sum_func serves to link the items for the set functions
  that are aggregated in the containing query blocks. Circular chains of such
  functions are attached to the corresponding SELECT_LEX structures
  through the field inner_sum_func_list.

  Exploiting the fact that the members mentioned above are used in one
  recursive function we could have allocated them on the thread stack.
  Yet we don't do it now.

  It is assumed that the nesting level of subqueries does not exceed 63
  (valid nesting levels are stored in a 64-bit bitmap called nesting_map).
  The assumption is enforced in LEX::new_query().

  WINDOW FUNCTIONS

  Most set functions (e.g. SUM, COUNT, AVG) can also be used as window
  functions. In that case, notable differences compared to set functions are:
  - not using any Aggregator
  - not supporting DISTINCT
  - val_*() does more than returning the function's current value: it
  first accumulates the function's argument into the function's
  state. Execution (e.g. end_write_wf()) manipulates temporary tables which
  contain input for WFs; each input row is passed to copy_funcs() which calls
  the WF's val_*() to accumulate it.
*/

class Item_sum : public Item_result_field {
  typedef Item_result_field super;

  friend class Aggregator_distinct;
  friend class Aggregator_simple;

 protected:
  /**
    Aggregator class instance. Not set initially. Allocated only after
    it is determined if the incoming data are already distinct.
  */
  Aggregator *aggr;

  /**
    If sum is a window function, this field contains the window.
  */
  PT_window *m_window;
  /**
    True if we have already resolved this window functions window reference.
    Used in execution of prepared statement to avoid re-resolve.
  */
  bool m_window_resolved;

 private:
  /**
    Used in making ROLLUP. Set for the ROLLUP copies of the original
    Item_sum and passed to create_tmp_field() to cause it to work
    over the temp table buffer that is referenced by
    Item_result_field::result_field.
  */
  bool force_copy_fields;

  /**
    Indicates how the aggregate function was specified by the parser :
     true if it was written as AGGREGATE(DISTINCT),
     false if it was AGGREGATE()
  */
  bool with_distinct;

 public:
  bool has_force_copy_fields() const { return force_copy_fields; }
  bool has_with_distinct() const { return with_distinct; }

  enum Sumfunctype {
    COUNT_FUNC,           // COUNT
    COUNT_DISTINCT_FUNC,  // COUNT (DISTINCT)
    SUM_FUNC,             // SUM
    SUM_DISTINCT_FUNC,    // SUM (DISTINCT)
    AVG_FUNC,             // AVG
    AVG_DISTINCT_FUNC,    // AVG (DISTINCT)
    MIN_FUNC,             // MIN
    MAX_FUNC,             // MAX
    STD_FUNC,             // STD/STDDEV/STDDEV_POP
    VARIANCE_FUNC,        // VARIANCE/VAR_POP and VAR_SAMP
    SUM_BIT_FUNC,         // BIT_AND, BIT_OR and BIT_XOR
    UDF_SUM_FUNC,         // user defined functions
    GROUP_CONCAT_FUNC,    // GROUP_CONCAT
    JSON_AGG_FUNC,        // JSON_ARRAYAGG and JSON_OBJECTAGG
    ROW_NUMBER_FUNC,      // Window functions
    RANK_FUNC,
    DENSE_RANK_FUNC,
    CUME_DIST_FUNC,
    PERCENT_RANK_FUNC,
    NTILE_FUNC,
    LEAD_LAG_FUNC,
    FIRST_LAST_VALUE_FUNC,
    NTH_VALUE_FUNC
  };

  /**
    @note most member variables below serve only for grouped aggregate
    functions.
  */

  /**
    For a group aggregate which is aggregated into an outer query
    block; none, or just the first or both cells may be non-zero. They are
    filled with references to the group aggregate (for example if it is the
    argument of a function; it is then a pointer to that function's args[i]
    pointer).
  */
  Item **referenced_by[2];
  Item_sum *next_sum;     ///< next in the circular chain of registered objects
  Item_sum *in_sum_func;  ///< the containing set function if any
  SELECT_LEX *base_select;  ///< query block where function is placed
  /**
    For a group aggregate, query block where function is aggregated. For a
    window function, nullptr, as such function is always aggregated in
    base_select, as it mustn't contain any outer reference.
  */
  SELECT_LEX *aggr_select;
  int8 max_aggr_level;  ///< max level of unbound column references
  int8
      max_sum_func_level;  ///< max level of aggregation for contained functions
  bool allow_group_via_temp_table;  ///< If incremental update of fields is
                                    ///< supported.
  /**
    WFs are forbidden when resolving Item_sum; this member is used to restore
    WF allowance status afterwards.
  */
  nesting_map save_deny_window_func;

 protected:
  uint arg_count;
  Item **args, *tmp_args[2];
  table_map used_tables_cache;
  bool forced_const;
  static ulonglong ram_limitation(THD *thd);

 public:
  void mark_as_sum_func();
  void mark_as_sum_func(SELECT_LEX *);

  Item_sum(const POS &pos, PT_window *w)
      : super(pos),
        m_window(w),
        m_window_resolved(false),
        next_sum(nullptr),
        allow_group_via_temp_table(true),
        arg_count(0),
        args(nullptr),
        used_tables_cache(0),
        forced_const(false) {
    init_aggregator();
  }

  Item_sum(Item *a)
      : m_window(nullptr),
        m_window_resolved(false),
        next_sum(nullptr),
        allow_group_via_temp_table(true),
        arg_count(1),
        args(tmp_args),
        used_tables_cache(0),
        forced_const(false) {
    args[0] = a;
    mark_as_sum_func();
    init_aggregator();
  }

  Item_sum(const POS &pos, Item *a, PT_window *w)
      : super(pos),
        m_window(w),
        m_window_resolved(false),
        next_sum(nullptr),
        allow_group_via_temp_table(true),
        arg_count(1),
        args(tmp_args),
        used_tables_cache(0),
        forced_const(false) {
    args[0] = a;
    init_aggregator();
  }

  Item_sum(const POS &pos, Item *a, Item *b, PT_window *w)
      : super(pos),
        m_window(w),
        m_window_resolved(false),
        next_sum(nullptr),
        allow_group_via_temp_table(true),
        arg_count(2),
        args(tmp_args),
        used_tables_cache(0),
        forced_const(false) {
    args[0] = a;
    args[1] = b;
    init_aggregator();
  }

  Item_sum(const POS &pos, PT_item_list *opt_list, PT_window *w);

  /// Copy constructor, need to perform subqueries with temporary tables
  Item_sum(THD *thd, const Item_sum *item);

  bool itemize(Parse_context *pc, Item **res) override;
  Type type() const override { return SUM_FUNC_ITEM; }
  virtual enum Sumfunctype sum_func() const = 0;

  /**
    Resets the aggregate value to its default and aggregates the current
    value of its attribute(s).
  */
  inline bool reset_and_add() {
    aggregator_clear();
    return aggregator_add();
  }

  /*
    Called when new group is started and results are being saved in
    a temporary table. Similarly to reset_and_add() it resets the
    value to its default and aggregates the value of its
    attribute(s), but must also store it in result_field.
    This set of methods (result_item(), reset_field, update_field()) of
    Item_sum is used only if allow_group_via_temp_table is true. Otherwise
    copy_or_same() is used to obtain a copy of this item.
  */
  virtual void reset_field() = 0;
  /*
    Called for each new value in the group, when temporary table is in use.
    Similar to add(), but uses temporary table field to obtain current value,
    Updated value is then saved in the field.
  */
  virtual void update_field() = 0;
  virtual bool keep_field_type() const { return false; }
  bool resolve_type(THD *) override;
  virtual Item *result_item(Field *field) { return new Item_field(field); }
  table_map used_tables() const override { return used_tables_cache; }
  void update_used_tables() override;
  void fix_after_pullout(SELECT_LEX *parent_select,
                         SELECT_LEX *removed_select) override;
  void add_used_tables_for_aggr_func();
  bool is_null() override { return null_value; }
  void make_const() {
    used_tables_cache = 0;
    forced_const = true;
  }
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  void fix_num_length_and_dec();
  bool eq(const Item *item, bool binary_cmp) const override;
  /**
    Mark an aggregate as having no rows.

    This function is called by the execution engine to assign 'NO ROWS
    FOUND' value to an aggregate item, when the underlying result set
    has no rows. Such value, in a general case, may be different from
    the default value of the item after 'clear()': e.g. a numeric item
    may be initialized to 0 by clear() and to NULL by
    no_rows_in_result().
  */
  void no_rows_in_result() override {
    set_aggregator(with_distinct ? Aggregator::DISTINCT_AGGREGATOR
                                 : Aggregator::SIMPLE_AGGREGATOR);
    aggregator_clear();
  }
  virtual void make_unique() { force_copy_fields = true; }
  virtual Field *create_tmp_field(bool group, TABLE *table);

  /// argument used by walk method collect_grouped_aggregates ("cga")
  struct Collect_grouped_aggregate_info {
    /// accumulated all aggregates found
    std::vector<Item_sum *> list;
    /**
      The query block we walk from. All found aggregates must aggregate in
      this; if some aggregate in outer query blocks, break off transformation.
    */
    SELECT_LEX *m_select{nullptr};
    /// true: break off transformation
    bool m_break_off{false};
    Collect_grouped_aggregate_info(SELECT_LEX *select) : m_select(select) {}
  };

  bool collect_grouped_aggregates(uchar *) override;
  Item *replace_aggregate(uchar *) override;
  bool collect_scalar_subqueries(uchar *) override;
  bool collect_item_field_or_view_ref_processor(uchar *) override;

  bool walk(Item_processor processor, enum_walk walk, uchar *arg) override;
  Item *transform(Item_transformer transformer, uchar *arg) override;
  bool clean_up_after_removal(uchar *arg) override;
  bool aggregate_check_group(uchar *arg) override;
  bool aggregate_check_distinct(uchar *arg) override;
  bool has_aggregate_ref_in_group_by(uchar *arg) override;
  bool init_sum_func_check(THD *thd);
  bool check_sum_func(THD *thd, Item **ref);

  Item *get_arg(uint i) { return args[i]; }
  Item *set_arg(uint i, THD *thd, Item *new_val);
  uint get_arg_count() const { return arg_count; }
  /// @todo delete this when we no longer support temporary transformations
  Item **get_arg_ptr(uint i) { return &args[i]; }

  bool fix_fields(THD *thd, Item **ref) override;

  /* Initialization of distinct related members */
  void init_aggregator() {
    aggr = nullptr;
    with_distinct = false;
    force_copy_fields = false;
  }

  /**
    Called to initialize the aggregator.
  */

  inline bool aggregator_setup(THD *thd) { return aggr->setup(thd); }

  /**
    Called to cleanup the aggregator.
  */

  inline void aggregator_clear() { aggr->clear(); }

  /**
    Called to add value to the aggregator.
  */

  inline bool aggregator_add() { return aggr->add(); }

  /* stores the declared DISTINCT flag (from the parser) */
  void set_distinct(bool distinct) {
    with_distinct = distinct;
    allow_group_via_temp_table = !with_distinct;
  }

  /*
    Set the type of aggregation : DISTINCT or not.

    May be called multiple times.
  */

  int set_aggregator(Aggregator::Aggregator_type aggregator);

  virtual void clear() = 0;
  virtual bool add() = 0;
  virtual bool setup(THD *) { return false; }

  /**
    Only relevant for aggregates qua window functions. Checks semantics after
    windows have been set up and checked. Window functions have specific
    requirements on the window specifications.

    @param thd                    Current thread
    @param select                 The current select
    @param [out] reqs             Holds collected requirements from this wf

    @returns true if error
   */
  virtual bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                                  Window_evaluation_requirements *reqs);

  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;

  void cleanup() override;

  const Window *window() const { return down_cast<Window *>(m_window); }
  bool reset_wf_state(uchar *arg) override;

  /**
    All aggregates are framing, i.e. they work on the window's frame. If none
    is defined, the frame is by default the entire partition, unless ORDER BY
    is defined, in which case it is the set of rows from the start of the
    partition to and including the peer set of the current row.

    Some window functions are not framing, i.e. they always work on the entire
    partition. For such window functions, the method is overridden to
    return false.
  */
  virtual bool framing() const { return true; }

  /**
    Only for framing window functions. True if this function only needs to
    read one row per frame.
  */
  virtual bool uses_only_one_row() const { return false; }

  /**
    Return true if we need to make two passes over the rows in the partition -
    either because we need the cardinality of it (and we need to read all
    rows to detect the next partition), or we need to have all partition rows
    available to evaluate the window function for some other reason, e.g.
    we may need the last row in the partition in the frame buffer to be able
    to evaluate LEAD.
  */
  virtual bool needs_card() const { return false; }

  /**
    Common initial actions for window functions. For non-buffered processing
    ("on-the-fly"), check partition change and possible reset partition
    state. In this case return false.
    For buffered processing, if windowing state m_do_copy_null is true, set
    null_value to true and return true.

    @return true if case two above holds, else false
  */
  bool wf_common_init();

 protected:
  /*
    Raise an error (ER_NOT_SUPPORTED_YET) with the detail that this
    function is not yet supported as a window function.
  */
  void unsupported_as_wf() {
    char buff[STRING_BUFFER_USUAL_SIZE];
    snprintf(buff, sizeof(buff), "%s as window function", func_name());
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), buff);
  }
};

class Unique;

/**
 The distinct aggregator.
 Implements AGGFN (DISTINCT ..)
 Collects all the data into an Unique (similarly to what Item_sum_distinct
 does currently) and then (if applicable) iterates over the list of
 unique values and pumps them back into its object
*/

class Aggregator_distinct : public Aggregator {
  friend class Item_sum_sum;

  /*
    flag to prevent consecutive runs of endup(). Normally in endup there are
    expensive calculations (like walking the distinct tree for example)
    which we must do only once if there are no data changes.
    We can re-use the data for the second and subsequent val_xxx() calls.
    endup_done set to true also means that the calculated values for
    the aggregate functions are correct and don't need recalculation.
  */
  bool endup_done;

  /*
    Used depending on the type of the aggregate function and the presence of
    blob columns in it:
    - For COUNT(DISTINCT) and no blob fields this points to a real temporary
      table. It's used as a hash table.
    - For AVG/SUM(DISTINCT) or COUNT(DISTINCT) with blob fields only the
      in-memory data structure of a temporary table is constructed.
      It's used by the Field classes to transform data into row format.
  */
  TABLE *table;

  /*
    An array of field lengths on row allocated and used only for
    COUNT(DISTINCT) with multiple columns and no blobs. Used in
    Aggregator_distinct::composite_key_cmp (called from Unique to compare
    nodes
  */
  uint32 *field_lengths;

  /*
    Used in conjunction with 'table' to support the access to Field classes
    for COUNT(DISTINCT). Needed by copy_fields()/copy_funcs().
  */
  Temp_table_param *tmp_table_param;

  /*
    If there are no blobs in the COUNT(DISTINCT) arguments, we can use a tree,
    which is faster than heap table. In that case, we still use the table
    to help get things set up, but we insert nothing in it.
    For AVG/SUM(DISTINCT) we always use this tree (as it takes a single
    argument) to get the distinct rows.
  */
  Unique *tree;

  /*
    The length of the temp table row. Must be a member of the class as it
    gets passed down to simple_raw_key_cmp () as a compare function argument
    to Unique. simple_raw_key_cmp () is used as a fast comparison function
    when the entire row can be binary compared.
  */
  uint tree_key_length;

  enum Const_distinct {
    NOT_CONST = 0,
    /**
      Set to true if the result is known to be always NULL.
      If set deactivates creation and usage of the temporary table (in the
      'table' member) and the Unique instance (in the 'tree' member) as well as
      the calculation of the final value on the first call to
      @c Item_sum::val_xxx(),
      @c Item_avg::val_xxx(),
      @c Item_count::val_xxx().
     */
    CONST_NULL,
    /**
      Set to true if count distinct is on only const items. Distinct on a const
      value will always be the constant itself. And count distinct of the same
      would always be 1. Similar to CONST_NULL, it avoids creation of temporary
      table and the Unique instance.
     */
    CONST_NOT_NULL
  } const_distinct;

  /**
    When feeding back the data in endup() from Unique/temp table back to
    Item_sum::add() methods we must read the data from Unique (and not
    recalculate the functions that are given as arguments to the aggregate
    function.
    This flag is to tell the arg_*() methods to take the data from the Unique
    instead of calling the relevant val_..() method.
  */
  bool use_distinct_values;

 public:
  Aggregator_distinct(Item_sum *sum)
      : Aggregator(sum),
        table(nullptr),
        tmp_table_param(nullptr),
        tree(nullptr),
        const_distinct(NOT_CONST),
        use_distinct_values(false) {}
  ~Aggregator_distinct() override;
  Aggregator_type Aggrtype() override { return DISTINCT_AGGREGATOR; }

  bool setup(THD *) override;
  void clear() override;
  bool add() override;
  void endup() override;
  my_decimal *arg_val_decimal(my_decimal *value) override;
  double arg_val_real() override;
  bool arg_is_null(bool use_null_value) override;

  bool unique_walk_function(void *element);
  static int composite_key_cmp(const void *arg, const void *a, const void *b);
};

/**
  The pass-through aggregator.
  Implements AGGFN (DISTINCT ..) by knowing it gets distinct data on input.
  So it just pumps them back to the Item_sum descendant class.
*/
class Aggregator_simple : public Aggregator {
 public:
  Aggregator_simple(Item_sum *sum) : Aggregator(sum) {}
  Aggregator_type Aggrtype() override { return Aggregator::SIMPLE_AGGREGATOR; }

  bool setup(THD *thd) override { return item_sum->setup(thd); }
  void clear() override { item_sum->clear(); }
  bool add() override { return item_sum->add(); }
  void endup() override {}
  my_decimal *arg_val_decimal(my_decimal *value) override;
  double arg_val_real() override;
  bool arg_is_null(bool use_null_value) override;
};

class Item_sum_num : public Item_sum {
  typedef Item_sum super;

 protected:
  /*
   val_xxx() functions may be called several times during the execution of a
   query. Derived classes that require extensive calculation in val_xxx()
   maintain cache of aggregate value. This variable governs the validity of
   that cache.
  */
  bool is_evaluated;

 public:
  Item_sum_num(const POS &pos, Item *item_par, PT_window *window)
      : Item_sum(pos, item_par, window), is_evaluated(false) {}

  Item_sum_num(const POS &pos, PT_item_list *list, PT_window *w)
      : Item_sum(pos, list, w), is_evaluated(false) {}

  Item_sum_num(THD *thd, Item_sum_num *item)
      : Item_sum(thd, item), is_evaluated(item->is_evaluated) {}
  bool fix_fields(THD *, Item **) override;
  longlong val_int() override {
    DBUG_ASSERT(fixed == 1);
    return llrint_with_overflow_check(val_real()); /* Real as default */
  }
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_numeric(ltime, fuzzydate); /* Decimal or real */
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_numeric(ltime); /* Decimal or real */
  }
  void reset_field() override;
};

class Item_sum_int : public Item_sum_num {
 public:
  Item_sum_int(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_num(pos, item_par, w) {
    set_data_type_longlong();
  }

  Item_sum_int(const POS &pos, PT_item_list *list, PT_window *w)
      : Item_sum_num(pos, list, w) {
    set_data_type_longlong();
  }

  Item_sum_int(THD *thd, Item_sum_int *item) : Item_sum_num(thd, item) {
    set_data_type_longlong();
  }

  bool resolve_type(THD *) override {
    maybe_null = false;
    for (uint i = 0; i < arg_count; i++) {
      maybe_null |= args[i]->maybe_null;
    }
    null_value = false;
    return false;
  }
  double val_real() override {
    DBUG_ASSERT(fixed);
    return static_cast<double>(val_int());
  }
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override { return get_time_from_int(ltime); }
  enum Item_result result_type() const override { return INT_RESULT; }
};

class Item_sum_sum : public Item_sum_num {
 protected:
  Item_result hybrid_type;
  double sum;
  my_decimal dec_buffs[2];
  uint curr_dec_buff;
  bool resolve_type(THD *thd) override;
  /**
    Execution state: this is for counting rows entering and leaving the window
    frame, see #m_frame_null_count.
  */
  ulonglong m_count;

  /**
    Execution state: this is for counting NULLs of rows entering and leaving
    the window frame, when we use optimized inverse-based computations. By
    comparison with m_count we can know how many non-NULLs are in the frame.
  */
  ulonglong m_frame_null_count;

 public:
  Item_sum_sum(const POS &pos, Item *item_par, bool distinct, PT_window *window)
      : Item_sum_num(pos, item_par, window),
        hybrid_type(INVALID_RESULT),
        m_count(0),
        m_frame_null_count(0) {
    set_distinct(distinct);
  }

  Item_sum_sum(THD *thd, Item_sum_sum *item);
  enum Sumfunctype sum_func() const override {
    return has_with_distinct() ? SUM_DISTINCT_FUNC : SUM_FUNC;
  }
  void clear() override;
  bool add() override;
  double val_real() override;
  longlong val_int() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *) override;
  enum Item_result result_type() const override { return hybrid_type; }
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  void reset_field() override;
  void update_field() override;
  void no_rows_in_result() override {}
  const char *func_name() const override { return "sum"; }
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_count : public Item_sum_int {
  longlong count;

  friend class Aggregator_distinct;

  void clear() override;
  bool add() override;
  void cleanup() override;

 public:
  Item_sum_count(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_int(pos, item_par, w), count(0) {}

  /**
    Constructs an instance for COUNT(DISTINCT)

    @param pos  Position of token in the parser.
    @param list A list of the arguments to the aggregate function
    @param w    A window, if COUNT is used as a windowing function

    This constructor is called by the parser only for COUNT (DISTINCT).
  */

  Item_sum_count(const POS &pos, PT_item_list *list, PT_window *w)
      : Item_sum_int(pos, list, w), count(0) {
    set_distinct(true);
  }
  Item_sum_count(THD *thd, Item_sum_count *item)
      : Item_sum_int(thd, item), count(item->count) {}
  enum Sumfunctype sum_func() const override {
    return has_with_distinct() ? COUNT_DISTINCT_FUNC : COUNT_FUNC;
  }
  bool resolve_type(THD *) override {
    maybe_null = false;
    null_value = false;
    return false;
  }
  void no_rows_in_result() override { count = 0; }
  void make_const(longlong count_arg) {
    count = count_arg;
    Item_sum::make_const();
  }
  longlong val_int() override;
  void reset_field() override;
  void update_field() override;
  const char *func_name() const override { return "count"; }
  Item *copy_or_same(THD *thd) override;
};

/* Item to get the value of a stored sum function */

class Item_sum_avg;
class Item_sum_bit;

/**
  This is used in connection which a parent Item_sum:
  - which can produce different result types (is "hybrid")
  - which stores function's value into a temporary table's column (one
  row per group).
  - which stores in the column some internal piece of information which should
  not be returned to the user, so special implementations are needed.
*/
class Item_sum_hybrid_field : public Item_result_field {
 protected:
  /// The tmp table's column containing the value of the set function.
  Field *field;
  /// Stores the Item's result type.
  Item_result hybrid_type;

 public:
  enum Item_result result_type() const override { return hybrid_type; }
  bool mark_field_in_map(uchar *arg) override {
    /*
      Filesort (find_all_keys) over a temporary table collects the columns it
      needs.
    */
    return Item::mark_field_in_map(pointer_cast<Mark_field *>(arg), field);
  }
  bool check_function_as_value_generator(uchar *args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(args);
    func_arg->banned_function_name = func_name();
    return true;
  }
};

/**
  Common abstract class for:
    Item_avg_field
    Item_variance_field
*/
class Item_sum_num_field : public Item_sum_hybrid_field {
 public:
  longlong val_int() override {
    /* can't be fix_fields()ed */
    return llrint_with_overflow_check(val_real());
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_numeric(ltime, fuzzydate); /* Decimal or real */
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_numeric(ltime); /* Decimal or real */
  }
  bool is_null() override {
    /*
      TODO : Implement error handling for this function as
      update_null_value() can return error.
    */
    (void)update_null_value();
    return null_value;
  }
};

class Item_avg_field : public Item_sum_num_field {
 public:
  uint f_precision, f_scale, dec_bin_size;
  uint prec_increment;
  Item_avg_field(Item_result res_type, Item_sum_avg *item);
  enum Type type() const override { return FIELD_AVG_ITEM; }
  double val_real() override;
  my_decimal *val_decimal(my_decimal *) override;
  String *val_str(String *) override;
  bool resolve_type(THD *) override { return false; }
  const char *func_name() const override {
    DBUG_ASSERT(0);
    return "avg_field";
  }
};

/// This is used in connection with an Item_sum_bit, @see Item_sum_hybrid_field
class Item_sum_bit_field : public Item_sum_hybrid_field {
 protected:
  ulonglong reset_bits;

 public:
  Item_sum_bit_field(Item_result res_type, Item_sum_bit *item,
                     ulonglong reset_bits);
  longlong val_int() override;
  double val_real() override;
  my_decimal *val_decimal(my_decimal *) override;
  String *val_str(String *) override;
  bool resolve_type(THD *) override { return false; }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  enum Type type() const override { return FIELD_BIT_ITEM; }
  const char *func_name() const override {
    DBUG_ASSERT(0);
    return "sum_bit_field";
  }
};

/// Common abstraction for Item_sum_json_array and Item_sum_json_object
class Item_sum_json : public Item_sum {
  typedef Item_sum super;

 protected:
  /// String used when reading JSON binary values or JSON text values.
  String m_value;
  /// String used for converting JSON text values to utf8mb4 charset.
  String m_conversion_buffer;
  /// Wrapper around the container (object/array) which accumulates the value.
  unique_ptr_destroy_only<Json_wrapper> m_wrapper;

  /**
    Construct an Item_sum_json instance.

    @param wrapper a wrapper around the Json_array or Json_object that contains
                   the aggregated result
    @param parent_args arguments to forward to Item_sum's constructor
  */
  template <typename... Args>
  explicit Item_sum_json(unique_ptr_destroy_only<Json_wrapper> wrapper,
                         Args &&... parent_args);

 public:
  ~Item_sum_json() override;
  bool fix_fields(THD *thd, Item **pItem) override;
  enum Sumfunctype sum_func() const override { return JSON_AGG_FUNC; }
  Item_result result_type() const override { return STRING_RESULT; }

  double val_real() override;
  longlong val_int() override;
  String *val_str(String *str) override;
  bool val_json(Json_wrapper *wr) override;
  my_decimal *val_decimal(my_decimal *decimal_buffer) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;

  void reset_field() override;
  void update_field() override;

  bool check_wf_semantics(THD *thd MY_ATTRIBUTE((unused)),
                          SELECT_LEX *select MY_ATTRIBUTE((unused)),
                          Window_evaluation_requirements *reqs
                              MY_ATTRIBUTE((unused))) override;
};

/// Implements aggregation of values into an array.
class Item_sum_json_array final : public Item_sum_json {
  /// Accumulates the final value.
  unique_ptr_destroy_only<Json_array> m_json_array;

 public:
  Item_sum_json_array(THD *thd, Item_sum *item,
                      unique_ptr_destroy_only<Json_wrapper> wrapper,
                      unique_ptr_destroy_only<Json_array> array);
  Item_sum_json_array(const POS &pos, Item *a, PT_window *w,
                      unique_ptr_destroy_only<Json_wrapper> wrapper,
                      unique_ptr_destroy_only<Json_array> array);
  ~Item_sum_json_array() override;
  const char *func_name() const override { return "json_arrayagg"; }
  void clear() override;
  bool add() override;
  Item *copy_or_same(THD *thd) override;
};

/// Implements aggregation of values into an object.
class Item_sum_json_object final : public Item_sum_json {
  /// Accumulates the final value.
  unique_ptr_destroy_only<Json_object> m_json_object;
  /// Buffer used to get the value of the key.
  String m_tmp_key_value;
  /**
     Map of keys in Json_object and the count for each key
     within a window frame. It is used in handling rows
     leaving a window frame when rows are not sorted
     according to the key in Json_object.
   */
  std::map<std::string, int> m_key_map;
  /**
    If window provides ordering on the key in Json_object,
    a key_map is not needed to handle rows leaving a window
    frame. In this case, process_buffered_windowing_record()
    will set flags when a key/value pair can be removed from
    the Json_object.
  */
  bool m_optimize{false};

 public:
  Item_sum_json_object(THD *thd, Item_sum *item,
                       unique_ptr_destroy_only<Json_wrapper> wrapper,
                       unique_ptr_destroy_only<Json_object> object);
  Item_sum_json_object(const POS &pos, Item *a, Item *b, PT_window *w,
                       unique_ptr_destroy_only<Json_wrapper> wrapper,
                       unique_ptr_destroy_only<Json_object> object);
  ~Item_sum_json_object() override;
  const char *func_name() const override { return "json_objectagg"; }
  void clear() override;
  bool add() override;
  Item *copy_or_same(THD *thd) override;
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
};

class Item_sum_avg final : public Item_sum_sum {
 public:
  uint prec_increment;
  uint f_precision, f_scale, dec_bin_size;
  typedef Item_sum_sum super;
  my_decimal m_avg_dec;
  double m_avg;

  Item_sum_avg(const POS &pos, Item *item_par, bool distinct, PT_window *w)
      : Item_sum_sum(pos, item_par, distinct, w) {}

  Item_sum_avg(THD *thd, Item_sum_avg *item)
      : Item_sum_sum(thd, item), prec_increment(item->prec_increment) {}

  bool resolve_type(THD *thd) override;
  enum Sumfunctype sum_func() const override {
    return has_with_distinct() ? AVG_DISTINCT_FUNC : AVG_FUNC;
  }
  void clear() override;
  bool add() override;
  double val_real() override;
  // In SPs we might force the "wrong" type with select into a declare variable
  longlong val_int() override { return llrint_with_overflow_check(val_real()); }
  my_decimal *val_decimal(my_decimal *) override;
  String *val_str(String *str) override;
  void reset_field() override;
  void update_field() override;
  Item *result_item(Field *) override {
    return new Item_avg_field(hybrid_type, this);
  }
  void no_rows_in_result() override {}
  const char *func_name() const override { return "avg"; }
  Item *copy_or_same(THD *thd) override;
  Field *create_tmp_field(bool group, TABLE *table) override;
  void cleanup() override {
    m_count = 0;
    m_frame_null_count = 0;
    Item_sum_sum::cleanup();
  }
};

class Item_sum_variance;

class Item_variance_field : public Item_sum_num_field {
 protected:
  uint sample;

 public:
  Item_variance_field(Item_sum_variance *item);
  enum Type type() const override { return FIELD_VARIANCE_ITEM; }
  double val_real() override;
  String *val_str(String *str) override { return val_string_from_real(str); }
  my_decimal *val_decimal(my_decimal *dec_buf) override {
    return val_decimal_from_real(dec_buf);
  }
  bool resolve_type(THD *) override { return false; }
  const char *func_name() const override {
    DBUG_ASSERT(0);
    return "variance_field";
  }
  bool check_function_as_value_generator(uchar *args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(args);
    func_arg->err_code = func_arg->get_unnamed_function_error_code();
    return true;
  }
};

/*
  variance(a) =

  =  sum (ai - avg(a))^2 / count(a) )
  =  sum (ai^2 - 2*ai*avg(a) + avg(a)^2) / count(a)
  =  (sum(ai^2) - sum(2*ai*avg(a)) + sum(avg(a)^2))/count(a) =
  =  (sum(ai^2) - 2*avg(a)*sum(a) + count(a)*avg(a)^2)/count(a) =
  =  (sum(ai^2) - 2*sum(a)*sum(a)/count(a) + count(a)*sum(a)^2/count(a)^2
  )/count(a) = =  (sum(ai^2) - 2*sum(a)^2/count(a) + sum(a)^2/count(a)
  )/count(a) = =  (sum(ai^2) - sum(a)^2/count(a))/count(a)

  But, this falls prey to catastrophic cancellation.
  Instead, we use recurrence formulas in Algorithm I mentoned below
  for group aggregates.

  Algorithm I:
  M_{1} = x_{1}, ~ M_{k} = M_{k-1} + (x_{k} - M_{k-1}) / k newline
  S_{1} = 0, ~ S_{k} = S_{k-1} + (x_{k} - M_{k-1}) times (x_{k} - M_{k}) newline
  for 2 <= k <= n newline
  ital variance = S_{n} / (n-1)

  For aggregate window functions algorithm I cannot be optimized for
  moving frames since M_{i} changes for every row. So we use the
  following algorithm.

  Algorithm II:

  K = 0
  n = 0
  ex = 0
  ex2 = 0

  def add_sample(x):
  if (n == 0):
  K = x
  n = n + 1
  ex += x - K
  ex2 += (x - K) * (x - K)

  def remove_sample(x):
  n = n - 1
  ex -= (x - K)
  ex2 -= (x - K) * (x - K)

  def variance():
  return (ex2 - (ex*ex)/n) / (n-1)

  This formula facilitates incremental computation enabling us to
  optimize in case of moving window frames. The optimized codepath is taken
  only when windowing_use_high_precision is set to false. By default,
  aggregate window functions take the non-optimized codepath.
  Note:
  Results could differ between optimized and non-optimized code path.
  Hence algorithm II is used only when user sets
  windowing_use_high_precision to false.
*/

class Item_sum_variance : public Item_sum_num {
  bool resolve_type(THD *) override;

 public:
  Item_result hybrid_type;
  /**
    Used in recurrence relation.
  */
  double recurrence_m, recurrence_s;
  double recurrence_s2;
  ulonglong count;
  uint sample;
  uint prec_increment;
  /**
    If set, uses a algorithm II mentioned in the class description
    to calculate the variance which helps in optimizing windowing
    functions in presence of frames.
  */
  bool optimize;

  Item_sum_variance(const POS &pos, Item *item_par, uint sample_arg,
                    PT_window *w)
      : Item_sum_num(pos, item_par, w),
        hybrid_type(REAL_RESULT),
        count(0),
        sample(sample_arg),
        optimize(false) {}

  Item_sum_variance(THD *thd, Item_sum_variance *item);
  enum Sumfunctype sum_func() const override { return VARIANCE_FUNC; }
  void clear() override;
  bool add() override;
  double val_real() override;
  my_decimal *val_decimal(my_decimal *) override;
  void reset_field() override;
  void update_field() override;
  Item *result_item(Field *) override { return new Item_variance_field(this); }
  void no_rows_in_result() override {}
  const char *func_name() const override {
    return sample ? "var_samp" : "variance";
  }
  Item *copy_or_same(THD *thd) override;
  Field *create_tmp_field(bool group, TABLE *table) override;
  enum Item_result result_type() const override { return REAL_RESULT; }
  void cleanup() override {
    count = 0;
    Item_sum_num::cleanup();
  }
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
};

class Item_sum_std;

class Item_std_field final : public Item_variance_field {
 public:
  Item_std_field(Item_sum_std *item);
  enum Type type() const override { return FIELD_STD_ITEM; }
  double val_real() override;
  my_decimal *val_decimal(my_decimal *) override;
  enum Item_result result_type() const override { return REAL_RESULT; }
  const char *func_name() const override {
    DBUG_ASSERT(0);
    return "std_field";
  }
  bool check_function_as_value_generator(uchar *args) override {
    Check_function_as_value_generator_parameters *func_arg =
        pointer_cast<Check_function_as_value_generator_parameters *>(args);
    func_arg->err_code = func_arg->get_unnamed_function_error_code();
    return true;
  }
};

/*
   standard_deviation(a) = sqrt(variance(a))
*/

class Item_sum_std : public Item_sum_variance {
 public:
  Item_sum_std(const POS &pos, Item *item_par, uint sample_arg, PT_window *w)
      : Item_sum_variance(pos, item_par, sample_arg, w) {}

  Item_sum_std(THD *thd, Item_sum_std *item) : Item_sum_variance(thd, item) {}
  enum Sumfunctype sum_func() const override { return STD_FUNC; }
  double val_real() override;
  Item *result_item(Field *) override { return new Item_std_field(this); }
  const char *func_name() const override {
    return sample ? "stddev_samp" : "std";
  }
  Item *copy_or_same(THD *thd) override;
  enum Item_result result_type() const override { return REAL_RESULT; }
};

// This class is a string or number function depending on num_func
class Arg_comparator;

/**
  Abstract base class for the MIN and MAX aggregate functions.
*/
class Item_sum_hybrid : public Item_sum {
  typedef Item_sum super;

 private:
  /**
    Tells if this is the MIN function (true) or the MAX function (false).
  */
  const bool m_is_min;
  /*
    For window functions MIN/MAX with optimized code path, no comparisons
    are needed beyond NULL detection: MIN/MAX are then roughly equivalent to
    FIRST/LAST_VALUE. For this case, 'value' is the value of
    the window function a priori taken from args[0], while arg_cache is used to
    remember the value from the previous row. NULLs need a bit of careful
    treatment.
  */
  Item_cache *value, *arg_cache;
  Arg_comparator *cmp;
  Item_result hybrid_type;
  bool was_values;  // Set if we have found at least one row (for max/min only)
  /**
    Set to true if the window is ordered ascending.
  */
  bool m_nulls_first;
  /**
    Set to true when min/max can be optimized using window's ordering.
  */
  bool m_optimize;
  /**
    For min() - Set to true when results are ordered in ascending and
    false when descending.
    For max() - Set to true when results are ordered in descending and
    false when ascending.
    Valid only when m_optimize is true.
  */
  bool m_want_first;  ///< Want first non-null value, else last non_null value
  /**
    Execution state: keeps track if this is the first row in the frame
    when buffering is not needed.
    Valid only when m_optimize is true.
  */
  int64 m_cnt;

  /**
    Execution state: keeps track of at which row we saved a non-null last
    value.
  */
  int64 m_saved_last_value_at;

  /**
    This function implements the optimized version of retrieving min/max
    value. When we have "ordered ASC" results in a window, min will always
    be the first value in the result set (neglecting the NULL's) and max
    will always be the last value (or the other way around, if ordered DESC).
    It is based on the implementation of FIRST_VALUE/LAST_VALUE, except for
    the NULL handling.

    @return true if computation yielded a NULL or error
  */
  bool compute();

  /**
    MIN/MAX function setup.

    Setup cache/comparator of MIN/MAX functions. When called by the
    copy_or_same() function, the value_arg parameter contains the calculated
    value of the original MIN/MAX object, and it is saved in this object's
    cache.

    @param item       the argument of the MIN/MAX function
    @param value_arg  the calculated value of the MIN/MAX function
    @return false on success, true on error
  */
  bool setup_hybrid(Item *item, Item *value_arg);

  /** Create a clone of this object. */
  virtual Item_sum_hybrid *clone_hybrid(THD *thd) const = 0;

 protected:
  Item_sum_hybrid(Item *item_par, bool is_min)
      : Item_sum(item_par),
        m_is_min(is_min),
        value(nullptr),
        arg_cache(nullptr),
        cmp(nullptr),
        hybrid_type(INT_RESULT),
        was_values(true),
        m_nulls_first(false),
        m_optimize(false),
        m_want_first(false),
        m_cnt(0),
        m_saved_last_value_at(0) {
    collation.set(&my_charset_bin);
  }

  Item_sum_hybrid(const POS &pos, Item *item_par, bool is_min, PT_window *w)
      : Item_sum(pos, item_par, w),
        m_is_min(is_min),
        value(nullptr),
        arg_cache(nullptr),
        cmp(nullptr),
        hybrid_type(INT_RESULT),
        was_values(true),
        m_nulls_first(false),
        m_optimize(false),
        m_want_first(false),
        m_cnt(0),
        m_saved_last_value_at(0) {
    collation.set(&my_charset_bin);
  }

  Item_sum_hybrid(THD *thd, const Item_sum_hybrid *item)
      : Item_sum(thd, item),
        m_is_min(item->m_is_min),
        value(item->value),
        arg_cache(nullptr),
        hybrid_type(item->hybrid_type),
        was_values(item->was_values),
        m_nulls_first(item->m_nulls_first),
        m_optimize(item->m_optimize),
        m_want_first(item->m_want_first),
        m_cnt(item->m_cnt),
        m_saved_last_value_at(0) {}

 public:
  bool fix_fields(THD *, Item **) override;
  void clear() override;
  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;
  double val_real() override;
  longlong val_int() override;
  longlong val_time_temporal() override;
  longlong val_date_temporal() override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  void reset_field() override;
  String *val_str(String *) override;
  bool val_json(Json_wrapper *wr) override;
  bool keep_field_type() const override { return true; }
  enum Item_result result_type() const override { return hybrid_type; }
  void update_field() override;
  void cleanup() override;
  bool any_value() { return was_values; }
  void no_rows_in_result() override;
  Field *create_tmp_field(bool group, TABLE *table) override;
  bool uses_only_one_row() const override { return m_optimize; }
  bool add() override;
  Item *copy_or_same(THD *thd) override;
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *r) override;

 private:
  /*
    These functions check if the value on the current row exceeds the maximum or
    minimum value seen so far, and update the current max/min stored in
    result_field, if needed.
  */
  void min_max_update_str_field();
  void min_max_update_temporal_field();
  void min_max_update_json_field();
  void min_max_update_real_field();
  void min_max_update_int_field();
  void min_max_update_decimal_field();
};

class Item_sum_min final : public Item_sum_hybrid {
 public:
  Item_sum_min(Item *item_par) : Item_sum_hybrid(item_par, true) {}
  Item_sum_min(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_hybrid(pos, item_par, true, w) {}
  Item_sum_min(THD *thd, const Item_sum_min *item)
      : Item_sum_hybrid(thd, item) {}
  enum Sumfunctype sum_func() const override { return MIN_FUNC; }
  const char *func_name() const override { return "min"; }

 private:
  Item_sum_min *clone_hybrid(THD *thd) const override;
};

class Item_sum_max final : public Item_sum_hybrid {
 public:
  Item_sum_max(Item *item_par) : Item_sum_hybrid(item_par, false) {}
  Item_sum_max(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_hybrid(pos, item_par, false, w) {}
  Item_sum_max(THD *thd, const Item_sum_max *item)
      : Item_sum_hybrid(thd, item) {}
  enum Sumfunctype sum_func() const override { return MAX_FUNC; }
  const char *func_name() const override { return "max"; }

 private:
  Item_sum_max *clone_hybrid(THD *thd) const override;
};

/**
  Base class used to implement BIT_AND, BIT_OR and BIT_XOR.

  Each of them is both a set function and a framing window function.
*/
class Item_sum_bit : public Item_sum {
  typedef Item_sum super;
  /// Stores the neutral element for function
  ulonglong reset_bits;
  /// Stores the result value for the INT_RESULT
  ulonglong bits;
  /// Stores the result value for the STRING_RESULT
  String value_buff;
  /// Stores the Item's result type. Can only be INT_RESULT or STRING_RESULT
  Item_result hybrid_type;
  /// Buffer used to avoid String allocation in the constructor
  const char initial_value_buff_storage[1] = {0};

  /**
    Execution state (windowing): this is for counting rows entering and leaving
    the window frame, see #m_frame_null_count.
   */
  ulonglong m_count;

  /**
    Execution state (windowing): this is for counting NULLs of rows entering
    and leaving the window frame, when we use optimized inverse-based
    computations. By comparison with m_count we can know how many non-NULLs are
    in the frame.
  */
  ulonglong m_frame_null_count;

  /**
    Execution state (windowing): Used for AND, OR to be able to invert window
    functions in optimized mode.

    For the optimized code path of BIT_XXX wfs, we keep track of the number of
    bit values (0's or 1's; see below) seen in a frame using a 64 bits counter
    pr bit. This lets us compute the value of OR by just inspecting:

       - the number of 1's in the previous frame
       - whether any removed row(s) is a 1
       - whether any added row(s) is a 1

    Similarly for AND, we keep track of the number of 0's seen for a particular
    bit. To do this trick we need a counter per bit position. This array holds
    these counters.

    Note that for XOR, the inverse operation is identical to the operation,
    so we don't need the above.
  */
  ulonglong *m_digit_cnt;
  /*
    Size of allocated array m_digit_cnt.
    The size is DIGIT_CNT_CARD (for integer types) or ::max_length * 8 for bit
    strings.
  */
  uint m_digit_cnt_card;

  static constexpr uint DIGIT_CNT_CARD = sizeof(ulonglong) * 8;

 protected:
  bool m_is_xor;  ///< true iff BIT_XOR

 public:
  Item_sum_bit(const POS &pos, Item *item_par, ulonglong reset_arg,
               PT_window *w)
      : Item_sum(pos, item_par, w),
        reset_bits(reset_arg),
        bits(reset_arg),
        value_buff(initial_value_buff_storage, 1, &my_charset_bin),
        m_count(0),
        m_frame_null_count(0),
        m_digit_cnt(nullptr),
        m_digit_cnt_card(0),
        m_is_xor(false) {}

  /// Copy constructor, used for executing subqueries with temporary tables
  Item_sum_bit(THD *thd, Item_sum_bit *item)
      : Item_sum(thd, item),
        reset_bits(item->reset_bits),
        bits(item->bits),
        value_buff(initial_value_buff_storage, 1, &my_charset_bin),
        hybrid_type(item->hybrid_type),
        m_count(item->m_count),
        m_frame_null_count(item->m_frame_null_count),
        m_digit_cnt(nullptr),
        m_digit_cnt_card(0),
        m_is_xor(item->m_is_xor) {
    /*
       This constructor should only be called during the Optimize stage.
       Asserting that the item was not evaluated yet.
    */
    DBUG_ASSERT(item->value_buff.length() == 1);
    DBUG_ASSERT(item->bits == item->reset_bits);
  }

  Item *result_item(Field *) override {
    return new Item_sum_bit_field(hybrid_type, this, reset_bits);
  }

  enum Sumfunctype sum_func() const override { return SUM_BIT_FUNC; }
  enum Item_result result_type() const override { return hybrid_type; }
  void clear() override;
  longlong val_int() override;
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *decimal_value) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  void reset_field() override;
  void update_field() override;
  bool resolve_type(THD *) override;
  bool fix_fields(THD *thd, Item **ref) override;
  void cleanup() override {
    bits = reset_bits;
    // At end of one execution of statement, free buffer to reclaim memory:
    value_buff.set(initial_value_buff_storage, 1, &my_charset_bin);
    Item_sum::cleanup();
  }

  /**
    Common implementation of Item_sum_or::add, Item_sum_and:add
    and Item_sum_xor::add.
  */
  bool add() override;
  /// @returns true iff this is BIT_AND.
  inline bool is_and() const { return reset_bits != 0; }

 private:
  /**
    Accumulate the value of 's1' (if in string mode) or of 'b1' (if in integer
    mode). Updates 'value_buff' or 'bits'.

    @param s1  argument to accumulate
    @param b1  argument to accumulate

    @returns true if error
  */
  bool add_bits(const String *s1, ulonglong b1);

  /**
    For windowing: perform inverse aggregation. "De-accumulate" the value of
    's1' (if in string mode) or of 'b1' (if in integer mode). Updates
    'value_buff' or 'bits'.

    For BIT_XOR we simply apply XOR as it's its inverse operation. For BIT_OR
    and BIT_AND we do the rest below.

    For each bit in argument, decrement the corresponding bits's counter
    ('m_digit_cnt') for that bit as follows: for BIT_AND, decrement the
    counter if we see a zero in that bit; for BIT_OR decrement the counter if
    we see a 1 in that bit.  Next, update 'value_buff' or 'bits' using the
    resulting counters: for each bit, for BIT_AND, set bit if we have counter
    == 0, i.e. we have no zero bits for that bit in the frame (yet).  For
    BIT_OR, set bit if we have counter > 0, i.e. at least one row in the frame
    has that bit set.

    @param  s1  the bits to be inverted from the aggregate value
    @param  b1  the bits to be inverted from the aggregate value
  */
  void remove_bits(const String *s1, ulonglong b1);
};

class Item_sum_or final : public Item_sum_bit {
 public:
  Item_sum_or(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_bit(pos, item_par, 0LL, w) {}

  Item_sum_or(THD *thd, Item_sum_or *item) : Item_sum_bit(thd, item) {}
  const char *func_name() const override { return "bit_or"; }
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_and final : public Item_sum_bit {
 public:
  Item_sum_and(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_bit(pos, item_par, ULLONG_MAX, w) {}

  Item_sum_and(THD *thd, Item_sum_and *item) : Item_sum_bit(thd, item) {}
  const char *func_name() const override { return "bit_and"; }
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_xor final : public Item_sum_bit {
 public:
  Item_sum_xor(const POS &pos, Item *item_par, PT_window *w)
      : Item_sum_bit(pos, item_par, 0LL, w) {
    m_is_xor = true;
  }

  Item_sum_xor(THD *thd, Item_sum_xor *item) : Item_sum_bit(thd, item) {}
  const char *func_name() const override { return "bit_xor"; }
  Item *copy_or_same(THD *thd) override;
};

/*
  User defined aggregates
*/

class Item_udf_sum : public Item_sum {
  typedef Item_sum super;

 protected:
  udf_handler udf;

 public:
  Item_udf_sum(const POS &pos, udf_func *udf_arg, PT_item_list *opt_list)
      : Item_sum(pos, opt_list, nullptr), udf(udf_arg) {
    allow_group_via_temp_table = false;
  }
  Item_udf_sum(THD *thd, Item_udf_sum *item)
      : Item_sum(thd, item), udf(item->udf) {
    udf.not_original = true;
  }

  bool itemize(Parse_context *pc, Item **res) override;
  const char *func_name() const override { return udf.name(); }
  bool fix_fields(THD *thd, Item **ref) override {
    DBUG_ASSERT(fixed == 0);

    if (init_sum_func_check(thd)) return true;

    fixed = true;
    if (udf.fix_fields(thd, this, this->arg_count, this->args)) return true;

    return check_sum_func(thd, ref);
  }
  enum Sumfunctype sum_func() const override { return UDF_SUM_FUNC; }

  void clear() override;
  bool add() override;
  void reset_field() override {}
  void update_field() override {}
  void cleanup() override;
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
};

class Item_sum_udf_float final : public Item_udf_sum {
 public:
  Item_sum_udf_float(const POS &pos, udf_func *udf_arg, PT_item_list *opt_list)
      : Item_udf_sum(pos, udf_arg, opt_list) {}
  Item_sum_udf_float(THD *thd, Item_sum_udf_float *item)
      : Item_udf_sum(thd, item) {}
  longlong val_int() override {
    DBUG_ASSERT(fixed == 1);
    return (longlong)rint(Item_sum_udf_float::val_real());
  }
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_real(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_real(ltime);
  }
  bool resolve_type(THD *) override {
    set_data_type(MYSQL_TYPE_DOUBLE);
    fix_num_length_and_dec();
    return false;
  }
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_udf_int final : public Item_udf_sum {
 public:
  Item_sum_udf_int(const POS &pos, udf_func *udf_arg, PT_item_list *opt_list)
      : Item_udf_sum(pos, udf_arg, opt_list) {}
  Item_sum_udf_int(THD *thd, Item_sum_udf_int *item)
      : Item_udf_sum(thd, item) {}
  longlong val_int() override;
  double val_real() override {
    DBUG_ASSERT(fixed == 1);
    return (double)Item_sum_udf_int::val_int();
  }
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_int(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override { return get_time_from_int(ltime); }
  enum Item_result result_type() const override { return INT_RESULT; }
  bool resolve_type(THD *) override {
    set_data_type_longlong();
    return false;
  }
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_udf_str final : public Item_udf_sum {
 public:
  Item_sum_udf_str(const POS &pos, udf_func *udf_arg, PT_item_list *opt_list)
      : Item_udf_sum(pos, udf_arg, opt_list) {}
  Item_sum_udf_str(THD *thd, Item_sum_udf_str *item)
      : Item_udf_sum(thd, item) {}
  String *val_str(String *) override;
  double val_real() override {
    int err_not_used;
    const char *end_not_used;
    String *res;
    res = val_str(&str_value);
    return res ? my_strntod(res->charset(), res->ptr(), res->length(),
                            &end_not_used, &err_not_used)
               : 0.0;
  }
  longlong val_int() override {
    int err_not_used;
    String *res;
    const CHARSET_INFO *cs;

    if (!(res = val_str(&str_value))) return 0; /* Null value */
    cs = res->charset();
    const char *end = res->ptr() + res->length();
    return cs->cset->strtoll10(cs, res->ptr(), &end, &err_not_used);
  }
  my_decimal *val_decimal(my_decimal *dec) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_string(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_string(ltime);
  }
  enum Item_result result_type() const override { return STRING_RESULT; }
  bool resolve_type(THD *) override;
  Item *copy_or_same(THD *thd) override;
};

class Item_sum_udf_decimal final : public Item_udf_sum {
 public:
  Item_sum_udf_decimal(const POS &pos, udf_func *udf_arg,
                       PT_item_list *opt_list)
      : Item_udf_sum(pos, udf_arg, opt_list) {}
  Item_sum_udf_decimal(THD *thd, Item_sum_udf_decimal *item)
      : Item_udf_sum(thd, item) {}
  String *val_str(String *) override;
  double val_real() override;
  longlong val_int() override;
  my_decimal *val_decimal(my_decimal *) override;
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_decimal(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_decimal(ltime);
  }
  enum Item_result result_type() const override { return DECIMAL_RESULT; }
  bool resolve_type(THD *) override {
    set_data_type(MYSQL_TYPE_NEWDECIMAL);
    fix_num_length_and_dec();
    return false;
  }
  Item *copy_or_same(THD *thd) override;
};

int group_concat_key_cmp_with_distinct(const void *arg, const void *key1,
                                       const void *key2);
int group_concat_key_cmp_with_order(const void *arg, const void *key1,
                                    const void *key2);
int dump_leaf_key(void *key_arg, element_count count MY_ATTRIBUTE((unused)),
                  void *item_arg);

class Item_func_group_concat final : public Item_sum {
  typedef Item_sum super;

  Temp_table_param *tmp_table_param;
  String result;
  String *separator;
  TREE tree_base;
  TREE *tree;

  /**
     If DISTINCT is used with this GROUP_CONCAT, this member is used to filter
     out duplicates.
     @see Item_func_group_concat::setup
     @see Item_func_group_concat::add
     @see Item_func_group_concat::clear
   */
  Unique *unique_filter;
  TABLE *table;
  Mem_root_array<ORDER> order_array;
  Name_resolution_context *context;
  /** The number of ORDER BY items. */
  uint arg_count_order;
  /** The number of selected items, aka the expr list. */
  uint arg_count_field;
  uint row_count;
  /** The maximum permitted result length in bytes as set for
      group_concat_max_len system variable */
  uint group_concat_max_len;
  bool distinct;
  bool warning_for_row;
  bool always_null;
  bool force_copy_fields;
  /** True if result has been written to output buffer. */
  bool m_result_finalized;
  /*
    Following is 0 normal object and pointer to original one for copy
    (to correctly free resources)
  */
  Item_func_group_concat *original;

  friend int group_concat_key_cmp_with_distinct(const void *arg,
                                                const void *key1,
                                                const void *key2);
  friend int group_concat_key_cmp_with_order(const void *arg, const void *key1,
                                             const void *key2);
  friend int dump_leaf_key(void *key_arg,
                           element_count count MY_ATTRIBUTE((unused)),
                           void *item_arg);

 public:
  Item_func_group_concat(const POS &pos, bool is_distinct,
                         PT_item_list *select_list,
                         PT_order_list *opt_order_list, String *separator,
                         PT_window *w);

  Item_func_group_concat(THD *thd, Item_func_group_concat *item);
  ~Item_func_group_concat() override;

  bool itemize(Parse_context *pc, Item **res) override;
  void cleanup() override;

  enum Sumfunctype sum_func() const override { return GROUP_CONCAT_FUNC; }
  const char *func_name() const override { return "group_concat"; }
  Item_result result_type() const override { return STRING_RESULT; }
  Field *make_string_field(TABLE *table_arg) const override;
  void clear() override;
  bool add() override;
  void reset_field() override { DBUG_ASSERT(0); }   // not used
  void update_field() override { DBUG_ASSERT(0); }  // not used
  bool fix_fields(THD *, Item **) override;
  bool setup(THD *thd) override;
  void make_unique() override;
  double val_real() override {
    String *res;
    res = val_str(&str_value);
    return res ? my_atof(res->c_ptr()) : 0.0;
  }
  longlong val_int() override {
    String *res;
    int error;
    if (!(res = val_str(&str_value))) return (longlong)0;
    const char *end_ptr = res->ptr() + res->length();
    return my_strtoll10(res->ptr(), &end_ptr, &error);
  }
  my_decimal *val_decimal(my_decimal *decimal_value) override {
    return val_decimal_from_string(decimal_value);
  }
  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_string(ltime, fuzzydate);
  }
  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_string(ltime);
  }
  String *val_str(String *str) override;
  Item *copy_or_same(THD *thd) override;
  void no_rows_in_result() override {}
  void print(const THD *thd, String *str,
             enum_query_type query_type) const override;
  bool change_context_processor(uchar *arg) override {
    context = reinterpret_cast<Item_ident::Change_context *>(arg)->m_context;
    return false;
  }

  bool check_wf_semantics(THD *thd MY_ATTRIBUTE((unused)),
                          SELECT_LEX *select MY_ATTRIBUTE((unused)),
                          Window_evaluation_requirements *reqs
                              MY_ATTRIBUTE((unused))) override {
    unsupported_as_wf();
    return true;
  }
};

/**
  Common parent class for window functions that always work on the entire
  partition, even if a frame is defined.

  The subclasses can be divided in two disjoint sub-categories:
     - one-pass
     - two-pass (requires partition cardinality to be evaluated)
  cf. method needs_card.
*/
class Item_non_framing_wf : public Item_sum {
  typedef Item_sum super;

 public:
  Item_non_framing_wf(const POS &pos, PT_window *w) : Item_sum(pos, w) {}
  Item_non_framing_wf(const POS &pos, Item *a, PT_window *w)
      : Item_sum(pos, a, w) {}
  Item_non_framing_wf(const POS &pos, PT_item_list *opt_list, PT_window *w)
      : Item_sum(pos, opt_list, w) {}
  Item_non_framing_wf(THD *thd, Item_non_framing_wf *i) : Item_sum(thd, i) {}

  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override {
    return get_date_from_numeric(ltime, fuzzydate);
  }

  bool get_time(MYSQL_TIME *ltime) override {
    return get_time_from_numeric(ltime);
  }

  void reset_field() override { DBUG_ASSERT(false); }
  void update_field() override { DBUG_ASSERT(false); }
  bool add() override {
    DBUG_ASSERT(false);
    return false;
  }

  bool fix_fields(THD *thd, Item **items) override;

  bool framing() const override { return false; }
};

/**
  ROW_NUMBER window function, cf. SQL 2003 Section 6.10 \<window function\>
*/
class Item_row_number : public Item_non_framing_wf {
  // Execution state variables
  ulonglong m_ctr;  ///< Increment for each row in partition

 public:
  Item_row_number(const POS &pos, PT_window *w)
      : Item_non_framing_wf(pos, w), m_ctr(0) {
    unsigned_flag = true;
  }

  const char *func_name() const override { return "row_number"; }
  enum Sumfunctype sum_func() const override { return ROW_NUMBER_FUNC; }

  bool resolve_type(THD *thd MY_ATTRIBUTE((unused))) override {
    set_data_type_longlong();
    return false;
  }

  longlong val_int() override;
  double val_real() override;
  my_decimal *val_decimal(my_decimal *buff) override;
  String *val_str(String *) override;

  void clear() override;

  Item_result result_type() const override { return INT_RESULT; }

  bool check_wf_semantics(THD *thd MY_ATTRIBUTE((unused)),
                          SELECT_LEX *select MY_ATTRIBUTE((unused)),
                          Window_evaluation_requirements *reqs
                              MY_ATTRIBUTE((unused))) override {
    return false;
  }
};

/**
  RANK or DENSE_RANK window function, cf. SQL 2003 Section 6.10 \<window
  function\>
*/
class Item_rank : public Item_non_framing_wf {
  typedef Item_non_framing_wf super;
  bool m_dense;  ///< If true, the object represents DENSE_RANK
  // Execution state variables
  ulonglong m_rank_ctr;    ///< Increment when window order columns change
  ulonglong m_duplicates;  ///< Needed to make RANK different from DENSE_RANK
  List<Cached_item> m_previous;  ///< Values of previous row's ORDER BY items
 public:
  Item_rank(const POS &pos, bool dense, PT_window *w)
      : Item_non_framing_wf(pos, w),
        m_dense(dense),
        m_rank_ctr(0),
        m_duplicates(0),
        m_previous()

  {
    unsigned_flag = true;
  }

  const char *func_name() const override {
    return m_dense ? "dense_rank" : "rank";
  }

  enum Sumfunctype sum_func() const override {
    return m_dense ? DENSE_RANK_FUNC : RANK_FUNC;
  }

  bool resolve_type(THD *thd MY_ATTRIBUTE((unused))) override {
    set_data_type_longlong();
    return false;
  }

  longlong val_int() override;
  double val_real() override;
  my_decimal *val_decimal(my_decimal *buff) override;
  String *val_str(String *) override;

  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  /**
    Clear state for a new partition
  */
  void clear() override;
  /**
    Cleanup after query, free up resources
  */
  void cleanup() override;
  Item_result result_type() const override { return INT_RESULT; }
};

/**
  CUME_DIST window function, cf. SQL 2003 Section 6.10 \<window function\>
*/
class Item_cume_dist : public Item_non_framing_wf {
  typedef Item_non_framing_wf super;

 public:
  Item_cume_dist(const POS &pos, PT_window *w) : Item_non_framing_wf(pos, w) {}

  const char *func_name() const override { return "cume_dist"; }
  enum Sumfunctype sum_func() const override { return CUME_DIST_FUNC; }

  bool resolve_type(THD *thd MY_ATTRIBUTE((unused))) override {
    set_data_type_double();
    return false;
  }

  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;

  bool needs_card() const override { return true; }
  void clear() override {}
  longlong val_int() override;
  double val_real() override;
  String *val_str(String *) override;
  my_decimal *val_decimal(my_decimal *buffer) override;
  Item_result result_type() const override { return REAL_RESULT; }
};

/**
  PERCENT_RANK window function, cf. SQL 2003 Section 6.10 \<window function\>
*/
class Item_percent_rank : public Item_non_framing_wf {
  typedef Item_non_framing_wf super;
  // Execution state variables
  ulonglong m_rank_ctr;  ///< Increment when window order columns change
  ulonglong m_peers;     ///< Needed to make PERCENT_RANK same for peers
  /**
    Set when the last peer has been visited. Needed to increment m_rank_ctr.
  */
  bool m_last_peer_visited;

 public:
  Item_percent_rank(const POS &pos, PT_window *w)
      : Item_non_framing_wf(pos, w),
        m_rank_ctr(0),
        m_peers(0),
        m_last_peer_visited(false) {}

  const char *func_name() const override { return "percent_rank"; }
  enum Sumfunctype sum_func() const override { return PERCENT_RANK_FUNC; }

  bool resolve_type(THD *thd MY_ATTRIBUTE((unused))) override {
    set_data_type_double();
    return false;
  }

  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  bool needs_card() const override { return true; }

  void clear() override;
  void cleanup() override;
  longlong val_int() override;
  double val_real() override;
  String *val_str(String *) override;
  my_decimal *val_decimal(my_decimal *buffer) override;
  Item_result result_type() const override { return REAL_RESULT; }
};

/**
  NTILE window function, cf. SQL 2011 Section 6.10 \<window function\>
*/
class Item_ntile : public Item_non_framing_wf {
  typedef Item_non_framing_wf super;
  longlong m_value;

 public:
  Item_ntile(const POS &pos, Item *a, PT_window *w)
      : Item_non_framing_wf(pos, a, w), m_value(0) {
    unsigned_flag = true;
  }

  const char *func_name() const override { return "ntile"; }
  enum Sumfunctype sum_func() const override { return NTILE_FUNC; }

  bool resolve_type(THD *thd MY_ATTRIBUTE((unused))) override {
    set_data_type_longlong();
    return false;
  }

  bool fix_fields(THD *thd, Item **items) override;

  longlong val_int() override;
  double val_real() override;
  my_decimal *val_decimal(my_decimal *buff) override;
  String *val_str(String *) override;

  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  Item_result result_type() const override { return INT_RESULT; }
  void clear() override {}
  bool needs_card() const override { return true; }
};

/**
  LEAD/LAG window functions, cf. SQL 2011 Section 6.10 \<window function\>
*/
class Item_lead_lag : public Item_non_framing_wf {
  enum_null_treatment m_null_treatment;
  bool m_is_lead;  ///< if true, the function is LEAD, else LAG
  int64 m_n;       ///< canonicalized offset value
  Item_result m_hybrid_type;
  Item_cache *m_value;
  Item_cache *m_default;
  /**
    Execution state: if set, we already have a value for current row.
    State is used to avoid interference with other LEAD/LAG functions on
    the same window, since they share the same eval loop and they should
    trigger evaluation only when they are on the "right" row relative to
    current row. For other offsets, return NULL if we don't know the value
    for this function yet, or if we do (m_has_value==true), return the
    found value.
  */
  bool m_has_value;
  bool m_use_default;  ///< execution state: use default value for current row
  typedef Item_non_framing_wf super;

 public:
  Item_lead_lag(const POS &pos, bool lead,
                PT_item_list *opt_list,  // [0] expr, [1] offset, [2] default
                enum_null_treatment null_treatment, PT_window *w)
      : Item_non_framing_wf(pos, opt_list, w),
        m_null_treatment(null_treatment),
        m_is_lead(lead),
        m_n(0),
        m_hybrid_type(INVALID_RESULT),
        m_value(nullptr),
        m_default(nullptr),
        m_has_value(false),
        m_use_default(false) {}

  const char *func_name() const override {
    return (m_is_lead ? "lead" : "lag");
  }
  enum Sumfunctype sum_func() const override { return LEAD_LAG_FUNC; }

  bool resolve_type(THD *thd) override;
  bool fix_fields(THD *thd, Item **items) override;
  void clear() override;
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  enum Item_result result_type() const override { return m_hybrid_type; }

  longlong val_int() override;
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *decimal_buffer) override;

  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  bool val_json(Json_wrapper *wr) override;

  bool needs_card() const override {
    /*
      A possible optimization here: if LAG, we are only interested in rows we
      have already seen, so we might compute the result without reading the
      entire partition as soon as we have the current row.  Similarly, a small
      LEAD value might avoid reading the entire partition also, giving shorter
      time to first result. For now, we read the entirely partition for these
      window functions - for simplicity.
    */
    return true;
  }

  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;

  void set_has_value(bool value) { m_has_value = value; }
  bool has_value() const { return m_has_value; }

  void set_use_default(bool value) { m_use_default = value; }
  bool use_default() const { return m_use_default; }

 private:
  bool setup_lead_lag();
  /**
    Core logic of LEAD/LAG window functions

    @return true if computation yielded a NULL or error
  */
  bool compute();
};

/**
  FIRST_VALUE/LAST_VALUE window functions, cf. SQL 2011 Section 6.10 \<window
  function\>
*/
class Item_first_last_value : public Item_sum {
  bool m_is_first;  ///< if true, the function is FIRST_VALUE, else LAST_VALUE
  enum_null_treatment m_null_treatment;
  Item_result m_hybrid_type;
  Item_cache *m_value;
  int64 cnt;  ///< used when evaluating on-the-fly (non-buffered processing)
  typedef Item_sum super;

 public:
  Item_first_last_value(const POS &pos, bool first, Item *a,
                        enum_null_treatment null_treatment, PT_window *w)
      : Item_sum(pos, a, w),
        m_is_first(first),
        m_null_treatment(null_treatment),
        m_hybrid_type(INVALID_RESULT),
        m_value(nullptr),
        cnt(0) {}

  const char *func_name() const override {
    return m_is_first ? "first_value" : "last_value";
  }

  enum Sumfunctype sum_func() const override { return FIRST_LAST_VALUE_FUNC; }

  bool resolve_type(THD *thd) override;
  bool fix_fields(THD *thd, Item **items) override;
  void clear() override;
  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;
  enum Item_result result_type() const override { return m_hybrid_type; }

  longlong val_int() override;
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *decimal_buffer) override;

  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  bool val_json(Json_wrapper *wr) override;

  void reset_field() override { DBUG_ASSERT(false); }
  void update_field() override { DBUG_ASSERT(false); }
  bool add() override {
    DBUG_ASSERT(false);
    return false;
  }

  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;
  bool uses_only_one_row() const override { return true; }

 private:
  bool setup_first_last();
  /**
    Core logic of FIRST/LAST_VALUE window functions

    @return true if computation yielded a NULL or error
  */
  bool compute();
};

/**
  NTH_VALUE window function, cf. SQL 2011 Section 6.10 \<window
  function\>
*/
class Item_nth_value : public Item_sum {
  enum_null_treatment m_null_treatment;
  int64 m_n;         ///< The N of the function
  bool m_from_last;  ///< true iff FROM_LAST was specified
  Item_result m_hybrid_type;
  enum_field_types m_hybrid_field_type;
  Item_cache *m_value;
  int64 m_cnt;  ///< used when evaluating on-the-fly (non-buffered processing)

  typedef Item_sum super;

 public:
  Item_nth_value(const POS &pos, PT_item_list *a, bool from_last,
                 enum_null_treatment null_treatment, PT_window *w)
      : Item_sum(pos, a, w),
        m_null_treatment(null_treatment),
        m_n(0),
        m_from_last(from_last),
        m_hybrid_type(INVALID_RESULT),
        m_value(nullptr),
        m_cnt(0) {}

  const char *func_name() const override { return "nth_value"; }
  enum Sumfunctype sum_func() const override { return NTH_VALUE_FUNC; }

  bool resolve_type(THD *thd) override;
  bool fix_fields(THD *thd, Item **items) override;
  bool setup_nth();
  void clear() override;

  bool check_wf_semantics(THD *thd, SELECT_LEX *select,
                          Window_evaluation_requirements *reqs) override;

  enum Item_result result_type() const override { return m_hybrid_type; }

  longlong val_int() override;
  double val_real() override;
  String *val_str(String *str) override;
  my_decimal *val_decimal(my_decimal *decimal_buffer) override;

  bool get_date(MYSQL_TIME *ltime, my_time_flags_t fuzzydate) override;
  bool get_time(MYSQL_TIME *ltime) override;
  bool val_json(Json_wrapper *wr) override;

  void reset_field() override { DBUG_ASSERT(false); }
  void update_field() override { DBUG_ASSERT(false); }
  bool add() override {
    DBUG_ASSERT(false);
    return false;
  }

  void split_sum_func(THD *thd, Ref_item_array ref_item_array,
                      List<Item> &fields) override;
  bool uses_only_one_row() const override { return true; }

 private:
  /**
    Core logic of NTH_VALUE window functions

    @return true if computation yielded a NULL or error
  */
  bool compute();
};

/**
  Class for implementation of the GROUPING function. The GROUPING
  function distinguishes super-aggregate rows from regular grouped
  rows. GROUP BY extensions such as ROLLUP and CUBE produce
  super-aggregate rows where the set of all values is represented
  by null. Using the GROUPING function, you can distinguish a null
  representing the set of all values in a super-aggregate row from
  a NULL in a regular row.
*/
class Item_func_grouping : public Item_int_func {
 public:
  Item_func_grouping(const POS &pos, PT_item_list *a) : Item_int_func(pos, a) {
    set_grouping_func();
  }
  const char *func_name() const override { return "grouping"; }
  enum Functype functype() const override { return GROUPING_FUNC; }
  longlong val_int() override;
  bool aggregate_check_group(uchar *arg) override;
  bool fix_fields(THD *thd, Item **ref) override;
  void update_used_tables() override;
};

#endif /* ITEM_SUM_INCLUDED */
