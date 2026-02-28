/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file sql/filesort.cc

  Standard external sort. We read rows into a buffer until there's no more room.
  At that point, we use it (using the sorting algorithms from STL), and write it
  to disk (thus the name “filesort”). When there are no more rows, we merge
  chunks recursively, seven and seven (although we can go all the way up to 15
  in the final pass if it helps us do one pass less).

  If all the rows fit in a single chunk, the data never hits disk, but remains
  in RAM.
 */

#include "sql/filesort.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <new>
#include <vector>

#include "add_with_saturate.h"
#include "decimal.h"
#include "field_types.h"  // enum_field_types
#include "m_ctype.h"
#include "map_helpers.h"
#include "my_basename.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_config.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "nullable.h"
#include "priority_queue.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/bounded_queue.h"
#include "sql/cmp_varlen_keys.h"
#include "sql/debug_sync.h"
#include "sql/derror.h"
#include "sql/error_handler.h"
#include "sql/field.h"
#include "sql/filesort_utils.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_subselect.h"
#include "sql/json_dom.h"  // Json_wrapper
#include "sql/key_spec.h"
#include "sql/malloc_allocator.h"
#include "sql/merge_many_buff.h"
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // mysql_tmpdir
#include "sql/opt_costmodel.h"
#include "sql/opt_trace.h"
#include "sql/opt_trace_context.h"
#include "sql/pfs_batch_mode.h"
#include "sql/psi_memory_key.h"
#include "sql/row_iterator.h"
#include "sql/sort_param.h"
#include "sql/sorting_iterator.h"
#include "sql/sql_array.h"
#include "sql/sql_base.h"
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_sort.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thr_malloc.h"
#include "sql/tztime.h"
#include "sql_string.h"
#include "template_utils.h"

using Mysql::Nullable;
using std::max;
using std::min;

namespace {

struct Mem_compare_queue_key {
  Mem_compare_queue_key() : m_compare_length(0), m_param(nullptr) {}

  Mem_compare_queue_key(const Mem_compare_queue_key &that)
      : m_compare_length(that.m_compare_length), m_param(that.m_param) {}

  bool operator()(const uchar *s1, const uchar *s2) const {
    if (m_param)
      return cmp_varlen_keys(m_param->local_sortorder, m_param->use_hash, s1,
                             s2);

    // memcmp(s1, s2, 0) is guaranteed to return zero.
    return memcmp(s1, s2, m_compare_length) < 0;
  }

  size_t m_compare_length;
  Sort_param *m_param;
};

}  // namespace

/* functions defined in this file */

static ha_rows read_all_rows(
    THD *thd, Sort_param *param, Filesort_info *fs_info, IO_CACHE *buffer_file,
    IO_CACHE *chunk_file,
    Bounded_queue<uchar *, uchar *, Sort_param, Mem_compare_queue_key> *pq,
    RowIterator *source_iterator, ha_rows *found_rows, size_t *longest_key,
    size_t *longest_addons);
static int write_keys(Sort_param *param, Filesort_info *fs_info, uint count,
                      IO_CACHE *buffer_file, IO_CACHE *tempfile);
static int merge_index(THD *thd, Sort_param *param, Sort_buffer sort_buffer,
                       Merge_chunk_array chunk_array, IO_CACHE *tempfile,
                       IO_CACHE *outfile);
static bool save_index(Sort_param *param, uint count, Filesort_info *table_sort,
                       Sort_result *sort_result);

static bool check_if_pq_applicable(Opt_trace_context *trace, Sort_param *param,
                                   Filesort_info *info, ha_rows records,
                                   ulong memory_available);

void Sort_param::decide_addon_fields(Filesort *file_sort, TABLE *table,
                                     bool force_sort_positions) {
  if (m_addon_fields_status != Addon_fields_status::unknown_status) {
    // Already decided.
    return;
  }

  DBUG_EXECUTE_IF("filesort_force_sort_row_ids", {
    m_addon_fields_status = Addon_fields_status::keep_rowid;
    return;
  });

  // Generally, prefer using addon fields (ie., sorting rows instead of just
  // row IDs) if we can.
  //
  // NOTE: If the table is already in memory (e.g. the MEMORY engine; see the
  // HA_FAST_KEY_READ flag), it would normally be beneficial to sort row IDs
  // over rows to get smaller sort chunks. However, eliding the temporary table
  // entirely is even better than using row IDs, and only possible if we sort
  // rows. Furthermore, since we set up filesort at optimize time, we don't know
  // yet whether the source table would _remain_ in memory, or could be spilled
  // to disk using InnoDB. Thus, the only case that reasonably remains where
  // we'd want to use row IDs without being forced would be on user (ie.,
  // non-temporary) MEMORY tables, which doesn't seem reasonable to add
  // complexity for.

  if (table->fulltext_searched) {
    // MATCH() (except in “boolean mode”) doesn't use the actual value, it just
    // goes and asks the handler directly for the current row. Thus, we need row
    // IDs, so that the row is positioned correctly.
    //
    // When sorting a join, table->fulltext_searched will be false, but items
    // (like Item_func_match) are materialized (by StreamingIterator or
    // MaterializeIterator) before the sort, so this is moot.
    m_addon_fields_status = Addon_fields_status::fulltext_searched;
  } else if (force_sort_positions) {
    m_addon_fields_status = Addon_fields_status::keep_rowid;
  } else {
    /*
      Get the descriptors of all fields whose values are appended
      to sorted fields and get its total length in m_addon_length.
    */
    addon_fields = file_sort->get_addon_fields(
        table, &m_addon_fields_status, &m_addon_length, &m_packable_length);
  }
}

void Sort_param::init_for_filesort(Filesort *file_sort,
                                   Bounds_checked_array<st_sort_field> sf_array,
                                   uint sortlen, TABLE *table, ha_rows maxrows,
                                   bool remove_duplicates) {
  m_fixed_sort_length = sortlen;
  m_force_stable_sort = file_sort->m_force_stable_sort;
  m_remove_duplicates = remove_duplicates;
  ref_length = table->file->ref_length;

  local_sortorder = sf_array;

  decide_addon_fields(file_sort, table, file_sort->m_force_sort_positions);
  if (using_addon_fields()) {
    fixed_res_length = m_addon_length;
  } else {
    fixed_res_length = ref_length;
    /*
      The reference to the record is considered
      as an additional sorted field
    */
    AddWithSaturate(ref_length, &m_fixed_sort_length);
  }

  m_num_varlen_keys = count_varlen_keys();
  m_num_json_keys = count_json_keys();
  if (using_varlen_keys()) {
    AddWithSaturate(size_of_varlength_field, &m_fixed_sort_length);
  }
  /*
    Add hash at the end of sort key to order cut values correctly.
    Needed for GROUPing, rather than for ORDERing.
  */
  if (using_json_keys()) {
    use_hash = true;
    AddWithSaturate(sizeof(ulonglong), &m_fixed_sort_length);
  }

  m_fixed_rec_length = AddWithSaturate(m_fixed_sort_length, m_addon_length);
  max_rows = maxrows;
}

void Sort_param::try_to_pack_addons() {
  if (!using_addon_fields() ||  // no addons, or
      using_packed_addons())    // already packed
    return;

  // Heuristic: skip packing if potential savings are less than 10 bytes.
  const uint sz = Addon_fields::size_of_length_field;
  if (m_packable_length < (10 + sz)) {
    m_addon_fields_status = Addon_fields_status::skip_heuristic;
    return;
  }

  Addon_fields_array::iterator addonf = addon_fields->begin();
  for (; addonf != addon_fields->end(); ++addonf) {
    addonf->offset += sz;
    addonf->null_offset += sz;
  }
  addon_fields->set_using_packed_addons(true);
  m_using_packed_addons = true;

  AddWithSaturate(sz, &m_addon_length);
  AddWithSaturate(sz, &fixed_res_length);
  AddWithSaturate(sz, &m_fixed_rec_length);
}

int Sort_param::count_varlen_keys() const {
  int retval = 0;
  for (const auto &sf : local_sortorder) {
    if (sf.is_varlen) {
      ++retval;
    }
  }
  return retval;
}

int Sort_param::count_json_keys() const {
  int retval = 0;
  for (const auto &sf : local_sortorder) {
    if (sf.field_type == MYSQL_TYPE_JSON) {
      ++retval;
    }
  }
  return retval;
}

size_t Sort_param::get_record_length(uchar *p) const {
  uchar *start_of_payload = get_start_of_payload(p);
  uint size_of_payload = using_packed_addons()
                             ? Addon_fields::read_addon_length(start_of_payload)
                             : fixed_res_length;
  uchar *end_of_payload = start_of_payload + size_of_payload;
  return end_of_payload - p;
}

void Sort_param::get_rec_and_res_len(uchar *record_start, uint *recl,
                                     uint *resl) {
  if (!using_packed_addons() && !using_varlen_keys()) {
    *recl = m_fixed_rec_length;
    *resl = fixed_res_length;
    return;
  }
  uchar *plen = get_start_of_payload(record_start);
  if (using_packed_addons())
    *resl = Addon_fields::read_addon_length(plen);
  else
    *resl = fixed_res_length;
  DBUG_ASSERT(*resl <= fixed_res_length);
  const uchar *record_end = plen + *resl;
  *recl = static_cast<uint>(record_end - record_start);
}

static void trace_filesort_information(Opt_trace_context *trace,
                                       const st_sort_field *sortorder,
                                       uint s_length) {
  if (!trace->is_started()) return;

  Opt_trace_array trace_filesort(trace, "filesort_information");
  for (; s_length--; sortorder++) {
    Opt_trace_object oto(trace);
    oto.add_alnum("direction", sortorder->reverse ? "desc" : "asc");
    oto.add("expression", sortorder->item);
  }
}

