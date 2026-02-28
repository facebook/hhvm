/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PSI_ERROR_H
#define MYSQL_PSI_ERROR_H

/**
  @file include/mysql/psi/psi_error.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_error Error Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_error_bits.h"

/**
  @def PSI_ERROR_VERSION_1
  Performance Schema Error Interface number for version 1.
  This version is supported.
*/
#define PSI_ERROR_VERSION_1 1

/**
  @def PSI_CURRENT_ERROR_VERSION
  Performance Schema Error Interface number for the most recent version.
  The most current version is @c PSI_ERROR_VERSION_1
*/
#define PSI_CURRENT_ERROR_VERSION 1

/** Entry point for the performance schema interface. */
struct PSI_error_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_ERROR_VERSION_1
    @sa PSI_ERROR_VERSION_2
    @sa PSI_CURRENT_ERROR_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_error_bootstrap PSI_error_bootstrap;

#ifdef HAVE_PSI_ERROR_INTERFACE

/**
  Performance Schema Error Interface, version 1.
  @since PSI_ERROR_VERSION_1
*/
struct PSI_error_service_v1 {
  /** @sa log_error_v1_t. */
  log_error_v1_t log_error;
};

typedef struct PSI_error_service_v1 PSI_error_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_error_service_t *psi_error_service;

#endif /* HAVE_PSI_ERROR_INTERFACE */

/** @} (end of group psi_abi_error) */

#endif /* MYSQL_PSI_ERROR_H */
