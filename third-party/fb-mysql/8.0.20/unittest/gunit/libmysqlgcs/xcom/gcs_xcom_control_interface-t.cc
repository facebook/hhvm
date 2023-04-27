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
#include "unittest/gunit/libmysqlgcs/include/gcs_base_test.h"

#include <memory>
#include <string>
#include <vector>

#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_control_interface.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_group_member_information.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_notification.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_state_exchange.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/gcs_xcom_utils.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/xcom/node_set.h"
#include "plugin/group_replication/libmysqlgcs/src/bindings/xcom/xcom/synode_no.h"
#include "plugin/group_replication/libmysqlgcs/xdr_gen/xcom_vp.h"
#include "template_utils.h"

using std::string;

namespace gcs_xcom_control_unittest {
typedef enum { JJ = 0, LL = 1, JL = 2, LJ = 3 } InvocationOrder;

class InvocationHelper {
 public:
  InvocationHelper(Gcs_xcom_control *x, InvocationOrder o)
      : xcom_control_if(x),
        mutex(),
        counter(0),
        order(o),
        count_fail(0),
        count_success(0) {
    mutex.init(PSI_NOT_INSTRUMENTED, nullptr);
  }

  ~InvocationHelper() { mutex.destroy(); }

  void invokeMethod() {
    int mycounter = 0;
    enum_gcs_error ret = GCS_OK;

    if (order == JJ) {
      ret = xcom_control_if->join();
    } else if (order == LL) {
      ret = xcom_control_if->leave();
    } else if (order == JL) {
      mutex.lock();
      mycounter = counter++;
      if (mycounter == 0) {
        ret = xcom_control_if->join();
      } else {
        ret = xcom_control_if->leave();
      }
      mutex.unlock();
    } else if (order == LJ) {
      mutex.lock();
      mycounter = counter++;
      if (mycounter == 0) {
        ret = xcom_control_if->leave();
      } else {
        ret = xcom_control_if->join();
      }
      mutex.unlock();
    } else {
      assert(false);
    }

    mutex.lock();
    if (ret == GCS_OK) {
      count_success++;
    } else {
      count_fail++;
    }
    mutex.unlock();
  }

 private:
  Gcs_xcom_control *xcom_control_if;
  My_xp_mutex_impl mutex;
  int counter;
  InvocationOrder order;

 public:
  int count_fail;
  int count_success;
};

void homemade_free_site_def(unsigned int n, site_def *s,
                            node_address *node_addrs) {
  // TODO: replace the following with free_site_def(site_config) once
  //       the header file in site_def.h is fixed
  for (unsigned int i = 0; i < n; i++) free(node_addrs[i].uuid.data.data_val);
  free_node_set(&s->global_node_set);
  free_node_set(&s->local_node_set);
  remove_node_list(n, node_addrs, &s->nodes);
  free(s->nodes.node_list_val);
  free(s);
}

class mock_gcs_xcom_view_change_control_interface
    : public Gcs_xcom_view_change_control_interface {
 private:
  Gcs_view *m_current_view;
  bool m_belongs_to_group;
  My_xp_mutex_impl m_mutex_current_view;
  My_xp_mutex_impl m_joining_leaving_mutex;
  bool m_joining;
  bool m_leaving;

 public:
  mock_gcs_xcom_view_change_control_interface()
      : m_current_view(nullptr),
        m_belongs_to_group(false),
        m_mutex_current_view(),
        m_joining_leaving_mutex(),
        m_joining(false),
        m_leaving(false)

  {
    m_mutex_current_view.init(PSI_NOT_INSTRUMENTED, nullptr);
    m_joining_leaving_mutex.init(PSI_NOT_INSTRUMENTED, nullptr);
  }

  ~mock_gcs_xcom_view_change_control_interface() {
    m_mutex_current_view.destroy();
    m_joining_leaving_mutex.destroy();
  }

  MOCK_METHOD0(start_view_exchange, void());
  MOCK_METHOD0(end_view_exchange, void());
  MOCK_METHOD0(wait_for_view_change_end, void());
  MOCK_METHOD0(is_view_changing, bool());

  bool start_leave() {
    bool retval = false;

    m_joining_leaving_mutex.lock();
    retval = m_joining || m_leaving;
    if (!retval) m_leaving = true;
    m_joining_leaving_mutex.unlock();

    return !retval;
  }

  void end_leave() {
    m_joining_leaving_mutex.lock();
    m_leaving = false;
    m_joining_leaving_mutex.unlock();
  }

  bool is_leaving() {
    bool retval;

    m_joining_leaving_mutex.lock();
    retval = m_leaving;
    m_joining_leaving_mutex.unlock();

    return retval;
  }

  bool start_join() {
    bool retval = false;

    m_joining_leaving_mutex.lock();
    retval = m_joining || m_leaving;
    if (!retval) m_joining = true;
    m_joining_leaving_mutex.unlock();

    return !retval;
  }

  void end_join() {
    m_joining_leaving_mutex.lock();
    m_joining = false;
    m_joining_leaving_mutex.unlock();
  }

  bool is_joining() {
    bool retval;

    m_joining_leaving_mutex.lock();
    retval = m_joining;
    m_joining_leaving_mutex.unlock();

    return retval;
  }

  void set_current_view(Gcs_view *view) {
    m_mutex_current_view.lock();
    delete m_current_view;
    m_current_view = view;
    m_mutex_current_view.unlock();
  }

  void set_unsafe_current_view(Gcs_view *view) { set_current_view(view); }

  Gcs_view *get_current_view() {
    Gcs_view *view = nullptr;

    m_mutex_current_view.lock();
    if (m_current_view != nullptr) view = new Gcs_view(*m_current_view);
    m_mutex_current_view.unlock();

    return view;
  }

  Gcs_view *get_unsafe_current_view() { return m_current_view; }

  bool belongs_to_group() { return m_belongs_to_group; }

  void set_belongs_to_group(bool belong) { m_belongs_to_group = belong; }
};

// This typedef is needed because GMock can't deal with multiple template args.
typedef std::map<Gcs_member_identifier, Xcom_member_state *> Stored_States;

class mock_gcs_xcom_state_exchange_interface
    : public Gcs_xcom_state_exchange_interface {
 private:
  int m_process_member_state_iteration;

  bool free_xcom_member_state(Xcom_member_state *m) {
    delete m;
    if (m_process_member_state_iteration == 0) {
      m_process_member_state_iteration++;
      return false;
    } else
      return true;
  }

  bool free_members_joined(std::vector<Gcs_member_identifier *> &total,
                           std::vector<Gcs_member_identifier *> &joined) {
    std::vector<Gcs_member_identifier *>::iterator it;

    for (it = total.begin(); it != total.end(); it++) delete (*it);
    total.clear();

    for (it = joined.begin(); it != joined.end(); it++) delete (*it);
    joined.clear();

    return false;
  }

 public:
  mock_gcs_xcom_state_exchange_interface()
      : m_process_member_state_iteration(0) {
    ON_CALL(*this, process_member_state(_, _, _, _))
        .WillByDefault(WithArgs<0>(Invoke(
            this,
            &mock_gcs_xcom_state_exchange_interface::free_xcom_member_state)));

    ON_CALL(*this, state_exchange(_, _, _, _, _, _, _, _, _))
        .WillByDefault(WithArgs<1, 3>(Invoke(
            this,
            &mock_gcs_xcom_state_exchange_interface::free_members_joined)));
    ON_CALL(*this, compute_incompatible_members())
        .WillByDefault(Return(std::vector<Gcs_xcom_node_information>()));
    ON_CALL(*this, process_recovery_state()).WillByDefault(Return(true));
  }

  MOCK_METHOD0(init, void());
  MOCK_METHOD0(reset, void());
  MOCK_METHOD0(reset_with_flush, void());
  MOCK_METHOD0(end, void());

  MOCK_METHOD0(compute_incompatible_members,
               std::vector<Gcs_xcom_node_information>());
  MOCK_METHOD0(process_recovery_state, bool());

  MOCK_METHOD9(
      state_exchange,
      bool(synode_no configuration_id,
           std::vector<Gcs_member_identifier *> &total,
           std::vector<Gcs_member_identifier *> &left,
           std::vector<Gcs_member_identifier *> &joined,
           std::vector<std::unique_ptr<Gcs_message_data>> &exchangeable_data,
           Gcs_view *current_view, std::string *group,
           const Gcs_member_identifier &local_info, const Gcs_xcom_nodes &));
  MOCK_METHOD4(process_member_state,
               bool(Xcom_member_state *ms_info,
                    const Gcs_member_identifier &p_id,
                    Gcs_protocol_version max_protocol_version,
                    Gcs_protocol_version used_protocol_version));
  MOCK_METHOD0(get_new_view_id, Gcs_xcom_view_identifier *());
  MOCK_METHOD0(get_joined, std::set<Gcs_member_identifier *> *());
  MOCK_METHOD0(get_left, std::set<Gcs_member_identifier *> *());
  MOCK_METHOD0(get_total, std::set<Gcs_member_identifier *> *());
  MOCK_METHOD0(get_group, string *());
  MOCK_METHOD0(get_member_states, Stored_States *());
  MOCK_METHOD0(compute_maximum_supported_protocol_version, void());
};

class mock_gcs_xcom_proxy : public Gcs_xcom_proxy_base {
 private:
  node_list nl;

