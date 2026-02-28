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

#include "sql/sorting_iterator.h"

#include <stdio.h>
#include <sys/types.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <new>

#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/basic_row_iterators.h"
#include "sql/field.h"
#include "sql/filesort.h"  // Filesort
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/mysqld.h"  // stage_executing
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/sort_param.h"
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_executor.h"
#include "sql/sql_lex.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_optimizer.h"  // JOIN
#include "sql/sql_show.h"       // get_schema_tables_result
#include "sql/sql_sort.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "thr_lock.h"
#include "varlen_sort.h"

using std::string;
using std::vector;

SortFileIndirectIterator::SortFileIndirectIterator(THD *thd, TABLE *table,
                                                   IO_CACHE *tempfile,
                                                   bool request_cache,
                                                   bool ignore_not_found_rows,
                                                   ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_io_cache(tempfile),
      m_examined_rows(examined_rows),
      m_record(table->record[0]),
      m_ref_pos(table->file->ref),
      m_ignore_not_found_rows(ignore_not_found_rows),
      m_using_cache(request_cache),
      m_ref_length(table->file->ref_length) {}

SortFileIndirectIterator::~SortFileIndirectIterator() {
  (void)table()->file->ha_index_or_rnd_end();

  close_cached_file(m_io_cache);
  my_free(m_io_cache);
}

bool SortFileIndirectIterator::Init() {
  // The sort's source iterator could have initialized an index
  // read, and it won't call end until it's destroyed (which we
  // can't do before destroying SortingIterator, since we may need
  // to scan/sort multiple times). Thus, as a small hack, we need
  // to reset it here.
  table()->file->ha_index_or_rnd_end();

  // Item_func_match::val_real() seemingly uses the existence of
  // table->file->ft_handler as check for whether the match score
  // is already present (which is the case when scanning the base
  // table, but not when running this iterator), so we need to
  // clear it out.
  table()->file->ft_end();

  int error = table()->file->ha_rnd_init(false);
  if (error) {
    PrintError(error);
    return true;
  }

  if (m_using_cache && thd()->variables.read_rnd_buff_size &&
      !(table()->file->ha_table_flags() & HA_FAST_KEY_READ) &&
      (table()->db_stat & HA_READ_ONLY ||
       table()->reginfo.lock_type <= TL_READ_NO_INSERT) &&
      (ulonglong)table()->s->reclength *
              (table()->file->stats.records + table()->file->stats.deleted) >
          (ulonglong)MIN_FILE_LENGTH_TO_USE_ROW_CACHE &&
      m_io_cache->end_of_file / m_ref_length * table()->s->reclength >
          (my_off_t)MIN_ROWS_TO_USE_TABLE_CACHE &&
      !table()->s->blob_fields && m_ref_length <= MAX_REFLENGTH) {
    m_using_cache = !InitCache();
  } else {
    m_using_cache = false;
  }

  DBUG_PRINT("info", ("using cache: %d", m_using_cache));
  return false;
}

/**
  Initialize caching of records from temporary file.

  @retval
    false OK, use caching.
    true  Buffer is too small, or cannot be allocated.
          Skip caching, and read records directly from temporary file.
 */
bool SortFileIndirectIterator::InitCache() {
  m_struct_length = 3 + MAX_REFLENGTH;
  m_reclength = ALIGN_SIZE(table()->s->reclength + 1);
  if (m_reclength < m_struct_length) m_reclength = ALIGN_SIZE(m_struct_length);

  m_error_offset = table()->s->reclength;
  m_cache_records =
      thd()->variables.read_rnd_buff_size / (m_reclength + m_struct_length);
  uint rec_cache_size = m_cache_records * m_reclength;
  m_rec_cache_size = m_cache_records * m_ref_length;

  if (m_cache_records <= 2) {
    return true;
  }

  m_cache.reset((uchar *)my_malloc(
      key_memory_READ_RECORD_cache,
      rec_cache_size + m_cache_records * m_struct_length, MYF(0)));
  if (m_cache == nullptr) {
    return true;
  }
  DBUG_PRINT("info", ("Allocated buffer for %d records", m_cache_records));
  m_read_positions = m_cache.get() + rec_cache_size;
  m_cache_pos = m_cache_end = m_cache.get();
  return false;
}

int SortFileIndirectIterator::Read() {
  if (m_using_cache) {
    return CachedRead();
  } else {
    return UncachedRead();
  }
}

int SortFileIndirectIterator::UncachedRead() {
  for (;;) {
    if (my_b_read(m_io_cache, m_ref_pos, m_ref_length))
      return -1; /* End of file */
    int tmp = table()->file->ha_rnd_pos(m_record, m_ref_pos);
    if (tmp == 0) {
      if (m_examined_rows != nullptr) {
        ++*m_examined_rows;
      }
      return 0;
    }
    /* The following is extremely unlikely to happen */
    if (tmp == HA_ERR_RECORD_DELETED ||
        (tmp == HA_ERR_KEY_NOT_FOUND && m_ignore_not_found_rows))
      continue;
    return HandleError(tmp);
  }
}

int SortFileIndirectIterator::CachedRead() {
  uint i;
  ulong length;
  my_off_t rest_of_file;
  int16 error;
  uchar *position, *ref_position, *record_pos;
  ulong record;

  for (;;) {
    if (m_cache_pos != m_cache_end) {
      if (m_cache_pos[m_error_offset]) {
        error = shortget(m_cache_pos);
        if (error == HA_ERR_KEY_NOT_FOUND && m_ignore_not_found_rows) {
          m_cache_pos += m_reclength;
          continue;
        }
        PrintError(error);
      } else {
        error = 0;
        memcpy(m_record, m_cache_pos, (size_t)table()->s->reclength);
      }
      m_cache_pos += m_reclength;
      if (error == 0 && m_examined_rows != nullptr) {
        ++*m_examined_rows;
      }
      return ((int)error);
    }
    length = m_rec_cache_size;
    rest_of_file = m_io_cache->end_of_file - my_b_tell(m_io_cache);
    if ((my_off_t)length > rest_of_file) length = (ulong)rest_of_file;
    if (!length || my_b_read(m_io_cache, m_cache.get(), length)) {
      DBUG_PRINT("info", ("Found end of file"));
      return -1; /* End of file */
    }

    length /= m_ref_length;
    position = m_cache.get();
    ref_position = m_read_positions;
    for (i = 0; i < length; i++, position += m_ref_length) {
      memcpy(ref_position, position, (size_t)m_ref_length);
      ref_position += MAX_REFLENGTH;
      int3store(ref_position, (long)i);
      ref_position += 3;
    }
    size_t ref_length = m_ref_length;
    DBUG_ASSERT(ref_length <= MAX_REFLENGTH);
    varlen_sort(m_read_positions, m_read_positions + length * m_struct_length,
                m_struct_length, [ref_length](const uchar *a, const uchar *b) {
                  return memcmp(a, b, ref_length) < 0;
                });

    position = m_read_positions;
    for (i = 0; i < length; i++) {
      memcpy(m_ref_pos, position, (size_t)m_ref_length);
      position += MAX_REFLENGTH;
      record = uint3korr(position);
      position += 3;
      record_pos = m_cache.get() + record * m_reclength;
      error = (int16)table()->file->ha_rnd_pos(record_pos, m_ref_pos);
      if (error) {
        record_pos[m_error_offset] = 1;
        shortstore(record_pos, error);
        DBUG_PRINT("error", ("Got error: %d:%d when reading row", my_errno(),
                             (int)error));
      } else
        record_pos[m_error_offset] = 0;
    }
    m_cache_pos = m_cache.get();
    m_cache_end = m_cache_pos + length * m_reclength;
  }
}

vector<string> SortFileIndirectIterator::DebugString() const {
  // Not used, because sort result iterator is not decided at EXPLAIN time
  // (we can't know whether the buffer would stay in RAM or not).
  return {string("Read sorted data: Row IDs from file, records from ") +
          table()->alias};
}

template <bool Packed_addon_fields>
SortFileIterator<Packed_addon_fields>::SortFileIterator(THD *thd, TABLE *table,
                                                        IO_CACHE *tempfile,
                                                        Filesort_info *sort,
                                                        ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_rec_buf(sort->addon_fields->get_addon_buf()),
      m_ref_length(sort->addon_fields->get_addon_buf_length()),
      m_io_cache(tempfile),
      m_sort(sort),
      m_examined_rows(examined_rows) {}

template <bool Packed_addon_fields>
SortFileIterator<Packed_addon_fields>::~SortFileIterator() {
  close_cached_file(m_io_cache);
  my_free(m_io_cache);
}

/**
  Read a result set record from a temporary file after sorting.

  The function first reads the next sorted record from the temporary file.
  into a buffer. If a success it calls a callback function that unpacks
  the fields values use in the result set from this buffer into their
  positions in the regular record buffer.

  @tparam Packed_addon_fields Are the addon fields packed?
     This is a compile-time constant, to avoid if (....) tests during execution.
  @retval
    0   Record successfully read.
  @retval
    -1   There is no record to be read anymore.
*/
template <bool Packed_addon_fields>
int SortFileIterator<Packed_addon_fields>::Read() {
  uchar *destination = m_rec_buf;
  if (Packed_addon_fields) {
    const uint len_sz = Addon_fields::size_of_length_field;

    // First read length of the record.
    if (my_b_read(m_io_cache, destination, len_sz)) return -1;
    uint res_length = Addon_fields::read_addon_length(destination);
    DBUG_ASSERT(res_length > len_sz);
    DBUG_ASSERT(m_sort->using_addon_fields());

    // Then read the rest of the record.
    if (my_b_read(m_io_cache, destination + len_sz, res_length - len_sz))
      return -1; /* purecov: inspected */
  } else {
    if (my_b_read(m_io_cache, destination, m_ref_length)) return -1;
  }

  m_sort->unpack_addon_fields<Packed_addon_fields>(destination);

  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

template <bool Packed_addon_fields>
vector<string> SortFileIterator<Packed_addon_fields>::DebugString() const {
  // Not used, because sort result iterator is not decided at EXPLAIN time
  // (we can't know whether the buffer would stay in RAM or not).
  return {string("Read sorted data from file (originally from ") +
          table()->alias + ")"};
}

template <bool Packed_addon_fields>
SortBufferIterator<Packed_addon_fields>::SortBufferIterator(
    THD *thd, TABLE *table, Filesort_info *sort, Sort_result *sort_result,
    ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_sort(sort),
      m_sort_result(sort_result),
      m_examined_rows(examined_rows) {}

template <bool Packed_addon_fields>
SortBufferIterator<Packed_addon_fields>::~SortBufferIterator() {
  m_sort_result->sorted_result.reset();
  m_sort_result->sorted_result_in_fsbuf = false;
}

template <bool Packed_addon_fields>
bool SortBufferIterator<Packed_addon_fields>::Init() {
  m_unpack_counter = 0;
  return false;
}

/**
  Read a result set record from a buffer after sorting.

  Get the next record from the filesort buffer,
  then unpack the fields into their positions in the regular record buffer.

  @tparam Packed_addon_fields Are the addon fields packed?
     This is a compile-time constant, to avoid if (....) tests during execution.

  TODO: consider templatizing on is_varlen as well.
  Variable / Fixed size key is currently handled by
  Filesort_info::get_start_of_payload

  @retval
    0   Record successfully read.
  @retval
    -1   There is no record to be read anymore.
*/
template <bool Packed_addon_fields>
int SortBufferIterator<Packed_addon_fields>::Read() {
  if (m_unpack_counter ==
      m_sort_result->found_records)  // XXX send in as a parameter?
    return -1;                       /* End of buffer */

  uchar *record = m_sort->get_sorted_record(m_unpack_counter++);
  uchar *payload = get_start_of_payload(m_sort, record);
  m_sort->unpack_addon_fields<Packed_addon_fields>(payload);
  if (m_examined_rows != nullptr) {
    ++*m_examined_rows;
  }
  return 0;
}

template <bool Packed_addon_fields>
vector<string> SortBufferIterator<Packed_addon_fields>::DebugString() const {
  // Not used, because sort result iterator is not decided at EXPLAIN time
  // (we can't know whether the buffer would stay in RAM or not).
  return {string("Read sorted data from memory (originally from ") +
          table()->alias + ")"};
}

SortBufferIndirectIterator::SortBufferIndirectIterator(
    THD *thd, TABLE *table, Sort_result *sort_result,
    bool ignore_not_found_rows, ha_rows *examined_rows)
    : TableRowIterator(thd, table),
      m_sort_result(sort_result),
      m_ref_length(table->file->ref_length),
      m_examined_rows(examined_rows),
      m_record(table->record[0]),
      m_ignore_not_found_rows(ignore_not_found_rows) {}

SortBufferIndirectIterator::~SortBufferIndirectIterator() {
  m_sort_result->sorted_result.reset();
  DBUG_ASSERT(!m_sort_result->sorted_result_in_fsbuf);
  m_sort_result->sorted_result_in_fsbuf = false;

  (void)table()->file->ha_index_or_rnd_end();
}

bool SortBufferIndirectIterator::Init() {
  // The sort's source iterator could have initialized an index
  // read, and it won't call end until it's destroyed (which we
  // can't do before destroying SortingIterator, since we may need
  // to scan/sort multiple times). Thus, as a small hack, we need
  // to reset it here.
  table()->file->ha_index_or_rnd_end();

  // Item_func_match::val_real() seemingly uses the existence of
  // table->file->ft_handler as check for whether the match score
  // is already present (which is the case when scanning the base
  // table, but not when running this iterator), so we need to
  // clear it out.
  table()->file->ft_end();

  int error = table()->file->ha_rnd_init(false);
  if (error) {
    PrintError(error);
    return true;
  }
  m_cache_pos = m_sort_result->sorted_result.get();
  m_cache_end =
      m_cache_pos + m_sort_result->found_records * table()->file->ref_length;
  return false;
}

int SortBufferIndirectIterator::Read() {
  for (;;) {
    if (m_cache_pos == m_cache_end) return -1; /* End of file */
    uchar *cache_pos = m_cache_pos;
    m_cache_pos += m_ref_length;

    int tmp = table()->file->ha_rnd_pos(m_record, cache_pos);
    if (tmp == 0) {
      if (m_examined_rows != nullptr) {
        ++*m_examined_rows;
      }
      return 0;
    }

    /* The following is extremely unlikely to happen */
    if (tmp == HA_ERR_RECORD_DELETED ||
        (tmp == HA_ERR_KEY_NOT_FOUND && m_ignore_not_found_rows))
      continue;
    return HandleError(tmp);
  }
}

vector<string> SortBufferIndirectIterator::DebugString() const {
  // Not used, because sort result iterator is not decided at EXPLAIN time
  // (we can't know whether the buffer would stay in RAM or not).
  return {string("Read sorted data: Row IDs from memory, records from ") +
          table()->alias};
}

SortingIterator::SortingIterator(THD *thd, QEP_TAB *qep_tab, Filesort *filesort,
                                 unique_ptr_destroy_only<RowIterator> source,
                                 ha_rows *examined_rows)
    : RowIterator(thd),
      m_filesort(filesort),
      m_qep_tab(qep_tab),
      m_source_iterator(move(source)),
      m_examined_rows(examined_rows) {}

SortingIterator::~SortingIterator() {
  ReleaseBuffers();
  CleanupAfterQuery();
}

void SortingIterator::CleanupAfterQuery() {
  m_fs_info.free_sort_buffer();
  my_free(m_fs_info.merge_chunks.array());
  m_fs_info.merge_chunks = Merge_chunk_array(nullptr, 0);
  m_fs_info.addon_fields = nullptr;
}

void SortingIterator::ReleaseBuffers() {
  m_result_iterator.reset();
  if (m_sort_result.io_cache) {
    // NOTE: The io_cache is only owned by us if it were never used.
    close_cached_file(m_sort_result.io_cache);
    my_free(m_sort_result.io_cache);
    m_sort_result.io_cache = nullptr;
  }
  m_sort_result.sorted_result.reset();
  m_sort_result.sorted_result_in_fsbuf = false;

  // Keep the sort buffer in m_fs_info.
}

bool SortingIterator::Init() {
  ReleaseBuffers();

  // Both empty result and error count as errors. (TODO: Why? This is a legacy
  // choice that doesn't always seem right to me, although it should nearly
  // never happen in practice.)
  if (DoSort() != 0) return true;

  // Prepare the result iterator for actually reading the data. Read()
  // will proxy to it.
  TABLE *table = m_filesort->table;
  if (m_sort_result.io_cache && my_b_inited(m_sort_result.io_cache)) {
    // Test if ref-records was used
    if (m_fs_info.using_addon_fields()) {
      DBUG_PRINT("info", ("using SortFileIterator"));
      if (m_fs_info.addon_fields->using_packed_addons())
        m_result_iterator.reset(
            new (&m_result_iterator_holder.sort_file_packed_addons)
                SortFileIterator<true>(thd(), table, m_sort_result.io_cache,
                                       &m_fs_info, m_examined_rows));
      else
        m_result_iterator.reset(
            new (&m_result_iterator_holder.sort_file)
                SortFileIterator<false>(thd(), table, m_sort_result.io_cache,
                                        &m_fs_info, m_examined_rows));
    } else {
      /*
        m_fs_info->addon_field is checked because if we use addon fields,
        it doesn't make sense to use cache - we don't read from the table
        and m_fs_info->io_cache is read sequentially
      */
      bool request_cache = !m_fs_info.using_addon_fields();
      m_result_iterator.reset(
          new (&m_result_iterator_holder.sort_file_indirect)
              SortFileIndirectIterator(
                  thd(), table, m_sort_result.io_cache, request_cache,
                  /*ignore_not_found_rows=*/false, m_examined_rows));
    }
    m_sort_result.io_cache =
        nullptr;  // The result iterator has taken ownership.
  } else {
    DBUG_ASSERT(m_sort_result.has_result_in_memory());
    if (m_fs_info.using_addon_fields()) {
      DBUG_PRINT("info", ("using SortBufferIterator"));
      DBUG_ASSERT(m_sort_result.sorted_result_in_fsbuf);
      if (m_fs_info.addon_fields->using_packed_addons())
        m_result_iterator.reset(
            new (&m_result_iterator_holder.sort_buffer_packed_addons)
                SortBufferIterator<true>(thd(), table, &m_fs_info,
                                         &m_sort_result, m_examined_rows));
      else
        m_result_iterator.reset(
            new (&m_result_iterator_holder.sort_buffer)
                SortBufferIterator<false>(thd(), table, &m_fs_info,
                                          &m_sort_result, m_examined_rows));
    } else {
      DBUG_PRINT("info", ("using SortBufferIndirectIterator (sort)"));
      m_result_iterator.reset(
          new (&m_result_iterator_holder.sort_buffer_indirect)
              SortBufferIndirectIterator(thd(), table, &m_sort_result,
                                         /*ignore_not_found_rows=*/false,
                                         m_examined_rows));
    }
  }

  return m_result_iterator->Init();
}

/*
  Do the actual sort, by calling filesort. The result will be left in one of
  several places depending on what sort strategy we chose; it is up to Init() to
  figure out what happened and create the appropriate iterator to read from it.

  RETURN VALUES
    0		ok
    -1		Some fatal error
    1		No records
*/

int SortingIterator::DoSort() {
  DBUG_ASSERT(m_sort_result.io_cache == nullptr);
  m_sort_result.io_cache =
      (IO_CACHE *)my_malloc(key_memory_TABLE_sort_io_cache, sizeof(IO_CACHE),
                            MYF(MY_WME | MY_ZEROFILL));

  if (m_qep_tab != nullptr) {
    JOIN *join = m_qep_tab->join();
    if (join != nullptr && join->unit->root_iterator() == nullptr) {
      /* Fill schema tables with data before filesort if it's necessary */
      if ((join->select_lex->active_options() & OPTION_SCHEMA_TABLE) &&
          get_schema_tables_result(join, PROCESSED_BY_CREATE_SORT_INDEX))
        return -1;
    }
  }

  ha_rows found_rows;
  bool error = filesort(thd(), m_filesort, m_source_iterator.get(), &m_fs_info,
                        &m_sort_result, &found_rows);
  if (m_qep_tab != nullptr) {
    m_qep_tab->set_records(found_rows);  // For SQL_CALC_FOUND_ROWS
  }
  m_filesort->table->set_keyread(false);  // Restore if we used indexes
  return error;
}

template <bool Packed_addon_fields>
inline void Filesort_info::unpack_addon_fields(uchar *buff) {
  Sort_addon_field *addonf = addon_fields->begin();

  const uchar *start_of_record = buff + addonf->offset;

  for (; addonf != addon_fields->end(); ++addonf) {
    Field *field = addonf->field;
    if (addonf->null_bit && (addonf->null_bit & buff[addonf->null_offset])) {
      field->set_null();
      continue;
    }
    field->set_notnull();
    if (Packed_addon_fields)
      start_of_record = field->unpack(field->ptr, start_of_record);
    else
      field->unpack(field->ptr, buff + addonf->offset);
  }
}

vector<string> SortingIterator::DebugString() const {
  string ret;
  if (m_filesort->using_addon_fields()) {
    ret = "Sort";
  } else {
    ret = "Sort row IDs";
  }
  if (m_filesort->m_remove_duplicates) {
    ret += " with duplicate removal: ";
  } else {
    ret += ": ";
  }

  bool first = true;
  for (unsigned i = 0; i < m_filesort->sort_order_length(); ++i) {
    if (first) {
      first = false;
    } else {
      ret += ", ";
    }

    const st_sort_field *order = &m_filesort->sortorder[i];
    ret += ItemToString(order->item);
    if (order->reverse) {
      ret += " DESC";
    }
  }
  if (m_filesort->limit != HA_POS_ERROR) {
    char buf[256];
    snprintf(buf, sizeof(buf), ", limit input to %llu row(s) per chunk",
             m_filesort->limit);
    ret += buf;
  }

  return {ret};
}
