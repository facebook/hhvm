#ifndef SQL_HASH_JOIN_ITERATOR_H_
#define SQL_HASH_JOIN_ITERATOR_H_

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

#include <stdio.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "my_alloc.h"
#include "my_inttypes.h"
#include "sql/hash_join_buffer.h"
#include "sql/hash_join_chunk.h"
#include "sql/item_cmpfunc.h"
#include "sql/mem_root_array.h"
#include "sql/row_iterator.h"
#include "sql/table.h"
#include "sql_string.h"

class THD;
class QEP_TAB;

struct ChunkPair {
  HashJoinChunk probe_chunk;
  HashJoinChunk build_chunk;
};

/// @file
///
/// An iterator for joining two inputs by using hashing to match rows from
/// the inputs.
///
/// The iterator starts out by doing everything in-memory. If everything fits
/// into memory, the joining algorithm for inner joins works like this:
///
/// 1) Designate one input as the "build" input and one input as the "probe"
/// input. Ideally, the smallest input measured in total size (not number of
/// rows) should be designated as the build input.
///
/// 2) Read all the rows from the build input into an in-memory hash table.
/// The hash key used in the hash table is calculated from the join attributes,
/// e.g., if we have the following query where "orders" is designated as the
/// build input:
///
///   SELECT * FROM lineitem
///     INNER JOIN orders ON orders.o_orderkey = lineitem.l_orderkey;
///
/// the hash value will be calculated from the values in the column
/// orders.o_orderkey. Note that the optimizer recognizes implicit join
/// conditions, so this also works for SQL statements like:
///
///   SELECT * FROM orders, lineitem
///     WHERE orders.o_orderkey = lineitem.l_orderkey;
///
/// 3) Then, we read the rows from the probe input, one by one. For each row,
/// a hash key is calculated for the other side of the join (the probe input)
/// using the join attribute (lineitem.l_orderkey in the above example) and the
/// same hash function as in step 2. This hash key is used to do a lookup in the
/// hash table, and for each match, an output row is produced. Note that the row
/// from the probe input is already located in the table record buffers, and the
/// matching row stored in the hash table is restored back to the record buffers
/// where it originally came from. For details around how rows are stored and
/// restored, see comments on hash_join_buffer::StoreFromTableBuffers.
///
/// The size of the in-memory hash table is controlled by the system variable
/// join_buffer_size. If we run out of memory during step 2, we degrade into a
/// hybrid hash join. The data already in memory is processed using regular hash
/// join, and the remainder is processed using on-disk hash join. It works like
/// this:
///
/// 1) The rest of the rows in the build input that did not fit into the hash
/// table are partitioned out into a given amount of files, represented by
/// HashJoinChunks. We create an equal number of chunk files for both the probe
/// and build input. We determine which file to put a row in by calculating a
/// hash from the join attribute like in step 2 above, but using a different
/// hash function.
///
/// 2) Then, we read the rows from the probe input, one by one. We look for a
/// match in the hash table as described above, but the row is also written out
/// to the chunk file on disk, since it might match a row from the build input
/// that we've written to disk.
///
/// 3) When the entire probe input is read, we run the "classic" hash join on
/// each of the corresponding chunk file probe/build pairs. Since the rows are
/// partitioned using the same hash function for probe and build inputs, we know
/// that matching rows must be located in the same pair of chunk files.
///
/// The algorithm for semijoin is quite similar to inner joins:
///
/// 1) Designate the inner table (i.e. the IN-side of a semijoin) as the build
/// input. As semijoins only needs the first matching row from the inner table,
/// we do not store duplicate keys in the hash table.
///
/// 2) Output all rows from the probe input where there is at least one matching
/// row in the hash table. In case we have degraded into on-disk hash join, we
/// write the probe row out to chunk file only if we did not find a matching row
/// in the hash table.
///
/// The optimizer may set up semijoins with conditions that are not pure join
/// conditions, but that must be attached to the hash join iterator anyways.
/// Consider the following query and (slightly modified) execution plan:
///
///   SELECT c FROM t WHERE 1 IN (SELECT t.c = col1 FROM t1);
///
///   -> Hash semijoin (no condition), extra conditions: (1 = (t.c = t1.col1))
///       -> Table scan on t
///       -> Hash
///           -> Table scan on t1
///
/// In this query, the optimizer has set up the condition (1 = (t.c = t1.col1))
/// as the semijoin condition. We cannot use this as a join condition, since
/// hash join only supports equi-join conditions. However, we cannot attach this
/// as a filter after the join, as that would cause wrong results. We attach
/// these conditions as "extra" conditions to the hash join iterator, and causes
/// these notable behaviors:
///
/// a. If we have any extra conditions, we cannot reject duplicate keys in the
///    hash table: the first row matching the join condition could fail the
///    extra condition(s).
///
/// b. We can only output rows if all extra conditions pass. If any of the extra
///    conditions fail, we must go to the next matching row in the hash table.
///
/// c. In case of on-disk hash join, we must write the probe row to disk _after_
///    we have checked that there are no rows in the hash table that match any
///    of the extra conditions.
///
/// If we are able to execute the hash join in memory (classic hash join),
/// the output will be sorted the same as the left (probe) input. If we start
/// spilling to disk, we lose any reasonable ordering properties.
///
/// Note that we still might end up in a case where a single chunk file from
/// disk won't fit into memory. This is resolved by reading as much as possible
/// into the hash table, and then reading the entire probe chunk file for each
/// time the hash table is reloaded. This might happen if we have a very skewed
/// data set, for instance.
///
/// When we start spilling to disk, we allocate a maximum of "kMaxChunks"
/// chunk files on disk for each of the two inputs. The reason for having an
/// upper limit is to avoid running out of file descriptors.
///
/// There is also a flag we can set to avoid hash join spilling to disk
/// regardless of the input size. If the flag is set, the join algorithm works
/// like this:
///
/// 1) Read as many rows as possible from the build input into an in-memory hash
/// table.
/// 2) When the hash table is full (we have reached the limit set by the system
/// variable join_buffer_size), start reading from the beginning of the probe
/// input, probing for matches in the hash table. Output a row for each match
/// found.
/// 3) When the probe input is empty, see if there are any remaining rows in the
/// build input. If so, clear the in-memory hash table and go to step 1,
/// continuing from the build input where we stopped the last time. If not, the
/// join is done.
///
/// Doing everything in memory can be beneficial in a few cases. Currently, it
/// is used when we have a LIMIT without sorting or grouping in the query. The
/// gain is that we start producing output rows a lot earlier than if we were to
/// spill both inputs out to disk. It could also be beneficial if the build
/// input _almost_ fits in memory; it would likely be better to read the probe
/// input twice instead of writing both inputs out to disk. However, we do not
/// currently do any such cost based optimization.
///
/// There is a concept called "probe row saving" in the iterator. This is a
/// technique that is enabled in two different scenarios: when a hash join build
/// chunk does not fit entirely in memory and when hash join is not allowed to
/// spill to disk. Common for these two scenarios is that a probe row will be
/// read multiple times. For certain join types (semijoin), we must take care so
/// that the same probe row is not sent to the client multiple times. Probe row
/// saving takes care of this by doing the following:
///
/// - If we realize that we are going to read the same probe row multiple times,
///   we enable probe row saving.
/// - When a probe row is read, we write the row out to a probe row saving write
///   file, given that it matches certain conditions (for semijoin we only save
///   unmatched probe rows).
/// - After the probe input is consumed, we will swap the probe row saving
///   _write_ file and the probe row saving _read_ file, making the write file
///   available for writing again.
/// - When we are to read the probe input again, we read the probe rows from the
///   probe row saving read file. This ensures that we i.e. do not output the
///   same probe row twice for semijoin. Note that if the rows we read from the
///   probe row saving read file will be read again (e.g., we have a big hash
///   join build chunk that is many times bigger than the available hash table
///   memory, causing us to process the chunk file in chunks), we will again
///   write the rows to a new probe row saving write file. This reading from the
///   read file and writing to a new write file continues until we know that we
///   are seeing the probe rows for the last time.
///
/// We use the same methods as on-disk hash join (HashJoinChunk) for reading and
/// writing rows to files. Note that probe row saving is never enabled for inner
/// joins, since we do want to output the same probe row multiple times if it
/// matches muliple rows from the build input. There are some differences
/// regarding when probe row saving is enabled, depending on the hash join type
/// (see enum HashJoinType):
///
/// - IN_MEMORY: Probe row saving is never activated, since the probe input is
///   read only once.
/// - SPILL_TO_DISK: If a build chunk file does not fit in memory (may happen
///   with skewed data set), we will have to read the corresponding probe chunk
///   multiple times. In this case, probe row saving is enabled as soon as we
///   see that the build chunk does not fit in memory, and remains active until
///   the entire build chunk is consumed. After the probe chunk is read once,
///   we swap the probe row saving write file and probe row saving read file so
///   that probe rows will be read from the probe row saving read file. Probe
///   row saving is deactivated once we move to the next pair of chunk files.
/// - IN_MEMORY_WITH_HASH_TABLE_REFILL: Probe row saving is activated when we
///   see that the build input is too large to fit in memory. Once the probe
///   iterator has been consumed once, we swap the probe row saving write file
///   and probe row saving read file so that probe rows will be read from the
///   probe row saving read file. As long as the build input is not fully
///   consumed, we write probe rows from the read file out to a new write file,
///   swapping these files for every hash table refill. Probe row saving is
///   never deactivated in this hash join type.
///
/// Note that we always write the entire row when writing to probe row saving
/// file. It would be possible to only write the match flag, but this is tricky
/// as long as we have the hash join type IN_MEMORY_WITH_HASH_TABLE_REFILL. If
/// we were to write only match flags in this hash join type, we would have to
/// read the probe iterator multiple times. But there is no guarantee that rows
/// will come in the same order when reading an iterator multiple times (e.g.
/// NDB does not guarantee this), so it would require us to store match flags in
/// a lookup structure using a row ID as the key. Due to this, we will
/// reconsider this if the hash join type IN_MEMORY_WITH_HASH_TABLE_REFILL goes
/// away.
class HashJoinIterator final : public RowIterator {
 public:
  /// Construct a HashJoinIterator.
  ///
  /// @param thd
  ///   the thread handle
  /// @param build_input
  ///   the iterator for the build input
  /// @param build_input_tables
  ///   a bitmap of all the tables in the build input. The tables are needed for
  ///   two things:
  ///   1) Accessing the columns when creating the join key during creation of
  ///   the hash table,
  ///   2) and accessing the column data when creating the row to be stored in
  ///   the hash table and/or the chunk file on disk.
  /// @param probe_input
  ///   the iterator for the probe input
  /// @param probe_input_tables
  ///   the probe input tables. Needed for the same reasons as
  ///   build_input_tables.
  /// @param max_memory_available
  ///   the amount of memory available, in bytes, for this hash join iterator.
  ///   This can be user-controlled by setting the system variable
  ///   join_buffer_size.
  /// @param join_conditions
  ///   a list of all the join conditions between the two inputs
  /// @param allow_spill_to_disk
  ///   whether the hash join can spill to disk. This is set to false in some
  ///   cases where we have a LIMIT in the query
  /// @param join_type
  ///   The join type.
  /// @param join
  ///   The join we are a part of.
  /// @param extra_conditions
  ///   A list of extra conditions that the iterator will evaluate after a
  ///   lookup in the hash table is done, but before the row is returned. The
  ///   conditions are AND-ed together into a single Item.
  HashJoinIterator(THD *thd, unique_ptr_destroy_only<RowIterator> build_input,
                   qep_tab_map build_input_tables,
                   unique_ptr_destroy_only<RowIterator> probe_input,
                   qep_tab_map probe_input_tables, size_t max_memory_available,
                   const std::vector<HashJoinCondition> &join_conditions,
                   bool allow_spill_to_disk, JoinType join_type,
                   const JOIN *join,
                   const std::vector<Item *> &extra_conditions);

