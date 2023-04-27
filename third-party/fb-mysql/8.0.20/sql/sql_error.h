/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_ERROR_H
#define SQL_ERROR_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql_com.h" /* MYSQL_ERRMSG_SIZE */
#include "sql/sql_list.h"
#include "sql/sql_plist.h" /* I_P_List */
#include "sql_string.h"    /* String */

class THD;
class my_decimal;
struct MYSQL_TIME;

///////////////////////////////////////////////////////////////////////////

/**
  Representation of a SQL condition.
  A SQL condition can be a completion condition (note, warning),
  or an exception condition (error, not found).
*/
class Sql_condition {
 public:
  /**
    Enumeration value describing the severity of the condition.
  */
  enum enum_severity_level { SL_NOTE, SL_WARNING, SL_ERROR, SEVERITY_END };

  /**
    Get the MESSAGE_TEXT of this condition.
    @return the message text.
  */
  const char *message_text() const { return m_message_text.ptr(); }

  /**
    Get the MESSAGE_OCTET_LENGTH of this condition.
    @return the length in bytes of the message text.
  */
  size_t message_octet_length() const { return m_message_text.length(); }

  /**
    Get the RETURNED_SQLSTATE of this condition.
    @return the sql state.
  */
  const char *returned_sqlstate() const { return m_returned_sqlstate; }

  /**
    Get the MYSQL_ERRNO of this condition.
    @return the sql error number condition item.
  */
  uint mysql_errno() const { return m_mysql_errno; }

  /**
    Get the severity level of this condition.
    @return the severity level condition item.
  */
  Sql_condition::enum_severity_level severity() const {
    return m_severity_level;
  }

 private:
  /*
    The interface of Sql_condition is mostly private, by design,
    so that only the following code:
    - various raise_error() or raise_warning() methods in class THD,
    - the implementation of SIGNAL / RESIGNAL / GET DIAGNOSTICS
    - catch / re-throw of SQL conditions in stored procedures (sp_rcontext)
    is allowed to create / modify a SQL condition.
    Enforcing this policy prevents confusion, since the only public
    interface available to the rest of the server implementation
    is the interface offered by the THD methods (THD::raise_error()),
    which should be used.
  */
  friend class THD;
  friend class Diagnostics_area;
  friend class Sql_cmd_common_signal;
  friend class Sql_cmd_signal;
  friend class Sql_cmd_resignal;
  friend class sp_rcontext;
  friend class Condition_information_item;

  /**
    Constructor.

    @param mem_root Memory root to use for the condition items
                    of this condition.
  */
  Sql_condition(MEM_ROOT *mem_root);

  /**
    Constructor.

    @param mem_root          Memory root to use for the condition items
                             of this condition.
    @param mysql_errno       MYSQL_ERRNO
    @param returned_sqlstate RETURNED_SQLSTATE
    @param severity          Severity level - error, warning or note.
    @param message_text      MESSAGE_TEXT
  */
  Sql_condition(MEM_ROOT *mem_root, uint mysql_errno,
                const char *returned_sqlstate,
                Sql_condition::enum_severity_level severity,
                const char *message_text);

  /** Destructor. */
  ~Sql_condition() {}

  /**
    Copy optional condition items attributes.
    @param cond the condition to copy.
  */
  void copy_opt_attributes(const Sql_condition *cond);

  /**
    Set the condition message test.
    @param message_text  Message text, expressed in the character set derived
                         from the server --language option
  */
  void set_message_text(const char *message_text);

  /** Set the RETURNED_SQLSTATE of this condition. */
  void set_returned_sqlstate(const char *sqlstate) {
    memcpy(m_returned_sqlstate, sqlstate, SQLSTATE_LENGTH);
    m_returned_sqlstate[SQLSTATE_LENGTH] = '\0';
  }

  /** Set the CLASS_ORIGIN and SUBCLASS_ORIGIN of this condition. */
  void set_class_origins();

 private:
  /** SQL CLASS_ORIGIN condition item. */
  String m_class_origin;

  /** SQL SUBCLASS_ORIGIN condition item. */
  String m_subclass_origin;

