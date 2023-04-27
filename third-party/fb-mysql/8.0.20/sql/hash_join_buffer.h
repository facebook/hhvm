#ifndef SQL_HASH_JOIN_BUFFER_H_
#define SQL_HASH_JOIN_BUFFER_H_

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

/// @file
///
/// This file contains the HashJoinRowBuffer class and related
/// functions/classes.
///
/// A HashJoinBuffer is a row buffer that can hold a certain amount of rows.
/// The rows are stored in a hash table, which allows for constant-time lookup.
/// The HashJoinBuffer maintains its own internal MEM_ROOT, where all of the
/// data is allocated.
///
/// The HashJoinBuffer contains an operand with rows from one or more tables,
/// keyed on the value we join on. Consider the following trivial example:
///
///   SELECT t1.data FROM t1 JOIN t2 ON (t1.key = t2.key);
///
/// Let us say that the table "t2" is stored in a HashJoinBuffer. In this case,
/// the hash table key will be the value found in "t2.key", since that is the
/// join condition that belongs to t2. If we have multiple equalities, they
/// will be concatenated together in order to form the hash table key. The hash
/// table key is bascially an std::string_view, and should be replaced when we
/// go to C++17.
///
/// In order to store a row, we use the function StoreFromTableBuffers. See the
/// comments attached to the function for more details.
///
/// The amount of memory a HashJoinBuffer instance can use is limited by the
/// system variable "join_buffer_size". However, note that we check whether we
/// have exceeded the memory limit _after_ we have inserted data into the row
/// buffer. As such, we will probably use a little bit more memory than
/// specified by join_buffer_size.
///
/// Some basic profiling shows that the majority of the time is used on
/// constructing the hash table. It would be a future improvement to replace
/// std::unordered_multimap with a more performant hash table, as the literature
/// suggests that there are better alternatives (see e.g.
/// https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/).
///
/// The primary use case for these classes is, as the name implies,
/// for implementing hash join.

#include <stddef.h>
#include <stdint.h>
#include <memory>
#include <vector>

#include "extra/lz4/my_xxhash.h"
#include "field_types.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "prealloced_array.h"
#include "sql/item_cmpfunc.h"
#include "sql/table.h"
#include "sql_string.h"

class Field;
class QEP_TAB;

namespace hash_join_buffer {

/// A class that represents a field, which also holds a cached value of the
/// field's data type.
struct Column {
  explicit Column(Field *field);
  Field *const field;

  // The field type is used frequently, and caching it gains around 30% in some
  // of our microbenchmarks.
  const enum_field_types field_type;
};

/// This struct is primarily used for holding the extracted columns in a hash
/// join. When the hash join iterator is constructed, we extract the columns
/// that are needed to satisfy the SQL query.
struct Table {
  explicit Table(QEP_TAB *qep_tab);
  QEP_TAB *qep_tab;
  Prealloced_array<Column, 8> columns;

  // Whether to copy the NULL flags or not.
  bool copy_null_flags{false};

  // A cached value of QEP_TAB::rowid_status. This determines whether we need to
  // copy/restore the row ID for each row, and how we should retrieve the row ID
  // (i.e., should we call handler::position() or not). See the comment on
  // QEP_TAB::rowid_status for more details.
  rowid_statuses rowid_status;
};

/// A structure that contains a list of tables for the hash join operation,
/// and some pre-computed properties for the tables.
class TableCollection {
 public:
  TableCollection() = default;

  explicit TableCollection(QEP_TAB *qep_tab) {
    // Single table.
    AddTable(qep_tab);
  }

  TableCollection(const JOIN *join, qep_tab_map tables);  // Multiple tables.

  const Prealloced_array<Table, 4> &tables() const { return m_tables; }

  table_map tables_bitmap() const { return m_tables_bitmap; }

  size_t ref_and_null_bytes_size() const { return m_ref_and_null_bytes_size; }

  bool has_blob_column() const { return m_has_blob_column; }

 private:
  void AddTable(QEP_TAB *qep_tab);

  Prealloced_array<Table, 4> m_tables{PSI_NOT_INSTRUMENTED};

  // We frequently use the bitmap to determine which side of the join an Item
  // belongs to, so precomputing the bitmap saves quite some time.
  table_map m_tables_bitmap = 0;

