/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <vector>

#include "gcs_base_test.h"

#include "gcs_xcom_state_exchange.h"
#include "mysql/gcs/gcs_interface.h"

#include "gcs_xcom_communication_interface.h"
#include "mysql/gcs/gcs_communication_interface.h"
#include "mysql/gcs/gcs_control_interface.h"

#include "gcs_internal_message.h"
#include "gcs_xcom_utils.h"

namespace gcs_xcom_state_exchange_unittest {

class mock_gcs_control_interface : public Gcs_control_interface {
 public:
  MOCK_METHOD0(join, enum_gcs_error());
  MOCK_METHOD0(leave, enum_gcs_error());
  MOCK_METHOD0(belongs_to_group, bool());
  MOCK_METHOD0(get_current_view, Gcs_view *());
  MOCK_CONST_METHOD0(get_local_member_identifier,
                     const Gcs_member_identifier());
  MOCK_METHOD0(get_minimum_write_concurrency, uint32_t());
  MOCK_METHOD0(get_maximum_write_concurrency, uint32_t());
  MOCK_METHOD1(get_write_concurrency,
               enum_gcs_error(uint32_t &write_concurrency));
  MOCK_METHOD1(set_write_concurrency,
               enum_gcs_error(uint32_t write_concurrency));
  MOCK_METHOD1(set_xcom_cache_size, enum_gcs_error(uint64_t));
  MOCK_METHOD1(add_event_listener,
               int(const Gcs_control_event_listener &event_listener));
  MOCK_METHOD1(remove_event_listener, void(int event_listener_handle));
};

class mock_gcs_xcom_communication_interface
    : public Gcs_xcom_communication_interface {
 public:
  MOCK_METHOD1(send_message,
               enum_gcs_error(const Gcs_message &message_to_send));
  MOCK_METHOD1(add_event_listener,
               int(const Gcs_communication_event_listener &event_listener));
  MOCK_METHOD1(remove_event_listener, void(int event_listener_handle));
  MOCK_METHOD3(do_send_message,
               enum_gcs_error(const Gcs_message &message_to_send,
                              unsigned long long *message_length,
                              Cargo_type type));
  /* Mocking fails compilation on Windows. It attempts to copy the
   * std::unique_ptr which is non-copyable. */
  void buffer_incoming_packet(Gcs_packet &&packet,
                              std::unique_ptr<Gcs_xcom_nodes> &&xcom_nodes) {
    buffer_incoming_packet_mock(packet, xcom_nodes);
  }
  MOCK_METHOD2(buffer_incoming_packet_mock,
               void(Gcs_packet &packet,
                    std::unique_ptr<Gcs_xcom_nodes> &xcom_nodes));
  MOCK_METHOD0(deliver_buffered_packets, void());
  MOCK_METHOD0(cleanup_buffered_packets, void());
  MOCK_METHOD0(number_buffered_packets, size_t());
  MOCK_METHOD2(update_members_information, void(const Gcs_member_identifier &me,
                                                const Gcs_xcom_nodes &members));
  MOCK_METHOD1(
      recover_packets,
      bool(std::unordered_set<Gcs_xcom_synode> const &required_synods));
  /*
   Mocking fails compilation on Windows. It attempts to copy the
   std::unique_ptr which is non-copyable.
   */
  Gcs_message *convert_packet_to_message(
      Gcs_packet &&packet, std::unique_ptr<Gcs_xcom_nodes> &&xcom_nodes) {
    return convert_packet_to_message_mock(packet, xcom_nodes);
  }
  MOCK_METHOD2(convert_packet_to_message_mock,
               Gcs_message *(Gcs_packet &packet,
                             std::unique_ptr<Gcs_xcom_nodes> &xcom_nodes));
  /*
   Mocking fails compilation on Windows. It attempts to copy the
   std::unique_ptr which is non-copyable.
   */
  void process_user_data_packet(Gcs_packet &&packet,
                                std::unique_ptr<Gcs_xcom_nodes> &&xcom_nodes) {
    process_user_data_packet_mock(packet, xcom_nodes);
  }
  MOCK_METHOD2(process_user_data_packet_mock,
               void(Gcs_packet &packet,
                    std::unique_ptr<Gcs_xcom_nodes> &xcom_nodes));
  MOCK_CONST_METHOD0(get_protocol_version, Gcs_protocol_version());
  /*
   Mocking fails compilation on Windows. It attempts to copy the std::future
   which is non-copyable.
   */
  std::pair<bool, std::future<void>> set_protocol_version(
      Gcs_protocol_version new_version) {
    auto future = std::async([this, new_version]() {
      return set_protocol_version_mock(new_version);
    });
    return std::make_pair(true, std::move(future));
  }
  MOCK_METHOD1(set_protocol_version_mock,
               void(Gcs_protocol_version new_version));
  MOCK_METHOD2(update_in_transit,
               void(Gcs_message const &message, Cargo_type cargo));
  MOCK_CONST_METHOD0(is_protocol_change_ongoing, bool());
  MOCK_METHOD1(set_groups_maximum_supported_protocol_version,
               void(Gcs_protocol_version version));
  MOCK_CONST_METHOD0(get_maximum_supported_protocol_version,
                     Gcs_protocol_version());

  virtual Gcs_message_pipeline &get_msg_pipeline() { return m_msg_pipeline; }

 private:
  Gcs_message_pipeline m_msg_pipeline;
};

class XComStateExchangeTest : public GcsBaseTest {
 protected:
  XComStateExchangeTest() {}

