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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#ifndef _WIN32
#include <netdb.h>
#endif

#include <cstdint>

#include "my_io.h"  // NOLINT(build/include_subdir)
#include "plugin/x/src/io/xpl_listener_tcp.h"
#include "unittest/gunit/xplugin/xpl/mock/ngs_general.h"

namespace xpl {
namespace test {

using namespace ::testing;  // NOLINT(build/namespaces)

const char *const ADDRESS = "0.1.2.3";
const char *const ALL_INTERFACES_4 = "0.0.0.0";
const char *const ALL_INTERFACES_6 = "::";
const uint16_t PORT = 3030;
const char *const PORT_STRING = "3030";
const uint32_t PORT_TIMEOUT = 123;
const uint32_t BACKLOG = 122;
const my_socket SOCKET_OK = 10;
const int POSIX_OK = 0;
const int POSIX_FAILURE = -1;

MATCHER(EqInvalidSocket, "") {
  return INVALID_SOCKET == mysql_socket_getfd(arg);
}

MATCHER_P(EqCastToCStr, expected, "") {
  std::string force_string = expected;
  return force_string == static_cast<char *>(arg);
}

class Listener_tcp_testsuite : public Test {
 public:
  void SetUp() {
    KEY_socket_x_tcpip = 1;

    m_mock_factory = std::make_shared<StrictMock<Mock_factory>>();
    m_mock_socket = std::make_shared<StrictMock<Mock_socket>>();
    m_mock_system = std::make_shared<StrictMock<Mock_system>>();
    m_mock_socket_invalid = std::make_shared<StrictMock<Mock_socket>>();

    ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  }

  void assert_verify_and_reinitailize_rules() {
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_factory.get()));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_socket_invalid.get()));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_socket.get()));
    ASSERT_TRUE(Mock::VerifyAndClearExpectations(m_mock_system.get()));

    EXPECT_CALL(*m_mock_factory, create_system_interface())
        .WillRepeatedly(Return(m_mock_system));
    EXPECT_CALL(*m_mock_factory, create_socket(EqInvalidSocket()))
        .WillRepeatedly(Return(m_mock_socket_invalid));
    EXPECT_CALL(*m_mock_socket_invalid, get_socket_fd())
        .WillRepeatedly(Return(INVALID_SOCKET));
    EXPECT_CALL(*m_mock_socket, get_socket_fd())
        .WillRepeatedly(Return(SOCKET_OK));
    ON_CALL(*m_mock_system, get_socket_error_and_message(_, _))
        .WillByDefault(DoAll(SetArgPointee<0>(0), SetArgPointee<1>("")));
  }

  void make_sut(const std::string &interface, const uint32_t port = PORT,
                const uint32_t port_timeout = PORT_TIMEOUT) {
    m_resulting_bind_address = interface;
    sut = std::make_shared<Listener_tcp>(
        m_mock_factory, std::ref(m_resulting_bind_address), "", port,
        port_timeout, std::ref(m_mock_socket_events), BACKLOG);
  }

  void expect_create_socket(addrinfo &ai, const std::string &interface,
                            const int family,
                            const int64_t result = SOCKET_OK) {
    make_sut(interface, PORT, PORT_TIMEOUT);

    EXPECT_CALL(*m_mock_system,
                getaddrinfo(StrEq(interface), StrEq(PORT_STRING), _, _))
        .WillOnce(DoAll(SetArgPointee<3>(&ai), Return(POSIX_OK)));

    EXPECT_CALL(*m_mock_socket, get_socket_fd()).WillOnce(Return(result));
    EXPECT_CALL(*m_mock_factory,
                create_socket(KEY_socket_x_tcpip, family, SOCK_STREAM, 0))
        .WillOnce(Return(m_mock_socket));

#ifdef IPV6_V6ONLY
    EXPECT_CALL(*m_mock_socket,
                set_socket_opt(IPPROTO_IPV6, IPV6_V6ONLY, _, sizeof(int)))
        .WillRepeatedly(Return(POSIX_OK));
#endif
  }

  void expect_listen_socket(std::shared_ptr<Mock_socket> mock_socket,
                            addrinfo &ai,
                            const bool socket_events_listen = true) {
    EXPECT_CALL(*mock_socket, set_socket_thread_owner());
    EXPECT_CALL(*mock_socket,
                bind(ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen)))
        .WillOnce(Return(POSIX_OK));
    EXPECT_CALL(*mock_socket, listen(BACKLOG)).WillOnce(Return(POSIX_OK));
    std::shared_ptr<iface::Socket> socket_ptr = mock_socket;
    EXPECT_CALL(m_mock_socket_events, listen(socket_ptr, _))
        .WillOnce(Return(socket_events_listen));
  }

  struct addrinfo get_ai_ipv6() {
    struct addrinfo result;
    static struct sockaddr_in6 in6;

    in6.sin6_family = result.ai_family = AF_INET6;
    result.ai_socktype = 0;
    result.ai_protocol = 0;
    result.ai_addrlen = sizeof(in6);
    result.ai_addr = reinterpret_cast<sockaddr *>(&in6);
    result.ai_next = nullptr;

    return result;
  }

  struct addrinfo get_ai_ipv4() {
    struct addrinfo result;
    static struct sockaddr_in in4;

    in4.sin_family = result.ai_family = AF_INET;
    result.ai_socktype = 0;
    result.ai_protocol = 0;
    result.ai_addrlen = sizeof(in4);
    result.ai_addr = reinterpret_cast<sockaddr *>(&in4);
    result.ai_next = nullptr;

    return result;
  }
  std::string m_resulting_bind_address;

  std::shared_ptr<Mock_socket> m_mock_socket;
  std::shared_ptr<Mock_socket> m_mock_socket_invalid;
  std::shared_ptr<Mock_system> m_mock_system;
  StrictMock<Mock_socket_events> m_mock_socket_events;
  std::shared_ptr<Mock_factory> m_mock_factory;

  std::shared_ptr<Listener_tcp> sut;
};

