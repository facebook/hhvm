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

#include "mysql_server_runnable_imp.h"
#include <mysql/components/service_implementation.h>
#include <string>

extern int mysqld_main(int argc, char **argv);

/**
  This is the runnable implementation which is used by the test applications
  that loads component_mysql_server library. This api is used to start the
  mysqld server.

  @param argc Name of the program(mysqld server) to execute.
  @param argv variable list, arguments for the program.
*/
DEFINE_METHOD(int, mysql_server_runnable_imp::run, (int argc, char **argv)) {
  if (argc == 0) {
    return 1;
  } else {
    return mysqld_main(argc, argv);
  }
}
