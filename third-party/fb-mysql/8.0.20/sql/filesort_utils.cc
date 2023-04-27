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

#include "sql/filesort_utils.h"

#include <string.h>
#include <algorithm>
#include <cmath>

#include "add_with_saturate.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_pointer_arithmetic.h"
#include "sql/cmp_varlen_keys.h"
#include "sql/opt_costmodel.h"
#include "sql/sort_param.h"
#include "sql/sql_sort.h"
#include "sql/thr_malloc.h"

PSI_memory_key key_memory_Filesort_buffer_sort_keys;

using std::max;
using std::min;
using std::sort;
using std::stable_sort;
using std::unique;
using std::vector;

namespace {

/*
  An inline function which does memcmp().
  This one turns out to be pretty fast on all platforms, except sparc.
  See the accompanying unit tests, which measure various implementations.
 */
inline bool my_mem_compare(const uchar *s1, const uchar *s2, size_t len) {
  DBUG_ASSERT(s1 != nullptr);
  DBUG_ASSERT(s2 != nullptr);
  for (size_t i = 0; i < len; ++i) {
    if (s1[i] != s2[i]) return s1[i] < s2[i];
  }
  return false;
}

#define COMPARE(N) \
  if (s1[N] != s2[N]) return s1[N] < s2[N]

inline bool my_mem_compare_longkey(const uchar *s1, const uchar *s2,
                                   size_t len) {
  COMPARE(0);
  COMPARE(1);
  COMPARE(2);
  COMPARE(3);
  return memcmp(s1 + 4, s2 + 4, len - 4) < 0;
}

class Mem_compare {
 public:
  explicit Mem_compare(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) const {
#ifdef __sun
    // The native memcmp is faster on SUN.
    return memcmp(s1, s2, m_size) < 0;
#else
    return my_mem_compare(s1, s2, m_size);
#endif
  }

 private:
  size_t m_size;
};

class Mem_compare_longkey {
 public:
  explicit Mem_compare_longkey(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) const {
#ifdef __sun
    // The native memcmp is faster on SUN.
    return memcmp(s1, s2, m_size) < 0;
#else
    return my_mem_compare_longkey(s1, s2, m_size);
#endif
  }

 private:
  size_t m_size;
};

class Mem_compare_varlen_key {
 public:
  Mem_compare_varlen_key(const Bounds_checked_array<st_sort_field> sfa,
                         bool use_hash_arg)
      : sort_field_array(sfa.array(), sfa.size()), use_hash(use_hash_arg) {}

  bool operator()(const uchar *s1, const uchar *s2) const {
    return cmp_varlen_keys(sort_field_array, use_hash, s1, s2);
  }

 private:
  Bounds_checked_array<st_sort_field> sort_field_array;
  bool use_hash;
};

template <class Comp>
class Equality_from_less {
 public:
  explicit Equality_from_less(const Comp &comp) : m_comp(comp) {}

  template <class A, class B>
  bool operator()(const A &a, const B &b) const {
    return !(m_comp(a, b) || m_comp(b, a));
  }

 private:
  const Comp &m_comp;
};

}  // namespace

