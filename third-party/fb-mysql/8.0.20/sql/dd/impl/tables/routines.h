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

#ifndef DD_TABLES__ROUTINES_INCLUDED
#define DD_TABLES__ROUTINES_INCLUDED

#include <string>

#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_table_impl.h"  // dd::Dictionary_obj...
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/routine.h"  // dd::Routine

namespace dd {
class Object_key;
class Routine_name_key;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Routines : public Entity_object_table_impl {
 public:
  static const Routines &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_SCHEMA_ID,
    FIELD_NAME,
    FIELD_TYPE,
    FIELD_RESULT_DATA_TYPE,
    FIELD_RESULT_DATA_TYPE_UTF8,
    FIELD_RESULT_IS_ZEROFILL,
    FIELD_RESULT_IS_UNSIGNED,
    FIELD_RESULT_CHAR_LENGTH,
    FIELD_RESULT_NUMERIC_PRECISION,
    FIELD_RESULT_NUMERIC_SCALE,
    FIELD_RESULT_DATETIME_PRECISION,
    FIELD_RESULT_COLLATION_ID,
    FIELD_DEFINITION,
    FIELD_DEFINITION_UTF8,
    FIELD_PARAMETER_STR,
    FIELD_IS_DETERMINISTIC,
    FIELD_SQL_DATA_ACCESS,
    FIELD_SECURITY_TYPE,
    FIELD_DEFINER,
    FIELD_SQL_MODE,
    FIELD_CLIENT_COLLATION_ID,
    FIELD_CONNECTION_COLLATION_ID,
    FIELD_SCHEMA_COLLATION_ID,
    FIELD_CREATED,
    FIELD_LAST_ALTERED,
    FIELD_COMMENT,
    FIELD_OPTIONS,
    FIELD_EXTERNAL_LANGUAGE,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_SCHEMA_ID_TYPE_NAME,
    INDEX_K_RESULT_COLLATION_ID,
    INDEX_K_CLIENT_COLLATION_ID,
    INDEX_K_CONNECTION_COLLATION_ID,
    INDEX_K_SCHEMA_COLLATION_ID
  };

  enum enum_foreign_keys {
    FK_SCHEMA_ID,
    FK_RESULT_COLLATION_ID,
    FK_CLIENT_COLLATION_ID,
    FK_CONNECTION_COLLATION_ID,
    FK_SCHEMA_COLLATION_ID
  };

  Routines();

  virtual Routine *create_entity_object(const Raw_record &) const;

  static bool update_object_key(Routine_name_key *key, Object_id schema_id,
                                Routine::enum_routine_type type,
                                const String_type &routine_name);

  static Object_key *create_key_by_schema_id(Object_id schema_id);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__ROUTINES_INCLUDED
