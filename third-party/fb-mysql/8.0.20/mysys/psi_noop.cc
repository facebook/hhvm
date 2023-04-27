/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2.0,
  as published by the Free Software Foundation.

  This program is also distributed with certain software (including
  but not limited to OpenSSL) that is licensed under separate terms,
  as designated in a particular file or component or in included license
  documentation.  The authors of MySQL hereby grant you an additional
  permission to link the program and your derivative works with the
  separately licensed software that they have included with MySQL.

  Without limiting anything contained in the foregoing, this file,
  which is part of C Driver for MySQL (Connector/C), is also subject to the
  Universal FOSS Exception, version 1.0, a copy of which can be found at
  http://oss.oracle.com/licenses/universal-foss-exception.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License, version 2.0, for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/psi_noop.cc
  Always provide the noop performance interface, for plugins.
*/

#define HAVE_PSI_MUTEX_INTERFACE
#define HAVE_PSI_RWLOCK_INTERFACE
#define HAVE_PSI_COND_INTERFACE
#define HAVE_PSI_FILE_INTERFACE
#define HAVE_PSI_THREAD_INTERFACE
#define HAVE_PSI_TABLE_INTERFACE
#define HAVE_PSI_STAGE_INTERFACE
#define HAVE_PSI_STATEMENT_INTERFACE
#define HAVE_PSI_SP_INTERFACE
#define HAVE_PSI_PS_INTERFACE
#define HAVE_PSI_STATEMENT_DIGEST_INTERFACE
#define HAVE_PSI_TRANSACTION_INTERFACE
#define HAVE_PSI_SOCKET_INTERFACE
#define HAVE_PSI_MEMORY_INTERFACE
#define HAVE_PSI_ERROR_INTERFACE
#define HAVE_PSI_IDLE_INTERFACE
#define HAVE_PSI_METADATA_INTERFACE
#define HAVE_PSI_DATA_LOCK_INTERFACE
#define HAVE_PSI_SYSTEM_INTERFACE

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/types.h>
#include <time.h>

#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"  // IWYU pragma: keep
#include "my_thread.h"
#include "mysql/psi/psi_base.h"
#include "mysql/psi/psi_cond.h"
#include "mysql/psi/psi_data_lock.h"
#include "mysql/psi/psi_error.h"
#include "mysql/psi/psi_file.h"
#include "mysql/psi/psi_idle.h"
#include "mysql/psi/psi_mdl.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/psi/psi_mutex.h"
#include "mysql/psi/psi_rwlock.h"
#include "mysql/psi/psi_socket.h"
#include "mysql/psi/psi_stage.h"
#include "mysql/psi/psi_statement.h"
#include "mysql/psi/psi_system.h"
#include "mysql/psi/psi_table.h"
#include "mysql/psi/psi_thread.h"
#include "mysql/psi/psi_transaction.h"

class THD;
struct MDL_key;

// ===========================================================================

static void register_thread_noop(const char *, PSI_thread_info *, int) {
  return;
}

static int spawn_thread_noop(PSI_thread_key, my_thread_handle *thread,
                             const my_thread_attr_t *attr,
                             my_start_routine start_routine, void *arg) {
  return my_thread_create(thread, attr, start_routine, arg);
}

static PSI_thread *new_thread_noop(PSI_thread_key, const void *, ulonglong) {
  return nullptr;
}

static void set_thread_id_noop(PSI_thread *, ulonglong) { return; }

static ulonglong get_current_thread_internal_id_noop() { return 0; }

static ulonglong get_thread_internal_id_noop(PSI_thread *) { return 0; }

static PSI_thread *get_thread_by_id_noop(ulonglong) { return nullptr; }

static void set_thread_THD_noop(PSI_thread *, THD *) { return; }

static void set_thread_os_id_noop(PSI_thread *) { return; }

static ulonglong get_thread_os_id_noop(PSI_thread *) { return 0; }

static void set_thread_priority_noop(PSI_thread *, int) { return; }

static int get_thread_priority_noop(PSI_thread *) { return 0; }