  bool Init() override;

  int Read() override;

  void SetNullRowFlag(bool is_null_row) override {
    m_build_input->SetNullRowFlag(is_null_row);
    m_probe_input->SetNullRowFlag(is_null_row);
  }

  void EndPSIBatchModeIfStarted() override {
    m_build_input->EndPSIBatchModeIfStarted();
    m_probe_input->EndPSIBatchModeIfStarted();
  }

  void UnlockRow() override {
    // Since both inputs may have been materialized to disk, we cannot unlock
    // them.
  }

  std::vector<std::string> DebugString() const override;

  std::vector<Child> children() const override {
    return std::vector<Child>{{m_probe_input.get(), ""},
                              {m_build_input.get(), "Hash"}};
  }

 private:
  /// Read all rows from the build input and store the rows into the in-memory
  /// hash table. If the hash table goes full, the rest of the rows are written
  /// out to chunk files on disk. See the class comment for more details.
  ///
  /// @retval true in case of error
  bool BuildHashTable();

  /// Read all rows from the next chunk file into the in-memory hash table.
  /// See the class comment for details.
  ///
  /// @retval true in case of error
  bool ReadNextHashJoinChunk();

  /// Read a single row from the probe iterator input into the tables' record
  /// buffers. If we have started spilling to disk, the row is written out to a
  /// chunk file on disk as well.
  ///
  /// The end condition is that either:
  /// a) a row is ready in the tables' record buffers, and the state will be set
  ///    to READING_FIRST_ROW_FROM_HASH_TABLE.
  /// b) There are no more rows to process from the probe input, so the iterator
  ///    state will be LOADING_NEXT_CHUNK_PAIR.
  ///
  /// @retval true in case of error
  bool ReadRowFromProbeIterator();

