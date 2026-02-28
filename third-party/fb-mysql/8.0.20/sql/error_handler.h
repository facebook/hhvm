/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef ERROR_HANDLER_INCLUDED
#define ERROR_HANDLER_INCLUDED

#include <string>

#include <stddef.h>
#include <sys/types.h>

#include "mysqld_error.h"   // ER_*
#include "sql/sql_error.h"  // Sql_condition

class Create_field;
class Field;
class String;
class THD;
struct TABLE_LIST;
class handler;

/**
  This class represents the interface for internal error handlers.
  Internal error handlers are exception handlers used by the server
  implementation.
*/
class Internal_error_handler {
 protected:
  Internal_error_handler() : m_prev_internal_handler(nullptr) {}

  Internal_error_handler *prev_internal_handler() const {
    return m_prev_internal_handler;
  }

  virtual ~Internal_error_handler() {}

 public:
  /**
    Handle a sql condition.
    This method can be implemented by a subclass to achieve any of the
    following:
    - mask a warning/error internally, prevent exposing it to the user,
    - mask a warning/error and throw another one instead.
    When this method returns true, the sql condition is considered
    'handled', and will not be propagated to upper layers.
    It is the responsibility of the code installing an internal handler
    to then check for trapped conditions, and implement logic to recover
    from the anticipated conditions trapped during runtime.

    This mechanism is similar to C++ try/throw/catch:
    - 'try' correspond to <code>THD::push_internal_handler()</code>,
    - 'throw' correspond to <code>my_error()</code>,
    which invokes <code>my_message_sql()</code>,
    - 'catch' correspond to checking how/if an internal handler was invoked,
    before removing it from the exception stack with
    <code>THD::pop_internal_handler()</code>.

    @param thd the calling thread
    @param sql_errno the error number for the condition raised.
    @param sqlstate the SQLSTATE for the condition raised.
    @param level the severity level for the condition raised.
    @param msg the error message for the condition raised.
    @return true if the condition is handled
  */
  virtual bool handle_condition(THD *thd, uint sql_errno, const char *sqlstate,
                                Sql_condition::enum_severity_level *level,
                                const char *msg) = 0;

 private:
  Internal_error_handler *m_prev_internal_handler;
  friend class THD;
};

/**
  Implements the trivial error handler which cancels all error states
  and prevents an SQLSTATE to be set.
*/

class Dummy_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    /* Ignore error */
    return true;
  }
};

/**
  Implements the error handler for SET_VAR hint.
  For Sys_var_hint::update_vars handler accepts first warning or error.
  Subsequent error are ignored to avoid message duplication.
  For Sys_var_hint::restore_vars all warnings and errors are ignored
  since valid value is restored.
*/

class Set_var_error_handler : public Internal_error_handler {
 public:
  Set_var_error_handler(bool ignore_warn_arg)
      : Internal_error_handler(),
        ignore_warn(ignore_warn_arg),
        ignore_subsequent_messages(false) {}

  virtual bool handle_condition(THD *, uint, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *) {
    if (*level == Sql_condition::SL_ERROR) (*level) = Sql_condition::SL_WARNING;

    if (ignore_subsequent_messages) return true;
    ignore_subsequent_messages = true;

    return ignore_warn;
  }

  void reset_state() { ignore_subsequent_messages = false; }

 private:
  bool ignore_warn;
  bool ignore_subsequent_messages;
};

/**
  This class is an internal error handler implementation for
  DROP TABLE statements. The thing is that there may be warnings during
  execution of these statements, which should not be exposed to the user.
  This class is intended to silence such warnings.
*/

class Drop_table_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *thd, uint sql_errno, const char *sqlstate,
                                Sql_condition::enum_severity_level *level,
                                const char *msg);
};

/**
  Internal error handler to process an error from MDL_context::upgrade_lock()
  and mysql_lock_tables(). Used by implementations of HANDLER READ and
  LOCK TABLES LOCAL.
*/

