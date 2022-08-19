/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* This implements 'user defined functions' */

#include "sql/sql_udf.h"

#include "my_config.h"

#include <stdio.h>
#include <string.h>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"  // my_stpcpy
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"  // ER_*
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"   // opt_allow_suspicious_udfs
#include "sql/records.h"  // unique_ptr_destroy_only<RowIterator>
#include "sql/row_iterator.h"
#include "sql/sql_base.h"   // close_mysql_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_parse.h"   // check_string_char_length
#include "sql/sql_plugin.h"  // check_valid_path
#include "sql/sql_table.h"   // write_bin_log
#include "sql/table.h"       // TABLE_LIST
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_*
#include "thr_lock.h"
#include "udf_registration_imp.h"

#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

/**
  @page page_ext_udf User Defined Functions

  @todo Document me

  @sa add_udf, udf_hash_delete.
*/

/**
  A local flag indicating whether SQL based UDF operations are allowed.
  Now the UDF structures are always allocated/deallocated due to
  the component service.

  So this variable does not cover initialization/deinitialization of these.
  \ref mem and \ref THR_LOCK_udf are always initialized, even in
  --skip-grant-tables mode.
*/
static bool initialized = false;
static MEM_ROOT mem;
static collation_unordered_map<std::string, udf_func *> *udf_hash;
static mysql_rwlock_t THR_LOCK_udf;

static udf_func *add_udf(LEX_STRING *name, Item_result ret, char *dl,
                         Item_udftype typ);
static void udf_hash_delete(udf_func *udf);
static void *find_udf_dl(const char *dl);

static char *init_syms(udf_func *tmp, char *nm) {
  char *end;

  if (!((tmp->func = (Udf_func_any)dlsym(tmp->dlhandle, tmp->name.str))))
    return tmp->name.str;

  end = my_stpcpy(nm, tmp->name.str);

  if (tmp->type == UDFTYPE_AGGREGATE) {
    (void)my_stpcpy(end, "_clear");
    if (!((tmp->func_clear = (Udf_func_clear)dlsym(tmp->dlhandle, nm))))
      return nm;
    (void)my_stpcpy(end, "_add");
    if (!((tmp->func_add = (Udf_func_add)dlsym(tmp->dlhandle, nm)))) return nm;
  }

  (void)my_stpcpy(end, "_deinit");
  tmp->func_deinit = (Udf_func_deinit)dlsym(tmp->dlhandle, nm);

  (void)my_stpcpy(end, "_init");
  tmp->func_init = (Udf_func_init)dlsym(tmp->dlhandle, nm);

  /*
    to prevent loading "udf" from, e.g. libc.so
    let's ensure that at least one auxiliary symbol is defined
  */
  if (!tmp->func_init && !tmp->func_deinit && tmp->type != UDFTYPE_AGGREGATE) {
    if (!opt_allow_suspicious_udfs) return nm;
    LogErr(WARNING_LEVEL, ER_FAILED_TO_FIND_DL_ENTRY, nm);
  }
  return nullptr;
}

static PSI_memory_key key_memory_udf_mem;

#ifdef HAVE_PSI_INTERFACE
static PSI_rwlock_key key_rwlock_THR_LOCK_udf;

static PSI_rwlock_info all_udf_rwlocks[] = {{&key_rwlock_THR_LOCK_udf,
                                             "THR_LOCK_udf", PSI_FLAG_SINGLETON,
                                             0, PSI_DOCUMENT_ME}};

static PSI_memory_info all_udf_memory[] = {{&key_memory_udf_mem, "udf_mem",
                                            PSI_FLAG_ONLY_GLOBAL_STAT, 0,
                                            PSI_DOCUMENT_ME}};

static void init_udf_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_udf_rwlocks));
  mysql_rwlock_register(category, all_udf_rwlocks, count);

  count = static_cast<int>(array_elements(all_udf_memory));
  mysql_memory_register(category, all_udf_memory, count);
}
#endif

