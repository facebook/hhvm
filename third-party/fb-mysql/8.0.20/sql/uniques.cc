/* Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  Function to handle quick removal of duplicates
  This code is used when doing multi-table deletes to find the rows in
  reference tables that needs to be deleted.

  The basic idea is as follows:

  Store first all strings in a binary tree, ignoring duplicates.
  When the tree uses more memory than 'max_heap_table_size',
  write the tree (in sorted order) out to disk and start with a new tree.
  When all data has been generated, merge the trees (removing any found
  duplicates).

  The unique entries will be returned in sort order, to ensure that we do the
  deletes in disk order.
*/

#include "sql/uniques.h"  // Unique

#include <string.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <vector>

#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_tree.h"  // element_count
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "priority_queue.h"
#include "sql/field.h"    // Field
#include "sql/handler.h"  // handler
#include "sql/malloc_allocator.h"
#include "sql/merge_many_buff.h"
#include "sql/mysqld.h"  // mysql_tmpdir
#include "sql/opt_costmodel.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_base.h"  // TEMP_PREFIX
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_executor.h"  // check_unique_constraint
#include "sql/sql_sort.h"
#include "sql/sql_tmp_table.h"  // create_duplicate_weedout_tmp_table
#include "sql/table.h"

namespace {

struct Merge_chunk_compare_context {
  qsort2_cmp key_compare;
  const void *key_compare_arg;
};

class Uniq_param {
 public:
  uint rec_length;           // Length of sorted records.
  uint max_keys_per_buffer;  // Max keys / buffer.
  ha_rows max_rows;          // Select limit, or HA_POS_ERROR if unlimited.

  uchar *unique_buff;
  bool not_killable;

  // The fields below are used only by Unique class.
  Merge_chunk_compare_context cmp_context;
  typedef int (*chunk_compare_fun)(Merge_chunk_compare_context *ctx,
                                   uchar *arg1, uchar *arg2);
  chunk_compare_fun compare;

  Uniq_param() { memset(this, 0, sizeof(*this)); }

  // Not copyable
  Uniq_param(const Uniq_param &) = delete;
  Uniq_param &operator=(const Uniq_param &) = delete;
};

}  // namespace

/**
  Read data to buffer.

  @returns
    (uint)-1 if something goes wrong
*/
static uint uniq_read_to_buffer(IO_CACHE *fromfile, Merge_chunk *merge_chunk,
                                Uniq_param *param) {
  DBUG_TRACE;
  const uint rec_length = param->rec_length;

  const ha_rows count =
      std::min(merge_chunk->max_keys(), merge_chunk->rowcount());
  if (count) {
    const size_t bytes_to_read = rec_length * static_cast<size_t>(count);

    DBUG_PRINT("info", ("uniq_read_to_buffer %p at file_pos %llu bytes %llu",
                        merge_chunk,
                        static_cast<ulonglong>(merge_chunk->file_position()),
                        static_cast<ulonglong>(bytes_to_read)));
    if (mysql_file_pread(fromfile->file, merge_chunk->buffer_start(),
                         bytes_to_read, merge_chunk->file_position(), MYF_RW))
      return (uint)-1; /* purecov: inspected */

    merge_chunk->init_current_key();
    merge_chunk->advance_file_position(bytes_to_read);
    merge_chunk->decrement_rowcount(count);
    merge_chunk->set_mem_count(count);
    return bytes_to_read;
  }

  return 0;
} /* uniq_read_to_buffer */

