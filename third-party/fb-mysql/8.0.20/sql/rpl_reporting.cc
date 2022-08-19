/* Copyright (c) 2007, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_reporting.h"

#include <stdio.h>

#include "m_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/log.h"
#include "sql/mysqld.h"     // slave_trans_retries
#include "sql/sql_class.h"  // THD
#include "sql/sql_error.h"  // Diagnostics_area
#include "sql/transaction_info.h"
#include "thr_mutex.h"

Slave_reporting_capability::Slave_reporting_capability(char const *thread_name)
    : m_thread_name(thread_name) {
  mysql_mutex_init(key_mutex_slave_reporting_capability_err_lock, &err_lock,
                   MY_MUTEX_INIT_FAST);
}

/**
  Check if the current error is of temporary nature or not.
  Some errors are temporary in nature, such as
  ER_LOCK_DEADLOCK and ER_LOCK_WAIT_TIMEOUT.  Ndb also signals
  that the error is temporary by pushing a warning with the error code
  ER_GET_TEMPORARY_ERRMSG, if the originating error is temporary.

  @param      thd  a THD instance, typically of the slave SQL thread's.
  @param error_arg  the error code for assessment.
              defaults to zero which makes the function check the top
              of the reported errors stack.
  @param silent     bool indicating whether the error should be silently
  handled.

  @return 1 as the positive and 0 as the negative verdict
*/
int Slave_reporting_capability::has_temporary_error(THD *thd, uint error_arg,
                                                    bool *silent) const {
  uint error;
  DBUG_TRACE;

  DBUG_EXECUTE_IF(
      "all_errors_are_temporary_errors", if (thd->get_stmt_da()->is_error()) {
        thd->clear_error();
        my_error(ER_LOCK_DEADLOCK, MYF(0));
      });

  /*
    The slave can't be regarded as experiencing a temporary failure in cases of
    is_fatal_error is true, or if no error is in THD and error_arg is not set.
  */
  if (thd->is_fatal_error() || (!thd->is_error() && error_arg == 0)) return 0;

  error = (error_arg == 0) ? thd->get_stmt_da()->mysql_errno() : error_arg;

  /*
    Temporary error codes:
    currently, InnoDB deadlock detected by InnoDB or lock
    wait timeout (innodb_lock_wait_timeout exceeded).
    Notice, the temporary error requires slave_trans_retries != 0)
  */
  if (slave_trans_retries &&
      (error == ER_LOCK_DEADLOCK || error == ER_LOCK_WAIT_TIMEOUT))
    return 1;

  /*
    currently temporary error set in ndbcluster
  */
  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  const Sql_condition *err;
  while ((err = it++)) {
    DBUG_PRINT("info", ("has condition %d %s", err->mysql_errno(),
                        err->message_text()));
    switch (err->mysql_errno()) {
      case ER_GET_TEMPORARY_ERRMSG:
        return 1;
      case ER_SLAVE_SILENT_RETRY_TRANSACTION: {
        if (silent != nullptr) *silent = true;
        return 1;
      }
      default:
        break;
    }
  }
  return 0;
}

void Slave_reporting_capability::report(loglevel level, int err_code,
                                        const char *msg, ...) const {
  va_list args;
  va_start(args, msg);
  do_report(level, err_code, msg, args);
  va_end(args);
}

void Slave_reporting_capability::va_report(loglevel level, int err_code,
                                           const char *prefix_msg,
                                           const char *msg,
                                           va_list args) const {
  THD *thd = current_thd;
  char buff[MAX_SLAVE_ERRMSG];
  char *pbuff = buff;
  char *curr_buff;
  uint pbuffsize = sizeof(buff);

  if (thd && level == ERROR_LEVEL && has_temporary_error(thd, err_code) &&
      !thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::SESSION))
    level = WARNING_LEVEL;

  mysql_mutex_lock(&err_lock);
  switch (level) {
    case ERROR_LEVEL:
      /*
        It's an error, it must be reported in Last_error and Last_errno in SHOW
        SLAVE STATUS.
      */
      pbuff = m_last_error.message;
      pbuffsize = sizeof(m_last_error.message);
      m_last_error.number = err_code;
      m_last_error.update_timestamp();
      break;
    case WARNING_LEVEL:
    case INFORMATION_LEVEL:
      break;
    default:
      DBUG_ASSERT(0);  // should not come here
      // don't crash production builds, just do nothing
      mysql_mutex_unlock(&err_lock);
      return;
  }
  curr_buff = pbuff;
  if (prefix_msg)
    curr_buff += snprintf(curr_buff, pbuffsize, "%s; ", prefix_msg);
  vsnprintf(curr_buff, pbuffsize - (curr_buff - pbuff), msg, args);

  mysql_mutex_unlock(&err_lock);

  /* If the msg string ends with '.', do not add a ',' it would be ugly */
  LogEvent()
      .type(LOG_TYPE_ERROR)
      .subsys(LOG_SUBSYSTEM_TAG)
      .prio(level)
      // if it's a client err-code, flag it as from diagnostics area
      .errcode((err_code < ER_SERVER_RANGE_START)
                   ? ER_RPL_SLAVE_ERROR_INFO_FROM_DA
                   : err_code)
      .subsys("Repl")
      /*
        We'll use the original error-code in the actual message,
        even if it's out of range. That should make it easier
        to find, debug, and fix.
      */
      .message("Slave %s%s: %s%s Error_code: MY-%06d", m_thread_name,
               get_for_channel_str(false), pbuff,
               (curr_buff[0] && *(strend(curr_buff) - 1) == '.') ? "" : ",",
               err_code);

  // we shouldn't really use client error codes here, so bomb out in debug mode
  // DBUG_ASSERT(err_code >= ER_SERVER_RANGE_START);
}

Slave_reporting_capability::~Slave_reporting_capability() {
  mysql_mutex_destroy(&err_lock);
}
