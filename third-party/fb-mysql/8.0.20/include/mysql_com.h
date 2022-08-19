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

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/mysql_com.h
  Common definition between mysql server & client.
*/

#ifndef _mysql_com_h
#define _mysql_com_h

#ifndef MYSQL_ABI_CHECK
#include <stdbool.h>
#include <stdint.h>
#endif

#include "my_command.h"
#include "my_compress.h"

/*
  We need a definition for my_socket. On the client, <mysql.h> already provides
  it, but on the server side, we need to get it from a header.
*/
#ifndef my_socket_defined
#include "my_io.h"
#include "mysql/components/services/my_io_bits.h"
#endif

#ifndef MYSQL_ABI_CHECK
#include <stdbool.h>
#endif

#define SYSTEM_CHARSET_MBMAXLEN 3
#define FILENAME_CHARSET_MBMAXLEN 5
#define NAME_CHAR_LEN 64 /**< Field/table name length */
#define PARTITION_EXPR_CHAR_LEN                \
  2048 /**< Maximum expression length in chars \
        */
#define USERNAME_CHAR_LENGTH 80
#define USERNAME_CHAR_LENGTH_STR "80"
#ifndef NAME_LEN
#define NAME_LEN (NAME_CHAR_LEN * SYSTEM_CHARSET_MBMAXLEN)
#endif
#define USERNAME_LENGTH (USERNAME_CHAR_LENGTH * SYSTEM_CHARSET_MBMAXLEN)
#define CONNECT_STRING_MAXLEN 1024

#define MYSQL_AUTODETECT_CHARSET_NAME "auto"

#define SERVER_VERSION_LENGTH 60
#define SQLSTATE_LENGTH 5

/**
  Maximum length of comments

  pre 5.6: 60 characters
*/
#define TABLE_COMMENT_INLINE_MAXLEN 180
#define TABLE_COMMENT_MAXLEN 2048
#define COLUMN_COMMENT_MAXLEN 1024
#define INDEX_COMMENT_MAXLEN 1024
#define TABLE_PARTITION_COMMENT_MAXLEN 1024
#define TABLESPACE_COMMENT_MAXLEN 2048

/**
  Maximum length of protocol packet.
  @ref page_protocol_basic_ok_packet length limit also restricted to this value
  as any length greater than this value will have first byte of
  @ref page_protocol_basic_ok_packet to be 254 thus does not
  provide a means to identify if this is @ref page_protocol_basic_ok_packet or
  @ref page_protocol_basic_eof_packet.
*/
#define MAX_PACKET_LENGTH (256L * 256L * 256L - 1)

#define LOCAL_HOST "localhost"
#define LOCAL_HOST_NAMEDPIPE "."

#if defined(_WIN32)
#define MYSQL_NAMEDPIPE "MySQL"
#define MYSQL_SERVICENAME "MySQL"
#endif /* _WIN32 */

/** The length of the header part for each generated column in the .frm file.*/
#define FRM_GCOL_HEADER_SIZE 4
/**
  Maximum length of the expression statement defined for generated columns.
*/
#define GENERATED_COLUMN_EXPRESSION_MAXLEN 65535 - FRM_GCOL_HEADER_SIZE
/**
  Length of random string sent by server on handshake; this is also length of
  obfuscated password, received from client
*/
#define SCRAMBLE_LENGTH 20
#define AUTH_PLUGIN_DATA_PART_1_LENGTH 8
/** length of password stored in the db: new passwords are preceeded with '*'*/
#define SCRAMBLED_PASSWORD_CHAR_LENGTH (SCRAMBLE_LENGTH * 2 + 1)

/**
   @defgroup group_cs_column_definition_flags Column Definition Flags
   @ingroup group_cs

   @brief Values for the flags bitmask used by ::Send_field:flags

   Currently need to fit into 32 bits.

   Each bit represents an optional feature of the protocol.

   Both the client and the server are sending these.

   The intersection of the two determines what optional parts of the
   protocol will be used.
*/

/**
  @addtogroup group_cs_column_definition_flags
  @{
*/

#define NOT_NULL_FLAG 1     /**< Field can't be NULL */
#define PRI_KEY_FLAG 2      /**< Field is part of a primary key */
#define UNIQUE_KEY_FLAG 4   /**< Field is part of a unique key */
#define MULTIPLE_KEY_FLAG 8 /**< Field is part of a key */
#define BLOB_FLAG 16        /**< Field is a blob */
#define UNSIGNED_FLAG 32    /**< Field is unsigned */
#define ZEROFILL_FLAG 64    /**< Field is zerofill */
#define BINARY_FLAG 128     /**< Field is binary   */

/* The following are only sent to new clients */
#define ENUM_FLAG 256              /**< field is an enum */
#define AUTO_INCREMENT_FLAG 512    /**< field is a autoincrement field */
#define TIMESTAMP_FLAG 1024        /**< Field is a timestamp */
#define SET_FLAG 2048              /**< field is a set */
#define NO_DEFAULT_VALUE_FLAG 4096 /**< Field doesn't have default value */
#define ON_UPDATE_NOW_FLAG 8192    /**< Field is set to NOW on UPDATE */
#define NUM_FLAG 32768             /**< Field is num (for clients) */
#define PART_KEY_FLAG 16384        /**< Intern; Part of some key */
#define GROUP_FLAG 32768           /**< Intern: Group field */
#define UNIQUE_FLAG 65536          /**< Intern: Used by sql_yacc */
#define BINCMP_FLAG 131072         /**< Intern: Used by sql_yacc */
#define GET_FIXED_FIELDS_FLAG                                                  \
  (1 << 18)                               /**< Used to get fields in item tree \
                                           */
