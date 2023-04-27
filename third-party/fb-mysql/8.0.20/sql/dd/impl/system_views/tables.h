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

#ifndef DD_SYSTEM_VIEWS__TABLES_INCLUDED
#define DD_SYSTEM_VIEWS__TABLES_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"
#include "sql/dd/string_type.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.TABLES system view definition.
*/
class Tables_base
    : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields {
    FIELD_TABLE_CATALOG,
    FIELD_TABLE_SCHEMA,
    FIELD_TABLE_NAME,
    FIELD_TABLE_TYPE,
    FIELD_ENGINE,
    FIELD_VERSION,
    FIELD_ROW_FORMAT,
    FIELD_TABLE_ROWS,
    FIELD_AVG_ROW_LENGTH,
    FIELD_DATA_LENGTH,
    FIELD_MAX_DATA_LENGTH,
    FIELD_INDEX_LENGTH,
    FIELD_DATA_FREE,
    FIELD_AUTO_INCREMENT,
    FIELD_CREATE_TIME,
    FIELD_UPDATE_TIME,
    FIELD_CHECK_TIME,
    FIELD_TABLE_COLLATION,
    FIELD_CHECKSUM,
    FIELD_CREATE_OPTIONS,
    FIELD_TABLE_COMMENT
  };

  Tables_base();

  virtual const String_type &name() const = 0;
};

/*
 The class representing INFORMATION_SCHEMA.TABLES system view definition.
*/
class Tables : public Tables_base {
 public:
  Tables();

  static const Tables_base &instance();

  static const String_type &view_name() {
    static String_type s_view_name("TABLES");
    return s_view_name;
  }

  virtual const String_type &name() const { return Tables::view_name(); }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__TABLES_INCLUDED
