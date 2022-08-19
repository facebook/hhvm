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

#include "sql/auth/sql_authorization.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <boost/concept/usage.hpp>
#include <boost/function.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphml.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/irange.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <cstdlib>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mf_wcomp.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/plugin_audit.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_internal.h"
#include "sql/auth/auth_utility.h"
#include "sql/auth/dynamic_privilege_table.h"
#include "sql/auth/partial_revokes.h"
#include "sql/auth/role_tables.h"
#include "sql/auth/roles.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/auth/sql_user_table.h"
#include "sql/current_thd.h"
#include "sql/dd/dd_table.h"  // dd::table_exists
#include "sql/debug_sync.h"
#include "sql/derror.h"        /* ER_THD */
#include "sql/error_handler.h" /* error_handler */
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/key_spec.h" /* Key_spec */
#include "sql/mdl.h"
#include "sql/mysqld.h" /* lower_case_table_names */
#include "sql/nested_join.h"
#include "sql/protocol.h"
#include "sql/rpl_slave.h"  /* get_active_master_info */
#include "sql/sp.h"         /* sp_exist_routines */
#include "sql/sql_admin.h"  // enum role_enum
#include "sql/sql_alter.h"
#include "sql/sql_audit.h"
#include "sql/sql_base.h"  /* open_and_lock_tables */
#include "sql/sql_class.h" /* THD */
#include "sql/sql_connect.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"   /* get_current_user */
#include "sql/sql_rewrite.h" /* Grant_params */
#include "sql/sql_show.h"    /* append_identifier */
#include "sql/sql_view.h"    /* VIEW_ANY_ACL */
#include "sql/strfunc.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "violite.h"

/**
   @file sql_authorization.cc

   AUTHORIZATION CODE

*/

/**
  @page AUTHORIZATION_PAGE Authorization IDs, roles and users

  @section AUTHORIZATION_ID Authentication ID
  @subsection AUTH_ID_DEFINITION Definition
 Each row in the mysql.user table is identified by a user and host tuple. This
 tuple is the authorization ID.
 A client can authenticate with an authorization ID and a password. The ID is
 then referred to as a user or user name.

  @section AUTHORIZATION_PRIVILEGES Privileges ID
  @subsection AUTH_PRIV_DEFINITION Definition
 A privilege ID is a named token which can be granted to an authorization ID.

 A privilege can either be effective or not effective. An effective privilege is
 a privilege which used in a session to evaluate if a particular operation is
 allowed or not. All effective privileges are granted or inherited but not all
 privileges are effective.

  @section AUTHORIZATION_ROLES Roles
  @subsection AUTH_ROLES_DEFINITION Definition
 A role is an authorization ID which can be granted to another authorization ID
 by forming an directed edge between them in the role graph where every vertex
 is a unique authorization ID. When the effective privilege is calculated, all
 connected roles are visited according to their edge direction and their
 corresponding granted privileges are aggregated.

  @subsection ACTIVE_ROLE Active roles
 A role can either be active or inactive. Active roles are kept in a thread
 local list which exists solely for the lifetime of a client session. Granted
 roles can be made active by
   1) a SET ROLE statement,
   2) after authentication if the role is a default role,
   3) after authentication if the global variable
opt_always_activate_roles_on_login is set to true.

  Example: To set the grated role ``team``\@``%`` as an active role, after
    authentication, execute: SET ROLE team

  @subsection DEFAULT_ROLE Default roles
 Each authorization ID has a list of default roles. Default roles belonging to
 an authorization ID are made into active roles after authentication iff they
 are granted to this ID. If the list of default roles is empty then no roles are
 made active after authentication unless the client sets a
 SET ROLE statement.

  @subsection MANDATORY_ROLE Mandatory roles
 A mandatory role is an authorization ID which is implicitly granted to every
 other authorization ID which has authenticated, regardless if this role has
 been previously granted or not. Mandatory roles are specified in a global
 variable. It's not required that the specified list maps to any existing
 authorization ID but if there's no previous authorization ID then no mandatory
 role can be granted. Mandatory roles are processed sequentially as any other
 granted role when the effective privilege of an authorization ID needs to be
 calculated iff they are active.

 @section AUTHORIZATION_CACHE The effective privilege cache
 @subsection OVERVIEW Overview
 To avoid recalculating the effective privilege at every step the result is
 saved into a cache (See Acl_cache ). The key to this cache is
 formed by concatenating authorization ID, the active roles and the version ID
 of the cache.

 The cache is a lockless hash storage and each element is assembled using
 read-only operations on the shared role graph.
 @see get_privilege_access_maps

 @section AUTHORIZATION_SHOW_GRANTS SHOW GRANTS

 The statements @code SHOW GRANT @endcode shows all effective privileges using
 the currently active roles for the current user.

 The statement @code SHOW GRANT FOR x USING y @endcode is used for listing the
 effective privilege for x given y as active roles. If If y isn't specified then
 no roles are used. If x isn't specified then the current_user() is used.
 Mandatory roles are always excluded from the list of granted roles when this
 statement is used.

 Example: To show the privilege for a role using no roles:
 @code SHOW GRANTS FOR x. @endcode

 SHOW-statements does not use the privilege cache and the effective privilege is
 recalculated on every execution.
 @see mysql_show_grants

 To show the role graph use @code SELECT roles_graphml() @endcode

 To investigate the role graph use the built in XML functions or the
 mysql.role_edges table.

 */

namespace {
/**
  Class to handle sanity checks for GRANT ... AS ... statement
*/

class Grant_validator {
 public:
  explicit Grant_validator(THD *thd, const char *db,
                           const List<LEX_USER> &user_list, ulong rights,
                           bool revoke,
                           const List<LEX_CSTRING> &dynamic_privilege,
                           bool grant_all, LEX_GRANT_AS *grant_as,
                           TABLE *dynamic_priv_table)
      : m_thd(thd),
        m_db(db),
        m_user_list(user_list),
        m_rights(rights),
        m_revoke(revoke),
        m_dynamic_privilege(dynamic_privilege),
        m_grant_all(grant_all),
        m_grant_as(grant_as),
        m_dynamic_priv_table(dynamic_priv_table),
        m_restore(false),
        m_backup(nullptr) {}
  ~Grant_validator();
  bool validate();

 private:
  bool mask_and_return_error();
  bool validate_system_user_privileges();
  bool validate_dynamic_privileges();
  bool validate_and_process_grant_as();

 private:
  THD *m_thd;
  const char *m_db;
  const List<LEX_USER> &m_user_list;
  ulong m_rights;
  bool m_revoke;
  const List<LEX_CSTRING> &m_dynamic_privilege;
  bool m_grant_all;
  LEX_GRANT_AS *m_grant_as;
  TABLE *m_dynamic_priv_table;
  bool m_restore;
  Security_context *m_backup;
  Security_context m_security_context;
};

/*
  Destructor. Restores original security context.
*/
Grant_validator::~Grant_validator() {
  if (m_restore)
    m_thd->security_context()->restore_security_context(m_thd, m_backup);
}

/**
  Helper function to mask specific error with generic one.

  @returns true always.
*/
bool Grant_validator::mask_and_return_error() {
  DBUG_TRACE;
  /* Restore security context */
  if (m_restore)
    m_thd->security_context()->restore_security_context(m_thd, m_backup);
  m_restore = false;
  /*
    Any error set before this point may potentially give away
    information about user and/or role. So, clear any error
    that may have been raised and replace it with a generic error.
  */
  m_thd->get_stmt_da()->reset_diagnostics_area();
  my_error(ER_UKNOWN_AUTH_ID_OR_ACCESS_DENIED_FOR_GRANT_AS, MYF(0));
  return true;
}

/**
  Perform sanity checks for GRANT ... AS ...

  @returns status of checks
    @retval false Success. Security context may have been changed
    @retval true  Failure. Error has been raised.
*/
bool Grant_validator::validate_and_process_grant_as() {
  DBUG_TRACE;

  if (m_grant_as == nullptr || !m_grant_as->grant_as_used) return false;

  LEX_USER *user = get_current_user(m_thd, m_grant_as->user);
  if (user == nullptr) return mask_and_return_error();

  /* Change security context */
  if (m_security_context.change_security_context(m_thd, user->user, user->host,
                                                 nullptr, &m_backup, true))
    return mask_and_return_error();

  m_restore = true;

  Roles::Role_activation role_activation(m_thd, m_thd->security_context(),
                                         m_grant_as->role_type,
                                         m_grant_as->role_list, false);

  if (role_activation.activate()) return mask_and_return_error();

  /* Compare restrictions */
  Restrictions this_restrictions = m_thd->security_context()->restrictions();
  Restrictions other_restrictions = m_backup->restrictions();
  if (this_restrictions.has_more_db_restrictions(other_restrictions, m_rights))
    return mask_and_return_error();

  return false;
}

/**
  Validate that if grantee has SYSTEM_USER privileges, current user has it too.

  @returns status of the check
    @retval false Success
    @retval true  Current user lacks SYSTEM_USER privilege
*/
bool Grant_validator::validate_system_user_privileges() {
  DBUG_TRACE;
  if (check_system_user_privilege(m_thd, m_user_list)) return true;
  return false;
}

/**
  Permission and sanity checks for dynamic privileges.

  We check:
  1. Dynamic privilege is granted at *.* level
  2. Current user's ability to grant dynamic privilege
  3. SYSTEM_USER is not granted to mandatory roles

  @returns status of checks
    @retval false Success
    @retval true  Error validating dynamic privileges
*/
bool Grant_validator::validate_dynamic_privileges() {
  DBUG_TRACE;

  /* Dynamic privileges are allowed only for global grants */
  if (m_db && m_db != any_db && m_dynamic_privilege.elements > 0) {
    String privs;
    bool comma = false;
    for (const LEX_CSTRING &priv : m_dynamic_privilege) {
      if (comma) privs.append(",");
      privs.append(priv.str, priv.length);
      comma = true;
    }
    my_error(ER_ILLEGAL_PRIVILEGE_LEVEL, MYF(0), privs.c_ptr());
    return true;
  }

  /* Sanity checks for dynamic privileges */
  if (!m_db && (m_dynamic_privilege.elements > 0 || m_grant_all)) {
    LEX_CSTRING *priv;
    Update_dynamic_privilege_table update_table(m_thd, m_dynamic_priv_table);
    List<LEX_CSTRING> *privileges_to_check;
    if (m_grant_all) {
      /*
        Copy all currently available dynamic privileges to the list of
        dynamic privileges to grant.
      */
      privileges_to_check = new (m_thd->mem_root) List<LEX_CSTRING>;
      iterate_all_dynamic_privileges(m_thd, [&](const char *str) {
        LEX_CSTRING *new_str = (LEX_CSTRING *)m_thd->alloc(sizeof(LEX_CSTRING));
        new_str->str = str;
        new_str->length = strlen(str);
        privileges_to_check->push_back(new_str);
        return false;
      });
    } else
      privileges_to_check =
          &const_cast<List<LEX_CSTRING> &>(m_dynamic_privilege);
    List_iterator<LEX_CSTRING> priv_it(*privileges_to_check);
    bool error = false;
    Security_context *sctx = m_thd->security_context();
    while ((priv = priv_it++) && !error) {
      /*
        Privilege to grant dynamic privilege to others is granted if the user
        either has super user privileges (currently UPDATE_ACL on mysql.*) or
        if the user has a GRANT_OPTION on the specific dynamic privilege he
        wants to grant.
        Note that this is different than the rules which apply for other
        privileges since for them the GRANT OPTION applies on a privilege
        scope level (ie global, db or table level).
        From a user POV it might appear confusing that some privileges are
        more strictly associated with GRANT OPTION than others, but this
        choice is made to preserve back compatibility while also paving way
        for future improvements where all privileges objects have their own
        grant option.
      */
      if (check_access(m_thd, UPDATE_ACL, consts::mysql.c_str(), nullptr,
                       nullptr, true, true) &&
          !sctx->has_global_grant(priv->str, priv->length).second) {
        my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "GRANT OPTION");
        return true;
      }

      if (!m_revoke) {
        // Do not grant SYSTEM_USER privilege to a mandatory role
        if (consts::system_user.compare(priv->str) == 0) {
          std::vector<Role_id> mandatory_roles;
          get_mandatory_roles(&mandatory_roles);
          List_iterator<LEX_USER> str_list(
              const_cast<List<LEX_USER> &>(m_user_list));
          LEX_USER *user, *target_user;
          while ((target_user = str_list++)) {
            if (!(user = get_current_user(m_thd, target_user))) {
              my_error(ER_OUTOFMEMORY, MYF(0), sizeof(LEX_USER));
              return true;
            }
            Auth_id_ref auth_id = create_authid_from(user);
            for (const auto &rid : mandatory_roles) {
              if (rid == auth_id) {
                my_error(ER_CANNOT_GRANT_SYSTEM_PRIV_TO_MANDATORY_ROLE, MYF(0),
                         auth_id.first.str, auth_id.second.str, priv->str);
                return true;
              }
            }
          }
        }
      }
    }
  }

  return false;
}

/**
  Umbrella method to perform validation

  A possible side effect of this method is that active security context of the
  session may have been changed. This is true if GRANT ... AS ... is used.

  @returns status of checks
    @retval false Success
    @retval true  Error found during validation
*/
bool Grant_validator::validate() {
  DBUG_TRACE;
  if (validate_system_user_privileges()) return true;
  if (validate_dynamic_privileges()) return true;

  /*
    This must be the last check because it may change
    the active security context of a thread.
  */
  if (validate_and_process_grant_as()) return true;
  return false;
}

/**
  The dynamic privilege is probed in the global map that keeps track of
  dynamic privileges registered with server. The policy is that
  - Plugin/Component may register a privilege ID
  - Any privilege ID that exist in mysql.global_grants is a valid privilege ID

  This method assumes that caller must have acquired the necessory ACL_LOCK.

  @param [in] privilege Privilege to be checked in the dynamic privilege map

  @retval true Privilege is registered
  @retval false Otherwise
*/
bool is_dynamic_privilege_registered(const std::string &privilege) {
  if (get_dynamic_privilege_register()->find(privilege) !=
      get_dynamic_privilege_register()->end()) {
    return true;
  }
  return false;
}
}  // namespace

Granted_roles_graph *g_granted_roles = nullptr;
Role_index_map *g_authid_to_vertex = nullptr;
static char g_active_dummy_user[] = "active dummy user";
extern bool initialized;
extern Default_roles *g_default_roles;
typedef boost::graph_traits<Granted_roles_graph>::adjacency_iterator
    Role_adjacency_iterator;
User_to_dynamic_privileges_map *g_dynamic_privileges_map = nullptr;
const char *any_db = "*any*";  // Special symbol for check_access

static bool check_routine_level_acl(THD *thd, const char *db, const char *name,
                                    bool is_proc);
void get_granted_roles(Role_vertex_descriptor &v,
                       List_of_granted_roles *granted_roles);

/**
  This utility function is used by revoke_role() and remove_all_granted_roles()
  for removing a specific edge from the role graph.
  @param thd Thread handler
  @param authid_role The role which should be revoked
  @param authid_user The user who will get its role revoked
  @param [out] user_vert The vertex descriptor of the user
  @param [out] role_vert The vertex descriptor of the role

  @return Success state
    @retval true No such user
    @retval false User was removed
*/

bool revoke_role_helper(THD *thd MY_ATTRIBUTE((unused)),
                        std::string &authid_role, std::string &authid_user,
                        Role_vertex_descriptor *user_vert,
                        Role_vertex_descriptor *role_vert) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));

  Role_index_map::iterator it = g_authid_to_vertex->find(authid_user);
  if (it == g_authid_to_vertex->end()) {
    // No such user
    return true;
  } else
    *user_vert = it->second;

  it = g_authid_to_vertex->find(authid_role);
  if (it == g_authid_to_vertex->end()) {
    // No such role
    return true;
  } else
    *role_vert = it->second;

  boost::remove_edge(*user_vert, *role_vert, *g_granted_roles);

  return false;
}

/**
  This utility function checks for the connecting vertices of the role
  descriptor(authid node) and updates the role flag of the corresponding
  ACL user. If there are no incoming edges to this authid node then this
  is not a role id anymore. It assumes that acl user and role descriptor
  are, valid and passed correctly.

  @param [in] role_vert The role vertex descriptor
  @param [in,out] acl_user The acl role

*/
void static update_role_flag_of_acl_user(
    const Role_vertex_descriptor &role_vert, ACL_USER *acl_user) {
  degree_s_t count = boost::in_degree(role_vert, *g_granted_roles);
  acl_user->is_role = (count > 0) ? true : false;
}

/**
 Used by mysql_revoke_role() for revoking a specified role from a specified
 user.

 @param thd Thread handler
 @param role The role which will be revoked
 @param user The user who will get its role revoked

*/
void revoke_role(THD *thd, ACL_USER *role, ACL_USER *user) {
  std::string authid_role = create_authid_str_from(role);
  std::string authid_user = create_authid_str_from(user);
  Role_vertex_descriptor user_vert;
  Role_vertex_descriptor role_vert;
  if (!revoke_role_helper(thd, authid_role, authid_user, &user_vert,
                          &role_vert)) {
    update_role_flag_of_acl_user(role_vert, role);
  }
}

/**
    Since the gap in the vertex vector was removed all the vertex descriptors
    has changed. As a consequence we now need to rebuild the authid_to_vertex
    index.
*/
void rebuild_vertex_index(THD *thd MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  for (auto &acl_user : *acl_users) {
    create_role_vertex(&acl_user);
  }
  g_authid_to_vertex->clear();
  boost::graph_traits<Granted_roles_graph>::vertex_iterator vert_it, vert_end;
  boost::tie(vert_it, vert_end) = boost::vertices(*g_granted_roles);
  for (; vert_it != vert_end; ++vert_it) {
    ACL_USER acl_user =
        boost::get(boost::vertex_acl_user_t(),
                   *g_granted_roles)[boost::vertex(*vert_it, *g_granted_roles)];
    if (acl_user.user == g_active_dummy_user) {
      (*g_authid_to_vertex)["root"] = *vert_it;
    } else {
      std::string authid = create_authid_str_from(&acl_user);
      (*g_authid_to_vertex)[authid] = *vert_it;
    }
  }
}

bool drop_role(THD *thd, TABLE *edge_table, TABLE *defaults_table,
               const Auth_id_ref &authid_user) {
  DBUG_TRACE;
  bool error = false;
  std::vector<ACL_USER> users;
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  std::string authid_user_str = create_authid_str_from(authid_user);
  Role_index_map::iterator it;

  if ((it = g_authid_to_vertex->find(authid_user_str)) !=
      g_authid_to_vertex->end()) {
    /* Fetch source vertex details */
    ACL_USER source_acl_user = boost::get(
        boost::vertex_acl_user_t(),
        *g_granted_roles)[boost::vertex(it->second, *g_granted_roles)];
    Auth_id_ref source_user = create_authid_from(&source_acl_user);

    /*
      Lambda function that drops all adjacent edges(if exists) from the
      source_user present in the role_edges table and, keep track of
      target acl user.
      It assumes all the paramaters and captures, are valid and sane.
    */
    auto modify_role_edges = [&thd, &edge_table, &error,
                              &source_user](const ACL_USER &target_acl_user) {
      Auth_id_ref target_user = create_authid_from(&target_acl_user);
      error = modify_role_edges_in_table(thd, edge_table, source_user,
                                         target_user, false, true);
      error |= modify_role_edges_in_table(thd, edge_table, target_user,
                                          source_user, false, true);
    };

    /* Fetch the neighboring vertices from the outgoing edges */
    out_edge_itr_t oute_itr, oute_end;
    boost::tie(oute_itr, oute_end) =
        boost::out_edges(it->second, *g_granted_roles);
    for (; oute_itr != oute_end; ++oute_itr) {
      ACL_USER target_acl_user = boost::get(
          boost::vertex_acl_user_t(),
          *g_granted_roles)[boost::target(*oute_itr, *g_granted_roles)];
      modify_role_edges(target_acl_user);
      users.push_back(target_acl_user);
    }

    /* Fetch the neighboring vertices from the incoming edges */
    in_edge_itr_t ine_itr, ine_end;
    boost::tie(ine_itr, ine_end) =
        boost::in_edges(it->second, *g_granted_roles);
    for (; ine_itr != ine_end; ++ine_itr) {
      ACL_USER target_acl_user = boost::get(
          boost::vertex_acl_user_t(),
          *g_granted_roles)[boost::source(*ine_itr, *g_granted_roles)];
      modify_role_edges(target_acl_user);
    }

    /* Remove this vertex from the graph (along with its edges) */
    DBUG_PRINT("info", ("Removing %s from graph and rebuild the index.",
                        authid_user_str.c_str()));
    /*
      We clear all edges connecting this vertex but we avoid removing it
      from the graph at this time as it would invalidate the vertex
      descriptors and we would have to rebuild all indexes. For now it is
      enough to remove the index entry. As the roles  are reloaded from the
      tables the dropped roles will disappear.
    */
    boost::clear_vertex(it->second, *g_granted_roles);

    /*
      If the role authid does not have any incoming edges then update
      the role flag of corresponding ACL role.
    */
    for (auto &&user_itr : users) {
      Role_index_map::iterator role_it =
          g_authid_to_vertex->find(create_authid_str_from(&user_itr));
      if (role_it != g_authid_to_vertex->end()) {
        ACL_USER *acl_role =
            find_acl_user(user_itr.host.get_host(), user_itr.user, true);
        DBUG_ASSERT(acl_role != nullptr);
        update_role_flag_of_acl_user(role_it->second, acl_role);
      }
    }
  }
  // Remove all default role policies assigned to this authid.
  clear_default_roles(thd, defaults_table, authid_user, nullptr);
  // Remove all default role policies in which this authid is a default role.
  std::vector<Default_roles::iterator> delete_policies;
  for (auto policy = g_default_roles->begin(); policy != g_default_roles->end();
       ++policy) {
    if (policy->second == authid_user) {
      delete_policies.push_back(policy);
    }
  }
  for (auto &&policy : delete_policies) {
    modify_default_roles_in_table(thd, defaults_table,
                                  create_authid_from(policy->first),
                                  create_authid_from(policy->second), true);
    g_default_roles->erase(policy);
  }
  return error;
}

/**
  Used by @ref mysql_drop_user. Will drop all
  @param thd
  @param edge_table
  @param defaults_table
  @param user_name

  @retval true An error occurred
  @retval false Success
*/
bool revoke_all_roles_from_user(THD *thd, TABLE *edge_table,
                                TABLE *defaults_table, LEX_USER *user_name) {
  List_of_granted_roles granted_roles;
  get_granted_roles(user_name, &granted_roles);
  Auth_id_ref user_name_authid = create_authid_from(user_name);
  bool error = drop_role(thd, edge_table, defaults_table, user_name_authid);
  return error;
}

/**
  If possible, it will revoke all roles and default roles from user_from and
  set them for user_to instead.

  @param thd Thread handle
  @param table A table handler
  @param user_from The name of the ACL_USER which will be renamed.
  @param [out] granted_roles A list of roles that were successfully revoked.

  @return success state
    @retval true En error occurred
    @retval false Successful
*/
bool revoke_all_granted_roles(THD *thd, TABLE *table, LEX_USER *user_from,
                              List_of_granted_roles *granted_roles) {
  DBUG_TRACE;
  std::string authid_user = create_authid_str_from(user_from);
  Role_index_map::iterator it;
  if ((it = g_authid_to_vertex->find(authid_user)) ==
      g_authid_to_vertex->end()) {
    /* The user from wasn't in the role graph index; nothing to do. */
    return true;
  }

  get_granted_roles(it->second, granted_roles);
  Role_vertex_descriptor user_vert;
  Role_vertex_descriptor role_vert;
  bool errors = false;
  for (auto &&ref : *granted_roles) {
    std::string role_id_str;
    ref.first.auth_str(&role_id_str);
    std::string user_from_str = create_authid_str_from(user_from);
    Auth_id_ref role_id = create_authid_from(ref.first);
    errors = modify_role_edges_in_table(thd, table, role_id,
                                        {user_from->user, user_from->host},
                                        ref.second, true);
    if (errors) break;
    /*
      If the role is revoked then update the flag in the
      corresponding ACL authid.
    */
    if (!revoke_role_helper(thd, role_id_str, user_from_str, &user_vert,
                            &role_vert)) {
      ACL_USER *acl_role = find_acl_user(ref.first.host().c_str(),
                                         ref.first.user().c_str(), ref.second);
      DBUG_ASSERT(acl_role != nullptr);
      update_role_flag_of_acl_user(role_vert, acl_role);
    }
  }
  return errors;
}

bool is_role_id(LEX_USER *authid) {
  ACL_USER *acl_user = find_acl_user(authid->host.str, authid->user.str, true);
  if (acl_user == nullptr) return false;
  return acl_user->is_role;
}

/**
  Grants a single role to a single user. The change is made to the in-memory
  roles graph and not persistent.

  @see mysql_grant_role

  @param role A pointer to the role to be granted
  @param user A pointer to the user which will be granted
  @param with_admin_opt True if the user should have the ability to pass on the
    granted role to another authorization id.
*/
void grant_role(ACL_USER *role, const ACL_USER *user, bool with_admin_opt) {
  DBUG_TRACE;
  bool is_added;
  std::string authid_role = create_authid_str_from(role);
  std::string authid_user = create_authid_str_from(user);
  Role_vertex_descriptor user_vert, role_vert;
  Role_index_map::iterator it;

  if ((it = g_authid_to_vertex->find(authid_user)) ==
      g_authid_to_vertex->end()) {
    user_vert = boost::add_vertex(*g_granted_roles);
    g_authid_to_vertex->insert(make_pair(authid_user, user_vert));
  } else
    user_vert = it->second;

  if ((it = g_authid_to_vertex->find(authid_role)) ==
      g_authid_to_vertex->end()) {
    role_vert = boost::add_vertex(*g_granted_roles);
    g_authid_to_vertex->insert(make_pair(authid_role, role_vert));
  } else
    role_vert = it->second;

  boost::property_map<Granted_roles_graph, boost::vertex_name_t>::type
      user_pname,
      role_pname;
  user_pname = boost::get(boost::vertex_name_t(), *g_granted_roles);
  boost::put(user_pname, user_vert, authid_user);
  role_pname = boost::get(boost::vertex_name_t(), *g_granted_roles);
  boost::put(role_pname, role_vert, authid_role);

  boost::property_map<Granted_roles_graph, boost::vertex_acl_user_t>::type
      user_pacl_user,
      role_pacl_user;
  user_pacl_user = boost::get(boost::vertex_acl_user_t(), *g_granted_roles);
  boost::put(user_pacl_user, user_vert, *user);
  role_pacl_user = boost::get(boost::vertex_acl_user_t(), *g_granted_roles);
  boost::put(role_pacl_user, role_vert, *role);

  Role_edge_descriptor edge;
  tie(edge, is_added) = add_edge(user_vert, role_vert, *g_granted_roles);

  boost::property_map<Granted_roles_graph, boost::edge_capacity_t>::type
      edge_colors;
  edge_colors = boost::get(boost::edge_capacity_t(), *g_granted_roles);
  boost::put(edge_colors, edge, (with_admin_opt ? 1 : 0));
  role->is_role = true;
}

/**
  Helper function for create_roles_vertices. Creates a vertex in the role
  graph and associate it with an ACL_USER. If the ACL_USER already exists in
  the vertex-to-acl-user index then we ignore this request.

  @param role_acl_user The acial user to be mapped to a vertex.
*/
void create_role_vertex(ACL_USER *role_acl_user) {
  Role_vertex_descriptor role_vertex;
  Role_index_map::iterator it;
  std::string key = create_authid_str_from(role_acl_user);
  if ((it = g_authid_to_vertex->find(key)) == g_authid_to_vertex->end()) {
    role_vertex = boost::add_vertex(*g_granted_roles);
    boost::property_map<Granted_roles_graph, boost::vertex_acl_user_t>::type
        root_prop;
    root_prop = boost::get(boost::vertex_acl_user_t(), *g_granted_roles);
    boost::put(root_prop, role_vertex, *role_acl_user);
    boost::property_map<Granted_roles_graph, boost::vertex_name_t>::type
        role_pname;
    role_pname = boost::get(boost::vertex_name_t(), *g_granted_roles);
    boost::put(role_pname, role_vertex, key);
    g_authid_to_vertex->insert(std::make_pair(key, role_vertex));
  }
}

/**
  Renames a user in the mysql.role_edge and the mysql.default_roles
  tables. user_to must already exist in the acl_user cache, but user_from
  may not as long as it exist in the role graph.

  @param thd Thread handler
  @param edge_table An open table handle for mysql.edge_mysql
  @param defaults_table An open table handle for mysql.default_roles
  @param user_from The user to rename
  @param user_to The target user name

  @see mysql_rename_user

  @retval true An error occurred
  @retval false Success
*/

bool roles_rename_authid(THD *thd, TABLE *edge_table, TABLE *defaults_table,
                         LEX_USER *user_from, LEX_USER *user_to) {
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  ACL_USER *acl_user_to =
      find_acl_user(user_to->host.str, user_to->user.str, true);
  if (acl_user_to == nullptr) {
    /* The target user doesn't exist yet? */
    return true;
  }
  /* Update default roles */
  std::vector<Role_id> old_roles;
  Auth_id_ref authid_user_from = create_authid_from(user_from);
  clear_default_roles(thd, defaults_table, authid_user_from, &old_roles);
  List_of_auth_id_refs new_default_role_ref;
  for (auto &&role : old_roles) {
    Auth_id_ref authid = create_authid_from(role);
    new_default_role_ref.push_back(authid);
  }
  bool ret = alter_user_set_default_roles(thd, defaults_table, user_to,
                                          new_default_role_ref);

  if (ret) {
    String warning;
    append_identifier(thd, &warning, user_from->user.str,
                      user_from->user.length);
    append_identifier(thd, &warning, user_from->host.str,
                      user_from->host.length);
    LogErr(WARNING_LEVEL, ER_SQL_AUTHOR_DEFAULT_ROLES_FAIL, warning.c_ptr());
    ret = false;
  }

  List_of_granted_roles granted_roles;
  ret = revoke_all_granted_roles(thd, edge_table, user_from, &granted_roles);
  if (!ret) {
    for (auto &&ref : granted_roles) {
      ACL_USER *acl_role = find_acl_user(ref.first.host().c_str(),
                                         ref.first.user().c_str(), ref.second);
      if (acl_role == nullptr) {
        /* An invalid reference was encountered; just ignore it. */
        continue;
      }
      grant_role(acl_role, acl_user_to, ref.second);
      Auth_id_ref authid_role = create_authid_from(acl_role);
      Auth_id_ref authid_user = create_authid_from(acl_user_to);
      ret = modify_role_edges_in_table(thd, edge_table, authid_role,
                                       authid_user, ref.second, false);
      if (ret) break;
    }
  }
  return ret;
}

