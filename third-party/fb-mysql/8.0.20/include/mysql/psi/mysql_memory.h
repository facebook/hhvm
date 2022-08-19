/* Copyright (c) 2012, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_MEMORY_H
#define MYSQL_MEMORY_H

/**
  @file include/mysql/psi/mysql_memory.h
  Instrumentation helpers for memory allocation.
*/

#include "my_compiler.h"
#include "mysql/psi/psi_memory.h"

#ifndef PSI_MEMORY_CALL
#define PSI_MEMORY_CALL(M) psi_memory_service->M
#endif

/**
  @defgroup psi_api_memory Memory Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_memory_register(P1, P2, P3)
  Memory registration.
*/
#define mysql_memory_register(P1, P2, P3) \
  inline_mysql_memory_register(P1, P2, P3)

static inline void inline_mysql_memory_register(
#ifdef HAVE_PSI_MEMORY_INTERFACE
    const char *category, PSI_memory_info *info, int count)
#else
    const char *category MY_ATTRIBUTE((unused)),
    void *info MY_ATTRIBUTE((unused)), int count MY_ATTRIBUTE((unused)))
#endif
{
#ifdef HAVE_PSI_MEMORY_INTERFACE
  PSI_MEMORY_CALL(register_memory)(category, info, count);
#endif
}

/** @} (end of group psi_api_memory) */

#endif
