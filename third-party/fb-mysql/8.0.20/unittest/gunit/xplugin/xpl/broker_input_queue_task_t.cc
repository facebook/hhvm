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
#include <chrono>  // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)

#include "plugin/x/src/mq/broker_task.h"
#include "plugin/x/src/mq/notice_input_queue.h"
#include "unittest/gunit/xplugin/xpl/mock/session.h"

namespace xpl {
namespace test {

using ::testing::_;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrictMock;
using ::testing::Test;

using Notice_type = ngs::Notice_type;
using Notice_descriptor = ngs::Notice_descriptor;

const auto kId1 = Notice_type::k_group_replication_quorum_loss;
const auto kId2 = Notice_type::k_group_replication_view_changed;

class Broker_input_queue_testsuite : public Test {
 public:
  using Task_context = iface::Server_task::Task_context;

 public:
  void SetUp() override {
    m_client_list.add(mock_client);
    sut_task->prepare(&m_sut_context);

    EXPECT_CALL(*mock_client, session()).WillRepeatedly(Return(&mock_session));
    EXPECT_CALL(mock_session, get_notice_output_queue())
        .WillRepeatedly(ReturnRef(mock_notice_out_queue));
  }

  void assert_do_loop() { sut_task->loop(); }

  std::shared_ptr<Mock_client> mock_client{new Mock_client()};
  Mock_session mock_session;
  StrictMock<Mock_notice_output_queue> mock_notice_out_queue;

  ngs::Client_list m_client_list;
  Task_context m_sut_context{Task_context::On_connection(), nullptr,
                             &m_client_list};
  Notice_input_queue m_sut_queue;
  std::unique_ptr<iface::Server_task> sut_task{
      m_sut_queue.create_broker_task()};
};

TEST_F(Broker_input_queue_testsuite, does_nothing) {}

TEST_F(Broker_input_queue_testsuite, queues_all_until_looped2) {
  m_sut_queue.emplace(kId1, "payload");
  m_sut_queue.emplace(kId2, "payload");

  EXPECT_CALL(mock_notice_out_queue, emplace(_, _)).Times(2);

  sut_task->loop();
}

TEST_F(Broker_input_queue_testsuite, queues_all_until_looped4) {
  m_sut_queue.emplace(kId1, "payload");
  m_sut_queue.emplace(kId2, "payload");
  m_sut_queue.emplace(kId2, "payload");

  EXPECT_CALL(mock_notice_out_queue, emplace(_, _)).Times(3);

  sut_task->loop();
}

TEST_F(Broker_input_queue_testsuite, publish_sequence_is_same_as_queue) {
  m_sut_queue.emplace(kId1, "payload");
  m_sut_queue.emplace(kId2, "payload");
  m_sut_queue.emplace(kId2, "payload");
  m_sut_queue.emplace(kId1, "payload");
  m_sut_queue.emplace(kId2, "payload");

  InSequence s;
  EXPECT_CALL(mock_notice_out_queue, emplace(kId1, _)).RetiresOnSaturation();
  EXPECT_CALL(mock_notice_out_queue, emplace(kId2, _)).RetiresOnSaturation();
  EXPECT_CALL(mock_notice_out_queue, emplace(kId2, _)).RetiresOnSaturation();
  EXPECT_CALL(mock_notice_out_queue, emplace(kId1, _)).RetiresOnSaturation();
  EXPECT_CALL(mock_notice_out_queue, emplace(kId2, _)).RetiresOnSaturation();

  sut_task->loop();
}

TEST_F(Broker_input_queue_testsuite, queues_one_by_one) {
  m_sut_queue.emplace(kId1, "payload");
  EXPECT_CALL(mock_notice_out_queue, emplace(kId1, _));
  assert_do_loop();

  m_sut_queue.emplace(kId2, "payload");
  EXPECT_CALL(mock_notice_out_queue, emplace(kId2, _));
  assert_do_loop();

  m_sut_queue.emplace(kId1, "payload");
  EXPECT_CALL(mock_notice_out_queue, emplace(kId1, _));
  assert_do_loop();
}

TEST_F(Broker_input_queue_testsuite, stop_blocks_until_task_ended) {
  bool stop_ended = false;

  sut_task->pre_loop();
  std::thread call_stop([&stop_ended, this]() {
    sut_task->stop(iface::Server_task::Stop_cause::k_normal_shutdown);
    stop_ended = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  ASSERT_FALSE(stop_ended);

  assert_do_loop();
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  ASSERT_FALSE(stop_ended);

  sut_task->post_loop();
  call_stop.join();
  ASSERT_TRUE(stop_ended);
}

}  // namespace test
}  // namespace xpl
