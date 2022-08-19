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

#ifndef SQL_CLASS_INCLUDED
#define SQL_CLASS_INCLUDED

/*
  This file contains the declaration of the THD class and classes which THD
  depends on. It should contain as little else as possible to increase
  cohesion and reduce coupling. Since THD is used in many places, many files
  are dependent on this header and thus require recompilation if it changes.
  Historically this file contained "Classes in mysql".
*/

#include "my_config.h"

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "m_ctype.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_idle_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/components/services/psi_transaction_bits.h"
#include "mysql/psi/mysql_thread.h"
#include "pfs_thread_provider.h"
#include "sql/psi_memory_key.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/xa.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <atomic>
#include <bitset>
#include <memory>
#include <new>
#include <string>

#include "dur_prop.h"  // durability_properties
#include "include/my_md5.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_command.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_table_map.h"
#include "my_thread_local.h"
#include "mysql/components/services/my_thread_bits.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_idle_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/components/services/psi_transaction_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/psi_base.h"
#include "mysql/thread_type.h"
#include "mysql_com.h"
#include "mysql_com_server.h"  // NET_SERVER
#include "mysqld_error.h"
#include "pfs_thread_provider.h"
#include "prealloced_array.h"
#include "rpl_master.h"
#include "sql/auth/sql_security_ctx.h"  // Security_context
#include "sql/column_statistics_dt.h"
#include "sql/current_thd.h"
#include "sql/discrete_interval.h"  // Discrete_interval
#include "sql/index_statistics.h"
#include "sql/locked_tables_list.h"  // enum_locked_tables_mode
#include "sql/mdl.h"
#include "sql/opt_costmodel.h"
#include "sql/opt_trace_context.h"  // Opt_trace_context
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/rpl_context.h"  // Rpl_thd_context
#include "sql/rpl_gtid.h"
#include "sql/rpl_lag_manager.h"
#include "sql/session_tracker.h"  // Session_tracker
#include "sql/snapshot.h"
#include "sql/sql_admission_control.h"
#include "sql/sql_connect.h"
#include "sql/sql_const.h"
#include "sql/sql_digest_stream.h"  // sql_digest_state
#include "sql/sql_error.h"
#include "sql/sql_info.h"
#include "sql/sql_list.h"
#include "sql/sql_plugin_ref.h"
#include "sql/sys_vars_resource_mgr.h"  // Session_sysvar_resource_manager
#include "sql/system_variables.h"       // system_variables
#include "sql/table.h"
#include "sql/transaction_info.h"  // Ha_trx_info
#include "sql/xa.h"
#include "sql_db.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "violite.h"

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

enum enum_check_fields : int;
enum enum_tx_isolation : int;
enum ha_notification_type : int;
enum enum_db_read_only : int;
class Item;
class Parser_state;
class PROFILING;
class Query_tables_list;
class Relay_log_info;
class THD;
class partition_info;
class Protocol;
class Protocol_binary;
class Protocol_classic;
class Protocol_text;
class sp_rcontext;
class user_var_entry;
struct LEX;
struct LEX_USER;
struct TABLE;
struct TABLE_LIST;
struct timeval;
struct User_level_lock;
struct YYLTYPE;

namespace dd {
namespace cache {
class Dictionary_client;
}

class DD_kill_immunizer;
}  // namespace dd

namespace utils {
class PerfCounter;
}

class Internal_error_handler;
class Modification_plan;
class Query_result;
class Reprepare_observer;
class Rows_log_event;
class Time_zone;
class sp_cache;
struct Binlog_user_var_event;
struct LOG_INFO;

typedef struct user_conn USER_CONN;
struct MYSQL_LOCK;
struct st_ac_node;
using st_ac_node_ptr = std::shared_ptr<st_ac_node>;
using database_container = std::unordered_set<std::string>;

enum enum_slave_use_idempotent_for_recovery {
  SLAVE_USE_IDEMPOTENT_FOR_RECOVERY_NO,
  SLAVE_USE_IDEMPOTENT_FOR_RECOVERY_YES
};

enum enum_mts_dependency_replication {
  DEP_RPL_NONE,
  DEP_RPL_TABLE,
  DEP_RPL_STATEMENT,
};

enum enum_mts_dependency_order_commits {
  DEP_RPL_ORDER_NONE,
  DEP_RPL_ORDER_DB,
  DEP_RPL_ORDER_GLOBAL,
};

enum enum_i_s_engine {
  I_S_MEMORY,
  I_S_TEMPTABLE,
};

extern "C" void thd_enter_cond(void *opaque_thd, mysql_cond_t *cond,
                               mysql_mutex_t *mutex,
                               const PSI_stage_info *stage,
                               PSI_stage_info *old_stage,
                               const char *src_function, const char *src_file,
                               int src_line);
extern "C" void thd_exit_cond(void *opaque_thd, const PSI_stage_info *stage,
                              const char *src_function, const char *src_file,
                              int src_line);

extern "C" void thd_enter_stage(void *opaque_thd,
                                const PSI_stage_info *new_stage,
                                PSI_stage_info *old_stage,
                                const char *src_function, const char *src_file,
                                int src_line);

extern "C" void thd_set_waiting_for_disk_space(void *opaque_thd,
                                               const bool waiting);

#define THD_STAGE_INFO(thd, stage) \
  (thd)->enter_stage(&stage, NULL, __func__, __FILE__, __LINE__)

extern char empty_c_string[1];
extern LEX_STRING NULL_STR;
extern LEX_CSTRING EMPTY_CSTR;
extern LEX_CSTRING NULL_CSTR;

/*
  We preallocate data for several storage engine plugins.
  so: innodb + bdb + ndb + binlog + myisam + myisammrg + archive +
      example + csv + heap + blackhole + federated + 0
  (yes, the sum is deliberately inaccurate)
*/
constexpr size_t PREALLOC_NUM_HA = 15;

/*
  When the thread is killed, we store the reason why it is killed in this buffer
  so that clients can understand why their queries fail
*/
constexpr size_t KILLED_REASON_MAX_LEN = 128;

/**
  To be used for pool-of-threads (implemented differently on various OSs)
*/
class thd_scheduler {
 public:
  void *data; /* scheduler-specific data structure */

  thd_scheduler() : data(nullptr) {}

  ~thd_scheduler() {}
};

PSI_thread *thd_get_psi(THD *thd);
void thd_set_psi(THD *thd, PSI_thread *psi);

/**
  the struct aggregates two paramenters that identify an event
  uniquely in scope of communication of a particular master and slave couple.
  I.e there can not be 2 events from the same staying connected master which
  have the same coordinates.
  @note
  Such identifier is not yet unique generally as the event originating master
  is resetable. Also the crashed master can be replaced with some other.
*/
typedef struct rpl_event_coordinates {
  char *file_name;  // binlog file name (directories stripped)
  my_off_t pos;     // event's position in the binlog file
} LOG_POS_COORD;

#define THD_SENTRY_MAGIC 0xfeedd1ff
#define THD_SENTRY_GONE 0xdeadbeef

#define THD_CHECK_SENTRY(thd) DBUG_ASSERT(thd->dbug_sentry == THD_SENTRY_MAGIC)

class Query_arena {
 private:
  /*
    List of items created for this query. Every item adds itself to the list
    on creation (see Item::Item() for details)
  */
  Item *m_item_list;

 public:
  MEM_ROOT *mem_root;  // Pointer to current memroot
  /*
    The states reflects three different life cycles for three
    different types of statements:
    Prepared statement: STMT_INITIALIZED -> STMT_PREPARED -> STMT_EXECUTED.
    Stored procedure:   STMT_INITIALIZED_FOR_SP -> STMT_EXECUTED.
    Other statements:   STMT_REGULAR_EXECUTION never changes.
  */
  enum enum_state {
    STMT_INITIALIZED = 0,
    STMT_INITIALIZED_FOR_SP = 1,
    STMT_PREPARED = 2,
    STMT_REGULAR_EXECUTION = 3,
    STMT_EXECUTED = 4,
    STMT_ERROR = -1
  };

  /*
    State and state changes in SP:
    1) When state is STMT_INITIALIZED_FOR_SP, objects in the item tree are
       created on the statement memroot. This is enforced through
       ps_arena_holder checking the state.
    2) After the first execute (call p1()), this state should change to
       STMT_EXECUTED. Objects will be created on the execution memroot and will
       be destroyed at the end of each execution.
    3) In case an ER_NEED_REPREPARE error occurs, state should be changed to
       STMT_INITIALIZED_FOR_SP and objects will again be created on the
       statement memroot. At the end of this execution, state should change to
       STMT_EXECUTED.
  */
 private:
  enum_state state;

 public:
  Query_arena(MEM_ROOT *mem_root_arg, enum enum_state state_arg)
      : m_item_list(nullptr), mem_root(mem_root_arg), state(state_arg) {}

  /*
    This constructor is used only when Query_arena is created as
    backup storage for another instance of Query_arena.
  */
  Query_arena()
      : m_item_list(nullptr), mem_root(nullptr), state(STMT_INITIALIZED) {}

  virtual ~Query_arena() = default;

  Item *item_list() const { return m_item_list; }
  void reset_item_list() { m_item_list = nullptr; }
  void set_item_list(Item *item) { m_item_list = item; }
  void add_item(Item *item);
  void free_items();
  void set_state(enum_state state_arg) { state = state_arg; }
  enum_state get_state() const { return state; }
  bool is_stmt_prepare() const { return state == STMT_INITIALIZED; }
  bool is_stmt_prepare_or_first_sp_execute() const {
    return (int)state < (int)STMT_PREPARED;
  }
  bool is_stmt_prepare_or_first_stmt_execute() const {
    return (int)state <= (int)STMT_PREPARED;
  }
  /// @returns true if a regular statement, ie not prepared and not stored proc
  bool is_regular() const { return state == STMT_REGULAR_EXECUTION; }

  void *alloc(size_t size) { return mem_root->Alloc(size); }
  void *mem_calloc(size_t size) {
    void *ptr;
    if ((ptr = mem_root->Alloc(size))) memset(ptr, 0, size);
    return ptr;
  }
  template <typename T>
  T *alloc_typed() {
    void *m = alloc(sizeof(T));
    return m == nullptr ? nullptr : new (m) T;
  }
  template <typename T>
  T *memdup_typed(const T *mem) {
    return static_cast<T *>(memdup_root(mem_root, mem, sizeof(T)));
  }
  char *mem_strdup(const char *str) { return strdup_root(mem_root, str); }
  char *strmake(const char *str, size_t size) const {
    return strmake_root(mem_root, str, size);
  }
  void *memdup(const void *str, size_t size) {
    return memdup_root(mem_root, str, size);
  }

  /**
    Copies memory-managing members from `set`. No references are kept to it.

    @param set A Query_arena from which members are copied.
  */
  void set_query_arena(const Query_arena &set);

  /**
    Copy the current arena to `backup` and set the current
    arena to match `source`

    @param source A Query_arena from which members are copied.
    @param backup A Query_arena to which members are first saved.
  */
  void swap_query_arena(const Query_arena &source, Query_arena *backup);
};

class Prepared_statement;

/**
  Container for all prepared statements created/used in a connection.

  Prepared statements in Prepared_statement_map have unique id
  (guaranteed by id assignment in Prepared_statement::Prepared_statement).

  Non-empty statement names are unique too: attempt to insert a new statement
  with duplicate name causes older statement to be deleted.

  Prepared statements are auto-deleted when they are removed from the map
  and when the map is deleted.
*/

class Prepared_statement_map {
 public:
  Prepared_statement_map();

  /**
    Insert a new statement to the thread-local prepared statement map.

    If there was an old statement with the same name, replace it with the
    new one. Otherwise, check if max_prepared_stmt_count is not reached yet,
    increase prepared_stmt_count, and insert the new statement. It's okay
    to delete an old statement and fail to insert the new one.

    All named prepared statements are also present in names_hash.
    Prepared statement names in names_hash are unique.
    The statement is added only if prepared_stmt_count < max_prepard_stmt_count
    m_last_found_statement always points to a valid statement or is 0

    @retval 0  success
    @retval 1  error: out of resources or max_prepared_stmt_count limit has been
                      reached. An error is sent to the client, the statement
                      is deleted.
  */
  int insert(Prepared_statement *statement);

  /** Find prepared statement by name. */
  Prepared_statement *find_by_name(const LEX_CSTRING &name);

  /** Find prepared statement by ID. */
  Prepared_statement *find(ulong id);

  /** Erase all prepared statements (calls Prepared_statement destructor). */
  void erase(Prepared_statement *statement);

  void claim_memory_ownership();

  void reset();

  ~Prepared_statement_map();

 private:
  malloc_unordered_map<ulong, std::unique_ptr<Prepared_statement>> st_hash;
  collation_unordered_map<std::string, Prepared_statement *> names_hash;
  Prepared_statement *m_last_found_statement;
};

/**
  A registry for item tree transformations performed during
  query optimization. We register only those changes which require
  a rollback to re-execute a prepared statement or stored procedure
  yet another time.
*/

class Item_change_record : public ilink<Item_change_record> {
 private:
  // not used
  Item_change_record() {}

 public:
  Item_change_record(Item **place, Item *new_value)
      : place(place), old_value(*place), new_value(new_value) {}
  Item **place;
  Item *old_value;
  Item *new_value;
  bool m_cancel{false};
};

typedef I_List<Item_change_record> Item_change_list;

/**
  Class that holds information about tables which were opened and locked
  by the thread. It is also used to save/restore this information in
  push_open_tables_state()/pop_open_tables_state().
*/

class Open_tables_state {
 private:
  /**
    A stack of Reprepare_observer-instances. The top most instance is the
    currently active one. This stack is used during execution of prepared
    statements and stored programs in order to detect metadata changes.
    The locking subsystem reports a metadata change if the top-most item is not
    NULL.

    When Open_tables_state part of THD is reset to open a system or
    INFORMATION_SCHEMA table, NULL is temporarily pushed to avoid spurious
    ER_NEED_REPREPARE errors -- system and INFORMATION_SCHEMA tables are not
    subject to metadata version tracking.

    A stack is used here for the convenience -- in some cases we need to
    temporarily override/disable current Reprepare_observer-instance.

    NOTE: This is not a list of observers, only the top-most element will be
    notified in case of a metadata change.

    @sa check_and_update_table_version()
  */
  Prealloced_array<Reprepare_observer *, 4> m_reprepare_observers;

 public:
  Reprepare_observer *get_reprepare_observer() const {
    return m_reprepare_observers.size() > 0 ? m_reprepare_observers.back()
                                            : NULL;
  }

  void push_reprepare_observer(Reprepare_observer *o) {
    m_reprepare_observers.push_back(o);
  }

  Reprepare_observer *pop_reprepare_observer() {
    Reprepare_observer *retval = m_reprepare_observers.back();
    m_reprepare_observers.pop_back();
    return retval;
  }

  void reset_reprepare_observers() { m_reprepare_observers.clear(); }

 public:
  /**
    List of regular tables in use by this thread. Contains persistent base
    tables that were opened with @see open_tables().
  */
  TABLE *open_tables;
  /**
    List of temporary tables used by this thread. Contains user-level
    temporary tables, created with CREATE TEMPORARY TABLE, and
    intermediate tables used in ALTER TABLE implementation.
  */
  TABLE *temporary_tables;
  /*
    During a MySQL session, one can lock tables in two modes: automatic
    or manual. In automatic mode all necessary tables are locked just before
    statement execution, and all acquired locks are stored in 'lock'
    member. Unlocking takes place automatically as well, when the
    statement ends.
    Manual mode comes into play when a user issues a 'LOCK TABLES'
    statement. In this mode the user can only use the locked tables.
    Trying to use any other tables will give an error.
    The locked tables are also stored in this member, however,
    thd->locked_tables_mode is turned on.  Manual locking is described in
    the 'LOCK_TABLES' chapter of the MySQL manual.
    See also lock_tables() for details.
  */
  MYSQL_LOCK *lock;

  /*
    CREATE-SELECT keeps an extra lock for the table being
    created. This field is used to keep the extra lock available for
    lower level routines, which would otherwise miss that lock.
   */
  MYSQL_LOCK *extra_lock;

  /*
    Enum enum_locked_tables_mode and locked_tables_mode member are
    used to indicate whether the so-called "locked tables mode" is on,
    and what kind of mode is active.

    Locked tables mode is used when it's necessary to open and
    lock many tables at once, for usage across multiple
    (sub-)statements.
    This may be necessary either for queries that use stored functions
    and triggers, in which case the statements inside functions and
    triggers may be executed many times, or for implementation of
    LOCK TABLES, in which case the opened tables are reused by all
    subsequent statements until a call to UNLOCK TABLES.

    The kind of locked tables mode employed for stored functions and
    triggers is also called "prelocked mode".
    In this mode, first open_tables() call to open the tables used
    in a statement analyses all functions used by the statement
    and adds all indirectly used tables to the list of tables to
    open and lock.
    It also marks the parse tree of the statement as requiring
    prelocking. After that, lock_tables() locks the entire list
    of tables and changes THD::locked_tables_modeto LTM_PRELOCKED.
    All statements executed inside functions or triggers
    use the prelocked tables, instead of opening their own ones.
    Prelocked mode is turned off automatically once close_thread_tables()
    of the main statement is called.
  */
  enum enum_locked_tables_mode locked_tables_mode;

  enum enum_flags {
    BACKUPS_AVAIL = (1U << 0), /* There are backups available. */
    SYSTEM_TABLES = (1U << 1)  /* We are opening system tables. */
  };

  /*
    Flags with information about the open tables state.
  */
  uint state_flags;
  /**
     This constructor initializes Open_tables_state instance which can only
     be used as backup storage. To prepare Open_tables_state instance for
     operations which open/lock/close tables (e.g. open_table()) one has to
     call init_open_tables_state().
  */
  Open_tables_state()
      : m_reprepare_observers(PSI_INSTRUMENT_ME), state_flags(0U) {}

  void set_open_tables_state(Open_tables_state *state);

  void reset_open_tables_state();
};

/**
  Storage for backup of Open_tables_state. Must
  be used only to open system tables (TABLE_CATEGORY_SYSTEM
  and TABLE_CATEGORY_LOG).
*/

class Open_tables_backup : public Open_tables_state {
 public:
  /**
    When we backup the open tables state to open a system
    table or tables, we want to save state of metadata
    locks which were acquired before the backup. It is used
    to release metadata locks on system tables after they are
    no longer used.
  */
  MDL_savepoint mdl_system_tables_svp;
};

/**
  Enum that represents which phase of secondary engine optimization
  the current statement is in.
*/
enum class Secondary_engine_optimization {
  /**
    The current statement should only use tables from primary storage
    engines. Use of secondary storage engines is disabled.
  */
  PRIMARY_ONLY,

  /**
    The current statement should only use tables from the primary
    storage engine. However, use of secondary storage engines is not
    disabled, so the optimizer may choose to trigger a repreparation
    against the secondary storage engine if it believes that use of a
    secondary storage engine is beneficial.
  */
  PRIMARY_TENTATIVELY,

  /**
    The current statement should use tables from a secondary storage
    engine if possible. Otherwise, fall back to using tables from
    primary storage engine only.
  */
  SECONDARY,
};

/**
  @class Sub_statement_state
  @brief Used to save context when executing a function or trigger
*/

/* Defines used for Sub_statement_state::in_sub_stmt */

#define SUB_STMT_TRIGGER 1
#define SUB_STMT_FUNCTION 2

class Sub_statement_state {
 public:
  ulonglong option_bits;
  ulonglong first_successful_insert_id_in_prev_stmt;
  ulonglong first_successful_insert_id_in_cur_stmt;
  Discrete_intervals_list auto_inc_intervals_forced;
  ulonglong current_found_rows;
  ulonglong previous_found_rows;
  ha_rows num_truncated_fields, sent_row_count, examined_row_count;
  ulong client_capabilities;
  uint in_sub_stmt;
  bool enable_slow_log;
  SAVEPOINT *savepoints;
  enum enum_check_fields check_for_truncated_fields;
};

inline char const *show_system_thread(enum_thread_type thread) {
#define RETURN_NAME_AS_STRING(NAME) \
  case (NAME):                      \
    return #NAME
  switch (thread) {
    static char buf[64];
    RETURN_NAME_AS_STRING(NON_SYSTEM_THREAD);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_SLAVE_IO);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_SLAVE_SQL);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_NDBCLUSTER_BINLOG);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_EVENT_SCHEDULER);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_EVENT_WORKER);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_INFO_REPOSITORY);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_SLAVE_WORKER);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_COMPRESS_GTID_TABLE);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_BACKGROUND);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_DD_INITIALIZE);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_DD_RESTART);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_SERVER_INITIALIZE);
    RETURN_NAME_AS_STRING(SYSTEM_THREAD_INIT_FILE);
    default:
      sprintf(buf, "<UNKNOWN SYSTEM THREAD: %d>", thread);
      return buf;
  }
