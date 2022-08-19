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

#ifndef SERVICE_H
#define SERVICE_H

/**
 Specific type for the service status return values.

 0 is false, non-zero is true. Corresponds to C++ bool.

 @sa DEFINE_BOOL_METHOD, DECLARE_BOOL_METHOD
*/
typedef int mysql_service_status_t;

/**
  @page PAGE_COMPONENTS_SERVICE A Service and a Service Implementation

  The Service is basic concept of the Components subsystem. A Service is a
  named, well-defined stateless interface to one functionality, expressed as a
  list of pointers to a specific methods. The service name will consist of ASCII
  symbols excluding the dot ('.') symbol. Each Service will be accompanied with
  a (part of a) header file that defines the Service allowing other Components
  to consume it. Each Service should work only on basic C types and structures
  of these types to prevent problems with not fully-defined C++ objects ABI. The
  Service is a default way to operate inside the Components subsystem as a mean
  to show that one is interested only on functionality interface, not its exact
  implementation. The Services are not versioned - any change to interface must
  require Service being copied to one with different name before applying
  changes. The Services by itself do not carry any state, all methods are
  stateless. This does not prevent one from having some state-carrying objects
  created and returned as handles to them. Such concept is shown for example in
  create(), get() and release() methods of the registry_query Service. This
  concept relies on implementation of generator of the Opaque pointers with
  d-pointer described here: https://en.wikipedia.org/wiki/Opaque_pointer .

  For any specific Service a Service Implementation is defined as a structure
  of the Service type filled with pointers to methods of specified
  implementation. The name of the Service Implementation is a name of the
  Service and the implementation related name separated with a dot. In most
  cases the implementation related name should be the Component name in which it
  is being defined. Each Service can have several Service Implementations.

  @file include/mysql/components/service.h
*/

/**
  Generates the standard Service type name. It does not have const specifier,
  it should be used only when really necessary.
*/
#define SERVICE_TYPE_NO_CONST(name) mysql_service_##name##_t

/**
  Generates the standard Service type name.
*/
#define SERVICE_TYPE(name) const SERVICE_TYPE_NO_CONST(name)

/**
  Declares a new Service. It creates a structure for pointers to Service
  methods. A list of methods defined using DECLARE_METHOD and
  DECLARE_BOOL_METHOD macros must follow this macro, with a closing
  END_SERVICE_DEFINITION macro usage.

  @param name Service name, must be a valid C name.
*/
#define BEGIN_SERVICE_DEFINITION(name) typedef struct s_mysql_##name {
/**
  A macro to end the last Service definition started with the
  BEGIN_SERVICE_DEFINITION macro.
*/
#define END_SERVICE_DEFINITION(name) \
  }                                  \
  SERVICE_TYPE_NO_CONST(name);
/**
  Declares a method as a part of the Service definition. To be used within the
  SERVICE_DEFINITION macro.

  @param retval Type of the return value for the method declaration. Should be a
    simple type or structure, not a C++ object with undefined ABI.
  @param name Name of the method, must be a valid C name.
  @param args The list of arguments of the method taken in parentheses.
*/
#define DECLARE_METHOD(retval, name, args) retval(*name) args

/**
  Declares a method that returns bool as a part of the Service definition. To be
  used within the SERVICE_DEFINITION macro.

  @param name Name of the method, must be a valid C name.
  @param args The list of arguments of the method taken in parentheses.
*/
#define DECLARE_BOOL_METHOD(name, args) \
  DECLARE_METHOD(mysql_service_status_t, name, args)

/**
  Defines an object type that is meant for carrying handles to the
  implementation-specific objects used in the Service interface, but without
  their structure exposed, keeping it as an abstraction. This follows a Opaque
  Pointer design pattern, see more: https://en.wikipedia.org/wiki/Opaque_pointer
  If you would like to have a C++ RAII class to manage the resource with
  additional methods to call raw service calls, please create your class
  {handle_name} instead of following macro. Please make sure it does not use any
  virtual methods to keep binary compatibility, and try use only one member, the
  d-pointer to hide all implementation details and keep headers unmodified from
  the point of publishing it.

  @param name Handle name, must be a valid C name.
*/
#define DEFINE_SERVICE_HANDLE(name) typedef struct name##_imp *name

#endif /* SERVICE_H */
