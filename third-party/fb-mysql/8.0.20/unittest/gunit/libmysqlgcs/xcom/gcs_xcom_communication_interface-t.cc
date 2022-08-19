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

#include "gcs_base_test.h"

#include <array>
#include <vector>

#include "gcs_message_stage_lz4.h"
#include "gcs_message_stage_split.h"
#include "gcs_xcom_communication_interface.h"
#include "gcs_xcom_statistics_interface.h"
#include "mysql/gcs/gcs_message.h"
#include "template_utils.h"

namespace gcs_xcom_communication_unittest {

class mock_gcs_xcom_view_change_control_interface
    : public Gcs_xcom_view_change_control_interface {
 public:
  MOCK_METHOD0(start_view_exchange, void());
  MOCK_METHOD0(end_view_exchange, void());
  MOCK_METHOD0(wait_for_view_change_end, void());
  MOCK_METHOD0(is_view_changing, bool());
  MOCK_METHOD0(start_leave, bool());
  MOCK_METHOD0(end_leave, void());
  MOCK_METHOD0(is_leaving, bool());
  MOCK_METHOD0(start_join, bool());
  MOCK_METHOD0(end_join, void());
  MOCK_METHOD0(is_joining, bool());

  MOCK_METHOD1(set_current_view, void(Gcs_view *));
  MOCK_METHOD0(get_current_view, Gcs_view *());
  MOCK_METHOD0(belongs_to_group, bool());
  MOCK_METHOD1(set_belongs_to_group, void(bool));
  MOCK_METHOD1(set_unsafe_current_view, void(Gcs_view *));
  MOCK_METHOD0(get_unsafe_current_view, Gcs_view *());
};

class mock_gcs_xcom_statistics_updater : public Gcs_xcom_statistics_updater {
 public:
  MOCK_METHOD1(update_message_sent, void(unsigned long long message_length));
  MOCK_METHOD1(update_message_received, void(long message_length));
};

class mock_gcs_communication_event_listener
    : public Gcs_communication_event_listener {
 public:
  MOCK_CONST_METHOD1(on_message_received, void(const Gcs_message &message));
};

class mock_gcs_xcom_proxy : public Gcs_xcom_proxy_base {
 public:
  mock_gcs_xcom_proxy() {
    ON_CALL(*this, xcom_open_handlers(_, _)).WillByDefault(Return(false));
    ON_CALL(*this, xcom_close_handlers()).WillByDefault(Return(false));
    ON_CALL(*this, xcom_client_add_node(_, _, _)).WillByDefault(Return(false));
    ON_CALL(*this, xcom_client_send_data(_, _)).WillByDefault(Return(false));
  }

