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

#ifndef MYSQL_SERVER_PERSISTENT_DYNAMIC_LOADER_H
#define MYSQL_SERVER_PERSISTENT_DYNAMIC_LOADER_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/persistent_dynamic_loader.h>
#include <mysql/psi/mysql_mutex.h>
#include <atomic>
#include <map>

#include "my_inttypes.h"

/*
  This header file is used in mysql_server component, which is not
  server-enabled, and can't have following declaration acquired by including
  mysql headers.
*/

bool persistent_dynamic_loader_init(void *thd);
void persistent_dynamic_loader_deinit();

/**
  Allows to wrap another Service Implementation of the Dynamic Loader service
  and add ability to store a list of groups of loaded components. It reacts on
  successful invocations of the underlying Dynamic Loader Service Implementation
  methods load() and unload() and saves changes. It also loads during the start
  of the MySQL Server all groups of the Components that were still loaded on
  last MySQL Server shutdown, they are loaded in order of the original loads.
  It assumes that components does not any not fully-deterministic loads of
  another Components, which would break dependencies if they are decided to load
  other Components than last time. The list of groups of Components is stored in
  the 'mysql.component' table. It is valid to unload only part of the group of
  the previously loaded group of Components. In such a situation, as long as all
  dependencies are met, which is assured by the underlying Service
  Implementation, the rest of Components in group should load successfully after
  the MySQL Server restart.
*/
class mysql_persistent_dynamic_loader_imp {
 public:
  /**
    Initializes persistence store, loads all groups of components registered in
    component table. Shouldn't be called multiple times. We assume the order
    specified by group ID is correct one. This should be assured by dynamic
    loader as long as it will not allow to unload the component that has
    dependency on, in case there would be a possibility to switch that
    dependency to other component that is not to be unloaded. If this is
    assured, then it will not be possible for components with lower group IDs
    to have a dependency on component with higher group ID, even after state is
    restored in this initialization method.

    @param thdp Current thread execution context
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static bool init(void *thdp);
  /**
    De-initializes persistence loader.
  */
  static void deinit();

  /**
   Initialisation status of persistence loader. An helper function.
  */
  static bool initialized();

 public: /* service implementations */
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

    @param thd_ptr Current thread execution context.
    @param urns List of URNs of Components to load.
    @param component_count Number of Components on list to load.
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(load, (void *thd_ptr, const char *urns[],
                                   int component_count));

  /**
    Unloads specified group of Components by URN, deinitializes them and
    unregisters all service implementations present in these components.
    Assumes, although does not check it, all dependencies of not unloaded
    components will still be met after unloading specified components.
    The dependencies may be circular, in such case it's necessary to specify
    all components on cycle to unload in one batch. From URNs specified the
    scheme part of URN (part before "://") is extracted and used to acquire
    service implementation of scheme component loader service for specified
    scheme. URN specified should be identical to ones specified in load()
    method, i.e. all letters must have the same case. If the unloading process
    successes then a group of Components by their URN is added to the component
    table.

    @param thd_ptr Current thread execution context.
    @param urns List of URNs of components to unload.
    @param component_count Number of components on list to unload.
    @return Status of performed operation
    @retval false success
    @retval true failure
  */
  static DEFINE_BOOL_METHOD(unload, (void *thd_ptr, const char *urns[],
                                     int component_count));

 private:
  /**
    Stores last group ID used in component table. It is initialized on init()
    on component table scan with maximum group ID used in table.
  */
  static std::atomic<uint64> s_group_id;
  /**
    Stores mapping of component URNs to their component_id used in component
    table, to ease row deletion.
  */
  static std::map<std::string, uint64> component_id_by_urn;
  /**
    Indicates the initialization status of dynamic loader persistence.
  */
  static bool is_initialized;

  /**
    Serializes access to @ref component_id_by_urn
  */
  static mysql_mutex_t component_id_by_urn_mutex;
};

#endif /* MYSQL_SERVER_PERSISTENT_DYNAMIC_LOADER_H */
