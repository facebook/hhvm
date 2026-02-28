/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.
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

#include "sql/auth/sql_security_ctx.h"

#include <map>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_internal.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_authorization.h"
#include "sql/current_thd.h"
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/table.h"

extern bool initialized;

Security_context::Security_context(THD *thd /*= nullptr */)
    : m_restrictions(nullptr), m_thd(thd) {
  init();
}

Security_context::Security_context(MEM_ROOT *mem_root, THD *thd /* = nullptr*/)
    : m_restrictions(mem_root), m_thd(thd) {
  init();
}

Security_context::~Security_context() { destroy(); }

Security_context::Security_context(const Security_context &src_sctx)
    : m_restrictions(nullptr), m_thd(nullptr) {
  copy_security_ctx(src_sctx);
}

Security_context &Security_context::operator=(
    const Security_context &src_sctx) {
  DBUG_TRACE;

  if (this != &src_sctx) {
    destroy();
    copy_security_ctx(src_sctx);
  }

  return *this;
}

void Security_context::init() {
  DBUG_TRACE;

  m_user.set((const char *)nullptr, 0, system_charset_info);
  m_host.set("", 0, system_charset_info);
  m_ip.set("", 0, system_charset_info);
  m_host_or_ip.set(STRING_WITH_LEN("connecting host"), system_charset_info);
  m_external_user.set("", 0, system_charset_info);
  m_priv_user[0] = m_priv_host[0] = m_proxy_user[0] = '\0';
  m_priv_user_length = m_priv_host_length = m_proxy_user_length = 0;
  m_master_access = 0;
  m_db_access = NO_ACCESS;
  m_acl_map = nullptr;
  m_map_checkout_count = 0;
  m_password_expired = false;
  m_is_locked = false;
  m_is_skip_grants_user = false;
  m_has_drop_policy = false;
  m_executed_drop_policy = false;
}

void Security_context::logout() {
  if (m_acl_map) {
    DBUG_PRINT("info",
               ("(logout) Security_context for %s@%s returns Acl_map to cache. "
                "Map reference count= %u",
                m_user.c_ptr(), m_host.c_ptr(), m_acl_map->reference_count()));
    get_global_acl_cache()->return_acl_map(m_acl_map);
    m_acl_map = nullptr;
    clear_active_roles();
    clear_db_restrictions();
  }
}

bool Security_context::has_drop_policy(void) { return m_has_drop_policy; }

bool Security_context::has_executed_drop_policy(void) {
  return m_executed_drop_policy;
}

void Security_context::execute_drop_policy(void) {
  if (m_has_drop_policy && !m_executed_drop_policy) {
    (*m_drop_policy)(this);
    m_executed_drop_policy = true;
  }
}

void Security_context::set_drop_policy(
    const std::function<void(Security_context *)> &func) {
  m_drop_policy.reset(new std::function<void(Security_context *)>(func));
  m_has_drop_policy = true;
  m_executed_drop_policy = false;
}

void Security_context::destroy() {
  DBUG_TRACE;
  execute_drop_policy();
  if (m_acl_map) {
    DBUG_PRINT(
        "info",
        ("(destroy) Security_context for %s@%s returns Acl_map to cache. "
         "Map reference count= %u",
         m_user.c_ptr(), m_host.c_ptr(), m_acl_map->reference_count()));
    get_global_acl_cache()->return_acl_map(m_acl_map);
    clear_active_roles();
  }
  m_acl_map = nullptr;
  if (m_user.length())
    m_user.set((const char *)nullptr, 0, system_charset_info);

  if (m_host.length()) m_host.set("", 0, system_charset_info);

  if (m_ip.length()) m_ip.set("", 0, system_charset_info);

  if (m_host_or_ip.length()) m_host_or_ip.set("", 0, system_charset_info);

  if (m_external_user.length()) m_external_user.set("", 0, system_charset_info);

  m_priv_user[0] = m_priv_host[0] = m_proxy_user[0] = 0;
  m_priv_user_length = m_priv_host_length = m_proxy_user_length = 0;

  m_master_access = m_db_access = 0;
  m_password_expired = false;
  m_is_skip_grants_user = false;
  clear_db_restrictions();
}

/**
  Grants all privilegs to user. Sets the user and host name of privilege user.

  @param[in]  user User name for current_user to set.
                   Default value is "skip-grants user"
  @param[in]  host Host name for the current user to set.
                   Default value is "skip-grants host"
*/
void Security_context::skip_grants(const char *user /*= "skip-grants user"*/,
                                   const char *host /*= "skip-grants host"*/) {
  DBUG_TRACE;

  /* privileges for the user are unknown everything is allowed */
  set_host_or_ip_ptr("", 0);
  assign_priv_user(user, strlen(user));
  assign_priv_host(host, strlen(host));
  m_master_access = ~NO_ACCESS;
  m_is_skip_grants_user = true;

  /*
    If the security context is tied upto to the THD object and it is
    current security context in THD then set the flag to true.
  */
  if (m_thd && m_thd->security_context() == this) {
    m_thd->set_system_user(true);
  }
}

