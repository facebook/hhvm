/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SERVICE_IMPLEMENTATION_H
#define SERVICE_IMPLEMENTATION_H

#include "service.h"

/**
  @file include/mysql/components/service_implementation.h
  Specifies macros to define Service Implementations.
*/

/**
  Reference to the name of the service implementation variable

  @ref BEGIN_SERVICE_IMPLEMENTATION defines a local variable of
  the structure type for the service (see @ref SERVICE_TYPE).
  The variable is then used by the @ref PROVIDES_SERVICE macro to
  construct the list of service provided by the component.
  This macro allows one to use @ref BEGIN_SERVICE_IMPLEMENTATION ,
  @ref DEFINE_METHOD and @ref END_SERVICE_IMPLEMENTATION macros to build
  a service defintion structure and variable to be used outside of the
  component definition context.

  @param component Name of the implementation of the service.
    Usually a component name.
  @param service  Name of the service to create the implementation for.

*/
#define SERVICE_IMPLEMENTATION(component, service) imp_##component##_##service

/**
  Declares a Service Implementation. It builds standard implementation
  info structure. Only a series of pointers to methods the Service
  Implementation implements as respective Service methods are expected to be
  used after this macro and before the END_SERVICE_IMPLEMENTATION counterpart.

  @param component Name of the Component to create implementation in.
  @param service Name of the Service to create implementation for.
*/
#define BEGIN_SERVICE_IMPLEMENTATION(component, service) \
  SERVICE_TYPE(service) SERVICE_IMPLEMENTATION(component, service) = {
/**
  A macro to end the last declaration of a Service Implementation.
*/
#define END_SERVICE_IMPLEMENTATION() }

/**
  A macro to ensure method implementation has required properties, that is it
  does not throw exceptions and is static. This macro should be used with
  exactly the same arguments as DECLARE_METHOD.

  @param retval Type of return value. It cannot contain comma characters, but
    as only simple structures are possible, this shouldn't be a problem.
  @param name Method name.
  @param args a list of arguments in parenthesis.
*/
#define DEFINE_METHOD(retval, name, args) retval name args noexcept

/**
  A short macro to define method that returns bool, which is the most common
  case.

  @param name Method name.
  @param args a list of arguments in parenthesis.
*/
#define DEFINE_BOOL_METHOD(name, args) \
  DEFINE_METHOD(mysql_service_status_t, name, args)

#endif /* SERVICE_IMPLEMENTATION_H */
