/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include <sstream>
#include <string>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_table_map.h"
#include "sql/item_cmpfunc.h"
#include "sql/parse_tree_helpers.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/test_utils.h"

namespace item_filter_unittest {

using my_testing::Server_initializer;

/**
  Helper class to print which line a failing test was called from.
*/
class TestFailLinePrinter {
 public:
  explicit TestFailLinePrinter(int line) : m_line(line) {}
  int m_line;
};
std::ostream &operator<<(std::ostream &s, const TestFailLinePrinter &v) {
  return s << "called from line " << v.m_line;
}

class ItemFilterTest : public ::testing::Test {
 protected:
  ItemFilterTest() : rows_in_table(200) {}

  virtual void SetUp() {
    initializer.SetUp();
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_alloc,
                   thd()->variables.range_alloc_block_size, 0);
  }

  virtual void TearDown() {
    delete m_table;

    initializer.TearDown();
    free_root(&m_alloc, MYF(0));
  }

  THD *thd() { return initializer.thd(); }

  /**
    Create a table with the requested number of columns without
    creating indexes.

    @param  nbr_columns       The number of columns in the table
    @param  columns_nullable  Whether or not columns in the table can be NULL
  */
  void create_table(int nbr_columns, bool columns_nullable) {
    m_table = new Fake_TABLE(nbr_columns, columns_nullable);
    m_field = m_table->field;
    m_table_list = m_table->pos_in_table_list;
  }

  /**
    Create a table with the requested number of non-nullable columns
    without creating indexes.

    @param  nbr_columns       The number of columns in the table
  */
  void create_table(int nbr_columns) { create_table(nbr_columns, false); }

  /**
    Utility funtion used to simplify creation of func items used as
    range predicates.

    @param type            The type of range predicate operator requested
    @param fld             The field used in the range predicate
    @param val1            Value used in the range predicate (if the predicate
                           takes two or more arguments)
    @param val2            Value used in the range predicate (if the predicate
                           takes three or more arguments)

    @return Item for the specified range predicate
  */
  Item_func *create_item(Item_func::Functype type, Field *fld, int val1,
                         int val2);

// Undefined at end of this file
#define create_item_check_filter(er, op, f1, v1, v2, ut, fti) \
  do_create_item_check_filter(er, op, f1, v1, v2, ut, fti,    \
                              TestFailLinePrinter(__LINE__))
  Item *do_create_item_check_filter(const float expected_result,
                                    const Item_func::Functype type, Field *fld,
                                    const int val1, const int val2,
                                    const table_map used_tables,
                                    MY_BITMAP *fields_to_ignore,
                                    TestFailLinePrinter called_from_line) {
    SCOPED_TRACE(called_from_line);
    Item *item = create_item(type, fld, val1, val2);

    const float filter =
        item->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                   fields_to_ignore, rows_in_table);
    EXPECT_FLOAT_EQ(expected_result, filter);
    return item;
  }

#define create_anditem_check_filter(er, lst, ut, fti) \
  do_create_anditem_check_filter(er, lst, ut, fti,    \
                                 TestFailLinePrinter(__LINE__))
  Item_cond_and *do_create_anditem_check_filter(
      const float expected_result, List<Item> &lst, const table_map used_tables,
      MY_BITMAP *fields_to_ignore, TestFailLinePrinter called_from_line) {
    SCOPED_TRACE(called_from_line);

    Item_cond_and *and_item = new Item_cond_and(lst);

    Item *itm = static_cast<Item *>(and_item);
    and_item->fix_fields(thd(), &itm);

    const float filter =
        and_item->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                       fields_to_ignore, rows_in_table);
    EXPECT_FLOAT_EQ(expected_result, filter);

    return and_item;
  }

