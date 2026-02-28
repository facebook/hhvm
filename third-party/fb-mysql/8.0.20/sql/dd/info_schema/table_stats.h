/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_INFO_SCHEMA_TABLE_STATS_INCLUDED
#define DD_INFO_SCHEMA_TABLE_STATS_INCLUDED

#include <sys/types.h>
#include <string>

#include "sql/dd/object_id.h"    // Object_id
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/handler.h"         // ha_statistics
#include "sql_string.h"          // String

class THD;
struct TABLE_LIST;

namespace dd {
namespace info_schema {

/**
  Get dynamic table statistics of a table and store them into
  mysql.table_stats.

  @param thd   Thread.
  @param table TABLE_LIST pointing to table info.

  @returns false on success.
           true on failure.
*/
bool update_table_stats(THD *thd, TABLE_LIST *table);

/**
  Get dynamic index statistics of a table and store them into
  mysql.index_stats.

  @param thd   Thread.
  @param table TABLE_LIST pointing to table info.

  @returns false on success.
           true on failure.
*/
bool update_index_stats(THD *thd, TABLE_LIST *table);

/**
  If the db is 'information_schema' then convert 'db' to
  lowercase and 'table_name' to upper case. Mainly because all
  information schema tables are stored in upper case in server.

  @param db          Database name
  @param table_name  Table name.

  @returns true if the conversion was done.
           false if not.
*/
bool convert_table_name_case(char *db, char *table_name);

// Statistics that are cached.
enum class enum_table_stats_type {
  TABLE_ROWS,
  TABLE_AVG_ROW_LENGTH,
  DATA_LENGTH,
  MAX_DATA_LENGTH,
  INDEX_LENGTH,
  DATA_FREE,
  AUTO_INCREMENT,
  CHECKSUM,
  TABLE_UPDATE_TIME,
  CHECK_TIME,
  INDEX_COLUMN_CARDINALITY
};

/**
  The class hold dynamic table statistics for a table.
  This cache is used by internal UDF's defined for the purpose
  of INFORMATION_SCHEMA queries which retrieve dynamic table
  statistics. The class caches statistics for just one table.

  Overall aim of introducing this cache is to avoid making
  multiple calls to same SE API to retrieve the statistics.
*/

class Table_statistics {
 public:
  Table_statistics() : m_checksum(0), m_read_stats_by_open(false) {}

  /**
    Check if the stats are cached for given db.table_name.

    @param db_name          - Schema name.
    @param table_name       - Table name.
    @param partition_name   - Partition name.

    @return true if stats are cached, else false.
  */
  bool is_stat_cached_in_mem(const String &db_name, const String &table_name,
                             const char *partition_name) {
    return (m_key == form_key(db_name, table_name, partition_name));
  }

  bool is_stat_cached_in_mem(const String &db_name, const String &table_name) {
    return is_stat_cached_in_mem(db_name, table_name, nullptr);
  }

  /**
    Store the statistics form the given handler

    @param db_name          - Schema name.
    @param table_name       - Table name.
    @param partition_name   - Partition name.
    @param file             - Handler object for the table.
  */
  void cache_stats_in_mem(const String &db_name, const String &table_name,
                          const char *partition_name, handler *file) {
    m_stats = file->stats;
    m_checksum = file->checksum();
    m_error.clear();
    set_stat_cached(db_name, table_name, partition_name);
  }

  void cache_stats_in_mem(const String &db_name, const String &table_name,
                          handler *file) {
    cache_stats_in_mem(db_name, table_name, nullptr, file);
  }

  /**
    Store the statistics

    @param db_name          - Schema name.
    @param table_name       - Table name.
    @param partition_name   - Partition name.
    @param stats            - ha_statistics of the table.
  */
  void cache_stats_in_mem(const String &db_name, const String &table_name,
                          const char *partition_name, ha_statistics &stats) {
    m_stats = stats;
    m_checksum = 0;
    m_error.clear();
    set_stat_cached(db_name, table_name, partition_name);
  }

