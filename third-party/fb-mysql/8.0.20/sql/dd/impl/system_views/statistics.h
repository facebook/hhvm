/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_SYSTEM_VIEWS__STATISTICS_INCLUDED
#define DD_SYSTEM_VIEWS__STATISTICS_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"
#include "sql/dd/string_type.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.STATISTICS system view
  definition.
*/
class Statistics_base
    : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields {
    FIELD_TABLE_CATALOG,
    FIELD_TABLE_SCHEMA,
    FIELD_TABLE_NAME,
    FIELD_NON_UNIQUE,
    FIELD_INDEX_SCHEMA,
    FIELD_INDEX_NAME,
    FIELD_SEQ_IN_INDEX,
    FIELD_COLUMN_NAME,
    FIELD_COLLATION,
    FIELD_CARDINALITY,
    FIELD_SUB_PART,
    FIELD_PACKED,
    FIELD_NULLABLE,
    FIELD_INDEX_TYPE,
    FIELD_COMMENT,
    FIELD_INDEX_COMMENT,
    FIELD_IS_VISIBLE,
    FIELD_INDEX_ORDINAL_POSITION,
    FIELD_COLUMN_ORDINAL_POSITION,
    FIELD_EXPRESSION
  };

  Statistics_base();

  virtual const String_type &name() const = 0;
};

/*
 The class representing INFORMATION_SCHEMA.STATISTICS system view definition
*/
class Statistics : public Statistics_base {
 public:
  Statistics();

  static const Statistics_base &instance();

  static const String_type &view_name() {
    static String_type s_view_name("STATISTICS");
    return s_view_name;
  }

  virtual const String_type &name() const { return Statistics::view_name(); }
};

/*
 The class represents system view definition used by SHOW STATISTICS when
*/
class Show_statistics : public Statistics {
 public:
  Show_statistics();

  static const Statistics_base &instance();

  static const String_type &view_name() {
    static String_type s_view_name("SHOW_STATISTICS");
    return s_view_name;
  }

  virtual const String_type &name() const {
    return Show_statistics::view_name();
  }

  // This view definition is hidden from user.
  virtual bool hidden() const { return true; }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__STATISTICS_INCLUDED