static PSI_thread *get_thread_noop(void) { return nullptr; }

static void set_thread_user_noop(const char *, int) { return; }

static void set_thread_user_host_noop(const char *, int, const char *, int) {
  return;
}

static void set_thread_db_noop(const char *, int) { return; }

static void set_thread_command_noop(int) { return; }

static void set_connection_type_noop(opaque_vio_type) { return; }

static void set_thread_start_time_noop(time_t) { return; }

static void set_thread_info_noop(const char *, uint) { return; }

static void set_thread_noop(PSI_thread *) { return; }

static int set_thread_resource_group_noop(const char *, int, void *) {
  return 0;
}

static int set_thread_resource_group_by_id_noop(PSI_thread *, ulonglong,
                                                const char *, int, void *) {
  return 0;
}

static void aggregate_thread_status_noop(PSI_thread *) { return; }

static void delete_current_thread_noop(void) { return; }

static void delete_thread_noop(PSI_thread *) { return; }

static int set_thread_connect_attrs_noop(
    const char *buffer MY_ATTRIBUTE((unused)),
    uint length MY_ATTRIBUTE((unused)),
    const void *from_cs MY_ATTRIBUTE((unused))) {
  return 0;
}

static int set_thread_client_attrs_noop(
    const uchar *client_id MY_ATTRIBUTE((unused)),
    const char *client_attributes MY_ATTRIBUTE((unused)),
    uint client_attributes_length MY_ATTRIBUTE((unused))) {
  return 0;
}

static void get_current_thread_event_id_noop(ulonglong *thread_internal_id,
                                             ulonglong *event_id) {
  *thread_internal_id = 0;
  *event_id = 0;
}

static void get_thread_event_id_noop(PSI_thread *,
                                     ulonglong *thread_internal_id,
                                     ulonglong *event_id) {
  *thread_internal_id = 0;
  *event_id = 0;
}

static int get_thread_system_attrs_noop(PSI_thread_attrs *) { return 0; }

static int get_thread_system_attrs_by_id_noop(PSI_thread *, ulonglong,
                                              PSI_thread_attrs *) {
  return 0;
}

static int register_notification_noop(const PSI_notification *, bool) {
  return 0;
}

static int unregister_notification_noop(int) { return 0; }

static void notify_session_connect_noop(PSI_thread *) { return; }

static void notify_session_disconnect_noop(PSI_thread *) { return; }

static void notify_session_change_user_noop(PSI_thread *) { return; }

static PSI_thread_service_t psi_thread_noop = {
    register_thread_noop,
    spawn_thread_noop,
    new_thread_noop,
    set_thread_id_noop,
    get_current_thread_internal_id_noop,
    get_thread_internal_id_noop,
    get_thread_by_id_noop,
    set_thread_THD_noop,
    set_thread_os_id_noop,
    get_thread_os_id_noop,
    set_thread_priority_noop,
    get_thread_priority_noop,
    get_thread_noop,
    set_thread_user_noop,
    set_thread_user_host_noop,
    set_thread_db_noop,
    set_thread_command_noop,
    set_connection_type_noop,
    set_thread_start_time_noop,
    set_thread_info_noop,
    set_thread_resource_group_noop,
    set_thread_resource_group_by_id_noop,
    set_thread_noop,
    aggregate_thread_status_noop,
    delete_current_thread_noop,
    delete_thread_noop,
    set_thread_connect_attrs_noop,
    set_thread_client_attrs_noop,
    get_current_thread_event_id_noop,
    get_thread_event_id_noop,
    get_thread_system_attrs_noop,
    get_thread_system_attrs_by_id_noop,
    register_notification_noop,
    unregister_notification_noop,
    notify_session_connect_noop,
    notify_session_disconnect_noop,
    notify_session_change_user_noop};

struct PSI_thread_bootstrap *psi_thread_hook = nullptr;
PSI_thread_service_t *psi_thread_service = &psi_thread_noop;

void set_psi_thread_service(void *psi) {
  psi_thread_service = (PSI_thread_service_t *)psi;
}

// ===========================================================================

