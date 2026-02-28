/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <atomic>
#include <chrono>
#include "gcs_base_test.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_communication_protocol_changer.h"

namespace gcs_xcom_communication_protocol_changer_unittest {

class GcsXcomCommunicationProtocolChangerTest : public GcsBaseTest {
 public:
  static std::atomic<bool> s_change_started;
  static std::future<void> s_change_finished;

  GcsXcomCommunicationProtocolChangerTest()
      : m_myself("127.0.0.1:8080"), m_engine(), m_pipeline(), m_changer() {}

  void SetUp() override {
    m_engine.initialize(nullptr);
    m_pipeline = std::make_unique<Gcs_message_pipeline>();
    ASSERT_FALSE(m_pipeline->register_pipeline({
        {Gcs_protocol_version::V1, {}},
        {Gcs_protocol_version::V2, {}},
    }));
    ASSERT_FALSE(m_pipeline->set_version(Gcs_protocol_version::V1));
    m_changer = std::make_unique<Gcs_xcom_communication_protocol_changer>(
        m_engine, *m_pipeline);
    static_cast<Gcs_xcom_interface *>(Gcs_xcom_interface::get_interface())
        ->set_node_address(m_myself.get_member_address());
  }

  void TearDown() override { m_engine.finalize(nullptr); }

 protected:
  Gcs_xcom_node_address m_myself;
  Gcs_xcom_engine m_engine;
  std::unique_ptr<Gcs_message_pipeline> m_pipeline;
  std::unique_ptr<Gcs_xcom_communication_protocol_changer> m_changer;
};

std::atomic<bool> GcsXcomCommunicationProtocolChangerTest::s_change_started;
std::future<void> GcsXcomCommunicationProtocolChangerTest::s_change_finished;

typedef void(xcom_set_protocol_functor)(
    Gcs_xcom_communication_protocol_changer *, Gcs_protocol_version);
/**
  Notification used to start a protocol change via the GCS engine.
*/
class Set_protocol_notification : public Parameterized_notification<false> {
 public:
  explicit Set_protocol_notification(
      xcom_set_protocol_functor *functor,
      Gcs_xcom_communication_protocol_changer *protocol_changer,
      Gcs_protocol_version version)
      : m_functor(functor), m_changer(protocol_changer), m_version(version) {}

  ~Set_protocol_notification() {}

 private:
  void do_execute() { (*m_functor)(m_changer, m_version); }

