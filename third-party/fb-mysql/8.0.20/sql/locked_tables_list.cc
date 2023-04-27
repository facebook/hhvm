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

#include "sql/locked_tables_list.h"

#include "sql/lock.h"       // mysql_unlock_tables
#include "sql/mysqld.h"     // table_alias_charset
#include "sql/sql_base.h"   // close_thread_tables
#include "sql/sql_class.h"  // THD
#include "sql/table.h"      // TABLE TABLE_LIST

Locked_tables_list::Locked_tables_list()
    : m_locked_tables(nullptr),
      m_locked_tables_last(&m_locked_tables),
      m_reopen_array(nullptr),
      m_locked_tables_count(0) {
  init_sql_alloc(key_memory_locked_table_list, &m_locked_tables_root,
                 MEM_ROOT_BLOCK_SIZE, 0);
}

/**
  Enter LTM_LOCK_TABLES mode.

  Enter the LOCK TABLES mode using all the tables that are
  currently open and locked in this connection.
  Initializes a TABLE_LIST instance for every locked table.

  @param  thd  thread handle

  @return true if out of memory.
*/

bool Locked_tables_list::init_locked_tables(THD *thd) {
  DBUG_ASSERT(thd->locked_tables_mode == LTM_NONE);
  DBUG_ASSERT(m_locked_tables == nullptr);
  DBUG_ASSERT(m_reopen_array == nullptr);
  DBUG_ASSERT(m_locked_tables_count == 0);

  for (TABLE *table = thd->open_tables; table;
       table = table->next, m_locked_tables_count++) {
    TABLE_LIST *src_table_list = table->pos_in_table_list;
    char *db, *table_name, *alias;
    size_t db_len = src_table_list->db_length;
    size_t table_name_len = src_table_list->table_name_length;
    size_t alias_len = strlen(src_table_list->alias);
    TABLE_LIST *dst_table_list;

    if (!multi_alloc_root(&m_locked_tables_root, &dst_table_list,
                          sizeof(*dst_table_list), &db, db_len + 1, &table_name,
                          table_name_len + 1, &alias, alias_len + 1, NullS)) {
      unlock_locked_tables(nullptr);
      return true;
    }

    memcpy(db, src_table_list->db, db_len + 1);
    memcpy(table_name, src_table_list->table_name, table_name_len + 1);
    memcpy(alias, src_table_list->alias, alias_len + 1);
    /**
      Sic: remember the *actual* table level lock type taken, to
      acquire the exact same type in reopen_tables().
      E.g. if the table was locked for write, src_table_list->lock_type is
      TL_WRITE_DEFAULT, whereas reginfo.lock_type has been updated from
      thd->update_lock_default.
    */
    new (dst_table_list)
        TABLE_LIST(table, db, db_len, table_name, table_name_len, alias,
                   src_table_list->table->reginfo.lock_type);

    dst_table_list->mdl_request.ticket = src_table_list->mdl_request.ticket;

    /* Link last into the list of tables */
    *(dst_table_list->prev_global = m_locked_tables_last) = dst_table_list;
    m_locked_tables_last = &dst_table_list->next_global;
    table->pos_in_locked_tables = dst_table_list;
  }
  if (m_locked_tables_count) {
    /**
      Allocate an auxiliary array to pass to mysql_lock_tables()
      in reopen_tables(). reopen_tables() is a critical
      path and we don't want to complicate it with extra allocations.
    */
    m_reopen_array = (TABLE **)m_locked_tables_root.Alloc(
        sizeof(TABLE *) * (m_locked_tables_count + 1));
    if (m_reopen_array == nullptr) {
      unlock_locked_tables(nullptr);
      return true;
    }
  }

  if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
    TX_TRACKER_GET(tst);
    tst->add_trx_state(thd, TX_LOCKED_TABLES);
  }

  thd->enter_locked_tables_mode(LTM_LOCK_TABLES);

  return false;
}

/**
  Leave LTM_LOCK_TABLES mode if it's been entered.

  Close all locked tables, free memory, and leave the mode.

  @note This function is a no-op if we're not in LOCK TABLES.
*/

void Locked_tables_list::unlock_locked_tables(THD *thd)

