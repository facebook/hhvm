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
  @file include/mysql.h
  This file defines the client API to MySQL and also the ABI of the
  dynamically linked libmysqlclient.

  The ABI should never be changed in a released product of MySQL,
  thus you need to take great care when changing the file. In case
  the file is changed so the ABI is broken, you must also update
  the SHARED_LIB_MAJOR_VERSION in cmake/mysql_version.cmake
*/

#ifndef _mysql_h
#define _mysql_h

#ifndef MYSQL_ABI_CHECK
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#endif

// Legacy definition for the benefit of old code. Use uint64_t in new code.
// If you get warnings from printf, use the PRIu64 macro, or, if you need
// compatibility with older versions of the client library, cast
// before printing.
typedef uint64_t my_ulonglong;

#ifndef my_socket_defined
#define my_socket_defined
#ifdef _WIN32
#include <windows.h>
#ifdef WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif
#define my_socket SOCKET
#else
typedef int my_socket;
#endif /* _WIN32 */
#endif /* my_socket_defined */

// Small extra definition to avoid pulling in my_compiler.h in client code.
// IWYU pragma: no_include "my_compiler.h"
#ifndef MY_COMPILER_INCLUDED
#if !defined(_WIN32)
#define STDCALL
#else
#define STDCALL __stdcall
#endif
#endif /* MY_COMPILER_INCLUDED */

#include "field_types.h"
#include "my_list.h"
#include "mysql_com.h"

/* Include declarations of plug-in API */
#include "mysql/client_plugin.h"  // IWYU pragma: keep

/*
  The client should be able to know which version it is compiled against,
  even if mysql.h doesn't use this information directly.
*/
#include "mysql_version.h"  // IWYU pragma: keep

// MYSQL_TIME is part of our public API.
#include "mysql_time.h"  // IWYU pragma: keep

// The error messages are part of our public API.
#include "errmsg.h"  // IWYU pragma: keep

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int mysql_port;
extern char *mysql_unix_port;

#define CLIENT_NET_RETRY_COUNT 1                 /* Retry count */
#define CLIENT_NET_READ_TIMEOUT 365 * 24 * 3600  /* Timeout on read */
#define CLIENT_NET_WRITE_TIMEOUT 365 * 24 * 3600 /* Timeout on write */

#define IS_PRI_KEY(n) ((n)&PRI_KEY_FLAG)
#define IS_NOT_NULL(n) ((n)&NOT_NULL_FLAG)
#define IS_BLOB(n) ((n)&BLOB_FLAG)
/**
   Returns true if the value is a number which does not need quotes for
   the sql_lex.cc parser to parse correctly.
*/
#define IS_NUM(t)                                              \
  (((t) <= MYSQL_TYPE_INT24 && (t) != MYSQL_TYPE_TIMESTAMP) || \
   (t) == MYSQL_TYPE_YEAR || (t) == MYSQL_TYPE_NEWDECIMAL)
#define IS_LONGDATA(t) ((t) >= MYSQL_TYPE_TINY_BLOB && (t) <= MYSQL_TYPE_STRING)

typedef struct MYSQL_FIELD {
  char *name;               /* Name of column */
  char *org_name;           /* Original column name, if an alias */
  char *table;              /* Table of column if column was a field */
  char *org_table;          /* Org table name, if table was an alias */
  char *db;                 /* Database for table */
  char *catalog;            /* Catalog for table */
  char *def;                /* Default value (set by mysql_list_fields) */
  unsigned long length;     /* Width of column (create length) */
  unsigned long max_length; /* Max width for selected set */
  unsigned int name_length;
  unsigned int org_name_length;
  unsigned int table_length;
  unsigned int org_table_length;
  unsigned int db_length;
  unsigned int catalog_length;
  unsigned int def_length;
  unsigned int flags;         /* Div flags */
  unsigned int decimals;      /* Number of decimals in field */
  unsigned int charsetnr;     /* Character set */
  enum enum_field_types type; /* Type of field. See mysql_com.h for types */
  void *extension;
} MYSQL_FIELD;

typedef char **MYSQL_ROW;                /* return data as array of strings */
typedef unsigned int MYSQL_FIELD_OFFSET; /* offset to current field */

#define MYSQL_COUNT_ERROR (~(uint64_t)0)

/* backward compatibility define - to be removed eventually */
#define ER_WARN_DATA_TRUNCATED WARN_DATA_TRUNCATED

typedef struct MYSQL_ROWS {
  struct MYSQL_ROWS *next; /* list of rows */
  MYSQL_ROW data;
  unsigned long length;
} MYSQL_ROWS;

