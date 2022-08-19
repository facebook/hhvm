/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "plugin/x/src/account_verification_handler.h"
#include "plugin/x/src/capabilities/handler_auth_mech.h"
#include "plugin/x/src/capabilities/handler_client_interactive.h"
#include "plugin/x/src/capabilities/handler_connection_attributes.h"
#include "plugin/x/src/capabilities/handler_tls.h"
#include "plugin/x/src/sql_user_require.h"
#include "unittest/gunit/xplugin/xpl/assert_error_code.h"
#include "unittest/gunit/xplugin/xpl/mock/capabilities.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"
#include "unittest/gunit/xplugin/xpl/mysqlx_pb_wrapper.h"

namespace xpl {

namespace test {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;
using ::testing::StrictMock;
using ::testing::Test;
using ::testing::WithParamInterface;
using xpl::test::Mock_authentication_container;

class CapabilityHanderTlsTestSuite : public Test {
 public:
  CapabilityHanderTlsTestSuite() : sut(mock_client) {
    EXPECT_CALL(mock_client, connection())
        .WillRepeatedly(ReturnRef(mock_connection));
    EXPECT_CALL(mock_client, server()).WillRepeatedly(ReturnRef(mock_server));
    EXPECT_CALL(mock_server, ssl_context())
        .WillRepeatedly(Return(&mock_ssl_context));
  }

  StrictMock<Mock_vio> mock_connection;
  StrictMock<Mock_client> mock_client;
  StrictMock<Mock_ssl_context> mock_ssl_context;
  StrictMock<Mock_server> mock_server;

  Capability_tls sut;
};

TEST_F(
    CapabilityHanderTlsTestSuite,
    isSupported_returnsCurrentConnectionOption_on_supported_connection_type) {
  EXPECT_CALL(mock_ssl_context, has_ssl())
      .WillOnce(Return(true))
      .WillOnce(Return(false));
  EXPECT_CALL(mock_connection, get_type())
      .WillOnce(Return(xpl::Connection_tcpip))
      .WillOnce(Return(xpl::Connection_tcpip));

  ASSERT_TRUE(sut.is_supported());
  ASSERT_FALSE(sut.is_supported());
}

TEST_F(CapabilityHanderTlsTestSuite,
       isSupported_returnsFailure_on_unsupported_connection_type) {
  EXPECT_CALL(mock_ssl_context, has_ssl())
      .WillOnce(Return(true))
      .WillOnce(Return(false));
  EXPECT_CALL(mock_connection, get_type())
      .WillOnce(Return(xpl::Connection_namedpipe))
      .WillOnce(Return(xpl::Connection_namedpipe));

  ASSERT_FALSE(sut.is_supported());
  ASSERT_FALSE(sut.is_supported());
}

TEST_F(CapabilityHanderTlsTestSuite, name_returnsTls_always) {
  ASSERT_STREQ("tls", sut.name().c_str());
}

TEST_F(CapabilityHanderTlsTestSuite,
       get_returnsCurrentConnectionOption_always) {
  const bool expected_result = true;
  ::Mysqlx::Datatypes::Any any;

  EXPECT_CALL(mock_connection, get_type())
      .WillOnce(Return(xpl::Connection_type::Connection_tls));

  sut.get(&any);

  ASSERT_EQ(::Mysqlx::Datatypes::Any::SCALAR, any.type());
  ASSERT_EQ(::Mysqlx::Datatypes::Scalar::V_BOOL, any.scalar().type());
  ASSERT_EQ(expected_result, any.scalar().v_bool());
}

class Set_params {
 public:
  template <typename T>
  Set_params(T any, bool tls) : m_tls_active(tls) {
    m_any = Any{Scalar{any}};

    m_tls_active = tls;
  }

