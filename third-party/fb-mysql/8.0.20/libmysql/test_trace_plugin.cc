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

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#if defined(DBUG_OFF)
// This code can be used only in debug builds.
#error You cannot use test trace plugin when DBUG_OFF is defined. \
       Test trace plugin will work in debug builds only.
#else

/**
  @file

  ===========================
   Test trace plugin
  ===========================

  If WITH_TEST_TRACE_PLUGIN build option was checked when building libmysql
  then this plugin is built into client library (no plugin loading required).

  The plugin follows the protocol flow based on the trace events reported to
  it. If MYSQL_TEST_TRACE_DEBUG environment variable is non-zero then it logs
  information about received trace events and reports if a particular event
  is invalid.

  If MYSQL_TEST_TRACE_CRASH environment variable is set to something else than 0
  then the plugin will crash if an invalid trace event was reported to it.
*/

#include <ctype.h>  /* isprint() */
#include <stdio.h>  /* snprintf() */
#include <string.h> /* memset() */

#include "my_byteorder.h"
#include "my_dbug.h"
#include "mysql.h"
#include "mysql_trace.h"

/*
  Definition of the plugin
  ========================

  Whole implementation of the test trace plugin is inside test_trace namespace
  to avoid conflicts with symbols used by application linked to the
  client library. The only external symbol defined here is the test_trace_plugin
  plugin structure.
*/

namespace test_trace {

extern "C" {

static tracing_start_callback trace_start;
static tracing_stop_callback trace_stop;
static trace_event_handler trace_event;

static int plugin_init(char *a, size_t b, int argc, va_list args);
static int plugin_deinit();
}

}  // namespace test_trace

extern "C" {

/*
  Test_trace_plugin symbol is to be used by C code (client.c) and thus it
  is declared inside extern "C" to avoid C++ name mangling problems. Current
  C++ compilers do not mangle data symbol names but to be on the safe side
  we explicitly declare it as extern "C".
*/

struct st_mysql_client_plugin_TRACE test_trace_plugin = {
    MYSQL_CLIENT_TRACE_PLUGIN,
    MYSQL_CLIENT_TRACE_PLUGIN_INTERFACE_VERSION,
    "test_trace_plugin",
    "Rafal Somla",
    "A plugin for testing protocol tracing infrastructure",
    {0, 1, 0},
    "GPL",
    NULL,
    test_trace::plugin_init,
    test_trace::plugin_deinit,
    NULL,  // option handling
    test_trace::trace_start,
    test_trace::trace_stop,
    test_trace::trace_event};
}

namespace test_trace {

typedef unsigned char byte;

/**
  State information maintained by the test trace plugin for each
  traced connection.

  ----------- ---------------------------------------------------
  last_cmd    last command reported with SEND_COMMAND event
  next_stage  expected next protocol stage
  col_count   the number of columns in the result-set of the last
              statement which was prepared with COM_STMT_PREPARE
  multi_resultset  true if a multi result-set was detected
  errnum      error number of the last error reported with ERROR
              event
  ----------- ---------------------------------------------------
*/

struct st_trace_data {
  int last_cmd;
  enum protocol_stage next_stage;
  unsigned int param_count;
  unsigned int col_count;
  bool multi_resultset;
  int errnum;
};

/*
  Logging for test trace plugin
  =============================

  Usage:

  {
    LOGGER(mysql);   // declare logger for given connection - without this
                     // declaration a global, connection-less logger will be
  used
    ...
    LOG(("format string", arg1, ..., argN));  // log using logger declared above
    ...
    DUMP("some data", buf, sizeof(buf));      // dump binary data to the log
    ...
  }

  Normally logger outputs messages on stderr. This can be disabled by setting
  logger_enabled flag to false. The flag is initialized based on
  MYSQL_TRACE_DEBUG env. variable (see plugin_init() below).
*/

static bool logger_enabled = false;

#define LOGGER(M) ::test_trace::Logger logger(M);
#define LOG(ARGS)    \
  do {               \
    logger.log ARGS; \
  } while (0)
#define DUMP(KEY, DATA, LEN)           \
  do {                                 \
    logger.dump((KEY), (DATA), (LEN)); \
  } while (0)

/*
  Logger class
  ------------
*/

class Logger {
  unsigned long connection_id;
  char buffer[1024];
  char *end;
  size_t header();
  void send();