  virtual void SetUp() {
    control_mock = new mock_gcs_control_interface();
    comm_mock = new mock_gcs_xcom_communication_interface();
    state_exchange = new Gcs_xcom_state_exchange(comm_mock);
  }

  virtual void TearDown() {
    delete state_exchange;
    delete comm_mock;
    delete control_mock;
  }

  void DecodeStateExchangeMessage(
      Gcs_protocol_version encoder_protocol_version) {
    /* Encode a message. */
    uchar *buffer = nullptr;
    uchar *slider = nullptr;
    uint64_t buffer_len = 0;
    uint64_t exchangeable_header_len = 0;
    uint64_t exchangeable_data_len = 0;
    uint64_t exchangeable_snapshot_len = 0;

    Gcs_xcom_view_identifier dummy_view_id(1, 1);
    synode_no dummy_cfg_id;
    dummy_cfg_id.group_id = 1;
    dummy_cfg_id.msgno = 1;
    dummy_cfg_id.node = 1;
    Gcs_xcom_synode_set dummy_snapshot;
    std::string encoded_payload = "I am sooper dooper payload";

    Xcom_member_state encoded_member_state(dummy_view_id, dummy_cfg_id,
                                           encoder_protocol_version,
                                           dummy_snapshot, nullptr, 0);

    /*
      Allocate a buffer that will contain the header, the data, and the packet
      recovery snapshot.
    */
    exchangeable_data_len = encoded_payload.size();
    exchangeable_header_len = encoded_member_state.get_encode_header_size();
    exchangeable_snapshot_len = encoded_member_state.get_encode_snapshot_size();

    buffer_len = exchangeable_header_len + exchangeable_data_len +
                 exchangeable_snapshot_len;
    buffer = static_cast<uchar *>(std::malloc(buffer_len * sizeof(uchar)));
    ASSERT_NE(buffer, nullptr);
    slider = buffer;

    /*
     Serialize the state exchange message.

     Its wire format is:

         +--------+------------------+----------+
         | header | upper-layer data | snapshot |
         +--------+------------------+----------+

     For more context, see Xcom_member_state.
     */
    encoded_member_state.encode_header(slider, &exchangeable_header_len);
    slider += exchangeable_header_len;

    std::memcpy(slider, encoded_payload.c_str(), exchangeable_data_len);
    slider += exchangeable_data_len;

    encoded_member_state.encode_snapshot(slider, &exchangeable_snapshot_len);
    slider += exchangeable_snapshot_len;

    /* Decode the message. */
    Xcom_member_state decoded_member_state(encoder_protocol_version, buffer,
                                           buffer_len);
    std::string decoded_payload(
        reinterpret_cast<const char *>(decoded_member_state.get_data()),
        decoded_member_state.get_data_size());
    ASSERT_EQ(encoded_payload, decoded_payload);

    // Cleanup.
    std::free(buffer);
  }

