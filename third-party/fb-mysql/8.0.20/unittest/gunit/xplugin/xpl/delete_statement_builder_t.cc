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

#include "plugin/x/src/delete_statement_builder.h"
#include "plugin/x/src/expr_generator.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

class Delete_statement_builder_test : public ::testing::Test {
 public:
  Delete_statement_builder_test()
      : args(*msg.mutable_args()),
        expr_gen(&query, args, schema, true),
        builder(expr_gen) {}
  Delete_statement_builder::Delete msg;
  Expression_generator::Arg_list &args;
  Query_string_builder query;
  std::string schema;
  Expression_generator expr_gen;
  Delete_statement_builder builder;
};

TEST_F(Delete_statement_builder_test, build_table) {
  msg = Delete({"xtable", "xschema"}, Mysqlx::Crud::TABLE)
            .criteria(Operator(">", Column_identifier{"delta"}, Scalar{1.0}))
            .order({Column_identifier("gamma"), Order::Base::DESC})
            .limit({2});
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ(
      "DELETE FROM `xschema`.`xtable` "
      "WHERE (`delta` > 1) "
      "ORDER BY `gamma` DESC "
      "LIMIT 2",
      query.get().c_str());
}

TEST_F(Delete_statement_builder_test, build_document) {
  msg =
      Delete({"xcoll", "xschema"}, Mysqlx::Crud::DOCUMENT)
          .criteria(Operator(">", Column_identifier{Document_path{"delta"}},
                             Scalar{1.0}))
          .order({Column_identifier{Document_path{"gamma"}}, Order::Base::DESC})
          .limit({2});
  ASSERT_NO_THROW(builder.build(msg));
  EXPECT_STREQ(
      "DELETE FROM `xschema`.`xcoll` "
      "WHERE (JSON_EXTRACT(doc,'$.delta') > 1) "
      "ORDER BY JSON_EXTRACT(doc,'$.gamma') DESC "
      "LIMIT 2",
      query.get().c_str());
}

}  // namespace test
}  // namespace xpl
