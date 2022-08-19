/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/conn_handler/init_net_server_extension.h"

#include <stddef.h>

#include "lex_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_psi_config.h"
#include "mysql/components/services/psi_socket_bits.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_idle.h"  // MYSQL_SOCKET_SET_STATE,
#include "mysql/psi/mysql_socket.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql_com.h"
#include "mysql_com_server.h"
// MYSQL_START_IDLE_WAIT
#include "sql/mysqld.h"  // stage_starting
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"  // THD
#include "violite.h"

#ifdef HAVE_PSI_STATEMENT_INTERFACE  // TODO: << nonconformance with
                                     // HAVE_PSI_INTERFACE
PSI_statement_info stmt_info_new_packet;
#endif

static void net_before_header_psi(NET *net MY_ATTRIBUTE((unused)),
                                  void *user_data, size_t /* unused: count */) {
  THD *thd;
  thd = static_cast<THD *>(user_data);
  DBUG_ASSERT(thd != nullptr);

  if (thd->m_server_idle) {
    /*
      The server is IDLE, waiting for the next command.
      Technically, it is a wait on a socket, which may take a long time,
      because the call is blocking.
      Disable the socket instrumentation, to avoid recording a SOCKET event.
      Instead, start explicitly an IDLE event.
    */
    MYSQL_SOCKET_SET_STATE(net->vio->mysql_socket, PSI_SOCKET_STATE_IDLE);
    MYSQL_START_IDLE_WAIT(thd->m_idle_psi, &thd->m_idle_state);
  }
}

static void net_after_header_psi(NET *net MY_ATTRIBUTE((unused)),
                                 void *user_data, size_t /* unused: count */,
                                 bool rc) {
  THD *thd;
  thd = static_cast<THD *>(user_data);
  DBUG_ASSERT(thd != nullptr);

  if (thd->m_server_idle) {
    /*
      The server just got data for a network packet header,
      from the network layer.
      The IDLE event is now complete, since we now have a message to process.
      We need to:
      - start a new STATEMENT event
      - start a new STAGE event, within this statement,
      - start recording SOCKET WAITS events, within this stage.
      The proper order is critical to get events numbered correctly,
      and nested in the proper parent.
    */
    MYSQL_END_IDLE_WAIT(thd->m_idle_psi);

    if (!rc) {
      DBUG_ASSERT(thd->m_statement_psi == nullptr);
      thd->m_statement_psi = MYSQL_START_STATEMENT(
          &thd->m_statement_state, stmt_info_new_packet.m_key, thd->db().str,
          thd->db().length, thd->charset(), nullptr);

      /*
        Starts a new stage in performance schema, if compiled in and enabled.
        Also sets THD::proc_info (used by SHOW PROCESSLIST, column STATE)
      */
      THD_STAGE_INFO(thd, stage_starting);
    }

    /*
      TODO: consider recording a SOCKET event for the bytes just read,
      by also passing count here.
    */
    MYSQL_SOCKET_SET_STATE(net->vio->mysql_socket, PSI_SOCKET_STATE_ACTIVE);
    thd->m_server_idle = false;
  }
}

void init_net_server_extension(THD *thd) {
  /* Start with a clean state for connection events. */
  thd->m_idle_psi = nullptr;
  thd->m_statement_psi = nullptr;
  thd->m_server_idle = false;

  /* Hook up the NET_SERVER callback in the net layer. */
  thd->m_net_server_extension.m_user_data = thd;
  thd->m_net_server_extension.m_before_header = net_before_header_psi;
  thd->m_net_server_extension.m_after_header = net_after_header_psi;
  thd->m_net_server_extension.compress_ctx.algorithm = MYSQL_UNCOMPRESSED;
  /* Activate this private extension for the mysqld server. */
  thd->get_protocol_classic()->get_net()->extension =
      &thd->m_net_server_extension;
}
