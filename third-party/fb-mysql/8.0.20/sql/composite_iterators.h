#ifndef SQL_COMPOSITE_ITERATORS_INCLUDED
#define SQL_COMPOSITE_ITERATORS_INCLUDED

/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file composite_iterators.h

  A composite row iterator is one that takes in one or more existing iterators
  and processes their rows in some interesting way. They are usually not bound
  to a single table or similar, but are the inner (non-leaf) nodes of the
  iterator execution tree. They consistently own their source iterator, although
  not its memory (since we never allocate row iterators on the heap--usually on
  a MEM_ROOT>). This means that in the end, you'll end up with a single root
  iterator which then owns everything else recursively.

  SortingIterator is also a composite iterator, but is defined in its own file.
 */

#include <stdio.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_table_map.h"
#include "prealloced_array.h"
#include "sql/item.h"
#include "sql/row_iterator.h"
#include "sql/table.h"

class FollowTailIterator;
template <class T>
class List;
class JOIN;
class SELECT_LEX;
class SJ_TMP_TABLE;
class THD;
class Temp_table_param;
class Window;

/**
  An iterator that takes in a stream of rows and passes through only those that
  meet some criteria (i.e., a condition evaluates to true). This is typically
  used for WHERE/HAVING.
 */
class FilterIterator final : public RowIterator {
 public:
  FilterIterator(THD *thd, unique_ptr_destroy_only<RowIterator> source,
                 Item *condition)
      : RowIterator(thd), m_source(move(source)), m_condition(condition) {}

  bool Init() override { return m_source->Init(); }

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override { m_source->UnlockRow(); }

  std::vector<Child> children() const override;

  std::vector<std::string> DebugString() const override {
    return {"Filter: " + ItemToString(m_condition)};
  }

 private:
  unique_ptr_destroy_only<RowIterator> m_source;
  Item *m_condition;
};

/**
  Handles LIMIT and/or OFFSET; Init() eats the first "offset" rows, and Read()
  stops as soon as it's seen "limit" rows (including any skipped by offset).
 */
class LimitOffsetIterator final : public RowIterator {
 public:
  /**
    @param thd Thread context
    @param source Row source
    @param limit Maximum number of rows to read, including the ones skipped by
      offset. Can be HA_POS_ERROR for no limit.
    @param offset Number of initial rows to skip. Can be 0 for no offset.
    @param count_all_rows If true, the query will run to completion to get
      more accurate numbers for skipped_rows, so you will not get any
      performance benefits of early end.
    @param skipped_rows If not nullptr, is incremented for each row skipped by
      offset or limit.
   */
  LimitOffsetIterator(THD *thd, unique_ptr_destroy_only<RowIterator> source,
                      ha_rows limit, ha_rows offset, bool count_all_rows,
                      ha_rows *skipped_rows)
      : RowIterator(thd),
        m_source(move(source)),
        m_limit(limit),
        m_offset(offset),
        m_count_all_rows(count_all_rows),
        m_skipped_rows(skipped_rows) {
    if (count_all_rows) {
      DBUG_ASSERT(m_skipped_rows != nullptr);
    }
  }

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override { m_source->UnlockRow(); }

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

  std::vector<std::string> DebugString() const override {
    char buf[256];
    if (m_offset == 0) {
      snprintf(buf, sizeof(buf), "Limit: %llu row(s)", m_limit);
    } else if (m_limit == HA_POS_ERROR) {
      snprintf(buf, sizeof(buf), "Offset: %llu row(s)", m_offset);
    } else {
      snprintf(buf, sizeof(buf), "Limit/Offset: %llu/%llu row(s)",
               m_limit - m_offset, m_offset);
    }
    if (m_count_all_rows) {
      return {std::string(buf) + " (no early end due to SQL_CALC_FOUND_ROWS)"};
    } else {
      return {std::string(buf)};
    }
  }

 private:
  unique_ptr_destroy_only<RowIterator> m_source;

  // Note: The number of seen rows starts off at m_limit if we have OFFSET,
  // which means we don't need separate LIMIT and OFFSET tests on the
  // fast path of Read().
  ha_rows m_seen_rows;

  /**
     Whether we have OFFSET rows that we still need to skip.
   */
  bool m_needs_offset;

  const ha_rows m_limit, m_offset;
  const bool m_count_all_rows;
  ha_rows *m_skipped_rows;
};