/**
  Maps a global ACL to a string representation.

  @param thd Thread handler
  @param want_access An ACL
  @param acl_user The associated user which carries the ACL
  @param [out] global The resulting string

*/

void make_global_privilege_statement(THD *thd, ulong want_access,
                                     ACL_USER *acl_user, String *global) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  global->length(0);
  global->append(STRING_WITH_LEN("GRANT "));

  if (!(want_access & ~GRANT_ACL))
    global->append(STRING_WITH_LEN("USAGE"));
  else {
    bool found = false;
    ulong test_access = want_access & ~GRANT_ACL;
    int counter = 0;
    ulong j = SELECT_ACL;
    for (; j <= GLOBAL_ACLS; counter++, j <<= 1) {
      if (test_access & j) {
        if (found) global->append(STRING_WITH_LEN(", "));
        found = true;
        global->append(global_acls_vector[counter].c_str(),
                       global_acls_vector[counter].length());
      }
    }
  }
  global->append(STRING_WITH_LEN(" ON *.* TO "));
  size_t len = acl_user->user == nullptr ? 0 : strlen(acl_user->user);
  append_identifier(thd, global, acl_user->user, len);
  global->append('@');
  append_identifier(thd, global, acl_user->host.get_host(),
                    acl_user->host.get_host_len());
  if (want_access & GRANT_ACL)
    global->append(STRING_WITH_LEN(" WITH GRANT OPTION"));
}

/**
  Maps a set of database level ACLs to string representations and sends them
  through the client protocol.

  @param thd The thread handler
  @param role The authid associated with the ACLs
  @param protocol A handler used for sending data to the client
  @param db_map A list of database level ACLs
  @param db_wild_map A list of database level ACLs which use pattern matching
  @param restrictions List of databases on which there exists different
         restrictions for the ACL_USER.
*/

void make_database_privilege_statement(THD *thd, ACL_USER *role,
                                       Protocol *protocol,
                                       const Db_access_map &db_map,
                                       const Db_access_map &db_wild_map,
                                       const DB_restrictions &restrictions) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));

  auto make_grant_stmts = [thd, role, protocol](const Db_access_map &map) {
    for (const Db_access_map::value_type &it : map) {
      ulong want_access = it.second;
      const std::string &db_name = it.first;

      String db;
      db.length(0);
      db.append(STRING_WITH_LEN("GRANT "));

      if (test_all_bits(want_access, (DB_OP_ACLS)))
        db.append(STRING_WITH_LEN("ALL PRIVILEGES"));
      else if (!(want_access & ~GRANT_ACL))
        db.append(STRING_WITH_LEN("USAGE"));
      else {
        int found = 0, cnt;
        ulong j, test_access = want_access & ~GRANT_ACL;
        for (cnt = 0, j = SELECT_ACL; j <= DB_OP_ACLS; cnt++, j <<= 1) {
          if (test_access & j) {
            if (found) db.append(STRING_WITH_LEN(", "));
            found = 1;
            db.append(global_acls_vector[cnt].c_str(),
                      global_acls_vector[cnt].length());
          }
        }
      }
      db.append(STRING_WITH_LEN(" ON "));
      append_identifier(thd, &db, db_name.c_str(), db_name.length());
      db.append(STRING_WITH_LEN(".* TO "));
      append_identifier(thd, &db, role->user,
                        role->user ? strlen(role->user) : 0);
      db.append('@');
      // host and lex_user->host are equal except for case
      append_identifier(thd, &db, role->host.get_host(),
                        role->host.get_host_len());
      if (want_access & GRANT_ACL)
        db.append(STRING_WITH_LEN(" WITH GRANT OPTION"));
      protocol->start_row();
      protocol->store_string(db.ptr(), db.length(), db.charset());
      protocol->end_row();
    }
  };
  auto make_partial_db_revoke_stmts = [thd, protocol,
                                       restrictions](ACL_USER *acl_user) {
    if (mysqld_partial_revokes()) {
      /*
       Copy the unordered restrictions into an array.
       Send the sorted partial revokes to the client.
      */
      Mem_root_array<std::pair<std::string, ulong>> restrictions_array(
          thd->mem_root);
      for (const auto &rl_itr : restrictions.get()) {
        restrictions_array.push_back({rl_itr.first, rl_itr.second});
      }
      std::sort(restrictions_array.begin(), restrictions_array.end(),
                [](const auto &p1, const auto &p2) -> bool {
                  return (p1.first.compare(p2.first) <= 0);
                });
      for (const auto &rl_itr : restrictions_array) {
        String db;
        db.length(0);
        db.append(STRING_WITH_LEN("REVOKE "));
        int found = 0, cnt;
        ulong j, test_access = rl_itr.second & ~GRANT_ACL;
        for (cnt = 0, j = SELECT_ACL; j <= DB_ACLS; cnt++, j <<= 1) {
          if (test_access & j) {
            if (found) db.append(STRING_WITH_LEN(", "));
            found = 1;
            db.append(global_acls_vector[cnt].c_str(),
                      global_acls_vector[cnt].length());
          }
        }

        db.append(STRING_WITH_LEN(" ON "));
        append_identifier(thd, &db, rl_itr.first.c_str(),
                          rl_itr.first.length());
        db.append(STRING_WITH_LEN(".* FROM "));
        append_identifier(thd, &db, acl_user->user,
                          acl_user->user ? strlen(acl_user->user) : 0);
        db.append('@');
        // host and lex_user->host are equal except for case
        append_identifier(thd, &db, acl_user->host.get_host(),
                          acl_user->host.get_host_len());
        protocol->start_row();
        protocol->store_string(db.ptr(), db.length(), db.charset());
        protocol->end_row();
      }
    }
  };

  make_grant_stmts(db_map);
  make_grant_stmts(db_wild_map);
  make_partial_db_revoke_stmts(role);
}

/**
  Maps a set of global level proxy ACLs to string representations and sends them
  through the client protocol.

  @param thd The thread handler
  @param user The authid associated with the proxy ACLs.
  @param protocol The handler used for sending data through the client protocol

*/

void make_proxy_privilege_statement(THD *thd MY_ATTRIBUTE((unused)),
                                    ACL_USER *user, Protocol *protocol) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  for (ACL_PROXY_USER *proxy = acl_proxy_users->begin();
       proxy != acl_proxy_users->end(); ++proxy) {
    if (proxy->granted_on(user->host.get_host(), user->user)) {
      String global;
      proxy->print_grant(&global);
      protocol->start_row();
      protocol->store_string(global.ptr(), global.length(), global.charset());
      protocol->end_row();
    }
  }
}

/**
  Maps a set of database level ACLs for stored programs to string
  representations and sends them through the client protocol.

  @param thd A thread handler
  @param role The authid associated with the ACLs
  @param protocol The handler used for sending data through the client protocol
  @param sp_map The ACLs granted to role
  @param type Either 0 for procedures or 1 for functions

*/

void make_sp_privilege_statement(THD *thd, ACL_USER *role, Protocol *protocol,
                                 SP_access_map &sp_map, int type) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  SP_access_map::iterator it = sp_map.begin();
  for (; it != sp_map.end(); ++it) {
    ulong want_access = it->second;
    std::string sp_name = it->first;

    String db;
    db.length(0);
    db.append(STRING_WITH_LEN("GRANT "));

    if (test_all_bits(want_access, (DB_OP_ACLS)))
      db.append(STRING_WITH_LEN("ALL PRIVILEGES"));
    else if (!(want_access & ~GRANT_ACL))
      db.append(STRING_WITH_LEN("USAGE"));
    else {
      int found = 0, cnt;
      ulong j, test_access = want_access & ~GRANT_ACL;
      for (cnt = 0, j = SELECT_ACL; j <= DB_OP_ACLS; cnt++, j <<= 1) {
        if (test_access & j) {
          if (found) db.append(STRING_WITH_LEN(", "));
          found = 1;
          db.append(global_acls_vector[cnt].c_str(),
                    global_acls_vector[cnt].length());
        }
      }
    }
    db.append(STRING_WITH_LEN(" ON "));
    if (type == 0)
      db.append(STRING_WITH_LEN("PROCEDURE "));
    else
      db.append(STRING_WITH_LEN("FUNCTION "));
    db.append(sp_name.c_str(), sp_name.length());
    db.append(STRING_WITH_LEN(" TO "));
    append_identifier(thd, &db, role->user,
                      role->user ? strlen(role->user) : 0);
    db.append(STRING_WITH_LEN("@"));
    // host and lex_user->host are equal except for case
    append_identifier(thd, &db, role->host.get_host(),
                      role->host.get_host_len());
    if (want_access & GRANT_ACL)
      db.append(STRING_WITH_LEN(" WITH GRANT OPTION"));
    protocol->start_row();
    protocol->store_string(db.ptr(), db.length(), db.charset());
    protocol->end_row();
  }
}

void make_with_admin_privilege_statement(
    THD *thd, ACL_USER *acl_user, Protocol *protocol,
    const Grant_acl_set &with_admin_acl,
    const List_of_granted_roles &granted_roles) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  if (granted_roles.size() == 0) return;
  std::set<std::string> sorted_copy_of_granted_role_str;
  for (auto &rid : granted_roles) {
    if (!rid.second) continue;  // this is not granted WITH ADMIN
    std::string key;
    rid.first.auth_str(&key);
    sorted_copy_of_granted_role_str.insert(key);
  }
  for (auto &s : with_admin_acl) {
    sorted_copy_of_granted_role_str.insert(s);
  }

  if (sorted_copy_of_granted_role_str.size() == 0) return;
  std::set<std::string>::iterator it = sorted_copy_of_granted_role_str.begin();
  String global;
  global.append(STRING_WITH_LEN("GRANT "));
  bool found = false;
  for (; it != sorted_copy_of_granted_role_str.end(); ++it) {
    if (it != sorted_copy_of_granted_role_str.begin()) global.append(',');
    global.append(it->c_str(), it->length());
    found = true;
  }
  if (found) {
    global.append(STRING_WITH_LEN(" TO "));
    append_identifier(thd, &global, acl_user->user, strlen(acl_user->user));
    global.append('@');
    append_identifier(thd, &global, acl_user->host.get_host(),
                      acl_user->host.get_host_len());
    global.append(" WITH ADMIN OPTION");

    protocol->start_row();
    protocol->store_string(global.ptr(), global.length(), global.charset());
    protocol->end_row();
  }
}

void make_dynamic_privilege_statement(THD *thd, ACL_USER *role,
                                      Protocol *protocol,
                                      const Dynamic_privileges &dyn_priv) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  bool found = false;
  /*
    On first iteration create a statement out of all the grants which don't
    have a grant option.
    On second iteration process all privileges with a grant option.
  */
  for (int grant_option = 0; grant_option < 2; ++grant_option) {
    String global;
    global.append(STRING_WITH_LEN("GRANT "));
    for (auto &&priv : dyn_priv) {
      if (grant_option == 0 && priv.second) continue;
      if (grant_option == 1 && !priv.second) continue;
      if (found) global.append(',');
      global.append(priv.first.c_str(), priv.first.length());
      found = true;
    }
    if (found) {
      /* Dynamic privileges are always applied on global level */
      global.append(STRING_WITH_LEN(" ON *.* TO "));
      if (role->user != nullptr)
        append_identifier(thd, &global, role->user, strlen(role->user));
      else
        global.append(STRING_WITH_LEN("''"));
      global.append('@');
      append_identifier(thd, &global, role->host.get_host(),
                        role->host.get_host_len());
      if (grant_option) global.append(" WITH GRANT OPTION");
      protocol->start_row();
      protocol->store_string(global.ptr(), global.length(), global.charset());
      protocol->end_row();
    }
    found = false;
  }  // end for
}

void make_roles_privilege_statement(THD *thd, ACL_USER *role,
                                    Protocol *protocol,
                                    List_of_granted_roles &granted_roles,
                                    bool show_mandatory_roles) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  String global;
  global.append(STRING_WITH_LEN("GRANT "));

  bool found = false;
  std::vector<Role_id> mandatory_roles;
  /*
    Because the output of SHOW GRANTS is used by tools like mysqldump we
    cannot include mandatory roles if the FOR clause is used.
  */
  if (show_mandatory_roles) {
    get_mandatory_roles(&mandatory_roles);
  }
  if (granted_roles.size() == 0 && mandatory_roles.size() == 0) return;
  /* First list granted roles which doesn't have WITH ADMIN */
  std::sort(granted_roles.begin(), granted_roles.end());
  List_of_granted_roles::iterator it = granted_roles.begin();
  std::vector<Role_id>::iterator it2 = mandatory_roles.begin();
  bool got_more_mandatory_roles = (it2 != mandatory_roles.end());
  bool got_more_granted_roles = (it != granted_roles.end());
  while (got_more_mandatory_roles || got_more_granted_roles) {
    if (got_more_granted_roles && !it->second &&
        !(got_more_mandatory_roles && *it2 < it->first)) {
      if (found) global.append(',');
      append_identifier(thd, &global, it->first.user().c_str(),
                        it->first.user().length());
      global.append('@');
      append_identifier(thd, &global, it->first.host().c_str(),
                        it->first.host().length());
      found = true;
      if (got_more_mandatory_roles && it->first == *it2) ++it2;
      ++it;
    } else if (got_more_mandatory_roles) {
      if (found) global.append(',');
      append_identifier(thd, &global, it2->user().c_str(),
                        it2->user().length());
      global.append('@');
      append_identifier(thd, &global, it2->host().c_str(),
                        it2->host().length());
      found = true;
      ++it2;
    } else
      ++it;
    got_more_mandatory_roles = (it2 != mandatory_roles.end());
    got_more_granted_roles = (it != granted_roles.end());
  }  // end while
  if (found) {
    global.append(STRING_WITH_LEN(" TO "));
    append_identifier(thd, &global, role->user,
                      role->user ? strlen(role->user) : 0);
    global.append('@');
    append_identifier(thd, &global, role->host.get_host(),
                      role->host.get_host_len());
    protocol->start_row();
    protocol->store_string(global.ptr(), global.length(), global.charset());
    protocol->end_row();
  }
}

void make_table_privilege_statement(THD *thd, ACL_USER *role,
                                    Protocol *protocol,
                                    Table_access_map &table_map) {
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  Table_access_map::iterator it = table_map.begin();
  for (; it != table_map.end(); ++it) {
    std::string qualified_table_name = it->first;
    Grant_table_aggregate agg = it->second;
    String global;
    ulong test_access = (agg.table_access | agg.cols) & ~GRANT_ACL;

    global.length(0);
    global.append(STRING_WITH_LEN("GRANT "));

    if (test_all_bits(agg.table_access, (TABLE_OP_ACLS)))
      global.append(STRING_WITH_LEN("ALL PRIVILEGES"));
    else if (!test_access)
      global.append(STRING_WITH_LEN("USAGE"));
    else {
      /* Add specific column access */
      int found = 0;
      ulong j;
      ulong counter;
      for (counter = 0, j = SELECT_ACL; j <= TABLE_OP_ACLS;
           counter++, j <<= 1) {
        if (test_access & j) {
          if (found) global.append(STRING_WITH_LEN(", "));
          found = 1;
          global.append(global_acls_vector[counter].c_str(),
                        global_acls_vector[counter].length());

          if (agg.cols) {
            uint found_col = 0;
            Column_map::iterator col_it = agg.columns.begin();
            for (; col_it != agg.columns.end(); ++col_it) {
              if (col_it->second & j) {
                if (!found_col) {
                  found_col = 1;
                  /*
                    If we have a duplicated table level privilege, we
                    must write the access privilege name again.
                  */
                  if (agg.table_access & j) {
                    global.append(STRING_WITH_LEN(", "));
                    global.append(global_acls_vector[counter].c_str(),
                                  global_acls_vector[counter].length());
                  }
                  global.append(STRING_WITH_LEN(" ("));
                } else
                  global.append(STRING_WITH_LEN(", "));
                append_identifier(thd, &global, col_it->first.c_str(),
                                  col_it->first.length());
              }
            }
            if (found_col) global.append(')');
          }
        }
      }
    }
    global.append(STRING_WITH_LEN(" ON "));
    global.append(qualified_table_name.c_str(), qualified_table_name.length());
    global.append(STRING_WITH_LEN(" TO "));
    append_identifier(thd, &global, role->user, strlen(role->user));
    global.append('@');
    // host and lex_user->host are equal except for case
    append_identifier(thd, &global, role->host.get_host(),
                      role->host.get_host_len());
    if (agg.table_access & GRANT_ACL)
      global.append(STRING_WITH_LEN(" WITH GRANT OPTION"));
    protocol->start_row();
    protocol->store_string(global.ptr(), global.length(), global.charset());
    protocol->end_row();
  }
}

void get_sp_access_map(
    ACL_USER *acl_user, SP_access_map *sp_map,
    malloc_unordered_multimap<std::string, unique_ptr_destroy_only<GRANT_NAME>>
        *hash) {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  /* Add routine access */
  for (const auto &key_and_value : *hash) {
    GRANT_NAME *grant_proc = key_and_value.second.get();
    const char *user, *host;
    if (!(user = grant_proc->user)) user = "";
    if (!(host = grant_proc->host.get_host())) host = "";
    const char *acl_user_host, *acl_user_user;
    if (!(acl_user_host = acl_user->host.get_host())) acl_user_host = "";
    if (!(acl_user_user = acl_user->user)) acl_user_user = "";

    /*
      We do not make SHOW GRANTS case-sensitive here (like REVOKE),
      but make it case-insensitive because that's the way they are
      actually applied, and showing fewer privileges than are applied
      would be wrong from a security point of view.
    */

    if (!strcmp(acl_user_user, user) &&
        !my_strcasecmp(system_charset_info, acl_user_host, host)) {
      ulong proc_access = grant_proc->privs;
      if (proc_access != 0) {
        String key;
        append_identifier(&key, grant_proc->db, strlen(grant_proc->db));
        key.append(".");
        append_identifier(&key, grant_proc->tname, strlen(grant_proc->tname));
        (*sp_map)[std::string(key.c_ptr())] |= proc_access;
      }
    }
  }
}

void get_table_access_map(ACL_USER *acl_user, Table_access_map *table_map) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  for (const auto &key_and_value : *column_priv_hash) {
    GRANT_TABLE *grant_table = key_and_value.second.get();
    const char *user, *host;

    if (!(user = grant_table->user)) user = "";
    if (!(host = grant_table->host.get_host())) host = "";
    const char *acl_user_host, *acl_user_user;
    if (!(acl_user_host = acl_user->host.get_host())) acl_user_host = "";
    if (!(acl_user_user = acl_user->user)) acl_user_user = "";

    /*
      We do not make SHOW GRANTS case-sensitive here (like REVOKE),
      but make it case-insensitive because that's the way they are
      actually applied, and showing fewer privileges than are applied
      would be wrong from a security point of view.
    */
    if (!strcmp(acl_user_user, user) &&
        !my_strcasecmp(system_charset_info, acl_user_host, host)) {
      ulong table_access = grant_table->privs;
      if ((table_access | grant_table->cols) != 0) {
        String q_name;
        const THD *thd = table_map->get_thd();
        append_identifier(thd, &q_name, grant_table->db,
                          strlen(grant_table->db));
        q_name.append(".");
        append_identifier(thd, &q_name, grant_table->tname,
                          strlen(grant_table->tname));
        Grant_table_aggregate agg = (*table_map)[std::string(q_name.c_ptr())];
        // cols is an ACL of all privileges found as column privileges in the
        // table given any column in that table. Before a hash look up
        // you can check in this column if the column exist in the first place
        // for the required privilege
        agg.cols |= grant_table->cols;
        agg.table_access |= grant_table->privs;
        if (grant_table->cols) {
          DBUG_PRINT("info", ("Collecting column privileges for %s@%s",
                              acl_user->user, acl_user->host.get_host()));
          // Iterate over all column ACLs for this table.
          for (const auto &key_and_value_acl : grant_table->hash_columns) {
            String q_col_name;
            GRANT_COLUMN *col = key_and_value_acl.second.get();
            // TODO why can this be 0x0 ?!
            if (col) {
              std::string str_column_name(col->column);
              ulong col_access = agg.columns[str_column_name];
              col_access |= col->rights;
              agg.columns[str_column_name] = col_access;
              DBUG_PRINT("info", ("Found privilege %lu on %s.%s", col_access,
                                  q_name.c_ptr(), q_col_name.c_ptr()));
            }
          }
        }
        (*table_map)[std::string(q_name.c_ptr())] = agg;
      }
    }
  }  // end for
}

void get_dynamic_privileges(ACL_USER *acl_user, Dynamic_privileges *acl) {
  Role_id key(create_authid_from(acl_user));
  User_to_dynamic_privileges_map::iterator it, it_end;

  std::tie(it, it_end) = g_dynamic_privileges_map->equal_range(key);
  for (; it != it_end; ++it) {
    auto aggr = acl->find(it->second.first);
    if (aggr != acl->end() && aggr->second != it->second.second) {
      /*
        If this privID was already in the aggregate we make sure that the
        grant option take precedence; any GRANT OPTION will be sticky through
        out role privilege aggregation.
      */
      aggr->second = true;
    } else
      acl->insert(it->second);
  }
}

bool has_wildcard_characters(const LEX_CSTRING &db) {
  return (memchr(db.str, wild_one, db.length) != nullptr ||
          memchr(db.str, wild_many, db.length) != nullptr);
}

void get_database_access_map(ACL_USER *acl_user, Db_access_map *db_map,
                             Db_access_map *db_wild_map) {
  ACL_DB *acl_db;
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  for (acl_db = acl_dbs->begin(); acl_db != acl_dbs->end(); ++acl_db) {
    const char *acl_db_user, *acl_db_host;
    if (!(acl_db_user = acl_db->user)) acl_db_user = "";
    if (!(acl_db_host = acl_db->host.get_host())) acl_db_host = "";
    const char *acl_user_host, *acl_user_user;
    if (!(acl_user_host = acl_user->host.get_host())) acl_user_host = "";
    if (!(acl_user_user = acl_user->user)) acl_user_user = "";

    /*
      We do not make SHOW GRANTS case-sensitive here (like REVOKE),
      but make it case-insensitive because that's the way they are
      actually applied, and showing fewer privileges than are applied
      would be wrong from a security point of view.
    */

    if (!strcmp(acl_user_user, acl_db_user) &&
        !my_strcasecmp(system_charset_info, acl_user_host, acl_db_host)) {
      ulong want_access = acl_db->access;
      if (want_access) {
        if (has_wildcard_characters({acl_db->db, strlen(acl_db->db)})) {
          (*db_wild_map)[std::string(acl_db->db)] |= want_access;
        } else {
          (*db_map)[std::string(acl_db->db)] |= want_access;
        }
        DBUG_PRINT("info", ("Role: %s db: %s acl: %lu", acl_user_user,
                            acl_db->db, want_access));
      }  // end if access
    }
  }  // end for
}

/**
  A graph visitor used for doing breadth-first traversal of the global role
  graph. The visitor takes a set of access maps and aggregate all discovered
  privileges into these maps.
*/
class Get_access_maps : public boost::default_bfs_visitor {
 public:
  Get_access_maps(ACL_USER *acl_user, ulong *access, Db_access_map *db_map,
                  Db_access_map *db_wild_map, Table_access_map *table_map,
                  SP_access_map *sp_map, SP_access_map *func_map,
                  Grant_acl_set *with_admin_acl, Dynamic_privileges *dyn_acl,
                  Restrictions *restrictions)
      : m_access(access),
        m_db_map(db_map),
        m_db_wild_map(db_wild_map),
        m_table_map(table_map),
        m_sp_map(sp_map),
        m_func_map(func_map),
        m_with_admin_acl(with_admin_acl),
        m_dynamic_acl(dyn_acl),
        m_restrictions(restrictions),
        m_grantee{acl_user->user, strlen(acl_user->user),
                  acl_user->host.get_host(), acl_user->host.get_host_len()} {}
  template <typename Vertex, typename Graph>
  void discover_vertex(Vertex u, const Graph &) const {
    ACL_USER acl_user = get(boost::vertex_acl_user_t(), *g_granted_roles)[u];
    if (acl_user.user == g_active_dummy_user) return;  // skip root node
    DBUG_PRINT("info",
               ("Role visitor in %s@%s, adding global access %lu\n",
                acl_user.user, acl_user.host.get_host(), acl_user.access));
    /* Add database access */
    get_database_access_map(&acl_user, m_db_map, m_db_wild_map);

    /* Add restrictions */
    {
      /* DB Restrictions */
      const Auth_id granter(&acl_user);
      Restrictions restrictions =
          acl_restrictions->find_restrictions(&acl_user);
      std::unique_ptr<Restrictions_aggregator> aggregator =
          Restrictions_aggregator_factory::create(
              granter, m_grantee, acl_user.access & DB_ACLS,
              *m_access & DB_ACLS, restrictions.db(), m_restrictions->db(),
              acl_user.access & DB_ACLS, m_db_map);
      if (aggregator) {
        DB_restrictions db_restrictions(nullptr);
        if (aggregator->generate(db_restrictions)) return;
        m_restrictions->set_db(db_restrictions);
      }
    }
    /* Add global access */
    /*
      Up-cast to base class to avoid gcc 7.1.1 warning:
      dereferencing type-punned pointer will break strict-aliasing rules
     */
    *m_access |= implicit_cast<ACL_ACCESS *>(&acl_user)->access;

    /* Add table access */
    get_table_access_map(&acl_user, m_table_map);

    /* Add stored procedure access */
    get_sp_access_map(&acl_user, m_sp_map, proc_priv_hash.get());

    /* Add user function access */
    get_sp_access_map(&acl_user, m_func_map, func_priv_hash.get());

    /* Add dynamic privileges */
    get_dynamic_privileges(&acl_user, m_dynamic_acl);
  }

  template <typename Edge, typename Graph>
  void examine_edge(const Edge &edge, Graph &granted_roles) {
    ACL_USER to_user =
        boost::get(boost::vertex_acl_user_t(),
                   granted_roles)[boost::target(edge, granted_roles)];
    int with_admin_opt =
        boost::get(boost::edge_capacity_t(), granted_roles)[edge];
    if (with_admin_opt) {
      String qname;
      append_identifier(&qname, to_user.user, strlen(to_user.user));
      qname.append('@');
      /* Up-cast to base class, see above. */
      append_identifier(
          &qname, implicit_cast<ACL_ACCESS *>(&to_user)->host.get_host(),
          implicit_cast<ACL_ACCESS *>(&to_user)->host.get_host_len());
      /* We save the granted role in the Acl_map of the granted user */
      m_with_admin_acl->insert(
          std::string(qname.c_ptr_quick(), qname.length()));
    }
  }

 private:
  ulong *m_access;
  Db_access_map *m_db_map;
  Db_access_map *m_db_wild_map;
  Table_access_map *m_table_map;
  SP_access_map *m_sp_map;
  SP_access_map *m_func_map;
  Grant_acl_set *m_with_admin_acl;
  Dynamic_privileges *m_dynamic_acl;
  Restrictions *m_restrictions;
  Auth_id m_grantee;
};

/**
  Get a cached internal schema access.
  @param grant_internal_info the cache
  @param schema_name the name of the internal schema
*/
const ACL_internal_schema_access *get_cached_schema_access(
    GRANT_INTERNAL_INFO *grant_internal_info, const char *schema_name) {
  if (grant_internal_info) {
    if (!grant_internal_info->m_schema_lookup_done) {
      grant_internal_info->m_schema_access =
          ACL_internal_schema_registry::lookup(schema_name);
      grant_internal_info->m_schema_lookup_done = true;
    }
    return grant_internal_info->m_schema_access;
  }
  return ACL_internal_schema_registry::lookup(schema_name);
}

/**
  Get a cached internal table access.
  @param grant_internal_info the cache
  @param schema_name the name of the internal schema
  @param table_name the name of the internal table
*/
const ACL_internal_table_access *get_cached_table_access(
    GRANT_INTERNAL_INFO *grant_internal_info, const char *schema_name,
    const char *table_name) {
  DBUG_ASSERT(grant_internal_info);
  if (!grant_internal_info->m_table_lookup_done) {
    const ACL_internal_schema_access *schema_access;
    schema_access = get_cached_schema_access(grant_internal_info, schema_name);
    if (schema_access)
      grant_internal_info->m_table_access = schema_access->lookup(table_name);
    grant_internal_info->m_table_lookup_done = true;
  }
  return grant_internal_info->m_table_access;
}

ACL_internal_access_result IS_internal_schema_access::check(
    ulong want_access, ulong *save_priv) const {
  want_access &= ~SELECT_ACL;

  /*
    We don't allow any simple privileges but SELECT_ACL on
    the information_schema database.
  */
  if (unlikely(want_access & DB_ACLS)) return ACL_INTERNAL_ACCESS_DENIED;

  /* Always grant SELECT for the information schema. */
  *save_priv |= SELECT_ACL;

  return want_access ? ACL_INTERNAL_ACCESS_CHECK_GRANT
                     : ACL_INTERNAL_ACCESS_GRANTED;
}

const ACL_internal_table_access *IS_internal_schema_access::lookup(
    const char *) const {
  /* There are no per table rules for the information schema. */
  return nullptr;
}

/**
  Check privileges for LOCK TABLES statement.

  @param thd     Thread context.
  @param tables  List of tables to be locked.

  @retval false - Success.
  @retval true  - Failure.
*/

bool lock_tables_precheck(THD *thd, TABLE_LIST *tables) {
  TABLE_LIST *first_not_own_table = thd->lex->first_not_own_table();

  for (TABLE_LIST *table = tables; table != first_not_own_table && table;
       table = table->next_global) {
    if (is_temporary_table(table)) continue;

    if (check_table_access(thd, LOCK_TABLES_ACL | SELECT_ACL, table, false, 1,
                           false))
      return true;
  }

  return false;
}

/**
  CREATE TABLE query pre-check.

  @param thd			Thread handler
  @param tables		Global table list
  @param create_table	        Table which will be created

  @retval
    false   OK
  @retval
    true   Error
*/

