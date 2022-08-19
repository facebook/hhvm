/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "include/config.h"

#include "include/pfs_cond_provider.h"
#include "include/pfs_mutex_provider.h"
#include "include/pfs_rwlock_provider.h"
#include "include/pfs_socket_provider.h"
#include "include/pfs_thread_provider.h"
#include "my_inttypes.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/components/services/psi_socket_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/components/services/psi_thread_bits.h"

#ifndef WITH_LOCK_ORDER

#ifdef HAVE_PSI_COND_INTERFACE
void pfs_broadcast_cond_v1(PSI_cond *) {}
void pfs_destroy_cond_v1(PSI_cond *) {}
void pfs_end_cond_wait_v1(PSI_cond_locker *, int) {}
PSI_cond *pfs_init_cond_v1(PSI_cond_key, const void *) { return nullptr; }
void pfs_register_cond_v1(char const *, PSI_cond_info_v1 *, int) {}
void pfs_signal_cond_v1(PSI_cond *) {}
PSI_cond_locker *pfs_start_cond_wait_v1(PSI_cond_locker_state *, PSI_cond *,
                                        PSI_mutex *, PSI_cond_operation,
                                        const char *, uint) {
  return nullptr;
}
#endif  // HAVE_PSI_COND_INTERFACE

#ifdef HAVE_PSI_MUTEX_INTERFACE
void pfs_destroy_mutex_v1(PSI_mutex *) {}
void pfs_end_mutex_wait_v1(PSI_mutex_locker *, int) {}
PSI_mutex *pfs_init_mutex_v1(PSI_mutex_key, const void *) { return nullptr; }
void pfs_register_mutex_v1(char const *, PSI_mutex_info_v1 *, int) {}
PSI_mutex_locker *pfs_start_mutex_wait_v1(PSI_mutex_locker_state_v1 *,
                                          PSI_mutex *, PSI_mutex_operation,
                                          char const *, uint) {
  return nullptr;
}
void pfs_unlock_mutex_v1(PSI_mutex *) {}
#endif  // HAVE_PSI_MUTEX_INTERFACE

#ifdef HAVE_PSI_RWLOCK_INTERFACE
void pfs_destroy_rwlock_v2(PSI_rwlock *) {}
void pfs_end_rwlock_rdwait_v2(PSI_rwlock_locker *, int) {}
void pfs_end_rwlock_wrwait_v2(PSI_rwlock_locker *, int) {}
PSI_rwlock *pfs_init_rwlock_v2(PSI_rwlock_key, const void *) { return nullptr; }
void pfs_register_rwlock_v2(char const *, PSI_rwlock_info_v1 *, int) {}
PSI_rwlock_locker *pfs_start_rwlock_rdwait_v2(PSI_rwlock_locker_state_v1 *,
                                              PSI_rwlock *,
                                              PSI_rwlock_operation,
                                              char const *, uint) {
  return nullptr;
}
PSI_rwlock_locker *pfs_start_rwlock_wrwait_v2(PSI_rwlock_locker_state_v1 *,
                                              PSI_rwlock *,
                                              PSI_rwlock_operation,
                                              char const *, uint) {
  return nullptr;
}
void pfs_unlock_rwlock_v2(PSI_rwlock *, PSI_rwlock_operation) {}
#endif  // HAVE_PSI_RWLOCK_INTERFACE

#ifdef HAVE_PSI_SOCKET_INTERFACE
void pfs_destroy_socket_v1(PSI_socket *) {}
void pfs_end_socket_wait_v1(PSI_socket_locker *, size_t) {}
PSI_socket *pfs_init_socket_v1(PSI_socket_key, const my_socket *,
                               const struct sockaddr *, socklen_t) {
  return nullptr;
}
void pfs_register_socket_v1(char const *, PSI_socket_info_v1 *, int) {}
void pfs_set_socket_info_v1(PSI_socket *, const my_socket *,
                            const struct sockaddr *, socklen_t) {}
void pfs_set_socket_state_v1(PSI_socket *, PSI_socket_state) {}
void pfs_set_socket_thread_owner_v1(PSI_socket *) {}
PSI_socket_locker *pfs_start_socket_wait_v1(PSI_socket_locker_state_v1 *,
                                            PSI_socket *, PSI_socket_operation,
                                            size_t, char const *, uint) {
  return nullptr;
}
#endif  // HAVE_PSI_SOCKET_INTERFACE

#ifdef HAVE_PSI_THREAD_INTERFACE
void pfs_delete_current_thread_vc() {}
PSI_thread *pfs_new_thread_vc(PSI_thread_key, const void *, ulonglong) {
  return nullptr;
}
void pfs_register_thread_vc(char const *, PSI_thread_info_v1 *, int) {}
void pfs_set_thread_account_vc(char const *, int, char const *, int) {}
void pfs_set_thread_os_id_vc(PSI_thread *) {}
unsigned long long pfs_get_thread_os_id_vc(PSI_thread *) { return 0; }
void pfs_set_thread_priority_vc(PSI_thread *, int) {}
int pfs_get_thread_priority_vc(PSI_thread *) { return 0; }
void pfs_set_thread_vc(PSI_thread *) {}
int pfs_spawn_thread_vc(PSI_thread_key, my_thread_handle *,
                        const my_thread_attr_t *, void *(*)(void *), void *) {
  return 0;
}
int pfs_set_thread_connect_attrs_vc(char const *, unsigned int, void const *) {
  return 0;
}
#endif  // HAVE_PSI_THREAD_INTERFACE

#endif /* WITH_LOCK_ORDER */
