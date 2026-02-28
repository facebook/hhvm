/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "plugin/x/ngs/include/ngs/server_client_timeout.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {

namespace test {

using namespace ::testing;  // NOLINT(build/namespaces)

// The chrono is missing string to Time_point conversion
// lets make initialize time constants in constructor
// relatively from now()
const xpl::chrono::Time_point TIMEPOINT_RELEASE_ALL_BEFORE = xpl::chrono::now();
const xpl::chrono::Duration DELTA_TO_RELEASE_1 =
    xpl::chrono::Milliseconds(-500);
const xpl::chrono::Duration DELTA_TO_RELEASE_2 =
    xpl::chrono::Milliseconds(-1000);
const xpl::chrono::Duration DELTA_TO_RELEASE_3 =
    xpl::chrono::Milliseconds(-2000);
const xpl::chrono::Duration DELTA_NOT_TO_RELEASE_1 =
    xpl::chrono::Milliseconds(2000);
const xpl::chrono::Duration DELTA_NOT_TO_RELEASE_2 =
    xpl::chrono::Milliseconds(1000);
const xpl::chrono::Duration DELTA_NOT_TO_RELEASE_3 =
    xpl::chrono::Milliseconds(500);

const xpl::chrono::Time_point TP_TO_RELEASE_1 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_TO_RELEASE_1;
const xpl::chrono::Time_point TP_TO_RELEASE_2 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_TO_RELEASE_2;
const xpl::chrono::Time_point TP_TO_RELEASE_3 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_TO_RELEASE_3;
const xpl::chrono::Time_point TP_NOT_TO_RELEASE_1 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_NOT_TO_RELEASE_1;
const xpl::chrono::Time_point TP_NOT_TO_RELEASE_2 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_NOT_TO_RELEASE_2;
const xpl::chrono::Time_point TP_NOT_TO_RELEASE_3 =
    TIMEPOINT_RELEASE_ALL_BEFORE + DELTA_NOT_TO_RELEASE_3;

class Server_client_timeout_test_suite : public Test {
 public:
  Server_client_timeout_test_suite()
      : sut(new ngs::Server_client_timeout(TIMEPOINT_RELEASE_ALL_BEFORE)) {}

  std::shared_ptr<iface::Client> expectClientValid(
      const xpl::chrono::Time_point &tp,
      const iface::Client::Client::State state) {
    std::shared_ptr<StrictMock<Mock_client>> result;

    result.reset(new StrictMock<Mock_client>());

    EXPECT_CALL(*result.get(), get_accept_time()).WillOnce(Return(tp));
    EXPECT_CALL(*result.get(), get_state()).WillOnce(Return(state));

    sut->validate_client_state(result);

    return result;
  }

  std::shared_ptr<iface::Client> expectClientNotValid(
      const xpl::chrono::Time_point &tp,
      const iface::Client::Client::State state) {
    std::shared_ptr<StrictMock<Mock_client>> result;

    result.reset(new StrictMock<Mock_client>());

    EXPECT_CALL(*result.get(), get_accept_time()).WillOnce(Return(tp));
    EXPECT_CALL(*result.get(), get_state()).WillOnce(Return(state));
    EXPECT_CALL(*result.get(), on_auth_timeout_void());
    EXPECT_CALL(*result.get(), client_id()).WillRepeatedly(Return(""));

    sut->validate_client_state(result);

    return result;
  }

  std::unique_ptr<ngs::Server_client_timeout> sut;
};

TEST_F(Server_client_timeout_test_suite,
       returnInvalidDate_whenNoClientWasProcessed) {
  ASSERT_FALSE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
}

struct ClientParams {
  ClientParams(const xpl::chrono::Duration &Duration,
               const iface::Client ::Client::State state)
      : m_Duration(Duration),
        m_tp(TIMEPOINT_RELEASE_ALL_BEFORE + Duration),
        m_state(state) {}

