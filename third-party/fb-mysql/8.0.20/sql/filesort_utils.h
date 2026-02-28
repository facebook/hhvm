/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef FILESORT_UTILS_INCLUDED
#define FILESORT_UTILS_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <utility>
#include <vector>

#include "map_helpers.h"
#include "my_base.h"  // ha_rows
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/service_mysql_alloc.h"  // my_free
#include "sql/sql_array.h"              // Bounds_checked_array

class Cost_model_table;
class Sort_param;

/**
  Buffer used for storing records to be sorted. The records are stored in
  a series of buffers that are allocated incrementally, growing 50% each
  time, similar to how a MEM_ROOT works. This allows the user to set a large
  maximum buffer size without getting huge allocations for sorting small result
  sets. It means that if you actually _do_ use the entire buffer, there will be
  more allocations than one large allocation up-front, but this is a worthwhile
  tradeoff (those allocation will tend to disappear into the cost of actually
  getting all the rows and sorting them).

  In addition, Filesort_buffer stores a vector of pointers to the beginning of
  each record. It is these pointers that are actually sorted in filesort.
  If the records are small, this can add up to overhead on top of the amount of
  memory the user expected to use. We _do_ take already allocated pointers into
  account when calculating how big a new block can be, so the amount of badness
  is bounded:

  Assume that we have set maximum record size to infinity, but that in
  practice, they are are about the smallest size possible (4-byte sort key plus
  4-byte rowid) and that we are on a 64-bit system. Then, the worst possible
  overhead is that we use as much space on pointers as the size of the last
  (largest) block. We can look at the two possible extremes:

    - Smallest possible sort buffer (32 kB): 32 kB overhead.
    - A huge sort buffer (x kB): If the last block is y kB, the total size will
      be y + 2/3y + (2/3)²y + ... = 3y, which means the last block is 1/3 of the
      total size. Thus, pointer overhead will be no worse than 33%.

  In most practical cases, it will be much better than this. In particular,
  even when allocating a block (where we don't yet know how many records will
  fit), we allow space for the record pointers we'd need given maximum-sized
  rows.

  The buffer must be kept available for multiple executions of the
  same sort operation, so one can call reset() for reuse. Again similar
  to MEM_ROOT, this keeps the last (largest) block and discards the others.
*/
class Filesort_buffer {
 public:
  Filesort_buffer()
      : m_next_rec_ptr(nullptr),
        m_current_block_end(nullptr),
        m_max_size_in_bytes(0),
        m_current_block_size(0),
        m_space_used_other_blocks(0) {}

  /** Sort me...
    @return Number of records, after any deduplication
   */
  unsigned sort_buffer(Sort_param *param, uint count);

  /**
    Prepares the buffer for the next batch of records to process.
   */
  void reset();

  /**
    Where should the next record be stored?

    If a block is returned, it is always at least "min_size" bytes long.
    If the returned block is not large enough for your purposes,
    call get_next_record_pointer() again with a larger value of min_size than
    the size you got back. Just increasing the size by one byte is fine;
    the class will still try to make exponentially larger blocks each time.

    If there's no room for a record of the given size, returns nullptr.

    After you've written data to the given record, call commit_used_memory()
    with the number of bytes you've actually written. This ensures it will
    not get reused for subsequent records.
  */
  Bounds_checked_array<uchar> get_next_record_pointer(size_t min_size) {
    DBUG_ASSERT(min_size != 0xFFFFFFFFu);
    // See if we need to allocate a new block.
    if (m_next_rec_ptr + min_size > m_current_block_end) {
      if (allocate_block(min_size)) return Bounds_checked_array<uchar>();
    }

    // Allocate space within the current block.
    return Bounds_checked_array<uchar>(m_next_rec_ptr,
                                       m_current_block_end - m_next_rec_ptr);
  }

  void commit_used_memory(size_t num_bytes) {
    m_record_pointers.push_back(m_next_rec_ptr);
    m_next_rec_ptr += num_bytes;
  }

  /**
    Removes any existing rows and allocates `num_records` maximum-sized rows
    (call get_sorted_record() to get their pointers). This is somewhat more
    efficient than calling reset() and then get_next_record_pointer()
    repeatedly, as it guarantees that at most one allocation is needed.

    @returns true on memory allocation error, including if the allocated
    size would exceed max_size_in_bytes().
  */
  bool preallocate_records(size_t num_records);

  size_t max_size_in_bytes() const { return m_max_size_in_bytes; }