{
  if (thd) {
    DBUG_ASSERT(!thd->in_sub_stmt &&
                !(thd->state_flags & Open_tables_state::BACKUPS_AVAIL));
    /*
      Sic: we must be careful to not close open tables if
      we're not in LOCK TABLES mode: unlock_locked_tables() is
      sometimes called implicitly, expecting no effect on
      open tables, e.g. from begin_trans().
    */
    if (thd->locked_tables_mode != LTM_LOCK_TABLES) return;

    for (TABLE_LIST *table_list = m_locked_tables; table_list;
         table_list = table_list->next_global) {
      /*
        Clear the position in the list, the TABLE object will be
        returned to the table cache.
      */
      table_list->table->pos_in_locked_tables = nullptr;
    }
    thd->leave_locked_tables_mode();

    if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
      TX_TRACKER_GET(tst);
      tst->clear_trx_state(thd, TX_LOCKED_TABLES);
    }

    DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT));
    close_thread_tables(thd);
    /*
      We rely on the caller to implicitly commit the
      transaction and release transactional locks.
    */
  }
  /*
    After closing tables we can free memory used for storing lock
    request for metadata locks and TABLE_LIST elements.
  */
  free_root(&m_locked_tables_root, MYF(0));
  m_locked_tables = nullptr;
  m_locked_tables_last = &m_locked_tables;
  m_reopen_array = nullptr;
  m_locked_tables_count = 0;
}

/**
  Unlink a locked table from the locked tables list, either
  temporarily or permanently.

  @param  thd        thread handle
  @param  table_list the element of locked tables list.
                     The implementation assumes that this argument
                     points to a TABLE_LIST element linked into
                     the locked tables list. Passing a TABLE_LIST
                     instance that is not part of locked tables
                     list will lead to a crash.
  @param  remove_from_locked_tables
                      true if the table is removed from the list
                      permanently.

  This function is a no-op if we're not under LOCK TABLES.

  @sa Locked_tables_list::reopen_tables()
*/

void Locked_tables_list::unlink_from_list(const THD *thd,
                                          TABLE_LIST *table_list,
                                          bool remove_from_locked_tables) {
  /*
    If mode is not LTM_LOCK_TABLES, we needn't do anything. Moreover,
    outside this mode pos_in_locked_tables value is not trustworthy.
  */
  if (thd->locked_tables_mode != LTM_LOCK_TABLES) return;

  /*
    table_list must be set and point to pos_in_locked_tables of some
    table.
  */
  DBUG_ASSERT(table_list->table->pos_in_locked_tables == table_list);

  /* Clear the pointer, the table will be returned to the table cache. */
  table_list->table->pos_in_locked_tables = nullptr;

  /* Mark the table as closed in the locked tables list. */
  table_list->table = nullptr;

  /*
    If the table is being dropped or renamed, remove it from
    the locked tables list (implicitly drop the LOCK TABLES lock
    on it).
  */
  if (remove_from_locked_tables) {
    *table_list->prev_global = table_list->next_global;
    if (table_list->next_global == nullptr)
      m_locked_tables_last = table_list->prev_global;
    else
      table_list->next_global->prev_global = table_list->prev_global;
  }
}

/**
  This is an attempt to recover (somewhat) in case of an error.
  If we failed to reopen a closed table, let's unlink it from the
  list and forget about it. From a user perspective that would look
  as if the server "lost" the lock on one of the locked tables.

  @note This function is a no-op if we're not under LOCK TABLES.
*/

void Locked_tables_list::unlink_all_closed_tables(THD *thd, MYSQL_LOCK *lock,
                                                  size_t reopen_count) {
  /* If we managed to take a lock, unlock tables and free the lock. */
  if (lock) mysql_unlock_tables(thd, lock);
  /*
    If a failure happened in reopen_tables(), we may have succeeded
    reopening some tables, but not all.
    This works when the connection was killed in mysql_lock_tables().
  */
  if (reopen_count) {
    while (reopen_count--) {
      /*
        When closing the table, we must remove it
        from thd->open_tables list.
        We rely on the fact that open_table() that was used
        in reopen_tables() always links the opened table
        to the beginning of the open_tables list.
      */
      DBUG_ASSERT(thd->open_tables == m_reopen_array[reopen_count]);

      thd->open_tables->pos_in_locked_tables->table = nullptr;

      close_thread_table(thd, &thd->open_tables);
    }
  }
  /* Exclude all closed tables from the LOCK TABLES list. */
  for (TABLE_LIST *table_list = m_locked_tables; table_list;
       table_list = table_list->next_global) {
    if (table_list->table == nullptr) {
      /* Unlink from list. */
      *table_list->prev_global = table_list->next_global;
      if (table_list->next_global == nullptr)
        m_locked_tables_last = table_list->prev_global;
      else
        table_list->next_global->prev_global = table_list->prev_global;
    }
  }
}

