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

#ifndef PLUGIN_TRACE_INCLUDED
#define PLUGIN_TRACE_INCLUDED
/**
  @file include/mysql/plugin_trace.h

  Declarations for client-side plugins of type MYSQL_CLIENT_TRACE_PLUGIN.

  See libmysql/mysql_trace.c for a brief description of the client-side
  protocol tracing infrastructure.
*/

#include <mysql/client_plugin.h>

/*
  Lists of protocol stages and trace events
  =========================================

  These lists are defined with PROTOCOL_STAGE_LIST() and TRACE_EVENT_LIST(),
  respectively. Macros accept a disposition name as an argument.

  For example, to process list of protocol stages using disposition "foo",
  define protocol_stage_foo(Stage) macro and then put

    PROTOCOL_STAGE_LIST(foo)

  in your code. This will expand to sequence of protocol_stage_foo(X)
  macros where X ranges over the list of protocol stages, and these macros
  should generate the actual code. See below how this technique is used
  to generate protocol_stage and trace_events enums.
*/

/**
  Protocol stages
  ---------------

  A client following the MySQL protocol goes through several stages of it. Each
  stage determines what packets can be expected from the server or can be send
  by the client.

  Upon receiving each trace event, trace plugin will be notified of the current
  protocol stage so that it can correctly interpret the event.

  These are the possible protocol stages and the transitions between them.

  .. digraph:: protocol_stages

    CONNECTING -> WAIT_FOR_INIT_PACKET;
    CONNECTING -> DISCONNECTED [ label = "failed connection" ];

    WAIT_FOR_INIT_PACKET -> AUTHENTICATE;
    WAIT_FOR_INIT_PACKET -> SSL_NEGOTIATION -> AUTHENTICATE;

    AUTHENTICATE -> READY_FOR_COMMAND [ label = "accepted" ];
    AUTHENTICATE -> DISCONNECTED [ label = "rejected" ];

    READY_FOR_COMMAND -> DISCONNECTED [ label = "COM_QUIT" ];
    READY_FOR_COMMAND -> AUTHENTICATE [ label="after change user" ];
    READY_FOR_COMMAND -> WAIT_FOR_PACKET
         [ label="wait for a single packet after, e.g., COM_STATISTICS" ];
    READY_FOR_COMMAND -> WAIT_FOR_RESULT;
    READY_FOR_COMMAND -> WAIT_FOR_PS_DESCRIPTION
                                      [ label="after prepare command" ];

    WAIT_FOR_PACKET   -> READY_FOR_COMAND;

    WAIT_FOR_RESULT -> READY_FOR_COMMAND [ label="simple reply" ];
    WAIT_FOR_RESULT -> WAIT_FOR_FIELD_DEF;
    WAIT_FOR_RESULT -> FILE_REQUEST;

    WAIT_FOR_FIELD_DEF -> WAIT_FOR_ROW [ label="in a resultset" ];
    WAIT_FOR_FIELD_DEF -> READY_FOR_COMMAND
                          [ label="after describe table or prepare command" ];

    WAIT_FOR_ROW -> READY_FOR_COMMAND;
    WAIT_FOR_ROW -> WAIT_FOR_RESULT [ label="multi-resultset" ];

    WAIT_FOR_PS_DESCRIPTION -> WAIT_FOR_PARAM_DEF;
    WAIT_FOR_PS_DESCRIPTION -> READY_FOR_COMMAND
                                       [ label="no params and result" ];
    WAIT_FOR_PS_DESCRIPTION -> WAIT_FOR_FIELD_DEF [ label="no params" ];

    WAIT_FOR_PARAM_DEF -> WAIT_FOR_FIELD_DEF;
    WAIT_FOR_PARAM_DEF -> READY_FOR_COMMAND [ label="no result" ];

    FILE_REQUEST -> WAIT_FOR_RESULT [label="when whole file sent"];
*/

