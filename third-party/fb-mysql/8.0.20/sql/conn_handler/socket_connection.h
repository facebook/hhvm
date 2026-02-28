/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef SOCKET_CONNECTION_INCLUDED
#define SOCKET_CONNECTION_INCLUDED

#include "my_config.h"

#include <sys/types.h>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "my_psi_config.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_socket.h"  // MYSQL_SOCKET
#ifdef HAVE_POLL_H
#include <poll.h>
#endif

class Channel_info;

extern const char *MY_BIND_ALL_ADDRESSES;
extern const char *ipv4_all_addresses;
extern const char *ipv6_all_addresses;

#ifdef HAVE_PSI_STATEMENT_INTERFACE
extern PSI_statement_info stmt_info_new_packet;
#endif

/**
  Key Comparator for socket_map_t used in Mysqld_socket_listener
*/
struct Socket_lt_type {
  bool operator()(const MYSQL_SOCKET &s1, const MYSQL_SOCKET &s2) const {
    return mysql_socket_getfd(s1) < mysql_socket_getfd(s2);
  }
};

// Enum denoting type of socket whether unix socket or tcp socket.
enum class Socket_type { UNIX_SOCKET, TCP_SOCKET };
// Listen socket attributes.
struct Socket_attr {
  explicit Socket_attr(Socket_type socket_type) : m_socket_type(socket_type) {}
  Socket_attr(Socket_type socket_type, const std::string &network_namespace)
      : m_socket_type(socket_type), m_network_namespace(network_namespace) {}
  Socket_type m_socket_type;
  std::string m_network_namespace;
};

/**
  Typedef representing socket map type which hold the sockets and a
  corresponding bool which is true if it is unix socket and false for tcp
  socket.
*/
typedef std::map<MYSQL_SOCKET, Socket_attr, Socket_lt_type> socket_map_t;

// iterator type for socket map type.
typedef std::map<MYSQL_SOCKET, Socket_attr, Socket_lt_type>::const_iterator
    socket_map_const_iterator_t;

/**
  Plain structure to collect together a host name/ip address and
  a corresponding network namespace if set and pass these information
  to different functions as a single unit.
*/
struct Bind_address_info {
  std::string address, network_namespace;
  Bind_address_info() = default;

  explicit Bind_address_info(const std::string &addr) : address(addr) {}

  Bind_address_info(const std::string &addr, const std::string &nspace)
      : address(addr), network_namespace(nspace) {}
};

/**
  This class represents the Mysqld_socket_listener which prepare the
  listener sockets to recieve connection events from the client. The
  Mysqld_socket_listener may be composed of either or both a tcp socket
  which listen on a default mysqld tcp port or a user specified  port
  via mysqld command-line  and a unix socket which is bind to a mysqld
  defaul pathname.
*/
class Mysqld_socket_listener {
  /*
    Addresses to listen to and network namespace for
    every address if set.
  */
  std::list<Bind_address_info> m_bind_addresses;
  /*
    Address to listen to an admin connection request
    and network namespace if set.
  */
  Bind_address_info m_admin_bind_address;
  uint m_tcp_port;        // TCP port to bind to
  uint m_admin_tcp_port;  // TCP port to bind to for support admin connection
  bool m_use_separate_thread_for_admin;  // use a separate thread for listening
                                         // to admin interface
  uint m_backlog;       // backlog specifying length of pending connection queue
  uint m_port_timeout;  // port timeout value
  std::string m_unix_sockname;  // unix socket pathname to bind to
  bool m_unlink_sockname;       // Unlink socket & lock file if true.
  /*
    Map indexed by MYSQL socket fds and correspoding bool to distinguish
    between unix and tcp socket.
  */
  socket_map_t m_socket_map;  // map indexed by mysql socket fd and index
  MYSQL_SOCKET m_admin_interface_listen_socket;

#ifdef HAVE_POLL
  struct poll_info_t {
    std::vector<struct pollfd> m_fds;
    std::vector<MYSQL_SOCKET> m_pfs_fds;
  };
  // poll related info. used in poll for listening to connection events.
  poll_info_t m_poll_info;
#else
  struct select_info_t {
    fd_set m_read_fds, m_client_fds;
    my_socket m_max_used_connection;
    select_info_t() : m_max_used_connection(0) { FD_ZERO(&m_client_fds); }
  };
  // select info for used in select for listening to connection events.
  select_info_t m_select_info;
#endif  // HAVE_POLL

 public:
  /**
    Constructor to setup a listener for listen to connect events from
    clients.

    @param   bind_addresses  list of addresses to listen to
    @param   tcp_port        TCP port to bind to
    @param   admin_bind_addr address to listen admin connection
    @param   admin_tcp_port  TCP port for admin connection to bind
    @param   use_separate_thread_for_admin  Listen to connection requests
                             on admin interface in a separate thread
    @param   backlog         backlog specifying length of pending
                             connection queue used in listen.
    @param   port_timeout    portname.
    @param   unix_sockname   pathname for unix socket to bind to
  */
  Mysqld_socket_listener(const std::list<Bind_address_info> &bind_addresses,
                         uint tcp_port,
                         const Bind_address_info &admin_bind_addr,
                         uint admin_tcp_port,
                         bool use_separate_thread_for_admin, uint backlog,
                         uint port_timeout, std::string unix_sockname);

  /**
    Set up a listener - set of sockets to listen for connection events
    from clients.

    In case a server is started with the option
    use_separate_thread_for_admin=true invocation of this method also spawns a
    thread to handle incoming connection requests on admin interface.

    @retval false  listener sockets setup to be used to listen for connect
    events true   failure in setting up the listener.
  */
  bool setup_listener();

  /**
    The body of the event loop that listen for connection events from clients.

    @retval Channel_info   Channel_info object abstracting the connected client
                           details for processing this connection.
  */
  Channel_info *listen_for_connection_event();

  /**
    Close the listener.

    In case a server is started with the option
    use_separate_thread_for_admin=true this method also shutdowns a thread for
    handling of incoming connection requests on admin interface and joins it.
  */
  void close_listener();

  ~Mysqld_socket_listener() {
    if (!m_socket_map.empty()) close_listener();
  }

 private:
  /**
    Add a socket to a set of sockets being waiting for a new
    connection request.

    @param listen_socket  Socket to listen for.
  */
  void add_socket_to_listener(MYSQL_SOCKET listen_socket);

  /**
    Get a socket ready to accept incoming connection.
    @param[out] is_unix_socket  has the value true in case a new incoming
                                connection ready for acceptance pertains
                                to unix socket domain.
    @param[out] is_admin_socket  has the value true in case a new incoming
                                 connection is waiting for acceptance on
                                 admin interface.

    @return A socket ready to accept a new incoming connection.
  */
  MYSQL_SOCKET get_ready_socket(bool *is_unix_socket,
                                bool *is_admin_socket) const;

  /**
    Set up connection events for poll or select.

    @param socket_map  sockets to listen for connection requests.
  */
  void setup_connection_events(const socket_map_t &socket_map);
};

ulong get_connection_errors_select();

ulong get_connection_errors_accept();

ulong get_connection_errors_tcpwrap();

#endif  // SOCKET_CONNECTION_INCLUDED.