#define FIELD_IN_PART_FUNC_FLAG (1 << 19) /**< Field part of partition func */
/**
  Intern: Field in TABLE object for new version of altered table,
          which participates in a newly added index.
*/
#define FIELD_IN_ADD_INDEX (1 << 20)
#define FIELD_IS_RENAMED (1 << 21)   /**< Intern: Field is being renamed */
#define FIELD_FLAGS_STORAGE_MEDIA 22 /**< Field storage media, bit 22-23 */
#define FIELD_FLAGS_STORAGE_MEDIA_MASK (3 << FIELD_FLAGS_STORAGE_MEDIA)
#define FIELD_FLAGS_COLUMN_FORMAT 24 /**< Field column format, bit 24-25 */
#define FIELD_FLAGS_COLUMN_FORMAT_MASK (3 << FIELD_FLAGS_COLUMN_FORMAT)
#define FIELD_IS_DROPPED (1 << 26) /**< Intern: Field is being dropped */
#define EXPLICIT_NULL_FLAG                        \
  (1 << 27) /**< Field is explicitly specified as \
               NULL by the user */
#define FIELD_IS_MARKED                   \
  (1 << 28) /**< Intern: field is marked, \
                 general purpose */

/** Field will not be loaded in secondary engine. */
#define NOT_SECONDARY_FLAG (1 << 29)

/** @}*/

/**
   @defgroup group_cs_com_refresh_flags COM_REFRESH Flags
   @ingroup group_cs

   @brief Values for the `sub_command` in ::COM_REFRESH

   Currently the protocol carries only 8 bits of these flags.

   The rest (8-end) are used only internally in the server.
*/

/**
  @addtogroup group_cs_com_refresh_flags
  @{
*/

#define REFRESH_GRANT 1    /**< Refresh grant tables, FLUSH PRIVILEGES */
#define REFRESH_LOG 2      /**< Start on new log file, FLUSH LOGS */
#define REFRESH_TABLES 4   /**< close all tables, FLUSH TABLES */
#define REFRESH_HOSTS 8    /**< Flush host cache, FLUSH HOSTS */
#define REFRESH_STATUS 16  /**< Flush status variables, FLUSH STATUS */
#define REFRESH_THREADS 32 /**< Flush thread cache */
#define REFRESH_SLAVE                         \
  64 /**< Reset master info and restart slave \
        thread, RESET SLAVE */
#define REFRESH_MASTER                                                 \
  128                            /**< Remove all bin logs in the index \
                                    and truncate the index, RESET MASTER */
#define REFRESH_ERROR_LOG 256    /**< Rotate only the erorr log */
#define REFRESH_ENGINE_LOG 512   /**< Flush all storage engine logs */
#define REFRESH_BINARY_LOG 1024  /**< Flush the binary log */
#define REFRESH_RELAY_LOG 2048   /**< Flush the relay log */
#define REFRESH_GENERAL_LOG 4096 /**< Flush the general log */
#define REFRESH_SLOW_LOG 8192    /**< Flush the slow query log */
#define REFRESH_READ_LOCK 16384  /**< Lock tables for read. */
/**
  Wait for an impending flush before closing the tables.

  @sa REFRESH_READ_LOCK, handle_reload_request, close_cached_tables
*/
#define REFRESH_FAST 32768
#define REFRESH_USER_RESOURCES 0x80000L   /** FLISH RESOUCES. @sa ::reset_mqh */
#define REFRESH_FOR_EXPORT 0x100000L      /** FLUSH TABLES ... FOR EXPORT */
#define REFRESH_OPTIMIZER_COSTS 0x200000L /** FLUSH OPTIMIZER_COSTS */
#define REFRESH_PERSIST 0x400000L         /** RESET PERSIST */

/** @}*/

/**
   @defgroup group_cs_capabilities_flags Capabilities Flags
   @ingroup group_cs

   @brief Values for the capabilities flag bitmask used by the MySQL protocol

   Currently need to fit into 32 bits.

   Each bit represents an optional feature of the protocol.

   Both the client and the server are sending these.

   The intersection of the two determines whast optional parts of the
   protocol will be used.
*/

/**
  @addtogroup group_cs_capabilities_flags
  @{
*/

