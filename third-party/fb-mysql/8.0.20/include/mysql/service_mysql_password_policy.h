/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_SERVICE_MYSQL_PLUGIN_AUTH_INCLUDED
#define MYSQL_SERVICE_MYSQL_PLUGIN_AUTH_INCLUDED

/**
  @file include/mysql/service_mysql_password_policy.h

  Definitions for the password validation service
*/

/**
  @ingroup group_ext_plugin_services

  This service allows plugins to validate passwords based on a common policy.

  This is a "bridge service", i.e. facade with no
  real functionality that just calls the actual password validation plugin
  APIs. This serive is needed by other plugins to call into the password
  validation plugins and thus overcome the limitation that only the server
  can call plugins.

  @sa st_mysql_validate_password
*/
extern "C" struct mysql_password_policy_service_st {
  /**
    Validates a password.

    @sa my_validate_password_policy,
    st_mysql_validate_password::validate_password
  */
  int (*my_validate_password_policy_func)(const char *, unsigned int);
  /**
    Evaluates the strength of a password in a scale 0-100

    @sa my_calculate_password_strength,
    st_mysql_validate_password::get_password_strength
  */
  int (*my_calculate_password_strength_func)(const char *, unsigned int);
} * mysql_password_policy_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define my_validate_password_policy(buffer, length)                       \
  mysql_password_policy_service->my_validate_password_policy_func(buffer, \
                                                                  length)
#define my_calculate_password_strength(buffer, length)                       \
  mysql_password_policy_service->my_calculate_password_strength_func(buffer, \
                                                                     length)

#else

int my_validate_password_policy(const char *, unsigned int);
int my_calculate_password_strength(const char *, unsigned int);

#endif

#endif
