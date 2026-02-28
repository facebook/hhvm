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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>
#include <sys/types.h>

#include "sql/parse_tree_helpers.h"
#include "sql/sql_optimizer.cc"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/test_utils.h"

// Unit tests of the ref optimizer.
namespace opt_ref_unittest {

using my_testing::Server_initializer;

bool scrap_bool;  // Needed by Key_field CTOR.

/*
  Class for easy creation of an array of Key_field's. Must be the same
  size as Key_field.
*/
class Fake_key_field : public Key_field {
 public:
  Fake_key_field()
      : Key_field(nullptr, nullptr, 0, 0, false, false, &scrap_bool, 0) {}
};

/*
  Class that tests the ref optimizer. The class creates the fake table
  definitions:

  t1(a int, b int, key(a, b))
  t2(a int, b int)
*/
class OptRefTest : public ::testing::Test {
 public:
  OptRefTest()
      : field_t1_a("field1", true),
        field_t1_b("field2", true),
        field_t2_a("field3", true),
        field_t2_b("field4", true),
        t1(&field_t1_a, &field_t1_b),
        t2(&field_t2_a, &field_t2_b),
        t1_key_fields(&t1_key_field_arr[0]) {
    index_over_t1ab_id = t1.create_index(&field_t1_a, &field_t1_b);
    indexes.set_bit(index_over_t1ab_id);

    t1_join_tab.set_qs(&t1_qep_shared);
    t1_join_tab.set_table(&t1);
    t1_table_list.select_lex = t1.pos_in_table_list->select_lex;
    t1.set_pos_in_table_list(&t1_table_list);

    t1_table_list.table = &t1;
    t1_table_list.embedding = nullptr;
    t1_table_list.derived_keys_ready = true;
    t1_table_list.set_tableno(0);
  }

  virtual void SetUp() {
    // We do some pointer arithmetic on these
    static_assert(sizeof(Fake_key_field) == sizeof(Key_field), "");
    initializer.SetUp();

    item_zero = new Item_int(0);
    item_one = new Item_int(1);

    item_field_t1_a = new Item_field(&field_t1_a);
    item_field_t1_b = new Item_field(&field_t1_b);

    item_field_t2_a = new Item_field(&field_t2_a);
    item_field_t2_b = new Item_field(&field_t2_b);
  }

  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Key_map indexes;

  Mock_field_long field_t1_a, field_t1_b;
  Mock_field_long field_t2_a, field_t2_b;

  Fake_TABLE t1;
  Fake_TABLE t2;
  TABLE_LIST t1_table_list;
  TABLE_LIST t2_table_list;

  JOIN_TAB t1_join_tab;
  QEP_shared t1_qep_shared;
  Fake_key_field t1_key_field_arr[10];
  Key_field *t1_key_fields;

  Item_int *item_zero;
  Item_int *item_one;

  Item_field *item_field_t1_a, *item_field_t1_b;
  Item_field *item_field_t2_a, *item_field_t2_b;

  int index_over_t1ab_id;

  void call_add_key_fields(Item *cond) {
    uint and_level = 0;
    (void)add_key_fields(thd(), nullptr /* join */, &t1_key_fields, &and_level,
                         cond, ~0ULL, nullptr);
  }