  // Sum of the NULL bytes and the row ID for all of the tables.
  size_t m_ref_and_null_bytes_size = 0;

  // Whether any of the tables has a BLOB/TEXT column. This is used to determine
  // whether we need to estimate the row size every time we store a row to the
  // row buffer or to a chunk file on disk. If this is set to false, we can
  // pre-allocate any necessary buffers we need during the hash join, and thus
  // eliminate the need for recalculating the row size every time.
  bool m_has_blob_column = false;
};

/// Count up how many bytes a single row from the given tables will occupy,
/// in "packed" format. Note that this is an upper bound, so the length after
/// calling Field::pack may very well be shorter than the size returned by this
/// function.
///
/// The value returned from this function will sum up
/// 1) The row-id if that is to be kept.
/// 2) Size of the NULL flags.
/// 3) Size of the buffer returned by pack() on all columns marked in the
///    read_set.
///
/// Note that if any of the tables has a BLOB/TEXT column, this function looks
/// at the data stored in the record buffers. This means that the function can
/// not be called before reading any rows if tables.has_blob_column is true.
size_t ComputeRowSizeUpperBound(const TableCollection &tables);

/// The key type for the hash structure in HashJoinRowBuffer.
///
/// A key consists of the value from one or more columns, taken from the join
/// condition(s) in the query.  E.g., if the join condition is
/// (t1.col1 = t2.col1 AND t1.col2 = t2.col2), the key is (col1, col2), with the
/// two key parts concatenated together.
///
/// What the data actually contains depends on the comparison context for the
/// join condition. For instance, if the join condition is between a string
/// column and an integer column, the comparison will be done in a string
/// context, and thus the integers will be converted to strings before storing.
/// So the data we store in the key are in some cases converted, so that we can
/// hash and compare them byte-by-byte (i.e. decimals), while other types are
/// already comparable byte-by-byte (i.e. integers), and thus stored as-is.
///
/// Note that the key data can come from items as well as fields if the join
/// condition is an expression. E.g. if the join condition is
/// UPPER(t1.col1) = UPPER(t2.col1), the join key data will come from an Item
/// instead of a Field.
///
/// The Key class never takes ownership of the data. As such, the user must
/// ensure that the data has the proper lifetime. When storing rows in the row
/// buffer, the data must have the same lifetime as the row buffer itself.
/// When using the Key class for lookups in the row buffer, the same lifetime is
/// not needed; the key object is only needed when the lookup is done.
///
/// When we move to C++17, this class should most likely be replaced with
/// std::string_view.
class Key {
 public:
  /// Note that data can be nullptr if size is 0.
  Key(const uchar *data, size_t size) : m_data(data), m_size(size) {}

  bool operator==(const Key &other) const {
    if (other.size() != size()) {
      return false;
    } else if (size() == 0) {
      return true;
    }

    return memcmp(other.data(), data(), size()) == 0;
  }

  const uchar *data() const { return m_data; }

  size_t size() const { return m_size; }

 private:
  const uchar *m_data;
  const size_t m_size;
};

// A row in the hash join buffer is generally the same as the Key class. In
// C++17, both of them should most likely be replaced with std::string_view. For
// now, we create an alias to distinguish between Key and a row.
using BufferRow = Key;

/// We rely on xxHash64 to do the hashing of the key. xxHash64 was chosen as it
/// both is very fast _and_ produces reasonably good-quality hashes
/// (see https://github.com/rurban/smhasher).
class KeyHasher {
 public:
  explicit KeyHasher(uint32_t seed) : m_seed(seed) {}

  size_t operator()(hash_join_buffer::Key key) const {
    return MY_XXH64(key.data(), key.size(), m_seed);
  }

