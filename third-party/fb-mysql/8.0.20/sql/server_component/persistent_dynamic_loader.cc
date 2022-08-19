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

#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "m_ctype.h"
#include "m_string.h"
#include "mutex_lock.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/components/service_implementation.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysqld_error.h"
#include "persistent_dynamic_loader_imp.h"
#include "scope_guard.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // commit_and_close_mysql_tables
#include "sql/debug_sync.h"
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/mysqld.h"
#include "sql/records.h"
#include "sql/row_iterator.h"
#include "sql/sql_base.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"
#include "sql_string.h"
#include "thr_lock.h"
#include "thr_mutex.h"

class THD;
SERVICE_TYPE(dynamic_loader) * dynamic_loader_srv;

typedef std::string my_string;

enum enum_component_table_field {
  CT_FIELD_COMPONENT_ID = 0,
  CT_FIELD_GROUP_ID,
  CT_FIELD_COMPONENT_URN,
  CT_FIELD_COUNT /* a cool trick to count the number of fields :) */
};

static const TABLE_FIELD_TYPE component_table_fields[CT_FIELD_COUNT] = {
    {{STRING_WITH_LEN("component_id")}, {STRING_WITH_LEN("int")}, {nullptr, 0}},
    {{STRING_WITH_LEN("component_group_id")},
     {STRING_WITH_LEN("int")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("component_urn")},
     {STRING_WITH_LEN("text")},
     {STRING_WITH_LEN("utf8")}}};

static const TABLE_FIELD_DEF component_table_def = {CT_FIELD_COUNT,
                                                    component_table_fields};

class Component_db_intact : public Table_check_intact {
 protected:
  void report_error(uint ecode, const char *fmt, ...)
      MY_ATTRIBUTE((format(printf, 3, 4))) {
    longlong log_ecode = 0;
    switch (ecode) {
      case 0:
        log_ecode = ER_SERVER_TABLE_CHECK_FAILED;
        break;
      case ER_CANNOT_LOAD_FROM_TABLE_V2:
        log_ecode = ER_SERVER_CANNOT_LOAD_FROM_TABLE_V2;
        break;
      case ER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2:
        log_ecode = ER_SERVER_COL_COUNT_DOESNT_MATCH_PLEASE_UPDATE_V2;
        break;
      case ER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2:
        log_ecode = ER_SERVER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2;
        break;
      default:
        DBUG_ASSERT(false);
        return;
    }

    va_list args;
    va_start(args, fmt);
    LogEvent()
        .type(LOG_TYPE_ERROR)
        .prio(ERROR_LEVEL)
        .errcode(log_ecode)
        .messagev(fmt, args);
    va_end(args);
  }
};

/** In case of an error, a message is printed to the error log. */
static Component_db_intact table_intact;

/**
  Open mysql.component table for read or write.

  Note that if the table can't be locked successfully this operation will
  close it. Therefore it provides guarantee that it either opens and locks
  table or fails without leaving any tables open.

  @param thd Thread context
  @param lock_type How to lock the table
  @param [out] table Pointer to table structure to store the open table into.
  @param acl_to_check acl type to check
  @return Status of performed operation
  @retval true open and lock failed - an error message is pushed into the
    stack.
  @retval false success
*/
static bool open_component_table(THD *thd, enum thr_lock_type lock_type,
                                 TABLE **table, ulong acl_to_check) {
  TABLE_LIST tables("mysql", "component", lock_type);

  if (mysql_persistent_dynamic_loader_imp::initialized() && !opt_noacl &&
      check_one_table_access(thd, acl_to_check, &tables))
    return true;

  if (!(*table =
            open_ltable(thd, &tables, lock_type, MYSQL_LOCK_IGNORE_TIMEOUT))) {
    return true;
  }

  *table = tables.table;
  (*table)->use_all_columns();

  if (table_intact.check(thd, *table, &component_table_def)) {
    trans_rollback_stmt(thd);
    close_mysql_tables(thd);
    return true;
  }

  return false;
}

