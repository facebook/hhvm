/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include "my_inttypes.h"
#include "sql/current_thd.h"
#include "sql/select_lex_visitor.h"
#include "sql/sql_lex.h"
#include "sql/sql_optimizer.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

namespace select_lex_visitor_unittest {

using my_testing::Server_initializer;
using std::vector;

class SelectLexVisitorTest : public ParserTest {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }
};

/// A visitor that remembers what it has seen.
class Remembering_visitor : public Select_lex_visitor {
 public:
  vector<int> seen_items;
  vector<const char *> field_names;

  Remembering_visitor()
      : m_saw_select_lex(false), m_saw_select_lex_unit(false) {}

  virtual bool visit_union(SELECT_LEX_UNIT *) {
    m_saw_select_lex_unit = true;
    return false;
  }

  virtual bool visit_query_block(SELECT_LEX *) {
    m_saw_select_lex = true;
    return false;
  }

  virtual bool visit_item(Item *item) {
    // Not possible to call val_XXX on item_field. So just store the name.
    if (item->type() == Item::FIELD_ITEM)
      field_names.push_back(item->full_name());
    else
      seen_items.push_back(item->val_int());
    return false;
  }

  bool saw_select_lex() { return m_saw_select_lex; }
  bool saw_select_lex_unit() { return m_saw_select_lex_unit; }

  ~Remembering_visitor() {}

 private:
  bool m_saw_select_lex, m_saw_select_lex_unit;
};

/**
  Google mock only works for objects allocated on the stack, and the Item
  classes appear to have been designed to make it impossible to allocate them
  on the stack because of the mandatory free list. But this little mix-in
  class lets us inherit any item class and do that. See Mock_item_int below
  how to use it.
*/
template <class Item_class>
class Stack_allocated_item : public Item_class {
 public:
  Stack_allocated_item(int value_arg) : Item_class(value_arg) {
    // Undo what Item::Item() does.
    THD *thd = current_thd;
    thd->set_item_list(this->next_free);
    this->next_free = nullptr;
  }
};

class Mock_item_int : public Stack_allocated_item<Item_int> {
 public:
  Mock_item_int() : Stack_allocated_item<Item_int>(42) {}
  MOCK_METHOD3(walk, bool(Item_processor, enum_walk, uchar *));
};

TEST_F(SelectLexVisitorTest, SelectLex) {
  using ::testing::_;

  Mock_item_int where;
  Mock_item_int having;
  EXPECT_CALL(where, walk(_, _, _)).Times(1);
  EXPECT_CALL(having, walk(_, _, _)).Times(1);

  SELECT_LEX query_block(thd()->mem_root, &where, &having);

  SELECT_LEX_UNIT unit(CTX_NONE);

  LEX lex;
  query_block.include_down(&lex, &unit);
  List<Item> items;
  JOIN join(thd(), &query_block);
  join.where_cond = &where;
  join.having_for_explain = &having;

  query_block.join = &join;
  query_block.parent_lex = &lex;

  Remembering_visitor visitor;
  unit.accept(&visitor);
  EXPECT_TRUE(visitor.saw_select_lex());
  EXPECT_TRUE(visitor.saw_select_lex_unit());
}

TEST_F(SelectLexVisitorTest, InsertList) {
  SELECT_LEX *select_lex = parse("INSERT INTO t VALUES (1, 2, 3)", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(3U, visitor.seen_items.size());
  EXPECT_EQ(1, visitor.seen_items[0]);
  EXPECT_EQ(2, visitor.seen_items[1]);
  EXPECT_EQ(3, visitor.seen_items[2]);
}

TEST_F(SelectLexVisitorTest, InsertList2) {
  SELECT_LEX *select_lex = parse("INSERT INTO t VALUES (1, 2), (3, 4)", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(4U, visitor.seen_items.size());
  EXPECT_EQ(1, visitor.seen_items[0]);
  EXPECT_EQ(2, visitor.seen_items[1]);
  EXPECT_EQ(3, visitor.seen_items[2]);
  EXPECT_EQ(4, visitor.seen_items[3]);
}

TEST_F(SelectLexVisitorTest, InsertSet) {
  SELECT_LEX *select_lex = parse("INSERT INTO t SET a=1, b=2, c=3", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(3U, visitor.seen_items.size());
  EXPECT_EQ(1, visitor.seen_items[0]);
  EXPECT_EQ(2, visitor.seen_items[1]);
  EXPECT_EQ(3, visitor.seen_items[2]);

  ASSERT_EQ(3U, visitor.field_names.size());
  EXPECT_STREQ("a", visitor.field_names[0]);
  EXPECT_STREQ("b", visitor.field_names[1]);
  EXPECT_STREQ("c", visitor.field_names[2]);
}

TEST_F(SelectLexVisitorTest, ReplaceList) {
  SELECT_LEX *select_lex =
      parse("REPLACE INTO t(a, b, c) VALUES (1,2,3), (4,5,6)", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(6U, visitor.seen_items.size());
  EXPECT_EQ(1, visitor.seen_items[0]);
  EXPECT_EQ(4, visitor.seen_items[3]);
  EXPECT_EQ(6, visitor.seen_items[5]);

  ASSERT_EQ(3U, visitor.field_names.size());
  EXPECT_STREQ("a", visitor.field_names[0]);
  EXPECT_STREQ("b", visitor.field_names[1]);
  EXPECT_STREQ("c", visitor.field_names[2]);
}

TEST_F(SelectLexVisitorTest, InsertOnDuplicateKey) {
  SELECT_LEX *select_lex = parse(
      "INSERT INTO t VALUES (1,2) ON DUPLICATE KEY UPDATE c= 44, a= 55", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(4U, visitor.seen_items.size());
  EXPECT_EQ(1, visitor.seen_items[0]);
  EXPECT_EQ(44, visitor.seen_items[2]);
  EXPECT_EQ(55, visitor.seen_items[3]);

  ASSERT_EQ(2U, visitor.field_names.size());
  EXPECT_STREQ("c", visitor.field_names[0]);
  EXPECT_STREQ("a", visitor.field_names[1]);
}

TEST_F(SelectLexVisitorTest, Update) {
  SELECT_LEX *select_lex = parse("UPDATE t SET a= 0, c= 25", 0);
  ASSERT_FALSE(select_lex == nullptr);

  Remembering_visitor visitor;
  thd()->lex->accept(&visitor);
  ASSERT_EQ(2U, visitor.seen_items.size());
  EXPECT_EQ(0, visitor.seen_items[0]);
  EXPECT_EQ(25, visitor.seen_items[1]);

  ASSERT_EQ(2U, visitor.field_names.size());
  EXPECT_STREQ("a", visitor.field_names[0]);
  EXPECT_STREQ("c", visitor.field_names[1]);
}
}  // namespace select_lex_visitor_unittest