/**
  Deep copy status of sctx object to this.

  @param[in]    src_sctx   Object from which status should be copied.
*/

void Security_context::copy_security_ctx(const Security_context &src_sctx) {
  DBUG_TRACE;

  assign_user(src_sctx.m_user.ptr(), src_sctx.m_user.length());
  assign_host(src_sctx.m_host.ptr(), src_sctx.m_host.length());
  assign_ip(src_sctx.m_ip.ptr(), src_sctx.m_ip.length());
  if (!strcmp(src_sctx.m_host_or_ip.ptr(), my_localhost))
    set_host_or_ip_ptr(my_localhost, strlen(my_localhost));
  else
    set_host_or_ip_ptr();
  assign_external_user(src_sctx.m_external_user.ptr(),
                       src_sctx.m_external_user.length());
  assign_priv_user(src_sctx.m_priv_user, src_sctx.m_priv_user_length);
  assign_proxy_user(src_sctx.m_proxy_user, src_sctx.m_proxy_user_length);
  assign_priv_host(src_sctx.m_priv_host, src_sctx.m_priv_host_length);
  m_db_access = src_sctx.m_db_access;
  m_master_access = src_sctx.m_master_access;
  m_password_expired = src_sctx.m_password_expired;
  m_acl_map =
      nullptr;  // acl maps are reference counted we can't copy or share them!
  m_has_drop_policy = false;  // you cannot copy a drop policy
  m_executed_drop_policy = false;
  m_restrictions = src_sctx.restrictions();
}

/**
  Initialize this security context from the passed in credentials
  and activate it in the current thread.

  @param       thd
  @param       definer_user
  @param       definer_host
  @param       db
  @param[out]  backup  Save a pointer to the current security context
                       in the thread. In case of success it points to the
                       saved old context, otherwise it points to NULL.
  @param       force   Force context switch


  @note The Security_context_factory should be used as a replacement to this
    function at every opportunity.

  During execution of a statement, multiple security contexts may
  be needed:
  - the security context of the authenticated user, used as the
    default security context for all top-level statements
  - in case of a view or a stored program, possibly the security
    context of the definer of the routine, if the object is
    defined with SQL SECURITY DEFINER option.

  The currently "active" security context is parameterized in THD
  member security_ctx. By default, after a connection is
  established, this member points at the "main" security context
  - the credentials of the authenticated user.

  Later, if we would like to execute some sub-statement or a part
  of a statement under credentials of a different user, e.g.
  definer of a procedure, we authenticate this user in a local
  instance of Security_context by means of this method (and
  ultimately by means of acl_getroot), and make the
  local instance active in the thread by re-setting
  thd->m_security_ctx pointer.

  Note, that the life cycle and memory management of the "main" and
  temporary security contexts are different.
  For the main security context, the memory for user/host/ip is
  allocated on system heap, and the THD class frees this memory in
  its destructor. The only case when contents of the main security
  context may change during its life time is when someone issued
  CHANGE USER command.
  Memory management of a "temporary" security context is
  responsibility of the module that creates it.

  @retval true  there is no user with the given credentials. The erro
                is reported in the thread.
  @retval false success
*/

bool Security_context::change_security_context(
    THD *thd, const LEX_CSTRING &definer_user, const LEX_CSTRING &definer_host,
    const char *db, Security_context **backup, bool force) {
  bool needs_change;

  DBUG_TRACE;

  DBUG_ASSERT(definer_user.str && definer_host.str);

  *backup = nullptr;
  needs_change =
      (strcmp(definer_user.str, thd->security_context()->priv_user().str) ||
       my_strcasecmp(system_charset_info, definer_host.str,
                     thd->security_context()->priv_host().str));
  if (needs_change || force) {
    if (acl_getroot(thd, this, definer_user.str, definer_host.str,
                    definer_host.str, db)) {
      my_error(ER_NO_SUCH_USER, MYF(0), definer_user.str, definer_host.str);
      return true;
    }
    *backup = thd->security_context();
    thd->set_security_context(this);
  }

  return false;
}

void Security_context::restore_security_context(THD *thd,
                                                Security_context *backup) {
  if (backup) thd->set_security_context(backup);
}

bool Security_context::user_matches(Security_context *them) {
  DBUG_TRACE;

  const char *them_user = them->user().str;

  return (m_user.ptr() != nullptr) && (them_user != nullptr) &&
         !strcmp(m_user.ptr(), them_user);
}

bool Security_context::check_access(ulong want_access,
                                    const std::string &db_name /* = "" */,
                                    bool match_any) {
  DBUG_TRACE;
  if ((want_access & DB_ACLS) &&
      (is_access_restricted_on_db(want_access, db_name))) {
    return false;
  }
  return (match_any ? (m_master_access & want_access)
                    : ((m_master_access & want_access) == want_access));
}

ulong Security_context::master_access(const std::string &db_name) const {
  return filter_access(m_master_access, db_name);
}

