/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_TABLES__SPATIAL_REFERENCE_SYSTEMS_INCLUDED
#define DD_TABLES__SPATIAL_REFERENCE_SYSTEMS_INCLUDED

#include <string>

#include "sql/dd/impl/types/entity_object_table_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/spatial_reference_system.h"

namespace dd {

class Item_name_key;
class Object_key;
class Raw_record;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Spatial_reference_systems : public Entity_object_table_impl {
 public:
  static const Spatial_reference_systems &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_CATALOG_ID,
    FIELD_NAME,
    FIELD_LAST_ALTERED,
    FIELD_CREATED,
    FIELD_ORGANIZATION,
    FIELD_ORGANIZATION_COORDSYS_ID,
    FIELD_DEFINITION,
    FIELD_DESCRIPTION,
    FIELD_OPTIONS,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_CATALOG_ID_NAME = static_cast<uint>(Common_index::UK_NAME),
    INDEX_UK_CATALOG_ID_ORG_ID
  };

  enum enum_foreign_keys { FK_CATALOG_ID };

  Spatial_reference_systems();

  virtual Spatial_reference_system *create_entity_object(
      const Raw_record &) const;

  static bool update_object_key(Item_name_key *key, Object_id catalog_id,
                                const String_type &name);

  static Object_key *create_key_by_catalog_id(Object_id catalog_id);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__SPATIAL_REFERENCE_SYSTEMS_INCLUDED
