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

#include "sql/auth/sql_auth_cache.h"

#include <stdarg.h>
#include <boost/graph/properties.hpp>
#include <new>

#include <sql/ssl_acceptor_context.h>
#include "m_ctype.h"
#include "m_string.h"  // LEX_CSTRING
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/plugin.h"
#include "mysql/plugin_audit.h"
#include "mysql/plugin_auth.h"  // st_mysql_auth
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"    // ACL_internal_schema_access
#include "sql/auth/auth_internal.h"  // auth_plugin_is_built_in
#include "sql/auth/auth_utility.h"
#include "sql/auth/dynamic_privilege_table.h"
#include "sql/auth/sql_authentication.h"  // g_cached_authentication_plugins
#include "sql/auth/sql_security_ctx.h"
#include "sql/auth/sql_user_table.h"
#include "sql/auth/user_table.h"  // read_user_table
#include "sql/current_thd.h"      // current_thd
#include "sql/debug_sync.h"
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/field.h"          // Field
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"          // my_localhost
#include "sql/psi_memory_key.h"  // key_memory_acl_mem
#include "sql/records.h"         // unique_ptr_destroy_only<RowIterator>
#include "sql/row_iterator.h"
#include "sql/set_var.h"
#include "sql/sql_audit.h"
#include "sql/sql_base.h"   // open_and_lock_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_plugin.h"  // my_plugin_lock_by_name
#include "sql/sql_plugin_ref.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/tztime.h"  // Time_zone
#include "sql/xa.h"
#include "sql_string.h"
#include "thr_lock.h"
#include "thr_mutex.h"

#define INVALID_DATE "0000-00-00 00:00:00"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

using std::make_unique;
using std::min;
using std::move;
using std::string;
using std::unique_ptr;

PSI_mutex_key key_LOCK_acl_cache_flush;
PSI_mutex_info all_acl_cache_mutexes[] = {
    {&key_LOCK_acl_cache_flush, "LOCK_acl_cache_flush", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};
Acl_cache *g_acl_cache = nullptr;
Acl_cache *get_global_acl_cache() { return g_acl_cache; }
ulong get_global_acl_cache_size() { return g_acl_cache->size(); }
void init_acl_cache();
extern Role_index_map *g_authid_to_vertex;
extern Granted_roles_graph *g_granted_roles;
#include <boost/property_map/property_map.hpp>

struct ACL_internal_schema_registry_entry {
  const LEX_CSTRING *m_name;
  const ACL_internal_schema_access *m_access;
};
/**
  Internal schema registered.
  Currently, this is only:
  - performance_schema
  - information_schema,
  This can be reused later for:
  - mysql
*/
static ACL_internal_schema_registry_entry registry_array[2];
static uint m_registry_array_size = 0;

MEM_ROOT global_acl_memory;
MEM_ROOT memex;
Prealloced_array<ACL_USER, ACL_PREALLOC_SIZE> *acl_users = nullptr;
Prealloced_array<ACL_PROXY_USER, ACL_PREALLOC_SIZE> *acl_proxy_users = nullptr;
Prealloced_array<ACL_DB, ACL_PREALLOC_SIZE> *acl_dbs = nullptr;
Prealloced_array<ACL_HOST_AND_IP, ACL_PREALLOC_SIZE> *acl_wild_hosts = nullptr;
Db_access_map acl_db_map;
Default_roles *g_default_roles = nullptr;
std::vector<Role_id> *g_mandatory_roles = nullptr;

unique_ptr<
    malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_TABLE>>>
    column_priv_hash;
unique_ptr<
    malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_NAME>>>
    proc_priv_hash, func_priv_hash;
malloc_unordered_map<std::string, unique_ptr_my_free<acl_entry>> db_cache{
    key_memory_acl_cache};
collation_unordered_map<std::string, ACL_USER *> *acl_check_hosts = nullptr;
unique_ptr<Acl_restrictions> acl_restrictions = nullptr;

/**
  A hashmap on user part of account name for quick lookup.
*/
typedef std::unordered_map<
    std::string, Acl_user_ptr_list, std::hash<std::string>,
    std::equal_to<std::string>,
    Acl_cache_allocator<std::pair<const std::string, Acl_user_ptr_list>>>
    Name_to_userlist;
Name_to_userlist *name_to_userlist = nullptr;

bool initialized = false;
bool skip_grant_tables(void) { return !initialized; }
bool acl_cache_initialized = false;
bool allow_all_hosts = true;
uint grant_version = 0; /* Version of priv tables */
bool validate_user_plugins = true;

#define IP_ADDR_STRLEN (3 + 1 + 3 + 1 + 3 + 1 + 3)
#define ACL_KEY_LENGTH (IP_ADDR_STRLEN + 1 + NAME_LEN + 1 + USERNAME_LENGTH + 1)

/**
  Allocates the memory in the the global_acl_memory MEM_ROOT.
*/
void init_acl_memory() {
  init_sql_alloc(key_memory_acl_mem, &global_acl_memory, ACL_ALLOC_BLOCK_SIZE,
                 0);
}

struct acl_lookup_entry {
  /*
    List of hosts sorted in the order of specific -> less specific
    so that more specific ACL entry wins - see get_sort for details
   */
  Prealloced_array<ACL_ACCESS, ACL_PREALLOC_SIZE> *hosts;

  acl_lookup_entry() {
    hosts = new Prealloced_array<ACL_ACCESS, ACL_PREALLOC_SIZE>(
        key_memory_acl_cache);
  }

  ~acl_lookup_entry() {
    delete hosts;
    hosts = nullptr;
  }

  struct deleter {
    void operator()(acl_lookup_entry *entry) {
      entry->~acl_lookup_entry();
      my_free(entry);
    }
  };

  bool find(const char *host, const char *ip, ulong *access) {
    /*
      List of hosts sorted in the order of specific -> less specific
      so that more specific ACL entry wins - see get_sort for details
    */
    for (auto it = hosts->begin(); it < hosts->end(); ++it) {
      if (it->host.compare_hostname(host, ip)) {
        *access = it->access;
        return true;
      }
    }

    return false;
  }

  void update(ACL_DB *acl_db) {
    DBUG_ASSERT(acl_db != NULL);

    for (auto it = hosts->begin(); it < hosts->end(); ++it) {
      if (it != hosts->end() && it->host.is_same(&acl_db->host)) {
        it->access = acl_db->access;
#ifndef DBUG_OFF
        validate();
#endif
        return;
      }
    }
  }

  void remove(ACL_DB *acl_db) {
    DBUG_ASSERT(acl_db != NULL);

    for (auto it = hosts->begin(); it < hosts->end(); ++it) {
      if (it != hosts->end() && it->host.is_same(&acl_db->host)) {
        hosts->erase(it);
#ifndef DBUG_OFF
        validate();
#endif
        return;
      }
    }
  }

  void insert(ACL_DB *acl_db) {
    ACL_ACCESS new_entry;

    new_entry.host = acl_db->host;
    new_entry.access = acl_db->access;

    /* Generate sort key so that more specific host pattern wins */
    new_entry.sort = get_sort(1, acl_db->host.get_host());

    /* Find the location to insert in descending order */
    auto it = hosts->begin();
    for (; it < hosts->end(); ++it) {
      if (new_entry.sort > it->sort) break;
    }

    hosts->insert(it, new_entry);

#ifndef DBUG_OFF
    validate();
#endif
  }

#ifndef DBUG_OFF
  void validate() {
    if (hosts->empty()) {
      return;
    }
    auto it = hosts->begin() + 1;
    for (; it < hosts->end(); ++it) {
      DBUG_ASSERT((it - 1)->sort >= it->sort);
    }
    std::sort(hosts->begin(), hosts->end(), ACL_compare());
    it = hosts->begin() + 1;
    for (; it < hosts->end(); ++it) {
      DBUG_ASSERT((it - 1)->sort >= it->sort);
    }
  }
#endif
};

#define ACL_LOOKUP_KEY_LENGTH (NAME_LEN + 1 + USERNAME_LENGTH + 1)
#define ACL_LOOKUP_TBL_SIZE 1024

/*
  Simple ACL lookup table that only looks up exact (user, db) pairs and do a
  linear scan over the hosts. This is much faster than a linear scan over the
  entire acl_dbs table and in many cases there is only one item for each
  (user, db) pair anyway
 */
class acl_lookup_tbl {
 private:
  /* Build the key (user, db) */
  std::string build_key(ACL_DB *acl_db) {
    char key[ACL_LOOKUP_KEY_LENGTH], *end;
    end = my_stpcpy(my_stpcpy(key, acl_db->user ? acl_db->user : "") + 1,
                    acl_db->db);
    size_t key_length = (size_t)(end - key);
    DBUG_ASSERT(key_length <= sizeof(key));
    return std::string(key, key_length);
  }

  std::string build_key(const char *user, const char *db) {
    char key[ACL_LOOKUP_KEY_LENGTH], *end;
    end = my_stpcpy(my_stpcpy(key, user ? user : "") + 1, db);
    size_t key_length = (size_t)(end - key);
    DBUG_ASSERT(key_length <= sizeof(key));
    return std::string(key, key_length);
  }

 public:
  void insert(ACL_DB *acl_db) {
    /* We don't support wildcard in db in this lookup table */
    if (!acl_db->db || contains_wildcard(acl_db->db)) return;

    std::string cache_key = build_key(acl_db);
    auto it = _cache.find(cache_key);
    if (it != _cache.end()) {
      /* Insert the host into array of hosts for this (user, db) pair */
      it->second->insert(acl_db);
    } else {
      /* First one - create a new entry */
      auto new_entry = (acl_lookup_entry *)my_malloc(
          key_memory_acl_cache, sizeof(acl_lookup_entry), MYF(0));

      /* Call the ctor */
      new (new_entry) acl_lookup_entry();

      /* Insert the host into array of hosts for this (user, db) pair */
      new_entry->insert(acl_db);

      /* Insert into the top level (user, db) -> (hosts) */
      _cache[cache_key] =
          unique_ptr<acl_lookup_entry, acl_lookup_entry::deleter>(new_entry);
    }
  }

  void update(ACL_DB *acl_db) {
    /* We don't support wildcard in db in this lookup table */
    if (!acl_db->db || contains_wildcard(acl_db->db)) return;

    std::string cache_key = build_key(acl_db);
    auto it = _cache.find(cache_key);
    if (it != _cache.end()) {
      it->second->update(acl_db);
    }
  }

  void remove(ACL_DB *acl_db) {
    /* We don't support wildcard in db in this lookup table */
    if (!acl_db->db || contains_wildcard(acl_db->db)) return;

    std::string cache_key = build_key(acl_db);
    auto it = _cache.find(cache_key);
    if (it != _cache.end()) {
      auto &entry = it->second;
      entry->remove(acl_db);
      if (entry->hosts->empty()) {
        /* Last entry - delete the (user, db) hash entry */
        _cache.erase(it);
      }
    }
  }

  bool find(const char *user, const char *db, const char *host, const char *ip,
            ulong *access) {
    /* Just in case */
    if (contains_wildcard(db)) {
      return false;
    }

    std::string cache_key = build_key(user, db);
    auto it = _cache.find(cache_key);
    if (it != _cache.end()) {
      return it->second->find(host, ip, access);
    }

    return false;
  }

  void clear() { _cache.clear(); }

 private:
  malloc_unordered_map<std::string,
                       unique_ptr<acl_lookup_entry, acl_lookup_entry::deleter>>
      _cache{key_memory_acl_cache};
};

static acl_lookup_tbl *acl_fast_lookup = nullptr;

/**
  Add an internal schema to the registry.
  @param name the schema name
  @param access the schema ACL specific rules
*/
void ACL_internal_schema_registry::register_schema(
    const LEX_CSTRING &name, const ACL_internal_schema_access *access) {
  DBUG_ASSERT(m_registry_array_size < array_elements(registry_array));

  /* Not thread safe, and does not need to be. */
  registry_array[m_registry_array_size].m_name = &name;
  registry_array[m_registry_array_size].m_access = access;
  m_registry_array_size++;
}

/**
  Search per internal schema ACL by name.
  @param name a schema name
  @return per schema rules, or NULL
*/
const ACL_internal_schema_access *ACL_internal_schema_registry::lookup(
    const char *name) {
  DBUG_ASSERT(name != nullptr);

  uint i;

  for (i = 0; i < m_registry_array_size; i++) {
    if (my_strcasecmp(system_charset_info, registry_array[i].m_name->str,
                      name) == 0)
      return registry_array[i].m_access;
  }
  return nullptr;
}

const char *ACL_HOST_AND_IP::calc_ip(const char *ip_arg, long *val, char end) {
  long ip_val, tmp;
  if (!(ip_arg = str2int(ip_arg, 10, 0, 255, &ip_val)) || *ip_arg != '.')
    return nullptr;
  ip_val <<= 24;
  if (!(ip_arg = str2int(ip_arg + 1, 10, 0, 255, &tmp)) || *ip_arg != '.')
    return nullptr;
  ip_val += tmp << 16;
  if (!(ip_arg = str2int(ip_arg + 1, 10, 0, 255, &tmp)) || *ip_arg != '.')
    return nullptr;
  ip_val += tmp << 8;
  if (!(ip_arg = str2int(ip_arg + 1, 10, 0, 255, &tmp)) || *ip_arg != end)
    return nullptr;
  *val = ip_val + tmp;
  return ip_arg;
}

/**
  @brief Update the hostname. Updates ip and ip_mask accordingly.

  @param host_arg Value to be stored
 */
void ACL_HOST_AND_IP::update_hostname(const char *host_arg) {
  hostname = host_arg;  // This will not be modified!
  hostname_length = hostname ? strlen(hostname) : 0;
  if (!host_arg || (!(host_arg = calc_ip(host_arg, &ip, '/')) ||
                    !(host_arg = calc_ip(host_arg + 1, &ip_mask, '\0')))) {
    ip = ip_mask = 0;  // Not a masked ip
  }
}

/*
   @brief Comparing of hostnames.

   @TODO This function should ideally only
   be called during authentication and not from authorization code. You may
   authenticate with a hostmask, but all authentication should be against a
   specific security context with a specific authentication ID.

   @param  host_arg    Hostname to be compared with
   @param  ip_arg      IP address to be compared with

   @notes
   A hostname may be of type:
   1) hostname   (May include wildcards);   monty.pp.sci.fi
   2) ip     (May include wildcards);   192.168.0.0
   3) ip/netmask                        192.168.0.0/255.255.255.0
   A net mask of 0.0.0.0 is not allowed.

   @return
   true   if matched
   false  if not matched
 */

bool ACL_HOST_AND_IP::compare_hostname(const char *host_arg,
                                       const char *ip_arg) {
  long tmp;
  if (ip_mask && ip_arg && calc_ip(ip_arg, &tmp, '\0')) {
    return (tmp & ip_mask) == ip;
  }
  return (!hostname ||
          (host_arg &&
           !wild_case_compare(system_charset_info, host_arg, hostname)) ||
          (ip_arg && !wild_compare(ip_arg, strlen(ip_arg), hostname,
                                   strlen(hostname), false)));
}

bool ACL_HOST_AND_IP::is_same(const ACL_HOST_AND_IP *host) {
  return hostname == host->hostname &&
         hostname_length == host->hostname_length && ip == host->ip &&
         ip_mask == host->ip_mask;
}

ACL_USER::ACL_USER() {
  /* ACL_ACCESS is initialized by its constructor */
  {
    /* USER_RESOURCES */
    user_resource.questions = 0;
    user_resource.updates = 0;
    user_resource.conn_per_hour = 0;
    user_resource.user_conn = 0;
    user_resource.specified_limits = 0;
  }

  user = nullptr;

  {
    /* TLS restrictions */
    ssl_type = SSL_TYPE_NONE;
    ssl_cipher = nullptr;
    x509_issuer = nullptr;
    x509_subject = nullptr;
  }

  plugin = EMPTY_CSTR;
  password_expired = false;
  can_authenticate = false;
  password_last_changed.time_type = MYSQL_TIMESTAMP_ERROR;
  password_lifetime = 0;
  use_default_password_lifetime = false;
  account_locked = false;
  is_role = false;
  password_history_length = 0;
  use_default_password_history = false;
  password_reuse_interval = 0;
  use_default_password_reuse_interval = false;
  password_require_current = Lex_acl_attrib_udyn::DEFAULT;
  /* Acl_credentials is initialized by its constructor */
}

void ACL_USER::Password_locked_state::set_parameters(
    uint password_lock_time_days, uint failed_login_attempts) {
  m_password_lock_time_days = password_lock_time_days;
  m_remaining_login_attempts = m_failed_login_attempts = failed_login_attempts;
  m_daynr_locked = 0;
}

/**
  Updates the password locked state based on the time of day fetched from the
  THD

  @param thd the session to use to calculate time
  @param successful_login true if the login succeeded
  @param[out] ret_days_remaining remaining number of days. Filled only if
  update returns locked account
  @retval false account not locked
  @retval true account locked
*/
bool ACL_USER::Password_locked_state::update(THD *thd, bool successful_login,
                                             long *ret_days_remaining) {
  /* stop if the user is not tracking failed logins */
  if (!is_active()) return false;

  /* reset on a successful login if the account is not locked */
  if (successful_login && m_daynr_locked == 0) {
    m_remaining_login_attempts = m_failed_login_attempts;
    return false;
  }

  /* decreases the remaining login attempts if any */
  if (!successful_login && m_remaining_login_attempts > 0) {
    m_remaining_login_attempts--;
    DBUG_ASSERT(m_daynr_locked == 0);
  }

  if (m_remaining_login_attempts) return false;

  long now_day;
  /* fetch the current day */
  MYSQL_TIME tm_now;
  thd->time_zone()->gmt_sec_to_TIME(&tm_now, thd->query_start_timeval_trunc(6));
  now_day = calc_daynr(tm_now.year, tm_now.month, tm_now.day);

  DBUG_EXECUTE_IF("account_lock_daynr_add_one", { now_day += 1; });

  DBUG_EXECUTE_IF("account_lock_daynr_add_ten", { now_day += 10; });

  /* last unsuccessful login. lock the account */
  if (m_daynr_locked == 0) {
    DBUG_ASSERT(!successful_login);
    m_daynr_locked = now_day;
    *ret_days_remaining = m_password_lock_time_days;
    return true;
  };

  /* if the lock should never expire we stop here */
  if (m_daynr_locked > 0 && m_password_lock_time_days < 0) return true;

  /* check if the account is still to be locked */
  if (now_day - m_daynr_locked < (long)m_password_lock_time_days) {
    *ret_days_remaining =
        ((long)m_password_lock_time_days) - (now_day - m_daynr_locked);
    return true;
  }
  /* reset the account lock if the time has expired */
  if (now_day - m_daynr_locked >= (long)m_password_lock_time_days) {
    m_daynr_locked = 0;
    m_remaining_login_attempts = m_failed_login_attempts;
    return false;
  }

  /* it should never get to here */
  DBUG_ASSERT(false);
  return false;
}

ACL_USER *ACL_USER::copy(MEM_ROOT *root) {
  ACL_USER *dst = (ACL_USER *)root->Alloc(sizeof(ACL_USER));
  if (!dst) return nullptr;
  *dst = *this;
  dst->user = safe_strdup_root(root, user);
  dst->ssl_cipher = safe_strdup_root(root, ssl_cipher);
  dst->x509_issuer = safe_strdup_root(root, x509_issuer);
  dst->x509_subject = safe_strdup_root(root, x509_subject);
  /*
     If the plugin is built in we don't need to reallocate the name of the
     plugin.
   */
  if (auth_plugin_is_built_in(dst->plugin.str))
    dst->plugin = plugin;
  else {
    dst->plugin.str = strmake_root(root, plugin.str, plugin.length);
    dst->plugin.length = plugin.length;
  }
  for (int i = 0; i < NUM_CREDENTIALS; ++i) {
    dst->credentials[i].m_auth_string.str =
        safe_strdup_root(root, credentials[i].m_auth_string.str);
    dst->credentials[i].m_auth_string.length =
        credentials[i].m_auth_string.length;
    dst->credentials[i].m_salt_len = credentials[i].m_salt_len;
    memcpy(dst->credentials[i].m_salt, credentials[i].m_salt,
           credentials[i].m_salt_len);
  }
  dst->host.update_hostname(safe_strdup_root(root, host.get_host()));
  dst->password_require_current = password_require_current;
  dst->password_locked_state = password_locked_state;
  return dst;
}

void ACL_PROXY_USER::init(const char *host_arg, const char *user_arg,
                          const char *proxied_host_arg,
                          const char *proxied_user_arg, bool with_grant_arg) {
  user = (user_arg && *user_arg) ? user_arg : nullptr;
  host.update_hostname((host_arg && *host_arg) ? host_arg : nullptr);
  proxied_user =
      (proxied_user_arg && *proxied_user_arg) ? proxied_user_arg : nullptr;
  proxied_host.update_hostname(
      (proxied_host_arg && *proxied_host_arg) ? proxied_host_arg : nullptr);
  with_grant = with_grant_arg;
  sort =
      get_sort(4, host.get_host(), user, proxied_host.get_host(), proxied_user);
}

void ACL_PROXY_USER::init(MEM_ROOT *mem, const char *host_arg,
                          const char *user_arg, const char *proxied_host_arg,
                          const char *proxied_user_arg, bool with_grant_arg) {
  init((host_arg && *host_arg) ? strdup_root(mem, host_arg) : nullptr,
       (user_arg && *user_arg) ? strdup_root(mem, user_arg) : nullptr,
       (proxied_host_arg && *proxied_host_arg)
           ? strdup_root(mem, proxied_host_arg)
           : nullptr,
       (proxied_user_arg && *proxied_user_arg)
           ? strdup_root(mem, proxied_user_arg)
           : nullptr,
       with_grant_arg);
}

void ACL_PROXY_USER::init(TABLE *table, MEM_ROOT *mem) {
  init(get_field(mem, table->field[MYSQL_PROXIES_PRIV_HOST]),
       get_field(mem, table->field[MYSQL_PROXIES_PRIV_USER]),
       get_field(mem, table->field[MYSQL_PROXIES_PRIV_PROXIED_HOST]),
       get_field(mem, table->field[MYSQL_PROXIES_PRIV_PROXIED_USER]),
       table->field[MYSQL_PROXIES_PRIV_WITH_GRANT]->val_int() != 0);
}

bool ACL_PROXY_USER::check_validity(bool check_no_resolve) {
  if (check_no_resolve &&
      (hostname_requires_resolving(host.get_host()) ||
       hostname_requires_resolving(proxied_host.get_host())) &&
      strcmp(host.get_host(), "localhost") != 0) {
    LogErr(WARNING_LEVEL, ER_AUTHCACHE_PROXIES_PRIV_SKIPPED_NEEDS_RESOLVE,
           proxied_user ? proxied_user : "",
           proxied_host.get_host() ? proxied_host.get_host() : "",
           user ? user : "", host.get_host() ? host.get_host() : "");
  }
  return false;
}

bool ACL_PROXY_USER::matches(const char *host_arg, const char *user_arg,
                             const char *ip_arg, const char *proxied_user_arg,
                             bool any_proxy_user) {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("compare_hostname(%s,%s,%s) &&"
              "compare_hostname(%s,%s,%s) &&"
              "wild_compare (%s,%s) &&"
              "wild_compare (%s,%s)",
              host.get_host() ? host.get_host() : "<NULL>",
              host_arg ? host_arg : "<NULL>", ip_arg ? ip_arg : "<NULL>",
              proxied_host.get_host() ? proxied_host.get_host() : "<NULL>",
              host_arg ? host_arg : "<NULL>", ip_arg ? ip_arg : "<NULL>",
              user_arg ? user_arg : "<NULL>", user ? user : "<NULL>",
              proxied_user_arg ? proxied_user_arg : "<NULL>",
              proxied_user ? proxied_user : "<NULL>"));
  return host.compare_hostname(host_arg, ip_arg) &&
         proxied_host.compare_hostname(host_arg, ip_arg) &&
         (!user || (user_arg && !wild_compare(user_arg, strlen(user_arg), user,
                                              strlen(user), true))) &&
         (any_proxy_user || !proxied_user ||
          (proxied_user &&
           !wild_compare(proxied_user_arg, strlen(proxied_user_arg),
                         proxied_user, strlen(proxied_user), true)));
}

