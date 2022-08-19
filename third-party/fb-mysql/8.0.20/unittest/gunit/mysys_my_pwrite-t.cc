/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

// For testing my_pwrite.
extern ssize_t (*mock_pwrite)(int fd, const void *buf, size_t count,
                              off_t offset);

namespace mysys_my_pwrite_unittest {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::SetErrnoAndReturn;

class MockWrite {
 public:
  virtual ~MockWrite() {}
  MOCK_METHOD4(mockwrite, ssize_t(int, const void *, size_t, off_t));
};

MockWrite *mockfs = nullptr;

ssize_t mockfs_pwrite(int fd, const void *buf, size_t count, off_t offset) {
  return mockfs->mockwrite(fd, buf, count, offset);
}

class MysysMyPWriteTest : public ::testing::Test {
  virtual void SetUp() {
    mock_pwrite = mockfs_pwrite;
    mockfs = new MockWrite;
    m_offset = 0;
  }
  virtual void TearDown() {
    mock_pwrite = nullptr;
    delete mockfs;
    mockfs = nullptr;
  }

 public:
  my_off_t m_offset;
};

// Test of normal case: write OK
TEST_F(MysysMyPWriteTest, MyPWriteOK) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(Return(4096));

  const size_t result = my_pwrite(42, buf, 4096, m_offset, 0);
  EXPECT_EQ(4096U, result);
}

// Test of normal case: write OK with MY_NABP
TEST_F(MysysMyPWriteTest, MyPWriteOKNABP) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(Return(4096));

  const size_t result = my_pwrite(42, buf, 4096, m_offset, MYF(MY_NABP));
  EXPECT_EQ(0U, result);
}

// Test of disk full: write not OK
TEST_F(MysysMyPWriteTest, MyPWriteFail) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_pwrite(42, buf, 4096, m_offset, 0);
  EXPECT_EQ(MY_FILE_ERROR, result);
}

// Test of disk full: write not OK, with MY_NABP
TEST_F(MysysMyPWriteTest, MyPWriteFailNABP) {
  uchar buf[4096];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_pwrite(42, buf, 4096, m_offset, MYF(MY_NABP));
  EXPECT_EQ(MY_FILE_ERROR, result);
}

// Test of disk full after partial write.
TEST_F(MysysMyPWriteTest, MyPWrite8192) {
  uchar buf[8192];
  InSequence s;
  // Expect call to write 8192 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 8192, _))
      .Times(1)
      .WillOnce(Return(4096));
  // Expect second call to write remaining 4096 bytes, return disk full.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_pwrite(42, buf, 8192, m_offset, 0);
  EXPECT_EQ(4096U, result);
}

// Test of disk full after partial write.
TEST_F(MysysMyPWriteTest, MyPWrite8192NABP) {
  uchar buf[8192];
  InSequence s;
  // Expect call to write 8192 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 8192, _))
      .Times(1)
      .WillOnce(Return(4096));
  // Expect second call to write remaining 4096 bytes, return disk full.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(ENOSPC, -1));

  const size_t result = my_pwrite(42, buf, 8192, m_offset, MYF(MY_NABP));
  EXPECT_EQ(MY_FILE_ERROR, result);
}

// Test of partial write, followed by interrupt, followed by successful write.
TEST_F(MysysMyPWriteTest, MyPWrite8192Interrupt) {
  uchar buf[8192];
  InSequence s;
  // Expect call to write 8192 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 8192, _))
      .Times(1)
      .WillOnce(Return(4096));
  // Expect second call to write remaining 4096 bytes, return interrupt.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(EINTR, -1));
  // Expect third call to write remaining 4096 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(Return(4096));

  const size_t result = my_pwrite(42, buf, 8192, m_offset, 0);
  EXPECT_EQ(8192U, result);
}

// Test of partial write, followed by interrupt, followed by successful write.
TEST_F(MysysMyPWriteTest, MyPWrite8192InterruptNABP) {
  uchar buf[8192];
  InSequence s;
  // Expect call to write 8192 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 8192, _))
      .Times(1)
      .WillOnce(Return(4096));
  // Expect second call to write remaining 4096 bytes, return interrupt.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(EINTR, -1));
  // Expect third call to write remaining 4096 bytes, return 4096.
  EXPECT_CALL(*mockfs, mockwrite(_, _, 4096, _))
      .Times(1)
      .WillOnce(Return(4096));

  const size_t result = my_pwrite(42, buf, 8192, m_offset, MYF(MY_NABP));
  EXPECT_EQ(0U, result);
}

// Test of partial write, followed successful write.
TEST_F(MysysMyPWriteTest, MyPWrite400) {
  uchar buf[400];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 400, _)).Times(1).WillOnce(Return(200));
  EXPECT_CALL(*mockfs, mockwrite(_, _, 200, _)).Times(1).WillOnce(Return(200));

  const size_t result = my_pwrite(42, buf, 400, m_offset, 0);
  EXPECT_EQ(400U, result);
}

// Test of partial write, followed successful write.
TEST_F(MysysMyPWriteTest, MyPWrite400NABP) {
  uchar buf[400];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 400, _)).Times(1).WillOnce(Return(200));
  EXPECT_CALL(*mockfs, mockwrite(_, _, 200, _)).Times(1).WillOnce(Return(200));

  const size_t result = my_pwrite(42, buf, 400, m_offset, MYF(MY_NABP));
  EXPECT_EQ(0U, result);
}

// Test of partial write, followed by failure, followed successful write.
TEST_F(MysysMyPWriteTest, MyPWrite300) {
  uchar buf[300];
  InSequence s;
  EXPECT_CALL(*mockfs, mockwrite(_, _, 300, _)).Times(1).WillOnce(Return(100));
  EXPECT_CALL(*mockfs, mockwrite(_, _, 200, _))
      .Times(1)
      .WillOnce(SetErrnoAndReturn(EAGAIN, 0));
  EXPECT_CALL(*mockfs, mockwrite(_, _, 200, _)).Times(1).WillOnce(Return(200));

  const size_t result = my_pwrite(42, buf, 300, m_offset, 0);
  EXPECT_EQ(300U, result);
}

}  // namespace mysys_my_pwrite_unittest
#endif
