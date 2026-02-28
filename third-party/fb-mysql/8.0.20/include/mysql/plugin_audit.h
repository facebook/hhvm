/* Copyright (c) 2007, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_audit_h
#define _my_audit_h

/**
  @file include/mysql/plugin_audit.h
*/

#include "mysql/mysql_lex_string.h"
#include "plugin.h"
#ifndef MYSQL_PLUGIN_STRUCT_DEFS_ONLY
#ifndef MYSQL_ABI_CHECK
#include "m_string.h"
#endif
#else
struct CHARSET_INFO;
#ifndef __cplusplus
typedef struct CHARSET_INTO CHARSET_INFO;
#endif
#endif
#include "my_command.h"
#include "my_sqlcommand.h"
#include "plugin_audit_message_types.h"

#define MYSQL_AUDIT_INTERFACE_VERSION 0x0403

/**
 @enum mysql_event_class_t

 Audit event classes.
*/
typedef enum {
  MYSQL_AUDIT_GENERAL_CLASS = 0,
  MYSQL_AUDIT_CONNECTION_CLASS = 1,
  MYSQL_AUDIT_PARSE_CLASS = 2,
  MYSQL_AUDIT_AUTHORIZATION_CLASS = 3,
  MYSQL_AUDIT_TABLE_ACCESS_CLASS = 4,
  MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS = 5,
  MYSQL_AUDIT_SERVER_STARTUP_CLASS = 6,
  MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS = 7,
  MYSQL_AUDIT_COMMAND_CLASS = 8,
  MYSQL_AUDIT_QUERY_CLASS = 9,
  MYSQL_AUDIT_STORED_PROGRAM_CLASS = 10,
  MYSQL_AUDIT_AUTHENTICATION_CLASS = 11,
  MYSQL_AUDIT_MESSAGE_CLASS = 12,
  /* This item must be last in the list. */
  MYSQL_AUDIT_CLASS_MASK_SIZE
} mysql_event_class_t;

/**
  @struct st_mysql_audit

  The descriptor structure that is referred from st_mysql_plugin.
*/
struct st_mysql_audit {
  /**
    Interface version.
  */
  int interface_version;

  /**
    Event occurs when the event class consumer is to be
    disassociated from the specified THD.This would typically occur
    before some operation which may require sleeping - such as when
    waiting for the next query from the client.
  */
  void (*release_thd)(MYSQL_THD);

  /**
    Invoked whenever an event occurs which is of any
    class for which the plugin has interest.The second argument
    indicates the specific event class and the third argument is data
    as required for that class.
  */
  int (*event_notify)(MYSQL_THD, mysql_event_class_t, const void *);

  /**
    An array of bits used to indicate what event classes
    that this plugin wants to receive.
  */
  unsigned long class_mask[MYSQL_AUDIT_CLASS_MASK_SIZE];
};

/**
  @typedef enum_sql_command_t

  SQL command type definition.
*/
typedef enum enum_sql_command enum_sql_command_t;

/**
  @enum mysql_event_general_subclass_t

  Events for the MYSQL_AUDIT_GENERAL_CLASS event class.
*/
typedef enum {
  /** occurs before emitting to the general query log. */
  MYSQL_AUDIT_GENERAL_LOG = 1 << 0,
  /** occurs before transmitting errors to the user. */
  MYSQL_AUDIT_GENERAL_ERROR = 1 << 1,
  /** occurs after transmitting a resultset to the user. */
  MYSQL_AUDIT_GENERAL_RESULT = 1 << 2,
  /** occurs after transmitting a resultset or errors */
  MYSQL_AUDIT_GENERAL_STATUS = 1 << 3,
  /** occurs after instrumented warning is logged */
  MYSQL_AUDIT_GENERAL_WARNING_INSTR = 1 << 4,
  /** occurs after instrumented error is logged */
  MYSQL_AUDIT_GENERAL_ERROR_INSTR = 1 << 5
} mysql_event_general_subclass_t;

#define MYSQL_AUDIT_GENERAL_ALL                              \
  (MYSQL_AUDIT_GENERAL_LOG | MYSQL_AUDIT_GENERAL_ERROR |     \
   MYSQL_AUDIT_GENERAL_RESULT | MYSQL_AUDIT_GENERAL_STATUS | \
   MYSQL_AUDIT_GENERAL_WARNING_INSTR | MYSQL_AUDIT_GENERAL_ERROR_INSTR)

/**
  @struct mysql_event_general

  Structure for the MYSQL_AUDIT_GENERAL_CLASS event class.
*/
struct mysql_event_general {
  mysql_event_general_subclass_t event_subclass;
  int general_error_code;
  unsigned long general_thread_id;
  MYSQL_LEX_CSTRING general_user;
  MYSQL_LEX_CSTRING general_command;
  MYSQL_LEX_CSTRING general_query;
  CHARSET_INFO *general_charset;
  unsigned long long general_time;
  unsigned long long general_rows;
  MYSQL_LEX_CSTRING general_host;
  MYSQL_LEX_CSTRING general_sql_command;
  MYSQL_LEX_CSTRING general_external_user;
  MYSQL_LEX_CSTRING general_ip;
  /* Added in version 402 */
  long long query_id;
  MYSQL_LEX_CSTRING database;
  long long affected_rows;
  unsigned int port;
  /** Shard of the database if present in DB_METADATA */
  MYSQL_LEX_CSTRING shard;
};

/**
  @enum mysql_event_connection_subclass_t

  Events for MYSQL_AUDIT_CONNECTION_CLASS event class.
*/
typedef enum {
  /** occurs after authentication phase is completed. */
  MYSQL_AUDIT_CONNECTION_CONNECT = 1 << 0,
  /** occurs after connection is terminated. */
  MYSQL_AUDIT_CONNECTION_DISCONNECT = 1 << 1,
  /** occurs after COM_CHANGE_USER RPC is completed. */
  MYSQL_AUDIT_CONNECTION_CHANGE_USER = 1 << 2,
  /** occurs before authentication. */
  MYSQL_AUDIT_CONNECTION_PRE_AUTHENTICATE = 1 << 3
} mysql_event_connection_subclass_t;

#define MYSQL_AUDIT_CONNECTION_ALL                                      \
  (MYSQL_AUDIT_CONNECTION_CONNECT | MYSQL_AUDIT_CONNECTION_DISCONNECT | \
   MYSQL_AUDIT_CONNECTION_CHANGE_USER |                                 \
   MYSQL_AUDIT_CONNECTION_PRE_AUTHENTICATE)
/**
  @struct mysql_event_connection

  Structure for the MYSQL_AUDIT_CONNECTION_CLASS event class.
*/
struct mysql_event_connection {
  /** Event subclass. */
  mysql_event_connection_subclass_t event_subclass;
  /** Current status of the connection. */
  int status;
  /** Connection id. */
  unsigned long connection_id;
  /** User name of this connection. */
  MYSQL_LEX_CSTRING user;
  /** Priv user name. */
  MYSQL_LEX_CSTRING priv_user;
  /** External user name. */
  MYSQL_LEX_CSTRING external_user;
  /** Proxy user used for this connection. */
  MYSQL_LEX_CSTRING proxy_user;
  /** Connection host. */
  MYSQL_LEX_CSTRING host;
  /** IP of the connection. */
  MYSQL_LEX_CSTRING ip;
  /** Database name specified at connection time. */
  MYSQL_LEX_CSTRING database;
  /** Connection type:
        - 0 Undefined
        - 1 TCP/IP
        - 2 Socket
        - 3 Named pipe
        - 4 SSL
        - 5 Shared memory
  */
  int connection_type;
  MYSQL_LEX_CSTRING connection_certificate;
  unsigned int port;
  /** Shard of the database if present in DB_METADATA */
  MYSQL_LEX_CSTRING shard;
};

/**
@enum mysql_event_parse_subclass_t

Events for MYSQL_AUDIT_PARSE_CLASS event class.
*/
typedef enum {
  /** occurs before the query parsing. */
  MYSQL_AUDIT_PARSE_PREPARSE = 1 << 0,
  /** occurs after the query parsing. */
  MYSQL_AUDIT_PARSE_POSTPARSE = 1 << 1
} mysql_event_parse_subclass_t;

#define MYSQL_AUDIT_PARSE_ALL \
  (MYSQL_AUDIT_PARSE_PREPARSE | MYSQL_AUDIT_PARSE_POSTPARSE)

typedef enum {
  MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_NONE = 0,
  /// mysql_event_parse::flags Must be set by a plugin if the query is
  /// rewritten.
  MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_QUERY_REWRITTEN = 1 << 0,
  /// mysql_event_parse::flags Is set by the server if the query is prepared
  /// statement.
  MYSQL_AUDIT_PARSE_REWRITE_PLUGIN_IS_PREPARED_STATEMENT = 1 << 1
} mysql_event_parse_rewrite_plugin_flag;

/** Data for the MYSQL_AUDIT_PARSE events */
struct mysql_event_parse {
  /** MYSQL_AUDIT_[PRE|POST]_PARSE event id */
  mysql_event_parse_subclass_t event_subclass;

  /** one of FLAG_REWRITE_PLUGIN_* */
  mysql_event_parse_rewrite_plugin_flag *flags;

  /** input: the original query text */
  MYSQL_LEX_CSTRING query;

  /** output: returns the null-terminated rewriten query allocated by
   * my_malloc() */
  MYSQL_LEX_CSTRING *rewritten_query;
};

/**
  @enum mysql_event_authorization_subclass_t

  Events for MYSQL_AUDIT_AUTHORIZATION_CLASS event class.
*/
typedef enum {
  MYSQL_AUDIT_AUTHORIZATION_USER = 1 << 0,
  /** Occurs when database privilege is checked. */
  MYSQL_AUDIT_AUTHORIZATION_DB = 1 << 1,
  /** Occurs when table privilege is checked. */
  MYSQL_AUDIT_AUTHORIZATION_TABLE = 1 << 2,
  /** Occurs when column privilege is checked. */
  MYSQL_AUDIT_AUTHORIZATION_COLUMN = 1 << 3,
  /** Occurs when procedure privilege is checked. */
  MYSQL_AUDIT_AUTHORIZATION_PROCEDURE = 1 << 4,
  /** Occurs when proxy privilege is checked. */
  MYSQL_AUDIT_AUTHORIZATION_PROXY = 1 << 5
} mysql_event_authorization_subclass_t;

#define MYSQL_AUDIT_AUTHORIZATION_ALL                                   \
  (MYSQL_AUDIT_AUTHORIZATION_USER | MYSQL_AUDIT_AUTHORIZATION_DB |      \
   MYSQL_AUDIT_AUTHORIZATION_TABLE | MYSQL_AUDIT_AUTHORIZATION_COLUMN | \
   MYSQL_AUDIT_AUTHORIZATION_PROCEDURE | MYSQL_AUDIT_AUTHORIZATION_PROXY)
/**
  @struct mysql_event_authorization

  Structure for MYSQL_AUDIT_AUTHORIZATION_CLASS event class.
*/
struct mysql_event_authorization {
  /** Event subclass. */
  mysql_event_authorization_subclass_t event_subclass;
  /** Event status. */
  int status;
  /** Connection id. */
  unsigned int connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** SQL query text. */
  MYSQL_LEX_CSTRING query;
  /** SQL query charset. */
  const CHARSET_INFO *query_charset;
  /** Database name. */
  MYSQL_LEX_CSTRING database;
  /** Table name. */
  MYSQL_LEX_CSTRING table;
  /** Other name associated with the event. */
  MYSQL_LEX_CSTRING object;
  /** Requested authorization privileges. */
  unsigned long requested_privilege;
  /** Currently granted authorization privileges. */
  unsigned long granted_privilege;
};

/**
  Events for MYSQL_AUDIT_TABLE_ACCESS_CLASS event class.
*/
enum mysql_event_table_access_subclass_t {
  /** Occurs when table data are read. */
  MYSQL_AUDIT_TABLE_ACCESS_READ = 1 << 0,
  /** Occurs when table data are inserted. */
  MYSQL_AUDIT_TABLE_ACCESS_INSERT = 1 << 1,
  /** Occurs when table data are updated. */
  MYSQL_AUDIT_TABLE_ACCESS_UPDATE = 1 << 2,
  /** Occurs when table data are deleted. */
  MYSQL_AUDIT_TABLE_ACCESS_DELETE = 1 << 3
};

typedef enum mysql_event_table_access_subclass_t
    mysql_event_table_access_subclass_t;

#define MYSQL_AUDIT_TABLE_ACCESS_ALL                                 \
  (MYSQL_AUDIT_TABLE_ACCESS_READ | MYSQL_AUDIT_TABLE_ACCESS_INSERT | \
   MYSQL_AUDIT_TABLE_ACCESS_UPDATE | MYSQL_AUDIT_TABLE_ACCESS_DELETE)

/**
  @struct mysql_event_table_row_access

  Structure for MYSQL_AUDIT_TABLE_ACCES_CLASS event class.
*/
struct mysql_event_table_access {
  /** Event subclass. */
  mysql_event_table_access_subclass_t event_subclass;
  /** Connection id. */
  unsigned long connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** SQL query. */
  MYSQL_LEX_CSTRING query;
  /** SQL query charset. */
  const CHARSET_INFO *query_charset;
  /** Database name. */
  MYSQL_LEX_CSTRING table_database;
  /** Table name. */
  MYSQL_LEX_CSTRING table_name;
};

/**
  @enum mysql_event_global_variable_subclass_t

  Events for MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS event class.
*/
typedef enum {
  /** Occurs when global variable is retrieved. */
  MYSQL_AUDIT_GLOBAL_VARIABLE_GET = 1 << 0,
  /** Occurs when global variable is set. */
  MYSQL_AUDIT_GLOBAL_VARIABLE_SET = 1 << 1
} mysql_event_global_variable_subclass_t;

#define MYSQL_AUDIT_GLOBAL_VARIABLE_ALL \
  (MYSQL_AUDIT_GLOBAL_VARIABLE_GET | MYSQL_AUDIT_GLOBAL_VARIABLE_SET)

/** Events for MYSQL_AUDIT_GLOBAL_VARIABLE_CLASS event class. */
struct mysql_event_global_variable {
  /** Event subclass. */
  mysql_event_global_variable_subclass_t event_subclass;
  /** Connection id. */
  unsigned long connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** Variable name. */
  MYSQL_LEX_CSTRING variable_name;
  /** Variable value. */
  MYSQL_LEX_CSTRING variable_value;
};

/**
  @enum mysql_event_server_startup_subclass_t

  Events for MYSQL_AUDIT_SERVER_STARTUP_CLASS event class.
*/
typedef enum {
  /** Occurs after all subsystem are initialized during system start. */
  MYSQL_AUDIT_SERVER_STARTUP_STARTUP = 1 << 0
} mysql_event_server_startup_subclass_t;

#define MYSQL_AUDIT_SERVER_STARTUP_ALL (MYSQL_AUDIT_SERVER_STARTUP_STARTUP)

/**
  @struct mysql_event_server_startup

  Structure for MYSQL_AUDIT_SERVER_STARTUP_CLASS event class.
*/
struct mysql_event_server_startup {
  /** Event subclass. */
  mysql_event_server_startup_subclass_t event_subclass;
  /** Command line arguments. */
  const char **argv;
  /** Command line arguments count. */
  unsigned int argc;
};

/**
  @enum mysql_event_server_shutdown_subclass_t

  Events for MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS event class.
*/
typedef enum {
  /** Occurs when global variable is set. */
  MYSQL_AUDIT_SERVER_SHUTDOWN_SHUTDOWN = 1 << 0
} mysql_event_server_shutdown_subclass_t;

#define MYSQL_AUDIT_SERVER_SHUTDOWN_ALL (MYSQL_AUDIT_SERVER_SHUTDOWN_SHUTDOWN)

/**
  @enum mysql_server_shutdown_reason_t

  Server shutdown reason.
*/
typedef enum {
  /** User requested shut down. */
  MYSQL_AUDIT_SERVER_SHUTDOWN_REASON_SHUTDOWN,
  /** The server aborts. */
  MYSQL_AUDIT_SERVER_SHUTDOWN_REASON_ABORT
} mysql_server_shutdown_reason_t;

/**
  @struct mysql_event_server_shutdown

  Structure for MYSQL_AUDIT_SERVER_SHUTDOWN_CLASS event class.
*/
struct mysql_event_server_shutdown {
  /** Shutdown event. */
  mysql_event_server_shutdown_subclass_t event_subclass;
  /** Exit code associated with the shutdown event. */
  int exit_code;
  /** Shutdown reason. */
  mysql_server_shutdown_reason_t reason;
};

/**
  @enum mysql_event_command_subclass_t

  Events for MYSQL_AUDIT_COMMAND_CLASS event class.
*/
typedef enum {
  /** Command start event. */
  MYSQL_AUDIT_COMMAND_START = 1 << 0,
  /** Command end event. */
  MYSQL_AUDIT_COMMAND_END = 1 << 1
} mysql_event_command_subclass_t;

#define MYSQL_AUDIT_COMMAND_ALL \
  (MYSQL_AUDIT_COMMAND_START | MYSQL_AUDIT_COMMAND_END)
/**
  @typedef enum_server_command_t

  Server command type definition.
*/
typedef enum enum_server_command enum_server_command_t;

/**
  @struct mysql_event_command

  Event for MYSQL_AUDIT_COMMAND_CLASS event class.
  Events generated as a result of RPC command requests.
*/
struct mysql_event_command {
  /** Command event subclass. */
  mysql_event_command_subclass_t event_subclass;
  /** Command event status. */
  int status;
  /** Connection id. */
  unsigned long connection_id;
  /** Command id. */
  enum_server_command_t command_id;
};

/**
  @enum mysql_event_query_subclass_t

  Events for MYSQL_AUDIT_QUERY_CLASS event class.
*/
typedef enum {
  /** Query start event. */
  MYSQL_AUDIT_QUERY_START = 1 << 0,
  /** Nested query start event. */
  MYSQL_AUDIT_QUERY_NESTED_START = 1 << 1,
  /** Query post parse event. */
  MYSQL_AUDIT_QUERY_STATUS_END = 1 << 2,
  /** Nested query status end event. */
  MYSQL_AUDIT_QUERY_NESTED_STATUS_END = 1 << 3
} mysql_event_query_subclass_t;

#define MYSQL_AUDIT_QUERY_ALL                                 \
  (MYSQL_AUDIT_QUERY_START | MYSQL_AUDIT_QUERY_NESTED_START | \
   MYSQL_AUDIT_QUERY_STATUS_END | MYSQL_AUDIT_QUERY_NESTED_STATUS_END)
/**
  @struct mysql_event_command

  Event for MYSQL_AUDIT_COMMAND_CLASS event class.
*/
struct mysql_event_query {
  /** Event subclass. */
  mysql_event_query_subclass_t event_subclass;
  /** Event status. */
  int status;
  /** Connection id. */
  unsigned long connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** SQL query. */
  MYSQL_LEX_CSTRING query;
  /** SQL query charset. */
  const CHARSET_INFO *query_charset;
};

/**
  @enum mysql_event_stored_program_subclass_t

  Events for MYSQL_AUDIT_STORED_PROGRAM_CLASS event class.
*/
typedef enum {
  /** Stored program execution event. */
  MYSQL_AUDIT_STORED_PROGRAM_EXECUTE = 1 << 0
} mysql_event_stored_program_subclass_t;

