#ifndef SQL_COMMON_INCLUDED
#define SQL_COMMON_INCLUDED

/* Copyright (c) 2003, 2019, Oracle and/or its affiliates. All rights reserved.

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

#define SQL_COMMON_INCLUDED

/**
  @file include/sql_common.h
*/

#include <mysql.h>
#include <stddef.h>
#include <sys/types.h>

#include "errmsg.h"
#include "my_command.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_list.h"
#include "mysql_com.h"

#ifdef MYSQL_SERVER
#include "mysql_com_server.h"
#endif

#ifdef HAVE_OPENSSL
#include <openssl/x509v3.h>
#endif

struct MEM_ROOT;

#ifdef __cplusplus
extern "C" {
#endif

extern const char *unknown_sqlstate;
extern const char *cant_connect_sqlstate;
extern const char *not_error_sqlstate;

/*
  Free all memory allocated in MYSQL handle except the
  current options.
*/
void mysql_close_free(MYSQL *mysql);

/*
  Clear connection options stored in MYSQL handle and
  free memory used by them.
*/
void mysql_close_free_options(MYSQL *mysql);

/**
  The structure is used to hold the state change information
  received from the server. LIST functions are used for manipulation
  of the members of the structure.
*/
struct STATE_INFO_NODE {
  /** head_node->data is a LEX_STRING which contains the variable name. */
  LIST *head_node;
  LIST *current_node;
};

/**
  Store the change info received from the server in an array of linked lists
  with STATE_INFO_NODE elements (one per state type).
*/
struct STATE_INFO {
  /** Array of STATE_NODE_INFO elements (one per state type). */
  STATE_INFO_NODE info_list[SESSION_TRACK_END + 1];
};

/*
  Access to MYSQL::extension member.

  Note: functions mysql_extension_{init,free}() are defined
  in client.c.
*/

struct st_mysql_trace_info;

struct MYSQL_EXTENSION {
  struct st_mysql_trace_info *trace_data;
  STATE_INFO state_change;
  /* Struct to track the state of asynchronous operations */
  struct MYSQL_ASYNC *mysql_async_context;
#ifdef MYSQL_SERVER
  // Used by replication to pass around compression context data.
  NET_SERVER *server_extn;
#endif
};

/* "Constructor/destructor" for MYSQL extension structure. */
MYSQL_EXTENSION *mysql_extension_init(MYSQL *);
void mysql_extension_free(MYSQL_EXTENSION *);
/*
  Note: Allocated extension structure is freed in mysql_close_free()
  called by mysql_close().
*/
#define MYSQL_EXTENSION_PTR(H)              \
  ((MYSQL_EXTENSION *)((H)->extension       \
                           ? (H)->extension \
                           : ((H)->extension = mysql_extension_init(H))))

#define ASYNC_DATA(M) \
  (NULL != (M) ? (MYSQL_EXTENSION_PTR(M)->mysql_async_context) : NULL)
#ifdef MYSQL_SERVER
inline void mysql_extension_set_server_extn(MYSQL *mysql, NET_SERVER *extn) {
  MYSQL_EXTENSION_PTR(mysql)->server_extn = extn;
}
#endif

#ifdef HAVE_OPENSSL
/*
  Signature for the callback function used as MYSQL_OPT_TLS_CERT_CALLBACK
  parameter.

  The callback is responsible for validating server certificate
  passed to it as "server_cert" parameter. An optional callback context
  can be set by the client using MYSQL_OPT_TLS_CERT_CALLBACK_CONTEXT option.
  This context will be passed as "context" parameter to the callback.
  The callback is expected to return "0" if cert validation was successful,
  or to return "1" to indicate cert validation failure. In case of failure
  the callback should update "errptr" with a pointer to the error message
  string.
 */
typedef int (*server_cert_validator_ptr)(X509 *server_cert, const void *context,
                                         const char **errptr);
#endif

struct st_mysql_options_extention {
  char *plugin_dir;
  char *default_auth;
  char *ssl_crl;     /* PEM CRL file */
  char *ssl_crlpath; /* PEM directory of CRL-s? */
  struct My_hash *connection_attributes;
  char *server_public_key_path;
  size_t connection_attributes_length;
  struct My_hash *query_attributes;
  size_t query_attributes_length;
  bool enable_cleartext_plugin;
  bool get_server_public_key; /* Former ssl_enforce */
  char *tls_version;          /* TLS version option */
  long ssl_ctx_flags;         /* SSL ctx options flag */
  unsigned int ssl_mode;
  unsigned int retry_count;
  unsigned int ssl_fips_mode; /* SSL fips mode for enforced encryption.*/
  char *tls_ciphersuites;
  char *compression_algorithm;
  unsigned int total_configured_compression_algorithms;
  unsigned int zstd_compression_level;
  bool connection_compressed;
  char *tls_sni_servername; /* TLS sni server name */
  void *ssl_session;
  void *ssl_context;
  /* pointer to the function validating server certificate */
#ifdef HAVE_OPENSSL
  server_cert_validator_ptr server_cert_validator;
  const void *server_cert_validator_context;
#endif
  unsigned int tos;
};

struct MYSQL_METHODS {
  bool (*read_query_result)(MYSQL *mysql);
  bool (*advanced_command)(MYSQL *mysql, enum enum_server_command command,
                           const unsigned char *header, size_t header_length,
                           const unsigned char *arg, size_t arg_length,
                           bool skip_check, MYSQL_STMT *stmt);
  MYSQL_DATA *(*read_rows)(MYSQL *mysql, MYSQL_FIELD *mysql_fields,
                           unsigned int fields);
  MYSQL_RES *(*use_result)(MYSQL *mysql);
  void (*fetch_lengths)(unsigned long *to, MYSQL_ROW column,
                        unsigned int field_count);
  void (*flush_use_result)(MYSQL *mysql, bool flush_all_results);
  int (*read_change_user_result)(MYSQL *mysql);
#if !defined(MYSQL_SERVER) && !defined(MYSQL_COMPONENT)
  MYSQL_FIELD *(*list_fields)(MYSQL *mysql);
  bool (*read_prepare_result)(MYSQL *mysql, MYSQL_STMT *stmt);
  int (*stmt_execute)(MYSQL_STMT *stmt);
  int (*read_binary_rows)(MYSQL_STMT *stmt);
  int (*unbuffered_fetch)(MYSQL *mysql, char **row);
  const char *(*read_statistics)(MYSQL *mysql);
  bool (*next_result)(MYSQL *mysql);
  int (*read_rows_from_cursor)(MYSQL_STMT *stmt);
  void (*free_rows)(MYSQL_DATA *cur);
#endif
  enum net_async_status (*read_query_result_nonblocking)(MYSQL *mysql);
  enum net_async_status (*advanced_command_nonblocking)(
      MYSQL *mysql, enum enum_server_command command,
      const unsigned char *header, unsigned long header_length,
      const unsigned char *arg, unsigned long arg_length, bool skip_check,
      MYSQL_STMT *stmt, bool *error);
  enum net_async_status (*read_rows_nonblocking)(MYSQL *mysql,
                                                 MYSQL_FIELD *mysql_fields,
                                                 unsigned int fields,
                                                 MYSQL_DATA **result);
  enum net_async_status (*flush_use_result_nonblocking)(MYSQL *mysql,
                                                        bool flush_all_results);
  enum net_async_status (*next_result_nonblocking)(MYSQL *mysql);
  enum net_async_status (*read_change_user_result_nonblocking)(MYSQL *mysql,
                                                               ulong *res);
};

#define simple_command(mysql, command, arg, length, skip_check)              \
  ((mysql)->methods                                                          \
       ? (*(mysql)->methods->advanced_command)(mysql, command, 0, 0, arg,    \
                                               length, skip_check, NULL)     \
       : (set_mysql_error(mysql, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate), \
          1))
#define simple_command_nonblocking(mysql, command, arg, length, skip_check, \
                                   error)                                   \
  (*(mysql)->methods->advanced_command_nonblocking)(                        \
      mysql, command, 0, 0, arg, length, skip_check, NULL, error)