/**
  Initialize the UDF global structures.
  This is done as a separate step so that the UDF registration
  service can work when initalizing plugins, which happens
  before reading the UDF table.
*/
void udf_init_globals() {
  DBUG_TRACE;
  if (initialized) return;

#ifdef HAVE_PSI_INTERFACE
  init_udf_psi_keys();
#endif

  mysql_rwlock_init(key_rwlock_THR_LOCK_udf, &THR_LOCK_udf);
  init_sql_alloc(key_memory_udf_mem, &mem, UDF_ALLOC_BLOCK_SIZE, 0);

  udf_hash = new collation_unordered_map<std::string, udf_func *>(
      system_charset_info, key_memory_udf_mem);
}

/*
  Read all predeclared functions from mysql.func and accept all that
  can be used.
  The global structures must be initialized first.
*/
void udf_read_functions_table() {
  udf_func *tmp;
  TABLE *table;
  unique_ptr_destroy_only<RowIterator> iterator;
  int error;
  DBUG_TRACE;
  char db[] = "mysql"; /* A subject to casednstr, can't be constant */

  if (initialized) {
    DBUG_ASSERT("wrong init order: reading UDFs from the table twice");
    return;
  }

  initialized = true;

  THD *new_thd = new (std::nothrow) THD;
  if (new_thd == nullptr) {
    LogErr(ERROR_LEVEL, ER_UDF_CANT_ALLOC_FOR_STRUCTURES);
    free_root(&mem, MYF(0));
    delete new_thd;
    return;
  }
  new_thd->thread_stack = (char *)&new_thd;
  new_thd->store_globals();
  {
    LEX_CSTRING db_lex_cstr = {STRING_WITH_LEN(db)};
    new_thd->set_db(db_lex_cstr);
  }

  TABLE_LIST tables(db, "func", TL_READ, MDL_SHARED_READ_ONLY);

  if (open_trans_system_tables_for_read(new_thd, &tables)) {
    DBUG_PRINT("error", ("Can't open udf table"));
    LogErr(ERROR_LEVEL, ER_UDF_CANT_OPEN_FUNCTION_TABLE);
    goto end;
  }

  table = tables.table;
  iterator = init_table_iterator(new_thd, table, nullptr, false,
                                 /*ignore_not_found_rows=*/false);
  if (iterator == nullptr) goto end;
  while (!(error = iterator->Read())) {
    DBUG_PRINT("info", ("init udf record"));
    LEX_STRING name;
    name.str = get_field(&mem, table->field[0]);
    name.length = strlen(name.str);
    char *dl_name = get_field(&mem, table->field[2]);
    bool new_dl = false;
    Item_udftype udftype = UDFTYPE_FUNCTION;
    if (table->s->fields >= 4)  // New func table
      udftype = (Item_udftype)table->field[3]->val_int();

    /*
      Ensure that the .dll doesn't have a path
      This is done to ensure that only approved dll from the system
      directories are used (to make this even remotely secure).

      On windows we must check both FN_LIBCHAR and '/'.
    */

    LEX_CSTRING name_cstr = {name.str, name.length};
    if (check_valid_path(dl_name, strlen(dl_name)) ||
        check_string_char_length(name_cstr, "", NAME_CHAR_LEN,
                                 system_charset_info, true)) {
      LogErr(ERROR_LEVEL, ER_UDF_INVALID_ROW_IN_FUNCTION_TABLE, name.str);
      continue;
    }

    if (!(tmp = add_udf(&name, (Item_result)table->field[1]->val_int(), dl_name,
                        udftype))) {
      LogErr(ERROR_LEVEL, ER_UDF_CANT_ALLOC_FOR_FUNCTION, name.str);
      continue;
    }

    void *dl = find_udf_dl(tmp->dl);
    if (dl == nullptr) {
      char dlpath[FN_REFLEN];
      strxnmov(dlpath, sizeof(dlpath) - 1, opt_plugin_dir, "/", tmp->dl, NullS);
      (void)unpack_filename(dlpath, dlpath);
      if (!(dl = dlopen(dlpath, RTLD_NOW))) {
        const char *errmsg;
        int error_number = dlopen_errno;
        DLERROR_GENERATE(errmsg, error_number);

        // Print warning to log
        LogErr(ERROR_LEVEL, ER_FAILED_TO_OPEN_SHARED_LIBRARY, tmp->dl,
               error_number, errmsg);
        // Keep the udf in the hash so that we can remove it later
        continue;
      }
      new_dl = true;
    }
    tmp->dlhandle = dl;
    {
      char buf[NAME_LEN + 16], *missing;
      if ((missing = init_syms(tmp, buf))) {
        LogErr(ERROR_LEVEL, ER_FAILED_TO_FIND_DL_ENTRY, missing);
        udf_hash_delete(tmp);
        if (new_dl) dlclose(dl);
      }
    }
  }
  if (error > 0) LogErr(ERROR_LEVEL, ER_UNKNOWN_ERROR_NUMBER, my_errno());
  iterator.reset();
  table->m_needs_reopen = true;  // Force close to free memory

end:
  close_trans_system_tables(new_thd);
  delete new_thd;
}

