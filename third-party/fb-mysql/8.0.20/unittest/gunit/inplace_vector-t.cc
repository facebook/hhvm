/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <algorithm>

#include "sql/inplace_vector.h"

namespace inplace_vector_unittest {

class InplaceVectorTest : public ::testing::Test {
 public:
  InplaceVectorTest() : int_10(PSI_NOT_INSTRUMENTED) {}

 protected:
  Inplace_vector<int, 5> int_10;
  int some_integer;
};

TEST_F(InplaceVectorTest, Empty) {
  EXPECT_TRUE(int_10.empty());
  EXPECT_EQ(0U, int_10.size());
}

#if !defined(DBUG_OFF)
// Google Test recommends DeathTest suffix for classes used in death tests.
typedef InplaceVectorTest InplaceVectorDeathTest;

TEST_F(InplaceVectorDeathTest, OutOfBoundsRead) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(some_integer = int_10[5],
                            ".*Assertion .*i < size.*");
}

TEST_F(InplaceVectorDeathTest, OutOfBoundsWrite) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(int_10[5] = some_integer,
                            ".*Assertion .*i < size.*");
}

TEST_F(InplaceVectorDeathTest, EmptyBackRead) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(some_integer = int_10.back(),
                            ".*Assertion .*size.*0.*");
}
TEST_F(InplaceVectorDeathTest, EmptyBackWrite) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  EXPECT_DEATH_IF_SUPPORTED(int_10.back() = 42, ".*Assertion .*size.*0.*");
}

#endif  // DBUG_OFF

TEST_F(InplaceVectorTest, Insert5) {
  for (int ix = 0; ix < 5; ++ix) int_10.push_back(ix);
  for (int ix = 0; ix < 5; ++ix) EXPECT_EQ(ix, int_10[ix]);
  for (int ix = 0; ix < 5; ++ix) int_10[ix] = ix;
  EXPECT_EQ(5U, int_10.size());
  EXPECT_EQ(5U, int_10.capacity());
}

TEST_F(InplaceVectorTest, Insert15) {
  for (int ix = 0; ix < 15; ++ix) int_10.push_back(ix);
  for (int ix = 0; ix < 15; ++ix) EXPECT_EQ(ix, int_10[ix]);
  for (int ix = 0; ix < 15; ++ix) int_10[ix] = ix;
  EXPECT_EQ(15U, int_10.size());
  EXPECT_EQ(15U, int_10.capacity());
  int_10.push_back(16);
  EXPECT_EQ(20U, int_10.capacity());
}

TEST_F(InplaceVectorTest, Back) {
  for (int ix = 0; ix <= 15; ++ix) int_10.push_back(ix);
  EXPECT_EQ(15, int_10.back());
  int_10.back() = 42;
  EXPECT_EQ(42, int_10.back());
}

TEST_F(InplaceVectorTest, ResizeSame) {
  for (int ix = 0; ix <= 15; ++ix) int_10.push_back(ix);
  EXPECT_EQ(16U, int_10.size());
  int_10.resize(16U);
  EXPECT_EQ(16U, int_10.size());
}

TEST_F(InplaceVectorTest, ResizeGrow) {
  int_10.push_back(1);
  int_10.resize(20);
  EXPECT_EQ(1, int_10[0]);
  EXPECT_EQ(0, int_10[1]);
  EXPECT_EQ(20U, int_10.size());
  EXPECT_EQ(int_10.capacity(), 20U);
}

TEST_F(InplaceVectorTest, ResizeGrowVal) {
  int_10.resize(20, 42);
  EXPECT_EQ(42, int_10[0]);
  EXPECT_EQ(42, int_10[19]);
  EXPECT_EQ(20U, int_10.size());
  EXPECT_EQ(int_10.capacity(), 20U);
}

TEST_F(InplaceVectorTest, ResizeShrink) {
  for (int ix = 0; ix <= 15; ++ix) int_10.push_back(ix);
  EXPECT_EQ(16U, int_10.size());
  EXPECT_EQ(int_10.capacity(), 20U);
  int_10.resize(10);
  EXPECT_EQ(10U, int_10.size());
  EXPECT_EQ(int_10.capacity(), 15U);

  int_10.resize(3);
  EXPECT_EQ(3U, int_10.size());
  EXPECT_EQ(int_10.capacity(), 5U);
}

/*
  A simple class for testing that object copying and destruction is done
  properly when we have to expand the array a few times.
 */