bool ACL_PROXY_USER::pk_equals(ACL_PROXY_USER *grant) {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("strcmp(%s,%s) &&"
              "strcmp(%s,%s) &&"
              "wild_compare (%s,%s) &&"
              "wild_compare (%s,%s)",
              user ? user : "<NULL>", grant->user ? grant->user : "<NULL>",
              proxied_user ? proxied_user : "<NULL>",
              grant->proxied_user ? grant->proxied_user : "<NULL>",
              host.get_host() ? host.get_host() : "<NULL>",
              grant->host.get_host() ? grant->host.get_host() : "<NULL>",
              proxied_host.get_host() ? proxied_host.get_host() : "<NULL>",
              grant->proxied_host.get_host() ? grant->proxied_host.get_host()
                                             : "<NULL>"));

  return auth_element_equals(user, grant->user) &&
         auth_element_equals(proxied_user, grant->proxied_user) &&
         auth_element_equals(host.get_host(), grant->host.get_host()) &&
         auth_element_equals(proxied_host.get_host(),
                             grant->proxied_host.get_host());
}

void ACL_PROXY_USER::print_grant(String *str) {
  str->append(STRING_WITH_LEN("GRANT PROXY ON '"));
  if (proxied_user) str->append(proxied_user, strlen(proxied_user));
  str->append(STRING_WITH_LEN("'@'"));
  if (proxied_host.get_host())
    str->append(proxied_host.get_host(), strlen(proxied_host.get_host()));
  str->append(STRING_WITH_LEN("' TO '"));
  if (user) str->append(user, strlen(user));
  str->append(STRING_WITH_LEN("'@'"));
  if (host.get_host()) str->append(host.get_host(), strlen(host.get_host()));
  str->append(STRING_WITH_LEN("'"));
  if (with_grant) str->append(STRING_WITH_LEN(" WITH GRANT OPTION"));
}

int ACL_PROXY_USER::store_pk(TABLE *table, const LEX_CSTRING &hostname,
                             const LEX_CSTRING &user,
                             const LEX_CSTRING &proxied_host,
                             const LEX_CSTRING &proxied_user) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("host=%s, user=%s, proxied_host=%s, proxied_user=%s",
                      hostname.str ? hostname.str : "<NULL>",
                      user.str ? user.str : "<NULL>",
                      proxied_host.str ? proxied_host.str : "<NULL>",
                      proxied_user.str ? proxied_user.str : "<NULL>"));
  if (table->field[MYSQL_PROXIES_PRIV_HOST]->store(
          hostname.str, hostname.length, system_charset_info))
    return true;
  if (table->field[MYSQL_PROXIES_PRIV_USER]->store(user.str, user.length,
                                                   system_charset_info))
    return true;
  if (table->field[MYSQL_PROXIES_PRIV_PROXIED_HOST]->store(
          proxied_host.str, proxied_host.length, system_charset_info))
    return true;
  if (table->field[MYSQL_PROXIES_PRIV_PROXIED_USER]->store(
          proxied_user.str, proxied_user.length, system_charset_info))
    return true;

  return false;
}

int ACL_PROXY_USER::store_with_grant(TABLE *table, bool with_grant) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("with_grant=%s", with_grant ? "TRUE" : "FALSE"));
  if (table->field[MYSQL_PROXIES_PRIV_WITH_GRANT]->store(with_grant ? 1 : 0,
                                                         true))
    return true;

  return false;
}

int ACL_PROXY_USER::store_data_record(TABLE *table, const LEX_CSTRING &hostname,
                                      const LEX_CSTRING &user,
                                      const LEX_CSTRING &proxied_host,
                                      const LEX_CSTRING &proxied_user,
                                      bool with_grant, const char *grantor) {
  DBUG_TRACE;
  if (store_pk(table, hostname, user, proxied_host, proxied_user)) return true;
  if (store_with_grant(table, with_grant)) return true;
  if (table->field[MYSQL_PROXIES_PRIV_GRANTOR]->store(grantor, strlen(grantor),
                                                      system_charset_info))
    return true;

  return false;
}

/**
  Performs wildcard matching, aka globbing, on the input string with
  the given wildcard pattern, and the specified wildcard characters.
  This method does case insensitive comparisons.

  @param[in] cs character set of the input string and wildcard pattern
  @param[in] str input which should be matched against pattern
  @param[in] str_len length of the input string
  @param[in] wildstr pattern with wildcards
  @param[in] wildstr_len length of the wildcards pattern

  @return 0 if input string match with the pattern
  @return 1 otherwise
*/
int wild_case_compare(CHARSET_INFO *cs, const char *str, size_t str_len,
                      const char *wildstr, size_t wildstr_len) {
  int flag;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("str: '%s'  wildstr: '%s'", str, wildstr));
  const char *wildstr_end = wildstr + wildstr_len;
  const char *str_end = str + str_len;

  /*
    Empty string matches only if there is only a wild_many(%) char
    in the string to be matched with.
  */
  if (str_len == 0) {
    bool ret_value = true;
    if (wildstr_len == 1) {
      ret_value = !(*wildstr == wild_many);
    }
    return ret_value;
  }

  while (wildstr != wildstr_end && str != str_end) {
    while (wildstr != wildstr_end && *wildstr != wild_many &&
           *wildstr != wild_one && str != str_end) {
      if (*wildstr == wild_prefix && wildstr[1]) wildstr++;
      if (my_toupper(cs, *wildstr++) != my_toupper(cs, *str++)) return 1;
    }
    if (wildstr == wildstr_end) {
      return str != str_end;
    }
    if (str == str_end) {
      if (*wildstr == '%' && wildstr + 1 == wildstr_end)
        return 0; /* % match empty string */
      return (wildstr != wildstr_end);
    }
    if (*wildstr++ == wild_one) {
      ++str;
      if (str == str_end) /* One char; skip */
      {
        return wildstr != wildstr_end;
      }
    } else {                                 /* Found wild_many */
      if (wildstr == wildstr_end) return 0;  // empty matches wild_many
      flag = (*wildstr != wild_many && *wildstr != wild_one);
      do {
        if (flag) {
          char cmp;
          if ((cmp = *wildstr) == wild_prefix && wildstr[1]) cmp = wildstr[1];
          cmp = my_toupper(cs, cmp);
          while (str != str_end && my_toupper(cs, *str) != cmp) str++;
          if (str == str_end) return 1;
        }
        if (wild_case_compare(cs, str, str_end - str, wildstr,
                              wildstr_end - wildstr) == 0) {
          return 0;
        }
        ++str;
      } while (str != str_end);
      return 1;
    }
  }
  return str != str_end;
}

int wild_case_compare(CHARSET_INFO *cs, const char *str, const char *wildstr) {
  return wild_case_compare(cs, str, strlen(str), wildstr, strlen(wildstr));
}

/*
  Return a number which, if sorted 'desc', puts strings in this order:
    no wildcards
    strings containg wildcards and non-wildcard characters
    single muilt-wildcard character('%')
    empty string
*/

ulong get_sort(uint count, ...) {
  va_list args;
  va_start(args, count);
  ulong sort = 0;

  /* Should not use this function with more than 4 arguments for compare. */
  DBUG_ASSERT(count <= 4);

  while (count--) {
    char *start, *str = va_arg(args, char *);
    uint chars = 0;
    uint wild_pos = 0;

    /*
      wild_pos
        0                            if string is empty
        1                            if string is a single muilt-wildcard
                                     character('%')
        first wildcard position + 1  if string containg wildcards and
                                     non-wildcard characters
    */

    if ((start = str)) {
      for (; *str; str++) {
        if (*str == wild_prefix && str[1])
          str++;
        else if (*str == wild_many || *str == wild_one) {
          wild_pos = (uint)(str - start) + 1;
          if (!(wild_pos == 1 && *str == wild_many && *(++str) == '\0'))
            wild_pos++;
          break;
        }
        chars = 128;  // Marker that chars existed
      }
    }
    sort = (sort << 8) + (wild_pos ? min(wild_pos, 127U) : chars);
  }
  va_end(args);
  return sort;
}

/**
  Check if the given host name needs to be resolved or not.
  Host name has to be resolved if it actually contains *name*.

  For example:
    192.168.1.1               --> false
    192.168.1.0/255.255.255.0 --> false
    %                         --> false
    192.168.1.%               --> false
    AB%                       --> false

    AAAAFFFF                  --> true (Hostname)
    AAAA:FFFF:1234:5678       --> false
    ::1                       --> false

  This function does not check if the given string is a valid host name or
  not. It assumes that the argument is a valid host name.

  @param hostname   the string to check.

  @return a flag telling if the argument needs to be resolved or not.
  @retval true the argument is a host name and needs to be resolved.
  @retval false the argument is either an IP address, or a patter and
          should not be resolved.
*/

bool hostname_requires_resolving(const char *hostname) {
  /* called only for --skip-name-resolve */
  DBUG_ASSERT(specialflag & SPECIAL_NO_RESOLVE);

  if (!hostname) return false;

  /*
    If the string contains any of {':', '%', '_', '/'}, it is definitely
    not a host name:
      - ':' means that the string is an IPv6 address;
      - '%' or '_' means that the string is a pattern;
      - '/' means that the string is an IPv4 network address;
  */

  for (const char *p = hostname; *p; ++p) {
    switch (*p) {
      case ':':
      case '%':
      case '_':
      case '/':
        return false;
    }
  }

  /*
    Now we have to tell a host name (ab.cd, 12.ab) from an IPv4 address
    (12.34.56.78). The assumption is that if the string contains only
    digits and dots, it is an IPv4 address. Otherwise -- a host name.
  */

  for (const char *p = hostname; *p; ++p) {
    if (*p != '.' && !my_isdigit(&my_charset_latin1, *p))
      return true; /* a "letter" has been found. */
  }

  return false; /* all characters are either dots or digits. */
}