#define stmt_command(mysql, command, arg, length, stmt)                      \
  ((mysql)->methods                                                          \
       ? (*(mysql)->methods->advanced_command)(mysql, command, 0, 0, arg,    \
                                               length, 1, stmt)              \
       : (set_mysql_error(mysql, CR_COMMANDS_OUT_OF_SYNC, unknown_sqlstate), \
          1))

extern CHARSET_INFO *default_client_charset_info;
MYSQL_FIELD *unpack_fields(MYSQL *mysql, MYSQL_ROWS *data, MEM_ROOT *alloc,
                           uint fields, bool default_value,
                           uint server_capabilities);
MYSQL_FIELD *cli_read_metadata_ex(MYSQL *mysql, MEM_ROOT *alloc,
                                  unsigned long field_count,
                                  unsigned int fields);
MYSQL_FIELD *cli_read_metadata(MYSQL *mysql, unsigned long field_count,
                               unsigned int fields);
void free_rows(MYSQL_DATA *cur);
void free_old_query(MYSQL *mysql);
void end_server(MYSQL *mysql);
bool mysql_reconnect(MYSQL *mysql);
void mysql_read_default_options(struct st_mysql_options *options,
                                const char *filename, const char *group);
bool cli_advanced_command(MYSQL *mysql, enum enum_server_command command,
                          const unsigned char *header, size_t header_length,
                          const unsigned char *arg, size_t arg_length,
                          bool skip_check, MYSQL_STMT *stmt);
