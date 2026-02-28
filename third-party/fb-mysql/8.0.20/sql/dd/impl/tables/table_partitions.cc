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

#include "sql/dd/impl/tables/table_partitions.h"

#include <memory>
#include <new>

#include "my_dbug.h"
#include "sql/dd/impl/object_key.h"
#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/raw/raw_record.h"        // dd::Raw_record
#include "sql/dd/impl/raw/raw_table.h"         // dd::Raw_table
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/transaction_impl.h"      // dd::Transaction_ro
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/types/table.h"
#include "sql/handler.h"

namespace dd {
namespace tables {

const Table_partitions &Table_partitions::instance() {
  static Table_partitions *s_instance = new Table_partitions();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Table_partitions::name_collation() {
  return &my_charset_utf8_tolower_ci;
}

///////////////////////////////////////////////////////////////////////////

Table_partitions::Table_partitions() {
  m_target_def.set_table_name("table_partitions");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_TABLE_ID, "FIELD_TABLE_ID",
                         "table_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_PARENT_PARTITION_ID, "FIELD_PARENT_PARTITION_ID",
                         "parent_partition_id BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_NUMBER, "FIELD_NUMBER",
                         "number SMALLINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_ENGINE, "FIELD_ENGINE",
                         "engine VARCHAR(64) NOT NULL COLLATE utf8_general_ci");
  m_target_def.add_field(FIELD_DESCRIPTION_UTF8, "FIELD_DESCRIPTION_UTF8",
                         "description_utf8 TEXT");
  m_target_def.add_field(FIELD_COMMENT, "FIELD_COMMENT",
                         "comment VARCHAR(2048) NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");
  m_target_def.add_field(FIELD_SE_PRIVATE_ID, "FIELD_SE_PRIVATE_ID",
                         "se_private_id BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_TABLESPACE_ID, "FIELD_TABLESPACE_ID",
                         "tablespace_id BIGINT UNSIGNED");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_TABLE_ID_NAME, "INDEX_UK_TABLE_ID_NAME",
                         "UNIQUE KEY(table_id, name)");
  m_target_def.add_index(INDEX_UK_TABLE_ID_PARENT_PARTITION_ID_NUMBER,
                         "INDEX_UK_TABLE_ID_PARENT_PARTITION_ID_NUMBER",
                         "UNIQUE KEY(table_id, parent_partition_id, number)");
  m_target_def.add_index(INDEX_UK_ENGINE_SE_PRIVATE_ID,
                         "INDEX_UK_ENGINE_SE_PRIVATE_ID",
                         "UNIQUE KEY(engine, se_private_id)");
  m_target_def.add_index(INDEX_K_ENGINE, "INDEX_K_ENGINE", "KEY(engine)");
  m_target_def.add_index(INDEX_K_TABLESPACE_ID, "INDEX_K_TABLESPACE_ID",
                         "KEY(tablespace_id)");
  m_target_def.add_index(INDEX_K_PARENT_PARTITION_ID,
                         "INDEX_K_PARENT_PARTITION_ID",
                         "KEY(parent_partition_id)");

  m_target_def.add_foreign_key(FK_TABLE_ID, "FK_TABLE_ID",
                               "FOREIGN KEY (table_id) REFERENCES "
                               "tables(id)");
  m_target_def.add_foreign_key(FK_TABLESPACE_ID, "FK_TABLESPACE_ID",
                               "FOREIGN KEY (tablespace_id) REFERENCES "
                               "tablespaces(id)");
  m_target_def.add_foreign_key(FK_PARENT_PARTITION_ID, "FK_PARENT_PARTITION_ID",
                               "FOREIGN KEY (parent_partition_id) REFERENCES "
                               "table_partitions(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Table_partitions::create_key_by_table_id(Object_id table_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_TABLE_ID_NAME, FIELD_TABLE_ID, table_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Table_partitions::create_key_by_parent_partition_id(
    Object_id table_id, Object_id parent_partition_id) {
  return new (std::nothrow) Sub_partition_range_key(
      INDEX_UK_TABLE_ID_PARENT_PARTITION_ID_NUMBER, FIELD_TABLE_ID, table_id,
      FIELD_PARENT_PARTITION_ID, parent_partition_id);
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Object_id Table_partitions::read_table_id(const Raw_record &r) {
  return r.read_uint(Table_partitions::FIELD_TABLE_ID, -1);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Object_key *Table_partitions::create_se_private_key(const String_type &engine,
                                                    Object_id se_private_id) {
  return new (std::nothrow)
      Se_private_id_key(INDEX_UK_ENGINE_SE_PRIVATE_ID, FIELD_ENGINE, engine,
                        FIELD_SE_PRIVATE_ID, se_private_id);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

bool Table_partitions::get_partition_table_id(THD *thd,
                                              const String_type &engine,
                                              ulonglong se_private_id,
                                              Object_id *oid) {
  DBUG_TRACE;

  DBUG_ASSERT(oid);
  *oid = INVALID_OBJECT_ID;

  Transaction_ro trx(thd, ISO_READ_COMMITTED);
  trx.otx.register_tables<dd::Table>();
  if (trx.otx.open_tables()) return true;

  const std::unique_ptr<Object_key> k(
      create_se_private_key(engine, se_private_id));

  Raw_table *t = trx.otx.get_table(instance().name());
  DBUG_ASSERT(t);

  // Find record by the object-key.
  std::unique_ptr<Raw_record> r;
  if (t->find_record(*k, r)) {
    return true;
  }

  if (r.get()) *oid = read_table_id(*r.get());

  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