  MOCK_METHOD3(new_node_address_uuid,
               node_address *(unsigned int n, char *names[], blob uuids[]));
  MOCK_METHOD2(delete_node_address, void(unsigned int n, node_address *na));
  MOCK_METHOD3(xcom_client_add_node, bool(connection_descriptor *con,
                                          node_list *nl, uint32_t group_id));
  MOCK_METHOD2(xcom_client_remove_node, bool(node_list *nl, uint32_t group_id));
  MOCK_METHOD3(xcom_client_remove_node, bool(connection_descriptor *con,
                                             node_list *nl, uint32_t group_id));
  MOCK_METHOD2(xcom_client_get_event_horizon,
               bool(uint32_t group_id, xcom_event_horizon &event_horizon));
  MOCK_METHOD2(xcom_client_set_event_horizon,
               bool(uint32_t group_id, xcom_event_horizon event_horizon));
  MOCK_METHOD4(xcom_client_get_synode_app_data,
               bool(connection_descriptor *con, uint32_t group_id_hash,
                    synode_no_array &synodes, synode_app_data_array &reply));
  MOCK_METHOD1(xcom_client_set_cache_size, bool(uint64_t));
  MOCK_METHOD2(xcom_client_boot, bool(node_list *nl, uint32_t group_id));
  MOCK_METHOD2(xcom_client_open_connection,
               connection_descriptor *(std::string, xcom_port port));
  MOCK_METHOD1(xcom_client_close_connection, bool(connection_descriptor *con));
  MOCK_METHOD2(xcom_client_send_data,
               bool(unsigned long long size, char *data));
  MOCK_METHOD1(xcom_init, void(xcom_port listen_port));
  MOCK_METHOD0(xcom_exit, void());
  MOCK_METHOD0(xcom_set_cleanup, void());
  MOCK_METHOD1(xcom_get_ssl_mode, int(const char *mode));
  MOCK_METHOD1(xcom_set_ssl_mode, int(int mode));
  MOCK_METHOD1(xcom_get_ssl_fips_mode, int(const char *mode));
  MOCK_METHOD1(xcom_set_ssl_fips_mode, int(int mode));
  MOCK_METHOD0(xcom_init_ssl, bool());
  MOCK_METHOD0(xcom_destroy_ssl, void());
  MOCK_METHOD0(xcom_use_ssl, bool());
  MOCK_METHOD2(xcom_set_ssl_parameters,
               void(ssl_parameters ssl, tls_parameters tls));
  MOCK_METHOD1(find_site_def, site_def const *(synode_no synode));
  MOCK_METHOD2(xcom_open_handlers, bool(std::string saddr, xcom_port port));
  MOCK_METHOD0(xcom_close_handlers, bool());
  MOCK_METHOD0(xcom_acquire_handler, int());
  MOCK_METHOD1(xcom_release_handler, void(int index));
  MOCK_METHOD0(xcom_wait_ready, enum_gcs_error());
  MOCK_METHOD0(xcom_is_ready, bool());
  MOCK_METHOD1(xcom_set_ready, void(bool value));
  MOCK_METHOD0(xcom_signal_ready, void());
  MOCK_METHOD1(xcom_wait_for_xcom_comms_status_change, void(int &status));
  MOCK_METHOD0(xcom_has_comms_status_changed, bool());
  MOCK_METHOD1(xcom_set_comms_status, void(int status));
  MOCK_METHOD1(xcom_signal_comms_status_changed, void(int status));
  MOCK_METHOD0(xcom_wait_exit, enum_gcs_error());
  MOCK_METHOD0(xcom_is_exit, bool());
  MOCK_METHOD1(xcom_set_exit, void(bool));
  MOCK_METHOD0(xcom_signal_exit, void());
  MOCK_METHOD3(xcom_client_force_config, int(connection_descriptor *fd,
                                             node_list *nl, uint32_t group_id));
  MOCK_METHOD2(xcom_client_force_config,
               bool(node_list *nl, uint32_t group_id));

  MOCK_METHOD0(get_should_exit, bool());
  MOCK_METHOD1(set_should_exit, void(bool should_exit));

  MOCK_METHOD2(xcom_input_connect,
               bool(std::string const &address, xcom_port port));
  MOCK_METHOD0(xcom_input_disconnect, void());
  MOCK_METHOD1(xcom_input_try_push, bool(app_data_ptr data));
  /* Mocking fails compilation on Windows. It attempts to copy the std::future
   * which is non-copyable. */
  Gcs_xcom_input_queue::future_reply xcom_input_try_push_and_get_reply(
      app_data_ptr) {
    return std::future<std::unique_ptr<Gcs_xcom_input_queue::Reply>>();
  }
  MOCK_METHOD0(xcom_input_try_pop, xcom_input_request_ptr());
};

class XComCommunicationTest : public GcsBaseTest {
 protected:
  virtual void SetUp() {
    engine = new Gcs_xcom_engine();
    engine->initialize(nullptr);

    mock_gid = new Gcs_group_identifier("mock_group");
    static_cast<Gcs_xcom_interface *>(Gcs_xcom_interface::get_interface())
        ->set_xcom_group_information(mock_gid->get_group_id());
    mock_xcom_address = new Gcs_xcom_node_address("127.0.0.1:12345");
    static_cast<Gcs_xcom_interface *>(Gcs_xcom_interface::get_interface())
        ->set_node_address(mock_xcom_address->get_member_address());
    mock_stats = new mock_gcs_xcom_statistics_updater();
    mock_proxy = new mock_gcs_xcom_proxy();
    mock_vce = new mock_gcs_xcom_view_change_control_interface();
    xcom_comm_if = new Gcs_xcom_communication(mock_stats, mock_proxy, mock_vce,
                                              engine, *mock_gid);

    // clang-format off
    xcom_comm_if->get_msg_pipeline().register_stage<Gcs_message_stage_lz4>();
    xcom_comm_if->get_msg_pipeline().register_stage<Gcs_message_stage_lz4_v2>();
    xcom_comm_if->get_msg_pipeline().register_pipeline({
      {
        Gcs_protocol_version::V1, { Stage_code::ST_LZ4_V1 }
      },
      {
        Gcs_protocol_version::V2, { Stage_code::ST_LZ4_V2 }
      }
    });
    // clang-format on
  }