/**
  This method pushes a role to the list of active roles. It requires
  Acl_cache_lock_guard.

  This method allocates memory which must be freed when the role is
  deactivated.

  @param role The role name
  @param role_host The role hostname-part.
  @param validate_access True if access validation should be performed.
    Default value is false.
*/
int Security_context::activate_role(LEX_CSTRING role, LEX_CSTRING role_host,
                                    bool validate_access) {
  auto res = std::find(m_active_roles.begin(), m_active_roles.end(),
                       create_authid_from(role, role_host));
  /* silently ignore requests of activating an already active role */
  if (res != m_active_roles.end()) return 0;
  LEX_CSTRING dup_role = {
      my_strdup(PSI_NOT_INSTRUMENTED, role.str, MYF(MY_WME)), role.length};
  LEX_CSTRING dup_role_host = {
      my_strdup(PSI_NOT_INSTRUMENTED, role_host.str, MYF(MY_WME)),
      role_host.length};
  if (validate_access && !check_if_granted_role(priv_user(), priv_host(),
                                                dup_role, dup_role_host)) {
    my_free(const_cast<char *>(dup_role.str));
    my_free(const_cast<char *>(dup_role_host.str));
    return ER_ACCESS_DENIED_ERROR;
  }
  m_active_roles.push_back(std::make_pair(dup_role, dup_role_host));
  return 0;
}

/**
  Subscribes to a cache entry of aggregated ACLs.
  A Security_context can only have one subscription at a time. If another one
  is requested, the former will be returned.

  We do this subscription before execution of every statement(prepared or
  conventional) as the global acl version might have increased due to
  a grant/revoke or flush. Hence, the granularity of after effects of
  grant/revoke or flush due to roles is per statement.
*/
void Security_context::checkout_access_maps(void) {
  DBUG_TRACE;

  /*
    If we're checkout out a map before we return it now, because we're only
    allowed to have one map at a time.
    However, if we've just authenticated we don't need to checkout a new map
    so we check if there has been any previous checkouts.
  */
  if (m_acl_map != nullptr) {
    DBUG_PRINT(
        "info",
        ("(checkout) Security_context for %.*s@%.*s returns Acl_map to cache. "
         "Map reference count= %u",
         (int)m_priv_user_length, m_priv_user, (int)m_priv_host_length,
         m_priv_host, m_acl_map->reference_count()));
    get_global_acl_cache()->return_acl_map(m_acl_map);
    m_acl_map = nullptr;
  }

  if (m_active_roles.size() == 0) return;
  ++m_map_checkout_count;
  Auth_id_ref uid;
  uid.first.str = this->m_priv_user;
  uid.first.length = this->m_priv_user_length;
  uid.second.str = this->m_priv_host;
  uid.second.length = this->m_priv_host_length;
  m_acl_map =
      get_global_acl_cache()->checkout_acl_map(this, uid, m_active_roles);
  if (m_acl_map != nullptr) {
    DBUG_PRINT("info",
               ("Roles are active and global access for %.*s@%.*s is set to"
                " %lu",
                (int)m_priv_user_length, m_priv_user, (int)m_priv_host_length,
                m_priv_host, m_acl_map->global_acl()));
    set_master_access(m_acl_map->global_acl(), m_acl_map->restrictions());
  } else {
    set_master_access(0);
  }
}

/**
  This helper method clears the active roles list and frees the allocated
  memory used for any previously activated roles.
*/
void Security_context::clear_active_roles(void) {
  for (List_of_auth_id_refs::iterator it = m_active_roles.begin();
       it != m_active_roles.end(); ++it) {
    my_free(const_cast<char *>(it->first.str));
    it->first.str = nullptr;
    it->first.length = 0;
    my_free(const_cast<char *>(it->second.str));
    it->second.str = nullptr;
    it->second.length = 0;
  }
  m_active_roles.clear();
  /*
    Clear does not actually free the memory as an optimization for reuse.
    This confuses valgrind, so we swap with an empty vector to ensure the
    memory is freed when testing with valgrind
  */
  List_of_auth_id_refs().swap(m_active_roles);
}

List_of_auth_id_refs *Security_context::get_active_roles(void) {
  return &m_active_roles;
}

size_t Security_context::get_num_active_roles(void) const {
  return m_active_roles.size();
}

/**
  Get sorted list of roles in LEX_USER format

  @param [in]  thd  For mem_root
  @param [out] list List of active roles
*/
void Security_context::get_active_roles(THD *thd, List<LEX_USER> &list) {
  List_of_granted_roles roles;
  for (const auto &role : m_active_roles) {
    roles.push_back(std::make_pair(Role_id(role.first, role.second), false));
  }
  if (roles.size()) std::sort(roles.begin(), roles.end());
  for (const auto &role : roles) {
    LEX_STRING user, host;
    user.str = strmake_root(thd->mem_root, role.first.user().c_str(),
                            role.first.user().length());
    user.length = role.first.user().length();
    host.str = strmake_root(thd->mem_root, role.first.host().c_str(),
                            role.first.host().length());
    host.length = role.first.host().length();
    LEX_USER *user_lex = LEX_USER::alloc(thd, &user, &host);
    list.push_back(user_lex);
  }
}