#undef RETURN_NAME_AS_STRING
}

/**
  Storage engine specific thread local data.
*/

struct Ha_data {
  /**
    Storage engine specific thread local data.
    Lifetime: one user connection.
  */
  void *ha_ptr;
  /**
    A memorizer to engine specific "native" transaction object to provide
    storage engine detach-re-attach facility.
    The server level transaction object can dissociate from storage engine
    transactions. The released "native" transaction reference
    can be hold in the member until it is reconciled later.
    Lifetime: Depends on caller of @c hton::replace_native_transaction_in_thd.
    For instance in the case of slave server applier handling XA transaction
    it is from XA START to XA PREPARE.
  */
  void *ha_ptr_backup;
  /**
    0: Life time: one statement within a transaction. If @@autocommit is
    on, also represents the entire transaction.
    @sa trans_register_ha()

    1: Life time: one transaction within a connection.
    If the storage engine does not participate in a transaction,
    this should not be used.
    @sa trans_register_ha()
  */
  Ha_trx_info ha_info[2];

  /**
    NULL: engine is not bound to this thread
    non-NULL: engine is bound to this thread, engine shutdown forbidden
  */
  plugin_ref lock;

  Ha_data() : ha_ptr(nullptr), ha_ptr_backup(nullptr), lock(nullptr) {}
};

/**
  An instance of the global read lock in a connection.
  Implemented in lock.cc.
*/

class Global_read_lock {
 public:
  enum enum_grl_state {
    GRL_NONE,
    GRL_ACQUIRED,
    GRL_ACQUIRED_AND_BLOCKS_COMMIT
  };

  Global_read_lock()
      : m_state(GRL_NONE),
        m_mdl_global_shared_lock(nullptr),
        m_mdl_blocks_commits_lock(nullptr) {}

  bool lock_global_read_lock(THD *thd);
  void unlock_global_read_lock(THD *thd);

  /**
    Used by innodb memcached server to check if any connections
    have global read lock
  */
  static bool global_read_lock_active() { return m_atomic_active_requests > 0; }

  /**
    Check if this connection can acquire protection against GRL and
    emit error if otherwise.
  */
  bool can_acquire_protection() const {
    if (m_state) {
      my_error(ER_CANT_UPDATE_WITH_READLOCK, MYF(0));
      return true;
    }
    return false;
  }
  bool make_global_read_lock_block_commit(THD *thd);
  bool is_acquired() const { return m_state != GRL_NONE; }
  void set_explicit_lock_duration(THD *thd);

 private:
  static std::atomic<int32> m_atomic_active_requests;
  enum_grl_state m_state;
  /**
    In order to acquire the global read lock, the connection must
    acquire shared metadata lock in GLOBAL namespace, to prohibit
    all DDL.
  */
  MDL_ticket *m_mdl_global_shared_lock;
  /**
    Also in order to acquire the global read lock, the connection
    must acquire a shared metadata lock in COMMIT namespace, to
    prohibit commits.
  */
  MDL_ticket *m_mdl_blocks_commits_lock;
};

extern "C" void my_message_sql(uint error, const char *str, myf MyFlags);

/**
  @class THD
  For each client connection we create a separate thread with THD serving as
  a thread/connection descriptor
*/