 public:
  Logger() : connection_id(0), end(buffer) {}
  Logger(MYSQL *conn);
  void log(const char *format, ...) MY_ATTRIBUTE((format(printf, 2, 3)));
  void dump(const char *key, const void *data, size_t len);
};

Logger::Logger(MYSQL *conn) : end(buffer) {
  connection_id = mysql_thread_id(conn);
};

size_t Logger::header() {
  size_t len = 0;

  len = snprintf(buffer, sizeof(buffer), "test_trace: ");
  end = buffer + len;

  if (connection_id) {
    len += snprintf(end, sizeof(buffer) - len, "%03zu: ", connection_id);
    end = buffer + len;
  }

  return len;
}

void Logger::log(const char *format, ...) {
  size_t len = header();
  va_list args;

  va_start(args, format);
  end += vsnprintf(end, sizeof(buffer) - len, format, args);
  va_end(args);

  send();
}

void Logger::dump(const char *key, const void *data, size_t data_len) {
  size_t len = header();
  const unsigned char *ptr = static_cast<const unsigned char *>(data);

  end += snprintf(end, sizeof(buffer) - len, "%s: %lu bytes", key, data_len);

  /*
    Dump max 2 rows with 16 bytes each. The dump format is like this:

      03 64 65 66 00 00 00 06  75 73 65 72 28 29 00 0C   .def....user()..
      08 00 4D 00 00 00 FD 00  00 1F 00 00               ..M.........
  */
  for (int row = 0; row < 2; ++row) {
    *(end++) = '\n';
    *(end++) = ' ';

    /*
      Char_disp points at the next location in the right pane, where
      characters are displayed.
    */
    char *char_disp = end + 2 * 8 * 3 + 4;

    // Clear with spaces to avoid random data.
    memset(end, ' ', char_disp - end);

    // We print in 2 columns with extra space to separate them.

    for (int col = 0; col < 2; ++col) {
      for (int pos = 0; pos < 8; ++pos) {
        if (0 == data_len) {
          /*
            Wipe-out '\0' terminator put there by snprintf()
            and make sure end points past the last character in
            the output.
          */
          *end = ' ';
          end = char_disp;
          goto done;
        }

        end += snprintf(end, 200, " %02X", *ptr);
        *(char_disp++) = isprint(*ptr) ? *ptr : '.';

        ++ptr;
        --data_len;
      }

      // Add column separator.

      *(end++) = ' ';
    }

    *end = ' ';
    end = char_disp;
  }

done:

  send();
}

void Logger::send() {
  if (!logger_enabled) return;

  /* Make sure there is space for terminating "\n\0" */
  if (buffer + sizeof(buffer) - end < 2) end = buffer + sizeof(buffer) - 2;

  end[0] = '\n';
  end[1] = '\0';
  fputs(buffer, stderr);
  fflush(stderr);
}

/**
  The "top-level" logger used when no connection context is given.
*/
static Logger logger;

/*
   Plugin initialization and de-initialization
  =============================================
*/

/**
  Global flag telling if test trace plugin should crash if it detects
  incorrect protocol flow (unexpected stage or invalid event for the
  current stage).

  This can be used for detecting invalid protocol trace events in
  MTR tests, for example. The flag is set from environment variable
  MYSQL_TEST_TRACE_CRASH upon plugin initialization. By default it
  is disabled.
*/
static bool opt_crash = false;

int plugin_init(char *, size_t, int, va_list) {
  const char *opt = getenv("MYSQL_TEST_TRACE_CRASH");

  if (opt && '0' != *opt) opt_crash = true;

  opt = getenv("MYSQL_TEST_TRACE_DEBUG");

  if (opt && '0' != *opt) logger_enabled = true;

  LOG(("Test trace plugin initialized"));
  return 0;
}

int plugin_deinit() {
  LOG(("Test trace plugin de-initialized"));
  return 0;
}

/*
   Handling of trace events
  ==========================
*/

#define OK_PACKET(PKT) (*((const byte *)PKT) == 0x00)
#define ERR_PACKET(PKT) (*((const byte *)PKT) == 0xFF)
#define EOF_PACKET(PKT) (*((const byte *)PKT) == 0xFE)
#define FILE_REQUEST(PKT) (*((const byte *)PKT) == 0xFB)

void *trace_start(struct st_mysql_client_plugin_TRACE *, MYSQL *conn,
                  enum protocol_stage stage) {
  LOGGER(conn);
  LOG(("Starting tracing in stage %s", protocol_stage_name(stage)));

  struct st_trace_data *plugin_data = new st_trace_data;