/**
  Merge buffers to one buffer.

  @param thd
  @param param          Sort parameter
  @param from_file      File with source data (Merge_chunks point to this file)
  @param to_file        File to write the sorted result data.
  @param sort_buffer    Buffer for data to store up to MERGEBUFF2 sort keys.
  @param [out] last_chunk Store here Merge_chunk describing data written to
                        to_file.
  @param chunk_array    Array of chunks to merge.
  @param flag           0 - write full record, 1 - write addon/ref

  @returns
    0      OK
  @returns
    other  error
*/
static int merge_buffers(THD *thd, Uniq_param *param, IO_CACHE *from_file,
                         IO_CACHE *to_file, Sort_buffer sort_buffer,
                         Merge_chunk *last_chunk, Merge_chunk_array chunk_array,
                         int flag MY_ATTRIBUTE((unused))) {
  int error = 0;
  uint rec_length;
  ha_rows maxcount;
  ha_rows max_rows, org_max_rows;
  my_off_t to_start_filepos;
  uchar *strpos;
  Merge_chunk *merge_chunk;
  Uniq_param::chunk_compare_fun cmp;
  Merge_chunk_compare_context *first_cmp_arg;
  DBUG_TRACE;

  thd->inc_status_sort_merge_passes();

  rec_length = param->rec_length;

  const uint offset = 0;
  maxcount = (param->max_keys_per_buffer / chunk_array.size());
  to_start_filepos = my_b_tell(to_file);
  strpos = sort_buffer.array();
  org_max_rows = max_rows = param->max_rows;

  /* The following will fire if there is not enough space in sort_buffer */
  DBUG_ASSERT(maxcount != 0);

  DBUG_ASSERT(param->unique_buff != nullptr);

  cmp = param->compare;
  first_cmp_arg = &param->cmp_context;

  auto greater = [&cmp, &first_cmp_arg](const Merge_chunk *a,
                                        const Merge_chunk *b) {
    return cmp(first_cmp_arg, a->current_key(), b->current_key()) > 0;
  };

  Priority_queue<Merge_chunk *,
                 std::vector<Merge_chunk *, Malloc_allocator<Merge_chunk *>>,
                 decltype(greater)>
  queue(greater,
        Malloc_allocator<Merge_chunk *>(key_memory_Filesort_info_merge));

  if (queue.reserve(chunk_array.size())) return 1;

  for (merge_chunk = chunk_array.begin(); merge_chunk != chunk_array.end();
       merge_chunk++) {
    merge_chunk->set_buffer(
        strpos, strpos + (sort_buffer.size() / (chunk_array.size())));
    merge_chunk->set_max_keys(maxcount);
    uint bytes_read = uniq_read_to_buffer(from_file, merge_chunk, param);

    if (static_cast<int>(bytes_read) == -1) return -1; /* purecov: inspected */

    strpos += bytes_read;
    merge_chunk->set_buffer_end(strpos);

    // If less data in buffers than expected
    merge_chunk->set_max_keys(merge_chunk->mem_count());
    (void)queue.push(merge_chunk);
  }

  {
    /*
       Called by Unique::get()
       Copy the first argument to param->unique_buff for unique removal.
       Store it also in 'to_file'.
    */
    merge_chunk = queue.top();
    memcpy(param->unique_buff, merge_chunk->current_key(), rec_length);
    if (my_b_write(to_file, merge_chunk->current_key(), rec_length)) {
      return 1; /* purecov: inspected */
    }
    merge_chunk->advance_current_key(rec_length);
    merge_chunk->decrement_mem_count();
    if (--max_rows == 0) {
      error = 0; /* purecov: inspected */
      goto end;  /* purecov: inspected */
    }
    // The top chunk may actually contain only a single element
    if (merge_chunk->mem_count() == 0) {
      if (!(error = (int)uniq_read_to_buffer(from_file, merge_chunk, param))) {
        queue.pop();
        reuse_freed_buff(merge_chunk, &queue);
      } else if (error == -1)
        return error;
    }
    queue.update_top();  // Top element has been used
  }

  while (queue.size() > 1) {
    if (thd->killed && !param->not_killable) {
      return 1; /* purecov: inspected */
    }
    for (;;) {
      merge_chunk = queue.top();
      {
        uchar *current_key = merge_chunk->current_key();
        if (!(*cmp)(first_cmp_arg, param->unique_buff, current_key))
          goto skip_duplicate;
        memcpy(param->unique_buff, merge_chunk->current_key(), rec_length);
      }
      {
        const uint bytes_to_write = param->rec_length;

        DBUG_PRINT("info", ("write record at %llu len %u", my_b_tell(to_file),
                            bytes_to_write));
        if (my_b_write(to_file, merge_chunk->current_key() + offset,
                       bytes_to_write)) {
          return 1; /* purecov: inspected */
        }
        if (--max_rows == 0) {
          error = 0; /* purecov: inspected */
          goto end;  /* purecov: inspected */
        }
      }

    skip_duplicate:
      merge_chunk->advance_current_key(rec_length);
      merge_chunk->decrement_mem_count();
      if (0 == merge_chunk->mem_count()) {
        if (!(error =
                  (int)uniq_read_to_buffer(from_file, merge_chunk, param))) {
          queue.pop();
          reuse_freed_buff(merge_chunk, &queue);
          break; /* One buffer has been removed */
        } else if (error == -1)
          return error; /* purecov: inspected */
      }
      /*
        The Merge_chunk at the queue's top had one of its keys consumed, thus
        it may now rank differently in the comparison order of the queue, so:
      */
      queue.update_top();
    }
  }
  merge_chunk = queue.top();
  merge_chunk->set_buffer(sort_buffer.array(),
                          sort_buffer.array() + sort_buffer.size());
  merge_chunk->set_max_keys(param->max_keys_per_buffer);

  /*
    As we know all entries in the buffer are unique, we only have to
    check if the first one is the same as the last one we wrote
  */
  {
    uchar *current_key = merge_chunk->current_key();
    if (!(*cmp)(first_cmp_arg, param->unique_buff, current_key)) {
      merge_chunk->advance_current_key(rec_length);  // Remove duplicate
      merge_chunk->decrement_mem_count();
    }
  }

  do {
    if (merge_chunk->mem_count() > max_rows) {
      merge_chunk->set_mem_count(max_rows); /* Don't write too many records */
      merge_chunk->set_rowcount(0);         /* Don't read more */
    }
    max_rows -= merge_chunk->mem_count();

    for (uint ix = 0; ix < merge_chunk->mem_count(); ++ix) {
      const uint bytes_to_write = param->rec_length;
      if (my_b_write(to_file, merge_chunk->current_key() + offset,
                     bytes_to_write)) {
        return 1; /* purecov: inspected */
      }
      merge_chunk->advance_current_key(rec_length);
    }
  } while ((error = (int)uniq_read_to_buffer(from_file, merge_chunk, param)) !=
               -1 &&
           error != 0);

end:
  last_chunk->set_rowcount(std::min(org_max_rows - max_rows, param->max_rows));
  last_chunk->set_file_position(to_start_filepos);

  return error;
} /* merge_buffers */

