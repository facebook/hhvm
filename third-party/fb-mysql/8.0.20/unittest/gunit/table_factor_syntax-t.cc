/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <string>

#include "sql/item_func.h"
#include "sql/nested_join.h"
#include "sql/sql_lex.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

namespace table_factor_syntax_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class TableFactorSyntaxTest : public ParserTest {
 protected:
  void test_table_factor_syntax(const char *query, int num_terms,
                                bool expect_syntax_error) {
    SELECT_LEX *term1 = parse(query, expect_syntax_error ? ER_PARSE_ERROR : 0);
    EXPECT_EQ(nullptr, term1->first_inner_unit());
    EXPECT_EQ(nullptr, term1->next_select_in_list());
    EXPECT_EQ(1, term1->get_item_list()->head()->val_int());

    SELECT_LEX_UNIT *top_union = term1->master_unit();
    EXPECT_EQ(nullptr, top_union->outer_select());

    if (num_terms > 1) {
      SELECT_LEX *term2 = term1->next_select();
      ASSERT_FALSE(term2 == nullptr);

      EXPECT_EQ(nullptr, term2->first_inner_unit());
      EXPECT_EQ(term1, term2->next_select_in_list());
      EXPECT_EQ(2, term2->get_item_list()->head()->val_int());

      if (num_terms <= 2) {
        EXPECT_EQ(nullptr, term2->next_select());
      }

      EXPECT_EQ(top_union, term2->master_unit());
    }
  }

  void test_global_limit(const char *query) {
    SELECT_LEX *first_term = parse(query);
    SELECT_LEX_UNIT *unit = first_term->master_unit();
    EXPECT_EQ(1U, unit->global_parameters()->order_list.elements) << query;
    EXPECT_FALSE(unit->global_parameters()->select_limit == nullptr) << query;
  }
};

void check_query_block(SELECT_LEX *block, int select_list_item,
                       const char *tablename) {
  EXPECT_EQ(1U, block->item_list.elements);
  EXPECT_EQ(select_list_item, block->item_list.head()->val_int());

  EXPECT_EQ(1U, block->top_join_list.size());
  EXPECT_STREQ(tablename, block->top_join_list.front()->alias);
}

TEST_F(TableFactorSyntaxTest, Single) {
  SELECT_LEX *term = parse("SELECT 2 FROM (SELECT 1 FROM t1) dt;", 0);
  EXPECT_EQ(nullptr, term->outer_select());
  SELECT_LEX_UNIT *top_union = term->master_unit();

  EXPECT_EQ(term, top_union->first_select());
  EXPECT_EQ(nullptr, term->next_select());

  ASSERT_EQ(1U, term->top_join_list.size());
  EXPECT_STREQ("dt", term->top_join_list.front()->alias);

  SELECT_LEX_UNIT *inner_union = term->first_inner_unit();

  SELECT_LEX *inner_term = inner_union->first_select();

  check_query_block(inner_term, 1, "t1");
}

TEST_F(TableFactorSyntaxTest, TablelessTableSubquery) {
  SELECT_LEX *term = parse("SELECT 1 FROM (SELECT 2) a;", 0);
  EXPECT_EQ(nullptr, term->outer_select());
  SELECT_LEX_UNIT *top_union = term->master_unit();

  EXPECT_EQ(term, top_union->first_select());
  EXPECT_EQ(nullptr, term->next_select());

  ASSERT_EQ(1U, term->top_join_list.size());
  EXPECT_STREQ("a", term->top_join_list.front()->alias);

  SELECT_LEX_UNIT *inner_union = term->first_inner_unit();

  SELECT_LEX *inner_term = inner_union->first_select();

  EXPECT_EQ(nullptr, inner_term->first_inner_unit());

  EXPECT_NE(term, term->table_list.first->derived_unit()->first_select())
      << "No cycle in the AST, please.";
}

