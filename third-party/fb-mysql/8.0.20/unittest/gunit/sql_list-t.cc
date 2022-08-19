/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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

/*
  This is a simple example of how to use the google unit test framework.

  For an introduction to the constructs used below, see:
  http://code.google.com/p/googletest/wiki/GoogleTestPrimer
*/

#include <gtest/gtest.h>
#include <stddef.h>

#include "my_inttypes.h"
#include "my_thread.h"
#include "sql/sql_error.h"
#include "sql/sql_list.h"
#include "sql/thr_malloc.h"
#include "sql_string.h"
#include "unittest/gunit/test_utils.h"

namespace sql_list_unittest {

// A simple helper function to insert values into a List.
template <class T, int size>
void insert_values(T (&array)[size], List<T> *list) {
  for (int ix = 0; ix < size; ++ix) {
    EXPECT_FALSE(list->push_back(&array[ix]));
  }
}

/*
  The fixture for testing the MySQL List and List_iterator classes.
  A fresh instance of this class will be created for each of the
  TEST_F functions below.
  The functions SetUp(), TearDown(), SetUpTestCase(), TearDownTestCase() are
  inherited from ::testing::Test (google naming style differs from MySQL).
*/
class SqlListTest : public ::testing::Test {
 protected:
  SqlListTest()
      : m_mem_root_p(&m_mem_root), m_int_list(), m_int_list_iter(m_int_list) {}

  virtual void SetUp() {
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_mem_root, 1024, 0);
    THR_MALLOC = &m_mem_root_p;
  }

  virtual void TearDown() { free_root(&m_mem_root, MYF(0)); }

  static void SetUpTestCase() {
    current_thd = nullptr;
    THR_MALLOC = nullptr;
  }

  MEM_ROOT m_mem_root;
  MEM_ROOT *m_mem_root_p;
  List<int> m_int_list;
  List_iterator<int> m_int_list_iter;

 private:
  // Declares (but does not define) copy constructor and assignment operator.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(SqlListTest);
};

// Tests that we can construct and destruct lists.
TEST_F(SqlListTest, ConstructAndDestruct) {
  EXPECT_TRUE(m_int_list.is_empty());
  List<int> *p_int_list = new (*THR_MALLOC) List<int>;
  EXPECT_TRUE(p_int_list->is_empty());
  destroy(p_int_list);
}

// Tests basic operations push and pop.
TEST_F(SqlListTest, BasicOperations) {
  int i1 = 1;
  int i2 = 2;
  EXPECT_FALSE(m_int_list.push_front(&i1));
  EXPECT_FALSE(m_int_list.push_back(&i2));
  EXPECT_FALSE(m_int_list.is_empty());
  EXPECT_EQ(2U, m_int_list.elements);

  EXPECT_EQ(&i1, m_int_list.head());
  EXPECT_EQ(&i1, m_int_list.pop());
  EXPECT_EQ(&i2, m_int_list.head());
  EXPECT_EQ(&i2, m_int_list.pop());
  EXPECT_TRUE(m_int_list.is_empty()) << "The list should be empty now!";
}

// Tests list copying.
TEST_F(SqlListTest, DeepCopy) {
  int values[] = {11, 22, 33, 42, 5};
  insert_values(values, &m_int_list);
  MEM_ROOT mem_root;
  init_alloc_root(PSI_NOT_INSTRUMENTED, &mem_root, 4096, 4096);
  List<int> list_copy(m_int_list, &mem_root);
  EXPECT_EQ(list_copy.elements, m_int_list.elements);
  while (!list_copy.is_empty()) {
    EXPECT_EQ(*m_int_list.pop(), *list_copy.pop());
  }
  EXPECT_TRUE(m_int_list.is_empty());
  free_root(&mem_root, MYF(0));
}

// Tests that we can iterate over values.
TEST_F(SqlListTest, Iterate) {
  int values[] = {3, 2, 1};
  insert_values(values, &m_int_list);
  for (size_t ix = 0; ix < array_elements(values); ++ix) {
    EXPECT_EQ(values[ix], *m_int_list_iter++);
  }
  m_int_list_iter.init(m_int_list);
  int *value;
  int value_number = 0;
  while ((value = m_int_list_iter++)) {
    EXPECT_EQ(values[value_number++], *value);
  }
}

