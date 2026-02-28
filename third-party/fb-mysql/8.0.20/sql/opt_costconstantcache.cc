/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/opt_costconstantcache.h"

#include <memory>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"  // current_thd
#include "sql/field.h"        // Field
#include "sql/mysqld.h"       // key_LOCK_cost_const
#include "sql/records.h"      // unique_ptr_destroy_only<RowIterator>
#include "sql/row_iterator.h"
#include "sql/sql_base.h"   // open_and_lock_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"        // lex_start/lex_end
#include "sql/sql_tmp_table.h"  // init_cache_tmp_engine_properties
#include "sql/table.h"          // TABLE
#include "sql/transaction.h"    // trans_commit_stmt
#include "sql_string.h"
#include "template_utils.h"  // pointer_cast
#include "thr_lock.h"
#include "thr_mutex.h"

Cost_constant_cache *cost_constant_cache = nullptr;

static void read_cost_constants(Cost_model_constants *cost_constants);

/**
  Minimal initialization of the object. The main initialization is done
  by calling init().
*/

Cost_constant_cache::Cost_constant_cache()
    : current_cost_constants(nullptr), m_inited(false) {}

Cost_constant_cache::~Cost_constant_cache() {
  // Verify that close has been called
  DBUG_ASSERT(current_cost_constants == nullptr);
  DBUG_ASSERT(m_inited == false);
}

void Cost_constant_cache::init() {
  DBUG_TRACE;

  DBUG_ASSERT(m_inited == false);

  // Initialize the mutex that is used for protecting the cost constants
  mysql_mutex_init(key_LOCK_cost_const, &LOCK_cost_const, MY_MUTEX_INIT_FAST);

  // Create cost constants from constants found in the source code
  Cost_model_constants *cost_constants = create_defaults();

  // Set this to be the current set of cost constants
  update_current_cost_constants(cost_constants);

  m_inited = true;
}

void Cost_constant_cache::close() {
  DBUG_TRACE;

  DBUG_ASSERT(m_inited);

  if (m_inited == false) return; /* purecov: inspected */

  // Release the current cost constant set
  if (current_cost_constants) {
    release_cost_constants(current_cost_constants);
    current_cost_constants = nullptr;
  }

  // To ensure none is holding the mutex when deleting it, lock/unlock it.
  mysql_mutex_lock(&LOCK_cost_const);
  mysql_mutex_unlock(&LOCK_cost_const);

  mysql_mutex_destroy(&LOCK_cost_const);

  m_inited = false;
}

void Cost_constant_cache::reload() {
  DBUG_TRACE;
  DBUG_ASSERT(m_inited = true);

  // Create cost constants from the constants defined in the source code
  Cost_model_constants *cost_constants = create_defaults();

  // Update the cost constants from the database tables
  read_cost_constants(cost_constants);

  // Set this to be the current set of cost constants
  update_current_cost_constants(cost_constants);
}

Cost_model_constants *Cost_constant_cache::create_defaults() const {
  // Create default cost constants
  Cost_model_constants *cost_constants = new Cost_model_constants();

  return cost_constants;
}

void Cost_constant_cache::update_current_cost_constants(
    Cost_model_constants *new_cost_constants) {
  /*
    Increase the ref counter to ensure that the new cost constants
    are not deleted until next time we have a new set of cost constants.
  */
  new_cost_constants->inc_ref_count();

  /*
    The mutex needs to be held for the entire period for removing the
    current cost constants and adding the new cost constants to ensure
    that no user of this class can access the object when there is no
    current cost constants.
  */
  mysql_mutex_lock(&LOCK_cost_const);

  // Release the current cost constants by decrementing the ref counter
  if (current_cost_constants) {
    const unsigned int ref_count = current_cost_constants->dec_ref_count();

    // If there is none using the current cost constants then delete them
    if (ref_count == 0) delete current_cost_constants;
  }

  // Start to use the new cost constants
  current_cost_constants = new_cost_constants;

  mysql_mutex_unlock(&LOCK_cost_const);
}

/**
  Write warnings about illegal entries in the server_cost table

  The warnings are written to the MySQL error log.

  @param cost_name  name of the cost constant
  @param value      value it was attempted set to
  @param error      error status
*/

