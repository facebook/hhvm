/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "libbinlogevents/include/trx_boundary_parser.h"

#include <string.h>

#include "libbinlogevents/include/binlog_event.h"
#include "m_string.h"
#include "my_byteorder.h"

#ifndef DBUG_OFF
/* Event parser state names */
static const char *event_parser_state_names[] = {"None", "GTID", "DDL", "DML",
                                                 "Error"};
#endif

/*
  -----------------------------------------
  Transaction_boundary_parser class methods
  -----------------------------------------
*/

Transaction_boundary_parser::~Transaction_boundary_parser() {}

/**
   Reset the transaction boundary parser.

   This method initialize the boundary parser state.
*/
void Transaction_boundary_parser::reset() {
  BAPI_PRINT("info", ("transaction boundary parser is changing state "
                      "from '%s' to '%s'",
                      event_parser_state_names[current_parser_state],
                      event_parser_state_names[EVENT_PARSER_NONE]));
  current_parser_state = EVENT_PARSER_NONE;
  last_parser_state = EVENT_PARSER_NONE;
  m_current_boundary_state = EVENT_BOUNDARY_TYPE_ERROR;
}

bool Transaction_boundary_parser::feed_event(
    binary_log::Log_event_basic_info log_event_info, bool throw_warnings) {
  BAPI_TRACE;

  m_current_boundary_state =
      get_event_boundary_type(log_event_info, throw_warnings);
  return update_state(m_current_boundary_state, throw_warnings);
}

bool Transaction_boundary_parser::check_row_logging_constraints(
    binary_log::Log_event_basic_info log_event_info) {
  BAPI_TRACE;

  // If the boundary parser is in error state return true
  if (EVENT_BOUNDARY_TYPE_ERROR == m_current_boundary_state) return true;

  // INTVAR_EVENT, RAND_EVENT, USER_VAR_EVENT
  if (EVENT_BOUNDARY_TYPE_PRE_STATEMENT == m_current_boundary_state) {
    // An USER_VAR_EVENT can appear with RBR when associated to DDL events
    return log_event_info.event_type != binary_log::USER_VAR_EVENT ||
           current_parser_state != EVENT_PARSER_DDL;
  }

  // DDL or TRANSACTION_PAYLOAD_EVENT
  if (EVENT_BOUNDARY_TYPE_STATEMENT == m_current_boundary_state ||
      current_parser_state == EVENT_PARSER_NONE) {
    if (log_event_info.query_length > 16 &&
        (!native_strncasecmp(log_event_info.query,
                             STRING_WITH_LEN("CREATE TEMPORARY")) ||
         !native_strncasecmp(log_event_info.query,
                             STRING_WITH_LEN("DROP TEMPORARY")))) {
      return true;
    }
  }

  // Defend against injected LOAD events
  if (log_event_info.event_type == binary_log::BEGIN_LOAD_QUERY_EVENT ||
      log_event_info.event_type == binary_log::EXECUTE_LOAD_QUERY_EVENT ||
      log_event_info.event_type == binary_log::APPEND_BLOCK_EVENT ||
      log_event_info.event_type == binary_log::DELETE_FILE_EVENT)
    return true;

  // DML using non row events
  if (EVENT_BOUNDARY_TYPE_STATEMENT == m_current_boundary_state &&
      current_parser_state == EVENT_PARSER_DML)
    if (log_event_info.event_type != binary_log::TABLE_MAP_EVENT &&
        log_event_info.event_type != binary_log::WRITE_ROWS_EVENT &&
        log_event_info.event_type != binary_log::UPDATE_ROWS_EVENT &&
        log_event_info.event_type != binary_log::DELETE_ROWS_EVENT &&
        log_event_info.event_type != binary_log::WRITE_ROWS_EVENT_V1 &&
        log_event_info.event_type != binary_log::UPDATE_ROWS_EVENT_V1 &&
        log_event_info.event_type != binary_log::DELETE_ROWS_EVENT_V1 &&
        log_event_info.event_type != binary_log::PARTIAL_UPDATE_ROWS_EVENT &&
        log_event_info.event_type != binary_log::ROWS_QUERY_LOG_EVENT &&
        log_event_info.event_type != binary_log::VIEW_CHANGE_EVENT) {
      // We allow XA control statements
      if (log_event_info.event_type == binary_log::QUERY_EVENT) {
        if (!native_strncasecmp(log_event_info.query,
                                STRING_WITH_LEN("XA END")) ||
            !native_strncasecmp(log_event_info.query,
                                STRING_WITH_LEN("XA PREPARE")) ||
            !native_strncasecmp(log_event_info.query,
                                STRING_WITH_LEN("SAVEPOINT")))
          return false;
      }
      return true;
    }

  return false;
}

