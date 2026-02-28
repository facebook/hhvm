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

#ifndef RESOURCEGROUPS_RESOURCE_GROUP_MGR_H_
#define RESOURCEGROUPS_RESOURCE_GROUP_MGR_H_

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/components/service.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/pfs_notification.h"
#include "mysql/components/services/pfs_resource_group.h"
#include "mysql/components/services/psi_thread_bits.h"
#include "mysql/components/services/registry.h"
#include "mysql/psi/mysql_mutex.h"
#include "sql/debug_sync.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/sql_class.h"

namespace dd {
class Resource_group;
}  // namespace dd
namespace resourcegroups {
class Resource_group;
}  // namespace resourcegroups
template <class Key, class Value>
class collation_unordered_map;

namespace resourcegroups {

/**
  This is a singleton class that provides various functionalities related to
  Resource group management, more importantly the managing and the mapping of
  resource group names to the corresponding in-memory resource group object.
*/

class Resource_group_mgr {
 public:
  /**
    Singleton method to return an instance of this class.
  */

  static Resource_group_mgr *instance();

  /**
    Destroy the singleton instance.
  */

  static void destroy_instance();

  /**
    Check if support for Resource group exists at runtime.

    @return true if resource group feature is supported else false.
  */

  bool resource_group_support() { return m_resource_group_support; }

  /**
    Reason for resource group not being supported.

    @return pointer to string which indicate reason for resource group unsupport
  */

  const char *unsupport_reason() { return m_unsupport_reason.c_str(); }

  /**
    Set reason for resource group not being supported.

    @param  reason string representing reason for resource group unsupport.
  */

  void set_unsupport_reason(const std::string &reason) {
    m_unsupport_reason = reason;
  }

  /**
    Initialize the Resource group manager.
    Must be called before instance() can be used.

    @return true if initialization failed, false otherwise.
  */

  bool init();

  /**
    Post initialization sequence during mysqld startup.

    @return true if post initialization failed, else false.
  */

  bool post_init();

  /**
    Disable and deinitialize the resource group if it was initialized before.
  */

  void disable_resource_group() {
    if (m_resource_group_support) {
      LogErr(INFORMATION_LEVEL, ER_RES_GRP_FEATURE_NOT_AVAILABLE);
      deinit();
      m_resource_group_support = false;
    }
  }

  /**
    Get the in-memory Resource group object corresponding
    to a resource group name.

    @param  resource_group_name Name of the resource group.
    @return Pointer to the Resource group if it exists else nullptr.
  */

  Resource_group *get_resource_group(const std::string &resource_group_name);

  /**
    Get the thread attributes identified by PSI thread ID.

    @param [out] pfs_thread_attr Pointer to thread attribute object which
                                 shall be populated if the call is successful.
    @param thread_id PFS thread identifier representing a thread of mysqld.

    @return true if call failed else false.
  */

  bool get_thread_attributes(PSI_thread_attrs *pfs_thread_attr,
                             ulonglong thread_id) {
    DBUG_ASSERT(m_resource_group_support);
    return m_resource_group_svc->get_thread_system_attrs_by_id(
               nullptr, thread_id, pfs_thread_attr) != 0;
  }

  /**
    Add the resource group to the Resource group map.

    @param  resource_group_ptr  pointer to in-memory Resource_group.
    @return true if the call failed else false.
  */

  bool add_resource_group(std::unique_ptr<Resource_group> resource_group_ptr);

  /**
    Remove the resource group from the map identified by it's name.

    @param name of the resource group.
  */

  void remove_resource_group(const std::string &name);

  /**
    Create an in-memory resource group identified by its attributes
    and add it to the resource group map.

    @param name     Name of resource group.
    @param type     Type of resource group.
    @param enabled  Is resource group enabled?
    @param cpus     Pointer to list of Range objects representing CPU IDS.
    @param thr_priority Thread priority.

    @returns Pointer to Resource Group object if call is successful else null.
  */

