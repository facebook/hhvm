/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include <memory>

#include <cstdint>

#include "plugin/x/ngs/include/ngs/protocol_decoder.h"
#include "plugin/x/src/operations_factory.h"
#include "plugin/x/src/variables/system_variables.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SetArrayArgument;
using ::testing::StrictMock;

ACTION_P2(SetSocketErrnoAndReturn, err, result) {
  xpl::Operations_factory operations_factory;

  operations_factory.create_system_interface()->set_socket_errno(err);

  return result;
}

const uint32_t k_max_wait_timeout_in_sec = 2;
const uint32_t k_max_read_timeout_in_sec = 2;

class Protocol_decoder_test_suite : public ::testing::Test {
 public:
  using Strict_mock_client = StrictMock<xpl::test::Mock_client>;
  using Strict_mock_vio = StrictMock<Mock_vio>;
  using Strict_mock_pmonitor = StrictMock<Mock_protocol_monitor>;
  using Strict_Mock_wait_for_io = Mock_wait_for_io;

 public:
  void SetUp() override {
    EXPECT_CALL(*m_mock_vio, set_state(_)).Times(AtLeast(1));
    EXPECT_CALL(*m_mock_vio, get_mysql_socket())
        .WillRepeatedly(ReturnRef(m_socket));
    EXPECT_CALL(*m_mock_vio, set_timeout_in_ms(_, _)).Times(AtLeast(1));
    m_config_global->m_timeouts.m_wait_timeout = k_max_wait_timeout_in_sec;
    m_config_global->m_timeouts.m_read_timeout = k_max_read_timeout_in_sec;

    m_sut.reset(new ngs::Protocol_decoder(&m_mock_dispatcher, m_mock_vio,
                                          &m_mock_protocol_monitor, m_config));
  }

  void TearDown() override { m_config_global->m_timeouts = {}; }

  MYSQL_SOCKET m_socket{INVALID_SOCKET, nullptr};
  const std::vector<unsigned char> m_msg{
      1, 0, 0, 0, 1};  // 1 = size, 0, 0, 0, 1 = Msg_CapGet

  std::shared_ptr<Strict_mock_vio> m_mock_vio{new Strict_mock_vio()};
  std::shared_ptr<ngs::Protocol_global_config> m_config_global{
      new ngs::Protocol_global_config()};
  std::shared_ptr<ngs::Protocol_config> m_config{
      new ngs::Protocol_config(m_config_global)};

  Strict_mock_pmonitor m_mock_protocol_monitor;
  Strict_Mock_wait_for_io m_mock_wait_for_io;
  Mock_message_dispatcher m_mock_dispatcher;

  std::unique_ptr<ngs::Protocol_decoder> m_sut;
};

/*
 * Test checks if protocol_decoder does't call 'before read' callback
 * in case when reporting is forbidden.
 */
TEST_F(Protocol_decoder_test_suite, no_need_for_idle_reporting_read_msg) {
  {
    InSequence s;
    EXPECT_CALL(m_mock_wait_for_io, has_to_report_idle_waiting())
        .WillOnce(Return(false));
    EXPECT_CALL(m_mock_wait_for_io, on_idle_or_before_read()).Times(0);
    EXPECT_CALL(*m_mock_vio, read(_, _))
        .WillOnce(DoAll(SetArrayArgument<0>(m_msg.begin(), m_msg.end()),
                        Return(m_msg.size())));
    EXPECT_CALL(m_mock_dispatcher, handle(_));
    EXPECT_CALL(m_mock_protocol_monitor, on_receive(_));
  }

  m_sut->read_and_decode(&m_mock_wait_for_io);
}

/*
 * Test checks if protocol_decoder calls 'before read' callback
 * in case when reporting is required.
 */
TEST_F(Protocol_decoder_test_suite, need_idle_reporting_read_msg) {
  {
    InSequence s;
    EXPECT_CALL(m_mock_wait_for_io, has_to_report_idle_waiting())
        .WillOnce(Return(true));
    EXPECT_CALL(m_mock_wait_for_io, on_idle_or_before_read());
    EXPECT_CALL(*m_mock_vio, read(_, _))
        .WillOnce(DoAll(SetArrayArgument<0>(m_msg.begin(), m_msg.end()),
                        Return(m_msg.size())));
    EXPECT_CALL(m_mock_dispatcher, handle(_));
    EXPECT_CALL(m_mock_protocol_monitor, on_receive(_));
  }

  m_sut->read_and_decode(&m_mock_wait_for_io);
}

/*
 * Test generates multiple read timeouts that sum of those timeouts
 * exceeds `k_max_wait_timeout_in_sec`. In such case Protocol_decoder
 * must generate an error.
 */
TEST_F(Protocol_decoder_test_suite,
       need_idle_reporting_mutiple_idle_timeouts_exceeds_wait_timeout) {
  {
    InSequence s;
    EXPECT_CALL(m_mock_wait_for_io, has_to_report_idle_waiting())
        .WillOnce(Return(true));
    EXPECT_CALL(m_mock_wait_for_io, on_idle_or_before_read())
        .RetiresOnSaturation();

    // the `wait_timeout` is exceeded after 4 `read` calls
    // For first three (nonfatal) the idle function is called.
    for (int i = 0; i < 3; ++i) {
      EXPECT_CALL(*m_mock_vio, read(_, _))
          .WillOnce(SetSocketErrnoAndReturn(SOCKET_ETIMEDOUT, -1))
          .RetiresOnSaturation();
      EXPECT_CALL(m_mock_wait_for_io, on_idle_or_before_read())
          .RetiresOnSaturation();
    }

    // For last failing read operation there is no
    // `on_idle_or_before_read` call.
    EXPECT_CALL(*m_mock_vio, read(_, _))
        .WillOnce(SetSocketErrnoAndReturn(SOCKET_ETIMEDOUT, -1))
        .RetiresOnSaturation();
  }

  m_sut->read_and_decode(&m_mock_wait_for_io);
}

}  // namespace test
}  // namespace xpl