/**
   Deintialize the UDF subsystem.

   This function closes the shared libaries.
*/
void udf_unload_udfs() {
  DBUG_TRACE;
  if (udf_hash != nullptr) {
    for (auto it1 = udf_hash->begin(); it1 != udf_hash->end(); ++it1) {
      udf_func *udf = it1->second;
      if (udf->dlhandle)  // Not closed before
      {
        /* Mark all versions using the same handler as closed */
        for (auto it2 = std::next(it1); it2 != udf_hash->end(); ++it2) {
          udf_func *tmp = it2->second;
          if (udf->dlhandle == tmp->dlhandle)
            tmp->dlhandle = nullptr;  // Already closed
        }
        dlclose(udf->dlhandle);
      }
    }
  }
}

/**
   Deintialize the UDF subsystem.

   This function does the following:
   1. Free the UDF hash.
   2. Free the memroot allocated.
   3. Destroy the RW mutex object.
*/
void udf_deinit_globals() {
  DBUG_TRACE;
  if (udf_hash != nullptr) {
    delete udf_hash;
    udf_hash = nullptr;
  }
  free_root(&mem, MYF(0));
  initialized = false;

  mysql_rwlock_destroy(&THR_LOCK_udf);
}

/**
   Delete the UDF function from the UDF hash.

   @param udf  Pointer to the UDF function.

   @note The function remove the udf function from the udf
         hash if it is not in use. If the function is in use,
         the function name is renamed so that it is not used.
         The function shall be removed when no threads use it.
*/
static void udf_hash_delete(udf_func *udf) {
  DBUG_TRACE;

  mysql_rwlock_wrlock(&THR_LOCK_udf);

  const auto it = udf_hash->find(to_string(udf->name));
  if (it == udf_hash->end()) {
    DBUG_ASSERT(false);
    return;
  }

  if (!--udf->usage_count) {
    udf_hash->erase(it);
    using_udf_functions = !udf_hash->empty();
  } else {
    /*
      The functions is in use ; Rename the functions instead of removing it.
      The functions will be automaticly removed when the least threads
      doesn't use it anymore
    */
    udf_hash->erase(it);
    char new_name[32];
    snprintf(new_name, sizeof(new_name), "*<%p>", udf);
    udf_hash->emplace(new_name, udf);
  }
  mysql_rwlock_unlock(&THR_LOCK_udf);
}

void free_udf(udf_func *udf) {
  DBUG_TRACE;

  if (!initialized) return;

  mysql_rwlock_wrlock(&THR_LOCK_udf);
  if (!--udf->usage_count) {
    /*
      We come here when someone has deleted the udf function
      while another thread still was using the udf
    */
    const auto it = udf_hash->find(to_string(udf->name));
    if (it == udf_hash->end()) {
      DBUG_ASSERT(false);
      return;
    }
    udf_hash->erase(it);
    using_udf_functions = !udf_hash->empty();
    if (udf->dlhandle && !find_udf_dl(udf->dl)) dlclose(udf->dlhandle);
  }
  mysql_rwlock_unlock(&THR_LOCK_udf);
}