  xpl::chrono::Duration m_Duration;
  xpl::chrono::Time_point m_tp;
  iface::Client::State m_state;
};

void PrintTo(const ClientParams &x, ::std::ostream *os) {
  *os << "{ state:" << static_cast<int>(x.m_state)
      << ", Durations:" << x.m_Duration.count() << " }";
}

class ServerClientTimeoutTestSuiteWithClientsState
    : public Server_client_timeout_test_suite,
      public ::testing::WithParamInterface<ClientParams> {};

class ExpiredClient : public ServerClientTimeoutTestSuiteWithClientsState {};
class NoExpiredClient_stateNotOk
    : public ServerClientTimeoutTestSuiteWithClientsState {};
class NoExpiredClient_stateOk
    : public ServerClientTimeoutTestSuiteWithClientsState {};

TEST_P(ExpiredClient,
       returnInvalidDateNoFurtherNeedOfChecking_clientReleasedInitiated) {
  expectClientNotValid(GetParam().m_tp, GetParam().m_state);

  ASSERT_FALSE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
}

TEST_P(NoExpiredClient_stateNotOk,
       returnClientsAcceptanceDate_thereIsANeedOfFutureChecking) {
  const xpl::chrono::Time_point clients_tp = GetParam().m_tp;
  expectClientValid(clients_tp, GetParam().m_state);

  ASSERT_TRUE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
  ASSERT_EQ(clients_tp, sut->get_oldest_client_accept_time());
}

TEST_P(NoExpiredClient_stateOk,
       returnInvalidDate_clientRunsCorrectlyNoNeedOfFutureChecking) {
  const xpl::chrono::Time_point clients_tp = GetParam().m_tp;
  expectClientValid(clients_tp, GetParam().m_state);

  ASSERT_FALSE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
}

INSTANTIATE_TEST_CASE_P(
    InstantiationOfClientsThatExpiredAndAreInNotValidState, ExpiredClient,
    Values(ClientParams(DELTA_TO_RELEASE_1, iface::Client::State::k_accepted),
           ClientParams(DELTA_TO_RELEASE_2, iface::Client::State::k_accepted),
           ClientParams(DELTA_TO_RELEASE_3, iface::Client::State::k_accepted),
           ClientParams(DELTA_TO_RELEASE_1,
                        iface::Client::State::k_authenticating_first),
           ClientParams(DELTA_TO_RELEASE_2,
                        iface::Client::State::k_authenticating_first),
           ClientParams(DELTA_TO_RELEASE_3,
                        iface::Client::State::k_authenticating_first)));

INSTANTIATE_TEST_CASE_P(
    InstantiationOfClientsThatExpiredAndAreInNotValidState,
    NoExpiredClient_stateNotOk,
    Values(
        ClientParams(DELTA_NOT_TO_RELEASE_1, iface::Client::State::k_accepted),
        ClientParams(DELTA_NOT_TO_RELEASE_2, iface::Client::State::k_accepted),
        ClientParams(DELTA_NOT_TO_RELEASE_3, iface::Client::State::k_accepted),
        ClientParams(DELTA_NOT_TO_RELEASE_1,
                     iface::Client::State::k_authenticating_first),
        ClientParams(DELTA_NOT_TO_RELEASE_2,
                     iface::Client::State::k_authenticating_first),
        ClientParams(DELTA_NOT_TO_RELEASE_3,
                     iface::Client::State::k_authenticating_first)));

INSTANTIATE_TEST_CASE_P(
    InstantiationOfClientsThatNoExpiredAndAreInValidState,
    NoExpiredClient_stateOk,
    Values(
        ClientParams(DELTA_NOT_TO_RELEASE_1,
                     iface::Client::State::k_accepted_with_session),
        ClientParams(DELTA_NOT_TO_RELEASE_1, iface::Client::State::k_running),
        ClientParams(DELTA_NOT_TO_RELEASE_1, iface::Client::State::k_closing),
        ClientParams(DELTA_NOT_TO_RELEASE_1, iface::Client::State::k_closed),
        ClientParams(DELTA_TO_RELEASE_1,
                     iface::Client::State::k_accepted_with_session),
        ClientParams(DELTA_TO_RELEASE_1, iface::Client::State::k_running),
        ClientParams(DELTA_TO_RELEASE_1, iface::Client::State::k_closing),
        ClientParams(DELTA_TO_RELEASE_1, iface::Client::State::k_closed)));

TEST_F(
    Server_client_timeout_test_suite,
    returnDateOfOldestProcessedClient_whenMultipleValidNonAuthClientWereProcessed) {
  expectClientValid(TP_NOT_TO_RELEASE_1, iface::Client::State::k_accepted);
  expectClientValid(TP_NOT_TO_RELEASE_2, iface::Client::State::k_accepted);
  expectClientValid(TP_NOT_TO_RELEASE_3, iface::Client::State::k_accepted);

  ASSERT_TRUE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
  ASSERT_EQ(TP_NOT_TO_RELEASE_3, sut->get_oldest_client_accept_time());
}

class Server_client_timeout_test_suite_param
    : public Server_client_timeout_test_suite,
      public testing::WithParamInterface<iface::Client::State> {};

TEST_P(Server_client_timeout_test_suite_param,
       returnDateOfOldestNotExpiredNotAuthClient_whenWithMixedClientSet) {
  expectClientValid(TP_NOT_TO_RELEASE_1, iface::Client::State::k_accepted);
  expectClientValid(TP_NOT_TO_RELEASE_2, iface::Client::State::k_accepted);
  expectClientValid(TP_NOT_TO_RELEASE_3, iface::Client::State::k_accepted);
  expectClientNotValid(TP_TO_RELEASE_1, GetParam());

  ASSERT_TRUE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
  ASSERT_EQ(TP_NOT_TO_RELEASE_3, sut->get_oldest_client_accept_time());
}

INSTANTIATE_TEST_CASE_P(InstantiationOfTargetedStates,
                        Server_client_timeout_test_suite_param,
                        Values(iface::Client::State::k_invalid,
                               iface::Client::State::k_accepted,
                               iface::Client::State::k_authenticating_first));

TEST_F(Server_client_timeout_test_suite,
       returnInvalidDate_whenAllClientAreAuthenticated) {
  expectClientValid(TP_TO_RELEASE_1, iface::Client::State::k_running);
  expectClientValid(TP_TO_RELEASE_2, iface::Client::State::k_closing);
  expectClientValid(TP_TO_RELEASE_3, iface::Client::State::k_closing);

  ASSERT_FALSE(xpl::chrono::is_valid(sut->get_oldest_client_accept_time()));
}

}  // namespace test
}  // namespace xpl
