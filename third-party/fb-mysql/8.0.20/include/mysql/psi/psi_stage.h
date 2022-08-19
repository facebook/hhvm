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

#ifndef MYSQL_PSI_STAGE_H
#define MYSQL_PSI_STAGE_H

/**
  @file include/mysql/psi/psi_stage.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_stage Stage Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sharedlib.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "psi_base.h"

/** Entry point for the performance schema interface. */
struct PSI_stage_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_STAGE_VERSION_1
    @sa PSI_STAGE_VERSION_2
    @sa PSI_CURRENT_STAGE_VERSION
  */
  void *(*get_interface)(int version);
};

#ifdef HAVE_PSI_STAGE_INTERFACE

/**
  Performance Schema Stage Interface, version 1.
  @since PSI_STAGE_VERSION_1
*/
struct PSI_stage_service_v1 {
  /** @sa register_stage_v1_t. */
  register_stage_v1_t register_stage;
  /** @sa start_stage_v1_t. */
  start_stage_v1_t start_stage;
  /** @sa get_current_stage_progress_v1_t. */
  get_current_stage_progress_v1_t get_current_stage_progress;
  /** @sa end_stage_v1_t. */
  end_stage_v1_t end_stage;
};

typedef struct PSI_stage_service_v1 PSI_stage_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_stage_service_t *psi_stage_service;

#endif /* HAVE_PSI_STAGE_INTERFACE */

/** @} (end of group psi_abi_stage) */

#endif /* MYSQL_PSI_FILE_H */