/* This is only called if using_udf_functions != 0 */

udf_func *find_udf(const char *name, size_t length, bool mark_used) {
  udf_func *udf = nullptr;
  DBUG_TRACE;

  if (!initialized) return nullptr;

  /* TODO: This should be changed to reader locks someday! */
  if (mark_used)
    mysql_rwlock_wrlock(&THR_LOCK_udf); /* Called during fix_fields */
  else
    mysql_rwlock_rdlock(&THR_LOCK_udf); /* Called during parsing */

  std::string key = length ? std::string(name, length) : std::string(name);
  const auto it = udf_hash->find(key);

  if (it != udf_hash->end()) {
    udf = it->second;
    if (mark_used) udf->usage_count++;
  }
  mysql_rwlock_unlock(&THR_LOCK_udf);
  return udf;
}

static void *find_udf_dl(const char *dl) {
  DBUG_TRACE;

  if (!dl) return nullptr;
  /*
    Because only the function name is hashed, we have to search trough
    all rows to find the dl.
  */
  for (const auto &key_and_value : *udf_hash) {
    udf_func *udf = key_and_value.second;
    if (udf->dl && !strcmp(dl, udf->dl) && udf->dlhandle != nullptr)
      return udf->dlhandle;
  }
  return nullptr;
}

/* Assume that name && dl is already allocated */

static udf_func *add_udf(LEX_STRING *name, Item_result ret, char *dl,
                         Item_udftype type) {
  if (!name || !dl || !(uint)type || (uint)type > (uint)UDFTYPE_AGGREGATE)
    return nullptr;

  udf_func *tmp = (udf_func *)mem.Alloc(sizeof(udf_func));
  if (!tmp) return nullptr;
  memset(tmp, 0, sizeof(*tmp));
  tmp->name = *name;  // dup !!
  tmp->dl = dl;
  tmp->returns = ret;
  tmp->type = type;
  tmp->usage_count = 1;

  mysql_rwlock_wrlock(&THR_LOCK_udf);

  udf_hash->emplace(to_string(tmp->name), tmp);
  using_udf_functions = true;

  mysql_rwlock_unlock(&THR_LOCK_udf);
  return tmp;
}

/**
   Commit or rollback a transaction. Also close tables
   which it has opened and release metadata locks.
   Add/Remove from the in-memory hash depending on transaction
   commit or rollback and the bool flag passed to this function.

   @param thd                 THD context.
   @param rollback            Rollback transaction if true.
   @param udf                 Pointer to UDF function.
   @param insert_udf          Insert UDF in hash if true.

   @retval False - Success.
   @retval True  - Error.
*/

static bool udf_end_transaction(THD *thd, bool rollback, udf_func *udf,
                                bool insert_udf) {
  bool result;
  bool rollback_transaction = thd->transaction_rollback_request || rollback;
  udf_func *u_f = nullptr;

  DBUG_ASSERT(stmt_causes_implicit_commit(thd, CF_IMPLICIT_COMMIT_END));

  if (!rollback_transaction && insert_udf) {
    udf->name.str = strdup_root(&mem, udf->name.str);
    udf->dl = strdup_root(&mem, udf->dl);
    // create entry in mysql.func table
    u_f = add_udf(&udf->name, udf->returns, udf->dl, udf->type);
    if (u_f != nullptr) {
      u_f->dlhandle = udf->dlhandle;
      u_f->func = udf->func;
      u_f->func_init = udf->func_init;
      u_f->func_deinit = udf->func_deinit;
      u_f->func_clear = udf->func_clear;
      u_f->func_add = udf->func_add;
    }
  }

  rollback_transaction = rollback_transaction || (insert_udf && u_f == nullptr);

  /*
    CREATE/DROP UDF operations must acquire IX Backup Lock in order
    to be mutually exclusive with LOCK INSTANCE FOR BACKUP.
  */
  DBUG_ASSERT(thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::BACKUP_LOCK, "", "", MDL_INTENTION_EXCLUSIVE));

  /*
    Rollback the transaction if there is an error or there is a request by the
    SE (which is unlikely).
  */
  if (rollback_transaction) {
    result = trans_rollback_stmt(thd);
    result = result || trans_rollback_implicit(thd);
  } else {
    result = trans_commit_stmt(thd);
    result = result || trans_commit_implicit(thd);
  }

  /*
    Delete UDF from the hash if
      * the transaction commit fails for CREATE UDF operation
      * OR if the transaction is committed successfully for the DROP UDF
        operation.
  */
  if (!rollback_transaction &&
      ((insert_udf && result) || (!insert_udf && !result)))
    udf_hash_delete(udf);

  close_thread_tables(thd);
  thd->mdl_context.release_transactional_locks();

  return result || rollback || (insert_udf && u_f == nullptr);
}

