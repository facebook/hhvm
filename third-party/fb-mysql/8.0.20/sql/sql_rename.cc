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

/**
  @file sql/sql_rename.cc
  Atomic rename of table;  RENAME TABLE t1 to t2, tmp to t1 [,...]
*/

#include "sql/sql_rename.h"

#include <string.h>
#include <set>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_murmur3.h"
#include "my_sys.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_table.h"                 // dd::table_storage_engine
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/types/abstract_table.h"     // dd::Abstract_table
#include "sql/dd/types/table.h"              // dd::Table
#include "sql/dd_sql_view.h"                 // View_metadata_updater
#include "sql/derror.h"                      // ER_THD
#include "sql/handler.h"
#include "sql/log.h"          // query_logger
#include "sql/mysqld.h"       // lower_case_table_names
#include "sql/sp_cache.h"     // sp_cache_invalidate
#include "sql/sql_base.h"     // tdc_remove_table,
                              // lock_table_names,
#include "sql/sql_class.h"    // THD
#include "sql/sql_handler.h"  // mysql_ha_rm_tables
#include "sql/sql_table.h"    // write_bin_log,
                              // build_table_filename
#include "sql/sql_trigger.h"  // change_trigger_table_name
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit_stmt

namespace dd {
class Schema;
}  // namespace dd

typedef std::set<handlerton *> post_ddl_htons_t;

static TABLE_LIST *rename_tables(
    THD *thd, TABLE_LIST *table_list, bool *int_commit_done,
    post_ddl_htons_t *post_ddl_htons,
    Foreign_key_parents_invalidator *fk_invalidator);

static TABLE_LIST *reverse_table_list(TABLE_LIST *table_list);

namespace {
struct table_list_hash {
  size_t operator()(const TABLE_LIST *table) const {
    return static_cast<size_t>(murmur3_32(table->mdl_request.key.ptr(),
                                          table->mdl_request.key.length(), 0));
  }
};

struct table_list_equal {
  bool operator()(const TABLE_LIST *a, const TABLE_LIST *b) const {
    return a->mdl_request.key.is_equal(&b->mdl_request.key);
  }
};
}  // namespace

/**
  Check if connection owns SNRW metadata lock on table or view.
  Report apropriate error if not.

  @note Unlike find_table_for_mdl_upgrade() this call can handle views.
*/

static bool check_if_owns_upgradable_mdl(THD *thd, const char *db,
                                         const char *table_name) {
  if (thd->mdl_context.owns_equal_or_stronger_lock(
          MDL_key::TABLE, db, table_name, MDL_SHARED_NO_READ_WRITE))
    return false;  // Success.

  if (thd->mdl_context.owns_equal_or_stronger_lock(
          MDL_key::TABLE, db, table_name, MDL_SHARED_READ_ONLY))
    my_error(ER_TABLE_NOT_LOCKED_FOR_WRITE, MYF(0), table_name);
  else
    my_error(ER_TABLE_NOT_LOCKED, MYF(0), table_name);
  return true;  // Failure.
}

/**
  Find metadata lock request for table's schema in the set of schema
  requests and set duration of corresponding lock to explicit.

  @note We assume that there are no duplicate schemata in schema_reqs
        array.
*/

static void find_and_set_explicit_duration_for_schema_mdl(
    THD *thd, TABLE_LIST *table,
    Prealloced_array<MDL_request *, 1> *schema_reqs) {
  auto same_db = [table](const MDL_request *mdl_request) {
    return table->db_length == mdl_request->key.db_name_length() &&
           memcmp(table->db, mdl_request->key.db_name(), table->db_length) == 0;
  };

  auto sch_req =
      std::find_if(schema_reqs->begin(), schema_reqs->end(), same_db);

  if (sch_req != schema_reqs->end()) {
    thd->mdl_context.set_lock_duration((*sch_req)->ticket, MDL_EXPLICIT);
    /*
      Remove found request by replacing it with the last one.

      This is necessary to avoid setting duration for the same schema ticket
      twice and also to speed up further calls to this function.
    */
    *sch_req = schema_reqs->back();
    schema_reqs->pop_back();
  } else {
    // We must have handled this schema already.
  }
}

/**
  Rename tables from the list.

  @param thd          Thread context.
  @param table_list   Every two entries in the table_list form
                      a pair of original name and the new name.

  @return True - on failure, false - on success.
*/

