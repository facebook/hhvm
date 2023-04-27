/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/auth/roles.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_authorization.h"
#include "sql/auth/sql_security_ctx.h"

#include "sql/sql_admin.h" /* role_enum */
#include "sql/sql_class.h" /* THD */
#include "sql/sql_error.h" /* Diagonstics_area */
#include "sql/sql_list.h"  /* List */
#include "sql/sql_parse.h" /* get_current_user */
#include "sql/table.h"     /* LEX_USER */

#include "m_string.h" /* native_strncasecmp */

extern Role_index_map *g_authid_to_vertex;

namespace Roles {

/**
  Constructor.

  @param [in] thd         THD handle
  @param [in] sctx        Security context handle
  @param [in] type        Role activation type
  @param [in] role_list   List of roles - depends on type
  @param [in] raise_error Flag to raise error
*/
Role_activation::Role_activation(
    THD *thd, Security_context *sctx, role_enum type,
    const List<LEX_USER> *role_list /* = nullptr */,
    bool raise_error /* = false */)
    : m_thd(thd),
      m_sctx(sctx != nullptr ? sctx : thd->security_context()),
      m_type(type),
      m_role_list(role_list),
      m_raise_error(raise_error) {
  if (!m_thd || (m_type == role_enum::ROLE_NAME && m_role_list == nullptr))
    m_valid = false;
  else
    m_valid = true;
}

/**
  Activate roles for given session

  @returns status of role activation
    @retval false Success
    @retval true  Failure. Error is raised.
*/
bool Role_activation::activate() {
  DBUG_TRACE;
  bool set_role_status = true;
  switch (m_type) {
    case role_enum::ROLE_DEFAULT:
      set_role_status = activate_role_default();
      break;
    case role_enum::ROLE_ALL:
      set_role_status = activate_role_all();
      break;
    case role_enum::ROLE_NAME:
      set_role_status = activate_role_name();
      break;
    case role_enum::ROLE_NONE:
      // Fall through
    default:
      set_role_status = activate_role_none();
  }
  return set_role_status;
}

/**
  Deactivate all roles for current session

  @returns Operation status
    @retval false Success
    @retval true Error
*/
bool Role_activation::activate_role_none() {
  DBUG_TRACE;
  m_sctx->clear_active_roles();
  m_sctx->clear_db_restrictions();
  m_sctx->checkout_access_maps();
  ulong new_db_access = m_sctx->db_acl(m_thd->db());
  m_sctx->cache_current_db_access(new_db_access);
  Acl_cache_lock_guard acl_cache_lock(m_thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(m_raise_error)) return true;
  ACL_USER *user =
      find_acl_user(m_sctx->priv_host().str, m_sctx->priv_user().str, true);
  if (user) {
    m_sctx->set_master_access(user->access,
                              acl_restrictions->find_restrictions(user));
  }
  return false;
}

/**
  Activate default roles for current session

  @returns Operation status
    @retval false Success
    @retval true Error
*/
bool Role_activation::activate_role_default() {
  DBUG_TRACE;
  bool ret = false;
  Acl_cache_lock_guard acl_cache_lock(m_thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(m_raise_error)) return true;
  List_of_auth_id_refs *active_list = m_sctx->get_active_roles();
  List_of_auth_id_refs authids;
  List_of_auth_id_refs backup_active_list;
  backup_active_list.reserve(active_list->size());
  /* Shallow copy of LEX_CSTRING pairs. Memory not duplicated */
  std::copy(active_list->begin(), active_list->end(),
            std::back_inserter(backup_active_list));
  /* Clear active roles but don't free memory */
  m_sctx->get_active_roles()->clear();
  LEX_USER current_user;
  /* hack for the current_user token */
  get_default_definer(m_thd, &current_user);
  Auth_id_ref current_user_authid = create_authid_from(&current_user);
  /*
    Search global structure for target user;
    authids have their own memory storage (Role_id)
  */
  get_default_roles(current_user_authid, authids);
  if (authids.size() > 0) {
    List_of_auth_id_refs::iterator it = authids.begin();
    for (; it != authids.end() && ret == 0; ++it) {
      /*
         Activating a role allocates new memory for the activated role
         and perform a deep copy of the default role.
      */
      ret = m_sctx->activate_role(it->first, it->second, true);
      if (ret && m_raise_error) {
        my_error(ER_ROLE_NOT_GRANTED, MYF(0), it->first.str, it->second.str,
                 current_user_authid.first.str, current_user_authid.second.str);
      }
    }
  }
  if (ret == 0) {
    m_sctx->checkout_access_maps();
    ulong new_db_access = m_sctx->db_acl(m_thd->db());
    m_sctx->cache_current_db_access(new_db_access);
    /* Old memory in the backup list must now be freed. */
    for (auto &&role : backup_active_list) {
      my_free(const_cast<char *>(role.first.str));
      my_free(const_cast<char *>(role.second.str));
    }
  } else {
    /*
      Failing to activate all roles will rollback the statement and reset
      the previous roles.
      1. Remove any newly activated roles and deallocate memory
      2. Copy the backup elements to the active_list (shallow copy)
    */
    m_sctx->clear_active_roles();
    std::copy(backup_active_list.begin(), backup_active_list.end(),
              std::back_inserter(*active_list));
  }
  return ret;
}

/**
  Activate all granted roles - except those specified through m_role_list

  @returns Operation status
    @retval false Success
    @retval true Error
*/
bool Role_activation::activate_role_all() {
  DBUG_TRACE;
  Acl_cache_lock_guard acl_cache_lock(m_thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(m_raise_error)) return true;

  List_of_auth_id_refs *active_list = m_sctx->get_active_roles();
  List_of_auth_id_refs backup_active_list;
  backup_active_list.reserve(active_list->size());
  std::copy(active_list->begin(), active_list->end(),
            std::back_inserter(backup_active_list));
  m_sctx->get_active_roles()->clear();

  m_sctx->clear_active_roles();
  bool ret = false;
  LEX_USER *current_user = create_default_definer(m_thd);
  std::string authid = create_authid_str_from(current_user);
  Role_index_map::iterator it;
  List_of_granted_roles granted_roles;

  if ((it = g_authid_to_vertex->find(authid)) != g_authid_to_vertex->end()) {
    Role_vertex_descriptor user_vertex = it->second;
    get_granted_roles(user_vertex, &granted_roles);
    std::vector<Role_id> mandatory_roles;
    get_mandatory_roles(&mandatory_roles);
    for (auto &&rid : mandatory_roles) {
      granted_roles.push_back(std::make_pair(rid, false));
    }
    List_of_granted_roles::iterator role_it = granted_roles.begin();
    for (; role_it != granted_roles.end(); ++role_it) {
      bool found_except_role = false;
      if (m_role_list && m_role_list->elements > 0) {
        for (const LEX_USER &except_role : *m_role_list) {
          if ((except_role.user.length == role_it->first.user().length()) &&
              (except_role.host.length == role_it->first.host().length()) &&
              strncmp(except_role.user.str, role_it->first.user().c_str(),
                      except_role.user.length) == 0 &&
              native_strncasecmp(except_role.host.str,
                                 role_it->first.host().c_str(),
                                 except_role.host.length) == 0) {
            found_except_role = true;
            break;
          }
        }
      }
      if (!found_except_role) {
        ret = m_sctx->activate_role(
            {role_it->first.user().c_str(), role_it->first.user().length()},
            {role_it->first.host().c_str(), role_it->first.host().length()},
            true);
        if (ret != 0 && m_raise_error) {
          my_error(ER_ROLE_NOT_GRANTED, MYF(0), role_it->first.user().c_str(),
                   role_it->first.host().c_str(), current_user->user.str,
                   current_user->host.str);
          break;
        }
      }
    }  // end for
  }
  if (ret == 0) {
    m_sctx->checkout_access_maps();
    ulong new_db_access = m_sctx->db_acl(m_thd->db());
    m_sctx->cache_current_db_access(new_db_access);
    /* Drop backup */
    for (auto &&ref : backup_active_list) {
      my_free(const_cast<char *>(ref.first.str));
      my_free(const_cast<char *>(ref.second.str));
    }
  } else {
    /*
      Failing to activate all roles will rollback the statement and reset
      the previous roles.
      1. Remove any newly activated roles and deallocate memory
      2. Copy the backup elements to the active_list (shallow copy)
    */
    m_sctx->clear_active_roles();
    std::copy(backup_active_list.begin(), backup_active_list.end(),
              std::back_inserter(*active_list));
  }
  return ret;
}

/**
  Activate roles available through m_role_list

  @returns Operation status
    @retval false Success
    @retval true Error
*/
bool Role_activation::activate_role_name() {
  DBUG_TRACE;
  bool ret = false;
  Acl_cache_lock_guard acl_cache_lock(m_thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock(m_raise_error)) return true;

  List_of_auth_id_refs *active_list = m_sctx->get_active_roles();
  List_of_auth_id_refs backup_active_list;
  backup_active_list.reserve(active_list->size());
  std::copy(active_list->begin(), active_list->end(),
            std::back_inserter(backup_active_list));
  m_sctx->get_active_roles()->clear();
  List_iterator<LEX_USER> it(*(const_cast<List<LEX_USER> *>(m_role_list)));
  LEX_USER *role = nullptr;
  while (ret == 0 && (role = it++)) {
    ret = m_sctx->activate_role(role->user, role->host, true);
  }

  if (ret == 0) {
    m_sctx->checkout_access_maps();
    ulong new_db_access = m_sctx->db_acl(m_thd->db());
    m_sctx->cache_current_db_access(new_db_access);
    /* Drop backup */
    for (auto &&ref : backup_active_list) {
      my_free(const_cast<char *>(ref.first.str));
      my_free(const_cast<char *>(ref.second.str));
    }
  } else {
    if (role && m_raise_error) {
      my_error(ER_ROLE_NOT_GRANTED, MYF(0), role->user.str, role->host.str,
               m_sctx->priv_user().str, m_sctx->priv_host().str);
    }
    /*
      Failing to activate all roles will rollback the statement and reset
      the previous roles.
      1. Remove any newly activated roles and deallocate memory
      2. Copy the backup elements to the active_list (shallow copy)
    */
    m_sctx->clear_active_roles();
    std::copy(backup_active_list.begin(), backup_active_list.end(),
              std::back_inserter(*active_list));
  }
  return ret;
}

}  // namespace Roles
