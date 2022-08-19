/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <errno.h>
#include <sys/cpuset.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>

#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

namespace resourcegroups {
namespace platform {

bool is_platform_supported() { return true; }

bool bind_to_cpu(cpu_id_t cpu_id) {
  return bind_to_cpu(cpu_id, my_thread_os_id());
}

bool bind_to_cpu(cpu_id_t cpu_id, my_thread_os_id_t thread_id) {
  DBUG_TRACE;

  cpuset_t cpu_set;

  CPU_ZERO(&cpu_set);
  CPU_SET(cpu_id, &cpu_set);
  if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, thread_id,
                         sizeof(cpu_set), &cpu_set) == -1) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SET_THR_AFFINITY_FAILED, thread_id, cpu_id,
           my_errno(), my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids) {
  return bind_to_cpus(cpu_ids, my_thread_os_id());
}

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids,
                  my_thread_os_id_t thread_id) {
  DBUG_TRACE;

  if (cpu_ids.empty()) return false;

  cpuset_t cpu_set;

  CPU_ZERO(&cpu_set);
  for (const auto &cpu_id : cpu_ids) CPU_SET(cpu_id, &cpu_set);

  if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, thread_id,
                         sizeof(cpu_set), &cpu_set) == -1) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SET_THR_AFFINITY_FAILED, thread_id,
           my_errno(), my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool unbind_thread() { return unbind_thread(my_thread_os_id()); }

bool unbind_thread(my_thread_os_id_t thread_id) {
  DBUG_TRACE;

  cpuset_t cpu_set;

  CPU_ZERO(&cpu_set);
  uint32_t num_cpus = num_vcpus();
  if (num_cpus == 0) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_THD_UNBIND_FROM_CPU_FAILED, thread_id,
           my_errno(), my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  for (cpu_id_t cpu_id = 0; cpu_id < num_cpus; ++cpu_id)
    CPU_SET(cpu_id, &cpu_set);
  if (cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, thread_id,
                         sizeof(cpu_set), &cpu_set) == -1) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_THD_UNBIND_FROM_CPU_FAILED, thread_id,
           my_errno(), my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }

  return false;
}

int thread_priority() { return getpriority(PRIO_PROCESS, my_thread_os_id()); }

int thread_priority(my_thread_os_id_t) {
  DBUG_ASSERT(0);
  LogErr(WARNING_LEVEL, ER_RES_GRP_GET_THREAD_PRIO_NOT_SUPPORTED, "FreeBSD");
  return 0;
}

bool set_thread_priority(int priority) {
  return set_thread_priority(priority, my_thread_os_id());
}

bool set_thread_priority(int, my_thread_os_id_t) {
  DBUG_TRACE;
  // Thread priority setting unsupported in FreeBSD.
  return false;
}

uint32_t num_vcpus_using_affinity() {
  uint32_t num_vcpus = 0;

#ifdef HAVE_PTHREAD_GETAFFINITY_NP
  cpuset_t set;

  if (pthread_getaffinity_np(pthread_self(), sizeof(set), &set) == 0)
    num_vcpus = CPU_COUNT(&set);
#endif  // HAVE_PTHREAD_GETAFFINITY_NP
  return num_vcpus;
}

uint32_t num_vcpus_using_config() {
  cpu_id_t num_vcpus = 0;
  size_t num_vcpus_size = sizeof(cpu_id_t);
  if (sysctlbyname("hw.ncpu", &num_vcpus, &num_vcpus_size, nullptr, 0) != 0) {
    LogErr(ERROR_LEVEL, ER_RES_GRP_FAILED_DETERMINE_CPU_COUNT);
    num_vcpus = 0;
  }
  return num_vcpus;
}

bool can_thread_priority_be_set() { return false; }
}  // namespace platform
}  // namespace resourcegroups
