/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_TRACE_INCLUDED
#define MYSQL_TRACE_INCLUDED
/**
  @file

  =====================================================
   Declarations for client-side tracing infrastructure
  =====================================================

  See libmysql/mysql_trace.c for a brief description. Trace plugin
  declarations are in plugin_trace.h header.
*/

#include <stddef.h>

#include "my_macros.h"

/*
  Disable trace hooks if the infrastructure is not enabled
*/
#if !defined(CLIENT_PROTOCOL_TRACING) || defined(MYSQL_SERVER)

#define MYSQL_TRACE(E, M, ARGS)
#define MYSQL_TRACE_STAGE(M, S)

#else

#include <mysql/plugin_trace.h>
#include <stdarg.h>

#include "sql_common.h" /* for MYSQL_EXTENSION() */

/**
  Per connection protocol tracing state

  For each connection which is traced an instance of this structure
  is pointed by the trace_data member of MYSQL_EXTENSION structure
  attached to that connection handle.

  If trace_data is NULL, for an initialized connection, then it means
  that tracing in this connection is disabled.
*/

struct st_mysql_trace_info {
  struct st_mysql_client_plugin_TRACE *plugin;
  void *trace_plugin_data;
  enum protocol_stage stage;
};

#define TRACE_DATA(M) (MYSQL_EXTENSION_PTR(M)->trace_data)

/*
  See libmysql/mysql_trace.c for documentation and implementation of
  these functions.
*/

void mysql_trace_trace(MYSQL *, enum trace_event, struct st_trace_event_args);
void mysql_trace_start(MYSQL *);

/**
  The main protocol tracing hook.

  It is placed in places in the libmysql code where trace events occur.
  If tracing of the connection is not disabled, it calls
  mysql_trace_trace() function to report the event to the
  trace plugin.

  @param E    trace event (without TRACE_EVENT_ prefix)
  @param M    connection handle (pointer to MYSQL structure)
  @param ARGS event specific arguments

  Event arguments are processed with appropriate TRACE_ARGS_* macro
  (see below) to put them inside st_trace_event_args structure.
*/

#define MYSQL_TRACE(E, M, ARGS)                                    \
  do {                                                             \
    if (NULL == TRACE_DATA(M)) break;                              \
    {                                                              \
      struct st_trace_event_args event_args = TRACE_ARGS_##E ARGS; \
      mysql_trace_trace(M, TRACE_EVENT_##E, event_args);           \
    }                                                              \
  } while (0)

/**
  A hook to set the current protocol stage.

  @param M  connection handle (pointer to MYSQL structure)
  @param S  the current stage (without PROTOCOL_STAGE_ prefix)

  If tracing is not disabled, the stage is stored in connection's
  tracing state. A special case is if the current stage is the
  initial CONNECTING one. In that case function mysql_trace_start()
  is called to initialize tracing in this connection, provided that
  a trace plugin is loaded.
*/

#define MYSQL_TRACE_STAGE(M, S)                                 \
  do {                                                          \
    if (TRACE_DATA(M))                                          \
      TRACE_DATA(M)->stage = PROTOCOL_STAGE_##S;                \
    else if (trace_plugin &&                                    \
             (PROTOCOL_STAGE_CONNECTING == PROTOCOL_STAGE_##S)) \
      mysql_trace_start(M);                                     \
  } while (0)

/*
  Macros to parse event arguments and initialize the
  st_trace_event_args structure accordingly. See description of
  the structure in plugin_trace.h.
*/

#define TRACE_ARGS_SEND_SSL_REQUEST(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }
#define TRACE_ARGS_SEND_AUTH_RESPONSE(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }
#define TRACE_ARGS_SEND_AUTH_DATA(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }
#define TRACE_ARGS_AUTH_PLUGIN(PluginName) \
  { PluginName, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_SEND_COMMAND(Command, HdrSize, ArgSize, Header, Args) \
  { NULL, Command, Header, HdrSize, Args, ArgSize }
#define TRACE_ARGS_SEND_FILE(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }

#define TRACE_ARGS_PACKET_SENT(Size) \
  { NULL, 0, NULL, 0, NULL, Size }
#define TRACE_ARGS_PACKET_RECEIVED(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }
#define TRACE_ARGS_INIT_PACKET_RECEIVED(Size, Packet) \
  { NULL, 0, NULL, 0, Packet, Size }

#define TRACE_ARGS_ERROR() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_READ_PACKET() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_CONNECTING() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_CONNECTED() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_DISCONNECTED() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_AUTHENTICATED() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_SSL_CONNECT() \
  { NULL, 0, NULL, 0, NULL, 0 }
#define TRACE_ARGS_SSL_CONNECTED() \
  { NULL, 0, NULL, 0, NULL, 0 }

#endif /* !defined(CLIENT_PROTOCOL_TRACING) || defined(MYSQL_SERVER) */

#endif