  Any m_any;
  bool m_tls_active;
};

::std::ostream &operator<<(::std::ostream &os, const Set_params &set_param) {
  return os << "tls-active:" << set_param.m_tls_active << std::endl;
}

class SuccessSetCapabilityHanderTlsTestSuite
    : public CapabilityHanderTlsTestSuite,
      public WithParamInterface<Set_params> {
 public:
};

TEST_P(SuccessSetCapabilityHanderTlsTestSuite,
       get_success_forValidParametersAndTlsSupportedOnTcpip) {
  auto s = GetParam();

  EXPECT_CALL(mock_ssl_context, has_ssl()).WillOnce(Return(true));
  EXPECT_CALL(mock_connection, get_type())
      .WillRepeatedly(Return(xpl::Connection_tcpip));

  ASSERT_EQ(ER_X_SUCCESS, sut.set(s.m_any).error);

  EXPECT_CALL(mock_client, activate_tls_void());

  sut.commit();
}

TEST_P(SuccessSetCapabilityHanderTlsTestSuite,
       get_failure_forValidParametersAndTlsSupportedOnNamedPipe) {
  Set_params s = GetParam();

  EXPECT_CALL(mock_ssl_context, has_ssl()).WillOnce(Return(true));
  EXPECT_CALL(mock_connection, get_type())
      .WillRepeatedly(Return(xpl::Connection_namedpipe));

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut.set(s.m_any).error);
}

TEST_P(SuccessSetCapabilityHanderTlsTestSuite,
       get_failure_forValidParametersAndTlsIsntSupported) {
  Set_params s = GetParam();

  EXPECT_CALL(mock_ssl_context, has_ssl()).WillOnce(Return(false));
  EXPECT_CALL(mock_connection, get_type())
      .WillRepeatedly(Return(xpl::Connection_tcpip));

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut.set(s.m_any).error);
}

INSTANTIATE_TEST_CASE_P(
    SuccessInstantiation, SuccessSetCapabilityHanderTlsTestSuite,
    ::testing::Values(Set_params(true, false), Set_params(1, false),
                      Set_params(2, false), Set_params(3u, false),
                      Set_params(1.0, false)));

class FaildSetCapabilityHanderTlsTestSuite
    : public SuccessSetCapabilityHanderTlsTestSuite {};

TEST_P(FaildSetCapabilityHanderTlsTestSuite, get_failure_forValidParameters) {
  Set_params s = GetParam();

  EXPECT_CALL(mock_connection, get_type())
      .WillRepeatedly(
          Return(s.m_tls_active ? xpl::Connection_tls : xpl::Connection_tcpip));

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut.set(s.m_any).error);

  sut.commit();
}

INSTANTIATE_TEST_CASE_P(
    FaildInstantiationAlreadySet, FaildSetCapabilityHanderTlsTestSuite,
    ::testing::Values(Set_params(true, true), Set_params(1, true),
                      Set_params(2, true), Set_params(3u, true),
                      Set_params(1.0, true)));

INSTANTIATE_TEST_CASE_P(
    FaildInstantiationCantDisable, FaildSetCapabilityHanderTlsTestSuite,
    ::testing::Values(Set_params(false, true), Set_params(0, true),
                      Set_params(0u, true), Set_params(0.0, true)));

INSTANTIATE_TEST_CASE_P(FaildInstantiationAlreadyDisabled,
                        FaildSetCapabilityHanderTlsTestSuite,
                        ::testing::Values(Set_params(0, false),
                                          Set_params(false, false)));

class CapabilityHanderAuthMechTestSuite : public Test {
 public:
  CapabilityHanderAuthMechTestSuite() : sut(mock_client) {
    mock_server = std::make_shared<StrictMock<Mock_server>>();

    EXPECT_CALL(mock_client, connection())
        .WillRepeatedly(ReturnRef(mock_connection));
    EXPECT_CALL(mock_client, server()).WillRepeatedly(ReturnRef(*mock_server));
  }

  std::shared_ptr<StrictMock<Mock_server>> mock_server;

  StrictMock<Mock_vio> mock_connection;
  StrictMock<Mock_client> mock_client;

  Capability_auth_mech sut;
};

TEST_F(CapabilityHanderAuthMechTestSuite, isSupported_returnsTrue_always) {
  ASSERT_TRUE(sut.is_supported());
}

TEST_F(CapabilityHanderAuthMechTestSuite, set_returnsFalse_always) {
  Set_params set(1, false);

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut.set(set.m_any).error);
}

TEST_F(CapabilityHanderAuthMechTestSuite, commit_doesNothing_always) {
  sut.commit();
}

TEST_F(CapabilityHanderAuthMechTestSuite, name) {
  ASSERT_STREQ("authentication.mechanisms", sut.name().c_str());
}

