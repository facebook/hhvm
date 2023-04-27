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

#ifndef MYSQL_RUNTIME_ERROR_SERVICE_H
#define MYSQL_RUNTIME_ERROR_SERVICE_H

//! @cond Doxygen_Suppress
#include <mysql/components/component_implementation.h>
//! @endcond
#include <mysql/components/service.h>
#include <mysql/components/services/mysql_runtime_error.h>
//! @cond Doxygen_Suppress
#include <stdarg.h>
//! @endcond

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error);

/**
  This function is substitute api service for my_error function.
  To use this api, mysql_runtime_error service has to be acquired by the caller.
  To use this api, components has to define
  REQUIRES_SERVICE(mysql_runtime_error) and pass it to this api.
*/
inline void mysql_error_service_emit_printf(SERVICE_TYPE(mysql_runtime_error) *
                                                error_svc_handle,
                                            int error_id, int flags, ...) {
  va_list args;
  va_start(args, flags);
  error_svc_handle->emit(error_id, flags, args);
  va_end(args);
}

/**
  This function can be used in components code as a replacement for my_error()
  server function.
  To use this api, components has to define
  REQUIRES_SERVICE_PLACEHOLDER(mysql_runtime_error) and
  REQUIRES_SERVICE(mysql_runtime_error).
*/
inline void mysql_error_service_printf(int error_id, int flags, ...) {
  va_list args;
  va_start(args, flags);
  mysql_service_mysql_runtime_error->emit(error_id, flags, args);
  va_end(args);
}

typedef int myf; /* Type of MyFlags in my_funcs */

/* Macros for converting *constants* to the right type */
#define MYF(v) (myf)(v)

#ifndef MYSQL_SERVER
#define my_error mysql_error_service_printf
#endif

#endif /* MYSQL_RUNTIME_ERROR_SERVICE_H */
