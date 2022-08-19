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

/**
  @addtogroup Replication
  @{

  @file

  @brief Transaction boundary parser definitions. This includes code for
  parsing a stream of events identifying the transaction boundaries (like
  if the event is starting a transaction, is in the middle of a transaction
  or if the event is ending a transaction).
*/

#ifndef TRX_BOUNDARY_PARSER_H
#define TRX_BOUNDARY_PARSER_H

#include <stddef.h>

#include "libbinlogevents/include/control_events.h"

/**
  @class Transaction_boundary_parser

  This is the base class for verifying transaction boundaries.
*/
class Transaction_boundary_parser {
 public:
  /**
    The context where the parser is used
  */
  enum enum_trx_boundary_parser_context {
    /* Parser used on a receiver, like an IO thread */
    TRX_BOUNDARY_PARSER_RECEIVER,
    /* Parser used in an applier parsing Relay Log files */
    TRX_BOUNDARY_PARSER_APPLIER
  };

  /**
     Constructor.
     @param context If this parser is used on a receiver or applier context
  */
  Transaction_boundary_parser(enum_trx_boundary_parser_context context)
      : current_parser_state(EVENT_PARSER_NONE),
        last_parser_state(EVENT_PARSER_NONE),
        m_current_boundary_state(EVENT_BOUNDARY_TYPE_ERROR),
        m_trx_boundary_parser_context(context) {}

  /**
    Destructor
  */
  virtual ~Transaction_boundary_parser();

  /**
     Reset the transaction boundary parser state.
  */
  void reset();

  /*
    In an event stream, an event is considered safe to be separated from the
    next if it is not inside a transaction.
    We need to know this in order to evaluate if we will let the relay log
    to be rotated or not.
  */

  /**
     State if the transaction boundary parser is inside a transaction.
     This "inside a transaction" means that the parser was fed with at least
     one event of a transaction, but the transaction wasn't completely fed yet.
     This also means that the last event fed depends on following event(s) to
     be correctly applied.

     @return  false if the boundary parser is not inside a transaction.
              true if the boundary parser is inside a transaction.
  */
  inline bool is_inside_transaction() {
    return (current_parser_state != EVENT_PARSER_ERROR &&
            current_parser_state != EVENT_PARSER_NONE);
  }

  /**
     State if the transaction boundary parser is not inside a transaction.
     This "not inside a transaction" means that the parser was fed with an
     event that doesn't depend on following events.

     @return  false if the boundary parser is inside a transaction.
              true if the boundary parser is not inside a transaction.
  */
  inline bool is_not_inside_transaction() {
    return (current_parser_state == EVENT_PARSER_NONE);
  }

  /**
     State if the transaction boundary parser was fed with a sequence of events
     that the parser wasn't able to parse correctly.

     @return  false if the boundary parser is not in the error state.
              true if the boundary parser is in the error state.
  */
  inline bool is_error() {
    return (current_parser_state == EVENT_PARSER_ERROR);
  }

  /**
   Feed the transaction boundary parser with a Log_event of any type
   in object type.

   @param log_event_info the event object
   @param throw_warnings If the function should throw warning messages while
                         updating the boundary parser state.
                         While initializing the Relay_log_info the
                         relay log is scanned backwards and this could
                         generate false errors. So, in this case, we
                         don't want to throw warnings.

   @return  false if the transaction boundary parser accepted the event.
            true if the transaction boundary parser didn't accepted the event.
  */
  bool feed_event(binary_log::Log_event_basic_info log_event_info,
                  bool throw_warnings);

  /**
    Evaluate given the current info about boundary type, event type and
    parser state if the given event violates any restriction associated
    to row based only modes.

    @param event_info the event information: type, query, is it ignorable

    @return true if it violates any restrictions
            false if it passes all tests
  */
  bool check_row_logging_constraints(
      binary_log::Log_event_basic_info event_info);

  /**
     Rolls back to the last parser state.

     This should be called in the case of a failed queued event.
  */
  void rollback() { current_parser_state = last_parser_state; }

  /**
    Internal error indentifiers for parser issues
  */
  enum enum_event_parser_error {
    /* Unexpected event that the parser can't ignore  */
    ER_TRX_BOUND_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM,
    /* Unexpected GTID event in the stream */
    ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM,
    /* Unexpected BEGIN event in the stream */
    ER_TRX_BOUND_UNEXPECTED_BEGIN_IN_STREAM,
    /* Unexpected Commit event in the stream  */
    ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM,
    /* Unexpected XA Rollback event in the stream */
    ER_TRX_BOUND_UNEXPECTED_XA_ROLLBACK_IN_STREAM
  };