/**
  Handles aggregation (typically used for GROUP BY) for the case where the rows
  are already properly grouped coming in, ie., all rows that are supposed to be
  part of the same group are adjacent in the input stream. (This could be
  because they were sorted earlier, because we are scanning an index that
  already gives us the rows in a group-compatible order, or because there is no
  grouping.)

  AggregateIterator is special in that it's one of the very few row iterators
  that actually change the shape of the rows; some columns are dropped as part
  of aggregation, others (the aggregates) are added. For this reason (and also
  because we need to make copies of the group expressions -- see Read()), it
  conceptually always outputs to a temporary table. If we _are_ outputting to a
  temporary table, that's not a problem -- we take over responsibility for
  copying the group expressions from MaterializeIterator, which would otherwise
  do it.

  However, if we are outputting directly to the user, we need somewhere to store
  the output. This is solved by abusing the slice system; since we only need to
  buffer a single row, we can set up just enough items in the
  REF_SLICE_ORDERED_GROUP_BY slice, so that it can hold a single row. This row
  is then used for our output, and we then switch to it just before the end of
  Read() so that anyone reading from the buffers will get that output.
  The caller knows the context about where our output goes, and thus also picks
  the appropriate output slice for us.

  This isn't very pretty. What should be done is probably a more abstract
  concept of sending a row around and taking copies of it if needed, as opposed
  to it implicitly staying in the table's buffer. (This would also solve some
  issues in EQRefIterator and when synthesizing NULL rows for outer joins.)
  However, that's a large refactoring.
 */
class AggregateIterator final : public RowIterator {
 public:
  AggregateIterator(THD *thd, unique_ptr_destroy_only<RowIterator> source,
                    JOIN *join, Temp_table_param *temp_table_param,
                    int output_slice, bool rollup);

  bool Init() override;
  int Read() override;
  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override {
    // Most likely, HAVING failed. Ideally, we'd like to backtrack and
    // unlock all rows that went into this aggregate, but we can't do that,
    // and we also can't unlock the _current_ row, since that belongs to a
    // different group. Thus, do nothing.
  }

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

  std::vector<std::string> DebugString() const override;

 private:
  enum {
    READING_FIRST_ROW,
    LAST_ROW_STARTED_NEW_GROUP,
    READING_ROWS,
    OUTPUTTING_ROLLUP_ROWS,
    DONE_OUTPUTTING_ROWS
  } m_state;

  unique_ptr_destroy_only<RowIterator> m_source;

  /**
    The join we are part of. It would be nicer not to rely on this,
    but we need a large number of members from there, like which
    aggregate functions we have, the THD, temporary table parameters
    and so on.
   */
  JOIN *m_join = nullptr;

  /// The slice of the fields we are reading from (see the class comment).
  int m_input_slice;

  /// The slice of the fields we are outputting to. See the class comment.
  int m_output_slice;

  /// Whether we have seen the last input row.
  bool m_seen_eof;

  /**
    Used to save NULL information in the specific case where we have
    zero input rows.
   */
  table_map m_save_nullinfo;

  /// The parameters for the temporary table we are materializing into, if any.
  Temp_table_param *m_temp_table_param;

  /// Whether this is a rollup query.
  const bool m_rollup;

  /**
    Whether we have a rollup query where we needed to replace m_join->fields
    with a set of Item_refs. See the constructor for more information.
   */
  bool m_replace_field_list;

  /// If we have replaced the field list, contains the original field list.
  List<Item> *m_original_fields = nullptr;

  /**
    If we have replaced the field list, this is the list of Item pointers
    that each Item_ref points into.
   */
  Mem_root_array<Item *> *m_current_fields = nullptr;

  /**
    The list that the current values in m_current_fields come from.
    This is used purely as an optimization so that SwitchFieldList()
    does not have to do copying work if m_current_fields is already set up
    correctly. Only used if m_replace_field_list is true.
   */
  const List<Item> *m_current_fields_source = nullptr;

  /**
    For rollup: The index of the first group item that did _not_ change when we
    last switched groups. E.g., if we have group fields A,B,C,D and then switch
    to group A,B,D,D, this value will become 1 (which means that we need
    to output rollup rows for 2 -- A,B,D,NULL -- and then 1 -- A,B,NULL,NULL).
    m_current_rollup_position will count down from the end until it becomes
    less than this value.

    In addition, it is important to know this value so that we known
    which aggregates to reset once we start reading rows again; e.g.,
    in the given example, the two first aggregates should keep counting,
    while the two last ones should be reset. join->sum_funcs_end contains
    the right end pointers for this purpose.

    If we do not have rollup, this value is perennially zero, because there
    is only one element in join->sum_funcs_end (representing all aggregates
    in the query).
   */
  int m_last_unchanged_group_item_idx;

  /**
    If we are in state OUTPUTTING_ROLLUP_ROWS, where we are in the iteration.
    This value will start at the index of the last group expression and then
    count backwards down to and including m_last_unchanged_group_item_idx.
    It is used to know which field list we should send.
   */
  int m_current_rollup_position;

  void SwitchFieldList(List<Item> *fields) {
    if (!m_replace_field_list || m_current_fields_source == fields) {
      return;
    }

    size_t item_index = 0;
    for (Item &item : *fields) {
      (*m_current_fields)[item_index++] = &item;
    }
    m_current_fields_source = fields;
  }
};