static void register_mutex_noop(const char *, PSI_mutex_info *, int) { return; }

static PSI_mutex *init_mutex_noop(PSI_mutex_key, const void *) {
  return nullptr;
}

static void destroy_mutex_noop(PSI_mutex *) { return; }

static PSI_mutex_locker *start_mutex_wait_noop(PSI_mutex_locker_state *,
                                               PSI_mutex *, PSI_mutex_operation,
                                               const char *, uint) {
  return nullptr;
}

static void end_mutex_wait_noop(PSI_mutex_locker *, int) { return; }

static void unlock_mutex_noop(PSI_mutex *) { return; }

static PSI_mutex_service_t psi_mutex_noop = {
    register_mutex_noop,   init_mutex_noop,     destroy_mutex_noop,
    start_mutex_wait_noop, end_mutex_wait_noop, unlock_mutex_noop};

struct PSI_mutex_bootstrap *psi_mutex_hook = nullptr;
PSI_mutex_service_t *psi_mutex_service = &psi_mutex_noop;

void set_psi_mutex_service(void *psi) {
  psi_mutex_service = (PSI_mutex_service_t *)psi;
}

// ===========================================================================

static void register_rwlock_noop(const char *, PSI_rwlock_info *, int) {
  return;
}

static PSI_rwlock *init_rwlock_noop(PSI_rwlock_key, const void *) {
  return nullptr;
}

static void destroy_rwlock_noop(PSI_rwlock *) { return; }

static PSI_rwlock_locker *start_rwlock_rdwait_noop(
    struct PSI_rwlock_locker_state_v1 *, struct PSI_rwlock *,
    enum PSI_rwlock_operation, const char *, uint) {
  return nullptr;
}

static void end_rwlock_rdwait_noop(PSI_rwlock_locker *, int) { return; }

static struct PSI_rwlock_locker *start_rwlock_wrwait_noop(
    struct PSI_rwlock_locker_state_v1 *, struct PSI_rwlock *,
    enum PSI_rwlock_operation, const char *, uint) {
  return nullptr;
}

static void end_rwlock_wrwait_noop(PSI_rwlock_locker *, int) { return; }

static void unlock_rwlock_noop(PSI_rwlock *, enum PSI_rwlock_operation) {
  return;
}

static PSI_rwlock_service_t psi_rwlock_noop = {
    register_rwlock_noop,     init_rwlock_noop,       destroy_rwlock_noop,
    start_rwlock_rdwait_noop, end_rwlock_rdwait_noop, start_rwlock_wrwait_noop,
    end_rwlock_wrwait_noop,   unlock_rwlock_noop};

struct PSI_rwlock_bootstrap *psi_rwlock_hook = nullptr;
PSI_rwlock_service_t *psi_rwlock_service = &psi_rwlock_noop;

void set_psi_rwlock_service(void *psi) {
  psi_rwlock_service = (PSI_rwlock_service_t *)psi;
}

// ===========================================================================

static void register_cond_noop(const char *, PSI_cond_info *, int) { return; }

static PSI_cond *init_cond_noop(PSI_cond_key, const void *) { return nullptr; }

static void destroy_cond_noop(PSI_cond *) { return; }

static void signal_cond_noop(PSI_cond *) { return; }

static void broadcast_cond_noop(PSI_cond *) { return; }

static struct PSI_cond_locker *start_cond_wait_noop(
    struct PSI_cond_locker_state_v1 *, struct PSI_cond *, struct PSI_mutex *,
    enum PSI_cond_operation, const char *, uint) {
  return nullptr;
}

static void end_cond_wait_noop(PSI_cond_locker *, int) { return; }

static PSI_cond_service_t psi_cond_noop = {
    register_cond_noop, init_cond_noop,      destroy_cond_noop,
    signal_cond_noop,   broadcast_cond_noop, start_cond_wait_noop,
    end_cond_wait_noop};

struct PSI_cond_bootstrap *psi_cond_hook = nullptr;
PSI_cond_service_t *psi_cond_service = &psi_cond_noop;

