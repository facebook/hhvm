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

#include "sql/dd/impl/tables/routines.h"

#include <new>

#include "sql/dd/dd.h"                         // dd::create_object
#include "sql/dd/impl/raw/object_keys.h"       // dd::Routine_name_key
#include "sql/dd/impl/raw/raw_record.h"        // dd::Raw_record
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/types/function.h"   // dd::Function
#include "sql/dd/types/procedure.h"  // dd::Procedure

namespace dd {
namespace tables {

const Routines &Routines::instance() {
  static Routines *s_instance = new Routines();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Routines::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Routines::Routines() {
  m_target_def.set_table_name("routines");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_SCHEMA_ID, "FIELD_SCHEMA_ID",
                         "schema_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_TYPE, "FIELD_TYPE",
                         "type ENUM('FUNCTION', 'PROCEDURE') NOT NULL");
  m_target_def.add_field(FIELD_RESULT_DATA_TYPE, "FIELD_RESULT_DATA_TYPE",
                         "result_data_type ENUM(\n"
                         "    'MYSQL_TYPE_DECIMAL', 'MYSQL_TYPE_TINY',\n"
                         "    'MYSQL_TYPE_SHORT',  'MYSQL_TYPE_LONG',\n"
                         "    'MYSQL_TYPE_FLOAT',  'MYSQL_TYPE_DOUBLE',\n"
                         "    'MYSQL_TYPE_NULL', 'MYSQL_TYPE_TIMESTAMP',\n"
                         "    'MYSQL_TYPE_LONGLONG','MYSQL_TYPE_INT24',\n"
                         "    'MYSQL_TYPE_DATE',   'MYSQL_TYPE_TIME',\n"
                         "    'MYSQL_TYPE_DATETIME', 'MYSQL_TYPE_YEAR',\n"
                         "    'MYSQL_TYPE_NEWDATE', 'MYSQL_TYPE_VARCHAR',\n"
                         "    'MYSQL_TYPE_BIT', 'MYSQL_TYPE_TIMESTAMP2',\n"
                         "    'MYSQL_TYPE_DATETIME2', 'MYSQL_TYPE_TIME2',\n"
                         "    'MYSQL_TYPE_NEWDECIMAL', 'MYSQL_TYPE_ENUM',\n"
                         "    'MYSQL_TYPE_SET', 'MYSQL_TYPE_TINY_BLOB',\n"
                         "    'MYSQL_TYPE_MEDIUM_BLOB', "
                         "    'MYSQL_TYPE_LONG_BLOB', 'MYSQL_TYPE_BLOB',\n"
                         "    'MYSQL_TYPE_VAR_STRING',\n"
                         "    'MYSQL_TYPE_STRING', 'MYSQL_TYPE_GEOMETRY',\n"
                         "    'MYSQL_TYPE_JSON'\n"
                         "  ) DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_DATA_TYPE_UTF8,
                         "FIELD_RESULT_DATA_TYPE_UTF8",
                         "result_data_type_utf8 MEDIUMTEXT NOT NULL");
  m_target_def.add_field(FIELD_RESULT_IS_ZEROFILL, "FIELD_RESULT_IS_ZEROFILL",
                         "result_is_zerofill BOOL DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_IS_UNSIGNED, "FIELD_RESULT_IS_UNSIGNED",
                         "result_is_unsigned BOOL DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_CHAR_LENGTH, "FIELD_RESULT_CHAR_LENGTH",
                         "result_char_length INT UNSIGNED DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_NUMERIC_PRECISION,
                         "FIELD_RESULT_NUMERIC_PRECISION",
                         "result_numeric_precision INT UNSIGNED DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_NUMERIC_SCALE,
                         "FIELD_RESULT_NUMERIC_SCALE",
                         "result_numeric_scale INT UNSIGNED DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_DATETIME_PRECISION,
                         "FIELD_RESULT_DATETIME_PRECISION",
                         "result_datetime_precision INT UNSIGNED DEFAULT NULL");
  m_target_def.add_field(FIELD_RESULT_COLLATION_ID, "FIELD_RESULT_COLLATION_ID",
                         "result_collation_id BIGINT UNSIGNED DEFAULT NULL");
  m_target_def.add_field(FIELD_DEFINITION, "FIELD_DEFINITION",
                         "definition LONGBLOB");
  m_target_def.add_field(FIELD_DEFINITION_UTF8, "FIELD_DEFINITION_UTF8",
                         "definition_utf8 LONGTEXT");
  m_target_def.add_field(FIELD_PARAMETER_STR, "FIELD_PARAMETER_STR",
                         "parameter_str BLOB");
  m_target_def.add_field(FIELD_IS_DETERMINISTIC, "FIELD_IS_DETERMINISTIC",
                         "is_deterministic BOOL NOT NULL");
  m_target_def.add_field(FIELD_SQL_DATA_ACCESS, "FIELD_SQL_DATA_ACCESS",
                         "sql_data_access ENUM('CONTAINS SQL', 'NO SQL',\n"
                         "     'READS SQL DATA',\n"
                         "     'MODIFIES SQL DATA') NOT NULL");
  m_target_def.add_field(
      FIELD_SECURITY_TYPE, "FIELD_SECURITY_TYPE",
      "security_type ENUM('DEFAULT', 'INVOKER', 'DEFINER') NOT NULL");
  m_target_def.add_field(FIELD_DEFINER, "FIELD_DEFINER",
                         "definer VARCHAR(288) NOT NULL");
  m_target_def.add_sql_mode_field(FIELD_SQL_MODE, "FIELD_SQL_MODE");
  m_target_def.add_field(FIELD_CLIENT_COLLATION_ID, "FIELD_CLIENT_COLLATION_ID",
                         "client_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_CONNECTION_COLLATION_ID,
                         "FIELD_CONNECTION_COLLATION_ID",
                         "connection_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_SCHEMA_COLLATION_ID, "FIELD_SCHEMA_COLLATION_ID",
                         "schema_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_CREATED, "FIELD_CREATED",
                         "created TIMESTAMP NOT NULL");
  m_target_def.add_field(FIELD_LAST_ALTERED, "FIELD_LAST_ALTERED",
                         "last_altered TIMESTAMP NOT NULL");
  m_target_def.add_field(FIELD_COMMENT, "FIELD_COMMENT",
                         "comment TEXT NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(
      FIELD_EXTERNAL_LANGUAGE, "FIELD_EXTERNAL_LANGUAGE",
      "external_language VARCHAR(64) NOT NULL DEFAULT 'SQL'");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_SCHEMA_ID_TYPE_NAME,
                         "INDEX_UK_SCHEMA_ID_TYPE_NAME",
                         "UNIQUE KEY(schema_id, type, name)");
  m_target_def.add_index(INDEX_K_RESULT_COLLATION_ID,
                         "INDEX_K_RESULT_COLLATION_ID",
                         "KEY(result_collation_id)");
  m_target_def.add_index(INDEX_K_CLIENT_COLLATION_ID,
                         "INDEX_K_CLIENT_COLLATION_ID",
                         "KEY(client_collation_id)");
  m_target_def.add_index(INDEX_K_CONNECTION_COLLATION_ID,
                         "INDEX_K_CONNECTION_COLLATION_ID",
                         "KEY(connection_collation_id)");
  m_target_def.add_index(INDEX_K_SCHEMA_COLLATION_ID,
                         "INDEX_K_SCHEMA_COLLATION_ID",
                         "KEY(schema_collation_id)");

