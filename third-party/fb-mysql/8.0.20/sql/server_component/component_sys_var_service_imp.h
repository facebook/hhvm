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

#ifndef COMPONENT_SYSTEM_VAR_SERVICE_H
#define COMPONENT_SYSTEM_VAR_SERVICE_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/component_sys_var_service.h>

void mysql_comp_sys_var_services_init();

/**
  An implementation of the configutration system variables Service to register
  variable and unregister variable.
*/
class mysql_component_sys_variable_imp {
 public:
  /**
    Register's component system variables.
    @param component_name  name of the componentnt
    @param var_name variable name
    @param flags tells about the variable type
    @param comment variable comment message
    @param check_func function pointer, which is called at variable check time
    @param update_func function pointer, which is called at update time
    @param check_arg  type defined check constraints block
    @param variable_value place holder for variable value
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(register_variable,
                            (const char *component_name, const char *var_name,
                             int flags, const char *comment,
                             mysql_sys_var_check_func check_func,
                             mysql_sys_var_update_func update_func,
                             void *check_arg, void *variable_value));

  /**
    Get the component system variable value from the global structure.
    @param component_name Name of the component
    @param var_name Name of the variable
    @param[in,out] val On input: a buffer to hold the value. On output a pointer
    to the value.
    @param[in,out] out_length_of_val On input: size of longest string that the
    buffer can contain. On output the length of the copied string.
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(get_variable,
                            (const char *component_name, const char *var_name,
                             void **val, size_t *out_length_of_val));

  /**
    Unregister's component system variable.
    @param component_name name of the component
    @param var_name Variable name
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(unregister_variable,
                            (const char *component_name, const char *var_name));
};
#endif /* COMPONENT_SYSTEM_VAR_SERVICE_H */