typedef MYSQL_ROWS *MYSQL_ROW_OFFSET; /* offset to current row */

struct MEM_ROOT;

typedef struct MYSQL_DATA {
  MYSQL_ROWS *data;
  struct MEM_ROOT *alloc;
  uint64_t rows;
  unsigned int fields;
} MYSQL_DATA;

enum mysql_option {
  MYSQL_OPT_CONNECT_TIMEOUT,
  MYSQL_OPT_COMPRESS,
  MYSQL_OPT_NAMED_PIPE,
  MYSQL_INIT_COMMAND,
  MYSQL_READ_DEFAULT_FILE,
  MYSQL_READ_DEFAULT_GROUP,
  MYSQL_SET_CHARSET_DIR,
  MYSQL_SET_CHARSET_NAME,
  MYSQL_OPT_LOCAL_INFILE,
  MYSQL_OPT_PROTOCOL,
  MYSQL_SHARED_MEMORY_BASE_NAME,
  MYSQL_OPT_READ_TIMEOUT,
  MYSQL_OPT_WRITE_TIMEOUT,
  MYSQL_OPT_USE_RESULT,
  MYSQL_REPORT_DATA_TRUNCATION,
  MYSQL_OPT_RECONNECT,
  MYSQL_PLUGIN_DIR,
  MYSQL_DEFAULT_AUTH,
  MYSQL_OPT_BIND,
  MYSQL_OPT_SSL_KEY,
  MYSQL_OPT_SSL_CERT,
  MYSQL_OPT_SSL_CA,
  MYSQL_OPT_SSL_CAPATH,
  MYSQL_OPT_SSL_CIPHER,
  MYSQL_OPT_SSL_CRL,
  MYSQL_OPT_SSL_CRLPATH,
  MYSQL_OPT_CONNECT_ATTR_RESET,
  MYSQL_OPT_CONNECT_ATTR_ADD,
  MYSQL_OPT_CONNECT_ATTR_DELETE,
  MYSQL_OPT_QUERY_ATTR_RESET,
  MYSQL_OPT_QUERY_ATTR_ADD,
  MYSQL_OPT_QUERY_ATTR_DELETE,
  MYSQL_SERVER_PUBLIC_KEY,
  MYSQL_ENABLE_CLEARTEXT_PLUGIN,
  MYSQL_OPT_CAN_HANDLE_EXPIRED_PASSWORDS,
  MYSQL_OPT_MAX_ALLOWED_PACKET,
  MYSQL_OPT_NET_BUFFER_LENGTH,
  MYSQL_OPT_TLS_VERSION,
  MYSQL_OPT_SSL_MODE,
  MYSQL_OPT_GET_SERVER_PUBLIC_KEY,
  MYSQL_OPT_RETRY_COUNT,
  MYSQL_OPT_OPTIONAL_RESULTSET_METADATA,
  MYSQL_OPT_SSL_FIPS_MODE,
  MYSQL_OPT_TLS_CIPHERSUITES,
  MYSQL_OPT_COMPRESSION_ALGORITHMS,
  MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
  MYSQL_OPT_SSL_SESSION,
  MYSQL_OPT_SSL_CONTEXT,
  MYSQL_OPT_NET_RECEIVE_BUFFER_SIZE,
  MYSQL_OPT_CONNECT_TIMEOUT_MS,
  MYSQL_OPT_READ_TIMEOUT_MS,
  MYSQL_OPT_WRITE_TIMEOUT_MS,
  MYSQL_OPT_TLS_SNI_SERVERNAME,
  MYSQL_OPT_TLS_CERT_CALLBACK,
  MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT,
  MYSQL_OPT_TOS,
};

/**
  @todo remove the "extension", move st_mysql_options completely
  out of mysql.h
*/
struct st_mysql_options_extention;

struct st_mysql_options {
  timeout_t connect_timeout, read_timeout, write_timeout;
  unsigned int port, protocol;
  unsigned long client_flag;
  char *host, *user, *password, *unix_socket, *db;
  struct Init_commands_array *init_commands;
  char *my_cnf_file, *my_cnf_group, *charset_dir, *charset_name;
  char *ssl_key;    /* PEM key file */
  char *ssl_cert;   /* PEM cert file */
  char *ssl_ca;     /* PEM CA file */
  char *ssl_capath; /* PEM directory of CA-s? */
  char *ssl_cipher; /* cipher to use */
  char *shared_memory_base_name;
  unsigned long max_allowed_packet;
  bool compress, named_pipe;
  /**
    The local address to bind when connecting to remote server.
  */
  char *bind_address;
  /* 0 - never report, 1 - always report (default) */
  bool report_data_truncation;

