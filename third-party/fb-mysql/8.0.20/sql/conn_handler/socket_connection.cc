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

#include "sql/conn_handler/socket_connection.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>

#include <limits.h>
#ifndef _WIN32
#include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <algorithm>
#include <atomic>
#include <memory>  // std::unique_ptr
#include <new>
#include <utility>

#include "m_string.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_thread.h"
#include "mysqld_error.h"
#include "sql-common/net_ns.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/conn_handler/channel_info.h"               // Channel_info
#include "sql/conn_handler/init_net_server_extension.h"  // init_net_server_extension
#include "sql/log.h"
#include "sql/mysqld.h"     // key_socket_tcpip
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "violite.h"  // Vio
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_LIBWRAP
#include <syslog.h>
#ifndef HAVE_LIBWRAP_PROTOTYPES
extern "C" {
#include <tcpd.h>
}
#else
#include <tcpd.h>
#endif
#endif
#include "connection_handler_manager.h"

using std::max;

/** Number of connection errors when selecting on the listening port */
static std::atomic<ulong> connection_errors_select{0};

/** Number of connection errors when accepting sockets in the listening port. */
static std::atomic<ulong> connection_errors_accept{0};

/** Number of connection errors from TCP wrappers. */
static std::atomic<ulong> connection_errors_tcpwrap{0};

namespace {
struct FreeAddrInfoDeleter {
  void operator()(addrinfo *ai) {
    if (ai != nullptr) {
      freeaddrinfo(ai);
    }
  }
};

using AddrInfoPtr = std::unique_ptr<addrinfo, FreeAddrInfoDeleter>;
AddrInfoPtr GetAddrInfoPtr(const char *node, const char *service,
                           const addrinfo *hints) {
  addrinfo *p = nullptr;
  int err = getaddrinfo(node, service, hints, &p);
  AddrInfoPtr nrv{p};
  return (err == 0 ? std::move(nrv) : nullptr);
}
}  // namespace

ulong get_connection_errors_select() { return connection_errors_select.load(); }

ulong get_connection_errors_accept() { return connection_errors_accept.load(); }

ulong get_connection_errors_tcpwrap() {
  return connection_errors_tcpwrap.load();
}

#ifdef HAVE_LIBWRAP
static const char *libwrap_name;
#endif

///////////////////////////////////////////////////////////////////////////
// Channel_info_local_socket implementation
///////////////////////////////////////////////////////////////////////////

/**
  This class abstracts the info. about local socket mode of communication with
  the server.
*/
class Channel_info_local_socket : public Channel_info {
  // connect socket object
  MYSQL_SOCKET m_connect_sock;

 protected:
  virtual Vio *create_and_init_vio() const {
    Vio *vio =
        mysql_socket_vio_new(m_connect_sock, VIO_TYPE_SOCKET, VIO_LOCALHOST);
#ifdef USE_PPOLL_IN_VIO
    if (vio != nullptr) {
      vio->thread_id = my_thread_self();
      vio->signal_mask = mysqld_signal_mask;
    }
#endif
    return vio;
  }

 public:
  /**
    Constructor that sets the connect socket.

    @param connect_socket set connect socket descriptor.
  */
  Channel_info_local_socket(MYSQL_SOCKET connect_socket)
      : m_connect_sock(connect_socket) {}

  virtual THD *create_thd() {
    THD *thd = Channel_info::create_thd();

    if (thd != nullptr) {
      init_net_server_extension(thd);
      thd->security_context()->set_host_ptr(my_localhost, strlen(my_localhost));
    }
    return thd;
  }

  virtual void send_error_and_close_channel(uint errorcode, int error,
                                            bool senderror) {
    Channel_info::send_error_and_close_channel(errorcode, error, senderror);

    mysql_socket_shutdown(m_connect_sock, SHUT_RDWR);
    mysql_socket_close(m_connect_sock);
  }
};

///////////////////////////////////////////////////////////////////////////
// Channel_info_tcpip_socket implementation
///////////////////////////////////////////////////////////////////////////

/**
  This class abstracts the info. about TCP/IP socket mode of communication with
  the server.
*/
class Channel_info_tcpip_socket : public Channel_info {
  // connect socket object
  MYSQL_SOCKET m_connect_sock;
  /*
    Flag specifying whether a connection is admin connection or
    ordinary connection.
  */
  bool m_is_admin_conn;
#ifdef HAVE_SETNS
  /*
    Network namespace associated with the socket.
  */
  std::string m_network_namespace;
#endif

 protected:
  virtual Vio *create_and_init_vio() const {
    Vio *vio = mysql_socket_vio_new(m_connect_sock, VIO_TYPE_TCPIP, 0);
#ifdef USE_PPOLL_IN_VIO
    if (vio != nullptr) {
      vio->thread_id = my_thread_self();
      vio->signal_mask = mysqld_signal_mask;
    }
#endif

#ifdef HAVE_SETNS
    strncpy(vio->network_namespace, m_network_namespace.c_str(),
            sizeof(vio->network_namespace) - 1);
    vio->network_namespace[sizeof(vio->network_namespace) - 1] = '\0';
#endif

    return vio;
  }

 public:
  /**
    Constructor that sets the connect socket.

    @param connect_socket set connect socket descriptor.
    @param is_admin_conn  flag specifying whether a connection is admin
                          connection.
  */
  Channel_info_tcpip_socket(MYSQL_SOCKET connect_socket, bool is_admin_conn)
      : m_connect_sock(connect_socket), m_is_admin_conn(is_admin_conn) {}

  virtual THD *create_thd() {
    THD *thd = Channel_info::create_thd();

    if (thd != nullptr) {
      thd->set_admin_connection(m_is_admin_conn);
      init_net_server_extension(thd);
    }
    return thd;
  }

  virtual void send_error_and_close_channel(uint errorcode, int error,
                                            bool senderror) {
    Channel_info::send_error_and_close_channel(errorcode, error, senderror);

    mysql_socket_shutdown(m_connect_sock, SHUT_RDWR);
    mysql_socket_close(m_connect_sock);
  }

  virtual bool is_admin_connection() const { return m_is_admin_conn; }

#ifdef HAVE_SETNS
  /**
    Set a network namespace for channel.

    @param network_namespace  Network namespace associated with a channel.
  */
  void set_network_namespace(const std::string &network_namespace) {
    m_network_namespace = network_namespace;
  }
#endif
};