Transaction_boundary_parser::enum_event_boundary_type
Transaction_boundary_parser::get_event_boundary_type(
    binary_log::Log_event_basic_info event_info, bool throw_warnings) {
  BAPI_TRACE;

  enum_event_boundary_type boundary_type;

  switch (event_info.event_type) {
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_GTID;
      break;

    /*
      There are four types of queries that we have to deal with: BEGIN, COMMIT,
      ROLLBACK and the rest.
    */
    case binary_log::QUERY_EVENT: {
      if (event_info.query_length == 0) {
        /* purecov: begin inspected */
        BAPI_ASSERT(event_info.query == nullptr);
        boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
        break;
        /* purecov: end */
      }

      /*
        BEGIN is always the begin of a DML transaction.
      */
      if (!strncmp(event_info.query, "BEGIN", event_info.query_length) ||
          !strncmp(event_info.query, STRING_WITH_LEN("XA START")))
        boundary_type = EVENT_BOUNDARY_TYPE_BEGIN_TRX;
      /*
        COMMIT and ROLLBACK are always the end of a transaction.
      */
      else if (!strncmp(event_info.query, "COMMIT", event_info.query_length) ||
               (!native_strncasecmp(event_info.query,
                                    STRING_WITH_LEN("ROLLBACK")) &&
                native_strncasecmp(event_info.query,
                                   STRING_WITH_LEN("ROLLBACK TO "))))
        boundary_type = EVENT_BOUNDARY_TYPE_END_TRX;
      /*
        XA ROLLBACK is always the end of a XA transaction.
      */
      else if (!native_strncasecmp(event_info.query,
                                   STRING_WITH_LEN("XA ROLLBACK")))
        boundary_type = EVENT_BOUNDARY_TYPE_END_XA_TRX;
      /*
        If the query is not (BEGIN | XA START | COMMIT | [XA] ROLLBACK), it can
        be considered an ordinary statement.
      */
      else
        boundary_type = EVENT_BOUNDARY_TYPE_STATEMENT;

      break;
    }

    /*
      XID events are always the end of a transaction.
    */
    case binary_log::XID_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_END_TRX;
      break;
    /*
      XA_prepare event ends XA-prepared group of events (prepared XA
      transaction).
    */
    case binary_log::XA_PREPARE_LOG_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_END_TRX;
      break;

    /*
      Intvar, Rand and User_var events are always considered as pre-statements.
    */
    case binary_log::INTVAR_EVENT:
    case binary_log::RAND_EVENT:
    case binary_log::USER_VAR_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_PRE_STATEMENT;
      break;

    /*
      The following event types are always considered as statements
      because they will always be wrapped between BEGIN/COMMIT.
    */
    case binary_log::EXECUTE_LOAD_QUERY_EVENT:
    case binary_log::TABLE_MAP_EVENT:
    case binary_log::APPEND_BLOCK_EVENT:
    case binary_log::BEGIN_LOAD_QUERY_EVENT:
    case binary_log::ROWS_QUERY_LOG_EVENT:
    case binary_log::WRITE_ROWS_EVENT:
    case binary_log::UPDATE_ROWS_EVENT:
    case binary_log::DELETE_ROWS_EVENT:
    case binary_log::WRITE_ROWS_EVENT_V1:
    case binary_log::UPDATE_ROWS_EVENT_V1:
    case binary_log::DELETE_ROWS_EVENT_V1:
    case binary_log::VIEW_CHANGE_EVENT:
    case binary_log::PARTIAL_UPDATE_ROWS_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_STATEMENT;
      break;

    /*
      Incident events have their own boundary type.
    */
    case binary_log::INCIDENT_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_INCIDENT;
      break;

    /*
      Rotate, Format_description and Heartbeat should be ignored.
      Also, any other kind of event not listed in the "cases" above
      will be ignored.
    */
    case binary_log::ROTATE_EVENT:
    case binary_log::FORMAT_DESCRIPTION_EVENT:
    case binary_log::HEARTBEAT_LOG_EVENT:
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
    case binary_log::STOP_EVENT:
    case binary_log::METADATA_EVENT:
    case binary_log::DELETE_FILE_EVENT:
    case binary_log::TRANSACTION_CONTEXT_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_IGNORE;
      break;

    case binary_log::TRANSACTION_PAYLOAD_EVENT:
      boundary_type = EVENT_BOUNDARY_TYPE_TRANSACTION_PAYLOAD;
      break;

    /*
      If the event is none of above supported event types, this is probably
      an event type unsupported by this server version. So, we must check if
      this event is ignorable or not.
    */
    default:
      if (event_info.ignorable_event)
        boundary_type = EVENT_BOUNDARY_TYPE_IGNORE;
      else {
        boundary_type = EVENT_BOUNDARY_TYPE_ERROR;
        if (throw_warnings)
          log_server_warning(
              ER_TRX_BOUND_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM, nullptr);
      }
  } /* End of switch(event_type) */

  return boundary_type;
}

