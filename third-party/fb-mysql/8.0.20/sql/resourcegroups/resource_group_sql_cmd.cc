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

#include "sql/resourcegroups/resource_group_sql_cmd.h"

#include <stdint.h>
#include <string.h>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "pfs_thread_provider.h"
#include "sql/auth/auth_acls.h"    // SUPER_ACL
#include "sql/auth/auth_common.h"  // check_readonly
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"                 // current_thd
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_resource_group.h"        // resource_group_exists, etc.
#include "sql/dd/string_type.h"              // String_type
#include "sql/derror.h"                      // ER_THD
#include "sql/lock.h"                        // acquire_shared_global...
#include "sql/mdl.h"
#include "sql/mysqld_thd_manager.h"  // Find_thd_with_id
#include "sql/parse_tree_helpers.h"
#include "sql/resourcegroups/resource_group.h"           // Resource_group
#include "sql/resourcegroups/resource_group_mgr.h"       // Resource_group_mgr
#include "sql/resourcegroups/thread_resource_control.h"  // Thread_resource_control
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_class.h"        // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"  // is_invalid_string
#include "sql/system_variables.h"
#include "sql/thd_raii.h"

namespace dd {
class Resource_group;
}  // namespace dd

namespace {
/**
  Acquire an exclusive MDL lock on resource group name.

  @param  thd  Pointer to THD context.
  @param  res_grp_name  Resource group name.

  @return true if lock acquisition failed else false.
*/

static bool acquire_exclusive_mdl_for_resource_group(THD *thd,
                                                     const char *res_grp_name) {
  DBUG_TRACE;

  MDL_key mdl_key;
  dd::Resource_group::create_mdl_key(res_grp_name, &mdl_key);

  MDL_request mdl_request;
  MDL_REQUEST_INIT_BY_KEY(&mdl_request, &mdl_key, MDL_EXCLUSIVE,
                          MDL_TRANSACTION);
  if (thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  return false;
}

/**
  Validate CPU id ranges provided by a user in the statements
  CREATE RESOURCE GROUP, ALTER RESOURCE GROUP

  @param[out] vcpu_range_vector  vector of validated resourcegroups::Range
                                 objects.
  @param      cpu_list           Array of resourcegroups::Range objects
                                 representing CPU identifiers or ranges of
                                 CPU identifiers specified in the statements
                                 CREATE RESOURCE GROUP, ALTER RESOURCE GROUP.
  @param      num_vcpus          Number of VCPUS.
*/

bool validate_vcpu_range_vector(
    std::vector<resourcegroups::Range> *vcpu_range_vector,
    const Mem_root_array<resourcegroups::Range> *cpu_list, uint32_t num_vcpus) {
  DBUG_TRACE;
  for (auto vcpu_range : *cpu_list) {
    if (vcpu_range.m_start > vcpu_range.m_end) {
      my_error(ER_INVALID_VCPU_RANGE, MYF(0), vcpu_range.m_start,
               vcpu_range.m_end);
      return true;
    }

    if (vcpu_range.m_start >= num_vcpus || vcpu_range.m_end >= num_vcpus) {
      my_error(ER_INVALID_VCPU_ID, MYF(0),
               vcpu_range.m_start >= num_vcpus ? vcpu_range.m_start
                                               : vcpu_range.m_end);
      return true;
    }

    vcpu_range_vector->emplace_back(
        resourcegroups::Range(vcpu_range.m_start, vcpu_range.m_end));
  }
  return false;
}

/**
  This class represents a functional call to move a thread specified by
  pfs_thread_id to a resource group specified in class' constructor.
*/

class Move_thread_to_default_group {
 public:
  explicit Move_thread_to_default_group(
      resourcegroups::Resource_group *resource_group)
      : m_resource_group(resource_group) {}

  void operator()(ulonglong pfs_thread_id) {
    auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();
    auto applied_res_grp =
        m_resource_group->type() == resourcegroups::Type::SYSTEM_RESOURCE_GROUP
            ? res_grp_mgr->sys_default_resource_group()
            : res_grp_mgr->usr_default_resource_group();
    PSI_thread_attrs pfs_thread_attr;

    memset(&pfs_thread_attr, 0, sizeof(pfs_thread_attr));
    if (!res_grp_mgr->get_thread_attributes(&pfs_thread_attr, pfs_thread_id)) {
      applied_res_grp->controller()->apply_control(
          pfs_thread_attr.m_thread_os_id);
      res_grp_mgr->set_res_grp_in_pfs(applied_res_grp->name().c_str(),
                                      applied_res_grp->name().length(),
                                      pfs_thread_id);
      if (!pfs_thread_attr.m_system_thread) {
        Find_thd_with_id find_thd_with_id(pfs_thread_attr.m_processlist_id);
        THD *thd =
            Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
        if (thd != nullptr) {
          thd->resource_group_ctx()->m_cur_resource_group = nullptr;
          mysql_mutex_assert_owner(&thd->LOCK_thd_data);
          mysql_mutex_unlock(&thd->LOCK_thd_data);
        }
      }
    }
  }

 private:
  resourcegroups::Resource_group *m_resource_group;
};

/**
  Check if given resource group name is a default resource group.
*/

inline bool is_default_resource_group(const char *res_grp_name) {
  return my_strcasecmp(system_charset_info, "USR_default", res_grp_name) == 0 ||
         my_strcasecmp(system_charset_info, "SYS_default", res_grp_name) == 0;
}

}  // Anonymous namespace

bool resourcegroups::Sql_cmd_create_resource_group::execute(THD *thd) {
  DBUG_TRACE;

  if (check_readonly(thd, true)) return true;

  Security_context *sctx = thd->security_context();
  if (!sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "RESOURCE_GROUP_ADMIN");
    return true;
  }