TEST_F(Listener_tcp_testsuite,
       setup_listener_does_nothing_when_resolve_failes) {
  make_sut(ADDRESS);

  EXPECT_CALL(*m_mock_system,
              getaddrinfo(StrEq(ADDRESS), StrEq(PORT_STRING), _, _))
      .WillOnce(Return(POSIX_FAILURE));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

TEST_F(
    Listener_tcp_testsuite,
    setup_listener_does_resolved_IP6_and_IP4_localhost_when_asterisk_and_IP6_supported) {  // NOLINT(whitespace/line_length)
  make_sut("*");

  EXPECT_CALL(*m_mock_socket, get_socket_fd()).WillOnce(Return(SOCKET_OK));
  EXPECT_CALL(*m_mock_factory,
              create_socket(static_cast<PSI_socket_key>(PSI_NOT_INSTRUMENTED),
                            AF_INET6, SOCK_STREAM, 0))
      .WillOnce(Return(m_mock_socket));

  EXPECT_CALL(*m_mock_system,
              getaddrinfo(StrEq(ALL_INTERFACES_6), StrEq(PORT_STRING), _, _))
      .WillOnce(Return(POSIX_FAILURE));

  EXPECT_CALL(*m_mock_system,
              getaddrinfo(StrEq(ALL_INTERFACES_4), StrEq(PORT_STRING), _, _))
      .WillOnce(Return(POSIX_FAILURE));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

TEST_F(
    Listener_tcp_testsuite,
    setup_listener_does_resolved_IP4_localhost_when_asterisk_and_IP6_not_supported) {  // NOLINT(whitespace/line_length)
  make_sut("*");

  EXPECT_CALL(*m_mock_socket, get_socket_fd()).WillOnce(Return(INVALID_SOCKET));
  EXPECT_CALL(*m_mock_factory,
              create_socket(static_cast<PSI_socket_key>(PSI_NOT_INSTRUMENTED),
                            AF_INET6, SOCK_STREAM, 0))
      .WillOnce(Return(m_mock_socket));

  EXPECT_CALL(*m_mock_system,
              getaddrinfo(StrEq(ALL_INTERFACES_4), StrEq(PORT_STRING), _, _))
      .WillOnce(Return(POSIX_FAILURE));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

struct TimeOutAndExpectedRetries {
  TimeOutAndExpectedRetries(const uint32_t timeout,
                            const uint32_t expected_retries)
      : m_timeout(timeout), m_expected_retries(expected_retries) {}

  uint32_t m_timeout;
  uint32_t m_expected_retries;
};

class Listener_tcp_retry_testsuite
    : public Listener_tcp_testsuite,
      public WithParamInterface<TimeOutAndExpectedRetries> {};

TEST_P(Listener_tcp_retry_testsuite,
       setup_listener_retry_socket_allocation_when_it_is_in_use) {
  addrinfo ai = get_ai_ipv6();

  make_sut(ALL_INTERFACES_6, PORT, GetParam().m_timeout);

  EXPECT_CALL(*m_mock_system,
              getaddrinfo(StrEq(ALL_INTERFACES_6), StrEq(PORT_STRING), _, _))
      .WillOnce(DoAll(SetArgPointee<3>(&ai), Return(POSIX_OK)));

  const int n = GetParam().m_expected_retries;

  ON_CALL(*m_mock_system, get_socket_error_and_message(_, _))
      .WillByDefault(DoAll(SetArgPointee<0>(0), SetArgPointee<1>("")));

  EXPECT_CALL(*m_mock_socket, get_socket_fd())
      .Times(n)
      .WillRepeatedly(Return(INVALID_SOCKET));
  EXPECT_CALL(*m_mock_factory,
              create_socket(KEY_socket_x_tcpip, AF_INET6, SOCK_STREAM, 0))
      .Times(n)
      .WillRepeatedly(Return(m_mock_socket));
  EXPECT_CALL(*m_mock_system, get_socket_error_and_message(_, _)).Times(n);
  EXPECT_CALL(*m_mock_system, get_socket_errno())
      .Times(n)
      .WillRepeatedly(Return(SOCKET_EADDRINUSE));
  EXPECT_CALL(*m_mock_system, sleep(Gt(0u))).Times(n);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

INSTANTIATE_TEST_CASE_P(
    Instantiation_tcp_retry_when_already_in_use, Listener_tcp_retry_testsuite,
    Values(TimeOutAndExpectedRetries(0, 1), TimeOutAndExpectedRetries(1, 2),
           TimeOutAndExpectedRetries(5, 3), TimeOutAndExpectedRetries(6, 3),
           TimeOutAndExpectedRetries(7, 4),
           TimeOutAndExpectedRetries(PORT_TIMEOUT, 10)));  // 123, 10

TEST_F(Listener_tcp_testsuite, setup_listener_bind_failure) {
  addrinfo ai = get_ai_ipv6();

  expect_create_socket(ai, ALL_INTERFACES_6, AF_INET6, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));
  EXPECT_CALL(*m_mock_socket, set_socket_thread_owner());

  EXPECT_CALL(*m_mock_socket, bind(ai.ai_addr, ai.ai_addrlen))
      .WillOnce(Return(POSIX_FAILURE));
  EXPECT_CALL(*m_mock_system, get_socket_error_and_message(_, _));
  EXPECT_CALL(*m_mock_system, get_socket_errno())
      .WillRepeatedly(Return(SOCKET_ETIMEDOUT));

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

TEST_F(Listener_tcp_testsuite, setup_listener_listen_failure) {
  addrinfo ai = get_ai_ipv6();

  expect_create_socket(ai, ALL_INTERFACES_6, AF_INET6, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _,
                             static_cast<socklen_t>(sizeof(int))))
      .WillOnce(Return(POSIX_OK));
  EXPECT_CALL(*m_mock_socket, set_socket_thread_owner());
  EXPECT_CALL(*m_mock_socket, bind(ai.ai_addr, ai.ai_addrlen))
      .WillOnce(Return(POSIX_OK));

  EXPECT_CALL(*m_mock_socket, listen(BACKLOG)).WillOnce(Return(POSIX_FAILURE));
  EXPECT_CALL(*m_mock_system, get_socket_error_and_message(_, _));
  EXPECT_CALL(*m_mock_system, get_socket_errno())
      .WillRepeatedly(Return(SOCKET_ETIMEDOUT));

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));
}

TEST_F(Listener_tcp_testsuite, setup_listener_ipv6_success) {
  addrinfo ai = get_ai_ipv6();

  expect_create_socket(ai, ALL_INTERFACES_6, AF_INET6, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  expect_listen_socket(m_mock_socket, ai);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*m_mock_socket, close());
}

TEST_F(Listener_tcp_testsuite, setup_listener_ipv4_success) {
  addrinfo ai = get_ai_ipv4();

  expect_create_socket(ai, ALL_INTERFACES_4, AF_INET, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  expect_listen_socket(m_mock_socket, ai);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*m_mock_socket, close());
}

TEST_F(Listener_tcp_testsuite,
       setup_listener_failure_when_socket_event_registry_failed) {
  addrinfo ai = get_ai_ipv4();

  expect_create_socket(ai, ALL_INTERFACES_4, AF_INET, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  const bool socket_event_listen_failed = false;
  expect_listen_socket(m_mock_socket, ai, socket_event_listen_failed);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_FALSE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_stopped));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
}

