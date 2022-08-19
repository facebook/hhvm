/* Copyright (c) 2001, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* HANDLER ... commands - direct access to the storage engine */

/* TODO:
  HANDLER blabla OPEN [ AS foobar ] [ (column-list) ]

  the most natural (easiest, fastest) way to do it is to
  compute List<Item> field_list not in mysql_ha_read
  but in mysql_ha_open, and then store it in TABLE structure.

  The problem here is that mysql_parse calls free_item to free all the
  items allocated at the end of every query. The workaround would to
  keep two item lists per THD - normal free_list and handler_items.
  The second is to be freeed only on thread end. mysql_ha_open should
  then do { handler_items=concat(handler_items, free_list); free_list=0; }

  But !!! do_command calls free_root at the end of every query and frees up
  all the sql_alloc'ed memory. It's harder to work around...
*/

/*
  The information about open HANDLER objects is stored in a HASH.
  It holds objects of type TABLE_LIST, which are indexed by table
  name/alias, and allows us to quickly find a HANDLER table for any
  operation at hand - be it HANDLER READ or HANDLER CLOSE.

  It also allows us to maintain an "open" HANDLER even in cases
  when there is no physically open cursor. E.g. a FLUSH TABLE
  statement in this or some other connection demands that all open
  HANDLERs against the flushed table are closed. In order to
  preserve the information about an open HANDLER, we don't perform
  a complete HANDLER CLOSE, but only close the TABLE object.  The
  corresponding TABLE_LIST is kept in the cache with 'table'
  pointer set to NULL. The table will be reopened on next access
  (this, however, leads to loss of cursor position, unless the
  cursor points at the first record).
*/

#include "sql/sql_handler.h"

#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <memory>
#include <new>
#include <unordered_map>
#include <utility>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_pointer_arithmetic.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"         // check_table_access
#include "sql/dd/types/abstract_table.h"  // dd::enum_table_type
#include "sql/error_handler.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/key.h"   // key_copy
#include "sql/lock.h"  // mysql_unlock_tables
#include "sql/mdl.h"
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_audit.h"  // mysql_audit_table_access_notify
#include "sql/sql_base.h"   // close_thread_tables
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "sql_string.h"
#include "typelib.h"

#define HANDLER_TABLES_HASH_SIZE 120

static enum_ha_read_modes rkey_to_rnext[] = {
    enum_ha_read_modes::RNEXT_SAME, enum_ha_read_modes::RNEXT,
    enum_ha_read_modes::RPREV,      enum_ha_read_modes::RNEXT,
    enum_ha_read_modes::RPREV,      enum_ha_read_modes::RNEXT,
    enum_ha_read_modes::RPREV,      enum_ha_read_modes::RPREV};

static bool mysql_ha_open_table(THD *thd, TABLE_LIST *table);

/**
  Close a HANDLER table.

  @param thd Thread identifier.
  @param tables A list of tables with the first entry to close.

  @note Though this function takes a list of tables, only the first list entry
  will be closed.
  @note Broadcasts refresh if it closed a table with old version.
*/

static void mysql_ha_close_table(THD *thd, TABLE_LIST *tables) {
  if (tables->table && !tables->table->s->tmp_table) {
    /* Non temporary table. */
    tables->table->file->ha_index_or_rnd_end();
    tables->table->open_by_handler = false;
    close_thread_table(thd, &tables->table);
    thd->mdl_context.release_lock(tables->mdl_request.ticket);
  } else if (tables->table) {
    /* Must be a temporary table */
    TABLE *table = tables->table;
    table->file->ha_index_or_rnd_end();
    table->query_id = thd->query_id;
    table->open_by_handler = false;
    mark_tmp_table_for_reuse(table);
  }

  /* Mark table as closed, ready for re-open if necessary. */
  tables->table = nullptr;
  /* Safety, cleanup the pointer to satisfy MDL assertions. */
  tables->mdl_request.ticket = nullptr;
}

/**
  Execute a HANDLER OPEN statement.

  @param  thd   The current thread.

  @retval false on success.
  @retval true on failure.
*/

