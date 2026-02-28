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

#ifndef UDF_METADATA_IMP_H
#define UDF_METADATA_IMP_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/udf_metadata.h>
#include <mysql/udf_registration_types.h>

extern REQUIRES_SERVICE_PLACEHOLDER(mysql_udf_metadata);

/**
  A helper class for the implementation of the udf_extension functions

  Needed because we register these service implementations
  as part of the mysql_server component.
*/
class mysql_udf_metadata_imp {
 public: /* service implementations */
  /* Retrieves the extension argument from a UDF argument. */
  static DEFINE_BOOL_METHOD(argument_get,
                            (UDF_ARGS * udf_args, const char *extension_type,
                             unsigned int index, void **out_value));

  /* Retrieves the extension argument from return value of UDF. */
  static DEFINE_BOOL_METHOD(result_get,
                            (UDF_INIT * udf_init, const char *extension_type,
                             void **argument));
  /* Sets the extension argument for a UDF argument. */
  static DEFINE_BOOL_METHOD(argument_set,
                            (UDF_ARGS * udf_args, const char *extension_type,
                             unsigned int index, void *in_value));

  /* Sets the extension argument for a return value of UDF. */
  static DEFINE_BOOL_METHOD(result_set,
                            (UDF_INIT * udf_init, const char *extension_type,
                             void *argument));
};

#endif  // UDF_METADATA_IMP_H
