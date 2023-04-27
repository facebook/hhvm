#ifndef SQL_BKA_ITERATOR_H_
#define SQL_BKA_ITERATOR_H_

/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
  Batch key access (BKA) is a join strategy that uses multi-range read (MRR)
  to get better read ordering on the table on the inner side. It reads
  a block of rows from the outer side, picks up the join keys (refs) from
  each row, and sends them all off in one big read request. The handler can
  then order these and read them in whatever order it would prefer. This is
  especially attractive when used with rotating media; the reads can then be
  ordered such that it does not require constant seeking (disk-sweep MRR,
  or DS-MRR).

  BKA is implemented with two iterators working in concert. The BKAIterator
  reads rows from the outer side into a buffer. When the buffer is full or we
  are out of rows, it then sets up the key ranges and hand it over to the
  MultiRangeRowIterator, which does the actual request, and reads rows from it
  until there are none left. For each inner row returned, MultiRangeRowIterator
  loads the appropriate outer row(s) from the buffer, doing the actual join.

  The reason for this split is twofold. First, it allows us to accurately time
  (for EXPLAIN ANALYZE) the actual table read. Second, and more importantly,
  we can have other iterators between the BKAIterator and MultiRangeRowIterator,
  in particular FilterIterator.
 */

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <string>
#include <vector>

#include "my_alloc.h"
#include "my_inttypes.h"
#include "sql/handler.h"
#include "sql/hash_join_buffer.h"
#include "sql/mem_root_array.h"
#include "sql/row_iterator.h"
#include "sql_string.h"
#include "template_utils.h"

class Item;
class MultiRangeRowIterator;
class QEP_TAB;
class THD;
struct KEY_MULTI_RANGE;
struct TABLE;
struct TABLE_REF;

/**
  The BKA join iterator, with an arbitrary iterator tree on the outer side
  and a MultiRangeRowIterator on the inner side (possibly with a filter or
  similar in-between). See file comment for more details.
 */
