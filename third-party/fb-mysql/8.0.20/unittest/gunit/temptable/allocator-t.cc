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

#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <memory>
#include <vector>

#include "allocator_helper.h"
#include "storage/temptable/include/temptable/allocator.h"
#include "storage/temptable/src/allocator.cc"

namespace temptable_test {

/* TestItem class. */

class TestItem {
 public:
  explicit TestItem();

  TestItem(int first, int second);

  ~TestItem();

  int get_first() const;

  int get_second() const;

  static void reset_counters();

  static int get_default_constructor_call_count();

  static int get_parametrized_constructor_call_count();

  static int get_destructor_call_count();

 private:
  int m_first;

  int m_second;

  static int s_default_constructor_call_count;

  static int s_parametrized_constructor_call_count;

  static int s_destructor_call_count;
};

TestItem::TestItem() : m_first(0), m_second(0) {
  ++s_default_constructor_call_count;
}

TestItem::TestItem(int first, int second) : m_first(first), m_second(second) {
  ++s_parametrized_constructor_call_count;
}

TestItem::~TestItem() { ++s_destructor_call_count; }

int TestItem::get_first() const { return m_first; }

int TestItem::get_second() const { return m_second; }

void TestItem::reset_counters() {
  s_default_constructor_call_count = 0;
  s_parametrized_constructor_call_count = 0;
  s_destructor_call_count = 0;
}

int TestItem::get_default_constructor_call_count() {
  return s_default_constructor_call_count;
}

int TestItem::get_parametrized_constructor_call_count() {
  return s_parametrized_constructor_call_count;
}

int TestItem::get_destructor_call_count() { return s_destructor_call_count; }

int TestItem::s_default_constructor_call_count = -1;

int TestItem::s_parametrized_constructor_call_count = -1;

int TestItem::s_destructor_call_count = -1;

/* End of TestItem class. */

namespace {

void init_allocator_once() {
  static bool inited = false;
  if (!inited) {
    temptable::Allocator<int>::init();
  }
}

}  // namespace

TEST(Allocator, BasicAlloc) {
  Allocator_helper::set_allocator_max_ram_default();
  init_allocator_once();

  using Item = int;
  const int ITEM_COUNT = 100;

  temptable::Allocator<Item> allocator;

  std::vector<int *> item_pointers;
  item_pointers.assign(ITEM_COUNT, nullptr);

  /* Allocate and then deallocate in same order. */
  for (int i = 0; i < ITEM_COUNT; ++i) {
    EXPECT_NO_THROW(item_pointers[i] = allocator.allocate(i + 1));
  }
  for (int i = 0; i < ITEM_COUNT; ++i) {
    EXPECT_NO_THROW(allocator.deallocate(item_pointers[i], i + 1));
    item_pointers[i] = nullptr;
  }

  /* Allocate and then deallocate in reverse order. */
  for (int i = 0; i < ITEM_COUNT; ++i) {
    EXPECT_NO_THROW(item_pointers[i] = allocator.allocate(i + 1));
  }
  for (int i = ITEM_COUNT - 1; i >= 0; --i) {
    EXPECT_NO_THROW(allocator.deallocate(item_pointers[i], i + 1));
    item_pointers[i] = nullptr;
  }

  /* Allocate and then deallocate in quasi-random order. */
  for (int i = 0; i < ITEM_COUNT; ++i) {
    EXPECT_NO_THROW(item_pointers[i] = allocator.allocate(i + 1));
  }
  for (int i = 0; i < ITEM_COUNT; i += 3) {
    EXPECT_NO_THROW(allocator.deallocate(item_pointers[i], i + 1));
    item_pointers[i] = nullptr;
  }
  for (int i = ITEM_COUNT - 1; i >= 0; --i) {
    if (item_pointers[i]) {
      EXPECT_NO_THROW(allocator.deallocate(item_pointers[i], i + 1));
      item_pointers[i] = nullptr;
    }
  }
}

TEST(Allocator, ZeroSize) {
  Allocator_helper::set_allocator_max_ram_default();
  init_allocator_once();

  temptable::Allocator<int> allocator;

  int *item = nullptr;

  EXPECT_NO_THROW(item = allocator.allocate(0));
  EXPECT_NO_THROW(allocator.deallocate(item, 0));
}

TEST(Allocator, ConstructDestroy) {
  Allocator_helper::set_allocator_max_ram_default();
  init_allocator_once();

  temptable::Allocator<TestItem> allocator;

  TestItem *item = nullptr;

  TestItem::reset_counters();

  EXPECT_NO_THROW(item = allocator.allocate(1));
  EXPECT_NO_THROW(allocator.construct(item));
  EXPECT_EQ(item->get_first(), 0);
  EXPECT_EQ(item->get_second(), 0);
  EXPECT_NO_THROW(allocator.destroy(item));
  EXPECT_NO_THROW(allocator.deallocate(item, 1));
  item = nullptr;

  EXPECT_NO_THROW(item = allocator.allocate(1));
  EXPECT_NO_THROW(allocator.construct(item, 1, 2));
  EXPECT_EQ(item->get_first(), 1);
  EXPECT_EQ(item->get_second(), 2);
  EXPECT_NO_THROW(allocator.destroy(item));
  EXPECT_NO_THROW(allocator.deallocate(item, 1));
  item = nullptr;

  EXPECT_EQ(TestItem::get_default_constructor_call_count(), 1);
  EXPECT_EQ(TestItem::get_parametrized_constructor_call_count(), 1);
  EXPECT_EQ(TestItem::get_destructor_call_count(), 2);
}

TEST(Allocator, Casts) {
  Allocator_helper::set_allocator_max_ram_default();
  init_allocator_once();

  using ItemType1 = char;
  using ItemType2 = int;
  using ItemType3 = TestItem;

  temptable::Allocator<ItemType1> allocator1;
  temptable::Allocator<ItemType2> allocator2(allocator1);
  temptable::Allocator<ItemType3> allocator3(allocator2);

  EXPECT_EQ(allocator1, allocator2);
  EXPECT_EQ(allocator1, allocator3);
  EXPECT_EQ(allocator2, allocator3);

  ItemType1 *items1[2] = {};
  ItemType2 *items2[2] = {};
  ItemType3 *items3[2] = {};

  EXPECT_NO_THROW(items1[0] = allocator1.allocate(1));
  EXPECT_NO_THROW(items2[0] = allocator2.allocate(1));
  EXPECT_NO_THROW(items3[0] = allocator3.allocate(1));
  EXPECT_NO_THROW(items1[1] = allocator1.allocate(1));
  EXPECT_NO_THROW(items2[1] = allocator2.allocate(1));
  EXPECT_NO_THROW(items3[1] = allocator3.allocate(1));

  EXPECT_NO_THROW(allocator3.deallocate(items3[0], 1));
  EXPECT_NO_THROW(allocator3.deallocate(items3[1], 1));
  EXPECT_NO_THROW(allocator2.deallocate(items2[0], 1));
  EXPECT_NO_THROW(allocator2.deallocate(items2[1], 1));
  EXPECT_NO_THROW(allocator1.deallocate(items1[0], 1));
  EXPECT_NO_THROW(allocator1.deallocate(items1[1], 1));
}

}  // namespace temptable_test