  void reset_me() { ::delete_node_address(nl.node_list_len, nl.node_list_val); }

 public:
  mock_gcs_xcom_proxy() {
    ON_CALL(*this, xcom_input_connect(_, _)).WillByDefault(Return(true));
    ON_CALL(*this, test_xcom_tcp_connection(_, _)).WillByDefault(Return(true));
    ON_CALL(*this, xcom_client_boot(_, _)).WillByDefault(Return(true));
    ON_CALL(*this, xcom_client_add_node(_, _, _)).WillByDefault(Return(false));
    ON_CALL(*this, xcom_client_send_data(_, _)).WillByDefault(Return(10));
    ON_CALL(*this, new_node_address_uuid(_, _, _))
        .WillByDefault(WithArgs<0, 1, 2>(Invoke(::new_node_address_uuid)));
    ON_CALL(*this, delete_node_address(_, _))
        .WillByDefault(WithArgs<0, 1>(Invoke(::delete_node_address)));
    ON_CALL(*this, xcom_wait_ready()).WillByDefault(Return(GCS_OK));
    ON_CALL(*this, xcom_wait_for_xcom_comms_status_change(_))
        .WillByDefault(SetArgReferee<0>(XCOM_COMMS_OK));
    ON_CALL(*this, xcom_wait_exit()).WillByDefault(Return(GCS_OK));
    ON_CALL(*this, xcom_is_exit()).WillByDefault(Return(true));
  }

  MOCK_METHOD3(new_node_address_uuid,
               node_address *(unsigned int n, char *names[], blob uuids[]));
  MOCK_METHOD2(delete_node_address, void(unsigned int n, node_address *na));
  MOCK_METHOD3(xcom_client_add_node, bool(connection_descriptor *con,
                                          node_list *nl, uint32_t group_id));
  MOCK_METHOD3(xcom_client_get_event_horizon,
               bool(connection_descriptor *con, uint32_t group_id,
                    xcom_event_horizon &event_horizon));
  MOCK_METHOD2(xcom_client_get_event_horizon,
               bool(uint32_t group_id, xcom_event_horizon &event_horizon));
  MOCK_METHOD3(xcom_client_set_event_horizon,
               bool(connection_descriptor *con, uint32_t group_id,
                    xcom_event_horizon event_horizon));
  MOCK_METHOD2(xcom_client_set_event_horizon,
               bool(uint32_t group_id, xcom_event_horizon event_horizon));
  MOCK_METHOD4(xcom_client_get_synode_app_data,
               bool(connection_descriptor *con, uint32_t group_id,
                    synode_no_array &synodes, synode_app_data_array &reply));
  MOCK_METHOD1(xcom_client_set_cache_size, bool(uint64_t size));
  MOCK_METHOD2(xcom_client_remove_node, bool(node_list *nl, uint32_t group_id));
  MOCK_METHOD3(xcom_client_remove_node, bool(connection_descriptor *con,
                                             node_list *nl, uint32_t group_id));
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
  MOCK_METHOD2(test_xcom_tcp_connection,
               bool(std::string &host, xcom_port port));
};

class mock_gcs_control_event_listener : public Gcs_control_event_listener {
 public:
  MOCK_CONST_METHOD2(on_view_changed,
                     void(const Gcs_view &new_view,
                          const Exchanged_data &exchanged_data));
  MOCK_CONST_METHOD0(get_exchangeable_data, Gcs_message_data *());
  MOCK_CONST_METHOD2(
      on_suspicions,
      void(const std::vector<Gcs_member_identifier> &members,
           const std::vector<Gcs_member_identifier> &unreachable));
};

class mock_my_xp_socket_util : public My_xp_socket_util {
 public:
  mock_my_xp_socket_util() {
    ON_CALL(*this, disable_nagle_in_socket(_)).WillByDefault(Return(0));
  }

  MOCK_METHOD1(disable_nagle_in_socket, int(int fd));
};

class mock_gcs_xcom_control : public Gcs_xcom_control {
 public:
  mock_gcs_xcom_control(Gcs_xcom_node_address *xcom_node_address,
                        std::vector<Gcs_xcom_node_address *> &xcom_peers,
                        Gcs_group_identifier group_identifier,
                        Gcs_xcom_proxy *xcom_proxy,
                        Gcs_xcom_group_management *xcom_group_management,
                        Gcs_xcom_engine *gcs_engine,
                        Gcs_xcom_state_exchange_interface *state_exchange,
                        Gcs_xcom_view_change_control_interface *view_control,
                        bool boot, My_xp_socket_util *socket_util)
      : Gcs_xcom_control(xcom_node_address, xcom_peers, group_identifier,
                         xcom_proxy, xcom_group_management, gcs_engine,
                         state_exchange, view_control, boot, socket_util) {}

  enum_gcs_error join() { return join(nullptr); }

  enum_gcs_error join(Gcs_view *view) {
    enum_gcs_error ret = GCS_NOK;

    if (!m_view_control->start_join()) {
      return GCS_NOK;
    }

    if (belongs_to_group()) {
      m_view_control->end_join();
      return GCS_NOK;
    }

    if (!m_boot && m_initial_peers.empty()) {
      m_view_control->end_join();
      return GCS_NOK;
    }

    ret = do_join(false);

    if (ret == GCS_OK) {
      m_view_control->set_current_view(view);
      m_view_control->set_belongs_to_group(true);
    }

    return ret;
  }

  enum_gcs_error leave() {
    enum_gcs_error ret = GCS_NOK;

    if (!m_view_control->start_leave()) {
      return GCS_NOK;
    }

    if (!belongs_to_group()) {
      m_view_control->end_leave();
      return GCS_NOK;
    }

    ret = do_leave();

    if (ret == GCS_OK) {
      m_view_control->set_current_view(nullptr);
      m_view_control->set_belongs_to_group(false);
    }

    return ret;
  }

  void set_xcom_running(bool running) { m_xcom_running = running; }
};

static mock_gcs_xcom_control *extern_xcom_control_if;

void finalize_xcom() { extern_xcom_control_if->do_leave(); }

class XComControlTest : public GcsBaseTest {
 protected:
  XComControlTest() {}

  virtual ~XComControlTest() {}

  virtual void SetUp() {
    m_wait_called = false;
    m_wait_called_mutex.init(PSI_NOT_INSTRUMENTED, nullptr);
    m_wait_called_cond.init(PSI_NOT_INSTRUMENTED);

    mock_se = new mock_gcs_xcom_state_exchange_interface();

    mock_vce = new mock_gcs_xcom_view_change_control_interface();

    xcom_node_address = new Gcs_xcom_node_address("127.0.0.1:12345");
    peers.push_back(new Gcs_xcom_node_address("127.0.0.1:12345"));
    peers.push_back(new Gcs_xcom_node_address("127.0.0.1:12346"));
    peers.push_back(new Gcs_xcom_node_address("127.0.0.1:12347"));

    string group_name("only_group");
    group_id = new Gcs_group_identifier(group_name);

    mock_socket_util = new mock_my_xp_socket_util();

    gcs_engine.initialize(nullptr);

    xcom_group_mgm = new Gcs_xcom_group_management(&proxy, *group_id);

    xcom_control_if = new mock_gcs_xcom_control(
        xcom_node_address, peers, *group_id, &proxy, xcom_group_mgm,
        &gcs_engine, mock_se, mock_vce, true, mock_socket_util);
    extern_xcom_control_if = xcom_control_if;

    My_xp_util::init_time();
  }

