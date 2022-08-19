/*
   Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_component.h"

#include <stddef.h>
#include <string.h>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/my_service.h"
#include "mysql/components/service.h"
#include "mysql/components/services/persistent_dynamic_loader.h"
#include "mysql/mysql_lex_string.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/mysqld.h"                      // srv_registry
#include "sql/resourcegroups/resource_group_mgr.h"  // Resource_group_mgr
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_class.h"        // THD
#include "sql/sql_plugin.h"       // end_transaction
#include "sql/thd_raii.h"

bool Sql_cmd_install_component::execute(THD *thd) {
  my_service<SERVICE_TYPE(persistent_dynamic_loader)> service_dynamic_loader(
      "persistent_dynamic_loader", srv_registry);
  if (service_dynamic_loader) {
    my_error(ER_COMPONENTS_CANT_ACQUIRE_SERVICE_IMPLEMENTATION, MYF(0),
             "persistent_dynamic_loader");
    return true;
  }

  if (acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  Disable_autocommit_guard autocommit_guard(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  DBUG_EXECUTE_IF("disable_rg_pfs_notifications", {
    auto name = "file://component_test_pfs_notification";
    if (m_urns.size() == 1 && strcmp(name, m_urns[0].str) == 0)
      resourcegroups::Resource_group_mgr::instance()
          ->disable_pfs_notification();
  });

  std::vector<const char *> urns(m_urns.size());
  for (size_t i = 0; i < m_urns.size(); ++i) {
    urns[i] = m_urns[i].str;
  }
  if (service_dynamic_loader->load(thd, urns.data(), m_urns.size())) {
    return (end_transaction(thd, true));
  }

  my_ok(thd);
  return (end_transaction(thd, false));
}

bool Sql_cmd_uninstall_component::execute(THD *thd) {
  my_service<SERVICE_TYPE(persistent_dynamic_loader)> service_dynamic_loader(
      "persistent_dynamic_loader", srv_registry);
  if (service_dynamic_loader) {
    my_error(ER_COMPONENTS_CANT_ACQUIRE_SERVICE_IMPLEMENTATION, MYF(0),
             "persistent_dynamic_loader");
    return true;
  }

  if (acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  Disable_autocommit_guard autocommit_guard(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  std::vector<const char *> urns(m_urns.size());
  for (size_t i = 0; i < m_urns.size(); ++i) {
    urns[i] = m_urns[i].str;
  }
  if (service_dynamic_loader->unload(thd, urns.data(), m_urns.size())) {
    return (end_transaction(thd, true));
  }
  my_ok(thd);
  return (end_transaction(thd, false));
}