bool Sql_cmd_handler_open::execute(THD *thd) {
  TABLE_LIST *hash_tables = nullptr;
  char *db, *name, *alias;
  TABLE_LIST *tables = thd->lex->select_lex->get_table_list();
  DBUG_TRACE;
  DBUG_PRINT("enter", ("'%s'.'%s' as '%s'", tables->db, tables->table_name,
                       tables->alias));

  if (thd->locked_tables_mode) {
    my_error(ER_LOCK_OR_ACTIVE_TRANSACTION, MYF(0));
    return true;
  }
  if (tables->schema_table) {
    my_error(ER_WRONG_USAGE, MYF(0), "HANDLER OPEN",
             INFORMATION_SCHEMA_NAME.str);
    DBUG_PRINT("exit", ("ERROR"));
    return true;
  }

  /*
    We might have a handler with the same name already.

    Note that it is safe to disclose this information before doing privilege
    check. Current user can always find out that handler is open by using
    HANDLER ... READ command, which doesn't requires any privileges.
  */
  if (thd->handler_tables_hash.count(tables->alias) != 0) {
    DBUG_PRINT("info", ("duplicate '%s'", tables->alias));
    DBUG_PRINT("exit", ("ERROR"));
    my_error(ER_NONUNIQ_TABLE, MYF(0), tables->alias);
    return true;
  }

  /* copy the TABLE_LIST struct */
  const size_t db_alloc_len = strlen(tables->db) + 1;
  const size_t name_alloc_len = strlen(tables->table_name) + 1;
  const size_t alias_alloc_len = strlen(tables->alias) + 1;

  if (!(my_multi_malloc(key_memory_THD_handler_tables_hash, MYF(MY_WME),
                        &hash_tables, sizeof(*hash_tables), &db, db_alloc_len,
                        &name, name_alloc_len, &alias, alias_alloc_len,
                        NullS))) {
    DBUG_PRINT("exit", ("ERROR"));
    return true;
  }
  memcpy(db, tables->db, db_alloc_len);
  memcpy(name, tables->table_name, name_alloc_len);
  memcpy(alias, tables->alias, alias_alloc_len);
  /*
    We can't request lock with explicit duration for this table
    right from the start as open_tables() can't handle properly
    back-off for such locks.
  */
  DBUG_ASSERT(tables->table == nullptr);
  new (hash_tables) TABLE_LIST(db, tables->db_length, name,
                               tables->table_name_length, alias, MDL_SHARED);

  /* for now HANDLER can be used only for real TABLES */
  hash_tables->required_type = dd::enum_table_type::BASE_TABLE;
  /* add to hash */
  thd->handler_tables_hash.emplace(alias,
                                   unique_ptr_my_free<TABLE_LIST>(hash_tables));

  if (open_temporary_tables(thd, hash_tables) ||
      check_table_access(thd, SELECT_ACL, hash_tables, false, UINT_MAX,
                         false) ||
      mysql_ha_open_table(thd, hash_tables))

  {
    thd->handler_tables_hash.erase(alias);
    DBUG_PRINT("exit", ("ERROR"));
    return true;
  }

  my_ok(thd);

  DBUG_PRINT("exit", ("OK"));
  return false;
}

/**
  Auxiliary function which opens or re-opens table for HANDLER statements.

  @param thd          Thread context..
  @param hash_tables  Table list element for table to open.

  @retval false - Success.
  @retval true  - Failure.
*/

