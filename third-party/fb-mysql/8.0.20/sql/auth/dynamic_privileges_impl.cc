/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include "sql/auth/dynamic_privileges_impl.h"

#include <ctype.h>
#include <mysql/components/my_service.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/dynamic_privilege.h>
#include <mysql/service_plugin_registry.h>
#include <stddef.h>
#include <string>
#include <unordered_set>
#include <utility>

#include "m_string.h"
#include "mysql/components/service.h"
#include "mysql/components/services/registry.h"
#include "mysql/psi/psi_base.h"
#include "sql/auth/dynamic_privilege_table.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/sql_thd_internal_api.h"  // create_thd

class THD;

/**
  This helper class is used for either selecting a previous THD or
  if it's missing, create a new THD.
*/
class Thd_creator {
 public:
  Thd_creator(THD *thd) : m_thd(thd), m_tmp_thd(nullptr) {}

  /**
    Returns a THD handle either by creating a new one or by returning a
    previously created THD.
  */
  THD *operator()() {
    if (m_thd == nullptr && m_tmp_thd == nullptr) {
      /*
        Initiate a THD without plugins,
        without attaching to the Global_THD_manager, and without setting
        an OS thread ID.
      */
      m_tmp_thd = create_thd(false, true, false, PSI_NOT_INSTRUMENTED);
      return m_tmp_thd;
    } else if (m_thd == nullptr) {
      return m_tmp_thd;
    }
    return m_thd;
  }

  /**
    Automatically frees any THD handle created by this class.
  */
  ~Thd_creator() {
    if (m_thd == nullptr && m_tmp_thd != nullptr) {
      destroy_thd(m_tmp_thd);
    }
  }

 private:
  THD *m_thd;
  THD *m_tmp_thd;
};

/**
  Register a privilege identifiers in the list of known identifiers. This
  enable the SQL syntax to recognize the identifier as a valid token.
  @param privilege_str The privilege identifier string
  @param privilege_str_len The length of the identifier string

  @note This function acquires the THD from the current_thd

  @returns Error flag
    @return true The privilege ID couldn't be inserted.
    @return false The privilege ID was successfully registered.
*/

DEFINE_BOOL_METHOD(dynamic_privilege_services_impl::register_privilege,
                   (const char *privilege_str, size_t privilege_str_len)) {
  Thd_creator get_thd(current_thd);
  try {
    Acl_cache_lock_guard acl_cache_lock(get_thd(),
                                        Acl_cache_lock_mode::WRITE_MODE);
    Dynamic_privilege_register *reg = get_dynamic_privilege_register();
    std::string priv;
    const char *c = &privilege_str[0];
    for (size_t i = 0; i < privilege_str_len; ++i, ++c)
      priv.append(1, static_cast<char>(toupper(*c)));
    if (reg->find(priv) != reg->end()) {
      /* If the privilege ID already is registered; report success */
      return false;
    }
    return !get_dynamic_privilege_register()->insert(priv).second;
  } catch (...) {
    return true;
  }
}

/**
  Unregister a privilege identifiers in the list of known identifiers. This
  disables the SQL syntax from recognizing the identifier as a valid token.
  @param privilege_str The privilege identifier string
  @param privilege_str_len The length of the identifier string

  @note This function acquires the THD from the current_thd

  @returns Error flag
    @return true The privilege ID wasn't in the list or remove failed.
    @return false The privilege ID was successfully unregistered.
*/

DEFINE_BOOL_METHOD(dynamic_privilege_services_impl::unregister_privilege,
                   (const char *privilege_str, size_t privilege_str_len)) {
  Thd_creator get_thd(current_thd);
  try {
    Acl_cache_lock_guard acl_cache_lock(get_thd(),
                                        Acl_cache_lock_mode::WRITE_MODE);
    std::string priv;
    const char *c = &privilege_str[0];
    for (size_t i = 0; i < privilege_str_len; ++i, ++c)
      priv.append(1, static_cast<char>(toupper(*c)));
    return (get_dynamic_privilege_register()->erase(priv) == 0);
  } catch (...) {
    return true;
  }
}

/**
  Checks if a user has a specified privilege ID granted to it.

  @param handle The active security context of the user to be checked.
  @param privilege_str The privilege identifier string
  @param privilege_str_len The length of the identifier string

  @returns Success state
    @return true The user has the grant
    @return false The user hasn't the grant
*/

DEFINE_BOOL_METHOD(dynamic_privilege_services_impl::has_global_grant,
                   (Security_context_handle handle, const char *privilege_str,
                    size_t privilege_str_len)) {
  Security_context *sctx = reinterpret_cast<Security_context *>(handle);
  return sctx->has_global_grant(privilege_str, privilege_str_len).first;
}

/**
  Boostrap the dynamic privilege service by seeding it with server
  implementation specific data.
*/

bool dynamic_privilege_init(void) {
  // Set up default dynamic privileges
  SERVICE_TYPE(registry) *r = mysql_plugin_registry_acquire();
  bool ret = false;
  {
    my_service<SERVICE_TYPE(dynamic_privilege_register)> service(
        "dynamic_privilege_register.mysql_server", r);
    if (service.is_valid()) {
      ret |= service->register_privilege(STRING_WITH_LEN("ROLE_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("SYSTEM_VARIABLES_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("BINLOG_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("REPLICATION_SLAVE_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("GROUP_REPLICATION_ADMIN"));
      ret |=
          service->register_privilege(STRING_WITH_LEN("ENCRYPTION_KEY_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("CONNECTION_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("SET_USER_ID"));
      ret |= service->register_privilege(STRING_WITH_LEN("XA_RECOVER_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("PERSIST_RO_VARIABLES_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("BACKUP_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("CLONE_ADMIN"));
      ret |=
          service->register_privilege(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN"));
      ret |=
          service->register_privilege(STRING_WITH_LEN("RESOURCE_GROUP_USER"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("SESSION_VARIABLES_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("BINLOG_ENCRYPTION_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("SERVICE_CONNECTION_ADMIN"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("APPLICATION_PASSWORD_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("SYSTEM_USER"));
      ret |= service->register_privilege(
          STRING_WITH_LEN("TABLE_ENCRYPTION_ADMIN"));
      ret |= service->register_privilege(STRING_WITH_LEN("AUDIT_ADMIN"));
      ret |=
          service->register_privilege(STRING_WITH_LEN("REPLICATION_APPLIER"));
      ret |= service->register_privilege(STRING_WITH_LEN("SHOW_ROUTINE"));
    }
  }  // exist scope
  mysql_plugin_registry_release(r);
  return ret;
}