bool create_table_precheck(THD *thd, TABLE_LIST *tables,
                           TABLE_LIST *create_table) {
  LEX *lex = thd->lex;
  SELECT_LEX *select_lex = lex->select_lex;
  ulong want_priv;
  bool error = true;  // Error message is given
  DBUG_TRACE;

  /*
    Require CREATE [TEMPORARY] privilege on new table; for
    CREATE TABLE ... SELECT, also require INSERT.
  */

  want_priv =
      (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)
          ? CREATE_TMP_ACL
          : (CREATE_ACL | (select_lex->item_list.elements ? INSERT_ACL : 0));

  if (check_access(thd, want_priv, create_table->db,
                   &create_table->grant.privilege,
                   &create_table->grant.m_internal, false, false))
    goto err;

  /* If it is a merge table, check privileges for merge children. */
  if (lex->create_info->merge_list.first) {
    /*
      The user must have (SELECT_ACL | UPDATE_ACL | DELETE_ACL) on the
      underlying base tables, even if there are temporary tables with the same
      names.

      From user's point of view, it might look as if the user must have these
      privileges on temporary tables to create a merge table over them. This is
      one of two cases when a set of privileges is required for operations on
      temporary tables (see also CREATE TABLE).

      The reason for this behavior stems from the following facts:

        - For merge tables, the underlying table privileges are checked only
          at CREATE TABLE / ALTER TABLE time.

          In other words, once a merge table is created, the privileges of
          the underlying tables can be revoked, but the user will still have
          access to the merge table (provided that the user has privileges on
          the merge table itself).

        - Temporary tables shadow base tables.

          I.e. there might be temporary and base tables with the same name, and
          the temporary table takes the precedence in all operations.

        - For temporary MERGE tables we do not track if their child tables are
          base or temporary. As result we can't guarantee that privilege check
          which was done in presence of temporary child will stay relevant later
          as this temporary table might be removed.

      If SELECT_ACL | UPDATE_ACL | DELETE_ACL privileges were not checked for
      the underlying *base* tables, it would create a security breach as in
      Bug#12771903.
    */

    if (check_table_access(thd, SELECT_ACL | UPDATE_ACL | DELETE_ACL,
                           lex->create_info->merge_list.first, false, UINT_MAX,
                           false))
      goto err;
  }

  if (want_priv != CREATE_TMP_ACL &&
      check_grant(thd, want_priv, create_table, false, 1, false))
    goto err;

  if (select_lex->item_list.elements) {
    /* Check permissions for used tables in CREATE TABLE ... SELECT */
    if (tables &&
        check_table_access(thd, SELECT_ACL, tables, false, UINT_MAX, false))
      goto err;
  } else if (lex->create_info->options & HA_LEX_CREATE_TABLE_LIKE) {
    if (check_table_access(thd, SELECT_ACL, tables, false, UINT_MAX, false))
      goto err;
  }

  if (check_fk_parent_table_access(thd, lex->create_info, lex->alter_info))
    goto err;

  error = false;

err:
  return error;
}

/**
  @brief Performs standardized check whether to prohibit (true)
    or allow (false) operations based on read_only and super_read_only
    state.
  @param thd              Thread handler
  @param err_if_readonly  Boolean indicating whether or not
    to add the error to the thread context if read-only is
    violated.

  @returns Status code
    @retval true The operation should be prohibited.
@   retval false The operation should be allowed.
*/
bool check_readonly(THD *thd, bool err_if_readonly) {
  DBUG_TRACE;

  /* read_only=OFF, do not prohibit operation: */
  if (!opt_readonly) return false;

  /*
    Thread is replication slave or skip_read_only check is enabled for the
    command, do not prohibit operation.
  */
  if (thd->slave_thread || thd->is_cmd_skip_readonly()) return false;

  Security_context *sctx = thd->security_context();
  bool is_super =
      sctx->check_access(SUPER_ACL) ||
      sctx->has_global_grant(STRING_WITH_LEN("CONNECTION_ADMIN")).first;

  /* super_read_only=OFF and user has SUPER privilege,
  do not prohibit operation:
  However, if sql_log_bin is enabled for super, prevent the write to avoid
  local binlog transactions on non-system threads. System threads,
  like InnoDB background threads that truncate the undo log, can commit
  changes to the data dictionary as needed. These changes are not binlogged.

  Checks for analyze/optimize no_write_to_binlog commands in the
  enable_super_log_bin_read_only case are needed here.
  */
  if (is_super && !opt_super_readonly &&
      !(enable_super_log_bin_read_only &&
        (thd->variables.option_bits & OPTION_BIN_LOG) &&
        !thd->is_system_thread() &&
        !(thd->lex &&
          (thd->lex->sql_command == SQLCOM_ANALYZE ||
           thd->lex->sql_command == SQLCOM_OPTIMIZE) &&
          thd->lex->no_write_to_binlog)))
    return false;

  /* throw error in standardized way if requested: */
  if (err_if_readonly) err_readonly(thd);

  /* in all other cases, prohibit operation: */
  return true;
}

/**
  @brief Generates appropriate error messages for read-only state
    depending on whether user has SUPER privilege or not.

  @param thd              Thread handler

*/
void err_readonly(THD *thd) {
  std::string extra_info = get_active_master_info();
  my_error(ER_OPTION_PREVENTS_STATEMENT, MYF(0),
           thd->security_context()->check_access(SUPER_ACL) ||
                   thd->security_context()
                       ->has_global_grant(STRING_WITH_LEN("CONNECTION_ADMIN"))
                       .first
               ? (opt_super_readonly ? "--super-read-only"
                                     : "--sql_log_bin, --read-only and "
                                       "--enable_super_log_bin_read_only")
               : "--read-only",
           extra_info.c_str());
}

/**
  Check grants for commands which work only with one table and all other
  tables belonging to subselects or implicitly opened tables.

  @param thd			Thread handler
  @param privilege		requested privilege
  @param all_tables		global table list of query

  @returns false on success, true on access denied error
*/

bool check_one_table_access(THD *thd, ulong privilege, TABLE_LIST *all_tables) {
  if (check_single_table_access(thd, privilege, all_tables, false)) return true;

  // Check privileges on tables from subqueries and implicitly opened tables
  TABLE_LIST *subquery_table;
  TABLE_LIST *const view = all_tables->is_view() ? all_tables : nullptr;

  if ((subquery_table = all_tables->next_global)) {
    /*
      Access rights asked for the first table of a view should be the same
      as for the view
    */
    if (view && subquery_table->belong_to_view == view) {
      if (check_single_table_access(thd, privilege, subquery_table, false))
        return true; /* purecov: inspected */
      subquery_table = subquery_table->next_global;
    }
    if (subquery_table && check_table_access(thd, SELECT_ACL, subquery_table,
                                             false, UINT_MAX, false))
      return true;
  }
  return false;
}

/**
  Check grants for commands which work only with one table.

  @param thd                    Thread handler
  @param privilege              requested privilege
  @param all_tables             global table list of query
  @param no_errors              false/true - report/don't report error to
                            the client (using my_error() call).

  @retval
    0   OK
  @retval
    1   access denied, error is sent to client
*/

bool check_single_table_access(THD *thd, ulong privilege,
                               TABLE_LIST *all_tables, bool no_errors) {
  if (all_tables->is_internal()) {
    // Optimizer internal tables does not need any privilege checking.
    all_tables->set_privileges(privilege);
    return false;
  }

  Security_context *backup_ctx = thd->security_context();

  /* we need to switch to the saved context (if any) */
  if (all_tables->security_ctx)
    thd->set_security_context(all_tables->security_ctx);

  const char *db_name;
  if ((all_tables->is_view() || all_tables->field_translation) &&
      !all_tables->schema_table)
    db_name = all_tables->view_db.str;
  else
    db_name = all_tables->db;

  if (check_access(thd, privilege, db_name, &all_tables->grant.privilege,
                   &all_tables->grant.m_internal, false, no_errors))
    goto deny;

  /* Show only 1 table for check_grant */
  if (!(all_tables->belong_to_view &&
        (thd->lex->sql_command == SQLCOM_SHOW_FIELDS)) &&
      check_grant(thd, privilege, all_tables, false, 1, no_errors))
    goto deny;

  thd->set_security_context(backup_ctx);
  return false;

deny:
  thd->set_security_context(backup_ctx);
  return true;
}

bool check_routine_access(THD *thd, ulong want_access, const char *db,
                          char *name, bool is_proc, bool no_errors) {
  DBUG_TRACE;
  TABLE_LIST tables[1];

  new (&tables[0]) TABLE_LIST();
  tables->db = db;
  tables->db_length = strlen(db);
  tables->table_name = tables->alias = name;
  tables->table_name_length = strlen(tables->table_name);

  /*
    The following test is just a shortcut for check_access() (to avoid
    calculating db_access) under the assumption that it's common to
    give persons global right to execute all stored SP (but not
    necessary to create them).
    Note that this effectively bypasses the ACL_internal_schema_access checks
    that are implemented for the INFORMATION_SCHEMA and PERFORMANCE_SCHEMA,
    which are located in check_access().
    Since the I_S and P_S do not contain routines, this bypass is ok,
    as long as this code path is not abused to create routines.
    The assert enforce that.
  */
  DBUG_ASSERT((want_access & CREATE_PROC_ACL) == 0);
  if (thd->security_context()->check_access(want_access, db))
    tables->grant.privilege = want_access;
  else {
    DBUG_PRINT("info", ("Checking routine %s.%s for schema level access.", db,
                        tables->table_name));
    if (check_access(thd, want_access, db, &tables->grant.privilege,
                     &tables->grant.m_internal, false, no_errors))

      return true;
  }

  DBUG_PRINT("info", ("Checking routine %s.%s for routine level access.", db,
                      tables->table_name));
  return check_grant_routine(thd, want_access, tables, is_proc, no_errors);
}

/**
  Check if the given table has any of the asked privileges

  @param thd		 Thread handler
  @param want_access	 Bitmap of possible privileges to check for
  @param table The table for which access needs to be validated
  @retval
    0  ok
  @retval
    1  error
*/

bool check_some_access(THD *thd, ulong want_access, TABLE_LIST *table) {
  ulong access;
  DBUG_TRACE;

  /* This loop will work as long as we have less than 32 privileges */
  for (access = 1; access < want_access; access <<= 1) {
    if (access & want_access) {
      if (!check_access(thd, access, table->db, &table->grant.privilege,
                        &table->grant.m_internal, false, true) &&
          !check_grant(thd, access, table, false, 1, true))
        return false;
    }
  }
  DBUG_PRINT("exit", ("no matching access rights"));
  return true;
}

/**
  @brief Check if user has full access to view routine's properties (i.e
  including stored routine code). User must have GLOBAL SELECT or SHOW_ROUTINE
  privilege, or be the definer of this routine.

  @param thd                  Thread handler
  @param db                   Database name
  @param definer_user         Definer username
  @param definer_host         Definer host

  @retval false   no full access.
  @retval true    has full access.
*/
bool has_full_view_routine_access(THD *thd, const char *db,
                                  const char *definer_user,
                                  const char *definer_host) {
  DBUG_TRACE;
  Security_context *sctx = thd->security_context();

  return sctx->check_access(SELECT_ACL, db) ||
         sctx->has_global_grant(STRING_WITH_LEN("SHOW_ROUTINE")).first ||
         (!strcmp(definer_user, sctx->priv_user().str) &&
          !my_strcasecmp(system_charset_info, definer_host,
                         sctx->priv_host().str));
}

/**
  @brief Check if user has partial access to view routine's properties
  (i.e. excluding stored routine code). User must have EXECUTE/CREATE/ALTER
  ROUTINE privileges.

  @param thd                  Thread handler
  @param db                   Database name
  @param routine_name         Routine name
  @param is_proc              True if this routine is a stored procedure, rather
  than a stored function.

  @retval false   no access.
  @retval true    has partial access.
*/

bool has_partial_view_routine_access(THD *thd, const char *db,
                                     const char *routine_name, bool is_proc) {
  DBUG_TRACE;

  /*
    The following test is just a shortcut for check_access() (to avoid
    calculating db_access)
    Note that this effectively bypasses the ACL_internal_schema_access checks
    that are implemented for the INFORMATION_SCHEMA and PERFORMANCE_SCHEMA,
    which are located in check_access().
    Since the I_S and P_S do not contain routines, this bypass is ok,
    as it only opens SHOW_PROC_ACLS.
  */
  if (thd->security_context()->check_access(SHOW_PROC_ACLS, db ? db : "", true))
    return true;

  ulong save_priv;
  if (!check_access(thd, SHOW_PROC_ACLS, db, &save_priv, nullptr, false,
                    true) ||
      (save_priv & SHOW_PROC_ACLS))
    return true;

  return !check_routine_level_acl(thd, db, routine_name, is_proc);
}

/**
  @brief Compare requested privileges with the privileges acquired from the
    User- and Db-tables.
  @param thd          Thread handler
  @param want_access  The requested access privileges.
  @param db           A pointer to the Db name.
  @param[out] save_priv A pointer to the granted privileges will be stored.
  @param grant_internal_info A pointer to the internal grant cache.
  @param dont_check_global_grants True if no global grants are checked.
  @param no_errors    True if no errors should be sent to the client.

  'save_priv' is used to save the User-table (global) and Db-table grants for
  the supplied db name. Note that we don't store db level grants if the global
  grants is enough to satisfy the request AND the global grants contains a
  SELECT grant.

  For internal databases (INFORMATION_SCHEMA, PERFORMANCE_SCHEMA),
  additional rules apply, see ACL_internal_schema_access.

  @see check_grant

  @return Status of denial of access by exclusive ACLs.
    @retval false Access can't exclusively be denied by Db- and User-table
      access unless Column- and Table-grants are checked too.
    @retval true Access denied. The DA is set if no_error = false!
*/

bool check_access(THD *thd, ulong want_access, const char *db, ulong *save_priv,
                  GRANT_INTERNAL_INFO *grant_internal_info,
                  bool dont_check_global_grants, bool no_errors) {
  Security_context *sctx = thd->security_context();
  ulong db_access;
  const std::string &db_name = db ? db : "";

  /*
    GRANT command:
    In case of database level grant the database name may be a pattern,
    in case of table|column level grant the database name can not be a pattern.
    We use 'dont_check_global_grants' as a flag to determine
    if it's database level grant command
    (see SQLCOM_GRANT case, mysql_execute_command() function) and
    set db_is_pattern according to 'dont_check_global_grants' value.
  */
  bool db_is_pattern = ((want_access & GRANT_ACL) && dont_check_global_grants);
  ulong dummy;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("db: %s  want_access: %lu  master_access: %lu", db_name.c_str(),
              want_access, sctx->master_access(db_name)));

  if (save_priv)
    *save_priv = 0;
  else {
    save_priv = &dummy;
    dummy = 0;
  }

  THD_STAGE_INFO(thd, stage_checking_permissions);
  if ((!db || !db[0]) && !thd->db().str && !dont_check_global_grants) {
    DBUG_PRINT("error", ("No database"));
    if (!no_errors) my_error(ER_NO_DB_ERROR, MYF(0)); /* purecov: tested */
    return true;                                      /* purecov: tested */
  }

  if (db != nullptr) {
    if (db != any_db) {
      const ACL_internal_schema_access *access;
      access = get_cached_schema_access(grant_internal_info, db);
      if (access) {
        switch (access->check(want_access, save_priv)) {
          case ACL_INTERNAL_ACCESS_GRANTED:
            /*
              All the privileges requested have been granted internally.
              [out] *save_privileges= Internal privileges.
            */
            return false;
          case ACL_INTERNAL_ACCESS_DENIED:
            if (!no_errors) {
              my_error(ER_DBACCESS_DENIED_ERROR, MYF(0), sctx->priv_user().str,
                       sctx->priv_host().str, db);
            }
            return true;
          case ACL_INTERNAL_ACCESS_CHECK_GRANT:
            /*
              Only some of the privilege requested have been granted internally,
              proceed with the remaining bits of the request (want_access).
            */
            want_access &= ~(*save_priv);
            break;
        }
      }
    }
  }

  if (sctx->check_access(want_access, db_name)) {
    /*
      1. If we don't have a global SELECT privilege, we have to get the
      database specific access rights to be able to handle queries of type
      UPDATE t1 SET a=1 WHERE b > 0
      2. Change db access if it isn't current db which is being addressed
    */
    if (!(sctx->check_access(SELECT_ACL, db_name))) {
      if (db &&
          (!thd->db().str || db_is_pattern || strcmp(db, thd->db().str))) {
        if (sctx->get_active_roles()->size() > 0) {
          bool use_patterns =
              ((want_access & GRANT_ACL) ? true : !dont_check_global_grants);
          db_access = sctx->db_acl({db, strlen(db)}, use_patterns);
        } else {
          db_access = acl_get(thd, sctx->host().str, sctx->ip().str,
                              sctx->priv_user().str, db, db_is_pattern);
        }
      } else {
        /* get access for current db */
        db_access = sctx->current_db_access();
      }
      /*
        The effective privileges are the union of the global privileges
        and the intersection of db- and host-privileges,
        plus the internal privileges.
      */
      *save_priv |= sctx->master_access(db_name) | db_access;
    } else
      *save_priv |= sctx->master_access(db_name);
    return false;
  }

  if (((want_access & ~sctx->master_access(db_name)) & ~DB_ACLS) ||
      (!db && dont_check_global_grants)) {  // We can never grant this
    DBUG_PRINT("error", ("No possible access"));
    if (!no_errors) {
      if (thd->password == 2)
        my_error(ER_ACCESS_DENIED_NO_PASSWORD_ERROR, MYF(0),
                 sctx->priv_user().str, sctx->priv_host().str);
      else
        my_error(ER_ACCESS_DENIED_ERROR, MYF(0), sctx->priv_user().str,
                 sctx->priv_host().str,
                 (thd->password ? ER_THD(thd, ER_YES)
                                : ER_THD(thd, ER_NO))); /* purecov: tested */
    }
    return true; /* purecov: tested */
  }

  if (db == any_db) {
    /*
      Access granted; Allow select on *any* db.
      [out] *save_privileges= 0
    */
    return false;
  }

  if (db && (!thd->db().str || db_is_pattern || strcmp(db, thd->db().str))) {
    if (sctx->get_active_roles()->size() > 0) {
      bool flags =
          ((want_access & GRANT_ACL) ? true : !dont_check_global_grants);
      db_access = sctx->db_acl({db, strlen(db)}, flags);
      DBUG_PRINT("info", ("check_access using db-level privilege for %s. "
                          "ACL: %lu",
                          db, db_access));
    } else {
      db_access = acl_get(thd, sctx->host().str, sctx->ip().str,
                          sctx->priv_user().str, db, db_is_pattern);
    }
  } else
    db_access = sctx->current_db_access();
  DBUG_PRINT("info",
             ("db_access: %lu  want_access: %lu", db_access, want_access));

  /*
    Save the union of User-table and the intersection between Db-table and
    Host-table privileges, with the already saved internal privileges.
  */
  db_access = (db_access | sctx->master_access(db_name));
  *save_priv |= db_access;

  /*
    We need to investigate column- and table access if all requested privileges
    belongs to the bit set of .
  */
  bool need_table_or_column_check =
      (want_access & (TABLE_ACLS | PROC_ACLS | db_access)) == want_access;
  /*
    Grant access if the requested access is in the intersection of
    host- and db-privileges (as retrieved from the acl cache),
    also grant access if all the requested privileges are in the union of
    TABLES_ACLS and PROC_ACLS; see check_grant.
  */
  if (((db_access & want_access) == want_access) ||
      (!dont_check_global_grants && need_table_or_column_check)) {
    /*
       Ok; but need to check table- and column privileges.
       [out] *save_privileges is (User-priv | (Db-priv & Host-priv) |
       Internal-priv)
    */
    return false;
  }

  /*
    Access is denied;
    [out] *save_privileges is (User-priv | (Db-priv & Host-priv) |
    Internal-priv)
  */
  DBUG_PRINT("error", ("Access denied"));
  if (!no_errors)
    my_error(ER_DBACCESS_DENIED_ERROR, MYF(0), sctx->priv_user().str,
             sctx->priv_host().str,
             (db ? db : (thd->db().str ? thd->db().str : "unknown")));
  return true;
}

/**
  @brief Check if the requested privileges exists in either User-, DB- or,
    tables- tables.
  @param thd          Thread context
  @param requirements Privileges requested
  @param tables       List of tables to be compared against
  @param no_errors    Don't report error to the client (using my_error() call).
  @param any_combination_of_privileges_will_do true if any privileges on any
    column combination is enough.
  @param number       Only the first 'number' tables in the linked list are
                      relevant.

  The supplied table list contains cached privileges. This functions calls the
  help functions check_access and check_grant to verify the first three steps
  in the privileges check queue:
  1. Global privileges
  2. OR (db privileges AND host privileges)
  3. OR table privileges
  4. OR column privileges (not checked by this function!)
  5. OR routine privileges (not checked by this function!)

  @see check_access
  @see check_grant

  @note This functions assumes that table list used and
  thd->lex->query_tables_own_last value correspond to each other
  (the latter should be either 0 or point to next_global member
  of one of elements of this table list).

  @retval false OK
  @retval true  Access denied; But column or routine privileges might need to
      be checked also.
*/

bool check_table_access(THD *thd, ulong requirements, TABLE_LIST *tables,
                        bool any_combination_of_privileges_will_do, uint number,
                        bool no_errors) {
  DBUG_TRACE;
  TABLE_LIST *org_tables = tables;
  TABLE_LIST *first_not_own_table = thd->lex->first_not_own_table();
  uint i = 0;
  Security_context *sctx = thd->security_context();
  Security_context *backup_ctx = thd->security_context();

  /*
    The check that first_not_own_table is not reached is for the case when
    the given table list refers to the list for prelocking (contains tables
    of other queries). For simple queries first_not_own_table is 0.
  */
  for (; i < number && tables != first_not_own_table && tables;
       tables = tables->next_global, i++) {
    TABLE_LIST *const table_ref =
        tables->correspondent_table ? tables->correspondent_table : tables;
    ulong want_access = requirements;
    if (table_ref->security_ctx)
      sctx = table_ref->security_ctx;
    else
      sctx = backup_ctx;

    /*
      We should not encounter table list elements for reformed SHOW
      statements unless this is first table list element in the main
      select.
      Such table list elements require additional privilege check
      (see check_show_access()). This check is carried out by caller,
      but only for the first table list element from the main select.
    */
    DBUG_ASSERT(!table_ref->schema_table_reformed ||
                table_ref == thd->lex->select_lex->table_list.first);

    DBUG_PRINT("info",
               ("table: %s derived: %d  view: %d", table_ref->table_name,
                table_ref->is_derived(), table_ref->is_view()));

    if (table_ref->is_internal()) continue;

    thd->set_security_context(sctx);

    if (check_access(thd, want_access, table_ref->get_db_name(),
                     &table_ref->grant.privilege, &table_ref->grant.m_internal,
                     false, no_errors))
      goto deny;
  }
  thd->set_security_context(backup_ctx);

  DBUG_EXECUTE_IF("force_check_table_access_return_ok", return false;);
  return check_grant(thd, requirements, org_tables,
                     any_combination_of_privileges_will_do, number, no_errors);
deny:
  thd->set_security_context(backup_ctx);
  return true;
}

/**
  Check if a current user has the privilege TABLE_ENCRYPTION_ADMIN required
  to create encrypted table. We skip the same for slave threads.

  @param thd    Current thread

  @retval false  A user has the privilege TABLE_ENCRYPTION_ADMIN
  @retval true   A user doesn't have the privilege TABLE_ENCRYPTION_ADMIN
*/

bool check_table_encryption_admin_access(THD *thd) {
  Security_context *sctx = thd->security_context();

  /* replication slave thread can do anything */
  if (thd->slave_thread) {
    return false;
  }

  if (!sctx->has_global_grant(STRING_WITH_LEN("TABLE_ENCRYPTION_ADMIN"))
           .first) {
    return true;
  }

  return false;
}

/**
  Given a TABLE_LIST object this function checks against
   1. global privileges
   2. db privileges
   3. table level privileges

  This function only checks the existence of required ACL on a single table
  object. No special consideration is made for the table type (derived, view,
  temporary etc).

  @param thd Thread handle
  @param required_acl The privileges which are required to continue
  @param table An initialized, single TABLE_LIST object

  @retval true Access is granted
  @retval false Access denied
*/

bool is_granted_table_access(THD *thd, ulong required_acl, TABLE_LIST *table) {
  DBUG_TRACE;
  const char *table_name = table->get_table_name();
  const char *db_name = table->get_db_name();
  if (thd->security_context()->get_active_roles()->size() != 0) {
    /* Check privilege against the role privilege cache */
    ulong global_acl = thd->security_context()->master_access(db_name);
    if ((global_acl & required_acl) == required_acl) {
      DBUG_PRINT("info", ("Access granted for %s.%s by global privileges",
                          db_name, table_name));
      return true;
    }
    ulong db_acl =
        thd->security_context()->db_acl({db_name, strlen(db_name)}, true) |
        global_acl;
    if ((db_acl & required_acl) == required_acl) {
      DBUG_PRINT("info", ("Access granted for %s.%s by schema privileges",
                          db_name, table_name));
      return true;
    }

    Grant_table_aggregate aggr = thd->security_context()->table_and_column_acls(
        {table_name, strlen(table_name)}, {db_name, strlen(db_name)});
    if (((aggr.table_access | db_acl) & required_acl) == required_acl) {
      DBUG_PRINT("info", ("Access granted for %s.%s by table privileges",
                          db_name, table_name));
      return true;
    }
  } else {
    /* No active roles */
    Security_context *sctx = thd->security_context();
    if ((sctx->master_access(db_name) & required_acl) == required_acl) {
      DBUG_PRINT("info",
                 ("(no role) Access granted for %s.%s by global privileges",
                  db_name, table_name));
      return true;
    }
    ulong db_access;
    if ((!thd->db().str || strcmp(db_name, thd->db().str)))
      db_access = acl_get(thd, sctx->host().str, sctx->ip().str,
                          sctx->priv_user().str, db_name, false);
    else
      db_access = sctx->current_db_access();
    db_access = (db_access | sctx->master_access(db_name));
    if ((db_access & required_acl) == required_acl) {
      DBUG_PRINT("info",
                 ("(no role)  Access granted for %s.%s by schema privileges",
                  db_name, table_name));
      return true;
    }
    GRANT_TABLE *grant_table =
        table_hash_search(sctx->host().str, sctx->ip().str, db_name,
                          sctx->priv_user().str, table_name, false);
    if (!grant_table) return false;

    if (((grant_table->privs | db_access) & required_acl) == required_acl) {
      DBUG_PRINT("info",
                 ("(no role) Access granted for %s.%s by table privileges",
                  db_name, table_name));
      return true;
    }
  }
  // permission denied
  return false;
}

/****************************************************************************
  Handle GRANT commands
****************************************************************************/

bool has_grant_role_privilege(THD *thd, const LEX_CSTRING &role_name,
                              const LEX_CSTRING &role_host) {
  DBUG_TRACE;
  Security_context *sctx = thd->security_context();
  if (sctx->check_access(SUPER_ACL) ||
      sctx->has_global_grant(STRING_WITH_LEN("ROLE_ADMIN")).first) {
    DBUG_PRINT("info", ("`%s`@`%s` has with admin privileges for `%s`@`%s` "
                        "through super privileges or ROLE_ADMIN",
                        sctx->priv_user().str, sctx->priv_host().str,
                        role_name.str, role_host.str));
    return true;
  }
  /*
    1. user has global ROLE_ADMIN or SUPER_ACL privileges
    2. user has inherited the GRANT r TO CURRENT_USER WITH ADMIN OPTION
       privileges, where r is a node in some active role graph R granted to
       CURRENT_USER.
  */

  if (sctx->has_with_admin_acl(role_name, role_host)) {
    DBUG_PRINT("info", ("`%s`@`%s` has with admin privileges for `%s`@`%s` by "
                        " WITH ADMIN from granted roles",
                        sctx->priv_user().str, sctx->priv_host().str,
                        role_name.str, role_host.str));
    return true;
  }

  DBUG_PRINT("info", ("`%s`@`%s` doesn't have admin privileges for `%s`@`%s`",
                      sctx->priv_user().str, sctx->priv_host().str,
                      role_name.str, role_host.str));
  return false;
}

/*
  Store table level and column level grants in the privilege tables

  SYNOPSIS
    mysql_table_grant()
    thd                 Thread handle
    table_list          List of tables to give grant
    user_list           List of users to give grant
    columns             List of columns to give grant
    rights              Table level grant
    revoke_grant        Set to true if this is a REVOKE command

  RETURN
    false ok
    true  error
*/