  virtual void TearDown() {
    gcs_engine.finalize(finalize_xcom);

    delete mock_socket_util;
    delete group_id;
    delete xcom_group_mgm;
    delete xcom_control_if;
    delete mock_se;
    delete mock_vce;
    delete xcom_node_address;
    std::vector<Gcs_xcom_node_address *>::iterator it;
    for (it = peers.begin(); it != peers.end(); ++it) delete (*it);
    peers.clear();

    m_wait_called_mutex.destroy();
    m_wait_called_cond.destroy();
  }

  Gcs_xcom_node_address *xcom_node_address;
  std::vector<Gcs_xcom_node_address *> peers;

  Gcs_group_identifier *group_id;

  mock_gcs_xcom_proxy proxy;
  mock_gcs_control_event_listener mock_ev_listener;

  mock_gcs_xcom_control *xcom_control_if;
  mock_gcs_xcom_state_exchange_interface *mock_se;
  mock_gcs_xcom_view_change_control_interface *mock_vce;
  mock_my_xp_socket_util *mock_socket_util;

  bool m_wait_called;
  My_xp_mutex_impl m_wait_called_mutex;
  My_xp_cond_impl m_wait_called_cond;

  Gcs_xcom_engine gcs_engine;

  Gcs_xcom_group_management *xcom_group_mgm;

 public:
  Gcs_view *create_fake_view() {
    string address = xcom_node_address->get_member_address();
    Gcs_member_identifier local_member_information(address);

    std::vector<Gcs_member_identifier> members;
    members.push_back(local_member_information);

    Gcs_xcom_view_identifier view_id(111111, 1);
    std::vector<Gcs_member_identifier> leaving;
    std::vector<Gcs_member_identifier> joined;

    Gcs_group_identifier fake_group_id(group_id->get_group_id());

    Gcs_view *fake_view =
        new Gcs_view(members, view_id, leaving, joined, fake_group_id);
    return fake_view;
  }
};

TEST_F(XComControlTest, JoinLeaveTest) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_))
      .Times(1)
      .WillOnce(SetArgReferee<0>(GCS_OK));
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_wait_exit()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

TEST_F(XComControlTest, JoinTestFailedMultipleJoins) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_))
      .Times(1)
      .WillOnce(SetArgReferee<0>(GCS_OK));
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_wait_exit()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

TEST_F(XComControlTest, JoinTestFailedToStartComms) {
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);

  /*
    The join is forced to wait until the XCOM's tread is running.
    In this test case though, we make the operation fail.
  */
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_))
      .Times(1)
      .WillOnce(SetArgReferee<0>(XCOM_COMMS_OTHER));

  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

/* GCS communicates with the local XCom instance via a queue. Whenever GCS
 * pushes a command into this queue, GCS signals XCom that the queue has
 * commands.
 * When GCS sets up XCom, GCS connects to the XCom signalling mechanism.
 * This tests ensures that GCS handles a failure to connect to the signalling
 * mechanism by failing to join the group. */
TEST_F(XComControlTest, JoinTestFailedToConnectToXComQueueSignallingMechanism) {
  /* We fail to connect to XCom's local server. */
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1).WillOnce(Return(false));
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);

  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

/* This tests ensures that GCS handles a failure to send the boot command to
 * XCom by failing to join the group. */
TEST_F(XComControlTest, JoinTestFailedToSendBootToXCom) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  /* We fail to send the boot command. */
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1).WillOnce(Return(false));
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);

  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

TEST_F(XComControlTest, JoinTestTimeoutStartingComms) {
  Gcs_xcom_proxy *my_proxy = new Gcs_xcom_proxy_impl();

  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);

  /*
    The join is forced to wait until the XCOM's tread is running.
    In this test case though, we make the operation time out.
  */
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_))
      .Times(1)
      .WillOnce(Invoke(
          my_proxy, &Gcs_xcom_proxy::xcom_wait_for_xcom_comms_status_change));

  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  delete my_proxy;
}

TEST_F(XComControlTest, JoinTestFailedToStartXCom) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1).WillOnce(Return(GCS_NOK));
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_)).Times(1);

  enum_gcs_error result = xcom_control_if->join();

  ASSERT_EQ(GCS_NOK, result);
}

TEST_F(XComControlTest, JoinTestTimeoutStartingXCom) {
  Gcs_xcom_proxy *my_proxy = new Gcs_xcom_proxy_impl();

  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready())
      .Times(1)
      .WillOnce(Invoke(my_proxy, &Gcs_xcom_proxy::xcom_wait_ready));
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_)).Times(1);

  enum_gcs_error result = xcom_control_if->join();

  ASSERT_EQ(GCS_NOK, result);

  delete my_proxy;
}

TEST_F(XComControlTest, JoinTestWithoutBootNorPeers) {
  EXPECT_CALL(proxy, new_node_address_uuid(_, _, _)).Times(0);
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(0);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(0);

  xcom_control_if->set_boot_node(false);
  std::vector<Gcs_xcom_node_address *> peers;
  xcom_control_if->set_peer_nodes(peers);

  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

TEST_F(XComControlTest, JoinTestSkipOwnNodeAndCycleThroughPeerNodes) {
  connection_descriptor *con =
      (connection_descriptor *)malloc(sizeof(connection_descriptor *));
  con->fd = 0;

  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  // Fail to connect to the peer every time.
  EXPECT_CALL(proxy, xcom_client_open_connection(Eq("127.0.0.1"), Eq(12346)))
      .Times(3)
      .WillRepeatedly(Return((connection_descriptor *)nullptr));

  /*
   Fail to connect on the first attempt.
   Connect on the second attempt, but fail to disable the Nagle algorithm.
   Succeed on the third attempt.
  */
  EXPECT_CALL(proxy, xcom_client_open_connection(Eq("127.0.0.1"), Eq(12347)))
      .Times(3)
      .WillOnce(Return((connection_descriptor *)nullptr))
      .WillRepeatedly(Return((connection_descriptor *)con));
  EXPECT_CALL(*mock_socket_util, disable_nagle_in_socket(_))
      .Times(2)
      .WillOnce(Return(-1))
      .WillOnce(Return(0));

  EXPECT_CALL(proxy, xcom_client_add_node(_, _, _))
      .Times(1)
      .WillOnce(Return(1));
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_close_connection(_))
      .Times(2)
      .WillRepeatedly(Return(0));

  xcom_control_if->set_boot_node(false);
  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  free(con);
}

TEST_F(XComControlTest, JoinTestAllPeersUnavailable) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(1);
  /*
   Fail to connect to all peers every time.
   peers.size() - 1 because we skip our own address.
  */
  EXPECT_CALL(proxy, xcom_client_open_connection(_, _))
      .Times((peers.size() - 1) * Gcs_xcom_control::CONNECTION_ATTEMPTS)
      .WillRepeatedly(Return((connection_descriptor *)nullptr));
  EXPECT_CALL(*mock_socket_util, disable_nagle_in_socket(_)).Times(0);
  EXPECT_CALL(proxy, xcom_client_add_node(_, _, _)).Times(0);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _, _)).Times(0);
  EXPECT_CALL(proxy, xcom_client_close_connection(_)).Times(0);

  xcom_control_if->set_boot_node(false);
  enum_gcs_error result = xcom_control_if->join();
  ASSERT_EQ(GCS_NOK, result);
}

