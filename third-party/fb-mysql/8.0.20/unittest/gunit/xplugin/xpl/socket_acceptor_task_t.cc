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

#include "plugin/x/ngs/include/ngs/socket_acceptors_task.h"
#include "plugin/x/src/interface/listener_factory.h"
#include "plugin/x/src/variables/system_variables.h"
#include "unittest/gunit/xplugin/xpl/mock/component_sys_variable_register.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Ref;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrEq;

const uint32_t k_backlog = 10;
const std::string k_unix_file = "unix test";
const std::string k_host = "host test";
const std::string k_net_ns = "";
const uint16_t k_port = 11;
const uint32_t k_open_timeout = 12;

class Socket_acceptors_task_suite : public ::testing::Test {
 public:
  void SetUp() override {
    xpl::Plugin_system_variables::initialize(&m_mock_sys_vars, {}, {}, false);
  }

  void TearDown() override { xpl::Plugin_system_variables::cleanup(); }

  Mock_listener_factory_interface m_mock_factory;
  ngs::test::Mock_service_sys_variables m_mock_sys_vars;
  std::shared_ptr<Mock_socket_events> m_mock_event{new Mock_socket_events()};
};

TEST_F(Socket_acceptors_task_suite, prepare_without_any_interface) {
  std::string skip_net = "NO";
  std::string socket = "some socket";
  EXPECT_CALL(m_mock_sys_vars, get_variable(_, StrEq("skip_networking"), _, _))
      .WillOnce(DoAll(SetArgPointee<2>(&skip_net[0]), Return(true)));
  EXPECT_CALL(m_mock_sys_vars, get_variable(_, StrEq("socket"), _, _))
      .WillOnce(DoAll(SetArgPointee<2>(&socket[0]), Return(true)));
  EXPECT_CALL(m_mock_factory,
              create_tcp_socket_listener_ptr(
                  StrEq(k_host), _, k_port, k_open_timeout,
                  Ref(*(iface::Socket_events *)m_mock_event.get()), k_backlog));

#if defined(HAVE_SYS_UN_H)
  EXPECT_CALL(m_mock_factory,
              create_unix_socket_listener_ptr(
                  StrEq(k_unix_file),
                  Ref(*(iface::Socket_events *)m_mock_event.get()), k_backlog));
#endif

  ngs::Socket_acceptors_task sut(m_mock_factory, k_host, "", k_port,
                                 k_open_timeout, k_unix_file, k_backlog,
                                 m_mock_event);

  iface::Server_task::Task_context context;
  ASSERT_FALSE(sut.prepare(&context));
}

}  // namespace test
}  // namespace xpl
