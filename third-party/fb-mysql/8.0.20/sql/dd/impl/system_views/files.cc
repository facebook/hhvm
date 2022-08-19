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

#include "sql/dd/impl/system_views/files.h"

namespace dd {
namespace system_views {

const Files &Files::instance() {
  static Files *s_instance = new Files();
  return *s_instance;
}

Files::Files() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_FILE_ID, "FILE_ID",
      "INTERNAL_TABLESPACE_ID(ts.name, tsf.file_name, ts.engine, "
      "ts.se_private_data)");
  /*
    Fix for Bug#26518545 might change below code. Mainly because the
    difference in DD not containing './' and filename maintained by InnoDB
    containing './' might change after the fix.
   */
  m_target_def.add_field(
      FIELD_FILE_NAME, "FILE_NAME",
      "REPLACE(IF( INSTR('./', LEFT(tsf.file_name,1)) = 0 AND "
      "            SUBSTRING(tsf.file_name,2,1) != ':', "
      "CONCAT('./', tsf.file_name), tsf.file_name), '\\\\', '/')");
  m_target_def.add_field(
      FIELD_FILE_TYPE, "FILE_TYPE",
      "INTERNAL_TABLESPACE_TYPE(ts.name, tsf.file_name, ts.engine, "
      "ts.se_private_data)");
  m_target_def.add_field(FIELD_TABLESPACE_NAME, "TABLESPACE_NAME", "ts.name");
  m_target_def.add_field(FIELD_TABLE_CATALOG, "TABLE_CATALOG", "''");
  m_target_def.add_field(FIELD_TABLE_SCHEMA, "TABLE_SCHEMA", "NULL");
  m_target_def.add_field(FIELD_TABLE_NAME, "TABLE_NAME", "NULL");
  m_target_def.add_field(FIELD_LOGFILE_GROUP_NAME, "LOGFILE_GROUP_NAME",
                         "INTERNAL_TABLESPACE_LOGFILE_GROUP_NAME(ts.name,"
                         "tsf.file_name, ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_LOGFILE_GROUP_NUMBER, "LOGFILE_GROUP_NUMBER",
                         "INTERNAL_TABLESPACE_LOGFILE_GROUP_NUMBER(ts.name,"
                         "tsf.file_name, ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_ENGINE, "ENGINE", "ts.engine");
  m_target_def.add_field(FIELD_FULLTEXT_KEYS, "FULLTEXT_KEYS", "NULL");
  m_target_def.add_field(FIELD_DELETED_ROWS, "DELETED_ROWS", "NULL");
  m_target_def.add_field(FIELD_UPDATE_COUNT, "UPDATE_COUNT", "NULL");
  m_target_def.add_field(
      FIELD_FREE_EXTENTS, "FREE_EXTENTS",
      "INTERNAL_TABLESPACE_FREE_EXTENTS(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(
      FIELD_TOTAL_EXTENTS, "TOTAL_EXTENTS",
      "INTERNAL_TABLESPACE_TOTAL_EXTENTS(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(
      FIELD_EXTENT_SIZE, "EXTENT_SIZE",
      "INTERNAL_TABLESPACE_EXTENT_SIZE(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(
      FIELD_INITIAL_SIZE, "INITIAL_SIZE",
      "INTERNAL_TABLESPACE_INITIAL_SIZE(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(
      FIELD_MAXIMUM_SIZE, "MAXIMUM_SIZE",
      "INTERNAL_TABLESPACE_MAXIMUM_SIZE(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(
      FIELD_AUTOEXTEND_SIZE, "AUTOEXTEND_SIZE",
      "INTERNAL_TABLESPACE_AUTOEXTEND_SIZE(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_CREATION_TIME, "CREATION_TIME", "NULL");
  m_target_def.add_field(FIELD_LAST_UPDATE_TIME, "LAST_UPDATE_TIME", "NULL");
  m_target_def.add_field(FIELD_LAST_ACCESS_TIME, "LAST_ACCESS_TIME", "NULL");
  m_target_def.add_field(FIELD_RECOVER_TIME, "RECOVER_TIME", "NULL");
  m_target_def.add_field(FIELD_TRANSACTION_COUNTER, "TRANSACTION_COUNTER",
                         "NULL");
  m_target_def.add_field(FIELD_VERSION, "VERSION",
                         "INTERNAL_TABLESPACE_VERSION(ts.name,"
                         "tsf.file_name, ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_ROW_FORMAT, "ROW_FORMAT",
                         "INTERNAL_TABLESPACE_ROW_FORMAT(ts.name,"
                         "tsf.file_name, ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_TABLE_ROWS, "TABLE_ROWS", "NULL");
  m_target_def.add_field(FIELD_AVG_ROW_LENGTH, "AVG_ROW_LENGTH", "NULL");
  m_target_def.add_field(FIELD_DATA_LENGTH, "DATA_LENGTH", "NULL");
  m_target_def.add_field(FIELD_MAX_DATA_LENGTH, "MAX_DATA_LENGTH", "NULL");
  m_target_def.add_field(FIELD_INDEX_LENGTH, "INDEX_LENGTH", "NULL");
  m_target_def.add_field(
      FIELD_DATA_FREE, "DATA_FREE",
      "INTERNAL_TABLESPACE_DATA_FREE(ts.name, tsf.file_name, "
      "ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_CREATE_TIME, "CREATE_TIME", "NULL");
  m_target_def.add_field(FIELD_UPDATE_TIME, "UPDATE_TIME", "NULL");
  m_target_def.add_field(FIELD_CHECK_TIME, "CHECK_TIME", "NULL");
  m_target_def.add_field(FIELD_CHECKSUM, "CHECKSUM", "NULL");
  m_target_def.add_field(FIELD_STATUS, "STATUS",
                         "INTERNAL_TABLESPACE_STATUS(ts.name, tsf.file_name, "
                         "ts.engine, ts.se_private_data)");
  m_target_def.add_field(FIELD_EXTRA, "EXTRA",
                         "INTERNAL_TABLESPACE_EXTRA(ts.name, tsf.file_name, "
                         "ts.engine, ts.se_private_data)");

  m_target_def.add_from("mysql.tablespaces ts");
  m_target_def.add_from(
      "JOIN mysql.tablespace_files tsf "
      "ON ts.id=tsf.tablespace_id");
}

}  // namespace system_views
}  // namespace dd