  m_target_def.add_foreign_key(FK_SCHEMA_ID, "FK_SCHEMA_ID",
                               "FOREIGN KEY (schema_id) "
                               "REFERENCES schemata(id)");
  m_target_def.add_foreign_key(FK_RESULT_COLLATION_ID, "FK_RESULT_COLLATION_ID",
                               "FOREIGN KEY (result_collation_id) "
                               "REFERENCES collations(id)");
  m_target_def.add_foreign_key(FK_CLIENT_COLLATION_ID, "FK_CLIENT_COLLATION_ID",
                               "FOREIGN KEY (client_collation_id) "
                               "REFERENCES collations(id)");
  m_target_def.add_foreign_key(FK_CONNECTION_COLLATION_ID,
                               "FK_CONNECTION_COLLATION_ID",
                               "FOREIGN KEY (connection_collation_id) "
                               "REFERENCES collations(id)");
  m_target_def.add_foreign_key(FK_SCHEMA_COLLATION_ID, "FK_SCHEMA_COLLATION_ID",
                               "FOREIGN KEY (schema_collation_id) "
                               "REFERENCES collations(id)");
}

///////////////////////////////////////////////////////////////////////////

Routine *Routines::create_entity_object(const Raw_record &r) const {
  Routine::enum_routine_type routine_type =
      (Routine::enum_routine_type)r.read_int(FIELD_TYPE);

  if (routine_type == Routine::RT_FUNCTION)
    return dd::create_object<Function>();
  else
    return dd::create_object<Procedure>();
}

///////////////////////////////////////////////////////////////////////////

bool Routines::update_object_key(Routine_name_key *key, Object_id schema_id,
                                 Routine::enum_routine_type type,
                                 const String_type &routine_name) {
  key->update(INDEX_UK_SCHEMA_ID_TYPE_NAME, FIELD_SCHEMA_ID, schema_id,
              FIELD_TYPE, type, FIELD_NAME, routine_name.c_str(),
              name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

Object_key *Routines::create_key_by_schema_id(Object_id schema_id) {
  return new (std::nothrow) Parent_id_range_key(INDEX_UK_SCHEMA_ID_TYPE_NAME,
                                                FIELD_SCHEMA_ID, schema_id);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
