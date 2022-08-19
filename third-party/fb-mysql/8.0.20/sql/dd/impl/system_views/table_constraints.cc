/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/table_constraints.h"

#include <string>

#include "sql/stateless_allocator.h"

namespace dd {
namespace system_views {

const Table_constraints &Table_constraints::instance() {
  static Table_constraints *s_instance = new Table_constraints();
  return *s_instance;
}

Table_constraints::Table_constraints() {
  m_target_def.set_view_name(view_name());

  // First SELECT for UNION
  System_view_select_definition_impl &first_select = m_target_def.get_select();

  first_select.add_field(FIELD_CONSTRAINT_CATALOG, "CONSTRAINT_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  first_select.add_field(FIELD_CONSTRAINT_SCHEMA, "CONSTRAINT_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  first_select.add_field(FIELD_CONSTRAINT_NAME, "CONSTRAINT_NAME", "idx.name");
  first_select.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  first_select.add_field(FIELD_TABLE_NAME, "TABLE_NAME",
                         "tbl.name" + m_target_def.fs_name_collation());
  first_select.add_field(FIELD_CONSTRAINT_TYPE, "CONSTRAINT_TYPE",
                         "IF (idx.type='PRIMARY', 'PRIMARY KEY', idx.type)");
  first_select.add_field(FIELD_ENFORCED, "ENFORCED", "'YES'");

  first_select.add_from("mysql.indexes idx");
  first_select.add_from("JOIN mysql.tables tbl ON idx.table_id=tbl.id");
  first_select.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  first_select.add_from(
      "JOIN mysql.catalogs cat ON cat.id=sch.catalog_id"
      " AND idx.type IN ('PRIMARY', 'UNIQUE')");

  first_select.add_where("CAN_ACCESS_TABLE(sch.name, tbl.name)");
  first_select.add_where("AND IS_VISIBLE_DD_OBJECT(tbl.hidden, idx.hidden)");

  // Second SELECT for UNION
  System_view_select_definition_impl &second_select = m_target_def.get_select();

  second_select.add_field(FIELD_CONSTRAINT_CATALOG, "CONSTRAINT_CATALOG",
                          "cat.name" + m_target_def.fs_name_collation());
  second_select.add_field(FIELD_CONSTRAINT_SCHEMA, "CONSTRAINT_SCHEMA",
                          "sch.name" + m_target_def.fs_name_collation());
  second_select.add_field(FIELD_CONSTRAINT_NAME, "CONSTRAINT_NAME",
                          "fk.name COLLATE utf8_tolower_ci");
  second_select.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                          "sch.name" + m_target_def.fs_name_collation());
  second_select.add_field(FIELD_TABLE_NAME, "TABLE_NAME",
                          "tbl.name" + m_target_def.fs_name_collation());
  second_select.add_field(FIELD_CONSTRAINT_TYPE, "CONSTRAINT_TYPE",
                          "'FOREIGN KEY'");
  second_select.add_field(FIELD_ENFORCED, "ENFORCED", "'YES'");

  second_select.add_from("mysql.foreign_keys fk");
  second_select.add_from("JOIN mysql.tables tbl ON fk.table_id=tbl.id");
  second_select.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  second_select.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");

  second_select.add_where("CAN_ACCESS_TABLE(sch.name, tbl.name)");
  second_select.add_where("AND IS_VISIBLE_DD_OBJECT(tbl.hidden)");

  // Third SELECT for UNION
  System_view_select_definition_impl &third_select = m_target_def.get_select();

  third_select.add_field(FIELD_CONSTRAINT_CATALOG, "CONSTRAINT_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  third_select.add_field(FIELD_CONSTRAINT_SCHEMA, "CONSTRAINT_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  third_select.add_field(FIELD_CONSTRAINT_NAME, "CONSTRAINT_NAME", "cc.name");
  third_select.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  third_select.add_field(FIELD_TABLE_NAME, "TABLE_NAME",
                         "tbl.name" + m_target_def.fs_name_collation());
  third_select.add_field(FIELD_CONSTRAINT_TYPE, "CONSTRAINT_TYPE", "'CHECK'");
  third_select.add_field(FIELD_ENFORCED, "ENFORCED", "cc.enforced");

  third_select.add_from("mysql.check_constraints cc");
  third_select.add_from("JOIN mysql.tables tbl ON cc.table_id=tbl.id");
  third_select.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  third_select.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");

  third_select.add_where("CAN_ACCESS_TABLE(sch.name, tbl.name)");
  third_select.add_where("AND IS_VISIBLE_DD_OBJECT(tbl.hidden)");
}

}  // namespace system_views
}  // namespace dd
