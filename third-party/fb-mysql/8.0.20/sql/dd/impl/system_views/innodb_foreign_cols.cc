/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/innodb_foreign_cols.h"

namespace dd {
namespace system_views {

const Innodb_foreign_cols &Innodb_foreign_cols::instance() {
  static Innodb_foreign_cols *s_instance = new Innodb_foreign_cols();
  return *s_instance;
}

Innodb_foreign_cols::Innodb_foreign_cols() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_FOREIGN_ID, "ID",
      "CONCAT(sch.name, '/', fk.name)" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_FOR_COL_NAME, "FOR_COL_NAME", "col.name");
  m_target_def.add_field(FIELD_REF_COL_NAME, "REF_COL_NAME",
                         "referenced_column_name");
  m_target_def.add_field(FIELD_COL_POS, "POS", "fk_col.ordinal_position");

  m_target_def.add_from("mysql.foreign_key_column_usage fk_col");
  m_target_def.add_from(
      "JOIN mysql.foreign_keys fk ON "
      "fk.id=fk_col.foreign_key_id");
  m_target_def.add_from("JOIN mysql.tables tbl ON fk.table_id=tbl.id");
  m_target_def.add_from("JOIN mysql.schemata sch ON fk.schema_id=sch.id");
  m_target_def.add_from(
      "JOIN mysql.columns col ON tbl.id=col.table_id "
      "AND fk_col.column_id=col.id");

  m_target_def.add_where("NOT tbl.type = 'VIEW'");
  m_target_def.add_where("AND tbl.hidden = 'Visible'");
  m_target_def.add_where("AND tbl.se_private_id IS NOT NULL");
  m_target_def.add_where("AND tbl.engine='INNODB'");
}

}  // namespace system_views
}  // namespace dd