  /** SQL CONSTRAINT_CATALOG condition item. */
  String m_constraint_catalog;

  /** SQL CONSTRAINT_SCHEMA condition item. */
  String m_constraint_schema;

  /** SQL CONSTRAINT_NAME condition item. */
  String m_constraint_name;

  /** SQL CATALOG_NAME condition item. */
  String m_catalog_name;

  /** SQL SCHEMA_NAME condition item. */
  String m_schema_name;

  /** SQL TABLE_NAME condition item. */
  String m_table_name;

  /** SQL COLUMN_NAME condition item. */
  String m_column_name;

  /** SQL CURSOR_NAME condition item. */
  String m_cursor_name;

  /** Message text, expressed in the character set implied by --language. */
  String m_message_text;

  /** MySQL extension, MYSQL_ERRNO condition item. */
  uint m_mysql_errno;

  /**
    SQL RETURNED_SQLSTATE condition item.
    This member is always NUL terminated.
  */
  char m_returned_sqlstate[SQLSTATE_LENGTH + 1];

  /** Severity (error, warning, note) of this condition. */
  Sql_condition::enum_severity_level m_severity_level;

  /** Pointers for participating in the list of conditions. */
  Sql_condition *m_next_condition;
  Sql_condition **m_prev_condition;

  /** Memory root to use to hold condition item values. */
  MEM_ROOT *m_mem_root;
};

///////////////////////////////////////////////////////////////////////////

size_t err_conv(char *buff, size_t to_length, const char *from,
                size_t from_length, const CHARSET_INFO *from_cs);

class ErrConvString {
  char err_buffer[MYSQL_ERRMSG_SIZE];
  size_t buf_length;

 public:
  explicit ErrConvString(const String *str) {
    buf_length = err_conv(err_buffer, sizeof(err_buffer), str->ptr(),
                          str->length(), str->charset());
  }

  ErrConvString(const char *str, const CHARSET_INFO *cs) {
    buf_length = err_conv(err_buffer, sizeof(err_buffer), str, strlen(str), cs);
  }

  ErrConvString(const char *str, size_t length) {
    buf_length = err_conv(err_buffer, sizeof(err_buffer), str, length,
                          &my_charset_latin1);
  }

  ErrConvString(const char *str, size_t length, const CHARSET_INFO *cs) {
    buf_length = err_conv(err_buffer, sizeof(err_buffer), str, length, cs);
  }

  ErrConvString(longlong nr) {
    buf_length = snprintf(err_buffer, sizeof(err_buffer), "%lld", nr);
  }

  ErrConvString(longlong nr, bool unsigned_flag) {
    buf_length = longlong10_to_str(nr, err_buffer, unsigned_flag ? 10 : -10) -
                 err_buffer;
  }

  ErrConvString(double nr);
  ErrConvString(const my_decimal *nr);
  ErrConvString(const MYSQL_TIME *ltime, uint dec);

  const char *ptr() const { return err_buffer; }
  size_t length() const { return buf_length; }
};

///////////////////////////////////////////////////////////////////////////

/**
  Stores status of the currently executed statement.
  Cleared at the beginning of the statement, and then
  can hold either OK, ERROR, or EOF status.
  Can not be assigned twice per statement.
*/
class Diagnostics_area {
  /** The type of the counted and doubly linked list of conditions. */
  typedef I_P_List<
      Sql_condition,
      I_P_List_adapter<Sql_condition, &Sql_condition::m_next_condition,
                       &Sql_condition::m_prev_condition>,
      I_P_List_counter, I_P_List_fast_push_back<Sql_condition>>
      Sql_condition_list;

 public:
  /** Const iterator used to iterate through the condition list. */
  typedef Sql_condition_list::Const_Iterator Sql_condition_iterator;

  enum enum_diagnostics_status {
    /** The area is cleared at start of a statement. */
    DA_EMPTY = 0,
    /** Set whenever one calls my_ok(). */
    DA_OK,
    /** Set whenever one calls my_eof(). */
    DA_EOF,
    /** Set whenever one calls my_error() or my_message(). */
    DA_ERROR,
    /** Set in case of a custom response, such as one from COM_STMT_PREPARE. */
    DA_DISABLED
  };

