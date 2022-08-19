/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PSI_MUTEX_H
#define MYSQL_PSI_MUTEX_H

/**
  @file include/mysql/psi/psi_mutex.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_mutex Mutex Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_mutex_bits.h"

/** Entry point for the performance schema interface. */
struct PSI_mutex_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_MUTEX_VERSION_1
    @sa PSI_MUTEX_VERSION_2
    @sa PSI_CURRENT_MUTEX_VERSION
  */
  void *(*get_interface)(int version);
};

#ifdef HAVE_PSI_MUTEX_INTERFACE

/**
  Performance Schema Mutex Interface, version 1.
  @since PSI_MUTEX_VERSION_1
*/
struct PSI_mutex_service_v1 {
  /** @sa register_mutex_v1_t. */
  register_mutex_v1_t register_mutex;
  /** @sa init_mutex_v1_t. */
  init_mutex_v1_t init_mutex;
  /** @sa destroy_mutex_v1_t. */
  destroy_mutex_v1_t destroy_mutex;
  /** @sa start_mutex_wait_v1_t. */
  start_mutex_wait_v1_t start_mutex_wait;
  /** @sa end_mutex_wait_v1_t. */
  end_mutex_wait_v1_t end_mutex_wait;
  /** @sa unlock_mutex_v1_t. */
  unlock_mutex_v1_t unlock_mutex;
};

typedef struct PSI_mutex_service_v1 PSI_mutex_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_mutex_service_t *psi_mutex_service;

#endif /* HAVE_PSI_MUTEX_INTERFACE */

/** @} (end of group psi_abi_mutex) */

#endif /* MYSQL_PSI_MUTEX_H */
