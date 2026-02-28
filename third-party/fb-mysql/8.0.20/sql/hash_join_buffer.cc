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

#include "sql/hash_join_buffer.h"

#include <cstddef>
#include <cstring>
#include <iterator>
#include <new>
#include <unordered_map>

#include "field_types.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item_cmpfunc.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_executor.h"
#include "sql/sql_join_buffer.h"
#include "sql/sql_optimizer.h"
#include "sql/table.h"
#include "tables_contained_in.h"
#include "template_utils.h"

namespace hash_join_buffer {

Column::Column(Field *field) : field(field), field_type(field->real_type()) {}

// Take in a QEP_TAB and extract the columns that are needed to satisfy the SQL
// query (determined by the read set of the table).
Table::Table(QEP_TAB *qep_tab)
    : qep_tab(qep_tab), columns(PSI_NOT_INSTRUMENTED) {
  const TABLE *table = qep_tab->table();
  for (uint i = 0; i < table->s->fields; ++i) {
    if (bitmap_is_set(table->read_set, i)) {
      columns.emplace_back(table->field[i]);
    }
  }

  // Cache the value of rowid_status, the value may be changed by other
  // iterators. See QEP_TAB::rowid_status for more details.
  rowid_status = qep_tab->rowid_status;
}

// Take a set of tables involed in a hash join and extract the columns that are
// needed to satisfy the SQL query. Note that we might very well include a table
// with no columns, like t2 in the following query:
//
//   SELECT t1.col1 FROM t1, t2;  # t2 will be included without any columns.
TableCollection::TableCollection(const JOIN *join, qep_tab_map tables) {
  for (QEP_TAB *qep_tab : TablesContainedIn(join, tables)) {
    AddTable(qep_tab);
  }
}

void TableCollection::AddTable(QEP_TAB *qep_tab) {
  m_tables_bitmap |= qep_tab->table_ref->map();

  // When constructing the iterator tree, we might end up adding a
  // WeedoutIterator _after_ a HashJoinIterator has been constructed.
  // When adding the WeedoutIterator, QEP_TAB::rowid_status will be changed
  // indicate that a row ID is needed. A side effect of this is that
  // rowid_status might say that no row ID is needed here, while it says
  // otherwise while hash join is executing. As such, we may write outside of
  // the allocated buffers since we did not take the size of the row ID into
  // account here. To overcome this, we always assume that the row ID should
  // be kept; reserving some extra bytes in a few buffers should not be an
  // issue.
  m_ref_and_null_bytes_size += qep_tab->table()->file->ref_length;

  if (qep_tab->table()->is_nullable()) {
    m_ref_and_null_bytes_size += sizeof(qep_tab->table()->null_row);
  }

  Table table(qep_tab);
  for (const hash_join_buffer::Column &column : table.columns) {
    // Field_typed_array will mask away the BLOB_FLAG for all types. Hence,
    // we will treat all Field_typed_array as blob columns.
    if ((column.field->flags & BLOB_FLAG) > 0 || column.field->is_array()) {
      m_has_blob_column = true;
    }

    // If a column is marked as nullable, we need to copy the NULL flags.
    if ((column.field->flags & NOT_NULL_FLAG) == 0) {
      table.copy_null_flags = true;
    }

    // BIT fields stores some of its data in the NULL flags of the table. So
    // if we have a BIT field, we must copy the NULL flags.
    if (column.field->type() == MYSQL_TYPE_BIT &&
        down_cast<const Field_bit *>(column.field)->bit_len > 0) {
      table.copy_null_flags = true;
    }
  }

  if (table.copy_null_flags) {
    m_ref_and_null_bytes_size += qep_tab->table()->s->null_bytes;
  }

  m_tables.push_back(table);
}

// Calculate how many bytes the data in the column uses. We don't bother
// calculating the exact size for all types, since we consider reserving some
// extra bytes in buffers harmless. In particular, as long as the column is not
// of type BLOB, TEXT, JSON or GEOMETRY, we return an upper bound of the storage
// size. In the case of said types, we return the actual storage size; we do not
// want to return 4 gigabytes for a BLOB column if it only contains 10 bytes of
// data.
static size_t CalculateColumnStorageSize(const Column &column) {
  bool is_blob_column = false;
  switch (column.field_type) {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_STRING:
      // Field_typed_array inherits from Field_blob, so we have to treat it as a
      // BLOB column. And is_array() the only way to detect if the field is a
      // typed array.
      is_blob_column = column.field->is_array();
      break;
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB: {
      is_blob_column = true;
      break;
    }
    case MYSQL_TYPE_TYPED_ARRAY: {
      // This type is only used for replication, so it should not occur here.
      DBUG_ASSERT(false);
      return 0;
    }
  }

  if (is_blob_column) {
    // If we have a BLOB type, look at the actual length instead of taking the
    // upper length, which could happen to be 4GB. Note that data_length()
    // does not include the size of the length variable for blob types, so we
    // have to add that ourselves.
    const Field_blob *field_blob = down_cast<const Field_blob *>(column.field);
    return field_blob->data_length() + field_blob->pack_length_no_ptr();
  }

  return column.field->max_data_length();
}

size_t ComputeRowSizeUpperBound(const TableCollection &tables) {
  size_t total_size = tables.ref_and_null_bytes_size();
  for (const Table &table : tables.tables()) {
    for (const Column &column : table.columns) {
      // Even though we only store non-null columns, we count up the size of all
      // columns unconditionally. This means that NULL columns may very well be
      // counted here, but the only effect is that we end up reserving a bit too
      // much space in the buffer for holding the row data. That is more welcome
      // than having to call Field::is_null() for every column in every row.
      total_size += CalculateColumnStorageSize(column);
    }
  }

  return total_size;
}

static bool ShouldCopyRowId(const hash_join_buffer::Table &tbl) {
  // It is not safe to copy the row ID if we have a NULL-complemented row; the
  // value is undefined, or the buffer location can even be nullptr.
  const TABLE *table = tbl.qep_tab->table();
  return tbl.rowid_status != NO_ROWID_NEEDED && !table->const_table &&
         !(table->is_nullable() && table->null_row);
}

bool StoreFromTableBuffers(const TableCollection &tables, String *buffer) {
  buffer->length(0);

  if (tables.has_blob_column()) {
    const size_t upper_data_length = ComputeRowSizeUpperBound(tables);
    if (buffer->alloced_length() < upper_data_length + buffer->length() &&
        buffer->reserve(upper_data_length)) {
      return true;
    }
  } else {
    // If the table doesn't have any blob columns, we expect that the caller
    // already has reserved enough space in the provided buffer.
    DBUG_ASSERT(buffer->alloced_length() >= ComputeRowSizeUpperBound(tables));
  }

  uchar *dptr = pointer_cast<uchar *>(buffer->ptr());
  for (const Table &tbl : tables.tables()) {
    const TABLE *table = tbl.qep_tab->table();

    // Store the NULL flags.
    if (tbl.copy_null_flags) {
      memcpy(dptr, table->null_flags, table->s->null_bytes);
      dptr += table->s->null_bytes;
    }

    if (tbl.qep_tab->table()->is_nullable()) {
      const size_t null_row_size = sizeof(tbl.qep_tab->table()->null_row);
      memcpy(dptr, pointer_cast<const uchar *>(&tbl.qep_tab->table()->null_row),
             null_row_size);
      dptr += null_row_size;
    }

    if (ShouldCopyRowId(tbl)) {
      // Store the row ID, since it is needed by weedout.
      memcpy(dptr, table->file->ref, table->file->ref_length);
      dptr += table->file->ref_length;
    }

    for (const Column &column : tbl.columns) {
      DBUG_ASSERT(bitmap_is_set(column.field->table->read_set,
                                column.field->field_index));
      if (!column.field->is_null()) {
        // Store the data in packed format. The packed format will also
        // include the length of the data if needed.
        dptr = column.field->pack(dptr, column.field->ptr);
      }
    }
  }

  DBUG_ASSERT(dptr <=
              pointer_cast<uchar *>(buffer->ptr()) + buffer->alloced_length());
  const size_t actual_length = dptr - pointer_cast<uchar *>(buffer->ptr());
  buffer->length(actual_length);
  return false;
}

// Take the contents of this row and put it back in the tables' record buffers
// (record[0]). The row ID and NULL flags will also be restored, if needed.
// Returns a pointer to where we ended reading.
const uchar *LoadIntoTableBuffers(const TableCollection &tables,
                                  const uchar *ptr) {
  for (const Table &tbl : tables.tables()) {
    TABLE *table = tbl.qep_tab->table();

    // If the NULL row flag is set, it may override the NULL flags for the
    // columns. This may in turn cause columns not to be restored when they
    // should, so clear the NULL row flag when restoring the row.
    table->reset_null_row();

    if (tbl.copy_null_flags) {
      memcpy(table->null_flags, ptr, table->s->null_bytes);
      ptr += table->s->null_bytes;
    }

    if (tbl.qep_tab->table()->is_nullable()) {
      const size_t null_row_size = sizeof(tbl.qep_tab->table()->null_row);
      memcpy(pointer_cast<uchar *>(&tbl.qep_tab->table()->null_row), ptr,
             null_row_size);
      ptr += null_row_size;
    }

    if (ShouldCopyRowId(tbl)) {
      memcpy(table->file->ref, ptr, table->file->ref_length);
      ptr += table->file->ref_length;
    }

    for (const Column &column : tbl.columns) {
      if (!column.field->is_null()) {
        ptr = column.field->unpack(column.field->ptr, ptr);
      }
    }
  }
  return ptr;
}

// A convenience form of the above that also verifies the end pointer for us.
void LoadIntoTableBuffers(const TableCollection &tables, BufferRow row) {
  const uchar *end MY_ATTRIBUTE((unused)) =
      LoadIntoTableBuffers(tables, row.data());
  DBUG_ASSERT(end == row.data() + row.size());
}

HashJoinRowBuffer::HashJoinRowBuffer(
    TableCollection tables, std::vector<HashJoinCondition> join_conditions,
    size_t max_mem_available)
    : m_join_conditions(move(join_conditions)),
      m_tables(std::move(tables)),
      m_mem_root(key_memory_hash_join, 16384 /* 16 kB */),
      m_hash_map(nullptr),
      m_max_mem_available(
          std::max<size_t>(max_mem_available, 16384 /* 16 kB */)) {}

bool HashJoinRowBuffer::Init(std::uint32_t hash_seed) {
  if (m_hash_map.get() != nullptr) {
    // Reset the iterator before clearing the data it may point to. Some
    // platforms (Windows in particular) will access the old data the iterator
    // pointed to in the assignment operator. So if we do not clear the
    // iterator state, the assignment operator may access uninitialized data.
    m_last_row_stored = hash_map_iterator();

    // Reset the unique_ptr, so that the hash map destructors are called before
    // clearing the MEM_ROOT.
    m_hash_map.reset(nullptr);
    m_mem_root.Clear();

    // Now that the destructors are finished and the MEM_ROOT is cleared,
    // we can allocate a new hash map.
  }

  if (!m_tables.has_blob_column()) {
    const size_t row_size_upper_bound = ComputeRowSizeUpperBound(m_tables);
    if (m_buffer.reserve(row_size_upper_bound)) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), row_size_upper_bound);
      return true;  // oom
    }
  }

  m_hash_map.reset(new (&m_mem_root)
                       hash_map_type(&m_mem_root, KeyHasher(hash_seed)));
  if (m_hash_map == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(hash_map_type));
    return true;
  }

  m_last_row_stored = m_hash_map->end();
  return false;
}

