/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/intrusive_list_iterator.h"

namespace intrusive_list_iterator_unittest {

struct IntrusivePointee;

class IntrusiveListIteratorTest : public ::testing::Test {
  void SetUp() override;
  void TearDown() override;

 protected:
  IntrusivePointee *first;
};

IntrusivePointee *GetNextInList(const IntrusivePointee *ip);

struct IntrusivePointee {
  int value;
  IntrusivePointee *next;
  IntrusivePointee *GetNext() const { return next; }
};

void IntrusiveListIteratorTest::SetUp() {
  auto two = new IntrusivePointee{2, nullptr};
  auto one = new IntrusivePointee{1, two};
  first = one;
}

void IntrusiveListIteratorTest::TearDown() {
  IntrusiveListContainer<IntrusivePointee, &IntrusivePointee::next> container(
      first);
  auto it = container.begin();
  while (it != container.end()) delete *(it++);
}

TEST_F(IntrusiveListIteratorTest, BarePointer) {
  IntrusiveListIterator<IntrusivePointee, &IntrusivePointee::next> it(first);
  EXPECT_EQ(1, (*it)->value);
}

TEST_F(IntrusiveListIteratorTest, PreIncrement) {
  IntrusiveListIterator<IntrusivePointee, &IntrusivePointee::next> it(first);
  EXPECT_EQ(1, (*it)->value);
  EXPECT_EQ(2, (*++it)->value);
}

// Testing that we can instantiate it without a definition of 'next'.
using TestedIterator = NextFunctionIterator<IntrusivePointee, GetNextInList>;

IntrusivePointee *GetNextInList(const IntrusivePointee *ip) { return ip->next; }

TEST_F(IntrusiveListIteratorTest, IndirectionFunction) {
  TestedIterator it(first);
  EXPECT_EQ(1, (*it)->value);
}

TEST_F(IntrusiveListIteratorTest, PostIncrement) {
  TestedIterator it(first);
  EXPECT_EQ(1, (*it++)->value);
  EXPECT_EQ(2, (*it)->value);
}

TEST_F(IntrusiveListIteratorTest, IteratorContainer) {
  IteratorContainer<TestedIterator> container(first);
  int answers[] = {1, 2};
  int i = 0;
  for (auto element : container) EXPECT_EQ(answers[i++], element->value);
}

TEST_F(IntrusiveListIteratorTest, NextFunctionContainer) {
  NextFunctionContainer<IntrusivePointee, GetNextInList> container(first);
  int answers[] = {1, 2};
  int i = 0;
  for (auto element : container) EXPECT_EQ(answers[i++], element->value);
}
}  // namespace intrusive_list_iterator_unittest