unsigned Filesort_buffer::sort_buffer(Sort_param *param, uint count) {
  const bool force_stable_sort = param->m_force_stable_sort;
  param->m_sort_algorithm = Sort_param::FILESORT_ALG_NONE;

  if (count <= 1) return count;
  if (param->max_compare_length() == 0) return count;

  const auto it_begin = begin(m_record_pointers);
  const auto it_end = begin(m_record_pointers) + count;

  if (param->using_varlen_keys()) {
    const Mem_compare_varlen_key comp(param->local_sortorder, param->use_hash);
    if (force_stable_sort) {
      param->m_sort_algorithm = Sort_param::FILESORT_ALG_STD_STABLE;
      stable_sort(it_begin, it_end, comp);
    } else {
      // TODO: Make more elaborate heuristics than just always picking
      // std::sort.
      param->m_sort_algorithm = Sort_param::FILESORT_ALG_STD_SORT;
      sort(it_begin, it_end, comp);
    }
    if (param->m_remove_duplicates) {
      return unique(it_begin, it_end,
                    Equality_from_less<Mem_compare_varlen_key>(comp)) -
             it_begin;
    }
    return count;
  }

  // If we don't use addon fields, we'll have the record position appended to
  // the end of each record. This disturbs our equality comparisons, so we'll
  // have to remove it. (Removing it also makes the comparisons ever so slightly
  // cheaper.)
  size_t key_len = param->max_compare_length();
  if (!param->using_addon_fields()) {
    key_len -= param->ref_length;
  }

  /*
    std::stable_sort has some extra overhead in allocating the temp buffer,
    which takes some time. The cutover point where it starts to get faster
    than quicksort seems to be somewhere around 10 to 40 records.
    So we're a bit conservative, and stay with quicksort up to 100 records.
  */
  if (count <= 100 && !force_stable_sort) {
    if (key_len < 10) {
      param->m_sort_algorithm = Sort_param::FILESORT_ALG_STD_SORT;
      sort(it_begin, it_end, Mem_compare(key_len));
      if (param->m_remove_duplicates) {
        return unique(it_begin, it_end,
                      Equality_from_less<Mem_compare>(Mem_compare(key_len))) -
               it_begin;
      }
      return count;
    }
    param->m_sort_algorithm = Sort_param::FILESORT_ALG_STD_SORT;
    sort(it_begin, it_end, Mem_compare_longkey(key_len));
    if (param->m_remove_duplicates) {
      auto new_end = unique(it_begin, it_end,
                            Equality_from_less<Mem_compare_longkey>(
                                Mem_compare_longkey(key_len)));
      return new_end - it_begin;
    }
    return count;
  }

  param->m_sort_algorithm = Sort_param::FILESORT_ALG_STD_STABLE;
  // Heuristics here: avoid function overhead call for short keys.
  if (key_len < 10) {
    stable_sort(it_begin, it_end, Mem_compare(key_len));
    if (param->m_remove_duplicates) {
      return unique(it_begin, it_end,
                    Equality_from_less<Mem_compare>(Mem_compare(key_len))) -
             it_begin;
    }
  } else {
    stable_sort(it_begin, it_end, Mem_compare_longkey(key_len));
    if (param->m_remove_duplicates) {
      return unique(it_begin, it_end,
                    Equality_from_less<Mem_compare_longkey>(
                        Mem_compare_longkey(key_len))) -
             it_begin;
    }
  }
  return count;
}

void Filesort_buffer::reset() {
  update_peak_memory_used();
  m_record_pointers.clear();
  if (m_blocks.size() >= 2) {
    // Free every block but the last.
    m_blocks.erase(m_blocks.begin(), m_blocks.end() - 1);
  }

  /*
    m_max_record_length can have changed since last time; if the remaining
    (largest) block is not large enough for a single row of the next size,
    then clear out that, too.
  */
  if (m_max_record_length > m_current_block_size) {
    free_sort_buffer();
  }

  if (m_blocks.empty()) {
    DBUG_ASSERT(m_next_rec_ptr == nullptr);
    DBUG_ASSERT(m_current_block_end == nullptr);
    DBUG_ASSERT(m_current_block_size == 0);
  } else {
    m_next_rec_ptr = m_blocks[0].get();
    DBUG_ASSERT(m_current_block_end == m_next_rec_ptr + m_current_block_size);
  }
  m_space_used_other_blocks = 0;
}

bool Filesort_buffer::preallocate_records(size_t num_records) {
  if (m_max_record_length == 0xFFFFFFFFu) {
    // The rest of the code uses this value for “infinite” and saturates to it,
    // so even if we have a large sort buffer (> 4 GB), we we can't know for
    // sure there's going to be room.
    return true;
  }

  reset();

  const size_t bytes_needed = num_records * m_max_record_length;
  if (bytes_needed + num_records * sizeof(m_record_pointers[0]) >
      m_max_size_in_bytes) {
    return true;
  }

  /*
    If the remaining block can't hold what we need, then it's of no
    use to us (it doesn't save us any allocations), so get rid of it
    and allocate one that's exactly the right size.
  */
  if (m_next_rec_ptr + bytes_needed > m_current_block_end) {
    free_sort_buffer();
    if (allocate_sized_block(bytes_needed)) {
      return true;
    }
    m_record_pointers.reserve(num_records);
  }
  while (m_record_pointers.size() < num_records) {
    Bounds_checked_array<uchar> ptr =
        get_next_record_pointer(m_max_record_length);
    (void)ptr;
    DBUG_ASSERT(ptr.array() != nullptr);
    commit_used_memory(m_max_record_length);
  }
  return false;
}

