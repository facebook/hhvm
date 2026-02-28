/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_INFO_SCHEMA_TABLESPACE_STATS_INCLUDED
#define DD_INFO_SCHEMA_TABLESPACE_STATS_INCLUDED

#include "sql/handler.h"  // ha_tablespace_statistics
#include "sql_string.h"   // String

class THD;
struct TABLE_LIST;

namespace dd {
namespace info_schema {

// Tablespace statistics that are cached.
enum class enum_tablespace_stats_type {
  TS_ID,
  TS_TYPE,
  TS_LOGFILE_GROUP_NAME,
  TS_LOGFILE_GROUP_NUMBER,
  TS_FREE_EXTENTS,
  TS_TOTAL_EXTENTS,
  TS_EXTENT_SIZE,
  TS_INITIAL_SIZE,
  TS_MAXIMUM_SIZE,
  TS_AUTOEXTEND_SIZE,
  TS_VERSION,
  TS_ROW_FORMAT,
  TS_DATA_FREE,
  TS_STATUS,
  TS_EXTRA
};

/**
  The class hold dynamic table statistics for a table.
  This cache is used by internal UDF's defined for the purpose
  of INFORMATION_SCHEMA queries which retrieve dynamic table
  statistics. The class caches statistics for just one table.

  Overall aim of introducing this cache is to avoid making
  multiple calls to same SE API to retrieve the statistics.
*/

class Tablespace_statistics {
 public:
  Tablespace_statistics() : m_found_error(false) {}

  /**
    Check if the stats are cached for given tablespace_name and file_name.

    @param tablespace_name       - Tablespace name.
    @param file_name             - File name.

    @return true if stats are cached, else false.
  */
  bool is_stat_cached(const String &tablespace_name, const String &file_name) {
    return (m_key == form_key(tablespace_name, file_name));
  }

  /**
    Store the statistics form the given handler

    @param tablespace_name  - Tablespace name.
    @param file_name        - File name.
    @param stats            - ha_tablespace_statistics.
  */
  void cache_stats(const String &tablespace_name, const String &file_name,
                   ha_tablespace_statistics &stats) {
    m_stats = stats;
    m_found_error = false;
    set_stat_cached(tablespace_name, file_name);
  }

  /**
    Read dynamic tablespace statistics from SE API OR by reading cached
    statistics from SELECT_LEX.

    @param thd                     - Current thread.
    @param tablespace_name_ptr     - Tablespace name of which we need stats.
    @param file_name_ptr           - File name.
    @param engine_name_ptr         - Engine name.
    @param ts_se_private_data      - Tablespace se private data.

    @return true if statistics were not fetched from SE, otherwise false.
  */
  bool read_stat(THD *thd, const String &tablespace_name_ptr,
                 const String &file_name_ptr, const String &engine_name_ptr,
                 const char *ts_se_private_data);

  // Invalidate the cache.
  void invalidate_cache(void) {
    m_key.clear();
    m_found_error = false;
  }

  /**
    Mark that error was found for the given key. The combination of
    tablespace and file name forms the key.

    @param tablespace_name  - Tablespace name.
    @param file_name        - File name.
  */
  void mark_as_error_found(const String &tablespace_name,
                           const String &file_name) {
    m_stats = {};
    m_found_error = true;
    m_key = form_key(tablespace_name, file_name);
  }

  /**
    Return statistics of the a given type.

    @param      stype  Type of statistics requested.
    @param[out] result Value for stype.
  */
  void get_stat(enum_tablespace_stats_type stype, ulonglong *result);

  void get_stat(enum_tablespace_stats_type stype, dd::String_type *result);

 private:
  /**
    Mark the cache as valid for a given table. This creates a key for the
    cache element. We store just a single table statistics in this cache.

    @param tablespace_name    - Tablespace name.
    @param file_name          - File name.
  */
  void set_stat_cached(const String &tablespace_name, const String &file_name) {
    m_key = form_key(tablespace_name, file_name);
  }

  /**
    Build a key representating the table for which stats are cached.

    @param tablespace_name          - Tablespace name.
    @param file_name                - File name.

    @returns String_type representing the key.
  */
  String_type form_key(const String &tablespace_name, const String &file_name) {
    return String_type(tablespace_name.ptr()) + String_type(file_name.ptr());
  }

  /**
    Check if we have seen a error.

    @param tablespace_name  - Tablespace name.
    @param file_name        - File name.

    @returns true if there is error reported.
             false if not.
  */
  inline bool check_error_for_key(const String &tablespace_name,
                                  const String &file_name) {
    if (is_stat_cached(tablespace_name, file_name) && m_found_error)
      return true;

    return false;
  }

  /**
    Read dynamic tablespace statistics from SE API.

    @param thd                     - Current thread.
    @param tablespace_name_ptr     - Tablespace name of which we need stats.
    @param file_name_ptr           - File name.
    @param engine_name_ptr         - Engine name.
    @param ts_se_private_data      - Tablespace se private data.

    @return true if statistics were not fetched from SE, otherwise false.
  */
  bool read_stat_from_SE(THD *thd, const String &tablespace_name_ptr,
                         const String &file_name_ptr,
                         const String &engine_name_ptr,
                         const char *ts_se_private_data);

 private:
  // The cache key
  String_type m_key;  // Format '<tablespace_name>'

  // Error found when reading statistics.
  bool m_found_error;

 public:
  // Cached statistics.
  ha_tablespace_statistics m_stats;
};

}  // namespace info_schema
}  // namespace dd

#endif  // DD_INFO_SCHEMA_TABLESPACE_STATS_INCLUDED