bool mysql_rename_tables(THD *thd, TABLE_LIST *table_list) {
  TABLE_LIST *ren_table = nullptr;
  DBUG_TRACE;

  mysql_ha_rm_tables(thd, table_list);

  /*
    The below Auto_releaser allows to keep uncommitted versions of data-
    dictionary objects cached in the Dictionary_client for the whole duration
    of the statement.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (query_logger.is_log_table_enabled(QUERY_LOG_GENERAL) ||
      query_logger.is_log_table_enabled(QUERY_LOG_SLOW)) {
    int to_table;
    const char *rename_log_table[2] = {nullptr, nullptr};

    /*
      Rules for rename of a log table:

      IF   1. Log tables are enabled
      AND  2. Rename operates on the log table and nothing is being
              renamed to the log table.
      DO   3. Throw an error message.
      ELSE 4. Perform rename.
    */

    for (to_table = 0, ren_table = table_list; ren_table;
         to_table = 1 - to_table, ren_table = ren_table->next_local) {
      int log_table_rename = 0;

      if ((log_table_rename =
               query_logger.check_if_log_table(ren_table, true))) {
        /*
          as we use log_table_rename as an array index, we need it to start
          with 0, while QUERY_LOG_SLOW == 1 and QUERY_LOG_GENERAL == 2.
          So, we shift the value to start with 0;
        */
        log_table_rename--;
        if (rename_log_table[log_table_rename]) {
          if (to_table)
            rename_log_table[log_table_rename] = nullptr;
          else {
            /*
              Two renames of "log_table TO" w/o rename "TO log_table" in
              between.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          }
        } else {
          if (to_table) {
            /*
              Attempt to rename a table TO log_table w/o renaming
              log_table TO some table.
            */
            my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), ren_table->table_name,
                     ren_table->table_name);
            return true;
          } else {
            /* save the name of the log table to report an error */
            rename_log_table[log_table_rename] = ren_table->table_name;
          }
        }
      }
    }
    if (rename_log_table[0] || rename_log_table[1]) {
      if (rename_log_table[0])
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[0],
                 rename_log_table[0]);
      else
        my_error(ER_CANT_RENAME_LOG_TABLE, MYF(0), rename_log_table[1],
                 rename_log_table[1]);
      return true;
    }
  }

  /*
    Array in which pointers to MDL requests for acquired schema locks are
    stored. Each schema can be present in this array only once.
  */
  Prealloced_array<MDL_request *, 1> schema_reqs(PSI_INSTRUMENT_ME);

  if (thd->locked_tables_mode) {
    /*
      LOCK TABLES case.

      Check that tables to be renamed are locked for WRITE. Take into
      account that name of table to be renamed might be result of some
      previous step in multi-step RENAME TABLES.

      In theory, we could disregard whether they locked or not and just try
      to acquire exclusive metadata locks on them, but this is too deadlock
      prone.

      Most probably, there is no tables which correspond to target table
      names, so similar check doesn't make sense for them.

      In theory, we can reduce chance of MDL deadlocks by also checking at
      this stage that all child and parent tables for FKs in which tables
      to be renamed participate are locked for WRITE (as we will have to
      acquire to exclusive MDLs on these tables later).
      But this is, probably, too severe restriction which will make
      RENAMES TABLES under LOCK TABLES hard to use in 3rd-party online
      ALTER TABLE tools.
    */
    malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
        new_names(PSI_INSTRUMENT_ME);

    TABLE_LIST *new_table;
    for (ren_table = table_list; ren_table; ren_table = new_table->next_local) {
      new_table = ren_table->next_local;

      auto new_name_it = new_names.find(ren_table);
      if (new_name_it == new_names.end()) {
        if (check_if_owns_upgradable_mdl(thd, ren_table->db,
                                         ren_table->table_name))
          return true;
      } else {
        new_names.erase(new_name_it);
      }
      new_names.insert(new_table);
    }

    /*
      Now proceed to acquiring exclusive metadata locks on both source and
      target table names as well as necessary schema, global and backup locks.
      Since we already have SNRW locks on source table names, we, in fact, are
      upgrading locks for them.
    */
  }

  if (lock_table_names_nsec(thd, table_list, nullptr,
                            thd->variables.lock_wait_timeout_nsec, 0,
                            &schema_reqs) ||
      lock_trigger_names(thd, table_list))
    return true;

  const dd::Table *table_def = nullptr;
  for (TABLE_LIST *table = table_list; table && table->next_local;
       table = table->next_local) {
    if (thd->dd_client()->acquire(table->db, table->table_name, &table_def)) {
      return true;
    }
    if (table_def && table_def->hidden() == dd::Abstract_table::HT_HIDDEN_SE) {
      my_error(ER_NO_SUCH_TABLE, MYF(0), table->db, table->table_name);
      return true;
    }
  }

  for (ren_table = table_list; ren_table; ren_table = ren_table->next_local) {
    if (thd->locked_tables_mode)
      close_all_tables_for_name(thd, ren_table->db, ren_table->table_name,
                                false);
    else
      tdc_remove_table(thd, TDC_RT_REMOVE_ALL, ren_table->db,
                       ren_table->table_name, false);
  }
  bool error = false;
  bool int_commit_done = false;
  /*
    Indicates whether we managed fully revert non-atomic RENAME TABLES
    after the failure.
  */
  bool int_commit_full_revert = false;
  std::set<handlerton *> post_ddl_htons;
  Foreign_key_parents_invalidator fk_invalidator;
  /*
    An exclusive lock on table names is satisfactory to ensure
    no other thread accesses this table.
  */
  if ((ren_table = rename_tables(thd, table_list, &int_commit_done,
                                 &post_ddl_htons, &fk_invalidator))) {
    /* Rename didn't succeed;  rename back the tables in reverse order */
    TABLE_LIST *table;

    if (int_commit_done) {
      /* Reverse the table list */
      table_list = reverse_table_list(table_list);

      /* Find the last renamed table */
      for (table = table_list; table->next_local != ren_table;
           table = table->next_local->next_local)
        ;
      table = table->next_local->next_local;  // Skip error table

      /*
        Revert to old names. In 5.7 we have ignored most of errors occurring
        in the process. However, this looks like a risky idea -- by ignoring
        errors we are likely to end up in some awkward state and not going to
        restore status quo ante.

        So starting from 8.0 we chose to abort reversal on the first failure.
        We will still end up in some awkward case in this case but at least
        no additional damage will be done. Note that since InnoDB tables are
        new default and this engine supports atomic DDL, non-atomic RENAME
        TABLES, which this code deals with, is not the main use case anyway.
      */
      int_commit_full_revert = !rename_tables(thd, table, &int_commit_done,
                                              &post_ddl_htons, &fk_invalidator);

      /* Revert the table list (for prepared statements) */
      table_list = reverse_table_list(table_list);
    }

    error = true;
  }

  if (!error) {
    error = write_bin_log(thd, true, thd->query().str, thd->query().length,
                          !int_commit_done);
  }

  if (!error) {
    Uncommitted_tables_guard uncommitted_tables(thd);

    for (ren_table = table_list; ren_table;
         ren_table = ren_table->next_local->next_local) {
      TABLE_LIST *new_table = ren_table->next_local;
      DBUG_ASSERT(new_table);

      uncommitted_tables.add_table(ren_table);
      uncommitted_tables.add_table(new_table);

      if ((error = update_referencing_views_metadata(
               thd, ren_table, new_table->db, new_table->table_name,
               int_commit_done, &uncommitted_tables)))
        break;
    }
  }

  if (!error && !int_commit_done) {
    error = (trans_commit_stmt(thd) || trans_commit_implicit(thd));

    if (!error) {
      /*
        Don't try to invalidate foreign key parents on error,
        as we might miss necessary locks on them.
      */
      fk_invalidator.invalidate(thd);
    }
  }

  if (error) {
    trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    trans_rollback(thd);
  }

  for (handlerton *hton : post_ddl_htons) hton->post_ddl(thd);

  if (thd->locked_tables_mode) {
    if (!error) {
      /*
        Adjust locked tables list and reopen tables under new names.
        Also calculate sets of metadata locks to release (on old table
        names) and to keep until UNLOCK TABLES (on new table names).

        In addition to keeping locks on tables we also do the same for
        schemas in order to keep set of metadata locks consistent with
        one acquired by LOCK TABLES. We don't release locks on old table
        schemas as it is non-trivial to figure out which locks can be
        released.

        Tablespaces do not need special handling though, as metadata locks
        on them are acquired at LOCK TABLES time and are unaffected by
        RENAME TABLES.
      */
      malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
          to_release(PSI_INSTRUMENT_ME), to_keep(PSI_INSTRUMENT_ME);
      TABLE_LIST *new_table;
      for (ren_table = table_list; ren_table;
           ren_table = new_table->next_local) {
        new_table = ren_table->next_local;
        thd->locked_tables_list.rename_locked_table(
            ren_table, new_table->db, new_table->table_name,
            new_table->mdl_request.ticket);
        to_release.insert(ren_table);
        to_keep.erase(ren_table);
        to_keep.insert(new_table);
        to_release.erase(new_table);
      }

      error = thd->locked_tables_list.reopen_tables(thd);

      for (TABLE_LIST *t : to_release) {
        // Also releases locks with EXPLICIT duration for the same name.
        thd->mdl_context.release_all_locks_for_name(t->mdl_request.ticket);
      }

      for (TABLE_LIST *t : to_keep) {
        thd->mdl_context.set_lock_duration(t->mdl_request.ticket, MDL_EXPLICIT);
        t->mdl_request.ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
        find_and_set_explicit_duration_for_schema_mdl(thd, t, &schema_reqs);
      }
    } else if (!int_commit_done || int_commit_full_revert) {
      /*
        Error happened but all (actually not quite all, see below) changes
        were reverted. We just need to reopen tables.

        Since changes were reverted no additional metadata locks need to
        be kept after statement end. All additional locks acquired by
        this statement will be released automatically at its end, since
        they have transactional duration.

        In case of non-atomic RENAME TABLE previously orphan foreign keys
        which got new parents will keep these parents after reversal, but
        this is not important in this context.
      */
      thd->locked_tables_list.reopen_tables(thd);
    } else {
      /*
        Error happened and we failed to revert all changes. We simply close
        all tables involved.
      */
      thd->locked_tables_list.unlink_all_closed_tables(thd, nullptr, 0);
      /*
        We need to keep metadata locks on both old and new table names
        to avoid breaking foreign key invariants for LOCK TABLES.
        So we set duration of locks on new names to explicit and downgrade
        them from X to SNRW metadata locks. Also keep locks for new schemas.

        Prune list of duplicates first as setting explicit duration for the
        same MDL ticket twice is disallowed.
      */
      malloc_unordered_set<TABLE_LIST *, table_list_hash, table_list_equal>
          to_keep(PSI_INSTRUMENT_ME);
      TABLE_LIST *new_table;
      for (ren_table = table_list; ren_table;
           ren_table = new_table->next_local) {
        new_table = ren_table->next_local;
        to_keep.insert(new_table);
      }
      for (TABLE_LIST *t : to_keep) {
        thd->mdl_context.set_lock_duration(t->mdl_request.ticket, MDL_EXPLICIT);
        t->mdl_request.ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
        find_and_set_explicit_duration_for_schema_mdl(thd, t, &schema_reqs);
      }
    }
  }

  if (!error) my_ok(thd);

  return error;
}

