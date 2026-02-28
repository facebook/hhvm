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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include "plugin/x/src/account_verification_handler.h"
#include "plugin/x/src/sql_user_require.h"
#include "plugin/x/src/xpl_resultset.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"
#include "unittest/gunit/xplugin/xpl/one_row_resultset.h"

namespace xpl {

#define ER_SUCCESS 0
#define NOT !

namespace test {
using namespace ::testing;  // NOLINT(build/namespaces)

namespace {
const char *const EMPTY = "";
const char *const USER_NAME = "TEST";
const char *const USER_DB = "TEST_DB";
const char *const USER_IP = "100.20.20.10";
const char *const EXPECTED_HASH = "AABBCCDD";
const char *const NOT_EXPECTED_HASH = "ALA_MA_KOTA";
const bool REQUIRE_SECURE_TRANSPORT = true;
const bool ACCOUNT_LOCKED = true;
const bool PASSWORD_EXPIRED = true;
const bool DISCONNECT_ON_EXPIRED_PASSWORD = true;
const bool OFFLINE_MODE = true;
const char *const AUTH_PLUGIN_NAME = "mysql_native_password";
const char *const WRONG_AUTH_PLUGIN_NAME = "wrong_password";
}  // namespace

class User_verification_test : public Test {
 public:
  StrictMock<Mock_client> mock_client;
  StrictMock<Mock_session> mock_session;
  StrictMock<Mock_vio> mock_connection;
  StrictMock<Mock_sql_data_context> mock_sql_data_context;
  Mock_account_verification *mock_account_verification{
      new StrictMock<Mock_account_verification>()};

  iface::Authentication_info m_auth_info;

  Account_verification_handler handler{
      &mock_session, iface::Account_verification::Account_type::k_native,
      mock_account_verification};

