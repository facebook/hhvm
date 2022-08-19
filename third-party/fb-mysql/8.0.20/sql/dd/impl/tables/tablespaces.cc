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

#include "sql/dd/impl/tables/tablespaces.h"

#include <new>

#include "sql/dd/impl/raw/object_keys.h"  // dd::Global_name_key
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/impl/types/tablespace_impl.h"  // dd::Tablespace_impl

namespace dd {
namespace tables {

const Tablespaces &Tablespaces::instance() {
  static Tablespaces *s_instance = new Tablespaces();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

Tablespaces::Tablespaces() {
  m_target_def.set_table_name("tablespaces");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  /*
    We allow name lengths up to 268 bytes, which is needed for InnoDB
    tablespace naming convention. The following explains the name format,

      <dbname> +'/'+ <tablename> +'#p#'+ <partitionname>
               +'#sp#'+ <subpartion_name> +'#tmp'
    Note: #tmp is appended during ALTER TABLE.

    Which accounts to total of 268 characters, considering the identifier
    names as 64 characters.
  */
  m_target_def.add_field(
      FIELD_NAME, "FIELD_NAME",
      "name VARCHAR(268) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::name_collation()->name));
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");
  m_target_def.add_field(FIELD_SE_PRIVATE_DATA, "FIELD_SE_PRIVATE_DATA",
                         "se_private_data MEDIUMTEXT");
  m_target_def.add_field(FIELD_COMMENT, "FIELD_COMMENT",
                         "comment VARCHAR(2048) NOT NULL");
  m_target_def.add_field(FIELD_ENGINE, "FIELD_ENGINE",
                         "engine VARCHAR(64) NOT NULL COLLATE utf8_general_ci");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_NAME, "INDEX_UK_NAME", "UNIQUE KEY(name)");
}

///////////////////////////////////////////////////////////////////////////

Tablespace *Tablespaces::create_entity_object(const Raw_record &) const {
  return new (std::nothrow) Tablespace_impl();
}

///////////////////////////////////////////////////////////////////////////

bool Tablespaces::update_object_key(Global_name_key *key,
                                    const String_type &tablespace_name) {
  key->update(FIELD_NAME, tablespace_name,
              Object_table_definition_impl::name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