/**
  Reopen the tables locked with LOCK TABLES and temporarily closed
  by a DDL statement or FLUSH TABLES.

  @note This function is a no-op if we're not under LOCK TABLES.

  @return true if an error reopening the tables. May happen in
               case of some fatal system error only, e.g. a disk
               corruption, out of memory or a serious bug in the
               locking.
*/

bool Locked_tables_list::reopen_tables(THD *thd) {
  Open_table_context ot_ctx(thd, MYSQL_OPEN_REOPEN);
  size_t reopen_count = 0;
  MYSQL_LOCK *lock;
  MYSQL_LOCK *merged_lock;

  /*
    DDL statements routinely call this method after reporting error.
    OTOH some code (e.g. fix_partitioning_func()) which is invoked
    while opening tables might fail in the presence of error status.
    To avoid problems we hide error status by installing temporary DA.
  */
  Diagnostics_area tmp_da(false);
  thd->push_diagnostics_area(&tmp_da, false);

  for (TABLE_LIST *table_list = m_locked_tables; table_list;
       table_list = table_list->next_global) {
    if (table_list->table) /* The table was not closed */
      continue;

    /* Links into thd->open_tables upon success */
    if (open_table(thd, table_list, &ot_ctx)) {
      unlink_all_closed_tables(thd, nullptr, reopen_count);
      thd->pop_diagnostics_area();
      if (!thd->get_stmt_da()->is_error() && tmp_da.is_error()) {
        // Copy the exception condition information.
        thd->get_stmt_da()->set_error_status(tmp_da.mysql_errno(),
                                             tmp_da.message_text(),
                                             tmp_da.returned_sqlstate());
      }
      thd->get_stmt_da()->copy_sql_conditions_from_da(thd, &tmp_da);
      return true;
    }
    table_list->table->pos_in_locked_tables = table_list;
    /* See also the comment on lock type in init_locked_tables(). */
    table_list->table->reginfo.lock_type = table_list->lock_descriptor().type;

    DBUG_ASSERT(reopen_count < m_locked_tables_count);
    m_reopen_array[reopen_count++] = table_list->table;
  }

  thd->pop_diagnostics_area();

  if (reopen_count) {
    thd->in_lock_tables = true;
    /*
      We re-lock all tables with mysql_lock_tables() at once rather
      than locking one table at a time because of the case
      reported in Bug#45035: when the same table is present
      in the list many times, thr_lock.c fails to grant READ lock
      on a table that is already locked by WRITE lock, even if
      WRITE lock is taken by the same thread. If READ and WRITE
      lock are passed to thr_lock.c in the same list, everything
      works fine. Patching legacy code of thr_lock.c is risking to
      break something else.
    */
    lock =
        mysql_lock_tables(thd, m_reopen_array, reopen_count, MYSQL_OPEN_REOPEN);
    thd->in_lock_tables = false;
    if (lock == nullptr ||
        (merged_lock = mysql_lock_merge(thd->lock, lock)) == nullptr) {
      unlink_all_closed_tables(thd, lock, reopen_count);
      if (!thd->killed) my_error(ER_LOCK_DEADLOCK, MYF(0));
      return true;
    }
    thd->lock = merged_lock;
  }
  return false;
}

/**
  Update database and table names of table locked with LOCK TABLES after
  table rename.

  @param old_table_list     Table list element representing old db/table name.
  @param new_db             Table's new database.
  @param new_table_name     Table's new name.
  @param target_mdl_ticket  Ticket representing metadata lock acquired on new
                            table name.

  @note This function is a no-op if we're not under LOCK TABLES.
*/

