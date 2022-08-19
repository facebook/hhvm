/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <thread>
#include "gcs_base_test.h"

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_message_stage_split.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_message_stages.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_communication_interface.h"

namespace gcs_message_stage_fragmentation_unittest {

class Mock_gcs_xcom_statistics_updater : public Gcs_xcom_statistics_updater {
 public:
  MOCK_METHOD1(update_message_sent, void(unsigned long long message_length));
  MOCK_METHOD1(update_message_received, void(long message_length));
};

bool mock_xcom_client_send_data(unsigned long long, char *data) {
  std::free(data);
  return true;
}

class Mock_gcs_xcom_proxy : public Gcs_xcom_proxy_base {
 public:
  Mock_gcs_xcom_proxy() {
    ON_CALL(*this, xcom_client_send_data(_, _))
        .WillByDefault(Invoke(&mock_xcom_client_send_data));
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

class Mock_gcs_xcom_view_change_control_interface
    : public Gcs_xcom_view_change_control_interface {
 public:
  Mock_gcs_xcom_view_change_control_interface() {
    ON_CALL(*this, belongs_to_group()).WillByDefault(Return(true));
  }
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

class Mock_gcs_communication_event_listener
    : public Gcs_communication_event_listener {
 public:
  MOCK_CONST_METHOD1(on_message_received, void(const Gcs_message &message));
};

class GcsMessageStageFragmentationTest : public GcsBaseTest {
 protected:
  Gcs_xcom_engine m_engine;
  Gcs_group_identifier m_mock_gid{"mock_group"};
  Gcs_xcom_node_address m_mock_xcom_address{"127.0.0.1:12345"};
  Mock_gcs_xcom_statistics_updater m_mock_stats;
  Mock_gcs_xcom_proxy m_mock_proxy;
  Mock_gcs_xcom_view_change_control_interface m_mock_vce;
  Gcs_xcom_communication m_xcom_comm_if{&m_mock_stats, &m_mock_proxy,
                                        &m_mock_vce, &m_engine, m_mock_gid};
  Gcs_message_stage_split_v2 *m_fragmentation_stage{nullptr};

 public:
  GcsMessageStageFragmentationTest() {
    static_cast<Gcs_xcom_interface *>(Gcs_xcom_interface::get_interface())
        ->set_xcom_group_information(m_mock_gid.get_group_id());
    static_cast<Gcs_xcom_interface *>(Gcs_xcom_interface::get_interface())
        ->set_node_address(m_mock_xcom_address.get_member_address());
  }

  void configure_pipeline(bool const fragmentation_enabled,
                          unsigned long long const fragmentation_threshold) {
    Gcs_message_pipeline &pipeline = m_xcom_comm_if.get_msg_pipeline();
    pipeline.register_stage<Gcs_message_stage_split_v2>(
        fragmentation_enabled, fragmentation_threshold);
    // clang-format off
    pipeline.register_pipeline({
      {
        Gcs_protocol_version::HIGHEST_KNOWN, { Stage_code::ST_SPLIT_V2 }
      }
    });
    // clang-format on
    m_fragmentation_stage = static_cast<Gcs_message_stage_split_v2 *>(
        &pipeline.get_stage(Stage_code::ST_SPLIT_V2));
  }
};

/* Verify that the reassembly of fragments whose delivery crosses views works.
 */
TEST_F(GcsMessageStageFragmentationTest, ReassemblyOfFragmentsThatCrossViews) {
  Mock_gcs_communication_event_listener ev_listener;
  int listener_ref = m_xcom_comm_if.add_event_listener(ev_listener);
  EXPECT_CALL(ev_listener, on_message_received(_)).Times(1);

  std::string payload("payload!");

  bool constexpr FRAGMENT = true;
  unsigned long long constexpr FRAGMENT_THRESHOLD = 10;
  configure_pipeline(FRAGMENT, FRAGMENT_THRESHOLD);

  /* Current view:
       0 -> some other guy
       1 -> me */
  std::unique_ptr<Gcs_xcom_nodes> xcom_nodes_first_view(new Gcs_xcom_nodes());
  xcom_nodes_first_view->add_node(
      Gcs_xcom_node_information("127.0.0.1:54321", Gcs_xcom_uuid(), 0, true));
  xcom_nodes_first_view->add_node(Gcs_xcom_node_information(
      m_mock_xcom_address.get_member_address(), Gcs_xcom_uuid(), 1, true));
  m_fragmentation_stage->update_members_information(
      Gcs_member_identifier(m_mock_xcom_address.get_member_address()),
      *xcom_nodes_first_view);

  Gcs_message_data *message_data = new Gcs_message_data(0, payload.size());
  message_data->append_to_payload(
      reinterpret_cast<uchar const *>(payload.c_str()), payload.size());
  ASSERT_GT(message_data->get_encode_size(), FRAGMENT_THRESHOLD);

  /* Get the serialized fragments. */
  bool error = true;
  std::vector<Gcs_packet> packets_out;
  std::tie(error, packets_out) =
      m_xcom_comm_if.get_msg_pipeline().process_outgoing(
          *message_data, Cargo_type::CT_USER_DATA);
  ASSERT_FALSE(error);
  ASSERT_EQ(packets_out.size(), 2);

  /* Mock sending the packets to affect the protocol changer. */
  Gcs_message message(
      Gcs_member_identifier(m_mock_xcom_address.get_member_address()),
      m_mock_gid, message_data);
  enum_gcs_error message_result = m_xcom_comm_if.send_message(message);
  ASSERT_EQ(GCS_OK, message_result);

  /* Receive first fragment in one view. */
  // I am currently node 1.
  synode_no synod_first_fragment;
  synod_first_fragment.group_id =
      Gcs_xcom_utils::build_xcom_group_id(m_mock_gid);
  synod_first_fragment.msgno = 0;
  synod_first_fragment.node = 1;
  Gcs_packet::buffer_ptr buffer_pointer = nullptr;
  unsigned long long buffer_size = 0;
  std::tie(buffer_pointer, buffer_size) = packets_out.at(0).serialize();
  auto first_fragment = Gcs_packet::make_incoming_packet(
      std::move(buffer_pointer), buffer_size, synod_first_fragment,
      m_xcom_comm_if.get_msg_pipeline());
  m_xcom_comm_if.process_user_data_packet(std::move(first_fragment),
                                          std::move(xcom_nodes_first_view));

  /* Receive last fragment in other view: 0 -> me. */
  std::unique_ptr<Gcs_xcom_nodes> xcom_nodes_last_view(new Gcs_xcom_nodes());
  xcom_nodes_last_view->add_node(Gcs_xcom_node_information(
      m_mock_xcom_address.get_member_address(), Gcs_xcom_uuid(), 0, true));
  m_fragmentation_stage->update_members_information(
      Gcs_member_identifier(m_mock_xcom_address.get_member_address()),
      *xcom_nodes_last_view);
  // I am currently node 0.
  synode_no synod_last_fragment;
  synod_last_fragment.group_id =
      Gcs_xcom_utils::build_xcom_group_id(m_mock_gid);
  synod_last_fragment.msgno = 1;
  synod_last_fragment.node = 0;
  std::tie(buffer_pointer, buffer_size) = packets_out.at(1).serialize();
  auto last_fragment = Gcs_packet::make_incoming_packet(
      std::move(buffer_pointer), buffer_size, synod_last_fragment,
      m_xcom_comm_if.get_msg_pipeline());
  Gcs_view mock_view(
      {Gcs_member_identifier(m_mock_xcom_address.get_member_address())},
      Gcs_xcom_view_identifier(0, 0), {}, {}, m_mock_gid);
  EXPECT_CALL(m_mock_vce, get_unsafe_current_view())
      .Times(1)
      .WillOnce(Return(&mock_view));
  m_xcom_comm_if.process_user_data_packet(std::move(last_fragment),
                                          std::move(xcom_nodes_last_view));

  m_xcom_comm_if.remove_event_listener(listener_ref);
}

}  // namespace gcs_message_stage_fragmentation_unittest
