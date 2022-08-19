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

#ifndef DD_TABLES__PARAMETERS_INCLUDED
#define DD_TABLES__PARAMETERS_INCLUDED

#include "sql/dd/impl/types/object_table_impl.h"  // dd::Object_table_impl
#include "sql/dd/object_id.h"                     // dd::Object_id
#include "sql/dd/string_type.h"

namespace dd {
class Object_key;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Parameters : public Object_table_impl {
 public:
  static const Parameters &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_ROUTINE_ID,
    FIELD_ORDINAL_POSITION,
    FIELD_MODE,
    FIELD_NAME,
    FIELD_DATA_TYPE,
    FIELD_DATA_TYPE_UTF8,
    FIELD_IS_ZEROFILL,
    FIELD_IS_UNSIGNED,
    FIELD_CHAR_LENGTH,
    FIELD_NUMERIC_PRECISION,
    FIELD_NUMERIC_SCALE,
    FIELD_DATETIME_PRECISION,
    FIELD_COLLATION_ID,
    FIELD_OPTIONS,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_ROUTINE_ID_ORDINAL_POSITION,
    INDEX_K_COLLATION_ID
  };

  enum enum_foreign_keys { FK_ROUTINE_ID, FK_COLLATION_ID };

  Parameters();

  static Object_key *create_key_by_routine_id(Object_id routine_id);

  static Object_key *create_primary_key(Object_id routine_id,
                                        int ordinal_position);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__PARAMETERS_INCLUDED