static int filesort_post_write(IO_CACHE *cache) {
  THD *thd = current_thd;
  my_off_t prior_usage = cache->reported_disk_usage;
  my_off_t current_usage = cache->pos_in_file;
  Filesort_info *fs_info = reinterpret_cast<Filesort_info *>(cache->arg);

  if (fs_info && thd->variables.filesort_max_file_size > 0 &&
      current_usage > thd->variables.filesort_max_file_size) {
#ifndef DBUG_OFF
    fs_info->file_size_exceeded = true;
#endif
    my_error(ER_FILESORT_MAX_FILE_SIZE_EXCEEDED, MYF(0));
    cache->error = ER_FILESORT_MAX_FILE_SIZE_EXCEEDED;
  } else if (is_tmp_disk_usage_over_max()) {
    my_error(ER_MAX_TMP_DISK_USAGE_EXCEEDED, MYF(0));
    cache->error = ER_MAX_TMP_DISK_USAGE_EXCEEDED;
  }

  if (current_usage > prior_usage) {
    thd->adjust_filesort_disk_usage(current_usage - prior_usage);
    cache->reported_disk_usage = current_usage;
  }

  return cache->error;
}

static int filesort_pre_close(IO_CACHE *cache) {
  if (cache->reported_disk_usage) {
    current_thd->adjust_filesort_disk_usage(-cache->reported_disk_usage);
    cache->reported_disk_usage = 0;
  }

  return 0;
}

bool filesort_open_cached_file(IO_CACHE *cache, const char *dir,
                               const char *prefix, size_t cache_size,
                               myf cache_myflags, void *fs_info) {
  bool result = open_cached_file(cache, dir, prefix, cache_size, cache_myflags);
  cache->post_write = filesort_post_write;
  cache->pre_close = filesort_pre_close;
  cache->arg = fs_info;
  return result;
}

/**
  Sort a table.
  Creates a set of pointers that can be used to read the rows
  in sorted order. This should be done with the functions
  in records.cc.

  The result set is stored in fs_info->io_cache or
  fs_info->sorted_result, or left in the main filesort buffer.

  @param      thd            Current thread
  @param      filesort       How to sort the table
  @param      source_iterator Where to read the rows to be sorted from.
  @param      fs_info        Owns the buffers for sort_result.
  @param      sort_result    Where to store the sort result.
  @param[out] found_rows     Store the number of found rows here.
                             This is the number of found rows after
                             applying WHERE condition.

  @note
    If we sort by position (like if sort_positions is 1) filesort() will
    call table->prepare_for_position().

  @returns   False if success, true if error
*/

bool filesort(THD *thd, Filesort *filesort, RowIterator *source_iterator,
              Filesort_info *fs_info, Sort_result *sort_result,
              ha_rows *found_rows) {
  int error;
  ulong memory_available = thd->variables.sortbuff_size;
  ha_rows num_rows_found = HA_POS_ERROR;
  ha_rows num_rows_estimate = HA_POS_ERROR;
  IO_CACHE tempfile;    // Temporary file for storing intermediate results.
  IO_CACHE chunk_file;  // For saving Merge_chunk structs.
  IO_CACHE *outfile;    // Contains the final, sorted result.
  Sort_param *param = &filesort->m_sort_param;
  TABLE *const table = filesort->table;
  ha_rows max_rows = filesort->limit;
  uint s_length = 0;

  DBUG_TRACE;
  // 'pushed_join' feature need to read the partial joined results directly
  // from the NDB API. First storing it into a temporary table, means that
  // any joined child results are effectively wasted, and we will have to
  // re-read them as non-pushed later.
  DBUG_ASSERT(!table->file->member_of_pushed_join());

  if (!(s_length = filesort->sort_order_length()))
    return true; /* purecov: inspected */

  DBUG_ASSERT(!table->reginfo.join_tab);

  DEBUG_SYNC(thd, "filesort_start");

  DBUG_ASSERT(sort_result->sorted_result == nullptr);
  sort_result->sorted_result_in_fsbuf = false;

  outfile = sort_result->io_cache;
  my_b_clear(&tempfile);
  my_b_clear(&chunk_file);
  error = 1;

  // Make sure the source iterator is initialized before init_for_filesort(),
  // since table->file (and in particular, ref_length) may not be initialized
  // before that.
  DBUG_EXECUTE_IF("bug14365043_1", DBUG_SET("+d,ha_rnd_init_fail"););
  if (source_iterator->Init()) {
    return HA_POS_ERROR;
  }

  /*
    We need a nameless wrapper, since we may be inside the "steps" of
    "join_execution".
  */
  Opt_trace_context *const trace = &thd->opt_trace;
  Opt_trace_object trace_wrapper(trace);
  trace_wrapper.add_alnum("sorting_table", table->alias);

  trace_filesort_information(trace, filesort->sortorder, s_length);

  param->init_for_filesort(filesort, make_array(filesort->sortorder, s_length),
                           sortlength(thd, filesort->sortorder, s_length),
                           table, max_rows, filesort->m_remove_duplicates);

  fs_info->addon_fields = param->addon_fields;
#ifndef DBUG_OFF
  fs_info->file_size_exceeded = false;
#endif /* DBUG_OFF */

  thd->inc_status_sort_scan();

  if (table->file->inited) {
    if (table->s->tmp_table)
      table->file->info(HA_STATUS_VARIABLE);  // Get record count
    num_rows_estimate = table->file->estimate_rows_upper_bound();
  } else {
    // If number of rows is not known, use as much of sort buffer as possible.
    num_rows_estimate = HA_POS_ERROR;
  }

  Bounded_queue<uchar *, uchar *, Sort_param, Mem_compare_queue_key> pq(
      param->max_record_length(),
      (Malloc_allocator<uchar *>(key_memory_Filesort_info_record_pointers)));

  // We would have liked to do this ahead-of-time so that we can show it
  // in EXPLAIN. However, we don't know the table->file->ref_length before
  // sorting time, which makes it hard to make the decision if we're row IDs.
  // (If we sort rows, we would know, but it's not much use knowing it
  // ahead-of-time _sometimes_.)
  //
  // However, do note this cannot change the addon fields status,
  // so that we at least know that when checking whether we can skip
  // in-between temporary tables (StreamingIterator).
  if (check_if_pq_applicable(trace, param, fs_info, num_rows_estimate,
                             memory_available)) {
    DBUG_PRINT("info", ("filesort PQ is applicable"));
    /*
      For PQ queries (with limit) we know exactly how many pointers/records
      we have in the buffer, so to simplify things, we initialize
      all pointers here. (We cannot pack fields anyways, so there is no
      point in doing incremental allocation).
     */
    if (fs_info->preallocate_records(param->max_rows_per_buffer)) {
      my_error(ER_OUT_OF_SORTMEMORY, ME_FATALERROR);
      LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_SORTMEMORY);
      goto err;
    }

    if (pq.init(param->max_rows, param, fs_info->get_sort_keys())) {
      /*
       If we fail to init pq, we have to give up:
       out of memory means my_malloc() will call my_error().
      */
      DBUG_PRINT("info", ("failed to allocate PQ"));
      fs_info->free_sort_buffer();
      DBUG_ASSERT(thd->is_error());
      goto err;
    }
    filesort->using_pq = true;
    param->using_pq = true;
    param->m_addon_fields_status = Addon_fields_status::using_priority_queue;
  } else {
    DBUG_PRINT("info", ("filesort PQ is not applicable"));
    filesort->using_pq = false;
    param->using_pq = false;

    /*
      When sorting using priority queue, we cannot use packed addons.
      Without PQ, we can try.
    */
    param->try_to_pack_addons();

    /*
      NOTE: param->max_rows_per_buffer is merely informative (for optimizer
      trace) in this case, not actually used.
    */
    if (num_rows_estimate < MERGEBUFF2) num_rows_estimate = MERGEBUFF2;
    ha_rows keys =
        memory_available / (param->max_record_length() + sizeof(char *));
    param->max_rows_per_buffer =
        min(num_rows_estimate > 0 ? num_rows_estimate : 1, keys);

    fs_info->set_max_size(memory_available, param->max_record_length());
  }

  param->sort_form = table;
  size_t longest_key, longest_addons;
  longest_addons = 0;

  // New scope, because subquery execution must be traced within an array.
  {
    Opt_trace_array ota(trace, "filesort_execution");
    num_rows_found =
        read_all_rows(thd, param, fs_info, &chunk_file, &tempfile,
                      param->using_pq ? &pq : nullptr, source_iterator,
                      found_rows, &longest_key, &longest_addons);
    if (num_rows_found == HA_POS_ERROR) goto err;
  }

  size_t num_chunks, num_initial_chunks;
  if (my_b_inited(&chunk_file)) {
    num_chunks =
        static_cast<size_t>(my_b_tell(&chunk_file)) / sizeof(Merge_chunk);
  } else {
    num_chunks = 0;
  }

  num_initial_chunks = num_chunks;

  if (num_chunks == 0)  // The whole set is in memory
  {
    ha_rows rows_in_chunk =
        param->using_pq ? pq.num_elements() : num_rows_found;
    if (save_index(param, rows_in_chunk, fs_info, sort_result)) goto err;
  } else {
    // If deduplicating, we'll need to remember the previous key somehow.
    if (filesort->m_remove_duplicates) {
      param->m_last_key_seen =
          static_cast<uchar *>(thd->mem_root->Alloc(longest_key));
    }

    // We will need an extra buffer in SortFileIndirectIterator
    if (fs_info->addon_fields != nullptr &&
        !(fs_info->addon_fields->allocate_addon_buf(longest_addons)))
      goto err; /* purecov: inspected */

    fs_info->read_chunk_descriptors(&chunk_file, num_chunks);
    if (fs_info->merge_chunks.is_null()) goto err; /* purecov: inspected */

    close_cached_file(&chunk_file);

    /* Open cached file if it isn't open */
    if (!my_b_inited(outfile) &&
        filesort_open_cached_file(outfile, mysql_tmpdir, TEMP_PREFIX,
                                  READ_RECORD_BUFFER, MYF(MY_WME)))
      goto err;
    if (reinit_io_cache(outfile, WRITE_CACHE, 0L, false, false)) goto err;

    param->max_rows_per_buffer = static_cast<uint>(
        fs_info->max_size_in_bytes() / param->max_record_length());

    Bounds_checked_array<uchar> merge_buf = fs_info->get_contiguous_buffer();
    if (merge_buf.array() == nullptr) {
      my_error(ER_OUT_OF_SORTMEMORY, ME_FATALERROR);
      LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_SORTMEMORY);
      goto err;
    }
    if (merge_many_buff(thd, param, merge_buf, fs_info->merge_chunks,
                        &num_chunks, &tempfile))
      goto err;
    if (flush_io_cache(&tempfile) ||
        reinit_io_cache(&tempfile, READ_CACHE, 0L, false, false))
      goto err;
    if (merge_index(
            thd, param, merge_buf,
            Merge_chunk_array(fs_info->merge_chunks.begin(), num_chunks),
            &tempfile, outfile))
      goto err;

    sort_result->found_records = num_rows_found;
  }

  if (trace->is_started()) {
    char buffer[100];
    String sort_mode(buffer, sizeof(buffer), &my_charset_bin);
    sort_mode.length(0);
    sort_mode.append("<");
    if (param->using_varlen_keys())
      sort_mode.append("varlen_sort_key");
    else
      sort_mode.append("fixed_sort_key");
    sort_mode.append(", ");
    sort_mode.append(param->using_packed_addons()
                         ? "packed_additional_fields"
                         : param->using_addon_fields() ? "additional_fields"
                                                       : "rowid");
    sort_mode.append(">");

    const char *algo_text[] = {"none", "std::sort", "std::stable_sort"};

    Opt_trace_object filesort_summary(trace, "filesort_summary");
    filesort_summary.add("memory_available", memory_available)
        .add("key_size", param->max_compare_length())
        .add("row_size", param->max_record_length())
        .add("max_rows_per_buffer", param->max_rows_per_buffer)
        .add("num_rows_estimate", num_rows_estimate)
        .add("num_rows_found", num_rows_found)
        .add("num_initial_chunks_spilled_to_disk", num_initial_chunks)
        .add("peak_memory_used", fs_info->peak_memory_used())
        .add_alnum("sort_algorithm", algo_text[param->m_sort_algorithm]);
    if (!param->using_packed_addons())
      filesort_summary.add_alnum(
          "unpacked_addon_fields",
          addon_fields_text(param->m_addon_fields_status));
    filesort_summary.add_alnum("sort_mode", sort_mode.c_ptr());
  }

  if (num_rows_found > param->max_rows) {
    // If read_all_rows() produced more results than the query LIMIT.
    num_rows_found = param->max_rows;
  }
  error = 0;