  if (plugin_data) {
    memset(plugin_data, 0, sizeof(st_trace_data));
    plugin_data->next_stage = PROTOCOL_STAGE_CONNECTING;
  } else {
    LOG(
        ("Could not allocate per-connection trace data"
         " - checking protocol flow will be disabled"));
  }
  return plugin_data;
}

void trace_stop(struct st_mysql_client_plugin_TRACE *, MYSQL *conn,
                void *data) {
  LOGGER(conn);
  LOG(("Tracing connection has ended"));
  if (data) delete static_cast<st_trace_data *>(data);
}

/*
  Declare functions for checking protocol flow (defined below). See
  plugin_trace.h for description how the trick with PROTOCOL_STAGE_LIST()
  macro works.
*/

#define protocol_stage_chk_ev_declare(S)                            \
  static int check_event_##S(                                       \
      MYSQL *conn, struct st_trace_data *data, enum trace_event ev, \
      struct st_trace_event_args args, enum protocol_stage *next_stage);

PROTOCOL_STAGE_LIST(chk_ev_declare)

/*
  Main plugin method called when a trace event is detected.
*/

int trace_event(struct st_mysql_client_plugin_TRACE *, void *data_ptr,
                MYSQL *conn, enum protocol_stage stage, enum trace_event ev,
                struct st_trace_event_args args) {
  LOGGER(conn);
  int check = 0;
  struct st_trace_data *data = static_cast<st_trace_data *>(data_ptr);

  LOG(("stage: %s, event: %s", protocol_stage_name(stage),
       trace_event_name(ev)));

  if (ev == TRACE_EVENT_DISCONNECTED) {
    LOG(("Connection  closed"));
    return 1;
  }
  /*
    Check if current protocol stage is as expected. The expected
    protocol stage is kept in data->next_stage. Check that it equals
    the stage reported here, with some exceptions.
  */

  switch (ev) {
    case TRACE_EVENT_SEND_COMMAND:
      /*
        Allow SEND_COMMAND to appear in any protocol stage -
        a client can interrupt whatever is being done now
        and send new command to the server.
      */
      if (data) {
        data->next_stage = PROTOCOL_STAGE_READY_FOR_COMMAND;
        data->errnum = 0;
      }
      break;

    case TRACE_EVENT_ERROR:
    case TRACE_EVENT_AUTH_PLUGIN:
      /*
        Test that current protocol stage is as expected. This check
        is disabled for the AUTH_PLUGIN event which, due to implementation
        details, can happen in any stage before COM_CHANGE_USER command is
        sent to the server.
      */
      break;

    default:
      /*
        Skip protocol stage checks if error was reported during last command
        and we are waiting for new one.
      */
      if (data && data->last_cmd && data->errnum) break;

      if (data && data->next_stage != stage) {
        LOG(("wrong stage, expected: %s",
             protocol_stage_name(data->next_stage)));
        if (opt_crash) DBUG_ASSERT(0);
      }
  }

  // Show trace event details

  switch (ev) {
    case TRACE_EVENT_ERROR: {
      int errnum = conn->net.last_errno;
      const char *error = conn->net.last_error;
      LOG(("error %d: %s", errnum, error));
      if (data) data->errnum = errnum;
      break;
    }

    case TRACE_EVENT_SEND_COMMAND:

      if (data) {
        data->multi_resultset = false;
        data->errnum = 0;
      }

      switch (args.cmd) {
        case COM_BINLOG_DUMP:
        case COM_BINLOG_DUMP_GTID:
        case COM_TABLE_DUMP:
          LOG(("Replication command (%d) - disabling further tracing",
               args.cmd));
          return 1;

        case COM_QUERY:
          LOG(("QUERY: %s", args.pkt));
          break;

        case COM_STMT_PREPARE:
          LOG(("STMT_PREPARE: %s", args.pkt));
          break;

        case COM_STMT_EXECUTE:
          LOG(("STMT_EXECUTE: %d", uint4korr(args.hdr)));
          break;

        case COM_STMT_RESET:
          LOG(("STMT_RESET: %d", uint4korr(args.hdr)));
          break;

        case COM_STMT_CLOSE:
          LOG(("STMT_CLOSE: %d", uint4korr(args.pkt)));
          break;

        case COM_CHANGE_USER:
          LOG(("CHANGE_USER: %s", args.hdr));
          break;

        case COM_QUIT:
          LOG(("QUIT"));
          break;

        default:
          LOG(("cmd: %d", args.cmd));
          if (args.hdr_len > 0) DUMP("cmd hdr", args.hdr, args.hdr_len);
          if (args.pkt_len > 0) DUMP("cmd args", args.pkt, args.pkt_len);
          break;
      };
      break;

    case TRACE_EVENT_AUTH_PLUGIN:
      LOG(("Using authentication plugin: %s", args.plugin_name));
      break;

    case TRACE_EVENT_SEND_SSL_REQUEST:
    case TRACE_EVENT_SEND_AUTH_RESPONSE:
      // TODO: Parse response and show info.
    case TRACE_EVENT_SEND_AUTH_DATA:
    case TRACE_EVENT_SEND_FILE:
      DUMP("sending packet", args.pkt, args.pkt_len);
      break;

    case TRACE_EVENT_PACKET_SENT:
      LOG(("packet sent: %zu bytes", args.pkt_len));

    case TRACE_EVENT_INIT_PACKET_RECEIVED:
      // TODO: Parse init packet and show info.
      break;

    case TRACE_EVENT_PACKET_RECEIVED:
      if (ERR_PACKET(args.pkt)) {
        const byte *pkt = args.pkt;
        unsigned int err_code = uint2korr(pkt + 1);
        pkt += 3;

        if ('#' == *pkt) {
          LOG(("Server error %d (%.5s): %.*s", err_code, pkt + 1,
               int(args.pkt_len - 9), pkt + 6));
        } else {
          LOG(("Server error %d: %.*s", err_code, int(args.pkt_len - 3), pkt));
        }
      } else
        DUMP("packet received", args.pkt, args.pkt_len);
      break;

    default:
      break;
  }

  /*
    Use the check_event_XXX() functions to check if the event is valid in the
    current stage and also update data->next_stage. The result of the check is
    stored in check variable, which is set to true if event is invalid.

    These checks are skipped if trace data is NULL. Also if error was reported
    during last command execution - in this case test trace plugin will wait for
    next client command before resuming checking further events.

    See plugin_trace.h for description how the trick with PROTOCOL_STAGE_LIST()
    macro works.
  */

  if (!data || (data->last_cmd && data->errnum)) return 0;

#define protocol_stage_check(S)                                         \
  case PROTOCOL_STAGE_##S:                                              \
    check = check_event_##S(conn, data, ev, args, &(data->next_stage)); \
    break;

  switch (stage) {
    PROTOCOL_STAGE_LIST(check)
    default:
      LOG(("invalid stage %d", stage));
      if (opt_crash) DBUG_ASSERT(0);
  }

  // Disable invalid event check in certain cases.

  switch (ev) {
    case TRACE_EVENT_SEND_COMMAND:
      /*
        Sending COM_QUIT is never an invalid event as it can happen at any stage
        of the protocol.
      */
      if (COM_QUIT == args.cmd) check = false;
      break;

    case TRACE_EVENT_ERROR:
    case TRACE_EVENT_AUTH_PLUGIN:
      check = false;
      break;

    default:
      break;
  }

  // Report invalid event if detected.

  if (check) {
    LOG(("invalid event detected"));
    if (opt_crash) DBUG_ASSERT(0);
  }

  return 0;
}