ulong Security_context::db_acl(LEX_CSTRING db, bool use_pattern_scan) const {
  DBUG_TRACE;
  if (m_acl_map == nullptr || db.length == 0) return 0;

  std::string key(db.str, db.length);
  Db_access_map::iterator found_acl_it = m_acl_map->db_acls()->find(key);
  if (found_acl_it == m_acl_map->db_acls()->end()) {
    if (use_pattern_scan) {
      Db_access_map::iterator it = m_acl_map->db_wild_acls()->begin();
      ulong access = 0;
      for (; it != m_acl_map->db_wild_acls()->end(); ++it) {
        /*
          Do the usual string comparision if partial_revokes is ON,
          otherwise do the wildcard grant comparision
        */
        if (mysqld_partial_revokes()
                ? (my_strcasecmp(system_charset_info, db.str,
                                 it->first.c_str()) == 0)
                : (wild_case_compare(system_charset_info, db.str, db.length,
                                     it->first.c_str(),
                                     it->first.size()) == 0)) {
          DBUG_PRINT("info", ("Found matching db pattern %s for key %s",
                              it->first.c_str(), key.c_str()));
          access |= it->second;
        }
      }
      return filter_access(access, key);
    } else {
      DBUG_PRINT("info", ("Db %s not found in cache (no pattern matching)",
                          key.c_str()));
      return 0;
    }
  } else {
    DBUG_PRINT("info", ("Found exact match for db %s", key.c_str()));
    return filter_access(found_acl_it->second, key);
  }
}

ulong Security_context::procedure_acl(LEX_CSTRING db,
                                      LEX_CSTRING procedure_name) {
  if (m_acl_map == nullptr)
    return 0;
  else {
    SP_access_map::iterator it;
    String q_name;
    append_identifier(&q_name, db.str, db.length);
    q_name.append(".");
    append_identifier(&q_name, procedure_name.str, procedure_name.length);
    it = m_acl_map->sp_acls()->find(q_name.c_ptr());
    if (it == m_acl_map->sp_acls()->end()) return 0;
    return filter_access(it->second, q_name.c_ptr());
  }
}

ulong Security_context::function_acl(LEX_CSTRING db, LEX_CSTRING func_name) {
  if (m_acl_map == nullptr)
    return 0;
  else {
    String q_name;
    append_identifier(&q_name, db.str, db.length);
    q_name.append(".");
    append_identifier(&q_name, func_name.str, func_name.length);
    SP_access_map::iterator it;
    it = m_acl_map->func_acls()->find(q_name.c_ptr());
    if (it == m_acl_map->func_acls()->end()) return 0;
    return filter_access(it->second, q_name.c_ptr());
  }
}

// return the entire element instead of just the acl?
Grant_table_aggregate Security_context::table_and_column_acls(
    LEX_CSTRING db, LEX_CSTRING table) {
  if (m_acl_map == nullptr) return Grant_table_aggregate();
  Table_access_map::iterator it;
  String q_name;
  append_identifier(&q_name, db.str, db.length);
  q_name.append(".");
  append_identifier(&q_name, table.str, table.length);
  it = m_acl_map->table_acls()->find(std::string(q_name.c_ptr_quick()));
  if (it == m_acl_map->table_acls()->end()) return Grant_table_aggregate();
  return it->second;
}

ulong Security_context::table_acl(LEX_CSTRING db, LEX_CSTRING table) {
  if (m_acl_map == nullptr) return 0;
  Grant_table_aggregate aggr = table_and_column_acls(db, table);
  return filter_access(aggr.table_access, db.str ? db.str : "");
}

bool Security_context::has_with_admin_acl(const LEX_CSTRING &role_name,
                                          const LEX_CSTRING &role_host) {
  DBUG_TRACE;
  if (m_acl_map == nullptr) return false;
  String q_name;
  append_identifier(&q_name, role_name.str, role_name.length);
  q_name.append("@");
  append_identifier(&q_name, role_host.str, role_host.length);
  Grant_acl_set::iterator it =
      m_acl_map->grant_acls()->find(std::string(q_name.c_ptr_quick()));
  if (it != m_acl_map->grant_acls()->end()) return true;
  return false;
}

bool Security_context::any_sp_acl(const LEX_CSTRING &db) {
  if ((db_acl(db, true) & PROC_ACLS) != 0) return true;
  SP_access_map::iterator it = m_acl_map->sp_acls()->begin();
  for (; it != m_acl_map->sp_acls()->end(); ++it) {
    String id_db;
    append_identifier(&id_db, db.str, db.length);
    if (it->first.compare(0, id_db.length(), id_db.c_ptr(), id_db.length()) ==
        0) {
      /* There's at least one SP with grants for this db */
      return true;
    }
  }
  return false;
}

bool Security_context::any_table_acl(const LEX_CSTRING &db) {
  if ((db_acl(db, true) & TABLE_ACLS) != 0) return true;
  Table_access_map::iterator table_it = m_acl_map->table_acls()->begin();
  for (; table_it != m_acl_map->table_acls()->end(); ++table_it) {
    String id_db;
    append_identifier(&id_db, db.str, db.length);
    if (table_it->first.compare(0, id_db.length(), id_db.c_ptr(),
                                id_db.length()) == 0) {
      /* There's at least one table with grants for this db*/
      return true;
    }
  }
  return false;
}

