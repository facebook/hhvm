/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>
#include <sstream>
#include <string>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/opt_range.cc"
#include "sql/parse_tree_helpers.h"
#include "unittest/gunit/fake_range_opt_param.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/handler-t.h"
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/test_utils.h"

namespace opt_range_unittest {

using std::string;
using std::vector;

/**
  Helper class to print which line a failing test was called from.
*/
class TestFailLinePrinter {
 public:
  explicit TestFailLinePrinter(int line) : m_line(line) {}
  int m_line;
};
static std::ostream &operator<<(std::ostream &s, const TestFailLinePrinter &v) {
  return s << "called from line " << v.m_line;
}

/**
  Keep in mind the following boolean algebra definitions and rules
  when reading the tests in this file:

  Operators:
    & (and)
    | (or)
    ! (negation)

  DeMorgans laws:
    DM1: !(X & Y) <==> !X | !Y
    DM2: !(X | Y) <==> !X & !Y

  Boolean axioms:
    A1 (associativity):    X & (Y & Z)  <==>  (X & Y) & Z
                           X | (Y | Z)  <==>  (X | Y) | Z
    A2 (commutativity):    X & Y        <==>  Y & X
                           X | Y        <==>  Y | X
    A3 (identity):         X | false    <==>  X
                           X | true     <==>  true
                           X & false    <==>  false
                           X & true     <==>  X
    A4 (distributivity):   X | (Y & Z)  <==>  (X | Y) & (X | Z)
                           X & (Y | Z)  <==>  (X & Y) | (X & Z)
    A5 (complements):      X | !X       <==>  true
                           X & !X       <==>  false
    A6 (idempotence of |): X | X        <==>  X
    A7 (idempotence of &): X & X        <==>  X

  Also note that the range optimizer follows a relaxed boolean algebra
  where the result may be bigger than boolean algebra rules dictate.
  @See get_mm_tree() for explanation.
*/

using my_testing::Server_initializer;

class OptRangeTest : public ::testing::Test {
 protected:
  OptRangeTest() : m_opt_param(nullptr) {}

  virtual void SetUp() {
    initializer.SetUp();
    init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_alloc,
                   thd()->variables.range_alloc_block_size, 0);
  }

  virtual void TearDown() {
    delete m_opt_param;

    initializer.TearDown();
    free_root(&m_alloc, MYF(0));
  }

  THD *thd() { return initializer.thd(); }

  /**
    Create a table with the requested number of fields. All fields are
    indexed.

    @param  nbr_fields     The number of fields in the table
  */
  void create_table_singlecol_idx(int nbr_fields) {
    create_table(nbr_fields);
    for (int i = 0; i < nbr_fields; i++)
      m_opt_param->add_key(m_opt_param->table->field[i]);
    m_field = m_opt_param->table->field;
  }

  /**
    Create a table with the requested number of fields without
    creating indexes.

    @param  nbr_fields     The number of fields in the table
  */
  void create_table(int nbr_fields, bool columns_nullable) {
    m_opt_param =
        new Fake_RANGE_OPT_PARAM(thd(), &m_alloc, nbr_fields, columns_nullable);
    m_field = m_opt_param->table->field;
  }

  void create_table(int nbr_fields) { create_table(nbr_fields, false); }

  /*
    The new_item_xxx are convenience functions for creating Item_func
    descendents from Field's and ints without having to wrap them in
    Item's and resolving them.
  */
  template <typename T>
  T *new_item(Field *field, int value) {
    T *item = new T(new Item_field(field), new Item_int(value));
    Item *item_base = item;
    item->fix_fields(thd(), &item_base);
    return item;
  }

  Item_func_lt *new_item_lt(Field *field, int value) {
    return new_item<Item_func_lt>(field, value);
  }

  Item_func_gt *new_item_gt(Field *field, int value) {
    return new_item<Item_func_gt>(field, value);
  }

  Item_func_equal *new_item_equal(Field *field, int value) {
    return new_item<Item_func_equal>(field, value);
  }

  Item_func_xor *new_item_xor(Field *field, int value) {
    return new_item<Item_func_xor>(field, value);
  }

  Item_cond_and *new_item_between(Field *fld, int val1, int val2) {
    return new Item_cond_and(new_item_gt(fld, val1), new_item_lt(fld, val2));
  }

  Item_cond_or *new_item_ne(Field *fld, int val1) {
    return new Item_cond_or(new_item_lt(fld, val1), new_item_gt(fld, val1));
  }

  /**
    Utility funtion used to simplify creation of SEL_TREEs with
    specified range predicate operators and values. Also verifies that
    the created SEL_TREE has the expected range conditions.

    @param type            The type of range predicate operator requested
    @param fld             The field used in the range predicate
    @param val1            The first value used in the range predicate
    @param val2            The second value used in the range predicate.
                           Only used for range predicates that takes two
                           values (BETWEEN).
    @param expected_result The range conditions the created SEL_TREE
                           is expected to consist of. The format of this
                           string is what opt_range.cc print_tree() produces.

    @return SEL_TREE that has been verified to have expected range conditions.
  */
// Undefined at end of this file
#define create_tree(i, er) do_create_tree(i, er, TestFailLinePrinter(__LINE__))
  SEL_TREE *do_create_tree(Item *item, const char *expected_result,
                           TestFailLinePrinter called_from_line) {
    SEL_TREE *result = get_mm_tree(m_opt_param, item);
    SCOPED_TRACE(called_from_line);
    check_tree_result(result, SEL_TREE::KEY, expected_result);
    return result;
  }

  /**
    Utility funtion used to simplify creation of func items used as
    range predicates.

    @param type            The type of range predicate operator requested
    @param fld             The field used in the range predicate
    @param value           The value used in the range predicate

    @return Item for the specified range predicate
  */
  Item_func *create_item(Item_func::Functype type, Field *fld, int value);

  /**
    Create instance of Xor Item_func.

    @param    item1     first item for xor condition.
    @param    item2     second item for xor condition.

    @return pointer to newly created instance of Xor Item.
  */
  Item_func_xor *create_xor_item(Item *item1, Item *item2);

  /**
    Check that the use_count of all SEL_ARGs in the SEL_TREE are
    correct.

    @param   tree   The SEL_TREE to check
  */
  void check_use_count(SEL_TREE *tree);
  /**
    Verify that a SEL_TREE has the type and conditions we expect it to
    have.

    @param   tree            The SEL_TREE to check
    @param   expected_type   The type 'tree' is expected to have
    @param   expected_result The range conditions 'tree' is expected
                             to consist of. The format of this string
                             is what opt_range.cc print_tree() produces.
  */
  void check_tree_result(SEL_TREE *tree, const SEL_TREE::Type expected_type,
                         const char *expected_result);
  /**
    Perform OR between two SEL_TREEs and verify that the result of the
    OR operation is as expected.

    @param   tree1           First SEL_TREE that will be ORed
    @param   tree2           Second SEL_TREE that will be ORed
    @param   expected_type   The type the ORed result is expected to have
    @param   expected_result The range conditions the ORed result is expected
                             to consist of. The format of this string
                             is what opt_range.cc print_tree() produces.

    @return SEL_TREE result of the OR operation between tree1 and
            tree2 that has been verified to have expected range
            conditions.
  */
// Undefined at end of this file
#define create_and_check_tree_or(t1, t2, et, er) \
  do_create_and_check_tree_or(t1, t2, et, er, TestFailLinePrinter(__LINE__))
  SEL_TREE *do_create_and_check_tree_or(SEL_TREE *tree1, SEL_TREE *tree2,
                                        const SEL_TREE::Type expected_type,
                                        const char *expected_result,
                                        TestFailLinePrinter called_from_line);
  /**
    Perform AND between two SEL_TREEs and verify that the result of the
    AND operation is as expected.

    @param   tree1           First SEL_TREE that will be ANDed
    @param   tree2           Second SEL_TREE that will be ANDed
    @param   expected_type   The type the ANDed result is expected to have
    @param   expected_result The range conditions the ANDed result is expected
                             to consist of. The format of this string
                             is what opt_range.cc print_tree() produces.

    @return SEL_TREE result of the AND operation between tree1 and
            tree2 that has been verified to have expected range
            conditions.
  */
// Undefined at end of this file
#define create_and_check_tree_and(t1, t2, et, er) \
  do_create_and_check_tree_and(t1, t2, et, er, TestFailLinePrinter(__LINE__))
  SEL_TREE *do_create_and_check_tree_and(SEL_TREE *tree1, SEL_TREE *tree2,
                                         const SEL_TREE::Type expected_type,
                                         const char *expected_result,
                                         TestFailLinePrinter called_from_line);

