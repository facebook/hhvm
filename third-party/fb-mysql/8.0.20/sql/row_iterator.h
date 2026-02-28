#ifndef SQL_ROW_ITERATOR_H_
#define SQL_ROW_ITERATOR_H_

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

#include <string>
#include <vector>

#include "my_dbug.h"

class Item;
class JOIN;
class THD;
struct TABLE;

/**
  A context for reading through a single table using a chosen access method:
  index read, scan, etc, use of cache, etc.. It is mostly meant as an interface,
  but also contains some private member functions that are useful for many
  implementations, such as error handling.

  A RowIterator is a simple iterator; you initialize it, and then read one
  record at a time until Read() returns EOF. A RowIterator can read from
  other Iterators if you want to, e.g., SortingIterator, which takes in records
  from another RowIterator and sorts them.

  The abstraction is not completely tight. In particular, it still leaves some
  specifics to TABLE, such as which columns to read (the read_set). This means
  it would probably be hard as-is to e.g. sort a join of two tables.

  Use by:
@code
  unique_ptr<RowIterator> iterator(new ...);
  if (iterator->Init())
    return true;
  while (iterator->Read() == 0) {
    ...
  }
@endcode
 */
class RowIterator {
 public:
  // NOTE: Iterators should typically be instantiated using NewIterator,
  // in sql/timing_iterator.h.
  RowIterator(THD *thd) : m_thd(thd) {}
  virtual ~RowIterator() {}

  /**
    Initialize or reinitialize the iterator. You must always call Init()
    before trying a Read() (but Init() does not imply Read()).

    You can call Init() multiple times; subsequent calls will rewind the
    iterator (or reposition it, depending on whether the iterator takes in
    e.g. a TABLE_REF) and allow you to read the records anew.
   */
  virtual bool Init() = 0;

  /**
    Read a single row. The row data is not actually returned from the function;
    it is put in the table's (or tables', in case of a join) record buffer, ie.,
    table->records[0].

    @retval
      0   OK
    @retval
      -1   End of records
    @retval
      1   Error
   */
  virtual int Read() = 0;

  /**
    Mark the current row buffer as containing a NULL row or not, so that if you
    read from it and the flag is true, you'll get only NULLs no matter what is
    actually in the buffer (typically some old leftover row). This is used
    for outer joins, when an iterator hasn't produced any rows and we need to
    produce a NULL-complemented row. Init() or Read() won't necessarily
    reset this flag, so if you ever set is to true, make sure to also set it
    to false when needed.

    Note that this can be called without Init() having been called first.
    For example, NestedLoopIterator can hit EOF immediately on the outer
    iterator, which means the inner iterator doesn't get an Init() call,
    but will still forward SetNullRowFlag to both inner and outer iterators.

    TODO: We shouldn't need this. See the comments on AggregateIterator for
    a bit more discussion on abstracting out a row interface.
   */
  virtual void SetNullRowFlag(bool is_null_row) = 0;

  // In certain queries, such as SELECT FOR UPDATE, UPDATE or DELETE queries,
  // reading rows will automatically take locks on them. (This means that the
  // set of locks taken will depend on whether e.g. the optimizer chose a table
  // scan or used an index, due to InnoDB's row locking scheme with “gap locks”
  // for B-trees instead of full predicate locks.)
  //
  // However, under some transaction isolation levels (READ COMMITTED or
  // less strict), it is possible to release such locks if and only if the row
  // failed a WHERE predicate, as only the returned rows are protected,
  // not _which_ rows are returned. Thus, if Read() returned a row that you did
  // not actually use, you should call UnlockRow() afterwards, which allows the
  // storage engine to release the row lock in such situations.
  //
  // TableRowIterator has a default implementation of this; other iterators
  // should usually either forward the call to their source iterator (if any)
  // or just ignore it. The right behavior depends on the iterator.
  virtual void UnlockRow() = 0;

  struct Child {
    RowIterator *iterator;

    // Normally blank. If not blank, a heading for this iterator
    // saying what kind of role it has to the parent if it is not
    // obvious. E.g., FilterIterator can print iterators that are
    // children because they come out of subselect conditions.
    std::string description;
  };

  /// List of zero or more iterators which are direct children of this one.
  /// By convention, if there are multiple ones (ie., we're doing a join),
  /// the outer iterator is listed first. So for a LEFT JOIN b, we'd list
  /// a before b.
  virtual std::vector<Child> children() const { return std::vector<Child>(); }

  /// Returns a short string (used for EXPLAIN FORMAT=tree) with user-readable
  /// information for this iterator. When implementing these, try to avoid
  /// internal jargon (e.g. “eq_ref”); prefer things that read like normal,
  /// technical English (e.g. “single-row index lookup”).
  ///
  /// For certain complex operations, such as MaterializeIterator, there can be
  /// multiple strings. If so, they are interpreted as nested operations,
  /// with the outermost, last-done operation first and the other ones indented
  /// as if they were child iterators.
  ///
  /// Callers should use FullDebugString() below, which adds costs
  /// (see set_estimated_cost() etc.) if present.
  virtual std::vector<std::string> DebugString() const = 0;

