/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/schemata.h"

#include <new>
#include <string>

#include "mysql_com.h"
#include "sql/dd/impl/raw/object_keys.h"  // Parent_id_range_key
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/impl/types/schema_impl.h"  // dd::Schema_impl
#include "sql/dd/string_type.h"             // dd::String_type
#include "sql/mysqld.h"
#include "sql/stateless_allocator.h"

namespace dd {
namespace tables {

const Schemata &Schemata::instance() {
  static Schemata *s_instance = new Schemata();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Schemata::name_collation() {
  return Object_table_definition_impl::fs_name_collation();
}

///////////////////////////////////////////////////////////////////////////

Schemata::Schemata() {
  m_target_def.set_table_name("schemata");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_CATALOG_ID, "FIELD_CATALOG_ID",
                         "catalog_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_DEFAULT_COLLATION_ID,
                         "FIELD_DEFAULT_COLLATION_ID",
                         "default_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_CREATED, "FIELD_CREATED",
                         "created TIMESTAMP NOT NULL");
  m_target_def.add_field(FIELD_LAST_ALTERED, "FIELD_LAST_ALTERED",
                         "last_altered TIMESTAMP NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(FIELD_DEFAULT_ENCRYPTION, "FIELD_DEFAULT_ENCRYPTION",
                         "default_encryption ENUM('NO', 'YES') NOT NULL");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
  m_target_def.add_index(INDEX_UK_CATALOG_ID_NAME, "INDEX_UK_CATALOG_ID_NAME",
                         "UNIQUE KEY (catalog_id, name)");
  m_target_def.add_index(INDEX_K_DEFAULT_COLLATION_ID,
                         "INDEX_K_DEFAULT_COLLATION_ID",
                         "KEY (default_collation_id)");

  m_target_def.add_foreign_key(FK_CATALOG_ID, "FK_CATALOG_ID",
                               "FOREIGN KEY (catalog_id) REFERENCES \
                                catalogs(id)");
  m_target_def.add_foreign_key(FK_DEFAULT_COLLATION_ID,
                               "FK_DEFAULT_COLLATION_ID",
                               "FOREIGN KEY (default_collation_id) \
                                REFERENCES collations(id)");

  m_target_def.add_populate_statement(
      "INSERT INTO schemata (catalog_id, name, default_collation_id, created, "
      "last_altered, options, default_encryption, se_private_data) VALUES "
      "(1,'information_schema',33, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, "
      "NULL, 'NO', NULL)");
}

///////////////////////////////////////////////////////////////////////////

bool Schemata::update_object_key(Item_name_key *key, Object_id catalog_id,
                                 const String_type &schema_name) {
  key->update(FIELD_CATALOG_ID, catalog_id, FIELD_NAME, schema_name,
              name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

Schema *Schemata::create_entity_object(const Raw_record &) const {
  return new (std::nothrow) Schema_impl();
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Object_key *Schemata::create_key_by_catalog_id(Object_id catalog_id) {
  return new (std::nothrow) Parent_id_range_key(INDEX_UK_CATALOG_ID_NAME,
                                                FIELD_CATALOG_ID, catalog_id);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