TEST_F(XComControlTest, LeaveTestWithoutJoin) {
  EXPECT_CALL(proxy, new_node_address_uuid(_, _, _)).Times(0);
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(0);
  EXPECT_CALL(proxy, xcom_init(_)).Times(0);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(0);

  enum_gcs_error result = xcom_control_if->leave();
  ASSERT_EQ(GCS_NOK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

TEST_F(XComControlTest, LeaveTestMultiMember) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);

  string member_id_1 = xcom_node_address->get_member_address();
  Gcs_member_identifier local_member_information_1(member_id_1);

  string member_id_2("127.0.0.1:12343");
  Gcs_member_identifier local_member_information_2(member_id_2);

  std::vector<Gcs_member_identifier> members;
  members.push_back(local_member_information_1);
  members.push_back(local_member_information_2);

  Gcs_xcom_view_identifier view_id(111111, 1);
  std::vector<Gcs_member_identifier> leaving;
  std::vector<Gcs_member_identifier> joined;

  Gcs_group_identifier fake_group_id(group_id->get_group_id());

  Gcs_view *fake_old_view =
      new Gcs_view(members, view_id, leaving, joined, fake_group_id);

  enum_gcs_error result = xcom_control_if->join(fake_old_view);
  ASSERT_EQ(GCS_OK, result);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
}

TEST_F(XComControlTest, GetLocalInformationTest) {
  Gcs_member_identifier result = xcom_control_if->get_local_member_identifier();
  std::string address = xcom_node_address->get_member_address();
  ASSERT_EQ(address, result.get_member_id());
}

TEST_F(XComControlTest, SetEventListenerTest) {
  mock_gcs_control_event_listener control_listener;

  int reference = xcom_control_if->add_event_listener(control_listener);

  ASSERT_NE(0, reference);
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->size());
}

TEST_F(XComControlTest, SetEventListenersTest) {
  mock_gcs_control_event_listener control_listener;
  mock_gcs_control_event_listener another_control_listener;

  int reference = xcom_control_if->add_event_listener(control_listener);
  int another_reference =
      xcom_control_if->add_event_listener(another_control_listener);

  ASSERT_NE(0, reference);
  ASSERT_NE(0, another_reference);
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->count(another_reference));
  ASSERT_EQ((long unsigned int)2,
            xcom_control_if->get_event_listeners()->size());
  ASSERT_NE(reference, another_reference);
}

