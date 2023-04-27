/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

#include "sql/dd/dd_resource_group.h"

#include <memory>

#include "my_dbug.h"                            // DBUG_*
#include "mysqld_error.h"                       // ER_RESOURCE_GROUP_EXISTS
#include "sql/dd/cache/dictionary_client.h"     // dd::cache::Dictionary_client
#include "sql/dd/dd.h"                          // dd::create_object
#include "sql/dd/types/resource_group.h"        // dd::Resource_group
#include "sql/resourcegroups/resource_group.h"  // resourcegroups::Resource_group
#include "sql/sql_class.h"                      // THD
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit

namespace dd {

bool resource_group_exists(dd::cache::Dictionary_client *dd_client,
                           const String_type &resource_group_name,
                           bool *exists) {
  DBUG_TRACE;
  DBUG_ASSERT(exists);

  const dd::Resource_group *resource_group_ptr = nullptr;
  dd::cache::Dictionary_client::Auto_releaser releaser(dd_client);
  if (dd_client->acquire(resource_group_name, &resource_group_ptr)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  *exists = resource_group_ptr != nullptr;

  return false;
}

static bool set_resource_group_attributes(
    dd::Resource_group *resource_group,
    const resourcegroups::Resource_group &res_grp_ref) {
  DBUG_TRACE;

  resource_group->set_name(res_grp_ref.name().c_str());
  resource_group->set_resource_group_type(res_grp_ref.type());
  resource_group->set_resource_group_enabled(res_grp_ref.enabled());

  const resourcegroups::Thread_resource_control *thr_res_ctrl =
      res_grp_ref.controller();
  resource_group->set_cpu_id_mask(thr_res_ctrl->vcpu_vector());
  resource_group->set_thread_priority(thr_res_ctrl->priority());

  return false;
}

bool create_resource_group(THD *thd,
                           const resourcegroups::Resource_group &res_grp_ref) {
  DBUG_TRACE;

  // Check if the same resource group already exists.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Resource_group *resource_group_ptr = nullptr;
  if (thd->dd_client()->acquire(dd::String_type(res_grp_ref.name().c_str(),
                                                res_grp_ref.name().length()),
                                &resource_group_ptr)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  if (resource_group_ptr != nullptr) {
    my_error(ER_RESOURCE_GROUP_EXISTS, MYF(0), res_grp_ref.name().c_str());
    return true;
  }

  // Create new resource group.
  std::unique_ptr<dd::Resource_group> resource_group(
      dd::create_object<dd::Resource_group>());

  // Set attributes of the resource group.
  if (set_resource_group_attributes(resource_group.get(), res_grp_ref))
    return true;  // Error already logged.

  Implicit_substatement_state_guard substatement_guard(thd);

  // Write changes to dictionary.
  if (thd->dd_client()->store(resource_group.get())) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}

bool update_resource_group(THD *thd, const String_type &resource_grp_name,
                           const resourcegroups::Resource_group &res_grp_ref) {
  DBUG_TRACE;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Resource_group *resource_group = nullptr;
  if (thd->dd_client()->acquire_for_modification(resource_grp_name,
                                                 &resource_group)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  if (resource_group == nullptr) {
    my_error(ER_RESOURCE_GROUP_NOT_EXISTS, MYF(0), resource_grp_name.c_str());
    return true;
  }

  // Set attributes of the resource group.
  if (set_resource_group_attributes(resource_group, res_grp_ref))
    return true;  // Error already logged.

  if (thd->dd_client()->update(resource_group)) {
    trans_rollback_stmt(thd);
    // Full rollback we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}

bool drop_resource_group(THD *thd, const String_type resource_grp_name) {
  DBUG_TRACE;

  // Acquire resource group.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Resource_group *resource_group = nullptr;
  if (thd->dd_client()->acquire(resource_grp_name, &resource_group)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }
  if (resource_group == nullptr) {
    my_error(ER_RESOURCE_GROUP_NOT_EXISTS, MYF(0), resource_grp_name.c_str());
    return true;
  }

  Implicit_substatement_state_guard substatement_guard(thd);

  // Drop resource group.
  if (thd->dd_client()->drop(resource_group)) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}
}  // namespace dd
