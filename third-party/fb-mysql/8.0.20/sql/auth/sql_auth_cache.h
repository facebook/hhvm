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
#ifndef SQL_USER_CACHE_INCLUDED
#define SQL_USER_CACHE_INCLUDED

#include <string.h>
#include <sys/types.h>
#include <atomic>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/property.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include "lex_string.h"
#include "lf.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "mf_wcomp.h"  // wild_many, wild_one, wild_prefix
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_sharedlib.h"
#include "my_sys.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/mysql_lex_string.h"
#include "mysql_com.h"   // SCRAMBLE_LENGTH
#include "mysql_time.h"  // MYSQL_TIME
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_internal.h"  // List_of_authid, Authid
#include "sql/auth/partial_revokes.h"
#include "sql/malloc_allocator.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_connect.h"  // USER_RESOURCES
#include "violite.h"          // SSL_type

/* Forward declarations */
class Security_context;
class String;
class THD;
struct TABLE;
template <typename Element_type, size_t Prealloc>
class Prealloced_array;
class Acl_restrictions;
enum class Lex_acl_attrib_udyn;

/* Classes */

class ACL_HOST_AND_IP {
  const char *hostname;
  size_t hostname_length;
  long ip, ip_mask;  // Used with masked ip:s

  const char *calc_ip(const char *ip_arg, long *val, char end);

 public:
  ACL_HOST_AND_IP()
      : hostname(nullptr), hostname_length(0), ip(0), ip_mask(0) {}
  const char *get_host() const { return hostname ? hostname : ""; }
  size_t get_host_len() const { return hostname_length; }

  bool has_wildcard() {
    return (strchr(get_host(), wild_many) || strchr(get_host(), wild_one) ||
            ip_mask);
  }

  bool check_allow_all_hosts() {
    return (!hostname || (hostname[0] == wild_many && !hostname[1]));
  }

  void update_hostname(const char *host_arg);

  bool compare_hostname(const char *host_arg, const char *ip_arg);

  bool is_same(const ACL_HOST_AND_IP *host);
};

class ACL_ACCESS {
 public:
  ACL_ACCESS() : host(), sort(0), access(0) {}
  ACL_HOST_AND_IP host;
  ulong sort;
  ulong access;
};

class ACL_compare {
 public:
  bool operator()(const ACL_ACCESS &a, const ACL_ACCESS &b);
  bool operator()(const ACL_ACCESS *a, const ACL_ACCESS *b);
};

/* ACL_HOST is used if no host is specified */

class ACL_HOST : public ACL_ACCESS {
 public:
  char *db;
};

#define NUM_CREDENTIALS 2
#define PRIMARY_CRED (NUM_CREDENTIALS - NUM_CREDENTIALS)
#define SECOND_CRED (PRIMARY_CRED + 1)

class Acl_credential {
 public:
  Acl_credential() {
    m_auth_string = {"", 0};
    memset(m_salt, 0, SCRAMBLE_LENGTH + 1);
    m_salt_len = 0;
  }

 public:
  LEX_CSTRING m_auth_string;
  /**
    The salt variable is used as the password hash for
    native_password_authetication.
  */
  uint8 m_salt[SCRAMBLE_LENGTH + 1];  // scrambled password in binary form
  /**
    In the old protocol the salt_len indicated what type of autnetication
    protocol was used: 0 - no password, 4 - 3.20, 8 - 4.0,  20 - 4.1.1
  */
  uint8 m_salt_len;
};

class ACL_USER : public ACL_ACCESS {
 public:
  USER_RESOURCES user_resource;
  char *user;
  enum SSL_type ssl_type;
  const char *ssl_cipher, *x509_issuer, *x509_subject;
  LEX_CSTRING plugin;
  bool password_expired;
  bool can_authenticate;
  MYSQL_TIME password_last_changed;
  uint password_lifetime;
  bool use_default_password_lifetime;
  /**
    Specifies whether the user account is locked or unlocked.
  */
  bool account_locked;
  /**
   If this ACL_USER was used as a role id then this flag is true.
   During RENAME USER this variable is used for determining if it is safe
   to rename the user or not.
  */
  bool is_role;

  /**
    The number of old passwords to check when setting a new password
  */
  uint32 password_history_length;

  /**
    Ignore @ref password_history_length,
    use the global default @ref global_password_history
  */
  bool use_default_password_history;

  /**
    The number of days that would have to pass before a password can be reused.
  */
  uint32 password_reuse_interval;
  /**
    Ignore @ref password_reuse_interval,
    use the global default @ref global_password_reuse_interval
  */
  bool use_default_password_reuse_interval;