int unique_write_to_file(void *v_key, element_count, void *v_unique) {
  uchar *key = static_cast<uchar *>(v_key);
  Unique *unique = static_cast<Unique *>(v_unique);
  /*
    Use unique->size (size of element stored in the tree) and not
    unique->tree.size_of_element. The latter is different from unique->size
    when tree implementation chooses to store pointer to key in TREE_ELEMENT
    (instead of storing the element itself there)
  */
  return my_b_write(&unique->file, key, unique->size) ? 1 : 0;
}

int unique_write_to_ptrs(void *v_key, element_count, void *v_unique) {
  uchar *key = static_cast<uchar *>(v_key);
  Unique *unique = static_cast<Unique *>(v_unique);
  memcpy(unique->record_pointers, key, unique->size);
  unique->record_pointers += unique->size;
  return 0;
}

Unique::Unique(qsort2_cmp comp_func, void *comp_func_fixed_arg, uint size_arg,
               ulonglong max_in_memory_size_arg)
    : file_ptrs(PSI_INSTRUMENT_ME),
      max_elements(0),
      max_in_memory_size(max_in_memory_size_arg),
      record_pointers(nullptr),
      size(size_arg),
      elements(0) {
  my_b_clear(&file);
  init_tree(&tree,
            /* memory_limit */ 0, size, comp_func,
            /* with_delete */ false,
            /* free_element */ nullptr, comp_func_fixed_arg);
  /*
    If you change the following, change it in get_max_elements function, too.
  */
  max_elements =
      (ulong)(max_in_memory_size / ALIGN_SIZE(sizeof(TREE_ELEMENT) + size));
  (void)open_cached_file(&file, mysql_tmpdir, TEMP_PREFIX, DISK_BUFFER_SIZE,
                         MYF(MY_WME));
}

/**
  Calculate log2(n!)

  Stirling's approximate formula is used:

      n! ~= sqrt(2*M_PI*n) * (n/M_E)^n

  Derivation of formula used for calculations is as follows:

    log2(n!) = log2(sqrt(2*M_PI*n)*(n/M_E)^n)

      = log2(2*M_PI*n)/2 + n*log2(n/M_E)

      = log2(2*M_PI)/2 + log2(n)/2 + n * (log2(n) - M_LOG2E);

  @param n the number to calculate log2(n!) for

  @return log2(n!) for the function argument
*/

static inline double log2_n_fact(ulong n) {
  /*
    Stirling's approximation produces a small negative value when n is
    1 so we handle this as a special case in order to avoid negative
    numbers in estimates. For n equal to 0, the formula below will
    produce NaN. Since 0! by definition is 1, we return 0 for this
    case too.
  */
  if (n <= 1) return 0.0;

  const auto log2_n = std::log2(n);
  return std::log2(2 * M_PI) / 2 + log2_n / 2 + n * (log2_n - M_LOG2E);
}