err:
  if (!filesort->keep_buffers) {
    if (!sort_result->sorted_result_in_fsbuf) fs_info->free_sort_buffer();
    my_free(fs_info->merge_chunks.array());
    fs_info->merge_chunks = Merge_chunk_array(nullptr, 0);
  }
  close_cached_file(&tempfile);
  close_cached_file(&chunk_file);
  if (my_b_inited(outfile)) {
    if (flush_io_cache(outfile)) error = 1;
    {
      my_off_t save_pos = outfile->pos_in_file;
      /* For following reads */
      if (reinit_io_cache(outfile, READ_CACHE, 0L, false, false)) error = 1;
      outfile->end_of_file = save_pos;
    }
  }
  if (error) {
    DBUG_ASSERT(thd->is_error() || thd->killed || fs_info->file_size_exceeded);
  } else
    thd->inc_status_sort_rows(num_rows_found);

  return error;
} /* filesort */

void filesort_free_buffers(TABLE *table, bool full) {
  DBUG_TRACE;

  table->unique_result.sorted_result.reset();
  DBUG_ASSERT(!table->unique_result.sorted_result_in_fsbuf);
  table->unique_result.sorted_result_in_fsbuf = false;

  if (full) {
    if (table->sorting_iterator != nullptr) {
      table->sorting_iterator->CleanupAfterQuery();
    }
    if (table->duplicate_removal_iterator != nullptr) {
      table->duplicate_removal_iterator->CleanupAfterQuery();
    }
  }
}

Filesort::Filesort(THD *thd, TABLE *table_arg, bool keep_buffers_arg,
                   ORDER *order, ha_rows limit_arg, bool force_stable_sort,
                   bool remove_duplicates, bool sort_positions)
    : m_thd(thd),
      table(table_arg),
      keep_buffers(keep_buffers_arg),
      limit(limit_arg),
      sortorder(nullptr),
      using_pq(false),
      m_force_stable_sort(
          force_stable_sort),  // keep relative order of equiv. elts
      m_remove_duplicates(remove_duplicates),
      m_force_sort_positions(sort_positions),
      m_sort_order_length(make_sortorder(order)) {}

uint Filesort::make_sortorder(ORDER *order) {
  uint count;
  st_sort_field *sort, *pos;
  ORDER *ord;
  DBUG_TRACE;

  count = 0;
  for (ord = order; ord; ord = ord->next) count++;
  DBUG_ASSERT(count > 0);

  const size_t sortorder_size = sizeof(*sortorder) * (count + 1);
  if (sortorder == nullptr)
    sortorder =
        static_cast<st_sort_field *>((*THR_MALLOC)->Alloc(sortorder_size));
  if (sortorder == nullptr) return 0; /* purecov: inspected */
  memset(sortorder, 0, sortorder_size);

  pos = sort = sortorder;
  for (ord = order; ord; ord = ord->next, pos++) {
    Item *const item = ord->item[0], *const real_item = item->real_item();
    if (real_item->type() == Item::COPY_STR_ITEM) {  // Blob patch
      pos->item = static_cast<Item_copy *>(real_item)->get_item();
    } else
      pos->item = real_item;
    pos->reverse = (ord->direction == ORDER_DESC);
  }
  return count;
}

void Filesort_info::read_chunk_descriptors(IO_CACHE *chunk_file, uint count) {
  DBUG_TRACE;

  // If we already have a chunk array, we're doing sort in a subquery.
  if (!merge_chunks.is_null() && merge_chunks.size() < count) {
    my_free(merge_chunks.array());                /* purecov: inspected */
    merge_chunks = Merge_chunk_array(nullptr, 0); /* purecov: inspected */
  }

  void *rawmem = merge_chunks.array();
  const size_t length = sizeof(Merge_chunk) * count;
  if (nullptr == rawmem) {
    rawmem = my_malloc(key_memory_Filesort_info_merge, length, MYF(MY_WME));
    if (rawmem == nullptr) return; /* purecov: inspected */
  }

  if (reinit_io_cache(chunk_file, READ_CACHE, 0L, false, false) ||
      my_b_read(chunk_file, static_cast<uchar *>(rawmem), length)) {
    my_free(rawmem);  /* purecov: inspected */
    rawmem = nullptr; /* purecov: inspected */
    count = 0;        /* purecov: inspected */
  }

  merge_chunks = Merge_chunk_array(static_cast<Merge_chunk *>(rawmem), count);
}

#ifndef DBUG_OFF
/*
  Print a text, SQL-like record representation into dbug trace.

  Note: this function is a work in progress: at the moment
   - column read bitmap is ignored (can print garbage for unused columns)
   - there is no quoting
*/
static void dbug_print_record(TABLE *table, bool print_rowid) {
  char buff[1024];
  Field **pfield;
  String tmp(buff, sizeof(buff), &my_charset_bin);
  DBUG_LOCK_FILE;

  fprintf(DBUG_FILE, "record (");
  for (pfield = table->field; *pfield; pfield++)
    fprintf(DBUG_FILE, "%s%s", (*pfield)->field_name, (pfield[1]) ? ", " : "");
  fprintf(DBUG_FILE, ") = ");

  fprintf(DBUG_FILE, "(");
  for (pfield = table->field; *pfield; pfield++) {
    Field *field = *pfield;

    if (field->is_null()) {
      if (fwrite("NULL", sizeof(char), 4, DBUG_FILE) != 4) {
        goto unlock_file_and_quit;
      }
    }

    if (field->type() == MYSQL_TYPE_BIT)
      (void)field->val_int_as_str(&tmp, true);
    else
      field->val_str(&tmp);

    if (fwrite(tmp.ptr(), sizeof(char), tmp.length(), DBUG_FILE) !=
        tmp.length()) {
      goto unlock_file_and_quit;
    }

    if (pfield[1]) {
      if (fwrite(", ", sizeof(char), 2, DBUG_FILE) != 2) {
        goto unlock_file_and_quit;
      }
    }
  }
  fprintf(DBUG_FILE, ")");
  if (print_rowid) {
    fprintf(DBUG_FILE, " rowid ");
    for (uint i = 0; i < table->file->ref_length; i++) {
      fprintf(DBUG_FILE, "%x", table->file->ref[i]);
    }
  }
  fprintf(DBUG_FILE, "\n");
unlock_file_and_quit:
  DBUG_UNLOCK_FILE;
}
#endif

/// Error handler for filesort.
class Filesort_error_handler : public Internal_error_handler {
  THD *m_thd;                 ///< The THD in which filesort is executed.
  bool m_seen_not_supported;  ///< Has a not supported warning has been seen?
 public:
  /**
    Create an error handler and push it onto the error handler
    stack. The handler will be automatically popped from the error
    handler stack when it is destroyed.
  */
  Filesort_error_handler(THD *thd) : m_thd(thd), m_seen_not_supported(false) {
    thd->push_internal_handler(this);
  }

  /**
    Pop the error handler from the error handler stack, and destroy
    it.
  */
  ~Filesort_error_handler() { m_thd->pop_internal_handler(); }

  /**
    Handle a condition.

    The handler will make sure that no more than a single
    ER_NOT_SUPPORTED_YET warning will be seen by the higher
    layers. This warning is generated by Json_wrapper::make_sort_key()
    for every value that it doesn't know how to create a sort key
    for. It is sufficient for the higher layers to report this warning
    only once per sort.
  */
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *) {
    if (*level == Sql_condition::SL_WARNING &&
        sql_errno == ER_NOT_SUPPORTED_YET) {
      if (m_seen_not_supported) return true;
      m_seen_not_supported = true;
    }

    return false;
  }
};

