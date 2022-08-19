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

#include <my_byteorder.h>

#include "plugin/x/src/prepare_param_handler.h"
#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {
namespace test {

using Arg_list = Prepare_param_handler::Arg_list;
using Param_list = Prepare_param_handler::Param_list;
using Placeholder_list = Prepare_param_handler::Placeholder_list;
using Param_value_list = Prepare_param_handler::Param_value_list;
using Param_svalue_list = Prepare_param_handler::Param_svalue_list;

class Prepare_param_handler_base_test : public testing::Test {
 public:
  Placeholder_list m_placeholders;
  Prepare_param_handler m_handler{m_placeholders};
};

struct Param_check_argument_placeholder_consistency {
  int expect_error_code;
  std::size_t args_size;
  Placeholder_list phs;
};

Param_check_argument_placeholder_consistency
    check_argument_placeholder_consistency_param[] = {
        {ER_X_SUCCESS, 0, {}},
        {ER_X_SUCCESS, 1, {0}},
        {ER_X_SUCCESS, 1, {0, 0, 0}},
        {ER_X_SUCCESS, 2, {1, 0, 1}},
        {ER_X_SUCCESS, 3, {0, 1, 2}},
        {ER_X_SUCCESS, 3, {1, 2, 0}},
        {ER_X_SUCCESS, 3, {2, 1, 0}},
        {ER_X_SUCCESS, 3, {2, 0, 1}},
        {ER_X_SUCCESS, 3, {1, 0, 2}},
        {ER_X_SUCCESS, 3, {0, 2, 1}},
        {ER_X_PREPARED_EXECUTE_ARGUMENT_CONSISTENCY, 0, {0}},
        {ER_X_PREPARED_EXECUTE_ARGUMENT_CONSISTENCY, 1, {2}},
};

class Prepare_param_handler_argument_consistency_test
    : public Prepare_param_handler_base_test,
      public testing::WithParamInterface<
          Param_check_argument_placeholder_consistency> {};

TEST_P(Prepare_param_handler_argument_consistency_test,
       check_argument_placeholder_consistency) {
  const Param_check_argument_placeholder_consistency &param = GetParam();
  m_placeholders = param.phs;
  ASSERT_ERROR_CODE(
      param.expect_error_code,
      m_handler.check_argument_placeholder_consistency(param.args_size, 0));
}

INSTANTIATE_TEST_CASE_P(
    Prepare_command_handler, Prepare_param_handler_argument_consistency_test,
    testing::ValuesIn(check_argument_placeholder_consistency_param));

namespace {

class Value {
 public:
  Value() {}
  explicit Value(const unsigned v) {
    int8store(buf.data(), static_cast<uint64_t>(v));
  }
  explicit Value(const int v) {
    int8store(buf.data(), static_cast<int64_t>(v));
  }
  explicit Value(const double v) { float8store(buf.data(), v); }
  explicit Value(const float v) { float4store(buf.data(), v); }
  explicit Value(const bool v) { buf[0] = v ? 1 : 0; }
  operator const Param_value_list::value_type &() const { return buf; }