int mysql_table_grant(THD *thd, TABLE_LIST *table_list,
                      List<LEX_USER> &user_list, List<LEX_COLUMN> &columns,
                      ulong rights, bool revoke_grant) {
  ulong column_priv = 0;
  List_iterator<LEX_USER> str_list(user_list);
  LEX_USER *Str, *tmp_Str;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  const char *db_name, *table_name;
  bool transactional_tables;
  acl_table::Pod_user_what_to_update what_to_set;
  bool result = false;
  int ret = 0;
  std::set<LEX_USER *> existing_users;

  DBUG_TRACE;

  DBUG_ASSERT(initialized);

  if (rights & ~TABLE_ACLS) {
    my_error(ER_ILLEGAL_GRANT_FOR_TABLE, MYF(0));
    return true;
  }

  if (!revoke_grant) {
    if (columns.elements) {
      class LEX_COLUMN *column;
      List_iterator<LEX_COLUMN> column_iter(columns);

      if (open_tables_for_query(thd, table_list, 0)) return true;

      if (table_list->is_view()) {
        if (table_list->resolve_derived(thd, false))
          return true; /* purecov: inspected */

        // Prepare a readonly (materialized) view for access to columns
        if (table_list->setup_materialized_derived(thd))
          return true; /* purecov: inspected */
      }
      while ((column = column_iter++)) {
        uint unused_field_idx = NO_CACHED_FIELD_INDEX;
        TABLE_LIST *dummy;
        Field *f = find_field_in_table_ref(
            thd, table_list, column->column.ptr(), column->column.length(),
            column->column.ptr(), nullptr, nullptr, nullptr,
            // check that we have the
            // to-be-granted privilege:
            column->rights, false, &unused_field_idx, false, &dummy);
        if (f == (Field *)nullptr) {
          my_error(ER_BAD_FIELD_ERROR, MYF(0), column->column.c_ptr(),
                   table_list->alias);
          return true;
        }
        if (f == (Field *)-1) return true;
        column_priv |= column->rights;
      }
      close_mysql_tables(thd);
    } else {
      if (!(rights & CREATE_ACL)) {
        // We need at least a shared MDL lock on the table to be allowed
        // to safely check its existence.
        MDL_request mdl_request;
        MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, table_list->db,
                         table_list->table_name, MDL_SHARED, MDL_TRANSACTION);
        if (thd->mdl_context.acquire_lock_nsec(
                &mdl_request, thd->variables.lock_wait_timeout_nsec))
          return true;

        bool exists;
        if (dd::table_exists(thd->dd_client(), table_list->db,
                             table_list->table_name, &exists))
          return true;

        if (!exists) {
          my_error(ER_NO_SUCH_TABLE, MYF(0), table_list->db, table_list->alias);
          return true;
        }
      }
      ulong missing_privilege = rights & ~table_list->grant.privilege;
      if (missing_privilege) {
        char command[128];
        get_privilege_desc(command, sizeof(command), missing_privilege);
        my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), command,
                 thd->security_context()->priv_user().str,
                 thd->security_context()->host_or_ip().str, table_list->alias);
        return true;
      }
    }
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  /*
    The lock api is depending on the thd->lex variable which needs to be
    re-initialized.
  */
  Query_tables_list backup;
  thd->lex->reset_n_backup_query_tables_list(&backup);
  /*
    Restore Query_tables_list::sql_command value, which was reset
    above, as the code writing query to the binary log assumes that
    this value corresponds to the statement being executed.
  */
  thd->lex->sql_command = backup.sql_command;

  { /* Critical Section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if ((ret = open_grant_tables(thd, tables, &transactional_tables))) {
      thd->lex->restore_backup_query_tables_list(&backup);
      return ret != 1; /* purecov: deadcode */
    }

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, user_list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    MEM_ROOT *old_root = thd->mem_root;
    thd->mem_root = &memex;
    grant_version++;

    while ((tmp_Str = str_list++)) {
      int error;
      GRANT_TABLE *grant_table;

      if (!(Str = get_current_user(thd, tmp_Str))) {
        result = true;
        continue;
      }

      Userhostpassword_list password_list;
      if (set_and_validate_user_attributes(
              thd, Str, what_to_set, false, false,
              &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], nullptr,
              revoke_grant ? "REVOKE" : "GRANT", password_list)) {
        result = true;
        continue;
      }

      ACL_USER *this_user = find_acl_user(Str->host.str, Str->user.str, true);
      if (this_user && (what_to_set.m_what & PLUGIN_ATTR))
        existing_users.insert(tmp_Str);

      db_name = table_list->get_db_name();
      thd->add_to_binlog_accessed_dbs(db_name);  // collecting db:s for MTS
      table_name = table_list->get_table_name();

      /* Find/create cached table grant */
      grant_table = table_hash_search(Str->host.str, NullS, db_name,
                                      Str->user.str, table_name, true);
      if (!grant_table) {
        if (revoke_grant) {
          my_error(ER_NONEXISTING_TABLE_GRANT, MYF(0), Str->user.str,
                   Str->host.str, table_list->table_name);
          result = true;
          continue;
        }

        DBUG_EXECUTE_IF("mysql_table_grant_out_of_memory",
                        DBUG_SET("+d,simulate_out_of_memory"););
        grant_table = new (thd->mem_root)
            GRANT_TABLE(Str->host.str, db_name, Str->user.str, table_name,
                        rights, column_priv);
        DBUG_EXECUTE_IF("mysql_table_grant_out_of_memory",
                        DBUG_SET("-d,simulate_out_of_memory"););

        if (!grant_table) {
          result = true; /* purecov: deadcode */
          break;         /* purecov: deadcode */
        }
        column_priv_hash->emplace(
            grant_table->hash_key,
            unique_ptr_destroy_only<GRANT_TABLE>(grant_table));
      }

      /* If revoke_grant, calculate the new column privilege for tables_priv */
      if (revoke_grant) {
        class LEX_COLUMN *column;
        List_iterator<LEX_COLUMN> column_iter(columns);
        GRANT_COLUMN *grant_column;

        /* Fix old grants */
        while ((column = column_iter++)) {
          grant_column = column_hash_search(grant_table, column->column.ptr(),
                                            column->column.length());
          if (grant_column) grant_column->rights &= ~(column->rights | rights);
        }
        /* scan trough all columns to get new column grant */
        column_priv = 0;
        for (const auto &key_and_value : grant_table->hash_columns) {
          grant_column = key_and_value.second.get();
          grant_column->rights &= ~rights;  // Fix other columns
          column_priv |= grant_column->rights;
        }
      } else {
        column_priv |= grant_table->cols;
      }

      /* update table and columns */

      // Hold on to grant_table if it gets deleted, since we use it below.
      std::unique_ptr<GRANT_TABLE, Destroy_only<GRANT_TABLE>>
          deleted_grant_table;

      if ((error = replace_table_table(
               thd, grant_table, &deleted_grant_table,
               tables[ACL_TABLES::TABLE_TABLES_PRIV].table, *Str, db_name,
               table_name, rights, column_priv, revoke_grant))) {
        result = true;
        if (error < 0) break;

        continue;
      }

      if (tables[3].table) {
        if ((error = replace_column_table(
                 thd, grant_table, tables[ACL_TABLES::TABLE_COLUMNS_PRIV].table,
                 *Str, columns, db_name, table_name, rights, revoke_grant))) {
          result = true;
          if (error < 0) break;

          continue;
        }
      }
    }
    thd->mem_root = old_root;

    DBUG_ASSERT(!result || thd->is_error());

    result = log_and_commit_acl_ddl(thd, transactional_tables);

    {
      /* Notify audit plugin. We will ignore the return value. */
      LEX_USER *existing_user;
      for (LEX_USER *one_user : existing_users) {
        if ((existing_user = get_current_user(thd, one_user)))
          mysql_audit_notify(
              thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE),
              thd->is_error(), existing_user->user.str, existing_user->host.str,
              existing_user->plugin.str, is_role_id(existing_user), nullptr,
              nullptr);
      }
    }
    get_global_acl_cache()->increase_version();
  } /* Critical section */

  if (!result) {
    my_ok(thd);
    /* Notify storage engines */
    acl_notify_htons(thd, revoke_grant ? SQLCOM_REVOKE : SQLCOM_GRANT,
                     &user_list);
  }

  thd->lex->restore_backup_query_tables_list(&backup);
  DEBUG_SYNC(thd, "after_table_grant_revoke");
  return result;
}

/**
  Store routine level grants in the privilege tables

  @param thd Thread handle
  @param table_list List of routines to give grant
  @param is_proc Is this a list of procedures?
  @param user_list List of users to give grant
  @param rights Table level grant
  @param revoke_grant Is this is a REVOKE command?
  @param write_to_binlog True if this statement should be written to binlog

  @retval false Success.
  @retval true An error occurred.
*/

bool mysql_routine_grant(THD *thd, TABLE_LIST *table_list, bool is_proc,
                         List<LEX_USER> &user_list, ulong rights,
                         bool revoke_grant, bool write_to_binlog) {
  List_iterator<LEX_USER> str_list(user_list);
  LEX_USER *Str, *tmp_Str;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  const char *db_name, *table_name;
  bool transactional_tables;
  acl_table::Pod_user_what_to_update what_to_set;
  bool result = false;
  int ret;
  std::set<LEX_USER *> existing_users;

  DBUG_TRACE;

  DBUG_ASSERT(initialized);

  if (rights & ~PROC_ACLS) {
    my_error(ER_ILLEGAL_GRANT_FOR_TABLE, MYF(0));
    return true;
  }

  if (!revoke_grant) {
    if (sp_exist_routines(thd, table_list, is_proc)) return true;
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);
  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, user_list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    MEM_ROOT *old_root = thd->mem_root;
    thd->mem_root = &memex;

    DBUG_PRINT("info", ("now time to iterate and add users"));

    while ((tmp_Str = str_list++)) {
      int error;
      GRANT_NAME *grant_name;

      if (!(Str = get_current_user(thd, tmp_Str))) {
        result = true;
        continue;
      }

      Userhostpassword_list password_list;
      if (set_and_validate_user_attributes(
              thd, Str, what_to_set, false, false,
              &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], nullptr,
              revoke_grant ? "REVOKE" : "GRANT", password_list)) {
        result = true;
        continue;
      }

      ACL_USER *this_user = find_acl_user(Str->host.str, Str->user.str, true);
      if (this_user && (what_to_set.m_what & PLUGIN_ATTR))
        existing_users.insert(tmp_Str);

      db_name = table_list->db;
      if (write_to_binlog) thd->add_to_binlog_accessed_dbs(db_name);
      table_name = table_list->table_name;
      grant_name =
          routine_hash_search(Str->host.str, NullS, db_name, Str->user.str,
                              table_name, is_proc, true);
      if (!grant_name) {
        if (revoke_grant) {
          my_error(ER_NONEXISTING_PROC_GRANT, MYF(0), Str->user.str,
                   Str->host.str, table_name);
          result = true;
          continue;
        }
        grant_name = new (thd->mem_root) GRANT_NAME(
            Str->host.str, db_name, Str->user.str, table_name, rights, true);
        if (!grant_name) {
          result = true;
          break;
        }
        if (is_proc)
          proc_priv_hash->emplace(
              grant_name->hash_key,
              unique_ptr_destroy_only<GRANT_NAME>(grant_name));
        else
          func_priv_hash->emplace(
              grant_name->hash_key,
              unique_ptr_destroy_only<GRANT_NAME>(grant_name));
      }

      if ((error = replace_routine_table(thd, grant_name, tables[4].table, *Str,
                                         db_name, table_name, is_proc, rights,
                                         revoke_grant))) {
        result = true;  // Remember error
        if (error < 0) break;

        continue;
      }
    }
    thd->mem_root = old_root;

    /*
      mysql_routine_grant can be called in following scenarios:
      1. As a part of GRANT statement
      2. As a part of CREATE PROCEDURE/ROUTINE statement

      In case of 2, even if we fail to grant permission on
      newly created routine, it is not a critical error and
      is suppressed by caller. Instead, a warning is thrown
      to user.

      So, if we are here and result is set to true, either of the following must
      be true:
      1. An error is set in THD
      2. Current statement is SQLCOM_CREATE_PROCEDURE or
         SQLCOM_CREATE_SPFUNCTION

      So assert for the same.
    */
    DBUG_ASSERT(!result || thd->is_error() ||
                thd->lex->sql_command == SQLCOM_CREATE_PROCEDURE ||
                thd->lex->sql_command == SQLCOM_CREATE_SPFUNCTION);

    result = log_and_commit_acl_ddl(thd, transactional_tables, nullptr, nullptr,
                                    result, write_to_binlog);

    {
      /* Notify audit plugin. We will ignore the return value. */
      for (LEX_USER *one_user : existing_users) {
        LEX_USER *existing_user;
        if ((existing_user = get_current_user(thd, one_user)))
          mysql_audit_notify(
              thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE),
              thd->is_error(), existing_user->user.str, existing_user->host.str,
              existing_user->plugin.str, is_role_id(existing_user), nullptr,
              nullptr);
      }
    }
    get_global_acl_cache()->increase_version();
  } /* Critical section */

  /* Notify storage engines */
  if (write_to_binlog && !result) {
    acl_notify_htons(thd, revoke_grant ? SQLCOM_REVOKE : SQLCOM_GRANT,
                     &user_list);
  }

  return result;
}

bool mysql_revoke_role(THD *thd, const List<LEX_USER> *users,
                       const List<LEX_USER> *roles) {
  DBUG_TRACE;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  List_iterator<LEX_USER> users_it(const_cast<List<LEX_USER> &>(*users));
  List_iterator<LEX_USER> roles_it(const_cast<List<LEX_USER> &>(*roles));
  bool errors = false;
  LEX_USER *lex_user;
  TABLE *table = nullptr;
  int ret;
  bool transactional_tables;
  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1; /* purecov: deadcode */

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, *users)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    table = tables[ACL_TABLES::TABLE_ROLE_EDGES].table;

    std::vector<Role_id> mandatory_roles;
    get_mandatory_roles(&mandatory_roles);
    while (LEX_USER *mand_role = roles_it++) {
      if (std::find_if(mandatory_roles.begin(), mandatory_roles.end(),
                       [&](const Role_id &id) -> bool {
                         Role_id id2(mand_role->user, mand_role->host);
                         return id == id2;
                       }) != mandatory_roles.end()) {
        Role_id authid(mand_role->user, mand_role->host);
        std::string out;
        authid.auth_str(&out);
        my_error(ER_MANDATORY_ROLE, MYF(0), out.c_str());
        return true;
      }
    }
    while ((lex_user = users_it++) && !errors) {
      roles_it.rewind();
      if (lex_user->user.str == nullptr) {
        // HACK: We're using CURRENT_USER()
        lex_user = get_current_user(thd, lex_user);
        DBUG_PRINT("note", ("current user= %s@%s", lex_user->user.str,
                            lex_user->host.str));
      }

      ACL_USER *acl_user;
      if ((acl_user = find_acl_user(lex_user->host.str, lex_user->user.str,
                                    true)) == nullptr) {
        my_error(ER_UNKNOWN_AUTHID, MYF(0), lex_user->user.str,
                 lex_user->host.str);
        errors = true;
        break;
      }
      LEX_USER *role;
      while ((role = roles_it++) && !errors) {
        ACL_USER *acl_role;
        if ((acl_role = find_acl_user(role->host.str, role->user.str, true)) ==
            nullptr) {
          my_error(ER_UNKNOWN_AUTHID, MYF(0), role->user.str, role->host.str);
          errors = true;
          break;
        } else {
          DBUG_PRINT("info", ("User %s@%s will drop parent %s@%s",
                              acl_user->user, acl_user->host.get_host(),
                              role->user.str, role->host.str));
          Security_context *sctx = thd->security_context();
          if (sctx->can_operate_with({role}, consts::system_user)) {
            my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
                     consts::system_user.c_str());
            errors = true;
            break;
          }

          Auth_id_ref from_user = create_authid_from(role);
          Auth_id_ref to_user = create_authid_from(acl_user);
          errors = modify_role_edges_in_table(thd, table, from_user, to_user,
                                              false, true);
          if (errors) {
            my_error(ER_FAILED_REVOKE_ROLE, MYF(0), role->user.str,
                     role->host.str);
            break;
          }
          revoke_role(thd, acl_role, acl_user);
          drop_default_role_policy(
              thd, tables[ACL_TABLES::TABLE_DEFAULT_ROLES].table,
              create_authid_from(acl_role), create_authid_from(acl_user));
        }
      }
    }
    DBUG_ASSERT(!errors || thd->is_error());

    errors = log_and_commit_acl_ddl(thd, transactional_tables);

    get_global_acl_cache()->increase_version();
  } /* Critical section */

  if (!errors) {
    my_ok(thd);
    /* Notify storage engines */
    acl_notify_htons(thd, SQLCOM_REVOKE, users);
  }

  return false;
}

bool has_dynamic_privilege_grant_option(Security_context *sctx,
                                        std::string priv) {
  return sctx->has_global_grant(priv.c_str(), priv.length()).second;
}

/**
  Grants a list of roles to a list of users. Changes are persistent and written
  in the mysql.roles_edges table.

  @param thd Thread handler
  @param users A list of authorization IDs
  @param roles A list of authorization IDs
  @param with_admin_opt True if the granted users should be able to pass on
    the roles to other authorization IDs

  @return Success state
    @retval true An error occurred and the DA is set.
    @retval false The operation was successful and DA is set.
*/

bool mysql_grant_role(THD *thd, const List<LEX_USER> *users,
                      const List<LEX_USER> *roles, bool with_admin_opt) {
  DBUG_TRACE;
  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  List_iterator<LEX_USER> users_it(const_cast<List<LEX_USER> &>(*users));
  bool errors = false;
  LEX_USER *lex_user;
  TABLE *table = nullptr;
  int ret;
  bool transactional_tables;

  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1; /* purecov: deadcode */

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, *users)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    table = tables[6].table;

    while ((lex_user = users_it++) && !errors) {
      List_iterator<LEX_USER> roles_it(const_cast<List<LEX_USER> &>(*roles));
      LEX_USER *role;
      if (lex_user->user.str == nullptr) {
        // HACK: We're using CURRENT_USER()
        lex_user = get_current_user(thd, lex_user);
        DBUG_PRINT("note", ("current user= %s@%s", lex_user->user.str,
                            lex_user->host.str));
      } else if (lex_user->user.length == 0 || *(lex_user->user.str) == '\0') {
        /* Granting roles to an anonymous user isn't allowed */
        my_error(ER_CANNOT_GRANT_ROLES_TO_ANONYMOUS_USER, MYF(0));
        return true;
      }

      ACL_USER *acl_user;
      if ((acl_user = find_acl_user(lex_user->host.str, lex_user->user.str,
                                    true)) == nullptr) {
        my_error(ER_UNKNOWN_AUTHID, MYF(0), lex_user->user.str,
                 lex_user->host.str);
        return true;
      }
      while ((role = roles_it++) && !errors) {
        ACL_USER *acl_role;
        if (role->user.length == 0 || *(role->user.str) == '\0') {
          /* Anonymous roles aren't allowed */
          errors = true;
          std::string user_str = create_authid_str_from(acl_user);
          std::string role_str = create_authid_str_from(role);
          my_error(ER_FAILED_ROLE_GRANT, MYF(0), role_str.c_str(),
                   user_str.c_str());
          break;
        } else if ((acl_role = find_acl_user(role->host.str, role->user.str,
                                             true)) == nullptr) {
          my_error(ER_UNKNOWN_AUTHID, MYF(0), role->user.str, role->host.str);
          errors = true;
          break;
        } else {
          DBUG_PRINT("info", ("User %s@%s will inherit from %s@%s",
                              acl_user->user, acl_user->host.get_host(),
                              role->user.str, role->host.str));
          Security_context *sctx = thd->security_context();
          if (sctx->can_operate_with({role}, consts::system_user)) {
            my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
                     consts::system_user.c_str());
            errors = true;
            break;
          }
          grant_role(acl_role, acl_user, with_admin_opt);
          Auth_id_ref from_user = create_authid_from(role);
          Auth_id_ref to_user = create_authid_from(acl_user);
          errors = modify_role_edges_in_table(thd, table, from_user, to_user,
                                              with_admin_opt, false);
          if (errors) {
            std::string user_str = create_authid_str_from(acl_user);
            std::string role_str = create_authid_str_from(role);
            my_error(ER_FAILED_ROLE_GRANT, MYF(0), user_str.c_str(),
                     role_str.c_str());
            break;
          }
        }
      }
    }

    DBUG_ASSERT(!errors || thd->is_error());

    errors = log_and_commit_acl_ddl(thd, transactional_tables);
    get_global_acl_cache()->increase_version();
  } /* Critical section */

  if (!errors) {
    my_ok(thd);
    /* Notify storage engines */
    acl_notify_htons(thd, SQLCOM_GRANT, users);
  }

  return errors;
}

bool mysql_grant(THD *thd, const char *db, List<LEX_USER> &list, ulong rights,
                 bool revoke_grant, bool is_proxy,
                 const List<LEX_CSTRING> &dynamic_privilege,
                 bool grant_all_current_privileges, LEX_GRANT_AS *grant_as) {
  List_iterator<LEX_USER> str_list(list);
  LEX_USER *user, *target_user, *proxied_user = nullptr;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  bool transactional_tables;
  acl_table::Pod_user_what_to_update what_to_set;
  bool error = false;
  int ret;
  TABLE *dynpriv_table;
  std::set<LEX_USER *> existing_users;
  bool partial_revokes = false;
  const List<LEX_CSTRING> *granted_dynamic_privs = &dynamic_privilege;
  DBUG_TRACE;
  DBUG_ASSERT(initialized);

  if (is_proxy) {
    DBUG_ASSERT(!db);
    proxied_user = str_list++;
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);
  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);
    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    bool with_grant_option = ((rights & GRANT_ACL) != 0);
    bool grant_option = thd->lex->grant_privilege;
    if (db == nullptr && with_grant_option && (rights & ~GRANT_ACL) == 0 &&
        dynamic_privilege.elements > 0) {
      /*
        If this is a grant on global privilege level and there only dynamic
        privileges specified; don't apply the GRANT OPTION on a global privilege
        level.
      */
      rights = 0;
    }

    dynpriv_table = tables[ACL_TABLES::TABLE_DYNAMIC_PRIV].table;
    Grant_validator grant_validator(
        thd, db, list, rights, revoke_grant, dynamic_privilege,
        grant_all_current_privileges, grant_as, dynpriv_table);
    if (grant_validator.validate()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    /* go through users in user_list */
    grant_version++;
    while ((target_user = str_list++)) {
      if (!(user = get_current_user(thd, target_user))) {
        error = true;
        continue;
      }

      Userhostpassword_list password_list;
      if (set_and_validate_user_attributes(
              thd, user, what_to_set, false, false,
              &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], nullptr,
              revoke_grant ? "REVOKE" : "GRANT", password_list)) {
        error = true;
        continue;
      }

      ACL_USER *this_user = find_acl_user(user->host.str, user->user.str, true);
      Restrictions restrictions(nullptr);
      DB_restrictions db_restrictions(nullptr);
      ulong filtered_rights = rights;
      std::unique_ptr<Restrictions_aggregator> aggregator =
          Restrictions_aggregator_factory::create(thd, this_user, db, rights,
                                                  grant_all_current_privileges);
      if (aggregator) {
        partial_revokes = true;
        if (aggregator->generate(db_restrictions)) {
          error = true;
          break;
        }
        restrictions.set_db(db_restrictions);
        what_to_set.m_what |= USER_ATTRIBUTES;
        what_to_set.m_user_attributes |= acl_table::USER_ATTRIBUTE_RESTRICTIONS;
      }
      if (this_user && (what_to_set.m_what & PLUGIN_ATTR))
        existing_users.insert(target_user);
      what_to_set.m_what |= ACCESS_RIGHTS_ATTR;
      if ((ret = replace_user_table(thd, tables[ACL_TABLES::TABLE_USER].table,
                                    user, (!db ? rights : 0), revoke_grant,
                                    false, what_to_set, &restrictions))) {
        error = true;
        if (ret < 0) break;

        continue;
      }
      /*
         DB table operation is needed in either of the following cases :
          - There is no partial revoke(s)
          - Hybrid operation; Partial revokes filtered some of the access
            for DB table
      */
      else if (db && (aggregator == nullptr ||
                      aggregator->find_if_require_next_level_operation(
                          filtered_rights))) {
        ulong db_rights = filtered_rights & DB_ACLS;
        if (db_rights == filtered_rights) {
          if ((ret = replace_db_table(thd, tables[ACL_TABLES::TABLE_DB].table,
                                      db, *user, db_rights, revoke_grant))) {
            error = true;
            if (ret < 0) break;

            continue;
          }
          thd->add_to_binlog_accessed_dbs(db);
        } else {
          my_error(ER_WRONG_USAGE, MYF(0), "DB GRANT", "GLOBAL PRIVILEGES");
          error = true;
          continue;
        }
      } else if (is_proxy) {
        if ((ret = replace_proxies_priv_table(
                 thd, tables[5].table, user, proxied_user,
                 rights & GRANT_ACL ? true : false, revoke_grant))) {
          error = true;
          if (ret < 0) break;

          continue;
        }
      }

      if (!db &&
          (dynamic_privilege.elements > 0 || grant_all_current_privileges)) {
        LEX_CSTRING *priv;
        Update_dynamic_privilege_table update_table(thd, dynpriv_table);
        List<LEX_CSTRING> *privileges_to_check;
        if (grant_all_current_privileges) {
          /*
            Copy all currently available dynamic privileges to the list of
            dynamic privileges to grant.
          */
          privileges_to_check = new (thd->mem_root) List<LEX_CSTRING>;
          iterate_all_dynamic_privileges(thd, [&](const char *str) {
            LEX_CSTRING *new_str =
                (LEX_CSTRING *)thd->alloc(sizeof(LEX_CSTRING));
            new_str->str = str;
            new_str->length = strlen(str);
            privileges_to_check->push_back(new_str);
            return false;
          });
          granted_dynamic_privs = privileges_to_check;
        } else
          privileges_to_check =
              &const_cast<List<LEX_CSTRING> &>(dynamic_privilege);
        List_iterator<LEX_CSTRING> priv_it(*privileges_to_check);
        while ((priv = priv_it++) && !error) {
          /* We already checked privileges required to perform GRANT/REVOKE */
          if (revoke_grant) {
            error = revoke_dynamic_privilege(*priv, user->user, user->host,
                                             update_table);
          } else {
            /* We performed SYSTEM_USER check before */
            error = grant_dynamic_privilege(*priv, user->user, user->host,
                                            with_grant_option, update_table);
          }
          if (error) {
            /*
              If the operation fails the DA might have been set already, but
              if wasn't we can assume the dynamic privilege wasn't
              registered in which case a syntax error is a reasonable response.
            */
            if (!thd->get_stmt_da()->is_error())
              my_error(ER_SYNTAX_ERROR, MYF(0));
            break;
          }
        }
      }
      if (!db && grant_option) {
        bool dynamic_privileges_error = false;
        Update_dynamic_privilege_table update_table(thd, dynpriv_table);
        if (!revoke_grant)
          dynamic_privileges_error =
              grant_grant_option_for_all_dynamic_privileges(
                  user->user, user->host, update_table);
        else
          dynamic_privileges_error =
              revoke_grant_option_for_all_dynamic_privileges(
                  user->user, user->host, update_table);
        if (dynamic_privileges_error) {
          if (!thd->get_stmt_da()->is_error())
            my_error(ER_SYNTAX_ERROR, MYF(0));
        }
      }
    }  // for each user

    DBUG_ASSERT(!error || thd->is_error());

    {
      /*
        We want to add AS ... clause while rewritting GRANT statement in
        following cases:

        1. The GRANT statement being executed contains AS clause.
           In this case we retain it as it is except rewriting
           CURRENT_USER() and use current session's user/host part.
        2. If all of the following condtions are met:
           - --partial_revokes is ON
           - Statement is a GRANT at global level (*.*)
           This is required because, current user may have restriction
           list and grant may propagate the same to grantee. In order
           to repliably replay this on other nodes in an HA setup, details
           of current user has to be captured.
      */
      LEX_GRANT_AS *grant_as_ptr = nullptr;
      bool grant_as_specified = false;
      LEX_GRANT_AS grant_as_for_rewrite;
      if (grant_as->grant_as_used) {
        grant_as_specified = grant_as->grant_as_used;
        grant_as_ptr = grant_as;
      } else if (partial_revokes && !revoke_grant && !is_proxy && !db) {
        /* Set LEX_GRANT_AS for given GRANT */
        grant_as_for_rewrite.grant_as_used = true;

        grant_as_for_rewrite.role_type =
            thd->security_context()->get_active_roles()->size()
                ? role_enum::ROLE_NAME
                : role_enum::ROLE_NONE;

        LEX_CSTRING priv_user = thd->security_context()->priv_user();
        LEX_CSTRING priv_host = thd->security_context()->priv_host();
        grant_as_for_rewrite.user = LEX_USER::alloc(
            thd, (LEX_STRING *)&priv_user, (LEX_STRING *)&priv_host);

        if (grant_as_for_rewrite.role_type == role_enum::ROLE_NAME) {
          grant_as_for_rewrite.role_list = new (thd->mem_root) List<LEX_USER>;
          thd->security_context()->get_active_roles(
              thd, *grant_as_for_rewrite.role_list);
        }
        grant_as_specified = false;
        grant_as_ptr = &grant_as_for_rewrite;
      }

      Grant_params grant_rewrite_params(grant_as_specified, grant_as_ptr);

      error = log_and_commit_acl_ddl(thd, transactional_tables, nullptr,
                                     &grant_rewrite_params);
    }

    {
      /* Notify audit plugin. We will ignore the return value. */
      LEX_USER *existing_user;
      for (LEX_USER *one_user : existing_users) {
        if ((existing_user = get_current_user(thd, one_user)))
          mysql_audit_notify(
              thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE),
              thd->is_error(), existing_user->user.str, existing_user->host.str,
              existing_user->plugin.str, is_role_id(existing_user), nullptr,
              nullptr);
      }
    }

    get_global_acl_cache()->increase_version();
  } /* Critical section */

  if (!error) {
    my_ok(thd);
    /* Notify storage engines */
    acl_notify_htons(thd, revoke_grant ? SQLCOM_REVOKE : SQLCOM_GRANT, &list,
                     nullptr, granted_dynamic_privs);
  }

  return error;
}

/**
  @brief Check table level grants

  @param thd          Thread handler
  @param want_access  Bits of privileges user needs to have.
  @param tables       List of tables to check. The user should have
                      'want_access' to all tables in list.
  @param any_combination_will_do true if it's enough to have any privilege for
    any combination of the table columns.
  @param number       Check at most this number of tables.
  @param no_errors    true if no error should be sent directly to the client.

  If table->grant.want_privilege != 0 then the requested privileges where
  in the set of COL_ACLS but access was not granted on the table level. As
  a consequence an extra check of column privileges is required.

  Specifically if this function returns false the user has some kind of
  privilege on a combination of columns in each table.

  This function is usually preceeded by check_access which establish the
  User-, Db- and Host access rights.

  @see check_access
  @see check_table_access

  @note This functions assumes that either number of tables to be inspected
     by it is limited explicitly (i.e. is is not UINT_MAX) or table list
     used and thd->lex->query_tables_own_last value correspond to each
     other (the latter should be either 0 or point to next_global member
     of one of elements of this table list).

   @return Access status
     @retval false Access granted; But column privileges need to be checked.
     @retval true The user did not have the requested privileges on any of the
      tables.

*/