  /* function pointers for local infile support */
  int (*local_infile_init)(void **, const char *, void *);
  int (*local_infile_read)(void *, char *, unsigned int);
  void (*local_infile_end)(void *);
  int (*local_infile_error)(void *, char *, unsigned int);
  void *local_infile_userdata;
  struct st_mysql_options_extention *extension;
};

enum mysql_status {
  MYSQL_STATUS_READY,
  MYSQL_STATUS_GET_RESULT,
  MYSQL_STATUS_USE_RESULT,
  MYSQL_STATUS_STATEMENT_GET_RESULT
};

enum mysql_protocol_type {
  MYSQL_PROTOCOL_DEFAULT,
  MYSQL_PROTOCOL_TCP,
  MYSQL_PROTOCOL_SOCKET,
  MYSQL_PROTOCOL_PIPE,
  MYSQL_PROTOCOL_MEMORY
};

enum mysql_ssl_mode {
  SSL_MODE_DISABLED = 1,
  SSL_MODE_PREFERRED,
  SSL_MODE_REQUIRED,
  SSL_MODE_VERIFY_CA,
  SSL_MODE_VERIFY_IDENTITY
};

enum mysql_ssl_fips_mode {
  SSL_FIPS_MODE_OFF = 0,
  SSL_FIPS_MODE_ON = 1,
  SSL_FIPS_MODE_STRICT
};

typedef struct character_set {
  unsigned int number;   /* character set number              */
  unsigned int state;    /* character set state               */
  const char *csname;    /* collation name                    */
  const char *name;      /* character set name                */
  const char *comment;   /* comment                           */
  const char *dir;       /* character set directory           */
  unsigned int mbminlen; /* min. length for multibyte strings */
  unsigned int mbmaxlen; /* max. length for multibyte strings */
} MY_CHARSET_INFO;

struct MYSQL_METHODS;
struct MYSQL_STMT;

typedef struct MYSQL {
  NET net;                     /* Communication parameters */
  unsigned char *connector_fd; /* ConnectorFd for SSL */
  char *host, *user, *passwd, *unix_socket, *server_version, *host_info;
  char *info, *db;
  struct CHARSET_INFO *charset;
  MYSQL_FIELD *fields;
  struct MEM_ROOT *field_alloc;
  uint64_t affected_rows;
  uint64_t insert_id;  /* id if insert on table with NEXTNR */
  uint64_t extra_info; /* Not used */
  /* Should the resultset checksum be calculated */
  bool should_record_checksum;
  /* Store the computed checksum from the resultset */
  unsigned long checksum;
  unsigned long thread_id; /* Id for connection in server */
  unsigned long packet_length;
  unsigned int port;
  unsigned long client_flag, server_capabilities;
  unsigned int protocol_version;
  unsigned int field_count;
  unsigned int server_status;
  unsigned int server_language;
  unsigned int warning_count;
  struct st_mysql_options options;
  enum mysql_status status;
  enum enum_resultset_metadata resultset_metadata;
  bool free_me;   /* If free in mysql_close */
  bool reconnect; /* set to 1 if automatic reconnect */

  /* session-wide random string */
  char scramble[SCRAMBLE_LENGTH + 1];

  LIST *stmts; /* list of all statements */
  const struct MYSQL_METHODS *methods;
  void *thd;
  /*
    Points to boolean flag in MYSQL_RES  or MYSQL_STMT. We set this flag
    from mysql_stmt_close if close had to cancel result set of this object.
  */
  bool *unbuffered_fetch_owner;
  void *extension;
} MYSQL;

typedef struct MYSQL_RES {
  uint64_t row_count;
  MYSQL_FIELD *fields;
  struct MYSQL_DATA *data;
  MYSQL_ROWS *data_cursor;
  unsigned long *lengths; /* column lengths of current row */
  MYSQL *handle;          /* for unbuffered reads */
  const struct MYSQL_METHODS *methods;
  MYSQL_ROW row;         /* If unbuffered read */
  MYSQL_ROW current_row; /* buffer to current row */
  struct MEM_ROOT *field_alloc;
  unsigned int field_count, current_field;
  bool eof; /* Used by mysql_fetch_row */
  /* mysql_stmt_close() had to cancel this result */
  bool unbuffered_fetch_cancelled;
  enum enum_resultset_metadata metadata;
  void *extension;
} MYSQL_RES;

/**
  Flag to indicate that COM_BINLOG_DUMP_GTID should
  be used rather than COM_BINLOG_DUMP in the @sa mysql_binlog_open().
*/
#define MYSQL_RPL_GTID (1 << 16)
/**
  Skip HEARBEAT events in the @sa mysql_binlog_fetch().
*/
#define MYSQL_RPL_SKIP_HEARTBEAT (1 << 17)

