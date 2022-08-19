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

#include "sql/dd/impl/system_views/routines.h"

namespace dd {
namespace system_views {

const Routines &Routines::instance() {
  static Routines *s_instance = new Routines();
  return *s_instance;
}

Routines::Routines() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_SPECIFIC_NAME, "SPECIFIC_NAME", "rtn.name");
  m_target_def.add_field(FIELD_ROUTINE_CATALOG, "ROUTINE_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_ROUTINE_SCHEMA, "ROUTINE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_ROUTINE_NAME, "ROUTINE_NAME", "rtn.name");
  m_target_def.add_field(FIELD_ROUTINE_TYPE, "ROUTINE_TYPE", "rtn.type");
  m_target_def.add_field(FIELD_DATA_TYPE, "DATA_TYPE",
                         "IF(rtn.type = 'PROCEDURE', '', "
                         "   SUBSTRING_INDEX(SUBSTRING_INDEX("
                         "     rtn.result_data_type_utf8, '(', 1), ' ', 1))");
  m_target_def.add_field(FIELD_CHARACTER_MAXIMUM_LENGTH,
                         "CHARACTER_MAXIMUM_LENGTH",
                         "INTERNAL_DD_CHAR_LENGTH(rtn.result_data_type,"
                         "  rtn.result_char_length, coll_result.name, 0)");
  m_target_def.add_field(FIELD_CHARACTER_OCTET_LENGTH, "CHARACTER_OCTET_LENGTH",
                         "INTERNAL_DD_CHAR_LENGTH(rtn.result_data_type,"
                         "  rtn.result_char_length, coll_result.name, 1)");
  m_target_def.add_field(FIELD_NUMERIC_PRECISION, "NUMERIC_PRECISION",
                         "rtn.result_numeric_precision");
  m_target_def.add_field(FIELD_NUMERIC_SCALE, "NUMERIC_SCALE",
                         "rtn.result_numeric_scale");
  m_target_def.add_field(FIELD_DATETIME_PRECISION, "DATETIME_PRECISION",
                         "rtn.result_datetime_precision");
  m_target_def.add_field(
      FIELD_CHARACTER_SET_NAME, "CHARACTER_SET_NAME",
      "CASE rtn.result_data_type"
      "  WHEN 'MYSQL_TYPE_STRING' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_VAR_STRING' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_VARCHAR' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_TINY_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_MEDIUM_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_LONG_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_ENUM' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  WHEN 'MYSQL_TYPE_SET' THEN "
      "    (IF (cs_result.name='binary',NULL, cs_result.name))"
      "  ELSE NULL END");
  m_target_def.add_field(
      FIELD_COLLATION_NAME, "COLLATION_NAME",
      "CASE rtn.result_data_type"
      "  WHEN 'MYSQL_TYPE_STRING' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_VAR_STRING' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_VARCHAR' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_TINY_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_MEDIUM_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_LONG_BLOB' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_ENUM' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  WHEN 'MYSQL_TYPE_SET' THEN "
      "    (IF (cs_result.name='binary',NULL, coll_result.name))"
      "  ELSE NULL END");
  m_target_def.add_field(
      FIELD_DTD_IDENTIFIER, "DTD_IDENTIFIER",
      "IF(rtn.type = 'PROCEDURE', NULL, rtn.result_data_type_utf8)");
  m_target_def.add_field(FIELD_ROUTINE_BODY, "ROUTINE_BODY", "'SQL'");
  m_target_def.add_field(
      FIELD_ROUTINE_DEFINITION, "ROUTINE_DEFINITION",
      "IF (CAN_ACCESS_ROUTINE(sch.name, rtn.name, rtn.type, rtn.definer, TRUE),"
      "    rtn.definition_utf8, NULL)");
  m_target_def.add_field(FIELD_EXTERNAL_NAME, "EXTERNAL_NAME", "NULL");
  m_target_def.add_field(FIELD_EXTERNAL_LANGUAGE, "EXTERNAL_LANGUAGE",
                         "rtn.external_language");
  m_target_def.add_field(FIELD_PARAMETER_STYLE, "PARAMETER_STYLE", "'SQL'");
  m_target_def.add_field(FIELD_IS_DETERMINISTIC, "IS_DETERMINISTIC",
                         "IF(rtn.is_deterministic=0, 'NO', 'YES')");
  m_target_def.add_field(FIELD_SQL_DATA_ACCESS, "SQL_DATA_ACCESS",
                         "rtn.sql_data_access");
  m_target_def.add_field(FIELD_SQL_PATH, "SQL_PATH", "NULL");
  m_target_def.add_field(FIELD_SECURITY_TYPE, "SECURITY_TYPE",
                         "rtn.security_type");
  m_target_def.add_field(FIELD_CREATED, "CREATED", "rtn.created");
  m_target_def.add_field(FIELD_LAST_ALTERED, "LAST_ALTERED",
                         "rtn.last_altered");
  m_target_def.add_field(FIELD_SQL_MODE, "SQL_MODE", "rtn.sql_mode");
  m_target_def.add_field(FIELD_ROUTINE_COMMENT, "ROUTINE_COMMENT",
                         "rtn.comment");
  m_target_def.add_field(FIELD_DEFINER, "DEFINER", "rtn.definer");
  m_target_def.add_field(FIELD_CHARACTER_SET_CLIENT, "CHARACTER_SET_CLIENT",
                         "cs_client.name");
  m_target_def.add_field(FIELD_COLLATION_CONNECTION, "COLLATION_CONNECTION",
                         "coll_conn.name");
  m_target_def.add_field(FIELD_DATABASE_COLLATION, "DATABASE_COLLATION",
                         "coll_db.name");

  m_target_def.add_from("mysql.routines rtn");
  m_target_def.add_from("JOIN mysql.schemata sch ON rtn.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_client "
      "ON coll_client.id=rtn.client_collation_id");
  m_target_def.add_from(
      "JOIN mysql.character_sets cs_client "
      "ON cs_client.id=coll_client.character_set_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_conn "
      "ON coll_conn.id=rtn.connection_collation_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_db "
      "ON coll_db.id=rtn.schema_collation_id");
  m_target_def.add_from(
      "LEFT JOIN mysql.collations coll_result "
      "ON coll_result.id=rtn.result_collation_id");
  m_target_def.add_from(
      "LEFT JOIN mysql.character_sets cs_result "
      "ON cs_result.id=coll_result.character_set_id");

  m_target_def.add_where(
      "CAN_ACCESS_ROUTINE(sch.name, rtn.name, rtn.type, "
      "rtn.definer, FALSE)");
}

}  // namespace system_views
}  // namespace dd