  Server_initializer initializer;
  MEM_ROOT m_alloc;

  Fake_RANGE_OPT_PARAM *m_opt_param;
  /*
    Pointer to m_opt_param->table->field. Only valid if the table was
    created by calling one of OptRangeTest::create_table*()
   */
  Field **m_field;
};

Item_func *OptRangeTest::create_item(Item_func::Functype type, Field *fld,
                                     int value) {
  Item_func *result;
  switch (type) {
    case Item_func::GT_FUNC:
      result = new Item_func_gt(new Item_field(fld), new Item_int(value));
      break;
    case Item_func::LT_FUNC:
      result = new Item_func_lt(new Item_field(fld), new Item_int(value));
      break;
    case Item_func::MULT_EQUAL_FUNC:
      result = new Item_equal(new Item_int(value), new Item_field(fld));
      break;
    case Item_func::XOR_FUNC:
      result = new Item_func_xor(new Item_field(fld), new Item_int(value));
      break;
    default:
      result = nullptr;
      DBUG_ASSERT(false);
      return result;
  }
  Item *itm = static_cast<Item *>(result);
  result->fix_fields(thd(), &itm);
  return result;
}

Item_func_xor *OptRangeTest::create_xor_item(Item *item1, Item *item2) {
  Item_func_xor *xor_item = new Item_func_xor(item1, item2);
  Item *itm = static_cast<Item *>(xor_item);
  xor_item->fix_fields(thd(), &itm);
  return xor_item;
}

void OptRangeTest::check_use_count(SEL_TREE *tree) {
  for (uint i = 0; i < m_opt_param->keys; i++) {
    SEL_ROOT *cur_range = tree->keys[i];
    if (cur_range != nullptr) {
      EXPECT_FALSE(cur_range->test_use_count(cur_range));
    }
  }

  if (!tree->merges.is_empty()) {
    List_iterator<SEL_IMERGE> it(tree->merges);
    SEL_IMERGE *merge = it++;

    for (SEL_TREE **current = merge->trees; current != merge->trees_next;
         current++)
      check_use_count(*current);
  }
}

void OptRangeTest::check_tree_result(SEL_TREE *tree,
                                     const SEL_TREE::Type expected_type,
                                     const char *expected_result) {
  EXPECT_EQ(expected_type, tree->type);
  if (expected_type != SEL_TREE::KEY) return;

  char buff[512];
  String actual_result(buff, sizeof(buff), system_charset_info);
  actual_result.set_charset(system_charset_info);
  actual_result.length(0);
  print_tree(&actual_result, "result", tree, m_opt_param, false);
  EXPECT_STREQ(expected_result, actual_result.c_ptr());
  SCOPED_TRACE("check_use_count");
  check_use_count(tree);
}

SEL_TREE *OptRangeTest::do_create_and_check_tree_or(
    SEL_TREE *tree1, SEL_TREE *tree2, const SEL_TREE::Type expected_type,
    const char *expected_result, TestFailLinePrinter called_from_line) {
  {
    // Check that tree use counts are OK before OR'ing
    SCOPED_TRACE(called_from_line);
    check_use_count(tree1);
    check_use_count(tree2);
  }

  SEL_TREE *result = tree_or(m_opt_param, tree1, tree2);

  // Tree returned from tree_or()
  SCOPED_TRACE(called_from_line);
  check_tree_result(result, expected_type, expected_result);

  return result;
}

SEL_TREE *OptRangeTest::do_create_and_check_tree_and(
    SEL_TREE *tree1, SEL_TREE *tree2, const SEL_TREE::Type expected_type,
    const char *expected_result, TestFailLinePrinter called_from_line) {
  {
    // Check that tree use counts are OK before AND'ing
    SCOPED_TRACE(called_from_line);
    check_use_count(tree1);
    check_use_count(tree2);
  }

  SEL_TREE *result = tree_and(m_opt_param, tree1, tree2);

  // Tree returned from tree_and()
  SCOPED_TRACE(called_from_line);
  check_tree_result(result, expected_type, expected_result);

  return result;
}

/*
 Experiment with these to measure performance of
   'new (thd->mem_root)' Foo vs. 'new Foo'.
 With gcc 4.4.2 I see ~4% difference (in optimized mode).
*/
const int num_iterations = 10;
const int num_allocs = 10;

TEST_F(OptRangeTest, AllocateExplicit) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    free_root(thd()->mem_root, MYF(MY_KEEP_PREALLOC));
    for (int ii = 0; ii < num_allocs; ++ii) new (thd()->mem_root) SEL_ARG;
  }
}

TEST_F(OptRangeTest, AllocateImplicit) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    free_root(thd()->mem_root, MYF(MY_KEEP_PREALLOC));
    for (int ii = 0; ii < num_allocs; ++ii) new (thd()->mem_root) SEL_ARG;
  }
}

/*
  We cannot do EXPECT_NE(NULL, get_mm_tree(...))
  because of limits in google test.
 */
const SEL_TREE *null_tree = nullptr;
const SEL_ROOT *null_root = nullptr;
const SEL_ARG *null_arg = nullptr;

static void print_selarg_ranges(String *s, SEL_ARG *sel_arg,
                                const KEY_PART_INFO *kpi) {
  for (SEL_ARG *cur = sel_arg->first(); cur != null_element; cur = cur->right) {
    String current_range;
    append_range(&current_range, kpi, cur->min_value, cur->max_value,
                 cur->min_flag | cur->max_flag);

    if (s->length() > 0) s->append(STRING_WITH_LEN("\n"));

    s->append(current_range);
  }
}

TEST_F(OptRangeTest, SimpleCond) {
  Fake_RANGE_OPT_PARAM opt_param(thd(), &m_alloc, 0, false);
  EXPECT_NE(null_tree, get_mm_tree(&opt_param, new Item_int(42)));
}

/*
  Exercise range optimizer without adding indexes
*/
TEST_F(OptRangeTest, EqualCondNoIndexes) {
  Fake_RANGE_OPT_PARAM opt_param(thd(), &m_alloc, 1, false);
  SEL_TREE *tree =
      get_mm_tree(&opt_param, new_item_equal(opt_param.table->field[0], 42));
  EXPECT_EQ(null_tree, tree);
}

/*
  Exercise range optimizer with xor operator.
*/
TEST_F(OptRangeTest, XorCondIndexes) {
  create_table(1);

  m_opt_param->add_key(m_field[0]);
  /*
    XOR is not range optimizible ATM and is treated as
    always true. No SEL_TREE is therefore expected.
  */
  SEL_TREE *tree = get_mm_tree(m_opt_param, new_item_xor(m_field[0], 42));
  EXPECT_EQ(null_tree, tree);
}

/*
  Exercise range optimizer with xor and different type of operator.
*/
TEST_F(OptRangeTest, XorCondWithIndexes) {
  create_table(5);

  m_opt_param->add_key(m_field[0]);
  m_opt_param->add_key(m_field[1]);
  m_opt_param->add_key(m_field[2]);
  m_opt_param->add_key(m_field[3]);
  m_opt_param->add_key(m_field[4]);

  /*
    Create SEL_TREE from "field1=7 AND (field1 XOR 42)". Since XOR is
    not range optimizible (treated as always true), we get a tree for
    "field1=7" only.
  */
  const char expected1[] = "result keys[0]: (7 <= field_1 <= 7)\n";

  SEL_TREE *tree = get_mm_tree(
      m_opt_param, new Item_cond_and(new_item_xor(m_field[0], 42),
                                     new_item_equal(m_field[0], 7)));
  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected1);

  /*
    Create SEL_TREE from "(field1 XOR 0) AND (field1>14)". Since XOR
    is not range optimizible (treated as always true), we get a tree
    for "field1>14" only.
  */
  const char expected2[] = "result keys[0]: (14 < field_1)\n";

  tree =
      get_mm_tree(m_opt_param, new Item_cond_and(new_item_xor(m_field[0], 0),
                                                 new_item_gt(m_field[0], 14)));
  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected2);

  /*
    Create SEL_TREE from "(field1<0 AND field1>14) XOR
    (field1>17)". Since XOR is not range optimizible (treated as
    always true), we get a NULL tree.
  */
  tree = get_mm_tree(
      m_opt_param,
      create_xor_item(new Item_cond_and(new_item_lt(m_field[0], 0),
                                        new_item_gt(m_field[0], 14)),
                      new_item_gt(m_field[0], 17)));
  SCOPED_TRACE("");
  EXPECT_EQ(null_tree, tree);

  /*
    Create SEL_TREE from
    (field1<0 AND field2>14) AND
    ((field3<0 and field4>14) XOR field5>17) ".
    Since XOR is not range optimizible (treated as always true),
    we get a tree for "field1<0 AND field2>14" only.
  */
  const char expected3[] =
      "result keys[0]: (field_1 < 0)\n"
      "result keys[1]: (14 < field_2)\n";

  tree = get_mm_tree(
      m_opt_param,
      new Item_cond_and(
          new Item_cond_and(new_item_lt(m_field[0], 0),
                            new_item_gt(m_field[1], 14)),
          create_xor_item(new Item_cond_and(new_item_lt(m_field[2], 0),
                                            new_item_gt(m_field[3], 14)),
                          new_item_gt(m_field[4], 17))));
  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected3);
}

