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

#include "sql/rpl_trx_boundary_parser.h"

#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/log.h"

void Replication_transaction_boundary_parser::log_server_warning(
    int error, const char *message) {
  int server_log_error = 0;
  switch (error) {
    case ER_TRX_BOUND_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM: {
      server_log_error = ER_RPL_UNSUPPORTED_UNIGNORABLE_EVENT_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_GTID_LOG_EVENT_IN_STREAM: {
      server_log_error = ER_RPL_GTID_LOG_EVENT_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_UNEXPECTED_BEGIN_IN_STREAM: {
      server_log_error = ER_RPL_UNEXPECTED_BEGIN_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM: {
      server_log_error =
          ER_RPL_UNEXPECTED_COMMIT_ROLLBACK_OR_XID_LOG_EVENT_IN_STREAM;
      break;
    }
    case ER_TRX_BOUND_UNEXPECTED_XA_ROLLBACK_IN_STREAM: {
      server_log_error = ER_RPL_UNEXPECTED_XA_ROLLBACK_IN_STREAM;
      break;
    }
    default:
      DBUG_ASSERT(false); /* purecov: inspected */
      return;
  }

  if (message != nullptr)
    LogErr(WARNING_LEVEL, server_log_error, message);
  else
    LogErr(WARNING_LEVEL, server_log_error);
}