/**
  Similar to AggregateIterator, but asusmes that the actual aggregates are
  already have been filled out (typically by QUICK_RANGE_MIN_MAX), and all the
  iterator needs to do is copy over the non-aggregated fields.
 */
class PrecomputedAggregateIterator final : public RowIterator {
 public:
  PrecomputedAggregateIterator(THD *thd,
                               unique_ptr_destroy_only<RowIterator> source,
                               JOIN *join, Temp_table_param *temp_table_param,
                               int output_slice)
      : RowIterator(thd),
        m_source(move(source)),
        m_join(join),
        m_temp_table_param(temp_table_param),
        m_output_slice(output_slice) {}

  bool Init() override;
  int Read() override;
  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override {
    // See AggregateIterator::UnlockRow().
  }

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

  std::vector<std::string> DebugString() const override;

 private:
  unique_ptr_destroy_only<RowIterator> m_source;

  /**
    The join we are part of. It would be nicer not to rely on this,
    but we need a large number of members from there, like which
    aggregate functions we have, the THD, temporary table parameters
    and so on.
   */
  JOIN *m_join = nullptr;

  /// The parameters for the temporary table we are materializing into, if any.
  Temp_table_param *m_temp_table_param;

  /// The slice of the fields we are outputting to.
  int m_output_slice;
};

/**
  A simple nested loop join, taking in two iterators (left/outer and
  right/inner) and joining them together. This may, of course, scan the inner
  iterator many times. It is currently the only form of join we have.

  The iterator works as a state machine, where the state records whether we need
  to read a new outer row or not, and whether we've seen any rows from the inner
  iterator at all (if not, an outer join need to synthesize a new NULL row).

  The iterator takes care of activating performance schema batch mode on the
  right iterator if needed; this is typically only used if it is the innermost
  table in the entire join (where the gains from turning on batch mode is the
  largest, and the accuracy loss from turning it off are the least critical).
 */
class NestedLoopIterator final : public RowIterator {
 public:
  NestedLoopIterator(THD *thd,
                     unique_ptr_destroy_only<RowIterator> source_outer,
                     unique_ptr_destroy_only<RowIterator> source_inner,
                     JoinType join_type, bool pfs_batch_mode)
      : RowIterator(thd),
        m_source_outer(move(source_outer)),
        m_source_inner(move(source_inner)),
        m_join_type(join_type),
        m_pfs_batch_mode(pfs_batch_mode) {
    DBUG_ASSERT(m_source_outer != nullptr);
    DBUG_ASSERT(m_source_inner != nullptr);

    // Batch mode makes no sense for anti- or semijoins, since they should only
    // be reading one row.
    if (join_type == JoinType::ANTI || join_type == JoinType::SEMI) {
      DBUG_ASSERT(!pfs_batch_mode);
    }
  }

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    // TODO: write something here about why we can't do this lazily.
    m_source_outer->SetNullRowFlag(is_null_row);
    m_source_inner->SetNullRowFlag(is_null_row);
  }

  void EndPSIBatchModeIfStarted() override {
    m_source_outer->EndPSIBatchModeIfStarted();
    m_source_inner->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    // Since we don't know which condition that caused the row to be rejected,
    // we can't know whether we could also unlock the outer row
    // (it may still be used as parts of other joined rows).
    if (m_state == READING_FIRST_INNER_ROW || m_state == READING_INNER_ROWS) {
      m_source_inner->UnlockRow();
    }
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source_outer.get(), ""},
                              {m_source_inner.get(), ""}};
  }

 private:
  enum {
    NEEDS_OUTER_ROW,
    READING_FIRST_INNER_ROW,
    READING_INNER_ROWS,
    END_OF_ROWS
  } m_state;

  unique_ptr_destroy_only<RowIterator> const m_source_outer;
  unique_ptr_destroy_only<RowIterator> const m_source_inner;
  const JoinType m_join_type;

  /** Whether to use batch mode when scanning the inner iterator. */
  const bool m_pfs_batch_mode;
};

/**
  An iterator that helps invalidating caches. Every time a row passes through it
  or it changes state in any other way, it increments its “generation” counter.
  This allows MaterializeIterator to see whether any of its dependencies has
  changed, and then force a rematerialization -- this is typically used for
  LATERAL tables, where we're joining in a derived table that depends on
  something earlier in the join.
 */
class CacheInvalidatorIterator final : public RowIterator {
 public:
  CacheInvalidatorIterator(THD *thd,
                           unique_ptr_destroy_only<RowIterator> source_iterator,
                           const std::string &name)
      : RowIterator(thd),
        m_source_iterator(move(source_iterator)),
        m_name(name) {}

  bool Init() override {
    ++m_generation;
    return m_source_iterator->Init();
  }

  int Read() override {
    ++m_generation;
    return m_source_iterator->Read();
  }

  void SetNullRowFlag(bool is_null_row) override {
    ++m_generation;
    m_source_iterator->SetNullRowFlag(is_null_row);
  }