class BKAIterator final : public RowIterator {
 public:
  /**
    @param thd Thread handle.
    @param join The JOIN we are part of.
    @param outer_input The iterator to read the outer rows from.
    @param outer_input_tables QEP_TAB for each outer table involved.
      Used to know which fields we are to read into our buffer.
    @param inner_input The iterator to read the inner rows from.
      Must end up in a MultiRangeRowIterator.
    @param max_memory_available Number of bytes available for row buffers,
      both outer rows and MRR buffers. Note that allocation is incremental,
      so we can allocate less than this.
    @param mrr_bytes_needed_for_single_inner_row Number of bytes MRR needs
      space for in its buffer for holding a single row from the inner table.
    @param expected_inner_rows_per_outer_row Number of inner rows we
      statistically expect for each outer row. Used for dividing the buffer
      space between inner rows and MRR row buffer (if we expect many inner
      rows, we can't load as many outer rows).
    @param mrr_iterator Pointer to the MRR iterator at the bottom of
      inner_input. Used to send row ranges and buffers.
    @param join_type What kind of join we are executing.
   */
  BKAIterator(THD *thd, JOIN *join,
              unique_ptr_destroy_only<RowIterator> outer_input,
              qep_tab_map outer_input_tables,
              unique_ptr_destroy_only<RowIterator> inner_input,
              size_t max_memory_available,
              size_t mrr_bytes_needed_for_single_inner_row,
              float expected_inner_rows_per_outer_row,
              MultiRangeRowIterator *mrr_iterator, JoinType join_type);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_outer_input->SetNullRowFlag(is_null_row);
    m_inner_input->SetNullRowFlag(is_null_row);
  }

  void UnlockRow() override {
    // Since we don't know which condition that caused the row to be rejected,
    // we can't know whether we could also unlock the outer row
    // (it may still be used as parts of other joined rows).
    if (m_state == State::RETURNING_JOINED_ROWS) {
      m_inner_input->UnlockRow();
    }
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{
        {m_outer_input.get(), "Batch input rows"},
        {m_inner_input.get(), ""},
    };
  }

  void EndPSIBatchModeIfStarted() override {
    m_outer_input->EndPSIBatchModeIfStarted();
    m_inner_input->EndPSIBatchModeIfStarted();
  }

 private:
  /// Clear out the MEM_ROOT and prepare for reading rows anew.
  void BeginNewBatch();

  /// If there are more outer rows, begin the next batch. If not,
  /// move to the EOF state.
  void BatchFinished();

  /// Find the next unmatched row, and load it for output as a NULL-complemented
  /// row. (Assumes the NULL row flag has already been set on the inner table
  /// iterator.) Returns 0 if a row was found, -1 if no row was found. (Errors
  /// cannot happen.)
  int MakeNullComplementedRow();

  /// Read a batch of outer rows (BeginNewBatch() must have been called
  /// earlier). Returns -1 for no outer rows found (sets state to END_OF_ROWS),
  /// 0 for OK (sets state to RETURNING_JOINED_ROWS) or 1 for error.
  int ReadOuterRows();

  enum class State {
    /**
      We are about to start reading outer rows into our buffer.
      A single Read() call will fill it up, so there is no
      in-between “currently reading” state.
     */
    NEED_OUTER_ROWS,

    /**
      We are returning rows from the MultiRangeRowIterator.
      (For antijoins, we are looking up the rows, but don't actually
      return them.)
     */
    RETURNING_JOINED_ROWS,

    /**
      We are an outer join or antijoin, and we're returning NULL-complemented
      rows for those outer rows that never had a matching inner row. Note that
      this is done in the BKAIterator and not the MRR iterator for two reasons:
      First, it gives more sensible EXPLAIN ANALYZE numbers. Second, the
      NULL-complemented rows could be filtered inadvertently by a FilterIterator
      before they reach the BKAIterator.
     */
    RETURNING_NULL_COMPLEMENTED_ROWS,

    /**
      Both the outer and inner side are out of rows.
     */
    END_OF_ROWS
  };

  State m_state;

  const unique_ptr_destroy_only<RowIterator> m_outer_input;
  const unique_ptr_destroy_only<RowIterator> m_inner_input;

  /// The MEM_ROOT we are storing the outer rows on, and also allocating MRR
  /// buffer from. In total, this should not go significantly over
  /// m_max_memory_available bytes.
  MEM_ROOT m_mem_root;

  /// Buffered outer rows.
  Mem_root_array<hash_join_buffer::BufferRow> m_rows;

  /// Tables and columns needed for each outer row. Rows/columns that are not
  /// needed are filtered out in the constructor; the rest are read and stored
  /// in m_rows.
  hash_join_buffer::TableCollection m_outer_input_tables;

  /// Used for serializing the row we read from the outer table(s), before it
  /// stored into the MEM_ROOT and put into m_rows. Should there not be room in
  /// m_rows for the row, it will stay in this variable until we start reading
  /// the next batch of outer rows.
  ///
  /// If there are no BLOB/TEXT column in the join, we calculate an upper bound
  /// of the row size that is used to preallocate this buffer. In the case of
  /// BLOB/TEXT columns, we cannot calculate a reasonable upper bound, and the
  /// row size is calculated per row. The allocated memory is kept for the
  /// duration of the iterator, so that we (most likely) avoid reallocations.
  String m_outer_row_buffer;

  /// Whether we have a row in m_outer_row_buffer from the previous batch of
  /// rows that we haven't stored in m_rows yet.
  bool m_has_row_from_previous_batch = false;

  /// For each outer row, how many bytes we need in the MRR buffer (ie., the
  /// number of bytes we expect to use on rows from the inner table).
  /// This is the expected number of inner rows per key, multiplied by the
  /// (fixed) size of each inner row. We use this information to stop scanning
  /// before we've used up the entire RAM allowance on outer rows, so that
  /// we have space remaining for the inner rows (in the MRR buffer), too.
  size_t m_mrr_bytes_needed_per_row;

  /// Estimated number of bytes used on m_mem_root so far.
  size_t m_bytes_used = 0;

  /// Whether we've seen EOF from the outer iterator.
  bool m_end_of_outer_rows = false;

  /// See max_memory_available in the constructor.
  const size_t m_max_memory_available;

  /// See max_memory_available in the constructor.
  const size_t m_mrr_bytes_needed_for_single_inner_row;

  /// See mrr_iterator in the constructor.
  MultiRangeRowIterator *const m_mrr_iterator;

  /// The join type of the BKA join.
  JoinType m_join_type;

  /// If we are synthesizing NULL-complemented rows (for an outer join or
  /// antijoin), points to the next row within "m_rows" that we haven't
  /// considered yet.
  hash_join_buffer::BufferRow *m_current_pos;
};