/**
  Create a user defined function.

  Atomicity:
    The operation to create a user defined function is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. UDF hash is in sync with operation state.
    UDF hash do not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between the
    data-dictionary table and the binary log.

  @param thd                 THD context.
  @param udf                 Pointer to UDF function.

  @note Like implementations of other DDL/DML in MySQL, this function
  relies on the caller to close the thread tables. This is done in the
  end of dispatch_command().
*/

bool mysql_create_function(THD *thd, udf_func *udf) {
  bool error = true;
  void *dl = nullptr;
  int new_dl = 0;
  TABLE *table;

  DBUG_TRACE;

  if (!initialized) {
    if (opt_noacl)
      my_error(ER_CANT_INITIALIZE_UDF, MYF(0), udf->name.str,
               "UDFs are unavailable with the --skip-grant-tables option");
    else
      my_error(ER_OUT_OF_RESOURCES, MYF(0));
    return error;
  }

  /* must not be dynamically registered */
  DBUG_ASSERT(udf->dl);

  /*
    Ensure that the .dll doesn't have a path
    This is done to ensure that only approved dll from the system
    directories are used (to make this even remotely secure).
  */
  if (check_valid_path(udf->dl, strlen(udf->dl))) {
    my_error(ER_UDF_NO_PATHS, MYF(0));
    return error;
  }
  LEX_CSTRING udf_name_cstr = {udf->name.str, udf->name.length};
  if (check_string_char_length(udf_name_cstr, "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), udf->name.str);
    return error;
  }

  /*
    Acquire MDL SNRW for TL_WRITE type so that deadlock and
    timeout errors are avoided from the Storage Engine.
  */
  TABLE_LIST tables("mysql", "func", TL_WRITE, MDL_SHARED_NO_READ_WRITE);

  if (open_and_lock_tables(thd, &tables, MYSQL_LOCK_IGNORE_TIMEOUT))
    return error;
  table = tables.table;
  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for CREATE FUNCTION command.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  mysql_rwlock_rdlock(&THR_LOCK_udf);
  if (udf_hash->count(to_string(udf->name)) != 0) {
    my_error(ER_UDF_EXISTS, MYF(0), udf->name.str);
    mysql_rwlock_unlock(&THR_LOCK_udf);
    return error;
  }
  dl = find_udf_dl(udf->dl);
  mysql_rwlock_unlock(&THR_LOCK_udf);

  if (dl == nullptr) {
    char dlpath[FN_REFLEN];
    strxnmov(dlpath, sizeof(dlpath) - 1, opt_plugin_dir, "/", udf->dl, NullS);
    (void)unpack_filename(dlpath, dlpath);

    if (!(dl = dlopen(dlpath, RTLD_NOW))) {
      const char *errmsg;
      int error_number = dlopen_errno;
      DLERROR_GENERATE(errmsg, error_number);

      DBUG_PRINT("error", ("dlopen of %s failed, error: %d (%s)", udf->dl,
                           error_number, errmsg));
      my_error(ER_CANT_OPEN_LIBRARY, MYF(0), udf->dl, error_number, errmsg);
      return error;
    }
    new_dl = 1;
  }
  udf->dlhandle = dl;
  {
    char buf[NAME_LEN + 16], *missing;
    if ((missing = init_syms(udf, buf))) {
      my_error(ER_CANT_FIND_DL_ENTRY, MYF(0), missing);
      if (new_dl) dlclose(dl);
      return error;
    }
  }

  // create entry in mysql.func table

  table->use_all_columns();
  restore_record(table, s->default_values);  // Default values for fields
  table->field[0]->store(udf->name.str, udf->name.length, system_charset_info);
  table->field[1]->store((longlong)udf->returns, true);
  table->field[2]->store(udf->dl, strlen(udf->dl), system_charset_info);
  if (table->s->fields >= 4)  // If not old func format
    table->field[3]->store((longlong)udf->type, true);
  error = (table->file->ha_write_row(table->record[0]) != 0);

  // Binlog the create function.
  if (!error)
    error = (write_bin_log(thd, true, thd->query().str, thd->query().length,
                           true) != 0);

  error = udf_end_transaction(thd, error, udf, true);

  if (error) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_error(ER_ERROR_ON_WRITE, MYF(0), "mysql.func", error,
             my_strerror(errbuf, sizeof(errbuf), error));
    if (new_dl) dlclose(dl);
  }
  return error;
}