/**
  Checks if the Current_user has the asked dynamic privilege.

  if the server is initializing the datadir, or current_user is
  --skip-grants-user then it returns that user has privilege with
  grant option.

  @param [in] priv      privilege to check
  @param [in] priv_len  length of privilege

  @returns  pair/<has_privilege, has_with_grant_option/>
    @retval /<true, true/>  has required privilege with grant option
    @retval /<true, false/> has required privilege without grant option
    @retval /<false, false/> does not have the required privilege
*/
std::pair<bool, bool> Security_context::has_global_grant(const char *priv,
                                                         size_t priv_len) {
  /* server started with --skip-grant-tables */
  if (!initialized || m_is_skip_grants_user) return std::make_pair(true, true);

  std::string privilege(priv, priv_len);

  if (m_acl_map == nullptr) {
    Acl_cache_lock_guard acl_cache_lock(current_thd,
                                        Acl_cache_lock_mode::READ_MODE);
    if (!acl_cache_lock.lock(false)) return std::make_pair(false, false);
    Role_id key(&m_priv_user[0], m_priv_user_length, &m_priv_host[0],
                m_priv_host_length);
    User_to_dynamic_privileges_map::iterator it, it_end;
    std::tie(it, it_end) = get_dynamic_privileges_map()->equal_range(key);
    it = std::find(it, it_end, privilege);
    if (it != it_end) {
      return std::make_pair(true, it->second.second);
    }
    return std::make_pair(false, false);
  }
  Dynamic_privileges::iterator it =
      m_acl_map->dynamic_privileges()->find(privilege);
  if (it != m_acl_map->dynamic_privileges()->end()) {
    return std::make_pair(true, it->second);
  }

  return std::make_pair(false, false);
}

/**
  Checks if the Auth_id have the asked dynamic privilege.

  @param [in] auth_id     Auth_id that could represent either a user or a role
  @param [in] privilege   privilege to check for
  @param [in] cumulative  Flag to decide how to fetch the privileges of ACL_USER
                          false - privilege granted directly or set through
                                  a role
                          true  - privileges granted directly or coming through
                                  roles granted to it irrespective the roles are
                                  active or not.

  @returns  pair/<has_privilege, has_with_grant_option/>
    @retval /<true, true/>  has required privilege with grant option
    @retval /<true, false/> has required privilege without grant option
    @retval /<false, false/> does not have the required privilege, OR
                             auth_id does not exist.
*/
std::pair<bool, bool> Security_context::has_global_grant(
    const Auth_id &auth_id, const std::string &privilege,
    bool cumulative /*= false*/) {
  std::pair<bool, bool> has_privilege{false, false};
  Acl_cache_lock_guard acl_cache_lock(current_thd,
                                      Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return has_privilege;

  ACL_USER *acl_user =
      find_acl_user(auth_id.host().c_str(), auth_id.user().c_str(), true);
  if (!acl_user) return has_privilege;

  return fetch_global_grant(*acl_user, privilege, cumulative);
}

/**
  Checks if the specified auth_id with privilege can work with the current_user.
  If the auth_id has the specified privilege then current_user must also have
  the same privilege. Throws error is the auth_id has the privilege but
  current_user does not have it.

  @param  [in]  auth_id     Auth_id that could represent either a user or a role
  @param  [in]  privilege   Privilege to check for mismatch
  @param  [in]  cumulative  Flag to decide how to check the privileges of
                            auth_id
                            false - privilege granted directly or set
                                    through a role
                            true  - privileges granted directly or coming
                                    through roles granted to it irrespective
                                    the roles are active or not.
  @param  [in]  ignore_if_nonextant Flag to decide how to treat the non-existing
                                    auth_id.
                                    true  - consider as privilege exists
                                    false - consider as privilege do not exist

  @retval true    auth_id has the privilege but the current_auth does not
  @retval false   Otherwise
*/
bool Security_context::can_operate_with(const Auth_id &auth_id,
                                        const std::string &privilege,
                                        bool cumulative /*= false */,
                                        bool ignore_if_nonextant /*= true */) {
  DBUG_TRACE;
  Acl_cache_lock_guard acl_cache_lock(current_thd,
                                      Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) {
    DBUG_PRINT("error", ("Could not check for the SYSTEM_USER privilege. "
                         "Could not lock Acl caches.\n"));
    return true;
  }
  ACL_USER *acl_user =
      find_acl_user(auth_id.host().c_str(), auth_id.user().c_str(), true);
  if (!acl_user) {
    if (ignore_if_nonextant)
      return false;
    else {
      my_error(ER_USER_DOES_NOT_EXIST, MYF(0), auth_id.auth_str().c_str());
      return true;
    }
  }

  bool is_mismatch = false;
  if (fetch_global_grant(*acl_user, privilege, cumulative).first) {
    is_mismatch = has_global_grant(privilege.c_str(), privilege.length()).first
                      ? false
                      : true;
  }
  if (is_mismatch) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), privilege.c_str());
  }
  return is_mismatch;
}

