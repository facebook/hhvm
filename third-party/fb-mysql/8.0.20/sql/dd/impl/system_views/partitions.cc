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

#include "sql/dd/impl/system_views/partitions.h"

namespace dd {
namespace system_views {

const Partitions &Partitions::instance() {
  static Partitions *s_instance = new Partitions();
  return *s_instance;
}

Partitions::Partitions() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "tbl.name");
  m_target_def.add_field(FIELD_PARTITION_NAME, "PARTITION_NAME", "part.name");
  m_target_def.add_field(FIELD_SUBPARTITION_NAME, "SUBPARTITION_NAME",
                         "sub_part.name");
  m_target_def.add_field(FIELD_PARTITION_ORDINAL_POSITION,
                         "PARTITION_ORDINAL_POSITION", "part.number+1");
  m_target_def.add_field(FIELD_SUBPARTITION_ORDINAL_POSITION,
                         "SUBPARTITION_ORDINAL_POSITION", "sub_part.number+1");
  m_target_def.add_field(FIELD_PARTITION_METHOD, "PARTITION_METHOD",
                         "CASE tbl.partition_type"
                         "  WHEN 'HASH' THEN 'HASH' "
                         "  WHEN 'RANGE' THEN 'RANGE' "
                         "  WHEN 'LIST' THEN 'LIST' "
                         "  WHEN 'AUTO' THEN 'AUTO' "
                         "  WHEN 'KEY_51' THEN 'KEY' "
                         "  WHEN 'KEY_55' THEN 'KEY' "
                         "  WHEN 'LINEAR_KEY_51' THEN 'LINEAR KEY' "
                         "  WHEN 'LINEAR_KEY_55' THEN 'LINEAR KEY' "
                         "  WHEN 'LINEAR_HASH' THEN 'LINEAR HASH' "
                         "  WHEN 'RANGE_COLUMNS' THEN 'RANGE COLUMNS' "
                         "  WHEN 'LIST_COLUMNS' THEN 'LIST COLUMNS' "
                         "  ELSE NULL END");
  m_target_def.add_field(FIELD_SUBPARTITION_METHOD, "SUBPARTITION_METHOD",
                         "CASE tbl.subpartition_type"
                         "  WHEN 'HASH' THEN 'HASH' "
                         "  WHEN 'RANGE' THEN 'RANGE' "
                         "  WHEN 'LIST' THEN 'LIST' "
                         "  WHEN 'AUTO' THEN 'AUTO' "
                         "  WHEN 'KEY_51' THEN 'KEY' "
                         "  WHEN 'KEY_55' THEN 'KEY' "
                         "  WHEN 'LINEAR_KEY_51' THEN 'LINEAR KEY' "
                         "  WHEN 'LINEAR_KEY_55' THEN 'LINEAR KEY' "
                         "  WHEN 'LINEAR_HASH' THEN 'LINEAR HASH' "
                         "  WHEN 'RANGE_COLUMNS' THEN 'RANGE COLUMNS' "
                         "  WHEN 'LIST_COLUMNS' THEN 'LIST COLUMNS' "
                         "  ELSE NULL END");
  m_target_def.add_field(FIELD_PARTITION_EXPRESSION, "PARTITION_EXPRESSION",
                         "tbl.partition_expression_utf8");
  m_target_def.add_field(FIELD_SUBPARTITION_EXPRESSION,
                         "SUBPARTITION_EXPRESSION",
                         "tbl.subpartition_expression_utf8");
  m_target_def.add_field(FIELD_PARTITION_DESCRIPTION, "PARTITION_DESCRIPTION",
                         "part.description_utf8");
  m_target_def.add_field(FIELD_TABLE_ROWS, "TABLE_ROWS",
                         "INTERNAL_TABLE_ROWS(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_AVG_ROW_LENGTH, "AVG_ROW_LENGTH",
                         "INTERNAL_AVG_ROW_LENGTH(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_DATA_LENGTH, "DATA_LENGTH",
                         "INTERNAL_DATA_LENGTH(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_MAX_DATA_LENGTH, "MAX_DATA_LENGTH",
                         "INTERNAL_MAX_DATA_LENGTH(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_INDEX_LENGTH, "INDEX_LENGTH",
                         "INTERNAL_INDEX_LENGTH(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_DATA_FREE, "DATA_FREE",
                         "INTERNAL_DATA_FREE(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_CREATE_TIME, "CREATE_TIME", "tbl.created");
  m_target_def.add_field(FIELD_UPDATE_TIME, "UPDATE_TIME",
                         "INTERNAL_UPDATE_TIME(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_CHECK_TIME, "CHECK_TIME",
                         "INTERNAL_CHECK_TIME(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(FIELD_CHECKSUM, "CHECKSUM",
                         "INTERNAL_CHECKSUM(sch.name, tbl.name,"
                         "  IF(ISNULL(tbl.partition_type), tbl.engine, ''),"
                         "  tbl.se_private_id, tbl.hidden != 'Visible', "
                         "  IF(sub_part.name IS NULL, "
                         "     IF(part.name IS NULL, tbl.se_private_data, "
                         "part_ts.se_private_data),"
                         "     sub_part_ts.se_private_data), 0, 0,"
                         "  IFNULL(sub_part.name, part.name))");
  m_target_def.add_field(
      FIELD_PARTITION_COMMENT, "PARTITION_COMMENT",
      "IF(sub_part.name IS NULL,"
      "IFNULL(part.comment,''), IFNULL(sub_part.comment,''))");
  m_target_def.add_field(
      FIELD_NODEGROUP, "NODEGROUP",
      "IF(part.name IS NULL, '', INTERNAL_GET_PARTITION_NODEGROUP( "
      "IF(sub_part.name IS NULL, part.options, sub_part.options)))");
  m_target_def.add_field(FIELD_TABLESPACE_NAME, "TABLESPACE_NAME",
                         "IFNULL(sub_part_ts.name, part_ts.name)");

  m_target_def.add_from("mysql.tables tbl");
  m_target_def.add_from("JOIN mysql.schemata sch ON sch.id=tbl.schema_id");
  m_target_def.add_from(
      "JOIN mysql.catalogs cat "
      "ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "LEFT JOIN mysql.table_partitions part ON "
      "part.table_id=tbl.id");
  m_target_def.add_from(
      "LEFT JOIN mysql.table_partitions sub_part "
      "ON sub_part.parent_partition_id=part.id");
  m_target_def.add_from(
      "LEFT JOIN mysql.tablespaces part_ts "
      "ON part_ts.id=part.tablespace_id");
  m_target_def.add_from(
      "LEFT JOIN mysql.tablespaces sub_part_ts "
      "ON sub_part.tablespace_id IS NOT NULL "
      "AND sub_part_ts.id=sub_part.tablespace_id");

  m_target_def.add_where("CAN_ACCESS_TABLE(sch.name, tbl.name)");
  m_target_def.add_where("AND IS_VISIBLE_DD_OBJECT(tbl.hidden)");
  m_target_def.add_where("AND part.parent_partition_id IS NULL");
}

}  // namespace system_views
}  // namespace dd
