/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>
#include <algorithm>
#include <functional>
#include <random>
#include <vector>

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_table_map.h"
#include "sql/current_thd.h"
#include "sql/mem_root_array.h"
#include "sql/mysqld.h"         // THR_MALLOC
#include "sql/sql_optimizer.h"  // Key_use_array

/**
   WL#5774 Decrease number of malloc's for normal DML queries.
   One of the malloc's was due to DYNAMIC_ARRAY keyuse;
   We replace the DYNAMIC_ARRAY with a std::vector-like class Mem_root_array.

   Below are unit tests for comparing performance, and for testing
   functionality of Mem_root_array.
*/

namespace dynarray_unittest {

// We generate some random data at startup, for testing of sorting.
void generate_test_data(Key_use *keys, TABLE_LIST *tables, int n) {
  int ix;
  for (ix = 0; ix < n; ++ix) {
    tables[ix].set_tableno(ix % 3);
    keys[ix] = Key_use(&tables[ix],
                       nullptr,  // Item      *val
                       0,        // table_map  used_tables
                       ix % 4,   // uint       key
                       ix % 2,   // uint       keypart
                       0,        // uint       optimize
                       0,        //            keypart_map
                       0,        // ha_rows    ref_table_rows
                       true,     // bool       null_rejecting
                       nullptr,  // bool      *cond_guard
                       0         // uint       sj_pred_no
    );
  }
  std::random_device rng;
  std::mt19937 urng(rng());
  std::shuffle(&keys[0], &keys[n], urng);
}

constexpr int num_elements = 200;

/*
  This class is for unit testing of Mem_root_array.
 */
class MemRootTest : public ::testing::Test {
 protected:
  MemRootTest() : m_mem_root_p(&m_mem_root), m_array_std(m_mem_root_p) {}

  virtual void SetUp() {
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_mem_root, 1024, 0);
    THR_MALLOC = &m_mem_root_p;

    m_array_std.reserve(num_elements);
    destroy_counter = 0;
  }

  virtual void TearDown() { free_root(&m_mem_root, MYF(0)); }

  static void SetUpTestCase() {
    generate_test_data(test_data, table_list, num_elements);
    THR_MALLOC = nullptr;
  }

  static void TearDownTestCase() { THR_MALLOC = nullptr; }

  MEM_ROOT m_mem_root;
  MEM_ROOT *m_mem_root_p;
  Key_use_array m_array_std;

 public:
  static size_t destroy_counter;

 private:
  static Key_use test_data[num_elements];
  static TABLE_LIST table_list[num_elements];

  GTEST_DISALLOW_COPY_AND_ASSIGN_(MemRootTest);
};

size_t MemRootTest::destroy_counter;
Key_use MemRootTest::test_data[num_elements];
TABLE_LIST MemRootTest::table_list[num_elements];

// Test that Mem_root_array re-expanding works.
TEST_F(MemRootTest, Reserve) {
  Mem_root_array<uint> intarr(m_mem_root_p);
  intarr.reserve(2);
  const uint num_pushes = 20;
  for (uint ix = 0; ix < num_pushes; ++ix) {
    EXPECT_EQ(ix, intarr.size());
    EXPECT_FALSE(intarr.push_back(ix));
    EXPECT_EQ(ix, intarr.at(ix));
  }
  for (uint ix = 0; ix < num_pushes; ++ix) {
    EXPECT_EQ(ix, intarr.at(ix));
  }
  EXPECT_EQ(sizeof(uint), intarr.element_size());
  EXPECT_EQ(num_pushes, intarr.size());
  EXPECT_LE(num_pushes, intarr.capacity());
}

class DestroyCounter {
 public:
  DestroyCounter() : p_counter(&MemRootTest::destroy_counter) {}
  DestroyCounter(const DestroyCounter &rhs) : p_counter(rhs.p_counter) {}
  explicit DestroyCounter(size_t *p) : p_counter(p) {}
  DestroyCounter &operator=(const DestroyCounter &) = default;
  ~DestroyCounter() { (*p_counter) += 1; }

 private:
  size_t *p_counter;
};

// Test chop() and clear() and that destructors are executed.
TEST_F(MemRootTest, ChopAndClear) {
  Mem_root_array<DestroyCounter> array(m_mem_root_p);
  const size_t nn = 4;
  array.reserve(nn);
  size_t counter = 0;
  DestroyCounter foo(&counter);
  for (size_t ix = 0; ix < array.capacity(); ++ix) array.push_back(foo);

  EXPECT_EQ(0U, counter);
  array.chop(nn / 2);
  EXPECT_EQ(nn / 2, counter);
  EXPECT_EQ(nn / 2, array.size());

  array.clear();
  EXPECT_EQ(nn, counter);
}

