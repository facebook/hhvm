/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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
#include "plugin/x/src/find_statement_builder.h"
#include "plugin/x/src/prepared_statement_builder.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

using Placeholder_list = Expression_generator::Prep_stmt_placeholder_list;

template <typename M>
struct Param_prepared_statement_builder {
  std::string expect;
  Placeholder_list expect_ids;
  M msg;
};

Param_prepared_statement_builder<Find> find_param[] = {
    {"SELECT JSON_OBJECT('zeta', ?) AS doc FROM `schema`.`col`",
     {0},
     Find({"col", "schema"}).projection({Placeholder{0}, "zeta"})},
    {"SELECT JSON_OBJECT('zeta', JSON_EXTRACT(doc,'$.alpha')) AS doc "
     "FROM `schema`.`col` WHERE (JSON_EXTRACT(doc,'$.delta') > ?)",
     {0},
     Find({"col", "schema"})
         .projection({Column_identifier{Document_path{"alpha"}}, "zeta"})
         .criteria(Operator(">", Column_identifier{Document_path{"delta"}},
                            Placeholder{0}))},
    {"SELECT JSON_OBJECT('zeta', JSON_EXTRACT(doc,'$.alpha')) AS doc "
     "FROM `schema`.`col` ORDER BY ?",
     {0},
     Find({"col", "schema"})
         .projection({Column_identifier{Document_path{"alpha"}}, "zeta"})
         .order({Placeholder{0}})},
    {"SELECT JSON_OBJECT('zeta', `_DERIVED_TABLE_`.`zeta`) AS doc FROM ("
     "SELECT JSON_EXTRACT(doc,'$.alpha') AS `zeta` "
     "FROM `schema`.`col` GROUP BY ?) AS `_DERIVED_TABLE_`",
     {0},
     Find({"col", "schema"})
         .projection({Column_identifier{Document_path{"alpha"}}, "zeta"})
         .grouping(Placeholder{0})},
    {"SELECT JSON_OBJECT('zeta', JSON_EXTRACT(doc,'$.alpha')) AS doc "
     "FROM `schema`.`col` HAVING (JSON_EXTRACT(doc,'$.delta') > ?)",
     {0},
     Find({"col", "schema"})
         .projection({Column_identifier{Document_path{"alpha"}}, "zeta"})
         .grouping_criteria(Operator(
             ">", Column_identifier{Document_path{"delta"}}, Placeholder{0}))},
};

Param_prepared_statement_builder<Delete> delete_param[] = {
    {"DELETE FROM `schema`.`col` WHERE (JSON_EXTRACT(doc,'$.delta') > ?)",
     {0},
     Delete({"col", "schema"})
         .criteria(Operator(">", Column_identifier{Document_path{"delta"}},
                            Placeholder{0}))},
    {"DELETE FROM `schema`.`col` ORDER BY ?",
     {0},
     Delete({"col", "schema"}).order({Placeholder{0}})},
};

Param_prepared_statement_builder<Update> update_param[] = {
    {"UPDATE `schema`.`col` "
     "SET doc=JSON_SET(JSON_SET(doc,'$.first',?),'$._id',"
     "JSON_EXTRACT(`doc`,'$._id'))",
     {0},
     Update({"col", "schema"})
         .operation({Update_operation::Base::ITEM_SET, Document_path{"first"},
                     Placeholder{0}})},
    {"UPDATE `schema`.`col` SET "
     "doc=JSON_SET(JSON_SET(doc,'$.first',1),'$._id',"
     "JSON_EXTRACT(`doc`,'$._id')) "
     "WHERE (JSON_EXTRACT(doc,'$.delta') > ?)",
     {0},
     Update({"col", "schema"})
         .operation({Update_operation::Base::ITEM_SET, Document_path{"first"},
                     Scalar{1.0}})
         .criteria(Operator(">", Column_identifier{Document_path{"delta"}},
                            Placeholder{0}))},
    {"UPDATE `schema`.`col` "
     "SET doc=JSON_SET(JSON_SET(doc,'$.first',1),'$._id',"
     "JSON_EXTRACT(`doc`,'$._id')) ORDER BY ?",
     {0},
     Update({"col", "schema"})
         .operation({Update_operation::Base::ITEM_SET, Document_path{"first"},
                     Scalar{1.0}})
         .order({Placeholder{0}})},
};

