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

#include "sql/dd/info_schema/tablespace_stats.h"  // dd::info_schema::Tables...

#include "sql/dd/properties.h"
#include "sql/error_handler.h"  // Info_schema_error_handler
#include "sql/sql_class.h"      // THD
#include "sql/strfunc.h"        // lex_cstring_handle

namespace dd {
namespace info_schema {

// Returns the required statistics from the cache.
void Tablespace_statistics::get_stat(enum_tablespace_stats_type stype,
                                     ulonglong *result) {
  switch (stype) {
    case enum_tablespace_stats_type::TS_ID:
      *result = m_stats.m_id;
      return;

    case enum_tablespace_stats_type::TS_LOGFILE_GROUP_NUMBER:
      *result = m_stats.m_logfile_group_number;
      return;

    case enum_tablespace_stats_type::TS_FREE_EXTENTS:
      *result = m_stats.m_free_extents;
      return;

    case enum_tablespace_stats_type::TS_TOTAL_EXTENTS:
      *result = m_stats.m_total_extents;
      return;

    case enum_tablespace_stats_type::TS_EXTENT_SIZE:
      *result = m_stats.m_extent_size;
      return;

    case enum_tablespace_stats_type::TS_INITIAL_SIZE:
      *result = m_stats.m_initial_size;
      return;

    case enum_tablespace_stats_type::TS_MAXIMUM_SIZE:
      *result = m_stats.m_maximum_size;
      return;

    case enum_tablespace_stats_type::TS_AUTOEXTEND_SIZE:
      *result = m_stats.m_autoextend_size;
      return;

    case enum_tablespace_stats_type::TS_VERSION:
      *result = m_stats.m_version;
      return;

    case enum_tablespace_stats_type::TS_DATA_FREE:
      *result = m_stats.m_data_free;
      return;

    default:
    case enum_tablespace_stats_type::TS_TYPE:
    case enum_tablespace_stats_type::TS_LOGFILE_GROUP_NAME:
    case enum_tablespace_stats_type::TS_ROW_FORMAT:
    case enum_tablespace_stats_type::TS_STATUS:
    case enum_tablespace_stats_type::TS_EXTRA:
      DBUG_ASSERT(!"Should not hit here");
      return;
  }

  DBUG_ASSERT(!"Should not hit here");
  return;
}

// Returns the required statistics from the cache.
void Tablespace_statistics::get_stat(enum_tablespace_stats_type stype,
                                     String_type *result) {
  switch (stype) {
    case enum_tablespace_stats_type::TS_TYPE:
      *result = m_stats.m_type;
      return;

    case enum_tablespace_stats_type::TS_LOGFILE_GROUP_NAME:
      *result = m_stats.m_logfile_group_name;
      return;

    case enum_tablespace_stats_type::TS_ROW_FORMAT:
      *result = m_stats.m_row_format;
      return;

    case enum_tablespace_stats_type::TS_STATUS:
      *result = m_stats.m_status;
      return;

    case enum_tablespace_stats_type::TS_EXTRA:
      *result = m_stats.m_extra;
      return;

    case enum_tablespace_stats_type::TS_ID:
    case enum_tablespace_stats_type::TS_LOGFILE_GROUP_NUMBER:
    case enum_tablespace_stats_type::TS_FREE_EXTENTS:
    case enum_tablespace_stats_type::TS_TOTAL_EXTENTS:
    case enum_tablespace_stats_type::TS_EXTENT_SIZE:
    case enum_tablespace_stats_type::TS_INITIAL_SIZE:
    case enum_tablespace_stats_type::TS_MAXIMUM_SIZE:
    case enum_tablespace_stats_type::TS_AUTOEXTEND_SIZE:
    case enum_tablespace_stats_type::TS_VERSION:
    case enum_tablespace_stats_type::TS_DATA_FREE:
    default:
      DBUG_ASSERT(!"Should not hit here");

      return;
  }
  DBUG_ASSERT(!"Should not hit here");

  return;
}

/*
  Read dynamic table statistics from SE OR by reading cached statistics
  from SELECT_LEX.
*/
bool Tablespace_statistics::read_stat(THD *thd,
                                      const String &tablespace_name_ptr,
                                      const String &file_name_ptr,
                                      const String &engine_name_ptr,
                                      const char *ts_se_private_data) {
  DBUG_TRACE;
  bool error = false;

  // Stop we have see and error already for this table.
  if (check_error_for_key(tablespace_name_ptr, file_name_ptr)) return true;

  //
  // Get statistics from cache, if available
  //

  if (is_stat_cached(tablespace_name_ptr, file_name_ptr)) return false;

  // NOTE: read_stat() may generate many "useless" warnings, which will be
  // ignored afterwards. On the other hand, there might be "useful"
  // warnings, which should be presented to the user. Diagnostics_area usually
  // stores no more than THD::variables.max_error_count warnings.
  // The problem is that "useless warnings" may occupy all the slots in the
  // Diagnostics_area, so "useful warnings" get rejected. In order to avoid
  // that problem we create a Diagnostics_area instance, which is capable of
  // storing "unlimited" number of warnings.
  Diagnostics_area *da = thd->get_stmt_da();
  Diagnostics_area tmp_da(true);

  // Don't copy existing conditions from the old DA so we don't get them twice
  // when we call copy_non_errors_from_da below.
  thd->push_diagnostics_area(&tmp_da, false);
  error = read_stat_from_SE(thd, tablespace_name_ptr, file_name_ptr,
                            engine_name_ptr, ts_se_private_data);
  thd->pop_diagnostics_area();

  // Pass an error if any.
  if (!thd->is_error() && tmp_da.is_error()) {
    da->set_error_status(tmp_da.mysql_errno(), tmp_da.message_text(),
                         tmp_da.returned_sqlstate());
    da->push_warning(thd, tmp_da.mysql_errno(), tmp_da.returned_sqlstate(),
                     Sql_condition::SL_ERROR, tmp_da.message_text());
  }

  // Pass warnings (if any).
  //
  // Filter out warnings with SL_ERROR level, because they
  // correspond to the errors which were filtered out in fill_table().
  da->copy_non_errors_from_da(thd, &tmp_da);

  return error;
}

// Fetch stats from SE
bool Tablespace_statistics::read_stat_from_SE(THD *thd,
                                              const String &tablespace_name_ptr,
                                              const String &file_name_ptr,
                                              const String &engine_name_ptr,
                                              const char *ts_se_private_data) {
  DBUG_TRACE;

  //
  // Get statistics from the SE
  //
  ha_tablespace_statistics ha_tablespace_stat;

  // Acquire MDL_EXPLICIT lock on table.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLESPACE, "",
                   tablespace_name_ptr.ptr(), MDL_SHARED, MDL_EXPLICIT);