///////////////////////////////////////////////////////////////////////////
// TCP_socket implementation
///////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
using Socket_error_message_buf = TCHAR[1024];
#endif

/**
  MY_BIND_ALL_ADDRESSES defines a special value for the bind-address option,
  which means that the server should listen to all available network addresses,
  both IPv6 (if available) and IPv4.

  Basically, this value instructs the server to make an attempt to bind the
  server socket to '::' address, and rollback to '0.0.0.0' if the attempt fails.
*/
const char *MY_BIND_ALL_ADDRESSES = "*";

const char *ipv4_all_addresses = "0.0.0.0";

const char *ipv6_all_addresses = "::";

/**
  TCP_socket class represents the TCP sockets abstraction. It provides
  the get_listener_socket that setup a TCP listener socket to listen.
*/
class TCP_socket {
  std::string m_bind_addr_str;      // IP address as string.
  std::string m_network_namespace;  // Network namespace if specified
  uint m_tcp_port;                  // TCP port to bind to
  uint m_backlog;       // Backlog length for queue of pending connections.
  uint m_port_timeout;  // Port timeout

  MYSQL_SOCKET create_socket(const struct addrinfo *addrinfo_list,
                             int addr_family, struct addrinfo **use_addrinfo) {
    for (const struct addrinfo *cur_ai = addrinfo_list; cur_ai != nullptr;
         cur_ai = cur_ai->ai_next) {
      if (cur_ai->ai_family != addr_family) continue;

      MYSQL_SOCKET sock =
          mysql_socket_socket(key_socket_tcpip, cur_ai->ai_family,
                              cur_ai->ai_socktype, cur_ai->ai_protocol);

      char ip_addr[INET6_ADDRSTRLEN];

      if (vio_getnameinfo(cur_ai->ai_addr, ip_addr, sizeof(ip_addr), nullptr, 0,
                          NI_NUMERICHOST)) {
        ip_addr[0] = 0;
      }

      if (mysql_socket_getfd(sock) == INVALID_SOCKET) {
        LogErr(ERROR_LEVEL, ER_CONN_TCP_NO_SOCKET,
               (addr_family == AF_INET) ? "IPv4" : "IPv6",
               (const char *)ip_addr, (int)socket_errno);
      } else {
        LogErr(INFORMATION_LEVEL, ER_CONN_TCP_CREATED, (const char *)ip_addr);

        *use_addrinfo = const_cast<addrinfo *>(cur_ai);
        return sock;
      }
    }

    return MYSQL_INVALID_SOCKET;
  }

 public:
  /**
    Constructor that takes tcp port and ip address string and other
    related parameters to set up listener tcp to listen for connection
    events.

    @param  bind_addr_str  ip address as string value.
    @param  network_namespace_str  network namespace as string value
    @param  tcp_port  tcp port number.
    @param  backlog backlog specifying length of pending connection queue.
    @param  port_timeout port timeout value
  */
  TCP_socket(std::string bind_addr_str, std::string network_namespace_str,
             uint tcp_port, uint backlog, uint port_timeout)
      : m_bind_addr_str(bind_addr_str),
        m_network_namespace(network_namespace_str),
        m_tcp_port(tcp_port),
        m_backlog(backlog),
        m_port_timeout(port_timeout) {}

  /**
    Set up a listener to listen for connection events.

    @retval   valid socket if successful else MYSQL_INVALID_SOCKET on failure.
  */
  MYSQL_SOCKET get_listener_socket() {
    const char *bind_address_str = nullptr;

    LogErr(INFORMATION_LEVEL, ER_CONN_TCP_ADDRESS, m_bind_addr_str.c_str(),
           m_tcp_port);

    // Get list of IP-addresses associated with the bind-address.

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    char port_buf[NI_MAXSERV];
    snprintf(port_buf, NI_MAXSERV, "%d", m_tcp_port);

    if (!m_network_namespace.empty()) {
#ifdef HAVE_SETNS
      if (set_network_namespace(m_network_namespace))
        return MYSQL_INVALID_SOCKET;
#else
      LogErr(ERROR_LEVEL, ER_NETWORK_NAMESPACES_NOT_SUPPORTED);
      return MYSQL_INVALID_SOCKET;
#endif
    }

    // Create a RAII guard for addrinfo struct.
    AddrInfoPtr ai_ptr{nullptr};

    if (native_strcasecmp(m_bind_addr_str.c_str(), MY_BIND_ALL_ADDRESSES) ==
        0) {
      /*
        That's the case when bind-address is set to a special value ('*'),
        meaning "bind to all available IP addresses". If the box supports
        the IPv6 stack, that means binding to '::'. If only IPv4 is available,
        bind to '0.0.0.0'.
      */

      bool ipv6_available = false;
      ai_ptr = GetAddrInfoPtr(ipv6_all_addresses, port_buf, &hints);
      if (ai_ptr) {
        /*
          IPv6 might be available (the system might be able to resolve an IPv6
          address, but not be able to create an IPv6-socket). Try to create a
          dummy IPv6-socket. Do not instrument that socket by P_S.
        */

        MYSQL_SOCKET s = mysql_socket_socket(0, AF_INET6, SOCK_STREAM, 0);
        ipv6_available = mysql_socket_getfd(s) != INVALID_SOCKET;
        if (ipv6_available) mysql_socket_close(s);
      }
      if (ipv6_available &&
          DBUG_EVALUATE_IF("sim_ipv6_unavailable", false, true)) {
        LogErr(INFORMATION_LEVEL, ER_CONN_TCP_IPV6_AVAILABLE);

        // Address info (ai) for IPv6 address is already set.

        bind_address_str = ipv6_all_addresses;
      } else {
        LogErr(INFORMATION_LEVEL, ER_CONN_TCP_IPV6_UNAVAILABLE);

        // Retrieve address info (ai) for IPv4 address.
        ai_ptr = GetAddrInfoPtr(ipv4_all_addresses, port_buf, &hints);
        if (!ai_ptr) {
#ifdef _WIN32
          Socket_error_message_buf msg_buff;
          FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)msg_buff, sizeof(msg_buff), NULL);
          LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, msg_buff);
#else
          LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, strerror(errno));
#endif
          LogErr(ERROR_LEVEL, ER_CONN_TCP_CANT_RESOLVE_HOSTNAME);
#ifdef HAVE_SETNS
          if (!m_network_namespace.empty())
            (void)restore_original_network_namespace();
#endif
          return MYSQL_INVALID_SOCKET;
        }  // !ai_ptr
        bind_address_str = ipv4_all_addresses;
      }
    } else {
      ai_ptr = GetAddrInfoPtr(m_bind_addr_str.c_str(), port_buf, &hints);
      if (!ai_ptr) {
#ifdef _WIN32
        Socket_error_message_buf msg_buff;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)msg_buff, sizeof(msg_buff), NULL);
        LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, msg_buff);