static bool alloc_and_make_sortkey(Sort_param *param, Filesort_info *fs_info,
                                   uchar *ref_pos, size_t *key_length,
                                   size_t *longest_addons) {
  size_t min_bytes = 1;
  for (;;) {  // Termination condition within loop.
    Bounds_checked_array<uchar> sort_key_buf =
        fs_info->get_next_record_pointer(min_bytes);
    if (sort_key_buf.array() == nullptr) return true;
    const uint rec_sz =
        param->make_sortkey(sort_key_buf, ref_pos, longest_addons);
    if (rec_sz > sort_key_buf.size()) {
      // The record wouldn't fit. Try again, asking for a larger buffer.
      min_bytes = sort_key_buf.size() + 1;
    } else {
      fs_info->commit_used_memory(rec_sz);
      *key_length = rec_sz;
      return false;
    }
  }
}

/**
  Read all rows, and write them into a temporary file
  (if we run out of space in the sort buffer).
  All produced sequences are guaranteed to be non-empty.

  @param thd               Thread handle
  @param param             Sorting parameter
  @param fs_info           Struct containing sort buffer etc.
  @param chunk_file        File to write Merge_chunks describing sorted segments
                           in tempfile.
  @param tempfile          File to write sorted sequences of sortkeys to.
  @param pq                If !NULL, use it for keeping top N elements
  @param source_iterator   Where to read the rows to be sorted from.
  @param [out] found_rows  The number of FOUND_ROWS().
                           For a query with LIMIT, this value will typically
                           be larger than the function return value.
  @param [out] longest_key The largest single key found, in bytes.
  @param [out] longest_addons
     The longest addon field row (sum of all addon fields for any single
     given row) found.

  @note
    Basic idea:
    @verbatim
     while (get_next_sortkey())
     {
       if (using priority queue)
         push sort key into queue
       else
       {
         try to put sort key into buffer;
         if (no free space in sort buffer)
         {
           do {
             allocate new, larger buffer;
             retry putting sort key into buffer;
           } until (record fits or no space for new buffer)
           if (no space for new buffer)
           {
             sort record pointers (all buffers);
             dump sorted sequence to 'tempfile';
             dump Merge_chunk describing sequence location into 'chunk_file';
           }
         }
         if (key was packed)
           tell sort buffer the actual number of bytes used;
       }
     }
     if (buffer has some elements && dumped at least once)
       sort-dump-dump as above;
     else
       don't sort, leave sort buffer to be sorted by caller.
  @endverbatim

  @returns
    Number of records written on success.
  @returns
    HA_POS_ERROR on error.
*/

static ha_rows read_all_rows(
    THD *thd, Sort_param *param, Filesort_info *fs_info, IO_CACHE *chunk_file,
    IO_CACHE *tempfile,
    Bounded_queue<uchar *, uchar *, Sort_param, Mem_compare_queue_key> *pq,
    RowIterator *source_iterator, ha_rows *found_rows, size_t *longest_key,
    size_t *longest_addons) {
  /*
    Set up an error handler for filesort. It is automatically pushed
    onto the internal error handler stack upon creation, and will be
    popped off the stack automatically when the handler goes out of
    scope.
  */
  Filesort_error_handler error_handler(thd);

  DBUG_TRACE;

  int error = 0;
  TABLE *sort_form = param->sort_form;
  handler *file = sort_form->file;
  *found_rows = 0;
  size_t longest_key_so_far = 0;
  size_t longest_addon_so_far = 0;
  uchar *ref_pos = &file->ref[0];

  // NOTE(sgunders): When we sort row IDs, our read sets are a bit larger
  // than required by read_all_rows(); in particular, columns that we
  // don't sort on will still be read. (In particular, this makes us read
  // and allocate blobs, where present.) However, with sorting row IDs being
  // a rather marginal case, it's not worth it for us to try to compute new
  // read sets to have different ones in the first phase of such sorts.
  //
  // This isn't the only case of too large read sets. For a query such as
  // SELECT a FROM t1 WHERE b ORDER BY c, all three fields a,b,c will be in
  // the read set; in particular, find_order_in_list() will include any
  // columns used in ORDER BY in the read sets, as part of resolving them.
  // This is required for correct operation. However, anything that is part
  // of the read set will by extension be included as addon fields,
  // unless we sort row IDs for some reason -- even c, which is part of
  // the key. To remedy this, one would probably need a system
  // of pushing read sets through the iterator tree (except it should
  // ideally be done before optimization, where we set them up), so that
  // each iterator can use the right read set for its time.

  DEBUG_SYNC(thd, "after_index_merge_phase1");
  ha_rows num_total_records = 0, num_records_this_chunk = 0;
  uint num_written_chunks = 0;
  if (pq == nullptr) {
    fs_info->reset();
    fs_info->clear_peak_memory_used();
  }

  PFSBatchMode batch_mode(source_iterator);
  for (;;) {
    DBUG_EXECUTE_IF("bug19656296", DBUG_SET("+d,ha_rnd_next_deadlock"););
    if ((error = source_iterator->Read())) {
      break;
    }
    // Note where we are, for the case where we are not using addon fields.
    if (!param->using_addon_fields()) {
      file->position(sort_form->record[0]);
    }
    DBUG_EXECUTE_IF("debug_filesort", dbug_print_record(sort_form, true););

    if (thd->killed) {
      DBUG_PRINT("info", ("Sort killed by user"));
      return HA_POS_ERROR;
    }

    ++(*found_rows);
    num_total_records++;
    if (pq)
      pq->push(ref_pos);
    else {
      size_t key_length;
      bool out_of_mem = alloc_and_make_sortkey(
          param, fs_info, ref_pos, &key_length, &longest_addon_so_far);
      if (out_of_mem) {
        // Out of room, so flush chunk to disk (if there's anything to flush).
        if (num_records_this_chunk > 0) {
          if (write_keys(param, fs_info, num_records_this_chunk, chunk_file,
                         tempfile)) {
            return HA_POS_ERROR;
          }
          num_records_this_chunk = 0;
          num_written_chunks++;
          fs_info->reset();

          // Now we should have room for a new row.
          out_of_mem = alloc_and_make_sortkey(
              param, fs_info, ref_pos, &key_length, &longest_addon_so_far);
        }

        // If we're still out of memory after flushing to disk, give up.
        if (out_of_mem) {
          my_error(ER_OUT_OF_SORTMEMORY, ME_FATALERROR);
          LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_SORTMEMORY);
          return HA_POS_ERROR;
        }
      }

      longest_key_so_far = max(longest_key_so_far, key_length);
      num_records_this_chunk++;
    }
    /* It does not make sense to read more keys in case of a fatal error */
    if (thd->is_error()) break;
  }

  if (thd->is_error()) {
    return HA_POS_ERROR;
  }

  DBUG_PRINT("test",
             ("error: %d  num_written_chunks: %d", error, num_written_chunks));
  if (error == 1) {
    return HA_POS_ERROR;
  }
  if (num_written_chunks != 0 && num_records_this_chunk != 0 &&
      write_keys(param, fs_info, num_records_this_chunk, chunk_file,
                 tempfile)) {
    return HA_POS_ERROR;  // purecov: inspected
  }

  DBUG_PRINT("info", ("read_all_rows return %lu", (ulong)num_total_records));

  *longest_key = longest_key_so_far;
  *longest_addons = longest_addon_so_far;
  return num_total_records;
} /* read_all_rows */

/**
  @details
  Sort the buffer and write:
  -# the sorted sequence to tempfile
  -# a Merge_chunk describing the sorted sequence position to chunk_file

  @param param          Sort parameters
  @param fs_info        Contains the buffer to be sorted and written.
  @param count          Number of records to write.
  @param chunk_file     One 'Merge_chunk' struct will be written into this file.
                        The Merge_chunk::{file_pos, count} will indicate where
                        the sorted data was stored.
  @param tempfile       The sorted sequence will be written into this file.

  @returns
    0 OK
  @returns
    1 Error
*/

static int write_keys(Sort_param *param, Filesort_info *fs_info, uint count,
                      IO_CACHE *chunk_file, IO_CACHE *tempfile) {
  THD *thd = current_thd;
  Merge_chunk merge_chunk;
  DBUG_TRACE;

  count = fs_info->sort_buffer(param, count);

  if (!my_b_inited(chunk_file) &&
      filesort_open_cached_file(chunk_file, mysql_tmpdir, TEMP_PREFIX,
                                DISK_BUFFER_SIZE, MYF(MY_WME)))
    return 1;

  if (!my_b_inited(tempfile) &&
      filesort_open_cached_file(tempfile, mysql_tmpdir, TEMP_PREFIX,
                                DISK_BUFFER_SIZE, MYF(MY_WME), fs_info))
    return 1; /* purecov: inspected */

  // Check that we wont have more chunks than we can possibly keep in memory.
  if (my_b_tell(chunk_file) + sizeof(Merge_chunk) > (ulonglong)UINT_MAX)
    return 1; /* purecov: inspected */

  merge_chunk.set_file_position(my_b_tell(tempfile));
  if (static_cast<ha_rows>(count) > param->max_rows) {
    // Write only SELECT LIMIT rows to the file
    count = static_cast<uint>(param->max_rows); /* purecov: inspected */
  }
  merge_chunk.set_rowcount(static_cast<ha_rows>(count));

  for (uint ix = 0; ix < count; ++ix) {
    uchar *record = fs_info->get_sorted_record(ix);
    size_t rec_length = param->get_record_length(record);

    if (my_b_write(tempfile, record, rec_length))
      return 1; /* purecov: inspected */

    /* store the number of temp bytes written into filesort space
     * into statement metrics tables
     */
    MYSQL_INC_STATEMENT_FILESORT_BYTES_WRITTEN(thd->m_statement_psi,
                                               (ulonglong)rec_length);
  }

  if (my_b_write(chunk_file, pointer_cast<uchar *>(&merge_chunk),
                 sizeof(merge_chunk)))
    return 1; /* purecov: inspected */

  /* store the number of temp bytes written into filesort space
   * into statement metrics tables
   */
  MYSQL_INC_STATEMENT_FILESORT_BYTES_WRITTEN(thd->m_statement_psi,
                                             (ulonglong)sizeof(merge_chunk));

  return 0;
} /* write_keys */