class MDL_deadlock_and_lock_abort_error_handler
    : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_LOCK_ABORTED || sql_errno == ER_LOCK_DEADLOCK)
      m_need_reopen = true;

    return m_need_reopen;
  }

  bool need_reopen() const { return m_need_reopen; }
  void init() { m_need_reopen = false; }

 private:
  bool m_need_reopen;
};

/**
   An Internal_error_handler that suppresses errors regarding views'
   underlying tables that occur during privilege checking. It hides errors which
   show view underlying table information.
   This happens in the cases when

   - A view's underlying table (e.g. referenced in its SELECT list) does not
     exist or columns of underlying table are altered. There should not be an
     error as no attempt was made to access it per se.

   - Access is denied for some table, column, function or stored procedure
     such as mentioned above. This error gets raised automatically, since we
     can't untangle its access checking from that of the view itself.

    There are currently two mechanisms at work that handle errors for views
    based on an Internal_error_handler. This one and another one is
    Show_create_error_handler. The latter handles errors encountered during
    execution of SHOW CREATE VIEW, while this mechanism using this method is
    handles SELECT from views. The two methods should not clash.

*/
class View_error_handler : public Internal_error_handler {
  TABLE_LIST *m_top_view;

 public:
  View_error_handler(TABLE_LIST *top_view) : m_top_view(top_view) {}
  virtual bool handle_condition(THD *thd, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *message);
};

/**
  This internal handler is used to trap ER_NO_SUCH_TABLE and ER_BAD_DB_ERROR.
*/

class No_such_table_error_handler : public Internal_error_handler {
 public:
  No_such_table_error_handler() : m_handled_errors(0), m_unhandled_errors(0) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_BAD_DB_ERROR || sql_errno == ER_NO_SUCH_TABLE) {
      m_handled_errors++;
      return true;
    }

    m_unhandled_errors++;
    return false;
  }

  /**
    Returns true if one or more ER_NO_SUCH_TABLE and ER_BAD_DB_ERROR errors have
    been trapped and no other errors have been seen. false otherwise.
  */
  bool safely_trapped_errors() const {
    /*
      If m_unhandled_errors != 0, something else, unanticipated, happened,
      so the error is not trapped but returned to the caller.
      Multiple ER_NO_SUCH_TABLE and ER_BAD_DB_ERROR can be raised in case of
      views.
    */
    return ((m_handled_errors > 0) && (m_unhandled_errors == 0));
  }

 private:
  int m_handled_errors;
  int m_unhandled_errors;
};

/**
  This internal handler implements downgrade from SL_ERROR to SL_WARNING
  for statements which support IGNORE.
*/

class Ignore_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *thd, uint sql_errno, const char *sqlstate,
                                Sql_condition::enum_severity_level *level,
                                const char *msg);
};

/**
  This internal handler implements upgrade from SL_WARNING to SL_ERROR
  for the error codes affected by STRICT mode. Currently STRICT mode does
  not affect SELECT statements.
*/

class Strict_error_handler : public Internal_error_handler {
 public:
  enum enum_set_select_behavior {
    DISABLE_SET_SELECT_STRICT_ERROR_HANDLER,
    ENABLE_SET_SELECT_STRICT_ERROR_HANDLER
  };

  Strict_error_handler()
      : m_set_select_behavior(DISABLE_SET_SELECT_STRICT_ERROR_HANDLER) {}

  Strict_error_handler(enum_set_select_behavior param)
      : m_set_select_behavior(param) {}

  virtual bool handle_condition(THD *thd, uint sql_errno, const char *sqlstate,
                                Sql_condition::enum_severity_level *level,
                                const char *msg);

 private:
  /*
    For SELECT and SET statement, we do not always give error in STRICT mode.
    For triggers, Strict_error_handler is pushed in the beginning of statement.
    If a SELECT or SET is executed from the Trigger, it should not always give
    error. We use this flag to choose when to give error and when warning.
  */
  enum_set_select_behavior m_set_select_behavior;
};

