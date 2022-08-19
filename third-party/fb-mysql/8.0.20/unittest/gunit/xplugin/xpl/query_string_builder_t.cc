/*
 * Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.
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
#include <list>
#include <stdexcept>

#include "plugin/x/ngs/include/ngs/error_code.h"
#include "plugin/x/src/query_string_builder.h"

namespace xpl {

namespace test {

namespace {

class Assign_list {
 public:
  typedef std::string Value_type;
  typedef std::list<Value_type> Container_type;

  Assign_list(const Value_type &value) { (*this)(value); }

  Assign_list &operator()(const Value_type &value) {
    m_values.push_back(value);
    return *this;
  }

  operator Container_type() { return m_values; }

 private:
  Container_type m_values;
};

}  // namespace

class Query_string_builder_testsuite : public ::testing::Test {
 public:
  Query_string_builder query;
};

TEST_F(Query_string_builder_testsuite,
       format_doesNothing_whenEmptyStringAndNoArgsPushed) {
  query.format();

  ASSERT_STREQ("", query.get().c_str());
}

TEST_F(Query_string_builder_testsuite,
       format_doesThrowsException_whenEmptyFormat) {
  ASSERT_THROW(query.format() % "Test", ngs::Error_code);

  ASSERT_STREQ("", query.get().c_str());
}

TEST_F(Query_string_builder_testsuite,
       format_doesThrowsException_whenNoTagFormat) {
  const char *format_query = "QUERY WITHOUT TAG";

  query.put(format_query);
  ASSERT_THROW(query.format() % "Test", ngs::Error_code);

  ASSERT_STREQ(format_query, query.get().c_str());
}

TEST_F(Query_string_builder_testsuite,
       format_doesThrowsException_whenAfterFillingFirstTag) {
  query.put("SELECT ? FROM table").format() % "test";

  ASSERT_THROW(query.format() % "test1", ngs::Error_code);
  ASSERT_THROW(query.format() % "test2", ngs::Error_code);

  ASSERT_STREQ("SELECT 'test' FROM table", query.get().c_str());
}

TEST_F(Query_string_builder_testsuite,
       format_fillsTwoTagsInQueryAndSkipsTagInValue_whenTwoTagValue) {
  query.put("SELECT ? FROM ?");
  query.format() % "?" % "test";

  ASSERT_STREQ("SELECT '?' FROM 'test'", query.get().c_str());
}

TEST_F(Query_string_builder_testsuite, format_fillNumericValues) {
  query.put("SELECT *,? FROM t WHERE x=?").format() % 1 % 1.1;

  ASSERT_STREQ("SELECT *,1 FROM t WHERE x=1.1", query.get().c_str());
}

struct Query_and_expected {
  Query_and_expected(std::string query, std::string expected, std::string value)
      : m_query(query), m_expected(expected), m_value(value) {}

  std::string m_query;
  std::string m_expected;
  std::string m_value;
};

::std::ostream &operator<<(::std::ostream &os,
                           const Query_and_expected &query_and_expected) {
  return os << "Query:" << query_and_expected.m_query
            << " expected:" << query_and_expected.m_expected
            << " value:" << query_and_expected.m_value << std::endl;
}

class Query_string_builder_param_testsuite
    : public Query_string_builder_testsuite,
      public ::testing::WithParamInterface<Query_and_expected> {
 public:
};

TEST_P(Query_string_builder_param_testsuite,
       format_putStringValueInsideQuery_whenOneTagInFormat) {
  const char *value = GetParam().m_value.c_str();
  const char *format_query = GetParam().m_query.c_str();
  const char *expected_query = GetParam().m_expected.c_str();

  query.put(format_query).format() % value;

  ASSERT_STREQ(expected_query, query.get().c_str());
}

INSTANTIATE_TEST_CASE_P(
    InstantiationPositiveTest, Query_string_builder_param_testsuite,
    ::testing::Values(
        Query_and_expected("SELECT ? FROM", "SELECT 'Test' FROM", "Test"),
        Query_and_expected("SELECT Next,?", "SELECT Next,'FROM'", "FROM"),
        Query_and_expected("?,Back,FROM", "'Select',Back,FROM", "Select"),
        Query_and_expected("SELECT Next ?", "SELECT Next ''", ""),
        Query_and_expected("SELECT ? From", "SELECT '' From", ""),
        Query_and_expected("? From", "'' From", ""),
        Query_and_expected("? From",
                           "'\\'first-word\\' ; \\\"second-word\\\"' From",
                           "'first-word' ; \"second-word\"")));

struct Query_and_expected_values {
  Query_and_expected_values(std::string query, std::string expected,
                            std::list<std::string> values)
      : m_query(query), m_expected(expected), m_values(values) {}

  std::string m_query;
  std::string m_expected;
  std::list<std::string> m_values;
};

::std::ostream &operator<<(
    ::std::ostream &os, const Query_and_expected_values &query_and_expected) {
  os << "Query:" << query_and_expected.m_query
     << " expected:" << query_and_expected.m_expected << std::endl
     << " [";

  std::ostream_iterator<std::string> out_it(os, ", ");
  std::copy(query_and_expected.m_values.begin(),
            query_and_expected.m_values.end(), out_it);

  return os << "]" << std::endl;
}

class Query_string_builder_multiple_tags_param_testsuite
    : public Query_string_builder_testsuite,
      public ::testing::WithParamInterface<Query_and_expected_values> {
 public:
  void SetUp() {
    values = GetParam().m_values;
    expected_query = &GetParam().m_expected[0];

    query.put(GetParam().m_query.c_str());
  }

  std::list<std::string> values;
  std::string expected_query;
};

TEST_P(Query_string_builder_multiple_tags_param_testsuite,
       format_putStringValueInsideQuery_whenMultipleTagsInFormat) {
  std::list<std::string>::const_iterator i = values.begin();

  // In each iteration format creates new object without previous state
  // Thus similar test are needed with one call/multiple args
  for (; i != values.end(); ++i) query.format() % *i;

  ASSERT_STREQ(expected_query.c_str(), query.get().c_str());
}

INSTANTIATE_TEST_CASE_P(
    InstantiationPositiveTests,
    Query_string_builder_multiple_tags_param_testsuite,
    ::testing::Values(
        Query_and_expected_values("SELECT ?,?,? FROM",
                                  "SELECT 'First','Second','Third' FROM",
                                  Assign_list("First")("Second")("Third")),
        Query_and_expected_values("SELECT ?,? FROM",
                                  "SELECT 'First','Second' FROM",
                                  Assign_list("First")("Second")),
        Query_and_expected_values("SELECT ? FROM ?",
                                  "SELECT 'Second' FROM 'First'",
                                  Assign_list("Second")("First")),
        Query_and_expected_values("SELECT ?? FROM t",
                                  "SELECT 'Second''First' FROM t",
                                  Assign_list("Second")("First")),
        Query_and_expected_values("SELECT ? FROM ?", "SELECT '?' FROM 'First'",
                                  Assign_list("?")("First"))));

INSTANTIATE_TEST_CASE_P(
    InstantiationPositiveTestsQueryOrValuesWithEscapedChars,
    Query_string_builder_multiple_tags_param_testsuite,
    ::testing::Values(
        Query_and_expected_values("SELECT ?/*,?*/ FROM t WHERE ?",
                                  "SELECT '\\\"'/*,?*/ FROM t WHERE 'First'",
                                  Assign_list("\"")("First")),
        Query_and_expected_values("SELECT ?,? FROM t",
                                  "SELECT '\\'','First' FROM t",
                                  Assign_list("'")("First")),
        Query_and_expected_values("SELECT ?,? FROM t",
                                  "SELECT '\\\"','First' FROM t",
                                  Assign_list("\"")("First")),
        Query_and_expected_values("SELECT \"?\'?'?\",?,? FROM t",
                                  "SELECT \"?\'?'?\",'','First' FROM t",
                                  Assign_list("")("First"))));