/**
  Use the improved version of Old Password Authentication.

  Not used.

  @note Assumed to be set since 4.1.1.
*/
#define CLIENT_LONG_PASSWORD 1
/**
  Send found rows instead of affected rows in @ref
  page_protocol_basic_eof_packet
*/
#define CLIENT_FOUND_ROWS 2
/**
  @brief Get all column flags

  Longer flags in Protocol::ColumnDefinition320.

  @todo Reference Protocol::ColumnDefinition320

  Server
  ------

  Supports longer flags.

  Client
  ------

  Expects longer flags.
*/
#define CLIENT_LONG_FLAG 4
/**
  Database (schema) name can be specified on connect in Handshake Response
  Packet.

  @todo Reference Handshake Response Packet.

  Server
  ------

  Supports schema-name in Handshake Response Packet.

  Client
  ------

  Handshake Response Packet contains a schema-name.

  @sa send_client_reply_packet()
*/
#define CLIENT_CONNECT_WITH_DB 8
#define CLIENT_NO_SCHEMA 16 /**< Don't allow database.table.column */
/**
  Compression protocol supported.

  @todo Reference Compression

  Server
  ------

  Supports compression.

  Client
  ------

  Switches to Compression compressed protocol after successful authentication.
*/
#define CLIENT_COMPRESS 32
/**
  Special handling of ODBC behavior.

  @note No special behavior since 3.22.
*/
#define CLIENT_ODBC 64
/**
  Can use LOAD DATA LOCAL.

  Server
  ------

  Enables the LOCAL INFILE request of LOAD DATA|XML.

  Client
  ------

  Will handle LOCAL INFILE request.
*/
#define CLIENT_LOCAL_FILES 128
/**
  Ignore spaces before '('

  Server
  ------

  Parser can ignore spaces before '('.

  Client
  ------

  Let the parser ignore spaces before '('.
*/
#define CLIENT_IGNORE_SPACE 256
/**
  New 4.1 protocol

  @todo Reference the new 4.1 protocol

  Server
  ------

  Supports the 4.1 protocol.

  Client
  ------

  Uses the 4.1 protocol.

  @note this value was CLIENT_CHANGE_USER in 3.22, unused in 4.0
*/
#define CLIENT_PROTOCOL_41 512
/**
  This is an interactive client

  Use @ref System_variables::net_wait_timeout
  versus @ref System_variables::net_interactive_timeout.

  Server
  ------

  Supports interactive and noninteractive clients.

  Client
  ------

  Client is interactive.

  @sa mysql_real_connect()
*/
#define CLIENT_INTERACTIVE 1024
/**
  Use SSL encryption for the session

  @todo Reference SSL

  Server
  ------

  Supports SSL

  Client
  ------

  Switch to SSL after sending the capability-flags.
*/
#define CLIENT_SSL 2048
/**
  Client only flag. Not used.

  Client
  ------

  Do not issue SIGPIPE if network failures occur (libmysqlclient only).

  @sa mysql_real_connect()
*/
#define CLIENT_IGNORE_SIGPIPE 4096
/**
  Client knows about transactions

  Server
  ------

  Can send status flags in @ref page_protocol_basic_ok_packet /
  @ref page_protocol_basic_eof_packet.

  Client
  ------

  Expects status flags in @ref page_protocol_basic_ok_packet /
  @ref page_protocol_basic_eof_packet.

  @note This flag is optional in 3.23, but always set by the server since 4.0.
  @sa send_server_handshake_packet(), parse_client_handshake_packet(),
  net_send_ok(), net_send_eof()
*/
#define CLIENT_TRANSACTIONS 8192
#define CLIENT_RESERVED2                                 \
  32768 /**< DEPRECATED: Old flag for 4.1 authentication \
           CLIENT_SECURE_CONNECTION */
/**
  Enable/disable multi-stmt support

  Also sets @ref CLIENT_MULTI_RESULTS. Currently not checked anywhere.

  Server
  ------

  Can handle multiple statements per COM_QUERY and COM_STMT_PREPARE.

  Client
  -------

  May send multiple statements per COM_QUERY and COM_STMT_PREPARE.

  @note Was named ::CLIENT_MULTI_QUERIES in 4.1.0, renamed later.

  Requires
  --------

  ::CLIENT_PROTOCOL_41

  @todo Reference COM_QUERY and COM_STMT_PREPARE
*/
#define CLIENT_MULTI_STATEMENTS (1UL << 16)
/**
  Enable/disable multi-results

  Server
  ------

  Can send multiple resultsets for COM_QUERY.
  Error if the server needs to send them and client
  does not support them.

  Client
  -------

  Can handle multiple resultsets for COM_QUERY.

  Requires
  --------

  ::CLIENT_PROTOCOL_41

  @sa mysql_execute_command(), sp_head::MULTI_RESULTS
*/
#define CLIENT_MULTI_RESULTS (1UL << 17)
/**
  Multi-results and OUT parameters in PS-protocol.

  Server
  ------

  Can send multiple resultsets for COM_STMT_EXECUTE.

  Client
  ------

  Can handle multiple resultsets for COM_STMT_EXECUTE.

  Requires
  --------

  ::CLIENT_PROTOCOL_41

  @todo Reference COM_STMT_EXECUTE and PS-protocol

  @sa Protocol_binary::send_out_parameters
*/
#define CLIENT_PS_MULTI_RESULTS (1UL << 18)

/**
  Client supports plugin authentication

  Server
  ------

  Sends extra data in Initial Handshake Packet and supports the pluggable
  authentication protocol.

  Client
  ------

  Supports authentication plugins.

  Requires
  --------

  ::CLIENT_PROTOCOL_41

  @todo Reference plugin authentication, Initial Handshake Packet,
  Authentication plugins

  @sa send_change_user_packet(), send_client_reply_packet(), run_plugin_auth(),
  parse_com_change_user_packet(), parse_client_handshake_packet()
*/
#define CLIENT_PLUGIN_AUTH (1UL << 19)
/**
  Client supports connection attributes

  Server
  ------

  Permits connection attributes in Protocol::HandshakeResponse41.

  Client
  ------

  Sends connection attributes in Protocol::HandshakeResponse41.

  @todo Reference Protocol::HandshakeResponse41

  @sa send_client_connect_attrs(), read_client_connect_attrs()
*/
#define CLIENT_CONNECT_ATTRS (1UL << 20)

