/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/table.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include "ft_global.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_pointer_arithmetic.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_thread_local.h"
#include "myisam.h"  // MI_MAX_KEY_LENGTH
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_table.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_table.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysql_version.h"  // MYSQL_VERSION_ID
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // acl_getroot
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"                      // mysql_bin_log
#include "sql/dd/cache/dictionary_client.h"  // dd::cache_Dictionary_client
#include "sql/dd/dd.h"                       // dd::get_dictionary
#include "sql/dd/dictionary.h"               // dd::Dictionary
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/table.h"  // dd::Table
#include "sql/dd/types/view.h"   // dd::View
#include "sql/debug_sync.h"      // DEBUG_SYNC
#include "sql/derror.h"          // ER_THD
#include "sql/error_handler.h"   // Strict_error_handler
#include "sql/field.h"
#include "sql/filesort.h"  // filesort_free_buffers
#include "sql/gis/srid.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"    // and_conds
#include "sql/item_json_func.h"  // Item_func_array_cast
#include "sql/json_diff.h"       // Json_diff_vector
#include "sql/json_dom.h"        // Json_wrapper
#include "sql/json_path.h"
#include "sql/key.h"  // find_ref_key
#include "sql/log.h"
#include "sql/my_decimal.h"
#include "sql/mysqld.h"  // reg_ext key_file_frm ...
#include "sql/nested_join.h"
#include "sql/opt_trace.h"  // opt_trace_disable_if_no_security_...
#include "sql/opt_trace_context.h"
#include "sql/parse_file.h"      // sql_parse_prepare
#include "sql/partition_info.h"  // partition_info
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/query_result.h"  // Query_result
#include "sql/sql_base.h"
#include "sql/sql_check_constraint.h"  // Sql_table_check_constraint
#include "sql/sql_class.h"             // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_opt_exec_shared.h"
#include "sql/sql_parse.h"       // check_stack_overrun
#include "sql/sql_partition.h"   // mysql_unpack_partition
#include "sql/sql_plugin.h"      // plugin_unlock
#include "sql/sql_select.h"      // actual_key_parts
#include "sql/sql_table.h"       // build_table_filename
#include "sql/sql_tablespace.h"  // validate_tablespace_name())
#include "sql/strfunc.h"         // find_type
#include "sql/system_variables.h"
#include "sql/table_cache.h"               // table_cache_manager
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/trigger_def.h"
#include "sql_const.h"
#include "sql_string.h"
#include "template_utils.h"  // down_cast
#include "thr_mutex.h"
/* INFORMATION_SCHEMA name */
LEX_CSTRING INFORMATION_SCHEMA_NAME = {STRING_WITH_LEN("information_schema")};

/* PERFORMANCE_SCHEMA name */
LEX_CSTRING PERFORMANCE_SCHEMA_DB_NAME = {
    STRING_WITH_LEN("performance_schema")};

/* MYSQL_SCHEMA name */
LEX_CSTRING MYSQL_SCHEMA_NAME = {STRING_WITH_LEN("mysql")};

/* MYSQL_TABLESPACE name */
LEX_CSTRING MYSQL_TABLESPACE_NAME = {STRING_WITH_LEN("mysql")};

/* GENERAL_LOG name */
LEX_CSTRING GENERAL_LOG_NAME = {STRING_WITH_LEN("general_log")};

/* SLOW_LOG name */
LEX_CSTRING SLOW_LOG_NAME = {STRING_WITH_LEN("slow_log")};

/* RLI_INFO name */
LEX_CSTRING RLI_INFO_NAME = {STRING_WITH_LEN("slave_relay_log_info")};

/* MI_INFO name */
LEX_CSTRING MI_INFO_NAME = {STRING_WITH_LEN("slave_master_info")};

/* WORKER_INFO name */
LEX_CSTRING WORKER_INFO_NAME = {STRING_WITH_LEN("slave_worker_info")};

/* GTID_EXECUTED name */
LEX_CSTRING GTID_EXECUTED_NAME = {STRING_WITH_LEN("gtid_executed")};

/* Keyword for parsing generated column functions */
LEX_CSTRING PARSE_GCOL_KEYWORD = {STRING_WITH_LEN("parse_gcol_expr")};

/* Functions defined in this file */

static Item *create_view_field(THD *thd, TABLE_LIST *view, Item **field_ref,
                               const char *name,
                               Name_resolution_context *context);
static void open_table_error(THD *thd, TABLE_SHARE *share, int error,
                             int db_errno);

inline bool is_system_table_name(const char *name, size_t length);

/**************************************************************************
  Object_creation_ctx implementation.
**************************************************************************/

Object_creation_ctx *Object_creation_ctx::set_n_backup(THD *thd) {
  Object_creation_ctx *backup_ctx;
  DBUG_TRACE;

  backup_ctx = create_backup_ctx(thd);
  change_env(thd);

  return backup_ctx;
}

void Object_creation_ctx::restore_env(THD *thd,
                                      Object_creation_ctx *backup_ctx) {
  if (!backup_ctx) return;

  backup_ctx->change_env(thd);
  backup_ctx->delete_backup_ctx();
}

/**************************************************************************
  Default_object_creation_ctx implementation.
**************************************************************************/

Default_object_creation_ctx::Default_object_creation_ctx(THD *thd)
    : m_client_cs(thd->variables.character_set_client),
      m_connection_cl(thd->variables.collation_connection) {}

Default_object_creation_ctx::Default_object_creation_ctx(
    const CHARSET_INFO *client_cs, const CHARSET_INFO *connection_cl)
    : m_client_cs(client_cs), m_connection_cl(connection_cl) {}

Object_creation_ctx *Default_object_creation_ctx::create_backup_ctx(
    THD *thd) const {
  return new Default_object_creation_ctx(thd);
}

void Default_object_creation_ctx::delete_backup_ctx() { delete this; }

void Default_object_creation_ctx::change_env(THD *thd) const {
  thd->variables.character_set_client = m_client_cs;
  thd->variables.collation_connection = m_connection_cl;

  thd->update_charset();
}

/**************************************************************************
  View_creation_ctx implementation.
**************************************************************************/

View_creation_ctx *View_creation_ctx::create(THD *thd) {
  View_creation_ctx *ctx = new (thd->mem_root) View_creation_ctx(thd);

  return ctx;
}

/*************************************************************************/

View_creation_ctx *View_creation_ctx::create(THD *thd, TABLE_LIST *view) {
  View_creation_ctx *ctx = new (thd->mem_root) View_creation_ctx(thd);

  /* Throw a warning if there is NULL cs name. */

  if (!view->view_client_cs_name.str || !view->view_connection_cl_name.str) {
    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_VIEW_NO_CREATION_CTX,
                        ER_THD(thd, ER_VIEW_NO_CREATION_CTX), view->db,
                        view->table_name);

    ctx->m_client_cs = system_charset_info;
    ctx->m_connection_cl = system_charset_info;

    return ctx;
  }

  /* Resolve cs names. Throw a warning if there is unknown cs name. */

  bool invalid_creation_ctx;

  invalid_creation_ctx = resolve_charset(
      view->view_client_cs_name.str, system_charset_info, &ctx->m_client_cs);

  invalid_creation_ctx =
      resolve_collation(view->view_connection_cl_name.str, system_charset_info,
                        &ctx->m_connection_cl) ||
      invalid_creation_ctx;

  if (invalid_creation_ctx) {
    LogErr(WARNING_LEVEL, ER_VIEW_UNKNOWN_CHARSET_OR_COLLATION, view->db,
           view->table_name, view->view_client_cs_name.str,
           view->view_connection_cl_name.str);

    push_warning_printf(
        thd, Sql_condition::SL_NOTE, ER_VIEW_INVALID_CREATION_CTX,
        ER_THD(thd, ER_VIEW_INVALID_CREATION_CTX), view->db, view->table_name);
  }

  return ctx;
}

/*************************************************************************/

GRANT_INFO::GRANT_INFO() {
  grant_table = nullptr;
  version = 0;
  privilege = NO_ACCESS;
}

/**
  Returns pointer to '.frm' extension of the file name.

  @param name       file name

    Checks file name part starting with the rightmost '.' character,
    and returns it if it is equal to '.frm'.

  @todo
    It is a good idea to get rid of this function modifying the code
    to garantee that the functions presently calling fn_rext() always
    get arguments in the same format: either with '.frm' or without '.frm'.

  @return
    Pointer to the '.frm' extension. If there is no extension,
    or extension is not '.frm', pointer at the end of file name.
*/

char *fn_rext(char *name) {
  char *res = strrchr(name, '.');
  if (res && !strcmp(res, reg_ext)) return res;
  return name + strlen(name);
}

TABLE_CATEGORY get_table_category(const LEX_CSTRING &db,
                                  const LEX_CSTRING &name) {
  DBUG_ASSERT(db.str != nullptr);
  DBUG_ASSERT(name.str != nullptr);

  if (is_infoschema_db(db.str, db.length)) return TABLE_CATEGORY_INFORMATION;

  if (is_perfschema_db(db.str, db.length)) return TABLE_CATEGORY_PERFORMANCE;

  if ((db.length == MYSQL_SCHEMA_NAME.length) &&
      (my_strcasecmp(system_charset_info, MYSQL_SCHEMA_NAME.str, db.str) ==
       0)) {
    if (is_system_table_name(name.str, name.length))
      return TABLE_CATEGORY_SYSTEM;

    if ((name.length == GENERAL_LOG_NAME.length) &&
        (my_strcasecmp(system_charset_info, GENERAL_LOG_NAME.str, name.str) ==
         0))
      return TABLE_CATEGORY_LOG;

    if ((name.length == SLOW_LOG_NAME.length) &&
        (my_strcasecmp(system_charset_info, SLOW_LOG_NAME.str, name.str) == 0))
      return TABLE_CATEGORY_LOG;

    if ((name.length == RLI_INFO_NAME.length) &&
        (my_strcasecmp(system_charset_info, RLI_INFO_NAME.str, name.str) == 0))
      return TABLE_CATEGORY_RPL_INFO;

    if ((name.length == MI_INFO_NAME.length) &&
        (my_strcasecmp(system_charset_info, MI_INFO_NAME.str, name.str) == 0))
      return TABLE_CATEGORY_RPL_INFO;

    if ((name.length == WORKER_INFO_NAME.length) &&
        (my_strcasecmp(system_charset_info, WORKER_INFO_NAME.str, name.str) ==
         0))
      return TABLE_CATEGORY_RPL_INFO;

    if ((name.length == GTID_EXECUTED_NAME.length) &&
        (my_strcasecmp(system_charset_info, GTID_EXECUTED_NAME.str, name.str) ==
         0))
      return TABLE_CATEGORY_GTID;

    if (dd::get_dictionary()->is_dd_table_name(MYSQL_SCHEMA_NAME.str, name.str))
      return TABLE_CATEGORY_DICTIONARY;
  }

  return TABLE_CATEGORY_USER;
}

/**
  Allocate and setup a TABLE_SHARE structure

  @param db          schema name.
  @param table_name  table name.
  @param key         table cache key (db \0 table_name \0...)
  @param key_length  length of the key
  @param open_secondary  true if the TABLE_SHARE represents a table
                         in a secondary storage engine

  @return            pointer to allocated table share
    @retval NULL     error (out of memory, too long path name)
*/

TABLE_SHARE *alloc_table_share(const char *db, const char *table_name,
                               const char *key, size_t key_length,
                               bool open_secondary) {
  MEM_ROOT mem_root;
  TABLE_SHARE *share = nullptr;
  char *key_buff, *path_buff;
  char path[FN_REFLEN + 1];
  size_t path_length;
  Table_cache_element **cache_element_array;
  bool was_truncated = false;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s'.'%s'", db, table_name));

  /*
    There are FN_REFLEN - reg_ext_length bytes available for the
    file path and the trailing '\0', which may be padded to the right
    of the length indicated by the length parameter. The returned
    path length does not include the trailing '\0'.
  */
  path_length = build_table_filename(path, sizeof(path) - 1 - reg_ext_length,
                                     db, table_name, "", 0, &was_truncated);

  /*
    The path now misses extension, but includes '\0'. Unless it was
    truncated, everything should be ok.
  */
  if (was_truncated) {
    my_error(ER_IDENT_CAUSES_TOO_LONG_PATH, MYF(0), sizeof(path) - 1, path);
    return nullptr;
  }

  init_sql_alloc(key_memory_table_share, &mem_root, TABLE_ALLOC_BLOCK_SIZE, 0);
  if (multi_alloc_root(&mem_root, &share, sizeof(*share), &key_buff, key_length,
                       &path_buff, path_length + 1, &cache_element_array,
                       table_cache_instances * sizeof(*cache_element_array),
                       NULL)) {
    new (share) TABLE_SHARE(refresh_version, open_secondary);

    share->set_table_cache_key(key_buff, key, key_length);

    share->path.str = path_buff;
    share->path.length = path_length;
    my_stpcpy(share->path.str, path);
    share->normalized_path.str = share->path.str;
    share->normalized_path.length = path_length;

    /*
      Since alloc_table_share() can be called without any locking (for
      example, ha_create_table... functions), we do not assign a table
      map id here.  Instead we assign a value that is not used
      elsewhere, and then assign a table map id inside open_table()
      under the protection of the LOCK_open mutex.
    */
    share->table_map_id = ~0ULL;
    share->cached_row_logging_check = -1;

    share->m_flush_tickets.empty();

    memset(cache_element_array, 0,
           table_cache_instances * sizeof(*cache_element_array));
    share->cache_element = cache_element_array;

    share->mem_root = std::move(mem_root);
    mysql_mutex_init(key_TABLE_SHARE_LOCK_ha_data, &share->LOCK_ha_data,
                     MY_MUTEX_INIT_FAST);
  }
  return share;
}

/**
  Initialize share for temporary tables

  @param thd         thread handle
  @param share	Share to fill
  @param key		Table_cache_key, as generated from create_table_def_key.
                must start with db name.
  @param key_length	Length of key
  @param table_name	Table name
  @param path	Path to file (possible in lower case) without .frm
  @param mem_root       MEM_ROOT to transfer (move) to the TABLE_SHARE; if
  NULL a new one is initialized.

  @note
    This is different from alloc_table_share() because temporary tables
    don't have to be shared between threads or put into the table def
    cache, so we can do some things notable simpler and faster

    If table is not put in thd->temporary_tables (happens only when
    one uses OPEN TEMPORARY) then one can specify 'db' as key and
    use key_length= 0 as neither table_cache_key or key_length will be used).
*/

void init_tmp_table_share(THD *thd, TABLE_SHARE *share, const char *key,
                          size_t key_length, const char *table_name,
                          const char *path, MEM_ROOT *mem_root) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s'.'%s'", key, table_name));

  new (share) TABLE_SHARE();

  if (mem_root)
    share->mem_root = std::move(*mem_root);
  else
    init_sql_alloc(key_memory_table_share, &share->mem_root,
                   TABLE_ALLOC_BLOCK_SIZE, 0);

  share->table_category = TABLE_CATEGORY_TEMPORARY;
  share->tmp_table = INTERNAL_TMP_TABLE;
  share->db.str = key;
  share->db.length = strlen(key);
  share->table_cache_key.str = key;
  share->table_cache_key.length = key_length;
  share->table_name.str = table_name;
  share->table_name.length = strlen(table_name);
  share->path.str = const_cast<char *>(path);
  share->normalized_path.str = path;
  share->path.length = share->normalized_path.length = strlen(path);

  share->cached_row_logging_check = -1;

  /*
    table_map_id is also used for MERGE tables to suppress repeated
    compatibility checks.
  */
  share->table_map_id = (ulonglong)thd->query_id;

  share->m_flush_tickets.empty();
}

Key_map TABLE_SHARE::usable_indexes(const THD *thd) const {
  Key_map usable_indexes(keys_in_use);
  if (!thd->optimizer_switch_flag(OPTIMIZER_SWITCH_USE_INVISIBLE_INDEXES))
    usable_indexes.intersect(visible_indexes);
  return usable_indexes;
}

#ifndef DBUG_OFF
/**
  Assert that the #LOCK_open mutex is held when the reference count of
  a TABLE_SHARE is accessed.

  @param share the TABLE_SHARE
  @return true if the assertion holds, terminates the process otherwise
*/
bool assert_ref_count_is_locked(const TABLE_SHARE *share) {
  // The mutex is not needed while the TABLE_SHARE is being
  // constructed, or if it is for a temporary table.
  if (share->table_category != TABLE_UNKNOWN_CATEGORY &&
      share->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_assert_owner(&LOCK_open);
  }
  return true;
}
#endif

void TABLE_SHARE::clear_version() {
  table_cache_manager.assert_owner_all_and_tdc();
  m_version = 0;
}

/**
  Release resources (plugins) used by the share and free its memory.
  TABLE_SHARE is self-contained -- it's stored in its own MEM_ROOT.
  Free this MEM_ROOT.
*/

void TABLE_SHARE::destroy() {
  uint idx;
  KEY *info_it;

  DBUG_TRACE;
  DBUG_PRINT("info", ("db: %s table: %s", db.str, table_name.str));
  if (ha_share) {
    delete ha_share;
    ha_share = nullptr;
  }
  if (m_part_info) {
    ::destroy(m_part_info);
    m_part_info = nullptr;
  }
  /* The mutex is initialized only for shares that are part of the TDC */
  if (tmp_table == NO_TMP_TABLE) mysql_mutex_destroy(&LOCK_ha_data);
  delete name_hash;
  name_hash = nullptr;

  delete m_histograms;
  m_histograms = nullptr;

  plugin_unlock(nullptr, db_plugin);
  db_plugin = nullptr;

  /* Release fulltext parsers */
  info_it = key_info;
  for (idx = keys; idx; idx--, info_it++) {
    if (info_it->flags & HA_USES_PARSER) {
      plugin_unlock(nullptr, info_it->parser);
      info_it->flags = 0;
    }
  }

  /* Destroy dd::Table object associated with temporary table's share. */
  delete tmp_table_def;
  tmp_table_def = nullptr;

  /* Delete the view object. */
  delete view_object;
  view_object = nullptr;

#ifdef HAVE_PSI_TABLE_INTERFACE
  PSI_TABLE_CALL(release_table_share)(m_psi);
#endif

  /*
    Make a copy since the share is allocated in its own root,
    and free_root() updates its argument after freeing the memory.
  */
  MEM_ROOT own_root = std::move(mem_root);
  free_root(&own_root, MYF(0));
}

/**
  Free table share and memory used by it

  @param share		Table share
*/

void free_table_share(TABLE_SHARE *share) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: %s.%s", share->db.str, share->table_name.str));
  DBUG_ASSERT(share->ref_count() == 0);

  if (share->m_flush_tickets.is_empty()) {
    /*
      No threads are waiting for this share to be flushed (the
      share is not old, is for a temporary table, or just nobody
      happens to be waiting for it). Destroy it.
    */
    share->destroy();
  } else {
    Wait_for_flush_list::Iterator it(share->m_flush_tickets);
    Wait_for_flush *ticket;
    /*
      We're about to iterate over a list that is used
      concurrently. Make sure this never happens without a lock.
    */
    mysql_mutex_assert_owner(&LOCK_open);

    while ((ticket = it++))
      (void)ticket->get_ctx()->m_wait.set_status(MDL_wait::GRANTED);
    /*
      If there are threads waiting for this share to be flushed,
      the last one to receive the notification will destroy the
      share. At this point the share is removed from the table
      definition cache, so is OK to proceed here without waiting
      for this thread to do the work.
    */
  }
}

/**
  Return true if a table name matches one of the system table names.
  Currently these are:

  help_category, help_keyword, help_relation, help_topic,
  proc, event
  time_zone, time_zone_leap_second, time_zone_name, time_zone_transition,
  time_zone_transition_type

  This function trades accuracy for speed, so may return false
  positives. Presumably mysql.* database is for internal purposes only
  and should not contain user tables.
*/

inline bool is_system_table_name(const char *name, size_t length) {
  CHARSET_INFO *ci = system_charset_info;

  return (
      /* mysql.proc table */
      (length == 4 && my_tolower(ci, name[0]) == 'p' &&
       my_tolower(ci, name[1]) == 'r' && my_tolower(ci, name[2]) == 'o' &&
       my_tolower(ci, name[3]) == 'c') ||

      (length > 4 &&
       (
           /* one of mysql.help* tables */
           (my_tolower(ci, name[0]) == 'h' && my_tolower(ci, name[1]) == 'e' &&
            my_tolower(ci, name[2]) == 'l' && my_tolower(ci, name[3]) == 'p') ||

           /* one of mysql.time_zone* tables */
           (my_tolower(ci, name[0]) == 't' && my_tolower(ci, name[1]) == 'i' &&
            my_tolower(ci, name[2]) == 'm' && my_tolower(ci, name[3]) == 'e') ||

           /* mysql.event table */
           (my_tolower(ci, name[0]) == 'e' && my_tolower(ci, name[1]) == 'v' &&
            my_tolower(ci, name[2]) == 'e' && my_tolower(ci, name[3]) == 'n' &&
            my_tolower(ci, name[4]) == 't'))));
}

/**
  Initialize key_part_flag from source field.
*/

void KEY_PART_INFO::init_flags() {
  DBUG_ASSERT(field);
  if (field->type() == MYSQL_TYPE_BLOB || field->type() == MYSQL_TYPE_GEOMETRY)
    key_part_flag |= HA_BLOB_PART;
  else if (field->real_type() == MYSQL_TYPE_VARCHAR)
    key_part_flag |= HA_VAR_LENGTH_PART;
  else if (field->type() == MYSQL_TYPE_BIT)
    key_part_flag |= HA_BIT_PART;
}

/**
  Initialize KEY_PART_INFO from the given field.

  @param fld The field to initialize keypart from
*/

void KEY_PART_INFO::init_from_field(Field *fld) {
  field = fld;
  fieldnr = field->field_index + 1;
  null_bit = field->null_bit;
  null_offset = field->null_offset();
  offset = field->offset(field->table->record[0]);
  length = (uint16)field->key_length();
  store_length = length;
  key_part_flag = 0;

  if (field->is_nullable()) store_length += HA_KEY_NULL_LENGTH;
  if (field->type() == MYSQL_TYPE_BLOB ||
      field->real_type() == MYSQL_TYPE_VARCHAR ||
      field->type() == MYSQL_TYPE_GEOMETRY) {
    store_length += HA_KEY_BLOB_LENGTH;
  }
  init_flags();

  ha_base_keytype key_type = field->key_type();
  type = (uint8)key_type;
  bin_cmp = key_type != HA_KEYTYPE_TEXT && key_type != HA_KEYTYPE_VARTEXT1 &&
            key_type != HA_KEYTYPE_VARTEXT2;
}

/**
  Setup key-related fields of Field object for given key and key part.

  @param[in]     share                    Pointer to TABLE_SHARE
  @param[in]     handler_file             Pointer to handler
  @param[in]     primary_key_n            Primary key number
  @param[in]     keyinfo                  Pointer to processed key
  @param[in]     key_n                    Processed key number
  @param[in]     key_part_n               Processed key part number
  @param[in,out] usable_parts             Pointer to usable_parts variable
  @param[in]     part_of_key_not_extended Set when column is part of the Key
                                          and not appended by the storage
                                          engine from primary key columns.
*/

void setup_key_part_field(TABLE_SHARE *share, handler *handler_file,
                          uint primary_key_n, KEY *keyinfo, uint key_n,
                          uint key_part_n, uint *usable_parts,
                          bool part_of_key_not_extended) {
  KEY_PART_INFO *key_part = &keyinfo->key_part[key_part_n];
  Field *field = key_part->field;

  /* Flag field as unique if it is the only keypart in a unique index */
  if (key_part_n == 0 && key_n != primary_key_n)
    field->flags |= (((keyinfo->flags & HA_NOSAME) &&
                      (keyinfo->user_defined_key_parts == 1))
                         ? UNIQUE_KEY_FLAG
                         : MULTIPLE_KEY_FLAG);
  if (key_part_n == 0) field->key_start.set_bit(key_n);
  field->m_indexed = true;

  const bool full_length_key_part =
      (field->key_length() == key_part->length && !(field->flags & BLOB_FLAG));
  /*
    part_of_key contains all non-prefix keys, part_of_prefixkey
    contains prefix keys.
    Note that prefix keys in the extended PK key parts
    (part_of_key_not_extended is false) are not considered.
  */
  if (full_length_key_part) {
    field->part_of_key.set_bit(key_n);
    if (part_of_key_not_extended)
      field->part_of_key_not_extended.set_bit(key_n);
  } else if (part_of_key_not_extended) {
    field->part_of_prefixkey.set_bit(key_n);
  }
  if ((handler_file->index_flags(key_n, key_part_n, false) & HA_KEYREAD_ONLY) &&
      field->type() != MYSQL_TYPE_GEOMETRY) {
    // Set the key as 'keys_for_keyread' even if it is prefix key.
    share->keys_for_keyread.set_bit(key_n);
  }

  if (full_length_key_part &&
      (handler_file->index_flags(key_n, key_part_n, true) & HA_READ_ORDER))
    field->part_of_sortkey.set_bit(key_n);

  if (!(key_part->key_part_flag & HA_REVERSE_SORT) &&
      *usable_parts == key_part_n)
    (*usable_parts)++;  // For FILESORT
}

/**
  Generate extended secondary keys by adding primary key parts to the
  existing secondary key. A primary key part is added if such part doesn't
  present in the secondary key or the part in the secondary key is a
  prefix of the key field. Key parts are added till:
  .) all parts were added
  .) number of key parts became bigger that MAX_REF_PARTS
  .) total key length became longer than MAX_REF_LENGTH
  depending on what occurs first first.
  Unlike existing secondary key parts which are initialized at
  open_binary_frm(), newly added ones are initialized here by copying
  KEY_PART_INFO structure from primary key part and calling
  setup_key_part_field().

  Function updates sk->actual/unused_key_parts and sk->actual_flags.

  @param[in]     sk            Secondary key
  @param[in]     sk_n          Secondary key number
  @param[in]     pk            Primary key
  @param[in]     pk_n          Primary key number
  @param[in]     share         Pointer to TABLE_SHARE
  @param[in]     handler_file  Pointer to handler
  @param[in,out] usable_parts  Pointer to usable_parts variable

  @retval                      Number of added key parts
*/

uint add_pk_parts_to_sk(KEY *sk, uint sk_n, KEY *pk, uint pk_n,
                        TABLE_SHARE *share, handler *handler_file,
                        uint *usable_parts) {
  uint max_key_length = sk->key_length;
  bool is_unique_key = false;
  KEY_PART_INFO *current_key_part = &sk->key_part[sk->user_defined_key_parts];

  /*
     For each keypart in the primary key: check if the keypart is
     already part of the secondary key and add it if not.
  */
  for (uint pk_part = 0; pk_part < pk->user_defined_key_parts; pk_part++) {
    KEY_PART_INFO *pk_key_part = &pk->key_part[pk_part];
    /* MySQL does not supports more key parts than MAX_REF_LENGTH */
    if (sk->actual_key_parts >= MAX_REF_PARTS) goto end;

    bool pk_field_is_in_sk = false;
    for (uint j = 0; j < sk->user_defined_key_parts; j++) {
      if (sk->key_part[j].fieldnr == pk_key_part->fieldnr &&
          share->field[pk_key_part->fieldnr - 1]->key_length() ==
              sk->key_part[j].length) {
        pk_field_is_in_sk = true;
        break;
      }
    }

    /* Add PK field to secondary key if it's not already  part of the key. */
    if (!pk_field_is_in_sk) {
      /* MySQL does not supports keys longer than MAX_KEY_LENGTH */
      if (max_key_length + pk_key_part->length > MAX_KEY_LENGTH) goto end;

      *current_key_part = *pk_key_part;
      setup_key_part_field(share, handler_file, pk_n, sk, sk_n,
                           sk->actual_key_parts, usable_parts, false);
      sk->actual_key_parts++;
      sk->unused_key_parts--;
      sk->rec_per_key[sk->actual_key_parts - 1] = 0;
      sk->set_records_per_key(sk->actual_key_parts - 1, REC_PER_KEY_UNKNOWN);
      current_key_part++;
      max_key_length += pk_key_part->length;
      /*
        Secondary key will be unique if the key  does not exceed
        key length limitation and key parts limitation.
      */
      is_unique_key = true;
    }
  }
  if (is_unique_key) sk->actual_flags |= HA_NOSAME;

end:
  return (sk->actual_key_parts - sk->user_defined_key_parts);
}

//////////////////////////////////////////////////////////////////////////

/*
  The following section adds code for the interface with the .frm file.
  These defines and functions comes from the file sql/field.h in 5.7

  Note:
  These functions should not be used any where else in the code.
  They are only used in upgrade scenario for migrating old data directory
  to be compatible with current server. They will be removed in future
  release.

  Any new code should not be added in this section.
*/

#define FIELDFLAG_DECIMAL 1
#define FIELDFLAG_BINARY 1  // Shares same flag
#define FIELDFLAG_NUMBER 2
#define FIELDFLAG_ZEROFILL 4
#define FIELDFLAG_PACK 120      // Bits used for packing
#define FIELDFLAG_INTERVAL 256  // mangled with decimals!
#define FIELDFLAG_BITFIELD 512  // mangled with decimals!
#define FIELDFLAG_BLOB 1024     // mangled with decimals!
#define FIELDFLAG_GEOM 2048     // mangled with decimals!
#define FIELDFLAG_JSON              \
  4096 /* mangled with decimals and \
          with bitfields! */

#define FIELDFLAG_TREAT_BIT_AS_CHAR 4096 /* use Field_bit_as_char */

#define FIELDFLAG_LEFT_FULLSCREEN 8192
#define FIELDFLAG_RIGHT_FULLSCREEN 16384
#define FIELDFLAG_FORMAT_NUMBER 16384       // predit: ###,,## in output
#define FIELDFLAG_NO_DEFAULT 16384          /* sql */
#define FIELDFLAG_SUM ((uint)32768)         // predit: +#fieldflag
#define FIELDFLAG_MAYBE_NULL ((uint)32768)  // sql
#define FIELDFLAG_PACK_SHIFT 3
#define FIELDFLAG_DEC_SHIFT 8
#define FIELDFLAG_MAX_DEC 31
#define FIELDFLAG_NUM_SCREEN_TYPE 0x7F01
#define FIELDFLAG_ALFA_SCREEN_TYPE 0x7800

#define MTYP_TYPENR(type) (type & 127) /* Remove bits from type */

#define FIELD_NR_MASK 16383 /* To get fieldnumber */

inline int f_is_dec(int x) { return (x & FIELDFLAG_DECIMAL); }
inline int f_is_num(int x) { return (x & FIELDFLAG_NUMBER); }
inline int f_is_zerofill(int x) { return (x & FIELDFLAG_ZEROFILL); }
inline int f_is_packed(int x) { return (x & FIELDFLAG_PACK); }
inline int f_packtype(int x) { return ((x >> FIELDFLAG_PACK_SHIFT) & 15); }
inline uint8 f_decimals(int x) {
  return ((uint8)((x >> FIELDFLAG_DEC_SHIFT) & FIELDFLAG_MAX_DEC));
}
inline int f_is_alpha(int x) { return (!f_is_num(x)); }
inline int f_is_binary(int x) {
  return (x & FIELDFLAG_BINARY);  // 4.0- compatibility
}
inline int f_is_enum(int x) {
  return ((x & (FIELDFLAG_INTERVAL | FIELDFLAG_NUMBER)) == FIELDFLAG_INTERVAL);
}
inline int f_is_bitfield(int x) {
  return ((x & (FIELDFLAG_BITFIELD | FIELDFLAG_NUMBER)) == FIELDFLAG_BITFIELD);
}

inline int f_is_blob(int x) {
  return ((x & (FIELDFLAG_BLOB | FIELDFLAG_NUMBER)) == FIELDFLAG_BLOB);
}
inline int f_is_geom(int x) {
  return ((x & (FIELDFLAG_GEOM | FIELDFLAG_NUMBER)) == FIELDFLAG_GEOM);
}
inline int f_is_json(int x) {
  return ((x & (FIELDFLAG_JSON | FIELDFLAG_NUMBER | FIELDFLAG_BITFIELD)) ==
          FIELDFLAG_JSON);
}
inline int f_is_equ(int x) { return (x & (1 + 2 + FIELDFLAG_PACK + 31 * 256)); }
inline int f_settype(int x) { return (x << FIELDFLAG_PACK_SHIFT); }
inline int f_maybe_null(int x) { return (x & FIELDFLAG_MAYBE_NULL); }
inline int f_no_default(int x) { return (x & FIELDFLAG_NO_DEFAULT); }
inline int f_bit_as_char(int x) { return (x & FIELDFLAG_TREAT_BIT_AS_CHAR); }

/**
  Read string from a file with malloc

  @note We add an \0 at end of the read string to make reading of C strings
  easier. This function is added to read .frm file in upgrade scenario. It
  should not be used any where else in the code. This function will be removed
  later.

  @param[in]  file        file handler
  @param[out] to          pointer to read string
  @param[in]  length      length of string

  @retval 0  Error
  @retval 1  Success
*/

static int read_string(File file, uchar **to, size_t length) {
  DBUG_TRACE;

  my_free(*to);
  if (!(*to = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, length + 1,
                                 MYF(MY_WME))) ||
      mysql_file_read(file, *to, length, MYF(MY_NABP))) {
    my_free(*to);  /* purecov: inspected */
    *to = nullptr; /* purecov: inspected */
    return 1;      /* purecov: inspected */
  }
  *((char *)*to + length) = '\0';
  return 0;
} /* read_string */

/**
  Un-hex all elements in a typelib.

  @param[in] interval  TYPELIB (struct of pointer to values + lengths + count)

  @note This function is added to read .frm file in upgrade scenario. It should
  not be used any where else in the code. This function will be removed later.
*/

static void unhex_type2(TYPELIB *interval) {
  for (uint pos = 0; pos < interval->count; pos++) {
    char *from, *to;
    for (from = to = const_cast<char *>(interval->type_names[pos]); *from;) {
      /*
        Note, hexchar_to_int(*from++) doesn't work
        one some compilers, e.g. IRIX. Looks like a compiler
        bug in inline functions in combination with arguments
        that have a side effect. So, let's use from[0] and from[1]
        and increment 'from' by two later.
      */

      *to++ = (char)(hexchar_to_int(from[0]) << 4) + hexchar_to_int(from[1]);
      from += 2;
    }
    interval->type_lengths[pos] /= 2;
  }
}

/**
 Search after a field with given start & length
 If an exact field isn't found, return longest field with starts
 at right position.

 @note This is needed because in some .frm fields 'fieldnr' was saved wrong.
 This function is added to read .frm file in upgrade scenario. It should not
 be used any where else in the code. This function will be removed later.

 @retval 0               error
 @retval field number +1 success
*/

static uint find_field(Field **fields, uchar *record, uint start, uint length) {
  Field **field;
  uint i, pos;

  pos = 0;
  for (field = fields, i = 1; *field; i++, field++) {
    if ((*field)->offset(record) == start) {
      if ((*field)->key_length() == length) return (i);
      if (!pos || fields[pos - 1]->pack_length() < (*field)->pack_length())
        pos = i;
    }
  }
  return (pos);
}

/**
  fix a str_type to a array type
  typeparts separated with some char. differents types are separated
  with a '\0'

  @note This function is added to read .frm file in upgrade scenario. It
  should not be used any where else in the code. This function will be
  removed later.

  @param[out]  array          Pointer to interval array
  @param[in]   point_to_type  Pointer to intervals
  @param[in]   types          number of intervals
  @param[out]  names          name of intervals

*/