  virtual void TearDown() {
    delete mock_stats;
    delete mock_vce;
    delete mock_proxy;
    delete xcom_comm_if;
    delete mock_xcom_address;
    delete mock_gid;
    engine->finalize(nullptr);
    delete engine;
  }

  Gcs_xcom_communication *xcom_comm_if;
  mock_gcs_xcom_statistics_updater *mock_stats;
  mock_gcs_xcom_proxy *mock_proxy;
  mock_gcs_xcom_view_change_control_interface *mock_vce;
  Gcs_xcom_node_address *mock_xcom_address;
  Gcs_xcom_engine *engine;
  Gcs_group_identifier *mock_gid;
};

TEST_F(XComCommunicationTest, SetEventListenerTest) {
  mock_gcs_communication_event_listener comm_listener;

  int reference = xcom_comm_if->add_event_listener(comm_listener);

  ASSERT_NE(0, reference);
  ASSERT_EQ((long unsigned int)1,
            xcom_comm_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1, xcom_comm_if->get_event_listeners()->size());
}

TEST_F(XComCommunicationTest, SetEventListenersTest) {
  mock_gcs_communication_event_listener comm_listener;
  mock_gcs_communication_event_listener another_comm_listener;

  int reference = xcom_comm_if->add_event_listener(comm_listener);
  int another_reference =
      xcom_comm_if->add_event_listener(another_comm_listener);

  ASSERT_NE(0, reference);
  ASSERT_NE(0, another_reference);
  ASSERT_EQ((long unsigned int)1,
            xcom_comm_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_comm_if->get_event_listeners()->count(another_reference));
  ASSERT_EQ((long unsigned int)2, xcom_comm_if->get_event_listeners()->size());
  ASSERT_NE(reference, another_reference);
}

TEST_F(XComCommunicationTest, RemoveEventListenerTest) {
  mock_gcs_communication_event_listener comm_listener;
  mock_gcs_communication_event_listener another_comm_listener;

  int reference = xcom_comm_if->add_event_listener(comm_listener);
  int another_reference =
      xcom_comm_if->add_event_listener(another_comm_listener);

  xcom_comm_if->remove_event_listener(reference);

  ASSERT_NE(0, reference);
  ASSERT_NE(0, another_reference);
  ASSERT_EQ((long unsigned int)0,
            xcom_comm_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_comm_if->get_event_listeners()->count(another_reference));
  ASSERT_EQ((long unsigned int)1, xcom_comm_if->get_event_listeners()->size());
  ASSERT_NE(reference, another_reference);
}

bool mock_xcom_client_send_data(unsigned long long, char *data) {
  free(data);
  return true;
}

TEST_F(XComCommunicationTest, SendMessageTest) {
  // Test Expectations
  EXPECT_CALL(*mock_proxy, xcom_client_send_data(_, _))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_send_data));
  EXPECT_CALL(*mock_stats, update_message_sent(_)).Times(1);
  EXPECT_CALL(*mock_vce, belongs_to_group()).Times(1).WillOnce(Return(true));

  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_member_identifier member_id("member");
  Gcs_group_identifier group_id("group");
  Gcs_message_data *message_data =
      new Gcs_message_data(test_header.length(), test_payload.length());

  Gcs_message message(member_id, group_id, message_data);

  message.get_message_data().append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());

  message.get_message_data().append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  enum_gcs_error message_result = xcom_comm_if->send_message(message);
  ASSERT_EQ(GCS_OK, message_result);
}