/**
  Enable authentication response packet to be larger than 255 bytes.

  When the ability to change default plugin require that the initial password
  field in the Protocol::HandshakeResponse41 paclet can be of arbitrary size.
  However, the 4.1 client-server protocol limits the length of the
  auth-data-field sent from client to server to 255 bytes.
  The solution is to change the type of the field to a true length encoded
  string and indicate the protocol change
  with this client capability flag.

  Server
  ------

  Understands length-encoded integer for auth response data in
  Protocol::HandshakeResponse41.

  Client
  ------

  Length of auth response data in Protocol::HandshakeResponse41
  is a length-encoded integer.

  @todo Reference Protocol::HandshakeResponse41

  @note The flag was introduced in 5.6.6, but had the wrong value.

  @sa send_client_reply_packet(), parse_client_handshake_packet(),
  get_56_lenc_string(), get_41_lenc_string()
*/
#define CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA (1UL << 21)

/**
  Don't close the connection for a user account with expired password.

  Server
  ------

  Announces support for expired password extension.

  Client
  ------

  Can handle expired passwords.

  @todo Reference expired password

  @sa MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, disconnect_on_expired_password
  ACL_USER::password_expired, check_password_lifetime(), acl_authenticate()
*/
#define CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS (1UL << 22)

/**
  Capable of handling server state change information. Its a hint to the
  server to include the state change information in
  @ref page_protocol_basic_ok_packet.

  Server
  ------
  Can set ::SERVER_SESSION_STATE_CHANGED in the ::SERVER_STATUS_flags_enum
  and send @ref sect_protocol_basic_ok_packet_sessinfo in a
  @ref page_protocol_basic_ok_packet.

  Client
  ------

  Expects the server to send @ref sect_protocol_basic_ok_packet_sessinfo in
  a @ref page_protocol_basic_ok_packet.

  @sa enum_session_state_type, read_ok_ex(), net_send_ok(), Session_tracker,
  State_tracker
*/
#define CLIENT_SESSION_TRACK (1UL << 23)
/**
  Client no longer needs @ref page_protocol_basic_eof_packet and will
  use @ref page_protocol_basic_ok_packet instead.
  @sa net_send_ok()

  Server
  ------

  Can send OK after a Text Resultset.

  Client
  ------

  Expects an @ref page_protocol_basic_ok_packet (instead of
  @ref page_protocol_basic_eof_packet) after the resultset rows of a
  Text Resultset.

  Background
  ----------

  To support ::CLIENT_SESSION_TRACK, additional information must be sent after
  all successful commands. Although the @ref page_protocol_basic_ok_packet is
  extensible, the @ref page_protocol_basic_eof_packet is not due to the overlap
  of its bytes with the content of the Text Resultset Row.

  Therefore, the @ref page_protocol_basic_eof_packet in the
  Text Resultset is replaced with an @ref page_protocol_basic_ok_packet.
  @ref page_protocol_basic_eof_packet is deprecated as of MySQL 5.7.5.

  @todo Reference Text Resultset

  @sa cli_safe_read_with_ok(), read_ok_ex(), net_send_ok(), net_send_eof()
*/
#define CLIENT_DEPRECATE_EOF (1UL << 24)

/**
  Verify server certificate.

  Client only flag.

  @deprecated in favor of --ssl-mode.
*/
#define CLIENT_SSL_VERIFY_SERVER_CERT (1UL << 30)

/**
  The client can handle optional metadata information in the resultset.
*/
#define CLIENT_OPTIONAL_RESULTSET_METADATA (1UL << 25)

/**
  Compression protocol extended to support zstd compression method

  This capability flag is used to send zstd compression level between
  client and server provided both client and server are enabled with
  this flag.

  Server
  ------
  Server sets this flag when global variable protocol-compression-algorithms
  has zstd in its list of supported values.

  Client
  ------
  Client sets this flag when it is configured to use zstd compression method.

*/
#define CLIENT_ZSTD_COMPRESSION_ALGORITHM (1UL << 26)
#define CLIENT_ZSTD_STREAM_COMPRESSION_ALGORITHM 16384
/** former CLIENT_RESERVED   */
#define CLIENT_LZ4F_STREAM_COMPRESSION_ALGORITHM (1UL << 28)

/**
  This flag will be reserved to extend the 32bit capabilities structure to
  64bits.
*/
#define CLIENT_CAPABILITY_EXTENSION (1UL << 29)
/**
  Don't reset the options after an unsuccessful connect

  Client only flag.

  Typically passed via ::mysql_real_connect() 's client_flag parameter.

  @sa mysql_real_connect()
*/
#define CLIENT_REMEMBER_OPTIONS (1UL << 31)
/** @}*/

/** a compatibility alias for CLIENT_COMPRESS */
#define CAN_CLIENT_COMPRESS CLIENT_COMPRESS