GRANT_COLUMN::GRANT_COLUMN(String &c, ulong y)
    : rights(y), column(c.ptr(), c.length()) {}

void GRANT_NAME::set_user_details(const char *h, const char *d, const char *u,
                                  const char *t, bool is_routine) {
  /* Host given by user */
  host.update_hostname(strdup_root(&memex, h));
  if (db != d) {
    db = strdup_root(&memex, d);
    if (lower_case_table_names) my_casedn_str(files_charset_info, db);
  }
  user = strdup_root(&memex, u);
  sort = get_sort(3, host.get_host(), db, user);
  if (tname != t) {
    tname = strdup_root(&memex, t);
    if (lower_case_table_names || is_routine)
      my_casedn_str(files_charset_info, tname);
  }

  hash_key = user;
  hash_key.push_back('\0');
  hash_key.append(db);
  hash_key.push_back('\0');
  hash_key.append(tname);
  hash_key.push_back('\0');
}

GRANT_NAME::GRANT_NAME(const char *h, const char *d, const char *u,
                       const char *t, ulong p, bool is_routine)
    : db(nullptr), tname(nullptr), privs(p) {
  set_user_details(h, d, u, t, is_routine);
}

GRANT_TABLE::GRANT_TABLE(const char *h, const char *d, const char *u,
                         const char *t, ulong p, ulong c)
    : GRANT_NAME(h, d, u, t, p, false),
      cols(c),
      hash_columns(system_charset_info, key_memory_acl_memex) {}

GRANT_NAME::GRANT_NAME(TABLE *form, bool is_routine) {
  host.update_hostname(get_field(&memex, form->field[0]));
  db = get_field(&memex, form->field[1]);
  user = get_field(&memex, form->field[2]);
  if (!user) user = "";
  sort = get_sort(3, host.get_host(), db, user);
  tname = get_field(&memex, form->field[3]);
  if (!db || !tname) {
    /* Wrong table row; Ignore it */
    privs = 0;
    return; /* purecov: inspected */
  }
  if (lower_case_table_names) {
    my_casedn_str(files_charset_info, db);
  }
  if (lower_case_table_names || is_routine) {
    my_casedn_str(files_charset_info, tname);
  }

  hash_key = user;
  hash_key.push_back('\0');
  hash_key.append(db);
  hash_key.push_back('\0');
  hash_key.append(tname);
  hash_key.push_back('\0');

  if (form->field[MYSQL_TABLES_PRIV_FIELD_TABLE_PRIV]) {
    privs = (ulong)form->field[MYSQL_TABLES_PRIV_FIELD_TABLE_PRIV]->val_int();
    privs = fix_rights_for_table(privs);
  }
}

GRANT_TABLE::GRANT_TABLE(TABLE *form)
    : GRANT_NAME(form, false),
      hash_columns(system_charset_info, key_memory_acl_memex) {
  if (!db || !tname) {
    /* Wrong table row; Ignore it */
    cols = 0;
    return;
  }

  if (form->field[MYSQL_TABLES_PRIV_FIELD_COLUMN_PRIV]) {
    cols = (ulong)form->field[MYSQL_TABLES_PRIV_FIELD_COLUMN_PRIV]->val_int();
    cols = fix_rights_for_column(cols);
  } else
    cols = 0;
}

GRANT_TABLE::~GRANT_TABLE() {}

bool GRANT_TABLE::init(TABLE *col_privs) {
  int error;

  if (cols) {
    uchar key[MAX_KEY_LENGTH];
    uint key_prefix_len;

    KEY_PART_INFO *key_part = col_privs->key_info->key_part;
    col_privs->field[0]->store(host.get_host(),
                               host.get_host() ? host.get_host_len() : 0,
                               system_charset_info);
    col_privs->field[1]->store(db, strlen(db), system_charset_info);
    col_privs->field[2]->store(user, strlen(user), system_charset_info);
    col_privs->field[3]->store(tname, strlen(tname), system_charset_info);

    key_prefix_len = (key_part[0].store_length + key_part[1].store_length +
                      key_part[2].store_length + key_part[3].store_length);
    key_copy(key, col_privs->record[0], col_privs->key_info, key_prefix_len);
    col_privs->field[4]->store("", 0, &my_charset_latin1);

    error = col_privs->file->ha_index_init(0, true);
    DBUG_EXECUTE_IF("wl7158_grant_table_1", col_privs->file->ha_index_end();
                    error = HA_ERR_LOCK_DEADLOCK;);
    if (error) {
      acl_print_ha_error(error);
      return true;
    }

    error =
        col_privs->file->ha_index_read_map(col_privs->record[0], (uchar *)key,
                                           (key_part_map)15, HA_READ_KEY_EXACT);
    DBUG_ASSERT(col_privs->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                error != HA_ERR_LOCK_DEADLOCK);
    DBUG_ASSERT(col_privs->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                error != HA_ERR_LOCK_WAIT_TIMEOUT);
    DBUG_EXECUTE_IF("wl7158_grant_table_2", error = HA_ERR_LOCK_DEADLOCK;);
    if (error) {
      bool ret = false;
      cols = 0;
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
        acl_print_ha_error(error);
        ret = true;
      }
      col_privs->file->ha_index_end();
      return ret;
    }

    do {
      String *res, column_name;
      GRANT_COLUMN *mem_check;
      /* As column name is a string, we don't have to supply a buffer */
      res = col_privs->field[4]->val_str(&column_name);
      ulong priv = (ulong)col_privs->field[6]->val_int();
      DBUG_EXECUTE_IF("mysql_grant_table_init_out_of_memory",
                      DBUG_SET("+d,simulate_out_of_memory"););
      if (!(mem_check = new (*THR_MALLOC)
                GRANT_COLUMN(*res, fix_rights_for_column(priv)))) {
        /* Don't use this entry */
        col_privs->file->ha_index_end();

        DBUG_EXECUTE_IF("mysql_grant_table_init_out_of_memory",
                        DBUG_SET("-d,simulate_out_of_memory"););
        return true;
      }
      hash_columns.emplace(mem_check->column,
                           unique_ptr_destroy_only<GRANT_COLUMN>(mem_check));

      error = col_privs->file->ha_index_next(col_privs->record[0]);
      DBUG_ASSERT(col_privs->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_DEADLOCK);
      DBUG_ASSERT(col_privs->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_WAIT_TIMEOUT);
      DBUG_EXECUTE_IF("wl7158_grant_table_3", error = HA_ERR_LOCK_DEADLOCK;);
      if (error && error != HA_ERR_END_OF_FILE) {
        acl_print_ha_error(error);
        col_privs->file->ha_index_end();
        return true;
      }
    } while (!error && !key_cmp_if_same(col_privs, key, 0, key_prefix_len));
    col_privs->file->ha_index_end();
  }

  return false;
}

/**
  Build the lists of ACL_USERs which share name or have no name

  All accounts with same name will be chained so that they can be
  retrieved by a single lookup. These entries are sorted using
  ACL_compare to make sure that most specific account is picked up
  first. Anonymous user is added to each chain.
*/

void rebuild_cached_acl_users_for_name(void) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("acl_users size: %zu", acl_users->size()));

  DBUG_ASSERT(!current_thd || assert_acl_cache_write_lock(current_thd));

  if (name_to_userlist) {
    name_to_userlist->clear();
  } else {
    size_t size = sizeof(Name_to_userlist);
    myf_t flags = MYF(MY_WME | ME_FATALERROR);
    void *bytes = my_malloc(key_memory_acl_cache, size, flags);
    name_to_userlist = new (bytes) Name_to_userlist();
  }

  std::list<ACL_USER *> anons;

  /* first build each named list */
  for (ACL_USER *acl_user = acl_users->begin(); acl_user != acl_users->end();
       ++acl_user) {
    std::string name = acl_user->user ? acl_user->user : "";
    (*name_to_userlist)[name].push_back(acl_user);

    /* keep track of anonymous acl_users */
    if (!name.compare("")) anons.push_back(acl_user);
  }

  /* add the anonymous acl_users to each non-anon list */
  for (auto it = name_to_userlist->begin(); it != name_to_userlist->end();
       ++it) {
    std::string name = it->first;
    if (!name.compare("")) continue;

    auto *list = &it->second;
    for (auto it2 = anons.begin(); it2 != anons.end(); ++it2) {
      list->push_back(*it2);
    }

    list->sort(ACL_compare());
  }
}

/**
  Fetch the list of ACL_USERs which share name or have no name

  @param [in] name User entry to be searched

  @returns List of users that share same name
*/

Acl_user_ptr_list *cached_acl_users_for_name(const char *name) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s'", name));

  DBUG_ASSERT(!current_thd || assert_acl_cache_read_lock(current_thd));

  std::string user_name = name ? name : "";

  auto it = name_to_userlist->find(user_name);
  if (it != name_to_userlist->end()) return &it->second;

  it = name_to_userlist->find("");
  if (it != name_to_userlist->end()) return &it->second;

  return nullptr;
}

/*
  Find first entry that matches the current user
*/

ACL_USER *find_acl_user(const char *host, const char *user, bool exact) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("host: '%s'  user: '%s'", host, user));

  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));

  if (likely(acl_users)) {
    Acl_user_ptr_list *list = cached_acl_users_for_name(user);
    if (!list) {
      return nullptr;
    }

    for (auto it = list->begin(); it != list->end(); ++it) {
      ACL_USER *acl_user = (*it);
      DBUG_PRINT("info",
                 ("strcmp('%s','%s'), compare_hostname('%s','%s'),", user,
                  acl_user->user ? acl_user->user : "", host,
                  acl_user->host.get_host() ? acl_user->host.get_host() : ""));
      if (acl_user->user || (user && !user[0])) {
        if (exact ? !my_strcasecmp(system_charset_info, host ? host : "",
                                   acl_user->host.get_host()
                                       ? acl_user->host.get_host()
                                       : "")
                  : acl_user->host.compare_hostname(host, host)) {
          return acl_user;
        }
      }
    }
  }
  return nullptr;
}

/*
  Find user in ACL

  SYNOPSIS
    is_acl_user()
    thd                  Handle to THD
    host                 host name
    user                 user name

  RETURN
   false  user not fond
   true   there are such user
*/

bool is_acl_user(THD *thd, const char *host, const char *user) {
  bool res = true;

  /* --skip-grants */
  if (!initialized) return res;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return res;

  res = find_acl_user(host, user, true) != nullptr;
  return res;
}

/**
  Validate if a user can proxy as another user

  @param user              the logged in user (proxy user)
  @param host              the hostname part of the logged in userid
  @param ip                the ip of the logged in userid
  @param authenticated_as  the effective user a plugin is trying to
                           impersonate as (proxied user)
  @param [out] proxy_used  True if a proxy is found
  @return                  proxy user definition
    @retval NULL           proxy user definition not found or not applicable
    @retval non-null       the proxy user data
*/

ACL_PROXY_USER *acl_find_proxy_user(const char *user, const char *host,
                                    const char *ip, char *authenticated_as,
                                    bool *proxy_used) {
  /* if the proxied and proxy user are the same return OK */
  DBUG_TRACE;
  DBUG_PRINT("info", ("user=%s host=%s ip=%s authenticated_as=%s", user, host,
                      ip, authenticated_as));

  if (!strcmp(authenticated_as, user)) {
    DBUG_PRINT("info", ("user is the same as authenticated_as"));
    return nullptr;
  }

  bool find_any = check_proxy_users && !*authenticated_as;

  if (!find_any) *proxy_used = true;
  for (ACL_PROXY_USER *proxy = acl_proxy_users->begin();
       proxy != acl_proxy_users->end(); ++proxy) {
    if (proxy->matches(host, user, ip, authenticated_as, find_any)) {
      DBUG_PRINT("info", ("proxy matched=%s@%s", proxy->get_proxied_user(),
                          proxy->get_proxied_host()));
      if (!find_any) {
        DBUG_PRINT(
            "info",
            ("returning specific match as authenticated_as was specified"));
        *proxy_used = true;
        return proxy;
      } else {
        // we never use anonymous users when mapping
        // proxy users for internal plugins:
        if (strcmp(proxy->get_proxied_user() ? proxy->get_proxied_user() : "",
                   "")) {
          if (find_acl_user(proxy->get_proxied_host(),
                            proxy->get_proxied_user(), true)) {
            DBUG_PRINT("info",
                       ("setting proxy_used to true, as \
              find_all search matched real user=%s host=%s",
                        proxy->get_proxied_user(), proxy->get_proxied_host()));
            *proxy_used = true;
            strcpy(authenticated_as, proxy->get_proxied_user());
          } else {
            DBUG_PRINT("info", ("skipping match because ACL user \
              does not exist, looking for next match to map"));
          }
          if (*proxy_used) {
            DBUG_PRINT("info", ("returning matching user"));
            return proxy;
          }
        }
      }
    }
  }
  DBUG_PRINT("info", ("No matching users found, returning null"));
  return nullptr;
}

void clear_and_init_db_cache() { db_cache.clear(); }

/**
  Insert a new entry in db_cache

  @param [in] thd    Handle to THD object
  @param [in] entry  Entry to be inserted in db_cache
*/

static void insert_entry_in_db_cache(THD *thd, acl_entry *entry) {
  DBUG_TRACE;
  /* Either have WRITE lock or none at all */
  DBUG_ASSERT(assert_acl_cache_write_lock(thd) ||
              !assert_acl_cache_read_lock(thd));

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

  /*
    In following cases release memory and return
    1. Could not lock cache : This is ok because db_cache
       second level cache anyways.
    2. Someone already inserted a similar entry.
  */
  unique_ptr_my_free<acl_entry> entry_ptr(entry);
  if (!acl_cache_lock.lock(false)) return;
  db_cache.emplace(std::string(entry->key, entry->length),
                   std::move(entry_ptr));
  acl_db_cache_size = db_cache.size();
}

/**
  Get privilege for a host, user, and db
  combination.

  @note db_cache is not used if db_is_pattern
  is set.

  @param thd   Thread handler
  @param host  Host name
  @param ip    Ip
  @param user  user name
  @param db    We look for the ACL of this database
  @param db_is_pattern

  @return Database ACL
*/

ulong acl_get(THD *thd, const char *host, const char *ip, const char *user,
              const char *db, bool db_is_pattern) {
  ulong host_access = ~(ulong)0, db_access = 0;
  size_t key_length, copy_length;
  char key[ACL_KEY_LENGTH], *tmp_db, *end;
  acl_entry *entry;
  DBUG_TRACE;

  copy_length =
      (strlen(ip ? ip : "") + strlen(user ? user : "") + strlen(db ? db : "")) +
      2; /* Added 2 at the end to avoid
            buffer overflow at my_stpcpy()*/
  /*
    Make sure that my_stpcpy() operations do not result in buffer overflow.
  */
  if (copy_length >= ACL_KEY_LENGTH) return 0;
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return db_access;

  end = my_stpcpy(
      (tmp_db = my_stpcpy(my_stpcpy(key, ip ? ip : "") + 1, user) + 1), db);
  if (lower_case_table_names) {
    my_casedn_str(files_charset_info, tmp_db);
    db = tmp_db;
  }
  key_length = (size_t)(end - key);

  if (!db_is_pattern && acl_fast_lookup) {
    /*
      First lookup for exact (user, db) -> (hosts) hash table
      Note that db that has patterns won't be here but >99% of db entries
      are exact so this works out pretty well in practice
     */
    if (acl_fast_lookup->find(user, db, host, ip, &db_access)) {
      DBUG_PRINT("exit", ("access: 0x%lx", db_access));
      return db_access;
    }

    /*
      Count the number of misses - useful to detect cases where there is
      a large number of unsupported patterns showing up
     */
    acl_fast_lookup_miss++;
  }

  if (!db_is_pattern && enable_acl_db_cache) {
    const auto it = db_cache.find(std::string(key, key_length));
    if (it != db_cache.end()) {
      db_access = it->second->access;
      DBUG_PRINT("exit", ("access: 0x%lx", db_access));
      return db_access;
    }
  }

  /*
    Check if there are some access rights for database and user
  */
  acl_db_cache_slow_lookup++;
  for (ACL_DB *acl_db = acl_dbs->begin(); acl_db != acl_dbs->end(); ++acl_db) {
    if (!acl_db->user || !strcmp(user, acl_db->user)) {
      if (acl_db->host.compare_hostname(host, ip)) {
        /*
          Do the usual string comparision if partial_revokes is ON,
          otherwise do the wildcard grant comparision
        */
        if (!acl_db->db ||
            (db &&
             (mysqld_partial_revokes()
                  ? (!strcmp(db, acl_db->db))
                  : (!wild_compare(db, strlen(db), acl_db->db,
                                   strlen(acl_db->db), db_is_pattern))))) {
          db_access = acl_db->access;
          if (acl_db->host.get_host()) goto exit;  // Fully specified. Take it
          break;                                   /* purecov: tested */
        }
      }
    }
  }
  if (!db_access) goto exit;  // Can't be better

exit:
  /* Save entry in cache for quick retrieval */
  if (!db_is_pattern && enable_acl_db_cache &&
      (entry = (acl_entry *)my_malloc(
           key_memory_acl_cache, sizeof(acl_entry) + key_length, MYF(0)))) {
    entry->access = (db_access & host_access);
    entry->length = key_length;
    memcpy((uchar *)entry->key, key, key_length);
    acl_cache_lock.unlock();
    insert_entry_in_db_cache(thd, entry);
  }
  DBUG_PRINT("exit", ("access: 0x%lx", db_access & host_access));
  return db_access & host_access;
}

