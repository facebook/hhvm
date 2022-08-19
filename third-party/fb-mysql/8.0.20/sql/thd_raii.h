/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef THD_RAII_INCLUDED
#define THD_RAII_INCLUDED 1

/**
 * @file thd_raii.h
 * Some RAII classes that set THD state.
 */

#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/query_options.h"
#include "sql/rpl_slave_commit_order_manager.h"  // has_commit_order_manager
#include "sql/sql_alter.h"
#include "sql/sql_class.h"
#include "sql/system_variables.h"
#include "sql/transaction_info.h"

struct MEM_ROOT;

/*************************************************************************/

/** RAII class for temporarily turning off @@autocommit in the connection. */

class Disable_autocommit_guard {
 public:
  /**
    @param thd  non-NULL - pointer to the context of connection in which
                           @@autocommit mode needs to be disabled.
                NULL     - if @@autocommit mode needs to be left as is.
  */
  explicit Disable_autocommit_guard(THD *thd)
      : m_thd(thd), m_save_option_bits(thd ? thd->variables.option_bits : 0) {
    if (m_thd) {
      /*
        We can't disable auto-commit if there is ongoing transaction as this
        might easily break statement/session transaction invariants.
      */
      DBUG_ASSERT(m_thd->get_transaction()->is_empty(Transaction_ctx::STMT) &&
                  m_thd->get_transaction()->is_empty(Transaction_ctx::SESSION));

      m_thd->variables.option_bits &= ~OPTION_AUTOCOMMIT;
      m_thd->variables.option_bits |= OPTION_NOT_AUTOCOMMIT;
    }
  }

  ~Disable_autocommit_guard() {
    if (m_thd) {
      /*
        Both session and statement transactions need to be finished by the
        time when we enable auto-commit mode back.
      */
      DBUG_ASSERT(m_thd->get_transaction()->is_empty(Transaction_ctx::STMT) &&
                  m_thd->get_transaction()->is_empty(Transaction_ctx::SESSION));
      m_thd->variables.option_bits = m_save_option_bits;
    }
  }

 private:
  THD *m_thd;
  ulonglong m_save_option_bits;
};

/**
  The mode for Implicit_substatement_state_guard based on which it determine
  whether to enable or disable updating Gtid_state and invocation of commit
  order.
*/
enum class enum_implicit_substatement_guard_mode {
  DISABLE_GTID_AND_SPCO,
  DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE,
  ENABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE
};

/**
  RAII class which allows to temporary disable updating Gtid_state and
  disable invocation of commit order for intermediate commits.
*/
class Implicit_substatement_state_guard {
 public:
  /**
    Constructs a new object and set thd->is_operating_substatement_implicitly
    and thd->skip_gtid_rollback according to
    enum_implicit_substatement_guard_mode mode argument.

    @param thd         Thread context.
    @param mode        If mode is not ENABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE then
                       temporary disable updating Gtid_state and invocation of
                       commit order (Commit_order_manager::wait).
  */
  explicit Implicit_substatement_state_guard(
      THD *thd,
      enum_implicit_substatement_guard_mode mode =
          enum_implicit_substatement_guard_mode::DISABLE_GTID_AND_SPCO)
      : m_thd(thd),
        m_save_is_operating_substatement_implicitly(
            thd->is_operating_substatement_implicitly),
        m_save_skip_gtid_rollback(thd->skip_gtid_rollback),
        m_guard_ignored(false) {
    /*
      For modes depending on SPCO being active,
      ignore and return if SPCO is not active.
    */
    if (mode != enum_implicit_substatement_guard_mode::DISABLE_GTID_AND_SPCO &&
        !has_commit_order_manager(thd)) {
      m_guard_ignored = true;
      return;
    }

    bool disable_gtid_and_spco =
        (mode != enum_implicit_substatement_guard_mode ::
                     ENABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE);
    m_thd->is_operating_substatement_implicitly = disable_gtid_and_spco;
    m_thd->skip_gtid_rollback = disable_gtid_and_spco;
  }

  ~Implicit_substatement_state_guard() {
    DBUG_TRACE;
    if (m_guard_ignored) return;
    m_thd->is_operating_substatement_implicitly =
        m_save_is_operating_substatement_implicitly;
    m_thd->skip_gtid_rollback = m_save_skip_gtid_rollback;
  }