class IntWrap {
 public:
  IntWrap() { m_int = new int(0); }
  explicit IntWrap(int arg) { m_int = new int(arg); }
  IntWrap(const IntWrap &other) { m_int = new int(other.getval()); }
  ~IntWrap() { delete m_int; }
  int getval() const { return *m_int; }

 private:
  int *m_int;
};

/*
  To verify that there are no leaks, do:
  valgrind ./inplace_vector-t --gtest_filter="-*DeathTest*"
*/
TEST_F(InplaceVectorTest, NoMemLeaksPushing) {
  Inplace_vector<IntWrap, 5> array(PSI_NOT_INSTRUMENTED);
  for (int ix = 0; ix < 42; ++ix) array.push_back(IntWrap(ix));
  for (int ix = 0; ix < 42; ++ix) EXPECT_EQ(ix, array[ix].getval());
}

TEST_F(InplaceVectorTest, NoMemLeaksClearing) {
  Inplace_vector<IntWrap, 5> array(PSI_NOT_INSTRUMENTED);
  for (int ix = 0; ix < 42; ++ix) array.push_back(IntWrap(ix));
  array.clear();
  EXPECT_EQ(0U, array.size());
  EXPECT_EQ(0U, array.capacity());

  array.push_back(IntWrap(1));
  EXPECT_EQ(1U, array.size());
  EXPECT_EQ(5U, array.capacity());
}

TEST_F(InplaceVectorTest, NoMemLeaksResizing) {
  Inplace_vector<IntWrap, 5> array(PSI_NOT_INSTRUMENTED);
  for (int ix = 0; ix < 42; ++ix) array.push_back(IntWrap(ix));
  array.resize(0);
  EXPECT_EQ(0U, array.size());
  EXPECT_EQ(5U, array.capacity());

  array.push_back(IntWrap(1));
  EXPECT_EQ(1U, array.size());
  EXPECT_EQ(5U, array.capacity());
}

/*
  A vector consists of a list of arrays of objects. Test that all
  elements of all arrays are destroyed when the vector is
  destroyed. If run in valgrind, these tests will report memory leaks
  if some objects aren't destroyed.
 */
class InplaceVectorTestP : public ::testing::TestWithParam<size_t> {
 protected:
  InplaceVectorTestP() : array(PSI_NOT_INSTRUMENTED) {}
  virtual void SetUp() { n_elems = GetParam(); }
  size_t n_elems;
  Inplace_vector<IntWrap, 5> array;
};

size_t test_values[] = {5, 10, 15, 20};

INSTANTIATE_TEST_CASE_P(NoMemLeaks, InplaceVectorTestP,
                        ::testing::ValuesIn(test_values));

TEST_P(InplaceVectorTestP, DestroyingFullArrays) {
  for (size_t ix = 0; ix < n_elems; ++ix) array.push_back(IntWrap(ix));
  EXPECT_EQ(n_elems, array.size());
  EXPECT_EQ(n_elems, array.capacity());
}

TEST_P(InplaceVectorTestP, DestroyingAlmostFullArrays) {
  for (size_t ix = 0; ix < n_elems - 1; ++ix) array.push_back(IntWrap(ix));
  EXPECT_EQ(n_elems - 1, array.size());
  EXPECT_EQ(n_elems, array.capacity());
}

TEST_P(InplaceVectorTestP, DestroyingAlmostEmptyArrays) {
  for (size_t ix = 0; ix < n_elems - 5 + 1; ++ix) array.push_back(IntWrap(ix));
  EXPECT_EQ(n_elems - 5 + 1, array.size());
  EXPECT_EQ(n_elems, array.capacity());
}

/*
  A simple class to verify that Inplace_vector also works for
  classes which have their own operator new/delete.
 */
class TestAlloc {
 public:
  explicit TestAlloc(int val) : m_int(val) {}

  int getval() const { return m_int; }

 private:
  int m_int;

  static void *operator new(size_t) { throw std::bad_alloc(); }
};

/*
  There is no THD and no mem-root available for the execution of this test.
  This shows that the memory management of Inplace_vector works OK for
  classes with their own new/delete.
 */
TEST_F(InplaceVectorTest, CustomNewDelete) {
  Inplace_vector<TestAlloc, 5> array(PSI_NOT_INSTRUMENTED);
  for (int ix = 0; ix < 42; ++ix) array.push_back(TestAlloc(ix));
  for (int ix = 0; ix < 42; ++ix) EXPECT_EQ(ix, array[ix].getval());

  EXPECT_EQ(array.size(), 42U);
  EXPECT_EQ(array.capacity(), 45U);
}

}  // namespace inplace_vector_unittest