/**
  Struct for information about a replication stream.

  @sa mysql_binlog_open()
  @sa mysql_binlog_fetch()
  @sa mysql_binlog_close()
*/
typedef struct MYSQL_RPL {
  size_t file_name_length; /** Length of the 'file_name' or 0     */
  const char *file_name;   /** Filename of the binary log to read */
  uint64_t start_position; /** Position in the binary log to      */
                           /*  start reading from                 */
  unsigned int server_id;  /** Server ID to use when identifying  */
                           /*  with the master                    */
  unsigned int flags;      /** Flags, e.g. MYSQL_RPL_GTID         */

  /** Size of gtid set data              */
  size_t gtid_set_encoded_size;
  /** Callback function which is called  */
  /*  from @sa mysql_binlog_open() to    */
  /*  fill command packet gtid set       */
  void (*fix_gtid_set)(struct MYSQL_RPL *rpl, unsigned char *packet_gtid_set);
  void *gtid_set_arg; /** GTID set data or an argument for   */
                      /*  fix_gtid_set() callback function   */

  unsigned long size;          /** Size of the packet returned by     */
                               /*  mysql_binlog_fetch()               */
  const unsigned char *buffer; /** Pointer to returned data           */
} MYSQL_RPL;

/*
  Set up and bring down the server; to ensure that applications will
  work when linked against either the standard client library or the
  embedded server library, these functions should be called.
*/
int STDCALL mysql_server_init(int argc, char **argv, char **groups);
void STDCALL mysql_server_end(void);

/*
  mysql_server_init/end need to be called when using libmysqld or
  libmysqlclient (exactly, mysql_server_init() is called by mysql_init() so
  you don't need to call it explicitely; but you need to call
  mysql_server_end() to free memory). The names are a bit misleading
  (mysql_SERVER* to be used when using libmysqlCLIENT). So we add more general
  names which suit well whether you're using libmysqld or libmysqlclient. We
  intend to promote these aliases over the mysql_server* ones.
*/
#define mysql_library_init mysql_server_init
#define mysql_library_end mysql_server_end

/*
  Set up and bring down a thread; these function should be called
  for each thread in an application which opens at least one MySQL
  connection.  All uses of the connection(s) should be between these
  function calls.
*/
bool STDCALL mysql_thread_init(void);
void STDCALL mysql_thread_end(void);

/*
  Functions to get information from the MYSQL and MYSQL_RES structures
  Should definitely be used if one uses shared libraries.
*/

uint64_t STDCALL mysql_num_rows(MYSQL_RES *res);
unsigned int STDCALL mysql_num_fields(MYSQL_RES *res);
bool STDCALL mysql_eof(MYSQL_RES *res);
MYSQL_FIELD *STDCALL mysql_fetch_field_direct(MYSQL_RES *res,
                                              unsigned int fieldnr);
MYSQL_FIELD *STDCALL mysql_fetch_fields(MYSQL_RES *res);
MYSQL_ROW_OFFSET STDCALL mysql_row_tell(MYSQL_RES *res);
MYSQL_FIELD_OFFSET STDCALL mysql_field_tell(MYSQL_RES *res);
enum enum_resultset_metadata STDCALL mysql_result_metadata(MYSQL_RES *result);

unsigned int STDCALL mysql_field_count(MYSQL *mysql);
uint64_t STDCALL mysql_affected_rows(MYSQL *mysql);
uint64_t STDCALL mysql_insert_id(MYSQL *mysql);
unsigned int STDCALL mysql_errno(MYSQL *mysql);
const char *STDCALL mysql_error(MYSQL *mysql);
const char *STDCALL mysql_sqlstate(MYSQL *mysql);
unsigned int STDCALL mysql_warning_count(MYSQL *mysql);
const char *STDCALL mysql_info(MYSQL *mysql);
unsigned long STDCALL mysql_thread_id(MYSQL *mysql);
const char *STDCALL mysql_character_set_name(MYSQL *mysql);
int STDCALL mysql_set_character_set(MYSQL *mysql, const char *csname);

MYSQL *STDCALL mysql_init(MYSQL *mysql);
bool STDCALL mysql_ssl_set(MYSQL *mysql, const char *key, const char *cert,
                           const char *ca, const char *capath,
                           const char *cipher);
