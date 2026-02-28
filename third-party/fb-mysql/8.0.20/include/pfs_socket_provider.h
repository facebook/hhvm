/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_SOCKET_PROVIDER_H
#define PFS_SOCKET_PROVIDER_H

/**
  @file include/pfs_socket_provider.h
  Performance schema instrumentation (declarations).
*/

#include "my_config.h"

#include "my_psi_config.h"

#ifdef HAVE_PSI_SOCKET_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include <stddef.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <sys/types.h>

#include "my_io.h"
#include "my_macros.h"
#include "mysql/psi/psi_socket.h"

#define PSI_SOCKET_CALL(M) pfs_##M##_v1

void pfs_register_socket_v1(const char *category, PSI_socket_info_v1 *info,
                            int count);

PSI_socket *pfs_init_socket_v1(PSI_socket_key key, const my_socket *fd,
                               const struct sockaddr *addr, socklen_t addr_len);

void pfs_destroy_socket_v1(PSI_socket *socket);

PSI_socket_locker *pfs_start_socket_wait_v1(PSI_socket_locker_state *state,
                                            PSI_socket *socket,
                                            PSI_socket_operation op,
                                            size_t count, const char *src_file,
                                            uint src_line);

void pfs_end_socket_wait_v1(PSI_socket_locker *locker, size_t byte_count);

void pfs_set_socket_state_v1(PSI_socket *socket, PSI_socket_state state);

void pfs_set_socket_info_v1(PSI_socket *socket, const my_socket *fd,
                            const struct sockaddr *addr, socklen_t addr_len);

void pfs_set_socket_thread_owner_v1(PSI_socket *socket);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_SOCKET_INTERFACE */

#endif
