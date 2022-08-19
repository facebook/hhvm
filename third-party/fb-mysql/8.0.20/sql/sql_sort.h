#ifndef SQL_SORT_INCLUDED
#define SQL_SORT_INCLUDED

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

#include "map_helpers.h"
#include "my_base.h"  // ha_rows
#include "my_dbug.h"
#include "my_sys.h"
#include "sql/filesort_utils.h"  // Filesort_buffer

class Addon_fields;

/* Defines used by filesort and uniques */

constexpr size_t MERGEBUFF = 7;
constexpr size_t MERGEBUFF2 = 15;
// Number of bytes used to store varlen key's length
constexpr size_t VARLEN_PREFIX = 4;

/**
  Descriptor for a merge chunk to be sort-merged.
  A merge chunk is a sequence of pre-sorted records, written to a
  temporary file. A Merge_chunk instance describes where this chunk is stored
  in the file, and where it is located when it is in memory.

  It is a POD because we read/write them from/to files (but note,
  only m_file_position and m_rowcount are actually used in that
  situation).

  We have accessors (getters/setters) for all struct members.
 */
struct Merge_chunk {
 public:
  Merge_chunk()
      : m_current_key(nullptr),
        m_file_position(0),
        m_buffer_start(nullptr),
        m_buffer_end(nullptr),
        m_rowcount(0),
        m_mem_count(0),
        m_max_keys(0) {}

  my_off_t file_position() const { return m_file_position; }
  void set_file_position(my_off_t val) { m_file_position = val; }
  void advance_file_position(my_off_t val) { m_file_position += val; }

  uchar *buffer_start() { return m_buffer_start; }
  const uchar *buffer_end() const { return m_buffer_end; }

  void set_buffer(uchar *start, uchar *end) {
    m_buffer_start = start;
    m_buffer_end = end;
  }
  void set_buffer_start(uchar *start) { m_buffer_start = start; }
  void set_buffer_end(uchar *end) {
    DBUG_ASSERT(m_buffer_end == nullptr || end <= m_buffer_end);
    m_buffer_end = end;
  }

  void init_current_key() { m_current_key = m_buffer_start; }
  uchar *current_key() const { return m_current_key; }
  void advance_current_key(uint val) { m_current_key += val; }

  void decrement_rowcount(ha_rows val) { m_rowcount -= val; }
  void set_rowcount(ha_rows val) { m_rowcount = val; }
  ha_rows rowcount() const { return m_rowcount; }

  ha_rows mem_count() const { return m_mem_count; }
  void set_mem_count(ha_rows val) { m_mem_count = val; }
  ha_rows decrement_mem_count() { return --m_mem_count; }

  ha_rows max_keys() const { return m_max_keys; }
  void set_max_keys(ha_rows val) { m_max_keys = val; }

  size_t buffer_size() const { return m_buffer_end - m_buffer_start; }

  /**
    Tries to merge *this with *mc, returns true if successful.
    The assumption is that *this is no longer in use,
    and the space it has been allocated can be handed over to a
    buffer which is adjacent to it.
   */
  bool merge_freed_buff(Merge_chunk *mc) const {
    if (mc->m_buffer_end == m_buffer_start) {
      mc->m_buffer_end = m_buffer_end;
      mc->m_max_keys += m_max_keys;
      return true;
    } else if (mc->m_buffer_start == m_buffer_end) {
      mc->m_buffer_start = m_buffer_start;
      mc->m_max_keys += m_max_keys;
      return true;
    }
    return false;
  }

 private:
  uchar *m_current_key;      ///< The current key for this chunk.
  my_off_t m_file_position;  ///< Current position in the file to be sorted.
  uchar *m_buffer_start;     ///< Start of main-memory buffer for this chunk.
  uchar *m_buffer_end;       ///< End of main-memory buffer for this chunk.
  ha_rows m_rowcount;        ///< Number of unread rows in this chunk.
  ha_rows m_mem_count;       ///< Number of rows in the main-memory buffer.
  ha_rows m_max_keys;        ///< If we have fixed-size rows:
                             ///    max number of rows in buffer.
};

typedef Bounds_checked_array<Merge_chunk> Merge_chunk_array;

/*
  The result of Unique or filesort; can either be stored on disk
  (in which case io_cache points to the file) or in memory in one
  of two ways. See sorted_result_in_fsbuf.

  Note if sort_result points into memory, it does _not_ own the sort buffer;
  Filesort_info does.

  TODO: Clean up so that Filesort / Filesort_info / Filesort_buffer /
  Sort_result have less confusing overlap.
*/
class Sort_result {
 public:
  Sort_result() : sorted_result_in_fsbuf(false), sorted_result_end(nullptr) {}

  bool has_result_in_memory() const {
    return sorted_result || sorted_result_in_fsbuf;
  }