/** Gather all possible capabilites (flags) supported by the server */
#define CLIENT_ALL_FLAGS                                                       \
  (CLIENT_LONG_PASSWORD | CLIENT_FOUND_ROWS | CLIENT_LONG_FLAG |               \
   CLIENT_CONNECT_WITH_DB | CLIENT_NO_SCHEMA | CLIENT_COMPRESS | CLIENT_ODBC | \
   CLIENT_LOCAL_FILES | CLIENT_IGNORE_SPACE | CLIENT_PROTOCOL_41 |             \
   CLIENT_INTERACTIVE | CLIENT_SSL | CLIENT_IGNORE_SIGPIPE |                   \
   CLIENT_TRANSACTIONS | CLIENT_RESERVED2 | CLIENT_MULTI_STATEMENTS |          \
   CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS |                            \
   CLIENT_SSL_VERIFY_SERVER_CERT | CLIENT_REMEMBER_OPTIONS |                   \
   CLIENT_PLUGIN_AUTH | CLIENT_CONNECT_ATTRS |                                 \
   CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA |                                     \
   CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS | CLIENT_SESSION_TRACK |                \
   CLIENT_DEPRECATE_EOF | CLIENT_OPTIONAL_RESULTSET_METADATA |                 \
   CLIENT_ZSTD_COMPRESSION_ALGORITHM |                                         \
   CLIENT_ZSTD_STREAM_COMPRESSION_ALGORITHM |                                  \
   CLIENT_LZ4F_STREAM_COMPRESSION_ALGORITHM)

/**
  Switch off from ::CLIENT_ALL_FLAGS the flags that are optional and
  depending on build flags.
  If any of the optional flags is supported by the build it will be switched
  on before sending to the client during the connection handshake.
*/
#define CLIENT_BASIC_FLAGS                                          \
  (CLIENT_ALL_FLAGS &                                               \
   ~(CLIENT_SSL | CLIENT_COMPRESS | CLIENT_SSL_VERIFY_SERVER_CERT | \
     CLIENT_ZSTD_COMPRESSION_ALGORITHM |                            \
     CLIENT_ZSTD_STREAM_COMPRESSION_ALGORITHM |                     \
     CLIENT_LZ4F_STREAM_COMPRESSION_ALGORITHM))

/** The status flags are a bit-field */
enum SERVER_STATUS_flags_enum {
  /**
    Is raised when a multi-statement transaction
    has been started, either explicitly, by means
    of BEGIN or COMMIT AND CHAIN, or
    implicitly, by the first transactional
    statement, when autocommit=off.
  */
  SERVER_STATUS_IN_TRANS = 1,
  SERVER_STATUS_AUTOCOMMIT = 2,   /**< Server in auto_commit mode */
  SERVER_MORE_RESULTS_EXISTS = 8, /**< Multi query - next query exists */
  SERVER_QUERY_NO_GOOD_INDEX_USED = 16,
  SERVER_QUERY_NO_INDEX_USED = 32,
  /**
    The server was able to fulfill the clients request and opened a
    read-only non-scrollable cursor for a query. This flag comes
    in reply to COM_STMT_EXECUTE and COM_STMT_FETCH commands.
    Used by Binary Protocol Resultset to signal that COM_STMT_FETCH
    must be used to fetch the row-data.
    @todo Refify "Binary Protocol Resultset" and "COM_STMT_FETCH".
  */
  SERVER_STATUS_CURSOR_EXISTS = 64,
  /**
    This flag is sent when a read-only cursor is exhausted, in reply to
    COM_STMT_FETCH command.
  */
  SERVER_STATUS_LAST_ROW_SENT = 128,
  SERVER_STATUS_DB_DROPPED = 256, /**< A database was dropped */
  SERVER_STATUS_NO_BACKSLASH_ESCAPES = 512,
  /**
    Sent to the client if after a prepared statement reprepare
    we discovered that the new statement returns a different
    number of result set columns.
  */
  SERVER_STATUS_METADATA_CHANGED = 1024,
  SERVER_QUERY_WAS_SLOW = 2048,
  /**
    To mark ResultSet containing output parameter values.
  */
  SERVER_PS_OUT_PARAMS = 4096,

  /**
    Set at the same time as SERVER_STATUS_IN_TRANS if the started
    multi-statement transaction is a read-only transaction. Cleared
    when the transaction commits or aborts. Since this flag is sent
    to clients in OK and EOF packets, the flag indicates the
    transaction status at the end of command execution.
  */
  SERVER_STATUS_IN_TRANS_READONLY = 8192,

  /**
    This status flag, when on, implies that one of the state information has
    changed on the server because of the execution of the last statement.
  */
  SERVER_SESSION_STATE_CHANGED = (1UL << 14)
};

/**
  Server status flags that must be cleared when starting
  execution of a new SQL statement.
  Flags from this set are only added to the
  current server status by the execution engine, but
  never removed -- the execution engine expects them
  to disappear automagically by the next command.
*/
#define SERVER_STATUS_CLEAR_SET                                   \
  (SERVER_QUERY_NO_GOOD_INDEX_USED | SERVER_QUERY_NO_INDEX_USED | \
   SERVER_MORE_RESULTS_EXISTS | SERVER_STATUS_METADATA_CHANGED |  \
   SERVER_QUERY_WAS_SLOW | SERVER_STATUS_DB_DROPPED |             \
   SERVER_STATUS_CURSOR_EXISTS | SERVER_STATUS_LAST_ROW_SENT |    \
   SERVER_SESSION_STATE_CHANGED)

