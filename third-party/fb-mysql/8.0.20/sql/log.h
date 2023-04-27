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

/**
  @file sql/log.h

  Error logging, slow query logging, general query logging:
  If it's server-internal, and it's logging, it's here.

  Components/services should NOT include this, but include
  include/mysql/components/services/log_builtins.h instead
  to gain access to the error logging stack.

  Legacy plugins (pre-"services") will likely include
  include/mysql/service_my_plugin_log.h instead.
*/

#ifndef LOG_H
#define LOG_H

#include <mysql/components/services/log_shared.h>
#include <stdarg.h>
#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_command.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_thread_local.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_file_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql_com.h"
#include "sql/auth/sql_security_ctx.h"  // Security_context

class THD;
struct CHARSET_INFO;
struct TABLE_LIST;

////////////////////////////////////////////////////////////
//
// Slow/General Log
//
////////////////////////////////////////////////////////////

/*
  System variables controlling logging:

  log_output (--log-output)
    Values: NONE, FILE, TABLE
    Select output destination. Does not enable logging.
    Can set more than one (e.g. TABLE | FILE).

  general_log (--general_log)
  slow_query_log (--slow_query_log)
    Values: 0, 1
    Enable/disable general/slow query log.

  general_log_file (--general-log-file)
  slow_query_log_file (--slow-query-log-file)
    Values: filename
    Set name of general/slow query log file.

  sql_log_off
    Values: ON, OFF
    Enable/disable general query log (OPTION_LOG_OFF).

  log_queries_not_using_indexes (--log-queries-not-using-indexes)
    Values: ON, OFF
    Control slow query logging of queries that do not use indexes.

  --log-raw
    Values: ON, OFF
    Control query rewrite of passwords to the general log.

  --log-short-format
    Values: ON, OFF
    Write short format to the slow query log (and the binary log).

  --log-slow-admin-statements
    Values: ON, OFF
    Log statements such as OPTIMIZE TABLE, ALTER TABLE to the slow query log.

  --log-slow-slave-statements
    Values: ON, OFF

  log_throttle_queries_not_using_indexes
    Values: INT
    Number of queries not using indexes logged to the slow query log per min.
*/

/**
  Write a message to a log (for now just used for error log).
  This is a variadic convenience interface to the logging components
  (which use the log_line structure internally), e.g.

  log_message(LOG_TYPE_ERROR,
              LOG_ITEM_LOG_PRIO,    INFORMATION_LEVEL,
              LOG_ITEM_LOG_MESSAGE, "file %s is %f %% yellow",
                                    filename, yellowfication);

  For use by legacy sql_print_*(), legacy my_plugin_log_message();
  also available via the log_builtins service as message().

  Wherever possible, use the fluent C++ wrapper LogErr()
  (see log_builtins.h) instead.

  See log_shared.h for LOG_TYPEs as well as for allowed LOG_ITEM_ types.
*/
int log_vmessage(int log_type, va_list lili);
int log_message(int log_type, ...);

/**
  A helper that we can stubify so we don't have to pull all of THD
  into the unit tests.

  @param   thd  a thd
  @retval       its thread-ID
*/
my_thread_id log_get_thread_id(THD *thd);

/** Type of the log table */
enum enum_log_table_type {
  QUERY_LOG_NONE = 0,
  QUERY_LOG_SLOW = 1,
  QUERY_LOG_GENERAL = 2,
  QUERY_LOG_GAP_LOCK = 3
};

/**
   Abstract superclass for handling logging to slow/general logs.
   Currently has two subclasses, for table and file based logging.
*/
class Log_event_handler {
 public:
  Log_event_handler() {}
  virtual ~Log_event_handler() {}

  /**
     Log a query to the slow log.

     @param thd                THD of the query
     @param current_utime      Current timestamp in microseconds
     @param query_start_arg    Command start timestamp in microseconds
     @param user_host          The pointer to the string with user\@host info
     @param user_host_len      Length of the user_host string
     @param query_utime        Number of microseconds query execution took
     @param lock_utime         Number of microseconds the query was locked
     @param is_command         The flag which determines whether the sql_text
                               is a query or an administrator command
     @param sql_text           The query or administrator in textual form
     @param sql_text_len       The length of sql_text string
     @param query_start_status Pointer to a snapshot of thd->status_var taken
                               at the start of execution

     @return true if error, false otherwise.
  */
  virtual bool log_slow(THD *thd, ulonglong current_utime,
                        ulonglong query_start_arg, const char *user_host,
                        size_t user_host_len, ulonglong query_utime,
                        ulonglong lock_utime, bool is_command,
                        const char *sql_text, size_t sql_text_len,
                        struct System_status_var *query_start_status) = 0;

  /**
     Log command to the general log.

     @param  thd               THD of the query
     @param  event_utime       Command start timestamp in micro seconds
     @param  user_host         The pointer to the string with user\@host info
     @param  user_host_len     Length of the user_host string. this is computed
                               once and passed to all general log event handlers
     @param  thread_id         Id of the thread, issued a query
     @param  command_type      The type of the command being logged
     @param  command_type_len  The length of the string above
     @param  sql_text          The very text of the query being executed
     @param  sql_text_len      The length of sql_text string
     @param  client_cs         Character set to use for strings

     @return This function attempts to never call my_error(). This is
     necessary, because general logging happens already after a statement
     status has been sent to the client, so the client can not see the
     error anyway. Besides, the error is not related to the statement
     being executed and is internal, and thus should be handled
     internally (@todo: how?).
     If a write to the table has failed, the function attempts to
     write to a short error message to the file. The failure is also
     indicated in the return value.

     @retval  false   OK
     @retval  true    error occurred
  */
  virtual bool log_general(THD *thd, ulonglong event_utime,
                           const char *user_host, size_t user_host_len,
                           my_thread_id thread_id, const char *command_type,
                           size_t command_type_len, const char *sql_text,
                           size_t sql_text_len,
                           const CHARSET_INFO *client_cs) = 0;
  /**
    Log command to the gap lock log.

    @param  thd               THD of the query
    @param  event_utime       Command start timestamp in micro seconds
    @param  user_host         The pointer to the string with user\@host info
    @param  user_host_len     Length of the user_host string. this is computed
    once and passed to all general log event handlers
    @param  thread_id         Id of the thread, issued a query
    @param  command_type      The type of the command being logged
    @param  command_type_len  The length of the string above
    @param  sql_text          The very text of the query being executed
    @param  sql_text_len      The length of sql_text string

    @retval  false   OK
    @retval  true    error occured
  */
  virtual bool log_gap_lock(THD *thd, ulonglong event_utime,
                            const char *user_host, size_t user_host_len,
                            my_thread_id thread_id, const char *command_type,
                            size_t command_type_len, const char *sql_text,
                            size_t sql_text_len) = 0;
};

