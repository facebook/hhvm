/*
 *
 Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.
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
#include <initializer_list>
#include <memory>
#include <set>

#include "mysqlx_error.h"
#include "unittest/gunit/xplugin/xcl/message_helpers.h"
#include "unittest/gunit/xplugin/xcl/session_t.h"

namespace xcl {
namespace test {

template <typename E>
class Flags {
 public:
  Flags(const std::initializer_list<E> &set) : m_set(set) {}

  bool has(const E e) const { return 0 != m_set.count(e); }

 private:
  std::set<E> m_set;
};

class Xcl_session_negotiation_tests : public Xcl_session_impl_tests {
 public:
  enum Action_session {
    Action_ssl_configured,
    Action_connect,
    Action_compression,
  };

  void setup_state(const Flags<Action_session> config,
                   const std::string ssl_mode,
                   const std::string compression_mode) {
    EXPECT_CALL(m_mock_connection_state, is_ssl_configured())
        .WillRepeatedly(Return(config.has(Action_ssl_configured)));

    EXPECT_CALL(m_mock_connection_state, is_ssl_activated())
        .WillRepeatedly(Return(false));
    EXPECT_CALL(m_mock_connection_state, get_connection_type())
        .WillOnce(Return(Connection_type::Tcp));
    EXPECT_CALL(*m_mock_protocol, add_send_message_handler(_, _, _));
    EXPECT_CALL(*m_mock_protocol, remove_send_message_handler(_))
        .Times(testing::AtLeast(1));
    EXPECT_CALL(m_mock_connection, set_read_timeout(_))
        .WillRepeatedly(Return(XError{}));
    EXPECT_CALL(m_mock_connection, set_write_timeout(_))
        .WillRepeatedly(Return(XError{}));
    EXPECT_CALL(*m_mock_protocol, add_notice_handler(_, Handler_position::Begin,
                                                     Handler_priority_low))
        .Times(testing::AtLeast(1));
    EXPECT_CALL(*m_mock_protocol, remove_notice_handler(_))
        .Times(testing::AtLeast(1));

    EXPECT_CALL(m_mock_connection, connect(_, 33060, _))
        .WillOnce(Return(XError{0, ""}));

    if (config.has(Action_connect)) {
      EXPECT_CALL(*m_mock_protocol,
                  use_compression(config.has(Action_compression)
                                      ? Compression_algorithm::k_deflate
                                      : Compression_algorithm::k_none));

      EXPECT_CALL(*m_mock_protocol, execute_authenticate("", "", "", "MYSQL41"))
          .WillOnce(Return(XError{}));
    }

    ASSERT_FALSE(
        m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_mode, ssl_mode));
    ASSERT_FALSE(m_sut->set_mysql_option(
        XSession::Mysqlx_option::Compression_negotiation_mode,
        compression_mode));
  }

  void setup_level(const int64_t level) {
    ASSERT_FALSE(m_sut->set_mysql_option(
        XSession::Mysqlx_option::Compression_level_server, level));
  }

  void expect_tls(const int error_code = 0) {
    EXPECT_CALL(*m_mock_protocol,
                execute_set_capability(Cmp_msg(m_capability_set_tls.get())))
        .WillOnce(Return(XError(error_code, "")));

    if (0 == error_code) {
      EXPECT_CALL(m_mock_connection, activate_tls());
    }
  }

  void expect_fetch_capabilities(Mysqlx::Connection::Capabilities &src_caps) {
    auto capabilities = src_caps.New();
    capabilities->CopyFrom(src_caps);
    EXPECT_CALL(*m_mock_protocol, execute_fetch_capabilities_raw(_))
        .WillOnce(Return(capabilities));
  }

  void expect_set_capabilities(Mysqlx::Connection::CapabilitiesSet &cap_set,
                               int error_code = 0,
                               const bool is_fatal = false) {
    EXPECT_CALL(*m_mock_protocol, execute_set_capability(Cmp_msg(cap_set)))
        .WillOnce(Return(XError(error_code, "", is_fatal)));
  }

  Message_from_str<Mysqlx::Connection::CapabilitiesSet>
      m_capability_set_compression{
          "capabilities {"
          "  capabilities {"
          "    name: \"compression\""
          "    value {"
          "      type: OBJECT"
          "      obj {"
          "        fld {"
          "          key: \"algorithm\""
          "          value {"
          "            type: SCALAR"
          "            scalar {"
          "              type: V_STRING"
          "              v_string {"
          "                value: \"DEFLATE_STREAM\""
          "              }"
          "            }"
          "          }"
          "        }"
          "        fld {"
          "          key: \"server_combine_mixed_messages\""
          "          value {"
          "            type: SCALAR"
          "            scalar {"
          "              type: V_BOOL"
          "              v_bool: true"
          "            }"
          "          }"
          "        }"
          "        fld {"
          "          key: \"server_max_combine_messages\""
          "          value {"
          "            type: SCALAR"
          "            scalar {"
          "              type: V_SINT"
          "              v_signed_int: 0"
          "            }"
          "          }"
          "        }"
          "      }"
          "    }"
          "  }"
          "}"};

  Message_from_str<Mysqlx::Connection::CapabilitiesSet>
      m_capability_set_compression_with_level{
          " capabilities {"
          "   capabilities {"
          "     name: \"compression\""
          "     value {"
          "       type: OBJECT"
          "       obj {"
          "         fld {"
          "           key: \"algorithm\""
          "           value {"
          "             type: SCALAR"
          "             scalar {"
          "               type: V_STRING"
          "               v_string {"
          "                 value: \"DEFLATE_STREAM\""
          "               }"
          "             }"
          "           }"
          "         }"
          "         fld {"
          "           key: \"level\""
          "           value {"
          "             type: SCALAR"
          "             scalar {"
          "               type: V_SINT"
          "               v_signed_int: 3"
          "             }"
          "           }"
          "         }"
          "         fld {"
          "           key: \"server_combine_mixed_messages\""
          "           value {"
          "             type: SCALAR"
          "             scalar {"
          "               type: V_BOOL"
          "               v_bool: true"
          "             }"
          "           }"
          "         }"
          "         fld {"
          "           key: \"server_max_combine_messages\""
          "           value {"
          "             type: SCALAR"
          "             scalar {"
          "               type: V_SINT"
          "               v_signed_int: 0"
          "             }"
          "           }"
          "         }"
          "       }"
          "     }"
          "   }"
          " }"};

  Message_from_str<Mysqlx::Connection::CapabilitiesSet> m_capability_set_tls{
      "capabilities {"
      "  capabilities {"
      "    name: \"tls\""
      "    value {"
      "      type: SCALAR"
      "      scalar {"
      "           type: V_BOOL"
      "           v_bool: true"
      "      }"
      "    }"
      "  }"
      "}"};

  Message_from_str<Mysqlx::Connection::Capabilities> m_capabilities{
      " capabilities {"
      "    name: \"authentication.mechanisms\""
      "    value {"
      "      type: ARRAY"
      "      array {"
      "        value {"
      "          type: SCALAR"
      "          scalar {"
      "            type: V_STRING"
      "            v_string {"
      "              value: \"MYSQL41\""
      "            }"
      "          }"
      "        }"
      "        value {"
      "          type: SCALAR"
      "          scalar {"
      "            type: V_STRING"
      "            v_string {"
      "              value: \"SHA256_MEMORY\""
      "            }"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"
      "  capabilities {"
      "    name: \"doc.formats\""
      "    value {"
      "      type: SCALAR"
      "      scalar {"
      "        type: V_STRING"
      "        v_string {"
      "          value: \"text\""
      "        }"
      "      }"
      "    }"
      "  }"
      "  capabilities {"
      "    name: \"client.interactive\""
      "    value {"
      "      type: SCALAR"
      "      scalar {"
      "        type: V_BOOL"
      "        v_bool: false"
      "      }"
      "    }"
      "  }"
      "  capabilities {"
      "    name: \"compression\""
      "    value {"
      "      type: OBJECT"
      "      obj {"
      "        fld {"
      "          key: \"algorithm\""
      "          value {"
      "            type: ARRAY"
      "            array {"
      "              value {"
      "                type: SCALAR"
      "                scalar {"
      "                  type: V_STRING"
      "                  v_string {"
      "                    value: \"deflate_stream\""
      "                  }"
      "                }"
      "              }"
      "              value {"
      "                type: SCALAR"
      "                scalar {"
      "                  type: V_STRING"
      "                  v_string {"
      "                    value: \"lz4_message\""
      "                  }"
      "                }"
      "              }"
      "              value {"
      "                type: SCALAR"
      "                scalar {"
      "                  type: V_STRING"
      "                  v_string {"
      "                    value: \"zstd_stream\""
      "                  }"
      "                }"
      "              }"
      "            }"
      "          }"
      "        }"
      "      }"
      "    }"
      "  }"};

  Message_from_str<Mysqlx::Connection::Capabilities>
      m_capabilities_no_compression{
          " capabilities {"
          "    name: \"authentication.mechanisms\""
          "    value {"
          "      type: ARRAY"
          "      array {"
          "        value {"
          "          type: SCALAR"
          "          scalar {"
          "            type: V_STRING"
          "            v_string {"
          "              value: \"MYSQL41\""
          "            }"
          "          }"
          "        }"
          "        value {"
          "          type: SCALAR"
          "          scalar {"
          "            type: V_STRING"
          "            v_string {"
          "              value: \"SHA256_MEMORY\""
          "            }"
          "          }"
          "        }"
          "      }"
          "    }"
          "  }"
          "  capabilities {"
          "    name: \"doc.formats\""
          "    value {"
          "      type: SCALAR"
          "      scalar {"
          "        type: V_STRING"
          "        v_string {"
          "          value: \"text\""
          "        }"
          "      }"
          "    }"
          "  }"
          "  capabilities {"
          "    name: \"client.interactive\""
          "    value {"
          "      type: SCALAR"
          "      scalar {"
          "        type: V_BOOL"
          "        v_bool: false"
          "      }"
          "    }"
          "  }"};
};

TEST_F(Xcl_session_negotiation_tests, xsession_no_negotiation) {
  setup_state({Action_connect}, "DISABLED", "DISABLED");

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests, xsession_SSL_REQUIRED_server_support) {
  setup_state({Action_connect, Action_ssl_configured}, "REQUIRED", "DISABLED");

  expect_tls();

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_SSL_REQUIRED_server_doesnt_support) {
  setup_state({Action_ssl_configured}, "REQUIRED", "DISABLED");

  expect_tls(ER_X_CAPABILITIES_PREPARE_FAILED);

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_TRUE(error);
}

TEST_F(Xcl_session_negotiation_tests, xsession_SSL_PREFERRED_server_support) {
  setup_state({Action_connect, Action_ssl_configured}, "PREFERRED", "DISABLED");

  expect_tls();

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_SSL_PREFERRED_server_doesnt_support) {
  setup_state({Action_connect, Action_ssl_configured}, "PREFERRED", "DISABLED");

  expect_tls(ER_X_CAPABILITIES_PREPARE_FAILED);

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_REQUIRED_server_doesnt_support) {
  setup_state({}, "DISABLED", "REQUIRED");

  expect_fetch_capabilities(m_capabilities_no_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_TRUE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_REQUIRED_server_support) {
  setup_state({Action_connect, Action_compression}, "DISABLED", "REQUIRED");

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_PREFERRED_server_doesnt_support) {
  setup_state({Action_connect}, "DISABLED", "PREFERRED");

  expect_fetch_capabilities(m_capabilities_no_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_PREFERRED_server_support) {
  setup_state({Action_connect, Action_compression}, "DISABLED", "PREFERRED");

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_REQUIRED_WITH_LEVEL_server_doesnt_support_LEVEL) {
  setup_state({Action_connect, Action_compression}, "DISABLED", "REQUIRED");
  setup_level(3);

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression_with_level.get(),
                          ER_X_CAPABILITIES_PREPARE_FAILED);
  expect_set_capabilities(m_capability_set_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_REQUIRED_WITH_LEVEL_server_fatal_error) {
  setup_state({}, "DISABLED", "REQUIRED");
  setup_level(3);

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression_with_level.get(),
                          ER_X_CAPABILITIES_PREPARE_FAILED, true);

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_TRUE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_COMPRESSION_PREFERRED_WITH_LEVEL_server_doesnt_support_LEVEL) {
  setup_state({Action_connect, Action_compression}, "DISABLED", "PREFERRED");
  setup_level(3);

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression_with_level.get(),
                          ER_X_CAPABILITIES_PREPARE_FAILED);
  expect_set_capabilities(m_capability_set_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(
    Xcl_session_negotiation_tests,
    xsession_COMPRESSION_PREFERRED_WITH_LEVEL_server_doesnt_support_compression) {
  setup_state({Action_connect}, "DISABLED", "PREFERRED");
  setup_level(3);

  expect_fetch_capabilities(m_capabilities_no_compression.get());

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

TEST_F(Xcl_session_negotiation_tests,
       xsession_SSL_REQUIRED_and_COMPRESSION_REQUIRED_server_support) {
  setup_state({Action_connect, Action_compression, Action_ssl_configured},
              "REQUIRED", "REQUIRED");

  expect_fetch_capabilities(m_capabilities.get());
  expect_set_capabilities(m_capability_set_compression.get());
  expect_tls();

  const auto error = m_sut->connect(nullptr, 0, nullptr, nullptr, nullptr);

  ASSERT_FALSE(error);
}

}  // namespace test
}  // namespace xcl
