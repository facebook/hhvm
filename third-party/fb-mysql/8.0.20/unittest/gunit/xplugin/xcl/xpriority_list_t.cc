/*
 * Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "plugin/x/client/xpriority_list.h"

namespace xcl {
namespace test {

using ::testing::ContainerEq;
using ::testing::Test;
using ::testing::Values;
using ::testing::WithParamInterface;

class String_with_prio {
 public:
  String_with_prio(const std::string &v, int prio) : m_v(v), m_prio(prio) {}

  static bool compare(const String_with_prio &lhs,
                      const String_with_prio &rhs) {
    return lhs.m_prio < rhs.m_prio;
  }

  bool operator==(const std::string &expected_value) const {
    return expected_value == get_value();
  }

  std::string get_value() const { return m_v; }

  std::string m_v;
  int m_prio;
};

class Direction {
 public:
  Direction() : m_element("", 0) {}

  Direction(const bool is_front, const std::string &value, const int prio)
      : m_element(value, prio), m_is_front(is_front) {}

  operator std::string() const { return m_element.get_value(); }

  bool operator==(const std::string &expected_value) const {
    return expected_value == m_element.get_value();
  }

  String_with_prio m_element;
  bool m_is_front;
};

template <bool is_front>
class Front_back : public Direction {
 public:
  Front_back(const std::string &value, const int prio)
      : Direction(is_front, value, prio) {}
};

using F = Front_back<true>;
using B = Front_back<false>;

class Xcl_xpriority_list_tests : public Test {
 public:
  using Test_list = Priority_list<String_with_prio>;

 public:
  void push(const Direction &d) {
    if (d.m_is_front) {
      m_sut->push_front(d.m_element);
    } else {
      m_sut->push_back(d.m_element);
    }
  }

  void push_and_assert(const std::string &scope_name,
                       const std::vector<Direction> &elements) {
    std::vector<std::string> expected_from_elements(elements.size());

    std::copy(elements.begin(), elements.end(), expected_from_elements.begin());

    push_and_assert(scope_name, expected_from_elements, elements);
  }

  void push_and_assert(const std::string &scope_name,
                       const std::vector<std::string> &expected_elements,
                       const std::vector<Direction> &elements) {
    SCOPED_TRACE(scope_name.c_str());

    m_sut->clear();

    for (const auto &element : elements) {
      push(element);
    }

    assert_plist(expected_elements);
  }

  void assert_plist(const std::vector<std::string> &expected_elements) {
    auto plist_element = m_sut->begin();

    for (const auto &expected_element : expected_elements) {
      ASSERT_NE(m_sut->end(), plist_element);
      ASSERT_EQ(expected_element, (*plist_element).get_value());

      ++plist_element;
    }

    ASSERT_EQ(m_sut->end(), plist_element);
  }

  void assert_plist_size(const uint32_t expected_elements_count) {
    const auto elements_count = std::distance(m_sut->begin(), m_sut->end());

    ASSERT_EQ(expected_elements_count, elements_count);
  }

  void assert_plist_remove(const std::string &element) {
    auto plist_iterator = std::find(m_sut->begin(), m_sut->end(), element);

    ASSERT_NE(m_sut->end(), plist_iterator);

    m_sut->erase(plist_iterator);
  }

  void assert_plist_remove_element_and_verify(
      const std::string &scope_name, const std::string &element_to_remove,
      const std::vector<std::string> &expected_elements_after_remove) {
    SCOPED_TRACE(scope_name.c_str());

    assert_plist_remove(element_to_remove);
    assert_plist_size(
        static_cast<uint32_t>(expected_elements_after_remove.size()));
    assert_plist(expected_elements_after_remove);
  }

  // priority list
  std::unique_ptr<Test_list> m_sut{new Test_list()};
};

TEST_F(Xcl_xpriority_list_tests, simple_operations) {
  push_and_assert("Push_one_element", {"A"}, {F{"A", 1}});
  assert_plist_size(1);

  m_sut->erase(m_sut->begin());
  assert_plist_size(0);

  push_and_assert("Push_one_element", {"B"}, {F{"B", 2}});
  assert_plist_size(1);

  m_sut->erase(m_sut->begin());
  assert_plist_size(0);
}

// All elements has different priorities.
TEST_F(Xcl_xpriority_list_tests, sequence_of_pushes_doesn_t_matter) {
  const std::vector<std::string> expected_sequence{"a1", "b2", "c3", "d4",
                                                   "e5"};

  push_and_assert("All_front", expected_sequence,
                  {F{"a1", 1}, F{"b2", 2}, F{"c3", 3}, F{"d4", 4}, F{"e5", 5}});

  push_and_assert("All_back", expected_sequence,
                  {B{"a1", 1}, B{"b2", 2}, B{"c3", 3}, B{"d4", 4}, B{"e5", 5}});

  push_and_assert("Mixed_front_back", expected_sequence,
                  {F{"e5", 5}, B{"a1", 1}, B{"d4", 4}, F{"b2", 2}, F{"c3", 3}});
}

TEST_F(Xcl_xpriority_list_tests, sequence_of_pushes_matters_on_same_priority) {
  push_and_assert("All_front", {"e1", "d1", "c1", "b1", "a1"},
                  {F{"a1", 1}, F{"b1", 1}, F{"c1", 1}, F{"d1", 1}, F{"e1", 1}});

  push_and_assert("All_back", {"a1", "b1", "c1", "d1", "e1"},
                  {B{"a1", 1}, B{"b1", 1}, B{"c1", 1}, B{"d1", 1}, B{"e1", 1}});

  push_and_assert("Mixed_front_back", {"d1", "b1", "a1", "c1", "e1"},
                  {B{"a1", 1}, F{"b1", 1}, B{"c1", 1}, F{"d1", 1}, B{"e1", 1}});
}

TEST_F(Xcl_xpriority_list_tests,
       sequence_of_pushes_matters_on_mixed_priorities) {
  push_and_assert("All_front", {"b1", "a1", "b2", "a2", "a3"},
                  {F{"a1", 1}, F{"a2", 2}, F{"b2", 2}, F{"b1", 1}, F{"a3", 3}});

  push_and_assert("All_back", {"a1", "b1", "a2", "b2", "a3"},
                  {B{"a1", 1}, B{"a2", 2}, B{"b2", 2}, B{"b1", 1}, B{"a3", 3}});

  push_and_assert("Mixed_front_back",
                  {"c1", "a1", "b1", "b2", "a2", "c2", "b3", "a3"},
                  {B{"a2", 2}, F{"a1", 1}, F{"b2", 2}, B{"c2", 2}, B{"b1", 1},
                   B{"a3", 3}, F{"b3", 3}, F{"c1", 1}});
}

TEST_F(Xcl_xpriority_list_tests, remove_holds_elements_in_sequence) {
  push_and_assert("Initialize_and_verify_the_plist",
                  {"c1", "a1", "b1", "b2", "a2", "c2", "b3", "a3"},
                  {B{"a2", 2}, F{"a1", 1}, F{"b2", 2}, B{"c2", 2}, B{"b1", 1},
                   B{"a3", 3}, F{"b3", 3}, F{"c1", 1}});

  assert_plist_size(8);

  assert_plist_remove_element_and_verify(
      "remove_first", "b1", {"c1", "a1", "b2", "a2", "c2", "b3", "a3"});

  assert_plist_remove_element_and_verify("remove_second", "b2",
                                         {"c1", "a1", "a2", "c2", "b3", "a3"});

  assert_plist_remove_element_and_verify("remove_third", "b3",
                                         {"c1", "a1", "a2", "c2", "a3"});

  assert_plist_remove_element_and_verify("remove_fourth", "c2",
                                         {"c1", "a1", "a2", "a3"});

  assert_plist_remove_element_and_verify("remove_fifth", "a3",
                                         {
                                             "c1",
                                             "a1",
                                             "a2",
                                         });

  assert_plist_remove_element_and_verify("remove_sixth", "a1", {"c1", "a2"});

  assert_plist_remove_element_and_verify("remove_seventh", "a2",
                                         {
                                             "c1",
                                         });

  assert_plist_remove_element_and_verify("remove_eight", "c1", {});
}

}  // namespace test
}  // namespace xcl
