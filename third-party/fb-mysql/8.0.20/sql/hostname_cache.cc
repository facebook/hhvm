/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file

  @brief
  Get hostname for an IP address.

  Hostnames are checked with reverse name lookup and checked that they
  doesn't resemble an IP address.
*/

#include "sql/hostname_cache.h"

#include "my_config.h"

#include "map_helpers.h"
#include "mutex_lock.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_systime.h"  // my_micro_time()
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "thr_mutex.h"

#ifndef _WIN32
#include <netdb.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <list>
#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/log.h"
#include "sql/mysqld.h"  // specialflag
#include "sql/psi_memory_key.h"
#include "violite.h"  // vio_getnameinfo,

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_PSI_INTERFACE
extern PSI_mutex_key key_hash_filo_lock;
#endif  // HAVE_PSI_INTERFACE

using std::list;
using std::string;
using std::unique_ptr;
using std::unordered_map;

Host_errors::Host_errors()
    : m_connect(0),
      m_host_blocked(0),
      m_nameinfo_transient(0),
      m_nameinfo_permanent(0),
      m_format(0),
      m_addrinfo_transient(0),
      m_addrinfo_permanent(0),
      m_FCrDNS(0),
      m_host_acl(0),
      m_no_auth_plugin(0),
      m_auth_plugin(0),
      m_handshake(0),
      m_proxy_user(0),
      m_proxy_user_acl(0),
      m_authentication(0),
      m_ssl(0),
      m_max_user_connection(0),
      m_max_user_connection_per_hour(0),
      m_default_database(0),
      m_init_connect(0),
      m_local(0) {}

Host_errors::~Host_errors() {}

void Host_errors::reset() {
  m_connect = 0;
  m_host_blocked = 0;
  m_nameinfo_transient = 0;
  m_nameinfo_permanent = 0;
  m_format = 0;
  m_addrinfo_transient = 0;
  m_addrinfo_permanent = 0;
  m_FCrDNS = 0;
  m_host_acl = 0;
  m_no_auth_plugin = 0;
  m_auth_plugin = 0;
  m_handshake = 0;
  m_proxy_user = 0;
  m_proxy_user_acl = 0;
  m_authentication = 0;
  m_ssl = 0;
  m_max_user_connection = 0;
  m_max_user_connection_per_hour = 0;
  m_default_database = 0;
  m_init_connect = 0;
  m_local = 0;
}

void Host_errors::aggregate(const Host_errors *errors) {
  m_connect += errors->m_connect;
  m_host_blocked += errors->m_host_blocked;
  m_nameinfo_transient += errors->m_nameinfo_transient;
  m_nameinfo_permanent += errors->m_nameinfo_permanent;
  m_format += errors->m_format;
  m_addrinfo_transient += errors->m_addrinfo_transient;
  m_addrinfo_permanent += errors->m_addrinfo_permanent;
  m_FCrDNS += errors->m_FCrDNS;
  m_host_acl += errors->m_host_acl;
  m_no_auth_plugin += errors->m_no_auth_plugin;
  m_auth_plugin += errors->m_auth_plugin;
  m_handshake += errors->m_handshake;
  m_proxy_user += errors->m_proxy_user;
  m_proxy_user_acl += errors->m_proxy_user_acl;
  m_authentication += errors->m_authentication;
  m_ssl += errors->m_ssl;
  m_max_user_connection += errors->m_max_user_connection;
  m_max_user_connection_per_hour += errors->m_max_user_connection_per_hour;
  m_default_database += errors->m_default_database;
  m_init_connect += errors->m_init_connect;
  m_local += errors->m_local;
}

static size_t hostname_cache_max_size;
static list<unique_ptr<Host_entry>> *hostname_cache_lru;
static malloc_unordered_map<string, list<unique_ptr<Host_entry>>::iterator>
    *hostname_cache_by_ip;
static mysql_mutex_t hostname_cache_mutex;
static bool hostname_cache_mutex_inited = false;

void hostname_cache_refresh() {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);
  hostname_cache_by_ip->clear();
  hostname_cache_lru->clear();
}

uint hostname_cache_size() {
  mysql_mutex_assert_owner(&hostname_cache_mutex);
  return hostname_cache_max_size;
}

void hostname_cache_resize(uint size) {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);
  hostname_cache_by_ip->clear();
  hostname_cache_lru->clear();
  hostname_cache_max_size = size;
}

bool hostname_cache_init(uint size) {
  hostname_cache_by_ip =
      new malloc_unordered_map<string, list<unique_ptr<Host_entry>>::iterator>(
          key_memory_host_cache_hostname);
  hostname_cache_lru = new list<unique_ptr<Host_entry>>();

  hostname_cache_max_size = size;
  mysql_mutex_init(key_hash_filo_lock, &hostname_cache_mutex,
                   MY_MUTEX_INIT_FAST);
  hostname_cache_mutex_inited = true;

  return false;
}

void hostname_cache_free() {
  delete hostname_cache_by_ip;
  hostname_cache_by_ip = nullptr;
  delete hostname_cache_lru;
  hostname_cache_lru = nullptr;

  if (hostname_cache_mutex_inited) {
    mysql_mutex_destroy(&hostname_cache_mutex);
    hostname_cache_mutex_inited = false;
  }
}

void hostname_cache_lock() {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);
  mysql_mutex_lock(&hostname_cache_mutex);
}

void hostname_cache_unlock() {
  mysql_mutex_assert_owner(&hostname_cache_mutex);
  mysql_mutex_unlock(&hostname_cache_mutex);
}

list<unique_ptr<Host_entry>>::iterator hostname_cache_begin() {
  mysql_mutex_assert_owner(&hostname_cache_mutex);
  return hostname_cache_lru->begin();
}

list<unique_ptr<Host_entry>>::iterator hostname_cache_end() {
  mysql_mutex_assert_owner(&hostname_cache_mutex);

  return hostname_cache_lru->end();
}

static inline Host_entry *hostname_cache_search(const char *ip_string) {
  mysql_mutex_assert_owner(&hostname_cache_mutex);

  auto it = hostname_cache_by_ip->find(ip_string);
  if (it == hostname_cache_by_ip->end()) return nullptr;

  // Move to the front of the LRU list.
  hostname_cache_lru->splice(hostname_cache_lru->begin(), *hostname_cache_lru,
                             it->second);
  return it->second->get();
}

static void add_hostname_impl(const char *ip_string, const char *hostname,
                              bool validated, Host_errors *errors,
                              ulonglong now) {
  Host_entry *entry;
  bool need_add = false;

  mysql_mutex_assert_owner(&hostname_cache_mutex);

  entry = hostname_cache_search(ip_string);

  if (likely(entry == nullptr)) {
    entry = new (std::nothrow) Host_entry;
    if (entry == nullptr) return;

    need_add = true;
    strcpy(entry->ip_key, ip_string);
    entry->m_errors.reset();
    entry->m_hostname_length = 0;
    entry->m_host_validated = false;
    entry->m_first_seen = now;
    entry->m_last_seen = now;
    entry->m_first_error_seen = 0;
    entry->m_last_error_seen = 0;
  } else {
    entry->m_last_seen = now;
  }

  if (validated) {
    if (hostname != nullptr) {
      size_t len = strlen(hostname);
      if (len > sizeof(entry->m_hostname) - 1)
        len = sizeof(entry->m_hostname) - 1;
      memcpy(entry->m_hostname, hostname, len);
      entry->m_hostname[len] = '\0';
      entry->m_hostname_length = static_cast<uint>(len);

      DBUG_PRINT(
          "info",
          ("Adding/Updating '%s' -> '%s' (validated) to the hostname cache...'",
           ip_string, entry->m_hostname));
    } else {
      entry->m_hostname_length = 0;
      DBUG_PRINT(
          "info",
          ("Adding/Updating '%s' -> NULL (validated) to the hostname cache...'",
           ip_string));
    }
    entry->m_host_validated = true;
    /*
      New errors that are considered 'blocking',
      that will eventually cause the IP to be black listed and blocked.
    */
    errors->sum_connect_errors();
  } else {
    entry->m_hostname_length = 0;
    entry->m_host_validated = false;
    /* Do not count new blocking errors during DNS failures. */
    errors->clear_connect_errors();
    DBUG_PRINT("info", ("Adding/Updating '%s' -> NULL (not validated) to the "
                        "hostname cache...'",
                        ip_string));
  }

  if (errors->has_error()) entry->set_error_timestamps(now);

  entry->m_errors.aggregate(errors);

  if (need_add) {
    if (hostname_cache_lru->size() >= hostname_cache_max_size) {
      hostname_cache_by_ip->erase(hostname_cache_lru->front()->ip_key);
      hostname_cache_lru->pop_front();
    }
    DBUG_ASSERT(hostname_cache_lru->size() < hostname_cache_max_size);
    hostname_cache_lru->emplace_front(entry);
    hostname_cache_by_ip->emplace(entry->ip_key, hostname_cache_lru->begin());
  }

  return;
}