static void fix_type_pointers(const char ***array, TYPELIB *point_to_type,
                              uint types, char **names) {
  char *type_name, *ptr;
  char chr;

  ptr = *names;
  while (types--) {
    point_to_type->name = nullptr;
    point_to_type->type_names = *array;

    if ((chr = *ptr)) /* Test if empty type */
    {
      while ((type_name = strchr(ptr + 1, chr)) != NullS) {
        *((*array)++) = ptr + 1;
        *type_name = '\0'; /* End string */
        ptr = type_name;
      }
      ptr += 2; /* Skip end mark and last 0 */
    } else
      ptr++;
    point_to_type->count = (uint)(*array - point_to_type->type_names);
    point_to_type++;
    *((*array)++) = NullS; /* End of type */
  }
  *names = ptr; /* Update end */
  return;
} /* fix_type_pointers */

/**
  Find where a form starts.

  @note This function is added to read .frm file in upgrade scenario. It should
  not be used any where else in the code. This function will be removed later.

  @param[in] file   File handler
  @param[in] head   The start of the form file.

  @remark If formname is NULL then only formnames is read.

  @retval The form position.
*/

static ulong get_form_pos(File file, uchar *head) {
  uchar *pos, *buf;
  uint names, length;
  ulong ret_value = 0;
  DBUG_TRACE;

  names = uint2korr(head + 8);

  if (!(names = uint2korr(head + 8))) return 0;

  length = uint2korr(head + 4);

  mysql_file_seek(file, 64L, MY_SEEK_SET, MYF(0));

  if (!(buf = (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, length + names * 4,
                                 MYF(MY_WME))))
    return 0;

  if (mysql_file_read(file, buf, length + names * 4, MYF(MY_NABP))) {
    my_free(buf);
    return 0;
  }

  pos = buf + length;
  ret_value = uint4korr(pos);

  my_free(buf);

  return ret_value;
}

#define STORAGE_TYPE_MASK 7
#define COLUMN_FORMAT_MASK 7
#define COLUMN_FORMAT_SHIFT 3

/**
  Auxiliary function which creates Field object from in-memory
  representation of .FRM file.

  NOTES:
  This function is added to read .frm file in upgrade scenario. It should not
  be used any where else in the code. This function will be removed later.

  @param         thd                   Connection context.
  @param         share                 TABLE_SHARE for which Field object
                                       needs to be constructed.
  @param         frm_context           FRM_context for the structures removed
                                       from TABLE_SHARE.
  @param         new_frm_ver           .FRM file version.
  @param         use_hash              Indicates whether we use hash or linear
                                       search to lookup fields by name.
  @param         field_idx             Field index in TABLE_SHARE::field array.
  @param         strpos                Pointer to part of .FRM's screens
                                       section describing the field to be
                                       created.
  @param         format_section_fields Array where each byte contains packed
                                       values of COLUMN_FORMAT/STORAGE options
                                       for corresponding column.
  @param[in,out] comment_pos           Pointer to part of column comments
                                       section of .FRM which corresponds
                                       to current field. Advanced to the
                                       position corresponding to comment
                                       for the next column.
  @param[in,out] gcol_screen_pos       Pointer to part of generated columns
                                       section of .FRM which corresponds
                                       to current generated field. If field
                                       to be created is generated advanced
                                       to the position for the next column
  @param[in,out] null_pos              Current byte in the record preamble
                                       to be used for field's null/leftover
                                       bits if necessary.
  @param[in,out] null_bit_pos          Current bit in the current preamble
                                       byte to be used for field's null/
                                       leftover bits if necessary.
  @param[out]    errarg                Additional argument for the error to
                                       be reported.

  @retval 0      Success.
  @retval non-0  Error number (@sa open_table_def() for details).
*/

static int make_field_from_frm(THD *thd, TABLE_SHARE *share,
                               FRM_context *frm_context, uint new_frm_ver,
                               bool use_hash, uint field_idx, uchar *strpos,
                               uchar *format_section_fields, char **comment_pos,
                               char **gcol_screen_pos, uchar **null_pos,
                               uint *null_bit_pos, int *errarg) {
  uint pack_flag, interval_nr, unireg_type, recpos, field_length;
  uint gcol_info_length = 0;
  enum_field_types field_type;
  const CHARSET_INFO *charset = nullptr;
  Field::geometry_type geom_type = Field::GEOM_GEOMETRY;
  LEX_CSTRING comment;
  Value_generator *gcol_info = nullptr;
  bool fld_stored_in_db = true;
  Field *reg_field;

  if (new_frm_ver >= 3) {
    /* new frm file in 4.1 */
    field_length = uint2korr(strpos + 3);
    recpos = uint3korr(strpos + 5);
    pack_flag = uint2korr(strpos + 8);
    unireg_type = (uint)strpos[10];
    interval_nr = (uint)strpos[12];
    uint comment_length = uint2korr(strpos + 15);
    field_type = (enum_field_types)(uint)strpos[13];

    /* charset and geometry_type share the same byte in frm */
    if (field_type == MYSQL_TYPE_GEOMETRY) {
      geom_type = (Field::geometry_type)strpos[14];
      charset = &my_charset_bin;
    } else {
      uint csid = strpos[14] + (((uint)strpos[11]) << 8);
      if (!csid)
        charset = &my_charset_bin;
      else if (!(charset = get_charset(csid, MYF(0)))) {
        // Unknown or unavailable charset
        *errarg = (int)csid;
        return 5;
      }
    }

    if (!comment_length) {
      comment.str = "";
      comment.length = 0;
    } else {
      comment.str = *comment_pos;
      comment.length = comment_length;
      (*comment_pos) += comment_length;
    }

    if (unireg_type & FRM_context::GENERATED_FIELD) {
      /*
        Get generated column data stored in the .frm file as follows:
        byte 1      = 1 (always 1 to allow for future extensions)
        byte 2,3    = expression length
        byte 4      = flags, as of now:
                        0 - no flags
                        1 - field is physically stored
        byte 5-...  = generated column expression (text data)
      */
      gcol_info = new (thd->mem_root) Value_generator();
      if ((uint)(*gcol_screen_pos)[0] != 1) return 4;

      gcol_info_length = uint2korr(*gcol_screen_pos + 1);
      DBUG_ASSERT(gcol_info_length);  // Expect non-null expression

      fld_stored_in_db = (bool)(uint)(*gcol_screen_pos)[3];
      gcol_info->set_field_stored(fld_stored_in_db);
      gcol_info->dup_expr_str(&share->mem_root,
                              *gcol_screen_pos + (uint)FRM_GCOL_HEADER_SIZE,
                              gcol_info_length);
      (*gcol_screen_pos) += gcol_info_length + FRM_GCOL_HEADER_SIZE;
      share->vfields++;
    }
  } else {
    field_length = (uint)strpos[3];
    recpos = uint2korr(strpos + 4), pack_flag = uint2korr(strpos + 6);
    pack_flag &= ~FIELDFLAG_NO_DEFAULT;  // Safety for old files
    unireg_type = (uint)strpos[8];
    interval_nr = (uint)strpos[10];

    /* old frm file */
    field_type = (enum_field_types)f_packtype(pack_flag);
    if (f_is_binary(pack_flag)) {
      /*
        Try to choose the best 4.1 type:
        - for 4.0 "CHAR(N) BINARY" or "VARCHAR(N) BINARY"
          try to find a binary collation for character set.
        - for other types (e.g. BLOB) just use my_charset_bin.
      */
      if (!f_is_blob(pack_flag)) {
        // 3.23 or 4.0 string
        if (!(charset = get_charset_by_csname(share->table_charset->csname,
                                              MY_CS_BINSORT, MYF(0))))
          charset = &my_charset_bin;
      } else
        charset = &my_charset_bin;
    } else
      charset = share->table_charset;
    memset(&comment, 0, sizeof(comment));
  }

  if (interval_nr && charset->mbminlen > 1) {
    /* Unescape UCS2 intervals from HEX notation */
    TYPELIB *interval = share->intervals + interval_nr - 1;
    unhex_type2(interval);
  }

  if (field_type == MYSQL_TYPE_NEWDECIMAL && !share->mysql_version) {
    /*
      Fix pack length of old decimal values from 5.0.3 -> 5.0.4
      The difference is that in the old version we stored precision
      in the .frm table while we now store the display_length
    */
    uint decimals = f_decimals(pack_flag);
    field_length = my_decimal_precision_to_length(field_length, decimals,
                                                  f_is_dec(pack_flag) == 0);
    LogErr(ERROR_LEVEL, ER_TABLE_INCOMPATIBLE_DECIMAL_FIELD,
           frm_context->fieldnames.type_names[field_idx], share->table_name.str,
           share->table_name.str);
    push_warning_printf(thd, Sql_condition::SL_WARNING, ER_CRASHED_ON_USAGE,
                        ER_THD(thd, ER_TABLE_INCOMPATIBLE_DECIMAL_FIELD),
                        frm_context->fieldnames.type_names[field_idx],
                        share->table_name.str, share->table_name.str);
    share->crashed = true;  // Marker for CHECK TABLE
  }

  if (field_type == MYSQL_TYPE_YEAR && field_length != 4) {
    LogErr(ERROR_LEVEL, ER_TABLE_INCOMPATIBLE_YEAR_FIELD,
           frm_context->fieldnames.type_names[field_idx], share->table_name.str,
           share->table_name.str);
    push_warning_printf(thd, Sql_condition::SL_WARNING, ER_CRASHED_ON_USAGE,
                        ER_THD(thd, ER_TABLE_INCOMPATIBLE_YEAR_FIELD),
                        frm_context->fieldnames.type_names[field_idx],
                        share->table_name.str, share->table_name.str);
    share->crashed = true;
  }

  FRM_context::utype unireg = (FRM_context::utype)MTYP_TYPENR(unireg_type);
  // Construct auto_flag
  uchar auto_flags = Field::NONE;

  if (unireg == FRM_context::TIMESTAMP_DN_FIELD ||
      unireg == FRM_context::TIMESTAMP_DNUN_FIELD)
    auto_flags |= Field::DEFAULT_NOW;
  if (unireg == FRM_context::TIMESTAMP_UN_FIELD ||
      unireg == FRM_context::TIMESTAMP_DNUN_FIELD)
    auto_flags |= Field::ON_UPDATE_NOW;

  if (unireg == FRM_context::NEXT_NUMBER) auto_flags |= Field::NEXT_NUMBER;

  share->field[field_idx] = reg_field = make_field(
      thd->mem_root, share,
      share->default_values - 1 + recpos,  // recpos starts from 1.
      (uint32)field_length, *null_pos, *null_bit_pos, field_type, charset,
      geom_type, auto_flags,
      (interval_nr ? share->intervals + interval_nr - 1 : (TYPELIB *)nullptr),
      frm_context->fieldnames.type_names[field_idx], f_maybe_null(pack_flag),
      f_is_zerofill(pack_flag) != 0, f_is_dec(pack_flag) == 0,
      f_decimals(pack_flag), f_bit_as_char(pack_flag), 0, {},
      // Array fields aren't supported in .frm-based tables
      false);
  if (!reg_field) {
    // Not supported field type
    return 4;
  }

  reg_field->field_index = field_idx;
  reg_field->comment = comment;
  reg_field->gcol_info = gcol_info;
  reg_field->stored_in_db = fld_stored_in_db;
  if (field_type == MYSQL_TYPE_BIT && !f_bit_as_char(pack_flag)) {
    if (((*null_bit_pos) += field_length & 7) > 7) {
      (*null_pos)++;
      (*null_bit_pos) -= 8;
    }
  }
  if (!(reg_field->flags & NOT_NULL_FLAG)) {
    if (!(*null_bit_pos = (*null_bit_pos + 1) & 7)) (*null_pos)++;
  }
  if (f_no_default(pack_flag)) reg_field->flags |= NO_DEFAULT_VALUE_FLAG;

  if (unireg == FRM_context::NEXT_NUMBER)
    share->found_next_number_field = share->field + field_idx;

  if (use_hash) {
    Field **field = share->field + field_idx;
    share->name_hash->emplace((*field)->field_name, field);
  }

  if (format_section_fields) {
    const uchar field_flags = format_section_fields[field_idx];
    const uchar field_storage = (field_flags & STORAGE_TYPE_MASK);
    const uchar field_column_format =
        ((field_flags >> COLUMN_FORMAT_SHIFT) & COLUMN_FORMAT_MASK);
    DBUG_PRINT("debug", ("field flags: %u, storage: %u, column_format: %u",
                         field_flags, field_storage, field_column_format));
    reg_field->set_storage_type((ha_storage_media)field_storage);
    reg_field->set_column_format((column_format_type)field_column_format);
  }

  if (!reg_field->stored_in_db) {
    frm_context->stored_fields--;
    if (share->stored_rec_length >= recpos)
      share->stored_rec_length = recpos - 1;
  }

  return 0;
}

static const longlong FRM_VER = 6;
static const longlong FRM_VER_TRUE_VARCHAR = (FRM_VER + 4); /* 10 */

/**
  Read data from a binary .frm file from MySQL 3.23 - 5.0 into TABLE_SHARE

  @note Much of the logic here is duplicated in create_tmp_table()
  (see sql_select.cc). Hence, changes to this function may have to be
  repeated there.

  This function is added to read .frm file in upgrade scenario. It should not
  be used any where else in the code. This function will be removed later.

  @param  thd         thread handle
  @param  share       TABLE_SHARE to be populated.
  @param  frm_context structures removed from TABLE_SHARE
  @param  head        frm file header
  @param  file        File handle
*/

static int open_binary_frm(THD *thd, TABLE_SHARE *share,
                           FRM_context *frm_context, uchar *head, File file) {
  int error, errarg = 0;
  uint new_frm_ver, field_pack_length, new_field_pack_flag;
  uint interval_count, interval_parts, read_length, int_length;
  uint db_create_options, keys, key_parts, n_length;
  uint key_info_length, com_length, null_bit_pos, gcol_screen_length;
  uint extra_rec_buf_length;
  uint i, j;
  bool use_extended_sk;  // Supported extending of secondary keys with PK parts
  bool use_hash;
  char *keynames, *names, *comment_pos, *gcol_screen_pos;
  char *orig_comment_pos, *orig_gcol_screen_pos;
  uchar forminfo[288];
  uchar *record;
  uchar *disk_buff, *strpos, *null_flags, *null_pos;
  ulong pos, record_offset, *rec_per_key, rec_buff_length;
  rec_per_key_t *rec_per_key_float;
  handler *handler_file = nullptr;
  KEY *keyinfo;
  KEY_PART_INFO *key_part;
  Field **field_ptr;
  const char **interval_array;
  enum legacy_db_type legacy_db_type;
  my_bitmap_map *bitmaps;
  uchar *extra_segment_buff = nullptr;
  const uint format_section_header_size = 8;
  uchar *format_section_fields = nullptr;
  bool has_vgc = false;
  DBUG_TRACE;

  new_field_pack_flag = head[27];
  new_frm_ver = (head[2] - FRM_VER);
  field_pack_length = new_frm_ver < 2 ? 11 : 17;
  disk_buff = nullptr;

  error = 3;
  /* Position of the form in the form file. */
  if (!(pos = get_form_pos(file, head))) goto err; /* purecov: inspected */

  mysql_file_seek(file, pos, MY_SEEK_SET, MYF(0));
  if (mysql_file_read(file, forminfo, 288, MYF(MY_NABP))) goto err;
  frm_context->frm_version = head[2];
  /*
    Check if .frm file created by MySQL 5.0. In this case we want to
    display CHAR fields as CHAR and not as VARCHAR.
    We do it this way as we want to keep the old frm version to enable
    MySQL 4.1 to read these files.
  */
  if (frm_context->frm_version == FRM_VER_TRUE_VARCHAR - 1 && head[33] == 5)
    frm_context->frm_version = FRM_VER_TRUE_VARCHAR;

  if (*(head + 61) &&
      !(frm_context->default_part_db_type = ha_checktype(
            thd, (enum legacy_db_type)(uint) * (head + 61), true, false)))
    goto err;
  DBUG_PRINT("info", ("default_part_db_type = %u", head[61]));
  legacy_db_type = (enum legacy_db_type)(uint) * (head + 3);
  DBUG_ASSERT(share->db_plugin == nullptr);
  /*
    if the storage engine is dynamic, no point in resolving it by its
    dynamically allocated legacy_db_type. We will resolve it later by name.
  */
  if (legacy_db_type > DB_TYPE_UNKNOWN &&
      legacy_db_type < DB_TYPE_FIRST_DYNAMIC)
    share->db_plugin = ha_lock_engine(
        nullptr, ha_checktype(thd, legacy_db_type, false, false));
  share->db_create_options = db_create_options = uint2korr(head + 30);
  share->db_options_in_use = share->db_create_options;
  share->mysql_version = uint4korr(head + 51);
  frm_context->null_field_first = false;
  if (!head[32])  // New frm file in 3.23
  {
    share->avg_row_length = uint4korr(head + 34);
    share->row_type = (row_type)head[40];
    share->table_charset =
        get_charset((((uint)head[41]) << 8) + (uint)head[38], MYF(0));
    frm_context->null_field_first = true;
    share->stats_sample_pages = uint2korr(head + 42);
    share->stats_auto_recalc = static_cast<enum_stats_auto_recalc>(head[44]);
  }
  if (!share->table_charset) {
    /* unknown charset in head[38] or pre-3.23 frm */
    if (use_mb(default_charset_info)) {
      /* Warn that we may be changing the size of character columns */
      LogErr(WARNING_LEVEL, ER_INVALID_CHARSET_AND_DEFAULT_IS_MB,
             share->path.str);
    }
    share->table_charset = default_charset_info;
  }
  /* Set temporarily a good value for db_low_byte_first */
  share->db_low_byte_first = (legacy_db_type != DB_TYPE_ISAM);
  error = 4;
  share->max_rows = uint4korr(head + 18);
  share->min_rows = uint4korr(head + 22);

  /* Read keyinformation */
  key_info_length = (uint)uint2korr(head + 28);
  mysql_file_seek(file, (ulong)uint2korr(head + 6), MY_SEEK_SET, MYF(0));
  if (read_string(file, &disk_buff, key_info_length))
    goto err; /* purecov: inspected */
  if (disk_buff[0] & 0x80) {
    share->keys = keys = (disk_buff[1] << 7) | (disk_buff[0] & 0x7f);
    share->key_parts = key_parts = uint2korr(disk_buff + 2);
  } else {
    share->keys = keys = disk_buff[0];
    share->key_parts = key_parts = disk_buff[1];
  }
  share->visible_indexes.init(0);
  share->keys_for_keyread.init(0);
  share->keys_in_use.init(keys);

  strpos = disk_buff + 6;

  use_extended_sk = ha_check_storage_engine_flag(share->db_type(),
                                                 HTON_SUPPORTS_EXTENDED_KEYS);

  uint total_key_parts;
  if (use_extended_sk) {
    uint primary_key_parts =
        keys ? (new_frm_ver >= 3) ? (uint)strpos[4] : (uint)strpos[3] : 0;
    total_key_parts = key_parts + primary_key_parts * (keys - 1);
  } else
    total_key_parts = key_parts;
  n_length = keys * sizeof(KEY) + total_key_parts * sizeof(KEY_PART_INFO);

  /*
    Allocate memory for the KEY object, the key part array, and the
    two rec_per_key arrays.
  */
  if (!multi_alloc_root(&share->mem_root, &keyinfo,
                        n_length + uint2korr(disk_buff + 4), &rec_per_key,
                        sizeof(ulong) * total_key_parts, &rec_per_key_float,
                        sizeof(rec_per_key_t) * total_key_parts, NULL))
    goto err; /* purecov: inspected */

  memset(keyinfo, 0, n_length);
  share->key_info = keyinfo;
  key_part = reinterpret_cast<KEY_PART_INFO *>(keyinfo + keys);

  for (i = 0; i < keys; i++, keyinfo++) {
    keyinfo->table = nullptr;  // Updated in open_frm
    if (new_frm_ver >= 3) {
      keyinfo->flags = (uint)uint2korr(strpos) ^ HA_NOSAME;
      keyinfo->key_length = (uint)uint2korr(strpos + 2);
      keyinfo->user_defined_key_parts = (uint)strpos[4];
      keyinfo->algorithm = (enum ha_key_alg)strpos[5];
      keyinfo->block_size = uint2korr(strpos + 6);
      strpos += 8;
    } else {
      keyinfo->flags = ((uint)strpos[0]) ^ HA_NOSAME;
      keyinfo->key_length = (uint)uint2korr(strpos + 1);
      keyinfo->user_defined_key_parts = (uint)strpos[3];
      // The algorithm was HA_KEY_ALG_UNDEF in 5.7
      keyinfo->algorithm = HA_KEY_ALG_SE_SPECIFIC;
      strpos += 4;
    }

    keyinfo->key_part = key_part;
    keyinfo->set_rec_per_key_array(rec_per_key, rec_per_key_float);
    keyinfo->set_in_memory_estimate(IN_MEMORY_ESTIMATE_UNKNOWN);

    for (j = keyinfo->user_defined_key_parts; j--; key_part++) {
      *rec_per_key++ = 0;
      *rec_per_key_float++ = REC_PER_KEY_UNKNOWN;

      key_part->fieldnr = (uint16)(uint2korr(strpos) & FIELD_NR_MASK);
      key_part->offset = (uint)uint2korr(strpos + 2) - 1;
      // key_part->field=   (Field*) 0; // Will be fixed later
      if (new_frm_ver >= 1) {
        key_part->key_part_flag = *(strpos + 4);
        key_part->length = (uint)uint2korr(strpos + 7);
        strpos += 9;
      } else {
        key_part->length = *(strpos + 4);
        key_part->key_part_flag = 0;
        if (key_part->length > 128) {
          key_part->length &= 127;                   /* purecov: inspected */
          key_part->key_part_flag = HA_REVERSE_SORT; /* purecov: inspected */
        }
        strpos += 7;
      }
      key_part->store_length = key_part->length;
    }
    /*
      Add primary key parts if engine supports primary key extension for
      secondary keys. Here we add unique first key parts to the end of
      secondary key parts array and increase actual number of key parts.
      Note that primary key is always first if exists. Later if there is no
      primary key in the table then number of actual keys parts is set to
      user defined key parts.
    */
    keyinfo->actual_key_parts = keyinfo->user_defined_key_parts;
    keyinfo->actual_flags = keyinfo->flags;
    if (use_extended_sk && i && !(keyinfo->flags & HA_NOSAME)) {
      const uint primary_key_parts = share->key_info->user_defined_key_parts;
      keyinfo->unused_key_parts = primary_key_parts;
      key_part += primary_key_parts;
      rec_per_key += primary_key_parts;
      rec_per_key_float += primary_key_parts;
      share->key_parts += primary_key_parts;
    }
  }
  keynames = (char *)key_part;
  strpos += (my_stpcpy(keynames, (char *)strpos) - keynames) + 1;

  // reading index comments
  for (keyinfo = share->key_info, i = 0; i < keys; i++, keyinfo++) {
    if (keyinfo->flags & HA_USES_COMMENT) {
      keyinfo->comment.length = uint2korr(strpos);
      keyinfo->comment.str = strmake_root(&share->mem_root, (char *)strpos + 2,
                                          keyinfo->comment.length);
      strpos += 2 + keyinfo->comment.length;
    }
    DBUG_ASSERT(((keyinfo->flags & HA_USES_COMMENT) != 0) ==
                (keyinfo->comment.length > 0));
  }

  share->reclength = uint2korr((head + 16));
  share->stored_rec_length = share->reclength;
  if (*(head + 26) == 1) share->system = true; /* one-record-database */

  record_offset = (ulong)(uint2korr(head + 6) + ((uint2korr(head + 14) == 0xffff
                                                      ? uint4korr(head + 47)
                                                      : uint2korr(head + 14))));

  if ((n_length = uint4korr(head + 55))) {
    /* Read extra data segment */
    uchar *next_chunk, *buff_end;
    DBUG_PRINT("info", ("extra segment size is %u bytes", n_length));
    if (!(extra_segment_buff =
              (uchar *)my_malloc(PSI_NOT_INSTRUMENTED, n_length, MYF(MY_WME))))
      goto err;
    next_chunk = extra_segment_buff;
    if (mysql_file_pread(file, extra_segment_buff, n_length,
                         record_offset + share->reclength, MYF(MY_NABP))) {
      goto err;
    }
    share->connect_string.length = uint2korr(next_chunk);
    if (!(share->connect_string.str =
              strmake_root(&share->mem_root, (char *)next_chunk + 2,
                           share->connect_string.length))) {
      goto err;
    }
    next_chunk += share->connect_string.length + 2;
    buff_end = extra_segment_buff + n_length;
    if (next_chunk + 2 < buff_end) {
      uint str_db_type_length = uint2korr(next_chunk);
      LEX_CSTRING name;
      name.str = (char *)next_chunk + 2;
      name.length = str_db_type_length;

      plugin_ref tmp_plugin = ha_resolve_by_name(thd, &name, false);
      if (tmp_plugin != nullptr &&
          !plugin_equals(tmp_plugin, share->db_plugin)) {
        if (legacy_db_type > DB_TYPE_UNKNOWN &&
            legacy_db_type < DB_TYPE_FIRST_DYNAMIC &&
            legacy_db_type !=
                ha_legacy_type(plugin_data<handlerton *>(tmp_plugin))) {
          /* bad file, legacy_db_type did not match the name */
          goto err;
        }
        /*
          tmp_plugin is locked with a local lock.
          we unlock the old value of share->db_plugin before
          replacing it with a globally locked version of tmp_plugin
        */
        plugin_unlock(nullptr, share->db_plugin);
        share->db_plugin = my_plugin_lock(nullptr, &tmp_plugin);
        DBUG_PRINT("info", ("setting dbtype to '%.*s' (%d)", str_db_type_length,
                            next_chunk + 2, ha_legacy_type(share->db_type())));
      } else if (!tmp_plugin && name.length == 18 &&
                 !strncmp(name.str, "PERFORMANCE_SCHEMA", name.length)) {
        /*
          A FRM file is present on disk,
          for a PERFORMANCE_SCHEMA table,
          but this server binary is not compiled with the performance_schema,
          as ha_resolve_by_name() did not find the storage engine.
          This can happen:
          - in production, when random binaries (without P_S) are thrown
            on top of random installed database instances on disk (with P_S).
          For the sake of robustness, pretend the table simply does not exist,
          so that in particular it does not pollute the information_schema
          with errors when scanning the disk for FRM files.
          Note that ER_NO_SUCH_TABLE has a special treatment
          in fill_schema_table_by_open()
        */
        error = 1;
        my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str,
                 share->table_name.str);
        goto err;
      } else if (!tmp_plugin) {
        /* purecov: begin inspected */
        error = 8;
        const_cast<char *>(name.str)[name.length] = 0;
        my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), name.str);
        goto err;
        /* purecov: end */
      }
      next_chunk += str_db_type_length + 2;
    }
    if (next_chunk + 5 < buff_end) {
      uint32 partition_info_str_len = uint4korr(next_chunk);
      if ((share->partition_info_str_len = partition_info_str_len)) {
        if (!(share->partition_info_str =
                  (char *)memdup_root(&share->mem_root, next_chunk + 4,
                                      partition_info_str_len + 1))) {
          goto err;
        }
      }
      next_chunk += 5 + partition_info_str_len;
    }
    if (share->mysql_version >= 50110 && next_chunk < buff_end) {
      /* New auto_partitioned indicator introduced in 5.1.11 */
      share->auto_partitioned = *next_chunk;
      next_chunk++;
    }
    keyinfo = share->key_info;
    for (i = 0; i < keys; i++, keyinfo++) {
      if (keyinfo->flags & HA_USES_PARSER) {
        if (next_chunk >= buff_end) {
          DBUG_PRINT("error",
                     ("fulltext key uses parser that is not defined in .frm"));
          goto err;
        }
        LEX_CSTRING parser_name = {
            reinterpret_cast<char *>(next_chunk),
            strlen(reinterpret_cast<char *>(next_chunk))};
        next_chunk += parser_name.length + 1;
        keyinfo->parser =
            my_plugin_lock_by_name(nullptr, parser_name, MYSQL_FTPARSER_PLUGIN);
        if (!keyinfo->parser) {
          my_error(ER_PLUGIN_IS_NOT_LOADED, MYF(0), parser_name.str);
          goto err;
        }
      }
    }
    if (forminfo[46] == (uchar)255) {
      // reading long table comment
      if (next_chunk + 2 > buff_end) {
        DBUG_PRINT("error", ("long table comment is not defined in .frm"));
        goto err;
      }
      share->comment.length = uint2korr(next_chunk);
      if (!(share->comment.str =
                strmake_root(&share->mem_root, (char *)next_chunk + 2,
                             share->comment.length))) {
        goto err;
      }
      next_chunk += 2 + share->comment.length;
    }

    if (next_chunk + format_section_header_size < buff_end) {
      /*
        New extra data segment called "format section" with additional
        table and column properties introduced by MySQL Cluster
        based on 5.1.20

        Table properties:
        TABLESPACE <ts> and STORAGE [DISK|MEMORY]

        Column properties:
        COLUMN_FORMAT [DYNAMIC|FIXED] and STORAGE [DISK|MEMORY]
      */
      DBUG_PRINT("info", ("Found format section"));

      /* header */
      const uint format_section_length = uint2korr(next_chunk);
      const uint format_section_flags = uint4korr(next_chunk + 2);
      /* 2 bytes unused */

      if (next_chunk + format_section_length > buff_end) {
        DBUG_PRINT("error", ("format section length too long: %u",
                             format_section_length));
        goto err;
      }
      DBUG_PRINT("info", ("format_section_length: %u, format_section_flags: %u",
                          format_section_length, format_section_flags));

      share->default_storage_media =
          (enum ha_storage_media)(format_section_flags & 0x7);

      /* tablespace */
      const char *tablespace =
          (const char *)next_chunk + format_section_header_size;
      const size_t tablespace_length = strlen(tablespace);
      share->tablespace = nullptr;
      if (tablespace_length) {
        Tablespace_name_error_handler error_handler;
        thd->push_internal_handler(&error_handler);
        bool name_check_error = validate_tablespace_name_length(tablespace);
        thd->pop_internal_handler();
        if (!name_check_error &&
            !(share->tablespace = strmake_root(&share->mem_root, tablespace,
                                               tablespace_length + 1))) {
          goto err;
        }
      }
      DBUG_PRINT("info", ("tablespace: '%s'",
                          share->tablespace ? share->tablespace : "<null>"));

      /* pointer to format section for fields */
      format_section_fields =
          next_chunk + format_section_header_size + tablespace_length + 1;

      next_chunk += format_section_length;
    }

    if (next_chunk + 2 <= buff_end) {
      share->compress.length = uint2korr(next_chunk);
      if (!(share->compress.str =
                strmake_root(&share->mem_root, (char *)next_chunk + 2,
                             share->compress.length))) {
        goto err;
      }
      next_chunk += 2 + share->compress.length;
    }

    if (next_chunk + 2 <= buff_end) {
      share->encrypt_type.length = uint2korr(next_chunk);
      if (!(share->encrypt_type.str =
                strmake_root(&share->mem_root, (char *)next_chunk + 2,
                             share->encrypt_type.length))) {
        goto err;
      }
      next_chunk += 2 + share->encrypt_type.length;
    }
  }
  share->key_block_size = uint2korr(head + 62);

  error = 4;
  extra_rec_buf_length = uint2korr(head + 59);
  rec_buff_length = ALIGN_SIZE(share->reclength + 1 + extra_rec_buf_length);
  share->rec_buff_length = rec_buff_length;
  if (!(record = (uchar *)share->mem_root.Alloc(rec_buff_length)))
    goto err; /* purecov: inspected */
  share->default_values = record;
  if (mysql_file_pread(file, record, (size_t)share->reclength, record_offset,
                       MYF(MY_NABP)))
    goto err; /* purecov: inspected */

  mysql_file_seek(file, pos + 288, MY_SEEK_SET, MYF(0));

  share->fields = uint2korr(forminfo + 258);
  pos = uint2korr(forminfo + 260); /* Length of all screens */
  n_length = uint2korr(forminfo + 268);
  interval_count = uint2korr(forminfo + 270);
  interval_parts = uint2korr(forminfo + 272);
  int_length = uint2korr(forminfo + 274);
  share->null_fields = uint2korr(forminfo + 282);
  com_length = uint2korr(forminfo + 284);
  gcol_screen_length = uint2korr(forminfo + 286);
  share->vfields = 0;
  frm_context->stored_fields = share->fields;
  if (forminfo[46] != (uchar)255) {
    share->comment.length = (int)(forminfo[46]);
    share->comment.str = strmake_root(&share->mem_root, (char *)forminfo + 47,
                                      share->comment.length);
  }

  DBUG_PRINT("info", ("i_count: %d  i_parts: %d  index: %d  n_length: %d  "
                      "int_length: %d  com_length: %d  gcol_screen_length: %d",
                      interval_count, interval_parts, share->keys, n_length,
                      int_length, com_length, gcol_screen_length));
  if (!(field_ptr = (Field **)share->mem_root.Alloc((uint)(
            (share->fields + 1) * sizeof(Field *) +
            interval_count * sizeof(TYPELIB) +
            (share->fields + interval_parts + keys + 3) * sizeof(char *) +
            (n_length + int_length + com_length + gcol_screen_length)))))
    goto err; /* purecov: inspected */

  share->field = field_ptr;
  read_length =
      (uint)(share->fields * field_pack_length + pos +
             (uint)(n_length + int_length + com_length + gcol_screen_length));
  if (read_string(file, &disk_buff, read_length))
    goto err; /* purecov: inspected */

  strpos = disk_buff + pos;

  share->intervals = (TYPELIB *)(field_ptr + share->fields + 1);
  interval_array = (const char **)(share->intervals + interval_count);
  names = (char *)(interval_array + share->fields + interval_parts + keys + 3);
  if (!interval_count) share->intervals = nullptr;  // For better debugging
  memcpy(names, strpos + (share->fields * field_pack_length),
         (uint)(n_length + int_length));
  orig_comment_pos = comment_pos = names + (n_length + int_length);
  memcpy(comment_pos, disk_buff + read_length - com_length - gcol_screen_length,
         com_length);
  orig_gcol_screen_pos = gcol_screen_pos =
      names + (n_length + int_length + com_length);
  memcpy(gcol_screen_pos, disk_buff + read_length - gcol_screen_length,
         gcol_screen_length);

  fix_type_pointers(&interval_array, &frm_context->fieldnames, 1, &names);
  if (frm_context->fieldnames.count != share->fields) goto err;
  fix_type_pointers(&interval_array, share->intervals, interval_count, &names);

  {
    /* Set ENUM and SET lengths */
    TYPELIB *interval;
    for (interval = share->intervals;
         interval < share->intervals + interval_count; interval++) {
      uint count = (uint)(interval->count + 1) * sizeof(uint);
      if (!(interval->type_lengths = (uint *)share->mem_root.Alloc(count)))
        goto err;
      for (count = 0; count < interval->count; count++) {
        const char *val = interval->type_names[count];
        interval->type_lengths[count] = strlen(val);
      }
      interval->type_lengths[count] = 0;
    }
  }

  if (keynames)
    fix_type_pointers(&interval_array, &share->keynames, 1, &keynames);

  /* Allocate handler */
  if (!(handler_file =
            get_new_handler(share, share->partition_info_str_len != 0,
                            thd->mem_root, share->db_type())))
    goto err;

  if (handler_file->set_ha_share_ref(&share->ha_share)) goto err;

  if (frm_context->null_field_first) {
    null_flags = null_pos = share->default_values;
    null_bit_pos = (db_create_options & HA_OPTION_PACK_RECORD) ? 0 : 1;
    /*
      null_bytes below is only correct under the condition that
      there are no bit fields.  Correct values is set below after the
      table struct is initialized
    */
    share->null_bytes = (share->null_fields + null_bit_pos + 7) / 8;
  } else {
    share->null_bytes = (share->null_fields + 7) / 8;
    null_flags = null_pos =
        share->default_values + share->reclength - share->null_bytes;
    null_bit_pos = 0;
  }

  use_hash = share->fields >= MAX_FIELDS_BEFORE_HASH;
  if (use_hash)
    share->name_hash = new collation_unordered_map<std::string, Field **>(
        system_charset_info, PSI_INSTRUMENT_ME);

  for (i = 0; i < share->fields; i++, strpos += field_pack_length) {
    if (new_frm_ver >= 3 &&
        (strpos[10] &
         FRM_context::GENERATED_FIELD) &&   // former Field::unireg_check
        !(bool)(uint)(gcol_screen_pos[3]))  // Field::stored_in_db
    {
      /*
        Skip virtual generated columns as we will do separate pass for them.

        We still need to advance pointers to current comment and generated
        column info in for such fields.
      */
      comment_pos += uint2korr(strpos + 15);
      gcol_screen_pos += uint2korr(gcol_screen_pos + 1) + FRM_GCOL_HEADER_SIZE;
      has_vgc = true;
    } else {
      if ((error = make_field_from_frm(
               thd, share, frm_context, new_frm_ver, use_hash, i, strpos,
               format_section_fields, &comment_pos, &gcol_screen_pos, &null_pos,
               &null_bit_pos, &errarg)))
        goto err;
    }
  }

  if (has_vgc) {
    /*
      We need to do separate pass through field descriptions for virtual
      generated columns to ensure that they get allocated null/leftover
      bits at the tail of record preamble.
    */
    strpos = disk_buff + pos;
    comment_pos = orig_comment_pos;
    gcol_screen_pos = orig_gcol_screen_pos;
    // Generated columns can be present only in new .FRMs.
    DBUG_ASSERT(new_frm_ver >= 3);
    for (i = 0; i < share->fields; i++, strpos += field_pack_length) {
      if ((strpos[10] &
           FRM_context::GENERATED_FIELD) &&   // former Field::unireg_check
          !(bool)(uint)(gcol_screen_pos[3]))  // Field::stored_in_db
      {
        if ((error = make_field_from_frm(
                 thd, share, frm_context, new_frm_ver, use_hash, i, strpos,
                 format_section_fields, &comment_pos, &gcol_screen_pos,
                 &null_pos, &null_bit_pos, &errarg)))
          goto err;
      } else {
        /*
          Advance pointers to current comment and generated columns
          info for stored fields.
        */
        comment_pos += uint2korr(strpos + 15);
        if (strpos[10] &
            FRM_context::GENERATED_FIELD)  // former Field::unireg_check
        {
          gcol_screen_pos +=
              uint2korr(gcol_screen_pos + 1) + FRM_GCOL_HEADER_SIZE;
        }
      }
    }
  }
  error = 4;
  share->field[share->fields] = nullptr;  // End marker
  /* Sanity checks: */
  DBUG_ASSERT(share->fields >= frm_context->stored_fields);
  DBUG_ASSERT(share->reclength >= share->stored_rec_length);

  /* Fix key->name and key_part->field */
  if (key_parts) {
    const int pk_off =
        find_type(primary_key_name, &share->keynames, FIND_TYPE_NO_PREFIX);
    uint primary_key = (pk_off > 0 ? pk_off - 1 : MAX_KEY);

    /*
      The following if-else is here for MyRocks:
      set share->primary_key as early as possible, because the return value
      of ha_rocksdb::index_flags(key, ...) (HA_KEYREAD_ONLY bit in particular)
      depends on whether the key is the primary key.
    */
    if (primary_key < MAX_KEY && share->keys_in_use.is_set(primary_key)) {
      share->primary_key = primary_key;
    } else {
      share->primary_key = MAX_KEY;
    }

    longlong ha_option = handler_file->ha_table_flags();
    keyinfo = share->key_info;
    key_part = keyinfo->key_part;

    for (uint key = 0; key < share->keys; key++, keyinfo++) {
      uint usable_parts = 0;
      keyinfo->name = share->keynames.type_names[key];
      /* Fix fulltext keys for old .frm files */
      if (share->key_info[key].flags & HA_FULLTEXT)
        share->key_info[key].algorithm = HA_KEY_ALG_FULLTEXT;

      if (primary_key >= MAX_KEY && (keyinfo->flags & HA_NOSAME)) {
        /*
          If the UNIQUE key doesn't have NULL columns and is not a part key
          declare this as a primary key.
        */
        primary_key = key;
        for (i = 0; i < keyinfo->user_defined_key_parts; i++) {
          DBUG_ASSERT(key_part[i].fieldnr > 0);
          // Table field corresponding to the i'th key part.
          Field *table_field = share->field[key_part[i].fieldnr - 1];

          // Index on virtual generated columns is not allowed to be PK
          // even when the conditions below are true, so this case must be
          // rejected here.
          if (table_field->is_virtual_gcol()) {
            primary_key = MAX_KEY;  // Can't be used
            break;
          }

          /*
            If the key column is of NOT NULL BLOB type, then it
            will definitly have key prefix. And if key part prefix size
            is equal to the BLOB column max size, then we can promote
            it to primary key.
          */
          if (!table_field->is_nullable() &&
              table_field->type() == MYSQL_TYPE_BLOB &&
              table_field->field_length == key_part[i].length)
            continue;

          if (table_field->is_nullable() ||
              table_field->key_length() != key_part[i].length)

          {
            primary_key = MAX_KEY;  // Can't be used
            break;
          }
        }

        /*
          The following is here for MyRocks. See the comment above
          about "set share->primary_key as early as possible"
        */
        if (primary_key < MAX_KEY && share->keys_in_use.is_set(primary_key)) {
          share->primary_key = primary_key;
        }
      }

      for (i = 0; i < keyinfo->user_defined_key_parts; key_part++, i++) {
        Field *field;
        if (new_field_pack_flag <= 1)
          key_part->fieldnr = (uint16)find_field(
              share->field, share->default_values, (uint)key_part->offset,
              (uint)key_part->length);
        if (!key_part->fieldnr) {
          error = 4;  // Wrong file
          goto err;
        }
        field = key_part->field = share->field[key_part->fieldnr - 1];
        key_part->type = field->key_type();
        if (field->is_nullable()) {
          key_part->null_offset = field->null_offset(share->default_values);
          key_part->null_bit = field->null_bit;
          key_part->store_length += HA_KEY_NULL_LENGTH;
          keyinfo->flags |= HA_NULL_PART_KEY;
          keyinfo->key_length += HA_KEY_NULL_LENGTH;
        }
        if (field->type() == MYSQL_TYPE_BLOB ||
            field->real_type() == MYSQL_TYPE_VARCHAR ||
            field->type() == MYSQL_TYPE_GEOMETRY) {
          key_part->store_length += HA_KEY_BLOB_LENGTH;
          if (i + 1 <= keyinfo->user_defined_key_parts)
            keyinfo->key_length += HA_KEY_BLOB_LENGTH;
        }
        key_part->init_flags();

        if (field->is_virtual_gcol()) keyinfo->flags |= HA_VIRTUAL_GEN_KEY;

        setup_key_part_field(share, handler_file, primary_key, keyinfo, key, i,
                             &usable_parts, true);

        field->flags |= PART_KEY_FLAG;
        if (key == primary_key) {
          field->flags |= PRI_KEY_FLAG;
          /*
            "if (ha_option & HA_PRIMARY_KEY_IN_READ_INDEX)" ... was moved below
            for MyRocks
          */
        }
        if (field->key_length() != key_part->length) {
          if (field->type() == MYSQL_TYPE_NEWDECIMAL) {
            /*
              Fix a fatal error in decimal key handling that causes crashes
              on Innodb. We fix it by reducing the key length so that
              InnoDB never gets a too big key when searching.
              This allows the end user to do an ALTER TABLE to fix the
              error.
            */
            keyinfo->key_length -= (key_part->length - field->key_length());
            key_part->store_length -=
                (uint16)(key_part->length - field->key_length());
            key_part->length = (uint16)field->key_length();
            LogErr(ERROR_LEVEL, ER_TABLE_WRONG_KEY_DEFINITION,
                   share->table_name.str, share->table_name.str);
            push_warning_printf(thd, Sql_condition::SL_WARNING,
                                ER_CRASHED_ON_USAGE,
                                "Found wrong key definition in %s; "
                                "Please do \"ALTER TABLE `%s` FORCE\" to fix "
                                "it!",
                                share->table_name.str, share->table_name.str);
            share->crashed = true;  // Marker for CHECK TABLE
            continue;
          }
          key_part->key_part_flag |= HA_PART_KEY_SEG;
        }
      }

      if (use_extended_sk && primary_key < MAX_KEY && key &&
          !(keyinfo->flags & HA_NOSAME))
        key_part +=
            add_pk_parts_to_sk(keyinfo, key, share->key_info, primary_key,
                               share, handler_file, &usable_parts);

      /* Skip unused key parts if they exist */
      key_part += keyinfo->unused_key_parts;

      keyinfo->usable_key_parts = usable_parts;  // Filesort

      share->max_key_length =
          std::max(share->max_key_length,
                   keyinfo->key_length + keyinfo->user_defined_key_parts);
      share->total_key_length += keyinfo->key_length;
      /*
        MERGE tables do not have unique indexes. But every key could be
        an unique index on the underlying MyISAM table. (Bug #10400)
      */
      if ((keyinfo->flags & HA_NOSAME) ||
          (ha_option & HA_ANY_INDEX_MAY_BE_UNIQUE))
        share->max_unique_length =
            std::max(share->max_unique_length, keyinfo->key_length);
    }

    /*
      The next call is here for MyRocks:  Now, we have filled in field and key
      definitions, give the storage engine a chance to adjust its properties.
      MyRocks may (and typically does) adjust HA_PRIMARY_KEY_IN_READ_INDEX
      flag in this call.
    */
    if (handler_file->init_with_fields()) goto err;

    if (primary_key < MAX_KEY &&
        (handler_file->ha_table_flags() & HA_PRIMARY_KEY_IN_READ_INDEX)) {
      keyinfo = &share->key_info[primary_key];
      key_part = keyinfo->key_part;
      for (i = 0; i < keyinfo->user_defined_key_parts; key_part++, i++) {
        Field *field = key_part->field;
        /*
          If this field is part of the primary key and all keys contains
          the primary key, then we can use any key to find this column
        */
        if (field->key_length() == key_part->length &&
            !(field->flags & BLOB_FLAG))
          field->part_of_key = share->keys_in_use;
        if (field->part_of_sortkey.is_set(primary_key))
          field->part_of_sortkey = share->keys_in_use;
      }
    }

    if (share->primary_key != MAX_KEY) {
      /*
        If we are using an integer as the primary key then allow the user to
        refer to it as '_rowid'
      */
      if (share->key_info[primary_key].user_defined_key_parts == 1) {
        Field *field = share->key_info[primary_key].key_part[0].field;
        if (field && field->result_type() == INT_RESULT) {
          /* note that fieldnr here (and rowid_field_offset) starts from 1 */
          share->rowid_field_offset =
              (share->key_info[primary_key].key_part[0].fieldnr);
        }
      }
    }
  } else
    share->primary_key = MAX_KEY;
  my_free(disk_buff);
  disk_buff = nullptr;
  if (new_field_pack_flag <= 1) {
    /* Old file format with default as not null */
    uint null_length = (share->null_fields + 7) / 8;
    memset(share->default_values + (null_flags - record), 255, null_length);
  }

  if (share->found_next_number_field) {
    Field *reg_field = *share->found_next_number_field;
    if ((int)(share->next_number_index = (uint)find_ref_key(
                  share->key_info, share->keys, share->default_values,
                  reg_field, &share->next_number_key_offset,
                  &share->next_number_keypart)) < 0) {
      /* Wrong field definition */
      error = 4;
      goto err;
    } else
      reg_field->flags |= AUTO_INCREMENT_FLAG;
  }

  if (share->blob_fields) {
    Field **ptr;
    uint k, *save;

    /* Store offsets to blob fields to find them fast */
    if (!(share->blob_field = save = (uint *)share->mem_root.Alloc(
              (uint)(share->blob_fields * sizeof(uint)))))
      goto err;
    for (k = 0, ptr = share->field; *ptr; ptr++, k++) {
      if ((*ptr)->flags & BLOB_FLAG) (*save++) = k;
    }
  }

  /*
    the correct null_bytes can now be set, since bitfields have been taken
    into account
  */
  share->null_bytes = (null_pos - null_flags + (null_bit_pos + 7) / 8);
  share->last_null_bit_pos = null_bit_pos;

  share->db_low_byte_first = handler_file->low_byte_first();
  share->column_bitmap_size = bitmap_buffer_size(share->fields);

  if (!(bitmaps =
            (my_bitmap_map *)share->mem_root.Alloc(share->column_bitmap_size)))
    goto err;
  bitmap_init(&share->all_set, bitmaps, share->fields);
  bitmap_set_all(&share->all_set);

  destroy(handler_file);
  my_free(extra_segment_buff);
  return 0;