static void report_server_cost_warnings(const LEX_CSTRING &cost_name,
                                        double value,
                                        cost_constant_error error) {
  switch (error) {
    case UNKNOWN_COST_NAME:
      LogErr(WARNING_LEVEL, ER_SERVER_COST_UNKNOWN_COST_CONSTANT,
             cost_name.str);
      break;
    case INVALID_COST_VALUE:
      LogErr(WARNING_LEVEL, ER_SERVER_COST_INVALID_COST_CONSTANT, cost_name.str,
             value);
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

/**
  Write warnings about illegal entries in the engine_cost table

  The warnings are written to the MySQL error log.

  @param se_name          name of storage engine
  @param storage_category device type
  @param cost_name        name of the cost constant
  @param value            value it was attempted set to
  @param error            error status
*/

static void report_engine_cost_warnings(const LEX_CSTRING &se_name,
                                        int storage_category,
                                        const LEX_CSTRING &cost_name,
                                        double value,
                                        cost_constant_error error) {
  switch (error) {
    case UNKNOWN_COST_NAME:
      LogErr(WARNING_LEVEL, ER_ENGINE_COST_UNKNOWN_COST_CONSTANT,
             cost_name.str);
      break;
    case UNKNOWN_ENGINE_NAME:
      LogErr(WARNING_LEVEL, ER_ENGINE_COST_UNKNOWN_STORAGE_ENGINE, se_name.str);
      break;
    case INVALID_DEVICE_TYPE:
      LogErr(WARNING_LEVEL, ER_ENGINE_COST_INVALID_DEVICE_TYPE_FOR_SE,
             storage_category, se_name.str, cost_name.str);
      break;
    case INVALID_COST_VALUE:
      LogErr(WARNING_LEVEL,
             ER_ENGINE_COST_INVALID_CONST_CONSTANT_FOR_SE_AND_DEVICE,
             cost_name.str, se_name.str, storage_category, value);
      break;
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
  }
}

/**
  Read the table that contains the cost constants for the server.

  The table must already be opened. The cost constant object is updated
  with cost constants found in the configuration table.

  @param thd                    the THD
  @param table                  the table to read from
  @param [in,out] cost_constants cost constant object
*/

static void read_server_cost_constants(THD *thd, TABLE *table,
                                       Cost_model_constants *cost_constants) {
  DBUG_TRACE;

  /*
    The server constant table has the following columns:

    cost_name   VARCHAR(64) NOT NULL COLLATE utf8_general_ci
    cost_value  FLOAT DEFAULT NULL
    last_update TIMESTAMP
    comment     VARCHAR(1024) DEFAULT NULL
  */

  // Prepare to read from the table
  unique_ptr_destroy_only<RowIterator> iterator =
      init_table_iterator(thd, table, nullptr, false,
                          /*ignore_not_found_rows=*/false);
  if (iterator != nullptr) {
    table->use_all_columns();

    // Read one record
    while (!iterator->Read()) {
      /*
        Check if a non-default value has been configured for this cost
        constant.
      */
      if (!table->field[1]->is_null()) {
        char cost_name_buf[MAX_FIELD_WIDTH];
        String cost_name(cost_name_buf, sizeof(cost_name_buf),
                         &my_charset_utf8_general_ci);

        // Read the name of the cost constant
        table->field[0]->val_str(&cost_name);
        cost_name[cost_name.length()] = 0;  // Null-terminate

        // Read the value this cost constant should have
        const float value = static_cast<float>(table->field[1]->val_real());

        // Update the cost model with this cost constant
        const LEX_CSTRING cost_constant = cost_name.lex_cstring();
        const cost_constant_error err =
            cost_constants->update_server_cost_constant(cost_constant, value);

        if (err != COST_CONSTANT_OK)
          report_server_cost_warnings(cost_constant, value, err);
      }
    }
  } else {
    LogErr(WARNING_LEVEL, ER_SERVER_COST_FAILED_TO_READ);
  }
}

/**
  Read the table that contains the cost constants for the storage engines.

  The table must already be opened. The cost constant object is updated
  with cost constants found in the configuration table.

  @param thd                    the THD
  @param table                  the table to read from
  @param [in,out] cost_constants cost constant object
*/

static void read_engine_cost_constants(THD *thd, TABLE *table,
                                       Cost_model_constants *cost_constants) {
  DBUG_TRACE;

  /*
    The engine constant table has the following columns:

    engine_name VARCHAR(64) NOT NULL COLLATE utf8_general_ci,
    device_type INTEGER NOT NULL,
    cost_name   VARCHAR(64) NOT NULL COLLATE utf8_general_ci,
    cost_value  FLOAT DEFAULT NULL,
    last_update TIMESTAMP
    comment     VARCHAR(1024) DEFAULT NULL,
  */

  // Prepare to read from the table
  unique_ptr_destroy_only<RowIterator> iterator =
      init_table_iterator(thd, table, nullptr, false,
                          /*ignore_not_found_rows=*/false);
  if (iterator != nullptr) {
    table->use_all_columns();

    // Read one record
    while (!iterator->Read()) {
      /*
        Check if a non-default value has been configured for this cost
        constant.
      */
      if (!table->field[3]->is_null()) {
        char engine_name_buf[MAX_FIELD_WIDTH];
        String engine_name(engine_name_buf, sizeof(engine_name_buf),
                           &my_charset_utf8_general_ci);
        char cost_name_buf[MAX_FIELD_WIDTH];
        String cost_name(cost_name_buf, sizeof(cost_name_buf),
                         &my_charset_utf8_general_ci);

        // Read the name of the storage engine
        table->field[0]->val_str(&engine_name);
        engine_name[engine_name.length()] = 0;  // Null-terminate

        // Read the device type
        const int device_type = static_cast<int>(table->field[1]->val_int());

        // Read the name of the cost constant
        table->field[2]->val_str(&cost_name);
        cost_name[cost_name.length()] = 0;  // Null-terminate

        // Read the value this cost constant should have
        const float value = static_cast<float>(table->field[3]->val_real());

        // Update the cost model with this cost constant
        const LEX_CSTRING engine = engine_name.lex_cstring();
        const LEX_CSTRING cost_constant = cost_name.lex_cstring();
        const cost_constant_error err =
            cost_constants->update_engine_cost_constant(
                thd, engine, device_type, cost_constant, value);
        if (err != COST_CONSTANT_OK)
          report_engine_cost_warnings(engine, device_type, cost_constant, value,
                                      err);
      }
    }
  } else {
    LogErr(WARNING_LEVEL, ER_ENGINE_COST_FAILED_TO_READ);
  }
}

/**
  Read the cost configuration tables and update the cost constant set.

  The const constant set must be initialized with default values when
  calling this function.

  @param cost_constants set with cost constants
*/

static void read_cost_constants(Cost_model_constants *cost_constants) {
  DBUG_TRACE;

  /*
    This function creates its own THD. If there exists a current THD this needs
    to be restored at the end of this function. The reason the current THD
    can not be used is that this might already have opened and closed tables
    and thus opening new tables will fail.
  */
  THD *orig_thd = current_thd;

  // Create and initialize a new THD.
  THD *thd = new THD;
  DBUG_ASSERT(thd);
  thd->thread_stack = pointer_cast<char *>(&thd);
  thd->store_globals();
  lex_start(thd);

  TABLE_LIST tables[2] = {TABLE_LIST("mysql", "server_cost", TL_READ),
                          TABLE_LIST("mysql", "engine_cost", TL_READ)};
  tables[0].next_global = tables[0].next_local =
      tables[0].next_name_resolution_table = &tables[1];

  if (!open_and_lock_tables(thd, tables, MYSQL_LOCK_IGNORE_TIMEOUT)) {
    DBUG_ASSERT(tables[0].table != nullptr);
    DBUG_ASSERT(tables[1].table != nullptr);

    // Read the server constants table
    read_server_cost_constants(thd, tables[0].table, cost_constants);
    // Read the storage engine table
    read_engine_cost_constants(thd, tables[1].table, cost_constants);
  } else {
    LogErr(WARNING_LEVEL, ER_FAILED_TO_OPEN_COST_CONSTANT_TABLES);
  }

  trans_commit_stmt(thd);
  close_thread_tables(thd);
  lex_end(thd->lex);

  // Delete the locally created THD
  delete thd;

  // If the caller already had a THD, this must be restored
  if (orig_thd) orig_thd->store_globals();
}

void init_optimizer_cost_module(bool enable_plugins) {
  DBUG_ASSERT(cost_constant_cache == nullptr);
  cost_constant_cache = new Cost_constant_cache();
  cost_constant_cache->init();
  /*
    Initialize max_key_length and max_key_part_length for internal temporary
    table engines.
  */
  if (enable_plugins) init_cache_tmp_engine_properties();
}

void delete_optimizer_cost_module() {
  if (cost_constant_cache) {
    cost_constant_cache->close();
    delete cost_constant_cache;
    cost_constant_cache = nullptr;
  }
}

void reload_optimizer_cost_constants() {
  if (cost_constant_cache) cost_constant_cache->reload();
}
