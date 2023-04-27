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

#ifndef DD_TABLES__RESOURCE_GROUPS_INCLUDED
#define DD_TABLES__RESOURCE_GROUPS_INCLUDED

#include <new>
#include <string>

#include "sql/dd/impl/raw/object_keys.h"
#include "sql/dd/impl/types/entity_object_table_impl.h"  // dd::Object_table_i...
#include "sql/dd/impl/types/resource_group_impl.h"
#include "sql/dd/types/resource_group.h"

namespace dd {

class Raw_record;

namespace tables {
class Resource_groups : public Entity_object_table_impl {
 public:
  Resource_groups();

  static const Resource_groups &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_RESOURCE_GROUP_NAME,
    FIELD_RESOURCE_GROUP_TYPE,
    FIELD_RESOURCE_GROUP_ENABLED,
    FIELD_CPU_ID_MASK,
    FIELD_THREAD_PRIORITY,
    FIELD_OPTIONS,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_RESOURCE_GROUP_NAME = static_cast<uint>(Common_index::UK_NAME)
  };

  enum enum_foreign_keys {};

 public:
  Resource_group *create_entity_object(const Raw_record &) const override {
    return new (std::nothrow) Resource_group_impl();
  }

  static bool update_object_key(Global_name_key *key,
                                const String_type &resource_group_name);
};
}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__RESOURCE_GROUPS_INCLUDED