 private:
  Param_value_list::value_type buf{{0}};
};

}  // namespace

struct Param_prepare_parameters {
  int expect_error_code;
  Param_list expect_params;
  Param_value_list expect_param_values;
  Param_svalue_list expect_param_svalues;
  Arg_list args;
  Placeholder_list phs;
};

#define NLL \
  { true, MYSQL_TYPE_NULL, false, nullptr, 0ul }
#define SIN \
  { false, MYSQL_TYPE_LONGLONG, false, nullptr, sizeof(int64_t) }
#define UIN \
  { false, MYSQL_TYPE_LONGLONG, true, nullptr, sizeof(uint64_t) }
#define STR(len) \
  { false, MYSQL_TYPE_STRING, false, nullptr, len }
#define DBL \
  { false, MYSQL_TYPE_DOUBLE, false, nullptr, sizeof(double) }
#define FLT \
  { false, MYSQL_TYPE_FLOAT, false, nullptr, sizeof(float) }
#define BOL \
  { false, MYSQL_TYPE_TINY, false, nullptr, 1ul }

#define RAW(id) \
  { id, Placeholder_info::Type::k_raw }
#define JSN(id) \
  { id, Placeholder_info::Type::k_json }

using Octets = Scalar::Octets;
using String = Scalar::String;

Param_prepare_parameters prepare_parameters_param[] = {
    {ER_X_SUCCESS, {}, {}, {}, {}, {}},
    {ER_X_SUCCESS, {NLL}, {}, {}, Any_list{Scalar::Null()}, {RAW(0)}},
    {ER_X_SUCCESS, {STR(4)}, {}, {"null"}, Any_list{Scalar::Null()}, {JSN(0)}},
    {ER_X_SUCCESS, {SIN}, {Value(-1)}, {}, Any_list{Scalar(-1)}, {RAW(0)}},
    {ER_X_SUCCESS, {SIN}, {Value(-1)}, {}, Any_list{Scalar(-1)}, {JSN(0)}},
    {ER_X_SUCCESS, {UIN}, {Value(1u)}, {}, Any_list{Scalar(1u)}, {RAW(0)}},
    {ER_X_SUCCESS, {UIN}, {Value(1u)}, {}, Any_list{Scalar(1u)}, {JSN(0)}},
    {ER_X_SUCCESS, {STR(3)}, {}, {}, Any_list{String("abc")}, {RAW(0)}},
    {ER_X_SUCCESS,
     {STR(5)},
     {},
     {"\"abc\""},
     Any_list{String("abc")},
     {JSN(0)}},
    {ER_X_SUCCESS, {STR(3)}, {}, {}, Any_list{Octets("abc")}, {RAW(0)}},
    {ER_X_SUCCESS,
     {STR(3)},
     {},
     {},
     Any_list{Octets("abc", Octets::Content_type::k_json)},
     {RAW(0)}},
    {ER_X_SUCCESS,
     {STR(5)},
     {},
     {"\"abc\""},
     Any_list{Octets("abc")},
     {JSN(0)}},
    {ER_X_SUCCESS,
     {STR(3)},
     {},
     {"abc"},
     Any_list{Octets("abc", Octets::Content_type::k_json)},
     {JSN(0)}},
    {ER_X_SUCCESS, {DBL}, {Value(1.1)}, {}, Any_list{Scalar(1.1)}, {RAW(0)}},
    {ER_X_SUCCESS, {DBL}, {Value(1.1)}, {}, Any_list{Scalar(1.1)}, {JSN(0)}},
    {ER_X_SUCCESS, {FLT}, {Value(1.1f)}, {}, Any_list{Scalar(1.1f)}, {RAW(0)}},
    {ER_X_SUCCESS, {FLT}, {Value(1.1f)}, {}, Any_list{Scalar(1.1f)}, {JSN(0)}},
    {ER_X_SUCCESS, {BOL}, {Value(true)}, {}, Any_list{Scalar(true)}, {RAW(0)}},
    {ER_X_SUCCESS,
     {BOL},
     {Value(false)},
     {},
     Any_list{Scalar(false)},
     {RAW(0)}},
    {ER_X_SUCCESS, {STR(4)}, {}, {"true"}, Any_list{Scalar(true)}, {JSN(0)}},
    {ER_X_SUCCESS, {STR(5)}, {}, {"false"}, Any_list{Scalar(false)}, {JSN(0)}},
    {ER_X_SUCCESS,
     {UIN, SIN},
     {Value(2u), Value(1)},
     {},
     Any_list{Scalar(2u), Scalar(1)},
     {RAW(0), RAW(1)}},
    {ER_X_SUCCESS,
     {SIN, UIN},
     {Value(1), Value(2u)},
     {},
     Any_list{Scalar(2u), Scalar(1)},
     {RAW(1), RAW(0)}},
    {ER_X_SUCCESS,
     {SIN, SIN, SIN},
     {Value(1), Value(1), Value(1)},
     {},
     Any_list{Scalar(1)},
     {RAW(0), RAW(0), RAW(0)}},
    {ER_X_SUCCESS,
     {NLL, SIN, NLL},
     {Value(1)},
     {},
     Any_list{Scalar::Null(), Scalar(1)},
     {RAW(0), RAW(1), RAW(0)}},
    {ER_X_SUCCESS,
     {NLL, STR(2), STR(3)},
     {},
     {},
     Any_list{Scalar::String("ab"), Scalar::Octets("abc"), Scalar::Null()},
     {RAW(2), RAW(0), RAW(1)}},
    {ER_X_PREPARED_EXECUTE_ARGUMENT_NOT_SUPPORTED,
     {},
     {},
     {},
     Any_list{Any::Object()},
     {RAW(0)}},
    {ER_X_PREPARED_EXECUTE_ARGUMENT_NOT_SUPPORTED,
     {},
     {},
     {},
     Any_list{Any::Array()},
     {RAW(0)}},
    {ER_X_SUCCESS,
     {SIN},
     {Value(1)},
     {},
     Any_list{Scalar(1), Any::Array()},
     {RAW(0)}},
    {ER_X_SUCCESS,
     {BOL, STR(4)},
     {Value(true)},
     {"true"},
     Any_list{Scalar(true)},
     {RAW(0), JSN(0)}},
};

class Prepare_command_handler_prepare_parameters_test
    : public Prepare_param_handler_base_test,
      public testing::WithParamInterface<Param_prepare_parameters> {};

MATCHER(Eq_param, "") {
  using ::testing::get;
  // intentionally skip comparison of "value" value;
  return get<0>(arg).null_bit == get<1>(arg).null_bit &&
         get<0>(arg).type == get<1>(arg).type &&
         get<0>(arg).unsigned_type == get<1>(arg).unsigned_type &&
         get<0>(arg).length == get<1>(arg).length;
}

TEST_P(Prepare_command_handler_prepare_parameters_test, prepare_parameters) {
  const auto &param = GetParam();
  m_placeholders = param.phs;
  ASSERT_ERROR_CODE(param.expect_error_code,
                    m_handler.prepare_parameters(param.args));
  EXPECT_THAT(m_handler.get_params(),
              testing::Pointwise(Eq_param(), param.expect_params));
  EXPECT_EQ(param.expect_param_svalues, m_handler.get_string_values());
  EXPECT_EQ(param.expect_param_values, m_handler.get_values());
}

INSTANTIATE_TEST_CASE_P(Prepare_command_handler,
                        Prepare_command_handler_prepare_parameters_test,
                        testing::ValuesIn(prepare_parameters_param));

}  // namespace test
}  // namespace xpl
