#ifndef MYSQL_SERVICE_COMMAND_INCLUDED
#define MYSQL_SERVICE_COMMAND_INCLUDED
/*  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file include/mysql/service_command.h
  Header file for the Command service. This service is to provide means
  of executing different commands, like COM_QUERY, COM_STMT_PREPARE,
  in the server.
*/

#include "mysql/com_data.h"
#include "mysql/service_srv_session.h"

#include "decimal.h"
#include "mysql_time.h"
#ifndef MYSQL_ABI_CHECK
#include <stdint.h> /* uint32_t */
#include "field_types.h"
#include "m_ctype.h"
#endif

/* POD structure for the field metadata from the server */
struct st_send_field {
  const char *db_name;
  const char *table_name;
  const char *org_table_name;
  const char *col_name;
  const char *org_col_name;
  unsigned long length;
  unsigned int charsetnr;
  unsigned int flags;
  unsigned int decimals;
  enum_field_types type;
};

/**
  Indicates beginning of metadata for the result set

  @param ctx      Plugin's context
  @param num_cols Number of fields being sent
  @param flags    Flags to alter the metadata sending
  @param resultcs Charset of the result set

  @note resultcs is the charset in which the data should be encoded before
  sent to the client. This is the value of the session variable
  character_set_results. The implementor most probably will need to save
  this value in the context and use it as "to" charset in get_string().

  In case of CS_BINARY_REPRESENTATION, get_string() receives as a parameter
  the charset of the string, as it is stored on disk.

  In case of CS_TEXT_REPRESENTATION, the string value might be already a
  stringified value or non-string data, which is in character_set_results.

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*start_result_metadata_t)(void *ctx, uint num_cols, uint flags,
                                       const CHARSET_INFO *resultcs);

/**
  Field metadata is provided via this callback

  @param ctx     Plugin's context
  @param field   Field's metadata (see field.h)
  @param charset Field's charset

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*field_metadata_t)(void *ctx, struct st_send_field *field,
                                const CHARSET_INFO *charset);

/**
  Indicates end of metadata for the result set

  @param ctx            Plugin's context
  @param server_status  Status of server (see mysql_com.h, SERVER_STATUS_*)
  @param warn_count     Number of warnings generated during execution to the
                          moment when the metadata is sent.
  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*end_result_metadata_t)(void *ctx, uint server_status,
                                     uint warn_count);

/**
  Indicates the beginning of a new row in the result set/metadata

  @param ctx   Plugin's context

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*start_row_t)(void *ctx);

/**
  Indicates the end of the current row in the result set/metadata

  @param ctx   Plugin's context

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*end_row_t)(void *ctx);

/**
  An error occurred during execution

  This callback indicates that an error occurred during command
  execution and the partial row should be dropped. Server will raise error
  and return.

  @param ctx   Plugin's context
*/
typedef void (*abort_row_t)(void *ctx);

/**
  Return client's capabilities (see mysql_com.h, CLIENT_*)

  @param ctx     Plugin's context

  @return Bitmap of client's capabilities
*/
typedef ulong (*get_client_capabilities_t)(void *ctx);

/**
  Receive NULL value from server

  @param ctx  Plugin's context

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_null_t)(void *ctx);

/**
  Receive TINY/SHORT/LONG value from server

  @param ctx           Plugin's context
  @param value         Value received

  @note In order to know which type exactly was received, the plugin must
  track the metadata that was sent just prior to the result set.

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_integer_t)(void *ctx, longlong value);

/**
  Get LONGLONG value from server

  @param ctx           Plugin's context
  @param value         Value received
  @param is_unsigned   TRUE <=> value is unsigned

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_longlong_t)(void *ctx, longlong value, uint is_unsigned);

/**
  Receive DECIMAL value from server

  @param ctx   Plugin's context
  @param value Value received

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_decimal_t)(void *ctx, const decimal_t *value);

/**
  Receive FLOAT/DOUBLE from server

  @param ctx      Plugin's context
  @param value    Value received
  @param decimals Number of decimals

  @note In order to know which type exactly was received, the plugin must
  track the metadata that was sent just prior to the result set.

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_double_t)(void *ctx, double value, uint32_t decimals);

/**
  Get DATE value from server

  @param ctx      Plugin's context
  @param value    Value received

  @returns
    1  an error occurred during storing, server will abort the command
    0  ok
*/
typedef int (*get_date_t)(void *ctx, const MYSQL_TIME *value);

/**
  Receive TIME value from server

  @param ctx      Plugin's context
  @param value    Value received
  @param decimals Number of decimals

  @returns
    1  an error occurred during storing, server will abort the command
    0  ok
*/
typedef int (*get_time_t)(void *ctx, const MYSQL_TIME *value, uint decimals);

/**
  Receive DATETIME value from server

  @param ctx      Plugin's context
  @param value    Value received
  @param decimals Number of decimals

  @returns
    1  an error occurred during storing, server will abort the command
    0  ok
*/
typedef int (*get_datetime_t)(void *ctx, const MYSQL_TIME *value,
                              uint decimals);

/**
  Get STRING value from server

  @param ctx     Plugin's context
  @param value   Data
  @param length  Data length
  @param valuecs Data charset

  @note In case of CS_BINARY_REPRESENTATION, get_string() receives as
  a parameter the charset of the string, as it is stored on disk.

  In case of CS_TEXT_REPRESENTATION, the string value might be already a
  stringified value or non-string data, which is in character_set_results.

  @see start_result_metadata()

  @returns
    1  an error occurred, server will abort the command
    0  ok
*/
typedef int (*get_string_t)(void *ctx, const char *value, size_t length,
                            const CHARSET_INFO *valuecs);

