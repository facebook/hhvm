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

#ifndef MYSQL_PSI_RWLOCK_H
#define MYSQL_PSI_RWLOCK_H

/**
  @file include/mysql/psi/psi_rwlock.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_rwlock Rwlock Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "psi_base.h"

/*
  MAINTAINER:
  The following pattern:
    typedef struct XYZ XYZ;
  is not needed in C++, but required for C.
*/

/** Entry point for the performance schema interface. */
struct PSI_rwlock_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_RWLOCK_VERSION_1
    @sa PSI_RWLOCK_VERSION_2
    @sa PSI_CURRENT_RWLOCK_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_rwlock_bootstrap PSI_rwlock_bootstrap;

#ifdef HAVE_PSI_RWLOCK_INTERFACE

/**
  Performance Schema Rwlock Interface, version 2.
  @since PSI_RWLOCK_VERSION_2
*/
struct PSI_rwlock_service_v2 {
  /** @sa register_rwlock_v1_t. */
  register_rwlock_v1_t register_rwlock;
  /** @sa init_rwlock_v1_t. */
  init_rwlock_v1_t init_rwlock;
  /** @sa destroy_rwlock_v1_t. */
  destroy_rwlock_v1_t destroy_rwlock;
  /** @sa start_rwlock_rdwait_v1_t. */
  start_rwlock_rdwait_v1_t start_rwlock_rdwait;
  /** @sa end_rwlock_rdwait_v1_t. */
  end_rwlock_rdwait_v1_t end_rwlock_rdwait;
  /** @sa start_rwlock_wrwait_v1_t. */
  start_rwlock_wrwait_v1_t start_rwlock_wrwait;
  /** @sa end_rwlock_wrwait_v1_t. */
  end_rwlock_wrwait_v1_t end_rwlock_wrwait;
  /** @sa unlock_rwlock_v2_t. */
  unlock_rwlock_v2_t unlock_rwlock;
};

typedef struct PSI_rwlock_service_v2 PSI_rwlock_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_rwlock_service_t *psi_rwlock_service;

#endif /* HAVE_PSI_RWLOCK_INTERFACE */

/** @} (end of group psi_abi_rwlock) */

#endif /* MYSQL_PSI_RWLOCK_H */
