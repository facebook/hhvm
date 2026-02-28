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

#include "sql/dd/impl/system_views/innodb_fields.h"

namespace dd {
namespace system_views {

const Innodb_fields &Innodb_fields::instance() {
  static Innodb_fields *s_instance = new Innodb_fields();
  return *s_instance;
}

Innodb_fields::Innodb_fields() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_INDEX_ID, "INDEX_ID",
      "GET_DD_INDEX_PRIVATE_DATA(idx.se_private_data, 'id')");
  m_target_def.add_field(FIELD_NAME, "NAME", "col.name");
  m_target_def.add_field(FIELD_POS, "POS", "fld.ordinal_position - 1");

  m_target_def.add_from("mysql.index_column_usage fld");
  m_target_def.add_from("JOIN mysql.columns col ON fld.column_id=col.id");
  m_target_def.add_from("JOIN mysql.indexes idx ON fld.index_id=idx.id");
  m_target_def.add_from("JOIN mysql.tables tbl ON tbl.id=idx.table_id");

  m_target_def.add_where("NOT tbl.type = 'VIEW'");
  m_target_def.add_where("AND tbl.hidden = 'Visible'");
  m_target_def.add_where("AND NOT fld.hidden");
  m_target_def.add_where("AND tbl.se_private_id IS NOT NULL");
  m_target_def.add_where("AND tbl.engine='INNODB'");
}

}  // namespace system_views
}  // namespace dd
