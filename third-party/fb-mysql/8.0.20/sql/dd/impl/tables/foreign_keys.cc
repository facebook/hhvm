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

#include "sql/dd/impl/tables/foreign_keys.h"

#include <new>
#include <string>

#include "sql/dd/impl/raw/object_keys.h"   // Parent_id_range_key
#include "sql/dd/impl/raw/raw_record.h"    // dd::Raw_record
#include "sql/dd/impl/raw/raw_table.h"     // dd::Raw_table
#include "sql/dd/impl/transaction_impl.h"  // Transaction_ro
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/mysqld.h"
#include "sql/stateless_allocator.h"

namespace dd {
namespace tables {

const Foreign_keys &Foreign_keys::instance() {
  static Foreign_keys *s_instance = new Foreign_keys();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Foreign_keys::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Foreign_keys::Foreign_keys() {
  m_target_def.set_table_name("foreign_keys");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_SCHEMA_ID, "FIELD_SCHEMA_ID",
                         "schema_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_TABLE_ID, "FIELD_TABLE_ID",
                         "table_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(
      FIELD_UNIQUE_CONSTRAINT_NAME, "FIELD_UNIQUE_CONSTRAINT_NAME",
      "unique_constraint_name VARCHAR(64) COLLATE utf8_tolower_ci");
  m_target_def.add_field(FIELD_MATCH_OPTION, "FIELD_MATCH_OPTION",
                         "match_option ENUM('NONE', 'PARTIAL', 'FULL') "
                         "NOT NULL");
  m_target_def.add_field(FIELD_UPDATE_RULE, "FIELD_UPDATE_RULE",
                         "update_rule ENUM(\n"
                         "  'NO ACTION', 'RESTRICT',\n"
                         "  'CASCADE', 'SET NULL',\n"
                         "  'SET DEFAULT'\n"
                         ") NOT NULL");
  m_target_def.add_field(FIELD_DELETE_RULE, "FIELD_DELETE_RULE",
                         "delete_rule ENUM(\n"
                         "  'NO ACTION', 'RESTRICT',\n"
                         "  'CASCADE', 'SET NULL',\n"
                         "  'SET DEFAULT'\n"
                         ") NOT NULL");
  m_target_def.add_field(
      FIELD_REFERENCED_TABLE_CATALOG, "FIELD_REFERENCED_TABLE_CATALOG",
      "referenced_table_catalog "
      "VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_REFERENCED_TABLE_SCHEMA, "FIELD_REFERENCED_TABLE_SCHEMA",
      "referenced_table_schema "
      "VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_REFERENCED_TABLE, "FIELD_REFERENCED_TABLE",
      "referenced_table_name "
      "VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
  m_target_def.add_index(INDEX_UK_SCHEMA_ID_NAME, "INDEX_UK_SCHEMA_ID_NAME",
                         "UNIQUE KEY (schema_id, name)");
  m_target_def.add_index(INDEX_UK_TABLE_ID_NAME, "INDEX_UK_TABLE_ID_NAME",
                         "UNIQUE KEY (table_id, name)");
  m_target_def.add_index(INDEX_K_REF_CATALOG_REF_SCHEMA_REF_TABLE,
                         "INDEX_K_REF_CATALOG_REF_SCHEMA_REF_TABLE",
                         "KEY (referenced_table_catalog, "
                         "referenced_table_schema, referenced_table_name)");

  m_target_def.add_foreign_key(FK_SCHEMA_ID, "FK_SCHEMA_ID",
                               "FOREIGN KEY (schema_id) REFERENCES "
                               "schemata(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Foreign_keys::create_key_by_foreign_key_name(
    Object_id schema_id, const String_type &foreign_key_name) {
  return new (std::nothrow)
      Item_name_key(FIELD_SCHEMA_ID, schema_id, FIELD_NAME, foreign_key_name,
                    name_collation());
}

Object_key *Foreign_keys::create_key_by_table_id(Object_id table_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_TABLE_ID_NAME, FIELD_TABLE_ID, table_id);
}

Object_key *Foreign_keys::create_key_by_referenced_name(
    const String_type &referenced_catalog, const String_type &referenced_schema,
    const String_type &referenced_table) {
  return new (std::nothrow) Table_reference_range_key(
      INDEX_K_REF_CATALOG_REF_SCHEMA_REF_TABLE, FIELD_REFERENCED_TABLE_CATALOG,
      referenced_catalog, FIELD_REFERENCED_TABLE_SCHEMA, referenced_schema,
      FIELD_REFERENCED_TABLE, referenced_table);
}

///////////////////////////////////////////////////////////////////////////

bool Foreign_keys::check_foreign_key_exists(THD *thd, Object_id schema_id,
                                            const String_type &foreign_key_name,
                                            bool *exists) {
  DBUG_TRACE;

  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<dd::Foreign_key>();
  if (trx.otx.open_tables()) return true;

  const std::unique_ptr<Object_key> key(
      create_key_by_foreign_key_name(schema_id, foreign_key_name.c_str()));

  Raw_table *table = trx.otx.get_table(instance().name());
  DBUG_ASSERT(table != nullptr);

  // Find record by the object-key.
  std::unique_ptr<Raw_record> record;
  if (table->find_record(*key, record)) return true;

  if (record.get())
    *exists = true;
  else
    *exists = false;

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