/*
  reverse table list

  SYNOPSIS
    reverse_table_list()
    table_list pointer to table _list

  RETURN
    pointer to new (reversed) list
*/
static TABLE_LIST *reverse_table_list(TABLE_LIST *table_list) {
  TABLE_LIST *prev = nullptr;

  while (table_list) {
    TABLE_LIST *next = table_list->next_local;
    table_list->next_local = prev;
    prev = table_list;
    table_list = next;
  }
  return (prev);
}

/**
  Rename a single table or a view.

  @param[in]      thd               Thread handle.
  @param[in]      ren_table         A table/view to be renamed.
  @param[in]      new_db            The database to which the
                                    table to be moved to.
  @param[in]      new_table_name    The new table/view name.
  @param[in]      new_table_alias   The new table/view alias.
  @param[in,out]  int_commit_done   Whether intermediate commits
                                    were done.
  @param[in,out]  post_ddl_htons    Set of SEs supporting atomic DDL
                                    for which post-DDL hooks needs
                                    to be called.
  @param[in,out]  fk_invalidator    Object keeping track of which
                                    dd::Table objects to invalidate.

  @note Unless int_commit_done is true failure of this call requires
        rollback of transaction before doing anything else.

  @return False on success, True if rename failed.
*/

static bool do_rename(THD *thd, TABLE_LIST *ren_table, const char *new_db,
                      const char *new_table_name, const char *new_table_alias,
                      bool *int_commit_done,
                      std::set<handlerton *> *post_ddl_htons,
                      Foreign_key_parents_invalidator *fk_invalidator) {
  const char *new_alias = new_table_name;
  const char *old_alias = ren_table->table_name;

  DBUG_TRACE;

  if (lower_case_table_names == 2) {
    old_alias = ren_table->alias;
    new_alias = new_table_alias;
  }
  DBUG_ASSERT(new_alias);

  // Fail if the target table already exists
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *from_schema = nullptr;
  const dd::Schema *to_schema = nullptr;
  dd::Abstract_table *from_at = nullptr;
  const dd::Abstract_table *to_table = nullptr;
  if (thd->dd_client()->acquire(ren_table->db, &from_schema) ||
      thd->dd_client()->acquire(new_db, &to_schema) ||
      thd->dd_client()->acquire(new_db, new_alias, &to_table) ||
      thd->dd_client()->acquire_for_modification(
          ren_table->db, ren_table->table_name, &from_at))
    return true;

  if (to_table != nullptr) {
    my_error(ER_TABLE_EXISTS_ERROR, MYF(0), new_alias);
    return true;
  }

  if (from_schema == nullptr) {
    my_error(ER_BAD_DB_ERROR, MYF(0), ren_table->db);
    return true;
  }

  if (to_schema == nullptr) {
    my_error(ER_BAD_DB_ERROR, MYF(0), new_db);
    return true;
  }

  if (from_at == nullptr) {
    my_error(ER_NO_SUCH_TABLE, MYF(0), ren_table->db, old_alias);
    return true;
  }

  // So here we know the source table exists and the target table does
  // not exist. Next is to act based on the table type.
  switch (from_at->type()) {
    case dd::enum_table_type::BASE_TABLE: {
      handlerton *hton = nullptr;
      dd::Table *from_table = dynamic_cast<dd::Table *>(from_at);
      // If the engine is not found, my_error() has already been called
      if (dd::table_storage_engine(thd, from_table, &hton)) return true;

      if ((hton->flags & HTON_SUPPORTS_ATOMIC_DDL) && (hton->post_ddl))
        post_ddl_htons->insert(hton);

      if (check_table_triggers_are_not_in_the_same_schema(ren_table->db,
                                                          *from_table, new_db))
        return true;

      // The below code assumes that only SE capable of atomic DDL support FK.
      DBUG_ASSERT(!(hton->flags & HTON_SUPPORTS_FOREIGN_KEYS) ||
                  (hton->flags & HTON_SUPPORTS_ATOMIC_DDL));

      /*
        If we are performing rename with intermediate commits then
        invalidation of foreign key parents should have happened
        already, right after commit. Code below dealing with failure to
        acquire locks on parent and child tables relies on this
        invariant.
      */
      DBUG_ASSERT(!(*int_commit_done) || fk_invalidator->is_empty());

      // Find if table uses general tablespace and is it encrypted.
      bool is_general_tablespace = false;
      bool is_table_encrypted = false;
      dd::Encrypt_result new_er =
          dd::is_tablespace_encrypted(thd, *from_table, &is_general_tablespace);
      if (new_er.error) {
        return true;
      }
      is_table_encrypted = new_er.value;
      if (!is_general_tablespace &&
          from_table->options().exists("encrypt_type")) {
        dd::String_type et;
        (void)from_table->options().get("encrypt_type", &et);
        DBUG_ASSERT(et.empty() == false);
        is_table_encrypted = is_encrypted(et);
      }

      /*
        Check if we are allowed to move the table, if destination schema
        is changed and its default encryption differs from tables
        encryption type.
      */
      if (from_schema->id() != to_schema->id() &&
          to_schema->default_encryption() != is_table_encrypted) {
        if (opt_table_encryption_privilege_check) {
          if (check_table_encryption_admin_access(thd)) {
            my_error(ER_CANNOT_SET_TABLE_ENCRYPTION, MYF(0));
            return true;
          }
        } else if (to_schema->default_encryption() && !is_table_encrypted) {
          push_warning(thd, Sql_condition::SL_WARNING,
                       WARN_UNENCRYPTED_TABLE_IN_ENCRYPTED_DB,
                       ER_THD(thd, WARN_UNENCRYPTED_TABLE_IN_ENCRYPTED_DB));
        }
      }

      /*
        Obtain exclusive metadata lock on all tables being referenced by the
        old table, since these tables must be invalidated to force a cache miss
        on next acquisition, in order to refresh their FK information.

        Also lock all tables referencing the old table. The FK information in
        these tables must be updated to refer to the new table name.

        And also lock all tables referencing the new table. The FK information
        in these tables must be updated to refer to the (possibly) new
        unique index name.

        TODO: Long-term we should consider acquiring these locks in
              mysql_rename_tables() together with locks on other tables.
              This should decrease probability of deadlock and improve
              crash-safety for RENAME TABLES which mix InnoDB and non-InnoDB
              tables (as all waiting will happen before any changes to SEs).
      */
      if (hton->flags & HTON_SUPPORTS_FOREIGN_KEYS) {
        /*
          If we are under LOCK TABLES check that all previously orphan tables
          which reference new table  name through foreign keys are locked for
          write. Otherwise this ALTER will leave after itself parent table
          locked for WRITE without child tables locked for WRITE. This will
          break FK LOCK TABLES invariants if some of previously orphan FKs
          have referential actions which update child table.

          Note that doing this check at earlier phase is possible but seems
          to be tricky since determining orphans can be non-trivial task in
          case of multi-step RENAME TABLES statement.
        */
        if (thd->locked_tables_mode == LTM_LOCK_TABLES ||
            thd->locked_tables_mode == LTM_PRELOCKED_UNDER_LOCK_TABLES) {
          MDL_request_list orphans_mdl_requests;
          if (collect_fk_children(thd, new_db, new_alias, hton, MDL_EXCLUSIVE,
                                  &orphans_mdl_requests)) {
            // See explanation for clearing foreign key invalidator below.
            fk_invalidator->clear();
            return true;
          }

          MDL_request_list::Iterator it(orphans_mdl_requests);
          MDL_request *mdl_request;

          while ((mdl_request = it++) != nullptr) {
            if (mdl_request->key.mdl_namespace() != MDL_key::TABLE) continue;

            if (check_if_owns_upgradable_mdl(thd, mdl_request->key.db_name(),
                                             mdl_request->key.name())) {
              // See explanation for clearing foreign key invalidator below.
              fk_invalidator->clear();
              return true;
            }
          }
        }

        if (collect_and_lock_fk_tables_for_rename_table(
                thd, ren_table->db, old_alias, from_table, new_db, new_alias,
                hton, fk_invalidator)) {
          /*
            If we are performing RENAME TABLES with intermediate commits
            FK invalidator was empty before the above call. So at this
            point it only contains entries on which we might miss locks.
            We need to clear invalidator before starting process of
            reverse renames.
            If we are performing RENAME TABLES without intermediate commits
            the whole statement will be rolled back and invalidation won't
            happen. So it is safe to clear invalidator.
          */
          fk_invalidator->clear();
          return true;
        }
      }

      if (lock_check_constraint_names_for_rename(thd, ren_table->db, old_alias,
                                                 from_table, new_db, new_alias))
        return true;

      /*
        We commit changes to data-dictionary immediately after renaming
        table in storage engine if SE doesn't support atomic DDL or
        there were intermediate commits already. In the latter case
        the whole statement is not crash-safe anyway and clean-up is
        simpler this way.

        The FKs of the renamed table must be changed to reflect the new table.
        The tables referencing the old and new table names must have their FK
        information updated to reflec the correct table- and unique index name.
        The parents of the old FKs must be invalidated to make sure they
        update the cached FK parent information upon next acquisition.

        If renaming fails, my_error() has already been called
      */
      if (mysql_rename_table(
              thd, hton, ren_table->db, old_alias, ren_table->db, old_alias,
              *to_schema, new_db, new_alias,
              ((hton->flags & HTON_SUPPORTS_ATOMIC_DDL) ? NO_DD_COMMIT : 0)) ||
          ((hton->flags & HTON_SUPPORTS_FOREIGN_KEYS) &&
           adjust_fks_for_rename_table(thd, ren_table->db, old_alias, new_db,
                                       new_alias, hton))) {
        /*
          If RENAME TABLE is non-atomic as whole but we didn't try to commit
          the above changes we need to clean-up them before returning.
        */
        if (*int_commit_done && (hton->flags & HTON_SUPPORTS_ATOMIC_DDL)) {
          Implicit_substatement_state_guard substatement_guard(thd);
          trans_rollback_stmt(thd);
          // Full rollback in case we have THD::transaction_rollback_request.
          trans_rollback(thd);
          /*
            Preserve the invariant that FK invalidator is empty after each
            step of non-atomic RENAME TABLE.
          */
          fk_invalidator->clear();
        }
        return true;
      }

      /*
        If RENAME TABLE is non-atomic but we have not committed the above
        rename and changes to FK we need to do it now.
      */
      if (*int_commit_done && (hton->flags & HTON_SUPPORTS_ATOMIC_DDL)) {
        Implicit_substatement_state_guard substatement_guard(thd);

        if (trans_commit_stmt(thd) || trans_commit(thd)) {
          /*
            Preserve the invariant that FK invalidator is empty after each
            step of non-atomic RENAME TABLE.
          */
          fk_invalidator->clear();
          return true;
        }
      }

      *int_commit_done |= !(hton->flags & HTON_SUPPORTS_ATOMIC_DDL);

      if (*int_commit_done) {
        /*
          For non-atomic RENAME TABLE we try to invalidate FK parents right
          after transaction commit. This enforces invariant that invalidator
          is empty after each step of such RENAME TABLE.

          We perform invalidation if there was commit above to handle two
          cases:
          - We committed rename of table in SE supporting atomic DDL (and so
            possibly supporting FKs) since this RENAME TABLE already started
            doing intermediate commits.
          - We committed rename of table in SE not supporting atomic DDL.
            Invalidation still necessary as this might be first non-atomic
            rename which follows chain of atomic renames which might have
            added pending invalidation requests to invalidator.
        */
        fk_invalidator->invalidate(thd);
      }

      break;
    }
    case dd::enum_table_type::SYSTEM_VIEW:  // Fall through
    case dd::enum_table_type::USER_VIEW: {
      // Changing the schema of a view is not allowed.
      if (strcmp(ren_table->db, new_db)) {
        my_error(ER_FORBID_SCHEMA_CHANGE, MYF(0), ren_table->db, new_db);
        return true;
      }

      /* Rename view in the data-dictionary. */
      Implicit_substatement_state_guard substatement_guard(thd);

      // Set schema id and view name.
      from_at->set_name(new_alias);

      // Do the update. Errors will be reported by the dictionary subsystem.
      if (thd->dd_client()->update(from_at)) {
        if (*int_commit_done) {
          trans_rollback_stmt(thd);
          // Full rollback in case we have THD::transaction_rollback_request.
          trans_rollback(thd);
        }
        return true;
      }

      if (*int_commit_done) {
        if (trans_commit_stmt(thd) || trans_commit(thd)) return true;
      }

      sp_cache_invalidate();
      break;
    }
    default:
      DBUG_ASSERT(false); /* purecov: deadcode */
  }

  // Now, we know that rename succeeded, and can log the schema access
  thd->add_to_binlog_accessed_dbs(ren_table->db);
  thd->add_to_binlog_accessed_dbs(new_db);

  return false;
}
/*
  Rename all tables in list;
  Return pointer to wrong entry if something goes
  wrong.  Note that the table_list may be empty!
*/