#else
        LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, strerror(errno));
#endif
        LogErr(ERROR_LEVEL, ER_CONN_TCP_CANT_RESOLVE_HOSTNAME);
#ifdef HAVE_SETNS
        if (!m_network_namespace.empty())
          (void)restore_original_network_namespace();
#endif

        return MYSQL_INVALID_SOCKET;
      }  // !ai_ptr
      bind_address_str = m_bind_addr_str.c_str();
    }

    // Log all the IP-addresses
    for (struct addrinfo *cur_ai = ai_ptr.get(); cur_ai != nullptr;
         cur_ai = cur_ai->ai_next) {
      char ip_addr[INET6_ADDRSTRLEN];

      if (vio_getnameinfo(cur_ai->ai_addr, ip_addr, sizeof(ip_addr), nullptr, 0,
                          NI_NUMERICHOST)) {
        LogErr(ERROR_LEVEL, ER_CONN_TCP_IP_NOT_LOGGED);
        continue;
      }

      LogErr(INFORMATION_LEVEL, ER_CONN_TCP_RESOLVE_INFO, bind_address_str,
             ip_addr);
    }

    /*
      If the 'bind-address' option specifies the hostname, which resolves to
      multiple IP-address, use the following rule:
      - if there are IPv4-addresses, use the first IPv4-address
      returned by getaddrinfo();
      - if there are IPv6-addresses, use the first IPv6-address
      returned by getaddrinfo();
    */

    struct addrinfo *a = nullptr;

    MYSQL_SOCKET listener_socket = create_socket(ai_ptr.get(), AF_INET, &a);

    if (mysql_socket_getfd(listener_socket) == INVALID_SOCKET)
      listener_socket = create_socket(ai_ptr.get(), AF_INET6, &a);

#ifdef HAVE_SETNS
    if (!m_network_namespace.empty() && restore_original_network_namespace())
      return MYSQL_INVALID_SOCKET;
#endif

    // Report user-error if we failed to create a socket.
    if (mysql_socket_getfd(listener_socket) == INVALID_SOCKET) {
#ifdef _WIN32
      Socket_error_message_buf msg_buff;
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg_buff,
                    sizeof(msg_buff), NULL);
      LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, msg_buff);
#else
      LogErr(ERROR_LEVEL, ER_CONN_TCP_ERROR_WITH_STRERROR, strerror(errno));
#endif
      return MYSQL_INVALID_SOCKET;
    }

    mysql_socket_set_thread_owner(listener_socket);

#ifndef _WIN32
    /*
      We should not use SO_REUSEADDR on windows as this would enable a
      user to open two mysqld servers with the same TCP/IP port.
    */
    {
      int option_flag = 1;
      (void)mysql_socket_setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR,
                                    (char *)&option_flag, sizeof(option_flag));
    }
#endif
#ifdef IPV6_V6ONLY
    /*
      For interoperability with older clients, IPv6 socket should
      listen on both IPv6 and IPv4 wildcard addresses.
      Turn off IPV6_V6ONLY option.

      NOTE: this will work starting from Windows Vista only.
      On Windows XP dual stack is not available, so it will not
      listen on the corresponding IPv4-address.
    */
    if (a->ai_family == AF_INET6) {
      int option_flag = 0;

      if (mysql_socket_setsockopt(listener_socket, IPPROTO_IPV6, IPV6_V6ONLY,
                                  (char *)&option_flag, sizeof(option_flag))) {
        LogErr(WARNING_LEVEL, ER_CONN_TCP_CANT_RESET_V6ONLY, (int)socket_errno);
      }
    }
#endif
    /*
      Sometimes the port is not released fast enough when stopping and
      restarting the server. This happens quite often with the test suite
      on busy Linux systems. Retry to bind the address at these intervals:
      Sleep intervals: 1, 2, 4,  6,  9, 13, 17, 22, ...
      Retry at second: 1, 3, 7, 13, 22, 35, 52, 74, ...
      Limit the sequence by m_port_timeout (set --port-open-timeout=#).
    */
    uint this_wait = 0;
    int ret = 0;
    for (uint waited = 0, retry = 1;; retry++, waited += this_wait) {
      if (((ret = mysql_socket_bind(listener_socket, a->ai_addr,
                                    a->ai_addrlen)) >= 0) ||
          (socket_errno != SOCKET_EADDRINUSE) || (waited >= m_port_timeout))
        break;
      LogErr(INFORMATION_LEVEL, ER_CONN_TCP_BIND_RETRY, mysqld_port);
      this_wait = retry * retry / 3 + 1;
      sleep(this_wait);
    }

    if (ret < 0) {
      DBUG_PRINT("error", ("Got error: %d from bind", socket_errno));
#ifdef _WIN32
      Socket_error_message_buf msg_buff;
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg_buff,
                    sizeof(msg_buff), NULL);
      LogErr(ERROR_LEVEL, ER_CONN_TCP_BIND_FAIL, msg_buff);
#else
      LogErr(ERROR_LEVEL, ER_CONN_TCP_BIND_FAIL, strerror(socket_errno));
#endif
      LogErr(ERROR_LEVEL, ER_CONN_TCP_IS_THERE_ANOTHER_USING_PORT, m_tcp_port);
      mysql_socket_close(listener_socket);
      return MYSQL_INVALID_SOCKET;
    }

    if (mysql_socket_listen(listener_socket, static_cast<int>(m_backlog)) < 0) {
#ifdef _WIN32
      Socket_error_message_buf msg_buff;
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg_buff,
                    sizeof(msg_buff), NULL);
      LogErr(ERROR_LEVEL, ER_CONN_TCP_START_FAIL, msg_buff);
#else
      LogErr(ERROR_LEVEL, ER_CONN_TCP_START_FAIL, strerror(errno));
