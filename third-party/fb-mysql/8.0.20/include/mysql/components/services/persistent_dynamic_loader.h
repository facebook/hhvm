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

#ifndef MYSQL_PERSISTENT_DYNAMIC_LOADER_H
#define MYSQL_PERSISTENT_DYNAMIC_LOADER_H

#include <mysql/components/service.h>

/**
  Service for managing the list of loaded Components. It assures the list to be
  persisted in MySQL table.

  This service breaks a little a concept of having only simple types
  as arguments. This possibly could be improved further into process of making
  more system parts implemented as proper components. Once we have a way to
  access (system) tables through services, this would not be needed and
  persistent dynamic loader could be another implementation of dynamic_loader
  service. Also, the THD parameter could be replaced by getting the value of
  the current_thd.
*/
BEGIN_SERVICE_DEFINITION(persistent_dynamic_loader)
/**
  Loads specified group of components by URN, initializes them and
  registers all service implementations present in these components.
  Assures all dependencies will be met after loading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to load in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  service implementation of scheme component loader service for specified
  scheme. If the loading process successes then a group of Components by their
  URN is added to the component table.

  @param thd Current thread execution context.
  @param urns List of URNs of Components to load.
  @param component_count Number of Components on list to load.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(load, (void *thd, const char *urns[], int component_count));
/**
  Unloads specified group of Components by URN, deinitializes them and
  unregisters all service implementations present in these components.
  Assumes, although does not check it, all dependencies of not unloaded
  components will still be met after unloading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to unload in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  service implementation of scheme component loader service for specified
  scheme. If the unloading process successes then a group of Components by
  their URN is added to the component table.

  @param thd Current thread execution context.
  @param urns List of URNs of components to unload.
  @param component_count Number of components on list to unload.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(unload,
                    (void *thd, const char *urns[], int component_count));
END_SERVICE_DEFINITION(persistent_dynamic_loader)

#endif /* MYSQL_PERSISTENT_DYNAMIC_LOADER_H */