LEX_CSTRING Security_context::priv_user() const {
  LEX_CSTRING priv_user;
  DBUG_TRACE;
  priv_user.str = m_priv_user;
  priv_user.length = m_priv_user_length;
  return priv_user;
}

/**
  Getter method for member m_user.

  @retval LEX_CSTRING object having constant pointer to m_user.Ptr
          and its length.
*/
LEX_CSTRING Security_context::user() const {
  LEX_CSTRING user;

  DBUG_TRACE;

  user.str = m_user.ptr();
  user.length = m_user.length();

  return user;
}

/**
  Setter method for member m_user.
  Function just sets the user_arg pointer value to the
  m_user, user_arg value is *not* copied.

  @param[in]    user_arg         New user value for m_user.
  @param[in]    user_arg_length  Length of "user_arg" param.
*/

void Security_context::set_user_ptr(const char *user_arg,
                                    const size_t user_arg_length) {
  DBUG_TRACE;

  if (user_arg == m_user.ptr()) return;

  // set new user value to m_user.
  m_user.set(user_arg, user_arg_length, system_charset_info);
}

/**
  Setter method for member m_user.

  Copies user_arg value to the m_user if it is not null else m_user is set
  to NULL.

  @param[in]    user_arg         New user value for m_user.
  @param[in]    user_arg_length  Length of "user_arg" param.
*/

void Security_context::assign_user(const char *user_arg,
                                   const size_t user_arg_length) {
  DBUG_TRACE;

  if (user_arg == m_user.ptr()) return;

  if (user_arg)
    m_user.copy(user_arg, user_arg_length, system_charset_info);
  else
    m_user.set((const char *)nullptr, 0, system_charset_info);
}

/**
  Getter method for member m_host.

  @retval LEX_CSTRING object having constant pointer to m_host.Ptr
          and its length.
*/
LEX_CSTRING Security_context::host() const {
  LEX_CSTRING host;

  DBUG_TRACE;

  host.str = m_host.ptr();
  host.length = m_host.length();

  return host;
}

/**
  Setter method for member m_host.
  Function just sets the host_arg pointer value to the
  m_host, host_arg value is *not* copied.
  host_arg value must not be NULL.

  @param[in]    host_arg         New user value for m_host.
  @param[in]    host_arg_length  Length of "host_arg" param.
*/
void Security_context::set_host_ptr(const char *host_arg,
                                    const size_t host_arg_length) {
  DBUG_TRACE;

  DBUG_ASSERT(host_arg != nullptr);

  if (host_arg == m_host.ptr()) return;

  // set new host value to m_host.
  m_host.set(host_arg, host_arg_length, system_charset_info);
}

/**
  Setter method for member m_host.

  Copies host_arg value to the m_host if it is not null else m_user is set
  to empty string.


  @param[in]    host_arg         New user value for m_host.
  @param[in]    host_arg_length  Length of "host_arg" param.
*/

void Security_context::assign_host(const char *host_arg,
                                   const size_t host_arg_length) {
  DBUG_TRACE;

  if (host_arg == nullptr) {
    m_host.set("", 0, system_charset_info);
    goto end;
  } else if (host_arg == m_host.ptr()) {
    goto end;
  } else if (*host_arg) {
    m_host.copy(host_arg, host_arg_length, system_charset_info);
    goto end;
  }

end:
  return;
}

/**
  Getter method for member m_ip.

  @retval LEX_CSTRING object having constant pointer to m_ip.Ptr
          and its length
*/
LEX_CSTRING Security_context::ip() const {
  LEX_CSTRING ip;

  DBUG_TRACE;

  ip.str = m_ip.ptr();
  ip.length = m_ip.length();

  return ip;
}

/**
  Setter method for member m_ip.
  Function just sets the ip_arg pointer value to the
  m_ip, ip_arg value is *not* copied.

  @param[in]    ip_arg         New user value for m_ip.
  @param[in]    ip_arg_length  Length of "ip_arg" param.
*/

void Security_context::set_ip_ptr(const char *ip_arg, const int ip_arg_length) {
  DBUG_TRACE;

  if (ip_arg == m_ip.ptr()) return;

  // set new ip value to m_ip.
  m_ip.set(ip_arg, ip_arg_length, system_charset_info);
}

/**
  Setter method for member m_ip.

  Copies ip_arg value to the m_ip if it is not null else m_ip is set
  to NULL.


  @param[in]    ip_arg         New user value for m_ip.
  @param[in]    ip_arg_length  Length of "ip_arg" param.
*/

void Security_context::assign_ip(const char *ip_arg, const int ip_arg_length) {
  DBUG_TRACE;

  if (ip_arg == m_ip.ptr()) return;

  if (ip_arg)
    m_ip.copy(ip_arg, ip_arg_length, system_charset_info);
  else
    m_ip.set((const char *)nullptr, 0, system_charset_info);
}

/**
  Setter method for member m_external_user.
  Function just sets the ext_user_arg pointer to the
  m_external_user, ext_user_arg is *not* copied.

  @param[in]    ext_user_arg         New user value for m_external_user.
  @param[in]    ext_user_arg_length  Length of "ext_user_arg" param.
*/

