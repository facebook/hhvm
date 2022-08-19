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

#ifndef DD_TABLES__CATALOGS_INCLUDED
#define DD_TABLES__CATALOGS_INCLUDED

#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_impl.h"

namespace dd {

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Catalogs : public Object_table_impl {
 public:
  static const Catalogs &instance() {
    static Catalogs *s_instance = new Catalogs();
    return *s_instance;
  }

  static const CHARSET_INFO *name_collation() {
    return Object_table_definition_impl::fs_name_collation();
  }

  enum enum_fields {
    FIELD_ID = static_cast<uint>(Common_field::ID),
    FIELD_NAME,
    FIELD_CREATED,
    FIELD_LAST_ALTERED,
    FIELD_OPTIONS,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_NAME = static_cast<uint>(Common_index::UK_NAME)
  };

  Catalogs() {
    m_target_def.set_table_name("catalogs");

    m_target_def.add_field(FIELD_ID, "FIELD_ID",
                           "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
    m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                           "name VARCHAR(64) NOT NULL COLLATE " +
                               String_type(name_collation()->name));
    m_target_def.add_field(FIELD_CREATED, "FIELD_CREATED",
                           "created TIMESTAMP NOT NULL");
    m_target_def.add_field(FIELD_LAST_ALTERED, "FIELD_LAST_ALTERED",
                           "last_altered TIMESTAMP NOT NULL");
    m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS",
                           "options MEDIUMTEXT");

    m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
    m_target_def.add_index(INDEX_UK_NAME, "INDEX_UK_NAME", "UNIQUE KEY (name)");

    m_target_def.add_populate_statement(
        "INSERT INTO catalogs(id, name, options, created, last_altered) "
        "VALUES (1, 'def', NULL, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)");
  }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__CATALOGS_INCLUDED
