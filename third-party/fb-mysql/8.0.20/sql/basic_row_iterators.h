#ifndef SQL_BASIC_ROW_ITERATORS_H_
#define SQL_BASIC_ROW_ITERATORS_H_

/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
  Row iterators that scan a single table without reference to other tables
  or iterators.
 */

#include <sys/types.h>
#include <memory>

#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_inttypes.h"
#include "sql/mem_root_array.h"
#include "sql/row_iterator.h"
#include "sql/sql_list.h"

class Filesort_info;
class Item;
class QEP_TAB;
class QUICK_SELECT_I;
class Sort_result;
class THD;
class handler;
struct IO_CACHE;
struct TABLE;

/**
  Scan a table from beginning to end.

  This is the most basic access method of a table using rnd_init,
  ha_rnd_next and rnd_end. No indexes are used.
 */
class TableScanIterator final : public TableRowIterator {
 public:
  // Accepts nullptr for qep_tab; qep_tab is used only for setting up record
  // buffers.
  //
  // The pushed condition can be nullptr.
  //
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  TableScanIterator(THD *thd, TABLE *table, QEP_TAB *qep_tab,
                    ha_rows *examined_rows);
  ~TableScanIterator() override;

  bool Init() override;
  int Read() override;

  std::vector<std::string> DebugString() const override;

 private:
  uchar *const m_record;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
};

/** Perform a full index scan along an index. */
template <bool Reverse>
class IndexScanIterator final : public TableRowIterator {
 public:
  // use_order must be set to true if you actually need to get the records
  // back in index order. It can be set to false if you wish to scan
  // using the index (e.g. for an index-only scan of the entire table),
  // but do not actually care about the order. In particular, partitioned
  // tables can use this to deliver more efficient scans.
  //
  // Accepts nullptr for qep_tab; qep_tab is used only for setting up record
  // buffers.
  //
  // The pushed condition can be nullptr.
  //
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  IndexScanIterator(THD *thd, TABLE *table, int idx, bool use_order,
                    QEP_TAB *qep_tab, ha_rows *examined_rows);
  ~IndexScanIterator() override;

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  uchar *const m_record;
  const int m_idx;
  const bool m_use_order;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;
  bool m_first = true;
};

/**
  Scan a given range of the table (a “quick”), using an index.

  IndexRangeScanIterator uses one of the QUICK_SELECT classes in opt_range.cc
  to perform an index scan. There are loads of functionality hidden
  in these quick classes. It handles all index scans of various kinds.

  TODO: Convert the QUICK_SELECT framework to RowIterator, so that
  we do not need this adapter.
 */
class IndexRangeScanIterator final : public TableRowIterator {
 public:
  // Does _not_ take ownership of "quick" (but maybe it should).
  //
  // Accepts nullptr for qep_tab; qep_tab is used only for setting up record
  // buffers.
  //
  // The pushed condition can be nullptr.
  //
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  IndexRangeScanIterator(THD *thd, TABLE *table, QUICK_SELECT_I *quick,
                         QEP_TAB *qep_tab, ha_rows *examined_rows);

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  // NOTE: No destructor; quick_range will call ha_index_or_rnd_end() for us.
  QUICK_SELECT_I *const m_quick;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
  ulonglong *ius_requested_rows;

  // After m_quick has returned EOF, some of its members are destroyed, making
  // subsequent requests for new rows undefined. We flag EOF so that the
  // iterator does not request a new row.
  bool m_seen_eof{false};
};

// Readers relating to reading sorted data (from filesort).
//
// Filesort will produce references to the records sorted; these
// references can be stored in memory or in a temporary file.
//
// The temporary file is normally used when the references doesn't fit into
// a properly sized memory buffer. For most small queries the references
// are stored in the memory buffer.
//
// The temporary file is also used when performing an update where a key is
// modified.

/**
  Fetch the records from a memory buffer.

  This method is used when table->sort.addon_field is allocated.
  This is allocated for most SELECT queries not involving any BLOB's.
  In this case the records are fetched from a memory buffer.
 */
template <bool Packed_addon_fields>
class SortBufferIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  // The table is used solely for NULL row flags.
  SortBufferIterator(THD *thd, TABLE *table, Filesort_info *sort,
                     Sort_result *sort_result, ha_rows *examined_rows);
  ~SortBufferIterator() override;

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;
  void UnlockRow() override {}

 private:
  // NOTE: No m_record -- unpacks directly into each Field's field->ptr.
  Filesort_info *const m_sort;
  Sort_result *const m_sort_result;
  unsigned m_unpack_counter;
  ha_rows *const m_examined_rows;
};

/**
  Fetch the record IDs from a memory buffer, but the records themselves from
  the table on disk.

  Used when the above (comment on SortBufferIterator) is not true, UPDATE,
  DELETE and so forth and SELECT's involving large BLOBs. It is also used for
  the result of Unique, which returns row IDs in the same format as filesort.
  In this case the record data is fetched from the handler using the saved
  reference using the rnd_pos handler call.
 */
class SortBufferIndirectIterator final : public TableRowIterator {
 public:
  // Ownership here is suboptimal: Takes only partial ownership of
  // "sort_result", so it must be alive for as long as the RowIterator is.
  // However, it _does_ free the buffers within on destruction.
  //
  // The pushed condition can be nullptr.
  //
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  SortBufferIndirectIterator(THD *thd, TABLE *table, Sort_result *sort_result,
                             bool ignore_not_found_rows,
                             ha_rows *examined_rows);
  ~SortBufferIndirectIterator() override;
  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  Sort_result *const m_sort_result;
  const uint m_ref_length;
  ha_rows *const m_examined_rows;
  uchar *m_record = nullptr;
  uchar *m_cache_pos = nullptr, *m_cache_end = nullptr;
  bool m_ignore_not_found_rows;
};

/**
  Fetch the records from a tempoary file.

  There used to be a comment here saying “should obviously not really happen
  other than in strange configurations”, but especially with packed addons
  and InnoDB (where fetching rows needs a primary key lookup), it's not
  necessarily suboptimal compared to e.g. SortBufferIndirectIterator.
 */
template <bool Packed_addon_fields>
class SortFileIterator final : public TableRowIterator {
 public:
  // Takes ownership of tempfile.
  // The table is used solely for NULL row flags.
  SortFileIterator(THD *thd, TABLE *table, IO_CACHE *tempfile,
                   Filesort_info *sort, ha_rows *examined_rows);
  ~SortFileIterator() override;

  bool Init() override { return false; }
  int Read() override;
  std::vector<std::string> DebugString() const override;
  void UnlockRow() override {}

 private:
  uchar *const m_rec_buf;
  const uint m_ref_length;
  IO_CACHE *const m_io_cache;
  Filesort_info *const m_sort;
  ha_rows *const m_examined_rows;
};

/**
  Fetch the record IDs from a temporary file, then the records themselves from
  the table on disk.

  Same as SortBufferIndirectIterator except that references are fetched
  from temporary file instead of from a memory buffer. So first the record IDs
  are read from file, then those record IDs are used to look up rows in the
  table.
 */
class SortFileIndirectIterator final : public TableRowIterator {
 public:
  // Takes ownership of tempfile.
  //
  // The pushed condition can be nullptr.
  //
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  SortFileIndirectIterator(THD *thd, TABLE *table, IO_CACHE *tempfile,
                           bool request_cache, bool ignore_not_found_rows,
                           ha_rows *examined_rows);
  ~SortFileIndirectIterator() override;

  bool Init() override;
  int Read() override;
  std::vector<std::string> DebugString() const override;

 private:
  bool InitCache();
  int CachedRead();
  int UncachedRead();

  IO_CACHE *m_io_cache = nullptr;
  ha_rows *const m_examined_rows;
  uchar *m_record = nullptr;
  uchar *m_ref_pos = nullptr; /* pointer to form->refpos */
  bool m_ignore_not_found_rows;

  // This is a special variant that can be used for
  // handlers that is not using the HA_FAST_KEY_READ table flag. Instead
  // of reading the references one by one from the temporary file it reads
  // a set of them, sorts them and reads all of them into a buffer which
  // is then used for a number of subsequent calls to Read().
  // It is only used for SELECT queries and a number of other conditions
  // on table size.
  bool m_using_cache;
  uint m_cache_records;
  uint m_ref_length, m_struct_length, m_reclength, m_rec_cache_size,
      m_error_offset;
  unique_ptr_my_free<uchar[]> m_cache;
  uchar *m_cache_pos = nullptr, *m_cache_end = nullptr,
        *m_read_positions = nullptr;
};

// Used when the plan is const, ie. is known to contain a single row
// (and all values have been read in advance, so we don't need to read
// a single table).
class FakeSingleRowIterator final : public RowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  FakeSingleRowIterator(THD *thd, ha_rows *examined_rows)
      : RowIterator(thd), m_examined_rows(examined_rows) {}

  bool Init() override {
    m_has_row = true;
    return false;
  }

  int Read() override {
    if (m_has_row) {
      m_has_row = false;
      if (m_examined_rows != nullptr) {
        ++*m_examined_rows;
      }
      return 0;
    } else {
      return -1;
    }
  }

  std::vector<std::string> DebugString() const override {
    return {"Rows fetched before execution"};
  }

  void SetNullRowFlag(bool is_null_row MY_ATTRIBUTE((unused))) override {
    DBUG_ASSERT(!is_null_row);
  }

  void UnlockRow() override {}

 private:
  bool m_has_row;
  ha_rows *const m_examined_rows;
};

/**
  An iterator for unqualified COUNT(*) (ie., no WHERE, no join conditions,
  etc.), taking a special fast path in the handler. It returns a single row,
  much like FakeSingleRowIterator; however, unlike said iterator, it actually
  does the counting in Read() instead of expecting all fields to already be
  filled out.
 */
class UnqualifiedCountIterator final : public RowIterator {
 public:
  UnqualifiedCountIterator(THD *thd, JOIN *join)
      : RowIterator(thd), m_join(join) {}

  std::vector<std::string> DebugString() const override;

  bool Init() override {
    m_has_row = true;
    return false;
  }

  int Read() override;

  void SetNullRowFlag(bool) override { DBUG_ASSERT(false); }

  void UnlockRow() override {}

 private:
  bool m_has_row;
  JOIN *const m_join;
};

/**
  A simple iterator that takes no input and produces zero output rows.
  Used when the optimizer has figured out ahead of time that a given table
  can produce no output (e.g. SELECT ... WHERE 2+2 = 5).

  The child iterator is optional (can be nullptr) if SetNullRowFlag() is
  not to be called. It is used when a subtree used on the inner side of an
  outer join is found to be never executable, and replaced with a
  ZeroRowsIterator; in that case, we need to forward the SetNullRowFlag call
  to it. This child is not printed as part of the iterator tree.
 */
class ZeroRowsIterator final : public RowIterator {
 public:
  ZeroRowsIterator(THD *thd, const char *reason,
                   unique_ptr_destroy_only<RowIterator> child_iterator)
      : RowIterator(thd),
        m_reason(reason),
        m_child_iterator(std::move(child_iterator)) {}

  bool Init() override { return false; }

  int Read() override { return -1; }

  std::vector<std::string> DebugString() const override {
    return {std::string("Zero rows (") + m_reason + ")"};
  }

  void SetNullRowFlag(bool is_null_row) override {
    DBUG_ASSERT(m_child_iterator != nullptr);
    m_child_iterator->SetNullRowFlag(is_null_row);
  }

  void UnlockRow() override {}

 private:
  const char *m_reason;
  unique_ptr_destroy_only<RowIterator> m_child_iterator;
};

class SELECT_LEX;

/**
  Like ZeroRowsIterator, but produces a single output row, since there are
  aggregation functions present and no GROUP BY. E.g.,

    SELECT SUM(f1) FROM t1 WHERE 2+2 = 5;

  should produce a single row, containing only the value NULL.
 */
class ZeroRowsAggregatedIterator final : public RowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  ZeroRowsAggregatedIterator(THD *thd, const char *reason, JOIN *join,
                             ha_rows *examined_rows)
      : RowIterator(thd),
        m_reason(reason),
        m_join(join),
        m_examined_rows(examined_rows) {}

  bool Init() override {
    m_has_row = true;
    return false;
  }

  int Read() override;

  std::vector<std::string> DebugString() const override {
    return {std::string("Zero input rows (") + m_reason +
            "), aggregated into one output row"};
  }

  void SetNullRowFlag(bool) override { DBUG_ASSERT(false); }

  void UnlockRow() override {}

 private:
  bool m_has_row;
  const char *const m_reason;
  JOIN *const m_join;
  ha_rows *const m_examined_rows;
};