/**
  The iterator actually doing the reads from the inner table during BKA.
  See file comment.
 */
class MultiRangeRowIterator final : public TableRowIterator {
 public:
  /**
    @param thd Thread handle.
    @param cache_idx_cond See m_cache_idx_cond.
    @param table The inner table to scan.
    @param keep_current_rowid If true, get the row ID on the inner table
      for each row that we return. (Row IDs for outer tables will be
      controlled by outer_input_tables.)
    @param ref The index condition we are looking up on.
    @param mrr_flags Flags passed on to MRR.
   */
  MultiRangeRowIterator(THD *thd, Item *cache_idx_cond, TABLE *table,
                        bool keep_current_rowid, TABLE_REF *ref, int mrr_flags);

  /**
    Tell the MRR iterator which tables are on the left side of the BKA join
    (the MRR iterator is always alone on the right side). This is needed so
    that it can unpack the rows into the right tables, with the right format.

    Must be called exactly once, before first Init(). Set by BKAIterator's
    constructor; it's not easily available at the point where we construct
    MultiRangeRowIterator.
   */
  void set_outer_input_tables(JOIN *join, qep_tab_map outer_input_tables);

  /**
    Tell the MRR iterator what kind of BKA join it is part of.

    Must be called exactly once, before first Init(). Set by BKAIterator's
    constructor; it's not easily available at the point where we construct
    MultiRangeRowIterator.
   */
  void set_join_type(JoinType join_type) { m_join_type = join_type; }

  /**
    Specify which outer rows to read inner rows for.
    Must be called before Init(), and be valid until the last Read().
   */
  void set_rows(const hash_join_buffer::BufferRow *begin,
                const hash_join_buffer::BufferRow *end) {
    m_begin = begin;
    m_end = end;
  }

  /**
    Specify an unused chunk of memory MRR can use for the returned inner rows.
    Must be called before Init(), and must be at least big enough to hold
    one inner row.
   */
  void set_mrr_buffer(uchar *ptr, size_t size) {
    m_mrr_buffer.buffer = ptr;
    m_mrr_buffer.buffer_end = ptr + size;
  }

  /**
    Specify an unused chunk of memory that we can use to mark which inner rows
    have been read (by the parent BKA iterator) or not. This is used for outer
    joins to know which rows need NULL-complemented versions, and for semijoins
    and antijoins to avoid matching the same inner row more than once.

    Must be called before Init() for semijoins, outer joins and antijoins, and
    never called otherwise. There must be room at least for one bit per row
    given in set_rows().
   */
  void set_match_flag_buffer(uchar *ptr) { m_match_flag_buffer = ptr; }

  /**
    Mark that the BKA iterator has seen the last row we returned from Read().
    (It could have been discarded by a FilterIterator before it reached them.)
    Will be a no-op for inner joins; see set_match_flag_buffer()..
   */
  void MarkLastRowAsRead() {
    if (m_match_flag_buffer != nullptr) {
      size_t row_number = std::distance(m_begin, m_last_row_returned);
      m_match_flag_buffer[row_number / 8] |= 1 << (row_number % 8);
    }
  }

  /**
    Check whether the given row has been marked as read
    (using MarkLastRowAsRead()) or not. Used internally when doing semijoins,
    and also by the BKAIterator when synthesizing NULL-complemented rows for
    outer joins or antijoins.
   */
  bool RowHasBeenRead(const hash_join_buffer::BufferRow *row) const {
    DBUG_ASSERT(m_match_flag_buffer != nullptr);
    size_t row_number = std::distance(m_begin, row);
    return m_match_flag_buffer[row_number / 8] & (1 << (row_number % 8));
  }

  /**
    Do the actual multi-range read with the rows given by set_rows() and using
    the temporary buffer given in set_mrr_buffer().
   */
  bool Init() override;

  /**
    Read another inner row (if any) and load the appropriate outer row(s)
    into the associated table buffers.
   */
  int Read() override;

  std::vector<std::string> DebugString() const override;

