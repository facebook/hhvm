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

#include "sql/dd/info_schema/table_stats.h"  // dd::info_schema::*

#include "my_time.h"  // TIME_to_ulonglong_datetime
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd.h"          // dd::create_object
#include "sql/dd/impl/utils.h"  // dd::my_time_t_to_ull_datetime()
#include "sql/dd/properties.h"
#include "sql/dd/types/index_stat.h"             // dd::Index_stat
#include "sql/dd/types/table_stat.h"             // dd::Table_stat
#include "sql/debug_sync.h"                      // DEBUG_SYNC
#include "sql/error_handler.h"                   // Info_schema_error_handler
#include "sql/mysqld.h"                          // super_read_only
#include "sql/partition_info.h"                  // partition_info
#include "sql/partitioning/partition_handler.h"  // Partition_handler
#include "sql/sql_base.h"                        // open_tables_for_query
#include "sql/sql_class.h"                       // THD
#include "sql/sql_lex.h"
#include "sql/sql_show.h"  // make_table_list
#include "sql/sql_time.h"  // my_longlong_to_datetime_with_warn
#include "sql/strfunc.h"   // lex_cstring_handle
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit
#include "sql/tztime.h"       // Time_zone

namespace {

/**
  Update the data in the mysql.index_stats table if,
    - information_schema_stats_expiry is not ZERO OR

    - None of the innodb_read_only, read_only, super_read_only
      or transactional_read_only is ON, OR

    - Table is not a partitioned table.

    - Table is not a performance schema table.

  @param thd           - Thread ID
  @param schema_name   - The schema name.

  @returns true if we can update the statistics, otherwise false.
*/
inline bool can_persist_I_S_dynamic_statistics(THD *thd,
                                               const char *schema_name,
                                               const char *partition_name) {
  handlerton *ddse = ha_resolve_by_legacy_type(thd, DB_TYPE_INNODB);
  if (ddse == nullptr || ddse->is_dict_readonly()) return false;

  return (thd->variables.information_schema_stats_expiry &&
          !thd->variables.transaction_read_only && !super_read_only &&
          !thd->in_sub_stmt && !read_only && !partition_name &&
          (strcmp(schema_name, "performance_schema") != 0));
}

inline bool is_persistent_statistics_expired(
    THD *thd, const ulonglong &cached_timestamp) {
  // Consider it as expired if timestamp or timeout is ZERO.
  if (!cached_timestamp || !thd->variables.information_schema_stats_expiry)
    return true;

  // Convert longlong time to MYSQL_TIME format
  MYSQL_TIME cached_mysql_time;
  my_longlong_to_datetime_with_warn(cached_timestamp, &cached_mysql_time,
                                    MYF(0));

  /*
    Convert MYSQL_TIME to epoc second according to local time_zone as
    cached_timestamp value is with local time_zone
  */
  my_time_t cached_epoc_secs;
  bool not_used;
  cached_epoc_secs =
      thd->variables.time_zone->TIME_to_gmt_sec(&cached_mysql_time, &not_used);
  long curtime = thd->query_start_in_secs();
  ulonglong time_diff = curtime - static_cast<long>(cached_epoc_secs);

  return (time_diff > thd->variables.information_schema_stats_expiry);
}

/**
  A RAII to used to allow updates in the DD tables mysql.index_stats and
  mysql.index_stats.

  These tables  are marked as system tables and cannot be updated directly by
  the user. Also, we always use  non-locking reads to read DD tables in I_S
  queries. Active transaction in this  thread or other connections can only
  do non-locking reads on the these dictionary tables. Hence, deadlocks are
  not possible when doing updates to index_stats/table_stats. So it is safe
  to use attachable read-write transaction for this purpose.
*/

class Update_I_S_statistics_ctx {
 public:
  Update_I_S_statistics_ctx(THD *thd) : m_thd(thd) {
    m_thd->begin_attachable_rw_transaction();
    /*
      Turn Autocommit OFF to avoid assert in dictionary write.
      Autocommit was switched ON by Attachable transaction.
    */
    thd->variables.option_bits &= ~OPTION_AUTOCOMMIT;
    thd->variables.option_bits |= OPTION_NOT_AUTOCOMMIT;
  }

  ~Update_I_S_statistics_ctx() { m_thd->end_attachable_transaction(); }