/*
  Check if there are any possible matching entries for this host

  NOTES
    All host names without wild cards are stored in a hash table,
    entries with wildcards are stored in a dynamic array
*/

static void init_check_host(void) {
  DBUG_TRACE;
  if (acl_wild_hosts != nullptr)
    acl_wild_hosts->clear();
  else
    acl_wild_hosts = new Prealloced_array<ACL_HOST_AND_IP, ACL_PREALLOC_SIZE>(
        key_memory_acl_mem);

  size_t acl_users_size = acl_users ? acl_users->size() : 0;

  acl_check_hosts = new collation_unordered_map<std::string, ACL_USER *>(
      system_charset_info, key_memory_acl_mem);
  if (acl_users_size && !allow_all_hosts) {
    for (ACL_USER *acl_user = acl_users->begin(); acl_user != acl_users->end();
         ++acl_user) {
      if (acl_user->host.get_host()) {
        if (acl_user->host.has_wildcard()) {  // Has wildcard
          ACL_HOST_AND_IP *acl = nullptr;
          for (acl = acl_wild_hosts->begin(); acl != acl_wild_hosts->end();
               ++acl) {  // Check if host already exists
            if (!my_strcasecmp(system_charset_info, acl_user->host.get_host(),
                               acl->get_host()))
              break;  // already stored
          }
          if (acl == acl_wild_hosts->end())  // If new
            acl_wild_hosts->push_back(acl_user->host);
        } else {
          // Will be ignored if there's already an entry.
          acl_check_hosts->emplace(acl_user->host.get_host(), acl_user);
        }
      }
    }
  }
  acl_wild_hosts->shrink_to_fit();
}

/*
  Rebuild lists used for checking of allowed hosts

  We need to rebuild 'acl_check_hosts' and 'acl_wild_hosts' after adding,
  dropping or renaming user, since they contain pointers to elements of
  'acl_user' array, which are invalidated by drop operation, and use
  ACL_USER::host::hostname as a key, which is changed by rename.
*/
void rebuild_check_host(void) {
  delete acl_wild_hosts;
  acl_wild_hosts = nullptr;
  delete acl_check_hosts;
  acl_check_hosts = nullptr;
  init_check_host();
}

/*
  Gets user credentials without authentication and resource limit checks. This
  function is used to initialized a new Security_context. It's a terrible
  anti-pattern that needs to go.

  SYNOPSIS
    acl_getroot()
      thd                Handle to THD
      sctx               Context which should be initialized
      user               user name
      host               host name
      ip                 IP
      db                 current data base name

  RETURN
    false  OK
    true   Error
*/

bool acl_getroot(THD *thd, Security_context *sctx, const char *user,
                 const char *host, const char *ip, const char *db) {
  int res = 1;
  ACL_USER *acl_user = nullptr;
  DBUG_TRACE;

  DBUG_PRINT("enter", ("Host: '%s', Ip: '%s', User: '%s', db: '%s'",
                       (host ? host : "(NULL)"), (ip ? ip : "(NULL)"), user,
                       (db ? db : "(NULL)")));
  sctx->set_user_ptr(user, user ? strlen(user) : 0);
  sctx->set_host_ptr(host, host ? strlen(host) : 0);
  sctx->set_ip_ptr(ip, ip ? strlen(ip) : 0);
  sctx->set_host_or_ip_ptr();

  if (!initialized) {
    /*
      here if mysqld's been started with --skip-grant-tables option.
    */
    sctx->skip_grants();
    return false;
  }

  sctx->set_master_access(0);
  sctx->cache_current_db_access(0);
  sctx->assign_priv_user("", 0);
  sctx->assign_priv_host("", 0);

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return true;

  /*
     Find acl entry in user database.
     This is specially tailored to suit the check we do for CALL of
     a stored procedure; user is set to what is actually a
     priv_user, which can be ''.
  */
  for (ACL_USER *acl_user_tmp = acl_users->begin();
       acl_user_tmp != acl_users->end(); ++acl_user_tmp) {
    if ((!acl_user_tmp->user && !user[0]) ||
        (acl_user_tmp->user && strcmp(user, acl_user_tmp->user) == 0)) {
      if (acl_user_tmp->host.compare_hostname(host, ip)) {
        acl_user = acl_user_tmp;
        res = 0;
        break;
      }
    }
  }

  if (acl_user) {
    sctx->clear_active_roles();
    List_of_auth_id_refs default_roles;
    Auth_id_ref authid = create_authid_from(acl_user);
    /* Needs Acl_cache_lock_guard in read mode */
    get_default_roles(authid, default_roles);
    List_of_auth_id_refs::iterator it = default_roles.begin();
    for (; it != default_roles.end(); ++it) {
      if (sctx->activate_role(it->first, it->second)) {
        sctx->clear_active_roles();
        break;
      }
    }

    if (sctx->get_active_roles()->size() == 0) {
      for (ACL_DB *acl_db = acl_dbs->begin(); acl_db != acl_dbs->end();
           ++acl_db) {
        if (!acl_db->user || (user && user[0] && !strcmp(user, acl_db->user))) {
          if (acl_db->host.compare_hostname(host, ip)) {
            /*
              Do the usual string comparision if partial_revokes is ON,
              otherwise do the wildcard grant comparision
            */
            if (!acl_db->db ||
                (db && (mysqld_partial_revokes()
                            ? (!strcmp(db, acl_db->db))
                            : (!wild_compare(db, strlen(db), acl_db->db,
                                             strlen(acl_db->db), false))))) {
              sctx->cache_current_db_access(acl_db->access);
              break;
            }
          }
        }  // end if
      }    // end for
      sctx->set_master_access(acl_user->access,
                              acl_restrictions->find_restrictions(acl_user));
    }  // end if
    sctx->assign_priv_user(user, user ? strlen(user) : 0);
    sctx->assign_priv_host(
        acl_user->host.get_host(),
        acl_user->host.get_host() ? strlen(acl_user->host.get_host()) : 0);

    sctx->set_password_expired(acl_user->password_expired);
    sctx->lock_account(acl_user->account_locked);
  }  // end if

  if (acl_user && sctx->get_active_roles()->size() > 0) {
    sctx->checkout_access_maps();
    ulong db_acl = db ? sctx->db_acl({db, strlen(db)}) : 0;
    sctx->cache_current_db_access(db_acl);
  }
  return res;
}

/**
  Convert scrambled password to binary form, according to scramble type,
  Binary form is stored in user.salt.

  @param acl_user The object where to store the salt

  Despite the name of the function it is used when loading ACLs from disk
  to store the password hash in the ACL_USER object.
  Note that it works only for native and "old" mysql authentication built-in
  plugins.

  Assumption : user's authentication plugin information is available.

  @return Password hash validation
    @retval false Hash is of suitable length
    @retval true Hash is of wrong length or format
*/

bool set_user_salt(ACL_USER *acl_user) {
  bool result = false;
  plugin_ref plugin = nullptr;

  plugin = my_plugin_lock_by_name(nullptr, acl_user->plugin,
                                  MYSQL_AUTHENTICATION_PLUGIN);
  if (plugin) {
    st_mysql_auth *auth = (st_mysql_auth *)plugin_decl(plugin)->info;

    for (int i = 0; i < NUM_CREDENTIALS && !result; ++i) {
      result = auth->set_salt(acl_user->credentials[i].m_auth_string.str,
                              acl_user->credentials[i].m_auth_string.length,
                              acl_user->credentials[i].m_salt,
                              &acl_user->credentials[i].m_salt_len);
    }
    plugin_unlock(nullptr, plugin);
  }
  return result;
}

/**
  Iterate over the user records and check for irregularities.
  Currently this includes :
   - checking if the plugin referenced is present.
   - if there's sha256 users and there's neither SSL nor RSA configured
*/
static void validate_user_plugin_records() {
  DBUG_TRACE;
  if (!validate_user_plugins) return;

  lock_plugin_data();
  for (ACL_USER *acl_user = acl_users->begin(); acl_user != acl_users->end();
       ++acl_user) {
    struct st_plugin_int *plugin;

    if (acl_user->plugin.length) {
      /* rule 1 : plugin does exit */
      if (!auth_plugin_is_built_in(acl_user->plugin.str)) {
        plugin =
            plugin_find_by_type(acl_user->plugin, MYSQL_AUTHENTICATION_PLUGIN);

        if (!plugin) {
          LogErr(WARNING_LEVEL, ER_AUTHCACHE_PLUGIN_MISSING,
                 (int)acl_user->plugin.length, acl_user->plugin.str,
                 acl_user->user,
                 static_cast<int>(acl_user->host.get_host_len()),
                 acl_user->host.get_host());
        }
      }
      if (Cached_authentication_plugins::compare_plugin(PLUGIN_SHA256_PASSWORD,
                                                        acl_user->plugin) &&
          sha256_rsa_auth_status() && !SslAcceptorContext::have_ssl()) {
        LogErr(WARNING_LEVEL, ER_AUTHCACHE_PLUGIN_CONFIG, acl_user->plugin.str,
               acl_user->user, static_cast<int>(acl_user->host.get_host_len()),
               acl_user->host.get_host(), "but neither SSL nor RSA keys are");
      }
    }
  }
  unlock_plugin_data();
}

/**
  Audit notification for flush

  @param [in] thd Handle to THD
*/

void notify_flush_event(THD *thd) {
  mysql_audit_notify(thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_FLUSH), 0,
                     nullptr, nullptr, nullptr, false, nullptr, nullptr);
}

/**
  Initialize roles structures from role tables handle.

  This function is called by acl_reload and may fail to
  initialize role structures if handle to role_edges and/or
  default_roles are NUL

  @param [in] thd      Handle to THD object
  @param [in] tablelst Handle to Roles tables

  @returns status of cache update
    @retval false Success
    @retval true failure
*/
static bool reload_roles_cache(THD *thd, TABLE_LIST *tablelst) {
  DBUG_TRACE;
  DBUG_ASSERT(tablelst);
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  thd->variables.sql_mode &= ~MODE_PAD_CHAR_TO_FULL_LENGTH;

  /*
    Attempt to reload the role cache only if the role_edges and
    default_roles tables exist.
  */
  if ((tablelst[0].table) && (tablelst[1].table) &&
      populate_roles_caches(thd, tablelst)) {
    thd->variables.sql_mode = old_sql_mode;
    return true;
  }

  thd->variables.sql_mode = old_sql_mode;
  return false;
}

/*
  Initialize structures responsible for user/db-level privilege checking and
  load privilege information for them from tables in the 'mysql' database.

  SYNOPSIS
    acl_init()
      dont_read_acl_tables  true if we want to skip loading data from
                            privilege tables and disable privilege checking.

  NOTES
    This function is mostly responsible for preparatory steps, main work
    on initialization and grants loading is done in acl_reload().

  RETURN VALUES
    0   ok
    1   Could not initialize grant's
*/

bool acl_init(bool dont_read_acl_tables) {
  THD *thd;
  bool return_val;
  DBUG_TRACE;

  init_acl_cache();

  acl_cache_initialized = true;

  /*
    cache built-in authentication plugins,
    to avoid hash searches and a global mutex lock on every connect
  */
  g_cached_authentication_plugins = new Cached_authentication_plugins();
  unknown_accounts = new Map_with_rw_lock<Auth_id, uint>(0);
  if (!g_cached_authentication_plugins->is_valid()) return true;

  if (dont_read_acl_tables) {
    return false; /* purecov: tested */
  }

  /*
    To be able to run this from boot, we allocate a temporary THD
  */
  if (!(thd = new THD)) return true; /* purecov: inspected */
  thd->thread_stack = (char *)&thd;
  thd->store_globals();

  /*
    Check storage engine type for every ACL table and output warning
    message in case it's different from supported one (InnoDB). We
    still allow server to start-up if tables are in different SE
    since this is necessary to be able to perform privilege table
    upgrade without extra server restarts. Account management
    statements do their own checks and refuse to operate if privilege
    tables are using unsupported SE.
  */
  return_val = check_engine_type_for_acl_table(thd, false);

  /*
    Check all the ACL tables are intact and output warning message in
    case any of the ACL tables are corrupted.
  */
  check_acl_tables_intact(thd, false);

  /*
    It is safe to call acl_reload() since acl_* arrays and hashes which
    will be freed there are global static objects and thus are initialized
    by zeros at startup.
  */
  return_val |= acl_reload(thd, false);
  notify_flush_event(thd);
  thd->release_resources();
  delete thd;

  return return_val;
}

void clean_user_cache() {
  if (name_to_userlist) name_to_userlist->clear();
  acl_users->clear();
}

/*
  Initialize structures responsible for user/db-level privilege checking
  and load information about grants from open privilege tables.

  SYNOPSIS
    acl_load()
      thd     Current thread
      tables  List containing open "mysql.host", "mysql.user",
              "mysql.db" and "mysql.proxies_priv", "mysql.global_grants"
              tables in that order.

  RETURN VALUES
    false  Success
    true   Error
*/

static bool acl_load(THD *thd, TABLE_LIST *tables) {
  TABLE *table;
  unique_ptr_destroy_only<RowIterator> iterator;
  bool return_val = true;
  bool check_no_resolve = specialflag & SPECIAL_NO_RESOLVE;
  char tmp_name[NAME_LEN + 1];
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  DBUG_TRACE;

  DBUG_EXECUTE_IF(
      "wl_9262_set_max_length_hostname",
      thd->security_context()->assign_priv_host("oh_my_gosh_this_is_a_long_"
                                                "hostname_look_at_it_it_has_60"
                                                "_char",
                                                60);
      thd->security_context()->assign_host("oh_my_gosh_this_is_a_long_"
                                           "hostname_look_at_it_it_has_60"
                                           "_char",
                                           60);
      thd->security_context()->set_host_or_ip_ptr(););

  thd->variables.sql_mode &= ~MODE_PAD_CHAR_TO_FULL_LENGTH;

  grant_version++; /* Privileges updated */

  clear_and_init_db_cache();  // Clear locked hostname cache
  init_acl_memory();          // Allocate the memory blocks in the MEM_ROOT

  if (read_user_table(thd, tables[0].table)) goto end;

  /*
    Prepare reading from the mysql.db table
  */
  iterator = init_table_iterator(thd, table = tables[1].table, nullptr, false,
                                 /*ignore_not_found_rows=*/false);
  if (iterator == nullptr) goto end;
  table->use_all_columns();
  acl_dbs->clear();
  if (acl_fast_lookup) {
    acl_fast_lookup->clear();
  }
  int read_rec_errcode;
  while (!(read_rec_errcode = iterator->Read())) {
    /* Reading record in mysql.db */
    ACL_DB db;
    db.host.update_hostname(
        get_field(&global_acl_memory, table->field[MYSQL_DB_FIELD_HOST]));
    db.db = get_field(&global_acl_memory, table->field[MYSQL_DB_FIELD_DB]);
    if (!db.db) {
      LogErr(WARNING_LEVEL, ER_AUTHCACHE_DB_IGNORED_EMPTY_NAME);
      continue;
    }
    db.user = get_field(&global_acl_memory, table->field[MYSQL_DB_FIELD_USER]);
    if (check_no_resolve && hostname_requires_resolving(db.host.get_host()) &&
        strcmp(db.host.get_host(), "localhost") != 0) {
      LogErr(WARNING_LEVEL, ER_AUTHCACHE_DB_SKIPPED_NEEDS_RESOLVE, db.db,
             db.user ? db.user : "",
             db.host.get_host() ? db.host.get_host() : "");
    }
    db.access = get_access(table, 3, nullptr);
    db.access = fix_rights_for_db(db.access);
    if (lower_case_table_names) {
      /*
        convert db to lower case and give a warning if the db wasn't
        already in lower case
      */
      (void)my_stpcpy(tmp_name, db.db);
      my_casedn_str(files_charset_info, db.db);
      if (strcmp(db.db, tmp_name) != 0) {
        LogErr(WARNING_LEVEL, ER_AUTHCACHE_DB_ENTRY_LOWERCASED_REVOKE_WILL_FAIL,
               db.db, db.user ? db.user : "",
               db.host.get_host() ? db.host.get_host() : "");
      }
    }
    db.sort = get_sort(3, db.host.get_host(), db.db, db.user);
    if (table->s->fields <= 9) {  // Without grant
      if (db.access & CREATE_ACL)
        db.access |= REFERENCES_ACL | INDEX_ACL | ALTER_ACL;
    }
    acl_dbs->push_back(db);
    if (acl_fast_lookup) {
      acl_fast_lookup->insert(&db);
    }
  }  // END reading records from mysql.db tables

  iterator.reset();
  if (read_rec_errcode > 0) goto end;

  std::sort(acl_dbs->begin(), acl_dbs->end(), ACL_compare());
  acl_dbs->shrink_to_fit();

  /* Prepare to read records from the mysql.proxies_priv table */
  acl_proxy_users->clear();

  if (tables[2].table) {
    iterator = init_table_iterator(thd, table = tables[2].table, nullptr, false,
                                   /*ignore_not_found_rows=*/false);
    if (iterator == nullptr) goto end;
    table->use_all_columns();
    while (!(read_rec_errcode = iterator->Read())) {
      /* Reading record in mysql.proxies_priv */
      ACL_PROXY_USER proxy;
      proxy.init(table, &global_acl_memory);
      if (proxy.check_validity(check_no_resolve)) continue;
      if (acl_proxy_users->push_back(proxy)) {
        goto end;
      }
    }  // END reading records from the mysql.proxies_priv table

    iterator.reset();
    if (read_rec_errcode > 0) goto end;

    std::sort(acl_proxy_users->begin(), acl_proxy_users->end(), ACL_compare());
  } else {
    LogErr(WARNING_LEVEL, ER_AUTHCACHE_TABLE_PROXIES_PRIV_MISSING);
  }
  acl_proxy_users->shrink_to_fit();
  validate_user_plugin_records();
  init_check_host();

  /* Load dynamic privileges */
  if (tables[3].table) {
    if (populate_dynamic_privilege_caches(thd, &tables[3])) {
      return_val = true;
      goto end;
    }
  } else {
    LogErr(WARNING_LEVEL, ER_MISSING_GRANT_SYSTEM_TABLE);
  }

  initialized = true;
  return_val = false;

end:
  thd->variables.sql_mode = old_sql_mode;
  DBUG_EXECUTE_IF("induce_acl_load_failure", return_val = true;);
  return return_val;
}