  // Push deadlock error handler
  bool error = false;
  Info_schema_error_handler info_schema_error_handler(thd,
                                                      &tablespace_name_ptr);
  thd->push_internal_handler(&info_schema_error_handler);

  // Check if engine supports fetching tablespace statistics.
  plugin_ref tmp_plugin = ha_resolve_by_name_raw(
      thd, lex_cstring_handle(dd::String_type(engine_name_ptr.ptr())));
  handlerton *hton = nullptr;
  if (!tmp_plugin) {
    my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), engine_name_ptr.ptr());
    error = true;
  } else if (!(hton = plugin_data<handlerton *>(tmp_plugin)) ||
             !hton->get_tablespace_statistics) {
    DBUG_ASSERT(!hton->get_tablespace_statistics);
    my_error(ER_NOT_IMPLEMENTED_GET_TABLESPACE_STATISTICS, MYF(0),
             engine_name_ptr.ptr());
    error = true;
  }

  error = error || thd->mdl_context.acquire_lock_nsec(
                       &mdl_request, thd->variables.lock_wait_timeout_nsec);
  thd->pop_internal_handler();

  if (!error) {
    // Prepare dd::Properties object for se_private_data and send it to SE.
    std::unique_ptr<dd::Properties> ts_se_private_data_obj(
        dd::Properties::parse_properties(ts_se_private_data ? ts_se_private_data
                                                            : ""));

    DBUG_ASSERT(ts_se_private_data_obj.get());

    //
    // Read statistics from SE
    //

    error = hton->get_tablespace_statistics(
        tablespace_name_ptr.ptr(), file_name_ptr.ptr(),
        *ts_se_private_data_obj.get(), &ha_tablespace_stat);

    // Release the lock we got
    thd->mdl_context.release_lock(mdl_request.ticket);
  }

  if (!error) {
    // Cache statistics.
    cache_stats(tablespace_name_ptr, file_name_ptr, ha_tablespace_stat);
  }

  if (thd->is_error()) {
    push_warning(thd, Sql_condition::SL_WARNING,
                 thd->get_stmt_da()->mysql_errno(),
                 thd->get_stmt_da()->message_text());
    thd->clear_error();
    mark_as_error_found(tablespace_name_ptr, file_name_ptr);
  }

  return error;
}

}  // namespace info_schema
}  // namespace dd