  virtual std::string TimingString() const {
    // Valid for TimingIterator only.
    DBUG_ASSERT(false);
    return "";
  }

  // If this is the root iterator of a join, points back to the join object.
  // This has one single purpose: EXPLAIN uses it to be able to get the SELECT
  // list and print out any subselects in it; they are not children of
  // the iterator per se, but need to be printed with it.
  //
  // We could have stored the list of these extra subselect iterators directly
  // on the iterator (it breaks the abstraction a bit to refer to JOIN here),
  // but setting a single pointer is cheaper, especially considering that most
  // queries are not EXPLAIN queries and we don't want the overhead for them.
  JOIN *join_for_explain() const { return m_join_for_explain; }

  // Should be called by JOIN::create_iterators() only.
  void set_join_for_explain(JOIN *join) { m_join_for_explain = join; }

  /**
    Start performance schema batch mode, if supported (otherwise ignored).

    PFS batch mode is a mitigation to reduce the overhead of performance schema,
    typically applied at the innermost table of the entire join. If you start
    it before scanning the table and then end it afterwards, the entire set
    of handler calls will be timed only once, as a group, and the costs will
    be distributed evenly out. This reduces timer overhead.

    If you start PFS batch mode, you must also take care to end it at the
    end of the scan, one way or the other. Do note that this is true even
    if the query ends abruptly (LIMIT is reached, or an error happens).
    The easiest workaround for this is to simply call EndPSIBatchModeIfStarted()
    on the root iterator at the end of the scan. See the PFSBatchMode class for
    a useful helper.

    The rules for starting batch and ending mode are:

      1. If you are an iterator with exactly one child (FilterIterator etc.),
         forward any StartPSIBatchMode() calls to it.
      2. If you drive an iterator (read rows from it using a for loop
         or similar), use PFSBatchMode as described above.
      3. If you have multiple children, ignore the call and do your own
         handling of batch mode as appropriate. For materialization,
         #2 would typically apply. For joins, it depends on the join type
         (e.g., NestedLoopIterator applies batch mode only when scanning
         the innermost table).

    The upshot of this is that when scanning a single table, batch mode
    will typically be activated for that table (since we call
    StartPSIBatchMode() on the root iterator, and it will trickle all the way
    down to the table iterator), but for a join, the call will be ignored
    and the join iterator will activate batch mode by itself as needed.
   */
  virtual void StartPSIBatchMode() {}

  /**
    Ends performance schema batch mode, if started. It's always safe to
    call this.

    Iterators that have children (composite iterators) must forward the
    EndPSIBatchModeIfStarted() call to every iterator they could conceivably
    have called StartPSIBatchMode() on. This ensures that after such a call
    to on the root iterator, all handlers are out of batch mode.
   */
  virtual void EndPSIBatchModeIfStarted() {}

  // The information below is used for EXPLAIN only. We store it on the
  // iterators, because it corresponds naturally 1:1 to the them.
  // However, RowIterator is an execution structure, and as such, estimated
  // costs don't really belong here. When we go to an optimizer that plans
  // natively using iterators, we should have a class setup where
  // each execution iterator has a corresponding planning structure
  // (e.g. TableScanIterator vs. PlannedTableScan), and the costs should move
  // to the planning structures.

  void set_estimated_cost(double estimated_cost) {
    m_estimated_cost = estimated_cost;
  }
  double estimated_cost() const { return m_estimated_cost; }

  void set_expected_rows(double expected_rows) {
    m_expected_rows = expected_rows;
  }
  double expected_rows() const { return m_expected_rows; }

  /**
    If this iterator is wrapping a different iterator (e.g. TimingIterator<T>)
    and you need to down_cast<> to a specific iterator type, this allows getting
    at the wrapped iterator.
   */
  virtual RowIterator *real_iterator() { return this; }
  virtual const RowIterator *real_iterator() const { return this; }

 protected:
  THD *thd() const { return m_thd; }

 private:
  THD *const m_thd;
  JOIN *m_join_for_explain = nullptr;
  double m_estimated_cost = -1.0;
  double m_expected_rows = -1.0;
};

class TableRowIterator : public RowIterator {
 public:
  TableRowIterator(THD *thd, TABLE *table) : RowIterator(thd), m_table(table) {}

  void UnlockRow() override;
  void SetNullRowFlag(bool is_null_row) override;
  void StartPSIBatchMode() override;
  void EndPSIBatchModeIfStarted() override;

 protected:
  int HandleError(int error);
  void PrintError(int error);
  TABLE *table() const { return m_table; }

 private:
  TABLE *const m_table;

  friend class AlternativeIterator;
};

// Return iterator.DebugString(), but with cost and timing information appended
// in textual form, if available.
std::vector<std::string> FullDebugString(const THD *thd,
                                         const RowIterator &iterator);

// Used to describe what kind of join an iterator is executing.
enum class JoinType { INNER, OUTER, ANTI, SEMI };

#endif  // SQL_ROW_ITERATOR_H_
