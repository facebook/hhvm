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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/src/sql_statement_builder.h"

#include "plugin/x/ngs/include/ngs/error_code.h"
#include "plugin/x/src/query_string_builder.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

using Placeholder_list = Sql_statement_builder::Placeholder_list;

template <typename T>
class Sql_statement_builder_test_base : public testing::TestWithParam<T> {
 public:
  Query_string_builder m_qb;
  Sql_statement_builder m_builder{&m_qb};
};

struct Param_sql_statement_builder {
  std::string expect_query;
  std::string query;
  Any_list args;
};

Param_sql_statement_builder sql_statement_builder_success_param[] = {
    {"", "", {}},
    {"?", "?", {}},
    {"abc", "abc", {}},
    {"1", "?", {Scalar(1)}},
    {"'abc'", "?", {Scalar::String("abc")}},
    {"'efg'", "?", {Scalar::Octets("efg")}},
    {"3.14", "?", {Scalar(3.14f)}},
    {"2.7182", "?", {Scalar(2.7182)}},
    {"2, 3, 1", "?, ?, ?", {Scalar(2), Scalar(3), Scalar(1)}},
    {"2, 3, ?", "?, ?, ?", {Scalar(2), Scalar(3)}},
};

using Sql_statement_builder_success_test =
    Sql_statement_builder_test_base<Param_sql_statement_builder>;

TEST_P(Sql_statement_builder_success_test, build_query_success) {
  const ParamType &param = GetParam();
  ASSERT_NO_THROW(m_builder.build(param.query, param.args));
  EXPECT_STREQ(param.expect_query.c_str(), m_qb.get().c_str());
}

INSTANTIATE_TEST_CASE_P(Sql_statement_builder,
                        Sql_statement_builder_success_test,
                        testing::ValuesIn(sql_statement_builder_success_param));

struct Param_sql_statement_builder_fail {
  std::string query;
  Any_list args;
};

Param_sql_statement_builder_fail sql_statement_builder_fail_param[] = {
    {"", {Scalar(1)}},
    {"?", {Scalar(2), Scalar(3)}},
    {"?", {Any::Array{1, 2, 3}}},
    {"?", {Any::Object{{"a", 1}}}},
};

using Sql_statement_builder_fail_test =
    Sql_statement_builder_test_base<Param_sql_statement_builder_fail>;

TEST_P(Sql_statement_builder_fail_test, build_query_fail) {
  const ParamType &param = GetParam();
  ASSERT_THROW(m_builder.build(param.query, param.args), ngs::Error_code);
}

INSTANTIATE_TEST_CASE_P(Sql_statement_builder, Sql_statement_builder_fail_test,
                        testing::ValuesIn(sql_statement_builder_fail_param));

struct Param_sql_statement_builder_placeholders {
  std::string expect_query;
  Placeholder_list expect_placeholders;
  std::string query;
  Any_list args;
};

Param_sql_statement_builder_placeholders
    sql_statement_builder_placeholders_param[] = {
        {"", {}, "", {}},
        {"?", {0}, "?", {}},
        {"abc", {}, "abc", {}},
        {"1", {}, "?", {Scalar(1)}},
        {"'abc'", {}, "?", {Scalar::String("abc")}},
        {"'efg'", {}, "?", {Scalar::Octets("efg")}},
        {"3.14", {}, "?", {Scalar(3.14f)}},
        {"2.7182", {}, "?", {Scalar(2.7182)}},
        {"2, 3, 1", {}, "?, ?, ?", {Scalar(2), Scalar(3), Scalar(1)}},
        {"2, 3, ?", {0}, "?, ?, ?", {Scalar(2), Scalar(3)}},
        {"?, ?, ?", {0, 1, 2}, "?, ?, ?", {}},
};

using Sql_statement_builder_placeholders_test =
    Sql_statement_builder_test_base<Param_sql_statement_builder_placeholders>;

TEST_P(Sql_statement_builder_placeholders_test,
       build_query_placeholders_success) {
  const ParamType &param = GetParam();
  Placeholder_list phs;
  ASSERT_NO_THROW(m_builder.build(param.query, param.args, &phs));
  EXPECT_STREQ(param.expect_query.c_str(), m_qb.get().c_str());
  EXPECT_EQ(param.expect_placeholders, phs);
}

INSTANTIATE_TEST_CASE_P(
    Sql_statement_builder_placeholders, Sql_statement_builder_placeholders_test,
    testing::ValuesIn(sql_statement_builder_placeholders_param));

}  // namespace test
}  // namespace xpl
