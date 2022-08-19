/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UDF_METADATA_H
#define UDF_METADATA_H

#include <mysql/components/service.h>
#include <mysql/udf_registration_types.h>

/**
  Service for getting and setting the extension attributes of UDF arguments
  and return value.
*/
BEGIN_SERVICE_DEFINITION(mysql_udf_metadata)

/**
  Retrieves extension attributes of a UDF argument. The extension attributes
  are retrieved based on the extension_type. Following extension_type are
  supported as of now :

  (1) charset - Character set name of a UDF argument
  (2) collation - Collation name of a UDF argument.

  The method returns the requested extension attribute in the output
  parameter. One must cast the output value to a respective data type.
  It sets error message if it is unable to retrieve extension attribute

  One could retrieve the charset of first UDF argument as following.

  void *out_value = nullptr;
  my_service<SERVICE_TYPE(mysql_udf_metadata)> service("mysql_udf_metadata",
                                              mysql_plugin_registry_acquire());
  service->argument_get(udf_args, "charset", 0, &out_value);
  const char *charset_name = static_cast<const char *>(out_value);

  @param [in]   udf_args  Pointer to the UDF arguments struct that contains the
                          extension pointer.
  @param [in]   extension_type  Argument type to get
  @param [in]   index           Index of UDF argument
  @param [out]  out_value Pointer to the character set

  @return
    true  Error conditions. For instance :
          1. invalid argument type 'extension_type'
          2. null extension pointer
    false Otherwise
*/
DECLARE_BOOL_METHOD(argument_get,
                    (UDF_ARGS * udf_args, const char *extension_type,
                     unsigned int index, void **out_value));

/**
  Retrieves the extension attribute of UDF return value.
  a UDF argument. The extension attributes are retrieved based on the
  extension_type. Following extension_type are supported:

  (1) charset - Character set name of a UDF argument
  (2) collation - Collation name of a UDF argument.

  The method returns the requested extension attribute in the output
  parameter. One must cast the output value to a respective data type.
  It sets error message if it is unable to retrieve extension attribute

  One could retrieve the charset of return value as following.

  void *out_value = nullptr;
  my_service<SERVICE_TYPE(mysql_udf_metadata)> service("mysql_udf_metadata",
                                              mysql_plugin_registry_acquire());
  service->result_get(udf_init, "charset", 0, &out_value);
  const char *charset_name = static_cast<const char *>(out_value);

  @param [in]   udf_init  Pointer to the UDF_INIT struct struct that contains
                          the extension pointer for return value
  @param [in]   extension_type  Argument type to get
  @param [out]  out_value Pointer to the arguments.

  @return
    true  Error conditions. For instance :
          1. invalid argument type 'extension_type'
          2. null extension pointer
    false Otherwise
*/
DECLARE_BOOL_METHOD(result_get, (UDF_INIT * udf_init,
                                 const char *extension_type, void **out_value));
/**
  Sets the extension attribute of a UDF argument. The extension attribute
  could be set only for supported extension type. Following extension_type
  are supported:

  (1) charset - Character set name of a UDF argument
  (2) collation - Collation name of a UDF argument.

  The method sets the input value as the extension attribute of corresponding
  UDF argument. It sets error message if it is unable to set the extension
  attribute.

  One could set the charset of first argument as following.

  const char* name = "utf8mb4";
  char *value = const_cast<char*>(name);
  my_service<SERVICE_TYPE(mysql_udf_metadata)> service("mysql_udf_metadata",
                                              mysql_plugin_registry_acquire());
  service->argument_set(udf_args, "charset", 0, static_cast<void *>(value));

  @param [in, out]  udf_args  Pointer to the UDF arguments struct that contains
                              the extension pointer.
  @param [in] extension_type  Argument type to set.
  @param [in] arguments       Pointer to input arguments to set.If it is a
                              char* then that must be null terminated.

  @return
    true  Error conditions. For instance :
          1. invalid argument type 'extension_type'
          2. null extension pointer
    false Otherwise
*/
DECLARE_BOOL_METHOD(argument_set,
                    (UDF_ARGS * udf_args, const char *extension_type,
                     unsigned int index, void *in_value));

/**
  Sets the extension attribute of the UDF return value. The extension attribute
  could be set only for supported extension type. Following extension_type
  are supported:

  (1) charset - Character set name of a UDF argument
  (2) collation - Collation name of a UDF argument.

  The method sets the input value as the extension attribute of return value.
  It sets error message if it is unable to set the extension attribute.

  One could set the charset of return value as following.

  const char* name = "utf8mb4";
  char *value = const_cast<char*>(name);
  my_service<SERVICE_TYPE(mysql_udf_metadata)> service("mysql_udf_metadata",
                                              mysql_plugin_registry_acquire());
  service->result_set(udf_init, "charset", 0, static_cast<void *>(value));

  @param [in, out]  udf_init  Pointer to UDF_INIT argument that contains the
                              extension  pointer.
  @param [in] extension_type  Argument type that has to be set as an extension
                              argument. If it is invalid then the method does
                              nothing.
  @param [in] in_value        Input argument that has to be read. If it is a
                              char* then that must be null terminated.
  @return
    true  Error conditions. For instance :
          1. invalid argument type 'extension_type'
          2. null extension pointer
    false Otherwise
*/
DECLARE_BOOL_METHOD(result_set, (UDF_INIT * udf_init,
                                 const char *extension_type, void *in_value));

END_SERVICE_DEFINITION(mysql_udf_metadata)

#endif  // UDF_METADATA_H
