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

#include "sql/dd/impl/tables/view_table_usage.h"

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

const View_table_usage &View_table_usage::instance() {
  static View_table_usage *s_instance = new (std::nothrow) View_table_usage();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

View_table_usage::View_table_usage() {
  m_target_def.set_table_name("view_table_usage");

  m_target_def.add_field(FIELD_VIEW_ID, "FIELD_VIEW_ID",
                         "view_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(
      FIELD_TABLE_CATALOG, "FIELD_TABLE_CATALOG",
      "table_catalog VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_TABLE_SCHEMA, "FIELD_TABLE_SCHEMA",
      "table_schema VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));
  m_target_def.add_field(
      FIELD_TABLE_NAME, "FIELD_TABLE_NAME",
      "table_name VARCHAR(64) NOT NULL COLLATE " +
          String_type(Object_table_definition_impl::fs_name_collation()->name));

  m_target_def.add_index(
      INDEX_PK_VIEW_ID_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME,
      "INDEX_PK_VIEW_ID_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME",
      "PRIMARY KEY(view_id, table_catalog, "
      "table_schema, table_name)");
  m_target_def.add_index(INDEX_K_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME,
                         "INDEX_K_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME",
                         "KEY (table_catalog, table_schema, table_name)");

  m_target_def.add_foreign_key(FK_VIEW_ID, "FK_VIEW_ID",
                               "FOREIGN KEY (view_id) REFERENCES "
                               "tables(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_table_usage::create_key_by_view_id(Object_id view_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_PK_VIEW_ID_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME, FIELD_VIEW_ID,
      view_id);
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_table_usage::create_primary_key(
    Object_id view_id, const String_type &table_catalog,
    const String_type &table_schema, const String_type &table_name) {
  return new (std::nothrow) Composite_obj_id_3char_key(
      INDEX_PK_VIEW_ID_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME, FIELD_VIEW_ID,
      view_id, FIELD_TABLE_CATALOG, table_catalog, FIELD_TABLE_SCHEMA,
      table_schema, FIELD_TABLE_NAME, table_name);
}

///////////////////////////////////////////////////////////////////////////

Object_key *View_table_usage::create_key_by_name(
    const String_type &table_catalog, const String_type &table_schema,
    const String_type &table_name) {
  return new (std::nothrow) Table_reference_range_key(
      INDEX_K_TABLE_CATALOG_TABLE_SCHEMA_TABLE_NAME, FIELD_TABLE_CATALOG,
      table_catalog, FIELD_TABLE_SCHEMA, table_schema, FIELD_TABLE_NAME,
      table_name);
}

}  // namespace tables
}  // namespace dd