void set_psi_cond_service(void *psi) {
  psi_cond_service = (PSI_cond_service_t *)psi;
}

// ===========================================================================

static void register_file_noop(const char *, PSI_file_info *, int) { return; }

static PSI_file_locker *get_thread_file_name_locker_noop(
    PSI_file_locker_state *, PSI_file_key, enum PSI_file_operation,
    const char *, const void *) {
  return nullptr;
}

static PSI_file_locker *get_thread_file_stream_locker_noop(
    PSI_file_locker_state *, PSI_file *, enum PSI_file_operation) {
  return nullptr;
}

static PSI_file_locker *get_thread_file_descriptor_locker_noop(
    PSI_file_locker_state *, File, enum PSI_file_operation) {
  return nullptr;
}

static void create_file_noop(PSI_file_key, const char *, File) { return; }

static void start_file_open_wait_noop(PSI_file_locker *, const char *, uint) {
  return;
}

static PSI_file *end_file_open_wait_noop(PSI_file_locker *, void *) {
  return nullptr;
}

static void end_file_open_wait_and_bind_to_descriptor_noop(PSI_file_locker *,
                                                           File) {
  return;
}

static void end_temp_file_open_wait_and_bind_to_descriptor_noop(
    PSI_file_locker *, File, const char *) {
  return;
}

static void start_file_wait_noop(PSI_file_locker *, size_t, const char *,
                                 uint) {
  return;
}

static void end_file_wait_noop(PSI_file_locker *, size_t) { return; }

static void start_file_close_wait_noop(PSI_file_locker *, const char *, uint) {
  return;
}

static void end_file_close_wait_noop(PSI_file_locker *, int) { return; }

static void start_file_rename_wait_noop(PSI_file_locker *, size_t, const char *,
                                        const char *, const char *, uint) {
  return;
}

static void end_file_rename_wait_noop(PSI_file_locker *, const char *,
                                      const char *, int) {
  return;
}

static PSI_file_service_t psi_file_noop = {
    register_file_noop,
    create_file_noop,
    get_thread_file_name_locker_noop,
    get_thread_file_stream_locker_noop,
    get_thread_file_descriptor_locker_noop,
    start_file_open_wait_noop,
    end_file_open_wait_noop,
    end_file_open_wait_and_bind_to_descriptor_noop,
    end_temp_file_open_wait_and_bind_to_descriptor_noop,
    start_file_wait_noop,
    end_file_wait_noop,
    start_file_close_wait_noop,
    end_file_close_wait_noop,
    start_file_rename_wait_noop,
    end_file_rename_wait_noop};

struct PSI_file_bootstrap *psi_file_hook = nullptr;
PSI_file_service_t *psi_file_service = &psi_file_noop;

void set_psi_file_service(void *psi) {
  psi_file_service = (PSI_file_service_t *)psi;
}

// ===========================================================================

static void register_socket_noop(const char *, PSI_socket_info *, int) {
  return;
}

static PSI_socket *init_socket_noop(PSI_socket_key, const my_socket *,
                                    const struct sockaddr *, socklen_t) {
  return nullptr;
}

static void destroy_socket_noop(PSI_socket *) { return; }

static PSI_socket_locker *start_socket_wait_noop(PSI_socket_locker_state *,
                                                 PSI_socket *,
                                                 PSI_socket_operation, size_t,
                                                 const char *, uint) {
  return nullptr;
}

static void end_socket_wait_noop(PSI_socket_locker *, size_t) { return; }

static void set_socket_state_noop(PSI_socket *, enum PSI_socket_state) {
  return;
}

static void set_socket_info_noop(PSI_socket *, const my_socket *,
                                 const struct sockaddr *, socklen_t) {
  return;
}

static void set_socket_thread_owner_noop(PSI_socket *) { return; }

static PSI_socket_service_t psi_socket_noop = {
    register_socket_noop, init_socket_noop,
    destroy_socket_noop,  start_socket_wait_noop,
    end_socket_wait_noop, set_socket_state_noop,
    set_socket_info_noop, set_socket_thread_owner_noop};

