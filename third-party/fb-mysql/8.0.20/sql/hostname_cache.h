/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef HOSTNAME_CACHE_INCLUDED
#define HOSTNAME_CACHE_INCLUDED

#include "my_config.h"

#include <sys/types.h>
#include <list>
#include <memory>

#include "my_hostname.h"
#include "my_inttypes.h"
#include "mysql_com.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

struct Host_errors {
 public:
  Host_errors();
  ~Host_errors();

  void reset();
  void aggregate(const Host_errors *errors);

  /** Number of connect errors. */
  ulong m_connect;

  /** Number of host blocked errors. */
  ulong m_host_blocked;
  /** Number of transient errors from getnameinfo(). */
  ulong m_nameinfo_transient;
  /** Number of permanent errors from getnameinfo(). */
  ulong m_nameinfo_permanent;
  /** Number of errors from is_hostname_valid(). */
  ulong m_format;
  /** Number of transient errors from getaddrinfo(). */
  ulong m_addrinfo_transient;
  /** Number of permanent errors from getaddrinfo(). */
  ulong m_addrinfo_permanent;
  /** Number of errors from Forward-Confirmed reverse DNS checks. */
  ulong m_FCrDNS;
  /** Number of errors from host grants. */
  ulong m_host_acl;
  /** Number of errors from missing auth plugin. */
  ulong m_no_auth_plugin;
  /** Number of errors from auth plugin. */
  ulong m_auth_plugin;
  /** Number of errors from authentication plugins. */
  ulong m_handshake;
  /** Number of errors from proxy user. */
  ulong m_proxy_user;
  /** Number of errors from proxy user acl. */
  ulong m_proxy_user_acl;
  /** Number of errors from authentication. */
  ulong m_authentication;
  /** Number of errors from ssl. */
  ulong m_ssl;
  /** Number of errors from max user connection. */
  ulong m_max_user_connection;
  /** Number of errors from max user connection per hour. */
  ulong m_max_user_connection_per_hour;
  /** Number of errors from the default database. */
  ulong m_default_database;
  /** Number of errors from init_connect. */
  ulong m_init_connect;
  /** Number of errors from the server itself. */
  ulong m_local;

  bool has_error() const {
    return (
        (m_host_blocked != 0) || (m_nameinfo_transient != 0) ||
        (m_nameinfo_permanent != 0) || (m_format != 0) ||
        (m_addrinfo_transient != 0) || (m_addrinfo_permanent != 0) ||
        (m_FCrDNS != 0) || (m_host_acl != 0) || (m_no_auth_plugin != 0) ||
        (m_auth_plugin != 0) || (m_handshake != 0) || (m_proxy_user != 0) ||
        (m_proxy_user_acl != 0) || (m_authentication != 0) || (m_ssl != 0) ||
        (m_max_user_connection != 0) || (m_max_user_connection_per_hour != 0) ||
        (m_default_database != 0) || (m_init_connect != 0) || (m_local != 0));
  }

  void sum_connect_errors() {
    /* Current (historical) behavior: */
    m_connect = m_handshake;
  }

  void clear_connect_errors() { m_connect = 0; }
};

/** Size of IP address string in the hash cache. */
#define HOST_ENTRY_KEY_SIZE INET6_ADDRSTRLEN

/**
  An entry in the hostname hash table cache.

  Host name cache does two things:
    - caches host names to save DNS look ups;
    - counts errors from IP.

  Host name can be empty (that means DNS look up failed),
  but errors still are counted.
*/
class Host_entry {
 public:
  /**
    Client IP address. This is the key used with the hash table.

    The client IP address is always expressed in IPv6, even when the
    network IPv6 stack is not present.

    This IP address is never used to connect to a socket.
  */
  char ip_key[HOST_ENTRY_KEY_SIZE];

  /**
    One of the host names for the IP address. May be a zero length string.
  */
  char m_hostname[HOSTNAME_LENGTH + 1];
  /** Length in bytes of @c m_hostname. */
  uint m_hostname_length;
  /** The hostname is validated and used for authorization. */
  bool m_host_validated;
  ulonglong m_first_seen;
  ulonglong m_last_seen;
  ulonglong m_first_error_seen;
  ulonglong m_last_error_seen;
  /** Error statistics. */
  Host_errors m_errors;

  void set_error_timestamps(ulonglong now) {
    if (m_first_error_seen == 0) m_first_error_seen = now;
    m_last_error_seen = now;
  }
};

#define RC_OK 0
#define RC_BLOCKED_HOST 1
int ip_to_hostname(struct sockaddr_storage *ip_storage, const char *ip_string,
                   char **hostname, uint *connect_errors);

void inc_host_errors(const char *ip_string, Host_errors *errors);
void reset_host_connect_errors(const char *ip_string);
bool hostname_cache_init(uint size);
void hostname_cache_free();
void hostname_cache_refresh(void);
uint hostname_cache_size();
void hostname_cache_resize(uint size);
void hostname_cache_lock();
void hostname_cache_unlock();
std::list<std::unique_ptr<Host_entry>>::iterator hostname_cache_begin();
std::list<std::unique_ptr<Host_entry>>::iterator hostname_cache_end();

#endif /* HOSTNAME_CACHE_INCLUDED */