/*
  Calculate cost of merge_buffers function call for given sequence of
  input stream lengths and store the number of rows in result stream in *last.

  SYNOPSIS
    get_merge_buffers_cost()
      buff_elems  Array of #s of elements in buffers
      elem_size   Size of element stored in buffer
      first       Pointer to first merged element size
      last        Pointer to last merged element size

  RETURN
    Cost of merge_buffers operation in disk seeks.

  NOTES
    It is assumed that no rows are eliminated during merge.
    The cost is calculated as

      cost(read_and_write) + cost(merge_comparisons).

    All bytes in the sequences is read and written back during merge so cost
    of disk io is 2*elem_size*total_buf_elems/IO_SIZE (2 is for read + write)

    For comparisons cost calculations we assume that all merged sequences have
    the same length, so each of total_buf_size elements will be added to a sort
    heap with (n_buffers-1) elements. This gives the comparison cost:

      key_compare_cost(total_buf_elems * log2(n_buffers));
*/

static double get_merge_buffers_cost(Unique::Imerge_cost_buf_type buff_elems,
                                     uint elem_size, uint first, uint last,
                                     const Cost_model_table *cost_model) {
  uint total_buf_elems = 0;
  for (uint pbuf = first; pbuf <= last; pbuf++)
    total_buf_elems += buff_elems[pbuf];
  buff_elems[last] = total_buf_elems;

  const size_t n_buffers = last - first + 1;

  const double io_ops =
      static_cast<double>(total_buf_elems * elem_size) / IO_SIZE;
  const double io_cost = cost_model->io_block_read_cost(io_ops);
  const double cpu_cost =
      cost_model->key_compare_cost(total_buf_elems * std::log2(n_buffers));

  return 2 * io_cost + cpu_cost;
}

/*
  Calculate cost of merging buffers into one in Unique::get, i.e. calculate
  how long (in terms of disk seeks) the two calls
    merge_many_buffs(...);
    merge_buffers(...);
  will take.

  SYNOPSIS
    get_merge_many_buffs_cost()
      buffer        buffer space for temporary data, at least
                    Unique::get_cost_calc_buff_size bytes
      maxbuffer     # of full buffers
      max_n_elems   # of elements in first maxbuffer buffers
      last_n_elems  # of elements in last buffer
      elem_size     size of buffer element

  NOTES
    maxbuffer+1 buffers are merged, where first maxbuffer buffers contain
    max_n_elems elements each and last buffer contains last_n_elems elements.

    The current implementation does a dumb simulation of merge_many_buffs
    function actions.

  RETURN
    Cost of merge in disk seeks.
*/

static double get_merge_many_buffs_cost(Unique::Imerge_cost_buf_type buffer,
                                        uint maxbuffer, uint max_n_elems,
                                        uint last_n_elems, int elem_size,
                                        const Cost_model_table *cost_model) {
  uint i;
  double total_cost = 0.0;
  Unique::Imerge_cost_buf_type buff_elems =
      buffer; /* #s of elements in each of merged sequences */

  /*
    Set initial state: first maxbuffer sequences contain max_n_elems elements
    each, last sequence contains last_n_elems elements.
  */
  for (i = 0; i < maxbuffer; i++) buff_elems[i] = max_n_elems;
  buff_elems[maxbuffer] = last_n_elems;

  /*
    Do it exactly as merge_many_buff function does, calling
    get_merge_buffers_cost to get cost of merge_buffers.
  */
  if (maxbuffer >= MERGEBUFF2) {
    while (maxbuffer >= MERGEBUFF2) {
      uint lastbuff = 0;
      for (i = 0; i <= maxbuffer - MERGEBUFF * 3U / 2U; i += MERGEBUFF) {
        total_cost += get_merge_buffers_cost(buff_elems, elem_size, i,
                                             i + MERGEBUFF - 1, cost_model);
        lastbuff++;
      }
      total_cost += get_merge_buffers_cost(buff_elems, elem_size, i, maxbuffer,
                                           cost_model);
      maxbuffer = lastbuff;
    }
  }

  /* Simulate final merge_buff call. */
  total_cost +=
      get_merge_buffers_cost(buff_elems, elem_size, 0, maxbuffer, cost_model);
  return total_cost;
}

