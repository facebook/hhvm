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

#include "sql/dd/impl/system_views/view_routine_usage.h"

#include <string>

#include "sql/stateless_allocator.h"

namespace dd {
namespace system_views {

const View_routine_usage &View_routine_usage::instance() {
  static View_routine_usage *s_instance = new View_routine_usage();
  return *s_instance;
}

View_routine_usage::View_routine_usage() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME",
                         "vw.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(
      FIELD_SPECIFIC_CATALOG, "SPECIFIC_CATALOG",
      "vru.routine_catalog" + m_target_def.fs_name_collation());
  m_target_def.add_field(
      FIELD_SPECIFIC_SCHEMA, "SPECIFIC_SCHEMA",
      "vru.routine_schema" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_SPECIFIC_NAME, "SPECIFIC_NAME",
                         "vru.routine_name");

  m_target_def.add_from("mysql.tables vw");
  m_target_def.add_from("JOIN mysql.schemata sch ON vw.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "JOIN mysql.view_routine_usage vru "
      "ON vru.view_id=vw.id");
  m_target_def.add_from(
      "JOIN mysql.routines rtn "
      "ON vru.routine_catalog= cat.name AND "
      "vru.routine_schema= sch.name AND "
      "vru.routine_name= rtn.name");

  m_target_def.add_where("vw.type = 'VIEW'");
  m_target_def.add_where(
      "AND CAN_ACCESS_ROUTINE(vru.routine_schema, "
      "vru.routine_name, rtn.type, rtn.definer, FALSE)");
  m_target_def.add_where(
      "AND CAN_ACCESS_VIEW(sch.name, vw.name, "
      "vw.view_definer, vw.options)");
}

}  // namespace system_views
}  // namespace dd
