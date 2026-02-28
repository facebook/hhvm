/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
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

#include "mysqld_error.h"
#include "plugin/x/src/update_statement_builder.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

class Update_statement_builder_stub : public Update_statement_builder {
 public:
  explicit Update_statement_builder_stub(Expression_generator *gen)
      : Update_statement_builder(*gen) {}
  using Update_statement_builder::add_document_operation;
  using Update_statement_builder::add_document_operation_item;
  using Update_statement_builder::add_operation;
  using Update_statement_builder::add_table_operation;
  using Update_statement_builder::Operation_list;
};

class Update_statement_builder_test : public ::testing::Test {
 public:
  Update_statement_builder_stub &builder() {
    expr_gen.reset(new Expression_generator(&query, args, schema,
                                            is_table_data_model(msg)));
    stub.reset(new Update_statement_builder_stub(expr_gen.get()));
    return *stub;
  }

  Update_statement_builder::Update msg;
  Expression_generator::Arg_list &args = *msg.mutable_args();
  Query_string_builder query;
  std::string schema;
  std::unique_ptr<Expression_generator> expr_gen;
  std::unique_ptr<Update_statement_builder_stub> stub;

  Update_operation::Update_type operation_id =
      Update_operation::Base::ITEM_REMOVE;

  enum { DM_DOCUMENT = 0, DM_TABLE = 1 };

  void fill_table_msg(const Limit &limit) {
    msg = Update({"xtable", "xschema"}, Mysqlx::Crud::TABLE)
              .operation({Update_operation::Base::SET,
                          Column_identifier{"yfield"}, Scalar{"booom"}})
              .criteria(Operator(">", Column_identifier{"xfield"}, Scalar{1.0}))
              .order({Column_identifier("xfield"), Order::Base::DESC})
              .limit(limit);
  }
};  // namespace test

TEST_F(Update_statement_builder_test, add_operation_empty_list) {
  EXPECT_THROW(builder().add_operation(Operation_list(), DM_TABLE),
               ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_table_operation_one_item) {
  EXPECT_NO_THROW(builder().add_table_operation(
      Operation_list{{Update_operation::Base::SET, Column_identifier("xfield"),
                      Scalar(1.0)}}));
  EXPECT_STREQ("`xfield`=1", query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_two_items) {
  EXPECT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::SET, Column_identifier("xfield"), Scalar(1.0)},
      {Update_operation::Base::SET, Column_identifier("yfield"),
       Scalar("two")}}));
  EXPECT_STREQ("`xfield`=1,`yfield`='two'", query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_table_operation_two_items_same_source) {
  EXPECT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::SET, Column_identifier("xfield"), Scalar(1.0)},
      {Update_operation::Base::SET, Column_identifier("xfield"),
       Scalar("two")}}));
  EXPECT_STREQ("`xfield`=1,`xfield`='two'", query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_table_operation_two_items_placeholder) {
  args = Expression_list{2.2};

  EXPECT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::SET, Column_identifier("xfield"), Scalar(1.0)},
      {Update_operation::Base::SET, Column_identifier("yfield"),
       Placeholder(0)}}));
  EXPECT_STREQ("`xfield`=1,`yfield`=2.2", query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_empty_name) {
  EXPECT_THROW(
      builder().add_table_operation(Operation_list{
          {Update_operation::Base::SET, Column_identifier(""), Scalar(1.0)}}),
      ngs::Error_code);
}

TEST_F(Update_statement_builder_test,
       add_table_operation_item_name_with_table) {
  EXPECT_THROW(builder().add_table_operation(Operation_list{
                   {Update_operation::Base::SET,
                    Column_identifier("xfield", "xtable"), Scalar(1.0)}}),
               ngs::Error_code);
}

