/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SYSTEM_VIEWS__ROUTINES_INCLUDED
#define DD_SYSTEM_VIEWS__ROUTINES_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"
#include "sql/dd/string_type.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.ROUTINES system view definition.
*/
class Routines : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields {
    FIELD_SPECIFIC_NAME,
    FIELD_ROUTINE_CATALOG,
    FIELD_ROUTINE_SCHEMA,
    FIELD_ROUTINE_NAME,
    FIELD_ROUTINE_TYPE,
    FIELD_DATA_TYPE,
    FIELD_CHARACTER_MAXIMUM_LENGTH,
    FIELD_CHARACTER_OCTET_LENGTH,
    FIELD_NUMERIC_PRECISION,
    FIELD_NUMERIC_SCALE,
    FIELD_DATETIME_PRECISION,
    FIELD_CHARACTER_SET_NAME,
    FIELD_COLLATION_NAME,
    FIELD_DTD_IDENTIFIER,
    FIELD_ROUTINE_BODY,
    FIELD_ROUTINE_DEFINITION,
    FIELD_EXTERNAL_NAME,
    FIELD_EXTERNAL_LANGUAGE,
    FIELD_PARAMETER_STYLE,
    FIELD_IS_DETERMINISTIC,
    FIELD_SQL_DATA_ACCESS,
    FIELD_SQL_PATH,
    FIELD_SECURITY_TYPE,
    FIELD_CREATED,
    FIELD_LAST_ALTERED,
    FIELD_SQL_MODE,
    FIELD_ROUTINE_COMMENT,
    FIELD_DEFINER,
    FIELD_CHARACTER_SET_CLIENT,
    FIELD_COLLATION_CONNECTION,
    FIELD_DATABASE_COLLATION
  };

  Routines();

  static const Routines &instance();

  static const String_type &view_name() {
    static String_type s_view_name("ROUTINES");
    return s_view_name;
  }
  virtual const String_type &name() const { return Routines::view_name(); }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__ROUTINES_INCLUDED
