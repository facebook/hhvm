/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/tables/resource_groups.h"

#include "sql/dd/impl/raw/object_keys.h"  // dd::Global_name_key
#include "sql/dd/impl/types/object_table_definition_impl.h"  // dd::Raw_record
#include "sql/dd/impl/types/resource_group_impl.h"  // dd::Resource_group_impl

namespace dd {
namespace tables {

Resource_groups::Resource_groups() {
  m_target_def.set_table_name("resource_groups");

  m_target_def.add_field(FIELD_ID, "FIELD_ID",
                         "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT");
  m_target_def.add_field(FIELD_RESOURCE_GROUP_NAME, "FIELD_RESOURCE_GROUP_NAME",
                         "resource_group_name VARCHAR(64) NOT NULL COLLATE " +
                             String_type(name_collation()->name));
  m_target_def.add_field(FIELD_RESOURCE_GROUP_TYPE, "FIELD_RESOURCE_GROUP_TYPE",
                         "resource_group_type enum('SYSTEM', 'USER') NOT NULL");
  m_target_def.add_field(FIELD_RESOURCE_GROUP_ENABLED,
                         "FIELD_RESOURCE_GROUP_ENABLED",
                         "resource_group_enabled  boolean NOT NULL");
  m_target_def.add_field(FIELD_CPU_ID_MASK, "FIELD_CPU_ID_MASK",
                         "cpu_id_mask VARCHAR(1024) NOT NULL");
  m_target_def.add_field(FIELD_THREAD_PRIORITY, "FIELD_THREAD_PRIORITY",
                         "thread_priority int NOT NULL");
  m_target_def.add_field(FIELD_OPTIONS, "FIELD_OPTIONS", "options MEDIUMTEXT");

  m_target_def.add_index(INDEX_PK_ID, "INDEX_PK_ID", "PRIMARY KEY(id)");
  m_target_def.add_index(INDEX_UK_RESOURCE_GROUP_NAME,
                         "INDEX_UK_RESOURCE_GROUP_NAME",
                         "UNIQUE KEY (resource_group_name)");
}

const Resource_groups &Resource_groups::instance() {
  static Resource_groups *s_instance = new Resource_groups();
  return *s_instance;
}

const CHARSET_INFO *Resource_groups::name_collation() {
  return &my_charset_utf8_general_ci;
}

bool Resource_groups::update_object_key(Global_name_key *key,
                                        const String_type &name) {
  key->update(FIELD_RESOURCE_GROUP_NAME, name, name_collation());
  return false;
}

}  // namespace tables
}  // namespace dd