/**
  Clear second level cache on account names.
*/

void free_name_to_userlist() {
  if (!name_to_userlist) return;

  name_to_userlist->~unordered_map();
  my_free(name_to_userlist);
  name_to_userlist = nullptr;
}

void acl_free(bool end /*= false*/) {
  free_name_to_userlist();
  delete acl_users;
  acl_users = nullptr;
  delete acl_dbs;
  acl_dbs = nullptr;
  delete acl_wild_hosts;
  acl_wild_hosts = nullptr;
  delete acl_proxy_users;
  acl_proxy_users = nullptr;
  delete acl_check_hosts;
  acl_check_hosts = nullptr;
  delete acl_fast_lookup;
  acl_fast_lookup = nullptr;
  if (!end)
    clear_and_init_db_cache();
  else {
    shutdown_acl_cache();
    if (acl_cache_initialized == true) {
      db_cache.clear();
      acl_db_cache_size = 0;
      delete g_cached_authentication_plugins;
      g_cached_authentication_plugins = nullptr;
      delete unknown_accounts;
      unknown_accounts = nullptr;
      acl_cache_initialized = false;
    }
  }
  free_root(&global_acl_memory, MYF(0));
}

bool check_engine_type_for_acl_table(THD *thd, bool mdl_locked) {
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  uint flags = mdl_locked
                   ? MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_LOCK_IGNORE_TIMEOUT |
                         MYSQL_OPEN_IGNORE_FLUSH
                   : MYSQL_LOCK_IGNORE_TIMEOUT;

  /*
    Open the following ACL tables to check their consistency.
    Although we don't read here from the tables being opened we still
    request a lock type MDL_SHARED_READ_ONLY for the sake of consistency
    with other code.
  */

  grant_tables_setup_for_open(tables, TL_READ, MDL_SHARED_READ_ONLY);

  bool result = open_and_lock_tables(thd, tables, flags);
  if (!result) {
    check_engine_type_for_acl_table(tables, false);
    if (!mdl_locked)
      commit_and_close_mysql_tables(thd);
    else
      close_thread_tables(thd);
  }

  return result;
}

/*
  This internal handler implements downgrade from SL_ERROR to SL_WARNING
  for acl_init()/handle_reload_request().
*/
class Acl_ignore_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *) {
    switch (sql_errno) {
      case ER_CANNOT_LOAD_FROM_TABLE_V2:
      case ER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2:
      case ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2:
        (*level) = Sql_condition::SL_WARNING;
        break;
      default:
        break;
    }
    return false;
  }
};

/**
  Helper function that checks the sanity of tables object present in
  the TABLE_LIST object. it logs a warning message when a table is
  missing

  @param thd        Handle of current thread.
  @param tables     A valid table list pointer

  @retval
    false       OK.
    true        Error.
*/
bool check_acl_tables_intact(THD *thd, TABLE_LIST *tables) {
  Acl_table_intact table_intact(thd, WARNING_LEVEL);
  bool result_acl = false;

  DBUG_ASSERT(tables);
  for (auto idx = 0; idx < ACL_TABLES::LAST_ENTRY; idx++) {
    if (tables[idx].table) {
      result_acl |= table_intact.check(tables[idx].table, (ACL_TABLES)idx);
    } else {
      LogErr(WARNING_LEVEL, ER_MISSING_ACL_SYSTEM_TABLE,
             tables[idx].table_name_length, tables[idx].table_name);
      result_acl |= true;
    }
  }
  /* say that we're still gonna give reading a try */
  if (result_acl)
    LogErr(INFORMATION_LEVEL, ER_ACL_WRONG_OR_MISSING_ACL_TABLES_LOG);

  return result_acl;
}

/**
  Opens the ACL tables and checks their sanity. This method reports error
  only if it is unable to open or lock tables. It is called in situations
  when server has to continue even if a corrupt table was found -
  For example - acl_init()

  @param thd        Handle of current thread.
  @param mdl_locked MDL is locked

  @retval
    false       OK.
    true        Unable to open the table(s).
*/
bool check_acl_tables_intact(THD *thd, bool mdl_locked) {
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  Acl_ignore_error_handler acl_ignore_handler;
  uint flags = mdl_locked
                   ? MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_LOCK_IGNORE_TIMEOUT |
                         MYSQL_OPEN_IGNORE_FLUSH
                   : MYSQL_LOCK_IGNORE_TIMEOUT;

  grant_tables_setup_for_open(tables, TL_READ, MDL_SHARED_READ_ONLY);

  bool result_acl = open_and_lock_tables(thd, tables, flags);

  thd->push_internal_handler(&acl_ignore_handler);
  if (!result_acl) {
    check_acl_tables_intact(thd, tables);
    if (!mdl_locked)
      commit_and_close_mysql_tables(thd);
    else
      close_thread_tables(thd);
  }
  thd->pop_internal_handler();

  return result_acl;
}

/**
  Small helper function which allows to determine if error which caused
  failure to open and lock privilege tables should not be reported to error
  log (because this is expected or temporary condition).
*/

static bool is_expected_or_transient_error(THD *thd) {
  return !thd->get_stmt_da()->is_error() ||  // Interrupted/no error condition.
         thd->get_stmt_da()->mysql_errno() == ER_TABLE_NOT_LOCKED ||
         thd->get_stmt_da()->mysql_errno() == ER_LOCK_DEADLOCK;
}

/*
  Forget current user/db-level privileges and read new privileges
  from the privilege tables.

  SYNOPSIS
    acl_reload()
      thd  Current thread

  NOTE
    All tables of calling thread which were open and locked by LOCK TABLES
    statement will be unlocked and closed.
    This function is also used for initialization of structures responsible
    for user/db-level privilege checking.

  RETURN VALUE
    false  Success
    true   Failure
*/

bool acl_reload(THD *thd, bool mdl_locked) {
  MEM_ROOT old_mem;
  bool return_val = true;
  uint flags = mdl_locked
                   ? MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_LOCK_IGNORE_TIMEOUT |
                         MYSQL_OPEN_IGNORE_FLUSH
                   : MYSQL_LOCK_IGNORE_TIMEOUT;
  Prealloced_array<ACL_USER, ACL_PREALLOC_SIZE> *old_acl_users = nullptr;
  Prealloced_array<ACL_DB, ACL_PREALLOC_SIZE> *old_acl_dbs = nullptr;
  Prealloced_array<ACL_PROXY_USER, ACL_PREALLOC_SIZE> *old_acl_proxy_users =
      nullptr;
  Granted_roles_graph *old_granted_roles = nullptr;
  Default_roles *old_default_roles = nullptr;
  Role_index_map *old_authid_to_vertex = nullptr;
  acl_lookup_tbl *old_acl_fast_lookup = nullptr;
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);
  User_to_dynamic_privileges_map *old_dyn_priv_map;
  unique_ptr<Acl_restrictions> old_acl_restrictions = nullptr;
  DBUG_TRACE;

  // Interchange the global role cache ptrs with the local role cache ptrs.
  auto swap_role_cache = [&]() {
    std::swap(old_granted_roles, g_granted_roles);
    std::swap(old_default_roles, g_default_roles);
    std::swap(old_authid_to_vertex, g_authid_to_vertex);
  };
  // Delete the memory pointed by the local role cache ptrs.
  auto delete_old_role_cache = [&]() {
    delete old_granted_roles;
    delete old_default_roles;
    delete old_authid_to_vertex;
  };

  /*
    To avoid deadlocks we should obtain table locks before obtaining
    acl_cache->lock mutex.
  */
  TABLE_LIST tables[6] = {
      TABLE_LIST("mysql", "user", TL_READ, MDL_SHARED_READ_ONLY),
      /*
        For a TABLE_LIST element that is inited with a lock type TL_READ
        the type MDL_SHARED_READ_ONLY of MDL is requested for.
        Acquiring strong MDL lock allows to avoid deadlock and timeout errors
        from SE level.
      */
      TABLE_LIST("mysql", "db", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "proxies_priv", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "global_grants", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "role_edges", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "default_roles", TL_READ, MDL_SHARED_READ_ONLY)};

  tables[0].next_local = tables[0].next_global = tables + 1;
  tables[1].next_local = tables[1].next_global = tables + 2;
  tables[2].next_local = tables[2].next_global = tables + 3;
  tables[3].next_local = tables[3].next_global = tables + 4;
  tables[4].next_local = tables[4].next_global = tables + 5;

  tables[0].open_type = tables[1].open_type = tables[2].open_type =
      tables[3].open_type = tables[4].open_type = tables[5].open_type =
          OT_BASE_ONLY;
  tables[3].open_strategy = tables[4].open_strategy = tables[5].open_strategy =
      TABLE_LIST::OPEN_IF_EXISTS;

  if (open_and_lock_tables(thd, tables, flags)) {
    /*
      Execution might have been interrupted; only print the error message
      if a user error condition has been raised. Also do not print expected/
      transient errors about tables not being locked (occurs when user does
      FLUSH PRIVILEGES under LOCK TABLES) and MDL deadlocks. These errors
      can't occurr at start-up and will be reported to user anyway.
    */
    if (!is_expected_or_transient_error(thd)) {
      LogErr(ERROR_LEVEL, ER_AUTHCACHE_CANT_OPEN_AND_LOCK_PRIVILEGE_TABLES,
             thd->get_stmt_da()->message_text());
    }
    goto end;
  }

  if (!acl_cache_lock.lock()) goto end;

  old_acl_users = acl_users;
  old_acl_dbs = acl_dbs;
  old_acl_proxy_users = acl_proxy_users;
  old_acl_restrictions = move(acl_restrictions);
  old_acl_fast_lookup = acl_fast_lookup;
  swap_role_cache();
  roles_init();

  acl_users =
      new Prealloced_array<ACL_USER, ACL_PREALLOC_SIZE>(key_memory_acl_mem);
  acl_dbs = new Prealloced_array<ACL_DB, ACL_PREALLOC_SIZE>(key_memory_acl_mem);
  acl_proxy_users = new Prealloced_array<ACL_PROXY_USER, ACL_PREALLOC_SIZE>(
      key_memory_acl_mem);
  acl_restrictions = make_unique<Acl_restrictions>();
  if (enable_acl_fast_lookup) {
    acl_fast_lookup = new acl_lookup_tbl();
  } else {
    acl_fast_lookup = nullptr;
  }

  // acl_load() overwrites global_acl_memory, so we need to free it.
  // However, we can't do that immediately, because acl_load() might fail,
  // and then we'd need to keep it.
  old_mem = move(global_acl_memory);
  delete acl_wild_hosts;
  acl_wild_hosts = nullptr;
  delete acl_check_hosts;
  acl_check_hosts = nullptr;
  old_dyn_priv_map =
      swap_dynamic_privileges_map(new User_to_dynamic_privileges_map());

  /*
    Revert to the old acl caches, if either loading of acl cache or role
    cache failed. We do this because roles caches maintain the shallow
    copies of the ACL_USER(s).
  */
  if ((return_val = acl_load(thd, tables)) ||
      (return_val = reload_roles_cache(
           thd, (tables + 4)))) {  // Error. Revert to old list
    DBUG_PRINT("error", ("Reverting to old privileges"));
    acl_free(); /* purecov: inspected */
    acl_users = old_acl_users;
    acl_dbs = old_acl_dbs;
    acl_proxy_users = old_acl_proxy_users;
    acl_fast_lookup = old_acl_fast_lookup;
    global_acl_memory = move(old_mem);
    acl_restrictions = move(old_acl_restrictions);
    // Revert to the old role caches
    swap_role_cache();
    // Old caches must be pointing to the global role caches right now
    delete_old_role_cache();

    init_check_host();
    delete swap_dynamic_privileges_map(old_dyn_priv_map);
    if (!old_dyn_priv_map) dynamic_privileges_init();
    if (acl_users) rebuild_cached_acl_users_for_name();
  } else {
    delete old_acl_users;
    delete old_acl_dbs;
    delete old_acl_proxy_users;
    delete old_dyn_priv_map;
    acl_fast_lookup_enabled = (acl_fast_lookup != nullptr);
    delete old_acl_fast_lookup;
    // Delete the old role caches
    delete_old_role_cache();
    free_root(&old_mem, MYF(0));
  }

end:
  if (!mdl_locked)
    commit_and_close_mysql_tables(thd);
  else
    close_thread_tables(thd);
  get_global_acl_cache()->increase_version();
  DEBUG_SYNC(thd, "after_acl_reload");
  return return_val;
}

void acl_insert_proxy_user(ACL_PROXY_USER *new_value) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  acl_proxy_users->push_back(*new_value);
  std::sort(acl_proxy_users->begin(), acl_proxy_users->end(), ACL_compare());
}

struct Free_grant_table {
  void operator()(GRANT_TABLE *grant_table) const {
    grant_table->~GRANT_TABLE();
  }
};

/* Free grant array if possible */

void grant_free(void) {
  DBUG_TRACE;
  column_priv_hash.reset();
  proc_priv_hash.reset();
  func_priv_hash.reset();
  free_root(&memex, MYF(0));
}

/**
  @brief Initialize structures responsible for table/column-level privilege
   checking and load information for them from tables in the 'mysql' database.

  @param skip_grant_tables  true if the command line option
    --skip-grant-tables is specified, else false.

  @return Error status
    @retval false OK
    @retval true  Could not initialize grant subsystem.
*/

bool grant_init(bool skip_grant_tables) {
  THD *thd;
  bool return_val;
  DBUG_TRACE;

  if (skip_grant_tables) return false;

  if (!(thd = new THD)) return true; /* purecov: deadcode */
  thd->thread_stack = (char *)&thd;
  thd->store_globals();

  return_val = grant_reload(thd, false);

  if (return_val && thd->get_stmt_da()->is_error())
    LogErr(ERROR_LEVEL, ER_AUTHCACHE_CANT_INIT_GRANT_SUBSYSTEM,
           thd->get_stmt_da()->message_text());

  if (opt_mandatory_roles.length > 0) {
    return_val |= check_authorization_id_string(thd, opt_mandatory_roles);
  }

  thd->release_resources();
  delete thd;

  return return_val;
}

/**
  @brief Helper function to grant_reload_procs_priv

  Reads the procs_priv table into memory hash.

  @param p_table A pointer to the procs_priv table structure.

  @see grant_reload
  @see grant_reload_procs_priv

  @return Error state
    @retval true An error occurred
    @retval false Success
*/