#define create_oritem_check_filter(er, lst, ut, fti) \
  do_create_oritem_check_filter(er, lst, ut, fti, TestFailLinePrinter(__LINE__))
  Item_cond_or *do_create_oritem_check_filter(
      const float expected_result, List<Item> &lst, const table_map used_tables,
      MY_BITMAP *fields_to_ignore, TestFailLinePrinter called_from_line) {
    SCOPED_TRACE(called_from_line);

    Item_cond_or *or_item = new Item_cond_or(lst);

    Item *itm = static_cast<Item *>(or_item);
    or_item->fix_fields(thd(), &itm);

    const float filter =
        or_item->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                      fields_to_ignore, rows_in_table);
    EXPECT_FLOAT_EQ(expected_result, filter);

    return or_item;
  }

#define create_initem_check_filter(er, lst, ut, fti) \
  do_create_initem_check_filter(er, lst, ut, fti, TestFailLinePrinter(__LINE__))
  Item_func_in *do_create_initem_check_filter(
      const float expected_result, List<Item> &lst, const table_map used_tables,
      MY_BITMAP *fields_to_ignore, TestFailLinePrinter called_from_line) {
    SCOPED_TRACE(called_from_line);

    PT_item_list *list = new (thd()->mem_root) PT_item_list;
    list->value = lst;
    Item_func_in *in_item = new Item_func_in(POS(), list, false);
    Parse_context pc(thd(), thd()->lex->current_select());
    EXPECT_FALSE(in_item->itemize(&pc, (Item **)&in_item));

    Item *itm = static_cast<Item *>(in_item);
    in_item->fix_fields(thd(), &itm);

    const float filter =
        in_item->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                      fields_to_ignore, rows_in_table);
    EXPECT_FLOAT_EQ(expected_result, filter);

    return in_item;
  }

  /*
    Many filtering estimates depends on the number of rows in the
    table. For example, P("col = <const>") = max(0.005, 1/rows_in_table)

    This variable determines the number of rows in the table.
  */
  double rows_in_table;
  Server_initializer initializer;
  MEM_ROOT m_alloc;

  Fake_TABLE *m_table;
  TABLE_LIST *m_table_list;
  /*
    Pointer to m_table->field. Only valid if the table was
    created by calling one of ItemFilterTest::create_table*()
   */
  Field **m_field;
};

Item_func *ItemFilterTest::create_item(Item_func::Functype type, Field *fld,
                                       int val1, int val2) {
  Item_func *result;
  switch (type) {
    case Item_func::GT_FUNC:
      result = new Item_func_gt(new Item_field(fld), new Item_int(val1));
      break;
    case Item_func::GE_FUNC:
      result = new Item_func_ge(new Item_field(fld), new Item_int(val1));
      break;
    case Item_func::LT_FUNC:
      result = new Item_func_lt(new Item_field(fld), new Item_int(val1));
      break;
    case Item_func::LE_FUNC:
      result = new Item_func_le(new Item_field(fld), new Item_int(val1));
      break;
    case Item_func::MULT_EQUAL_FUNC:
      result = new Item_equal(new Item_int(val1), new Item_field(fld));
      break;
    case Item_func::XOR_FUNC:
      result = new Item_func_xor(new Item_field(fld), new Item_int(val1));
      break;
    case Item_func::ISNULL_FUNC:
      result = new Item_func_isnull(new Item_field(fld));
      break;
    case Item_func::ISNOTNULL_FUNC:
      result = new Item_func_isnotnull(new Item_field(fld));
      break;
    case Item_func::BETWEEN: {
      Parse_context pc(thd(), thd()->lex->current_select());
      result =
          new Item_func_between(POS(), new Item_field(fld), new Item_int(val1),
                                new Item_int(val2), false);
      EXPECT_FALSE(result->itemize(&pc, (Item **)&result));
      break;
    }
    default:
      result = nullptr;
      DBUG_ASSERT(false);
      return result;
  }
  Item *itm = static_cast<Item *>(result);
  result->fix_fields(thd(), &itm);
  return result;
}