struct PSI_socket_bootstrap *psi_socket_hook = nullptr;
PSI_socket_service_t *psi_socket_service = &psi_socket_noop;

void set_psi_socket_service(void *psi) {
  psi_socket_service = (PSI_socket_service_t *)psi;
}

// ===========================================================================

static PSI_table_share *get_table_share_noop(bool, struct TABLE_SHARE *) {
  return nullptr;
}

static void release_table_share_noop(PSI_table_share *) { return; }

static void drop_table_share_noop(bool, const char *, int, const char *, int) {
  return;
}

static PSI_table *open_table_noop(PSI_table_share *, const void *) {
  return nullptr;
}

static void unbind_table_noop(PSI_table *) { return; }

static PSI_table *rebind_table_noop(PSI_table_share *, const void *,
                                    PSI_table *) {
  return nullptr;
}

static void close_table_noop(struct TABLE_SHARE *, PSI_table *) { return; }

static struct PSI_table_locker *start_table_io_wait_noop(
    struct PSI_table_locker_state *, struct PSI_table *,
    enum PSI_table_io_operation, uint, const char *, uint) {
  return nullptr;
}

static void end_table_io_wait_noop(PSI_table_locker *, ulonglong) { return; }

static struct PSI_table_locker *start_table_lock_wait_noop(
    struct PSI_table_locker_state *, struct PSI_table *,
    enum PSI_table_lock_operation, ulong, const char *, uint) {
  return nullptr;
}

static void end_table_lock_wait_noop(PSI_table_locker *) { return; }

static void unlock_table_noop(PSI_table *) { return; }

static PSI_table_service_t psi_table_noop = {
    get_table_share_noop,     release_table_share_noop,
    drop_table_share_noop,    open_table_noop,
    unbind_table_noop,        rebind_table_noop,
    close_table_noop,         start_table_io_wait_noop,
    end_table_io_wait_noop,   start_table_lock_wait_noop,
    end_table_lock_wait_noop, unlock_table_noop};

struct PSI_table_bootstrap *psi_table_hook = nullptr;
PSI_table_service_t *psi_table_service = &psi_table_noop;

void set_psi_table_service(void *psi) {
  psi_table_service = (PSI_table_service_t *)psi;
}

// ===========================================================================

static PSI_metadata_lock *create_metadata_lock_noop(void *, const MDL_key *,
                                                    opaque_mdl_type,
                                                    opaque_mdl_duration,
                                                    opaque_mdl_status,
                                                    const char *, uint) {
  return nullptr;
}

static void set_metadata_lock_status_noop(PSI_metadata_lock *,
                                          opaque_mdl_status) {}

static void destroy_metadata_lock_noop(PSI_metadata_lock *) {}

static PSI_metadata_locker *start_metadata_wait_noop(
    PSI_metadata_locker_state *, PSI_metadata_lock *, const char *, uint) {
  return nullptr;
}

static void end_metadata_wait_noop(PSI_metadata_locker *, int) {}

static PSI_mdl_service_t psi_mdl_noop = {
    create_metadata_lock_noop, set_metadata_lock_status_noop,
    destroy_metadata_lock_noop, start_metadata_wait_noop,
    end_metadata_wait_noop};

struct PSI_mdl_bootstrap *psi_mdl_hook = nullptr;
PSI_mdl_service_t *psi_mdl_service = &psi_mdl_noop;

void set_psi_mdl_service(void *psi) {
  psi_mdl_service = (PSI_mdl_service_t *)psi;
}

// ===========================================================================

static PSI_idle_locker *start_idle_wait_noop(PSI_idle_locker_state *,
                                             const char *, uint) {
  return nullptr;
}

static void end_idle_wait_noop(PSI_idle_locker *) { return; }

static PSI_idle_service_t psi_idle_noop = {start_idle_wait_noop,
                                           end_idle_wait_noop};

struct PSI_idle_bootstrap *psi_idle_hook = nullptr;
PSI_idle_service_t *psi_idle_service = &psi_idle_noop;

