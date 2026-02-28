/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/triggers.h"

#include <memory>
#include <new>

#include "my_dbug.h"
#include "sql/dd/impl/object_key.h"
#include "sql/dd/impl/raw/object_keys.h"       // dd::Global_name_key
#include "sql/dd/impl/raw/raw_record.h"        // dd::Raw_record
#include "sql/dd/impl/raw/raw_table.h"         // dd::Raw_table
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/transaction_impl.h"      // Transaction_ro
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/types/table.h"
#include "sql/handler.h"

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Triggers::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Triggers::Triggers() {
  m_target_def.set_table_name("triggers");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_SCHEMA_ID, "FIELD_SCHEMA_ID",
                         "schema_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_EVENT_TYPE, "FIELD_EVENT_TYPE",
                         "event_type ENUM('INSERT', 'UPDATE', 'DELETE') "
                         "NOT NULL");
  m_target_def.add_field(FIELD_TABLE_ID, "FIELD_TABLE_ID",
                         "table_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_ACTION_TIMING, "FIELD_ACTION_TIMING",
                         "action_timing ENUM('BEFORE', 'AFTER') NOT NULL");
  m_target_def.add_field(FIELD_ACTION_ORDER, "FIELD_ACTION_ORDER",
                         "action_order INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_ACTION_STATEMENT, "FIELD_ACTION_STATEMENT",
                         "action_statement LONGBLOB NOT NULL");
  m_target_def.add_field(FIELD_ACTION_STATEMENT_UTF8,
                         "FIELD_ACTION_STATEMENT_UTF8",
                         "action_statement_utf8 LONGTEXT NOT NULL");
  m_target_def.add_field(FIELD_CREATED, "FIELD_CREATED",
                         "created TIMESTAMP(2) NOT NULL");
  m_target_def.add_field(FIELD_LAST_ALTERED, "FIELD_LAST_ALTERED",
                         "last_altered TIMESTAMP(2) NOT NULL");
  m_target_def.add_sql_mode_field(FIELD_SQL_MODE, "FIELD_SQL_MODE");
  m_target_def.add_field(FIELD_DEFINER, "FIELD_DEFINER",
                         "definer VARCHAR(288) NOT NULL");
  m_target_def.add_field(FIELD_CLIENT_COLLATION_ID, "FIELD_CLIENT_COLLATION_ID",
                         "client_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_CONNECTION_COLLATION_ID,
                         "FIELD_CONNECTION_COLLATION_ID",
                         "connection_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_SCHEMA_COLLATION_ID, "FIELD_SCHEMA_COLLATION_ID",
                         "schema_collation_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_SCHEMA_ID_NAME, "INDEX_UK_SCHEMA_ID_NAME",
                         "UNIQUE KEY (schema_id, name)");
  m_target_def.add_index(
      INDEX_UK_TABLE_ID_EVENT_TYPE_ACTION_TIMING_ACTION_ORDER,
      "INDEX_UK_TABLE_ID_EVENT_TYPE_ACTION_TIMING_ACTION_ORDER",
      "UNIQUE KEY (table_id, event_type, "
      "action_timing, action_order)");
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
  m_target_def.add_foreign_key(FK_TABLE_ID, "FK_TABLE_ID",
                               "FOREIGN KEY (table_id) "
                               "REFERENCES tables(id)");
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

/* purecov: begin deadcode */
Object_key *Triggers::create_key_by_schema_id(Object_id schema_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_SCHEMA_ID_NAME, FIELD_SCHEMA_ID, schema_id);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

Object_key *Triggers::create_key_by_table_id(Object_id table_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_UK_TABLE_ID_EVENT_TYPE_ACTION_TIMING_ACTION_ORDER, FIELD_TABLE_ID,
      table_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Triggers::create_key_by_trigger_name(Object_id schema_id,
                                                 const char *trigger_name) {
  return new (std::nothrow) Item_name_key(
      FIELD_SCHEMA_ID, schema_id, FIELD_NAME, trigger_name, name_collation());
}

///////////////////////////////////////////////////////////////////////////

Object_id Triggers::read_table_id(const Raw_record &r) {
  return r.read_uint(FIELD_TABLE_ID, -1);
}

///////////////////////////////////////////////////////////////////////////

bool Triggers::get_trigger_table_id(THD *thd, Object_id schema_id,
                                    const String_type &trigger_name,
                                    Object_id *oid) {
  DBUG_TRACE;

  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<dd::Table>();
  if (trx.otx.open_tables()) return true;

  DBUG_ASSERT(oid != nullptr);
  *oid = INVALID_OBJECT_ID;

  const std::unique_ptr<Object_key> key(
      create_key_by_trigger_name(schema_id, trigger_name.c_str()));

  Raw_table *table = trx.otx.get_table(instance().name());
  DBUG_ASSERT(table != nullptr);

  // Find record by the object-key.
  std::unique_ptr<Raw_record> record;
  if (table->find_record(*key, record)) return true;

  if (record.get()) *oid = read_table_id(*record.get());

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
