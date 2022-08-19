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

#include "sql/dd/impl/tables/table_partition_values.h"

#include <new>
#include <sstream>  // std::stringstream

#include "my_base.h"  // HA_WHOLE_KEY
#include "mysql/udf_registration_types.h"
#include "sql/dd/impl/object_key.h"            // dd::Object_key
#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/raw/raw_key.h"           // dd::Raw_key
#include "sql/dd/impl/raw/raw_table.h"         // dd::Raw_table
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/field.h"           // Field
#include "sql/key.h"
#include "sql/table.h"

namespace dd {
namespace tables {

const Table_partition_values &Table_partition_values::instance() {
  static Table_partition_values *s_instance = new Table_partition_values();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Table_partition_values::Table_partition_values() {
  m_target_def.set_table_name("table_partition_values");

  m_target_def.add_field(FIELD_PARTITION_ID, "FIELD_PARTITION_ID",
                         "partition_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_LIST_NUM, "FIELD_LIST_NUM",
                         "list_num TINYINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_COLUMN_NUM, "FIELD_COLUMN_NUM",
                         "column_num TINYINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_VALUE_UTF8, "FIELD_VALUE_UTF8",
                         "value_utf8 TEXT NULL");
  m_target_def.add_field(FIELD_MAX_VALUE, "FIELD_MAX_VALUE",
                         "max_value BOOL NOT NULL");

  m_target_def.add_index(INDEX_PK_PARTITION_ID_LIST_NUM_COLUMN_NUM,
                         "INDEX_PK_PARTITION_ID_LIST_NUM_COLUMN_NUM",
                         "PRIMARY KEY(partition_id, list_num, column_num)");

  m_target_def.add_foreign_key(FK_TABLE_PARTITION_ID, "FK_TABLE_PARTITION_ID",
                               "FOREIGN KEY (partition_id) REFERENCES "
                               "table_partitions(id)");
}

///////////////////////////////////////////////////////////////////////////

// Primary key (PK) class for TABLE_PARTITION_VALUES table.

class Table_partition_values_pk : public Object_key {
 public:
  Table_partition_values_pk(int partition_id, int list_num, int column_num)
      : m_partition_id(partition_id),
        m_list_num(list_num),
        m_column_num(column_num) {}

 public:
  virtual Raw_key *create_access_key(Raw_table *db_table) const;

  virtual String_type str() const;

 private:
  int m_partition_id;
  int m_list_num;
  int m_column_num;
};

///////////////////////////////////////////////////////////////////////////

Object_key *Table_partition_values::create_key_by_partition_id(
    Object_id partition_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_PK_PARTITION_ID_LIST_NUM_COLUMN_NUM,
                          FIELD_PARTITION_ID, partition_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Table_partition_values::create_primary_key(Object_id partition_id,
                                                       int list_num,
                                                       int column_num) {
  return new (std::nothrow)
      Table_partition_values_pk(partition_id, list_num, column_num);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

Raw_key *Table_partition_values_pk::create_access_key(
    Raw_table *db_table) const {
  const int INDEX_NO =
      Table_partition_values::INDEX_PK_PARTITION_ID_LIST_NUM_COLUMN_NUM;

  TABLE *t = db_table->get_table();

  t->use_all_columns();

  t->field[Table_partition_values::FIELD_PARTITION_ID]->store(m_partition_id,
                                                              true);
  t->field[Table_partition_values::FIELD_LIST_NUM]->store(m_list_num, true);
  t->field[Table_partition_values::FIELD_COLUMN_NUM]->store(m_column_num, true);

  KEY *key_info = t->key_info + INDEX_NO;

  Raw_key *k =
      new (std::nothrow) Raw_key(INDEX_NO, key_info->key_length, HA_WHOLE_KEY);

  key_copy(k->key, t->record[0], key_info, k->key_len);

  return k;
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin inspected */
String_type Table_partition_values_pk::str() const {
  dd::Stringstream_type ss;
  ss << m_partition_id << ":" << m_list_num << ":" << m_column_num;
  return ss.str();
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