  Gcs_xcom_state_exchange *state_exchange;
  mock_gcs_control_interface *control_mock;
  mock_gcs_xcom_communication_interface *comm_mock;
};

static Gcs_xcom_synode_set cache_snapshot;

TEST_F(XComStateExchangeTest, StateExchangeBroadcastJoinerTest) {
  // Setting expectations
  EXPECT_CALL(*comm_mock, do_send_message(_, _, _))
      .Times(1)
      .WillOnce(Return(GCS_OK));

  std::string member_1_addr("127.0.0.1:12345");
  std::string member_2_addr("127.0.0.1:12346");

  // Set up parameters
  std::vector<Gcs_member_identifier *> total_members;
  total_members.push_back(new Gcs_member_identifier(member_1_addr));
  total_members.push_back(new Gcs_member_identifier(member_2_addr));

  std::vector<Gcs_member_identifier *> joined_members;
  joined_members.push_back(new Gcs_member_identifier(member_2_addr));

  std::vector<Gcs_member_identifier *> left_members;

  std::vector<std::unique_ptr<Gcs_message_data>> data_to_exchange;

  std::string group_name("group_name");

  Gcs_member_identifier *mi = new Gcs_member_identifier(member_2_addr);
  synode_no configuration_id = null_synode;

  Gcs_xcom_nodes nodes;
  nodes.add_node(Gcs_xcom_node_information(member_1_addr));
  nodes.add_node(Gcs_xcom_node_information(member_2_addr));

  bool leaving = state_exchange->state_exchange(
      configuration_id, total_members, left_members, joined_members,
      data_to_exchange, nullptr, &group_name, *mi, nodes);

  ASSERT_FALSE(leaving);

  delete mi;
}

uchar *copied_payload = nullptr;
uint64_t copied_length = 0;
enum_gcs_error copy_message_content(const Gcs_message &msg) {
  copied_length = msg.get_message_data().get_payload_length();
  copied_payload = static_cast<uchar *>(malloc(sizeof(uchar) * copied_length));
  memcpy(copied_payload, msg.get_message_data().get_payload(), copied_length);

  return GCS_OK;
}

TEST_F(XComStateExchangeTest, StateExchangeProcessStatesPhase) {
  EXPECT_CALL(*comm_mock, do_send_message(_, _, _))
      .WillOnce(WithArgs<0>(Invoke(copy_message_content)));

  /*
    Define that the first view delivered has two members, i.e.
    two members are simultaneously joining the view.
  */
  synode_no configuration_id = null_synode;

  std::string group_name("group_name");

  std::string member_1_addr("127.0.0.1:12345");
  Gcs_member_identifier *member_id_1 = new Gcs_member_identifier(member_1_addr);

  std::string member_2_addr("127.0.0.1:12346");
  Gcs_member_identifier *member_id_2 = new Gcs_member_identifier(member_2_addr);

  std::vector<Gcs_member_identifier *> total_members;
  total_members.push_back(new Gcs_member_identifier(member_1_addr));
  total_members.push_back(new Gcs_member_identifier(member_2_addr));

  std::vector<Gcs_member_identifier *> joined_members;
  joined_members.push_back(new Gcs_member_identifier(member_1_addr));
  joined_members.push_back(new Gcs_member_identifier(member_2_addr));

  std::vector<Gcs_member_identifier *> left_members;

  Gcs_xcom_nodes nodes;
  nodes.add_node(Gcs_xcom_node_information(member_1_addr));
  nodes.add_node(Gcs_xcom_node_information(member_2_addr));

  /*
    No application metadata shall be sent during the state exchange
    process.
  */
  std::vector<std::unique_ptr<Gcs_message_data>> data_to_exchange;

  /*
    Send a state exchange message on behalf of member 1.
  */
  bool leaving = state_exchange->state_exchange(
      configuration_id, total_members, left_members, joined_members,
      data_to_exchange, nullptr, &group_name, *member_id_1, nodes);
  ASSERT_FALSE(leaving);

  /*
    Check whether the state exchange message was properly sent
    and the state exchange state machine has the expected data.
  */
  Xcom_member_state *state_1 = new Xcom_member_state(
      Gcs_protocol_version::HIGHEST_KNOWN, copied_payload, copied_length);

  ASSERT_TRUE(state_1->get_view_id()->get_fixed_part() != 0);
  ASSERT_EQ(state_1->get_view_id()->get_monotonic_part(), 0u);
  ASSERT_EQ(state_1->get_data_size(), 0u);
  ASSERT_TRUE(synode_eq(state_1->get_configuration_id(), configuration_id));

  ASSERT_EQ(state_exchange->get_total()->size(), 2u);
  ASSERT_EQ(state_exchange->get_joined()->size(), 2u);
  ASSERT_EQ(state_exchange->get_left()->size(), 0u);
  ASSERT_EQ(*(state_exchange->get_group()), group_name);
  ASSERT_EQ(state_exchange->get_member_states()->size(), 0u);

  /*
    Simulate message received by member 1.
  */
  bool can_install = state_exchange->process_member_state(
      state_1, *member_id_1, Gcs_protocol_version::V1,
      Gcs_protocol_version::V1);
  ASSERT_FALSE(can_install);
  ASSERT_EQ(state_exchange->get_member_states()->size(), 1u);

  /*
    Simulate message received by member 2.
  */
  const Gcs_xcom_view_identifier view_id_2(99999, 0);
  Gcs_xcom_synode_set snapshot;
  Xcom_member_state *state_2 =
      new Xcom_member_state(view_id_2, configuration_id,
                            Gcs_protocol_version::V1, snapshot, nullptr, 0);
  can_install = state_exchange->process_member_state(state_2, *member_id_2,
                                                     Gcs_protocol_version::V1,
                                                     Gcs_protocol_version::V1);
  ASSERT_TRUE(can_install);
  ASSERT_EQ(state_exchange->get_member_states()->size(), 2u);

  /*
    Simulate how the view is calculated.
  */
  Gcs_xcom_view_identifier *new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(view_id_2.get_fixed_part(), new_view_id->get_fixed_part());
  ASSERT_EQ(view_id_2.get_monotonic_part(), new_view_id->get_monotonic_part());

  delete member_id_1;
  delete member_id_2;
  free(copied_payload);
}

TEST_F(XComStateExchangeTest, StateExchangeChoosingView) {
  /*
    Prepare configuration to simulate state exchanges and
    calculate the new view.
  */
  synode_no configuration_id = null_synode;

  std::string member_1_addr("127.0.0.1:12345");
  Gcs_member_identifier *member_id_1 = new Gcs_member_identifier(member_1_addr);

  std::string member_2_addr("127.0.0.1:12348");
  Gcs_member_identifier *member_id_2 = new Gcs_member_identifier(member_2_addr);

  std::string member_3_addr("127.0.0.1:12346");
  Gcs_member_identifier *member_id_3 = new Gcs_member_identifier(member_3_addr);

  std::string member_4_addr("127.0.0.1:12347");
  Gcs_member_identifier *member_id_4 = new Gcs_member_identifier(member_4_addr);

  /*
    Check the map between member identifiers and states is empty.
  */
  std::map<Gcs_member_identifier, Xcom_member_state *> *member_states =
      state_exchange->get_member_states();
  ASSERT_EQ(member_states->size(), 0u);

  /*
    If there is one view, there is no much choice and the view is picked.
  */
  Gcs_xcom_view_identifier *new_view_id = nullptr;

  Gcs_xcom_view_identifier view_id_1(99999, 0);
  Gcs_xcom_synode_set snapshot;
  Xcom_member_state *state_1 = new Xcom_member_state(
      view_id_1, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_1] = state_1;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 1u);
  ASSERT_EQ(view_id_1.get_fixed_part(), new_view_id->get_fixed_part());
  ASSERT_EQ(view_id_1.get_monotonic_part(), new_view_id->get_monotonic_part());

