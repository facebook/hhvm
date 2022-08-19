/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/check_constraints.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"   // dd::Parent_id_range_key
#include "sql/dd/impl/raw/raw_record.h"    // dd::Raw_record
#include "sql/dd/impl/raw/raw_table.h"     // dd::Raw_table
#include "sql/dd/impl/transaction_impl.h"  // Transaction_ro
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

const Check_constraints &Check_constraints::instance() {
  static Check_constraints *s_instance = new (std::nothrow) Check_constraints();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Check_constraints::name_collation() {
  return &my_charset_utf8_tolower_ci;
}

///////////////////////////////////////////////////////////////////////////

Check_constraints::Check_constraints() {
  m_target_def.set_table_name("check_constraints");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_SCHEMA_ID, "FIELD_SCHEMA_ID",
                         "schema_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_TABLE_ID, "FIELD_TABLE_ID",
                         "table_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_ENFORCED, "FIELD_ENFORCED",
                         "enforced ENUM('NO', 'YES') NOT NULL");
  m_target_def.add_field(FIELD_CHECK_CLAUSE, "FIELD_CHECK_CLAUSE",
                         "check_clause LONGBLOB NOT NULL");
  m_target_def.add_field(FIELD_CHECK_CLAUSE_UTF8, "FIELD_CHECK_CLAUSE_UTF8",
                         "check_clause_utf8 LONGTEXT NOT NULL");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
  m_target_def.add_index(INDEX_UK_SCHEMA_ID_NAME, "INDEX_UK_SCHEMA_ID_NAME",
                         "UNIQUE KEY (schema_id, name)");
  m_target_def.add_index(INDEX_UK_TABLE_ID_NAME, "INDEX_UK_TABLE_ID_NAME",
                         "UNIQUE KEY (table_id, name)");

  m_target_def.add_foreign_key(FK_SCHEMA_ID, "FK_SCHEMA_ID",
                               "FOREIGN KEY (schema_id) REFERENCES "
                               "schemata(id)");
  m_target_def.add_foreign_key(FK_TABLE_ID, "FK_TABLE_ID",
                               "FOREIGN KEY (table_id) REFERENCES "
                               "tables(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Check_constraints::create_key_by_table_id(Object_id table_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_TABLE_ID_NAME, FIELD_TABLE_ID, table_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Check_constraints::create_key_by_check_constraint_name(
    Object_id schema_id, const String_type &check_cons_name) {
  return new (std::nothrow)
      Item_name_key(FIELD_SCHEMA_ID, schema_id, FIELD_NAME, check_cons_name,
                    name_collation());
}

///////////////////////////////////////////////////////////////////////////

bool Check_constraints::check_constraint_exists(
    THD *thd, Object_id schema_id, const String_type &check_cons_name,
    bool *exists) {
  DBUG_TRACE;

  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<dd::Check_constraint>();
  if (trx.otx.open_tables()) return true;

  const std::unique_ptr<Object_key> key(
      create_key_by_check_constraint_name(schema_id, check_cons_name.c_str()));

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