  void SetUp() {
    EXPECT_CALL(mock_session, data_context())
        .WillRepeatedly(ReturnRef(mock_sql_data_context));
    EXPECT_CALL(mock_session, client()).WillRepeatedly(ReturnRef(mock_client));
  }
};

TEST_F(User_verification_test, everything_matches_and_hash_is_right) {
  One_row_resultset data{NOT REQUIRE_SECURE_TRANSPORT,
                         EXPECTED_HASH,
                         AUTH_PLUGIN_NAME,
                         NOT ACCOUNT_LOCKED,
                         NOT PASSWORD_EXPIRED,
                         NOT DISCONNECT_ON_EXPIRED_PASSWORD,
                         NOT OFFLINE_MODE,
                         EMPTY,
                         EMPTY,
                         EMPTY,
                         EMPTY};

  EXPECT_CALL(mock_sql_data_context, execute(_, _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  EXPECT_CALL(mock_client, connection())
      .WillRepeatedly(ReturnRef(mock_connection));

  EXPECT_CALL(*mock_account_verification,
              verify_authentication_string(_, _, _, _))
      .WillOnce(Return(true));

  EXPECT_EQ(
      ER_SUCCESS,
      handler.verify_account(USER_NAME, USER_IP, EXPECTED_HASH, &m_auth_info)
          .error);
}

TEST_F(User_verification_test, forwards_error_from_query_execution) {
  const ngs::Error_code expected_error(ER_DATA_OUT_OF_RANGE, "");

  EXPECT_CALL(mock_sql_data_context, execute(_, _, _))
      .WillOnce(Return(expected_error));

  EXPECT_EQ(
      expected_error.error,
      handler.verify_account(USER_NAME, USER_IP, EXPECTED_HASH, &m_auth_info)
          .error);
}

TEST_F(User_verification_test, dont_match_anything_when_hash_isnt_right) {
  One_row_resultset data{NOT REQUIRE_SECURE_TRANSPORT,
                         NOT_EXPECTED_HASH,
                         AUTH_PLUGIN_NAME,
                         NOT ACCOUNT_LOCKED,
                         NOT PASSWORD_EXPIRED,
                         NOT DISCONNECT_ON_EXPIRED_PASSWORD,
                         NOT OFFLINE_MODE,
                         EMPTY,
                         EMPTY,
                         EMPTY,
                         EMPTY};

  EXPECT_CALL(mock_sql_data_context, execute(_, _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  EXPECT_CALL(*mock_account_verification,
              verify_authentication_string(_, _, _, _))
      .WillOnce(Return(false));

  EXPECT_EQ(
      ER_ACCESS_DENIED_ERROR,
      handler.verify_account(USER_NAME, USER_IP, EXPECTED_HASH, &m_auth_info)
          .error);
}

struct Test_param {
  bool account_locked;
  bool offline_mode;
  bool password_expired;
  std::string plugin_name;
  int expected_error;
};

class User_verification_param_test : public User_verification_test,
                                     public WithParamInterface<Test_param> {};

TEST_P(User_verification_param_test, User_verification_on_given_account_param) {
  const Test_param &param = GetParam();

  One_row_resultset data{NOT REQUIRE_SECURE_TRANSPORT,
                         EXPECTED_HASH,
                         param.plugin_name.c_str(),
                         param.account_locked,
                         param.password_expired,
                         NOT DISCONNECT_ON_EXPIRED_PASSWORD,
                         param.offline_mode,
                         EMPTY,
                         EMPTY,
                         EMPTY,
                         EMPTY};

  EXPECT_CALL(mock_client, client_hostname_or_address())
      .WillRepeatedly(Return(""));

  EXPECT_CALL(mock_sql_data_context, execute(_, _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  if (param.plugin_name == AUTH_PLUGIN_NAME)
    EXPECT_CALL(*mock_account_verification,
                verify_authentication_string(_, _, _, _))
        .WillOnce(Return(true));

  EXPECT_EQ(
      param.expected_error,
      handler.verify_account(USER_NAME, USER_IP, EXPECTED_HASH, &m_auth_info)
          .error);
}

Test_param combinations[] = {
    {ACCOUNT_LOCKED, NOT OFFLINE_MODE, NOT PASSWORD_EXPIRED, AUTH_PLUGIN_NAME,
     ER_ACCOUNT_HAS_BEEN_LOCKED},
    {NOT ACCOUNT_LOCKED, NOT OFFLINE_MODE, PASSWORD_EXPIRED, AUTH_PLUGIN_NAME,
     ER_MUST_CHANGE_PASSWORD_LOGIN},
    {NOT ACCOUNT_LOCKED, OFFLINE_MODE, NOT PASSWORD_EXPIRED, AUTH_PLUGIN_NAME,
     ER_SERVER_OFFLINE_MODE},
    {NOT ACCOUNT_LOCKED, NOT OFFLINE_MODE, NOT PASSWORD_EXPIRED,
     WRONG_AUTH_PLUGIN_NAME, ER_ACCESS_DENIED_ERROR}};

INSTANTIATE_TEST_CASE_P(User_verification, User_verification_param_test,
                        ValuesIn(combinations));

struct Test_param_connection_type {
  bool requires_secure;
  Connection_type type;
  int expected_error;
};

class User_verification_param_test_with_connection_type_combinations
    : public User_verification_test,
      public WithParamInterface<Test_param_connection_type> {};

TEST_P(User_verification_param_test_with_connection_type_combinations,
       User_verification_on_given_account_connection_type) {
  const Test_param_connection_type &param = GetParam();

  EXPECT_CALL(mock_client, connection())
      .WillRepeatedly(ReturnRef(mock_connection));

  if (param.requires_secure)
    EXPECT_CALL(mock_connection, get_type()).WillOnce(Return(param.type));

  EXPECT_CALL(*mock_account_verification,
              verify_authentication_string(_, _, _, _))
      .WillOnce(Return(true));

  One_row_resultset data{param.requires_secure,
                         EXPECTED_HASH,
                         AUTH_PLUGIN_NAME,
                         NOT ACCOUNT_LOCKED,
                         NOT PASSWORD_EXPIRED,
                         NOT DISCONNECT_ON_EXPIRED_PASSWORD,
                         NOT OFFLINE_MODE,
                         EMPTY,
                         EMPTY,
                         EMPTY,
                         EMPTY};

  EXPECT_CALL(mock_sql_data_context, execute(_, _, _))
      .WillOnce(DoAll(SetUpResultset(data), Return(ngs::Success())));

  EXPECT_EQ(
      param.expected_error,
      handler.verify_account(USER_NAME, USER_IP, EXPECTED_HASH, &m_auth_info)
          .error);
}

Test_param_connection_type connection_combinations[] = {
    {NOT REQUIRE_SECURE_TRANSPORT, Connection_tcpip, ER_SUCCESS},
    {NOT REQUIRE_SECURE_TRANSPORT, Connection_namedpipe, ER_SUCCESS},
    {NOT REQUIRE_SECURE_TRANSPORT, Connection_tls, ER_SUCCESS},
    {NOT REQUIRE_SECURE_TRANSPORT, Connection_unixsocket, ER_SUCCESS},
    {REQUIRE_SECURE_TRANSPORT, Connection_unixsocket, ER_SUCCESS},
    {REQUIRE_SECURE_TRANSPORT, Connection_tls, ER_SUCCESS},
    {REQUIRE_SECURE_TRANSPORT, Connection_tcpip, ER_SECURE_TRANSPORT_REQUIRED},
    {REQUIRE_SECURE_TRANSPORT, Connection_namedpipe,
     ER_SECURE_TRANSPORT_REQUIRED}};

INSTANTIATE_TEST_CASE_P(
    User_verification,
    User_verification_param_test_with_connection_type_combinations,
    ValuesIn(connection_combinations));

struct Test_param_sasl_message {
  std::string schema;
  std::string user;
  std::string password;
  int expected_error;
  std::string get_message() const {
    return schema + '\0' + user + '\0' + password;
  }
};

class Split_sasl_message_test
    : public User_verification_test,
      public WithParamInterface<Test_param_sasl_message> {
 public:
  StrictMock<Mock_authentication_interface> mock_authentication;
};

TEST_P(Split_sasl_message_test, Split_sasl_message_on_given_param) {
  const Test_param_sasl_message &param = GetParam();

  if (param.expected_error == ER_SUCCESS) {
    EXPECT_CALL(mock_client, client_address());
    EXPECT_CALL(mock_client, client_hostname());
    EXPECT_CALL(mock_client, supports_expired_passwords());
    EXPECT_CALL(mock_sql_data_context, password_expired())
        .WillOnce(Return(false));
    EXPECT_CALL(mock_session, data_context())
        .WillOnce(ReturnRef(mock_sql_data_context));
    EXPECT_CALL(
        mock_sql_data_context,
        authenticate(StrEq(param.user), _, _, StrEq(param.schema),
                     StrEq(param.password), Ref(mock_authentication), _))
        .WillOnce(Return(ngs::Success()));
  }

  EXPECT_EQ(
      param.expected_error,
      handler
          .authenticate(mock_authentication, &m_auth_info, param.get_message())
          .error);
}

Test_param_sasl_message sasl_message[] = {
    {EMPTY, EMPTY, EMPTY, ER_ACCESS_DENIED_ERROR},
    {USER_DB, EMPTY, EMPTY, ER_ACCESS_DENIED_ERROR},
    {EMPTY, USER_NAME, EMPTY, ER_SUCCESS},
    {EMPTY, EMPTY, EXPECTED_HASH, ER_ACCESS_DENIED_ERROR},
    {USER_DB, USER_NAME, EMPTY, ER_SUCCESS},
    {EMPTY, USER_NAME, EXPECTED_HASH, ER_SUCCESS},
    {USER_DB, EMPTY, EXPECTED_HASH, ER_ACCESS_DENIED_ERROR},
    {USER_DB, USER_NAME, EXPECTED_HASH, ER_SUCCESS}};

INSTANTIATE_TEST_CASE_P(User_verification, Split_sasl_message_test,
                        ValuesIn(sasl_message));
}  // namespace test
}  // namespace xpl
