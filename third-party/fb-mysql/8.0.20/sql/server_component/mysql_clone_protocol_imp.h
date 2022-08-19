/*  Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef MYSQL_CLONE_PROTOCOL_INCLUDED
#define MYSQL_CLONE_PROTOCOL_INCLUDED

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/clone_protocol_service.h>

#include "mysql.h"
#include "sql/sql_class.h"
#include "sql/sql_thd_internal_api.h"
#include "sql_common.h"

/**
  Start and set session and statement key form current thread
  @param[in,out] thd  server session THD
  @param[in]     thread_key  PSI key for thread
  @param[in]     statement_key  PSI Key for statement
*/
DEFINE_METHOD(void, mysql_clone_start_statement,
              (THD * &thd, PSI_thread_key thread_key,
               PSI_statement_key statement_key));

/**
  Finish statement and session
  @param[in,out]  thd    server session THD
*/
DEFINE_METHOD(void, mysql_clone_finish_statement, (THD * thd));

/**
  Get all character set and collations
  @param[in,out]  thd        server session THD
  @param[out]     char_sets  all character set collations
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_get_charsets,
              (THD * thd, Mysql_Clone_Values &char_sets));

/**
  Check if all characters sets are supported by server
  @param[in,out]  thd        server session THD
  @param[in]      char_sets  all character set collations to validate
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_validate_charsets,
              (THD * thd, Mysql_Clone_Values &char_sets));

/**
  Get system configuration parameter values.
  @param[in,out]  thd        server session THD
  @param[in,out]  configs    a list of configuration key value pair
                             keys are input and values are output
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_get_configs,
              (THD * thd, Mysql_Clone_Key_Values &configs));

/**
  Check if configuration parameter values match
  @param[in,out]  thd        server session THD
  @param[in]      configs    a list of configuration key value pair
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_validate_configs,
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
DEFINE_METHOD(MYSQL *, mysql_clone_connect,
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
DEFINE_METHOD(int, mysql_clone_send_command,
              (THD * thd, MYSQL *connection, bool set_active, uchar command,
               uchar *com_buffer, size_t buffer_length));

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
DEFINE_METHOD(int, mysql_clone_get_response,
              (THD * thd, MYSQL *connection, bool set_active, uint32_t timeout,
               uchar **packet, size_t *length, size_t *net_length));

/**
  Kill a remote connection
  @param[in,out] connection        connection object
  @param[in]     kill_connection   connection to kill
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_kill,
              (MYSQL * connection, MYSQL *kill_connection));

/**
  Disconnect from a remote server
  @param[in,out] thd         local session THD
  @param[in,out] mysql       connection object
  @param[in]     is_fatal    if closing after fatal error
  @param[in]     clear_error clear any earlier error in session
*/
DEFINE_METHOD(void, mysql_clone_disconnect,
              (THD * thd, MYSQL *mysql, bool is_fatal, bool clear_error));

/**
  Get error number and message.
  @param[in,out] thd         local session THD
  @param[out]    err_num     error number
  @param[out]    err_mesg    error message text
*/
DEFINE_METHOD(void, mysql_clone_get_error,
              (THD * thd, uint32_t *err_num, const char **err_mesg));

/**
  Get command from client
  @param[in,out] thd            server session THD
  @param[out]    command        remote command
  @param[out]    com_buffer     data following command
  @param[out]    buffer_length  data length
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_get_command,
              (THD * thd, uchar *command, uchar **com_buffer,
               size_t *buffer_length));

/**
  Send response to client.
  @param[in,out] thd     server session THD
  @param[in]     secure  needs to be sent over secure connection
  @param[in]     packet  response packet
  @param[in]     length  packet length
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_send_response,
              (THD * thd, bool secure, uchar *packet, size_t length));

/**
  Send error to client
  @param[in,out] thd           server session THD
  @param[in]     err_cmd       error response command
  @param[in]     is_fatal      if fatal error
  @return error code.
*/
DEFINE_METHOD(int, mysql_clone_send_error,
              (THD * thd, uchar err_cmd, bool is_fatal));

#endif /* MYSQL_CLONE_PROTOCOL_INCLUDED */