static bool mysql_ha_open_table(THD *thd, TABLE_LIST *hash_tables) {
  TABLE *backup_open_tables;
  MDL_savepoint mdl_savepoint;
  uint counter;
  bool error;

  DBUG_TRACE;

  DBUG_ASSERT(!thd->locked_tables_mode);

  /*
    Save and reset the open_tables list so that open_tables() won't
    be able to access (or know about) the previous list. And on return
    from open_tables(), thd->open_tables will contain only the opened
    table.

    See open_table() back-off comments for more details.
  */
  backup_open_tables = thd->open_tables;
  thd->set_open_tables(nullptr);
  mdl_savepoint = thd->mdl_context.mdl_savepoint();

  /*
    'hash_tables->table' must be NULL, unless there is pre-opened
    temporary table. open_tables() will set it if successful.
  */
  DBUG_ASSERT(!hash_tables->table || is_temporary_table(hash_tables));

  error = open_tables(thd, &hash_tables, &counter, 0);

  if (!error &&
      !(hash_tables->table->file->ha_table_flags() & HA_CAN_SQL_HANDLER)) {
    my_error(ER_ILLEGAL_HA, MYF(0), hash_tables->alias);
    error = true;
  }
  if (!error && hash_tables->mdl_request.ticket &&
      thd->mdl_context.has_lock(mdl_savepoint,
                                hash_tables->mdl_request.ticket)) {
    /* The ticket returned is within a savepoint. Make a copy.  */
    error = thd->mdl_context.clone_ticket(&hash_tables->mdl_request);
    hash_tables->table->mdl_ticket = hash_tables->mdl_request.ticket;
  }
  if (error) {
    /*
      No need to rollback statement transaction, it's not started.
      If called for re-open, no need to rollback either,
      it will be done at statement end.
    */
    DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT));
    close_thread_tables(thd);
    thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
    thd->set_open_tables(backup_open_tables);
    hash_tables->table = nullptr;
    /* Safety, cleanup the pointer to satisfy MDL assertions. */
    hash_tables->mdl_request.ticket = nullptr;
    DBUG_PRINT("exit", ("ERROR"));
    return true;
  }
  thd->set_open_tables(backup_open_tables);
  if (hash_tables->mdl_request.ticket) {
    thd->mdl_context.set_lock_duration(hash_tables->mdl_request.ticket,
                                       MDL_EXPLICIT);
    thd->mdl_context.set_needs_thr_lock_abort(true);
  }

  /*
    Assert that the above check prevents opening of views and merge tables.
    For temporary tables, TABLE::next can be set even if only one table
    was opened for HANDLER as it is used to link them together
    (see thd->temporary_tables).
  */
  DBUG_ASSERT(hash_tables->table->next == nullptr ||
              hash_tables->table->s->tmp_table);
  /*
    If it's a temp table, don't reset table->query_id as the table is
    being used by this handler. For non-temp tables we use this flag
    in asserts.
  */
  hash_tables->table->open_by_handler = true;

  /*
    Generated column expressions have been resolved using the MEM_ROOT of the
    current HANDLER statement, which is cleared when the statement has finished.
    Clean up the expressions so that subsequent HANDLER ... READ calls don't
    access data allocated on a cleared MEM_ROOT. The generated column
    expressions have to be re-resolved on each HANDLER ... READ call.
  */
  hash_tables->table->cleanup_value_generator_items();

  DBUG_PRINT("exit", ("OK"));
  return false;
}

/**
  Execute a HANDLER CLOSE statement.

  @param  thd   The current thread.

  @note  Closes the table that is associated (on the handler tables hash)
         with the name (TABLE_LIST::alias) of the specified table.

  @retval false on success.
  @retval true on failure.
*/

bool Sql_cmd_handler_close::execute(THD *thd) {
  TABLE_LIST *tables = thd->lex->select_lex->get_table_list();
  DBUG_TRACE;
  DBUG_PRINT("enter", ("'%s'.'%s' as '%s'", tables->db, tables->table_name,
                       tables->alias));

  if (thd->locked_tables_mode) {
    my_error(ER_LOCK_OR_ACTIVE_TRANSACTION, MYF(0));
    return true;
  }
  auto it = thd->handler_tables_hash.find(tables->alias);
  if (it != thd->handler_tables_hash.end()) {
    mysql_ha_close_table(thd, it->second.get());
    thd->handler_tables_hash.erase(it);
  } else {
    my_error(ER_UNKNOWN_TABLE, MYF(0), tables->alias, "HANDLER");
    DBUG_PRINT("exit", ("ERROR"));
    return true;
  }

  /*
    Mark MDL_context as no longer breaking protocol if we have
    closed last HANDLER.
  */
  if (thd->handler_tables_hash.empty())
    thd->mdl_context.set_needs_thr_lock_abort(false);

  my_ok(thd);
  DBUG_PRINT("exit", ("OK"));
  return false;
}

/**
  Execute a HANDLER READ statement.

  @param  thd   The current thread.

  @note  Closes the table that is associated (on the handler tables hash)
         with the name (TABLE_LIST::alias) of the specified table.

  @retval false on success.
  @retval true on failure.
*/

