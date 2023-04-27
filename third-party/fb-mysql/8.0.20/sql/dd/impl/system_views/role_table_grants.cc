/* Copyright (c) 2019, 2020,  Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/role_table_grants.h"
#include "sql/dd/impl/system_views/applicable_roles.h"

namespace dd {
namespace system_views {

const Role_table_grants &Role_table_grants::instance() {
  static Role_table_grants *s_instance = new Role_table_grants();
  return *s_instance;
}

Role_table_grants::Role_table_grants() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_distinct();

  m_target_def.add_cte_expression(Applicable_roles::cte_expression());

  m_target_def.add_field(FIELD_GRANTOR, "GRANTOR",
                         "INTERNAL_GET_USERNAME(Grantor)");
  m_target_def.add_field(FIELD_GRANTOR_HOST, "GRANTOR_HOST",
                         "INTERNAL_GET_HOSTNAME(Grantor)");
  m_target_def.add_field(FIELD_GRANTEE, "GRANTEE", "tp.User");
  m_target_def.add_field(FIELD_GRANTEE_HOST, "GRANTEE_HOST", "tp.Host");
  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG", "'def'");
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA", "Db");
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "Table_name");
  m_target_def.add_field(FIELD_PRIVILEGE_TYPE, "PRIVILEGE_TYPE", "Table_priv");
  m_target_def.add_field(FIELD_IS_GRANTABLE, "IS_GRANTABLE",
                         "IF(FIND_IN_SET('Grant',Table_priv)>0, 'YES', 'NO')");

  m_target_def.add_from("mysql.tables_priv tp");
  m_target_def.add_from(
      " JOIN role_graph rg ON "
      " tp.User = rg.c_from_user AND "
      "  CONVERT(tp.Host using utf8mb4) = rg.c_from_host");

  m_target_def.add_where("Table_priv > 0");
  m_target_def.add_where(" AND c_to_user != ''");
  m_target_def.add_where(" AND c_enabled = TRUE");
}

}  // namespace system_views
}  // namespace dd