 private:
  // Thunks from function pointers to the actual callbacks.
  static range_seq_t MrrInitCallbackThunk(void *init_params, uint n_ranges,
                                          uint flags) {
    return (pointer_cast<MultiRangeRowIterator *>(init_params))
        ->MrrInitCallback(n_ranges, flags);
  }
  static uint MrrNextCallbackThunk(void *init_params, KEY_MULTI_RANGE *range) {
    return (pointer_cast<MultiRangeRowIterator *>(init_params))
        ->MrrNextCallback(range);
  }
  static bool MrrSkipIndexTupleCallbackThunk(range_seq_t seq,
                                             char *range_info) {
    return (reinterpret_cast<MultiRangeRowIterator *>(seq))
        ->MrrSkipIndexTuple(range_info);
  }
  static bool MrrSkipRecordCallbackThunk(range_seq_t seq, char *range_info,
                                         uchar *) {
    return (reinterpret_cast<MultiRangeRowIterator *>(seq))
        ->MrrSkipRecord(range_info);
  }

  // Callbacks we get from the handler during the actual read.
  range_seq_t MrrInitCallback(uint n_ranges, uint flags);
  uint MrrNextCallback(KEY_MULTI_RANGE *range);
  bool MrrSkipIndexTuple(char *range_info);
  bool MrrSkipRecord(char *range_info);

  /**
    There are certain conditions that would normally be pushed down to indexes,
    but that depend on the values of outer tables in the BKA join (ie., they are
    join conditions), which are not set when we actually read the inner row.[1]
    Thus, we cannot push them all the way down to the handler; however, MRR
    gives us a similar mechanism that we can use. Specifically, if we set
    “skip_index_tuple” to a function pointer, we will be called back for each
    row, and can load the outer table row(s) we need to evaluate the condition.
    This allows us to reject the rows based on the index entry alone, without
    loading the row itself.

    It is unclear how much benefit this gives us over simply not pushing these
    conditions at all. The case of a join condition that is satisfiable using
    the index tuple but not simply pushable down into the ref is rare; it has to
    either be on a keypart we couldn't use (e.g., an index on A,B,C where we
    join A and C but not B -- A then becomes part of our ref, but C needs to be
    an index condition) or a condition that needs to be rechecked, which happens
    only when mixing PAD SPACE / NO PAD in a join (e.g. looking up in a CHAR
    column, but wanting the comparison as NO PAD). Especially the latter case
    would seem unlikely to filter away a significant amount of rows.

    [1] In the DebugString output, we call such conditions “dependent
        index conditions”, since they depend on values from other tables,
        analogous to dependent subqueries. Internally, they are called
        cache_idx_cond, presumably because BKA originated in join buffering,
        also known as join cache.
   */
  Item *const m_cache_idx_cond;

  /// See constructor.
  const bool m_keep_current_rowid;

  /// The table we are reading from.
  TABLE *const m_table;

  /// Handler for the table we are reading from.
  handler *const m_file;

  /// The index condition.
  TABLE_REF *const m_ref;

  /// Flags passed on to MRR.
  const int m_mrr_flags;

  /// Current outer rows to read inner rows for. Set by set_rows().
  const hash_join_buffer::BufferRow *m_begin;
  const hash_join_buffer::BufferRow *m_end;

  /// Which row we are at in the [m_begin, m_end) range.
  /// Used during the MRR callbacks.
  const hash_join_buffer::BufferRow *m_current_pos;

  /// What row we last returned from Read() (used for MarkLastRowAsRead()).
  const hash_join_buffer::BufferRow *m_last_row_returned;

  /// Temporary space for storing inner rows, used by MRR.
  /// Set by set_mrr_buffer().
  HANDLER_BUFFER m_mrr_buffer;

  /// See set_match_flag_buffer().
  uchar *m_match_flag_buffer = nullptr;

  /// Tables and columns needed for each outer row. Same as m_outer_input_tables
  /// in the corresponding BKAIterator.
  hash_join_buffer::TableCollection m_outer_input_tables;

  /// The join type of the BKA join we are part of. Same as m_join_type in the
  /// corresponding BKAIterator.
  JoinType m_join_type = JoinType::INNER;
};

#endif  // SQL_BKA_ITERATOR_H_
