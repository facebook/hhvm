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

#include "sql/check_stack.h"

#include "my_config.h"

#include <stdio.h>
#include <algorithm>
#include <atomic>
#include <new>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/derror.h"
#include "sql/sql_class.h"

/****************************************************************************
        Check stack size; Send error if there isn't enough stack to continue
****************************************************************************/

#if STACK_DIRECTION < 0
#define used_stack(A, B) (long)(A - B)
#else
#define used_stack(A, B) (long)(B - A)
#endif

#ifndef DBUG_OFF
std::atomic<long> max_stack_used;
#endif

/**
  Check stack for a overrun

  @param thd            Thread handler.
  @param margin         Minimal acceptable unused space in the stack.
  @param buf            See a note below.

  @returns false if success, true if error (reported).

  @note
  Note: The 'buf' parameter is necessary, even if it is unused here.
  - fix_fields functions has a "dummy" buffer large enough for the
    corresponding exec. (Thus we only have to check in fix_fields.)
  - Passing to check_stack_overrun() prevents the compiler from removing it.
*/
bool check_stack_overrun(const THD *thd, long margin,
                         unsigned char *buf MY_ATTRIBUTE((unused))) {
  DBUG_ASSERT(thd == current_thd);
  long stack_used =
      used_stack(thd->thread_stack, reinterpret_cast<char *>(&stack_used));
  if (stack_used >= static_cast<long>(my_thread_stack_size - margin) ||
      DBUG_EVALUATE_IF("simulate_stack_overrun", true, false)) {
    /*
      Do not use stack for the message buffer to ensure correct
      behaviour in cases we have close to no stack left.
    */
    char *ebuff = new (std::nothrow) char[MYSQL_ERRMSG_SIZE];
    if (ebuff) {
      snprintf(ebuff, MYSQL_ERRMSG_SIZE,
               ER_THD(thd, ER_STACK_OVERRUN_NEED_MORE), stack_used,
               my_thread_stack_size, margin);
      my_message(ER_STACK_OVERRUN_NEED_MORE, ebuff, MYF(ME_FATALERROR));
      delete[] ebuff;
    }
    return true;
  }
#ifndef DBUG_OFF
  max_stack_used = std::max(max_stack_used.load(), stack_used);
#endif
  return false;
}
