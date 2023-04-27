/*
   Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_alloc.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysys_err.h"

extern "C" void mock_error_handler_hook(uint err, const char *str, myf MyFlags);

/**
  An alternative error_handler for non-server unit tests since it does
  not rely on THD.  It sets the global error handler function.
*/
class Mock_global_error_handler {
 public:
  Mock_global_error_handler(uint expected_error)
      : m_expected_error(expected_error), m_handle_called(0) {
    current = this;
    m_old_error_handler_hook = error_handler_hook;
    error_handler_hook = mock_error_handler_hook;
  }

  virtual ~Mock_global_error_handler() {
    if (m_expected_error == 0) {
      EXPECT_EQ(0, m_handle_called);
    } else {
      EXPECT_GT(m_handle_called, 0);
    }
    error_handler_hook = m_old_error_handler_hook;
    current = nullptr;
  }

  void error_handler(uint err) {
    EXPECT_EQ(m_expected_error, err);
    ++m_handle_called;
  }

  int handle_called() const { return m_handle_called; }

  static Mock_global_error_handler *current;

 private:
  uint m_expected_error;
  int m_handle_called;

  void (*m_old_error_handler_hook)(uint, const char *, myf);
};

Mock_global_error_handler *Mock_global_error_handler::current = nullptr;

/*
  Error handler function.
*/
extern "C" void mock_error_handler_hook(uint err, const char *, myf) {
  if (Mock_global_error_handler::current)
    Mock_global_error_handler::current->error_handler(err);
}

namespace my_alloc_unittest {

const size_t num_iterations = 1ULL;

class MyAllocTest : public ::testing::TestWithParam<size_t> {
 protected:
  virtual void SetUp() {
    init_alloc_root(PSI_NOT_INSTRUMENTED, &m_root, 1024, 0);
  }
  virtual void TearDown() { free_root(&m_root, MYF(0)); }
  size_t m_num_objects;
  MEM_ROOT m_root;
};

class MyPreAllocTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    init_alloc_root(PSI_NOT_INSTRUMENTED, &m_prealloc_root, 1024, 2048);
  }
  virtual void TearDown() { free_root(&m_prealloc_root, MYF(0)); }
  size_t m_num_objects;
  MEM_ROOT m_prealloc_root;
};

size_t test_values[] = {100, 1000, 10000, 100000};

INSTANTIATE_TEST_CASE_P(MyAlloc, MyAllocTest, ::testing::ValuesIn(test_values));

TEST_P(MyAllocTest, NoMemoryLimit) {
  m_num_objects = GetParam();
  for (size_t ix = 0; ix < num_iterations; ++ix) {
    for (size_t objcount = 0; objcount < m_num_objects; ++objcount)
      m_root.Alloc(100);
  }
}

TEST_P(MyAllocTest, WithMemoryLimit) {
  m_num_objects = GetParam();
  m_root.set_max_capacity(num_iterations * m_num_objects * 100);
  for (size_t ix = 0; ix < num_iterations; ++ix) {
    for (size_t objcount = 0; objcount < m_num_objects; ++objcount)
      m_root.Alloc(100);
  }
}

TEST_F(MyAllocTest, CheckErrorReporting) {
  const void *null_pointer = nullptr;
  EXPECT_TRUE(m_root.Alloc(1000));
  m_root.set_max_capacity(100);
  EXPECT_EQ(null_pointer, m_root.Alloc(1000));
  m_root.set_error_for_capacity_exceeded(true);
  Mock_global_error_handler error_handler(EE_CAPACITY_EXCEEDED);
  EXPECT_TRUE(m_root.Alloc(1000));
  EXPECT_EQ(1, error_handler.handle_called());
}

TEST_F(MyAllocTest, MoveConstructorDoesNotLeak) {
  MEM_ROOT alloc1(PSI_NOT_INSTRUMENTED, 512);
  (void)alloc1.Alloc(10);
  MEM_ROOT alloc2(PSI_NOT_INSTRUMENTED, 512);
  (void)alloc2.Alloc(30);
  alloc1 = std::move(alloc2);
}

TEST_F(MyAllocTest, ExceptionalBlocksAreNotReusedForLargerAllocations) {
  MEM_ROOT alloc(PSI_NOT_INSTRUMENTED, 512);
  void *ptr = alloc.Alloc(600);
  alloc.ClearForReuse();

  if (alloc.allocated_size() == 0) {
    /*
      The MEM_ROOT was all cleared out (probably because we're running under
      Valgrind/ASAN), so we are obviously not doing any reuse. Moreover,
      the test below is unsafe in this case, since the system malloc() could
      reuse the block.
    */
    return;
  }

  // The allocated block is too small to satisfy this new, larger allocation.
  void *ptr2 = alloc.Alloc(605);
  EXPECT_NE(ptr, ptr2);
}

}  // namespace my_alloc_unittest