void Locked_tables_list::rename_locked_table(TABLE_LIST *old_table_list,
                                             const char *new_db,
                                             const char *new_table_name,
                                             MDL_ticket *target_mdl_ticket) {
  for (TABLE_LIST *table_list = m_locked_tables; table_list;
       table_list = table_list->next_global) {
    if (my_strcasecmp(table_alias_charset, table_list->db,
                      old_table_list->db) == 0 &&
        my_strcasecmp(table_alias_charset, table_list->table_name,
                      old_table_list->table_name) == 0) {
      DBUG_ASSERT(table_list->table == nullptr);

      /*
        Update TABLE_LIST element with new db and name. Allocate
        them on Locked_tables_list private memory root.
      */
      size_t new_db_len = strlen(new_db);
      size_t new_table_name_len = strlen(new_table_name);
      const char *new_db_root =
          strmake_root(&m_locked_tables_root, new_db, new_db_len);
      const char *new_table_name_root = strmake_root(
          &m_locked_tables_root, new_table_name, new_table_name_len);

      if (new_db_root != nullptr && new_table_name_root != nullptr) {
        TABLE_LIST *save_next_global = table_list->next_global;
        TABLE_LIST **save_prev_global = table_list->prev_global;

        /*
          If explicit alias was used in LOCK TABLES then it makes sense
          to preserve it after rename. We might have several instances of
          the same table locked in different modes, so alias is useful to
          differentiate between them.
        */
        bool real_alias =
            my_strcasecmp(table_alias_charset, table_list->table_name,
                          table_list->alias) != 0;

        *table_list = TABLE_LIST(
            new_db_root, new_db_len, new_table_name_root, new_table_name_len,
            real_alias ? table_list->alias : new_table_name_root,
            table_list->lock_descriptor().type);

        table_list->mdl_request.ticket = target_mdl_ticket;
        table_list->next_global = save_next_global;
        table_list->prev_global = save_prev_global;
      } else {
        // OOM. We just unlink table from the list of locked tables.
        *table_list->prev_global = table_list->next_global;
        if (table_list->next_global == nullptr)
          m_locked_tables_last = table_list->prev_global;
        else
          table_list->next_global->prev_global = table_list->prev_global;
      }
    }
  }
}

void Locked_tables_list::add_rename_tablespace_mdls(MDL_ticket *src,
                                                    MDL_ticket *dst) {
  DBUG_ASSERT(m_locked_tables != nullptr);
  DBUG_ASSERT(dst->get_duration() == MDL_TRANSACTION);
  m_rename_tablespace_mdls.push_back({src, dst});
}

namespace {
/*
  For purposes of Locked_tables_list::adjust_renamed_tablespace_mdls()
  we need to treat two tickets belonging to the same lock as equal.
*/
struct MDL_ticket_same_lock_hash {
  size_t operator()(const MDL_ticket *ticket) const {
    return std::hash<MDL_lock *>()(ticket->get_lock());
  }
};

struct MDL_ticket_same_lock_eq {
  bool operator()(const MDL_ticket *a, const MDL_ticket *b) const {
    return a->get_lock() == b->get_lock();
  }
};
}  // namespace

void Locked_tables_list::adjust_renamed_tablespace_mdls(MDL_context *mctx) {
  /*
    Iterate through MDLs on renamed tablespaces and figure out which
    should be released and which should be kept.
    We can't simply release all locks on source and keep locks on destination
    as it won't work correctly when the same name is used multiple times within
    the same RENAME TABLES.
  */
  std::unordered_set<MDL_ticket *, MDL_ticket_same_lock_hash,
                     MDL_ticket_same_lock_eq>
      to_release, to_keep;

  for (auto &mp : m_rename_tablespace_mdls) {
    to_release.insert(mp.m_src);
    to_keep.erase(mp.m_src);
    to_keep.insert(mp.m_dst);
    to_release.erase(mp.m_dst);
  }
  m_rename_tablespace_mdls.clear();

  for (MDL_ticket *t : to_release) mctx->release_all_locks_for_name(t);

  for (MDL_ticket *t : to_keep) {
    mctx->set_lock_duration(t, MDL_EXPLICIT);
    t->downgrade_lock(MDL_INTENTION_EXCLUSIVE);
  }
}