/** Max length of a error message. Should be kept in sync with ::ERRMSGSIZE. */
#define MYSQL_ERRMSG_SIZE 512
#define NET_READ_TIMEOUT 30          /**< Timeout on read */
#define NET_WRITE_TIMEOUT 60         /**< Timeout on write */
#define NET_WAIT_TIMEOUT 8 * 60 * 60 /**< Wait for new query */

/**
  Flag used by the parser. Kill only the query and not the connection.

  @sa SQLCOM_KILL, sql_kill(), LEX::type
*/
#define ONLY_KILL_QUERY 1

#ifndef MYSQL_VIO
struct Vio;
#define MYSQL_VIO struct Vio *
#endif

#define MAX_TINYINT_WIDTH 3     /**< Max width for a TINY w.o. sign */
#define MAX_SMALLINT_WIDTH 5    /**< Max width for a SHORT w.o. sign */
#define MAX_MEDIUMINT_WIDTH 8   /**< Max width for a INT24 w.o. sign */
#define MAX_INT_WIDTH 10        /**< Max width for a LONG w.o. sign */
#define MAX_BIGINT_WIDTH 20     /**< Max width for a LONGLONG */
#define MAX_CHAR_WIDTH 255      /**< Max length for a CHAR colum */
#define MAX_BLOB_WIDTH 16777216 /**< Default width for blob */

/*
  In order to avoid confusion about whether a timeout value is in
  seconds or milliseconds, a timeout_t struct is used.  It simply tracks
  milliseconds but this helps ensure type safety and clear intention
  when converting for use in syscalls etc.
*/

typedef struct {
  uint value_ms_;
} timeout_t;

typedef struct NET {
  MYSQL_VIO vio;
  unsigned char *buff, *buff_end, *write_pos, *read_pos;
  my_socket fd; /* For Perl DBI/dbd */
  /**
    Set if we are doing several queries in one
    command ( as in LOAD TABLE ... FROM MASTER ),
    and do not want to confuse the client with OK at the wrong time
  */
  unsigned long remain_in_buf, length, buf_length, where_b;
  unsigned long max_packet, max_packet_size;
  unsigned int pkt_nr, compress_pkt_nr;
  timeout_t write_timeout, read_timeout;
  unsigned int retry_count;
  int fcntl;
  unsigned int *return_status;
  unsigned char reading_or_writing;
  unsigned char save_char;
  bool compress;
  unsigned int last_errno;
  unsigned char error;
  /** Client library error message buffer. Actually belongs to struct MYSQL. */
  char last_error[MYSQL_ERRMSG_SIZE];
  /** Client library sqlstate buffer. Set along with the error message. */
  char sqlstate[SQLSTATE_LENGTH + 1];
  /**
    Extension pointer, for the caller private use.
    Any program linking with the networking library can use this pointer,
    which is handy when private connection specific data needs to be
    maintained.
    The mysqld server process uses this pointer internally,
    to maintain the server internal instrumentation for the connection.
  */
  void *extension;
  unsigned int receive_buffer_size;
} NET;

#define packet_error (~(unsigned long)0)

/**
  @addtogroup group_cs_backward_compatibility Backward compatibility
  @ingroup group_cs
  @{
*/
#define CLIENT_MULTI_QUERIES CLIENT_MULTI_STATEMENTS
#define FIELD_TYPE_DECIMAL MYSQL_TYPE_DECIMAL
#define FIELD_TYPE_NEWDECIMAL MYSQL_TYPE_NEWDECIMAL
#define FIELD_TYPE_TINY MYSQL_TYPE_TINY
#define FIELD_TYPE_SHORT MYSQL_TYPE_SHORT
#define FIELD_TYPE_LONG MYSQL_TYPE_LONG
#define FIELD_TYPE_FLOAT MYSQL_TYPE_FLOAT
#define FIELD_TYPE_DOUBLE MYSQL_TYPE_DOUBLE
#define FIELD_TYPE_NULL MYSQL_TYPE_NULL
#define FIELD_TYPE_TIMESTAMP MYSQL_TYPE_TIMESTAMP
#define FIELD_TYPE_LONGLONG MYSQL_TYPE_LONGLONG
#define FIELD_TYPE_INT24 MYSQL_TYPE_INT24
#define FIELD_TYPE_DATE MYSQL_TYPE_DATE
#define FIELD_TYPE_TIME MYSQL_TYPE_TIME
#define FIELD_TYPE_DATETIME MYSQL_TYPE_DATETIME
#define FIELD_TYPE_YEAR MYSQL_TYPE_YEAR
#define FIELD_TYPE_NEWDATE MYSQL_TYPE_NEWDATE
#define FIELD_TYPE_ENUM MYSQL_TYPE_ENUM
#define FIELD_TYPE_SET MYSQL_TYPE_SET
#define FIELD_TYPE_TINY_BLOB MYSQL_TYPE_TINY_BLOB
#define FIELD_TYPE_MEDIUM_BLOB MYSQL_TYPE_MEDIUM_BLOB
#define FIELD_TYPE_LONG_BLOB MYSQL_TYPE_LONG_BLOB
#define FIELD_TYPE_BLOB MYSQL_TYPE_BLOB
#define FIELD_TYPE_VAR_STRING MYSQL_TYPE_VAR_STRING
#define FIELD_TYPE_STRING MYSQL_TYPE_STRING
#define FIELD_TYPE_CHAR MYSQL_TYPE_TINY
#define FIELD_TYPE_INTERVAL MYSQL_TYPE_ENUM
#define FIELD_TYPE_GEOMETRY MYSQL_TYPE_GEOMETRY
#define FIELD_TYPE_BIT MYSQL_TYPE_BIT
/** @}*/