void set_psi_idle_service(void *psi) {
  psi_idle_service = (PSI_idle_service_t *)psi;
}

// ===========================================================================

static void register_stage_noop(const char *, PSI_stage_info **, int) {
  return;
}

static PSI_stage_progress *start_stage_noop(PSI_stage_key, const char *, int) {
  return nullptr;
}

static PSI_stage_progress *get_current_stage_progress_noop() { return nullptr; }

static void end_stage_noop(void) { return; }

static PSI_stage_service_t psi_stage_noop = {
    register_stage_noop, start_stage_noop, get_current_stage_progress_noop,
    end_stage_noop};

struct PSI_stage_bootstrap *psi_stage_hook = nullptr;
PSI_stage_service_t *psi_stage_service = &psi_stage_noop;

void set_psi_stage_service(void *psi) {
  psi_stage_service = (PSI_stage_service_t *)psi;
}

// ===========================================================================

static void register_statement_noop(const char *, PSI_statement_info *, int) {
  return;
}

static PSI_statement_locker *get_thread_statement_locker_noop(
    PSI_statement_locker_state *, PSI_statement_key, const void *,
    PSI_sp_share *) {
  return nullptr;
}

static PSI_statement_locker *refine_statement_noop(PSI_statement_locker *,
                                                   PSI_statement_key) {
  return nullptr;
}

static void start_statement_noop(PSI_statement_locker *, const char *, uint,
                                 const char *, uint) {
  return;
}

static void set_statement_text_noop(PSI_statement_locker *, const char *,
                                    uint) {
  return;
}

static void set_statement_query_id_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void set_statement_lock_time_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void set_statement_cpu_time_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void set_statement_rows_sent_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void set_statement_rows_examined_noop(PSI_statement_locker *,
                                             ulonglong) {
  return;
}

static void inc_statement_rows_deleted_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void inc_statement_rows_inserted_noop(PSI_statement_locker *,
                                             ulonglong) {
  return;
}

static void inc_statement_rows_updated_noop(PSI_statement_locker *, ulonglong) {
  return;
}

static void inc_statement_tmp_table_bytes_written_noop(PSI_statement_locker *,
                                                       ulonglong) {
  return;
}

static void inc_statement_filesort_bytes_written_noop(PSI_statement_locker *,
                                                      ulonglong) {
  return;
}

static void inc_statement_index_dive_count_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_index_dive_cpu_noop(PSI_statement_locker *,
                                              ulonglong) {
  return;
}

static void inc_statement_compilation_cpu_noop(PSI_statement_locker *,
                                               ulonglong) {
  return;
}

static void inc_statement_created_tmp_disk_tables_noop(PSI_statement_locker *,
                                                       ulong) {
  return;
}

static void inc_statement_created_tmp_tables_noop(PSI_statement_locker *,
                                                  ulong) {
  return;
}

static void inc_statement_select_full_join_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_select_full_range_join_noop(PSI_statement_locker *,
                                                      ulong) {
  return;
}

static void inc_statement_select_range_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_select_range_check_noop(PSI_statement_locker *,
                                                  ulong) {
  return;
}

static void inc_statement_select_scan_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_sort_merge_passes_noop(PSI_statement_locker *,
                                                 ulong) {
  return;
}

static void inc_statement_sort_range_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_sort_rows_noop(PSI_statement_locker *, ulong) {
  return;
}

static void inc_statement_sort_scan_noop(PSI_statement_locker *, ulong) {
  return;
}

static void set_statement_no_index_used_noop(PSI_statement_locker *) { return; }

static void set_statement_no_good_index_used_noop(PSI_statement_locker *) {
  return;
}

static void update_statement_filesort_disk_usage_noop(PSI_statement_locker *,
                                                      ulonglong) {
  return;
}

static void update_statement_tmp_table_disk_usage_noop(PSI_statement_locker *,
                                                       ulonglong) {
  return;
}

static void end_statement_noop(PSI_statement_locker *, void *) { return; }

static PSI_prepared_stmt *create_prepared_stmt_noop(void *, uint,
                                                    PSI_statement_locker *,
                                                    const char *, size_t,
                                                    const char *, size_t) {
  return nullptr;
}

