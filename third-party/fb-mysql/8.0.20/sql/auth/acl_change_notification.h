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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef ACL_CHANGE_NOTIFICATION_INCLUDED
#define ACL_CHANGE_NOTIFICATION_INCLUDED

#include "my_sqlcommand.h"  // enum_sql_command
#include "sql/table.h"      // LEX_USER, LEX_CSTRING, List

class Rewrite_params;  // forward declaration

class Acl_change_notification {
 public:
  struct User {
    std::string name;
    std::string host;
    User(const LEX_USER &lex_user)
        : name(lex_user.user.str, lex_user.user.length),
          host(lex_user.host.str, lex_user.host.length) {}
  };

  Acl_change_notification(class THD *thd, enum_sql_command op,
                          const List<LEX_USER> *users,
                          Rewrite_params *rewrite_params,
                          const List<LEX_CSTRING> *dynamic_privs);

 private:
  enum_sql_command operation;
  std::string db;
  std::vector<User> user_list;
  std::vector<std::string> dynamic_privilege_list;
  Rewrite_params *rewrite_params;

 public:
  enum_sql_command get_operation() const { return operation; }
  const std::string &get_db() const { return db; }
  const std::vector<User> &get_user_list() const { return user_list; }
  const std::vector<std::string> &get_dynamic_privilege_list() const {
    return dynamic_privilege_list;
  }
  Rewrite_params *get_rewrite_params() const { return rewrite_params; }
};

#endif