/**
  @addtogroup group_cs_shutdown_kill_constants Shutdown/kill enums and constants
  @ingroup group_cs

  @sa THD::is_killable
  @{
*/
#define MYSQL_SHUTDOWN_KILLABLE_CONNECT (unsigned char)(1 << 0)
#define MYSQL_SHUTDOWN_KILLABLE_TRANS (unsigned char)(1 << 1)
#define MYSQL_SHUTDOWN_KILLABLE_LOCK_TABLE (unsigned char)(1 << 2)
#define MYSQL_SHUTDOWN_KILLABLE_UPDATE (unsigned char)(1 << 3)

/**
  We want levels to be in growing order of hardness (because we use number
  comparisons).

  @note ::SHUTDOWN_DEFAULT does not respect the growing property, but it's ok.
*/
enum mysql_enum_shutdown_level {
  SHUTDOWN_DEFAULT = 0,
  /** Wait for existing connections to finish */
  SHUTDOWN_WAIT_CONNECTIONS = MYSQL_SHUTDOWN_KILLABLE_CONNECT,
  /** Wait for existing transactons to finish */
  SHUTDOWN_WAIT_TRANSACTIONS = MYSQL_SHUTDOWN_KILLABLE_TRANS,
  /** Wait for existing updates to finish (=> no partial MyISAM update) */
  SHUTDOWN_WAIT_UPDATES = MYSQL_SHUTDOWN_KILLABLE_UPDATE,
  /** Flush InnoDB buffers and other storage engines' buffers*/
  SHUTDOWN_WAIT_ALL_BUFFERS = (MYSQL_SHUTDOWN_KILLABLE_UPDATE << 1),
  /** Don't flush InnoDB buffers, flush other storage engines' buffers*/
  SHUTDOWN_WAIT_CRITICAL_BUFFERS = (MYSQL_SHUTDOWN_KILLABLE_UPDATE << 1) + 1,
  /** Query level of the KILL command */
  KILL_QUERY = 254,
  /** Connection level of the KILL command */
  KILL_CONNECTION = 255
};
/** @}*/

enum enum_resultset_metadata {
  /** No metadata will be sent. */
  RESULTSET_METADATA_NONE = 0,
  /** The server will send all metadata. */
  RESULTSET_METADATA_FULL = 1
};

enum enum_cursor_type {
  CURSOR_TYPE_NO_CURSOR = 0,
  CURSOR_TYPE_READ_ONLY = 1,
  CURSOR_TYPE_FOR_UPDATE = 2,
  CURSOR_TYPE_SCROLLABLE = 4
};

/** options for ::mysql_options() */
enum enum_mysql_set_option {
  MYSQL_OPTION_MULTI_STATEMENTS_ON,
  MYSQL_OPTION_MULTI_STATEMENTS_OFF
};

/*
  Describes the current state of Asynchronous connection phase state machine
*/
enum connect_stage {
  CONNECT_STAGE_INVALID = 0,
  CONNECT_STAGE_NOT_STARTED,
  CONNECT_STAGE_NET_BEGIN_CONNECT,
  CONNECT_STAGE_NET_WAIT_CONNECT,
  CONNECT_STAGE_NET_COMPLETE_CONNECT,
  CONNECT_STAGE_READ_GREETING,
  CONNECT_STAGE_PARSE_HANDSHAKE,
  CONNECT_STAGE_ESTABLISH_SSL,
  CONNECT_STAGE_AUTHENTICATE,
  CONNECT_STAGE_PREP_SELECT_DATABASE,
  CONNECT_STAGE_PREP_INIT_COMMANDS,
  CONNECT_STAGE_SEND_ONE_INIT_COMMAND,
  CONNECT_STAGE_COMPLETE
};

/**
  Type of state change information that the server can include in the Ok
  packet.

  @note
    - session_state_type shouldn't go past 255 (i.e. 1-byte boundary).
    - Modify the definition of ::SESSION_TRACK_END when a new member is added.
*/
enum enum_session_state_type {
  SESSION_TRACK_SYSTEM_VARIABLES, /**< Session system variables */
  SESSION_TRACK_SCHEMA,           /**< Current schema */
  SESSION_TRACK_STATE_CHANGE,     /**< track session state changes */
  SESSION_TRACK_GTIDS,            /**< See also: session_track_gtids */
  SESSION_TRACK_TRANSACTION_CHARACTERISTICS, /**< Transaction chistics */
  SESSION_TRACK_TRANSACTION_STATE,           /**< Transaction state */

