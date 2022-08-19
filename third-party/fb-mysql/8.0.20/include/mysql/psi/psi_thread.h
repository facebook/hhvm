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

#ifndef MYSQL_PSI_THREAD_H
#define MYSQL_PSI_THREAD_H

/**
  @file include/mysql/psi/psi_thread.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_thread Thread Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_thread_bits.h"

/**
  @def PSI_THREAD_VERSION_1
  Performance Schema Thread Interface number for version 1.
  This version is obsolete.
*/
#define PSI_THREAD_VERSION_1 1

/**
  @def PSI_THREAD_VERSION_2
  Performance Schema Thread Interface number for version 2.
  This version is obsolete.
*/
#define PSI_THREAD_VERSION_2 2

/**
  @def PSI_THREAD_VERSION_3
  Performance Schema Thread Interface number for version 3.
  This version is supported.
*/
#define PSI_THREAD_VERSION_3 3

/**
  @def PSI_CURRENT_THREAD_VERSION
  Performance Schema Thread Interface number for the most recent version.
  The most current version is @c PSI_THREAD_VERSION_3
*/
#define PSI_CURRENT_THREAD_VERSION 3

/** Entry point for the performance schema interface. */
struct PSI_thread_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_THREAD_VERSION_1
    @sa PSI_THREAD_VERSION_2
    @sa PSI_THREAD_VERSION_3
    @sa PSI_CURRENT_THREAD_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_thread_bootstrap PSI_thread_bootstrap;

#ifdef HAVE_PSI_THREAD_INTERFACE

/**
  Performance Schema Thread Interface, version 3.
  @since PSI_THREAD_VERSION_3
*/
struct PSI_thread_service_v3 {
  /** @sa register_thread_v1_t. */
  register_thread_v1_t register_thread;
  /** @sa spawn_thread_v1_t. */
  spawn_thread_v1_t spawn_thread;
  /** @sa new_thread_v1_t. */
  new_thread_v1_t new_thread;
  /** @sa set_thread_id_v1_t. */
  set_thread_id_v1_t set_thread_id;
  /** @sa get_current_thread_internal_id_v2_t. */
  get_current_thread_internal_id_v2_t get_current_thread_internal_id;
  /** @sa get_thread_internal_id_v2_t. */
  get_thread_internal_id_v2_t get_thread_internal_id;
  /** @sa get_thread_by_id_v2_t. */
  get_thread_by_id_v2_t get_thread_by_id;
  /** @sa set_thread_THD_v1_t. */
  set_thread_THD_v1_t set_thread_THD;
  /** @sa set_thread_os_id_v1_t. */
  set_thread_os_id_v1_t set_thread_os_id;
  /** @sa get_thread_os_id_v1_t. */
  get_thread_os_id_v1_t get_thread_os_id;
  /** @sa set_thread_priority_v1_t. */
  set_thread_priority_v1_t set_thread_priority;
  /** @sa get_thread_priority_v1_t. */
  get_thread_priority_v1_t get_thread_priority;
  /** @sa get_thread_v1_t. */
  get_thread_v1_t get_thread;
  /** @sa set_thread_user_v1_t. */
  set_thread_user_v1_t set_thread_user;
  /** @sa set_thread_account_v1_t. */
  set_thread_account_v1_t set_thread_account;
  /** @sa set_thread_db_v1_t. */
  set_thread_db_v1_t set_thread_db;
  /** @sa set_thread_command_v1_t. */
  set_thread_command_v1_t set_thread_command;
  /** @sa set_connection_type_v1_t. */
  set_connection_type_v1_t set_connection_type;
  /** @sa set_thread_start_time_v1_t. */
  set_thread_start_time_v1_t set_thread_start_time;
  /** @sa set_thread_info_v1_t. */
  set_thread_info_v1_t set_thread_info;
  /** @sa set_thread_resource_group_v1_t. */
  set_thread_resource_group_v1_t set_thread_resource_group;
  /** @sa set_thread_resource_group_by_id_v1_t. */
  set_thread_resource_group_by_id_v1_t set_thread_resource_group_by_id;
  /** @sa set_thread_v1_t. */
  set_thread_v1_t set_thread;
  /** @sa aggregate_thread_status_v1_t. */
  aggregate_thread_status_v2_t aggregate_thread_status;
  /** @sa delete_current_thread_v1_t. */
  delete_current_thread_v1_t delete_current_thread;
  /** @sa delete_thread_v1_t. */
  delete_thread_v1_t delete_thread;
  /** @sa set_thread_connect_attrs_v1_t. */
  set_thread_connect_attrs_v1_t set_thread_connect_attrs;
  /** @sa set_thread_client_attrs_v1_t. */
  set_thread_client_attrs_v1_t set_thread_client_attrs;
  /** @sa get_current_thread_event_id_v2_t. */
  get_current_thread_event_id_v2_t get_current_thread_event_id;
  /** @sa get_thread_event_id_v2_t. */
  get_thread_event_id_v2_t get_thread_event_id;
  /** @sa get_thread_system_attrs_v1_t. */
  get_thread_system_attrs_v3_t get_thread_system_attrs;
  /** @sa get_thread_system_attrs_by_id_v1_t. */
  get_thread_system_attrs_by_id_v3_t get_thread_system_attrs_by_id;
  /** @sa register_notification_v1_t. */
  register_notification_v3_t register_notification;
  /** @sa unregister_notification_v1_t. */
  unregister_notification_v1_t unregister_notification;
  /** @sa notify_session_connect_v1_t. */
  notify_session_connect_v1_t notify_session_connect;
  /** @sa notify_session_disconnect_v1_t. */
  notify_session_disconnect_v1_t notify_session_disconnect;
  /** @sa notify_session_change_user_v1_t. */
  notify_session_change_user_v1_t notify_session_change_user;
};

typedef struct PSI_thread_service_v3 PSI_thread_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_thread_service_t *psi_thread_service;

#endif /* HAVE_PSI_THREAD_INTERFACE */

/** @} (end of group psi_abi_thread) */

#endif /* MYSQL_PSI_THREAD_H */
