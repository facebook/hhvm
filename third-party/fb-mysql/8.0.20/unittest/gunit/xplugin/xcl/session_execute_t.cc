/*
 * Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include "my_compiler.h"

#include "unittest/gunit/xplugin/xcl/message_helpers.h"
#include "unittest/gunit/xplugin/xcl/mock/query_result.h"
#include "unittest/gunit/xplugin/xcl/session_t.h"

namespace xcl {
namespace test {
namespace {

const char *expected_sql = "SELECT 1;";
const int expected_error_code = 2003;

}  // namespace

template <bool connected>
class Xcl_session_impl_tests_execute : public Xcl_session_impl_tests {
 public:
  using StmtExecute = Mysqlx::Sql::StmtExecute;

 public:
  void SetUp() override { m_sut = std::move(make_sut(connected)); }
};

using Xcl_session_impl_tests_execute_connected =
    Xcl_session_impl_tests_execute<true>;
using Xcl_session_impl_tests_execute_disconnected =
    Xcl_session_impl_tests_execute<false>;
using XQuery_result_ptr = XQuery_result *;

TEST_F(Xcl_session_impl_tests_execute_disconnected, execute_sql_not_usable) {
  XError out_error;

  auto query_result = m_sut->execute_sql(expected_sql, &out_error);

  ASSERT_FALSE(query_result);
  ASSERT_EQ(CR_CONNECTION_ERROR, out_error.error());
}

TEST_F(Xcl_session_impl_tests_execute_connected, execute_sql_fails) {
  XError out_error;
  EXPECT_CALL(*m_mock_protocol,
              execute_stmt_raw(Cmp_msg("stmt: \"SELECT 1;\""), _))
      .WillOnce(Invoke([](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                          XError *out_r) -> XQuery_result_ptr {
        *out_r = XError{expected_error_code, ""};
        return {};
      }));

  auto query_result = m_sut->execute_sql(expected_sql, &out_error);
  ASSERT_FALSE(query_result);
  ASSERT_EQ(expected_error_code, out_error.error());

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_disconnected, execute_stmt_not_usable) {
  XError out_error;

  auto query_result = m_sut->execute_stmt("", expected_sql, {}, &out_error);

  ASSERT_FALSE(query_result);
  ASSERT_EQ(CR_CONNECTION_ERROR, out_error.error());
}

TEST_F(Xcl_session_impl_tests_execute_connected, execute_stmt_fails) {
  XError out_error;
  EXPECT_CALL(
      *m_mock_protocol,
      execute_stmt_raw(
          Cmp_msg("stmt: \"SELECT 1;\" namespace: \"our-namespace\""), _))
      .WillOnce(Invoke([](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                          XError *out_r) -> XQuery_result_ptr {
        *out_r = XError{expected_error_code, ""};
        return {};
      }));

  auto query_result =
      m_sut->execute_stmt("our-namespace", expected_sql, {}, &out_error);

  ASSERT_FALSE(query_result);
  ASSERT_EQ(expected_error_code, out_error.error());

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_connected, execute_stmt_no_args) {
  XError out_error;
  std::unique_ptr<XQuery_result> dummy_result{new Mock_query_result};
  auto dummy_result_ptr = dummy_result.get();

  EXPECT_CALL(
      *m_mock_protocol,
      execute_stmt_raw(
          Cmp_msg("stmt: \"SELECT 1;\" namespace: \"our-namespace\""), _))
      .WillOnce(
          Invoke([&dummy_result](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                                 XError *out_r) -> XQuery_result_ptr {
            *out_r = {};
            return dummy_result.release();
          }));

  auto query_result =
      m_sut->execute_stmt("our-namespace", expected_sql, {}, &out_error);

  ASSERT_EQ(dummy_result_ptr, query_result.get());
  ASSERT_FALSE(out_error);

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_connected, execute_stmt_args_one_scalar) {
  XError out_error;
  const int64_t argument_value = 1002;
  std::unique_ptr<XQuery_result> dummy_result{new Mock_query_result};
  auto dummy_result_ptr = dummy_result.get();

  EXPECT_CALL(*m_mock_protocol,
              execute_stmt_raw(Cmp_msg("stmt: \"SELECT 1;\" "
                                       "namespace: \"s\" "
                                       "args { type:SCALAR scalar {"
                                       "   type: V_SINT "
                                       "   v_signed_int: 1002} }"),
                               _))
      .WillOnce(
          Invoke([&dummy_result](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                                 XError *out_r) -> XQuery_result_ptr {
            *out_r = {};
            return dummy_result.release();
          }));

  auto query_result = m_sut->execute_stmt(
      "s", expected_sql, {Argument_value(argument_value)}, &out_error);

  ASSERT_EQ(dummy_result_ptr, query_result.get());
  ASSERT_FALSE(out_error);

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_connected,
       execute_stmt_args_all_scalars) {
  XError out_error;
  const int64_t argument_value_int = 1002;
  const uint64_t argument_value_uint = 1003;
  const bool argument_value_bool = true;
  const double argument_value_double = 20.1;
  const float argument_value_float = 10.1f;
  const std::string argument_value_string = "str1";
  const std::string argument_value_octet = "octet2";
  const std::string argument_value_decimal = "decimal3";

  std::unique_ptr<XQuery_result> dummy_result{new Mock_query_result};
  auto dummy_result_ptr = dummy_result.get();

  EXPECT_CALL(*m_mock_protocol,
              execute_stmt_raw(Cmp_msg("stmt: \"SELECT 1;\" "
                                       "namespace: \"s\" "
                                       "args { type:SCALAR scalar {"
                                       "   type: V_SINT "
                                       "   v_signed_int: 1002} }"
                                       "args { type:SCALAR scalar {"
                                       "   type: V_UINT "
                                       "   v_unsigned_int: 1003} }"
                                       "args { type:SCALAR scalar {"
                                       "   type: V_BOOL "
                                       "   v_bool: true} }"
                                       "args { type:SCALAR scalar {"
                                       "   type: V_DOUBLE "
                                       "   v_double: 20.1} }"
                                       "args { type:SCALAR scalar {"
                                       "   type: V_FLOAT "
                                       "   v_float: 10.1} }"
                                       "args { type:SCALAR scalar {"
                                       "   type: V_STRING "
                                       "   v_string { "
                                       "      value: \"str1\" }}} "
                                       "args { type:SCALAR scalar {"
                                       "   type: V_OCTETS "
                                       "   v_octets { "
                                       "      value: \"octet2\" }}} "
                                       "args { type:SCALAR scalar {"
                                       "   type: V_STRING "
                                       "   v_string { "
                                       "      value: \"decimal3\" }}} "
                                       "args { type:SCALAR scalar {"
                                       "   type: V_NULL } }"),
                               _))
      .WillOnce(
          Invoke([&dummy_result](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                                 XError *out_r) -> XQuery_result_ptr {
            *out_r = {};
            return dummy_result.release();
          }));

  auto query_result = m_sut->execute_stmt(
      "s", expected_sql,
      {Argument_value(argument_value_int), Argument_value(argument_value_uint),
       Argument_value(argument_value_bool),
       Argument_value(argument_value_double),
       Argument_value(argument_value_float),
       Argument_value(argument_value_string,
                      Argument_value::String_type::k_string),
       Argument_value(argument_value_octet,
                      Argument_value::String_type::k_octets),
       Argument_value(argument_value_decimal,
                      Argument_value::String_type::k_decimal),
       Argument_value()},
      &out_error);

  ASSERT_EQ(dummy_result_ptr, query_result.get());
  ASSERT_FALSE(out_error);

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_connected,
       execute_stmt_args_object_scalars) {
  XError out_error;
  const Argument_value argument_value_int{static_cast<uint64_t>(1002)};
  const Argument_value argument_value_double{20.1};
  const Argument_value argument_value_string{"str1"};

  std::unique_ptr<XQuery_result> dummy_result{new Mock_query_result};
  auto dummy_result_ptr = dummy_result.get();

  EXPECT_CALL(*m_mock_protocol,
              execute_stmt_raw(Cmp_msg("stmt: \"SELECT 1;\" "
                                       "namespace: \"s\" "
                                       "args { type:OBJECT obj { "
                                       "       fld { "
                                       "         key: \"key1\" "
                                       "         value { "
                                       "           type: SCALAR scalar { "
                                       "             type: V_UINT "
                                       "             v_unsigned_int: 1002 "
                                       "       } } } "
                                       "       fld { "
                                       "         key: \"key2\" "
                                       "         value { "
                                       "           type: SCALAR scalar { "
                                       "             type: V_DOUBLE "
                                       "             v_double: 20.1 "
                                       "       } } } "
                                       "       fld { "
                                       "         key: \"key3\" "
                                       "         value { "
                                       "           type: SCALAR scalar { "
                                       "             type: V_STRING "
                                       "             v_string { "
                                       "               value: \"str1\" } "
                                       "       } } } "
                                       "} }"),
                               _))
      .WillOnce(
          Invoke([&dummy_result](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                                 XError *out_r) -> XQuery_result_ptr {
            *out_r = {};
            return dummy_result.release();
          }));

  Argument_value::Object obj;
  obj.emplace("key1", argument_value_int);
  obj.emplace("key2", argument_value_double);
  obj.emplace("key3", argument_value_string);

  auto query_result =
      m_sut->execute_stmt("s", expected_sql, {Argument_value{obj}}, &out_error);

  ASSERT_EQ(dummy_result_ptr, query_result.get());
  ASSERT_FALSE(out_error);

  expect_connection_close();
}

TEST_F(Xcl_session_impl_tests_execute_connected,
       execute_stmt_args_array_scalars) {
  XError out_error;
  const Argument_value argument_value_int{static_cast<uint64_t>(1002)};
  const Argument_value argument_value_double{20.1};
  const Argument_value argument_value_string{"str1"};

  std::unique_ptr<XQuery_result> dummy_result{new Mock_query_result};
  auto dummy_result_ptr = dummy_result.get();

  EXPECT_CALL(*m_mock_protocol,
              execute_stmt_raw(Cmp_msg("stmt: \"SELECT 1;\" "
                                       "namespace: \"s\" "
                                       "args { type:ARRAY array { "
                                       "       value { "
                                       "         type: SCALAR scalar { "
                                       "           type: V_UINT "
                                       "           v_unsigned_int: 1002 "
                                       "       } } "
                                       "       value { "
                                       "         type: SCALAR scalar { "
                                       "           type: V_DOUBLE "
                                       "           v_double: 20.1 "
                                       "       } } "
                                       "       value { "
                                       "         type: SCALAR scalar { "
                                       "             type: V_STRING "
                                       "             v_string { "
                                       "               value: \"str1\" } "
                                       "       } } "
                                       "} }"),
                               _))
      .WillOnce(
          Invoke([&dummy_result](const StmtExecute &stmt MY_ATTRIBUTE((unused)),
                                 XError *out_r) -> XQuery_result_ptr {
            *out_r = {};
            return dummy_result.release();
          }));

  auto query_result = m_sut->execute_stmt(
      "s", expected_sql,
      {Argument_value{Argument_array{argument_value_int, argument_value_double,
                                     argument_value_string}}},
      &out_error);

  ASSERT_EQ(dummy_result_ptr, query_result.get());
  ASSERT_FALSE(out_error);

  expect_connection_close();
}

}  // namespace test
}  // namespace xcl
