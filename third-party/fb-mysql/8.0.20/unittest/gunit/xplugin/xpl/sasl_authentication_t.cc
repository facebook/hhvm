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
#include <stdexcept>

#include "my_config.h"  // NOLINT(build/include_subdir)

#include "plugin/x/src/auth_plain.h"
#include "plugin/x/src/sql_user_require.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {

namespace test {

using namespace ::testing;  // NOLINT(build/namespaces)

template <typename Auth_type>
class AuthenticationTestSuite : public Test {
 public:
  void SetUp() {
    sut = Auth_type::create(&mock_session, nullptr);

    ON_CALL(mock_data_context, authenticate(_, _, _, _, _, _, _))
        .WillByDefault(Return(default_error));
    EXPECT_CALL(mock_connection, get_type())
        .WillRepeatedly(Return(Connection_tls));
    EXPECT_CALL(mock_client, connection())
        .WillRepeatedly(ReturnRef(mock_connection));
    EXPECT_CALL(mock_session, data_context())
        .WillRepeatedly(ReturnRef(mock_data_context));
    EXPECT_CALL(mock_session, client()).WillRepeatedly(ReturnRef(mock_client));
  }

  void assert_responce(
      const iface::Authentication::Response &result,
      const std::string &data = "",
      const iface::Authentication::Status status =
          iface::Authentication::Status::k_error,
      const int error_code = ER_NET_PACKETS_OUT_OF_ORDER) const {
    ASSERT_EQ(data, result.data);
    ASSERT_EQ(status, result.status);
    ASSERT_EQ(error_code, result.error_code);
  }

  ngs::Error_code default_error;

  StrictMock<Mock_sql_data_context> mock_data_context;
  StrictMock<Mock_client> mock_client;
  StrictMock<Mock_vio> mock_connection;
  StrictMock<Mock_session> mock_session;
  std::unique_ptr<iface::Authentication> sut;
};

typedef AuthenticationTestSuite<Sasl_plain_auth> SaslAuthenticationTestSuite;

TEST_F(SaslAuthenticationTestSuite, handleContinue_fails_always) {
  iface::Authentication::Response result = sut->handle_continue("");

  assert_responce(result, "", iface::Authentication::Status::k_error,
                  ER_NET_PACKETS_OUT_OF_ORDER);
}

template <typename Auth_type>
class ExpectedValuesAuthenticationTestSuite
    : public AuthenticationTestSuite<Auth_type> {
 public:
  ExpectedValuesAuthenticationTestSuite()
      : expected_database("test_database"),
        expected_login("test_login"),
        expected_password("test_password"),
        expected_password_hash("*4414E26EDED6D661B5386813EBBA95065DBC4728"),
        expected_host("test_host"),
        expected_hostname("test_host"),
        sasl_separator("\0", 1),
        ec_failur(1, ""),
        ec_success(0, "") {}

  std::string get_sasl_message(const std::string &login,
                               const std::string &password,
                               const std::string &autorization = "") {
    return autorization + sasl_separator + login + sasl_separator + password;
  }

  const char *expected_database;
  const char *expected_login;
  const char *expected_password;
  const char *expected_password_hash;
  const char *expected_host;
  const std::string expected_hostname;

  const std::string sasl_separator;

  const ngs::Error_code ec_failur;
  const ngs::Error_code ec_success;
};

typedef ExpectedValuesAuthenticationTestSuite<Sasl_plain_auth>
    ExpectedValuesSaslAuthenticationTestSuite;

TEST_F(ExpectedValuesSaslAuthenticationTestSuite,
       handleStart_autenticateAndReturnsError_whenIllformedStringNoSeparator) {
  std::string sasl_login_string = expected_login;

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "Invalid user or password",
                  iface::Authentication::Status::k_failed,
                  ER_ACCESS_DENIED_ERROR);
}

TEST_F(ExpectedValuesSaslAuthenticationTestSuite,
       handleStart_autenticateAndReturnsError_whenIllformedStringOneSeparator) {
  std::string sasl_login_string = "some data" + sasl_separator + "some data";

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "Invalid user or password",
                  iface::Authentication::Status::k_failed,
                  ER_ACCESS_DENIED_ERROR);
}

TEST_F(
    ExpectedValuesSaslAuthenticationTestSuite,
    handleStart_autenticateAndReturnsError_whenIllformedStringThusUserNameEmpty) {  // NOLINT(whitespace/line_length)
  const std::string empty_user = "";
  std::string sasl_login_string =
      get_sasl_message(empty_user, expected_password, "autorize_as");

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "Invalid user or password",
                  iface::Authentication::Status::k_failed,
                  ER_ACCESS_DENIED_ERROR);
}

TEST_F(ExpectedValuesSaslAuthenticationTestSuite,
       handleStart_autenticateAndReturnsSuccess_whenPasswordEmptyButValid) {
  const std::string empty_password = "";
  std::string sasl_login_string =
      get_sasl_message(expected_login, empty_password);
  EXPECT_CALL(mock_client, client_address()).WillOnce(Return(expected_host));
  EXPECT_CALL(mock_client, supports_expired_passwords())
      .WillOnce(Return(false));
  EXPECT_CALL(mock_client, client_hostname())
      .WillOnce(Return(expected_hostname.c_str()));
  EXPECT_CALL(mock_data_context, password_expired()).WillOnce(Return(false));
  EXPECT_CALL(
      mock_data_context,
      authenticate(StrEq(expected_login), StrEq(expected_hostname.c_str()),
                   StrEq(expected_host), StrEq(""), _, _, false))
      .WillOnce(Return(ec_success));

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "", iface::Authentication::Status::k_succeeded, 0);
}

TEST_F(ExpectedValuesSaslAuthenticationTestSuite,
       handleStart_autenticateAndReturnsSuccess_whenAuthSucceeded) {
  std::string sasl_login_string =
      get_sasl_message(expected_login, expected_password, expected_database);

  EXPECT_CALL(mock_client, client_address()).WillOnce(Return(expected_host));
  EXPECT_CALL(mock_client, supports_expired_passwords())
      .WillOnce(Return(false));
  EXPECT_CALL(mock_client, client_hostname())
      .WillOnce(Return(expected_hostname.c_str()));
  EXPECT_CALL(
      mock_data_context,
      authenticate(StrEq(expected_login), StrEq(expected_hostname),
                   StrEq(expected_host), StrEq(expected_database), _, _, false))
      .WillOnce(Return(ec_success));
  EXPECT_CALL(mock_data_context, password_expired()).WillOnce(Return(false));

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "", iface::Authentication::Status::k_succeeded, 0);
}

TEST_F(ExpectedValuesSaslAuthenticationTestSuite,
       handleStart_autenticateAndReturnsFailure_whenAuthFailures) {
  std::string sasl_login_string =
      get_sasl_message(expected_login, expected_password, expected_database);

  EXPECT_CALL(mock_client, client_address()).WillOnce(Return(expected_host));
  EXPECT_CALL(mock_client, client_hostname())
      .WillOnce(Return(expected_hostname.c_str()));
  EXPECT_CALL(mock_client, supports_expired_passwords())
      .WillOnce(Return(false));
  EXPECT_CALL(
      mock_data_context,
      authenticate(StrEq(expected_login), StrEq(expected_host),
                   StrEq(expected_host), StrEq(expected_database), _, _, false))
      .WillOnce(Return(ec_failur));

  iface::Authentication::Response result =
      sut->handle_start("", sasl_login_string, "");

  assert_responce(result, "", iface::Authentication::Status::k_failed, 1);
}

}  // namespace test

}  // namespace xpl