/**
  Initializes persistence store, loads all groups of components registered in
  component table. Shouldn't be called multiple times. We assume the order
  specified by group ID is correct one. This should be assured by dynamic
  loader as long as it will not allow to unload the component that has
  dependency on, in case there would be a possibility to switch that
  dependency to other component that is not to be unloaded. If this is
  assured, then it will not be possible for components with lower group IDs
  to have a dependency on component with higher group ID, even after state is
  restored in this initialization method.

  @param thdp Current thread execution context
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
bool mysql_persistent_dynamic_loader_imp::init(void *thdp) {
  try {
    THD *thd = reinterpret_cast<THD *>(thdp);

    if (mysql_persistent_dynamic_loader_imp::initialized()) {
      return true;
    }

    static PSI_mutex_key key_component_id_by_urn_mutex;
    static PSI_mutex_info all_dyloader_mutexes[] = {
        {&key_component_id_by_urn_mutex, "key_component_id_by_urn_mutex", 0, 0,
         PSI_DOCUMENT_ME}};

    int count = (int)array_elements(all_dyloader_mutexes);
    mysql_mutex_register("p_dyn_loader", all_dyloader_mutexes, count);

    mysql_mutex_init(key_component_id_by_urn_mutex, &component_id_by_urn_mutex,
                     MY_MUTEX_INIT_SLOW);

    TABLE *component_table;
    int res;

    mysql_persistent_dynamic_loader_imp::s_group_id = 0;

    /* Open component table and scan read all records. */
    if (open_component_table(thd, TL_READ, &component_table, SELECT_ACL)) {
      push_warning(thd, Sql_condition::SL_WARNING, ER_COMPONENT_TABLE_INCORRECT,
                   ER_THD(thd, ER_COMPONENT_TABLE_INCORRECT));
      mysql_persistent_dynamic_loader_imp::is_initialized = true;
      return false;
    }

    auto guard =
        create_scope_guard([&thd]() { commit_and_close_mysql_tables(thd); });

    unique_ptr_destroy_only<RowIterator> iterator =
        init_table_iterator(thd, component_table, nullptr, false,
                            /*ignore_not_found_rows=*/false);
    if (iterator == nullptr) {
      push_warning(thd, Sql_condition::SL_WARNING, ER_COMPONENT_TABLE_INCORRECT,
                   ER_THD(thd, ER_COMPONENT_TABLE_INCORRECT));
      return false;
    }

    if (component_table->s->fields < CT_FIELD_COUNT) {
      push_warning(thd, Sql_condition::SL_WARNING, ER_COMPONENT_TABLE_INCORRECT,
                   ER_THD(thd, ER_COMPONENT_TABLE_INCORRECT));
      return false;
    }

    /* All read records will be aggregated in groups by group ID. */
    std::map<uint64, std::vector<std::string>> component_groups;

    for (;;) {
      res = iterator->Read();
      if (res != 0) {
        break;
      }

      uint64 component_id =
          component_table->field[CT_FIELD_COMPONENT_ID]->val_int();
      uint64 component_group_id =
          component_table->field[CT_FIELD_GROUP_ID]->val_int();
      String component_urn_str;
      component_table->field[CT_FIELD_COMPONENT_URN]->val_str(
          &component_urn_str);

      std::string component_urn(component_urn_str.ptr(),
                                component_urn_str.length());

      mysql_persistent_dynamic_loader_imp::s_group_id =
          std::max(mysql_persistent_dynamic_loader_imp::s_group_id.load(),
                   component_group_id);

      component_groups[component_group_id].push_back(component_urn);
      {
        MUTEX_LOCK(lock, &component_id_by_urn_mutex);
        mysql_persistent_dynamic_loader_imp::component_id_by_urn.emplace(
            component_urn, component_id);
      }
    }

    iterator.reset();

    /* res is guaranteed to be != 0, -1 means end of records encountered, which
      is interpreted as a success. */
    DBUG_ASSERT(res != 0);
    if (res != -1) {
      return true;
    }

    for (auto it = component_groups.begin(); it != component_groups.end();
         ++it) {
      std::vector<const char *> urns;
      for (auto group_it = it->second.begin(); group_it != it->second.end();
           ++group_it) {
        urns.push_back(group_it->c_str());
      }
      /* We continue process despite of any errors. */
      dynamic_loader_srv->load(urns.data(), (int)urns.size());
    }

    mysql_persistent_dynamic_loader_imp::is_initialized = true;

    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }

  return true;
}
/**
  De-initializes persistence loader.
*/
void mysql_persistent_dynamic_loader_imp::deinit() {
  if (is_initialized) {
    mysql_persistent_dynamic_loader_imp::component_id_by_urn.clear();
    mysql_mutex_destroy(&component_id_by_urn_mutex);
  }
  mysql_persistent_dynamic_loader_imp::is_initialized = false;
}
/**
 Initialisation status of persistence loader.

 @return Status of performed operation
 @retval true initialization is done
 @retval false initialization is not done
*/
bool mysql_persistent_dynamic_loader_imp::initialized() {
  return mysql_persistent_dynamic_loader_imp::is_initialized;
}