bool check_grant(THD *thd, ulong want_access, TABLE_LIST *tables,
                 bool any_combination_will_do, uint number, bool no_errors) {
  TABLE_LIST *tl;
  TABLE_LIST *const first_not_own_table = thd->lex->first_not_own_table();
  Security_context *sctx = thd->security_context();
  ulong orig_want_access = want_access;
  std::vector<TABLE_LIST *> tables_to_be_processed_further;
  DBUG_TRACE;
  DBUG_ASSERT(number > 0);

  for (tl = tables; tl && number-- && tl != first_not_own_table;
       tl = tl->next_global) {
    TABLE_LIST *const t_ref =
        tl->correspondent_table ? tl->correspondent_table : tl;
    sctx = (t_ref->security_ctx != nullptr) ? t_ref->security_ctx
                                            : thd->security_context();
    const char *db_name = t_ref->get_db_name();
    const ACL_internal_table_access *access = get_cached_table_access(
        &t_ref->grant.m_internal, db_name, t_ref->get_table_name());

    if (access) {
      switch (access->check(orig_want_access, &t_ref->grant.privilege)) {
        case ACL_INTERNAL_ACCESS_GRANTED:
          /*
             Grant all access to the table to skip column checks.
             Depend on the controls in the P_S table itself.
          */
          t_ref->grant.privilege |= TMP_TABLE_ACLS;
          continue;
        case ACL_INTERNAL_ACCESS_DENIED:
          goto err;
        case ACL_INTERNAL_ACCESS_CHECK_GRANT:
          break;
      }
    }

    want_access = orig_want_access;
    want_access &= ~sctx->master_access(db_name);
    if (!want_access) continue;  // ok

    if (!(~t_ref->grant.privilege & want_access) || t_ref->is_internal() ||
        t_ref->schema_table)
      continue;

    if (is_temporary_table(t_ref)) {
      /*
        If this table list element corresponds to a pre-opened temporary
        table skip checking of all relevant table-level privileges for it.
        Note that during creation of temporary table we still need to check
        if user has CREATE_TMP_ACL.
      */
      t_ref->grant.privilege |= TMP_TABLE_ACLS;
      continue;
    }

    if (sctx->get_active_roles()->size() != 0) {
      t_ref->grant.grant_table = nullptr;
      t_ref->grant.version = grant_version;

      Grant_table_aggregate aggr = sctx->table_and_column_acls(
          {t_ref->get_db_name(), strlen(t_ref->get_db_name())},
          {t_ref->get_table_name(), strlen(t_ref->get_table_name())});

      DBUG_PRINT("info",
                 ("Acl_map table %s.%s has access %lu", t_ref->get_db_name(),
                  t_ref->get_table_name(), aggr.table_access));
      /*
        For SHOW COLUMNS, SHOW INDEX it is enough to have some
        privileges on any column combination on the table.
      */
      if (any_combination_will_do && (aggr.cols != 0 || aggr.table_access != 0))
        continue;

      /*
        For SHOW COLUMNS, SHOW INDEX it is enough to have some
        privileges on any column combination on the table.
      */
      if (any_combination_will_do) continue;
      t_ref->grant.privilege = aggr.table_access;
      if (!(~t_ref->grant.privilege & want_access)) {
        DBUG_PRINT("info",
                   ("Access not denied because of column acls for %s.%s."
                    "want_access= %lu, grant.privilege= %lu",
                    t_ref->get_db_name(), t_ref->get_table_name(), want_access,
                    t_ref->grant.privilege));
        continue;
      }
      if (want_access & ~(aggr.cols | t_ref->grant.privilege)) {
        want_access &= ~(aggr.cols | t_ref->grant.privilege);
        DBUG_PRINT("info", ("Access denied for %s.%s. Unfulfilled access: %lu",
                            t_ref->get_db_name(), t_ref->get_table_name(),
                            want_access));
        goto err;
      }
    } else {
      /*
        For these tables we have to access ACL caches.
        We will first go over all TABLE_LIST objects and
        create a list of objects to be processed further.
        Later, we will access ACL caches for these tables.
        It allows us to delay locking of ACL caches.
      */
      tables_to_be_processed_further.push_back(tl);
    }  // end else
  }    // end for

  if (!tables_to_be_processed_further.empty()) {
    tl = nullptr;
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
    if (!acl_cache_lock.lock(!no_errors)) return true;

    for (TABLE_LIST *tl_tmp : tables_to_be_processed_further) {
      tl = tl_tmp;
      TABLE_LIST *const t_ref =
          tl->correspondent_table ? tl->correspondent_table : tl;
      sctx = (t_ref->security_ctx != nullptr) ? t_ref->security_ctx
                                              : thd->security_context();
      const char *db_name = t_ref->get_db_name();
      want_access = orig_want_access;
      want_access &= ~sctx->master_access(db_name);
      DBUG_ASSERT(want_access != 0);

      GRANT_TABLE *grant_table = table_hash_search(
          sctx->host().str, sctx->ip().str, db_name, sctx->priv_user().str,
          t_ref->get_table_name(), false);

      if (!grant_table) {
        DBUG_PRINT("info",
                   ("Table %s didn't exist in the legacy table acl cache",
                    t_ref->get_table_name()));
        want_access &= ~t_ref->grant.privilege;
        goto err;  // No grants
      }

      /*
        For SHOW COLUMNS, SHOW INDEX it is enough to have some
        privileges on any column combination on the table.
      */
      if (any_combination_will_do) continue;

      t_ref->grant.grant_table = grant_table;  // Remember for column test
      t_ref->grant.version = grant_version;
      t_ref->grant.privilege |= grant_table->privs;

      DBUG_PRINT("info",
                 ("t_ref->grant.privilege = %lu", t_ref->grant.privilege));
      if (!(~t_ref->grant.privilege & want_access)) continue;

      if (want_access & ~(grant_table->cols | t_ref->grant.privilege)) {
        want_access &= ~(grant_table->cols | t_ref->grant.privilege);
        goto err;  // impossible
      }
    }
  }
  return false;

err:

  if (!no_errors)  // Not a silent skip of table
  {
    char command[128];
    get_privilege_desc(command, sizeof(command), want_access);
    my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), command,
             sctx->priv_user().str, sctx->host_or_ip().str,
             tl ? tl->get_table_name() : "unknown");
  }
  return true;
}

/*
  Check column rights in given security context

  SYNOPSIS
    check_grant_column()
    thd                  thread handler
    grant                grant information structure
    db_name              db name
    table_name           table  name
    name                 column name
    length               column name length
    sctx                 security context
    want_privilege       wanted privileges

  RETURN
    false OK
    true  access denied
*/

bool check_grant_column(THD *thd, GRANT_INFO *grant, const char *db_name,
                        const char *table_name, const char *name, size_t length,
                        Security_context *sctx, ulong want_privilege) {
  GRANT_TABLE *grant_table;
  GRANT_COLUMN *grant_column;
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("table: %s  want_privilege: %lu", table_name, want_privilege));

  // Adjust wanted privileges based on privileges granted to table:
  want_privilege &= ~grant->privilege;
  if (!want_privilege) return false;  // Already checked
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  /* reload table if someone has modified any grants */
  if (sctx->get_active_roles()->size() != 0) {
    DBUG_ASSERT(grant->grant_table == nullptr);
    Grant_table_aggregate agg = sctx->table_and_column_acls(
        {(const char *)db_name, strlen(db_name)},
        {(const char *)table_name, strlen(table_name)});
    std::string q_name(name);
    Column_map::iterator it = agg.columns.find(q_name);
    if (it != agg.columns.end()) {
      if (!(~(it->second) & want_privilege)) {
        DBUG_PRINT("info", ("Sufficient column privileges found for %s.%s.%s",
                            db_name, table_name, name));
        return false;
      }
    } else {
      DBUG_PRINT("info", ("No column privileges found for %s.%s.%s", db_name,
                          table_name, name));
    }
  } else {
    if (grant->version != grant_version) {
      grant->grant_table = table_hash_search(
          sctx->host().str, sctx->ip().str, db_name, sctx->priv_user().str,
          table_name, false);         /* purecov: inspected */
      grant->version = grant_version; /* purecov: inspected */
    }
    if (!(grant_table = grant->grant_table)) goto err; /* purecov: deadcode */

    grant_column = column_hash_search(grant_table, name, length);
    if (grant_column && !(~grant_column->rights & want_privilege)) {
      return false;
    }
  }

err:
  char command[128];
  get_privilege_desc(command, sizeof(command), want_privilege);
  my_error(ER_COLUMNACCESS_DENIED_ERROR, MYF(0), command, sctx->priv_user().str,
           sctx->host_or_ip().str, name, table_name);
  return true;
}

/**
  Check the privileges for a column depending on the type of table.

  @param thd              thread handler
  @param table_ref        table reference where to check the field
  @param name             name of field to check
  @param length           length of name
  @param want_privilege   wanted privileges

  Check the privileges for a column depending on the type of table the column
  belongs to. The function provides a generic interface to check column
  privileges that hides the heterogeneity of the column representation -
  whether it belongs to a view or a base table.

  Notice that this function does not understand that a column from a view
  reference must be checked for privileges both in the view and in the
  underlying base table (or view) reference. This is the responsibility of
  the caller.

  Columns from temporary tables and derived tables are ignored by this function.

  @returns false if success, true if error (access denied)
*/

bool check_column_grant_in_table_ref(THD *thd, TABLE_LIST *table_ref,
                                     const char *name, size_t length,
                                     ulong want_privilege) {
  DBUG_TRACE;
  GRANT_INFO *grant;
  const char *db_name;
  const char *table_name;
  Security_context *sctx = (table_ref->security_ctx != nullptr)
                               ? table_ref->security_ctx
                               : thd->security_context();

  DBUG_ASSERT(want_privilege);

  if (is_temporary_table(table_ref) || table_ref->is_internal()) {
    // Temporary table or optimizer internal table: no need to evaluate
    // privileges
    return false;
  } else if (table_ref->is_view() || table_ref->field_translation) {
    /* View or derived information schema table. */
    ulong view_privs;
    grant = &(table_ref->grant);
    db_name = table_ref->view_db.str;
    table_name = table_ref->view_name.str;
    if (table_ref->belong_to_view &&
        thd->lex->sql_command == SQLCOM_SHOW_FIELDS) {
      if (sctx->get_active_roles()->size() > 0) {
        view_privs = sctx->table_acl({db_name, strlen(db_name)},
                                     {table_name, strlen(table_name)});
        DBUG_PRINT("info", ("Found role privileges for %s.%s : %lu", db_name,
                            table_name, view_privs));
      } else
        view_privs = get_column_grant(thd, grant, db_name, table_name, name);
      if (view_privs & VIEW_ANY_ACL) {
        return false;
      }
      my_error(ER_VIEW_NO_EXPLAIN, MYF(0));
      return true;
    }
  } else if (table_ref->nested_join) {
    for (TABLE_LIST *table : table_ref->nested_join->join_list) {
      if (check_column_grant_in_table_ref(thd, table, name, length,
                                          want_privilege)) {
        return true;
      }
    }
    return false;
  } else {
    // Regular, persistent base table
    grant = &table_ref->grant;
    db_name = table_ref->db;
    table_name = table_ref->table_name;
    DBUG_ASSERT(strcmp(db_name, table_ref->table->s->db.str) == 0 &&
                strcmp(table_name, table_ref->table->s->table_name.str) == 0);
  }

  if (check_grant_column(thd, grant, db_name, table_name, name, length, sctx,
                         want_privilege))
    return true;
  return false;
}

/**
  @brief check if a query can access a set of columns

  @param  thd  the current thread
  @param  want_access_arg  the privileges requested
  @param  fields an iterator over the fields of a table reference.
  @return Operation status
    @retval 0 Success
    @retval 1 Falure
  @details This function walks over the columns of a table reference
   The columns may originate from different tables, depending on the kind of
   table reference, e.g. join, view.
   For each table it will retrieve the grant information and will use it
   to check the required access privileges for the fields requested from it.
*/
bool check_grant_all_columns(THD *thd, ulong want_access_arg,
                             Field_iterator_table_ref *fields) {
  Security_context *sctx = thd->security_context();
  ulong want_access = want_access_arg;
  const char *table_name = nullptr;
  const char *field_name = nullptr;
  const char *db_name = nullptr;
  GRANT_INFO *grant = nullptr;
  GRANT_TABLE *grant_table = nullptr;
  /*
     Flag that gets set if privilege checking has to be performed on column
     level.
  */
  bool using_column_privileges = false;
  bool has_roles = thd->security_context()->get_active_roles()->size() > 0;
  Grant_table_aggregate aggr;
  DEBUG_SYNC(thd, "in_check_grant_all_columns");
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  for (; !fields->end_of_fields(); fields->next()) {
    grant = fields->grant(); /* Get cached GRANT_INFO on field */
    // Check the privileges at column level if table does not have wanted access
    want_access = want_access_arg & ~grant->privilege;
    if (want_access) {
      field_name = fields->name();
      table_name = fields->get_table_name();
      db_name = fields->get_db_name();
      if (has_roles) {
        LEX_CSTRING str_db_name = {db_name, strlen(db_name)};
        LEX_CSTRING str_table_name = {table_name, strlen(table_name)};
        aggr = thd->security_context()->table_and_column_acls(str_db_name,
                                                              str_table_name);
        /* Update it to reflect current role privileges */
        grant->privilege = aggr.table_access;
        /* Reduce remaining privilege requirements */
        want_access = want_access_arg & ~grant->privilege;
        /* Does any of the columns have the access we need? */
        if (aggr.cols & want_access) {
          /* Find our column */
          std::string q_name(field_name);
          Column_map::iterator it = aggr.columns.find(q_name);
          if (it != aggr.columns.end()) using_column_privileges = true;
          if (it == aggr.columns.end() || (~(it->second) & want_access)) {
            DBUG_PRINT("info", ("No column privileges found for %s.%s.%s",
                                db_name, table_name, field_name));
            goto err;
          }
          DBUG_PRINT("info", ("Sufficient column privileges found for %s.%s.%s",
                              db_name, table_name, field_name));
        }
      } else {
        /* reload table if someone has modified any grants */
        if (grant->version != grant_version) {
          grant->grant_table = table_hash_search(
              sctx->host().str, sctx->ip().str, db_name, sctx->priv_user().str,
              table_name, false);         /* purecov: inspected */
          grant->version = grant_version; /* purecov: inspected */
        }
        grant_table = grant->grant_table;
        if (grant_table == nullptr) goto err;
        GRANT_COLUMN *grant_column =
            column_hash_search(grant_table, field_name, strlen(field_name));
        if (grant_column) using_column_privileges = true;
        if (grant_column == nullptr || (~grant_column->rights & want_access))
          goto err;
      }
    }
  }  // next field
  return false;

err:

  char command[128];
  get_privilege_desc(command, sizeof(command), want_access);
  /*
    Do not give an error message listing a column name unless the user has
    privilege to see all columns.
  */
  if (using_column_privileges)
    my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), command,
             sctx->priv_user().str, sctx->host_or_ip().str, table_name);
  else
    my_error(ER_COLUMNACCESS_DENIED_ERROR, MYF(0), command,
             sctx->priv_user().str, sctx->host_or_ip().str, fields->name(),
             table_name);
  return true;
}

static bool check_grant_db_routine(
    THD *thd, const char *db,
    malloc_unordered_multimap<std::string, unique_ptr_destroy_only<GRANT_NAME>>
        *hash) {
  DBUG_TRACE;
  Security_context *sctx = thd->security_context();
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));

  if (sctx->get_active_roles()->size() != 0 && db != nullptr) {
    DBUG_PRINT("info",
               ("Using roles Acl_map to detect schema level privileges"));
    ulong acl = sctx->db_acl({db, strlen(db)});
    return acl == 0;
  } else {
    for (const auto &key_and_value : *hash) {
      GRANT_NAME *item = key_and_value.second.get();

      if (strcmp(item->user, sctx->priv_user().str) == 0 &&
          strcmp(item->db, db) == 0 &&
          item->host.compare_hostname(sctx->host().str, sctx->ip().str)) {
        return false;
      }
    }  // end for
  }

  return true;
}

bool has_any_table_acl(Security_context *sctx, const LEX_CSTRING &str) {
  if (sctx->get_active_roles()->size() > 0) {
    return sctx->any_table_acl(str);
  }
  return false;
}

bool has_any_routine_acl(Security_context *sctx, const LEX_CSTRING &db) {
  if (sctx->get_active_roles()->size() > 0) {
    return sctx->any_sp_acl(db);
  }
  return false;
}

/**
  Check if a user has the right to access a database.
  Access is accepted if the user has a database operations related grant
  (i.e. not including the GRANT_ACL) for any table/column/routine in the
  database.

  @param thd The thread handler
  @param db The name of the database

  @retval 1 Access is denied
  @retval 0 Otherwise
*/
bool check_grant_db(THD *thd, const char *db) {
  DBUG_TRACE;
  Security_context *sctx = thd->security_context();
  LEX_CSTRING priv_user = sctx->priv_user();
  bool error = true;

  if (sctx->get_active_roles()->size() > 0) {
    size_t db_len = strlen(db);
    ulong db_access = sctx->db_acl({db, db_len});
    if ((db_access & DB_OP_ACLS) != 0) return false;
    return !has_any_table_acl(sctx, {db, db_len}) &&
           !has_any_routine_acl(sctx, {db, db_len});
  }

  std::string key = to_string(priv_user);
  key.push_back('\0');
  key.append(db);
  key.push_back('\0');

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  for (const auto &key_and_value : *column_priv_hash) {
    GRANT_TABLE *grant_table = key_and_value.second.get();
    if (grant_table->hash_key.compare(0, key.size(), key) == 0 &&
        grant_table->host.compare_hostname(sctx->host().str, sctx->ip().str) &&
        ((grant_table->privs | grant_table->cols) & TABLE_OP_ACLS)) {
      error = false; /* Found match. */
      DBUG_PRINT("info", ("Detected table level acl in column_priv_hash"));
      break;
    }
  }

  if (error) {
    DBUG_PRINT("info", ("No table level acl in column_priv_hash; checking "
                        "for schema level acls"));
    error = check_grant_db_routine(thd, db, proc_priv_hash.get()) &&
            check_grant_db_routine(thd, db, func_priv_hash.get());
  }

  return error;
}

/****************************************************************************
  Check routine level grants

  SYNPOSIS
   bool check_grant_routine()
   thd          Thread handler
   want_access  Bits of privileges user needs to have
   procs        List of routines to check. The user should have 'want_access'
   is_proc      True if the list is all procedures, else functions
   no_errors    If 0 then we write an error. The error is sent directly to
                the client

   RETURN
     false  ok
     true  Error: User did not have the requested privielges
****************************************************************************/

bool check_grant_routine(THD *thd, ulong want_access, TABLE_LIST *procs,
                         bool is_proc, bool no_errors) {
  TABLE_LIST *table;
  Security_context *sctx = thd->security_context();
  const char *user = sctx->priv_user().str;
  const char *host = sctx->priv_host().str;
  const std::string db_name(procs->db, procs->db_length);
  bool has_roles = thd->security_context()->get_active_roles()->size() > 0;
  DBUG_TRACE;

  want_access &= ~sctx->master_access(db_name);
  if (!want_access) return false;  // ok

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  for (table = procs; table; table = table->next_global) {
    if (has_roles) {
      ulong acl;
      if (is_proc) {
        acl =
            sctx->procedure_acl({table->db, table->db_length},
                                {table->table_name, table->table_name_length});
      } else {
        acl = sctx->function_acl({table->db, table->db_length},
                                 {table->table_name, table->table_name_length});
      }
      table->grant.privilege |= acl;
      DBUG_PRINT("info",
                 ("Checking Acl_map for proc acls in %s.%s; "
                  "found %lu",
                  table->db, table->table_name, table->grant.privilege));
    } else {
      GRANT_NAME *grant_proc;
      if ((grant_proc =
               routine_hash_search(host, sctx->ip().str, table->db, user,
                                   table->table_name, is_proc, false))) {
        table->grant.privilege |= grant_proc->privs;
        DBUG_PRINT("info", ("Checking for routine acls in %s; "
                            "found %lu",
                            table->db, grant_proc->privs));
      }
    }
    if ((want_access & table->grant.privilege) != want_access) {
      want_access &= ~table->grant.privilege;
      goto err;
    }
  }
  return false;

err:
  if (!no_errors) {
    char buff[1024];
    const char *command = "";
    if (table) strxmov(buff, table->db, ".", table->table_name, NullS);
    if (want_access & EXECUTE_ACL)
      command = "execute";
    else if (want_access & ALTER_PROC_ACL)
      command = "alter routine";
    else if (want_access & GRANT_ACL)
      command = "grant";
    my_error(ER_PROCACCESS_DENIED_ERROR, MYF(0), command, user, host,
             table ? buff : "unknown");
  }
  return true;
}

/*
  Check if routine has any of the
  routine level grants

  SYNPOSIS
   bool    check_routine_level_acl()
   thd          Thread handler
   db           Database name
   name         Routine name

  RETURN
   false           Ok
   true            error
*/

static bool check_routine_level_acl(THD *thd, const char *db, const char *name,
                                    bool is_proc) {
  DBUG_TRACE;
  bool no_routine_acl = true;
  GRANT_NAME *grant_proc;
  Security_context *sctx = thd->security_context();
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return no_routine_acl;

  if ((grant_proc =
           routine_hash_search(sctx->priv_host().str, sctx->ip().str, db,
                               sctx->priv_user().str, name, is_proc, false)))
    no_routine_acl = !(grant_proc->privs & SHOW_PROC_ACLS);
  return no_routine_acl;
}

/*****************************************************************************
  Functions to retrieve the grant for a table/column  (for SHOW functions)
*****************************************************************************/

ulong get_table_grant(THD *thd, TABLE_LIST *table) {
  ulong privilege;
  Security_context *sctx = thd->security_context();
  const char *db = table->db ? table->db : thd->db().str;
  GRANT_TABLE *grant_table;
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);

  if (!acl_cache_lock.lock(false)) return (NO_ACCESS);

  grant_table =
      table_hash_search(sctx->host().str, sctx->ip().str, db,
                        sctx->priv_user().str, table->table_name, false);
  table->grant.grant_table = grant_table;  // Remember for column test
  table->grant.version = grant_version;
  if (grant_table) table->grant.privilege |= grant_table->privs;
  privilege = table->grant.privilege;
  return privilege;
}

/*
  Determine the access priviliges for a field.

  SYNOPSIS
    get_column_grant()
    thd         thread handler
    grant       grants table descriptor
    db_name     name of database that the field belongs to
    table_name  name of table that the field belongs to
    field_name  name of field

  DESCRIPTION
    The procedure may also modify: grant->grant_table and grant->version.

  RETURN
    The access priviliges for the field db_name.table_name.field_name
*/

ulong get_column_grant(THD *thd, GRANT_INFO *grant, const char *db_name,
                       const char *table_name, const char *field_name) {
  GRANT_TABLE *grant_table;
  GRANT_COLUMN *grant_column;
  ulong priv;
  Security_context *sctx = thd->security_context();
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);

  if (!acl_cache_lock.lock(false)) return (NO_ACCESS);

  /* reload table if someone has modified any grants */
  /*
    note: grant->grant_table is set iff we need to check column level routines
    and table level privileges hasn't isn't enough to fulfill the requirement.
    However, we might be using this routine to check table level privileges
    too so we must still check these before we continue. Doh..
  */
  if (sctx->get_active_roles()->size() != 0 && !grant->grant_table) {
    priv = grant->privilege;
    Grant_table_aggregate aggr = sctx->table_and_column_acls(
        {(const char *)db_name, strlen(db_name)},
        {(const char *)table_name, strlen(table_name)});
    priv |= aggr.table_access;
    /* Find our column */
    std::string q_name;
    q_name.append(field_name);
    Column_map::iterator it = aggr.columns.find(std::string(q_name.c_str()));
    if (it != aggr.columns.end()) {
      priv |= it->second;
    }

  } else {
    if (grant->version != grant_version) {
      grant->grant_table = table_hash_search(
          sctx->host().str, sctx->ip().str, db_name, sctx->priv_user().str,
          table_name, false);         /* purecov: inspected */
      grant->version = grant_version; /* purecov: inspected */
    }

    if (!(grant_table = grant->grant_table))
      priv = grant->privilege;
    else {
      grant_column =
          column_hash_search(grant_table, field_name, strlen(field_name));
      if (!grant_column)
        priv = (grant->privilege | grant_table->privs);
      else
        priv = (grant->privilege | grant_table->privs | grant_column->rights);
    }
  }
  return priv;
}

/*
  Make a clear-text version of the requested privilege.
*/

void get_privilege_desc(char *to, uint max_length, ulong access) {
  uint pos;
  char *start = to;
  DBUG_ASSERT(max_length >= 30);  // For end ', ' removal

  if (access) {
    max_length--;  // Reserve place for end-zero
    for (pos = 0; access; pos++, access >>= 1) {
      if ((access & 1) &&
          global_acls_vector[pos].length() + (uint)(to - start) < max_length) {
        to = my_stpcpy(to, global_acls_vector[pos].c_str());
        *to++ = ',';
        *to++ = ' ';
      }
    }
    to--;  // Remove end ' '
    to--;  // Remove end ','
  }
  *to = 0;
}

/**
  Iterate a string by comma separation and apply a function on each chunk
  separated by the commas.
  @param str The string to be iterated
  @param f   A function which will receive the comma separated strings.

*/
void iterate_comma_separated_quoted_string(
    std::string str, const std::function<bool(const std::string)> &f) {
  if (str.length() == 0) return;
  std::string::iterator i = str.begin();
  std::stringstream ss;
  bool q1 = false;
  bool q2 = false;
  bool q3 = false;
  while (i != str.end()) {
    if (!q2 && !q3 && *i == '`') {
      if (q1)
        q1 = false;
      else
        q1 = true;
    } else if (!q1 && !q3 && *i == '\'') {
      if (q2)
        q2 = false;
      else
        q2 = true;
    } else if (!q1 && !q2 && *i == '\'') {
      if (q3)
        q3 = false;
      else
        q3 = true;
    } else if (q1 == false && q2 == false && q3 == false && *i == ',') {
      if (f(ss.str())) return;
      ss.str("");
      ++i;
      continue;
    } else if (q1 == false && q2 == false && q3 == false && *i == ' ') {
      ++i;
      continue;
    }
    ss << *i;
    ++i;
  }
  f(ss.str());
}

/**
  Return the unquoted authorization id as a user,host-tuple
  @param str The quoted or unquoted string representation of an authid

  @return The unquoted authorization id as a user,host-tuple
*/

std::pair<std::string, std::string> get_authid_from_quoted_string(
    std::string str) {
  std::string::iterator i;
  std::stringstream user;
  std::stringstream host;
  int ct = 0;
  bool q1 = false;
  bool q2 = false;
  bool q3 = false;
  for (i = str.begin(); i != str.end(); ++i) {
    if (!q2 && !q3 && *i == '`') {
      if (q1)
        q1 = false;
      else
        q1 = true;
      continue;
    } else if (!q1 && !q3 && *i == '\'') {
      if (q2)
        q2 = false;
      else
        q2 = true;
      continue;
    } else if (!q1 && !q2 && *i == '"') {
      if (q3)
        q3 = false;
      else
        q3 = true;
      continue;
    } else if (q1 == false && q2 == false && q3 == false && *i == '@') {
      ++ct;
      continue;
    } else if (q1 == false && q2 == false && q3 == false && *i == ' ') {
      continue;
    }
    if (ct == 0) {
      user << *i;
    } else {
      host << *i;
    }
  }
  if (ct == 0 && !user.str().empty()) host << '%';
  return std::make_pair(user.str(), host.str());
}

bool operator==(const std::pair<Role_id, bool> &rid, const Auth_id_ref &ref) {
  return (rid.first.user() == std::string(ref.first.str, ref.first.length) &&
          rid.first.host() == std::string(ref.second.str, ref.second.length));
}

bool operator==(const Auth_id_ref &ref, const std::pair<Role_id, bool> &rid) {
  return operator==(rid, ref);
}

void get_privilege_access_maps(
    ACL_USER *acl_user, const List_of_auth_id_refs *using_roles, ulong *access,
    Db_access_map *db_map, Db_access_map *db_wild_map,
    Table_access_map *table_map, SP_access_map *sp_map, SP_access_map *func_map,
    List_of_granted_roles *granted_roles, Grant_acl_set *with_admin_acl,
    Dynamic_privileges *dynamic_acl, Restrictions &restrictions) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  List_of_auth_id_refs activated_roles_ref;
  boost::graph_traits<Granted_roles_graph>::edge_iterator ei, ei_end;
  /* First we check the current users access control */
  // Get global access
  *access = acl_user->access;

  DBUG_PRINT("info", ("Global access for acl_user %s@%s is %lu", acl_user->user,
                      acl_user->host.get_host(), acl_user->access));
  // Get database access
  get_database_access_map(acl_user, db_map, db_wild_map);
  // Get table- and column privileges
  get_table_access_map(acl_user, table_map);
  // get stored procedure privileges
  get_sp_access_map(acl_user, sp_map, proc_priv_hash.get());
  // get user function privileges
  get_sp_access_map(acl_user, func_map, func_priv_hash.get());
  // get dynamic privileges
  get_dynamic_privileges(acl_user, dynamic_acl);
  /* Find out the existing restrictions of the current user. */
  restrictions = acl_restrictions->find_restrictions(acl_user);

  /* We don't support role hierarchies for anonymous accounts. */
  if (acl_user->user == nullptr) return;

  /*
    Temporarily apply the mandatory roles on this user for the sake of
    generating an Acl_map.
  */
  std::vector<Role_id> mandatory_roles;
  std::vector<Role_vertex_descriptor> mandatory_roles_vertex_ids;
  get_mandatory_roles(&mandatory_roles);

  /* Only check roles if there are any granted roles at all */
  boost::tie(ei, ei_end) = boost::edges(*g_granted_roles);
  Role_vertex_descriptor user_vertex, active_role_vertex;

  std::string user_key = create_authid_str_from(acl_user);
  Role_index_map::iterator user_vertex_it = g_authid_to_vertex->find(user_key);
  bool has_granted_roles = (ei != ei_end);
  std::set<Role_id> granted_active_roles;
  std::set<Role_id> all_granted_roles;
  std::set<Role_id> all_active_roles;
  boost::vector_property_map<boost::default_color_type> v_color(
      boost::num_vertices(*g_granted_roles));

  Get_access_maps vis(acl_user, access, db_map, db_wild_map, table_map, sp_map,
                      func_map, with_admin_acl, dynamic_acl, &restrictions);
  if (has_granted_roles || mandatory_roles.size() > 0) {
    bool acl_user_has_vertex = (user_vertex_it != g_authid_to_vertex->end());
    if (!acl_user_has_vertex) return;
    user_vertex = user_vertex_it->second;
    if (acl_user_has_vertex) {
      get_granted_roles(user_vertex, [&](const Role_id &rid, bool with_admin) {
        all_granted_roles.insert(rid);
        granted_roles->push_back(std::make_pair(rid, with_admin));
      });
      for (auto &rid : mandatory_roles) {
        all_granted_roles.insert(rid);
      }
      for (auto &rid : *using_roles) {
        Role_id id(rid.first, rid.second);
        all_active_roles.insert(id);
      }

      std::set_intersection(
          all_granted_roles.begin(), all_granted_roles.end(),
          all_active_roles.begin(), all_active_roles.end(),
          std::inserter(granted_active_roles, granted_active_roles.begin()));
      int vertex_count = 0;
      for (auto &&rid : granted_active_roles) {
        String rolestr;
        append_identifier(&rolestr, rid.user().c_str(), rid.user().length());
        rolestr.append('@');
        append_identifier(&rolestr, rid.host().c_str(), rid.host().length());
        Role_index_map::iterator rindex =
            g_authid_to_vertex->find(rolestr.c_ptr_quick());
        if (rindex == g_authid_to_vertex->end()) {
          THD *thd = current_thd;
          if (thd) {
            push_warning_printf(thd, Sql_condition::SL_WARNING,
                                ER_UNKNOWN_AUTHID,
                                "Illegal role %s@%s was ignored.",
                                rid.user().c_str(), rid.host().c_str());
          }
          continue;  // next role
        }
        active_role_vertex = rindex->second;
        if (vertex_count == 0) {
          /* breadth_first_search will initialize our v_color vector for us */
          boost::breadth_first_search(*g_granted_roles, active_role_vertex,
                                      boost::color_map(v_color).visitor(vis));
        } else {
          /* breadth_first_visit will not reinitialize the v_color vector */
          boost::breadth_first_visit(*g_granted_roles, active_role_vertex,
                                     boost::color_map(v_color).visitor(vis));
          ++vertex_count;
        }

        /*
          An active edge might have been granted WITH ADMIN; make sure
          we update the temporary edge with this property.
        */
        Role_edge_descriptor edge;
        bool found;
        boost::tie(edge, found) =
            boost::edge(user_vertex, active_role_vertex, *g_granted_roles);
        if (found) {
          int with_admin_opt =
              boost::get(boost::edge_capacity_t(), *g_granted_roles)[edge];
          if (with_admin_opt) {
            with_admin_acl->insert(std::string(rolestr.c_ptr()));
          }
        }

      }  // end for
    }    // if user_vertex_it != g_authid_to_vertex->end()
  }      // if has_granted_roles
  DBUG_PRINT("info", ("Global access for role user %s@%s is %lu",
                      acl_user->user, acl_user->host.get_host(), *access));
}