err:
  my_free(disk_buff);
  my_free(extra_segment_buff);
  destroy(handler_file);
  delete share->name_hash;
  share->name_hash = nullptr;

  open_table_error(thd, share, error, my_errno());
  return error;
} /*open_binary_frm*/

//////////////////////////////////////////////////////////////////////////

/**
  Validate the expression to see whether there are invalid Item objects.

  Needs to be done after fix_fields to allow checking references
  to other generated columns, default value expressions or check constraints.

  @param expr         Pointer to the expression
  @param source       Source of value generator(a generated column, a regular
                      column with generated default value or
                      a check constraint).
  @param source_name  Name of the source (generated column, a reguler column
                      with generated default value or a check constraint).
  @param column_index The column order.

  @retval true  The generated expression has some invalid objects
  @retval false No illegal objects in the generated expression
 */
static bool validate_value_generator_expr(Item *expr,
                                          Value_generator_source source,
                                          const char *source_name,
                                          int column_index) {
  DBUG_TRACE;
  DBUG_ASSERT(expr);

  // Map to get actual error code from error_type for the source.
  enum error_type { ER_NAME_FUNCTION, ER_FUNCTION, ER_VARIABLES, MAX_ERROR };
  uint error_code_map[][MAX_ERROR] = {
      // Generated column errors.
      {ER_GENERATED_COLUMN_NAMED_FUNCTION_IS_NOT_ALLOWED,
       ER_GENERATED_COLUMN_FUNCTION_IS_NOT_ALLOWED,
       ER_GENERATED_COLUMN_VARIABLES},
      // Default expressions errors.
      {ER_DEFAULT_VAL_GENERATED_NAMED_FUNCTION_IS_NOT_ALLOWED,
       ER_DEFAULT_VAL_GENERATED_FUNCTION_IS_NOT_ALLOWED,
       ER_DEFAULT_VAL_GENERATED_VARIABLES},
      // Check constraint errors.
      {ER_CHECK_CONSTRAINT_NAMED_FUNCTION_IS_NOT_ALLOWED,
       ER_CHECK_CONSTRAINT_FUNCTION_IS_NOT_ALLOWED,
       ER_CHECK_CONSTRAINT_VARIABLES}};
  uint err_code = error_code_map[source][ER_NAME_FUNCTION];

  // No non-deterministic functions are allowed as GC but most of them are
  // allowed as default value expressions
  if ((expr->used_tables() & RAND_TABLE_BIT &&
       (source == VGS_GENERATED_COLUMN))) {
    Item_func *func_item;
    if (expr->type() == Item::FUNC_ITEM &&
        ((func_item = down_cast<Item_func *>(expr)))) {
      my_error(err_code, MYF(0), source_name, func_item->func_name());
      return true;
    } else {
      my_error(error_code_map[source][ER_FUNCTION], MYF(0), source_name);
      return true;
    }
  }
  // System variables or parameters are not allowed
  else if (expr->used_tables() & INNER_TABLE_BIT) {
    my_error(error_code_map[source][ER_VARIABLES], MYF(0), source_name);
    return true;
  }

  // Assert that we aren't dealing with ROW values (rejected in
  // pre_validate_value_generator_expr()).
  DBUG_ASSERT(expr->cols() == 1);

  // Sub-queries are not allowed (already checked by parser, hence the assert)
  DBUG_ASSERT(!expr->has_subquery());
  /*
    Walk through the Item tree, checking the validity of items
    belonging to the expression.
  */
  Check_function_as_value_generator_parameters checker_args(err_code, source);
  checker_args.col_index = column_index;
  if (expr->walk(&Item::check_function_as_value_generator, enum_walk::POSTFIX,
                 pointer_cast<uchar *>(&checker_args))) {
    my_error(checker_args.err_code, MYF(0), source_name,
             checker_args.banned_function_name);
    return true;
  }

  // Stored programs are not allowed. This case is already covered, but still
  // keeping it here as a safetynet.
  if (expr->has_stored_program()) {
    /* purecov: begin deadcode */
    DBUG_ASSERT(false);
    my_error(err_code, MYF(0), source_name, "stored progam");
    return true;
    /* purecov: end */
  }

  return false;
}

/**
  Process the generated expression, generated default value of the column or
  check constraint expression.

  @param thd             The thread object
  @param table           The table to which the column belongs
  @param val_generator   The expression to unpack
  @param source          Source of value generator(a generated column, a regular
                         column with generated default value or
                         a check constraint).
  @param source_name     Name of the source (generated column, a reguler column
                         with generated default value or a check constraint).
  @param field           Field to which the val_generator is attached to for
                         generated columns and default expression.

  @retval true An error occurred, something was wrong with the function.
  @retval false Ok, generated expression is fixed sucessfully
 */
static bool fix_value_generators_fields(THD *thd, TABLE *table,
                                        Value_generator *val_generator,
                                        Value_generator_source source,
                                        const char *source_name, Field *field) {
  uint dir_length, home_dir_length;
  bool result = true;
  Item *func_expr = val_generator->expr_item;
  TABLE_LIST tables;
  TABLE_LIST *save_table_list, *save_first_table, *save_last_table;
  int error;
  Name_resolution_context *context;
  const char *save_where;
  char *db_name;
  char db_name_string[FN_REFLEN];
  bool save_use_only_table_context;
  std::unique_ptr<Functional_index_error_handler>
      functional_index_error_handler;
  enum_mark_columns save_mark_used_columns = thd->mark_used_columns;
  DBUG_ASSERT(func_expr);
  DBUG_TRACE;

  // Insert a error handler that takes care of converting column names to
  // functional index names. Since functional indexes is implemented as
  // indexed hidden generated columns, we may end up printing out the
  // auto-generated column name if we don't have an extra error handler.
  if (source == VGS_GENERATED_COLUMN)
    functional_index_error_handler =
        std::unique_ptr<Functional_index_error_handler>(
            new Functional_index_error_handler(field, thd));

  /*
    Set-up the TABLE_LIST object to be a list with a single table
    Set alias and real name to table name and get database name from file name.
  */

  tables.alias = tables.table_name = table->s->table_name.str;
  tables.table = table;
  tables.next_local = nullptr;
  tables.next_name_resolution_table = nullptr;
  my_stpmov(db_name_string, table->s->normalized_path.str);
  dir_length = dirname_length(db_name_string);
  db_name_string[dir_length ? dir_length - 1 : 0] = 0;
  home_dir_length = dirname_length(db_name_string);
  db_name = &db_name_string[home_dir_length];
  tables.db = db_name;

  thd->mark_used_columns = MARK_COLUMNS_NONE;

  context = thd->lex->current_context();
  table->get_fields_in_item_tree = true;
  save_table_list = context->table_list;
  save_first_table = context->first_name_resolution_table;
  save_last_table = context->last_name_resolution_table;
  context->table_list = &tables;
  context->first_name_resolution_table = &tables;
  context->last_name_resolution_table = nullptr;
  Item_ident::Change_context ctx(context);
  func_expr->walk(&Item::change_context_processor, enum_walk::POSTFIX,
                  (uchar *)&ctx);
  save_where = thd->where;

  std::string where_str;
  if (source == VGS_GENERATED_COLUMN || source == VGS_DEFAULT_EXPRESSION) {
    thd->where = field->is_field_for_functional_index()
                     ? "functional index"
                     : (source == VGS_GENERATED_COLUMN)
                           ? "generated column function"
                           : "default value expression";
  } else {
    DBUG_ASSERT(source == VGS_CHECK_CONSTRAINT);
    where_str.reserve(256);
    where_str.append(STRING_WITH_LEN("check constraint "));
    where_str.append(source_name);
    where_str.append(STRING_WITH_LEN(" expression"));
    thd->where = where_str.c_str();
  }

  /* Save the context before fixing the fields*/
  save_use_only_table_context = thd->lex->use_only_table_context;
  thd->lex->use_only_table_context = true;

  bool charset_switched = false;
  const CHARSET_INFO *saved_collation_connection = func_expr->default_charset();
  if (saved_collation_connection != table->s->table_charset) {
    thd->variables.collation_connection = table->s->table_charset;
    charset_switched = true;
  }

  Item *new_func = func_expr;
  if (field && field->is_field_for_functional_index())
    func_expr->allow_array_cast();
  error = func_expr->fix_fields(thd, &new_func);

  /* Virtual columns expressions that substitute themselves are invalid */
  DBUG_ASSERT(new_func == func_expr);

  /* Restore the current connection character set and collation. */
  if (charset_switched)
    thd->variables.collation_connection = saved_collation_connection;

  /* Restore the original context*/
  thd->lex->use_only_table_context = save_use_only_table_context;
  context->table_list = save_table_list;
  context->first_name_resolution_table = save_first_table;
  context->last_name_resolution_table = save_last_table;

  /*
    Above, 'context' is either the one of unpack_value_generator()'s temporary
    fresh LEX 'new_lex', or the one of the top query as used in
    TABLE::refix_value_generator_items(). None of them reflects where the val
    generator is situated in the query.  Moreover, a gcol_info may be shared
    by N references to the same gcol, each ref being in a different context
    (top query, subquery). So, underlying items are not situated in a defined
    place: give them a null context.
  */
  Item_ident::Change_context nul_ctx(nullptr);
  func_expr->walk(&Item::change_context_processor, enum_walk::POSTFIX,
                  (uchar *)&nul_ctx);

  if (unlikely(error)) {
    DBUG_PRINT("info",
               ("Field in generated column function not part of table"));
    goto end;
  }
  thd->where = save_where;
  /*
    Checking if all items are valid to be part of the expression.
  */
  if (validate_value_generator_expr(func_expr, source, source_name,
                                    field ? field->field_index : 0))
    goto end;

  result = false;

  func_expr->walk(&Item::strip_db_table_name_processor, enum_walk::POSTFIX,
                  nullptr);

end:
  table->get_fields_in_item_tree = false;
  thd->mark_used_columns = save_mark_used_columns;
  return result;
}

/**
  Calculate the base_columns_map and num_non_virtual_base_cols members of
  this generated column

  @param table    Table with the checked field

  @retval true if error
 */

bool Value_generator::register_base_columns(TABLE *table) {
  DBUG_TRACE;
  my_bitmap_map *bitbuf = static_cast<my_bitmap_map *>(
      table->mem_root.Alloc(bitmap_buffer_size(table->s->fields)));
  DBUG_ASSERT(num_non_virtual_base_cols == 0);
  bitmap_init(&base_columns_map, bitbuf, table->s->fields);

  MY_BITMAP *save_old_read_set = table->read_set;
  table->read_set = &base_columns_map;
  Mark_field mark_fld(MARK_COLUMNS_TEMP);
  expr_item->walk(&Item::mark_field_in_map, enum_walk::PREFIX,
                  (uchar *)&mark_fld);
  table->read_set = save_old_read_set;

  /* Calculate the number of non-virtual base columns */
  for (uint i = 0; i < table->s->fields; i++) {
    Field *field = table->field[i];
    if (bitmap_is_set(&base_columns_map, field->field_index) &&
        field->stored_in_db)
      num_non_virtual_base_cols++;
  }
  return false;
}

void Value_generator::dup_expr_str(MEM_ROOT *root, const char *src,
                                   size_t len) {
  expr_str.str = pointer_cast<char *>(memdup_root(root, src, len));
  expr_str.length = len;
}

void Value_generator::print_expr(THD *thd, String *out) {
  out->length(0);
  Sql_mode_parse_guard parse_guard(thd);
  // Printing db and table name is useless
  auto flags = enum_query_type(QT_NO_DB | QT_NO_TABLE | QT_FORCE_INTRODUCERS);
  expr_item->print(thd, out, flags);
}

bool unpack_value_generator(THD *thd, TABLE *table,
                            Value_generator **val_generator,
                            Value_generator_source source,
                            const char *source_name, Field *field,
                            bool is_create_table, bool *error_reported) {
  DBUG_TRACE;
  DBUG_ASSERT(field == nullptr || field->table == table);

  LEX_STRING *val_gen_expr = &(*val_generator)->expr_str;
  DBUG_ASSERT(val_gen_expr);
  DBUG_ASSERT(!(*val_generator)->expr_item);  // No Item in TABLE_SHARE
  /*
    Step 1: Construct a statement for the parser.
    The parsed string needs to take the following format:
    "PARSE_GCOL_EXPR (<expr_string_from_frm>)"
  */
  char *gcol_expr_str;
  int str_len = 0;
  const CHARSET_INFO *old_character_set_client;
  bool disable_strict_mode = false;
  bool status;

  Strict_error_handler strict_handler;

  LEX *const old_lex = thd->lex;
  LEX new_lex;
  thd->lex = &new_lex;
  if (lex_start(thd)) {
    thd->lex = old_lex;
    return true;  // OOM
  }

  if (!(gcol_expr_str = (char *)table->mem_root.Alloc(
            val_gen_expr->length + PARSE_GCOL_KEYWORD.length + 3))) {
    return true;
  }
  memcpy(gcol_expr_str, PARSE_GCOL_KEYWORD.str, PARSE_GCOL_KEYWORD.length);
  str_len = PARSE_GCOL_KEYWORD.length;
  memcpy(gcol_expr_str + str_len, "(", 1);
  str_len++;
  memcpy(gcol_expr_str + str_len, val_gen_expr->str, val_gen_expr->length);
  str_len += val_gen_expr->length;
  memcpy(gcol_expr_str + str_len, ")", 1);
  str_len++;
  memcpy(gcol_expr_str + str_len, "\0", 1);
  str_len++;
  Gcol_expr_parser_state parser_state;
  parser_state.init(thd, gcol_expr_str, str_len);

  /*
    Step 2: Setup thd for parsing.
  */
  Query_arena *backup_stmt_arena_ptr = thd->stmt_arena;
  Query_arena backup_arena;
  Query_arena gcol_arena(&table->mem_root, Query_arena::STMT_REGULAR_EXECUTION);
  thd->swap_query_arena(gcol_arena, &backup_arena);
  thd->stmt_arena = &gcol_arena;
  ulong save_old_privilege = thd->want_privilege;
  thd->want_privilege = 0;

  old_character_set_client = thd->variables.character_set_client;
  // Subquery is not allowed in generated expression
  const bool save_allow_subselects = thd->lex->expr_allows_subselect;
  thd->lex->expr_allows_subselect = false;
  // allow_sum_func is also 0, banning group aggregates and window functions.

  /*
    Step 3: Use the parser to build an Item object from.
  */
  if (parse_sql(thd, &parser_state, nullptr)) {
    goto parse_err;
  }
  thd->lex->expr_allows_subselect = save_allow_subselects;

  /*
    From now on use val_generator generated by the parser. It has an
    expr_item, and no expr_str.
  */
  *val_generator = parser_state.result;
  /* Keep attribute of generated column */
  if (field != nullptr) (*val_generator)->set_field_stored(field->stored_in_db);

  DBUG_ASSERT((*val_generator)->expr_item && !(*val_generator)->expr_str.str);

  /* Use strict mode regardless of strict mode setting when validating */
  if (!thd->is_strict_sql_mode()) {
    thd->variables.sql_mode |= MODE_STRICT_ALL_TABLES;
    thd->push_internal_handler(&strict_handler);
    disable_strict_mode = true;
  }

  /* Validate the Item tree. */
  status = fix_value_generators_fields(thd, table, (*val_generator), source,
                                       source_name, field);

  // Permanent changes to the item_tree are completed.
  if (!thd->lex->is_ps_or_view_context_analysis())
    (*val_generator)->permanent_changes_completed = true;

  if (disable_strict_mode) {
    thd->pop_internal_handler();
    thd->variables.sql_mode &= ~MODE_STRICT_ALL_TABLES;
  }
  if (status) {
    if (is_create_table) {
      /*
        During CREATE/ALTER TABLE it is ok to receive errors here.
        It is not ok if it happens during the opening of an frm
        file as part of a normal query.
      */
      *error_reported = true;
    }
    // Any memory allocated in this function is freed in parse_err
    *val_generator = nullptr;
    goto parse_err;
  }
  if ((*val_generator)->register_base_columns(table)) goto parse_err;
  lex_end(thd->lex);
  thd->lex = old_lex;
  (*val_generator)->backup_stmt_unsafe_flags(new_lex.get_stmt_unsafe_flags());
  thd->stmt_arena = backup_stmt_arena_ptr;
  thd->swap_query_arena(backup_arena, &gcol_arena);
  (*val_generator)->item_list = gcol_arena.item_list();
  thd->want_privilege = save_old_privilege;
  thd->lex->expr_allows_subselect = save_allow_subselects;

  return false;

parse_err:
  // Any created window is eliminated as not allowed:
  thd->lex->current_select()->m_windows.empty();
  thd->free_items();
  lex_end(thd->lex);
  thd->lex = old_lex;
  thd->stmt_arena = backup_stmt_arena_ptr;
  thd->swap_query_arena(backup_arena, &gcol_arena);
  thd->variables.character_set_client = old_character_set_client;
  thd->want_privilege = save_old_privilege;
  thd->lex->expr_allows_subselect = save_allow_subselects;
  return true;
}

// Unpack partition
bool unpack_partition_info(THD *thd, TABLE *outparam, TABLE_SHARE *share,
                           handlerton *engine_type, bool is_create_table) {
  /*
    Currently we still need to run the parser for extracting
    Item trees (for partition expression and COLUMNS values).
    To avoid too big refactoring in this patch, we still generate
    the syntax when reading the DD (read_from_dd_partitions) and
    parse it for each TABLE instance.
    TODO:
    To avoid multiple copies of information, we should try to
    point to the TABLE_SHARE where possible:
    - partition names etc. I.e. reuse the partition_elements!
    This is not possible with columns partitions, since they use
    Item for storing the values!?
    Also make sure that part_state is never altered without proper locks
    (like MDL exclusive locks on the table! since they would be shared by all
    instances of a table!)
    TODO: Use field images instead?
    TODO: Look on how DEFAULT values will be stored in the new DD
    and reuse that if possible!
    TODO: wl#7840 to get a more light weight parsing of expressions
    Create a new partition_info object on the table's mem_root,
    by parsing a minimalistic string generated from the share.
    And then fill in the missing parts from the part_info on the share.
  */

  /*
    In this execution we must avoid calling thd->change_item_tree since
    we might release memory before statement is completed. We do this
    by changing to a new statement arena. As part of this arena we also
    set the memory root to be the memory root of the table since we
    call the parser and fix_fields which both can allocate memory for
    item objects. We keep the arena to ensure that we can release the
    item list when closing the table object.
    SEE Bug #21658
  */

  // Can use TABLE's mem_root, as it's surely not an internal tmp table
  DBUG_ASSERT(share->table_category != TABLE_CATEGORY_TEMPORARY);

  Query_arena *backup_stmt_arena_ptr = thd->stmt_arena;
  Query_arena backup_arena;
  Query_arena part_func_arena(&outparam->mem_root,
                              Query_arena::STMT_INITIALIZED);
  thd->swap_query_arena(part_func_arena, &backup_arena);
  thd->stmt_arena = &part_func_arena;
  bool tmp;
  bool work_part_info_used;

  tmp = mysql_unpack_partition(
      thd, share->partition_info_str, share->partition_info_str_len, outparam,
      is_create_table, engine_type, &work_part_info_used);
  if (tmp) {
    thd->stmt_arena = backup_stmt_arena_ptr;
    thd->swap_query_arena(backup_arena, &part_func_arena);
    return true;
  }
  outparam->part_info->is_auto_partitioned = share->auto_partitioned;
  DBUG_PRINT("info", ("autopartitioned: %u", share->auto_partitioned));
  /*
    We should perform the fix_partition_func in either local or
    caller's arena depending on work_part_info_used value.
  */
  if (!work_part_info_used)
    tmp = fix_partition_func(thd, outparam, is_create_table);
  thd->stmt_arena = backup_stmt_arena_ptr;
  thd->swap_query_arena(backup_arena, &part_func_arena);
  if (!tmp) {
    if (work_part_info_used)
      tmp = fix_partition_func(thd, outparam, is_create_table);
  }
  outparam->part_info->item_list = part_func_arena.item_list();
  // TODO: Compare with share->part_info for validation of code!
  DBUG_ASSERT(!share->m_part_info || share->m_part_info->column_list ==
                                         outparam->part_info->column_list);
  DBUG_ASSERT(!share->m_part_info ||
              outparam->part_info->list_of_part_fields ==
                  share->m_part_info->list_of_part_fields);

  // part_info->part_expr->table_name and
  // part_info->subpart_expr->table_name will have been set to
  // TABLE::alias (passed on from Field). But part_info cannot refer
  // to TABLE::alias since this may be changed when the table object
  // is reused. Traverse part_expr and subpart_expr and set table_name
  // to nullptr, to avoid dereferencing an invalid pointer.

  // @todo bug#29354690: When part_info handling is refactored and properly
  // attached to a single TABLE object, this extra code can be
  // deleted.
  if (outparam->part_info->part_expr != nullptr) {
    outparam->part_info->part_expr->walk(&Item::set_table_name,
                                         enum_walk::SUBQUERY_POSTFIX, nullptr);
  }
  if (outparam->part_info->subpart_expr != nullptr) {
    outparam->part_info->subpart_expr->walk(
        &Item::set_table_name, enum_walk::SUBQUERY_POSTFIX, nullptr);
  }
  return tmp;
}

/**
  Create a copy of the key_info from TABLE_SHARE object to TABLE object.

  Wherever prefix key is present, allocate a new Field object, having its
  field_length set to the prefix key length, and point the table's matching
  key_part->field to this new Field object.

  This ensures that unpack_partition_info() reads the correct prefix length of
  partitioned fields
*/

bool create_key_part_field_with_prefix_length(TABLE *table, MEM_ROOT *root) {
  DBUG_TRACE;
  TABLE_SHARE *share = table->s;
  KEY *key_info = nullptr;
  KEY_PART_INFO *key_part = nullptr;
  uint n_length;

  DBUG_ASSERT(share->key_parts);

  n_length =
      share->keys * sizeof(KEY) + share->key_parts * sizeof(KEY_PART_INFO);

  // Allocate new memory for table.key_info
  if (!(key_info = static_cast<KEY *>(root->Alloc(n_length)))) return true;

  table->key_info = key_info;
  key_part = (reinterpret_cast<KEY_PART_INFO *>(key_info + share->keys));

  // Copy over the key_info from share to table.
  memcpy(key_info, share->key_info, sizeof(*key_info) * share->keys);
  memcpy(key_part, share->key_info[0].key_part,
         (sizeof(*key_part) * share->key_parts));

  for (KEY *key_info_end = key_info + share->keys; key_info < key_info_end;
       key_info++) {
    key_info->table = table;
    key_info->key_part = key_part;

    for (KEY_PART_INFO *key_part_end = key_part + key_info->actual_key_parts;
         key_part < key_part_end; key_part++) {
      Field *field = key_part->field = table->field[key_part->fieldnr - 1];

      if (field->key_length() != key_part->length &&
          !(field->flags & BLOB_FLAG)) {
        /*
          We are using only a prefix of the column as a key:
          Create a new field for the key part that matches the index
        */
        field = key_part->field = field->new_field(root, table);
        field->set_field_length(key_part->length);
      }
    }

    // Skip unused key parts if they exist
    key_part += key_info->unused_key_parts;
  }

  return false;
}