INSTANTIATE_TEST_CASE_P(
    InstantiationPositiveTestsQueryWithComment,
    Query_string_builder_multiple_tags_param_testsuite,
    ::testing::Values(
        Query_and_expected_values("SELECT ?/*,?*/ FROM t WHERE ?",
                                  "SELECT '\\\"'/*,?*/ FROM t WHERE 'First'",
                                  Assign_list("\"")("First")),
        Query_and_expected_values("SELECT ?#,?*\n FROM t WHERE ?",
                                  "SELECT '\\\"'#,?*\n FROM t WHERE 'First'",
                                  Assign_list("\"")("First")),
        Query_and_expected_values("SELECT ?-- ,?*\n FROM t WHERE ?",
                                  "SELECT '\\\"'-- ,?*\n FROM t WHERE 'First'",
                                  Assign_list("\"")("First")),
        Query_and_expected_values(
            "SELECT ?-- ,?*\n FROM -- ?\nt WHERE ?",
            "SELECT '\\\"'-- ,?*\n FROM -- ?\nt WHERE 'First'",
            Assign_list("\"")("First")),
        Query_and_expected_values(
            "SELECT ?-- ,?*\n FROM # ?\nt WHERE ?",
            "SELECT '\\\"'-- ,?*\n FROM # ?\nt WHERE 'First'",
            Assign_list("\"")("First")),
        Query_and_expected_values(
            "SELECT ?-- ,?*\n FROM /* ?*/t WHERE ?",
            "SELECT '\\\"'-- ,?*\n FROM /* ?*/t WHERE 'First'",
            Assign_list("\"")("First"))));

class Query_string_builder_multiple_too_many_tags_param_testsuite
    : public Query_string_builder_multiple_tags_param_testsuite {};

TEST_P(Query_string_builder_multiple_too_many_tags_param_testsuite,
       format_putStringValueInsideQuery_whenMultipleTagsInFormat) {
  std::list<std::string>::const_iterator i = values.begin();

  ASSERT_LT(0u, values.size());

  // In each iteration format creates new object without previous state
  // Thus similar test are needed with one call/multiple args
  for (; &*i != &values.back(); ++i) query.format() % *i;

  ASSERT_THROW(query.format() % values.back(), ngs::Error_code);
}

INSTANTIATE_TEST_CASE_P(
    InstantiationNegativeTest,
    Query_string_builder_multiple_too_many_tags_param_testsuite,
    ::testing::Values(
        Query_and_expected_values("SELECT ?,? FROM", "",
                                  Assign_list("First")("Second")("Third")),
        Query_and_expected_values("SELECT ? FROM", "",
                                  Assign_list("First")("Second"))));

}  // namespace test

}  // namespace xpl