/**
  Make a sort key for the JSON value in an Item.

  This function is called by Sort_param::make_sortkey(). We don't want
  it to be inlined, since that seemed to have a negative impact on
  some performance tests.

  @param[in]     item    The item for which to create a sort key.

  @param[out]    to      Pointer into the buffer to which the sort key should
                         be written. It will point to where the data portion
                         of the key should start.

  @param[out]    null_indicator
                         For nullable items, the NULL indicator byte.
                         (Ignored otherwise.) Should be initialized by the
                         caller to a value that indicates not NULL.

  @param[in]     length  The length of the sort key, not including the NULL
                         indicator byte at the beginning of the sort key for
                         nullable items.

  @param[in,out] hash    The hash key of the JSON values in the current row.

  @returns
    length of the key stored
*/
NO_INLINE
static uint make_json_sort_key(Item *item, uchar *to, uchar *null_indicator,
                               size_t length, ulonglong *hash) {
  DBUG_ASSERT(!item->maybe_null || *null_indicator == 1);

  Json_wrapper wr;
  if (item->val_json(&wr)) {
    // An error occurred, no point to continue making key, set it to null.
    if (item->maybe_null) *null_indicator = 0;
    return 0;
  }

  if (item->null_value) {
    /*
      Got NULL. The sort key should be all zeros. The caller has
      already tentatively set the NULL indicator byte at *null_indicator to
      not-NULL, so we need to clear that byte too.
    */
    if (item->maybe_null) {
      // Don't store anything but null flag.
      *null_indicator = 0;
      return 0;
    }
    /* purecov: begin inspected */
    DBUG_PRINT("warning", ("Got null on something that shouldn't be null"));
    DBUG_ASSERT(false);
    return 0;
    /* purecov: end */
  }

  size_t actual_length = wr.make_sort_key(to, length);
  *hash = wr.make_hash_key(*hash);
  return actual_length;
}

namespace {

/*
  Returns true if writing the given uint8_t would overflow <to> past <to_end>.
  Writes the value and advances <to> otherwise.
*/
inline bool write_uint8_overflows(uint8_t val, uchar *to_end, uchar **to) {
  if (to_end - *to < 1) return true;
  **to = val;
  (*to)++;
  return false;
}

/*
  Returns true if writing <num_bytes> zeros would overflow <to> past <to_end>.
  Writes the zeros and advances <to> otherwise.
*/
inline bool clear_overflows(size_t num_bytes, uchar *to_end, uchar **to) {
  if (static_cast<size_t>(to_end - *to) < num_bytes) return true;
  memset(*to, 0, num_bytes);
  *to += num_bytes;
  return false;
}

/*
  Returns true if advancing <to> by <num_bytes> would put it past <to_end>.
  Advances <to> otherwise (does not write anything to the buffer).
*/
inline bool advance_overflows(size_t num_bytes, uchar *to_end, uchar **to) {
  if (static_cast<size_t>(to_end - *to) < num_bytes) return true;
  *to += num_bytes;
  return false;
}

static inline longlong get_int_sort_key_for_item_inline(Item *item) {
  // Temporal items are sorted on the underlying UTC value,
  // so temporarily set the time zone.
  Time_zone *old_tz = current_thd->variables.time_zone;
  current_thd->variables.time_zone = my_tz_UTC;
  longlong value = item->data_type() == MYSQL_TYPE_TIME
                       ? item->val_time_temporal()
                       : item->is_temporal_with_date()
                             ? item->val_date_temporal()
                             : item->val_int();
  current_thd->variables.time_zone = old_tz;
  return value;
}

/*
  Writes a NULL indicator byte (if the field may be NULL), leaves space for a
  varlength prefix (if varlen and not NULL), and then the actual sort key.
  Returns the length of the key, sans NULL indicator byte and varlength prefix,
  or UINT_MAX if the value would not provably fit within the given bounds.
*/
size_t make_sortkey_from_item(Item *item, Item_result result_type,
                              Nullable<size_t> dst_length, String *tmp_buffer,
                              uchar *to, uchar *to_end, bool *maybe_null,
                              ulonglong *hash) {
  bool is_varlen = !dst_length.has_value();

  uchar *null_indicator = nullptr;
  *maybe_null = item->maybe_null;
  if (item->maybe_null) {
    null_indicator = to;
    /*
      Assume not NULL by default. Will be overwritten if needed.
      Note that we can't check item->null_value at this time,
      because it will only get properly set after a call to val_*().
    */
    if (write_uint8_overflows(1, to_end, &to)) return UINT_MAX;
  }

  if (is_varlen) {
    // Check that there is room for the varlen prefix, and advance past it.
    if (advance_overflows(VARLEN_PREFIX, to_end, &to)) return UINT_MAX;
  } else {
    // Check that there is room for the fixed-size value.
    if (static_cast<size_t>(to_end - to) < dst_length.value()) return UINT_MAX;
  }

  switch (result_type) {
    case STRING_RESULT: {
      if (item->data_type() == MYSQL_TYPE_JSON) {
        DBUG_ASSERT(is_varlen);
        return make_json_sort_key(item, to, null_indicator, to_end - to, hash);
      }

      const CHARSET_INFO *cs = item->collation.collation;

      String *res = item->val_str(tmp_buffer);
      if (res == nullptr)  // Value is NULL.
      {
        DBUG_ASSERT(item->maybe_null);
        *null_indicator = 0;
        if (is_varlen) {
          // Don't store anything except the NULL flag.
          return 0;
        }
        memset(to, 0, dst_length.value());
        return dst_length.value();
      }

      uint src_length = static_cast<uint>(res->length());
      const char *from = res->ptr();

      size_t actual_length;
      if (is_varlen) {
        size_t max_length = to_end - to;
        if (max_length % 2 != 0) {
          // Heed the contract that strnxfrm needs an even number of bytes.
          --max_length;
        }
        actual_length = cs->coll->strnxfrm(
            cs, to, max_length, item->max_char_length(),
            pointer_cast<const uchar *>(from), src_length, 0);
        if (actual_length == max_length) {
          /*
            The sort key eithen fit perfectly, or overflowed; we can't
            distinguish between the two, so we have to count it as overflow.
          */
          return UINT_MAX;
        }
      } else {
        actual_length = cs->coll->strnxfrm(
            cs, to, dst_length.value(), item->max_char_length(),
            pointer_cast<const uchar *>(from), src_length,
            MY_STRXFRM_PAD_TO_MAXLEN);
        DBUG_ASSERT(actual_length == dst_length.value());
      }
      DBUG_ASSERT(to + actual_length <= to_end);
      return actual_length;
    }
    case INT_RESULT: {
      DBUG_ASSERT(!is_varlen);
      longlong value = get_int_sort_key_for_item_inline(item);

      /*
        Note: item->null_value can't be trusted alone here; there are cases
        (for the DATE data type in particular) where we can have
        item->null_value set without maybe_null being set! This really should be
        cleaned up, but until that happens, we need to have a more conservative
        check.
      */
      if (item->maybe_null && item->null_value) {
        *null_indicator = 0;
        memset(to, 0, dst_length.value());
      } else
        copy_native_longlong(to, dst_length.value(), value,
                             item->unsigned_flag);
      return dst_length.value();
    }
    case DECIMAL_RESULT: {
      DBUG_ASSERT(!is_varlen);
      my_decimal dec_buf, *dec_val = item->val_decimal(&dec_buf);
      /*
        Note: item->null_value can't be trusted alone here; there are cases
        where we can have item->null_value set without maybe_null being set!
        (There are also cases where dec_val can return non-nullptr even in
        the case of a NULL result.) This really should be cleaned up, but until
        that happens, we need to have a more conservative check.
      */
      if (item->maybe_null && item->null_value) {
        *null_indicator = 0;
        memset(to, 0, dst_length.value());
      } else if (dst_length.value() < DECIMAL_MAX_FIELD_SIZE) {
        uchar buf[DECIMAL_MAX_FIELD_SIZE];
        my_decimal2binary(E_DEC_FATAL_ERROR, dec_val, buf,
                          item->max_length - (item->decimals ? 1 : 0),
                          item->decimals);
        memcpy(to, buf, dst_length.value());
      } else {
        my_decimal2binary(E_DEC_FATAL_ERROR, dec_val, to,
                          item->max_length - (item->decimals ? 1 : 0),
                          item->decimals);
      }
      return dst_length.value();
    }
    case REAL_RESULT: {
      DBUG_ASSERT(!is_varlen);
      double value = item->val_real();
      if (item->null_value) {
        DBUG_ASSERT(item->maybe_null);
        *null_indicator = 0;
        memset(to, 0, dst_length.value());
      } else if (dst_length.value() < sizeof(double)) {
        uchar buf[sizeof(double)];
        change_double_for_sort(value, buf);
        memcpy(to, buf, dst_length.value());
      } else {
        change_double_for_sort(value, to);
      }
      return dst_length.value();
    }
    case ROW_RESULT:
    default:
      // This case should never be choosen
      DBUG_ASSERT(0);
      return dst_length.value();
  }
}

}  // namespace

// Expose for Item_func_weight_string.
longlong get_int_sort_key_for_item(Item *item) {
  return get_int_sort_key_for_item_inline(item);
}