const char* STDCALL mysql_get_ssl_version(MYSQL *mysql);
const char *STDCALL mysql_get_ssl_cipher(MYSQL *mysql);
void *STDCALL mysql_get_ssl_session(MYSQL *mysql);
bool STDCALL mysql_get_ssl_session_reused(MYSQL *mysql);
/*
  Take ownership of the OpenSSL SSL_CTX instance associated with this
  connection.  In general, SSL_CTX objects should be re-used.  Either one can
  be taken from an existing connection or created by hand (in which case the
  mysql SSL options such as MYSQL_OPT_SSL_KEY are ignored since they
  influence the SSL_CTX object).

  The caller is responsible for freeing this via SSL_CTX_free.  Note that this
  is a void* strictly to avoid including SSL headers in this file.
*/
void *STDCALL mysql_take_ssl_context_ownership(MYSQL *mysql);
bool STDCALL mysql_change_user(MYSQL *mysql, const char *user,
                               const char *passwd, const char *db);
enum net_async_status STDCALL mysql_change_user_nonblocking(MYSQL *mysql,
                                                            const char *user,
                                                            const char *passwd,
                                                            const char *db);
MYSQL *STDCALL mysql_real_connect(MYSQL *mysql, const char *host,
                                  const char *user, const char *passwd,
                                  const char *db, unsigned int port,
                                  const char *unix_socket,
                                  unsigned long clientflag);
int STDCALL mysql_select_db(MYSQL *mysql, const char *db);
int STDCALL mysql_query(MYSQL *mysql, const char *q);
int STDCALL mysql_send_query(MYSQL *mysql, const char *q, unsigned long length);
int STDCALL mysql_real_query(MYSQL *mysql, const char *q, unsigned long length);
MYSQL_RES *STDCALL mysql_store_result(MYSQL *mysql);
MYSQL_RES *STDCALL mysql_use_result(MYSQL *mysql);

enum net_async_status STDCALL mysql_real_connect_nonblocking(
    MYSQL *mysql, const char *host, const char *user, const char *passwd,
    const char *db, unsigned int port, const char *unix_socket,
    unsigned long clientflag);
enum net_async_status STDCALL mysql_send_query_nonblocking(
    MYSQL *mysql, const char *query, unsigned long length);
enum net_async_status STDCALL mysql_real_query_nonblocking(
    MYSQL *mysql, const char *query, unsigned long length);
enum net_async_status STDCALL
mysql_store_result_nonblocking(MYSQL *mysql, MYSQL_RES **result);
enum net_async_status STDCALL mysql_next_result_nonblocking(MYSQL *mysql);
enum net_async_status STDCALL mysql_select_db_nonblocking(MYSQL *mysql,
                                                          const char *db,
                                                          bool *error);
int STDCALL mysql_get_socket_descriptor(MYSQL *mysql);
void STDCALL mysql_get_character_set_info(MYSQL *mysql,
                                          MY_CHARSET_INFO *charset);

int STDCALL mysql_session_track_get_first(MYSQL *mysql,
                                          enum enum_session_state_type type,
                                          const char **data, size_t *length);
int STDCALL mysql_session_track_get_next(MYSQL *mysql,
                                         enum enum_session_state_type type,
                                         const char **data, size_t *length);
int STDCALL mysql_resp_attr_find(MYSQL *mysql, const char *lookup,
                                 const char **data, size_t *length);

enum connect_stage STDCALL mysql_get_connect_stage(MYSQL *mysql);
/* local infile support */

#define LOCAL_INFILE_ERROR_LEN 512

void mysql_set_local_infile_handler(
    MYSQL *mysql, int (*local_infile_init)(void **, const char *, void *),
    int (*local_infile_read)(void *, char *, unsigned int),
    void (*local_infile_end)(void *),
    int (*local_infile_error)(void *, char *, unsigned int), void *);

void mysql_set_local_infile_default(MYSQL *mysql);
int STDCALL mysql_shutdown(MYSQL *mysql,
                           enum mysql_enum_shutdown_level shutdown_level);
int STDCALL mysql_dump_debug_info(MYSQL *mysql);
int STDCALL mysql_refresh(MYSQL *mysql, unsigned int refresh_options);
int STDCALL mysql_kill(MYSQL *mysql, unsigned long pid);
int STDCALL mysql_set_server_option(MYSQL *mysql,
                                    enum enum_mysql_set_option option);
int STDCALL mysql_ping(MYSQL *mysql);
const char *STDCALL mysql_stat(MYSQL *mysql);
const char *STDCALL mysql_get_server_info(MYSQL *mysql);
const char *STDCALL mysql_get_client_info(void);
unsigned long STDCALL mysql_get_client_version(void);
const char *STDCALL mysql_get_host_info(MYSQL *mysql);
unsigned long STDCALL mysql_get_server_version(MYSQL *mysql);
unsigned int STDCALL mysql_get_proto_info(MYSQL *mysql);
MYSQL_RES *STDCALL mysql_list_dbs(MYSQL *mysql, const char *wild);
MYSQL_RES *STDCALL mysql_list_tables(MYSQL *mysql, const char *wild);
MYSQL_RES *STDCALL mysql_list_processes(MYSQL *mysql);
int STDCALL mysql_options(MYSQL *mysql, enum mysql_option option,
                          const void *arg);
