/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/table_stats.h"

#include "sql/dd/impl/raw/object_keys.h"       // Composite_char_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

Table_stats::Table_stats() {
  m_target_def.set_table_name("table_stats");

  m_target_def.add_field(FIELD_SCHEMA_NAME, "FIELD_SCHEMA_NAME",
                         "schema_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_TABLE_NAME, "FIELD_TABLE_NAME",
                         "table_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_TABLE_ROWS, "FIELD_TABLE_ROWS",
                         "table_rows BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_AVG_ROW_LENGTH, "FIELD_AVG_ROW_LENGTH",
                         "avg_row_length BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_DATA_LENGTH, "FIELD_DATA_LENGTH",
                         "data_length BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_MAX_DATA_LENGTH, "FIELD_MAX_DATA_LENGTH",
                         "max_data_length BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_INDEX_LENGTH, "FIELD_INDEX_LENGTH",
                         "index_length BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_DATA_FREE, "FIELD_DATA_FREE",
                         "data_free BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_AUTO_INCREMENT, "FIELD_AUTO_INCREMENT",
                         "auto_increment BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_CHECKSUM, "FIELD_CHECKSUM",
                         "checksum BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_UPDATE_TIME, "FIELD_UPDATE_TIME",
                         "update_time TIMESTAMP NULL");
  m_target_def.add_field(FIELD_CHECK_TIME, "FIELD_CHECK_TIME",
                         "check_time TIMESTAMP NULL");
  m_target_def.add_field(FIELD_CACHED_TIME, "FIELD_CACHED_TIME",
                         "cached_time TIMESTAMP NOT NULL");

  m_target_def.add_index(INDEX_PK_SCHEMA_ID_TABLE_NAME,
                         "INDEX_PK_SCHEMA_ID_TABLE_NAME",
                         "PRIMARY KEY (schema_name, table_name)");
}

///////////////////////////////////////////////////////////////////////////

const Table_stats &Table_stats::instance() {
  static Table_stats *s_instance = new Table_stats();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Table_stat::Name_key *Table_stats::create_object_key(
    const String_type &schema_name, const String_type &table_name) {
  return new (std::nothrow)
      Composite_char_key(INDEX_PK_SCHEMA_ID_TABLE_NAME, FIELD_SCHEMA_NAME,
                         schema_name, FIELD_TABLE_NAME, table_name);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
