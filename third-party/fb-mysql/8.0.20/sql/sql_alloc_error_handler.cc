/* Copyright (c) 2010, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_dbug.h"
#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/log.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"

extern "C" void sql_alloc_error_handler(void) {
  THD *thd = current_thd;
  if (thd && !thd->is_error()) {
    /*
      This thread is Out Of Memory.

      An OOM condition is a fatal error. It should not be caught by error
      handlers in stored procedures.

      Recording this SQL condition in the condition area could cause more
      memory allocations, which in turn could raise more OOM conditions,
      causing recursion in the error handling code itself. As a result,
      my_error() should not be invoked, and the thread Diagnostics Area is
      set to an error status directly.

      Note that Diagnostics_area::set_error_status() is safe, since it does
      not call any memory allocation routines.

      The visible result for a client application will be:
        - a query fails with an ER_OUT_OF_RESOURCES error, returned in the
          error packet.
        - SHOW ERROR/SHOW WARNINGS may be empty.
    */
    thd->get_stmt_da()->set_error_status(thd, ER_OUT_OF_RESOURCES);
  }

  /* Skip writing to the error log to avoid mtr complaints */
  DBUG_EXECUTE_IF("simulate_out_of_memory", return;);

  LogErr(ERROR_LEVEL, ER_SERVER_OUT_OF_RESOURCES);
}