/*
  Calculate cost of using Unique for processing nkeys elements of size
  key_size using max_in_memory_size memory.

  SYNOPSIS
    Unique::get_use_cost()
      buffer    space for temporary data, use Unique::get_cost_calc_buff_size
                to get # bytes needed.
      nkeys     #of elements in Unique
      key_size  size of each elements in bytes
      max_in_memory_size amount of memory Unique will be allowed to use

  RETURN
    Cost in disk seeks.

  NOTES
    cost(using_unqiue) =
      cost(create_trees) +  (see #1)
      cost(merge) +         (see #2)
      cost(read_result)     (see #3)

    1. Cost of trees creation
      For each Unique::put operation there will be 2*log2(n+1) elements
      comparisons, where n runs from 1 tree_size (we assume that all added
      elements are different). Together this gives:

      n_compares = 2*(log2(2) + log2(3) + ... + log2(N+1)) = 2*log2((N+1)!)

      then cost(tree_creation) = key_compare_cost(n_compares);

      Total cost of creating trees:
      (n_trees - 1)*max_size_tree_cost + non_max_size_tree_cost.

      Approximate value of log2(N!) is calculated by log2_n_fact function.

    2. Cost of merging.
      If only one tree is created by Unique no merging will be necessary.
      Otherwise, we model execution of merge_many_buff function and count
      #of merges. (The reason behind this is that number of buffers is small,
      while size of buffers is big and we don't want to loose precision with
      O(x)-style formula)

    3. If only one tree is created by Unique no disk io will happen.
      Otherwise, ceil(key_len*n_keys) disk seeks are necessary. We assume
      these will be random seeks.
*/

double Unique::get_use_cost(Imerge_cost_buf_type buffer, uint nkeys,
                            uint key_size, ulonglong max_in_memory_size,
                            const Cost_model_table *cost_model) {
  ulong max_elements_in_tree;
  ulong last_tree_elems;
  int n_full_trees; /* number of trees in unique - 1 */

  max_elements_in_tree =
      ((ulong)max_in_memory_size / ALIGN_SIZE(sizeof(TREE_ELEMENT) + key_size));

  n_full_trees = nkeys / max_elements_in_tree;
  last_tree_elems = nkeys % max_elements_in_tree;

  /* Calculate cost of creating trees */
  double n_compares = 2 * log2_n_fact(last_tree_elems + 1);
  if (n_full_trees)
    n_compares += n_full_trees * log2_n_fact(max_elements_in_tree + 1);
  double result = cost_model->key_compare_cost(n_compares);

  DBUG_PRINT("info",
             ("unique trees sizes: %u=%u*%lu + %lu", nkeys, n_full_trees,
              n_full_trees ? max_elements_in_tree : 0, last_tree_elems));

  if (!n_full_trees) return result;

  /*
    There is more then one tree and merging is necessary.
    First, add cost of writing all trees to disk, assuming that all disk
    writes are sequential.
  */
  result += cost_model->disk_seek_base_cost() * n_full_trees *
            ceil(((double)key_size) * max_elements_in_tree / IO_SIZE);
  result += cost_model->disk_seek_base_cost() *
            ceil(((double)key_size) * last_tree_elems / IO_SIZE);

  /* Cost of merge */
  double merge_cost =
      get_merge_many_buffs_cost(buffer, n_full_trees, max_elements_in_tree,
                                last_tree_elems, key_size, cost_model);
  if (merge_cost < 0.0) return merge_cost;

  result += merge_cost;
  /*
    Add cost of reading the resulting sequence, assuming there were no
    duplicate elements.
  */
  const double n_blocks = ceil((double)key_size * nkeys / IO_SIZE);
  result += cost_model->io_block_read_cost(n_blocks);

  return result;
}

Unique::~Unique() {
  close_cached_file(&file);
  delete_tree(&tree);
}

/* Write tree to disk; clear tree */
bool Unique::flush() {
  Merge_chunk file_ptr;
  elements += tree.elements_in_tree;
  file_ptr.set_rowcount(tree.elements_in_tree);
  file_ptr.set_file_position(my_b_tell(&file));

  if (tree_walk(&tree, unique_write_to_file, this, left_root_right) ||
      file_ptrs.push_back(file_ptr))
    return true; /* purecov: inspected */
  delete_tree(&tree);
  return false;
}

/*
  Clear the tree and the file.
  You must call reset() if you want to reuse Unique after walk().
*/