/*
  Exercise range optimizer with single column index
*/
TEST_F(OptRangeTest, GetMMTreeSingleColIndex) {
  // Create a single-column table with index
  create_table_singlecol_idx(1);

  // Expected result of next test:
  const char expected[] = "result keys[0]: (42 <= field_1 <= 42)\n";
  create_tree(new_item_equal(m_field[0], 42), expected);

  // Expected result of next test:
  const char expected2[] =
      "result keys[0]: (42 <= field_1 <= 42) OR (43 <= field_1 <= 43)\n";
  SEL_TREE *tree = get_mm_tree(
      m_opt_param, new Item_cond_or(new_item_equal(m_field[0], 42),
                                    new_item_equal(m_field[0], 43)));

  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected2);

  // Expected result of next test:
  const char expected3[] =
      "result keys[0]: "
      "(1 <= field_1 <= 1) OR (2 <= field_1 <= 2) OR "
      "(3 <= field_1 <= 3) OR (4 <= field_1 <= 4) OR "
      "(5 <= field_1 <= 5) OR (6 <= field_1 <= 6) OR "
      "(7 <= field_1 <= 7) OR (8 <= field_1 <= 8)\n";
  List<Item> or_list1;
  or_list1.push_back(new_item_equal(m_field[0], 1));
  or_list1.push_back(new_item_equal(m_field[0], 2));
  or_list1.push_back(new_item_equal(m_field[0], 3));
  or_list1.push_back(new_item_equal(m_field[0], 4));
  or_list1.push_back(new_item_equal(m_field[0], 5));
  or_list1.push_back(new_item_equal(m_field[0], 6));
  or_list1.push_back(new_item_equal(m_field[0], 7));
  or_list1.push_back(new_item_equal(m_field[0], 8));

  tree = get_mm_tree(m_opt_param, new Item_cond_or(or_list1));
  check_tree_result(tree, SEL_TREE::KEY, expected3);

  // Expected result of next test:
  const char expected4[] = "result keys[0]: (7 <= field_1 <= 7)\n";
  tree = get_mm_tree(m_opt_param,
                     new Item_cond_and(new Item_cond_or(or_list1),
                                       new_item_equal(m_field[0], 7)));
  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected4);

  // Expected result of next test:
  const char expected5[] =
      "result keys[0]: "
      "(1 <= field_1 <= 1) OR (3 <= field_1 <= 3) OR "
      "(5 <= field_1 <= 5) OR (7 <= field_1 <= 7)\n";
  List<Item> or_list2;
  or_list2.push_back(new_item_equal(m_field[0], 1));
  or_list2.push_back(new_item_equal(m_field[0], 3));
  or_list2.push_back(new_item_equal(m_field[0], 5));
  or_list2.push_back(new_item_equal(m_field[0], 7));
  or_list2.push_back(new_item_equal(m_field[0], 9));

  tree =
      get_mm_tree(m_opt_param, new Item_cond_and(new Item_cond_or(or_list1),
                                                 new Item_cond_or(or_list2)));
  SCOPED_TRACE("");
  check_tree_result(tree, SEL_TREE::KEY, expected5);
}

/*
  Exercise range optimizer with multiple column index
*/
TEST_F(OptRangeTest, GetMMTreeMultipleSingleColIndex) {
  // Create a single-column table without index
  create_table(1);

  // Add two indexes covering the same field
  m_opt_param->add_key(m_field[0]);
  m_opt_param->add_key(m_field[0]);

  char buff[512];
  String range_string(buff, sizeof(buff), system_charset_info);
  range_string.set_charset(system_charset_info);

  // Expected result of next test:
  const char expected[] =
      "result keys[0]: (42 <= field_1 <= 42)\n"
      "result keys[1]: (42 <= field_1 <= 42)\n";
  create_tree(new_item_equal(m_field[0], 42), expected);
}

/*
  Exercise range optimizer with multiple single column indexes
*/
TEST_F(OptRangeTest, GetMMTreeOneTwoColIndex) {
  create_table(2);

  m_opt_param->add_key(m_field[0], m_field[1]);

  char buff[512];
  String range_string(buff, sizeof(buff), system_charset_info);
  range_string.set_charset(system_charset_info);

  // Expected result of next test:
  const char expected[] = "result keys[0]: (42 <= field_1 <= 42)\n";
  create_tree(new_item_equal(m_field[0], 42), expected);

  // Expected result of next test:
  const char expected2[] =
      "result keys[0]: (42 <= field_1 <= 42 AND 10 <= field_2 <= 10)\n";
  SEL_TREE *tree = get_mm_tree(
      m_opt_param, new Item_cond_and(new_item_equal(m_field[0], 42),
                                     new_item_equal(m_field[1], 10)));

  range_string.length(0);
  print_tree(&range_string, "result", tree, m_opt_param, false);
  EXPECT_STREQ(expected2, range_string.c_ptr());
}

/*
  Optimizer tracing should only print ranges for applicable keyparts,
  except when argument for print_tree() parameter 'print_full' is true.
*/
TEST_F(OptRangeTest, GetMMTreeNonApplicableKeypart) {
  create_table(3);

  List<Field> index_list;
  index_list.push_back(m_field[0]);
  index_list.push_back(m_field[1]);
  index_list.push_back(m_field[2]);
  m_opt_param->add_key(index_list);

  char buff[512];
  String range_string(buff, sizeof(buff), system_charset_info);
  range_string.set_charset(system_charset_info);

  /*
    Expected result is range only on first keypart. Third keypart is
    not applicable because there are no predicates on the second
    keypart.
  */
  const char expected1[] = "result keys[0]: (42 <= field_1 <= 42)\n";
  SEL_TREE *tree = get_mm_tree(
      m_opt_param, new Item_cond_and(new_item_equal(m_field[0], 42),
                                     new_item_equal(m_field[2], 10)));
  range_string.length(0);
  print_tree(&range_string, "result", tree, m_opt_param, false);
  EXPECT_STREQ(expected1, range_string.c_ptr());

  /*
    Same SEL_ARG tree, but print_full argument is now true.
    Non-applicable key parts are also printed in this case.
  */
  const char expected1_printfull[] =
      "result keys[0]: (42 <= field_1 <= 42 AND 10 <= field_3 <= 10)\n";

  range_string.length(0);
  print_tree(&range_string, "result", tree, m_opt_param, true);
  EXPECT_STREQ(expected1_printfull, range_string.c_ptr());

  /*
    Expected result is range only on first keypart. Second keypart is
    not applicable because the predicate on the first keypart does not
    use an equality operator.
  */
  const char expected2[] = "result keys[0]: (field_1 < 42)\n";

  tree = get_mm_tree(m_opt_param,
                     new Item_cond_and(new_item_lt(m_field[0], 42),
                                       new_item_equal(m_field[1], 10)));

  range_string.length(0);
  print_tree(&range_string, "result", tree, m_opt_param, false);
  EXPECT_STREQ(expected2, range_string.c_ptr());

  /*
    Same SEL_ARG tree, but print_full argument is now true.
    Non-applicable key parts are also printed in this case.
  */
  const char expected2_printfull[] =
      "result keys[0]: (field_1 < 42 AND 10 <= field_2 <= 10)\n";
  range_string.length(0);
  print_tree(&range_string, "result", tree, m_opt_param, true);
  EXPECT_STREQ(expected2_printfull, range_string.c_ptr());
}