/**
  FollowTailIterator is a special version of TableScanIterator that is used
  as part of WITH RECURSIVE queries. It is designed to read from a temporary
  table at the same time as MaterializeIterator writes to the same table,
  picking up new records in the order they come in -- it follows the tail,
  much like the UNIX tool “tail -f”.

  Furthermore, when materializing a recursive query expression consisting of
  multiple query blocks, MaterializeIterator needs to run each block several
  times until convergence. (For a single query block, one iteration suffices,
  since the iterator sees new records as they come in.) Each such run, the
  recursive references should see only rows that were added since the last
  iteration, even though Init() is called anew. FollowTailIterator is thus
  different from TableScanIterator in that subsequent calls to Init() do not
  move the cursor back to the start.

  In addition, FollowTailIterator implements the WITH RECURSIVE iteration limit.
  This is not specified in terms of Init() calls, since one run can encompass
  many iterations. Instead, it keeps track of the number of records in the table
  at the start of iteration, and when it has read all of those records, the next
  iteration is deemed to have begun. If the iteration counter is above the
  user-set limit, it raises an error to stop runaway queries with infinite
  recursion.
 */
class FollowTailIterator final : public TableRowIterator {
 public:
  // "examined_rows", if not nullptr, is incremented for each successful Read().
  FollowTailIterator(THD *thd, TABLE *table, QEP_TAB *qep_tab,
                     ha_rows *examined_rows);
  ~FollowTailIterator() override;

  bool Init() override;
  int Read() override;

  std::vector<std::string> DebugString() const override;

  /**
    Signal where we can expect to find the number of generated rows for this
    materialization (this points into the MaterializeIterator's data).

    This must be called when we start materializing the CTE,
    before Init() runs.
   */
  void set_stored_rows_pointer(ha_rows *stored_rows) {
    m_stored_rows = stored_rows;
  }

  /**
    Signal to the iterator that the underlying table was closed and replaced
    with an InnoDB table with the same data, due to a spill-to-disk
    (e.g. the table used to be MEMORY and now is InnoDB). This is
    required so that Read() can continue scanning from the right place.
    Called by MaterializeIterator::MaterializeRecursive().
   */
  bool RepositionCursorAfterSpillToDisk();

 private:
  uchar *const m_record;
  QEP_TAB *const m_qep_tab;
  ha_rows *const m_examined_rows;
  ha_rows m_read_rows;
  ha_rows m_end_of_current_iteration;
  unsigned m_recursive_iteration_count;

  // Points into MaterializeIterator's data; set by BeginMaterialization() only.
  ha_rows *m_stored_rows = nullptr;
};

/**
  TableValueConstructor is the iterator for the table value constructor case of
  a query_primary (i.e. queries of the form VALUES row_list; e.g. VALUES ROW(1,
  10), ROW(2, 20)).

  The iterator is passed the field list of its parent JOIN object, which may
  contain Item_values_column objects that are created during
  SELECT_LEX::prepare_values(). This is required so that Read() can replace the
  currently selected row by simply changing the references of Item_values_column
  objects to the next row.

  The iterator must output multiple rows without being materialized, and does
  not scan any tables. The indirection of Item_values_column is required, as the
  executor outputs what is contained in join->fields (either directly, or
  indirectly through ConvertItemsToCopy), and is thus responsible for ensuring
  that join->fields contains the correct next row.
 */
class TableValueConstructorIterator final : public RowIterator {
 public:
  TableValueConstructorIterator(THD *thd, ha_rows *examined_rows,
                                const List<List<Item>> &row_value_list,
                                List<Item> *join_fields);

  bool Init() override;
  int Read() override;

  std::vector<std::string> DebugString() const override {
    return {"Rows fetched before execution"};
  }

  void SetNullRowFlag(bool) override { DBUG_ASSERT(false); }

  void UnlockRow() override {}

 private:
  ha_rows *const m_examined_rows{nullptr};

  /// Contains the row values that are part of a VALUES clause. Read() will
  /// modify contained Item objects during execution by calls to is_null() and
  /// the required val function to extract its value.
  const List<List<Item>> &m_row_value_list;
  List_STL_Iterator<const List<Item>> m_row_it;

  /// References to the row we currently want to output. When multiple rows must
  /// be output, this contains Item_values_column objects. In this case, each
  /// call to Read() will replace its current reference with the next row.
  List<Item> *const m_output_refs;
};

#endif  // SQL_BASIC_ROW_ITERATORS_H_
