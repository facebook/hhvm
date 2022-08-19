/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/st_geometry_columns.h"

namespace dd {
namespace system_views {

const St_geometry_columns &St_geometry_columns::instance() {
  static St_geometry_columns *s_instance = new St_geometry_columns();
  return *s_instance;
}

St_geometry_columns::St_geometry_columns() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG",
                         "cols.TABLE_CATALOG");
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "cols.TABLE_SCHEMA");
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "cols.TABLE_NAME");
  m_target_def.add_field(FIELD_COLUMN_NAME, "COLUMN_NAME", "cols.COLUMN_NAME");
  m_target_def.add_field(FIELD_SRS_NAME, "SRS_NAME", "srs.SRS_NAME");
  m_target_def.add_field(FIELD_SRS_ID, "SRS_ID", "cols.SRS_ID");
  m_target_def.add_field(FIELD_GEOMETRY_TYPE_NAME, "GEOMETRY_TYPE_NAME",
                         "cols.DATA_TYPE");

  m_target_def.add_from("INFORMATION_SCHEMA.COLUMNS cols");
  m_target_def.add_from(
      "LEFT JOIN "
      "  INFORMATION_SCHEMA.ST_SPATIAL_REFERENCE_SYSTEMS srs "
      "  ON (cols.SRS_ID = srs.SRS_ID)");

  m_target_def.add_where(
      "DATA_TYPE IN ('geometry','point','linestring','polygon', 'multipoint',"
      "              'multilinestring', 'multipolygon','geomcollection')");
}

}  // namespace system_views
}  // namespace dd
