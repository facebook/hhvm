/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql_restart_server.h"

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#endif  // _WIN32

#include "my_thread.h"                               // my_thread_handle
#include "mysql/components/services/log_builtins.h"  // LogErr
#include "mysqld.h"                                  // opt_noacl
#include "mysqld_error.h"               // ER_SPECIFIC_ACCESS_DENIED_ERROR
#include "sql/auth/auth_acls.h"         // SHUTDOWN_ACL
#include "sql/auth/sql_security_ctx.h"  // Security_context
#include "sql/log.h"                    // Query_logger
#include "sql/sql_class.h"              // THD
#include "sql/sql_lex.h"                // LEX

extern my_thread_handle signal_thread_id;

/**
  Check if a current user has the privilege SHUTDOWN_ACL  required to run
  the statement RESTART.

  @param thd Current thread

  @retval false A user has the privilege SHUTDOWN_ACL.
  @retval true  A user doesn't have the privilege SHUTDOWN_ACL..
*/

static inline bool check_restart_server_admin_privilege(THD *thd) {
  return !opt_noacl && check_global_access(thd, SHUTDOWN_ACL);
}

bool is_mysqld_managed() {
#ifndef _WIN32
  char *parent_pid = getenv("MYSQLD_PARENT_PID");
  if (parent_pid == nullptr) return false;

  return atoi(parent_pid) == getppid();
#else
  return !(opt_debugging || opt_no_monitor);
#endif  // _WIN32
}

bool Sql_cmd_restart_server::execute(THD *thd) {
  DBUG_TRACE;

  if (check_restart_server_admin_privilege(thd)) return true;

  // RESTART shall not be binlogged.
  thd->lex->no_write_to_binlog = true;

  query_logger.general_log_print(thd, COM_QUERY, NullS);

  if (signal_restart_server()) {
    my_error(ER_RESTART_SERVER_FAILED, MYF(0), "Restart server failed");
    return true;
  }

  LogErr(SYSTEM_LEVEL, ER_RESTART_RECEIVED_INFO,
         thd->security_context()->user().str, server_version);
  my_ok(thd);

  return false;
}