/**
  SHOW GRANTS FOR user USING [ALL | role [,role ...]]
  @param thd
  @param lex_user
  @param using_roles An forward iterable container of LEX_STRING std::pair
  @param show_mandatory_roles true means mandatory roles are listed
  @param have_using_clause true means there's a non-empty USING clause specified

  @return Success status
*/
bool mysql_show_grants(THD *thd, LEX_USER *lex_user,
                       const List_of_auth_id_refs &using_roles,
                       bool show_mandatory_roles, bool have_using_clause) {
  int error = 0;
  ACL_USER *acl_user = nullptr;
  char buff[1024];
  DBUG_TRACE;

  DBUG_ASSERT(initialized);
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  acl_user = find_acl_user(lex_user->host.str, lex_user->user.str, true);
  if (!acl_user) {
    my_error(ER_NONEXISTING_GRANT, MYF(0), lex_user->user.str,
             lex_user->host.str);
    return true;
  }

  /*
    For a SHOW GRANTS USING one needs to check if the session has access to
    the roles specied in the USING clause.
    But if there's no USING clause the list of active session roles is used
    insetad. But since this list is a copy into the thread's security context
    the active roles might have stopped being granted into the global
    structure.
    Thus a check if these are still granted might fail.
    So we skip the check if there's no explict USING knowing that the check
    has already been perfromed for these when they were set.
  */
  if (have_using_clause) {
    std::vector<Role_id> mandatory_roles;
    get_mandatory_roles(&mandatory_roles);
    List_of_granted_roles granted_roles;
    get_granted_roles(lex_user, &granted_roles);
    for (auto &role_ref : using_roles) {
      std::string authid(create_authid_str_from(role_ref));
      if (find(granted_roles.begin(), granted_roles.end(), authid) ==
          granted_roles.end()) {
        if (std::find_if(mandatory_roles.begin(), mandatory_roles.end(),
                         [&](const Role_id &id) -> bool {
                           std::string id_str, rid_str;
                           id.auth_str(&id_str);
                           Role_id rid(role_ref.first, role_ref.second);
                           rid.auth_str(&rid_str);
                           return (Role_id(role_ref.first, role_ref.second) ==
                                   id);
                         }) == mandatory_roles.end()) {
          my_error(ER_ROLE_NOT_GRANTED, MYF(0), role_ref.first.str,
                   role_ref.second.str, lex_user->user.str, lex_user->host.str);
          return true;
        }
      }
    }
  }

  Item_string *field = new Item_string("", 0, &my_charset_latin1);
  List<Item> field_list;
  field->max_length = 1024;
  strxmov(buff, "Grants for ", lex_user->user.str, "@", lex_user->host.str,
          NullS);
  field->item_name.set(buff);
  field_list.push_back(field);
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    return true;
  }
  // aggregate over the active role and user privileges
  Db_access_map db_map;
  Db_access_map db_wild_map;
  Table_access_map table_map;
  SP_access_map sp_map;
  SP_access_map func_map;
  Grant_acl_set with_admin_acl;
  Dynamic_privileges dynamic_acl;
  List_of_granted_roles granted_roles;
  Restrictions restrictions(thd->mem_root);
  ulong access;
  table_map.set_thd(thd);
  get_privilege_access_maps(acl_user, &using_roles, &access, &db_map,
                            &db_wild_map, &table_map, &sp_map, &func_map,
                            &granted_roles, &with_admin_acl, &dynamic_acl,
                            restrictions);
  String output;
  make_global_privilege_statement(thd, access, acl_user, &output);
  Protocol *protocol = thd->get_protocol();
  protocol->start_row();
  protocol->store_string(output.ptr(), output.length(), output.charset());
  protocol->end_row();

  make_dynamic_privilege_statement(thd, acl_user, protocol, dynamic_acl);
  make_database_privilege_statement(thd, acl_user, protocol, db_map,
                                    db_wild_map, restrictions.db());
  make_table_privilege_statement(thd, acl_user, protocol, table_map);
  make_sp_privilege_statement(thd, acl_user, protocol, sp_map, 0);
  make_sp_privilege_statement(thd, acl_user, protocol, func_map, 1);
  make_proxy_privilege_statement(thd, acl_user, protocol);
  make_roles_privilege_statement(thd, acl_user, protocol, granted_roles,
                                 show_mandatory_roles);
  make_with_admin_privilege_statement(thd, acl_user, protocol, with_admin_acl,
                                      granted_roles);

  my_eof(thd);
  return error;
}

void roles_graphml(THD *thd, String *str) {
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return;
  boost::dynamic_properties dp;
  dp.property("name", boost::get(boost::vertex_name_t(), *g_granted_roles));
  dp.property("color", boost::get(boost::edge_capacity_t(), *g_granted_roles));
  std::stringstream ss;
  boost::write_graphml(ss, *g_granted_roles, dp, true);
  std::string out = ss.str();
  str->copy(out.c_str(), out.length(), system_charset_info);
}

/**
  Remove db access privileges.

  @param thd    Current thread execution context.
  @param table  Pointer to a TABLE object for opened table mysql.db.
  @param lex_user  User information.

  @return  Operation result
    @retval  0    OK.
    @retval  1    Application error happen, it is allowed
                  continuing of operations.
    @retval  < 0  Engine error.
*/

static int remove_db_access_privileges(THD *thd, TABLE *table,
                                       const LEX_USER &lex_user) {
  ACL_DB *acl_db;
  int revoked, result = 0;

  /*
    Because acl_dbs shrink and may re-order as privileges are removed,
    removal occurs in a repeated loop until no more privileges are revoked.
  */
  do {
    for (revoked = 0, acl_db = acl_dbs->begin(); acl_db != acl_dbs->end();) {
      const char *user, *host;

      if (!(user = acl_db->user)) user = "";
      if (!(host = acl_db->host.get_host())) host = "";

      if (!strcmp(lex_user.user.str, user) &&
          !strcmp(lex_user.host.str, host)) {
        int ret =
            replace_db_table(thd, table, acl_db->db, lex_user, ~(ulong)0, true);
        if (!ret) {
          /*
            Don't increment loop variable as replace_db_table deleted the
            current element in acl_dbs.
          */
          revoked = 1;
          continue;
        } else if (ret < 0)
          return ret;  // Something went wrong
        else
          /*
            For the case when replace_db_table() returns 1 we continue
            iteration in order to remove all db access privileges. It is safe
            since this function is called as part of handling the statement
            REVOKE ALL.
          */
          result = 1;
      }
      ++acl_db;
    }
  } while (revoked);

  return result;
}

/**
  Remove column access privileges.

  @param thd                 Thread handler.
  @param tables_priv_table   Pointer to a TABLE object for opened table
                             mysql.tables_priv_table.
  @param columns_priv_table  Pointer to a TABLE object for opened table
                             mysql.columns_priv_table.
  @param lex_user            User information.

  @return  Operation result
    @retval  0    OK.
    @retval  1    Application error happen, it is allowed
                  continuing of operations.
    @retval  < 0  Engine error.
*/

static int remove_column_access_privileges(THD *thd, TABLE *tables_priv_table,
                                           TABLE *columns_priv_table,
                                           const LEX_USER &lex_user) {
  bool revoked = false;
  int result = 0;
  /*
    Remove column access.
    Because column_priv_hash shrink and may re-order as privileges are removed,
    removal occurs in a repeated loop until no more privileges are revoked.
  */
  do {
    revoked = false;
    for (auto it = column_priv_hash->begin(), next_it = it;
         it != column_priv_hash->end(); it = next_it) {
      /*
        Store an iterator pointing to the next element now, since
        replace_table_table could delete elements, invalidating "it".
      */
      next_it = next(it);

      const char *user, *host;
      GRANT_TABLE *grant_table = it->second.get();
      if (!(user = grant_table->user)) user = "";
      if (!(host = grant_table->host.get_host())) host = "";

      if (!strcmp(lex_user.user.str, user) &&
          !strcmp(lex_user.host.str, host)) {
        // Hold on to grant_table if it gets deleted, since we use it below.
        std::unique_ptr<GRANT_TABLE, Destroy_only<GRANT_TABLE>>
            deleted_grant_table;

        int ret = replace_table_table(
            thd, grant_table, &deleted_grant_table, tables_priv_table, lex_user,
            grant_table->db, grant_table->tname, ~(ulong)0, 0, true);
        if (ret < 0) {
          return ret;
        } else if (ret > 0) {
          /*
            For the case when replace_table_table() returns 1 we continue
            iteration in order to remove all column access privileges.
          */
          result = 1;
          revoked = true;
          break;
        } else {
          if (!grant_table->cols) {
            revoked = true;
            break;
          }
          List<LEX_COLUMN> columns;
          ret = replace_column_table(thd, grant_table, columns_priv_table,
                                     lex_user, columns, grant_table->db,
                                     grant_table->tname, ~(ulong)0, true);
          if (!ret) {
            revoked = true;
            break;
          }
          /*
            If we come there then the variable ret always has a value < 0 since
            the actual argument 'columns' doesn't contain any elements
          */
          DBUG_ASSERT(ret < 0);

          return ret;
        }
      }
    }
  } while (revoked);

  return result;
}

/**
  Remove procedure access privileges.

  @param thd                 Thread handler.
  @param procs_priv_table    Pointer to a TABLE object for opened table
                             mysql.procs_priv_table.
  @param lex_user            User information.

  @return  Operation result.
    @retval  0    OK.
    @retval  1    Application error happen, it is allowed
                  continuing of operations.
    @retval  < 0  Engine error.
*/

static int remove_procedure_access_privileges(THD *thd, TABLE *procs_priv_table,
                                              const LEX_USER &lex_user) {
  /* Remove procedure access */
  int result = 0;
  bool revoked;
  for (int is_proc = 0; is_proc < 2; is_proc++) do {
      malloc_unordered_multimap<std::string,
                                unique_ptr_destroy_only<GRANT_NAME>> *hash =
          is_proc ? proc_priv_hash.get() : func_priv_hash.get();
      revoked = false;
      for (auto it = hash->begin(), next_it = it; it != hash->end();
           it = next_it) {
        /*
          Store an iterator pointing to the next element now, since
          replace_routine_table could delete elements, invalidating "it".
        */
        next_it = next(it);

        const char *user, *host;
        GRANT_NAME *grant_proc = it->second.get();
        if (!(user = grant_proc->user)) user = "";
        if (!(host = grant_proc->host.get_host())) host = "";

        if (!strcmp(lex_user.user.str, user) &&
            !strcmp(lex_user.host.str, host)) {
          int ret = replace_routine_table(
              thd, grant_proc, procs_priv_table, lex_user, grant_proc->db,
              grant_proc->tname, is_proc, ~(ulong)0, true);

          if (!ret) {
            revoked = true;
            continue;
          } else if (ret < 0)
            return ret;
          else
            /*
              For the case when replace_routine_table() returns 1 we continue
              iteration in order to remove all procedure access privileges.
              It is safe since this function is called as part of handling
              the statement REVOKE ALL.
            */
            result = 1;
        }
      }
    } while (revoked);

  return result;
}

/*
  Revoke all privileges from a list of users.

  SYNOPSIS
    mysql_revoke_all()
    thd                         The current thread.
    list                        The users to revoke all privileges from.

  RETURN
    > 0         Error. Error message already sent.
    0           OK.
    < 0         Error. Error message not yet sent.
*/

bool mysql_revoke_all(THD *thd, List<LEX_USER> &list) {
  bool result = false;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  bool transactional_tables;
  int ret = 0;
  DBUG_TRACE;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);
  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    TABLE *dynpriv_table = tables[ACL_TABLES::TABLE_DYNAMIC_PRIV].table;
    LEX_USER *lex_user, *tmp_lex_user;
    List_iterator<LEX_USER> user_list(list);

    while ((tmp_lex_user = user_list++)) {
      ulong what_to_set = 0;
      if (!(lex_user = get_current_user(thd, tmp_lex_user))) {
        result = true;
        continue;
      }
      ACL_USER *acl_user =
          find_acl_user(lex_user->host.str, lex_user->user.str, true);
      if (acl_user == nullptr) {
        result = true;
        continue;
      }

      Update_dynamic_privilege_table update_table(thd, dynpriv_table);
      if ((result = revoke_all_dynamic_privileges(
               lex_user->user, lex_user->host, update_table))) {
        break;
      }
      /* copy password expire attributes to individual user */
      lex_user->alter_status = thd->lex->alter_password;

      acl_table::Pod_user_what_to_update what_to_update;
      what_to_update.m_what = (what_to_set | ACCESS_RIGHTS_ATTR);
      ulong rights = ~(ulong)0;
      DB_restrictions db_restrictions(nullptr);
      Restrictions restrictions(nullptr);
      std::unique_ptr<Restrictions_aggregator> aggregator =
          Restrictions_aggregator_factory::create(thd, acl_user, nullptr,
                                                  rights, false);
      if (aggregator) {
        if (aggregator->generate(db_restrictions)) {
          result = true;
          continue;
        }
        what_to_update.m_what |= USER_ATTRIBUTES;
        what_to_update.m_user_attributes |=
            acl_table::USER_ATTRIBUTE_RESTRICTIONS;
        restrictions.set_db(db_restrictions);
      }
      if ((ret = replace_user_table(thd, tables[ACL_TABLES::TABLE_USER].table,
                                    lex_user, rights, true, false,
                                    what_to_update, &restrictions))) {
        result = true;
        if (ret < 0) break;

        continue;
      }

      int ret1, ret2, ret3;
      if ((ret1 = remove_db_access_privileges(
               thd, tables[ACL_TABLES::TABLE_DB].table, *lex_user)) < 0 ||
          (ret2 = remove_column_access_privileges(
               thd, tables[ACL_TABLES::TABLE_TABLES_PRIV].table,
               tables[ACL_TABLES::TABLE_COLUMNS_PRIV].table, *lex_user)) < 0 ||
          (ret3 = remove_procedure_access_privileges(
               thd, tables[ACL_TABLES::TABLE_PROCS_PRIV].table, *lex_user)) <
              0) {
        result = true;  // Something went wrong
        break;
      } else if (ret1 || ret2 || ret3) {
        result = true;
        continue;
      }
    }  // end while

    DBUG_EXECUTE_IF("force_mysql_revoke_all_fail", { result = 1; });

    if (result && !thd->is_error()) my_error(ER_REVOKE_GRANTS, MYF(0));

    result = log_and_commit_acl_ddl(thd, transactional_tables);
    get_global_acl_cache()->increase_version();
  } /* Critical section */

  /* Notify storage engines */
  if (!result) {
    acl_notify_htons(thd, SQLCOM_REVOKE_ALL, &list);
  }

  return result;
}

/**
  If the defining user for a routine does not exist, then the ACL lookup
  code should raise two errors which we should intercept.  We convert the more
  descriptive error into a warning, and consume the other.

  If any other errors are raised, then we set a flag that should indicate
  that there was some failure we should complain at a higher level.
*/
class Silence_routine_definer_errors : public Internal_error_handler {
 public:
  Silence_routine_definer_errors() : is_grave(false) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *) {
    if (*level == Sql_condition::SL_ERROR) {
      if (sql_errno == ER_NONEXISTING_PROC_GRANT) {
        /* Convert the error into a warning. */
        *level = Sql_condition::SL_WARNING;
        return true;
      } else
        is_grave = true;
    }

    return false;
  }

  bool has_errors() const { return is_grave; }

 private:
  bool is_grave;
};

/**
  Revoke privileges for all users on a stored procedure.  Use an error handler
  that converts errors about missing grants into warnings.

  @param thd       The current thread.
  @param sp_db     DB of the stored procedure
  @param sp_name   Name of the stored procedure
  @param is_proc   True if this is a SP rather than a function.

  @retval
    false       OK.
  @retval
    true        Error. Error message not yet sent.
*/

bool sp_revoke_privileges(THD *thd, const char *sp_db, const char *sp_name,
                          bool is_proc) {
  bool revoked;
  int int_result;
  bool result = false;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  Silence_routine_definer_errors error_handler;
  bool transactional_tables;
  DBUG_TRACE;

  if (0 != (int_result = open_grant_tables(thd, tables, &transactional_tables)))
    return int_result != 1;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);
  if (!acl_cache_lock.lock()) {
    commit_and_close_mysql_tables(thd);
    return true;
  }

  /* Be sure to pop this before exiting this scope! */
  thd->push_internal_handler(&error_handler);

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  /* Remove procedure access */
  malloc_unordered_multimap<std::string, unique_ptr_destroy_only<GRANT_NAME>>
      *hash = is_proc ? proc_priv_hash.get() : func_priv_hash.get();
  do {
    revoked = false;
    for (auto it = hash->begin(), next_it = it; it != hash->end();
         it = next_it) {
      /*
        Store an iterator pointing to the next element now, since
        replace_routine_table could delete elements, invalidating "it".
      */
      next_it = next(it);
      GRANT_NAME *grant_proc = it->second.get();
      if (!my_strcasecmp(&my_charset_utf8_bin, grant_proc->db, sp_db) &&
          !my_strcasecmp(system_charset_info, grant_proc->tname, sp_name)) {
        LEX_USER lex_user;
        lex_user.user.str = grant_proc->user;
        lex_user.user.length = strlen(grant_proc->user);
        lex_user.host.str =
            grant_proc->host.get_host() ? grant_proc->host.get_host() : "";
        lex_user.host.length = grant_proc->host.get_host()
                                   ? strlen(grant_proc->host.get_host())
                                   : 0;

        int ret = replace_routine_table(
            thd, grant_proc, tables[4].table, lex_user, grant_proc->db,
            grant_proc->tname, is_proc, ~(ulong)0, true);
        if (ret < 0) {
          result = true;
          revoked = false;
          break;
        } else if (ret == 0) {
          revoked = true;
          continue;
        }
      }
    }
  } while (revoked);

  /* We don't want to write to binlog or notify htons about this. */
  result |= log_and_commit_acl_ddl(thd, transactional_tables, nullptr, nullptr,
                                   result, false);

  thd->pop_internal_handler();
  return error_handler.has_errors() || result;
}

/**
  Grant EXECUTE,ALTER privilege for a stored procedure

  @param      thd                  The current thread.
  @param      sp_db                DB of the stored procedure.
  @param      sp_name              Name of the stored procedure
  @param      is_proc              True if this is a SP rather than a function

  @retval false Success
  @retval true An error occurred. Error message not yet sent.
*/

bool sp_grant_privileges(THD *thd, const char *sp_db, const char *sp_name,
                         bool is_proc) {
  TABLE_LIST tables[1];
  List<LEX_USER> user_list;
  bool result = true;
  Dummy_error_handler error_handler;

  DBUG_TRACE;

  LEX_CSTRING sctx_user = thd->security_context()->priv_user();
  LEX_CSTRING sctx_host = thd->security_context()->priv_host();
  LEX_USER *combo =
      LEX_USER::alloc(thd, (LEX_STRING *)&sctx_user, (LEX_STRING *)&sctx_host);
  if (combo == nullptr) return true;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  ACL_USER *au = find_acl_user(combo->host.str, combo->user.str, false);
  if (au == nullptr) {
    result = true;
    goto end;
  }

  acl_cache_lock.unlock();

  new (&tables[0]) TABLE_LIST();
  user_list.empty();

  tables->db = sp_db;
  tables->table_name = tables->alias = sp_name;

  lex_string_strmake(thd->mem_root, &combo->user, combo->user.str,
                     strlen(combo->user.str));
  lex_string_strmake(thd->mem_root, &combo->host, combo->host.str,
                     strlen(combo->host.str));

  if (user_list.push_back(combo)) return true;

  thd->lex->ssl_type = SSL_TYPE_NOT_SPECIFIED;
  thd->lex->ssl_cipher = thd->lex->x509_subject = thd->lex->x509_issuer =
      nullptr;
  memset(&thd->lex->mqh, 0, sizeof(thd->lex->mqh));
  /* set default values */
  thd->lex->alter_password.cleanup();

  combo->alter_status = thd->lex->alter_password;

  /*
    Only care about whether the operation failed or succeeded
    as all errors will be handled later.
  */
  thd->push_internal_handler(&error_handler);
  result = mysql_routine_grant(thd, tables, is_proc, user_list,
                               DEFAULT_CREATE_PROC_ACLS, false, false);
  thd->pop_internal_handler();
end:
  return result;
}

static bool update_schema_privilege(THD *thd, TABLE *table, char *buff,
                                    const char *db, const char *t_name,
                                    const char *column, size_t col_length,
                                    const char *priv, size_t priv_length,
                                    const char *is_grantable) {
  int i = 2;
  CHARSET_INFO *cs = system_charset_info;
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  restore_record(table, s->default_values);
  table->field[0]->store(buff, strlen(buff), cs);
  table->field[1]->store(STRING_WITH_LEN("def"), cs);
  if (db) table->field[i++]->store(db, strlen(db), cs);
  if (t_name) table->field[i++]->store(t_name, strlen(t_name), cs);
  if (column) table->field[i++]->store(column, col_length, cs);
  table->field[i++]->store(priv, priv_length, cs);
  table->field[i]->store(is_grantable, strlen(is_grantable), cs);
  return schema_table_store_record(thd, table);
}

/*
  fill effective privileges for table

  SYNOPSIS
    fill_effective_table_privileges()
    thd     thread handler
    grant   grants table descriptor
    db      db name
    table   table name
*/

void fill_effective_table_privileges(THD *thd, GRANT_INFO *grant,
                                     const char *db, const char *table) {
  Security_context *sctx = thd->security_context();
  LEX_CSTRING priv_user = sctx->priv_user();
  DBUG_TRACE;
  DBUG_PRINT("enter", ("Host: '%s', Ip: '%s', User: '%s', table: `%s`.`%s`",
                       sctx->priv_host().str,
                       (sctx->ip().length ? sctx->ip().str : "(NULL)"),
                       (priv_user.str ? priv_user.str : "(NULL)"), db, table));
  /*
    This function is not intended for derived tables which doesn't have a
    name. If this happens something is wrong.
  */
  /* --skip-grants */
  if (!initialized) {
    DBUG_PRINT("info", ("skip grants"));
    grant->privilege = ~NO_ACCESS;  // everything is allowed
    DBUG_PRINT("info", ("privilege 0x%lx", grant->privilege));
    return;
  }
  DBUG_PRINT("info", ("Effective table privileges are deduced from active roles"
                      " (%lu)",
                      (unsigned long)sctx->get_active_roles()->size()));
  std::string db_name = db ? db : "";
  if (sctx->get_active_roles()->size() > 0) {
    /* global privileges */
    grant->privilege = sctx->master_access(db_name);
    LEX_CSTRING str_db = {db, strlen(db)};
    /* db privileges */
    grant->privilege |= sctx->db_acl(str_db);
    LEX_CSTRING str_table = {table, strlen(table)};
    /* table privileges */
    grant->privilege |= sctx->table_acl(str_db, str_table);
    grant->grant_table = nullptr;
    DBUG_PRINT("info", ("Role used: %s db: %s db-acl: %lu all-acl: %lu ",
                        sctx->get_active_roles()->at(0).first.str, db,
                        sctx->db_acl(str_db), grant->privilege));
  } else {
    /* global privileges */
    grant->privilege = sctx->master_access(db_name);

    /* db privileges */
    grant->privilege |= acl_get(thd, sctx->host().str, sctx->ip().str,
                                priv_user.str, db, false);

    DEBUG_SYNC(thd, "fill_effective_table_privileges");
    /* table privileges */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
    if (!acl_cache_lock.lock(false)) return;

    if (grant->version != grant_version) {
      grant->grant_table =
          table_hash_search(sctx->host().str, sctx->ip().str, db, priv_user.str,
                            table, false); /* purecov: inspected */
      grant->version = grant_version;      /* purecov: inspected */
    }
    if (grant->grant_table != nullptr) {
      grant->privilege |= grant->grant_table->privs;
    }
  }

  // Allow SELECT privilege for INFORMATION_SCHEMA.
  if (is_infoschema_db(db)) grant->privilege |= SELECT_ACL;

  DBUG_PRINT("info", ("privilege 0x%lx", grant->privilege));
}

bool acl_check_proxy_grant_access(THD *thd, const char *host, const char *user,
                                  bool with_grant MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("user=%s host=%s with_grant=%d", user, host, (int)with_grant));
  DBUG_ASSERT(initialized);
  /* replication slave thread can do anything */
  if (thd->slave_thread) {
    DBUG_PRINT("info", ("replication slave"));
    return false;
  }

  /*
    one can grant proxy for self to others.
    Security context in THD contains two pairs of (user,host):
    1. (user,host) pair referring to inbound connection.
    2. (priv_user,priv_host) pair obtained from mysql.user table after doing
        authnetication of incoming connection.
    Privileges should be checked wrt (priv_user, priv_host) tuple, because
    (user,host) pair obtained from inbound connection may have different
    values than what is actually stored in mysql.user table and while granting
    or revoking proxy privilege, user is expected to provide entries mentioned
    in mysql.user table.
  */
  if (!strcmp(thd->security_context()->priv_user().str, user) &&
      !my_strcasecmp(system_charset_info, host,
                     thd->security_context()->priv_host().str)) {
    DBUG_PRINT("info", ("strcmp (%s, %s) my_casestrcmp (%s, %s) equal",
                        thd->security_context()->priv_user().str, user, host,
                        thd->security_context()->priv_host().str));
    return false;
  }
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  /* check for matching WITH PROXY rights */
  for (ACL_PROXY_USER *proxy = acl_proxy_users->begin();
       proxy != acl_proxy_users->end(); ++proxy) {
    DEBUG_SYNC(thd, "before_proxy_matches");
    if (proxy->matches(thd->security_context()->host().str,
                       thd->security_context()->user().str,
                       thd->security_context()->ip().str, user, false) &&
        proxy->get_with_grant()) {
      DBUG_PRINT("info", ("found"));
      return false;
    }
  }

  my_error(ER_ACCESS_DENIED_NO_PASSWORD_ERROR, MYF(0),
           thd->security_context()->user().str,
           thd->security_context()->host_or_ip().str);
  return true;
}

/**
  Grantee is of form 'user'@'hostname', so add +1 for '@' and +4 for the
  single qoutes. And +1 for null byte too.

  Note that we use USERNAME_LENGTH and not USERNAME_CHAR_LENGTH here
  because the username can be utf8.
*/
static const int GRANTEE_MAX_BUFF_LENGTH =
    USERNAME_LENGTH + 1 + HOSTNAME_LENGTH + 4 + 1;

int fill_schema_user_privileges(THD *thd, TABLE_LIST *tables, Item *) {
  int error = 0;
  ACL_USER *acl_user;
  ulong want_access;
  char buff[GRANTEE_MAX_BUFF_LENGTH];
  TABLE *table = tables->table;
  bool no_global_access = check_access(thd, SELECT_ACL, consts::mysql.c_str(),
                                       nullptr, nullptr, true, true);
  const char *curr_host = thd->security_context()->priv_host_name();
  DBUG_TRACE;

  if (!initialized) return 0;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return 1;

  for (acl_user = acl_users->begin(); acl_user != acl_users->end();
       ++acl_user) {
    const char *user, *host, *is_grantable = "YES";
    if (!(user = acl_user->user)) user = "";
    if (!(host = acl_user->host.get_host())) host = "";

    if (no_global_access &&
        (strcmp(thd->security_context()->priv_user().str, user) ||
         my_strcasecmp(system_charset_info, curr_host, host)))
      continue;

    want_access = acl_user->access;
    if (!(want_access & GRANT_ACL)) is_grantable = "NO";

    strxmov(buff, "'", user, "'@'", host, "'", NullS);
    if (!(want_access & ~GRANT_ACL)) {
      if (update_schema_privilege(thd, table, buff, nullptr, nullptr, nullptr,
                                  0, STRING_WITH_LEN("USAGE"), is_grantable)) {
        error = 1;
        goto err;
      }
    } else {
      uint priv_id;
      ulong j, test_access = want_access & ~GRANT_ACL;
      for (priv_id = 0, j = SELECT_ACL; j <= GLOBAL_ACLS; priv_id++, j <<= 1) {
        if (test_access & j) {
          if (update_schema_privilege(
                  thd, table, buff, nullptr, nullptr, nullptr, 0,
                  global_acls_vector[priv_id].c_str(),
                  global_acls_vector[priv_id].length(), is_grantable)) {
            error = 1;
            goto err;
          }
        }
      }
    }
    /* Process all global privileges */
    Role_id key(create_authid_from(acl_user));
    User_to_dynamic_privileges_map::iterator it, it_end;
    std::tie(it, it_end) = g_dynamic_privileges_map->equal_range(key);
    for (; it != it_end; ++it) {
      size_t str_len = it->second.first.length();
      if (it->second.second)
        is_grantable = "YES";
      else
        is_grantable = "NO";
      if (update_schema_privilege(thd, table, buff, nullptr, nullptr, nullptr,
                                  0, it->second.first.c_str(), str_len,
                                  is_grantable)) {
        error = 1;
        goto err;
      }
    }
  }  // end for each user

err:
  return error;
}

