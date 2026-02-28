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

#ifndef DD_TABLES__INDEX_COLUMN_USAGE_INCLUDED
#define DD_TABLES__INDEX_COLUMN_USAGE_INCLUDED

#include "sql/dd/impl/types/object_table_impl.h"  // dd::Object_table_impl
#include "sql/dd/object_id.h"                     // dd::Object_id
#include "sql/dd/string_type.h"

namespace dd {
class Object_key;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Index_column_usage : public Object_table_impl {
 public:
  static const Index_column_usage &instance();

  enum enum_fields {
    FIELD_INDEX_ID,
    FIELD_ORDINAL_POSITION,
    FIELD_COLUMN_ID,
    FIELD_LENGTH,
    FIELD_ORDER,
    FIELD_HIDDEN,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_UK_INDEX_ID_ORDINAL_POSITION,
    INDEX_UK_INDEX_ID_COLUMN_ID_HIDDEN,
    INDEX_K_COLUMN_ID
  };

  enum enum_foreign_keys { FK_INDEX_ID, FK_COLUMN_ID };

  Index_column_usage();

  static Object_key *create_key_by_index_id(Object_id index_id);

  static Object_key *create_primary_key(Object_id index_id,
                                        int ordinal_position);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__INDEX_COLUMN_USAGE_INCLUDED
