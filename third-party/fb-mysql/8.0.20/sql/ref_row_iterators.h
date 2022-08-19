#ifndef SQL_REF_ROW_ITERATORS_H
#define SQL_REF_ROW_ITERATORS_H

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

#include <sys/types.h>
#include <memory>

#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_inttypes.h"
#include "sql/basic_row_iterators.h"
#include "sql/row_iterator.h"
#include "sql/sql_sort.h"

class QEP_TAB;
class THD;
struct TABLE;
struct TABLE_REF;

/**
  For each record on the left side of a join (given in Init()), returns one or
  more matching rows from the given table, i.e., WHERE column=\<ref\>.
 */
template <bool Reverse>
class RefIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  RefIterator(THD *thd, TABLE *table, TABLE_REF *ref, bool use_order,
              QEP_TAB *qep_tab, ha_rows *examined_rows)
      : TableRowIterator(thd, table),
        m_ref(ref),
        m_use_order(use_order),
        m_qep_tab(qep_tab),
        m_examined_rows(examined_rows) {}

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  TABLE_REF *const m_ref;
  const bool m_use_order;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
  bool m_first_record_since_init;
  ulonglong *ius_requested_rows;
};

/**
  Like RefIterator, but after it's returned all its rows, will also search for
  rows that match NULL, i.e., WHERE column=\<ref\> OR column IS NULL.
 */
class RefOrNullIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  RefOrNullIterator(THD *thd, TABLE *table, TABLE_REF *ref, bool use_order,
                    QEP_TAB *qep_tab, ha_rows *examined_rows);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  TABLE_REF *const m_ref;
  const bool m_use_order;
  bool m_reading_first_row;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;
};

/**
  Like RefIterator, but used in situations where we're guaranteed to have
  exactly zero or one rows for each reference (due to e.g. unique constraints).
  It adds extra buffering to reduce the number of calls to the storage engine in
  the case where many consecutive rows on the left side contain the same value.
 */
class EQRefIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  EQRefIterator(THD *thd, TABLE *table, TABLE_REF *ref, bool use_order,
                ha_rows *examined_rows);

  bool Init() override;
  int Read() override;
  void UnlockRow() override;
  std::vector<std::string> DebugString() const override;

  // Performance schema batch mode on EQRefIterator does not make any sense,
  // since it (by definition) can never scan more than one row. Normally,
  // we should not get this (for nested loop joins, PFS batch mode is not
  // enabled if the innermost iterator is an EQRefIterator); however,
  // we cannot DBUG_ASSERT(false), since it could happen if we only have
  // a single table. Thus, just ignore the call should it happen.
  void StartPSIBatchMode() override {}

 private:
  TABLE_REF *const m_ref;
  const bool m_use_order;
  bool m_first_record_since_init;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;
};

/**
  An iterator that reads from a table where only a single row is known to be
  matching, no matter what's on the left side, i.e., WHERE column=\<const\>.
 */
class ConstIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  ConstIterator(THD *thd, TABLE *table, TABLE_REF *table_ref,
                ha_rows *examined_rows);

  bool Init() override;
  int Read() override;

  /**
    Rows from const tables are read once but potentially used
    multiple times during execution of a query.
    Ensure such rows are never unlocked during query execution.
  */
  void UnlockRow() override {}

  std::vector<std::string> DebugString() const override;

 private:
  TABLE_REF *const m_ref;
  bool m_first_record_since_init;
  ha_rows *const m_examined_rows;
};

/** An iterator that does a search through a full-text index. */
class FullTextSearchIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  FullTextSearchIterator(THD *thd, TABLE *table, TABLE_REF *ref, bool use_order,
                         ha_rows *examined_rows);
  ~FullTextSearchIterator() override;

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  TABLE_REF *const m_ref;
  const bool m_use_order;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;
};

/*
  This is for QS_DYNAMIC_RANGE, i.e., "Range checked for each
  record". The trace for the range analysis below this point will
  be printed with different ranges for every record to the left of
  this table in the join; the range optimizer can either select any
  QUICK_SELECT_I (aka IndexRangeScanIterator) or a full table
  scan, and any Read() is just proxied over to that.

  Note in particular that this means the range optimizer will be
  executed anew on every single call to Init(), and modify the
  query plan accordingly! It is not clear whether this is an actual
  win in a typical query.
 */
class DynamicRangeIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  DynamicRangeIterator(THD *thd, TABLE *table, QEP_TAB *qep_tab,
                       ha_rows *examined_rows);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  QEP_TAB *m_qep_tab;

  unique_ptr_destroy_only<RowIterator> m_iterator;

  /**
    Used by optimizer tracing to decide whether or not dynamic range
    analysis of this select has been traced already. If optimizer
    trace option DYNAMIC_RANGE is enabled, range analysis will be
    traced with different ranges for every record to the left of this
    table in the join. If disabled, range analysis will only be traced
    for the first range.
  */
  bool m_quick_traced_before = false;

  ha_rows *const m_examined_rows;

  /**
    A read set we can use when we fall back to table scans,
    to get the base columns we need for virtual generated columns.
    See add_virtual_gcol_base_cols().
   */
  MY_BITMAP m_table_scan_read_set;

  /// The original value of table->read_set.
  MY_BITMAP *m_original_read_set;
};

/**
   Read a table *assumed* to be included in execution of a pushed join.
   This is the counterpart of RefIterator / EQRefIterator for child
   tables in a pushed join.

   When the table access is performed as part of the pushed join,
   all 'linked' child colums are prefetched together with the parent row.
   The handler will then only format the row as required by MySQL and set
   table status accordingly.

   However, there may be situations where the prepared pushed join was not
   executed as assumed. It is the responsibility of the handler to handle
   these situation by letting @c ha_index_read_pushed() then effectively do a
   plain old' index_read_map(..., HA_READ_KEY_EXACT);
 */
class PushedJoinRefIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  PushedJoinRefIterator(THD *thd, TABLE *table, TABLE_REF *ref, bool use_order,
                        ha_rows *examined_rows);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  TABLE_REF *const m_ref;
  const bool m_use_order;
  bool m_first_record_since_init;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;
};

/**
  An iterator that switches between another iterator (typically a RefIterator
  or similar) and a TableScanIterator.

  This is used when predicates have been pushed down into an IN subquery
  and then created ref accesses, but said predicates should not be checked for
  a NULL value (so we need to revert to table scans). See
  QEP_TAB::pick_table_access_method() for a more thorough explanation.
 */
class AlternativeIterator final : public RowIterator {
 public:
  // Takes ownership of "source", and is responsible for
  // calling Init() on it, but does not hold the memory.
  AlternativeIterator(THD *thd, TABLE *table, QEP_TAB *qep_tab,
                      ha_rows *examined_rows,
                      unique_ptr_destroy_only<RowIterator> source,
                      TABLE_REF *ref);

  bool Init() override;

  int Read() override { return m_iterator->Read(); }

  void SetNullRowFlag(bool is_null_row) override {
    // Init() may not have been called yet, so just forward to both iterators.
    m_source_iterator->SetNullRowFlag(is_null_row);
    m_table_scan_iterator->SetNullRowFlag(is_null_row);
  }

  void UnlockRow() override { m_iterator->UnlockRow(); }

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_source_iterator.get(), ""},
                              {m_table_scan_iterator.get(), ""}};
  }

  std::vector<std::string> DebugString() const override;

 private:
  // The reference value with condition guards that we are switching on.
  TABLE_REF *m_ref;

  // If any of these are false during Init(), we are having a NULL IN ( ... ),
  // and need to fall back to table scan. Extracted from m_ref.
  std::vector<bool *> m_applicable_cond_guards;

  // Points to either m_source_iterator or m_table_scan_iterator,
  // depending on the value of applicable_cond_guards. Set up during Init().
  RowIterator *m_iterator = nullptr;

  // Points to the last iterator that was Init()-ed. Used to reset the handler
  // when switching from one iterator to the other.
  RowIterator *m_last_iterator_inited = nullptr;

  // The iterator we are normally reading records from (a RefIterator or
  // similar).
  unique_ptr_destroy_only<RowIterator> m_source_iterator;

  // Our fallback iterator (possibly wrapped in a TimingIterator).
  unique_ptr_destroy_only<RowIterator> m_table_scan_iterator;

  // The underlying table.
  TABLE *const m_table;

  /**
    A read set we can use when we fall back to table scans,
    to get the base columns we need for virtual generated columns.
    See add_virtual_gcol_base_cols().
   */
  MY_BITMAP m_table_scan_read_set;

  /// The original value of table->read_set.
  MY_BITMAP *m_original_read_set;
};

#endif  // SQL_REF_ROW_ITERATORS_H
