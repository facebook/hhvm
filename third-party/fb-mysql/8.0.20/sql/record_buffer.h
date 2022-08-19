#ifndef RECORD_BUFFER_INCLUDED
#define RECORD_BUFFER_INCLUDED

/*
   Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "my_base.h"  // ha_rows
#include "my_dbug.h"

/**
  This class represents a buffer that can be used for multi-row reads. It is
  allocated by the executor and given to the storage engine through the
  handler, using handler::ha_set_record_buffer(), so that the storage engine
  can fill the buffer with multiple rows in a single read call.

  For now, the record buffer is only used internally by the storage engine as
  a prefetch cache. The storage engine fills the record buffer when the
  executor requests the first record, but it returns a single record only to
  the executor. If the executor wants to access the records in the buffer, it
  has to call a handler function such as handler::ha_index_next() or
  handler::ha_rnd_next(). Then the storage engine will copy the next record
  from the record buffer to the memory area specified in the arguments of the
  handler function, typically TABLE::record[0].
*/
class Record_buffer {
  /// The maximum number of records that can be stored in the buffer.
  ha_rows m_max_records;
  /// The number of bytes available for each record.
  size_t m_record_size;
  /// The number of records currently stored in the buffer.
  ha_rows m_count = 0;
  /// The @c uchar buffer that holds the records.
  uchar *m_buffer;
  /// Tells if end-of-range was found while filling the buffer.
  bool m_out_of_range = false;

 public:
  /**
    Create a new record buffer with the specified size.

    @param records      the number of records that can be stored in the buffer
    @param record_size  the size of each record
    @param buffer       the @c uchar buffer that will hold the records (its
                        size should be at least
                        `Record_buffer::buffer_size(records, record_size)`)
  */
  Record_buffer(ha_rows records, size_t record_size, uchar *buffer)
      : m_max_records(records), m_record_size(record_size), m_buffer(buffer) {}

  /**
    This function calculates how big the @c uchar buffer provided to
    Record_buffer's constructor must be, given a number of records and
    the record size.

    @param records      the maximum number of records in the buffer
    @param record_size  the size of each record
    @return the total number of bytes needed for all the records
  */
  static constexpr size_t buffer_size(ha_rows records, size_t record_size) {
    return static_cast<size_t>(records * record_size);
  }

  /**
    Get the number of records that can be stored in the buffer.
    @return the maximum number of records in the buffer
  */
  ha_rows max_records() const { return m_max_records; }

  /**
    Get the amount of space allocated for each record in the buffer.
    @return the record size
  */
  size_t record_size() const { return m_record_size; }

  /**
    Get the number of records currently stored in the buffer.
    @return the number of records stored in the buffer
  */
  ha_rows records() const { return m_count; }

  /**
    Get the buffer that holds the record on position @a pos.
    @param pos the record number (must be smaller than records())
    @return the @c uchar buffer that holds the specified record
  */
  uchar *record(ha_rows pos) const {
    DBUG_ASSERT(pos < max_records());
    return m_buffer + m_record_size * pos;
  }

  /**
    Add a new record at the end of the buffer.
    @return the @c uchar buffer of the added record
  */
  uchar *add_record() {
    DBUG_ASSERT(m_count < max_records());
    return record(m_count++);
  }

  /**
    Remove the record that was last added to the buffer.
  */
  void remove_last() {
    DBUG_ASSERT(m_count > 0);
    --m_count;
  }

  /**
    Clear the buffer. Remove all the records. The end-of-range flag is
    preserved.
  */
  void clear() { m_count = 0; }

  /**
    Reset the buffer. Remove all records and clear the end-of-range flag.
  */
  void reset() {
    m_count = 0;
    set_out_of_range(false);
  }

  /**
    Set whether the end of the range was reached while filling the buffer.
    @param val true if end of range was reached, false if still within range
  */
  void set_out_of_range(bool val) { m_out_of_range = val; }

  /**
    Check if the end of the range was reached while filling the buffer.
    @retval true if the end range was reached
    @retval false if the scan is still within the range
  */
  bool is_out_of_range() const { return m_out_of_range; }
};

#endif /* RECORD_BUFFER_INCLUDED */