bool Sql_cmd_handler_read::execute(THD *thd) {
  TABLE_LIST *hash_tables = nullptr;
  TABLE *table, *backup_open_tables;
  MYSQL_LOCK *lock;
  List<Item> list;
  Protocol *protocol = thd->get_protocol();
  char buff[MAX_FIELD_WIDTH];
  String buffer(buff, sizeof(buff), system_charset_info);
  int error, keyno = -1;
  uint num_rows;
  uchar *key = nullptr;
  uint key_len = 0;
  MDL_deadlock_and_lock_abort_error_handler sql_handler_lock_error;
  LEX *lex = thd->lex;
  SELECT_LEX *select_lex = lex->select_lex;
  SELECT_LEX_UNIT *unit = lex->unit;
  TABLE_LIST *tables = select_lex->get_table_list();
  enum_ha_read_modes mode = m_read_mode;
  Item *cond = select_lex->where_cond();
  ha_rows select_limit_cnt, offset_limit_cnt;
  MDL_savepoint mdl_savepoint;
  bool res;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("'%s'.'%s' as '%s'", tables->db, tables->table_name,
                       tables->alias));

  if (thd->locked_tables_mode) {
    my_error(ER_LOCK_OR_ACTIVE_TRANSACTION, MYF(0));
    return true;
  }

  /* Accessing data in XA_IDLE or XA_PREPARED is not allowed. */
  if (thd->get_transaction()->xid_state()->check_xa_idle_or_prepared(true))
    return true;

  /*
    There is no need to check for table permissions here, because
    if a user has no permissions to read a table, he won't be
    able to open it (with SQLCOM_HA_OPEN) in the first place.
  */

  /* Get limit counters from SELECT_LEX. */
  unit->prepare_limit(thd, select_lex);
  unit->set_limit(thd, select_lex);
  select_limit_cnt = unit->select_limit_cnt;
  offset_limit_cnt = unit->offset_limit_cnt;

  select_lex->context.resolve_in_table_list_only(tables);
  list.push_front(new Item_field(&select_lex->context, nullptr, nullptr, "*"));
  List_iterator<Item> it(list);
  it++;