/**
  Drop a user defined function.

  Atomicity:
    The operation to drop a user defined function is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. UDF hash is in sync with operation state.
    UDF hash do not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between the
    data-dictionary table and the binary log.

  @param thd                 THD context.
  @param udf_name            Name of the UDF function.
*/

bool mysql_drop_function(THD *thd, const LEX_STRING *udf_name) {
  TABLE *table;
  udf_func *udf;
  bool error = true;

  DBUG_TRACE;

  if (!initialized) {
    if (opt_noacl)
      my_error(ER_FUNCTION_NOT_DEFINED, MYF(0), udf_name->str);
    else
      my_error(ER_OUT_OF_RESOURCES, MYF(0));
    return error;
  }

  TABLE_LIST tables("mysql", "func", TL_WRITE, MDL_SHARED_NO_READ_WRITE);

  if (open_and_lock_tables(thd, &tables, MYSQL_LOCK_IGNORE_TIMEOUT))
    return error;
  table = tables.table;
  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for DROP FUNCTION command.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  mysql_rwlock_rdlock(&THR_LOCK_udf);
  const auto it = udf_hash->find(to_string(*udf_name));
  if (it == udf_hash->end()) {
    my_error(ER_FUNCTION_NOT_DEFINED, MYF(0), udf_name->str);
    mysql_rwlock_unlock(&THR_LOCK_udf);
    return error;
  }
  udf = it->second;
  if (!udf->dl) {
    mysql_rwlock_unlock(&THR_LOCK_udf);
    my_error(ER_UDF_DROP_DYNAMICALLY_REGISTERED, MYF(0));
    return error;
  }

  mysql_rwlock_unlock(&THR_LOCK_udf);

  table->use_all_columns();
  table->field[0]->store(udf->name.str, udf->name.length, &my_charset_bin);
  if (!table->file->ha_index_read_idx_map(table->record[0], 0,
                                          table->field[0]->ptr, HA_WHOLE_KEY,
                                          HA_READ_KEY_EXACT)) {
    int delete_err;
    if ((delete_err = table->file->ha_delete_row(table->record[0])))
      table->file->print_error(delete_err, MYF(0));
    error = delete_err != 0;
  }

  /*
    Binlog the drop function. Keep the table open and locked
    while binlogging, to avoid binlog inconsistency.
  */
  if (!error)
    error = (write_bin_log(thd, true, thd->query().str, thd->query().length,
                           true) != 0);

  error = udf_end_transaction(thd, error, udf, false);

  /*
    Close the handle if this was function that was found during boot or
    CREATE FUNCTION and it's not in use by any other udf function
  */
  if (udf->dlhandle && !find_udf_dl(udf->dl)) dlclose(udf->dlhandle);

  return error;
}