 private:
  const uint32_t m_seed;
};

/// Take the data marked for reading in "tables" and store it in the provided
/// buffer. What data to store is determined by the read set of each table.
/// Note that any existing data in "buffer" will be overwritten.
///
/// The output buffer will contain three things:
///
/// 1) NULL flags for each nullable column.
/// 2) The row ID for each row. This is only stored if QEP_TAB::rowid_status !=
///    NO_ROWID_NEEDED.
/// 3) The actual data from the columns.
///
/// @retval true if error, false otherwise
bool StoreFromTableBuffers(const TableCollection &tables, String *buffer);

/// Take the data in "ptr" and put it back to the tables' record buffers.
/// The tables must be _exactly_ the same as when the row was created.
/// That is, it must contain the same tables in the same order, and the read set
/// of each table must be identical when storing and restoring the row.
/// If that's not the case, you will end up with undefined and unpredictable
/// behavior.
///
/// Returns a pointer to where we ended reading.
const uchar *LoadIntoTableBuffers(const TableCollection &tables,
                                  const uchar *ptr);

// A convenience form of the above that also verifies the end pointer for us.
void LoadIntoTableBuffers(const TableCollection &tables, BufferRow row);

enum class StoreRowResult { ROW_STORED, BUFFER_FULL, FATAL_ERROR };

class HashJoinRowBuffer {
 public:
  // Construct the buffer. Note that Init() must be called before the buffer can
  // be used.
  HashJoinRowBuffer(TableCollection tables,
                    std::vector<HashJoinCondition> join_conditions,
                    size_t max_mem_available_bytes);

  // Initialize the HashJoinRowBuffer so it is ready to store rows. This
  // function can be called multiple times; subsequent calls will only clear the
  // buffer for existing rows.
  bool Init(uint32_t hash_seed);

  /// Store the row that is currently lying in the tables record buffers.
  /// The hash map key is extracted from the join conditions that the row buffer
  /// holds.
  ///
  /// @param thd the thread handler
  /// @param reject_duplicate_keys If true, reject rows with duplicate keys.
  ///        If a row is rejected, the function will still return ROW_STORED.
  /// @param store_rows_with_null_in_condition Whether to store rows where the
  ///        join conditions contains SQL NULL.
  ///
  /// @retval ROW_STORED the row was stored.
  /// @retval BUFFER_FULL the row was stored, and the buffer is full.
  /// @retval FATAL_ERROR an unrecoverable error occured (most likely,
  ///         malloc failed). It is the callers responsibility to call
  ///         my_error().
  StoreRowResult StoreRow(THD *thd, bool reject_duplicate_keys,
                          bool store_rows_with_null_in_condition);

  size_t size() const { return m_hash_map->size(); }

  bool empty() const { return m_hash_map->empty(); }

  using hash_map_type = mem_root_unordered_multimap<Key, BufferRow, KeyHasher>;

  using hash_map_iterator = hash_map_type::const_iterator;

  std::pair<hash_map_iterator, hash_map_iterator> equal_range(
      const Key &key) const {
    return m_hash_map->equal_range(key);
  }

  hash_map_iterator find(const Key &key) const { return m_hash_map->find(key); }

  hash_map_iterator begin() const { return m_hash_map->begin(); }

  hash_map_iterator end() const { return m_hash_map->end(); }

  hash_map_iterator LastRowStored() const {
    DBUG_ASSERT(Initialized());
    return m_last_row_stored;
  }

  bool Initialized() const { return m_hash_map.get() != nullptr; }

  bool contains(const Key &key) const { return find(key) != end(); }

 private:
  const std::vector<HashJoinCondition> m_join_conditions;

  // A row can consist of parts from different tables. This structure tells us
  // which tables that are involved.
  const TableCollection m_tables;

  // The MEM_ROOT on which all of the hash table data is allocated.
  MEM_ROOT m_mem_root;

  // The hash table where the rows are stored.
  unique_ptr_destroy_only<hash_map_type> m_hash_map;

  // A buffer we can use when we are constructing a join key from a join
  // condition. In order to avoid reallocating memory, the buffer never shrinks.
  String m_buffer;

  // The maximum size of the buffer, given in bytes.
  const size_t m_max_mem_available;

  // The last row that was stored in the hash table, or end() if the hash table
  // is empty. We may have to put this row back into the tables' record buffers
  // if we have a child iterator that expects the record buffers to contain the
  // last row returned by the storage engine (the probe phase of hash join may
  // put any row in the hash table in the tables' record buffer). See
  // HashJoinIterator::BuildHashTable() for an example of this.
  hash_map_iterator m_last_row_stored;
};

}  // namespace hash_join_buffer

#endif  // SQL_HASH_JOIN_BUFFER_H_
