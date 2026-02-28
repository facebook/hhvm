/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/innodb_foreign.h"

namespace dd {
namespace system_views {

const Innodb_foreign &Innodb_foreign::instance() {
  static Innodb_foreign *s_instance = new Innodb_foreign();
  return *s_instance;
}

Innodb_foreign::Innodb_foreign() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_FOREIGN_ID, "ID",
      "CONCAT(sch.name, '/', fk.name)" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_FOREIGN_NAME, "FOR_NAME",
                         "CONCAT(sch.name, '/', tbl.name)");
  m_target_def.add_field(
      FIELD_REF_NAME, "REF_NAME",
      "CONCAT(fk.referenced_table_schema, '/', fk.referenced_table_name)");
  m_target_def.add_field(FIELD_N_COLS, "N_COLS", "COUNT(*)");
  m_target_def.add_field(FIELD_TYPE, "TYPE",
                         "IF(fk.delete_rule='CASCADE',1,0)|"
                         "IF(fk.delete_rule='SET NULL',2,0)|"
                         "IF(fk.update_rule='CASCADE',4,0)|"
                         "IF(fk.update_rule='SET NULL',8,0)|"
                         "IF(fk.delete_rule='NO ACTION',16,0)|"
                         "IF(fk.update_rule='NO ACTION',32,0)");

  m_target_def.add_from("mysql.foreign_keys fk");
  m_target_def.add_from("JOIN mysql.tables tbl ON fk.table_id=tbl.id");
  m_target_def.add_from("JOIN mysql.schemata sch ON fk.schema_id=sch.id");
  m_target_def.add_from(
      "JOIN mysql.foreign_key_column_usage col "
      "ON fk.id=col.foreign_key_id");

  m_target_def.add_where("NOT tbl.type = 'VIEW'");
  m_target_def.add_where("AND tbl.hidden = 'Visible'");
  m_target_def.add_where("AND tbl.se_private_id IS NOT NULL");
  m_target_def.add_where("AND tbl.engine='INNODB'");
  m_target_def.add_where("GROUP BY fk.id");
}

}  // namespace system_views
}  // namespace dd