/**
   Update the boundary parser state based on a given boundary type.

   @param event_boundary_type The event boundary type of the event used to
                              fed the boundary parser.
   @param throw_warnings      If the function should throw warnings while
                              updating the boundary parser state.
                              Please see comments on this at feed_event().

   @return  false State updated successfully.
            true  There was an error updating the state.
*/
bool Transaction_boundary_parser::update_state(
    enum_event_boundary_type event_boundary_type, bool throw_warnings) {
  BAPI_TRACE;

  enum_event_parser_state new_parser_state = EVENT_PARSER_NONE;

  bool error = false;

  switch (event_boundary_type) {
    /*
      GTIDs are always the start of a transaction stream.
    */
    case EVENT_BOUNDARY_TYPE_GTID:
      /* In any case, we will update the state to GTID */
      new_parser_state = EVENT_PARSER_GTID;
      /* The following switch is mostly to differentiate the warning messages */
      switch (current_parser_state) {
        case EVENT_PARSER_GTID:
        case EVENT_PARSER_DDL:
        case EVENT_PARSER_DML:
          /*
            We don't consider an unexpected GTID appearing in the applier as
            being an error. In this context it can mean several things:
            - A transaction was logged partially in an old relay log
            - A transaction is retrying after an error
          */
          if (m_trx_boundary_parser_context != TRX_BOUNDARY_PARSER_APPLIER) {
            if (throw_warnings) {
              if (current_parser_state == EVENT_PARSER_GTID)
                log_server_warning(
                    ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM,
                    "after a GTID_LOG_EVENT or an ANONYMOUS_GTID_LOG_EVENT");
              else {
                if (current_parser_state == EVENT_PARSER_DDL)
                  log_server_warning(ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM,
                                     "in the middle of a DDL");
                else
                  log_server_warning(ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM,
                                     "in the middle of a DML");
              }
            }
            error = true;
          }
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          /* FALL THROUGH */
        case EVENT_PARSER_NONE:
          break;
      }
      break;

    /*
      There are four types of queries that we have to deal with: BEGIN, COMMIT,
      ROLLBACK and the rest.
    */
    case EVENT_BOUNDARY_TYPE_BEGIN_TRX:
      /* In any case, we will update the state to DML */
      new_parser_state = EVENT_PARSER_DML;
      /* The following switch is mostly to differentiate the warning messages */
      switch (current_parser_state) {
        case EVENT_PARSER_DDL:
        case EVENT_PARSER_DML:
          if (throw_warnings) {
            if (current_parser_state == EVENT_PARSER_DDL)
              log_server_warning(ER_TRX_BOUND_UNEXPECTED_BEGIN_IN_STREAM,
                                 "DDL");
            else
              log_server_warning(ER_TRX_BOUND_UNEXPECTED_BEGIN_IN_STREAM,
                                 "DML");
          }
          error = true;
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          /* FALL THROUGH */
        case EVENT_PARSER_NONE:
        case EVENT_PARSER_GTID:
          break;
      }
      break;

    case EVENT_BOUNDARY_TYPE_END_TRX:
      /* In any case, we will update the state to NONE */
      new_parser_state = EVENT_PARSER_NONE;
      /* The following switch is mostly to differentiate the warning messages */
      switch (current_parser_state) {
        case EVENT_PARSER_NONE:
        case EVENT_PARSER_GTID:
        case EVENT_PARSER_DDL:
          if (throw_warnings) {
            if (current_parser_state == EVENT_PARSER_NONE)
              log_server_warning(
                  ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM,
                  "outside a transaction");
            else {
              if (current_parser_state == EVENT_PARSER_GTID)
                log_server_warning(
                    ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM,
                    "after a GTID_LOG_EVENT");
              else
                log_server_warning(
                    ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM,
                    "in the middle of a DDL");
            }
          } /* EVENT_PARSER_DDL */
          error = true;
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          /* FALL THROUGH */
        case EVENT_PARSER_DML:
          break;
      }
      break;

    case EVENT_BOUNDARY_TYPE_END_XA_TRX:
      /* In any case, we will update the state to NONE */
      new_parser_state = EVENT_PARSER_NONE;
      /* The following switch is mostly to differentiate the warning messages */
      switch (current_parser_state) {
        case EVENT_PARSER_NONE:
        case EVENT_PARSER_DDL:
          if (throw_warnings) {
            if (current_parser_state == EVENT_PARSER_NONE) {
              log_server_warning(ER_TRX_BOUND_UNEXPECTED_XA_ROLLBACK_IN_STREAM,
                                 "outside a transaction");
            } else {
              log_server_warning(ER_TRX_BOUND_UNEXPECTED_XA_ROLLBACK_IN_STREAM,
                                 "in the middle of a DDL");
            }
          } /* EVENT_PARSER_DDL */
          error = true;
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          /* FALL THROUGH */
        case EVENT_PARSER_DML:
        /* XA ROLLBACK can appear after a GTID event */
        case EVENT_PARSER_GTID:
          break;
      }
      break;

    case EVENT_BOUNDARY_TYPE_STATEMENT:
      switch (current_parser_state) {
        case EVENT_PARSER_NONE:
          new_parser_state = EVENT_PARSER_NONE;
          break;
        case EVENT_PARSER_GTID:
        case EVENT_PARSER_DDL:
          new_parser_state = EVENT_PARSER_NONE;
          break;
        case EVENT_PARSER_DML:
          new_parser_state = current_parser_state;
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          break;
      }
      break;

    case EVENT_BOUNDARY_TYPE_TRANSACTION_PAYLOAD:
      switch (current_parser_state) {
        case EVENT_PARSER_GTID:
          new_parser_state = EVENT_PARSER_NONE;
          break;
        case EVENT_PARSER_NONE:
        case EVENT_PARSER_DML:
        case EVENT_PARSER_DDL:
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          break;
      }
      break;

    /*
      Intvar, Rand and User_var events might be inside of a transaction stream
      if any Intvar, Rand and User_var was fed before, if BEGIN was fed before
      or if GTID was fed before. In the case of no GTID, no BEGIN and no
      previous Intvar, Rand or User_var it will be considered the start of a
      transaction stream.
    */
    case EVENT_BOUNDARY_TYPE_PRE_STATEMENT:
      switch (current_parser_state) {
        case EVENT_PARSER_NONE:
        case EVENT_PARSER_GTID:
          new_parser_state = EVENT_PARSER_DDL;
          break;
        case EVENT_PARSER_DDL:
        case EVENT_PARSER_DML:
          new_parser_state = current_parser_state;
          break;
        case EVENT_PARSER_ERROR: /* we probably threw a warning before */
          error = true;
          break;
      }
      break;

    /*
      Incident events can happen without a GTID (before BUG#19594845 fix) or
      with its own GTID in order to be skipped. In any case, it should always
      mark "the end" of a transaction.
    */
    case EVENT_BOUNDARY_TYPE_INCIDENT:
      /* In any case, we will update the state to NONE */
      new_parser_state = EVENT_PARSER_NONE;
      break;

    /*
      Rotate, Format_description and Heartbeat should be ignored.
      The rotate might be fake, like when the IO thread receives from dump
      thread Previous_gtid and Heartbeat events due to reconnection/auto
      positioning.
    */
    case EVENT_BOUNDARY_TYPE_IGNORE:
      new_parser_state = current_parser_state;
      break;

    case EVENT_BOUNDARY_TYPE_ERROR:
      error = true;
      new_parser_state = EVENT_PARSER_ERROR;
      break;
  }

  BAPI_PRINT("info", ("transaction boundary parser is changing state "
                      "from '%s' to '%s'",
                      event_parser_state_names[current_parser_state],
                      event_parser_state_names[new_parser_state]));

  last_parser_state = current_parser_state;
  current_parser_state = new_parser_state;

  return error;
}

void Transaction_boundary_parser::log_server_warning(int, const char *) {}