  Diagnostics_area(bool allow_unlimited_conditions);
  ~Diagnostics_area();

  void set_overwrite_status(bool can_overwrite_status) {
    m_can_overwrite_status = can_overwrite_status;
  }

  bool is_sent() const { return m_is_sent; }

  void set_is_sent(bool is_sent) { m_is_sent = is_sent; }

  /**
    Set OK status -- ends commands that do not return a
    result set, e.g. INSERT/UPDATE/DELETE.

    @param affected_rows  The number of rows affected by the last statement.
                          @sa Diagnostics_area::m_affected_rows.
    @param last_insert_id The value to be returned by LAST_INSERT_ID().
                          @sa Diagnostics_area::m_last_insert_id.
    @param message_text   The OK-message text.
  */
  void set_ok_status(ulonglong affected_rows, ulonglong last_insert_id,
                     const char *message_text);

  /**
    Set EOF status.

    @param thd  Thread context.
  */
  void set_eof_status(THD *thd);

  /**
    Set ERROR status in the Diagnostics Area. This function should be used to
    report fatal errors (such as out-of-memory errors) when no further
    processing is possible.

    @param thd              Thread handle
    @param mysql_errno      SQL-condition error number
  */
  void set_error_status(THD *thd, uint mysql_errno);

  /**
    Set ERROR status in the Diagnostics Area.

    @param mysql_errno        SQL-condition error number
    @param message_text       SQL-condition message
    @param returned_sqlstate  SQL-condition state
  */
  void set_error_status(uint mysql_errno, const char *message_text,
                        const char *returned_sqlstate);

  /**
    Mark the Diagnostics Area as 'DISABLED'.

    This is used in rare cases when the COM_ command at hand sends a response
    in a custom format. One example is COM_STMT_PREPARE.
  */
  void disable_status() {
    DBUG_ASSERT(m_status == DA_EMPTY);
    m_status = DA_DISABLED;
  }

  /**
    Clear this Diagnostics Area.

    Normally called at the end of a statement.
  */
  void reset_diagnostics_area();

  bool is_set() const { return m_status != DA_EMPTY; }

  bool is_error() const { return m_status == DA_ERROR; }

  bool is_eof() const { return m_status == DA_EOF; }

  bool is_ok() const { return m_status == DA_OK; }

  bool is_disabled() const { return m_status == DA_DISABLED; }

  enum_diagnostics_status status() const { return m_status; }

  const char *message_text() const {
    DBUG_ASSERT(m_status == DA_ERROR || m_status == DA_OK);
    return m_message_text;
  }

  uint mysql_errno() const {
    DBUG_ASSERT(m_status == DA_ERROR);
    return m_mysql_errno;
  }

  const char *returned_sqlstate() const {
    DBUG_ASSERT(m_status == DA_ERROR);
    return m_returned_sqlstate;
  }

  ulonglong affected_rows() const {
    DBUG_ASSERT(m_status == DA_OK);
    return m_affected_rows;
  }

  ulonglong last_insert_id() const {
    DBUG_ASSERT(m_status == DA_OK);
    return m_last_insert_id;
  }

  uint last_statement_cond_count() const {
    DBUG_ASSERT(m_status == DA_OK || m_status == DA_EOF);
    return m_last_statement_cond_count;
  }

  /** Return the number of conditions raised by the current statement. */
  ulong current_statement_cond_count() const {
    return m_current_statement_cond_count;
  }

  /**
    Reset between two COM_ commands. Conditions are preserved
    between commands, but m_current_statement_cond_count indicates
    the number of conditions of this particular statement only.
  */
  void reset_statement_cond_count() { m_current_statement_cond_count = 0; }

  /**
    Checks if the condition list contains SQL-condition with the given message.

    @param message_text    Message text
    @param message_length  Length of message_text

    @return true if the condition list contains an SQL-condition with the given
    message text.
  */
  bool has_sql_condition(const char *message_text, size_t message_length) const;