  void UnlockRow() override { m_source_iterator->UnlockRow(); }
  std::vector<std::string> DebugString() const override;
  std::vector<Child> children() const override {
    return {Child{m_source_iterator.get(), ""}};
  }

  int64_t generation() const { return m_generation; }
  std::string name() const { return m_name; }

 private:
  unique_ptr_destroy_only<RowIterator> m_source_iterator;
  int64_t m_generation = 0;
  std::string m_name;
};

/**
  Handles materialization; the first call to Init() will scan the given iterator
  to the end, store the results in a temporary table (optionally with
  deduplication), and then Read() will allow you to read that table repeatedly
  without the cost of executing the given subquery many times (unless you ask
  for rematerialization).

  When materializing, MaterializeIterator takes care of evaluating any items
  that need so, and storing the results in the fields of the outgoing table --
  which items is governed by the temporary table parameters.

  Conceptually (although not performance-wise!), the MaterializeIterator is a
  no-op if you don't ask for deduplication, and in some cases (e.g. when
  scanning a table only once), we elide it. However, it's not necessarily
  straightforward to do so by just not inserting the iterator, as the optimizer
  will have set up everything (e.g., read sets, or what table upstream items
  will read from) assuming the materialization will happen, so the realistic
  option is setting up everything as if materialization would happen but not
  actually write to the table; see StreamingIterator for details.

  MaterializeIterator conceptually materializes iterators, not JOINs or
  SELECT_LEX_UNITs. However, there are many details that leak out
  (e.g., setting performance schema batch mode, slices, reusing CTEs,
  etc.), so we need to send them in anyway.
 */
class MaterializeIterator final : public TableRowIterator {
 public:
  struct QueryBlock {
    /// The iterator to read the actual rows from.
    unique_ptr_destroy_only<RowIterator> subquery_iterator;

    /// Used only for optimizer trace.
    int select_number;

    /// The JOIN that this query block represents. Used for performance
    /// schema batch mode: When materializing a query block that consists of
    /// a single table, MaterializeIterator needs to set up schema batch mode,
    /// since there is no nested loop iterator to do it. (This is similar to
    /// what ExecuteIteratorQuery() needs to do at the top level.)
    JOIN *join;

    /// If true, unique constraint checking via hash key is disabled
    /// when materializing this query block (ie., we simply avoid calling
    /// check_unique_constraint() for each row). Used when materializing
    /// UNION DISTINCT and UNION ALL parts into the same table.
    /// We'd like to just use a unique constraint via unique index instead,
    /// but there might be other indexes on the destination table
    /// that we'd like to keep, and the implementation doesn't allow
    /// disabling only one index.
    ///
    /// If you use this on a query block, doing_hash_deduplication()
    /// must be true.
    bool disable_deduplication_by_hash_field = false;

    /// If set to false, the Field objects in the output row are
    /// presumed already to be filled out. This is the case iff
    /// there's an AggregateIterator earlier in the chain.
    bool copy_fields_and_items;

    /// If copy_fields_and_items is true, used for copying the Field objects
    /// into the temporary table row. Otherwise unused.
    Temp_table_param *temp_table_param;

    // Whether this query block is a recursive reference back to the
    // output of the materialization.
    bool is_recursive_reference = false;

    // If is_recursive_reference is true, contains the FollowTailIterator
    // in the query block (there can be at most one recursive reference
    // in a join list, as per the SQL standard, so there should be exactly one).
    // Used for informing the iterators about various shared state in the
    // materialization (including coordinating rematerializations).
    FollowTailIterator *recursive_reader = nullptr;
  };

  /**
    @param thd Thread handler.
    @param query_blocks_to_materialize List of query blocks to materialize.
    @param table Handle to table to materialize into.
    @param table_iterator Iterator used for scanning the temporary table
      after materialization.
    @param cte If materializing a CTE, points to it (see m_cte), otherwise
      nullptr.
    @param unit The query expression we are materializing (see m_unit).
    @param join
      When materializing within the same JOIN (e.g., into a temporary table
      before sorting), as opposed to a derived table or a CTE, we may need
      to change the slice on the join before returning rows from the result
      table. If so, join and ref_slice would need to be set, and
      query_blocks_to_materialize should contain only one member, with the same
      join.
    @param ref_slice See join. If we are materializing across JOINs,
      e.g. derived tables, ref_slice should be left at -1.
    @param rematerialize true if rematerializing on every Init() call
      (e.g., because we have a dependency on a value from outside the query
      block).
    @param limit_rows
      Used for when pushing LIMIT down to MaterializeIterator; this is
      more efficient than having a LimitOffsetIterator above the
      MaterializeIterator, since we can stop materializing when there are
      enough rows. (This is especially important for recursive CTEs.)
      Note that we cannot have a LimitOffsetIterator _below_ the
      MaterializeIterator, as that would count wrong if we have deduplication,
      and would not work at all for recursive CTEs.
      Set to HA_POS_ERROR for no limit.
   */
  MaterializeIterator(THD *thd,
                      Mem_root_array<QueryBlock> query_blocks_to_materialize,
                      TABLE *table,
                      unique_ptr_destroy_only<RowIterator> table_iterator,
                      const Common_table_expr *cte, SELECT_LEX_UNIT *unit,
                      JOIN *join, int ref_slice, bool rematerialize,
                      ha_rows limit_rows);

