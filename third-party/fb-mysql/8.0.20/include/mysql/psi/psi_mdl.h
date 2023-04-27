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

#ifndef MYSQL_PSI_MDL_H
#define MYSQL_PSI_MDL_H

/**
  @file include/mysql/psi/psi_mdl.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_mdl Metadata Lock Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_mdl_bits.h"

/**
  @def PSI_MDL_VERSION_1
  Performance Schema Metadata Lock Interface number for version 1.
  This version is supported.
*/
#define PSI_MDL_VERSION_1 1

/**
  @def PSI_CURRENT_MDL_VERSION
  Performance Schema Metadata Lock Interface number for the most recent version.
  The most current version is @c PSI_MDL_VERSION_1
*/
#define PSI_CURRENT_MDL_VERSION 1

/** Entry point for the performance schema interface. */
struct PSI_mdl_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_MDL_VERSION_1
    @sa PSI_MDL_VERSION_2
    @sa PSI_CURRENT_MDL_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_mdl_bootstrap PSI_mdl_bootstrap;

#ifdef HAVE_PSI_METADATA_INTERFACE

/**
  Performance Schema Metadata Lock Interface, version 1.
  @since PSI_TRANSACTION_VERSION_1
*/
struct PSI_mdl_service_v1 {
  create_metadata_lock_v1_t create_metadata_lock;
  set_metadata_lock_status_v1_t set_metadata_lock_status;
  destroy_metadata_lock_v1_t destroy_metadata_lock;
  start_metadata_wait_v1_t start_metadata_wait;
  end_metadata_wait_v1_t end_metadata_wait;
};

typedef struct PSI_mdl_service_v1 PSI_mdl_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_mdl_service_t *psi_mdl_service;

#endif /* HAVE_PSI_METADATA_INTERFACE */

/** @} (end of group psi_abi_mdl) */

#endif /* MYSQL_PSI_MDL_H */