/*
  Checking validity of events and tracing protocol stage changes
  ==============================================================

  For each stage XXX function check_event_XXX() checks if given event is
  valid in stage XXX and also computes the next expected protocol stage
  which is stored in next_stage parameter.

  Functions check_event_XXX() return 0 if event is valid and next stage was
  computed, non-zero otherwise.
*/

#define NEXT_STAGE(S)                                 \
  do {                                                \
    if (next_stage) *next_stage = PROTOCOL_STAGE_##S; \
  } while (0)

int check_event_CONNECTING(MYSQL *, struct st_trace_data *, enum trace_event ev,
                           struct st_trace_event_args,
                           enum protocol_stage *next_stage) {
  /*
    This is the first stage of the protocol, when client is establishing
    physical connection with the server. After connection is established
    protocol moves to WAIT_FOR_CHALLENGE stage where the first
    authentication challenge packet is expected from the server.
  */
  switch (ev) {
    case TRACE_EVENT_CONNECTING:
      return 0;
    case TRACE_EVENT_CONNECTED:
      NEXT_STAGE(WAIT_FOR_INIT_PACKET);
      return 0;
    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_INIT_PACKET(MYSQL *, struct st_trace_data *,
                                     enum trace_event ev,
                                     struct st_trace_event_args,
                                     enum protocol_stage *next_stage) {
  /*
    In this stage client waits for the first challenge packet from the
    server. When it is received, protocol moves to AUTHENTICATE stage
    where client authorizes itself against server.
  */
  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED:
      return 0;
    case TRACE_EVENT_INIT_PACKET_RECEIVED:
      NEXT_STAGE(AUTHENTICATE);
      return 0;
    default:
      return 1; /* invalid event */
  };
}

int check_event_AUTHENTICATE(MYSQL *conn, struct st_trace_data *data,
                             enum trace_event ev,
                             struct st_trace_event_args args,
                             enum protocol_stage *next_stage) {
  /*
    In this stage client exchanges various packets with the server to:
    - authenticate itself against the server,
    - negotiate client/server capabilities and connection parameters,
    - establish encrypted SSL connection.
    At any moment during this exchange server can send an ERR packet
    indicating that it rejects the connection.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_AUTHENTICATED:
      NEXT_STAGE(READY_FOR_COMMAND);
      return 0;

    case TRACE_EVENT_PACKET_RECEIVED: {
      /*
        Move to DISCONNECTED stage if ERR packet was received from the
        server.
      */
      if (ERR_PACKET(args.pkt)) {
        LOG(("Server rejected connection (ERR packet)"));
        if (COM_CHANGE_USER == data->last_cmd)
          NEXT_STAGE(READY_FOR_COMMAND);
        else
          NEXT_STAGE(DISCONNECTED);
      }

      return 0;
    }

    case TRACE_EVENT_SEND_SSL_REQUEST:
      NEXT_STAGE(SSL_NEGOTIATION);
      return 0;

    case TRACE_EVENT_AUTH_PLUGIN:
    case TRACE_EVENT_SEND_AUTH_RESPONSE:
    case TRACE_EVENT_SEND_AUTH_DATA:
    case TRACE_EVENT_READ_PACKET:
    case TRACE_EVENT_PACKET_SENT:
      return 0;

    default:
      return 1; /* invalid event */
  };
}