  /**
    A convenience form for materializing a single table only.

    @param thd Thread handler.
    @param subquery_iterator The iterator to read the actual rows from.
    @param temp_table_param If copy_fields_and_items is true, used for copying
      the Field objects into the temporary table row. Otherwise unused.
    @param table Handle to table to materialize into.
    @param table_iterator Iterator used for scanning the temporary table
      after materialization.
    @param cte If materializing a CTE, points to it (see m_cte), otherwise
      nullptr.
    @param select_number Used only for optimizer trace.
    @param unit The query expression we are materializing (see m_unit).
    @param join
      When materializing within the same JOIN (e.g., into a temporary table
      before sorting), as opposed to a derived table or a CTE, we may need
      to change the slice on the join before returning rows from the result
      table. If so, join and ref_slice would need to be set, and
      query_blocks_to_materialize should contain only one member, with the same
      join.
    @param ref_slice See join. If we are materializing across JOINs,
      e.g. derived tables, ref_slice should be left at -1.
    @param copy_fields_and_items If set to false, the Field objects in the
      output row are presumed already to be filled out. This is the case iff
      there's an AggregateIterator earlier in the chain.
    @param rematerialize true if rematerializing on every Init() call
      (e.g., because we have a dependency on a value from outside the query
      block).
    @param limit_rows See limit_rows on the other constructor.
   */
  MaterializeIterator(THD *thd,
                      unique_ptr_destroy_only<RowIterator> subquery_iterator,
                      Temp_table_param *temp_table_param, TABLE *table,
                      unique_ptr_destroy_only<RowIterator> table_iterator,
                      const Common_table_expr *cte, int select_number,
                      SELECT_LEX_UNIT *unit, JOIN *join, int ref_slice,
                      bool copy_fields_and_items, bool rematerialize,
                      ha_rows limit_rows);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override;

  void SetNullRowFlag(bool is_null_row) override {
    m_table_iterator->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_table_iterator->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override;

  // The temporary table is private to us, so there's no need to worry about
  // locks to other transactions.
  void UnlockRow() override {}

  /**
    Add a cache invalidator that must be checked on every Init().
    If its generation has increased since last materialize, we need to
    rematerialize even if m_rematerialize is false.
   */
  void AddInvalidator(const CacheInvalidatorIterator *invalidator);

 private:
  Mem_root_array<QueryBlock> m_query_blocks_to_materialize;
  unique_ptr_destroy_only<RowIterator> m_table_iterator;

  /// If we are materializing a CTE, points to it (otherwise nullptr).
  /// Used so that we see if some other iterator already materialized the table,
  /// avoiding duplicate work.
  const Common_table_expr *m_cte;

  /// The query expression we are materializing. For derived tables,
  /// we materialize the entire query expression; for materialization within
  /// a query expression (e.g. for sorting or for windowing functions),
  /// we materialize only parts of it. Used to clear correlated CTEs within
  /// the unit when we rematerialize, since they depend on values from
  /// outside the query expression, and those values may have changed
  /// since last materialization.
  SELECT_LEX_UNIT *m_unit;

  /// See constructor.
  JOIN *const m_join;

  /// The slice to set when accessing temporary table; used if anything upstream
  /// (e.g. WHERE, HAVING) wants to evaluate values based on its contents.
  /// See constructor.
  const int m_ref_slice;

  /// If true, we need to materialize anew for each Init() (because the contents
  /// of the table will depend on some outer non-constant value).
  const bool m_rematerialize;

  /// See constructor.
  const ha_rows m_limit_rows;

  struct Invalidator {
    const CacheInvalidatorIterator *iterator;
    int64_t generation_at_last_materialize;
  };
  Mem_root_array<Invalidator> m_invalidators;

  /// Whether we are deduplicating using a hash field on the temporary
  /// table. (This condition mirrors check_unique_constraint().)
  /// If so, we compute a hash value for every row, look up all rows with
  /// the same hash and manually compare them to the row we are trying to
  /// insert.
  ///
  /// Note that this is _not_ the common way of deduplicating as we go.
  /// The common method is to have a regular index on the table
  /// over the right columns, and in that case, ha_write_row() will fail
  /// with an ignorable error, so that the row is ignored even though
  /// check_unique_constraint() is not called. However, B-tree indexes
  /// have limitations, in particular on length, that sometimes require us
  /// to do this instead. See create_tmp_table() for details.
  bool doing_hash_deduplication() const { return table()->hash_field; }

  /// Whether we are deduplicating, whether through a hash field
  /// or a regular unique index.
  bool doing_deduplication() const;

  bool MaterializeRecursive();
  bool MaterializeQueryBlock(const QueryBlock &query_block,
                             ha_rows *stored_rows);
};

/**
  StreamingIterator is a minimal version of MaterializeIterator that does not
  actually materialize; instead, every Read() just forwards the call to the
  subquery iterator and does the required copying from one set of fields to
  another.

  It is used for when the optimizer would normally set up a materialization,
  but you don't actually need one, ie. you don't want to read the rows multiple
  times after writing them, and you don't want to access them by index (only
  a single table scan). If you don't need the copy functionality (ie., you
  have an AggregateIterator, which does this job already), you still need a
  StreamingIterator, to set the NULL row flag on the temporary table.
 */
class StreamingIterator final : public TableRowIterator {
 public:
  StreamingIterator(THD *thd,
                    unique_ptr_destroy_only<RowIterator> subquery_iterator,
                    Temp_table_param *temp_table_param, TABLE *table,
                    bool copy_fields_and_items);