TEST_F(CapabilityHanderAuthMechTestSuite, get_doesNothing_whenEmptySetReceive) {
  std::vector<std::string> names{};
  ::Mysqlx::Datatypes::Any any;
  Mock_authentication_container mock_auth;

  EXPECT_CALL(*mock_server, get_authentications())
      .WillOnce(ReturnRef(mock_auth));

  EXPECT_CALL(mock_auth, get_authentication_mechanisms(&mock_client))
      .WillOnce(Return(names));
  sut.get(&any);

  ASSERT_EQ(::Mysqlx::Datatypes::Any::ARRAY, any.type());
  EXPECT_EQ(0, any.array().value_size());
}

TEST_F(CapabilityHanderAuthMechTestSuite,
       get_returnAuthMethodsFromServer_always) {
  std::vector<std::string> names{"first", "second"};
  ::Mysqlx::Datatypes::Any any;

  StrictMock<Mock_authentication_container> mock_auth;

  EXPECT_CALL(*mock_server, get_authentications())
      .WillOnce(ReturnRef(mock_auth));

  EXPECT_CALL(mock_auth, get_authentication_mechanisms(_))
      .WillOnce(Return(names));

  sut.get(&any);

  ASSERT_EQ(::Mysqlx::Datatypes::Any::ARRAY, any.type());
  ASSERT_EQ(static_cast<int>(names.size()), any.array().value_size());

  for (std::size_t i = 0; i < names.size(); ++i) {
    const auto &a = any.array().value(static_cast<int>(i));

    ASSERT_EQ(::Mysqlx::Datatypes::Any::SCALAR, a.type());
    ASSERT_EQ(::Mysqlx::Datatypes::Scalar::V_STRING, a.scalar().type());
    ASSERT_STREQ(names[i].c_str(), a.scalar().v_string().value().c_str());
  }
}

class Capability_hander_client_interactive_test_suite : public Test {
 public:
  Capability_hander_client_interactive_test_suite() = default;

  void SetUp() {
    EXPECT_CALL(mock_client, is_interactive()).WillOnce(Return(false));
    sut.reset(new Capability_client_interactive(mock_client));
  }

  std::unique_ptr<Capability_client_interactive> sut;
  StrictMock<Mock_client> mock_client;
};

TEST_F(Capability_hander_client_interactive_test_suite,
       is_supported_returns_true_always) {
  ASSERT_TRUE(sut->is_supported());
}

TEST_F(Capability_hander_client_interactive_test_suite,
       name_returns_client_interactive_always) {
  ASSERT_STREQ("client.interactive", sut->name().c_str());
}

TEST_F(Capability_hander_client_interactive_test_suite,
       get_when_client_is_interactive) {
  EXPECT_CALL(mock_client, is_interactive()).WillOnce(Return(true));
  sut.reset(new Capability_client_interactive(mock_client));

  const bool expected_result = true;
  ::Mysqlx::Datatypes::Any any;

  sut->get(&any);

  ASSERT_EQ(::Mysqlx::Datatypes::Any::SCALAR, any.type());
  ASSERT_EQ(::Mysqlx::Datatypes::Scalar::V_BOOL, any.scalar().type());
  ASSERT_EQ(expected_result, any.scalar().v_bool());
}

TEST_F(Capability_hander_client_interactive_test_suite,
       get_when_client_is_not_interactive) {
  EXPECT_CALL(mock_client, is_interactive()).WillOnce(Return(false));
  sut.reset(new Capability_client_interactive(mock_client));

  const bool expected_result = false;
  ::Mysqlx::Datatypes::Any any;

  sut->get(&any);

  ASSERT_EQ(::Mysqlx::Datatypes::Any::SCALAR, any.type());
  ASSERT_EQ(::Mysqlx::Datatypes::Scalar::V_BOOL, any.scalar().type());
  ASSERT_EQ(expected_result, any.scalar().v_bool());
}

TEST_F(Capability_hander_client_interactive_test_suite,
       set_and_commit_valid_type) {
  Any any{Scalar{true}};

  ASSERT_EQ(ER_X_SUCCESS, sut->set(any).error);

  EXPECT_CALL(mock_client, set_is_interactive(true));

  sut->commit();
}

TEST_F(Capability_hander_client_interactive_test_suite,
       set_and_commit_invalid_type) {
  Any any{Scalar{"invalid"}};

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut->set(any).error);

  EXPECT_CALL(mock_client, set_is_interactive(false));

  sut->commit();
}