  /// Read a single row from the current probe chunk file into the tables'
  /// record buffers. The end conditions are the same as for
  /// ReadRowFromProbeIterator().
  ///
  /// @retval true in case of error
  bool ReadRowFromProbeChunkFile();

  /// Read a single row from the probe row saving file into the tables' record
  /// buffers.
  ///
  /// @retval true in case of error
  bool ReadRowFromProbeRowSavingFile();

  // Do a lookup in the hash table for matching rows from the build input.
  // The lookup is done by computing the join key from the probe input, and
  // using that join key for doing a lookup in the hash table. If the join key
  // contains one or more SQL NULLs, the row cannot match anything and will be
  // skipped, and the iterator state will be READING_ROW_FROM_PROBE_INPUT. If
  // not, the iterator state will be READING_FIRST_ROW_FROM_HASH_TABLE.
  //
  // After this function is called, ReadJoinedRow() will return false until
  // there are no more matching rows for the computed join key.
  void LookupProbeRowInHashTable();

  /// Take the next matching row from the hash table, and put the row into the
  /// build tables' record buffers. The function expects that
  /// LookupProbeRowInHashTable() has been called up-front. The user must
  /// call ReadJoinedRow() as long as it returns false, as there may be
  /// multiple matching rows from the hash table. It is up to the caller to set
  /// a new state in case of EOF.
  ///
  /// @retval 0 if a match was found and the row is put in the build tables'
  ///         record buffers
  /// @retval -1 if there are no more matching rows in the hash table
  int ReadJoinedRow();