static bool grant_load_procs_priv(TABLE *p_table) {
  MEM_ROOT *memex_ptr;
  bool return_val = true;
  int error;
  bool check_no_resolve = specialflag & SPECIAL_NO_RESOLVE;
  MEM_ROOT **save_mem_root_ptr = THR_MALLOC;
  DBUG_TRACE;
  proc_priv_hash.reset(
      new malloc_unordered_multimap<string,
                                    unique_ptr_destroy_only<GRANT_NAME>>(
          key_memory_acl_memex));
  func_priv_hash.reset(
      new malloc_unordered_multimap<string,
                                    unique_ptr_destroy_only<GRANT_NAME>>(
          key_memory_acl_memex));
  error = p_table->file->ha_index_init(0, true);
  DBUG_EXECUTE_IF("wl7158_grant_load_proc_1", p_table->file->ha_index_end();
                  error = HA_ERR_LOCK_DEADLOCK;);
  if (error) {
    acl_print_ha_error(error);
    return true;
  }
  p_table->use_all_columns();

  error = p_table->file->ha_index_first(p_table->record[0]);
  DBUG_ASSERT(p_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
              error != HA_ERR_LOCK_DEADLOCK);
  DBUG_ASSERT(p_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
              error != HA_ERR_LOCK_WAIT_TIMEOUT);
  DBUG_EXECUTE_IF("wl7158_grant_load_proc_2", error = HA_ERR_LOCK_DEADLOCK;);

  if (error) {
    if (error == HA_ERR_END_OF_FILE)
      return_val = false;  // Return Ok.
    else
      acl_print_ha_error(error);
  } else {
    memex_ptr = &memex;
    THR_MALLOC = &memex_ptr;
    do {
      GRANT_NAME *mem_check;
      malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_NAME>>
          *hash;
      if (!(mem_check = new (memex_ptr) GRANT_NAME(p_table, true))) {
        /* This could only happen if we are out memory */
        goto end_unlock;
      }

      if (check_no_resolve) {
        if (hostname_requires_resolving(mem_check->host.get_host())) {
          LogErr(WARNING_LEVEL, ER_AUTHCACHE_PROCS_PRIV_SKIPPED_NEEDS_RESOLVE,
                 mem_check->tname, mem_check->user,
                 mem_check->host.get_host() ? mem_check->host.get_host() : "");
        }
      }
      const enum_sp_type sp_type = to_sp_type(p_table->field[4]->val_int());
      if (sp_type == enum_sp_type::PROCEDURE) {
        hash = proc_priv_hash.get();
      } else if (sp_type == enum_sp_type::FUNCTION) {
        hash = func_priv_hash.get();
      } else {
        LogErr(WARNING_LEVEL,
               ER_AUTHCACHE_PROCS_PRIV_ENTRY_IGNORED_BAD_ROUTINE_TYPE,
               mem_check->tname);
        continue;
      }

      mem_check->privs = fix_rights_for_procedure(mem_check->privs);
      if (!mem_check->ok()) {
        destroy(mem_check);
      } else {
        hash->emplace(mem_check->hash_key,
                      unique_ptr_destroy_only<GRANT_NAME>(mem_check));
      }
      error = p_table->file->ha_index_next(p_table->record[0]);
      DBUG_ASSERT(p_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_DEADLOCK);
      DBUG_ASSERT(p_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_WAIT_TIMEOUT);
      DBUG_EXECUTE_IF("wl7158_grant_load_proc_3",
                      error = HA_ERR_LOCK_DEADLOCK;);
      if (error) {
        if (error == HA_ERR_END_OF_FILE)
          return_val = false;
        else
          acl_print_ha_error(error);
        goto end_unlock;
      }
    } while (true);
  }

end_unlock:
  p_table->file->ha_index_end();
  THR_MALLOC = save_mem_root_ptr;
  return return_val;
}

/**
  @brief Initialize structures responsible for table/column-level privilege
    checking and load information about grants from open privilege tables.

  @param thd Current thread
  @param tables List containing open "mysql.tables_priv" and
    "mysql.columns_priv" tables.

  @see grant_reload

  @return Error state
    @retval false Success
    @retval true Error
*/

static bool grant_load(THD *thd, TABLE_LIST *tables) {
  bool return_val = true;
  int error;
  TABLE *t_table = nullptr, *c_table = nullptr;
  bool check_no_resolve = specialflag & SPECIAL_NO_RESOLVE;
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  DBUG_TRACE;

  thd->variables.sql_mode &= ~MODE_PAD_CHAR_TO_FULL_LENGTH;

  column_priv_hash.reset(
      new malloc_unordered_multimap<string,
                                    unique_ptr_destroy_only<GRANT_TABLE>>(
          key_memory_acl_memex));

  t_table = tables[0].table;
  c_table = tables[1].table;
  error = t_table->file->ha_index_init(0, true);
  DBUG_EXECUTE_IF("wl7158_grant_load_1", t_table->file->ha_index_end();
                  error = HA_ERR_LOCK_DEADLOCK;);
  if (error) {
    acl_print_ha_error(error);
    goto end_index_init;
  }
  t_table->use_all_columns();
  c_table->use_all_columns();

  error = t_table->file->ha_index_first(t_table->record[0]);
  DBUG_ASSERT(t_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
              error != HA_ERR_LOCK_DEADLOCK);
  DBUG_ASSERT(t_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
              error != HA_ERR_LOCK_WAIT_TIMEOUT);
  DBUG_EXECUTE_IF("wl7158_grant_load_2", error = HA_ERR_LOCK_DEADLOCK;);
  if (error) {
    if (error == HA_ERR_END_OF_FILE)
      return_val = false;  // Return Ok.
    else
      acl_print_ha_error(error);
  } else {
    Swap_mem_root_guard guard(thd, &memex);
    do {
      GRANT_TABLE *mem_check = new (thd->mem_root) GRANT_TABLE(t_table);

      if (!mem_check) {
        /* This could only happen if we are out memory */
        goto end_unlock;
      }

      if (mem_check->init(c_table)) {
        destroy(mem_check);
        goto end_unlock;
      }

      if (check_no_resolve) {
        if (hostname_requires_resolving(mem_check->host.get_host()) &&
            strcmp(mem_check->host.get_host(), "localhost") != 0) {
          LogErr(WARNING_LEVEL, ER_AUTHCACHE_TABLES_PRIV_SKIPPED_NEEDS_RESOLVE,
                 mem_check->tname, mem_check->user ? mem_check->user : "",
                 mem_check->host.get_host() ? mem_check->host.get_host() : "");
        }
      }

      if (!mem_check->ok()) {
        destroy(mem_check);
      } else {
        column_priv_hash->emplace(
            mem_check->hash_key,
            unique_ptr_destroy_only<GRANT_TABLE>(mem_check));
      }
      error = t_table->file->ha_index_next(t_table->record[0]);
      DBUG_ASSERT(t_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_DEADLOCK);
      DBUG_ASSERT(t_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_WAIT_TIMEOUT);
      DBUG_EXECUTE_IF("wl7158_grant_load_3", error = HA_ERR_LOCK_DEADLOCK;);
      if (error) {
        if (error != HA_ERR_END_OF_FILE)
          acl_print_ha_error(error);
        else
          return_val = false;
        goto end_unlock;
      }

    } while (true);
  }

end_unlock:
  t_table->file->ha_index_end();
end_index_init:
  thd->variables.sql_mode = old_sql_mode;
  return return_val;
}

/**
  @brief Helper function to grant_reload. Reloads procs_priv table is it
    exists.

  @param table A pointer to the table list.

  @see grant_reload

  @return Error state
    @retval false Success
    @retval true An error has occurred.
*/

static bool grant_reload_procs_priv(TABLE_LIST *table) {
  DBUG_TRACE;

  /* Save a copy of the current hash if we need to undo the grant load */
  unique_ptr<
      malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_NAME>>>
      old_proc_priv_hash(move(proc_priv_hash));
  unique_ptr<
      malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_NAME>>>
      old_func_priv_hash(move(func_priv_hash));
  bool return_val = false;

  if ((return_val = grant_load_procs_priv(table->table))) {
    /* Error; Reverting to old hash */
    DBUG_PRINT("error", ("Reverting to old privileges"));
    proc_priv_hash = move(old_proc_priv_hash);
    func_priv_hash = move(old_func_priv_hash);
  }

  return return_val;
}

/**
  @brief Reload information about table and column level privileges if
  possible

  @param thd        Current thread
  @param mdl_locked MDL lock status - affects open/close table operations

  Locked tables are checked by acl_reload() and doesn't have to be checked
  in this call.
  This function is also used for initialization of structures responsible
  for table/column-level privilege checking.

  @return Error state
    @retval false Success
    @retval true  Error
*/

bool grant_reload(THD *thd, bool mdl_locked) {
  MEM_ROOT old_mem;
  bool return_val = true;
  uint flags = mdl_locked
                   ? MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_LOCK_IGNORE_TIMEOUT |
                         MYSQL_OPEN_IGNORE_FLUSH
                   : MYSQL_LOCK_IGNORE_TIMEOUT;
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

  DBUG_TRACE;

  /* Don't do anything if running with --skip-grant-tables */
  if (!initialized) return false;

  TABLE_LIST tables[3] = {

      /*
        Acquiring strong MDL lock allows to avoid deadlock and timeout errors
        from SE level.
      */
      TABLE_LIST("mysql", "tables_priv", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "columns_priv", TL_READ, MDL_SHARED_READ_ONLY),

      TABLE_LIST("mysql", "procs_priv", TL_READ, MDL_SHARED_READ_ONLY)};

  tables[0].next_local = tables[0].next_global = tables + 1;
  tables[1].next_local = tables[1].next_global = tables + 2;
  tables[0].open_type = tables[1].open_type = tables[2].open_type =
      OT_BASE_ONLY;

  if (open_and_lock_tables(thd, tables, flags)) {
    if (!is_expected_or_transient_error(thd)) {
      LogErr(ERROR_LEVEL, ER_AUTHCACHE_CANT_OPEN_AND_LOCK_PRIVILEGE_TABLES,
             thd->get_stmt_da()->message_text());
    }
    goto end;
  }

  if (!acl_cache_lock.lock()) goto end;

  {
    unique_ptr<
        malloc_unordered_multimap<string, unique_ptr_destroy_only<GRANT_TABLE>>>
        old_column_priv_hash(move(column_priv_hash));

    /*
      Create a new memory pool but save the current memory pool to make an
      undo opertion possible in case of failure.
    */
    old_mem = move(memex);
    init_sql_alloc(key_memory_acl_memex, &memex, ACL_ALLOC_BLOCK_SIZE, 0);
    /*
      tables[2].table i.e. procs_priv can be null if we are working with
      pre 4.1 privilage tables
    */
    if ((return_val = (grant_load(thd, tables) ||
                       grant_reload_procs_priv(
                           &tables[2])))) {  // Error. Revert to old hash
      DBUG_PRINT("error", ("Reverting to old privileges"));
      column_priv_hash = move(old_column_priv_hash); /* purecov: deadcode */
      free_root(&memex, MYF(0));
      memex = move(old_mem); /* purecov: deadcode */
    } else {                 // Reload successful
      old_column_priv_hash.reset();
      free_root(&old_mem, MYF(0));
      grant_version++;
      get_global_acl_cache()->increase_version();
    }
  }

end:
  if (!mdl_locked)
    commit_and_close_mysql_tables(thd);
  else
    close_thread_tables(thd);
  return return_val;
}

void acl_update_user(const char *user, const char *host, enum SSL_type ssl_type,
                     const char *ssl_cipher, const char *x509_issuer,
                     const char *x509_subject, USER_RESOURCES *mqh,
                     ulong privileges, const LEX_CSTRING &plugin,
                     const LEX_CSTRING &auth, const std::string &second_auth,
                     const MYSQL_TIME &password_change_time,
                     const LEX_ALTER &password_life, Restrictions &restrictions,
                     acl_table::Pod_user_what_to_update &what_to_update,
                     uint failed_login_attempts, int password_lock_time) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  for (ACL_USER *acl_user = acl_users->begin(); acl_user != acl_users->end();
       ++acl_user) {
    if ((!acl_user->user && !user[0]) ||
        (acl_user->user && !strcmp(user, acl_user->user))) {
      if ((!acl_user->host.get_host() && !host[0]) ||
          (acl_user->host.get_host() &&
           !my_strcasecmp(system_charset_info, host,
                          acl_user->host.get_host()))) {
        if (plugin.length > 0) {
          acl_user->plugin.str = plugin.str;
          acl_user->plugin.length = plugin.length;
          optimize_plugin_compare_by_pointer(&acl_user->plugin);
          if (!auth_plugin_is_built_in(acl_user->plugin.str))
            acl_user->plugin.str =
                strmake_root(&global_acl_memory, plugin.str, plugin.length);
          /* Update auth string only when specified in ALTER/GRANT */
          if (auth.str) {
            if (auth.length == 0)
              acl_user->credentials[PRIMARY_CRED].m_auth_string = EMPTY_CSTR;
            else
              acl_user->credentials[PRIMARY_CRED].m_auth_string.str =
                  strmake_root(&global_acl_memory, auth.str, auth.length);
            acl_user->credentials[PRIMARY_CRED].m_auth_string.length =
                auth.length;
            set_user_salt(acl_user);
            if (password_change_time.time_type != MYSQL_TIMESTAMP_ERROR)
              acl_user->password_last_changed = password_change_time;
          }

          if (what_to_update.m_what & USER_ATTRIBUTES) {
            if (what_to_update.m_user_attributes &
                acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD) {
              acl_user->credentials[SECOND_CRED].m_auth_string.str =
                  strmake_root(&global_acl_memory, second_auth.c_str(),
                               second_auth.length());
              acl_user->credentials[SECOND_CRED].m_auth_string.length =
                  second_auth.length();
            }
            if (what_to_update.m_user_attributes &
                acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD) {
              acl_user->credentials[SECOND_CRED].m_auth_string = EMPTY_CSTR;
            }
            if (what_to_update.m_user_attributes &
                (acl_table::USER_ATTRIBUTE_FAILED_LOGIN_ATTEMPTS |
                 acl_table::USER_ATTRIBUTE_PASSWORD_LOCK_TIME)) {
              acl_user->password_locked_state.set_parameters(
                  password_lock_time, failed_login_attempts);
            }
            set_user_salt(acl_user);
          }
        }
        DBUG_PRINT("info",
                   ("Updates global privilege for %s@%s to %lu", acl_user->user,
                    acl_user->host.get_host(), privileges));
        acl_user->access = privileges;
        if (mqh->specified_limits & USER_RESOURCES::QUERIES_PER_HOUR)
          acl_user->user_resource.questions = mqh->questions;
        if (mqh->specified_limits & USER_RESOURCES::UPDATES_PER_HOUR)
          acl_user->user_resource.updates = mqh->updates;
        if (mqh->specified_limits & USER_RESOURCES::CONNECTIONS_PER_HOUR)
          acl_user->user_resource.conn_per_hour = mqh->conn_per_hour;
        if (mqh->specified_limits & USER_RESOURCES::USER_CONNECTIONS)
          acl_user->user_resource.user_conn = mqh->user_conn;
        if (ssl_type != SSL_TYPE_NOT_SPECIFIED) {
          acl_user->ssl_type = ssl_type;
          acl_user->ssl_cipher =
              (ssl_cipher ? strdup_root(&global_acl_memory, ssl_cipher)
                          : nullptr);
          acl_user->x509_issuer =
              (x509_issuer ? strdup_root(&global_acl_memory, x509_issuer)
                           : nullptr);
          acl_user->x509_subject =
              (x509_subject ? strdup_root(&global_acl_memory, x509_subject)
                            : nullptr);
        }
        /* update details related to password lifetime, password expiry */
        if (password_life.update_password_expired_column ||
            what_to_update.m_what & PLUGIN_ATTR)
          acl_user->password_expired =
              password_life.update_password_expired_column;
        if (!password_life.update_password_expired_column &&
            password_life.update_password_expired_fields) {
          if (!password_life.use_default_password_lifetime) {
            acl_user->password_lifetime = password_life.expire_after_days;
            acl_user->use_default_password_lifetime = false;
          } else
            acl_user->use_default_password_lifetime = true;
        }

        if (password_life.update_account_locked_column) {
          acl_user->account_locked = password_life.account_locked;

          /* reset the runtime locked state if there is account locking */
          if (!acl_user->account_locked &&
              acl_user->password_locked_state.is_active())
            acl_user->password_locked_state.set_parameters(
                acl_user->password_locked_state.get_password_lock_time_days(),
                acl_user->password_locked_state.get_failed_login_attempts());
        }

        /* Update role graph  */
        string authid_role = create_authid_str_from(acl_user);
        Role_index_map::iterator it = g_authid_to_vertex->find(authid_role);
        if (it != g_authid_to_vertex->end()) {
          boost::property_map<Granted_roles_graph,
                              boost::vertex_acl_user_t>::type user_pacl_user;
          user_pacl_user =
              boost::get(boost::vertex_acl_user_t(), *g_granted_roles);
          boost::put(user_pacl_user, it->second, *acl_user);
        }

        /* update password history */
        if (password_life.update_password_history) {
          acl_user->use_default_password_history =
              password_life.use_default_password_history;
          acl_user->password_history_length =
              password_life.use_default_password_history
                  ? 0
                  : password_life.password_history_length;
        }
        /* update password history */
        if (password_life.update_password_reuse_interval) {
          acl_user->use_default_password_reuse_interval =
              password_life.use_default_password_reuse_interval;
          acl_user->password_reuse_interval =
              password_life.use_default_password_reuse_interval
                  ? 0
                  : password_life.password_reuse_interval;
        }
        /* update current password field value */
        if (password_life.update_password_require_current !=
            Lex_acl_attrib_udyn::UNCHANGED) {
          acl_user->password_require_current =
              password_life.update_password_require_current;
        }
        acl_restrictions->upsert_restrictions(acl_user, restrictions);

        /* search complete: */
        break;
      }
    }
  }
}