void Unique::reset() {
  reset_tree(&tree);
  /*
    If elements != 0, some trees were stored in the file (see how
    flush() works). Note, that we can not count on my_b_tell(&file) == 0
    here, because it can return 0 right after walk(), and walk() does not
    reset any Unique member.
  */
  if (elements) {
    file_ptrs.clear();
    reinit_io_cache(&file, WRITE_CACHE, 0L, false, true);
  }
  /*
    If table is used - finish index access and delete all records.
    use_table is set to false, so on the next run the memory tree will be used
    again at the beginning.
  */
  elements = 0;
}

/*
  The comparison function, used by the Priority_queue in merge_buffers()
  When the called from Uniques::get() must use comparison function of
  Uniques::tree, but compare members of struct Merge_chunk.
*/

static int merge_chunk_compare(Merge_chunk_compare_context *ctx,
                               uchar *key_ptr1, uchar *key_ptr2) {
  return ctx->key_compare(ctx->key_compare_arg, key_ptr1, key_ptr2);
}

namespace {

struct Merge_chunk_less {
  Merge_chunk_less(const Merge_chunk_compare_context context)
      : m_context(context) {}
  bool operator()(Merge_chunk *a, Merge_chunk *b) {
    return m_context.key_compare(m_context.key_compare_arg, a->current_key(),
                                 b->current_key()) > 0;
  }
  Merge_chunk_compare_context m_context;
};

}  // namespace

/*
  DESCRIPTION

    Function is very similar to merge_buffers, but instead of writing sorted
    unique keys to the output file, it invokes walk_action for each key.
    This saves I/O if you need to pass through all unique keys only once.

  SYNOPSIS
    merge_walk()
  All params are 'IN' (but see comment for begin, end):
    merge_buffer       buffer to perform cached piece-by-piece loading
                       of trees; initially the buffer is empty
    merge_buffer_size  size of merge_buffer. Must be aligned with
                       key_length
    key_length         size of tree element; key_length * (end - begin)
                       must be less or equal than merge_buffer_size.
    begin              pointer to Merge_chunk struct for the first tree.
    end                pointer to Merge_chunk struct for the last tree;
                       end > begin and [begin, end) form a consecutive
                       range. Merge_chunks structs in that range are used and
                       overwritten in merge_walk().
    walk_action        element visitor. Action is called for each unique
                       key.
    walk_action_arg    argument to walk action. Passed to it on each call.
    compare            elements comparison function
    compare_arg        comparison function argument
    file               file with all trees dumped. Trees in the file
                       must contain sorted unique values. Cache must be
                       initialized in read mode.
  RETURN VALUE
    0     ok
    <> 0  error
*/

static bool merge_walk(uchar *merge_buffer, size_t merge_buffer_size,
                       size_t key_length, Merge_chunk *begin, Merge_chunk *end,
                       tree_walk_action walk_action, void *walk_action_arg,
                       qsort2_cmp compare, const void *compare_arg,
                       IO_CACHE *file) {
  if (end <= begin ||
      merge_buffer_size < (ulong)(key_length * (end - begin + 1)))
    return true;

  Merge_chunk_compare_context compare_context = {compare, compare_arg};
  Priority_queue<Merge_chunk *,
                 std::vector<Merge_chunk *, Malloc_allocator<Merge_chunk *>>,
                 Merge_chunk_less>
      queue((Merge_chunk_less(compare_context)),
            (Malloc_allocator<Merge_chunk *>(key_memory_Unique_merge_buffer)));
  if (queue.reserve(end - begin)) return true;

  /* we need space for one key when a piece of merge buffer is re-read */
  merge_buffer_size -= key_length;
  uchar *save_key_buff = merge_buffer + merge_buffer_size;
  uint max_key_count_per_piece =
      (uint)(merge_buffer_size / (end - begin) / key_length);
  /* if piece_size is aligned reuse_freed_buffer will always hit */
  size_t piece_size = max_key_count_per_piece * key_length;
  uint bytes_read; /* to hold return value of uniq_read_to_buffer */
  Merge_chunk *top;
  int res = 1;

  // uniq_read_to_buffer() needs only rec_length.
  Uniq_param uniq_param;
  uniq_param.rec_length = key_length;

  /*
    Invariant: queue must contain top element from each tree, until a tree
    is not completely walked through.
    Here we're forcing the invariant, inserting one element from each tree
    to the queue.
  */
  for (top = begin; top != end; ++top) {
    top->set_buffer_start(merge_buffer + (top - begin) * piece_size);
    top->set_buffer_end(top->buffer_start() + piece_size);
    top->set_max_keys(max_key_count_per_piece);
    bytes_read = uniq_read_to_buffer(file, top, &uniq_param);
    if (bytes_read == (uint)(-1)) goto end;
    DBUG_ASSERT(bytes_read);
    queue.push(top);
  }
  top = queue.top();
  while (queue.size() > 1) {
    /*
      Every iteration one element is removed from the queue, and one is
      inserted by the rules of the invariant. If two adjacent elements on
      the top of the queue are not equal, biggest one is unique, because all
      elements in each tree are unique. Action is applied only to unique
      elements.
    */
    void *old_key = top->current_key();
    /*
      read next key from the cache or from the file and push it to the
      queue; this gives new top.
    */
    top->advance_current_key(key_length);
    top->decrement_mem_count();
    if (top->mem_count())
      queue.update_top();
    else /* next piece should be read */
    {
      /* save old_key not to overwrite it in uniq_read_to_buffer */
      memcpy(save_key_buff, old_key, key_length);
      old_key = save_key_buff;
      bytes_read = uniq_read_to_buffer(file, top, &uniq_param);
      if (bytes_read == (uint)(-1))
        goto end;
      else if (bytes_read > 0) /* top->key, top->mem_count are reset */
        queue.update_top();    /* in uniq_read_to_buffer */
      else {
        /*
          Tree for old 'top' element is empty: remove it from the queue and
          give all its memory to the nearest tree.
        */
        queue.pop();
        reuse_freed_buff(top, &queue);
      }
    }
    top = queue.top();
    /* new top has been obtained; if old top is unique, apply the action */
    if (compare(compare_arg, old_key, top->current_key())) {
      if (walk_action(old_key, 1, walk_action_arg)) goto end;
    }
  }
  /*
    Applying walk_action to the tail of the last tree: this is safe because
    either we had only one tree in the beginning, either we work with the
    last tree in the queue.
  */
  do {
    do {
      if (walk_action(top->current_key(), 1, walk_action_arg)) goto end;
      top->advance_current_key(key_length);
    } while (top->decrement_mem_count());
    bytes_read = uniq_read_to_buffer(file, top, &uniq_param);
    if (bytes_read == (uint)(-1)) goto end;
  } while (bytes_read);
  res = 0;
end:
  return res;
}