 private:
  Server_initializer initializer;
};

static Item_row *make_item_row(Item *a, Item *b) {
  /*
    The Item_row CTOR doesn't store the reference to the list, hence
    it can live on the stack.
  */
  List<Item> items;
  items.push_front(b);
  return new Item_row(POS(), a, items);
}

TEST_F(OptRefTest, addKeyFieldsFromInOneRow) {
  /*
    We simulate the where condition (a, b) IN ((0, 0)). Note that this
    can't happen in practice since the parser is hacked to parse such
    an expression in to (a, b) = (0, 0), which gets rewritten into a =
    0 AND b = 0 before the ref optimizer runs.
   */
  PT_item_list *all_args = new (current_thd->mem_root) PT_item_list;
  all_args->push_front(make_item_row(item_zero, item_zero));
  all_args->push_front(make_item_row(item_field_t1_a, item_field_t1_b));
  Item *cond = new Item_func_in(POS(), all_args, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(cond->itemize(&pc, &cond));

  call_add_key_fields(cond);

  // We expect the key_fields pointer not to be incremented.
  EXPECT_EQ(0, t1_key_fields - static_cast<Key_field *>(&t1_key_field_arr[0]));
  EXPECT_EQ(indexes, t1_join_tab.const_keys)
      << "SARGable index not present in const_keys";
  EXPECT_EQ(indexes, t1_join_tab.keys());
  EXPECT_EQ(0U, t1_key_field_arr[0].level);
  EXPECT_EQ(0U, t1_key_field_arr[1].level);
}

TEST_F(OptRefTest, addKeyFieldsFromInTwoRows) {
  // We simulate the where condition (col_a, col_b) IN ((0, 0), (1, 1))
  PT_item_list *all_args = new (current_thd->mem_root) PT_item_list;
  all_args->push_front(make_item_row(item_one, item_one));
  all_args->push_front(make_item_row(item_zero, item_zero));
  all_args->push_front(make_item_row(item_field_t1_a, item_field_t1_b));
  Item *cond = new Item_func_in(POS(), all_args, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(cond->itemize(&pc, &cond));

  call_add_key_fields(cond);

  // We expect the key_fields pointer not to be incremented.
  EXPECT_EQ(0, t1_key_fields - static_cast<Key_field *>(&t1_key_field_arr[0]));
  EXPECT_EQ(indexes, t1_join_tab.const_keys)
      << "SARGable index not present in const_keys";
  EXPECT_EQ(indexes, t1_join_tab.keys());
}

TEST_F(OptRefTest, addKeyFieldsFromInOneRowWithCols) {
  // We simulate the where condition (t1.a, t1.b) IN ((t2.a, t2.b))
  PT_item_list *all_args = new (current_thd->mem_root) PT_item_list;
  all_args->push_front(make_item_row(item_field_t2_a, item_field_t2_b));
  all_args->push_front(make_item_row(item_field_t1_a, item_field_t1_b));
  Item *cond = new Item_func_in(POS(), all_args, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(cond->itemize(&pc, &cond));

  call_add_key_fields(cond);

  // We expect the key_fields pointer not to be incremented.
  EXPECT_EQ(0, t1_key_fields - static_cast<Key_field *>(&t1_key_field_arr[0]));
  EXPECT_EQ(Key_map(0), t1_join_tab.const_keys);
  EXPECT_EQ(indexes, t1_join_tab.keys());

  EXPECT_EQ(t2.pos_in_table_list->map(), t1_join_tab.key_dependent);
}

TEST_F(OptRefTest, addKeyFieldsFromEq) {
  // We simulate the where condition a = 0 AND b = 0
  Item_func_eq *eq1 = new Item_func_eq(item_field_t1_a, item_zero);
  Item_func_eq *eq2 = new Item_func_eq(item_field_t1_b, item_zero);
  Item_cond_and *cond = new Item_cond_and(eq1, eq2);

  call_add_key_fields(cond);

  /*
    We expect 2 Key_field's to be written. Actually they're always
    written, but we expect the pointer to be incremented.
  */
  EXPECT_EQ(2, t1_key_fields - static_cast<Key_field *>(&t1_key_field_arr[0]));
  EXPECT_EQ(indexes, t1_join_tab.const_keys)
      << "SARGable index not present in const_keys";
  EXPECT_EQ(indexes, t1_join_tab.keys());

  EXPECT_EQ(0U, t1_join_tab.key_dependent);

  EXPECT_EQ(0U, t1_key_field_arr[0].level);
  EXPECT_EQ(0U, t1_key_field_arr[1].level);
}

}  // namespace opt_ref_unittest