  void cache_stats_in_mem(const String &db_name, const String &table_name,
                          ha_statistics &stats) {
    cache_stats_in_mem(db_name, table_name, nullptr, stats);
  }

  /**
    @brief
    Read dynamic table/index statistics from SE by opening the user table
    provided OR by reading cached statistics from SELECT_LEX.

    @param thd                     - Current thread.
    @param schema_name_ptr         - Schema name of table.
    @param table_name_ptr          - Table name of which we need stats.
    @param index_name_ptr          - Index name of which we need stats.
    @param partition_name          - Partition name.
    @param column_name_ptr         - Column name for index.
    @param index_ordinal_position  - Ordinal position of index in table.
    @param column_ordinal_position - Ordinal position of column in table.
    @param engine_name_ptr         - Engine of the table.
    @param se_private_id           - se_private_id of the table.
    @param ts_se_private_data      - Tablespace SE private data.
    @param tbl_se_private_data     - Table SE private data.
    @param table_stat_data         - Cached data from mysql.table_stats /
                                       mysql.index_stats table
    @param cached_time             - Timestamp value when data was cached.
    @param stype                   - Enum specifying the stat we are
                                     interested to read.

    @return ulonglong representing value for the status being read.
  */
  ulonglong read_stat(
      THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
      const String &index_name_ptr, const char *partition_name,
      const String &column_name_ptr, uint index_ordinal_position,
      uint column_ordinal_position, const String &engine_name_ptr,
      dd::Object_id se_private_id, const char *ts_se_private_data,
      const char *tbl_se_private_data, const ulonglong &table_stat_data,
      const ulonglong &cached_time, enum_table_stats_type stype);

  // Fetch table stats. Invokes the above method.
  ulonglong read_stat(
      THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
      const String &engine_name_ptr, const char *partition_name,
      dd::Object_id se_private_id, const char *ts_se_private_data,
      const char *tbl_se_private_data, const ulonglong &table_stat_data,
      const ulonglong &cached_time, enum_table_stats_type stype) {
    const String tmp;
    return read_stat(thd, schema_name_ptr, table_name_ptr, tmp, partition_name,
                     tmp, 0, 0, engine_name_ptr, se_private_id,
                     ts_se_private_data, tbl_se_private_data, table_stat_data,
                     cached_time, stype);
  }

  // Invalidate the cache.
  void invalidate_cache(void) {
    m_key.clear();
    m_error.clear();
  }

  // Get error string. Its empty if a error is not reported.
  inline String_type error() { return m_error; }

  /**
    Set error string for the given key. The combination of (db, table and
    partition name) forms the key.

    @param db_name          - Schema name.
    @param table_name       - Table name.
    @param partition_name   - Partition name.
    @param error_msg        - Error message.

    @note We store the error message so that the error message is shown in
    I_S.TABLES.COMMENT field. Apart from storing the error message, the
    below function resets the statistics, this will make sure,

     1. We do not invoke open_tables_for_query() again for other
        dynamic columns that are fetch from the current row being
        processed.

     2. We will not see junk values for statistics in results.
  */
  void store_error_message(const String &db_name, const String &table_name,
                           const char *partition_name,
                           const String_type &error_msg) {
    m_stats = {};
    m_checksum = 0;
    m_error = error_msg;
    m_key = form_key(db_name, table_name, partition_name);
  }

  /**
    Check if we have seen a error.

    @param db_name     Database name.
    @param table_name  Table name.

    @returns true if there is error reported.
             false if not.
  */
  inline bool check_error_for_key(const String &db_name,
                                  const String &table_name) {
    if (is_stat_cached_in_mem(db_name, table_name) && !m_error.empty())
      return true;

    return false;
  }

  /// Check if open table in progress.
  bool is_reading_stats_by_open() const { return m_read_stats_by_open; }

