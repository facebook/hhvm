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

#ifndef SYSTEM_VARIABLE_SOURCE_H
#define SYSTEM_VARIABLE_SOURCE_H

#include <mysql/components/service.h>
#include "system_variable_source_type.h"

/**
  A service to deal with source of system variable. A system variable
  could be set from different sources for eg: Command line, Configuration
  file etc. This service exposes method to get the source information of
  a given system variable.
*/
BEGIN_SERVICE_DEFINITION(system_variable_source)

/**
  Get source information of given system variable.

  @param [in]  name Name of sytem variable in system charset
  @param [in]  length Name length of sytem variable
  @param [out]  source Source of system variable
  @return Status of performance operation
  @retval false Success
  @retval true Failure
*/

DECLARE_BOOL_METHOD(get, (const char *name, unsigned int length,
                          enum enum_variable_source *source));

END_SERVICE_DEFINITION(system_variable_source)

#endif /* SYSTEM_VARIABLE_SOURCE_H */