static void add_hostname(const char *ip_string, const char *hostname,
                         bool validated, Host_errors *errors) {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  if (specialflag & SPECIAL_NO_HOST_CACHE) return;

  ulonglong now = my_micro_time();

  MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);
  if (hostname_cache_size() != 0)
    add_hostname_impl(ip_string, hostname, validated, errors, now);

  return;
}

void inc_host_errors(const char *ip_string, Host_errors *errors) {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  if (!ip_string) return;

  ulonglong now = my_micro_time();

  MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);

  Host_entry *entry = hostname_cache_search(ip_string);

  if (entry) {
    if (entry->m_host_validated)
      errors->sum_connect_errors();
    else
      errors->clear_connect_errors();

    entry->m_errors.aggregate(errors);
    entry->set_error_timestamps(now);
  }
}

void reset_host_connect_errors(const char *ip_string) {
  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  if (!ip_string) return;

  MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);

  Host_entry *entry = hostname_cache_search(ip_string);

  if (entry) entry->m_errors.clear_connect_errors();
}

static inline bool is_ip_loopback(const struct sockaddr *ip) {
  switch (ip->sa_family) {
    case AF_INET: {
      /* Check for IPv4 127.0.0.1. */
      const struct in_addr *ip4 =
          &(pointer_cast<const struct sockaddr_in *>(ip))->sin_addr;
      return ntohl(ip4->s_addr) == INADDR_LOOPBACK;
    }

    case AF_INET6: {
      /* Check for IPv6 ::1. */
      const struct in6_addr *ip6 =
          &(pointer_cast<const struct sockaddr_in6 *>(ip))->sin6_addr;
      return IN6_IS_ADDR_LOOPBACK(ip6);
    }

    default:
      return false;
  }
}

static inline bool is_hostname_valid(const char *hostname) {
  /*
    A hostname is invalid if it starts with a number followed by a dot
    (IPv4 address).
  */

  if (!my_isdigit(&my_charset_latin1, hostname[0])) return true;

  const char *p = hostname + 1;

  while (my_isdigit(&my_charset_latin1, *p)) ++p;

  return *p != '.';
}

/**
  Resolve IP-address to host name.

  This function does the following things:
    - resolves IP-address;
    - employs Forward Confirmed Reverse DNS technique to validate IP-address;
    - returns host name if IP-address is validated;
    - set value to out-variable connect_errors -- this variable represents the
      number of connection errors from the specified IP-address.
    - update the host_cache statistics

  NOTE: connect_errors are counted (are supported) only for the clients
  where IP-address can be resolved and FCrDNS check is passed.

  @param [in]  ip_storage IP address (sockaddr). Must be set.
  @param [in]  ip_string  IP address (string). Must be set.
  @param [out] hostname   Hostname if IP-address is valid.
  @param [out] connect_errors  Represents number of connection errors.

  @return Error status
  @retval 0 Success
  @retval RC_BLOCKED_HOST The host is blocked.

  The function does not set/report MySQL server error in case of failure.
  It's caller's responsibility to handle failures of this function
  properly.
*/