retry:
  const auto hash_it = thd->handler_tables_hash.find(tables->alias);
  if (hash_it != thd->handler_tables_hash.end()) {
    hash_tables = hash_it->second.get();
    /*
      Handler interface sometimes uses "tables", sometimes it uses "hash_tables"
      thus we need grant information in both objects.
    */
    tables->grant = hash_tables->grant;
    table = hash_tables->table;

    DBUG_PRINT("info-in-hash",
               ("'%s'.'%s' as '%s' table: %p", hash_tables->db,
                hash_tables->table_name, hash_tables->alias, table));
    if (!table) {
      /*
        The handler table has been closed. Re-open it.
      */
      if (mysql_ha_open_table(thd, hash_tables)) {
        DBUG_PRINT("exit", ("reopen failed"));
        goto err0;
      }

      table = hash_tables->table;
      DBUG_PRINT("info", ("re-opened '%s'.'%s' as '%s' tab %p", hash_tables->db,
                          hash_tables->table_name, hash_tables->alias, table));
    }
  } else
    table = nullptr;

  if (!table) {
    my_error(ER_UNKNOWN_TABLE, MYF(0), tables->alias, "HANDLER");
    goto err0;
  }

  sql_handler_lock_error.init();

  /*
    For non-temporary tables we need to acquire SR lock in order to ensure
    that HANDLER READ is blocked by LOCK TABLES WRITE in other connections
    for storage engines which don't use THR_LOCK locks (e.g. InnoDB).

    To simplify clean-up code we take MDL_savepoint even for temporary tables.
  */
  mdl_savepoint = thd->mdl_context.mdl_savepoint();

  if (hash_tables->table->s->tmp_table == NO_TMP_TABLE) {
    MDL_request read_request;

    MDL_REQUEST_INIT_BY_KEY(&read_request, &hash_tables->mdl_request.key,
                            MDL_SHARED_READ, MDL_TRANSACTION);

    thd->push_internal_handler(&sql_handler_lock_error);

    error = thd->mdl_context.acquire_lock_nsec(
        &read_request, thd->variables.lock_wait_timeout_nsec);
    thd->pop_internal_handler();

    if (sql_handler_lock_error.need_reopen()) {
      /*
        HANDLER READ statement's attempt to upgrade lock on the subject table
        may get aborted if there is a pending DDL. In that case we close the
        table, reopen it, and try to read again.
        This is implicit and obscure, since HANDLER position is lost in the
        process, but it's the legacy server behaviour we should preserve.
      */
      DBUG_ASSERT(error && !thd->is_error());
      mysql_ha_close_table(thd, hash_tables);
      goto retry;
    }

    if (error) goto err0;
  }

  /* save open_tables state */
  backup_open_tables = thd->open_tables;
  /* Always a one-element list, see mysql_ha_open(). */
  DBUG_ASSERT(hash_tables->table->next == nullptr ||
              hash_tables->table->s->tmp_table);
  /*
    mysql_lock_tables() needs thd->open_tables to be set correctly to
    be able to handle aborts properly.
  */
  thd->set_open_tables(hash_tables->table);

  /* Re-use Sql_handler_lock_error instance which was initialized earlier. */
  DBUG_ASSERT(!sql_handler_lock_error.need_reopen());
  thd->push_internal_handler(&sql_handler_lock_error);

  lock = mysql_lock_tables(thd, &thd->open_tables, 1, 0);

  thd->pop_internal_handler();
  /*
    In 5.1 and earlier, mysql_lock_tables() could replace the TABLE
    object with another one (reopen it). This is no longer the case
    with new MDL.
  */
  DBUG_ASSERT(hash_tables->table == thd->open_tables);
  /* Restore previous context. */
  thd->set_open_tables(backup_open_tables);

  if (sql_handler_lock_error.need_reopen()) {
    DBUG_ASSERT(!lock && !thd->is_error());
    /*
      Always close statement transaction explicitly,
      so that the engine doesn't have to count locks.
      There should be no need to perform transaction
      rollback due to deadlock.
    */
    DBUG_ASSERT(!thd->transaction_rollback_request);
    trans_rollback_stmt(thd);
    thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
    mysql_ha_close_table(thd, hash_tables);
    goto retry;
  }

  if (!lock) goto err1;  // mysql_lock_tables() printed error message already

  // Always read all columns
  hash_tables->table->read_set = &hash_tables->table->s->all_set;
  tables->table = hash_tables->table;

  if (cond) {
    /*
      Privilege check not needed since all columns are selected and checked
      by insert_fields().
    */
    Column_privilege_tracker column_privilege(thd, 0);

    if (table->query_id != thd->query_id) cond->cleanup();  // File was reopened
    if ((!cond->fixed && cond->fix_fields(thd, &cond)) || cond->check_cols(1))
      goto err;
  }

  if (m_key_name) {
    keyno = find_type(m_key_name, &table->s->keynames, FIND_TYPE_NO_PREFIX) - 1;
    if (keyno < 0) {
      my_error(ER_KEY_DOES_NOT_EXITS, MYF(0), m_key_name, tables->alias);
      goto err;
    }
    /* Check if the same index involved. */
    if ((uint)keyno != table->file->get_index()) {
      if (mode == enum_ha_read_modes::RNEXT)
        mode = enum_ha_read_modes::RFIRST;
      else if (mode == enum_ha_read_modes::RPREV)
        mode = enum_ha_read_modes::RLAST;
    }
  }

  if (insert_fields(thd, &select_lex->context, tables->db, tables->alias, &it,
                    false))
    goto err;

  DBUG_EXECUTE_IF("simulate_handler_read_failure",
                  DBUG_SET("+d,simulate_net_write_failure"););
  res = thd->send_result_metadata(&list,
                                  Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF);
  DBUG_EXECUTE_IF("simulate_handler_read_failure",
                  DBUG_SET("-d,simulate_net_write_failure"););
  if (res) goto err;

  if (mysql_audit_table_access_notify(thd, hash_tables)) goto err;

  /*
    In ::external_lock InnoDB resets the fields which tell it that
    the handle is used in the HANDLER interface. Tell it again that
    we are using it for HANDLER.
  */

  table->file->init_table_handle_for_HANDLER();

  /*
    Resolve the generated column expressions. They have to be cleaned up before
    returning, since the resolved expressions may point to memory allocated on
    the MEM_ROOT of the current HANDLER ... READ statement, which will be
    cleared when the statement has completed.
  */
  if (table->refix_value_generator_items(thd)) goto err;

  for (num_rows = 0; num_rows < select_limit_cnt;) {
    switch (mode) {
      case enum_ha_read_modes::RNEXT:
        if (m_key_name) {
          if (table->file->inited == handler::INDEX) {
            /* Check if we read from the same index. */
            DBUG_ASSERT((uint)keyno == table->file->get_index());
            error = table->file->ha_index_next(table->record[0]);
            break;
          }
        } else if (table->file->inited == handler::RND) {
          error = table->file->ha_rnd_next(table->record[0]);
          break;
        }
        /*
          Fall through to HANDLER ... READ ... FIRST case if we are trying
          to read next row in index order after starting reading rows in
          natural order, or, vice versa, trying to read next row in natural
          order after reading previous rows in index order.
        */
      case enum_ha_read_modes::RFIRST:
        if (m_key_name) {
          if (!(error = table->file->ha_index_or_rnd_end()) &&
              !(error = table->file->ha_index_init(keyno, true)))
            error = table->file->ha_index_first(table->record[0]);
        } else {
          if (!(error = table->file->ha_index_or_rnd_end()) &&
              !(error = table->file->ha_rnd_init(true)))
            error = table->file->ha_rnd_next(table->record[0]);
        }
        mode = enum_ha_read_modes::RNEXT;
        break;
      case enum_ha_read_modes::RPREV:
        DBUG_ASSERT(m_key_name != nullptr);
        /* Check if we read from the same index. */
        DBUG_ASSERT((uint)keyno == table->file->get_index());
        if (table->file->inited == handler::INDEX) {
          error = table->file->ha_index_prev(table->record[0]);
          break;
        }
        /* else fall through, for more info, see comment before 'case RFIRST'.
         */
      case enum_ha_read_modes::RLAST:
        DBUG_ASSERT(m_key_name != nullptr);
        if (!(error = table->file->ha_index_or_rnd_end()) &&
            !(error = table->file->ha_index_init(keyno, true)))
          error = table->file->ha_index_last(table->record[0]);
        mode = enum_ha_read_modes::RPREV;
        break;
      case enum_ha_read_modes::RNEXT_SAME:
        /* Continue scan on "(keypart1,keypart2,...)=(c1, c2, ...)  */
        DBUG_ASSERT(table->file->inited == handler::INDEX);
        error = table->file->ha_index_next_same(table->record[0], key, key_len);
        break;
      case enum_ha_read_modes::RKEY: {
        DBUG_ASSERT(m_key_name != nullptr);
        KEY *keyinfo = table->key_info + keyno;
        KEY_PART_INFO *key_part = keyinfo->key_part;
        if (m_key_expr->elements > keyinfo->user_defined_key_parts) {
          my_error(ER_TOO_MANY_KEY_PARTS, MYF(0),
                   keyinfo->user_defined_key_parts);
          goto err;
        }
        /*
          Privilege check not needed since all columns are selected and checked
          by insert_fields().
        */
        Column_privilege_tracker column_privilege(thd, 0);

        List_iterator<Item> it_ke(*m_key_expr);
        Item *item;
        key_part_map keypart_map;
        for (keypart_map = key_len = 0; (item = it_ke++); key_part++) {
          my_bitmap_map *old_map;
          // 'item' can be changed by fix_fields() call
          if ((!item->fixed && item->fix_fields(thd, it_ke.ref())) ||
              (item = *it_ke.ref())->check_cols(1))
            goto err;
          if (item->used_tables() & ~RAND_TABLE_BIT) {
            my_error(ER_WRONG_ARGUMENTS, MYF(0), "HANDLER ... READ");
            goto err;
          }
          old_map = dbug_tmp_use_all_columns(table, table->write_set);
          type_conversion_status conv_status =
              item->save_in_field(key_part->field, true);
          dbug_tmp_restore_column_map(table->write_set, old_map);
          /*
            If conversion status is TYPE_ERR_BAD_VALUE or
            TYPE_ERR_NULL_CONSTRAINT_VIOLATION, the target index value
            is not stored into record buffer, so we can't proceed with the
            index search.
          */
          if (conv_status == TYPE_ERR_BAD_VALUE) {
            my_error(ER_WRONG_ARGUMENTS, MYF(0), "HANDLER ... READ");
            goto err;
          }
          if (conv_status == TYPE_ERR_NULL_CONSTRAINT_VIOLATION) {
            my_error(ER_BAD_NULL_ERROR, MYF(0), m_key_name);
            goto err;
          }

          key_len += key_part->store_length;
          keypart_map = (keypart_map << 1) | 1;
        }

        if (!(key = (uchar *)thd->mem_calloc(ALIGN_SIZE(key_len)))) goto err;
        if ((error = table->file->ha_index_or_rnd_end())) break;
        key_copy(key, table->record[0], table->key_info + keyno, key_len);
        if (!(error = table->file->ha_index_init(keyno, true)))
          error = table->file->ha_index_read_map(table->record[0], key,
                                                 keypart_map, m_rkey_mode);
        mode = rkey_to_rnext[(int)m_rkey_mode];
        break;
      }
      default:
        my_error(ER_ILLEGAL_HA, MYF(0));
        goto err;
    }

    if (error) {
      if (error == HA_ERR_RECORD_DELETED) continue;
      if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
        LogErr(ERROR_LEVEL, ER_SQL_HA_READ_FAILED, error, tables->table_name);
        table->file->print_error(error, MYF(0));
        goto err;
      }
      goto ok;
    }
    thd->inc_examined_row_count(1);
    if (cond && !cond->val_int()) {
      if (thd->is_error()) goto err;
      continue;
    }
    if (num_rows >= offset_limit_cnt) {
      protocol->start_row();
      if (thd->send_result_set_row(&list)) goto err;

      if (protocol->end_row()) goto err;
    }
    num_rows++;
    thd->inc_sent_row_count(1);
  }