static void destroy_prepared_stmt_noop(PSI_prepared_stmt *) { return; }

static void reprepare_prepared_stmt_noop(PSI_prepared_stmt *) { return; }

static void execute_prepared_stmt_noop(PSI_statement_locker *,
                                       PSI_prepared_stmt *) {
  return;
}

static void set_prepared_stmt_text_noop(PSI_prepared_stmt *, const char *,
                                        uint) {
  return;
}

static struct PSI_digest_locker *digest_start_noop(PSI_statement_locker *) {
  return nullptr;
}

static void digest_end_noop(PSI_digest_locker *,
                            const struct sql_digest_storage *) {
  return;
}

static PSI_sp_share *get_sp_share_noop(uint, const char *, uint, const char *,
                                       uint) {
  return nullptr;
}

static void release_sp_share_noop(PSI_sp_share *) { return; }

static PSI_sp_locker *start_sp_noop(PSI_sp_locker_state *, PSI_sp_share *) {
  return nullptr;
}

static void end_sp_noop(PSI_sp_locker *) { return; }

static void drop_sp_noop(uint, const char *, uint, const char *, uint) {
  return;
}

static PSI_statement_service_t psi_statement_noop = {
    register_statement_noop,
    get_thread_statement_locker_noop,
    refine_statement_noop,
    start_statement_noop,
    set_statement_text_noop,
    set_statement_query_id_noop,
    set_statement_lock_time_noop,
    set_statement_cpu_time_noop,
    set_statement_rows_sent_noop,
    set_statement_rows_examined_noop,
    inc_statement_rows_deleted_noop,
    inc_statement_rows_inserted_noop,
    inc_statement_rows_updated_noop,
    inc_statement_tmp_table_bytes_written_noop,
    inc_statement_filesort_bytes_written_noop,
    inc_statement_index_dive_count_noop,
    inc_statement_index_dive_cpu_noop,
    inc_statement_compilation_cpu_noop,
    inc_statement_created_tmp_disk_tables_noop,
    inc_statement_created_tmp_tables_noop,
    inc_statement_select_full_join_noop,
    inc_statement_select_full_range_join_noop,
    inc_statement_select_range_noop,
    inc_statement_select_range_check_noop,
    inc_statement_select_scan_noop,
    inc_statement_sort_merge_passes_noop,
    inc_statement_sort_range_noop,
    inc_statement_sort_rows_noop,
    inc_statement_sort_scan_noop,
    set_statement_no_index_used_noop,
    set_statement_no_good_index_used_noop,
    update_statement_filesort_disk_usage_noop,
    update_statement_tmp_table_disk_usage_noop,
    end_statement_noop,
    create_prepared_stmt_noop,
    destroy_prepared_stmt_noop,
    reprepare_prepared_stmt_noop,
    execute_prepared_stmt_noop,
    set_prepared_stmt_text_noop,
    digest_start_noop,
    digest_end_noop,
    get_sp_share_noop,
    release_sp_share_noop,
    start_sp_noop,
    end_sp_noop,
    drop_sp_noop};

struct PSI_statement_bootstrap *psi_statement_hook = nullptr;
PSI_statement_service_t *psi_statement_service = &psi_statement_noop;

void set_psi_statement_service(void *psi) {
  psi_statement_service = (PSI_statement_service_t *)psi;
}

// ===========================================================================

static PSI_transaction_locker *get_thread_transaction_locker_noop(
    PSI_transaction_locker_state *, const void *, const ulonglong *, int, bool,
    bool) {
  return nullptr;
}

static void start_transaction_noop(PSI_transaction_locker *, const char *,
                                   uint) {
  return;
}

static void set_transaction_xid_noop(PSI_transaction_locker *, const void *,
                                     int) {
  return;
}

static void set_transaction_xa_state_noop(PSI_transaction_locker *, int) {
  return;
}

static void set_transaction_gtid_noop(PSI_transaction_locker *, const void *,
                                      const void *) {
  return;
}

