/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/index_stats.h"

#include "sql/dd/impl/raw/object_keys.h"
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

Index_stats::Index_stats() {
  m_target_def.set_table_name("index_stats");

  m_target_def.add_field(FIELD_SCHEMA_NAME, "FIELD_SCHEMA_NAME",
                         "schema_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_TABLE_NAME, "FIELD_TABLE_NAME",
                         "table_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_INDEX_NAME, "FIELD_INDEX_NAME",
                         "index_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_COLUMN_NAME, "FIELD_COLUMN_NAME",
                         "column_name VARCHAR(64) NOT NULL");
  m_target_def.add_field(FIELD_CARDINALITY, "FIELD_CARDINALITY",
                         "cardinality BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_CACHED_TIME, "FIELD_CACHED_TIME",
                         "cached_time TIMESTAMP NOT NULL");

  m_target_def.add_index(INDEX_UK_SCHEMA_TABLE_INDEX_COLUMN,
                         "INDEX_UK_SCHEMA_TABLE_INDEX_COLUMN",
                         "UNIQUE KEY (schema_name, table_name, "
                         "index_name, column_name)");
}

///////////////////////////////////////////////////////////////////////////

const Index_stats &Index_stats::instance() {
  static Index_stats *s_instance = new Index_stats();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Index_stat::Name_key *Index_stats::create_object_key(
    const String_type &schema_name, const String_type &table_name,
    const String_type &index_name, const String_type &column_name) {
  return new (std::nothrow) Composite_4char_key(
      INDEX_UK_SCHEMA_TABLE_INDEX_COLUMN, FIELD_SCHEMA_NAME, schema_name,
      FIELD_TABLE_NAME, table_name, FIELD_INDEX_NAME, index_name,
      FIELD_COLUMN_NAME, column_name);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Index_stats::create_range_key_by_table_name(
    const String_type &schema_name, const String_type &table_name) {
  return new (std::nothrow) Index_stat_range_key(
      INDEX_UK_SCHEMA_TABLE_INDEX_COLUMN, FIELD_SCHEMA_NAME, schema_name,
      FIELD_TABLE_NAME, table_name);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