int fill_schema_schema_privileges(THD *thd, TABLE_LIST *tables, Item *) {
  int error = 0;
  ACL_DB *acl_db;
  ulong want_access;
  char buff[GRANTEE_MAX_BUFF_LENGTH];
  TABLE *table = tables->table;
  bool no_global_access = check_access(thd, SELECT_ACL, consts::mysql.c_str(),
                                       nullptr, nullptr, true, true);
  const char *curr_host = thd->security_context()->priv_host_name();
  DBUG_TRACE;

  if (!initialized) return 0;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return 1;

  for (acl_db = acl_dbs->begin(); acl_db != acl_dbs->end(); ++acl_db) {
    const char *user, *host, *is_grantable = "YES";

    if (!(user = acl_db->user)) user = "";
    if (!(host = acl_db->host.get_host())) host = "";

    if (no_global_access &&
        (strcmp(thd->security_context()->priv_user().str, user) ||
         my_strcasecmp(system_charset_info, curr_host, host)))
      continue;

    want_access = acl_db->access;
    if (want_access) {
      if (!(want_access & GRANT_ACL)) {
        is_grantable = "NO";
      }
      strxmov(buff, "'", user, "'@'", host, "'", NullS);
      if (!(want_access & ~GRANT_ACL)) {
        if (update_schema_privilege(thd, table, buff, acl_db->db, nullptr,
                                    nullptr, 0, STRING_WITH_LEN("USAGE"),
                                    is_grantable)) {
          error = 1;
          goto err;
        }
      } else {
        int cnt;
        ulong j, test_access = want_access & ~GRANT_ACL;
        for (cnt = 0, j = SELECT_ACL; j <= DB_ACLS; cnt++, j <<= 1)
          if (test_access & j) {
            if (update_schema_privilege(
                    thd, table, buff, acl_db->db, nullptr, nullptr, 0,
                    global_acls_vector[cnt].c_str(),
                    global_acls_vector[cnt].length(), is_grantable)) {
              error = 1;
              goto err;
            }
          }
      }
    }
  }
err:

  return error;
}

int fill_schema_table_privileges(THD *thd, TABLE_LIST *tables, Item *) {
  int error = 0;
  char buff[GRANTEE_MAX_BUFF_LENGTH];
  TABLE *table = tables->table;
  bool no_global_access = check_access(thd, SELECT_ACL, consts::mysql.c_str(),
                                       nullptr, nullptr, true, true);
  const char *curr_host = thd->security_context()->priv_host_name();
  DBUG_TRACE;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return 1;

  if (column_priv_hash == nullptr) return error;

  for (const auto &key_and_value : *column_priv_hash) {
    const char *user, *host, *is_grantable = "YES";
    GRANT_TABLE *grant_table = key_and_value.second.get();
    if (!(user = grant_table->user)) user = "";
    if (!(host = grant_table->host.get_host())) host = "";

    if (no_global_access &&
        (strcmp(thd->security_context()->priv_user().str, user) ||
         my_strcasecmp(system_charset_info, curr_host, host)))
      continue;

    ulong table_access = grant_table->privs;
    if (table_access) {
      ulong test_access = table_access & ~GRANT_ACL;
      /*
        We should skip 'usage' privilege on table if
        we have any privileges on column(s) of this table
      */
      if (!test_access && grant_table->cols) continue;
      if (!(table_access & GRANT_ACL)) is_grantable = "NO";

      strxmov(buff, "'", user, "'@'", host, "'", NullS);
      if (!test_access) {
        if (update_schema_privilege(thd, table, buff, grant_table->db,
                                    grant_table->tname, nullptr, 0,
                                    STRING_WITH_LEN("USAGE"), is_grantable)) {
          error = 1;
          goto err;
        }
      } else {
        ulong j;
        int cnt;
        for (cnt = 0, j = SELECT_ACL; j <= TABLE_ACLS; cnt++, j <<= 1) {
          if (test_access & j) {
            if (update_schema_privilege(
                    thd, table, buff, grant_table->db, grant_table->tname,
                    nullptr, 0, global_acls_vector[cnt].c_str(),
                    global_acls_vector[cnt].length(), is_grantable)) {
              error = 1;
              goto err;
            }
          }
        }
      }
    }
  }
err:

  return error;
}

int fill_schema_column_privileges(THD *thd, TABLE_LIST *tables, Item *) {
  int error = 0;
  char buff[GRANTEE_MAX_BUFF_LENGTH];
  TABLE *table = tables->table;
  bool no_global_access = check_access(thd, SELECT_ACL, consts::mysql.c_str(),
                                       nullptr, nullptr, true, true);
  const char *curr_host = thd->security_context()->priv_host_name();
  DBUG_TRACE;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return 1;

  if (column_priv_hash == nullptr) return error;

  for (const auto &key_and_value : *column_priv_hash) {
    const char *user, *host, *is_grantable = "YES";
    GRANT_TABLE *grant_table = key_and_value.second.get();
    if (!(user = grant_table->user)) user = "";
    if (!(host = grant_table->host.get_host())) host = "";

    if (no_global_access &&
        (strcmp(thd->security_context()->priv_user().str, user) ||
         my_strcasecmp(system_charset_info, curr_host, host)))
      continue;

    ulong table_access = grant_table->cols;
    if (table_access != 0) {
      if (!(grant_table->privs & GRANT_ACL)) is_grantable = "NO";

      ulong test_access = table_access & ~GRANT_ACL;
      strxmov(buff, "'", user, "'@'", host, "'", NullS);
      if (!test_access)
        continue;
      else {
        ulong j;
        int cnt;
        for (cnt = 0, j = SELECT_ACL; j <= TABLE_ACLS; cnt++, j <<= 1) {
          if (test_access & j) {
            for (const auto &key_and_value_gt : grant_table->hash_columns) {
              GRANT_COLUMN *grant_column = key_and_value_gt.second.get();
              if ((grant_column->rights & j) && (table_access & j)) {
                if (update_schema_privilege(
                        thd, table, buff, grant_table->db, grant_table->tname,
                        grant_column->column.data(),
                        grant_column->column.size(),
                        global_acls_vector[cnt].c_str(),
                        global_acls_vector[cnt].length(), is_grantable)) {
                  error = 1;
                  goto err;
                }
              }
            }
          }
        }
      }
    }
  }
err:

  return error;
}

bool is_privileged_user_for_credential_change(THD *thd) {
  if (thd->slave_thread) return true;
  return (!check_access(thd, UPDATE_ACL, consts::mysql.c_str(), nullptr,
                        nullptr, true, true) ||
          thd->security_context()->check_access(CREATE_USER_ACL,
                                                consts::mysql.c_str(), false));
}

/**
  Check if user has enough privileges for execution of SHOW statement,
  which was converted to query to one of I_S tables.

  @param thd    Thread context.
  @param table  Table list element for I_S table to be queried..

  @retval false - Success.
  @retval true  - Failure.
*/

bool check_show_access(THD *thd, TABLE_LIST *table) {
  // perform privilege checking for show statements on new dd tables
  switch (thd->lex->sql_command) {
    case SQLCOM_SHOW_DATABASES: {
      return (specialflag & SPECIAL_SKIP_SHOW_DB) &&
             check_global_access(thd, SHOW_DB_ACL);
    }
    case SQLCOM_SHOW_EVENTS: {
      const char *db = thd->lex->select_lex->db;
      DBUG_ASSERT(db != nullptr);
      /*
        Nobody has EVENT_ACL for I_S and P_S,
        even with a GRANT ALL to *.*,
        because these schemas have additional ACL restrictions:
        see ACL_internal_schema_registry.

        Yet there are no events in I_S and P_S to hide either,
        so this check voluntarily does not enforce ACL for
        SHOW EVENTS in I_S or P_S,
        to return an empty list instead of an access denied error.

        This is more user friendly, in particular for tools.

        EVENT_ACL is not fine grained enough to differentiate:
        - creating / updating / deleting events
        - viewing existing events
      */
      if (!is_infoschema_db(db) && !is_perfschema_db(db) &&
          check_access(thd, EVENT_ACL, db, nullptr, nullptr, false, false))
        return true;
    }
    // Fall through
    case SQLCOM_SHOW_TABLES:
    case SQLCOM_SHOW_TABLE_STATUS:
    case SQLCOM_SHOW_TRIGGERS: {
      const char *dst_db_name = thd->lex->select_lex->db;
      DBUG_ASSERT(dst_db_name != nullptr);
      if (!dst_db_name) break;

      // Check if the user has global access
      if (check_access(thd, SELECT_ACL, dst_db_name, &thd->col_access, nullptr,
                       false, false))
        return true;

      // Now check, if user has access to any of database/table/column/routine
      if (!(thd->col_access & DB_OP_ACLS) && check_grant_db(thd, dst_db_name)) {
        my_error(ER_DBACCESS_DENIED_ERROR, MYF(0),
                 thd->security_context()->priv_user().str,
                 thd->security_context()->priv_host().str, dst_db_name);
        return true;
      }
      return false;
    }
    case SQLCOM_SHOW_FIELDS:
    case SQLCOM_SHOW_KEYS: {
      TABLE_LIST *dst_table;
      dst_table = table->schema_select_lex->table_list.first;

      DBUG_ASSERT(dst_table);
      /*
        Open temporary tables to be able to detect them during privilege check.
      */
      if (open_temporary_tables(thd, dst_table)) return true;

      if (check_access(thd, SELECT_ACL, dst_table->db,
                       &dst_table->grant.privilege,
                       &dst_table->grant.m_internal, false, false))
        return true; /* Access denied */

      /*
        Check_grant will grant access if there is any column privileges on
        all of the tables thanks to the fourth parameter (bool show_table).
      */
      if (check_grant(thd, SELECT_ACL, dst_table, true, UINT_MAX, false))
        return true; /* Access denied */

      close_thread_tables(thd);
      dst_table->table = nullptr;

      /* Access granted */
      return false;
    }
    default:
      break;
  }

  return false;
}

/**
  check for global access and give descriptive error message if it fails.

  @param thd			Thread handler
  @param want_access		Use should have any of these global rights

  @warning
    One gets access right if one has ANY of the rights in want_access.
    This is useful as one in most cases only need one global right,
    but in some case we want to check if the user has SUPER or
    REPL_CLIENT_ACL rights.

  @retval
    0	ok
  @retval
    1	Access denied.  In this case an error is sent to the client
*/

bool check_global_access(THD *thd, ulong want_access) {
  DBUG_TRACE;
  char command[128];
  if (thd->security_context()->check_access(
          want_access, thd->db().str ? thd->db().str : "", true))
    return false;
  get_privilege_desc(command, sizeof(command), want_access);
  my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), command);
  return true;
}

/**
  Checks foreign key's parent table access.

  @param [in] thd               Thread handler
  @param [in] create_info       Create information (like MAX_ROWS, ENGINE or
                                temporary table flag)
  @param [in] alter_info        Initial list of columns and indexes for the
                                table to be created

  @retval
   false  ok.
  @retval
   true	  error or access denied. Error is sent to client in this case.
*/
bool check_fk_parent_table_access(THD *thd, HA_CREATE_INFO *create_info,
                                  Alter_info *alter_info) {
  DBUG_ASSERT(alter_info != nullptr);

  handlerton *db_type =
      create_info->db_type ? create_info->db_type : ha_default_handlerton(thd);

  // Return if engine does not support Foreign key Constraint.
  if (!ha_check_storage_engine_flag(db_type, HTON_SUPPORTS_FOREIGN_KEYS))
    return false;

  for (const Key_spec *key : alter_info->key_list) {
    if (key->type == KEYTYPE_FOREIGN) {
      const Foreign_key_spec *fk_key = down_cast<const Foreign_key_spec *>(key);

      TABLE_LIST parent_table(fk_key->ref_db.str, fk_key->ref_db.length,
                              fk_key->ref_table.str, fk_key->ref_table.length,
                              fk_key->ref_table.str, TL_IGNORE);

      /*
       Check if user has REFERENCES_ACL privilege at table level on
       "parent_table".
       Having privilege on any of the parent_table column is not
       enough so checking whether user has REFERENCES_ACL privilege
       at table level here.
      */
      if ((check_access(thd, REFERENCES_ACL, parent_table.db,
                        &parent_table.grant.privilege,
                        &parent_table.grant.m_internal, false, true) ||
           check_grant(thd, REFERENCES_ACL, &parent_table, false, 1, true)) ||
          (parent_table.grant.privilege & REFERENCES_ACL) == 0) {
        char fqtn_buff[NAME_LEN + 1 + NAME_LEN + 1];
        snprintf(fqtn_buff, sizeof(fqtn_buff), "%s.%s", fk_key->ref_db.str,
                 fk_key->ref_table.str);
        my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), "REFERENCES",
                 thd->security_context()->priv_user().str,
                 thd->security_context()->host_or_ip().str, fqtn_buff);

        return true;
      }
    }
  }

  return false;
}

/**
  Examines if a user\@host authid is connected to a role\@role_host authid by
  comparing all out-edges if the user\@host vertex in the global role graph.

  @retval true the two vertices are connected (role is granted)
  @retval false not connected (role is not granted)
*/
bool check_if_granted_role(LEX_CSTRING user, LEX_CSTRING host, LEX_CSTRING role,
                           LEX_CSTRING role_host) {
  String key;
  append_identifier(&key, user.str, user.length);
  key.append('@');
  append_identifier(&key, host.str, host.length);
  Role_index_map::iterator it =
      g_authid_to_vertex->find(std::string(key.c_ptr_quick()));
  if (it != g_authid_to_vertex->end()) {
    /* Check if role is part of current role graph */
    if (find_if_granted_role(it->second, role, role_host)) return true;
  }

  /*
     No grated role match the requested role for the current user;
     Check if a mandatory role is granted instead.
   */
  std::vector<Role_id> mandatory_roles;
  get_mandatory_roles(&mandatory_roles);
  for (auto &&rid : mandatory_roles) {
    if (rid == Role_id(role, role_host)) return true;
  }
  return false;
}

/**
  Given a vertex in the roles graph, this function finds a directly connected
  vertex given a (role, role_host) tuple. The resulting vertex is returned to
  the caller through an out-param.

  @param v Vertex descriptor of the authid which might have a granted role
  @param role User name part of an authid
  @param role_host Host name part of an authid
  @param [out] found_vertex The corresponding vertex of the granted role.

  @return Success state
   @retval true The role is granted and the corresponding vertex is returned.
   @retval false No such role is granted.
*/
bool find_if_granted_role(Role_vertex_descriptor v, LEX_CSTRING role,
                          LEX_CSTRING role_host,
                          Role_vertex_descriptor *found_vertex) {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  boost::graph_traits<Granted_roles_graph>::out_edge_iterator ei, ei_end;
  boost::tie(ei, ei_end) = boost::out_edges(v, *g_granted_roles);
  /* Iterate all neighboring vertices */
  for (; ei != ei_end; ++ei) {
    /* find current user in role graph */
    ACL_USER acl_user =
        get(boost::vertex_acl_user_t(),
            *g_granted_roles)[boost::target(*ei, *g_granted_roles)];
    if ((role.length == strlen(acl_user.user)) &&
        (role_host.length == acl_user.host.get_host_len()) &&
        !strncmp(role.str, acl_user.user, role.length) &&
        (role_host.length == 0 ||
         !strncmp(role_host.str, acl_user.host.get_host(), role_host.length))) {
      /* Found a vertex matching the active role */
      if (found_vertex != nullptr)
        *found_vertex = boost::target(*ei, *g_granted_roles);
      return true;
    }
  }
  return false;
}

void get_granted_roles(Role_vertex_descriptor &v,
                       std::function<void(const Role_id &, bool)> f) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  boost::graph_traits<Granted_roles_graph>::out_edge_iterator ei, ei_end;
  boost::tie(ei, ei_end) = boost::out_edges(v, *g_granted_roles);
  /* Iterate all neighboring vertices */
  for (; ei != ei_end; ++ei) {
    /* find current user in role graph */
    ACL_USER acl_user =
        get(boost::vertex_acl_user_t(),
            *g_granted_roles)[boost::target(*ei, *g_granted_roles)];
    auto edge_with_admin =
        boost::get(boost::edge_capacity_t(), *g_granted_roles);
    int with_admin_opt = edge_with_admin[*ei];
    LEX_CSTRING tmp_user, tmp_host;
    tmp_user.str = acl_user.user;
    tmp_user.length = strlen(acl_user.user);
    tmp_host.str = acl_user.host.get_host();
    tmp_host.length = acl_user.host.get_host_len();
    Role_id id(tmp_user, tmp_host);
    f(id, with_admin_opt != 0);
  }
}

/**
  Populates a list of authorization IDs that are connected to a specified
  graph vertex in the global roles graph.

  The constructed list contains references to a shared memory. The authIDs
  are not copied!

  The list of granted roles is /appended/ to the out variable.

  @param v A valid vertex descriptor from the global roles graph
  @param [out] granted_roles A list of authorization IDs
*/
void get_granted_roles(Role_vertex_descriptor &v,
                       List_of_granted_roles *granted_roles) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  get_granted_roles(v, [&](const Role_id &rid, bool with_admin_opt) {
    granted_roles->push_back(std::make_pair(rid, with_admin_opt));
  });
}

/**
  Activates all roles granted to the auth_id.

  @param  [in]  acl_user ACL_USER for which all granted roles to be activated.
  @param  [in]  sctx     Push the activated role to secruity context
*/
void activate_all_granted_roles(const ACL_USER *acl_user,
                                Security_context *sctx) {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  std::string key = create_authid_str_from(acl_user);
  Role_index_map::iterator it = g_authid_to_vertex->find(key);
  if (it == g_authid_to_vertex->end()) return;  // No user vertex founds
  get_granted_roles(it->second, [&](const Role_id rid, bool) {
    LEX_CSTRING str_user = {rid.user().c_str(), rid.user().length()};
    LEX_CSTRING str_host = {rid.host().c_str(), rid.host().length()};
    sctx->activate_role(str_user, str_host, false);
  });
}

/**
  Activates all the mandatory roles for the current user

  @param  [in]  sctx     Push the activated role to secruity context
*/
void activate_all_mandatory_roles(Security_context *sctx) {
  DBUG_ASSERT(assert_acl_cache_read_lock(current_thd));
  std::vector<Role_id> mandatory_roles;
  get_mandatory_roles(&mandatory_roles);
  for (auto &rid : mandatory_roles) {
    LEX_CSTRING str_user = {rid.user().c_str(), rid.user().length()};
    LEX_CSTRING str_host = {rid.host().c_str(), rid.host().length()};
    sctx->activate_role(str_user, str_host, false);
  }
}

void activate_all_granted_and_mandatory_roles(const ACL_USER *acl_user,
                                              Security_context *sctx) {
  activate_all_granted_roles(acl_user, sctx);
  activate_all_mandatory_roles(sctx);
}

/**
  This is a convenience function.
  @see get_granted_roles(Role_vertex_descriptor &v,
                         List_of_granted_roles *granted_roles)
  @param user The authid to check for granted roles
  @param [out] granted_roles A list of granted authids
*/

void get_granted_roles(LEX_USER *user, List_of_granted_roles *granted_roles) {
  Role_index_map::iterator it;
  std::string str_user = create_authid_str_from(user);
  if ((it = g_authid_to_vertex->find(str_user)) != g_authid_to_vertex->end()) {
    get_granted_roles(it->second, granted_roles);
  }
}

/**
  Helper function for func_current_role used for Item_func_current_role.
  @param thd The thread handler
  @param roles [out] A list of Role_id granted to the current user.
*/
void get_active_roles(const THD *thd, List_of_granted_roles *roles) {
  /*
    We need the order of the current roles to stay consistent across platforms
    so we copy the list of active roles here and sort the list.
    Copying is crucial as the std::sort algorithms operates on pointers and
    not on values which cause all references to become invalid.
  */
  for (auto &ref : *thd->security_context()->get_active_roles()) {
    roles->push_back(std::make_pair(Role_id(ref.first, ref.second), false));
  }
}

/**
  Helper function for Item_func_current_role.
  @param thd Thread handler
  @param active_role [out] Comma separated list of auth ids
 */

void func_current_role(const THD *thd, String *active_role) {
  List_of_granted_roles roles;
  get_active_roles(thd, &roles);
  if (roles.size() == 0) {
    active_role->set_ascii("NONE", 4);
    return;
  }
  std::sort(roles.begin(), roles.end());
  bool first = true;
  for (auto &rid : roles) {
    if (!first) {
      active_role->append(',');
    } else {
      first = false;
    }
    append_identifier(thd, active_role, rid.first.user().c_str(),
                      rid.first.user().length());
    active_role->append("@");
    append_identifier(thd, active_role, rid.first.host().c_str(),
                      rid.first.host().length());
  }
  return;
}

/**
  Shallow copy a list of default role authorization IDs from an Role_id storage

  @param acl_user A valid authID for which we want the default roles.
  @param [out] authlist The target list to be populated. The target list is set
                        to empty if no default role is found.
*/
void get_default_roles(const Auth_id_ref &acl_user,
                       List_of_auth_id_refs &authlist) {
  if (g_default_roles == nullptr) return;

  authlist.clear();  // Remove all items

  Role_id user(acl_user);
  Default_roles::iterator role_it, role_end;
  boost::tie(role_it, role_end) = g_default_roles->equal_range(user);
  for (; role_it != role_end; ++role_it) {
    Auth_id_ref ref = create_authid_from(role_it->second);
    authlist.push_back(ref);
  }
}

/**
  Copy a list of mandatory role authorization IDs.

  @param [out] mandatory_roles Pointer to the target list to be populated.
                               The target list is set to empty if no
                               mandatory role is found.
*/
bool lock_and_get_mandatory_roles(std::vector<Role_id> *mandatory_roles) {
  DBUG_EXECUTE_IF("simulate_acl_cache_lock_failure", { return true; });
  Acl_cache_lock_guard acl_cache_lock(current_thd,
                                      Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return true;

  // Retrieve mandatory roles
  get_mandatory_roles(mandatory_roles);
  return false;
}

/**
  Removes all default role policies assigned to user. If the user is used as a
  default role policy, this policy needs to be removed too.
  Removed policies are copied to the vector supplied in the arguments.

  @param thd Thread handler
  @param table Open table handler
  @param user_auth_id A reference to the authorization ID to clear
  @param [out] default_roles The vector to which the removed roles are copied.

  @retval true An error occurred.
  @retval false Success
*/
bool clear_default_roles(THD *thd, TABLE *table,
                         const Auth_id_ref &user_auth_id,
                         std::vector<Role_id> *default_roles) {
  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  Default_roles::iterator role_it, role_end, begin_it;
  Role_id user_role_id(user_auth_id);
  boost::tie(begin_it, role_end) = g_default_roles->equal_range(user_role_id);
  role_it = begin_it;
  bool error = false;
  for (; role_it != role_end && !error; ++role_it) {
    if (default_roles != nullptr) {
      default_roles->push_back(role_it->second);
    }
    Auth_id_ref role_auth_id = create_authid_from(role_it->second);
    error = modify_default_roles_in_table(thd, table, user_auth_id,
                                          role_auth_id, true);
  }
  g_default_roles->erase(begin_it, role_end);

  return error;
}

/**
  Drop a specific default role policy given the role- and user names.

  @param thd Thread handler
  @param table An open table handler to the default_roles table
  @param default_role_policy The role name
  @param user The user name

  @retval Error state
    @retval true An error occurred
    @retval false Success
*/

bool drop_default_role_policy(THD *thd, TABLE *table,
                              const Auth_id_ref &default_role_policy,
                              const Auth_id_ref &user) {
  Role_id id(user);
  auto range = g_default_roles->equal_range(id);
  for (; range.first != range.second; ++range.first) {
    if (range.first->second == default_role_policy) {
      g_default_roles->erase(range.first);
      return modify_default_roles_in_table(thd, table, user,
                                           default_role_policy, true);
    }
  }
  return false;
}

/**
  Set the default roles to NONE, ALL or list of authorization IDs as
  roles, depending upon the role_type argument. It writes to table
  mysql.default_roles and binlog.

  @param thd        Thread handler
  @param role_type  default role type specified by the user.
  @param users      Users for whom the default roles are set.
  @param roles      list of default roles to be set.

  @retval true An error occurred and DA is set
  @retval false Successful
*/
bool mysql_alter_or_clear_default_roles(THD *thd, role_enum role_type,
                                        const List<LEX_USER> *users,
                                        const List<LEX_USER> *roles) {
  DBUG_TRACE;

  List<LEX_USER> *tmp_users = const_cast<List<LEX_USER> *>(users);
  List<LEX_USER> *tmp_roles = const_cast<List<LEX_USER> *>(roles);
  List_iterator<LEX_USER> users_it(*tmp_users);
  List_iterator<LEX_USER> roles_it;
  List_of_auth_id_refs authids;
  Auth_id_ref authid;
  LEX_USER *user = nullptr;
  LEX_USER *role = nullptr;
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  int result = 0;
  bool transactional_tables = false;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication. The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  if ((result = open_grant_tables(thd, tables, &transactional_tables)))
    return result != 1;

  TABLE *table = tables[ACL_TABLES::TABLE_DEFAULT_ROLES].table;

  if (!table) {
    my_error(ER_OPEN_ROLE_TABLES, MYF(MY_WME));
    return true;
  }

  bool ret = false;
  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, *users)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    while ((user = users_it++) && !ret) {
      // Check for CURRENT_USER token
      user = get_current_user(thd, user);
      if (strcmp(thd->security_context()->priv_user().str, user->user.str) !=
              0 ||
          strcmp(thd->security_context()->priv_host().str, user->host.str) !=
              0) {
        if (check_access(thd, UPDATE_ACL, consts::mysql.c_str(), nullptr,
                         nullptr, true, true) &&
            check_global_access(thd, CREATE_USER_ACL)) {
          my_error(ER_ACCESS_DENIED_ERROR, MYF(0), user->user.str,
                   user->host.str,
                   (thd->password ? ER_THD(thd, ER_YES) : ER_THD(thd, ER_NO)));
          return true;
        }
        if (roles != nullptr) {
          roles_it = *tmp_roles;
          while ((role = roles_it++)) {
            if (!is_granted_role(user->user, user->host, role->user,
                                 role->host)) {
              my_error(ER_ROLE_NOT_GRANTED, MYF(0), role->user.str,
                       role->host.str, user->user.str, user->host.str);
              return true;
            }
            authid = std::make_pair(role->user, role->host);
            authids.push_back(authid);
          }
        }
      } else {
        // Verify that the user actually is granted the role before it is
        // set as default.
        if (roles != nullptr) {
          roles_it = *tmp_roles;
          while ((role = roles_it++)) {
            if (!is_granted_role(thd->security_context()->priv_user(),
                                 thd->security_context()->priv_host(),
                                 role->user, role->host)) {
              my_error(ER_ROLE_NOT_GRANTED, MYF(0), role->user.str,
                       role->host.str, thd->security_context()->priv_user().str,
                       thd->security_context()->priv_host().str);
              return true;
            }
            authid = std::make_pair(role->user, role->host);
            authids.push_back(authid);
          }
        }
      }

      if (role_type == role_enum::ROLE_NONE) {
        authid = create_authid_from(user);
        ret = clear_default_roles(thd, table, authid, nullptr);
      } else if (role_type == role_enum::ROLE_ALL) {
        ret = alter_user_set_default_roles_all(thd, table, user);
      } else if (role_type == role_enum::ROLE_NAME) {
        ret = alter_user_set_default_roles(thd, table, user, authids);
      }

      if (ret) {
        my_error(ER_FAILED_DEFAULT_ROLES, MYF(0));
      }
    }

    ret = log_and_commit_acl_ddl(thd, transactional_tables, nullptr, nullptr,
                                 ret);
    get_global_acl_cache()->increase_version();
  } /* Critical section */

  /* Notify storage engines */
  if (!ret) {
    acl_notify_htons(thd, SQLCOM_ALTER_USER, users);
  }

  return ret;
}

/**
  Set all granted role as default roles. Writes to table mysql.default_roles
  and binlog.

  @param thd Thread handler
  @param def_role_table Default role table
  @param user The user whose default roles are set.

  @retval true An error occurred and DA is set
  @retval false Successful
*/

bool alter_user_set_default_roles_all(THD *thd, TABLE *def_role_table,
                                      LEX_USER *user) {
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  std::string authid_role = create_authid_str_from(user);
  Role_index_map::iterator it = g_authid_to_vertex->find(authid_role);
  if (it == g_authid_to_vertex->end()) {
    /* No such user */
    my_error(ER_UNKNOWN_AUTHID, MYF(MY_WME), user->user.str, user->host.str);
    return true;
  }
  List_of_granted_roles granted_roles;
  get_granted_roles(it->second, &granted_roles);
  List_of_auth_id_refs new_default_role_ref;
  for (auto &&role : granted_roles) {
    Auth_id_ref authid = create_authid_from(role.first);
    new_default_role_ref.push_back(authid);
  }
  std::vector<Role_id> mandatory_roles;
  get_mandatory_roles(&mandatory_roles);
  for (auto &role : mandatory_roles) {
    Auth_id_ref authid = create_authid_from(role);
    auto res = std::find(new_default_role_ref.begin(),
                         new_default_role_ref.end(), authid);
    if (res == new_default_role_ref.end()) {
      new_default_role_ref.push_back(authid);
    }
  }
  bool errors = alter_user_set_default_roles(thd, def_role_table, user,
                                             new_default_role_ref);

  return errors;
}

/**
  Set the default roles for a particular user.

  @param thd           Thread handle
  @param table         Table handle to an open table
  @param user          AST component for the user for which we set def roles
  @param new_auth_ids  Default roles to set

  @retval true  Operation failed
  @retval false Operation was successful.
*/

bool alter_user_set_default_roles(THD *thd, TABLE *table, LEX_USER *user,
                                  const List_of_auth_id_refs &new_auth_ids) {
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  bool errors = false;

  ACL_USER *acl_user = find_acl_user(user->host.str, user->user.str, true);
  if (acl_user == nullptr) return true;

  if (new_auth_ids.size() != 0) {
    Default_roles::iterator role_it, role_end;
    Auth_id_ref user_auth_id = create_authid_from(user);
    Role_id user_role_id(user_auth_id);
    boost::tie(role_it, role_end) = g_default_roles->equal_range(user_role_id);
    for (; role_it != role_end && !errors; ++role_it) {
      if (std::find(new_auth_ids.begin(), new_auth_ids.end(),
                    role_it->second) == new_auth_ids.end()) {
        Auth_id_ref role_auth_id = create_authid_from(role_it->second);
        errors = modify_default_roles_in_table(thd, table, user_auth_id,
                                               role_auth_id, true);
      }
    }
    List_of_auth_id_refs::const_iterator it = new_auth_ids.begin();
    boost::tie(role_it, role_end) = g_default_roles->equal_range(user_role_id);
    for (; it != new_auth_ids.end() && !errors; ++it) {
      if (find(role_it, role_end, *it) == role_end) {
        errors =
            modify_default_roles_in_table(thd, table, user_auth_id, *it, false);
      }
    }
    boost::tie(role_it, role_end) = g_default_roles->equal_range(user_role_id);
    g_default_roles->erase(role_it, role_end);
    it = new_auth_ids.begin();
    for (; it != new_auth_ids.end() && !errors; ++it) {
      Role_id role_role_id(*it);
      g_default_roles->insert(std::make_pair(user_role_id, role_role_id));
    }
  }

  return errors;
}