/*
  Exercise range optimizer with three single column indexes
*/
TEST_F(OptRangeTest, treeAndSingleColIndex1) {
  create_table_singlecol_idx(3);

  // Expected outputs
  // Single-field range predicates
  const char expected_fld1[] = "result keys[0]: (10 < field_1 < 13)\n";
  const char expected_fld2_1[] = "result keys[1]: (field_2 < 11)\n";
  const char expected_fld2_2[] = "result keys[1]: (8 < field_2)\n";
  const char expected_fld3[] = "result keys[2]: (20 < field_3 < 30)\n";

  /*
    Expected result when performing AND of:
      "(field_1 BETWEEN 10 AND 13) & (field_2 < 11)"
  */
  const char expected_and1[] =
      "result keys[0]: (10 < field_1 < 13)\n"
      "result keys[1]: (field_2 < 11)\n";

  /*
    Expected result when performing AND of:
      "((field_1 BETWEEN 10 AND 13) & (field_2 < 11))
       &
       (field_3 BETWEEN 20 AND 30)"
  */
  const char expected_and2[] =
      "result keys[0]: (10 < field_1 < 13)\n"
      "result keys[1]: (field_2 < 11)\n"
      "result keys[2]: (20 < field_3 < 30)\n";

  /*
    Expected result when performing AND of:
      "((field_1 BETWEEN 10 AND 13) &
        (field_2 < 11) &
        (field_3 BETWEEN 20 AND 30)
       )
       &
       field_2 > 8"
  */
  const char expected_and3[] =
      "result keys[0]: (10 < field_1 < 13)\n"
      "result keys[1]: (8 < field_2 < 11)\n"  // <- notice lower bound
      "result keys[2]: (20 < field_3 < 30)\n";

  SEL_TREE *tree_and = create_and_check_tree_and(
      create_and_check_tree_and(
          create_tree(new_item_between(m_field[0], 10, 13), expected_fld1),
          create_tree(new_item_lt(m_field[1], 11), expected_fld2_1),
          SEL_TREE::KEY, expected_and1),
      create_tree(new_item_between(m_field[2], 20, 30), expected_fld3),
      SEL_TREE::KEY, expected_and2);

  /*
    Testing Axiom 7: AND'ing a predicate already part of a SEL_TREE
    has no effect.
  */
  create_and_check_tree_and(
      tree_and,
      create_tree(new_item_between(m_field[2], 20, 30), expected_fld3),
      SEL_TREE::KEY, expected_and2  // conditions did not change
  );

  create_and_check_tree_and(
      tree_and, create_tree(new_item_gt(m_field[1], 8), expected_fld2_2),
      SEL_TREE::KEY, expected_and3);
}

/*
  Exercise range optimizer with three single column indexes
*/
TEST_F(OptRangeTest, treeOrSingleColIndex1) {
  create_table_singlecol_idx(3);

  // Expected outputs
  // Single-field range predicates
  const char expected_fld1[] = "result keys[0]: (10 < field_1 < 13)\n";
  const char expected_fld2_1[] = "result keys[1]: (field_2 < 11)\n";
  const char expected_fld2_2[] = "result keys[1]: (8 < field_2)\n";
  const char expected_fld3[] = "result keys[2]: (20 < field_3 < 30)\n";

  /*
    Expected result when performing OR of:
      "(field_1 Item_func::BETWEEN 10 AND 13) | (field_2 < 11)"
  */
  const char expected_or1[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 < field_1 < 13)\n"
      "  merge_tree keys[1]: (field_2 < 11)\n";

  /*
    Expected result when performing OR of:
      "((field_1 BETWEEN 10 AND 13) | (field_2 < 11))
       |
       (field_3 BETWEEN 20 AND 30)"
  */
  const char expected_or2[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 < field_1 < 13)\n"
      "  merge_tree keys[1]: (field_2 < 11)\n"
      "  merge_tree keys[2]: (20 < field_3 < 30)\n";

  SEL_TREE *tree_or2 = create_and_check_tree_or(
      create_and_check_tree_or(
          create_tree(new_item_between(m_field[0], 10, 13), expected_fld1),
          create_tree(new_item_lt(m_field[1], 11), expected_fld2_1),
          SEL_TREE::KEY, expected_or1),
      create_tree(new_item_between(m_field[2], 20, 30), expected_fld3),
      SEL_TREE::KEY, expected_or2);

  /*
    Testing Axiom 6: OR'ing a predicate already part of a SEL_TREE
    has no effect.
  */
  SEL_TREE *tree_or3 = create_and_check_tree_or(
      tree_or2,
      create_tree(new_item_between(m_field[2], 20, 30), expected_fld3),
      SEL_TREE::KEY, expected_or2);

  /*
    Perform OR of:
      "((field_1 BETWEEN 10 AND 13) |
        (field_2 < 11) |
        (field_3 BETWEEN 20 AND 30)
       ) |
       (field_2 > 8)"

    This is always TRUE due to
       (field_2 < 11) | (field_2 > 8)  <==> true
  */
  create_and_check_tree_or(
      tree_or3, create_tree(new_item_gt(m_field[1], 8), expected_fld2_2),
      SEL_TREE::ALWAYS, "");
}

/*
  Exercise range optimizer with three single column indexes
*/
TEST_F(OptRangeTest, treeAndOrComboSingleColIndex1) {
  create_table_singlecol_idx(3);

  // Expected outputs
  // Single-field range predicates
  const char exected_fld1[] = "result keys[0]: (10 < field_1 < 13)\n";
  const char exected_fld2[] = "result keys[1]: (field_2 < 11)\n";
  const char exected_fld3[] = "result keys[2]: (20 < field_3 < 30)\n";

  // What "exected_fld1 & exected_fld2" should produce
  const char expected_and[] =
      "result keys[0]: (10 < field_1 < 13)\n"
      "result keys[1]: (field_2 < 11)\n";

  /*
    What "(exected_fld1 & exected_fld2) | exected_fld3" should
    produce.

    By Axiom 4 (see above), we have that
       X | (Y & Z)  <==>  (X | Y) & (X | Z)

    Thus:

       ((field_1 BETWEEN 10 AND 13) & field_2 < 11) |
       (field_3 BETWEEN 20 AND 30)

         <==> (Axiom 4)

       (field_1 BETWEEN ... | field_3 BETWEEN ...) &
       (field_2 < ...       | field_3 BETWEEN ...)

    But the result above is not created. Instead the following, which
    is incorrect (reads more rows than necessary), is the result:

       (field_1 BETWEEN ... | field_2 < 11 | field_3 BETWEEN ...)
  */
  const char expected_incorrect_or[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 < field_1 < 13)\n"
      "  merge_tree keys[1]: (field_2 < 11)\n"
      "  merge_tree keys[2]: (20 < field_3 < 30)\n";

  create_and_check_tree_or(
      create_and_check_tree_and(
          create_tree(new_item_between(m_field[0], 10, 13), exected_fld1),
          create_tree(new_item_lt(m_field[1], 11), exected_fld2), SEL_TREE::KEY,
          expected_and),
      create_tree(new_item_between(m_field[2], 20, 30), exected_fld3),
      SEL_TREE::KEY, expected_incorrect_or);
}

