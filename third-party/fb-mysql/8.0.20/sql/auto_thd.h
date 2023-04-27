/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef AUTO_THD_H
#define AUTO_THD_H

#include <sys/types.h>

#include "my_compiler.h"
#include "sql/error_handler.h"
#include "sql/sql_error.h"

/**
  Self destroying THD.
*/
class Auto_THD : public Internal_error_handler {
 public:
  /**
    Create THD object and initialize internal variables.
  */
  Auto_THD();

  /**
    Deinitialize THD.
  */
  virtual ~Auto_THD();

  /**
    Error handler that prints error message on to the error log.

    @param thd       Current THD.
    @param sql_errno Error id.
    @param sqlstate  State of the SQL error.
    @param level     Error level.
    @param msg       Message to be reported.

    @return This function always return false.
  */
  virtual bool handle_condition(class THD *thd MY_ATTRIBUTE((unused)),
                                uint sql_errno MY_ATTRIBUTE((unused)),
                                const char *sqlstate MY_ATTRIBUTE((unused)),
                                Sql_condition::enum_severity_level *level
                                    MY_ATTRIBUTE((unused)),
                                const char *msg);

  /** Thd associated with the object. */
  class THD *thd;
};

#endif  // AUTO_THD_H
