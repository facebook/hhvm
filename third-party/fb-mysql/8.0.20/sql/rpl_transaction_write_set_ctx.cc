/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_transaction_write_set_ctx.h"

#include <stddef.h>
#include <utility>

#include "m_string.h"
#include "my_dbug.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/service_rpl_transaction_write_set.h"  // Transaction_write_set
#include "sql/current_thd.h"                          // current_thd
#include "sql/debug_sync.h"                           // debug_sync_set_action
#include "sql/mysqld_thd_manager.h"                   // Global_THD_manager
#include "sql/psi_memory_key.h"
#include "sql/sql_class.h"  // THD
#include "sql/transaction_info.h"

Rpl_transaction_write_set_ctx::Rpl_transaction_write_set_ctx()
    : m_has_missing_keys(false), m_has_related_foreign_keys(false) {
  DBUG_TRACE;
  /*
    In order to speed-up small transactions write-set extraction,
    we preallocate 12 elements.
    12 is a sufficient number to hold write-sets for single
    statement transactions, even on tables with foreign keys.
  */
  write_set.reserve(12);
}

void Rpl_transaction_write_set_ctx::add_write_set(uint64 hash) {
  DBUG_TRACE;
  write_set.push_back(hash);
}

std::vector<uint64> *Rpl_transaction_write_set_ctx::get_write_set() {
  DBUG_TRACE;
  return &write_set;
}

void Rpl_transaction_write_set_ctx::clear_write_set() {
  DBUG_TRACE;
  write_set.clear();
  savepoint.clear();
  savepoint_list.clear();
  m_has_missing_keys = m_has_related_foreign_keys = false;
}

void Rpl_transaction_write_set_ctx::set_has_missing_keys() {
  DBUG_TRACE;
  m_has_missing_keys = true;
}

bool Rpl_transaction_write_set_ctx::get_has_missing_keys() {
  DBUG_TRACE;
  return m_has_missing_keys;
}

void Rpl_transaction_write_set_ctx::set_has_related_foreign_keys() {
  DBUG_TRACE;
  m_has_related_foreign_keys = true;
}

bool Rpl_transaction_write_set_ctx::get_has_related_foreign_keys() {
  DBUG_TRACE;
  return m_has_related_foreign_keys;
}

/**
  Implementation of service_rpl_transaction_write_set, see
  @file include/mysql/service_rpl_transaction_write_set.h
*/

Transaction_write_set *get_transaction_write_set(unsigned long m_thread_id) {
  DBUG_TRACE;
  THD *thd = nullptr;
  Transaction_write_set *result_set = nullptr;
  Find_thd_with_id find_thd_with_id(m_thread_id);

  thd = Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
  if (thd) {
    Rpl_transaction_write_set_ctx *transaction_write_set_ctx =
        thd->get_transaction()->get_transaction_write_set_ctx();
    int write_set_size = transaction_write_set_ctx->get_write_set()->size();
    if (write_set_size == 0) {
      mysql_mutex_unlock(&thd->LOCK_thd_data);
      return nullptr;
    }

    result_set = (Transaction_write_set *)my_malloc(
        key_memory_write_set_extraction, sizeof(Transaction_write_set), MYF(0));
    result_set->write_set_size = write_set_size;
    result_set->write_set = (unsigned long long *)my_malloc(
        key_memory_write_set_extraction,
        write_set_size * sizeof(unsigned long long), MYF(0));
    int result_set_index = 0;
    for (std::vector<uint64>::iterator it =
             transaction_write_set_ctx->get_write_set()->begin();
         it != transaction_write_set_ctx->get_write_set()->end(); ++it) {
      uint64 temp = *it;
      result_set->write_set[result_set_index++] = temp;
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }
  return result_set;
}

void Rpl_transaction_write_set_ctx::add_savepoint(char *name) {
  DBUG_TRACE;
  std::string identifier(name);

  DBUG_EXECUTE_IF("transaction_write_set_savepoint_clear_on_commit_rollback", {
    DBUG_ASSERT(savepoint.size() == 0);
    DBUG_ASSERT(write_set.size() == 0);
    DBUG_ASSERT(savepoint_list.size() == 0);
  });

  DBUG_EXECUTE_IF("transaction_write_set_savepoint_level",
                  DBUG_ASSERT(savepoint.size() == 0););

  std::map<std::string, size_t>::iterator it;

  /*
    Savepoint with the same name, the old savepoint is deleted and a new one
    is set
  */
  if ((it = savepoint.find(name)) != savepoint.end()) savepoint.erase(it);

  savepoint.insert(
      std::pair<std::string, size_t>(identifier, write_set.size()));

  DBUG_EXECUTE_IF(
      "transaction_write_set_savepoint_add_savepoint",
      DBUG_ASSERT(savepoint.find(identifier)->second == write_set.size()););
}

void Rpl_transaction_write_set_ctx::del_savepoint(char *name) {
  DBUG_TRACE;
  std::string identifier(name);

  DBUG_EXECUTE_IF("transaction_write_set_savepoint_block_before_release", {
    const char act[] = "now wait_for signal.unblock_release";
    DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
  });

  savepoint.erase(identifier);
}

void Rpl_transaction_write_set_ctx::rollback_to_savepoint(char *name) {
  DBUG_TRACE;
  size_t position = 0;
  std::string identifier(name);
  std::map<std::string, size_t>::iterator elem;

  if ((elem = savepoint.find(identifier)) != savepoint.end()) {
    DBUG_ASSERT(elem->second <= write_set.size());

    DBUG_EXECUTE_IF("transaction_write_set_savepoint_block_before_rollback", {
      const char act[] = "now wait_for signal.unblock_rollback";
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    });

    position = elem->second;

    // Remove all savepoints created after the savepoint identifier given as
    // parameter
    std::map<std::string, size_t>::iterator it = savepoint.begin();
    while (it != savepoint.end()) {
      if (it->second > position)
        savepoint.erase(it++);
      else
        ++it;
    }

    /*
      We need to check that:
       - starting index of the range we want to erase does exist.
       - write_set size have elements to be removed
    */
    if (write_set.size() > 0 && position < write_set.size()) {
      // Clear all elements after savepoint
      write_set.erase(write_set.begin() + position, write_set.end());
    }

    DBUG_EXECUTE_IF("transaction_write_set_savepoint_add_savepoint",
                    DBUG_ASSERT(write_set.size() == 1););

    DBUG_EXECUTE_IF("transaction_write_set_size_2",
                    DBUG_ASSERT(write_set.size() == 2););
  }
}

void Rpl_transaction_write_set_ctx::reset_savepoint_list() {
  DBUG_TRACE;

  savepoint_list.push_back(savepoint);
  savepoint.clear();
}

void Rpl_transaction_write_set_ctx::restore_savepoint_list() {
  DBUG_TRACE;

  if (!savepoint_list.empty()) {
    savepoint = savepoint_list.back();
    savepoint_list.pop_back();
  }
}
