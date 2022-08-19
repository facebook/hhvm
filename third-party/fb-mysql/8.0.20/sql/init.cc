/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file

  @brief
  Sets up a few global variables.
*/

#include "sql/init.h"

#include "my_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "m_string.h"  // my_stpcpy
#include "my_dbug.h"
#include "my_sys.h"
#include "my_time.h"     // my_init_time
#include "sql/mysqld.h"  // connection_events_loop_aborted(), ...

#ifdef _WIN32
#include <process.h>  // getpid
#endif

void unireg_init(ulong options) {
  DBUG_TRACE;

  error_handler_hook = my_message_stderr;
  set_connection_events_loop_aborted(false);

  current_pid = (ulong)getpid(); /* Save for later ref */
  my_init_time();                /* Init time-functions (read zone) */

  (void)my_stpcpy(reg_ext, ".frm");
  reg_ext_length = 4;
  specialflag = options; /* Set options from argv */
}
