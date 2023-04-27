/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <errno.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_inttypes.h"
#include "my_sys.h"

// Ignore test on windows, as we are mocking away a unix function, see below.
#ifndef _WIN32

// For testing my_read.
extern ssize_t (*mock_read)(int fd, void *buf, size_t count);

namespace mysys_my_read_unittest {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetErrnoAndReturn;

class MockRead {
 public:
  virtual ~MockRead() {}
  MOCK_METHOD3(mockread, ssize_t(int, void *, size_t));
};

MockRead *mockfs = nullptr;

ssize_t mockfs_read(int fd, void *buf, size_t count) {
  return mockfs->mockread(fd, buf, count);
}

class MysysMyReadTest : public ::testing::Test {
  virtual void SetUp() {
    mock_read = mockfs_read;
    mockfs = new MockRead;
  }
  virtual void TearDown() {
    mock_read = nullptr;
    delete mockfs;
    mockfs = nullptr;
  }
};

// Test of normal case: read OK
TEST_F(MysysMyReadTest, MyReadOK) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockread(_, _, 4096)).Times(1).WillOnce(Return(4096));

  const size_t result = my_read(42, buf, 4096, 0);
  EXPECT_EQ(4096U, result);
}

// Test of normal case: read OK with MY_NABP
TEST_F(MysysMyReadTest, MyReadOKNABP) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockread(_, _, 4096)).Times(1).WillOnce(Return(4096));

  const size_t result = my_read(42, buf, 4096, MYF(MY_NABP));
  EXPECT_EQ(0U, result);
}

// Test of disk full: read not OK
TEST_F(MysysMyReadTest, MyReadFail) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockread(_, _, 4096))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_read(42, buf, 4096, 0);
  EXPECT_EQ(MY_FILE_ERROR, result);
}

// Test of disk full: read not OK, with MY_NABP
TEST_F(MysysMyReadTest, MyReadFailNABP) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockread(_, _, 4096))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_read(42, buf, 4096, MYF(MY_NABP));
  EXPECT_EQ(MY_FILE_ERROR, result);
}

// Test of normal case: read OK, with MY_FULL_IO
TEST_F(MysysMyReadTest, MyReadOkFULLIO) {
  uchar buf[132096];  // Read 129Kb
  InSequence s;
  EXPECT_CALL(*mockfs, mockread(_, _, 132096))
      .Times(1)
      .WillOnce(Return(132096));

  const size_t result = my_read(42, buf, 132096, MYF(MY_FULL_IO));
  EXPECT_EQ(132096U, result);
}
}  // namespace mysys_my_read_unittest
#endif