TEST_F(XComCommunicationTest, ReceiveMessageTest) {
  mock_gcs_communication_event_listener ev_listener;

  // Test Expectations
  EXPECT_CALL(ev_listener, on_message_received(_)).Times(1);
  EXPECT_CALL(*mock_stats, update_message_received(_)).Times(1);

  // Test
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_member_identifier member_id("member");
  Gcs_group_identifier group_id("group");
  Gcs_message_data message_data(test_header.length(), test_payload.length());

  message_data.append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());

  message_data.append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  bool error;
  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      xcom_comm_if->get_msg_pipeline().process_outgoing(
          message_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_len;
  std::tie(buffer, buffer_len) = packets_out.at(0).serialize();

  // Mock the sending of a dummy message instead of our test payload.
  EXPECT_CALL(*mock_vce, belongs_to_group()).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*mock_vce, is_view_changing()).Times(1).WillOnce(Return(false));
  Gcs_view mock_view(
      {Gcs_member_identifier(mock_xcom_address->get_member_address())},
      Gcs_xcom_view_identifier(0, 0), {}, {}, group_id);
  EXPECT_CALL(*mock_vce, get_unsafe_current_view())
      .Times(1)
      .WillOnce(Return(&mock_view));
  EXPECT_CALL(*mock_proxy, xcom_client_send_data(_, _))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_send_data));
  EXPECT_CALL(*mock_stats, update_message_sent(_)).Times(1);
  enum_gcs_error message_result = xcom_comm_if->send_message(
      Gcs_message(member_id, group_id, new Gcs_message_data(0, 0)));
  ASSERT_EQ(GCS_OK, message_result);

  // Act as if we received our test payload
  synode_no packet_synode;
  packet_synode.group_id = Gcs_xcom_utils::build_xcom_group_id(*mock_gid);
  packet_synode.msgno = 0;
  packet_synode.node = 0;
  auto packet = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_len, packet_synode,
      xcom_comm_if->get_msg_pipeline());

  int listener_ref = xcom_comm_if->add_event_listener(ev_listener);

  std::unique_ptr<Gcs_xcom_nodes> xcom_nodes(new Gcs_xcom_nodes());
  xcom_nodes->add_node(Gcs_xcom_node_information(
      "127.0.0.1:12345", Gcs_xcom_uuid(), packet_synode.node, true));

  xcom_comm_if->process_user_data_packet(std::move(packet),
                                         std::move(xcom_nodes));

  xcom_comm_if->remove_event_listener(listener_ref);
}

