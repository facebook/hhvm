/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>
#include <memory>

#include "sql/record_buffer.h"

namespace record_buffer_unittest {

/**
  Struct that holds a (records, record_size) pair that tells the size
  of the Record_buffer to use in a test case.
*/
struct Buffer_param {
  const ha_rows m_records;
  const size_t m_record_size;
};

/// Run the test case with the following test parameters.
static const Buffer_param PARAMS[] = {{0, 0},    {1, 1},    {2, 2},
                                      {0, 100},  {100, 0},  {10, 10},
                                      {10, 100}, {100, 10}, {1024, 1024}};

class RecordBufferTestP : public ::testing::TestWithParam<Buffer_param> {};

TEST_P(RecordBufferTestP, BasicTest) {
  const auto param = GetParam();
  const auto bufsize =
      Record_buffer::buffer_size(param.m_records, param.m_record_size);
  std::unique_ptr<uchar[]> ptr(new uchar[bufsize]);

  Record_buffer buf(param.m_records, param.m_record_size, ptr.get());
  EXPECT_EQ(param.m_records, buf.max_records());
  EXPECT_EQ(param.m_record_size, buf.record_size());
  EXPECT_EQ(0U, buf.records());
  EXPECT_FALSE(buf.is_out_of_range());

  for (size_t i = 0; i < param.m_records; ++i) {
    /*
      Add a record and verify that the number of records has grown,
      whereas the maximum size and the record size stay the same.
    */
    const auto rec = buf.add_record();
    EXPECT_EQ(i + 1, buf.records());
    EXPECT_NE(nullptr, rec);
    EXPECT_EQ(rec, buf.record(i));
    EXPECT_EQ(param.m_records, buf.max_records());
    EXPECT_EQ(param.m_record_size, buf.record_size());

    /*
      Remove the last added record, and see that the record count
      decreases. And add it back again.
    */
    buf.remove_last();
    EXPECT_EQ(i, buf.records());
    EXPECT_EQ(rec, buf.add_record());
    EXPECT_EQ(i + 1, buf.records());
  }

  // Test setting the out-of-range flag.
  EXPECT_FALSE(buf.is_out_of_range());
  buf.set_out_of_range(true);
  EXPECT_TRUE(buf.is_out_of_range());

  /*
    reset() should clear the out-of-range flag and the record count,
    but the maximum size and the record size should stay the same.
  */
  buf.reset();
  EXPECT_FALSE(buf.is_out_of_range());
  EXPECT_EQ(0U, buf.records());
  EXPECT_EQ(param.m_records, buf.max_records());
  EXPECT_EQ(param.m_record_size, buf.record_size());
}

INSTANTIATE_TEST_CASE_P(Test, RecordBufferTestP, ::testing::ValuesIn(PARAMS));

TEST(RecordBufferTest, Clear) {
  constexpr ha_rows rows = 10;
  constexpr size_t row_size = 10;
  uchar ch[Record_buffer::buffer_size(rows, row_size)];
  Record_buffer buf(rows, row_size, ch);
  buf.add_record();
  buf.add_record();
  buf.set_out_of_range(true);
  EXPECT_TRUE(buf.is_out_of_range());
  EXPECT_EQ(2U, buf.records());
  /*
    Record_buffer::clear() should remove all records, but it should
    keep the out-of-range flag.
  */
  buf.clear();
  EXPECT_TRUE(buf.is_out_of_range());
  EXPECT_EQ(0U, buf.records());
}

TEST(RecordBufferTest, Reset) {
  constexpr ha_rows rows = 10;
  constexpr size_t row_size = 10;
  uchar ch[Record_buffer::buffer_size(rows, row_size)];
  Record_buffer buf(rows, row_size, ch);
  buf.add_record();
  buf.add_record();
  buf.set_out_of_range(true);
  EXPECT_TRUE(buf.is_out_of_range());
  EXPECT_EQ(2U, buf.records());
  /*
    Record_buffer::reset() should remove all records and clear the
    out-of-range flag.
  */
  buf.reset();
  EXPECT_FALSE(buf.is_out_of_range());
  EXPECT_EQ(0U, buf.records());
}

}  // namespace record_buffer_unittest
