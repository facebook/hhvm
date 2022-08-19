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

#include "sql/dd/impl/system_views/parameters.h"

namespace dd {
namespace system_views {

const Parameters &Parameters::instance() {
  static Parameters *s_instance = new Parameters();
  return *s_instance;
}

Parameters::Parameters() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_SPECIFIC_CATALOG, "SPECIFIC_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_SPECIFIC_SCHEMA, "SPECIFIC_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_SPECIFIC_NAME, "SPECIFIC_NAME", "rtn.name");
  m_target_def.add_field(FIELD_ORDINAL_POSITION, "ORDINAL_POSITION",
                         "IF (rtn.type = 'FUNCTION', prm.ordinal_position-1, "
                         "prm.ordinal_position)");
  m_target_def.add_field(FIELD_PARAMETER_MODE, "PARAMETER_MODE",
                         "IF (rtn.type = 'FUNCTION' AND prm.ordinal_position = "
                         "1, NULL, prm.mode)");
  m_target_def.add_field(FIELD_PARAMETER_NAME, "PARAMETER_NAME",
                         "IF (rtn.type = 'FUNCTION' AND prm.ordinal_position = "
                         "1, NULL, prm.name)");
  m_target_def.add_field(
      FIELD_DATA_TYPE, "DATA_TYPE",
      "SUBSTRING_INDEX(SUBSTRING_INDEX(prm.data_type_utf8, '(', 1), ' ', 1)");
  m_target_def.add_field(
      FIELD_CHARACTER_MAXIMUM_LENGTH, "CHARACTER_MAXIMUM_LENGTH",
      "INTERNAL_DD_CHAR_LENGTH(prm.data_type, prm.char_length, col.name, 0)");
  m_target_def.add_field(
      FIELD_CHARACTER_OCTET_LENGTH, "CHARACTER_OCTET_LENGTH",
      "INTERNAL_DD_CHAR_LENGTH(prm.data_type, prm.char_length, col.name, 1)");
  m_target_def.add_field(FIELD_NUMERIC_PRECISION, "NUMERIC_PRECISION",
                         "prm.numeric_precision");
  m_target_def.add_field(
      FIELD_NUMERIC_SCALE, "NUMERIC_SCALE",
      "IF(ISNULL(prm.numeric_precision), NULL, IFNULL(prm.numeric_scale, 0))");
  m_target_def.add_field(FIELD_DATETIME_PRECISION, "DATETIME_PRECISION",
                         "prm.datetime_precision");
  m_target_def.add_field(FIELD_CHARACTER_SET_NAME, "CHARACTER_SET_NAME",
                         "CASE prm.data_type"
                         "  WHEN 'MYSQL_TYPE_STRING' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_VAR_STRING' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_VARCHAR' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_TINY_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_MEDIUM_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_LONG_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_ENUM' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  WHEN 'MYSQL_TYPE_SET' THEN "
                         "    (IF (cs.name='binary',NULL, cs.name))"
                         "  ELSE NULL END");
  m_target_def.add_field(FIELD_COLLATION_NAME, "COLLATION_NAME",
                         "CASE prm.data_type"
                         "  WHEN 'MYSQL_TYPE_STRING' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_VAR_STRING' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_VARCHAR' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_TINY_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_MEDIUM_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_LONG_BLOB' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_ENUM' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  WHEN 'MYSQL_TYPE_SET' THEN "
                         "    (IF (cs.name='binary',NULL, col.name))"
                         "  ELSE NULL END");
  m_target_def.add_field(FIELD_DTD_IDENTIFIER, "DTD_IDENTIFIER",
                         "prm.data_type_utf8");
  m_target_def.add_field(FIELD_ROUTINE_TYPE, "ROUTINE_TYPE", "rtn.type");

  m_target_def.add_from(
      "mysql.parameters prm JOIN mysql.routines rtn "
      "ON prm.routine_id=rtn.id");
  m_target_def.add_from("JOIN mysql.schemata sch ON rtn.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from("JOIN mysql.collations col ON prm.collation_id=col.id");
  m_target_def.add_from(
      "JOIN mysql.character_sets cs "
      "ON col.character_set_id=cs.id");

  m_target_def.add_where(
      "CAN_ACCESS_ROUTINE(sch.name, rtn.name, rtn.type, "
      "rtn.definer, FALSE)");
}

}  // namespace system_views
}  // namespace dd