 private:
  THD *m_thd;
  bool m_save_is_operating_substatement_implicitly;
  bool m_save_skip_gtid_rollback;
  bool m_guard_ignored;
};

/**
  RAII class to temporarily disable binlogging.
*/

class Disable_binlog_guard {
 public:
  explicit Disable_binlog_guard(THD *thd)
      : m_thd(thd),
        m_binlog_disabled(thd->variables.option_bits & OPTION_BIN_LOG) {
    thd->variables.option_bits &= ~OPTION_BIN_LOG;
  }

  ~Disable_binlog_guard() {
    if (m_binlog_disabled) m_thd->variables.option_bits |= OPTION_BIN_LOG;
  }

 private:
  THD *const m_thd;
  const bool m_binlog_disabled;
};

class Disable_sql_log_bin_guard {
 public:
  explicit Disable_sql_log_bin_guard(THD *thd)
      : m_thd(thd), m_saved_sql_log_bin(thd->variables.sql_log_bin) {
    thd->variables.sql_log_bin = false;
  }

  ~Disable_sql_log_bin_guard() {
    m_thd->variables.sql_log_bin = m_saved_sql_log_bin;
  }

 private:
  THD *const m_thd;
  const bool m_saved_sql_log_bin;
};

/**
  RAII class which allows to save, clear and store binlog format state
  There are two variables in THD class that will decide the binlog
  format of a statement
    i) THD::current_stmt_binlog_format
   ii) THD::variables.binlog_format
  Saving or Clearing or Storing of binlog format state should be done
  for these two variables together all the time.
*/
class Save_and_Restore_binlog_format_state {
 public:
  explicit Save_and_Restore_binlog_format_state(THD *thd)
      : m_thd(thd),
        m_global_binlog_format(thd->variables.binlog_format),
        m_current_stmt_binlog_format(BINLOG_FORMAT_STMT) {
    if (thd->is_current_stmt_binlog_format_row())
      m_current_stmt_binlog_format = BINLOG_FORMAT_ROW;

    thd->variables.binlog_format = BINLOG_FORMAT_STMT;
    thd->clear_current_stmt_binlog_format_row();
  }

  ~Save_and_Restore_binlog_format_state() {
    DBUG_ASSERT(!m_thd->is_current_stmt_binlog_format_row());
    m_thd->variables.binlog_format = m_global_binlog_format;
    if (m_current_stmt_binlog_format == BINLOG_FORMAT_ROW)
      m_thd->set_current_stmt_binlog_format_row();
  }

 private:
  THD *m_thd;
  ulong m_global_binlog_format;
  enum_binlog_format m_current_stmt_binlog_format;
};

/**
  RAII class to temporarily turn off SQL modes that affect parsing
  of expressions. Can also be used when printing expressions even
  if it turns off more SQL modes than strictly necessary for it
  (these extra modes are harmless as they do not affect expression
  printing).
*/
class Sql_mode_parse_guard {
 public:
  explicit Sql_mode_parse_guard(THD *thd)
      : m_thd(thd), m_old_sql_mode(thd->variables.sql_mode) {
    /*
      Switch off modes which can prevent normal parsing of expressions:

      - MODE_REAL_AS_FLOAT            affect only CREATE TABLE parsing
      + MODE_PIPES_AS_CONCAT          affect expression parsing
      + MODE_ANSI_QUOTES              affect expression parsing
      + MODE_IGNORE_SPACE             affect expression parsing
      - MODE_NOT_USED                 not used :)
      * MODE_ONLY_FULL_GROUP_BY       affect execution
      * MODE_NO_UNSIGNED_SUBTRACTION  affect execution
      - MODE_NO_DIR_IN_CREATE         affect table creation only
      - MODE_POSTGRESQL               compounded from other modes
      - MODE_ORACLE                   compounded from other modes
      - MODE_MSSQL                    compounded from other modes
      - MODE_DB2                      compounded from other modes
      - MODE_MAXDB                    affect only CREATE TABLE parsing
      - MODE_NO_KEY_OPTIONS           affect only SHOW
      - MODE_NO_TABLE_OPTIONS         affect only SHOW
      - MODE_NO_FIELD_OPTIONS         affect only SHOW
      - MODE_MYSQL323                 affect only SHOW
      - MODE_MYSQL40                  affect only SHOW
      - MODE_ANSI                     compounded from other modes
                                      (+ transaction mode)
      ? MODE_NO_AUTO_VALUE_ON_ZERO    affect UPDATEs
      + MODE_NO_BACKSLASH_ESCAPES     affect expression parsing
    */
    thd->variables.sql_mode &= ~(MODE_PIPES_AS_CONCAT | MODE_ANSI_QUOTES |
                                 MODE_IGNORE_SPACE | MODE_NO_BACKSLASH_ESCAPES);
  }