  /**
    The current password needed to be specified while changing it.
  */
  Lex_acl_attrib_udyn password_require_current;

  /**
    Additional credentials
  */
  Acl_credential credentials[NUM_CREDENTIALS];

  ACL_USER *copy(MEM_ROOT *root);
  ACL_USER();

  class Password_locked_state {
   public:
    bool is_active() const {
      return m_password_lock_time_days != 0 && m_failed_login_attempts != 0;
    }
    int get_password_lock_time_days() const {
      return m_password_lock_time_days;
    }
    uint get_failed_login_attempts() const { return m_failed_login_attempts; }
    void set_parameters(uint password_lock_time_days,
                        uint failed_login_attempts);
    bool update(THD *thd, bool successful_login, long *ret_days_remaining);
    Password_locked_state()
        : m_password_lock_time_days(0),
          m_failed_login_attempts(0),
          m_remaining_login_attempts(0),
          m_daynr_locked(0) {}

   protected:
    /**
      read from the user config. The number of days to keep the accont locked
    */
    int m_password_lock_time_days;
    /**
      read from the user config. The number of failed login attemps before the
      account is locked
    */
    uint m_failed_login_attempts;
    /**
      The remaining login tries, valid ony if @ref m_failed_login_attempts and
      @ref m_password_lock_time_days are non-zero
    */
    uint m_remaining_login_attempts;
    /** The day the account is locked, 0 if not locked */
    long m_daynr_locked;
  } password_locked_state;
};

class ACL_DB : public ACL_ACCESS {
 public:
  char *user, *db;
};

class ACL_PROXY_USER : public ACL_ACCESS {
  const char *user;
  ACL_HOST_AND_IP proxied_host;
  const char *proxied_user;
  bool with_grant;

  typedef enum {
    MYSQL_PROXIES_PRIV_HOST,
    MYSQL_PROXIES_PRIV_USER,
    MYSQL_PROXIES_PRIV_PROXIED_HOST,
    MYSQL_PROXIES_PRIV_PROXIED_USER,
    MYSQL_PROXIES_PRIV_WITH_GRANT,
    MYSQL_PROXIES_PRIV_GRANTOR,
    MYSQL_PROXIES_PRIV_TIMESTAMP
  } old_acl_proxy_users;

 public:
  ACL_PROXY_USER() {}

  void init(const char *host_arg, const char *user_arg,
            const char *proxied_host_arg, const char *proxied_user_arg,
            bool with_grant_arg);

  void init(MEM_ROOT *mem, const char *host_arg, const char *user_arg,
            const char *proxied_host_arg, const char *proxied_user_arg,
            bool with_grant_arg);

  void init(TABLE *table, MEM_ROOT *mem);

  bool get_with_grant() { return with_grant; }
  const char *get_user() { return user; }
  const char *get_proxied_user() { return proxied_user; }
  const char *get_proxied_host() { return proxied_host.get_host(); }
  void set_user(MEM_ROOT *mem, const char *user_arg) {
    user = user_arg && *user_arg ? strdup_root(mem, user_arg) : nullptr;
  }

  bool check_validity(bool check_no_resolve);

  bool matches(const char *host_arg, const char *user_arg, const char *ip_arg,
               const char *proxied_user_arg, bool any_proxy_user);

  inline static bool auth_element_equals(const char *a, const char *b) {
    return (a == b || (a != nullptr && b != nullptr && !strcmp(a, b)));
  }

  bool pk_equals(ACL_PROXY_USER *grant);

  bool granted_on(const char *host_arg, const char *user_arg) {
    return (
        ((!user && (!user_arg || !user_arg[0])) ||
         (user && user_arg && !strcmp(user, user_arg))) &&
        ((!host.get_host() && (!host_arg || !host_arg[0])) ||
         (host.get_host() && host_arg && !strcmp(host.get_host(), host_arg))));
  }

  void print_grant(String *str);

  void set_data(ACL_PROXY_USER *grant) { with_grant = grant->with_grant; }

  static int store_pk(TABLE *table, const LEX_CSTRING &host,
                      const LEX_CSTRING &user, const LEX_CSTRING &proxied_host,
                      const LEX_CSTRING &proxied_user);

  static int store_with_grant(TABLE *table, bool with_grant);