TEST_F(Update_statement_builder_test,
       add_table_operation_item_name_with_table_and_schema) {
  EXPECT_THROW(
      builder().add_table_operation(Operation_list{
          {Update_operation::Base::SET,
           Column_identifier("xfield", "xtable", "xschema"), Scalar(1.0)}}),
      ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_operation_one_item_for_table) {
  EXPECT_NO_THROW(builder().add_operation(
      Operation_list{{Update_operation::Base::SET, Column_identifier("xfield"),
                      Scalar(1.0)}},
      DM_TABLE));
  EXPECT_STREQ(" SET `xfield`=1", query.get().c_str());
}

TEST_F(Update_statement_builder_test, build_update_for_table) {
  fill_table_msg(Limit(2));
  EXPECT_NO_THROW(builder().build(msg));
  EXPECT_STREQ(
      "UPDATE `xschema`.`xtable`"
      " SET `yfield`='booom'"
      " WHERE (`xfield` > 1)"
      " ORDER BY `xfield` DESC"
      " LIMIT 2",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       build_update_for_table_forrbiden_offset_in_limit) {
  fill_table_msg(Limit(2, 5));
  EXPECT_THROW(builder().build(msg), ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_document_operation_not_allowed_set) {
  EXPECT_THROW(
      builder().add_document_operation(Operation_list{
          {Update_operation::Base::SET, Document_path{"first"}, Scalar(1.0)}}),
      ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_document_operation_remove) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_REMOVE, Document_path{"first"}}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_REMOVE(doc,'$.first'),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_remove_value) {
  EXPECT_THROW(
      builder().add_document_operation(Operation_list{
          {Update_operation::Base::ITEM_REMOVE, Document_path{"first"}, 666}}),
      ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_document_operation_set) {
  EXPECT_NO_THROW(builder().add_document_operation(
      Operation_list{{Update_operation::Base::ITEM_SET, Document_path{"first"},
                      Scalar(1.0)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(doc,'$.first',1),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_replace) {
  EXPECT_NO_THROW(builder().add_document_operation(
      Operation_list{{Update_operation::Base::ITEM_REPLACE,
                      Document_path{"first"}, Scalar(1.0)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_REPLACE(doc,'$.first',1),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_array_insert) {
  EXPECT_NO_THROW(builder().add_document_operation(
      Operation_list{{Update_operation::Base::ARRAY_INSERT,
                      Document_path{"first", 0}, Scalar(1.0)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_ARRAY_INSERT(doc,'$.first[0]',1),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_array_append) {
  EXPECT_NO_THROW(builder().add_document_operation(
      Operation_list{{Update_operation::Base::ARRAY_APPEND,
                      Document_path{"first"}, Scalar(1.0)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_ARRAY_APPEND(doc,'$.first',1),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_document_operation_array_append_twice) {
  EXPECT_NO_THROW(builder().add_document_operation(
      Operation_list{{Update_operation::Base::ARRAY_APPEND,
                      Document_path{"first"}, Scalar(1.0)},
                     {Update_operation::Base::ARRAY_APPEND,
                      Document_path{"first"}, Scalar("two")}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_ARRAY_APPEND(doc,'$.first',1,'$.first','two'),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_remove_twice) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_REMOVE, Document_path{"first"}},
      {Update_operation::Base::ITEM_REMOVE, Document_path{"second"}}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_REMOVE(doc,'$.first','$.second'),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_set_twice) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_SET, Document_path{"first"}, Scalar(1.0)},
      {Update_operation::Base::ITEM_SET, Document_path{"second"},
       Scalar("two")}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(doc,'$.first',1,'$.second','two'),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_document_operation_set_twice_placeholder) {
  args = Expression_list{2.2};
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_SET, Document_path{"first"}, Scalar(1.0)},
      {Update_operation::Base::ITEM_SET, Document_path{"second"},
       Placeholder(0)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(doc,'$.first',1,'$.second',2.2),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_document_operation_merge_no_empty_path) {
  EXPECT_THROW(builder().add_document_operation(Operation_list{
                   {Update_operation::Base::ITEM_MERGE, Document_path{"first"},
                    Scalar("{\"two\": 2.0}")}}),
               ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_document_operation_merge_to_array) {
  EXPECT_THROW(builder().add_document_operation(Operation_list{
                   {Update_operation::Base::ITEM_MERGE,
                    Document_path{"first", 3}, Scalar("{\"two\": 2.0}")}}),
               ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_document_operation_remove_set) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_REMOVE, Document_path{"first"}},
      {Update_operation::Base::ITEM_SET, Document_path{"second"},
       Scalar("two")}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(JSON_REMOVE(doc,'$.first'),'$.second','two'),"
      "'$._id',JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_remove_twice_set) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_REMOVE, Document_path{"first"}},
      {Update_operation::Base::ITEM_REMOVE, Document_path{"second"}},
      {Update_operation::Base::ITEM_SET, Document_path{"third"}, Scalar(-3)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(JSON_REMOVE(doc,'$.first','$.second'),"
      "'$.third',-3),'$._id',JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_set_remove_set) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_SET, Document_path{"first"}, Scalar(1.0)},
      {Update_operation::Base::ITEM_REMOVE, Document_path{"second"}},
      {Update_operation::Base::ITEM_SET, Document_path{"third"}, Scalar(-3)}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(JSON_REMOVE(JSON_SET(doc,'$.first',1),'$.second'),"
      "'$.third',-3),'$._id',JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_forbiden_column) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(Update_operation::Base::ITEM_SET,
                                    Column_identifier("xcolumn"), Scalar(-3)),
                   &operation_id),
               ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_forbiden_schema) {
  ASSERT_THROW(
      builder().add_document_operation_item(
          Update_operation(Update_operation::Base::ITEM_SET,
                           Column_identifier("", "", "xschema"), Scalar(-3)),
          &operation_id),
      ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_forbiden_table) {
  ASSERT_THROW(
      builder().add_document_operation_item(
          Update_operation(Update_operation::Base::ITEM_SET,
                           Column_identifier("", "xtable"), Scalar(-3)),
          &operation_id),
      ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_forbiden_id_change) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(Update_operation::Base::ITEM_SET,
                                    Document_path{"_id"}, Scalar(-3)),
                   &operation_id),
               ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_empty_document_path) {
  operation_id = Update_operation::Base::ITEM_SET;
  ASSERT_NO_THROW(builder().add_document_operation_item(
      Update_operation(Update_operation::Base::ITEM_SET, Column_identifier(),
                       Scalar(-3)),
      &operation_id));
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
  EXPECT_STREQ(",'$',-3", query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_item_root_path) {
  ASSERT_NO_THROW(builder().add_document_operation_item(
      Update_operation(Update_operation::Base::ITEM_SET, Document_path(),
                       Scalar(-3)),
      &operation_id));
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_empty_member) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(Update_operation::Base::ITEM_SET,
                                    Document_path{"first", ""}, Scalar(-3)),
                   &operation_id),
               xpl::Expression_generator::Error);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_empty_member_reverse) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(Update_operation::Base::ITEM_SET,
                                    Document_path{"", "first"}, Scalar(-3)),
                   &operation_id),
               xpl::Expression_generator::Error);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_root_as_array) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(Update_operation::Base::ITEM_SET,
                                    Document_path{0}, Scalar(-3)),
                   &operation_id),
               ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_root_as_array_asterisk) {
  ASSERT_THROW(
      builder().add_document_operation_item(
          Update_operation(
              Update_operation::Base::ITEM_SET,
              Document_path{Document_path_item::Base::ARRAY_INDEX_ASTERISK},
              Scalar(-3)),
          &operation_id),
      ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test,
       add_document_operation_item_root_double_asterisk) {
  ASSERT_THROW(builder().add_document_operation_item(
                   Update_operation(
                       Update_operation::Base::ITEM_SET,
                       Document_path{Document_path_item::Base::DOUBLE_ASTERISK},
                       Scalar(-3)),
                   &operation_id),
               ngs::Error_code);
  ASSERT_EQ(Update_operation::Base::ITEM_SET, operation_id);
}

TEST_F(Update_statement_builder_test, add_operation_one_item_for_document) {
  ASSERT_NO_THROW(builder().add_operation(
      Operation_list{{Update_operation::Base::ITEM_SET, Document_path{"first"},
                      Scalar(1.0)}},
      DM_DOCUMENT));
  EXPECT_STREQ(
      " SET doc=JSON_SET(JSON_SET(doc,'$.first',1),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, build_update_for_document) {
  msg =
      Update({"xtable", "xschema"}, Mysqlx::Crud::DOCUMENT)
          .operation({Update_operation::Base::ITEM_SET, Document_path{"first"},
                      Scalar(1.0)})
          .criteria(Operator(">", Column_identifier{Document_path{"second"}},
                             Scalar{1.0}))
          .order({Column_identifier{Document_path{"third"}}, Order::Base::DESC})
          .limit({2});
  EXPECT_NO_THROW(builder().build(msg));
  EXPECT_STREQ(
      "UPDATE `xschema`.`xtable`"
      " SET doc=JSON_SET(JSON_SET(doc,'$.first',1),"
      "'$._id',JSON_EXTRACT(`doc`,'$._id'))"
      " WHERE (JSON_EXTRACT(doc,'$.second') > 1)"
      " ORDER BY JSON_EXTRACT(doc,'$.third')"
      " DESC LIMIT 2",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_document_operation_set_whole_doc) {
  ASSERT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_SET, Document_path{""}, Scalar("two")}}));
  EXPECT_STREQ(
      "doc=JSON_SET(JSON_SET(doc,'$','two'),'$._id',"
      "JSON_EXTRACT(`doc`,'$._id'))",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_table_operation_set_needless_doc_path) {
  ASSERT_THROW(
      builder().add_table_operation(Operation_list{
          {Update_operation::Base::SET,
           Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)}}),
      ngs::Error_code);
}

TEST_F(Update_statement_builder_test,
       add_table_operation_item_set_missing_doc_path) {
  ASSERT_THROW(builder().add_table_operation(
                   Operation_list{{Update_operation::Base::ITEM_SET,
                                   Column_identifier("xfield"), Scalar(1.0)}}),
               ngs::Error_code);
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)}}));
  EXPECT_STREQ("`xfield`=JSON_SET(`xfield`,'$.first',1)", query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")}}));
  EXPECT_STREQ("`xfield`=JSON_SET(`xfield`,'$.first',1,'$.second','two')",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_table_operation_item_set_twice_but_different) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "yfield"), Scalar("two")}}));
  EXPECT_STREQ(
      "`xfield`=JSON_SET(`xfield`,'$.first',1),"
      "`yfield`=JSON_SET(`yfield`,'$.second','two')",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_triple) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"third"}, "xfield"), Scalar(-3)}}));
  EXPECT_STREQ(
      "`xfield`=JSON_SET(`xfield`,'$.first',1,'$.second','two','$.third',-3)",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_mix_first) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::SET, Column_identifier("xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"third"}, "xfield"), Scalar(-3)}}));
  EXPECT_STREQ(
      "`xfield`=1,"
      "`xfield`=JSON_SET(`xfield`,'$.second','two','$.third',-3)",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_mix_last) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"third"}, "xfield"), Scalar(-3)},
      {Update_operation::Base::SET, Column_identifier("xfield"),
       Scalar(1.0)}}));
  EXPECT_STREQ(
      "`xfield`=JSON_SET(`xfield`,'$.second','two','$.third',-3),"
      "`xfield`=1",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_mix_middle) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")},
      {Update_operation::Base::SET, Column_identifier("xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"third"}, "xfield"), Scalar(-3)}}));
  EXPECT_STREQ(
      "`xfield`=JSON_SET(`xfield`,'$.second','two'),"
      "`xfield`=1,"
      "`xfield`=JSON_SET(`xfield`,'$.third',-3)",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_set_fourth) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"first"}, "yfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_SET,
       Column_identifier(Document_path{"second"}, "yfield"), Scalar("two")}}));
  EXPECT_STREQ(
      "`xfield`=JSON_SET(`xfield`,'$.first',1,'$.second','two'),"
      "`yfield`=JSON_SET(`yfield`,'$.first',1,'$.second','two')",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_remove_one) {
  ASSERT_NO_THROW(builder().add_table_operation(
      Operation_list{{Update_operation::Base::ITEM_REMOVE,
                      Column_identifier(Document_path{"first"}, "xfield")}}));
  EXPECT_STREQ("`xfield`=JSON_REMOVE(`xfield`,'$.first')", query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_remove_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(
      Operation_list{{Update_operation::Base::ITEM_REMOVE,
                      Column_identifier(Document_path{"first"}, "xfield")},
                     {Update_operation::Base::ITEM_REMOVE,
                      Column_identifier(Document_path{"second"}, "xfield")}}));
  EXPECT_STREQ("`xfield`=JSON_REMOVE(`xfield`,'$.first','$.second')",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_replace_one) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_REPLACE,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)}}));
  EXPECT_STREQ("`xfield`=JSON_REPLACE(`xfield`,'$.first',1)",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_item_replace_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ITEM_REPLACE,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ITEM_REPLACE,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")}}));
  EXPECT_STREQ("`xfield`=JSON_REPLACE(`xfield`,'$.first',1,'$.second','two')",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_array_insert_one) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ARRAY_INSERT,
       Column_identifier(Document_path{0}, "xfield"), Scalar(1.0)}}));
  EXPECT_STREQ("`xfield`=JSON_ARRAY_INSERT(`xfield`,'$[0]',1)",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_array_insert_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ARRAY_INSERT,
       Column_identifier(Document_path{0}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ARRAY_INSERT,
       Column_identifier(Document_path{1}, "xfield"), Scalar("two")}}));
  EXPECT_STREQ("`xfield`=JSON_ARRAY_INSERT(`xfield`,'$[0]',1,'$[1]','two')",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_array_append_one) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ARRAY_APPEND,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)}}));
  EXPECT_STREQ("`xfield`=JSON_ARRAY_APPEND(`xfield`,'$.first',1)",
               query.get().c_str());
}

