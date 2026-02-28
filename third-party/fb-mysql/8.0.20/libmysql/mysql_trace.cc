/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file

  =============================================
   Client-side protocol tracing infrastructure
  =============================================

  If a plugin of type MYSQL_CLIENT_TRACE_PLUGIN is loaded into
  libmysql and its instance is pointed by the global trace_plugin
  pointer, this plugin is notified of various protocol trace events.
  See include/mysql/plugin_trace.h for documentation of trace plugin
  methods and the list of possible trace events.

  These trace events are generated with MYSQL_TRACE() macro put in
  relevant places in libmysql code. The macro calls mysql_trace_trace()
  function defined here. This function calls trace plugin's
  trace_event() method, if it is defined.

  For each traced connection, the state is kept in st_mysql_trace_info
  structure (see mysql_trace.h).

  To correctly interpret each trace event, trace plugin is informed
  of the current protocol stage (see include/mysql/plugin_trace.h for
  list of stages). The current protocol stage is part of the
  connection tracing state. It is updated with MYSQL_TRACE_STAGE()
  hooks within libmysql code.
*/

#include "mysql_trace.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql.h"
#include "mysql/service_mysql_alloc.h"

/*
  Definition of the global trace_plugin pointer - see plugin_trace.h
  for declaration and description.
*/
struct st_mysql_client_plugin_TRACE *trace_plugin = nullptr;

/*
  Macros for manipulating trace_info structure.
*/
#define GET_DATA(TI) (TI)->trace_plugin_data
#define GET_STAGE(TI) (TI)->stage
#define TEST_STAGE(TI, X) (GET_STAGE(TI) == PROTOCOL_STAGE_##X)

/**
  Initialize tracing in a given connection.

  This function is called from MYSQL_TRACE_STAGE() when the initial
  CONNECTING stage is reported. It allocates and initializes trace_info
  structure which is then attached to the connection handle.
*/

void mysql_trace_start(MYSQL *m) {
  struct st_mysql_trace_info *trace_info;

  trace_info = (st_mysql_trace_info *)my_malloc(
      PSI_NOT_INSTRUMENTED, sizeof(struct st_mysql_trace_info),
      MYF(MY_ZEROFILL));
  if (!trace_info) {
    /*
      Note: in this case trace_data of the connection will
      remain NULL and thus tracing will be disabled.
    */
    return;
  }

  /*
    This function should be called only when a trace plugin
    is loaded and thus trace_plugin pointer is not NULL. This
    is handled in MYSQL_TRACE_STAGE() macro (mysql_trace.h).
  */
  DBUG_ASSERT(trace_plugin);

  trace_info->plugin = trace_plugin;
  trace_info->stage = PROTOCOL_STAGE_CONNECTING;

  /*
    Call plugin's tracing_start() method, if defined.
  */

  if (trace_info->plugin->tracing_start) {
    trace_info->trace_plugin_data = trace_info->plugin->tracing_start(
        trace_info->plugin, m, PROTOCOL_STAGE_CONNECTING);
  } else {
    trace_info->trace_plugin_data = nullptr;
  }

  /* Store trace_info in the connection handle. */

  TRACE_DATA(m) = trace_info;
}

/**
  Report a protocol trace event to trace plugin.

  Calls plugin's trace_event() method, if it is defined, passing to
  it the event, the current protocol stage and event arguments (if any).

  Terminates tracing of the connection when appropriate.

  @param m        MYSQL connection handle
  @param ev       trace event to be reported
  @param args     trace event arguments
*/

void mysql_trace_trace(MYSQL *m, enum trace_event ev,
                       struct st_trace_event_args args) {
  struct st_mysql_trace_info *trace_info = TRACE_DATA(m);
  struct st_mysql_client_plugin_TRACE *plugin =
      trace_info ? trace_info->plugin : nullptr;
  int quit_tracing = 0;

  /*
    If trace_info is NULL then this connection is not traced and this
    function should not be called - this is handled inside MYSQL_TRACE()
    macro.
  */
  DBUG_ASSERT(trace_info);

  /* Call plugin's trace_event() method if defined */

  if (plugin->trace_event) {
    /*
      Temporarily disable tracing while executing plugin's method
      by setting trace data pointer to NULL. Also, set reconnect
      flag to 0 in case plugin executes any queries.
    */
    bool saved_reconnect_flag = m->reconnect;

    TRACE_DATA(m) = nullptr;
    m->reconnect = false;
    quit_tracing = plugin->trace_event(plugin, GET_DATA(trace_info), m,
                                       GET_STAGE(trace_info), ev, args);
    m->reconnect = saved_reconnect_flag;
    TRACE_DATA(m) = trace_info;
  }

  /* Stop tracing if requested or end of connection. */

  if (quit_tracing || TEST_STAGE(trace_info, DISCONNECTED) ||
      TRACE_EVENT_DISCONNECTED == ev) {
    /* Note: this disables further tracing */
    TRACE_DATA(m) = nullptr;

    if (plugin->tracing_stop)
      plugin->tracing_stop(plugin, m, GET_DATA(trace_info));

    my_free(trace_info);
  }
}

#ifndef DBUG_OFF
/*
  These functions are declared in plugin_trace.h.

  Consult documentation of *_LIST() macros (plugin_trace.h) to see how
  switch() bodies are constructed with the *_get_name() macros.
*/

#define protocol_stage_get_name(X) \
  case PROTOCOL_STAGE_##X:         \
    return #X;

const char *protocol_stage_name(enum protocol_stage stage) {
  switch (stage) {
    PROTOCOL_STAGE_LIST(get_name)
    default:
      return "<unknown stage>";
  }
}

#define trace_event_get_name(X) \
  case TRACE_EVENT_##X:         \
    return #X;

const char *trace_event_name(enum trace_event ev) {
  switch (ev) {
    TRACE_EVENT_LIST(get_name)
    default:
      return "<unknown event>";
  }
}

#endif