  /*
    If there is two views where all the monotonic parts are zero, the one
    with the greater member identifier is picked.
  */
  Gcs_xcom_view_identifier view_id_2(88888, 0);
  Xcom_member_state *state_2 = new Xcom_member_state(
      view_id_2, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_2] = state_2;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 2u);
  ASSERT_TRUE(*member_id_1 < *member_id_2);
  ASSERT_EQ(view_id_2.get_fixed_part(), new_view_id->get_fixed_part());
  ASSERT_EQ(view_id_2.get_monotonic_part(), new_view_id->get_monotonic_part());

  /*
    If there are n views where their monotonic parts are zero, the one
    with the greater member identifier is picked.
  */
  Gcs_xcom_view_identifier view_id_3(66666, 0);
  Xcom_member_state *state_3 = new Xcom_member_state(
      view_id_3, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_3] = state_3;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 3u);
  ASSERT_TRUE(*member_id_1 < *member_id_2);
  ASSERT_TRUE(*member_id_3 < *member_id_2);
  ASSERT_EQ(view_id_2.get_fixed_part(), new_view_id->get_fixed_part());
  ASSERT_EQ(view_id_2.get_monotonic_part(), new_view_id->get_monotonic_part());

  /*
    If there are views where their monotonic parts are not zero, the first
    one where the monotonic part is not zero is picked. The system must
    guarantee that all elements that have the monotonic part different from
    zero has the same value.

    This basically means that a previous view has been installed and all
    the members that are part of the previous view must have the same
    view identifier.
  */
  Gcs_xcom_view_identifier view_id_4(77777, 1);
  Xcom_member_state *state_4 = new Xcom_member_state(
      view_id_4, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_4] = state_4;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 4u);
  ASSERT_TRUE(*member_id_1 < *member_id_2);
  ASSERT_TRUE(*member_id_3 < *member_id_2);
  ASSERT_TRUE(*member_id_4 < *member_id_2);
  ASSERT_EQ(view_id_4.get_fixed_part(), new_view_id->get_fixed_part());
  ASSERT_EQ(view_id_4.get_monotonic_part(), new_view_id->get_monotonic_part());

  delete member_id_1;
  delete member_id_2;
  delete member_id_3;
  delete member_id_4;
}

