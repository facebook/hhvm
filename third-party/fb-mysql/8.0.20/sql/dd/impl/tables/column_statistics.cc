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

#include "sql/dd/impl/tables/column_statistics.h"

#include <new>
#include <string>

#include "sql/dd/impl/raw/object_keys.h"               // Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"          // TARGET_DD_VERSION
#include "sql/dd/impl/types/column_statistics_impl.h"  // Column_statistic_impl
#include "sql/dd/impl/types/object_table_definition_impl.h"  // Object_table_defi ...
#include "sql/mysqld.h"
#include "sql/stateless_allocator.h"

namespace dd {
class Raw_record;
}  // namespace dd

namespace dd {
namespace tables {

const Column_statistics &Column_statistics::instance() {
  static Column_statistics *s_instance = new Column_statistics();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Column_statistics::Column_statistics() {
  m_target_def.set_table_name("column_statistics");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_CATALOG_ID, "FIELD_CATALOG_ID",
                         "catalog_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(255) NOT NULL");
  m_target_def.add_field(
      FIELD_SCHEMA_NAME, "FIELD_SCHEMA_NAME",
      "schema_name VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_TABLE_NAME, "FIELD_TABLE_NAME",
      "table_name VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(FIELD_COLUMN_NAME, "FIELD_COLUMN_NAME",
                         "column_name VARCHAR(64) NOT NULL COLLATE \
                          utf8_tolower_ci");
  m_target_def.add_field(FIELD_HISTOGRAM, "FIELD_HISTOGRAM",
                         "histogram JSON NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
  m_target_def.add_index(INDEX_UK_CATALOG_ID_NAME, "INDEX_UK_CATALOG_ID_NAME",
                         "UNIQUE KEY (catalog_id, name)");
  m_target_def.add_index(
      INDEX_UK_CATALOG_ID_SCHEMA_NAME_TABLE_NAME_COLUMN_NAME,
      "INDEX_UK_CATALOG_ID_SCHEMA_NAME_TABLE_NAME_COLUMN_NAME",
      "UNIQUE KEY (catalog_id, schema_name, \
                                    table_name, column_name)");

  m_target_def.add_foreign_key(FK_CATALOG_ID, "FK_CATALOGS_ID",
                               "FOREIGN KEY (catalog_id) REFERENCES \
                                catalogs (id)");
}

///////////////////////////////////////////////////////////////////////////

dd::Column_statistics *Column_statistics::create_entity_object(
    const Raw_record &) const {
  return new (std::nothrow) Column_statistics_impl();
}

bool Column_statistics::update_object_key(Item_name_key *key,
                                          Object_id catalog_id,
                                          const String_type &name) {
  key->update(FIELD_CATALOG_ID, catalog_id, FIELD_NAME, name,
              Object_table_definition_impl::name_collation());
  return false;
}

}  // namespace tables
}  // namespace dd