/** Class responsible for table based logging. */
class Log_to_csv_event_handler : public Log_event_handler {
 public:
  /** @see Log_event_handler::log_slow(). */
  virtual bool log_slow(THD *thd, ulonglong current_utime,
                        ulonglong query_start_arg, const char *user_host,
                        size_t user_host_len, ulonglong query_utime,
                        ulonglong lock_utime, bool is_command,
                        const char *sql_text, size_t sql_text_len,
                        struct System_status_var *query_start_status);

  /** @see Log_event_handler::log_general(). */
  virtual bool log_general(THD *thd, ulonglong event_utime,
                           const char *user_host, size_t user_host_len,
                           my_thread_id thread_id, const char *command_type,
                           size_t command_type_len, const char *sql_text,
                           size_t sql_text_len, const CHARSET_INFO *client_cs);

  /** @see Log_event_handler::log_gap_lock(). */
  virtual bool log_gap_lock(THD *thd, ulonglong event_utime,
                            const char *user_host, size_t user_host_len,
                            my_thread_id thread_id, const char *command_type,
                            size_t command_type_len, const char *sql_text,
                            size_t sql_text_len);

 private:
  /**
     Check if log table for given log type exists and can be opened.

     @param thd       Thread handle
     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL

     @return true if table could not be opened, false otherwise.
  */
  bool activate_log(THD *thd, enum_log_table_type log_type);

  friend class Query_logger;
};

/* Log event handler flags */
static const uint LOG_NONE = 1;
static const uint LOG_FILE = 2;
static const uint LOG_TABLE = 4;

class Log_to_file_event_handler;

/** Class which manages slow and general log event handlers. */
class Query_logger {
  /**
     Currently we have only 2 kinds of logging functions: old-fashioned
     file logs and csv logging routines.
  */
  static const uint MAX_LOG_HANDLERS_NUM = 2;

  /**
     RW-lock protecting Query_logger.
     R-lock taken when writing to slow/general query log.
     W-lock taken when activating/deactivating logs.
  */
  mysql_rwlock_t LOCK_logger;

  /** Available log handlers. */
  Log_to_csv_event_handler table_log_handler;
  Log_to_file_event_handler *file_log_handler;

  /** NULL-terminated arrays of log handlers. */
  Log_event_handler *slow_log_handler_list[MAX_LOG_HANDLERS_NUM + 1];
  Log_event_handler *general_log_handler_list[MAX_LOG_HANDLERS_NUM + 1];
  Log_event_handler *gap_lock_log_handler_list[MAX_LOG_HANDLERS_NUM + 1];

 private:
  /**
     Setup log event handlers for the given log_type.

     @param log_type     QUERY_LOG_SLOW or QUERY_LOG_GENERAL
     @param log_printer  Bitmap of LOG_NONE, LOG_FILE, LOG_TABLE
  */
  void init_query_log(enum_log_table_type log_type, ulonglong log_printer);

 public:
  Query_logger() : file_log_handler(nullptr) {}

  /**
     Check if table logging is turned on for the given log_type.

     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL

     @return true if table logging is on, false otherwise.
  */
  bool is_log_table_enabled(enum_log_table_type log_type) const;

  /**
     Check if file logging is turned on for the given log type.

     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL

     @return true if the file logging is on, false otherwise.
  */
  bool is_log_file_enabled(enum_log_table_type log_type) const;

  /**
     Perform basic log initialization: create file-based log handler.

     We want to initialize all log mutexes as soon as possible,
     but we cannot do it in constructor, as safe_mutex relies on
     initialization, performed by MY_INIT(). This why this is done in
     this function.
  */
  void init();

  /** Free memory. Nothing could be logged after this function is called. */
  void cleanup();

  /**
     Log slow query with all enabled log event handlers.

     @param thd                 THD of the statement being logged.
     @param query               The query string being logged.
     @param query_length        The length of the query string.
     @param query_start_status  Pointer to a snapshot of thd->status_var taken
                                at the start of execution

     @return true if error, false otherwise.
  */
  bool slow_log_write(THD *thd, const char *query, size_t query_length,
                      struct System_status_var *query_start_status);

  /**
     Write printf style message to general query log.

     @param thd           THD of the statement being logged.
     @param command       COM of statement being logged.
     @param format        Printf style format of message.
     @param ...           Printf parameters to write.

     @return true if error, false otherwise.
  */
  bool general_log_print(THD *thd, enum_server_command command,
                         const char *format, ...)
      MY_ATTRIBUTE((format(printf, 4, 5)));

  /**
     Write query to general query log.

     @param thd           THD of the statement being logged.
     @param command       COM of statement being logged.
     @param query         The query string being logged.
     @param query_length  The length of the query string.

     @return true if error, false otherwise.
  */
  bool general_log_write(THD *thd, enum_server_command command,
                         const char *query, size_t query_length);