TEST_F(XComCommunicationTest, BufferMessageTest) {
  mock_gcs_communication_event_listener ev_listener;

  // Test Expectations
  EXPECT_CALL(ev_listener, on_message_received(_)).Times(1);

  EXPECT_CALL(*mock_stats, update_message_received(_)).Times(1);

  // Set up the environment.
  std::string test_header("header");
  std::string test_payload("payload");
  Gcs_member_identifier member_id("member");
  Gcs_group_identifier group_id("group");
  int listener_ref = xcom_comm_if->add_event_listener(ev_listener);
  Gcs_message_data message_data(test_header.length(), test_payload.length());

  message_data.append_to_header(
      pointer_cast<const uchar *>(test_header.c_str()), test_header.length());

  message_data.append_to_payload(
      pointer_cast<const uchar *>(test_payload.c_str()), test_payload.length());

  bool error;
  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      xcom_comm_if->get_msg_pipeline().process_outgoing(
          message_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 1);

  Gcs_packet::buffer_ptr buffer;
  unsigned long long buffer_len;
  std::tie(buffer, buffer_len) = packets_out.at(0).serialize();

  // Mock the sending of a dummy message instead of our test payload.
  EXPECT_CALL(*mock_vce, belongs_to_group()).Times(1).WillOnce(Return(true));
  EXPECT_CALL(*mock_vce, is_view_changing()).Times(1).WillOnce(Return(false));
  Gcs_view mock_view(
      {Gcs_member_identifier(mock_xcom_address->get_member_address())},
      Gcs_xcom_view_identifier(0, 0), {}, {}, group_id);
  EXPECT_CALL(*mock_vce, get_unsafe_current_view())
      .Times(1)
      .WillOnce(Return(&mock_view));
  EXPECT_CALL(*mock_proxy, xcom_client_send_data(_, _))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_send_data));
  EXPECT_CALL(*mock_stats, update_message_sent(_)).Times(1);
  enum_gcs_error message_result = xcom_comm_if->send_message(
      Gcs_message(member_id, group_id, new Gcs_message_data(0, 0)));
  ASSERT_EQ(GCS_OK, message_result);

  // Act as if we received our test payload
  synode_no packet_synode;
  packet_synode.group_id = Gcs_xcom_utils::build_xcom_group_id(*mock_gid);
  packet_synode.msgno = 0;
  packet_synode.node = 0;
  auto packet = Gcs_packet::make_incoming_packet(
      std::move(buffer), buffer_len, packet_synode,
      xcom_comm_if->get_msg_pipeline());

  /*
     Try to send a message when the view is not installed. They are
     buffered but we flush them out.
  */
  ON_CALL(*mock_vce, belongs_to_group()).WillByDefault(Return(false));
  ON_CALL(*mock_vce, is_view_changing()).WillByDefault(Return(true));

  std::unique_ptr<Gcs_xcom_nodes> xcom_nodes(new Gcs_xcom_nodes());
  xcom_nodes->add_node(Gcs_xcom_node_information(
      "127.0.0.1:12345", Gcs_xcom_uuid(), packet_synode.node, true));

  xcom_comm_if->process_user_data_packet(std::move(packet),
                                         std::move(xcom_nodes));

  ON_CALL(*mock_vce, belongs_to_group()).WillByDefault(Return(true));
  ON_CALL(*mock_vce, is_view_changing()).WillByDefault(Return(false));

  xcom_comm_if->deliver_buffered_packets();

  xcom_comm_if->remove_event_listener(listener_ref);
}

/*
 This function mocks a successful recovery for SuccessfulSynodRecoveryTest.
 It receives a request for synodes 1 and 2, and sends back their application
 payloads.
 */
static Gcs_packet::buffer_ptr synode_1_data{nullptr,
                                            Gcs_packet_buffer_deleter()};
static u_int synode_1_data_len = 0;
static Gcs_packet::buffer_ptr synode_2_data{nullptr,
                                            Gcs_packet_buffer_deleter()};
static u_int synode_2_data_len = 0;
bool mock_xcom_client_get_synode_app_data(connection_descriptor *, uint32_t,
                                          synode_no_array &synodes,
                                          synode_app_data_array &reply) {
  u_int const nr_synodes = synodes.synode_no_array_len;
  assert(nr_synodes == 2);

  reply.synode_app_data_array_val = static_cast<synode_app_data *>(
      malloc(nr_synodes * sizeof(synode_app_data)));
  assert(reply.synode_app_data_array_val != nullptr);

  reply.synode_app_data_array_len = nr_synodes;

  assert(synode_1_data != nullptr);
  assert(synode_1_data_len != 0);
  reply.synode_app_data_array_val[0].synode = synodes.synode_no_array_val[0];
  reply.synode_app_data_array_val[0].data.data_len = synode_1_data_len;
  reply.synode_app_data_array_val[0].data.data_val =
      reinterpret_cast<char *>(synode_1_data.release());

  assert(synode_2_data != nullptr);
  assert(synode_2_data_len != 0);
  reply.synode_app_data_array_val[1].synode = synodes.synode_no_array_val[1];
  reply.synode_app_data_array_val[1].data.data_len = synode_2_data_len;
  reply.synode_app_data_array_val[1].data.data_val =
      reinterpret_cast<char *>(synode_2_data.release());

  std::free(synodes.synode_no_array_val);

  return true;
}

int mock_xcom_client_close_connection(connection_descriptor *con) {
  ::free(con);
  return 1;
}