#endif

      LogErr(ERROR_LEVEL, ER_CONN_TCP_LISTEN_FAIL, socket_errno);
      mysql_socket_close(listener_socket);
      return MYSQL_INVALID_SOCKET;
    }

#if !defined(NO_FCNTL_NONBLOCK)
    (void)mysql_sock_set_nonblocking(listener_socket);
#endif

    return listener_socket;
  }
};

#if defined(HAVE_SYS_UN_H)
///////////////////////////////////////////////////////////////////////////
// Unix_socket implementation
///////////////////////////////////////////////////////////////////////////

/**
  The Unix_socket class represents an abstraction for creating a unix
  socket ready to listen for new connections from clients.
*/
class Unix_socket {
  std::string m_unix_sockname;  // pathname for socket to bind to.
  uint m_backlog;  // backlog specifying lenght of pending queue connection.
  /**
    Create a lockfile which contains the pid of the mysqld instance started
    and pathname as name of unix socket pathname appended with .lock

    @retval   false if lockfile creation is successful else true if lockfile
              file could not be created.

  */
  bool create_lockfile();

 public:
  /**
    Constructor that takes pathname for unix socket to bind to
    and backlog specifying the length of pending connection queue.

    @param  unix_sockname pointer to pathname for the created unix socket
            to bind.
    @param  backlog   specifying the length of pending connection queue.
  */
  Unix_socket(const std::string *unix_sockname, uint backlog)
      : m_unix_sockname(*unix_sockname), m_backlog(backlog) {}

  /**
    Set up a listener socket which is ready to listen for connection from
    clients.

    @retval   valid socket if successful else MYSQL_INVALID_SOCKET on failure.
  */
  MYSQL_SOCKET get_listener_socket() {
    struct sockaddr_un UNIXaddr;
    DBUG_PRINT("general", ("UNIX Socket is %s", m_unix_sockname.c_str()));

    // Check path length, probably move to set unix port?
    if (m_unix_sockname.length() > (sizeof(UNIXaddr.sun_path) - 1)) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_PATH_TOO_LONG,
             (uint)sizeof(UNIXaddr.sun_path) - 1, m_unix_sockname.c_str());
      return MYSQL_INVALID_SOCKET;
    }

    if (create_lockfile()) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_FAIL);
      return MYSQL_INVALID_SOCKET;
    }

    MYSQL_SOCKET listener_socket =
        mysql_socket_socket(key_socket_unix, AF_UNIX, SOCK_STREAM, 0);

    if (mysql_socket_getfd(listener_socket) < 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_NO_FD, strerror(errno));
      return MYSQL_INVALID_SOCKET;
    }

    mysql_socket_set_thread_owner(listener_socket);

    memset(&UNIXaddr, 0, sizeof(UNIXaddr));
    UNIXaddr.sun_family = AF_UNIX;
    my_stpcpy(UNIXaddr.sun_path, m_unix_sockname.c_str());
    (void)unlink(m_unix_sockname.c_str());

    // Set socket option SO_REUSEADDR
    int option_enable = 1;
    (void)mysql_socket_setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR,
                                  (char *)&option_enable,
                                  sizeof(option_enable));
    // bind
    umask(0);
    if (mysql_socket_bind(listener_socket,
                          reinterpret_cast<struct sockaddr *>(&UNIXaddr),
                          sizeof(UNIXaddr)) < 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_NO_BIND_NO_START, strerror(errno));
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_IS_THERE_ANOTHER_USING_SOCKET,
             m_unix_sockname.c_str());
      mysql_socket_close(listener_socket);
      return MYSQL_INVALID_SOCKET;
    }
    umask(((~my_umask) & 0666));

    // listen
    if (mysql_socket_listen(listener_socket, (int)m_backlog) < 0)
      LogErr(WARNING_LEVEL, ER_CONN_UNIX_LISTEN_FAILED, socket_errno);

      // set sock fd non blocking.
#if !defined(NO_FCNTL_NONBLOCK)
    (void)mysql_sock_set_nonblocking(listener_socket);
#endif

    return listener_socket;
  }
};

bool Unix_socket::create_lockfile() {
  int fd;
  char buffer[8];
  pid_t cur_pid = getpid();
  std::string lock_filename = m_unix_sockname + ".lock";

  static_assert(sizeof(pid_t) == 4, "");
  int retries = 3;
  while (true) {
    if (!retries--) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_GIVING_UP,
             lock_filename.c_str());
      return true;
    }

    fd = open(lock_filename.c_str(), O_RDWR | O_CREAT | O_EXCL, 0600);

    if (fd >= 0) break;

    if (errno != EEXIST) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_CREATE,
             lock_filename.c_str());
      return true;
    }

    fd = open(lock_filename.c_str(), O_RDONLY, 0600);
    if (fd < 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_OPEN,
             lock_filename.c_str());
      return true;
    }

    ssize_t len;
    if ((len = read(fd, buffer, sizeof(buffer) - 1)) < 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_READ,
             lock_filename.c_str());
      close(fd);
      return true;
    }

    close(fd);

    if (len == 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_EMPTY, lock_filename.c_str());
      return true;
    }
    buffer[len] = '\0';

    pid_t parent_pid = getppid();
    pid_t read_pid = atoi(buffer);

    if (read_pid <= 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_PIDLESS,
             lock_filename.c_str());
      return true;
    }

    if (read_pid != cur_pid && read_pid != parent_pid) {
      if (kill(read_pid, 0) == 0) {
        LogErr(ERROR_LEVEL, ER_CONN_UNIX_PID_CLAIMED_SOCKET_FILE,
               static_cast<int>(read_pid));
        return true;
      }
    }

    /*
      Unlink the lock file as it is not associated with any process and
      retry.
    */
    if (unlink(lock_filename.c_str()) < 0) {
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_DELETE,
             lock_filename.c_str(), errno);
      return true;
    }
  }

  snprintf(buffer, sizeof(buffer), "%d\n", static_cast<int>(cur_pid));
  if (write(fd, buffer, strlen(buffer)) !=
      static_cast<signed>(strlen(buffer))) {
    close(fd);
    LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_WRITE,
           lock_filename.c_str(), errno);

    if (unlink(lock_filename.c_str()) == -1)
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_DELETE,
             lock_filename.c_str(), errno);

    return true;
  }

  if (fsync(fd) != 0) {
    close(fd);
    LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_SYNC, lock_filename.c_str(),
           errno);

    if (unlink(lock_filename.c_str()) == -1)
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_DELETE,
             lock_filename.c_str(), errno);

    return true;
  }

  if (close(fd) != 0) {
    LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_CLOSE,
           lock_filename.c_str(), errno);

    if (unlink(lock_filename.c_str()) == -1)
      LogErr(ERROR_LEVEL, ER_CONN_UNIX_LOCK_FILE_CANT_DELETE,
             lock_filename.c_str(), errno);

    return true;
  }
  return false;
}