  // Resource group name validation.
  if (is_invalid_string(m_name, system_charset_info)) return true;

  // VCPU IDs list validation.
  uint32_t num_vcpus =
      resourcegroups::Resource_group_mgr::instance()->num_vcpus();
  DBUG_PRINT("info", ("Number of VCPUS: %u", num_vcpus));

  auto vcpu_range_vector = std::unique_ptr<std::vector<Range>>(
      new (std::nothrow) std::vector<Range>);
  if (vcpu_range_vector == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
    return true;
  }

  if (validate_vcpu_range_vector(vcpu_range_vector.get(), m_cpu_list,
                                 num_vcpus))
    return true;

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  // Acquire exclusive lock on the resource group name.
  if (acquire_exclusive_mdl_for_resource_group(thd, m_name.str)) return true;

  auto res_grp_mgr = Resource_group_mgr::instance();
  // Check whether resource group exists in-memory.
  if (res_grp_mgr->get_resource_group(m_name.str) != nullptr) {
    my_error(ER_RESOURCE_GROUP_EXISTS, MYF(0), m_name.str);
    return true;
  }

  bool resource_group_exists;
  // Check the disk also for existence of resource group.
  if (dd::resource_group_exists(thd->dd_client(), dd::String_type(m_name.str),
                                &resource_group_exists))
    return true;

  if (resource_group_exists) {
    my_error(ER_RESOURCE_GROUP_EXISTS, MYF(0), m_name.str);
    return true;
  }

  auto resource_group_ptr = res_grp_mgr->create_and_add_in_resource_group_hash(
      m_name, m_type, m_enabled, std::move(vcpu_range_vector), m_priority);

  if (resource_group_ptr == nullptr) return true;

  Disable_autocommit_guard autocommit_guard(thd);
  if (dd::create_resource_group(thd, *resource_group_ptr)) {
    Resource_group_mgr::instance()->remove_resource_group(
        std::string(m_name.str));
    return true;
  }

  my_ok(thd);
  return false;
}