ok:
  /*
    Always close statement transaction explicitly,
    so that the engine doesn't have to count locks.
  */
  trans_commit_stmt(thd);
  mysql_unlock_tables(thd, lock);
  thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
  table->cleanup_value_generator_items();
  my_eof(thd);
  DBUG_PRINT("exit", ("OK"));
  return false;

err:
  trans_rollback_stmt(thd);
  mysql_unlock_tables(thd, lock);
  table->cleanup_value_generator_items();
err1:
  thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
err0:
  DBUG_PRINT("exit", ("ERROR"));
  return true;
}

/**
  Scan the handler tables hash for matching tables.

  @param thd Thread identifier.
  @param tables The list of tables to remove.

  @return Pointer to head of linked list (TABLE_LIST::next_local) of matching
          TABLE_LIST elements from handler_tables_hash. Otherwise, NULL if no
          table was matched.
*/

static TABLE_LIST *mysql_ha_find(THD *thd, TABLE_LIST *tables) {
  TABLE_LIST *head = nullptr, *first = tables;
  DBUG_TRACE;

  /* search for all handlers with matching table names */
  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *hash_tables = key_and_value.second.get();
    for (tables = first; tables; tables = tables->next_local) {
      if (tables->is_derived()) continue;
      if ((!*tables->get_db_name() ||
           !my_strcasecmp(&my_charset_latin1, hash_tables->get_db_name(),
                          tables->get_db_name())) &&
          !my_strcasecmp(&my_charset_latin1, hash_tables->get_table_name(),
                         tables->get_table_name()))
        break;
    }
    if (tables) {
      hash_tables->next_local = head;
      head = hash_tables;
    }
  }

  return head;
}

