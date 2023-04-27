/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/indexes.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

const Indexes &Indexes::instance() {
  static Indexes *s_instance = new Indexes();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Indexes::name_collation() {
  return &my_charset_utf8_tolower_ci;
}

///////////////////////////////////////////////////////////////////////////

Indexes::Indexes() {
  m_target_def.set_table_name("indexes");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_TABLE_ID, "FIELD_TABLE_ID",
                         "table_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_TYPE, "FIELD_TYPE",
                         "type ENUM(\n"
                         "  'PRIMARY',\n"
                         "  'UNIQUE',\n"
                         "  'MULTIPLE',\n"
                         "  'FULLTEXT',\n"
                         "  'SPATIAL'\n"
                         ") NOT NULL");
  m_target_def.add_field(FIELD_ALGORITHM, "FIELD_ALGORITHM",
                         "algorithm ENUM(\n"
                         "  'SE_SPECIFIC',\n"
                         "  'BTREE',\n"
                         "  'RTREE',\n"
                         "  'HASH',\n"
                         "  'FULLTEXT'\n"
                         ") NOT NULL");
  m_target_def.add_field(FIELD_IS_ALGORITHM_EXPLICIT,
                         "FIELD_IS_ALGORITHM_EXPLICIT",
                         "is_algorithm_explicit BOOL NOT NULL");
  m_target_def.add_field(FIELD_IS_VISIBLE, "FIELD_IS_VISIBLE",
                         "is_visible BOOL NOT NULL");
  m_target_def.add_field(FIELD_IS_GENERATED, "FIELD_IS_GENERATED",
                         "is_generated BOOL NOT NULL");
  m_target_def.add_field(FIELD_HIDDEN, "FIELD_HIDDEN", "hidden BOOL NOT NULL");
  m_target_def.add_field(FIELD_ORDINAL_POSITION, "FIELD_ORDINAL_POSITION",
                         "ordinal_position INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_COMMENT, "FIELD_COMMENT",
                         "comment VARCHAR(2048) NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");
  m_target_def.add_field(FIELD_TABLESPACE_ID, "FIELD_TABLESPACE_ID",
                         "tablespace_id BIGINT UNSIGNED");
  m_target_def.add_field(FIELD_ENGINE, "FIELD_ENGINE",
                         "engine VARCHAR(64) NOT NULL COLLATE utf8_general_ci");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_TABLE_ID_NAME, "INDEX_UK_TABLE_ID_NAME",
                         "UNIQUE KEY(table_id, name)");
  m_target_def.add_index(INDEX_K_TABLESPACE_ID, "INDEX_K_TABLESPACE_ID",
                         "KEY(tablespace_id)");

  m_target_def.add_foreign_key(FK_TABLE_ID, "FK_TABLE_ID",
                               "FOREIGN KEY (table_id) REFERENCES "
                               "tables(id)");
  m_target_def.add_foreign_key(FK_TABLESPACE_ID, "FK_TABLESPACE_ID",
                               "FOREIGN KEY (tablespace_id) REFERENCES "
                               "tablespaces(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Indexes::create_key_by_table_id(Object_id table_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_TABLE_ID_NAME, FIELD_TABLE_ID, table_id);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
