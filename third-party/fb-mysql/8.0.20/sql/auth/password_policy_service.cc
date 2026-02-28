/*  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>

#include "lex_string.h"
#include "m_ctype.h" /* my_charset_utf8_bin */
#include "m_string.h"
#include "my_dbug.h" /* DBUG_ASSERT */
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/plugin.h"
#include "mysql/plugin_validate_password.h" /* validate_password plugin */
#include "mysqld_error.h"                   /* To get ER_NOT_VALID_PASSWORD */
#include "sql/current_thd.h"
#include "sql/mysqld.h" /* srv_registry */
#include "sql/sql_plugin.h"
#include "sql/sql_plugin_ref.h"
#include "sql_string.h"

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/validate_password.h>

/**
  Static name of the built in plugin used by mysql_password_policy_service_st
  for password validation.
*/
LEX_CSTRING validate_password_plugin = {STRING_WITH_LEN("validate_password")};

/**
  Invoke the component/plugin to validate the input password.

  Implementation of a plugin service @ref mysql_password_policy_service_st
  method.
  Calls the @ref validate_password_plugin / validate_password_component
  plugin's / component's @ref
  st_mysql_validate_password::validate_password method.
  Constructs a temporary binary @ref String object out of the password
  supplied.

  @param password        password which needs to be validated against the
                         defined policies
  @param password_len    length of password

  @retval 0 Password OK
  @retval 1 Password incompatible with policy

  @sa st_mysql_validate_password,
  mysql_password_policy_service_st::my_validate_password_policy_func
*/

int my_validate_password_policy(const char *password,
                                unsigned int password_len) {
  plugin_ref plugin;
  String password_str;
  my_h_service h_pv_svc = nullptr;
  SERVICE_TYPE(validate_password) * ret;
  int res = 0;

  if (password) {
    String tmp_str(password, password_len, &my_charset_utf8_bin);
    password_str = tmp_str;
  }
  if (!srv_registry->acquire("validate_password", &h_pv_svc)) {
    ret = reinterpret_cast<SERVICE_TYPE(validate_password) *>(h_pv_svc);
    if (ret->validate((void *)current_thd, (my_h_string)&password_str)) {
      my_error(ER_NOT_VALID_PASSWORD, MYF(0));
      res = 1;
    }
    srv_registry->release(h_pv_svc);
  } else {
    plugin = my_plugin_lock_by_name(nullptr, validate_password_plugin,
                                    MYSQL_VALIDATE_PASSWORD_PLUGIN);
    if (plugin) {
      st_mysql_validate_password *password_validate =
          (st_mysql_validate_password *)plugin_decl(plugin)->info;

      if (!password_validate->validate_password(&password_str)) {
        my_error(ER_NOT_VALID_PASSWORD, MYF(0));
        res = 1;
      }
      plugin_unlock(nullptr, plugin);
    }
  }

  return res;
}

/**
  Invoke the component/plugin to evalue the strength of a password.

  Implementation of a plugin service @ref mysql_password_policy_service_st
  method.
  Typically called when new user is created or exsisting password is changed.
  Calls the @ref validate_password_plugin / validate_password_component
  plugin's / component's @ref st_mysql_validate_password::get_password_strength
  method.
  Constructs a temporary binary @ref String object out of the password
  supplied.

  @param password        password which needs to be validated against the
                         defined policies
  @param password_len    length of password

  @return password strength score (0-100)

  @sa st_mysql_validate_password,
  mysql_password_policy_service_st::my_calculate_password_strength_func
*/
int my_calculate_password_strength(const char *password,
                                   unsigned int password_len) {
  int res = 0;
  unsigned int strength;
  DBUG_ASSERT(password != nullptr);

  my_h_service h_pv_svc = nullptr;
  SERVICE_TYPE(validate_password) * ret;
  String password_str;

  if (password) password_str.set(password, password_len, &my_charset_utf8_bin);
  if (!srv_registry->acquire("validate_password", &h_pv_svc)) {
    ret = reinterpret_cast<SERVICE_TYPE(validate_password) *>(h_pv_svc);
    if (!ret->get_strength((void *)current_thd, (my_h_string)&password_str,
                           &strength))
      res = strength;
    srv_registry->release(h_pv_svc);
  } else {
    plugin_ref plugin = my_plugin_lock_by_name(
        nullptr, validate_password_plugin, MYSQL_VALIDATE_PASSWORD_PLUGIN);
    if (plugin) {
      st_mysql_validate_password *password_strength =
          (st_mysql_validate_password *)plugin_decl(plugin)->info;

      res = password_strength->get_password_strength(&password_str);
      plugin_unlock(nullptr, plugin);
    }
  }
  return (res);
}