unsigned long cli_safe_read(MYSQL *mysql, bool *is_data_packet);
enum net_async_status cli_safe_read_nonblocking(MYSQL *mysql,
                                                bool *is_data_packet,
                                                ulong *res);
unsigned long cli_safe_read_with_ok(MYSQL *mysql, bool parse_ok,
                                    bool *is_data_packet);
void net_clear_error(NET *net);
void set_stmt_errmsg(MYSQL_STMT *stmt, NET *net);
void set_stmt_error(MYSQL_STMT *stmt, int errcode, const char *sqlstate,
                    const char *err);
void set_mysql_error(MYSQL *mysql, int errcode, const char *sqlstate);
void set_mysql_extended_error(MYSQL *mysql, int errcode, const char *sqlstate,
                              const char *format, ...)
    MY_ATTRIBUTE((format(printf, 4, 5)));

/* client side of the pluggable authentication */
struct MYSQL_PLUGIN_VIO_INFO;

void mpvio_info(MYSQL_VIO vio, MYSQL_PLUGIN_VIO_INFO *info);
int run_plugin_auth(MYSQL *mysql, char *data, uint data_len,
                    const char *data_plugin, const char *db);
enum net_async_status run_plugin_auth_nonblocking_wrapper(MYSQL *mysql,
                                                          const char *user,
                                                          const char *passwd,
                                                          const char *db);
int mysql_client_plugin_init();
void mysql_client_plugin_deinit();

struct st_mysql_client_plugin;

extern struct st_mysql_client_plugin *mysql_client_builtins[];
uchar *send_client_connect_attrs(MYSQL *mysql, uchar *buf);
extern bool libmysql_cleartext_plugin_enabled;
int is_file_or_dir_world_writable(const char *filepath);
void read_ok_ex(MYSQL *mysql, unsigned long len);

#ifdef __cplusplus
}
#endif

#define protocol_41(A) ((A)->server_capabilities & CLIENT_PROTOCOL_41)

#endif /* SQL_COMMON_INCLUDED */