 private:
  THD *m_thd;
};

/**
  Store the statistics into DD tables mysql.table_stats and
  mysql.index_stats.
*/
template <typename T>
bool store_statistics_record(THD *thd, T *object) {
  Update_I_S_statistics_ctx ctx(thd);
  Implicit_substatement_state_guard substatement_guard(thd);

  // Store tablespace object in dictionary
  if (thd->dd_client()->store(object)) {
    trans_rollback_stmt(thd);
    trans_rollback(thd);
    /**
      It is ok to ignore ER_DUP_ENTRY, because there is possibility
      that another thread would have updated statistics in high
      concurrent environment. See Bug#29948755 for more information.
    */
    if (thd->get_stmt_da()->mysql_errno() == ER_DUP_ENTRY) {
      thd->clear_error();
      return false;
    }
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}

template bool store_statistics_record(THD *thd, dd::Table_stat *);
template bool store_statistics_record(THD *thd, dd::Index_stat *);

inline void setup_table_stats_record(THD *thd, dd::Table_stat *obj,
                                     dd::String_type schema_name,
                                     dd::String_type table_name,
                                     const ha_statistics &stats,
                                     ulonglong checksum, bool has_checksum,
                                     bool has_autoinc) {
  obj->set_schema_name(schema_name);
  obj->set_table_name(table_name);
  obj->set_table_rows(stats.records);
  obj->set_avg_row_length(stats.mean_rec_length);
  obj->set_data_length(stats.data_file_length);
  obj->set_max_data_length(stats.max_data_file_length);
  obj->set_index_length(stats.index_file_length);
  obj->set_data_free(stats.delete_length);

  if (stats.update_time) {
    obj->set_update_time(dd::my_time_t_to_ull_datetime(stats.update_time));
  }

  if (stats.check_time) {
    obj->set_check_time(dd::my_time_t_to_ull_datetime(stats.check_time));
  }

  if (has_checksum) obj->set_checksum(checksum);

  if (has_autoinc) obj->set_auto_increment(stats.auto_increment_value);

  // Store statement start time.
  obj->set_cached_time(
      dd::my_time_t_to_ull_datetime(thd->query_start_in_secs()));
}

inline void setup_index_stats_record(THD *thd, dd::Index_stat *obj,
                                     dd::String_type schema_name,
                                     dd::String_type table_name,
                                     dd::String_type index_name,
                                     dd::String_type column_name,
                                     ulonglong records) {
  obj->set_schema_name(schema_name);
  obj->set_table_name(table_name);
  obj->set_index_name(index_name);
  obj->set_column_name(column_name);
  obj->set_cardinality(records);

  // Calculate time to be stored as cached time.
  obj->set_cached_time(
      dd::my_time_t_to_ull_datetime(thd->query_start_in_secs()));
}

/**
  Get dynamic table statistics of a table and store them into
  mysql.table_stats.

  @param thd             Thread.
  @param stats           Statistics object
  @param schema_name_ptr Schema name
  @param table_name_ptr  Table name
  @param checksum        Checksum value

  @returns false on success, otherwise true.
*/
static bool persist_i_s_table_stats(THD *thd, const ha_statistics &stats,
                                    const String &schema_name_ptr,
                                    const String &table_name_ptr,
                                    const ulonglong &checksum) {
  // Create a object to be stored.
  std::unique_ptr<dd::Table_stat> ts_obj(dd::create_object<dd::Table_stat>());

  setup_table_stats_record(
      thd, ts_obj.get(),
      dd::String_type(schema_name_ptr.ptr(), schema_name_ptr.length()),
      dd::String_type(table_name_ptr.ptr(), table_name_ptr.length()), stats,
      checksum, true, true);

  return store_statistics_record(thd, ts_obj.get());
}

/**
  Get dynamic index statistics of a table and store them into
  mysql.index_stats.

  @param thd             Thread.
  @param schema_name_ptr Schema name
  @param table_name_ptr  Table name
  @param index_name_ptr  Index name
  @param column_name_ptr Column name
  @param records          Value for cardinality

  @returns false on success, otherwise true.
*/

static bool persist_i_s_index_stats(THD *thd, const String &schema_name_ptr,
                                    const String &table_name_ptr,
                                    const String &index_name_ptr,
                                    const String &column_name_ptr,
                                    ulonglong records) {
  // Create a object to be stored.
  std::unique_ptr<dd::Index_stat> obj(dd::create_object<dd::Index_stat>());

  setup_index_stats_record(
      thd, obj.get(),
      dd::String_type(schema_name_ptr.ptr(), schema_name_ptr.length()),
      dd::String_type(table_name_ptr.ptr(), table_name_ptr.length()),
      dd::String_type(index_name_ptr.ptr(), index_name_ptr.length()),
      dd::String_type(column_name_ptr.ptr(), column_name_ptr.length()),
      records);

  return store_statistics_record(thd, obj.get());
}

}  // Anonymous namespace

namespace dd {
namespace info_schema {

bool update_table_stats(THD *thd, TABLE_LIST *table) {
  // Update the object properties
  HA_CREATE_INFO create_info;

  TABLE *analyze_table = table->table;
  handler *file = analyze_table->file;
  if (analyze_table->file->info(HA_STATUS_VARIABLE | HA_STATUS_TIME |
                                HA_STATUS_VARIABLE_EXTRA | HA_STATUS_AUTO) != 0)
    return true;

  file->update_create_info(&create_info);

  // Create a object to be stored.
  std::unique_ptr<Table_stat> ts_obj(create_object<Table_stat>());

  setup_table_stats_record(
      thd, ts_obj.get(), dd::String_type(table->db, strlen(table->db)),
      dd::String_type(table->alias, strlen(table->alias)), file->stats,
      file->checksum(), file->ha_table_flags() & (ulong)HA_HAS_CHECKSUM,
      analyze_table->found_next_number_field);

  // Store the object
  if (thd->dd_client()->store(ts_obj.get())) {
    my_error(ER_UNABLE_TO_STORE_STATISTICS, MYF(0), "table");
    return true;
  }

  return false;
}

bool update_index_stats(THD *thd, TABLE_LIST *table) {
  // Update the object properties
  TABLE *analyze_table = table->table;
  KEY *key_info = analyze_table->s->key_info;
  if (analyze_table->file->info(HA_STATUS_VARIABLE | HA_STATUS_TIME |
                                HA_STATUS_VARIABLE_EXTRA | HA_STATUS_AUTO) != 0)
    return true;

  // Create a object to be stored.
  std::unique_ptr<Index_stat> obj(create_object<Index_stat>());

  for (uint i = 0; i < analyze_table->s->keys; i++, key_info++) {
    KEY_PART_INFO *key_part = key_info->key_part;
    const char *str;
    ha_rows records;
    for (uint j = 0; j < key_info->user_defined_key_parts; j++, key_part++) {
      str = (key_part->field ? key_part->field->field_name : "?unknown field?");

      KEY *key = analyze_table->key_info + i;
      if (key->has_records_per_key(j)) {
        double recs =
            (analyze_table->file->stats.records / key->records_per_key(j));
        records = static_cast<longlong>(round(recs));
      } else
        records = -1;  // Treated as NULL

      setup_index_stats_record(
          thd, obj.get(), dd::String_type(table->db, strlen(table->db)),
          dd::String_type(table->alias, strlen(table->alias)),
          dd::String_type(key_info->name, strlen(key_info->name)),
          dd::String_type(str, strlen(str)), records);

      // Store the object
      if (thd->dd_client()->store(obj.get())) {
        my_error(ER_UNABLE_TO_STORE_STATISTICS, MYF(0), "index");
        return true;
      }

    }  // Key part info

  }  // Keys

  return false;
}

// Convert IS db to lowercase and table case upper case.
bool convert_table_name_case(char *db, char *table_name) {
  if (db && is_infoschema_db(db)) {
    my_casedn_str(system_charset_info, db);
    if (table_name && strncmp(table_name, "ndb", 3))
      my_caseup_str(system_charset_info, table_name);

    return true;
  }

  return false;
}

// Returns the required statistics from the cache.
ulonglong Table_statistics::get_stat(ha_statistics &stat,
                                     enum_table_stats_type stype) {
  switch (stype) {
    case enum_table_stats_type::TABLE_ROWS:
      return (stat.records);

    case enum_table_stats_type::TABLE_AVG_ROW_LENGTH:
      return (stat.mean_rec_length);

    case enum_table_stats_type::DATA_LENGTH:
      return (stat.data_file_length);

    case enum_table_stats_type::MAX_DATA_LENGTH:
      return (stat.max_data_file_length);

    case enum_table_stats_type::INDEX_LENGTH:
      return (stat.index_file_length);

    case enum_table_stats_type::DATA_FREE:
      return (stat.delete_length);

    case enum_table_stats_type::AUTO_INCREMENT:
      return (stat.auto_increment_value);

    case enum_table_stats_type::CHECKSUM:
      return (get_checksum());

    case enum_table_stats_type::TABLE_UPDATE_TIME:
      return (stat.update_time);

    case enum_table_stats_type::CHECK_TIME:
      return (stat.check_time);

    default:
      DBUG_ASSERT(!"Should not hit here");
  }

  return 0;
}

// Read dynamic table statistics from SE by opening the user table
// provided OR by reading cached statistics from SELECT_LEX.
ulonglong Table_statistics::read_stat(
    THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
    const String &index_name_ptr, const char *partition_name,
    const String &column_name_ptr, uint index_ordinal_position,
    uint column_ordinal_position, const String &engine_name_ptr,
    Object_id se_private_id, const char *ts_se_private_data,
    const char *tbl_se_private_data, const ulonglong &table_stat_data,
    const ulonglong &cached_timestamp, enum_table_stats_type stype) {
  DBUG_TRACE;
  ulonglong result;

  // Stop we have see and error already for this table.
  if (check_error_for_key(schema_name_ptr, table_name_ptr)) return 0;

  // Check if we can directly use the value passed from mysql.stats tables.
  if (!is_persistent_statistics_expired(thd, cached_timestamp)) {
    return table_stat_data;
  }

  /*
    Get statistics from cache, if available.
    If other statistics except cardinality is present in Statistics cache.
    return the value from cache.
  */
  if (stype != enum_table_stats_type::INDEX_COLUMN_CARDINALITY &&
      is_stat_cached_in_mem(schema_name_ptr, table_name_ptr, partition_name))
    return get_stat(stype);

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

  /*
    Check if engine supports fetching table statistics.
    The engine name for partitioned table is empty string, because the
    hton->get_table_statistics is not yet implemented to support
    partitioned table.
  */
  plugin_ref tmp_plugin = ha_resolve_by_name_raw(
      thd, lex_cstring_handle(dd::String_type(engine_name_ptr.ptr())));
  handlerton *hton = nullptr;
  bool hton_implements_get_statistics = false;
  if (tmp_plugin && (hton = plugin_data<handlerton *>(tmp_plugin))) {
    if (stype == enum_table_stats_type::INDEX_COLUMN_CARDINALITY) {
      hton_implements_get_statistics =
          hton->get_index_column_cardinality != nullptr;
    } else {
      hton_implements_get_statistics = hton->get_table_statistics != nullptr;
    }
  }

  // Try to get statistics without opening the table.
  if (!partition_name && hton_implements_get_statistics)
    result = read_stat_from_SE(
        thd, schema_name_ptr, table_name_ptr, index_name_ptr, column_name_ptr,
        index_ordinal_position, column_ordinal_position, se_private_id,
        ts_se_private_data, tbl_se_private_data, stype, hton);
  else
    result = read_stat_by_open_table(
        thd, schema_name_ptr, table_name_ptr, index_name_ptr, partition_name,
        column_name_ptr, column_ordinal_position, stype);

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

  return result;
}

// Fetch stats from SE
ulonglong Table_statistics::read_stat_from_SE(
    THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
    const String &index_name_ptr, const String &column_name_ptr,
    uint index_ordinal_position, uint column_ordinal_position,
    Object_id se_private_id, const char *ts_se_private_data,
    const char *tbl_se_private_data, enum_table_stats_type stype,
    handlerton *hton) {
  DBUG_TRACE;

  ulonglong return_value = 0;

  DBUG_EXECUTE_IF("information_schema_fetch_table_stats",
                  DBUG_ASSERT(strncmp(table_name_ptr.ptr(), "fts", 3)););

  // No engines implement these statistics retrieval. We always return zero.
  if (stype == enum_table_stats_type::CHECK_TIME ||
      stype == enum_table_stats_type::CHECKSUM)
    return 0;

  //
  // Get statistics from SE
  //
  ha_statistics ha_stat;
  uint error = 0;

  // Acquire MDL_EXPLICIT lock on table.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, schema_name_ptr.ptr(),
                   table_name_ptr.ptr(), MDL_SHARED_HIGH_PRIO, MDL_EXPLICIT);

