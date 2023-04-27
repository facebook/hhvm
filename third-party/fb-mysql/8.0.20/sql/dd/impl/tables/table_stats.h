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

#ifndef DD_TABLES__TABLE_STATS_INCLUDED
#define DD_TABLES__TABLE_STATS_INCLUDED

#include <new>
#include <string>

#include "sql/dd/impl/raw/object_keys.h"  // Composite_char_key
#include "sql/dd/impl/types/entity_object_table_impl.h"
// Entity_object_table_impl
#include "sql/dd/impl/types/table_stat_impl.h"  // Table_stat
#include "sql/dd/string_type.h"
#include "sql/dd/types/table_stat.h"

namespace dd {

class Raw_record;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Table_stats : virtual public Entity_object_table_impl {
 public:
  Table_stats();

  static const Table_stats &instance();

  enum enum_fields {
    FIELD_SCHEMA_NAME,
    FIELD_TABLE_NAME,
    FIELD_TABLE_ROWS,
    FIELD_AVG_ROW_LENGTH,
    FIELD_DATA_LENGTH,
    FIELD_MAX_DATA_LENGTH,
    FIELD_INDEX_LENGTH,
    FIELD_DATA_FREE,
    FIELD_AUTO_INCREMENT,
    FIELD_CHECKSUM,
    FIELD_UPDATE_TIME,
    FIELD_CHECK_TIME,
    FIELD_CACHED_TIME,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes { INDEX_PK_SCHEMA_ID_TABLE_NAME };

  enum enum_foreign_keys {};

  virtual Table_stat *create_entity_object(const Raw_record &) const {
    return new (std::nothrow) Table_stat_impl();
  }

  static Table_stat::Name_key *create_object_key(const String_type &schema_name,
                                                 const String_type &table_name);
};

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__TABLE_STATS_INCLUDED
