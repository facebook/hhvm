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

#ifndef MINIMAL_CHASSIS_H
#define MINIMAL_CHASSIS_H

#include <mysql/components/component_implementation.h>

/**
  This is the entry function for minimal_chassis static library, called by
  the application code.
  Bootstraps service registry and dynamic loader. And registry handle will be
  assigned, if provided empty handle address. And loads provided component
  services into the registry, if provided component reference which is
  statically linked to this library.

  @param [out] registry A service handle to registry service.
  @param [in]  comp_ref A component structure referance name.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
bool minimal_chassis_init(SERVICE_TYPE_NO_CONST(registry) * *registry,
                          mysql_component_t *comp_ref);

/**
  This is the exit function for minimal_chassis static library, called just
  before the exit of the application.
  Releases the service registry and dynamic loader services.
  Releases the registry handle, which is acquired at the time of
  minimal_chassis_init(), if provided the handle address.
  And un-registers the component services, if provided component
  reference which is statically linked to this library.

  @param [in] registry A service handle to registry service.
  @param [in]  comp_ref A component structure referance name.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
bool minimal_chassis_deinit(SERVICE_TYPE_NO_CONST(registry) * registry,
                            mysql_component_t *comp_ref);

/**
  This function refreshes the global service handles based on the use_related
  flag.
  The global services are mysql_runtime_error, mysql_psi_system_v1 and
  mysql_rwlock_v1.
  If the use_related is ON then the globals are leaded with minimal chassis
  service implementations else they are loaded with the default service
  implementations

  @param use_related Used to decide which service implementaion to load
         for globals.
*/
void minimal_chassis_services_refresh(bool use_related);

void mysql_components_handle_std_exception(const char *funcname);
#endif /* MINIMAL_CHASSIS_H */