int check_event_SSL_NEGOTIATION(MYSQL *conn, struct st_trace_data *,
                                enum trace_event ev, struct st_trace_event_args,
                                enum protocol_stage *next_stage) {
  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_PACKET_SENT:
    case TRACE_EVENT_PACKET_RECEIVED:
    case TRACE_EVENT_SSL_CONNECT:
      return 0;

    case TRACE_EVENT_SSL_CONNECTED:
      NEXT_STAGE(AUTHENTICATE);
      return 0;

    default:
      return 1; /* invalid event */
  }
}

int check_event_READY_FOR_COMMAND(MYSQL *conn, struct st_trace_data *data,
                                  enum trace_event ev,
                                  struct st_trace_event_args args,
                                  enum protocol_stage *next_stage) {
  /*
    This is the stage when client can send a command to the server.
    After sending command packet to the server, PACKET_SENT trace
    event happens. The next stage depends on whether a result set
    is expected after the command, or just OK/ERR reply or it is
    a command with no reply from server.
  */

  LOGGER(conn);

  /*
    Reset multi_resultset flag in case it was set when processing
    previous query.
  */
  data->multi_resultset = false;

  switch (ev) {
    case TRACE_EVENT_SEND_COMMAND:
      /*
        Save the command code (to be examined in the following
        PACKET_SENT event) and reset PS information stored in
        trace info record
      */
      if (data) data->last_cmd = args.cmd;

      /*
        Replication commands are not supported - report them as
        invalid event.
      */
      switch (args.cmd) {
        case COM_BINLOG_DUMP:
        case COM_BINLOG_DUMP_GTID:
        case COM_TABLE_DUMP:
          return 1;

        default:
          return 0;
      }

    case TRACE_EVENT_PACKET_SENT:
      /*
        Move to correct stage based on the command that was
        reported in the preceding SEND_COMMAND event.
      */
      if (!data) return 1;
      switch (data->last_cmd) {
        case COM_STMT_PREPARE:
          NEXT_STAGE(WAIT_FOR_PS_DESCRIPTION);
          break;

        case COM_QUERY:
          NEXT_STAGE(WAIT_FOR_RESULT);
          break;

        case COM_STMT_EXECUTE:
          /*
            Result of COM_STMT_EXECUTE is always followed by OK packet. We set
            multi_resultset flag to correctly expect the OK packet.
          */
          data->multi_resultset = true;
          NEXT_STAGE(WAIT_FOR_RESULT);
          break;

        case COM_STMT_FETCH:
          NEXT_STAGE(WAIT_FOR_ROW);
          return 0;

        /*
           No server reply is expected after these commands so we remain ready
           for the next command.
        */
        case COM_QUIT:
        case COM_STMT_SEND_LONG_DATA:
        case COM_STMT_CLOSE:
        case COM_REGISTER_SLAVE:
          return 0;

        /*
          These replication commands are not supported (the SEND_COMMAND
          event was already reported as invalid).
        */
        case COM_BINLOG_DUMP:
        case COM_BINLOG_DUMP_GTID:
        case COM_TABLE_DUMP:
          return 1;

        /*
          After COM_PROCESS_INFO server sends a regular result set.
        */
        case COM_PROCESS_INFO:
          NEXT_STAGE(WAIT_FOR_RESULT);
          break;

        /*
          After COM_FIELD_LIST server directly sends field descriptions.
        */
        case COM_FIELD_LIST:
          NEXT_STAGE(WAIT_FOR_FIELD_DEF);
          break;

        /*
          Server replies to COM_STATISTICS with a single packet
          containing a string with statistics information.
        */
        case COM_STATISTICS:
          NEXT_STAGE(WAIT_FOR_PACKET);
          break;

        /*
          After COM_CHANGE_USER a regular authentication exchange
          is performed.
        */
        case COM_CHANGE_USER:
          NEXT_STAGE(AUTHENTICATE);
          break;

        /*
          For all other commands a simple OK/ERR reply from server
          is expected.
        */
        default:
          NEXT_STAGE(WAIT_FOR_RESULT);
          break;
      };
      return 0;

    case TRACE_EVENT_AUTH_PLUGIN:
      /*
        The AUTH_PLUGIN event can happen in READY_FOR_COMMAND stage
        before COM_CHANGE_USER is executed. When the command is executed
        the stage will be changed to AUTHENTICATE.
      */
      return 0;

    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_PACKET(MYSQL *conn, struct st_trace_data *,
                                enum trace_event ev, struct st_trace_event_args,
                                enum protocol_stage *next_stage) {
  /*
    After some commands (COM_STATISTICS) server sends just a
    single packet and then it is ready for processing more
    commands.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED:
      NEXT_STAGE(READY_FOR_COMMAND);
      return 0;
    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_RESULT(MYSQL *conn, struct st_trace_data *data,
                                enum trace_event ev,
                                struct st_trace_event_args args,
                                enum protocol_stage *next_stage) {
  /*
    This stage is reached after a command which can produce a result set
    (COM_QUERY or COM_STMT_EXECUTE). In this stage a single packet is
    received from the server informing about the result of the query:
    - ERR packet informs that error has happened when processing query,
    - OK packet informs that query produced no result set,
    - FILE_REQUEST packet asks client to send contents of a given file,
    Otherwise information about the number of columns in the result set
    is received.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED: {
      byte *pkt = const_cast<byte *>(args.pkt);
      /*
        If server sends ERR, query processing is done
        and next stage is READY_FOR_COMMAND.
      */
      if (ERR_PACKET(pkt) || EOF_PACKET(pkt)) {
        NEXT_STAGE(READY_FOR_COMMAND);
        return 0;
      }

      /*
        If server sends FILE_REQUEST packet, we move to FILE_REQUEST
        stage of the protocol.
      */
      if (FILE_REQUEST(pkt)) {
        LOG(("Server requests file: %s", pkt + 1));
        NEXT_STAGE(FILE_REQUEST);
        return 0;
      }

      /*
        If query generates no data, server replies with OK packet. In
        case of a multi-query, more result sets can follow, as
        indicated by SERVER_MORE_RESULTS_EXISTS flag in the OK packet.
        If the flag is set we move to WAIT_FOR_RESULTSET stage to
        read the following result set(s).
      */
      if (OK_PACKET(args.pkt)) {
        /* Extract flags from the OK packet */
        unsigned int flags;
        unsigned int affected_rows;
        unsigned int last_insert_id;

        pkt++;                                   /* cmd header */
        affected_rows = net_field_length(&pkt);  /* affected_rows */
        last_insert_id = net_field_length(&pkt); /* last_insert_id */
        flags = uint2korr(pkt);

        LOG(
            ("OK from server; flags: 0x%02X, affected rows: %u, last "
             "insert_id: %u",
             flags, affected_rows, last_insert_id));

        if (SERVER_MORE_RESULTS_EXISTS & flags) {
          LOG(("Expecting next result set"));
          NEXT_STAGE(WAIT_FOR_RESULT);
        } else
          NEXT_STAGE(READY_FOR_COMMAND);
        return 0;
      }

      /*
        Otherwise a result set is expected from the server so we move
        to WAIT_FOR_FIELD_DEF stage.
      */
      /* update col_count so that it is used in next stage */
      data->col_count = net_field_length(&pkt);
      LOG(("Expecting result set with %u columns", data->col_count));
      NEXT_STAGE(WAIT_FOR_FIELD_DEF);
      return 0;
    }
    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_FIELD_DEF(MYSQL *conn, struct st_trace_data *data,
                                   enum trace_event ev,
                                   struct st_trace_event_args args,
                                   enum protocol_stage *next_stage) {
  /*
    In this stage definitions of row fields are read from server. This
    can happen when reading query result set, or after preparing
    a statement which produces result set or upon explicit COM_FIELD_LIST
    request.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED: {
      /*
        If the received packet is not an EOF packet, then it is description
        of the next row field and we can continue, remaining in the same stage.

        To correctly recognize EOF packets we have to check its length, because
        a field description packet can theoretically start with byte 0xFE - the
        same as the first byte of the EOF packet.

        This can happen if the first length-encoded string in the packet starts
        with 8-byte length-encoded integer whose first byte is 0xFE. In this
        case the string and thus the whole packet is longer than 9 bytes and\
        EOF packet is always shorter than 9 bytes.
      */
      /*
        With new metadata result set format there is no EOF packet to follow
        and hence check for col_count if col_count is 1 it means all the
        columns are processed.
        In case of COM_FIELD_LIST reply there is no column count in first byte,
        instead the reply is a list of column definitions followed by OK or EOF.
        Hence in this case check for OK or EOF packet at the end.
      */
      bool new_client = (conn->server_capabilities & CLIENT_DEPRECATE_EOF);
      bool metadata_eof = (data->last_cmd != COM_FIELD_LIST &&
                           data->col_count == 1 && new_client);
      bool eof_packet =
          (EOF_PACKET(args.pkt) && args.pkt_len < MAX_PACKET_LENGTH);
      if (!eof_packet && !metadata_eof) {
        if (data->last_cmd != COM_FIELD_LIST) data->col_count--;
        LOG(("Received next field definition"));
        return 0;
      }

      LOG(("No more fields"));
      /*
        If this WAIT_FOR_FIELD_DEF stage was reached after COM_STMT_PREPARE or
        COM_FIELD_LIST then the EOF packet ends processing of the command and
        next stage is READY_FOR_COMMAND.
        Otherwise we are reading a result set and the field definitions have
        ended - next stage is WAIT_FOR_ROW.
      */
      switch (data->last_cmd) {
        case COM_STMT_PREPARE:
        case COM_FIELD_LIST:
          NEXT_STAGE(READY_FOR_COMMAND);
          break;
        default:
          LOG(("Reading result set rows"));
          NEXT_STAGE(WAIT_FOR_ROW);
      }

      return 0;
    }
    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_ROW(MYSQL *conn, struct st_trace_data *data,
                             enum trace_event ev,
                             struct st_trace_event_args args,
                             enum protocol_stage *next_stage) {
  /*
    This stage is entered when reading a result set, after receiving
    its metadata. In this stage server sends packets with rows until
    EOF or ERR packet is sent.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED: {
      byte *pkt = const_cast<byte *>(args.pkt);
      unsigned int flags;

      /*
        If ERR packet is received, an error has happened and reading
        the result set is done. Next stage is READY_FOR_COMMAND.
      */
      if (ERR_PACKET(pkt)) {
        NEXT_STAGE(READY_FOR_COMMAND);
        return 0;
      }

      /*
        Otherwise row packets are received until we get an EOF packet
        or OK packet starting with 0xFE.
        We need to check packet length again, because a text-protocol
        row packet starts with a length-encoded string and thus its
        first byte can be 0xFE.
      */
      /*
        Since result set row can be terminated with EOF as well as
        OK packet, we need to identify for EOF packet in case of old
        clients and OK packet starting with 0xFE in case of new clients
      */
      if (!(EOF_PACKET(pkt) && args.pkt_len < MAX_PACKET_LENGTH)) {
        LOG(("Received next row"));
        return 0;
      }

      LOG(("End of data"));
      /*
        In case of a multi-resutset a SERVER_MORE_RESULTS_EXISTS flag can
        be set in the EOF packet. In that case next resultset will follow
        and the next stage is WAIT_FOR_RESULTSET. If
        SERVER_MORE_RESULTS_EXISTS flag is not set, next stage is
        READY_FOR_COMMAND.
      */
      flags = uint2korr(pkt + 3);

      if (data->multi_resultset || SERVER_MORE_RESULTS_EXISTS & flags) {
        LOG(("Expecting next result set"));
        data->multi_resultset = true;
        NEXT_STAGE(WAIT_FOR_RESULT);
      } else
        NEXT_STAGE(READY_FOR_COMMAND);

      return 0;
    }
    default:
      return 1; /* invalid event */
  };
}

int check_event_WAIT_FOR_PS_DESCRIPTION(MYSQL *conn, struct st_trace_data *data,
                                        enum trace_event ev,
                                        struct st_trace_event_args args,
                                        enum protocol_stage *next_stage) {
  /*
    This stage is entered after executing COM_PREPARE_STMT command.
    Server  should send stmt OK packet with information about the
    statement.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED: {
      byte *pkt = const_cast<byte *>(args.pkt);
      long unsigned int stmt_id;
      unsigned int col_count, param_count;

      /*
        If we do not get OK packet here, something is wrong.
        We return to READY_FOR_COMMAND stage and continue tracing.
      */
      if (!OK_PACKET(pkt)) {
        LOG(("Wrong PS description"));
        NEXT_STAGE(READY_FOR_COMMAND);
        return 0;
      }

      /*
        Otherwise we read information about statement parameters
        (param_count) and produced result set (col_count) and store
        it in the data structure.
      */

      stmt_id = uint4korr(pkt + 1);
      col_count = uint2korr(pkt + 5);
      param_count = uint2korr(pkt + 7);
      data->col_count = col_count;
      data->param_count = param_count;

      LOG(("Prepared statement %lu; params: %u, cols: %u", stmt_id, param_count,
           col_count));

      /*
        If statement has parameters then next stage is
        WAIT_FOR_PS_PARAMETER, where server sends parameter
        descriptions.
      */
      if (0 < param_count) {
        NEXT_STAGE(WAIT_FOR_PARAM_DEF);
        return 0;
      }

      /*
        Otherwise, if statement has no parameters but produces
        a result set, the next stage is WAIT_FOR_FIELD_DEF where
        server sends descriptions of result set fields.
      */
      if (0 < col_count) {
        NEXT_STAGE(WAIT_FOR_FIELD_DEF);
        return 0;
      }

      /*
        Otherwise (no parameters and no result set), preparing
        the statement is done and next stage is READY_FOR_COMMAND.
      */
      NEXT_STAGE(READY_FOR_COMMAND);
      return 0;
    }
    default:
      return 1;
  }
}

