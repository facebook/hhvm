/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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
#ifndef SQL_SECURITY_CTX_INCLUDED
#define SQL_SECURITY_CTX_INCLUDED
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_hostname.h"  // HOSTNAME_LENGTH
#include "mysql_com.h"    // USERNAME_LENGTH
#include "sql/auth/auth_common.h"
#include "sql/auth/partial_revokes.h"
#include "sql/sql_const.h"
#include "sql_string.h"

/* Forward declaration. Depends on sql_auth_cache.h (which depends on this file)
 */
class Acl_map;
class ACL_USER;
class THD;
struct TABLE;
struct Grant_table_aggregate;

/**
  @class Security_context
  @brief A set of THD members describing the current authenticated user.
*/

class Security_context {
 public:
  Security_context(THD *thd = nullptr);
  Security_context(MEM_ROOT *m_mem_root, THD *thd = nullptr);
  ~Security_context();

  Security_context(const Security_context &src_sctx);

  Security_context &operator=(const Security_context &src_sctx);

  void skip_grants(const char *user = "skip-grants user",
                   const char *host = "skip-grants host");
  bool is_skip_grants_user();

  /**
    Getter method for member m_user.

    @retval LEX_CSTRING object having constant pointer to m_user.Ptr
    and its length.
  */
  LEX_CSTRING user() const;

  void set_user_ptr(const char *user_arg, const size_t user_arg_length);

  void assign_user(const char *user_arg, const size_t user_arg_length);

  std::pair<bool, bool> has_global_grant(const char *priv, size_t priv_len);
  std::pair<bool, bool> has_global_grant(const Auth_id &auth_id,
                                         const std::string &privilege,
                                         bool cumulative = false);
  bool can_operate_with(const Auth_id &auth_id, const std::string &privilege,
                        bool cumulative = false,
                        bool ignore_if_nonextant = true);
  int activate_role(LEX_CSTRING user, LEX_CSTRING host,
                    bool validate_access = false);
  void clear_active_roles(void);
  List_of_auth_id_refs *get_active_roles();
  size_t get_num_active_roles() const;
  void get_active_roles(THD *, List<LEX_USER> &);
  void checkout_access_maps(void);
  ulong db_acl(LEX_CSTRING db, bool use_pattern_scan = true) const;
  ulong procedure_acl(LEX_CSTRING db, LEX_CSTRING procedure_name);
  ulong function_acl(LEX_CSTRING db, LEX_CSTRING procedure_name);
  ulong table_acl(LEX_CSTRING db, LEX_CSTRING table);
  Grant_table_aggregate table_and_column_acls(LEX_CSTRING db,
                                              LEX_CSTRING table);
  bool has_with_admin_acl(const LEX_CSTRING &role_name,
                          const LEX_CSTRING &role_host);
  bool any_sp_acl(const LEX_CSTRING &db);
  bool any_table_acl(const LEX_CSTRING &db);

  bool is_table_blocked(ulong priv, TABLE const *table);
  bool has_column_access(ulong priv, TABLE const *table,
                         std::vector<std::string> column);

  /**
    Getter method for member m_host.

    @retval LEX_CSTRING object having constant pointer to m_host.Ptr
    and its length.
  */

  LEX_CSTRING host() const;

  void set_host_ptr(const char *host_arg, const size_t host_arg_length);

  void assign_host(const char *host_arg, const size_t host_arg_length);

  /**
    Getter method for member m_ip.

    @retval LEX_CSTRING object having constant pointer to m_ip.Ptr
    and its length
  */
  LEX_CSTRING ip() const;

  void set_ip_ptr(const char *ip_arg, const int ip_arg_length);

  void assign_ip(const char *ip_arg, const int ip_arg_length);

  /**
    Getter method for member m_host_or_ip.

    @retval LEX_CSTRING object having constant pointer to m_host_or_ip.Ptr
    and its length
  */
  LEX_CSTRING host_or_ip() const;

  /**
    Setter method for member m_host_or_ip.
  */
  void set_host_or_ip_ptr();

  /**
    Setter method for member m_host_or_ip.

    @param[in]    host_or_ip_arg         New user value for m_host_or_ip.
    @param[in]    host_or_ip_arg_length  Length of "host_or_ip_arg" param.
  */
  void set_host_or_ip_ptr(const char *host_or_ip_arg,
                          const int host_or_ip_arg_length);