/**
  Open a table based on a TABLE_SHARE

  @param thd              Thread handler
  @param share            Table definition
  @param alias            Alias for table
  @param db_stat          Open flags (for example HA_OPEN_KEYFILE|
                          HA_OPEN_RNDFILE..) can be 0 (example in
                          ha_example_table)
  @param prgflag          READ_ALL etc..
  @param ha_open_flags    HA_OPEN_ABORT_IF_LOCKED etc..
  @param outparam         Result table.
  @param is_create_table  Indicates that table is opened as part
                          of CREATE or ALTER and does not yet exist in SE.
  @param table_def_param  dd::Table object describing the table to be
                          opened in SE. Can be nullptr, which case this
                          function will try to retrieve such object from
                          the data-dictionary before opening table in SE.

  @retval 0	ok
  @retval 1	Error (see open_table_error)
  @retval 2 Error (see open_table_error)
  @retval 4    Error (see open_table_error)
  @retval 7    Table definition has changed in engine
  @retval 8    Table row format has changed in engine
*/

int open_table_from_share(THD *thd, TABLE_SHARE *share, const char *alias,
                          uint db_stat, uint prgflag, uint ha_open_flags,
                          TABLE *outparam, bool is_create_table,
                          const dd::Table *table_def_param) {
  int error;
  uint records, i, bitmap_size;
  bool error_reported = false;
  bool has_default_values = false;
  const bool internal_tmp = share->table_category == TABLE_CATEGORY_TEMPORARY;
  DBUG_ASSERT(!internal_tmp || share->ref_count() != 0);
  uchar *record, *bitmaps;
  Field **field_ptr;
  Field *fts_doc_id_field = nullptr;
  ptrdiff_t move_offset;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s.%s'  form: %p", share->db.str,
                       share->table_name.str, outparam));

  error = 1;
  new (outparam) TABLE();
  outparam->in_use = thd;
  outparam->s = share;
  outparam->db_stat = db_stat;
  outparam->write_row_record = nullptr;

  MEM_ROOT *root;
  if (!internal_tmp) {
    root = &outparam->mem_root;
    init_sql_alloc(key_memory_TABLE, root, TABLE_ALLOC_BLOCK_SIZE, 0);
  } else
    root = &share->mem_root;

  /*
    For internal temporary tables we allocate the 'alias' in the
    TABLE_SHARE's mem_root rather than on the heap as it gives simpler
    freeing.
  */
  outparam->alias = internal_tmp
                        ? strdup_root(root, alias)
                        : my_strdup(key_memory_TABLE, alias, MYF(MY_WME));
  if (!outparam->alias) goto err;

  outparam->quick_keys.init();
  outparam->possible_quick_keys.init();
  outparam->covering_keys.init();
  outparam->merge_keys.init();
  outparam->keys_in_use_for_query.init();

  /* Allocate handler */
  outparam->file = nullptr;
  if (!(prgflag & SKIP_NEW_HANDLER)) {
    if (!(outparam->file = get_new_handler(share, share->m_part_info != nullptr,
                                           root, share->db_type())))
      goto err;
    if (outparam->file->set_ha_share_ref(&share->ha_share)) goto err;
  } else {
    DBUG_ASSERT(!db_stat);
  }

  error = 4;
  outparam->reginfo.lock_type = TL_UNLOCK;
  outparam->current_lock = F_UNLCK;
  records = 0;
  if ((db_stat & HA_OPEN_KEYFILE) || (prgflag & DELAYED_OPEN)) records = 1;
  if (prgflag & (READ_ALL + EXTRA_RECORD)) records++;

  record = root->ArrayAlloc<uchar>(share->rec_buff_length * records +
                                   share->null_bytes);
  if (record == nullptr) goto err; /* purecov: inspected */

  if (records == 0) {
    /* We are probably in hard repair, and the buffers should not be used */
    outparam->record[0] = outparam->record[1] = share->default_values;
    has_default_values = true;
  } else {
    outparam->record[0] = record;
    if (records > 1)
      outparam->record[1] = record + share->rec_buff_length;
    else
      outparam->record[1] = outparam->record[0];  // Safety
  }
  outparam->null_flags_saved = record + (records * share->rec_buff_length);
  memset(outparam->null_flags_saved, '\0', share->null_bytes);

  if (!(field_ptr = root->ArrayAlloc<Field *>(share->fields + 1)))
    goto err; /* purecov: inspected */

  outparam->field = field_ptr;

  record = (uchar *)outparam->record[0] - 1; /* Fieldstart = 1 */
  outparam->null_flags = (uchar *)record + 1;

  /*
    We will create fields by cloning TABLE_SHARE's fields; then we will need
    to make all new fields' pointers point into the new TABLE's record[0], by
    applying an offset to them.
    Calculate the "source" offset depending on table type:
    - For non-internal temporary tables, source is share->default_values
    - For internal tables, source is first TABLE's record[0], which
    happens to be created in same memory block as share->default_values, with
    offset 2 * share->rec_buff_length (see create_tmp_table()).
  */
  move_offset = outparam->record[0] - share->default_values +
                (internal_tmp ? 2 * share->rec_buff_length : 0);

  /* Setup copy of fields from share, but use the right alias and record */
  for (i = 0; i < share->fields; i++, field_ptr++) {
    Field *new_field = share->field[i]->clone(root);
    *field_ptr = new_field;
    if (new_field == nullptr) goto err;
    new_field->init(outparam);
    new_field->move_field_offset(move_offset);
    /*
       Initialize Field::pack_length() number of bytes for new_field->ptr
       only if there are no default values for the field.
    */
    if (!has_default_values)
      memset(new_field->ptr, 0, new_field->pack_length());
    /* Check if FTS_DOC_ID column is present in the table */
    if (outparam->file &&
        (outparam->file->ha_table_flags() & HA_CAN_FULLTEXT_EXT) &&
        !strcmp(outparam->field[i]->field_name, FTS_DOC_ID_COL_NAME))
      fts_doc_id_field = new_field;
  }
  (*field_ptr) = nullptr;  // End marker

  if (share->found_next_number_field)
    outparam->found_next_number_field =
        outparam->field[(uint)(share->found_next_number_field - share->field)];

  /* Fix key->name and key_part->field */
  if (share->key_parts) {
    if (create_key_part_field_with_prefix_length(outparam, root)) goto err;
    KEY *key_info = outparam->key_info;
    for (KEY *key_info_end = key_info + share->keys; key_info < key_info_end;
         key_info++) {
      /* Set TABLE::fts_doc_id_field for tables with FT KEY */
      if ((key_info->flags & HA_FULLTEXT))
        outparam->fts_doc_id_field = fts_doc_id_field;
    }
  }

  // Parse partition expression and create Items
  if (share->partition_info_str_len && outparam->file &&
      unpack_partition_info(thd, outparam, share,
                            share->m_part_info->default_engine_type,
                            is_create_table)) {
    if (is_create_table) {
      /*
        During CREATE/ALTER TABLE it is ok to receive errors here.
        It is not ok if it happens during the opening of an frm
        file as part of a normal query.
      */
      error_reported = true;
    }
    goto err;
  }

  /* Check generated columns against table's storage engine. */
  if (share->vfields && outparam->file &&
      !(outparam->file->ha_table_flags() & HA_GENERATED_COLUMNS)) {
    my_error(ER_UNSUPPORTED_ACTION_ON_GENERATED_COLUMN, MYF(0),
             "Specified storage engine");
    error_reported = true;
    goto err;
  }

  /*
    Allocate bitmaps
    This needs to be done prior to generated columns as they'll call
    fix_fields and functions might want to access bitmaps.
  */

  bitmap_size = share->column_bitmap_size;
  if (!(bitmaps = root->ArrayAlloc<uchar>(bitmap_size * 8))) goto err;
  bitmap_init(&outparam->def_read_set, (my_bitmap_map *)bitmaps, share->fields);
  bitmap_init(&outparam->def_write_set,
              (my_bitmap_map *)(bitmaps + bitmap_size), share->fields);
  bitmap_init(&outparam->tmp_set, (my_bitmap_map *)(bitmaps + bitmap_size * 2),
              share->fields);
  bitmap_init(&outparam->cond_set, (my_bitmap_map *)(bitmaps + bitmap_size * 3),
              share->fields);
  bitmap_init(&outparam->def_fields_set_during_insert,
              (my_bitmap_map *)(bitmaps + bitmap_size * 4), share->fields);
  bitmap_init(&outparam->fields_for_functional_indexes,
              (my_bitmap_map *)(bitmaps + bitmap_size * 5), share->fields);
  bitmap_init(&outparam->pack_row_tmp_set,
              (my_bitmap_map *)(bitmaps + bitmap_size * 6), share->fields);
  bitmap_init(&outparam->tmp_write_set,
              (my_bitmap_map *)(bitmaps + bitmap_size * 7), share->fields);
  outparam->default_column_bitmaps();

  /*
    Process generated columns, if any.
  */
  outparam->vfield = nullptr;
  if (share->vfields) {
    Field **vfield_ptr = root->ArrayAlloc<Field *>(share->vfields + 1);
    if (!vfield_ptr) goto err;

    outparam->vfield = vfield_ptr;

    for (field_ptr = outparam->field; *field_ptr; field_ptr++) {
      if ((*field_ptr)->gcol_info) {
        if (unpack_value_generator(thd, outparam, &(*field_ptr)->gcol_info,
                                   VGS_GENERATED_COLUMN,
                                   (*field_ptr)->field_name, *field_ptr,
                                   is_create_table, &error_reported)) {
          *vfield_ptr = nullptr;
          error = 4;  // in case no error is reported
          goto err;
        }

        // Mark hidden generated columns for functional indexes.
        if ((*field_ptr)->is_field_for_functional_index()) {
          bitmap_set_bit(&outparam->fields_for_functional_indexes,
                         (*field_ptr)->field_index);
        }
        *(vfield_ptr++) = *field_ptr;
      }
    }
    *vfield_ptr = nullptr;  // End marker
  }

  // Check default value expressions against table's storage engine
  if (share->gen_def_field_count && outparam->file &&
      (!(outparam->file->ha_table_flags() & HA_SUPPORTS_DEFAULT_EXPRESSION))) {
    my_error(ER_UNSUPPORTED_ACTION_ON_DEFAULT_VAL_GENERATED, MYF(0),
             "Specified storage engine");
    error_reported = true;
    goto err;
  }

  // Unpack generated default fields and store reference to this type of fields
  outparam->gen_def_fields_ptr = nullptr;
  if (share->gen_def_field_count) {
    Field **gen_def_field =
        root->ArrayAlloc<Field *>(share->gen_def_field_count + 1);
    if (!gen_def_field) goto err;

    outparam->gen_def_fields_ptr = gen_def_field;
    for (field_ptr = outparam->field; *field_ptr; field_ptr++) {
      if ((*field_ptr)->has_insert_default_general_value_expression()) {
        if (unpack_value_generator(
                thd, outparam, &(*field_ptr)->m_default_val_expr,
                VGS_DEFAULT_EXPRESSION, (*field_ptr)->field_name, *field_ptr,
                is_create_table, &error_reported)) {
          (*field_ptr)->m_default_val_expr = nullptr;
          *gen_def_field = nullptr;
          // In case no error is reported
          error = 4;
          goto err;
        }
        *(gen_def_field++) = *field_ptr;
      }
    }
    *gen_def_field = nullptr;  // End marker
  }

  /*
    Set up table check constraints from the table share and unpack check
    constraint expression.
  */
  if (share->check_constraint_share_list != nullptr) {
    DBUG_ASSERT(share->check_constraint_share_list->size() > 0);

    outparam->table_check_constraint_list =
        new (root) Sql_table_check_constraint_list(root);
    if (outparam->table_check_constraint_list == nullptr) goto err;  // OOM

    if (outparam->table_check_constraint_list->reserve(
            share->check_constraint_share_list->size()))
      goto err;  // OOM

    for (auto &cc_share : *share->check_constraint_share_list) {
      // Unpack check constraint expression.
      Value_generator val_gen;
      val_gen.expr_str = to_lex_string(cc_share.expr_str());
      Value_generator *val_gen_ptr = &val_gen;
      if (unpack_value_generator(thd, outparam, &val_gen_ptr,
                                 VGS_CHECK_CONSTRAINT, cc_share.name().str,
                                 nullptr, is_create_table, &error_reported))
        goto err;

      outparam->table_check_constraint_list->push_back(
          Sql_table_check_constraint(cc_share.name(), cc_share.expr_str(),
                                     cc_share.is_enforced(), val_gen_ptr,
                                     outparam));
    }
  }

  /* The table struct is now initialized;  Open the table */
  error = 2;
  if (db_stat) {
    const dd::Table *table_def = table_def_param;
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

    if (!table_def) {
      if (thd->dd_client()->acquire(share->db.str, share->table_name.str,
                                    &table_def)) {
        error_reported = true;
        goto err;
      }

      if (!table_def) {
        error = 1;
        set_my_errno(ENOENT);
        goto err;
      }
    }

    int ha_err;
    if ((ha_err = (outparam->file->ha_open(
             outparam, share->normalized_path.str,
             (db_stat & HA_READ_ONLY ? O_RDONLY : O_RDWR),
             ((db_stat & HA_OPEN_TEMPORARY
                   ? HA_OPEN_TMP_TABLE
                   : (db_stat & HA_WAIT_IF_LOCKED)
                         ? HA_OPEN_WAIT_IF_LOCKED
                         : (db_stat & (HA_ABORT_IF_LOCKED | HA_GET_INFO))
                               ? HA_OPEN_ABORT_IF_LOCKED
                               : HA_OPEN_IGNORE_IF_LOCKED) |
              ha_open_flags),
             table_def)))) {
      /* Set a flag if the table is crashed and it can be auto. repaired */
      share->crashed = ((ha_err == HA_ERR_CRASHED_ON_USAGE) &&
                        outparam->file->auto_repair() &&
                        !(ha_open_flags & HA_OPEN_FOR_REPAIR));

      switch (ha_err) {
        case HA_ERR_TABLESPACE_MISSING:
          /*
            In case of Innodb table space header may be corrupted or
            ibd file might be missing
          */
          error = 1;
          DBUG_ASSERT(my_errno() == HA_ERR_TABLESPACE_MISSING);
          break;
        case HA_ERR_NO_SUCH_TABLE:
          /*
            The table did not exists in storage engine, use same error message
            as if the .frm file didn't exist
          */
          error = 1;
          set_my_errno(ENOENT);
          break;
        case EMFILE:
          /*
            Too many files opened, use same error message as if the .frm
            file can't open
           */
          DBUG_PRINT("error",
                     ("open file: %s failed, too many files opened (errno: %d)",
                      share->normalized_path.str, ha_err));
          error = 1;
          set_my_errno(EMFILE);
          break;
        default:
          outparam->file->print_error(ha_err, MYF(0));
          error_reported = true;
          if (ha_err == HA_ERR_TABLE_DEF_CHANGED)
            error = 7;
          else if (ha_err == HA_ERR_ROW_FORMAT_CHANGED)
            error = 8;
          break;
      }
      goto err; /* purecov: inspected */
    }
  } else if (outparam->file)  // if db_stat!=0, ha_open() set those pointers:
    outparam->file->change_table_ptr(outparam, share);

  if ((share->table_category == TABLE_CATEGORY_LOG) ||
      (share->table_category == TABLE_CATEGORY_RPL_INFO) ||
      (share->table_category == TABLE_CATEGORY_GTID)) {
    outparam->no_replicate = true;
  } else if (outparam->file) {
    handler::Table_flags flags = outparam->file->ha_table_flags();
    outparam->no_replicate =
        !(flags & (HA_BINLOG_STMT_CAPABLE | HA_BINLOG_ROW_CAPABLE)) ||
        (flags & HA_HAS_OWN_BINLOGGING);
  } else {
    outparam->no_replicate = false;
  }

  /* Increment the opened_tables counter, only when open flags set. */
  if (db_stat) thd->status_var.opened_tables++;

  /* set creation time */
  outparam->set_last_access_time();

  return 0;

err:
  if (!error_reported) open_table_error(thd, share, error, my_errno());
  destroy(outparam->file);
  if (outparam->part_info) free_items(outparam->part_info->item_list);
  if (outparam->vfield) {
    for (Field **vfield = outparam->vfield; *vfield; vfield++)
      free_items((*vfield)->gcol_info->item_list);
  }
  if (outparam->gen_def_fields_ptr) {
    for (Field **gen_def = outparam->gen_def_fields_ptr; *gen_def; gen_def++)
      free_items((*gen_def)->m_default_val_expr->item_list);
  }
  if (outparam->table_check_constraint_list != nullptr) {
    for (auto &table_cc : *outparam->table_check_constraint_list) {
      free_items(table_cc.value_generator()->item_list);
    }
  }
  outparam->file = nullptr;  // For easier error checking
  outparam->db_stat = 0;
  if (!internal_tmp) free_root(root, MYF(0));
  my_free(const_cast<char *>(outparam->alias));
  return error;
}

/**
  Free information allocated by openfrm

  @param table		TABLE object to free
  @param free_share		Is 1 if we also want to free table_share
*/

int closefrm(TABLE *table, bool free_share) {
  int error = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: %p", table));

  if (table->db_stat) error = table->file->ha_close();
  my_free(const_cast<char *>(table->alias));
  table->alias = nullptr;
  if (table->field) {
    for (Field **ptr = table->field; *ptr; ptr++) {
      if ((*ptr)->gcol_info) free_items((*ptr)->gcol_info->item_list);
      if ((*ptr)->m_default_val_expr)
        free_items((*ptr)->m_default_val_expr->item_list);
      destroy(*ptr);
    }
    table->field = nullptr;
  }
  if (table->table_check_constraint_list != nullptr) {
    for (auto &table_cc : *table->table_check_constraint_list) {
      free_items(table_cc.value_generator()->item_list);
    }
  }
  destroy(table->file);
  table->file = nullptr; /* For easier errorchecking */
  if (table->part_info) {
    /* Allocated through table->mem_root, freed below */
    free_items(table->part_info->item_list);
    table->part_info->item_list = nullptr;
    table->part_info = nullptr;
  }
  if (free_share) {
    if (table->s->tmp_table == NO_TMP_TABLE)
      release_table_share(table->s);
    else
      free_table_share(table->s);
  }
  free_root(&table->mem_root, MYF(0));
  return error;
}

/* Deallocate temporary blob storage */

void free_blobs(TABLE *table) {
  uint *ptr, *end;
  for (ptr = table->s->blob_field, end = ptr + table->s->blob_fields;
       ptr != end; ptr++) {
    /*
      Reduced TABLE objects which are used by row-based replication for
      type conversion might have some fields missing. Skip freeing BLOB
      buffers for such missing fields.
    */
    if (table->field[*ptr]) ((Field_blob *)table->field[*ptr])->mem_free();
  }
}

/**
  Reclaims temporary blob storage which is bigger than a threshold.
  Resets blob pointer. Unsets m_keep_old_value.

  @param table A handle to the TABLE object containing blob fields
  @param size The threshold value.
*/

void free_blob_buffers_and_reset(TABLE *table, uint32 size) {
  uint *ptr, *end;
  for (ptr = table->s->blob_field, end = ptr + table->s->blob_fields;
       ptr != end; ptr++) {
    Field_blob *blob = down_cast<Field_blob *>(table->field[*ptr]);
    if (blob->get_field_buffer_size() > size) blob->mem_free();
    blob->reset();

    if (blob->is_virtual_gcol()) blob->set_keep_old_value(false);
  }
}

/* error message when opening a table defintion */

static void open_table_error(THD *thd, TABLE_SHARE *share, int error,
                             int db_errno) {
  int err_no;
  char buff[FN_REFLEN];
  char errbuf[MYSYS_STRERROR_SIZE];
  DBUG_TRACE;

  switch (error) {
    case 8:
    case 7:
    case 1:
      switch (db_errno) {
        case ENOENT:
          my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str,
                   share->table_name.str);
          break;
        case HA_ERR_TABLESPACE_MISSING:
          snprintf(errbuf, MYSYS_STRERROR_SIZE, "`%s`.`%s`", share->db.str,
                   share->table_name.str);
          my_error(ER_TABLESPACE_MISSING, MYF(0), errbuf);
          break;
        default:
          strxmov(buff, share->normalized_path.str, reg_ext, NullS);
          my_error((db_errno == EMFILE) ? ER_CANT_OPEN_FILE : ER_FILE_NOT_FOUND,
                   MYF(0), buff, db_errno,
                   my_strerror(errbuf, sizeof(errbuf), db_errno));
          LogErr(ERROR_LEVEL,
                 (db_errno == EMFILE) ? ER_SERVER_CANT_OPEN_FILE
                                      : ER_SERVER_FILE_NOT_FOUND,
                 buff, db_errno, my_strerror(errbuf, sizeof(errbuf), db_errno));
      }
      break;
    case 2: {
      handler *file = nullptr;
      const char *datext = "";

      if (share->db_type() != nullptr) {
        if ((file = get_new_handler(share, share->m_part_info != nullptr,
                                    thd->mem_root, share->db_type()))) {
          if (!file->ht->file_extensions ||
              !(datext = file->ht->file_extensions[0]))
            datext = "";
        }
      }
      err_no = (db_errno == ENOENT)
                   ? ER_FILE_NOT_FOUND
                   : (db_errno == EAGAIN) ? ER_FILE_USED : ER_CANT_OPEN_FILE;
      strxmov(buff, share->normalized_path.str, datext, NullS);
      my_error(err_no, MYF(0), buff, db_errno,
               my_strerror(errbuf, sizeof(errbuf), db_errno));
      LogErr(ERROR_LEVEL,
             (db_errno == ENOENT)
                 ? ER_SERVER_FILE_NOT_FOUND
                 : (db_errno == EAGAIN) ? ER_SERVER_FILE_USED
                                        : ER_SERVER_CANT_OPEN_FILE,
             buff, db_errno, my_strerror(errbuf, sizeof(errbuf), db_errno));
      destroy(file);
      break;
    }
    default: /* Better wrong error than none */
    case 4:
      strxmov(buff, share->normalized_path.str, reg_ext, NullS);
      my_error(ER_NOT_FORM_FILE, MYF(0), buff);
      LogErr(ERROR_LEVEL, ER_SERVER_NOT_FORM_FILE, buff);
      break;
  }
} /* open_table_error */

/* Check that the integer is in the internal */

int set_zone(int nr, int min_zone, int max_zone) {
  if (nr <= min_zone) return (min_zone);
  if (nr >= max_zone) return (max_zone);
  return (nr);
} /* set_zone */

/**
  Store an SQL quoted string.

  @param res		result String
  @param pos		string to be quoted
  @param length	it's length

  NOTE
    This function works correctly with utf8 or single-byte charset strings.
    May fail with some multibyte charsets though.
*/

void append_unescaped(String *res, const char *pos, size_t length) {
  const char *end = pos + length;

  if (res->reserve(length + 2)) return;

  res->append('\'');

  for (; pos != end; pos++) {
    switch (*pos) {
      case 0: /* Must be escaped for 'mysql' */
        res->append('\\');
        res->append('0');
        break;
      case '\n': /* Must be escaped for logs */
        res->append('\\');
        res->append('n');
        break;
      case '\r':
        res->append('\\'); /* This gives better readability */
        res->append('r');
        break;
      case '\\':
        res->append('\\'); /* Because of the sql syntax */
        res->append('\\');
        break;
      case '\'':
        res->append('\''); /* Because of the sql syntax */
        res->append('\'');
        break;
      default:
        res->append(*pos);
        break;
    }
  }
  res->append('\'');
}

void update_create_info_from_table(HA_CREATE_INFO *create_info, TABLE *table) {
  TABLE_SHARE *share = table->s;
  DBUG_TRACE;

  create_info->max_rows = share->max_rows;
  create_info->min_rows = share->min_rows;
  create_info->table_options = share->db_create_options;
  create_info->avg_row_length = share->avg_row_length;
  create_info->row_type = share->row_type;
  create_info->default_table_charset = share->table_charset;
  create_info->table_charset = nullptr;
  create_info->comment = share->comment;
  create_info->storage_media = share->default_storage_media;
  create_info->tablespace = share->tablespace;
  create_info->compress = share->compress;
  create_info->encrypt_type = share->encrypt_type;
  create_info->secondary_engine = share->secondary_engine;
}

int rename_file_ext(const char *from, const char *to, const char *ext) {
  char from_b[FN_REFLEN], to_b[FN_REFLEN];
  (void)strxmov(from_b, from, ext, NullS);
  (void)strxmov(to_b, to, ext, NullS);
  return (mysql_file_rename(key_file_frm, from_b, to_b, MYF(MY_WME)));
}

/**
  Allocate string field in MEM_ROOT and return it as String

  @param mem   	MEM_ROOT for allocating
  @param field 	Field for retrieving of string
  @param res    result String

  @retval  1   string is empty
  @retval  0	all ok
*/

bool get_field(MEM_ROOT *mem, Field *field, String *res) {
  char buff[MAX_FIELD_WIDTH], *to;
  String str(buff, sizeof(buff), &my_charset_bin);
  size_t length;

  field->val_str(&str);
  if (!(length = str.length())) {
    res->length(0);
    return true;
  }
  if (!(to = strmake_root(mem, str.ptr(), length))) length = 0;  // Safety fix
  res->set(to, length, field->charset());
  return false;
}

/**
  Allocate string field in MEM_ROOT and return it as NULL-terminated string

  @param mem   	MEM_ROOT for allocating
  @param field 	Field for retrieving of string

  @retval  NullS  string is empty
  @retval  other  pointer to NULL-terminated string value of field
*/

char *get_field(MEM_ROOT *mem, Field *field) {
  char buff[MAX_FIELD_WIDTH], *to;
  String str(buff, sizeof(buff), &my_charset_bin);
  size_t length;

  field->val_str(&str);
  length = str.length();
  if (!length || !(to = (char *)mem->Alloc(length + 1))) return NullS;
  memcpy(to, str.ptr(), length);
  to[length] = 0;
  return to;
}

/**
  Check if database name is valid

  @param name             Name of database
  @param length           Length of name

  @retval  Ident_name_check::OK        Identifier name is Ok (Success)
  @retval  Ident_name_check::WRONG     Identifier name is Wrong
                                       (ER_WRONG_TABLE_NAME)
  @retval  Ident_name_check::TOO_LONG  Identifier name is too long if it is
                                       greater than 64 characters
                                       (ER_TOO_LONG_IDENT)

  @note In case of Ident_name_check::WRONG and Ident_name_check::TOO_LONG, this
        function reports an error (my_error)
*/

Ident_name_check check_db_name(const char *name, size_t length) {
  Ident_name_check ident_check_status;

  if (!length || length > NAME_LEN) {
    my_error(ER_WRONG_DB_NAME, MYF(0), name);
    return Ident_name_check::WRONG;
  }

  ident_check_status = check_table_name(name, length);
  if (ident_check_status == Ident_name_check::WRONG)
    my_error(ER_WRONG_DB_NAME, MYF(0), name);
  else if (ident_check_status == Ident_name_check::TOO_LONG)
    my_error(ER_TOO_LONG_IDENT, MYF(0), name);
  return ident_check_status;
}

/**
  Check if database name is valid, and convert to lower case if necessary

  @param org_name             Name of database and length
  @param preserve_lettercase  Preserve lettercase if true

  @note If lower_case_table_names is true and preserve_lettercase
  is false then database is converted to lower case

  @retval  Ident_name_check::OK        Identifier name is Ok (Success)
  @retval  Ident_name_check::WRONG     Identifier name is Wrong
                                       (ER_WRONG_TABLE_NAME)
  @retval  Ident_name_check::TOO_LONG  Identifier name is too long if it is
                                       greater than 64 characters
                                       (ER_TOO_LONG_IDENT)

  @note In case of Ident_name_check::WRONG and Ident_name_check::TOO_LONG, this
        function reports an error (my_error)
*/

Ident_name_check check_and_convert_db_name(LEX_STRING *org_name,
                                           bool preserve_lettercase) {
  char *name = org_name->str;
  size_t name_length = org_name->length;
  Ident_name_check ident_check_status;

  if (!name_length || name_length > NAME_LEN) {
    my_error(ER_WRONG_DB_NAME, MYF(0), org_name->str);
    return Ident_name_check::WRONG;
  }

  if (!preserve_lettercase && lower_case_table_names && name != any_db)
    my_casedn_str(files_charset_info, name);

  ident_check_status = check_table_name(name, name_length);
  if (ident_check_status == Ident_name_check::WRONG)
    my_error(ER_WRONG_DB_NAME, MYF(0), org_name->str);
  else if (ident_check_status == Ident_name_check::TOO_LONG)
    my_error(ER_TOO_LONG_IDENT, MYF(0), org_name->str);
  return ident_check_status;
}

/**
  Function to check if table name is valid or not. If it is invalid,
  return appropriate error in each case to the caller.

  @param name                  Table name
  @param length                Length of table name

  @retval  Ident_name_check::OK        Identifier name is Ok (Success)
  @retval  Ident_name_check::WRONG     Identifier name is Wrong
                                       (ER_WRONG_TABLE_NAME)
  @retval  Ident_name_check::TOO_LONG  Identifier name is too long if it is
                                       greater than 64 characters
                                       (ER_TOO_LONG_IDENT)

  @note Reporting error to the user is the responsiblity of the caller.
*/

Ident_name_check check_table_name(const char *name, size_t length) {
  // name length in symbols
  size_t name_length = 0;
  const char *end = name + length;
  if (!length || length > NAME_LEN) return Ident_name_check::WRONG;
  bool last_char_is_space = false;

  while (name != end) {
    last_char_is_space = my_isspace(system_charset_info, *name);
    if (use_mb(system_charset_info)) {
      int len = my_ismbchar(system_charset_info, name, end);
      if (len) {
        name += len;
        name_length++;
        continue;
      }
    }
    name++;
    name_length++;
  }
  if (last_char_is_space)
    return Ident_name_check::WRONG;
  else if (name_length > NAME_CHAR_LEN)
    return Ident_name_check::TOO_LONG;
  return Ident_name_check::OK;
}

bool check_column_name(const char *name) {
  // name length in symbols
  size_t name_length = 0;
  bool last_char_is_space = true;

  while (*name) {
    last_char_is_space = my_isspace(system_charset_info, *name);
    if (use_mb(system_charset_info)) {
      int len = my_ismbchar(system_charset_info, name,
                            name + system_charset_info->mbmaxlen);
      if (len) {
        name += len;
        name_length++;
        continue;
      }
    }
    if (*name == NAMES_SEP_CHAR) return true;
    name++;
    name_length++;
  }
  /* Error if empty or too long column name */
  return last_char_is_space || (name_length > NAME_CHAR_LEN);
}

/**
  Checks whether a table is intact. Should be done *just* after the table has
  been opened.

  @param[in] table             The table to check
  @param[in] table_def         Expected structure of the table (column name
                               and type)

  @retval  false  OK
  @retval  true   There was an error.
*/

bool Table_check_intact::check(THD *thd MY_ATTRIBUTE((unused)), TABLE *table,
                               const TABLE_FIELD_DEF *table_def) {
  uint i;
  bool error = false;
  const TABLE_FIELD_TYPE *field_def = table_def->field;
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("table: %s  expected_count: %d", table->alias, table_def->count));

  /* Whether the table definition has already been validated. */
  if (table->s->table_field_def_cache == table_def) goto end;

  if (table->s->fields != table_def->count) {
    DBUG_PRINT("info", ("Column count has changed, checking the definition"));

    /* previous MySQL version */
    if (MYSQL_VERSION_ID > table->s->mysql_version) {
      report_error(ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2,
                   ER_THD(thd, ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2),
                   table->s->db.str, table->alias, table_def->count,
                   table->s->fields, static_cast<int>(table->s->mysql_version),
                   MYSQL_VERSION_ID);
      return true;
    } else if (MYSQL_VERSION_ID == table->s->mysql_version) {
      report_error(ER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2,
                   ER_THD(thd, ER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2),
                   table->s->db.str, table->s->table_name.str, table_def->count,
                   table->s->fields);
      return true;
    }
    /*
      Something has definitely changed, but we're running an older
      version of MySQL with new system tables.
      Let's check column definitions. If a column was added at
      the end of the table, then we don't care much since such change
      is backward compatible.
    */
  }
  char buffer[STRING_BUFFER_USUAL_SIZE];
  for (i = 0; i < table_def->count; i++, field_def++) {
    String sql_type(buffer, sizeof(buffer), system_charset_info);
    sql_type.length(0);
    if (i < table->s->fields) {
      Field *field = table->field[i];

      if (strncmp(field->field_name, field_def->name.str,
                  field_def->name.length)) {
        /*
          Name changes are not fatal, we use ordinal numbers to access columns.
          Still this can be a sign of a tampered table, output an error
          to the error log.
        */
        report_error(0,
                     "Incorrect definition of table %s.%s: "
                     "expected column '%s' at position %d, found '%s'.",
                     table->s->db.str, table->alias, field_def->name.str, i,
                     field->field_name);
      }
      field->sql_type(sql_type);
      /*
        Generally, if column types don't match, then something is
        wrong.

        However, we only compare column definitions up to the
        length of the original definition, since we consider the
        following definitions compatible:

        1. DATETIME and DATETIM
        2. INT(11) and INT(11
        3. SET('one', 'two') and SET('one', 'two', 'more')

        For SETs or ENUMs, if the same prefix is there it's OK to
        add more elements - they will get higher ordinal numbers and
        the new table definition is backward compatible with the
        original one.
       */
      if (strncmp(sql_type.c_ptr_safe(), field_def->type.str,
                  field_def->type.length - 1)) {
        report_error(ER_CANNOT_LOAD_FROM_TABLE_V2,
                     "Incorrect definition of "
                     "table %s.%s: expected column '%s' at position %d to "
                     "have type %s, found type %s.",
                     table->s->db.str, table->alias, field_def->name.str, i,
                     field_def->type.str, sql_type.c_ptr_safe());
        error = true;
      } else if (field_def->cset.str && !field->has_charset()) {
        report_error(ER_CANNOT_LOAD_FROM_TABLE_V2,
                     "Incorrect definition of "
                     "table %s.%s: expected the type of column '%s' at "
                     "position %d to have character set '%s' but the type "
                     "has no character set.",
                     table->s->db.str, table->alias, field_def->name.str, i,
                     field_def->cset.str);
        error = true;
      } else if (field_def->cset.str &&
                 strcmp(field->charset()->csname, field_def->cset.str)) {
        report_error(ER_CANNOT_LOAD_FROM_TABLE_V2,
                     "Incorrect definition of "
                     "table %s.%s: expected the type of column '%s' at "
                     "position %d to have character set '%s' but found "
                     "character set '%s'.",
                     table->s->db.str, table->alias, field_def->name.str, i,
                     field_def->cset.str, field->charset()->csname);
        error = true;
      }
    } else {
      report_error(ER_CANNOT_LOAD_FROM_TABLE_V2,
                   "Incorrect definition of "
                   "table %s.%s: expected column '%s' at position %d to "
                   "have type %s but the column is not found.",
                   table->s->db.str, table->alias, field_def->name.str, i,
                   field_def->type.str);
      error = true;
    }
  }

  if (!error) table->s->table_field_def_cache = table_def;

