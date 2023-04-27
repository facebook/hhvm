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

#ifndef MYSQL_PSI_SOCKET_H
#define MYSQL_PSI_SOCKET_H

/**
  @file include/mysql/psi/psi_socket.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_socket Socket Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_socket_bits.h"

/**
  @def PSI_SOCKET_VERSION_1
  Performance Schema Socket Interface number for version 1.
  This version is supported.
*/
#define PSI_SOCKET_VERSION_1 1

/** Entry point for the performance schema interface. */
struct PSI_socket_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_SOCKET_VERSION_1
    @sa PSI_SOCKET_VERSION_2
    @sa PSI_CURRENT_SOCKET_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_socket_bootstrap PSI_socket_bootstrap;

#ifdef HAVE_PSI_SOCKET_INTERFACE

/**
  Performance Schema Socket Interface, version 1.
  @since PSI_SOCKET_VERSION_1
*/
struct PSI_socket_service_v1 {
  /** @sa register_socket_v1_t. */
  register_socket_v1_t register_socket;
  /** @sa init_socket_v1_t. */
  init_socket_v1_t init_socket;
  /** @sa destroy_socket_v1_t. */
  destroy_socket_v1_t destroy_socket;
  /** @sa start_socket_wait_v1_t. */
  start_socket_wait_v1_t start_socket_wait;
  /** @sa end_socket_wait_v1_t. */
  end_socket_wait_v1_t end_socket_wait;
  /** @sa set_socket_state_v1_t. */
  set_socket_state_v1_t set_socket_state;
  /** @sa set_socket_info_v1_t. */
  set_socket_info_v1_t set_socket_info;
  /** @sa set_socket_thread_owner_v1_t. */
  set_socket_thread_owner_v1_t set_socket_thread_owner;
};

typedef struct PSI_socket_service_v1 PSI_socket_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_socket_service_t *psi_socket_service;

#endif /* HAVE_PSI_SOCKET_INTERFACE */

/** @} (end of group psi_abi_socket) */

#endif /* MYSQL_PSI_SOCKET_H */