  bool Init() override;

  int Read() override;

  std::vector<std::string> DebugString() const override {
    return {"Stream results"};
  }

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_subquery_iterator.get(), ""}};
  }

  void StartPSIBatchMode() override {
    m_subquery_iterator->StartPSIBatchMode();
  }
  void EndPSIBatchModeIfStarted() override {
    m_subquery_iterator->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override { m_subquery_iterator->UnlockRow(); }

 private:
  unique_ptr_destroy_only<RowIterator> m_subquery_iterator;
  Temp_table_param *m_temp_table_param;
  const bool m_copy_fields_and_items;
  ha_rows m_row_number;

  // Whether the iterator should generate and provide a row ID. Only true if the
  // iterator is part of weedout, where the iterator will create a fake row ID
  // to uniquely identify the rows it produces.
  bool m_provide_rowid{false};
};

/**
  Aggregates unsorted data into a temporary table, using update operations
  to keep running aggregates. After that, works as a MaterializeIterator
  in that it allows the temporary table to be scanned.
 */
class TemptableAggregateIterator final : public TableRowIterator {
 public:
  TemptableAggregateIterator(
      THD *thd, unique_ptr_destroy_only<RowIterator> subquery_iterator,
      Temp_table_param *temp_table_param, TABLE *table,
      unique_ptr_destroy_only<RowIterator> table_iterator,
      SELECT_LEX *select_lex, JOIN *join, int ref_slice);

  bool Init() override;
  int Read() override;
  void SetNullRowFlag(bool is_null_row) override {
    m_table_iterator->SetNullRowFlag(is_null_row);
  }
  void EndPSIBatchModeIfStarted() override {
    m_table_iterator->EndPSIBatchModeIfStarted();
    m_subquery_iterator->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override {}
  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override;

 private:
  /// The iterator we are reading rows from.
  unique_ptr_destroy_only<RowIterator> m_subquery_iterator;

  /// The iterator used to scan the resulting temporary table.
  unique_ptr_destroy_only<RowIterator> m_table_iterator;

  Temp_table_param *m_temp_table_param;
  SELECT_LEX *m_select_lex;
  JOIN *const m_join;
  const int m_ref_slice;

  // See MaterializeIterator::doing_hash_deduplication().
  bool using_hash_key() const { return table()->hash_field; }
};

/**
  An iterator that wraps a Table_function (e.g. JSON_TABLE) and allows you to
  iterate over the materialized temporary table. The table is materialized anew
  for every Init().

  TODO: Just wrapping it is probably not the optimal thing to do;
  Table_function is highly oriented around materialization, but never caches.
  Thus, perhaps we should rewrite Table_function to return a RowIterator
  instead of going through a temporary table.
 */
class MaterializedTableFunctionIterator final : public TableRowIterator {
 public:
  MaterializedTableFunctionIterator(
      THD *thd, Table_function *table_function, TABLE *table,
      unique_ptr_destroy_only<RowIterator> table_iterator);

  bool Init() override;
  int Read() override { return m_table_iterator->Read(); }
  std::vector<std::string> DebugString() const override {
    return {{"Materialize table function"}};
  }
  void SetNullRowFlag(bool is_null_row) override {
    m_table_iterator->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_table_iterator->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_table_iterator->EndPSIBatchModeIfStarted();
  }

  // The temporary table is private to us, so there's no need to worry about
  // locks to other transactions.
  void UnlockRow() override {}

 private:
  unique_ptr_destroy_only<RowIterator> m_table_iterator;