  bool has_result() const {
    return has_result_in_memory() || (io_cache && my_b_inited(io_cache));
  }

  IO_CACHE *io_cache{nullptr};

  /**
    If the entire result fits in memory, we skip the merge phase.
    We may leave the result in the parent Filesort_info's filesort_buffer
    (indicated by sorted_result_in_fsbuf), or we may strip away
    the sort keys, and copy the sorted result into a new buffer.
    Unique always uses the latter.
    This new buffer is [sorted_result ... sorted_result_end]
    @see save_index()
   */
  bool sorted_result_in_fsbuf{false};
  unique_ptr_my_free<uchar> sorted_result{nullptr};
  uchar *sorted_result_end{nullptr};

  ha_rows found_records{0};  ///< How many records in sort.
};

/**
  A class wrapping misc buffers used for sorting.
 */
class Filesort_info {
  /// Buffer for sorting keys.
  Filesort_buffer filesort_buffer;

 public:
#ifndef DBUG_OFF
  bool file_size_exceeded; /* If filesort exceeded max file size */
#endif                     /* DBUG_OFF */

  Merge_chunk_array merge_chunks;  ///< Array of chunk descriptors

  Addon_fields *addon_fields{nullptr};  ///< Addon field descriptors.

  bool m_using_varlen_keys{false};
  uint m_sort_length{0};

  Filesort_info(const Filesort_info &) = delete;
  Filesort_info &operator=(const Filesort_info &) = delete;

  Filesort_info() : m_using_varlen_keys(false), m_sort_length(0) {}

  /** Sort filesort_buffer
    @return Number of records, after any deduplication
   */
  unsigned sort_buffer(Sort_param *param, uint count) {
    return filesort_buffer.sort_buffer(param, count);
  }

  /**
    Copies (unpacks) values appended to sorted fields from a buffer back to
    their regular positions specified by the Field::ptr pointers.
    @param buff            Buffer which to unpack the value from
  */
  template <bool Packed_addon_fields>
  inline void unpack_addon_fields(uchar *buff);

  /**
    Reads 'count' number of chunk descriptors into the merge_chunks array.
    In case of error, the merge_chunks array will be empty.
    @param chunk_file File containing the descriptors.
    @param count      Number of chunks to read.
  */
  void read_chunk_descriptors(IO_CACHE *chunk_file, uint count);

  /// Are we using "addon fields"?
  bool using_addon_fields() const { return addon_fields != nullptr; }

  void reset() { filesort_buffer.reset(); }

  void clear_peak_memory_used() { filesort_buffer.clear_peak_memory_used(); }

  Bounds_checked_array<uchar> get_next_record_pointer(size_t min_size) {
    return filesort_buffer.get_next_record_pointer(min_size);
  }

  void commit_used_memory(size_t num_bytes) {
    filesort_buffer.commit_used_memory(num_bytes);
  }

  uchar *get_sorted_record(uint idx) {
    return filesort_buffer.get_sorted_record(idx);
  }

  uchar **get_sort_keys() { return filesort_buffer.get_sort_keys(); }

  Bounds_checked_array<uchar> get_contiguous_buffer() {
    return filesort_buffer.get_contiguous_buffer();
  }

  void set_max_size(size_t max_size, size_t record_length) {
    filesort_buffer.set_max_size(max_size, record_length);
  }

  void free_sort_buffer() { filesort_buffer.free_sort_buffer(); }

  bool preallocate_records(size_t num_records) {
    return filesort_buffer.preallocate_records(num_records);
  }

  size_t peak_memory_used() const { return filesort_buffer.peak_memory_used(); }

  size_t max_size_in_bytes() const {
    return filesort_buffer.max_size_in_bytes();
  }

  uint sort_length() const { return m_sort_length; }
  bool using_varlen_keys() const { return m_using_varlen_keys; }

  void set_sort_length(uint val, bool is_varlen) {
    m_sort_length = val;
    m_using_varlen_keys = is_varlen;
  }
};

typedef Bounds_checked_array<uchar> Sort_buffer;

/**
  Put all room used by freed buffer to use in adjacent buffer.

  Note, that we can't simply distribute memory evenly between all buffers,
  because new areas must not overlap with old ones.
*/
template <typename Heap_type>
void reuse_freed_buff(Merge_chunk *old_top, Heap_type *heap) {
  typename Heap_type::iterator it = heap->begin();
  typename Heap_type::iterator end = heap->end();
  for (; it != end; ++it) {
    if (old_top->merge_freed_buff(*it)) return;
  }
  DBUG_ASSERT(0);
}

bool filesort_open_cached_file(IO_CACHE *cache, const char *dir,
                               const char *prefix, size_t cache_size,
                               myf cache_myflags, void *fs_info = nullptr);

#endif /* SQL_SORT_INCLUDED */
