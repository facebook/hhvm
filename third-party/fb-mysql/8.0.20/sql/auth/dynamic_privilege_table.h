/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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
#ifndef DYNAMIC_PRIVILEGE_TABLE_H
#define DYNAMIC_PRIVILEGE_TABLE_H

#include <functional>
#include <string>
#include <unordered_set>

#include "lex_string.h"
#include "sql/auth/auth_common.h"

class THD;
struct TABLE;
struct TABLE_LIST;

typedef std::unordered_set<std::string> Dynamic_privilege_register;

bool populate_dynamic_privilege_caches(THD *thd, TABLE_LIST *tablelst);
bool modify_dynamic_privileges_in_table(THD *thd, TABLE *table,
                                        const Auth_id_ref &auth_id,
                                        const LEX_CSTRING &privilege,
                                        bool with_grant_option,
                                        bool delete_option);
class Update_dynamic_privilege_table {
 public:
  enum Operation { GRANT, REVOKE };
  Update_dynamic_privilege_table() : m_no_update(true) {}
  Update_dynamic_privilege_table(THD *thd, TABLE *table)
      : m_thd(thd), m_table(table), m_no_update(false) {}
  bool operator()(const std::string &priv, const Auth_id_ref &auth_id,
                  bool grant_option,
                  Update_dynamic_privilege_table::Operation op) {
    if (m_no_update) return false;
    LEX_CSTRING cstr_priv = {priv.c_str(), priv.length()};
    return modify_dynamic_privileges_in_table(m_thd, m_table, auth_id,
                                              cstr_priv, grant_option,
                                              op == Operation::REVOKE);
  }

 private:
  THD *m_thd;
  TABLE *m_table;
  bool m_no_update;
};

Dynamic_privilege_register *get_dynamic_privilege_register(void);
void register_dynamic_privilege_impl(const std::string &priv);
bool iterate_all_dynamic_privileges(THD *thd,
                                    std::function<bool(const char *)> action);
#endif
