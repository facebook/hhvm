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

#include "sql/dd/impl/system_views/statistics.h"

#include <string>

#include "sql/stateless_allocator.h"

namespace dd {
namespace system_views {

const Statistics_base &Statistics::instance() {
  static Statistics_base *s_instance = new Statistics();
  return *s_instance;
}

const Statistics_base &Show_statistics::instance() {
  static Statistics_base *s_instance = new Show_statistics();
  return *s_instance;
}

Statistics_base::Statistics_base() {
  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME",
                         "tbl.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(
      FIELD_NON_UNIQUE, "NON_UNIQUE",
      "IF (idx.type = 'PRIMARY' OR idx.type = 'UNIQUE',0,1)");
  m_target_def.add_field(FIELD_INDEX_SCHEMA, "INDEX_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_INDEX_NAME, "INDEX_NAME",
                         "idx.name COLLATE utf8_tolower_ci");
  m_target_def.add_field(FIELD_SEQ_IN_INDEX, "SEQ_IN_INDEX",
                         "icu.ordinal_position");
  m_target_def.add_field(
      FIELD_COLUMN_NAME, "COLUMN_NAME",
      "IF (col.hidden = 'SQL', NULL, col.name COLLATE utf8_tolower_ci)");
  m_target_def.add_field(FIELD_COLLATION, "COLLATION",
                         "CASE WHEN icu.order = 'DESC' THEN 'D' "
                         "WHEN icu.order = 'ASC'  THEN 'A' "
                         "ELSE NULL END");
  m_target_def.add_field(FIELD_SUB_PART, "SUB_PART",
                         "GET_DD_INDEX_SUB_PART_LENGTH(icu.length,"
                         "col.type, col.char_length, col.collation_id,"
                         "idx.type)");
  m_target_def.add_field(FIELD_PACKED, "PACKED", "NULL");
  m_target_def.add_field(FIELD_NULLABLE, "NULLABLE",
                         "IF (col.is_nullable = 1, 'YES','')");
  m_target_def.add_field(FIELD_INDEX_TYPE, "INDEX_TYPE",
                         "CASE WHEN idx.type = 'SPATIAL' THEN 'SPATIAL' "
                         "WHEN idx.algorithm = 'SE_PRIVATE' THEN '' "
                         "ELSE idx.algorithm END ");
  m_target_def.add_field(
      FIELD_COMMENT, "COMMENT",
      "IF (idx.type = 'PRIMARY' OR idx.type = 'UNIQUE', "
      "    '',IF(INTERNAL_KEYS_DISABLED(tbl.options),'disabled', ''))");
  m_target_def.add_field(FIELD_INDEX_COMMENT, "INDEX_COMMENT", "idx.comment");
  m_target_def.add_field(FIELD_IS_VISIBLE, "IS_VISIBLE",
                         "IF (idx.is_visible, 'YES', 'NO')");

  m_target_def.add_field(
      FIELD_EXPRESSION, "EXPRESSION",
      "IF (col.hidden = 'SQL', col.generation_expression_utf8, NULL)");

  m_target_def.add_from("mysql.index_column_usage icu");
  m_target_def.add_from("JOIN mysql.indexes idx ON idx.id=icu.index_id");
  m_target_def.add_from("JOIN mysql.tables tbl ON idx.table_id=tbl.id");
  m_target_def.add_from("JOIN mysql.columns col ON icu.column_id=col.id");
  m_target_def.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll "
      "ON tbl.collation_id=coll.id");

  m_target_def.add_where("CAN_ACCESS_TABLE(sch.name, tbl.name)");
  m_target_def.add_where(
      "AND IS_VISIBLE_DD_OBJECT(tbl.hidden, "
      "idx.hidden OR icu.hidden)");
}

Statistics::Statistics() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_CARDINALITY, "CARDINALITY",
      "INTERNAL_INDEX_COLUMN_CARDINALITY(sch.name, tbl.name, idx.name,"
      "col.name, idx.ordinal_position,"
      "icu.ordinal_position,"
      "IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
      "tbl.se_private_id,"
      "tbl.hidden != 'Visible' OR idx.hidden OR icu.hidden,"
      "COALESCE(stat.cardinality, CAST(-1 AS UNSIGNED)),"
      "COALESCE(CAST(stat.cached_time as UNSIGNED), 0))");

  m_target_def.add_from(
      "LEFT JOIN mysql.index_stats stat"
      "  ON tbl.name=stat.table_name"
      " AND sch.name=stat.schema_name"
      " AND idx.name=stat.index_name"
      " AND col.name=stat.column_name");
}

Show_statistics::Show_statistics() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_INDEX_ORDINAL_POSITION, "INDEX_ORDINAL_POSITION",
                         "idx.ordinal_position");
  m_target_def.add_field(FIELD_COLUMN_ORDINAL_POSITION,
                         "COLUMN_ORDINAL_POSITION", "icu.ordinal_position");
}

}  // namespace system_views
}  // namespace dd
