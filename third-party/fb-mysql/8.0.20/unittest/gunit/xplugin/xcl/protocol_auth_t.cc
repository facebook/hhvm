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

#include <string>

#include "unittest/gunit/xplugin/xcl/message_helpers.h"
#include "unittest/gunit/xplugin/xcl/protocol_t.h"

namespace xcl {
namespace test {

struct Auth_details {
  using Authenticate_start = ::Mysqlx::Session::AuthenticateStart;
  using Authenticate_continue = ::Mysqlx::Session::AuthenticateContinue;
  using Authenticate_ok = ::Mysqlx::Session::AuthenticateOk;

  std::string m_auth_name;
  Message_from_str<::Mysqlx::Session::AuthenticateStart> m_start_message;
  Message_from_str<::Mysqlx::Session::AuthenticateContinue> m_continue_message;
};

struct Auth_test_messages {
  const std::string expected_user = "user";
  const std::string expected_pass = "pass";
  const std::string expected_schema = "schema";

  const Message_from_str<Auth_details::Authenticate_start> msg_auth_start_plain{
      "mech_name: \"PLAIN\" "
      "auth_data: \"schema\\0user\\0pass\" "};

  const Message_from_str<Auth_details::Authenticate_start>
      msg_auth_start_mysql41{"mech_name: \"MYSQL41\" "};

  const Message_from_str<Auth_details::Authenticate_start>
      msg_auth_start_sha256_memory{"mech_name: \"SHA256_MEMORY\" "};

  const Message_from_str<Auth_details::Authenticate_continue>
      msg_auth_cont_mysql41{
          "auth_data: \"schema\\0user\\0"
          "*ACFC0C3FA7F3C1F39849B44177D8B82C7F75E0D1\""};

  const Message_from_str<Auth_details::Authenticate_continue>
      msg_auth_cont_sha256_memory{
          "auth_data: \"schema\\0user\\0"
          "8F7B8968AA5212A7FD28DAC463913BE1444D61BE7C891A242"
          "BA133460774D5CD\""};
};

class Xcl_protocol_impl_tests_auth
    : public Xcl_protocol_impl_tests,
      public ::testing::WithParamInterface<Auth_details> {
 public:
  void assert_authenticate(const std::string &mech,
                           const int32_t error_code = 0) {
    auto error = m_sut->execute_authenticate(m_messages.expected_user,
                                             m_messages.expected_pass,
                                             m_messages.expected_schema, mech);

    ASSERT_EQ(error_code, error.error());
  }

  Auth_test_messages m_messages;
};

TEST_F(Xcl_protocol_impl_tests_auth, execute_authenticate_invalid_method) {
  assert_authenticate("INVALID", CR_X_INVALID_AUTH_METHOD);
  assert_authenticate("plain", CR_X_INVALID_AUTH_METHOD);
  assert_authenticate("mysql41", CR_X_INVALID_AUTH_METHOD);
  assert_authenticate("sha256_memory", CR_X_INVALID_AUTH_METHOD);
}

TEST_F(Xcl_protocol_impl_tests_auth, execute_authenticate_plain_method) {
  expect_write_message(Auth_test_messages().msg_auth_start_plain.get());
  expect_read_message_without_payload(Auth_details::Authenticate_ok());

  assert_authenticate("PLAIN");
}

TEST_F(Xcl_protocol_impl_tests_auth,
       execute_authenticate_plain_method_error_msg) {
  const int32_t expected_error_code = 30001;
  auto msg_error = Server_message<::Mysqlx::Error>::make_required();

  msg_error.set_code(expected_error_code);

  expect_write_message(Auth_test_messages().msg_auth_start_plain.get());
  expect_read_message(msg_error);

  assert_authenticate("PLAIN", expected_error_code);
}

TEST_F(Xcl_protocol_impl_tests_auth,
       execute_authenticate_plain_method_unexpected_msg) {
  auto msg_stmt_ok =
      Server_message<::Mysqlx::Sql::StmtExecuteOk>::make_required();

  expect_write_message(Auth_test_messages().msg_auth_start_plain.get());
  expect_read_message_without_payload(msg_stmt_ok);

  assert_authenticate("PLAIN", CR_MALFORMED_PACKET);
}

TEST_F(Xcl_protocol_impl_tests_auth,
       execute_authenticate_plain_method_io_error_at_write) {
  const int32_t expected_error_code = 30002;

  expect_write_message(Auth_test_messages().msg_auth_start_plain.get(),
                       expected_error_code);

  assert_authenticate("PLAIN", expected_error_code);
}

TEST_F(Xcl_protocol_impl_tests_auth,
       execute_authenticate_plain_method_io_error_at_read) {
  const int32_t expected_error_code = 30003;

  expect_write_message(Auth_test_messages().msg_auth_start_plain.get());
  expect_read_message_without_payload(Auth_details::Authenticate_ok(),
                                      expected_error_code);

  assert_authenticate("PLAIN", expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method) {
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message(msg_auth_cont_s);
    expect_write_message(GetParam().m_continue_message.get());
    expect_read_message_without_payload(Auth_details::Authenticate_ok());
  }

  assert_authenticate(GetParam().m_auth_name);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_error_msg1) {
  const int32_t expected_error_code = 30004;
  auto msg_error = Server_message<::Mysqlx::Error>::make_required();

  msg_error.set_code(expected_error_code);

  expect_write_message(GetParam().m_start_message.get());
  expect_read_message(msg_error);

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_error_msg2) {
  const int32_t expected_error_code = 30005;
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  auto msg_error = Server_message<::Mysqlx::Error>::make_required();

  msg_error.set_code(expected_error_code);

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message(msg_auth_cont_s);
    expect_write_message(GetParam().m_continue_message.get());
    expect_read_message(msg_error);
  }

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_unexpected_msg1) {
  auto msg_unexpected = Server_message<::Mysqlx::Ok>::make_required();

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message_without_payload(msg_unexpected);
  }

  assert_authenticate(GetParam().m_auth_name, CR_MALFORMED_PACKET);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_unexpected_msg2) {
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  auto msg_unexpected = Server_message<::Mysqlx::Ok>::make_required();

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message(msg_auth_cont_s);
    expect_write_message(GetParam().m_continue_message.get());
    expect_read_message_without_payload(msg_unexpected);
  }

  assert_authenticate(GetParam().m_auth_name, CR_MALFORMED_PACKET);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_io_error1) {
  const int32_t expected_error_code = 30006;
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  expect_write_message(GetParam().m_start_message.get());
  expect_read_message_without_payload(msg_auth_cont_s, expected_error_code);

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_recv_io_error2) {
  const int32_t expected_error_code = 30007;
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message(msg_auth_cont_s);
    expect_write_message(GetParam().m_continue_message.get());
    expect_read_message_without_payload(Auth_details::Authenticate_ok(),
                                        expected_error_code);
  }

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_write_io_error1) {
  const int32_t expected_error_code = 30008;

  expect_write_message(GetParam().m_start_message.get(), expected_error_code);

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

TEST_P(Xcl_protocol_impl_tests_auth,
       execute_authenticate_challenge_response_method_write_io_error2) {
  const int32_t expected_error_code = 30009;
  auto msg_auth_cont_s =
      Server_message<Auth_details::Authenticate_continue>::make_required();

  {
    InSequence s;

    expect_write_message(GetParam().m_start_message.get());
    expect_read_message(msg_auth_cont_s);
    expect_write_message(GetParam().m_continue_message.get(),
                         expected_error_code);
  }

  assert_authenticate(GetParam().m_auth_name, expected_error_code);
}

INSTANTIATE_TEST_CASE_P(
    InstantiationAuthDetails, Xcl_protocol_impl_tests_auth,
    ::testing::Values(
        Auth_details{"MYSQL41", Auth_test_messages().msg_auth_start_mysql41,
                     Auth_test_messages().msg_auth_cont_mysql41},
        Auth_details{"SHA256_MEMORY",
                     Auth_test_messages().msg_auth_start_sha256_memory,
                     Auth_test_messages().msg_auth_cont_sha256_memory}));

}  // namespace test
}  // namespace xcl
