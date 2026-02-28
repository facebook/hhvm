/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_CLONE_PROTOCOL_SERVICE
#define MYSQL_CLONE_PROTOCOL_SERVICE

/**
  @file
  This service provides functions for clone plugin to
  connect and interact with remote server's clone plugin
  counterpart.

*/

#ifdef __cplusplus
class THD;
#else
#define THD void
#endif

struct MYSQL;
struct MYSQL_SOCKET;

#include <mysql/components/service.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "mysql_com_server.h"

/** Connection parameters including SSL */
struct mysql_clone_ssl_context {
  /** Clone ssl mode. Same as mysql client --ssl-mode */
  int m_ssl_mode;
  /** Clone ssl private key. Same as mysql client --ssl-key */
  const char *m_ssl_key;
  /** Clone ssl certificate. Same as mysql client --ssl-cert */
  const char *m_ssl_cert;
  /** Clone ssl certificate authority. Same as mysql client --ssl-ca */
  const char *m_ssl_ca;

  /** Enable network compression. */
  bool m_enable_compression;
  NET_SERVER *m_server_extn;
};

/** Vector of sting Values */
using Mysql_Clone_Values = std::vector<std::string>;

/** Vector of string Key-Value pairs. */
using Mysql_Clone_Key_Values = std::vector<std::pair<std::string, std::string>>;

BEGIN_SERVICE_DEFINITION(clone_protocol)

/**
  Start and set session and statement key form current thread
  @param[in,out] thd  server session THD
  @param[in]     thread_key  PSI key for thread
  @param[in]     statement_key  PSI Key for statement
*/
DECLARE_METHOD(void, mysql_clone_start_statement,
               (THD * &thd, unsigned int thread_key,
                unsigned int statement_key));

/**
  Finish statement and session
  @param[in,out]  thd    server session THD
*/
DECLARE_METHOD(void, mysql_clone_finish_statement, (THD * thd));

/**
  Get all character set and collations
  @param[in,out]  thd        server session THD
  @param[out]     char_sets  all character set collations
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_get_charsets,
               (THD * thd, Mysql_Clone_Values &char_sets));

/**
  Check if all characters sets are supported by server
  @param[in,out]  thd        server session THD
  @param[in]      char_sets  all character set collations to validate
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_validate_charsets,
               (THD * thd, Mysql_Clone_Values &char_sets));

/**
  Get system configuration parameter values.
  @param[in,out]  thd        server session THD
  @param[in,out]  configs    a list of configuration key value pair
                             keys are input and values are output
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_get_configs,
               (THD * thd, Mysql_Clone_Key_Values &configs));

/**
  Check if configuration parameter values match
  @param[in,out]  thd        server session THD
  @param[in]      configs    a list of configuration key value pair
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_validate_configs,
               (THD * thd, Mysql_Clone_Key_Values &configs));

/**
  Connect to a remote server and switch to clone protocol
  @param[in,out] thd      server session THD
  @param[in]     host     host name to connect to
  @param[in]     port     port number to connect to
  @param[in]     user     user name on remote host
  @param[in]     passwd   password for the user
  @param[in]     ssl_ctx  client ssl context
  @param[out]    socket   Network socket for the connection

  @return Connection object if successful.
*/
DECLARE_METHOD(MYSQL *, mysql_clone_connect,
               (THD * thd, const char *host, uint32_t port, const char *user,
                const char *passwd, mysql_clone_ssl_context *ssl_ctx,
                MYSQL_SOCKET *socket));

/**
  Execute clone command on remote server
  @param[in,out] thd            local session THD
  @param[in,out] connection     connection object
  @param[in]     set_active     set socket active for current THD
  @param[in]     command        remote command
  @param[in]     com_buffer     data following command
  @param[in]     buffer_length  data length
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_send_command,
               (THD * thd, MYSQL *connection, bool set_active,
                unsigned char command, unsigned char *com_buffer,
                size_t buffer_length));

/**
  Get response from remote server
  @param[in,out] thd            local session THD
  @param[in,out] connection     connection object
  @param[in]     set_active     set socket active for current THD
  @param[in]     timeout        timeout in seconds
  @param[out]    packet         response packet
  @param[out]    length         packet length
  @param[out]    net_length     network data length for compressed data
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_get_response,
               (THD * thd, MYSQL *connection, bool set_active, uint32_t timeout,
                unsigned char **packet, size_t *length, size_t *net_length));

/**
  Kill a remote connection
  @param[in,out] connection        connection object
  @param[in]     kill_connection   connection to kill
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_kill,
               (MYSQL * connection, MYSQL *kill_connection));

/**
  Disconnect from a remote server
  @param[in,out] thd         local session THD
  @param[in,out] connection  connection object
  @param[in]     is_fatal    if closing after fatal error
  @param[in]     clear_error clear any earlier error in session
*/
DECLARE_METHOD(void, mysql_clone_disconnect,
               (THD * thd, MYSQL *connection, bool is_fatal, bool clear_error));
/**
  Get error number and message.
  @param[in,out] thd         local session THD
  @param[out]    err_num     error number
  @param[out]    err_mesg    error message text
*/
DECLARE_METHOD(void, mysql_clone_get_error,
               (THD * thd, uint32_t *err_num, const char **err_mesg));

/**
  Get command from client
  @param[in,out] thd            server session THD
  @param[out]    command        remote command
  @param[out]    com_buffer     data following command
  @param[out]    buffer_length  data length
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_get_command,
               (THD * thd, unsigned char *command, unsigned char **com_buffer,
                size_t *buffer_length));

/**
  Send response to client.
  @param[in,out] thd     server session THD
  @param[in]     secure  needs to be sent over secure connection
  @param[in]     packet  response packet
  @param[in]     length  packet length
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_send_response,
               (THD * thd, bool secure, unsigned char *packet, size_t length));

/**
  Send error to client
  @param[in,out] thd           server session THD
  @param[in]     err_cmd       error response command
  @param[in]     is_fatal      if fatal error
  @return error code.
*/
DECLARE_METHOD(int, mysql_clone_send_error,
               (THD * thd, unsigned char err_cmd, bool is_fatal));

END_SERVICE_DEFINITION(clone_protocol)

#endif /* MYSQL_CLONE_PROTOCOL_SERVICE */
