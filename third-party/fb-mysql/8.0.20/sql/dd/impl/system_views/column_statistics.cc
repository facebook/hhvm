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

#include "sql/dd/impl/system_views/column_statistics.h"

namespace dd {
namespace system_views {

const Column_statistics &Column_statistics::instance() {
  static Column_statistics *s_instance = new Column_statistics();
  return *s_instance;
}

Column_statistics::Column_statistics() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_SCHEMA_NAME, "SCHEMA_NAME", "SCHEMA_NAME");
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "TABLE_NAME");
  m_target_def.add_field(FIELD_COLUMN_NAME, "COLUMN_NAME", "COLUMN_NAME");
  m_target_def.add_field(FIELD_HISTOGRAM, "HISTOGRAM", "HISTOGRAM");
  m_target_def.add_from("mysql.column_statistics");
  m_target_def.add_where("CAN_ACCESS_TABLE(SCHEMA_NAME, TABLE_NAME)");
}

}  // namespace system_views
}  // namespace dd