#endif  // HAVE_SYS_UN_H

///////////////////////////////////////////////////////////////////////////
// Mysqld_socket_listener implementation
///////////////////////////////////////////////////////////////////////////

Mysqld_socket_listener::Mysqld_socket_listener(
    const std::list<Bind_address_info> &bind_addresses, uint tcp_port,
    const Bind_address_info &admin_bind_addr, uint admin_tcp_port,
    bool use_separate_thread_for_admin, uint backlog, uint port_timeout,
    std::string unix_sockname)
    : m_bind_addresses(bind_addresses),
      m_admin_bind_address(admin_bind_addr),
      m_tcp_port(tcp_port),
      m_admin_tcp_port(admin_tcp_port),
      m_use_separate_thread_for_admin(use_separate_thread_for_admin),
      m_backlog(backlog),
      m_port_timeout(port_timeout),
      m_unix_sockname(unix_sockname),
      m_unlink_sockname(false),
      m_admin_interface_listen_socket(mysql_socket_invalid()) {
#ifdef HAVE_LIBWRAP
  /*
    Set up syslog parameters on behalf of the TCP-wrappers.
    The loadable component that logs server errors to syslog
    may re-open it with user-defined attributes (logging of
    PIDS / ident) later, but we establish a sensible baseline
    here in case that log-sink is not used. Note that the
    wrapper is hard-coded to use LOG_AUTH in the syslog()
    call below, which lets the wrapper log to a different
    facility than the rest of the server (the facility of
    which defaults to LOG_DAEMON and is user-configurable)
    if desired.
  */
  libwrap_name = my_progname + dirname_length(my_progname);
  openlog(libwrap_name, LOG_PID, LOG_AUTH);
#endif /* HAVE_LIBWRAP */
}

void Mysqld_socket_listener::add_socket_to_listener(
    MYSQL_SOCKET listen_socket) {
  mysql_socket_set_thread_owner(listen_socket);

#ifdef HAVE_POLL
  m_poll_info.m_fds.emplace_back(
      pollfd{mysql_socket_getfd(listen_socket), POLLIN, 0});
  m_poll_info.m_pfs_fds.push_back(listen_socket);
#else   // HAVE_POLL
  FD_SET(mysql_socket_getfd(listen_socket), &m_select_info.m_client_fds);
  if ((uint)mysql_socket_getfd(listen_socket) >
      m_select_info.m_max_used_connection)
    m_select_info.m_max_used_connection = mysql_socket_getfd(listen_socket);
#endif  // HAVE_POLL
}

void Mysqld_socket_listener::setup_connection_events(
    const socket_map_t &socket_map) {
#ifdef HAVE_POLL
  const socket_map_t::size_type total_number_of_addresses_to_bind =
      socket_map.size();
  m_poll_info.m_fds.reserve(total_number_of_addresses_to_bind);
  m_poll_info.m_pfs_fds.reserve(total_number_of_addresses_to_bind);
#endif

  for (const auto &element : socket_map) add_socket_to_listener(element.first);
}

/**
  Accept a new connection on a ready listening socket.

  @param listen_sock  Listening socket ready to accept a new connection
  @param [out] connect_sock  Socket corresponding to a new accepted connection

  @return operation result
    @retval true on error
    @retval false on success
*/
static bool accept_connection(MYSQL_SOCKET listen_sock,
                              MYSQL_SOCKET *connect_sock) {
  struct sockaddr_storage c_addr;
  for (uint retry = 0; retry < MAX_ACCEPT_RETRY; retry++) {
    socket_len_t length = sizeof(struct sockaddr_storage);
    *connect_sock =
        mysql_socket_accept(key_socket_client_connection, listen_sock,
                            (struct sockaddr *)(&c_addr), &length);
    if (mysql_socket_getfd(*connect_sock) != INVALID_SOCKET ||
        (socket_errno != SOCKET_EINTR && socket_errno != SOCKET_EAGAIN))
      break;
  }
  if (mysql_socket_getfd(*connect_sock) == INVALID_SOCKET) {
    /*
      accept(2) failed on the listening port, after many retries.
      There is not much details to report about the client,
      increment the server global status variable.
    */

    if ((connection_errors_accept++ & 255) == 0) {  // This can happen often
#ifdef _WIN32
      Socket_error_message_buf msg_buff;
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, socket_errno,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)msg_buff,
                    sizeof(msg_buff), NULL);
      LogErr(ERROR_LEVEL, ER_CONN_SOCKET_ACCEPT_FAILED, msg_buff);
#else
      LogErr(ERROR_LEVEL, ER_CONN_SOCKET_ACCEPT_FAILED, strerror(errno));
#endif
    }
    if (socket_errno == SOCKET_ENFILE || socket_errno == SOCKET_EMFILE)
      sleep(1);  // Give other threads some time
    return true;
  }

  return false;
}

#ifdef HAVE_LIBWRAP
/**
  Ask TCP wrapper whether an accepted connection is allowed.

  @param connect_sock  Socket corresponding to accepted connection

  @return operation result
    @retval true  connection is prohibited by TCP wrapper's policy
    @retval false  connection is allowed by TCP wrapper's policy
*/
bool check_connection_refused_by_tcp_wrapper(MYSQL_SOCKET connect_sock) {
  struct request_info req;
  signal(SIGCHLD, SIG_DFL);
  request_init(&req, RQ_DAEMON, libwrap_name, RQ_FILE,
               mysql_socket_getfd(connect_sock), NULL);
  fromhost(&req);

  if (!hosts_access(&req)) {
    /*
      This may be stupid but refuse() includes an exit(0)
      which we surely don't want...
      clean_exit() - same stupid thing ...

      We're using syslog() here instead of my_syslog()
      as this lets us pass in a facility that may differ
      from that used by the error logging component.
      This is unproblematic as TCP-wrapper is unix specific,
      anyway.
    */
    syslog(LOG_AUTH | LOG_WARNING, "refused connect from %s",
           eval_client(&req));

#ifdef HAVE_LIBWRAP_PROTOTYPES
    // Some distros have patched tcpd.h to have proper prototypes
    if (req.sink) (req.sink)(req.fd);
#else
    // Some distros have not patched tcpd.h
    if (req.sink) ((void (*)(int))req.sink)(req.fd);
#endif
    /*
      The connection was refused by TCP wrappers.
      There are no details (by client IP) available to update the host_cache.
    */
    mysql_socket_shutdown(connect_sock, SHUT_RDWR);
    mysql_socket_close(connect_sock);

    connection_errors_tcpwrap++;
    return true;
  }

  return false;
}
#endif  // HAVE_LIBWRAP

