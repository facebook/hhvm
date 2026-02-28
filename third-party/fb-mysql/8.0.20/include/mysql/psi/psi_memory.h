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

#ifndef MYSQL_PSI_MEMORY_H
#define MYSQL_PSI_MEMORY_H

#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_memory_bits.h"

/*
  MAINTAINER:
  Note that this file is part of the public API,
  because mysql.h exports
    struct MEM_ROOT
  See
    - PSI_memory_key MEM_ROOT::m_psi_key
    - include/mysql.h.pp
*/

/**
  @file include/mysql/psi/psi_memory.h
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

/**
  @def PSI_MEMORY_VERSION_1
  Performance Schema Memory Interface number for version 1.
  This version is supported.
*/
#define PSI_MEMORY_VERSION_1 1

/**
  @def PSI_CURRENT_MEMORY_VERSION
  Performance Schema Memory Interface number for the most recent version.
  The most current version is @c PSI_MEMORY_VERSION_1
*/
#define PSI_CURRENT_MEMORY_VERSION 1

struct PSI_thread;

/** Entry point for the performance schema interface. */
struct PSI_memory_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_MEMORY_VERSION_1
    @sa PSI_MEMORY_VERSION_2
    @sa PSI_CURRENT_MEMORY_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_memory_bootstrap PSI_memory_bootstrap;

#ifdef HAVE_PSI_MEMORY_INTERFACE

/**
  Performance Schema Memory Interface, version 1.
  @since PSI_MEMORY_VERSION_1
*/
struct PSI_memory_service_v1 {
  /** @sa register_memory_v1_t. */
  register_memory_v1_t register_memory;
  /** @sa memory_alloc_v1_t. */
  memory_alloc_v1_t memory_alloc;
  /** @sa memory_realloc_v1_t. */
  memory_realloc_v1_t memory_realloc;
  /** @sa memory_claim_v1_t. */
  memory_claim_v1_t memory_claim;
  /** @sa memory_free_v1_t. */
  memory_free_v1_t memory_free;
};

typedef struct PSI_memory_service_v1 PSI_memory_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_memory_service_t *psi_memory_service;

#endif /* HAVE_PSI_MEMORY_INTERFACE */

/** @} (end of group psi_abi_memory) */

#endif /* MYSQL_PSI_MEMORY_H */