#define PROTOCOL_STAGE_LIST(X)                                                \
  protocol_stage_##X(CONNECTING) protocol_stage_##X(WAIT_FOR_INIT_PACKET)     \
      protocol_stage_##X(AUTHENTICATE) protocol_stage_##X(SSL_NEGOTIATION)    \
          protocol_stage_##X(READY_FOR_COMMAND)                               \
              protocol_stage_##X(WAIT_FOR_PACKET)                             \
                  protocol_stage_##X(WAIT_FOR_RESULT)                         \
                      protocol_stage_##X(WAIT_FOR_FIELD_DEF)                  \
                          protocol_stage_##X(WAIT_FOR_ROW)                    \
                              protocol_stage_##X(FILE_REQUEST)                \
                                  protocol_stage_##X(WAIT_FOR_PS_DESCRIPTION) \
                                      protocol_stage_##X(WAIT_FOR_PARAM_DEF)  \
                                          protocol_stage_##X(DISCONNECTED)

/**
  Trace events
  ------------

  The following events are generated during the various stages of the
  client-server conversation.

  ---------------------- -----------------------------------------------------
  Connection events
  ---------------------- -----------------------------------------------------
  CONNECTING             Client is connecting to the server.
  CONNECTED              Physical connection has been established.
  DISCONNECTED           Connection with server was broken.
  ---------------------- -----------------------------------------------------
  SSL events
  ---------------------- -----------------------------------------------------
  SEND_SSL_REQUEST       Client is sending SSL connection request.
  SSL_CONNECT            Client is initiating SSL handshake.
  SSL_CONNECTED          SSL connection has been established.
  ---------------------- -----------------------------------------------------
  Authentication events
  ---------------------- -----------------------------------------------------
  CHALLENGE_RECEIVED     Client received authentication challenge.
  AUTH_PLUGIN            Client selects an authentication plugin to be used
                         in the following authentication exchange.
  SEND_AUTH_RESPONSE     Client sends response to the authentication
                         challenge.
  SEND_AUTH_DATA         Client sends extra authentication data packet.
  AUTHENTICATED          Server has accepted connection.
  ---------------------- -----------------------------------------------------
  Command phase events
  ---------------------- -----------------------------------------------------
  SEND_COMMAND           Client is sending a command to the server.
  SEND_FILE              Client is sending local file contents to the server.
  ---------------------- -----------------------------------------------------
  General events
  ---------------------- -----------------------------------------------------
  READ_PACKET            Client starts waiting for a packet from server.
  PACKET_RECEIVED        A packet from server has been received.
  PACKET_SENT            After successful sending of a packet to the server.
  ERROR                  Client detected an error.
  ---------------------- -----------------------------------------------------
*/

#define TRACE_EVENT_LIST(X)                                                    \
  trace_event_##X(ERROR) trace_event_##X(CONNECTING) trace_event_##X(          \
      CONNECTED) trace_event_##X(DISCONNECTED)                                 \
      trace_event_##X(SEND_SSL_REQUEST) trace_event_##X(SSL_CONNECT)           \
          trace_event_##X(SSL_CONNECTED) trace_event_##X(INIT_PACKET_RECEIVED) \
              trace_event_##X(AUTH_PLUGIN) trace_event_##X(SEND_AUTH_RESPONSE) \
                  trace_event_##X(SEND_AUTH_DATA)                              \
                      trace_event_##X(AUTHENTICATED)                           \
                          trace_event_##X(SEND_COMMAND)                        \
                              trace_event_##X(SEND_FILE)                       \
                                  trace_event_##X(READ_PACKET)                 \
                                      trace_event_##X(PACKET_RECEIVED)         \
                                          trace_event_##X(PACKET_SENT)