static void set_transaction_trxid_noop(PSI_transaction_locker *,
                                       const ulonglong *) {
  return;
}

static void inc_transaction_savepoints_noop(PSI_transaction_locker *, ulong) {
  return;
}

static void inc_transaction_rollback_to_savepoint_noop(PSI_transaction_locker *,
                                                       ulong) {
  return;
}

static void inc_transaction_release_savepoint_noop(PSI_transaction_locker *,
                                                   ulong) {
  return;
}

static void end_transaction_noop(PSI_transaction_locker *, bool) { return; }

static PSI_transaction_service_t psi_transaction_noop = {
    get_thread_transaction_locker_noop,
    start_transaction_noop,
    set_transaction_xid_noop,
    set_transaction_xa_state_noop,
    set_transaction_gtid_noop,
    set_transaction_trxid_noop,
    inc_transaction_savepoints_noop,
    inc_transaction_rollback_to_savepoint_noop,
    inc_transaction_release_savepoint_noop,
    end_transaction_noop};

struct PSI_transaction_bootstrap *psi_transaction_hook = nullptr;
PSI_transaction_service_t *psi_transaction_service = &psi_transaction_noop;

void set_psi_transaction_service(void *psi) {
  psi_transaction_service = (PSI_transaction_service_t *)psi;
}

// ===========================================================================

static void log_error_noop(unsigned int, PSI_error_operation) {}

static PSI_error_service_t psi_error_noop = {log_error_noop};

struct PSI_error_bootstrap *psi_error_hook = nullptr;
PSI_error_service_t *psi_error_service = &psi_error_noop;

void set_psi_error_service(void *psi) {
  psi_error_service = (PSI_error_service_t *)psi;
}

// ===========================================================================

static void register_memory_noop(const char *, PSI_memory_info *, int) {
  return;
}

static PSI_memory_key memory_alloc_noop(PSI_memory_key, size_t,
                                        struct PSI_thread **owner) {
  *owner = nullptr;
  return PSI_NOT_INSTRUMENTED;
}

static PSI_memory_key memory_realloc_noop(PSI_memory_key, size_t, size_t,
                                          struct PSI_thread **owner) {
  *owner = nullptr;
  return PSI_NOT_INSTRUMENTED;
}

static PSI_memory_key memory_claim_noop(PSI_memory_key, size_t,
                                        struct PSI_thread **owner) {
  *owner = nullptr;
  return PSI_NOT_INSTRUMENTED;
}

static void memory_free_noop(PSI_memory_key, size_t, struct PSI_thread *) {
  return;
}

static PSI_memory_service_t psi_memory_noop = {
    register_memory_noop, memory_alloc_noop, memory_realloc_noop,
    memory_claim_noop, memory_free_noop};

struct PSI_memory_bootstrap *psi_memory_hook = nullptr;
PSI_memory_service_t *psi_memory_service = &psi_memory_noop;

void set_psi_memory_service(void *psi) {
  psi_memory_service = (PSI_memory_service_t *)psi;
}

// ===========================================================================

static void register_data_lock_noop(PSI_engine_data_lock_inspector *) {
  return;
}

static void unregister_data_lock_noop(PSI_engine_data_lock_inspector *) {
  return;
}

static PSI_data_lock_service_t psi_data_lock_noop = {register_data_lock_noop,
                                                     unregister_data_lock_noop};

struct PSI_data_lock_bootstrap *psi_data_lock_hook = nullptr;
PSI_data_lock_service_t *psi_data_lock_service = &psi_data_lock_noop;

void set_psi_data_lock_service(void *psi) {
  psi_data_lock_service = (PSI_data_lock_service_t *)psi;
}

// ===========================================================================

static void unload_plugin_noop(const char *) { return; }

static PSI_system_service_t psi_system_noop = {unload_plugin_noop};

struct PSI_system_bootstrap *psi_system_hook = nullptr;
PSI_system_service_t *psi_system_service = &psi_system_noop;

void set_psi_system_service(void *psi) {
  psi_system_service = (PSI_system_service_t *)psi;
}
