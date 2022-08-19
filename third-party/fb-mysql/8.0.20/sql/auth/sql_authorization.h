/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_AUTHORIZATION_INCLUDED
#define SQL_AUTHORIZATION_INCLUDED

#include <functional>
#include <string>
#include <utility>

#include "lex_string.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "sql/auth/sql_auth_cache.h"

class String;
class THD;

void roles_graphml(THD *thd, String *);
bool check_if_granted_role(LEX_CSTRING user, LEX_CSTRING host, LEX_CSTRING role,
                           LEX_CSTRING role_host);
bool find_if_granted_role(Role_vertex_descriptor v, LEX_CSTRING role,
                          LEX_CSTRING role_host,
                          Role_vertex_descriptor *found_vertex = nullptr);
void iterate_comma_separated_quoted_string(
    std::string str, const std::function<bool(const std::string)> &f);
void get_granted_roles(Role_vertex_descriptor &v,
                       List_of_granted_roles *granted_roles);
void get_granted_roles(Role_vertex_descriptor &v,
                       std::function<void(const Role_id &, bool)> f);
/* For for get_mandatory_roles and Sys_mandatory_roles */
extern mysql_mutex_t LOCK_mandatory_roles;
#endif /* SQL_AUTHORIZATION_INCLUDED */
