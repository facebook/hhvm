/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/spatial_reference_systems.h"

#include <string.h>
#include <new>

#include "m_ctype.h"
#include "sql/dd/impl/raw/object_keys.h"  // Parent_id_range_key
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/dd/impl/types/spatial_reference_system_impl.h"  // dd::Spatial_refere...

namespace dd {
namespace tables {

const Spatial_reference_systems &Spatial_reference_systems::instance() {
  static Spatial_reference_systems *s_instance =
      new Spatial_reference_systems();
  return *s_instance;
}

///////////////////////////////////////////////////////////////////////////

const CHARSET_INFO *Spatial_reference_systems::name_collation() {
  return &my_charset_utf8_general_ci;
}

///////////////////////////////////////////////////////////////////////////

Spatial_reference_systems::Spatial_reference_systems() {
  // Note: The maximum length of various strings is hard-coded here. These
  // lengths must match those in sql/sql_cmd_srs.cc.

  m_target_def.set_table_name("st_spatial_reference_systems");

  m_target_def.add_field(FIELD_ID, "FIELD_ID", "id INTEGER UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_CATALOG_ID, "FIELD_CATALOG_ID",
                         "catalog_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name CHARACTER VARYING(80)\n"
                         "NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  // Note that DEFAULT and ON UPDATE clauses are used since this table is
  // populated by means of DML statements unlike the other DD tables.
  m_target_def.add_field(FIELD_LAST_ALTERED, "FIELD_LAST_ALTERED",
                         "last_altered TIMESTAMP NOT NULL\n"
                         "DEFAULT CURRENT_TIMESTAMP\n"
                         "ON UPDATE CURRENT_TIMESTAMP");
  m_target_def.add_field(FIELD_CREATED, "FIELD_CREATED",
                         "created TIMESTAMP NOT NULL\n"
                         "DEFAULT CURRENT_TIMESTAMP");
  m_target_def.add_field(FIELD_ORGANIZATION, "FIELD_ORGANIZATION",
                         "organization CHARACTER VARYING(256)\n"
                         "COLLATE utf8_general_ci");
  m_target_def.add_field(FIELD_ORGANIZATION_COORDSYS_ID,
                         "FIELD_ORGANIZATION_COORDSYS_ID",
                         "organization_coordsys_id INTEGER UNSIGNED");
  m_target_def.add_field(FIELD_DEFINITION, "FIELD_DEFINITION",
                         "definition CHARACTER VARYING(4096)\n"
                         "NOT NULL");
  m_target_def.add_field(FIELD_DESCRIPTION, "FIELD_DESCRIPTION",
                         "description CHARACTER VARYING(2048)");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY (id)");
  m_target_def.add_index(INDEX_UK_CATALOG_ID_NAME, "INDEX_UK_CATALOG_ID_NAME",
                         "UNIQUE KEY SRS_NAME (catalog_id, name)");
  m_target_def.add_index(INDEX_UK_CATALOG_ID_ORG_ID,
                         "INDEX_UK_CATALOG_ID_ORG_ID",
                         "UNIQUE KEY ORGANIZATION_AND_ID (catalog_id, "
                         "organization, organization_coordsys_id)");

  m_target_def.add_foreign_key(FK_CATALOG_ID, "FK_CATALOG_ID",
                               "FOREIGN KEY (catalog_id) REFERENCES \
                                  catalogs(id)");

  m_target_def.add_populate_statement(
      "INSERT INTO st_spatial_reference_systems (id, catalog_id, name, "
      "definition) VALUES (0, 1, '', '')");
}

///////////////////////////////////////////////////////////////////////////

Spatial_reference_system *Spatial_reference_systems::create_entity_object(
    const Raw_record &) const {
  return new (std::nothrow) Spatial_reference_system_impl();
}

///////////////////////////////////////////////////////////////////////////

bool Spatial_reference_systems::update_object_key(Item_name_key *key,
                                                  Object_id catalog_id,
                                                  const String_type &name) {
  key->update(FIELD_CATALOG_ID, catalog_id, FIELD_NAME, name, name_collation());
  return false;
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Object_key *Spatial_reference_systems::create_key_by_catalog_id(
    Object_id catalog_id) {
  return new (std::nothrow) Parent_id_range_key(INDEX_UK_CATALOG_ID_NAME,
                                                FIELD_CATALOG_ID, catalog_id);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