#define EXPECT_VALUE(val)                                                    \
  "(SELECT JSON_INSERT(`_DERIVED_TABLE_`.`value`,'$._id',"                   \
  "CONVERT(MYSQLX_GENERATE_DOCUMENT_ID(@@AUTO_INCREMENT_OFFSET,"             \
  "@@AUTO_INCREMENT_INCREMENT,JSON_CONTAINS_PATH(`_DERIVED_TABLE_`.`value`," \
  "'one','$._id')) USING utf8mb4)) "                                         \
  "FROM (SELECT " val " AS `value`) AS `_DERIVED_TABLE_`)"

Param_prepared_statement_builder<Insert> insert_param[] = {
    {"INSERT INTO `schema`.`col` (doc) VALUES (" EXPECT_VALUE(
         "'{\\\"three\\\":3}'") ")",
     {},
     Insert({"col", "schema"})
         .args({Scalar(R"({"three":3})")})
         .row({{Placeholder{0}}})},
    {"INSERT INTO `schema`.`col` (doc) VALUES (" EXPECT_VALUE("?") ")",
     {0},
     Insert({"col", "schema"}).row({{Placeholder{0}}})},
    {"INSERT INTO `schema`.`col` (doc) VALUES (" EXPECT_VALUE(
         "'{\\\"three\\\":3}'") "),(" EXPECT_VALUE("?") ")",
     {0},
     Insert({"col", "schema"})
         .args({Scalar(R"({"three":3})")})
         .row({{Placeholder{0}}, {Placeholder{1}}})},
    {"INSERT INTO `schema`.`col` (doc) VALUES (" EXPECT_VALUE(
         "?") "),(" EXPECT_VALUE("?") ")",
     {0, 1},
     Insert({"col", "schema"}).row({{Placeholder{0}}, {Placeholder{1}}})},
};

Param_prepared_statement_builder<Stmt_execute> stmt_execute_param[] = {
    {"SELECT 1", {}, Stmt_execute("SELECT 1")},
    {"SELECT ?", {0}, Stmt_execute("SELECT ?")},
    {"SELECT 1", {}, Stmt_execute("SELECT ?").args({Scalar(1)})},
    {"SELECT 1, ?", {0}, Stmt_execute("SELECT ?, ?").args({Scalar(1)})},
};

template <typename M>
class Prepared_statement_builder_test
    : public testing::TestWithParam<Param_prepared_statement_builder<M>> {
 public:
  using Param = Param_prepared_statement_builder<M>;
  Query_string_builder query;
  Expression_generator::Prep_stmt_placeholder_list placeholders;
  Prepared_statement_builder builder{&query, &placeholders};
  void assert_placeholders() {
    const Param &param = this->GetParam();
    ASSERT_ERROR_CODE(ER_X_SUCCESS, builder.build(param.msg));
    EXPECT_STREQ(param.expect.c_str(), query.get().c_str());
    EXPECT_EQ(param.expect_ids, placeholders);
  }
};

using Find_test = Prepared_statement_builder_test<Find>;
TEST_P(Find_test, placeholders) { assert_placeholders(); }
INSTANTIATE_TEST_CASE_P(Prepared_statement_builder, Find_test,
                        testing::ValuesIn(find_param));

using Delete_test = Prepared_statement_builder_test<Delete>;
TEST_P(Delete_test, placeholders) { assert_placeholders(); }
INSTANTIATE_TEST_CASE_P(Prepared_statement_builder, Delete_test,
                        testing::ValuesIn(delete_param));

using Update_test = Prepared_statement_builder_test<Update>;
TEST_P(Update_test, placeholders) { assert_placeholders(); }
INSTANTIATE_TEST_CASE_P(Prepared_statement_builder, Update_test,
                        testing::ValuesIn(update_param));

using Insert_test = Prepared_statement_builder_test<Insert>;
TEST_P(Insert_test, placeholders) { assert_placeholders(); }
INSTANTIATE_TEST_CASE_P(Prepared_statement_builder, Insert_test,
                        testing::ValuesIn(insert_param));

using Stmt_execute_test = Prepared_statement_builder_test<Stmt_execute>;
TEST_P(Stmt_execute_test, placeholders) { assert_placeholders(); }
INSTANTIATE_TEST_CASE_P(Prepared_statement_builder, Stmt_execute_test,
                        testing::ValuesIn(stmt_execute_param));
}  // namespace test
}  // namespace xpl