TEST_F(XComControlTest, RemoveEventListenerTest) {
  mock_gcs_control_event_listener control_listener;
  mock_gcs_control_event_listener another_control_listener;

  int reference = xcom_control_if->add_event_listener(control_listener);
  int another_reference =
      xcom_control_if->add_event_listener(another_control_listener);

  xcom_control_if->remove_event_listener(reference);

  ASSERT_NE(0, reference);
  ASSERT_NE(0, another_reference);
  ASSERT_EQ((long unsigned int)0,
            xcom_control_if->get_event_listeners()->count(reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->count(another_reference));
  ASSERT_EQ((long unsigned int)1,
            xcom_control_if->get_event_listeners()->size());
  ASSERT_NE(reference, another_reference);
}

Gcs_message *create_state_exchange_msg(Gcs_member_identifier &member_id,
                                       Gcs_group_identifier &group_id,
                                       Stored_States *out_stored_states) {
  Gcs_message_data *dummy = new Gcs_message_data(0, 3);
  uchar to_append = (uchar)1;

  dummy->append_to_payload(&to_append, 1);
  dummy->append_to_payload(&to_append, 1);
  dummy->append_to_payload(&to_append, 1);

  const Gcs_xcom_view_identifier view_id(999999, 1);
  synode_no configuration_id = null_synode;
  Gcs_xcom_synode_set snapshot;
  Xcom_member_state *member_state = new Xcom_member_state(
      view_id, configuration_id, Gcs_protocol_version::HIGHEST_KNOWN, snapshot,
      nullptr, 0);

  (*out_stored_states)[member_id] = member_state;

  uint64_t buffer_len = member_state->get_encode_header_size() +
                        dummy->get_encode_size() +
                        member_state->get_encode_snapshot_size();
  uchar *buffer = static_cast<uchar *>(malloc(buffer_len * sizeof(uchar)));
  uchar *slider = buffer;

  auto header_len = member_state->get_encode_header_size();
  member_state->encode_header(slider, &header_len);
  slider += header_len;

  auto payload_len = dummy->get_encode_size();
  dummy->encode(slider, &payload_len);
  slider += payload_len;

  auto snapshot_len = member_state->get_encode_snapshot_size();
  member_state->encode_snapshot(slider, &snapshot_len);
  slider += snapshot_len;

  Gcs_message *msg =
      new Gcs_message(member_id, group_id, new Gcs_message_data(0, buffer_len));
  msg->get_message_data().append_to_payload(buffer, buffer_len);

  delete dummy;
  free(buffer);

  return msg;
}

TEST_F(XComControlTest, ViewChangedJoiningTest) {
  Gcs_xcom_uuid uuid_1 = Gcs_xcom_uuid::create_uuid();
  blob blob_1 = {{0, static_cast<char *>(malloc(uuid_1.actual_value.size()))}};
  uuid_1.encode(reinterpret_cast<uchar **>(&blob_1.data.data_val),
                &blob_1.data.data_len);

  Gcs_xcom_uuid uuid_2 = Gcs_xcom_uuid::create_uuid();
  blob blob_2 = {{0, static_cast<char *>(malloc(uuid_2.actual_value.size()))}};
  uuid_2.encode(reinterpret_cast<uchar **>(&blob_2.data.data_val),
                &blob_2.data.data_len);

  node_address node_addrs[2] = {
      {const_cast<char *>("127.0.0.1:12345"), blob_1, {x_1_0, x_1_2}},
      {const_cast<char *>("127.0.0.1:12346"), blob_2, {x_1_0, x_1_2}}};

  // Common unit test data
  Gcs_xcom_view_identifier *view_id = new Gcs_xcom_view_identifier(999999, 27);

  string member_addr_1(node_addrs[0].address);
  Gcs_member_identifier *node1_member_id =
      new Gcs_member_identifier(member_addr_1);

  string member_addr_2(node_addrs[1].address);
  Gcs_member_identifier *node2_member_id =
      new Gcs_member_identifier(member_addr_2);

  std::set<Gcs_member_identifier *> *total_set =
      new std::set<Gcs_member_identifier *>();
  std::set<Gcs_member_identifier *> *join_set =
      new std::set<Gcs_member_identifier *>();
  std::set<Gcs_member_identifier *> *left_set =
      new std::set<Gcs_member_identifier *>();

  total_set->insert(node1_member_id);
  total_set->insert(node2_member_id);

  join_set->insert(node2_member_id);

  site_def *site_config = new_site_def();
  init_site_def(2, node_addrs, site_config);
  site_config->x_proto = static_cast<xcom_proto>(1);
  site_config->nodeno = 1;

  node_set nodes;
  alloc_node_set(&nodes, 2);
  set_node_set(&nodes);

  Stored_States stored_states;
  Gcs_message *state_message1 =
      create_state_exchange_msg(*node1_member_id, *group_id, &stored_states);
  Gcs_message *state_message2 =
      create_state_exchange_msg(*node2_member_id, *group_id, &stored_states);

  EXPECT_CALL(proxy, find_site_def(_)).Times(0);
  EXPECT_CALL(mock_ev_listener, on_view_changed(_, _)).Times(1);
  EXPECT_CALL(*mock_se, state_exchange(_, _, _, _, _, _, _, _, _)).Times(1);
  EXPECT_CALL(*mock_se, process_member_state(_, _, _, _)).Times(2);
  EXPECT_CALL(*mock_se, process_recovery_state())
      .Times(1)
      .WillOnce(Return(true));
  EXPECT_CALL(*mock_se, get_new_view_id()).Times(1).WillOnce(Return(view_id));
  EXPECT_CALL(*mock_se, get_joined()).Times(1).WillOnce(Return(join_set));
  EXPECT_CALL(*mock_se, get_left()).Times(1).WillOnce(Return(left_set));
  EXPECT_CALL(*mock_se, get_total()).Times(1).WillOnce(Return(total_set));
  EXPECT_CALL(*mock_vce, start_view_exchange()).Times(1);
  EXPECT_CALL(*mock_vce, end_view_exchange()).Times(1);
  EXPECT_CALL(*mock_se, reset()).Times(0);
  EXPECT_CALL(*mock_se, reset_with_flush()).Times(1);
  EXPECT_CALL(*mock_se, end()).Times(1);
  EXPECT_CALL(*mock_se, get_member_states())
      .Times(1)
      .WillOnce(Return(&stored_states));
  EXPECT_CALL(*mock_vce, is_view_changing()).WillRepeatedly(Return(true));

  xcom_control_if->add_event_listener(mock_ev_listener);

  /*
    Initially the node does nto belong to a group and has not
    installed any view.
  */
  ASSERT_FALSE(xcom_control_if->belongs_to_group());
  ASSERT_TRUE(xcom_control_if->get_current_view() == nullptr);

  synode_no message_id;
  message_id.group_id = Gcs_xcom_utils::build_xcom_group_id(*this->group_id);
  message_id.msgno = 4;
  message_id.node = 0;

  Gcs_xcom_nodes *xcom_nodes = new Gcs_xcom_nodes(site_config, nodes);

  /*
    Process a global view message delivered by XCOM but say
    that a view with such information was never installed.

    Note that nodes are freed by the caller.
  */
  bool view_accepted = !xcom_control_if->xcom_receive_global_view(
      null_synode, message_id, xcom_nodes, false, null_synode);
  ASSERT_TRUE(view_accepted);

  /*
    Process a global view message delivered by XCOM but say
    that a view with such information was already installed.

    Note that nodes are freed by the caller.
  */
  view_accepted = !xcom_control_if->xcom_receive_global_view(
      null_synode, message_id, xcom_nodes, true, null_synode);
  ASSERT_FALSE(view_accepted);

  /*
    Process the state exchange messages so that the new
    view can be installed.
  */
  xcom_control_if->process_control_message(
      state_message1, Gcs_protocol_version::V1, Gcs_protocol_version::V1);
  xcom_control_if->process_control_message(
      state_message2, Gcs_protocol_version::V1, Gcs_protocol_version::V1);

  Gcs_view *current_view = xcom_control_if->get_current_view();
  ASSERT_TRUE(xcom_control_if->belongs_to_group());
  ASSERT_TRUE(current_view != nullptr);

  const Gcs_xcom_view_identifier &current_view_id =
      down_cast<const Gcs_xcom_view_identifier &>(current_view->get_view_id());
  ASSERT_TRUE((&current_view_id) != nullptr);
  ASSERT_EQ(typeid(Gcs_xcom_view_identifier).name(),
            typeid(current_view_id).name());

  Gcs_xcom_view_identifier *xcom_view_id =
      const_cast<Gcs_xcom_view_identifier *>(&current_view_id);

  ASSERT_EQ(view_id->get_fixed_part(), xcom_view_id->get_fixed_part());
  ASSERT_EQ(view_id->get_monotonic_part() + 1,
            xcom_view_id->get_monotonic_part());
  ASSERT_EQ((size_t)2, current_view->get_members().size());
  ASSERT_EQ((size_t)1, current_view->get_joined_members().size());

  delete view_id;
  delete node2_member_id;
  delete node1_member_id;
  delete total_set;
  delete join_set;
  delete left_set;
  delete current_view;
  delete xcom_nodes;
  mock_vce->set_current_view(nullptr);

  // TODO: replace the following with free_site_def(site_config) once
  //       the header file in site_def.h is fixed
  homemade_free_site_def(2, site_config, node_addrs);
  free_node_set(&nodes);

  // reclaim Xcom_member_states
  Stored_States::iterator it;
  for (it = stored_states.begin(); it != stored_states.end(); it++)
    delete (*it).second;
}

TEST_F(XComControlTest, FailedNodeRemovalTest) {
  // Setting Expectations and Return Values
  // First the node joins the group.
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(4);
  EXPECT_CALL(mock_ev_listener, on_view_changed(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(3);

  // Get suspicions manager and enable majority
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(0ul);

  Gcs_xcom_uuid uuid_1 = Gcs_xcom_uuid::create_uuid();
  blob blob_1 = {{0, static_cast<char *>(malloc(uuid_1.actual_value.size()))}};
  uuid_1.encode(reinterpret_cast<uchar **>(&blob_1.data.data_val),
                &blob_1.data.data_len);

  Gcs_xcom_uuid uuid_2 = Gcs_xcom_uuid::create_uuid();
  blob blob_2 = {{0, static_cast<char *>(malloc(uuid_2.actual_value.size()))}};
  uuid_2.encode(reinterpret_cast<uchar **>(&blob_2.data.data_val),
                &blob_2.data.data_len);

  Gcs_xcom_uuid uuid_3 = Gcs_xcom_uuid::create_uuid();
  blob blob_3 = {{0, static_cast<char *>(malloc(uuid_3.actual_value.size()))}};
  uuid_3.encode(reinterpret_cast<uchar **>(&blob_3.data.data_val),
                &blob_3.data.data_len);

  node_address node_addrs[3] = {
      {const_cast<char *>("127.0.0.1:12345"), blob_1, {x_1_0, x_1_2}},
      {const_cast<char *>("127.0.0.1:12343"), blob_2, {x_1_0, x_1_2}},
      {const_cast<char *>("127.0.0.1:12341"), blob_3, {x_1_0, x_1_2}}};

  site_def *site_config = new_site_def();
  init_site_def(3, node_addrs, site_config);
  site_config->x_proto = static_cast<xcom_proto>(1);
  site_config->nodeno = 0;

  node_set nodes;
  alloc_node_set(&nodes, 3);
  set_node_set(&nodes);
  nodes.node_set_val[1] = 0;

  EXPECT_CALL(proxy, find_site_def(_)).Times(0);

  // Setting fake values
  string member_id_1("127.0.0.1:12345");
  Gcs_member_identifier local_member_information_1(member_id_1);

  string member_id_2("127.0.0.1:12343");
  Gcs_member_identifier local_member_information_2(member_id_2);

  string member_id_3("127.0.0.1:12341");
  Gcs_member_identifier local_member_information_3(member_id_3);

  std::vector<Gcs_member_identifier> members;
  members.push_back(local_member_information_1);
  members.push_back(local_member_information_2);
  members.push_back(local_member_information_3);

  Gcs_xcom_view_identifier view_id(111111, 1);
  std::vector<Gcs_member_identifier> leaving;
  std::vector<Gcs_member_identifier> joined;

  Gcs_group_identifier fake_group_id(group_id->get_group_id());

  Gcs_view *fake_old_view =
      new Gcs_view(members, view_id, leaving, joined, fake_group_id);

  // registering the listener
  int listener_handle = xcom_control_if->add_event_listener(mock_ev_listener);

  // Test
  enum_gcs_error result = xcom_control_if->join(fake_old_view);
  ASSERT_EQ(GCS_OK, result);

  synode_no message_id;
  message_id.group_id = Gcs_xcom_utils::build_xcom_group_id(*this->group_id);
  message_id.msgno = 3;
  message_id.node = 0;

  Gcs_xcom_nodes *xcom_nodes = new Gcs_xcom_nodes(site_config, nodes);

  bool view_accepted = xcom_control_if->xcom_receive_global_view(
      null_synode, message_id, xcom_nodes, false, null_synode);
  ASSERT_TRUE(view_accepted);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if the manager kept the majority enabled
  ASSERT_EQ(mgr->has_majority(), true);

  // Process a local view.
  // Define nodes and emulate the failure of the second node.
  std::vector<Gcs_member_identifier> unreachable;
  unreachable.push_back(local_member_information_2);

  EXPECT_CALL(mock_ev_listener, on_suspicions(members, unreachable)).Times(1);
  xcom_control_if->xcom_receive_local_view(null_synode, xcom_nodes,
                                           null_synode);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);

  xcom_control_if->remove_event_listener(listener_handle);

  homemade_free_site_def(3, site_config, node_addrs);
  delete xcom_nodes;
  free_node_set(&nodes);
}

void check_view_ok(const Gcs_view &view) {
  ASSERT_EQ(view.get_error_code(), Gcs_view::OK);
}

void check_view_expelled(const Gcs_view &view) {
  ASSERT_EQ(view.get_error_code(), Gcs_view::MEMBER_EXPELLED);
}

TEST_F(XComControlTest, FailedNodeGlobalViewTest) {
  // Setting Expectations and Return Values
  // First the node joins the group.
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(mock_ev_listener, on_suspicions(_, _)).Times(0);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);

  EXPECT_CALL(mock_ev_listener, on_view_changed(_, _))
      .Times(1)
      .WillOnce(WithArgs<0>(Invoke(check_view_ok)));

  Gcs_xcom_uuid uuid_1 = Gcs_xcom_uuid::create_uuid();
  blob blob_1 = {{0, static_cast<char *>(malloc(uuid_1.actual_value.size()))}};
  uuid_1.encode(reinterpret_cast<uchar **>(&blob_1.data.data_val),
                &blob_1.data.data_len);

  Gcs_xcom_uuid uuid_2 = Gcs_xcom_uuid::create_uuid();
  blob blob_2 = {{0, static_cast<char *>(malloc(uuid_2.actual_value.size()))}};
  uuid_2.encode(reinterpret_cast<uchar **>(&blob_2.data.data_val),
                &blob_2.data.data_len);

  node_address node_addrs[2] = {
      {const_cast<char *>("127.0.0.1:12345"), blob_1, {x_1_0, x_1_2}},
      {const_cast<char *>("127.0.0.1:12343"), blob_2, {x_1_0, x_1_2}}};

  site_def *site_config = new_site_def();
  init_site_def(2, node_addrs, site_config);
  site_config->x_proto = static_cast<xcom_proto>(1);
  site_config->nodeno = 0;

  node_set nodes;
  alloc_node_set(&nodes, 2);
  set_node_set(&nodes);
  nodes.node_set_val[0] = 0;

  EXPECT_CALL(proxy, find_site_def(_)).Times(0);

  // Setting fake values
  string address_1("127.0.0.1:12343");
  Gcs_member_identifier local_member_information_1(address_1);

  string address_2("127.0.0.1:12343");
  Gcs_member_identifier local_member_information_2(address_2);

  std::vector<Gcs_member_identifier> members;
  members.push_back(local_member_information_1);
  members.push_back(local_member_information_2);

  Gcs_xcom_view_identifier view_id(111111, 1);
  std::vector<Gcs_member_identifier> leaving;
  std::vector<Gcs_member_identifier> joined;

  Gcs_group_identifier fake_group_id(group_id->get_group_id());

  Gcs_view *fake_old_view =
      new Gcs_view(members, view_id, leaving, joined, fake_group_id);

  // registering the listener
  int listener_handle = xcom_control_if->add_event_listener(mock_ev_listener);

  // Test
  enum_gcs_error result = xcom_control_if->join(fake_old_view);
  ASSERT_EQ(GCS_OK, result);

  synode_no message_id;
  message_id.group_id = Gcs_xcom_utils::build_xcom_group_id(*this->group_id);
  message_id.msgno = 2;
  message_id.node = 0;

  Gcs_xcom_nodes *xcom_nodes = new Gcs_xcom_nodes(site_config, nodes);

  bool view_accepted = xcom_control_if->xcom_receive_global_view(
      null_synode, message_id, xcom_nodes, true, null_synode);
  ASSERT_TRUE(view_accepted);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);

  xcom_control_if->remove_event_listener(listener_handle);

  homemade_free_site_def(2, site_config, node_addrs);
  delete xcom_nodes;
  free_node_set(&nodes);
}