  /**
    Checks if the condition list contains SQL-condition with the given error
    code.

    @param sql_errno    Error code

    @return true if the condition list contains an SQL-condition with the given
    error code.
  */
  bool has_sql_condition(uint sql_errno) const;

  /**
    Reset the current condition information stored in the Diagnostics Area.
    Clear all conditions, the number of conditions, reset current row counter
    to point to the first row.
  */
  void reset_condition_info(THD *thd);

  /** Return the current counter value. */
  ulong current_row_for_condition() const {
    return m_current_row_for_condition;
  }

  /** Increment the current row counter to point at the next row. */
  void inc_current_row_for_condition() { m_current_row_for_condition++; }

  /** Set the current row counter to point to the given row number. */
  void set_current_row_for_condition(ulong rowno) {
    m_current_row_for_condition = rowno;
  }

  /** Reset the current row counter. Start counting from 1. */
  void reset_current_row_for_condition() { m_current_row_for_condition = 1; }

  /**
    The number of errors, or number of rows returned by SHOW ERRORS,
    also the value of session variable @@error_count.
  */
  ulong error_count(THD *thd) const;

  /**
    Used for @@warning_count system variable, which prints
    the number of rows returned by SHOW WARNINGS.
 */
  ulong warn_count(THD *thd) const;

  /**
    The number of conditions (errors, warnings and notes) in the list.
  */
  uint cond_count() const { return m_conditions_list.elements(); }

  Sql_condition_iterator sql_conditions() const { return m_conditions_list; }

  const char *get_first_condition_message();

  /** Make sure there is room for the given number of conditions. */
  void reserve_number_of_conditions(THD *thd, uint count);

  /**
    Add a new SQL-condition to the current list and increment the respective
    counters.

    @param thd                Thread context.
    @param mysql_errno        SQL-condition error number.
    @param returned_sqlstate  SQL-condition state.
    @param severity           SQL-condition severity.
    @param message_text       SQL-condition message.

    @return a pointer to the added SQL-condition.
  */
  Sql_condition *push_warning(THD *thd, uint mysql_errno,
                              const char *returned_sqlstate,
                              Sql_condition::enum_severity_level severity,
                              const char *message_text);

  /**
    Mark current SQL-conditions so that we can later know which
    SQL-conditions have been added.
  */
  void mark_preexisting_sql_conditions();

  /**
    Copy SQL-conditions that have been added since
    mark_preexisting_sql_conditions() was called.

    @param thd    Thread context.
    @param src_da Diagnostics Area to copy from.
  */
  void copy_new_sql_conditions(THD *thd, const Diagnostics_area *src_da);

  /**
    Copy all SQL-conditions from src_da to this DA.

    @param thd    Thread context.
    @param src_da Diagnostics Area to copy from.
  */
  void copy_sql_conditions_from_da(THD *thd, const Diagnostics_area *src_da);

  /**
    Copy Sql_conditions that are not SL_ERROR from the source
    Diagnostics Area to the current Diagnostics Area.

    @param thd    Thread context.
    @param src_da Diagnostics Area to copy from.
  */
  void copy_non_errors_from_da(THD *thd, const Diagnostics_area *src_da);

  /**
    @return SQL-condition, which corresponds to the error state in
    Diagnostics Area.
  */
  Sql_condition *error_condition() const;

 private:
  /**
    Add a new SQL-condition to the current list and increment the respective
    counters.

    @param thd            Thread context.
    @param sql_condition  SQL-condition to copy values from.

    @return a pointer to the added SQL-condition.
  */
  Sql_condition *push_warning(THD *thd, const Sql_condition *sql_condition);

  /**
    Push the given Diagnostics Area on top of the stack.
    "This" will then become the stacked Diagnostics Area.
    Conditions present in the new stacked Diagnostics Area
    will be copied to the new top Diagnostics Area.

    @note This function will not set THD::m_stmt_da.
          Use THD::push_diagnostics_area() instead.

    @param thd  Thread context
    @param da   Diagnostics Area to be come the top of
                the Diagnostics Area stack.
    @param copy_conditions
                Copy the conditions from the new second Diagnostics Area
                to the new first Diagnostics Area, as per SQL standard.
  */
  void push_diagnostics_area(THD *thd, Diagnostics_area *da,
                             bool copy_conditions);