 private:
  /**
    Read dynamic table/index statistics from SE API's OR by reading
    cached statistics from SELECT_LEX.

    @param thd                     - Current thread.
    @param schema_name_ptr         - Schema name of table.
    @param table_name_ptr          - Table name of which we need stats.
    @param index_name_ptr          - Index name of which we need stats.
    @param column_name_ptr         - Column name for index.
    @param index_ordinal_position  - Ordinal position of index in table.
    @param column_ordinal_position - Ordinal position of column in table.
    @param se_private_id           - se_private_id of the table.
    @param ts_se_private_data      - Tablespace SE private data.
    @param tbl_se_private_data     - Table SE private data.
    @param stype                   - Enum specifying the stat we are
                                     interested to read.
    @param hton                    - Handle to SE for the given table.

    @return ulonglong representing value for the status being read.
  */
  ulonglong read_stat_from_SE(
      THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
      const String &index_name_ptr, const String &column_name_ptr,
      uint index_ordinal_position, uint column_ordinal_position,
      dd::Object_id se_private_id, const char *ts_se_private_data,
      const char *tbl_se_private_data, enum_table_stats_type stype,
      handlerton *hton);

  /**
    Read dynamic table/index statistics by opening the table OR by reading
    cached statistics from SELECT_LEX.

    @param thd                     - Current thread.
    @param schema_name_ptr         - Schema name of table.
    @param table_name_ptr          - Table name of which we need stats.
    @param index_name_ptr          - Index name of which we need stats.
    @param column_name_ptr         - Column name for index.
    @param column_ordinal_position - Ordinal position of column in table.
    @param partition_name          - Partition name.
    @param stype                   - Enum specifying the stat we are
                                     interested to read.

    @return ulonglong representing value for the status being read.
  */
  ulonglong read_stat_by_open_table(THD *thd, const String &schema_name_ptr,
                                    const String &table_name_ptr,
                                    const String &index_name_ptr,
                                    const char *partition_name,
                                    const String &column_name_ptr,
                                    uint column_ordinal_position,
                                    enum_table_stats_type stype);

  /**
    Mark the cache as valid for a given table. This creates a key for the
    cache element. We store just a single table statistics in this cache.

    @param db_name             - Database name.
    @param table_name          - Table name.
    @param partition_name      - Partition name.
  */
  void set_stat_cached(const String &db_name, const String &table_name,
                       const char *partition_name) {
    m_key = form_key(db_name, table_name, partition_name);
  }

  void set_stat_cached(const String &db_name, const String &table_name) {
    set_stat_cached(db_name, table_name, nullptr);
  }

  /**
    Build a key representing the table for which stats are cached.

    @param db_name             - Database name.
    @param table_name          - Table name.
    @param partition_name      - Partition name.

    @returns String_type representing the key.
  */
  String_type form_key(const String &db_name, const String &table_name,
                       const char *partition_name) {
    return String_type(db_name.ptr()) + "." + String_type(table_name.ptr()) +
           (partition_name ? ("." + String_type(partition_name)) : "");
  }

  /**
    Return statistics of the a given type.

    @param stat   ha_statistics for the current cached table.
    @param stype  Type of statistics requested.

    @returns ulonglong statistics value.
  */
  ulonglong get_stat(ha_statistics &stat, enum_table_stats_type stype);
  inline ulonglong get_stat(enum_table_stats_type stype) {
    return get_stat(m_stats, stype);
  }

  /// Set checksum
  void set_checksum(ulonglong &&checksum) { m_checksum = checksum; }

  /// Get checksum
  ulonglong get_checksum() const { return m_checksum; }

  /// Set open table in progress.
  void set_read_stats_by_open(bool status) { m_read_stats_by_open = status; }

 private:
  // The cache key
  String_type m_key;  // Format '<db_name>.<table_name>'

  // Error found when reading statistics.
  String_type m_error;

  // Table checksum value retrieved from SE.
  ulonglong m_checksum;

  /*
    Status if opening a table is in progress to read statistics.

    This is used by heap table, to avoid write a command "DELETE FROM
    TABLE" to binlog just after server restart. See open_table_entry_fini()
    for more info.
  */
  bool m_read_stats_by_open;

 public:
  // Cached statistics.
  ha_statistics m_stats;
};

}  // namespace info_schema
}  // namespace dd

#endif  // DD_INFO_SCHEMA_TABLE_STATS_INCLUDED