void Security_context::set_external_user_ptr(const char *ext_user_arg,
                                             const int ext_user_arg_length) {
  DBUG_TRACE;

  if (ext_user_arg == m_external_user.ptr()) return;

  // set new ip value to m_ip.
  m_external_user.set(ext_user_arg, ext_user_arg_length, system_charset_info);
}

/**
  Setter method for member m_external_user.

  Copies ext_user_arg value to the m_external_user if it is not null
  else m_external_user is set to NULL.

  @param[in]    ext_user_arg         New user value for m_external_user.
  @param[in]    ext_user_arg_length  Length of "ext_user_arg" param.
*/

void Security_context::assign_external_user(const char *ext_user_arg,
                                            const int ext_user_arg_length) {
  DBUG_TRACE;

  if (ext_user_arg == m_external_user.ptr()) return;

  if (ext_user_arg)
    m_external_user.copy(ext_user_arg, ext_user_arg_length,
                         system_charset_info);
  else
    m_external_user.set((const char *)nullptr, 0, system_charset_info);
}

/**
  Setter method for member m_priv_user.

  @param[in]    priv_user_arg         New user value for m_priv_user.
  @param[in]    priv_user_arg_length  Length of "priv_user_arg" param.
*/

void Security_context::assign_priv_user(const char *priv_user_arg,
                                        const size_t priv_user_arg_length) {
  DBUG_TRACE;

  if (priv_user_arg_length) {
    m_priv_user_length =
        std::min(priv_user_arg_length, sizeof(m_priv_user) - 1);
    strmake(m_priv_user, priv_user_arg, m_priv_user_length);
  } else {
    *m_priv_user = 0;
    m_priv_user_length = 0;
  }
}

/**
  Getter method for member m_proxy_user.

  @retval LEX_CSTRING object having constant pointer to m_proxy_user.Ptr
          and its length
*/
LEX_CSTRING Security_context::proxy_user() const {
  LEX_CSTRING proxy_user;

  DBUG_TRACE;

  proxy_user.str = m_proxy_user;
  proxy_user.length = m_proxy_user_length;

  return proxy_user;
}

/**
  Setter method for member m_proxy_user.

  @param[in]    proxy_user_arg         New user value for m_proxy_user.
  @param[in]    proxy_user_arg_length  Length of "proxy_user_arg" param.
*/

void Security_context::assign_proxy_user(const char *proxy_user_arg,
                                         const size_t proxy_user_arg_length) {
  DBUG_TRACE;

  if (proxy_user_arg_length) {
    m_proxy_user_length =
        std::min(proxy_user_arg_length, sizeof(m_proxy_user) - 1);
    strmake(m_proxy_user, proxy_user_arg, m_proxy_user_length);
  } else {
    *m_proxy_user = 0;
    m_proxy_user_length = 0;
  }
}

/**
  Getter method for member m_priv_host.

  @retval LEX_CSTRING object having constant pointer to m_priv_host.Ptr
          and its length
*/
LEX_CSTRING Security_context::priv_host() const {
  LEX_CSTRING priv_host;

  DBUG_TRACE;

  priv_host.str = m_priv_host;
  priv_host.length = m_priv_host_length;

  return priv_host;
}

/**
  Setter method for member m_priv_host.

  @param[in]    priv_host_arg         New user value for m_priv_host.
  @param[in]    priv_host_arg_length  Length of "priv_host_arg" param.
*/

void Security_context::assign_priv_host(const char *priv_host_arg,
                                        const size_t priv_host_arg_length) {
  DBUG_TRACE;

  if (priv_host_arg_length) {
    m_priv_host_length =
        std::min(priv_host_arg_length, sizeof(m_priv_host) - 1);
    strmake(m_priv_host, priv_host_arg, m_priv_host_length);
  } else {
    *m_priv_host = 0;
    m_priv_host_length = 0;
  }
}

void Security_context::init_restrictions(const Restrictions &restrictions) {
  m_restrictions = restrictions;
}

bool Security_context::is_access_restricted_on_db(
    ulong want_access, const std::string &db_name) const {
  ulong filtered_access = filter_access(want_access, db_name);
  return (filtered_access != want_access);
}

/**
  If there is a restriction attached to an access on the given database
  then remove that access otherwise return the access without any change.

  @param[in]  access    access mask to be scanned to remove
  @param[in]  db_name   database to be searched in the restrictions

  @retval filtered access mask
*/
ulong Security_context::filter_access(const ulong access,
                                      const std::string &db_name) const {
  ulong access_mask = access;
  auto &db_restrictions = m_restrictions.db();
  if (db_restrictions.is_not_empty()) {
    ulong restrictions_mask;
    if (db_restrictions.find(db_name, restrictions_mask))
      access_mask = (access_mask & restrictions_mask) ^ access;
  }
  return access_mask;
}