  /**
    Pop "this" off the Diagnostics Area stack.

    @note This function will not set THD::m_stmt_da.
          Use THD::pop_diagnostics_area() instead.

    @returns The new top of the Diagnostics Area stack.
  */
  Diagnostics_area *pop_diagnostics_area();

  /**
    Returns the Diagnostics Area below the current diagnostics
    area on the stack.
  */
  const Diagnostics_area *stacked_da() const { return m_stacked_da; }

 private:
  /** Pointer to the Diagnostics Area below on the stack. */
  Diagnostics_area *m_stacked_da;

  /** A memory root to allocate conditions */
  MEM_ROOT m_condition_root;

  /** List of conditions of all severities. */
  Sql_condition_list m_conditions_list;

  /** List of conditions present in DA at handler activation. */
  List<const Sql_condition> m_preexisting_sql_conditions;

  /** True if status information is sent to the client. */
  bool m_is_sent;

  /** Set to make set_error_status after set_{ok,eof}_status possible. */
  bool m_can_overwrite_status;

  /** Indicates if push_warning() allows unlimited number of conditions. */
  bool m_allow_unlimited_conditions;

  enum_diagnostics_status m_status;

 private:
  /*
    This section contains basic attributes of Sql_condition to store
    information about error (SQL-condition of error severity) or OK-message.
    The attributes are inlined here (instead of using Sql_condition) to be able
    to store the information in case of out-of-memory error.
  */

  /**
    Message buffer. It is used only when DA is in OK or ERROR status.
    If DA status is ERROR, it's the MESSAGE_TEXT attribute of SQL-condition.
    If DA status is OK, it's the OK-message to be sent.
  */
  char m_message_text[MYSQL_ERRMSG_SIZE];

  /**
    SQL RETURNED_SQLSTATE condition item.
    This member is always NUL terminated.
  */
  char m_returned_sqlstate[SQLSTATE_LENGTH + 1];

  /**
    SQL error number. One of ER_ codes from share/errmsg.txt.
    Set by set_error_status.
  */
  uint m_mysql_errno;

  /**
    The number of rows affected by the last statement. This is
    semantically close to thd->row_count_func, but has a different
    life cycle. thd->row_count_func stores the value returned by
    function ROW_COUNT() and is cleared only by statements that
    update its value, such as INSERT, UPDATE, DELETE and few others.
    This member is cleared at the beginning of the next statement.

    We could possibly merge the two, but life cycle of thd->row_count_func
    can not be changed.
  */
  ulonglong m_affected_rows;

  /**
    Similarly to the previous member, this is a replacement of
    thd->first_successful_insert_id_in_prev_stmt, which is used
    to implement LAST_INSERT_ID().
  */
  ulonglong m_last_insert_id;

  /**
    Number of conditions of this last statement. May differ from
    the number of conditions returned by SHOW WARNINGS e.g. in case
    the statement doesn't clear the conditions, and doesn't generate
    them.
  */
  uint m_last_statement_cond_count;

  /**
    The number of conditions of the current statement. m_conditions_list
    life cycle differs from statement life cycle -- it may span
    multiple statements. In that case we get
    m_current_statement_cond_count 0, whereas m_conditions_list is not empty.
  */
  uint m_current_statement_cond_count;

  /** A break down of the number of conditions per severity (level). */
  uint m_current_statement_cond_count_by_sl[(uint)Sql_condition::SEVERITY_END];

  /**
    Row counter, to print in errors and warnings. Not increased in
    create_sort_index(); may differ from examined_row_count.
  */
  ulong m_current_row_for_condition;

  /** Save @@error_count before pre-clearing the DA. */
  ulong m_saved_error_count;

  /** Save @@warning_count before pre-clearing the DA. */
  ulong m_saved_warn_count;

  friend class THD;
};

///////////////////////////////////////////////////////////////////////////

void push_warning(THD *thd, Sql_condition::enum_severity_level severity,
                  uint code, const char *message_text);

