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

#ifndef MYSQL_PSI_COND_H
#define MYSQL_PSI_COND_H

/**
  @file include/mysql/psi/psi_cond.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_cond Cond Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "psi_base.h"

/** Entry point for the performance schema interface. */
struct PSI_cond_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_COND_VERSION_1
    @sa PSI_COND_VERSION_2
    @sa PSI_CURRENT_COND_VERSION
  */
  void *(*get_interface)(int version);
};

#ifdef HAVE_PSI_COND_INTERFACE

/**
  Performance Schema Cond Interface, version 1.
  @since PSI_COND_VERSION_1
*/
struct PSI_cond_service_v1 {
  /** @sa register_cond_v1_t. */
  register_cond_v1_t register_cond;
  /** @sa init_cond_v1_t. */
  init_cond_v1_t init_cond;
  /** @sa destroy_cond_v1_t. */
  destroy_cond_v1_t destroy_cond;
  /** @sa signal_cond_v1_t. */
  signal_cond_v1_t signal_cond;
  /** @sa broadcast_cond_v1_t. */
  broadcast_cond_v1_t broadcast_cond;
  /** @sa start_cond_wait_v1_t. */
  start_cond_wait_v1_t start_cond_wait;
  /** @sa end_cond_wait_v1_t. */
  end_cond_wait_v1_t end_cond_wait;
};

typedef struct PSI_cond_service_v1 PSI_cond_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_cond_service_t *psi_cond_service;

#endif /* HAVE_PSI_COND_INTERFACE */

/** @} (end of group psi_abi_cond) */

#endif /* MYSQL_PSI_MUTEX_H */
