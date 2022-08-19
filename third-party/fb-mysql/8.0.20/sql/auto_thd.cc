/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/auto_thd.h"

#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "sql/log.h"
#include "sql/sql_class.h"             // THD
#include "sql/sql_thd_internal_api.h"  // create_thd / destroy_thd

/**
  Create THD object and initialize internal variables.
*/
Auto_THD::Auto_THD() : thd(create_thd(false, true, false, 0)) {
  thd->push_internal_handler(this);
}

/**
  Deinitialize THD.
*/
Auto_THD::~Auto_THD() {
  thd->pop_internal_handler();
  destroy_thd(thd);
}

/**
  Error handler that prints error message on to the error log.

  @param thd       Current THD.
  @param sql_errno Error id.
  @param sqlstate  State of the SQL error.
  @param level     Error level.
  @param msg       Message to be reported.

  @return This function always return false.
*/
bool Auto_THD::handle_condition(
    THD *thd MY_ATTRIBUTE((unused)), uint sql_errno, const char *sqlstate,
    Sql_condition::enum_severity_level *level MY_ATTRIBUTE((unused)),
    const char *msg) {
  int log_err_level = 0;

  if (*level == Sql_condition::SL_WARNING)
    log_err_level = WARNING_LEVEL;
  else if (*level == Sql_condition::SL_ERROR)
    log_err_level = ERROR_LEVEL;
  else if (*level == Sql_condition::SL_NOTE)
    log_err_level = INFORMATION_LEVEL;

  LogEvent()
      .type(LOG_TYPE_ERROR)
      .prio(log_err_level)
      .sqlstate(sqlstate)
      .errcode(ER_SERVER_NO_SESSION_TO_SEND_TO)  // override if exists
      .lookup(ER_SERVER_NO_SESSION_TO_SEND_TO, sql_errno, msg);

  return false;
}