  xcom_set_protocol_functor *m_functor;
  Gcs_xcom_communication_protocol_changer *m_changer;
  Gcs_protocol_version m_version;
};

void do_function_set_protocol(Gcs_xcom_communication_protocol_changer *changer,
                              Gcs_protocol_version version) {
  bool change_started;
  std::future<void> change_finished;
  std::tie(change_started, change_finished) =
      changer->set_protocol_version(version);
  ASSERT_TRUE(change_started);

  GcsXcomCommunicationProtocolChangerTest::s_change_finished =
      std::move(change_finished);
  GcsXcomCommunicationProtocolChangerTest::s_change_started = true;
}

/*
 Validate that the sender successfully finishes the ongoing protocol change if
 the sender triggers the finish condition.
 */
TEST_F(GcsXcomCommunicationProtocolChangerTest,
       SenderThreadFinishesProtocolChange) {
  s_change_started = false;

  // Increment the number of packets in transit to one.
  m_changer->atomically_increment_nr_packets_in_transit(
      Cargo_type::CT_USER_DATA);

  // Start a protocol change, will wait for the in transit packet.
  Gcs_xcom_notification *notification = new Set_protocol_notification(
      do_function_set_protocol, m_changer.get(), Gcs_protocol_version::V2);
  ASSERT_NE(notification, nullptr);
  ASSERT_TRUE(m_engine.push(notification));

  // Wait until the change has actually started.
  while (!s_change_started)
    ;

  // Confirm it is indeed waiting for the in transit packet.
  using namespace std::chrono_literals;
  auto status = s_change_finished.wait_for(0ns);
  ASSERT_EQ(status, std::future_status::timeout);

  // Trick to set the number of packets in transit back to zero.
  m_changer->adjust_nr_packets_in_transit(Cargo_type::CT_USER_DATA, -1);

  /*
   This call is concurrent with the protocol change.
   It will increment the number of in transit packets to one, and then rollback
   to zero, triggering the condition to finish the protocol change.
  */
  m_changer->atomically_increment_nr_packets_in_transit(
      Cargo_type::CT_USER_DATA);

  // Wait for the change to complete, and confirm it.
  s_change_finished.get();
  ASSERT_EQ(m_pipeline->get_version(), Gcs_protocol_version::V2);
}

/*
 Validate that the sender successfully finishes the ongoing protocol change if
 the sender triggers the finish condition.
 */
TEST_F(GcsXcomCommunicationProtocolChangerTest,
       ReceiverThreadFinishesProtocolChange) {
  s_change_started = false;

  // Increment the number of packets in transit to one.
  m_changer->atomically_increment_nr_packets_in_transit(
      Cargo_type::CT_USER_DATA);

  // Start a protocol change, will wait for the in transit packet.
  std::tie(s_change_started, s_change_finished) =
      m_changer->set_protocol_version(Gcs_protocol_version::V2);
  ASSERT_TRUE(s_change_started);

  // Confirm it is indeed waiting for the in transit packet.
  using namespace std::chrono_literals;
  auto status = s_change_finished.wait_for(0ns);
  ASSERT_EQ(status, std::future_status::timeout);

  // Receive the required in transit packet.
  bool packet_ok;
  Gcs_packet packet;
  std::tie(packet_ok, packet) = Gcs_packet::make_outgoing_packet(
      Cargo_type::CT_USER_DATA, Gcs_protocol_version::V1, {}, {}, 1);
  ASSERT_TRUE(packet_ok);
  Gcs_xcom_nodes nodes;
  nodes.add_node(Gcs_xcom_node_information(m_myself.get_member_address(),
                                           Gcs_xcom_uuid(), 0, true));
  m_changer->decrement_nr_packets_in_transit(packet, nodes);

  // Wait for the change to complete, and confirm it.
  s_change_finished.get();
  ASSERT_EQ(m_pipeline->get_version(), Gcs_protocol_version::V2);
}

/*
 Validate that the protocol change is properly aborted if the desired
 protocol is unsupported.
 */
TEST_F(GcsXcomCommunicationProtocolChangerTest,
       SenderThreadFinishesProtocolChangeUnsupportedVersion) {
  s_change_started = false;

  // Start a protocol change.
  std::tie(s_change_started, s_change_finished) =
      m_changer->set_protocol_version(Gcs_protocol_version::MAXIMUM);

  // Confirm it indeed failed.
  ASSERT_FALSE(s_change_started);
  ASSERT_EQ(m_pipeline->get_version(), Gcs_protocol_version::V1);
}

/*
 Validate that the protocol change is properly aborted if the desired
 protocol is unsupported.
 */
TEST_F(GcsXcomCommunicationProtocolChangerTest,
       ReceiverThreadFinishesProtocolChangeUnsupportedVersion) {
  s_change_started = false;

  // Increment the number of packets in transit to one.
  m_changer->atomically_increment_nr_packets_in_transit(
      Cargo_type::CT_USER_DATA);

  // Start a protocol change.
  std::tie(s_change_started, s_change_finished) =
      m_changer->set_protocol_version(Gcs_protocol_version::MAXIMUM);

  // Confirm it is indeed failed.
  ASSERT_FALSE(s_change_started);

  // Receive the in transit packet. The receiver thread will try to
  // finish the protocol change.
  bool packet_ok;
  Gcs_packet packet;
  std::tie(packet_ok, packet) = Gcs_packet::make_outgoing_packet(
      Cargo_type::CT_USER_DATA, Gcs_protocol_version::V1, {}, {}, 1);
  ASSERT_TRUE(packet_ok);
  Gcs_xcom_nodes nodes;
  nodes.add_node(Gcs_xcom_node_information(m_myself.get_member_address(),
                                           Gcs_xcom_uuid(), 0, true));
  m_changer->decrement_nr_packets_in_transit(packet, nodes);

  ASSERT_EQ(m_pipeline->get_version(), Gcs_protocol_version::V1);
}

}  // namespace gcs_xcom_communication_protocol_changer_unittest
