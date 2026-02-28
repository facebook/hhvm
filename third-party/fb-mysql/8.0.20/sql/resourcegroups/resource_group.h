/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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
#ifndef RESOURCEGROUPS_RESOURCE_GROUP_H_
#define RESOURCEGROUPS_RESOURCE_GROUP_H_

#include "resource_group_sql_cmd.h"

#include <functional>
#include <memory>
#include <mutex>
#include <set>

#include "sql/log.h"  // sql_print_warning
#include "sql/resourcegroups/thread_resource_control.h"

namespace resourcegroups {

/**
  Class that represents an abstraction of the Resource Group.
  It has generic attributes of Resource group name, type,
  active or inactive, a pointer to a Resource control object.
*/

class Resource_group {
 public:
  /**
    Construct a Resource_group object.

    @param name name of the Resource_group.
    @param type type of the Resource_group.
    @param enabled Check if Resource_group is enabled or not.
  */

  Resource_group(const std::string &name, const Type type, bool enabled)
      : m_name(name), m_type(type), m_enabled(enabled) {}

  const std::string &name() const { return m_name; }

  Type type() const { return m_type; }

  bool enabled() const { return m_enabled; }

  void set_type(Type type) { m_type = type; }

  void set_enabled(bool enabled) { m_enabled = enabled; }

  Thread_resource_control *controller() { return &m_thread_resource_control; }

  const Thread_resource_control *controller() const {
    return &m_thread_resource_control;
  }

  /**
    Check if resource group is associated with threads.

    @return true if some threads are mapped with this resource group
            else false.
  */

  bool is_bound_to_threads() {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    return !m_pfs_thread_id_set.empty();
  }

  /**
    Is pfs thread id already exists in the set.

    @param pfs_thread_id  PFS thread id.

    @return true if thread id exists in the set else false.
  */

  bool is_pfs_thread_id_exists(const ulonglong pfs_thread_id) {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    return m_pfs_thread_id_set.find(pfs_thread_id) != m_pfs_thread_id_set.end();
  }

  /**
    Add thread_id to the thread id set associated with this resource group.

    @param pfs_thread_id  PFS thread id.
  */

  void add_pfs_thread_id(const ulonglong pfs_thread_id) {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    (void)m_pfs_thread_id_set.insert(pfs_thread_id);
  }

  /**
    Remove the PFS thread id.

    @param pfs_thread_id Remove pfs thread id.
  */

  void remove_pfs_thread_id(const ulonglong pfs_thread_id) {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    (void)m_pfs_thread_id_set.erase(pfs_thread_id);
  }

  /**
    Clear the thread id set associated with this resource group.
  */

  void clear() {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    (void)m_pfs_thread_id_set.clear();
  }

  /**
    Apply a control function on threads associated with this resource group.

    @param control_func pointer to Control function.
  */

  void apply_control_func(std::function<void(ulonglong)> control_func) {
    std::unique_lock<std::mutex> lock(m_set_mutex);
    for (auto pfs_thread_id : m_pfs_thread_id_set) control_func(pfs_thread_id);
  }

  ~Resource_group() {}

 private:
  /**
    Name of the resource group.
  */
  std::string m_name;

  /**
    Type whether it is user or system resource group.
  */
  Type m_type;

  /**
    bool flag whether resource is enabled or disabled.
  */
  bool m_enabled;

  /**
    Thread resource controller object.
  */
  Thread_resource_control m_thread_resource_control;

  /**
    Threads associated with this resource group.
  */
  std::set<ulonglong> m_pfs_thread_id_set;

  /**
    Mutex protecting the resource group set.
  */
  std::mutex m_set_mutex;

  /**
    Disable copy construction and assignment.
  */
  Resource_group(const Resource_group &) = delete;
  void operator=(const Resource_group &) = delete;
};
}  // namespace resourcegroups
#endif  // RESOURCEGROUPS_RESOURCE_GROUP_H_