class THD : public MDL_context_owner,
            public Query_arena,
            public Open_tables_state {
 private:
  inline bool is_stmt_prepare() const {
    DBUG_ASSERT(0);
    return Query_arena::is_stmt_prepare();
  }

  inline bool is_stmt_prepare_or_first_sp_execute() const {
    DBUG_ASSERT(0);
    return Query_arena::is_stmt_prepare_or_first_sp_execute();
  }

  inline bool is_stmt_prepare_or_first_stmt_execute() const {
    DBUG_ASSERT(0);
    return Query_arena::is_stmt_prepare_or_first_stmt_execute();
  }

  inline bool is_regular() const {
    DBUG_ASSERT(0);
    return Query_arena::is_regular();
  }

 public:
  MDL_context mdl_context;

  /*
    MARK_COLUMNS_NONE:  Means mark_used_colums is not set and no indicator to
                        handler of fields used is set
    MARK_COLUMNS_READ:  Means a bit in read set is set to inform handler
                        that the field is to be read. Update covering_keys
                        and merge_keys too.
    MARK_COLUMNS_WRITE: Means a bit is set in write set to inform handler
                        that it needs to update this field in write_row
                        and update_row.
    MARK_COLUMNS_TEMP:  Mark bit in read set, but ignore key sets.
                        Used by filesort().
  */
  enum enum_mark_columns mark_used_columns;
  /**
    Used by Item::check_column_privileges() to tell which privileges
    to check for.
    Set to ~0ULL before starting to resolve a statement.
    Set to desired privilege mask before calling a resolver function that will
    call Item::check_column_privileges().
    After use, restore previous value as current value.
  */
  ulong want_privilege;

 private:
  /**
    The lex to hold the parsed tree of conventional (non-prepared) queries.
    Whereas for prepared and stored procedure statements we use an own lex
    instance for each new query, for conventional statements we reuse
    the same lex. (@see mysql_parse for details).
  */
  std::unique_ptr<LEX> main_lex;

 public:
  LEX *lex;                                        // parse tree descriptor
  dd::cache::Dictionary_client *dd_client() const  // Get the dictionary client.
  {
    return m_dd_client.get();
  }

 private:
  std::unique_ptr<dd::cache::Dictionary_client> m_dd_client;

  /**
    The query associated with this statement.
  */
  LEX_CSTRING m_query_string;
  String m_normalized_query;

  /**
    Currently selected catalog.
  */

  LEX_CSTRING m_catalog;
  /**
    Name of the current (default) database.

    If there is the current (default) database, "db" contains its name. If
    there is no current (default) database, "db" is NULL and "db_length" is
    0. In other words, "db", "db_length" must either be NULL, or contain a
    valid database name.

    @note this attribute is set and alloced by the slave SQL thread (for
    the THD of that thread); that thread is (and must remain, for now) the
    only responsible for freeing this member.
  */
  LEX_CSTRING m_db;

  /**
    Resource group context indicating the current resource group
    and the name of the resource group to switch to during execution
    of a query.
  */
  resourcegroups::Resource_group_ctx m_resource_group_ctx;

  /**
    In some cases, we may want to modify the query (i.e. replace
    passwords with their hashes before logging the statement etc.).

    In case the query was rewritten, the original query will live in
    m_query_string, while the rewritten query lives in rewritten_query.
    If rewritten_query is empty, m_query_string should be logged.
    If rewritten_query is non-empty, the rewritten query it contains
    should be used in logs (general log, slow query log, binary log).

    Currently, password obfuscation is the only rewriting we do; more
    may follow at a later date, both pre- and post parsing of the query.
    Rewriting of binloggable statements must preserve all pertinent
    information.

    Similar restrictions as for m_query_string (see there) hold for locking:
    - Value may only be (re)set from owning thread (current_thd)
    - Value must be modified using (reset|swap)_rewritten_query().
      Doing so will protect the update with LOCK_thd_query.
    - The owner (current_thd) may read the value without holding the lock.
    - Other threads may read the value, but must hold LOCK_thd_query to do so.
  */
  String m_rewritten_query;

 public:
  /* Used to execute base64 coded binlog events in MySQL server */
  Relay_log_info *rli_fake;
  /* Slave applier execution context */
  Relay_log_info *rli_slave;

  /* Is transaction commit still pending */
  bool tx_commit_pending;

  /* Column usage statistics for the SQL statements */
  std::set<ColumnUsageInfo> column_usage_info;

  /* Index usage statistics. Only rows_requested is supported for now. */
  std::unordered_map<IndexSpecification, ulonglong, IndexSpecificationHash>
      thd_ius;

  /* Nonsuper_connections reference counting */
 private:
  ulong nonsuper_ref = 0;

 public:
  ulong add_nonsuper_connections_ref();
  void remove_nonsuper_connections_ref(bool clear = false);

  /* Next HLC value */
  uint64_t hlc_time_ns_next = 0;
  bool should_write_hlc = false;
  database_container databases;

  /**
    The function checks whether the thread is processing queries from binlog,
    as automatically generated by mysqlbinlog.

    @return true  when the thread is a binlog applier
  */
  bool is_binlog_applier() const {
    return rli_fake && variables.pseudo_slave_mode;
  }

  /**
    When the thread is a binlog or slave applier it detaches the engine
    ha_data associated with it and memorizes the fact of that.
  */
  void rpl_detach_engine_ha_data();

  /**
    When the thread is a binlog or slave applier it reattaches the engine
    ha_data associated with it and memorizes the fact of that.
  */
  void rpl_reattach_engine_ha_data();

  /**
    @return true   when the current binlog (rli_fake) or slave (rli_slave)
                   applier thread has detached the engine ha_data,
                   see @c rpl_detach_engine_ha_data.
    @note The detached transaction applier resets a memo
          mark at once with this check.
  */
  bool rpl_unflag_detached_engine_ha_data() const;

  void reset_for_next_command();
  /*
    Constant for THD::where initialization in the beginning of every query.

    It's needed because we do not save/restore THD::where normally during
    primary (non subselect) query execution.
  */
  static const char *const DEFAULT_WHERE;

  /** Additional network instrumentation for the server only. */
  NET_SERVER m_net_server_extension;
  /**
    Hash for user variables.
    User variables are per session,
    but can also be monitored outside of the session,
    so a lock is needed to prevent race conditions.
    Protected by @c LOCK_thd_data.
  */
  collation_unordered_map<std::string, unique_ptr_with_deleter<user_var_entry>>
      user_vars{system_charset_info, key_memory_user_var_entry};
  struct rand_struct rand;                      // used for authentication
  struct System_variables variables;            // Changeable local variables
  struct System_status_var status_var;          // Per thread statistic vars
  struct System_status_var *initial_status_var; /* used by show status */
  // has status_var already been added to global_status_var?
  bool status_var_aggregated;

  /**
    Session's connection attributes for the connected client
  */
  std::vector<char> m_connection_attributes;

  /**
    Current query cost.
    @sa system_status_var::last_query_cost
  */
  double m_current_query_cost;
  /**
    Current query partial plans.
    @sa system_status_var::last_query_partial_plans
  */
  ulonglong m_current_query_partial_plans;

  /**
    Clear the query costs attributes for the current query.
  */
  void clear_current_query_costs() {
    m_current_query_cost = 0.0;
    m_current_query_partial_plans = 0;
  }

  /**
    Save the current query costs attributes in
    the thread session status.
    Use this method only after the query execution is completed,
    so that
      @code SHOW SESSION STATUS like 'last_query_%' @endcode
      @code SELECT * from performance_schema.session_status
      WHERE VARIABLE_NAME like 'last_query_%' @endcode
    actually reports the previous query, not itself.
  */
  void save_current_query_costs() {
    DBUG_ASSERT(!status_var_aggregated);
    status_var.last_query_cost = m_current_query_cost;
    status_var.last_query_partial_plans = m_current_query_partial_plans;
  }

  THR_LOCK_INFO lock_info;  // Locking info of this thread
  /**
    Protects THD data accessed from other threads.
    The attributes protected are:
    - thd->is_killable (used by KILL statement and shutdown).
    - thd->user_vars (user variables, inspected by monitoring)
    Is locked when THD is deleted.
  */
  mysql_mutex_t LOCK_thd_data;

  /**
    Protects THD::m_query_string. No other mutexes should be locked
    while having this mutex locked.
  */
  mysql_mutex_t LOCK_thd_query;

  /**
    Protects THD::variables while being updated. This should be taken inside
    of LOCK_thd_data and outside of LOCK_global_system_variables.
  */
  mysql_mutex_t LOCK_thd_sysvar;

  /**
    Protects THD::m_protocol when it gets removed in x plugin.
  */
  mysql_mutex_t LOCK_thd_protocol;

  /**
    Protects THD::db_read_only_hash.
  */
  mysql_mutex_t LOCK_thd_db_read_only_hash;
  mysql_mutex_t LOCK_thd_db_context;
  mysql_mutex_t LOCK_thd_audit_data;
  mysql_mutex_t LOCK_thd_db_default_collation_hash;
  /**
    Protects query plan (SELECT/UPDATE/DELETE's) from being freed/changed
    while another thread explains it. Following structures are protected by
    this mutex:
      THD::Query_plan
      Modification_plan
      SELECT_LEX::join
      JOIN::plan_state
      Tree of SELECT_LEX_UNIT after THD::Query_plan was set till
        THD::Query_plan cleanup
      JOIN_TAB::select->quick
    Code that changes objects above should take this mutex.
    Explain code takes this mutex to block changes to named structures to
    avoid crashes in following functions:
      explain_single_table_modification
      explain_query
      Sql_cmd_explain_other_thread::execute
    When doing EXPLAIN CONNECTION:
      all explain code assumes that this mutex is already taken.
    When doing ordinary EXPLAIN:
      the mutex does need to be taken (no need to protect reading my own data,
      moreover EXPLAIN CONNECTION can't run on an ordinary EXPLAIN).
  */
 private:
  mysql_mutex_t LOCK_query_plan;

 public:
  /// Locks the query plan of this THD
  void lock_query_plan() { mysql_mutex_lock(&LOCK_query_plan); }
  void unlock_query_plan() { mysql_mutex_unlock(&LOCK_query_plan); }

  /** All prepared statements of this connection. */
  Prepared_statement_map stmt_map;
  /*
    A pointer to the stack frame of handle_one_connection(),
    which is called first in the thread for handling a client
  */
  const char *thread_stack;

  /**
    @note
    Some members of THD (currently 'Statement::db',
    'catalog' and 'query')  are set and alloced by the slave SQL thread
    (for the THD of that thread); that thread is (and must remain, for now)
    the only responsible for freeing these 3 members. If you add members
    here, and you add code to set them in replication, don't forget to
    free_them_and_set_them_to_0 in replication properly. For details see
    the 'err:' label of the handle_slave_sql() in sql/slave.cc.

    @see handle_slave_sql
  */

  Security_context m_main_security_ctx;
  Security_context *m_security_ctx;

  Security_context *security_context() const { return m_security_ctx; }
  void set_security_context(Security_context *sctx) { m_security_ctx = sctx; }
  List<Security_context> m_view_ctx_list;

  /**
    @note
    The optional password validation plugin doesn't have any API for
    temporally disable its functionality for a particular session.
    To get around this issue we introduce a boolean variable in the THD
    which we check before each call to the password validation plugin.
    Password validation is invoked from within the authentication plugin
    in the generate_authentication_string() method.

    @see generate_authentication_string
  */
  bool m_disable_password_validation;

  /*
    Points to info-string that we show in SHOW PROCESSLIST
    You are supposed to update thd->proc_info only if you have coded
    a time-consuming piece that MySQL can get stuck in for a long time.

    Set it using the  thd_proc_info(THD *thread, const char *message)
    macro/function.

    This member is accessed and assigned without any synchronization.
    Therefore, it may point only to constant (statically
    allocated) strings, which memory won't go away over time.
  */
  const char *proc_info;

  std::unique_ptr<Protocol_text> protocol_text;      // Normal protocol
  std::unique_ptr<Protocol_binary> protocol_binary;  // Binary protocol

  const Protocol *get_protocol() const { return m_protocol; }

  Protocol *get_protocol() { return m_protocol; }

  SSL_handle get_ssl() const {
#ifndef DBUG_OFF
    if (current_thd != this) {
      /*
        When inspecting this thread from monitoring,
        the monitoring thread MUST lock LOCK_thd_data,
        to be allowed to safely inspect SSL status variables.
      */
      mysql_mutex_assert_owner(&LOCK_thd_data);
    }
#endif
    return m_SSL;
  }

  /**
    Asserts that the protocol is of type text or binary and then
    returns the m_protocol casted to Protocol_classic. This method
    is needed to prevent misuse of pluggable protocols by legacy code
  */
  const Protocol_classic *get_protocol_classic() const {
    DBUG_ASSERT(is_classic_protocol());
    return pointer_cast<const Protocol_classic *>(m_protocol);
  }

  Protocol_classic *get_protocol_classic() {
    DBUG_ASSERT(is_classic_protocol());
    return pointer_cast<Protocol_classic *>(m_protocol);
  }

 private:
  Protocol *m_protocol;  // Current protocol
  /**
    SSL data attached to this connection.
    This is an opaque pointer,
    When building with SSL, this pointer is non NULL
    only if the connection is using SSL.
    When building without SSL, this pointer is always NULL.
    The SSL data can be inspected to read per thread
    status variables,
    and this can be inspected while the thread is running.
  */
  SSL_handle m_SSL = {nullptr};

 public:
  /**
     Query plan for EXPLAINable commands, should be locked with
     LOCK_query_plan before using.
  */
  class Query_plan {
   private:
    THD *const thd;
    /// Original sql_command;
    enum_sql_command sql_command;
    /// LEX of topmost statement
    LEX *lex;
    /// Query plan for UPDATE/DELETE/INSERT/REPLACE
    const Modification_plan *modification_plan;
    /// True if query is run in prepared statement
    bool is_ps;

    explicit Query_plan(const Query_plan &);    ///< not defined
    Query_plan &operator=(const Query_plan &);  ///< not defined

   public:
    /// Asserts that current_thd has locked this plan, if it does not own it.
    void assert_plan_is_locked_if_other() const
#ifdef DBUG_OFF
    {
    }
#else
        ;
#endif

    explicit Query_plan(THD *thd_arg)
        : thd(thd_arg),
          sql_command(SQLCOM_END),
          lex(nullptr),
          modification_plan(nullptr),
          is_ps(false) {}

    /**
      Set query plan.

      @note This function takes THD::LOCK_query_plan mutex.
    */
    void set_query_plan(enum_sql_command sql_cmd, LEX *lex_arg, bool ps);

    /*
      The 4 getters below expect THD::LOCK_query_plan to be already taken
      if called from another thread.
    */
    enum_sql_command get_command() const {
      assert_plan_is_locked_if_other();
      return sql_command;
    }
    LEX *get_lex() const {
      assert_plan_is_locked_if_other();
      return lex;
    }
    Modification_plan const *get_modification_plan() const {
      assert_plan_is_locked_if_other();
      return modification_plan;
    }
    bool is_ps_query() const {
      assert_plan_is_locked_if_other();
      return is_ps;
    }
    bool is_single_table_plan() const;
    void set_modification_plan(Modification_plan *plan_arg);

  } query_plan;

  const LEX_CSTRING &catalog() const { return m_catalog; }

  void set_catalog(const LEX_CSTRING &catalog) { m_catalog = catalog; }

 private:
  unsigned int m_current_stage_key;

 public:
  // See comment in THD::enter_cond about why SUPPRESS_TSAN is needed.
  void enter_stage(const PSI_stage_info *stage, PSI_stage_info *old_stage,
                   const char *calling_func, const char *calling_file,
                   const unsigned int calling_line) SUPPRESS_TSAN;
  const char *get_proc_info() const { return proc_info; }

  /*
    Used in error messages to tell user in what part of MySQL we found an
    error. E. g. when where= "having clause", if fix_fields() fails, user
    will know that the error was in having clause.
  */
  const char *where;

  ulong max_client_packet_length;

  collation_unordered_map<std::string, unique_ptr_my_free<TABLE_LIST>>
      handler_tables_hash{&my_charset_latin1,
                          key_memory_THD_handler_tables_hash};
  /*
    A thread can hold named user-level locks. This variable
    contains granted tickets if a lock is present. See item_func.cc and
    chapter 'Miscellaneous functions', for functions GET_LOCK, RELEASE_LOCK.
  */
  malloc_unordered_map<std::string, User_level_lock *> ull_hash{
      key_memory_User_level_lock};
#ifndef DBUG_OFF
  uint dbug_sentry;  // watch out for memory corruption
#endif
  bool is_killable;
  /**
    Mutex protecting access to current_mutex and current_cond.
  */
  mysql_mutex_t LOCK_current_cond;
  /**
    The mutex used with current_cond.
    @see current_cond
  */
  std::atomic<mysql_mutex_t *> current_mutex;
  /**
    Pointer to the condition variable the thread owning this THD
    is currently waiting for. If the thread is not waiting, the
    value is NULL. Set by THD::enter_cond().

    If this thread is killed (shutdown or KILL stmt), another
    thread will broadcast on this condition variable so that the
    thread can be unstuck.
  */
  std::atomic<mysql_cond_t *> current_cond;
  /**
    Condition variable used for waiting by the THR_LOCK.c subsystem.
  */
  mysql_cond_t COND_thr_lock;

 private:
  /**
    Type of current query: COM_STMT_PREPARE, COM_QUERY, etc.
    Set from first byte of the packet in do_command()
  */
  enum enum_server_command m_command;

 private:
  bool m_is_admin_conn;

 public:
  void set_admin_connection(bool admin) { m_is_admin_conn = admin; }
  bool is_admin_connection() const { return m_is_admin_conn; }

  uint32 unmasked_server_id;
  uint32 server_id;
  uint32 file_id;  // for LOAD DATA INFILE
  /* remote (peer) port */
  uint16 peer_port;
  struct timeval start_time;
  struct timeval user_time;
  ulonglong start_utime, utime_after_lock, pre_exec_time;
  struct timespec start_cputime;
  /* record the semisync ack time */
  ulonglong semisync_ack_time = 0;
  /* record the engine commit time */
  ulonglong engine_commit_time = 0;

  /**
    Type of lock to be used for all DML statements, except INSERT, in cases
    when lock is not specified explicitly.  Set to TL_WRITE or
    TL_WRITE_LOW_PRIORITY depending on whether low_priority_updates option is
    off or on.
  */
  thr_lock_type update_lock_default;
  /**
    Type of lock to be used for INSERT statement if lock is not specified
    explicitly. Set to TL_WRITE_CONCURRENT_INSERT or TL_WRITE_LOW_PRIORITY
    depending on whether low_priority_updates option is off or on.
  */
  thr_lock_type insert_lock_default;

  /* <> 0 if we are inside of trigger or stored function. */
  uint in_sub_stmt;

  /**
    Used by fill_status() to avoid acquiring LOCK_status mutex twice
    when this function is called recursively (e.g. queries
    that contains SELECT on I_S.GLOBAL_STATUS with subquery on the
    same I_S table).
    Incremented each time fill_status() function is entered and
    decremented each time before it returns from the function.
  */
  uint fill_status_recursion_level;
  uint fill_variables_recursion_level;

 private:
  /* container for handler's private per-connection data */
  Prealloced_array<Ha_data, PREALLOC_NUM_HA> ha_data;

 public:
  /**
    Retrieve Ha_data for a given slot. Each handler has a fixed slot nr.
  */
  Ha_data *get_ha_data(int slot) { return &ha_data[slot]; }

  /**
    Copy ha_data into the provided argument. Used by Attachble_transaction.
  */
  void backup_ha_data(Prealloced_array<Ha_data, PREALLOC_NUM_HA> *backup) {
    /*
      Protect with LOCK_thd_data avoid accessing ha_data while it
      is being modified.
    */
    mysql_mutex_lock(&this->LOCK_thd_data);
    *backup = ha_data;
    mysql_mutex_unlock(&this->LOCK_thd_data);
  }

  /**
    Restore ha_data from the provided backup copy.
    Used by Attachable_Transaction.
  */
  void restore_ha_data(
      const Prealloced_array<Ha_data, PREALLOC_NUM_HA> &backup) {
    /*
      Protect with LOCK_thd_data to avoid e.g. KILL CONNECTION
      reading ha_data while it is being modified.
    */
    mysql_mutex_lock(&this->LOCK_thd_data);
    ha_data = backup;
    mysql_mutex_unlock(&this->LOCK_thd_data);
  }

  /*
    Position of first event in Binlog
    *after* last event written by this
    thread.
  */
  rpl_event_coordinates binlog_next_event_pos;
  void set_next_event_pos(const char *_filename, ulonglong _pos);
  void clear_next_event_pos();

  /*
     Ptr to row event extra data to be written to Binlog /
     received from Binlog.

   */
  uchar *binlog_row_event_extra_data;

  int binlog_setup_trx_data();

  /*
    Public interface to write RBR events to the binlog
  */
  int binlog_write_table_map(TABLE *table, bool is_transactional,
                             bool binlog_rows_query);
  int binlog_write_row(TABLE *table, bool is_transactional,
                       const uchar *new_data,
                       const unsigned char *extra_row_info);
  int binlog_delete_row(TABLE *table, bool is_transactional,
                        const uchar *old_data,
                        const unsigned char *extra_row_info);
  int binlog_update_row(TABLE *table, bool is_transactional,
                        const uchar *old_data, const uchar *new_data,
                        const uchar *extra_row_info);

  std::string gen_trx_metadata();

  bool add_time_metadata(rapidjson::Document &meta_data_root);
  bool add_db_metadata(rapidjson::Document &meta_data_root);

  void set_server_id(uint32 sid) { server_id = sid; }

  /*
    Member functions to handle pending event for row-level logging.
  */
  template <class RowsEventT>
  Rows_log_event *binlog_prepare_pending_rows_event(
      TABLE *table, uint32 serv_id, size_t needed, bool is_transactional,
      const unsigned char *extra_row_info, uint32 source_part_id = INT_MAX);
  Rows_log_event *binlog_get_pending_rows_event(bool is_transactional) const;
  inline int binlog_flush_pending_rows_event(bool stmt_end) {
    return (binlog_flush_pending_rows_event(stmt_end, false) ||
            binlog_flush_pending_rows_event(stmt_end, true));
  }
  int binlog_flush_pending_rows_event(bool stmt_end, bool is_transactional);

  void binlog_reset_pending_rows_event(bool is_transactional);

  /**
    Determine the binlog format of the current statement.

    @retval 0 if the current statement will be logged in statement
    format.
    @retval nonzero if the current statement will be logged in row
    format.
   */
  int is_current_stmt_binlog_format_row() const {
    DBUG_ASSERT(current_stmt_binlog_format == BINLOG_FORMAT_STMT ||
                current_stmt_binlog_format == BINLOG_FORMAT_ROW);
    return current_stmt_binlog_format == BINLOG_FORMAT_ROW;
  }

  /**
    Determine if binlogging is currently disabled for this session.

    There are two ways that binlogging can be disabled:

     1. The binary log file is closed (globally). This can happen for
        two reasons: either --skip-log-bin was used on the command line,
        or a binlog write error happened when binlog_error_action=IGNORE_ERROR.

     2. The binary log is disabled on session level. This can happen for
        two reasons: either the user has set @@session.sql_log_bin = 0,
        or the server code has internally disabled the binary log (by
        either setting thd->variables.option_bits &= ~OPTION_BIN_LOG or
        creating a Disable_binlog_guard object).

    Even if this function returns true and the binary log is disabled,
    it is possible that the statement will be written to the binary log,
    in the cases where the server has merely temporarily disabled binary
    logging.

    And even if this function returns false and the binary log is
    enabled, it is possible that the statement will not be written to
    the binary log, e.g. in case it is a no-op, it fails, it gets rolled
    back, or some other session closes the binary log due to a write
    error when using binlog_error_action=IGNORE_ERROR.

    @retval true The binary log is currently disabled for the statement.

    @retval false The binary log is currently enabled for the statement.
  */
  bool is_current_stmt_binlog_disabled() const;

  /* Flag which tells us if the current trx/stmt needs to produce binlogs using
     row logging functions (see @force_write_to_binlog()). Currently this is
     being used only during idempotent recovery where we log the relay log
     contents instead of generating from logging functions */
  bool m_skip_row_logging_functions = false;

  /**
    Determine if binlogging is currently disabled for this session.
    If the binary log is disabled for this thread (either by log_bin=0 or
    sql_log_bin=0 or by log_slave_updates=0 for a slave thread), then the
    statement will not be written to the binary log.

    @retval true The binary log is currently disabled for the statement.

    @retval false The binary log is currently enabled for the statement.
  */
  bool is_current_stmt_binlog_log_slave_updates_disabled() const;

  /**
    Determine if binloging is enabled in row format and write set extraction is
    enabled for this session
    @retval true  if is enable
    @retval false otherwise
  */
  bool is_current_stmt_binlog_row_enabled_with_write_set_extraction() const;

  /**
    Determine if the recovery in idempotent mode is enabled
   */
  bool is_enabled_idempotent_recovery() const noexcept;

  /** Tells whether the given optimizer_switch flag is on */
  inline bool optimizer_switch_flag(ulonglong flag) const {
    return (variables.optimizer_switch & flag);
  }

  enum binlog_filter_state {
    BINLOG_FILTER_UNKNOWN,
    BINLOG_FILTER_CLEAR,
    BINLOG_FILTER_SET
  };

  inline void reset_binlog_local_stmt_filter() {
    m_binlog_filter_state = BINLOG_FILTER_UNKNOWN;
  }

  inline void clear_binlog_local_stmt_filter() {
    DBUG_ASSERT(m_binlog_filter_state == BINLOG_FILTER_UNKNOWN);
    m_binlog_filter_state = BINLOG_FILTER_CLEAR;
  }

  inline void set_binlog_local_stmt_filter() {
    DBUG_ASSERT(m_binlog_filter_state == BINLOG_FILTER_UNKNOWN);
    m_binlog_filter_state = BINLOG_FILTER_SET;
  }

  binlog_filter_state get_binlog_local_stmt_filter() const {
    return m_binlog_filter_state;
  }

  /** Holds active timer object */
  struct THD_timer_info *timer;
  /**
    After resetting(cancelling) timer, current timer object is cached
    with timer_cache timer to reuse.
  */
  struct THD_timer_info *timer_cache;

 private:
  /*
    Indicates that the command which is under execution should ignore the
    'read_only' and 'super_read_only' options.
  */
  bool skip_readonly_check;
  /**
    Indicate if the current statement should be discarded
    instead of written to the binlog.
    This is used to discard special statements, such as
    DML or DDL that affects only 'local' (non replicated)
    tables, such as performance_schema.*
  */
  binlog_filter_state m_binlog_filter_state;

  /**
    Indicates the format in which the current statement will be
    logged.  This can only be set from @c decide_logging_format().
  */
  enum_binlog_format current_stmt_binlog_format;

  /**
    Bit field for the state of binlog warnings.

    The first Lex::BINLOG_STMT_UNSAFE_COUNT bits list all types of
    unsafeness that the current statement has.

    This must be a member of THD and not of LEX, because warnings are
    detected and issued in different places (@c
    decide_logging_format() and @c binlog_query(), respectively).
    Between these calls, the THD->lex object may change; e.g., if a
    stored routine is invoked.  Only THD persists between the calls.
  */
  uint32 binlog_unsafe_warning_flags;

  /*
    Number of outstanding table maps, i.e., table maps in the
    transaction cache.
  */
  uint binlog_table_maps;
  /*
    MTS: db names listing to be updated by the query databases
  */
  List<char> *binlog_accessed_db_names;

  /**
    The binary log position of the transaction.

    The file and position are zero if the current transaction has not
    been written to the binary log.

    @see set_trans_pos
    @see get_trans_pos

    @todo Similar information is kept in the patch for BUG#11762277
    and by the master/slave heartbeat implementation.  We should merge
    these positions instead of maintaining three different ones.
   */
  /**@{*/
  const char *m_trans_log_file;
  const char *m_trans_fixed_log_file;
  char *m_trans_fixed_log_path;
  my_off_t m_trans_end_pos;
  /**
   current transaction gtid in this THD
  */
  const char *m_trans_gtid;
  /**
   max transaction gtid in all THDs
  */
  const char *m_trans_max_gtid;
  char trans_gtid[Gtid::MAX_TEXT_LENGTH + 1];
  char trans_max_gtid[Gtid::MAX_TEXT_LENGTH + 1];
  /**@}*/
  // NOTE: Ideally those two should be in Protocol,
  // but currently its design doesn't allow that.
  NET net;        // client connection descriptor
  String packet;  // dynamic buffer for network I/O

  /**
   * Relay log positions for the transaction
   */
  std::pair<std::string, my_off_t> m_trans_relay_log_pos;

  /* The term and index that need to be communicated across different raft
   * plugin hooks. These fields are not protected by locks since they are
   * accessed by the same THD serially during different stages of ordered commit
   * today. Protect this by locks if things change in future. */
  int64_t term_ = -1;
  int64_t index_ = -1;

  std::string safe_purge_file;

 public:
  bool has_net_vio() const noexcept { return net.vio != nullptr; }
  const Vio *get_net_vio() const noexcept { return net.vio; }
  bool has_net_vio_ssl_arg() const noexcept {
    return has_net_vio() && get_net_vio()->ssl_arg != nullptr;
  }
  const void *get_net_vio_ssl_arg() const noexcept {
    return has_net_vio() ? get_net_vio()->ssl_arg : nullptr;
  }

  void set_skip_readonly_check() { skip_readonly_check = true; }

  bool is_cmd_skip_readonly() const { return skip_readonly_check; }

  void reset_skip_readonly_check() {
    if (skip_readonly_check) skip_readonly_check = false;
  }

  void issue_unsafe_warnings();

  uint get_binlog_table_maps() const { return binlog_table_maps; }
  void clear_binlog_table_maps() { binlog_table_maps = 0; }

  /**
   * Clear the raft opid which was cached, in preparation for next apply round
   * Should be only called at a safe point, like finish_commit or ends_group of
   * a slave applier.
   *
   */
  void clear_raft_opid() {
    term_ = -1;
    index_ = -1;
  }

  /*
    MTS: accessor to binlog_accessed_db_names list
  */
  List<char> *get_binlog_accessed_db_names() const {
    return binlog_accessed_db_names;
  }

  /* MTS: method inserts a new unique name into binlog_updated_dbs */
  void add_to_binlog_accessed_dbs(const char *db);

 private:
  std::unique_ptr<Transaction_ctx> m_transaction;

  /** An utility struct for @c Attachable_trx */
  struct Transaction_state {
    Transaction_state();
    ~Transaction_state();
    void backup(THD *thd);
    void restore(THD *thd);

    /// SQL-command.
    enum_sql_command m_sql_command;

    Query_tables_list *m_query_tables_list;

    /// Open-tables state.
    Open_tables_backup m_open_tables_state;

    /// SQL_MODE.
    sql_mode_t m_sql_mode;

    /// Transaction isolation level.
    enum_tx_isolation m_tx_isolation;

    /// Ha_data array.
    Prealloced_array<Ha_data, PREALLOC_NUM_HA> m_ha_data;

    /// Transaction_ctx instance.
    Transaction_ctx *m_trx;

    /// Transaction read-only state.
    bool m_tx_read_only;

    /// THD options.
    ulonglong m_thd_option_bits;

    /// Current transaction instrumentation.
    PSI_transaction_locker *m_transaction_psi;

    /// Server status flags.
    uint m_server_status;

    /// THD::in_lock_tables value.
    bool m_in_lock_tables;

    /**
      Current time zone (i.e. @@session.time_zone) usage indicator.

      Saving it allows data-dictionary code to read timestamp values
      as datetimes from system tables without disturbing user's statement.

      TODO: We need to change DD code not to use @@session.time_zone at all and
      stick to UTC for internal storage of timestamps in DD objects.
    */
    bool m_time_zone_used;

    /**
      Transaction rollback request flag.

      InnoDB can try to access table definition while rolling back regular
      transaction. So we need to be able to start attachable transaction
      without being affected by, and affecting, the rollback state of regular
      transaction.
    */
    bool m_transaction_rollback_request;
  };

 public:
  enum enum_reset_lex { RESET_LEX, DO_NOT_RESET_LEX };

 private:
  /**
    Class representing read-only attachable transaction, encapsulates
    knowledge how to backup state of current transaction, start
    read-only attachable transaction in SE, finalize it and then restore
    state of original transaction back. Also serves as a base class for
    read-write attachable transaction implementation.
  */
  class Attachable_trx {
   public:
    Attachable_trx(THD *thd, Attachable_trx *prev_trx);
    virtual ~Attachable_trx();
    Attachable_trx *get_prev_attachable_trx() const {
      return m_prev_attachable_trx;
    }
    virtual bool is_read_only() const { return true; }

   protected:
    /// THD instance.
    THD *m_thd;

    enum_reset_lex m_reset_lex;

    /**
      Attachable_trx which was active for the THD before when this
      transaction was started (NULL in most cases).
    */
    Attachable_trx *m_prev_attachable_trx;

    /// Transaction state data.
    Transaction_state m_trx_state;

   private:
    Attachable_trx(const Attachable_trx &);
    Attachable_trx &operator=(const Attachable_trx &);
  };

  /**
    A derived from THD::Attachable_trx class allows updates in
    the attachable transaction. Callers of the class methods must
    make sure the attachable_rw won't cause deadlock with the main transaction.
    The destructor does not invoke ha_commit_{stmt,trans} nor ha_rollback_trans
    on purpose.
    Burden to terminate the read-write instance also lies on the caller!
    In order to use this interface it *MUST* prove that no side effect to
    the global transaction state can be inflicted by a chosen method.

    This class is being used only by class Gtid_table_access_context by
    replication and by dd::info_schema::Table_statistics.
  */

  class Attachable_trx_rw : public Attachable_trx {
   public:
    bool is_read_only() const { return false; }
    explicit Attachable_trx_rw(THD *thd);

   private:
    Attachable_trx_rw(const Attachable_trx_rw &);
    Attachable_trx_rw &operator=(const Attachable_trx_rw &);
  };

  Attachable_trx *m_attachable_trx;

 public:
  Transaction_ctx *get_transaction() { return m_transaction.get(); }

  const Transaction_ctx *get_transaction() const { return m_transaction.get(); }

  /**
    Changes the Transaction_ctx instance within THD-object. The previous
    Transaction_ctx instance is destroyed.

    @note this is a THD-internal operation which MUST NOT be used outside.

    @param transaction_ctx new Transaction_ctx instance to be associated with
    the THD-object.
  */
  void set_transaction(Transaction_ctx *transaction_ctx);

  Global_read_lock global_read_lock;

  Vio *active_vio = {nullptr};

  /* Active network vio for clone remote connection. */
  Vio *clone_vio = {nullptr};

  /*
    This is to track items changed during execution of a prepared
    statement/stored procedure. It's created by
    register_item_tree_change() in memory root of THD, and freed in
    rollback_item_tree_changes(). For conventional execution it's always
    empty.
  */
  Item_change_list change_list;

  /*
    A permanent memory area of the statement. For conventional
    execution, the parsed tree and execution runtime reside in the same
    memory root. In this case stmt_arena points to THD. In case of
    a prepared statement or a stored procedure statement, thd->mem_root
    conventionally points to runtime memory, and thd->stmt_arena
    points to the memory of the PS/SP, where the parsed tree of the
    statement resides. Whenever you need to perform a permanent
    transformation of a parsed tree, you should allocate new memory in
    stmt_arena, to allow correct re-execution of PS/SP.
    Note: in the parser, stmt_arena == thd, even for PS/SP.
  */
  Query_arena *stmt_arena;

  /*
    map for tables that will be updated for a multi-table update query
    statement, for other query statements, this will be zero.
  */
  table_map table_map_for_update;

  /* Tells if LAST_INSERT_ID(#) was called for the current statement */
  bool arg_of_last_insert_id_function;
  /*
    ALL OVER THIS FILE, "insert_id" means "*automatically generated* value for
    insertion into an auto_increment column".
  */
  /*
    This is the first autogenerated insert id which was *successfully*
    inserted by the previous statement (exactly, if the previous statement
    didn't successfully insert an autogenerated insert id, then it's the one
    of the statement before, etc).
    It can also be set by SET LAST_INSERT_ID=# or SELECT LAST_INSERT_ID(#).
    It is returned by LAST_INSERT_ID().
  */
  ulonglong first_successful_insert_id_in_prev_stmt;
  /*
    Variant of the above, used for storing in statement-based binlog. The
    difference is that the one above can change as the execution of a stored
    function progresses, while the one below is set once and then does not
    change (which is the value which statement-based binlog needs).
  */
  ulonglong first_successful_insert_id_in_prev_stmt_for_binlog;
  /*
    This is the first autogenerated insert id which was *successfully*
    inserted by the current statement. It is maintained only to set
    first_successful_insert_id_in_prev_stmt when statement ends.
  */
  ulonglong first_successful_insert_id_in_cur_stmt;
  /*
    We follow this logic:
    - when stmt starts, first_successful_insert_id_in_prev_stmt contains the
    first insert id successfully inserted by the previous stmt.
    - as stmt makes progress, handler::insert_id_for_cur_row changes;
    every time get_auto_increment() is called,
    auto_inc_intervals_in_cur_stmt_for_binlog is augmented with the
    reserved interval (if statement-based binlogging).
    - at first successful insertion of an autogenerated value,
    first_successful_insert_id_in_cur_stmt is set to
    handler::insert_id_for_cur_row.
    - when stmt goes to binlog,
    auto_inc_intervals_in_cur_stmt_for_binlog is binlogged if
    non-empty.
    - when stmt ends, first_successful_insert_id_in_prev_stmt is set to
    first_successful_insert_id_in_cur_stmt.
  */
  /*
    stmt_depends_on_first_successful_insert_id_in_prev_stmt is set when
    LAST_INSERT_ID() is used by a statement.
    If it is set, first_successful_insert_id_in_prev_stmt_for_binlog will be
    stored in the statement-based binlog.
    This variable is CUMULATIVE along the execution of a stored function or
    trigger: if one substatement sets it to 1 it will stay 1 until the
    function/trigger ends, thus making sure that
    first_successful_insert_id_in_prev_stmt_for_binlog does not change anymore
    and is propagated to the caller for binlogging.
  */
  bool stmt_depends_on_first_successful_insert_id_in_prev_stmt;
  /*
    List of auto_increment intervals reserved by the thread so far, for
    storage in the statement-based binlog.
    Note that its minimum is not first_successful_insert_id_in_cur_stmt:
    assuming a table with an autoinc column, and this happens:
    INSERT INTO ... VALUES(3);
    SET INSERT_ID=3; INSERT IGNORE ... VALUES (NULL);
    then the latter INSERT will insert no rows
    (first_successful_insert_id_in_cur_stmt == 0), but storing "INSERT_ID=3"
    in the binlog is still needed; the list's minimum will contain 3.
    This variable is cumulative: if several statements are written to binlog
    as one (stored functions or triggers are used) this list is the
    concatenation of all intervals reserved by all statements.
  */
  Discrete_intervals_list auto_inc_intervals_in_cur_stmt_for_binlog;
  /* Used by replication and SET INSERT_ID */
  Discrete_intervals_list auto_inc_intervals_forced;
  /*
    There is BUG#19630 where statement-based replication of stored
    functions/triggers with two auto_increment columns breaks.
    We however ensure that it works when there is 0 or 1 auto_increment
    column; our rules are
    a) on master, while executing a top statement involving substatements,
    first top- or sub- statement to generate auto_increment values wins the
    exclusive right to see its values be written to binlog (the write
    will be done by the statement or its caller), and the losers won't see
    their values be written to binlog.
    b) on slave, while replicating a top statement involving substatements,
    first top- or sub- statement to need to read auto_increment values from
    the master's binlog wins the exclusive right to read them (so the losers
    won't read their values from binlog but instead generate on their own).
    a) implies that we mustn't backup/restore
    auto_inc_intervals_in_cur_stmt_for_binlog.
    b) implies that we mustn't backup/restore auto_inc_intervals_forced.

    If there are more than 1 auto_increment columns, then intervals for
    different columns may mix into the
    auto_inc_intervals_in_cur_stmt_for_binlog list, which is logically wrong,
    but there is no point in preventing this mixing by preventing intervals
    from the secondly inserted column to come into the list, as such
    prevention would be wrong too.
    What will happen in the case of
    INSERT INTO t1 (auto_inc) VALUES(NULL);
    where t1 has a trigger which inserts into an auto_inc column of t2, is
    that in binlog we'll store the interval of t1 and the interval of t2 (when
    we store intervals, soon), then in slave, t1 will use both intervals, t2
    will use none; if t1 inserts the same number of rows as on master,
    normally the 2nd interval will not be used by t1, which is fine. t2's
    values will be wrong if t2's internal auto_increment counter is different
    from what it was on master (which is likely). In 5.1, in mixed binlogging
    mode, row-based binlogging is used for such cases where two
    auto_increment columns are inserted.
  */
  inline void record_first_successful_insert_id_in_cur_stmt(ulonglong id_arg) {
    if (first_successful_insert_id_in_cur_stmt == 0)
      first_successful_insert_id_in_cur_stmt = id_arg;
  }
  inline ulonglong read_first_successful_insert_id_in_prev_stmt(void) {
    if (!stmt_depends_on_first_successful_insert_id_in_prev_stmt) {
      /* It's the first time we read it */
      first_successful_insert_id_in_prev_stmt_for_binlog =
          first_successful_insert_id_in_prev_stmt;
      stmt_depends_on_first_successful_insert_id_in_prev_stmt = true;
    }
    return first_successful_insert_id_in_prev_stmt;
  }
  inline void reset_first_successful_insert_id() {
    arg_of_last_insert_id_function = false;
    first_successful_insert_id_in_prev_stmt = 0;
    first_successful_insert_id_in_cur_stmt = 0;
    first_successful_insert_id_in_prev_stmt_for_binlog = 0;
    stmt_depends_on_first_successful_insert_id_in_prev_stmt = false;
  }

  /*
    Used by Intvar_log_event::do_apply_event() and by "SET INSERT_ID=#"
    (mysqlbinlog). We'll soon add a variant which can take many intervals in
    argument.
  */
  inline void force_one_auto_inc_interval(ulonglong next_id) {
    auto_inc_intervals_forced.empty();  // in case of multiple SET INSERT_ID
    auto_inc_intervals_forced.append(next_id, ULLONG_MAX, 0);
  }

  /* Was there an error during consensus commit of a trx? This is usually set
   * when the raft plugin fails to obtain majority acks (in before_commit hook)
   */
  bool commit_consensus_error = false;

  /**
    Stores the result of the FOUND_ROWS() function.  Set at query end, stable
    throughout the query.
  */
  ulonglong previous_found_rows;
  /**
    Dynamic, collected and set also in subqueries. Not stable throughout query.
    previous_found_rows is a snapshot of this take at query end making it
    stable throughout the next query, see update_previous_found_rows.
  */
  ulonglong current_found_rows;

  /*
    Indicate if the gtid_executed table is being operated implicitly
    within current transaction. This happens because we are inserting
    a GTID specified through SET GTID_NEXT by user client or
    slave SQL thread/workers.
  */
  bool is_operating_gtid_table_implicitly;
  /*
    Indicate that a sub-statement is being operated implicitly
    within current transaction.
    As we don't want that this implicit sub-statement to consume the
    GTID of the actual transaction, we set it true at the beginning of
    the sub-statement and set it false again after "committing" the
    sub-statement.
    When it is true, the applier will not save the transaction owned
    gtid into mysql.gtid_executed table before transaction prepare, as
    it does when binlog is disabled, or binlog is enabled and
    log_slave_updates is disabled.
    Also the flag is made to defer updates to the slave info table from
    intermediate commits by non-atomic DDL.
    Rpl_info_table::do_flush_info(), rpl_rli.h::is_atomic_ddl_commit_on_slave()
    uses this flag.
  */
  bool is_operating_substatement_implicitly;

 private:
  /**
    Stores the result of ROW_COUNT() function.

    ROW_COUNT() function is a MySQL extension, but we try to keep it
    similar to ROW_COUNT member of the GET DIAGNOSTICS stack of the SQL
    standard (see SQL99, part 2, search for ROW_COUNT). It's value is
    implementation defined for anything except INSERT, DELETE, UPDATE.

    ROW_COUNT is assigned according to the following rules:

      - In my_ok():
        - for DML statements: to the number of affected rows;
        - for DDL statements: to 0.

      - In my_eof(): to -1 to indicate that there was a result set.

        We derive this semantics from the JDBC specification, where int
        java.sql.Statement.getUpdateCount() is defined to (sic) "return the
        current result as an update count; if the result is a ResultSet
        object or there are no more results, -1 is returned".

      - In my_error(): to -1 to be compatible with the MySQL C API and
        MySQL ODBC driver.

      - For SIGNAL statements: to 0 per WL#2110 specification (see also
        sql_signal.cc comment). Zero is used since that's the "default"
        value of ROW_COUNT in the Diagnostics Area.
  */

  longlong m_row_count_func; /* For the ROW_COUNT() function */

 public:
  inline longlong get_row_count_func() const { return m_row_count_func; }

  inline void set_row_count_func(longlong row_count_func) {
    m_row_count_func = row_count_func;
  }

  ha_rows num_truncated_fields;

 private:
  /**
    Number of rows we actually sent to the client, including "synthetic"
    rows in ROLLUP etc.
  */
  ha_rows m_sent_row_count;

  /**
    Number of rows read and/or evaluated for a statement. Used for
    slow log reporting.

    An examined row is defined as a row that is read and/or evaluated
    according to a statement condition, including in
    create_sort_index(). Rows may be counted more than once, e.g., a
    statement including ORDER BY could possibly evaluate the row in
    filesort() before reading it for e.g. update.
  */
  ha_rows m_examined_row_count;

  /**
     Total binlog bytes written per stmt.
  */
  ulonglong m_binlog_bytes_written;

  /**
     Captures the time when we start writing rows for a stmt.
  */
  timespec m_stmt_start_write_time;

  /**
     Whether we set the value of m_stmt_start_write_time.
  */
  bool m_stmt_start_write_time_is_set;

  /**
     Total time(micro-secs) spent writing rows for stmt.
  */
  ulonglong m_stmt_total_write_time;

  /**
     DML row count for the transaction. This is incremented as the
     DML operations take place
  */
  ha_rows m_trx_dml_row_count;

  /* record that a warning that a DML cpu time exceeded the limit was raised */
  bool m_trx_dml_cpu_time_limit_warning = false;

  timespec m_trx_dml_start_time;

  bool m_trx_dml_start_time_is_set = false;

 private:
  using explicit_snapshot_ptr = std::shared_ptr<explicit_snapshot>;
  explicit_snapshot_ptr m_explicit_snapshot;

 public:
  /**
   * Creates an explicit snapshot and associates it with the current conn
   * return true if error, false otherwise
   */
  bool create_explicit_snapshot();

  explicit_snapshot_ptr get_explicit_snapshot() const {
    return m_explicit_snapshot;
  }

  void set_explicit_snapshot(explicit_snapshot_ptr s) {
    m_explicit_snapshot = std::move(s);
  }

  /**
   * Attaches an existing explicit snapshot to the current conn
   * return true if error, false otherwise
   */
  bool attach_explicit_snapshot(const ulonglong snapshot_id);

  /**
   * Releases the explicit snapshot associated with the current conn
   * return true if error, false otherwise
   */
  bool release_explicit_snapshot();

 private:
  USER_CONN *m_user_connect;

  void update_global_binlog_max_gtid(void);

  typedef struct {
    std::string db_metadata;
    std::string db_shard_id;
  } db_context_t;
  std::unordered_map<std::string, db_context_t> m_db_context_hash;
  db_context_t *m_current_db_context;

 public:
  void set_user_connect(USER_CONN *uc);
  const USER_CONN *get_user_connect() const { return m_user_connect; }

  void increment_user_connections_counter();
  void decrement_user_connections_counter();

  void increment_con_per_hour_counter();

  void increment_updates_counter();

  void increment_questions_counter();

  void time_out_user_resource_limits();

  const char *get_user_name() {
    return security_context()->user().length > 0
               ? security_context()->user().str
               : "NULL";
  }

  const char *get_db_name() { return m_db.str != NULL ? m_db.str : "NULL"; }

 public:
  ha_rows get_sent_row_count() const { return m_sent_row_count; }

  ha_rows get_examined_row_count() const { return m_examined_row_count; }

  void reset_stmt_stats();

  ulonglong get_row_binlog_bytes_written() const {
    return m_binlog_bytes_written;
  }

  void inc_row_binlog_bytes_written(ulonglong val) {
    m_binlog_bytes_written += val;
  }

  ulonglong get_stmt_total_write_time() const {
    return m_stmt_total_write_time;
  }

  void set_stmt_total_write_time();

  void set_stmt_start_write_time();

  ulong get_query_or_connect_attr_value(const char *attr_name,
                                        ulong default_value, ulong max_value);

  const std::string &get_query_attr(const std::string &qattr_key);
  const std::string &get_connection_attr(const std::string &cattr_key);

  std::list<std::pair<const char *, const char *>> get_query_tables();
  std::pair<std::list<std::pair<const char *, const char *>>,
            std::list<std::pair<const char *, const char *>>>
  get_read_write_tables();

  void get_mt_keys_for_write_query(
      std::array<std::string, WRITE_STATISTICS_DIMENSION_COUNT> &keys);

  enum_control_level get_mt_throttle_tag_level() const;

  void set_sent_row_count(ha_rows count);

  void set_trx_dml_row_count(ha_rows count) { m_trx_dml_row_count = count; }

  ha_rows get_trx_dml_row_count() { return m_trx_dml_row_count; }

  void set_trx_dml_cpu_time_limit_warning(bool val) {
    m_trx_dml_cpu_time_limit_warning = val;
  }

  void set_trx_dml_start_time();

  ulonglong get_trx_dml_write_time();
  bool dml_execution_cpu_limit_exceeded();

  void inc_sent_row_count(ha_rows count);
  void inc_examined_row_count(ha_rows count);
  void inc_deleted_row_count(ha_rows count);
  void inc_inserted_row_count(ha_rows count);
  void inc_updated_row_count(ha_rows count);

  void inc_status_created_tmp_disk_tables();
  void inc_status_created_tmp_tables();
  void inc_status_select_full_join();
  void inc_status_select_full_range_join();
  void inc_status_select_range();
  void inc_status_select_range_check();
  void inc_status_select_scan();
  void inc_status_sort_merge_passes();
  void inc_status_sort_range();
  void inc_status_sort_rows(ha_rows count);
  void inc_status_sort_scan();
  void set_status_no_index_used();
  void set_status_no_good_index_used();

  void capture_system_thread_id();

  /* Reset status vars on REFRESH_STATUS. */
  void reset_status_vars();

  /* Adjust disk usage for current session. */
  void adjust_filesort_disk_usage(longlong delta);
  void adjust_tmp_table_disk_usage(longlong delta);
  void propagate_pending_global_disk_usage();

  /* Get current disk usage and peak. */
  ulonglong get_filesort_disk_usage() { return m_filesort_disk_usage; }
  ulonglong get_filesort_disk_usage_peak() {
    return m_filesort_disk_usage_peak;
  }
  ulonglong get_tmp_table_disk_usage() { return m_tmp_table_disk_usage; }
  ulonglong get_tmp_table_disk_usage_peak() {
    return m_tmp_table_disk_usage_peak;
  }

 private:
  /* Current filesort usage and peak since last reset. */
  ulonglong m_filesort_disk_usage{0};
  std::atomic<ulonglong> m_filesort_disk_usage_peak{0};

  /* Current tmp table usage and peak since last reset. */
  ulonglong m_tmp_table_disk_usage{0};
  std::atomic<ulonglong> m_tmp_table_disk_usage_peak{0};

  /* Reporting of session disk usage to global counters is done in batches
     to avoid contention on global variables. */
  std::atomic<longlong> m_unreported_global_filesort_delta{0};
  std::atomic<longlong> m_unreported_global_tmp_table_delta{0};

  /* Offsets for statement peak disk usage. */
  ulonglong m_stmt_filesort_disk_usage_offset{0};
  ulonglong m_stmt_tmp_table_disk_usage_offset{0};

 public:
  std::unordered_map<std::string, enum_db_read_only> m_db_read_only_hash;
  std::unordered_map<std::string, const CHARSET_INFO *> m_db_default_collation;
  const CHARSET_INFO *db_charset;

  void update_db_metadata(const char *db_name, const std::string &metadata);
  void remove_db_metadata(const char *db_name);

#if defined(ENABLED_PROFILING)
  std::unique_ptr<PROFILING> profiling;
#endif

  /** Current stage progress instrumentation. */
  PSI_stage_progress *m_stage_progress_psi;
  /** Current statement digest. */
  sql_digest_state *m_digest;
  /** Current statement digest token array. */
  unsigned char *m_token_array;
  /** Top level statement digest. */
  sql_digest_state m_digest_state;

  void get_query_digest(String *digest_buffer, const char **str, size_t *length,
                        const CHARSET_INFO **cs);

  /** Types of keys used to track various attributes of a query
      For each key we maintain an ID computed as a MD5 and a status
      of whether the ID has been set or not (i.e, valid for read).
      Both are stored in an array indexed by the corresponding enum value

      SQL ID    = compute_digest_hash(statement digest)
      Client ID = compute_md5_hash(client attributes)
      Plan ID   = compute_md5_hash(sql plan)
      SQL Hash  = compute_md5_hash(statement text + flags from QC)
   */
  enum enum_mt_key {
    SQL_ID = 0,
    CLIENT_ID = 1,
    SQL_HASH = 2,
    //    PLAN_ID   = 3
    MT_KEY_MAX
  };

  digest_key mt_key_val[MT_KEY_MAX] = {};
  std::atomic_bool mt_key_val_set[MT_KEY_MAX];

  /* SQL_ID and SQL_PLAN may be accessed by another thread executing
     SHOW PROCESSLIST or a SQL reading from I_S.PROCESSLIST
  */
  void mt_mutex_lock(enum_mt_key key_name) {
    if (key_name == SQL_ID || key_name == CLIENT_ID
        // || key_name == PLAN_ID
    )
      mysql_mutex_lock(&LOCK_thd_data);
  }
  void mt_mutex_unlock(enum_mt_key key_name) {
    if (key_name == SQL_ID || key_name == CLIENT_ID
        // || key_name == PLAN_ID
    )
      mysql_mutex_unlock(&LOCK_thd_data);
  }
  void mt_key_set(enum_mt_key key_name, const unsigned char *key_val,
                  uint len) {
    DBUG_ASSERT(key_name < MT_KEY_MAX && len <= DIGEST_HASH_SIZE);
    mt_mutex_lock(key_name);
    memcpy(mt_key_val[key_name].data(), key_val, len);
    mt_key_val_set[key_name] = true;
    mt_mutex_unlock(key_name);
  }
  bool mt_key_is_set(enum_mt_key key_name) {
    DBUG_ASSERT(key_name < MT_KEY_MAX);
    return mt_key_val_set[key_name];
  }
  digest_key &mt_key_value(enum_mt_key key_name) {
    DBUG_ASSERT(key_name < MT_KEY_MAX);
    return mt_key_val[key_name];
  }
  void mt_hex_value(enum_mt_key key_name, char *hex_val, uint len);
  void mt_key_clear(enum_mt_key key_name) {
    DBUG_ASSERT(key_name < MT_KEY_MAX);
    mt_key_val_set[key_name] = false;
  }

  /**
    Check whether we can raise warning/error when gap locks are used in a query
    @return true if we can raise one, false otherwise.
  */
  bool gap_lock_raise_allowed() {
    return (variables.gap_lock_raise_error != GAP_LOCK_RAISE_OFF ? true
                                                                 : false);
  }

  /**
    Check whether we can raise a warning when gap locks are used in a query
    @return true if we can raise a warning, false otherwise.
  */
  bool gap_lock_raise_warning() {
    return (variables.gap_lock_raise_error == GAP_LOCK_RAISE_WARNING ? true
                                                                     : false);
  }

  /**
    Check whether we can raise am error when gap locks are used in a query
    @return true if we can raise an error, false otherwise.
  */
  bool gap_lock_raise_error() {
    return (variables.gap_lock_raise_error == GAP_LOCK_RAISE_ERROR ? true
                                                                   : false);
  }

  /** Current statement instrumentation. */
  PSI_statement_locker *m_statement_psi;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  /** Current statement instrumentation state. */
  PSI_statement_locker_state m_statement_state;
#endif /* HAVE_PSI_STATEMENT_INTERFACE */

  /** Current transaction instrumentation. */
  PSI_transaction_locker *m_transaction_psi;
#ifdef HAVE_PSI_TRANSACTION_INTERFACE
  /** Current transaction instrumentation state. */
  PSI_transaction_locker_state m_transaction_state;
#endif /* HAVE_PSI_TRANSACTION_INTERFACE */

  /** Idle instrumentation. */
  PSI_idle_locker *m_idle_psi;
#ifdef HAVE_PSI_IDLE_INTERFACE
  /** Idle instrumentation state. */
  PSI_idle_locker_state m_idle_state;
#endif /* HAVE_PSI_IDLE_INTERFACE */
  /** True if the server code is IDLE for this connection. */
  bool m_server_idle;

  /*
    Id of current query. Statement can be reused to execute several queries
    query_id is global in context of the whole MySQL server.
    ID is automatically generated from mutex-protected counter.
    It's used in handler code for various purposes: to check which columns
    from table are necessary for this select, to check if it's necessary to
    update auto-updatable fields (like auto_increment and timestamp).
  */
  query_id_t query_id;
  ulong col_access;

  /* Statement id is thread-wide. This counter is used to generate ids */
  ulong statement_id_counter;
  ulong rand_saved_seed1, rand_saved_seed2;
  my_thread_t real_id; /* For debugging */
                       /**
                         This counter is 32 bit because of the client protocol.
                         @note It is not meant to be used for my_thread_self(), see @c real_id for
                         this.
                         @note Set to reserved_thread_id on initialization. This is a magic
                         value that is only to be used for temporary THDs not present in
                         the global THD list.
                       */
 private:
  my_thread_id m_thread_id;
#ifdef _WIN32
  uint m_system_thread_id;
#else
  pid_t m_system_thread_id;
#endif  // _WIN32

 public:
  /**
    Assign a value to m_thread_id by calling
    Global_THD_manager::get_new_thread_id().
  */
  void set_new_thread_id();
  my_thread_id thread_id() const { return m_thread_id; }
  ulong system_thread_id() const { return m_system_thread_id; }
  uint tmp_table;
  uint server_status, open_options;
  enum enum_thread_type system_thread;

  // Check if this THD belongs to a system thread.
  bool is_system_thread() const { return system_thread != NON_SYSTEM_THREAD; }

  // Check if this THD belongs to a dd bootstrap system thread.
  bool is_dd_system_thread() const {
    return system_thread == SYSTEM_THREAD_DD_INITIALIZE ||
           system_thread == SYSTEM_THREAD_DD_RESTART;
  }

  // Check if this THD belongs to the initialize system thread. The
  // initialize thread executes statements that are compiled into the
  // server.
  bool is_initialize_system_thread() const {
    return system_thread == SYSTEM_THREAD_SERVER_INITIALIZE;
  }

  // Check if this THD is executing statements passed through a init file.
  bool is_init_file_system_thread() const {
    return system_thread == SYSTEM_THREAD_INIT_FILE;
  }

  // Check if this THD belongs to a bootstrap system thread. Note that
  // this thread type may execute statements submitted by the user.
  bool is_bootstrap_system_thread() const {
    return is_dd_system_thread() || is_initialize_system_thread() ||
           is_init_file_system_thread();
  }

  // Check if this THD belongs to a server upgrade thread. Server upgrade
  // threads execute statements that are compiled into the server.
  bool is_server_upgrade_thread() const {
    return system_thread == SYSTEM_THREAD_SERVER_UPGRADE;
  }

  /*
    Current or next transaction isolation level.
    When a connection is established, the value is taken from
    @@session.tx_isolation (default transaction isolation for
    the session), which is in turn taken from @@global.tx_isolation
    (the global value).
    If there is no transaction started, this variable
    holds the value of the next transaction's isolation level.
    When a transaction starts, the value stored in this variable
    becomes "actual".
    At transaction commit or rollback, we assign this variable
    again from @@session.tx_isolation.
    The only statement that can otherwise change the value
    of this variable is SET TRANSACTION ISOLATION LEVEL.
    Its purpose is to effect the isolation level of the next
    transaction in this session. When this statement is executed,
    the value in this variable is changed. However, since
    this statement is only allowed when there is no active
    transaction, this assignment (naturally) only affects the
    upcoming transaction.
    At the end of the current active transaction the value is
    be reset again from @@session.tx_isolation, as described
    above.
  */
  enum_tx_isolation tx_isolation;
  /*
    Current or next transaction access mode.
    See comment above regarding tx_isolation.
  */
  bool tx_read_only;
  /*
    Transaction cannot be rolled back must be given priority.
    When two transactions conflict inside InnoDB, the one with
    greater priority wins.
  */
  int tx_priority;
  /*
    All transactions executed by this thread will have high
    priority mode, independent of tx_priority value.
  */
  int thd_tx_priority;

  enum_check_fields check_for_truncated_fields;

  // For user variables replication
  Prealloced_array<Binlog_user_var_event *, 2> user_var_events;
  MEM_ROOT *user_var_events_alloc; /* Allocate above array elements here */

  /**
    Used by MYSQL_BIN_LOG to maintain the commit queue for binary log
    group commit.
  */
  THD *next_to_commit;

  /**
    The member is served for marking a query that CREATEs or ALTERs
    a table declared with a TIMESTAMP column as dependent on
    @@session.explicit_defaults_for_timestamp.
    Is set to true by parser, unset at the end of the query.
    Possible marking in checked by binary logger.
  */
  bool binlog_need_explicit_defaults_ts;

  /**
     Functions to set and get transaction position.

     These functions are used to set the transaction position for the
     transaction written when committing this transaction.
   */
  /**@{*/
  void set_trans_pos(const char *file, my_off_t pos) {
    DBUG_TRACE;
    DBUG_ASSERT(((file == nullptr) && (pos == 0)) ||
                ((file != nullptr) && (pos != 0)));
    if (file) {
      DBUG_PRINT("enter", ("file: %s, pos: %llu", file, pos));
      // Only the file name should be used, not the full path
      m_trans_log_file = file + dirname_length(file);
      if (!m_trans_fixed_log_path)
        m_trans_fixed_log_path = (char *)main_mem_root.Alloc(FN_REFLEN + 1);
      DBUG_ASSERT(strlen(file) <= FN_REFLEN);
      strcpy(m_trans_fixed_log_path, file);
      m_trans_fixed_log_file =
          m_trans_fixed_log_path + dirname_length(m_trans_fixed_log_path);
    } else {
      m_trans_log_file = nullptr;
      m_trans_fixed_log_file = m_trans_fixed_log_path = nullptr;
    }

    m_trans_end_pos = pos;

    if (!owned_gtid.is_empty() && owned_gtid.gno > 0) {
      owned_gtid.to_string(global_sid_map, trans_gtid, true);
      m_trans_gtid = trans_gtid;

      update_global_binlog_max_gtid();
      m_trans_max_gtid = trans_max_gtid;
    } else {
      m_trans_gtid = nullptr;
    }
    DBUG_PRINT("return",
               ("m_trans_log_file: %s, m_trans_fixed_log_file: %s, "
                "m_trans_end_pos: %llu, "
                "m_trans_gtid: %s, m_trans_max_gtid: %s",
                m_trans_log_file, m_trans_fixed_log_file, m_trans_end_pos,
                m_trans_gtid != nullptr ? m_trans_gtid : "<null>",
                m_trans_max_gtid != nullptr ? m_trans_max_gtid : "<null>"));
    return;
  }

  void get_trans_pos(const char **file_var, my_off_t *pos_var,
                     const char **gtid_var, const char **max_gtid_var) const {
    DBUG_TRACE;
    if (file_var) *file_var = m_trans_log_file;
    if (pos_var) *pos_var = m_trans_end_pos;
    if (gtid_var) *gtid_var = m_trans_gtid;
    if (max_gtid_var) *max_gtid_var = m_trans_max_gtid;

    DBUG_PRINT(
        "return",
        ("file: %s, pos: %llu, gtid: %s", file_var ? *file_var : "<none>",
         pos_var ? *pos_var : 0, gtid_var ? m_trans_gtid : "<none>"));
    return;
  }

  const char *get_trans_fixed_log_path() const {
    return m_trans_fixed_log_path;
  }

  void get_trans_fixed_pos(const char **file_var, my_off_t *pos_var) const {
    DBUG_TRACE;
    if (file_var) *file_var = m_trans_fixed_log_file;
    if (pos_var) *pos_var = m_trans_end_pos;
    DBUG_PRINT("return",
               ("file: %s, pos: %llu", file_var ? *file_var : "<none>",
                pos_var ? *pos_var : 0));
    return;
  }

  void get_trans_relay_log_pos(const char **file_var, my_off_t *pos_var) const {
    if (file_var) *file_var = m_trans_relay_log_pos.first.c_str();
    if (pos_var) *pos_var = m_trans_relay_log_pos.second;
  }

  void set_trans_relay_log_pos(const std::string &file, my_off_t pos) {
    m_trans_relay_log_pos.first = file;
    m_trans_relay_log_pos.second = pos;
  }

  /**@}*/

  /* Get the trans marker i.e (term, index) tuple stashed in this THD */
  void get_trans_marker(int64_t *term, int64_t *index) const;

  /* Stash the trans marker i.e (term, index) tuple in this THD */
  void set_trans_marker(int64_t term, int64_t index);

  /* Returns the file that is considered safe to be deleted */
  std::string get_safe_purge_file() const;

  /* Sets the file that is considered safe to be deleted  */
  void set_safe_purge_file(std::string purge_file);

  /* Clear the safe purge file */
  void clear_safe_purge_file();

  /*
    Error code from committing or rolling back the transaction.
  */
  enum Commit_error {
    CE_NONE = 0,
    CE_FLUSH_ERROR,
    CE_SYNC_ERROR,
    CE_COMMIT_ERROR,
    CE_ERROR_COUNT
  } commit_error;

  /*
    Define durability properties that engines may check to
    improve performance.
  */
  enum durability_properties durability_property;

  /*
    If checking this in conjunction with a wait condition, please
    include a check after enter_cond() if you want to avoid a race
    condition. For details see the implementation of awake(),
    especially the "broadcast" part.
  */
  enum killed_state {
    NOT_KILLED = 0,
    KILL_CONNECTION = ER_SERVER_SHUTDOWN,
    KILL_QUERY = ER_QUERY_INTERRUPTED,
    KILL_TIMEOUT = ER_QUERY_TIMEOUT,
    KILLED_NO_VALUE /* means neither of the states */
  };
  std::atomic<killed_state> killed;
  char *killed_reason;

  /**
    Whether we are currently in the execution phase of an EXPLAIN ANALYZE query.
    If so, send_kill_message() won't actually set an error; we will add a
    warning near the end of the execution instead.
   */
  bool running_explain_analyze = false;

  /**
    When operation on DD tables is in progress then THD is set to kill immune
    mode.
    This member holds DD_kill_immunizer object created to make DD operations
    immune from the kill operations. Member also indicated whether THD is in
    kill immune mode or not.
  */
  dd::DD_kill_immunizer *kill_immunizer;

  /* scramble - random string sent to client on handshake */
  char scramble[SCRAMBLE_LENGTH + 1];

  /// @todo: slave_thread is completely redundant, we should use 'system_thread'
  /// instead /sven
  bool slave_thread;

  uchar password;

 private:
  /**
    Set to true if execution of the current compound statement
    can not continue. In particular, disables activation of
    CONTINUE or EXIT handlers of stored routines.
    Reset in the end of processing of the current user request, in
    @see mysql_reset_thd_for_next_command().
  */
  bool m_is_fatal_error;

 public:
  /**
    Set by a storage engine to request the entire
    transaction (that possibly spans multiple engines) to
    rollback. Reset in ha_rollback.
  */
  bool transaction_rollback_request;
  /**
    true if we are in a sub-statement and the current error can
    not be safely recovered until we left the sub-statement mode.
    In particular, disables activation of CONTINUE and EXIT
    handlers inside sub-statements. E.g. if it is a deadlock
    error and requires a transaction-wide rollback, this flag is
    raised (traditionally, MySQL first has to close all the reads
    via @see handler::ha_index_or_rnd_end() and only then perform
    the rollback).
    Reset to false when we leave the sub-statement mode.
  */
  bool is_fatal_sub_stmt_error;
  bool query_start_usec_used;
  bool rand_used, time_zone_used;
  bool in_lock_tables;
  /**
    True if a slave error. Causes the slave to stop. Not the same
    as the statement execution error (is_error()), since
    a statement may be expected to return an error, e.g. because
    it returned an error on master, and this is OK on the slave.
  */
  bool is_slave_error;

  /**  is set if some thread specific value(s) used in a statement. */
  bool thread_specific_used;
  /**
    is set if a statement accesses a temporary table created through
    CREATE TEMPORARY TABLE.
  */
  bool charset_is_system_charset, charset_is_collation_connection;
  bool charset_is_character_set_filesystem;
  bool enable_slow_log; /* enable slow log for current statement */
  /* set during loop of derived table processing */
  bool derived_tables_processing;
  // Set while parsing INFORMATION_SCHEMA system views.
  bool parsing_system_view;

  /**
     Transiently enabled when we need to error/log for strict mode violations.
     Uses system variables error_partial_strict/audit_instrumented_event
  */
  bool really_error_partial_strict;
  ulong really_audit_instrumented_event;
  bool audited_event_for_command;

  /** Current SP-runtime context. */
  sp_rcontext *sp_runtime_ctx;
  sp_cache *sp_proc_cache;
  sp_cache *sp_func_cache;

  /** number of name_const() substitutions, see sp_head.cc:subst_spvars() */
  bool non_xid_trx = false;
  uint query_name_consts;

  /*
    If we do a purge of binary logs, log index info of the threads
    that are currently reading it needs to be adjusted. To do that
    each thread that is using LOG_INFO needs to adjust the pointer to it
  */
  LOG_INFO *current_linfo;
  /* Used by the sys_var class to store temporary values */
  union {
    bool bool_value;
    long long_value;
    ulong ulong_value;
    ulonglong ulonglong_value;
    double double_value;
  } sys_var_tmp;

  struct {
    /*
      If true, mysql_bin_log::write(Log_event) call will not write events to
      binlog, and maintain 2 below variables instead (use
      mysql_bin_log.start_union_events to turn this on)
    */
    bool do_union;
    /*
      If true, at least one mysql_bin_log::write(Log_event) call has been
      made after last mysql_bin_log.start_union_events() call.
    */
    bool unioned_events;
    /*
      If true, at least one mysql_bin_log::write(Log_event e), where
      e.cache_stmt == true call has been made after last
      mysql_bin_log.start_union_events() call.
    */
    bool unioned_events_trans;

    /*
      'queries' (actually SP statements) that run under inside this binlog
      union have thd->query_id >= first_query_id.
    */
    query_id_t first_query_id;
  } binlog_evt_union;

  /**
    Internal parser state.
    Note that since the parser is not re-entrant, we keep only one parser
    state here. This member is valid only when executing code during parsing.
  */
  Parser_state *m_parser_state;

  Locked_tables_list locked_tables_list;

  partition_info *work_part_info;

  /**
    Array of active audit plugins which have been used by this THD.
    This list is later iterated to invoke release_thd() on those
    plugins.
  */
  Plugin_array audit_class_plugins;
  /**
    Array of bits indicating which audit classes have already been
    added to the list of audit plugins which are currently in use.
  */
  Prealloced_array<unsigned long, 11> audit_class_mask;

#if defined(ENABLED_DEBUG_SYNC)
  /* Debug Sync facility. See debug_sync.cc. */
  struct st_debug_sync_control *debug_sync_control;
#endif /* defined(ENABLED_DEBUG_SYNC) */

  // We don't want to load/unload plugins for unit tests.
  bool m_enable_plugins;

  /**
     Used by some transformations that need Item:transform to make a permanent
     transform. Will be voided by WL#6570.
  */
  bool m_permanent_transform{false};

  /**
     RAII class to push m_permanent_transform in a scope
  */
  class Permanent_transform {
   private:
    bool m_old_value;
    THD *m_thd;

   public:
    Permanent_transform(THD *thd) {
      m_thd = thd;
      m_old_value = thd->m_permanent_transform;
      thd->m_permanent_transform = true;
    }
    ~Permanent_transform() { m_thd->m_permanent_transform = m_old_value; }
  };

  /**
    A flag to mark that SQL statement using Gap Lock was already written.
    This is to prevent to log the same query multiple times.
  */
  bool m_gap_lock_log_written;

  THD(bool enable_plugins = true, bool is_slave = false);

  /*
    The THD dtor is effectively split in two:
      THD::release_resources() and ~THD().

    We want to minimize the time we hold LOCK_thd_list,
    so when destroying a global thread, do:

    thd->release_resources()
    Global_THD_manager::get_instance()->remove_thd();
    delete thd;
   */
  ~THD();

  void release_resources();
  bool release_resources_done() const { return m_release_resources_done; }

 private:
  bool m_release_resources_done;
  bool cleanup_done;
  void cleanup(void);

  void init(bool is_slave);

 public:
  void fix_capability_based_variables();
  /**
    Initialize memory roots necessary for query processing and (!)
    pre-allocate memory for it. We can't do that in THD constructor because
    there are use cases (acl_init, watcher threads,
    killing mysqld) where it's vital to not allocate excessive and not used
    memory. Note, that we still don't return error from init_query_mem_roots()
    if preallocation fails, we should notice that at the first call to
    alloc_root.
  */
  void init_query_mem_roots();
  void cleanup_connection(void);
  void cleanup_after_query();
  void store_globals();
  void restore_globals();

  inline void set_active_vio(Vio *vio) {
    mysql_mutex_lock(&LOCK_thd_data);
    active_vio = vio;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  inline void set_ssl(Vio *vio) {
    mysql_mutex_lock(&LOCK_thd_data);
    m_SSL = (SSL *)vio->ssl_arg;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  inline void clear_active_vio() {
    mysql_mutex_lock(&LOCK_thd_data);
    active_vio = nullptr;
    m_SSL = nullptr;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  /** Set active clone network Vio for remote clone.
  @param[in] vio network vio */
  inline void set_clone_vio(Vio *vio) {
    mysql_mutex_lock(&LOCK_thd_data);
    clone_vio = vio;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  /** Clear clone network Vio for remote clone. */
  inline void clear_clone_vio() {
    mysql_mutex_lock(&LOCK_thd_data);
    clone_vio = nullptr;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  /** Check if clone network Vio is active. */
  inline bool check_clone_vio() {
    mysql_mutex_lock(&LOCK_thd_data);
    bool is_active = (clone_vio != nullptr);
    mysql_mutex_unlock(&LOCK_thd_data);
    return (is_active);
  }

  /** Shutdown clone vio, if active. */
  void shutdown_clone_vio();

  enum_vio_type get_vio_type() const;

  void shutdown_active_vio();
  void awake(THD::killed_state state_to_set, const char *reason = nullptr);

  /** Disconnect the associated communication endpoint. */
  void disconnect(bool server_shutdown = false);

  enum enum_binlog_query_type {
    /* The query can be logged in row format or in statement format. */
    ROW_QUERY_TYPE,

    /* The query has to be logged in statement format. */
    STMT_QUERY_TYPE,

    QUERY_TYPE_COUNT
  };

  int binlog_query(enum_binlog_query_type qtype, const char *query,
                   size_t query_len, bool is_trans, bool direct,
                   bool suppress_use, int errcode);

  // Begin implementation of MDL_context_owner interface.

  void enter_cond(mysql_cond_t *cond, mysql_mutex_t *mutex,
                  const PSI_stage_info *stage, PSI_stage_info *old_stage,
                  const char *src_function, const char *src_file,
                  int src_line) {
    DBUG_TRACE;
    mysql_mutex_assert_owner(mutex);
    /*
      Sic: We don't lock LOCK_current_cond here.
      If we did, we could end up in deadlock with THD::awake()
      which locks current_mutex while LOCK_current_cond is locked.
    */
    current_mutex = mutex;
    current_cond = cond;
    enter_stage(stage, old_stage, src_function, src_file, src_line);
    return;
  }

  void exit_cond(const PSI_stage_info *stage, const char *src_function,
                 const char *src_file, int src_line) {
    DBUG_TRACE;
    /*
      current_mutex must be unlocked _before_ LOCK_current_cond is
      locked (if that would not be the case, you'll get a deadlock if someone
      does a THD::awake() on you).
    */
    mysql_mutex_assert_not_owner(current_mutex.load());
    mysql_mutex_lock(&LOCK_current_cond);
    current_mutex = nullptr;
    current_cond = nullptr;
    mysql_mutex_unlock(&LOCK_current_cond);
    enter_stage(stage, nullptr, src_function, src_file, src_line);
    return;
  }

  virtual int is_killed() const final { return killed; }
  virtual THD *get_thd() { return this; }

  /**
    A callback to the server internals that is used to address
    special cases of the locking protocol.
    Invoked when acquiring an exclusive lock, for each thread that
    has a conflicting shared metadata lock.

    This function aborts waiting of the thread on a data lock, to make
    it notice the pending exclusive lock and back off.

    @note This function does not wait for the thread to give away its
          locks. Waiting is done outside for all threads at once.

    @param ctx_in_use           The MDL context owner (thread) to wake up.
    @param needs_thr_lock_abort Indicates that to wake up thread
                                this call needs to abort its waiting
                                on table-level lock.
   */
  virtual void notify_shared_lock(MDL_context_owner *ctx_in_use,
                                  bool needs_thr_lock_abort);

  /**
      This function is similar to notify_shared_lock that is used to kill
      blocking threads which hold shared MDL locks.
      Invoked when acquiring an exclusive lock, for each thread that
      has a conflicting shared metadata lock.
      @note The current thread must hold an upgradable MDL lock, and must be a
      super user to kill other threads.
      @param ctx_in_use           The MDL context owner (thread) to wake up.
     */
  virtual bool kill_shared_lock(MDL_context_owner *in_use);

  virtual bool notify_hton_pre_acquire_exclusive(const MDL_key *mdl_key,
                                                 bool *victimized);

  virtual void notify_hton_post_release_exclusive(const MDL_key *mdl_key);

  /**
    Provide thread specific random seed for MDL_context's PRNG.

    Note that even if two connections will request seed during handling of
    statements which were started at exactly the same time, and thus will
    get the same values in PRNG at the start, they will naturally diverge
    soon, since calls to PRNG in MDL subsystem are affected by many factors
    making process quite random. OTOH the fact that we use time as a seed
    gives more randomness and thus better coverage in tests as opposed to
    using thread_id for the same purpose.
  */
  virtual uint get_rand_seed() const { return (uint)start_utime; }

  // End implementation of MDL_context_owner interface.

  inline bool install_strict_handler() const {
    return is_strict_sql_mode() || variables.error_partial_strict ||
           variables.audit_instrumented_event > 0;
  }
  inline bool is_strict_sql_mode() const {
    return (variables.sql_mode &
            (MODE_STRICT_TRANS_TABLES | MODE_STRICT_ALL_TABLES));
  }

  inline const CHARSET_INFO *collation() {
    return variables.collation_server ? variables.collation_server
                                      : default_charset_info;
  }
  inline Time_zone *time_zone() {
    time_zone_used = true;
    return variables.time_zone;
  }
  time_t query_start_in_secs() const { return start_time.tv_sec; }
  timeval query_start_timeval_trunc(uint decimals);
  void set_time();
  void set_start_cputime();
  ulonglong get_cpu_time();
  void set_time(const struct timeval *t) {
    user_time = *t;
    set_time();
  }
  void set_time_after_lock();
  inline bool is_fsp_truncate_mode() const {
    return (variables.sql_mode & MODE_TIME_TRUNCATE_FRACTIONAL);
  }

  /**
   Evaluate the current time, and if it exceeds the long-query-time
   setting, mark the query as slow.
  */
  void update_slow_query_status();

  /**
   Check for any session state changes that warrant setting the
   SESSION_STATE_CHANGE_TRACKER
  */
  void finalize_session_trackers() {
    if (((locked_tables_mode != LTM_NONE) ||
         mdl_context.has_locks(MDL_key::USER_LEVEL_LOCK)) &&
        session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
            ->is_enabled()) {
      session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
          ->mark_as_changed(this, NULL);
    }
  }

  ulonglong found_rows() const { return previous_found_rows; }

  /*
    Call when it is clear that the query is ended and we have collected the
    right value for current_found_rows. Calling this method makes a snapshot of
    that value and makes it ready and stable for subsequent FOUND_ROWS() call
    in the next statement.
  */
  inline void update_previous_found_rows() {
    previous_found_rows = current_found_rows;
  }

  /**
    Returns true if session is in a multi-statement transaction mode.

    OPTION_NOT_AUTOCOMMIT: When autocommit is off, a multi-statement
    transaction is implicitly started on the first statement after a
    previous transaction has been ended.

    OPTION_BEGIN: Regardless of the autocommit status, a multi-statement
    transaction can be explicitly started with the statements "START
    TRANSACTION", "BEGIN [WORK]", "[COMMIT | ROLLBACK] AND CHAIN", etc.

    Note: this doesn't tell you whether a transaction is active.
    A session can be in multi-statement transaction mode, and yet
    have no active transaction, e.g., in case of:
    set \@\@autocommit=0;
    set \@a= 3;                                    <-- these statements don't
    set transaction isolation level serializable;  <-- start an active
    flush tables;                                  <-- transaction

    I.e. for the above scenario this function returns true, even
    though no active transaction has begun.
    @sa in_active_multi_stmt_transaction()
  */
  inline bool in_multi_stmt_transaction_mode() const {
    return variables.option_bits & (OPTION_NOT_AUTOCOMMIT | OPTION_BEGIN);
  }
  /**
    true if the session is in a multi-statement transaction mode
    (@sa in_multi_stmt_transaction_mode()) *and* there is an
    active transaction, i.e. there is an explicit start of a
    transaction with BEGIN statement, or implicit with a
    statement that uses a transactional engine.

    For example, these scenarios don't start an active transaction
    (even though the server is in multi-statement transaction mode):

    @verbatim
    set @@autocommit=0;
    select * from nontrans_table;
    set @var = true;
    flush tables;
    @endverbatim

    Note, that even for a statement that starts a multi-statement
    transaction (i.e. select * from trans_table), this
    flag won't be set until we open the statement's tables
    and the engines register themselves for the transaction
    (see trans_register_ha()),
    hence this method is reliable to use only after
    open_tables() has completed.

    Why do we need a flag?
    ----------------------
    We need to maintain a (at first glance redundant)
    session flag, rather than looking at thd->transaction.all.ha_list
    because of explicit start of a transaction with BEGIN.

    I.e. in case of
    BEGIN;
    select * from nontrans_t1; <-- in_active_multi_stmt_transaction() is true
  */
  inline bool in_active_multi_stmt_transaction() const {
    return server_status & SERVER_STATUS_IN_TRANS;
  }
  bool fill_information_schema_tables() const {
    return !stmt_arena->is_stmt_prepare();
  }

  bool convert_string(LEX_STRING *to, const CHARSET_INFO *to_cs,
                      const char *from, size_t from_length,
                      const CHARSET_INFO *from_cs, bool report_error = false);

  int send_explain_fields(Query_result *result);

  /**
    Clear the current error, if any.
    We do not clear is_fatal_error or is_fatal_sub_stmt_error since we
    assume this is never called if the fatal error is set.
    @todo: To silence an error, one should use Internal_error_handler
    mechanism. In future this function will be removed.
  */
  inline void clear_error() {
    DBUG_TRACE;
    if (get_stmt_da()->is_error()) get_stmt_da()->reset_diagnostics_area();
    is_slave_error = false;
    return;
  }

  bool is_classic_protocol() const;

  /** Return false if connection to client is broken. */
  bool is_connected() final;

  /**
    Mark the current error as fatal. Warning: this does not
    set any error, it sets a property of the error, so must be
    followed or prefixed with my_error().
  */
  void fatal_error() { m_is_fatal_error = true; }
  bool is_fatal_error() const { return m_is_fatal_error; }
  /**
    true if there is an error in the error stack.

    Please use this method instead of direct access to
    net.report_error.

    If true, the current (sub)-statement should be aborted.
    The main difference between this member and is_fatal_error
    is that a fatal error can not be handled by a stored
    procedure continue handler, whereas a normal error can.

    To raise this flag, use my_error().
  */
  inline bool is_error() const { return get_stmt_da()->is_error(); }

  /// Returns first Diagnostics Area for the current statement.
  Diagnostics_area *get_stmt_da() { return m_stmt_da; }

  /// Returns first Diagnostics Area for the current statement.
  const Diagnostics_area *get_stmt_da() const { return m_stmt_da; }

  /// Returns the second Diagnostics Area for the current statement.
  const Diagnostics_area *get_stacked_da() const {
    return get_stmt_da()->stacked_da();
  }

  /**
    Returns thread-local Diagnostics Area for parsing.
    We need to have a clean DA in case errors or warnings are thrown
    during parsing, but we can't just reset the main DA in case we
    have a diagnostic statement on our hand that needs the old DA
    to answer questions about the previous execution.
    Keeping a static per-thread DA for parsing is less costly than
    allocating a temporary one for each statement we parse.
  */
  Diagnostics_area *get_parser_da() { return &m_parser_da; }

  /**
    Returns thread-local Diagnostics Area to be used by query rewrite plugins.
    Query rewrite plugins use their own diagnostics area. The reason is that
    they are invoked right before and right after parsing, and we don't want
    conditions raised by plugins in either statement nor parser DA until we
    know which type of statement we have parsed.

    @note The diagnostics area is instantiated the first time it is asked for.
  */
  Diagnostics_area *get_query_rewrite_plugin_da() {
    return m_query_rewrite_plugin_da_ptr;
  }

  /**
    Push the given Diagnostics Area on top of the stack, making
    it the new first Diagnostics Area. Conditions in the new second
    Diagnostics Area will be copied to the new first Diagnostics Area.

    @param da   Diagnostics Area to be come the top of
                the Diagnostics Area stack.
    @param copy_conditions
                Copy the conditions from the new second Diagnostics Area
                to the new first Diagnostics Area, as per SQL standard.
  */
  void push_diagnostics_area(Diagnostics_area *da,
                             bool copy_conditions = true) {
    get_stmt_da()->push_diagnostics_area(this, da, copy_conditions);
    m_stmt_da = da;
  }

  /// Pop the top DA off the Diagnostics Area stack.
  void pop_diagnostics_area() {
    m_stmt_da = get_stmt_da()->pop_diagnostics_area();
  }

  /**
    Inserts the new protocol at the top of the protocol stack, and make it
    the current protocol for this thd.

    @param protocol Protocol to be inserted.
  */
  void push_protocol(Protocol *protocol);

  template <typename ProtocolClass>
  void push_protocol(const std::unique_ptr<ProtocolClass> &protocol) {
    push_protocol(protocol.get());
  }

  /**
    Pops the top protocol of the Protocol stack and sets the previous one
    as the current protocol.
  */
  void pop_protocol();

 public:
  const CHARSET_INFO *charset() const { return variables.character_set_client; }
  void update_charset();

  /**
    Update the place with new_value.
    If THD::m_permanent_transform is false:
      - Use a plain assignment (iff THD::stmt_area->is_regular())
        or assign and register place for rollback.
    If THD::m_permanent_transform is true:
      - we a) remove any rollback requests for this location and b) do the
        assignment without registering any rollback request.

    @param place     The location at which we want set a new value
    @param new_value The new value
  */
  void change_item_tree(Item **place, Item *new_value);

  /**
    Remember that place was updated with new_value so it can be restored
    by rollback_item_tree_changes().

    @param[in] place the location that will change, and whose old value
               we need to remember for restoration
    @param[in] new_value new value about to be inserted into *place, remember
               for associative lookup, see replace_rollback_place()
  */
  void nocheck_register_item_tree_change(Item **place, Item *new_value);

  /**
    Find and update change record of an underlying item based on the new
    value for a place.

    If we have already saved a position to rollback for new_value,
    forget that rollback position and register the new place instead,
    typically because a transformation has made the old place irrelevant.
    If not, a no-op.

    @param new_place  The new location in which we have presumably saved
                      the new value, but which need to be rolled back to
                      the old value.
                      This location must also contain the new value.
    @returns old_value if one was found, else nullptr
  */
  Item *replace_rollback_place(Item **new_place);

  /**
    Place has an alias of a new value which should possibly be rolled back.
    If so, this location should be rolled back as well.
    The value will already be registered with a rollback value in another
    place. Find that rollback value, and register a rollback record for this
    place as well, using the same rollback value.

    @param place the location of the alias
  */
  void alias_rollback(Item **place);

  void update_ident_context(SELECT_LEX *orig_block, SELECT_LEX *new_block);
  void cancel_rollback(Item *new_item);
  void cancel_rollback_at(Item **place);
  /**
    Restore locations set by calls to nocheck_register_item_tree_change().  The
    value to be restored depends on whether replace_rollback_place()
    has been called. If not, we restore the original value. If it has been
    called, we restore the one supplied by the latest call to
    replace_rollback_place()
  */
  void rollback_item_tree_changes();

  /*
    Cleanup statement parse state (parse tree, lex) and execution
    state after execution of a non-prepared SQL statement.
  */
  void end_statement();
  void send_kill_message() const;

  void reset_n_backup_open_tables_state(Open_tables_backup *backup,
                                        uint add_state_flags);
  void restore_backup_open_tables_state(Open_tables_backup *backup);
  void reset_sub_statement_state(Sub_statement_state *backup, uint new_state);
  void restore_sub_statement_state(Sub_statement_state *backup);

 public:
  /**
    Start a read-only attachable transaction.
    There must be no active attachable transactions (in other words, there can
    be only one active attachable transaction at a time).
  */
  void begin_attachable_ro_transaction();

  /**
    Start a read-write attachable transaction.
    All the read-only class' requirements apply.
    Additional requirements are documented along the class
    declaration.
  */
  void begin_attachable_rw_transaction();

  /**
    End an active attachable transaction. Applies to both the read-only
    and the read-write versions.
    Note, that the read-write attachable transaction won't be terminated
    inside this method.
    To invoke the function there must be active attachable transaction.
  */
  void end_attachable_transaction();

  /**
    @return true if there is an active attachable transaction.
  */
  bool is_attachable_ro_transaction_active() const {
    return m_attachable_trx != nullptr && m_attachable_trx->is_read_only();
  }

  /**
    @return true if there is an active attachable transaction.
  */
  bool is_attachable_transaction_active() const {
    return m_attachable_trx != nullptr;
  }

  /**
    @return true if there is an active rw attachable transaction.
  */
  bool is_attachable_rw_transaction_active() const {
    return m_attachable_trx != nullptr && !m_attachable_trx->is_read_only();
  }

 public:
  /*
    @todo Make these methods private or remove them completely.  Only
    decide_logging_format should call them. /Sven
  */
  inline void set_current_stmt_binlog_format_row_if_mixed() {
    DBUG_TRACE;
    /*
      This should only be called from decide_logging_format.

      @todo Once we have ensured this, uncomment the following
      statement, remove the big comment below that, and remove the
      in_sub_stmt==0 condition from the following 'if'.
    */
    /* DBUG_ASSERT(in_sub_stmt == 0); */
    /*
      If in a stored/function trigger, the caller should already have done the
      change. We test in_sub_stmt to prevent introducing bugs where people
      wouldn't ensure that, and would switch to row-based mode in the middle
      of executing a stored function/trigger (which is too late, see also
      reset_current_stmt_binlog_format_row()); this condition will make their
      tests fail and so force them to propagate the
      lex->binlog_row_based_if_mixed upwards to the caller.
    */
    if ((variables.binlog_format == BINLOG_FORMAT_MIXED) && (in_sub_stmt == 0))
      set_current_stmt_binlog_format_row();

    return;
  }
  inline void set_current_stmt_binlog_format_row() {
    DBUG_TRACE;
    current_stmt_binlog_format = BINLOG_FORMAT_ROW;
    return;
  }
  inline void clear_current_stmt_binlog_format_row() {
    DBUG_TRACE;
    current_stmt_binlog_format = BINLOG_FORMAT_STMT;
    return;
  }
  inline void reset_current_stmt_binlog_format_row() {
    DBUG_TRACE;
    DBUG_PRINT("debug", ("in_sub_stmt: %d, system_thread: %s", in_sub_stmt != 0,
                         show_system_thread(system_thread)));
    if (in_sub_stmt == 0) {
      if (variables.binlog_format == BINLOG_FORMAT_ROW)
        set_current_stmt_binlog_format_row();
      else
        clear_current_stmt_binlog_format_row();
    }
    return;
  }

  /**
    Copies variables.original_commit_timestamp to
    ((Slave_worker *)rli_slave)->original_commit_timestamp,
    if this is a slave thread.
  */
  void set_original_commit_timestamp_for_slave_thread();

  /// Return the value of @@gtid_next_list: either a Gtid_set or NULL.
  Gtid_set *get_gtid_next_list() {
    return variables.gtid_next_list.is_non_null
               ? variables.gtid_next_list.gtid_set
               : nullptr;
  }

  /// Return the value of @@gtid_next_list: either a Gtid_set or NULL.
  const Gtid_set *get_gtid_next_list_const() const {
    return const_cast<THD *>(this)->get_gtid_next_list();
  }

  /**
    Return true if the statement/transaction cache is currently empty,
    false otherwise.

    @param is_transactional if true, check the transaction cache.
    If false, check the statement cache.
  */
  bool is_binlog_cache_empty(bool is_transactional) const;

  /**
    The GTID of the currently owned transaction.

    ==== Modes of ownership ====

    The following modes of ownership are possible:

    - owned_gtid.sidno==0: the thread does not own any transaction.

    - owned_gtid.sidno==THD::OWNED_SIDNO_ANONYMOUS(==-2): the thread
      owns an anonymous transaction

    - owned_gtid.sidno>0 and owned_gtid.gno>0: the thread owns a GTID
      transaction.

    - (owned_gtid.sidno==THD::OWNED_SIDNO_GTID_SET(==-1): this is
      currently not used.  It was reserved for the case where multiple
      GTIDs are owned (using gtid_next_list).  This was one idea to
      make GTIDs work with NDB: due to the epoch concept, multiple
      transactions can be combined into one in NDB, and therefore a
      single transaction on a slave can have multiple GTIDs.)

    ==== Life cycle of ownership ====

    Generally, transaction ownership starts when the transaction is
    assigned its GTID and ends when the transaction commits or rolls
    back.  On a master (GTID_NEXT=AUTOMATIC), the GTID is assigned
    just before binlog flush; on a slave (GTID_NEXT=UUID:NUMBER or
    GTID_NEXT=ANONYMOUS) it is assigned before starting the
    transaction.

    A new client always starts with owned_gtid.sidno=0.

    Ownership can be acquired in the following ways:

    A1. If GTID_NEXT = 'AUTOMATIC' and GTID_MODE = OFF/OFF_PERMISSIVE:
        The thread acquires anonymous ownership in
        gtid_state->generate_automatic_gtid called from
        MYSQL_BIN_LOG::write_transaction.

    A2. If GTID_NEXT = 'AUTOMATIC' and GTID_MODE = ON/ON_PERMISSIVE:
        The thread generates the GTID and acquires ownership in
        gtid_state->generate_automatic_gtid called from
        MYSQL_BIN_LOG::write_transaction.

    A3. If GTID_NEXT = 'UUID:NUMBER': The thread acquires ownership in
        the following ways:

        - In a client, the SET GTID_NEXT statement acquires ownership.

        - The slave's analogy to a clients SET GTID_NEXT statement is
          Gtid_log_event::do_apply_event.  So the slave acquires
          ownership in this function.

        Note: if the GTID UUID:NUMBER is already included in
        GTID_EXECUTED, then the transaction must be skipped (the GTID
        auto-skip feature).  Thus, ownership is *not* acquired in this
        case and owned_gtid.sidno==0.

    A4. If GTID_NEXT = 'ANONYMOUS':

        - In a client, the SET GTID_NEXT statement acquires ownership.

        - In a slave thread, Gtid_log_event::do_apply_event acquires
          ownership.

        - Contrary to the case of GTID_NEXT='UUID:NUMBER', it is
          allowed to execute two transactions in sequence without
          changing GTID_NEXT (cf. R1 and R2 below).  Both transactions
          should be executed as anonymous transactions.  But ownership
          is released when the first transaction commits.  Therefore,
          when GTID_NEXT='ANONYMOUS', we also acquire anonymous
          ownership when starting to execute a statement, in
          gtid_reacquire_ownership_if_anonymous called from
          gtid_pre_statement_checks (usually called from
          mysql_execute_command).

    A5. Slave applier threads start in a special mode, having
        GTID_NEXT='NOT_YET_DETERMINED'.  This mode cannot be set in a
        regular client.  When GTID_NEXT=NOT_YET_DETERMINED, the slave
        thread is postponing the decision of the value of GTID_NEXT
        until it has more information.  There are three cases:

        - If the first transaction of the relay log has a
          Gtid_log_event, then it will set GTID_NEXT=GTID:NUMBER and
          acquire GTID ownership in Gtid_log_event::do_apply_event.

        - If the first transaction of the relay log has a
          Anonymous_gtid_log_event, then it will set
          GTID_NEXT=ANONYMOUS and acquire anonymous ownership in
          Gtid_log_event::do_apply_event.

        - If the relay log was received from a pre-5.7.6 master with
          GTID_MODE=OFF (or a pre-5.6 master), then there are neither
          Gtid_log_events nor Anonymous_log_events in the relay log.
          In this case, the slave sets GTID_NEXT=ANONYMOUS and
          acquires anonymous ownership when executing a
          Query_log_event (Query_log_event::do_apply_event calls
          mysql_parse which calls gtid_pre_statement_checks which
          calls gtid_reacquire_ownership_if_anonymous).

    Ownership is released in the following ways:

    R1. A thread that holds GTID ownership releases ownership at
        transaction commit or rollback.  If GTID_NEXT=AUTOMATIC, all
        is fine. If GTID_NEXT=UUID:NUMBER, the UUID:NUMBER cannot be
        used for another transaction, since only one transaction can
        have any given GTID.  To avoid the user mistake of forgetting
        to set back GTID_NEXT, on commit we set
        thd->variables.gtid_next.type=UNDEFINED_GTID.  Then, any
        statement that user tries to execute other than SET GTID_NEXT
        will generate an error.

    R2. A thread that holds anonymous ownership releases ownership at
        transaction commit or rollback.  In this case there is no harm
        in leaving GTID_NEXT='ANONYMOUS', so
        thd->variables.gtid_next.type will remain ANONYMOUS_GTID and
        not UNDEFINED_GTID.

    There are statements that generate multiple transactions in the
    binary log. This includes the following:

    M1. DROP TABLE that is used with multiple tables, and the tables
        belong to more than one of the following groups: non-temporary
        table, temporary transactional table, temporary
        non-transactional table.  DROP TABLE is split into one
        transaction for each of these groups of tables.

    M2. DROP DATABASE that fails e.g. because rmdir fails. Then a
        single DROP TABLE is generated, which lists all tables that
        were dropped before the failure happened. But if the list of
        tables is big, and grows over a limit, the statement will be
        split into multiple statements.

    M3. CREATE TABLE ... SELECT that is logged in row format.  Then
        the server generates a single CREATE statement, followed by a
        BEGIN ... row events ... COMMIT transaction.

    M4. A statement that updates both transactional and
        non-transactional tables in the same statement, and is logged
        in row format.  Then it generates one transaction for the
        non-transactional row updates, followed by one transaction for
        the transactional row updates.

    M5. CALL is executed as multiple transactions and logged as
        multiple transactions.

    The general rules for multi-transaction statements are:

    - If GTID_NEXT=AUTOMATIC and GTID_MODE=ON or ON_PERMISSIVE, one
      GTID should be generated for each transaction within the
      statement. Therefore, ownership must be released after each
      commit so that a new GTID can be generated by the next
      transaction. Typically mysql_bin_log.commit() is called to
      achieve this. (Note that some of these statements are currently
      disallowed when GTID_MODE=ON.)

    - If GTID_NEXT=AUTOMATIC and GTID_MODE=OFF or OFF_PERMISSIVE, one
      Anonymous_gtid_log_event should be generated for each
      transaction within the statement. Similar to the case above, we
      call mysql_bin_log.commit() and release ownership between
      transactions within the statement.

      This works for all the special cases M1-M5 except M4.  When a
      statement writes both non-transactional and transactional
      updates to the binary log, both the transaction cache and the
      statement cache are flushed within the same call to
      flush_thread_caches(THD) from within the binary log group commit
      code.  At that point we cannot use mysql_bin_log.commit().
      Instead we release ownership using direct calls to
      gtid_state->release_anonymous_ownership() and
      thd->clear_owned_gtids() from binlog_cache_mngr::flush.

    - If GTID_NEXT=ANONYMOUS, anonymous ownership must be *preserved*
      between transactions within the statement, to prevent that a
      concurrent SET GTID_MODE=ON makes it impossible to log the
      statement. To avoid that ownership is released if
      mysql_bin_log.commit() is called, we set
      thd->is_commit_in_middle_of_statement before calling
      mysql_bin_log.commit.  Note that we must set this flag only if
      GTID_NEXT=ANONYMOUS, not if the transaction is anonymous when
      GTID_NEXT=AUTOMATIC and GTID_MODE=OFF.

      This works for all the special cases M1-M5 except M4.  When a
      statement writes non-transactional updates in the middle of a
      transaction, but keeps some transactional updates in the
      transaction cache, then it is not easy to know at the time of
      calling mysql_bin_log.commit() whether anonymous ownership needs
      to be preserved or not.  Instead, we directly check if the
      transaction cache is nonempty before releasing anonymous
      ownership inside Gtid_state::update_gtids_impl.

    - If GTID_NEXT='UUID:NUMBER', it is impossible to log a
      multi-transaction statement, since each GTID can only be used by
      one transaction. Therefore, an error must be generated in this
      case.  Errors are generated in different ways for the different
      statement types:

      - DROP TABLE: we can detect the situation before it happens,
        since the table type is known once the tables are opened. So
        we generate an error before even executing the statement.

      - DROP DATABASE: we can't detect the situation until it is too
        late; the tables have already been dropped and we cannot log
        anything meaningful.  So we don't log at all.

      - CREATE TABLE ... SELECT: this is not allowed when
        enforce_gtid_consistency is ON; the statement will be
        forbidden in is_ddl_gtid_compatible.

      - Statements that update both transactional and
        non-transactional tables are disallowed when GTID_MODE=ON, so
        this normally does not happen. However, it can happen if the
        slave uses a different engine type than the master, so that a
        statement that updates InnoDB+InnoDB on master updates
        InnoDB+MyISAM on slave.  In this case the statement will be
        forbidden in is_dml_gtid_compatible and will not be allowed to
        execute.

      - CALL: the second statement will generate an error because
        GTID_NEXT is 'undefined'.  Note that this situation can only
        happen if user does it on purpose: A CALL on master is logged
        as multiple statements, so a slave never executes CALL with
        GTID_NEXT='UUID:NUMBER'.

    Finally, ownership release is suppressed in one more corner case:

    C1. Administration statements including OPTIMIZE TABLE, REPAIR
        TABLE, or ANALYZE TABLE are written to the binary log even if
        they fail.  This means that the thread first calls
        trans_rollack, and then writes the statement to the binlog.
        Rollback normally releases ownership.  But ownership must be
        kept until writing the binlog.  The solution is that these
        statements set thd->skip_gtid_rollback=true before calling
        trans_rollback, and Gtid_state::update_on_rollback does not
        release ownership if the flag is set.

    @todo It would probably be better to encapsulate this more, maybe
    use Gtid_specification instead of Gtid.
  */
  Gtid owned_gtid;
  static const int OWNED_SIDNO_GTID_SET = -1;
  static const int OWNED_SIDNO_ANONYMOUS = -2;

  /**
    For convenience, this contains the SID component of the GTID
    stored in owned_gtid.
  */
  rpl_sid owned_sid;

  /** SE GTID persistence flag types. */
  enum Se_GTID_flag : size_t {
    /** Pin owned GTID */
    SE_GTID_PIN = 0,
    /** Cleanup GTID during unpin. */
    SE_GTID_CLEANUP,
    /** SE would persist GTID for current transaction. */
    SE_GTID_PERSIST,
    /** If RESET log in progress. */
    SE_GTID_RESET_LOG,
    /** Max element holding the biset size. */
    SE_GTID_MAX
  };

  using Se_GTID_flagset = std::bitset<SE_GTID_MAX>;

  /** Flags for SE GTID persistence. */
  Se_GTID_flagset m_se_gtid_flags;

  /** Defer freeing owned GTID and SID till unpinned. */
  void pin_gtid() { m_se_gtid_flags.set(SE_GTID_PIN); }

  /** Unpin and free GTID and SID. */
  void unpin_gtid() {
    m_se_gtid_flags.reset(SE_GTID_PIN);
    /* Do any deferred cleanup */
    if (m_se_gtid_flags[SE_GTID_CLEANUP]) {
      clear_owned_gtids();
      m_se_gtid_flags.reset(SE_GTID_CLEANUP);
    }
  }

  /** @return true, if single phase XA commit operation. */
  bool is_one_phase_commit();

  /** Set when binlog reset operation is started. */
  void set_log_reset() { m_se_gtid_flags.set(SE_GTID_RESET_LOG); }

  /** Cleared after flushing SE logs during binlog reset. */
  void clear_log_reset() { m_se_gtid_flags.reset(SE_GTID_RESET_LOG); }

  /** @return true, if binlog reset operation. */
  bool is_log_reset() const { return (m_se_gtid_flags[SE_GTID_RESET_LOG]); }

  /** Set by SE when it guarantees GTID persistence. */
  void set_gtid_persisted_by_se() { m_se_gtid_flags.set(SE_GTID_PERSIST); }

  /** Reset by SE at transaction end after persisting GTID. */
  void reset_gtid_persisted_by_se() { m_se_gtid_flags.reset(SE_GTID_PERSIST); }

  /** @return true, if SE persists GTID for current transaction. */
  bool se_persists_gtid() const {
    DBUG_EXECUTE_IF("disable_se_persists_gtid", return (false););
    auto trx = get_transaction();
    auto xid_state = trx->xid_state();
    /* XA transactions are always persisted by Innodb. */
    return (!xid_state->has_state(XID_STATE::XA_NOTR) ||
            m_se_gtid_flags[SE_GTID_PERSIST]);
  }

#ifdef HAVE_GTID_NEXT_LIST
  /**
    If this thread owns a set of GTIDs (i.e., GTID_NEXT_LIST != NULL),
    then this member variable contains the subset of those GTIDs that
    are owned by this thread.
  */
  Gtid_set owned_gtid_set;
#endif

 public:
  static std::string extract_peer_certificate_info(const THD *thd,
                                                   bool printable);

 private:
  friend int acl_authenticate(THD *, enum_server_command);

  std::string m_connection_certificate;

  void update_connection_certificate() {
    m_connection_certificate =
        extract_peer_certificate_info(this, false /* pem format */);
  }
  void reset_connection_certificate() {
    m_connection_certificate.clear();
    m_connection_certificate.shrink_to_fit();
  }

 public:
  std::string const &get_connection_certificate() const noexcept {
    return m_connection_certificate;
  }

  /*
   Replication related context.

   @todo: move more parts of replication related fields in THD to inside this
          class.
  */
  Rpl_thd_context rpl_thd_ctx;

  void clear_owned_gtids() {
    /* Defer GTID cleanup if pinned. Used for XA transactions where
    SE(Innodb) needs to read GTID. */
    if (m_se_gtid_flags[SE_GTID_PIN]) {
      m_se_gtid_flags.set(SE_GTID_CLEANUP);
      return;
    }
    if (owned_gtid.sidno == OWNED_SIDNO_GTID_SET) {
#ifdef HAVE_GTID_NEXT_LIST
      owned_gtid_set.clear();
#else
      DBUG_ASSERT(0);
#endif
    }
    owned_gtid.clear();
    owned_sid.clear();
    owned_gtid.dbug_print(nullptr, "set owned_gtid in clear_owned_gtids");
  }

  /** @return true, if owned GTID is empty or waiting for deferred cleanup. */
  bool owned_gtid_is_empty() {
    if (m_se_gtid_flags[SE_GTID_CLEANUP]) {
      return (true);
    }
    return (owned_gtid.is_empty());
  }

  /*
    There are some statements (like OPTIMIZE TABLE, ANALYZE TABLE and
    REPAIR TABLE) that might call trans_rollback_stmt() and also will be
    successfully executed and will have to go to the binary log.
    For these statements, the skip_gtid_rollback flag must be set to avoid
    problems when the statement is executed with a GTID_NEXT set to
    ASSIGNED_GTID (like the SQL thread do when applying events from other
    server). When this flag is set, a call to gtid_rollback() will do nothing.
  */
  bool skip_gtid_rollback;
  /*
    There are some statements (like DROP DATABASE that fails on rmdir
    and gets rewritten to multiple DROP TABLE statements) that may
    call trans_commit_stmt() before it has written all statements to
    the binlog.  When using GTID_NEXT = ANONYMOUS, such statements
    should not release ownership of the anonymous transaction until
    all statements have been written to the binlog.  To prevent that
    update_gtid_impl releases ownership, such statements must set this
    flag.
  */
  bool is_commit_in_middle_of_statement;

  /*
    True while the transaction is executing, if one of
    is_ddl_gtid_consistent or is_dml_gtid_consistent returned false.
  */
  bool has_gtid_consistency_violation;

  const LEX_CSTRING &db() const { return m_db; }

  /**
  Set the db_metadata string for the thread
    If the current database is set for the thread, we get the db_metadata option
    for the database and set it as a thd property. This is called whenever
    set_db() or reset_db() are called.
  */
  void set_db_metadata();

  /**
    Extract and set the shard_id string for the thread.
    The shard information is extracted from db_metadata, so that has to be set
    first.
    This is called within set_db_metadata() after metadata is set.
  */
  void set_shard_id();
  std::string shard_id();

  /**
    Set the current database; use deep copy of C-string.

    @param new_db     the new database name.

    Initialize the current database from a NULL-terminated string with
    length. If we run out of memory, we free the current database and
    return true.  This way the user will notice the error as there will be
    no current database selected (in addition to the error message set by
    malloc). Also, set the db_metadata string for the thd.

    @note This operation just sets {db, db_length} and updated db_metadata
    string for the thd. Switching the current database usually involves other
    actions, like switching other database attributes including security
    context. In the future, this operation will be made private and more
    convenient interface will be provided.

    @return Operation status
      @retval false Success
      @retval true  Out-of-memory error
  */
  bool set_db(const LEX_CSTRING &new_db);

  /**
    Set the current database; use shallow copy of C-string.

    @param new_db     the new database name.
    @param lock_held_skip_metadata  whether LOCK_thd_data is held and skip
    calling set_db_metadata()

    @note This operation just sets {db, db_length} and updated db_metadata
    string for the thd. Switching the current database usually involves other
    actions, like switching other database attributes including security
    context. In the future, this operation will be made private and more
    convenient interface will be provided.
  */
  void reset_db(const LEX_CSTRING &new_db,
                bool lock_held_skip_metadata = false) {
    m_db.str = new_db.str;
    m_db.length = new_db.length;
#ifdef HAVE_PSI_THREAD_INTERFACE
    PSI_THREAD_CALL(set_thread_db)(new_db.str, static_cast<int>(new_db.length));
#endif
    /*
      Due to a recursive lock acquisition on the LOCK_thd_data in the
      DD kill immunizer code, set_db_metadata() should only be called if the
      LOCK_thd_data is not held. Once the immunizer code is removed, then this
      parameter can be removed.

      If the LOCK_thd_data is held when reset_db() is called, it is the
      caller's responsibility to call set_db_metadata()
     */
    if (!lock_held_skip_metadata) set_db_metadata();
  }
  /*
    Copy the current database to the argument. Use the current arena to
    allocate memory for a deep copy: current database may be freed after
    a statement is parsed but before it's executed.
  */
  bool copy_db_to(char const **p_db, size_t *p_db_length) const {
    if (m_db.str == nullptr) {
      my_error(ER_NO_DB_ERROR, MYF(0));
      return true;
    }
    *p_db = strmake(m_db.str, m_db.length);
    *p_db_length = m_db.length;
    return false;
  }

  bool copy_db_to(char **p_db, size_t *p_db_length) const {
    return copy_db_to(const_cast<char const **>(p_db), p_db_length);
  }

  thd_scheduler scheduler;

  /**
    Get resource group context.

    @returns pointer to resource group context.
  */

  resourcegroups::Resource_group_ctx *resource_group_ctx() {
    return &m_resource_group_ctx;
  }

 public:
  /**
    Save the performance schema thread instrumentation
    associated with this user session.
    @param psi Performance schema thread instrumentation
  */
  void set_psi(PSI_thread *psi) { m_psi = psi; }

  /**
    Read the performance schema thread instrumentation
    associated with this user session.
    This method is safe to use from a different thread.
  */
  PSI_thread *get_psi() const { return m_psi; }

 private:
  /**
    Performance schema thread instrumentation for this session.
    This member is maintained using atomic operations,
    do not access it directly.
    @sa set_psi
    @sa get_psi
  */
  std::atomic<PSI_thread *> m_psi;

 public:
  const Internal_error_handler *get_internal_handler() const {
    return m_internal_handler;
  }

  /**
    Add an internal error handler to the thread execution context.
    @param handler the exception handler to add
  */
  void push_internal_handler(Internal_error_handler *handler);

 private:
  /**
    Handle a sql condition.
    @param sql_errno the condition error number
    @param sqlstate the condition sqlstate
    @param level the condition level
    @param msg the condition message text
    @return true if the condition is handled
  */
  bool handle_condition(uint sql_errno, const char *sqlstate,
                        Sql_condition::enum_severity_level *level,
                        const char *msg);

 public:
  /**
    Remove the error handler last pushed.
  */
  Internal_error_handler *pop_internal_handler();

  Opt_trace_context opt_trace;  ///< optimizer trace of current statement
  /**
    Raise an exception condition.
    @param code the MYSQL_ERRNO error code of the error
  */
  void raise_error(uint code);

  /**
    Raise an exception condition, with a formatted message.
    @param code the MYSQL_ERRNO error code of the error
  */
  void raise_error_printf(uint code, ...);

  /**
    Raise a completion condition (warning).
    @param code the MYSQL_ERRNO error code of the warning
  */
  void raise_warning(uint code);

  /**
    Raise a completion condition (warning), with a formatted message.
    @param code the MYSQL_ERRNO error code of the warning
  */
  void raise_warning_printf(uint code, ...);

  /**
    Raise a completion condition (note), with a fixed message.
    @param code the MYSQL_ERRNO error code of the note
  */
  void raise_note(uint code);

  /**
    Raise an completion condition (note), with a formatted message.
    @param code the MYSQL_ERRNO error code of the note
  */
  void raise_note_printf(uint code, ...);

 private:
  /*
    Only the implementation of the SIGNAL and RESIGNAL statements
    is permitted to raise SQL conditions in a generic way,
    or to raise them by bypassing handlers (RESIGNAL).
    To raise a SQL condition, the code should use the public
    raise_error() or raise_warning() methods provided by class THD.
  */
  friend class Sql_cmd_common_signal;
  friend class Sql_cmd_signal;
  friend class Sql_cmd_resignal;
  friend void push_warning(THD *thd,
                           Sql_condition::enum_severity_level severity,
                           uint code, const char *message_text);
  friend void my_message_sql(uint, const char *, myf);

  /**
    Raise a generic SQL condition. Also calls mysql_audit_notify() unless
    the condition is handled by a SQL condition handler.

    @param sql_errno the condition error number
    @param sqlstate the condition SQLSTATE
    @param level the condition level
    @param msg the condition message text
    @param fatal_error should the fatal_error flag be set?
    @return The condition raised, or NULL
  */
  Sql_condition *raise_condition(uint sql_errno, const char *sqlstate,
                                 Sql_condition::enum_severity_level level,
                                 const char *msg, bool fatal_error = false);

 public:
  void set_command(enum enum_server_command command);

  inline enum enum_server_command get_command() const { return m_command; }

  /**
    For safe and protected access to the query string, the following
    rules should be followed:
    1: Only the owner (current_thd) can set the query string.
       This will be protected by LOCK_thd_query.
    2: The owner (current_thd) can read the query string without
       locking LOCK_thd_query.
    3: Other threads must lock LOCK_thd_query before reading
       the query string.

    This means that write-write conflicts are avoided by LOCK_thd_query.
    Read(by owner or other thread)-write(other thread) are disallowed.
    Read(other thread)-write(by owner) conflicts are avoided by LOCK_thd_query.
    Read(by owner)-write(by owner) won't happen as THD=thread.

    We want to keep current_thd out of header files, so the debug assert,
    is moved to the .cc file. In optimized mode, we want this getter to
    be fast, so we inline it.
  */
  void debug_assert_query_locked() const;
  const LEX_CSTRING &query() const {
#ifndef DBUG_OFF
    debug_assert_query_locked();
#endif
    return m_query_string;
  }

  /**
    The current query in normalized form. The format is intended to be
    identical to the digest text of performance_schema, but not limited in
    size. In this case the parse tree is traversed as opposed to a (limited)
    token buffer. The string is allocated by this Statement and will be
    available until the next call to this function or this object is deleted.

    @note We have no protection against out-of-memory errors as this function
    relies on the Item::print() interface which does not propagate errors.

    @return The current query in normalized form.
  */
  const String normalized_query();

  /**
    Set query to be displayed in performance schema (threads table etc.).
  */
  void set_query_for_display(const char *query_arg MY_ATTRIBUTE((unused)),
                             size_t query_length_arg MY_ATTRIBUTE((unused))) {
    // Set in pfs events statements table
    MYSQL_SET_STATEMENT_TEXT(m_statement_psi, query_arg,
                             static_cast<uint>(query_length_arg));
#ifdef HAVE_PSI_THREAD_INTERFACE
    // Set in pfs threads table
    PSI_THREAD_CALL(set_thread_info)
    (query_arg, static_cast<uint>(query_length_arg));
#endif
  }
  void reset_query_for_display(void) { set_query_for_display(nullptr, 0); }

  /**
    Assign a new value to thd->m_query_string.
    Protected with the LOCK_thd_query mutex.
  */
  void set_query(const char *query_arg, size_t query_length_arg) {
    LEX_CSTRING tmp = {query_arg, query_length_arg};
    set_query(tmp);
  }
  void set_query(LEX_CSTRING query_arg);
  void reset_query() { set_query(LEX_CSTRING()); }

  /**
    Set the rewritten query (with passwords obfuscated etc.) on the THD.
    Wraps this in the LOCK_thd_query mutex to protect against race conditions
    with SHOW PROCESSLIST inspecting that string.

    This uses swap() and therefore "steals" the argument from the caller;
    the caller MUST take care not to try to use its own string after calling
    this function! This is an optimization for mysql_rewrite_query() so we
    don't copy its temporary string (which may get very long, up to
    @@max_allowed_packet).

    Using this outside of mysql_rewrite_query() is almost certainly wrong;
    please check with the runtime team!

    @param query_arg  The rewritten query to use for slow/bin/general logging.
                      The value will be released in the caller and MUST NOT
                      be used there after calling this function.
  */
  void swap_rewritten_query(String &query_arg);

  /**
    Get the rewritten query (with passwords obfuscated etc.) from the THD.
    If done from a different thread (from the one that the rewritten_query
    is set on), the caller must hold LOCK_thd_query while calling this!
  */
  const String &rewritten_query() const {
#ifndef DBUG_OFF
    if (current_thd != this) mysql_mutex_assert_owner(&LOCK_thd_query);
#endif
    return m_rewritten_query;
  }

  /**
    Reset thd->m_rewritten_query. Protected with the LOCK_thd_query mutex.
 */
  void reset_rewritten_query() {
    String empty;
    swap_rewritten_query(empty);
  }

  /**
    Assign a new value to thd->query_id.
    Protected with the LOCK_thd_data mutex.
  */
  void set_query_id(query_id_t new_query_id) {
    mysql_mutex_lock(&LOCK_thd_data);
    query_id = new_query_id;
    mysql_mutex_unlock(&LOCK_thd_data);
    MYSQL_SET_STATEMENT_QUERY_ID(m_statement_psi, new_query_id);
  }

  /**
    Assign a new value to open_tables.
    Protected with the LOCK_thd_data mutex.
  */
  void set_open_tables(TABLE *open_tables_arg) {
    mysql_mutex_lock(&LOCK_thd_data);
    open_tables = open_tables_arg;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  /**
    Assign a new value to is_killable
    Protected with the LOCK_thd_data mutex.
  */
  void set_is_killable(bool is_killable_arg) {
    mysql_mutex_lock(&LOCK_thd_data);
    is_killable = is_killable_arg;
    mysql_mutex_unlock(&LOCK_thd_data);
  }

  void enter_locked_tables_mode(enum_locked_tables_mode mode_arg) {
    DBUG_ASSERT(locked_tables_mode == LTM_NONE);

    if (mode_arg == LTM_LOCK_TABLES) {
      /*
        When entering LOCK TABLES mode we should set explicit duration
        for all metadata locks acquired so far in order to avoid releasing
        them till UNLOCK TABLES statement.
        We don't do this when entering prelocked mode since sub-statements
        don't release metadata locks and restoring status-quo after leaving
        prelocking mode gets complicated.
      */
      mdl_context.set_explicit_duration_for_all_locks();
    }

    locked_tables_mode = mode_arg;
  }
  void leave_locked_tables_mode();
  int decide_logging_format(TABLE_LIST *tables);
  /**
    is_dml_gtid_compatible() and is_ddl_gtid_compatible() check if the
    statement that is about to be processed will safely get a
    GTID. Currently, the following cases may lead to errors
    (e.g. duplicated GTIDs) and as such are forbidden:

     1. DML statements that mix non-transactional updates with
        transactional updates.

     2. Transactions that use non-transactional tables after
        having used transactional tables.

     3. CREATE...SELECT statement;

     4. CREATE TEMPORARY TABLE or DROP TEMPORARY TABLE within a
        transaction

    The first two conditions have to be checked in
    decide_logging_format, because that's where we know if the table
    is transactional or not.  These are implemented in
    is_dml_gtid_compatible().

    The third and fourth conditions have to be checked in
    mysql_execute_command because (1) that prevents implicit commit
    from being executed if the statement fails; (2) DROP TEMPORARY
    TABLE does not invoke decide_logging_format.  These are
    implemented in is_ddl_gtid_compatible().

    In the cases where GTID violations generate errors,
    is_ddl_gtid_compatible() needs to be called before the implicit
    pre-commit, so that there is no implicit commit if the statement
    fails.

    In the cases where GTID violations do not generate errors,
    is_ddl_gtid_compatible() needs to be called after the implicit
    pre-commit, because in these cases the function will increase the
    global counter automatic_gtid_violating_transaction_count or
    anonymous_gtid_violating_transaction_count.  If there is an
    ongoing transaction, the implicit commit will commit the
    transaction, which will call update_gtids_impl, which should
    decrease the counters depending on whether the *old* was violating
    GTID-consistency or not.  Thus, we should increase the counters
    only after the old transaction is committed.

    @param some_transactional_table true if the statement updates some
    transactional table; false otherwise.

    @param some_non_transactional_table true if the statement updates
    some non-transactional table; false otherwise.

    @param non_transactional_tables_are_tmp true if all updated
    non-transactional tables are temporary.

    @retval true if the statement is compatible;
    @retval false if the statement is not compatible.
  */
  bool is_dml_gtid_compatible(bool some_transactional_table,
                              bool some_non_transactional_table,
                              bool non_transactional_tables_are_tmp);
  bool is_ddl_gtid_compatible();
  void binlog_invoker() { m_binlog_invoker = true; }
  bool need_binlog_invoker() const { return m_binlog_invoker; }
  void get_definer(LEX_USER *definer);
  void set_invoker(const LEX_STRING *user, const LEX_STRING *host) {
    m_invoker_user.str = user->str;
    m_invoker_user.length = user->length;
    m_invoker_host.str = host->str;
    m_invoker_host.length = host->length;
  }
  LEX_CSTRING get_invoker_user() const { return m_invoker_user; }
  LEX_CSTRING get_invoker_host() const { return m_invoker_host; }
  bool has_invoker() const { return m_invoker_user.str != nullptr; }

  void mark_transaction_to_rollback(bool all);

  // Maps column name to col type info (type and length).
  using Column_type_info =
      std::map<std::string, std::pair<enum_field_types, uint>>;
  // Maps (schema, table) to column type information
  std::map<std::pair<std::string, std::string>, Column_type_info>
      schema_info_attrs;
  int validate_schema_info(TABLE_LIST *tl);

  void set_connection_attrs(const char *attrs, size_t length);
  void set_query_attrs(const char *attrs, size_t length);
  int parse_query_info_attr();
  void reset_query_attrs() {
    mysql_mutex_lock(&LOCK_thd_data);
    query_attrs_list.clear();
    mysql_mutex_unlock(&LOCK_thd_data);

    query_attrs_string.clear();
    trace_id.clear();
    num_queries = 0;
    query_type.clear();
    schema_info_attrs.clear();
  }
  inline const char *query_attrs() const { return query_attrs_string.c_str(); }
  inline uint32 query_attrs_length() const {
    return query_attrs_string.length();
  }
  // serialize client attributes and compute CLIENT_ID
  void serialize_client_attrs(const char *query, size_t query_length);
  std::vector<std::pair<std::string, std::string>> query_attrs_list;
  std::unordered_map<std::string, std::string> connection_attrs_map;
  uint64_t num_queries;
  std::string query_type;
  std::string trace_id;
  uint64_t pc_val;
  std::shared_ptr<utils::PerfCounter> query_perf;

 private:
  /** The current internal error handler for this thread, or NULL. */
  Internal_error_handler *m_internal_handler;

  /**
    This memory root is used for two purposes:
    - for conventional queries, to allocate structures stored in main_lex
    during parsing, and allocate runtime data (execution plan, etc.)
    during execution.
    - for prepared queries, only to allocate runtime data. The parsed
    tree itself is reused between executions and thus is stored elsewhere.
  */
  MEM_ROOT main_mem_root;
  Diagnostics_area main_da;
  Diagnostics_area m_parser_da; /**< cf. get_parser_da() */
  Diagnostics_area m_query_rewrite_plugin_da;
  Diagnostics_area *m_query_rewrite_plugin_da_ptr;

  Diagnostics_area *m_stmt_da;

  /**
    It will be set TURE if CURRENT_USER() is called in account management
    statements or default definer is set in CREATE/ALTER SP, SF, Event,
    TRIGGER or VIEW statements.

    Current user will be binlogged into Query_log_event if current_user_used
    is true; It will be stored into m_invoker_host and m_invoker_user by SQL
    thread.
   */
  bool m_binlog_invoker;

  /**
    It points to the invoker in the Query_log_event.
    SQL thread use it as the default definer in CREATE/ALTER SP, SF, Event,
    TRIGGER or VIEW statements or current user in account management
    statements if it is not NULL.
   */
  LEX_CSTRING m_invoker_user;
  LEX_CSTRING m_invoker_host;

  friend class Protocol_classic;

 private:
  std::string query_attrs_string;
  /**
    Optimizer cost model for server operations.
  */
  Cost_model_server m_cost_model;

 public:
  /* connection timeout error message */
  char *conn_timeout_err_msg;

  /* protected by LOCK_thd_query */
  std::string row_query;

  /**
    Initialize the optimizer cost model.

    This function should be called each time a new query is started.
  */
  void init_cost_model() { m_cost_model.init(); }

  /**
    Retrieve the optimizer cost model for this connection.
  */
  const Cost_model_server *cost_model() const { return &m_cost_model; }

  Session_tracker session_tracker;
  Session_sysvar_resource_manager session_sysvar_res_mgr;

  void syntax_error() { syntax_error(ER_SYNTAX_ERROR); }
  void syntax_error(const char *format, ...)
      MY_ATTRIBUTE((format(printf, 2, 3)));
  void syntax_error(int mysql_errno, ...);

  void syntax_error_at(const YYLTYPE &location) {
    syntax_error_at(location, ER_SYNTAX_ERROR);
  }
  void syntax_error_at(const YYLTYPE &location, const char *format, ...)
      MY_ATTRIBUTE((format(printf, 3, 4)));
  void syntax_error_at(const YYLTYPE &location, int mysql_errno, ...);

  void vsyntax_error_at(const YYLTYPE &location, const char *format,
                        va_list args) MY_ATTRIBUTE((format(printf, 3, 0)));
  void vsyntax_error_at(const char *pos_in_lexer_raw_buffer, const char *format,
                        va_list args) MY_ATTRIBUTE((format(printf, 3, 0)));

  /**
    Send name and type of result to client.

    Sum fields has table name empty and field_name.

    @param list         List of items to send to client
    @param flags        Bit mask with the following functions:
                          - 1 send number of rows
                          - 2 send default values
                          - 4 don't write eof packet

    @retval
      false ok
    @retval
      true Error  (Note that in this case the error is not sent to the client)
  */

  bool send_result_metadata(List<Item> *list, uint flags);

  /**
    Send one result set row.

    @param row_items a collection of column values for that row

    @return Error status.
    @retval true  Error.
    @retval false Success.
  */

  bool send_result_set_row(List<Item> *row_items);

  /*
    Send the status of the current statement execution over network.

    In MySQL, there are two types of SQL statements: those that return
    a result set and those that return status information only.

    If a statement returns a result set, it consists of 3 parts:
    - result set meta-data
    - variable number of result set rows (can be 0)
    - followed and terminated by EOF or ERROR packet

    Once the  client has seen the meta-data information, it always
    expects an EOF or ERROR to terminate the result set. If ERROR is
    received, the result set rows are normally discarded (this is up
    to the client implementation, libmysql at least does discard them).
    EOF, on the contrary, means "successfully evaluated the entire
    result set". Since we don't know how many rows belong to a result
    set until it's evaluated, EOF/ERROR is the indicator of the end
    of the row stream. Note, that we can not buffer result set rows
    on the server -- there may be an arbitrary number of rows. But
    we do buffer the last packet (EOF/ERROR) in the Diagnostics_area and
    delay sending it till the very end of execution (here), to be able to
    change EOF to an ERROR if commit failed or some other error occurred
    during the last cleanup steps taken after execution.

    A statement that does not return a result set doesn't send result
    set meta-data either. Instead it returns one of:
    - OK packet
    - ERROR packet.
    Similarly to the EOF/ERROR of the previous statement type, OK/ERROR
    packet is "buffered" in the Diagnostics Area and sent to the client
    in the end of statement.

    @note This method defines a template, but delegates actual
    sending of data to virtual Protocol::send_{ok,eof,error}. This
    allows for implementation of protocols that "intercept" ok/eof/error
    messages, and store them in memory, etc, instead of sending to
    the client.

    @pre  The Diagnostics Area is assigned or disabled. It can not be empty
          -- we assume that every SQL statement or COM_* command
          generates OK, ERROR, or EOF status.

    @post The status information is encoded to protocol format and sent to the
          client.

    @return We conventionally return void, since the only type of error
            that can happen here is a NET (transport) error, and that one
            will become visible when we attempt to read from the NET the
            next command.
            Diagnostics_area::is_sent is set for debugging purposes only.
  */

  void send_statement_status();

  /**
    This is only used by master dump threads.
    When the master receives a new connection from a slave with a
    UUID (for slave versions >= 5.6)/server_id(for slave versions < 5.6)
    that is already connected, it will set this flag true
    before killing the old slave connection.
  */
  bool duplicate_slave_id;

  /**
    Claim all the memory used by the THD object.
    This method is to keep memory instrumentation statistics
    updated, when an object is transfered across threads.
  */
  void claim_memory_ownership();

  bool is_a_srv_session() const { return is_a_srv_session_thd; }
  void mark_as_srv_session() { is_a_srv_session_thd = true; }

  /**
    Returns the plugin, the thd belongs to.
    @return pointer to the plugin id
   */
  const st_plugin_int *get_plugin() const { return m_plugin; }
  /**
    Sets the plugin id to the value provided as parameter
    @param plugin the id of the plugin the thd belongs to
   */
  void set_plugin(const st_plugin_int *plugin) { m_plugin = plugin; }
#ifndef DBUG_OFF
  uint get_tmp_table_seq_id() { return tmp_table_seq_id++; }
  void set_tmp_table_seq_id(uint arg) { tmp_table_seq_id = arg; }
#endif

  bool is_plugin_fake_ddl() const { return m_is_plugin_fake_ddl; }
  void mark_plugin_fake_ddl(bool flag) { m_is_plugin_fake_ddl = flag; }

 private:
  /**
    Variable to mark if the object is part of a Srv_session object, which
    aggregates THD.
  */
  bool is_a_srv_session_thd;

  /// Stores the plugin id it is attached to (if any).
  const st_plugin_int *m_plugin{nullptr};

  /**
    Creating or dropping plugin native table through a plugin service.
    This variable enables the DDL command execution from
    dd::create_native_table() to be executed without committing the
    transaction.
  */
  bool m_is_plugin_fake_ddl;

#ifndef DBUG_OFF
  /**
    Sequential number of internal tmp table created in the statement. Useful for
    tracking tmp tables when number of them is involved in a query.
  */
  uint tmp_table_seq_id;

 public:
  /*
    The member serves to guard against duplicate use of the same xid
    at binary logging.
  */
  XID debug_binlog_xid_last;
#endif
 private:
  /*
    Flag set by my_write before waiting for disk space.

    This is used by replication to decide if the I/O thread should be
    killed or not when stopping the replication threads.

    In ordinary STOP SLAVE case, the I/O thread will wait for disk space
    or to be killed regardless of this flag value.

    In server shutdown case, if this flag is true, the I/O thread will be
    signaled with KILL_CONNECTION to abort the waiting, letting the server
    to shutdown promptly.
  */
  bool waiting_for_disk_space = false;

 public:
  /**
    Set the waiting_for_disk_space flag.

    @param waiting The value to set in the flag.
  */
  void set_waiting_for_disk_space(bool waiting) {
    waiting_for_disk_space = waiting;
  }
  /**
    Returns the current waiting_for_disk_space flag value.
  */
  bool is_waiting_for_disk_space() const { return waiting_for_disk_space; }

  bool sql_parser();

  /// Enables or disables use of secondary storage engines in this session.
  void set_secondary_engine_optimization(Secondary_engine_optimization state) {
    m_secondary_engine_optimization = state;
  }

  /**
    Can secondary storage engines be used for query execution in
    this session?
  */
  Secondary_engine_optimization secondary_engine_optimization() const {
    return m_secondary_engine_optimization;
  }

  /**
    Checks if queries in this session can use a secondary storage engine for
    execution. A secondary engine cannot be used if any of the following
    conditions is true:

    - Secondary engines are disabled in the session
    - The user has disabled secondary engines
    - LOCK TABLES mode is active
    - Multi-statement transaction mode is active
    - It is a sub-statement of a stored procedure

    @return true if secondary storage engines can be used in this
    session, or false otherwise
  */
  bool secondary_storage_engine_eligible() const;

  /* whether the session is already in admission control for queries */
  bool is_in_ac = false;
  st_ac_node_ptr ac_node;
  enum enum_admission_control_request_mode readmission_mode = AC_REQUEST_NONE;

  /* Level of nested thd_wait_begin/thd_wait_end calls. */
  int readmission_nest_level = 0;

  ulonglong last_yield_counter = 0;
  ulonglong yield_counter = 0;
  ulonglong readmission_count = 0;
  std::function<bool()> yield_cond;

  /**
    Check if we should exit and reenter admission control.
  */
  void check_yield(std::function<bool()> cond);

  /**
    Callback for thd_wait_begin.

    @param wait_type Wait type.
  */
  void wait_begin(int wait_type);

  /**
    Callback for thd_wait_end.
  */
  void wait_end();

  /**
    Concurrency control for query.

    @return 0 if the query is admitted, 1 otherwise
   */
  int admit_query();

  /**
    Check if wait type should release AC slot.

    @return true if should release, false otherwise.
  */
  bool filter_wait_type(int wait_type,
                        enum_admission_control_request_mode &new_mode);

 private:
  /**
    This flag tells if a secondary storage engine can be used to
    execute a query in this session.
  */
  Secondary_engine_optimization m_secondary_engine_optimization =
      Secondary_engine_optimization::PRIMARY_ONLY;

  void cleanup_after_parse_error();
  /**
    Flag that indicates if the user of current session has SYSTEM_USER privilege
  */
  std::atomic<bool> m_is_system_user;

 public:
  bool is_system_user();
  void set_system_user(bool system_user_flag);

  my_thread_os_id_t get_thread_os_id() const;

  int get_thread_priority() const;

  bool set_thread_priority(int pri);

  bool set_thread_priority() {
    return set_thread_priority(variables.thread_priority);
  }

  bool set_dscp_on_socket();
};

/**
  Mark a scope as thd_wait_begin/thd_wait_end.
*/
class Thd_wait_scope {
  THD *m_thd;

 public:
  Thd_wait_scope(THD *thd, int wait_type);
  ~Thd_wait_scope();
};

/**
   Return lock_tables_mode for secondary engine.
   @param cthd thread context
   @retval true if lock_tables_mode is on
   @retval false, otherwise
 */
inline bool secondary_engine_lock_tables_mode(const THD &cthd) {
  return (cthd.locked_tables_mode == LTM_LOCK_TABLES ||
          cthd.locked_tables_mode == LTM_PRELOCKED_UNDER_LOCK_TABLES);
}

/** A short cut for thd->get_stmt_da()->set_ok_status(). */
void my_ok(THD *thd, ulonglong affected_rows = 0, ulonglong id = 0,
           const char *message = nullptr);

/** A short cut for thd->get_stmt_da()->set_eof_status(). */
void my_eof(THD *thd);

bool add_item_to_list(THD *thd, Item *item);

/*************************************************************************/

/**
  The function re-attaches the engine ha_data (which was previously detached by
  detach_ha_data_from_thd) to THD.
  This is typically done to replication applier executing
  one of XA-PREPARE, XA-COMMIT ONE PHASE or rollback.

  @param thd         thread context
  @param hton        pointer to handlerton
*/

void reattach_engine_ha_data_to_thd(THD *thd, const struct handlerton *hton);

/**
  Check if engine substitution is allowed in the current thread context.

  @param thd         thread context
  @retval            true if engine substitution is allowed
  @retval            false otherwise
*/

inline bool is_engine_substitution_allowed(const THD *thd) {
  return !(thd->variables.sql_mode & MODE_NO_ENGINE_SUBSTITUTION);
}

/**
  Returns if the user of the session has the SYSTEM_USER privilege or not.

  @retval true  User has SYSTEM_USER privilege
  @retval false Otherwise
*/
inline bool THD::is_system_user() {
  return m_is_system_user.load(std::memory_order_seq_cst);
}

/**
  Sets the system_user flag atomically for the current session.

  @param [in] system_user_flag  boolean flag that indicates value to set.
*/
inline void THD::set_system_user(bool system_user_flag) {
  m_is_system_user.store(system_user_flag, std::memory_order_seq_cst);
}

#endif /* SQL_CLASS_INCLUDED */