  Resource_group *create_and_add_in_resource_group_hash(
      const LEX_CSTRING &name, Type type, bool enabled,
      std::unique_ptr<std::vector<Range>> cpus, int thr_priority);

  /**
    Move the Resource group of the current thread identified by from_res_group
    to the Resource group to_res_grp.

    @param from_res_grp  Pointer to current Resource_group of the thread.
    @param to_res_grp    Pointer to the destination Resource group which the
                          thread will be switched to.
    @return true if move_resource_group failed else false.
  */

  bool move_resource_group(Resource_group *from_res_grp,
                           Resource_group *to_res_grp);

  /**
    Deserialize a DD resource group object into an
    in-memory Resource group object.

    @param res_grp  Pointer to DD Resource_group object.
    @return Pointer to in-memory Resource_group subject if
             Successful else null.
  */

  Resource_group *deserialize_resource_group(const dd::Resource_group *res_grp);

  /**
    Set Resource group name in the PFS table performance_schema.threads for
    the PFS thread id.

    @param name Pointer to name of resource group.
    @param length length of the resource group name.
    @param thread_id PFS thread id of the thread,
  */

  void set_res_grp_in_pfs(const char *name, int length, ulonglong thread_id) {
    DBUG_ASSERT(m_resource_group_support);
    m_resource_group_svc->set_thread_resource_group_by_id(
        nullptr, thread_id, name, length, nullptr);
  }

  /**
    Return the SYS_default resource group instance.

    @return pointer to the SYS_default resource group.
  */

  Resource_group *sys_default_resource_group() {
    return m_sys_default_resource_group;
  }

  /**
    Return the USR_default resource group instance.

    @return pointer to the USR_default resource group.
  */

  Resource_group *usr_default_resource_group() {
    return m_usr_default_resource_group;
  }

  /**
    Check if a given Resource group is either SYS_default or USR_default.

    @return true if resource is USR_default or SYS_default else false.
  */

  bool is_resource_group_default(const Resource_group *res_grp) {
    return (res_grp == m_usr_default_resource_group ||
            res_grp == m_sys_default_resource_group);
  }

  /**
    Check if a thread priority setting can be done.

    @return true if thread priority setting could be done else false.
  */

  bool thread_priority_available() {
    DBUG_ASSERT(m_resource_group_support);
    return m_thread_priority_available;
  }

  /**
    Acquire an shared MDL lock on resource group name.

    @param        thd                Pointer to THD context.
    @param        res_grp_name       Resource group name.
    @param        lock_duration      Duration of lock.
    @param[out]   ticket             reference to ticket object.
    @param        acquire_lock       true if one needs to wait on lock
                                     else false.

    @return true if lock acquisition failed else false.
  */

  bool acquire_shared_mdl_for_resource_group(THD *thd, const char *res_grp_name,
                                             enum_mdl_duration lock_duration,
                                             MDL_ticket **ticket,
                                             bool acquire_lock);

  /**
    Release the shared MDL lock held on a resource group.

    @param thd        THD context.
    @param ticket     Pointer to lock ticket object.
  */

  void release_shared_mdl_for_resource_group(THD *thd, MDL_ticket *ticket) {
    DBUG_ASSERT(ticket != nullptr);
    thd->mdl_context.release_lock(ticket);
  }

  /**
    String corressponding to the type of resource group.

    @param type Type of resource group.

    @return string corressponding to resource group type.
  */

  const char *resource_group_type_str(const Type &type) {
    return type == Type::USER_RESOURCE_GROUP ? "User" : "System";
  }

  /**
    Number of VCPUs present in the system.

    @returns Number of VCPUs in the system.
  */

  uint32_t num_vcpus() { return m_num_vcpus; }

  void deinit();

#ifndef DBUG_OFF  // The belows methods are required in debug build for testing.
  bool disable_pfs_notification();
#endif

