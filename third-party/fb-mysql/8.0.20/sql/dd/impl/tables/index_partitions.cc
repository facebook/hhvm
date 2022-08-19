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

#include "sql/dd/impl/tables/index_partitions.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

const Index_partitions &Index_partitions::instance() {
  static Index_partitions *s_instance = new Index_partitions();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Index_partitions::Index_partitions() {
  m_target_def.set_table_name("index_partitions");

  m_target_def.add_field(FIELD_PARTITION_ID, "FIELD_PARTITION_ID",
                         "partition_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_INDEX_ID, "FIELD_INDEX_ID",
                         "index_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");
  m_target_def.add_field(FIELD_TABLESPACE_ID, "FIELD_TABLESPACE_ID",
                         "tablespace_id BIGINT UNSIGNED");

  m_target_def.add_index(INDEX_PK_PARTITION_ID_INDEX_ID,
                         "INDEX_PK_PARTITION_ID_INDEX_ID",
                         "PRIMARY KEY(partition_id, index_id)");
  m_target_def.add_index(INDEX_K_INDEX_ID, "INDEX_K_INDEX_ID", "KEY(index_id)");
  m_target_def.add_index(INDEX_K_TABLESPACE_ID, "INDEX_K_TABLESPACE_ID",
                         "KEY(tablespace_id)");

  m_target_def.add_foreign_key(FK_TABLE_PARTITION_ID, "FK_TABLE_PARTITION_ID",
                               "FOREIGN KEY (partition_id) REFERENCES "
                               "table_partitions(id)");
  m_target_def.add_foreign_key(FK_INDEX_ID, "FK_INDEX_ID",
                               "FOREIGN KEY (index_id) REFERENCES "
                               "indexes(id)");
  m_target_def.add_foreign_key(FK_TABLESPACE_ID, "FK_TABLESPACE_ID",
                               "FOREIGN KEY (tablespace_id) REFERENCES "
                               "tablespaces(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Index_partitions::create_key_by_partition_id(
    Object_id partition_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_PK_PARTITION_ID_INDEX_ID, FIELD_PARTITION_ID, partition_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Index_partitions::create_primary_key(Object_id partition_id,
                                                 Object_id index_id) {
  return new (std::nothrow)
      Composite_pk(INDEX_PK_PARTITION_ID_INDEX_ID, FIELD_PARTITION_ID,
                   partition_id, FIELD_INDEX_ID, index_id);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