end:

  if (has_keys && !error && !table->key_info) {
    my_error(ER_MISSING_KEY, MYF(0), table->s->db.str,
             table->s->table_name.str);
    error = true;
  }

  return error;
}

/**
  Traverse portion of wait-for graph which is reachable through edge
  represented by this flush ticket in search for deadlocks.

  @retval true  A deadlock is found. A victim is remembered
                by the visitor.
  @retval false Success, no deadlocks.
*/

bool Wait_for_flush::accept_visitor(MDL_wait_for_graph_visitor *gvisitor) {
  return m_share->visit_subgraph(this, gvisitor);
}

uint Wait_for_flush::get_deadlock_weight() const { return m_deadlock_weight; }

/**
  Traverse portion of wait-for graph which is reachable through this
  table share in search for deadlocks.

  @param wait_for_flush Undocumented.
  @param gvisitor        Deadlock detection visitor.

  @retval true  A deadlock is found. A victim is remembered
                by the visitor.
  @retval false No deadlocks, it's OK to begin wait.
*/

bool TABLE_SHARE::visit_subgraph(Wait_for_flush *wait_for_flush,
                                 MDL_wait_for_graph_visitor *gvisitor) {
  TABLE *table;
  MDL_context *src_ctx = wait_for_flush->get_ctx();
  bool result = true;
  bool locked = false;

  /*
    To protect used_tables list from being concurrently modified
    while we are iterating through it we acquire LOCK_open.
    This does not introduce deadlocks in the deadlock detector
    because we won't try to acquire LOCK_open while
    holding a write-lock on MDL_lock::m_rwlock.
  */
  if (gvisitor->m_lock_open_count++ == 0) {
    locked = true;
    table_cache_manager.lock_all_and_tdc();
  }

  Table_cache_iterator tables_it(this);

  /*
    In case of multiple searches running in parallel, avoid going
    over the same loop twice and shortcut the search.
    Do it after taking the lock to weed out unnecessary races.
  */
  if (src_ctx->m_wait.get_status() != MDL_wait::WS_EMPTY) {
    result = false;
    goto end;
  }

  if (gvisitor->enter_node(src_ctx)) goto end;

  while ((table = tables_it++)) {
    if (gvisitor->inspect_edge(&table->in_use->mdl_context)) {
      goto end_leave_node;
    }
  }

  tables_it.rewind();
  while ((table = tables_it++)) {
    if (table->in_use->mdl_context.visit_subgraph(gvisitor)) {
      goto end_leave_node;
    }
  }

  result = false;

end_leave_node:
  gvisitor->leave_node(src_ctx);

end:
  gvisitor->m_lock_open_count--;
  if (locked) {
    DBUG_ASSERT(gvisitor->m_lock_open_count == 0);
    table_cache_manager.unlock_all_and_tdc();
  }

  return result;
}

const histograms::Histogram *TABLE_SHARE::find_histogram(
    uint field_index) const {
  if (m_histograms == nullptr) return nullptr;

  const auto found = m_histograms->find(field_index);
  if (found == m_histograms->end()) return nullptr;

  return found->second;
}

/**
  Wait until the subject share is removed from the table
  definition cache and make sure it's destroyed.

  @note This method may access the share concurrently with another
  thread if the share is in the process of being opened, i.e., that
  m_open_in_progress is true. In this case, close_cached_tables() may
  iterate over elements in the table definition cache, and call this
  method regardless of the share being opened or not. This works anyway
  since a new flush ticket is added below, and LOCK_open ensures
  that the share may not be destroyed by another thread in the time
  between finding this share (having an old version) and adding the flush
  ticket. Thus, after this thread has added the flush ticket, the thread
  opening the table will eventually call free_table_share (as a result of
  releasing the share after using it, or as a result of a failing
  open_table_def()), which will notify the owners of the flush tickets,
  and the last one being notified will actually destroy the share.

  @param thd Session.
  @param abstime         Timeout for waiting as absolute time value.
  @param deadlock_weight Weight of this wait for deadlock detector.

  @pre LOCK_open is write locked, the share is used (has
       non-zero reference count), is marked for flush and
       this connection does not reference the share.
       LOCK_open will be unlocked temporarily during execution.

  @retval false - Success.
  @retval true  - Error (OOM, deadlock, timeout, etc...).
*/

bool TABLE_SHARE::wait_for_old_version(THD *thd, struct timespec *abstime,
                                       uint deadlock_weight) {
  MDL_context *mdl_context = &thd->mdl_context;
  Wait_for_flush ticket(mdl_context, this, deadlock_weight);
  MDL_wait::enum_wait_status wait_status;

  mysql_mutex_assert_owner(&LOCK_open);
  /*
    We should enter this method only when share's version is not
    up to date and the share is referenced. Otherwise our
    thread will never be woken up from wait.
  */
  DBUG_ASSERT(has_old_version() && ref_count() != 0);

  m_flush_tickets.push_front(&ticket);

  mdl_context->m_wait.reset_status();

  mysql_mutex_unlock(&LOCK_open);

  mdl_context->will_wait_for(&ticket);

  mdl_context->find_deadlock();

  DEBUG_SYNC(thd, "flush_complete");

  wait_status = mdl_context->m_wait.timed_wait(thd, abstime, true,
                                               &stage_waiting_for_table_flush);

  mdl_context->done_waiting_for();

  mysql_mutex_lock(&LOCK_open);

  m_flush_tickets.remove(&ticket);

  if (m_flush_tickets.is_empty() && ref_count() == 0) {
    /*
      If our thread was the last one using the share,
      we must destroy it here.
    */
    destroy();
  }

  DEBUG_SYNC(thd, "share_destroyed");

  /*
    In cases when our wait was aborted by KILL statement,
    a deadlock or a timeout, the share might still be referenced,
    so we don't delete it. Note, that we can't determine this
    condition by checking wait_status alone, since, for example,
    a timeout can happen after all references to the table share
    were released, but before the share is removed from the
    cache and we receive the notification. This is why
    we first destroy the share, and then look at
    wait_status.
  */
  switch (wait_status) {
    case MDL_wait::GRANTED:
      return false;
    case MDL_wait::VICTIM:
      my_error(ER_LOCK_DEADLOCK, MYF(0));
      return true;
    case MDL_wait::TIMEOUT:
      my_error(ER_LOCK_WAIT_TIMEOUT, MYF(0),
               timeout_message("table closing", db.str, table_name.str)
                   .c_ptr_safe());
      return true;
    case MDL_wait::KILLED:
      return true;
    default:
      DBUG_ASSERT(0);
      return true;
  }
}

ulonglong TABLE_SHARE::get_table_ref_version() const {
  if (table_category == TABLE_CATEGORY_DICTIONARY ||
      tmp_table == SYSTEM_TMP_TABLE ||
      (is_view && view_object &&
       view_object->type() == dd::enum_table_type::SYSTEM_VIEW))
    return 0;

  return table_map_id.id();
}

Blob_mem_storage::Blob_mem_storage() : truncated_value(false) {
  init_alloc_root(key_memory_blob_mem_storage, &storage,
                  MAX_FIELD_VARCHARLENGTH, 0);
}

Blob_mem_storage::~Blob_mem_storage() { free_root(&storage, MYF(0)); }

/**
  Initialize TABLE instance (newly created, or coming either from table
  cache or THD::temporary_tables list) and prepare it for further use
  during statement execution. Set the 'alias' attribute from the specified
  TABLE_LIST element. Remember the TABLE_LIST element in the
  TABLE::pos_in_table_list member.

  @param thd  Thread context.
  @param tl   TABLE_LIST element.
*/

void TABLE::init(THD *thd, TABLE_LIST *tl) {
#ifndef DBUG_OFF
  if (s->tmp_table == NO_TMP_TABLE) {
    mysql_mutex_lock(&LOCK_open);
    DBUG_ASSERT(s->ref_count() > 0);
    mysql_mutex_unlock(&LOCK_open);
  }
#endif

  if (thd->lex->need_correct_ident())
    alias_name_used =
        my_strcasecmp(table_alias_charset, s->table_name.str, tl->alias);
  /* Fix alias if table name changes. */
  if (strcmp(alias, tl->alias)) {
    size_t length = strlen(tl->alias) + 1;
    alias = static_cast<char *>(my_realloc(
        key_memory_TABLE, const_cast<char *>(alias), length, MYF(MY_WME)));
    memcpy(const_cast<char *>(alias), tl->alias, length);
  }

  const_table = false;
  nullable = false;
  force_index = false;
  force_index_order = false;
  force_index_group = false;
  set_not_started();
  insert_values = nullptr;
  fulltext_searched = false;
  file->ft_handler = nullptr;
  reginfo.impossible_range = false;
  master_had_triggers = false; /* used in RBR Triggers */

  /* Catch wrong handling of the autoinc_field_has_explicit_non_null_value. */
  DBUG_ASSERT(!autoinc_field_has_explicit_non_null_value);
  autoinc_field_has_explicit_non_null_value = false;

  set_pos_in_table_list(tl);

  clear_column_bitmaps();

  DBUG_ASSERT(key_read == 0);
  no_keyread = false;

  /* Tables may be reused in a sub statement. */
  DBUG_ASSERT(!db_stat || !file->ha_extra(HA_EXTRA_IS_ATTACHED_CHILDREN));

  /*
    Do not call refix_value_generator_items() for tables which are not directly
    used by the statement (i.e. used by the substatements of routines or
    triggers to be invoked by the statement).

    Firstly, there will be call to refix_value_generator_items() at the start
    of execution of substatement which directly uses this table anyway.
    Secondly, cleanup of generated column (call to
    cleanup_value_generator_items()) for the table will be done only at the
    end of execution of substatement which uses it. Because of this call to
    refix_value_generator_items() for prelocking
    placeholder will miss corresponding call to cleanup_value_generator_items()
    if substatement which uses the table is not executed for some reason.
  */
  if (!pos_in_table_list->prelocking_placeholder) {
    bool error MY_ATTRIBUTE((unused)) = refix_value_generator_items(thd);
    DBUG_ASSERT(!error);
  }
}

/**
  Initialize table as internal tmp table

  @param thd        thread handle
  @param share      table share
  @param m_root     table's mem root
  @param charset    table's charset
  @param alias_arg  table's alias
  @param fld        table's fields array
  @param blob_fld   buffer for blob field index
  @param is_virtual true <=> it's a virtual tmp table

  @returns
    true  OOM
    false otherwise
*/

bool TABLE::init_tmp_table(THD *thd, TABLE_SHARE *share, MEM_ROOT *m_root,
                           CHARSET_INFO *charset, const char *alias_arg,
                           Field **fld, uint *blob_fld, bool is_virtual) {
  if (!is_virtual) {
    char *name, path[FN_REFLEN];
    DBUG_ASSERT(sizeof(my_thread_id) == 4);
    sprintf(path, "%s%lx_%x_%x", tmp_file_prefix, current_pid, thd->thread_id(),
            thd->tmp_table++);
    fn_format(path, path, mysql_tmpdir, "",
              MY_REPLACE_EXT | MY_UNPACK_FILENAME);
    if (!(name = (char *)m_root->Alloc(strlen(path) + 1))) return true;
    my_stpcpy(name, path);

    init_tmp_table_share(thd, share, "", 0, name, name, m_root);
  }
  s = share;
  in_use = thd;

  share->blob_field = blob_fld;
  share->db_low_byte_first = true;  // True for HEAP and MyISAM
  share->increment_ref_count();
  share->primary_key = MAX_KEY;
  share->visible_indexes.init();
  share->keys_for_keyread.init();
  share->keys_in_use.init();
  share->keys = 0;
  share->field = field = fld;
  share->table_charset = charset;
  set_not_started();
  alias = alias_arg;
  reginfo.lock_type = TL_WRITE; /* Will be updated */
  db_stat = HA_OPEN_KEYFILE + HA_OPEN_RNDFILE;
  copy_blobs = true;
  quick_keys.init();
  possible_quick_keys.init();
  covering_keys.init();
  merge_keys.init();
  keys_in_use_for_query.init();
  keys_in_use_for_group_by.init();
  keys_in_use_for_order_by.init();
#ifndef DBUG_OFF
  set_tmp_table_seq_id(thd->get_tmp_table_seq_id());
#endif
  return false;
}

bool TABLE::refix_value_generator_items(THD *thd) {
  if (vfield) {
    for (Field **vfield_ptr = vfield; *vfield_ptr; vfield_ptr++) {
      Field *vfield = *vfield_ptr;
      DBUG_ASSERT(vfield->gcol_info && vfield->gcol_info->expr_item);
      refix_inner_value_generator_items(thd, vfield->gcol_info, vfield,
                                        vfield->table, VGS_GENERATED_COLUMN,
                                        vfield->field_name);
    }
  }

  if (gen_def_fields_ptr)
    for (Field **gen_def_col = gen_def_fields_ptr; *gen_def_col;
         gen_def_col++) {
      Value_generator *gen_def_expr = (*gen_def_col)->m_default_val_expr;
      DBUG_ASSERT(gen_def_expr && gen_def_expr->expr_item);
      refix_inner_value_generator_items(
          thd, gen_def_expr, (*gen_def_col), (*gen_def_col)->table,
          VGS_DEFAULT_EXPRESSION, (*gen_def_col)->field_name);
    }

  if (table_check_constraint_list != nullptr) {
    for (auto &table_cc : *table_check_constraint_list) {
      Value_generator *cc_expr = table_cc.value_generator();
      DBUG_ASSERT(cc_expr != nullptr && cc_expr->expr_item != nullptr);
      refix_inner_value_generator_items(thd, cc_expr, nullptr, table_cc.table(),
                                        VGS_CHECK_CONSTRAINT,
                                        table_cc.name().str);
    }
  }

  return false;
}

bool TABLE::refix_inner_value_generator_items(THD *thd, Value_generator *g_expr,
                                              Field *field, TABLE *table,
                                              Value_generator_source source,
                                              const char *source_name) {
  if (!g_expr->expr_item->fixed) {
    bool res = false;
    /*
      The call to fix_value_generators_fields() may create new item objects in
      the item tree for the val generation expression. If these are permanent
      changes to the item tree, the new items must have the same life-span
      as the ones created during parsing of the generated expression
      string. We achieve this by temporarily switching to use the TABLE's
      mem_root if the permanent changes to the item tree haven't been
      completed (by checking the status of
      gcol_info->permanent_changes_completed) and this call is not part of
      context analysis (like prepare or show create table).
    */
    Query_arena *backup_stmt_arena_ptr = thd->stmt_arena;
    Query_arena backup_arena;
    Query_arena gcol_arena(&mem_root, Query_arena::STMT_REGULAR_EXECUTION);
    if (!g_expr->permanent_changes_completed &&
        !thd->lex->is_ps_or_view_context_analysis()) {
      thd->swap_query_arena(gcol_arena, &backup_arena);
      thd->stmt_arena = &gcol_arena;
    }

    /*
      Temporarily disable privileges check; already done when first fixed,
      and then based on definer's (owner's) rights: this thread has
      invoker's rights
    */
    ulong sav_want_priv = thd->want_privilege;
    thd->want_privilege = 0;

    if (fix_value_generators_fields(thd, table, g_expr, source, source_name,
                                    field))
      res = true;

    if (!g_expr->permanent_changes_completed &&
        !thd->lex->is_ps_or_view_context_analysis()) {
      // Switch back to the original stmt_arena.
      thd->stmt_arena = backup_stmt_arena_ptr;
      thd->swap_query_arena(backup_arena, &gcol_arena);

      // Append the new items to the original item_free_list.
      Item *item = g_expr->item_list;
      while (item->next_free) item = item->next_free;
      item->next_free = gcol_arena.item_list();

      // Permanent changes to the item_tree are completed.
      g_expr->permanent_changes_completed = true;
    }

    // Restore any privileges check
    thd->want_privilege = sav_want_priv;
    get_fields_in_item_tree = false;

    /* error occurs */
    if (res) return res;
  }
  return false;
}

void TABLE::cleanup_value_generator_items() {
  if (gen_def_fields_ptr)
    for (Field **vfield_ptr = gen_def_fields_ptr; *vfield_ptr; vfield_ptr++)
      cleanup_items((*vfield_ptr)->m_default_val_expr->item_list);

  if (table_check_constraint_list != nullptr) {
    for (auto &table_cc : *table_check_constraint_list)
      cleanup_items(table_cc.value_generator()->item_list);
  }

  if (!has_gcol()) return;

  for (Field **vfield_ptr = vfield; *vfield_ptr; vfield_ptr++)
    cleanup_items((*vfield_ptr)->gcol_info->item_list);
}

/**
  Create Item_field for each column in the table.

  SYNPOSIS
    TABLE::fill_item_list()
      item_list          a pointer to an empty list used to store items

    Create Item_field object for each column in the table and
    initialize it with the corresponding Field. New items are
    created in the current THD memory root.

  @retval 0 success
  @retval 1 out of memory
*/

bool TABLE::fill_item_list(List<Item> *item_list) const {
  /*
    All Item_field's created using a direct pointer to a field
    are fixed in Item_field constructor.
  */
  uint i = 0;
  for (Field **ptr = visible_field_ptr(); *ptr; ptr++, i++) {
    Item_field *item = new Item_field(*ptr);
    if (!item || item_list->push_back(item)) return true;
  }
  return false;
}

/**
  Reset an existing list of Item_field items to point to the
  Fields of this table.

  SYNPOSIS
    TABLE::reset_item_list()
      item_list          a non-empty list with Item_fields

    This is a counterpart of fill_item_list used to redirect
    Item_fields to the fields of a newly created table.
*/

void TABLE::reset_item_list(List<Item> *item_list) const {
  List_iterator_fast<Item> it(*item_list);
  uint i = 0;
  for (Field **ptr = visible_field_ptr(); *ptr; ptr++, i++) {
    Item_field *item_field = (Item_field *)it++;
    DBUG_ASSERT(item_field != nullptr);
    item_field->reset_field(*ptr);
  }
}

/**
  Create a TABLE_LIST object representing a nested join

  @param allocator  Mem root allocator that object is created from.
  @param alias      Name of nested join object
  @param embedding  Pointer to embedding join nest (or NULL if top-most)
  @param belongs_to List of tables this nest belongs to (never NULL).
  @param select     The query block that this join nest belongs within.

  @returns Pointer to created join nest object, or NULL if error.
*/

TABLE_LIST *TABLE_LIST::new_nested_join(
    MEM_ROOT *allocator, const char *alias, TABLE_LIST *embedding,
    mem_root_deque<TABLE_LIST *> *belongs_to, SELECT_LEX *select) {
  DBUG_ASSERT(belongs_to && select);

  TABLE_LIST *const join_nest = new (allocator) TABLE_LIST;
  if (join_nest == nullptr) return nullptr;

  join_nest->nested_join = new (allocator) NESTED_JOIN;
  if (join_nest->nested_join == nullptr) return nullptr;

  join_nest->db = "";
  join_nest->db_length = 0;
  join_nest->table_name = "";
  join_nest->table_name_length = 0;
  join_nest->alias = alias;

  join_nest->embedding = embedding;
  join_nest->join_list = belongs_to;
  join_nest->select_lex = select;
  join_nest->nested_join->first_nested = NO_PLAN_IDX;

  join_nest->nested_join->join_list.clear();

  return join_nest;
}

/**
  Merge tables from a query block into a nested join structure.

  @param select Query block containing tables to be merged into nested join

  @return false if success, true if error
*/

bool TABLE_LIST::merge_underlying_tables(SELECT_LEX *select) {
  DBUG_ASSERT(nested_join->join_list.empty());

  for (TABLE_LIST *tl : select->top_join_list) {
    tl->embedding = this;
    tl->join_list = &nested_join->join_list;
    nested_join->join_list.push_back(tl);
  }

  return false;
}

/**
   Reset a table before starting optimization
*/
void TABLE::reset() {
  const_table = false;
  nullable = false;
  set_not_started();

  force_index = false;
  force_index_order = false;
  force_index_group = false;
  merge_keys.clear_all();
  quick_keys.clear_all();
  covering_keys = s->keys_for_keyread;
  possible_quick_keys.clear_all();
  set_keyread(false);
  no_keyread = false;
  reginfo.not_exists_optimize = false;
  reginfo.impossible_range = false;
  m_record_buffer = Record_buffer{0, 0, nullptr};
  memset(const_key_parts, 0, sizeof(key_part_map) * s->keys);
  insert_values = nullptr;
  autoinc_field_has_explicit_non_null_value = false;

  file->ft_handler = nullptr;

  pos_in_table_list = nullptr;
}

/**
  Merge WHERE condition of view or derived table into outer query.

  If the derived table is on the inner side of an outer join, its WHERE
  condition is merged into the respective join operation's join condition,
  otherwise the WHERE condition is merged with the derived table's
  join condition.

  @param thd    thread handler

  @return false if success, true if error
*/

bool TABLE_LIST::merge_where(THD *thd) {
  DBUG_TRACE;

  DBUG_ASSERT(is_merged());

  Item *const condition = derived_unit()->first_select()->where_cond();

  if (!condition) return false;

  /*
    Save the WHERE condition separately. This is needed because it is already
    resolved, so we need to explicitly update used tables information after
    merging this derived table into the outer query.
  */
  derived_where_cond = condition;

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  /*
    Merge WHERE condition with the join condition of the outer join nest
    and attach it to join nest representing this derived table.
  */
  set_join_cond(and_conds(join_cond(), condition));
  if (!join_cond()) return true; /* purecov: inspected */

  return false;
}

/**
  Create field translation for merged derived table/view.

  @param thd  Thread handle

  @return false if success, true if error.
*/

bool TABLE_LIST::create_field_translation(THD *thd) {
  Item *item;
  SELECT_LEX *select = derived->first_select();
  List_iterator_fast<Item> it(select->item_list);
  uint field_count = 0;

  DBUG_ASSERT(derived->is_prepared());

  DBUG_ASSERT(!field_translation);

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  // Create view fields translation table
  Field_translator *transl = (Field_translator *)thd->stmt_arena->alloc(
      select->item_list.elements * sizeof(Field_translator));
  if (!transl) return true; /* purecov: inspected */

  while ((item = it++)) {
    /*
      Notice that all items keep their nullability here.
      All items are later wrapped within Item_direct_view objects.
      If the view is used on the inner side of an outer join, these
      objects will reflect the correct nullability of the selected
      expressions.
      The name is either explicitely specified in a list of column
      names, or is derived from the name of the expression in the SELECT
      list.
    */
    transl[field_count].name = m_derived_column_names
                                   ? (*m_derived_column_names)[field_count].str
                                   : item->item_name.ptr();
    transl[field_count++].item = item;
  }
  field_translation = transl;
  field_translation_end = transl + field_count;

  return false;
}

/**
  Return merged WHERE clause and join conditions for a view

  @param thd          thread handle
  @param table        table for the VIEW
  @param[out] pcond   Pointer to the built condition (NULL if none)

  This function returns the result of ANDing the WHERE clause and the
  join conditions of the given view.

  @returns  false for success, true for error
*/

static bool merge_join_conditions(THD *thd, TABLE_LIST *table, Item **pcond) {
  DBUG_TRACE;

  *pcond = nullptr;
  DBUG_PRINT("info", ("alias: %s", table->alias));
  if (table->join_cond()) {
    if (!(*pcond = table->join_cond()->copy_andor_structure(thd)))
      return true; /* purecov: inspected */
  }
  if (!table->nested_join) return false;
  for (TABLE_LIST *tbl : table->nested_join->join_list) {
    if (tbl->is_view()) continue;
    Item *cond;
    if (merge_join_conditions(thd, tbl, &cond))
      return true; /* purecov: inspected */
    if (cond && !(*pcond = and_conds(*pcond, cond)))
      return true; /* purecov: inspected */
  }
  return false;
}

/**
  Prepare check option expression of table

  @param thd            thread handler
  @param is_cascaded     True if parent view requests that this view's
  filtering condition be treated as WITH CASCADED CHECK OPTION; this is for
  recursive calls; user code should omit this argument.

  This function builds check option condition for use in regular execution or
  subsequent SP/PS executions.

  This function must be called after the WHERE clause and join condition
  of this and all underlying derived tables/views have been resolved.

  The function will always call itself recursively for all underlying views
  and base tables.

  On first invocation, the check option condition is built bottom-up in
  statement mem_root, and check_option_processed is set true.

  On subsequent executions, check_option_processed is true and no
  expression building is necessary. However, the function needs to assure that
  the expression is resolved by calling fix_fields() on it.

  @returns false if success, true if error
*/

bool TABLE_LIST::prepare_check_option(THD *thd, bool is_cascaded) {
  DBUG_TRACE;
  DBUG_ASSERT(is_view());

  /*
    True if conditions of underlying views should be treated as WITH CASCADED
    CHECK OPTION
  */
  is_cascaded |= (with_check == VIEW_CHECK_CASCADED);

  for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local) {
    if (tbl->is_view() && tbl->prepare_check_option(thd, is_cascaded))
      return true; /* purecov: inspected */
  }

  if (!check_option_processed) {
    Prepared_stmt_arena_holder ps_arena_holder(thd);
    if ((with_check || is_cascaded) &&
        merge_join_conditions(thd, this, &check_option))
      return true; /* purecov: inspected */

    for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local) {
      if (tbl->check_option &&
          !(check_option = and_conds(check_option, tbl->check_option)))
        return true; /* purecov: inspected */
    }

    check_option_processed = true;
  }

  if (check_option && !check_option->fixed) {
    const char *save_where = thd->where;
    thd->where = "check option";
    if (check_option->fix_fields(thd, &check_option) ||
        check_option->check_cols(1))
      return true; /* purecov: inspected */
    thd->where = save_where;
  }

  return false;
}

/**
  Prepare replace filter for a table that is inserted into via a view.

  Used with REPLACE command to filter out rows that should not be deleted.
  Concatenate WHERE clauses from multiple views into one permanent field:
  TABLE::replace_filter.

  Since REPLACE is not possible against a join view, there is no need to
  process join conditions, only WHERE clause is needed. But we still call
  merge_join_conditions() since this is a general function that handles both
  join conditions (if any) and the original WHERE clause.

  @param thd            thread handler

  @returns false if success, true if error
*/

bool TABLE_LIST::prepare_replace_filter(THD *thd) {
  DBUG_TRACE;

  for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local) {
    if (tbl->is_view() && tbl->prepare_replace_filter(thd)) return true;
  }

  if (!replace_filter_processed) {
    Prepared_stmt_arena_holder ps_arena_holder(thd);

    if (merge_join_conditions(thd, this, &replace_filter))
      return true; /* purecov: inspected */
    for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local) {
      if (tbl->replace_filter) {
        if (!(replace_filter = and_conds(replace_filter, tbl->replace_filter)))
          return true;
      }
    }
    replace_filter_processed = true;
  }

  if (replace_filter && !replace_filter->fixed) {
    const char *save_where = thd->where;
    thd->where = "replace filter";
    if (replace_filter->fix_fields(thd, &replace_filter) ||
        replace_filter->check_cols(1))
      return true;
    thd->where = save_where;
  }

  return false;
}

/**
  Cleanup items belonged to view fields translation table
*/

void TABLE_LIST::cleanup_items() {
  if (!field_translation) return;

  for (Field_translator *transl = field_translation;
       transl < field_translation_end; transl++)
    transl->item->walk(&Item::cleanup_processor, enum_walk::POSTFIX, nullptr);
}

/**
  Check CHECK OPTION condition

  @param thd       thread handler

  @retval VIEW_CHECK_OK     OK
  @retval VIEW_CHECK_ERROR  FAILED
  @retval VIEW_CHECK_SKIP   FAILED, but continue
*/

int TABLE_LIST::view_check_option(THD *thd) const {
  if (check_option && check_option->val_int() == 0) {
    const TABLE_LIST *main_view = top_table();
    my_error(ER_VIEW_CHECK_FAILED, MYF(0), main_view->view_db.str,
             main_view->view_name.str);
    if (thd->lex->is_ignore()) return (VIEW_CHECK_SKIP);
    return (VIEW_CHECK_ERROR);
  }
  return (VIEW_CHECK_OK);
}

/**
  Find table in underlying tables by map and check that only this
  table belong to given map.

  @param[out] table_ref reference to found table
                        (must be set to NULL by caller)
  @param      map       bit mask of tables

  @retval false table not found or found only one (table_ref is non-NULL)
  @retval true  found several tables
*/

bool TABLE_LIST::check_single_table(TABLE_LIST **table_ref, table_map map) {
  for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local) {
    if (tbl->is_view_or_derived() && tbl->is_merged()) {
      if (tbl->check_single_table(table_ref, map)) return true;
    } else if (tbl->map() & map) {
      if (*table_ref) return true;

      *table_ref = tbl;
    }
  }
  return false;
}

/**
  Set insert_values buffer

  @param mem_root   memory pool for allocating

  @returns false if success, true if error (out of memory)
*/

bool TABLE_LIST::set_insert_values(MEM_ROOT *mem_root) {
  if (table) {
    DBUG_ASSERT(table->insert_values == nullptr);
    if (!table->insert_values &&
        !(table->insert_values =
              (uchar *)mem_root->Alloc(table->s->rec_buff_length)))
      return true; /* purecov: inspected */
  } else {
    DBUG_ASSERT(view && merge_underlying_list);
    for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local)
      if (tbl->set_insert_values(mem_root))
        return true; /* purecov: inspected */
  }
  return false;
}

/**
  Test if this is a leaf with respect to name resolution.


    A table reference is a leaf with respect to name resolution if
    it is either a leaf node in a nested join tree (table, view,
    schema table, subquery), or an inner node that represents a
    NATURAL/USING join, or a nested join with materialized join
    columns.

  @retval true if a leaf, false otherwise.
*/
bool TABLE_LIST::is_leaf_for_name_resolution() const {
  return (is_view_or_derived() || is_natural_join || is_join_columns_complete ||
          !nested_join);
}

/**
  Retrieve the first (left-most) leaf in a nested join tree with
  respect to name resolution.


    Given that 'this' is a nested table reference, recursively walk
    down the left-most children of 'this' until we reach a leaf
    table reference with respect to name resolution.

    The left-most child of a nested table reference is the last element
    in the list of children because the children are inserted in
    reverse order.

  @retval If 'this' is a nested table reference - the left-most child of
  @retval the tree rooted in 'this',
    else return 'this'
*/

TABLE_LIST *TABLE_LIST::first_leaf_for_name_resolution() {
  TABLE_LIST *cur_table_ref = nullptr;
  NESTED_JOIN *cur_nested_join;

  if (is_leaf_for_name_resolution()) return this;
  DBUG_ASSERT(nested_join);

  for (cur_nested_join = nested_join; cur_nested_join;
       cur_nested_join = cur_table_ref->nested_join) {
    cur_table_ref = cur_nested_join->join_list.front();

    /*
      If the current nested join is a RIGHT JOIN, the operands in
      'join_list' are in reverse order, thus the first operand is
      already at the front of the list. Otherwise the first operand
      is in the end of the list of join operands.
    */
    if (cur_table_ref->outer_join != JOIN_TYPE_RIGHT) {
      cur_table_ref = cur_nested_join->join_list.back();
    }
    if (cur_table_ref->is_leaf_for_name_resolution()) break;
  }
  return cur_table_ref;
}

/**
  Retrieve the last (right-most) leaf in a nested join tree with
  respect to name resolution.


    Given that 'this' is a nested table reference, recursively walk
    down the right-most children of 'this' until we reach a leaf
    table reference with respect to name resolution.

    The right-most child of a nested table reference is the first
    element in the list of children because the children are inserted
    in reverse order.

  @retval - If 'this' is a nested table reference - the right-most child of
  @retval the tree rooted in 'this',
  @retval - else - 'this'
*/

TABLE_LIST *TABLE_LIST::last_leaf_for_name_resolution() {
  TABLE_LIST *cur_table_ref = this;
  NESTED_JOIN *cur_nested_join;

  if (is_leaf_for_name_resolution()) return this;
  DBUG_ASSERT(nested_join);

  for (cur_nested_join = nested_join; cur_nested_join;
       cur_nested_join = cur_table_ref->nested_join) {
    cur_table_ref = cur_nested_join->join_list.front();
    /*
      If the current nested is a RIGHT JOIN, the operands in
      'join_list' are in reverse order, thus the last operand is in the
      end of the list.
    */
    if (cur_table_ref->outer_join == JOIN_TYPE_RIGHT) {
      cur_table_ref = cur_nested_join->join_list.back();
    }
    if (cur_table_ref->is_leaf_for_name_resolution()) break;
  }
  return cur_table_ref;
}

/**
  Load security context information for this view

  @param thd                  thread handler

  @retval false OK
  @retval true Error
*/

bool TABLE_LIST::prepare_view_security_context(THD *thd) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: %s", alias));

  DBUG_ASSERT(!prelocking_placeholder && view);
  if (view_suid) {
    DBUG_PRINT("info", ("This table is suid view => load contest"));
    DBUG_ASSERT(view && view_sctx);
    if (acl_getroot(thd, view_sctx, definer.user.str, definer.host.str,
                    definer.host.str, thd->db().str)) {
      if ((thd->lex->sql_command == SQLCOM_SHOW_CREATE) ||
          (thd->lex->sql_command == SQLCOM_SHOW_FIELDS)) {
        push_warning_printf(thd, Sql_condition::SL_NOTE, ER_NO_SUCH_USER,
                            ER_THD(thd, ER_NO_SUCH_USER), definer.user.str,
                            definer.host.str);
      } else {
        if (thd->security_context()->check_access(SUPER_ACL)) {
          my_error(ER_NO_SUCH_USER, MYF(0), definer.user.str, definer.host.str);

        } else {
          if (thd->password == 2)
            my_error(ER_ACCESS_DENIED_NO_PASSWORD_ERROR, MYF(0),
                     thd->security_context()->priv_user().str,
                     thd->security_context()->priv_host().str);
          else
            my_error(
                ER_ACCESS_DENIED_ERROR, MYF(0),
                thd->security_context()->priv_user().str,
                thd->security_context()->priv_host().str,
                (thd->password ? ER_THD(thd, ER_YES) : ER_THD(thd, ER_NO)));
        }
        return true;
      }
    }
  }
  return false;
}

/**
  Find security context of current view

  @param thd                  thread handler

*/

Security_context *TABLE_LIST::find_view_security_context(THD *thd) {
  Security_context *sctx;
  TABLE_LIST *upper_view = this;
  DBUG_TRACE;

  DBUG_ASSERT(view);
  while (upper_view && !upper_view->view_suid) {
    DBUG_ASSERT(!upper_view->prelocking_placeholder);
    upper_view = upper_view->referencing_view;
  }
  if (upper_view) {
    DBUG_PRINT("info",
               ("Security context of view %s will be used", upper_view->alias));
    sctx = upper_view->view_sctx;
    DBUG_ASSERT(sctx);
  } else {
    DBUG_PRINT("info", ("Current global context will be used"));
    sctx = thd->security_context();
  }
  return sctx;
}

/**
  Prepare security context and load underlying tables priveleges for view

  @param thd                  thread handler

  @retval false OK
  @retval true Error
*/