TEST_F(TableFactorSyntaxTest, Union) {
  SELECT_LEX *block = parse(
      "SELECT 1 FROM (SELECT 1 FROM t1 UNION SELECT 2 FROM t2) dt "
      "WHERE d1.a = 1",
      0);
  SELECT_LEX_UNIT *top_union = block->master_unit();

  EXPECT_EQ(block, top_union->first_select());
  EXPECT_EQ(nullptr, block->next_select());

  TABLE_LIST *dt = block->table_list.first;
  EXPECT_EQ(dt, block->context.first_name_resolution_table);

  Item_func *top_where_cond = down_cast<Item_func *>(block->where_cond());
  Item_field *d1a = down_cast<Item_field *>(top_where_cond->arguments()[0]);
  ASSERT_FALSE(d1a->context == nullptr);
  EXPECT_EQ(dt, d1a->context->first_name_resolution_table);

  EXPECT_EQ(1U, block->top_join_list.size());
  EXPECT_STREQ("dt", block->top_join_list.front()->alias);

  SELECT_LEX_UNIT *inner_union = block->first_inner_unit();

  SELECT_LEX *first_inner_block = inner_union->first_select();
  SELECT_LEX *second_inner_block = first_inner_block->next_select();

  TABLE_LIST *t1 = first_inner_block->table_list.first;
  TABLE_LIST *t2 = second_inner_block->table_list.first;

  EXPECT_EQ(t1, first_inner_block->context.first_name_resolution_table);
  EXPECT_EQ(t2, second_inner_block->context.first_name_resolution_table);

  EXPECT_EQ(nullptr, t1->nested_join);
  EXPECT_EQ(nullptr, t2->nested_join);

  check_query_block(first_inner_block, 1, "t1");
  check_query_block(second_inner_block, 2, "t2");

  EXPECT_EQ(nullptr, block->outer_select());
}

TEST_F(TableFactorSyntaxTest, NestedJoin) {
  SELECT_LEX *term = parse("SELECT * FROM (t1 JOIN t2 ON TRUE)", 0);
  SELECT_LEX_UNIT *top_union = term->master_unit();

  EXPECT_EQ(term, top_union->first_select());
}

TEST_F(TableFactorSyntaxTest, NestedNestedJoin) {
  SELECT_LEX *term =
      parse("SELECT * FROM ((t1 JOIN t2 ON TRUE) JOIN t3 ON TRUE)", 0);
  SELECT_LEX_UNIT *top_union = term->master_unit();

  EXPECT_EQ(term, top_union->first_select());
}

TEST_F(TableFactorSyntaxTest, NestedTableReferenceList) {
  SELECT_LEX *term1 =
      parse("SELECT * FROM t1 LEFT JOIN ( t2 JOIN t3 JOIN t4 ) ON t1.a = t3.a");

  SELECT_LEX *term2 =
      parse("SELECT * FROM t1 LEFT JOIN ( t2, t3, t4 ) ON t1.a = t3.a");

  SELECT_LEX_UNIT *top_union = term1->master_unit();
  SELECT_LEX_UNIT *top_union2 = term2->master_unit();

  EXPECT_EQ(term1, top_union->first_select());
  EXPECT_EQ(term2, top_union2->first_select());

  EXPECT_EQ(4U, term1->table_list.elements);
  ASSERT_EQ(4U, term2->table_list.elements);

  EXPECT_STREQ("t1", term1->table_list.first->alias);
  EXPECT_STREQ("t1", term2->table_list.first->alias);

  EXPECT_STREQ("(nest_last_join)", term1->join_list->front()->alias);
  EXPECT_STREQ("(nest_last_join)", term2->join_list->front()->alias);

  TABLE_LIST *t2_join_t3_join_t4 = term1->join_list->front();
  TABLE_LIST *t2_join_t3_join_t4_2 = term2->join_list->front();

  TABLE_LIST *t3_join_t4 = t2_join_t3_join_t4->nested_join->join_list.front();
  TABLE_LIST *t3_join_t4_2 =
      t2_join_t3_join_t4_2->nested_join->join_list.front();

  EXPECT_STREQ("(nest_last_join)", t3_join_t4->alias);
  EXPECT_STREQ("(nest_last_join)", t3_join_t4_2->alias);

  EXPECT_STREQ("t4", t3_join_t4->nested_join->join_list.front()->alias);
}

TEST_F(TableFactorSyntaxTest, LimitAndOrder) {
  test_global_limit("SELECT 1 AS c UNION (SELECT 1 AS c) ORDER BY c LIMIT 1");
  test_global_limit("(SELECT 1 AS c UNION SELECT 1 AS c) ORDER BY c LIMIT 1");
  test_global_limit("((SELECT 1 AS c) UNION SELECT 1 AS c) ORDER BY c LIMIT 1");
  test_global_limit("(SELECT 1 AS c UNION (SELECT 1 AS c)) ORDER BY c LIMIT 1");
}

}  // namespace table_factor_syntax_unittest
