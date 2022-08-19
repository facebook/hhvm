/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/parameter_type_elements.h"

#include <new>

#include "my_dbug.h"
#include "sql/dd/impl/raw/object_keys.h"       // Parent_id_range_key
#include "sql/dd/impl/tables/dd_properties.h"  // TARGET_DD_VERSION
#include "sql/dd/impl/types/object_table_definition_impl.h"
#include "sql/sql_const.h"  // MAX_INTERVAL_VALUE_LENGTH

namespace dd {
namespace tables {

const Parameter_type_elements &Parameter_type_elements::instance() {
  static Parameter_type_elements *s_instance = new Parameter_type_elements();
  return *s_instance;
}

Parameter_type_elements::Parameter_type_elements() {
  m_target_def.set_table_name("parameter_type_elements");

  m_target_def.add_field(FIELD_PARAMETER_ID, "FIELD_PARAMETER_ID",
                         "parameter_id BIGINT UNSIGNED NOT NULL");
  m_target_def.add_field(FIELD_ELEMENT_INDEX, "FIELD_ELEMENT_INDEX",
                         "element_index INT UNSIGNED NOT NULL");
  // Fail if the max length of enum/set elements is increased.
  // If it's changed, the corresponding column length must be
  // increased, but this must be treated as a DD table upgrade
  // requiring special care.
  DBUG_ASSERT(MAX_INTERVAL_VALUE_LENGTH <= 255);

  // Leave room for four bytes per character, which is used
  // by e.g. utf8mb4, i.e. 255 * 4 = 1020 bytes.
  m_target_def.add_field(FIELD_NAME, "FIELD_NAME",
                         "name VARBINARY(1020) NOT NULL");

  m_target_def.add_index(INDEX_PK_PARAMETER_ID_ELEMENT_INDEX,
                         "INDEX_PK_PARAMETER_ID_ELEMENT_INDEX",
                         "PRIMARY KEY(parameter_id, element_index)");
  // We may have multiple similar element names. Do we plan to deprecate it?
  // m_target_def.add_index("UNIQUE KEY(column_id, name)");

  m_target_def.add_foreign_key(FK_PARAMETER_ID, "FK_PARAMETER_ID",
                               "FOREIGN KEY (parameter_id) REFERENCES "
                               "parameters(id)");
}

///////////////////////////////////////////////////////////////////////////

Object_key *Parameter_type_elements::create_key_by_parameter_id(
    Object_id parameter_id) {
  return new (std::nothrow) Parent_id_range_key(
      INDEX_PK_PARAMETER_ID_ELEMENT_INDEX, FIELD_PARAMETER_ID, parameter_id);
}

///////////////////////////////////////////////////////////////////////////

/* purecov: begin deadcode */
Object_key *Parameter_type_elements::create_primary_key(Object_id parameter_id,
                                                        int index) {
  return new (std::nothrow)
      Composite_pk(INDEX_PK_PARAMETER_ID_ELEMENT_INDEX, FIELD_PARAMETER_ID,
                   parameter_id, FIELD_ELEMENT_INDEX, index);
}
/* purecov: end */

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd
