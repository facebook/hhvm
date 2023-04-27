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

#include <sys/processor.h>
#include <sys/procset.h>
#include <sys/types.h>
#include "sys/pset.h"

#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"

namespace resourcegroups {
namespace platform {
bool is_platform_supported() { return true; }

bool bind_to_cpu(cpu_id_t cpu_id) {
  if (processor_bind(P_LWPID, P_MYID, static_cast<processorid_t>(cpu_id),
                     nullptr) == -1) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_BIND_TO_CPUID_FAILED,
           cpu_id, my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool bind_to_cpu(cpu_id_t cpu_id, my_thread_os_id_t thread_id) {
  if (processor_bind(P_LWPID, thread_id, static_cast<processorid_t>(cpu_id),
                     nullptr) == -1) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_BIND_TO_THREAD_FAILED,
           thread_id, cpu_id, my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids) {
  if (cpu_ids.empty()) return false;

  procset_t ps;
  uint_t nids = cpu_ids.size();
  id_t *ids = reinterpret_cast<id_t *>(const_cast<unsigned *>(cpu_ids.data()));
  uint32_t flags = PA_TYPE_CPU | PA_AFF_STRONG;

  setprocset(&ps, POP_AND, P_PID, P_MYID, P_LWPID, my_thread_os_id());
  if (processor_affinity(&ps, &nids, ids, &flags) != 0) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_AFFINITY_FAILED,
           "bind_to_cpus", my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids,
                  my_thread_os_id_t thread_id) {
  procset_t ps;
  uint_t nids = cpu_ids.size();
  id_t *ids = reinterpret_cast<id_t *>(const_cast<unsigned *>(cpu_ids.data()));
  uint32_t flags = PA_TYPE_CPU | PA_AFF_STRONG;

  setprocset(&ps, POP_AND, P_PID, P_MYID, P_LWPID,
             static_cast<id_t>(thread_id));

  if (processor_affinity(&ps, &nids, ids, &flags) != 0) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_AFFINITY_FAILED,
           "bind_to_cpus", my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool unbind_thread() {
  procset_t ps;
  uint32_t flags = PA_CLEAR;

  setprocset(&ps, POP_AND, P_PID, P_MYID, P_LWPID, my_thread_os_id());

  if (processor_affinity(&ps, nullptr, nullptr, &flags) != 0) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_AFFINITY_FAILED,
           "unbind_thread", my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

bool unbind_thread(my_thread_os_id_t thread_id) {
  procset_t ps;
  uint32_t flags = PA_CLEAR;

  setprocset(&ps, POP_AND, P_PID, P_MYID, P_LWPID, thread_id);

  if (processor_affinity(&ps, nullptr, nullptr, &flags) != 0) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    LogErr(ERROR_LEVEL, ER_RES_GRP_SOLARIS_PROCESSOR_AFFINITY_FAILED,
           "unbind_thread", my_errno(),
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
    return true;
  }
  return false;
}

int thread_priority() { return getpriority(PRIO_PROCESS, my_thread_os_id()); }

int thread_priority(my_thread_os_id_t thread_id) {
  DBUG_TRACE;
  return getpriority(PRIO_PROCESS, thread_id);
}

bool set_thread_priority(int priority) {
  return set_thread_priority(priority, my_thread_os_id());
}

bool set_thread_priority(int, my_thread_os_id_t) {
  DBUG_TRACE;
  // Setting thread priority on solaris is not supported.
  return false;
}

uint32_t num_vcpus_using_affinity() {
  uint32_t num_vcpus = 0;
  pid_t pid = getpid();
  psetid_t pset = PS_NONE;

  if (pset_bind(PS_QUERY, P_PID, pid, &pset) == 0 && pset != PS_NONE) {
    pset_info(pset, nullptr, &num_vcpus, nullptr);
  }
  return num_vcpus;
}

uint32_t num_vcpus_using_config() {
  uint32_t num_vcpus = 0;

  pset_info(P_MYID, nullptr, &num_vcpus, nullptr);
  return num_vcpus;
}

bool can_thread_priority_be_set() { return false; }
}  // namespace platform
}  // namespace resourcegroups