// Test that elements are destroyed if push_back() needs to call reserve().
TEST_F(MemRootTest, ReserveDestroy) {
  Mem_root_array<DestroyCounter> array(m_mem_root_p);
  const size_t nn = 4;
  array.reserve(nn / 2);
  size_t counter = 0;
  DestroyCounter foo(&counter);
  for (size_t ix = 0; ix < nn; ++ix) array.push_back(foo);

  EXPECT_EQ(nn / 2, counter);
  EXPECT_EQ(nn, array.size());

  counter = 0;
  array.clear();
  EXPECT_EQ(nn, counter);
}

TEST_F(MemRootTest, ResizeSame) {
  Mem_root_array<DestroyCounter> array(m_mem_root_p);
  array.reserve(100);
  size_t counter = 0;
  DestroyCounter foo(&counter);
  for (int ix = 0; ix < 10; ++ix) array.push_back(foo);
  EXPECT_EQ(10U, array.size());
  array.resize(10U);
  EXPECT_EQ(10U, array.size());
  array.clear();
  EXPECT_EQ(10U, counter);
}

TEST_F(MemRootTest, ResizeGrow) {
  Mem_root_array<DestroyCounter> array(m_mem_root_p);
  array.reserve(100);
  size_t counter = 0;
  DestroyCounter foo(&counter);
  array.resize(10, foo);
  EXPECT_EQ(0U, counter);
  array.clear();
  EXPECT_EQ(0U, MemRootTest::destroy_counter);
  EXPECT_EQ(10U, counter);
}

TEST_F(MemRootTest, ResizeShrink) {
  size_t counter = 0;
  Mem_root_array<DestroyCounter> array(m_mem_root_p);
  array.reserve(100);
  DestroyCounter foo(&counter);
  array.resize(10, foo);
  EXPECT_EQ(0U, counter);
  array.resize(5);
  EXPECT_EQ(5U, counter);
}

TEST_F(MemRootTest, Erase) {
  using A = Mem_root_array<DestroyCounter>;
  size_t counter = 0;
  DestroyCounter foo(&counter);
  A array(m_mem_root_p);
  array.resize(10, foo);
  EXPECT_EQ(10U, array.size());
  EXPECT_EQ(0U, counter);

  A::iterator it = array.erase(array.cbegin() + 2, array.cbegin() + 4);
  EXPECT_EQ(8U, array.size());
  EXPECT_EQ(array.begin() + 2, it);
  EXPECT_EQ(2U, counter);

  it = array.erase(array.cend(), array.cend());
  EXPECT_EQ(8U, array.size());
  EXPECT_EQ(array.cend(), it);
  EXPECT_EQ(2U, counter);

  it = array.erase(array.cbegin(), array.cbegin());
  EXPECT_EQ(8U, array.size());
  EXPECT_EQ(array.cbegin(), it);
  EXPECT_EQ(2U, counter);

  it = array.erase(array.cbegin(), array.cend());
  EXPECT_EQ(0U, array.size());
  EXPECT_EQ(array.cbegin(), it);
  EXPECT_EQ(array.cend(), it);
  EXPECT_EQ(10U, counter);
}

TEST_F(MemRootTest, Erase2) {
  using A = Mem_root_array<DestroyCounter>;
  size_t counter = 0;
  DestroyCounter foo(&counter);
  A array(m_mem_root_p);
  array.resize(10, foo);
  EXPECT_EQ(10U, array.size());
  EXPECT_EQ(0U, counter);

  A::iterator it = array.erase(5);
  EXPECT_EQ(9U, array.size());
  EXPECT_EQ(std::next(array.cbegin(), 5), it);
  EXPECT_EQ(1U, counter);

  it = array.erase(static_cast<size_t>(0));
  EXPECT_EQ(8U, array.size());
  EXPECT_EQ(array.cbegin(), it);
  EXPECT_EQ(2U, counter);

  it = array.erase(7);
  EXPECT_EQ(7U, array.size());
  EXPECT_EQ(array.cend(), it);
  EXPECT_EQ(3U, counter);
}

TEST_F(MemRootTest, Insert) {
  using A = Mem_root_array<int>;
  A array(m_mem_root_p);
  A::iterator it = array.insert(array.cbegin(), 1);
  EXPECT_EQ(array.cbegin(), it);
  it = array.insert(array.cbegin(), 2);
  EXPECT_EQ(array.cbegin(), it);
  it = array.insert(array.cbegin() + 1, 3);
  EXPECT_EQ(array.cbegin() + 1, it);
  it = array.insert(array.cend(), 4);
  EXPECT_EQ(array.cend() - 1, it);
  EXPECT_EQ(4U, array.size());
  EXPECT_EQ(2, array[0]);
  EXPECT_EQ(3, array[1]);
  EXPECT_EQ(1, array[2]);
  EXPECT_EQ(4, array[3]);
}

}  // namespace dynarray_unittest
