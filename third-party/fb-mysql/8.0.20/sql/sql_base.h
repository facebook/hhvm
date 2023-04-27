/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_BASE_INCLUDED
#define SQL_BASE_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <string>

#include "lex_string.h"
#include "map_helpers.h"
#include "mem_root_deque.h"
#include "my_base.h"  // ha_extra_function
#include "my_inttypes.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "prealloced_array.h"  // Prealloced_array
#include "sql/mdl.h"           // MDL_savepoint
#include "sql/sql_array.h"     // Bounds_checked_array
#include "sql/sql_const.h"     // enum_resolution_type
#include "sql/trigger_def.h"   // enum_trigger_event_type
#include "thr_lock.h"          // thr_lock_type

class COPY_INFO;
class Field;
class Item;
class Item_ident;
class Open_table_context;
class Open_tables_backup;
class Prelocking_strategy;
class Query_tables_list;
class SELECT_LEX;
class Sroutine_hash_entry;
class THD;
class sp_head;
struct LEX;
struct MY_BITMAP;
struct Name_resolution_context;
struct OPEN_TABLE_LIST;
struct TABLE;
struct TABLE_LIST;
struct TABLE_SHARE;
struct handlerton;
template <class T>
class List;
template <class T>
class List_iterator;

typedef Bounds_checked_array<Item *> Ref_item_array;
namespace dd {
class Table;
}  // namespace dd

#define TEMP_PREFIX "MY"

/* Defines for use with openfrm, openprt and openfrd */

#define READ_ALL 1        /* openfrm: Read all parameters */
#define EXTRA_RECORD 8    /* Reservera plats f|r extra record */
#define DELAYED_OPEN 4096 /* Open table later */
/**
  If set, open_table_from_share() will skip calling get_new_handler() to
  create a new handler object for the table. Designed to be used when
  opening a table from inside storage engines.
*/
#define SKIP_NEW_HANDLER 32768

enum find_item_error_report_type {
  REPORT_ALL_ERRORS,
  REPORT_EXCEPT_NOT_FOUND,
  IGNORE_ERRORS,
  REPORT_EXCEPT_NON_UNIQUE,
  IGNORE_EXCEPT_NON_UNIQUE
};

enum enum_tdc_remove_table_type {
  TDC_RT_REMOVE_ALL,
  TDC_RT_REMOVE_NOT_OWN,
  TDC_RT_REMOVE_UNUSED,
  TDC_RT_REMOVE_NOT_OWN_KEEP_SHARE
};

// A wrapper around std::stoul that handles exceptions
int stoul_noexcept(const std::string str, ulong *val);

extern mysql_mutex_t LOCK_open;
bool table_def_init(void);
void table_def_free(void);
void table_def_start_shutdown(void);
void assign_new_table_id(TABLE_SHARE *share);
uint cached_table_definitions(void);
size_t get_table_def_key(const TABLE_LIST *table_list, const char **key);
TABLE_SHARE *get_table_share(THD *thd, const char *db, const char *table_name,
                             const char *key, size_t key_length, bool open_view,
                             bool open_secondary = false);
void release_table_share(TABLE_SHARE *share);

TABLE *open_ltable(THD *thd, TABLE_LIST *table_list, thr_lock_type update,
                   uint lock_flags);