/**
  Helper used for producing a key to a key-value-map
*/
std::string create_authid_str_from(const LEX_USER *user) {
  String tmp;
  append_identifier(&tmp, user->user.str, user->user.length);
  tmp.append('@');
  append_identifier(&tmp, user->host.str, user->host.length);
  return std::string(tmp.c_ptr_quick());
}

Auth_id_ref create_authid_from(const LEX_USER *user) {
  Auth_id_ref id;
  id = std::make_pair(user->user, user->host);
  return id;
}

Auth_id_ref create_authid_from(const Role_id &user) {
  Auth_id_ref id;
  LEX_CSTRING lex_user;
  lex_user.str = user.user().c_str();
  lex_user.length = user.user().length();
  LEX_CSTRING lex_host;
  lex_host.str = user.host().c_str();
  lex_host.length = user.host().length();
  id = std::make_pair(lex_user, lex_host);
  return id;
}

Auth_id_ref create_authid_from(const LEX_CSTRING &user,
                               const LEX_CSTRING &host) {
  return std::make_pair(user, host);
}

/**
  Helper used for producing a key to a key-value-map
*/
std::string create_authid_str_from(const ACL_USER *user) {
  String tmp;
  size_t length = user->user == nullptr ? 0 : strlen(user->user);
  append_identifier(&tmp, user->user, length);
  tmp.append("@");
  append_identifier(&tmp, user->host.get_host(), user->host.get_host_len());
  return std::string(tmp.c_ptr_quick());
}

std::string create_authid_str_from(const Auth_id_ref &user) {
  String tmp;
  append_identifier(&tmp, user.first.str, user.first.length);
  tmp.append("@");
  append_identifier(&tmp, user.second.str, user.second.length);
  return std::string(tmp.c_ptr_quick());
}

Auth_id_ref create_authid_from(const ACL_USER *user) {
  Auth_id_ref id;
  LEX_CSTRING username;
  LEX_CSTRING host;
  username.str = user->user;
  if (user->user != nullptr)
    username.length = strlen(user->user);
  else
    username.length = 0;
  host.str = user->host.get_host();
  host.length = user->host.get_host_len();
  id = std::make_pair(username, host);
  return id;
}

/**
  Reset active roles

  @param [in] thd THD handle

  @returns status of resetting active roles
    @retval false Success
    @retval true  Error
*/
bool mysql_set_active_role_none(THD *thd) {
  DBUG_TRACE;
  bool ret = false;
  Roles::Role_activation role_activation(thd, thd->security_context(),
                                         role_enum::ROLE_NONE, nullptr, false);
  ret = role_activation.activate();
  if (!ret) my_ok(thd);
  return ret;
}

/**
   Activates all the default roles in the current security context

   This function acquires the Acl_cache_lock_guard in read lock.

   @param thd A valid THD handle

   @return Error code
     @retval 0 Success; the specified role was activated.
     @retval != 0 Failure. DA is set.
*/
bool mysql_set_role_default(THD *thd) {
  DBUG_TRACE;
  bool ret = false;
  Roles::Role_activation role_activation(thd, thd->security_context(),
                                         role_enum::ROLE_DEFAULT, nullptr);
  ret = role_activation.activate();
  if (!ret) my_ok(thd);
  return ret;
}

/**
   Activates all granted role in the current security context

   This function acquires the acl_user->lock mutex.

   @param thd A valid THD handle
   @param except_users A pointer to a list of LEX_USER objects which represent
 roles that shouldn't be activated.

   @return Error code
     @retval 0 Success; the specified role was activated.
     @retval != 0 Failure. DA is set.
*/
bool mysql_set_active_role_all(THD *thd, const List<LEX_USER> *except_users) {
  DBUG_TRACE;
  bool ret = false;
  Roles::Role_activation role_activation(thd, thd->security_context(),
                                         role_enum::ROLE_ALL, except_users);
  ret = role_activation.activate();
  if (!ret) my_ok(thd);
  return ret;
}

bool mysql_set_active_role(THD *thd, const List<LEX_USER> *role_list) {
  DBUG_TRACE;
  bool ret = false;
  Roles::Role_activation role_activation(thd, thd->security_context(),
                                         role_enum::ROLE_NAME, role_list);
  ret = role_activation.activate();
  if (!ret) my_ok(thd);
  return ret;
}

/**
  This function works just like check_if_granted_role, but also guarantees that
  the proper lock is taken so that the function can be used in a wider context.
  @param user The user name part of a authid which should be tested
  @param host The host name part of a authid which should be tested
  @param role The role name part of the role authid
  @param role_host The host name part of the role authid

  @return success value
    @retval true The value user\@host was previously granted role\@role_host
    @retval false role\@role_host is not granted to user\@host
*/

bool is_granted_role(LEX_CSTRING user, LEX_CSTRING host, LEX_CSTRING role,
                     LEX_CSTRING role_host) {
  bool ret = false;
  Acl_cache_lock_guard acl_cache_lock(current_thd,
                                      Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(false)) return false;

  ret = check_if_granted_role(user, host, role, role_host);
  return ret;
}

/**
  Determine if a role\@role_host authid is a mandatory role.

  @param role                Role name.
  @param role_host           Host name of role.
  @param[out] is_mandatory   Pointer to boolean hold status of check.

  @retval true if failed to determine. e.g., ACL lock acquire failed.
  @retval false otherwise.
*/
bool is_mandatory_role(LEX_CSTRING role, LEX_CSTRING role_host,
                       bool *is_mandatory) {
  // Fetch all mandatory role.
  std::vector<Role_id> mandatory_roles;
  if (lock_and_get_mandatory_roles(&mandatory_roles)) return true;

  // Check if role is in mandatory role list.
  *is_mandatory = false;
  for (auto &&rid : mandatory_roles) {
    if (rid == Role_id(role, role_host)) {
      *is_mandatory = true;
      break;
    }
  }
  return false;
}

/**
  Grant one privilege to one user
  @param str_priv
  @param str_user
  @param str_host
  @param with_grant_option
  @param update_table

  @return Error state
    @retval true An error occurred. DA must be checked.
    @retval false Success

*/
bool grant_dynamic_privilege(const LEX_CSTRING &str_priv,
                             const LEX_CSTRING &str_user,
                             const LEX_CSTRING &str_host,
                             bool with_grant_option,
                             Update_dynamic_privilege_table &update_table) {
  try {
    const std::string priv(str_priv.str, str_priv.length);
    if (!is_dynamic_privilege_registered(priv)) return true;
    const Role_id id(str_user, str_host);
    /*
      Is this grant already present? If so we will make an update by removing
      the previous grant only if the grant_option property has changed.
    */
    auto range = g_dynamic_privileges_map->equal_range(id);
    for (auto it = range.first; it != range.second; ++it) {
      if (it->second.first == priv) {
        /*
          WITH GRANT OPTION is cumulative which means that if a previous GRANT
          exists we only update it if we're adding GRANT OPTION to it.
          We never remove the GRANT OPTION as a result of GRANT statement.
        */
        if (with_grant_option == true &&
            it->second.second != with_grant_option) {
          if (update_table(priv, {str_user, str_host}, false,
                           Update_dynamic_privilege_table::REVOKE))
            return true;
          g_dynamic_privileges_map->erase(it);
          break;
        }
        /* If the entry already exist we're done */
        return false;
      }
    }

    if (update_table(priv, {str_user, str_host}, with_grant_option,
                     Update_dynamic_privilege_table::GRANT))
      return true;
    g_dynamic_privileges_map->insert(
        std::make_pair(id, std::make_pair(priv, with_grant_option)));
  } catch (...) {
    return true;
  }
  return false;
}

/**
  Grant grant option to one user for all dynamic privileges
  @param str_user
  @param str_host
  @param update_table

  @return Error state
    @retval true An error occurred. DA must be checked.
    @retval false Success

*/
bool grant_grant_option_for_all_dynamic_privileges(
    const LEX_CSTRING &str_user, const LEX_CSTRING &str_host,
    Update_dynamic_privilege_table &update_table) {
  try {
    Role_id id(str_user, str_host);
    /*
      For all dynamic privileges associated for a particular user
      grant with grant option.
    */
    auto range = g_dynamic_privileges_map->equal_range(id);
    std::vector<std::string> priv_list;
    for (auto it = range.first; it != range.second; ++it) {
      std::string priv(it->second.first);
      if (it->second.second != true) {
        /*
          if with grant option is not set reovke this privilege and
          later update the privilege along with "WITH GRANT OPTION".
        */
        update_table(priv, {str_user, str_host}, false,
                     Update_dynamic_privilege_table::REVOKE);
      } else
        continue;

      if (update_table(priv, {str_user, str_host}, true,
                       Update_dynamic_privilege_table::GRANT))
        return true;
      /* keep track of privileges, later used to update cache */
      priv_list.push_back(priv);
    }
    for (auto it = priv_list.begin(); it != priv_list.end(); ++it) {
      g_dynamic_privileges_map->insert(
          std::make_pair(id, std::make_pair(*it, true)));
    }
  } catch (...) {
    return true;
  }
  return false;
}

/**
  Revoke grant option to one user for all dynamic privileges
  @param str_user
  @param str_host
  @param update_table

  @return Error state
    @retval true An error occurred. DA must be checked.
    @retval false Success

*/
bool revoke_grant_option_for_all_dynamic_privileges(
    const LEX_CSTRING &str_user, const LEX_CSTRING &str_host,
    Update_dynamic_privilege_table &update_table) {
  try {
    Role_id id(str_user, str_host);
    /*
      For all dynamic privileges associated for a particular user
      revoke with grant option.
    */
    auto range = g_dynamic_privileges_map->equal_range(id);
    std::vector<std::string> priv_list;
    for (auto it = range.first; it != range.second; ++it) {
      std::string priv(it->second.first);
      if (it->second.second == true) {
        if (update_table(priv, {str_user, str_host}, true,
                         Update_dynamic_privilege_table::REVOKE))
          return true;
      } else
        continue;

      if (update_table(priv, {str_user, str_host}, false,
                       Update_dynamic_privilege_table::GRANT))
        return true;
      /* keep track of privileges, later used to update cache */
      priv_list.push_back(priv);
    }
    for (auto it = priv_list.begin(); it != priv_list.end(); ++it) {
      g_dynamic_privileges_map->insert(
          std::make_pair(id, std::make_pair(*it, false)));
    }
  } catch (...) {
    return true;
  }
  return false;
}

/**
  Grant needed dynamic privielges to in memory internal auth id.

  @param id            auth id to which privileges needs to be granted
  @param priv_list     List of privileges to be added to internal auth id

  @retval True    In case privilege is not registered
  @retval False   Success
*/
bool grant_dynamic_privileges_to_auth_id(
    const Role_id &id, const std::vector<std::string> &priv_list) {
  DBUG_TRACE;
  Update_dynamic_privilege_table update_table;

  /* --skip-grants */
  if (!initialized) return false;
  for (auto it : priv_list) {
    LEX_CSTRING priv = {it.c_str(), it.length()};
    LEX_CSTRING user = {id.user().c_str(), id.user().length()};
    LEX_CSTRING host = {id.host().c_str(), id.host().length()};
    if (grant_dynamic_privilege(priv, user, host, false, update_table))
      return true;
  }
  return false;
}

/**
  Revoke dynamic privielges from in memory internal auth id.

  @param id            auth id from which privileges needs to be revoked
  @param priv_list     List of privileges to be removed for internal auth id
*/
void revoke_dynamic_privileges_from_auth_id(
    const Role_id &id, const std::vector<std::string> &priv_list) {
  DBUG_TRACE;
  if (!initialized) return;
  Update_dynamic_privilege_table update_table;
  for (auto priv_it : priv_list) {
    LEX_CSTRING user = {id.user().c_str(), id.user().length()};
    LEX_CSTRING host = {id.host().c_str(), id.host().length()};
    LEX_CSTRING priv = {priv_it.c_str(), priv_it.length()};
    revoke_dynamic_privilege(priv, user, host, update_table);
  }
}

/**
  Revoke one privilege from one user
  @param str_priv
  @param str_user
  @param str_host
  @param update_table

  @return Error state
    @retval true An error occurred. DA must be checked.
    @retval false Success
*/

bool revoke_dynamic_privilege(const LEX_CSTRING &str_priv,
                              const LEX_CSTRING &str_user,
                              const LEX_CSTRING &str_host,
                              Update_dynamic_privilege_table &update_table) {
  try {
    const std::string priv(str_priv.str, str_priv.length);
    const Role_id id(str_user, str_host);
    if (is_dynamic_privilege_registered(priv)) {
      auto range = g_dynamic_privileges_map->equal_range(id);
      for (auto it = range.first; it != range.second; ++it) {
        if (it->second.first == priv) {
          if (update_table(priv, {str_user, str_host}, false,
                           Update_dynamic_privilege_table::REVOKE))
            return true;
          g_dynamic_privileges_map->erase(it);
          break;
        }
      }
    } else {
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING,
          ER_WARN_DA_PRIVILEGE_NOT_REGISTERED,
          ER_THD(current_thd, ER_WARN_DA_PRIVILEGE_NOT_REGISTERED),
          str_priv.str);
    }
  } catch (...) {
    return true;
  }
  return false;
}

/**
  Revoke all dynamic global privileges.
  @param user The target user name
  @param host The target host name
  @param update_table Functor for updating a table

  @return Error state
    @retval true An error occurred. DA might not be set.
    @retval false Success
*/
bool revoke_all_dynamic_privileges(
    const LEX_CSTRING &user, const LEX_CSTRING &host,
    Update_dynamic_privilege_table &update_table) {
  try {
    if (g_dynamic_privileges_map->size() > 0) {
      Role_id id(user, host);
      auto range = g_dynamic_privileges_map->equal_range(id);
      for (auto it = range.first; it != range.second; ++it) {
        if (update_table(it->second.first, {user, host}, false,
                         Update_dynamic_privilege_table::REVOKE)) {
          return true;
        }
      }
      g_dynamic_privileges_map->erase(range.first, range.second);
    }
  } catch (...) {
    return true;
  }
  return false;
}

bool rename_dynamic_grant(const LEX_CSTRING &old_user,
                          const LEX_CSTRING &old_host,
                          const LEX_CSTRING &new_user,
                          const LEX_CSTRING &new_host,
                          Update_dynamic_privilege_table &update_table) {
  try {
    if (g_dynamic_privileges_map->size() > 0) {
      /*
        Revoke all privileges using the old authorization identifier but don't
        update the cache.
      */
      Role_id id(old_user, old_host);
      auto range = g_dynamic_privileges_map->equal_range(id);
      std::vector<Grant_privilege> privileges(
          std::distance(range.first, range.second));
      int grants_count = 0;
      for (auto it = range.first; it != range.second; ++it, ++grants_count) {
        privileges[grants_count] =
            std::make_pair(it->second.first, it->second.second);
      }
      for (auto it = range.first; it != range.second; ++it) {
        if (update_table(it->second.first, {old_user, old_host}, false,
                         Update_dynamic_privilege_table::REVOKE)) {
          return true;
        }
      }
      /*
        Remove the old entries from the cache
      */
      g_dynamic_privileges_map->erase(range.first, range.second);

      /*
        Grant the new authorization id the same privileges as the old and update
        the cache with the new entries.
      */
      while (grants_count > 0) {
        --grants_count;
        LEX_CSTRING priv = {privileges[grants_count].first.c_str(),
                            privileges[grants_count].first.length()};
        if (grant_dynamic_privilege(priv, new_user, new_host,
                                    privileges[grants_count].second,
                                    update_table)) {
          return true;
        }
      }
    }  // end for
  } catch (...) {
    return true;
  }
  return false;
}
/**
  Initialize the default role map that keeps the content from the
  default_roles table.
*/
void default_roles_init() { g_default_roles = new Default_roles; }

/**
  Delete the default role instance
*/
void default_roles_delete() { delete g_default_roles; }

/**
  Initialize the roles graph artifacts
*/
void roles_graph_init() {
  g_authid_to_vertex = new Role_index_map;
  g_granted_roles = new Granted_roles_graph;
}

/**
  Delete the ACL role graph artifacts
*/
void roles_graph_delete() {
  delete g_granted_roles;
  delete g_authid_to_vertex;
}

/**
  Initialize the roles caches that consist of the role graphs related
  artifacts and default role map. In theory, default role map is
  supposed to be a policy which has to be kept in sync with role graphs.
*/
void roles_init() {
  roles_graph_init();
  default_roles_init();
}

/**
  Delete the role caches
*/
void roles_delete() {
  roles_graph_delete();
  default_roles_delete();
}

void dynamic_privileges_init() {
  g_dynamic_privileges_map = new User_to_dynamic_privileges_map();
}

void dynamic_privileges_delete() {
  if (g_dynamic_privileges_map) delete g_dynamic_privileges_map;
  g_dynamic_privileges_map = nullptr;
}

User_to_dynamic_privileges_map *get_dynamic_privileges_map() {
  return g_dynamic_privileges_map;
}

void set_dynamic_privileges_map(User_to_dynamic_privileges_map *map) {
  g_dynamic_privileges_map = map;
}

User_to_dynamic_privileges_map *swap_dynamic_privileges_map(
    User_to_dynamic_privileges_map *map) {
  User_to_dynamic_privileges_map *old_map = g_dynamic_privileges_map;
  g_dynamic_privileges_map = map;
  return old_map;
}

bool assert_valid_privilege_id(const List<LEX_USER> *priv_list) {
  /*
    Because we need to combine the parsing rule of roles with the parsing
    rule of dynamic privileges LEX_USER::user is used to carry the name of
    the dynamic privilege.
  */
  for (const LEX_USER &priv : *priv_list) {
    Dynamic_privilege_register::iterator it =
        get_dynamic_privilege_register()->find(
            std::string(priv.user.str, priv.user.length));
    if (it == get_dynamic_privilege_register()->end()) {
      String error;
      error.append("No such privilege identifier: ");
      error.append(priv.user.str, priv.user.length);
      my_error(ER_UNKNOWN_ERROR, MYF(0), error.c_ptr_quick());
      return false;
    }
  }
  return true;
}

bool check_authorization_id_string(THD *thd, LEX_STRING &mandatory_roles) {
  bool error = false;
  std::string authid_str(mandatory_roles.str, mandatory_roles.length);
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) {
    error = true;
  } else {
    iterate_comma_separated_quoted_string(
        authid_str, [&thd, &error, &mandatory_roles](const std::string item) {
          auto el = get_authid_from_quoted_string(item);
          if (el.second != "" && el.first == "")
            error = true;
          else if (thd->security_context()
                       ->has_global_grant({el.first, el.second},
                                          consts::system_user, true)
                       .first) {
            // Send error to both, client and server error log.
            if (mysqld_server_started) {
              my_error(ER_DA_AUTH_ID_WITH_SYSTEM_USER_PRIV_IN_MANDATORY_ROLES,
                       MYF(0), el.first.c_str(), el.second.c_str(),
                       consts::system_user.c_str());
              LogErr(ERROR_LEVEL,
                     ER_AUTH_ID_WITH_SYSTEM_USER_PRIV_IN_MANDATORY_ROLES,
                     el.first.c_str(), el.second.c_str(),
                     consts::system_user.c_str());
              error = true;
            } else {
              LogErr(WARNING_LEVEL,
                     ER_WARN_AUTH_ID_WITH_SYSTEM_USER_PRIV_IN_MANDATORY_ROLES,
                     el.first.c_str(), el.second.c_str(),
                     consts::system_user.c_str());
              /*
                It is safe to reset the LEX_STRING since it is allocated in
                the memroot of THD.
              */
              mandatory_roles.str = empty_c_string;
              mandatory_roles.length = 0;
            }
          }
          return error;
        });
  }
  return error;
}

void get_mandatory_roles(std::vector<Role_id> *mandatory_roles) {
  mysql_mutex_lock(&LOCK_mandatory_roles);
  if (opt_mandatory_roles_cache) {
    /* Use pre-parsed auth ids from the cache */
    std::copy(g_mandatory_roles->begin(), g_mandatory_roles->end(),
              std::back_inserter(*mandatory_roles));
    mysql_mutex_unlock(&LOCK_mandatory_roles);
    return;
  }
  g_mandatory_roles->clear();
  /*
    We set this flag to indicate that we've already parsed the mandatory_roles
    option SQL variable.
  */
  opt_mandatory_roles_cache = true;
  std::string role_str;
  role_str.append(opt_mandatory_roles.str, opt_mandatory_roles.length);
  iterate_comma_separated_quoted_string(role_str, [&mandatory_roles](
                                                      const std::string item) {
    auto el = get_authid_from_quoted_string(item);
    if (el.second == "") el.second = "%";
    Role_id role_id(el.first, el.second);
    if (role_id.user() == "") {
      LogErr(WARNING_LEVEL, ER_ANONYMOUS_AUTH_ID_NOT_ALLOWED_IN_MANDATORY_ROLES,
             role_id.user().c_str(), role_id.host().c_str());
    } else if (find_acl_user(role_id.host().c_str(), role_id.user().c_str(),
                             true) != nullptr) {
      if (std::find(g_mandatory_roles->begin(), g_mandatory_roles->end(),
                    role_id) == g_mandatory_roles->end()) {
        mandatory_roles->push_back(role_id);
        g_mandatory_roles->push_back(role_id);
      }
    } else {
      LogErr(WARNING_LEVEL, ER_UNKNOWN_AUTH_ID_IN_MANDATORY_ROLE,
             role_id.user().c_str(), role_id.host().c_str());
    }
    return false;  // continue iterating
  });
  std::sort(g_mandatory_roles->begin(), g_mandatory_roles->end());
  mysql_mutex_unlock(&LOCK_mandatory_roles);
}

void update_mandatory_roles(void) {
  mysql_mutex_assert_owner(&LOCK_mandatory_roles);
  opt_mandatory_roles_cache = false;
  get_global_acl_cache()->increase_version();
}

Default_local_authid::Default_local_authid(const THD *thd) : m_thd(thd) {}

/**
 Check if the security context can be created as a local authid
 @param[out] sctx The authid to be checked.
 @return Success status
  @retval true an error occurred
  @retval false success
*/
bool Default_local_authid::precheck(
    Security_context *sctx MY_ATTRIBUTE((unused))) {
  return false;
}

/**
 Create a local authid without modifying any tables.
 @param[out] sctx The authid that will be extended with a user profile
 @return Success status
  @retval true an error occurred
  @retval false success
*/
bool Default_local_authid::create(
    Security_context *sctx MY_ATTRIBUTE((unused))) {
  return false;
}

Grant_temporary_dynamic_privileges::Grant_temporary_dynamic_privileges(
    const THD *thd, std::vector<std::string> privs)
    : m_thd(thd), m_privs(std::move(privs)) {}

bool Grant_temporary_dynamic_privileges::precheck(
    Security_context *sctx MY_ATTRIBUTE((unused))) {
  return false;
}

/**
 Grant dynamic privileges to an in-memory global authid
 @param sctx The authid to grant privileges to.
 @return Success status
  @retval true an error occurred
  @retval false success
 */
bool Grant_temporary_dynamic_privileges::grant_privileges(
    Security_context *sctx) {
  return grant_dynamic_privileges_to_auth_id(
      Role_id(sctx->priv_user(), sctx->priv_host()), m_privs);
}

void Drop_temporary_dynamic_privileges::operator()(Security_context *sctx) {
  revoke_dynamic_privileges_from_auth_id(
      Role_id(sctx->priv_user(), sctx->priv_host()), m_privs);
}

Grant_temporary_static_privileges::Grant_temporary_static_privileges(
    const THD *thd, ulong privs)
    : m_thd(thd), m_privs(privs) {}

bool Grant_temporary_static_privileges::precheck(
    Security_context *sctx MY_ATTRIBUTE((unused))) {
  return false;
}

bool Grant_temporary_static_privileges::grant_privileges(
    Security_context *sctx) {
  sctx->set_master_access(m_privs);
  return false;
}

bool Security_context_factory::apply_pre_constructed_policies(
    Security_context *sctx) {
  bool error = true;
  while (error) {
    if (m_user_profile) {
      // 1. Precheck conditions for creating the authid under current policy
      if (m_user_profile(sctx, Security_context_policy::Precheck)) break;
      // 2. Create the authid under the given policy
      if (m_user_profile(sctx, Security_context_policy::Execute)) break;
    }
    if (m_privileges) {
      // 3. Check preconditions for assigning privileges under the current
      //    policy
      if (m_privileges(sctx, Security_context_policy::Precheck)) break;
      // 4. Assign the privileges
      if (m_privileges(sctx, Security_context_policy::Execute)) break;
    }
    if (m_static_privileges) {
      // 5. Check preconditions for assigning privileges under the current
      //    policy
      if (m_static_privileges(sctx, Security_context_policy::Precheck)) break;
      // 6. Assign static privileges
      if (m_static_privileges(sctx, Security_context_policy::Execute)) break;
    }
    error = false;
  }
  if (error == false && m_drop_policy) sctx->set_drop_policy(m_drop_policy);
  return error;
}

Sctx_ptr<Security_context> Security_context_factory::create(
    MEM_ROOT *mem_root) {
  /* Setup default Security context */
  Security_context *sctx = new Security_context(mem_root);
  sctx->assign_user(m_user.c_str(), m_user.length());
  sctx->assign_host(m_host.c_str(), m_host.length());
  sctx->assign_priv_user(m_user.c_str(), m_user.length());
  sctx->assign_priv_host(m_host.c_str(), m_host.length());

  /* check if policies applied  successfully */
  if (apply_pre_constructed_policies(sctx)) {
    /* Each specific policy must raise its own errors */
    return nullptr;
  }

  return Sctx_ptr<Security_context>(sctx, [](Security_context *ptr) {
    if (ptr->has_drop_policy()) {
      ptr->execute_drop_policy();
      if (ptr->has_executed_drop_policy()) delete ptr;
    }
  });
}

bool operator==(const Role_id &a, const std::string &b) {
  std::string tmp;
  a.auth_str(&tmp);
  return tmp == b;
}

bool operator==(const std::pair<Role_id, bool> &a, const std::string &b) {
  return a.first == b;
}

bool operator==(const Role_id &a, const Auth_id_ref &b) {
  return ((a.user().length() == b.first.length) &&
          (a.host().length() == b.second.length) &&
          strncmp(a.user().c_str(), b.first.str, b.first.length) == 0 &&
          my_strcasecmp(system_charset_info, a.host().c_str(), b.second.str) ==
              0);
}

bool operator==(const Auth_id_ref &a, const Role_id &b) { return b == a; }

bool operator==(const std::pair<const Role_id, const Role_id> &a,
                const Auth_id_ref &b) {
  return ((a.second.user().length() == b.first.length) &&
          (a.second.host().length() == b.second.length) &&
          strncmp(a.second.user().c_str(), b.first.str, b.first.length) == 0 &&
          my_strcasecmp(system_charset_info, a.second.host().c_str(),
                        b.second.str) == 0);
}

bool operator==(const Role_id &a, const Role_id &b) {
  return ((a.user() == b.user()) && (a.host().length() == b.host().length()) &&
          (my_strcasecmp(system_charset_info, a.host().c_str(),
                         b.host().c_str()) == 0));
}

bool operator<(const Auth_id_ref &a, const Auth_id_ref &b) {
  if (a.first.length != b.first.length) return a.first.length < b.first.length;
  if (a.second.length != b.second.length)
    return a.second.length < b.second.length;

  int first = memcmp(a.first.str, b.first.str, a.first.length);
  if (first != 0) return first < 0;
  int second = memcmp(a.second.str, b.second.str, a.second.length);
  if (second != 0) return second < 0;
  return false;
}

bool operator==(std::pair<const Role_id, std::pair<std::string, bool>> &a,
                const std::string &b) {
  return a.second.first == b;
}

bool operator==(const LEX_CSTRING &a, const LEX_CSTRING &b) {
  return (a.length == b.length &&
          ((a.length == 0) || (memcmp(a.str, b.str, a.length) == 0)));
}

/**
  Checks if current user needs to be changed in case it is same as the LEX_USER.
  This check is useful to take backup of security context in case current user
  renames itself.

  @param sctx           The security context to check
  @param from_user_ptr  User name to be renamed

  @retval true          security context need to be updated
  @retval false         otherwise
*/
bool do_update_sctx(Security_context *sctx, LEX_USER *from_user_ptr) {
  const char *sctx_user = sctx->priv_user().str;
  const char *sctx_host = sctx->priv_host().str;
  const char *from_user = from_user_ptr->user.str;
  const char *from_host = from_user_ptr->host.str;

  /* If the user is connected as a proxied user, verify against proxy user */
  if (sctx->proxy_user().str && *sctx->proxy_user().str != '\0') {
    sctx_user = sctx->user().str;
  }

  /* Update the security context if current_user is going to be changed. */
  if (strcmp(from_user, sctx_user) == 0 &&
      my_strcasecmp(system_charset_info, from_host, sctx_host) == 0) {
    return true;
  }
  return false;
}

void update_sctx(Security_context *sctx, LEX_USER *to_user_ptr) {
  const char *to_user = to_user_ptr->user.str;
  const char *to_host = to_user_ptr->host.str;
  if (!to_host) to_host = "";
  if (!to_user) to_user = "";

  sctx->assign_priv_user(to_user_ptr->user.str, to_user_ptr->user.length);
  sctx->assign_priv_host(to_user_ptr->host.str, to_user_ptr->host.length);
}

/**
  Checks if any of the users has SYSTEM_USER privilege then current user
  must also have SYSTEM_USER privilege.
  It is a wrapper over the  Privilege_checker class that does
  privilege checks for one user at a time.

  @param [in] thd  Thread handle for security context
  @param [in] list List of user being processed

  @returns If needed, whether current user has SYSTEM_USER privilege or not
    @retval false Either none of the users in list has SYSTEM_USER
                  privilege or current user has SYSTEM_USER privilege
    @retval true  Failed in get_current_user() OR one of the user in the
                  list has SYSTEM_USER privilege but current user does not.
*/
bool check_system_user_privilege(THD *thd, List<LEX_USER> list) {
  LEX_USER *user, *tmp_user;
  Security_context *sctx = thd->security_context();
  List_iterator<LEX_USER> user_list(list);
  DBUG_ASSERT(assert_acl_cache_read_lock(thd));
  if (list.size() == 0) return (false);
  while ((tmp_user = user_list++)) {
    if (!(user = get_current_user(thd, tmp_user))) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
               consts::system_user.c_str());
      return (true);
    }
    if (sctx->can_operate_with({user}, consts::system_user)) return (true);
  }
  return (false);
}
