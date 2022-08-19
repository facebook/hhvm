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

#ifndef DD_SYSTEM_VIEWS__FILES_INCLUDED
#define DD_SYSTEM_VIEWS__FILES_INCLUDED

#include "sql/dd/impl/system_views/system_view_definition_impl.h"
#include "sql/dd/impl/system_views/system_view_impl.h"

namespace dd {
namespace system_views {

/*
  The class representing INFORMATION_SCHEMA.FILES system view definition.
*/
class Files : public System_view_impl<System_view_select_definition_impl> {
 public:
  enum enum_fields {
    FIELD_FILE_ID,
    FIELD_FILE_NAME,
    FIELD_FILE_TYPE, /* ndb/innodb */
    FIELD_TABLESPACE_NAME,
    FIELD_TABLE_CATALOG,
    FIELD_TABLE_SCHEMA,
    FIELD_TABLE_NAME,
    FIELD_LOGFILE_GROUP_NAME,   /* ndb */
    FIELD_LOGFILE_GROUP_NUMBER, /* ndb */
    FIELD_ENGINE,
    FIELD_FULLTEXT_KEYS,
    FIELD_DELETED_ROWS,
    FIELD_UPDATE_COUNT,
    FIELD_FREE_EXTENTS,    /* ndb/innodb */
    FIELD_TOTAL_EXTENTS,   /* ndb/innodb */
    FIELD_EXTENT_SIZE,     /* ndb/innodb */
    FIELD_INITIAL_SIZE,    /* ndb/innodb */
    FIELD_MAXIMUM_SIZE,    /* ndb/innodb */
    FIELD_AUTOEXTEND_SIZE, /* ndb/innodb */
    FIELD_CREATION_TIME,
    FIELD_LAST_UPDATE_TIME,
    FIELD_LAST_ACCESS_TIME,
    FIELD_RECOVER_TIME,
    FIELD_TRANSACTION_COUNTER,
    FIELD_VERSION,    /* ndb */
    FIELD_ROW_FORMAT, /* ndb */
    FIELD_TABLE_ROWS,
    FIELD_AVG_ROW_LENGTH,
    FIELD_DATA_LENGTH,
    FIELD_MAX_DATA_LENGTH,
    FIELD_INDEX_LENGTH,
    FIELD_DATA_FREE, /* ndb/innodb */
    FIELD_CREATE_TIME,
    FIELD_UPDATE_TIME,
    FIELD_CHECK_TIME,
    FIELD_CHECKSUM,
    FIELD_STATUS, /* ndb/innodb */
    FIELD_EXTRA   /* no-innodb/ndb*/
  };

  Files();

  static const Files &instance();

  static const String_type &view_name() {
    static String_type s_view_name("FILES");
    return s_view_name;
  }
  virtual const String_type &name() const { return Files::view_name(); }
};

}  // namespace system_views
}  // namespace dd

#endif  // DD_SYSTEM_VIEWS__FILES_INCLUDED