/**
  Test for BUG#16164031
*/
TEST_F(OptRangeTest, treeAndOrComboSingleColIndex2) {
  create_table_singlecol_idx(3);

  // Single-index predicates
  const char exp_f2_eq1[] = "result keys[1]: (1 <= field_2 <= 1)\n";
  const char exp_f2_eq2[] = "result keys[1]: (2 <= field_2 <= 2)\n";
  const char exp_f3_eq[] = "result keys[2]: (1 <= field_3 <= 1)\n";
  const char exp_f1_lt1[] = "result keys[0]: (field_1 < 256)\n";

  // OR1: Result of OR'ing f2_eq with f3_eq
  const char exp_or1[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (1 <= field_2 <= 1)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n";

  // OR2: Result of OR'ing f1_lt with f2_eq
  const char exp_or2[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (field_1 < 256)\n"
      "  merge_tree keys[1]: (2 <= field_2 <= 2)\n";

  // AND1: Result of "OR1 & OR2"
  const char exp_and1[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (1 <= field_2 <= 1)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (field_1 < 256)\n"
      "  merge_tree keys[1]: (2 <= field_2 <= 2)\n";

  SEL_TREE *tree_and1 = create_and_check_tree_and(
      create_and_check_tree_or(
          create_tree(new_item_equal(m_field[1], 1), exp_f2_eq1),
          create_tree(new_item_equal(m_field[2], 1), exp_f3_eq), SEL_TREE::KEY,
          exp_or1),
      create_and_check_tree_or(
          create_tree(new_item_lt(m_field[0], 256), exp_f1_lt1),
          create_tree(new_item_equal(m_field[1], 2), exp_f2_eq2), SEL_TREE::KEY,
          exp_or2),
      SEL_TREE::KEY, exp_and1);

  // OR3: Result of "AND1 | field3 = 1"
  const char exp_or3[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (1 <= field_2 <= 1)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (field_1 < 256)\n"
      "  merge_tree keys[1]: (2 <= field_2 <= 2)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n";

  SEL_TREE *tree_or3 = create_and_check_tree_or(
      tree_and1, create_tree(new_item_equal(m_field[2], 1), exp_f3_eq),
      SEL_TREE::KEY, exp_or3);

  // More single-index predicates
  const char exp_f1_lt2[] = "result keys[0]: (field_1 < 35)\n";
  const char exp_f1_gt2[] = "result keys[0]: (257 < field_1)\n";
  const char exp_f1_or[] =
      "result keys[0]: (field_1 < 35) OR (257 < field_1)\n";

  // OR4: Result of "OR3 | exp_f1_or"
  const char exp_or4[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (1 <= field_2 <= 1)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n"
      "  merge_tree keys[0]: (field_1 < 35) OR (257 < field_1)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (field_1 < 256) OR (257 < field_1)\n"
      "  merge_tree keys[1]: (2 <= field_2 <= 2)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n";

  SEL_TREE *tree_or4 = create_and_check_tree_or(
      tree_or3,
      create_and_check_tree_or(
          create_tree(new_item_lt(m_field[0], 35), exp_f1_lt2),
          create_tree(new_item_gt(m_field[0], 257), exp_f1_gt2), SEL_TREE::KEY,
          exp_f1_or),
      SEL_TREE::KEY, exp_or4);

  // More single-index predicates
  const char exp_f1_neq[] =
      "result keys[0]: (field_1 < 255) OR (255 < field_1)\n";
  const char exp_f2_eq3[] = "result keys[1]: (3 <= field_2 <= 3)\n";

  // AND2: Result of ANDing these two ^
  const char exp_and2[] =
      "result keys[0]: (field_1 < 255) OR (255 < field_1)\n"
      "result keys[1]: (3 <= field_2 <= 3)\n";

  // OR5: Result of "OR4 | AND3"
  /*
    "(field_1 < 255) OR (255 < field_1)" is lost when performing this
    OR. This results in a bigger set than correct boolean algebra
    rules dictate. @See note about relaxed boolean algebra in
    get_mm_tree().
  */
  const char exp_or5[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (1 <= field_2 <= 1) OR (3 <= field_2 <= 3)\n"
      "  merge_tree keys[2]: (1 <= field_3 <= 1)\n"
      "  merge_tree keys[0]: (field_1 < 35) OR (257 < field_1)\n";

  create_and_check_tree_or(
      tree_or4,
      create_and_check_tree_and(
          create_tree(new_item_ne(m_field[0], 255), exp_f1_neq),
          create_tree(new_item_equal(m_field[1], 3), exp_f2_eq3), SEL_TREE::KEY,
          exp_and2),
      SEL_TREE::KEY, exp_or5);
}

/**
  Test for BUG#16241773
*/
TEST_F(OptRangeTest, treeAndOrComboSingleColIndex3) {
  create_table_singlecol_idx(2);

  // Single-index predicates
  const char exp_f1_eq10[] = "result keys[0]: (10 <= field_1 <= 10)\n";
  const char exp_f2_gtr20[] = "result keys[1]: (20 < field_2)\n";

  const char exp_f1_eq11[] = "result keys[0]: (11 <= field_1 <= 11)\n";
  const char exp_f2_gtr10[] = "result keys[1]: (10 < field_2)\n";

  // OR1: Result of ORing f1_eq10 and f2_gtr20
  const char exp_or1[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 <= field_1 <= 10)\n"
      "  merge_tree keys[1]: (20 < field_2)\n";

  // OR2: Result of ORing f1_eq11 and f2_gtr10
  const char exp_or2[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (11 <= field_1 <= 11)\n"
      "  merge_tree keys[1]: (10 < field_2)\n";

  // AND1: Result of ANDing OR1 and OR2
  const char exp_and1[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 <= field_1 <= 10)\n"
      "  merge_tree keys[1]: (20 < field_2)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (11 <= field_1 <= 11)\n"
      "  merge_tree keys[1]: (10 < field_2)\n";

  SEL_TREE *tree_and1 = create_and_check_tree_and(
      create_and_check_tree_or(
          create_tree(new_item_equal(m_field[0], 10), exp_f1_eq10),
          create_tree(new_item_gt(m_field[1], 20), exp_f2_gtr20), SEL_TREE::KEY,
          exp_or1),
      create_and_check_tree_or(
          create_tree(new_item_equal(m_field[0], 11), exp_f1_eq11),
          create_tree(new_item_gt(m_field[1], 10), exp_f2_gtr10), SEL_TREE::KEY,
          exp_or2),
      SEL_TREE::KEY, exp_and1);

  const char exp_f2_eq5[] = "result keys[1]: (5 <= field_2 <= 5)\n";
  // OR3: Result of OR'ing AND1 with f2_eq5
  const char exp_or3[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 <= field_1 <= 10)\n"
      "  merge_tree keys[1]: (5 <= field_2 <= 5) OR (20 < field_2)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (11 <= field_1 <= 11)\n"
      "  merge_tree keys[1]: (5 <= field_2 <= 5) OR (10 < field_2)\n";
  SEL_TREE *tree_or3 = create_and_check_tree_or(
      tree_and1, create_tree(new_item_equal(m_field[1], 5), exp_f2_eq5),
      SEL_TREE::KEY, exp_or3);

  const char exp_f2_lt2[] = "result keys[1]: (field_2 < 2)\n";
  // OR4: Result of OR'ing OR3 with f2_lt2
  const char exp_or4[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[0]: (10 <= field_1 <= 10)\n"
      "  merge_tree keys[1]: (field_2 < 2) OR "
      "(5 <= field_2 <= 5) OR (20 < field_2)\n\n"
      "--- alternative 2 ---\n"
      "  merge_tree keys[0]: (11 <= field_1 <= 11)\n"
      "  merge_tree keys[1]: (field_2 < 2) OR "
      "(5 <= field_2 <= 5) OR (10 < field_2)\n";

  create_and_check_tree_or(tree_or3,
                           create_tree(new_item_lt(m_field[1], 2), exp_f2_lt2),
                           SEL_TREE::KEY, exp_or4);
}

/*
  Create SelArg with various single valued predicate
*/
TEST_F(OptRangeTest, SelArgOnevalue) {
  Fake_TABLE fake_table({7}, false);
  Field *field_long7 = fake_table.field[0];

  KEY_PART_INFO kpi;
  kpi.init_from_field(field_long7);

  uchar range_val7[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long7->get_key_image(range_val7, kpi.length, Field::itRAW);

  SEL_ARG sel_arg7(field_long7, range_val7, range_val7, true);
  String range_string;
  print_selarg_ranges(&range_string, &sel_arg7, &kpi);
  const char expected[] = "7 <= field_1 <= 7";
  EXPECT_STREQ(expected, range_string.c_ptr());

  sel_arg7.min_flag |= NO_MIN_RANGE;
  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg7, &kpi);
  const char expected2[] = "field_1 <= 7";
  EXPECT_STREQ(expected2, range_string.c_ptr());

  sel_arg7.max_flag = NEAR_MAX;
  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg7, &kpi);
  const char expected3[] = "field_1 < 7";
  EXPECT_STREQ(expected3, range_string.c_ptr());

  sel_arg7.min_flag = NEAR_MIN;
  sel_arg7.max_flag = NO_MAX_RANGE;
  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg7, &kpi);
  const char expected4[] = "7 < field_1";
  EXPECT_STREQ(expected4, range_string.c_ptr());

  sel_arg7.min_flag = 0;
  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg7, &kpi);
  const char expected5[] = "7 <= field_1";
  EXPECT_STREQ(expected5, range_string.c_ptr());
}

/*
  Create SelArg with a between predicate
*/
TEST_F(OptRangeTest, SelArgBetween) {
  Fake_TABLE fake_table({3, 5}, false);
  Field *field_long3 = fake_table.field[0];
  Field *field_long5 = fake_table.field[1];

  KEY_PART_INFO kpi;
  kpi.init_from_field(field_long3);

  uchar range_val3[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long3->get_key_image(range_val3, kpi.length, Field::itRAW);

  uchar range_val5[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long5->get_key_image(range_val5, kpi.length, Field::itRAW);

  SEL_ARG sel_arg35(field_long3, range_val3, range_val5, true);

  String range_string;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected[] = "3 <= field_1 <= 5";
  EXPECT_STREQ(expected, range_string.c_ptr());

  range_string.length(0);
  sel_arg35.min_flag = NEAR_MIN;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected2[] = "3 < field_1 <= 5";
  EXPECT_STREQ(expected2, range_string.c_ptr());

  range_string.length(0);
  sel_arg35.max_flag = NEAR_MAX;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected3[] = "3 < field_1 < 5";
  EXPECT_STREQ(expected3, range_string.c_ptr());

  range_string.length(0);
  sel_arg35.min_flag = 0;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected4[] = "3 <= field_1 < 5";
  EXPECT_STREQ(expected4, range_string.c_ptr());

  range_string.length(0);
  sel_arg35.min_flag = NO_MIN_RANGE;
  sel_arg35.max_flag = 0;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected5[] = "field_1 <= 5";
  EXPECT_STREQ(expected5, range_string.c_ptr());

  range_string.length(0);
  sel_arg35.min_flag = 0;
  sel_arg35.max_flag = NO_MAX_RANGE;
  print_selarg_ranges(&range_string, &sel_arg35, &kpi);
  const char expected6[] = "3 <= field_1";
  EXPECT_STREQ(expected6, range_string.c_ptr());
}

/*
  Test SelArg::CopyMax
*/
TEST_F(OptRangeTest, CopyMax) {
  Fake_TABLE fake_table({3, 5}, false);
  Field *field_long3 = fake_table.field[0];
  Field *field_long5 = fake_table.field[1];

  KEY_PART_INFO kpi;
  kpi.init_from_field(field_long3);

  uchar range_val3[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long3->get_key_image(range_val3, kpi.length, Field::itRAW);

  uchar range_val5[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long5->get_key_image(range_val5, kpi.length, Field::itRAW);

  SEL_ARG sel_arg3(field_long3, range_val3, range_val3, true);
  sel_arg3.min_flag = NO_MIN_RANGE;
  SEL_ARG sel_arg5(field_long5, range_val5, range_val5, true);
  sel_arg5.min_flag = NO_MIN_RANGE;

  String range_string;
  print_selarg_ranges(&range_string, &sel_arg3, &kpi);
  const char expected[] = "field_1 <= 3";
  EXPECT_STREQ(expected, range_string.c_ptr());

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg5, &kpi);
  const char expected2[] = "field_1 <= 5";
  EXPECT_STREQ(expected2, range_string.c_ptr());

  /*
    Ranges now:
                       -inf ----------------3-5----------- +inf
    sel_arg3:          [-------------------->
    sel_arg5:          [---------------------->
    Below: merge these two ranges into sel_arg3 using copy_max()
  */
  bool full_range = sel_arg3.copy_max(&sel_arg5);
  // The merged range does not cover all possible values
  EXPECT_FALSE(full_range);

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg3, &kpi);
  const char expected3[] = "field_1 <= 5";
  EXPECT_STREQ(expected3, range_string.c_ptr());

  range_string.length(0);
  sel_arg5.min_flag = 0;
  sel_arg5.max_flag = NO_MAX_RANGE;
  print_selarg_ranges(&range_string, &sel_arg5, &kpi);
  const char expected4[] = "5 <= field_1";
  EXPECT_STREQ(expected4, range_string.c_ptr());

  /*
    Ranges now:
                       -inf ----------------3-5----------- +inf
    sel_arg3:          [---------------------->
    sel_arg5:                                 <---------------]
    Below: merge these two ranges into sel_arg3 using copy_max()
  */

  full_range = sel_arg3.copy_max(&sel_arg5);
  // The new range covers all possible values
  EXPECT_TRUE(full_range);

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg3, &kpi);
  const char expected5[] = "field_1";
  EXPECT_STREQ(expected5, range_string.c_ptr());
}

/*
  Test SelArg::CopyMin
*/
TEST_F(OptRangeTest, CopyMin) {
  Fake_TABLE fake_table({3, 5}, false);
  Field *field_long3 = fake_table.field[0];
  Field *field_long5 = fake_table.field[1];

  KEY_PART_INFO kpi;
  kpi.init_from_field(field_long3);

  uchar range_val3[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long3->get_key_image(range_val3, kpi.length, Field::itRAW);

  uchar range_val5[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long5->get_key_image(range_val5, kpi.length, Field::itRAW);

  SEL_ARG sel_arg3(field_long3, range_val3, range_val3, true);
  sel_arg3.max_flag = NO_MAX_RANGE;
  SEL_ARG sel_arg5(field_long5, range_val5, range_val5, true);
  sel_arg5.max_flag = NO_MAX_RANGE;

  String range_string;
  print_selarg_ranges(&range_string, &sel_arg3, &kpi);
  const char expected[] = "3 <= field_1";
  EXPECT_STREQ(expected, range_string.c_ptr());

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg5, &kpi);
  const char expected2[] = "5 <= field_1";
  EXPECT_STREQ(expected2, range_string.c_ptr());

  /*
    Ranges now:
                       -inf ----------------3-5----------- +inf
    sel_arg3:                               <-----------------]
    sel_arg5:                                 <---------------]
    Below: merge these two ranges into sel_arg3 using copy_max()
  */
  bool full_range = sel_arg5.copy_min(&sel_arg3);
  // The merged range does not cover all possible values
  EXPECT_FALSE(full_range);

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg5, &kpi);
  const char expected3[] = "3 <= field_1";
  EXPECT_STREQ(expected3, range_string.c_ptr());

  range_string.length(0);
  sel_arg3.max_flag = 0;
  sel_arg3.min_flag = NO_MIN_RANGE;
  print_selarg_ranges(&range_string, &sel_arg3, &kpi);
  const char expected4[] = "field_1 <= 3";
  EXPECT_STREQ(expected4, range_string.c_ptr());

  /*
    Ranges now:
                       -inf ----------------3-5----------- +inf
    sel_arg3:          [-------------------->
    sel_arg5:                               <-----------------]
    Below: merge these two ranges into sel_arg5 using copy_min()
  */

  full_range = sel_arg5.copy_min(&sel_arg3);
  // The new range covers all possible values
  EXPECT_TRUE(full_range);

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg5, &kpi);
  const char expected5[] = "field_1";
  EXPECT_STREQ(expected5, range_string.c_ptr());
}

/*
  Test SelArg::KeyOr
*/
TEST_F(OptRangeTest, KeyOr1) {
  Fake_RANGE_OPT_PARAM opt_param(thd(), &m_alloc, 0, false);

  Fake_TABLE fake_table({3, 4}, false);
  Field *field_long3 = fake_table.field[0];
  Field *field_long4 = fake_table.field[1];

  KEY_PART_INFO kpi;
  kpi.init_from_field(field_long3);

  uchar range_val3[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long3->get_key_image(range_val3, kpi.length, Field::itRAW);

  uchar range_val4[Fake_TABLE::DEFAULT_PACK_LENGTH];
  field_long4->get_key_image(range_val4, kpi.length, Field::itRAW);

  SEL_ARG sel_arg_lt3(field_long3, range_val3, range_val3, true);
  sel_arg_lt3.part = 0;
  sel_arg_lt3.min_flag = NO_MIN_RANGE;
  sel_arg_lt3.max_flag = NEAR_MAX;

  SEL_ARG sel_arg_gt3(field_long3, range_val3, range_val3, true);
  sel_arg_gt3.part = 0;
  sel_arg_gt3.min_flag = NEAR_MIN;
  sel_arg_gt3.max_flag = NO_MAX_RANGE;

  SEL_ARG sel_arg_lt4(field_long4, range_val4, range_val4, true);
  sel_arg_lt4.part = 0;
  sel_arg_lt4.min_flag = NO_MIN_RANGE;
  sel_arg_lt4.max_flag = NEAR_MAX;

  String range_string;
  print_selarg_ranges(&range_string, &sel_arg_lt3, &kpi);
  const char expected_lt3[] = "field_1 < 3";
  EXPECT_STREQ(expected_lt3, range_string.c_ptr());

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg_gt3, &kpi);
  const char expected_gt3[] = "3 < field_1";
  EXPECT_STREQ(expected_gt3, range_string.c_ptr());

  range_string.length(0);
  print_selarg_ranges(&range_string, &sel_arg_lt4, &kpi);
  const char expected_lt4[] = "field_1 < 4";
  EXPECT_STREQ(expected_lt4, range_string.c_ptr());

  /*
    Ranges now:
                       -inf ----------------34----------- +inf
    sel_arg_lt3:       [-------------------->
    sel_arg_gt3:                             <---------------]
    sel_arg_lt4:       [--------------------->
  */

  SEL_ROOT *tmp =
      key_or(&opt_param, new (thd()->mem_root) SEL_ROOT(&sel_arg_lt3),
             new (thd()->mem_root) SEL_ROOT(&sel_arg_gt3));

  /*
    Ranges now:
                       -inf ----------------34----------- +inf
    tmp:               [--------------------><---------------]
    sel_arg_lt4:       [--------------------->
  */
  range_string.length(0);
  print_selarg_ranges(&range_string, tmp->root, &kpi);
  const char expected_merged[] =
      "field_1 < 3\n"
      "3 < field_1";
  EXPECT_STREQ(expected_merged, range_string.c_ptr());

  SEL_ROOT *tmp2 =
      key_or(&opt_param, tmp, new (thd()->mem_root) SEL_ROOT(&sel_arg_lt4));
  EXPECT_EQ(null_root, tmp2);
}

/*
  Test SelArg::KeyOr (BUG#17619119)
*/
TEST_F(OptRangeTest, KeyOr2) {
  create_table(2);

  m_opt_param->add_key(m_field[1]);
  m_opt_param->add_key(m_field[0], m_field[1]);

  SEL_TREE *fld1_20 = create_tree(new_item_equal(m_field[0], 20),
                                  "result keys[1]: (20 <= field_1 <= 20)\n");

  /*
    Expected result when performing AND of:
      "(field_1 = 20) TREE_AND (field_2 = 1)"
  */
  SEL_TREE *tree_and1 = create_and_check_tree_and(
      fld1_20,
      create_tree(new_item_equal(m_field[1], 1),
                  "result keys[0]: (1 <= field_2 <= 1)\n"    // range idx #1
                  "result keys[1]: (1 <= field_2 <= 1)\n"),  // range idx #2
      SEL_TREE::KEY,
      "result keys[0]: (1 <= field_2 <= 1)\n"                          // idx #1
      "result keys[1]: (20 <= field_1 <= 20 AND 1 <= field_2 <= 1)\n"  // idx #2
  );

  /*
    Expected result when performing AND of:
      "(field_1 = 4) TREE_AND (field_2 = 42)"
  */
  SEL_TREE *tree_and2 = create_and_check_tree_and(
      create_tree(new_item_equal(m_field[0], 4),
                  "result keys[1]: (4 <= field_1 <= 4)\n"),
      create_tree(new_item_equal(m_field[1], 42),
                  "result keys[0]: (42 <= field_2 <= 42)\n"    // range idx #1
                  "result keys[1]: (42 <= field_2 <= 42)\n"),  // range idx #2
      SEL_TREE::KEY,
      "result keys[0]: (42 <= field_2 <= 42)\n"                        // idx #1
      "result keys[1]: (4 <= field_1 <= 4 AND 42 <= field_2 <= 42)\n"  // idx #2
  );

  /*
    Expected result when performing OR of:
      "((field_1 = 20) AND (field_2 = 1))
          TREE_OR
       ((field_1 = 4) AND (field_2 = 42))"
  */
  SEL_TREE *tree_or1 = create_and_check_tree_or(
      tree_and1, tree_and2, SEL_TREE::KEY,
      "result keys[0]: (1 <= field_2 <= 1) OR (42 <= field_2 <= 42)\n"
      "result keys[1]: "
      "(4 <= field_1 <= 4 AND 42 <= field_2 <= 42) OR "
      "(20 <= field_1 <= 20 AND 1 <= field_2 <= 1)\n");

  /*
    Expected result when performing OR of:
      "(field_1 > 13) TREE_OR (field_2 = 14)"

    NOTE: if m_opt_param->remove_jump_scans was 'false', the merge
    would contain another alternative with this range as well:
        "  merge_tree keys[1]: (14 <= field_2 <= 14)\n";
  */
  SEL_TREE *tree_or2 = create_and_check_tree_or(
      create_tree(new_item_gt(m_field[0], 13),
                  "result keys[1]: (13 < field_1)\n"),
      create_tree(new_item_equal(m_field[1], 14),
                  "result keys[0]: (14 <= field_2 <= 14)\n"    // range idx #1
                  "result keys[1]: (14 <= field_2 <= 14)\n"),  // range idx #2
      SEL_TREE::KEY,
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (13 < field_1)\n"
      "  merge_tree keys[0]: (14 <= field_2 <= 14)\n");

  /*
    Expected result when performing OR of:
    ((field_1 = 4) AND (field_2 = 42)) OR ((field_1 = 20) AND (field_2 = 1))
       TREE_OR
    ((field_1 > 13) OR (field_2 = 14))      <- merge

    "field_1=20 AND field_2=1" from the first tree is removed by
    key_or() since it is covered by "field_1 > 13" from the second tree.
  */
  const char exp_or3[] =
      "result contains the following merges\n"
      "--- alternative 1 ---\n"
      "  merge_tree keys[1]: (4 <= field_1 <= 4 AND 42 <= field_2 <= 42) OR "
      "(13 < field_1)\n"
      "  merge_tree keys[0]: (14 <= field_2 <= 14)\n";
  create_and_check_tree_or(tree_or1, tree_or2, SEL_TREE::KEY, exp_or3);

  /*
    fld1_20 was modified to reflect the AND in tree_and1 (and these
    trees are the same). They are no longer used, and trashed.
  */
  EXPECT_EQ(fld1_20, tree_and1);
}

class Mock_SEL_ARG : public SEL_ARG {
 public:
  Mock_SEL_ARG(SEL_ROOT *next_key_part_ptr) {
    next_key_part = next_key_part_ptr;
    make_root();
  }

  Mock_SEL_ARG() {
    make_root();
    part = 1;
    min_flag = 0;
    max_flag = 0;
    maybe_flag = false;
  }
};

/**
  @todo
  - Move some place it can be reused
  - Use varargs instead of copy-paste.
*/
static Item_row *new_Item_row(int a, int b) {
  /*
    The Item_row CTOR doesn't store the reference to the list, hence
    it can live on the stack.
  */
  List<Item> items;
  items.push_front(new Item_int(b));
  return new Item_row(POS(), new Item_int(a), items);
}

static Item_row *new_Item_row(int a, int b, int c) {
  /*
    The Item_row CTOR doesn't store the reference to the list, hence
    it can live on the stack.
  */
  List<Item> items;
  items.push_front(new Item_int(c));
  items.push_front(new Item_int(b));
  return new Item_row(POS(), new Item_int(a), items);
}

/// @todo Move some place it can be reused.
static Item_row *new_Item_row(Field **fields, int count) {
  /*
    The Item_row CTOR doesn't store the reference to the list, hence
    it can live on the stack.
  */
  List<Item> items;
  for (int i = count - 1; i > 0; --i)
    items.push_front(new Item_field(fields[i]));
  return new Item_row(POS(), new Item_field(fields[0]), items);
}

TEST_F(OptRangeTest, RowConstructorIn2) {
  create_table(2);

  m_opt_param->add_key();

  // We build the expression (field_1, field_2) IN ((3, 4), (1, 2)) ...
  PT_item_list *all_args = new (current_thd->mem_root) PT_item_list;
  all_args->push_front(new_Item_row(1, 2));
  all_args->push_front(new_Item_row(3, 4));
  all_args->push_front(new_Item_row(m_opt_param->table->field, 2));
  Item *cond = new Item_func_in(POS(), all_args, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(cond->itemize(&pc, &cond));

  // ... and resolve it.
  Item *item = cond;
  cond->fix_fields(thd(), &item);

  SEL_TREE *sel_tree = get_mm_tree(m_opt_param, cond);

  EXPECT_FALSE(sel_tree == nullptr);
  EXPECT_EQ(Key_map(1), sel_tree->keys_map);

  const char *expected =
      "result keys[0]: "
      "(1 <= field_1 <= 1 AND 2 <= field_2 <= 2) OR "
      "(3 <= field_1 <= 3 AND 4 <= field_2 <= 4)\n";
  check_tree_result(sel_tree, SEL_TREE::KEY, expected);
}

TEST_F(OptRangeTest, RowConstructorIn3) {
  create_table(3);

  m_opt_param->add_key();

  // We build the expression (field_1, field_2) IN ((3, 4), (1, 2)) ...
  PT_item_list *all_args = new (current_thd->mem_root) PT_item_list;
  all_args->push_front(new_Item_row(1, 2, 3));
  all_args->push_front(new_Item_row(4, 5, 6));
  all_args->push_front(new_Item_row(m_opt_param->table->field, 3));
  Item *cond = new Item_func_in(POS(), all_args, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(cond->itemize(&pc, &cond));

  // ... and resolve it.
  Item *item = cond;
  cond->fix_fields(thd(), &item);

  SEL_TREE *sel_tree = get_mm_tree(m_opt_param, cond);

  EXPECT_FALSE(sel_tree == nullptr);
  EXPECT_EQ(Key_map(1), sel_tree->keys_map);

  const char *expected =
      "result keys[0]: "
      "(1 <= field_1 <= 1 AND 2 <= field_2 <= 2 AND 3 <= field_3 <= 3) OR "
      "(4 <= field_1 <= 4 AND 5 <= field_2 <= 5 AND 6 <= field_3 <= 6)\n";

  check_tree_result(sel_tree, SEL_TREE::KEY, expected);
}

TEST_F(OptRangeTest, CombineAlways) {
  RANGE_OPT_PARAM param;  // Not really used
  {
    Mock_SEL_ARG always_root;
    always_root.min_flag = NO_MIN_RANGE;
    always_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always(&always_root);

    Mock_SEL_ARG key_range_root;
    SEL_ROOT key_range(&key_range_root);

    EXPECT_TRUE(key_or(&param, &always, &key_range) == &always);
  }
  {
    Mock_SEL_ARG always_root;
    always_root.min_flag = NO_MIN_RANGE;
    always_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always(&always_root);

    Mock_SEL_ARG key_range_root;
    SEL_ROOT key_range(&key_range_root);

    EXPECT_TRUE(key_or(&param, &key_range, &always) == &always);
  }
  {
    Mock_SEL_ARG always1_root;
    always1_root.min_flag = NO_MIN_RANGE;
    always1_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always1(&always1_root);

    Mock_SEL_ARG always2_root;
    always2_root.min_flag = NO_MIN_RANGE;
    always2_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always2(&always2_root);

    EXPECT_TRUE(key_or(&param, &always1, &always2) == &always1);
  }
  {
    Mock_SEL_ARG always_root;
    always_root.min_flag = NO_MIN_RANGE;
    always_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always(&always_root);

    Mock_SEL_ARG key_range_root;
    SEL_ROOT key_range(&key_range_root);

    EXPECT_TRUE(key_and(&param, &key_range, &always) == &key_range);
  }
  {
    Mock_SEL_ARG always_root;
    always_root.min_flag = NO_MIN_RANGE;
    always_root.max_flag = NO_MAX_RANGE;
    SEL_ROOT always(&always_root);

    Mock_SEL_ARG key_range_root;
    SEL_ROOT key_range(&key_range_root);

    EXPECT_TRUE(key_and(&param, &always, &key_range) == &key_range);
  }
}

TEST_F(OptRangeTest, CombineAlways2) {
  class Fake_sel_arg : public SEL_ARG {
   public:
    Fake_sel_arg() {
      part = 0;
      left = nullptr;
      next = nullptr;
      min_flag = max_flag = maybe_flag = false;
      set_endpoints(1, 2);
      next_key_part = nullptr;
      make_root();
    }

    void add_next_key_part(SEL_ROOT *next_arg) {
      set_next_key_part(next_arg);
      next_arg->root->part = part + 1;
    }

   private:
    void set_endpoints(int min, int max) {
      set_endpoint(min, min_value_buff, &min_value);
      set_endpoint(max, max_value_buff, &max_value);
    }
    void set_endpoint(int value, char *buff, uchar **variable) {
      buff[0] = value;
      buff[1] = 0;
      *variable = reinterpret_cast<uchar *>(buff);
    }
    char min_value_buff[10], max_value_buff[10];
  };

  class Fake_key_part_info : public KEY_PART_INFO {
   public:
    Fake_key_part_info(Mock_field_long *field_arg) {
      field = field_arg;
      length = 1;
    }
  };

  RANGE_OPT_PARAM param;
  Fake_sel_arg always_root, key_range_root;
  always_root.min_flag = NO_MIN_RANGE;
  always_root.max_flag = NO_MAX_RANGE;
  SEL_ROOT always(&always_root), key_range(&key_range_root);
  Mock_field_long field1("col_1", false);
  Mock_field_long field2("col_2", false);
  Fake_TABLE table(&field1, &field2);
  String res(1000), so_far(1000);
  Fake_key_part_info key_part_info[] = {Fake_key_part_info(&field1),
                                        Fake_key_part_info(&field2)};

  Fake_sel_arg other_root;
  other_root.add_next_key_part(&key_range);
  SEL_ROOT other(&other_root);
  append_range_all_keyparts(nullptr, &res, &so_far, &other, key_part_info,
                            true);

  // Let's make sure we built the expression we expected ...
  EXPECT_STREQ("(1 <= col_1 <= 2 AND 1 <= col_2 <= 2)", res.ptr());

  EXPECT_TRUE(key_or(&param, &always, &other) == &always);
}

TEST_F(OptRangeTest, AppendRange) {
  String out(100);
  Mock_field_long field("my_field", false);
  Fake_TABLE table(&field);
  KEY_PART_INFO kp;
  kp.field = &field;
  kp.length = 1;
  uchar value = 42;
  append_range(&out, &kp, &value, &value, NEAR_MIN | NEAR_MAX);
  EXPECT_STREQ("42 < my_field < 42", out.c_ptr());
}

TEST_F(OptRangeTest, TreeRootGetsUpdated) {
  /*
    Create a bunch of SEL_ARGs (from 0 up to 10). The simplest way
    of creating them seems to just be calling get_mm_tree() and deleting
    the resulting SEL_ARG.
  */
  Fake_RANGE_OPT_PARAM opt_param(thd(), &m_alloc, 1, false);
  opt_param.add_key(opt_param.table->field[0]);
  std::vector<SEL_ARG *> args;
  for (int i = 0; i < 10; ++i) {
    SEL_TREE *tree =
        get_mm_tree(&opt_param, new_item_equal(opt_param.table->field[0], i));
    ASSERT_NE(nullptr, tree);
    SEL_ROOT *root = tree->keys[0];
    ASSERT_EQ(1, root->elements);
    SEL_ARG *arg = root->root;
    root->tree_delete(arg);
    args.push_back(arg);
  }

  // Make a SEL_ROOT with the first element in it.
  SEL_ROOT root(args[0]);
  EXPECT_EQ(args[0], root.root);

  /*
    Now insert the nine others; since they're all bigger, the root should
    be a different one in any reasonably balanced tree, so we can verify
    this works as it should.
  */
  for (int i = 1; i < 10; ++i) {
    root.insert(args[i]);
  }
  EXPECT_EQ(args.size(), root.elements);
  EXPECT_NE(args[0], root.root);
}

}  // namespace opt_range_unittest

#undef create_tree
#undef create_and_check_tree_and
#undef create_and_check_tree_or