TEST_F(XComCommunicationTest, SuccessfulSynodRecoveryTest) {
  bool error = true;
  synode_no base_synod;
  base_synod.group_id = 1;
  base_synod.msgno = 0;
  base_synod.node = 0;

  /* Payload will be split in 4 packets of 4 bytes. */
  std::string const payload("Yay");
  std::size_t const payload_length = payload.size() + 1;  // +1 for '\0'
  Gcs_message_data message_data(0, payload_length);
  message_data.append_to_payload(
      reinterpret_cast<unsigned char const *>(payload.c_str()), payload_length);
  unsigned long long constexpr split_threshold = 4;
  std::size_t constexpr nr_fragments = 4;
  Gcs_message_pipeline &pipeline = xcom_comm_if->get_msg_pipeline();

  /* Reset pipeline. */
  pipeline.cleanup();
  // clang-format off
    pipeline.register_stage<Gcs_message_stage_split_v2>(true, split_threshold);
    error = pipeline.register_pipeline({
      {
        Gcs_protocol_version::V2, { Stage_code::ST_SPLIT_V2 }
      }
    });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  /* Setup group members, me and other dummy guy for recovery. */
  Gcs_member_identifier me(mock_xcom_address->get_member_address());
  Gcs_xcom_node_information other("127.0.0.2:12345");
  Gcs_xcom_nodes xcom_nodes;
  xcom_nodes.add_node(Gcs_xcom_node_information(me.get_member_id()));
  xcom_nodes.add_node(other);
  xcom_comm_if->update_members_information(me, xcom_nodes);

  /* Fragment packet. */
  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(message_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), nr_fragments);

  /*
   Now we are in imagination land, and the test begins.
   I am a joining node that will require the first two packets via recovery.
   The split packets will each have synod 1, 2, 3, and 4.
   */
  synode_no synode_1 = base_synod;
  synode_1.msgno = 1;
  synode_no synode_2 = base_synod;
  synode_2.msgno = 2;
  synode_no synode_3 = base_synod;
  synode_3.msgno = 3;
  synode_no synode_4 = base_synod;
  synode_4.msgno = 4;
  std::unordered_set<Gcs_xcom_synode> synodes{Gcs_xcom_synode(synode_1),
                                              Gcs_xcom_synode(synode_2)};

  /* Mock the connection to the donor. */
  EXPECT_CALL(*mock_proxy, xcom_client_open_connection(_, _))
      .Times(1)
      .WillOnce(Return(::new_connection(0, nullptr)));

  /*
   Mock the request-reply interaction with the remote XCom.
   We use the global variables synode_*_data{,_len} to pass the synod payloads
   to the mocked function.
   */
  std::tie(synode_1_data, synode_1_data_len) = packets_out[0].serialize();
  std::tie(synode_2_data, synode_2_data_len) = packets_out[1].serialize();
  EXPECT_CALL(*mock_proxy, xcom_client_get_synode_app_data(_, _, _, _))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_get_synode_app_data));

  /* Mock the disconnection from the donor. */
  EXPECT_CALL(*mock_proxy, xcom_client_close_connection(_))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_close_connection));

  /* Recover! */
  bool const recovered = xcom_comm_if->recover_packets(synodes);
  ASSERT_TRUE(recovered);

  /* Receive the last two packets normally. */
  std::vector<Gcs_packet> packets_in;
  std::array<synode_no, 4> synodes_in_order{synode_1, synode_2, synode_3,
                                            synode_4};
  Gcs_packet packet_in;
  for (std::size_t i = 2; i < 4; i++) {
    /* Construct the packet from the "incoming" buffer. */
    Gcs_packet &packet_out = packets_out[i];
    Gcs_packet::buffer_ptr buffer;
    unsigned long long buffer_len;
    std::tie(buffer, buffer_len) = packet_out.serialize();
    synode_no const &packet_synode = synodes_in_order.at(i);
    auto packet = Gcs_packet::make_incoming_packet(
        std::move(buffer), buffer_len, packet_synode, pipeline);

    /* Reassemble the packets. */
    Gcs_pipeline_incoming_result error_code;
    std::tie(error_code, packet_in) =
        pipeline.process_incoming(std::move(packet));
    ASSERT_TRUE(
        (i == 2 && error_code == Gcs_pipeline_incoming_result::OK_NO_PACKET) ||
        (i == 3 && error_code == Gcs_pipeline_incoming_result::OK_PACKET));
  }

  /* Compare against original payload. */
  Gcs_message_data received_data(packet_in.get_payload_length());
  ASSERT_FALSE(received_data.decode(packet_in.get_payload_pointer(),
                                    packet_in.get_payload_length()));
  std::string received_payload(
      reinterpret_cast<char const *>(received_data.get_payload()));
  ASSERT_EQ(payload, received_payload);
}