  // Have we degraded into on-disk hash join?
  bool on_disk_hash_join() const { return !m_chunk_files_on_disk.empty(); }

  /// Write the last row read from the probe input out to chunk files on disk,
  /// if applicable.
  ///
  /// For inner joins, we must write all probe rows to chunk files, since we
  /// need to match the row against rows from the build input that are written
  /// out to chunk files. For semijoin, we can only write probe rows that do not
  /// match any of the rows in the hash table. Writing a probe row with a
  /// matching row in the hash table could cause the row to be returned multiple
  /// times.
  ///
  /// @retval true in case of errors.
  bool WriteProbeRowToDiskIfApplicable();

  /// @retval true if the last joined row passes all of the extra conditions.
  bool JoinedRowPassesExtraConditions() const;

  /// If true, reject duplicate keys in the hash table.
  ///
  /// Semijoins/antijoins are only interested in the first matching row from the
  /// hash table, so we can avoid storing duplicate keys in order to save some
  /// memory. However, this cannot be applied if we have any "extra" conditions:
  /// the first matching row in the hash table may fail the extra condition(s).
  ///
  /// @retval true if we can reject duplicate keys in the hash table.
  bool RejectDuplicateKeys() const {
    return m_extra_condition == nullptr &&
           (m_join_type == JoinType::SEMI || m_join_type == JoinType::ANTI);
  }

  /// Clear the row buffer and reset all iterators pointing to it. This may be
  /// called multiple times to re-init the row buffer.
  ///
  /// @retval true in case of error. my_error has been called
  bool InitRowBuffer();

  /// Prepare to read the probe iterator from the beginning, and enable batch
  /// mode if applicable. The iterator state will remain unchanged.
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitProbeIterator();

  /// Mark that probe row saving is enabled, and prepare the probe row saving
  /// file for writing.
  /// @see m_write_to_probe_row_saving
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitWritingToProbeRowSavingFile();

  /// Mark that we should read from the probe row saving file. The probe row
  /// saving file is rewinded to the beginning.
  /// @see m_read_from_probe_row_saving
  ///
  /// @retval true in case of error. my_error has been called.
  bool InitReadingFromProbeRowSavingFile();

  /// Set the iterator state to the correct READING_ROW_FROM_PROBE_*-state.
  /// Which state we end up in depends on which hash join type we are executing
  /// (in-memory, on-disk or in-memory with hash table refill).
  void SetReadingProbeRowState();

