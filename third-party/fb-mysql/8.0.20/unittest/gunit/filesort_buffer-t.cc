/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>
#include <utility>

#include "my_inttypes.h"
#include "my_pointer_arithmetic.h"
#include "sql/filesort_utils.h"
#include "sql/table.h"

namespace filesort_buffer_unittest {

class FileSortBufferTest : public ::testing::Test {
 protected:
  virtual void TearDown() {
    fs_info.free_sort_buffer();
    EXPECT_TRUE(nullptr == fs_info.get_sort_keys());
  }

  Filesort_buffer fs_info;
};

TEST_F(FileSortBufferTest, Basic) {
  const char letters[10] = "abcdefghi";  // Zero-terminated.

  fs_info.set_max_size(32768, 10);
  for (uint ix = 0; ix < 10; ++ix) {
    Bounds_checked_array<uchar> buf = fs_info.get_next_record_pointer(10);
    ASSERT_GE(buf.size(), 10);
    memcpy(buf.array(), letters, 10);
    fs_info.commit_used_memory(10);
  }
  uchar **data = fs_info.get_sort_keys();
  for (uint ix = 0; ix < 10; ++ix) {
    const char *str = reinterpret_cast<const char *>(data[ix]);
    EXPECT_STREQ(letters, str);
  }

  EXPECT_GE(fs_info.peak_memory_used(), 100);
}

TEST_F(FileSortBufferTest, SizeNotGivenUpFront) {
  const char letters[10] = "abcdefghi";  // Zero-terminated.
  bool ever_given_too_little = false;

  fs_info.set_max_size(10485760, 10);
  for (uint ix = 0; ix < 10000; ++ix) {
    Bounds_checked_array<uchar> buf;
    size_t min_size = 1;
    for (;;)  // Termination condition within loop.
    {
      buf = fs_info.get_next_record_pointer(min_size);
      ASSERT_GE(buf.size(), min_size);
      if (buf.size() >= 10) break;
      ever_given_too_little = true;
      min_size = buf.size() + 1;
    }
    memcpy(buf.array(), letters, 10);
    fs_info.commit_used_memory(10);
  }

  /*
    Since MIN_SORT_MEMORY (32768) is not divisible by 10, we should get a
    record that's too small (8 bytes long, actually) at least once.
  */
  EXPECT_TRUE(ever_given_too_little);

  uchar **data = fs_info.get_sort_keys();
  for (uint ix = 0; ix < 10000; ++ix) {
    const char *str = reinterpret_cast<const char *>(data[ix]);
    EXPECT_STREQ(letters, str);
  }

  EXPECT_GE(fs_info.peak_memory_used(), 100000);
  EXPECT_LT(fs_info.peak_memory_used(), 1000000);
}

TEST_F(FileSortBufferTest, OneBigRecordFits) {
  /*
    Set the record size to maximum possible to ensure we don't
    estimate any space for record pointers. This is depending on an
    implementation detail; if the record pointer allocation gets
    smarter in the future, the test will have to be adjusted.
  */
  fs_info.set_max_size(10485760, 0xFFFFFFFFu);
  Bounds_checked_array<uchar> buf;
  size_t min_size = 1;
  for (;;)  // Termination condition within loop.
  {
    buf = fs_info.get_next_record_pointer(min_size);

    // Should increase by _more_ than just one byte each reallocation.
    ASSERT_GE(buf.size(), min_size + 1);

    if (buf.size() >= 10485760) break;
    min_size = buf.size() + 1;
  }

  EXPECT_EQ(10485760, buf.size());
}

TEST_F(FileSortBufferTest, RecordPointersGetReclaimed) {
  // Allocate tiny records until there is no more room.
  fs_info.set_max_size(300000, 1);
  Bounds_checked_array<uchar> buf;
  size_t num_records = 0;
  for (;;) {  // Termination condition within loop.
    buf = fs_info.get_next_record_pointer(1);
    if (buf.array() == nullptr) break;
    fs_info.commit_used_memory(1);
    ++num_records;
  }

  // Verify that we took record pointers into account.
  EXPECT_LT(num_records, 100000);

  fs_info.reset();

  /*
    Now we will have tons of record pointers that need to be reclaimed
    if we want to be able to to store 250 kB of large rows.
  */
  fs_info.set_max_size(300000, 10000);
  for (uint ix = 0; ix < 25; ++ix) {
    buf = fs_info.get_next_record_pointer(10000);
    EXPECT_NE(nullptr, buf.array());
    fs_info.commit_used_memory(10000);
  }
}

TEST_F(FileSortBufferTest, PreallocateRecords) {
  fs_info.set_max_size(32768, sizeof(char));
  fs_info.preallocate_records(10);
  uchar **ptr = fs_info.get_sort_keys();
  EXPECT_NE(nullptr, ptr);
  for (uint ix = 0; ix < 10 - 1; ++ix) {
    uchar **nxt = ptr + 1;
    EXPECT_EQ(1, *nxt - *ptr) << "index:" << ix;
    ++ptr;
  }
}

}  // namespace filesort_buffer_unittest
