/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQLBINLOG_INCLUDED
#define MYSQLBINLOG_INCLUDED

#include <stdarg.h>
#include <sys/types.h>

#include "my_compiler.h"
#include "my_inttypes.h"

extern bool force_opt;
extern bool short_form;
extern ulong opt_server_id_mask;
extern ulong opt_binlog_rows_event_max_size;

/*
  error() is used in macro BINLOG_ERROR which is invoked in
  rpl_gtid.h, hence the early forward declaration.
*/
void error(const char *format, ...) MY_ATTRIBUTE((format(printf, 1, 2)));
void warning(const char *format, ...) MY_ATTRIBUTE((format(printf, 1, 2)));
void error_or_warning(const char *format, va_list args, const char *msg)
    MY_ATTRIBUTE((format(printf, 1, 0)));
void sql_print_error(const char *format, ...)
    MY_ATTRIBUTE((format(printf, 1, 2)));

#endif  // MYSQLBINLOG_INCLUDED