TEST_F(Update_statement_builder_test, add_table_operation_array_append_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ARRAY_APPEND,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ARRAY_APPEND,
       Column_identifier(Document_path{"second"}, "xfield"), Scalar("two")}}));
  EXPECT_STREQ(
      "`xfield`=JSON_ARRAY_APPEND(`xfield`,'$.first',1,'$.second','two')",
      query.get().c_str());
}

TEST_F(Update_statement_builder_test,
       add_table_operation_array_append_twice_placeholder) {
  args = Expression_list{2.2};
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {Update_operation::Base::ARRAY_APPEND,
       Column_identifier(Document_path{"first"}, "xfield"), Scalar(1.0)},
      {Update_operation::Base::ARRAY_APPEND,
       Column_identifier(Document_path{"second"}, "xfield"), Placeholder(0)}}));
  EXPECT_STREQ(
      "`xfield`=JSON_ARRAY_APPEND(`xfield`,'$.first',1,'$.second',2.2)",
      query.get().c_str());
}

class Merge_param {
 public:
  Merge_param(const Update_operation::Update_type type,
              const std::string &function)
      : m_type(type), m_function(function) {}

  const Update_operation::Update_type m_type;
  const std::string m_function;
};

class Update_statement_builder_op_merge_test
    : public Update_statement_builder_test,
      public ::testing::WithParamInterface<Merge_param> {};

