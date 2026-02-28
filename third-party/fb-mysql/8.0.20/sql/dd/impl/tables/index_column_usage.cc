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

#include "sql/dd/impl/tables/index_column_usage.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

const Index_column_usage &Index_column_usage::instance() {
  static Index_column_usage *s_instance = new Index_column_usage();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Index_column_usage::Index_column_usage() {
  m_target_def.set_table_name("index_column_usage");

  m_target_def.add_field(FIELD_INDEX_ID, "FIELD_INDEX_ID",
                         "index_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_ORDINAL_POSITION, "FIELD_ORDINAL_POSITION",
                         "ordinal_position INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_COLUMN_ID, "FIELD_COLUMN_ID",
                         "column_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_LENGTH, "FIELD_LENGTH", "length INT UNSIGNED");
  /*
    TODO-WIKI6599.Task20: What value we are supposed to use for indexes which
    don't support ordering? How this can be mapped to I_S?
    Perhaps make it nullable?
  */
  m_target_def.add_field(FIELD_ORDER, "FIELD_ORDER",
                         "`order` ENUM('UNDEF', 'ASC', 'DESC') "
                         "NOT NULL");
  m_target_def.add_field(FIELD_HIDDEN, "FIELD_HIDDEN", "hidden BOOL NOT NULL");

  m_target_def.add_index(INDEX_UK_INDEX_ID_ORDINAL_POSITION,
                         "INDEX_UK_INDEX_ID_ORDINAL_POSITION",
                         "UNIQUE KEY (index_id, ordinal_position)");
  m_target_def.add_index(INDEX_UK_INDEX_ID_COLUMN_ID_HIDDEN,
                         "INDEX_UK_INDEX_ID_COLUMN_ID_HIDDEN",
                         "UNIQUE KEY (index_id, column_id, hidden)");
  m_target_def.add_index(INDEX_K_COLUMN_ID, "INDEX_K_COLUMN_ID",
                         "KEY (column_id)");

  m_target_def.add_foreign_key(FK_INDEX_ID, "FK_INDEX_ID",
                               "FOREIGN KEY (index_id) REFERENCES "
                               "indexes(id)");
  m_target_def.add_foreign_key(FK_COLUMN_ID, "FK_COLUMN_ID",
                               "FOREIGN KEY (column_id) REFERENCES "
                               "columns(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Index_column_usage::create_key_by_index_id(Object_id index_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_UK_INDEX_ID_ORDINAL_POSITION, FIELD_INDEX_ID, index_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Index_column_usage::create_primary_key(Object_id index_id,
                                                   int ordinal_position) {
  return new (std::nothrow)
      Composite_pk(INDEX_UK_INDEX_ID_ORDINAL_POSITION, FIELD_INDEX_ID, index_id,
                   FIELD_ORDINAL_POSITION, ordinal_position);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
