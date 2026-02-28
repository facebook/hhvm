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

#ifndef MYSQL_PSI_FILE_H
#define MYSQL_PSI_FILE_H

/**
  @file include/mysql/psi/psi_file.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_file File Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_file_bits.h"

/**
  @def PSI_FILE_VERSION_1
  Performance Schema File Interface number for version 1.
  This version is obsolete.
*/
#define PSI_FILE_VERSION_1 1

/**
  @def PSI_FILE_VERSION_2
  Performance Schema File Interface number for version 2.
  This version is supported.
*/
#define PSI_FILE_VERSION_2 2

/**
  @def PSI_CURRENT_FILE_VERSION
  Performance Schema File Interface number for the most recent version.
  The most current version is @c PSI_FILE_VERSION_2
*/
#define PSI_CURRENT_FILE_VERSION 2

/** Entry point for the performance schema interface. */
struct PSI_file_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_FILE_VERSION_1
    @sa PSI_FILE_VERSION_2
    @sa PSI_CURRENT_FILE_VERSION
  */
  void *(*get_interface)(int version);
};

#ifdef HAVE_PSI_FILE_INTERFACE

/**
  Performance Schema file Interface, version 2.
  @since PSI_FILE_VERSION_2
*/
struct PSI_file_service_v2 {
  /** @sa register_file_v1_t. */
  register_file_v1_t register_file;
  /** @sa create_file_v1_t. */
  create_file_v1_t create_file;
  /** @sa get_thread_file_name_locker_v1_t. */
  get_thread_file_name_locker_v1_t get_thread_file_name_locker;
  /** @sa get_thread_file_stream_locker_v1_t. */
  get_thread_file_stream_locker_v1_t get_thread_file_stream_locker;
  /** @sa get_thread_file_descriptor_locker_v1_t. */
  get_thread_file_descriptor_locker_v1_t get_thread_file_descriptor_locker;
  /** @sa start_file_open_wait_v1_t. */
  start_file_open_wait_v1_t start_file_open_wait;
  /** @sa end_file_open_wait_v1_t. */
  end_file_open_wait_v1_t end_file_open_wait;
  /** @sa end_file_open_wait_and_bind_to_descriptor_v1_t. */
  end_file_open_wait_and_bind_to_descriptor_v1_t
      end_file_open_wait_and_bind_to_descriptor;
  /** @sa end_temp_file_open_wait_and_bind_to_descriptor_v1_t. */
  end_temp_file_open_wait_and_bind_to_descriptor_v1_t
      end_temp_file_open_wait_and_bind_to_descriptor;
  /** @sa start_file_wait_v1_t. */
  start_file_wait_v1_t start_file_wait;
  /** @sa end_file_wait_v1_t. */
  end_file_wait_v1_t end_file_wait;
  /** @sa start_file_close_wait_v1_t. */
  start_file_close_wait_v1_t start_file_close_wait;
  /** @sa end_file_close_wait_v1_t. */
  end_file_close_wait_v1_t end_file_close_wait;
  /** @sa start_file_rename_wait_v1_t. */
  start_file_rename_wait_v1_t start_file_rename_wait;
  /** @sa end_file_rename_wait_v1_t. */
  end_file_rename_wait_v1_t end_file_rename_wait;
};

typedef struct PSI_file_service_v2 PSI_file_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_file_service_t *psi_file_service;

#endif /* HAVE_PSI_FILE_INTERFACE */

/** @} (end of group psi_abi_file) */

#endif /* MYSQL_PSI_FILE_H */
