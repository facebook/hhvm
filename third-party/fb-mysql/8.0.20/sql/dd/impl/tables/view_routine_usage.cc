/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/view_routine_usage.h"

#include <new>
#include <string>

#include "sql/dd/impl/raw/object_keys.h"       // dd::Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/mysqld.h"
#include "sql/stateless_allocator.h"

namespace dd {
namespace tables {

///////////////////////////////////////////////////////////////////////////

const View_routine_usage &View_routine_usage::instance() {
  static View_routine_usage *s_instance =
      new (std::nothrow) View_routine_usage();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

View_routine_usage::View_routine_usage() {
  m_target_def.set_table_name("view_routine_usage");

  m_target_def.add_field(FIELD_VIEW_ID, "FIELD_VIEW_ID",
                         "view_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(
      FIELD_ROUTINE_CATALOG, "FIELD_ROUTINE_CATALOG",
      "routine_catalog VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_ROUTINE_SCHEMA, "FIELD_ROUTINE_SCHEMA",
      "routine_schema VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(FIELD_ROUTINE_NAME, "FIELD_ROUTINE_NAME",
                         "routine_name VARCHAR(64) NOT NULL COLLATE "
                         " utf8_general_ci");

  m_target_def.add_index(INDEX_PK_VIEW_ID_ROUTINE_CATALOG,
                         "INDEX_PK_VIEW_ID_ROUTINE_CATALOG",
                         "PRIMARY KEY(view_id, routine_catalog, "
                         "routine_schema, routine_name)");
  m_target_def.add_index(INDEX_K_ROUTINE_CATALOG_ROUTINE_SCHEMA_ROUTINE_NAME,
                         "INDEX_K_ROUTINE_CATALOG_ROUTINE_SCHEMA_ROUTINE_NAME",
                         "KEY (routine_catalog, routine_schema, "
                         "routine_name)");

  m_target_def.add_foreign_key(FK_VIEW_ID, "FK_VIEW_ID",
                               "FOREIGN KEY (view_id) REFERENCES "
                               "tables(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_routine_usage::create_key_by_view_id(Object_id view_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_PK_VIEW_ID_ROUTINE_CATALOG, FIELD_VIEW_ID, view_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_routine_usage::create_primary_key(
    Object_id view_id, const String_type &routine_catalog,
    const String_type &routine_schema, const String_type &routine_name) {
  return new (std::nothrow) Composite_obj_id_3char_key(
      INDEX_PK_VIEW_ID_ROUTINE_CATALOG, FIELD_VIEW_ID, view_id,
      FIELD_ROUTINE_CATALOG, routine_catalog, FIELD_ROUTINE_SCHEMA,
      routine_schema, FIELD_ROUTINE_NAME, routine_name);
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_routine_usage::create_key_by_name(
    const String_type &routine_catalog, const String_type &routine_schema,
    const String_type &routine_name) {
  return new (std::nothrow) Table_reference_range_key(
      INDEX_K_ROUTINE_CATALOG_ROUTINE_SCHEMA_ROUTINE_NAME,
      FIELD_ROUTINE_CATALOG, routine_catalog, FIELD_ROUTINE_SCHEMA,
      routine_schema, FIELD_ROUTINE_NAME, routine_name);
}

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