/**
  Remove matching tables from the HANDLER's hash table.

  @param thd Thread identifier.
  @param tables The list of tables to remove.

  @note Broadcasts refresh if it closed a table with old version.
*/

void mysql_ha_rm_tables(THD *thd, TABLE_LIST *tables) {
  TABLE_LIST *hash_tables, *next;
  DBUG_TRACE;

  DBUG_ASSERT(tables);

  hash_tables = mysql_ha_find(thd, tables);

  while (hash_tables) {
    next = hash_tables->next_local;
    if (hash_tables->table) mysql_ha_close_table(thd, hash_tables);
    thd->handler_tables_hash.erase(hash_tables->alias);
    hash_tables = next;
  }

  /*
    Mark MDL_context as no longer breaking protocol if we have
    closed last HANDLER.
  */
  if (thd->handler_tables_hash.empty())
    thd->mdl_context.set_needs_thr_lock_abort(false);
}

/**
  Close cursors of matching tables from the HANDLER's hash table.

  @param thd Thread identifier.
  @param all_tables The list of tables to flush.
*/

void mysql_ha_flush_tables(THD *thd, TABLE_LIST *all_tables) {
  DBUG_TRACE;

  for (TABLE_LIST *table_list = all_tables; table_list;
       table_list = table_list->next_global) {
    TABLE_LIST *hash_tables = mysql_ha_find(thd, table_list);
    /* Close all aliases of the same table. */
    while (hash_tables) {
      TABLE_LIST *next_local = hash_tables->next_local;
      if (hash_tables->table) mysql_ha_close_table(thd, hash_tables);
      hash_tables = next_local;
    }
  }
}