class Capability_handler_connection_attributes : public Test {
 public:
  void SetUp() override {
    ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, m_sut.set(m_any).error);
  }

  Any m_any;
  Capability_connection_attributes m_sut;
};

TEST_F(Capability_handler_connection_attributes, is_supported) {
  ASSERT_TRUE(m_sut.is_supported());
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_empty_object) {
  const auto empty_obj = Any::Object{};
  m_any = Any{empty_obj};
  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, m_sut.set(m_any).error);
}
TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_empty_field) {
  const Any::Object::Fld field{};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_field_with_empty_value) {
  const Any::Object::Fld field{"some_key", {}};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_field_with_empty_scalar) {
  Scalar empty_scalar;
  Any value{empty_scalar};
  const Any::Object::Fld field{"some_key", value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_TYPE, m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_field_with_scalar_other_than_string) {
  Scalar scalar{true};
  Any value{scalar};
  const Any::Object::Fld field{"some_key", value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_TYPE, m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_field_with_empty_string_scalar) {
  ::Mysqlx::Datatypes::Object obj;
  ::Mysqlx::Datatypes::Any value;
  ::Mysqlx::Datatypes::Scalar scalar;
  ::Mysqlx::Datatypes::Scalar_String empty_scalar_string;
  scalar.mutable_v_string()->CopyFrom(empty_scalar_string);
  value.mutable_scalar()->CopyFrom(scalar);
  auto field = obj.add_fld();
  field->set_key("some_key");
  field->mutable_value()->CopyFrom(value);
  ::Mysqlx::Datatypes::Any any;
  any.mutable_obj()->CopyFrom(obj);
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_TYPE, m_sut.set(any).error);
}

TEST_F(Capability_handler_connection_attributes,
       set_invalid_capability_field_without_a_key) {
  ::Mysqlx::Datatypes::Object obj;
  ::Mysqlx::Datatypes::Any value;
  ::Mysqlx::Datatypes::Scalar scalar;
  ::Mysqlx::Datatypes::Scalar_String scalar_string;
  scalar_string.set_value("some value");
  scalar.mutable_v_string()->CopyFrom(scalar_string);
  value.mutable_scalar()->CopyFrom(scalar);
  auto field = obj.add_fld();
  field->mutable_value()->CopyFrom(value);
  ::Mysqlx::Datatypes::Any any;
  any.mutable_obj()->CopyFrom(obj);
  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, m_sut.set(any).error);
}

TEST_F(Capability_handler_connection_attributes, key_max_size_exceeded) {
  Scalar::String scalar_string{"some value"};
  Scalar scalar{scalar_string};
  Any value{scalar};
  std::string long_string(128, 'x');
  const Any::Object::Fld field{long_string, value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_KEY_LENGTH,
            m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes, empty_key) {
  Scalar::String scalar_string{"some value"};
  Scalar scalar{scalar_string};
  Any value{scalar};
  const Any::Object::Fld field{"", value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_EMPTY_KEY,
            m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes, value_max_size_exceeded) {
  std::string long_string(2048, 'x');
  Scalar::String long_scalar_string{long_string};
  Scalar scalar{long_scalar_string};
  Any value{scalar};
  const Any::Object::Fld field{"some key", value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_BAD_CONNECTION_SESSION_ATTRIBUTE_VALUE_LENGTH,
            m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes, set_one_key_value_pair) {
  Scalar::String scalar_string{"some value"};
  Scalar scalar{scalar_string};
  Any value{scalar};
  const Any::Object::Fld field{"some key", value};
  const Any::Object obj{field};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_SUCCESS, m_sut.set(m_any).error);
}

TEST_F(Capability_handler_connection_attributes, set_two_key_value_pairs) {
  Scalar::String scalar_string1{"some value"};
  Scalar::String scalar_string2{"other value"};
  Scalar scalar1{scalar_string1};
  Scalar scalar2{scalar_string1};
  Any value1{scalar1};
  Any value2{scalar2};
  const Any::Object::Fld field1{"some key", value1};
  const Any::Object::Fld field2{"other key", value2};
  const Any::Object obj{{field1, field2}};
  m_any = Any{obj};
  ASSERT_EQ(ER_X_SUCCESS, m_sut.set(m_any).error);
}

}  // namespace test

}  // namespace xpl