bool TABLE_LIST::prepare_security(THD *thd) {
  DBUG_TRACE;
  Security_context *save_security_ctx = thd->security_context();

  DBUG_ASSERT(!prelocking_placeholder);
  if (prepare_view_security_context(thd)) return true;
  /* Acl_map was previously checked out by get_aclroot */
  thd->set_security_context(find_view_security_context(thd));
  opt_trace_disable_if_no_security_context_access(thd);
  for (TABLE_LIST *tbl : *view_tables) {
    DBUG_ASSERT(tbl->referencing_view);
    const char *local_db, *local_table_name;
    if (tbl->is_view()) {
      local_db = tbl->view_db.str;
      local_table_name = tbl->view_name.str;
    } else if (tbl->is_derived()) {
      /* Initialize privileges for derived tables */
      tbl->grant.privilege = SELECT_ACL;
      continue;
    } else {
      local_db = tbl->db;
      local_table_name = tbl->get_table_name();
    }
    fill_effective_table_privileges(thd, &tbl->grant, local_db,
                                    local_table_name);
  }
  thd->set_security_context(save_security_ctx);
  return false;
}

Natural_join_column::Natural_join_column(Field_translator *field_param,
                                         TABLE_LIST *tab) {
  DBUG_ASSERT(tab->field_translation);
  view_field = field_param;
  table_field = nullptr;
  table_ref = tab;
  is_common = false;
}

Natural_join_column::Natural_join_column(Item_field *field_param,
                                         TABLE_LIST *tab) {
  DBUG_ASSERT(tab->table == field_param->field->table);
  table_field = field_param;
  /*
    Cache table, to have no resolution problem after natural join nests have
    been changed to ordinary join nests.
  */
  if (tab->cacheable_table) field_param->cached_table = tab;
  view_field = nullptr;
  table_ref = tab;
  is_common = false;
}

const char *Natural_join_column::name() {
  if (view_field) {
    DBUG_ASSERT(table_field == nullptr);
    return view_field->name;
  }

  return table_field->field_name;
}

Item *Natural_join_column::create_item(THD *thd) {
  if (view_field) {
    DBUG_ASSERT(table_field == nullptr);
    SELECT_LEX *select = thd->lex->current_select();
    return create_view_field(thd, table_ref, &view_field->item,
                             view_field->name, &select->context);
  }
  return table_field;
}

Field *Natural_join_column::field() {
  if (view_field) {
    DBUG_ASSERT(table_field == nullptr);
    return nullptr;
  }
  return table_field->field;
}

const char *Natural_join_column::table_name() {
  DBUG_ASSERT(table_ref);
  return table_ref->alias;
}

const char *Natural_join_column::db_name() {
  if (view_field) return table_ref->view_db.str;

  /*
    Test that TABLE_LIST::db is the same as TABLE_SHARE::db to
    ensure consistency. An exception are I_S schema tables, which
    are inconsistent in this respect.
  */
  DBUG_ASSERT(!strcmp(table_ref->db, table_ref->table->s->db.str) ||
              (table_ref->schema_table &&
               is_infoschema_db(table_ref->table->s->db.str,
                                table_ref->table->s->db.length)));
  return table_ref->db;
}

GRANT_INFO *Natural_join_column::grant() { return &table_ref->grant; }

void Field_iterator_view::set(TABLE_LIST *table) {
  DBUG_ASSERT(table->field_translation);
  view = table;
  ptr = table->field_translation;
  array_end = table->field_translation_end;
}

const char *Field_iterator_table::name() { return (*ptr)->field_name; }

Item *Field_iterator_table::create_item(THD *thd) {
  SELECT_LEX *select = thd->lex->current_select();

  Item_field *item = new Item_field(thd, &select->context, *ptr);
  if (!item) return nullptr;
  /*
    This function creates Item-s which don't go through fix_fields(); see same
    code in Item_field::fix_fields().
    */
  if (is_null_on_empty_table(thd, item)) {
    item->maybe_null = true;
    (*ptr)->table->set_nullable();
  }

  return item;
}

const char *Field_iterator_view::name() { return ptr->name; }

Item *Field_iterator_view::create_item(THD *thd) {
  SELECT_LEX *select = thd->lex->current_select();
  return create_view_field(thd, view, &ptr->item, ptr->name, &select->context);
}

static Item *create_view_field(THD *thd, TABLE_LIST *view, Item **field_ref,
                               const char *name,
                               Name_resolution_context *context) {
  Item *field = *field_ref;
  const char *table_name;
  DBUG_TRACE;

  if (view->schema_table_reformed) {
    /*
      Translation table items are always Item_fields and already fixed
      ('mysql_schema_table' function). So we can return directly the
      field. This case happens only for 'show & where' commands.
    */
    DBUG_ASSERT(field && field->fixed);
    return field;
  }

  DBUG_ASSERT(field);
  if (!field->fixed) {
    if (field->fix_fields(thd, field_ref))
      return nullptr; /* purecov: inspected */
    field = *field_ref;
  }

  /*
    Original table name of a field is calculated as follows:
    - For a view or base table, the view or base table name.
    - For a derived table, the base table name.
    - For an expression that is not a simple column reference, an empty string.
  */
  if (view->is_derived()) {
    while (field->type() == Item::REF_ITEM) {
      field = down_cast<Item_ref *>(field)->ref[0];
    }
    if (field->type() == Item::FIELD_ITEM)
      table_name = thd->mem_strdup(down_cast<Item_field *>(field)->table_name);
    else
      table_name = "";
  } else {
    table_name = view->table_name;
  }
  /*
    @note Creating an Item_view_ref object on top of an Item_field
          means that the underlying Item_field object may be shared by
          multiple occurrences of superior fields. This is a vulnerable
          practice, so special precaution must be taken to avoid programming
          mistakes, such as forgetting to mark the use of a field in both
          read_set and write_set (may happen e.g in an UPDATE statement).
  */
  Item *item = new Item_view_ref(context, field_ref, view->alias, table_name,
                                 name, view);
  if (item != nullptr && (*field_ref)->type() == Item::FIELD_ITEM &&
      down_cast<Item_field *>(*field_ref)->table_ref->m_was_scalar_subquery)
    // This logic can be removed after WL#6570
    thd->alias_rollback(field_ref);
  return item;
}

void Field_iterator_natural_join::set(TABLE_LIST *table_ref) {
  DBUG_ASSERT(table_ref->join_columns);
  column_ref_it.init(*(table_ref->join_columns));
  cur_column_ref = column_ref_it++;
}

void Field_iterator_natural_join::next() {
  cur_column_ref = column_ref_it++;
  DBUG_ASSERT(!cur_column_ref || !cur_column_ref->table_field ||
              cur_column_ref->table_ref->table ==
                  cur_column_ref->table_field->field->table);
}

void Field_iterator_table_ref::set_field_iterator() {
  DBUG_TRACE;
  /*
    If the table reference we are iterating over is a natural join, or it is
    an operand of a natural join, and TABLE_LIST::join_columns contains all
    the columns of the join operand, then we pick the columns from
    TABLE_LIST::join_columns, instead of the  orginial container of the
    columns of the join operator.
  */
  if (table_ref->is_join_columns_complete) {
    /* Necesary, but insufficient conditions. */
    DBUG_ASSERT(
        table_ref->is_natural_join || table_ref->nested_join ||
        (table_ref->join_columns &&
         /* This is a merge view. */
         ((table_ref->field_translation &&
           table_ref->join_columns->elements ==
               (ulong)(table_ref->field_translation_end -
                       table_ref->field_translation)) ||
          /* This is stored table or a tmptable view. */
          (!table_ref->field_translation &&
           table_ref->join_columns->elements == table_ref->table->s->fields))));
    field_it = &natural_join_it;
    DBUG_PRINT("info", ("field_it for '%s' is Field_iterator_natural_join",
                        table_ref->alias));
  }
  /* This is a merge view, so use field_translation. */
  else if (table_ref->field_translation) {
    DBUG_ASSERT(table_ref->is_merged());
    field_it = &view_field_it;
    DBUG_PRINT("info",
               ("field_it for '%s' is Field_iterator_view", table_ref->alias));
  }
  /* This is a base table or stored view. */
  else {
    DBUG_ASSERT(table_ref->table || table_ref->is_view());
    field_it = &table_field_it;
    DBUG_PRINT("info",
               ("field_it for '%s' is Field_iterator_table", table_ref->alias));
  }
  field_it->set(table_ref);
}

void Field_iterator_table_ref::set(TABLE_LIST *table) {
  DBUG_ASSERT(table);
  first_leaf = table->first_leaf_for_name_resolution();
  last_leaf = table->last_leaf_for_name_resolution();
  DBUG_ASSERT(first_leaf && last_leaf);
  table_ref = first_leaf;
  set_field_iterator();
}

void Field_iterator_table_ref::next() {
  /* Move to the next field in the current table reference. */
  field_it->next();
  /*
    If all fields of the current table reference are exhausted, move to
    the next leaf table reference.
  */
  if (field_it->end_of_fields() && table_ref != last_leaf) {
    table_ref = table_ref->next_name_resolution_table;
    DBUG_ASSERT(table_ref);
    set_field_iterator();
  }
}

const char *Field_iterator_table_ref::get_table_name() {
  if (table_ref->is_view())
    return table_ref->view_name.str;
  else if (table_ref->is_natural_join)
    return natural_join_it.column_ref()->table_name();

  DBUG_ASSERT(
      table_ref->is_table_function() ||
      !strcmp(table_ref->table_name, table_ref->table->s->table_name.str));
  return table_ref->table_name;
}

const char *Field_iterator_table_ref::get_db_name() {
  if (table_ref->is_view())
    return table_ref->view_db.str;
  else if (table_ref->is_natural_join)
    return natural_join_it.column_ref()->db_name();

  /*
    Test that TABLE_LIST::db is the same as TABLE_SHARE::db to
    ensure consistency. An exception are I_S schema tables, which
    are inconsistent in this respect and any_db (used in the handler
    interface to manage aliases).
  */
  DBUG_ASSERT(!strcmp(table_ref->db, table_ref->table->s->db.str) ||
              table_ref->db == any_db ||
              (table_ref->schema_table &&
               is_infoschema_db(table_ref->table->s->db.str,
                                table_ref->table->s->db.length)));

  return table_ref->db == any_db ? table_ref->table->s->db.str : table_ref->db;
}

GRANT_INFO *Field_iterator_table_ref::grant() {
  if (table_ref->is_natural_join)
    return natural_join_it.column_ref()->grant();
  else
    return &table_ref->grant;
}

/**
  Create new or return existing column reference to a column of a
  natural/using join.

  @param thd Session.
  @param parent_table_ref  the parent table reference over which the
                      iterator is iterating

    Create a new natural join column for the current field of the
    iterator if no such column was created, or return an already
    created natural join column. The former happens for base tables or
    views, and the latter for natural/using joins. If a new field is
    created, then the field is added to 'parent_table_ref' if it is
    given, or to the original table referene of the field if
    parent_table_ref == NULL.

  @note
    This method is designed so that when a Field_iterator_table_ref
    walks through the fields of a table reference, all its fields
    are created and stored as follows:
    - If the table reference being iterated is a stored table, view or
      natural/using join, store all natural join columns in a list
      attached to that table reference.
    - If the table reference being iterated is a nested join that is
      not natural/using join, then do not materialize its result
      fields. This is OK because for such table references
      Field_iterator_table_ref iterates over the fields of the nested
      table references (recursively). In this way we avoid the storage
      of unnecessay copies of result columns of nested joins.

  @retval other Pointer to a column of a natural join (or its operand)
  @retval NULL No memory to allocate the column
*/

Natural_join_column *Field_iterator_table_ref::get_or_create_column_ref(
    THD *thd, TABLE_LIST *parent_table_ref) {
  Natural_join_column *nj_col;
  bool is_created = true;
  uint field_count = 0;
  TABLE_LIST *add_table_ref = parent_table_ref ? parent_table_ref : table_ref;

  if (field_it == &table_field_it) {
    /* The field belongs to a stored table. */
    Field *tmp_field = table_field_it.field();
    Item_field *tmp_item =
        new Item_field(thd, &thd->lex->current_select()->context, tmp_field);
    if (!tmp_item) return nullptr;
    nj_col = new (thd->mem_root) Natural_join_column(tmp_item, table_ref);
    field_count = table_ref->table->s->fields;
  } else if (field_it == &view_field_it) {
    /* The field belongs to a merge view or information schema table. */
    Field_translator *translated_field = view_field_it.field_translator();
    nj_col =
        new (thd->mem_root) Natural_join_column(translated_field, table_ref);
    field_count =
        table_ref->field_translation_end - table_ref->field_translation;
  } else {
    /*
      The field belongs to a NATURAL join, therefore the column reference was
      already created via one of the two constructor calls above. In this case
      we just return the already created column reference.
    */
    DBUG_ASSERT(table_ref->is_join_columns_complete);
    is_created = false;
    nj_col = natural_join_it.column_ref();
    DBUG_ASSERT(nj_col);
  }
  DBUG_ASSERT(!nj_col->table_field ||
              nj_col->table_ref->table == nj_col->table_field->field->table);

  /*
    If the natural join column was just created add it to the list of
    natural join columns of either 'parent_table_ref' or to the table
    reference that directly contains the original field.
  */
  if (is_created) {
    /* Make sure not all columns were materialized. */
    DBUG_ASSERT(!add_table_ref->is_join_columns_complete);
    if (!add_table_ref->join_columns) {
      /* Create a list of natural join columns on demand. */
      if (!(add_table_ref->join_columns =
                new (thd->mem_root) List<Natural_join_column>))
        return nullptr;
      add_table_ref->is_join_columns_complete = false;
    }
    add_table_ref->join_columns->push_back(nj_col);
    /*
      If new fields are added to their original table reference, mark if
      all fields were added. We do it here as the caller has no easy way
      of knowing when to do it.
      If the fields are being added to parent_table_ref, then the caller
      must take care to mark when all fields are created/added.
    */
    if (!parent_table_ref &&
        add_table_ref->join_columns->elements == field_count)
      add_table_ref->is_join_columns_complete = true;
  }

  return nj_col;
}

/**
  Return an existing reference to a column of a natural/using join.


    The method should be called in contexts where it is expected that
    all natural join columns are already created, and that the column
    being retrieved is a Natural_join_column.

  @retval other Pointer to a column of a natural join (or its operand)
  @retval NULL No memory to allocate the column
*/

Natural_join_column *Field_iterator_table_ref::get_natural_column_ref() {
  Natural_join_column *nj_col;

  DBUG_ASSERT(field_it == &natural_join_it);
  /*
    The field belongs to a NATURAL join, therefore the column reference was
    already created via one of the two constructor calls above. In this case
    we just return the already created column reference.
  */
  nj_col = natural_join_it.column_ref();
  DBUG_ASSERT(nj_col &&
              (!nj_col->table_field ||
               nj_col->table_ref->table == nj_col->table_field->field->table));
  return nj_col;
}

/*****************************************************************************
  Functions to handle column usage bitmaps (read_set, write_set etc...)
*****************************************************************************/

/* Reset all columns bitmaps */

void TABLE::clear_column_bitmaps() {
  /*
    Reset column read/write usage. It's identical to:
    bitmap_clear_all(&table->def_read_set);
    bitmap_clear_all(&table->def_write_set);
  */
  memset(def_read_set.bitmap, 0, s->column_bitmap_size * 2);
  column_bitmaps_set(&def_read_set, &def_write_set);

  bitmap_clear_all(&def_fields_set_during_insert);
  fields_set_during_insert = &def_fields_set_during_insert;

  bitmap_clear_all(&tmp_set);
  bitmap_clear_all(&cond_set);

  if (m_partial_update_columns != nullptr)
    bitmap_clear_all(m_partial_update_columns);
}

/**
  Tell handler we are going to call position() and rnd_pos() later.

  This is needed for handlers that uses the primary key to find the
  row. In this case we have to extend the read bitmap with the primary
  key fields.

  @note: Calling this function does not initialize the table for
  reading using rnd_pos(). rnd_init() still has to be called before
  rnd_pos().
*/

void TABLE::prepare_for_position() {
  DBUG_TRACE;

  if ((file->ha_table_flags() & HA_PRIMARY_KEY_REQUIRED_FOR_POSITION) &&
      s->primary_key < MAX_KEY) {
    mark_columns_used_by_index_no_reset(s->primary_key, read_set);
    /* signal change */
    file->column_bitmaps_signal();
  }
}

/**
  Mark column as either read or written (or none) according to mark_used.

  @note If marking a written field, set thd->dup_field if the column is
        already marked.

  @note If TABLE::get_fields_in_item_tree is set, set the flag bit
        GET_FIXED_FIELDS_FLAG for the field.

  @param field    The column to be marked as used
  @param mark      =MARK_COLUMNS_NONE: Only update flag field, if applicable
                   =MARK_COLUMNS_READ: Mark column as read
                   =MARK_COLUMNS_WRITE: Mark column as written
                   =MARK_COLUMNS_TEMP: Mark column as read, used by filesort()
                                       and processing of generated columns
*/

void TABLE::mark_column_used(Field *field, enum enum_mark_columns mark) {
  DBUG_TRACE;

  switch (mark) {
    case MARK_COLUMNS_NONE:
      if (get_fields_in_item_tree) field->flags |= GET_FIXED_FIELDS_FLAG;
      break;

    case MARK_COLUMNS_READ: {
      Key_map part_of_key = field->part_of_key;
      bitmap_set_bit(read_set, field->field_index);

      part_of_key.merge(field->part_of_prefixkey);
      covering_keys.intersect(part_of_key);
      merge_keys.merge(field->part_of_key);
      if (get_fields_in_item_tree) field->flags |= GET_FIXED_FIELDS_FLAG;
      if (field->is_virtual_gcol()) mark_gcol_in_maps(field);
      break;
    }
    case MARK_COLUMNS_WRITE:
      bitmap_set_bit(write_set, field->field_index);
      DBUG_ASSERT(!get_fields_in_item_tree);

      if (field->is_gcol()) mark_gcol_in_maps(field);
      break;

    case MARK_COLUMNS_TEMP:
      bitmap_set_bit(read_set, field->field_index);
      if (field->is_virtual_gcol()) mark_gcol_in_maps(field);
      break;
  }
}

/*
  Mark that only fields from one key is used

  NOTE:
    This changes the bitmap to use the tmp bitmap
    After this, you can't access any other columns in the table until
    bitmaps are reset, for example with TABLE::clear_column_bitmaps().
*/

void TABLE::mark_columns_used_by_index(uint index) {
  MY_BITMAP *bitmap = &tmp_set;
  DBUG_TRACE;

  set_keyread(true);
  bitmap_clear_all(bitmap);
  mark_columns_used_by_index_no_reset(index, bitmap);
  column_bitmaps_set(bitmap, bitmap);
}

/*
  mark columns used by key, but don't reset other fields

  The parameter key_parts is used for controlling how many of the
  key_parts that will be marked in the bitmap. It has the following
  interpretation:

  = 0:                 Use all regular key parts from the key
                       (user_defined_key_parts)
  >= actual_key_parts: Use all regular and extended columns
  < actual_key_parts:  Use this exact number of key parts

  To use all regular key parts, the caller can use the default value (0).
  To use all regular and extended key parts, use UINT_MAX.

  @note The bit map is not cleared by this function. Only bits
  corresponding to a column used by the index will be set. Bits
  representing columns not used by the index will not be changed.

  @param index     index number
  @param bitmap    bitmap to mark
  @param key_parts number of leading key parts to mark. Default is 0.

  @todo consider using actual_key_parts(key_info[index]) instead of
  key_info[index].user_defined_key_parts: if the PK suffix of a secondary
  index is usable it should be marked.
*/

void TABLE::mark_columns_used_by_index_no_reset(uint index, MY_BITMAP *bitmap,
                                                uint key_parts) const {
  // If key_parts has the default value, then include user defined key parts
  if (key_parts == 0)
    key_parts = key_info[index].user_defined_key_parts;
  else if (key_parts > key_info[index].actual_key_parts)
    key_parts = key_info[index].actual_key_parts;

  KEY_PART_INFO *key_part = key_info[index].key_part;
  KEY_PART_INFO *key_part_end = key_part + key_parts;
  for (; key_part != key_part_end; key_part++)
    bitmap_set_bit(bitmap, key_part->fieldnr - 1);
}

/**
  Mark auto-increment fields as used fields in both read and write maps

  @note
    This is needed in insert & update as the auto-increment field is
    always set and sometimes read.
*/

void TABLE::mark_auto_increment_column() {
  DBUG_ASSERT(found_next_number_field);
  /*
    We must set bit in read set as update_auto_increment() is using the
    store() to check overflow of auto_increment values
  */
  bitmap_set_bit(read_set, found_next_number_field->field_index);
  bitmap_set_bit(write_set, found_next_number_field->field_index);
  if (s->next_number_keypart)
    mark_columns_used_by_index_no_reset(s->next_number_index, read_set);
  file->column_bitmaps_signal();
}

/*
  Mark columns needed for doing an delete of a row

  DESCRIPTON
    Some table engines don't have a cursor on the retrieve rows
    so they need either to use the primary key or all columns to
    be able to delete a row.

    If the engine needs this, the function works as follows:
    - If primary key exits, mark the primary key columns to be read.
    - If not, mark all columns to be read

    If the engine has HA_REQUIRES_KEY_COLUMNS_FOR_DELETE, we will
    mark all key columns as 'to-be-read'. This allows the engine to
    loop over the given record to find all keys and doesn't have to
    retrieve the row again.
*/

void TABLE::mark_columns_needed_for_delete(THD *thd) {
  mark_columns_per_binlog_row_image(thd);

  if (triggers && triggers->mark_fields(TRG_EVENT_DELETE)) return;

  if (file->ha_table_flags() & HA_REQUIRES_KEY_COLUMNS_FOR_DELETE) {
    Field **reg_field;
    for (reg_field = field; *reg_field; reg_field++) {
      if ((*reg_field)->flags & PART_KEY_FLAG)
        bitmap_set_bit(read_set, (*reg_field)->field_index);
    }
    file->column_bitmaps_signal();
  }
  if (file->ha_table_flags() & HA_PRIMARY_KEY_REQUIRED_FOR_DELETE) {
    /*
      If the handler has no cursor capabilites we have to read
      either the primary key, the hidden primary key or all columns to
      be able to do an delete
    */
    if (s->primary_key == MAX_KEY) {
      /*
        If in RBR, we have alreay marked the full before image
        in mark_columns_per_binlog_row_image, if not, then use
        the hidden primary key
      */
      if (!(mysql_bin_log.is_open() && in_use &&
            in_use->is_current_stmt_binlog_format_row()))
        file->use_hidden_primary_key();
    } else
      mark_columns_used_by_index_no_reset(s->primary_key, read_set);

    file->column_bitmaps_signal();
  }
  if (vfield) {
    /*
      InnoDB's delete_row may need to log pre-image of the index entries to
      its UNDO log. Thus, indexed virtual generated column must be made ready
      for evaluation.
    */
    mark_generated_columns(true);
  }
}

/**
  @brief
  Mark columns needed for doing an update of a row

  @details
    Some engines needs to have all columns in an update (to be able to
    build a complete row). If this is the case, we mark all not
    updated columns to be read.

    If this is not the case, we do like in the delete case and mark
    if neeed, either the primary key column or all columns to be read.
    (see mark_columns_needed_for_delete() for details)

    If the engine has HA_REQUIRES_KEY_COLUMNS_FOR_DELETE, we will
    mark all USED key columns as 'to-be-read'. This allows the engine to
    loop over the given record to find all changed keys and doesn't have to
    retrieve the row again.

    Unlike other similar methods, it doesn't mark fields used by triggers,
    that is the responsibility of the caller to do, by using
    Table_trigger_dispatcher::mark_used_fields(TRG_EVENT_UPDATE)!

    Note: Marking additional columns as per binlog_row_image requirements will
    influence query execution plan. For example in the case of
    binlog_row_image=FULL the entire read_set and write_set needs to be flagged.
    This will influence update query to think that 'used key is being modified'
    and query will create a temporary table to process the update operation.
    Which will result in performance degradation. Hence callers who don't want
    their query execution to be influenced as per binlog_row_image requirements
    can skip marking binlog specific columns here and they should make an
    explicit call to 'mark_columns_per_binlog_row_image()' function to mark
    binlog_row_image specific columns.
*/

void TABLE::mark_columns_needed_for_update(THD *thd, bool mark_binlog_columns) {
  DBUG_TRACE;
  if (mark_binlog_columns) mark_columns_per_binlog_row_image(thd);
  if (file->ha_table_flags() & HA_REQUIRES_KEY_COLUMNS_FOR_DELETE) {
    /* Mark all used key columns for read */
    Field **reg_field;
    for (reg_field = field; *reg_field; reg_field++) {
      /* Merge keys is all keys that had a column refered to in the query */
      if (merge_keys.is_overlapping((*reg_field)->part_of_key))
        bitmap_set_bit(read_set, (*reg_field)->field_index);
    }
    file->column_bitmaps_signal();
  }

  if (file->ha_table_flags() & HA_PRIMARY_KEY_REQUIRED_FOR_DELETE) {
    /*
      If the handler has no cursor capabilites we have to read either
      the primary key, the hidden primary key or all columns to be
      able to do an update
    */
    if (s->primary_key == MAX_KEY) {
      /*
        If in RBR, we have alreay marked the full before image
        in mark_columns_per_binlog_row_image, if not, then use
        the hidden primary key
      */
      if (!(mysql_bin_log.is_open() && in_use &&
            in_use->is_current_stmt_binlog_format_row()))
        file->use_hidden_primary_key();
    } else
      mark_columns_used_by_index_no_reset(s->primary_key, read_set);

    file->column_bitmaps_signal();
  }
  /* Mark dependent generated columns as writable */
  if (vfield) mark_generated_columns(true);
  /* Mark columns needed for check constraints evaluation */
  if (table_check_constraint_list != nullptr)
    mark_check_constraint_columns(true);
}

/*
  Mark columns according the binlog row image option.

  When logging in RBR, the user can select whether to
  log partial or full rows, depending on the table
  definition, and the value of binlog_row_image.

  Semantics of the binlog_row_image are the following
  (PKE - primary key equivalent, ie, PK fields if PK
  exists, all fields otherwise):

  binlog_row_image= MINIMAL
    - This marks the PKE fields in the read_set
    - This marks all fields where a value was specified
      in the write_set

  binlog_row_image= NOBLOB
    - This marks PKE + all non-blob fields in the read_set
    - This marks all fields where a value was specified
      and all non-blob fields in the write_set

  binlog_row_image= FULL
    - all columns in the read_set
    - all columns in the write_set

  binlog_row_image= COMPLETE
   - all columns in the read_set
   - This marks all fields where a value was specified
     in the write_set

  This marking is done without resetting the original
  bitmaps. This means that we will strip extra fields in
  the read_set at binlogging time (for those cases that
  we only want to log a PK and we needed other fields for
  execution).
 */
void TABLE::mark_columns_per_binlog_row_image(THD *thd, const bool is_insert) {
  DBUG_TRACE;
  DBUG_ASSERT(read_set->bitmap);
  DBUG_ASSERT(write_set->bitmap);

  /**
    If in RBR we may need to mark some extra columns,
    depending on the binlog-row-image command line argument.
   */
  if ((mysql_bin_log.is_open() && in_use &&
       in_use->is_current_stmt_binlog_format_row() &&
       !ha_check_storage_engine_flag(s->db_type(), HTON_NO_BINLOG_ROW_OPT))) {
    /* if there is no PK, then mark all columns for the BI. */
    if (s->primary_key >= MAX_KEY) bitmap_set_all(read_set);

    switch (thd->variables.binlog_row_image) {
      case BINLOG_ROW_IMAGE_FULL:
        if (s->primary_key < MAX_KEY) bitmap_set_all(read_set);
        bitmap_set_all(write_set);
        break;
      case BINLOG_ROW_IMAGE_COMPLETE:
        if (s->primary_key < MAX_KEY) bitmap_set_all(read_set);
        if (is_insert) bitmap_set_all(write_set);
        break;
      case BINLOG_ROW_IMAGE_NOBLOB:
        /* for every field that is not set, mark it unless it is a blob */
        for (Field **ptr = field; *ptr; ptr++) {
          Field *my_field = *ptr;
          /*
            bypass blob fields. These can be set or not set, we don't care.
            Later, at binlogging time, if we don't need them in the before
            image, we will discard them.

            If set in the AI, then the blob is really needed, there is
            nothing we can do about it.
           */
          if ((s->primary_key < MAX_KEY) &&
              ((my_field->flags & PRI_KEY_FLAG) ||
               (my_field->type() != MYSQL_TYPE_BLOB)))
            bitmap_set_bit(read_set, my_field->field_index);

          if (my_field->type() != MYSQL_TYPE_BLOB)
            bitmap_set_bit(write_set, my_field->field_index);
        }
        break;
      case BINLOG_ROW_IMAGE_MINIMAL:
        /* mark the primary key if available in the read_set */
        if (s->primary_key < MAX_KEY)
          mark_columns_used_by_index_no_reset(s->primary_key, read_set);
        break;

      default:
        DBUG_ASSERT(false);
    }
    file->column_bitmaps_signal();
  }
}

/**
  Allocate space for keys, for a materialized derived table.

  @param key_count     Number of keys to allocate.
  @param modify_share  Do modificationts to TABLE_SHARE.

  When modifying TABLE, modifications to TABLE_SHARE are needed, so that both
  objects remain consistent. Even if several TABLEs point to the same
  TABLE_SHARE, those modifications must be done only once (consider for
  example, incremementing TABLE_SHARE::keys).  Should they be done when
  processing the first TABLE, or the second, or? In case this function, when
  updating TABLE, relies on TABLE_SHARE members which are the subject of
  modifications, we follow this rule: do those TABLE_SHARE member
  modifications first: thus, TABLE-modifying code can be identical for all
  TABLEs. So the _first_ TABLE calling this function, only, should pass
  'true': all next ones should not modify the TABLE_SHARE.

  @returns true if error
*/

bool TABLE::alloc_tmp_keys(uint key_count, bool modify_share) {
  const size_t bytes = sizeof(KEY) * key_count;

  if (modify_share) {
    s->max_tmp_keys = key_count;
    /*
      s->keyinfo may pre-exist, if keys have already been added to another
      reference to the same CTE in another query block.
    */
    KEY *old_ki = s->key_info;
    if (!(s->key_info = static_cast<KEY *>(s->mem_root.Alloc(bytes))))
      return true; /* purecov: inspected */
    memset(s->key_info, 0, bytes);
    if (old_ki) memcpy(s->key_info, old_ki, sizeof(KEY) * s->keys);
  }

  // Catch if the caller didn't respect the rule for 'modify_share'
  DBUG_ASSERT(s->max_tmp_keys == key_count);

  KEY *old_ki = key_info;
  if (!(key_info = static_cast<KEY *>(s->mem_root.Alloc(bytes))))
    return true; /* purecov: inspected */
  memset(key_info, 0, bytes);
  if (old_ki) memcpy(key_info, old_ki, sizeof(KEY) * s->keys);

  return false;
}

/**
  @brief Add one key to a materialized derived table.

  @param key_parts      bitmap of fields that take a part in the key.
  @param key_name       name of the key
  @param invisible      If true, set up bitmaps so the key is never used by
                        this TABLE
  @param modify_share   @see alloc_tmp_keys

  @details
  Creates a key for this table from fields which corresponds the bits set to 1
  in the 'key_parts' bitmap. The 'key_name' name is given to the newly created
  key. In the key, columns are in the same order as in the table.
  @see add_derived_key

  @todo somehow manage to create keys in tmp_table_param for unification
        purposes

  @return true OOM error.
  @return false the key was created or ignored (too long key).
*/

bool TABLE::add_tmp_key(Field_map *key_parts, char *key_name, bool invisible,
                        bool modify_share) {
  DBUG_ASSERT(!created && key_parts);

  Field **reg_field;
  uint i;
  bool key_start = true;
  uint field_count = 0;
  uint key_len = 0;

  for (i = 0, reg_field = field; *reg_field; i++, reg_field++) {
    if (key_parts->is_set(i)) {
      KEY_PART_INFO tkp;
      // Ensure that we're not creating a key over a blob field.
      DBUG_ASSERT(!((*reg_field)->flags & BLOB_FLAG));
      /*
        Check if possible key is too long, ignore it if so.
        The reason to use MI_MAX_KEY_LENGTH (myisam's default) is that it is
        smaller than MAX_KEY_LENGTH (heap's default) and it's unknown whether
        myisam or heap will be used for tmp table.
      */
      tkp.init_from_field(*reg_field);
      key_len += tkp.store_length;
      if (key_len > MI_MAX_KEY_LENGTH) {
        return false;
      }
    }
    field_count++;
  }
  const uint key_part_count = key_parts->bits_set();

  // Code above didn't change TABLE; start with changing TABLE_SHARE:
  if (modify_share) {
    s->max_key_length = std::max(s->max_key_length, key_len);
    s->key_parts += key_part_count;
    DBUG_ASSERT(s->keys < s->max_tmp_keys);
    s->keys++;
  }

  const uint keyno = s->keys - 1;
  KEY *cur_key = key_info + keyno;

  cur_key->usable_key_parts = cur_key->user_defined_key_parts = key_part_count;
  cur_key->actual_key_parts = cur_key->user_defined_key_parts;
  cur_key->key_length = key_len;
  cur_key->algorithm = HA_KEY_ALG_BTREE;
  cur_key->name = key_name;
  cur_key->actual_flags = cur_key->flags = HA_GENERATED_KEY;
  cur_key->set_in_memory_estimate(IN_MEMORY_ESTIMATE_UNKNOWN);

  /*
    Allocate storage for the key part array and the two rec_per_key arrays in
    the tables' mem_root.
  */
  const size_t key_buf_size = sizeof(KEY_PART_INFO) * key_part_count;
  ulong *rec_per_key;
  rec_per_key_t *rec_per_key_float;
  uchar *key_buf;
  KEY_PART_INFO *key_part_info;

  if (!multi_alloc_root(&s->mem_root, &key_buf, key_buf_size, &rec_per_key,
                        sizeof(ulong) * key_part_count, &rec_per_key_float,
                        sizeof(rec_per_key_t) * key_part_count, NULL))
    return true; /* purecov: inspected */

  memset(key_buf, 0, key_buf_size);
  cur_key->key_part = key_part_info = (KEY_PART_INFO *)key_buf;
  cur_key->set_rec_per_key_array(rec_per_key, rec_per_key_float);
  cur_key->table = this;

  /* Initialize rec_per_key and rec_per_key_float */
  for (uint kp = 0; kp < key_part_count; ++kp) {
    cur_key->rec_per_key[kp] = 0;
    cur_key->set_records_per_key(kp, REC_PER_KEY_UNKNOWN);
  }

  if (!invisible) {
    if (field_count == key_part_count) covering_keys.set_bit(keyno);
    keys_in_use_for_group_by.set_bit(keyno);
    keys_in_use_for_order_by.set_bit(keyno);
  }

  for (i = 0, reg_field = field; *reg_field; i++, reg_field++) {
    if (!(key_parts->is_set(i))) continue;

    if (key_start) (*reg_field)->key_start.set_bit(keyno);
    key_start = false;
    (*reg_field)->part_of_key.set_bit(keyno);
    (*reg_field)->part_of_sortkey.set_bit(keyno);
    (*reg_field)->flags |= PART_KEY_FLAG;
    key_part_info->init_from_field(*reg_field);
    key_part_info++;
  }

  if (modify_share) {
    /*
      We copy the TABLE's key_info to the TABLE_SHARE's key_info. Some of the
      copied info is constant over all instances of TABLE,
      e.g. s->key_info[keyno].key_part[i].key_part_flag, so can be
      legally accessed from the share. On the other hand, TABLE-specific
      members (rec_per_key, field, etc) of the TABLE's key_info shouldn't be
      accessed from the share.
    */
    KEY &sk = s->key_info[keyno];
    sk = *cur_key;
    sk.table = nullptr;  // catch any illegal access
    sk.set_rec_per_key_array(nullptr, nullptr);
  }

  return false;
}

