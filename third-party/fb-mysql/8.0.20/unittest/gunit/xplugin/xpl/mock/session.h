/*
 * Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.
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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_SESSION_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_SESSION_H_

#include <memory>
#include <string>
#include <vector>

#include "plugin/x/ngs/include/ngs/client.h"
#include "plugin/x/ngs/include/ngs/scheduler.h"
#include "plugin/x/src/account_verification_handler.h"
#include "plugin/x/src/interface/account_verification.h"
#include "plugin/x/src/interface/client.h"
#include "plugin/x/src/interface/document_id_aggregator.h"
#include "plugin/x/src/interface/protocol_encoder.h"
#include "plugin/x/src/interface/server.h"
#include "plugin/x/src/interface/sql_session.h"
#include "plugin/x/src/interface/ssl_context.h"
#include "plugin/x/src/interface/vio.h"
#include "plugin/x/src/sql_data_context.h"
#include "plugin/x/src/xpl_resultset.h"
#include "plugin/x/src/xpl_session.h"

namespace xpl {
namespace test {

class Mock_vio : public iface::Vio {
 public:
  MOCK_METHOD2(read, ssize_t(uchar *buffer, ssize_t bytes_to_send));
  MOCK_METHOD2(write, ssize_t(const uchar *buffer, ssize_t bytes_to_send));
  MOCK_METHOD2(set_timeout_in_ms,
               void(const iface::Vio::Direction, const uint64_t timeout));
  MOCK_METHOD1(set_state, void(PSI_socket_state state));
  MOCK_METHOD0(set_thread_owner, void());
  MOCK_METHOD0(get_fd, my_socket());
  MOCK_CONST_METHOD0(get_type, Connection_type());
  MOCK_METHOD2(peer_addr,
               sockaddr_storage *(std::string *address, uint16_t *port));
  MOCK_METHOD0(shutdown, int32_t());
  MOCK_METHOD0(delete_vio, void());
  MOCK_METHOD0(get_vio, ::Vio *());
  MOCK_METHOD0(get_mysql_socket, MYSQL_SOCKET &());
};

class Mock_ssl_context : public iface::Ssl_context {
 public:
  MOCK_METHOD8(setup, bool(const char *tls_version, const char *ssl_key,
                           const char *ssl_ca, const char *ssl_capath,
                           const char *ssl_cert, const char *ssl_cipher,
                           const char *ssl_crl, const char *ssl_crlpath));
  MOCK_METHOD2(activate_tls,
               bool(iface::Vio *conn, const int32_t handshake_timeout));
  MOCK_METHOD0(options, iface::Ssl_context_options &());
  MOCK_METHOD0(has_ssl, bool());
  MOCK_METHOD0(reset, void());
};

class Mock_authentication_container
    : public xpl::iface::Authentication_container {
 public:
  std::unique_ptr<xpl::iface::Authentication> get_auth_handler(
      const std::string &name, xpl::iface::Session *session) override {
    return std::unique_ptr<xpl::iface::Authentication>{
        get_auth_handler_raw(name, session)};
  }

  MOCK_METHOD2(get_auth_handler_raw,
               xpl::iface::Authentication *(const std::string &name,
                                            xpl::iface::Session *session));
  MOCK_METHOD1(get_authentication_mechanisms,
               std::vector<std::string>(xpl::iface::Client *client));
};

class Mock_scheduler_dynamic : public ngs::Scheduler_dynamic {
 public:
  Mock_scheduler_dynamic() : ngs::Scheduler_dynamic("", PSI_NOT_INSTRUMENTED) {}

  MOCK_METHOD0(launch, void());
  MOCK_METHOD0(stop, void());
  MOCK_METHOD0(thread_init, bool());
  MOCK_METHOD0(thread_end, void());
  MOCK_METHOD1(set_num_workers, unsigned int(unsigned int n));
};

class Mock_server : public iface::Server {
 public:
  MOCK_METHOD0(get_authentications, xpl::iface::Authentication_container &());

  MOCK_METHOD0(reset, bool());
  MOCK_METHOD0(start_failed, void());
  MOCK_METHOD0(prepare, bool());
  MOCK_METHOD0(start_tasks, void());
  MOCK_METHOD1(stop, void(const bool));
  MOCK_METHOD0(delayed_start_tasks, void());

  MOCK_CONST_METHOD0(get_config,
                     std::shared_ptr<ngs::Protocol_global_config>());
  MOCK_METHOD0(is_running, bool());
  MOCK_CONST_METHOD0(get_worker_scheduler,
                     std::shared_ptr<ngs::Scheduler_dynamic>());
  MOCK_CONST_METHOD0(ssl_context, iface::Ssl_context *());
  MOCK_METHOD1(on_client_closed, void(const iface::Client &));
  MOCK_METHOD3(create_session,
               std::shared_ptr<iface::Session>(iface::Client *,
                                               iface::Protocol_encoder *,
                                               const int));

  MOCK_METHOD0(get_client_list, ngs::Client_list &());
  MOCK_METHOD1(get_client, std::shared_ptr<xpl::iface::Client>(const THD *));
  MOCK_METHOD2(kill_client,
               ngs::Error_code(const uint64_t, xpl::iface::Session *));

  MOCK_METHOD0(get_client_exit_mutex, xpl::Mutex &());
  MOCK_METHOD0(restart_client_supervision_timer, void());

  MOCK_METHOD0(reset_globals, bool());
  MOCK_CONST_METHOD0(get_document_id_generator,
                     iface::Document_id_generator &());
};

class Mock_authentication_interface : public iface::Authentication {
 public:
  MOCK_METHOD3(handle_start, Response(const std::string &, const std::string &,
                                      const std::string &));

  MOCK_METHOD1(handle_continue, Response(const std::string &));

  MOCK_CONST_METHOD3(authenticate_account,
                     ngs::Error_code(const std::string &, const std::string &,
                                     const std::string &));

  MOCK_CONST_METHOD0(get_tried_user_name, std::string());
  MOCK_CONST_METHOD0(get_authentication_info, iface::Authentication_info());
};

class Mock_account_verification : public iface::Account_verification {
 public:
  MOCK_CONST_METHOD0(get_salt, const std::string &());
  MOCK_CONST_METHOD4(verify_authentication_string,
                     bool(const std::string &, const std::string &,
                          const std::string &, const std::string &));
};

class Mock_sql_data_context : public iface::Sql_session {
 public:
  MOCK_METHOD1(set_connection_type,
               ngs::Error_code(const xpl::Connection_type));
  MOCK_METHOD1(execute_kill_sql_session, ngs::Error_code(uint64_t));
  MOCK_CONST_METHOD0(is_killed, bool());
  MOCK_CONST_METHOD0(password_expired, bool());
  MOCK_CONST_METHOD0(get_authenticated_user_name, std::string());
  MOCK_CONST_METHOD0(get_authenticated_user_host, std::string());
  MOCK_CONST_METHOD0(has_authenticated_user_a_super_priv, bool());
  MOCK_CONST_METHOD0(mysql_session_id, uint64_t());
  MOCK_METHOD7(authenticate,
               ngs::Error_code(const char *, const char *, const char *,
                               const char *, const std::string &,
                               const iface::Authentication &, bool));
  MOCK_METHOD3(execute,
               ngs::Error_code(const char *, std::size_t, iface::Resultset *));
  MOCK_METHOD3(execute_sql,
               ngs::Error_code(const char *, std::size_t, iface::Resultset *));
  MOCK_METHOD3(fetch_cursor,
               ngs::Error_code(const std::uint32_t, const std::uint32_t,
                               iface::Resultset *));
  MOCK_METHOD3(prepare_prep_stmt,
               ngs::Error_code(const char *, std::size_t, iface::Resultset *));
  MOCK_METHOD2(deallocate_prep_stmt,
               ngs::Error_code(const uint32_t, iface::Resultset *));
  MOCK_METHOD5(execute_prep_stmt,
               ngs::Error_code(const uint32_t, const bool, const PS_PARAM *,
                               std::size_t, iface::Resultset *));
  MOCK_METHOD0(attach, ngs::Error_code());
  MOCK_METHOD0(detach, ngs::Error_code());
  MOCK_METHOD0(reset, ngs::Error_code());
  MOCK_METHOD1(is_sql_mode_set, bool(const std::string &));
};

class Mock_protocol_encoder : public iface::Protocol_encoder {
 public:
  MOCK_METHOD1(send_result, bool(const ngs::Error_code &));
  MOCK_METHOD0(send_ok, bool());
  MOCK_METHOD1(send_ok, bool(const std::string &));
  MOCK_METHOD1(send_init_error, bool(const ngs::Error_code &));
  MOCK_METHOD4(send_notice,
               bool(const iface::Frame_type, const iface::Frame_scope,
                    const std::string &, const bool));
  MOCK_METHOD1(send_rows_affected, void(uint64_t value));
  MOCK_METHOD2(send_local_warning, void(const std::string &, bool));
  MOCK_METHOD1(send_auth_ok, void(const std::string &));
  MOCK_METHOD1(send_auth_continue, void(const std::string &));
  MOCK_METHOD0(send_nothing, bool());
  MOCK_METHOD0(send_exec_ok, bool());
  MOCK_METHOD0(send_result_fetch_done, bool());
  MOCK_METHOD0(send_result_fetch_suspended, bool());
  MOCK_METHOD0(send_result_fetch_done_more_results, bool());
  MOCK_METHOD0(send_result_fetch_done_more_out_params, bool());

  MOCK_METHOD1(send_column_metadata,
               bool(const ngs::Encode_column_info *column_info));
  MOCK_METHOD0(row_builder, protocol::XRow_encoder *());
  MOCK_METHOD0(raw_encoder, protocol::XMessage_encoder *());
  MOCK_METHOD0(start_row, void());
  MOCK_METHOD0(abort_row, void());
  MOCK_METHOD0(send_row, bool());
  MOCK_METHOD3(send_protobuf_message,
               bool(const uint8_t, const ngs::Message &, bool));
  MOCK_METHOD1(on_error, void(int error));
  MOCK_METHOD0(get_protocol_monitor, iface::Protocol_monitor &());
  MOCK_METHOD0(get_metadata_builder, ngs::Metadata_builder *());

  MOCK_METHOD1(enqueue_empty_message, uint8_t *(const uint8_t message_id));

  MOCK_METHOD0(get_flusher, iface::Protocol_flusher *());

  MOCK_METHOD2(send_error,
               bool(const ngs::Error_code &error_code, const bool init_error));

  MOCK_METHOD1(send_notice_rows_affected, void(const uint64_t value));
  MOCK_METHOD1(send_notice_client_id, void(const uint64_t id));
  MOCK_METHOD1(send_notice_last_insert_id, void(const uint64_t id));
  MOCK_METHOD0(send_notice_account_expired, void());
  MOCK_METHOD1(send_notice_generated_document_ids,
               void(const std::vector<std::string> &ids));
  MOCK_METHOD1(send_notice_txt_message, void(const std::string &message));
  MOCK_METHOD1(set_flusher_raw,
               iface::Protocol_flusher *(iface::Protocol_flusher *flusher));

  std::unique_ptr<iface::Protocol_flusher> set_flusher(
      std::unique_ptr<iface::Protocol_flusher> flusher) override {
    std::unique_ptr<iface::Protocol_flusher> result{
        set_flusher_raw(flusher.get())};
    return result;
  }
};

class Mock_session : public iface::Session {
 public:
  MOCK_CONST_METHOD0(session_id, Session_id());
  MOCK_METHOD0(init, ngs::Error_code());
  MOCK_METHOD1(on_close, void(const bool));
  MOCK_METHOD0(on_kill, void());
  MOCK_METHOD1(on_auth_success, void(const iface::Authentication::Response &));
  MOCK_METHOD1(on_auth_failure, void(const iface::Authentication::Response &));
  MOCK_METHOD0(on_reset, void());
  MOCK_METHOD1(handle_message, bool(const ngs::Message_request &));
  MOCK_CONST_METHOD0(state, State());
  MOCK_CONST_METHOD0(state_before_close, State());
  MOCK_METHOD0(get_status_variables, ngs::Session_status_variables &());
  MOCK_METHOD0(client, iface::Client &());
  MOCK_CONST_METHOD0(client, const iface::Client &());
  MOCK_CONST_METHOD1(can_see_user, bool(const std::string &));

  MOCK_METHOD0(mark_as_tls_session, void());
  MOCK_CONST_METHOD0(get_thd, THD *());
  MOCK_METHOD0(data_context, iface::Sql_session &());
  MOCK_METHOD0(proto, iface::Protocol_encoder &());
  MOCK_METHOD1(set_proto, void(iface::Protocol_encoder *));
  MOCK_METHOD0(get_notice_configuration, iface::Notice_configuration &());
  MOCK_METHOD0(get_notice_output_queue, iface::Notice_output_queue &());
  MOCK_CONST_METHOD2(get_prepared_statement_id,
                     bool(const uint32_t, uint32_t *));
  MOCK_METHOD1(update_status, void(ngs::Common_status_variables::Variable
                                       ngs::Common_status_variables::*));
  MOCK_METHOD0(get_document_id_aggregator, iface::Document_id_aggregator &());
};

class Mock_notice_configuration : public iface::Notice_configuration {
 public:
  MOCK_CONST_METHOD2(get_notice_type_by_name,
                     bool(const std::string &name,
                          ngs::Notice_type *out_notice_type));
  MOCK_CONST_METHOD2(get_name_by_notice_type,
                     bool(const ngs::Notice_type notice_type,
                          std::string *out_name));
  MOCK_CONST_METHOD1(is_notice_enabled,
                     bool(const ngs::Notice_type notice_type));
  MOCK_METHOD2(set_notice, void(const ngs::Notice_type notice_type,
                                const bool should_be_enabled));
  MOCK_CONST_METHOD0(is_any_dispatchable_notice_enabled, bool());
};

class Mock_protocol_monitor : public iface::Protocol_monitor {
 public:
  MOCK_METHOD0(on_notice_warning_send, void());
  MOCK_METHOD0(on_notice_other_send, void());
  MOCK_METHOD0(on_notice_global_send, void());
  MOCK_METHOD0(on_fatal_error_send, void());
  MOCK_METHOD0(on_init_error_send, void());
  MOCK_METHOD0(on_row_send, void());
  MOCK_METHOD1(on_send, void(const uint32_t));
  MOCK_METHOD1(on_send_compressed, void(const uint32_t bytes_transferred));
  MOCK_METHOD1(on_send_before_compression,
               void(const uint32_t bytes_transferred));
  MOCK_METHOD1(on_receive, void(const uint32_t));
  MOCK_METHOD1(on_receive_compressed, void(const uint32_t bytes_transferred));
  MOCK_METHOD1(on_receive_after_decompression,
               void(const uint32_t bytes_transferred));
  MOCK_METHOD0(on_error_send, void());
  MOCK_METHOD0(on_error_unknown_msg_type, void());
  MOCK_METHOD1(on_messages_sent, void(const uint32_t messages));
};

class Mock_id_generator : public iface::Document_id_generator {
 public:
  MOCK_METHOD1(generate,
               std::string(const iface::Document_id_generator::Variables &));
};

class Mock_wait_for_io : public iface::Waiting_for_io {
 public:
  MOCK_METHOD0(has_to_report_idle_waiting, bool());
  MOCK_METHOD0(on_idle_or_before_read, void());
};

class Mock_notice_output_queue : public iface::Notice_output_queue {
 public:
  MOCK_METHOD2(emplace, void(const ngs::Notice_type type,
                             const Buffer_shared &binary_notice));
  MOCK_METHOD0(get_callbacks_waiting_for_io, iface::Waiting_for_io *());
  MOCK_METHOD1(encode_queued_items,
               void(const bool last_notice_does_force_fulsh));

  MOCK_METHOD1(set_encoder, void(iface::Protocol_encoder *encoder));
};

class Mock_id_aggregator : public iface::Document_id_aggregator {
 public:
  MOCK_METHOD0(generate_id, std::string());
  MOCK_METHOD1(generate_id, std::string(const Variables &));
  MOCK_METHOD0(clear_ids, void());
  MOCK_CONST_METHOD0(get_ids, const Document_id_list &());
  MOCK_METHOD1(configue, ngs::Error_code(iface::Sql_session *));
  MOCK_METHOD1(set_id_retention, void(const bool));
};

class Mock_message_dispatcher
    : public ngs::Message_decoder::Message_dispatcher_interface {
 public:
  MOCK_METHOD1(handle, void(ngs::Message_request *));
};

class Mock_ngs_client : public ngs::Client {
 public:
  using ngs::Client::Client;
  using ngs::Client::read_one_message_and_dispatch;
  using ngs::Client::set_encoder;

  MOCK_METHOD0(resolve_hostname, std::string());
  MOCK_CONST_METHOD0(is_interactive, bool());
  MOCK_METHOD1(set_is_interactive, void(const bool));
  MOCK_METHOD0(kill, void());
  MOCK_METHOD1(handle_message, void(ngs::Message_request *));
};

class Mock_client : public iface::Client {
 public:
  MOCK_METHOD0(get_session_exit_mutex, Mutex &());

  MOCK_CONST_METHOD0(client_id, const char *());

  MOCK_METHOD0(kill, void());
  MOCK_CONST_METHOD0(client_address, const char *());
  MOCK_CONST_METHOD0(client_hostname, const char *());
  MOCK_CONST_METHOD0(client_hostname_or_address, const char *());
  MOCK_CONST_METHOD0(connection, iface::Vio &());
  MOCK_CONST_METHOD0(server, iface::Server &());
  MOCK_CONST_METHOD0(protocol, iface::Protocol_encoder &());

  MOCK_CONST_METHOD0(client_id_num, Client_id());
  MOCK_CONST_METHOD0(client_port, int());

  MOCK_CONST_METHOD0(get_accept_time, chrono::Time_point());
  MOCK_CONST_METHOD0(get_state, Client::State());

  MOCK_METHOD0(session, iface::Session *());
  MOCK_CONST_METHOD0(session_shared_ptr, std::shared_ptr<iface::Session>());
  MOCK_CONST_METHOD0(supports_expired_passwords, bool());

  MOCK_CONST_METHOD0(is_interactive, bool());
  MOCK_METHOD1(set_is_interactive, void(bool));

  MOCK_METHOD1(set_wait_timeout, void(const unsigned int));
  MOCK_METHOD1(set_interactive_timeout, void(const unsigned int));
  MOCK_METHOD1(set_read_timeout, void(const unsigned int));
  MOCK_METHOD1(set_write_timeout, void(const unsigned int));

  MOCK_METHOD4(configure_compression_opts,
               void(const ngs::Compression_algorithm algo,
                    const int64_t max_msg, const bool combine,
                    const Optional_value<int64_t> &level));
  MOCK_METHOD1(handle_message, void(ngs::Message_request *));

  MOCK_METHOD1(get_capabilities,
               void(const Mysqlx::Connection::CapabilitiesGet &));
  MOCK_METHOD1(set_capabilities,
               void(const Mysqlx::Connection::CapabilitiesSet &));

 public:
  MOCK_METHOD1(on_session_reset_void, bool(iface::Session &));
  MOCK_METHOD1(on_session_close_void, bool(iface::Session &));
  MOCK_METHOD1(on_session_auth_success_void, bool(iface::Session &));
  MOCK_METHOD1(on_connection_close_void, bool(iface::Session &));

  MOCK_METHOD0(disconnect_and_trigger_close_void, bool());
  MOCK_CONST_METHOD1(is_handler_thd, bool(const THD *));
  MOCK_CONST_METHOD2(get_prepared_statement_id,
                     bool(const uint32_t, uint32_t *));

  MOCK_METHOD0(activate_tls_void, bool());
  MOCK_METHOD0(on_auth_timeout_void, bool());
  MOCK_METHOD0(on_server_shutdown_void, bool());
  MOCK_METHOD0(run_void, bool());
  MOCK_METHOD0(reset_accept_time_void, bool());

  void on_session_reset(iface::Session *arg) override {
    on_session_reset_void(*arg);
  }

  void on_session_close(iface::Session *arg) override {
    on_session_close_void(*arg);
  }

  void on_session_auth_success(iface::Session *arg) override {
    on_session_auth_success_void(*arg);
  }

  void disconnect_and_trigger_close() override {
    disconnect_and_trigger_close_void();
  }

  void activate_tls() override { activate_tls_void(); }

  void on_auth_timeout() override { on_auth_timeout_void(); }

  void on_server_shutdown() override { on_server_shutdown_void(); }

  void run() override { run_void(); }

  void reset_accept_time() override { reset_accept_time_void(); }
};

class Mock_account_verification_handler : public Account_verification_handler {
 public:
  explicit Mock_account_verification_handler(Session *session)
      : Account_verification_handler(session) {}

  MOCK_CONST_METHOD3(authenticate,
                     ngs::Error_code(const iface::Authentication &,
                                     iface::Authentication_info *,
                                     const std::string &));
  MOCK_CONST_METHOD1(get_account_verificator,
                     const iface::Account_verification *(
                         const iface::Account_verification::Account_type));
};

}  // namespace test
}  // namespace xpl

#endif  //  UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_SESSION_H_