/**
  Check if a resource group specified by a name is present in memory.
  Load a resource group from the Data Dictionary if it missed in memory.

  @param  thd  THD context.
  @param  resource_group_name  Resource group name.

  @return nullptr in case of error, else return a pointer to an instance of
  class resourcegroups::Resource_group.

  @note in case nullptr is returned an error is set in Diagnostics_area.
*/

static inline resourcegroups::Resource_group *check_and_load_resource_group(
    THD *thd, const LEX_CSTRING &resource_group_name) {
  resourcegroups::Resource_group *resource_group =
      resourcegroups::Resource_group_mgr::instance()->get_resource_group(
          resource_group_name.str);

  if (resource_group == nullptr) {
    // Check if resource group exists on-disk.
    bool exists;
    if (dd::resource_group_exists(thd->dd_client(),
                                  dd::String_type(resource_group_name.str),
                                  &exists))
      // Error is reported by the dictionary subsystem.
      return nullptr;

    if (!exists) {
      my_error(ER_RESOURCE_GROUP_NOT_EXISTS, MYF(0), resource_group_name.str);
      return nullptr;
    }

    const dd::Resource_group *dd_resource_group = nullptr;
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    if (thd->dd_client()->acquire(dd::String_type(resource_group_name.str),
                                  &dd_resource_group))
      // Error is reported by the dictionary subsystem.
      return nullptr;

    resource_group = resourcegroups::Resource_group_mgr::instance()
                         ->deserialize_resource_group(dd_resource_group);
    if (resource_group == nullptr) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
      return nullptr;
    }
  }

  return resource_group;
}

bool resourcegroups::Sql_cmd_alter_resource_group::execute(THD *thd) {
  DBUG_TRACE;

  if (check_readonly(thd, true)) return true;

  Security_context *sctx = thd->security_context();
  if (!sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "RESOURCE_GROUP_ADMIN");
    return true;
  }

  // Resource group name validation.
  if (is_invalid_string(m_name, system_charset_info)) return true;

  // Disallow altering USR_default & SYS_default resource group.
  if (is_default_resource_group(m_name.str)) {
    my_error(ER_DISALLOWED_OPERATION, MYF(0), "Alter",
             "default resource groups.");
    return true;
  }

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  // Acquire exclusive lock on the resource group name.
  if (acquire_exclusive_mdl_for_resource_group(thd, m_name.str)) return true;

  auto resource_group = check_and_load_resource_group(thd, m_name);

  if (resource_group == nullptr) return true;

  // VCPU IDs list validation.
  uint32_t num_vcpus = Resource_group_mgr::instance()->num_vcpus();
  DBUG_PRINT("info", ("Number of VCPUS: %u", num_vcpus));

  auto vcpu_range_vector = std::unique_ptr<std::vector<Range>>(
      new (std::nothrow) std::vector<Range>);
  if (vcpu_range_vector == nullptr) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
    return true;
  }

  if (validate_vcpu_range_vector(vcpu_range_vector.get(), m_cpu_list,
                                 num_vcpus))
    return true;

  if (validate_resource_group_priority(thd, &m_priority, m_name,
                                       resource_group->type()))
    return true;

  // FORCE option is not paired with DISABLE option.
  if (m_force && (!m_use_enable || m_enable)) {
    my_error(ER_INVALID_USE_OF_FORCE_OPTION, MYF(0));
    return true;
  }

  Thread_resource_control *thr_res_ctrl = resource_group->controller();
  bool thr_res_ctrl_change = false;
  if (m_priority != thr_res_ctrl->priority()) {
    thr_res_ctrl->set_priority(m_priority);
    thr_res_ctrl_change = true;
  }

  if (!vcpu_range_vector->empty()) {
    thr_res_ctrl->set_vcpu_vector(*vcpu_range_vector);
    thr_res_ctrl_change = true;
  }

  if (m_use_enable && m_enable != resource_group->enabled()) {
    resource_group->set_enabled(m_enable);
    thr_res_ctrl_change = m_enable;
  } else
    thr_res_ctrl_change = resource_group->enabled();

  // Update on-disk resource group.
  Disable_autocommit_guard autocommit_guard(thd);
  dd::String_type name(m_name.str);
  if (update_resource_group(thd, name, *resource_group)) return true;

  /*
    Reapply controls on threads if there was some change in
    the thread resource controls and the resource group is enabled.
  */
  if (thr_res_ctrl_change) {
    resource_group->apply_control_func(
        [resource_group](ulonglong pfs_thread_id) {
          auto res_grp_mgr_ptr = Resource_group_mgr::instance();
          PSI_thread_attrs pfs_thread_attr;
          memset(&pfs_thread_attr, 0, sizeof(pfs_thread_attr));
          if (!res_grp_mgr_ptr->get_thread_attributes(&pfs_thread_attr,
                                                      pfs_thread_id))
            resource_group->controller()->apply_control(
                pfs_thread_attr.m_thread_os_id);
        });
  }

  /*
    If some threads are bound with resource group, then
    (i) If FORCE option is specified, move the threads bound with this
        resource group to respective default resource groups.
    (ii) If FORCE option is not specified, the resource group is just
         disabled.
  */
  if (resource_group->is_bound_to_threads()) {
    if (m_force) {
      // Move all threads associated with this to default resource groups.
      resource_group->apply_control_func(
          Move_thread_to_default_group(resource_group));
      resource_group->clear();
    }
  }
  my_ok(thd);
  return false;
}