/**
  Rename all tables/views in the list.

  @param[in]      thd               Thread handle.
  @param[in]      table_list        List of tables to rename.
  @param[in,out]  int_commit_done   Whether intermediate commits
                                    were done.
  @param[in,out]  post_ddl_htons    Set of SEs supporting atomic DDL
                                    for which post-DDL hooks needs
                                    to be called.
  @param[in,out]  fk_invalidator    Object keeping track of which
                                    dd::Table objects to invalidate.

  @note
    Take a table/view name from and odd list element and rename it to a
    the name taken from list element+1. Note that the table_list may be
    empty.

  @note Unless int_commit_done is true failure of this call requires
        rollback of transaction before doing anything else.

  @return 0 - on success, pointer to problematic entry if something
          goes wrong.
*/

static TABLE_LIST *rename_tables(
    THD *thd, TABLE_LIST *table_list, bool *int_commit_done,
    post_ddl_htons_t *post_ddl_htons,
    Foreign_key_parents_invalidator *fk_invalidator)

{
  TABLE_LIST *ren_table, *new_table;

  DBUG_TRACE;

  for (ren_table = table_list; ren_table; ren_table = new_table->next_local) {
    new_table = ren_table->next_local;
    if (do_rename(thd, ren_table, new_table->db, new_table->table_name,
                  new_table->alias, int_commit_done, post_ddl_htons,
                  fk_invalidator))
      return ren_table;
  }
  return nullptr;
}
