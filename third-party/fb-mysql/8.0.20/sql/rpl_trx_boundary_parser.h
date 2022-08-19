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

  @brief Replication transaction boundary parser. This includes code for
  an extension of the transaction boundary parser to parse a replication
  stream of events identifying the transaction boundaries (like if the event
  is starting a transaction, is in the middle of a transaction or if the event
  is ending a transaction).
*/

#ifndef RPL_TRX_BOUNDARY_PARSER_H
#define RPL_TRX_BOUNDARY_PARSER_H

#include <stddef.h>

#include "libbinlogevents/include/trx_boundary_parser.h"

/**
  @class Replication_transaction_boundary_parser

  This is the class for verifying transaction boundaries in a replication
  event stream.
*/
class Replication_transaction_boundary_parser
    : public Transaction_boundary_parser {
 public:
  /**
    The constructor
    @param context If this parser is used on a receiver or applier context
  */
  Replication_transaction_boundary_parser(
      Transaction_boundary_parser::enum_trx_boundary_parser_context context)
      : Transaction_boundary_parser(context) {}
  /**
    Log warnings into the error log.

    @param error the error number
    @param message the error message
  */
  virtual void log_server_warning(int error, const char *message);
};

/**
  @} (End of group Replication)
*/

#endif /* RPL_TRX_BOUNDARY_PARSER_H */