  Table_function *m_table_function;
};

/**
  Like semijoin materialization, weedout works on the basic idea that a semijoin
  is just like an inner join as we long as we can get rid of the duplicates
  somehow. (This is advantageous, because inner joins can be reordered, whereas
  semijoins generally can't.) However, unlike semijoin materialization, weedout
  removes duplicates after the join, not before it. Consider something like

    SELECT * FROM t1 WHERE a IN ( SELECT b FROM t2 );

  Semijoin materialization solves this by materializing t2, with deduplication,
  and then joining. Weedout joins t1 to t2 and then leaves only one output row
  per t1 row. The disadvantage is that this potentially needs to discard more
  rows; the (potential) advantage is that we deduplicate on t1 instead of t2.

  Weedout, unlike materialization, works in a streaming fashion; rows are output
  (or discarded) as they come in, with a temporary table used for recording the
  row IDs we've seen before. (We need to deduplicate on t1's row IDs, not its
  contents.) See SJ_TMP_TABLE for details about the table format.
 */
class WeedoutIterator final : public RowIterator {
 public:
  WeedoutIterator(THD *thd, unique_ptr_destroy_only<RowIterator> source,
                  SJ_TMP_TABLE *sj);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override { m_source->UnlockRow(); }

 private:
  unique_ptr_destroy_only<RowIterator> m_source;
  SJ_TMP_TABLE *m_sj;

  // The cached value of QEP_TAB::rowid_status for each of the tables in the
  // weedout. Index 0 corresponds to the first table in m_sj.
  // See QEP_TAB::rowid_status for why we need to cache this value.
  Prealloced_array<rowid_statuses, 4> m_rowid_status;
};

/**
  An iterator that removes consecutive rows that are the same according to
  a given index (or more accurately, its keypart), so-called “loose scan”
  (not to be confused with “loose index scan”, which is a QUICK_SELECT_I).
  This is similar in spirit to WeedoutIterator above (removing duplicates
  allows us to treat the semijoin as a normal join), but is much cheaper
  if the data is already ordered/grouped correctly, as the removal can
  happen before the join, and it does not need a temporary table.
 */
class RemoveDuplicatesIterator final : public RowIterator {
 public:
  RemoveDuplicatesIterator(THD *thd,
                           unique_ptr_destroy_only<RowIterator> source,
                           const TABLE *table, KEY *key, size_t key_len);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }
  void UnlockRow() override { m_source->UnlockRow(); }

 private:
  unique_ptr_destroy_only<RowIterator> m_source;
  const TABLE *m_table;
  KEY *m_key;
  uchar *m_key_buf;  // Owned by the THD's MEM_ROOT.
  const size_t m_key_len;
  bool m_first_row;
};

/**
  An iterator that is semantically equivalent to a semijoin NestedLoopIterator
  immediately followed by a RemoveDuplicatesIterator. It is used to implement
  the “loose scan” strategy in queries with multiple tables on the inside of a
  semijoin, like

    ... FROM t1 WHERE ... IN ( SELECT ... FROM t2 JOIN t3 ... )

  In this case, the query tree without this iterator would ostensibly look like

    -> Nested loop join
       -> Table scan on t1
       -> Remove duplicates on t2_idx
          -> Nested loop semijoin
             -> Index scan on t2 using t2_idx
             -> Filter (e.g. t3.a = t2.a)
                -> Table scan on t3

  (t3 will be marked as “first match” on t2 when implementing loose scan,
  thus the semijoin.)

  First note that we can't put the duplicate removal directly on t2 in this
  case, as the first t2 row doesn't necessarily match anything in t3, so it
  needs to be above. However, this is wasteful, because once we find a matching
  t2/t3 pair, we should stop scanning t3 until we have a new t2.

  NestedLoopSemiJoinWithDuplicateRemovalIterator solves the problem by doing
  exactly this; it gets a row from the outer side, gets exactly one row from the
  inner side, and then skips over rows from the outer side (_without_ scanning
  the inner side) until its keypart changes.
 */
class NestedLoopSemiJoinWithDuplicateRemovalIterator final
    : public RowIterator {
 public:
  NestedLoopSemiJoinWithDuplicateRemovalIterator(
      THD *thd, unique_ptr_destroy_only<RowIterator> source_outer,
      unique_ptr_destroy_only<RowIterator> source_inner, const TABLE *table,
      KEY *key, size_t key_len);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source_outer->SetNullRowFlag(is_null_row);
    m_source_inner->SetNullRowFlag(is_null_row);
  }

  void EndPSIBatchModeIfStarted() override {
    m_source_outer->EndPSIBatchModeIfStarted();
    m_source_inner->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    m_source_outer->UnlockRow();
    m_source_inner->UnlockRow();
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source_outer.get(), ""},
                              {m_source_inner.get(), ""}};
  }

 private:
  unique_ptr_destroy_only<RowIterator> const m_source_outer;
  unique_ptr_destroy_only<RowIterator> const m_source_inner;

  const TABLE *m_table_outer;
  KEY *m_key;
  uchar *m_key_buf;  // Owned by the THD's MEM_ROOT.
  const size_t m_key_len;
  bool m_deduplicate_against_previous_row;
};

