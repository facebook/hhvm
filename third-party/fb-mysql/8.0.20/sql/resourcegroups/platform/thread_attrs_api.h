/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RESOURCEGROUPS_PLATFORM_THREAD_ATTRS_API_H_
#define RESOURCEGROUPS_PLATFORM_THREAD_ATTRS_API_H_

#include <cinttypes>  // uint32_t
#include <vector>     // std::vector

#include "my_thread_os_id.h"  // my_thread_os_id

namespace resourcegroups {
namespace platform {
using cpu_id_t = uint32_t;

/**
  Check if platform supports binding CPUS to thread.

  @returns true if platform supports CPU binding else false.
*/

bool is_platform_supported();

/**
  Bind current thread to run on CPU cpu_id.

  @param   cpu_id   ID of the CPU.

  @returns false if binding was successful else true.
*/

bool bind_to_cpu(cpu_id_t cpu_id);

/**
  Bind thread specified by thread_id to run on CPU cpu_id.

  @param   cpu_id    ID of CPU.
  @param   thread_id OS ID of the thread.

  @returns true if binding was unsuccessful else false.
*/

bool bind_to_cpu(cpu_id_t cpu_id, my_thread_os_id_t thread_id);

/**
  Bind current thread to run on the list of CPUS specified in cpu_ids.

  @param  cpu_ids List of CPU IDs to bind current thread.

  @returns false if binding was successful else true.
*/

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids);

/**
  Bind thread specified by thread_id to run on list of CPUs specified by
  cpu_ids.

  @param  cpu_ids   List of CPU IDs to bind the thread.
  @param  thread_id  OS ID of thread.

  @returns false if binding was successful else true.
*/

bool bind_to_cpus(const std::vector<cpu_id_t> &cpu_ids,
                  my_thread_os_id_t thread_id);

/**
  Unbind current thread to run on all CPUs.

  @returns true if unbind was unsuccessful else false.
*/

bool unbind_thread();

/**
  Unbind thread specified by thread_id to run on all CPUs.

  @returns true if unbind was unsuccessful else false.
*/

bool unbind_thread(my_thread_os_id_t thread_id);

/**
  Get priority of thread specified by thread_id.

  @param  thread_id OS ID of the thread.

  @returns an int indicating priority of the thread.
*/

int thread_priority(my_thread_os_id_t thread_id);

/**
  Set priority of current thread.

  @param  priority Priority to set to.

  @returns true if call was unsuccessful else false.
*/

bool set_thread_priority(int priority);

/**
  Set priority of thread specified by thread_id.

  @param   priority    Priority to set to.
  @param   thread_id   OS ID of the thread.

  @returns  true if call was unsuccessful else false.
*/

bool set_thread_priority(int priority, my_thread_os_id_t thread_id);

/**
  Find number of VCPUs as seen by the current process based on the
  affinity between each process and VCPU.
*/
uint32_t num_vcpus_using_affinity();

/**
  Get the number of VCPUS based on system configuration.
*/

uint32_t num_vcpus_using_config();

/**
  Get the number of VCPU.
*/

uint32_t num_vcpus();

/**
  Check if thread priority setting is allowed on the platform or not.

  @returns true if thread priority setting is allowed else false.
*/

bool can_thread_priority_be_set();

/**
  Check if thread priority value is valid.

  @returns true if thread priority value is valid else false.
*/

bool is_valid_thread_priority(int priority);

/**
  Get the minimum priority value

  @returns int indicating minimum priority value
*/

int min_thread_priority_value();

/**
  Get the maximum priority value

  @returns int indicating max priority value.
*/

int max_thread_priority_value();
}  // namespace platform
}  // namespace resourcegroups
#endif  // RESOURCEGROUPS_PLATFORM_THREAD_ATTRS_API_H_
