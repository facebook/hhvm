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

#ifndef DD_TABLES__COLUMN_STATISTICS_INCLUDED
#define DD_TABLES__COLUMN_STATISTICS_INCLUDED

#include "sql/dd/impl/types/entity_object_table_impl.h"
#include "sql/dd/object_id.h"  // Object_id
#include "sql/dd/string_type.h"
#include "sql/dd/types/column_statistics.h"

namespace dd {

class Dictionary_object;
class Item_name_key;
class Object_key;
class Raw_record;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Column_statistics final : public Entity_object_table_impl {
 public:
  static const Column_statistics &instance();

  enum enum_fields {
    FIELD_ID = static_cast<uint>(Common_field::ID),
    FIELD_CATALOG_ID,
    FIELD_NAME,
    FIELD_SCHEMA_NAME,
    FIELD_TABLE_NAME,
    FIELD_COLUMN_NAME,
    FIELD_HISTOGRAM,
    FIELD_OPTIONS,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_CATALOG_ID_NAME = static_cast<uint>(Common_index::UK_NAME),
    INDEX_UK_CATALOG_ID_SCHEMA_NAME_TABLE_NAME_COLUMN_NAME
  };

  enum enum_foreign_keys { FK_CATALOG_ID };

  Column_statistics();

  dd::Column_statistics *create_entity_object(
      const Raw_record &) const override;

  static bool update_object_key(Item_name_key *key, Object_id catalog_id,
                                const String_type &name);

  static Object_key *create_key_by_catalog_id(Object_id catalog_id);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__COLUMN_STATISTICS_INCLUDED
