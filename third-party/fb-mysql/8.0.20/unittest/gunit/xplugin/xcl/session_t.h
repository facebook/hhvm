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

#ifndef UNITTEST_GUNIT_XPLUGIN_XCL_SESSION_T_H_
#define UNITTEST_GUNIT_XPLUGIN_XCL_SESSION_T_H_

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "errmsg.h"
#include "plugin/x/client/xsession_impl.h"
#include "unittest/gunit/xplugin/xcl/mock/connection.h"
#include "unittest/gunit/xplugin/xcl/mock/connection_state.h"
#include "unittest/gunit/xplugin/xcl/mock/factory.h"
#include "unittest/gunit/xplugin/xcl/mock/protocol.h"

namespace xcl {
namespace test {

using ::testing::_;
using ::testing::An;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SaveArg;
using ::testing::StrictMock;
using ::testing::Test;
using ::testing::Values;
using ::testing::WithParamInterface;

class Xcl_session_impl_tests : public Test {
 public:
  void SetUp() override { m_sut = make_sut(false); }

  std::unique_ptr<Session_impl> prepare_session() {
    std::unique_ptr<Session_impl> result;
    /** Create mock and return it to the `Session_impl` object.
     The object takes ownership of the pointer. We hold a copy
     of it to setup mocking expectations.
     */
    m_mock_protocol = new StrictMock<Mock_protocol>();
    m_mock_factory = new StrictMock<Mock_factory>();

    EXPECT_CALL(*m_mock_factory, create_protocol_raw(_))
        .WillOnce(DoAll(Invoke(this, &Xcl_session_impl_tests::assign_configs),
                        Return(m_mock_protocol)));
    EXPECT_CALL(*m_mock_protocol, add_notice_handler(_, Handler_position::End,
                                                     Handler_priority_high))
        .WillOnce(DoAll(SaveArg<0>(&m_out_message_handler), Return(2)));
    EXPECT_CALL(*m_mock_protocol, add_notice_handler(_, Handler_position::End,
                                                     Handler_priority_low))
        .WillOnce(Return(1));
    EXPECT_CALL(*m_mock_protocol, get_connection())
        .WillRepeatedly(ReturnRef(m_mock_connection));
    EXPECT_CALL(m_mock_connection, state())
        .WillRepeatedly(ReturnRef(m_mock_connection_state));
    EXPECT_CALL(*m_mock_protocol, use_compression(_, _))
        .WillRepeatedly(Return());

    result.reset(
        new Session_impl(std::unique_ptr<Protocol_factory>{m_mock_factory}));

    return result;
  }

  std::unique_ptr<Session_impl> make_sut(const bool is_connected,
                                         const std::string &auth = "MYSQL41") {
    auto result = prepare_session();

    EXPECT_CALL(m_mock_connection_state, is_connected())
        .WillRepeatedly(Return(false));
    result->set_mysql_option(
        xcl::XSession::Mysqlx_option::Authentication_method, auth);
    ::testing::Mock::VerifyAndClearExpectations(&m_mock_connection_state);
    EXPECT_CALL(m_mock_connection_state, is_connected())
        .WillRepeatedly(Return(is_connected));

    return result;
  }

  void expect_connection_close() { EXPECT_CALL(m_mock_connection, close()); }

  bool encode_session_state_change(
      const Mysqlx::Notice::SessionStateChanged_Parameter param,
      const uint64_t value, std::string *out_payload) {
    Mysqlx::Notice::SessionStateChanged session_state_changed;
    std::string session_state_changed_binary;

    session_state_changed.set_param(param);

    auto v = session_state_changed.mutable_value()->Add();
    v->set_type(Mysqlx::Datatypes::Scalar::V_UINT);
    v->set_v_unsigned_int(value);

    return session_state_changed.SerializeToString(out_payload);
  }

  void assign_configs(const std::shared_ptr<Context> &context) {
    m_out_ssl_config = &context->m_ssl_config;
    m_out_connection_config = &context->m_connection_config;
  }

  XError assert_ssl_mode(const std::string &value) {
    return m_sut->set_mysql_option(XSession::Mysqlx_option::Ssl_mode, value);
  }

  XError assert_resolve_to(const std::string &value) {
    return m_sut->set_mysql_option(XSession::Mysqlx_option::Hostname_resolve_to,
                                   value);
  }

  XProtocol::Notice_handler m_out_message_handler;
  const Ssl_config *m_out_ssl_config{nullptr};
  const Connection_config *m_out_connection_config{nullptr};
  StrictMock<Mock_protocol> *m_mock_protocol{nullptr};
  StrictMock<Mock_connection> m_mock_connection;
  StrictMock<Mock_connection_state> m_mock_connection_state;
  StrictMock<Mock_factory> *m_mock_factory;
  std::unique_ptr<Session_impl> m_sut;
};

}  // namespace test
}  // namespace xcl

#endif  // UNITTEST_GUNIT_XPLUGIN_XCL_SESSION_T_H_