  static int store_data_record(TABLE *table, const LEX_CSTRING &host,
                               const LEX_CSTRING &user,
                               const LEX_CSTRING &proxied_host,
                               const LEX_CSTRING &proxied_user, bool with_grant,
                               const char *grantor);
};

class acl_entry {
 public:
  ulong access;
  uint16 length;
  char key[1];  // Key will be stored here
};

class GRANT_COLUMN {
 public:
  ulong rights;
  std::string column;
  GRANT_COLUMN(String &c, ulong y);
};

class GRANT_NAME {
 public:
  ACL_HOST_AND_IP host;
  char *db;
  const char *user;
  char *tname;
  ulong privs;
  ulong sort;
  std::string hash_key;
  GRANT_NAME(const char *h, const char *d, const char *u, const char *t,
             ulong p, bool is_routine);
  GRANT_NAME(TABLE *form, bool is_routine);
  virtual ~GRANT_NAME() {}
  virtual bool ok() { return privs != 0; }
  void set_user_details(const char *h, const char *d, const char *u,
                        const char *t, bool is_routine);
};

class GRANT_TABLE : public GRANT_NAME {
 public:
  ulong cols;
  collation_unordered_multimap<std::string,
                               unique_ptr_destroy_only<GRANT_COLUMN>>
      hash_columns;

  GRANT_TABLE(const char *h, const char *d, const char *u, const char *t,
              ulong p, ulong c);
  explicit GRANT_TABLE(TABLE *form);
  bool init(TABLE *col_privs);
  ~GRANT_TABLE();
  bool ok() { return privs != 0 || cols != 0; }
};

/*
 * A default/no-arg constructor is useful with containers-of-containers
 * situations in which a two-allocator scoped_allocator_adapter is not enough.
 * This custom allocator provides a Malloc_allocator with a no-arg constructor
 * by hard-coding the key_memory_acl_cache constructor argument.
 * This "solution" lacks beauty, yet is pragmatic.
 */
template <class T>
class Acl_cache_allocator : public Malloc_allocator<T> {
 public:
  Acl_cache_allocator() : Malloc_allocator<T>(key_memory_acl_cache) {}
  template <class U>
  struct rebind {
    typedef Acl_cache_allocator<U> other;
  };

  template <class U>
  Acl_cache_allocator(
      const Acl_cache_allocator<U> &other MY_ATTRIBUTE((unused)))
      : Malloc_allocator<T>(key_memory_acl_cache) {}

  template <class U>
  Acl_cache_allocator &operator=(
      const Acl_cache_allocator<U> &other MY_ATTRIBUTE((unused))) {}
};
typedef Acl_cache_allocator<ACL_USER *> Acl_user_ptr_allocator;
typedef std::list<ACL_USER *, Acl_user_ptr_allocator> Acl_user_ptr_list;
Acl_user_ptr_list *cached_acl_users_for_name(const char *name);
void rebuild_cached_acl_users_for_name(void);

/* Data Structures */
extern MEM_ROOT global_acl_memory;
extern MEM_ROOT memex;
const size_t ACL_PREALLOC_SIZE = 10U;
extern Prealloced_array<ACL_USER, ACL_PREALLOC_SIZE> *acl_users;
extern Prealloced_array<ACL_PROXY_USER, ACL_PREALLOC_SIZE> *acl_proxy_users;
extern Prealloced_array<ACL_DB, ACL_PREALLOC_SIZE> *acl_dbs;
extern Prealloced_array<ACL_HOST_AND_IP, ACL_PREALLOC_SIZE> *acl_wild_hosts;
extern std::unique_ptr<malloc_unordered_multimap<
    std::string, unique_ptr_destroy_only<GRANT_TABLE>>>
    column_priv_hash;
extern std::unique_ptr<
    malloc_unordered_multimap<std::string, unique_ptr_destroy_only<GRANT_NAME>>>
    proc_priv_hash, func_priv_hash;
extern collation_unordered_map<std::string, ACL_USER *> *acl_check_hosts;
extern bool allow_all_hosts;
extern uint grant_version; /* Version of priv tables */
extern std::unique_ptr<Acl_restrictions> acl_restrictions;
// Search for a matching grant. Prefer exact grants before non-exact ones.

extern MYSQL_PLUGIN_IMPORT CHARSET_INFO *files_charset_info;

