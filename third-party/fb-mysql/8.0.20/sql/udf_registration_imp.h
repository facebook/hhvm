/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#if !defined(UDF_REGISTRATION_IMP_H)
#define UDF_REGISTRATION_IMP_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/udf_registration.h>

struct udf_func;

/**
  A helper class for the implementation of the
  udf_registration and udf_aggregate functions

  Needed because we register these service implementations
  as part of the mysql_server component.
*/
class mysql_udf_registration_imp {
 public: /* service implementations */
  /** udf_registration::udf_register */
  static DEFINE_BOOL_METHOD(udf_register,
                            (const char *name, Item_result return_type,
                             Udf_func_any func, Udf_func_init init_func,
                             Udf_func_deinit deinit_func));

  /** udf_registration::udf_unregister
      and udf_registration_aggregate::udf_register
  */
  static DEFINE_BOOL_METHOD(udf_unregister,
                            (const char *name, int *was_present));

  /** udf_registration_aggregate::udf_register */
  static DEFINE_BOOL_METHOD(udf_register_aggregate,
                            (const char *func_name,
                             enum Item_result return_type, Udf_func_any func,
                             Udf_func_init init_func,
                             Udf_func_deinit deinit_func, Udf_func_add add_func,
                             Udf_func_clear clear_func));

 private:
  static bool udf_register_inner(udf_func *func);
  static udf_func *alloc_udf(const char *func_name,
                             enum Item_result return_type, Udf_func_any func,
                             Udf_func_init init_func,
                             Udf_func_deinit deinit_func);
};

#endif
