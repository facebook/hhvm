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

#include "host_application_signal_imp.h"
#include "my_config.h"
#include "mysqld_error.h"

#include <mysql/components/minimal_chassis.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/log_builtins.h>
#include <mysql_version.h>
#include <sql/mysqld.h>

/**
  Process signals for the mysql server binary.

  Signals accepted:
  - @ref HOST_APPLICATION_SIGNAL_SHUTDOWN Initiate a normal server
    shutdown by calling kill_mysql(). Ignore the argument. Also log
    a message to the server log at start.

  @param signal_no one of @ref host_application_signal_signals
  @param arg Signal argument. Currently ignored.
  @retval false success
  @retval true failure

  @sa mysql_service_host_application_signal_t
*/
DEFINE_BOOL_METHOD(mysql_component_host_application_signal_imp::signal,
                   (int signal_no, void * /* arg */)) {
  bool retval = false;
  try {
    switch (signal_no) {
      case HOST_APPLICATION_SIGNAL_SHUTDOWN:
        LogErr(SYSTEM_LEVEL, ER_SERVER_SHUTDOWN_INFO,
               "<internal signal component service>", server_version,
               MYSQL_COMPILATION_COMMENT_SERVER);
        kill_mysql();
        break;
      default:
        retval = true;
        break;
    }
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return retval;
}