  /**
    Write printf style message to gap lock query log.

    @param thd           THD of the statement being logged.
    @param command       COM of statement being logged.
    @param format        Printf style format of message.
    @param ...           Printf parameters to write.

    @return true if error, false otherwise.
  */
  bool gap_lock_log_print(THD *thd, enum_server_command command,
                          const char *format, ...)
      MY_ATTRIBUTE((format(printf, 4, 5)));

  /**
    Write query to gap lock query log.

    @param thd           THD of the statement being logged.
    @param command       COM of statement being logged.
    @param query         The query string being logged.
    @param query_length  The length of the query string.

    @return true if error, false otherwise.
  */
  bool gap_lock_log_write(THD *thd, enum_server_command command,
                          const char *query, size_t query_length);

  /**
     Enable log event handlers for slow/general log.

     @param log_printer     Bitmask of log event handlers.

     @note Acceptable values are LOG_NONE, LOG_FILE, LOG_TABLE
  */
  void set_handlers(ulonglong log_printer);

  /**
     Activate log handlers for the given log type.

     @param thd       Thread handle
     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL

     @return true if error, false otherwise.
  */
  bool activate_log_handler(THD *thd, enum_log_table_type log_type);

  /**
     Close file log for the given log type.

     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL
  */
  void deactivate_log_handler(enum_log_table_type log_type);

  /**
     Close file log for the given log type and the reopen it.

     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL
  */
  bool reopen_log_file(enum_log_table_type log_type);

  /**
     Read log file name from global variable opt_*_logname.
     If called from a sys_var update function, the caller
     must hold a lock protecting the sys_var
     (LOCK_global_system_variables, a polylock for the
     variable, etc.).

     @param log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL
  */
  bool set_log_file(enum_log_table_type log_type);

  /**
     Check if given TABLE_LIST has a query log table name and
     optionally check if the query log is currently enabled.

     @param table_list       TABLE_LIST representing the table to check
     @param check_if_opened  Always return QUERY_LOG_NONE unless the
                             query log table is enabled.

     @retval QUERY_LOG_NONE, QUERY_LOG_SLOW or QUERY_LOG_GENERAL
  */
  enum_log_table_type check_if_log_table(TABLE_LIST *table_list,
                                         bool check_if_opened) const;
};

extern Query_logger query_logger;

/**
   Create the name of the query log specified.

   This method forms a new path + file name for the log specified.

   @param[in] buff      Location for building new string.
   @param[in] log_type  QUERY_LOG_SLOW or QUERY_LOG_GENERAL

   @returns Pointer to new string containing the name.
*/
char *make_query_log_name(char *buff, enum_log_table_type log_type);

/**
  Check given log name against certain blacklisted names/extensions.

  @param name     Log name to check
  @param len      Length of log name

  @returns true if name is valid, false otherwise.
*/
bool is_valid_log_name(const char *name, size_t len);

/**
  Check whether we need to write the current statement (or its rewritten
  version if it exists) to the slow query log.
  As a side-effect, a digest of suppressed statements may be written.

  @param thd          thread handle

  @retval
    true              statement needs to be logged
  @retval
    false             statement does not need to be logged
*/
bool log_slow_applicable(THD *thd);

/**
  Unconditionally writes the current statement (or its rewritten version if it
  exists) to the slow query log.

  @param thd                 thread handle
  @param query_start_status  Pointer to a snapshot of thd->status_var taken
                             at the start of execution
*/
void log_slow_do(THD *thd, struct System_status_var *query_start_status);

/**
  Check whether we need to write the current statement to the slow query
  log. If so, do so. This is a wrapper for the two functions above;
  most callers should use this wrapper.  Only use the above functions
  directly if you have expensive rewriting that you only need to do if
  the query actually needs to be logged (e.g. SP variables / NAME_CONST
  substitution when executing a PROCEDURE).
  A digest of suppressed statements may be logged instead of the current
  statement.

  @param thd                 thread handle
  @param query_start_status  Pointer to a snapshot of thd->status_var taken
                             at the start of execution
*/
void log_slow_statement(THD *thd, struct System_status_var *query_start_status);

/**
  @class Log_throttle
  @brief Base class for rate-limiting a log (slow query log etc.)
*/

class Log_throttle {
  /**
    When will/did current window end?
  */
  ulonglong window_end;

  /**
    Log no more than rate lines of a given type per window_size
    (e.g. per minute, usually LOG_THROTTLE_WINDOW_SIZE).
  */
  const ulong window_size;

  /**
   There have been this many lines of this type in this window,
   including those that we suppressed. (We don't simply stop
   counting once we reach the threshold as we'll write a summary
   of the suppressed lines later.)
  */
  ulong count;

 protected:
  /**
    Template for the summary line. Should contain %lu as the only
    conversion specification.
  */
  const char *summary_template;

  /**
    Start a new window.
  */
  void new_window(ulonglong now);

  /**
    Increase count of logs we're handling.

    @param rate  Limit on records to be logged during the throttling window.

    @retval true -  log rate limit is exceeded, so record should be supressed.
    @retval false - log rate limit is not exceeded, record should be logged.
  */
  bool inc_log_count(ulong rate) { return (++count > rate); }

  /**
    Check whether we're still in the current window. (If not, the caller
    will want to print a summary (if the logging of any lines was suppressed),
    and start a new window.)
  */
  bool in_window(ulonglong now) const { return (now < window_end); }

  /**
    Prepare a summary of suppressed lines for logging.
    This function returns the number of queries that were qualified for
    inclusion in the log, but were not printed because of the rate-limiting.
    The summary will contain this count as well as the respective totals for
    lock and execution time.
    This function assumes that the caller already holds the necessary locks.

    @param rate  Limit on records logged during the throttling window.
  */
  ulong prepare_summary(ulong rate);

  /**
    @param window_usecs  ... in this many micro-seconds
    @param msg           use this template containing %lu as only non-literal
  */
  Log_throttle(ulong window_usecs, const char *msg)
      : window_end(0),
        window_size(window_usecs),
        count(0),
        summary_template(msg) {}

