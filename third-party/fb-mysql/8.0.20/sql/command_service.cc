/*  Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "m_ctype.h"
#include "my_command.h"
#include "mysql/service_command.h"
#include "sql/srv_session.h"

/**
  @file
  Command service implementation. For more information please check
  the function comments.
*/

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
  @param text_or_binary         See enum cs_text_or_binary
  @param service_callbacks_ctx  Context passed to the command service callbacks

  @return
    0 success
    1 failure
*/
int command_service_run_command(Srv_session *session,
                                enum enum_server_command command,
                                const union COM_DATA *data,
                                const CHARSET_INFO *client_cs,
                                const struct st_command_service_cbs *callbacks,
                                enum cs_text_or_binary text_or_binary,
                                void *service_callbacks_ctx) {
  if (!session || !Srv_session::is_valid(session)) return true;

  return session->execute_command(command, data, client_cs, callbacks,
                                  text_or_binary, service_callbacks_ctx);
}
