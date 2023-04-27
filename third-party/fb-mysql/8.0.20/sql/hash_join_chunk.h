#ifndef SQL_HASH_JOIN_CHUNK_H_
#define SQL_HASH_JOIN_CHUNK_H_

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

#include <stddef.h>

#include "my_alloc.h"
#include "my_base.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/hash_join_buffer.h"

class String;

// A HashJoinChunk is a file located on disk that can be used to store rows.
// It is used in on-disk hash join when a table is to be partitioned out to
// several smaller files, a.k.a. HashJoinChunks.
//
// When writing a column to a HashJoinChunk, we use StoreFromTableBuffers for
// converting the necessary columns into a format suitable for storage on disk.
// Conveniently, StoreFromTableBuffers creates a contiguous range of bytes and a
// corresponding length that easily and efficiently can be written out to the
// file. When reading rows back from a file, LoadIntoTableBuffers() is used to
// put the row back into the table record buffers.
//
// The basic usage goes like this:
//
//   HashJoinChunk chunk;
//   // Initialize a chunk to hold data from the given tables without any match
//   // flags.
//   chunk.Init(tables, /*uses_match_flags=*/false);
//   String buffer; // A buffer that is used when copying data between tables
//                  // and the chunk file, and vica versa.
//   while (iterator->Read() == 0) {
//     // Write the row that lies in the record buffers of "tables" to this
//     // chunk, using the provided buffer. If the chunk file was initialized to
//     // use match flags, we would prefix the row with a match flag saying that
//     // this row did not have any matching row.
//     chunk.WriteRowToChunk(&buffer, /*matched=*/false);
//   };
//
//   chunk.Rewind(); // Prepare to read the first row in this chunk.
//
//   bool match_flag,
//   // Put the row from the chunk to the record buffers of "tables", using the
//   // provided buffer. If the chunk file was initialized to use match flags,
//   // the match flag for the row read would be stored in 'match_flag'.
//   chunk.LoadRowFromChunk(&buffer, &match_flag);
class HashJoinChunk {
 public:
  HashJoinChunk() = default;  // Constructible.

  HashJoinChunk(HashJoinChunk &&other);  // Movable.

  HashJoinChunk(const HashJoinChunk &obj) = delete;

  HashJoinChunk &operator=(HashJoinChunk &&other);

  ~HashJoinChunk();

  /// Initialize this HashJoinChunk.
  ///
  /// @param tables The tables to store row data from. Which column we store in
  ///   the chunk file is determined by each tables read set.
  /// @param uses_match_flags Whether each row should be prefixed with a match
  ///   flag, saying whether the row had a matching row.
  ///
  /// @returns true if the initialization failed.
  bool Init(const hash_join_buffer::TableCollection &tables,
            bool uses_match_flags);

  /// @returns the number of rows in this HashJoinChunk
  ha_rows num_rows() const { return m_num_rows; }

  /// Write a row to the HashJoinChunk.
  ///
  /// Read the row that lies in the record buffer (record[0]) of the given
  /// tables and write it out to the underlying file. If the QEP_TAB signals
  /// that the row ID should be kept, it is also written out. Note that
  /// TABLE::read_set is used to signal which columns that should be written to
  /// the chunk.
  ///
  /// @param buffer a buffer that is used when copying data from the tables to
  ///   the chunk file. Note that any existing data in "buffer" is overwritten.
  /// @param matched whether this row has seen a matching row from the other
  ///   input. The flag is only written if 'm_uses_match_flags' is set, and if
  ///   the row comes from the probe input.
  ///
  /// @retval true on error.
  bool WriteRowToChunk(String *buffer, bool matched);

  /// Read a row from the HashJoinChunk and put it in the record buffer.
  ///
  /// The function will read a row from file on disk and put it in the record
  /// buffers (table->record[0]) in the provided tables. The file on disk should
  /// already be pointing to the start of a row.
  ///
  /// @param buffer a buffer that is used when copying data from the chunk file
  ///   to the tables. Note that any existing data in "buffer" is overwritten.
  ///
  /// @param[out] matched whether this row has seen a matching row from the
  ///   other input. The flag is only restored if 'm_uses_match_flags' is set,
  ///   and if the row comes from the probe input.
  /// @retval true on error.
  bool LoadRowFromChunk(String *buffer, bool *matched);

  /// Flush the file buffer, and prepare the file for reading.
  ///
  /// @retval true on error
  bool Rewind();

 private:
  // A collection of which tables the chunk file holds data from. Used to
  // determine where to read data from, and where to put the data back.
  hash_join_buffer::TableCollection m_tables;

  // The number of rows in this chunk file.
  ha_rows m_num_rows{0};

  // The underlying file that is used when reading data to and from disk.
  IO_CACHE m_file;

  // Whether every row is prefixed with a match flag.
  bool m_uses_match_flags{false};
};

#endif  // SQL_HASH_JOIN_CHUNK_H_