bool resourcegroups::Sql_cmd_drop_resource_group::execute(THD *thd) {
  DBUG_TRACE;

  if (check_readonly(thd, true)) return true;

  Security_context *sctx = thd->security_context();
  if (!sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "RESOURCE_GROUP_ADMIN");
    return true;
  }

  // Resource group name validation.
  if (is_invalid_string(m_name, system_charset_info)) return true;

  // Disallow dropping USR_default & SYS_default resource group.
  if (is_default_resource_group(m_name.str)) {
    my_error(ER_DISALLOWED_OPERATION, MYF(0), "Drop operation ",
             "default resource groups.");
    return true;
  }

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec) ||
      acquire_shared_backup_lock_nsec(thd,
                                      thd->variables.lock_wait_timeout_nsec))
    return true;

  // Acquire exclusive lock on the resource group name.
  if (acquire_exclusive_mdl_for_resource_group(thd, m_name.str)) return true;

  auto resource_group = check_and_load_resource_group(thd, m_name);

  if (resource_group == nullptr) return true;

  if (resource_group->is_bound_to_threads()) {
    if (m_force)  // move all threads to the default resource group.
    {
      resource_group->apply_control_func(
          Move_thread_to_default_group(resource_group));
      resource_group->clear();
    } else {
      my_error(ER_RESOURCE_GROUP_BUSY, MYF(0), m_name.str);
      return true;
    }
  }

  // Remove from on-disk resource group.
  Disable_autocommit_guard autocommit_guard(thd);
  if (dd::drop_resource_group(thd, m_name.str)) return true;

  // Remove from in-memory hash the resource group.
  if (resource_group != nullptr)
    Resource_group_mgr::instance()->remove_resource_group(m_name.str);

  my_ok(thd);
  return false;
}

/**
  Check if resource group controls can be applied to thread
  identified by PFS thread id. Apply the controls to thread
  if the checks were successful.

  @param  thd              THD context
  @param  thread_id        Thread id of the thread.
  @param  resource_group   Pointer to resource group.
  @param  error            Log error so that error is returned to client.

  @returns true if the function fails else false.
*/