void acl_users_add_one(const char *user, const char *host,
                       enum SSL_type ssl_type, const char *ssl_cipher,
                       const char *x509_issuer, const char *x509_subject,
                       USER_RESOURCES *mqh, ulong privileges,
                       const LEX_CSTRING &plugin, const LEX_CSTRING &auth,
                       const LEX_CSTRING &second_auth,
                       const MYSQL_TIME &password_change_time,
                       const LEX_ALTER &password_life, bool add_role_vertex,
                       Restrictions &restrictions, uint failed_login_attempts,
                       int password_lock_time,
                       THD *thd MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  ACL_USER acl_user;

  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  /*
  All accounts can authenticate per default. This will change when
  we add a new field to the user table.

  Currently this flag is only set to false when authentication is attempted
  using an unknown user name.
  */
  acl_user.can_authenticate = true;

  acl_user.user =
      user && *user ? strdup_root(&global_acl_memory, user) : nullptr;
  acl_user.host.update_hostname(
      host && *host ? strdup_root(&global_acl_memory, host) : nullptr);
  DBUG_ASSERT(plugin.str);
  if (plugin.str[0]) {
    acl_user.plugin = plugin;
    optimize_plugin_compare_by_pointer(&acl_user.plugin);
    if (!auth_plugin_is_built_in(acl_user.plugin.str))
      acl_user.plugin.str =
          strmake_root(&global_acl_memory, plugin.str, plugin.length);
    acl_user.credentials[PRIMARY_CRED].m_auth_string.str =
        auth.str ? strmake_root(&global_acl_memory, auth.str, auth.length) : "";
    acl_user.credentials[PRIMARY_CRED].m_auth_string.length =
        auth.str ? auth.length : 0;
    if (second_auth.length) {
      acl_user.credentials[SECOND_CRED].m_auth_string.str =
          strmake_root(&global_acl_memory, second_auth.str, second_auth.length);
      acl_user.credentials[SECOND_CRED].m_auth_string.length =
          second_auth.length;
    } else {
      acl_user.credentials[SECOND_CRED].m_auth_string = EMPTY_CSTR;
    }
    optimize_plugin_compare_by_pointer(&acl_user.plugin);
  }

  acl_user.access = privileges;
  acl_user.user_resource = *mqh;
  acl_user.sort = get_sort(2, acl_user.host.get_host(), acl_user.user);
  // acl_user.hostname_length=(uint) strlen(host);
  acl_user.ssl_type =
      (ssl_type != SSL_TYPE_NOT_SPECIFIED ? ssl_type : SSL_TYPE_NONE);
  acl_user.ssl_cipher =
      ssl_cipher ? strdup_root(&global_acl_memory, ssl_cipher) : nullptr;
  acl_user.x509_issuer =
      x509_issuer ? strdup_root(&global_acl_memory, x509_issuer) : nullptr;
  acl_user.x509_subject =
      x509_subject ? strdup_root(&global_acl_memory, x509_subject) : nullptr;
  /* update details related to password lifetime, password expiry, history */
  acl_user.password_expired = password_life.update_password_expired_column;
  acl_user.password_lifetime = password_life.expire_after_days;
  acl_user.use_default_password_lifetime =
      password_life.use_default_password_lifetime;
  acl_user.password_last_changed = password_change_time;
  acl_user.account_locked = password_life.account_locked;
  acl_user.password_history_length =
      password_life.use_default_password_history
          ? 0
          : password_life.password_history_length;
  acl_user.use_default_password_history =
      password_life.use_default_password_history;
  acl_user.password_reuse_interval =
      password_life.use_default_password_reuse_interval
          ? 0
          : password_life.password_reuse_interval;
  acl_user.use_default_password_reuse_interval =
      password_life.use_default_password_reuse_interval;

  /*
  Assign the password_require_current field value to the ACL USER.
  if it was not specified then assign the default value
  */
  if (password_life.update_password_require_current ==
      Lex_acl_attrib_udyn::UNCHANGED) {
    acl_user.password_require_current = Lex_acl_attrib_udyn::DEFAULT;
  } else {
    acl_user.password_require_current =
        password_life.update_password_require_current;
  }
  acl_restrictions->upsert_restrictions(&acl_user, restrictions);
  set_user_salt(&acl_user);
  /* New user is not a role by default. */
  acl_user.is_role = false;

  acl_user.password_locked_state.set_parameters(password_lock_time,
                                                failed_login_attempts);
  acl_users->push_back(acl_user);
  if (acl_user.host.check_allow_all_hosts())
    allow_all_hosts = true;  // Anyone can connect /* purecov: tested */

  if (add_role_vertex) {
    /*
      Add vertex to role graph. ACL_USER object is copied with a shallow copy
    */
    create_role_vertex(&acl_user);
  }
}

void acl_insert_user(THD *thd MY_ATTRIBUTE((unused)), const char *user,
                     const char *host, enum SSL_type ssl_type,
                     const char *ssl_cipher, const char *x509_issuer,
                     const char *x509_subject, USER_RESOURCES *mqh,
                     ulong privileges, const LEX_CSTRING &plugin,
                     const LEX_CSTRING &auth,
                     const MYSQL_TIME &password_change_time,
                     const LEX_ALTER &password_life, Restrictions &restrictions,
                     uint failed_login_attempts, int password_lock_time) {
  DBUG_TRACE;
  acl_users_add_one(user, host, ssl_type, ssl_cipher, x509_issuer, x509_subject,
                    mqh, privileges, plugin, auth, EMPTY_CSTR,
                    password_change_time, password_life, true, restrictions,
                    failed_login_attempts, password_lock_time, thd);
  std::sort(acl_users->begin(), acl_users->end(), ACL_compare());
  rebuild_cached_acl_users_for_name();
  /* Rebuild 'acl_check_hosts' since 'acl_users' has been modified */
  rebuild_check_host();
  /* reparse mandatory roles variable */
  opt_mandatory_roles_cache = false;
}

void acl_update_proxy_user(ACL_PROXY_USER *new_value, bool is_revoke) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  for (ACL_PROXY_USER *acl_user = acl_proxy_users->begin();
       acl_user != acl_proxy_users->end(); ++acl_user) {
    if (acl_user->pk_equals(new_value)) {
      if (is_revoke) {
        DBUG_PRINT("info", ("delting ACL_PROXY_USER"));
        acl_proxy_users->erase(acl_user);
      } else {
        DBUG_PRINT("info", ("updating ACL_PROXY_USER"));
        acl_user->set_data(new_value);
      }
      break;
    }
  }
}

/*
  Remove acl_db from acl_dbs and return the next element
 */
ACL_DB *acl_remove_db(ACL_DB *acl_db) {
  if (acl_fast_lookup) {
    acl_fast_lookup->remove(acl_db);
  }
  return acl_dbs->erase(acl_db);
}

void acl_update_db(const char *user, const char *host, const char *db,
                   ulong privileges) {
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));

  for (ACL_DB *acl_db = acl_dbs->begin(); acl_db < acl_dbs->end();) {
    if ((!acl_db->user && !user[0]) ||
        (acl_db->user && !strcmp(user, acl_db->user))) {
      if ((!acl_db->host.get_host() && !host[0]) ||
          (acl_db->host.get_host() && !strcmp(host, acl_db->host.get_host()))) {
        if ((!acl_db->db && !db[0]) ||
            (acl_db->db && !strcmp(db, acl_db->db))) {
          if (privileges) {
            acl_db->access = privileges;
            if (acl_fast_lookup) {
              acl_fast_lookup->update(acl_db);
            }
          } else {
            acl_db = acl_remove_db(acl_db);
            // Don't increment loop variable.
            continue;
          }
        }
      }
    }
    ++acl_db;
  }
}

/*
  Insert a user/db/host combination into the global acl_cache

  SYNOPSIS
    acl_insert_db()
    user                User name
    host                Host name
    db                  Database name
    privileges          Bitmap of privileges

  NOTES
    Acl caches must be locked
*/

void acl_insert_db(const char *user, const char *host, const char *db,
                   ulong privileges) {
  ACL_DB acl_db;
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  acl_db.user = strdup_root(&global_acl_memory, user);
  acl_db.host.update_hostname(*host ? strdup_root(&global_acl_memory, host)
                                    : nullptr);
  acl_db.db = strdup_root(&global_acl_memory, db);
  acl_db.access = privileges;
  acl_db.sort = get_sort(3, acl_db.host.get_host(), acl_db.db, acl_db.user);
  acl_dbs->push_back(acl_db);
  if (acl_fast_lookup) {
    acl_fast_lookup->insert(&acl_db);
  }
  std::sort(acl_dbs->begin(), acl_dbs->end(), ACL_compare());
}

void get_mqh(THD *thd, const char *user, const char *host, USER_CONN *uc) {
  ACL_USER *acl_user;
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);

  if (initialized && acl_cache_lock.lock(false) &&
      (acl_user = find_acl_user(host, user, false)))
    uc->user_resources = acl_user->user_resource;
  else
    memset(&uc->user_resources, 0, sizeof(uc->user_resources));
}

/**
  Update the security context when updating the user

  Helper function.
  Update only if the security context is pointing to the same user and
  the user is not a proxied user for a different proxy user.
  And return true if the update happens (i.e. we're operating on the
  user account of the current user).
  Normalize the names for a safe compare.

  @param sctx           The security context to update
  @param acl_user_ptr   User account being updated
  @param expired        new value of the expiration flag
  @return               did the update happen ?
 */
bool update_sctx_cache(Security_context *sctx, ACL_USER *acl_user_ptr,
                       bool expired) {
  const char *acl_host = acl_user_ptr->host.get_host();
  const char *acl_user = acl_user_ptr->user;
  const char *sctx_user = sctx->priv_user().str;
  const char *sctx_host = sctx->priv_host().str;

  /* If the user is connected as a proxied user, verify against proxy user */
  if (sctx->proxy_user().str && *sctx->proxy_user().str != '\0') {
    sctx_user = sctx->user().str;
  }

  if (!acl_host) acl_host = "";
  if (!acl_user) acl_user = "";
  if (!sctx_host) sctx_host = "";
  if (!sctx_user) sctx_user = "";

  if (!strcmp(acl_user, sctx_user) && !strcmp(acl_host, sctx_host)) {
    sctx->set_password_expired(expired);
    return true;
  }

  return false;
}

struct Acl_hash_entry {
  uint64 version;
  uchar *key;
  unsigned key_length;
  Acl_map *map;
};

const uchar *hash_key(const uchar *el, size_t *length) {
  const Acl_hash_entry *entry = reinterpret_cast<const Acl_hash_entry *>(el);
  *length = entry->key_length;
  return entry->key;
}

/**
  Allocate a new cache key based on active roles, current user and
  global cache version

  @param [out] out_key The resulting key
  @param [out] key_len Key length
  @param version Global Acl_cache version
  @param uid The authorization ID of the current user
  @param active_roles The active roles of the current user

  @return Success state
    @retval true OK
    @retval false Fatal error occurred.
*/

bool create_acl_cache_hash_key(uchar **out_key, unsigned *key_len,
                               uint64 version, const Auth_id_ref &uid,
                               const List_of_auth_id_refs &active_roles) {
  List_of_auth_id_refs::const_iterator it = active_roles.begin();
  uint32 active_roles_size = 0;
  for (; it != active_roles.end(); ++it) {
    active_roles_size += it->first.length + it->second.length + 2;
  }
  auto auth_id(uid);
  *key_len = auth_id.first.length + auth_id.second.length + 2 + sizeof(uint64) +
             active_roles_size;
  *out_key =
      (uchar *)my_malloc(key_memory_acl_map_cache, *key_len, MYF(MY_WME));
  if (out_key == nullptr) return false;
  auth_id.first.str = auth_id.first.str ? auth_id.first.str : "";
  auth_id.second.str = auth_id.second.str ? auth_id.second.str : "";
  memcpy(*out_key, auth_id.first.str, auth_id.first.length);
  *(*out_key + uid.first.length) = '@';
  memcpy(*out_key + auth_id.first.length + 1, auth_id.second.str,
         auth_id.second.length);
  uint offset = auth_id.first.length + auth_id.second.length + 1;
  /* Separator between version and role */
  *(*out_key + offset) = '`';
  ++offset;
  memcpy(*out_key + offset, reinterpret_cast<void *>(&version), sizeof(uint64));
  it = active_roles.begin();
  offset += sizeof(uint64);

  for (; it != active_roles.end(); ++it) {
    memcpy(*out_key + offset, it->first.str, it->first.length);
    *(*out_key + offset + it->first.length) = '@';
    memcpy(*out_key + offset + it->first.length + 1, it->second.str,
           it->second.length);
    offset += it->first.length + it->second.length + 1;
    /* Separator between roles */
    *(*out_key + offset) = '`';
    ++offset;
  }
  DBUG_ASSERT(((offset - *key_len) == 0));
  return true;
}

Acl_cache::Acl_cache() : m_role_graph_version(0L) {
  const char *category = "sql";
  int count;
  count = static_cast<int>(array_elements(all_acl_cache_mutexes));
  mysql_mutex_register(category, all_acl_cache_mutexes, count);
  lf_hash_init(&m_cache, sizeof(Acl_hash_entry), LF_HASH_UNIQUE,
               0, /* key offset */
               0, /* key length not used */
               hash_key, &my_charset_bin);
  mysql_mutex_init(key_LOCK_acl_cache_flush, &m_cache_flush_mutex,
                   MY_MUTEX_INIT_SLOW);
}

Acl_cache::~Acl_cache() {
  mysql_mutex_destroy(&m_cache_flush_mutex);
  lf_hash_destroy(&m_cache);
}

Acl_map::Acl_map(Security_context *sctx, uint64 ver)
    : m_reference_count(0), m_version(ver), m_restrictions(nullptr) {
  DBUG_TRACE;
  Acl_cache_lock_guard acl_cache_lock(current_thd,
                                      Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) {
    DBUG_PRINT("error", ("Acl_map could not be constructed for user %s@%s => "
                         "Could not lock Acl caches.",
                         sctx->priv_user().str, sctx->priv_host().str));
    return;
  }
  m_global_acl = 0;
  ACL_USER *acl_user =
      find_acl_user(sctx->priv_host().str, sctx->priv_user().str, true);
  if (acl_user == nullptr) {
    DBUG_PRINT("error", ("Acl_map could not be constructed for user %s@%s => "
                         "No such user",
                         sctx->priv_user().str, sctx->priv_host().str));
    return;
  }
  List_of_granted_roles granted_roles;
  get_privilege_access_maps(
      acl_user, sctx->get_active_roles(), &m_global_acl, &m_db_acls,
      &m_db_wild_acls, &m_table_acls, &m_sp_acls, &m_func_acls, &granted_roles,
      &m_with_admin_acls, &m_dynamic_privileges, m_restrictions);
}

Acl_map::~Acl_map() {
  // Db_access_map is automatically destroyed and cleaned up.
}

Acl_map::Acl_map(const Acl_map &&map) : m_restrictions(nullptr) {
  operator=(map);
}

Acl_map &Acl_map::operator=(Acl_map &&map) {
  m_db_acls = move(map.m_db_acls);
  m_global_acl = map.m_global_acl;
  m_reference_count = map.m_reference_count.load();
  m_table_acls = move(map.m_table_acls);
  m_sp_acls = move(map.m_sp_acls);
  m_func_acls = move(map.m_func_acls);
  m_with_admin_acls = move(map.m_with_admin_acls);
  m_version = map.m_version;
  m_restrictions = map.m_restrictions;
  map.m_reference_count = 0;
  return *this;
}

Acl_map &Acl_map::operator=(const Acl_map &) { return *this; }

ulong Acl_map::global_acl() { return m_global_acl; }

Db_access_map *Acl_map::db_acls() { return &m_db_acls; }

Db_access_map *Acl_map::db_wild_acls() { return &m_db_wild_acls; }

Table_access_map *Acl_map::table_acls() { return &m_table_acls; }