/*
  DESCRIPTION
    Walks consecutively through all unique elements:
    if all elements are in memory, then it simply invokes 'tree_walk', else
    all flushed trees are loaded to memory piece-by-piece, pieces are
    sorted, and action is called for each unique value.
    Note: so as merging resets file_ptrs state, this method can change
    internal Unique state to undefined: if you want to reuse Unique after
    walk() you must call reset() first!
  SYNOPSIS
    Unique:walk()
  All params are 'IN':
    action  function-visitor, typed in include/my_tree.h
            function is called for each unique element
    arg     argument for visitor, which is passed to it on each call
  RETURN VALUE
    0    OK
    <> 0 error
 */

bool Unique::walk(tree_walk_action action, void *walk_action_arg) {
  int res;
  uchar *merge_buffer;

  if (elements == 0) /* the whole tree is in memory */
    return tree_walk(&tree, action, walk_action_arg, left_root_right);

  /* flush current tree to the file to have some memory for merge buffer */
  if (flush()) return true;
  if (flush_io_cache(&file) ||
      reinit_io_cache(&file, READ_CACHE, 0L, false, false))
    return true;

  /*
    Compute the size of the merge buffer used by merge_walk(). This buffer
    must at least be able to store one element from each file pointer plus
    one extra.
  */
  const size_t min_merge_buffer_size = (file_ptrs.size() + 1) * size;
  const size_t merge_buffer_size =
      std::max(min_merge_buffer_size, static_cast<size_t>(max_in_memory_size));

  if (!(merge_buffer = (uchar *)my_malloc(key_memory_Unique_merge_buffer,
                                          merge_buffer_size, MYF(0))))
    return true;
  res = merge_walk(merge_buffer, merge_buffer_size, size, file_ptrs.begin(),
                   file_ptrs.end(), action, walk_action_arg, tree.compare,
                   tree.custom_arg, &file);
  my_free(merge_buffer);
  return res;
}

/*
  Modify the TABLE element so that when one calls init_records()
  the rows will be read in priority order.
*/