TEST_F(XComStateExchangeTest, StateExchangeWrongAssumptionsView) {
  /*
    This test requires that all debug modes are set but it is not safe to
    set it only here because if it fails, the system may start logging
    messages that are not supposed to do so.
  */
  if (Gcs_debug_manager::get_current_debug_options() != GCS_DEBUG_ALL) {
    /* purecov: begin deadcode */
    return;
    /* purecov: end */
  }

  /*
    Prepare configuration to simulate state exchanges when there
    is a bug in the state exchange messages and members are not
    proposing the correct views.
  */
  Gcs_xcom_view_identifier *new_view_id = nullptr;
  std::map<Gcs_member_identifier, Xcom_member_state *>::iterator state_it;

  std::string member_1_addr("127.0.0.1:12345");
  Gcs_member_identifier *member_id_1 = new Gcs_member_identifier(member_1_addr);

  std::string member_2_addr("127.0.0.1:12348");
  Gcs_member_identifier *member_id_2 = new Gcs_member_identifier(member_2_addr);

  std::string member_3_addr("127.0.0.1:12346");
  Gcs_member_identifier *member_id_3 = new Gcs_member_identifier(member_3_addr);

  std::string member_4_addr("127.0.0.1:12347");
  Gcs_member_identifier *member_id_4 = new Gcs_member_identifier(member_4_addr);

  /*
    Check the map between member identifiers and states is empty.
  */
  std::map<Gcs_member_identifier, Xcom_member_state *> *member_states =
      state_exchange->get_member_states();
  ASSERT_EQ(member_states->size(), 0u);

  /*
    Two views where the monotonic part in each view is different from
    zero but the fixed parts don't match. This situation cannot happen
    in practice.
  */
  synode_no configuration_id = null_synode;
  Gcs_xcom_view_identifier view_id_1(99999, 1);
  Gcs_xcom_synode_set snapshot;
  Xcom_member_state *state_1 = new Xcom_member_state(
      view_id_1, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_1] = state_1;

  Gcs_xcom_view_identifier view_id_2(88888, 1);
  Xcom_member_state *state_2 = new Xcom_member_state(
      view_id_2, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_2] = state_2;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 2u);
  ASSERT_TRUE(new_view_id == nullptr);

  for (state_it = member_states->begin(); state_it != member_states->end();
       state_it++)
    delete (*state_it).second;
  member_states->clear();

  /*
    Two views where the monotonic part in each view is different from
    zero but they don't match. This situation cannot happen in practice.
  */
  Gcs_xcom_view_identifier view_id_3(99999, 1);
  Xcom_member_state *state_3 = new Xcom_member_state(
      view_id_3, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_3] = state_3;

  Gcs_xcom_view_identifier view_id_4(99999, 2);
  Xcom_member_state *state_4 = new Xcom_member_state(
      view_id_4, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  (*member_states)[*member_id_4] = state_4;
  new_view_id = state_exchange->get_new_view_id();
  ASSERT_EQ(member_states->size(), 2u);
  ASSERT_TRUE(new_view_id == nullptr);
  (void)new_view_id;

  for (state_it = member_states->begin(); state_it != member_states->end();
       state_it++)
    delete (*state_it).second;
  member_states->clear();

  delete member_id_1;
  delete member_id_2;
  delete member_id_3;
  delete member_id_4;
}

