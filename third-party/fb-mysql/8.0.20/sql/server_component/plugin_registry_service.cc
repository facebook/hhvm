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

#include <mysql/service_plugin_registry.h>
#include "sql/mysqld.h"  // srv_registry

/**
   Returns a new reference to the "registry" service.

   Implementation of
   @ref plugin_registry_service_st::mysql_plugin_registry_acquire_func

   Uses the global registry instance to search for the default implementation
   of the "registry" service by calling
   @ref mysql_service_registry_t::acquire().

   The reference must be released through calling
   @ref mysql_plugin_registry_release() to avoid resource leaks.

   @return The newly acquired registry service pointer.
   @retval NULL error. @ref mysql_service_registry_t::acquire() failed.

   See also @ref mysql_plugin_registry_release(),
   @ref PAGE_COMPONENTS, @ref PAGE_COMPONENTS_REGISTRY,
   @ref mysql_service_registry_t::acquire()
*/
SERVICE_TYPE(registry) * mysql_plugin_registry_acquire() {
  my_h_service registry_handle;
  if (srv_registry->acquire("registry", &registry_handle)) {
    return nullptr;
  }
  return reinterpret_cast<SERVICE_TYPE(registry) *>(registry_handle);
}

/**
  Releases a registry service reference

  Implementation of
  @ref plugin_registry_service_st::mysql_plugin_registry_release_func

  Uses the global registry instance to release a service reference to
  the "registry" service passed as a parameter by calling
  @ref mysql_service_registry_t::release()
  This is the reverse of mysql_plugin_registry_acquire().

  @param reg the registry service handle to release
  @return the result of mysql_service_registry_t::release()
  @retval 0 Success
  @retval non-zero Failure

  See also @ref mysql_plugin_registry_release(),
  @ref PAGE_COMPONENTS, @ref PAGE_COMPONENTS_REGISTRY,
  @ref mysql_service_registry_t::release()
*/
int mysql_plugin_registry_release(SERVICE_TYPE(registry) * reg) {
  using service_type_t = SERVICE_TYPE_NO_CONST(registry);
  return srv_registry->release(
      reinterpret_cast<my_h_service>(const_cast<service_type_t *>(reg)));
}