  /// Read a joined row from the hash table, and see if it passes any extra
  /// conditions. The last probe row read will also be written do disk if needed
  /// (see WriteProbeRowToDiskIfApplicable).
  ///
  /// @retval -1 There are no more matching rows in the hash table.
  /// @retval 0 A joined row is ready.
  /// @retval 1 An error occured.
  int ReadNextJoinedRowFromHashTable();

  enum class State {
    // We are reading a row from the probe input, where the row comes from
    // the iterator.
    READING_ROW_FROM_PROBE_ITERATOR,
    // We are reading a row from the probe input, where the row comes from a
    // chunk file.
    READING_ROW_FROM_PROBE_CHUNK_FILE,
    // We are reading a row from the probe input, where the row comes from a
    // probe row saving file.
    READING_ROW_FROM_PROBE_ROW_SAVING_FILE,
    // The iterator is moving to the next pair of chunk files, where the chunk
    // file from the build input will be loaded into the hash table.
    LOADING_NEXT_CHUNK_PAIR,
    // We are reading the first row returned from the hash table lookup that
    // also passes extra conditions.
    READING_FIRST_ROW_FROM_HASH_TABLE,
    // We are reading the remaining rows returned from the hash table lookup.
    READING_FROM_HASH_TABLE,
    // No more rows, both inputs are empty.
    END_OF_ROWS
  };

  State m_state;

  const unique_ptr_destroy_only<RowIterator> m_build_input;
  const unique_ptr_destroy_only<RowIterator> m_probe_input;

  // An iterator for reading rows from the hash table.
  // hash_join_buffer::HashJoinRowBuffer::Iterator m_hash_map_iterator;
  hash_join_buffer::HashJoinRowBuffer::hash_map_iterator m_hash_map_iterator;
  hash_join_buffer::HashJoinRowBuffer::hash_map_iterator m_hash_map_end;

  // These structures holds the tables and columns that are needed for the hash
  // join. Rows/columns that are not needed are filtered out in the constructor.
  // We need to know which tables that belong to each iterator, so that we can
  // compute the join key when needed.
  hash_join_buffer::TableCollection m_probe_input_tables;
  hash_join_buffer::TableCollection m_build_input_tables;

  // An in-memory hash table that holds rows from the build input (directly from
  // the build input iterator, or from a chunk file). See the class comment for
  // details on how and when this is used.
  hash_join_buffer::HashJoinRowBuffer m_row_buffer;

  // A list of the join conditions (all of them are equi-join conditions).
  Prealloced_array<HashJoinCondition, 4> m_join_conditions;

  // Array to hold the list of chunk files on disk in case we degrade into
  // on-disk hash join.
  Mem_root_array<ChunkPair> m_chunk_files_on_disk;

  // Which HashJoinChunk, if any, we are currently reading from, in both
  // LOADING_NEXT_CHUNK_PAIR and READING_ROW_FROM_PROBE_CHUNK_FILE.
  // It is incremented during the state LOADING_NEXT_CHUNK_PAIR.
  int m_current_chunk{-1};

  // The seeds that are used by xxHash64 when calculating the hash from a join
  // key. We need one seed for the hashing done in the in-memory hash table,
  // and one seed when calculating the hash that is used for determining which
  // chunk file a row should be placed in (in case of on-disk hash join). If we
  // were to use the same seed for both operations, we would get a really bad
  // hash table when loading a chunk file to the hash table. The numbers are
  // chosen randomly and have no special meaning.
  static constexpr uint32_t kHashTableSeed{156211};
  static constexpr uint32_t kChunkPartitioningHashSeed{899339};

  // Which row we currently are reading from each of the hash join chunk file.
  ha_rows m_build_chunk_current_row = 0;
  ha_rows m_probe_chunk_current_row = 0;

  // The maximum number of HashJoinChunks that is allocated for each of the
  // inputs in case we spill to disk. We might very well end up with an amount
  // less than this number, but we keep an upper limit so we don't risk running
  // out of file descriptors. We always use a power of two number of files,
  // which allows us to do some optimizations when calculating which chunk a row
  // should be placed in.
  static constexpr size_t kMaxChunks = 128;