TEST_F(XComStateExchangeTest, StateExchangeDiscardSynodes) {
  EXPECT_CALL(*comm_mock, do_send_message(_, _, _)).WillOnce(Return(GCS_OK));

  /*
    Define that the first view delivered has two members, i.e.
    two members are simultaneously joining the view.
  */
  synode_no configuration_id = null_synode;

  synode_no invalid_configuration_id = null_synode;
  invalid_configuration_id.group_id = 0;
  invalid_configuration_id.msgno = 1;
  invalid_configuration_id.node = 0;

  std::string group_name("group_name");

  std::string member_1_addr("127.0.0.1:12345");
  Gcs_member_identifier *member_id_1 = new Gcs_member_identifier(member_1_addr);

  std::vector<Gcs_member_identifier *> total_members;
  total_members.push_back(new Gcs_member_identifier(member_1_addr));

  std::vector<Gcs_member_identifier *> joined_members;
  joined_members.push_back(new Gcs_member_identifier(member_1_addr));

  std::vector<Gcs_member_identifier *> left_members;

  /*
    No application metadata shall be sent during the state exchange
    process.
  */
  std::vector<std::unique_ptr<Gcs_message_data>> data_to_exchange;

  Gcs_xcom_nodes nodes;
  nodes.add_node(Gcs_xcom_node_information(member_1_addr));

  /*
    Send a state exchange message on behalf of member 1.
  */
  state_exchange->state_exchange(configuration_id, total_members, left_members,
                                 joined_members, data_to_exchange, nullptr,
                                 &group_name, *member_id_1, nodes);

  /*
    If the synode does not match, the state exchange message is
    ignored.
  */
  const Gcs_xcom_view_identifier view_id_1(99999, 0);
  Gcs_xcom_synode_set snapshot;
  Xcom_member_state *state_1 = new Xcom_member_state(
      view_id_1, invalid_configuration_id, Gcs_protocol_version::HIGHEST_KNOWN,
      snapshot, nullptr, 0);
  bool can_install = state_exchange->process_member_state(
      state_1, *member_id_1, Gcs_protocol_version::V1,
      Gcs_protocol_version::V1);
  ASSERT_FALSE(can_install);
  ASSERT_EQ(state_exchange->get_member_states()->size(), 0u);

  delete member_id_1;
}

TEST_F(XComStateExchangeTest, DecodeStateExchangeMessage) {
  this->DecodeStateExchangeMessage(Gcs_protocol_version::HIGHEST_KNOWN);
}

TEST_F(XComStateExchangeTest, DecodeStateExchangeMessageV1) {
  this->DecodeStateExchangeMessage(Gcs_protocol_version::V1);
}

}  // namespace gcs_xcom_state_exchange_unittest
