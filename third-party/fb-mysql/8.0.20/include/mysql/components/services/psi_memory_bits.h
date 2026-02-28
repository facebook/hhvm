/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_MEMORY_BITS_H
#define COMPONENTS_SERVICES_PSI_MEMORY_BITS_H

#ifndef MYSQL_ABI_CHECK
#include <stddef.h> /* size_t */
#endif

/**
  @file
  Performance schema instrumentation interface.
*/

/**
  @defgroup psi_abi_memory Memory Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Instrumented memory key.
  To instrument memory, a memory key must be obtained using @c register_memory.
  Using a zero key always disable the instrumentation.
*/
typedef unsigned int PSI_memory_key;

struct PSI_thread;

/**
  Memory instrument information.
  @since PSI_MEMORY_VERSION_1
  This structure is used to register instrumented memory.
*/
struct PSI_memory_info_v1 {
  /** Pointer to the key assigned to the registered memory. */
  PSI_memory_key *m_key;
  /** The name of the memory instrument to register. */
  const char *m_name;
  /**
    The flags of the memory instrument to register.
    @sa PSI_FLAG_ONLY_GLOBAL_STAT
  */
  unsigned int m_flags;
  /** Volatility index. */
  int m_volatility;
  /** Documentation. */
  const char *m_documentation;
};
typedef struct PSI_memory_info_v1 PSI_memory_info_v1;

/**
  Memory registration API.
  @param category a category name (typically a plugin name)
  @param info an array of memory info to register
  @param count the size of the info array
*/
typedef void (*register_memory_v1_t)(const char *category,
                                     struct PSI_memory_info_v1 *info,
                                     int count);

/**
  Instrument memory allocation.
  @param key the memory instrument key
  @param size the size of memory allocated
  @param[out] owner the memory owner
  @return the effective memory instrument key
*/
typedef PSI_memory_key (*memory_alloc_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);

/**
  Instrument memory re allocation.
  @param key the memory instrument key
  @param old_size the size of memory previously allocated
  @param new_size the size of memory re allocated
  @param[in, out] owner the memory owner
  @return the effective memory instrument key
*/
typedef PSI_memory_key (*memory_realloc_v1_t)(PSI_memory_key key,
                                              size_t old_size, size_t new_size,
                                              struct PSI_thread **owner);

/**
  Instrument memory claim.
  @param key the memory instrument key
  @param size the size of memory allocated
  @param[in, out] owner the memory owner
  @return the effective memory instrument key
*/
typedef PSI_memory_key (*memory_claim_v1_t)(PSI_memory_key key, size_t size,
                                            struct PSI_thread **owner);

/**
  Instrument memory free.
  @param key the memory instrument key
  @param size the size of memory allocated
  @param owner the memory owner
*/
typedef void (*memory_free_v1_t)(PSI_memory_key key, size_t size,
                                 struct PSI_thread *owner);

typedef struct PSI_memory_info_v1 PSI_memory_info;

/** @} (end of group psi_abi_memory) */

#endif /* COMPONENTS_SERVICES_PSI_MEMORY_BITS_H */