TEST_P(Update_statement_builder_op_merge_test,
       add_document_operation_merge_twice) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {GetParam().m_type, Document_path{""}, Scalar("{\"two\": 2.0}")},
      {GetParam().m_type, Document_path{""}, Scalar("{\"three\": 3.0}")}}));
  EXPECT_EQ("doc=JSON_SET(" + GetParam().m_function +
                "(doc,'{\\\"two\\\": 2.0}',"
                "'{\\\"three\\\": 3.0}'),'$._id',"
                "JSON_EXTRACT(`doc`,'$._id'))",
            query.get().c_str());
}

TEST_P(Update_statement_builder_op_merge_test,
       add_document_operation_set_merge) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {Update_operation::Base::ITEM_SET, Document_path{"first"}, Scalar(1.0)},
      {GetParam().m_type, Column_identifier(), Scalar("{\"three\": 3.0}")}}));
  EXPECT_EQ("doc=JSON_SET(" + GetParam().m_function +
                "(JSON_SET(doc,'$.first',1),"
                "'{\\\"three\\\": 3.0}'),'$._id',JSON_EXTRACT(`doc`,'$._id'))",
            query.get().c_str());
}

TEST_P(Update_statement_builder_op_merge_test, add_document_operation_merge) {
  EXPECT_NO_THROW(builder().add_document_operation(Operation_list{
      {GetParam().m_type, Document_path{""}, Scalar("{\"two\": 2.0}")}}));
  EXPECT_EQ("doc=JSON_SET(" + GetParam().m_function +
                "(doc,'{\\\"two\\\": 2.0}'),'$._id',"
                "JSON_EXTRACT(`doc`,'$._id'))",
            query.get().c_str());
}

