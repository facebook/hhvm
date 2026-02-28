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

#ifndef DD_TABLES__TABLE_PARTITIONS_INCLUDED
#define DD_TABLES__TABLE_PARTITIONS_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/impl/types/object_table_impl.h"  // dd::Object_table_impl
#include "sql/dd/object_id.h"                     // dd::Object_id
#include "sql/dd/string_type.h"

class THD;

namespace dd {
class Object_key;
class Raw_record;

namespace tables {

///////////////////////////////////////////////////////////////////////////

class Table_partitions : public Object_table_impl {
 public:
  static const Table_partitions &instance();

  static const CHARSET_INFO *name_collation();

  enum enum_fields {
    FIELD_ID,
    FIELD_TABLE_ID,
    FIELD_PARENT_PARTITION_ID,
    FIELD_NUMBER,
    FIELD_NAME,
    FIELD_DESCRIPTION_UTF8,
    FIELD_ENGINE,
    FIELD_COMMENT,
    FIELD_OPTIONS,
    FIELD_SE_PRIVATE_DATA,
    FIELD_SE_PRIVATE_ID,
    FIELD_TABLESPACE_ID,
    NUMBER_OF_FIELDS  // Always keep this entry at the end of the enum
  };

  enum enum_indexes {
    INDEX_PK_ID = static_cast<uint>(Common_index::PK_ID),
    INDEX_UK_TABLE_ID_NAME = static_cast<uint>(Common_index::UK_NAME),
    INDEX_UK_TABLE_ID_PARENT_PARTITION_ID_NUMBER,
    INDEX_UK_ENGINE_SE_PRIVATE_ID,
    INDEX_K_ENGINE,
    INDEX_K_TABLESPACE_ID,
    INDEX_K_PARENT_PARTITION_ID
  };

  enum enum_foreign_keys {
    FK_TABLE_ID,
    FK_TABLESPACE_ID,
    FK_PARENT_PARTITION_ID
  };

  Table_partitions();

  static Object_key *create_key_by_table_id(Object_id table_id);

  static Object_key *create_key_by_parent_partition_id(
      Object_id table_id, Object_id parent_partition_id);

  static ulonglong read_table_id(const Raw_record &r);

  static Object_key *create_se_private_key(const String_type &engine,
                                           Object_id se_private_id);

  static bool get_partition_table_id(THD *thd, const String_type &engine,
                                     ulonglong se_private_id, Object_id *oid);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace tables
}  // namespace dd

#endif  // DD_TABLES__TABLE_PARTITIONS_INCLUDED