/* mysql_lock_tables() and open_table() flags bits */
#define MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK 0x0001
#define MYSQL_OPEN_IGNORE_FLUSH 0x0002
/* MYSQL_OPEN_TEMPORARY_ONLY (0x0004) is not used anymore. */
#define MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY 0x0008
#define MYSQL_LOCK_LOG_TABLE 0x0010
/**
  Do not try to acquire a metadata lock on the table: we
  already have one.
*/
#define MYSQL_OPEN_HAS_MDL_LOCK 0x0020
/**
  If in locked tables mode, ignore the locked tables and get
  a new instance of the table.
*/
#define MYSQL_OPEN_GET_NEW_TABLE 0x0040
/* 0x0080 used to be MYSQL_OPEN_SKIP_TEMPORARY */
/** Fail instead of waiting when conficting metadata lock is discovered. */
#define MYSQL_OPEN_FAIL_ON_MDL_CONFLICT 0x0100
/** Open tables using MDL_SHARED lock instead of one specified in parser. */
#define MYSQL_OPEN_FORCE_SHARED_MDL 0x0200
/**
  Open tables using MDL_SHARED_HIGH_PRIO lock instead of one specified
  in parser.
*/
#define MYSQL_OPEN_FORCE_SHARED_HIGH_PRIO_MDL 0x0400
/**
  When opening or locking the table, use the maximum timeout
  (LONG_TIMEOUT = 1 year) rather than the user-supplied timeout value.
*/
#define MYSQL_LOCK_IGNORE_TIMEOUT 0x0800
/**
  When acquiring "strong" (SNW, SNRW, X) metadata locks on tables to
  be open do not acquire global, tablespace-scope and schema-scope IX locks.
*/
#define MYSQL_OPEN_SKIP_SCOPED_MDL_LOCK 0x1000
/**
  When opening or locking a replication table through an internal
  operation rather than explicitly through an user thread.
*/
#define MYSQL_LOCK_RPL_INFO_TABLE 0x2000
/**
  Only check THD::killed if waits happen (e.g. wait on MDL, wait on
  table flush, wait on thr_lock.c locks) while opening and locking table.
*/
#define MYSQL_OPEN_IGNORE_KILLED 0x4000
/**
  For new TABLE instances constructed do not open table in the storage
  engine. Existing TABLE instances for which there is a handler object
  which represents table open in storage engines can still be used.
*/
#define MYSQL_OPEN_NO_NEW_TABLE_IN_SE 0x8000
/** Open a shadow copy of a table from a secondary storage engine. */
#define MYSQL_OPEN_SECONDARY_ENGINE 0x10000

/** Please refer to the internals manual. */
#define MYSQL_OPEN_REOPEN                                           \
  (MYSQL_OPEN_IGNORE_FLUSH | MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK |   \
   MYSQL_LOCK_IGNORE_GLOBAL_READ_ONLY | MYSQL_LOCK_IGNORE_TIMEOUT | \
   MYSQL_OPEN_IGNORE_KILLED | MYSQL_OPEN_GET_NEW_TABLE |            \
   MYSQL_OPEN_HAS_MDL_LOCK)

bool open_table(THD *thd, TABLE_LIST *table_list, Open_table_context *ot_ctx);

TABLE *open_table_uncached(THD *thd, const char *path, const char *db,
                           const char *table_name,
                           bool add_to_temporary_tables_list,
                           bool open_in_engine, const dd::Table &table_def);
TABLE *find_locked_table(TABLE *list, const char *db, const char *table_name);
thr_lock_type read_lock_type_for_table(THD *thd,
                                       Query_tables_list *prelocking_ctx,
                                       TABLE_LIST *table_list,
                                       bool routine_modifies_data);

bool mysql_rm_tmp_tables(void);
bool rm_temporary_table(THD *thd, handlerton *base, const char *path,
                        const dd::Table *table_def);
void close_tables_for_reopen(THD *thd, TABLE_LIST **tables,
                             const MDL_savepoint &start_of_statement_svp);
TABLE *find_temporary_table(THD *thd, const char *db, const char *table_name);
TABLE *find_temporary_table(THD *thd, const TABLE_LIST *tl);
void close_thread_tables(THD *thd);
bool fill_record_n_invoke_before_triggers(
    THD *thd, COPY_INFO *optype_info, List<Item> &fields, List<Item> &values,
    TABLE *table, enum enum_trigger_event_type event, int num_fields,
    bool raise_autoinc_has_expl_non_null_val, bool *is_row_changed);
bool fill_record_n_invoke_before_triggers(THD *thd, Field **field,
                                          List<Item> &values, TABLE *table,
                                          enum enum_trigger_event_type event,
                                          int num_fields);
bool resolve_var_assignments(THD *thd, LEX *lex);
bool insert_fields(THD *thd, Name_resolution_context *context,
                   const char *db_name, const char *table_name,
                   List_iterator<Item> *it, bool any_privileges);
bool setup_fields(THD *thd, Ref_item_array ref_item_array, List<Item> &item,
                  ulong privilege, List<Item> *sum_func_list,
                  bool allow_sum_func, bool column_update);
bool fill_record(THD *thd, TABLE *table, List<Item> &fields, List<Item> &values,
                 MY_BITMAP *bitmap, MY_BITMAP *insert_into_fields_bitmap,
                 bool raise_autoinc_has_expl_non_null_val);
