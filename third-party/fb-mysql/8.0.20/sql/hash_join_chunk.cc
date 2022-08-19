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

#include "sql/hash_join_chunk.h"

#include <cstring>

#include "my_base.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "scope_guard.h"
#include "sql/hash_join_buffer.h"
#include "sql/mysqld.h"
#include "sql/sql_base.h"
#include "sql/sql_const.h"

HashJoinChunk::HashJoinChunk(HashJoinChunk &&other)
    : m_tables(std::move(other.m_tables)),
      m_num_rows(other.m_num_rows),
      m_file(other.m_file),
      m_uses_match_flags(other.m_uses_match_flags) {
  // Reset the IO_CACHE structure so that the destructor doesn't close/clear the
  // file contents and it's buffers.
  new (&other.m_file) IO_CACHE();
}

HashJoinChunk &HashJoinChunk::operator=(HashJoinChunk &&other) {
  m_tables = std::move(other.m_tables);
  m_num_rows = other.m_num_rows;
  m_uses_match_flags = other.m_uses_match_flags;

  // Since the file we are replacing will become unreachable, free all resources
  // used by it.
  close_cached_file(&m_file);
  m_file = other.m_file;

  // Reset the IO_CACHE structure so that the destructor doesn't close/clear the
  // file contents and it's buffers.
  new (&other.m_file) IO_CACHE();
  return *this;
}

HashJoinChunk::~HashJoinChunk() { close_cached_file(&m_file); }

bool HashJoinChunk::Init(const hash_join_buffer::TableCollection &tables,
                         bool uses_match_flags) {
  m_tables = tables;
  m_file.file_key = key_file_hash_join;
  m_num_rows = 0;
  m_uses_match_flags = uses_match_flags;
  return open_cached_file(&m_file, mysql_tmpdir, TEMP_PREFIX, DISK_BUFFER_SIZE,
                          MYF(MY_WME));
}

bool HashJoinChunk::Rewind() {
  if (my_b_flush_io_cache(&m_file, /*need_append_buffer_lock=*/0) == -1 ||
      reinit_io_cache(&m_file, READ_CACHE, 0, false, false)) {
    my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
    return true;
  }

  return false;
}

bool HashJoinChunk::WriteRowToChunk(String *buffer, bool matched) {
  if (hash_join_buffer::StoreFromTableBuffers(m_tables, buffer)) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR),
             ComputeRowSizeUpperBound(m_tables));
    return true;
  }

  if (m_uses_match_flags) {
    if (my_b_write(&m_file, pointer_cast<const uchar *>(&matched),
                   sizeof(matched)) != 0) {
      my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
      return true;
    }
  }

  // Write out the length of the data.
  size_t data_length = buffer->length();
  if (my_b_write(&m_file, pointer_cast<const uchar *>(&data_length),
                 sizeof(data_length)) != 0) {
    my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
    return true;
  }

  // ... and then write the actual data.
  if (my_b_write(&m_file, pointer_cast<uchar *>(buffer->ptr()), data_length) !=
      0) {
    my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
    return true;
  }
  m_num_rows++;
  return false;
}

bool HashJoinChunk::LoadRowFromChunk(String *buffer, bool *matched) {
  if (m_uses_match_flags) {
    if (my_b_read(&m_file, pointer_cast<uchar *>(matched), sizeof(*matched)) !=
        0) {
      my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
      return true;
    }
  }

  // Read the length of the row.
  size_t row_length;
  if (my_b_read(&m_file, pointer_cast<uchar *>(&row_length),
                sizeof(row_length)) != 0) {
    my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
    return true;
  }

  // Read the actual data of the row.
  if (buffer->reserve(row_length)) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), row_length);
    return true;
  }

  buffer->length(row_length);
  if (my_b_read(&m_file, pointer_cast<uchar *>(buffer->ptr()), row_length) !=
      0) {
    my_error(ER_TEMP_FILE_WRITE_FAILURE, MYF(0));
    return true;
  }

  hash_join_buffer::LoadIntoTableBuffers(
      m_tables, {pointer_cast<const uchar *>(buffer->ptr()), buffer->length()});

  return false;
}