/**
  The purpose of this error handler is to print out more user friendly error
  messages when an error regarding a functional index happens. Since functional
  indexes are implemented as hidden generated columns with an auto-generated
  name, we would end up printing errors like "Out of range value for column
  '912ec803b2ce49e4a541068d495ab570' at row 0". With this error handler, we
  end up printing something like "Out of range value for functional index
  'functional_index_2' at row 0" instead.

  The handler keeps track of the previous error handler that was in use, and
  calls that error handler to get the correct severity among other things.
*/
class Functional_index_error_handler : public Internal_error_handler {
 public:
  Functional_index_error_handler(const Field *field, THD *thd);

  Functional_index_error_handler(Create_field *field,
                                 const std::string &functional_index_name,
                                 THD *thd);

  Functional_index_error_handler(const std::string &functional_index_name,
                                 THD *thd);

  bool handle_condition(THD *thd, uint sql_errno, const char *,
                        Sql_condition::enum_severity_level *level,
                        const char *message) override;

  ~Functional_index_error_handler() override;

  void force_error_code(int error_code) { m_force_error_code = error_code; }

 private:
  std::string m_functional_index_name;
  THD *m_thd;
  bool m_pop_error_handler;
  int m_force_error_code;
};

//////////////////////////////////////////////////////////////////////////

/**
  After retrieving the tablespace name, the tablespace name is validated.
  If the name is invalid, it is ignored. The function used to validate
  the name, 'validate_tablespace_name()', emits errors. In the context of
  retrieving tablespace names, the errors must be ignored. This error handler
  makes sure this is done.
*/

class Tablespace_name_error_handler : public Internal_error_handler {
 public:
  bool handle_condition(THD *, uint sql_errno, const char *,
                        Sql_condition::enum_severity_level *, const char *) {
    return (sql_errno == ER_WRONG_TABLESPACE_NAME ||
            sql_errno == ER_TOO_LONG_IDENT);
  }
};

/*
  Disable ER_TOO_LONG_KEY for creation of system tables.
  TODO: This is a Workaround due to bug#20629014.
  Remove this internal error handler when the bug is fixed.
*/
class Key_length_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    return (sql_errno == ER_TOO_LONG_KEY);
  }
};

/**
  Error handler class to convert ER_LOCK_DEADLOCK error to
  ER_WARN_I_S_SKIPPED_TABLE/TABLESPACE error.

  Handler is pushed for opening a table or acquiring a MDL lock on
  tables for INFORMATION_SCHEMA views (system views) operations.
*/
class Info_schema_error_handler : public Internal_error_handler {
 public:
  Info_schema_error_handler(THD *thd, const String *schema_name,
                            const String *table_name);

  Info_schema_error_handler(THD *thd, const String *tablespace_name);

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *);

  bool is_error_handled() const { return m_error_handled; }

 private:
  bool m_can_deadlock;

  // Schema name
  const String *m_schema_name;

  // Table name
  const String *m_table_name;

  // Tablespace name
  const String *m_tablespace_name;

  enum class Mdl_object_type { TABLE, TABLESPACE };
  Mdl_object_type m_object_type;

  // Flag to indicate whether deadlock error is handled by the handler or not.
  bool m_error_handled = false;
};

/**
   An Internal_error_handler that converts errors related to foreign key
   constraint checks 'ER_NO_REFERENCED_ROW_2' and 'ER_ROW_IS_REFERENCED_2'
   to ER_NO_REFERENCED_ROW and ER_ROW_IS_REFERENCED based on privilege checks.
   This prevents from revealing parent and child tables information respectively
   when the foreign key constraint check fails and user does not have privileges
   to access those tables.
*/
class Foreign_key_error_handler : public Internal_error_handler {
  handler *m_table_handler;
  THD *m_thd;

 public:
  Foreign_key_error_handler(THD *thd, handler *table_handler)
      : m_table_handler(table_handler), m_thd(thd) {}
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *level,
                                const char *message);
};

/// An error handler that silences all warnings.
class Ignore_warnings_error_handler final : public Internal_error_handler {
 public:
  bool handle_condition(THD *, unsigned, const char *,
                        Sql_condition::enum_severity_level *level,
                        const char *) override {
    return *level == Sql_condition::SL_WARNING;
  }
};

#endif  // ERROR_HANDLER_INCLUDED