Grant_acl_set *Acl_map::grant_acls() { return &m_with_admin_acls; }

SP_access_map *Acl_map::sp_acls() { return &m_sp_acls; }

SP_access_map *Acl_map::func_acls() { return &m_func_acls; }

Dynamic_privileges *Acl_map::dynamic_privileges() {
  return &m_dynamic_privileges;
}

Restrictions &Acl_map::restrictions() { return m_restrictions; }

void Acl_map::increase_reference_count() { ++m_reference_count; }

void Acl_map::decrease_reference_count() { --m_reference_count; }

void Acl_cache::increase_version() {
  DBUG_TRACE;
  ++m_role_graph_version;
  flush_cache();
}

uint64 Acl_cache::version() { return m_role_graph_version.load(); }

int32 Acl_cache::size() { return m_cache.count.load(); }

/**
  Finds an Acl_map entry in the Acl_cache and increase its reference count.
  If no Acl_map is located, a new one is created with reference count one.
  The Acl_map is returned to the caller.

  @param sctx The target Security_context
  @param uid The target authid
  @param active_roles A list of active roles

  @return A pointer to an Acl_map
    @retval !NULL Success
    @retval NULL A fatal OOM error happened.
*/

Acl_map *Acl_cache::checkout_acl_map(Security_context *sctx, Auth_id_ref &uid,
                                     List_of_auth_id_refs &active_roles) {
  DBUG_TRACE;
  // CREATE KEY
  uchar *key;  // allocated by create_hash_key and released by
               // Acl_cache::flush_cache
  unsigned key_len;
  uint64 version = m_role_graph_version.load();
  if (!create_acl_cache_hash_key(&key, &key_len, version, uid, active_roles)) {
    /* OOM happened */
    active_roles.clear();
    return nullptr;
  }
  LF_PINS *pins = lf_hash_get_pins(&m_cache);
  Acl_hash_entry *entry =
      (Acl_hash_entry *)lf_hash_search(&m_cache, pins, key, key_len);
  if (entry == nullptr || entry == MY_LF_ERRPTR) {
    lf_hash_search_unpin(pins);
    Acl_map *map = create_acl_map(version, sctx);  // deleted in cache_flusher
    Acl_hash_entry new_entry;
    new_entry.version = version;
    new_entry.map = map;
    new_entry.key = key;
    new_entry.key_length = key_len;
    int rc =
        lf_hash_insert(&m_cache, pins, &new_entry);  // shallow copy of entry
    if (rc != 0) {
      /* There was a duplicate; throw away the allocated memory */
      lf_hash_put_pins(pins);
      my_free(key);
      delete map;
      DBUG_PRINT("info", ("Someone else checked out the cache key"));
      /* Potentially dangerous to dive here? */
      return checkout_acl_map(sctx, uid, active_roles);
    }
    map->increase_reference_count();
    lf_hash_put_pins(pins);
    DBUG_PRINT("info", ("Checked out new privilege map. Key= %s", key));
    return map;
  }
  Acl_map *map = entry->map;
  map->increase_reference_count();
  lf_hash_search_unpin(pins);
  lf_hash_put_pins(pins);
  my_free(key);
  DBUG_PRINT("info", ("Checked out old privilege map. Key= %s", key));
  return map;
}

void Acl_cache::return_acl_map(Acl_map *map) {
  map->decrease_reference_count();
}

/**
  This global is protected by the Acl_cache::m_cache_flush_mutex and used when
  iterating the Acl_map hash in Acl_cache::flush_cache
  @see Acl_cache::flush_cache
*/
uint64 l_cache_flusher_global_version;

/**
  Utility function for removing all items from the hash.
  @param ptr A pointer to a Acl_hash_entry
  @return Always 0 with the intention that this causes the hash_search
  function to iterate every single element in the hash.
*/
static int cache_flusher(const uchar *ptr) {
  DBUG_TRACE;
  const Acl_hash_entry *entry = reinterpret_cast<const Acl_hash_entry *>(ptr);
  if (entry != nullptr) {
    if (entry->map->reference_count() == 0 &&
        entry->map->version() < l_cache_flusher_global_version)
      return 1;
  }
  return 0;
}

void Acl_cache::flush_cache() {
  DBUG_TRACE;
  LF_PINS *pins = lf_hash_get_pins(&m_cache);
  Acl_hash_entry *entry = nullptr;
  mysql_mutex_lock(&m_cache_flush_mutex);
  l_cache_flusher_global_version = version();
  do {
    entry = static_cast<Acl_hash_entry *>(
        lf_hash_random_match(&m_cache, pins, &cache_flusher, 0));
    if (entry &&
        !lf_hash_delete(&m_cache, pins, entry->key, entry->key_length)) {
      // Hash element is removed from cache; safe to delete
      my_free(entry->key);
      delete entry->map;
    }
    lf_hash_search_unpin(pins);
  } while (entry != nullptr);
  lf_hash_put_pins(pins);
  mysql_mutex_unlock(&m_cache_flush_mutex);
}

Acl_map *Acl_cache::create_acl_map(uint64 version, Security_context *sctx) {
  Acl_map *map = new Acl_map(sctx, version);
  return map;
}

void *Acl_map::operator new(size_t size) {
  return my_malloc(key_memory_acl_map_cache, size, MYF(0));
}

void Acl_map::operator delete(void *p) { my_free(p); }

void init_acl_cache() {
  g_acl_cache = new Acl_cache();
  g_mandatory_roles = new std::vector<Role_id>;
  opt_mandatory_roles_cache = false;
}

/**
  Shutdown the global Acl_cache system which was only initialized if the
  rwlocks were initialized.
  @see acl_init()
*/

void shutdown_acl_cache() {
  if (!acl_cache_initialized) return;

  /* This should clean up all remaining Acl_cache items */
  g_acl_cache->increase_version();
  DBUG_ASSERT(g_acl_cache->size() == 0);
  delete g_acl_cache;
  g_acl_cache = nullptr;
  roles_delete();
  dynamic_privileges_delete();
  delete g_mandatory_roles;
}

/* Constants used by Acl_cache_lock_guard */
static const ulong ACL_CACHE_LOCK_TIMEOUT = 3600UL;
static const ulonglong ACL_CACHE_LOCK_TIMEOUT_NSEC =
    ACL_CACHE_LOCK_TIMEOUT * 1000000000ULL;
static const MDL_key ACL_CACHE_KEY(MDL_key::ACL_CACHE, "", "");

/**
  Internal_error_handler subclass to suppress ER_LOCK_DEADLOCK,
  ER_LOCK_WAIT_TIMEOUT, ER_QUERY_INTERRUPTED and ER_QUERY_TIMEOUT.
  Instead, we will use Acl_cache_lock_guard::lock()
  to raise ER_CANNOT_LOCK_USER_MANAGEMENT_CACHES error.
*/
class Acl_cache_error_handler : public Internal_error_handler {
 public:
  /**
    Handle an error condition

    @param [in] thd           THD handle
    @param [in] sql_errno     Error raised by MDL subsystem
    @param [in] sqlstate      SQL state. Unused.
    @param [in] level         Severity level. Unused.
    @param [in] msg           Message string. Unused.
  */

  virtual bool handle_condition(THD *thd MY_ATTRIBUTE((unused)), uint sql_errno,
                                const char *sqlstate MY_ATTRIBUTE((unused)),
                                Sql_condition::enum_severity_level *level
                                    MY_ATTRIBUTE((unused)),
                                const char *msg MY_ATTRIBUTE((unused))) {
    return (sql_errno == ER_LOCK_DEADLOCK ||
            sql_errno == ER_LOCK_WAIT_TIMEOUT ||
            sql_errno == ER_QUERY_INTERRUPTED || sql_errno == ER_QUERY_TIMEOUT);
  }
};

/*
  MDL_release_locks_visitor subclass to release MDL for ACL_CACHE.
*/
class Release_acl_cache_locks : public MDL_release_locks_visitor {
 public:
  /**
    Lock releaser.
    Check details of given key and see it is of type ACL_CACHE
    and if key name it matches with m_partition. If so, release it.

    @param [in] ticket      MDL Ticket returned by MDL subsystem

    @returns Whether ticket matches our criteria or not
      @retval true  Ticket matches
      @retval false Ticket does not match
  */

  virtual bool release(MDL_ticket *ticket) {
    return ticket->get_key()->mdl_namespace() == MDL_key::ACL_CACHE;
  }
};

/**
  Acl_cache_lock_guard constructor.

  @param [in] thd             THD Handle.
  @param [in] mode            Lock mode

*/

Acl_cache_lock_guard::Acl_cache_lock_guard(THD *thd, Acl_cache_lock_mode mode)
    : m_thd(thd), m_mode(mode), m_locked(false) {
  DBUG_ASSERT(thd);
}

/**
  Explicitly take lock on Acl_cache_lock_cache object.
  If cache was already locked, just return.

  @param [in] raise_error  Whether to raise error if we fail to acquire lock

  @returns status of lock
    @retval true Lock was acquired/already acquired.
    @retval false There was some problem acquiring lock.
*/

bool Acl_cache_lock_guard::lock(bool raise_error) /* = true */
{
  DBUG_ASSERT(!m_locked);

  if (already_locked()) return true;

  /* If we do not have MDL, we should not be holding LOCK_open */
  mysql_mutex_assert_not_owner(&LOCK_open);

  MDL_request lock_request;
  MDL_REQUEST_INIT_BY_KEY(
      &lock_request, &ACL_CACHE_KEY,
      m_mode == Acl_cache_lock_mode::READ_MODE ? MDL_SHARED : MDL_EXCLUSIVE,
      MDL_EXPLICIT);
  Acl_cache_error_handler handler;
  m_thd->push_internal_handler(&handler);
  m_locked = !m_thd->mdl_context.acquire_lock_nsec(&lock_request,
                                                   ACL_CACHE_LOCK_TIMEOUT_NSEC);
  m_thd->pop_internal_handler();

  if (!m_locked && raise_error)
    my_error(ER_CANNOT_LOCK_USER_MANAGEMENT_CACHES, MYF(0));

  return m_locked;
}

/**
  Explicitly unlock all acquired locks.
*/

void Acl_cache_lock_guard::unlock() {
  /*
    It is possible that we did not take any lock. E.g.
    1. Function 1 : Take shared lock on ACL caches
    2. Function 1 : Take Lock_open
    3. Call Function 2
    4. Function 2 : Try to acquire shared lock on ACL
    caches again

    In such cases, at 4, we do not actually take
    MDL because it will be in conflict with LOCK_open

    If unlock() is called as part of destructor or
    directly in Function 2 above, we should
    not release any locks because we never acquired
    any in the first place.
  */
  if (m_locked) {
    Release_acl_cache_locks lock_visitor;
    m_thd->mdl_context.release_locks(&lock_visitor);
    m_locked = false;
  }
}

/**
  Check whether lock is already obtained or not.

  @returns lock status
    @retval true  Lock has already been taken.
    @retval false No lock is taken with this Acl_cache_lock_guard object
*/

bool Acl_cache_lock_guard::already_locked() {
  return m_thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::ACL_CACHE, "", "",
      m_mode == Acl_cache_lock_mode::READ_MODE ? MDL_SHARED : MDL_EXCLUSIVE);
}

/**
  Assert that thread owns MDL_SHARED on partition specific to the thread

  @param [in] thd    Thread for which lock is to be checked

  @returns thread owns required lock or not
    @retval true    Thread owns lock
    @retval false   Thread does not own lock
*/

bool assert_acl_cache_read_lock(THD *thd) {
  return thd->mdl_context.owns_equal_or_stronger_lock(MDL_key::ACL_CACHE, "",
                                                      "", MDL_SHARED);
}

/**
  Assert that thread owns MDL_EXCLUSIVE on all partitions

  @param [in] thd    Thread for which lock is to be checked

  @returns thread owns required lock or not
    @retval true    Thread owns lock
    @retval false   Thread does not own lock
*/

bool assert_acl_cache_write_lock(THD *thd) {
  return thd->mdl_context.owns_equal_or_stronger_lock(MDL_key::ACL_CACHE, "",
                                                      "", MDL_EXCLUSIVE);
}

/** Global sysvar: the number of old passwords to check in the history. */
uint32 global_password_history = 0;
/** Global sysvar: the number of days before a password can be reused. */
uint32 global_password_reuse_interval = 0;

/**
  Reload all ACL caches

  @param [in] thd              THD handle
  @param [in] mdl_locked       MDL locks are taken
  @returns Status of reloading ACL caches
    @retval false Success
    @retval true Error
*/

bool reload_acl_caches(THD *thd, bool mdl_locked) {
  bool retval = true;
  DBUG_TRACE;

  if (check_engine_type_for_acl_table(thd, mdl_locked) ||
      check_acl_tables_intact(thd, mdl_locked) || acl_reload(thd, mdl_locked) ||
      grant_reload(thd, mdl_locked)) {
    goto end;
  }
  retval = false;

end:
  return retval;
}

/**
  Determine sort order for two user accounts

  @param [in] a First user account's sort value
  @param [in] b Secound user account's sort value

  @returns Whether a comes before b or not
*/
bool ACL_compare::operator()(const ACL_ACCESS &a, const ACL_ACCESS &b) {
  return a.sort > b.sort;
}

/**
  Determine sort order for two user accounts

  @param [in] a First user account's sort value
  @param [in] b Secound user account's sort value

  @returns Whether a comes before b or not
*/
bool ACL_compare::operator()(const ACL_ACCESS *a, const ACL_ACCESS *b) {
  return a->sort > b->sort;
}

/**
  Construstor
*/
Acl_restrictions::Acl_restrictions() : m_restrictions_map(key_memory_acl_mem) {}

/**
  Remove the Restrictions of the ACL_USER.

  @param [in] acl_user The ACL_USER for whom to remove the Restrictions
*/
void Acl_restrictions::remove_restrictions(const ACL_USER *acl_user) {
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  const Auth_id auth_id(acl_user);
  auto itr = m_restrictions_map.find(auth_id.auth_str());
  if (itr != m_restrictions_map.end()) m_restrictions_map.erase(itr);
}

/**
  Update, insert or remove the Restrictions for the ACL_USER.

  If ACL_USER has a Restrictions
   - If specified Restrictions is not empty then update ACL_USER's
  Restrictions
   - Otherwise clear the ACL_USER's restriction
  Else if there no Restrictions for the ACL_USER then insert the specified
    Restrictions.

  @param [in] acl_user The ACL_USER for whom to alter the Restrictions
  @param [in] restrictions Restrictions to be inserted for the ACL_USER
*/
void Acl_restrictions::upsert_restrictions(const ACL_USER *acl_user,
                                           const Restrictions &restrictions) {
  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));
  const Auth_id auth_id(acl_user);
  const std::string auth_str = auth_id.auth_str();
  auto restrictions_itr = m_restrictions_map.find(auth_str);
  if (restrictions_itr != m_restrictions_map.end()) {
    if (restrictions.is_empty()) {
      /* Empty restrictions means we want to remove that from global cache */
      m_restrictions_map.erase(restrictions_itr);
    } else {
      /* If there exists restrictions then update that in the global cache */
      restrictions_itr->second = restrictions;
    }
  } else if (!restrictions.is_empty()) {
    /* Insert non-empty restrictions object in the global cache */
    m_restrictions_map.emplace(auth_str, restrictions);
  }
}

/**
  Find the Restrictions of the ACL_USER.

  @param [in]  acl_user The ACL_USER for whom to find the Restrictions

  @returns valid Restrictions if found otherwise empty Restrictions
*/
Restrictions Acl_restrictions::find_restrictions(
    const ACL_USER *acl_user) const {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  const Auth_id auth_id(acl_user);
  auto restrictions_itr = m_restrictions_map.find(auth_id.auth_str());
  if (restrictions_itr != m_restrictions_map.end())
    return restrictions_itr->second;
  else
    return Restrictions(nullptr);
}

/**
  @returns the number of Restrictions present. It is not thread safe method.
*/
size_t Acl_restrictions::size() const { return m_restrictions_map.size(); }

/**
  Method to check if there exists at least one partial revokes in the cache.
  If the cache is not initialized at the time of the method call then it
  returns no partial revokes exists.

  @param [in] thd THD handle

  @retval  true  Partial revokes exists
  @retval  false Otherwise
*/
bool is_partial_revoke_exists(THD *thd) {
  bool partial_revoke = false;
  if (thd) {
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
    if (!acl_cache_lock.lock(false)) {
      return true;
    }
    DBUG_ASSERT(acl_restrictions);
    partial_revoke = (acl_restrictions->size() > 0);
  } else {
    /*
      We need to determine the number of partial revokes at the time of server
      start. In that case thd(s) is not be available so it is safe to
      determine the number of partial revokes without lock.
    */
    if (acl_restrictions) partial_revoke = (acl_restrictions->size() > 0);
  }
  return partial_revoke;
}

bool is_acl_inited() { return acl_cache_initialized; }