int STDCALL mysql_options4(MYSQL *mysql, enum mysql_option option,
                           const void *arg1, const void *arg2);
int STDCALL mysql_get_option(MYSQL *mysql, enum mysql_option option,
                             const void *arg);
void STDCALL mysql_free_result(MYSQL_RES *result);
enum net_async_status STDCALL mysql_free_result_nonblocking(MYSQL_RES *result);
void STDCALL mysql_data_seek(MYSQL_RES *result, uint64_t offset);
MYSQL_ROW_OFFSET STDCALL mysql_row_seek(MYSQL_RES *result,
                                        MYSQL_ROW_OFFSET offset);
MYSQL_FIELD_OFFSET STDCALL mysql_field_seek(MYSQL_RES *result,
                                            MYSQL_FIELD_OFFSET offset);
MYSQL_ROW STDCALL mysql_fetch_row(MYSQL_RES *result);
enum net_async_status STDCALL mysql_fetch_row_nonblocking(MYSQL_RES *res,
                                                          MYSQL_ROW *row);

unsigned long *STDCALL mysql_fetch_lengths(MYSQL_RES *result);
MYSQL_FIELD *STDCALL mysql_fetch_field(MYSQL_RES *result);
MYSQL_RES *STDCALL mysql_list_fields(MYSQL *mysql, const char *table,
                                     const char *wild);
unsigned long STDCALL mysql_escape_string(char *to, const char *from,
                                          unsigned long from_length);
unsigned long STDCALL mysql_hex_string(char *to, const char *from,
                                       unsigned long from_length);
unsigned long STDCALL mysql_real_escape_string(MYSQL *mysql, char *to,
                                               const char *from,
                                               unsigned long length);
unsigned long STDCALL mysql_real_escape_string_quote(MYSQL *mysql, char *to,
                                                     const char *from,
                                                     unsigned long length,
                                                     char quote);
void STDCALL mysql_debug(const char *debug);
void STDCALL myodbc_remove_escape(MYSQL *mysql, char *name);
unsigned int STDCALL mysql_thread_safe(void);
bool STDCALL mysql_read_query_result(MYSQL *mysql);
int STDCALL mysql_reset_connection(MYSQL *mysql);
enum net_async_status STDCALL mysql_reset_connection_nonblocking(MYSQL *mysql);

unsigned long STDCALL cli_safe_read(MYSQL *mysql, bool *is_data_packet);
enum net_async_status STDCALL cli_safe_read_nonblocking(MYSQL *mysql,
                                                        bool *is_data_packet,
                                                        unsigned long *res);

int STDCALL mysql_binlog_open(MYSQL *mysql, MYSQL_RPL *rpl);
int STDCALL mysql_binlog_fetch(MYSQL *mysql, MYSQL_RPL *rpl);
void STDCALL mysql_binlog_close(MYSQL *mysql, MYSQL_RPL *rpl);

/*
  The following definitions are added for the enhanced
  client-server protocol
*/

/* statement state */
enum enum_mysql_stmt_state {
  MYSQL_STMT_INIT_DONE = 1,
  MYSQL_STMT_PREPARE_DONE,
  MYSQL_STMT_EXECUTE_DONE,
  MYSQL_STMT_FETCH_DONE
};