 public:
  /**
    We're rate-limiting messages per minute; 60,000,000 microsecs = 60s
    Debugging is less tedious with a window in the region of 5000000
  */
  static const ulong LOG_THROTTLE_WINDOW_SIZE = 60000000;
};

/**
  @class Slow_log_throttle
  @brief Used for rate-limiting the slow query log.
*/

class Slow_log_throttle : public Log_throttle {
 private:
  /**
    We're using our own (empty) security context during summary generation.
    That way, the aggregate value of the suppressed queries isn't printed
    with a specific user's name (i.e. the user who sent a query when or
    after the time-window closes), as that would be misleading.
  */
  Security_context aggregate_sctx;

  /**
    Total of the execution times of queries in this time-window for which
    we suppressed logging. For use in summary printing.
  */
  ulonglong total_exec_time;

  /**
    Total of the lock times of queries in this time-window for which
    we suppressed logging. For use in summary printing.
  */
  ulonglong total_lock_time;

  /**
    A reference to the threshold ("no more than n log lines per ...").
    References a (system-?) variable in the server.
  */
  ulong *rate;

  /**
    The routine we call to actually log a line (i.e. our summary).
    The signature miraculously coincides with slow_log_print().
  */
  bool (*log_summary)(THD *, const char *, size_t, struct System_status_var *);

  /**
    Slow_log_throttle is shared between THDs.
  */
  mysql_mutex_t *LOCK_log_throttle;

  /**
    Start a new window.
  */
  void new_window(ulonglong now);

  /**
    Actually print the prepared summary to log.
  */
  void print_summary(THD *thd, ulong suppressed, ulonglong print_lock_time,
                     ulonglong print_exec_time);

 public:
  /**
    @param threshold     suppress after this many queries ...
    @param lock          mutex to use for consistency of calculations
    @param window_usecs  ... in this many micro-seconds
    @param logger        call this function to log a single line (our summary)
    @param msg           use this template containing %lu as only non-literal
  */
  Slow_log_throttle(ulong *threshold, mysql_mutex_t *lock, ulong window_usecs,
                    bool (*logger)(THD *, const char *, size_t,
                                   struct System_status_var *),
                    const char *msg);

  /**
    Prepare and print a summary of suppressed lines to log.
    (For now, slow query log.)
    The summary states the number of queries that were qualified for
    inclusion in the log, but were not printed because of the rate-limiting,
    and their respective totals for lock and execution time.
    This wrapper for prepare_summary() and print_summary() handles the
    locking/unlocking.

    @param thd                 The THD that tries to log the statement.
    @retval false              Logging was not supressed, no summary needed.
    @retval true               Logging was supressed; a summary was printed.
  */
  bool flush(THD *thd);

  /**
    Top-level function.
    @param thd                 The THD that tries to log the statement.
    @param eligible            Is the statement of the type we might suppress?
    @retval true               Logging should be supressed.
    @retval false              Logging should not be supressed.
  */
  bool log(THD *thd, bool eligible);
};

/**
  @class Slow_log_throttle
  @brief Used for rate-limiting a error logs.
*/

class Error_log_throttle : public Log_throttle {
 private:
  loglevel ll;
  uint err_code;
  const char *subsys;

  /**
    Actually print the prepared summary to log.
  */
  void print_summary(ulong suppressed) {
    log_message(LOG_TYPE_ERROR, LOG_ITEM_LOG_PRIO, (longlong)ll,
                LOG_ITEM_SQL_ERRCODE, (longlong)err_code, LOG_ITEM_SRV_SUBSYS,
                subsys, LOG_ITEM_LOG_MESSAGE, summary_template,
                (unsigned long)suppressed);
  }

 public:
  /**
    @param window_usecs  ... in this many micro-seconds (see Log_throttle)
    @param lvl           severity of the incident (error, warning, info)
    @param errcode       MySQL error code (e.g. ER_STARTUP)
    @param subsystem     subsystem tag, or nullptr for none
    @param msg           use this message template containing %lu as only
                         non-literal (for "number of suppressed events",
                         see Log_throttle)
  */
  Error_log_throttle(ulong window_usecs, loglevel lvl, uint errcode,
                     const char *subsystem, const char *msg)
      : Log_throttle(window_usecs, msg),
        ll(lvl),
        err_code(errcode),
        subsys(subsystem) {}

  /**
    Prepare and print a summary of suppressed lines to log.
    (For now, slow query log.)
    The summary states the number of queries that were qualified for
    inclusion in the log, but were not printed because of the rate-limiting.

    @retval false              Logging was not suppressed, no summary needed.
    @retval true               Logging was suppressed; a summary was printed.
  */
  bool flush();

  /**
    Top-level function.
    @retval true               Logging should be suppressed.
    @retval false              Logging should not be suppressed.
  */
  bool log();
};

extern Slow_log_throttle log_throttle_qni;
extern Slow_log_throttle log_throttle_ddl;

////////////////////////////////////////////////////////////
//
// Error Log
//
////////////////////////////////////////////////////////////

/*
  Set up some convenience defines to help us while we change
  old-style ("sql_print_...()") calls to new-style ones
  ("LogErr(...)").  New code should not use these, nor should
  it use sql_print_...().
*/

