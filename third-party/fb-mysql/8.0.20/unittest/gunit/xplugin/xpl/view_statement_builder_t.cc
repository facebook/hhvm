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

#include "plugin/x/src/view_statement_builder.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

class View_statement_builder_impl : public View_statement_builder {
 public:
  explicit View_statement_builder_impl(const Expression_generator &gen)
      : View_statement_builder(gen) {}

  using View_statement_builder::add_algorithm;
  using View_statement_builder::add_check_option;
  using View_statement_builder::add_columns;
  using View_statement_builder::add_definer;
  using View_statement_builder::add_sql_security;
};

class View_statement_builder_test : public ::testing::Test {
 public:
  View_statement_builder_test()
      : expr_gen(&query, args, schema, true), builder(expr_gen) {
    *find.mutable_collection() = Collection("A");
  }

  Expression_generator::Arg_list args;
  Query_string_builder query;
  std::string schema;
  Expression_generator expr_gen;
  View_statement_builder_impl builder;
  Find_statement_builder::Find find;

  using View_delete = View_statement_builder_impl::View_drop;
  using View_create = View_statement_builder_impl::View_create;
  using View_modify = View_statement_builder_impl::View_modify;
  using Column_list = Repeated_field_list<std::string, std::string>;
};

TEST_F(View_statement_builder_test, add_definer_empty) {
  std::string definer;
  ASSERT_NO_THROW(builder.add_definer(definer));
  EXPECT_STREQ("", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_definer) {
  std::string definer = "user";
  ASSERT_NO_THROW(builder.add_definer(definer));
  EXPECT_STREQ("DEFINER='user' ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_definer_with_domain) {
  std::string definer = "user@localhost";
  ASSERT_NO_THROW(builder.add_definer(definer));
  EXPECT_STREQ("DEFINER='user'@'localhost' ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_algorithm_undefined) {
  ASSERT_NO_THROW(builder.add_algorithm(Mysqlx::Crud::UNDEFINED));
  EXPECT_STREQ("ALGORITHM=UNDEFINED ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_algorithm_temptable) {
  ASSERT_NO_THROW(builder.add_algorithm(Mysqlx::Crud::TEMPTABLE));
  EXPECT_STREQ("ALGORITHM=TEMPTABLE ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_algorithm_merge) {
  ASSERT_NO_THROW(builder.add_algorithm(Mysqlx::Crud::MERGE));
  EXPECT_STREQ("ALGORITHM=MERGE ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_sql_security_definer) {
  ASSERT_NO_THROW(builder.add_sql_security(Mysqlx::Crud::DEFINER));
  EXPECT_STREQ("SQL SECURITY DEFINER ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_sql_security_invoker) {
  ASSERT_NO_THROW(builder.add_sql_security(Mysqlx::Crud::INVOKER));
  EXPECT_STREQ("SQL SECURITY INVOKER ", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_check_option_cascaded) {
  ASSERT_NO_THROW(builder.add_check_option(Mysqlx::Crud::CASCADED));
  EXPECT_STREQ(" WITH CASCADED CHECK OPTION", query.get().c_str());
}

TEST_F(View_statement_builder_test, add_check_option_local) {
  ASSERT_NO_THROW(builder.add_check_option(Mysqlx::Crud::LOCAL));
  EXPECT_STREQ(" WITH LOCAL CHECK OPTION", query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_two_columns) {
  ASSERT_NO_THROW(builder.add_columns(Column_list{"one", "two", "three"}));
  EXPECT_STREQ(" (`one`,`two`,`three`)", query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_simple) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_or_replace) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  msg.set_replace_existing(true);
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE OR REPLACE VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_no_replace) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  msg.set_replace_existing(false);
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_column) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_column() = Column_list{"one"};
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE VIEW `xview` (`one`) AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_definer) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_definer("User");
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE DEFINER='User' VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_algorithm) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_algorithm(Mysqlx::Crud::MERGE);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("CREATE ALGORITHM=MERGE VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_seciurity) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_security(Mysqlx::Crud::INVOKER);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ(
      "CREATE SQL SECURITY INVOKER VIEW `xview` AS SELECT doc FROM `A`",
      query.get().c_str());
}

TEST_F(View_statement_builder_test, build_create_view_check) {
  View_create msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_check(Mysqlx::Crud::CASCADED);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ(
      "CREATE VIEW `xview` AS SELECT doc FROM `A` WITH CASCADED CHECK OPTION",
      query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_simple) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("ALTER VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_column) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_column() = Column_list{"one"};
  *msg.mutable_stmt() = find;
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("ALTER VIEW `xview` (`one`) AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_definer) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_definer("User");
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("ALTER DEFINER='User' VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_algorithm) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_algorithm(Mysqlx::Crud::MERGE);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("ALTER ALGORITHM=MERGE VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_seciurity) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_security(Mysqlx::Crud::INVOKER);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("ALTER SQL SECURITY INVOKER VIEW `xview` AS SELECT doc FROM `A`",
               query.get().c_str());
}

TEST_F(View_statement_builder_test, build_modify_view_check) {
  View_modify msg;
  *msg.mutable_collection() = Collection("xview");
  *msg.mutable_stmt() = find;
  msg.set_check(Mysqlx::Crud::CASCADED);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ(
      "ALTER VIEW `xview` AS SELECT doc FROM `A` WITH CASCADED CHECK OPTION",
      query.get().c_str());
}

TEST_F(View_statement_builder_test, build_delete_view) {
  View_delete msg;
  *msg.mutable_collection() = Collection("xview");
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("DROP VIEW `xview`", query.get().c_str());
}

TEST_F(View_statement_builder_test, build_delete_view_verify_object) {
  View_delete msg;
  *msg.mutable_collection() = Collection("xview");
  msg.set_if_exists(true);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("DROP VIEW IF EXISTS `xview`", query.get().c_str());
}

TEST_F(View_statement_builder_test, build_delete_view_no_verify_object) {
  View_delete msg;
  *msg.mutable_collection() = Collection("xview");
  msg.set_if_exists(false);
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ("DROP VIEW `xview`", query.get().c_str());
}
}  // namespace test
}  // namespace xpl