TEST_F(Listener_tcp_testsuite,
       setup_listener_ipv4_and_ip6_addresses_successful_is_ip4) {
  addrinfo ai4 = get_ai_ipv4();
  addrinfo ai6 = get_ai_ipv6();

  ai4.ai_next = &ai6;

  expect_create_socket(ai4, ALL_INTERFACES_4, AF_INET, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  expect_listen_socket(m_mock_socket, ai4);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai4));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*m_mock_socket, close());
}

TEST_F(
    Listener_tcp_testsuite,
    setup_listener_ipv4_and_ip6_addresses_successful_is_ip4_beacause_it_is_always_first_to_try) {  // NOLINT(whitespace/line_length)
  addrinfo ai4 = get_ai_ipv4();
  addrinfo ai6 = get_ai_ipv6();

  ai4.ai_next = &ai6;

  expect_create_socket(ai4, ALL_INTERFACES_6, AF_INET, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  expect_listen_socket(m_mock_socket, ai4);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai4));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*m_mock_socket, close());
}

TEST_F(Listener_tcp_testsuite,
       setup_listener_ipv4_and_ip6_addresses_successful_is_ip6_at_retry) {
  addrinfo ai4 = get_ai_ipv4();
  addrinfo ai6 = get_ai_ipv6();

  ai4.ai_next = &ai6;

  expect_create_socket(ai4, ALL_INTERFACES_6, AF_INET, INVALID_SOCKET);

  std::shared_ptr<Mock_socket> mock_socket_ipv6(new StrictMock<Mock_socket>());
  EXPECT_CALL(*mock_socket_ipv6, get_socket_fd()).WillOnce(Return(SOCKET_OK));
  EXPECT_CALL(*m_mock_factory,
              create_socket(KEY_socket_x_tcpip, AF_INET6, SOCK_STREAM, 0))
      .WillOnce(Return(mock_socket_ipv6));

#ifdef IPV6_V6ONLY
  EXPECT_CALL(*mock_socket_ipv6,
              set_socket_opt(IPPROTO_IPV6, IPV6_V6ONLY, _, sizeof(int)))
      .WillRepeatedly(Return(POSIX_OK));
#endif

  EXPECT_CALL(*mock_socket_ipv6,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_OK));

  expect_listen_socket(mock_socket_ipv6, ai6);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai4));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*mock_socket_ipv6, close());
}

