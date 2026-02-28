/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENT_STAUTS_VARIABLE_SERVICE_H
#define COMPONENT_STAUTS_VARIABLE_SERVICE_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/component_status_var_service.h>
#include <mysql/status_var.h>

extern bool add_status_vars(const SHOW_VAR *);
extern void remove_status_vars(SHOW_VAR *);

/**
  An implementation of the component status variables Service to register
  variable and unregister variable.
*/
class mysql_status_variable_registration_imp {
 public:
  /**
    Register status variable.

    @param  status_var fully constructed status variable object.
    @return Status of performed operation
    @retval false success
    @retval true failure

    Note: Please see the components/test/test_status_var_service.cc file,
    to know how to construct status varables for different variable types.
  */
  static DEFINE_BOOL_METHOD(register_variable, (SHOW_VAR * status_var));

  /**
    Unregister's status variable.
    @param  status_var SHOW_VAR object with only the name of the variable,
                       which has to be removed from the global list.
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(unregister_variable, (SHOW_VAR * status_var));
};
#endif /* COMPONENT_STATUS_VARIABLE_SERVICE_H */