 private:
  enum enum_event_boundary_type {
    EVENT_BOUNDARY_TYPE_ERROR = -1,
    /* Gtid_log_event */
    EVENT_BOUNDARY_TYPE_GTID = 0,
    /* Query_log_event(BEGIN), Query_log_event(XA START) */
    EVENT_BOUNDARY_TYPE_BEGIN_TRX = 1,
    /* Xid, Query_log_event(COMMIT), Query_log_event(ROLLBACK),
       XA_Prepare_log_event */
    EVENT_BOUNDARY_TYPE_END_TRX = 2,
    /* Query_log_event(XA ROLLBACK) */
    EVENT_BOUNDARY_TYPE_END_XA_TRX = 3,
    /* User_var, Intvar and Rand */
    EVENT_BOUNDARY_TYPE_PRE_STATEMENT = 4,
    /*
      All other Query_log_events and all other DML events
      (Rows, Load_data, etc.)
    */
    EVENT_BOUNDARY_TYPE_STATEMENT = 5,
    /* Incident */
    EVENT_BOUNDARY_TYPE_INCIDENT = 6,
    /*
      All non DDL/DML events: Format_desc, Rotate,
      Previous_gtids, Stop, etc.
    */
    EVENT_BOUNDARY_TYPE_IGNORE = 7,
    /*
      Transaction payload boundary.
     */
    EVENT_BOUNDARY_TYPE_TRANSACTION_PAYLOAD = 8
  };

  /*
    Internal states for parsing a stream of events.

    DDL has the format:
      DDL-1: [GTID]
      DDL-2: [User] [Intvar] [Rand]
      DDL-3: Query

    DML has the format:
      DML-1: [GTID]
      DML-2: Query(BEGIN)
      DML-3: Statements
      DML-4: (Query(COMMIT) | Query([XA] ROLLBACK) | Xid | Xa_prepare)

    Compressed DML/DDL has the format:
      DDL-1: GTID
      RAW-1: Transaction_Payload
  */
  enum enum_event_parser_state {
    /* NONE is set after DDL-3 or DML-4 or RAW-1*/
    EVENT_PARSER_NONE,
    /* GTID is set after DDL-1 or DML-1 */
    EVENT_PARSER_GTID,
    /* DDL is set after DDL-2 */
    EVENT_PARSER_DDL,
    /* DML is set after DML-2 */
    EVENT_PARSER_DML,
    /* ERROR is set whenever the above pattern is not followed */
    EVENT_PARSER_ERROR
  };

  /**
     Current internal state of the event parser.
  */
  enum_event_parser_state current_parser_state;

  /**
     Last internal state of the event parser.

     This should be used if we had to roll back the last parsed event.
  */
  enum_event_parser_state last_parser_state;

  /**
    The last processed boundary event type
  */
  enum_event_boundary_type m_current_boundary_state;

  /**
    In which context of the boundary parser is used
  */
  enum_trx_boundary_parser_context m_trx_boundary_parser_context;

  /**
    Parses an raw event based on the event parser logic.

    @param event_info      Info about an event: type, query, is it ignorable
    @param throw_warnings  If the function should throw warning messages
                           while updating the boundary parser state.
    @return What is the boundary type associated to this event
  */
  enum_event_boundary_type get_event_boundary_type(
      binary_log::Log_event_basic_info event_info, bool throw_warnings);

  /**
    Set the boundary parser state based on the event parser logic.

    @param event_boundary_type  the current event boundary type
    @param throw_warnings       If the function should throw warning messages
                                while updating the boundary parser state.

    @return true if there is an error while updating the state, like unexpected
            event order
  */
  bool update_state(enum_event_boundary_type event_boundary_type,
                    bool throw_warnings);

  /**
    Log warnings using some defined logging interface.

    @note: this method is empty by default. Extend it to add a logging routine.

    @param error the error number
    @param message the error message
  */
  virtual void log_server_warning(int error, const char *message);
};

/**
  @} (End of group Replication)
*/

#endif /* TRX_BOUNDARY_PARSER_H */