bool mock_unsuccessful_xcom_client_get_synode_app_data(
    connection_descriptor *, uint32_t, synode_no_array &synodes,
    synode_app_data_array &) {
  std::free(synodes.synode_no_array_val);
  return false;
}

TEST_F(XComCommunicationTest, UnsuccessfulSynodRecoveryTest) {
  bool error = true;
  synode_no base_synod;
  base_synod.group_id = 1;
  base_synod.msgno = 0;
  base_synod.node = 0;

  /* Payload will be split in 4 packets of 4 bytes. */
  std::string const payload("Yay");
  std::size_t const payload_length = payload.size() + 1;  // +1 for '\0'
  Gcs_message_data message_data(0, payload_length);
  message_data.append_to_payload(
      reinterpret_cast<unsigned char const *>(payload.c_str()), payload_length);
  unsigned long long constexpr split_threshold = 4;
  std::size_t constexpr nr_fragments = 4;
  Gcs_message_pipeline &pipeline = xcom_comm_if->get_msg_pipeline();

  /* Reset pipeline. */
  pipeline.cleanup();
  // clang-format off
    pipeline.register_stage<Gcs_message_stage_split_v2>(true, split_threshold);
    error = pipeline.register_pipeline({
      {
        Gcs_protocol_version::V2, { Stage_code::ST_SPLIT_V2 }
      }
    });
  // clang-format on
  pipeline.set_version(Gcs_protocol_version::V2);
  ASSERT_FALSE(error);

  /* Setup group members, me and other dummy guy for recovery. */
  Gcs_member_identifier me(mock_xcom_address->get_member_address());
  Gcs_xcom_node_information other("127.0.0.2:12345");
  Gcs_xcom_nodes xcom_nodes;
  xcom_nodes.add_node(Gcs_xcom_node_information(me.get_member_id()));
  xcom_nodes.add_node(other);
  xcom_comm_if->update_members_information(me, xcom_nodes);

  /* Fragment packet. */
  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      pipeline.process_outgoing(message_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), nr_fragments);

  /*
   Now we are in imagination land, and the test begins.
   I am a joining node that will require the first two packets via recovery,
   but the recovery will fail.
   The split packets will each have synod 1, 2, 3, and 4.
   */
  synode_no synode_1 = base_synod;
  synode_1.msgno = 1;
  synode_no synode_2 = base_synod;
  synode_2.msgno = 2;
  std::unordered_set<Gcs_xcom_synode> synodes{Gcs_xcom_synode(synode_1),
                                              Gcs_xcom_synode(synode_2)};

  /* Mock the connection to the donor. */
  EXPECT_CALL(*mock_proxy, xcom_client_open_connection(_, _))
      .Times(1)
      .WillOnce(Return(::new_connection(0, nullptr)));

  /*
   Mock the request-reply interaction with the remote XCom.
   It will be unsuccessful, i.e. XCom no longer had the required synodes.
   */
  EXPECT_CALL(*mock_proxy, xcom_client_get_synode_app_data(_, _, _, _))
      .Times(1)
      .WillOnce(Invoke(&mock_unsuccessful_xcom_client_get_synode_app_data));

  /* Mock the disconnection from the donor. */
  EXPECT_CALL(*mock_proxy, xcom_client_close_connection(_))
      .Times(1)
      .WillOnce(Invoke(&mock_xcom_client_close_connection));

  /* Fail to recover! */
  bool const recovered = xcom_comm_if->recover_packets(synodes);
  ASSERT_FALSE(recovered);
}

}  // namespace gcs_xcom_communication_unittest
