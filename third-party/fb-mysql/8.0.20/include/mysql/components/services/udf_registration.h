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

#ifndef UDF_REGISTRATION_H
#define UDF_REGISTRATION_H

#include <mysql/components/service.h>
#include <mysql/udf_registration_types.h>

/**
  Service for adding and removing UDF functions
*/
BEGIN_SERVICE_DEFINITION(udf_registration)
/**
  Registers a UDF function with a given name.

  The name must be unique. Does not store in any table, just
  updates the global function list.
  Plugins/components need to handle registration deregistration
  during their initialization and deinitialization.
  Registers a scalar UDF by default.
  @sa udf_aggregates

  @param name name of the function
  @param return_type return type.
  @param func function to call
  @param init_func function to call at query start
  @param deinit_func function to call at query cleanup
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(udf_register,
                    (const char *func_name, enum Item_result return_type,
                     Udf_func_any func, Udf_func_init init_func,
                     Udf_func_deinit deinit_func));

/**
  Unregisters a UDF function with a given name.

  Does not store in any table, just updates the global function list.
  Plugins/components need to handle registration deregistration
  during their initialization and deinitialization.

  @param name name of the function
  @param[out] was_present set to non-zero if the UDF was present, but locked
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(udf_unregister, (const char *name, int *was_present));
END_SERVICE_DEFINITION(udf_registration)

/**
  Service for turning
*/
BEGIN_SERVICE_DEFINITION(udf_registration_aggregate)
/**
  Registers an aggregate UDF function with a given name.

  The name must be unique. Does not store in any table, just
  updates the global function list.
  Plugins/components need to handle registration deregistration
  during their initialization and deinitialization.
  You can use udf_registration::unregister to unregister the
  function.

  @sa udf_registration

  @param name name of the function
  @param return_type return type.
  @param func function to call
  @param init_func function to call at query start
  @param deinit_func function to call at query cleanup
  @param add_func function to call at adding a row to the current aggregate
  @param clear_func function to call at the start of a new aggregate group
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(udf_register,
                    (const char *func_name, enum Item_result return_type,
                     Udf_func_any func, Udf_func_init init_func,
                     Udf_func_deinit deinit_func, Udf_func_add add_func,
                     Udf_func_clear clear_func));

/**
  Unregisters an aggregate UDF function with a given name.

  Does not store in any table, just updates the global function list.
  Plugins/components need to handle registration deregistration
  during their initialization and deinitialization.

  Currently it's a synonym for udf_registration::udf_unregister.

  @param name name of the function
  @param[out] was_present set to non-zero if the UDF was present, but locked
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(udf_unregister, (const char *name, int *was_present));
END_SERVICE_DEFINITION(udf_registration_aggregate)
#endif