  /**
    Getter method for member m_external_user.

    @retval LEX_CSTRING object having constant pointer to m_external_host.Ptr
    and its length
  */
  LEX_CSTRING external_user() const;

  void set_external_user_ptr(const char *ext_user_arg,
                             const int ext_user_arg_length);

  void assign_external_user(const char *ext_user_arg,
                            const int ext_user_arg_length);

  /**
    Getter method for member m_priv_user.

    @retval LEX_CSTRING object having constant pointer to m_priv_user.Ptr
    and its length
  */
  LEX_CSTRING priv_user() const;

  void assign_priv_user(const char *priv_user_arg,
                        const size_t priv_user_arg_length);

  /**
    Getter method for member m_proxy_user.

    @retval LEX_CSTRING object having constant pointer to m_proxy_user.Ptr
    and its length
  */
  LEX_CSTRING proxy_user() const;

  void assign_proxy_user(const char *proxy_user_arg,
                         const size_t proxy_user_arg_length);

  /**
    Getter method for member m_priv_host.

    @retval LEX_CSTRING object having constant pointer to m_priv_host.Ptr
    and its length
  */
  LEX_CSTRING priv_host() const;

  void assign_priv_host(const char *priv_host_arg,
                        const size_t priv_host_arg_length);

  const char *priv_host_name() const;

  /**
    Getter method for member m_master_access.
  */
  ulong master_access() const;

  ulong master_access(const std::string &db_name) const;

  const Restrictions restrictions() const;

  void set_master_access(ulong master_access);

  void set_master_access(ulong master_access, const Restrictions &restrictions);

  /**
    Check if a an account has been assigned to the security context

    The account assigment to the security context is always executed in the
    following order:
    1) assign user's name to the context
    2) assign user's hostname to the context
    Whilst user name can be null, hostname cannot. This is why we can say that
    the full account has been assigned to the context when hostname is not
    equal to empty string.

    @return Account assignment status
      @retval true account has been assigned to the security context
      @retval false account has not yet been assigned to the security context
  */

  bool has_account_assigned() const;

  /**
    Check permission against m_master_access
  */

  /**
    Check global access
    @param want_access The required privileges
    @param db_name The database name to check if it has restrictions attached
    @param match_any if the security context must match all or any of the req.
   *                 privileges.
    @return True if the security context fulfills the access requirements.
  */
  bool check_access(ulong want_access, const std::string &db_name = "",
                    bool match_any = false);

  /**
   Returns the schema level effective privileges (with applied roles)
   for the currently active schema.
  */
  ulong current_db_access() const;

  /**
    Cache the schema level effective privileges (apply roles first!) for the
    currently active schema.
  */
  void cache_current_db_access(ulong db_access);

  /**
    Getter method for member m_password_expired.
  */
  bool password_expired() const;

  void set_password_expired(bool password_expired);

  bool change_security_context(THD *thd, const LEX_CSTRING &definer_user,
                               const LEX_CSTRING &definer_host, const char *db,
                               Security_context **backup, bool force = false);

  void restore_security_context(THD *thd, Security_context *backup);

  bool user_matches(Security_context *);

  void logout();
  /**
    Locked account can still be used as routine definers and when they are
    there shouldn't be any checks for expired passwords.
  */
  bool account_is_locked() { return m_is_locked; }

  void lock_account(bool is_locked) { m_is_locked = is_locked; }

  void set_drop_policy(const std::function<void(Security_context *)> &func);

  void add_as_local_temp_privs(const std::vector<std::string> &privs);
  bool check_in_local_temp_privs(const std::string &priv);

  bool has_drop_policy(void);

  bool has_executed_drop_policy(void);

  void execute_drop_policy(void);

  bool is_access_restricted_on_db(ulong want_access,
                                  const std::string &db_name) const;

  void clear_db_restrictions();

 private:
  void init();
  void destroy();
  void copy_security_ctx(const Security_context &src_sctx);
  ulong filter_access(const ulong access, const std::string &db_name) const;
  void init_restrictions(const Restrictions &restrictions);
  std::pair<bool, bool> fetch_global_grant(const ACL_USER &acl_user,
                                           const std::string &privilege,
                                           bool cumulative = false);
  bool has_table_access(ulong priv, TABLE_LIST *table);

 private:
  /**
    m_user - user of the client, set to NULL until the user has been read from
             the connection
  */
  String m_user;