bool fill_record(THD *thd, TABLE *table, Field **field, List<Item> &values,
                 MY_BITMAP *bitmap, MY_BITMAP *insert_into_fields_bitmap,
                 bool raise_autoinc_has_expl_non_null_val);

bool check_record(THD *thd, Field **ptr);

/**
  Invoke check constraints defined on the table.

  @param  thd                   Thread handle.
  @param  table                 Instance of TABLE.

  @retval  false  If all enforced check constraints are satisfied.
  @retval  true   Otherwise. THD::is_error() may be "true" in this case.
*/
bool invoke_table_check_constraints(THD *thd, const TABLE *table);

Field *find_field_in_tables(THD *thd, Item_ident *item, TABLE_LIST *first_table,
                            TABLE_LIST *last_table, Item **ref,
                            find_item_error_report_type report_error,
                            ulong want_privilege, bool register_tree_change);
Field *find_field_in_table_ref(THD *thd, TABLE_LIST *table_list,
                               const char *name, size_t length,
                               const char *item_name, const char *db_name,
                               const char *table_name, Item **ref,
                               ulong want_privilege, bool allow_rowid,
                               uint *cached_field_index_ptr,
                               bool register_tree_change,
                               TABLE_LIST **actual_table);
Field *find_field_in_table(TABLE *table, const char *name, size_t length,
                           bool allow_rowid, uint *cached_field_index_ptr);
Field *find_field_in_table_sef(TABLE *table, const char *name);
Item **find_item_in_list(THD *thd, Item *item, List<Item> &items, uint *counter,
                         find_item_error_report_type report_error,
                         enum_resolution_type *resolution);
bool setup_natural_join_row_types(THD *thd,
                                  mem_root_deque<TABLE_LIST *> *from_clause,
                                  Name_resolution_context *context);
bool wait_while_table_is_used(THD *thd, TABLE *table,
                              enum ha_extra_function function);

void update_non_unique_table_error(TABLE_LIST *update, const char *operation,
                                   TABLE_LIST *duplicate);
int setup_ftfuncs(const THD *thd, SELECT_LEX *select);
bool init_ftfuncs(THD *thd, SELECT_LEX *select);
int run_before_dml_hook(THD *thd);
bool get_and_lock_tablespace_names_nsec(THD *thd, TABLE_LIST *tables_start,
                                        TABLE_LIST *tables_end,
                                        ulonglong lock_wait_timeout_nsec,
                                        uint flags);
bool lock_table_names_nsec(
    THD *thd, TABLE_LIST *table_list, TABLE_LIST *table_list_end,
    ulonglong lock_wait_timeout_nsec, uint flags,
    Prealloced_array<MDL_request *, 1> *schema_reqs = nullptr);
bool open_tables(THD *thd, TABLE_LIST **tables, uint *counter, uint flags,
                 Prelocking_strategy *prelocking_strategy);
/* open_and_lock_tables */
bool open_and_lock_tables(THD *thd, TABLE_LIST *tables, uint flags,
                          Prelocking_strategy *prelocking_strategy);
/* simple open_and_lock_tables for single table */
TABLE *open_n_lock_single_table(THD *thd, TABLE_LIST *table_l,
                                thr_lock_type lock_type, uint flags,
                                Prelocking_strategy *prelocking_strategy);
bool open_tables_for_query(THD *thd, TABLE_LIST *tables, uint flags);
bool get_table_from_cache(THD *thd, TABLE_LIST *table);
void return_table_to_cache(THD *thd, TABLE_LIST *table);
bool lock_tables(THD *thd, TABLE_LIST *tables, uint counter, uint flags);
void free_io_cache(TABLE *entry);
void intern_close_table(TABLE *entry);
void close_thread_table(THD *thd, TABLE **table_ptr);
bool close_temporary_tables(THD *thd);
TABLE_LIST *unique_table(const TABLE_LIST *table, TABLE_LIST *table_list,
                         bool check_alias);
void drop_temporary_table(THD *thd, TABLE_LIST *table_list);
void close_temporary_table(THD *thd, TABLE *table, bool free_share,
                           bool delete_table);
void close_temporary(THD *thd, TABLE *table, bool free_share,
                     bool delete_table);
bool rename_temporary_table(THD *thd, TABLE *table, const char *new_db,
                            const char *table_name);
bool open_temporary_tables(THD *thd, TABLE_LIST *tl_list);
bool open_temporary_table(THD *thd, TABLE_LIST *tl);

