/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/* Basic functions needed by many modules */

#include "sql/sql_base.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>

#include "libbinlogevents/include/table_id.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "mf_wcomp.h"  // wild_one, wild_many
#include "mutex_lock.h"
#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_table_map.h"
#include "my_thread_local.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_table.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_table.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/thread_type.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_table_access
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"  // mysql_bin_log
#include "sql/check_stack.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd_schema.h"
#include "sql/dd/dd_table.h"       // dd::table_exists
#include "sql/dd/dd_tablespace.h"  // dd::fill_table_and_parts_tablespace_name
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/column.h"
#include "sql/dd/types/column_statistics.h"
#include "sql/dd/types/foreign_key.h"  // dd::Foreign_key
#include "sql/dd/types/function.h"
#include "sql/dd/types/procedure.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/types/table.h"  // dd::Table
#include "sql/dd/types/view.h"
#include "sql/dd_table_share.h"  // open_table_def
#include "sql/debug_sync.h"      // DEBUG_SYNC
#include "sql/derror.h"          // ER_THD
#include "sql/error_handler.h"   // Internal_error_handler
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/histograms/histogram.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"  // Item_func_eq
#include "sql/item_func.h"
#include "sql/item_subselect.h"
#include "sql/lock.h"  // mysql_lock_remove
#include "sql/log.h"
#include "sql/log_event.h"  // Query_log_event
#include "sql/mysqld.h"     // slave_open_temp_tables
#include "sql/nested_join.h"
#include "sql/partition_info.h"  // partition_info
#include "sql/psi_memory_key.h"  // key_memory_TABLE
#include "sql/query_options.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_handler.h"  // RUN_HOOK
#include "sql/rpl_rli.h"      //Relay_log_information
#include "sql/session_tracker.h"
#include "sql/sp.h"               // Sroutine_hash_entry
#include "sql/sp_cache.h"         // sp_cache_version
#include "sql/sp_head.h"          // sp_head
#include "sql/sql_audit.h"        // mysql_audit_table_access_notify
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_class.h"        // THD
#include "sql/sql_const.h"
#include "sql/sql_data_change.h"
#include "sql/sql_error.h"    // Sql_condition
#include "sql/sql_handler.h"  // mysql_ha_flush_tables
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"    // is_update_query
#include "sql/sql_prepare.h"  // Reprepare_observer
#include "sql/sql_select.h"   // reset_statement_timer
#include "sql/sql_show.h"     // append_identifier
#include "sql/sql_sort.h"
#include "sql/sql_table.h"   // build_table_filename
#include "sql/sql_update.h"  // records_are_comparable
#include "sql/sql_view.h"    // mysql_make_view
#include "sql/strfunc.h"
#include "sql/system_variables.h"
#include "sql/table.h"                     // TABLE_LIST
#include "sql/table_cache.h"               // table_cache_manager
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_rollback_stmt
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_mutex.h"

using std::equal_to;
using std::hash;
using std::pair;
using std::string;
using std::unique_ptr;
using std::unordered_map;

/**
  This internal handler is used to trap ER_NO_SUCH_TABLE and
  ER_WRONG_MRG_TABLE errors during CHECK/REPAIR TABLE for MERGE
  tables.
*/

class Repair_mrg_table_error_handler : public Internal_error_handler {
 public:
  Repair_mrg_table_error_handler()
      : m_handled_errors(false), m_unhandled_errors(false) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_NO_SUCH_TABLE || sql_errno == ER_WRONG_MRG_TABLE) {
      m_handled_errors = true;
      return true;
    }

    m_unhandled_errors = true;
    return false;
  }

  /**
    Returns true if there were ER_NO_SUCH_/WRONG_MRG_TABLE and there
    were no unhandled errors. false otherwise.
  */
  bool safely_trapped_errors() {
    /*
      Check for m_handled_errors is here for extra safety.
      It can be useful in situation when call to open_table()
      fails because some error which was suppressed by another
      error handler (e.g. in case of MDL deadlock which we
      decided to solve by back-off and retry).
    */
    return (m_handled_errors && (!m_unhandled_errors));
  }

 private:
  bool m_handled_errors;
  bool m_unhandled_errors;
};

/**
  @defgroup Data_Dictionary Data Dictionary
  @{
*/

/**
  LOCK_open protects the following variables/objects:

  1) The table_def_cache
     This is the hash table mapping table name to a table
     share object. The hash table can only be manipulated
     while holding LOCK_open.
  2) last_table_id
     Generation of a new unique table_map_id for a table
     share is done through incrementing last_table_id, a
     global variable used for this purpose.
  3) LOCK_open protects the initialisation of the table share
     object and all its members, however, it does not protect
     reading the .frm file from where the table share is
     initialised. In get_table_share, the lock is temporarily
     released while opening the table definition in order to
     allow a higher degree of concurrency. Concurrent access
     to the same share is controlled by introducing a condition
     variable for signaling when opening the share is completed.
  4) In particular the share->ref_count is updated each time
     a new table object is created that refers to a table share.
     This update is protected by LOCK_open.
  5) oldest_unused_share, end_of_unused_share and share->next
     and share->prev are variables to handle the lists of table
     share objects, these can only be read and manipulated while
     holding the LOCK_open mutex.
  6) table_def_shutdown_in_progress can be updated only while
     holding LOCK_open and ALL table cache mutexes.
  7) refresh_version
     This variable can only be updated while holding LOCK_open AND
     all table cache mutexes.
  8) share->version
     This variable is initialised while holding LOCK_open. It can only
     be updated while holding LOCK_open AND all table cache mutexes.
     So if a table share is found through a reference its version won't
     change if any of those mutexes are held.
  9) share->m_flush_tickets
*/

mysql_mutex_t LOCK_open;

/**
  COND_open synchronizes concurrent opening of the same share:

  If a thread calls get_table_share, it releases the LOCK_open
  mutex while reading the definition from file. If a different
  thread calls get_table_share for the same share at this point
  in time, it will find the share in the TDC, but with the
  m_open_in_progress flag set to true. This will make the
  (second) thread wait for the COND_open condition, while the
  first thread completes opening the table definition.

  When the first thread is done reading the table definition,
  it will set m_open_in_progress to false and broadcast the
  COND_open condition. Then, all threads waiting for COND_open
  will wake up and, re-search the TDC for the share, and:

  1) If the share is gone, the thread will continue to allocate
     and open the table definition. This happens, e.g., if the
     first thread failed when opening the table defintion and
     had to destroy the share.
  2) If the share is still in the cache, and m_open_in_progress
     is still true, the thread will wait for the condition again.
     This happens if a different thread finished opening a
     different share.
  3) If the share is still in the cache, and m_open_in_progress
     has become false, the thread will check if the share is ok
     (no error), increment the ref counter, and return the share.
*/

mysql_cond_t COND_open;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_open;
static PSI_cond_key key_COND_open;
static PSI_mutex_info all_tdc_mutexes[] = {
    {&key_LOCK_open, "LOCK_open", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}};
static PSI_cond_info all_tdc_conds[] = {
    {&key_COND_open, "COND_open", 0, 0, PSI_DOCUMENT_ME}};

/**
  Initialize performance schema instrumentation points
  used by the table cache.
*/

static void init_tdc_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_tdc_mutexes));
  mysql_mutex_register(category, all_tdc_mutexes, count);

  count = static_cast<int>(array_elements(all_tdc_conds));
  mysql_cond_register(category, all_tdc_conds, count);
}
#endif /* HAVE_PSI_INTERFACE */

using Table_definition_cache =
    malloc_unordered_map<std::string,
                         std::unique_ptr<TABLE_SHARE, Table_share_deleter>>;
Table_definition_cache *table_def_cache;
static TABLE_SHARE *oldest_unused_share, end_of_unused_share;
static bool table_def_shutdown_in_progress = false;

static bool check_and_update_table_version(THD *thd, TABLE_LIST *tables,
                                           TABLE_SHARE *table_share);
static bool open_table_entry_fini(THD *thd, TABLE_SHARE *share,
                                  const dd::Table *table, TABLE *entry);
static bool auto_repair_table(THD *thd, TABLE_LIST *table_list);
static TABLE *find_temporary_table(THD *thd, const char *table_key,
                                   size_t table_key_length);
static bool tdc_open_view(THD *thd, TABLE_LIST *table_list,
                          const char *cache_key, size_t cache_key_length);

/**
  Create a table cache/table definition cache key for a table. The
  table is neither a temporary table nor a table in a secondary
  storage engine.

  @note
    The table cache_key is created from:

        db_name + \0
        table_name + \0

  @param[in]  db_name     the database name
  @param[in]  table_name  the table name
  @param[out] key         buffer for the key to be created (must be of
                          size MAX_DBKEY_LENGTH)
  @return the length of the key
*/
static size_t create_table_def_key(const char *db_name, const char *table_name,
                                   char *key) {
  /*
    In theory caller should ensure that both db and table_name are
    not longer than NAME_LEN bytes. In practice we play safe to avoid
    buffer overruns.
  */
  DBUG_ASSERT(strlen(db_name) <= NAME_LEN && strlen(table_name) <= NAME_LEN);
  return strmake(strmake(key, db_name, NAME_LEN) + 1, table_name, NAME_LEN) -
         key + 1;
}

/**
  Create a table cache/table definition cache key for a temporary table.

  The key is constructed by appending the following to the key
  generated by #create_table_def_key():

  - 4 bytes for master thread id
  - 4 bytes pseudo thread id

  @param[in]  thd         thread context
  @param[in]  db_name     the database name
  @param[in]  table_name  the table name
  @param[out] key         buffer for the key to be created (must be of
                          size MAX_DBKEY_LENGTH)
  @return the length of the key
*/
static size_t create_table_def_key_tmp(const THD *thd, const char *db_name,
                                       const char *table_name, char *key) {
  size_t key_length = create_table_def_key(db_name, table_name, key);
  int4store(key + key_length, thd->server_id);
  int4store(key + key_length + 4, thd->variables.pseudo_thread_id);
  return key_length + TMP_TABLE_KEY_EXTRA;
}

/**
  Create a table cache/table definition cache key for a table in a
  secondary storage engine.

  The key is constructed by appending a single byte with the value 1
  to the key generated by #create_table_def_key().

  @param db_name     the database name
  @param table_name  the table name
  @return the key
*/
static std::string create_table_def_key_secondary(const char *db_name,
                                                  const char *table_name) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length = create_table_def_key(db_name, table_name, key);
  // Add a single byte to distinguish the secondary table from the
  // primary table. Their db name and table name are identical.
  key[key_length++] = 1;
  return {key, key_length};
}

/**
  Get table cache key for a table list element.

  @param [in] table_list Table list element.
  @param [out] key       On return points to table cache key for the table.

  @note Unlike create_table_def_key() call this function doesn't construct
        key in a buffer provider by caller. Instead it relies on the fact
        that table list element for which key is requested has properly
        initialized MDL_request object and the fact that table definition
        cache key is suffix of key used in MDL subsystem. So to get table
        definition key it simply needs to return pointer to appropriate
        part of MDL_key object nested in this table list element.
        Indeed, this means that lifetime of key produced by this call is
        limited by the lifetime of table list element which it got as
        parameter.

  @return Length of key.
*/

size_t get_table_def_key(const TABLE_LIST *table_list, const char **key) {
  /*
    This call relies on the fact that TABLE_LIST::mdl_request::key object
    is properly initialized, so table definition cache can be produced
    from key used by MDL subsystem.
    strcase is converted to strcasecmp because information_schema tables
    can be accessed with lower case and upper case table names.
  */
  DBUG_ASSERT(!my_strcasecmp(system_charset_info, table_list->get_db_name(),
                             table_list->mdl_request.key.db_name()) &&
              !my_strcasecmp(system_charset_info, table_list->get_table_name(),
                             table_list->mdl_request.key.name()));

  *key = (const char *)table_list->mdl_request.key.ptr() + 1;
  return table_list->mdl_request.key.length() - 1;
}

/*****************************************************************************
  Functions to handle table definition cach (TABLE_SHARE)
*****************************************************************************/

void Table_share_deleter::operator()(TABLE_SHARE *share) const {
  DBUG_TRACE;
  mysql_mutex_assert_owner(&LOCK_open);
  if (share->prev) {
    /* remove from old_unused_share list */
    *share->prev = share->next;
    share->next->prev = share->prev;
  }
  free_table_share(share);
}

bool table_def_init(void) {
#ifdef HAVE_PSI_INTERFACE
  init_tdc_psi_keys();
#endif
  mysql_mutex_init(key_LOCK_open, &LOCK_open, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_open, &COND_open);
  oldest_unused_share = &end_of_unused_share;
  end_of_unused_share.prev = &oldest_unused_share;

  if (table_cache_manager.init()) {
    mysql_cond_destroy(&COND_open);
    mysql_mutex_destroy(&LOCK_open);
    return true;
  }

  table_def_cache = new Table_definition_cache(key_memory_table_share);
  return false;
}

/**
  Notify table definition cache that process of shutting down server
  has started so it has to keep number of TABLE and TABLE_SHARE objects
  minimal in order to reduce number of references to pluggable engines.
*/

void table_def_start_shutdown(void) {
  if (table_def_cache != nullptr) {
    table_cache_manager.lock_all_and_tdc();
    /*
      Ensure that TABLE and TABLE_SHARE objects which are created for
      tables that are open during process of plugins' shutdown are
      immediately released. This keeps number of references to engine
      plugins minimal and allows shutdown to proceed smoothly.
    */
    table_def_shutdown_in_progress = true;
    table_cache_manager.unlock_all_and_tdc();
    /* Free all cached but unused TABLEs and TABLE_SHAREs. */
    close_cached_tables_nsec(nullptr, nullptr, false, LONG_TIMEOUT_NSEC);
  }
}

void table_def_free(void) {
  DBUG_TRACE;
  if (table_def_cache != nullptr) {
    /* Free table definitions. */
    delete table_def_cache;
    table_def_cache = nullptr;
    table_cache_manager.destroy();
    mysql_cond_destroy(&COND_open);
    mysql_mutex_destroy(&LOCK_open);
  }
}

uint cached_table_definitions(void) { return table_def_cache->size(); }

static TABLE_SHARE *process_found_table_share(THD *thd MY_ATTRIBUTE((unused)),
                                              TABLE_SHARE *share,
                                              bool open_view) {
  DBUG_TRACE;
  mysql_mutex_assert_owner(&LOCK_open);
#if defined(ENABLED_DEBUG_SYNC)
  if (!thd->is_attachable_ro_transaction_active())
    DEBUG_SYNC(thd, "get_share_found_share");
#endif
  /*
     We found an existing table definition. Return it if we didn't get
     an error when reading the table definition from file.
  */
  if (share->error) {
    /*
      Table definition contained an error.
      Note that we report ER_NO_SUCH_TABLE regardless of which error occurred
      when the other thread tried to open the table definition (e.g. OOM).
    */
    my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str, share->table_name.str);
    return nullptr;
  }
  if (share->is_view && !open_view) {
    my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str, share->table_name.str);
    return nullptr;
  }

  share->increment_ref_count();

  if (share->ref_count() == 1 && share->prev) {
    /*
      Share was not used before and it was in the old_unused_share list
      Unlink share from this list
    */
    DBUG_PRINT("info", ("Unlinking from not used list"));
    *share->prev = share->next;
    share->next->prev = share->prev;
    share->next = nullptr;
    share->prev = nullptr;
  }

  /* update access time */
  share->set_last_access_time();

  /* Free cache if too big */
  while (table_def_cache->size() > table_def_size && oldest_unused_share->next)
    table_def_cache->erase(to_string(oldest_unused_share->table_cache_key));

  DBUG_PRINT("exit", ("share: %p ref_count: %u", share, share->ref_count()));
  return share;
}

/**
  Read any existing histogram statistics from the data dictionary and
  store a copy of them in the TABLE_SHARE.

  @param thd Thread handler
  @param share The table share where to store the histograms
  @param schema Schema definition
  @param table_def Table definition

  @retval true on error
  @retval false on success
*/
static bool read_histograms(THD *thd, TABLE_SHARE *share,
                            const dd::Schema *schema,
                            const dd::Abstract_table *table_def) {
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  MDL_request_list mdl_requests;
  for (const auto column : table_def->columns()) {
    if (column->is_se_hidden()) continue;

    MDL_key mdl_key;
    dd::Column_statistics::create_mdl_key(schema->name(), table_def->name(),
                                          column->name(), &mdl_key);

    MDL_request *request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT_BY_KEY(request, &mdl_key, MDL_SHARED_READ, MDL_STATEMENT);
    mdl_requests.push_front(request);
  }

  if (thd->mdl_context.acquire_locks_nsec(
          &mdl_requests, thd->variables.lock_wait_timeout_nsec))
    return true; /* purecov: deadcode */

  for (const auto column : table_def->columns()) {
    if (column->is_se_hidden()) continue;

    const histograms::Histogram *histogram = nullptr;
    if (histograms::find_histogram(thd, schema->name().c_str(),
                                   table_def->name().c_str(),
                                   column->name().c_str(), &histogram)) {
      // Any error is reported by the dictionary subsystem.
      return true; /* purecov: deadcode */
    }

    if (histogram != nullptr) {
      /*
        Make a clone of the histogram so it survives together with the
        TABLE_SHARE in case the original histogram is thrown out of the
        dictionary cache.
      */
      const histograms::Histogram *histogram_copy =
          histogram->clone(&share->mem_root);
      share->m_histograms->emplace(column->ordinal_position() - 1,
                                   histogram_copy);
    }
  }

  return false;
}

/**
  Get the TABLE_SHARE for a table.

  Get a table definition from the table definition cache. If the share
  does not exist, create a new one from the persistently stored table
  definition, and temporarily release LOCK_open while retrieving it.
  Re-lock LOCK_open when the table definition has been retrieved, and
  broadcast this to other threads waiting for the share to become opened.

  If the share exists, and is in the process of being opened, wait for
  opening to complete before continuing.

  @pre  It is a precondition that the caller must own LOCK_open before
        calling this function.

  @note Callers of this function cannot rely on LOCK_open being
        held for the duration of the call. It may be temporarily
        released while the table definition is opened, and it may be
        temporarily released while the thread is waiting for a different
        thread to finish opening it.

  @note After share->m_open_in_progress is set, there should be no wait
        for resources like row- or metadata locks, table flushes, etc.
        Otherwise, we may end up in deadlocks that will not be detected.

  @param thd                thread handle
  @param db                 schema name
  @param table_name         table name
  @param key                table cache key
  @param key_length         length of key
  @param open_view          allow open of view
  @param open_secondary     get the share for a table in a secondary
                            storage engine

  @return Pointer to the new TABLE_SHARE, or NULL if there was an error
*/

TABLE_SHARE *get_table_share(THD *thd, const char *db, const char *table_name,
                             const char *key, size_t key_length, bool open_view,
                             bool open_secondary) {
  TABLE_SHARE *share;
  bool open_table_err = false;
  DBUG_TRACE;

  /* Make sure we own LOCK_open */
  mysql_mutex_assert_owner(&LOCK_open);

  /*
    To be able perform any operation on table we should own
    some kind of metadata lock on it.
  */
  DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, db, table_name, MDL_SHARED));

  /*
    Read table definition from the cache. If the share is being opened,
    wait for the appropriate condition. The share may be destroyed if
    open fails, so after cond_wait, we must repeat searching the
    hash table.
  */
  for (;;) {
    auto it = table_def_cache->find(string(key, key_length));
    if (it == table_def_cache->end()) {
      if (thd->mdl_context.owns_equal_or_stronger_lock(
              MDL_key::SCHEMA, db, "", MDL_INTENTION_EXCLUSIVE)) {
        break;
      }
      mysql_mutex_unlock(&LOCK_open);

      if (dd::mdl_lock_schema(thd, db, MDL_TRANSACTION)) {
        // Lock LOCK_open again to preserve function contract
        mysql_mutex_lock(&LOCK_open);
        return nullptr;
      }

      mysql_mutex_lock(&LOCK_open);
      // Need to re-try the find after getting the mutex again
      continue;
    }
    share = it->second.get();
    if (!share->m_open_in_progress)
      return process_found_table_share(thd, share, open_view);

    DEBUG_SYNC(thd, "get_share_before_COND_open_wait");
    mysql_cond_wait(&COND_open, &LOCK_open);
  }

  /*
    If alloc fails, the share object will not be present in the TDC, so no
    thread will be waiting for m_open_in_progress. Hence, a broadcast is
    not necessary.
  */
  if (!(share = alloc_table_share(db, table_name, key, key_length,
                                  open_secondary))) {
    return nullptr;
  }

  /*
    We assign a new table id under the protection of LOCK_open.
    We do this instead of creating a new mutex
    and using it for the sole purpose of serializing accesses to a
    static variable, we assign the table id here. We assign it to the
    share before inserting it into the table_def_cache to be really
    sure that it cannot be read from the cache without having a table
    id assigned.

    CAVEAT. This means that the table cannot be used for
    binlogging/replication purposes, unless get_table_share() has been
    called directly or indirectly.
  */
  assign_new_table_id(share);

  table_def_cache->emplace(to_string(share->table_cache_key),
                           unique_ptr<TABLE_SHARE, Table_share_deleter>(share));

  /*
    We must increase ref_count prior to releasing LOCK_open
    to keep the share from being deleted in tdc_remove_table()
    and TABLE_SHARE::wait_for_old_version. We must also set
    m_open_in_progress to indicate allocated but incomplete share.
  */
  share->increment_ref_count();      // Mark in use
  share->m_open_in_progress = true;  // Mark being opened

  /*
    Temporarily release LOCK_open before opening the table definition,
    which can be done without mutex protection.
  */
  mysql_mutex_unlock(&LOCK_open);

#if defined(ENABLED_DEBUG_SYNC)
  if (!thd->is_attachable_ro_transaction_active())
    DEBUG_SYNC(thd, "get_share_before_open");
#endif

  {
    // We must make sure the schema is released and unlocked in the right order.
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Schema *sch = nullptr;
    const dd::Abstract_table *abstract_table = nullptr;
    open_table_err = true;  // Assume error to simplify code below.
    if (thd->dd_client()->acquire(share->db.str, &sch) ||
        thd->dd_client()->acquire(share->db.str, share->table_name.str,
                                  &abstract_table)) {
    } else if (sch == nullptr)
      my_error(ER_BAD_DB_ERROR, MYF(0), share->db.str);
    else if (abstract_table == nullptr)
      my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str, share->table_name.str);
    else if (abstract_table->type() == dd::enum_table_type::USER_VIEW ||
             abstract_table->type() == dd::enum_table_type::SYSTEM_VIEW) {
      if (!open_view)  // We found a view but were trying to open table only.
        my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str,
                 share->table_name.str);
      else {
        /*
          Clone the view reference object and hold it in
          TABLE_SHARE member view_object.
        */
        share->is_view = true;
        const dd::View *tmp_view =
            dynamic_cast<const dd::View *>(abstract_table);
        share->view_object = tmp_view->clone();

        share->table_category =
            get_table_category(share->db, share->table_name);
        thd->status_var.opened_shares++;
        open_table_err = false;
      }
    } else {
      DBUG_ASSERT(abstract_table->type() == dd::enum_table_type::BASE_TABLE);
      open_table_err = open_table_def(
          thd, share, *dynamic_cast<const dd::Table *>(abstract_table));

      /*
        Read any existing histogram statistics from the data dictionary and
        store a copy of them in the TABLE_SHARE.

        We need to do this outside the protection of LOCK_open, since the data
        dictionary might have to open tables in order to read histogram data
        (such recursion will not work).
      */
      if (!open_table_err && read_histograms(thd, share, sch, abstract_table))
        open_table_err = true; /* purecov: deadcode */
    }
  }

  /*
    Get back LOCK_open before continuing. Notify all waiters that the
    opening is finished, even if there was a failure while opening.
  */
  mysql_mutex_lock(&LOCK_open);
  share->m_open_in_progress = false;
  mysql_cond_broadcast(&COND_open);

  /*
    Fake an open_table_def error in debug build, resulting in
    ER_NO_SUCH_TABLE.
  */
  DBUG_EXECUTE_IF("set_open_table_err", {
    open_table_err = true;
    my_error(ER_NO_SUCH_TABLE, MYF(0), share->db.str, share->table_name.str);
  });

  /*
    If there was an error while opening the definition, delete the
    share from the TDC, and (implicitly) destroy the share. Waiters
    will detect that the share is gone, and repeat the attempt at
    opening the table definition. The ref counter must be stepped
    down to allow the share to be destroyed.
  */
  if (open_table_err) {
    share->error = true;  // Allow waiters to detect the error
    share->decrement_ref_count();
    table_def_cache->erase(to_string(share->table_cache_key));
#if defined(ENABLED_DEBUG_SYNC)
    if (!thd->is_attachable_ro_transaction_active())
      DEBUG_SYNC(thd, "get_share_after_destroy");
#endif
    return nullptr;
  }

#ifdef HAVE_PSI_TABLE_INTERFACE
  share->m_psi = PSI_TABLE_CALL(get_table_share)(
      (share->tmp_table != NO_TMP_TABLE), share);
#else
  share->m_psi = NULL;
#endif

  /* update access time */
  share->set_last_access_time();

  DBUG_PRINT("exit", ("share: %p  ref_count: %u", share, share->ref_count()));

  /* If debug, assert that the share is actually present in the cache */
#ifndef DBUG_OFF
  DBUG_ASSERT(table_def_cache->count(string(key, key_length)) != 0);
#endif
  return share;
}

/**
  Get a table share. If it didn't exist, try creating it from engine

  For arguments and return values, see get_table_share()
*/

static TABLE_SHARE *get_table_share_with_discover(
    THD *thd, TABLE_LIST *table_list, const char *key, size_t key_length,
    bool open_secondary, int *error)

{
  TABLE_SHARE *share;
  bool exists;
  DBUG_TRACE;

  share = get_table_share(thd, table_list->db, table_list->table_name, key,
                          key_length, true, open_secondary);
  /*
    If share is not NULL, we found an existing share.

    If share is NULL, and there is no error, we're inside
    pre-locking, which silences 'ER_NO_SUCH_TABLE' errors
    with the intention to silently drop non-existing tables
    from the pre-locking list. In this case we still need to try
    auto-discover before returning a NULL share.

    Or, we're inside SHOW CREATE VIEW, which
    also installs a silencer for ER_NO_SUCH_TABLE error.

    If share is NULL and the error is ER_NO_SUCH_TABLE, this is
    the same as above, only that the error was not silenced by
    pre-locking or SHOW CREATE VIEW.

    In both these cases it won't harm to try to discover the
    table.

    Finally, if share is still NULL, it's a real error and we need
    to abort.

    @todo Rework alternative ways to deal with ER_NO_SUCH TABLE.
  */
  if (share || (thd->is_error() &&
                thd->get_stmt_da()->mysql_errno() != ER_NO_SUCH_TABLE)) {
    return share;
  }

  *error = 0;

  /* Table didn't exist. Check if some engine can provide it */
  if (ha_check_if_table_exists(thd, table_list->db, table_list->table_name,
                               &exists)) {
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    /* Conventionally, the storage engine API does not report errors. */
    my_error(ER_OUT_OF_RESOURCES, MYF(0));
  } else if (!exists) {
    /*
      No such table in any engine.
      Hide "Table doesn't exist" errors if the table belongs to a view.
      The check for thd->is_error() is necessary to not push an
      unwanted error in case the error was already silenced.
      @todo Rework the alternative ways to deal with ER_NO_SUCH TABLE.
    */
    if (thd->is_error()) {
      if (table_list->parent_l) {
        thd->clear_error();
        thd->get_stmt_da()->reset_condition_info(thd);
        my_error(ER_WRONG_MRG_TABLE, MYF(0));
      } else if (table_list->belong_to_view) {
        // Mention the top view in message, to not reveal underlying views.
        TABLE_LIST *view = table_list->belong_to_view;
        thd->clear_error();
        thd->get_stmt_da()->reset_condition_info(thd);
        my_error(ER_VIEW_INVALID, MYF(0), view->view_db.str,
                 view->view_name.str);
      }
    }
  } else {
    thd->clear_error();
    thd->get_stmt_da()->reset_condition_info(thd);
    *error = 7; /* Run auto-discover. */
  }
  return nullptr;
}

/**
  Mark that we are not using table share anymore.

  @param  share   Table share

  If the share has no open tables and (we have done a refresh or
  if we have already too many open table shares) then delete the
  definition.
*/

void release_table_share(TABLE_SHARE *share) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("share: %p  table: %s.%s  ref_count: %u  version: %lu",
                       share, share->db.str, share->table_name.str,
                       share->ref_count(), share->version()));

  mysql_mutex_assert_owner(&LOCK_open);

  DBUG_ASSERT(share->ref_count() != 0);
  if (share->decrement_ref_count() == 0) {
    if (share->has_old_version() || table_def_shutdown_in_progress)
      table_def_cache->erase(to_string(share->table_cache_key));
    else {
      /* Link share last in used_table_share list */
      DBUG_PRINT("info", ("moving share to unused list"));

      DBUG_ASSERT(share->next == nullptr);
      share->prev = end_of_unused_share.prev;
      *end_of_unused_share.prev = share;
      end_of_unused_share.prev = &share->next;
      share->next = &end_of_unused_share;

      if (table_def_cache->size() > table_def_size) {
        /* Delete the least used share to preserve LRU order. */
        table_def_cache->erase(to_string(oldest_unused_share->table_cache_key));
      }
    }
  }
}

/**
  Get an existing table definition from the table definition cache.

  Search the table definition cache for a share with the given key.
  If the share exists or if it is in the process of being opened
  by another thread (m_open_in_progress flag is true) return share.
  Do not wait for share opening to finish.

  @param db         database name.
  @param table_name table name.

  @retval NULL      a share for the table does not exist in the cache
  @retval != NULL   pointer to existing share in the cache
*/

static TABLE_SHARE *get_cached_table_share(const char *db,
                                           const char *table_name) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length;
  mysql_mutex_assert_owner(&LOCK_open);

  key_length = create_table_def_key(db, table_name, key);
  return find_or_nullptr(*table_def_cache, string(key, key_length));
}

/*
  Create a list for all open tables matching SQL expression

  SYNOPSIS
    list_open_tables()
    thd			Thread THD
    wild		SQL like expression

  NOTES
    One gets only a list of tables for which one has any kind of privilege.
    db and table names are allocated in result struct, so one doesn't need
    a lock on LOCK_open when traversing the return list.

  RETURN VALUES
    NULL	Error (Probably OOM)
    #		Pointer to list of names of open tables.
*/

OPEN_TABLE_LIST *list_open_tables(THD *thd, const char *db, const char *wild) {
  OPEN_TABLE_LIST **start_list, *open_list, *start, *prev;
  TABLE_LIST table_list;
  DBUG_TRACE;

  start_list = &open_list;
  open_list = nullptr;

  /*
    This is done in two parts:
    1. First, we will make OPEN_TABLE_LIST under LOCK_open
    2. Second, we will check permission and unlink OPEN_TABLE_LIST
       entries if permission check fails
  */

  table_cache_manager.lock_all_and_tdc();

  for (const auto &key_and_value : *table_def_cache) {
    TABLE_SHARE *share = key_and_value.second.get();

    /* Skip shares that are being opened */
    if (share->m_open_in_progress) continue;
    if (db && my_strcasecmp(system_charset_info, db, share->db.str)) continue;
    if (wild && wild_compare(share->table_name.str, share->table_name.length,
                             wild, strlen(wild), false))
      continue;

    if (!(*start_list = (OPEN_TABLE_LIST *)(*THR_MALLOC)
                            ->Alloc(sizeof(**start_list) +
                                    share->table_cache_key.length))) {
      open_list = nullptr;  // Out of memory
      break;
    }
    my_stpcpy((*start_list)->table =
                  my_stpcpy(((*start_list)->db = (char *)((*start_list) + 1)),
                            share->db.str) +
                  1,
              share->table_name.str);
    (*start_list)->in_use = 0;
    Table_cache_iterator it(share);
    while (it++) ++(*start_list)->in_use;
    (*start_list)->locked = 0; /* Obsolete. */
    start_list = &(*start_list)->next;
    *start_list = nullptr;
  }
  table_cache_manager.unlock_all_and_tdc();

  start = open_list;
  prev = start;

  while (start) {
    /* Check if user has SELECT privilege for any column in the table */
    table_list.db = start->db;
    table_list.table_name = start->table;
    table_list.grant.privilege = 0;

    if (check_table_access(thd, SELECT_ACL, &table_list, true, 1, true)) {
      /* Unlink OPEN_TABLE_LIST */
      if (start == open_list) {
        open_list = start->next;
        prev = open_list;
      } else
        prev->next = start->next;
    } else
      prev = start;
    start = start->next;
  }

  return open_list;
}

/*****************************************************************************
 *	 Functions to free open table cache
 ****************************************************************************/

void intern_close_table(TABLE *table) {  // Free all structures
  DBUG_TRACE;
  DBUG_PRINT("tcache",
             ("table: '%s'.'%s' %p", table->s ? table->s->db.str : "?",
              table->s ? table->s->table_name.str : "?", table));

  free_io_cache(table);
  destroy(table->triggers);
  if (table->file)                // Not true if placeholder
    (void)closefrm(table, true);  // close file
  destroy(table);
  my_free(table);
}

/* Free resources allocated by filesort() and read_record() */

void free_io_cache(TABLE *table) {
  DBUG_TRACE;
  if (table->unique_result.io_cache) {
    close_cached_file(table->unique_result.io_cache);
    my_free(table->unique_result.io_cache);
    table->unique_result.io_cache = nullptr;
  }
}

/*
  Close all tables which aren't in use by any thread

  @param thd Thread context
  @param tables List of tables to remove from the cache
  @param wait_for_refresh Wait for a impending flush
  @param timeout Timeout for waiting for flush to be completed.

  @note THD can be NULL, but then wait_for_refresh must be false
        and tables must be NULL.

  @note When called as part of FLUSH TABLES WITH READ LOCK this function
        ignores metadata locks held by other threads. In order to avoid
        situation when FLUSH TABLES WITH READ LOCK sneaks in at the moment
        when some write-locked table is being reopened (by FLUSH TABLES or
        ALTER TABLE) we have to rely on additional global shared metadata
        lock taken by thread trying to obtain global read lock.
*/

bool close_cached_tables_nsec(THD *thd, TABLE_LIST *tables,
                              bool wait_for_refresh, ulonglong timeout_nsec) {
  bool result = false;
  bool found = true;
  struct timespec abstime;
  DBUG_TRACE;
  DBUG_ASSERT(thd || (!wait_for_refresh && !tables));

  table_cache_manager.lock_all_and_tdc();
  if (!tables) {
    /*
      Force close of all open tables.

      Note that code in TABLE_SHARE::wait_for_old_version() assumes that
      incrementing of refresh_version and removal of unused tables and
      shares from TDC happens atomically under protection of LOCK_open,
      or putting it another way that TDC does not contain old shares
      which don't have any tables used.
    */
    refresh_version++;
    DBUG_PRINT("tcache",
               ("incremented global refresh_version to: %lu", refresh_version));

    /*
      Get rid of all unused TABLE and TABLE_SHARE instances. By doing
      this we automatically close all tables which were marked as "old".
    */
    table_cache_manager.free_all_unused_tables();
    /* Free table shares which were not freed implicitly by loop above. */
    while (oldest_unused_share->next)
      table_def_cache->erase(to_string(oldest_unused_share->table_cache_key));
  } else {
    bool share_found = false;
    for (TABLE_LIST *table = tables; table; table = table->next_local) {
      TABLE_SHARE *share = get_cached_table_share(table->db, table->table_name);

      if (share) {
        /*
          tdc_remove_table() also sets TABLE_SHARE::version to 0. Note that
          it will work correctly even if m_open_in_progress flag is true.
        */
        tdc_remove_table(thd, TDC_RT_REMOVE_UNUSED, table->db,
                         table->table_name, true);
        share_found = true;
      }
    }
    if (!share_found) wait_for_refresh = false;  // Nothing to wait for
  }

  table_cache_manager.unlock_all_and_tdc();

  if (!wait_for_refresh) return result;

  set_timespec_nsec(&abstime, timeout_nsec);

  if (thd->locked_tables_mode) {
    /*
      If we are under LOCK TABLES, we need to reopen the tables without
      opening a door for any concurrent threads to sneak in and get
      lock on our tables. To achieve this we use exclusive metadata
      locks.
    */
    TABLE_LIST *tables_to_reopen =
        (tables ? tables : thd->locked_tables_list.locked_tables());

    /* Close open HANLER instances to avoid self-deadlock. */
    mysql_ha_flush_tables(thd, tables_to_reopen);

    for (TABLE_LIST *table_list = tables_to_reopen; table_list;
         table_list = table_list->next_global) {
      /* A check that the table was locked for write is done by the caller. */
      TABLE *table = find_table_for_mdl_upgrade(thd, table_list->db,
                                                table_list->table_name, true);

      /* May return NULL if this table has already been closed via an alias. */
      if (!table) continue;

      if (wait_while_table_is_used(thd, table, HA_EXTRA_FORCE_REOPEN)) {
        result = true;
        goto err_with_reopen;
      }
      close_all_tables_for_name(thd, table->s, false, nullptr);
    }
  }

  /* Wait until all threads have closed all the tables we are flushing. */
  DBUG_PRINT("info", ("Waiting for other threads to close their open tables"));

  while (found && !thd->killed) {
    TABLE_SHARE *share = nullptr;
    found = false;
    /*
      To a self-deadlock or deadlocks with other FLUSH threads
      waiting on our open HANDLERs, we have to flush them.
    */
    mysql_ha_flush(thd);
    DEBUG_SYNC(thd, "after_flush_unlock");

    mysql_mutex_lock(&LOCK_open);

    if (!tables) {
      for (const auto &key_and_value : *table_def_cache) {
        share = key_and_value.second.get();
        if (share->has_old_version()) {
          found = true;
          break;
        }
      }
    } else {
      for (TABLE_LIST *table = tables; table; table = table->next_local) {
        share = get_cached_table_share(table->db, table->table_name);
        if (share && share->has_old_version()) {
          found = true;
          break;
        }
      }
    }

    if (found) {
      /*
        The method below temporarily unlocks LOCK_open and frees
        share's memory. Note that it works correctly even for
        shares with m_open_in_progress flag set.
      */
      if (share->wait_for_old_version(
              thd, &abstime, MDL_wait_for_subgraph::DEADLOCK_WEIGHT_DDL)) {
        mysql_mutex_unlock(&LOCK_open);
        result = true;
        goto err_with_reopen;
      }
    }

    mysql_mutex_unlock(&LOCK_open);
  }

err_with_reopen:
  if (thd->locked_tables_mode) {
    /*
      No other thread has the locked tables open; reopen them and get the
      old locks. This should succeed unless any dictionary operations fail
      (e.g. when opening a dictionary table on cache miss).
    */
    result |= thd->locked_tables_list.reopen_tables(thd);
    /*
      Since downgrade_lock() won't do anything with shared
      metadata lock it is much simpler to go through all open tables rather
      than picking only those tables that were flushed.
    */
    for (TABLE *tab = thd->open_tables; tab; tab = tab->next)
      tab->mdl_ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
  }
  return result || thd->killed;
}

/**
  Mark all temporary tables which were used by the current statement or
  substatement as free for reuse, but only if the query_id can be cleared.

  @param thd thread context

  @remark For temp tables associated with a open SQL HANDLER the query_id
          is not reset until the HANDLER is closed.
*/

static void mark_temp_tables_as_free_for_reuse(THD *thd) {
  for (TABLE *table = thd->temporary_tables; table; table = table->next) {
    if ((table->query_id == thd->query_id) && !table->open_by_handler) {
      mark_tmp_table_for_reuse(table);
      table->cleanup_value_generator_items();
      table->cleanup_partial_update();
    }
  }
}

/**
  Reset a single temporary table.
  Effectively this "closes" one temporary table,
  in a session.

  @param table     Temporary table.
*/

void mark_tmp_table_for_reuse(TABLE *table) {
  DBUG_ASSERT(table->s->tmp_table);

  table->query_id = 0;
  table->file->ha_reset();

  /* Detach temporary MERGE children from temporary parent. */
  DBUG_ASSERT(table->file);
  table->file->ha_extra(HA_EXTRA_DETACH_CHILDREN);

  /*
    Reset temporary table lock type to it's default value (TL_WRITE).

    Statements such as INSERT INTO .. SELECT FROM tmp, CREATE TABLE
    .. SELECT FROM tmp and UPDATE may under some circumstances modify
    the lock type of the tables participating in the statement. This
    isn't a problem for non-temporary tables since their lock type is
    reset at every open, but the same does not occur for temporary
    tables for historical reasons.

    Furthermore, the lock type of temporary tables is not really that
    important because they can only be used by one query at a time and
    not even twice in a query -- a temporary table is represented by
    only one TABLE object. Nonetheless, it's safer from a maintenance
    point of view to reset the lock type of this singleton TABLE object
    as to not cause problems when the table is reused.

    Even under LOCK TABLES mode its okay to reset the lock type as
    LOCK TABLES is allowed (but ignored) for a temporary table.
  */
  table->reginfo.lock_type = TL_WRITE;
}

/*
  Mark all tables in the list which were used by current substatement
  as free for reuse.

  SYNOPSIS
    mark_used_tables_as_free_for_reuse()
      thd   - thread context
      table - head of the list of tables

  DESCRIPTION
    Marks all tables in the list which were used by current substatement
    (they are marked by its query_id) as free for reuse.

  NOTE
    The reason we reset query_id is that it's not enough to just test
    if table->query_id != thd->query_id to know if a table is in use.

    For example
    SELECT f1_that_uses_t1() FROM t1;
    In f1_that_uses_t1() we will see one instance of t1 where query_id is
    set to query_id of original query.
*/

static void mark_used_tables_as_free_for_reuse(THD *thd, TABLE *table) {
  for (; table; table = table->next) {
    DBUG_ASSERT(table->pos_in_locked_tables == nullptr ||
                table->pos_in_locked_tables->table == table);
    if (table->query_id == thd->query_id) {
      table->query_id = 0;
      table->file->ha_reset();
    }
  }
}

/**
  Auxiliary function to close all tables in the open_tables list.

  @param thd Thread context.

  @remark It should not ordinarily be called directly.
*/

static void close_open_tables(THD *thd) {
  mysql_mutex_assert_not_owner(&LOCK_open);

  DBUG_PRINT("info", ("thd->open_tables: %p", thd->open_tables));

  while (thd->open_tables) close_thread_table(thd, &thd->open_tables);
}

/**
  Close all open instances of the table but keep the MDL lock.

  Works both under LOCK TABLES and in the normal mode.
  Removes all closed instances of the table from the table cache.

  @param  thd         Thread context.
  @param  key         TC/TDC key identifying the table.
  @param  key_length  Length of TC/TDC key identifying the table.
  @param  db          Database name.
  @param  table_name  Table name.
  @param  remove_from_locked_tables
                      True if the table is being dropped.
                      In that case the documented behaviour is to
                      implicitly remove the table from LOCK TABLES list.
  @param  skip_table  TABLE instance that should be kept open.

  @pre Must be called with an X MDL lock on the table.
*/
static void close_all_tables_for_name(THD *thd, const char *key,
                                      size_t key_length, const char *db,
                                      const char *table_name,
                                      bool remove_from_locked_tables,
                                      TABLE *skip_table) {
  mysql_mutex_assert_not_owner(&LOCK_open);
  for (TABLE **prev = &thd->open_tables; *prev;) {
    TABLE *table = *prev;

    if (table->s->table_cache_key.length == key_length &&
        !memcmp(table->s->table_cache_key.str, key, key_length) &&
        table != skip_table) {
      thd->locked_tables_list.unlink_from_list(thd, table->pos_in_locked_tables,
                                               remove_from_locked_tables);
      /*
        Does nothing if the table is not locked.
        This allows one to use this function after a table
        has been unlocked, e.g. in partition management.
      */
      mysql_lock_remove(thd, thd->lock, table);

      /* Inform handler that table will be dropped after close */
      if (table->db_stat && /* Not true for partitioned tables. */
          skip_table == nullptr)
        table->file->ha_extra(HA_EXTRA_PREPARE_FOR_DROP);
      close_thread_table(thd, prev);
    } else {
      /* Step to next entry in open_tables list. */
      prev = &table->next;
    }
  }
  if (skip_table == nullptr) {
    /* Remove the table share from the cache. */
    tdc_remove_table(thd, TDC_RT_REMOVE_ALL, db, table_name, false);
  }
}

void close_all_tables_for_name(THD *thd, TABLE_SHARE *share,
                               bool remove_from_locked_tables,
                               TABLE *skip_table) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length = share->table_cache_key.length;

  memcpy(key, share->table_cache_key.str, key_length);

  close_all_tables_for_name(thd, key, key_length,
                            key,                         // db
                            key + share->db.length + 1,  // table_name
                            remove_from_locked_tables, skip_table);
}

void close_all_tables_for_name(THD *thd, const char *db, const char *table_name,
                               bool remove_from_locked_tables) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length = create_table_def_key(db, table_name, key);

  close_all_tables_for_name(thd, key, key_length, db, table_name,
                            remove_from_locked_tables, nullptr);
}

// Check if we are under LOCK TABLE mode, and not prelocking.
static inline bool in_LTM(THD *thd) {
  return (thd->locked_tables_mode == LTM_LOCK_TABLES ||
          thd->locked_tables_mode == LTM_PRELOCKED_UNDER_LOCK_TABLES);
}

/**
  Check if the given TABLE_LIST belongs to a DD table.

  The function checks whether the table is a DD table being used in the
  context of a DD transaction, or whether it is referred by a system view.
  Then, it implies that if either of these two conditions hold, then this
  is a DD table. If in case this is a DD table being used in some other
  situation, then this function does not return 'true'. We do not know if
  there is such a situation right now.

  This function ignores TABLE_LIST's that is created by optimizer
  when processing a system view.

  @param    tl             TABLE_LIST point to the table.

  @retval   true           If table belongs to a DD table.
  @retval   false          If table does not.
*/
static bool belongs_to_dd_table(const TABLE_LIST *tl) {
  return (tl->is_dd_ctx_table ||
          (!tl->is_internal() && !tl->uses_materialization() &&
           tl->referencing_view && tl->referencing_view->is_system_view));
}

/**
  Performance Schema tables must be accessible independently of the LOCK TABLE
  mode. These macros handle the special case of P_S tables being used under
  LOCK TABLE mode.
  Check if the table belongs to the P_S, excluding setup and threads tables.
*/
static inline bool belongs_to_p_s(TABLE_LIST *tl) {
  return (!strcmp("performance_schema", tl->db) &&
          strcmp(tl->table_name, "threads") &&
          strstr(tl->table_name, "setup_") == nullptr);
}

/*
  Close all tables used by the current substatement, or all tables
  used by this thread if we are on the upper level.

  SYNOPSIS
    close_thread_tables()
    thd			Thread handler

  IMPLEMENTATION
    Unlocks tables and frees derived tables.
    Put all normal tables used by thread in free list.

    It will only close/mark as free for reuse tables opened by this
    substatement, it will also check if we are closing tables after
    execution of complete query (i.e. we are on upper level) and will
    leave prelocked mode if needed.
*/

void close_thread_tables(THD *thd) {
  DBUG_TRACE;

#ifdef EXTRA_DEBUG
  DBUG_PRINT("tcache", ("open tables:"));
  for (TABLE *table = thd->open_tables; table; table = table->next)
    DBUG_PRINT("tcache", ("table: '%s'.'%s' %p", table->s->db.str,
                          table->s->table_name.str, table));
#endif

#if defined(ENABLED_DEBUG_SYNC)
  /* debug_sync may not be initialized for some slave threads */
  if (thd->debug_sync_control) DEBUG_SYNC(thd, "before_close_thread_tables");
#endif

  // TODO: dd::Transaction_impl::end() does merge DD transaction into
  // thd->transaction.stmt. Later the can be second DD transaction
  // which would call close_thread_tables(). In this case, the
  // condition thd->transaction.stmt.is_empty() does not hold good.
  // So we comment this assert for now.
  //
  // We should consider retaining this assert if we plan to commit
  // DD RW transaction just before next close_thread_tables().
  // We are not sure if this is doable and needs to be explored.
  // Alik and myself plan to comment this assert for now temporarily
  // and address this TODO asap.
  //
  // DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT) ||
  //            thd->in_sub_stmt ||
  //            (thd->state_flags & Open_tables_state::BACKUPS_AVAIL));

  /* Detach MERGE children after every statement. Even under LOCK TABLES. */
  for (TABLE *table = thd->open_tables; table; table = table->next) {
    /* Table might be in use by some outer statement. */
    DBUG_PRINT("tcache", ("table: '%s'  query_id: %lu",
                          table->s->table_name.str, (ulong)table->query_id));
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES ||
        table->query_id == thd->query_id) {
      DBUG_ASSERT(table->file);
      if (table->db_stat) table->file->ha_extra(HA_EXTRA_DETACH_CHILDREN);
      table->cleanup_value_generator_items();
      table->cleanup_partial_update();
    }
  }

  /*
    Mark all temporary tables used by this statement as free for reuse.
  */
  mark_temp_tables_as_free_for_reuse(thd);

  if (thd->locked_tables_mode) {
    /*
      If we have
      1) Implicitly opened some DD tables that belong to IS system
         view executed in LOCK TABLE mode, then we should close them now.
      2) Close P_S tables opened implicitly under LOCK TABLE mode.
    */
    if (in_LTM(thd)) {
      for (TABLE **prev = &thd->open_tables; *prev;) {
        TABLE *table = *prev;

        /* Ignore tables locked explicitly by LOCK TABLE. */
        if (!table->pos_in_locked_tables) {
          /*
            We close tables only when all of following conditions satisfy,
            - The table is not locked explicitly by user using LOCK TABLE
            command.
            - We are not executing a IS queries as part of SF/Trigger.
            - The table belongs to a new DD table.
            OR
            - Close P_S tables unless the query is inside of a SP/trigger.
          */
          TABLE_LIST *tbl_list = table->pos_in_table_list;
          if (!thd->in_sub_stmt && (belongs_to_dd_table(tbl_list) ||
                                    belongs_to_p_s(table->pos_in_table_list))) {
            if (!table->s->tmp_table) {
              table->file->ha_index_or_rnd_end();
              table->set_keyread(false);
              table->open_by_handler = false;
              /*
                In case we have opened the DD table but the statement
                fails before calling ha_external_lock() requesting
                read lock in open_tables(), then we need to check
                if we have really requested lock and then unlock.
               */
              if (table->file->get_lock_type() != F_UNLCK)
                table->file->ha_external_lock(thd, F_UNLCK);
              close_thread_table(thd, prev);
              continue;
            }
          }
        }
        prev = &table->next;
      }  // End of for
    }

    /* Ensure we are calling ha_reset() for all used tables */
    mark_used_tables_as_free_for_reuse(thd, thd->open_tables);

    /*
      Mark this statement as one that has "unlocked" its tables.
      For purposes of Query_tables_list::lock_tables_state we treat
      any statement which passed through close_thread_tables() as
      such.
    */
    thd->lex->lock_tables_state = Query_tables_list::LTS_NOT_LOCKED;

    /*
      We are under simple LOCK TABLES or we're inside a sub-statement
      of a prelocked statement, so should not do anything else.

      Note that even if we are in LTM_LOCK_TABLES mode and statement
      requires prelocking (e.g. when we are closing tables after
      failing ot "open" all tables required for statement execution)
      we will exit this function a few lines below.
    */
    if (!thd->lex->requires_prelocking()) return;

    /*
      We are in the top-level statement of a prelocked statement,
      so we have to leave the prelocked mode now with doing implicit
      UNLOCK TABLES if needed.
    */
    if (thd->locked_tables_mode == LTM_PRELOCKED_UNDER_LOCK_TABLES)
      thd->locked_tables_mode = LTM_LOCK_TABLES;

    if (thd->locked_tables_mode == LTM_LOCK_TABLES) return;

    thd->leave_locked_tables_mode();

    /* Fallthrough */
  }

  if (thd->lock) {
    /*
      For RBR we flush the pending event just before we unlock all the
      tables.  This means that we are at the end of a topmost
      statement, so we ensure that the STMT_END_F flag is set on the
      pending event.  For statements that are *inside* stored
      functions, the pending event will not be flushed: that will be
      handled either before writing a query log event (inside
      binlog_query()) or when preparing a pending event.
     */
    (void)thd->binlog_flush_pending_rows_event(true);
    mysql_unlock_tables(thd, thd->lock);
    thd->lock = nullptr;
  }

  thd->lex->lock_tables_state = Query_tables_list::LTS_NOT_LOCKED;

  /*
    Closing a MERGE child before the parent would be fatal if the
    other thread tries to abort the MERGE lock in between.
  */
  if (thd->open_tables) close_open_tables(thd);
}

/**
  Helper function which returns TABLE to Table Cache or closes if
  table is marked as needing re-open.
*/
static void release_or_close_table(THD *thd, TABLE *table) {
  Table_cache *tc = table_cache_manager.get_cache(thd);

  tc->lock();

  if (table->s->has_old_version() || table->needs_reopen() ||
      table_def_shutdown_in_progress) {
    tc->remove_table(table);
    mysql_mutex_lock(&LOCK_open);
    intern_close_table(table);
    mysql_mutex_unlock(&LOCK_open);
  } else
    tc->release_table(thd, table);

  tc->unlock();
}

/* move one table to free list */

void close_thread_table(THD *thd, TABLE **table_ptr) {
  TABLE *table = *table_ptr;
  DBUG_TRACE;
  DBUG_ASSERT(table->key_read == 0);
  DBUG_ASSERT(!table->file || table->file->inited == handler::NONE);
  mysql_mutex_assert_not_owner(&LOCK_open);
  /*
    The metadata lock must be released after giving back
    the table to the table cache.
  */
  DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, table->s->db.str, table->s->table_name.str, MDL_SHARED));
  table->mdl_ticket = nullptr;
  table->pos_in_table_list = nullptr;

  mysql_mutex_lock(&thd->LOCK_thd_data);
  *table_ptr = table->next;
  mysql_mutex_unlock(&thd->LOCK_thd_data);

  if (!table->needs_reopen()) {
    /* Avoid having MERGE tables with attached children in unused_tables. */
    table->file->ha_extra(HA_EXTRA_DETACH_CHILDREN);
    /* Free memory and reset for next loop. */
    free_blob_buffers_and_reset(table, MAX_TDC_BLOB_SIZE);
    table->file->ha_reset();
  }

  /* Do this *before* entering the LOCK_open critical section. */
  if (table->file != nullptr) table->file->unbind_psi();

  release_or_close_table(thd, table);
}

/* close_temporary_tables' internal, 4 is due to uint4korr definition */
static inline uint tmpkeyval(TABLE *table) {
  return uint4korr(table->s->table_cache_key.str +
                   table->s->table_cache_key.length - 4);
}

/*
  Close all temporary tables created by 'CREATE TEMPORARY TABLE' for thread
  creates one DROP TEMPORARY TABLE binlog event for each pseudo-thread.

  TODO: In future, we should have temporary_table= 0 and
        slave_open_temp_tables.fetch_add() at one place instead of repeating
        it all across the function. An alternative would be to use
        close_temporary_table() instead of close_temporary() that maintains
        the correct invariant regarding empty list of temporary tables
        and zero slave_open_temp_tables already.
*/

bool close_temporary_tables(THD *thd) {
  DBUG_TRACE;
  TABLE *table;
  TABLE *next = nullptr;
  TABLE *prev_table;
  /* Assume thd->variables.option_bits has OPTION_QUOTE_SHOW_CREATE */
  bool was_quote_show = true;
  bool error = false;
  int slave_closed_temp_tables = 0;

  if (!thd->temporary_tables) return false;

  DBUG_ASSERT(!thd->slave_thread ||
              thd->system_thread != SYSTEM_THREAD_SLAVE_WORKER);

  /*
    Ensure we don't have open HANDLERs for tables we are about to close.
    This is necessary when close_temporary_tables() is called as part
    of execution of BINLOG statement (e.g. for format description event).
  */
  mysql_ha_rm_temporary_tables(thd);
  if (!mysql_bin_log.is_open()) {
    TABLE *tmp_next;
    for (TABLE *t = thd->temporary_tables; t; t = tmp_next) {
      tmp_next = t->next;
      mysql_lock_remove(thd, thd->lock, t);
      /*
        We should not meet temporary tables created by ALTER TABLE here.
        It is responsibility of ALTER statement to close them. Otherwise
        it might be necessary to remove them from DD as well.
      */
      DBUG_ASSERT(t->s->tmp_table_def);
      close_temporary(thd, t, true, true);
      slave_closed_temp_tables++;
    }

    thd->temporary_tables = nullptr;
    if (thd->slave_thread) {
      atomic_slave_open_temp_tables -= slave_closed_temp_tables;
      thd->rli_slave->get_c_rli()->atomic_channel_open_temp_tables -=
          slave_closed_temp_tables;
    }

    return false;
  }

  /*
    We are about to generate DROP TEMPORARY TABLE statements for all
    the left out temporary tables. If GTID_NEXT is set (e.g. if user
    did SET GTID_NEXT just before disconnecting the client), we must
    ensure that it will be able to generate GTIDs for the statements
    with this server's UUID. Therefore we set gtid_next to
    AUTOMATIC_GTID.
  */
  gtid_state->update_on_rollback(thd);
  thd->variables.gtid_next.set_automatic();

  /*
    We must separate transactional temp tables and
    non-transactional temp tables in two distinct DROP statements
    to avoid the splitting if a slave server reads from this binlog.
  */

  /* Better add "if exists", in case a RESET MASTER has been done */
  const char stub[] = "DROP /*!40005 TEMPORARY */ TABLE IF EXISTS ";
  uint stub_len = sizeof(stub) - 1;
  char buf_trans[256], buf_non_trans[256];
  String s_query_trans =
      String(buf_trans, sizeof(buf_trans), system_charset_info);
  String s_query_non_trans =
      String(buf_non_trans, sizeof(buf_non_trans), system_charset_info);
  bool found_user_tables = false;
  bool found_trans_table = false;
  bool found_non_trans_table = false;

  memcpy(buf_trans, stub, stub_len);
  memcpy(buf_non_trans, stub, stub_len);

  /*
    Insertion sort of temp tables by pseudo_thread_id to build ordered list
    of sublists of equal pseudo_thread_id
  */

  for (prev_table = thd->temporary_tables, table = prev_table->next; table;
       prev_table = table, table = table->next) {
    TABLE *prev_sorted /* same as for prev_table */, *sorted;
    /*
      We should not meet temporary tables created by ALTER TABLE here.
      It is responsibility of ALTER statement to close them. Otherwise
      it might be necessary to remove them from DD as well.
    */
    DBUG_ASSERT(table->s->tmp_table_def);
    if (is_user_table(table)) {
      if (!found_user_tables) found_user_tables = true;
      for (prev_sorted = nullptr, sorted = thd->temporary_tables;
           sorted != table; prev_sorted = sorted, sorted = sorted->next) {
        if (!is_user_table(sorted) || tmpkeyval(sorted) > tmpkeyval(table)) {
          /* move into the sorted part of the list from the unsorted */
          prev_table->next = table->next;
          table->next = sorted;
          if (prev_sorted) {
            prev_sorted->next = table;
          } else {
            thd->temporary_tables = table;
          }
          table = prev_table;
          break;
        }
      }
    }
  }

  /* We always quote db,table names though it is slight overkill */
  if (found_user_tables && !(was_quote_show = (thd->variables.option_bits &
                                               OPTION_QUOTE_SHOW_CREATE))) {
    thd->variables.option_bits |= OPTION_QUOTE_SHOW_CREATE;
  }

  /*
    Make LEX consistent with DROP TEMPORARY TABLES statement which we
    are going to log. This is important for the binary logging code.
  */
  LEX *lex = thd->lex;
  enum_sql_command sav_sql_command = lex->sql_command;
  bool sav_drop_temp = lex->drop_temporary;
  lex->sql_command = SQLCOM_DROP_TABLE;
  lex->drop_temporary = true;

  /* scan sorted tmps to generate sequence of DROP */
  for (table = thd->temporary_tables; table; table = next) {
    if (is_user_table(table) && table->should_binlog_drop_if_temp()) {
      bool save_thread_specific_used = thd->thread_specific_used;
      my_thread_id save_pseudo_thread_id = thd->variables.pseudo_thread_id;
      /* Set pseudo_thread_id to be that of the processed table */
      thd->variables.pseudo_thread_id = tmpkeyval(table);
      String db;
      db.append(table->s->db.str);
      /* Loop forward through all tables that belong to a common database
         within the sublist of common pseudo_thread_id to create single
         DROP query
      */
      for (s_query_trans.length(stub_len), s_query_non_trans.length(stub_len),
           found_trans_table = false, found_non_trans_table = false;
           table && is_user_table(table) &&
           tmpkeyval(table) == thd->variables.pseudo_thread_id &&
           table->s->db.length == db.length() &&
           strcmp(table->s->db.str, db.ptr()) == 0;
           table = next) {
        /* Separate transactional from non-transactional temp tables */
        if (table->should_binlog_drop_if_temp()) {
          /* Separate transactional from non-transactional temp tables */
          if (table->s->tmp_table == TRANSACTIONAL_TMP_TABLE) {
            found_trans_table = true;
            /*
              We are going to add ` around the table names and possible more
              due to special characters
            */
            append_identifier(thd, &s_query_trans, table->s->table_name.str,
                              strlen(table->s->table_name.str));
            s_query_trans.append(',');
          } else if (table->s->tmp_table == NON_TRANSACTIONAL_TMP_TABLE) {
            found_non_trans_table = true;
            /*
              We are going to add ` around the table names and possible more
              due to special characters
            */
            append_identifier(thd, &s_query_non_trans, table->s->table_name.str,
                              strlen(table->s->table_name.str));
            s_query_non_trans.append(',');
          }
        }

        next = table->next;
        mysql_lock_remove(thd, thd->lock, table);
        close_temporary(thd, table, true, true);
        slave_closed_temp_tables++;
      }
      thd->clear_error();
      const CHARSET_INFO *cs_save = thd->variables.character_set_client;
      thd->variables.character_set_client = system_charset_info;
      thd->thread_specific_used = true;

      if (found_trans_table) {
        Query_log_event qinfo(thd, s_query_trans.ptr(),
                              s_query_trans.length() - 1, false, true, false,
                              0);
        qinfo.db = db.ptr();
        qinfo.db_len = db.length();
        thd->variables.character_set_client = cs_save;

        thd->get_stmt_da()->set_overwrite_status(true);
        if ((error = (mysql_bin_log.write_event(&qinfo) ||
                      mysql_bin_log.commit(thd, true) || error))) {
          /*
            If we're here following THD::cleanup, thence the connection
            has been closed already. So lets print a message to the
            error log instead of pushing yet another error into the
            Diagnostics_area.

            Also, we keep the error flag so that we propagate the error
            up in the stack. This way, if we're the SQL thread we notice
            that close_temporary_tables failed. (Actually, the SQL
            thread only calls close_temporary_tables while applying old
            Start_log_event_v3 events.)
          */
          LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_WRITE_DROP_FOR_TEMP_TABLES);
        }
        thd->get_stmt_da()->set_overwrite_status(false);
      }

      if (found_non_trans_table) {
        Query_log_event qinfo(thd, s_query_non_trans.ptr(),
                              s_query_non_trans.length() - 1, false, true,
                              false, 0);
        qinfo.db = db.ptr();
        qinfo.db_len = db.length();
        thd->variables.character_set_client = cs_save;

        thd->get_stmt_da()->set_overwrite_status(true);
        if ((error = (mysql_bin_log.write_event(&qinfo) ||
                      mysql_bin_log.commit(thd, true) || error))) {
          /*
            If we're here following THD::cleanup, thence the connection
            has been closed already. So lets print a message to the
            error log instead of pushing yet another error into the
            Diagnostics_area.

            Also, we keep the error flag so that we propagate the error
            up in the stack. This way, if we're the SQL thread we notice
            that close_temporary_tables failed. (Actually, the SQL
            thread only calls close_temporary_tables while applying old
            Start_log_event_v3 events.)
          */
          LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_WRITE_DROP_FOR_TEMP_TABLES);
        }
        thd->get_stmt_da()->set_overwrite_status(false);
      }

      thd->variables.pseudo_thread_id = save_pseudo_thread_id;
      thd->thread_specific_used = save_thread_specific_used;
    } else {
      next = table->next;
      /*
        This is for those cases when we have acquired lock but drop temporary
        table will not be logged.
      */
      mysql_lock_remove(thd, thd->lock, table);
      close_temporary(thd, table, true, true);
      slave_closed_temp_tables++;
    }
  }
  lex->drop_temporary = sav_drop_temp;
  lex->sql_command = sav_sql_command;

  if (!was_quote_show)
    thd->variables.option_bits &=
        ~OPTION_QUOTE_SHOW_CREATE; /* restore option */

  thd->temporary_tables = nullptr;
  if (thd->slave_thread) {
    atomic_slave_open_temp_tables -= slave_closed_temp_tables;
    thd->rli_slave->get_c_rli()->atomic_channel_open_temp_tables -=
        slave_closed_temp_tables;
  }

  return error;
}

/**
  Find table in global list.

  @param table          Pointer to table list
  @param db_name        Data base name
  @param table_name     Table name

  @returns Pointer to found table.
  @retval NULL  Table not found
*/

TABLE_LIST *find_table_in_global_list(TABLE_LIST *table, const char *db_name,
                                      const char *table_name) {
  for (; table; table = table->next_global) {
    if ((table->table == nullptr ||
         table->table->s->tmp_table == NO_TMP_TABLE) &&
        strcmp(table->db, db_name) == 0 &&
        strcmp(table->table_name, table_name) == 0)
      break;
  }
  return table;
}

/**
  Test that table is unique (It's only exists once in the table list)

  @param  table        table to be checked (must be updatable base table)
  @param  table_list   list of tables
  @param  check_alias  whether to check tables' aliases

  NOTE: to exclude derived tables from check we use following mechanism:
    a) during derived table processing set THD::derived_tables_processing
    b) SELECT_LEX::prepare set SELECT::exclude_from_table_unique_test if
       THD::derived_tables_processing set. (we can't use JOIN::execute
       because for PS we perform only SELECT_LEX::prepare, but we can't set
       this flag in SELECT_LEX::prepare if we are not sure that we are in
       derived table processing loop, because multi-update call fix_fields()
       for some its items (which mean SELECT_LEX::prepare for subqueries)
       before unique_table call to detect which tables should be locked for
       write).
    c) find_dup_table skip all tables which belong to SELECT with
       SELECT::exclude_from_table_unique_test set.
    Also SELECT::exclude_from_table_unique_test used to exclude from check
    tables of main SELECT of multi-delete and multi-update

    We also skip tables with TABLE_LIST::prelocking_placeholder set,
    because we want to allow SELECTs from them, and their modification
    will rise the error anyway.

    TODO: when we will have table/view change detection we can do this check
          only once for PS/SP

  @retval !=0  found duplicate
  @retval 0 if table is unique
*/

static TABLE_LIST *find_dup_table(const TABLE_LIST *table,
                                  TABLE_LIST *table_list, bool check_alias) {
  TABLE_LIST *res;
  const char *d_name, *t_name, *t_alias;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table alias: %s", table->alias));

  DBUG_ASSERT(table == table->updatable_base_table());
  /*
    If this function called for CREATE command that we have not opened table
    (table->table equal to 0) and right names is in current TABLE_LIST
    object.
  */
  if (table->table) {
    /* All MyISAMMRG children are plain MyISAM tables. */
    DBUG_ASSERT(table->table->file->ht->db_type != DB_TYPE_MRG_MYISAM);

    /* temporary table is always unique */
    if (table->table->s->tmp_table != NO_TMP_TABLE) return nullptr;
  }

  d_name = table->db;
  t_name = table->table_name;
  t_alias = table->alias;

  DBUG_PRINT("info", ("real table: %s.%s", d_name, t_name));
  for (;;) {
    /*
      Table is unique if it is present only once in the global list
      of tables and once in the list of table locks.
    */
    if (!(res = find_table_in_global_list(table_list, d_name, t_name))) break;

    /* Skip if same underlying table. */
    if (res->table && (res->table == table->table)) goto next;

    /* Skip if table alias does not match. */
    if (check_alias) {
      if (lower_case_table_names
              ? my_strcasecmp(files_charset_info, t_alias, res->alias)
              : strcmp(t_alias, res->alias))
        goto next;
    }

    /*
      Skip if marked to be excluded (could be a derived table) or if
      entry is a prelocking placeholder.
    */
    if (res->select_lex && !res->select_lex->exclude_from_table_unique_test &&
        !res->prelocking_placeholder)
      break;

    /*
      If we found entry of this table or table of SELECT which already
      processed in derived table or top select of multi-update/multi-delete
      (exclude_from_table_unique_test) or prelocking placeholder.
    */
  next:
    table_list = res->next_global;
    DBUG_PRINT("info",
               ("found same copy of table or table which we should skip"));
  }
  return res;
}

/**
  Test that the subject table of INSERT/UPDATE/DELETE/CREATE
  or (in case of MyISAMMRG) one of its children are not used later
  in the query.

  For MyISAMMRG tables, it is assumed that all the underlying
  tables of @c table (if any) are listed right after it and that
  their @c parent_l field points at the main table.

  @param  table      table to be checked (must be updatable base table)
  @param  table_list List of tables to check against
  @param  check_alias whether to check tables' aliases

  @retval non-NULL The table list element for the table that
                   represents the duplicate.
  @retval NULL     No duplicates found.
*/

TABLE_LIST *unique_table(const TABLE_LIST *table, TABLE_LIST *table_list,
                         bool check_alias) {
  DBUG_ASSERT(table == table->updatable_base_table());

  TABLE_LIST *dup;
  if (table->table && table->table->file->ht->db_type == DB_TYPE_MRG_MYISAM) {
    TABLE_LIST *child;
    dup = nullptr;
    /* Check duplicates of all merge children. */
    for (child = table->next_global; child && child->parent_l == table;
         child = child->next_global) {
      if ((dup = find_dup_table(child, child->next_global, check_alias))) break;
    }
  } else
    dup = find_dup_table(table, table_list, check_alias);
  return dup;
}

/**
  Issue correct error message in case we found 2 duplicate tables which
  prevent some update operation

  @param update      table which we try to update
  @param operation   name of update operation
  @param duplicate   duplicate table which we found

  @note here we hide view underlying tables if we have them.
*/

void update_non_unique_table_error(TABLE_LIST *update, const char *operation,
                                   TABLE_LIST *duplicate) {
  update = update->top_table();
  duplicate = duplicate->top_table();
  if (!update->is_view() || !duplicate->is_view() ||
      update->view_query() == duplicate->view_query() ||
      update->view_name.length != duplicate->view_name.length ||
      update->view_db.length != duplicate->view_db.length ||
      my_strcasecmp(table_alias_charset, update->view_name.str,
                    duplicate->view_name.str) != 0 ||
      my_strcasecmp(table_alias_charset, update->view_db.str,
                    duplicate->view_db.str) != 0) {
    /*
      it is not the same view repeated (but it can be parts of the same copy
      of view), so we have to hide underlying tables.
    */
    if (update->is_view()) {
      // Issue the ER_NON_INSERTABLE_TABLE error for an INSERT
      if (duplicate->is_view() &&
          update->view_query() == duplicate->view_query())
        my_error(!strncmp(operation, "INSERT", 6) ? ER_NON_INSERTABLE_TABLE
                                                  : ER_NON_UPDATABLE_TABLE,
                 MYF(0), update->alias, operation);
      else
        my_error(ER_VIEW_PREVENT_UPDATE, MYF(0),
                 (duplicate->is_view() ? duplicate->alias : update->alias),
                 operation, update->alias);
      return;
    }
    if (duplicate->is_view()) {
      my_error(ER_VIEW_PREVENT_UPDATE, MYF(0), duplicate->alias, operation,
               update->alias);
      return;
    }
  }
  my_error(ER_UPDATE_TABLE_USED, MYF(0), update->alias);
}

/**
  Find temporary table specified by database and table names in the
  THD::temporary_tables list.

  @return TABLE instance if a temporary table has been found; NULL otherwise.
*/

TABLE *find_temporary_table(THD *thd, const char *db, const char *table_name) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length = create_table_def_key_tmp(thd, db, table_name, key);
  return find_temporary_table(thd, key, key_length);
}

/**
  Find a temporary table specified by TABLE_LIST instance in the
  THD::temporary_tables list.

  @return TABLE instance if a temporary table has been found; NULL otherwise.
*/

TABLE *find_temporary_table(THD *thd, const TABLE_LIST *tl) {
  const char *key;
  size_t key_length;
  char key_suffix[TMP_TABLE_KEY_EXTRA];
  TABLE *table;

  key_length = get_table_def_key(tl, &key);

  int4store(key_suffix, thd->server_id);
  int4store(key_suffix + 4, thd->variables.pseudo_thread_id);

  for (table = thd->temporary_tables; table; table = table->next) {
    if ((table->s->table_cache_key.length ==
         key_length + TMP_TABLE_KEY_EXTRA) &&
        !memcmp(table->s->table_cache_key.str, key, key_length) &&
        !memcmp(table->s->table_cache_key.str + key_length, key_suffix,
                TMP_TABLE_KEY_EXTRA))
      return table;
  }
  return nullptr;
}

/**
  Find a temporary table specified by a key in the THD::temporary_tables list.

  @return TABLE instance if a temporary table has been found; NULL otherwise.
*/

static TABLE *find_temporary_table(THD *thd, const char *table_key,
                                   size_t table_key_length) {
  for (TABLE *table = thd->temporary_tables; table; table = table->next) {
    if (table->s->table_cache_key.length == table_key_length &&
        !memcmp(table->s->table_cache_key.str, table_key, table_key_length)) {
      return table;
    }
  }

  return nullptr;
}

/**
  Drop a temporary table.

  - If the table is locked with LOCK TABLES or by prelocking,
    unlock it and remove it from the list of locked tables
    (THD::lock). Currently only transactional temporary tables
    are locked.
  - Close the temporary table.
  - Remove the table from the list of temporary tables.
*/

void drop_temporary_table(THD *thd, TABLE_LIST *table_list) {
  DBUG_TRACE;
  DBUG_PRINT("tmptable", ("closing table: '%s'.'%s'", table_list->db,
                          table_list->table_name));

  DBUG_ASSERT(is_temporary_table(table_list));

  TABLE *table = table_list->table;

  DBUG_ASSERT(!table->query_id || table->query_id == thd->query_id);

  /*
    If LOCK TABLES list is not empty and contains this table,
    unlock the table and remove the table from this list.
  */
  mysql_lock_remove(thd, thd->lock, table);
  close_temporary_table(thd, table, true, true);
  table_list->table = nullptr;
}

/*
  unlink from thd->temporary tables and close temporary table
*/

void close_temporary_table(THD *thd, TABLE *table, bool free_share,
                           bool delete_table) {
  DBUG_TRACE;
  DBUG_PRINT("tmptable",
             ("closing table: '%s'.'%s' %p  alias: '%s'", table->s->db.str,
              table->s->table_name.str, table, table->alias));

  if (table->prev) {
    table->prev->next = table->next;
    if (table->prev->next) table->next->prev = table->prev;
  } else {
    /* removing the item from the list */
    DBUG_ASSERT(table == thd->temporary_tables);
    /*
      slave must reset its temporary list pointer to zero to exclude
      passing non-zero value to end_slave via rli->save_temporary_tables
      when no temp tables opened, see an invariant below.
    */
    thd->temporary_tables = table->next;
    if (thd->temporary_tables) table->next->prev = nullptr;
  }
  if (thd->slave_thread) {
    /* natural invariant of temporary_tables */
    DBUG_ASSERT(thd->rli_slave->get_c_rli()->atomic_channel_open_temp_tables ||
                !thd->temporary_tables);
    --atomic_slave_open_temp_tables;
    --thd->rli_slave->get_c_rli()->atomic_channel_open_temp_tables;
  }
  close_temporary(thd, table, free_share, delete_table);
}

/*
  Close and delete a temporary table

  NOTE
    This dosn't unlink table from thd->temporary
    If this is needed, use close_temporary_table()
*/

void close_temporary(THD *thd, TABLE *table, bool free_share,
                     bool delete_table) {
  handlerton *table_type = table->s->db_type();
  DBUG_TRACE;
  DBUG_PRINT("tmptable", ("closing table: '%s'.'%s'", table->s->db.str,
                          table->s->table_name.str));

  free_io_cache(table);
  closefrm(table, false);
  if (delete_table) {
    DBUG_ASSERT(thd);
    rm_temporary_table(thd, table_type, table->s->path.str,
                       table->s->tmp_table_def);
  }

  if (free_share) {
    free_table_share(table->s);
    destroy(table);
    my_free(table);
  }
}

/*
  Used by ALTER TABLE when the table is a temporary one. It changes something
  only if the ALTER contained a RENAME clause (otherwise, table_name is the old
  name).
  Prepares a table cache key, which is the concatenation of db, table_name and
  thd->slave_proxy_id, separated by '\0'.
*/

bool rename_temporary_table(THD *thd, TABLE *table, const char *db,
                            const char *table_name) {
  char *key;
  size_t key_length;
  TABLE_SHARE *share = table->s;
  DBUG_TRACE;

  if (!(key = (char *)share->mem_root.Alloc(MAX_DBKEY_LENGTH)))
    return true; /* purecov: inspected */

  key_length = create_table_def_key_tmp(thd, db, table_name, key);
  share->set_table_cache_key(key, key_length);
  /* Also update table name in DD object. Database name is kept reset. */
  share->tmp_table_def->set_name(table_name);
  return false;
}

/**
   Force all other threads to stop using the table by upgrading
   metadata lock on it and remove unused TABLE instances from cache.

   @param thd      Thread handler
   @param table    Table to remove from cache
   @param function HA_EXTRA_PREPARE_FOR_DROP if table is to be deleted
                   HA_EXTRA_FORCE_REOPEN if table is not be used
                   HA_EXTRA_PREPARE_FOR_RENAME if table is to be renamed

   @note When returning, the table will be unusable for other threads
         until metadata lock is downgraded.

   @retval false Success.
   @retval true  Failure (e.g. because thread was killed).
*/

bool wait_while_table_is_used(THD *thd, TABLE *table,
                              enum ha_extra_function function) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s'  share: %p  db_stat: %u  version: %lu",
                       table->s->table_name.str, table->s, table->db_stat,
                       table->s->version()));

  if (thd->mdl_context.upgrade_shared_lock_nsec(
          table->mdl_ticket, MDL_EXCLUSIVE,
          thd->variables.lock_wait_timeout_nsec))
    return true;

  tdc_remove_table(thd, TDC_RT_REMOVE_NOT_OWN, table->s->db.str,
                   table->s->table_name.str, false);
  /* extra() call must come only after all instances above are closed */
  (void)table->file->ha_extra(function);
  return false;
}

/**
    Check that table exists in data-dictionary or in some storage engine.

    @param       thd     Thread context
    @param       table   Table list element
    @param[out]  exists  Out parameter which is set to true if table
                         exists and to false otherwise.

    @note If there is no table in data-dictionary but it exists in one
          of engines (e.g. it was created on another node of NDB cluster)
          this function will fetch and add proper table description to
          the data-dictionary.

    @retval  true   Some error occurred
    @retval  false  No error. 'exists' out parameter set accordingly.
*/

static bool check_if_table_exists(THD *thd, TABLE_LIST *table, bool *exists) {
  DBUG_TRACE;

  *exists = true;

  DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, table->db, table->table_name, MDL_SHARED));

  if (dd::table_exists(thd->dd_client(), table->db, table->table_name, exists))
    return true;  // Error is already reported.

  if (*exists) goto end;

  /* Table doesn't exist. Check if some engine can provide it. */
  if (ha_check_if_table_exists(thd, table->db, table->table_name, exists)) {
    my_printf_error(ER_OUT_OF_RESOURCES,
                    "Failed to open '%-.64s', error while "
                    "unpacking from engine",
                    MYF(0), table->table_name);
    return true;
  }
end:
  return false;
}

/**
  An error handler which converts, if possible, ER_LOCK_DEADLOCK error
  that can occur when we are trying to acquire a metadata lock to
  a request for back-off and re-start of open_tables() process.
*/

class MDL_deadlock_handler : public Internal_error_handler {
 public:
  MDL_deadlock_handler(Open_table_context *ot_ctx_arg)
      : m_ot_ctx(ot_ctx_arg), m_is_active(false) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (!m_is_active && sql_errno == ER_LOCK_DEADLOCK) {
      /* Disable the handler to avoid infinite recursion. */
      m_is_active = true;
      (void)m_ot_ctx->request_backoff_action(
          Open_table_context::OT_BACKOFF_AND_RETRY, nullptr);
      m_is_active = false;
      /*
        If the above back-off request failed, a new instance of
        ER_LOCK_DEADLOCK error was emitted. Thus the current
        instance of error condition can be treated as handled.
      */
      return true;
    }
    return false;
  }

 private:
  /** Open table context to be used for back-off request. */
  Open_table_context *m_ot_ctx;
  /**
    Indicates that we are already in the process of handling
    ER_LOCK_DEADLOCK error. Allows to re-emit the error from
    the error handler without falling into infinite recursion.
  */
  bool m_is_active;
};

/**
  Try to acquire an MDL lock for a table being opened.

  @param[in,out] thd      Session context, to report errors.
  @param[out]    ot_ctx   Open table context, to hold the back off
                          state. If we failed to acquire a lock
                          due to a lock conflict, we add the
                          failed request to the open table context.
  @param[in,out] table_list Table list element for the table being opened.
                            Its "mdl_request" member specifies the MDL lock
                            to be requested. If we managed to acquire a
                            ticket (no errors or lock conflicts occurred),
                            TABLE_LIST::mdl_request contains a reference
                            to it on return. However, is not modified if
                            MDL lock type- modifying flags were provided.
                            We also use TABLE_LIST::lock_type member to
                            detect cases when MDL_SHARED_WRITE_LOW_PRIO
                            lock should be acquired instead of the normal
                            MDL_SHARED_WRITE lock.
  @param[in]    flags flags MYSQL_OPEN_FORCE_SHARED_MDL,
                          MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL or
                          MYSQL_OPEN_FAIL_ON_MDL_CONFLICT
                          @sa open_table().
  @param[out]   mdl_ticket Only modified if there was no error.
                          If we managed to acquire an MDL
                          lock, contains a reference to the
                          ticket, otherwise is set to NULL.

  @retval true  An error occurred.
  @retval false No error, but perhaps a lock conflict, check mdl_ticket.
*/

static bool open_table_get_mdl_lock(THD *thd, Open_table_context *ot_ctx,
                                    TABLE_LIST *table_list, uint flags,
                                    MDL_ticket **mdl_ticket) {
  MDL_request *mdl_request = &table_list->mdl_request;
  MDL_request new_mdl_request;

  if (flags &
      (MYSQL_OPEN_FORCE_SHARED_MDL | MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL)) {
    /*
      MYSQL_OPEN_FORCE_SHARED_MDL flag means that we are executing
      PREPARE for a prepared statement and want to override
      the type-of-operation aware metadata lock which was set
      in the parser/during view opening with a simple shared
      metadata lock.
      This is necessary to allow concurrent execution of PREPARE
      and LOCK TABLES WRITE statement against the same table.

      MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL flag means that we open
      the table in order to get information about it for one of I_S
      queries and also want to override the type-of-operation aware
      shared metadata lock which was set earlier (e.g. during view
      opening) with a high-priority shared metadata lock.
      This is necessary to avoid unnecessary waiting and extra
      ER_WARN_I_S_SKIPPED_TABLE warnings when accessing I_S tables.

      These two flags are mutually exclusive.
    */
    DBUG_ASSERT(!(flags & MYSQL_OPEN_FORCE_SHARED_MDL) ||
                !(flags & MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL));

    MDL_REQUEST_INIT_BY_KEY(&new_mdl_request, &mdl_request->key,
                            (flags & MYSQL_OPEN_FORCE_SHARED_MDL)
                                ? MDL_SHARED
                                : MDL_SHARED_HIGH_PRIO,
                            MDL_TRANSACTION);
    mdl_request = &new_mdl_request;
  } else if (thd->variables.low_priority_updates &&
             mdl_request->type == MDL_SHARED_WRITE &&
             (table_list->lock_descriptor().type == TL_WRITE_DEFAULT ||
              table_list->lock_descriptor().type ==
                  TL_WRITE_CONCURRENT_DEFAULT)) {
    /*
      We are in @@low_priority_updates=1 mode and are going to acquire
      SW metadata lock on a table which for which neither LOW_PRIORITY nor
      HIGH_PRIORITY clauses were used explicitly.
      To keep compatibility with THR_LOCK locks and to avoid starving out
      concurrent LOCK TABLES READ statements, we need to acquire the low-prio
      version of SW lock instead of a normal SW lock in this case.
    */
    MDL_REQUEST_INIT_BY_KEY(&new_mdl_request, &mdl_request->key,
                            MDL_SHARED_WRITE_LOW_PRIO, MDL_TRANSACTION);
    mdl_request = &new_mdl_request;
  }

  if (flags & MYSQL_OPEN_FAIL_ON_MDL_CONFLICT) {
    /*
      When table is being open in order to get data for I_S table,
      we might have some tables not only open but also locked (e.g. when
      this happens under LOCK TABLES or in a stored function).
      As a result by waiting on a conflicting metadata lock to go away
      we may create a deadlock which won't entirely belong to the
      MDL subsystem and thus won't be detectable by this subsystem's
      deadlock detector.
      To avoid such situation we skip the trouble-making table if
      there is a conflicting lock.
    */
    if (thd->mdl_context.try_acquire_lock(mdl_request)) return true;
    if (mdl_request->ticket == nullptr) {
      my_error(ER_WARN_I_S_SKIPPED_TABLE, MYF(0), mdl_request->key.db_name(),
               mdl_request->key.name());
      return true;
    }
  } else {
    /*
      We are doing a normal table open. Let us try to acquire a metadata
      lock on the table. If there is a conflicting lock, acquire_lock()
      will wait for it to go away. Sometimes this waiting may lead to a
      deadlock, with the following results:
      1) If a deadlock is entirely within MDL subsystem, it is
         detected by the deadlock detector of this subsystem.
         ER_LOCK_DEADLOCK error is produced. Then, the error handler
         that is installed prior to the call to acquire_lock() attempts
         to request a back-off and retry. Upon success, ER_LOCK_DEADLOCK
         error is suppressed, otherwise propagated up the calling stack.
      2) Otherwise, a deadlock may occur when the wait-for graph
         includes edges not visible to the MDL deadlock detector.
         One such example is a wait on an InnoDB row lock, e.g. when:
         conn C1 gets SR MDL lock on t1 with SELECT * FROM t1
         conn C2 gets a row lock on t2 with  SELECT * FROM t2 FOR UPDATE
         conn C3 gets in and waits on C1 with DROP TABLE t0, t1
         conn C2 continues and blocks on C3 with SELECT * FROM t0
         conn C1 deadlocks by waiting on C2 by issuing SELECT * FROM
         t2 LOCK IN SHARE MODE.
         Such circular waits are currently only resolved by timeouts,
         e.g. @@innodb_lock_wait_timeout or @@lock_wait_timeout.

      Note that we want to force DML deadlock weight for our context
      when acquiring locks in this place. This is done to avoid situation
      when LOCK TABLES statement, which acquires strong SNRW and SRO locks
      on implicitly used tables, deadlocks with a concurrent DDL statement
      and the DDL statement is aborted since it is chosen as a deadlock
      victim. It is better to choose LOCK TABLES as a victim in this case
      as a deadlock can be easily caught here and handled by back-off and retry,
      without reporting any error to the user.
      We still have a few weird cases, like FLUSH TABLES <table-list> WITH
      READ LOCK, where we use "strong" metadata locks and open_tables() is
      called with some metadata locks pre-acquired. In these cases we still
      want to use DDL deadlock weight as back-off is not possible.
    */
    MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);

    thd->push_internal_handler(&mdl_deadlock_handler);
    thd->mdl_context.set_force_dml_deadlock_weight(ot_ctx->can_back_off());

    bool result = thd->mdl_context.acquire_lock_nsec(
        mdl_request, ot_ctx->get_timeout_nsec());

    thd->mdl_context.set_force_dml_deadlock_weight(false);
    thd->pop_internal_handler();

    if (result && !ot_ctx->can_recover_from_failed_open()) return true;
  }
  *mdl_ticket = mdl_request->ticket;
  return false;
}

/**
  Check if table's share is being removed from the table definition
  cache and, if yes, wait until the flush is complete.

  @param thd             Thread context.
  @param db              Database name.
  @param table_name      Table name.
  @param wait_timeout    Timeout for waiting.
  @param deadlock_weight Weight of this wait for deadlock detector.

  @retval false   Success. Share is up to date or has been flushed.
  @retval true    Error (OOM, our was killed, the wait resulted
                  in a deadlock or timeout). Reported.
*/

static bool tdc_wait_for_old_version_nsec(THD *thd, const char *db,
                                          const char *table_name,
                                          ulonglong wait_timeout_nsec,
                                          uint deadlock_weight) {
  TABLE_SHARE *share;
  bool res = false;

  mysql_mutex_lock(&LOCK_open);
  if ((share = get_cached_table_share(db, table_name)) &&
      share->has_old_version()) {
    struct timespec abstime;
    set_timespec_nsec(&abstime, wait_timeout_nsec);
    res = share->wait_for_old_version(thd, &abstime, deadlock_weight);
  }
  mysql_mutex_unlock(&LOCK_open);
  return res;
}

void return_table_to_cache(THD *thd, TABLE_LIST *table_list) {
  TABLE *table = nullptr;

  DBUG_ENTER("return_table_to_cache");

  if (!table_list) DBUG_VOID_RETURN;

  if (table_list->table != nullptr) {
    DBUG_ASSERT(thd->open_tables == table_list->table);
    DBUG_ASSERT(thd->open_tables->next == nullptr);
    DBUG_ASSERT(!table_list->table->s->has_old_version());

    table = table_list->table;

    /* Do this *before* entering the LOCK_open critical section. */
    if (table->file != nullptr) table->file->unbind_psi();

    release_or_close_table(thd, table);
    thd->open_tables = nullptr;
  }

  /* Release the meta data lock */
  thd->mdl_context.release_transactional_locks();

  table_list->mdl_request.ticket = nullptr;

  DBUG_VOID_RETURN;
}

/*
 * This function basically copies functionality from open_table() to
 * create a table from a share.
 */
static TABLE *init_table_from_share(THD *thd, TABLE_SHARE *share,
                                    TABLE_LIST *table_list) {
  DBUG_ENTER("init_table_from_share");

  TABLE *table = nullptr;
  int error = 0;
  uint flags = 0;
  const char *alias = table_list->alias;

  {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Table *table_def = nullptr;
    if (!(flags & MYSQL_OPEN_NO_NEW_TABLE_IN_SE) &&
        thd->dd_client()->acquire(share->db.str, share->table_name.str,
                                  &table_def)) {
      // Error is reported by the dictionary subsystem.
      goto err_lock;
    }

    if (table_def && table_def->hidden() == dd::Abstract_table::HT_HIDDEN_SE) {
      my_error(ER_NO_SUCH_TABLE, MYF(0), table_list->db,
               table_list->table_name);
      goto err_lock;
    }

    /* make a new table */
    if (!(table = (TABLE *)my_malloc(key_memory_TABLE, sizeof(*table),
                                     MYF(MY_WME))))
      goto err_lock;

    error = open_table_from_share(
        thd, share, alias,
        ((flags & MYSQL_OPEN_NO_NEW_TABLE_IN_SE)
             ? 0
             : ((uint)(HA_OPEN_KEYFILE | HA_OPEN_RNDFILE | HA_GET_INDEX |
                       HA_TRY_READ_ONLY))),
        EXTRA_RECORD, thd->open_options, table, false, table_def);

    if (error) {
      my_free(table);
      goto err_lock;
    } else if (share->crashed) {
      switch (thd->lex->sql_command) {
        case SQLCOM_ALTER_TABLE:
        case SQLCOM_REPAIR:
        case SQLCOM_CHECK:
        case SQLCOM_SHOW_CREATE:
          break;
        default:
          closefrm(table, 0);
          my_free(table);
          my_error(ER_CRASHED_ON_USAGE, MYF(0), share->table_name.str);
          goto err_lock;
      }
    }

    if (open_table_entry_fini(thd, share, table_def, table)) {
      closefrm(table, 0);
      my_free(table);
      goto err_lock;
    }
  }
  {
    /* Add new TABLE object to table cache for this connection. */
    Table_cache *tc = table_cache_manager.get_cache(thd);

    tc->lock();

    if (tc->add_used_table(thd, table)) {
      tc->unlock();
      goto err_lock;
    }
    tc->unlock();
  }
  thd->status_var.table_open_cache_misses++;

  table->mdl_ticket = table_list->mdl_request.ticket;

  table->reginfo.lock_type = TL_READ; /* Assume read */

  table->set_created();
  /*
    Check that there is no reference to a condition from an earlier query
    (cf. Bug#58553).
  */
  DBUG_ASSERT(table->file->pushed_cond == nullptr);
  table_list
      ->set_updatable();  // It is not derived table nor non-updatable VIEW
  table_list->set_insertable();

  table_list->table = table;

  DBUG_RETURN(table);

err_lock:
  mysql_mutex_lock(&LOCK_open);
  release_table_share(share);
  mysql_mutex_unlock(&LOCK_open);

  DBUG_RETURN(nullptr);
}

bool get_table_from_cache(THD *thd, TABLE_LIST *table_list) {
  DBUG_ENTER("get_table_from_cache");

  const char *key = nullptr;
  uint flags = 0;
  MDL_ticket *mdl_ticket = nullptr;
  Open_table_context ot_ctx(thd, flags);
  int error = 0;
  TABLE_SHARE *share = nullptr;
  TABLE *table = nullptr;
  uint key_length = get_table_def_key(table_list, &key);
  Table_cache *tc = table_cache_manager.get_cache(thd);

  /* Get a SHARE lock on the meta-data. Need to acquire this lock at the
     very least.
  */
  if (open_table_get_mdl_lock(thd, &ot_ctx, table_list, flags, &mdl_ticket) ||
      mdl_ticket == nullptr) {
    goto done;
  }

  tc->lock();
  table = tc->get_table(thd, key, key_length, &share);
  tc->unlock();

  /* Failed to get table definition from thread cache.
   * Try using the table share. */
  if (!table) {
    mysql_mutex_lock(&LOCK_open);

    /* Try to get a reference to the share */
    if (share) {
      share->increment_ref_count();
    } else {
      share = get_table_share_with_discover(thd, table_list, key, key_length,
                                            flags & MYSQL_OPEN_SECONDARY_ENGINE,
                                            &error);
    }

    mysql_mutex_unlock(&LOCK_open);

    /* We have a share ref! Try to init the table from the share */
    if (share) {
      table = init_table_from_share(thd, share, table_list);
    }
  }

  if (table != nullptr) {
    table->next = thd->open_tables;
    thd->set_open_tables(table);
  }

done:
  mysql_mutex_assert_not_owner(&LOCK_open);
  table_list->table = table;
  thd->clear_error();
  thd->get_stmt_da()->reset_condition_info(thd);
  DBUG_RETURN(table_list->table == nullptr);
}

/**
  Open a base table.

  @param thd            Thread context.
  @param table_list     Open first table in list.
  @param ot_ctx         Context with flags which modify how open works
                        and which is used to recover from a failed
                        open_table() attempt.
                        Some examples of flags:
                        MYSQL_OPEN_IGNORE_FLUSH - Open table even if
                        someone has done a flush. No version number
                        checking is done.
                        MYSQL_OPEN_HAS_MDL_LOCK - instead of acquiring
                        metadata locks rely on that caller already has
                        appropriate ones.

  Uses a cache of open tables to find a TABLE instance not in use.

  If TABLE_LIST::open_strategy is set to OPEN_IF_EXISTS, the table is
  opened only if it exists. If the open strategy is OPEN_STUB, the
  underlying table is never opened. In both cases, metadata locks are
  always taken according to the lock strategy.

  The function used to open temporary tables, but now it opens base tables
  only.

  @retval true  Open failed. "action" parameter may contain type of action
                needed to remedy problem before retrying again.
  @retval false Success. Members of TABLE_LIST structure are filled properly
                (e.g.  TABLE_LIST::table is set for real tables and
                TABLE_LIST::view is set for views).
*/

bool open_table(THD *thd, TABLE_LIST *table_list, Open_table_context *ot_ctx) {
  TABLE *table;
  const char *key;
  size_t key_length;
  const char *alias = table_list->alias;
  uint flags = ot_ctx->get_flags();
  MDL_ticket *mdl_ticket = nullptr;
  int error = 0;
  TABLE_SHARE *share;

  DBUG_TRACE;

  /*
    The table must not be opened already. The table can be pre-opened for
    some statements if it is a temporary table.

    open_temporary_table() must be used to open temporary tables.
    A derived table cannot be opened with this.
  */
  DBUG_ASSERT(!table_list->table && !table_list->is_derived());

  /* an open table operation needs a lot of the stack space */
  if (check_stack_overrun(thd, STACK_MIN_SIZE_FOR_OPEN, (uchar *)&alias))
    return true;

  // New DD- In current_thd->is_strict_sql_mode() mode we call open_table
  // on new DD tables like mysql.tables/* when CREATE fails and we
  // try to abort the operation and invoke quick_rm_table().
  // Currently, we ignore deleting table in strict mode. Need to fix this.
  // TODO.

  DBUG_EXECUTE_IF("kill_query_on_open_table_from_tz_find", {
    /*
      When on calling my_tz_find the following
      tables are opened in specified order: time_zone_name,
      time_zone, time_zone_transition_type,
      time_zone_transition. Emulate killing a query
      on opening the second table in the list.
    */
    if (!strcmp("time_zone", table_list->table_name))
      thd->killed = THD::KILL_QUERY;
  });

  if (!(flags & MYSQL_OPEN_IGNORE_KILLED) && thd->killed) return true;

  /*
    Check if we're trying to take a write lock in a read only transaction.

    Note that we allow write locks on log tables as otherwise logging
    to general/slow log would be disabled in read only transactions.
  */
  if (table_list->mdl_request.is_write_lock_request() && thd->tx_read_only &&
      !(flags & (MYSQL_LOCK_LOG_TABLE | MYSQL_OPEN_HAS_MDL_LOCK))) {
    my_error(ER_CANT_EXECUTE_IN_READ_ONLY_TRANSACTION, MYF(0));
    return true;
  }

  /*
    FLUSH TABLES is ignored for DD, I_S and P_S tables/views.
    Hence setting MYSQL_OPEN_IGNORE_FLUSH flag.
  */
  if (table_list->is_system_view || belongs_to_dd_table(table_list) ||
      belongs_to_p_s(table_list))
    flags |= MYSQL_OPEN_IGNORE_FLUSH;

  key_length = get_table_def_key(table_list, &key);

  // If a table in a secondary storage engine has been requested,
  // adjust the key to refer to the secondary table.
  std::string secondary_key;
  if ((flags & MYSQL_OPEN_SECONDARY_ENGINE) != 0) {
    secondary_key = create_table_def_key_secondary(
        table_list->get_db_name(), table_list->get_table_name());
    key = secondary_key.data();
    key_length = secondary_key.length();
  }

  /*
    If we're in pre-locked or LOCK TABLES mode, let's try to find the
    requested table in the list of pre-opened and locked tables. If the
    table is not there, return an error - we can't open not pre-opened
    tables in pre-locked/LOCK TABLES mode.

    There is a special case where we allow opening not pre-opened tables
    in LOCK TABLES mode for new DD tables. The reason is as following.
    With new DD, IS system views need to be accessible in LOCK TABLE
    mode without user explicitly calling LOCK TABLE on IS view or its
    underlying DD tables. This is required to keep the old behavior the
    MySQL server had without new DD.

    In case user executes IS system view under LOCK TABLE mode
    (LTM and not prelocking), then MySQL server implicitly opens system
    view and related DD tables. Such DD tables are then implicitly closed
    upon end of statement execution.

    Our goal is to hide DD tables from users, so there is no possibility of
    explicit locking DD table using LOCK TABLE. In case user does LOCK TABLE
    on IS system view explicitly, MySQL server throws a error.

    TODO: move this block into a separate function.
  */
  if (thd->locked_tables_mode && !(flags & MYSQL_OPEN_GET_NEW_TABLE) &&
      !(in_LTM(thd) &&
        (table_list->is_system_view || belongs_to_dd_table(table_list) ||
         belongs_to_p_s(table_list)))) {  // Using table locks
    TABLE *best_table = nullptr;
    int best_distance = INT_MIN;
    for (table = thd->open_tables; table; table = table->next) {
      if (table->s->table_cache_key.length == key_length &&
          !memcmp(table->s->table_cache_key.str, key, key_length)) {
        if (!my_strcasecmp(system_charset_info, table->alias, alias) &&
            table->query_id != thd->query_id && /* skip tables already used */
            (thd->locked_tables_mode == LTM_LOCK_TABLES ||
             table->query_id == 0)) {
          int distance = ((int)table->reginfo.lock_type -
                          (int)table_list->lock_descriptor().type);

          /*
            Find a table that either has the exact lock type requested,
            or has the best suitable lock. In case there is no locked
            table that has an equal or higher lock than requested,
            we us the closest matching lock to be able to produce an error
            message about wrong lock mode on the table. The best_table
            is changed if bd < 0 <= d or bd < d < 0 or 0 <= d < bd.

            distance <  0 - No suitable lock found
            distance >  0 - we have lock mode higher then we require
            distance == 0 - we have lock mode exactly which we need
          */
          if ((best_distance < 0 && distance > best_distance) ||
              (distance >= 0 && distance < best_distance)) {
            best_distance = distance;
            best_table = table;
            if (best_distance == 0) {
              /*
                We have found a perfect match and can finish iterating
                through open tables list. Check for table use conflict
                between calling statement and SP/trigger is done in
                lock_tables().
              */
              break;
            }
          }
        }
      }
    }
    if (best_table) {
      table = best_table;
      table->query_id = thd->query_id;
      DBUG_PRINT("info", ("Using locked table"));
      goto reset;
    }
    /*
      Is this table a view and not a base table?
      (it is work around to allow to open view with locked tables,
      real fix will be made after definition cache will be made)

      Since opening of view which was not explicitly locked by LOCK
      TABLES breaks metadata locking protocol (potentially can lead
      to deadlocks) it should be disallowed.
    */
    if (thd->mdl_context.owns_equal_or_stronger_lock(
            MDL_key::TABLE, table_list->db, table_list->table_name,
            MDL_SHARED)) {
      /*
        Note that we can't be 100% sure that it is a view since it's
        possible that we either simply have not found unused TABLE
        instance in THD::open_tables list or were unable to open table
        during prelocking process (in this case in theory we still
        should hold shared metadata lock on it).
      */
      dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
      const dd::View *view = nullptr;
      if (!thd->dd_client()->acquire(table_list->db, table_list->table_name,
                                     &view) &&
          view != nullptr) {
        /*
          If parent_l of the table_list is non null then a merge table
          has this view as child table, which is not supported.
        */
        if (table_list->parent_l) {
          my_error(ER_WRONG_MRG_TABLE, MYF(0));
          return true;
        }

        if (!tdc_open_view(thd, table_list, key, key_length)) {
          DBUG_ASSERT(table_list->is_view());
          return false;  // VIEW
        }
      }
    }
    /*
      No table in the locked tables list. In case of explicit LOCK TABLES
      this can happen if a user did not include the table into the list.
      In case of pre-locked mode locked tables list is generated automatically,
      so we may only end up here if the table did not exist when
      locked tables list was created.
    */
    if (thd->locked_tables_mode == LTM_PRELOCKED)
      my_error(ER_NO_SUCH_TABLE, MYF(0), table_list->db, table_list->alias);
    else
      my_error(ER_TABLE_NOT_LOCKED, MYF(0), alias);
    return true;
  }

  // Non pre-locked/LOCK TABLES mode, and not using secondary storage engine.
  // This is the normal use case.

  if ((flags & (MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_OPEN_SECONDARY_ENGINE)) == 0) {
    /*
      We are not under LOCK TABLES and going to acquire write-lock/
      modify the base table. We need to acquire protection against
      global read lock until end of this statement in order to have
      this statement blocked by active FLUSH TABLES WITH READ LOCK.

      We don't block acquire this protection under LOCK TABLES as
      such protection already acquired at LOCK TABLES time and
      not released until UNLOCK TABLES.

      We don't block statements which modify only temporary tables
      as these tables are not preserved by backup by any form of
      backup which uses FLUSH TABLES WITH READ LOCK.

      TODO: The fact that we sometimes acquire protection against
            GRL only when we encounter table to be write-locked
            slightly increases probability of deadlock.
            This problem will be solved once Alik pushes his
            temporary table refactoring patch and we can start
            pre-acquiring metadata locks at the beggining of
            open_tables() call.
    */
    if (table_list->mdl_request.is_write_lock_request() &&
        !(flags &
          (MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK | MYSQL_OPEN_FORCE_SHARED_MDL |
           MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL |
           MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK)) &&
        !ot_ctx->has_protection_against_grl()) {
      MDL_request protection_request;
      MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);

      if (thd->global_read_lock.can_acquire_protection()) return true;

      MDL_REQUEST_INIT(&protection_request, MDL_key::GLOBAL, "", "",
                       MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);

      /*
        Install error handler which if possible will convert deadlock error
        into request to back-off and restart process of opening tables.

        Prefer this context as a victim in a deadlock when such a deadlock
        can be easily handled by back-off and retry.
      */
      thd->push_internal_handler(&mdl_deadlock_handler);
      thd->mdl_context.set_force_dml_deadlock_weight(ot_ctx->can_back_off());

      bool result = thd->mdl_context.acquire_lock_nsec(
          &protection_request, ot_ctx->get_timeout_nsec());

      /*
        Unlike in other places where we acquire protection against global read
        lock, the read_only state is not checked here since we check its state
        later in mysql_lock_tables()
      */

      thd->mdl_context.set_force_dml_deadlock_weight(false);
      thd->pop_internal_handler();

      if (result) return true;

      ot_ctx->set_has_protection_against_grl();
    }

    if (open_table_get_mdl_lock(thd, ot_ctx, table_list, flags, &mdl_ticket) ||
        mdl_ticket == nullptr) {
      DEBUG_SYNC(thd, "before_open_table_wait_refresh");
      return true;
    }
    DEBUG_SYNC(thd, "after_open_table_mdl_shared");
  } else {
    /*
      Grab reference to the MDL lock ticket that was acquired
      by the caller.
    */
    mdl_ticket = table_list->mdl_request.ticket;
  }

  DBUG_EXECUTE_IF("sql_opening_table", {
    if (thd->slave_thread) {
      const char act[] = "now signal opening wait_for slave_killed";
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    }
  });

  if (table_list->open_strategy == TABLE_LIST::OPEN_IF_EXISTS ||
      table_list->open_strategy == TABLE_LIST::OPEN_FOR_CREATE) {
    bool exists;

    if (check_if_table_exists(thd, table_list, &exists)) return true;

    /*
      If the table does not exist then upgrade the lock to the EXCLUSIVE MDL
      lock.
    */
    if (!exists) {
      if (table_list->open_strategy == TABLE_LIST::OPEN_FOR_CREATE &&
          !(flags & (MYSQL_OPEN_FORCE_SHARED_MDL |
                     MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL))) {
        MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);

        thd->push_internal_handler(&mdl_deadlock_handler);

        DEBUG_SYNC(thd, "before_upgrading_lock_from_S_to_X_for_create_table");
        bool wait_result = thd->mdl_context.upgrade_shared_lock_nsec(
            table_list->mdl_request.ticket, MDL_EXCLUSIVE,
            thd->variables.lock_wait_timeout_nsec);

        thd->pop_internal_handler();
        DEBUG_SYNC(thd, "after_upgrading_lock_from_S_to_X_for_create_table");

        /* Deadlock or timeout occurred while upgrading the lock. */
        if (wait_result) return true;
      }

      return false;
    }

    /* Table exists. Let us try to open it. */
  } else if (table_list->open_strategy == TABLE_LIST::OPEN_STUB)
    return false;

retry_share : {
  Table_cache *tc = table_cache_manager.get_cache(thd);

  tc->lock();

  /*
    Try to get unused TABLE object or at least pointer to
    TABLE_SHARE from the table cache.
  */
  table = tc->get_table(thd, key, key_length, &share);

  if (table) {
    /* We have found an unused TABLE object. */

    if (!(flags & MYSQL_OPEN_IGNORE_FLUSH)) {
      /*
        TABLE_SHARE::version can only be initialised while holding the
        LOCK_open and in this case no one has a reference to the share
        object, if a reference exists to the share object it is necessary
        to lock both LOCK_open AND all table caches in order to update
        TABLE_SHARE::version. The same locks are required to increment
        refresh_version global variable.

        As result it is safe to compare TABLE_SHARE::version and
        refresh_version values while having only lock on the table
        cache for this thread.

        Table cache should not contain any unused TABLE objects with
        old versions.
      */
      DBUG_ASSERT(!share->has_old_version());

      /*
        Still some of already opened might become outdated (e.g. due to
        concurrent table flush). So we need to compare version of opened
        tables with version of TABLE object we just have got.
      */
      if (thd->open_tables &&
          thd->open_tables->s->version() != share->version()) {
        tc->release_table(thd, table);
        tc->unlock();
        (void)ot_ctx->request_backoff_action(
            Open_table_context::OT_REOPEN_TABLES, nullptr);
        return true;
      }
    }
    tc->unlock();

    /* Call rebind_psi outside of the critical section. */
    DBUG_ASSERT(table->file != nullptr);
    table->file->stats.reset_io_counters();
    table->file->rebind_psi();

    thd->status_var.table_open_cache_hits++;
    goto table_found;
  } else if (share) {
    /*
      We weren't able to get an unused TABLE object. Still we have
      found TABLE_SHARE for it. So let us try to create new TABLE
      for it. We start by incrementing share's reference count and
      checking its version.
    */
    mysql_mutex_lock(&LOCK_open);
    tc->unlock();
    share->increment_ref_count();
    goto share_found;
  } else {
    /*
      We have not found neither TABLE nor TABLE_SHARE object in
      table cache (this means that there are no TABLE objects for
      it in it).
      Let us try to get TABLE_SHARE from table definition cache or
      from disk and then to create TABLE object for it.
    */
    tc->unlock();
  }
}

  mysql_mutex_lock(&LOCK_open);

  if (!(share = get_table_share_with_discover(
            thd, table_list, key, key_length,
            flags & MYSQL_OPEN_SECONDARY_ENGINE, &error))) {
    mysql_mutex_unlock(&LOCK_open);
    /*
      If thd->is_error() is not set, we either need discover
      (error == 7), or the error was silenced by the prelocking
      handler (error == 0), in which case we should skip this
      table.
    */
    if (error == 7 && !thd->is_error()) {
      (void)ot_ctx->request_backoff_action(Open_table_context::OT_DISCOVER,
                                           table_list);
    }
    return true;
  }

  /*
    Check if this TABLE_SHARE-object corresponds to a view. Note, that there is
    no need to call TABLE_SHARE::has_old_version() as we do for regular tables,
    because view shares are always up to date.
  */
  if (share->is_view) {
    bool view_open_result = true;
    /*
      If parent_l of the table_list is non null then a merge table
      has this view as child table, which is not supported.
    */
    if (table_list->parent_l) my_error(ER_WRONG_MRG_TABLE, MYF(0));
    /*
      This table is a view. Validate its metadata version: in particular,
      that it was a view when the statement was prepared.
    */
    else if (check_and_update_table_version(thd, table_list, share))
      ;
    else if (table_list->open_strategy == TABLE_LIST::OPEN_FOR_CREATE) {
      /*
        Skip reading the view definition if the open is for a table to be
        created. This scenario will happen only when there exists a view and
        the current CREATE TABLE request is with the same name.
      */
      release_table_share(share);
      mysql_mutex_unlock(&LOCK_open);

      /*
        For SP and PS, LEX objects are created at the time of statement prepare.
        And open_table() is called for every execute after that. Skip creation
        of LEX objects if it is already present.
      */
      if (!table_list->is_view()) {
        Prepared_stmt_arena_holder ps_arena_holder(thd);

        /*
          Since we are skipping parse_view_definition(), which creates view LEX
          object used by the executor and other parts of the code to detect the
          presence of a view, a dummy LEX object needs to be created.
        */
        table_list->set_view_query((LEX *)new (thd->mem_root) st_lex_local);
        if (!table_list->is_view()) return true;

        // Create empty list of view_tables.
        table_list->view_tables =
            new (thd->mem_root) mem_root_deque<TABLE_LIST *>(thd->mem_root);
        if (table_list->view_tables == nullptr) return true;

        table_list->view_db.str = table_list->db;
        table_list->view_db.length = table_list->db_length;
        table_list->view_name.str = table_list->table_name;
        table_list->view_name.length = table_list->table_name_length;
      }

      return false;
    } else {
      /*
        Read definition of existing view.
      */
      view_open_result = open_and_read_view(thd, share, table_list);
    }

    /* TODO: Don't free this */
    release_table_share(share);
    mysql_mutex_unlock(&LOCK_open);

    if (view_open_result) return true;

    if (parse_view_definition(thd, table_list)) return true;

    DBUG_ASSERT(table_list->is_view());

    return false;
  }

share_found:
  if (!(flags & MYSQL_OPEN_IGNORE_FLUSH)) {
    if (share->has_old_version()) {
      /*
        We already have an MDL lock. But we have encountered an old
        version of table in the table definition cache which is possible
        when someone changes the table version directly in the cache
        without acquiring a metadata lock (e.g. this can happen during
        "rolling" FLUSH TABLE(S)).
        Release our reference to share, wait until old version of
        share goes away and then try to get new version of table share.
      */
      release_table_share(share);
      mysql_mutex_unlock(&LOCK_open);

      MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);
      bool wait_result;

      thd->push_internal_handler(&mdl_deadlock_handler);

      /*
        In case of deadlock we would like this thread to be preferred as
        a deadlock victim when this deadlock can be nicely handled by
        back-off and retry. We still have a few weird cases, like
        FLUSH TABLES <table-list> WITH READ LOCK, where we use strong
        metadata locks and open_tables() is called with some metadata
        locks pre-acquired. In these cases we still want to use DDL
        deadlock weight.
      */
      uint deadlock_weight = ot_ctx->can_back_off()
                                 ? MDL_wait_for_subgraph::DEADLOCK_WEIGHT_DML
                                 : mdl_ticket->get_deadlock_weight();

      wait_result = tdc_wait_for_old_version_nsec(
          thd, table_list->db, table_list->table_name,
          ot_ctx->get_timeout_nsec(), deadlock_weight);

      thd->pop_internal_handler();

      if (wait_result) return true;

      DEBUG_SYNC(thd, "open_table_before_retry");
      goto retry_share;
    }

    if (thd->open_tables &&
        thd->open_tables->s->version() != share->version()) {
      /*
        If the version changes while we're opening the tables,
        we have to back off, close all the tables opened-so-far,
        and try to reopen them. Note: refresh_version is currently
        changed only during FLUSH TABLES.
      */
      release_table_share(share);
      mysql_mutex_unlock(&LOCK_open);
      (void)ot_ctx->request_backoff_action(Open_table_context::OT_REOPEN_TABLES,
                                           nullptr);
      return true;
    }
  }

  mysql_mutex_unlock(&LOCK_open);

  DEBUG_SYNC(thd, "open_table_found_share");

  {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Table *table_def = nullptr;
    if (!(flags & MYSQL_OPEN_NO_NEW_TABLE_IN_SE) &&
        thd->dd_client()->acquire(share->db.str, share->table_name.str,
                                  &table_def)) {
      // Error is reported by the dictionary subsystem.
      goto err_lock;
    }

    if (table_def && table_def->hidden() == dd::Abstract_table::HT_HIDDEN_SE) {
      my_error(ER_NO_SUCH_TABLE, MYF(0), table_list->db,
               table_list->table_name);
      goto err_lock;
    }

    /* make a new table */
    if (!(table = (TABLE *)my_malloc(key_memory_TABLE, sizeof(*table),
                                     MYF(MY_WME))))
      goto err_lock;

    error = open_table_from_share(
        thd, share, alias,
        ((flags & MYSQL_OPEN_NO_NEW_TABLE_IN_SE)
             ? 0
             : ((uint)(HA_OPEN_KEYFILE | HA_OPEN_RNDFILE | HA_GET_INDEX |
                       HA_TRY_READ_ONLY))),
        EXTRA_RECORD, thd->open_options, table, false, table_def);

    if (error) {
      destroy(table);
      my_free(table);

      if (error == 7)
        (void)ot_ctx->request_backoff_action(Open_table_context::OT_DISCOVER,
                                             table_list);
      else if (error == 8)
        (void)ot_ctx->request_backoff_action(
            Open_table_context::OT_FIX_ROW_TYPE, table_list);
      else if (share->crashed)
        (void)ot_ctx->request_backoff_action(Open_table_context::OT_REPAIR,
                                             table_list);
      goto err_lock;
    } else if (share->crashed) {
      switch (thd->lex->sql_command) {
        case SQLCOM_ALTER_TABLE:
        case SQLCOM_REPAIR:
        case SQLCOM_CHECK:
        case SQLCOM_SHOW_CREATE:
          break;
        default:
          closefrm(table, false);
          destroy(table);
          my_free(table);
          my_error(ER_CRASHED_ON_USAGE, MYF(0), share->table_name.str);
          goto err_lock;
      }
    }

    if (open_table_entry_fini(thd, share, table_def, table)) {
      closefrm(table, false);
      destroy(table);
      my_free(table);
      goto err_lock;
    }
  }
  {
    /* Add new TABLE object to table cache for this connection. */
    Table_cache *tc = table_cache_manager.get_cache(thd);

    tc->lock();

    if (tc->add_used_table(thd, table)) {
      tc->unlock();
      goto err_lock;
    }
    tc->unlock();
  }
  thd->status_var.table_open_cache_misses++;

table_found:
  table->mdl_ticket = mdl_ticket;

  table->next = thd->open_tables; /* Link into simple list */
  thd->set_open_tables(table);

  table->reginfo.lock_type = TL_READ; /* Assume read */

reset:
  table->reset();
  table->set_created();
  /*
    Check that there is no reference to a condition from an earlier query
    (cf. Bug#58553).
  */
  DBUG_ASSERT(table->file->pushed_cond == nullptr);
  table_list
      ->set_updatable();  // It is not derived table nor non-updatable VIEW
  table_list->set_insertable();

  table_list->table = table;

  /*
    Position for each partition in the bitmap is read from the Handler_share
    instance of the table. In MYSQL_OPEN_NO_NEW_TABLE_IN_SE mode, table is not
    opened in the SE and Handler_share instance for it is not created. Hence
    skipping partitions bitmap setting in the MYSQL_OPEN_NO_NEW_TABLE_IN_SE
    mode.
  */
  if (!(flags & MYSQL_OPEN_NO_NEW_TABLE_IN_SE)) {
    if (table->part_info) {
      /* Set all [named] partitions as used. */
      if (table->part_info->set_partition_bitmaps(table_list)) return true;
    } else if (table_list->partition_names) {
      /* Don't allow PARTITION () clause on a nonpartitioned table */
      my_error(ER_PARTITION_CLAUSE_ON_NONPARTITIONED, MYF(0));
      return true;
    }
  }

  table->init(thd, table_list);

  /* Request a read lock for implicitly opened P_S tables. */
  if (in_LTM(thd) && table_list->table->file->get_lock_type() == F_UNLCK &&
      belongs_to_p_s(table_list)) {
    table_list->table->file->ha_external_lock(thd, F_RDLCK);
  }

  return false;

err_lock:
  mysql_mutex_lock(&LOCK_open);
  release_table_share(share);
  mysql_mutex_unlock(&LOCK_open);

  return true;
}

/**
   Find table in the list of open tables.

   @param list       List of TABLE objects to be inspected.
   @param db         Database name
   @param table_name Table name

   @return Pointer to the TABLE object found, 0 if no table found.
*/

TABLE *find_locked_table(TABLE *list, const char *db, const char *table_name) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length = create_table_def_key(db, table_name, key);

  for (TABLE *table = list; table; table = table->next) {
    if (table->s->table_cache_key.length == key_length &&
        !memcmp(table->s->table_cache_key.str, key, key_length))
      return table;
  }
  return (nullptr);
}

/**
   Find instance of TABLE with upgradable or exclusive metadata
   lock from the list of open tables, emit error if no such table
   found.

   @param thd        Thread context
   @param db         Database name.
   @param table_name Name of table.
   @param no_error   Don't emit error if no suitable TABLE
                     instance were found.

   @note This function checks if the connection holds a global IX
         metadata lock. If no such lock is found, it is not safe to
         upgrade the lock and ER_TABLE_NOT_LOCKED_FOR_WRITE will be
         reported.

   @return Pointer to TABLE instance with MDL_SHARED_UPGRADABLE
           MDL_SHARED_NO_WRITE, MDL_SHARED_NO_READ_WRITE, or
           MDL_EXCLUSIVE metadata lock, NULL otherwise.
*/

TABLE *find_table_for_mdl_upgrade(THD *thd, const char *db,
                                  const char *table_name, bool no_error) {
  TABLE *tab = find_locked_table(thd->open_tables, db, table_name);

  if (!tab) {
    if (!no_error) my_error(ER_TABLE_NOT_LOCKED, MYF(0), table_name);
    return nullptr;
  }

  /*
    It is not safe to upgrade the metadata lock without a global IX lock.
    This can happen with FLUSH TABLES <list> WITH READ LOCK as we in these
    cases don't take a global IX lock in order to be compatible with
    global read lock.
  */
  if (!thd->mdl_context.owns_equal_or_stronger_lock(MDL_key::GLOBAL, "", "",
                                                    MDL_INTENTION_EXCLUSIVE)) {
    if (!no_error) my_error(ER_TABLE_NOT_LOCKED_FOR_WRITE, MYF(0), table_name);
    return nullptr;
  }

  while (tab->mdl_ticket != nullptr &&
         !tab->mdl_ticket->is_upgradable_or_exclusive() &&
         (tab = find_locked_table(tab->next, db, table_name)))
    continue;

  if (!tab && !no_error)
    my_error(ER_TABLE_NOT_LOCKED_FOR_WRITE, MYF(0), table_name);

  return tab;
}

/*
  Function to assign a new table map id to a table share.

  PARAMETERS

    share - Pointer to table share structure

  DESCRIPTION

    We are intentionally not checking that share->mutex is locked
    since this function should only be called when opening a table
    share and before it is entered into the table_def_cache (meaning
    that it cannot be fetched by another thread, even accidentally).

  PRE-CONDITION(S)

    share is non-NULL
    The LOCK_open mutex is locked.

  POST-CONDITION(S)

    share->table_map_id is given a value that with a high certainty is
    not used by any other table (the only case where a table id can be
    reused is on wrap-around, which means more than 2^48 table
    share opens have been executed while one table was open all the
    time).

*/
static Table_id last_table_id;

void assign_new_table_id(TABLE_SHARE *share) {
  DBUG_TRACE;

  /* Preconditions */
  DBUG_ASSERT(share != nullptr);
  mysql_mutex_assert_owner(&LOCK_open);

  DBUG_EXECUTE_IF("dbug_table_map_id_500", last_table_id = 500;);
  DBUG_EXECUTE_IF("dbug_table_map_id_4B_UINT_MAX+501",
                  last_table_id = 501ULL + UINT_MAX;);
  DBUG_EXECUTE_IF("dbug_table_map_id_6B_UINT_MAX",
                  last_table_id = (~0ULL >> 16););

  share->table_map_id = last_table_id++;
  DBUG_PRINT("info", ("table_id=%llu", share->table_map_id.id()));
}

/**
  Compare metadata versions of an element obtained from the table
  definition cache and its corresponding node in the parse tree.

  @details If the new and the old values mismatch, invoke
  Metadata_version_observer.
  At prepared statement prepare, all TABLE_LIST version values are
  NULL and we always have a mismatch. But there is no observer set
  in THD, and therefore no error is reported. Instead, we update
  the value in the parse tree, effectively recording the original
  version.
  At prepared statement execute, an observer may be installed.  If
  there is a version mismatch, we push an error and return true.

  For conventional execution (no prepared statements), the
  observer is never installed.

  @sa Execute_observer
  @sa check_prepared_statement() to see cases when an observer is installed
  @sa TABLE_LIST::is_table_ref_id_equal()
  @sa TABLE_SHARE::get_table_ref_id()

  @param[in]      thd         used to report errors
  @param[in,out]  tables      TABLE_LIST instance created by the parser
                              Metadata version information in this object
                              is updated upon success.
  @param[in]      table_share an element from the table definition cache

  @retval  true  an error, which has been reported
  @retval  false success, version in TABLE_LIST has been updated
*/

static bool check_and_update_table_version(THD *thd, TABLE_LIST *tables,
                                           TABLE_SHARE *table_share) {
  if (!tables->is_table_ref_id_equal(table_share)) {
    Reprepare_observer *reprepare_observer = thd->get_reprepare_observer();

    if (reprepare_observer && reprepare_observer->report_error(thd)) {
      /*
        Version of the table share is different from the
        previous execution of the prepared statement, and it is
        unacceptable for this SQLCOM. Error has been reported.
      */
      DBUG_ASSERT(thd->is_error());
      return true;
    }
    /* Always maintain the latest version and type */
    tables->set_table_ref_id(table_share);
  }
  return false;
}

/**
  Compares versions of a stored routine obtained from the sp cache
  and the version used at prepare.

  @details If the new and the old values mismatch, invoke
  Metadata_version_observer.
  At prepared statement prepare, all Sroutine_hash_entry version values
  are NULL and we always have a mismatch. But there is no observer set
  in THD, and therefore no error is reported. Instead, we update
  the value in Sroutine_hash_entry, effectively recording the original
  version.
  At prepared statement execute, an observer may be installed.  If
  there is a version mismatch, we push an error and return true.

  For conventional execution (no prepared statements), the
  observer is never installed.

  @param[in]      thd         used to report errors
  @param[in,out]  rt          pointer to stored routine entry in the
                              parse tree
  @param[in]      sp          pointer to stored routine cache entry.
                              Can be NULL if there is no such routine.
  @retval  true  an error, which has been reported
  @retval  false success, version in Sroutine_hash_entry has been updated
*/

static bool check_and_update_routine_version(THD *thd, Sroutine_hash_entry *rt,
                                             sp_head *sp) {
  int64 spc_version = sp_cache_version();
  /* sp is NULL if there is no such routine. */
  int64 version = sp ? sp->sp_cache_version() : spc_version;
  /*
    If the version in the parse tree is stale,
    or the version in the cache is stale and sp is not used,
    we need to reprepare.
    Sic: version != spc_version <--> sp is not NULL.
  */
  if (rt->m_cache_version != version ||
      (version != spc_version && !sp->is_invoked())) {
    Reprepare_observer *reprepare_observer = thd->get_reprepare_observer();

    if (reprepare_observer && reprepare_observer->report_error(thd)) {
      /*
        Version of the sp cache is different from the
        previous execution of the prepared statement, and it is
        unacceptable for this SQLCOM. Error has been reported.
      */
      DBUG_ASSERT(thd->is_error());
      return true;
    }
    /* Always maintain the latest cache version. */
    rt->m_cache_version = version;
  }
  return false;
}

/**
   Open view by getting its definition from disk (and table cache in future).

   @param thd               Thread handle
   @param table_list        TABLE_LIST with db, table_name & belong_to_view
   @param cache_key         Key for table definition cache
   @param cache_key_length  Length of cache_key

   @todo This function is needed for special handling of views under
         LOCK TABLES. We probably should get rid of it in long term.

   @return false if success, true - otherwise.
*/

static bool tdc_open_view(THD *thd, TABLE_LIST *table_list,
                          const char *cache_key, size_t cache_key_length) {
  TABLE_SHARE *share;

  mysql_mutex_lock(&LOCK_open);

  if (!(share = get_table_share(thd, table_list->db, table_list->table_name,
                                cache_key, cache_key_length, true))) {
    mysql_mutex_unlock(&LOCK_open);
    return true;
  }

  /*
    Check TABLE_SHARE-version of view only if we have been instructed to do
    so. We do not need to check the version if we're executing CREATE VIEW or
    ALTER VIEW statements.

    In the future, this functionality should be moved out from
    tdc_open_view(), and  tdc_open_view() should became a part of a clean
    table-definition-cache interface.
  */
  if (check_and_update_table_version(thd, table_list, share)) {
    release_table_share(share);
    mysql_mutex_unlock(&LOCK_open);
    return true;
  }

  if (share->is_view) {
    bool view_open_result = open_and_read_view(thd, share, table_list);

    release_table_share(share);
    mysql_mutex_unlock(&LOCK_open);

    if (view_open_result) return true;

    return parse_view_definition(thd, table_list);
  }

  my_error(ER_WRONG_OBJECT, MYF(0), share->db.str, share->table_name.str,
           "VIEW");
  release_table_share(share);
  mysql_mutex_unlock(&LOCK_open);
  return true;
}

/**
   Finalize the process of TABLE creation by loading table triggers
   and taking action if a HEAP table content was emptied implicitly.
*/

static bool open_table_entry_fini(THD *thd, TABLE_SHARE *share,
                                  const dd::Table *table, TABLE *entry) {
  if (table != nullptr && table->has_trigger()) {
    Table_trigger_dispatcher *d = Table_trigger_dispatcher::create(entry);

    if (!d || d->check_n_load(thd, *table)) {
      destroy(d);
      return true;
    }

    entry->triggers = d;
  }

  /*
    If we are here, there was no fatal error (but error may be still
    uninitialized).

    Ignore handling implicit_emptied property (which is only for heap
    tables) when I_S query is opening this table to read table statistics.
    The reason for avoiding this is that the
    mysql_bin_log.write_dml_directly() invokes a commit(). And this commit
    is not expected to be invoked when fetching I_S table statistics.
  */
  if (unlikely(entry->file->implicit_emptied) &&
      (!thd->lex || !thd->lex->m_IS_table_stats.is_reading_stats_by_open())) {
    entry->file->implicit_emptied = false;
    if (mysql_bin_log.is_open()) {
      bool error = false;
      String temp_buf;
      error = temp_buf.append("DELETE FROM ");
      append_identifier(thd, &temp_buf, share->db.str, strlen(share->db.str));
      error = temp_buf.append(".");
      append_identifier(thd, &temp_buf, share->table_name.str,
                        strlen(share->table_name.str));
      error = temp_buf.append(
          " /* generated by server, implicitly emptying in-memory table */");
      if (mysql_bin_log.write_dml_directly(thd, temp_buf.c_ptr_safe(),
                                           temp_buf.length()))
        return true;
      if (error) {
        /*
          As replication is maybe going to be corrupted, we need to warn the
          DBA on top of warning the client (which will automatically be done
          because of MYF(MY_WME) in my_malloc() above).
        */
        LogErr(ERROR_LEVEL,
               ER_BINLOG_OOM_WRITING_DELETE_WHILE_OPENING_HEAP_TABLE,
               share->db.str, share->table_name.str);
        destroy(entry->triggers);
        return true;
      }
    }
  }
  return false;
}

/**
   Auxiliary routine which is used for performing automatical table repair.
*/

static bool auto_repair_table(THD *thd, TABLE_LIST *table_list) {
  const char *cache_key;
  size_t cache_key_length;
  TABLE_SHARE *share;
  TABLE *entry;
  bool result = true;

  cache_key_length = get_table_def_key(table_list, &cache_key);

  thd->clear_error();

  mysql_mutex_lock(&LOCK_open);

  if (!(share = get_table_share(thd, table_list->db, table_list->table_name,
                                cache_key, cache_key_length, true)))
    goto end_unlock;

  if (share->is_view) {
    release_table_share(share);
    goto end_unlock;
  }

  if (!(entry =
            (TABLE *)my_malloc(key_memory_TABLE, sizeof(TABLE), MYF(MY_WME)))) {
    release_table_share(share);
    goto end_unlock;
  }
  mysql_mutex_unlock(&LOCK_open);

  if (open_table_from_share(thd, share, table_list->alias,
                            (uint)(HA_OPEN_KEYFILE | HA_OPEN_RNDFILE |
                                   HA_GET_INDEX | HA_TRY_READ_ONLY),
                            EXTRA_RECORD, ha_open_options | HA_OPEN_FOR_REPAIR,
                            entry, false, nullptr) ||
      !entry->file ||
      (entry->file->is_crashed() && entry->file->ha_check_and_repair(thd))) {
    /* Give right error message */
    thd->clear_error();
    my_error(ER_NOT_KEYFILE, MYF(0), share->table_name.str);
    LogErr(ERROR_LEVEL, ER_FAILED_TO_REPAIR_TABLE, share->db.str,
           share->table_name.str);
    if (entry->file) closefrm(entry, false);
  } else {
    thd->clear_error();  // Clear error message
    closefrm(entry, false);
    result = false;
  }
  my_free(entry);

  table_cache_manager.lock_all_and_tdc();
  release_table_share(share);
  /* Remove the repaired share from the table cache. */
  tdc_remove_table(thd, TDC_RT_REMOVE_ALL, table_list->db,
                   table_list->table_name, true);
  table_cache_manager.unlock_all_and_tdc();
  return result;
end_unlock:
  mysql_mutex_unlock(&LOCK_open);
  return result;
}

/**
  Error handler class for supressing HA_ERR_ROW_FORMAT_CHANGED errors from SE.
*/

class Fix_row_type_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    return sql_errno == ER_GET_ERRNO && my_errno() == HA_ERR_ROW_FORMAT_CHANGED;
  }
};

/**
  Auxiliary routine for automatically updating row format for the table.
*/

static bool fix_row_type(THD *thd, TABLE_LIST *table_list) {
  const char *cache_key;
  size_t cache_key_length = get_table_def_key(table_list, &cache_key);

  thd->clear_error();

  TABLE_SHARE *share;

  {
    /*
      Hold LOCK_open until we can keep it and are likely to
      release TABLE_SHARE on return.
    */
    MUTEX_LOCK(lock_open_guard, &LOCK_open);

    No_such_table_error_handler no_such_table_handler;
    thd->push_internal_handler(&no_such_table_handler);

    share = get_table_share(thd, table_list->db, table_list->table_name,
                            cache_key, cache_key_length, true);

    thd->pop_internal_handler();

    if (!share) {
      /*
        Somebody managed to drop table after we have performed back-off
        before trying to fix row format for the table. Such situation is
        quite unlikely but theoretically possible. Do not report error
        (silence it using error handler), let the caller try to reopen
        tables and handle missing table in appropriate way (e.g. ignore
        this fact it if the table comes from prelocking list).
      */
      if (no_such_table_handler.safely_trapped_errors()) return false;

      return true;
    }

    if (share->is_view) {
      /*
        Somebody managed to replace our table with a view after we
        have performed back-off before trying to fix row format for
        the table. Such situation is quite unlikely but is OK.
        Do not report error, let the caller try to reopen tables.
      */
      release_table_share(share);
      return false;
    }
  }

  int error = 0;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Table *table_def = nullptr;
  if (thd->dd_client()->acquire_for_modification(
          share->db.str, share->table_name.str, &table_def))
    error = 1;

  DBUG_ASSERT(table_def != nullptr);

  /*
    Silence expected HA_ERR_ROW_FORMAT_CHANGED errors.
  */
  Fix_row_type_error_handler err_handler;
  thd->push_internal_handler(&err_handler);

  TABLE tmp_table;
  if (error == 0)
    error = open_table_from_share(thd, share, table_list->alias,
                                  (uint)(HA_OPEN_KEYFILE | HA_OPEN_RNDFILE |
                                         HA_GET_INDEX | HA_TRY_READ_ONLY),
                                  EXTRA_RECORD, ha_open_options, &tmp_table,
                                  false, table_def);

  thd->pop_internal_handler();

  if (error == 8) {
    Disable_autocommit_guard autocommit_guard(thd);
    HA_CREATE_INFO create_info;
    create_info.row_type = share->row_type;
    create_info.table_options = share->db_options_in_use;

    handler *file = get_new_handler(share, share->m_part_info != nullptr,
                                    thd->mem_root, share->db_type());
    if (file != nullptr) {
      row_type correct_row_type = file->get_real_row_type(&create_info);
      bool result = dd::fix_row_type(thd, table_def, correct_row_type);
      destroy(file);

      if (result) {
        trans_rollback_stmt(thd);
        trans_rollback(thd);
      } else {
        result = trans_commit_stmt(thd) || trans_commit(thd);
        if (!result) error = 0;
      }
    }
  } else if (error == 0)
    closefrm(&tmp_table, false);

  table_cache_manager.lock_all_and_tdc();
  release_table_share(share);
  /*
    Remove the share from the table cache. So attempt to reopen table
    will construct its new version with correct real_row_type value.
  */
  tdc_remove_table(thd, TDC_RT_REMOVE_ALL, table_list->db,
                   table_list->table_name, true);
  table_cache_manager.unlock_all_and_tdc();
  return error != 0;
}

/** Open_table_context */

Open_table_context::Open_table_context(THD *thd, uint flags)
    : m_thd(thd),
      m_failed_table(nullptr),
      m_start_of_statement_svp(thd->mdl_context.mdl_savepoint()),
      m_timeout_nsec(flags & MYSQL_LOCK_IGNORE_TIMEOUT
                         ? LONG_TIMEOUT_NSEC
                         : thd->variables.lock_wait_timeout_nsec),
      m_flags(flags),
      m_action(OT_NO_ACTION),
      m_has_locks(thd->mdl_context.has_locks()),
      m_has_protection_against_grl(false) {}

/**
  Check if we can back-off and set back off action if we can.
  Otherwise report and return error.

  @retval  true if back-off is impossible.
  @retval  false if we can back off. Back off action has been set.
*/

bool Open_table_context::request_backoff_action(
    enum_open_table_action action_arg, TABLE_LIST *table) {
  /*
    A back off action may be one of four kinds:

    * We met a broken table that needs repair, or a table that
      is not present on this MySQL server and needs re-discovery.
      To perform the action, we need an exclusive metadata lock on
      the table. Acquiring X lock while holding other shared
      locks can easily lead to deadlocks. We rely on MDL deadlock
      detector to discover them. If this is a multi-statement
      transaction that holds metadata locks for completed statements,
      we should keep these locks after discovery/repair.
      The action type in this case is OT_DISCOVER or OT_REPAIR.
    * We met a table that has outdated value in ROW_FORMAT column
      in the data-dictionary/value of TABLE_SHARE::real_row_type
      attribute, which need to be updated. To update the
      data-dictionary we not only need to acquire X lock on the
      table, but also need to commit the transaction. If there
      is an ongoing transaction (and some metadata locks acquired)
      we cannot proceed and report an error. The action type for
      this case is OT_FIX_ROW_TYPE.
    * Our attempt to acquire an MDL lock lead to a deadlock,
      detected by the MDL deadlock detector. The current
      session was chosen a victim. If this is a multi-statement
      transaction that holds metadata locks taken by completed
      statements, restarting locking for the current statement
      may lead to a livelock. Releasing locks of completed
      statements can not be done as will lead to violation
      of ACID. Thus, again, if m_has_locks is set,
      we report an error. Otherwise, when there are no metadata
      locks other than which belong to this statement, we can
      try to recover from error by releasing all locks and
      restarting the pre-locking.
      Similarly, a deadlock error can occur when the
      pre-locking process met a TABLE_SHARE that is being
      flushed, and unsuccessfully waited for the flush to
      complete. A deadlock in this case can happen, e.g.,
      when our session is holding a metadata lock that
      is being waited on by a session which is using
      the table which is being flushed. The only way
      to recover from this error is, again, to close all
      open tables, release all locks, and retry pre-locking.
      Action type name is OT_REOPEN_TABLES. Re-trying
      while holding some locks may lead to a livelock,
      and thus we don't do it.
    * Finally, this session has open TABLEs from different
      "generations" of the table cache. This can happen, e.g.,
      when, after this session has successfully opened one
      table used for a statement, FLUSH TABLES interfered and
      expelled another table used in it. FLUSH TABLES then
      blocks and waits on the table already opened by this
      statement.
      We detect this situation by ensuring that table cache
      version of all tables used in a statement is the same.
      If it isn't, all tables needs to be reopened.
      Note, that we can always perform a reopen in this case,
      even if we already have metadata locks, since we don't
      keep tables open between statements and a livelock
      is not possible.
  */
  if ((action_arg == OT_BACKOFF_AND_RETRY || action_arg == OT_FIX_ROW_TYPE) &&
      m_has_locks) {
    my_error(ER_LOCK_DEADLOCK, MYF(0));
    m_thd->mark_transaction_to_rollback(true);
    return true;
  }
  /*
    If auto-repair or discovery are requested, a pointer to table
    list element must be provided.
  */
  if (table) {
    DBUG_ASSERT(action_arg == OT_DISCOVER || action_arg == OT_REPAIR ||
                action_arg == OT_FIX_ROW_TYPE);
    m_failed_table = new (m_thd->mem_root)
        TABLE_LIST(table->db, table->db_length, table->table_name,
                   table->table_name_length, table->alias, TL_WRITE);
    if (m_failed_table == nullptr) return true;
    m_failed_table->mdl_request.set_type(MDL_EXCLUSIVE);
  }
  m_action = action_arg;
  return false;
}

/**
  An error handler to mark transaction to rollback on DEADLOCK error
  during DISCOVER / REPAIR.
*/
class MDL_deadlock_discovery_repair_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *thd, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_LOCK_DEADLOCK) {
      thd->mark_transaction_to_rollback(true);
    }
    /*
      We have marked this transaction to rollback. Return false to allow
      error to be reported or handled by other handlers.
    */
    return false;
  }
};

/**
   Recover from failed attempt of open table by performing requested action.

   @pre This function should be called only with "action" != OT_NO_ACTION
        and after having called @sa close_tables_for_reopen().

   @retval false - Success. One should try to open tables once again.
   @retval true  - Error
*/

bool Open_table_context::recover_from_failed_open() {
  if (m_action == OT_REPAIR) {
    DEBUG_SYNC(m_thd, "recover_ot_repair");
  }

  /*
    Skip repair and discovery in IS-queries as they require X lock
    which could lead to delays or deadlock. Instead set
    ER_WARN_I_S_SKIPPED_TABLE which will be converted to a warning
    later.
   */
  if ((m_action == OT_REPAIR || m_action == OT_DISCOVER ||
       m_action == OT_FIX_ROW_TYPE) &&
      (m_flags & MYSQL_OPEN_FAIL_ON_MDL_CONFLICT)) {
    my_error(ER_WARN_I_S_SKIPPED_TABLE, MYF(0),
             m_failed_table->mdl_request.key.db_name(),
             m_failed_table->mdl_request.key.name());
    return true;
  }

  bool result = false;
  MDL_deadlock_discovery_repair_handler handler;
  /*
    Install error handler to mark transaction to rollback on DEADLOCK error.
  */
  m_thd->push_internal_handler(&handler);

  /* Execute the action. */
  switch (m_action) {
    case OT_BACKOFF_AND_RETRY:
      break;
    case OT_REOPEN_TABLES:
      break;
    case OT_DISCOVER: {
      if ((result = lock_table_names_nsec(m_thd, m_failed_table, nullptr,
                                          get_timeout_nsec(), 0)))
        break;

      tdc_remove_table(m_thd, TDC_RT_REMOVE_ALL, m_failed_table->db,
                       m_failed_table->table_name, false);
      if (ha_create_table_from_engine(m_thd, m_failed_table->db,
                                      m_failed_table->table_name)) {
        result = true;
        break;
      }

      m_thd->get_stmt_da()->reset_condition_info(m_thd);
      m_thd->clear_error();  // Clear error message
      /*
        Rollback to start of the current statement to release exclusive lock
        on table which was discovered but preserve locks from previous
        statements in current transaction.
      */
      m_thd->mdl_context.rollback_to_savepoint(start_of_statement_svp());
      break;
    }
    case OT_REPAIR: {
      if ((result = lock_table_names_nsec(m_thd, m_failed_table, nullptr,
                                          get_timeout_nsec(), 0)))
        break;

      tdc_remove_table(m_thd, TDC_RT_REMOVE_ALL, m_failed_table->db,
                       m_failed_table->table_name, false);

      result = auto_repair_table(m_thd, m_failed_table);
      /*
        Rollback to start of the current statement to release exclusive lock
        on table which was discovered but preserve locks from previous
        statements in current transaction.
      */
      m_thd->mdl_context.rollback_to_savepoint(start_of_statement_svp());
      break;
    }
    case OT_FIX_ROW_TYPE: {
      /*
        Since we are going to commit changes to the data-dictionary there
        should not be any ongoing transaction.
        We already have checked that the connection holds no metadata locks
        earlier.
        Still there can be transaction started by START TRANSACTION, which
        we don't have right to implicitly finish (even more interesting case
        is START TRANSACTION WITH CONSISTENT SNAPSHOT). Hence explicit check
        for active transaction.
      */
      DBUG_ASSERT(!m_thd->mdl_context.has_locks());

      if (m_thd->in_active_multi_stmt_transaction()) {
        my_error(ER_LOCK_OR_ACTIVE_TRANSACTION, MYF(0));
        result = true;
        break;
      }

      if ((result = lock_table_names_nsec(m_thd, m_failed_table, nullptr,
                                          get_timeout_nsec(), 0)))
        break;

      result = fix_row_type(m_thd, m_failed_table);

      m_thd->mdl_context.release_transactional_locks();
      break;
    }
    default:
      DBUG_ASSERT(0);
  }
  m_thd->pop_internal_handler();
  /*
    Reset the pointers to conflicting MDL request and the
    TABLE_LIST element, set when we need auto-discovery or repair,
    for safety.
  */
  m_failed_table = nullptr;
  /*
    Reset flag indicating that we have already acquired protection
    against GRL. It is no longer valid as the corresponding lock was
    released by close_tables_for_reopen().
  */
  m_has_protection_against_grl = false;
  /* Prepare for possible another back-off. */
  m_action = OT_NO_ACTION;
  return result;
}

/*
  Return a appropriate read lock type given a table object.

  @param thd              Thread context
  @param prelocking_ctx   Prelocking context.
  @param table_list       Table list element for table to be locked.
  @param routine_modifies_data
                          Some routine that is invoked by statement
                          modifies data.

  @remark Due to a statement-based replication limitation, statements such as
          INSERT INTO .. SELECT FROM .. and CREATE TABLE .. SELECT FROM need
          to grab a TL_READ_NO_INSERT lock on the source table in order to
          prevent the replication of a concurrent statement that modifies the
          source table. If such a statement gets applied on the slave before
          the INSERT .. SELECT statement finishes, data on the master could
          differ from data on the slave and end-up with a discrepancy between
          the binary log and table state.
          This also applies to SELECT/SET/DO statements which use stored
          functions. Calls to such functions are going to be logged as a
          whole and thus should be serialized against concurrent changes
          to tables used by those functions. This is avoided when functions
          do not modify data but only read it, since in this case nothing is
          written to the binary log. Argument routine_modifies_data
          denotes the same. So effectively, if the statement is not a
          LOCK TABLE, not a update query and routine_modifies_data is false
          then prelocking_placeholder does not take importance.

          Furthermore, this does not apply to I_S and log tables as it's
          always unsafe to replicate such tables under statement-based
          replication as the table on the slave might contain other data
          (ie: general_log is enabled on the slave). The statement will
          be marked as unsafe for SBR in decide_logging_format().
  @remark Note that even in prelocked mode it is important to correctly
          determine lock type value. In this mode lock type is passed to
          handler::start_stmt() method and can be used by storage engine,
          for example, to determine what kind of row locks it should acquire
          when reading data from the table.
*/

thr_lock_type read_lock_type_for_table(THD *thd,
                                       Query_tables_list *prelocking_ctx,
                                       TABLE_LIST *table_list,
                                       bool routine_modifies_data) {
  /*
    In cases when this function is called for a sub-statement executed in
    prelocked mode we can't rely on OPTION_BIN_LOG flag in THD::options
    bitmap to determine that binary logging is turned on as this bit can
    be cleared before executing sub-statement. So instead we have to look
    at THD::variables::sql_log_bin member.
  */
  bool log_on = mysql_bin_log.is_open() && thd->variables.sql_log_bin;

  /*
    When we do not write to binlog or when we use row based replication,
    it is safe to use a weaker lock.
  */
  if (log_on == false || thd->variables.binlog_format == BINLOG_FORMAT_ROW)
    return TL_READ;

  if ((table_list->table->s->table_category == TABLE_CATEGORY_LOG) ||
      (table_list->table->s->table_category == TABLE_CATEGORY_RPL_INFO) ||
      (table_list->table->s->table_category == TABLE_CATEGORY_GTID) ||
      (table_list->table->s->table_category == TABLE_CATEGORY_PERFORMANCE))
    return TL_READ;

  // SQL queries which updates data need a stronger lock.
  if (is_update_query(prelocking_ctx->sql_command)) return TL_READ_NO_INSERT;

  /*
    table_list is placeholder for prelocking.
    Ignore prelocking_placeholder status for non "LOCK TABLE" statement's
    table_list objects when routine_modifies_data is false.
  */
  if (table_list->prelocking_placeholder &&
      (routine_modifies_data || thd->in_lock_tables))
    return TL_READ_NO_INSERT;

  if (thd->locked_tables_mode > LTM_LOCK_TABLES) return TL_READ_NO_INSERT;

  return TL_READ;
}

/**
  Process table's foreign keys (if any) by prelocking algorithm.

  @param  thd                   Thread context.
  @param  prelocking_ctx        Prelocking context of the statement.
  @param  share                 Table's share.
  @param  is_insert             Indicates whether statement is going to INSERT
                                into the table.
  @param  is_update             Indicates whether statement is going to UPDATE
                                the table.
  @param  is_delete             Indicates whether statement is going to DELETE
                                from the table.
  @param  belong_to_view        Uppermost view which uses this table element
                                (nullptr - if it is not used by a view).
  @param[out] need_prelocking   Set to true if method detects that prelocking
                                required, not changed otherwise.
*/
static void process_table_fks(THD *thd, Query_tables_list *prelocking_ctx,
                              TABLE_SHARE *share, bool is_insert,
                              bool is_update, bool is_delete,
                              TABLE_LIST *belong_to_view,
                              bool *need_prelocking) {
  if (!share->foreign_keys && !share->foreign_key_parents) {
    /*
      This table doesn't participate in any foreign keys, so nothing to
      process.
    */
    return;
  }

  *need_prelocking = true;

  /*
    In lower-case-table-names == 2 mode we store original versions of db
    and table names for tables participating in FK relationship, even
    though their comparison is performed in case insensitive fashion.
    Therefore we need to normalize/lowercase these names while prelocking
    set key is constructing from them.
  */
  bool normalize_db_names = (lower_case_table_names == 2);
  Sp_name_normalize_type name_normalize_type =
      (lower_case_table_names == 2) ? Sp_name_normalize_type::LOWERCASE_NAME
                                    : Sp_name_normalize_type::LEAVE_AS_IS;

  if (is_insert || is_update) {
    for (TABLE_SHARE_FOREIGN_KEY_INFO *fk = share->foreign_key;
         fk < share->foreign_key + share->foreign_keys; ++fk) {
      (void)sp_add_used_routine(
          prelocking_ctx, thd->stmt_arena,
          Sroutine_hash_entry::FK_TABLE_ROLE_PARENT_CHECK,
          fk->referenced_table_db.str, fk->referenced_table_db.length,
          fk->referenced_table_name.str, fk->referenced_table_name.length,
          normalize_db_names, name_normalize_type, false, belong_to_view);
    }
  }

  if (is_update || is_delete) {
    for (TABLE_SHARE_FOREIGN_KEY_PARENT_INFO *fk_p = share->foreign_key_parent;
         fk_p < share->foreign_key_parent + share->foreign_key_parents;
         ++fk_p) {
      if ((is_update &&
           (fk_p->update_rule == dd::Foreign_key::RULE_NO_ACTION ||
            fk_p->update_rule == dd::Foreign_key::RULE_RESTRICT)) ||
          (is_delete &&
           (fk_p->delete_rule == dd::Foreign_key::RULE_NO_ACTION ||
            fk_p->delete_rule == dd::Foreign_key::RULE_RESTRICT))) {
        (void)sp_add_used_routine(
            prelocking_ctx, thd->stmt_arena,
            Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_CHECK,
            fk_p->referencing_table_db.str, fk_p->referencing_table_db.length,
            fk_p->referencing_table_name.str,
            fk_p->referencing_table_name.length, normalize_db_names,
            name_normalize_type, false, belong_to_view);
      }

      if ((is_update &&
           (fk_p->update_rule == dd::Foreign_key::RULE_CASCADE ||
            fk_p->update_rule == dd::Foreign_key::RULE_SET_NULL ||
            fk_p->update_rule == dd::Foreign_key::RULE_SET_DEFAULT)) ||
          (is_delete &&
           (fk_p->delete_rule == dd::Foreign_key::RULE_SET_NULL ||
            fk_p->delete_rule == dd::Foreign_key::RULE_SET_DEFAULT))) {
        (void)sp_add_used_routine(
            prelocking_ctx, thd->stmt_arena,
            Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_UPDATE,
            fk_p->referencing_table_db.str, fk_p->referencing_table_db.length,
            fk_p->referencing_table_name.str,
            fk_p->referencing_table_name.length, normalize_db_names,
            name_normalize_type, false, belong_to_view);
      }

      if (is_delete && fk_p->delete_rule == dd::Foreign_key::RULE_CASCADE) {
        (void)sp_add_used_routine(
            prelocking_ctx, thd->stmt_arena,
            Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_DELETE,
            fk_p->referencing_table_db.str, fk_p->referencing_table_db.length,
            fk_p->referencing_table_name.str,
            fk_p->referencing_table_name.length, normalize_db_names,
            name_normalize_type, false, belong_to_view);
      }
    }
  }
}

/**
  Handle element of prelocking set other than table. E.g. cache routine
  and, if prelocking strategy prescribes so, extend the prelocking set
  with tables and routines used by it.

  @param[in]  thd                   Thread context.
  @param[in]  prelocking_ctx        Prelocking context.
  @param[in]  rt                    Element of prelocking set to be processed.
  @param[in]  prelocking_strategy   Strategy which specifies how the
                                    prelocking set should be extended when
                                    one of its elements is processed.
  @param[in]  has_prelocking_list   Indicates that prelocking set/list for
                                    this statement has already been built.
  @param[in]  ot_ctx                Context of open_table used to recover from
                                    locking failures.
  @param[out] need_prelocking       Set to true if it was detected that this
                                    statement will require prelocked mode for
                                    its execution, not touched otherwise.
  @param[out] routine_modifies_data Set to true if it was detected that this
                                    routine does modify table data.

  @retval false  Success.
  @retval true   Failure (Conflicting metadata lock, OOM, other errors).
*/

static bool open_and_process_routine(
    THD *thd, Query_tables_list *prelocking_ctx, Sroutine_hash_entry *rt,
    Prelocking_strategy *prelocking_strategy, bool has_prelocking_list,
    Open_table_context *ot_ctx, bool *need_prelocking,
    bool *routine_modifies_data) {
  *routine_modifies_data = false;
  DBUG_TRACE;

  switch (rt->type()) {
    case Sroutine_hash_entry::FUNCTION:
    case Sroutine_hash_entry::PROCEDURE: {
      sp_head *sp;
      /*
        Try to get MDL lock on the routine.
        Note that we do not take locks on top-level CALLs as this can
        lead to a deadlock. Not locking top-level CALLs does not break
        the binlog as only the statements in the called procedure show
        up there, not the CALL itself.
      */
      if (rt != prelocking_ctx->sroutines_list.first ||
          rt->type() != Sroutine_hash_entry::PROCEDURE) {
        MDL_request mdl_request;
        MDL_key mdl_key;

        if (rt->type() == Sroutine_hash_entry::FUNCTION)
          dd::Function::create_mdl_key(rt->db(), rt->name(), &mdl_key);
        else
          dd::Procedure::create_mdl_key(rt->db(), rt->name(), &mdl_key);

        MDL_REQUEST_INIT_BY_KEY(&mdl_request, &mdl_key, MDL_SHARED,
                                MDL_TRANSACTION);

        /*
          Waiting for a conflicting metadata lock to go away may
          lead to a deadlock, detected by MDL subsystem.
          If possible, we try to resolve such deadlocks by releasing all
          metadata locks and restarting the pre-locking process.
          To prevent the error from polluting the Diagnostics Area
          in case of successful resolution, install a special error
          handler for ER_LOCK_DEADLOCK error.
        */
        MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);

        thd->push_internal_handler(&mdl_deadlock_handler);
        bool result = thd->mdl_context.acquire_lock_nsec(
            &mdl_request, ot_ctx->get_timeout_nsec());
        thd->pop_internal_handler();

        if (result) return true;

        DEBUG_SYNC(thd, "after_shared_lock_pname");

        /* Ensures the routine is up-to-date and cached, if exists. */
        if (sp_cache_routine(thd, rt, has_prelocking_list, &sp)) return true;

        /* Remember the version of the routine in the parse tree. */
        if (check_and_update_routine_version(thd, rt, sp)) return true;

        /* 'sp' is NULL when there is no such routine. */
        if (sp) {
          *routine_modifies_data = sp->modifies_data();

          if (!has_prelocking_list)
            prelocking_strategy->handle_routine(thd, prelocking_ctx, rt, sp,
                                                need_prelocking);
        }
      } else {
        /*
          If it's a top level call, just make sure we have a recent
          version of the routine, if it exists.
          Validating routine version is unnecessary, since CALL
          does not affect the prepared statement prelocked list.
        */
        if (sp_cache_routine(thd, rt, false, &sp)) return true;
      }
    } break;
    case Sroutine_hash_entry::TRIGGER:
      /**
        We add trigger entries to lex->sroutines_list, but we don't
        load them here. The trigger entry is only used when building
        a transitive closure of objects used in a statement, to avoid
        adding to this closure objects that are used in the trigger more
        than once.
        E.g. if a trigger trg refers to table t2, and the trigger table t1
        is used multiple times in the statement (say, because it's used in
        function f1() twice), we will only add t2 once to the list of
        tables to prelock.

        We don't take metadata locks on triggers either: they are protected
        by a respective lock on the table, on which the trigger is defined.

        The only two cases which give "trouble" are SHOW CREATE TRIGGER
        and DROP TRIGGER statements. For these, statement syntax doesn't
        specify the table on which this trigger is defined, so we have
        to make a "dirty" read in the data dictionary to find out the
        table name. Once we discover the table name, we take a metadata
        lock on it, and this protects all trigger operations.
        Of course the table, in theory, may disappear between the dirty
        read and metadata lock acquisition, but in that case we just return
        a run-time error.

        Grammar of other trigger DDL statements (CREATE, DROP) requires
        the table to be specified explicitly, so we use the table metadata
        lock to protect trigger metadata in these statements. Similarly, in
        DML we always use triggers together with their tables, and thus don't
        need to take separate metadata locks on them.
      */
      break;
    case Sroutine_hash_entry::FK_TABLE_ROLE_PARENT_CHECK:
    case Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_CHECK:
    case Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_UPDATE:
    case Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_DELETE: {
      if (thd->locked_tables_mode == LTM_NONE) {
        MDL_request mdl_request;

        /*
          Adjust metadata lock type according to the table's role in the
          FK relationship. Also acquire stronger locks when we are locking
          on behalf of LOCK TABLES.
        */
        enum_mdl_type mdl_lock_type;
        bool executing_LT = (prelocking_ctx->sql_command == SQLCOM_LOCK_TABLES);

        if (rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_PARENT_CHECK ||
            rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_CHECK) {
          mdl_lock_type =
              (executing_LT ? MDL_SHARED_READ_ONLY : MDL_SHARED_READ);
        } else {
          mdl_lock_type =
              (executing_LT ? MDL_SHARED_NO_READ_WRITE : MDL_SHARED_WRITE);
        }

        MDL_REQUEST_INIT_BY_PART_KEY(&mdl_request, MDL_key::TABLE,
                                     rt->part_mdl_key(),
                                     rt->part_mdl_key_length(), rt->db_length(),
                                     mdl_lock_type, MDL_TRANSACTION);

        MDL_deadlock_handler mdl_deadlock_handler(ot_ctx);

        thd->push_internal_handler(&mdl_deadlock_handler);
        bool result = thd->mdl_context.acquire_lock_nsec(
            &mdl_request, ot_ctx->get_timeout_nsec());
        thd->pop_internal_handler();

        if (result) return true;
      } else {
        /*
          This function is called only if we are not in prelocked mode
          already. So we must be handling statement executed under
          LOCK TABLES in this case.
        */
        DBUG_ASSERT(thd->locked_tables_mode == LTM_LOCK_TABLES);

        /*
          Even though LOCK TABLES tries to automatically lock parent and child
          tables which might be necessary for foreign key checks/actions, there
          are some cases when we might miss them. So it is better to check that
          we have appropriate metadata lock explicitly and error out if not.

          Some examples of problematic cases are:

          *) We are executing DELETE FROM t1 under LOCK TABLES t1 READ
             and table t1 is a parent in a foreign key.
             In this case error about inappropriate lock on t1 will be
             reported at later stage than prelocking set is built.
             So we can't assume/assert that we have proper lock on the
             corresponding child table here.

         *)  Table t1 has a trigger, which contains DELETE FROM t2 and
             t2 is participating in FK as parent. In such situation
             LOCK TABLE t1 WRITE will lock t2 for write implicitly
             so both updates and delete on t2 will be allowed. However,
             t3 will be locked only in a way as if only deletes from
             t2 were allowed.

          *) Prelocking list has been built earlier. Both child and parent
             definitions might have changed since this time so at LOCK TABLES
             time FK which corresponds to this element of prelocked set
             might be no longer around. In theory, we might be processing
         statement which is not marked as requiring prelocked set invalidation
         (and thus ignoring table version mismatches) or tables might be missing
         and this error can be suppressed. In such case we might not have
         appropriate metadata lock on our child/parent table.
        */
        if (rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_PARENT_CHECK ||
            rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_CHECK) {
          if (!thd->mdl_context.owns_equal_or_stronger_lock(
                  MDL_key::TABLE, rt->db(), rt->name(), MDL_SHARED_READ_ONLY)) {
            my_error(ER_TABLE_NOT_LOCKED, MYF(0), rt->name());
            return true;
          }
        } else {
          if (!thd->mdl_context.owns_equal_or_stronger_lock(
                  MDL_key::TABLE, rt->db(), rt->name(),
                  MDL_SHARED_NO_READ_WRITE)) {
            my_error(ER_TABLE_NOT_LOCKED_FOR_WRITE, MYF(0), rt->name());
            return true;
          }
        }
      }

      if (rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_UPDATE ||
          rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_DELETE) {
        /*
          In order to continue building prelocked set or validating
          prelocked set which already has been built we need to get
          access to table's TABLE_SHARE.

          Getting unused TABLE object is more scalable that going
          directly for the TABLE_SHARE. If there are no unused TABLE
          object we might get at least pointer to the TABLE_SHARE
          from the table cache.

          Note that under LOCK TABLES we can't rely on that table is
          going to be in THD::open_tables list, as LOCK TABLES only
          pre-acquires metadata locks on FK tables but doesn't
          pre-open them.

          TODO: Perhaps we should give it a try as it can be more
                scalability friendly.
        */
        Table_cache *tc = table_cache_manager.get_cache(thd);
        TABLE *table;
        TABLE_SHARE *share;

        tc->lock();

        table = tc->get_table(thd, rt->part_mdl_key(),
                              rt->part_mdl_key_length(), &share);

        if (table) {
          DBUG_ASSERT(table->s == share);
          /*
            Don't check if TABLE_SHARE::version matches version of tables
            previously opened by this statement. It might be problematic
            under LOCK TABLES and possible version difference can't affect
            FK-related part of prelocking set.
          */
          tc->unlock();
        } else if (share) {
          /*
            TODO: If we constantly hit this case it would harm scalability...
                  Perhaps we need to create new unused TABLE instance in this
                  case.
          */
          mysql_mutex_lock(&LOCK_open);
          tc->unlock();
          share->increment_ref_count();
          mysql_mutex_unlock(&LOCK_open);

          /*
            Again, when building part of prelocking set related to foreign keys
            we can ignore fact that TABLE_SHARE::version is old.
          */
        } else {
          tc->unlock();

          /*
            If we are validating existing prelocking set then the table
            might have been dropped. We suppress this error in this case.
            Prelocking set will be either invalidated, or error will be
            reported the parent table is accessed.

            TODO: Perhaps we need to use get_table_share_with_discover()
                  here but it gets complicated under LOCK TABLES.
          */
          No_such_table_error_handler no_such_table_handler;
          thd->push_internal_handler(&no_such_table_handler);

          mysql_mutex_lock(&LOCK_open);
          share = get_table_share(thd, rt->db(), rt->name(), rt->part_mdl_key(),
                                  rt->part_mdl_key_length(), true);
          mysql_mutex_unlock(&LOCK_open);

          thd->pop_internal_handler();

          if (!share && no_such_table_handler.safely_trapped_errors()) {
            break;  // Jump out switch without error.
          }

          if (!share) {
            return true;
          }

          if (share->is_view) {
            /*
              Eeek! Somebody replaced the child table with a view. This can
              happen only when we are validating existing prelocked set.
              Parent either have been dropped or its definition has been
              changed. In either case our child table won't be accessed
              through the foreign key.
            */
            DBUG_ASSERT(has_prelocking_list);

            mysql_mutex_lock(&LOCK_open);
            release_table_share(share);
            mysql_mutex_unlock(&LOCK_open);

            Reprepare_observer *reprepare_observer =
                thd->get_reprepare_observer();
            if (reprepare_observer && reprepare_observer->report_error(thd)) {
              DBUG_ASSERT(thd->is_error());
              return true;
            }

            break;  // Jump out switch without error.
          }
        }

        auto release_table_lambda = [thd](TABLE *tab) {
          release_or_close_table(thd, tab);
        };
        std::unique_ptr<TABLE, decltype(release_table_lambda)>
            release_table_guard(table, release_table_lambda);

        /*
          We need to explicitly release TABLE_SHARE only if we don't
          have TABLE object.
        */
        auto release_share_lambda = [](TABLE_SHARE *tsh) {
          mysql_mutex_lock(&LOCK_open);
          release_table_share(tsh);
          mysql_mutex_unlock(&LOCK_open);
        };
        std::unique_ptr<TABLE_SHARE, decltype(release_share_lambda)>
            release_share_guard((table ? nullptr : share),
                                release_share_lambda);

        /*
          We need to maintain versioning of the prelocked tables since this
          is needed for correct handling of prepared statements to catch
          situations where a prelocked table (which is added to the prelocked
          set during PREPARE) is changed between repeated executions of the
          prepared statement.
         */
        int64 share_version = share->get_table_ref_version();

        if (rt->m_cache_version != share_version) {
          Reprepare_observer *reprepare_observer =
              thd->get_reprepare_observer();

          if (reprepare_observer && reprepare_observer->report_error(thd)) {
            /*
              Version of the cached table share is different from the
              previous execution of the prepared statement, and it is
              unacceptable for this SQLCOM. Error has been reported.
            */
            DBUG_ASSERT(thd->is_error());
            return true;
          }
          /* Always maintain the latest cache version. */
          rt->m_cache_version = share_version;
        }

        if (!has_prelocking_list) {
          bool is_update =
              (rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_UPDATE);
          bool is_delete =
              (rt->type() == Sroutine_hash_entry::FK_TABLE_ROLE_CHILD_DELETE);

          process_table_fks(thd, prelocking_ctx, share, false, is_update,
                            is_delete, rt->belong_to_view, need_prelocking);
        }
      }
    } break;
    default:
      /* Impossible type value. */
      DBUG_ASSERT(0);
  }
  return false;
}

/**
  Handle table list element by obtaining metadata lock, opening table or view
  and, if prelocking strategy prescribes so, extending the prelocking set with
  tables and routines used by it.

  @param[in]     thd                  Thread context.
  @param[in]     lex                  LEX structure for statement.
  @param[in]     tables               Table list element to be processed.
  @param[in,out] counter              Number of tables which are open.
  @param[in]     prelocking_strategy  Strategy which specifies how the
                                      prelocking set should be extended
                                      when table or view is processed.
  @param[in]     has_prelocking_list  Indicates that prelocking set/list for
                                      this statement has already been built.
  @param[in]     ot_ctx               Context used to recover from a failed
                                      open_table() attempt.

  @retval  false  Success.
  @retval  true   Error, reported unless there is a chance to recover from it.
*/

static bool open_and_process_table(THD *thd, LEX *lex, TABLE_LIST *const tables,
                                   uint *counter,
                                   Prelocking_strategy *prelocking_strategy,
                                   bool has_prelocking_list,
                                   Open_table_context *ot_ctx) {
  bool error = false;
  bool safe_to_ignore_table = false;
  DBUG_TRACE;
  DEBUG_SYNC(thd, "open_and_process_table");

  /*
    Ignore placeholders for derived tables. After derived tables
    processing, link to created temporary table will be put here.
    If this is derived table for view then we still want to process
    routines used by this view; for a non-view derived table, those routines
    are already part of the containing query's structures.
  */
  if (tables->is_derived() || tables->is_table_function()) goto end;

  DBUG_ASSERT(!tables->common_table_expr());

  /*
    If this TABLE_LIST object is a placeholder for an information_schema
    table, create a temporary table to represent the information_schema
    table in the query. Do not fill it yet - will be filled during
    execution.
  */
  if (tables->schema_table) {
    /*
      Since we no longer set TABLE_LIST::schema_table/table for table
      list elements representing mergeable view, we can't meet a table
      list element which represent information_schema table and a view
      at the same time. Otherwise, acquiring metadata lock om the view
      would have been necessary.
    */
    DBUG_ASSERT(!tables->is_view());

    if (!mysql_schema_table(thd, lex, tables) &&
        !check_and_update_table_version(thd, tables, tables->table->s)) {
      goto end;
    }
    error = true;
    goto end;
  }
  DBUG_PRINT("tcache", ("opening table: '%s'.'%s'  item: %p", tables->db,
                        tables->table_name,
                        tables));  // psergey: invalid read of size 1 here
  (*counter)++;

  /* Not a placeholder: must be a base/temporary table or a view. Let us open
   * it. */

  if (tables->table) {
    /*
      If this TABLE_LIST object has an associated open TABLE object
      (TABLE_LIST::table is not NULL), that TABLE object must be a pre-opened
      temporary table.
    */
    DBUG_ASSERT(is_temporary_table(tables));
  } else if (tables->open_type == OT_TEMPORARY_ONLY) {
    /*
      OT_TEMPORARY_ONLY means that we are in CREATE TEMPORARY TABLE statement.
      Also such table list element can't correspond to prelocking placeholder
      or to underlying table of merge table.
      So existing temporary table should have been preopened by this moment
      and we can simply continue without trying to open temporary or base
      table.
    */
    DBUG_ASSERT(tables->open_strategy);
    DBUG_ASSERT(!tables->prelocking_placeholder);
    DBUG_ASSERT(!tables->parent_l);
  } else if (tables->prelocking_placeholder) {
    /*
      For the tables added by the pre-locking code, attempt to open
      the table but fail silently if the table does not exist.
      The real failure will occur when/if a statement attempts to use
      that table.
    */
    No_such_table_error_handler no_such_table_handler;
    thd->push_internal_handler(&no_such_table_handler);

    /*
      We're opening a table from the prelocking list.

      Since this table list element might have been added after pre-opening
      of temporary tables we have to try to open temporary table for it.

      We can't simply skip this table list element and postpone opening of
      temporary tabletill the execution of substatement for several reasons:
      - Temporary table can be a MERGE table with base underlying tables,
        so its underlying tables has to be properly open and locked at
        prelocking stage.
      - Temporary table can be a MERGE table and we might be in PREPARE
        phase for a prepared statement. In this case it is important to call
        HA_ATTACH_CHILDREN for all merge children.
        This is necessary because merge children remember "TABLE_SHARE ref type"
        and "TABLE_SHARE def version" in the HA_ATTACH_CHILDREN operation.
        If HA_ATTACH_CHILDREN is not called, these attributes are not set.
        Then, during the first EXECUTE, those attributes need to be updated.
        That would cause statement re-preparing (because changing those
        attributes during EXECUTE is caught by THD::m_reprepare_observers).
        The problem is that since those attributes are not set in merge
        children, another round of PREPARE will not help.
    */
    error = open_temporary_table(thd, tables);

    if (!error && !tables->table) error = open_table(thd, tables, ot_ctx);

    thd->pop_internal_handler();
    safe_to_ignore_table = no_such_table_handler.safely_trapped_errors();
  } else if (tables->parent_l && (thd->open_options & HA_OPEN_FOR_REPAIR)) {
    /*
      Also fail silently for underlying tables of a MERGE table if this
      table is opened for CHECK/REPAIR TABLE statement. This is needed
      to provide complete list of problematic underlying tables in
      CHECK/REPAIR TABLE output.
    */
    Repair_mrg_table_error_handler repair_mrg_table_handler;
    thd->push_internal_handler(&repair_mrg_table_handler);

    error = open_temporary_table(thd, tables);
    if (!error && !tables->table) error = open_table(thd, tables, ot_ctx);

    thd->pop_internal_handler();
    safe_to_ignore_table = repair_mrg_table_handler.safely_trapped_errors();
  } else {
    if (tables->parent_l) {
      /*
        Even if we are opening table not from the prelocking list we
        still might need to look for a temporary table if this table
        list element corresponds to underlying table of a merge table.
      */
      error = open_temporary_table(thd, tables);
    }

    if (!error && !tables->table) error = open_table(thd, tables, ot_ctx);
  }

  if (error) {
    if (!ot_ctx->can_recover_from_failed_open() && safe_to_ignore_table) {
      DBUG_PRINT("info", ("open_table: ignoring table '%s'.'%s'", tables->db,
                          tables->alias));
      error = false;
    }
    goto end;
  }

  /*
    We can't rely on simple check for TABLE_LIST::is_view() to determine
    that this is a view since during re-execution we might reopen
    ordinary table in place of view and thus have TABLE_LIST::view
    set from repvious execution and TABLE_LIST::table set from
    current.
  */
  if (!tables->table && tables->is_view()) {
    /* VIEW placeholder */
    (*counter)--;

    /*
      tables->next_global list consists of two parts:
      1) Query tables and underlying tables of views.
      2) Tables used by all stored routines that this statement invokes on
         execution.
      We need to know where the bound between these two parts is. If we've
      just opened a view, which was the last table in part #1, and it
      has added its base tables after itself, adjust the boundary pointer
      accordingly.
    */
    if (lex->query_tables_own_last == &(tables->next_global) &&
        tables->view_query()->query_tables)
      lex->query_tables_own_last = tables->view_query()->query_tables_last;
    /*
      Let us free memory used by 'sroutines' hash here since we never
      call destructor for this LEX.
    */
    tables->view_query()->sroutines.reset();
    goto process_view_routines;
  }

  /*
    Special types of open can succeed but still don't set
    TABLE_LIST::table to anything.
  */
  if (tables->open_strategy && !tables->table) goto end;

  /*
    If we are not already in prelocked mode and extended table list is not
    yet built we might have to build the prelocking set for this statement.

    Since currently no prelocking strategy prescribes doing anything for
    tables which are only read, we do below checks only if table is going
    to be changed.
  */
  if (thd->locked_tables_mode <= LTM_LOCK_TABLES && !has_prelocking_list &&
      tables->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) {
    bool need_prelocking = false;
    TABLE_LIST **save_query_tables_last = lex->query_tables_last;
    /*
      Extend statement's table list and the prelocking set with
      tables and routines according to the current prelocking
      strategy.

      For example, for DML statements we need to add tables and routines
      used by triggers which are going to be invoked for this element of
      table list and also add tables required for handling of foreign keys.
    */
    error =
        prelocking_strategy->handle_table(thd, lex, tables, &need_prelocking);

    if (need_prelocking && !lex->requires_prelocking())
      lex->mark_as_requiring_prelocking(save_query_tables_last);

    if (error) goto end;
  }

  /* Check and update metadata version of a base table. */
  error = check_and_update_table_version(thd, tables, tables->table->s);

  if (error) goto end;
  /*
    After opening a MERGE table add the children to the query list of
    tables, so that they are opened too.
    Note that placeholders don't have the handler open.
  */
  /* MERGE tables need to access parent and child TABLE_LISTs. */
  DBUG_ASSERT(tables->table->pos_in_table_list == tables);
  /* Non-MERGE tables ignore this call. */
  if (tables->table->db_stat &&
      tables->table->file->ha_extra(HA_EXTRA_ADD_CHILDREN_LIST)) {
    error = true;
    goto end;
  }

process_view_routines:
  /*
    Again we may need cache all routines used by this view and add
    tables used by them to table list.
  */
  if (tables->is_view() && thd->locked_tables_mode <= LTM_LOCK_TABLES &&
      !has_prelocking_list) {
    bool need_prelocking = false;
    TABLE_LIST **save_query_tables_last = lex->query_tables_last;

    error =
        prelocking_strategy->handle_view(thd, lex, tables, &need_prelocking);

    if (need_prelocking && !lex->requires_prelocking())
      lex->mark_as_requiring_prelocking(save_query_tables_last);

    if (error) goto end;
  }

end:
  return error;
}

namespace {

struct schema_hash {
  size_t operator()(const TABLE_LIST *table) const {
    return std::hash<std::string>()(std::string(table->db, table->db_length));
  }
};

struct schema_key_equal {
  bool operator()(const TABLE_LIST *a, const TABLE_LIST *b) const {
    return a->db_length == b->db_length &&
           memcmp(a->db, b->db, a->db_length) == 0;
  }
};

}  // namespace

/**
  Run the server hook called "before_dml". This is a hook originated from
  replication that allow server plugins to execute code before any DML
  instruction is executed.
  In case of negative outcome, it will set my_error to
  ER_BEFORE_DML_VALIDATION_ERROR

  @param thd Thread context

  @return hook outcome
    @retval 0    Everything is fine
    @retval !=0  Error in the outcome of the hook.
 */
int run_before_dml_hook(THD *thd) {
  int out_value = 0;

  TX_TRACKER_GET(tst);
  tst->add_trx_state(thd, TX_STMT_DML);

  if (opt_group_replication_plugin_hooks) {
    (void)RUN_HOOK(transaction, before_dml, (thd, out_value));
  }

  if (out_value) {
    tst->clear_trx_state(thd, TX_STMT_DML);
    my_error(ER_BEFORE_DML_VALIDATION_ERROR, MYF(0));
  }

  return out_value;
}

/**
  Check whether a table being opened is a temporary table.

  @param table  table being opened

  @return true if a table is temporary table, else false
*/

static inline bool is_temporary_table_being_opened(const TABLE_LIST *table) {
  return table->open_type == OT_TEMPORARY_ONLY ||
         (table->open_type == OT_TEMPORARY_OR_BASE &&
          is_temporary_table(table));
}

/**
  Acquire IX metadata locks on tablespace names used by LOCK
  TABLES or by a DDL statement.

  @note That the tablespace MDL locks are taken only after locks
  on tables are acquired. So it is recommended to maintain this
  same lock order across the server. It is very easy to break the
  this lock order if we invoke acquire_locks() with list of MDL
  requests which contain both MDL_key::TABLE and
  MDL_key::TABLESPACE. We would end-up in deadlock then.

  @param thd               Thread context.
  @param tables_start      Start of list of tables on which locks
                           should be acquired.
  @param tables_end        End of list of tables.
  @param lock_wait_timeout Seconds to wait before timeout.
  @param flags             Bitmap of flags to modify how the
                           tables will be open, see open_table()
                           description for details.

  @retval true   Failure (e.g. connection was killed)
  @retval false  Success.
*/
bool get_and_lock_tablespace_names_nsec(THD *thd, TABLE_LIST *tables_start,
                                        TABLE_LIST *tables_end,
                                        ulonglong lock_wait_timeout_nsec,
                                        uint flags) {
  // If this is a DISCARD or IMPORT TABLESPACE command (indicated by
  // the THD:: tablespace_op flag), we skip this phase, because these
  // commands are only used for file-per-table tablespaces, which we
  // do not lock.  We also skip this phase if we are within the
  // context of a FLUSH TABLE WITH READ LOCK or FLUSH TABLE FOR EXPORT
  // statement, indicated by the MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK flag.
  if (flags & MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK || thd_tablespace_op(thd))
    return false;

  // Add tablespace names used under partition/subpartition definitions.
  Tablespace_hash_set tablespace_set(PSI_INSTRUMENT_ME);
  if ((thd->lex->sql_command == SQLCOM_CREATE_TABLE ||
       thd->lex->sql_command == SQLCOM_ALTER_TABLE) &&
      fill_partition_tablespace_names(thd->work_part_info, &tablespace_set))
    return true;

  // The first step is to loop over the tables, make sure we have
  // locked the names, and then get hold of the tablespace names from
  // the data dictionary.
  TABLE_LIST *table;
  for (table = tables_start; table && table != tables_end;
       table = table->next_global) {
    // Consider only non-temporary tables. The if clauses below have the
    // following meaning:
    //
    // !MDL_SHARED_READ_ONLY                   Not a LOCK TABLE ... READ.
    //                                         In that case, tables will not
    //                                         be altered, created or dropped,
    //                                         so no need to IX lock the
    //                                         tablespace.
    // is_ddl_or...request() || ...FOR_CREATE  Request for a strong DDL or
    //                                         LOCK TABLES type lock, or a
    //                                         table to be created.
    // !OT_TEMPORARY_ONLY                      Not a user defined tmp table.
    // !(OT_TEMPORARY_OR_BASE && is_temp...()) Not a pre-opened tmp table.
    if (table->mdl_request.type != MDL_SHARED_READ_ONLY &&
        (table->mdl_request.is_ddl_or_lock_tables_lock_request() ||
         table->open_strategy == TABLE_LIST::OPEN_FOR_CREATE) &&
        !is_temporary_table_being_opened(table) && !table->is_system_view) {
      // We have basically three situations here:
      //
      // 1. Lock only the target tablespace name and tablespace
      //    names that are used by partitions (e.g. CREATE TABLE
      //    explicitly specifying the tablespace names).
      // 2. Lock only the existing tablespace name and tablespace
      //    names that are used by partitions (e.g. ALTER TABLE t
      //    ADD COLUMN ... where t is defined in some tablespace s.
      // 3. Lock both the target and the existing tablespace names
      //    along with tablespace names used by partitions. (e.g.
      //    ALTER TABLE t TABLESPACE s2, where t is defined in
      //    some tablespace s)
      if (table->target_tablespace_name.length > 0) {
        tablespace_set.insert(table->target_tablespace_name.str);
      }

      // No need to try this for tables to be created since they are not
      // yet present in the dictionary.
      if (table->open_strategy != TABLE_LIST::OPEN_FOR_CREATE) {
        // Assert that we have an MDL lock on the table name. Needed to read
        // the dictionary safely.
        DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
            MDL_key::TABLE, table->db, table->table_name, MDL_SHARED));

        /*
          Add names of tablespaces used by table or by its
          partitions/subpartitions. Lookup data dictionary to get
          the information.
        */
        if (dd::fill_table_and_parts_tablespace_names(
                thd, table->db, table->table_name, &tablespace_set))
          return true;
      }
    }
  }  // End of for(;;)

  /*
    After we have identified the tablespace names, we iterate
    over the names and acquire IX locks on each of them.
  */
  if (lock_tablespace_names_nsec(thd, &tablespace_set, lock_wait_timeout_nsec))
    return true;

  return false;
}

/**
  Acquire "strong" (SRO, SNW, SNRW) metadata locks on tables used by
  LOCK TABLES or by a DDL statement.

  Acquire lock "S" on table being created in CREATE TABLE statement.

  @note  Under LOCK TABLES, we can't take new locks, so use
         open_tables_check_upgradable_mdl() instead.

  @param thd               Thread context.
  @param tables_start      Start of list of tables on which locks
                           should be acquired.
  @param tables_end        End of list of tables.
  @param lock_wait_timeout Seconds to wait before timeout.
  @param flags             Bitmap of flags to modify how the tables will be
                           open, see open_table() description for details.
  @param schema_reqs       When non-nullptr, pointer to array in which
                           pointers to MDL requests for acquired schema
                           locks to be stored. It is guaranteed that
                           each schema will be present in this array
                           only once.

  @retval false  Success.
  @retval true   Failure (e.g. connection was killed)
*/

bool lock_table_names_nsec(THD *thd, TABLE_LIST *tables_start,
                           TABLE_LIST *tables_end,
                           ulonglong lock_wait_timeout_nsec, uint flags,
                           Prealloced_array<MDL_request *, 1> *schema_reqs) {
  MDL_request_list mdl_requests;
  TABLE_LIST *table;
  MDL_request global_request;
  MDL_request backup_lock_request;
  malloc_unordered_set<TABLE_LIST *, schema_hash, schema_key_equal> schema_set(
      PSI_INSTRUMENT_ME);
  bool need_global_read_lock_protection = false;
  bool acquire_backup_lock = false;

  /*
    This function is not supposed to be used under LOCK TABLES normally.
    Instead open_tables_check_upgradable_mdl() or some other function
    checking if we have tables locked in proper mode should be used.

    The exception to this rule is RENAME TABLES code which uses this call
    to "upgrade" metadata lock on tables renamed along with acquiring
    exclusive locks on target table names, after checking that tables
    renamed are properly locked.
  */
  DBUG_ASSERT(!thd->locked_tables_mode ||
              thd->lex->sql_command == SQLCOM_RENAME_TABLE);

  // Phase 1: Iterate over tables, collect set of unique schema names, and
  //          construct a list of requests for table MDL locks.
  for (table = tables_start; table && table != tables_end;
       table = table->next_global) {
    if (is_temporary_table_being_opened(table)) {
      continue;
    }

    if (!table->mdl_request.is_ddl_or_lock_tables_lock_request() &&
        table->open_strategy != TABLE_LIST::OPEN_FOR_CREATE) {
      continue;
    } else {
      /*
        MDL_request::is_ddl_or_lock_tables_lock_request() returns true for
        DDL and LOCK TABLES statements. Since there isn't a way on MDL API level
        to determine whether a lock being acquired is requested as part of
        handling the statement LOCK TABLES, such check will be done by comparing
        a value of lex->sql_command against the constant SQLCOM_LOCK_TABLES.
        Also we shouldn't acquire IX backup lock in case a table being opened
        with requested MDL_SHARED_READ_ONLY lock. For example, such use case
        takes place when FLUSH PRIVILEGES executed.
      */
      if (thd->lex->sql_command != SQLCOM_LOCK_TABLES &&
          table->mdl_request.type != MDL_SHARED_READ_ONLY)
        acquire_backup_lock = true;
    }

    if (table->mdl_request.type != MDL_SHARED_READ_ONLY) {
      /* Write lock on normal tables is not allowed in a read only transaction.
       */
      if (thd->tx_read_only) {
        my_error(ER_CANT_EXECUTE_IN_READ_ONLY_TRANSACTION, MYF(0));
        return true;
      }

      if (!(flags & MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK)) {
        schema_set.insert(table);
      }
      need_global_read_lock_protection = true;
    }

    mdl_requests.push_front(&table->mdl_request);
  }

  // Phase 2: Iterate over the schema set, add an IX lock for each
  //          schema name.
  if (!(flags & MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK) && !mdl_requests.is_empty()) {
    /*
      Scoped locks: Take intention exclusive locks on all involved
      schemas.
    */
    for (const TABLE_LIST *table_l : schema_set) {
      MDL_request *schema_request = new (thd->mem_root) MDL_request;
      if (schema_request == nullptr) return true;
      MDL_REQUEST_INIT(schema_request, MDL_key::SCHEMA, table_l->db, "",
                       MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
      mdl_requests.push_front(schema_request);
      if (schema_reqs) schema_reqs->push_back(schema_request);
    }

    if (need_global_read_lock_protection) {
      /*
        Protect this statement against concurrent global read lock
        by acquiring global intention exclusive lock with statement
        duration.
      */
      if (thd->global_read_lock.can_acquire_protection()) return true;
      MDL_REQUEST_INIT(&global_request, MDL_key::GLOBAL, "", "",
                       MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
      mdl_requests.push_front(&global_request);
    }
  }

  if (acquire_backup_lock) {
    MDL_REQUEST_INIT(&backup_lock_request, MDL_key::BACKUP_LOCK, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
    mdl_requests.push_front(&backup_lock_request);
  }

  // Phase 3: Acquire the locks which have been requested so far.
  if (thd->mdl_context.acquire_locks_nsec(&mdl_requests,
                                          lock_wait_timeout_nsec))
    return true;

  /*
   Now when we have protection against concurrent change of read_only
   option we can safely re-check its value. Skip the check for
   FLUSH TABLES ... WITH READ LOCK and FLUSH TABLES ... FOR EXPORT
   as they are not supposed to be affected by read_only modes.
   */
  if (need_global_read_lock_protection &&
      !(flags & MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK) &&
      !(flags & MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY) &&
      check_readonly(thd, true))
    return true;

  /*
    Phase 4: Lock tablespace names. This cannot be done as part
    of the previous phases, because we need to read the
    dictionary to get hold of the tablespace name, and in order
    to do this, we must have acquired a lock on the table.
  */
  return get_and_lock_tablespace_names_nsec(thd, tables_start, tables_end,
                                            lock_wait_timeout_nsec, flags);
}

/**
  Check for upgradable (SNW, SNRW) metadata locks on tables to be opened
  for a DDL statement. Under LOCK TABLES, we can't take new locks, so we
  must check if appropriate locks were pre-acquired.

  @param thd           Thread context.
  @param tables_start  Start of list of tables on which upgradable locks
                       should be searched for.
  @param tables_end    End of list of tables.

  @retval false  Success.
  @retval true   Failure (e.g. connection was killed)
*/

static bool open_tables_check_upgradable_mdl(THD *thd, TABLE_LIST *tables_start,
                                             TABLE_LIST *tables_end) {
  TABLE_LIST *table;

  DBUG_ASSERT(thd->locked_tables_mode);

  for (table = tables_start; table && table != tables_end;
       table = table->next_global) {
    if (!table->mdl_request.is_ddl_or_lock_tables_lock_request() ||
        is_temporary_table_being_opened(table)) {
      continue;
    }

    if (table->mdl_request.type == MDL_SHARED_READ_ONLY) {
      if (!thd->mdl_context.owns_equal_or_stronger_lock(
              MDL_key::TABLE, table->db, table->table_name,
              MDL_SHARED_READ_ONLY)) {
        my_error(ER_TABLE_NOT_LOCKED, MYF(0), table->table_name);
        return true;
      }
    } else {
      /*
        We don't need to do anything about the found TABLE instance as it
        will be handled later in open_tables(), we only need to check that
        an upgradable lock is already acquired. When we enter LOCK TABLES
        mode, SNRW locks are acquired before all other locks. So if under
        LOCK TABLES we find that there is TABLE instance with upgradeable
        lock, all other instances of TABLE for the same table will have the
        same ticket.

        Note that this works OK even for CREATE TABLE statements which
        request X type of metadata lock. This is because under LOCK TABLES
        such statements don't create the table but only check if it exists
        or, in most complex case, only insert into it.
        Thus SNRW lock should be enough.

        Note that find_table_for_mdl_upgrade() will report an error if
        no suitable ticket is found.
      */
      if (!find_table_for_mdl_upgrade(thd, table->db, table->table_name, false))
        return true;
    }
  }

  return false;
}

/**
  Iterate along a list of tables and acquire BACKUP LOCK in shared mode
  in case a strong MDL request (DDL/LOCK TABLES-type) was specified
  for a table.

  @param[in]  thd      Thread context.
  @param[in]  tables_start  Pointer to a start of a list of tables to iterate
  @param[in]  tables_end    Pointer to a end of a list of tables where to stop

  @return  false on success, true on error.
*/

static bool acquire_backup_lock_in_lock_tables_mode(THD *thd,
                                                    TABLE_LIST *tables_start,
                                                    TABLE_LIST *tables_end) {
  TABLE_LIST *table;
  DBUG_ASSERT(thd->locked_tables_mode);

  for (table = tables_start; table && table != tables_end;
       table = table->next_global) {
    if (is_temporary_table_being_opened(table)) continue;

    if (table->mdl_request.is_ddl_or_lock_tables_lock_request() &&
        table->mdl_request.type != MDL_SHARED_READ_ONLY)
      return acquire_shared_backup_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec);
  }

  return false;
}

/**
  Open all tables in list

  @param[in]     thd      Thread context.
  @param[in,out] start    List of tables to be open (it can be adjusted for
                          statement that uses tables only implicitly, e.g.
                          for "SELECT f1()").
  @param[out]    counter  Number of tables which were open.
  @param[in]     flags    Bitmap of flags to modify how the tables will be
                          open, see open_table() description for details.
  @param[in]     prelocking_strategy  Strategy which specifies how prelocking
                                      algorithm should work for this statement.

  @note
    Unless we are already in prelocked mode and prelocking strategy prescribes
    so this function will also precache all SP/SFs explicitly or implicitly
    (via views and triggers) used by the query and add tables needed for their
    execution to table list. Statement that uses SFs, invokes triggers or
    requires foreign key checks will be marked as requiring prelocking.
    Prelocked mode will be enabled for such query during lock_tables() call.

    If query for which we are opening tables is already marked as requiring
    prelocking it won't do such precaching and will simply reuse table list
    which is already built.

  @retval  false  Success.
  @retval  true   Error, reported.
*/

bool open_tables(THD *thd, TABLE_LIST **start, uint *counter, uint flags,
                 Prelocking_strategy *prelocking_strategy) {
  /*
    We use pointers to "next_global" member in the last processed TABLE_LIST
    element and to the "next" member in the last processed Sroutine_hash_entry
    element as iterators over, correspondingly, the table list and stored
    routines list which stay valid and allow to continue iteration when new
    elements are added to the tail of the lists.
  */
  TABLE_LIST **table_to_open;
  Sroutine_hash_entry **sroutine_to_open;
  TABLE_LIST *tables;
  Open_table_context ot_ctx(thd, flags);
  bool error = false;
  bool some_routine_modifies_data = false;
  bool has_prelocking_list;
  DBUG_TRACE;
  bool audit_notified = false;

restart:
  /*
    Close HANDLER tables which are marked for flush or against which there
    are pending exclusive metadata locks. This is needed both in order to
    avoid deadlocks and to have a point during statement execution at
    which such HANDLERs are closed even if they don't create problems for
    the current session (i.e. to avoid having a DDL blocked by HANDLERs
    opened for a long time).
  */
  if (!thd->handler_tables_hash.empty()) mysql_ha_flush(thd);

  has_prelocking_list = thd->lex->requires_prelocking();
  table_to_open = start;
  sroutine_to_open = &thd->lex->sroutines_list.first;
  *counter = 0;

  if (!(thd->state_flags & Open_tables_state::SYSTEM_TABLES))
    THD_STAGE_INFO(thd, stage_opening_tables);

  /*
    If we are executing LOCK TABLES statement or a DDL statement
    (in non-LOCK TABLES mode) we might have to acquire upgradable
    semi-exclusive metadata locks (SNW or SNRW) on some of the
    tables to be opened.
    When executing CREATE TABLE .. If NOT EXISTS .. SELECT, the
    table may not yet exist, in which case we acquire an exclusive
    lock.
    We acquire all such locks at once here as doing this in one
    by one fashion may lead to deadlocks or starvation. Later when
    we will be opening corresponding table pre-acquired metadata
    lock will be reused (thanks to the fact that in recursive case
    metadata locks are acquired without waiting).
  */
  if (!(flags & (MYSQL_OPEN_HAS_MDL_LOCK | MYSQL_OPEN_FORCE_SHARED_MDL |
                 MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL))) {
    if (thd->locked_tables_mode) {
      /*
        Under LOCK TABLES, we can't acquire new locks, so we instead
        need to check if appropriate locks were pre-acquired.
      */
      TABLE_LIST *end_table = thd->lex->first_not_own_table();
      if (open_tables_check_upgradable_mdl(thd, *start, end_table) ||
          acquire_backup_lock_in_lock_tables_mode(thd, *start, end_table)) {
        error = true;
        goto err;
      }
    } else {
      TABLE_LIST *table;
      if (lock_table_names_nsec(thd, *start, thd->lex->first_not_own_table(),
                                ot_ctx.get_timeout_nsec(), flags)) {
        error = true;
        goto err;
      }
      for (table = *start; table && table != thd->lex->first_not_own_table();
           table = table->next_global) {
        if (table->mdl_request.is_ddl_or_lock_tables_lock_request() ||
            table->open_strategy == TABLE_LIST::OPEN_FOR_CREATE)
          table->mdl_request.ticket = nullptr;
      }
    }
  }

  /*
    Perform steps of prelocking algorithm until there are unprocessed
    elements in prelocking list/set.
  */
  while (*table_to_open ||
         (thd->locked_tables_mode <= LTM_LOCK_TABLES && *sroutine_to_open)) {
    /*
      For every table in the list of tables to open, try to find or open
      a table.
    */
    for (tables = *table_to_open; tables;
         table_to_open = &tables->next_global, tables = tables->next_global) {
      error = open_and_process_table(thd, thd->lex, tables, counter,
                                     prelocking_strategy, has_prelocking_list,
                                     &ot_ctx);

      if (error) {
        if (ot_ctx.can_recover_from_failed_open()) {
          /*
            We have met exclusive metadata lock or old version of table.
            Now we have to close all tables and release metadata locks.
            We also have to throw away set of prelocked tables (and thus
            close tables from this set that were open by now) since it
            is possible that one of tables which determined its content
            was changed.

            Instead of implementing complex/non-robust logic mentioned
            above we simply close and then reopen all tables.

            We have to save pointer to table list element for table which we
            have failed to open since closing tables can trigger removal of
            elements from the table list (if MERGE tables are involved),
          */
          close_tables_for_reopen(thd, start, ot_ctx.start_of_statement_svp());

          /*
            Here we rely on the fact that 'tables' still points to the valid
            TABLE_LIST element. Altough currently this assumption is valid
            it may change in future.
          */
          if (ot_ctx.recover_from_failed_open()) goto err;

          /* Re-open temporary tables after close_tables_for_reopen(). */
          if (open_temporary_tables(thd, *start)) goto err;

          error = false;
          goto restart;
        }
        goto err;
      }

      DEBUG_SYNC(thd, "open_tables_after_open_and_process_table");
    }

    /*
      Iterate through set of tables and generate table access audit events.
    */
    if (!audit_notified && mysql_audit_table_access_notify(thd, *start)) {
      error = true;
      goto err;
    }

    /*
      Event is not generated in the next loop. It may contain duplicated
      table entries as well as new tables discovered for stored procedures.
      Events for these tables will be generated during the queries of these
      stored procedures.
    */
    audit_notified = true;

    /*
      If we are not already in prelocked mode and extended table list is
      not yet built for our statement we need to cache routines it uses
      and build the prelocking list for it.
      If we are not in prelocked mode but have built the extended table
      list, we still need to call open_and_process_routine() to take
      MDL locks on the routines.
    */
    if (thd->locked_tables_mode <= LTM_LOCK_TABLES) {
      bool routine_modifies_data;
      /*
        Process elements of the prelocking set which are present there
        since parsing stage or were added to it by invocations of
        Prelocking_strategy methods in the above loop over tables.

        For example, if element is a routine, cache it and then,
        if prelocking strategy prescribes so, add tables it uses to the
        table list and routines it might invoke to the prelocking set.
      */
      for (Sroutine_hash_entry *rt = *sroutine_to_open; rt;
           sroutine_to_open = &rt->next, rt = rt->next) {
        bool need_prelocking = false;
        TABLE_LIST **save_query_tables_last = thd->lex->query_tables_last;

        error = open_and_process_routine(
            thd, thd->lex, rt, prelocking_strategy, has_prelocking_list,
            &ot_ctx, &need_prelocking, &routine_modifies_data);

        if (need_prelocking && !thd->lex->requires_prelocking())
          thd->lex->mark_as_requiring_prelocking(save_query_tables_last);

        if (need_prelocking && !*start) *start = thd->lex->query_tables;

        if (error) {
          if (ot_ctx.can_recover_from_failed_open()) {
            close_tables_for_reopen(thd, start,
                                    ot_ctx.start_of_statement_svp());
            if (ot_ctx.recover_from_failed_open()) goto err;

            /* Re-open temporary tables after close_tables_for_reopen(). */
            if (open_temporary_tables(thd, *start)) goto err;

            error = false;
            goto restart;
          }
          /*
            Serious error during reading stored routines from mysql.proc table.
            Something is wrong with the table or its contents, and an error has
            been emitted; we must abort.
          */
          goto err;
        }

        // Remember if any of SF modifies data.
        some_routine_modifies_data |= routine_modifies_data;
      }
    }
  }

  /* Accessing data in XA_IDLE or XA_PREPARED is not allowed. */
  if (*start &&
      thd->get_transaction()->xid_state()->check_xa_idle_or_prepared(true))
    return true;

  /*
   If some routine is modifying the table then the statement is not read only.
   If timer is enabled then resetting the timer in this case.
  */
  if (thd->timer && some_routine_modifies_data) {
    reset_statement_timer(thd);
    push_warning(thd, Sql_condition::SL_NOTE, ER_NON_RO_SELECT_DISABLE_TIMER,
                 ER_THD(thd, ER_NON_RO_SELECT_DISABLE_TIMER));
  }

  /*
    After successful open of all tables, including MERGE parents and
    children, attach the children to their parents. At end of statement,
    the children are detached. Attaching and detaching are always done,
    even under LOCK TABLES.

    We also convert all TL_WRITE_DEFAULT and TL_READ_DEFAULT locks to
    appropriate "real" lock types to be used for locking and to be passed
    to storage engine.
  */
  for (tables = *start; tables; tables = tables->next_global) {
    TABLE *tbl = tables->table;

    /*
      NOTE: temporary merge tables should be processed here too, because
      a temporary merge table can be based on non-temporary tables.
    */

    /* Schema tables may not have a TABLE object here. */
    if (tbl && tbl->file->ht->db_type == DB_TYPE_MRG_MYISAM) {
      /* MERGE tables need to access parent and child TABLE_LISTs. */
      DBUG_ASSERT(tbl->pos_in_table_list == tables);
      if (tbl->db_stat && tbl->file->ha_extra(HA_EXTRA_ATTACH_CHILDREN)) {
        error = true;
        goto err;
      }
    }

    /* Set appropriate TABLE::lock_type. */
    if (tbl && tables->lock_descriptor().type != TL_UNLOCK &&
        !thd->locked_tables_mode) {
      if (tables->lock_descriptor().type == TL_WRITE_DEFAULT)
        tbl->reginfo.lock_type = thd->update_lock_default;
      else if (tables->lock_descriptor().type == TL_WRITE_CONCURRENT_DEFAULT)
        tables->table->reginfo.lock_type = thd->insert_lock_default;
      else if (tables->lock_descriptor().type == TL_READ_DEFAULT)
        tbl->reginfo.lock_type = read_lock_type_for_table(
            thd, thd->lex, tables, some_routine_modifies_data);
      else
        tbl->reginfo.lock_type = tables->lock_descriptor().type;
    }

    /*
      Check if this is a DD table used under a I_S view
      then tell innodb to do non-locking reads on the table.
    */
    if (tbl && tables->referencing_view &&
        tables->referencing_view->is_system_view) {
      /*
        SELECT using a I_S system view with 'FOR UPDATE' and
        'LOCK IN SHARED MODE' clause is not allowed.
      */
      if (tables->lock_descriptor().type == TL_READ_WITH_SHARED_LOCKS) {
        my_error(ER_IS_QUERY_INVALID_CLAUSE, MYF(0), "LOCK IN SHARE MODE");
        error = true;
        goto err;
      }
      // Allow I_S system views to be locked by LOCK TABLE command.
      if (thd->lex->sql_command != SQLCOM_LOCK_TABLES &&
          tables->lock_descriptor().type >= TL_READ_NO_INSERT) {
        my_error(ER_IS_QUERY_INVALID_CLAUSE, MYF(0), "FOR UPDATE");
        error = true;
        goto err;
      }

      /* Convey to InnoDB (the DD table's engine) to do non-locking reads.

         It is assumed that all the tables used by I_S views are
         always a DD table. If this is not true, then we might
         need to invoke dd::Dictionary::is_dd_tablename() to make sure.
       */
      if (tbl->db_stat &&
          tbl->file->ha_extra(HA_EXTRA_SKIP_SERIALIZABLE_DD_VIEW)) {
        // Handler->ha_extra() for innodb does not fail ever as of now.
        // In case it is made to fail sometime later, we need to think
        // about the kind of error to be report to user.
        DBUG_ASSERT(0);

        error = true;
        goto err;
      }
    }
  }  // End of for(;;)

err:
  if (error && *table_to_open) {
    (*table_to_open)->table = nullptr;
  }
  DBUG_PRINT("open_tables", ("returning: %d", (int)error));
  return error;
}

/**
  Defines how prelocking algorithm for DML statements should handle routines:
  - For CALL statements we do unrolling (i.e. open and lock tables for each
    sub-statement individually). So for such statements prelocking is enabled
    only if stored functions are used in parameter list and only for period
    during which we calculate values of parameters. Thus in this strategy we
    ignore procedure which is directly called by such statement and extend
    the prelocking set only with tables/functions used by SF called from the
    parameter list.
  - For any other statement any routine which is directly or indirectly called
    by statement is going to be executed in prelocked mode. So in this case we
    simply add all tables and routines used by it to the prelocking set.

  @param[in]  thd              Thread context.
  @param[in]  prelocking_ctx   Prelocking context of the statement.
  @param[in]  rt               Prelocking set element describing routine.
  @param[in]  sp               Routine body.
  @param[out] need_prelocking  Set to true if method detects that prelocking
                               required, not changed otherwise.

  @retval false  Success.
  @retval true   Failure (OOM).
*/

bool DML_prelocking_strategy::handle_routine(THD *thd,
                                             Query_tables_list *prelocking_ctx,
                                             Sroutine_hash_entry *rt,
                                             sp_head *sp,
                                             bool *need_prelocking) {
  /*
    We assume that for any "CALL proc(...)" statement sroutines_list will
    have 'proc' as first element (it may have several, consider e.g.
    "proc(sp_func(...)))". This property is currently guaranted by the
    parser.
  */

  if (rt != prelocking_ctx->sroutines_list.first ||
      rt->type() != Sroutine_hash_entry::PROCEDURE) {
    *need_prelocking = true;
    sp_update_stmt_used_routines(thd, prelocking_ctx, &sp->m_sroutines,
                                 rt->belong_to_view);
    sp->add_used_tables_to_table_list(thd, &prelocking_ctx->query_tables_last,
                                      prelocking_ctx->sql_command,
                                      rt->belong_to_view);
  }
  sp->propagate_attributes(prelocking_ctx);
  return false;
}

/**
  Defines how prelocking algorithm for DML statements should handle table list
  elements:
  - If table has triggers we should add all tables and routines
    used by them to the prelocking set.
  - If table participates in a foreign key we should add another
    table from it to the prelocking set with an appropriate metadata
    lock.

  We do not need to acquire metadata locks on trigger names
  in DML statements, since all DDL statements
  that change trigger metadata always lock their
  subject tables.

  @param[in]  thd              Thread context.
  @param[in]  prelocking_ctx   Prelocking context of the statement.
  @param[in]  table_list       Table list element for table.
  @param[out] need_prelocking  Set to true if method detects that prelocking
                               required, not changed otherwise.

  @retval false  Success.
  @retval true   Failure (OOM).
*/

bool DML_prelocking_strategy::handle_table(THD *thd,
                                           Query_tables_list *prelocking_ctx,
                                           TABLE_LIST *table_list,
                                           bool *need_prelocking) {
  /* We rely on a caller to check that table is going to be changed. */
  DBUG_ASSERT(table_list->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE);

  if (table_list->trg_event_map) {
    if (table_list->table->triggers) {
      *need_prelocking = true;

      if (table_list->table->triggers->add_tables_and_routines_for_triggers(
              thd, prelocking_ctx, table_list))
        return true;
    }

    /*
      When FOREIGN_KEY_CHECKS is 0 we are not going to do any foreign key checks
      so we don't need to add child and parent tables to the prelocking list.

      However, since trigger or stored function might change this variable for
      their duration (it is, actually, advisable to do so in some scenarios),
      we can apply this optimization only to tables which are directly used by
      the top-level statement.

      While processing LOCK TABLES, we must disregard F_K_C too, since the
      prelocking set will be used while in LTM mode, and F_K_C may be turned
      on later, after the set has been established.
    */
    if ((!(thd->variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS) ||
         prelocking_ctx->sql_command == SQLCOM_LOCK_TABLES ||
         table_list->prelocking_placeholder) &&
        !(table_list->table->s->tmp_table)) {
      bool is_insert =
          (table_list->trg_event_map &
           static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_INSERT)));
      bool is_update =
          (table_list->trg_event_map &
           static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_UPDATE)));
      bool is_delete =
          (table_list->trg_event_map &
           static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_DELETE)));

      process_table_fks(thd, prelocking_ctx, table_list->table->s, is_insert,
                        is_update, is_delete, table_list->belong_to_view,
                        need_prelocking);
    }
  }
  return false;
}

/**
  Defines how prelocking algorithm for DML statements should handle view -
  all view routines should be added to the prelocking set.

  @param[in]  thd              Thread context.
  @param[in]  prelocking_ctx   Prelocking context of the statement.
  @param[in]  table_list       Table list element for view.
  @param[out] need_prelocking  Set to true if method detects that prelocking
                               required, not changed otherwise.

  @retval false  Success.
  @retval true   Failure (OOM).
*/

bool DML_prelocking_strategy::handle_view(THD *thd,
                                          Query_tables_list *prelocking_ctx,
                                          TABLE_LIST *table_list,
                                          bool *need_prelocking) {
  if (table_list->view_query()->uses_stored_routines()) {
    *need_prelocking = true;

    sp_update_stmt_used_routines(thd, prelocking_ctx,
                                 &table_list->view_query()->sroutines_list,
                                 table_list->top_table());
  }

  /*
    If a trigger was defined on one of the associated tables then assign the
    'trg_event_map' value of the view to the next table in table_list. When a
    Stored function is invoked, all the associated tables including the tables
    associated with the trigger are prelocked.
  */
  if (table_list->trg_event_map && table_list->next_global)
    table_list->next_global->trg_event_map = table_list->trg_event_map;
  return false;
}

/**
  Defines how prelocking algorithm for LOCK TABLES statement should handle
  table list elements.

  @param[in]  thd              Thread context.
  @param[in]  prelocking_ctx   Prelocking context of the statement.
  @param[in]  table_list       Table list element for table.
  @param[out] need_prelocking  Set to true if method detects that prelocking
                               required, not changed otherwise.

  @retval false  Success.
  @retval true   Failure (OOM).
*/

bool Lock_tables_prelocking_strategy::handle_table(
    THD *thd, Query_tables_list *prelocking_ctx, TABLE_LIST *table_list,
    bool *need_prelocking) {
  if (DML_prelocking_strategy::handle_table(thd, prelocking_ctx, table_list,
                                            need_prelocking))
    return true;

  /* We rely on a caller to check that table is going to be changed. */
  DBUG_ASSERT(table_list->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE);

  return false;
}

/**
  Defines how prelocking algorithm for ALTER TABLE statement should handle
  routines - do nothing as this statement is not supposed to call routines.

  We still can end up in this method when someone tries
  to define a foreign key referencing a view, and not just
  a simple view, but one that uses stored routines.
*/

bool Alter_table_prelocking_strategy::handle_routine(THD *, Query_tables_list *,
                                                     Sroutine_hash_entry *,
                                                     sp_head *, bool *) {
  return false;
}

/**
  Defines how prelocking algorithm for ALTER TABLE statement should handle
  table list elements.

  Unlike in DML, we do not process triggers here.
*/

bool Alter_table_prelocking_strategy::handle_table(THD *, Query_tables_list *,
                                                   TABLE_LIST *, bool *) {
  return false;
}

/**
  Defines how prelocking algorithm for ALTER TABLE statement
  should handle view - do nothing. We don't need to add view
  routines to the prelocking set in this case as view is not going
  to be materialized.
*/

bool Alter_table_prelocking_strategy::handle_view(THD *, Query_tables_list *,
                                                  TABLE_LIST *, bool *) {
  return false;
}

/**
  Check that lock is ok for tables; Call start stmt if ok

  @param thd             Thread handle.
  @param prelocking_ctx  Prelocking context.
  @param table_list      Table list element for table to be checked.

  @retval false - Ok.
  @retval true  - Error.
*/

static bool check_lock_and_start_stmt(THD *thd,
                                      Query_tables_list *prelocking_ctx,
                                      TABLE_LIST *table_list) {
  int error;
  thr_lock_type lock_type;
  DBUG_TRACE;

  /*
    Prelocking placeholder is not set for TABLE_LIST that
    are directly used by TOP level statement.
  */
  DBUG_ASSERT(table_list->prelocking_placeholder == false);

  /*
    TL_WRITE_DEFAULT, TL_READ_DEFAULT and TL_WRITE_CONCURRENT_DEFAULT
    are supposed to be parser only types of locks so they should be
    converted to appropriate other types to be passed to storage engine.
    The exact lock type passed to the engine is important as, for example,
    InnoDB uses it to determine what kind of row locks should be acquired
    when executing statement in prelocked mode or under LOCK TABLES with
    @@innodb_table_locks = 0.

    Last argument routine_modifies_data for read_lock_type_for_table()
    is ignored, as prelocking placeholder will never be set here.
  */
  if (table_list->lock_descriptor().type == TL_WRITE_DEFAULT)
    lock_type = thd->update_lock_default;
  else if (table_list->lock_descriptor().type == TL_WRITE_CONCURRENT_DEFAULT)
    lock_type = thd->insert_lock_default;
  else if (table_list->lock_descriptor().type == TL_READ_DEFAULT)
    lock_type = read_lock_type_for_table(thd, prelocking_ctx, table_list, true);
  else
    lock_type = table_list->lock_descriptor().type;

  if ((int)lock_type > (int)TL_WRITE_ALLOW_WRITE &&
      (int)table_list->table->reginfo.lock_type <= (int)TL_WRITE_ALLOW_WRITE) {
    my_error(ER_TABLE_NOT_LOCKED_FOR_WRITE, MYF(0), table_list->alias);
    return true;
  }
  if ((error = table_list->table->file->start_stmt(thd, lock_type))) {
    table_list->table->file->print_error(error, MYF(0));
    return true;
  }

  /*
    Record in transaction state tracking
  */
  if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
    TX_TRACKER_GET(tst);
    enum enum_tx_state s;

    s = tst->calc_trx_state(lock_type,
                            table_list->table->file->has_transactions());
    tst->add_trx_state(thd, s);
  }

  return false;
}

/**
  @brief Open and lock one table

  @param[in]    thd             thread handle
  @param[in]    table_l         table to open is first table in this list
  @param[in]    lock_type       lock to use for table
  @param[in]    flags           options to be used while opening and locking
                                table (see open_table(), mysql_lock_tables())
  @param[in]    prelocking_strategy  Strategy which specifies how prelocking
                                     algorithm should work for this statement.

  @return       table
    @retval     != NULL         OK, opened table returned
    @retval     NULL            Error

  @note
    If ok, the following are also set:
      table_list->lock_type 	lock_type
      table_list->table		table

  @note
    If table_l is a list, not a single table, the list is temporarily
    broken.

  @details
    This function is meant as a replacement for open_ltable() when
    MERGE tables can be opened. open_ltable() cannot open MERGE tables.

    There may be more differences between open_n_lock_single_table() and
    open_ltable(). One known difference is that open_ltable() does
    neither call thd->decide_logging_format() nor handle some other logging
    and locking issues because it does not call lock_tables().
*/

TABLE *open_n_lock_single_table(THD *thd, TABLE_LIST *table_l,
                                thr_lock_type lock_type, uint flags,
                                Prelocking_strategy *prelocking_strategy) {
  TABLE_LIST *save_next_global;
  DBUG_TRACE;

  /* Remember old 'next' pointer. */
  save_next_global = table_l->next_global;
  /* Break list. */
  table_l->next_global = nullptr;

  /* Set requested lock type. */
  table_l->set_lock({lock_type, THR_DEFAULT});
  /* Allow to open real tables only. */
  table_l->required_type = dd::enum_table_type::BASE_TABLE;

  /* Open the table. */
  if (open_and_lock_tables(thd, table_l, flags, prelocking_strategy))
    table_l->table = nullptr; /* Just to be sure. */

  /* Restore list. */
  table_l->next_global = save_next_global;

  return table_l->table;
}

/*
  Open and lock one table

  SYNOPSIS
    open_ltable()
    thd			Thread handler
    table_list		Table to open is first table in this list
    lock_type		Lock to use for open
    lock_flags          Flags passed to mysql_lock_table

  NOTE
    This function doesn't do anything like SP/SF/views/triggers analysis done
    in open_table()/lock_tables(). It is intended for opening of only one
    concrete table. And used only in special contexts.

  RETURN VALUES
    table		Opened table
    0			Error

    If ok, the following are also set:
      table_list->lock_type 	lock_type
      table_list->table		table
*/

TABLE *open_ltable(THD *thd, TABLE_LIST *table_list, thr_lock_type lock_type,
                   uint lock_flags) {
  TABLE *table;
  Open_table_context ot_ctx(thd, lock_flags);
  bool error;
  DBUG_TRACE;

  /* should not be used in a prelocked_mode context, see NOTE above */
  DBUG_ASSERT(thd->locked_tables_mode < LTM_PRELOCKED);

  if (!(thd->state_flags & Open_tables_state::SYSTEM_TABLES))
    THD_STAGE_INFO(thd, stage_opening_tables);

  /* open_ltable can be used only for BASIC TABLEs */
  table_list->required_type = dd::enum_table_type::BASE_TABLE;

  /* This function can't properly handle requests for such metadata locks. */
  DBUG_ASSERT(!table_list->mdl_request.is_ddl_or_lock_tables_lock_request());

  while ((error = open_table(thd, table_list, &ot_ctx)) &&
         ot_ctx.can_recover_from_failed_open()) {
    /*
      Even though we have failed to open table we still need to
      call release_transactional_locks() to release metadata locks which
      might have been acquired successfully.
    */
    thd->mdl_context.rollback_to_savepoint(ot_ctx.start_of_statement_svp());
    table_list->mdl_request.ticket = nullptr;
    if (ot_ctx.recover_from_failed_open()) break;
  }

  if (!error) {
    /*
      We can't have a view or some special "open_strategy" in this function
      so there should be a TABLE instance.
    */
    DBUG_ASSERT(table_list->table);
    table = table_list->table;
    if (table->file->ht->db_type == DB_TYPE_MRG_MYISAM) {
      /* A MERGE table must not come here. */
      /* purecov: begin tested */
      my_error(ER_WRONG_OBJECT, MYF(0), table->s->db.str,
               table->s->table_name.str, "BASE TABLE");
      table = nullptr;
      goto end;
      /* purecov: end */
    }

    table_list->set_lock({lock_type, THR_DEFAULT});
    if (thd->locked_tables_mode) {
      if (check_lock_and_start_stmt(thd, thd->lex, table_list)) table = nullptr;
    } else {
      DBUG_ASSERT(thd->lock == nullptr);  // You must lock everything at once
      if ((table->reginfo.lock_type = lock_type) != TL_UNLOCK)
        if (!(thd->lock =
                  mysql_lock_tables(thd, &table_list->table, 1, lock_flags))) {
          table = nullptr;
        }
    }
  } else
    table = nullptr;

end:
  if (table == nullptr) {
    if (!thd->in_sub_stmt) trans_rollback_stmt(thd);
    close_thread_tables(thd);
  }
  return table;
}

/**
  Open all tables in list, locks them and optionally process derived tables.

  @param thd		      Thread context.
  @param tables	              List of tables for open and locking.
  @param flags                Bitmap of options to be used to open and lock
                              tables (see open_tables() and mysql_lock_tables()
                              for details).
  @param prelocking_strategy  Strategy which specifies how prelocking algorithm
                              should work for this statement.

  @note
    The thr_lock locks will automatically be freed by close_thread_tables().

  @note
    open_and_lock_tables() is not intended for open-and-locking system tables
    in those cases when execution of statement has started already and other
    tables have been opened. Use open_trans_system_tables_for_read() instead.

  @retval false  OK.
  @retval true   Error
*/

bool open_and_lock_tables(THD *thd, TABLE_LIST *tables, uint flags,
                          Prelocking_strategy *prelocking_strategy) {
  uint counter;
  MDL_savepoint mdl_savepoint = thd->mdl_context.mdl_savepoint();
  DBUG_TRACE;

  /*
    open_and_lock_tables() must not be used to open system tables. There must
    be no active attachable transaction when open_and_lock_tables() is called.
    Exception is made to the read-write attachables with explicitly specified
    in the assert table.
    Callers in the read-write case must make sure no side effect to
    the global transaction state is inflicted when the attachable one
    will commmit.
  */
  DBUG_ASSERT(!thd->is_attachable_ro_transaction_active() &&
              (!thd->is_attachable_rw_transaction_active() ||
               !strcmp(tables->table_name, "gtid_executed")));

  if (open_tables(thd, &tables, &counter, flags, prelocking_strategy)) goto err;

  DBUG_EXECUTE_IF("sleep_open_and_lock_after_open", {
    const char *old_proc_info = thd->proc_info;
    thd->proc_info = "DBUG sleep";
    my_sleep(6000000);
    thd->proc_info = old_proc_info;
  });

  if (lock_tables(thd, tables, counter, flags)) goto err;

  return false;
err:
  // Rollback the statement execution done so far
  if (!thd->in_sub_stmt) trans_rollback_stmt(thd);
  close_thread_tables(thd);
  /* Don't keep locks for a failed statement. */
  thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
  return true;
}

/**
  Check if a secondary engine can be used to execute the current
  statement, and if so, replace the opened tables with their secondary
  counterparts.

  @param thd       thread handler
  @param flags     bitmap of flags to pass to open_table
  @return true if an error is raised, false otherwise
*/
static bool open_secondary_engine_tables(THD *thd, uint flags) {
  LEX *const lex = thd->lex;
  Sql_cmd *const sql_cmd = lex->m_sql_cmd;

  // The previous execution context should have been destroyed.
  DBUG_ASSERT(lex->secondary_engine_execution_context() == nullptr);

  // If use of secondary engines has been disabled for the statement,
  // there is nothing to do.
  if (sql_cmd == nullptr || sql_cmd->secondary_storage_engine_disabled())
    return false;

  // If the user has requested the use of a secondary storage engine
  // for this statement, skip past the initial optimization for the
  // primary storage engine and go straight to the secondary engine.
  if (thd->secondary_engine_optimization() ==
          Secondary_engine_optimization::PRIMARY_TENTATIVELY &&
      thd->variables.use_secondary_engine == SECONDARY_ENGINE_FORCED) {
    thd->set_secondary_engine_optimization(
        Secondary_engine_optimization::SECONDARY);
  }

  // Don't open the secondary engine tables for a PREPARE command. Use
  // of secondary engines is not decided until the optimization phase
  // of the execution, so only open them when a statement is executed.
  if (thd->stmt_arena->is_stmt_prepare()) return false;

  // Only open secondary engine tables if use of a secondary engine
  // has been requested.
  if (thd->secondary_engine_optimization() !=
      Secondary_engine_optimization::SECONDARY)
    return false;

  // If the statement cannot be executed in a secondary engine because
  // of a property of the statement, do not attempt to open the
  // secondary tables. Also disable use of secondary engines for
  // future executions of the statement, since these properties will
  // not change between executions.
  const LEX_CSTRING *secondary_engine =
      sql_cmd->eligible_secondary_storage_engine();
  const plugin_ref secondary_engine_plugin =
      secondary_engine == nullptr
          ? nullptr
          : ha_resolve_by_name(thd, secondary_engine, false);

  if ((secondary_engine_plugin == nullptr) ||
      !plugin_is_ready(*secondary_engine, MYSQL_STORAGE_ENGINE_PLUGIN)) {
    // Didn't find a secondary storage engine to use for the query.
    sql_cmd->disable_secondary_storage_engine();
    return false;
  }

  // If the statement cannot be executed in a secondary engine because
  // of a property of the environment, do not attempt to open the
  // secondary tables. However, do not disable use of secondary
  // storage engines for future executions of the statement, since the
  // environment may change before the next execution.
  if (!thd->secondary_storage_engine_eligible()) return false;

  auto hton = plugin_data<const handlerton *>(secondary_engine_plugin);
  sql_cmd->use_secondary_storage_engine(hton);

  // Replace the TABLE objects in the TABLE_LIST with secondary tables.
  Open_table_context ot_ctx(thd, flags | MYSQL_OPEN_SECONDARY_ENGINE);
  for (TABLE_LIST *tl = lex->query_tables; tl != nullptr;
       tl = tl->next_global) {
    if (tl->is_placeholder()) continue;
    TABLE *primary_table = tl->table;
    tl->table = nullptr;
    if (open_table(thd, tl, &ot_ctx)) return true;
    DBUG_ASSERT(tl->table->s->is_secondary_engine());
    tl->table->file->ha_set_primary_handler(primary_table->file);
  }

  // Prepare the secondary engine for executing the statement.
  return hton->prepare_secondary_engine != nullptr &&
         hton->prepare_secondary_engine(thd, lex);
}

/**
  Open all tables for a query or statement, in list started by "tables"

  @param       thd      thread handler
  @param       tables   list of tables for open
  @param       flags    bitmap of flags to modify how the tables will be open:
                        MYSQL_LOCK_IGNORE_FLUSH - open table even if someone has
                        done a flush on it.

  @retval false - ok
  @retval true  - error

  @note
    This is to be used on prepare stage when you don't read any
    data from the tables.

  @note
    Updates Query_tables_list::table_count as side-effect.
*/

bool open_tables_for_query(THD *thd, TABLE_LIST *tables, uint flags) {
  DML_prelocking_strategy prelocking_strategy;
  MDL_savepoint mdl_savepoint = thd->mdl_context.mdl_savepoint();
  DBUG_TRACE;

  DBUG_ASSERT(tables == thd->lex->query_tables);

  if (open_tables(thd, &tables, &thd->lex->table_count, flags,
                  &prelocking_strategy))
    goto end;

  if (open_secondary_engine_tables(thd, flags)) goto end;

  return false;
end:
  /*
    No need to commit/rollback the statement transaction: it's
    either not started or we're filling in an INFORMATION_SCHEMA
    table on the fly, and thus mustn't manipulate with the
    transaction of the enclosing statement.
  */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT) ||
              (thd->state_flags & Open_tables_state::BACKUPS_AVAIL) ||
              thd->in_sub_stmt);
  close_thread_tables(thd);
  /* Don't keep locks for a failed statement. */
  thd->mdl_context.rollback_to_savepoint(mdl_savepoint);

  return true; /* purecov: inspected */
}

/*
  Mark all real tables in the list as free for reuse.

  SYNOPSIS
    mark_real_tables_as_free_for_reuse()
      thd   - thread context
      table - head of the list of tables

  DESCRIPTION
    Marks all real tables in the list (i.e. not views, derived
    or schema tables) as free for reuse.
*/

static void mark_real_tables_as_free_for_reuse(TABLE_LIST *table_list) {
  TABLE_LIST *table;
  for (table = table_list; table; table = table->next_global)
    if (!table->is_placeholder()) {
      table->table->query_id = 0;
    }
  for (table = table_list; table; table = table->next_global)
    if (!table->is_placeholder() && table->table->db_stat) {
      /*
        Detach children of MyISAMMRG tables used in
        sub-statements, they will be reattached at open.
        This has to be done in a separate loop to make sure
        that children have had their query_id cleared.
      */
      table->table->file->ha_extra(HA_EXTRA_DETACH_CHILDREN);
    }
}

/**
  Lock all tables in a list.

  @param  thd           Thread handler
  @param  tables        Tables to lock
  @param  count         Number of opened tables
  @param  flags         Options (see mysql_lock_tables() for details)

  You can't call lock_tables() while holding thr_lock locks, as
  this would break the dead-lock-free handling thr_lock gives us.
  You must always get all needed locks at once.

  If the query for which we are calling this function is marked as
  requiring prelocking, this function will change
  locked_tables_mode to LTM_PRELOCKED.

  @retval false         Success.
  @retval true          A lock wait timeout, deadlock or out of memory.
*/

bool lock_tables(THD *thd, TABLE_LIST *tables, uint count, uint flags) {
  TABLE_LIST *table;

  DBUG_TRACE;
  /*
    We can't meet statement requiring prelocking if we already
    in prelocked mode.
  */
  DBUG_ASSERT(thd->locked_tables_mode <= LTM_LOCK_TABLES ||
              !thd->lex->requires_prelocking());

  /*
    lock_tables() should not be called if this statement has
    already locked its tables.
  */
  DBUG_ASSERT(thd->lex->lock_tables_state == Query_tables_list::LTS_NOT_LOCKED);

  if (!tables && !thd->lex->requires_prelocking()) {
    /*
      Even though we are not really locking any tables mark this
      statement as one that has locked its tables, so we won't
      call this function second time for the same execution of
      the same statement.
    */
    thd->lex->lock_tables_state = Query_tables_list::LTS_LOCKED;
    int ret = thd->decide_logging_format(tables);
    return ret;
  }

  /*
    Check for thd->locked_tables_mode to avoid a redundant
    and harmful attempt to lock the already locked tables again.
    Checking for thd->lock is not enough in some situations. For example,
    if a stored function contains
    "drop table t3; create temporary t3 ..; insert into t3 ...;"
    thd->lock may be 0 after drop tables, whereas locked_tables_mode
    is still on. In this situation an attempt to lock temporary
    table t3 will lead to a memory leak.
  */
  if (!thd->locked_tables_mode) {
    DBUG_ASSERT(thd->lock == nullptr);  // You must lock everything at once
    TABLE **start, **ptr;

    if (!(ptr = start = (TABLE **)thd->alloc(sizeof(TABLE *) * count)))
      return true;
    for (table = tables; table; table = table->next_global) {
      if (!table->is_placeholder()) *(ptr++) = table->table;
    }

    DEBUG_SYNC(thd, "before_lock_tables_takes_lock");

    if (!(thd->lock =
              mysql_lock_tables(thd, start, (uint)(ptr - start), flags)))
      return true;

    DEBUG_SYNC(thd, "after_lock_tables_takes_lock");

    if (thd->lex->requires_prelocking() &&
        thd->lex->sql_command != SQLCOM_LOCK_TABLES) {
      TABLE_LIST *first_not_own = thd->lex->first_not_own_table();
      /*
        We just have done implicit LOCK TABLES, and now we have
        to emulate first open_and_lock_tables() after it.

        When open_and_lock_tables() is called for a single table out of
        a table list, the 'next_global' chain is temporarily broken. We
        may not find 'first_not_own' before the end of the "list".
        Look for example at those places where open_n_lock_single_table()
        is called. That function implements the temporary breaking of
        a table list for opening a single table.
      */
      for (table = tables; table && table != first_not_own;
           table = table->next_global) {
        if (!table->is_placeholder()) {
          table->table->query_id = thd->query_id;
          if (check_lock_and_start_stmt(thd, thd->lex, table)) {
            mysql_unlock_tables(thd, thd->lock);
            thd->lock = nullptr;
            return true;
          }
        }
      }
      /*
        Let us mark all tables which don't belong to the statement itself,
        and was marked as occupied during open_tables() as free for reuse.
      */
      mark_real_tables_as_free_for_reuse(first_not_own);
      DBUG_PRINT("info", ("locked_tables_mode= LTM_PRELOCKED"));
      thd->enter_locked_tables_mode(LTM_PRELOCKED);
    }
  } else {
    /*
      When we implicitly open DD tables used by a IS query in LOCK TABLE mode,
      we do not go through mysql_lock_tables(), which sets lock type to use
      by SE. Here, we request SE to use read lock for these implicitly opened
      DD tables using ha_external_lock().

      TODO: In PRELOCKED under LOCKED TABLE mode, if sub-statement is a IS
            query then for DD table ha_external_lock is called more than once.
            This works for now as in this mode each sub-statement gets its own
            brand new TABLE instances for each table.
            Allocating a brand new TABLE instances for each sub-statement is
            a resources wastage. Once this issue is fixed, following code
            should be adjusted to not to call ha_external_lock in sub-statement
            mode (similar to how code in close_thread_table() behaves).
    */
    if (in_LTM(thd)) {
      for (table = tables; table; table = table->next_global) {
        TABLE *tbl = table->table;
        if (tbl && belongs_to_dd_table(table)) {
          DBUG_ASSERT(tbl->file->get_lock_type() == F_UNLCK);
          tbl->file->init_table_handle_for_HANDLER();
          tbl->file->ha_external_lock(thd, F_RDLCK);
        }
      }
    }

    TABLE_LIST *first_not_own = thd->lex->first_not_own_table();
    /*
      When open_and_lock_tables() is called for a single table out of
      a table list, the 'next_global' chain is temporarily broken. We
      may not find 'first_not_own' before the end of the "list".
      Look for example at those places where open_n_lock_single_table()
      is called. That function implements the temporary breaking of
      a table list for opening a single table.
    */
    for (table = tables; table && table != first_not_own;
         table = table->next_global) {
      if (table->is_placeholder()) continue;

      /*
        In a stored function or trigger we should ensure that we won't change
        a table that is already used by the calling statement.
      */
      if (thd->locked_tables_mode >= LTM_PRELOCKED &&
          table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) {
        for (TABLE *opentab = thd->open_tables; opentab;
             opentab = opentab->next) {
          if (table->table->s == opentab->s && opentab->query_id &&
              table->table->query_id != opentab->query_id) {
            my_error(ER_CANT_UPDATE_USED_TABLE_IN_SF_OR_TRG, MYF(0),
                     table->table->s->table_name.str);
            return true;
          }
        }
      }

      if (check_lock_and_start_stmt(thd, thd->lex, table)) {
        return true;
      }
    }
    /*
      If we are under explicit LOCK TABLES and our statement requires
      prelocking, we should mark all "additional" tables as free for use
      and enter prelocked mode.
    */
    if (thd->lex->requires_prelocking()) {
      mark_real_tables_as_free_for_reuse(first_not_own);
      DBUG_PRINT("info",
                 ("thd->locked_tables_mode= LTM_PRELOCKED_UNDER_LOCK_TABLES"));
      thd->locked_tables_mode = LTM_PRELOCKED_UNDER_LOCK_TABLES;
    }
  }

  /*
    Mark the statement as having tables locked. For purposes
    of Query_tables_list::lock_tables_state we treat any
    statement which passes through lock_tables() as such.
  */
  thd->lex->lock_tables_state = Query_tables_list::LTS_LOCKED;

  int ret = thd->decide_logging_format(tables);
  return ret;
}

/**
  Prepare statement for reopening of tables and recalculation of set of
  prelocked tables.

  @param[in] thd         Thread context.
  @param[in,out] tables  List of tables which we were trying to open
                         and lock.
  @param[in] start_of_statement_svp MDL savepoint which represents the set
                         of metadata locks which the current transaction
                         managed to acquire before execution of the current
                         statement and to which we should revert before
                         trying to reopen tables. NULL if no metadata locks
                         were held and thus all metadata locks should be
                         released.
*/

void close_tables_for_reopen(THD *thd, TABLE_LIST **tables,
                             const MDL_savepoint &start_of_statement_svp) {
  TABLE_LIST *first_not_own_table = thd->lex->first_not_own_table();
  TABLE_LIST *tmp;

  /*
    If table list consists only from tables from prelocking set, table list
    for new attempt should be empty, so we have to update list's root pointer.
  */
  if (first_not_own_table == *tables) *tables = nullptr;
  thd->lex->chop_off_not_own_tables();
  sp_remove_not_own_routines(thd->lex);
  for (tmp = *tables; tmp; tmp = tmp->next_global) {
    tmp->table = nullptr;
    tmp->mdl_request.ticket = nullptr;
    /* We have to cleanup translation tables of views. */
    tmp->cleanup_items();
  }
  /*
    No need to commit/rollback the statement transaction: it's
    either not started or we're filling in an INFORMATION_SCHEMA
    table on the fly, and thus mustn't manipulate with the
    transaction of the enclosing statement.
  */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT) ||
              (thd->state_flags & Open_tables_state::BACKUPS_AVAIL));
  close_thread_tables(thd);
  thd->mdl_context.rollback_to_savepoint(start_of_statement_svp);
}

/**
  Open a single table without table caching and don't add it to
  THD::open_tables. Depending on the 'add_to_temporary_tables_list' value,
  the opened TABLE instance will be addded to THD::temporary_tables list.

  @param thd                          Thread context.
  @param path                         Path (without .frm)
  @param db                           Database name.
  @param table_name                   Table name.
  @param add_to_temporary_tables_list Specifies if the opened TABLE
                                      instance should be linked into
                                      THD::temporary_tables list.
  @param open_in_engine               Indicates that we need to open table
                                      in storage engine in addition to
                                      constructing TABLE object for it.
  @param table_def                    A data-dictionary Table-object describing
                                      table to be used for opening.

  @note This function is used:
    - by alter_table() to open a temporary table;
    - when creating a temporary table with CREATE TEMPORARY TABLE.

  @return TABLE instance for opened table.
  @retval NULL on error.
*/

TABLE *open_table_uncached(THD *thd, const char *path, const char *db,
                           const char *table_name,
                           bool add_to_temporary_tables_list,
                           bool open_in_engine, const dd::Table &table_def) {
  TABLE *tmp_table;
  TABLE_SHARE *share;
  char cache_key[MAX_DBKEY_LENGTH], *saved_cache_key, *tmp_path;
  size_t key_length;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s'.'%s'  path: '%s'  server_id: %u  "
                       "pseudo_thread_id: %lu",
                       db, table_name, path, (uint)thd->server_id,
                       (ulong)thd->variables.pseudo_thread_id));

  /* Create the cache_key for temporary tables */
  key_length = create_table_def_key_tmp(thd, db, table_name, cache_key);

  if (!(tmp_table = (TABLE *)my_malloc(
            key_memory_TABLE,
            sizeof(*tmp_table) + sizeof(*share) + strlen(path) + 1 + key_length,
            MYF(MY_WME))))
    return nullptr; /* purecov: inspected */

#ifndef DBUG_OFF
  // In order to let purge thread callback call open_table_uncached()
  // we cannot grab LOCK_open here, as that will cause a deadlock.

  // The assert below safeguards against opening a table which is
  // already found in the table definition cache. Iff the table will
  // be opened in the SE below, we may get two conflicting copies of
  // SE private data in the two table_shares.

  // By only grabbing LOCK_open and check the assert only when
  // open_in_engine is true, we safeguard the engine private data while
  // also allowing the purge threads callbacks since they always call
  // with open_in_engine=false.
  if (open_in_engine) {
    mysql_mutex_lock(&LOCK_open);
    DBUG_ASSERT(table_def_cache->count(string(cache_key, key_length)) == 0);
    mysql_mutex_unlock(&LOCK_open);
  }
#endif

  share = (TABLE_SHARE *)(tmp_table + 1);
  tmp_path = (char *)(share + 1);
  saved_cache_key = my_stpcpy(tmp_path, path) + 1;
  memcpy(saved_cache_key, cache_key, key_length);

  init_tmp_table_share(thd, share, saved_cache_key, key_length,
                       strend(saved_cache_key) + 1, tmp_path, nullptr);

  if (open_table_def(thd, share, table_def)) {
    /* No need to lock share->mutex as this is not needed for tmp tables */
    free_table_share(share);
    destroy(tmp_table);
    my_free(tmp_table);
    return nullptr;
  }

#ifdef HAVE_PSI_TABLE_INTERFACE
  share->m_psi = PSI_TABLE_CALL(get_table_share)(true, share);
#else
  share->m_psi = NULL;
#endif

  if (open_table_from_share(
          thd, share, table_name,
          open_in_engine
              ? (uint)(HA_OPEN_KEYFILE | HA_OPEN_RNDFILE | HA_GET_INDEX)
              : 0,
          EXTRA_RECORD, ha_open_options, tmp_table,
          /*
            Set "is_create_table" if the table does not
            exist in SE
          */
          (open_in_engine ? false : true), &table_def)) {
    /* No need to lock share->mutex as this is not needed for tmp tables */
    free_table_share(share);
    destroy(tmp_table);
    my_free(tmp_table);
    return nullptr;
  }

  tmp_table->reginfo.lock_type = TL_WRITE;  // Simulate locked
  share->tmp_table =
      (tmp_table->file->has_transactions() ? TRANSACTIONAL_TMP_TABLE
                                           : NON_TRANSACTIONAL_TMP_TABLE);

  if (add_to_temporary_tables_list) {
    tmp_table->set_binlog_drop_if_temp(
        !thd->is_current_stmt_binlog_disabled() &&
        !thd->is_current_stmt_binlog_format_row());
    /* growing temp list at the head */
    tmp_table->next = thd->temporary_tables;
    if (tmp_table->next) tmp_table->next->prev = tmp_table;
    thd->temporary_tables = tmp_table;
    thd->temporary_tables->prev = nullptr;
    if (thd->slave_thread) {
      ++atomic_slave_open_temp_tables;
      ++thd->rli_slave->get_c_rli()->atomic_channel_open_temp_tables;
    }
  }
  tmp_table->pos_in_table_list = nullptr;

  tmp_table->set_created();

  DBUG_PRINT("tmptable", ("opened table: '%s'.'%s' %p", tmp_table->s->db.str,
                          tmp_table->s->table_name.str, tmp_table));
  return tmp_table;
}

/**
  Delete a temporary table.

  @param thd        Thread handle
  @param base       Handlerton for table to be deleted.
  @param path       Path to the table to be deleted (without
                    an extension).
  @param table_def  dd::Table object describing temporary table
                    to be deleted.

  @retval false - success.
  @retval true  - failure.
*/

bool rm_temporary_table(THD *thd, handlerton *base, const char *path,
                        const dd::Table *table_def) {
  bool error = false;
  handler *file;
  DBUG_TRACE;

  file = get_new_handler((TABLE_SHARE *)nullptr,
                         table_def->partition_type() != dd::Table::PT_NONE,
                         thd->mem_root, base);
  if (file && file->ha_delete_table(path, table_def)) {
    error = true;
    LogErr(WARNING_LEVEL, ER_FAILED_TO_REMOVE_TEMP_TABLE, path, my_errno());
  }
  destroy(file);
  return error;
}

/*****************************************************************************
 * The following find_field_in_XXX procedures implement the core of the
 * name resolution functionality. The entry point to resolve a column name in a
 * list of tables is 'find_field_in_tables'. It calls 'find_field_in_table_ref'
 * for each table reference. In turn, depending on the type of table reference,
 * 'find_field_in_table_ref' calls one of the 'find_field_in_XXX' procedures
 * below specific for the type of table reference.
 *
 * @todo: Refactor the error handling system used by these functions, so that
 *        it is clear when an error is reported and when an empty reference
 *        is returned.
 *
 ******************************************************************************/

/* Special Field pointers as return values of find_field_in_XXX functions. */
Field *not_found_field = (Field *)0x1;
Field *view_ref_found = (Field *)0x2;

#define WRONG_GRANT (Field *)-1

/**
  Find a temporary table specified by TABLE_LIST instance in the cache and
  prepare its TABLE instance for use.

  This function tries to resolve this table in the list of temporary tables
  of this thread. Temporary tables are thread-local and "shadow" base
  tables with the same name.

  @note In most cases one should use open_temporary_tables() instead
        of this call.

  @note One should finalize process of opening temporary table for table
        list element by calling open_and_process_table(). This function
        is responsible for table version checking and handling of merge
        tables.

  @note We used to check global_read_lock before opening temporary tables.
        However, that limitation was artificial and is removed now.

  @return Error status.
    @retval false On success. If a temporary table exists for the given
                  key, tl->table is set.
    @retval true  On error. my_error() has been called.
*/

bool open_temporary_table(THD *thd, TABLE_LIST *tl) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s'.'%s'", tl->db, tl->table_name));

  /*
    Code in open_table() assumes that TABLE_LIST::table can
    be non-zero only for pre-opened temporary tables.
  */
  DBUG_ASSERT(tl->table == nullptr);

  /*
    This function should not be called for cases when derived or I_S
    tables can be met since table list elements for such tables can
    have invalid db or table name.
    Instead open_temporary_tables() should be used.
  */
  DBUG_ASSERT(!tl->is_view_or_derived() && !tl->schema_table);

  if (tl->open_type == OT_BASE_ONLY) {
    DBUG_PRINT("info", ("skip_temporary is set"));
    return false;
  }

  TABLE *table = find_temporary_table(thd, tl);

  if (!table) {
    if (tl->open_type == OT_TEMPORARY_ONLY &&
        tl->open_strategy == TABLE_LIST::OPEN_NORMAL) {
      my_error(ER_NO_SUCH_TABLE, MYF(0), tl->db, tl->table_name);
      return true;
    }
    return false;
  }

  if (tl->partition_names) {
    /* Partitioned temporary tables is not supported. */
    DBUG_ASSERT(!table->part_info);
    my_error(ER_PARTITION_CLAUSE_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  if (table->query_id) {
    /*
      We're trying to use the same temporary table twice in a query.
      Right now we don't support this because a temporary table is always
      represented by only one TABLE object in THD, and it can not be
      cloned. Emit an error for an unsupported behaviour.
    */

    DBUG_PRINT("error", ("query_id: %lu  server_id: %u  pseudo_thread_id: %lu",
                         (ulong)table->query_id, (uint)thd->server_id,
                         (ulong)thd->variables.pseudo_thread_id));
    my_error(ER_CANT_REOPEN_TABLE, MYF(0), table->alias);
    return true;
  }

  table->query_id = thd->query_id;
  thd->thread_specific_used = true;

  tl->set_updatable();  // It is not derived table nor non-updatable VIEW.
  tl->set_insertable();

  tl->table = table;

  table->reset();
  table->init(thd, tl);

  DBUG_PRINT("info", ("Using temporary table"));
  return false;
}

/**
  Pre-open temporary tables corresponding to table list elements.

  @note One should finalize process of opening temporary tables
        by calling open_tables(). This function is responsible
        for table version checking and handling of merge tables.

  @return Error status.
    @retval false On success. If a temporary tables exists for the
                  given element, tl->table is set.
    @retval true  On error. my_error() has been called.
*/

bool open_temporary_tables(THD *thd, TABLE_LIST *tl_list) {
  TABLE_LIST *first_not_own = thd->lex->first_not_own_table();
  DBUG_TRACE;

  for (TABLE_LIST *tl = tl_list; tl && tl != first_not_own;
       tl = tl->next_global) {
    if (tl->is_view_or_derived() || tl->schema_table) {
      /*
        Derived and I_S tables will be handled by a later call to open_tables().
      */
      continue;
    }

    if (open_temporary_table(thd, tl)) return true;
  }

  return false;
}

/*
  Find a field by name in a view that uses merge algorithm.

  SYNOPSIS
    find_field_in_view()
    thd				thread handler
    table_list			view to search for 'name'
    name			name of field
    ref				expression substituted in VIEW should be passed
                                using this reference (return view_ref_found)
    register_tree_change        true if ref is not stack variable and we
                                need register changes in item tree

  RETURN
    0			field is not found
    view_ref_found	found value in VIEW (real result is in *ref)
    #			pointer to field - only for schema table fields
*/

static Field *find_field_in_view(THD *thd, TABLE_LIST *table_list,
                                 const char *name, Item **ref,
                                 bool register_tree_change) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("view: '%s', field name: '%s', ref %p",
                       table_list->alias, name, ref));
  Field_iterator_view field_it;
  field_it.set(table_list);

  DBUG_ASSERT(table_list->schema_table_reformed ||
              (ref != nullptr && table_list->is_merged()));
  for (; !field_it.end_of_fields(); field_it.next()) {
    if (!my_strcasecmp(system_charset_info, field_it.name(), name)) {
      Item *item;

      {
        /*
          Use own arena for Prepared Statements or data will be freed after
          PREPARE.
        */
        Prepared_stmt_arena_holder ps_arena_holder(
            thd, register_tree_change &&
                     thd->stmt_arena->is_stmt_prepare_or_first_stmt_execute());

        /*
          create_item() may, or may not create a new Item, depending on
          the column reference. See create_view_field() for details.
        */
        item = field_it.create_item(thd);

        if (!item) return nullptr;
      }

      /*
       *ref != NULL means that *ref contains the item that we need to
       replace. If the item was aliased by the user, set the alias to
       the replacing item.
       We need to set alias on both ref itself and on ref real item.
      */
      if (*ref && !(*ref)->item_name.is_autogenerated()) {
        item->item_name = (*ref)->item_name;
        item->real_item()->item_name = (*ref)->item_name;
      }
      if (register_tree_change)
        thd->change_item_tree(ref, item);
      else
        *ref = item;
      return view_ref_found;
    }
  }
  return nullptr;
}

/**
  Find field by name in a NATURAL/USING join table reference.

  @param thd thread handler
  @param table_ref table reference to search
  @param name name of field
  @param [in,out] ref if 'name' is resolved to a view field, ref is
                               set to point to the found view field
  @param register_tree_change true if ref is not stack variable and we
                               need register changes in item tree
  @param [out] actual_table    The original table reference where the field
                               belongs - differs from 'table_list' only for
                               NATURAL/USING joins

  DESCRIPTION
    Search for a field among the result fields of a NATURAL/USING join.
    Notice that this procedure is called only for non-qualified field
    names. In the case of qualified fields, we search directly the base
    tables of a natural join.

    Sometimes when a field is found, it is checked for privileges according to
    THD::want_privilege and marked according to THD::mark_used_columns.
    But it is unclear when, so caller generally has to do the same.

  RETURN
    NULL        if the field was not found
    WRONG_GRANT if no access rights to the found field
    #           Pointer to the found Field
*/

static Field *find_field_in_natural_join(THD *thd, TABLE_LIST *table_ref,
                                         const char *name, Item **ref,
                                         bool register_tree_change,
                                         TABLE_LIST **actual_table) {
  List_iterator_fast<Natural_join_column> field_it(*(table_ref->join_columns));
  Natural_join_column *nj_col, *curr_nj_col;
  Field *found_field = nullptr;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("field name: '%s', ref %p", name, ref));
  DBUG_ASSERT(table_ref->is_natural_join && table_ref->join_columns);
  DBUG_ASSERT(*actual_table == nullptr);

  for (nj_col = nullptr, curr_nj_col = field_it++; curr_nj_col;
       curr_nj_col = field_it++) {
    if (!my_strcasecmp(system_charset_info, curr_nj_col->name(), name)) {
      if (nj_col) {
        my_error(ER_NON_UNIQ_ERROR, MYF(0), name, thd->where);
        return nullptr;
      }
      nj_col = curr_nj_col;
    }
  }
  if (!nj_col) return nullptr;

  if (nj_col->view_field) {
    Item *item;

    {
      Prepared_stmt_arena_holder ps_arena_holder(thd, register_tree_change);

      /*
        create_item() may, or may not create a new Item, depending on the
        column reference. See create_view_field() for details.
      */
      item = nj_col->create_item(thd);

      if (!item) return nullptr;
    }

    /*
     *ref != NULL means that *ref contains the item that we need to
     replace. If the item was aliased by the user, set the alias to
     the replacing item.
     We need to set alias on both ref itself and on ref real item.
     */
    if (*ref && !(*ref)->item_name.is_autogenerated()) {
      item->item_name = (*ref)->item_name;
      item->real_item()->item_name = (*ref)->item_name;
    }

    DBUG_ASSERT(nj_col->table_field == nullptr);
    if (nj_col->table_ref->schema_table_reformed) {
      /*
        Translation table items are always Item_fields and fixed
        already('mysql_schema_table' function). So we can return
        ->field. It is used only for 'show & where' commands.
      */
      return ((Item_field *)(nj_col->view_field->item))->field;
    }
    if (register_tree_change)
      thd->change_item_tree(ref, item);
    else
      *ref = item;
    found_field = view_ref_found;
  } else {
    /* This is a base table. */
    DBUG_ASSERT(nj_col->view_field == nullptr);
    /*
      This fix_fields is not necessary (initially this item is fixed by
      the Item_field constructor; after reopen_tables the Item_func_eq
      calls fix_fields on that item), it's just a check during table
      reopening for columns that was dropped by the concurrent connection.
    */
    if (!nj_col->table_field->fixed &&
        nj_col->table_field->fix_fields(thd, (Item **)&nj_col->table_field)) {
      DBUG_PRINT("info",
                 ("column '%s' was dropped by the concurrent connection",
                  nj_col->table_field->item_name.ptr()));
      return nullptr;
    }
    DBUG_ASSERT(nj_col->table_ref->table == nj_col->table_field->field->table);
    found_field = nj_col->table_field->field;
  }

  *actual_table = nj_col->table_ref;

  return found_field;
}

/*
  Find field by name in a base table.

  No privileges are checked, and the column is not marked in read_set/write_set.

  SYNOPSIS
    find_field_in_table()
    table			table where to search for the field
    name			name of field
    length			length of name
    allow_rowid			do allow finding of "_rowid" field?
    cached_field_index_ptr	cached position in field list (used to speedup
                                lookup for fields in prepared tables)

  RETURN
    0	field is not found
    #	pointer to field
*/

Field *find_field_in_table(TABLE *table, const char *name, size_t length,
                           bool allow_rowid, uint *cached_field_index_ptr) {
  Field **field_ptr = nullptr, *field;
  uint cached_field_index = *cached_field_index_ptr;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: '%s', field name: '%s'", table->alias, name));

  /* We assume here that table->field < NO_CACHED_FIELD_INDEX = UINT_MAX */
  if (cached_field_index < table->s->fields &&
      !my_strcasecmp(system_charset_info,
                     table->field[cached_field_index]->field_name, name))
    field_ptr = table->field + cached_field_index;
  else if (table->s->name_hash != nullptr) {
    const auto it = table->s->name_hash->find(std::string(name, length));
    if (it != table->s->name_hash->end()) {
      /*
        field_ptr points to field in TABLE_SHARE. Convert it to the matching
        field in table
      */
      field_ptr = (table->field + (it->second - table->s->field));
    }
  } else {
    if (!(field_ptr = table->field)) return (Field *)nullptr;
    for (; *field_ptr; ++field_ptr)
      if (!my_strcasecmp(system_charset_info, (*field_ptr)->field_name, name))
        break;
  }

  if (field_ptr && *field_ptr) {
    *cached_field_index_ptr = field_ptr - table->field;
    field = *field_ptr;
  } else {
    if (!allow_rowid || my_strcasecmp(system_charset_info, name, "_rowid") ||
        table->s->rowid_field_offset == 0)
      return (Field *)nullptr;
    field = table->field[table->s->rowid_field_offset - 1];
  }

  return field;
}

/*
  Find field in a table reference.

  SYNOPSIS
    find_field_in_table_ref()
    thd			   [in]  thread handler
    table_list		   [in]  table reference to search
    name		   [in]  name of field
    length		   [in]  field length of name
    item_name              [in]  name of item if it will be created (VIEW)
    db_name                [in]  optional database name that qualifies the
    table_name             [in]  optional table name that qualifies the field
    ref		       [in/out] if 'name' is resolved to a view field, ref
                                 is set to point to the found view field
    want_privilege         [in]  privileges to check for column
                                 = 0: no privilege checking is needed
    allow_rowid		   [in]  do allow finding of "_rowid" field?
    cached_field_index_ptr [in]  cached position in field list (used to
                                 speedup lookup for fields in prepared tables)
    register_tree_change   [in]  true if ref is not stack variable and we
                                 need register changes in item tree
    actual_table           [out] the original table reference where the field
                                 belongs - differs from 'table_list' only for
                                 NATURAL_USING joins.

  DESCRIPTION
    Find a field in a table reference depending on the type of table
    reference. There are three types of table references with respect
    to the representation of their result columns:
    - an array of Field_translator objects for MERGE views and some
      information_schema tables,
    - an array of Field objects (and possibly a name hash) for stored
      tables,
    - a list of Natural_join_column objects for NATURAL/USING joins.
    This procedure detects the type of the table reference 'table_list'
    and calls the corresponding search routine.

    The function checks column-level privileges for the found field
    according to argument want_privilege.

    The function marks the column in corresponding table's read set or
    write set according to THD::mark_used_columns.

  RETURN
    0			field is not found
    view_ref_found	found value in VIEW (real result is in *ref)
    #			pointer to field
*/

Field *find_field_in_table_ref(THD *thd, TABLE_LIST *table_list,
                               const char *name, size_t length,
                               const char *item_name, const char *db_name,
                               const char *table_name, Item **ref,
                               ulong want_privilege, bool allow_rowid,
                               uint *cached_field_index_ptr,
                               bool register_tree_change,
                               TABLE_LIST **actual_table) {
  if (table_list->m_was_scalar_subquery) {
    // We may get here during EXECUTE if a scalar subquery has been transformed
    // into a derived table (table_list) during PREPARE. When resolving a GROUP
    // BY in the transformed query block (in the case where we do not transform
    // grouping into a separate derived table, i.e. we group on a scalar
    // subquery's alias), table_list hasn't yet been set up for materialization
    // so we cannot resolve against it, and in any case, no GROUP BY column
    // should be resolved against it (but rather against the scalar subquery's
    // alias, so just return. Remove after WL#6570.
    return nullptr;
  }
  Field *fld;
  DBUG_TRACE;
  DBUG_ASSERT(table_list->alias);
  DBUG_ASSERT(name);
  DBUG_ASSERT(item_name);
  DBUG_PRINT("enter", ("table: '%s'  field name: '%s'  item name: '%s'  ref %p",
                       table_list->alias, name, item_name, ref));

  /*
    Check that the table and database that qualify the current field name
    are the same as the table reference we are going to search for the field.

    Exclude from the test below nested joins because the columns in a
    nested join generally originate from different tables. Nested joins
    also have no table name, except when a nested join is a merge view
    or an information schema table.

    We include explicitly table references with a 'field_translation' table,
    because if there are views over natural joins we don't want to search
    inside the view, but we want to search directly in the view columns
    which are represented as a 'field_translation'.

    TODO: Ensure that table_name, db_name and tables->db always points to
          something !
  */
  if (/* Exclude nested joins. */
      (!table_list->nested_join ||
       /* Include merge views and information schema tables. */
       table_list->field_translation) &&
      /*
        Test if the field qualifiers match the table reference we plan
        to search.
      */
      table_name && table_name[0] &&
      (my_strcasecmp(table_alias_charset, table_list->alias, table_name) ||
       (db_name && db_name[0] && table_list->db && table_list->db[0] &&
        (table_list->schema_table
             ? my_strcasecmp(system_charset_info, db_name, table_list->db)
             : strcmp(db_name, table_list->db)))))
    return nullptr;

  *actual_table = nullptr;

  if (table_list->field_translation) {
    /* 'table_list' is a view or an information schema table. */
    if ((fld = find_field_in_view(thd, table_list, name, ref,
                                  register_tree_change)))
      *actual_table = table_list;
  } else if (!table_list->nested_join) {
    /* 'table_list' is a stored table. */
    DBUG_ASSERT(table_list->table);
    if ((fld = find_field_in_table(table_list->table, name, length, allow_rowid,
                                   cached_field_index_ptr)))
      *actual_table = table_list;
  } else {
    /*
      'table_list' is a NATURAL/USING join, or an operand of such join that
      is a nested join itself.

      If the field name we search for is qualified, then search for the field
      in the table references used by NATURAL/USING the join.
    */
    if (table_name && table_name[0]) {
      for (TABLE_LIST *table : table_list->nested_join->join_list) {
        if ((fld = find_field_in_table_ref(
                 thd, table, name, length, item_name, db_name, table_name, ref,
                 want_privilege, allow_rowid, cached_field_index_ptr,
                 register_tree_change, actual_table)))
          return fld;
      }
      return nullptr;
    }
    /*
      Non-qualified field, search directly in the result columns of the
      natural join. The condition of the outer IF is true for the top-most
      natural join, thus if the field is not qualified, we will search
      directly the top-most NATURAL/USING join.
    */
    fld = find_field_in_natural_join(thd, table_list, name, ref,
                                     register_tree_change, actual_table);
  }

  if (fld) {
    // Check if there are sufficient privileges to the found field.
    if (want_privilege) {
      if (fld != view_ref_found) {
        if (check_column_grant_in_table_ref(thd, *actual_table, name, length,
                                            want_privilege))
          return WRONG_GRANT;
      } else {
        DBUG_ASSERT(ref && *ref && (*ref)->fixed);
        DBUG_ASSERT(*actual_table ==
                    (down_cast<Item_ident *>(*ref))->cached_table);

        Column_privilege_tracker tracker(thd, want_privilege);
        if ((*ref)->walk(&Item::check_column_privileges, enum_walk::PREFIX,
                         (uchar *)thd))
          return WRONG_GRANT;
      }
    }

    /*
      Get read_set correct for this field so that the handler knows that
      this field is involved in the query and gets retrieved.
    */
    if (fld == view_ref_found) {
      Mark_field mf(thd->mark_used_columns);
      (*ref)->walk(&Item::mark_field_in_map, enum_walk::SUBQUERY_POSTFIX,
                   (uchar *)&mf);
    } else  // surely fld != NULL (see outer if())
      fld->table->mark_column_used(fld, thd->mark_used_columns);
  }
  return fld;
}

/*
  Find field in table, no side effects, only purpose is to check for field
  in table object and get reference to the field if found.

  SYNOPSIS
  find_field_in_table_sef()

  table                         table where to find
  name                          Name of field searched for

  RETURN
    0                   field is not found
    #                   pointer to field
*/

Field *find_field_in_table_sef(TABLE *table, const char *name) {
  Field **field_ptr = nullptr;
  if (table->s->name_hash != nullptr) {
    const auto it = table->s->name_hash->find(name);
    if (it != table->s->name_hash->end()) {
      /*
        field_ptr points to field in TABLE_SHARE. Convert it to the matching
        field in table
      */
      field_ptr = (table->field + (it->second - table->s->field));
      if (field_ptr) return *field_ptr;
    }
  } else {
    if (!table->field) return nullptr;
    for (uint i = 0; i < table->s->fields; ++i)
      if (table->field[i] && !my_strcasecmp(system_charset_info,
                                            table->field[i]->field_name, name))
        return table->field[i];
  }
  return nullptr;
}

/*
  Find field in table list.

  SYNOPSIS
    find_field_in_tables()
    thd			  pointer to current thread structure
    item		  field item that should be found
    first_table           list of tables to be searched for item
    last_table            end of the list of tables to search for item. If NULL
                          then search to the end of the list 'first_table'.
    ref			  if 'item' is resolved to a view field, ref is set to
                          point to the found view field
    report_error	  Degree of error reporting:
                          - IGNORE_ERRORS then do not report any error
                          - IGNORE_EXCEPT_NON_UNIQUE report only non-unique
                            fields, suppress all other errors
                          - REPORT_EXCEPT_NON_UNIQUE report all other errors
                            except when non-unique fields were found
                          - REPORT_ALL_ERRORS
    want_privilege        column privileges to check
                          = 0: no need to check privileges
    register_tree_change  true if ref is not a stack variable and we
                          to need register changes in item tree

  RETURN VALUES
    0			If error: the found field is not unique, or there are
                        no sufficient access priviliges for the found field,
                        or the field is qualified with non-existing table.
    not_found_field	The function was called with report_error ==
                        (IGNORE_ERRORS || IGNORE_EXCEPT_NON_UNIQUE) and a
                        field was not found.
    view_ref_found	View field is found, item passed through ref parameter
    found field         If a item was resolved to some field
*/

Field *find_field_in_tables(THD *thd, Item_ident *item, TABLE_LIST *first_table,
                            TABLE_LIST *last_table, Item **ref,
                            find_item_error_report_type report_error,
                            ulong want_privilege, bool register_tree_change) {
  Field *found = nullptr;
  const char *db = item->db_name;
  const char *table_name = item->table_name;
  const char *name = item->field_name;
  size_t length = strlen(name);
  char name_buff[NAME_LEN + 1];
  TABLE_LIST *actual_table;
  bool allow_rowid;

  if (!table_name || !table_name[0]) {
    table_name = nullptr;  // For easier test
    db = nullptr;
  }

  allow_rowid = table_name || (first_table && !first_table->next_local);

  if (item->cached_table) {
    /*
      This shortcut is used by prepared statements. We assume that
      TABLE_LIST *first_table is not changed during query execution (which
      is true for all queries except RENAME but luckily RENAME doesn't
      use fields...) so we can rely on reusing pointer to its member.
      With this optimization we also miss case when addition of one more
      field makes some prepared query ambiguous and so erroneous, but we
      accept this trade off.
    */
    TABLE_LIST *table_ref = item->cached_table;
    /*
      The condition (table_ref->view == NULL) ensures that we will call
      find_field_in_table even in the case of information schema tables
      when table_ref->field_translation != NULL.
      */
    if (table_ref->table && !table_ref->is_view()) {
      found = find_field_in_table(table_ref->table, name, length, true,
                                  &(item->cached_field_index));
      // Check if there are sufficient privileges to the found field.
      if (found && want_privilege &&
          check_column_grant_in_table_ref(thd, table_ref, name, length,
                                          want_privilege))
        found = WRONG_GRANT;
      if (found && found != WRONG_GRANT)
        table_ref->table->mark_column_used(found, thd->mark_used_columns);
    } else
      found = find_field_in_table_ref(
          thd, table_ref, name, length, item->item_name.ptr(), nullptr, nullptr,
          ref, want_privilege, true, &(item->cached_field_index),
          register_tree_change, &actual_table);
    if (found) {
      if (found == WRONG_GRANT) return nullptr;

      return found;
    }
  }

  if (db && (lower_case_table_names || is_infoschema_db(db, strlen(db)))) {
    /*
      convert database to lower case for comparison.
      We can't do this in Item_field as this would change the
      'name' of the item which may be used in the select list

      The 'information_schema' name is treated as case-insenstive
      identifier when specified in FROM clause even in
      lower_case_table_names=0. We lowercase the 'information_schema' name
      below to treat it as case-insensitive even when it is refered in WHERE
      or SELECT clause.
    */
    strmake(name_buff, db, sizeof(name_buff) - 1);
    my_casedn_str(files_charset_info, name_buff);
    db = name_buff;
  }

  /*
    @todo after WL#6570 which doesn't re-resolve, remove comment and simplify
    code (probably back to how it was before WL#8652).
    It can happen that end_lateral_table is NOT somewhere in the list between
    first_table and last_table. Indeed, consider:
    SELECT COUNT(*) FROM t1 GROUP BY t1.a
    HAVING t1.a IN (SELECT t3.a FROM t1 AS t3
    WHERE t3.b IN (SELECT b FROM t2, lateral (select t1.a) dt));
    We resolve the body of 'dt' then we do semijoin transformation:
    ... HAVING t1.a IN (SELECT FROM t3 SEMIJOIN (t2, dt) ON ...)
    We save that as prepared statement.
    Then we execute the statement: when we resolve the body of 'dt' again, we
    look up the outer reference (t1.a of dt's body) into the FROM clause of
    the immediate outer query block, which starts at t3. As semijoin
    transformation doesn't update TABLE_LIST::next_name_resolution_context
    (see comment in convert_subquery_to_semijoin()), the name resolution
    context of this FROM is {t3} only.
    When loop starts, 'last_table' is supposed to mean "stop loop when you
    meet this table". But the loop will not meet end_lateral_table (dt) so
    will go wrong.
    So we refined the condition to "stop loop when you meet this or that
    table". And added testcase to derived_correlated.test in -ps mode.
  */
  TABLE_LIST *last_table2 = nullptr;
  if (first_table && first_table->select_lex &&
      first_table->select_lex->end_lateral_table)
    last_table2 = first_table->select_lex->end_lateral_table;
  if (last_table) last_table = last_table->next_name_resolution_table;

  auto cur_table = first_table;
  for (; cur_table != last_table && cur_table != last_table2;
       cur_table = cur_table->next_name_resolution_table) {
    Field *cur_field = find_field_in_table_ref(
        thd, cur_table, name, length, item->item_name.ptr(), db, table_name,
        ref, want_privilege, allow_rowid, &(item->cached_field_index),
        register_tree_change, &actual_table);
    if ((cur_field == nullptr && thd->is_error()) || cur_field == WRONG_GRANT)
      return nullptr;

    if (cur_field) {
      /*
        Store the original table of the field, which may be different from
        cur_table in the case of NATURAL/USING join.
      */
      item->cached_table =
          (!actual_table->cacheable_table || found) ? nullptr : actual_table;

      DBUG_ASSERT(thd->where);
      /*
        If we found a fully qualified field we return it directly as it can't
        have duplicates.
       */
      if (db) return cur_field;

      if (found) {
        if (report_error == REPORT_ALL_ERRORS ||
            report_error == IGNORE_EXCEPT_NON_UNIQUE)
          my_error(ER_NON_UNIQ_ERROR, MYF(0),
                   table_name ? item->full_name() : name, thd->where);
        return (Field *)nullptr;
      }
      found = cur_field;
    }
  }

  if (found) return found;

  /*
    If the field was qualified and there were no tables to search, issue
    an error that an unknown table was given. The situation is detected
    as follows: if there were no tables we wouldn't go through the loop
    and cur_table wouldn't be updated by the loop increment part, so it
    will be equal to the first table.
  */
  if (table_name && (cur_table == first_table) &&
      (report_error == REPORT_ALL_ERRORS ||
       report_error == REPORT_EXCEPT_NON_UNIQUE)) {
    char buff[NAME_LEN * 2 + 2];
    if (db && db[0]) {
      strxnmov(buff, sizeof(buff) - 1, db, ".", table_name, NullS);
      table_name = buff;
    }
    my_error(ER_UNKNOWN_TABLE, MYF(0), table_name, thd->where);
  } else {
    if (report_error == REPORT_ALL_ERRORS ||
        report_error == REPORT_EXCEPT_NON_UNIQUE) {
      /* We now know that this column does not exist in any table_list
         of the query. If user does not have grant, then we should throw
         error stating 'access denied'. If user does have right then we can
         give proper error like column does not exist. Following is check
         to see if column has wrong grants and avoids error like 'bad field'
         and throw column access error.
      */
      if (!first_table || (want_privilege == 0) ||
          !check_column_grant_in_table_ref(thd, first_table, name, length,
                                           want_privilege))
        my_error(ER_BAD_FIELD_ERROR, MYF(0), item->full_name(), thd->where);
    } else
      found = not_found_field;
  }
  return found;
}

/*
  Find Item in list of items (find_field_in_tables analog)

  TODO
    is it better return only counter?

  SYNOPSIS
    find_item_in_list()
    find			Item to find
    items			List of items
    counter			To return number of found item
    report_error
      REPORT_ALL_ERRORS		report errors, return 0 if error
      REPORT_EXCEPT_NOT_FOUND	Do not report 'not found' error and
                                return not_found_item, report other errors,
                                return 0
      IGNORE_ERRORS		Do not report errors, return 0 if error
    resolution                  Set to the resolution type if the item is found
                                (it says whether the item is resolved
                                 against an alias name,
                                 or as a field name without alias,
                                 or as a field hidden by alias,
                                 or ignoring alias)

  RETURN VALUES
    0			Item is not found or item is not unique,
                        error message is reported
    not_found_item	Function was called with
                        report_error == REPORT_EXCEPT_NOT_FOUND and
                        item was not found. No error message was reported
                        found field
*/

/* Special Item pointer to serve as a return value from find_item_in_list(). */
Item **not_found_item = (Item **)0x1;

Item **find_item_in_list(THD *thd, Item *find, List<Item> &items, uint *counter,
                         find_item_error_report_type report_error,
                         enum_resolution_type *resolution) {
  List_iterator<Item> li(items);
  Item **found = nullptr, **found_unaliased = nullptr, *item;
  const char *db_name = nullptr;
  const char *field_name = nullptr;
  const char *table_name = nullptr;
  bool found_unaliased_non_uniq = false;
  /*
    true if the item that we search for is a valid name reference
    (and not an item that happens to have a name).
  */
  bool is_ref_by_name = false;
  uint unaliased_counter = 0;

  *resolution = NOT_RESOLVED;

  is_ref_by_name =
      (find->type() == Item::FIELD_ITEM || find->type() == Item::REF_ITEM);
  if (is_ref_by_name) {
    field_name = ((Item_ident *)find)->field_name;
    table_name = ((Item_ident *)find)->table_name;
    db_name = ((Item_ident *)find)->db_name;
  }

  for (uint i = 0; (item = li++); i++) {
    if (field_name && item->real_item()->type() == Item::FIELD_ITEM) {
      Item_ident *item_field = (Item_ident *)item;

      /*
        In case of group_concat() with ORDER BY condition in the QUERY
        item_field can be field of temporary table without item name
        (if this field created from expression argument of group_concat()),
        => we have to check presence of name before compare
      */
      if (!item_field->item_name.is_set()) continue;

      if (table_name) {
        /*
          If table name is specified we should find field 'field_name' in
          table 'table_name'. According to SQL-standard we should ignore
          aliases in this case.

          Since we should NOT prefer fields from the select list over
          other fields from the tables participating in this select in
          case of ambiguity we have to do extra check outside this function.

          We use strcmp for table names and database names as these may be
          case sensitive. In cases where they are not case sensitive, they
          are always in lower case.

          item_field->field_name and item_field->table_name can be 0x0 if
          item is not fix_field()'ed yet.
        */
        if (item_field->field_name && item_field->table_name &&
            !my_strcasecmp(system_charset_info, item_field->field_name,
                           field_name) &&
            !my_strcasecmp(table_alias_charset, item_field->table_name,
                           table_name) &&
            (!db_name ||
             (item_field->db_name && !strcmp(item_field->db_name, db_name)))) {
          if (found_unaliased) {
            if ((*found_unaliased)->eq(item, false)) continue;
            /*
              Two matching fields in select list.
              We already can bail out because we are searching through
              unaliased names only and will have duplicate error anyway.
            */
            if (report_error != IGNORE_ERRORS)
              my_error(ER_NON_UNIQ_ERROR, MYF(0), find->full_name(),
                       thd->where);
            return (Item **)nullptr;
          }
          found_unaliased = li.ref();
          unaliased_counter = i;
          *resolution = RESOLVED_IGNORING_ALIAS;
          if (db_name) break;  // Perfect match
        }
      } else {
        int fname_cmp = my_strcasecmp(system_charset_info,
                                      item_field->field_name, field_name);
        if (item_field->item_name.eq_safe(field_name)) {
          /*
            If table name was not given we should scan through aliases
            and non-aliased fields first. We are also checking unaliased
            name of the field in then next  else-if, to be able to find
            instantly field (hidden by alias) if no suitable alias or
            non-aliased field was found.
          */
          if (found) {
            if ((*found)->eq(item, false)) continue;  // Same field twice
            if (report_error != IGNORE_ERRORS)
              my_error(ER_NON_UNIQ_ERROR, MYF(0), find->full_name(),
                       thd->where);
            return (Item **)nullptr;
          }
          found = li.ref();
          *counter = i;
          *resolution =
              fname_cmp ? RESOLVED_AGAINST_ALIAS : RESOLVED_WITH_NO_ALIAS;
        } else if (!fname_cmp) {
          /*
            We will use non-aliased field or react on such ambiguities only if
            we won't be able to find aliased field.
            Again if we have ambiguity with field outside of select list
            we should prefer fields from select list.
          */
          if (found_unaliased) {
            if ((*found_unaliased)->eq(item, false))
              continue;  // Same field twice
            found_unaliased_non_uniq = true;
          }
          found_unaliased = li.ref();
          unaliased_counter = i;
        }
      }
    } else if (!table_name) {
      if (is_ref_by_name && item->item_name.eq_safe(find->item_name)) {
        found = li.ref();
        *counter = i;
        *resolution = RESOLVED_AGAINST_ALIAS;
        break;
      } else if (find->eq(item, false)) {
        found = li.ref();
        *counter = i;
        *resolution = RESOLVED_IGNORING_ALIAS;
        break;
      }
    } else if (table_name && item->type() == Item::REF_ITEM &&
               ((Item_ref *)item)->ref_type() == Item_ref::VIEW_REF) {
      /*
        TODO:Here we process prefixed view references only. What we should
        really do is process all types of Item_refs. But this will currently
        lead to a clash with the way references to outer SELECTs (from the
        HAVING clause) are handled in e.g. :
        SELECT 1 FROM t1 AS t1_o GROUP BY a
          HAVING (SELECT t1_o.a FROM t1 AS t1_i GROUP BY t1_i.a LIMIT 1).
        Processing all Item_refs here will cause t1_o.a to resolve to itself.
        We still need to process the special case of Item_view_ref
        because in the context of views they have the same meaning as
        Item_field for tables.
      */
      Item_ident *item_ref = (Item_ident *)item;
      if (item_ref->item_name.eq_safe(field_name) && item_ref->table_name &&
          !my_strcasecmp(table_alias_charset, item_ref->table_name,
                         table_name) &&
          (!db_name ||
           (item_ref->db_name && !strcmp(item_ref->db_name, db_name)))) {
        found = li.ref();
        *counter = i;
        *resolution = RESOLVED_IGNORING_ALIAS;
        break;
      }
    }
  }
  if (!found) {
    if (found_unaliased_non_uniq) {
      if (report_error != IGNORE_ERRORS)
        my_error(ER_NON_UNIQ_ERROR, MYF(0), find->full_name(), thd->where);
      return (Item **)nullptr;
    }
    if (found_unaliased) {
      found = found_unaliased;
      *counter = unaliased_counter;
      *resolution = RESOLVED_BEHIND_ALIAS;
    }
  }
  if (found) return found;
  if (report_error != REPORT_EXCEPT_NOT_FOUND) {
    if (report_error == REPORT_ALL_ERRORS)
      my_error(ER_BAD_FIELD_ERROR, MYF(0), find->full_name(), thd->where);
    return (Item **)nullptr;
  } else
    return not_found_item;
}

/*
  Test if a string is a member of a list of strings.

  SYNOPSIS
    test_if_string_in_list()
    find      the string to look for
    str_list  a list of strings to be searched

  DESCRIPTION
    Sequentially search a list of strings for a string, and test whether
    the list contains the same string.

  RETURN
    true  if find is in str_list
    false otherwise
*/

static bool test_if_string_in_list(const char *find, List<String> *str_list) {
  List_iterator<String> str_list_it(*str_list);
  String *curr_str;
  size_t find_length = strlen(find);
  while ((curr_str = str_list_it++)) {
    if (find_length != curr_str->length()) continue;
    if (!my_strcasecmp(system_charset_info, find, curr_str->ptr())) return true;
  }
  return false;
}

/*
  Create a new name resolution context for an item so that it is
  being resolved in a specific table reference.

  SYNOPSIS
    set_new_item_local_context()
    thd        pointer to current thread
    item       item for which new context is created and set
    table_ref  table ref where an item showld be resolved

  DESCRIPTION
    Create a new name resolution context for an item, so that the item
    is resolved only the supplied 'table_ref'.

  RETURN
    false  if all OK
    true   otherwise
*/

static bool set_new_item_local_context(THD *thd, Item_ident *item,
                                       TABLE_LIST *table_ref) {
  Name_resolution_context *context;
  if (!(context = new (thd->mem_root) Name_resolution_context))
    return true; /* purecov: inspected */
  context->init();
  context->first_name_resolution_table = context->last_name_resolution_table =
      table_ref;
  context->select_lex = table_ref->select_lex;
  context->next_context = table_ref->select_lex->first_context;
  table_ref->select_lex->first_context = context;
  item->context = context;
  return false;
}

/*
  Find and mark the common columns of two table references.

  SYNOPSIS
    mark_common_columns()
    thd                [in] current thread
    table_ref_1        [in] the first (left) join operand
    table_ref_2        [in] the second (right) join operand
    using_fields       [in] if the join is JOIN...USING - the join columns,
                            if NATURAL join, then NULL
    found_using_fields [out] number of fields from the USING clause that were
                             found among the common fields

  DESCRIPTION
    The procedure finds the common columns of two relations (either
    tables or intermediate join results), and adds an equi-join condition
    to the ON clause of 'table_ref_2' for each pair of matching columns.
    If some of table_ref_XXX represents a base table or view, then we
    create new 'Natural_join_column' instances for each column
    reference and store them in the 'join_columns' of the table
    reference.

  IMPLEMENTATION
    The procedure assumes that store_natural_using_join_columns() was
    called for the previous level of NATURAL/USING joins.

  RETURN
    true   error when some common column is non-unique, or out of memory
    false  OK
*/

static bool mark_common_columns(THD *thd, TABLE_LIST *table_ref_1,
                                TABLE_LIST *table_ref_2,
                                List<String> *using_fields,
                                uint *found_using_fields) {
  Field_iterator_table_ref it_1, it_2;
  Natural_join_column *nj_col_1, *nj_col_2;
  bool first_outer_loop = true;
  List<Field> fields;
  /*
    Leaf table references to which new natural join columns are added
    if the leaves are != NULL.
  */
  TABLE_LIST *leaf_1 =
      (table_ref_1->nested_join && !table_ref_1->is_natural_join) ? nullptr
                                                                  : table_ref_1;
  TABLE_LIST *leaf_2 =
      (table_ref_2->nested_join && !table_ref_2->is_natural_join) ? nullptr
                                                                  : table_ref_2;

  DBUG_TRACE;
  DBUG_PRINT("info", ("operand_1: %s  operand_2: %s", table_ref_1->alias,
                      table_ref_2->alias));

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  *found_using_fields = 0;

  for (it_1.set(table_ref_1); !it_1.end_of_fields(); it_1.next()) {
    bool found = false;
    const char *field_name_1;
    /* true if field_name_1 is a member of using_fields */
    bool is_using_column_1;
    if (!(nj_col_1 = it_1.get_or_create_column_ref(thd, leaf_1))) return true;
    field_name_1 = nj_col_1->name();
    is_using_column_1 =
        using_fields && test_if_string_in_list(field_name_1, using_fields);
    DBUG_PRINT("info", ("field_name_1=%s.%s",
                        nj_col_1->table_name() ? nj_col_1->table_name() : "",
                        field_name_1));

    /*
      Find a field with the same name in table_ref_2.

      Note that for the second loop, it_2.set() will iterate over
      table_ref_2->join_columns and not generate any new elements or
      lists.
    */
    nj_col_2 = nullptr;
    for (it_2.set(table_ref_2); !it_2.end_of_fields(); it_2.next()) {
      Natural_join_column *cur_nj_col_2;
      const char *cur_field_name_2;
      if (!(cur_nj_col_2 = it_2.get_or_create_column_ref(thd, leaf_2)))
        return true;
      cur_field_name_2 = cur_nj_col_2->name();
      DBUG_PRINT("info",
                 ("cur_field_name_2=%s.%s",
                  cur_nj_col_2->table_name() ? cur_nj_col_2->table_name() : "",
                  cur_field_name_2));

      /*
        Compare the two columns and check for duplicate common fields.
        A common field is duplicate either if it was already found in
        table_ref_2 (then found == true), or if a field in table_ref_2
        was already matched by some previous field in table_ref_1
        (then cur_nj_col_2->is_common == true).
        Note that it is too early to check the columns outside of the
        USING list for ambiguity because they are not actually "referenced"
        here. These columns must be checked only on unqualified reference
        by name (e.g. in SELECT list).
      */
      if (!my_strcasecmp(system_charset_info, field_name_1, cur_field_name_2)) {
        DBUG_PRINT("info", ("match c1.is_common=%d", nj_col_1->is_common));
        if (cur_nj_col_2->is_common ||
            (found && (!using_fields || is_using_column_1))) {
          my_error(ER_NON_UNIQ_ERROR, MYF(0), field_name_1, thd->where);
          return true;
        }
        nj_col_2 = cur_nj_col_2;
        found = true;
      }
    }
    if (first_outer_loop && leaf_2) {
      /*
        Make sure that the next inner loop "knows" that all columns
        are materialized already.
      */
      leaf_2->is_join_columns_complete = true;
      first_outer_loop = false;
    }
    if (!found) continue;  // No matching field

    /*
      field_1 and field_2 have the same names. Check if they are in the USING
      clause (if present), mark them as common fields, and add a new
      equi-join condition to the ON clause.
    */
    if (nj_col_2 && (!using_fields || is_using_column_1)) {
      Item *item_1 = nj_col_1->create_item(thd);
      if (!item_1) return true;
      Item *item_2 = nj_col_2->create_item(thd);
      if (!item_2) return true;

      Field *field_1 = nj_col_1->field();
      Field *field_2 = nj_col_2->field();
      Item_ident *item_ident_1, *item_ident_2;
      Item_func_eq *eq_cond;
      fields.push_back(field_1);
      fields.push_back(field_2);

      /*
        The created items must be of sub-classes of Item_ident.
      */
      DBUG_ASSERT(item_1->type() == Item::FIELD_ITEM ||
                  item_1->type() == Item::REF_ITEM);
      DBUG_ASSERT(item_2->type() == Item::FIELD_ITEM ||
                  item_2->type() == Item::REF_ITEM);

      /*
        We need to cast item_1,2 to Item_ident, because we need to hook name
        resolution contexts specific to each item.
      */
      item_ident_1 = (Item_ident *)item_1;
      item_ident_2 = (Item_ident *)item_2;
      /*
        Create and hook special name resolution contexts to each item in the
        new join condition . We need this to both speed-up subsequent name
        resolution of these items, and to enable proper name resolution of
        the items during the execute phase of PS.
      */
      if (set_new_item_local_context(thd, item_ident_1, nj_col_1->table_ref) ||
          set_new_item_local_context(thd, item_ident_2, nj_col_2->table_ref))
        return true;

      if (!(eq_cond = new Item_func_eq(item_ident_1, item_ident_2)))
        return true;  // Out of memory.

      /*
        Add the new equi-join condition to the ON clause. Notice that
        fix_fields() is applied to all ON conditions in setup_conds()
        so we don't do it here.
       */
      add_join_on(table_ref_1->outer_join == JOIN_TYPE_RIGHT ? table_ref_1
                                                             : table_ref_2,
                  eq_cond);

      nj_col_1->is_common = nj_col_2->is_common = true;
      DBUG_PRINT("info", ("%s.%s and %s.%s are common",
                          nj_col_1->table_name() ? nj_col_1->table_name() : "",
                          nj_col_1->name(),
                          nj_col_2->table_name() ? nj_col_2->table_name() : "",
                          nj_col_2->name()));

      // Mark fields in the read set
      if (field_1) {
        nj_col_1->table_ref->table->mark_column_used(field_1,
                                                     MARK_COLUMNS_READ);
      } else {
        Mark_field mf(MARK_COLUMNS_READ);
        item_1->walk(&Item::mark_field_in_map, enum_walk::SUBQUERY_POSTFIX,
                     (uchar *)&mf);
      }

      if (field_2) {
        nj_col_2->table_ref->table->mark_column_used(field_2,
                                                     MARK_COLUMNS_READ);
      } else {
        Mark_field mf(MARK_COLUMNS_READ);
        item_2->walk(&Item::mark_field_in_map, enum_walk::SUBQUERY_POSTFIX,
                     (uchar *)&mf);
      }

      if (using_fields != nullptr) ++(*found_using_fields);
    }
  }

  if (leaf_1) leaf_1->is_join_columns_complete = true;

  /*
    Everything is OK.
    Notice that at this point there may be some column names in the USING
    clause that are not among the common columns. This is an SQL error and
    we check for this error in store_natural_using_join_columns() when
    (found_using_fields < length(join_using_fields)).
  */
  return false;
}

/*
  Materialize and store the row type of NATURAL/USING join.

  SYNOPSIS
    store_natural_using_join_columns()
    thd                current thread
    natural_using_join the table reference of the NATURAL/USING join
    table_ref_1        the first (left) operand (of a NATURAL/USING join).
    table_ref_2        the second (right) operand (of a NATURAL/USING join).
    using_fields       if the join is JOIN...USING - the join columns,
                       if NATURAL join, then NULL
    found_using_fields number of fields from the USING clause that were
                       found among the common fields

  DESCRIPTION
    Iterate over the columns of both join operands and sort and store
    all columns into the 'join_columns' list of natural_using_join
    where the list is formed by three parts:
      part1: The coalesced columns of table_ref_1 and table_ref_2,
             sorted according to the column order of the first table.
      part2: The other columns of the first table, in the order in
             which they were defined in CREATE TABLE.
      part3: The other columns of the second table, in the order in
             which they were defined in CREATE TABLE.
    Time complexity - O(N1+N2), where Ni = length(table_ref_i).

  IMPLEMENTATION
    The procedure assumes that mark_common_columns() has been called
    for the join that is being processed.

  RETURN
    true    error: Some common column is ambiguous
    false   OK
*/

static bool store_natural_using_join_columns(THD *thd,
                                             TABLE_LIST *natural_using_join,
                                             TABLE_LIST *table_ref_1,
                                             TABLE_LIST *table_ref_2,
                                             List<String> *using_fields,
                                             uint found_using_fields) {
  Field_iterator_table_ref it_1, it_2;
  Natural_join_column *nj_col_1, *nj_col_2;
  List<Natural_join_column> *non_join_columns;
  DBUG_TRACE;

  DBUG_ASSERT(!natural_using_join->join_columns);

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  if (!(non_join_columns = new (thd->mem_root) List<Natural_join_column>) ||
      !(natural_using_join->join_columns =
            new (thd->mem_root) List<Natural_join_column>))
    return true;

  /* Append the columns of the first join operand. */
  for (it_1.set(table_ref_1); !it_1.end_of_fields(); it_1.next()) {
    nj_col_1 = it_1.get_natural_column_ref();
    if (nj_col_1->is_common) {
      natural_using_join->join_columns->push_back(nj_col_1);
      /* Reset the common columns for the next call to mark_common_columns. */
      nj_col_1->is_common = false;
    } else
      non_join_columns->push_back(nj_col_1);
  }

  /*
    Check that all columns in the USING clause are among the common
    columns. If this is not the case, report the first one that was
    not found in an error.
  */
  if (using_fields && found_using_fields < using_fields->elements) {
    String *using_field_name;
    List_iterator_fast<String> using_fields_it(*using_fields);
    while ((using_field_name = using_fields_it++)) {
      const char *using_field_name_ptr = using_field_name->c_ptr();
      List_iterator_fast<Natural_join_column> it(
          *(natural_using_join->join_columns));
      Natural_join_column *common_field;

      for (;;) {
        /* If reached the end of fields, and none was found, report error. */
        if (!(common_field = it++)) {
          my_error(ER_BAD_FIELD_ERROR, MYF(0), using_field_name_ptr,
                   thd->where);
          return true;
        }
        if (!my_strcasecmp(system_charset_info, common_field->name(),
                           using_field_name_ptr))
          break;  // Found match
      }
    }
  }

  /* Append the non-equi-join columns of the second join operand. */
  for (it_2.set(table_ref_2); !it_2.end_of_fields(); it_2.next()) {
    nj_col_2 = it_2.get_natural_column_ref();
    if (!nj_col_2->is_common)
      non_join_columns->push_back(nj_col_2);
    else {
      /* Reset the common columns for the next call to mark_common_columns. */
      nj_col_2->is_common = false;
    }
  }

  if (non_join_columns->elements > 0)
    natural_using_join->join_columns->concat(non_join_columns);
  natural_using_join->is_join_columns_complete = true;

  return false;
}

/*
  Precompute and store the row types of the top-most NATURAL/USING joins.

  SYNOPSIS
    store_top_level_join_columns()
    thd            current thread
    table_ref      nested join or table in a FROM clause
    left_neighbor  neighbor table reference to the left of table_ref at the
                   same level in the join tree
    right_neighbor neighbor table reference to the right of table_ref at the
                   same level in the join tree

  DESCRIPTION
    The procedure performs a post-order traversal of a nested join tree
    and materializes the row types of NATURAL/USING joins in a
    bottom-up manner until it reaches the TABLE_LIST elements that
    represent the top-most NATURAL/USING joins. The procedure should be
    applied to each element of SELECT_LEX::top_join_list (i.e. to each
    top-level element of the FROM clause).

  IMPLEMENTATION
    Notice that the table references in the list nested_join->join_list
    are in reverse order, thus when we iterate over it, we are moving
    from the right to the left in the FROM clause.

  RETURN
    true   Error
    false  OK
*/

static bool store_top_level_join_columns(THD *thd, TABLE_LIST *table_ref,
                                         TABLE_LIST *left_neighbor,
                                         TABLE_LIST *right_neighbor) {
  DBUG_TRACE;

  DBUG_ASSERT(!table_ref->nested_join->natural_join_processed);

  Prepared_stmt_arena_holder ps_arena_holder(thd);

  /* Call the procedure recursively for each nested table reference. */
  if (table_ref->nested_join && !table_ref->nested_join->join_list.empty()) {
    auto nested_it = table_ref->nested_join->join_list.begin();
    TABLE_LIST *same_level_left_neighbor = *nested_it++;
    TABLE_LIST *same_level_right_neighbor = nullptr;
    /* Left/right-most neighbors, possibly at higher levels in the join tree. */
    TABLE_LIST *real_left_neighbor, *real_right_neighbor;

    while (same_level_left_neighbor) {
      TABLE_LIST *cur_table_ref = same_level_left_neighbor;
      same_level_left_neighbor =
          (nested_it == table_ref->nested_join->join_list.end()) ? nullptr
                                                                 : *nested_it++;
      /*
        The order of RIGHT JOIN operands is reversed in 'join list' to
        transform it into a LEFT JOIN. However, in this procedure we need
        the join operands in their lexical order, so below we reverse the
        join operands. Notice that this happens only in the first loop,
        and not in the second one, as in the second loop
        same_level_left_neighbor == NULL.
        This is the correct behavior, because the second loop sets
        cur_table_ref reference correctly after the join operands are
        swapped in the first loop.
      */
      if (same_level_left_neighbor &&
          cur_table_ref->outer_join == JOIN_TYPE_RIGHT) {
        /* This can happen only for JOIN ... ON. */
        DBUG_ASSERT(table_ref->nested_join->join_list.size() == 2);
        std::swap(same_level_left_neighbor, cur_table_ref);
      }

      /*
        Pick the parent's left and right neighbors if there are no immediate
        neighbors at the same level.
      */
      real_left_neighbor =
          (same_level_left_neighbor) ? same_level_left_neighbor : left_neighbor;
      real_right_neighbor = (same_level_right_neighbor)
                                ? same_level_right_neighbor
                                : right_neighbor;

      if (cur_table_ref->nested_join &&
          !cur_table_ref->nested_join->natural_join_processed &&
          store_top_level_join_columns(thd, cur_table_ref, real_left_neighbor,
                                       real_right_neighbor))
        return true;
      same_level_right_neighbor = cur_table_ref;
    }
  }

  /*
    If this is a NATURAL/USING join, materialize its result columns and
    convert to a JOIN ... ON.
  */
  if (table_ref->is_natural_join) {
    DBUG_ASSERT(table_ref->nested_join &&
                table_ref->nested_join->join_list.size() == 2);
    auto operand_it = table_ref->nested_join->join_list.begin();
    /*
      Notice that the order of join operands depends on whether table_ref
      represents a LEFT or a RIGHT join. In a RIGHT join, the operands are
      in inverted order.
     */
    TABLE_LIST *table_ref_2 = *operand_it++; /* Second NATURAL join operand.*/
    TABLE_LIST *table_ref_1 = *operand_it++; /* First NATURAL join operand. */
    List<String> *using_fields = table_ref->join_using_fields;
    uint found_using_fields;

    /*
      The two join operands were interchanged in the parser, change the order
      back for 'mark_common_columns'.
    */
    if (table_ref_2->outer_join == JOIN_TYPE_RIGHT)
      std::swap(table_ref_1, table_ref_2);
    if (mark_common_columns(thd, table_ref_1, table_ref_2, using_fields,
                            &found_using_fields))
      return true;

    /*
      Swap the join operands back, so that we pick the columns of the second
      one as the coalesced columns. In this way the coalesced columns are the
      same as of an equivalent LEFT JOIN.
    */
    if (table_ref_1->outer_join == JOIN_TYPE_RIGHT)
      std::swap(table_ref_1, table_ref_2);
    if (store_natural_using_join_columns(thd, table_ref, table_ref_1,
                                         table_ref_2, using_fields,
                                         found_using_fields))
      return true;

    /*
      Change NATURAL JOIN to JOIN ... ON. We do this for both operands
      because either one of them or the other is the one with the
      natural join flag because RIGHT joins are transformed into LEFT,
      and the two tables may be reordered.
    */
    table_ref_1->natural_join = table_ref_2->natural_join = nullptr;

    /* Add a true condition to outer joins that have no common columns. */
    if (table_ref_2->outer_join && !table_ref_2->join_cond())
      table_ref_2->set_join_cond(new Item_func_true());

    /* Change this table reference to become a leaf for name resolution. */
    if (left_neighbor) {
      TABLE_LIST *last_leaf_on_the_left;
      last_leaf_on_the_left = left_neighbor->last_leaf_for_name_resolution();
      last_leaf_on_the_left->next_name_resolution_table = table_ref;
    }
    if (right_neighbor) {
      TABLE_LIST *first_leaf_on_the_right;
      first_leaf_on_the_right =
          right_neighbor->first_leaf_for_name_resolution();
      table_ref->next_name_resolution_table = first_leaf_on_the_right;
    } else
      table_ref->next_name_resolution_table = nullptr;
  }

  table_ref->nested_join->natural_join_processed = true;

  return false;
}

/*
  Compute and store the row types of the top-most NATURAL/USING joins
  in a FROM clause.

  SYNOPSIS
    setup_natural_join_row_types()
    thd          current thread
    from_clause  list of top-level table references in a FROM clause

  DESCRIPTION
    Apply the procedure 'store_top_level_join_columns' to each of the
    top-level table referencs of the FROM clause. Adjust the list of tables
    for name resolution - context->first_name_resolution_table to the
    top-most, lef-most NATURAL/USING join.

  IMPLEMENTATION
    Notice that the table references in 'from_clause' are in reverse
    order, thus when we iterate over it, we are moving from the right
    to the left in the FROM clause.

  RETURN
    true   Error
    false  OK
*/
bool setup_natural_join_row_types(THD *thd,
                                  mem_root_deque<TABLE_LIST *> *from_clause,
                                  Name_resolution_context *context) {
  DBUG_TRACE;
  thd->where = "from clause";
  if (from_clause->empty())
    return false; /* We come here in the case of UNIONs. */

  auto table_ref_it = from_clause->begin();
  /* Table reference to the left of the current. */
  TABLE_LIST *left_neighbor = *table_ref_it++;
  /* Table reference to the right of the current. */
  TABLE_LIST *right_neighbor = nullptr;

  /* Note that tables in the list are in reversed order */
  while (left_neighbor) {
    /* Current table reference. */
    TABLE_LIST *table_ref = left_neighbor;
    left_neighbor =
        (table_ref_it == from_clause->end()) ? nullptr : *table_ref_it++;

    /*
      Do not redo work if already done:
      - for prepared statements and stored procedures,
      - if already processed inside a derived table/view.
    */
    if (table_ref->nested_join &&
        !table_ref->nested_join->natural_join_processed) {
      if (store_top_level_join_columns(thd, table_ref, left_neighbor,
                                       right_neighbor))
        return true;
    }
    if (left_neighbor && context->select_lex->first_execution) {
      left_neighbor->next_name_resolution_table =
          table_ref->first_leaf_for_name_resolution();
    }
    right_neighbor = table_ref;
  }

  /*
    Store the top-most, left-most NATURAL/USING join, so that we start
    the search from that one instead of context->table_list. At this point
    right_neighbor points to the left-most top-level table reference in the
    FROM clause.
  */
  DBUG_ASSERT(right_neighbor);
  context->first_name_resolution_table =
      right_neighbor->first_leaf_for_name_resolution();

  return false;
}

/**
  Resolve variable assignments from LEX object

  @param thd     Thread handler
  @param lex     Lex object containing variable assignments

  @returns false if success, true if error

  @note
  set_entry() must be called before fix_fields() of the whole list of
  field items because:

  1) the list of field items has same order as in the query, and the
     Item_func_get_user_var item may go before the Item_func_set_user_var:

     @verbatim
        SELECT @a, @a := 10 FROM t;
     @endverbatim

  2) The entry->update_query_id value controls constantness of
     Item_func_get_user_var items, so in presence of Item_func_set_user_var
     items we have to refresh their entries before fixing of
     Item_func_get_user_var items.
*/

bool resolve_var_assignments(THD *thd, LEX *lex) {
  List_iterator<Item_func_set_user_var> li(lex->set_var_list);
  Item_func_set_user_var *var;
  while ((var = li++)) var->set_entry(thd, false);

  return false;
}

/****************************************************************************
** Check that all given fields exists and fill struct with current data
****************************************************************************/

/**
  Resolve a list of expressions and setup appropriate data

  @param thd                    thread handler
  @param[out] ref_item_array    filled in with references to items.
  @param[in,out] fields         list of expressions, populated with resolved
                                data about expressions.
  @param want_privilege         privilege representing desired operation.
                                whether the expressions are selected, inserted
                                or updated, or no operation is done.
                                will also decide inclusion in read/write maps.
  @param sum_func_list
  @param allow_sum_func         true if set operations are allowed in context.
  @param column_update          if true, reject expressions that do not resolve
                                to a base table column

  @returns false if success, true if error

  @note The function checks updatability/insertability for the table before
        checking column privileges, for consistent error reporting.
        This has consequences for columns that are specified to be updated:
        The column is first resolved without privilege check.
        This check is followed by an updatablity/insertability check.
        Finally, a column privilege check is run, and the column is marked
        for update.
*/

bool setup_fields(THD *thd, Ref_item_array ref_item_array, List<Item> &fields,
                  ulong want_privilege, List<Item> *sum_func_list,
                  bool allow_sum_func, bool column_update) {
  DBUG_TRACE;

  SELECT_LEX *const select = thd->lex->current_select();
  const enum_mark_columns save_mark_used_columns = thd->mark_used_columns;
  nesting_map save_allow_sum_func = thd->lex->allow_sum_func;
  Column_privilege_tracker column_privilege(thd,
                                            column_update ? 0 : want_privilege);

  // Function can only be used to set up one specific operation:
  DBUG_ASSERT(want_privilege == 0 || want_privilege == SELECT_ACL ||
              want_privilege == INSERT_ACL || want_privilege == UPDATE_ACL);
  DBUG_ASSERT(!(column_update && (want_privilege & SELECT_ACL)));
  if (want_privilege & SELECT_ACL)
    thd->mark_used_columns = MARK_COLUMNS_READ;
  else if (want_privilege & (INSERT_ACL | UPDATE_ACL) && !column_update)
    thd->mark_used_columns = MARK_COLUMNS_WRITE;
  else
    thd->mark_used_columns = MARK_COLUMNS_NONE;

  DBUG_PRINT("info", ("thd->mark_used_columns: %d", thd->mark_used_columns));
  if (allow_sum_func)
    thd->lex->allow_sum_func |= (nesting_map)1 << select->nest_level;
  thd->where = THD::DEFAULT_WHERE;
  bool save_is_item_list_lookup = select->is_item_list_lookup;
  select->is_item_list_lookup = false;

  /*
    To prevent fail on forward lookup we fill it with zerows,
    then if we got pointer on zero after find_item_in_list we will know
    that it is forward lookup.

    There is other way to solve problem: fill array with pointers to list,
    but it will be slower.

    TODO: remove it when (if) we made one list for allfields and ref_item_array
  */
  if (!ref_item_array.is_null()) {
    DBUG_ASSERT(ref_item_array.size() >= fields.elements);
    memset(ref_item_array.array(), 0, sizeof(Item *) * fields.elements);
  }

  Ref_item_array ref = ref_item_array;

  Item *item;
  List_iterator<Item> it(fields);
  while ((item = it++)) {
    if ((!item->fixed && item->fix_fields(thd, it.ref())) ||
        (item = *(it.ref()))->check_cols(1)) {
      DBUG_PRINT("info",
                 ("thd->mark_used_columns: %d", thd->mark_used_columns));
      return true; /* purecov: inspected */
    }

    // Check that we don't have a field that is hidden from users. This should
    // be caught in Item_field::fix_fields.
    DBUG_ASSERT(
        item->type() != Item::FIELD_ITEM ||
        !static_cast<const Item_field *>(item)->field->is_hidden_from_user());

    if (!ref.is_null()) {
      ref[0] = item;
      ref.pop_front();
    }
    if (column_update) {
      Item_field *const field = item->field_for_view_update();
      if (field == nullptr) {
        my_error(ER_NONUPDATEABLE_COLUMN, MYF(0), item->item_name.ptr());
        return true;
      }
      TABLE_LIST *tr = field->table_ref;
      if ((want_privilege & UPDATE_ACL) && !tr->is_updatable()) {
        /*
          The base table of the column may have beeen referenced through a view
          or derived table. If so, print the name of the upper-most view
          referring to this table in order to print the error message with the
          alias of the view as written in the original query instead of the
          alias of the base table.
        */
        my_error(ER_NON_UPDATABLE_TABLE, MYF(0), tr->top_table()->alias,
                 "UPDATE");
        return true;
      }
      if ((want_privilege & INSERT_ACL) && !tr->is_insertable()) {
        /* purecov: begin inspected */
        /*
          Generally unused as long as INSERT only can be applied against
          one base table, for which the INSERT privileges are checked in
          Sql_cmd_insert_base::prepare_inner()
        */
        my_error(ER_NON_INSERTABLE_TABLE, MYF(0), tr->top_table()->alias,
                 "INSERT");
        return true;
        /* purecov: end */
      }
      if (want_privilege & (INSERT_ACL | UPDATE_ACL)) {
        Column_privilege_tracker column_privilege_tr(thd, want_privilege);
        if (item->walk(&Item::check_column_privileges, enum_walk::PREFIX,
                       pointer_cast<uchar *>(thd)))
          return true;
      }
      Mark_field mf(MARK_COLUMNS_WRITE);
      item->walk(&Item::mark_field_in_map, enum_walk::POSTFIX,
                 pointer_cast<uchar *>(&mf));
    }

    if (sum_func_list) {
      /*
        (1) Contains a grouped aggregate but is not one. If it is one, we do
        not split, but in create_tmp_table() we look at its arguments and add
        them to the tmp table, which achieves the same result as for window
        functions in (2) but differently.
        @todo: unify this (do like (2), probably).
        (2) Contains a window function. Even if it is a window function, we
        have to collect its arguments and add them to the hidden list of
        items, as those arguments have to be stored in the first tmp tables,
        and carried forward up to the tmp table where the WF can be
        evaluated.
      */
      if ((item->has_aggregation() && !(item->type() == Item::SUM_FUNC_ITEM &&
                                        !item->m_is_window_function)) ||  //(1)
          item->has_wf())                                                 // (2)
        item->split_sum_func(thd, ref_item_array, *sum_func_list);
    }

    select->select_list_tables |= item->used_tables();
  }
  select->is_item_list_lookup = save_is_item_list_lookup;
  thd->lex->allow_sum_func = save_allow_sum_func;
  thd->mark_used_columns = save_mark_used_columns;
  DBUG_PRINT("info", ("thd->mark_used_columns: %d", thd->mark_used_columns));

  DBUG_ASSERT(!thd->is_error());
  return false;
}

/*
  Drops in all fields instead of current '*' field

  SYNOPSIS
    insert_fields()
    thd			Thread handler
    context             Context for name resolution
    db_name		Database name in case of 'database_name.table_name.*'
    table_name		Table name in case of 'table_name.*'
    it			Pointer to '*'
    any_privileges	0 If we should ensure that we have SELECT privileges
                          for all columns
                        1 If any privilege is ok
  RETURN
    0	ok     'it' is updated to point at last inserted
    1	error.  Error message is generated but not sent to client
*/

bool insert_fields(THD *thd, Name_resolution_context *context,
                   const char *db_name, const char *table_name,
                   List_iterator<Item> *it, bool any_privileges) {
  char name_buff[NAME_LEN + 1];
  DBUG_TRACE;
  DBUG_PRINT("arena", ("stmt arena: %p", thd->stmt_arena));

  if (db_name &&
      (lower_case_table_names || is_infoschema_db(db_name, strlen(db_name)))) {
    /*
      convert database to lower case for comparison
      We can't do this in Item_field as this would change the
      'name' of the item which may be used in the select list

      We lowercase the 'information_schema' name below to treat it as
      case-insensitive even in lower_case_table_names=0.
    */
    strmake(name_buff, db_name, sizeof(name_buff) - 1);
    my_casedn_str(files_charset_info, name_buff);
    db_name = name_buff;
  }

  bool found = false;

  /*
    If table names are qualified, then loop over all tables used in the query,
    else treat natural joins as leaves and do not iterate over their underlying
    tables.
  */
  for (TABLE_LIST *tables = (table_name ? context->table_list
                                        : context->first_name_resolution_table);
       tables; tables = (table_name ? tables->next_local
                                    : tables->next_name_resolution_table)) {
    Field_iterator_table_ref field_iterator;
    TABLE *const table = tables->table;

    DBUG_ASSERT(tables->is_leaf_for_name_resolution());

    if ((table_name &&
         my_strcasecmp(table_alias_charset, table_name, tables->alias)) ||
        (db_name && strcmp(tables->db, db_name)))
      continue;

    /*
       Ensure that we have access rights to all fields to be inserted. Under
       some circumstances, this check may be skipped.

       - If any_privileges is true, skip the check.

       - If the SELECT privilege has been found as fulfilled already,
         the check is skipped.

       NOTE: This check is not sufficient: If a user has SELECT_ACL privileges
       for a view, it does not mean having the same privileges for the
       underlying tables/view. Thus, we have to perform individual column
       privilege checks below (or recurse down to all underlying tables here).
    */
    if (!any_privileges && !(tables->grant.privilege & SELECT_ACL)) {
      field_iterator.set(tables);
      if (check_grant_all_columns(thd, SELECT_ACL, &field_iterator))
        return true;
    }

    /*
      Update the tables used in the query based on the referenced fields. For
      views and natural joins this update is performed inside the loop below.
    */
    if (table) {
      thd->lex->current_select()->select_list_tables |= tables->map();
    }

    /*
      Initialize a generic field iterator for the current table reference.
      Notice that it is guaranteed that this iterator will iterate over the
      fields of a single table reference, because 'tables' is a leaf (for
      name resolution purposes).
    */
    field_iterator.set(tables);

    for (; !field_iterator.end_of_fields(); field_iterator.next()) {
      Item *const item = field_iterator.create_item(thd);
      if (!item) return true; /* purecov: inspected */
      DBUG_ASSERT(item->fixed);

      bool is_hidden_from_user = false;
      if (item->type() == Item::FIELD_ITEM) {
        Item_field *field = static_cast<Item_field *>(item);
        is_hidden_from_user = field->field->is_hidden_from_user();
        /* cache the table for the Item_fields inserted by expanding stars */
        if (tables->cacheable_table) field->cached_table = tables;
      }

      // If the column is hidden from users, do not add this column in place
      // of '*'.
      if (!is_hidden_from_user) {
        if (!found) {
          found = true;
          it->replace(item); /* Replace '*' with the first found item. */
        } else
          it->after(item); /* Add 'item' to the SELECT list. */
      }

      /*
        Set privilege information for the fields of newly created views.
        We have that (any_priviliges == true) if and only if we are creating
        a view. In the time of view creation we can't use the MERGE algorithm,
        therefore if 'tables' is itself a view, it is represented by a
        temporary table. Thus in this case we can be sure that 'item' is an
        Item_field.
      */
      if (any_privileges) {
        DBUG_ASSERT((tables->field_translation == nullptr && table) ||
                    tables->is_natural_join);
        DBUG_ASSERT(item->type() == Item::FIELD_ITEM);
        Item_field *const fld = (Item_field *)item;
        const char *field_table_name = field_iterator.get_table_name();
        if (!tables->schema_table &&
            !(fld->have_privileges =
                  (get_column_grant(thd, field_iterator.grant(),
                                    field_iterator.get_db_name(),
                                    field_table_name, fld->field_name) &
                   VIEW_ANY_ACL))) {
          my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), "ANY",
                   thd->security_context()->priv_user().str,
                   thd->security_context()->host_or_ip().str, field_table_name);
          return true;
        }
      }

      thd->lex->current_select()->select_list_tables |= item->used_tables();

      Field *const field = field_iterator.field();
      if (field) {
        // Register underlying fields in read map if wanted.
        field->table->mark_column_used(field, thd->mark_used_columns);
      } else {
        if (thd->want_privilege && tables->is_view_or_derived()) {
          if (item->walk(&Item::check_column_privileges, enum_walk::PREFIX,
                         (uchar *)thd))
            return true;
        }

        // Register underlying fields in read map if wanted.
        Mark_field mf(thd->mark_used_columns);
        item->walk(&Item::mark_field_in_map, enum_walk::SUBQUERY_POSTFIX,
                   (uchar *)&mf);
      }
    }
  }
  if (found) return false;

  /*
    TODO: in the case when we skipped all columns because there was a
    qualified '*', and all columns were coalesced, we have to give a more
    meaningful message than ER_BAD_TABLE_ERROR.
  */
  if (!table_name || !*table_name)
    my_error(ER_NO_TABLES_USED, MYF(0));
  else {
    String tbl_name;
    if (db_name) {
      tbl_name.append(String(db_name, system_charset_info));
      tbl_name.append('.');
    }
    tbl_name.append(String(table_name, system_charset_info));

    my_error(ER_BAD_TABLE_ERROR, MYF(0), tbl_name.c_ptr_safe());
  }

  return true;
}

/******************************************************************************
** Fill a record with data (for INSERT or UPDATE)
** Returns : 1 if some field has wrong type
******************************************************************************/

/**
  Fill fields with given items.

  @param thd                                  Thread handler.
  @param table                                Table reference.
  @param fields                               Item_fields list to be filled
  @param values                               Values to fill with.
  @param bitmap                               Bitmap over fields to fill.
  @param insert_into_fields_bitmap            Bitmap for fields that is set
                                              in fill_record.
  @param raise_autoinc_has_expl_non_null_val  Set corresponding flag in TABLE
                                              object to true if non-NULL value
                                              is explicitly assigned to
                                              auto-increment field.

  @note fill_record() may set TABLE::autoinc_field_has_explicit_non_null_value
        to true (even in case of failure!) and its caller should make sure that
        it is reset before next call to this function (i.e. before processing
        next row) and/or before TABLE instance is returned to table cache.
        One can use helper Auto_increment_field_not_null_reset_guard class
        to do this.

  @note In order to simplify implementation this call is allowed to reset
        TABLE::autoinc_field_has_explicit_non_null_value flag even in case
        when raise_autoinc_has_expl_non_null_val is false. However, this
        should be fine since this flag is supposed to be reset already in
        such cases.

  @return Operation status
    @retval false   OK
    @retval true    Error occurred
*/

bool fill_record(THD *thd, TABLE *table, List<Item> &fields, List<Item> &values,
                 MY_BITMAP *bitmap, MY_BITMAP *insert_into_fields_bitmap,
                 bool raise_autoinc_has_expl_non_null_val) {
  DBUG_TRACE;

  DBUG_ASSERT(fields.elements == values.elements);
  /*
    In case when TABLE object comes to fill_record() from Table Cache it
    should have autoinc_field_has_explicit_non_null_value flag set to false.
    In case when TABLE object comes to fill_record() after processing
    previous row this flag should be reset to false by caller.

    Code which implements LOAD DATA is the exception to the above rule
    as it calls fill_record() to handle SET clause, after values for
    the columns directly coming from loaded from file are set and thus
    autoinc_field_has_explicit_non_null_value possibly set to true.
  */
  DBUG_ASSERT(table->autoinc_field_has_explicit_non_null_value == false ||
              (raise_autoinc_has_expl_non_null_val &&
               thd->lex->sql_command == SQLCOM_LOAD));

  Item *fld;
  List_iterator_fast<Item> f(fields), v(values);
  while ((fld = f++)) {
    Item_field *const field = fld->field_for_view_update();
    DBUG_ASSERT(field != nullptr && field->table_ref->table == table);

    Field *const rfield = field->field;
    Item *const value = v++;

    /* If bitmap over wanted fields are set, skip non marked fields. */
    if (bitmap && !bitmap_is_set(bitmap, rfield->field_index)) continue;

    bitmap_set_bit(table->fields_set_during_insert, rfield->field_index);
    if (insert_into_fields_bitmap)
      bitmap_set_bit(insert_into_fields_bitmap, rfield->field_index);

    /* Generated columns will be filled after all base columns are done. */
    if (rfield->is_gcol()) continue;

    if (raise_autoinc_has_expl_non_null_val &&
        rfield == table->next_number_field)
      table->autoinc_field_has_explicit_non_null_value = true;
    /*
      We handle errors from save_in_field() by first checking the return
      value and then testing thd->is_error(). thd->is_error() can be set
      even when save_in_field() does not return a negative value.
      @todo save_in_field returns an enum which should never be a negative
      value. We should change this test to check for correct enum value.

      The below call can reset TABLE::autoinc_field_has_explicit_non_null_value
      flag depending on value provided (for details please see
      set_field_to_null_with_conversions()). So evaluation of this flag can't
      be moved outside of fill_record(), to be done once per statement.
    */
    if (value->save_in_field(rfield, false) < 0) {
      my_error(ER_UNKNOWN_ERROR, MYF(0));
      return true;
    }
    if (thd->is_error()) return true;
  }

  if (table->has_gcol() &&
      update_generated_write_fields(bitmap ? bitmap : table->write_set, table))
    return true;

  /*
    TABLE::autoinc_field_has_explicit_non_null_value should not be set to
    true in raise_autoinc_has_expl_non_null_val == false mode.
  */
  DBUG_ASSERT(table->autoinc_field_has_explicit_non_null_value == false ||
              raise_autoinc_has_expl_non_null_val);

  return thd->is_error();
}

/**
  Check the NOT NULL constraint on all the fields of the current record.

  @param thd            Thread context.
  @param fields         Collection of fields.

  @return Error status.
*/
static bool check_record(THD *thd, List<Item> &fields) {
  List_iterator_fast<Item> f(fields);
  Item *fld;
  Item_field *field;

  while ((fld = f++)) {
    field = fld->field_for_view_update();
    if (field &&
        field->field->check_constraints(ER_BAD_NULL_ERROR) != TYPE_OK) {
      my_error(ER_UNKNOWN_ERROR, MYF(0));
      return true;
    }
  }
  return thd->is_error();
}

/**
  Check the NOT NULL constraint on all the fields of the current record.

  @param thd  Thread context.
  @param ptr  Fields.

  @return Error status.
*/
bool check_record(THD *thd, Field **ptr) {
  Field *field;
  while ((field = *ptr++) && !thd->is_error()) {
    if (field->check_constraints(ER_BAD_NULL_ERROR) != TYPE_OK) return true;
  }
  return thd->is_error();
}

/**
  Check the NOT NULL constraint on all the fields explicitly set
  in INSERT INTO statement or implicitly set in BEFORE trigger.

  @param thd  Thread context.
  @param ptr  Fields.

  @return Error status.
*/

static bool check_inserting_record(THD *thd, Field **ptr) {
  Field *field;

  while ((field = *ptr++) && !thd->is_error()) {
    if (bitmap_is_set(field->table->fields_set_during_insert,
                      field->field_index) &&
        field->check_constraints(ER_BAD_NULL_ERROR) != TYPE_OK)
      return true;
  }

  return thd->is_error();
}

/**
  Invoke check constraints defined on the table.

  @param  thd                   Thread handle.
  @param  table                 Instance of TABLE.

  @retval  false  If all enforced check constraints are satisfied.
  @retval  true   Otherwise. THD::is_error() may be "true" in this case.
*/

bool invoke_table_check_constraints(THD *thd, const TABLE *table) {
  if (table->table_check_constraint_list != nullptr) {
    for (auto &table_cc : *table->table_check_constraint_list) {
      if (table_cc.is_enforced()) {
        /*
          Invoke check constraints only if column(s) used by check constraint is
          updated.
        */
        if ((thd->lex->sql_command == SQLCOM_UPDATE ||
             thd->lex->sql_command == SQLCOM_UPDATE_MULTI) &&
            !bitmap_is_overlapping(
                &table_cc.value_generator()->base_columns_map,
                table->write_set)) {
          DEBUG_SYNC(thd, "skip_check_constraints_on_unaffected_columns");
          continue;
        }

        // Validate check constraint.
        Item *check_const_expr_item = table_cc.value_generator()->expr_item;
        check_const_expr_item->m_in_check_constraint_exec_ctx = true;
        bool is_constraint_violated = (!check_const_expr_item->val_bool() &&
                                       !check_const_expr_item->null_value);
        check_const_expr_item->m_in_check_constraint_exec_ctx = false;

        /*
          If check constraint is violated then report an error. If expression
          operand types are incompatible and reported error in conversion even
          then report a more user friendly error. Sql_conditions of DA still has
          a conversion(actual reported) error in the error stack.
        */
        if (is_constraint_violated || thd->is_error()) {
          if (thd->is_error()) thd->clear_error();
          my_error(ER_CHECK_CONSTRAINT_VIOLATED, MYF(0), table_cc.name().str);
          return true;
        }
      }
    }
  }

  return false;
}

/**
  Check if SQL-statement is INSERT/INSERT SELECT/REPLACE/REPLACE SELECT
  and trigger event is ON INSERT. When this condition is true that means
  that the statement basically can invoke BEFORE INSERT trigger if it
  was created before.

  @param event         event type for triggers to be invoked
  @param sql_command   Type of SQL statement

  @return Test result
    @retval true    SQL-statement is
                    INSERT/INSERT SELECT/REPLACE/REPLACE SELECT
                    and trigger event is ON INSERT
    @retval false   Either SQL-statement is not
                    INSERT/INSERT SELECT/REPLACE/REPLACE SELECT
                    or trigger event is not ON INSERT
*/
static inline bool command_can_invoke_insert_triggers(
    enum enum_trigger_event_type event, enum_sql_command sql_command) {
  /*
    If it's 'INSERT INTO ... ON DUPLICATE KEY UPDATE ...' statement
    the event is TRG_EVENT_UPDATE and the SQL-command is SQLCOM_INSERT.
  */
  return event == TRG_EVENT_INSERT &&
         (sql_command == SQLCOM_INSERT || sql_command == SQLCOM_INSERT_SELECT ||
          sql_command == SQLCOM_REPLACE ||
          sql_command == SQLCOM_REPLACE_SELECT);
}

/**
  Execute BEFORE INSERT trigger.

  @param thd                        thread context
  @param table                      TABLE-object holding list of triggers
                                    to be invoked
  @param event                      event type for triggers to be invoked
  @param insert_into_fields_bitmap  Bitmap for fields that is set
                                    in fill_record

  @return Operation status
    @retval false   OK
    @retval true    Error occurred
*/
inline bool call_before_insert_triggers(THD *thd, TABLE *table,
                                        enum enum_trigger_event_type event,
                                        MY_BITMAP *insert_into_fields_bitmap) {
  for (Field **f = table->field; *f; ++f) {
    if (((*f)->flags & NO_DEFAULT_VALUE_FLAG) &&
        !bitmap_is_set(insert_into_fields_bitmap, (*f)->field_index)) {
      (*f)->set_tmp_null();
    }
  }

  return table->triggers->process_triggers(thd, event, TRG_ACTION_BEFORE, true);
}

/**
  Fill fields in list with values from the list of items and invoke
  before triggers.

  @param      thd                                 Thread context.
  @param      optype_info                         COPY_INFO structure used for
                                                  default values handling.
  @param      fields                              Item_fields list to be filled.
  @param      values                              Values to fill with.
  @param      table                               TABLE-object for the table.
  @param      event                               Event type for triggers to be
                                                  invoked.
  @param      num_fields                          Number of fields in table.
  @param      raise_autoinc_has_expl_non_null_val Set corresponding flag in
                                                  TABLE to true if non-NULL
                                                  value is explicitly assigned
                                                  to auto-increment field.
  @param[out] is_row_changed                      Set to true if a row is
                                                  changed after filling record
                                                  and invoking before triggers
                                                  for UPDATE operation.
                                                  Otherwise set to false.

  @note This function assumes that fields which values will be set and
        triggers to be invoked belong to the same table, and that
        TABLE::record[0] and record[1] buffers correspond to new and old
        versions of row respectively.

  @note This call may set TABLE::autoinc_field_has_explicit_non_null_value to
        true (even in case of failure!) and its caller should make sure that
        it is reset appropriately (@sa fill_record()).

  @return Operation status
    @retval false   OK
    @retval true    Error occurred
*/

bool fill_record_n_invoke_before_triggers(
    THD *thd, COPY_INFO *optype_info, List<Item> &fields, List<Item> &values,
    TABLE *table, enum enum_trigger_event_type event, int num_fields,
    bool raise_autoinc_has_expl_non_null_val, bool *is_row_changed) {
  // is_row_changed is used by UPDATE operation to set compare_record() result.
  DBUG_ASSERT(is_row_changed == nullptr ||
              optype_info->get_operation_type() == COPY_INFO::UPDATE_OPERATION);
  /*
    Fill DEFAULT functions (like CURRENT_TIMESTAMP) and DEFAULT expressions on
    the columns that are not on the list of assigned columns.
  */
  auto fill_function_defaults = [table, optype_info, is_row_changed]() {
    /*
      Unlike INSERT and LOAD, UPDATE operation requires comparison of old
      and new records to determine whether function defaults have to be
      evaluated.
    */
    if (optype_info->get_operation_type() == COPY_INFO::UPDATE_OPERATION) {
      *is_row_changed =
          (!records_are_comparable(table) || compare_records(table));
      /*
        Evaluate function defaults for columns with ON UPDATE clause only
        if any other column of the row is updated.
      */
      if (*is_row_changed &&
          (optype_info->function_defaults_apply_on_columns(table->write_set)))
        optype_info->set_function_defaults(table);
    } else if (optype_info->function_defaults_apply_on_columns(
                   table->write_set))
      optype_info->set_function_defaults(table);
  };

  /*
    If it's 'INSERT INTO ... ON DUPLICATE KEY UPDATE ...' statement
    the event is TRG_EVENT_UPDATE and the SQL-command is SQLCOM_INSERT.
  */

  if (table->triggers) {
    bool rc;

    table->triggers->enable_fields_temporary_nullability(thd);

    if (table->triggers->has_triggers(event, TRG_ACTION_BEFORE) &&
        command_can_invoke_insert_triggers(event, thd->lex->sql_command)) {
      DBUG_ASSERT(num_fields);

      MY_BITMAP insert_into_fields_bitmap;
      bitmap_init(&insert_into_fields_bitmap, nullptr, num_fields);

      fill_function_defaults();

      rc = fill_record(thd, table, fields, values, nullptr,
                       &insert_into_fields_bitmap,
                       raise_autoinc_has_expl_non_null_val);

      if (!rc)
        rc = call_before_insert_triggers(thd, table, event,
                                         &insert_into_fields_bitmap);

      bitmap_free(&insert_into_fields_bitmap);
    } else {
      rc = fill_record(thd, table, fields, values, nullptr, nullptr,
                       raise_autoinc_has_expl_non_null_val);

      if (!rc) {
        fill_function_defaults();
        rc = table->triggers->process_triggers(thd, event, TRG_ACTION_BEFORE,
                                               true);
        // For UPDATE operation, check if row is updated by the triggers.
        if (!rc &&
            optype_info->get_operation_type() == COPY_INFO::UPDATE_OPERATION &&
            !(*is_row_changed))
          *is_row_changed =
              (!records_are_comparable(table) || compare_records(table));
      }
    }
    /*
      Re-calculate generated fields to cater for cases when base columns are
      updated by the triggers.
    */
    DBUG_ASSERT(table->pos_in_table_list &&
                !table->pos_in_table_list->is_view());
    if (!rc && table->has_gcol())
      rc = update_generated_write_fields(table->write_set, table);

    table->triggers->disable_fields_temporary_nullability();

    return rc || check_inserting_record(thd, table->field);
  } else {
    if (fill_record(thd, table, fields, values, nullptr, nullptr,
                    raise_autoinc_has_expl_non_null_val))
      return true;
    fill_function_defaults();
    return check_record(thd, fields);
  }
}

/**
  Fill field buffer with values from Field list.

  @param thd                                  Thread handler.
  @param table                                Table reference.
  @param ptr                                  Array of fields to fill in.
  @param values                               List of values to fill with.
  @param bitmap                               Bitmap over fields to fill.
  @param insert_into_fields_bitmap            Bitmap for fields that is set
                                              in fill_record.
  @param raise_autoinc_has_expl_non_null_val  Set corresponding flag in TABLE
                                              object to true if non-NULL value
                                              is explicitly assigned to
                                              auto-increment field.

  @note fill_record() may set TABLE::autoinc_field_has_explicit_non_null_value
        to true (even in case of failure!) and its caller should make sure that
        it is reset before next call to this function (i.e. before processing
        next row) and/or before TABLE instance is returned to table cache.
        One can use helper Auto_increment_field_not_null_reset_guard class
        to do this.

  @note In order to simplify implementation this call is allowed to reset
        TABLE::autoinc_field_has_explicit_non_null_value flag even in case
        when raise_autoinc_has_expl_non_null_val is false. However, this
        should be fine since this flag is supposed to be reset already in
        such cases.

  @return Operation status
    @retval false   OK
    @retval true    Error occurred
*/

bool fill_record(THD *thd, TABLE *table, Field **ptr, List<Item> &values,
                 MY_BITMAP *bitmap, MY_BITMAP *insert_into_fields_bitmap,
                 bool raise_autoinc_has_expl_non_null_val) {
  DBUG_TRACE;

  /*
    In case when TABLE object comes to fill_record() from Table Cache it
    should have autoinc_field_has_explicit_non_null_value flag set to false.
    In case when TABLE object comes to fill_record() after processing
    previous row this flag should be reset to false by caller.
  */
  DBUG_ASSERT(table->autoinc_field_has_explicit_non_null_value == false);

  Field *field;
  List_iterator_fast<Item> v(values);
  while ((field = *ptr++) && !thd->is_error()) {
    // Skip fields invisible to the user
    if (field->is_hidden_from_user()) continue;

    Item *const value = v++;
    DBUG_ASSERT(field->table == table);

    /* If bitmap over wanted fields are set, skip non marked fields. */
    if (bitmap && !bitmap_is_set(bitmap, field->field_index)) continue;

    /*
      fill_record could be called as part of multi update and therefore
      table->fields_set_during_insert could be NULL.
    */
    if (table->fields_set_during_insert)
      bitmap_set_bit(table->fields_set_during_insert, field->field_index);
    if (insert_into_fields_bitmap)
      bitmap_set_bit(insert_into_fields_bitmap, field->field_index);

    /* Generated columns will be filled after all base columns are done. */
    if (field->is_gcol()) continue;

    if (raise_autoinc_has_expl_non_null_val &&
        field == table->next_number_field)
      table->autoinc_field_has_explicit_non_null_value = true;

    /*
      @todo We should evaluate what other return values from save_in_field()
      that should be treated as errors instead of checking thd->is_error().

      The below call can reset TABLE::autoinc_field_has_explicit_non_null_value
      flag depending on value provided (for details please see
      set_field_to_null_with_conversions()). So evaluation of this flag can't
      be moved outside of fill_record(), to be done once per statement.
    */
    if (value->save_in_field(field, false) ==
            TYPE_ERR_NULL_CONSTRAINT_VIOLATION ||
        thd->is_error())
      return true;
  }

  if (table->has_gcol() &&
      update_generated_write_fields(bitmap ? bitmap : table->write_set, table))
    return true;

  DBUG_ASSERT(thd->is_error() || !v++);  // No extra value!

  /*
    TABLE::autoinc_field_has_explicit_non_null_value should not be set to
    true in raise_autoinc_has_expl_non_null_val == false mode.
  */
  DBUG_ASSERT(table->autoinc_field_has_explicit_non_null_value == false ||
              raise_autoinc_has_expl_non_null_val);

  return thd->is_error();
}

/**
  Fill fields in array with values from the list of items and invoke
  before triggers.

  @param  thd         Thread context.
  @param  ptr         NULL-ended array of fields to be filled.
  @param  values      Values to fill with.
  @param  table       TABLE-object holding list of triggers to be invoked.
  @param  event       Event type for triggers to be invoked.
  @param  num_fields  Number of fields in table.

  @note This function assumes that fields which values will be set and triggers
        to be invoked belong to the same table, and that TABLE::record[0] and
        record[1] buffers correspond to new and old versions of row
        respectively.
  @note This function is called during handling of statements INSERT/
        INSERT SELECT/CREATE SELECT. It means that the only trigger's type
        that can be invoked when this function is called is a BEFORE INSERT
        trigger so we don't need to make branching based on the result of
        execution function command_can_invoke_insert_triggers().

  @note Unlike another version of fill_record_n_invoke_before_triggers() this
        call tries to set TABLE::autoinc_field_has_explicit_non_null_value to
        correct value unconditionally. So this flag can be set to true (even
        in case of failure!) and the caller should make sure that it is reset
        appropriately (@sa fill_record()).

  @retval false   OK
  @retval true    Error occurred.
*/

bool fill_record_n_invoke_before_triggers(THD *thd, Field **ptr,
                                          List<Item> &values, TABLE *table,
                                          enum enum_trigger_event_type event,
                                          int num_fields) {
  bool rc;

  if (table->triggers) {
    DBUG_ASSERT(
        command_can_invoke_insert_triggers(event, thd->lex->sql_command));
    DBUG_ASSERT(num_fields);

    table->triggers->enable_fields_temporary_nullability(thd);

    MY_BITMAP insert_into_fields_bitmap;
    bitmap_init(&insert_into_fields_bitmap, nullptr, num_fields);

    rc = fill_record(thd, table, ptr, values, nullptr,
                     &insert_into_fields_bitmap, true);

    if (!rc)
      rc = call_before_insert_triggers(thd, table, event,
                                       &insert_into_fields_bitmap);

    /*
      Re-calculate generated fields to cater for cases when base columns are
      updated by the triggers.
    */
    if (!rc && *ptr) {
      TABLE *table_p = (*ptr)->table;
      if (table_p->has_gcol())
        rc = update_generated_write_fields(table_p->write_set, table_p);
    }
    bitmap_free(&insert_into_fields_bitmap);
    table->triggers->disable_fields_temporary_nullability();
  } else
    rc = fill_record(thd, table, ptr, values, nullptr, nullptr, true);

  if (rc) return true;

  return check_inserting_record(thd, ptr);
}

/**
  Drop all temporary tables which have been left from previous server run.
  Used on server start-up.

  @return False on success, true on error.
*/

bool mysql_rm_tmp_tables(void) {
  uint i, idx;
  char filePath[FN_REFLEN], *tmpdir;
  MY_DIR *dirp;
  FILEINFO *file;
  THD *thd;
  List<LEX_STRING> files;
  List_iterator<LEX_STRING> files_it;
  MEM_ROOT files_root;
  LEX_STRING *file_str;
  bool result = true;
  DBUG_TRACE;

  if (!(thd = new THD)) return true; /* purecov: inspected */
  thd->thread_stack = (char *)&thd;
  thd->store_globals();

  init_alloc_root(PSI_NOT_INSTRUMENTED, &files_root, 32768, 0);

  for (i = 0; i <= mysql_tmpdir_list.max; i++) {
    tmpdir = mysql_tmpdir_list.list[i];
    /* See if the directory exists */
    if (!(dirp = my_dir(tmpdir, MYF(MY_WME | MY_DONT_SORT)))) continue;

    /* Find all SQLxxx files in the directory. */

    for (idx = 0; idx < dirp->number_off_files; idx++) {
      file = dirp->dir_entry + idx;

      /* skiping . and .. */
      if (file->name[0] == '.' &&
          (!file->name[1] || (file->name[1] == '.' && !file->name[2])))
        continue;

      if (strlen(file->name) > tmp_file_prefix_length &&
          !memcmp(file->name, tmp_file_prefix, tmp_file_prefix_length)) {
        size_t filePath_len = snprintf(filePath, sizeof(filePath), "%s%c%s",
                                       tmpdir, FN_LIBCHAR, file->name);
        file_str = make_lex_string_root(&files_root, filePath, filePath_len);

        if (file_str == nullptr || files.push_back(file_str, &files_root)) {
          /* purecov: begin inspected */
          my_dirend(dirp);
          goto err;
          /* purecov: end */
        }
      }
    }
    my_dirend(dirp);
  }

  /*
    Ask SEs to delete temporary tables.
    Pass list of SQLxxx files as a refence.
  */
  result = ha_rm_tmp_tables(thd, &files);

  /* Mimic old behavior, remove suspicious files if SE have not done this. */
  files_it.init(files);
  while ((file_str = files_it++))
    (void)mysql_file_delete(key_file_misc, file_str->str, MYF(0));

err:
  free_root(&files_root, MYF(0));
  delete thd;
  return result;
}

/*****************************************************************************
        unireg support functions
*****************************************************************************/

/*
  free all unused tables

  NOTE
    This is called by 'handle_manager' when one wants to periodicly flush
    all not used tables.
*/

void tdc_flush_unused_tables() {
  table_cache_manager.lock_all_and_tdc();

  if (flush_only_old_table_cache_entries) {
    DBUG_EXECUTE_IF("tdc_flush_unused_tables", {
      static constexpr char act[] = "now SIGNAL signal.enabled";
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);

    const time_point flush_cutpoint =
        std::chrono::system_clock::now() - std::chrono::seconds(flush_time);
    table_cache_manager.free_old_unused_tables(flush_cutpoint);

    /* Free table shares which were not freed implicitly by loop above. */
    for (TABLE_SHARE *s = oldest_unused_share; s->next; s = s->next) {
      if (should_be_evicted(s->last_accessed, flush_cutpoint)) {
        table_def_cache->erase(to_string(s->table_cache_key));
      }
    }
  } else {
    DBUG_EXECUTE_IF("tdc_flush_unused_tables", {
      static constexpr char act[] = "now SIGNAL signal.disabled";
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);

    table_cache_manager.free_all_unused_tables();
    /* Free table shares which were not freed implicitly by loop above. */
    while (oldest_unused_share->next)
      table_def_cache->erase(to_string(oldest_unused_share->table_cache_key));
  }
  table_cache_manager.unlock_all_and_tdc();
}

/**
   Remove all or some (depending on parameter) instances of TABLE and
   TABLE_SHARE from the table definition cache.

   @param  thd          Thread context
   @param  remove_type  Type of removal:
                        TDC_RT_REMOVE_ALL     - remove all TABLE instances and
                                                TABLE_SHARE instance. There
                                                should be no used TABLE objects
                                                and caller should have exclusive
                                                metadata lock on the table.
                        TDC_RT_REMOVE_NOT_OWN - remove all TABLE instances
                                                except those that belong to
                                                this thread. There should be
                                                no TABLE objects used by other
                                                threads and caller should have
                                                exclusive metadata lock on the
                                                table.
                        TDC_RT_REMOVE_UNUSED  - remove all unused TABLE
                                                instances (if there are no
                                                used instances will also
                                                remove TABLE_SHARE).
                        TDC_RT_REMOVE_NOT_OWN_KEEP_SHARE -
                                                remove all TABLE instances
                                                except those that belong to
                                                this thread, but don't mark
                                                TABLE_SHARE as old. There
                                                should be no TABLE objects
                                                used by other threads and
                                                caller should have exclusive
                                                metadata lock on the table.
   @param  db           Name of database
   @param  table_name   Name of table
   @param  has_lock     If true, LOCK_open is already acquired

   @note It assumes that table instances are already not used by any
   (other) thread (this should be achieved by using meta-data locks).
*/

void tdc_remove_table(THD *thd, enum_tdc_remove_table_type remove_type,
                      const char *db, const char *table_name, bool has_lock) {
  char key[MAX_DBKEY_LENGTH];
  size_t key_length;

  if (!has_lock)
    table_cache_manager.lock_all_and_tdc();
  else
    table_cache_manager.assert_owner_all_and_tdc();

  DBUG_ASSERT(remove_type == TDC_RT_REMOVE_UNUSED ||
              thd->mdl_context.owns_equal_or_stronger_lock(
                  MDL_key::TABLE, db, table_name, MDL_EXCLUSIVE));

  key_length = create_table_def_key(db, table_name, key);

  auto it = table_def_cache->find(string(key, key_length));

  // If the table has a shadow copy in a secondary storage engine, or
  // if we don't know if the table has a shadow copy, we must also
  // attempt to evict the secondary table from the cache.
  const bool remove_secondary =
      it == table_def_cache->end() || it->second->has_secondary_engine();

  // Helper function that evicts the TABLE_SHARE pointed to by an iterator.
  auto remove_table = [&](Table_definition_cache::iterator my_it) {
    if (my_it == table_def_cache->end()) return;
    TABLE_SHARE *share = my_it->second.get();
    /*
      Since share->ref_count is incremented when a table share is opened
      in get_table_share(), before LOCK_open is temporarily released, it
      is sufficient to check this condition alone and ignore the
      share->m_open_in_progress flag.

      Note that it is safe to call table_cache_manager.free_table() for
      shares with m_open_in_progress == true, since such shares don't
      have any TABLE objects associated.
    */
    if (share->ref_count() > 0) {
      /*
        Set share's version to zero in order to ensure that it gets
        automatically deleted once it is no longer referenced.

        Note that code in TABLE_SHARE::wait_for_old_version() assumes
        that marking share as old and removal of its unused tables
        and of the share itself from TDC happens atomically under
        protection of LOCK_open, or, putting it another way, that
        TDC does not contain old shares which don't have any tables
        used.
      */
      if (remove_type != TDC_RT_REMOVE_NOT_OWN_KEEP_SHARE)
        share->clear_version();
      table_cache_manager.free_table(thd, remove_type, share);
    } else {
      // There are no TABLE objects associated, so just remove the
      // share immediately. (Assert: When called with
      // TDC_RT_REMOVE_NOT_OWN_KEEP_SHARE, there should always be a
      // TABLE object associated with the primary TABLE_SHARE.)
      DBUG_ASSERT(remove_type != TDC_RT_REMOVE_NOT_OWN_KEEP_SHARE ||
                  share->is_secondary_engine());
      table_def_cache->erase(to_string(share->table_cache_key));
    }
  };

  remove_table(it);

  if (remove_secondary)
    remove_table(
        table_def_cache->find(create_table_def_key_secondary(db, table_name)));

  if (!has_lock) table_cache_manager.unlock_all_and_tdc();
}

int setup_ftfuncs(const THD *thd, SELECT_LEX *select_lex) {
  DBUG_ASSERT(select_lex->has_ft_funcs());

  List_iterator<Item_func_match> li(*(select_lex->ftfunc_list)),
      lj(*(select_lex->ftfunc_list));
  Item_func_match *ftf, *ftf2;

  while ((ftf = li++)) {
    if (ftf->table_ref && ftf->fix_index(thd)) return 1;
    lj.rewind();

    /*
      Notice that expressions added late (e.g. in ORDER BY) may be deleted
      during resolving. It is therefore important that an "early" expression
      is used as master for a "late" one, and not the other way around.
    */
    while ((ftf2 = lj++) != ftf) {
      if (ftf->eq(ftf2, true) && !ftf->master) ftf2->set_master(ftf);
    }
  }

  return 0;
}

bool init_ftfuncs(THD *thd, SELECT_LEX *select_lex) {
  DBUG_ASSERT(select_lex->has_ft_funcs());

  List_iterator<Item_func_match> li(*(select_lex->ftfunc_list));
  DBUG_PRINT("info", ("Performing FULLTEXT search"));
  THD_STAGE_INFO(thd, stage_fulltext_initialization);

  Item_func_match *ifm;
  while ((ifm = li++)) {
    if (ifm->init_search(thd)) return true;
  }
  return false;
}

/**
  Open and lock transactional system tables for read.

  One must call close_trans_system_tables() to close systems tables opened
  with this call.

  @param thd        Thread context.
  @param table_list List of tables to open.

  @note THR_LOCK deadlocks are not possible here because of the
  restrictions we put on opening and locking of system tables for writing.
  Thus, the system tables can be opened and locked for reading even if some
  other tables have already been opened and locked.

  @note MDL-deadlocks are possible, but they are properly detected and
  reported.

  @note Row-level deadlocks should be either avoided altogether using
  non-locking reads (as it is done now for InnoDB), or should be correctly
  detected and reported (in case of other transactional SE).

  @note It is now technically possible to open non-transactional tables
  (MyISAM system tables) using this function. That situation might still happen
  if the user run the server on the elder data-directory or manually alters the
  system tables to reside in MyISAM instead of InnoDB. It will be forbidden in
  the future.

  @return Error status.
*/

bool open_trans_system_tables_for_read(THD *thd, TABLE_LIST *table_list) {
  uint counter;
  uint flags = MYSQL_OPEN_IGNORE_FLUSH | MYSQL_LOCK_IGNORE_TIMEOUT;

  DBUG_TRACE;

  DBUG_ASSERT(!thd->is_attachable_ro_transaction_active());

  // Begin attachable transaction.

  thd->begin_attachable_ro_transaction();

  // Open tables.

  if (open_tables(thd, &table_list, &counter, flags)) {
    thd->end_attachable_transaction();
    return true;
  }

  // Check the tables.

  for (TABLE_LIST *t = table_list; t; t = t->next_global) {
    // Ensure the t are in storage engines, which are compatible with the
    // attachable transaction requirements.

    if ((t->table->file->ha_table_flags() & HA_ATTACHABLE_TRX_COMPATIBLE) ==
        0) {
      // Crash in the debug build ...
      DBUG_ASSERT(!"HA_ATTACHABLE_TRX_COMPATIBLE is not set");

      // ... or report an error in the release build.
      my_error(ER_UNKNOWN_ERROR, MYF(0));
      thd->end_attachable_transaction();
      return true;
    }

    // The table should be in a transaction SE. This is not strict requirement
    // however. It will be make more strict in the future.

    if (!t->table->file->has_transactions())
      LogErr(WARNING_LEVEL, ER_SYSTEM_TABLE_NOT_TRANSACTIONAL,
             static_cast<int>(t->table_name_length), t->table_name);
  }

  // Lock the tables.

  if (lock_tables(thd, table_list, counter, flags)) {
    thd->end_attachable_transaction();
    return true;
  }

  // Mark the table columns for use.

  for (TABLE_LIST *tables = table_list; tables; tables = tables->next_global)
    tables->table->use_all_columns();

  return false;
}

/**
  Close transactional system tables, opened with
  open_trans_system_tables_for_read().

  @param thd        Thread context.
*/

void close_trans_system_tables(THD *thd) { thd->end_attachable_transaction(); }

/**
  A helper function to close a mysql.* table opened
  in an auxiliary THD during bootstrap or in the main
  connection, when we know that there are no locks
  held by the connection due to a preceding implicit
  commit.

  This function assumes that there is no
  statement transaction started for the operation
  itself, since mysql.* tables are not transactional
  and when they are used the binlog is off (DDL
  binlogging is always statement-based.

  We need this function since we'd like to not
  just close the system table, but also release
  the metadata lock on it.

  Note, that in LOCK TABLES mode this function
  does not release the metadata lock. But in this
  mode the table can be opened only if it is locked
  explicitly with LOCK TABLES.
*/

void close_mysql_tables(THD *thd) {
  /* No need to commit/rollback statement transaction, it's not started. */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT));
  close_thread_tables(thd);
  thd->mdl_context.release_transactional_locks();
}

/**
  Open a log table.
  Opening such tables is performed internally in the server
  implementation, and is a 'nested' open, since some tables
  might be already opened by the current thread.
  The thread context before this call is saved, and is restored
  when calling close_log_table().
  @param thd The current thread
  @param one_table Log table to open
  @param [out] backup Temporary storage used to save the thread context
*/
TABLE *open_log_table(THD *thd, TABLE_LIST *one_table,
                      Open_tables_backup *backup) {
  uint flags = (MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK |
                MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY | MYSQL_OPEN_IGNORE_FLUSH |
                MYSQL_LOCK_IGNORE_TIMEOUT | MYSQL_LOCK_LOG_TABLE);
  TABLE *table;
  /* Save value that is changed in mysql_lock_tables() */
  ulonglong save_utime_after_lock = thd->utime_after_lock;
  DBUG_TRACE;

  thd->reset_n_backup_open_tables_state(backup,
                                        Open_tables_state::SYSTEM_TABLES);

  if ((table = open_ltable(thd, one_table, one_table->lock_descriptor().type,
                           flags))) {
    DBUG_ASSERT(table->s->table_category == TABLE_CATEGORY_LOG);
    /* Make sure all columns get assigned to a default value */
    table->use_all_columns();
    DBUG_ASSERT(table->no_replicate);
  } else
    thd->restore_backup_open_tables_state(backup);

  thd->utime_after_lock = save_utime_after_lock;
  return table;
}

/**
  Close a log table.
  The last table opened by open_log_table()
  is closed, then the thread context is restored.
  @param thd The current thread
  @param backup The context to restore.
*/
void close_log_table(THD *thd, Open_tables_backup *backup) {
  Query_tables_list query_tables_list_backup;

  /*
    In order not affect execution of current statement we have to
    backup/reset/restore Query_tables_list part of LEX, which is
    accessed and updated in the process of closing tables.
  */
  thd->lex->reset_n_backup_query_tables_list(&query_tables_list_backup);
  close_thread_tables(thd);
  thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);
  thd->restore_backup_open_tables_state(backup);
}

/**
  @} (end of group Data_Dictionary)
*/
