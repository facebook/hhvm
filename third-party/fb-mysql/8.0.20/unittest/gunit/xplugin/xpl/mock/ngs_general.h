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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_NGS_GENERAL_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_NGS_GENERAL_H_

#include <memory>
#include <string>
#include <vector>

#include "plugin/x/src/interface/listener_factory.h"
#include "plugin/x/src/interface/operations_factory.h"
#include "plugin/x/src/interface/socket.h"
#include "plugin/x/src/interface/socket_events.h"
#include "plugin/x/src/interface/ssl_context_options.h"
#include "plugin/x/src/ssl_session_options.h"

namespace xpl {
namespace test {

class Mock_options_session : public iface::Ssl_session_options {
 public:
  MOCK_CONST_METHOD0(active_tls, bool());

  MOCK_CONST_METHOD0(ssl_cipher, std::string());
  MOCK_CONST_METHOD0(ssl_version, std::string());
  MOCK_CONST_METHOD0(ssl_cipher_list, std::vector<std::string>());

  MOCK_CONST_METHOD0(ssl_verify_depth, int64_t());
  MOCK_CONST_METHOD0(ssl_verify_mode, int64_t());
  MOCK_CONST_METHOD0(ssl_sessions_reused, int64_t());

  MOCK_CONST_METHOD0(ssl_get_verify_result_and_cert, int64_t());
  MOCK_CONST_METHOD0(ssl_get_peer_certificate_issuer, std::string());
  MOCK_CONST_METHOD0(ssl_get_peer_certificate_subject, std::string());
};

class Mock_options_context : public iface::Ssl_context_options {
 public:
  MOCK_METHOD0(ssl_ctx_verify_depth, int64_t());
  MOCK_METHOD0(ssl_ctx_verify_mode, int64_t());

  MOCK_METHOD0(ssl_server_not_after, std::string());
  MOCK_METHOD0(ssl_server_not_before, std::string());

  MOCK_METHOD0(ssl_sess_accept_good, int64_t());
  MOCK_METHOD0(ssl_sess_accept, int64_t());
  MOCK_METHOD0(ssl_accept_renegotiates, int64_t());

  MOCK_METHOD0(ssl_session_cache_mode, std::string());

  MOCK_METHOD0(ssl_session_cache_hits, int64_t());
  MOCK_METHOD0(ssl_session_cache_misses, int64_t());
  MOCK_METHOD0(ssl_session_cache_overflows, int64_t());
  MOCK_METHOD0(ssl_session_cache_size, int64_t());
  MOCK_METHOD0(ssl_session_cache_timeouts, int64_t());
  MOCK_METHOD0(ssl_used_session_cache_entries, int64_t());
};

class Mock_socket : public iface::Socket {
 public:
  MOCK_METHOD2(bind, int(const struct sockaddr *, socklen_t));
  MOCK_METHOD3(accept,
               MYSQL_SOCKET(PSI_socket_key, struct sockaddr *, socklen_t *));
  MOCK_METHOD1(listen, int(int));

  MOCK_METHOD0(close, void());

  MOCK_METHOD0(get_socket_mysql, MYSQL_SOCKET());
  MOCK_METHOD0(get_socket_fd, my_socket());

  MOCK_METHOD4(set_socket_opt, int(int, int, const SOCKBUF_T *, socklen_t));
  MOCK_METHOD0(set_socket_thread_owner, void());
};

class Mock_system : public iface::System {
 public:
  MOCK_METHOD1(unlink, int32_t(const char *));
  MOCK_METHOD2(kill, int32_t(int32_t, int32_t));

  MOCK_METHOD0(get_ppid, int32_t());
  MOCK_METHOD0(get_pid, int32_t());
  MOCK_METHOD0(get_errno, int32_t());

  MOCK_METHOD0(get_socket_errno, int32_t());
  MOCK_METHOD1(set_socket_errno, void(const int32_t));
  MOCK_METHOD2(get_socket_error_and_message, void(int32_t *, std::string *));

  MOCK_METHOD1(freeaddrinfo, void(addrinfo *));
  MOCK_METHOD4(getaddrinfo, int32_t(const char *, const char *,
                                    const addrinfo *, addrinfo **));

  MOCK_METHOD1(sleep, void(uint32_t));
};

class Mock_file : public iface::File {
 public:
  MOCK_METHOD0(is_valid, bool());

  MOCK_METHOD0(close, int());
  MOCK_METHOD2(read, int(void *, int));
  MOCK_METHOD2(write, int(void *, int));
  MOCK_METHOD0(fsync, int());
};

class Mock_factory : public iface::Operations_factory {
 public:
  MOCK_METHOD4(create_socket,
               std::shared_ptr<iface::Socket>(PSI_socket_key, int, int, int));
  MOCK_METHOD1(create_socket, std::shared_ptr<iface::Socket>(MYSQL_SOCKET));

  MOCK_METHOD3(open_file, std::shared_ptr<iface::File>(const char *, int, int));

  MOCK_METHOD0(create_system_interface, std::shared_ptr<iface::System>());
};

class Mock_socket_events : public iface::Socket_events {
 public:
  MOCK_METHOD2(listen, bool(std::shared_ptr<iface::Socket>,
                            std::function<void(iface::Connection_acceptor &)>));
  MOCK_METHOD2(add_timer, void(const std::size_t, std::function<bool()>));
  MOCK_METHOD0(loop, void());
  MOCK_METHOD0(break_loop, void());
};

class Mock_listener_factory_interface : public iface::Listener_factory {
 public:
  MOCK_CONST_METHOD3(create_unix_socket_listener_ptr,
                     iface::Listener *(const std::string &unix_socket_path,
                                       const iface::Socket_events &event,
                                       const uint32_t backlog));

  MOCK_CONST_METHOD6(create_tcp_socket_listener_ptr,
                     iface::Listener *(const std::string &bind_address,
                                       const std::string &network_namespace,
                                       const unsigned short port,
                                       const uint32_t port_open_timeout,
                                       const iface::Socket_events &event,
                                       const uint32_t backlog));

  std::unique_ptr<iface::Listener> create_unix_socket_listener(
      const std::string &unix_socket_path, iface::Socket_events *event,
      const uint32_t backlog) const override {
    return std::unique_ptr<iface::Listener>{
        create_unix_socket_listener_ptr(unix_socket_path, *event, backlog)};
  }

  std::unique_ptr<iface::Listener> create_tcp_socket_listener(
      std::string *bind_address, const std::string &network_namespace,
      const unsigned short port, const uint32_t port_open_timeout,
      iface::Socket_events *event, const uint32_t backlog) const override {
    return std::unique_ptr<iface::Listener>{
        create_tcp_socket_listener_ptr(*bind_address, network_namespace, port,
                                       port_open_timeout, *event, backlog)};
  }
};

}  // namespace test
}  // namespace xpl

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_MOCK_NGS_GENERAL_H_