/* Functions to work with system tables. */
bool open_trans_system_tables_for_read(THD *thd, TABLE_LIST *table_list);
void close_trans_system_tables(THD *thd);
void close_mysql_tables(THD *thd);
TABLE *open_log_table(THD *thd, TABLE_LIST *one_table,
                      Open_tables_backup *backup);
void close_log_table(THD *thd, Open_tables_backup *backup);

bool close_cached_tables_nsec(THD *thd, TABLE_LIST *tables,
                              bool wait_for_refresh, ulonglong timeout_nsec);

/**
  Close all open instances of the table but keep the MDL lock.

  Works both under LOCK TABLES and in the normal mode.
  Removes all closed instances of the table from the table cache.

  @param  thd         Thread context.
  @param  share       Table share, but is just a handy way to
                      access the table cache key.
  @param  remove_from_locked_tables
                      True if the table is being dropped.
                      In that case the documented behaviour is to
                      implicitly remove the table from LOCK TABLES list.
  @param  skip_table  TABLE instance that should be kept open.

  @pre Must be called with an X MDL lock on the table.
*/
void close_all_tables_for_name(THD *thd, TABLE_SHARE *share,
                               bool remove_from_locked_tables,
                               TABLE *skip_table);

/**
  Close all open instances of the table but keep the MDL lock.

  Works both under LOCK TABLES and in the normal mode.
  Removes all closed instances of the table from the table cache.

  @param  thd         Thread context.
  @param  db          Database name.
  @param  table_name  Table name.
  @param  remove_from_locked_tables
                      True if the table is being dropped.
                      In that case the documented behaviour is to
                      implicitly remove the table from LOCK TABLES list.

  @pre Must be called with an X MDL lock on the table.
*/
void close_all_tables_for_name(THD *thd, const char *db, const char *table_name,
                               bool remove_from_locked_tables);

OPEN_TABLE_LIST *list_open_tables(THD *thd, const char *db, const char *wild);
void tdc_remove_table(THD *thd, enum_tdc_remove_table_type remove_type,
                      const char *db, const char *table_name, bool has_lock);
void tdc_flush_unused_tables();
TABLE *find_table_for_mdl_upgrade(THD *thd, const char *db,
                                  const char *table_name, bool no_error);
void mark_tmp_table_for_reuse(TABLE *table);

extern Item **not_found_item;
extern Field *not_found_field;
extern Field *view_ref_found;

struct Table_share_deleter {
  void operator()(TABLE_SHARE *share) const;
};
extern malloc_unordered_map<std::string,
                            std::unique_ptr<TABLE_SHARE, Table_share_deleter>>
    *table_def_cache;

TABLE_LIST *find_table_in_global_list(TABLE_LIST *table, const char *db_name,
                                      const char *table_name);

/**
  An abstract class for a strategy specifying how the prelocking
  algorithm should extend the prelocking set while processing
  already existing elements in the set.
*/

class Prelocking_strategy {
 public:
  virtual ~Prelocking_strategy() {}

  virtual bool handle_routine(THD *thd, Query_tables_list *prelocking_ctx,
                              Sroutine_hash_entry *rt, sp_head *sp,
                              bool *need_prelocking) = 0;
  virtual bool handle_table(THD *thd, Query_tables_list *prelocking_ctx,
                            TABLE_LIST *table_list, bool *need_prelocking) = 0;
  virtual bool handle_view(THD *thd, Query_tables_list *prelocking_ctx,
                           TABLE_LIST *table_list, bool *need_prelocking) = 0;
};

/**
  A Strategy for prelocking algorithm suitable for DML statements.

  Ensures that all tables used by all statement's SF/SP/triggers and
  required for foreign key checks are prelocked and SF/SPs used are
  cached.
*/

class DML_prelocking_strategy : public Prelocking_strategy {
 public:
  virtual bool handle_routine(THD *thd, Query_tables_list *prelocking_ctx,
                              Sroutine_hash_entry *rt, sp_head *sp,
                              bool *need_prelocking);
  virtual bool handle_table(THD *thd, Query_tables_list *prelocking_ctx,
                            TABLE_LIST *table_list, bool *need_prelocking);
  virtual bool handle_view(THD *thd, Query_tables_list *prelocking_ctx,
                           TABLE_LIST *table_list, bool *need_prelocking);
};

/**
  A strategy for prelocking algorithm to be used for LOCK TABLES
  statement.
*/