  ~Sql_mode_parse_guard() { m_thd->variables.sql_mode = m_old_sql_mode; }

 private:
  THD *m_thd;
  const sql_mode_t m_old_sql_mode;
};

/**
  RAII class to temporarily swap thd->mem_root to a different mem_root.
*/
class Swap_mem_root_guard {
 public:
  Swap_mem_root_guard(THD *thd, MEM_ROOT *mem_root)
      : m_thd(thd), m_old_mem_root(thd->mem_root) {
    thd->mem_root = mem_root;
  }

  ~Swap_mem_root_guard() { m_thd->mem_root = m_old_mem_root; }

  MEM_ROOT *old_mem_root() { return m_old_mem_root; }

 private:
  THD *m_thd;
  MEM_ROOT *m_old_mem_root;
};

/**
  A simple holder for Internal_error_handler.
  The class utilizes RAII technique to not forget to pop the handler.

  @tparam Error_handler      Internal_error_handler to instantiate.
  @tparam Error_handler_arg  Type of the error handler ctor argument.
*/
template <typename Error_handler, typename Error_handler_arg>
class Internal_error_handler_holder {
  THD *m_thd;
  bool m_activate;
  Error_handler m_error_handler;

 public:
  Internal_error_handler_holder(THD *thd, bool activate, Error_handler_arg *arg)
      : m_thd(thd), m_activate(activate), m_error_handler(arg) {
    if (activate) thd->push_internal_handler(&m_error_handler);
  }

  ~Internal_error_handler_holder() {
    if (m_activate) m_thd->pop_internal_handler();
  }
};

/**
  A simple holder for the Prepared Statement Query_arena instance in THD.
  The class utilizes RAII technique to not forget to restore the THD arena.
*/
class Prepared_stmt_arena_holder {
 public:
  /**
    Constructs a new object, activates the persistent arena if requested and if
    a prepared statement or a stored procedure statement is being executed.

    @param thd                    Thread context.
    @param activate_now_if_needed Attempt to activate the persistent arena in
                                  the constructor or not.
  */
  explicit Prepared_stmt_arena_holder(THD *thd,
                                      bool activate_now_if_needed = true)
      : m_thd(thd), m_arena(nullptr) {
    if (activate_now_if_needed && !m_thd->stmt_arena->is_regular() &&
        m_thd->mem_root != m_thd->stmt_arena->mem_root) {
      m_thd->swap_query_arena(*m_thd->stmt_arena, &m_backup);
      m_arena = m_thd->stmt_arena;
    }
  }

  /**
    Deactivate the persistent arena (restore the previous arena) if it has
    been activated.
  */
  ~Prepared_stmt_arena_holder() {
    if (is_activated()) m_thd->swap_query_arena(m_backup, m_arena);
  }

  bool is_activated() const { return m_arena != nullptr; }

 private:
  /// The thread context to work with.
  THD *const m_thd;

  /// The arena set by this holder (by activate()).
  Query_arena *m_arena;

  /// The arena state to be restored.
  Query_arena m_backup;
};

/**
  RAII class for column privilege checking
*/
class Column_privilege_tracker {
 public:
  Column_privilege_tracker(THD *thd, ulong privilege)
      : m_thd(thd), m_saved_privilege(thd->want_privilege) {
    thd->want_privilege = privilege;
  }
  ~Column_privilege_tracker() { m_thd->want_privilege = m_saved_privilege; }

 private:
  THD *const m_thd;
  const ulong m_saved_privilege;
};

#endif  // THD_RAII_INCLUDED