  /**
    Switch Resource Group if it is requested by environment.

    @param      thd            THD context.
    @param[out] src_res_grp    Original resource group from that
    switching is performed
    @param[out] dest_res_grp   Destination resource group to that
    switching is performed
    @param[out] ticket         Pointer to MDL ticket object.
    @param[out] cur_ticket     Pointer to current resource group MDL ticket
                               object.

    @return true if switching was performed, else false
    */

  bool switch_resource_group_if_needed(
      THD *thd, resourcegroups::Resource_group **src_res_grp,
      resourcegroups::Resource_group **dest_res_grp, MDL_ticket **ticket,
      MDL_ticket **cur_ticket);

  /**
  Restore original resource group if
  resource group switching was performed before.

  @param      thd            Thread context.
  @param[out] src_res_grp    resource group to that
                             switching was performed
  @param[out] dest_res_grp   original resource group to that
                             switching is performed
*/

  void restore_original_resource_group(
      THD *thd, resourcegroups::Resource_group *src_res_grp,
      resourcegroups::Resource_group *dest_res_grp) {
    mysql_mutex_lock(&thd->LOCK_thd_data);
    if (thd->resource_group_ctx()->m_cur_resource_group == nullptr ||
        thd->resource_group_ctx()->m_cur_resource_group == src_res_grp) {
      resourcegroups::Resource_group_mgr::instance()->move_resource_group(
          dest_res_grp, thd->resource_group_ctx()->m_cur_resource_group);
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
    DBUG_EXECUTE_IF("pause_after_rg_switch", {
      const char act[] =
          "now "
          "SIGNAL restore_finished";
      DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(act)));
    };);
  }

 private:
  /**
    Pointer to singleton instance of the Resource_group_mgr class.
  */
  static Resource_group_mgr *m_instance;

  /**
    Handles to the PFS registry, Resource Group and
    Notification services.
  */

  SERVICE_TYPE(registry) * m_registry_svc;
  SERVICE_TYPE(pfs_resource_group_v3) * m_resource_group_svc;
  SERVICE_TYPE(pfs_notification_v3) * m_notify_svc;
  my_h_service m_h_res_grp_svc;
  my_h_service m_h_notification_svc;
  int m_notify_handle;

  /**
    Pointer to USR_default and SYS_default resource groups.
    Owernship of these pointers is resource group hash.
  */
  Resource_group *m_usr_default_resource_group;
  Resource_group *m_sys_default_resource_group;

  /**
    Map mapping resource group name with it's corresponding in-memory
    Resource_group object
  */
  collation_unordered_map<std::string, std::unique_ptr<Resource_group>>
      *m_resource_group_hash;

  /**
    Lock protecting the resource group map.
  */
  mysql_rwlock_t m_map_rwlock;

  /**
    Boolean value indicating whether setting of thread priority is allowed.
  */
  bool m_thread_priority_available;

  /**
    Bool value indicating support for resource group.
  */
  bool m_resource_group_support;

  /**
    String indicating reason for resource group not being supported.
  */
  std::string m_unsupport_reason;

  /**
    Value indicating the number of vcpus in the system. This is initialized
    during startup so that we do not call the platform API everytime
    which is expensive.
  */
  uint32_t m_num_vcpus;

  Resource_group_mgr()
      : m_registry_svc(nullptr),
        m_resource_group_svc(nullptr),
        m_notify_svc(nullptr),
        m_h_res_grp_svc(nullptr),
        m_h_notification_svc(nullptr),
        m_notify_handle(0),
        m_usr_default_resource_group(nullptr),
        m_sys_default_resource_group(nullptr),
        m_resource_group_hash(nullptr),
        m_thread_priority_available(false),
        m_resource_group_support(false),
        m_num_vcpus(0) {}

  ~Resource_group_mgr() {}

  // Disable copy construction and assignment for Resource_group_mgr class.
  Resource_group_mgr(const Resource_group_mgr &) = delete;
  void operator=(const Resource_group_mgr &) = delete;
};
}  // namespace resourcegroups
#endif  // RESOURCEGROUPS_RESOURCE_GROUP_MGR_H_
