/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_SERVICE_PLUGIN_REGISTRY_INCLUDED
/**
  @file
  Declaration of the registry plugin service
*/

#include <mysql/components/services/registry.h>

/**
  @ingroup group_ext_plugin_services
  A bridge service allowing plugins to work with the registry.

  This allows traditional MySQL plugins to interact with the service
  registry.
  All this plugins service does is to return a reference to the
  registry service.
  Using that the plugins can access the rest of the registry and
  dynamic loaders services, as well as other services present in the
  registry.
  Note that the plugins must release the service references they
  acquire otherwise resources will be leaked and normal unload order
  may be affected.
*/
extern "C" struct plugin_registry_service_st {
  /**
    Acquire a pointer to the registry service

    The reference must be released by calling
    plugin_registry_service_st::mysql_plugin_registry_release_func()
    See @ref mysql_plugin_registry_acquire() for more details.

    Once you receive the registry pointer you can use it to aquire
    references to other services your plugin might be interested in.

    @note
    This is to be considered an "expensive" operation because it
    requires access to the global structures of the
    @ref PAGE_COMPONENTS_REGISTRY. Avoid using it in situations
    where fast and scalable execution is requred.
    Since the registry service is very unlikely to change often
    holding on to the reference to it for extended time periods
    is a safe bet.

    @note
    Achieveing scalability through preserving references does not
    come for free.
    These are some of the effects on code that caches active
    references:
    - components implementing services to which active references
    are held cannot be unloaded.
    - code keeping an active refernece to e.g. a default service
    implementation will not switch to a possible new default
    service implementation installed by a component loaded in
    the meanwhile, as taking the updated default service implementation
    would only happen at the time of aquiring a new reference.

    @return the registry pointer

    See also: @ref PAGE_COMPONENTS, @ref PAGE_COMPONENTS_REGISTRY,
    @ref mysql_plugin_registry_acquire(), @ref mysql_plugin_registry_release()
  */
  SERVICE_TYPE(registry) * (*mysql_plugin_registry_acquire_func)();
  /**
    Release a pointer to the registry service

    Releases the reference to the registry service, as returned by
    @ref mysql_plugin_registry_acquire().
    After this call the reigstry_ptr is undefined and
    should not be used anymore.
    See @ref mysql_plugin_registry_release() for more details.

    @warning
    Before releasing the reference to the registry service please
    make sure you have released all the other service references
    that you explicitly or implicitly acquired. These can't be
    released without a valid reference to the registry service.

    @note
    This is also to be considered an "expensive" operation.
    See @ref plugin_registry_service_st::mysql_plugin_registry_acquire_func
    for more details on pros and cons of re-acquiring references vs caching
    and reusing them.

    @param registry_ptr   the registry pointer
    @return execution status
    @retval 0 success
    @retval non-zero failure

    See also @ref PAGE_COMPONENTS, @ref PAGE_COMPONENTS_REGISTRY,
    @ref mysql_plugin_registry_release(), @ref mysql_plugin_registry_acquire()
  */
  int (*mysql_plugin_registry_release_func)(SERVICE_TYPE(registry) *
                                            registry_ptr);
} * plugin_registry_service;

#ifdef MYSQL_DYNAMIC_PLUGIN
#define mysql_plugin_registry_acquire() \
  plugin_registry_service->mysql_plugin_registry_acquire_func()
#define mysql_plugin_registry_release(r) \
  plugin_registry_service->mysql_plugin_registry_release_func(r)
#else
SERVICE_TYPE(registry) * mysql_plugin_registry_acquire();
int mysql_plugin_registry_release(SERVICE_TYPE(registry) *);
#endif

#define MYSQL_SERVICE_PLUGIN_REGISTRY_INCLUDED
#endif /* MYSQL_SERVICE_PLUGIN_REGISTRY_INCLUDED */