/**
  Checks if the acl_user does have the asked dynamic privilege.
  This method assumes acl_cache_lock is already taken and ACL_USER is valid

  @param [in] acl_user    ACL_USER to check for privilege
  @param [in] privilege   privilege to check for
  @param [in] cumulative  Flag to decide how to fetch the privileges of ACL_USER
                          false - privilege granted directly or set through
                                  a role
                          true  - privileges granted directly or coming through
                                  roles granted to it irrespective the roles are
                                  active or not.
  @returns  pair/<has_privilege, has_with_grant_option/>
    @retval /<true, true/>   has required privilege with grant option
    @retval /<true, false/>  has required privilege without grant option
    @retval /<false, false/> does not have the required privilege
*/
std::pair<bool, bool> Security_context::fetch_global_grant(
    const ACL_USER &acl_user, const std::string &privilege,
    bool cumulative /*= false */) {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  std::pair<bool, bool> has_privilege{false, false};
  Security_context sctx;

  const char *user = acl_user.user;
  const char *host = acl_user.host.get_host();
  // Anonymous user
  if (user == nullptr) user = "";
  if (host == nullptr) host = "";

  sctx.assign_priv_user(user, strlen(user));
  sctx.assign_priv_host(acl_user.host.get_host(), acl_user.host.get_host_len());
  if (cumulative) {
    activate_all_granted_roles(&acl_user, &sctx);
    sctx.checkout_access_maps();
  }
  /* Check if AuthID being processed has dynamic privilege */
  has_privilege = sctx.has_global_grant(privilege.c_str(), privilege.length());
  return has_privilege;
}

/**
  Check if required access to given table is granted.

  @param [in]     priv Required access
  @param [in,out] tables Table list object

  @returns access information
  @retval true Sucess
  @retval false Failure
 */
bool Security_context::has_table_access(ulong priv, TABLE_LIST *tables) {
  DBUG_TRACE;
  DBUG_ASSERT(tables != nullptr);
  TABLE const *table = tables->table;
  LEX_CSTRING db, table_name;
  db.str = table->s->db.str;
  db.length = table->s->db.length;

  table_name.str = table->alias;
  table_name.length = strlen(table->alias);

  ulong acls = master_access({db.str, db.length});
  if (m_acl_map) {
    if (priv & acls) return true;

    acls = db_acl(db);
    if (priv & acls) return true;

    Grant_table_aggregate aggr = table_and_column_acls(db, table_name);
    acls = aggr.table_access | aggr.cols;
    if (priv & acls) return true;
  } else {
    /* Global and DB priv check */
    if (::check_access(m_thd, priv, db.str, &acls, nullptr, false, true))
      return false;

    if (priv & acls) return true;

    if (::check_grant(m_thd, priv, tables, false, 1, true)) return false;
    return true;
  }
  return false;
}

/**
  Check if required access to given table is not restricted.

  @param [in]     priv Required access
  @param [in,out] table Table object

  @returns access information
  @retval true Access to the table is blocked
  @retval false Access to the table is not blocked
 */
bool Security_context::is_table_blocked(ulong priv, TABLE const *table) {
  DBUG_TRACE;
  DBUG_ASSERT(table != nullptr);
  LEX_CSTRING db, table_name;
  db.str = table->s->db.str;
  db.length = table->s->db.length;

  table_name.str = table->alias;
  table_name.length = strlen(table->alias);

  /* Table privs */
  TABLE_LIST tables;
  tables.table = const_cast<TABLE *>(table);
  tables.db = db.str;
  tables.db_length = db.length;
  tables.table_name = table_name.str;
  tables.table_name_length = table_name.length;
  tables.grant.privilege = NO_ACCESS;

  return !has_table_access(priv, &tables);
}

/**
  Check if required access to given table column is granted.

  @param [in] priv Required access
  @param [in] table Table object
  @param [in] columns List of column names to check

  @returns access information
  @retval true Sucess
  @retval false Failure
 */
bool Security_context::has_column_access(ulong priv, TABLE const *table,
                                         std::vector<std::string> columns) {
  DBUG_TRACE;
  DBUG_ASSERT(table != nullptr);
  LEX_CSTRING db, table_name;
  db.str = table->s->db.str;
  db.length = table->s->db.length;

  table_name.str = table->alias;
  table_name.length = strlen(table->alias);

  /* Table privs */
  TABLE_LIST tables;
  tables.table = const_cast<TABLE *>(table);
  tables.db = db.str;
  tables.db_length = db.length;
  tables.table_name = table_name.str;
  tables.table_name_length = table_name.length;
  tables.grant.privilege = NO_ACCESS;

  // Check that general table access is possible
  if (!has_table_access(priv, &tables)) return false;

  // Try to get info about table specific grants
  {
    Acl_cache_lock_guard acl_cache_lock{this->m_thd,
                                        Acl_cache_lock_mode::READ_MODE};
    if (!acl_cache_lock.lock()) return false;

    if (table_hash_search(this->host().str, this->ip().str, db.str,
                          this->priv_user().str, table_name.str,
                          false) == nullptr) {
      // If there is no specific info about the table specific privileges, it
      // means that there are no column privileges configured for the table
      // columns. So, we let the general table access above to prevail.
      return true;
    }
  }

  for (auto column : columns) {
    if (check_column_grant_in_table_ref(m_thd, &tables, column.data(),
                                        column.length(), priv))
      return false;
  }
  return true;
}