/**
  Close cursors on the table from the HANDLER's hash.

  @param thd        Thread context.
  @param db_name    Database name for the table.
  @param table_name Table name.
*/
void mysql_ha_flush_table(THD *thd, const char *db_name,
                          const char *table_name) {
  DBUG_TRACE;

  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *hash_tables = key_and_value.second.get();
    if (!my_strcasecmp(&my_charset_latin1, hash_tables->get_db_name(),
                       db_name) &&
        !my_strcasecmp(&my_charset_latin1, hash_tables->get_table_name(),
                       table_name)) {
      if (hash_tables->table) mysql_ha_close_table(thd, hash_tables);
    }
  }
}

/**
  Flush (close and mark for re-open) all tables that should be should
  be reopen.

  @param thd Thread identifier.

  @note Broadcasts refresh if it closed a table with old version.
*/

void mysql_ha_flush(THD *thd) {
  DBUG_TRACE;

  mysql_mutex_assert_not_owner(&LOCK_open);

  /*
    Don't try to flush open HANDLERs when we're working with
    system tables. The main MDL context is backed up and we can't
    properly release HANDLER locks stored there.
  */
  if (thd->state_flags & Open_tables_state::BACKUPS_AVAIL) return;

  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *hash_tables = key_and_value.second.get();
    /*
      TABLE::mdl_ticket is 0 for temporary tables so we need extra check.
    */
    if (hash_tables->table &&
        ((hash_tables->table->mdl_ticket &&
          hash_tables->table->mdl_ticket->has_pending_conflicting_lock()) ||
         (!hash_tables->table->s->tmp_table &&
          hash_tables->table->s->has_old_version())))
      mysql_ha_close_table(thd, hash_tables);
  }
}

/**
  Remove temporary tables from the HANDLER's hash table. The reason
  for having a separate function, rather than calling
  mysql_ha_rm_tables() is that it is not always feasible (e.g. in
  close_temporary_tables) to obtain a TABLE_LIST containing the
  temporary tables.

  @sa close_temporary_tables
  @param thd Thread identifier.
*/
void mysql_ha_rm_temporary_tables(THD *thd) {
  DBUG_TRACE;

  TABLE_LIST *tmp_handler_tables = nullptr;
  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *handler_table = key_and_value.second.get();

    if (handler_table->table && handler_table->table->s->tmp_table) {
      handler_table->next_local = tmp_handler_tables;
      tmp_handler_tables = handler_table;
    }
  }

  while (tmp_handler_tables) {
    TABLE_LIST *nl = tmp_handler_tables->next_local;
    mysql_ha_close_table(thd, tmp_handler_tables);
    thd->handler_tables_hash.erase(tmp_handler_tables->alias);
    tmp_handler_tables = nl;
  }

  /*
    Mark MDL_context as no longer breaking protocol if we have
    closed last HANDLER.
  */
  if (thd->handler_tables_hash.empty()) {
    thd->mdl_context.set_needs_thr_lock_abort(false);
  }
}

/**
  Close all HANDLER's tables.

  @param thd Thread identifier.

  @note Broadcasts refresh if it closed a table with old version.
*/

void mysql_ha_cleanup(THD *thd) {
  DBUG_TRACE;

  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *hash_tables = key_and_value.second.get();
    if (hash_tables->table) mysql_ha_close_table(thd, hash_tables);
  }

  thd->handler_tables_hash.clear();
}

/**
  Set explicit duration for metadata locks corresponding to open HANDLERs
  to protect them from being released at the end of transaction.

  @param thd Thread identifier.
*/

void mysql_ha_set_explicit_lock_duration(THD *thd) {
  DBUG_TRACE;

  for (const auto &key_and_value : thd->handler_tables_hash) {
    TABLE_LIST *hash_tables = key_and_value.second.get();
    if (hash_tables->table && hash_tables->table->mdl_ticket)
      thd->mdl_context.set_lock_duration(hash_tables->table->mdl_ticket,
                                         MDL_EXPLICIT);
  }
}
