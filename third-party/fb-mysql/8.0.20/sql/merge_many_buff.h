/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MERGE_MANY_BUFF_INCLUDED
#define MERGE_MANY_BUFF_INCLUDED

#include <algorithm>

#include "my_dbug.h"
#include "my_sys.h"        // open_cached_file
#include "sql/mysqld.h"    // mysql_tmpdir
#include "sql/sql_base.h"  // TEMP_PREFIX
#include "sql/sql_sort.h"  // Sort_buffer

class THD;

/**
  Merges buffers to make < MERGEBUFF2 buffers.

  @param thd
  @param param        Sort parameters.
  @param sort_buffer  The main memory buffer.
  @param chunk_array  Array of chunk descriptors to merge.
  @param [out] p_num_chunks The number of chunks left in the output file.
  @param [out] t_file Where to store the result.

  @returns   false if success, true if error
*/

template <typename Merge_param>
bool merge_many_buff(THD *thd, Merge_param *param, Sort_buffer sort_buffer,
                     Merge_chunk_array chunk_array, size_t *p_num_chunks,
                     IO_CACHE *t_file) {
  IO_CACHE t_file2;
  DBUG_TRACE;

  size_t num_chunks = chunk_array.size();
  *p_num_chunks = num_chunks;

  if (num_chunks <= MERGEBUFF2) return false; /* purecov: inspected */

  if (flush_io_cache(t_file) ||
      filesort_open_cached_file(&t_file2, mysql_tmpdir, TEMP_PREFIX,
                                DISK_BUFFER_SIZE, MYF(MY_WME)))
    return true; /* purecov: inspected */

  IO_CACHE *from_file = t_file;
  IO_CACHE *to_file = &t_file2;

  while (num_chunks > MERGEBUFF2) {
    if (reinit_io_cache(from_file, READ_CACHE, 0L, false, false)) goto cleanup;
    if (reinit_io_cache(to_file, WRITE_CACHE, 0L, false, false)) goto cleanup;
    Merge_chunk *last_chunk = chunk_array.begin();
    uint i;
    for (i = 0; i < num_chunks - MERGEBUFF * 3U / 2U; i += MERGEBUFF) {
      if (merge_buffers(thd, param, from_file, to_file, sort_buffer,
                        last_chunk++,
                        Merge_chunk_array(&chunk_array[i], MERGEBUFF),
                        /*include_keys=*/true))
        goto cleanup;
    }
    if (merge_buffers(thd, param, from_file, to_file, sort_buffer, last_chunk++,
                      Merge_chunk_array(&chunk_array[i], num_chunks - i),
                      /*include_keys=*/true))
      break;                            /* purecov: inspected */
    if (flush_io_cache(to_file)) break; /* purecov: inspected */

    std::swap(from_file, to_file);
    setup_io_cache(from_file);
    setup_io_cache(to_file);
    num_chunks = last_chunk - chunk_array.begin();
  }
cleanup:
  close_cached_file(to_file);  // This holds old result
  if (to_file == t_file) {
    *t_file = t_file2;  // Copy result file
    setup_io_cache(t_file);
  }

  *p_num_chunks = num_chunks;
  return num_chunks > MERGEBUFF2; /* Return true if interrupted */
} /* merge_many_buff */

#endif  // MERGE_MANY_BUFF_INCLUDED