TEST_F(XComControlTest, SuspectMembersRemoval) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(2);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(3);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager and set default parameters values
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(0ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, member_suspect_nodes;
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12346"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12347"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12345", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12349", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if suspicions list is empty
  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

TEST_F(XComControlTest, SuspectMemberFailedRemovalDueToMajorityLoss) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(2);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(3);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager and set default parameters values
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(0ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, member_suspect_nodes;
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12346"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12347"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12345", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Check that the majority is disabled
  ASSERT_EQ(mgr->has_majority(), false);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if the manager has kept the majority disabled
  ASSERT_EQ(mgr->has_majority(), false);

  // Check if suspicions list is empty as they've timed out
  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Add two additional members in order to enable majority
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12349", false));

  // Process same view but with more group members
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  // Run thread to remove failed nodes
  mgr->run_process_suspicions(true);

  // Check if suspicions list is empty
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

TEST_F(XComControlTest, ThreeSuspectNodesRemoval) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(3);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(4);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(2u);
  mgr->set_non_member_expel_timeout_seconds(1ul);
  mgr->set_member_expel_timeout_seconds(5ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, non_member_suspect_nodes,
      member_suspect_nodes;
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12346"));
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12347"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12342", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12343", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12344", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12345", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, non_member_suspect_nodes, true,
                    null_synode);

  // Check that the majority is enabled
  ASSERT_EQ(mgr->has_majority(), true);

  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(non_member_suspect_nodes.size() + member_suspect_nodes.size(),
            number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Verify if duplicate suspicions aren't inserted
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, non_member_suspect_nodes, true,
                    null_synode);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(non_member_suspect_nodes.size() + member_suspect_nodes.size(),
            number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  /*
    Wait the period to and wake the thread to be sure nodes in
    non_member_suspect_nodes were expelled.
  */
  My_xp_util::sleep_seconds(mgr->get_suspicions_processing_period());

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if suspicions list is empty
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(member_suspect_nodes.size(), number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Wait for twice the period to be sure all suspect nodes were expelled
  My_xp_util::sleep_seconds(2 * mgr->get_suspicions_processing_period());

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if suspicions list is empty
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = non_member_suspect_nodes.begin();
       it != non_member_suspect_nodes.end(); ++it)
    delete (*it);
  non_member_suspect_nodes.clear();

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

TEST_F(XComControlTest, FalseThreeSuspectNodesWithdrawn) {
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(2);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(5ul);
  mgr->set_member_expel_timeout_seconds(5ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, non_member_suspect_nodes,
      member_suspect_nodes;
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12346"));
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12347"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, non_member_suspect_nodes, true,
                    null_synode);

  // Check that the majority is disabled
  ASSERT_EQ(mgr->has_majority(), false);

  // Enable majority
  mgr->inform_on_majority(true);

  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(3ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Wait for less than timeout and period in order to remove suspicions
  My_xp_util::sleep_seconds(3);

  std::vector<Gcs_member_identifier *> alive_nodes;
  alive_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12346"));
  alive_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12347"));
  alive_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.remove_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.remove_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.remove_node(Gcs_xcom_node_information("127.0.0.1:12348", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", true));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", true));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", true));

  // Remove suspicions in alive_nodes
  mgr->process_view(null_synode, &xcom_nodes, alive_nodes, no_nodes, no_nodes,
                    no_nodes, true, null_synode);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = non_member_suspect_nodes.begin();
       it != non_member_suspect_nodes.end(); ++it)
    delete (*it);
  non_member_suspect_nodes.clear();

  for (it = alive_nodes.begin(); it != alive_nodes.end(); ++it) delete (*it);
  alive_nodes.clear();

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

TEST_F(XComControlTest, ThreeSuspectNodesRemovalAndWithdrawn) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(2);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(3);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(11u);
  mgr->set_non_member_expel_timeout_seconds(5ul);
  mgr->set_member_expel_timeout_seconds(5ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, non_member_suspect_nodes,
      member_suspect_nodes;
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12346"));
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12347"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, non_member_suspect_nodes, true,
                    null_synode);

  // Check that the majority is disabled
  ASSERT_EQ(mgr->has_majority(), false);

  // Enable majority
  mgr->inform_on_majority(true);

  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(non_member_suspect_nodes.size() + member_suspect_nodes.size(),
            number_suspects);
  MYSQL_GCS_LOG_DEBUG("List has %lu suspects.", number_suspects);

  // Wait for less than timeout and period in order to remove suspicions
  My_xp_util::sleep_seconds(3);

  std::vector<Gcs_member_identifier *> alive_nodes;
  alive_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12346"));
  xcom_nodes.remove_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", true));

  mgr->process_view(null_synode, &xcom_nodes, alive_nodes, no_nodes, no_nodes,
                    no_nodes, true, null_synode);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  /*
    Check if the list still contains the two suspicions since they haven't
    timed out.
  */
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(2ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu number_suspects", number_suspects);

  alive_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.remove_node(Gcs_xcom_node_information("127.0.0.1:12348", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", true));

  mgr->process_view(null_synode, &xcom_nodes, alive_nodes, no_nodes, no_nodes,
                    no_nodes, true, null_synode);

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  // Check if the manager has kept the majority
  ASSERT_EQ(mgr->has_majority(), true);

  /*
    Check if the suspicions list still contains the remaining suspicion which
    still hasn't timed out.
  */
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(1ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu number_suspects", number_suspects);

  // Wait for period to be sure suspect nodes are expelled
  My_xp_util::sleep_seconds(mgr->get_suspicions_processing_period());

  // Run thread to remove failed node
  mgr->run_process_suspicions(true);

  /*
    Check if the suspicions list is empty, since the contained suspicion
    should already have timed out.
  */
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);

  MYSQL_GCS_LOG_TRACE("List has %lu number_suspects", number_suspects);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = non_member_suspect_nodes.begin();
       it != non_member_suspect_nodes.end(); ++it)
    delete (*it);
  non_member_suspect_nodes.clear();

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();

  for (it = alive_nodes.begin(); it != alive_nodes.end(); ++it) delete (*it);
  alive_nodes.clear();
}

TEST_F(XComControlTest, ThreeSuspectNodesRemovalAfterTimeoutReset) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(2);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(3);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(1u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(30ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, non_member_suspect_nodes,
      member_suspect_nodes;
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12346"));
  non_member_suspect_nodes.push_back(
      new Gcs_member_identifier("127.0.0.1:12347"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12348"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, non_member_suspect_nodes, true,
                    null_synode);

  // Check that the majority is disabled
  ASSERT_EQ(mgr->has_majority(), false);

  // Enable majority
  mgr->inform_on_majority(true);

  const Gcs_xcom_nodes &suspicions_list = mgr->get_suspicions();
  long unsigned int number_suspects = suspicions_list.get_size();
  ASSERT_EQ(non_member_suspect_nodes.size() + member_suspect_nodes.size(),
            number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Check if suspicions list contains all the suspect nodes
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(non_member_suspect_nodes.size() + member_suspect_nodes.size(),
            number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Check if the manager kept the majority enabled
  ASSERT_EQ(mgr->has_majority(), true);

  // Update the value of the timeout parameters
  mgr->set_non_member_expel_timeout_seconds(0ul);
  mgr->set_member_expel_timeout_seconds(0ul);

  // Wait for twice the period to be sure all suspect nodes were expelled
  My_xp_util::sleep_seconds(2 * mgr->get_suspicions_processing_period());

  // Check if suspicions list is empty
  number_suspects = suspicions_list.get_size();
  ASSERT_EQ(0ul, number_suspects);
  MYSQL_GCS_LOG_TRACE("List has %lu suspects.", number_suspects);

  // Check if the manager kept the majority enabled
  ASSERT_EQ(mgr->has_majority(), true);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = non_member_suspect_nodes.begin();
       it != non_member_suspect_nodes.end(); ++it)
    delete (*it);
  non_member_suspect_nodes.clear();

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

void *parallel_invocation(void *ptr) {
  InvocationHelper *helper = (InvocationHelper *)ptr;
  helper->invokeMethod();

  return nullptr;
}

TEST_F(XComControlTest, ParallellJoinsTest) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_for_xcom_comms_status_change(_)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);

  InvocationHelper *helper = new InvocationHelper(xcom_control_if, JJ);

  My_xp_thread_impl thread;
  thread.create(PSI_NOT_INSTRUMENTED, nullptr, parallel_invocation,
                (void *)helper);

  helper->invokeMethod();

  thread.join(nullptr);

  ASSERT_EQ(helper->count_success, 1);
  ASSERT_EQ(helper->count_fail, 1);

  // release memory
  delete helper;
}

TEST_F(XComControlTest, ParallelLeavesTest) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(2);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);

  InvocationHelper *helper = new InvocationHelper(xcom_control_if, LL);

  My_xp_thread_impl thread;
  thread.create(PSI_NOT_INSTRUMENTED, nullptr, parallel_invocation,
                (void *)helper);

  helper->invokeMethod();

  thread.join(nullptr);

  ASSERT_EQ(helper->count_success, 1);
  ASSERT_EQ(helper->count_fail, 1);

  // release memory
  delete helper;
}

TEST_F(XComControlTest, ParallelLeaveAndDelayedJoinTest) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(2);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(2);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(2);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(2);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(2);
  EXPECT_CALL(proxy, xcom_init(_)).Times(2);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);

  InvocationHelper *helper = new InvocationHelper(xcom_control_if, LJ);

  My_xp_thread_impl thread;
  thread.create(PSI_NOT_INSTRUMENTED, nullptr, parallel_invocation,
                (void *)helper);

  helper->invokeMethod();

  thread.join(nullptr);

  ASSERT_EQ(helper->count_success, 2);

  // release memory
  delete helper;
}

TEST_F(XComControlTest, ParallelJoinAndDelayedLeaveTest) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(2);

  InvocationHelper *helper = new InvocationHelper(xcom_control_if, JL);

  My_xp_thread_impl thread;
  thread.create(PSI_NOT_INSTRUMENTED, nullptr, parallel_invocation,
                (void *)helper);

  helper->invokeMethod();

  thread.join(nullptr);

  ASSERT_EQ(helper->count_success, 2);

  // release memory
  delete helper;
}

TEST_F(XComControlTest, NodeTooFarMessage) {
  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_wait_ready()).Times(1);
  EXPECT_CALL(proxy, xcom_init(_)).Times(1);
  EXPECT_CALL(proxy, xcom_exit()).Times(0);
  EXPECT_CALL(proxy, delete_node_address(_, _)).Times(2);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _)).Times(1);

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);

  // Get Gcs_suspicions_manager and set default parameters values
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  Gcs_xcom_nodes xcom_nodes;
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(60ul);

  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes, member_suspect_nodes;
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12346"));
  member_suspect_nodes.push_back(new Gcs_member_identifier("127.0.0.1:12347"));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12345", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12348", false));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12349", false));

  std::vector<Gcs_xcom_node_information>::iterator node_it;
  std::vector<Gcs_xcom_node_information> nodes = xcom_nodes.get_nodes();
  for (node_it = nodes.begin(); node_it != nodes.end(); ++node_it) {
    ASSERT_FALSE(node_it->has_lost_messages());
    ASSERT_TRUE(synode_eq(node_it->get_max_synode(), null_synode));
  }

  synode_no suspicion_synode = {1, 100, 0};
  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, suspicion_synode);

  // Run thread and check that messages have not yet been lost since nothing
  // has yet been removed from the cache.
  mgr->run_process_suspicions(true);

  std::vector<Gcs_member_identifier *>::iterator it;
  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it) {
    ASSERT_FALSE(mgr->get_suspicions().get_node(*(*it))->has_lost_messages());
    ASSERT_TRUE(
        synode_eq(mgr->get_suspicions().get_node(*(*it))->get_max_synode(),
                  suspicion_synode));
  }

  synode_no last_removed = {1, 200, 0};
  // Do it again with a higher last_removed_from_cache value
  mgr->update_last_removed(last_removed);
  mgr->run_process_suspicions(true);

  Gcs_xcom_node_information *node = nullptr;
  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it) {
    node = const_cast<Gcs_xcom_node_information *>(
        mgr->get_suspicions().get_node(*(*it)));
    ASSERT_TRUE(node->has_lost_messages());
    ASSERT_TRUE(synode_eq(node->get_max_synode(), suspicion_synode));
  }

  // Clear current suspicions...
  mgr->clear_suspicions();
  // ...and add them again to see if the message related vars are cleared.
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, last_removed);

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it) {
    node = const_cast<Gcs_xcom_node_information *>(
        mgr->get_suspicions().get_node(*(*it)));
    ASSERT_FALSE(node->has_lost_messages());
    ASSERT_TRUE(synode_eq(node->get_max_synode(), last_removed));
  }

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);

  for (it = member_suspect_nodes.begin(); it != member_suspect_nodes.end();
       ++it)
    delete (*it);
  member_suspect_nodes.clear();
}