int ip_to_hostname(struct sockaddr_storage *ip_storage, const char *ip_string,
                   char **hostname, uint *connect_errors) {
  const struct sockaddr *ip = (const sockaddr *)ip_storage;
  int err_code;
  Host_errors errors;

  mysql_mutex_assert_not_owner(&hostname_cache_mutex);

  DBUG_TRACE;
  DBUG_PRINT("info",
             ("IP address: '%s'; family: %d.", ip_string, (int)ip->sa_family));

  /* Default output values, for most cases. */
  *hostname = nullptr;
  *connect_errors = 0;

  /* Check if we have loopback address (127.0.0.1 or ::1). */

  if (is_ip_loopback(ip)) {
    DBUG_PRINT("info", ("Loopback address detected."));

    /* Do not count connect errors from localhost. */
    *hostname = const_cast<char *>(my_localhost);

    return 0;
  }

  /* Check first if we have host name in the cache. */

  if (!(specialflag & SPECIAL_NO_HOST_CACHE)) {
    ulonglong now = my_micro_time();

    MUTEX_LOCK(hostname_lock, &hostname_cache_mutex);

    Host_entry *entry = hostname_cache_search(ip_string);

    if (entry) {
      entry->m_last_seen = now;
      *connect_errors = entry->m_errors.m_connect;

      if (entry->m_errors.m_connect >= max_connect_errors) {
        entry->m_errors.m_host_blocked++;
        entry->set_error_timestamps(now);
        return RC_BLOCKED_HOST;
      }

      /*
        If there is an IP -> HOSTNAME association in the cache,
        but for a hostname that was not validated,
        do not return that hostname: perform the network validation again.
      */
      if (entry->m_host_validated) {
        if (entry->m_hostname_length)
          *hostname = my_strdup(key_memory_host_cache_hostname,
                                entry->m_hostname, MYF(0));

        DBUG_PRINT("info", ("IP (%s) has been found in the cache. "
                            "Hostname: '%s'",
                            ip_string, (*hostname ? *hostname : "null")));

        return 0;
      }
    }
  }

  /*
    Resolve host name. Return an error if a host name can not be resolved
    (instead of returning the numeric form of the host name).
  */

  char hostname_buffer[NI_MAXHOST];

  DBUG_PRINT("info", ("Resolving '%s'...", (const char *)ip_string));

  err_code =
      vio_getnameinfo(ip, hostname_buffer, NI_MAXHOST, nullptr, 0, NI_NAMEREQD);

  /*
  ===========================================================================
  DEBUG code only (begin)
  Simulate various output from vio_getnameinfo().
  ===========================================================================
  */

  DBUG_EXECUTE_IF("getnameinfo_error_noname", {
    strcpy(hostname_buffer, "<garbage>");
    err_code = EAI_NONAME;
  });

  DBUG_EXECUTE_IF("getnameinfo_error_again", {
    strcpy(hostname_buffer, "<garbage>");
    err_code = EAI_AGAIN;
  });

  DBUG_EXECUTE_IF("getnameinfo_fake_ipv4", {
    strcpy(hostname_buffer, "santa.claus.ipv4.example.com");
    err_code = 0;
  });

  DBUG_EXECUTE_IF("getnameinfo_fake_ipv6", {
    strcpy(hostname_buffer, "santa.claus.ipv6.example.com");
    err_code = 0;
  });

  DBUG_EXECUTE_IF("getnameinfo_format_ipv4", {
    strcpy(hostname_buffer, "12.12.12.12");
    err_code = 0;
  });

  DBUG_EXECUTE_IF("getnameinfo_format_ipv6", {
    strcpy(hostname_buffer, "12:DEAD:BEEF:0");
    err_code = 0;
  });

  DBUG_EXECUTE_IF("getnameinfo_fake_max_length", {
    std::string s(NI_MAXHOST - 1, 'a');
    strcpy(hostname_buffer, s.c_str());
    err_code = 0;
  });

  /*
  ===========================================================================
  DEBUG code only (end)
  ===========================================================================
  */

  if (err_code) {
    // NOTE: gai_strerror() returns a string ending by a dot.

    DBUG_PRINT("error", ("IP address '%s' could not be resolved: %s", ip_string,
                         gai_strerror(err_code)));

    LogErr(WARNING_LEVEL, ER_UNABLE_TO_RESOLVE_IP, ip_string,
           gai_strerror(err_code));

    bool validated;
    if (vio_is_no_name_error(err_code)) {
      /*
        The no-name error means that there is no reverse address mapping
        for the IP address. A host name can not be resolved.
      */
      errors.m_nameinfo_permanent = 1;
      validated = true;
    } else {
      /*
        If it is not the no-name error, we should not cache the hostname
        (or rather its absence), because the failure might be transient.
        Only the ip error statistics are cached.
      */
      errors.m_nameinfo_transient = 1;
      validated = false;
    }
    add_hostname(ip_string, nullptr, validated, &errors);

    return 0;
  }

  DBUG_PRINT("info", ("IP '%s' resolved to '%s'.", (const char *)ip_string,
                      (const char *)hostname_buffer));

  /*
    Validate hostname: the server does not accept host names, which
    resemble IP addresses.

    The thing is that theoretically, a host name can be in a form of IPv4
    address (123.example.org, or 1.2 or even 1.2.3.4). We have to deny such
    host names because ACL-systems is not designed to work with them.

    For example, it is possible to specify a host name mask (like
    192.168.1.%) for an ACL rule. Then, if IPv4-like hostnames are allowed,
    there is a security hole: instead of allowing access for
    192.168.1.0/255 network (which was assumed by the user), the access
    will be allowed for host names like 192.168.1.example.org.
  */

  if (!is_hostname_valid(hostname_buffer)) {
    DBUG_PRINT("error", ("IP address '%s' has been resolved "
                         "to the host name '%s', which resembles "
                         "IPv4-address itself.",
                         ip_string, hostname_buffer));

    LogErr(WARNING_LEVEL, ER_HOSTNAME_RESEMBLES_IPV4, ip_string,
           hostname_buffer);

    errors.m_format = 1;
    add_hostname(ip_string, hostname_buffer, false, &errors);

    return false;
  }

  /* Get IP-addresses for the resolved host name (FCrDNS technique). */

  struct addrinfo hints;
  struct addrinfo *addr_info_list;
  /*
    Makes fault injection with DBUG_EXECUTE_IF easier.
    Invoking free_addr_info(NULL) crashes on some platforms.
  */
  bool free_addr_info_list = false;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;

  DBUG_PRINT("info",
             ("Getting IP addresses for hostname '%s'...", hostname_buffer));

  err_code = getaddrinfo(hostname_buffer, nullptr, &hints, &addr_info_list);
  if (err_code == 0) free_addr_info_list = true;

  /*
  ===========================================================================
  DEBUG code only (begin)
  Simulate various output from getaddrinfo().
  ===========================================================================
  */
  DBUG_EXECUTE_IF("getaddrinfo_error_noname", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    addr_info_list = nullptr;
    err_code = EAI_NONAME;
    free_addr_info_list = false;
  });

  DBUG_EXECUTE_IF("getaddrinfo_error_again", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    addr_info_list = nullptr;
    err_code = EAI_AGAIN;
    free_addr_info_list = false;
  });

  DBUG_EXECUTE_IF("getaddrinfo_fake_bad_ipv4", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    struct sockaddr_in *debug_addr;
    /*
      Not thread safe, which is ok.
      Only one connection at a time is tested with
      fault injection.
    */
    static struct sockaddr_in debug_sock_addr[2];
    static struct addrinfo debug_addr_info[2];
    /* Simulating ipv4 192.0.2.126 */
    debug_addr = &debug_sock_addr[0];
    debug_addr->sin_family = AF_INET;
    debug_addr->sin_addr.s_addr = inet_addr("192.0.2.126");

    /* Simulating ipv4 192.0.2.127 */
    debug_addr = &debug_sock_addr[1];
    debug_addr->sin_family = AF_INET;
    debug_addr->sin_addr.s_addr = inet_addr("192.0.2.127");

    debug_addr_info[0].ai_addr = (struct sockaddr *)&debug_sock_addr[0];
    debug_addr_info[0].ai_addrlen = sizeof(struct sockaddr_in);
    debug_addr_info[0].ai_next = &debug_addr_info[1];

    debug_addr_info[1].ai_addr = (struct sockaddr *)&debug_sock_addr[1];
    debug_addr_info[1].ai_addrlen = sizeof(struct sockaddr_in);
    debug_addr_info[1].ai_next = nullptr;

    addr_info_list = &debug_addr_info[0];
    err_code = 0;
    free_addr_info_list = false;
  });

  DBUG_EXECUTE_IF("getaddrinfo_fake_good_ipv4", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    struct sockaddr_in *debug_addr;
    static struct sockaddr_in debug_sock_addr[2];
    static struct addrinfo debug_addr_info[2];
    /* Simulating ipv4 192.0.2.5 */
    debug_addr = &debug_sock_addr[0];
    debug_addr->sin_family = AF_INET;
    debug_addr->sin_addr.s_addr = inet_addr("192.0.2.5");

    /* Simulating ipv4 192.0.2.4 */
    debug_addr = &debug_sock_addr[1];
    debug_addr->sin_family = AF_INET;
    debug_addr->sin_addr.s_addr = inet_addr("192.0.2.4");

    debug_addr_info[0].ai_addr = (struct sockaddr *)&debug_sock_addr[0];
    debug_addr_info[0].ai_addrlen = sizeof(struct sockaddr_in);
    debug_addr_info[0].ai_next = &debug_addr_info[1];

    debug_addr_info[1].ai_addr = (struct sockaddr *)&debug_sock_addr[1];
    debug_addr_info[1].ai_addrlen = sizeof(struct sockaddr_in);
    debug_addr_info[1].ai_next = nullptr;

    addr_info_list = &debug_addr_info[0];
    err_code = 0;
    free_addr_info_list = false;
  });

  DBUG_EXECUTE_IF("getaddrinfo_fake_bad_ipv6", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    struct sockaddr_in6 *debug_addr;
    struct in6_addr *ip6;
    /*
      Not thread safe, which is ok.
      Only one connection at a time is tested with
      fault injection.
    */
    static struct sockaddr_in6 debug_sock_addr[2];
    static struct addrinfo debug_addr_info[2];
    /* Simulating ipv6 2001:DB8::6:7E */
    debug_addr = &debug_sock_addr[0];
    debug_addr->sin6_family = AF_INET6;
    ip6 = &debug_addr->sin6_addr;
    /* inet_pton not available on Windows XP. */
    ip6->s6_addr[0] = 0x20;
    ip6->s6_addr[1] = 0x01;
    ip6->s6_addr[2] = 0x0d;
    ip6->s6_addr[3] = 0xb8;
    ip6->s6_addr[4] = 0x00;
    ip6->s6_addr[5] = 0x00;
    ip6->s6_addr[6] = 0x00;
    ip6->s6_addr[7] = 0x00;
    ip6->s6_addr[8] = 0x00;
    ip6->s6_addr[9] = 0x00;
    ip6->s6_addr[10] = 0x00;
    ip6->s6_addr[11] = 0x00;
    ip6->s6_addr[12] = 0x00;
    ip6->s6_addr[13] = 0x06;
    ip6->s6_addr[14] = 0x00;
    ip6->s6_addr[15] = 0x7e;

    /* Simulating ipv6 2001:DB8::6:7F */
    debug_addr = &debug_sock_addr[1];
    debug_addr->sin6_family = AF_INET6;
    ip6 = &debug_addr->sin6_addr;
    ip6->s6_addr[0] = 0x20;
    ip6->s6_addr[1] = 0x01;
    ip6->s6_addr[2] = 0x0d;
    ip6->s6_addr[3] = 0xb8;
    ip6->s6_addr[4] = 0x00;
    ip6->s6_addr[5] = 0x00;
    ip6->s6_addr[6] = 0x00;
    ip6->s6_addr[7] = 0x00;
    ip6->s6_addr[8] = 0x00;
    ip6->s6_addr[9] = 0x00;
    ip6->s6_addr[10] = 0x00;
    ip6->s6_addr[11] = 0x00;
    ip6->s6_addr[12] = 0x00;
    ip6->s6_addr[13] = 0x06;
    ip6->s6_addr[14] = 0x00;
    ip6->s6_addr[15] = 0x7f;

    debug_addr_info[0].ai_addr = (struct sockaddr *)&debug_sock_addr[0];
    debug_addr_info[0].ai_addrlen = sizeof(struct sockaddr_in6);
    debug_addr_info[0].ai_next = &debug_addr_info[1];

    debug_addr_info[1].ai_addr = (struct sockaddr *)&debug_sock_addr[1];
    debug_addr_info[1].ai_addrlen = sizeof(struct sockaddr_in6);
    debug_addr_info[1].ai_next = nullptr;

    addr_info_list = &debug_addr_info[0];
    err_code = 0;
    free_addr_info_list = false;
  });

  DBUG_EXECUTE_IF("getaddrinfo_fake_good_ipv6", {
    if (free_addr_info_list) freeaddrinfo(addr_info_list);

    struct sockaddr_in6 *debug_addr;
    struct in6_addr *ip6;
    /*
      Not thread safe, which is ok.
      Only one connection at a time is tested with
      fault injection.
    */
    static struct sockaddr_in6 debug_sock_addr[2];
    static struct addrinfo debug_addr_info[2];
    /* Simulating ipv6 2001:DB8::6:7 */
    debug_addr = &debug_sock_addr[0];
    debug_addr->sin6_family = AF_INET6;
    ip6 = &debug_addr->sin6_addr;
    ip6->s6_addr[0] = 0x20;
    ip6->s6_addr[1] = 0x01;
    ip6->s6_addr[2] = 0x0d;
    ip6->s6_addr[3] = 0xb8;
    ip6->s6_addr[4] = 0x00;
    ip6->s6_addr[5] = 0x00;
    ip6->s6_addr[6] = 0x00;
    ip6->s6_addr[7] = 0x00;
    ip6->s6_addr[8] = 0x00;
    ip6->s6_addr[9] = 0x00;
    ip6->s6_addr[10] = 0x00;
    ip6->s6_addr[11] = 0x00;
    ip6->s6_addr[12] = 0x00;
    ip6->s6_addr[13] = 0x06;
    ip6->s6_addr[14] = 0x00;
    ip6->s6_addr[15] = 0x07;

    /* Simulating ipv6 2001:DB8::6:6 */
    debug_addr = &debug_sock_addr[1];
    debug_addr->sin6_family = AF_INET6;
    ip6 = &debug_addr->sin6_addr;
    ip6->s6_addr[0] = 0x20;
    ip6->s6_addr[1] = 0x01;
    ip6->s6_addr[2] = 0x0d;
    ip6->s6_addr[3] = 0xb8;
    ip6->s6_addr[4] = 0x00;
    ip6->s6_addr[5] = 0x00;
    ip6->s6_addr[6] = 0x00;
    ip6->s6_addr[7] = 0x00;
    ip6->s6_addr[8] = 0x00;
    ip6->s6_addr[9] = 0x00;
    ip6->s6_addr[10] = 0x00;
    ip6->s6_addr[11] = 0x00;
    ip6->s6_addr[12] = 0x00;
    ip6->s6_addr[13] = 0x06;
    ip6->s6_addr[14] = 0x00;
    ip6->s6_addr[15] = 0x06;

    debug_addr_info[0].ai_addr = (struct sockaddr *)&debug_sock_addr[0];
    debug_addr_info[0].ai_addrlen = sizeof(struct sockaddr_in6);
    debug_addr_info[0].ai_next = &debug_addr_info[1];

    debug_addr_info[1].ai_addr = (struct sockaddr *)&debug_sock_addr[1];
    debug_addr_info[1].ai_addrlen = sizeof(struct sockaddr_in6);
    debug_addr_info[1].ai_next = nullptr;

    addr_info_list = &debug_addr_info[0];
    err_code = 0;
    free_addr_info_list = false;
  });

  /*
  ===========================================================================
  DEBUG code only (end)
  ===========================================================================
  */

  if (err_code != 0) {
    LogErr(WARNING_LEVEL, ER_UNABLE_TO_RESOLVE_HOSTNAME, hostname_buffer,
           gai_strerror(err_code));

    bool validated;

    if (err_code == EAI_NONAME) {
      errors.m_addrinfo_permanent = 1;
      validated = true;
    } else {
      /*
        Don't cache responses when the DNS server is down, as otherwise
        transient DNS failure may leave any number of clients (those
        that attempted to connect during the outage) unable to connect
        indefinitely.
        Only cache error statistics.
      */
      errors.m_addrinfo_transient = 1;
      validated = false;
    }
    add_hostname(ip_string, nullptr, validated, &errors);

    return false;
  }

  /* Check that getaddrinfo() returned the used IP (FCrDNS technique). */

  DBUG_PRINT("info",
             ("The following IP addresses found for '%s':", hostname_buffer));

  for (struct addrinfo *addr_info = addr_info_list; addr_info;
       addr_info = addr_info->ai_next) {
    char ip_buffer[HOST_ENTRY_KEY_SIZE];

    {
      bool err_status MY_ATTRIBUTE((unused));
      err_status = vio_get_normalized_ip_string(addr_info->ai_addr,
                                                addr_info->ai_addrlen,
                                                ip_buffer, sizeof(ip_buffer));
      DBUG_ASSERT(!err_status);
    }

    DBUG_PRINT("info", ("  - '%s'", ip_buffer));

    if (native_strcasecmp(ip_string, ip_buffer) == 0) {
      /* Copy host name string to be stored in the cache. */

      *hostname =
          my_strdup(key_memory_host_cache_hostname, hostname_buffer, MYF(0));

      if (!*hostname) {
        DBUG_PRINT("error", ("Out of memory."));

        if (free_addr_info_list) freeaddrinfo(addr_info_list);
        return true;
      }

      break;
    }
  }

  /* Log resolved IP-addresses if no match was found. */

  if (!*hostname) {
    errors.m_FCrDNS = 1;

    LogErr(WARNING_LEVEL, ER_HOSTNAME_DOESNT_RESOLVE_TO, hostname_buffer,
           ip_string);
    LogErr(INFORMATION_LEVEL, ER_ADDRESSES_FOR_HOSTNAME_HEADER,
           hostname_buffer);

    for (struct addrinfo *addr_info = addr_info_list; addr_info;
         addr_info = addr_info->ai_next) {
      char ip_buffer[HOST_ENTRY_KEY_SIZE];

#ifndef DBUG_OFF
      bool err_status =
#endif
          vio_get_normalized_ip_string(addr_info->ai_addr,
                                       addr_info->ai_addrlen, ip_buffer,
                                       sizeof(ip_buffer));
      DBUG_ASSERT(!err_status);

      LogErr(INFORMATION_LEVEL, ER_ADDRESSES_FOR_HOSTNAME_LIST_ITEM, ip_buffer);
    }
  }

  /* Add an entry for the IP to the cache. */
  add_hostname(ip_string, *hostname, true, &errors);

  /* Free the result of getaddrinfo(). */
  if (free_addr_info_list) freeaddrinfo(addr_info_list);

  return false;
}