/*
  This structure is used to define bind information, and
  internally by the client library.
  Public members with their descriptions are listed below
  (conventionally `On input' refers to the binds given to
  mysql_stmt_bind_param, `On output' refers to the binds given
  to mysql_stmt_bind_result):

  buffer_type    - One of the MYSQL_* types, used to describe
                   the host language type of buffer.
                   On output: if column type is different from
                   buffer_type, column value is automatically converted
                   to buffer_type before it is stored in the buffer.
  buffer         - On input: points to the buffer with input data.
                   On output: points to the buffer capable to store
                   output data.
                   The type of memory pointed by buffer must correspond
                   to buffer_type. See the correspondence table in
                   the comment to mysql_stmt_bind_param.

  The two above members are mandatory for any kind of bind.

  buffer_length  - the length of the buffer. You don't have to set
                   it for any fixed length buffer: float, double,
                   int, etc. It must be set however for variable-length
                   types, such as BLOBs or STRINGs.

  length         - On input: in case when lengths of input values
                   are different for each execute, you can set this to
                   point at a variable containining value length. This
                   way the value length can be different in each execute.
                   If length is not NULL, buffer_length is not used.
                   Note, length can even point at buffer_length if
                   you keep bind structures around while fetching:
                   this way you can change buffer_length before
                   each execution, everything will work ok.
                   On output: if length is set, mysql_stmt_fetch will
                   write column length into it.

  is_null        - On input: points to a boolean variable that should
                   be set to TRUE for NULL values.
                   This member is useful only if your data may be
                   NULL in some but not all cases.
                   If your data is never NULL, is_null should be set to 0.
                   If your data is always NULL, set buffer_type
                   to MYSQL_TYPE_NULL, and is_null will not be used.

  is_unsigned    - On input: used to signify that values provided for one
                   of numeric types are unsigned.
                   On output describes signedness of the output buffer.
                   If, taking into account is_unsigned flag, column data
                   is out of range of the output buffer, data for this column
                   is regarded truncated. Note that this has no correspondence
                   to the sign of result set column, if you need to find it out
                   use mysql_stmt_result_metadata.
  error          - where to write a truncation error if it is present.
                   possible error value is:
                   0  no truncation
                   1  value is out of range or buffer is too small

  Please note that MYSQL_BIND also has internals members.
*/

typedef struct MYSQL_BIND {
  unsigned long *length; /* output length pointer */
  bool *is_null;         /* Pointer to null indicator */
  void *buffer;          /* buffer to get/put data */
  /* set this if you want to track data truncations happened during fetch */
  bool *error;
  unsigned char *row_ptr; /* for the current data position */
  void (*store_param_func)(NET *net, struct MYSQL_BIND *param);
  void (*fetch_result)(struct MYSQL_BIND *, MYSQL_FIELD *, unsigned char **row);
  void (*skip_result)(struct MYSQL_BIND *, MYSQL_FIELD *, unsigned char **row);
  /* output buffer length, must be set when fetching str/binary */
  unsigned long buffer_length;
  unsigned long offset;              /* offset position for char/binary fetch */
  unsigned long length_value;        /* Used if length is 0 */
  unsigned int param_number;         /* For null count and error messages */
  unsigned int pack_length;          /* Internal length for packed data */
  enum enum_field_types buffer_type; /* buffer type */
  bool error_value;                  /* used if error is 0 */
  bool is_unsigned;                  /* set if integer type is unsigned */
  bool long_data_used;               /* If used with mysql_send_long_data */
  bool is_null_value;                /* Used if is_null is 0 */
  void *extension;
} MYSQL_BIND;

struct MYSQL_STMT_EXT;

/* statement handler */
typedef struct MYSQL_STMT {
  struct MEM_ROOT *mem_root; /* root allocations */
  LIST list;                 /* list to keep track of all stmts */
  MYSQL *mysql;              /* connection handle */
  MYSQL_BIND *params;        /* input parameters */
  MYSQL_BIND *bind;          /* output parameters */
  MYSQL_FIELD *fields;       /* result set metadata */
  MYSQL_DATA result;         /* cached result set */
  MYSQL_ROWS *data_cursor;   /* current row in cached result */
  /*
    mysql_stmt_fetch() calls this function to fetch one row (it's different
    for buffered, unbuffered and cursor fetch).
  */
  int (*read_row_func)(struct MYSQL_STMT *stmt, unsigned char **row);
  /* copy of mysql->affected_rows after statement execution */
  uint64_t affected_rows;
  uint64_t insert_id;          /* copy of mysql->insert_id */
  unsigned long stmt_id;       /* Id for prepared statement */
  unsigned long flags;         /* i.e. type of cursor to open */
  unsigned long prefetch_rows; /* number of rows per one COM_FETCH */
  /*
    Copied from mysql->server_status after execute/fetch to know
    server-side cursor status for this statement.
  */
  unsigned int server_status;
  unsigned int last_errno;            /* error code */
  unsigned int param_count;           /* input parameter count */
  unsigned int field_count;           /* number of columns in result set */
  enum enum_mysql_stmt_state state;   /* statement state */
  char last_error[MYSQL_ERRMSG_SIZE]; /* error message */
  char sqlstate[SQLSTATE_LENGTH + 1];
  /* Types of input parameters should be sent to server */
  bool send_types_to_server;
  bool bind_param_done;           /* input buffers were supplied */
  unsigned char bind_result_done; /* output buffers were supplied */
  /* mysql_stmt_close() had to cancel this result */
  bool unbuffered_fetch_cancelled;
  /*
    Is set to true if we need to calculate field->max_length for
    metadata fields when doing mysql_stmt_store_result.
  */
  bool update_max_length;
  struct MYSQL_STMT_EXT *extension;
} MYSQL_STMT;