bool mysql_udf_registration_imp::udf_register_inner(udf_func *ufunc) {
  mysql_rwlock_wrlock(&THR_LOCK_udf);

  DBUG_ASSERT(ufunc->dl == nullptr);
  DBUG_ASSERT(ufunc->dlhandle == nullptr);

  auto res = udf_hash->emplace(to_string(ufunc->name), ufunc);
  if (!res.second)
    ufunc = nullptr;
  else
    using_udf_functions = true;

  mysql_rwlock_unlock(&THR_LOCK_udf);
  return ufunc == nullptr;
}

udf_func *mysql_udf_registration_imp::alloc_udf(const char *name,
                                                Item_result return_type,
                                                Udf_func_any func,
                                                Udf_func_init init_func,
                                                Udf_func_deinit deinit_func) {
  udf_func *ufunc;

  ufunc = (udf_func *)mem.Alloc(sizeof(udf_func));
  if (!ufunc) return nullptr;
  memset(ufunc, 0, sizeof(udf_func));
  ufunc->name.str = strdup_root(&mem, name);
  ufunc->name.length = strlen(name);
  ufunc->func = func;
  ufunc->func_init = init_func;
  ufunc->func_deinit = deinit_func;
  ufunc->returns = return_type;
  ufunc->usage_count = 1;

  return ufunc;
}

DEFINE_BOOL_METHOD(mysql_udf_registration_imp::udf_register,
                   (const char *name, Item_result return_type,
                    Udf_func_any func, Udf_func_init init_func,
                    Udf_func_deinit deinit_func)) {
  udf_func *ufunc;

  if (!func && !init_func && !deinit_func) return true;

  ufunc = alloc_udf(name, return_type, func, init_func, deinit_func);
  if (!ufunc) return true;
  ufunc->type = Item_udftype::UDFTYPE_FUNCTION;

  return udf_register_inner(ufunc);
}

DEFINE_BOOL_METHOD(mysql_udf_registration_imp::udf_register_aggregate,
                   (const char *name, enum Item_result return_type,
                    Udf_func_any func, Udf_func_init init_func,
                    Udf_func_deinit deinit_func, Udf_func_add add_func,
                    Udf_func_clear clear_func)) {
  udf_func *ufunc;

  if (!func && !add_func && !clear_func && !init_func && !deinit_func)
    return true;

  ufunc = alloc_udf(name, return_type, func, init_func, deinit_func);
  if (!ufunc) return true;
  ufunc->type = Item_udftype::UDFTYPE_AGGREGATE;
  ufunc->func_add = add_func;
  ufunc->func_clear = clear_func;

  return udf_register_inner(ufunc);
}

DEFINE_BOOL_METHOD(mysql_udf_registration_imp::udf_unregister,
                   (const char *name, int *was_present)) {
  udf_func *udf = nullptr;

  if (was_present) *was_present = 0;
  mysql_rwlock_wrlock(&THR_LOCK_udf);
  const auto it = udf_hash->find(name);
  if (it != udf_hash->end()) {
    if (was_present) *was_present = 1;

    udf = it->second;

    if (!udf->dl && !udf->dlhandle &&  // Not registered via CREATE FUNCTION
        !--udf->usage_count)           // Not used
    {
      udf_hash->erase(it);
      using_udf_functions = !udf_hash->empty();
    } else  // error
      udf = nullptr;
  }
  mysql_rwlock_unlock(&THR_LOCK_udf);
  return udf != nullptr ? false : true;
}

void udf_hash_rlock(void) { mysql_rwlock_rdlock(&THR_LOCK_udf); }

void udf_hash_unlock(void) { mysql_rwlock_unlock(&THR_LOCK_udf); }

ulong udf_hash_size(void) { return udf_hash->size(); }

void udf_hash_for_each(udf_hash_for_each_func_t *func, void *arg) {
  for (auto it : *udf_hash) func(it.second, arg);
}