int check_event_WAIT_FOR_PARAM_DEF(MYSQL *conn, struct st_trace_data *data,
                                   enum trace_event ev,
                                   struct st_trace_event_args args,
                                   enum protocol_stage *next_stage) {
  /*
    This stage is reached after COM_PREPARE_STMT command and after
    receiving the stmt OK packet from the server (if the statement
    has parameters). Server sends descriptions of the parameters in
    the same format in which result set fields are described.
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_READ_PACKET:
      return 0;
    case TRACE_EVENT_PACKET_RECEIVED: {
      /*
        Sending parameter descriptions continues until an EOF
        packet is received. To correctly recognize EOF packet
        we must check the packet length the same as
        in WAIT_FOR_FILED_DEF stage.
      */
      /*
        With new server which has the capability to deprecate EOF packet
        will not send EOF packet as part of parameter definitions thus
        check for param_count in this case.
      */

      bool new_client = (conn->server_capabilities & CLIENT_DEPRECATE_EOF);
      bool param_eof = (data->param_count == 1 && new_client);
      bool eof_packet =
          (EOF_PACKET(args.pkt) && args.pkt_len < 6 && !new_client);

      if (!eof_packet && !param_eof) {
        data->param_count--;
        LOG(("Received next parameter description"));
        return 0;
      }

      LOG(("No more parameters"));
      /*
        After receiving EOF packets next stage depends on whether
        the statement produces any result set, in which case column
        definitions will follow, or not.

        We decide this based on statement info stored in the data
        structure.
      */

      if (0 < data->col_count)
        NEXT_STAGE(WAIT_FOR_FIELD_DEF);
      else
        NEXT_STAGE(READY_FOR_COMMAND);

      return 0;
    }
    default:
      return 1;
  }
}

