/*
 * Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include <gtest/gtest.h>

#include "plugin/x/src/statement_builder.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

class Crud_statement_builder_stub : public Crud_statement_builder {
 public:
  explicit Crud_statement_builder_stub(Expression_generator *gen)
      : Crud_statement_builder(*gen) {}

  using Crud_statement_builder::add_collection;
  using Crud_statement_builder::add_filter;
  using Crud_statement_builder::add_limit;
  using Crud_statement_builder::add_limit_expr_field;
  using Crud_statement_builder::add_limit_field;
  using Crud_statement_builder::add_order;
};

class Crud_statement_builder_test : public ::testing::Test {
 public:
  Crud_statement_builder_stub &builder(
      const bool is_table_data_model = DM_TABLE) {
    expr_gen.reset(
        new Expression_generator(&query, args, schema, is_table_data_model));
    stub.reset(new Crud_statement_builder_stub(expr_gen.get()));
    return *stub;
  }

  Expression_generator::Arg_list args;
  Query_string_builder query;
  std::string schema;
  std::unique_ptr<Expression_generator> expr_gen;
  std::unique_ptr<Crud_statement_builder_stub> stub;

  enum { DM_DOCUMENT = 0, DM_TABLE = 1 };
};

TEST_F(Crud_statement_builder_test, add_table_only_name) {
  ASSERT_NO_THROW(builder().add_collection(Collection("xtable")));
  EXPECT_EQ("`xtable`", query.get());
}

TEST_F(Crud_statement_builder_test, add_collection_only_schema) {
  ASSERT_THROW(builder().add_collection(Collection("", "xschema")),
               ngs::Error_code);
}

TEST_F(Crud_statement_builder_test, add_collection_name_and_schema) {
  ASSERT_NO_THROW(builder().add_collection(Collection("xtable", "xschema")));
  EXPECT_EQ("`xschema`.`xtable`", query.get());
}

TEST_F(Crud_statement_builder_test, add_filter_uninitialized) {
  Filter filter;
  ASSERT_NO_THROW(builder().add_filter(filter));
  EXPECT_EQ("", query.get());
}

TEST_F(Crud_statement_builder_test, add_filter_initialized_column) {
  ASSERT_NO_THROW(builder().add_filter(
      Filter(Operator(">", Column_identifier("A"), Scalar(1.0)))));
  EXPECT_EQ(" WHERE (`A` > 1)", query.get());
}

TEST_F(Crud_statement_builder_test, add_filter_initialized_column_and_memeber) {
  ASSERT_NO_THROW(builder().add_filter(Filter(Operator(
      ">", Column_identifier(Document_path{"first"}, "A"), Scalar(1.0)))));
  EXPECT_EQ(" WHERE (JSON_EXTRACT(`A`,'$.first') > 1)", query.get());
}

TEST_F(Crud_statement_builder_test, add_filter_bad_expression) {
  ASSERT_THROW(builder().add_filter(Filter(Operator(
                   "><", Column_identifier("A"), Column_identifier("B")))),
               Expression_generator::Error);
}

TEST_F(Crud_statement_builder_test, add_filter_with_arg) {
  args = Expression_list{1.0};

  ASSERT_NO_THROW(builder().add_filter(
      Filter(Operator(">", Column_identifier("A"), Placeholder(0)))));
  EXPECT_EQ(" WHERE (`A` > 1)", query.get());
}

TEST_F(Crud_statement_builder_test, add_filter_missing_arg) {
  ASSERT_THROW(builder().add_filter(Filter(
                   Operator(">", Column_identifier("A"), Placeholder(0)))),
               Expression_generator::Error);
}

TEST_F(Crud_statement_builder_test, add_order_empty_list) {
  ASSERT_NO_THROW(builder().add_order(Order_list()));
  EXPECT_EQ("", query.get());
}

TEST_F(Crud_statement_builder_test, add_order_one_item) {
  ASSERT_NO_THROW(
      builder().add_order(Order_list{Order(Column_identifier("A"))}));
  EXPECT_EQ(" ORDER BY `A`", query.get());
}

TEST_F(Crud_statement_builder_test, add_order_two_items) {
  ASSERT_NO_THROW(builder().add_order(
      Order_list{{Column_identifier("A"), Mysqlx::Crud::Order::DESC},
                 {Column_identifier("B")}}));
  EXPECT_EQ(" ORDER BY `A` DESC,`B`", query.get());
}

TEST_F(Crud_statement_builder_test, add_order_two_items_placeholder) {
  args = Expression_list{2};

  ASSERT_NO_THROW(builder().add_order(Order_list{
      {Column_identifier("A"), Mysqlx::Crud::Order::DESC}, {Placeholder(0)}}));
  EXPECT_EQ(" ORDER BY `A` DESC,2", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_expr_uninitialized) {
  ASSERT_NO_THROW(builder().add_limit_field(Limit(), false));
  EXPECT_EQ("", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_field_only) {
  ASSERT_NO_THROW(builder().add_limit_field(Limit(2), false));
  EXPECT_EQ(" LIMIT 2", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_field_and_offset) {
  ASSERT_NO_THROW(builder().add_limit_field(Limit(2, 5), false));
  EXPECT_EQ(" LIMIT 5, 2", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_field_forbbiden_offset) {
  EXPECT_THROW(builder().add_limit_field(Limit(2, 5), true), ngs::Error_code);
}

TEST_F(Crud_statement_builder_test, add_limit_expr_field_uninitialized) {
  ASSERT_NO_THROW(builder().add_limit_expr_field(Limit_expr(), false));
  EXPECT_EQ("", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_expr_field_only) {
  ASSERT_NO_THROW(builder().add_limit_expr_field(Limit_expr(2), false));
  EXPECT_EQ(" LIMIT 2", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_expr_field_and_offset) {
  ASSERT_NO_THROW(builder().add_limit_expr_field(Limit_expr(2, 5), false));
  EXPECT_EQ(" LIMIT 5, 2", query.get());
}

TEST_F(Crud_statement_builder_test,
       add_limit_expr_field_row_and_offset_placeholder_row) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);
  ASSERT_NO_THROW(
      sut.add_limit_expr_field(Limit_expr(Placeholder(2), 5), false));
  EXPECT_EQ(" LIMIT 5, ?", query.get());
}

TEST_F(Crud_statement_builder_test,
       add_limit_expr_field_row_and_offset_placeholder_both) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);

  ASSERT_NO_THROW(sut.add_limit_expr_field(
      Limit_expr(Placeholder(2), Placeholder(1)), false));
  EXPECT_EQ(" LIMIT ?, ?", query.get());
}

TEST_F(Crud_statement_builder_test,
       add_limit_expr_field_row_and_offset_placeholder_offset) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);

  ASSERT_NO_THROW(
      sut.add_limit_expr_field(Limit_expr(2, Placeholder(1)), false));
  EXPECT_EQ(" LIMIT ?, 2", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_expr_field_row_placeholder_row) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);

  ASSERT_NO_THROW(sut.add_limit_expr_field(Limit_expr(Placeholder(1)), false));
  EXPECT_EQ(" LIMIT ?", query.get());
}

TEST_F(Crud_statement_builder_test, add_limit_expr_field_forbbiden_offset) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);

  EXPECT_THROW(sut.add_limit_expr_field(Limit_expr(2, 5), true),
               ngs::Error_code);
}

TEST_F(Crud_statement_builder_test, add_limit_and_limit_expr_fields_forbbiden) {
  auto &sut = builder();
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  expr_gen->set_prep_stmt_placeholder_list(&placeholders);
  Mysqlx::Crud::Find find;

  find.mutable_limit()->CopyFrom(Limit(2, 5));
  find.mutable_limit_expr()->CopyFrom(Limit_expr(2, 5));

  EXPECT_THROW(sut.add_limit(find, false), ngs::Error_code);
}

}  // namespace test
}  // namespace xpl
