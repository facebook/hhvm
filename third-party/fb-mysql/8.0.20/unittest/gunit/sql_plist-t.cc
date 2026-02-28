/* Copyright (c) 2011, 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>

#include "sql/sql_plist.h"
#include "unittest/gunit/test_utils.h"

namespace sql_plist_unittest {

// A simple helper function to insert values into a List.
template <class T, int size, class L>
void insert_values(T (&array)[size], L *list) {
  uint ix, elements = list->elements();
  for (ix = 0; ix < size; ++ix) list->push_back(&array[ix]);
  EXPECT_EQ(ix + elements, list->elements());
}

/*
  The fixture for testing the MySQL List and List_iterator classes.
  A fresh instance of this class will be created for each of the
  TEST_F functions below.
*/
class IPListTest : public ::testing::Test {
 protected:
  IPListTest() : m_int_list(), m_int_list_iter(m_int_list) {}

 public:
  template <typename V>
  struct I_P_ListTestValue {
    V value;
    I_P_ListTestValue(V val) : value(val) {}
    bool operator==(const I_P_ListTestValue<V> &obj) const {
      return value == obj.value;
    }
    struct I_P_ListTestValue<V> *next;
    struct I_P_ListTestValue<V> **prev;
  };

 protected:
  template <typename V>
  struct I_P_ListCountedPushBack {
    typedef I_P_ListTestValue<V> Value;
    typedef I_P_List<Value, I_P_List_adapter<Value, &Value::next, &Value::prev>,
                     I_P_List_counter, I_P_List_fast_push_back<Value>>
        Type;
  };

  I_P_ListCountedPushBack<int>::Type m_int_list;
  I_P_ListCountedPushBack<int>::Type::Iterator m_int_list_iter;

 private:
  // Declares (but does not define) copy constructor and assignment operator.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(IPListTest);
};

// Allow construction of test messages via the << operator.
template <typename T>
std::ostream &operator<<(std::ostream &s,
                         const IPListTest::I_P_ListTestValue<T> &v) {
  return s << v.value;
}

// Tests that we can construct and destruct lists.
TEST_F(IPListTest, ConstructAndDestruct) {
  EXPECT_TRUE(m_int_list.is_empty());
  I_P_ListCountedPushBack<int>::Type *p_int_list;
  p_int_list = new I_P_ListCountedPushBack<int>::Type;
  EXPECT_TRUE(p_int_list->is_empty());
  delete p_int_list;
}

// Tests basic operations push and remove.
TEST_F(IPListTest, BasicOperations) {
  I_P_ListTestValue<int> v1(1), v2(2), v3(3);
  m_int_list.push_front(&v1);
  m_int_list.insert_after(&v1, &v2);
  m_int_list.push_back(&v3);
  EXPECT_FALSE(m_int_list.is_empty());
  EXPECT_EQ(3U, m_int_list.elements());

  EXPECT_EQ(&v1, m_int_list.front());
  m_int_list.remove(&v1);
  EXPECT_EQ(&v2, m_int_list.front());
  m_int_list.remove(&v2);
  EXPECT_EQ(&v3, m_int_list.front());
  m_int_list.remove(&v3);
  EXPECT_TRUE(m_int_list.is_empty()) << "The list should be empty now!";
}

// Tests that we can iterate over values.
TEST_F(IPListTest, Iterate) {
  I_P_ListTestValue<int> values[] = {3, 2, 1};
  insert_values(values, &m_int_list);
  m_int_list_iter.init(m_int_list);
  for (size_t ix = 0; ix < array_elements(values); ++ix) {
    EXPECT_EQ(values[ix], *m_int_list_iter++);
  }
  m_int_list_iter.init(m_int_list);
  I_P_ListTestValue<int> *value;
  int value_number = 0;
  while ((value = m_int_list_iter++)) {
    EXPECT_EQ(values[value_number++], value->value);
  }
}

}  // namespace sql_plist_unittest