class Lock_tables_prelocking_strategy : public DML_prelocking_strategy {
  virtual bool handle_table(THD *thd, Query_tables_list *prelocking_ctx,
                            TABLE_LIST *table_list, bool *need_prelocking);
};

/**
  Strategy for prelocking algorithm to be used for ALTER TABLE statements.

  Unlike DML or LOCK TABLES strategy, it doesn't
  prelock triggers, views or stored routines, since they are not
  used during ALTER.
*/

class Alter_table_prelocking_strategy : public Prelocking_strategy {
 public:
  virtual bool handle_routine(THD *thd, Query_tables_list *prelocking_ctx,
                              Sroutine_hash_entry *rt, sp_head *sp,
                              bool *need_prelocking);
  virtual bool handle_table(THD *thd, Query_tables_list *prelocking_ctx,
                            TABLE_LIST *table_list, bool *need_prelocking);
  virtual bool handle_view(THD *thd, Query_tables_list *prelocking_ctx,
                           TABLE_LIST *table_list, bool *need_prelocking);
};

inline bool open_tables(THD *thd, TABLE_LIST **tables, uint *counter,
                        uint flags) {
  DML_prelocking_strategy prelocking_strategy;

  return open_tables(thd, tables, counter, flags, &prelocking_strategy);
}

inline TABLE *open_n_lock_single_table(THD *thd, TABLE_LIST *table_l,
                                       thr_lock_type lock_type, uint flags) {
  DML_prelocking_strategy prelocking_strategy;

  return open_n_lock_single_table(thd, table_l, lock_type, flags,
                                  &prelocking_strategy);
}

// open_and_lock_tables with default prelocking strategy
inline bool open_and_lock_tables(THD *thd, TABLE_LIST *tables, uint flags) {
  DML_prelocking_strategy prelocking_strategy;

  return open_and_lock_tables(thd, tables, flags, &prelocking_strategy);
}

/**
  A context of open_tables() function, used to recover
  from a failed open_table() or open_routine() attempt.
*/

class Open_table_context {
 public:
  enum enum_open_table_action {
    OT_NO_ACTION = 0,
    OT_BACKOFF_AND_RETRY,
    OT_REOPEN_TABLES,
    OT_DISCOVER,
    OT_REPAIR,
    OT_FIX_ROW_TYPE
  };
  Open_table_context(THD *thd, uint flags);

  bool recover_from_failed_open();
  bool request_backoff_action(enum_open_table_action action_arg,
                              TABLE_LIST *table);

  bool can_recover_from_failed_open() const { return m_action != OT_NO_ACTION; }

  /**
    When doing a back-off, we close all tables acquired by this
    statement.  Return an MDL savepoint taken at the beginning of
    the statement, so that we can rollback to it before waiting on
    locks.
  */
  const MDL_savepoint &start_of_statement_svp() const {
    return m_start_of_statement_svp;
  }

  inline ulong get_timeout_nsec() const { return m_timeout_nsec; }

  uint get_flags() const { return m_flags; }

  /**
    Set flag indicating that we have already acquired metadata lock
    protecting this statement against GRL while opening tables.
  */
  void set_has_protection_against_grl() { m_has_protection_against_grl = true; }

  bool has_protection_against_grl() const {
    return m_has_protection_against_grl;
  }

  bool can_back_off() const { return !m_has_locks; }

 private:
  /* THD for which tables are opened. */
  THD *m_thd;
  /**
    For OT_DISCOVER, OT_REPAIR and OT_FIX_ROW_TYPE actions, the table list
    element for the table which definition should be re-discovered/updated
    or which should be repaired.
  */
  TABLE_LIST *m_failed_table;
  MDL_savepoint m_start_of_statement_svp;
  /**
    Lock timeout in seconds. Initialized to LONG_TIMEOUT when opening system
    tables or to the "lock_wait_timeout" system variable for regular tables.
  */
  ulonglong m_timeout_nsec;
  /* open_table() flags. */
  uint m_flags;
  /** Back off action. */
  enum enum_open_table_action m_action;
  /**
    Whether we had any locks when this context was created.
    If we did, they are from the previous statement of a transaction,
    and we can't safely do back-off (and release them).
  */
  bool m_has_locks;
  /**
    Indicates that in the process of opening tables we have acquired
    protection against global read lock.
  */
  bool m_has_protection_against_grl;
};

#endif /* SQL_BASE_INCLUDED */