uint Sort_param::make_sortkey(Bounds_checked_array<uchar> dst,
                              const uchar *ref_pos,
                              size_t *longest_addon_so_far) {
  uchar *to = dst.array();
  uchar *to_end = dst.array() + dst.size();
  uchar *orig_to = to;
  const st_sort_field *sort_field;
  ulonglong hash = 0;

  if (using_varlen_keys()) {
    to += size_of_varlength_field;
    if (to >= to_end) return UINT_MAX;
  }
  for (sort_field = local_sortorder.begin();
       sort_field != local_sortorder.end(); sort_field++) {
    if (to >= to_end ||
        (!sort_field->is_varlen &&
         static_cast<size_t>(to_end - to) < sort_field->length)) {
      return UINT_MAX;
    }

    bool maybe_null;
    Nullable<size_t> dst_length;
    if (!sort_field->is_varlen) dst_length = sort_field->length;
    uint actual_length;
    Item *item = sort_field->item;
    DBUG_ASSERT(sort_field->field_type == item->data_type());

    actual_length =
        make_sortkey_from_item(item, sort_field->result_type, dst_length,
                               &tmp_buffer, to, to_end, &maybe_null, &hash);

    if (actual_length == UINT_MAX) {
      // Overflow.
      return UINT_MAX;
    }

    /*
      Now advance past the key that was just written, reversing the parts that
      we need to reverse.
    */

    bool is_null = maybe_null && *to == 0;
    if (maybe_null) {
      DBUG_ASSERT(*to == 0 || *to == 1);
      if (sort_field->reverse && is_null) {
        *to = 0xff;
      }
      ++to;
    }

    // Fill out the varlen prefix if it exists.
    if (sort_field->is_varlen && !is_null) {
      int4store(to, actual_length + VARLEN_PREFIX);
      to += VARLEN_PREFIX;
    }

    // Reverse the key if needed.
    if (sort_field->reverse) {
      while (actual_length--) {
        *to = (uchar)(~*to);
        to++;
      }
    } else {
      to += actual_length;
    }
  }

  if (use_hash) {
    if (to_end - to < 8) return UINT_MAX;
    int8store(to, hash);
    to += 8;
  }

  if (using_varlen_keys()) {
    // Store the length of the record as a whole.
    Sort_param::store_varlen_key_length(orig_to,
                                        static_cast<uint>(to - orig_to));
  }

  if (using_addon_fields()) {
    /*
      Save field values appended to sorted fields.
      First null bit indicators are appended then field values follow.
    */
    uchar *nulls = to;
    uchar *p_len = to;

    Addon_fields_array::const_iterator addonf = addon_fields->begin();
    if (clear_overflows(addonf->offset, to_end, &to)) return UINT_MAX;
    if (addon_fields->using_packed_addons()) {
      for (; addonf != addon_fields->end(); ++addonf) {
        Field *field = addonf->field;
        if (addonf->null_bit && field->is_null()) {
          nulls[addonf->null_offset] |= addonf->null_bit;
        } else {
          to = field->pack(to, field->ptr, to_end - to,
                           field->table->s->db_low_byte_first);
          if (to >= to_end) return UINT_MAX;
        }
      }
      Addon_fields::store_addon_length(p_len, to - p_len);
    } else {
      for (; addonf != addon_fields->end(); ++addonf) {
        Field *field = addonf->field;
        if (static_cast<size_t>(to_end - to) < addonf->max_length) {
          return UINT_MAX;
        }
        if (addonf->null_bit && field->is_null()) {
          nulls[addonf->null_offset] |= addonf->null_bit;
        } else {
          uchar *ptr MY_ATTRIBUTE((unused)) = field->pack(
              to, field->ptr, to_end - to, field->table->s->db_low_byte_first);
          DBUG_ASSERT(ptr <= to + addonf->max_length);
        }
        to += addonf->max_length;
      }
    }
    *longest_addon_so_far = max<size_t>(*longest_addon_so_far, to - p_len);
    DBUG_PRINT("info", ("make_sortkey %p %u", orig_to,
                        static_cast<unsigned>(to - p_len)));
  } else {
    if (static_cast<size_t>(to_end - to) < ref_length) {
      return UINT_MAX;
    }

    /* Save filepos last */
    memcpy(to, ref_pos, ref_length);
    to += ref_length;
  }
  return to - orig_to;
}

/**
  This function is used only if the entire result set fits in memory.

  For addon fields, we keep the result in the filesort buffer.
  This saves us a lot of memcpy calls.

  For row references, we copy the final sorted result into a buffer,
  but we do not copy the actual sort-keys, as they are no longer needed.
  We could have kept the result in the sort buffere here as well,
  but the new buffer - containing only row references - is probably a
  lot smaller.

  The result data will be unpacked by SortBufferIterator
  or SortBufferIndirectIterator

  Note that SortBufferIterator does not have access to a Sort_param.
  It does however have access to a Filesort_info, which knows whether
  we have variable sized keys or not.
  TODO: consider templatizing SortBufferIterator on is_varlen or not.

  @param [in]     param      Sort parameters.
  @param          count      Number of records
  @param [in,out] table_sort Information used by SortBufferIterator /
                             SortBufferIndirectIterator
  @param [out]    sort_result Where to store the actual result
 */
static bool save_index(Sort_param *param, uint count, Filesort_info *table_sort,
                       Sort_result *sort_result) {
  uchar *to;
  DBUG_TRACE;

  table_sort->set_sort_length(param->max_compare_length(),
                              param->using_varlen_keys());

  count = table_sort->sort_buffer(param, count);
  sort_result->found_records = count;

  if (param->using_addon_fields()) {
    sort_result->sorted_result_in_fsbuf = true;
    return false;
  }

  sort_result->sorted_result_in_fsbuf = false;
  const size_t buf_size = size_t{param->fixed_res_length} * count;

  DBUG_ASSERT(sort_result->sorted_result == nullptr);
  sort_result->sorted_result.reset(static_cast<uchar *>(my_malloc(
      key_memory_Filesort_info_record_pointers, buf_size, MYF(MY_WME))));
  if (!(to = sort_result->sorted_result.get()))
    return true; /* purecov: inspected */
  sort_result->sorted_result_end = sort_result->sorted_result.get() + buf_size;

  uint res_length = param->fixed_res_length;
  for (uint ix = 0; ix < count; ++ix) {
    uchar *record = table_sort->get_sorted_record(ix);
    uchar *start_of_payload = param->get_start_of_payload(record);
    memcpy(to, start_of_payload, res_length);
    to += res_length;
  }
  return false;
}

/**
  Test whether priority queue is worth using to get top elements of an
  ordered result set. If it is, then allocates buffer for required amount of
  records

  @param trace            Current trace context.
  @param param            Sort parameters.
  @param filesort_info    Filesort information.
  @param num_rows         Estimate of number of rows in source record set.
  @param memory_available Memory available for sorting.

  DESCRIPTION
    Given a query like this:
      SELECT ... FROM t ORDER BY a1,...,an LIMIT max_rows;
    This function tests whether a priority queue can be used to keep
    the result (ie., there is enough memory to store @<max_rows@> rows).

   @returns
    true  - if it's ok to use PQ
    false - or there is not enough memory.
*/

bool check_if_pq_applicable(Opt_trace_context *trace, Sort_param *param,
                            Filesort_info *filesort_info, ha_rows num_rows,
                            ulong memory_available) {
  DBUG_TRACE;

  /*
    How much Priority Queue sort is slower than qsort.
    Measurements (see unit test) indicate that PQ is roughly 3 times slower.
  */
  const double PQ_slowness = 3.0;

  Opt_trace_object trace_filesort(trace,
                                  "filesort_priority_queue_optimization");
  if (param->max_rows == HA_POS_ERROR) {
    trace_filesort.add("usable", false)
        .add_alnum("cause", "not applicable (no LIMIT)");
    return false;
  }

  if (param->m_remove_duplicates) {
    trace_filesort.add("usable", false)
        .add_alnum("cause", "duplicate removal not supported yet");
    return false;
  }

  trace_filesort.add("limit", param->max_rows);

  if (param->max_rows + 2 >= UINT_MAX) {
    trace_filesort.add("usable", false).add_alnum("cause", "limit too large");
    return false;
  }
  if (param->max_record_length() >= 0xFFFFFFFFu) {
    trace_filesort.add("usable", false)
        .add_alnum("cause", "contains records of unbounded length");
    return false;
  }

  ulong num_available_keys =
      memory_available / (param->max_record_length() + sizeof(char *));
  // We need 1 extra record in the buffer, when using PQ.
  param->max_rows_per_buffer = (uint)param->max_rows + 1;

  if (num_rows < num_available_keys) {
    // The whole source set fits into memory.
    if (param->max_rows < num_rows / PQ_slowness) {
      filesort_info->set_max_size(memory_available, param->max_record_length());
      trace_filesort.add("chosen", true);
      return filesort_info->max_size_in_bytes() > 0;
    } else {
      // PQ will be slower.
      trace_filesort.add("chosen", false).add_alnum("cause", "sort_is_cheaper");
      return false;
    }
  }

  // Do we have space for LIMIT rows in memory?
  if (param->max_rows_per_buffer < num_available_keys) {
    filesort_info->set_max_size(memory_available, param->max_record_length());
    trace_filesort.add("chosen", true);
    return filesort_info->max_size_in_bytes() > 0;
  }

  return false;
}

/**
  Read from a disk file into the merge chunk's buffer. We generally read as
  many complete rows as we can, except when bounded by max_keys() or rowcount().
  Incomplete rows will be left in the file.

  @returns
    Number of bytes read, or (uint)-1 if something went wrong.
*/
static uint read_to_buffer(IO_CACHE *fromfile, Merge_chunk *merge_chunk,
                           Sort_param *param) {
  DBUG_TRACE;
  uint rec_length = param->max_record_length();
  ha_rows count;

  const bool packed_addon_fields = param->using_packed_addons();
  const bool using_varlen_keys = param->using_varlen_keys();

  if (merge_chunk->rowcount() > 0) {
    size_t bytes_to_read;
    if (packed_addon_fields || using_varlen_keys) {
      count = merge_chunk->rowcount();
      bytes_to_read = min(merge_chunk->buffer_size(),
                          static_cast<size_t>(fromfile->end_of_file -
                                              merge_chunk->file_position()));
    } else {
      count = min(merge_chunk->max_keys(), merge_chunk->rowcount());
      bytes_to_read = rec_length * static_cast<size_t>(count);
      if (count == 0) {
        // Not even room for the first row.
        my_error(ER_OUT_OF_SORTMEMORY, ME_FATALERROR);
        LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_SORTMEMORY);
        return (uint)-1;
      }
    }

    DBUG_PRINT("info",
               ("read_to_buffer %p at file_pos %llu bytes %llu", merge_chunk,
                static_cast<ulonglong>(merge_chunk->file_position()),
                static_cast<ulonglong>(bytes_to_read)));
    if (mysql_file_pread(fromfile->file, merge_chunk->buffer_start(),
                         bytes_to_read, merge_chunk->file_position(), MYF_RW))
      return (uint)-1; /* purecov: inspected */

    size_t num_bytes_read;
    if (packed_addon_fields || using_varlen_keys) {
      /*
        The last record read is most likely not complete here.
        We need to loop through all the records, reading the length fields,
        and then "chop off" the final incomplete record.
       */
      uchar *record = merge_chunk->buffer_start();
      uint ix = 0;
      for (; ix < count; ++ix) {
        if (using_varlen_keys &&
            (record + Sort_param::size_of_varlength_field) >=
                merge_chunk->buffer_end())
          break;  // Incomplete record.

        uchar *start_of_payload = param->get_start_of_payload(record);
        if (start_of_payload >= merge_chunk->buffer_end())
          break;  // Incomplete record.

        if (packed_addon_fields &&
            start_of_payload + Addon_fields::size_of_length_field >=
                merge_chunk->buffer_end())
          break;  // Incomplete record.

        const uint res_length =
            packed_addon_fields
                ? Addon_fields::read_addon_length(start_of_payload)
                : param->fixed_res_length;

        if (start_of_payload + res_length >= merge_chunk->buffer_end())
          break;  // Incomplete record.

        DBUG_ASSERT(res_length > 0);
        record = start_of_payload + res_length;
      }
      if (ix == 0) {
        // Not even room for the first row.
        my_error(ER_OUT_OF_SORTMEMORY, ME_FATALERROR);
        LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_SORTMEMORY);
        return (uint)-1;
      }
      count = ix;
      num_bytes_read = record - merge_chunk->buffer_start();
      DBUG_PRINT("info", ("read %llu bytes of complete records",
                          static_cast<ulonglong>(bytes_to_read)));
    } else
      num_bytes_read = bytes_to_read;

    merge_chunk->init_current_key();
    merge_chunk->advance_file_position(num_bytes_read);
    merge_chunk->decrement_rowcount(count);
    merge_chunk->set_mem_count(count);
    return num_bytes_read;
  }

  return 0;
} /* read_to_buffer */

