/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <openssl/ossl_typ.h>
#include "mysql/plugin_auth_common.h"
#include "mysql_async.h"

/* this is a "superset" of MYSQL_PLUGIN_VIO, in C++ I use inheritance */
struct MCPVIO_EXT {
  int (*read_packet)(MYSQL_PLUGIN_VIO *vio, uchar **buf);
  int (*write_packet)(MYSQL_PLUGIN_VIO *vio, const uchar *pkt, int pkt_len);
  void (*info)(MYSQL_PLUGIN_VIO *vio, MYSQL_PLUGIN_VIO_INFO *info);
  net_async_status (*read_packet_nonblocking)(struct MYSQL_PLUGIN_VIO *vio,
                                              unsigned char **buf, int *result);
  net_async_status (*write_packet_nonblocking)(struct MYSQL_PLUGIN_VIO *vio,
                                               const unsigned char *pkt,
                                               int pkt_len, int *result);

  /* -= end of MYSQL_PLUGIN_VIO =- */
  MYSQL *mysql;
  auth_plugin_t *plugin; /**< what plugin we're under */
  const char *db;
  struct {
    uchar *pkt; /**< pointer into NET::buff */
    uint pkt_len;
  } cached_server_reply;
  int packets_read, packets_written; /**< counters for send/received packets */
  int mysql_change_user;             /**< if it's mysql_change_user() */
  int last_read_packet_len; /**< the length of the last *read* packet */
};

/* Our state machines have four simple return codes: */
enum mysql_state_machine_status {
  STATE_MACHINE_FAILED,      /* Completion with a failure. */
  STATE_MACHINE_CONTINUE,    /* Keep calling the state machine. */
  STATE_MACHINE_WOULD_BLOCK, /* Needs to block to continue. */
  STATE_MACHINE_DONE         /* Completion with a success. */
};

/* state machine for native password autheintication API */
enum client_auth_native_password_plugin_status {
  NATIVE_READING_PASSWORD = 1,
  NATIVE_WRITING_RESPONSE
};

enum client_auth_sha256_password_plugin_status {
  SHA256_READING_PASSWORD = 1,
  SHA256_REQUEST_PUBLIC_KEY,
  SHA256_READ_PUBLIC_KEY,
  SHA256_SEND_ENCRYPTED_PASSWORD,
  SHA256_SEND_PLAIN_PASSWORD
};

enum client_auth_caching_sha2_password_plugin_status {
  CACHING_SHA2_READING_PASSWORD = 1,
  CACHING_SHA2_WRITING_RESPONSE,
  CACHING_SHA2_CHALLENGE_RESPONSE,
  CACHING_SHA2_REQUEST_PUBLIC_KEY,
  CACHING_SHA2_READ_PUBLIC_KEY,
  CACHING_SHA2_SEND_ENCRYPTED_PASSWORD,
  CACHING_SHA2_SEND_PLAIN_PASSWORD
};

/* A state machine for authentication itself. */
struct mysql_async_auth;
typedef mysql_state_machine_status (*authsm_function)(mysql_async_auth *);

struct mysql_async_auth {
  MYSQL *mysql;
  bool non_blocking;

  char *data;
  uint data_len;
  const char *data_plugin;
  const char *db;

  const char *auth_plugin_name;
  auth_plugin_t *auth_plugin;
  MCPVIO_EXT mpvio;
  ulong pkt_length;
  int res;

  char *change_user_buff;
  int change_user_buff_len;

  int client_auth_plugin_state;
  authsm_function state_function;

  // to be used for mysql_change_user_nonblocking
  CHARSET_INFO *saved_cs;
  char *saved_user;
  char *saved_passwd;
  char *saved_db;
};

/*
  Connection is handled with a state machine.  Each state is
  represented by a function pointer (csm_function) which returns
  a mysql_state_machine_status to indicate the state of the
  connection.
  This state machine has boundaries around network IO to allow
  reuse between blocking and non-blocking clients.
*/
struct mysql_async_connect;
typedef mysql_state_machine_status (*csm_function)(mysql_async_connect *);

/*
  define different states of an asynchronous SSL connection phase
*/
enum ssl_exchange_state {
  SSL_REQUEST = 8100,
  SSL_CONNECT = 8101,
  SSL_COMPLETE = 8102,
  SSL_NONE = 8103
};

/*
  Struct to track the state of a connection being established.  Once
  the connection is established, the context should be discarded and
  relevant values copied out of it.
*/
struct mysql_async_connect {
  /* state for the overall connection process */
  MYSQL *mysql;
  const char *host;
  const char *user;
  const char *passwd;
  const char *db;
  uint port;
  const char *unix_socket;
  ulong client_flag;
  bool non_blocking;

  ulong pkt_length;
  char *host_info;
  char buff[NAME_LEN + USERNAME_LENGTH + 100];
  int scramble_data_len;
  char *scramble_data;
  const char *scramble_plugin;
  char *scramble_buffer;
  bool scramble_buffer_allocated;

  /* context needed to establish asynchronous authentication */
  struct mysql_async_auth *auth_context;
  /* state for running init_commands */
  bool saved_reconnect;
  char **current_init_command;

  ssl_exchange_state ssl_state;
  SSL *ssl;
  /* state function that will be called next */
  csm_function state_function;
};