  // Leave space before RPC_ID session tracking for other upstream values
  SESSION_TRACK_RESP_ATTR = 32 /* Response attributes */
};

/** start of ::enum_session_state_type */
#define SESSION_TRACK_BEGIN SESSION_TRACK_SYSTEM_VARIABLES

/** End of ::enum_session_state_type */
#define SESSION_TRACK_END SESSION_TRACK_RESP_ATTR

/** is T a valid session state type */
#define IS_SESSION_STATE_TYPE(T) \
  (((int)(T) >= SESSION_TRACK_BEGIN) && ((T) <= SESSION_TRACK_END))

#define net_new_transaction(net) ((net)->pkt_nr = 0)

bool my_net_init(struct NET *net, MYSQL_VIO vio);
void my_net_local_init(struct NET *net);
void net_end(struct NET *net);
// extern "C" since it is part of the libmysql ABI.
#ifdef __cplusplus
extern "C" {
#endif
void net_clear(struct NET *net, bool check_buffer);
#ifdef __cplusplus
}
#endif
void net_claim_memory_ownership(struct NET *net);
bool net_realloc(struct NET *net, size_t length);
// extern "C" since it is part of the libmysql ABI.
#ifdef __cplusplus
extern "C" {
#endif
bool net_flush(struct NET *net);
bool my_net_write(struct NET *net, const unsigned char *packet, size_t len);
#ifdef __cplusplus
}
#endif
bool net_write_command(struct NET *net, unsigned char command,
                       const unsigned char *header, size_t head_len,
                       const unsigned char *packet, size_t len);
bool net_write_packet(struct NET *net, const unsigned char *packet,
                      size_t length);
unsigned long my_net_read(struct NET *net);
void my_net_set_write_timeout(struct NET *net, const timeout_t timeout);
void my_net_set_read_timeout(struct NET *net, const timeout_t timeout);
void my_net_set_retry_count(struct NET *net, unsigned int retry_count);

timeout_t timeout_from_seconds(uint seconds);
timeout_t timeout_from_millis(uint ms);
timeout_t timeout_infinite(void);
bool timeout_is_infinite(const timeout_t t);
int timeout_is_nonzero(const timeout_t t);
unsigned int timeout_to_millis(const timeout_t t);
// toSeconds rounds down.
unsigned int timeout_to_seconds(const timeout_t t);

struct rand_struct {
  unsigned long seed1, seed2, max_value;
  double max_value_dbl;
};

/* Include the types here so existing UDFs can keep compiling */
#include <mysql/udf_registration_types.h>

/**
  @addtogroup group_cs_compresson_constants Constants when using compression
  @ingroup group_cs
  @{
*/
#define NET_HEADER_SIZE 4  /**< standard header size */
#define COMP_HEADER_SIZE 3 /**< compression header extra size */
/** @}*/

/* Prototypes to password functions */

/*
  These functions are used for authentication by client and server and
  implemented in sql/password.c
*/

void randominit(struct rand_struct *, unsigned long seed1, unsigned long seed2);
double my_rnd(struct rand_struct *);
void create_random_string(char *to, unsigned int length,
                          struct rand_struct *rand_st);

void hash_password(unsigned long *to, const char *password,
                   unsigned int password_len);
void make_scrambled_password_323(char *to, const char *password);
void scramble_323(char *to, const char *message, const char *password);
bool check_scramble_323(const unsigned char *reply, const char *message,
                        unsigned long *salt);
void get_salt_from_password_323(unsigned long *res, const char *password);
void make_password_from_salt_323(char *to, const unsigned long *salt);

void make_scrambled_password(char *to, const char *password);
void scramble(char *to, const char *message, const char *password);
bool check_scramble(const unsigned char *reply, const char *message,
                    const unsigned char *hash_stage2);
void get_salt_from_password(unsigned char *res, const char *password);
void make_password_from_salt(char *to, const unsigned char *hash_stage2);
char *octet2hex(char *to, const char *str, unsigned int len);

/* end of password.c */

bool generate_sha256_scramble(unsigned char *dst, size_t dst_size,
                              const char *src, size_t src_size, const char *rnd,
                              size_t rnd_size);

// extern "C" since it is an (undocumented) part of the libmysql ABI.
#ifdef __cplusplus
extern "C" {
#endif
char *get_tty_password(const char *opt_message);
#ifdef __cplusplus
}
#endif

const char *mysql_errno_to_sqlstate(unsigned int mysql_errno);

/* Some other useful functions */

// Need to be extern "C" for the time being, due to memcached.
#ifdef __cplusplus
extern "C" {
#endif
bool my_thread_init(void);
void my_thread_end(void);
#ifdef __cplusplus
}
#endif

#ifdef STDCALL
unsigned long STDCALL net_field_length(unsigned char **packet);
unsigned long STDCALL net_field_length_checked(unsigned char **packet,
                                               unsigned long max_length);
#endif
uint64_t net_field_length_ll(unsigned char **packet);
unsigned char *net_store_length(unsigned char *pkg, unsigned long long length);
unsigned int net_length_size(unsigned long long num);
unsigned int net_field_length_size(const unsigned char *pos);

#define NULL_LENGTH ((unsigned long)~0) /**< For ::net_store_length() */
#define MYSQL_STMT_HEADER 4
#define MYSQL_LONG_DATA_HEADER 6
#endif
