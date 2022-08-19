/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_TABLES__CHECK_CONSTRAINTS_INCLUDED
#define DD_TABLES__CHECK_CONSTRAINTS_INCLUDED

#include "sql/dd/impl/types/object_table_impl.h"  // dd::Object_table_impl
#include "sql/dd/object_id.h"                     // dd::Object_id

namespace dd {
class Object_key;

namespace tables {

///////////////////////////////////////////////////////////////////////////

// The Check_constraints table has one row for each check constraint.
class Check_constraints : virtual public Object_table_impl {
 public:
  static const Check_constraints &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_SCHEMA_ID,
    FIELD_TABLE_ID,
    FIELD_NAME,
    FIELD_ENFORCED,
    FIELD_CHECK_CLAUSE,
    FIELD_CHECK_CLAUSE_UTF8,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_SCHEMA_ID_NAME = static_cast<uint>(Common_index::UK_NAME),
    INDEX_UK_TABLE_ID_NAME
  };

  enum enum_foreign_keys { FK_SCHEMA_ID, FK_TABLE_ID };

  Check_constraints();

  static Object_key *create_key_by_table_id(Object_id table_id);

  static Object_key *create_key_by_check_constraint_name(
      Object_id schema_id, const String_type &check_cons_name);

  /**
    Check if schema contains check constraints with specified name.

    @param        thd               Thread context.
    @param        schema_id         Id of schema to be inspected.
    @param        check_cons_name   Name of the check constraint.
    @param[out]   exists            Set to true if check constraint
                                    the the name provided exists in
                                    the schema, false otherwise.

    @retval      false    No error.
    @retval      true     Error.
  */

  static bool check_constraint_exists(THD *thd, Object_id schema_id,
                                      const String_type &check_cons_name,
                                      bool *exists);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__CHECK_CONSTRAINTS_INCLUDED