// A simple helper class for testing intrusive lists.
class Linked_node : public ilink<Linked_node> {
 public:
  Linked_node(int val) : m_value(val) {}
  int get_value() const { return m_value; }

 private:
  int m_value;
};
const Linked_node *const null_node = nullptr;

// An example of a test without any fixture.
TEST(SqlIlistTest, ConstructAndDestruct) {
  I_List<Linked_node> i_list;
  I_List_iterator<Linked_node> i_list_iter(i_list);
  EXPECT_TRUE(i_list.is_empty());
  EXPECT_EQ(null_node, i_list_iter++);
}

// Tests iteration over intrusive lists.
TEST(SqlIlistTest, PushBackAndIterate) {
  I_List<Linked_node> i_list;
  I_List_iterator<Linked_node> i_list_iter(i_list);
  int values[] = {11, 22, 33, 42, 5};
  EXPECT_EQ(null_node, i_list.head());
  for (size_t ix = 0; ix < array_elements(values); ++ix) {
    i_list.push_back(new Linked_node(values[ix]));
  }

  Linked_node *node;
  size_t value_number = 0;
  while ((node = i_list_iter++)) {
    EXPECT_EQ(values[value_number++], node->get_value());
  }
  for (value_number = 0; (node = i_list.get()); ++value_number) {
    EXPECT_EQ(values[value_number], node->get_value());
    delete node;
  }
  EXPECT_EQ(array_elements(values), value_number);
}

// Another iteration test over intrusive lists.
TEST(SqlIlistTest, PushFrontAndIterate) {
  I_List<Linked_node> i_list;
  I_List_iterator<Linked_node> i_list_iter(i_list);
  int values[] = {11, 22, 33, 42, 5};
  for (size_t ix = 0; ix < array_elements(values); ++ix) {
    i_list.push_front(new Linked_node(values[ix]));
  }

  Linked_node *node;
  int value_number = array_elements(values) - 1;
  while ((node = i_list_iter++)) {
    EXPECT_EQ(values[value_number--], node->get_value());
  }
  while ((node = i_list.get())) delete node;
}

static int cmp_test(int *a, int *b) {
  return (*(int *)a < *(int *)b) ? -1 : (*(int *)a > *(int *)b) ? 1 : 0;
}

// Tests list sorting.
TEST_F(SqlListTest, Sort) {
  int values[] = {1, 9, 2, 7, 3, 6, 4, 5, 8};
  insert_values(values, &m_int_list);
  m_int_list.sort(cmp_test);
  for (int i = 1; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), i);
  }
  EXPECT_TRUE(m_int_list.is_empty());
  // Test sorting of empty string.
  m_int_list.sort(cmp_test);
  // Check that nothing has changed.
  EXPECT_TRUE(m_int_list.is_empty());
}

// Tests prepend on empty list followed by push_back, Bug#26813454
TEST_F(SqlListTest, PrependBug) {
  int values1[] = {1, 2};
  insert_values(values1, &m_int_list);
  EXPECT_EQ(2U, m_int_list.elements);

  List<int> ilist;
  EXPECT_TRUE(ilist.is_empty());
  ilist.prepend(&m_int_list);

  int values2[] = {3, 4};
  insert_values(values2, &ilist);
  EXPECT_EQ(4U, ilist.elements);

  for (int i = 1; i <= 4; i++) EXPECT_EQ(*ilist.pop(), i);
}

// Tests swap_elts
TEST_F(SqlListTest, Swap) {
  int values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(1, 1), false);
  // Expect no change
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), i);
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(9, 10), true /* error */);
  // Expect no change: 10 out of bounds
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), i);
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(10, 9), true /* error */);
  // Expect no change: 10 out of bounds
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), i);
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(10, 11), true /* error */);
  // Expect no change: 10, 11 out of bounds
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), i);
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(0, 1), false);

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), (i == 0 ? 1 : (i == 1 ? 0 : i)));
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(0, 9), false);

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), (i == 0 ? 9 : (i == 9 ? 0 : i)));
  }

  insert_values(values, &m_int_list);
  EXPECT_EQ(m_int_list.swap_elts(9, 0), false);

  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(*m_int_list.pop(), (i == 0 ? 9 : (i == 9 ? 0 : i)));
  }
}

}  // namespace sql_list_unittest