/**
 This test verifies that local views are never delivered to the application
 after a node is expelled from the group.
*/
TEST_F(XComControlTest, LocalViewAfterExpel) {
  // Two calls: expel and leave
  EXPECT_CALL(mock_ev_listener, on_view_changed(_, _)).Times(2);
  /*
    on_suspicion is never called because the local view is ignored
    after the node is expelled
  */
  EXPECT_CALL(mock_ev_listener, on_suspicions(_, _)).Times(0);
  xcom_control_if->add_event_listener(mock_ev_listener);

  // Create fake view with two members
  string member_id_1 = xcom_node_address->get_member_address();
  Gcs_member_identifier local_member_information_1(member_id_1);
  string member_id_2("127.0.0.1:12343");
  Gcs_member_identifier local_member_information_2(member_id_2);
  std::vector<Gcs_member_identifier> members;
  members.push_back(local_member_information_1);
  members.push_back(local_member_information_2);
  Gcs_xcom_view_identifier view_id(111111, 1);
  std::vector<Gcs_member_identifier> leaving;
  std::vector<Gcs_member_identifier> joined;
  Gcs_group_identifier fake_group_id(group_id->get_group_id());
  Gcs_view *fake_old_view =
      new Gcs_view(members, view_id, leaving, joined, fake_group_id);

  // Join the group with fake view
  enum_gcs_error result = xcom_control_if->join(fake_old_view);
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Create view without the local node to simulate expel
  synode_no message_id;
  message_id.group_id = Gcs_xcom_utils::build_xcom_group_id(*this->group_id);
  message_id.msgno = 4;
  message_id.node = 0;

  Gcs_xcom_uuid uuid = Gcs_xcom_uuid::create_uuid();
  blob blob = {{0, static_cast<char *>(malloc(uuid.actual_value.size()))}};
  uuid.encode(reinterpret_cast<uchar **>(&blob.data.data_val),
              &blob.data.data_len);

  node_address node_addrs[1] = {
      {const_cast<char *>(member_id_2.c_str()), blob, {x_1_0, x_1_2}}};

  site_def *site_config = new_site_def();
  init_site_def(1, node_addrs, site_config);
  site_config->x_proto = static_cast<xcom_proto>(1);
  site_config->nodeno = 0;

  node_set nodes;
  alloc_node_set(&nodes, 1);
  set_node_set(&nodes);

  Gcs_xcom_nodes *xcom_nodes = new Gcs_xcom_nodes(site_config, nodes);

  // Install expel view and verify that it succeeded
  bool expel_view_accepted = xcom_control_if->xcom_receive_global_view(
      null_synode, message_id, xcom_nodes, false, null_synode);
  ASSERT_TRUE(expel_view_accepted);
  Gcs_view *current_view = mock_vce->get_current_view();
  check_view_expelled(*current_view);

  // Try to Install local view and verify that it fails
  bool local_view_accepted = xcom_control_if->xcom_receive_local_view(
      null_synode, xcom_nodes, null_synode);
  ASSERT_FALSE(local_view_accepted);

  // Leave and cleanup
  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);

  homemade_free_site_def(1, site_config, node_addrs);
  free_node_set(&nodes);
  delete xcom_nodes;
  delete current_view;
  mock_vce->set_current_view(nullptr);
}

