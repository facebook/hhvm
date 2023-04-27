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

#include "sql/dd/impl/tables/tablespace_files.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"       // Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"

namespace dd {
namespace tables {

const Tablespace_files &Tablespace_files::instance() {
  static Tablespace_files *s_instance = new Tablespace_files();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Tablespace_files::Tablespace_files() {
  m_target_def.set_table_name("tablespace_files");

  m_target_def.add_field(FIELD_TABLESPACE_ID, "FIELD_TABLESPACE_ID",
                         "tablespace_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_ORDINAL_POSITION, "FIELD_ORDINAL_POSITION",
                         "ordinal_position INT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_FILE_NAME, "FIELD_FILE_NAME",
                         "file_name VARCHAR(512) NOT NULL");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");

  m_target_def.add_index(INDEX_UK_TABLESPACE_ID_ORDINAL_POSITION,
                         "INEDX_UK_TABLESPACE_ID_ORDINAL_POSITION",
                         "UNIQUE KEY (tablespace_id, ordinal_position)");
  m_target_def.add_index(INDEX_UK_FILE_NAME, "INEDX_UK_FILE_NAME",
                         "UNIQUE KEY (file_name)");

  m_target_def.add_foreign_key(FK_TABLESPACE_ID, "FK_TABLESPACE_ID",
                               "FOREIGN KEY (tablespace_id) \
                                REFERENCES tablespaces(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Tablespace_files::create_key_by_tablespace_id(
    Object_id tablespace_id) {
  return new (std::nothrow)
      Parent_id_range_key(INDEX_UK_TABLESPACE_ID_ORDINAL_POSITION,
                          FIELD_TABLESPACE_ID, tablespace_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *Tablespace_files::create_primary_key(Object_id tablespace_id,
                                                 int ordinal_position) {
  return new (std::nothrow)
      Composite_pk(INDEX_UK_TABLESPACE_ID_ORDINAL_POSITION, FIELD_TABLESPACE_ID,
                   tablespace_id, FIELD_ORDINAL_POSITION, ordinal_position);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