  // A buffer that is used during two phases:
  // 1) when constructing a join key from join conditions.
  // 2) when moving a row between tables' record buffers and the hash table.
  //
  // There are two functions that needs this buffer; ConstructJoinKey() and
  // StoreFromTableBuffers(). After calling one of these functions, the user
  // must take responsiblity of the data if it is needed for a longer lifetime.
  //
  // If there are no BLOB/TEXT column in the join, we calculate an upper bound
  // of the row size that is used to preallocate this buffer. In the case of
  // BLOB/TEXT columns, we cannot calculate a reasonable upper bound, and the
  // row size is calculated per row. The allocated memory is kept for the
  // duration of the iterator, so that we (most likely) avoid reallocations.
  String m_temporary_row_and_join_key_buffer;

  // Whether we should turn on batch mode for the probe input. Batch mode is
  // enabled if the probe input consists of exactly one table, and
  // QEP_TAB::pfs_batch_update() returns true for this table.
  bool m_probe_input_batch_mode{false};

  // Whether we are allowed to spill to disk.
  bool m_allow_spill_to_disk{true};

  // Whether the build iterator has more rows. This is used to stop the hash
  // join iterator asking for more rows when we know for sure that the entire
  // build input is consumed. The variable is only used if m_allow_spill_to_disk
  // is false, as we have to see if there are more rows in the build input after
  // the probe input is consumed.
  bool m_build_iterator_has_more_rows{true};

  // What kind of join the iterator should execute.
  const JoinType m_join_type;

  // If not nullptr, an extra condition that the iterator will evaluate after a
  // lookup in the hash table is done, but before the row is returned. This is
  // needed in case we have a semijoin condition that is not an equi-join
  // condition (i.e. 't1.col1 < t2.col1').
  Item *m_extra_condition{nullptr};

  // Whether we should write rows from the probe input to the probe row saving
  // write file. See the class comment on HashJoinIterator for details around
  // probe row saving.
  bool m_write_to_probe_row_saving{false};

  // Whether we should read rows from the probe row saving read file. See the
  // class comment on HashJoinIterator for details around probe row saving.
  bool m_read_from_probe_row_saving{false};

  // The probe row saving files where unmatched probe rows are written to and
  // read from.
  HashJoinChunk m_probe_row_saving_write_file;
  HashJoinChunk m_probe_row_saving_read_file;

  // Which row we currently are reading from in the probe row saving read file.
  // Used to know whether we have reached the end of the file. How many files
  // the probe row saving read file contains is contained in the HashJoinChunk
  // (see m_probe_row_saving_read_file).
  ha_rows m_probe_row_saving_read_file_current_row{0};

  // The "type" of hash join we are executing. We currently have three different
  // types of hash join:
  // - In memory: We do everything in memory without any refills of the hash
  //   table. Each input is read only once, and nothing is written to disk.
  // - Spill to disk: If the build input does not fit in memory, we write both
  //   inputs out to a set of chunk files. Both inputs are partitioned using a
  //   hash function over the join attribute, ensuring that matching rows can be
  //   found in the same set of chunk files. Each pair of chunk file is then
  //   processed as an in-memory hash join.
  // - In memory with hash table refill: This is enabled if we are not allowed
  //   to spill to disk, and the build input does not fit in memory. We read as
  //   much as possible from the build input into the hash table. We then read
  //   the entire probe input, probing for matching rows in the hash table.
  //   When the probe input returns EOF, the hash table is refilled with the
  //   rows that did not fit the first time. The entire probe input is read
  //   again, and this is repeated until the entire build input is consumed.
  enum class HashJoinType {
    IN_MEMORY,
    SPILL_TO_DISK,
    IN_MEMORY_WITH_HASH_TABLE_REFILL
  };
  HashJoinType m_hash_join_type{HashJoinType::IN_MEMORY};

  // The match flag for the last probe row read from chunk file.
  //
  // This is needed if a outer join spills to disk; a probe row can match a row
  // from the build input we haven't seen yet (it's been written out to disk
  // because the hash table was full). So when reading a probe row from a chunk
  // file, this variable holds the match flag. This flag must be a class member,
  // since one probe row may match multiple rows from the hash table; the
  // execution will go out of HashJoinIterator::Read() between each matching
  // row, causing any local match flag to lose the match flag info from the last
  // probe row read.
  bool m_probe_row_match_flag{false};
};

/// For each of the given tables, request that the row ID is filled in
/// (the equivalent of calling file->position()) if needed.
///
/// @param tables The tables to request row IDs for.
void RequestRowId(const Prealloced_array<hash_join_buffer::Table, 4> &tables);

#endif  // SQL_HASH_JOIN_ITERATOR_H_