/**
  For a materialized derived table: informs the share that certain
  not-yet-used keys are going to be used.

  @param k  Used keys
  @returns  New position of first not-yet-used key.
 */
uint TABLE_SHARE::find_first_unused_tmp_key(const Key_map &k) {
  while (first_unused_tmp_key < MAX_INDEXES && k.is_set(first_unused_tmp_key))
    first_unused_tmp_key++;  // locate the first free slot
  return first_unused_tmp_key;
}

/**
  For a materialized derived table: copies a KEY definition from a position to
  the first not-yet-used position (which is lower).

  @param old_idx        source position
  @param modify_share   @see alloc_tmp_keys
*/
void TABLE::copy_tmp_key(int old_idx, bool modify_share) {
  if (modify_share)
    s->key_info[s->first_unused_tmp_key++] = s->key_info[old_idx];
  const int new_idx = s->first_unused_tmp_key - 1;
  DBUG_ASSERT(!created && new_idx < old_idx && old_idx < (int)s->keys);
  key_info[new_idx] = key_info[old_idx];

  for (auto reg_field = field; *reg_field; reg_field++) {
    auto f = *reg_field;
    f->key_start.clear_bit(new_idx);
    if (f->key_start.is_set(old_idx)) f->key_start.set_bit(new_idx);
    f->part_of_key.clear_bit(new_idx);
    if (f->part_of_key.is_set(old_idx)) f->part_of_key.set_bit(new_idx);
    f->part_of_sortkey.clear_bit(new_idx);
    if (f->part_of_sortkey.is_set(old_idx)) f->part_of_sortkey.set_bit(new_idx);
  }
  covering_keys.clear_bit(new_idx);
  if (covering_keys.is_set(old_idx)) covering_keys.set_bit(new_idx);
  keys_in_use_for_group_by.clear_bit(new_idx);
  if (keys_in_use_for_group_by.is_set(old_idx))
    keys_in_use_for_group_by.set_bit(new_idx);
  keys_in_use_for_order_by.clear_bit(new_idx);
  if (keys_in_use_for_order_by.is_set(old_idx))
    keys_in_use_for_order_by.set_bit(new_idx);
}

/**
  For a materialized derived table: after copy_tmp_key() has copied all
  definitions of used KEYs, in TABLE::key_info we have a head of used keys
  followed by a tail of unused keys; this function chops the tail.
  @param modify_share   @see alloc_tmp_keys
*/
void TABLE::drop_unused_tmp_keys(bool modify_share) {
  if (modify_share) {
    DBUG_ASSERT(s->first_unused_tmp_key <= s->keys);
    s->keys = s->first_unused_tmp_key;
    s->key_parts = 0;
    for (uint i = 0; i < s->keys; i++)
      s->key_parts += s->key_info[i].user_defined_key_parts;
    if (s->first_unused_tmp_key == 0) s->key_info = nullptr;
  }
  if (!s->key_info) key_info = nullptr;
  const Key_map keys_to_keep(s->keys);
  for (auto reg_field = field; *reg_field; reg_field++) {
    auto f = *reg_field;
    f->key_start.intersect(keys_to_keep);
    f->part_of_key.intersect(keys_to_keep);
    if (f->part_of_key.is_clear_all()) f->flags &= ~PART_KEY_FLAG;
    f->part_of_sortkey.intersect(keys_to_keep);
  }

  // Eliminate unused keys; make other keys visible
  covering_keys.intersect(keys_to_keep);
  for (uint keyno = 0; keyno < s->keys; keyno++)
    if (key_info[keyno].actual_key_parts == s->fields)
      covering_keys.set_bit(keyno);
  keys_in_use_for_group_by.set_prefix(s->keys);
  keys_in_use_for_order_by.set_prefix(s->keys);
}

void TABLE::set_keyread(bool flag) {
  DBUG_ASSERT(file);
  if (flag && !key_read) {
    key_read = true;
    if (is_created()) file->ha_extra(HA_EXTRA_KEYREAD);
  } else if (!flag && key_read) {
    key_read = false;
    if (is_created()) file->ha_extra(HA_EXTRA_NO_KEYREAD);
  }
}

void TABLE::set_created() {
  if (created) return;
  if (key_read) file->ha_extra(HA_EXTRA_KEYREAD);
  created = true;
}

/*
  Mark columns the handler needs for doing an insert

  For now, this is used to mark fields used by the trigger
  as changed.
*/

void TABLE::mark_columns_needed_for_insert(THD *thd) {
  mark_columns_per_binlog_row_image(thd, true);

  if (found_next_number_field) mark_auto_increment_column();
  /* Mark all generated columns as writable */
  if (vfield) mark_generated_columns(false);
  /* Mark columns needed for check constraints evaluation */
  if (table_check_constraint_list != nullptr)
    mark_check_constraint_columns(false);
}

/*
  @brief Update the write/read_set for generated columns
         when doing update and insert operation.

  @param        is_update  true means the operation is UPDATE.
                           false means it's INSERT.

  @return       void

  @detail

  Prerequisites for INSERT:

  - write_map is filled with all base columns.

  - read_map is filled with base columns and generated columns to be read.
  Otherwise, it is empty. covering_keys and merge_keys are adjusted according
  to read_map.

  Actions for INSERT:

  - Fill write_map with all generated columns.
  Stored columns are needed because their values will be stored.
  Virtual columns are needed because their values must be checked against
  constraints and it might be referenced by latter generated columns.

  - Fill read_map with base columns for all generated columns.
  This has no technical reason, but is required because the function that
  evaluates generated functions asserts that base columns are in the read_map.
  covering_keys and merge_keys are adjusted according to read_map.

  Prerequisites for UPDATE:

  - write_map is filled with base columns to be updated.

  - read_map is filled with base columns and generated columns to be read
  prior to the row update. covering_keys and merge_keys are adjusted
  according to read_map.

  Actions for UPDATE:

  - Fill write_map with generated columns that are dependent on updated base
  columns and all virtual generated columns. Stored columns are needed because
  their values will be stored. Virtual columns are needed because their values
  must be checked against constraints and might be referenced by latter
  generated columns.
*/

void TABLE::mark_generated_columns(bool is_update) {
  Field **vfield_ptr, *tmp_vfield;
  bool bitmap_updated = false;

  if (is_update) {
    MY_BITMAP dependent_fields;
    my_bitmap_map
        bitbuf[bitmap_buffer_size(MAX_FIELDS) / sizeof(my_bitmap_map)];
    bitmap_init(&dependent_fields, bitbuf, s->fields);

    for (vfield_ptr = vfield; *vfield_ptr; vfield_ptr++) {
      tmp_vfield = *vfield_ptr;
      DBUG_ASSERT(tmp_vfield->gcol_info && tmp_vfield->gcol_info->expr_item);

      /*
        We need to evaluate the GC if:
        - it depends on any updated column
        - or it is virtual indexed, for example:
           * UPDATE changes the primary key's value, and the virtual index
           is a secondary index which includes the pk's value
           * the gcol is in a multi-column index, and UPDATE changes another
           column of this index
           * in both cases the entry in the index needs to change, so needs to
           be located first, for that the GC's value is needed.
      */
      if ((!tmp_vfield->stored_in_db && tmp_vfield->m_indexed) ||
          bitmap_is_overlapping(write_set,
                                &tmp_vfield->gcol_info->base_columns_map)) {
        // The GC needs to be updated
        tmp_vfield->table->mark_column_used(tmp_vfield, MARK_COLUMNS_WRITE);
        // In order to update the new value, we have to read the old value
        tmp_vfield->table->mark_column_used(tmp_vfield, MARK_COLUMNS_READ);
        bitmap_updated = true;
      }
    }
  } else  // Insert needs to evaluate all generated columns
  {
    for (vfield_ptr = vfield; *vfield_ptr; vfield_ptr++) {
      tmp_vfield = *vfield_ptr;
      DBUG_ASSERT(tmp_vfield->gcol_info && tmp_vfield->gcol_info->expr_item);
      tmp_vfield->table->mark_column_used(tmp_vfield, MARK_COLUMNS_WRITE);
      bitmap_updated = true;
    }
  }

  if (bitmap_updated) file->column_bitmaps_signal();
}

/*
  Update the read_map with columns needed for check constraint evaluation when
  doing update and insert operations.

  The read_map is filled with the base columns and generated columns to be read
  to evaluate check constraints. Prerequisites for UPDATE is, write_map is
  filled with the base columns to be updated and generated columns that are
  dependent on updated base columns.

  @param        is_update  true means the operation is UPDATE.
                           false means it's INSERT.

  @return       void
*/
void TABLE::mark_check_constraint_columns(bool is_update) {
  DBUG_ASSERT(table_check_constraint_list != nullptr);

  bool bitmap_updated = false;
  for (Sql_table_check_constraint &tbl_cc : *table_check_constraint_list) {
    if (tbl_cc.is_enforced()) {
      /*
        For update operation, check constraint should be evaluated if it is
        dependent on any of the updated column.
      */
      if (is_update &&
          !bitmap_is_overlapping(write_set,
                                 &tbl_cc.value_generator()->base_columns_map))
        continue;

      // Mark all the columns used in the check constraint.
      const MY_BITMAP *columns_map =
          &tbl_cc.value_generator()->base_columns_map;
      for (uint i = bitmap_get_first_set(columns_map); i != MY_BIT_NONE;
           i = bitmap_get_next_set(columns_map, i)) {
        DBUG_ASSERT(i < s->fields);
        mark_column_used(field[i], MARK_COLUMNS_READ);
      }
      bitmap_updated = true;
    }
  }

  if (bitmap_updated) file->column_bitmaps_signal();
}

/**
  Cleanup this table for re-execution.

*/

void TABLE_LIST::reinit_before_use(THD *thd) {
  /*
    Reset old pointers to TABLEs: they are not valid since the tables
    were closed in the end of previous prepare or execute call.
  */
  table = nullptr;

  /*
    Reset table_name and table_name_length for schema table.
    They are not valid as TABLEs were closed in the end of previous prepare
    or execute call.
  */
  if (schema_table_name) {
    table_name = schema_table_name;
    table_name_length = strlen(schema_table_name);
  }

  /* Reset is_schema_table_processed value(needed for I_S tables */
  schema_table_state = NOT_PROCESSED;

  mdl_request.ticket = nullptr;

  if (is_recursive_reference() && select_lex)
    set_derived_unit(select_lex->recursive_dummy_unit);

  /*
    Is this table part of a SECURITY DEFINER VIEW?
  */
  if (!prelocking_placeholder && view && view_suid && view_sctx) {
    /*
      The suid view needs to "login" again at this stage before privilege
      precheck is done. The THD::m_view_ctx list is used to keep track of the
      new authorized security context life time. When the THD is reset or
      destroyed the security context is safely logged out and and any Acl_maps
      returned to the Acl cache.
    */
    prepare_view_security_context(thd);
    thd->m_view_ctx_list.push_back(view_sctx);
  }
}

uint TABLE_LIST::query_block_id() const {
  if (!derived) return 0;
  return derived->first_select()->select_number;
}

uint TABLE_LIST::query_block_id_for_explain() const {
  if (!derived) return 0;
  if (!m_common_table_expr || !m_common_table_expr->tmp_tables.size())
    return derived->first_select()->select_number;
  return m_common_table_expr->tmp_tables[0]
      ->derived_unit()
      ->first_select()
      ->select_number;
}

/**
  Compiles the tagged hints list and fills up the bitmasks.

  @param thd The current session.
  @param tbl the TABLE to operate on.

    The parser collects the index hints for each table in a "tagged list"
    (TABLE_LIST::index_hints). Using the information in this tagged list
    this function sets the members st_table::keys_in_use_for_query,
    st_table::keys_in_use_for_group_by, st_table::keys_in_use_for_order_by,
    st_table::force_index, st_table::force_index_order,
    st_table::force_index_group and st_table::covering_keys.

    Current implementation of the runtime does not allow mixing FORCE INDEX
    and USE INDEX, so this is checked here. Then the FORCE INDEX list
    (if non-empty) is appended to the USE INDEX list and a flag is set.

    Multiple hints of the same kind are processed so that each clause
    is applied to what is computed in the previous clause.
    For example:
        USE INDEX (i1) USE INDEX (i2)
    is equivalent to
        USE INDEX (i1,i2)
    and means "consider only i1 and i2".

    Similarly
        USE INDEX () USE INDEX (i1)
    is equivalent to
        USE INDEX (i1)
    and means "consider only the index i1"

    It is OK to have the same index several times, e.g. "USE INDEX (i1,i1)" is
    not an error.

    Different kind of hints (USE/FORCE/IGNORE) are processed in the following
    order:
      1. All indexes in USE (or FORCE) INDEX are added to the mask.
      2. All IGNORE INDEX

    e.g. "USE INDEX i1, IGNORE INDEX i1, USE INDEX i1" will not use i1 at all
    as if we had "USE INDEX i1, USE INDEX i1, IGNORE INDEX i1".

  @retval false No errors found.
  @retval true Found and reported an error.
*/
bool TABLE_LIST::process_index_hints(const THD *thd, TABLE *tbl) {
  /* initialize the result variables */
  tbl->keys_in_use_for_query = tbl->keys_in_use_for_group_by =
      tbl->keys_in_use_for_order_by = tbl->s->usable_indexes(thd);

  /* index hint list processing */
  if (index_hints) {
    /* Temporary variables used to collect hints of each kind. */
    Key_map index_join[INDEX_HINT_FORCE + 1];
    Key_map index_order[INDEX_HINT_FORCE + 1];
    Key_map index_group[INDEX_HINT_FORCE + 1];
    Index_hint *hint;
    bool have_empty_use_join = false, have_empty_use_order = false,
         have_empty_use_group = false;
    List_iterator<Index_hint> iter(*index_hints);

    /* iterate over the hints list */
    while ((hint = iter++)) {
      uint pos;

      /* process empty USE INDEX () */
      if (hint->type == INDEX_HINT_USE && !hint->key_name.str) {
        if (hint->clause & INDEX_HINT_MASK_JOIN) {
          index_join[hint->type].clear_all();
          have_empty_use_join = true;
        }
        if (hint->clause & INDEX_HINT_MASK_ORDER) {
          index_order[hint->type].clear_all();
          have_empty_use_order = true;
        }
        if (hint->clause & INDEX_HINT_MASK_GROUP) {
          index_group[hint->type].clear_all();
          have_empty_use_group = true;
        }
        continue;
      }

      /*
        Check if an index with the given name exists and get his offset in
        the keys bitmask for the table
      */
      std::string rewritten_index;
      bool is_rewritten = tbl->s->keynames.type_names != nullptr &&
                          lookup_optimizer_force_index_rewrite(
                              to_string(hint->key_name), &rewritten_index);
      LEX_CSTRING index_to_use =
          is_rewritten
              ? LEX_CSTRING{rewritten_index.c_str(), rewritten_index.size()}
              : hint->key_name;
      if (tbl->s->keynames.type_names == nullptr ||
          (pos = find_type(&tbl->s->keynames, index_to_use.str,
                           index_to_use.length, true)) <= 0 ||
          !tbl->s->key_info[pos - 1].is_visible) {
        my_error(ER_KEY_DOES_NOT_EXITS, MYF(0), index_to_use.str, alias);
        return true;
      }

      pos--;

      /* add to the appropriate clause mask */
      if (hint->clause & INDEX_HINT_MASK_JOIN)
        index_join[hint->type].set_bit(pos);
      if (hint->clause & INDEX_HINT_MASK_ORDER)
        index_order[hint->type].set_bit(pos);
      if (hint->clause & INDEX_HINT_MASK_GROUP)
        index_group[hint->type].set_bit(pos);
    }

    /* cannot mix USE INDEX and FORCE INDEX */
    if ((!index_join[INDEX_HINT_FORCE].is_clear_all() ||
         !index_order[INDEX_HINT_FORCE].is_clear_all() ||
         !index_group[INDEX_HINT_FORCE].is_clear_all()) &&
        (!index_join[INDEX_HINT_USE].is_clear_all() || have_empty_use_join ||
         !index_order[INDEX_HINT_USE].is_clear_all() || have_empty_use_order ||
         !index_group[INDEX_HINT_USE].is_clear_all() || have_empty_use_group)) {
      my_error(ER_WRONG_USAGE, MYF(0), index_hint_type_name[INDEX_HINT_USE],
               index_hint_type_name[INDEX_HINT_FORCE]);
      return true;
    }

    /* process FORCE INDEX as USE INDEX with a flag */
    if (!index_order[INDEX_HINT_FORCE].is_clear_all()) {
      tbl->force_index_order = true;
      index_order[INDEX_HINT_USE].merge(index_order[INDEX_HINT_FORCE]);
    }

    if (!index_group[INDEX_HINT_FORCE].is_clear_all()) {
      tbl->force_index_group = true;
      index_group[INDEX_HINT_USE].merge(index_group[INDEX_HINT_FORCE]);
    }

    /*
      TODO: get rid of tbl->force_index (on if any FORCE INDEX is specified) and
      create tbl->force_index_join instead.
      Then use the correct force_index_XX instead of the global one.
    */
    if (!index_join[INDEX_HINT_FORCE].is_clear_all() ||
        tbl->force_index_group || tbl->force_index_order) {
      tbl->force_index = true;
      index_join[INDEX_HINT_USE].merge(index_join[INDEX_HINT_FORCE]);
    }

    /* apply USE INDEX */
    if (!index_join[INDEX_HINT_USE].is_clear_all() || have_empty_use_join)
      tbl->keys_in_use_for_query.intersect(index_join[INDEX_HINT_USE]);
    if (!index_order[INDEX_HINT_USE].is_clear_all() || have_empty_use_order)
      tbl->keys_in_use_for_order_by.intersect(index_order[INDEX_HINT_USE]);
    if (!index_group[INDEX_HINT_USE].is_clear_all() || have_empty_use_group)
      tbl->keys_in_use_for_group_by.intersect(index_group[INDEX_HINT_USE]);

    /* apply IGNORE INDEX */
    tbl->keys_in_use_for_query.subtract(index_join[INDEX_HINT_IGNORE]);
    tbl->keys_in_use_for_order_by.subtract(index_order[INDEX_HINT_IGNORE]);
    tbl->keys_in_use_for_group_by.subtract(index_group[INDEX_HINT_IGNORE]);
  }

  /* make sure covering_keys don't include indexes disabled with a hint */
  tbl->covering_keys.intersect(tbl->keys_in_use_for_query);
  return false;
}

/**
   Helper function which allows to allocate metadata lock request
   objects for all elements of table list.
*/

void init_mdl_requests(TABLE_LIST *table_list) {
  for (; table_list; table_list = table_list->next_global)
    MDL_REQUEST_INIT(&table_list->mdl_request, MDL_key::TABLE, table_list->db,
                     table_list->table_name,
                     mdl_type_for_dml(table_list->lock_descriptor().type),
                     MDL_TRANSACTION);
}

/**
  @returns true if view or derived table is mergeable, based on
  technical constraints.
*/
bool TABLE_LIST::is_mergeable() const {
  if (!is_view_or_derived() || algorithm == VIEW_ALGORITHM_TEMPTABLE)
    return false;
  /*
    If the table's content is non-deterministic and the query references it
    multiple times, merging it has the risk of creating different contents.
  */
  Common_table_expr *cte = common_table_expr();
  if (cte != nullptr && cte->references.size() >= 2 &&
      derived->uncacheable & UNCACHEABLE_RAND)
    return false;
  return derived->is_mergeable();
}

bool TABLE_LIST::materializable_is_const() const {
  DBUG_ASSERT(uses_materialization());
  const SELECT_LEX_UNIT *unit = derived_unit();
  return unit->query_result()->estimated_rowcount <= 1 &&
         (unit->first_select()->active_options() &
          OPTION_NO_SUBQUERY_DURING_OPTIMIZATION) == 0;
}

/**
  Return the number of leaf tables for a merged view.
*/

uint TABLE_LIST::leaf_tables_count() const {
  // Join nests are not permissible, except as merged views
  DBUG_ASSERT(nested_join == nullptr || is_merged());
  if (!is_merged())  // Base table or materialized view
    return 1;

  uint count = 0;
  for (TABLE_LIST *tbl = merge_underlying_list; tbl; tbl = tbl->next_local)
    count += tbl->leaf_tables_count();

  return count;
}

/**
  @brief
  Retrieve number of rows in the table

  @details
  Retrieve number of rows in the table referred by this TABLE_LIST and
  store it in the table's stats.records variable. If this TABLE_LIST refers
  to a materialized derived table/view, then the estimated number of rows of
  the derived table/view is used instead.

  @return 0          ok
  @return non zero   error
*/

int TABLE_LIST::fetch_number_of_rows() {
  int error = 0;
  if (is_table_function()) {
    // FIXME: open question - there's no estimate for table function.
    // return arbitrary, non-zero number;
    table->file->stats.records = PLACEHOLDER_TABLE_ROW_ESTIMATE;
  } else if (uses_materialization()) {
    /*
      @todo: CostModel: This updates the stats.record value to the
      estimated number of records. This number is used when estimating
      the cost of a table scan for a heap table (ie. it helps producing
      a reasonable good cost estimate for heap tables). If the materialized
      table is stored in MyISAM, this number is not used in the cost estimate
      for table scan. The table scan cost for MyISAM thus always becomes
      the estimate for an empty table.
    */
    table->file->stats.records = derived->query_result()->estimated_rowcount;
  } else if (is_recursive_reference()) {
    /*
      Use the estimated row count of all query blocks before this one, as the
      table will contain, at least, the rows produced by those blocks.
    */
    table->file->stats.records =
        std::max(select_lex->master_unit()->query_result()->estimated_rowcount,
                 // Recursive reference is never a const table
                 (ha_rows)PLACEHOLDER_TABLE_ROW_ESTIMATE);
  } else
    error = table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
  return error;
}

/**
  A helper function to add a derived key to the list of possible keys

  @param derived_key_list  list of all possible derived keys
  @param field             referenced field
  @param ref_by_tbl        the table that refers to given field

  @details The possible key to be used for join with table with ref_by_tbl
  table map is extended to include 'field'. If ref_by_tbl == 0 then the key
  that includes all referred fields is extended.

  @note
  Procedure of keys generation for result tables of materialized derived
  tables/views for allowing ref access to them.

  A key is generated for each equi-join pair (derived table, another table).
  Each generated key consists of fields of derived table used in equi-join.
  Example:

    SELECT * FROM (SELECT f1, f2, count(*) FROM t1 GROUP BY f1) tt JOIN
                  t1 ON tt.f1=t1.f3 and tt.f2=t1.f4;

  In this case for the derived table tt one key will be generated. It will
  consist of two parts f1 and f2.
  Example:

    SELECT * FROM (SELECT f1, f2, count(*) FROM t1 GROUP BY f1) tt JOIN
                  t1 ON tt.f1=t1.f3 JOIN
                  t2 ON tt.f2=t2.f4;

  In this case for the derived table tt two keys will be generated.
  One key over f1 field, and another key over f2 field.
  Currently optimizer may choose to use only one such key, thus the second
  one will be dropped after the range optimizer is finished.
  See also JOIN::finalize_derived_keys function.
  Example:

    SELECT * FROM (SELECT f1, f2, count(*) FROM t1 GROUP BY f1) tt JOIN
                  t1 ON tt.f1=a_function(t1.f3);

  In this case for the derived table tt one key will be generated. It will
  consist of one field - f1.
  In all cases beside one-per-table keys one additional key is generated.
  It includes all fields referenced by other tables.

  Implementation is split in three steps:
    gather information on all used fields of derived tables/view and
      store it in lists of possible keys, one per a derived table/view.
    add keys to result tables of derived tables/view using info from above
      lists.
    (...Planner selects best key...)
    drop unused keys from the table.

  The above procedure is implemented in 4 functions:
    TABLE_LIST::update_derived_keys
                          Create/extend list of possible keys for one derived
                          table/view based on given field/used tables info.
                          (Step one)
    JOIN::generate_derived_keys
                          This function is called from update_ref_and_keys
                          when all possible info on keys is gathered and it's
                          safe to add keys - no keys or key parts would be
                          missed.  Walk over list of derived tables/views and
                          call to TABLE_LIST::generate_keys to actually
                          generate keys. (Step two)
    TABLE_LIST::generate_keys
                          Walks over list of possible keys for this derived
                          table/view to add keys to the result table.
                          Calls to TABLE::add_tmp_key to actually add
                          keys (i.e. KEY objects in TABLE::key_info). (Step two)
    TABLE::add_tmp_key    Creates one index description according to given
                          bitmap of used fields. (Step two)
    [ Planner runs and possibly chooses one key, stored in Key_use->key ]
    JOIN::finalize_derived_keys Walk over list of derived tables/views to
                          destroy unused keys. (Step three)

  This design is used for derived tables, views and CTEs. As a CTE
  can be multi-referenced, some points are worth noting:

  1) Definitions

  - let's call the CTE 'X'
  - Key creation/deletion happens in a window between the start of
  update_derived_keys() and the end of finalize_derived_keys().

  2) Key array locking

  - Evaluation of constant subqueries (and thus their optimization)
  may happen either before, inside, or after the window above:
    * an example of "before": WHERE 1=(subq)), due to optimize_cond()
    * an example of "inside": WHERE col<>(subq), as make_join_plan()
  calls estimate_rowcount() which calls the range optimizer for <>, which
  evaluates subq
    * an example of "after": WHERE key_col=(subq), due to
  create_ref_for_key().
  - let's say that a being-optimized query block 'QB1' is entering that
  window; other query blocks are QB2, etc; let's say (subq) above is QB2, a
  subquery of QB1.
  - While QB1 is in this window, it is possible, as we saw above, that QB2
  gets optimized. Because it is not safe to have two query blocks
  reading/writing possible keys for a same table at the same time, a locking
  mechanism is in place: TABLE_SHARE::owner_of_possible_tmp_keys is a record
  of which query block entered first the window for this table and hasn't left
  it yet; only that query block is allowed to read/write possible keys for
  this table.

  3) Key array growth

  - let's say that a being-optimized query block 'QB1' is entering the
  window; other query blocks are QB2 (not necessarily the same QB2 as in
  previous paragraph), etc.
  - let's call "local" the references to X in QB1, let's call "nonlocal" the
  ones in other query blocks. For example,
  with X(n) as (select 1)
  select /+ QB_NAME(QB2) *_/ n from X as X2
  where X2.n = (select /+* QB_NAME(QB1) *_/ X1.n from X as X1)
  union
  select n+2 from X as X3;
  QB1 owns the window, then X1 is local, X2 and X3 are nonlocal.
  - when QB1 enters the window, update_derived_keys() starts for the local
  reference X1, other references to X may already have keys,
  defined by previously optimized query blocks on their
  references (e.g. QB2 on X2). At that stage the TABLE_SHARE::key_info array is
  of size TABLE_SHARE::keys, and the TABLE_SHARE::first_unused_tmp_key member
  points to 'where any new key should be added in this array', so it's equal
  to TABLE_SHARE::keys. Let's call the keys defined by QB2 the "existing
  keys": they exist at this point and will continue to do so. X2 in QB2 is
  already set up to read with such key. Here's the key_info array, with cell 0
  to the left, "E" meaning "an existing key, created by previous
  optimizations", "-" meaning "an empty cell created by alloc_keys()".

  EEEEEEEEEE-----------
            ^ s->first_unused_keys
            ^ s->keys

  - generate_keys() extends the key_info array and adds "possible" keys to the
  end. "Possible" is defined as "not yet existing", "might be dropped in the
  end". Even if a possible key is a duplicate of an existing key, it is
  added. TABLE_SHARE::keys is increased to include existing and possible
  keys. All TABLEs referencing X, local or not, are kept in sync (i.e. any
  possible key is added to all key_info arrays). But possible keys are set to
  be unusable by nonlocal references, so that the decision to drop those keys
  can be left to the window's owner. Key_info array now is ("P" means
  "possible key"):

  EEEEEEEEEEPPPPPPP---
            ^ s->first_unused_keys
                   ^ s->keys

  - All possible keys are unused, at this stage.
  - Planner selects the best key for each local reference, among existing and
  possible keys, it is recorded in Key_use.
  - finalize_derived_keys() looks at local references, and gathers the list
  of (existing and possible) keys which the Planner has chosen for them. We
  call this list the list of locally-used keys, marked below with "!":

      !       !  !
  EEEEEEEEEEPPPPPPP---
            ^ s->first_unused_keys
                   ^ s->keys

  - Any possible key which isn't locally-used is unnecessary.

  - finalize_derived_keys() re-organizes the possible locally-used keys and
  unnecessary keys, and does needed updates to TABLEs' bitmaps.

      !     !!
  EEEEEEEEEEPPPPPPP---
              ^ s->first_unused_keys
                   ^ s->keys

  The locally-used keys become existing keys and are made visible to nonlocal
  references. The unnecessary keys are chopped.
      !     !!
  EEEEEEEEEEEE-----
              ^ s->first_unused_keys
              ^ s->keys

  - After that, another query block can be optimized.
  - So, query block after query block, optimization phases grow the key_info
  array.
  - If a reference is considered constant in a query block and the Optimizer
  decides to evaluate it, this triggers materialization (creation in engine),
  which freezes the key definition: other query blocks will not be allowed to
  add keys.

  @return true  OOM
  @return false otherwise
*/

static bool add_derived_key(List<Derived_key> &derived_key_list, Field *field,
                            table_map ref_by_tbl) {
  uint key = 0;
  Derived_key *entry = nullptr;
  List_iterator<Derived_key> ki(derived_key_list);

  /* Search for already existing possible key. */
  while ((entry = ki++)) {
    key++;
    if (ref_by_tbl) {
      /* Search for the entry for the specified table.*/
      if (entry->referenced_by & ref_by_tbl) break;
    } else {
      /*
        Search for the special entry that should contain fields referred
        from any table.
      */
      if (!entry->referenced_by) break;
    }
  }
  /* Add new possible key if nothing is found. */
  if (!entry) {
    THD *thd = field->table->in_use;
    key++;
    entry = new (thd->mem_root) Derived_key();
    if (!entry) return true;
    entry->referenced_by = ref_by_tbl;
    entry->used_fields.clear_all();
    if (derived_key_list.push_back(entry, thd->mem_root)) return true;
  }
  /* Don't create keys longer than REF access can use. */
  if (entry->used_fields.bits_set() < MAX_REF_PARTS) {
    field->part_of_key.set_bit(key - 1);
    field->flags |= PART_KEY_FLAG;
    entry->used_fields.set_bit(field->field_index);
  }
  return false;
}

/*
  @brief
  Update derived table's list of possible keys

  @param thd        session context
  @param field      derived table's field to take part in a key
  @param values     array of values. Each value combined with "field"
                    forms an equality predicate.
  @param num_values number of elements in the array values
  @param[out] allocated true if key was allocated, false if unsupported

  @details
  This function creates/extends a list of possible keys for this derived
  table/view. For each table used by a value from the 'values' array the
  corresponding possible key is extended to include the 'field'.
  If there is no such possible key, then it is created. field's
  part_of_key bitmaps are updated accordingly.
  @see add_derived_key

  @returns false if success, true if error
*/

bool TABLE_LIST::update_derived_keys(THD *thd, Field *field, Item **values,
                                     uint num_values, bool *allocated) {
  *allocated = false;
  /*
    Don't bother with keys for CREATE VIEW, BLOB fields and fields with
    zero length.
  */
  if (thd->lex->is_ps_or_view_context_analysis() || field->flags & BLOB_FLAG ||
      field->field_length == 0)
    return false;

  const Sql_cmd *const cmd = thd->lex->m_sql_cmd;

  // Secondary storage engines do not support use of indexes on derived tables
  if (cmd != nullptr && cmd->using_secondary_storage_engine()) return false;

  /* Allow all keys to be used. */
  if (derived_key_list.elements == 0) table->keys_in_use_for_query.set_all();

  for (uint i = 0; i < num_values; i++) {
    table_map tables = values[i]->used_tables() & ~PSEUDO_TABLE_BITS;
    if (!tables || values[i]->real_item()->type() != Item::FIELD_ITEM) continue;
    for (table_map tbl = 1; tables >= tbl; tbl <<= 1) {
      if (!(tables & tbl)) continue;
      if (add_derived_key(derived_key_list, field, tbl)) return true;
    }
  }
  /* Extend key which includes all referenced fields. */
  if (add_derived_key(derived_key_list, field, (table_map)0)) return true;
  *allocated = true;

  return false;
}

/*
  Comparison function for Derived_key entries.
  See TABLE_LIST::generate_keys.
*/

static int Derived_key_comp(Derived_key *e1, Derived_key *e2) {
  /* Move entries for tables with greater table bit to the end. */
  return ((e1->referenced_by < e2->referenced_by)
              ? -1
              : ((e1->referenced_by > e2->referenced_by) ? 1 : 0));
}

/**
  @brief
  Generate keys for a materialized derived table/view.
  @details
  This function adds keys to the result table by walking over the list of
  possible keys for this derived table/view and calling the
  TABLE::add_tmp_key to actually add keys. A name @<auto_keyN@>, where N is a
  sequential number, is given to each key to ease debugging.
  @see add_derived_key

  @return true  an error occur.
  @return false all keys were successfully added.
*/

bool TABLE_LIST::generate_keys() {
  DBUG_ASSERT(uses_materialization());

  if (!derived_key_list.elements) return false;

  Derived_refs_iterator ref_it(this);
  while (TABLE *t = ref_it.get_next())
    if (t->is_created()) {
      /*
        The table may have been instantiated already, by another query
        block. Consider:
        with qn as (...) select * from qn where a=(select * from qn)
                         union select * from qn where b=3;
        Then the scalar subquery is non-correlated, and cache-able, so the
        optimization phase of the first UNION member evaluates this subquery,
        which instantiates qn, then this phase may want to add an index on 'a'
        (for 'a=') but it's too late. Or the upcoming optimization phase for
        the second UNION member may want to add an index on 'b'.
       */
      return false;
    }

  if (table->s->owner_of_possible_tmp_keys != nullptr &&
      table->s->owner_of_possible_tmp_keys != select_lex)
    return false;

  // Extend the key array of every reference

  const int new_key_count =
      std::min(table->s->keys + derived_key_list.elements, MAX_INDEXES);
  ref_it.rewind();
  while (TABLE *t = ref_it.get_next())
    if (t->alloc_tmp_keys(new_key_count, ref_it.is_first()))
      return true; /* purecov: inspected */

  /* Sort entries to make key numbers sequence deterministic. */
  derived_key_list.sort(Derived_key_comp);

  List_iterator<Derived_key> it(derived_key_list);
  Derived_key *entry;
  char buf[NAME_CHAR_LEN];

  while ((entry = it++)) {
    if (table->s->keys == MAX_INDEXES)
      break;  // Impossible to create more keys.
    sprintf(buf, "<auto_key%d>", table->s->keys);
    char *name_buf = table->in_use->mem_strdup(buf);
    ref_it.rewind();
    while (TABLE *t = ref_it.get_next()) {
      if (t->add_tmp_key(&entry->used_fields, name_buf,
                         t->pos_in_table_list->select_lex != select_lex,
                         ref_it.is_first()))
        return true; /* purecov: inspected */
    }
  }

  if (table->s->keys)
    table->s->owner_of_possible_tmp_keys = select_lex;  // Acquire lock

  return false;
}