/**
  Some trace events have additional arguments. These are stored in
  st_trace_event_args structure. Various events store their arguments in the
  structure as follows. Unused members are set to 0/NULL.

   AUTH_PLUGIN
  ------------- ----------------------------------
   plugin_name  the name of the plugin
  ------------- ----------------------------------

   SEND_COMMAND
  ------------- ----------------------------------
   cmd          the command code
   hdr          pointer to command packet header
   hdr_len      length of the header
   pkt          pointer to command arguments
   pkt_len      length of arguments
  ------------- ----------------------------------

   Other SEND_* and *_RECEIVED events
  ------------- ----------------------------------
   pkt          the data sent or received
   pkt_len      length of the data
  ------------- ----------------------------------

   PACKET_SENT
  ------------- ----------------------------------
   pkt_len      number of bytes sent
  ------------- ----------------------------------
*/

struct st_trace_event_args {
  const char *plugin_name;
  int cmd;
  const unsigned char *hdr;
  size_t hdr_len;
  const unsigned char *pkt;
  size_t pkt_len;
};

/* Definitions of protocol_stage and trace_event enums. */

#define protocol_stage_enum(X) PROTOCOL_STAGE_##X,

enum protocol_stage { PROTOCOL_STAGE_LIST(enum) PROTOCOL_STAGE_LAST };

#define trace_event_enum(X) TRACE_EVENT_##X,

enum trace_event { TRACE_EVENT_LIST(enum) TRACE_EVENT_LAST };

/*
  Trace plugin methods
  ====================
*/

struct st_mysql_client_plugin_TRACE;
struct MYSQL;

/**
  Trace plugin tracing_start() method.

  Called when tracing with this plugin starts on a connection. A trace
  plugin might want to maintain per-connection information. It can
  return a pointer to memory area holding such information. It will be
  stored in a connection handle and passed to other plugin methods.

  @param self   pointer to the plugin instance
  @param connection_handle Session
  @param stage  protocol stage in which tracing has started - currently
                it is always CONNECTING stage.

  @return A pointer to plugin-specific, per-connection data if any.
*/

typedef void *(tracing_start_callback)(
    struct st_mysql_client_plugin_TRACE *self, MYSQL *connection_handle,
    enum protocol_stage stage);

/**
  Trace plugin tracing_stop() method.

  Called when tracing of the connection has ended. If a plugin
  allocated any per-connection resources, it should de-allocate them
  here.

  @param self   pointer to the plugin instance
  @param connection_handle Session
  @param plugin_data pointer to plugin's per-connection data.
*/

typedef void(tracing_stop_callback)(struct st_mysql_client_plugin_TRACE *self,
                                    MYSQL *connection_handle,
                                    void *plugin_data);

/**
  Trace plugin trace_event() method.

  Called when a trace event occurs. Plugin can decide to stop tracing
  this connection by returning non-zero value.

  @param self         pointer to the plugin instance
  @param plugin_data  pointer to plugin's per-connection data
  @param connection_handle Session
  @param stage        current protocol stage
  @param event        the trace event
  @param args         trace event arguments

  @return Non-zero if tracing of the connection should end here.
*/

typedef int(trace_event_handler)(struct st_mysql_client_plugin_TRACE *self,
                                 void *plugin_data, MYSQL *connection_handle,
                                 enum protocol_stage stage,
                                 enum trace_event event,
                                 struct st_trace_event_args args);

struct st_mysql_client_plugin_TRACE {
  MYSQL_CLIENT_PLUGIN_HEADER
  tracing_start_callback *tracing_start;
  tracing_stop_callback *tracing_stop;
  trace_event_handler *trace_event;
};

/**
  The global trace_plugin pointer. If it is not NULL, it points at a
  loaded trace plugin which should be used to trace all connections made
  to the server.
*/
extern struct st_mysql_client_plugin_TRACE *trace_plugin;

#ifndef DBUG_OFF

/*
  Functions for getting names of trace events and protocol
  stages for debugging purposes.
*/
const char *protocol_stage_name(enum protocol_stage stage);
const char *trace_event_name(enum trace_event ev);

#endif

#endif
