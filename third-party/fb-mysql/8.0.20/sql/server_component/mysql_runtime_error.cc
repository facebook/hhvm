/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <my_sys.h>
#include <mysql/components/service_implementation.h>
#include <stdarg.h>
#include "mysql_runtime_error_imp.h"

/**
  This is the override implementation for emit api to call the mysql server
  error report functions.

  @param id    error ID, used to fetch the error message.
  @param flags this will tell, whether the error is a fatal statement error or
               write the error to error log file.
  @param args  va_list type, which hold the error message format string.
*/
DEFINE_METHOD(void, mysql_server_runtime_error_imp::emit,
              (int id, int flags, va_list args)) {
  const char *format;

  if (!(format = my_get_err_msg(id))) {
    char ebuff[32];
    (void)snprintf(ebuff, sizeof(ebuff), "Unknown error %d", id);
    my_message((unsigned int)id, ebuff, flags);
    return;
  }
  my_printv_error(id, format, flags, args);
}
