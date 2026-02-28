/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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
#include "thread_attrs_api.h"

#include <sys/sysctl.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "sql/log.h"

namespace resourcegroups {
namespace platform {

/*
  Mac OS doesn't have an explicit API to bind a set of processors with thread.
  Hence platform APIs are just stubs on this platform.
*/

bool is_platform_supported() { return false; }

bool bind_to_cpu(cpu_id_t) {
  DBUG_ASSERT(0);
  return true;
}

bool bind_to_cpu(cpu_id_t, my_thread_os_id_t) {
  DBUG_ASSERT(0);
  return true;
}

bool bind_to_cpus(const std::vector<cpu_id_t> &) {
  DBUG_ASSERT(0);
  return true;
}

bool bind_to_cpus(const std::vector<cpu_id_t> &, my_thread_os_id_t) {
  DBUG_ASSERT(0);
  return true;
}

bool unbind_thread() {
  DBUG_ASSERT(0);
  return true;
}

bool unbind_thread(my_thread_os_id_t) {
  DBUG_ASSERT(0);
  return true;
}

int thread_priority(my_thread_os_id_t) {
  DBUG_ASSERT(0);
  return 0;
}

bool set_thread_priority(int) {
  DBUG_ASSERT(0);
  return true;
}

bool set_thread_priority(int, my_thread_os_id_t) {
  DBUG_ASSERT(0);
  return true;
}

uint32_t num_vcpus_using_affinity() { return 0; }

uint32_t num_vcpus_using_config() {
  int name[2] = {CTL_HW, HW_AVAILCPU};
  int ncpu;

  size_t size = sizeof(ncpu);
  sysctl(name, 2, &ncpu, &size, nullptr, 0);
  return ncpu;
}

bool can_thread_priority_be_set() {
  DBUG_ASSERT(0);
  return false;
}
}  // namespace platform
}  // namespace resourcegroups
