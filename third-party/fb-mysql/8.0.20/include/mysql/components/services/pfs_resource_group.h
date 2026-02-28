/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_RESOURCE_GROUP_H
#define PFS_RESOURCE_GROUP_H

#include <mysql/components/service.h>
#include <mysql/psi/mysql_thread.h>

/**
  @page PAGE_PFS_RESOURCE_GROUP_SERVICE Resource group service

  @section PFS_RESOURCE_GROUP_INTRO Introduction
  The Performance Schema Resource Group service provides methods to:
  - assign a resource group name to a foreground (user) and background (system)
  threads
  - query the system attributes of a given thread, such as thread id, user name,
  host name, etc.

  Once assigned, the resource group name is visible in the
  PERFORMANCE_SCHEMA.THREADS table.

  @section PFS_RESOURCE_GROUP_SET Setting a group name

  A group name can be assigned to the current thread or to another thread
  identified by either a thread id or a pointer to thread instrumentation.

  User-defined data can also be assigned to the thread.

  To assign a group name to the current thread, use:

  @code
    int set_thread_resource_group(const char* group_name,
                                  int group_name_len,
                                  void *user_data)
  @endcode

  where
  - @c group_name is the resource group name string
  - @c group_name_len is the length of resource group name string
  - @c user_data is an optional user-defined context

  To assign a group name and user data to another thread, use:

  @code
    int set_thread_resource_group_by_id(PSI_thread *psi_thread,
                                        unsigned long long thread_id,
                                        const char* group_name,
                                        int group_name_len,
                                        void *user_data)
  @endcode

  where
  - @c psi_thread is the target thread instrumentation. Ignored if NULL.
  - @c thread_id is the thread id of the target thread (THREADS.THREAD_ID). Only
  used if thread is NULL.
  - @c group_name is the resource group name string
  - @c group_name_len is the length of resource group name string
  - @c user_data is the optional user-defined context

  Both functions return 0 if successful, or 1 otherwise.

  The group name is limited to 64 characters, UTF8. Names longer than 64
  characters will be truncated. user_data is an optional user-defined context
  associated with thread_id that will be returned to the callback function in
  the thread attributes structure.

  @section PFS_RESOURCE_GROUP_GET Getting thread attributes

  To get the system and security attributes for the current thread, use:

  @code
    int get_thread_system_attrs(PSI_thread_attrs *thread_attrs)
  @endcode

  where
  - @c thread_attrs is a pointer to a thread attribute structure
  #PSI_thread_attrs

  To get the system and security attributes for another thread identified either
  by a thread id or by the thread instrumentation, use:

  @code
    int get_thread_system_attrs_by_id(PSI_thread *psi_thread,
                                      unsigned long long thread_id,
                                      PSI_thread_attrs *thread_attrs)
  @endcode

  where
  @c psi_thread is the target thread instrumentation. Ignored if NULL.
  @c thread_id is the thread id of the target thread (THREADS.THREAD_ID). Only
  used if psi_thread is NULL.
  @c thread_attrs is a pointer to thread attribute structure, #PSI_thread_attrs

  Both function return 0 if successful or 1 otherwise.

*/

/*
  SERVICE_DEFINITION(pfs_resource_group)
  Introduced in MySQL 8.0.2
  Removed in MySQL 8.0.17
  Status: Removed, use version 3 instead.
*/

/*
  Version 3.
  Introduced in MySQL 8.0.17
  Status: active
*/

BEGIN_SERVICE_DEFINITION(pfs_resource_group_v3)
set_thread_resource_group_v1_t set_thread_resource_group;
set_thread_resource_group_by_id_v1_t set_thread_resource_group_by_id;
get_thread_system_attrs_v3_t get_thread_system_attrs;
get_thread_system_attrs_by_id_v3_t get_thread_system_attrs_by_id;
END_SERVICE_DEFINITION(pfs_resource_group_v3)

#define REQUIRES_PFS_RESOURCE_GROUP_SERVICE \
  REQUIRES_SERVICE(pfs_resource_group_v3)

#endif
