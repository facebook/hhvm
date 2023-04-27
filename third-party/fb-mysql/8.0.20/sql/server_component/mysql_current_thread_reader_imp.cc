/* Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/server_component/mysql_current_thread_reader_imp.h"

#include <mysql/components/minimal_chassis.h>
#include <mysql/components/service_implementation.h>
#include <sql/current_thd.h>

/**
  Return current thd

  @param[out] thd The placeholder to return the current THD in.
  @retval false success
  @retval true failure

  @sa mysql_service_mysql_current_thread_reader_t
*/
DEFINE_BOOL_METHOD(mysql_component_mysql_current_thread_reader_imp::get,
                   (MYSQL_THD * thd)) {
  try {
    if (thd) *thd = static_cast<MYSQL_THD>(current_thd);
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return false;
}