#define MYSQL_AUDIT_STORED_PROGRAM_ALL (MYSQL_AUDIT_STORED_PROGRAM_EXECUTE)

/**
  @struct mysql_event_command

Event for MYSQL_AUDIT_COMMAND_CLASS event class.
*/
struct mysql_event_stored_program {
  /** Event subclass. */
  mysql_event_stored_program_subclass_t event_subclass;
  /** Connection id. */
  unsigned long connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** SQL query text. */
  MYSQL_LEX_CSTRING query;
  /** SQL query charset. */
  const CHARSET_INFO *query_charset;
  /** The Database the procedure is defined in. */
  MYSQL_LEX_CSTRING database;
  /** Name of the stored program. */
  MYSQL_LEX_CSTRING name;
  /** Stored program parameters. */
  void *parameters;
};

/**
  @enum mysql_event_authentication_subclass_t

  Events for MYSQL_AUDIT_AUTHENTICATION_CLASS event class.

  Event handler can not terminate an event unless stated
  explicitly.
*/
typedef enum {
  /** Generated after FLUSH PRIVILEGES */
  MYSQL_AUDIT_AUTHENTICATION_FLUSH = 1 << 0,
  /** Generated after CREATE USER | CREATE ROLE */
  MYSQL_AUDIT_AUTHENTICATION_AUTHID_CREATE = 1 << 1,
  /**
    Generated after credential change through:
    - SET PASSWORD
    - ALTER USER
    - GRANT
  */
  MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE = 1 << 2,
  /** Generated after RENAME USER */
  MYSQL_AUDIT_AUTHENTICATION_AUTHID_RENAME = 1 << 3,
  /** Generated after DROP USER */
  MYSQL_AUDIT_AUTHENTICATION_AUTHID_DROP = 1 << 4
} mysql_event_authentication_subclass_t;

#define MYSQL_AUDIT_AUTHENTICATION_ALL            \
  (MYSQL_AUDIT_AUTHENTICATION_FLUSH |             \
   MYSQL_AUDIT_AUTHENTICATION_AUTHID_CREATE |     \
   MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE | \
   MYSQL_AUDIT_AUTHENTICATION_AUTHID_RENAME |     \
   MYSQL_AUDIT_AUTHENTICATION_AUTHID_DROP)

/**
  @struct mysql_event_authentication

  Structure for MYSQL_AUDIT_AUTHENTICATION_CLASS event class.
*/
struct mysql_event_authentication {
  /** Event subclass. */
  mysql_event_authentication_subclass_t event_subclass;
  /** Event status */
  int status;
  /** Connection id. */
  unsigned int connection_id;
  /** SQL command id. */
  enum_sql_command_t sql_command_id;
  /** SQL query text. */
  MYSQL_LEX_CSTRING query;
  /** SQL query charset. */
  const CHARSET_INFO *query_charset;
  /** User name */
  MYSQL_LEX_CSTRING user;
  /** Host name */
  MYSQL_LEX_CSTRING host;
  /** Authentication plugin */
  MYSQL_LEX_CSTRING authentication_plugin;
  /** New user name */
  MYSQL_LEX_CSTRING new_user;
  /** New host name */
  MYSQL_LEX_CSTRING new_host;
  /** AuthorizationID type */
  bool is_role;
};

#define MYSQL_AUDIT_MESSAGE_ALL \
  (MYSQL_AUDIT_MESSAGE_INTERNAL | MYSQL_AUDIT_MESSAGE_USER)

/**
  @struct mysql_event_message

  Structure for MYSQL_AUDIT_MESSAGE_CLASS event class.
*/
struct mysql_event_message {
  /** Event subclass. */
  mysql_event_message_subclass_t event_subclass;
  /** Component. */
  MYSQL_LEX_CSTRING component;
  /** Producer */
  MYSQL_LEX_CSTRING producer;
  /** Message */
  MYSQL_LEX_CSTRING message;
  /** Key value map pointer. */
  mysql_event_message_key_value_t *key_value_map;
  /** Key value map length. */
  size_t key_value_map_length;
};

#endif
