/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/ngs/include/ngs/mysqlx/getter_any.h"

namespace xpl {

namespace test {

using namespace Mysqlx::Datatypes;

class Type_handler {
 public:
  virtual ~Type_handler() {}

  virtual void put(const ::google::protobuf::int64 &value) = 0;
  virtual void put(const ::google::protobuf::uint64 &value) = 0;
  virtual void put(const double &value) = 0;
  virtual void put(const float &value) = 0;
  virtual void put(const bool &value) = 0;
  virtual void put(const std::string &value) = 0;
  virtual void put() = 0;
};

class Mock_type_handler : public Type_handler {
 public:
  // Workaround for GMOCK undefined behaviour with ResultHolder
  MOCK_METHOD1(put_void, bool(const ::google::protobuf::int64 &));
  MOCK_METHOD1(put_void, bool(const ::google::protobuf::uint64 &));
  MOCK_METHOD1(put_void, bool(const double &));
  MOCK_METHOD1(put_void, bool(const float &));
  MOCK_METHOD1(put_void, bool(const bool &));
  MOCK_METHOD1(put_void, bool(const std::string &));
  MOCK_METHOD0(put_void, bool());

  void put(const ::google::protobuf::int64 &arg) { put_void(arg); }

  void put(const ::google::protobuf::uint64 &arg) { put_void(arg); }

  void put(const double &arg) { put_void(arg); }

  void put(const float &arg) { put_void(arg); }

  void put(const bool &arg) { put_void(arg); }

  void put(const std::string &arg) { put_void(arg); }

  void put() { put_void(); }
};

class Getter_any_testsuite : public ::testing::Test {
 public:
  template <typename Value_type>
  void operator()(const Value_type &value) {
    mock.put(value);
  }
  void operator()(const std::string &value, const uint32_t) { mock.put(value); }
  void operator()() { mock.put(); }

  Mock_type_handler mock;
  Any any;
};

TEST_F(Getter_any_testsuite, put_throwError_whenPutAnyWithoutType) {
  ASSERT_THROW(ngs::Getter_any::put_scalar_value_to_functor(any, *this),
               ngs::Error_code);
}

TEST_F(Getter_any_testsuite, put_executesNullCallback) {
  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_NULL);

  EXPECT_CALL(mock, put_void());

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesSignedIntCallback) {
  const ::google::protobuf::int64 expected_value = -10;

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_SINT);
  any.mutable_scalar()->set_v_signed_int(expected_value);

  EXPECT_CALL(mock,
              put_void(::testing::Matcher<const ::google::protobuf::int64 &>(
                  expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesUnsignedIntCallback) {
  ::google::protobuf::uint64 expected_value = 10;

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_UINT);
  any.mutable_scalar()->set_v_unsigned_int(expected_value);

  EXPECT_CALL(mock,
              put_void(::testing::Matcher<const ::google::protobuf::uint64 &>(
                  expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesBoolCallback) {
  bool expected_value = true;

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_BOOL);
  any.mutable_scalar()->set_v_bool(expected_value);

  EXPECT_CALL(mock, put_void(::testing::Matcher<const bool &>(expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesFloatCallback) {
  float expected_value = 1.120f;

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_FLOAT);
  any.mutable_scalar()->set_v_float(expected_value);

  EXPECT_CALL(mock,
              put_void(::testing::Matcher<const float &>(expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesDoubleCallback) {
  double expected_value = 2.2120;

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_DOUBLE);
  any.mutable_scalar()->set_v_double(expected_value);

  EXPECT_CALL(mock,
              put_void(::testing::Matcher<const double &>(expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_throwsError_whenStringWithoutValue) {
  std::string expected_value = "Expected string";

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_STRING);
  any.mutable_scalar()->mutable_v_string();

  ASSERT_THROW(ngs::Getter_any::put_scalar_value_to_functor(any, *this),
               ngs::Error_code);
}

TEST_F(Getter_any_testsuite, put_executesStringCallback) {
  std::string expected_value = "Expected string";

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_STRING);
  any.mutable_scalar()->mutable_v_string()->set_value(expected_value);

  EXPECT_CALL(
      mock, put_void(::testing::Matcher<const std::string &>(expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

TEST_F(Getter_any_testsuite, put_executesOctetsCallback) {
  std::string expected_value = "Expected string";

  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(Scalar_Type_V_OCTETS);
  any.mutable_scalar()->mutable_v_octets()->set_value(expected_value);

  EXPECT_CALL(
      mock, put_void(::testing::Matcher<const std::string &>(expected_value)));

  ngs::Getter_any::put_scalar_value_to_functor(any, *this);
}

class Getter_any_type_testsuite
    : public Getter_any_testsuite,
      public ::testing::WithParamInterface<Any_Type> {};

TEST_P(Getter_any_type_testsuite, put_throwError_whenNotSupportedType) {
  any.set_type(GetParam());

  ASSERT_THROW(ngs::Getter_any::put_scalar_value_to_functor(any, *this),
               ngs::Error_code);
}

INSTANTIATE_TEST_CASE_P(InstantiationNegativeTests, Getter_any_type_testsuite,
                        ::testing::Values(Any_Type_OBJECT, Any_Type_ARRAY));

class Getter_scalar_type_testsuite
    : public Getter_any_testsuite,
      public ::testing::WithParamInterface<Scalar_Type> {};

TEST_P(Getter_scalar_type_testsuite, put_throwError_whenNotSupportedType) {
  any.set_type(Any_Type_SCALAR);
  any.mutable_scalar()->set_type(GetParam());

  ASSERT_THROW(ngs::Getter_any::put_scalar_value_to_functor(any, *this),
               ngs::Error_code);
}

INSTANTIATE_TEST_CASE_P(
    InstantiationNegativeTests, Getter_scalar_type_testsuite,
    ::testing::Values(Scalar_Type_V_SINT, Scalar_Type_V_UINT,
                      Scalar_Type_V_BOOL, Scalar_Type_V_FLOAT,
                      Scalar_Type_V_DOUBLE, Scalar_Type_V_STRING,
                      Scalar_Type_V_OCTETS));

}  // namespace test
}  // namespace xpl