/**
  WindowingIterator is similar to AggregateIterator, but deals with windowed
  aggregates (i.e., OVER expressions). It deals specifically with aggregates
  that don't need to buffer rows.

  WindowingIterator always outputs to a temporary table. Similarly to
  AggregateIterator, needs to do some of MaterializeIterator's work in
  copying fields and Items into the destination fields (see AggregateIterator
  for more information).
 */
class WindowingIterator final : public RowIterator {
 public:
  WindowingIterator(THD *thd, unique_ptr_destroy_only<RowIterator> source,
                    Temp_table_param *temp_table_param,  // Includes the window.
                    JOIN *join, int output_slice);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    // There's nothing we can do here.
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

 private:
  /// The iterator we are reading from.
  unique_ptr_destroy_only<RowIterator> const m_source;

  /// Parameters for the temporary table we are outputting to.
  Temp_table_param *m_temp_table_param;

  /// The window function itself.
  Window *m_window;

  /// The join we are a part of.
  JOIN *m_join;

  /// The slice we will be using when reading rows.
  int m_input_slice;

  /// The slice we will be using when outputting rows.
  int m_output_slice;
};

/**
  BufferingWindowingIterator is like WindowingIterator, but deals with window
  functions that need to buffer rows.
 */
class BufferingWindowingIterator final : public RowIterator {
 public:
  BufferingWindowingIterator(
      THD *thd, unique_ptr_destroy_only<RowIterator> source,
      Temp_table_param *temp_table_param,  // Includes the window.
      JOIN *join, int output_slice);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_source->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_source->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_source->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    // There's nothing we can do here.
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source.get(), ""}};
  }

 private:
  int ReadBufferedRow(bool new_partition_or_eof);

  /// The iterator we are reading from.
  unique_ptr_destroy_only<RowIterator> const m_source;

  /// Parameters for the temporary table we are outputting to.
  Temp_table_param *m_temp_table_param;

  /// The window function itself.
  Window *m_window;

  /// The join we are a part of.
  JOIN *m_join;

  /// The slice we will be using when reading rows.
  int m_input_slice;

  /// The slice we will be using when outputting rows.
  int m_output_slice;

  /// If true, we may have more buffered rows to process that need to be
  /// checked for before reading more rows from the source.
  bool m_possibly_buffered_rows;

  /// Whether the last input row started a new partition, and was tucked away
  /// to finalize the previous partition; if so, we need to bring it back
  /// for processing before we read more rows.
  bool m_last_input_row_started_new_partition;

  /// Whether we have seen the last input row.
  bool m_eof;
};

/**
  MaterializeInformationSchemaTableIterator makes sure a given I_S temporary
  table is materialized (filled out) before we try to scan it.
 */
class MaterializeInformationSchemaTableIterator final : public RowIterator {
 public:
  MaterializeInformationSchemaTableIterator(
      THD *thd, QEP_TAB *qep_tab,
      unique_ptr_destroy_only<RowIterator> table_iterator);

  bool Init() override;
  int Read() override { return m_table_iterator->Read(); }
  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    // We don't list the table iterator as an explicit child; we mark it in
    // our DebugString() instead. (Anything else would look confusingly much
    // like a join.)
    return {};
  }

  void SetNullRowFlag(bool is_null_row) override {
    m_table_iterator->SetNullRowFlag(is_null_row);
  }

  void StartPSIBatchMode() override { m_table_iterator->StartPSIBatchMode(); }
  void EndPSIBatchModeIfStarted() override {
    m_table_iterator->EndPSIBatchModeIfStarted();
  }

  // The temporary table is private to us, so there's no need to worry about
  // locks to other transactions.
  void UnlockRow() override {}

 private:
  /// The iterator that reads from the materialized table.
  unique_ptr_destroy_only<RowIterator> m_table_iterator;
  QEP_TAB *m_qep_tab;
};

/**
  Takes in two or more iterators and output rows from them sequentially
  (first all rows from the first one, the all from the second one, etc.).
  Used for implementing UNION ALL, typically together with StreamingIterator.
 */
class AppendIterator final : public RowIterator {
 public:
  AppendIterator(
      THD *thd,
      std::vector<unique_ptr_destroy_only<RowIterator>> &&sub_iterators);

  bool Init() override;
  int Read() override;

  std::vector<std::string> DebugString() const override { return {"Append"}; }
  std::vector<Child> children() const override;

  void StartPSIBatchMode() override;
  void EndPSIBatchModeIfStarted() override;

  void SetNullRowFlag(bool is_null_row) override;
  void UnlockRow() override;

 private:
  std::vector<unique_ptr_destroy_only<RowIterator>> m_sub_iterators;
  size_t m_current_iterator_index = 0;
  bool m_pfs_batch_mode_enabled = false;
};

#endif  // SQL_COMPOSITE_ITERATORS_INCLUDED