/**
  Command ended with success

  @param ctx                  Plugin's context
  @param server_status        Status of server (see mysql_com.h,
                              SERVER_STATUS_*)
  @param statement_warn_count Number of warnings thrown during execution
  @param affected_rows        Number of rows affected by the command
  @param last_insert_id       Last insert id being assigned during execution
  @param message              A message from server
*/
typedef void (*handle_ok_t)(void *ctx, uint server_status,
                            uint statement_warn_count, ulonglong affected_rows,
                            ulonglong last_insert_id, const char *message);

/**
  Command ended with ERROR

  @param ctx       Plugin's context
  @param sql_errno Error code
  @param err_msg   Error message
  @param sqlstate  SQL state corresponding to the error code
*/
typedef void (*handle_error_t)(void *ctx, uint sql_errno, const char *err_msg,
                               const char *sqlstate);

/**
  Callback for shutdown notification from the server.

  @param ctx              Plugin's context
  @param server_shutdown  Whether this is a normal connection shutdown (0) or
                          server shutdown (1).
*/
typedef void (*shutdown_t)(void *ctx, int server_shutdown);

struct st_command_service_cbs {
  /*
    For a statement that returns a result, the flow of called callbacks will be:

    start_result_metadata()
      field_metadata()
      ....
    end_result_metadata() (in the classic protocol this generates an EOF packet)
    start_row()
      get_xxx()
      ...
    end_row()
    start_row()
      get_xxx()
      ...
    end_row()
    handle_ok()           (with data for an EOF packet)

    For a statement that does NOT return a result, but only status, like
    INSERT, UPDATE, DELETE, REPLACE, TRUNCATE, CREATE, DROP, ALTER, etc. only
    handle_ok() will be invoked, in case of success.

    All statements that result in an error will invoke handle_error().

    For statements that return a result set, handle_error() might be invoked
    even after metadata was sent. This will indicate an error during the
    execution of the statement.
  */

  /* Getting metadata */

  start_result_metadata_t start_result_metadata;
  field_metadata_t field_metadata;
  end_result_metadata_t end_result_metadata;
  start_row_t start_row;
  end_row_t end_row;
  abort_row_t abort_row;
  get_client_capabilities_t get_client_capabilities;

  /* Getting data */

  get_null_t get_null;
  get_integer_t get_integer;
  get_longlong_t get_longlong;
  get_decimal_t get_decimal;
  get_double_t get_double;
  get_date_t get_date;
  get_time_t get_time;
  get_datetime_t get_datetime;
  get_string_t get_string;

  /* Getting execution status */

  handle_ok_t handle_ok;
  handle_error_t handle_error;
  shutdown_t shutdown;
};

enum cs_text_or_binary {
  CS_TEXT_REPRESENTATION = 1, /* Let the server convert everything to string */
  CS_BINARY_REPRESENTATION = 2, /* Let the server use native types */
};

extern "C" struct command_service_st {
  int (*run_command)(MYSQL_SESSION session, enum enum_server_command command,
                     const union COM_DATA *data, const CHARSET_INFO *client_cs,
                     const struct st_command_service_cbs *callbacks,
                     enum cs_text_or_binary text_or_binary,
                     void *service_callbacks_ctx);
} * command_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define command_service_run_command(t, command, data, cset, cbs, t_or_b, ctx) \
  command_service->run_command((t), (command), (data), (cset), (cbs),         \
                               (t_or_b), (ctx))
#else

/**
  Executes a server command in a session.


  There are two cases. Execution in a physical thread :
  1. initialized by the srv_session service
  2. NOT initialized by the srv_session service

  In case of 1, if there is currently attached session, and it is
  different from the passed one, the former will be automatically
  detached. The session to be used for the execution will then be
  attached. After the command is executed, the attached session will
  not be detached. It will be detached by a next call to run_command()
  with another session as parameter.  In other words, for all sessions
  used in a physical thread, there will be at most one in attached
  state.

  In case of 2, the current state (current_thd) will be
  preserved. Then the given session will move to attached state and
  the command will be executed. After the execution the state of the
  session will be changed to detached and the preserved state
  (current_thd) will be restored.

  The client charset is used for commands like COM_QUERY and
  COM_STMT_PREPARE to know how to threat the char* fields. This
  charset will be used until the next call of run_command when it may
  be changed again.

  @param session  The session
  @param command  The command to be executed.
  @param data     The data needed for the command to be executed
  @param client_cs The charset for the string data input(COM_QUERY for example)
  @param callbacks Callbacks to be used by the server to encode data and
                   to communicate with the client (plugin) side.
  @param text_or_binary Select which representation the server will use for the
                        data passed to the callbacks. For more information
                        @see cs_text_or_binary enum
  @param service_callbacks_ctx  Context passed to the command service callbacks

  @return
    0 success
    1 failure
*/
int command_service_run_command(MYSQL_SESSION session,
                                enum enum_server_command command,
                                const union COM_DATA *data,
                                const CHARSET_INFO *client_cs,
                                const struct st_command_service_cbs *callbacks,
                                enum cs_text_or_binary text_or_binary,
                                void *service_callbacks_ctx);

#endif /* MYSQL_DYNAMIC_PLUGIN */

#endif