/* This test validates that GCS, given the right sequence of views, does not
   expel the entire group.

   The test mimics the following scenario.

   Members:
     {me, suspect_1, suspect_2}

   Views, composed of a sequence of alive members A, and a set of failed
   members F:
      V1: A=(me, suspect_1) F={suspect_2}
      V2: A=(me, suspect_2) F={suspect_1}
      V3: A=(suspect_1, suspect_2) F={me}

   The test mimics the delivery of V1, V2, and V3 on member `me` before any
   expel takes effect, i.e. any expelled member is actually removed from the
   view. The test verifies that `me` does the following.

   Upon receiving V1, expels `suspect_2`.

   Upon receiving V2, does NOT expel `suspect_1`. The only reason to expel
   `suspect_1` is because the view states that there is a majority of members
   alive, `(me, suspect_2)` vs. `{suspect_1}`. But we have already ordered the
   expel of `suspect_2`, so it should be counted as if it is failed. In this
   case, there is no majority, `(me)` vs. `{suspect_1, suspect_2}`, so
   `suspect_1` should not be expelled.

   Upon receiving V3, expels `me`, due to the `force_remove` flag in the
   suspicion manager.
*/
TEST_F(XComControlTest, DoNotDisbandEntireGroup) {
  auto me = std::make_unique<Gcs_member_identifier>("127.0.0.1:12345");
  auto suspect_1 = std::make_unique<Gcs_member_identifier>("127.0.0.1:12346");
  auto suspect_2 = std::make_unique<Gcs_member_identifier>("127.0.0.1:12347");

  Gcs_xcom_nodes xcom_nodes;
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12345", true));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12346", true));
  xcom_nodes.add_node(Gcs_xcom_node_information("127.0.0.1:12347", true));

  auto suspect_is_me = [suspect = me.get()](node_list *nl) -> bool {
    EXPECT_EQ(nl->node_list_len, 1);
    EXPECT_EQ(std::string{nl->node_list_val[0].address},
              suspect->get_member_id());
    return true;
  };
  auto suspect_is_2 = [suspect = suspect_2.get()](node_list *nl) -> bool {
    EXPECT_EQ(nl->node_list_len, 1);
    EXPECT_EQ(std::string{nl->node_list_val[0].address},
              suspect->get_member_id());
    return true;
  };

  EXPECT_CALL(proxy, xcom_input_connect(_, _)).Times(1);
  EXPECT_CALL(proxy, test_xcom_tcp_connection(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_boot(_, _)).Times(1);
  EXPECT_CALL(proxy, xcom_client_remove_node(_, _))
      .Times(3)  // twice for expulsion, once for leave
      .WillOnce(WithArgs<0>(Invoke(suspect_is_2)))
      .WillOnce(WithArgs<0>(Invoke(suspect_is_me)))
      .WillOnce(WithArgs<0>(Invoke(suspect_is_me)));
  EXPECT_CALL(proxy, delete_node_address(_, _))
      .Times(4);  // once for boot, twice for expulsion, once for leave

  enum_gcs_error result = xcom_control_if->join(create_fake_view());
  ASSERT_EQ(GCS_OK, result);
  ASSERT_TRUE(xcom_control_if->is_xcom_running());

  // Get Gcs_suspicions_manager and set default parameters values
  Gcs_suspicions_manager *mgr = xcom_control_if->get_suspicions_manager();
  mgr->set_suspicions_processing_period(15u);
  mgr->set_non_member_expel_timeout_seconds(60ul);
  mgr->set_member_expel_timeout_seconds(0ul);

  /* Test expulsion of suspect_2.
     We should issue the expel, as specified in the EXPECT_CALL(proxy,
     xcom_client_remove_node(_, _)) above. */
  // Build vector with suspect nodes
  std::vector<Gcs_member_identifier *> no_nodes;
  std::vector<Gcs_member_identifier *> member_suspect_nodes{suspect_2.get()};

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Run thread to remove failed node suspect_2.
  mgr->run_process_suspicions(true);

  // Check that the manager has majority
  ASSERT_EQ(mgr->has_majority(), true);

  /* Test expulsion of suspect_1.
     We should NOT issue the expel, as specified in the EXPECT_CALL(proxy,
     xcom_client_remove_node(_, _)) above. */
  // Build vector with suspect nodes
  member_suspect_nodes = std::vector<Gcs_member_identifier *>{suspect_1.get()};

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Run thread to try to remove failed node me.
  mgr->run_process_suspicions(true);

  // Check that the manager does not have majority
  ASSERT_EQ(mgr->has_majority(), false);

  /* Test expulsion of me.
     We should issue the expel, as specified in the EXPECT_CALL(proxy,
     xcom_client_remove_node(_, _)) above. */
  // Build vector with suspect nodes
  member_suspect_nodes = std::vector<Gcs_member_identifier *>{me.get()};

  // Insert suspicions into manager
  mgr->process_view(null_synode, &xcom_nodes, no_nodes, no_nodes,
                    member_suspect_nodes, no_nodes, true, null_synode);

  // Run thread to try to remove failed node me.
  mgr->run_process_suspicions(true);

  // Check that the manager does not have majority
  ASSERT_EQ(mgr->has_majority(), false);

  result = xcom_control_if->leave();
  ASSERT_EQ(GCS_OK, result);
  ASSERT_FALSE(xcom_control_if->is_xcom_running());
}

}  // namespace gcs_xcom_control_unittest