TEST_P(Update_statement_builder_op_merge_test,
       add_table_operation_item_merge_twice) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {GetParam().m_type, Column_identifier(Document_path{"first"}, "xfield"),
       Scalar(1.0)},
      {GetParam().m_type, Column_identifier(Document_path{"second"}, "xfield"),
       Scalar("two")}}));
  EXPECT_EQ("`xfield`=" + GetParam().m_function + "(`xfield`,1,'two')",
            query.get().c_str());
}

TEST_P(Update_statement_builder_op_merge_test,
       add_table_operation_item_merge_one) {
  ASSERT_NO_THROW(builder().add_table_operation(Operation_list{
      {GetParam().m_type, Column_identifier(Document_path{"first"}, "xfield"),
       Scalar(1.0)}}));
  EXPECT_EQ("`xfield`=" + GetParam().m_function + "(`xfield`,1)",
            query.get().c_str());
}

INSTANTIATE_TEST_CASE_P(
    AllMerge_functions, Update_statement_builder_op_merge_test,
    ::testing::Values(
        Merge_param(Update_operation::Base::ITEM_MERGE, "JSON_MERGE_PRESERVE"),
        Merge_param(Update_operation::Base::MERGE_PATCH, "JSON_MERGE_PATCH")));

}  // namespace test
}  // namespace xpl