  // Push deadlock error handler
  Info_schema_error_handler info_schema_error_handler(thd, &schema_name_ptr,
                                                      &table_name_ptr);
  thd->push_internal_handler(&info_schema_error_handler);
  if (thd->mdl_context.acquire_lock_nsec(
          &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
    error = -1;
  }
  thd->pop_internal_handler();

  DEBUG_SYNC(thd, "after_acquiring_mdl_shared_to_fetch_stats");

  if (error == 0) {
    error = -1;

    // Prepare dd::Properties objects for se_private_data and send it to SE.
    std::unique_ptr<dd::Properties> ts_se_private_data_obj(
        dd::Properties::parse_properties(ts_se_private_data ? ts_se_private_data
                                                            : ""));
    std::unique_ptr<dd::Properties> tbl_se_private_data_obj(
        dd::Properties::parse_properties(
            tbl_se_private_data ? tbl_se_private_data : ""));

    DBUG_ASSERT(tbl_se_private_data_obj.get() && ts_se_private_data_obj.get());

    //
    // Read statistics from SE
    //
    return_value = -1;

    if (stype == enum_table_stats_type::INDEX_COLUMN_CARDINALITY &&
        !hton->get_index_column_cardinality(
            schema_name_ptr.ptr(), table_name_ptr.ptr(), index_name_ptr.ptr(),
            index_ordinal_position, column_ordinal_position, se_private_id,
            &return_value)) {
      error = 0;
    } else if (!hton->get_table_statistics(
                   schema_name_ptr.ptr(), table_name_ptr.ptr(), se_private_id,
                   *ts_se_private_data_obj.get(),
                   *tbl_se_private_data_obj.get(),
                   HA_STATUS_VARIABLE | HA_STATUS_TIME |
                       HA_STATUS_VARIABLE_EXTRA | HA_STATUS_AUTO,
                   &ha_stat)) {
      error = 0;
    }

    // Release the lock we got
    thd->mdl_context.release_lock(mdl_request.ticket);
  }

  // Cache and return the statistics
  if (error == 0) {
    if (stype != enum_table_stats_type::INDEX_COLUMN_CARDINALITY) {
      cache_stats_in_mem(schema_name_ptr, table_name_ptr, ha_stat);

      /*
        Update table statistics in the cache.
        All engines return ZERO for checksum, we hardcode it here.
      */
      if (can_persist_I_S_dynamic_statistics(thd, schema_name_ptr.ptr(),
                                             nullptr) &&
          persist_i_s_table_stats(thd, m_stats, schema_name_ptr, table_name_ptr,
                                  0)) {
        error = -1;
      } else
        return_value = get_stat(ha_stat, stype);

    }
    /*
      Only cardinality is not stored in the cache.
      Update index statistics in the mysql.index_stats.
    */
    else if (can_persist_I_S_dynamic_statistics(thd, schema_name_ptr.ptr(),
                                                nullptr) &&
             persist_i_s_index_stats(thd, schema_name_ptr, table_name_ptr,
                                     index_name_ptr, column_name_ptr,
                                     return_value)) {
      error = -1;
    }

    if (error == 0) return return_value;
  }

  // If we have a error, push a warning and clear the DA.
  if (thd->is_error()) {
    /*
      Hide error for a non-existing table.
      For example, this error can occur when we use a where condition
      with a db name and table, but the table does not exist.
     */
    if (!(thd->get_stmt_da()->mysql_errno() == ER_NO_SUCH_TABLE) &&
        !(thd->get_stmt_da()->mysql_errno() == ER_WRONG_OBJECT))
      push_warning(thd, Sql_condition::SL_WARNING,
                   thd->get_stmt_da()->mysql_errno(),
                   thd->get_stmt_da()->message_text());

    store_error_message(schema_name_ptr, table_name_ptr, nullptr,
                        thd->get_stmt_da()->message_text());
    thd->clear_error();
  }

  return error;
}

// Fetch stats by opening the table.
ulonglong Table_statistics::read_stat_by_open_table(
    THD *thd, const String &schema_name_ptr, const String &table_name_ptr,
    const String &index_name_ptr, const char *partition_name,
    const String &column_name_ptr, uint column_ordinal_position,
    enum_table_stats_type stype) {
  DBUG_TRACE;
  ulonglong return_value = 0;
  ulonglong error = 0;
  ha_statistics ha_stat;

  DEBUG_SYNC(thd, "before_open_in_IS_query");

  //
  // Get statistics by opening the table
  //

  Info_schema_error_handler info_schema_error_handler(thd, &schema_name_ptr,
                                                      &table_name_ptr);
  Open_tables_backup open_tables_state_backup;
  thd->reset_n_backup_open_tables_state(&open_tables_state_backup, 0);

  Query_arena i_s_arena(thd->mem_root, Query_arena::STMT_REGULAR_EXECUTION);
  Query_arena *old_arena = thd->stmt_arena;
  thd->stmt_arena = &i_s_arena;
  Query_arena backup_arena;
  thd->swap_query_arena(i_s_arena, &backup_arena);

  LEX temp_lex, *lex;
  LEX *old_lex = thd->lex;
  thd->lex = lex = &temp_lex;
  thd->lex->m_IS_table_stats.set_read_stats_by_open(true);

  lex_start(thd);
  lex->context_analysis_only = CONTEXT_ANALYSIS_ONLY_VIEW;

  LEX_CSTRING db_name_lex_cstr, table_name_lex_cstr;
  if (lex_string_strmake(thd->mem_root, &db_name_lex_cstr,
                         schema_name_ptr.ptr(), schema_name_ptr.length()) ||
      lex_string_strmake(thd->mem_root, &table_name_lex_cstr,
                         table_name_ptr.ptr(), table_name_ptr.length())) {
    error = -1;
    goto end;
  }

  if (make_table_list(thd, lex->select_lex, db_name_lex_cstr,
                      table_name_lex_cstr)) {
    error = -1;
    goto end;
  }

  TABLE_LIST *table_list;
  table_list = lex->select_lex->table_list.first;
  table_list->required_type = dd::enum_table_type::BASE_TABLE;

  /*
    Let us set fake sql_command so views won't try to merge
    themselves into main statement. If we don't do this,
    SELECT * from information_schema.xxxx will cause problems.
    SQLCOM_SHOW_FIELDS is used because it satisfies
    'only_view_structure()'.
   */
  lex->sql_command = SQLCOM_SELECT;

  DBUG_EXECUTE_IF("simulate_kill_query_on_open_table",
                  DBUG_SET("+d,kill_query_on_open_table_from_tz_find"););

  // Push deadlock error handler.
  thd->push_internal_handler(&info_schema_error_handler);

  bool open_result;
  open_result = open_tables_for_query(
      thd, table_list,
      MYSQL_OPEN_IGNORE_FLUSH | MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL);

  thd->pop_internal_handler();

  DBUG_EXECUTE_IF("simulate_kill_query_on_open_table",
                  DBUG_SET("-d,kill_query_on_open_table_from_tz_find"););
  DEBUG_SYNC(thd, "after_open_table_mdl_shared_to_fetch_stats");

  if (!open_result && table_list->is_view_or_derived()) {
    open_result = table_list->resolve_derived(thd, false);
    if (!open_result) open_result = table_list->setup_materialized_derived(thd);
  }

  /*
    Restore old value of sql_command back as it is being looked at in
    process_table() function.
   */
  lex->sql_command = old_lex->sql_command;

  if (open_result) {
    DBUG_ASSERT(thd->is_error() || thd->is_killed());

    if (thd->is_error()) {
      /*
        Hide error for a non-existing table.
        For example, this error can occur when we use a where condition
        with a db name and table, but the table does not exist.
      */
      if (!(thd->get_stmt_da()->mysql_errno() == ER_NO_SUCH_TABLE) &&
          !(thd->get_stmt_da()->mysql_errno() == ER_WRONG_OBJECT))
        push_warning(thd, Sql_condition::SL_WARNING,
                     thd->get_stmt_da()->mysql_errno(),
                     thd->get_stmt_da()->message_text());

      store_error_message(schema_name_ptr, table_name_ptr, partition_name,
                          thd->get_stmt_da()->message_text());
      thd->clear_error();
    } else {
      /*
        Table open fails even when query or connection is killed. In this
        case Diagnostics_area might not be set. So just returning error from
        here. Query is later terminated by call to send_kill_message() when
        we check thd->killed flag.
      */
      error = -1;
    }

    goto end;
  } else if (!table_list->is_view() && !table_list->schema_table) {
    ha_checksum check_sum = 0;
    bool have_partition_checksum = false;

    // Get statistics for just single partition.
    Partition_handler *part_handler =
        table_list->table->file->get_partition_handler();
    if (partition_name && part_handler) {
      partition_info *part_info = table_list->table->part_info;
      DBUG_ASSERT(part_info);

      uint part_id;
      if (part_info->get_part_elem(partition_name, &part_id) &&
          part_id != NOT_A_PARTITION_ID) {
        part_handler->get_dynamic_partition_info(&ha_stat, &check_sum, part_id);
        table_list->table->file->stats = ha_stat;
        have_partition_checksum = true;
      } else {
        my_error(ER_UNKNOWN_PARTITION, MYF(0), partition_name,
                 table_list->table->alias);
        error = -1;
      }
    }
    // Get statistics for whole table.
    else if (table_list->table->file->info(HA_STATUS_VARIABLE | HA_STATUS_TIME |
                                           HA_STATUS_VARIABLE_EXTRA |
                                           HA_STATUS_AUTO) != 0) {
      error = -1;
    }

    if (error) {
      if (thd->is_error()) {
        push_warning(thd, Sql_condition::SL_WARNING,
                     thd->get_stmt_da()->mysql_errno(),
                     thd->get_stmt_da()->message_text());

        store_error_message(schema_name_ptr, table_name_ptr, partition_name,
                            thd->get_stmt_da()->message_text());
        thd->clear_error();
      } else
        error = -1;

      goto end;
    }

    // If we are reading cardinality, just read and do not cache it.
    if (stype == enum_table_stats_type::INDEX_COLUMN_CARDINALITY) {
      TABLE *table = table_list->table;
      uint key_index = 0;

      // Search for key with the index name.
      while (key_index < table->s->keys) {
        if (!my_strcasecmp(system_charset_info,
                           (table->key_info + key_index)->name,
                           index_name_ptr.ptr()))
          break;

        key_index++;
      }

      KEY *key = table->s->key_info + key_index;

      // Calculate the cardinality.
      ha_rows records;
      if (key_index < table->s->keys &&
          key->has_records_per_key(column_ordinal_position)) {
        records = (table->file->stats.records /
                   key->records_per_key(column_ordinal_position));
        records = static_cast<longlong>(round(records));
      } else
        records = -1;  // Treated as NULL

      return_value = (ulonglong)records;

      // Update index statistics in the cache.
      if (can_persist_I_S_dynamic_statistics(thd, schema_name_ptr.ptr(),
                                             partition_name) &&
          persist_i_s_index_stats(thd, schema_name_ptr, table_name_ptr,
                                  index_name_ptr, column_name_ptr,
                                  (ulonglong)records)) {
        error = -1;
      }
    } else  // Get all statistics and cache them.
    {
      cache_stats_in_mem(schema_name_ptr, table_name_ptr, partition_name,
                         table_list->table->file);

      if (have_partition_checksum)
        set_checksum(static_cast<ulonglong>(check_sum));

      // Update table statistics in the cache.
      if (can_persist_I_S_dynamic_statistics(thd, schema_name_ptr.ptr(),
                                             partition_name) &&
          persist_i_s_table_stats(thd, m_stats, schema_name_ptr, table_name_ptr,
                                  m_checksum)) {
        error = -1;
      }

      return_value = get_stat(stype);
    }
  } else {
    error = -1;
    goto end;
  }

end:
  lex->unit->cleanup(thd, true);

  /* Restore original LEX value, statement's arena and THD arena values. */
  lex_end(thd->lex);

  // Free items, before restoring backup_arena below.
  DBUG_ASSERT(i_s_arena.item_list() == nullptr);
  thd->free_items();

  /*
    For safety reset list of open temporary tables before closing
    all tables open within this Open_tables_state.
   */
  close_thread_tables(thd);
  /*
    Release metadata lock we might have acquired.
    See comment in fill_schema_table_from_frm() for details.
   */
  thd->mdl_context.rollback_to_savepoint(
      open_tables_state_backup.mdl_system_tables_svp);

  thd->lex = old_lex;

  thd->stmt_arena = old_arena;
  thd->swap_query_arena(backup_arena, &i_s_arena);

  thd->restore_backup_open_tables_state(&open_tables_state_backup);

  /*
    ER_LOCK_DEADLOCK is converted to ER_WARN_I_S_SKIPPED_TABLE by deadlock
    error handler used here.
    If rollback request is set by other deadlock error handlers then
    reset it here.
  */
  if (info_schema_error_handler.is_error_handled() &&
      thd->transaction_rollback_request)
    thd->transaction_rollback_request = false;

  return error == 0 ? return_value : error;
}

}  // namespace info_schema
}  // namespace dd