static inline bool check_and_apply_resource_grp(
    THD *thd, ulonglong thread_id,
    resourcegroups::Resource_group *resource_group, bool error) {
  PSI_thread_attrs pfs_thread_attr;
  memset(&pfs_thread_attr, 0, sizeof(pfs_thread_attr));
  auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();

  if (res_grp_mgr->get_thread_attributes(&pfs_thread_attr, thread_id) ||
      thread_id != pfs_thread_attr.m_thread_internal_id) {
    if (error)
      my_error(ER_INVALID_THREAD_ID, MYF(0), thread_id);
    else
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_INVALID_THREAD_ID,
                          ER_THD(current_thd, ER_INVALID_THREAD_ID), thread_id);
    return true;
  }

  bool res_grp_match = pfs_thread_attr.m_system_thread
                           ? (resource_group->type() ==
                              resourcegroups::Type::SYSTEM_RESOURCE_GROUP)
                           : (resource_group->type() ==
                              resourcegroups::Type::USER_RESOURCE_GROUP);

  if (!res_grp_match) {
    if (error)
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), thread_id,
               "Resource group type and thread type doesn't match.");
    else
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_RESOURCE_GROUP_BIND_FAILED,
                          ER_THD(thd, ER_RESOURCE_GROUP_BIND_FAILED),
                          resource_group->name().c_str(), thread_id,
                          "Resource group type & thread type doesn't match");
    return true;
  }

  if (!res_grp_mgr->thread_priority_available() &&
      resource_group->controller()->priority() != 0)
    push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                        ER_ATTRIBUTE_IGNORED,
                        ER_THD(current_thd, ER_ATTRIBUTE_IGNORED),
                        "thread_priority", "using default value");

  // Check if resource group is already bound to this thread.
  if (resource_group->is_pfs_thread_id_exists(thread_id)) return false;

  MDL_ticket *ticket = nullptr;
  if (res_grp_mgr->acquire_shared_mdl_for_resource_group(
          thd, pfs_thread_attr.m_groupname, MDL_EXPLICIT, &ticket, true)) {
    if (error)
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), thread_id,
               "Unable to acquire  MDL lock.");
    else {
      thd->clear_error();
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_RESOURCE_GROUP_BIND_FAILED,
                          ER_THD(current_thd, ER_RESOURCE_GROUP_BIND_FAILED),
                          resource_group->name().c_str(), thread_id,
                          "Unable to acquire MDL lock.");
    }
    return true;
  }

  resourcegroups::Resource_group *prev_cur_res_grp =
      resourcegroups::Resource_group_mgr::instance()->get_resource_group(
          std::string(pfs_thread_attr.m_groupname));

  if (resource_group->controller()->apply_control(
          pfs_thread_attr.m_thread_os_id)) {
    if (error)
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), thread_id,
               "Failed to apply thread controls.");
    else
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_RESOURCE_GROUP_BIND_FAILED,
                          ER_THD(current_thd, ER_RESOURCE_GROUP_BIND_FAILED),
                          resource_group->name().c_str(), thread_id,
                          "Failed to apply thread controls.");
    res_grp_mgr->release_shared_mdl_for_resource_group(thd, ticket);
    return true;
  }

  // Set resource group context for non-system threads.
  if (!pfs_thread_attr.m_system_thread) {
    Find_thd_with_id find_thd_with_id(pfs_thread_attr.m_processlist_id);
    THD *cur_thd =
        Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
    if (cur_thd != nullptr) {
      cur_thd->resource_group_ctx()->m_cur_resource_group = resource_group;
      mysql_mutex_unlock(&cur_thd->LOCK_thd_data);
    }
  }

  if (prev_cur_res_grp != nullptr)
    prev_cur_res_grp->remove_pfs_thread_id(thread_id);
  resource_group->add_pfs_thread_id(thread_id);

  res_grp_mgr->set_res_grp_in_pfs(resource_group->name().c_str(),
                                  resource_group->name().length(), thread_id);
  res_grp_mgr->release_shared_mdl_for_resource_group(thd, ticket);
  return false;
}

/**
  Check if user has sufficient privilege to exercise SET RESOURCE GROUP.

  @param sctx Pointer to security context.

  @return true if sufficient privilege exists for SET RESOURCE GROUP else false.
*/

