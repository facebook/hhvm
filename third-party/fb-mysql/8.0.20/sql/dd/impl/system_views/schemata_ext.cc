/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/schemata_ext.h"

#include <string>

#include "sql/dd/impl/predefined_properties.h"  // Predefined_properties
#include "sql/stateless_allocator.h"

namespace dd {
namespace system_views {

const Schemata_ext &Schemata_ext::instance() {
  static Schemata_ext *s_instance = new Schemata_ext();
  return *s_instance;
}

Schemata_ext::Schemata_ext() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_CATALOG_NAME, "CATALOG_NAME",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_SCHEMA_NAME, "SCHEMA_NAME",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(
      FIELD_DB_METADATA, "DB_METADATA",
      String_type{"GET_DD_PROPERTY_KEY_VALUE(sch.options,'"} +
          Predefined_properties::get_db_metadata_key() + "')");

  m_target_def.add_from("mysql.schemata sch");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");

  m_target_def.add_where("CAN_ACCESS_DATABASE(sch.name)");
}

}  // namespace system_views
}  // namespace dd