bool Unique::get(TABLE *table) {
  table->unique_result.found_records = elements + tree.elements_in_tree;

  if (my_b_tell(&file) == 0) {
    /* Whole tree is in memory;  Don't use disk if you don't need to */
    DBUG_ASSERT(table->unique_result.sorted_result == nullptr);
    table->unique_result.sorted_result.reset(
        (uchar *)my_malloc(key_memory_Filesort_info_record_pointers,
                           size * tree.elements_in_tree, MYF(0)));
    if ((record_pointers = table->unique_result.sorted_result.get())) {
      (void)tree_walk(&tree, unique_write_to_ptrs, this, left_root_right);
      return false;
    }
  }
  /* Not enough memory; Save the result to file && free memory used by tree */
  if (flush()) return true;

  IO_CACHE *outfile = table->unique_result.io_cache;
  Merge_chunk *file_ptr = file_ptrs.begin();
  size_t num_chunks = file_ptrs.size();
  uchar *sort_memory;
  my_off_t save_pos;
  bool error = true;

  /* Open cached file if it isn't open */
  DBUG_ASSERT(table->unique_result.io_cache == nullptr);
  outfile = table->unique_result.io_cache = (IO_CACHE *)my_malloc(
      key_memory_TABLE_sort_io_cache, sizeof(IO_CACHE), MYF(MY_ZEROFILL));

  if (!outfile || (!my_b_inited(outfile) &&
                   open_cached_file(outfile, mysql_tmpdir, TEMP_PREFIX,
                                    READ_RECORD_BUFFER, MYF(MY_WME))))
    return true;
  reinit_io_cache(outfile, WRITE_CACHE, 0L, false, false);

  Uniq_param uniq_param;
  uniq_param.max_rows = elements;
  uniq_param.rec_length = size;
  uniq_param.max_keys_per_buffer =
      static_cast<uint>(max_in_memory_size / uniq_param.rec_length);
  uniq_param.not_killable = true;

  const size_t num_bytes =
      (uniq_param.max_keys_per_buffer + 1) * uniq_param.rec_length;
  if (!(sort_memory = (uchar *)my_malloc(key_memory_Unique_sort_buffer,
                                         num_bytes, MYF(0))))
    return true;
  uniq_param.unique_buff =
      sort_memory + (uniq_param.max_keys_per_buffer * uniq_param.rec_length);

  uniq_param.compare = merge_chunk_compare;
  uniq_param.cmp_context.key_compare = tree.compare;
  uniq_param.cmp_context.key_compare_arg = tree.custom_arg;

  /* Merge the buffers to one file, removing duplicates */
  if (merge_many_buff(table->in_use, &uniq_param,
                      Sort_buffer(sort_memory, num_bytes),
                      Merge_chunk_array(file_ptrs.begin(), file_ptrs.size()),
                      &num_chunks, &file))
    goto err;
  if (flush_io_cache(&file) ||
      reinit_io_cache(&file, READ_CACHE, 0L, false, false))
    goto err;
  if (merge_buffers(table->in_use, &uniq_param, &file, outfile,
                    Sort_buffer(sort_memory, num_bytes), file_ptr,
                    Merge_chunk_array(file_ptr, num_chunks), 0))
    goto err;
  error = false;
err:
  my_free(sort_memory);
  if (flush_io_cache(outfile)) error = true;

  /* Setup io_cache for reading */
  save_pos = outfile->pos_in_file;
  if (reinit_io_cache(outfile, READ_CACHE, 0L, false, false)) error = true;
  outfile->end_of_file = save_pos;
  return error;
}

bool Unique_on_insert::unique_add(void *ptr) {
  Field *key = *m_table->visible_field_ptr();
  if (key->store((const char *)ptr, m_size, &my_charset_bin) != TYPE_OK)
    return true; /* purecov: inspected */
  if (!check_unique_constraint(m_table)) return true;
  uint res = m_table->file->ha_write_row(m_table->record[0]);
  if (res) {
    bool dup = false;
    if (res == HA_ERR_FOUND_DUPP_KEY) return true;
    if (create_ondisk_from_heap(m_table->in_use, m_table, res, false, &dup) ||
        dup)
      return true;
  }
  return false;
}

void Unique_on_insert::reset(bool reinit) {
  /* Finish index access and delete all records.  */
  m_table->file->ha_index_or_rnd_end();
  m_table->file->ha_delete_all_rows();
  if (reinit) m_table->file->ha_index_init(0, true);
}

bool Unique_on_insert::init() {
  // If it's first run and table haven't been created yet - create it
  if (!m_table && !(m_table = create_duplicate_weedout_tmp_table(
                        current_thd, m_size, nullptr)))
    return true; /* purecov: inspected */
  return false;
}

void Unique_on_insert::cleanup() {
  reset(false);
  free_tmp_table(m_table->in_use, m_table);
}