int check_event_FILE_REQUEST(MYSQL *conn, struct st_trace_data *data,
                             enum trace_event ev,
                             struct st_trace_event_args args,
                             enum protocol_stage *next_stage) {
  /*
    At this stage client sends packets with the file contents
    followed by an empty packet. After that client waits for
    a result set from server (which can be empty).
  */

  LOGGER(conn);

  switch (ev) {
    case TRACE_EVENT_SEND_FILE:
      return 0;
    case TRACE_EVENT_PACKET_SENT: {
      /*
        See if the packet sent was empty and if yes, move to
        WAIT_FOR_RESULTSET stage.
      */
      if (0 == args.pkt_len) {
        LOG(("File transfer done"));
        NEXT_STAGE(WAIT_FOR_RESULT);
      }
      return 0;
    }

    case TRACE_EVENT_READ_PACKET:
      /*
        Client library reads extra packet in case error is reported
        during file transfer.
      */
      return data->errnum ? 0 : 1;

    default:
      return 1; /* invalid event */
  };
}

int check_event_DISCONNECTED(MYSQL *, struct st_trace_data *, enum trace_event,
                             struct st_trace_event_args,
                             enum protocol_stage *) {
  /*
    This is the final stage reached when connection with
    server ends. No events should happen in this stage.
  */
  return 1;
}

}  // namespace test_trace

#endif  // #if defined(DBUG_OFF)