StoreRowResult HashJoinRowBuffer::StoreRow(
    THD *thd, bool reject_duplicate_keys,
    bool store_rows_with_null_in_condition) {
  // Make the key from the join conditions.
  m_buffer.length(0);
  for (const HashJoinCondition &hash_join_condition : m_join_conditions) {
    bool null_in_join_condition =
        hash_join_condition.join_condition()->append_join_key_for_hash_join(
            thd, m_tables.tables_bitmap(), hash_join_condition, &m_buffer);

    if (null_in_join_condition && !store_rows_with_null_in_condition) {
      // SQL NULL values will never match in an inner join or semijoin, so skip
      // the row.
      return StoreRowResult::ROW_STORED;
    }
  }

  // TODO(efroseth): We should probably use an unordered_map instead of multimap
  // for these cases so we do not have to hash and lookup twice.
  if (reject_duplicate_keys &&
      contains(Key(pointer_cast<const uchar *>(m_buffer.ptr()),
                   m_buffer.length()))) {
    return StoreRowResult::ROW_STORED;
  }

  // Allocate the join key on the same MEM_ROOT that the hash table is
  // allocated on, so it has the same lifetime as the rest of the contents in
  // the hash map (until Clear() is called on the HashJoinBuffer).
  const size_t join_key_size = m_buffer.length();
  uchar *join_key_data = nullptr;
  if (join_key_size > 0) {
    join_key_data = m_mem_root.ArrayAlloc<uchar>(join_key_size);
    if (join_key_data == nullptr) {
      return StoreRowResult::FATAL_ERROR;
    }
    memcpy(join_key_data, m_buffer.ptr(), join_key_size);
  }

  // Save the contents of all columns marked for reading.
  if (StoreFromTableBuffers(m_tables, &m_buffer)) {
    return StoreRowResult::FATAL_ERROR;
  }

  // Give the row the same lifetime as the hash map, by allocating it on the
  // same MEM_ROOT as the hash map is allocated on.
  const size_t row_size = m_buffer.length();
  uchar *row = nullptr;
  if (row_size > 0) {
    row = m_mem_root.ArrayAlloc<uchar>(row_size);
    if (row == nullptr) {
      return StoreRowResult::FATAL_ERROR;
    }
    memcpy(row, m_buffer.ptr(), row_size);
  }

  m_last_row_stored = m_hash_map->emplace(Key(join_key_data, join_key_size),
                                          BufferRow(row, row_size));

  if (m_mem_root.allocated_size() > m_max_mem_available) {
    return StoreRowResult::BUFFER_FULL;
  }
  return StoreRowResult::ROW_STORED;
}

}  // namespace hash_join_buffer
