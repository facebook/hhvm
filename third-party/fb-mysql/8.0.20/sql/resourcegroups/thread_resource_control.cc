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

#include "thread_resource_control.h"

#include <stdint.h>

#include "my_dbug.h"  // DBUG_*
#include "sql/log.h"
#include "sql/resourcegroups/platform/thread_attrs_api.h"
#include "sql/resourcegroups/resource_group_mgr.h"  // num_vcpus

namespace resourcegroups {
bool Thread_resource_control::validate(const Type &type) const {
  DBUG_TRACE;

  auto mgr_ptr = Resource_group_mgr::instance();
  bool result = false;

  if (mgr_ptr->thread_priority_available()) {
    int min = 0;
    int max = 0;

    if (type == Type::USER_RESOURCE_GROUP)
      max = platform::max_thread_priority_value();
    else
      min = platform::min_thread_priority_value();

    if (m_priority < min || m_priority > max) {
      LogErr(WARNING_LEVEL, ER_RES_GRP_INVALID_THREAD_PRIORITY, m_priority,
             mgr_ptr->resource_group_type_str(type), min, max);
      result = true;
    }
  }

  uint32_t num_vcpus = Resource_group_mgr::instance()->num_vcpus();
  uint32_t vcpu_id;
  for (const auto &vcpu_range : m_vcpu_vector) {
    if (vcpu_range.m_start > vcpu_range.m_end) {
      LogErr(WARNING_LEVEL, ER_RES_GRP_INVALID_VCPU_RANGE, vcpu_range.m_start,
             vcpu_range.m_end);
      result = true;
    }

    if ((vcpu_id = vcpu_range.m_start) >= num_vcpus ||
        (vcpu_id = vcpu_range.m_end) >= num_vcpus) {
      LogErr(WARNING_LEVEL, ER_RES_GRP_INVALID_VCPU_ID, vcpu_id);
      result = true;
    }
  }

  return result;
}

bool Thread_resource_control::apply_control() {
  DBUG_TRACE;

  bool ret = false;
  std::vector<resourcegroups::platform::cpu_id_t> cpu_ids;
  if (!m_vcpu_vector.empty()) {
    for (const auto &cpu_range : m_vcpu_vector) {
      for (auto id = cpu_range.m_start; id <= cpu_range.m_end; ++id)
        cpu_ids.push_back(id);
    }
    ret = resourcegroups::platform::bind_to_cpus(cpu_ids) ||
          resourcegroups::platform::set_thread_priority(m_priority);
  } else {
    ret = resourcegroups::platform::unbind_thread() ||
          resourcegroups::platform::set_thread_priority(m_priority);
  }
  return ret;
}

bool Thread_resource_control::apply_control(my_thread_os_id_t thread_os_id) {
  DBUG_TRACE;

  bool ret = false;
  std::vector<resourcegroups::platform::cpu_id_t> cpu_ids;
  if (!m_vcpu_vector.empty()) {
    for (auto cpu_range : m_vcpu_vector) {
      for (auto id = cpu_range.m_start; id <= cpu_range.m_end; ++id)
        cpu_ids.push_back(id);
    }
    ret =
        resourcegroups::platform::bind_to_cpus(cpu_ids, thread_os_id) ||
        resourcegroups::platform::set_thread_priority(m_priority, thread_os_id);
  } else {
    ret =
        resourcegroups::platform::unbind_thread(thread_os_id) ||
        resourcegroups::platform::set_thread_priority(m_priority, thread_os_id);
  }
  return ret;
}
}  // namespace resourcegroups
