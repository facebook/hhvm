/* Copyright (c) 2016, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_SERVICE_FOO_INCLUDED

/**
  @file
  TODO: Fill in a description of your file here.
*/
#ifdef __cplusplus
extern "C" {
#endif

/**
  @ingroup group_ext_plugin_services

  TODO: Fill in the architecture of your service.

  This is the primary documentation of your new service
  and will be auto-added to the service description document
  because of it being a part of the doxygen group
  group_ext_plugin_services.
*/
extern struct foo_service_st {
  /**
    TODO: Interface description of foo_func1_type.
    Fix the prototype as appropriate.
    You can add a see-also to the implementation too.
  */
  int (*foo_func1_type)(...);
  /**
    TODO: Interface description of foo_func2_type.
    Fix the prototype as appropriate.
    You can add a see-also to the implementation too.
  */
  void (*foo_func2_type)(...);
} * foo_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define foo_func1(...) foo_service->foo_func1_type(...)
#define foo_func2(...) foo_service->foo_func2_type(...)

#else

int foo_func1_type(...);  /** TODO: fix the prototype as appropriate */
void foo_func2_type(...); /** TODO: fix the prototype as appropriate */

#endif

#ifdef __cplusplus
}
#endif

#define MYSQL_SERVICE_FOO_INCLUDED
#endif
