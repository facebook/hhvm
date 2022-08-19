/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PLUGIN_VALIDATE_PASSWORD_INCLUDED
#define MYSQL_PLUGIN_VALIDATE_PASSWORD_INCLUDED

/**
  @file include/mysql/plugin_validate_password.h
  API for validate_password plugin. (MYSQL_VALIDATE_PASSWORD_PLUGIN)
*/

#include <mysql/plugin.h>
#define MYSQL_VALIDATE_PASSWORD_INTERFACE_VERSION 0x0100

typedef void *mysql_string_handle;

/**
  This plugin type defines interface that the server uses to
  enforce a password policy.

  The policy is enfoced through st_mysql_validate_password::validate_password()
  that answers the question of whether this password is good enough or not.

  There's one auxilary functon
  st_mysql_validate_password::get_password_strength() that can be used by
  password changing UIs to display a password strength meter as the user enters
  a password.

  Since plugins may need that functionality there's a plugin service
  @ref mysql_password_policy_service_st exposing
  it to other plugins.

  There also is a default password policy plugin
  "validate_password" built into the server binary that
  implements this plugin API.

  @sa mysql_password_policy_service_st
*/
struct st_mysql_validate_password {
  int interface_version;
  /**
    Checks if a password is valid by the password policy

    @param password  The password to validate
    @retval true   password meets the password validation plugin policy
    @retval false  password does not meet the validation policy
  */
  int (*validate_password)(mysql_string_handle password);
  /**
    Calculates the strength of a password in the scale of 0 to 100.

    @param password The password to evaluate the strength of
    @return  The strength of the password (0-100)
  */
  int (*get_password_strength)(mysql_string_handle password);
};
#endif