/*
  Basic single predicate filter tests
*/
TEST_F(ItemFilterTest, BasicDefaultRows) {
  create_table(1, true);

  const int unused_int = 0;
  const table_map used_tables = 0;

  MY_BITMAP no_ignore_flds;
  bitmap_init(&no_ignore_flds, nullptr, m_table->s->fields);

  // Check filtering for predicate: field0 = 10
  create_item_check_filter(COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 > 10
  create_item_check_filter(COND_FILTER_INEQUALITY, Item_func::GT_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 >= 10
  create_item_check_filter(COND_FILTER_INEQUALITY, Item_func::GE_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 < 10
  create_item_check_filter(COND_FILTER_INEQUALITY, Item_func::LT_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 <= 10
  create_item_check_filter(COND_FILTER_INEQUALITY, Item_func::LE_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 IS NULL
  create_item_check_filter(COND_FILTER_EQUALITY, Item_func::ISNULL_FUNC,
                           m_field[0], unused_int, unused_int, used_tables,
                           &no_ignore_flds);
  // Check filtering for predicate: field0 IS NOT NULL
  create_item_check_filter(1.0 - COND_FILTER_EQUALITY,
                           Item_func::ISNOTNULL_FUNC, m_field[0], unused_int,
                           unused_int, used_tables, &no_ignore_flds);
  // Check filtering for predicate: field0 BETWEEN 10 AND 12
  create_item_check_filter(COND_FILTER_BETWEEN, Item_func::BETWEEN, m_field[0],
                           10, 12, used_tables, &no_ignore_flds);

  bitmap_free(&no_ignore_flds);
}

/*
  Basic single predicate filter tests when field should be ignored
*/
TEST_F(ItemFilterTest, BasicIgnoreField) {
  create_table(2, true);

  const int unused_int = 0;
  const table_map used_tables = 0;

  // Predicates on m_field[0] should be ignored
  MY_BITMAP ignore_fld0;
  bitmap_init(&ignore_fld0, nullptr, m_table->s->fields);
  bitmap_set_bit(&ignore_fld0, m_field[0]->field_index);

  // Check filtering for predicate on ignored field: field0 = 10
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::MULT_EQUAL_FUNC,
                           m_field[0], 10, unused_int, used_tables,
                           &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 > 10
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::GT_FUNC, m_field[0],
                           10, unused_int, used_tables, &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 >= 10
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::GE_FUNC, m_field[0],
                           10, unused_int, used_tables, &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 < 10
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::LT_FUNC, m_field[0],
                           10, unused_int, used_tables, &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 <= 10
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::LE_FUNC, m_field[0],
                           10, unused_int, used_tables, &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 IS NULL
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::ISNULL_FUNC,
                           m_field[0], unused_int, unused_int, used_tables,
                           &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 IS NOT NULL
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::ISNOTNULL_FUNC,
                           m_field[0], unused_int, unused_int, used_tables,
                           &ignore_fld0);
  // Check filtering for predicate on ignored field: field0 BETWEEN 10 AND 12
  create_item_check_filter(COND_FILTER_ALLPASS, Item_func::BETWEEN, m_field[0],
                           10, 12, used_tables, &ignore_fld0);

  // Verifying that predicates on other fields are not ignored
  create_item_check_filter(COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC,
                           m_field[1], 10, unused_int, used_tables,
                           &ignore_fld0);

  bitmap_free(&ignore_fld0);
}

/*
  Basic condition test: AND of const predicates
*/
TEST_F(ItemFilterTest, BasicConstAnd) {
  create_table(3, true);

  const int unused_int = 0;
  const table_map used_tables = 0;

  // Do not ignore predicates on any fields
  MY_BITMAP ignore_flds;
  bitmap_init(&ignore_flds, nullptr, m_table->s->fields);

  // Create predicate: field0 = 10
  Item *eq_item1 = create_item_check_filter(
      COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC, m_field[0], 10,
      unused_int, used_tables, &ignore_flds);
  // Create predicate: field1 = 10
  Item *eq_item2 = create_item_check_filter(
      COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC, m_field[1], 10,
      unused_int, used_tables, &ignore_flds);

  // Create predicate: field2 < 99
  Item *lt_item = create_item_check_filter(
      COND_FILTER_INEQUALITY, Item_func::LT_FUNC, m_field[2], 99, unused_int,
      used_tables, &ignore_flds);

  List<Item> and_lst;
  and_lst.push_back(eq_item1);
  and_lst.push_back(eq_item2);

  // Calculate filtering of (field0=10 AND field1=10)
  create_anditem_check_filter(COND_FILTER_EQUALITY * COND_FILTER_EQUALITY,
                              and_lst, used_tables, &ignore_flds);

  and_lst.push_back(lt_item);

  // Calculate filtering of (field0=10 AND field1=10 AND field2<99)
  Item *and_it = create_anditem_check_filter(
      COND_FILTER_EQUALITY * COND_FILTER_EQUALITY * COND_FILTER_INEQUALITY,
      and_lst, used_tables, &ignore_flds);

  /*
    Calculate filtering of (field0=10 AND field1=10 AND field2<99)
    while ignoring predicates on field0
  */
  bitmap_set_bit(&ignore_flds, m_field[0]->field_index);

  float filter = and_it->get_filtering_effect(
      thd(), m_table_list->map(), used_tables, &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_EQUALITY * COND_FILTER_INEQUALITY, filter);

  /*
    Calculate filtering of (field0=10 AND field1=10 AND field2<99)
    while ignoring predicates on field0 and field1
  */
  bitmap_set_bit(&ignore_flds, m_field[1]->field_index);
  filter = and_it->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                        &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_INEQUALITY, filter);

  /*
    Calculate filtering of (field0=10 AND field1=10 AND field2<99)
    while ignoring predicates on field0, field1 and field2
  */
  bitmap_set_bit(&ignore_flds, m_field[2]->field_index);
  filter = and_it->get_filtering_effect(thd(), m_table_list->map(), used_tables,
                                        &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_ALLPASS, filter);

  bitmap_free(&ignore_flds);
}

/*
  Basic condition test: OR of const predicates
*/
TEST_F(ItemFilterTest, BasicConstOr) {
  create_table(3, true);

  const int unused_int = 0;
  const table_map used_tables = 0;

  // Do not ignore predicates on any fields
  MY_BITMAP ignore_flds;
  bitmap_init(&ignore_flds, nullptr, m_table->s->fields);

  // Create predicate: field0 = 10
  Item *eq_item1 = create_item_check_filter(
      COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC, m_field[0], 10,
      unused_int, used_tables, &ignore_flds);
  // Create predicate: field1 = 10
  Item *eq_item2 = create_item_check_filter(
      COND_FILTER_EQUALITY, Item_func::MULT_EQUAL_FUNC, m_field[1], 10,
      unused_int, used_tables, &ignore_flds);

  // Create predicate: field2 < 99
  Item *lt_item = create_item_check_filter(
      COND_FILTER_INEQUALITY, Item_func::LT_FUNC, m_field[2], 99, unused_int,
      used_tables, &ignore_flds);

  List<Item> or_lst;
  or_lst.push_back(eq_item1);
  or_lst.push_back(eq_item2);

  // Calculate filtering of (field0=10 OR field1=10)
  const float expected1 =
      2 * COND_FILTER_EQUALITY - (COND_FILTER_EQUALITY * COND_FILTER_EQUALITY);
  create_oritem_check_filter(expected1, or_lst, used_tables, &ignore_flds);

  or_lst.push_back(lt_item);

  // Calculate filtering of (field0=10 OR field1=10 OR field2<99)
  const float expected2 =
      expected1 + COND_FILTER_INEQUALITY - (expected1 * COND_FILTER_INEQUALITY);
  Item *or_it =
      create_oritem_check_filter(expected2, or_lst, used_tables, &ignore_flds);

  /*
    Calculate filtering of (field0=10 OR field1=10 OR field2<99) while
    ignoring predicates on m_field[0]

    A predicate on an ignored field has filtering effect 1.0 (no
    filtering). The filtering effect for "A or B" is calculated as
    "Disjunction of independent events":

    P(A or B) =  P(A) + P(B) - P(A and B)

    The result is always 1.0 if any of the ORed predicates is 1.0.

  */
  bitmap_set_bit(&ignore_flds, m_field[0]->field_index);
  const float filt_ign0 = or_it->get_filtering_effect(
      thd(), m_table_list->map(), used_tables, &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_ALLPASS, filt_ign0);

  /*
    Calculate filtering of (field0=10 OR field1=10 OR field2<99) while
    ignoring predicates on m_field[0] and m_field[1]
  */
  bitmap_set_bit(&ignore_flds, m_field[1]->field_index);
  const float filt_ign1 = or_it->get_filtering_effect(
      thd(), m_table_list->map(), used_tables, &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_ALLPASS, filt_ign1);

  /*
    Calculate filtering of (field0=10 OR field1=10 OR field2<99) while
    ignoring predicates on m_field[2]
  */
  bitmap_clear_all(&ignore_flds);
  bitmap_set_bit(&ignore_flds, m_field[2]->field_index);
  const float filt_ign2 = or_it->get_filtering_effect(
      thd(), m_table_list->map(), used_tables, &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_ALLPASS, filt_ign2);

  bitmap_free(&ignore_flds);
}

/*
  Test filtering for IN predicates
*/
TEST_F(ItemFilterTest, InPredicate) {
  create_table(1, true);

  const table_map used_tables = 0;

  // Do not ignore predicates on any fields
  MY_BITMAP ignore_flds;
  bitmap_init(&ignore_flds, nullptr, m_table->s->fields);

  // Calculate filtering effect of "col IN (1)"
  List<Item> in_lst1;
  in_lst1.push_back(new Item_field(m_field[0]));
  in_lst1.push_back(new Item_int(1));

  create_initem_check_filter(COND_FILTER_EQUALITY, in_lst1, used_tables,
                             &ignore_flds);

  // Calculate filtering effect of "col IN (1, ..., 4)"
  List<Item> in_lst2;
  in_lst2.push_back(new Item_field(m_field[0]));
  in_lst2.push_back(new Item_int(1));
  in_lst2.push_back(new Item_int(2));
  in_lst2.push_back(new Item_int(3));
  in_lst2.push_back(new Item_int(4));

  const float filter = 4 * COND_FILTER_EQUALITY;
  Item_func_in *in_it =
      create_initem_check_filter(filter, in_lst2, used_tables, &ignore_flds);

  /*
    Calculate filtering effect of "col IN (1, ..., 110)"

    "col IN (<110 values>)" would mean a filtering effect of
    110 * COND_FILTER_EQUALITY = 0.6, but the filtering effect
    of IN has an upper limit of 0.5.
  */
  List<Item> in_lst3;
  in_lst3.push_back(new Item_field(m_field[0]));
  for (int i = 1; i <= 110; i++) in_lst3.push_back(new Item_int(i));

  const float capped_filter = 0.5;
  create_initem_check_filter(capped_filter, in_lst3, used_tables, &ignore_flds);

  /*
    There is no filtering effect for "col IN (1,..,6)" if 'col' is
    ignored.
  */
  bitmap_set_bit(&ignore_flds, m_field[0]->field_index);
  const float filt_ign = in_it->get_filtering_effect(
      thd(), m_table_list->map(), used_tables, &ignore_flds, rows_in_table);
  EXPECT_FLOAT_EQ(COND_FILTER_ALLPASS, filt_ign);
  bitmap_free(&ignore_flds);
}

}  // namespace item_filter_unittest
#undef create_item_check_filter
#undef create_anditem_check_filter
#undef create_oritem_check_filter
#undef create_initem_check_filter