/**
  Update TABLE::const_key_parts for single table UPDATE/DELETE query

  @param conds               WHERE clause expression

  @retval true   error (OOM)
  @retval false  success

  @note
    Set const_key_parts bits if key fields are equal to constants in
    the WHERE expression.
*/

bool TABLE::update_const_key_parts(Item *conds) {
  memset(const_key_parts, 0, sizeof(key_part_map) * s->keys);

  if (conds == nullptr) return false;

  for (uint index = 0; index < s->keys; index++) {
    KEY_PART_INFO *keyinfo = key_info[index].key_part;
    KEY_PART_INFO *keyinfo_end =
        keyinfo + key_info[index].user_defined_key_parts;

    for (key_part_map part_map = (key_part_map)1; keyinfo < keyinfo_end;
         keyinfo++, part_map <<= 1) {
      if (const_expression_in_where(conds, nullptr, keyinfo->field))
        const_key_parts[index] |= part_map;
    }
  }

  /*
    Handle error for the whole function here instead of along with the call for
    const_expression_in_where() as the function does not return true for errors.
  */
  return this->in_use && this->in_use->is_error();
}

/**
  Read removal is possible if the selected quick read
  method is using full unique index

  @see HA_READ_BEFORE_WRITE_REMOVAL

  @param index              Number of the index used for read

  @retval true   success, read removal started
  @retval false  read removal not started
*/

bool TABLE::check_read_removal(uint index) {
  bool retval = false;

  DBUG_TRACE;
  DBUG_ASSERT(file->ha_table_flags() & HA_READ_BEFORE_WRITE_REMOVAL);
  DBUG_ASSERT(index != MAX_KEY);

  // Index must be unique
  if ((key_info[index].flags & HA_NOSAME) == 0) return false;

  // Full index must be used
  bitmap_clear_all(&tmp_set);
  mark_columns_used_by_index_no_reset(index, &tmp_set);

  if (bitmap_cmp(&tmp_set, read_set)) {
    // Start read removal in handler
    retval = file->start_read_removal();
  }

  bitmap_clear_all(&tmp_set);
  return retval;
}

/**
  Test if the order list consists of simple field expressions

  @param order                Linked list of ORDER BY arguments

  @return true if @a order is empty or consist of simple field expressions
*/

bool is_simple_order(ORDER *order) {
  for (ORDER *ord = order; ord; ord = ord->next) {
    if (ord->item[0]->real_item()->type() != Item::FIELD_ITEM) return false;
  }
  return true;
}

/**
  Repoint a table's fields from old_rec to new_rec

  @param table     the table of fields needed to be repointed
  @param old_rec   the original record buffer fields point to
  @param new_rec   the target record buff fields need to repoint
*/

void repoint_field_to_record(TABLE *table, uchar *old_rec, uchar *new_rec) {
  Field **fields = table->field;
  ptrdiff_t ptrdiff = new_rec - old_rec;
  for (uint i = 0; i < table->s->fields; i++)
    fields[i]->move_field_offset(ptrdiff);
}

/**
  Evaluate necessary virtual generated columns.
  This is used right after reading a row from the storage engine.

  @note this is not necessary for stored generated columns, as they are
  provided by the storage engine.

  @param [in,out] buf    the buffer to store data
  @param table           the TABLE object
  @param active_index    the number of key for index scan (MAX_KEY is default)

  @return true if error.

  @todo see below for potential conflict with Bug#21815348 .
 */
bool update_generated_read_fields(uchar *buf, TABLE *table, uint active_index) {
  DBUG_TRACE;
  DBUG_ASSERT(table && table->vfield);
  if (table->in_use->is_error()) return true;
  if (active_index != MAX_KEY && table->key_read) {
    /*
      The covering index is providing all necessary columns, including
      generated ones.
      Note that this logic may have to be reconsidered when we fix
      Bug#21815348; indeed, for that bug it could be possible to implement the
      following optimization: if A is an indexed base column, and B is a
      virtual generated column dependent on A, "select B from t" could choose
      an index-only scan over the index of A and calculate values of B on the
      fly. In that case, we would come here, however calculation of B would
      still be needed.
      Currently MySQL doesn't choose an index scan in that case because it
      considers B as independent from A, in its index-scan decision logic.
    */
    return false;
  }

  int error = 0;

  /*
    If the buffer storing the record data is not record[0], then the field
    objects must be temporarily changed to point into the supplied buffer.
    The field pointers are restored at the end of this function.
  */
  if (buf != table->record[0])
    repoint_field_to_record(table, table->record[0], buf);

  for (Field **vfield_ptr = table->vfield; *vfield_ptr; vfield_ptr++) {
    Field *vfield = *vfield_ptr;
    DBUG_ASSERT(vfield->gcol_info && vfield->gcol_info->expr_item);
    /*
      Only calculate those virtual generated fields that are marked in the
      read_set bitmap.
    */
    if (vfield->is_virtual_gcol() &&
        bitmap_is_set(table->read_set, vfield->field_index)) {
      if (vfield->handle_old_value()) {
        (down_cast<Field_blob *>(vfield))->keep_old_value();
        (down_cast<Field_blob *>(vfield))->set_keep_old_value(true);
      }

      error = vfield->gcol_info->expr_item->save_in_field(vfield, false);
      DBUG_PRINT("info", ("field '%s' - updated", vfield->field_name));
      if (error && !table->in_use->is_error()) {
        /*
          Most likely a calculation error which only triggered a warning, so
          let's not make the read fail.
        */
        error = 0;
      }
    } else {
      DBUG_PRINT("info", ("field '%s' - skipped", vfield->field_name));
    }
  }

  if (buf != table->record[0])
    repoint_field_to_record(table, buf, table->record[0]);

  return error != 0;
  /*
    @todo
    this function is used by ha_rnd/etc, those ha_* functions are expected to
    return 0 or a HA_ERR code (and such codes are picked up by
    handler::print_error), but update_generated_read_fields returns true/false
    (0/1), which is then returned by the ha_* functions. If it
    returns 1 we get:
    ERROR 1030 (HY000): Got error 1 from storage engine
    which isn't informative for the user.
  */
}

/**
  Calculate data for each generated field marked for write in the
  corresponding column map.

  @note We need calculate data for both virtual and stored generated
  fields.

  @param bitmap         Bitmap over fields to update
  @param table          the TABLE object

  @retval false  Success
  @retval true   Error occurred during the generation/calculation of a generated
                 field value
 */
bool update_generated_write_fields(const MY_BITMAP *bitmap, TABLE *table) {
  DBUG_TRACE;
  Field **field_ptr;
  int error = 0;

  if (table->in_use->is_error()) return true;

  if (table->vfield) {
    /* Iterate over generated fields in the table */
    for (field_ptr = table->vfield; *field_ptr; field_ptr++) {
      Field *vfield;
      vfield = (*field_ptr);
      DBUG_ASSERT(vfield->gcol_info && vfield->gcol_info->expr_item);

      /* Only update those fields that are marked in the bitmap */
      if (bitmap_is_set(bitmap, vfield->field_index)) {
        /*
          For a virtual generated column of blob type, we have to keep
          the current blob value since this might be needed by the
          storage engine during updates.
          All arrays are BLOB fields.
        */
        if (vfield->handle_old_value()) {
          (down_cast<Field_blob *>(vfield))->keep_old_value();
          (down_cast<Field_blob *>(vfield))->set_keep_old_value(true);
        }

        /* Generate the actual value of the generated fields */
        error = vfield->gcol_info->expr_item->save_in_field(vfield, false);

        DBUG_PRINT("info", ("field '%s' - updated", vfield->field_name));
        if (error && !table->in_use->is_error()) error = 0;
        if (table->fields_set_during_insert)
          bitmap_set_bit(table->fields_set_during_insert, vfield->field_index);
      } else {
        DBUG_PRINT("info", ("field '%s' - skipped", vfield->field_name));
      }
    }
  }

  if (error > 0) return true;
  return false;
}

/**
  Adds a generated column and its dependencies to the read_set/write_set
  bitmaps.

  If the value of a generated column (gcol) must be calculated, it needs to
  be in write_set (to satisfy the assertion in Field::store); the value of
  its underlying base columns is necessary to the calculation so those must
  be in read_set.

  A gcol must be calculated in two cases:
  - we're sending the gcol to the engine
  - the gcol is virtual and we're reading it from the engine without using a
  covering index on it.
*/
void TABLE::mark_gcol_in_maps(const Field *field) {
  bitmap_set_bit(write_set, field->field_index);

  /*
    Typed array fields internally are using a conversion field, it needs to
    marked as readable in order to do conversions.
  */
  if (field->is_array()) bitmap_set_bit(read_set, field->field_index);

  /*
    Note that underlying base columns are here added to read_set but not added
    to requirements for an index to be covering (covering_keys is not touched).
    So, if we have:
    SELECT gcol FROM t :
    - an index covering gcol only (not including base columns), can still be
    chosen by the optimizer; note that InnoDB's build_template_needs_field()
    properly ignores read_set when MySQL asks for "index only" reads
    (table->key_read == true); if it didn't, it would do useless reads.
    - but if gcol is not read from an index, we will read base columns because
    they are in read_set.
    - Note how this relies on InnoDB's behaviour.
  */
  for (uint i = 0; i < s->fields; i++) {
    if (bitmap_is_set(&field->gcol_info->base_columns_map, i)) {
      bitmap_set_bit(read_set, i);
      if (this->field[i]->is_virtual_gcol()) bitmap_set_bit(write_set, i);
    }
  }
}

void TABLE::column_bitmaps_set(MY_BITMAP *read_set_arg,
                               MY_BITMAP *write_set_arg) {
  read_set = read_set_arg;
  write_set = write_set_arg;
  if (file && created) file->column_bitmaps_signal();
}

bool TABLE_LIST::set_recursive_reference() {
  if (select_lex->recursive_reference != nullptr) return true;
  select_lex->recursive_reference = this;
  m_is_recursive_reference = true;
  return false;
}

/**
  Propagate table map of a table up by nested join tree. Used to check
  dependencies for LATERAL JOIN of table functions.
  simplify_joins() calculates the same information but also does
  transformations, and we need this semantic check to be earlier than
  simplify_joins() and before transformations.

  @param map_arg  table map to propagate
*/

void TABLE_LIST::propagate_table_maps(table_map map_arg) {
  table_map prop_map;
  if (nested_join) {
    nested_join->used_tables |= map_arg;
    prop_map = nested_join->used_tables;
  } else
    prop_map = map();
  if (embedding) embedding->propagate_table_maps(prop_map);
}

LEX_USER *LEX_USER::alloc(THD *thd, LEX_STRING *user_arg,
                          LEX_STRING *host_arg) {
  LEX_USER *ret = static_cast<LEX_USER *>(thd->alloc(sizeof(LEX_USER)));
  if (ret == nullptr) return nullptr;
  /*
    Trim whitespace as the values will go to a CHAR field
    when stored.
  */
  trim_whitespace(system_charset_info, user_arg);
  if (host_arg) trim_whitespace(system_charset_info, host_arg);

  ret->user.str = user_arg->str;
  ret->user.length = user_arg->length;
  ret->host.str = host_arg ? host_arg->str : "%";
  ret->host.length = host_arg ? host_arg->length : 1;
  ret->plugin = EMPTY_CSTR;
  ret->auth = NULL_CSTR;
  ret->current_auth = NULL_CSTR;
  ret->uses_replace_clause = false;
  ret->uses_identified_by_clause = false;
  ret->uses_identified_with_clause = false;
  ret->uses_authentication_string_clause = false;
  ret->has_password_generator = false;
  ret->retain_current_password = false;
  ret->discard_old_password = false;
  ret->alter_status.account_locked = false;
  ret->alter_status.expire_after_days = 0;
  ret->alter_status.update_account_locked_column = false;
  ret->alter_status.update_password_expired_column = false;
  ret->alter_status.update_password_expired_fields = false;
  ret->alter_status.use_default_password_lifetime = true;
  ret->alter_status.use_default_password_history = true;
  ret->alter_status.update_password_require_current =
      Lex_acl_attrib_udyn::UNCHANGED;
  ret->alter_status.password_history_length = 0;
  ret->alter_status.password_reuse_interval = 0;
  ret->alter_status.failed_login_attempts = 0;
  ret->alter_status.password_lock_time = 0;
  ret->alter_status.update_failed_login_attempts = false;
  ret->alter_status.update_password_lock_time = false;
  if (check_string_char_length(ret->user, ER_THD(thd, ER_USERNAME),
                               USERNAME_CHAR_LENGTH, system_charset_info,
                               false) ||
      (host_arg && check_host_name(ret->host)))
    return nullptr;
  if (host_arg) {
    /*
      Convert hostname part of username to lowercase.
      It's OK to use in-place lowercase as long as
      the character set is utf8.
    */
    my_casedn_str(system_charset_info, host_arg->str);
    ret->host.str = host_arg->str;
  }
  return ret;
}

/**
  A struct that contains execution time state used for partial update of JSON
  columns.
*/
struct Partial_update_info {
  Partial_update_info(const TABLE *table, const MY_BITMAP *columns,
                      bool logical_diffs)
      : m_binary_diff_vectors(table->in_use->mem_root, table->s->fields,
                              nullptr),
        m_logical_diff_vectors(table->in_use->mem_root,
                               logical_diffs ? table->s->fields : 0, nullptr) {
    MEM_ROOT *const mem_root = table->in_use->mem_root;
    const size_t bitmap_size = table->s->column_bitmap_size;

    auto buffer = static_cast<my_bitmap_map *>(mem_root->Alloc(bitmap_size));
    if (buffer != nullptr) {
      bitmap_init(&m_enabled_binary_diff_columns, buffer, table->s->fields);
      bitmap_copy(&m_enabled_binary_diff_columns, columns);
    }

    buffer = static_cast<my_bitmap_map *>(mem_root->Alloc(bitmap_size));
    if (buffer != nullptr) {
      bitmap_init(&m_enabled_logical_diff_columns, buffer, table->s->fields);
      if (logical_diffs)
        bitmap_copy(&m_enabled_logical_diff_columns, columns);
      else
        bitmap_clear_all(&m_enabled_logical_diff_columns);
    }

    for (uint i = bitmap_get_first_set(columns); i != MY_BIT_NONE;
         i = bitmap_get_next_set(columns, i)) {
      m_binary_diff_vectors[i] = new (mem_root) Binary_diff_vector(mem_root);

      if (logical_diffs) {
        Json_diff_vector::allocator_type alloc(mem_root);
        m_logical_diff_vectors[i] = new (mem_root) Json_diff_vector(alloc);
      }
    }
  }

  ~Partial_update_info() {
    for (auto v : m_logical_diff_vectors) destroy(v);
  }

  /**
    The columns for which partial update using binary diffs is enabled
    in the current row.
  */
  MY_BITMAP m_enabled_binary_diff_columns;

  /**
    The columns for which partial update using logical JSON diffs is
    enabled in the current row.
  */
  MY_BITMAP m_enabled_logical_diff_columns;

  /**
    The binary diffs that have been collected for the current row.

    The Binary_diff_vector objects live entirely in a MEM_ROOT, so
    there is no need to destroy them when this object is destroyed.
  */
  Mem_root_array<Binary_diff_vector *> m_binary_diff_vectors;

  /**
    The logical diffs that have been collected for JSON operations in
    the current row.

    Whereas the Json_diff_vector objects live in a MEM_ROOT and their
    memory will be reclaimed automatically, the Json_diff objects
    within them can own memory allocated on the heap, so they will
    have to be destroyed when this object is destroyed.
  */
  Mem_root_array<Json_diff_vector *> m_logical_diff_vectors;

  /**
    A buffer that can be used to hold the partially updated column value while
    performing the update in memory.
  */
  String m_buffer;

  /// Should logical JSON diffs be collected in addition to binary diffs?
  bool collect_logical_diffs() const {
    /*
      We only allocate logical diff vectors when we want logical diffs
      to be collected, so check if we have any.
    */
    return !m_logical_diff_vectors.empty();
  }
};

bool TABLE::mark_column_for_partial_update(const Field *field) {
  DBUG_ASSERT(field->table == this);
  if (m_partial_update_columns == nullptr) {
    MY_BITMAP *map = new (&mem_root) MY_BITMAP;
    my_bitmap_map *buf =
        static_cast<my_bitmap_map *>(mem_root.Alloc(s->column_bitmap_size));
    if (map == nullptr || buf == nullptr || bitmap_init(map, buf, s->fields))
      return true; /* purecov: inspected */
    m_partial_update_columns = map;
  }

  bitmap_set_bit(m_partial_update_columns, field->field_index);
  return false;
}

void TABLE::disable_binary_diffs_for_current_row(const Field *field) {
  DBUG_ASSERT(field->table == this);
  DBUG_ASSERT(is_binary_diff_enabled(field));

  // Remove the diffs collected for the column.
  m_partial_update_info->m_binary_diff_vectors[field->field_index]->clear();

  // Mark the column as disabled.
  bitmap_clear_bit(&m_partial_update_info->m_enabled_binary_diff_columns,
                   field->field_index);
}

bool TABLE::is_marked_for_partial_update(const Field *field) const {
  DBUG_ASSERT(field->table == this);
  return m_partial_update_columns != nullptr &&
         bitmap_is_set(m_partial_update_columns, field->field_index);
}

bool TABLE::has_binary_diff_columns() const {
  return m_partial_update_info != nullptr &&
         !bitmap_is_clear_all(
             &m_partial_update_info->m_enabled_binary_diff_columns);
}

bool TABLE::setup_partial_update(bool logical_diffs) {
  DBUG_TRACE;
  DBUG_ASSERT(m_partial_update_info == nullptr);

  if (!has_columns_marked_for_partial_update()) return false;

  Opt_trace_context *trace = &in_use->opt_trace;
  if (trace->is_started()) {
    Opt_trace_object trace_wrapper(trace);
    Opt_trace_object trace_partial_update(trace, "json_partial_update");
    trace_partial_update.add_utf8_table(pos_in_table_list);
    Opt_trace_array columns(trace, "eligible_columns");
    for (uint i = bitmap_get_first_set(m_partial_update_columns);
         i != MY_BIT_NONE;
         i = bitmap_get_next_set(m_partial_update_columns, i)) {
      columns.add_utf8(s->field[i]->field_name);
    }
  }

  m_partial_update_info = new (in_use->mem_root)
      Partial_update_info(this, m_partial_update_columns, logical_diffs);
  return in_use->is_error();
}

bool TABLE::setup_partial_update() {
  bool logical_diffs = (in_use->variables.binlog_row_value_options &
                        PARTIAL_JSON_UPDATES) != 0 &&
                       mysql_bin_log.is_open() &&
                       (in_use->variables.option_bits & OPTION_BIN_LOG) != 0 &&
                       log_bin_use_v1_row_events == 0 &&
                       in_use->is_current_stmt_binlog_format_row();
  DBUG_PRINT(
      "info",
      ("TABLE::setup_partial_update(): logical_diffs=%d "
       "because binlog_row_value_options=%d binlog.is_open=%d "
       "sql_log_bin=%d use_v1_row_events=%d rbr=%d",
       logical_diffs,
       (in_use->variables.binlog_row_value_options & PARTIAL_JSON_UPDATES) != 0,
       mysql_bin_log.is_open(),
       (in_use->variables.option_bits & OPTION_BIN_LOG) != 0,
       log_bin_use_v1_row_events, in_use->is_current_stmt_binlog_format_row()));
  return setup_partial_update(logical_diffs);
}

bool TABLE::has_columns_marked_for_partial_update() const {
  /*
    Do we have any columns that satisfy the syntactical requirements for
    partial update?
  */
  return m_partial_update_columns != nullptr &&
         !bitmap_is_clear_all(m_partial_update_columns);
}

void TABLE::cleanup_partial_update() {
  DBUG_TRACE;
  destroy(m_partial_update_info);
  m_partial_update_info = nullptr;
}

String *TABLE::get_partial_update_buffer() {
  DBUG_ASSERT(m_partial_update_info != nullptr);
  return &m_partial_update_info->m_buffer;
}

void TABLE::clear_partial_update_diffs() {
  DBUG_TRACE;
  if (m_partial_update_info != nullptr) {
    for (auto v : m_partial_update_info->m_binary_diff_vectors)
      if (v != nullptr) v->clear();

    bitmap_copy(&m_partial_update_info->m_enabled_binary_diff_columns,
                m_partial_update_columns);

    if (m_partial_update_info->collect_logical_diffs()) {
      for (auto v : m_partial_update_info->m_logical_diff_vectors)
        if (v != nullptr) v->clear();

      bitmap_copy(&m_partial_update_info->m_enabled_logical_diff_columns,
                  m_partial_update_columns);
    }
  }
}

const Binary_diff_vector *TABLE::get_binary_diffs(const Field *field) const {
  if (!is_binary_diff_enabled(field)) return nullptr;
  return m_partial_update_info->m_binary_diff_vectors[field->field_index];
}

bool TABLE::add_binary_diff(const Field *field, size_t offset, size_t length) {
  DBUG_ASSERT(is_binary_diff_enabled(field));

  Binary_diff_vector *diffs =
      m_partial_update_info->m_binary_diff_vectors[field->field_index];

  /*
    Find the first diff that does not end before the diff we want to insert.
    That is, we find the first diff that is either overlapping with the diff we
    want to insert, adjacent to the diff we want to insert, or comes after the
    diff that we want to insert.

    In the case of overlapping or adjacent diffs, we want to merge the diffs
    rather than insert a new one.
  */
  Binary_diff_vector::iterator first_it =
      std::lower_bound(diffs->begin(), diffs->end(), offset,
                       [](const Binary_diff &diff, size_t start_offset) {
                         return diff.offset() + diff.length() < start_offset;
                       });

  if (first_it != diffs->end() && first_it->offset() <= offset + length) {
    /*
      The diff we found was overlapping or adjacent, so we want to merge the
      new diff with it. Find out if the new diff overlaps with or borders to
      some of the diffs behind it. The call below finds the first diff after
      first_it that is not overlapping with or adjacent to the new diff.
    */
    Binary_diff_vector::const_iterator last_it =
        std::upper_bound(first_it, diffs->end(), offset + length,
                         [](size_t end_offset, const Binary_diff &diff) {
                           return end_offset < diff.offset();
                         });

    // First and last adjacent or overlapping diff. They can be the same one.
    const Binary_diff &first_diff = *first_it;
    const Binary_diff &last_diff = *(last_it - 1);

    // Calculate the boundaries of the merged diff.
    size_t beg = std::min(offset, first_diff.offset());
    size_t end =
        std::max(offset + length, last_diff.offset() + last_diff.length());

    /*
      Replace the first overlapping/adjacent diff with the merged diff, and
      erase any subsequent diffs that are covered by the merged diff.
    */
    *first_it = Binary_diff(beg, end - beg);
    diffs->erase(first_it + 1, last_it);
    return false;
  }

  /*
    The new diff isn't overlapping with or adjacent to any of the existing
    diffs. Just insert it.
  */
  diffs->insert(first_it, Binary_diff(offset, length));
  return false;
}

const char *Binary_diff::new_data(Field *field) const {
  /*
    Currently, partial update is only supported for JSON columns, so it's
    safe to assume that the Field is in fact a Field_json.
  */
  auto fld = down_cast<Field_json *>(field);
  return fld->get_binary() + m_offset;
}

const char *Binary_diff::old_data(Field *field) const {
  ptrdiff_t ptrdiff = field->table->record[1] - field->table->record[0];
  field->move_field_offset(ptrdiff);
  const char *data = new_data(field);
  field->move_field_offset(-ptrdiff);
  return data;
}

void TABLE::add_logical_diff(const Field_json *field,
                             const Json_seekable_path &path,
                             enum_json_diff_operation operation,
                             const Json_wrapper *new_value) {
  DBUG_ASSERT(is_logical_diff_enabled(field));
  Json_diff_vector *diffs =
      m_partial_update_info->m_logical_diff_vectors[field->field_index];
  if (new_value == nullptr)
    diffs->add_diff(path, operation);
  else {
    diffs->add_diff(path, operation,
                    new_value->clone_dom(field->table->in_use));
  }
#ifndef DBUG_OFF
  StringBuffer<STRING_BUFFER_USUAL_SIZE> path_str;
  StringBuffer<STRING_BUFFER_USUAL_SIZE> value_str;
  if (diffs->at(diffs->size() - 1).path().to_string(&path_str))
    path_str.length(0); /* purecov: inspected */
  if (new_value == nullptr || new_value->type() == enum_json_type::J_ERROR)
    value_str.set_ascii("<none>", 6);
  else {
    if (new_value->to_string(&value_str, false, "add_logical_diff"))
      value_str.length(0); /* purecov: inspected */
  }
  DBUG_PRINT("info", ("add_logical_diff(operation=%d, path=%.*s, value=%.*s)",
                      (int)operation, (int)path_str.length(), path_str.ptr(),
                      (int)value_str.length(), value_str.ptr()));
#endif
}

const Json_diff_vector *TABLE::get_logical_diffs(
    const Field_json *field) const {
  if (!is_logical_diff_enabled(field)) return nullptr;
  return m_partial_update_info->m_logical_diff_vectors[field->field_index];
}

bool TABLE::is_binary_diff_enabled(const Field *field) const {
  return m_partial_update_info != nullptr &&
         bitmap_is_set(&m_partial_update_info->m_enabled_binary_diff_columns,
                       field->field_index);
}

bool TABLE::is_logical_diff_enabled(const Field *field) const {
  DBUG_TRACE;
  bool ret =
      m_partial_update_info != nullptr &&
      bitmap_is_set(&m_partial_update_info->m_enabled_logical_diff_columns,
                    field->field_index);
  DBUG_PRINT("info",
             ("field=%s "
              "is_logical_diff_enabled returns=%d "
              "(m_partial_update_info!=NULL)=%d "
              "m_enabled_logical_diff_columns[column]=%s",
              field->field_name, ret, m_partial_update_info != nullptr,
              m_partial_update_info != nullptr
                  ? (bitmap_is_set(
                         &m_partial_update_info->m_enabled_logical_diff_columns,
                         field->field_index)
                         ? "1"
                         : "0")
                  : "unknown"));
  return ret;
}

void TABLE::disable_logical_diffs_for_current_row(const Field *field) const {
  DBUG_ASSERT(field->table == this);
  DBUG_ASSERT(is_logical_diff_enabled(field));

  // Remove the diffs collected for the column.
  m_partial_update_info->m_logical_diff_vectors[field->field_index]->clear();

  // Mark the column as disabled.
  bitmap_clear_bit(&m_partial_update_info->m_enabled_logical_diff_columns,
                   field->field_index);
}

//////////////////////////////////////////////////////////////////////////

/*
  NOTE:

  The functions in this block are used to read .frm file.
  They should not be used any where else in the code. They are only used
  in upgrade scenario for migrating old data directory to be compatible
  with current server. They will be removed in future release.

  Any new code should not be added in this section.
*/

/**
  Open and Read .frm file.
  Based on header, it is decided if its a table or view.
  Prepare TABLE_SHARE if its a table.
  Prepare File_parser if its a view.

  @param  thd                       thread handle
  @param  share                     TABLE_SHARE object to be filled.
  @param  frm_context               FRM_context for structures removed from
                                    TABLE_SHARE
  @param  table                     table name
  @param  is_fix_view_cols_and_deps Flag to indicate that we are recreating view
                                    to create view dependency entry in DD tables

  @retval  true   Error
  @retval  false  Success
*/
static bool read_frm_file(THD *thd, TABLE_SHARE *share,
                          FRM_context *frm_context, const std::string &table,
                          bool is_fix_view_cols_and_deps) {
  File file;
  uchar head[64];
  char path[FN_REFLEN + 1];
  MEM_ROOT **root_ptr, *old_root;

  strxnmov(path, sizeof(path) - 1, share->normalized_path.str, reg_ext, NullS);
  LEX_STRING pathstr = {path, strlen(path)};

  if ((file = mysql_file_open(key_file_frm, path, O_RDONLY, MYF(0))) < 0) {
    LogErr(ERROR_LEVEL, ER_CANT_OPEN_FRM_FILE, path);
    return true;
  }

  if (mysql_file_read(file, head, 64, MYF(MY_NABP))) {
    LogErr(ERROR_LEVEL, ER_CANT_READ_FRM_FILE, path);
    goto err;
  }

  /*
    Checking if the given .frm file is TABLE or VIEW.
  */
  if (head[0] == (uchar)254 && head[1] == 1) {
    if (head[2] == FRM_VER || head[2] == FRM_VER + 1 ||
        (head[2] >= FRM_VER + 3 && head[2] <= FRM_VER + 4)) {
      /*
        This means this is a BASE_TABLE.
        Don't read .frm file for tables if we are recreating views
        to resolve dependency. At this time, all tables are already upgraded.
        .frm file should be only read for views.
      */
      if (is_fix_view_cols_and_deps) {
        mysql_file_close(file, MYF(MY_WME));
        return false;
      }
      int error;
      root_ptr = THR_MALLOC;
      old_root = *root_ptr;
      *root_ptr = &share->mem_root;

      error = open_binary_frm(thd, share, frm_context, head, file);

      *root_ptr = old_root;
      if (error) {
        LogErr(ERROR_LEVEL, ER_CANT_READ_FRM_FILE, path);
        goto err;
      }
    } else {
      LogErr(ERROR_LEVEL, ER_TABLE_CREATED_WITH_DIFFERENT_VERSION,
             table.c_str());
      goto err;
    }
  } else if (memcmp(head, STRING_WITH_LEN("TYPE=")) == 0) {
    if (memcmp(head + 5, "VIEW", 4) == 0) {
      // View found
      share->is_view = true;

      /*
        Create view file parser and hold it in
        FRM_context member view_def.
      */
      frm_context->view_def =
          sql_parse_prepare(&pathstr, &share->mem_root, true);
      if (!frm_context->view_def) {
        LogErr(ERROR_LEVEL, ER_VIEW_UNPARSABLE, pathstr.str);
        goto err;
      }
    } else {
      LogErr(ERROR_LEVEL, ER_FILE_TYPE_UNKNOWN, pathstr.str);
      goto err;
    }
  } else {
    LogErr(ERROR_LEVEL, ER_INVALID_INFO_IN_FRM, pathstr.str);
    goto err;
  }

  // Close file and return
  mysql_file_close(file, MYF(MY_WME));
  return false;

err:
  mysql_file_close(file, MYF(MY_WME));
  return true;
}

bool create_table_share_for_upgrade(THD *thd, const char *path,
                                    TABLE_SHARE *share,
                                    FRM_context *frm_context,
                                    const char *db_name, const char *table_name,
                                    bool is_fix_view_cols_and_deps) {
  DBUG_TRACE;

  init_tmp_table_share(thd, share, db_name, 0, table_name, path, nullptr);

  // Fix table categories set by init_tmp_table_share
  share->table_category = TABLE_UNKNOWN_CATEGORY;
  share->tmp_table = NO_TMP_TABLE;
  mysql_mutex_init(key_TABLE_SHARE_LOCK_ha_data, &share->LOCK_ha_data,
                   MY_MUTEX_INIT_FAST);

  if (read_frm_file(thd, share, frm_context, table_name,
                    is_fix_view_cols_and_deps)) {
    free_table_share(share);
    return true;
  }
  return false;
}

void TABLE::blobs_need_not_keep_old_value() {
  for (Field **vfield_ptr = vfield; *vfield_ptr; vfield_ptr++) {
    Field *vfield = *vfield_ptr;
    /*
      Set this flag so that all blob columns can keep the old value.
    */
    if (vfield->handle_old_value())
      (down_cast<Field_blob *>(vfield))->set_keep_old_value(false);
  }
}

void TABLE::set_binlog_drop_if_temp(bool should_binlog) {
  should_binlog_drop_if_temp_flag = should_binlog;
}

bool TABLE::should_binlog_drop_if_temp(void) const {
  return should_binlog_drop_if_temp_flag;
}

bool TABLE::empty_result_table() {
  materialized = false;
  set_not_started();
  if (!is_created()) return false;
  if (file->ha_index_or_rnd_end() || file->ha_extra(HA_EXTRA_RESET_STATE) ||
      file->ha_delete_all_rows())
    return true;
  free_io_cache(this);
  filesort_free_buffers(this, false);
  return false;
}

void TABLE::update_covering_prefix_keys(Field *field, uint16 key_read_length,
                                        Key_map *covering_prefix_keys) {
  for (uint keyno = 0; keyno < s->keys; keyno++)
    if (covering_prefix_keys->is_set(keyno)) {
      KEY *key_info = &this->key_info[keyno];
      for (KEY_PART_INFO *part = key_info->key_part,
                         *part_end = part + actual_key_parts(key_info);
           part != part_end; ++part)
        if ((part->key_part_flag & HA_PART_KEY_SEG) && field->eq(part->field)) {
          uint16 key_part_length = part->length / field->charset()->mbmaxlen;
          if (key_part_length < key_read_length) covering_keys.clear_bit(keyno);
        }
    }
}

/*
  Prepare triggers  for INSERT-like statement.
  SYNOPSIS
    prepare_triggers_for_insert_stmt_or_event()
  NOTE
    Prepare triggers for INSERT-like statement by marking fields
    used by triggers and inform handlers that batching of UPDATE/DELETE
    cannot be done if there are BEFORE UPDATE/DELETE triggers.
*/
void TABLE::prepare_triggers_for_insert_stmt_or_event() {
  if (triggers) {
    if (triggers->has_triggers(TRG_EVENT_DELETE, TRG_ACTION_AFTER)) {
      /*
        The table has AFTER DELETE triggers that might access to
        subject table and therefore might need delete to be done
        immediately. So we turn-off the batching.
      */
      (void)file->ha_extra(HA_EXTRA_DELETE_CANNOT_BATCH);
    }
    if (triggers->has_triggers(TRG_EVENT_UPDATE, TRG_ACTION_AFTER)) {
      /*
        The table has AFTER UPDATE triggers that might access to subject
        table and therefore might need update to be done immediately.
        So we turn-off the batching.
      */
      (void)file->ha_extra(HA_EXTRA_UPDATE_CANNOT_BATCH);
    }
  }
}

bool TABLE::prepare_triggers_for_delete_stmt_or_event() {
  if (triggers && triggers->has_triggers(TRG_EVENT_DELETE, TRG_ACTION_AFTER)) {
    /*
      The table has AFTER DELETE triggers that might access to subject table
      and therefore might need delete to be done immediately. So we turn-off
      the batching.
    */
    (void)file->ha_extra(HA_EXTRA_DELETE_CANNOT_BATCH);
    return true;
  }
  return false;
}

bool TABLE::prepare_triggers_for_update_stmt_or_event() {
  if (triggers && triggers->has_triggers(TRG_EVENT_UPDATE, TRG_ACTION_AFTER)) {
    /*
      The table has AFTER UPDATE triggers that might access to subject
      table and therefore might need update to be done immediately.
      So we turn-off the batching.
    */
    (void)file->ha_extra(HA_EXTRA_UPDATE_CANNOT_BATCH);
    return true;
  }
  return false;
}

void TABLE::set_pos_in_table_list(TABLE_LIST *table_list) noexcept {
  pos_in_table_list = table_list;
  if (table_list)
    disable_sql_log_bin_triggers = table_list->disable_sql_log_bin_triggers;
}

//////////////////////////////////////////////////////////////////////////

void TABLE_SHARE::set_last_access_time() noexcept {
  this->last_accessed = std::chrono::system_clock::now();
}

void TABLE::set_last_access_time() noexcept {
  this->last_accessed = std::chrono::system_clock::now();
}

bool should_be_evicted(time_point last_accessed, time_point cutpoint) noexcept {
  return last_accessed < cutpoint;
}