namespace {

/**
  This struct is used for merging chunks for filesort()
  For filesort() with fixed-size keys we use memcmp to compare rows.
  For variable length keys, we use cmp_varlen_keys to compare rows.
 */
struct Merge_chunk_greater {
  size_t m_len;
  Sort_param *m_param;

  // CTOR for filesort() with fixed-size keys
  explicit Merge_chunk_greater(size_t len) : m_len(len), m_param(nullptr) {}

  // CTOR for filesort() with varlen keys
  explicit Merge_chunk_greater(Sort_param *param) : m_len(0), m_param(param) {}

  bool operator()(Merge_chunk *a, Merge_chunk *b) const {
    return key_is_greater_than(a->current_key(), b->current_key());
  }

  bool key_is_greater_than(uchar *key1, uchar *key2) const {
    // Fixed len keys
    if (m_len) return memcmp(key1, key2, m_len) > 0;

    if (m_param)
      return !cmp_varlen_keys(m_param->local_sortorder, m_param->use_hash, key1,
                              key2);

    // We can actually have zero-length sort key for filesort().
    return false;
  }
};

}  // namespace

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
  @param include_keys   If true, write both the keys and the addons / row
  positions. If false, the keys will be skipped (useful only for the output of
  the final merge, where we don't need to compare rows further).

  @returns
    0      OK
  @returns
    other  error
*/
static int merge_buffers(THD *thd, Sort_param *param, IO_CACHE *from_file,
                         IO_CACHE *to_file, Sort_buffer sort_buffer,
                         Merge_chunk *last_chunk, Merge_chunk_array chunk_array,
                         bool include_keys) {
  int error = 0;
  ha_rows max_rows, org_max_rows;
  uchar *strpos;
  Merge_chunk *merge_chunk;
  DBUG_TRACE;

  thd->inc_status_sort_merge_passes();

  my_off_t to_start_filepos = my_b_tell(to_file);
  strpos = sort_buffer.array();
  org_max_rows = max_rows = param->max_rows;

  // Only relevant for fixed-length rows.
  ha_rows maxcount = param->max_rows_per_buffer / chunk_array.size();

  // If we don't use addon fields, we'll have the record position appended to
  // the end of each record. This disturbs our equality comparisons, so we'll
  // have to remove it. (Removing it also makes the comparisons ever so slightly
  // cheaper.)
  size_t key_len = param->max_compare_length();
  if (!param->using_addon_fields()) {
    key_len -= param->ref_length;
  }

  Merge_chunk_greater mcl = param->using_varlen_keys()
                                ? Merge_chunk_greater(param)
                                : Merge_chunk_greater(key_len);
  Priority_queue<Merge_chunk *,
                 std::vector<Merge_chunk *, Malloc_allocator<Merge_chunk *>>,
                 Merge_chunk_greater>
  queue(mcl, Malloc_allocator<Merge_chunk *>(key_memory_Filesort_info_merge));

  if (queue.reserve(chunk_array.size())) return 1;

  for (merge_chunk = chunk_array.begin(); merge_chunk != chunk_array.end();
       merge_chunk++) {
    const size_t chunk_sz = sort_buffer.size() / chunk_array.size();
    merge_chunk->set_buffer(strpos, strpos + chunk_sz);

    merge_chunk->set_max_keys(maxcount);
    strpos += chunk_sz;
    error = static_cast<int>(read_to_buffer(from_file, merge_chunk, param));

    if (error == -1) return error; /* purecov: inspected */
    // If less data in buffers than expected
    merge_chunk->set_max_keys(merge_chunk->mem_count());
    (void)queue.push(merge_chunk);
  }

  bool seen_any_records = false;  // Used for deduplication only.
  while (queue.size() > 1) {
    if (thd->killed) {
      return 1; /* purecov: inspected */
    }
    for (;;) {
      merge_chunk = queue.top();
      unsigned row_length, payload_length;
      {
        param->get_rec_and_res_len(merge_chunk->current_key(), &row_length,
                                   &payload_length);
        const uint bytes_to_write = include_keys ? row_length : payload_length;
        unsigned offset = include_keys ? 0 : (row_length - payload_length);

        bool is_duplicate = false;
        if (param->m_remove_duplicates) {
          if (seen_any_records &&
              !mcl.key_is_greater_than(merge_chunk->current_key(),
                                       param->m_last_key_seen)) {
            is_duplicate = true;
          } else {
            seen_any_records = true;
            memcpy(param->m_last_key_seen, merge_chunk->current_key(),
                   row_length - payload_length);
          }
        }

        if (!is_duplicate) {
          if (my_b_write(to_file, merge_chunk->current_key() + offset,
                         bytes_to_write)) {
            return 1; /* purecov: inspected */
          }

          /* store the number of temp bytes written into filesort space
           * into statement metrics tables
           */
          MYSQL_INC_STATEMENT_FILESORT_BYTES_WRITTEN(thd->m_statement_psi,
                                                     (ulonglong)bytes_to_write);

          if (!--max_rows) {
            error = 0; /* purecov: inspected */
            goto end;  /* purecov: inspected */
          }
        }
      }

      merge_chunk->advance_current_key(row_length);
      merge_chunk->decrement_mem_count();
      if (0 == merge_chunk->mem_count()) {
        // No more records in memory for this chunk. Read more, and if there's
        // none, take it out of the queue.
        if (!(error = (int)read_to_buffer(from_file, merge_chunk, param))) {
          queue.pop();
          reuse_freed_buff(merge_chunk, &queue);
          break; /* One buffer have been removed */
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

  // Only one chunk left; read all of its records.
  merge_chunk = queue.top();
  merge_chunk->set_buffer(sort_buffer.array(),
                          sort_buffer.array() + sort_buffer.size());
  merge_chunk->set_max_keys(param->max_rows_per_buffer);

  do {
    if (merge_chunk->mem_count() > max_rows) {
      merge_chunk->set_mem_count(max_rows); /* Don't write too many records */
      merge_chunk->set_rowcount(0);         /* Don't read more */
    }
    max_rows -= merge_chunk->mem_count();

    for (uint ix = 0; ix < merge_chunk->mem_count(); ++ix) {
      unsigned row_length, payload_length;
      param->get_rec_and_res_len(merge_chunk->current_key(), &row_length,
                                 &payload_length);
      const uint bytes_to_write = include_keys ? row_length : payload_length;
      unsigned offset = include_keys ? 0 : (row_length - payload_length);

      // Since there's only one chunk left, and it does not contain duplicates
      // internally, we only need to check for duplicates on the first
      // iteration of the loop.
      bool is_duplicate =
          (ix == 0 && param->m_remove_duplicates && seen_any_records &&
           !mcl.key_is_greater_than(merge_chunk->current_key(),
                                    param->m_last_key_seen));
      if (!is_duplicate) {
        if (my_b_write(to_file, merge_chunk->current_key() + offset,
                       bytes_to_write)) {
          return 1; /* purecov: inspected */
        }

        /* store the number of temp bytes written into filesort space
         * into statement metrics tables
         */
        MYSQL_INC_STATEMENT_FILESORT_BYTES_WRITTEN(thd->m_statement_psi,
                                                   (ulonglong)bytes_to_write);
      }
      merge_chunk->advance_current_key(row_length);
    }
  } while ((error = (int)read_to_buffer(from_file, merge_chunk, param)) != -1 &&
           error != 0);

end:
  last_chunk->set_rowcount(min(org_max_rows - max_rows, param->max_rows));
  last_chunk->set_file_position(to_start_filepos);

  return error;
} /* merge_buffers */

/* Do a merge to output-file (save only positions) */

static int merge_index(THD *thd, Sort_param *param, Sort_buffer sort_buffer,
                       Merge_chunk_array chunk_array, IO_CACHE *tempfile,
                       IO_CACHE *outfile) {
  DBUG_TRACE;
  if (merge_buffers(thd,
                    param,                // param
                    tempfile,             // from_file
                    outfile,              // to_file
                    sort_buffer,          // sort_buffer
                    chunk_array.begin(),  // last_chunk [out]
                    chunk_array,
                    false))  // include_keys
    return 1;                /* purecov: inspected */
  return 0;
} /* merge_index */

/**
  Calculate length of sort key.

  @param thd			  Thread handler
  @param sortorder		  Order of items to sort
  @param s_length	          Number of items to sort

  @note
    sortorder->length is updated for each sort item.

  @return
    Total length of sort buffer in bytes
*/

uint sortlength(THD *thd, st_sort_field *sortorder, uint s_length) {
  uint total_length = 0;

  // Heed the contract that strnxfrm() needs an even number of bytes.
  const uint max_sort_length_even = (thd->variables.max_sort_length + 1) & ~1;

  for (; s_length--; sortorder++) {
    bool is_string_type = false;
    const Item *item = sortorder->item;
    sortorder->result_type = item->result_type();
    sortorder->field_type = item->data_type();
    if (item->type() == Item::FIELD_ITEM &&
        (down_cast<const Item_field *>(item)->field->real_type() ==
             MYSQL_TYPE_ENUM ||
         down_cast<const Item_field *>(item)->field->real_type() ==
             MYSQL_TYPE_SET)) {
      // Sort enum and set fields as their underlying ints.
      sortorder->result_type = INT_RESULT;
    }
    if (sortorder->field_type == MYSQL_TYPE_JSON)
      sortorder->is_varlen = true;
    else
      sortorder->is_varlen = false;
    if (item->is_temporal()) sortorder->result_type = INT_RESULT;
    switch (sortorder->result_type) {
      case STRING_RESULT: {
        const CHARSET_INFO *cs = item->collation.collation;
        sortorder->length = item->max_length;

        if (cs->pad_attribute == NO_PAD) {
          sortorder->is_varlen = true;
        }

        if (sortorder->length < (10 << 20)) {  // 10 MB.
          // How many bytes do we need (including sort weights) for
          // strnxfrm()?
          sortorder->length = cs->coll->strnxfrmlen(cs, sortorder->length);
        } else {
          /*
            If over 10 MB, just set the length as effectively infinite, so we
            don't get overflows in strnxfrmlen().
           */
          sortorder->length = 0xFFFFFFFFu;
        }
        is_string_type = true;
        break;
      }
      case INT_RESULT:
#if SIZEOF_LONG_LONG > 4
        sortorder->length = 8;  // Size of intern longlong
#else
        sortorder->length = 4;
#endif
        break;
      case DECIMAL_RESULT:
        sortorder->length = my_decimal_get_binary_size(
            item->max_length - (item->decimals ? 1 : 0), item->decimals);
        break;
      case REAL_RESULT:
        sortorder->length = sizeof(double);
        break;
      case ROW_RESULT:
      default:
        // This case should never be choosen
        DBUG_ASSERT(0);
        break;
    }
    sortorder->maybe_null = item->maybe_null;
    if (!sortorder->is_varlen && is_string_type) {
      /*
        We would love to never have to care about max_sort_length anymore,
        but that would make it impossible for us to sort blobs (TEXT) with
        PAD SPACE collations, since those are not variable-length (the padding
        is serialized as part of the sort key) and thus require infinite space.
        Thus, as long as we need to sort such fields by storing their sort
        keys, we need to heed max_sort_length for such fields.
      */
      sortorder->length = std::min(sortorder->length, max_sort_length_even);
    }

    if (sortorder->maybe_null)
      AddWithSaturate(1u, &total_length);  // Place for NULL marker
    if (sortorder->is_varlen)
      AddWithSaturate(VARLEN_PREFIX, &sortorder->length);
    AddWithSaturate(sortorder->length, &total_length);
  }
  DBUG_PRINT("info", ("sort_length: %u", total_length));
  return total_length;
}

/**
  Get descriptors of fields appended to sorted fields and
  calculate their total length.

  The function first finds out what fields are used in the result set.
  Then it calculates the length of the buffer to store the values of
  these fields together with the value of sort values.
  If there are no large blobs (which prevent addon fields), the function
  allocates memory for an array of descriptors containing layouts for the values
  of the non-sorted fields in the buffer and fills them.

  @param table                 Which table we are reading from
  @param[out] addon_fields_status Reason for *not* using packed addon fields
  @param[out] plength          Total length of appended fields
  @param[out] ppackable_length Total length of appended fields having a
                               packable type

  @note
    The null bits for the appended values are supposed to be put together
    and stored into the buffer just ahead of the value of the first field.

  @return
    Pointer to the layout descriptors for the appended fields, if any
  @returns
    NULL   if we do not store field values with sort data.
*/

Addon_fields *Filesort::get_addon_fields(
    TABLE *table, Addon_fields_status *addon_fields_status, uint *plength,
    uint *ppackable_length) {
  uint total_length = 0;
  uint packable_length = 0;
  uint num_fields = 0;
  uint null_fields = 0;

  /*
    If there is a reference to a field in the query add it
    to the the set of appended fields.
    Note for future refinement:
    This this a too strong condition.
    Actually we need only the fields referred in the
    result set. And for some of them it makes sense to use
    the values directly from sorted fields.
  */
  *plength = *ppackable_length = 0;
  *addon_fields_status = Addon_fields_status::unknown_status;

  for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (!bitmap_is_set(table->read_set, field->field_index)) continue;

    // Having large blobs in addon fields could be very inefficient,
    // but small blobs are OK (where “small” is a bit fuzzy, and relative
    // to the size of the sort buffer). There are two types of small blobs:
    //
    //  - Those explicitly bounded to small lengths, namely tinyblob (255 bytes)
    //    and blob (65535 bytes).
    //  - Those that are _typically_ fairly small, which includes JSON and
    //    geometries. We don't actually declare anywhere that they are
    //    implemented using blobs under the hood, so it's not unreasonable to
    //    demand that the user have large enough sort buffers for a few rows.
    //    (If a user has multi-megabyte JSON rows and wishes to sort them,
    //    they would usually have a fair bit of RAM anyway, since they'd need
    //    that to hold the result set and process it in a reasonable fashion.)
    //
    // That leaves only mediumblob and longblob. If a user declares a field as
    // one of those, it's reasonable for them to expect that sorting doesn't
    // need to pull many of them up in memory, so we should stick to sorting row
    // IDs.
    if (field->type() == MYSQL_TYPE_BLOB &&
        field->max_packed_col_length() > 70000u) {
      DBUG_ASSERT(m_sort_param.addon_fields == nullptr);
      *addon_fields_status = Addon_fields_status::row_contains_blob;
      return nullptr;
    }

    const uint field_length = field->max_packed_col_length();
    AddWithSaturate(field_length, &total_length);

    const enum_field_types field_type = field->type();
    if (field->is_nullable() || field_type == MYSQL_TYPE_STRING ||
        field_type == MYSQL_TYPE_VARCHAR ||
        field_type == MYSQL_TYPE_VAR_STRING || (field->flags & BLOB_FLAG)) {
      AddWithSaturate(field_length, &packable_length);
    }
    if (field->is_nullable()) null_fields++;
    num_fields++;
  }
  if (0 == num_fields) return nullptr;

  AddWithSaturate((null_fields + 7) / 8, &total_length);

  *ppackable_length = packable_length;

  if (m_sort_param.addon_fields == nullptr) {
    void *rawmem1 = (*THR_MALLOC)->Alloc(sizeof(Addon_fields));
    void *rawmem2 = (*THR_MALLOC)->Alloc(sizeof(Sort_addon_field) * num_fields);
    if (rawmem1 == nullptr || rawmem2 == nullptr)
      return nullptr; /* purecov: inspected */
    Addon_fields_array addon_array(static_cast<Sort_addon_field *>(rawmem2),
                                   num_fields);
    m_sort_param.addon_fields = new (rawmem1) Addon_fields(addon_array);
  } else {
    /*
      Allocate memory only once, reuse descriptor array and buffer.
      Set using_packed_addons here, and size/offset details below.
     */
    DBUG_ASSERT(num_fields ==
                m_sort_param.addon_fields->num_field_descriptors());
    m_sort_param.addon_fields->set_using_packed_addons(false);
  }

  *plength = total_length;

  uint length = (null_fields + 7) / 8;
  null_fields = 0;
  Addon_fields_array::iterator addonf = m_sort_param.addon_fields->begin();
  for (Field **pfield = table->field; *pfield != nullptr; ++pfield) {
    Field *field = *pfield;
    if (!bitmap_is_set(table->read_set, field->field_index)) continue;
    DBUG_ASSERT(addonf != m_sort_param.addon_fields->end());

    addonf->field = field;
    addonf->offset = length;
    if (field->is_nullable()) {
      addonf->null_offset = null_fields / 8;
      addonf->null_bit = 1 << (null_fields & 7);
      null_fields++;
    } else {
      addonf->null_offset = 0;
      addonf->null_bit = 0;
    }
    addonf->max_length = field->max_packed_col_length();
    DBUG_PRINT("info", ("addon_field %s max_length %u",
                        addonf->field->field_name, addonf->max_length));

    AddWithSaturate(addonf->max_length, &length);
    addonf++;
  }

  DBUG_PRINT("info", ("addon_length: %d", length));
  *addon_fields_status = Addon_fields_status::using_addon_fields;
  return m_sort_param.addon_fields;
}

bool Filesort::using_addon_fields() {
  if (m_sort_param.m_addon_fields_status ==
      Addon_fields_status::unknown_status) {
    m_sort_param.decide_addon_fields(this, table, m_force_sort_positions);
  }
  return m_sort_param.using_addon_fields();
}

/*
** functions to change a double or float to a sortable string
** The following should work for IEEE
*/

void change_double_for_sort(double nr, uchar *to) {
  /*
    -0.0 and +0.0 compare identically, so make sure they use exactly the same
    bit pattern.
  */
  if (nr == 0.0) nr = 0.0;

  /*
    Positive doubles sort exactly as ints; negative doubles need
    bit flipping. The bit flipping sets the upper bit to 0
    unconditionally, so put 1 in there for positive numbers
    (so they sort later for our unsigned comparison).
    NOTE: This does not sort infinities or NaN correctly.
  */
  int64 nr_int;
  memcpy(&nr_int, &nr, sizeof(nr));
  nr_int = (nr_int ^ (nr_int >> 63)) | ((~nr_int) & 0x8000000000000000ULL);

  // TODO: Make store64be() or similar.
  memcpy(to, &nr_int, sizeof(nr_int));
#if !defined(WORDS_BIGENDIAN)
  using std::swap;
  swap(to[0], to[7]);
  swap(to[1], to[6]);
  swap(to[2], to[5]);
  swap(to[3], to[4]);
#endif
}