TEST_F(Listener_tcp_testsuite, setup_listener_success_evean_socket_opt_fails) {
  addrinfo ai = get_ai_ipv6();

  expect_create_socket(ai, ALL_INTERFACES_6, AF_INET6, SOCKET_OK);

  EXPECT_CALL(*m_mock_socket,
              set_socket_opt(SOL_SOCKET, SO_REUSEADDR, _, sizeof(int)))
      .WillOnce(Return(POSIX_FAILURE));
  EXPECT_CALL(*m_mock_system, get_socket_errno());

  expect_listen_socket(m_mock_socket, ai);

  EXPECT_CALL(*m_mock_system, freeaddrinfo(&ai));

  ASSERT_TRUE(sut->setup_listener(nullptr));
  ASSERT_TRUE(sut->get_state().is(iface::Listener::State::k_prepared));

  // SUT destructor
  ASSERT_NO_FATAL_FAILURE(assert_verify_and_reinitailize_rules());
  EXPECT_CALL(*m_mock_socket, close());
}

TEST_F(Listener_tcp_testsuite, get_name_and_configuration) {
  make_sut(ALL_INTERFACES_6, 2222);

  ASSERT_STREQ("bind-address: '::' port: 2222",
               sut->get_name_and_configuration().c_str());
}

TEST_F(Listener_tcp_testsuite,
       close_listener_does_nothing_when_socket_not_started) {
  make_sut(ALL_INTERFACES_6);

  sut->close_listener();

  // After stopping, start must not work !
  sut->setup_listener(nullptr);
}

TEST_F(Listener_tcp_testsuite, loop_does_nothing_always) {
  make_sut(ALL_INTERFACES_6);

  sut->loop();
}

}  // namespace test
}  // namespace xpl
