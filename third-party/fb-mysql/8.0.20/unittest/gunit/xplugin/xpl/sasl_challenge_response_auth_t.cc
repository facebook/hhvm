/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "plugin/x/src/auth_challenge_response.h"
#include "plugin/x/src/sql_user_require.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {

static const uint8_t ER_SUCCESS = 0;

namespace test {

using ::testing::_;

namespace {
const char *const EMPTY = "";
const char *const AUTH_DATA = "ALA_MA_KOTA";
const char *const SALT = "SALT";

::testing::AssertionResult assert_response(
    const char *e1_expr, const char *e2_expr,
    const iface::Authentication::Response &e1,
    const iface::Authentication::Response &e2) {
  return (e1.data == e2.data && e1.status == e2.status &&
          e1.error_code == e2.error_code)
             ? ::testing::AssertionSuccess()
             : (::testing::AssertionFailure()
                << "Value of: " << e2_expr << "\nActual: {"
                << static_cast<uint32_t>(e2.status) << ", " << e2.error_code
                << ", " << e2.data << "}\n"
                << "Expected: " << e1_expr << "\nWhich is: {"
                << static_cast<uint32_t>(e1.status) << ", " << e1.error_code
                << ", " << e1.data << "}");
}

template <typename T1, typename T2>
void ASSERT_RESPONSE(const T1 &a, const T2 &b) {
  ASSERT_PRED_FORMAT2(assert_response, a, b);
}

}  // namespace

struct Auth_selector {
  std::string m_name;
  iface::Account_verification::Account_type m_verificator_type;
};

class Sasl_challenge_response_auth_test
    : public ::testing::TestWithParam<Auth_selector> {
 public:
  void SetUp() {
    if (GetParam().m_name == "SHA256_MEMORY")
      auth = std::unique_ptr<Sasl_sha256_memory_auth>(
          new Sasl_sha256_memory_auth(mock_handler));
    else if (GetParam().m_name == "MYSQL41")
      auth = std::unique_ptr<Sasl_mysql41_auth>(
          new Sasl_mysql41_auth(mock_handler));
    else
      throw std::logic_error("Invalid test case auth method");
  }

  ::testing::StrictMock<Mock_account_verification_handler> *mock_handler{
      new ::testing::StrictMock<Mock_account_verification_handler>(nullptr)};
  std::unique_ptr<iface::Authentication> auth;
  ::testing::StrictMock<Mock_account_verification> mock_account_verification;
  ::testing::StrictMock<Mock_authentication_interface> mock_authentication;

  using Response = iface::Authentication::Response;
};

TEST_P(Sasl_challenge_response_auth_test, handle_start_get_salt) {
  EXPECT_CALL(*mock_handler,
              get_account_verificator(GetParam().m_verificator_type))
      .WillOnce(::testing::Return(&mock_account_verification));

  EXPECT_CALL(mock_account_verification, get_salt())
      .WillOnce(::testing::ReturnRefOfCopy(std::string(SALT)));

  ASSERT_RESPONSE(
      Response(iface::Authentication::Status::k_ongoing, ER_SUCCESS, SALT),
      auth->handle_start(GetParam().m_name, AUTH_DATA, EMPTY));
}

TEST_P(Sasl_challenge_response_auth_test, handle_start_call_twice) {
  EXPECT_CALL(*mock_handler,
              get_account_verificator(GetParam().m_verificator_type))
      .WillOnce(::testing::Return(&mock_account_verification));

  EXPECT_CALL(mock_account_verification, get_salt())
      .WillOnce(::testing::ReturnRefOfCopy(std::string(SALT)));

  ASSERT_RESPONSE(
      Response(iface::Authentication::Status::k_ongoing, ER_SUCCESS, SALT),
      auth->handle_start(GetParam().m_name, AUTH_DATA, EMPTY));
  ASSERT_RESPONSE(Response(iface::Authentication::Status::k_error,
                           ER_NET_PACKETS_OUT_OF_ORDER, EMPTY),
                  auth->handle_start(GetParam().m_name, AUTH_DATA, EMPTY));
}

TEST_P(Sasl_challenge_response_auth_test,
       handle_continue_without_previous_start) {
  ASSERT_RESPONSE(Response(iface::Authentication::Status::k_error,
                           ER_NET_PACKETS_OUT_OF_ORDER, EMPTY),
                  auth->handle_continue(AUTH_DATA));
}

TEST_P(Sasl_challenge_response_auth_test, handle_continue_succeeded) {
  EXPECT_CALL(*mock_handler,
              get_account_verificator(GetParam().m_verificator_type))
      .WillOnce(::testing::Return(&mock_account_verification));

  EXPECT_CALL(mock_account_verification, get_salt())
      .WillOnce(::testing::ReturnRefOfCopy(std::string(SALT)));

  ASSERT_RESPONSE(
      Response(iface::Authentication::Status::k_ongoing, ER_SUCCESS, SALT),
      auth->handle_start(GetParam().m_name, AUTH_DATA, EMPTY));

  EXPECT_CALL(*mock_handler, authenticate(_, _, AUTH_DATA))
      .WillOnce(::testing::Return(ngs::Success()));

  ASSERT_RESPONSE(
      Response(iface::Authentication::Status::k_succeeded, ER_SUCCESS, EMPTY),
      auth->handle_continue(AUTH_DATA));
}

TEST_P(Sasl_challenge_response_auth_test, handle_continue_failed) {
  EXPECT_CALL(*mock_handler,
              get_account_verificator(GetParam().m_verificator_type))
      .WillOnce(::testing::Return(&mock_account_verification));

  EXPECT_CALL(mock_account_verification, get_salt())
      .WillOnce(::testing::ReturnRefOfCopy(std::string(SALT)));

  ASSERT_RESPONSE(
      Response(iface::Authentication::Status::k_ongoing, ER_SUCCESS, SALT),
      auth->handle_start(GetParam().m_name, AUTH_DATA, EMPTY));

  ngs::Error_code expect_error(ER_NO_SUCH_USER, "Invalid user or password");
  EXPECT_CALL(*mock_handler, authenticate(_, _, AUTH_DATA))
      .WillOnce(::testing::Return(expect_error));

  ASSERT_RESPONSE(Response(iface::Authentication::Status::k_failed,
                           expect_error.error, expect_error.message),
                  auth->handle_continue(AUTH_DATA));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_auth_mechanism, Sasl_challenge_response_auth_test,
    ::testing::Values(
        Auth_selector{
            "SHA256_MEMORY",
            iface::Account_verification::Account_type::k_sha256_memory},
        Auth_selector{"MYSQL41",
                      iface::Account_verification::Account_type::k_native}));

}  // namespace test
}  // namespace xpl