/**
  Convenience function for sending a warning with level SL_WARNING and no
  arguments to the message.

  @param thd The session to send the warning to.
  @param code The warning number.
*/
void push_warning(THD *thd, uint code);

/*
  Note that this MY_ATTRIBUTE check cannot detect number/type mismatch
  since the format string is not known at compile time.
  It can however detect if push_warning_printf() is used without any
  printf arguments. In such cases, use push_warning() instead.
*/
void push_warning_printf(THD *thd, Sql_condition::enum_severity_level severity,
                         uint code, const char *format, ...)
    MY_ATTRIBUTE((format(printf, 4, 5)));

/**
  Generates a warning that a feature is deprecated.

  Using it as
    push_deprecated_warn(thd, "BAD", "'GOOD'");
  Will result in a warning:
    "The syntax 'BAD' is deprecated and will be removed in a
     future release. Please use 'GOOD' instead"

  If a function is deprecated, it should implement
  Item_func::is_deprecated() to return true to prevent the
  usage of the function in the generated column expression.

  @param thd         Thread context. If NULL, warning is written
                     to the error log, otherwise the warning is
                     sent to the client.
  @param old_syntax  Deprecated syntax.
  @param new_syntax  Replacement syntax.
*/
void push_deprecated_warn(THD *thd, const char *old_syntax,
                          const char *new_syntax);

/**
  Generates a warning that a feature is deprecated.

  Using it as
    push_deprecated_warn_no_replacement(thd, "old");
  Will result in a warning:
    "The syntax 'old' is deprecated and will be removed in a
     future release.

  If a function is deprecated, it should implement
  Item_func::is_deprecated() to return true to prevent the
  usage of the function in the generated column expression.

  @param thd         Thread context. If NULL, warning is written
                     to the error log, otherwise the warning is
                     sent to the client.
  @param old_syntax  Deprecated syntax.
*/
void push_deprecated_warn_no_replacement(THD *thd, const char *old_syntax);

bool mysqld_show_warnings(THD *thd, ulong levels_to_show);

size_t convert_error_message(char *to, size_t to_length,
                             const CHARSET_INFO *to_cs, const char *from,
                             size_t from_length, const CHARSET_INFO *from_cs,
                             uint *errors);

extern const LEX_CSTRING warning_level_names[];

bool is_sqlstate_valid(const LEX_STRING *sqlstate);

/**
  Checks if the specified SQL-state-string defines COMPLETION condition.
  This function assumes that the given string contains a valid SQL-state.

  @param s the condition SQLSTATE.

  @retval true if the given string defines COMPLETION condition.
  @retval false otherwise.
*/
inline bool is_sqlstate_completion(const char *s) {
  return s[0] == '0' && s[1] == '0';
}

/**
  Checks if the specified SQL-state-string defines WARNING condition.
  This function assumes that the given string contains a valid SQL-state.

  @param s the condition SQLSTATE.

  @retval true if the given string defines WARNING condition.
  @retval false otherwise.
*/
inline bool is_sqlstate_warning(const char *s) {
  return s[0] == '0' && s[1] == '1';
}

/**
  Checks if the specified SQL-state-string defines NOT FOUND condition.
  This function assumes that the given string contains a valid SQL-state.

  @param s the condition SQLSTATE.

  @retval true if the given string defines NOT FOUND condition.
  @retval false otherwise.
*/
inline bool is_sqlstate_not_found(const char *s) {
  return s[0] == '0' && s[1] == '2';
}

/**
  Checks if the specified SQL-state-string defines EXCEPTION condition.
  This function assumes that the given string contains a valid SQL-state.

  @param s the condition SQLSTATE.

  @retval true if the given string defines EXCEPTION condition.
  @retval false otherwise.
*/
inline bool is_sqlstate_exception(const char *s) {
  return s[0] != '0' || s[1] > '2';
}

void warn_on_deprecated_charset(THD *thd, const CHARSET_INFO *cs,
                                const char *alias,
                                const char *option = nullptr);
void warn_on_deprecated_collation(THD *thd, const CHARSET_INFO *collation,
                                  const char *option = nullptr);

#endif  // SQL_ERROR_H
