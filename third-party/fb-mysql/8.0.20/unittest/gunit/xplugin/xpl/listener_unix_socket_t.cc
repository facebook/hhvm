/*
 * Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include <errno.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

#include "my_config.h"  // NOLINT(build/include_subdir)
#include "my_io.h"      // NOLINT(build/include_subdir)

#include "plugin/x/src/io/xpl_listener_unix_socket.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"

namespace xpl {
namespace test {

using namespace ::testing;  // NOLINT(build/namespaces)

const uint32_t BACKLOG = 122;
const my_socket SOCKET_OK = 10;
const int BIND_OK = 0;
const int LISTEN_ERR = -1;
const int LISTEN_OK = 0;
const int OPEN_ERR = -1;
const int READ_ERR = -1;
const int WRITE_ERR = -1;
const int UNLINK_ERR = -1;
const int UNLINK_OK = 0;
const int FSYNC_ERR = -1;
const int FSYNC_OK = 0;
const int CLOSE_ERR = -1;
const int CLOSE_OK = 0;
const int CURRENT_PID = 6;
const char *const UNIX_SOCKET_FILE_CONTENT = "X6\n";  // "X%d" % CURRENT_PID
const char *const UNIX_SOCKET_FILE = "/tmp/xplugin_test.sock";
const char *const UNIX_SOCKET_LOCK_FILE = "/tmp/xplugin_test.sock.lock";

MATCHER(EqInvalidSocket, "") {
  return INVALID_SOCKET == mysql_socket_getfd(arg);
}

MATCHER_P(EqCastToCStr, expected, "") {
  std::string force_string = expected;
  return force_string == static_cast<char *>(arg);
}

class Listener_unix_socket_testsuite : public Test {
 public:
  void SetUp() {
    m_mock_factory = std::make_shared<StrictMock<Mock_factory>>();
    m_mock_socket = std::make_shared<StrictMock<Mock_socket>>();
    m_mock_system = std::make_shared<StrictMock<Mock_system>>();
    m_mock_file = std::make_shared<StrictMock<Mock_file>>();
    m_mock_socket_invalid = std::make_shared<StrictMock<Mock_socket>>();
    m_mock_file_invalid = std::make_shared<StrictMock<Mock_file>>();

    EXPECT_CALL(*m_mock_factory, create_system_interface())
        .WillRepeatedly(Return(m_mock_system));
    EXPECT_CALL(*m_mock_factory, create_socket(EqInvalidSocket()))
        .WillRepeatedly(Return(m_mock_socket_invalid));
    EXPECT_CALL(*m_mock_file_invalid, is_valid()).WillRepeatedly(Return(false));
    EXPECT_CALL(*m_mock_socket_invalid, get_socket_fd())
        .WillRepeatedly(Return(INVALID_SOCKET));
    EXPECT_CALL(*m_mock_socket, get_socket_fd())
        .WillRepeatedly(Return(SOCKET_OK));
    EXPECT_CALL(*m_mock_file, is_valid()).WillRepeatedly(Return(true));

    sut = std::make_shared<Listener_unix_socket>(
        m_mock_factory, UNIX_SOCKET_FILE, std::ref(m_mock_socket_events),
        BACKLOG);
  }

  void assert_valid_lock_file() {
    EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
    EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
        .WillOnce(Return(m_mock_file));
    EXPECT_CALL(*m_mock_file, write(EqCastToCStr(UNIX_SOCKET_FILE_CONTENT),
                                    strlen(UNIX_SOCKET_FILE_CONTENT)))
        .WillOnce(Return(strlen(UNIX_SOCKET_FILE_CONTENT)));
    EXPECT_CALL(*m_mock_file, fsync()).WillOnce(Return(FSYNC_OK));
    EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(CLOSE_OK));
  }

  void assert_setup_listener_successful() {
    ASSERT_NO_FATAL_FAILURE(assert_valid_lock_file());

    EXPECT_CALL(*m_mock_factory, create_socket(_, AF_UNIX, SOCK_STREAM, 0))
        .WillOnce(Return(m_mock_socket));
    EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_FILE)));
    EXPECT_CALL(*m_mock_socket, bind(_, _)).WillOnce(Return(BIND_OK));
    EXPECT_CALL(*m_mock_socket, listen(_)).WillOnce(Return(LISTEN_OK));
    EXPECT_CALL(*m_mock_socket, get_socket_fd())
        .WillOnce(Return(SOCKET_OK))         // after create_socket()
        .WillRepeatedly(Return(SOCKET_OK));  // back in setup_listener()
    EXPECT_CALL(*m_mock_socket, set_socket_thread_owner());

    std::shared_ptr<iface::Socket> socket = m_mock_socket;
    EXPECT_CALL(m_mock_socket_events, listen(socket, _)).WillOnce(Return(true));

    ASSERT_TRUE(sut->setup_listener(nullptr));
    ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

    ASSERT_NO_FATAL_FAILURE(assert_and_clear_mocks());
  }

  void assert_and_clear_mocks() {
    // In SUT destructor
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_system.get()));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_socket.get()));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(&m_mock_socket_events));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_factory.get()));
  }

  std::shared_ptr<Mock_socket> m_mock_socket;
  std::shared_ptr<Mock_socket> m_mock_socket_invalid;
  std::shared_ptr<Mock_system> m_mock_system;
  std::shared_ptr<Mock_file> m_mock_file_invalid;
  std::shared_ptr<Mock_file> m_mock_file;
  StrictMock<Mock_socket_events> m_mock_socket_events;
  std::shared_ptr<Mock_factory> m_mock_factory;

  std::shared_ptr<Listener_unix_socket> sut;
};

ACTION_P(SetArg0ToChar, value) { *static_cast<char *>(arg0) = value; }
ACTION_P(SetArg0ToChar2, value) { (static_cast<char *>(arg0)[1]) = value; }

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_try_to_create_empty_unixsocket_filename) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(
      m_mock_factory, "", std::ref(m_mock_socket_events), BACKLOG);

  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_try_to_create_unixsocket_with_too_long_filename) {
#if defined(HAVE_SYS_UN_H)
  std::string long_filename(2000, 'a');
  sut = std::make_shared<Listener_unix_socket>(
      m_mock_factory, long_filename, std::ref(m_mock_socket_events), BACKLOG);

  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_cant_create_a_lockfile) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(OPEN_ERR));
  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_cant_open_existing_lockfile) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file_invalid));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_cant_read_existing_lockfile) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_file, read(_, _)).WillOnce(Return(READ_ERR));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_read_empty_lockfile) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_file, read(_, _)).WillOnce(Return(0));
  EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(0));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_read_not_x_plugin_lockfile) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_file, read(_, _))
      .WillOnce(DoAll(SetArg0ToChar('Y'), Return(1)))
      .WillOnce(Return(0));
  EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(CLOSE_OK));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_read_x_plugin_lockfile_but_cant_kill) {
#if defined(HAVE_SYS_UN_H)
  const int expected_pid = 5;
  const char char_pid = '0' + expected_pid;
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_file, read(_, _))
      .WillOnce(DoAll(SetArg0ToChar('X'), SetArg0ToChar2(char_pid), Return(2)))
      .WillOnce(Return(0));
  EXPECT_CALL(*m_mock_system, get_ppid()).WillOnce(Return(4));
  EXPECT_CALL(*m_mock_system, kill(expected_pid, _)).WillOnce(Return(0));
  EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(CLOSE_OK));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_read_x_plugin_lockfile_same_process_but_cant_unlink) {
#if defined(HAVE_SYS_UN_H)
  const int expected_pid = 6;
  const char char_pid = '0' + expected_pid;
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file_invalid))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_system, get_errno()).WillOnce(Return(EEXIST));
  EXPECT_CALL(*m_mock_file, read(_, _))
      .WillOnce(DoAll(SetArg0ToChar('X'), SetArg0ToChar2(char_pid), Return(2)))
      .WillOnce(Return(0));
  EXPECT_CALL(*m_mock_system, get_ppid()).WillOnce(Return(expected_pid));
  EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(CLOSE_OK));
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_LOCK_FILE)))
      .WillOnce(Return(UNLINK_ERR));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_write_x_plugin_lockfile_failed) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_file, write(_, _)).WillOnce(Return(WRITE_ERR));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_fsync_x_plugin_lockfile_failed) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_file, write(EqCastToCStr(UNIX_SOCKET_FILE_CONTENT),
                                  strlen(UNIX_SOCKET_FILE_CONTENT)))
      .WillOnce(Return(strlen(UNIX_SOCKET_FILE_CONTENT)));
  EXPECT_CALL(*m_mock_file, fsync()).WillOnce(Return(FSYNC_ERR));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       unixsocket_close_x_plugin_lockfile_failed) {
#if defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_system, get_pid()).WillOnce(Return(CURRENT_PID));
  EXPECT_CALL(*m_mock_factory, open_file(StrEq(UNIX_SOCKET_LOCK_FILE), _, _))
      .WillOnce(Return(m_mock_file));
  EXPECT_CALL(*m_mock_file, write(EqCastToCStr(UNIX_SOCKET_FILE_CONTENT),
                                  strlen(UNIX_SOCKET_FILE_CONTENT)))
      .WillOnce(Return(strlen(UNIX_SOCKET_FILE_CONTENT)));
  EXPECT_CALL(*m_mock_file, fsync()).WillOnce(Return(FSYNC_OK));
  EXPECT_CALL(*m_mock_file, close()).WillOnce(Return(CLOSE_ERR));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_create_socket_failed) {
#if defined(HAVE_SYS_UN_H)
  ASSERT_NO_FATAL_FAILURE(assert_valid_lock_file());

  EXPECT_CALL(*m_mock_factory, create_socket(_, AF_UNIX, SOCK_STREAM, 0))
      .WillOnce(Return(m_mock_socket_invalid));
  EXPECT_CALL(*m_mock_system, get_socket_error_and_message(_, _))
      .WillOnce(DoAll(SetArgPointee<0>(0), SetArgPointee<1>("")));
  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_listen_failed) {
#if defined(HAVE_SYS_UN_H)
  ASSERT_NO_FATAL_FAILURE(assert_valid_lock_file());

  EXPECT_CALL(*m_mock_factory, create_socket(_, AF_UNIX, SOCK_STREAM, 0))
      .WillOnce(Return(m_mock_socket));
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_FILE)));
  EXPECT_CALL(*m_mock_socket, bind(_, _)).WillOnce(Return(BIND_OK));
  EXPECT_CALL(*m_mock_socket, listen(_)).WillOnce(Return(LISTEN_ERR));
  EXPECT_CALL(*m_mock_system, get_socket_error_and_message(_, _))
      .WillOnce(DoAll(SetArgPointee<0>(0), SetArgPointee<1>("")));
  EXPECT_CALL(*m_mock_socket, get_socket_fd())
      .WillOnce(Return(SOCKET_OK))              // before m_mock_socket.close()
      .WillRepeatedly(Return(INVALID_SOCKET));  // after  m_mock_socket.close()
  EXPECT_CALL(*m_mock_socket, close())
      .Times(2);  // first call in setup_listener
                  // second call in SUT destructor

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_event_regiester_failure) {
#if defined(HAVE_SYS_UN_H)
  ASSERT_NO_FATAL_FAILURE(assert_valid_lock_file());

  EXPECT_CALL(*m_mock_factory, create_socket(_, AF_UNIX, SOCK_STREAM, 0))
      .WillOnce(Return(m_mock_socket));
  // First time before creating new file
  // Second time at closure
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_FILE))).Times(2);
  EXPECT_CALL(*m_mock_socket, bind(_, _)).WillOnce(Return(BIND_OK));
  EXPECT_CALL(*m_mock_socket, listen(_)).WillOnce(Return(LISTEN_OK));
  EXPECT_CALL(*m_mock_socket, get_socket_fd())
      .WillOnce(Return(SOCKET_OK))         // after create_socket()
      .WillRepeatedly(Return(SOCKET_OK));  // back in setup_listener()
  EXPECT_CALL(*m_mock_socket, set_socket_thread_owner());

  std::shared_ptr<iface::Socket> socket = m_mock_socket;
  EXPECT_CALL(m_mock_socket_events, listen(socket, _)).WillOnce(Return(false));

  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_LOCK_FILE)))
      .WillOnce(Return(UNLINK_OK));

  EXPECT_CALL(*m_mock_socket, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));

#endif
}

TEST_F(Listener_unix_socket_testsuite, unixsocket_event_successful) {
#if defined(HAVE_SYS_UN_H)
  ASSERT_NO_FATAL_FAILURE(assert_setup_listener_successful());

  // Required by SUT destructor
  EXPECT_CALL(*m_mock_factory, create_system_interface())
      .WillRepeatedly(Return(m_mock_system));
  EXPECT_CALL(*m_mock_socket, get_socket_fd()).WillOnce(Return(SOCKET_OK));
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_LOCK_FILE)))
      .WillOnce(Return(UNLINK_OK));
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_FILE)))
      .WillOnce(Return(UNLINK_OK));

  EXPECT_CALL(*m_mock_socket, close());
#endif
}

TEST_F(Listener_unix_socket_testsuite, unix_socket_unsupported) {
#if !defined(HAVE_SYS_UN_H)
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  EXPECT_CALL(*m_mock_socket_invalid, close());

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
#endif
}

TEST_F(Listener_unix_socket_testsuite,
       close_listener_does_nothing_when_not_started) {
  sut = std::make_shared<Listener_unix_socket>(m_mock_factory, UNIX_SOCKET_FILE,
                                               std::ref(m_mock_socket_events),
                                               BACKLOG);

  sut->close_listener();
}

TEST_F(Listener_unix_socket_testsuite, close_listener_closes_valid_socket) {
#if defined(HAVE_SYS_UN_H)
  ASSERT_NO_FATAL_FAILURE(assert_setup_listener_successful());

  EXPECT_CALL(*m_mock_factory, create_system_interface())
      .WillRepeatedly(Return(m_mock_system));
  EXPECT_CALL(*m_mock_socket, get_socket_fd()).WillOnce(Return(SOCKET_OK));
  EXPECT_CALL(*m_mock_socket, close());
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_LOCK_FILE)))
      .WillOnce(Return(UNLINK_OK));
  EXPECT_CALL(*m_mock_system, unlink(StrEq(UNIX_SOCKET_FILE)))
      .WillOnce(Return(UNLINK_OK));
  sut->close_listener();

  ASSERT_NO_FATAL_FAILURE(assert_and_clear_mocks());
#endif
}

}  // namespace test
}  // namespace xpl