/**
  Set up basics, fetch message for "errcode", insert any va_args,
  call the new error stack.  A helper for the transition to the
  new stack.
*/
#define log_errlog(level, errcode, ...)                                  \
  log_message(LOG_TYPE_ERROR, LOG_ITEM_LOG_PRIO, (longlong)level,        \
              LOG_ITEM_SRV_SUBSYS, LOG_SUBSYSTEM_TAG, LOG_ITEM_SRC_LINE, \
              (longlong)__LINE__, LOG_ITEM_SRC_FILE, MY_BASENAME,        \
              LOG_ITEM_LOG_LOOKUP, (longlong)errcode, ##__VA_ARGS__)

/**
  Default tags + freeform message. A helper for re#defining sql_print_*()
  to go through the new error log service stack.

  Remember to never blindly LOG_MESSAGE a string you that may contain
  user input as it may contain % which will be treated as substitutions.

  BAD:   LOG_ITEM_LOG_MESSAGE,  dodgy_message
  OK:    LOG_ITEM_LOG_MESSAGE,  "%s", dodgy_message
  GOOD:  LOG_ITEM_LOG_VERBATIM, dodgy_message
*/
#define log_errlog_formatted(level, ...)                                 \
  log_message(LOG_TYPE_ERROR, LOG_ITEM_LOG_PRIO, (longlong)level,        \
              LOG_ITEM_SRV_SUBSYS, LOG_SUBSYSTEM_TAG, LOG_ITEM_SRC_LINE, \
              (longlong)__LINE__, LOG_ITEM_SRC_FILE, MY_BASENAME,        \
              LOG_ITEM_LOG_MESSAGE, ##__VA_ARGS__)

/**
  Set up the default tags, then let us add/override any key/value we like,
  call the new error stack.  A helper for the transition to the new stack.
*/
#define log_errlog_rich(level, ...)                                      \
  log_message(LOG_TYPE_ERROR, LOG_ITEM_LOG_PRIO, (longlong)level,        \
              LOG_ITEM_SRV_SUBSYS, LOG_SUBSYSTEM_TAG, LOG_ITEM_SRC_LINE, \
              (longlong)__LINE__, LOG_ITEM_SRC_FILE, MY_BASENAME, __VA_ARGS__)

/**
  Define sql_print_*() so they use the new log_message()
  variadic convenience interface to logging.  This lets
  us switch over the bulk of the messages right away until
  we can attend to them individually; it also verifies that
  we no longer use function pointers to log functions.

  As before, sql_print_*() only accepts a printf-style
  format string, and the arguments to same, if any.
*/
#define sql_print_information(...) \
  log_errlog_formatted(INFORMATION_LEVEL, ##__VA_ARGS__)

#define sql_print_warning(...) \
  log_errlog_formatted(WARNING_LEVEL, ##__VA_ARGS__)

#define sql_print_error(...) log_errlog_formatted(ERROR_LEVEL, ##__VA_ARGS__)

/**
  Prints a printf style message to the error log.

  A thin wrapper around log_message() for local_message_hook,
  Table_check_intact::report_error, and others.

  @param level          The level of the msg significance
  @param ecode          Error code of the error message.
  @param args           va_list list of arguments for the message
*/
void error_log_print(enum loglevel level, uint ecode, va_list args);

/**
  Initialize structures (e.g. mutex) needed by the error log.

  @note This function accesses shared resources without protection, so
  it should only be called while the server is running single-threaded.

  @note The error log can still be used before this function is called,
  but that should only be done single-threaded.

  @retval true   an error occurred
  @retval false  basic error logging is now available in multi-threaded mode
*/
bool init_error_log();

/**
  Open the error log and redirect stderr and optionally stdout
  to the error log file. The streams are reopened only for
  appending (writing at end of file).

  @note
    On error, my_error() is not called here.
    So, caller of this function should call my_error() to keep the protocol.

  @note This function also writes any error log messages that
  have been buffered by calling flush_error_log_messages().

  @param filename        Name of error log file
  @param get_lock        Should we acquire LOCK_error_log?
*/
bool open_error_log(const char *filename, bool get_lock);

/**
  Free any error log resources.

  @note This function accesses shared resources without protection, so
  it should only be called while the server is running single-threaded.

  @note The error log can still be used after this function is called,
  but that should only be done single-threaded. All buffered messages
  should be flushed before calling this function.
*/
void destroy_error_log();

/**
  Flush any pending data to disk and reopen the error log.
*/
bool reopen_error_log();

/**
  Discard all buffered messages and deallocate buffer without printing
  anything. Needed when terminating launching process after daemon
  has started. At this point we may have messages in the error log,
  but we don't want to show them to stderr (the daemon will output
  them in its error log).
 */
void discard_error_log_messages();

/**
  We buffer all error log messages that have been printed before the
  error log has been opened. This allows us to write them to the
  correct file once the error log has been opened.

  This function will explicitly flush buffered messages to stderr.
  It is only needed in cases where open_error_log() is not called
  as it otherwise will be done there.

  This function also turns buffering off (there is no way to turn
  buffering back on).
*/
void flush_error_log_messages();

/**
  Modular logger: log line and key/value manipulation helpers.
  Server-internal.  External services should access these via
  the log_builtins service API (cf. preamble for this file).
*/

/**
  Compare two NUL-terminated byte strings

  Note that when comparing without length limit, the long string
  is greater if they're equal up to the length of the shorter
  string, but the shorter string will be considered greater if
  its "value" up to that point is greater:

  compare 'abc','abcd':      -100  (longer wins if otherwise same)
  compare 'abca','abcd':       -3  (higher value wins)
  compare 'abcaaaaa','abcd':   -3  (higher value wins)

  @param  a                 the first string
  @param  b                 the second string
  @param  len               compare at most this many characters --
                            0 for no limit
  @param  case_insensitive  ignore upper/lower case in comparison

  @retval -1                a < b
  @retval  0                a == b
  @retval  1                a > b
*/
int log_string_compare(const char *a, const char *b, size_t len,
                       bool case_insensitive);

/**
  Predicate used to determine whether a type is generic
  (generic string, generic float, generic integer) rather
  than a well-known type.

  @param t          log item type to examine

  @retval  true     if generic type
  @retval  false    if wellknown type
*/
bool log_item_generic_type(log_item_type t);

/**
  Predicate used to determine whether a class is a string
  class (C-string or Lex-string).

  @param c          log item class to examine

  @retval   true    if of a string class
  @retval   false   if not of a string class
*/
bool log_item_string_class(log_item_class c);

/**
  Predicate used to determine whether a class is a numeric
  class (integer or float).

  @param c         log item class to examine

  @retval   true   if of a numeric class
  @retval   false  if not of a numeric class
*/
bool log_item_numeric_class(log_item_class c);

/**
  Get an integer value from a log-item of float or integer type.

  @param li      log item to get the value from
  @param i       longlong to store  the value in
*/
void log_item_get_int(log_item *li, longlong *i);

/**
  Get a float value from a log-item of float or integer type.

  @param li      log item to get the value from
  @param f       float to store  the value in
*/
void log_item_get_float(log_item *li, double *f);

/**
  Get a string value from a log-item of C-string or Lex string type.

  @param li      log item to get the value from
  @param str     char-pointer   to store the pointer to the value in
  @param len     size_t pointer to store the length of  the value in
*/
void log_item_get_string(log_item *li, char **str, size_t *len);

/**
  Set an integer value on a log_item.
  Fails gracefully if not log_item_data is supplied, so it can safely
  wrap log_line_item_set[_with_key]().

  @param  lid    log_item_data struct to set the value on
  @param  i      integer to set

  @retval true   lid was nullptr (possibly: OOM, could not set up log_item)
  @retval false  all's well
*/
bool log_item_set_int(log_item_data *lid, longlong i);

/**
  Set a floating point value on a log_item.
  Fails gracefully if not log_item_data is supplied, so it can safely
  wrap log_line_item_set[_with_key]().

  @param  lid    log_item_data struct to set the value on
  @param  f      float to set

  @retval true   lid was nullptr (possibly: OOM, could not set up log_item)
  @retval false  all's well
*/
bool log_item_set_float(log_item_data *lid, double f);

/**
  Set a string value on a log_item.
  Fails gracefully if not log_item_data is supplied, so it can safely
  wrap log_line_item_set[_with_key]().

  @param  lid    log_item_data struct to set the value on
  @param  s      pointer to string
  @param  s_len  length of string

  @retval true   lid was nullptr (possibly: OOM, could not set up log_item)
  @retval false  all's well
*/
bool log_item_set_lexstring(log_item_data *lid, const char *s, size_t s_len);

/**
  Set a string value on a log_item.
  Fails gracefully if not log_item_data is supplied, so it can safely
  wrap log_line_item_set[_with_key]().

  @param  lid    log_item_data struct to set the value on
  @param  s      pointer to NTBS

  @retval true   lid was nullptr (possibly: OOM, could not set up log_item)
  @retval false  all's well
*/
bool log_item_set_cstring(log_item_data *lid, const char *s);

/**
  See whether a string is a wellknown field name.

  @param key     potential key starts here
  @param len     length of the string to examine

  @retval        LOG_ITEM_TYPE_RESERVED:  reserved, but not "wellknown" key
  @retval        LOG_ITEM_TYPE_NOT_FOUND: key not found
  @retval        >0:                      index in array of wellknowns
*/
int log_item_wellknown_by_name(const char *key, size_t len);

/**
  See whether a type is wellknown.

  @param t       log item type to examine

  @retval        LOG_ITEM_TYPE_NOT_FOUND: key not found
  @retval        >0:                      index in array of wellknowns
*/
int log_item_wellknown_by_type(log_item_type t);

/**
  Accessor: from a record describing a wellknown key, get its name

  @param   idx  index in array of wellknowns, see log_item_wellknown_by_...()

  @retval       name (NTBS)
*/
const char *log_item_wellknown_get_name(uint idx);

/**
  Accessor: from a record describing a wellknown key, get its type

  @param idx     index in array of wellknowns, see log_item_wellknown_by_...()

  @retval        the log item type for the wellknown key
*/
log_item_type log_item_wellknown_get_type(uint idx);

/**
  Accessor: from a record describing a wellknown key, get its class

  @param idx     index in array of wellknowns, see log_item_wellknown_by_...()

  @retval        the log item class for the wellknown key
*/
log_item_class log_item_wellknown_get_class(uint idx);

/**
  Release any of key and value on a log-item that were dynamically allocated.

  @param  li  log-item to release the payload of
*/
void log_item_free(log_item *li);

/**
  Predicate indicating whether a log line is "willing" to accept any more
  key/value pairs.

  @param   ll     the log-line to examine

  @retval  false  if not full / if able to accept another log_item
  @retval  true   if full
*/
bool log_line_full(log_line *ll);

/**
  How many items are currently set on the given log_line?

  @param   ll     the log-line to examine

  @retval         the number of items set
*/
int log_line_item_count(log_line *ll);

/**
  Test whether a given type is presumed present on the log line.

  @param  ll  the log_line to examine
  @param  m   the log_type to test for

  @retval  0  not present
  @retval !=0 present
*/
log_item_type_mask log_line_item_types_seen(log_line *ll, log_item_type_mask m);

/**
  Initialize a log_line.

  @retval nullptr  could not set up buffer (too small?)
  @retval other    address of the newly initialized log_line
*/
log_line *log_line_init();

/**
  Release a log_line allocated with log_line_init.

  @retval nullptr  could not set up buffer (too small?)
  @retval other    address of the newly initialized log_line
*/
void log_line_exit(log_line *ll);

/**
  Release log line item (key/value pair) with the index elem in log line ll.
  This frees whichever of key and value were dynamically allocated.
  This leaves a "gap" in the bag that may immediately be overwritten
  with an updated element.  If the intention is to remove the item without
  replacing it, use log_line_item_remove() instead!

  @param         ll    log_line
  @param         elem  index of the key/value pair to release
*/
void log_line_item_free(log_line *ll, size_t elem);

/**
  Release all log line items (key/value pairs) in log line ll.
  This frees whichever keys and values were dynamically allocated.

  @param         ll    log_line
*/
void log_line_item_free_all(log_line *ll);

/**
  Release log line item (key/value pair) with the index elem in log line ll.
  This frees whichever of key and value were dynamically allocated.
  This moves any trailing items to fill the "gap" and decreases the counter
  of elements in the log line.  If the intention is to leave a "gap" in the
  bag that may immediately be overwritten with an updated element, use
  log_line_item_free() instead!

  @param         ll    log_line
  @param         elem  index of the key/value pair to release
*/
void log_line_item_remove(log_line *ll, int elem);

/**
  Find the (index of the) last key/value pair of the given name
  in the log line.

  @param         ll   log line
  @param         key  the key to look for

  @retval        -1:  none found
  @retval        -2:  invalid search-key given
  @retval        -3:  no log_line given
  @retval        >=0: index of the key/value pair in the log line
*/
int log_line_index_by_name(log_line *ll, const char *key);

/**
  Find the last item matching the given key in the log line.

  @param         ll   log line
  @param         key  the key to look for

  @retval        nullptr    item not found
  @retval        otherwise  pointer to the item (not a copy thereof!)
*/
log_item *log_line_item_by_name(log_line *ll, const char *key);

/**
  Find the (index of the) first key/value pair of the given type
  in the log line.

  @param         ll   log line
  @param         t    the log item type to look for

  @retval        <0:  none found
  @retval        >=0: index of the key/value pair in the log line
*/
int log_line_index_by_type(log_line *ll, log_item_type t);

/**
  Find the (index of the) first key/value pair of the given type
  in the log line. This variant accepts a reference item and looks
  for an item that is of the same type (for wellknown types), or
  one that is of a generic type, and with the same key name (for
  generic types).  For example, a reference item containing a
  generic string with key "foo" will a generic string, integer, or
  float with the key "foo".

  @param         ll   log line
  @param         ref  a reference item of the log item type to look for

  @retval        <0:  none found
  @retval        >=0: index of the key/value pair in the log line
*/
int log_line_index_by_item(log_line *ll, log_item *ref);

/**
  Initializes a log entry for use. This simply puts it in a defined
  state; if you wish to reset an existing item, see log_item_free().

  @param  li  the log-item to initialize
*/
void log_item_init(log_item *li);

/**
  Initializes an entry in a log line for use. This simply puts it in
  a defined state; if you wish to reset an existing item, see
  log_item_free().
  This resets the element beyond the last. The element count is not
  adjusted; this is for the caller to do once it sets up a valid
  element to suit its needs in the cleared slot. Finally, it is up
  to the caller to make sure that an element can be allocated.

  @param  ll  the log-line to initialize a log_item in

  @retval     the address of the cleared log_item
*/
log_item *log_line_item_init(log_line *ll);

/**
  Create new log item with key name "key", and allocation flags of
  "alloc" (see enum_log_item_free).
  Will return a pointer to the item's log_item_data struct for
  convenience.
  This is mostly interesting for filters and other services that create
  items that are not part of a log_line; sources etc. that intend to
  create an item for a log_line (the more common case) should usually
  use the below line_item_set_with_key() which creates an item (like
  this function does), but also correctly inserts it into a log_line.

  @param  li     the log_item to work on
  @param  t      the item-type
  @param  key    the key to set on the item.
                 ignored for non-generic types (may pass nullptr for those)
                 see alloc
  @param  alloc  LOG_ITEM_FREE_KEY  if key was allocated by caller
                 LOG_ITEM_FREE_NONE if key was not allocated
                 Allocated keys will automatically free()d when the
                 log_item is.
                 The log_item's alloc flags will be set to the
                 submitted value; specifically, any pre-existing
                 value will be clobbered.  It is therefore WRONG
                 a) to use this on a log_item that already has a key;
                    it should only be used on freshly init'd log_items;
                 b) to use this on a log_item that already has a
                    value (specifically, an allocated one); the correct
                    order is to init a log_item, then set up type and
                    key, and finally to set the value. If said value is
                    an allocated string, the log_item's alloc should be
                    bitwise or'd with LOG_ITEM_FREE_VALUE.

  @retval        a pointer to the log_item's log_data, for easy chaining:
                 log_item_set_with_key(...)->data_integer= 1;
*/
log_item_data *log_item_set_with_key(log_item *li, log_item_type t,
                                     const char *key, uint32 alloc);

/**
  Create new log item in log line "ll", with key name "key", and
  allocation flags of "alloc" (see enum_log_item_free).
  It is up to the caller to ensure the log_line can accept more items
  (e.g. by using log_line_full(ll)).
  On success, the number of registered items on the log line is increased,
  the item's type is added to the log_line's "seen" property,
  and a pointer to the item's log_item_data struct is returned for
  convenience.

  @param  ll     the log_line to work on
  @param  t      the item-type
  @param  key    the key to set on the item.
                 ignored for non-generic types (may pass nullptr for those)
                 see alloc
  @param  alloc  LOG_ITEM_FREE_KEY  if key was allocated by caller
                 LOG_ITEM_FREE_NONE if key was not allocated
                 Allocated keys will automatically free()d when the
                 log_item is.
                 The log_item's alloc flags will be set to the
                 submitted value; specifically, any pre-existing
                 value will be clobbered.  It is therefore WRONG
                 a) to use this on a log_item that already has a key;
                    it should only be used on freshly init'd log_items;
                 b) to use this on a log_item that already has a
                    value (specifically, an allocated one); the correct
                    order is to init a log_item, then set up type and
                    key, and finally to set the value. If said value is
                    an allocated string, the log_item's alloc should be
                    bitwise or'd with LOG_ITEM_FREE_VALUE.

  @retval        a pointer to the log_item's log_data, for easy chaining:
                 log_line_item_set_with_key(...)->data_integer= 1;
*/
log_item_data *log_line_item_set_with_key(log_line *ll, log_item_type t,
                                          const char *key, uint32 alloc);

/**
  As log_item_set_with_key(), except that the key is automatically
  derived from the wellknown log_item_type t.

  Create new log item with type "t".
  Will return a pointer to the item's log_item_data struct for
  convenience.
  This is mostly interesting for filters and other services that create
  items that are not part of a log_line; sources etc. that intend to
  create an item for a log_line (the more common case) should usually
  use the below line_item_set_with_key() which creates an item (like
  this function does), but also correctly inserts it into a log_line.

  The allocation of this item will be LOG_ITEM_FREE_NONE;
  specifically, any pre-existing value will be clobbered.
  It is therefore WRONG
  a) to use this on a log_item that already has a key;
     it should only be used on freshly init'd log_items;
  b) to use this on a log_item that already has a
     value (specifically, an allocated one); the correct
     order is to init a log_item, then set up type and
     key, and finally to set the value. If said value is
     an allocated string, the log_item's alloc should be
     bitwise or'd with LOG_ITEM_FREE_VALUE.

  @param  li     the log_item to work on
  @param  t      the item-type

  @retval        a pointer to the log_item's log_data, for easy chaining:
                 log_item_set_with_key(...)->data_integer= 1;
*/
log_item_data *log_item_set(log_item *li, log_item_type t);

/**
  Create a new log item of well-known type "t" in log line "ll".
  On success, the number of registered items on the log line is increased,
  the item's type is added to the log_line's "seen" property,
  and a pointer to the item's log_item_data struct is returned for
  convenience.

  It is up to the caller to ensure the log_line can accept more items
  (e.g. by using log_line_full(ll)).

  The allocation of this item will be LOG_ITEM_FREE_NONE;
  specifically, any pre-existing value will be clobbered.
  It is therefore WRONG
  a) to use this on a log_item that already has a key;
     it should only be used on freshly init'd log_items;
  b) to use this on a log_item that already has a
     value (specifically, an allocated one); the correct
     order is to init a log_item, then set up type and
     key, and finally to set the value. If said value is
     an allocated string, the log_item's alloc should be
     bitwise or'd with LOG_ITEM_FREE_VALUE.

  @param  ll     the log_line to work on
  @param  t      the item-type

  @retval        a pointer to the log_item's log_data, for easy chaining:
                 log_line_item_set_with_key(...)->data_integer= 1;
*/
log_item_data *log_line_item_set(log_line *ll, log_item_type t);

/**
  Convenience function: Derive a log label ("error", "warning",
  "information") from a severity.

  @param   prio       the severity/prio in question

  @return             a label corresponding to that priority.
  @retval  "ERROR"    for prio of ERROR_LEVEL or higher
  @retval  "Warning"  for prio of WARNING_LEVEL
  @retval  "Note"     otherwise
*/
const char *log_label_from_prio(int prio);

/**
  Complete, filter, and write submitted log items.

  This expects a log_line collection of log-related key/value pairs,
  e.g. from log_message().

  Where missing, timestamp, priority, thread-ID (if any) and so forth
  are added.

  Log item source services, log item filters, and log item sinks are
  then called.

  @param           ll                   key/value pairs describing info to log

  @retval          int                  number of fields in created log line
*/
int log_line_submit(log_line *ll);

/**
  Make and return an ISO 8601 / RFC 3339 compliant timestamp.
  Heeds log_timestamps.

  @param         buf         A buffer of at least 26 bytes to store
                             the timestamp in (19 + tzinfo tail + \0)
  @param         utime       Microseconds since the epoch
  @param         mode        if 0, use UTC; if 1, use local time

  @retval                    length of timestamp (excluding \0)
*/
int make_iso8601_timestamp(char *buf, ulonglong utime, int mode);

/**
  Set up custom error logging stack.

  @param        conf        The configuration string
  @param        check_only  If true, report on whether configuration is valid
                            (i.e. whether all requested services are available),
                            but do not apply the new configuration.
                            if false, set the configuration (acquire the
                            necessary services, update the hash by
                            adding/deleting entries as necessary)
  @param[out]   pos         If an error occurs and this pointer is non-null,
                            the position in the configuration string where
                            the error occurred will be written to the
                            pointed-to size_t.

  @retval              0    success
  @retval             -1    expected delimiter not found
  @retval             -2    one or more services not found
  @retval             -3    failed to create service cache entry
  @retval             -4    tried to open multiple instances of a singleton
  @retval             -5    failed to create service instance entry
  @retval             -6    last element in pipeline should be a sink
  @retval             -101  service name may not start with a delimiter
  @retval             -102  delimiters ',' and ';' may not be mixed
*/
int log_builtins_error_stack(const char *conf, bool check_only, size_t *pos);

/**
  Call flush() on all log_services.
  flush() function must not try to log anything, as we hold an
  exclusive lock on the stack.

  @retval   0   no problems
  @retval  -1   error
*/
int log_builtins_error_stack_flush();

/**
  Initialize the structured logging subsystem.

  Since we're initializing various locks here, we must call this late enough
  so this is clean, but early enough so it still happens while we're running
  single-threaded -- this specifically also means we must call it before we
  start plug-ins / storage engines / external components!

  @retval  0  no errors
  @retval -1  couldn't initialize stack lock
  @retval -2  couldn't initialize built-in default filter
  @retval -3  couldn't set up service hash
  @retval -4  couldn't initialize syseventlog lock
  @retval -5  couldn't set service pipeline
  @retval -6  couldn't initialize buffered logging lock
*/
int log_builtins_init();

/**
  De-initialize the structured logging subsystem.

  @retval  0  no errors
  @retval -1  not stopping, never started
*/
int log_builtins_exit();

/**
  Interim helper: write to the default error stream

  @param         buffer       buffer containing serialized error message
  @param         length       number of bytes in buffer
*/
void log_write_errstream(const char *buffer, size_t length);

#endif /* LOG_H */