enum enum_stmt_attr_type {
  /*
    When doing mysql_stmt_store_result calculate max_length attribute
    of statement metadata. This is to be consistent with the old API,
    where this was done automatically.
    In the new API we do that only by request because it slows down
    mysql_stmt_store_result sufficiently.
  */
  STMT_ATTR_UPDATE_MAX_LENGTH,
  /*
    unsigned long with combination of cursor flags (read only, for update,
    etc)
  */
  STMT_ATTR_CURSOR_TYPE,
  /*
    Amount of rows to retrieve from server per one fetch if using cursors.
    Accepts unsigned long attribute in the range 1 - ulong_max
  */
  STMT_ATTR_PREFETCH_ROWS
};

MYSQL_STMT *STDCALL mysql_stmt_init(MYSQL *mysql);
int STDCALL mysql_stmt_prepare(MYSQL_STMT *stmt, const char *query,
                               unsigned long length);
int STDCALL mysql_stmt_execute(MYSQL_STMT *stmt);
int STDCALL mysql_stmt_fetch(MYSQL_STMT *stmt);
int STDCALL mysql_stmt_fetch_column(MYSQL_STMT *stmt, MYSQL_BIND *bind_arg,
                                    unsigned int column, unsigned long offset);
int STDCALL mysql_stmt_store_result(MYSQL_STMT *stmt);
unsigned long STDCALL mysql_stmt_param_count(MYSQL_STMT *stmt);
bool STDCALL mysql_stmt_attr_set(MYSQL_STMT *stmt,
                                 enum enum_stmt_attr_type attr_type,
                                 const void *attr);
bool STDCALL mysql_stmt_attr_get(MYSQL_STMT *stmt,
                                 enum enum_stmt_attr_type attr_type,
                                 void *attr);
bool STDCALL mysql_stmt_bind_param(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
bool STDCALL mysql_stmt_bind_result(MYSQL_STMT *stmt, MYSQL_BIND *bnd);
bool STDCALL mysql_stmt_close(MYSQL_STMT *stmt);
bool STDCALL mysql_stmt_reset(MYSQL_STMT *stmt);
bool STDCALL mysql_stmt_free_result(MYSQL_STMT *stmt);
bool STDCALL mysql_stmt_send_long_data(MYSQL_STMT *stmt,
                                       unsigned int param_number,
                                       const char *data, unsigned long length);
MYSQL_RES *STDCALL mysql_stmt_result_metadata(MYSQL_STMT *stmt);
MYSQL_RES *STDCALL mysql_stmt_param_metadata(MYSQL_STMT *stmt);
unsigned int STDCALL mysql_stmt_errno(MYSQL_STMT *stmt);
const char *STDCALL mysql_stmt_error(MYSQL_STMT *stmt);
const char *STDCALL mysql_stmt_sqlstate(MYSQL_STMT *stmt);
MYSQL_ROW_OFFSET STDCALL mysql_stmt_row_seek(MYSQL_STMT *stmt,
                                             MYSQL_ROW_OFFSET offset);
MYSQL_ROW_OFFSET STDCALL mysql_stmt_row_tell(MYSQL_STMT *stmt);
void STDCALL mysql_stmt_data_seek(MYSQL_STMT *stmt, uint64_t offset);
uint64_t STDCALL mysql_stmt_num_rows(MYSQL_STMT *stmt);
uint64_t STDCALL mysql_stmt_affected_rows(MYSQL_STMT *stmt);
uint64_t STDCALL mysql_stmt_insert_id(MYSQL_STMT *stmt);
unsigned int STDCALL mysql_stmt_field_count(MYSQL_STMT *stmt);

bool STDCALL mysql_commit(MYSQL *mysql);
bool STDCALL mysql_rollback(MYSQL *mysql);
bool STDCALL mysql_autocommit(MYSQL *mysql, bool auto_mode);
bool STDCALL mysql_more_results(MYSQL *mysql);
int STDCALL mysql_next_result(MYSQL *mysql);
int STDCALL mysql_stmt_next_result(MYSQL_STMT *stmt);
void STDCALL mysql_close(MYSQL *sock);

/* Public key reset */
void STDCALL mysql_reset_server_public_key(void);

/* status return codes */
#define MYSQL_NO_DATA 100
#define MYSQL_DATA_TRUNCATED 101

#define mysql_reload(mysql) mysql_refresh((mysql), REFRESH_GRANT)

#define HAVE_MYSQL_REAL_CONNECT

#ifdef __cplusplus
}
#endif

#endif /* _mysql_h */