  /** m_host - host of the client */
  String m_host;

  /** m_ip - client IP */
  String m_ip;

  /**
    m_host_or_ip - points to host if host is available, otherwise points to ip
  */
  String m_host_or_ip;

  String m_external_user;

  /**
    m_priv_user - The user privilege we are using. May be "" for anonymous user.
  */
  char m_priv_user[USERNAME_LENGTH];
  size_t m_priv_user_length;

  char m_proxy_user[USERNAME_LENGTH + HOSTNAME_LENGTH + 6];
  size_t m_proxy_user_length;

  /**
    The host privilege we are using
  */
  char m_priv_host[HOSTNAME_LENGTH + 1];
  size_t m_priv_host_length;

  /**
    Global privileges from mysql.user.
  */
  ulong m_master_access;

  /**
    Privileges for current db
  */
  ulong m_db_access;

  /**
    password expiration flag.

    This flag is set according to connecting user's context and not the
    effective user.
  */
  bool m_password_expired;
  List_of_auth_id_refs m_active_roles;
  Acl_map *m_acl_map;
  int m_map_checkout_count;
  /**
    True if this account can't be logged into.
  */
  bool m_is_locked;
  /**
    True if the skip_grants_user is set.
  */
  bool m_is_skip_grants_user;

  bool m_executed_drop_policy;
  bool m_has_drop_policy;
  std::unique_ptr<std::function<void(Security_context *)>> m_drop_policy;
  Restrictions m_restrictions;

  /**
    m_thd - Thread handle, set to nullptr if this does not belong to any THD yet
  */
  THD *m_thd;
};

/**
  Getter method for member m_host_or_ip.

  @retval LEX_CSTRING object having constant pointer to m_host_or_ip.Ptr
  and its length
*/
inline LEX_CSTRING Security_context::host_or_ip() const {
  LEX_CSTRING host_or_ip;

  DBUG_TRACE;

  host_or_ip.str = m_host_or_ip.ptr();
  host_or_ip.length = m_host_or_ip.length();

  return host_or_ip;
}

inline void Security_context::set_host_or_ip_ptr() {
  DBUG_TRACE;

  /*
  Set host_or_ip to either host or ip if they are available else set it to
  empty string.
  */
  const char *host_or_ip =
      m_host.length() ? m_host.ptr() : (m_ip.length() ? m_ip.ptr() : "");

  m_host_or_ip.set(host_or_ip, strlen(host_or_ip), system_charset_info);
}

inline void Security_context::set_host_or_ip_ptr(
    const char *host_or_ip_arg, const int host_or_ip_arg_length) {
  DBUG_TRACE;

  m_host_or_ip.set(host_or_ip_arg, host_or_ip_arg_length, system_charset_info);
}

inline LEX_CSTRING Security_context::external_user() const {
  LEX_CSTRING ext_user;

  DBUG_TRACE;

  ext_user.str = m_external_user.ptr();
  ext_user.length = m_external_user.length();

  return ext_user;
}

inline ulong Security_context::master_access() const { return m_master_access; }

inline const Restrictions Security_context::restrictions() const {
  return m_restrictions;
}

inline void Security_context::set_master_access(ulong master_access) {
  DBUG_TRACE;
  m_master_access = master_access;
  DBUG_PRINT("info", ("Cached master access is %lu", m_master_access));
}

inline void Security_context::set_master_access(
    ulong master_access, const Restrictions &restrictions) {
  set_master_access(master_access);
  init_restrictions(restrictions);
}

inline const char *Security_context::priv_host_name() const {
  return (*m_priv_host ? m_priv_host : "%");
}

inline bool Security_context::has_account_assigned() const {
  return m_priv_host[0] != '\0';
}

inline ulong Security_context::current_db_access() const { return m_db_access; }

inline void Security_context::cache_current_db_access(ulong db_access) {
  m_db_access = db_access;
}

inline bool Security_context::password_expired() const {
  return m_password_expired;
}

inline void Security_context::set_password_expired(bool password_expired) {
  m_password_expired = password_expired;
}

inline bool Security_context::is_skip_grants_user() {
  return m_is_skip_grants_user;
}

inline void Security_context::clear_db_restrictions() {
  m_restrictions.clear_db();
}

#endif /* SQL_SECURITY_CTX_INCLUDED */