/**
  Loads specified group of components by URN, initializes them and
  registers all service implementations present in these components.
  Assures all dependencies will be met after loading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to load in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  service implementation of scheme component loader service for specified
  scheme. If the loading process successes then a group of Components by their
  URN is added to the component table.

  @param thd_ptr Current thread execution context.
  @param urns List of URNs of Components to load.
  @param component_count Number of Components on list to load.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_persistent_dynamic_loader_imp::load,
                   (void *thd_ptr, const char *urns[], int component_count)) {
  try {
    THD *thd = (THD *)thd_ptr;

    if (!mysql_persistent_dynamic_loader_imp::initialized()) {
      my_error(ER_COMPONENT_TABLE_INCORRECT, MYF(0));
      return true;
    }

    MUTEX_LOCK(lock, &component_id_by_urn_mutex);

    /* We don't replicate INSTALL COMPONENT */
    Disable_binlog_guard binlog_guard(thd);

    TABLE *component_table;
    auto guard_close_tables = create_scope_guard([&thd] {
      trans_rollback_stmt(thd);
      close_mysql_tables(thd);
    });

    if (open_component_table(thd, TL_WRITE, &component_table, INSERT_ACL)) {
      my_error(ER_COMPONENT_TABLE_INCORRECT, MYF(0));
      return true;
    }

    uint64 group_id = ++mysql_persistent_dynamic_loader_imp::s_group_id;

    /* Making a component_id_by_urn copy so, that if any error occurs it will
       be restored
    */
    std::map<my_string, uint64> copy_urn_map(
        mysql_persistent_dynamic_loader_imp::component_id_by_urn);

    auto guard = create_scope_guard([&copy_urn_map]() {
      /* Restoring back the component_id_by_urn */
      mysql_persistent_dynamic_loader_imp::component_id_by_urn = copy_urn_map;
    });

    /* Insert all component URNs into component table into one group. */
    CHARSET_INFO *system_charset = system_charset_info;
    for (int i = 0; i < component_count; ++i) {
      /* Get default values for fields. */
      restore_record(component_table, s->default_values);

      component_table->next_number_field =
          component_table->found_next_number_field;

      component_table->field[CT_FIELD_GROUP_ID]->store(group_id, true);
      component_table->field[CT_FIELD_COMPONENT_URN]->store(
          urns[i], strlen(urns[i]), system_charset);

      int res = component_table->file->ha_write_row(component_table->record[0]);
      if (res != 0) {
        my_error(ER_COMPONENT_MANIPULATE_ROW_FAILED, MYF(0), urns[i], res);
        component_table->file->ha_release_auto_increment();
        return true;
      }

      /* Use last insert auto-increment column value and store it by the URN. */
      mysql_persistent_dynamic_loader_imp::component_id_by_urn.emplace(
          urns[i], component_table->file->insert_id_for_cur_row);

      component_table->file->ha_release_auto_increment();
    }
    if (dynamic_loader_srv->load(urns, component_count)) {
      return true;
    }

    guard.commit();
    guard_close_tables.commit();
    trans_commit_stmt(thd);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Unloads specified group of Components by URN, deinitializes them and
  unregisters all service implementations present in these components.
  Assumes, although does not check it, all dependencies of not unloaded
  components will still be met after unloading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to unload in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  service implementation of scheme component loader service for specified
  scheme. URN specified should be identical to ones specified in load()
  method, i.e. all letters must have the same case. If the unloading process
  successes then a group of Components by their URN is added to the component
  table.

  @param thd_ptr Current thread execution context.
  @param urns List of URNs of components to unload.
  @param component_count Number of components on list to unload.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DEFINE_BOOL_METHOD(mysql_persistent_dynamic_loader_imp::unload,
                   (void *thd_ptr, const char *urns[], int component_count)) {
  try {
    THD *thd = (THD *)thd_ptr;

    if (!mysql_persistent_dynamic_loader_imp::initialized()) {
      my_error(ER_COMPONENT_TABLE_INCORRECT, MYF(0));
      return true;
    }

    MUTEX_LOCK(lock, &component_id_by_urn_mutex);

    TABLE *component_table;
    if (open_component_table(thd, TL_WRITE, &component_table, DELETE_ACL)) {
      my_error(ER_COMPONENT_TABLE_INCORRECT, MYF(0));
      return true;
    }

    /* We don't replicate UNINSTALL_COMPONENT */
    Disable_binlog_guard binlog_guard(thd);

    /* Making component_id_by_urn copy so, that if any error occurs it will
       be restored
    */
    std::map<my_string, uint64> copy_urn_map(
        mysql_persistent_dynamic_loader_imp::component_id_by_urn);
    auto guard_close_tables = create_scope_guard([&thd, &copy_urn_map]() {
      /* Restoring back the component_id_by_urn */
      mysql_persistent_dynamic_loader_imp::component_id_by_urn = copy_urn_map;
      trans_rollback_stmt(thd);
      close_mysql_tables(thd);
    });

    DBUG_ASSERT(component_table->key_info != nullptr);

    for (int i = 0; i < component_count; ++i) {
      /* Find component ID used in component table using memory mapping. */
      auto it = mysql_persistent_dynamic_loader_imp::component_id_by_urn.find(
          urns[i]);
      if (it ==
          mysql_persistent_dynamic_loader_imp::component_id_by_urn.end()) {
        /* Component was loaded with persistence bypassed? If we continue, the
          state will be still consistent. */
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_WARN_UNLOAD_THE_NOT_PERSISTED,
            ER_THD(thd, ER_WARN_UNLOAD_THE_NOT_PERSISTED), urns[i]);
        continue;
      }
      component_table->field[CT_FIELD_COMPONENT_ID]->store(it->second, true);

      /* Place PK index on specified record and delete it. */
      uchar key[MAX_KEY_LENGTH];
      key_copy(key, component_table->record[0], component_table->key_info,
               component_table->key_info->key_length);

      DEBUG_SYNC(thd, "before_ha_index_read_idx_map");
      int res = component_table->file->ha_index_read_idx_map(
          component_table->record[0], 0, key, HA_WHOLE_KEY, HA_READ_KEY_EXACT);
      if (res != 0) {
        my_error(ER_COMPONENT_MANIPULATE_ROW_FAILED, MYF(0), urns[i], res);
        return true;
      }

      res = component_table->file->ha_delete_row(component_table->record[0]);
      if (res != 0) {
        my_error(ER_COMPONENT_MANIPULATE_ROW_FAILED, MYF(0), urns[i], res);
        return true;
      }

      mysql_persistent_dynamic_loader_imp::component_id_by_urn.erase(it);
    }

    bool result = dynamic_loader_srv->unload(urns, component_count);
    if (result) {
      /* No need to specify error, underlying service implementation would add
        one. */
      return result;
    }

    guard_close_tables.commit();
    trans_commit_stmt(thd);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

std::atomic<uint64> mysql_persistent_dynamic_loader_imp::s_group_id;
std::map<my_string, uint64>
    mysql_persistent_dynamic_loader_imp::component_id_by_urn;
bool mysql_persistent_dynamic_loader_imp::is_initialized = false;

mysql_mutex_t mysql_persistent_dynamic_loader_imp::component_id_by_urn_mutex;

/**
  Initializes persistence store, loads all groups of components registered in
    component table.

  @param thd Current thread execution context
  @return Status of performed operation
  @retval false success
  @retval true failure
  */
bool persistent_dynamic_loader_init(void *thd) {
  return mysql_persistent_dynamic_loader_imp::init(
      reinterpret_cast<THD *>(thd));
}

void persistent_dynamic_loader_deinit() {
  mysql_persistent_dynamic_loader_imp::deinit();
}
