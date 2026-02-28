/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_MEMORY_PROVIDER_H
#define PFS_MEMORY_PROVIDER_H

#include "my_psi_config.h"

/**
  @file include/pfs_memory_provider.h
  Performance schema instrumentation (declarations).
*/

#ifdef HAVE_PSI_MEMORY_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include <stddef.h>

#include "my_macros.h"
#include "mysql/psi/psi_memory.h"

struct PSI_thread;

#define PSI_MEMORY_CALL(M) pfs_##M##_v1

void pfs_register_memory_v1(const char *category,
                            struct PSI_memory_info_v1 *info, int count);

PSI_memory_key pfs_memory_alloc_v1(PSI_memory_key key, size_t size,
                                   PSI_thread **owner);

PSI_memory_key pfs_memory_realloc_v1(PSI_memory_key key, size_t old_size,
                                     size_t new_size, PSI_thread **owner);

void pfs_memory_free_v1(PSI_memory_key key, size_t size, PSI_thread *owner);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_MEMORY_INTERFACE */

#endif