static bool check_resource_group_set_privilege(Security_context *sctx) {
  if (!(sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN")).first ||
        sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_USER")).first)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "RESOURCE_GROUP_ADMIN OR RESOURCE_GROUP_USER");
    return true;
  }
  return false;
}

bool resourcegroups::Sql_cmd_set_resource_group::execute(THD *thd) {
  DBUG_TRACE;

  Security_context *sctx = thd->security_context();
  if (check_resource_group_set_privilege(sctx)) return true;

  // Acquire exclusive lock on the resource group name to synchronize with hint.
  if (acquire_exclusive_mdl_for_resource_group(thd, m_name.str)) return true;

  auto resource_group = check_and_load_resource_group(thd, m_name);
  if (resource_group == nullptr) return true;

  if ((resource_group->type() == Type::SYSTEM_RESOURCE_GROUP) &&
      !sctx->has_global_grant(STRING_WITH_LEN("RESOURCE_GROUP_ADMIN")).first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "RESOURCE_GROUP_ADMIN");
    return true;
  }

  if (!resource_group->enabled()) {
    my_error(ER_RESOURCE_GROUP_DISABLED, MYF(0),
             resource_group->name().c_str());
    return true;
  }

  if (m_thread_id_list == nullptr || m_thread_id_list->empty()) {
    ulonglong pfs_thread_id = 0;
#ifdef HAVE_PSI_THREAD_INTERFACE
    pfs_thread_id = PSI_THREAD_CALL(get_current_thread_internal_id)();
#endif

    if (resource_group->type() != Type::USER_RESOURCE_GROUP) {
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), pfs_thread_id,
               "System resource group can't be applied to user thread.");
      return true;
    }

    auto res_grp_mgr = resourcegroups::Resource_group_mgr::instance();
    const char *resource_group_name = nullptr;
    mysql_mutex_lock(&thd->LOCK_thd_data);
    auto cur_res_grp = thd->resource_group_ctx()->m_cur_resource_group;
    if (cur_res_grp != nullptr)
      resource_group_name = cur_res_grp->name().c_str();
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    MDL_ticket *ticket = nullptr;
    if (resource_group_name != nullptr &&
        res_grp_mgr->acquire_shared_mdl_for_resource_group(
            thd, resource_group_name, MDL_EXPLICIT, &ticket, true)) {
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), pfs_thread_id,
               "Unable to acquire MDL lock.");
      return true;
    }

    if (resource_group->controller()->apply_control()) {
      my_error(ER_RESOURCE_GROUP_BIND_FAILED, MYF(0),
               resource_group->name().c_str(), pfs_thread_id,
               "Failed to apply thread resource controls");
      return true;
    }

    mysql_mutex_lock(&thd->LOCK_thd_data);
    cur_res_grp = thd->resource_group_ctx()->m_cur_resource_group;
    thd->resource_group_ctx()->m_cur_resource_group = resource_group;
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    if (cur_res_grp != nullptr)
      cur_res_grp->remove_pfs_thread_id(pfs_thread_id);

    resourcegroups::Resource_group_mgr::instance()->set_res_grp_in_pfs(
        resource_group->name().c_str(), resource_group->name().length(),
        pfs_thread_id);
    resource_group->add_pfs_thread_id(pfs_thread_id);
    if (ticket != nullptr)
      res_grp_mgr->release_shared_mdl_for_resource_group(thd, ticket);
  } else {
    if (m_thread_id_list->size() == 1 &&
        check_and_apply_resource_grp(thd, m_thread_id_list->at(0),
                                     resource_group, true))
      return true;
    else
      for (const auto &thread_id : *m_thread_id_list)
        (void)check_and_apply_resource_grp(thd, thread_id, resource_group,
                                           false);
  }

  my_ok(thd);
  return false;
}

bool resourcegroups::Sql_cmd_set_resource_group::prepare(THD *thd) {
  DBUG_TRACE;

  if (Sql_cmd::prepare(thd)) return true;

  bool rc = check_resource_group_set_privilege(thd->security_context());

  return rc;
}