  /**
    How much memory has been allocated (counting both the sort buffer and the
    record pointers) at most since last call to clear_peak_memory_used().
    Note in particular that reset() and free_sort_buffer() does _not_ zero this
    counter.
  */
  size_t peak_memory_used() const {
    update_peak_memory_used();
    return m_peak_memory_used;
  }

  /// See peak_memory_used.
  void clear_peak_memory_used() { m_peak_memory_used = 0; }

  /**
    Set the memory limit for the sort buffer before starting to add records.
    If trying to allocate space for a new row (in get_next_record_pointer)
    would take us past the set limit, allocation will fail. Note that we can go
    a bit over this limit due to having to store record pointers; see the class
    comment.

    @param max_size       Maximum size of the sort buffer, in bytes.
    @param record_length  Worst-case size of each record, in bytes.
  */
  void set_max_size(size_t max_size, size_t record_length) {
    m_max_size_in_bytes = max_size;
    m_max_record_length = record_length;
  }

  /**
    Frees all memory. Unlike reset(), which keeps one block for future use,
    this actually releases all blocks. It is intended to release memory
    in an error situation, for final shutdown, or if even the largest block
    will not be large enough for future allocations.

    You do not need to call this if you are destroying the object anyway.
  */
  void free_sort_buffer();

  /**
    Get the list of record pointers as a contiguous array. Will be invalidated
    by calling get_next_record_pointer() or otherwise changing the number of
    records.
  */
  uchar **get_sort_keys() {
    if (m_record_pointers.empty()) return nullptr;
    return m_record_pointers.data();
  }

  /**
    Gets sorted record number ix. @see get_sort_keys()
    Only valid after buffer has been sorted!
  */
  uchar *get_sorted_record(size_t ix) {
    DBUG_ASSERT(ix < m_record_pointers.size());
    return m_record_pointers[ix];
  }

  /**
    Clears all rows, then returns a contiguous buffer of maximum size.
    (This may or may not involve allocation.) This is for reusing the memory
    for merge buffers, which requires the memory to be a single contiguous
    chunk; one could in theory adjust merging to allow using multiple buffers
    like sorting does, but once we need to merge, that means we've hit disk
    anyway (or at the very least, need to talk to the OS' buffer cache),
    and the cost of a single allocation is small compared to I/O.

    If you use this memory area, you cannot also use the Filesort_buffer to
    store sort records (get_next_record_pointer etc.); that would use the
    same memory.

    Can return nullptr, if allocation fails.
  */
  Bounds_checked_array<uchar> get_contiguous_buffer();

 private:
  /**
    Allocate a new block with space for at least `num_bytes` bytes.

    @returns true if the allocation failed (including if m_max_size_in_bytes
    was exceeded).
  */
  bool allocate_block(size_t num_bytes);

  /**
    Allocate a new block of exactly `block_size` bytes, and sets it
    as the current block. Does not check m_max_size_in_bytes.

    @returns true if the allocation failed
  */
  bool allocate_sized_block(size_t num_bytes);

  /// See m_peak_memory_used.
  void update_peak_memory_used() const;

  uchar *m_next_rec_ptr;       ///< The next record will be inserted here.
  uchar *m_current_block_end;  ///< The limit of the current block, exclusive.

  /// The memory blocks used for the actual data.
  std::vector<unique_ptr_my_free<uchar[]>> m_blocks;

  /// Pointer to the beginning of each record.
  std::vector<uchar *> m_record_pointers;

  size_t m_max_record_length;  ///< Worst-case length of each record.

  /// Maximum number of bytes we are allowed to allocate in all.
  size_t m_max_size_in_bytes;

  /**
    The size of the current memory block (m_blocks.back()), in bytes
    (or 0 if no block). If nothing has been allocated from the block yet,
    the invariant m_next_rec_ptr + m_current_block_size == m_current_block_end
    holds.
  */
  size_t m_current_block_size;

  /**
    The total size of all blocks except the current one, not including
    record pointers. Used for bookkeeping how far away we are from
    reaching m_max_size_in_bytes.
  */
  size_t m_space_used_other_blocks;

  /**
    The largest amount of total memory we've been using since last call to
    clear_peak_memory_used(). This is updated lazily so that we don't need
    to do the calculations for every record (and thus is mutable). The only
    point where it _must_ be explicitly updated (by calling
    update_peak_memory_used()), except when being asked for the value,
    is right before we deallocate memory, as otherwise, there could be a peak
    we had forgotten.
  */
  mutable size_t m_peak_memory_used{0};

  // Privately movable, but not copyable.
  Filesort_buffer &operator=(const Filesort_buffer &rhs) = delete;
  Filesort_buffer &operator=(Filesort_buffer &&rhs) = default;
};

#endif  // FILESORT_UTILS_INCLUDED