template <class T>
T *name_hash_search(
    const malloc_unordered_multimap<std::string, unique_ptr_destroy_only<T>>
        &name_hash,
    const char *host, const char *ip, const char *db, const char *user,
    const char *tname, bool exact, bool name_tolower) {
  T *found = nullptr;

  std::string name = tname;
  if (name_tolower) my_casedn_str(files_charset_info, &name[0]);
  std::string key = user;
  key.push_back('\0');
  key.append(db);
  key.push_back('\0');
  key.append(name);
  key.push_back('\0');

  auto it_range = name_hash.equal_range(key);
  for (auto it = it_range.first; it != it_range.second; ++it) {
    T *grant_name = it->second.get();
    if (exact) {
      if (!grant_name->host.get_host() ||
          (host && !my_strcasecmp(system_charset_info, host,
                                  grant_name->host.get_host())) ||
          (ip && !strcmp(ip, grant_name->host.get_host())))
        return grant_name;
    } else {
      if (grant_name->host.compare_hostname(host, ip) &&
          (!found || found->sort < grant_name->sort))
        found = grant_name;  // Host ok
    }
  }
  return found;
}

inline GRANT_NAME *routine_hash_search(const char *host, const char *ip,
                                       const char *db, const char *user,
                                       const char *tname, bool proc,
                                       bool exact) {
  return name_hash_search(proc ? *proc_priv_hash : *func_priv_hash, host, ip,
                          db, user, tname, exact, true);
}

inline GRANT_TABLE *table_hash_search(const char *host, const char *ip,
                                      const char *db, const char *user,
                                      const char *tname, bool exact) {
  return name_hash_search(*column_priv_hash, host, ip, db, user, tname, exact,
                          false);
}

inline GRANT_COLUMN *column_hash_search(GRANT_TABLE *t, const char *cname,
                                        size_t length) {
  return find_or_nullptr(t->hash_columns, std::string(cname, length));
}

/* Role management */

/** Tag dispatch for custom Role_properties */
namespace boost {
enum vertex_acl_user_t { vertex_acl_user };
BOOST_INSTALL_PROPERTY(vertex, acl_user);
}  // namespace boost

/**
  Custom vertex properties used in Granted_roles_graph
  TODO ACL_USER contains too much information. We only need global access,
  username and hostname. If this was a POD we don't have to hold the same
  mutex as ACL_USER.
*/
typedef boost::property<boost::vertex_acl_user_t, ACL_USER,
                        boost::property<boost::vertex_name_t, std::string>>
    Role_properties;

typedef boost::property<boost::edge_capacity_t, int> Role_edge_properties;

/** A graph of all users/roles privilege inheritance */
typedef boost::adjacency_list<boost::setS,            // OutEdges
                              boost::vecS,            // Vertices
                              boost::bidirectionalS,  // Directed graph
                              Role_properties,        // Vertex props
                              Role_edge_properties>
    Granted_roles_graph;

/** The data type of a vertex in the Granted_roles_graph */
typedef boost::graph_traits<Granted_roles_graph>::vertex_descriptor
    Role_vertex_descriptor;

/** The data type of an edge in the Granted_roles_graph */
typedef boost::graph_traits<Granted_roles_graph>::edge_descriptor
    Role_edge_descriptor;

/** The datatype of the map between authids and graph vertex descriptors */
typedef std::unordered_map<std::string, Role_vertex_descriptor> Role_index_map;

/** The type used for the number of edges incident to a vertex in the graph.
 */
using degree_s_t = boost::graph_traits<Granted_roles_graph>::degree_size_type;

/** The type for the iterator returned by out_edges(). */
using out_edge_itr_t =
    boost::graph_traits<Granted_roles_graph>::out_edge_iterator;

/** The type for the iterator returned by in_edges(). */
using in_edge_itr_t =
    boost::graph_traits<Granted_roles_graph>::in_edge_iterator;

/** Container for global, schema, table/view and routine ACL maps */
class Acl_map {
 public:
  Acl_map(Security_context *sctx, uint64 ver);
  Acl_map(const Acl_map &map) = delete;
  Acl_map(const Acl_map &&map);
  ~Acl_map();

 private:
  Acl_map &operator=(const Acl_map &map);

 public:
  void *operator new(size_t size);
  void operator delete(void *p);
  Acl_map &operator=(Acl_map &&map);
  void increase_reference_count();
  void decrease_reference_count();

  ulong global_acl();
  Db_access_map *db_acls();
  Db_access_map *db_wild_acls();
  Table_access_map *table_acls();
  SP_access_map *sp_acls();
  SP_access_map *func_acls();
  Grant_acl_set *grant_acls();
  Dynamic_privileges *dynamic_privileges();
  Restrictions &restrictions();
  uint64 version() { return m_version; }
  uint32 reference_count() { return m_reference_count.load(); }