bool Filesort_buffer::allocate_block(size_t num_bytes) {
  DBUG_EXECUTE_IF("alloc_sort_buffer_fail",
                  DBUG_SET("+d,simulate_out_of_memory"););

  size_t next_block_size;
  if (m_current_block_size == 0) {
    // First block.
    next_block_size = MIN_SORT_MEMORY;
  } else {
    next_block_size = m_current_block_size + m_current_block_size / 2;
  }

  /*
    If our last block isn't used at all, we can safely free it
    before we try to allocate a larger one. Note that we do this
    after the calculation above, which uses m_current_block_size.
  */
  if (!m_blocks.empty() && m_blocks.back().get() == m_next_rec_ptr) {
    m_current_block_size = 0;
    m_next_rec_ptr = nullptr;
    m_current_block_end = nullptr;
    m_blocks.pop_back();
  }

  // Figure out how much space we've used, to see how much is left (if
  // anything).
  size_t space_used = m_current_block_size + m_space_used_other_blocks;
  space_used += m_record_pointers.capacity() * sizeof(m_record_pointers[0]);

  size_t space_left;
  if (space_used > m_max_size_in_bytes)
    space_left = 0;
  else
    space_left = m_max_size_in_bytes - space_used;

  /*
    Adjust space_left to take into account that filling this new buffer
    with records would necessarily also add pointers to m_record_pointers.
    Note that we know how much space record_pointers currently is using,
    but not how much it could potentially be using in the future as we add
    records; we take a best-case estimate based on maximum-size records.
    It's also impossible to say how capacity() will change since this
    is an implementation detail, so we don't take that into account.
    This means that, for smaller records, we could go above the maximum
    permitted total memory usage.
  */
  size_t min_num_rows_capacity =
      m_record_pointers.size() +
      space_left /
          AddWithSaturate(m_max_record_length, sizeof(m_record_pointers[0]));
  if (min_num_rows_capacity > m_record_pointers.capacity()) {
    space_left -= (min_num_rows_capacity - m_record_pointers.capacity()) *
                  sizeof(m_record_pointers[0]);
  }

  next_block_size = min(max(next_block_size, num_bytes), space_left);
  if (next_block_size < num_bytes) {
    /*
      If we're really out of space, but have at least 32 kB unused in
      m_record_pointers, try to reclaim some space and try again. This should
      only be needed in some very rare cases where we first sort a lot of very
      short rows (yielding a huge amount of record pointers) and then need to
      sort huge rows that wouldn't fit in the buffer otherwise -- in other
      words, nearly never.
    */
    size_t excess_bytes =
        (m_record_pointers.capacity() - m_record_pointers.size()) *
        sizeof(m_record_pointers[0]);
    if (excess_bytes >= 32768) {
      size_t old_capacity = m_record_pointers.capacity();
      m_record_pointers.shrink_to_fit();
      if (m_record_pointers.capacity() < old_capacity) {
        return allocate_block(num_bytes);
      }
    }

    // We're full.
    return true;
  }

  return allocate_sized_block(next_block_size);
}

bool Filesort_buffer::allocate_sized_block(size_t block_size) {
  unique_ptr_my_free<uchar[]> new_block((uchar *)my_malloc(
      key_memory_Filesort_buffer_sort_keys, block_size, MYF(0)));
  if (new_block == nullptr) {
    return true;
  }

  m_space_used_other_blocks += m_current_block_size;
  m_current_block_size = block_size;
  m_next_rec_ptr = new_block.get();
  m_current_block_end = new_block.get() + m_current_block_size;
  m_blocks.push_back(std::move(new_block));

  return false;
}

void Filesort_buffer::free_sort_buffer() {
  update_peak_memory_used();

  // std::vector::clear() does not necessarily free all the memory,
  // but moving or swapping in an empty vector typically does (and we
  // rely on this, even though we really shouldn't). This shouldn't have
  // been a problem since they will be cleared in the destructor, but
  // there are _many_ places scattered around the code that construct TABLE
  // objects (which indirectly contain Filesort_buffer objects) and never
  // destroy them properly. (You can find lots of them easily by adding an
  // std::unique_ptr<int> to Filesort_buffer and giving it a value in the
  // constructor; it will leak all over the place.) We should fix that,
  // but for the time being, we have this workaround instead.
  m_record_pointers = vector<uchar *>();
  m_blocks = vector<unique_ptr_my_free<uchar[]>>();

  m_space_used_other_blocks = 0;
  m_next_rec_ptr = nullptr;
  m_current_block_end = nullptr;
  m_current_block_size = 0;
}

Bounds_checked_array<uchar> Filesort_buffer::get_contiguous_buffer() {
  if (m_current_block_size != m_max_size_in_bytes) {
    free_sort_buffer();

    if (allocate_sized_block(m_max_size_in_bytes)) {
      return Bounds_checked_array<uchar>(nullptr, 0);
    }
  }
  return Bounds_checked_array<uchar>(m_blocks.back().get(),
                                     m_max_size_in_bytes);
}

void Filesort_buffer::update_peak_memory_used() const {
  m_peak_memory_used =
      max(m_peak_memory_used,
          m_record_pointers.capacity() * sizeof(m_record_pointers[0]) +
              m_current_block_size + m_space_used_other_blocks);
}