static my_thread_handle admin_socket_thread_id;
static my_thread_attr_t admin_socket_thread_attrib;
static bool admin_thread_started = false;

static mysql_mutex_t LOCK_start_admin_thread;
static mysql_cond_t COND_start_admin_thread;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_start_admin_thread;
static PSI_cond_key key_COND_start_admin_thread;

static PSI_mutex_info admin_socket_thread_mutexes[] = {
    {&key_LOCK_start_admin_thread, "LOCK_start_admin_thread",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static PSI_cond_info admin_socket_thread_conds[] = {
    {&key_COND_start_admin_thread, "COND_start_admin_thread",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};

static inline void init_psi_keys(void) {
  const char *category = "sql";
  int count = array_elements(admin_socket_thread_mutexes);
  mysql_mutex_register(category, admin_socket_thread_mutexes, count);

  count = array_elements(admin_socket_thread_conds);
  mysql_cond_register(category, admin_socket_thread_conds, count);
}
#endif

/**
  Signal to a spawning thread that spawned thread has been started.
*/
static inline void mark_admin_thread_started() {
  mysql_mutex_lock(&LOCK_start_admin_thread);
  admin_thread_started = true;
  mysql_cond_signal(&COND_start_admin_thread);
  mysql_mutex_unlock(&LOCK_start_admin_thread);
}

/**
  Wait until admin thread has started execution.
*/
static inline void wait_for_admin_thread_started() {
  mysql_mutex_lock(&LOCK_start_admin_thread);
  while (!admin_thread_started)
    mysql_cond_wait(&COND_start_admin_thread, &LOCK_start_admin_thread);
  mysql_mutex_unlock(&LOCK_start_admin_thread);
}

/**
  Listen to admin interface and accept incoming connection on it.
  This function is run in a separate thread.

  @return operation result. false on success, true on error
*/
static bool handle_admin_socket(
    /// pointer to a socket for listening to admin interface
    MYSQL_SOCKET admin_socket
#ifdef HAVE_SETNS
    ,
    /// network namespace associated with the socket
    const std::string &network_namespace_for_listening_socket
#endif
) {
  mark_admin_thread_started();

#ifdef HAVE_POLL
  static const int NUMBER_OF_POLLED_FDS = 1;
  struct pollfd fds[NUMBER_OF_POLLED_FDS] = {
      {mysql_socket_getfd(admin_socket), POLLIN, 0}};
#else
  fd_set client_fds;
  FD_ZERO(&client_fds);
  FD_SET(mysql_socket_getfd(admin_socket), &client_fds);
  int max_used_connection = mysql_socket_getfd(admin_socket);
#endif

  while (!connection_events_loop_aborted()) {
    /*
      Ensure server is fully started before we start accepting
      connections, otherwise you may run into crashes due to
      race conditions with acl_init
     */
    if (get_server_state() != SERVER_OPERATING) {
      my_sleep(100 * 1000);
      continue;
    }
#ifdef HAVE_POLL
    int retval = poll(fds, NUMBER_OF_POLLED_FDS, -1);
#else
    fd_set read_fds = client_fds;
    int retval = select(max_used_connection, &read_fds, 0, 0, 0);
#endif

    if (retval < 0 && socket_errno != SOCKET_EINTR) {
      /*
        select(2)/poll(2) failed on the listening port.
        There is not much details to report about the client,
        increment the server global status variable.
      */
      ++connection_errors_select;
      if (!select_errors++ && !connection_events_loop_aborted())
        LogErr(ERROR_LEVEL, ER_CONN_SOCKET_SELECT_FAILED, socket_errno);
    }

    if (retval < 0) return true;

    if (connection_events_loop_aborted()) return false;

#ifdef HAVE_SETNS
    /*
      If a network namespace is specified for a listening socket then set this
      network namespace as active before call to accept().
      It is not clear from manuals whether a socket returned by a call to
      accept() borrows a network namespace from a server socket used for
      accepting a new connection. For that reason, assign a network namespace
      explicitly before calling accept().
    */
    if (!network_namespace_for_listening_socket.empty() &&
        set_network_namespace(network_namespace_for_listening_socket))
      return true;
#endif

    MYSQL_SOCKET connect_sock;
    bool accept_retval = accept_connection(admin_socket, &connect_sock);

#ifdef HAVE_SETNS
    if (!network_namespace_for_listening_socket.empty() &&
        restore_original_network_namespace())
      return true;
#endif

    if (accept_retval) continue;

#ifdef HAVE_LIBWRAP
    if (check_connection_refused_by_tcp_wrapper(connect_sock)) return true;
#endif  // HAVE_LIBWRAP

    Channel_info_tcpip_socket *channel_info =
        new (std::nothrow) Channel_info_tcpip_socket(connect_sock, true);
    if (channel_info == nullptr) {
      (void)mysql_socket_shutdown(connect_sock, SHUT_RDWR);
      (void)mysql_socket_close(connect_sock);
      connection_errors_internal++;
      return true;
    }

#ifdef HAVE_SETNS
    if (!network_namespace_for_listening_socket.empty())
      channel_info->set_network_namespace(
          network_namespace_for_listening_socket);
#endif

    Connection_handler_manager *mgr =
        Connection_handler_manager::get_instance();

    if (channel_info != nullptr) mgr->process_new_connection(channel_info);
  }

  return false;
}

using admin_thread_arg_t = std::pair<MYSQL_SOCKET, std::string>;

/**
  Initialize thread's internal structures, run thread loop,
  deinitialize thread's internal structure on thread exit.

  @param arg  pointer to a socket for listening to admin interface
*/
extern "C" void *admin_socket_thread(void *arg) {
  my_thread_init();

  std::unique_ptr<admin_thread_arg_t> arg_for_admin_socket_thread(
      (admin_thread_arg_t *)arg);

  (void)handle_admin_socket(arg_for_admin_socket_thread->first
#ifdef HAVE_SETNS
                            ,
                            arg_for_admin_socket_thread->second
#endif
  );

  my_thread_end();
  my_thread_exit(nullptr);

  return nullptr;
}

/**
  Initialize context required for running a thread handling connection requests
  on admin interface. Such context include mutex LOCK_start_admin_thread,
  condition variable COND_start_admin_thread and attributes used for thread
  spawning.
*/
static inline void initialize_thread_context() {
#ifdef HAVE_PSI_INTERFACE
  init_psi_keys();
#endif

  mysql_mutex_init(key_LOCK_start_admin_thread, &LOCK_start_admin_thread,
                   MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_start_admin_thread, &COND_start_admin_thread);

  (void)my_thread_attr_init(&admin_socket_thread_attrib);
  my_thread_attr_setdetachstate(&admin_socket_thread_attrib,
                                MY_THREAD_CREATE_JOINABLE);
#ifndef _WIN32
  pthread_attr_setscope(&admin_socket_thread_attrib, PTHREAD_SCOPE_SYSTEM);
#endif
}

/**
  Spawn a thread for handling incoming connections request on admin interface.

  @param admin_socket  A socket to listen corresponding admin interface.
  @param network_namespace Network namespace to use for communicating
                           via admin socket
  @return Operation result. false on success, true on failure.
*/
static inline bool spawn_admin_thread(MYSQL_SOCKET admin_socket,
                                      const std::string &network_namespace) {
  initialize_thread_context();

  admin_thread_arg_t *arg_for_admin_socket_thread =
      new (std::nothrow) admin_thread_arg_t(admin_socket, network_namespace);

  if (arg_for_admin_socket_thread == nullptr) return true;

  int ret = mysql_thread_create(
      key_thread_handle_con_admin_sockets, &admin_socket_thread_id,
      &admin_socket_thread_attrib, admin_socket_thread,
      (void *)arg_for_admin_socket_thread);

  (void)my_thread_attr_destroy(&admin_socket_thread_attrib);

  if (ret) {
    LogErr(ERROR_LEVEL, ER_CANT_CREATE_ADMIN_THREAD, errno);
    return true;
  }

  wait_for_admin_thread_started();

  return false;
}

bool Mysqld_socket_listener::setup_listener() {
  /*
    It's matter to add a socket for admin connection listener firstly,
    before listening sockets for other connection types be added.
    It is done in order to check availability of new incoming connection
    on admin interface with higher priority than on other interfaces..
  */
  if (!m_admin_bind_address.address.empty()) {
    TCP_socket tcp_socket(m_admin_bind_address.address,
                          m_admin_bind_address.network_namespace,
                          m_admin_tcp_port, m_backlog, m_port_timeout);

    MYSQL_SOCKET mysql_socket = tcp_socket.get_listener_socket();
    if (mysql_socket.fd == INVALID_SOCKET) return true;

    m_admin_interface_listen_socket = mysql_socket;

    if (m_use_separate_thread_for_admin) {
      if (spawn_admin_thread(m_admin_interface_listen_socket,
                             m_admin_bind_address.network_namespace))
        return true;
    } else {
      Socket_attr s(Socket_type::TCP_SOCKET,
                    m_admin_bind_address.network_namespace);
      m_socket_map.insert(
          std::pair<MYSQL_SOCKET, Socket_attr>(mysql_socket, s));
    }
  }

  // Setup tcp socket listener
  if (m_tcp_port) {
    for (const auto &bind_address_info : m_bind_addresses) {
      TCP_socket tcp_socket(bind_address_info.address,
                            bind_address_info.network_namespace, m_tcp_port,
                            m_backlog, m_port_timeout);

      MYSQL_SOCKET mysql_socket = tcp_socket.get_listener_socket();
      if (mysql_socket.fd == INVALID_SOCKET) return true;

      Socket_attr s(Socket_type::TCP_SOCKET,
                    bind_address_info.network_namespace);
      m_socket_map.insert(
          std::pair<MYSQL_SOCKET, Socket_attr>(mysql_socket, s));
    }
  }
#if defined(HAVE_SYS_UN_H)
  // Setup unix socket listener
  if (m_unix_sockname != "") {
    Unix_socket unix_socket(&m_unix_sockname, m_backlog);

    MYSQL_SOCKET mysql_socket = unix_socket.get_listener_socket();
    if (mysql_socket.fd == INVALID_SOCKET) return true;

    m_socket_map.insert(std::pair<MYSQL_SOCKET, Socket_attr>(
        mysql_socket, Socket_attr(Socket_type::UNIX_SOCKET)));
    m_unlink_sockname = true;
  }
#endif /* HAVE_SYS_UN_H */

  setup_connection_events(m_socket_map);

  return false;
}

MYSQL_SOCKET Mysqld_socket_listener::get_ready_socket(
    bool *is_unix_socket, bool *is_admin_socket) const {
/*
  In case admin interface was set up, then first check whether an admin socket
  ready to accept a new connection. Doing this way provides higher priority
  to admin interface over other listeners.
*/
#ifdef HAVE_POLL
  uint start_index = 0;

  if (!m_admin_bind_address.address.empty() &&
      !m_use_separate_thread_for_admin) {
    if (m_poll_info.m_fds[0].revents & POLLIN) {
      *is_unix_socket = false;
      *is_admin_socket = true;
      return m_admin_interface_listen_socket;
    } else
      start_index = 1;
  }

  *is_admin_socket = false;
  for (uint i = start_index; i < m_socket_map.size(); ++i) {
    if (m_poll_info.m_fds[i].revents & POLLIN) {
      MYSQL_SOCKET listen_sock = m_poll_info.m_pfs_fds[i];
      *is_unix_socket = m_socket_map.at(listen_sock).m_socket_type ==
                        Socket_type::UNIX_SOCKET;
      return listen_sock;
    }
  }

#else  // HAVE_POLL
  if (!m_admin_bind_address.address.empty() &&
      !m_use_separate_thread_for_admin &&
      FD_ISSET(mysql_socket_getfd(m_admin_interface_listen_socket),
               &m_select_info.m_read_fds)) {
    *is_unix_socket = false;
    *is_admin_socket = true;
    return m_admin_interface_listen_socket;
  }

  *is_admin_socket = false;
  for (socket_map_const_iterator_t sock_map_const_iter = m_socket_map.cbegin();
       sock_map_const_iter != m_socket_map.cend(); ++sock_map_const_iter) {
    if (FD_ISSET(mysql_socket_getfd(sock_map_const_iter->first),
                 &m_select_info.m_read_fds)) {
      *is_unix_socket =
          sock_map_const_iter->second.m_socket_type == Socket_type::UNIX_SOCKET;
      return sock_map_const_iter->first;
    }
  }

#endif  // HAVE_POLL

  return MYSQL_INVALID_SOCKET;
}

Channel_info *Mysqld_socket_listener::listen_for_connection_event() {
#ifdef HAVE_POLL
  int retval = poll(&m_poll_info.m_fds[0], m_socket_map.size(), -1);
#else
  m_select_info.m_read_fds = m_select_info.m_client_fds;
  int retval = select((int)m_select_info.m_max_used_connection,
                      &m_select_info.m_read_fds, 0, 0, 0);
#endif

  if (retval < 0 && socket_errno != SOCKET_EINTR) {
    /*
      select(2)/poll(2) failed on the listening port.
      There is not much details to report about the client,
      increment the server global status variable.
    */
    ++connection_errors_select;
    if (!select_errors++ && !connection_events_loop_aborted())
      LogErr(ERROR_LEVEL, ER_CONN_SOCKET_SELECT_FAILED, socket_errno);
  }

  if (retval < 0 || connection_events_loop_aborted()) return nullptr;

  /* Is this a new connection request ? */
  bool is_unix_socket = false, is_admin_sock;
  MYSQL_SOCKET listen_sock = get_ready_socket(&is_unix_socket, &is_admin_sock);
  /*
    When poll/select returns control flow then at least one ready server socket
    must exist. Check that get_ready_socket() returns a valid socket.
  */
  DBUG_ASSERT(mysql_socket_getfd(listen_sock) != INVALID_SOCKET);

  MYSQL_SOCKET connect_sock;
#ifdef HAVE_SETNS
  /*
    If a network namespace is specified for a listening socket then set this
    network namespace as active before call to accept().
    It is not clear from manuals whether a socket returned by a call to
    accept() borrows a network namespace from a server socket used for
    accepting a new connection. For that reason, assign a network namespace
    explicitly before calling accept().
  */
  std::string network_namespace_for_listening_socket;
  if (!is_unix_socket) {
    network_namespace_for_listening_socket =
        m_socket_map.at(listen_sock).m_network_namespace;
    if (!network_namespace_for_listening_socket.empty() &&
        set_network_namespace(network_namespace_for_listening_socket))
      return nullptr;
  }
#endif
  if (accept_connection(listen_sock, &connect_sock)) {
#ifdef HAVE_SETNS
    if (!network_namespace_for_listening_socket.empty())
      (void)restore_original_network_namespace();
#endif
    return nullptr;
  }

#ifdef HAVE_SETNS
  if (!network_namespace_for_listening_socket.empty() &&
      restore_original_network_namespace())
    return nullptr;
#endif

#ifdef HAVE_LIBWRAP
  if (!is_unix_socket &&
      check_connection_refused_by_tcp_wrapper(connect_sock)) {
    return nullptr;
  }
#endif  // HAVE_LIBWRAP

  Channel_info *channel_info = nullptr;
  if (is_unix_socket)
    channel_info = new (std::nothrow) Channel_info_local_socket(connect_sock);
  else
    channel_info = new (std::nothrow)
        Channel_info_tcpip_socket(connect_sock, is_admin_sock);
  if (channel_info == nullptr) {
    (void)mysql_socket_shutdown(connect_sock, SHUT_RDWR);
    (void)mysql_socket_close(connect_sock);
    connection_errors_internal++;
    return nullptr;
  }

#ifdef HAVE_SETNS
  if (!is_unix_socket && !network_namespace_for_listening_socket.empty())
    static_cast<Channel_info_tcpip_socket *>(channel_info)
        ->set_network_namespace(network_namespace_for_listening_socket);
#endif
  return channel_info;
}

void Mysqld_socket_listener::close_listener() {
  for (socket_map_const_iterator_t sock_map_const_iter = m_socket_map.cbegin();
       sock_map_const_iter != m_socket_map.cend(); ++sock_map_const_iter) {
    (void)mysql_socket_shutdown(sock_map_const_iter->first, SHUT_RDWR);
    (void)mysql_socket_close(sock_map_const_iter->first);
  }

  /*
    In case a separate thread was spawned to handle incoming connection
    requests on admin interface, a socket corresponding to an admin interface
    being listened is not included in the m_socket_map. Instead, this socket
    referenced by the data member m_admin_interface_listen_socket.
  */
  if (m_use_separate_thread_for_admin) {
#ifdef _WIN32
    /*
      For Windows, first close the socket referenced by the data member
      m_admin_interface_listen_socket. It results in return from select()
      API call running from a separate thread.
    */
    (void)mysql_socket_close(m_admin_interface_listen_socket);
    my_thread_join(&admin_socket_thread_id, nullptr);
#else
    // First, finish listening thread.
    pthread_kill(admin_socket_thread_id.thread, SIGALRM);
    my_thread_join(&admin_socket_thread_id, nullptr);
    /*
      After a thread listening on admin interface finished, it is safe
      to close listening socket.
    */
    (void)mysql_socket_close(m_admin_interface_listen_socket);
#endif

    mysql_mutex_destroy(&LOCK_start_admin_thread);
    mysql_cond_destroy(&COND_start_admin_thread);
  }

#if defined(HAVE_SYS_UN_H)
  if (m_unix_sockname != "" && m_unlink_sockname) {
    std::string lock_filename = m_unix_sockname + ".lock";
    (void)unlink(lock_filename.c_str());
    (void)unlink(m_unix_sockname.c_str());
  }
#endif

#ifdef HAVE_SETNS
  release_network_namespace_resources();
#endif

  m_socket_map.clear();
  m_bind_addresses.clear();
}