 private:
  std::atomic<int32> m_reference_count;
  uint64 m_version;
  Db_access_map m_db_acls;
  Db_access_map m_db_wild_acls;
  Table_access_map m_table_acls;
  ulong m_global_acl;
  SP_access_map m_sp_acls;
  SP_access_map m_func_acls;
  Grant_acl_set m_with_admin_acls;
  Dynamic_privileges m_dynamic_privileges;
  Restrictions m_restrictions;
};

typedef LF_HASH Acl_cache_internal;

class Acl_cache {
 public:
  Acl_cache();
  ~Acl_cache();

  /**
    When ever the role graph is modified we must flatten the privileges again.
    This is done by increasing the role graph version counter. Next time
    a security context is created for an authorization id (aid) a request is
    also sent to the acl_cache to checkout a flattened acl_map for this
    particular aid. If a previous acl_map exists the version of this map is
    compared to the role graph version. If they don't match a new acl_map
    is calculated and inserted into the cache.
  */
  void increase_version();
  /**
    Returns a pointer to an acl map to the caller and increase the reference
    count on the object, iff the object version is the same as the global
    graph version.
    If no acl map exists which correspond to the current authorization id of
    the security context, a new acl map is calculated, inserted into the cache
    and returned to the user.
    A new object will also be created if the role graph version counter is
    different than the acl map object's version.

    @param uid
  */
  Acl_map *checkout_acl_map(Security_context *sctx, Auth_id_ref &uid,
                            List_of_auth_id_refs &active_roles);
  /**
    When the security context is done with the acl map it calls the cache
    to decrease the reference count on that object.
    @param map
  */
  void return_acl_map(Acl_map *map);
  /**
    Removes all acl map objects with a references count of zero.
  */
  void flush_cache();
  /**
    Return a lower boundary to the current version count.
  */
  uint64 version();
  /**
    Return a snapshot of the number of items in the cache
  */
  int32 size();

 private:
  /**
    Creates a new acl map for the authorization id of the security context.

    @param version The version of the new map
    @param sctx The associated security context
  */
  Acl_map *create_acl_map(uint64 version, Security_context *sctx);
  /** Role graph version counter */
  std::atomic<uint64> m_role_graph_version;
  Acl_cache_internal m_cache;
  mysql_mutex_t m_cache_flush_mutex;
};

Acl_cache *get_global_acl_cache();

/**
  Enum for specifying lock type over Acl cache
*/

enum class Acl_cache_lock_mode { READ_MODE = 1, WRITE_MODE };

/**
  Lock guard for ACL Cache.
  Destructor automatically releases the lock.
*/

class Acl_cache_lock_guard {
 public:
  Acl_cache_lock_guard(THD *thd, Acl_cache_lock_mode mode);

  /**
    Acl_cache_lock_guard destructor.

    Release lock(s) if taken
  */
  ~Acl_cache_lock_guard() { unlock(); }

  bool lock(bool raise_error = true);
  void unlock();

 private:
  bool already_locked();

 private:
  /** Handle to THD object */
  THD *m_thd;
  /** Lock mode */
  Acl_cache_lock_mode m_mode;
  /** Lock status */
  bool m_locked;
};

/**
  Cache to store the Restrictions of every auth_id.
  This cache is not thread safe.
  Callers must acquire acl_cache_write_lock before to amend the cache.
  Callers should acquire acl_cache_read_lock to probe the cache.

  Acl_restrictions is not part of ACL_USER because as of now latter is POD
  type class. We use copy-POD for ACL_USER that makes the explicit memory
  management of its members hard.
*/
class Acl_restrictions {
 public:
  Acl_restrictions();

  Acl_restrictions(const Acl_restrictions &) = delete;
  Acl_restrictions(Acl_restrictions &&) = delete;
  Acl_restrictions &operator=(const Acl_restrictions &) = delete;
  Acl_restrictions &operator=(Acl_restrictions &&) = delete;

  void remove_restrictions(const ACL_USER *acl_user);
  void upsert_restrictions(const ACL_USER *acl_user,
                           const Restrictions &restriction);

  Restrictions find_restrictions(const ACL_USER *acl_user) const;
  size_t size() const;

 private:
  malloc_unordered_map<std::string, Restrictions> m_restrictions_map;
};

ACL_DB *acl_remove_db(ACL_DB *acl_db);

#endif /* SQL_USER_CACHE_INCLUDED */
