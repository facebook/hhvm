/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_PSI_SOCKET_SERVICE_H
#define COMPONENTS_SERVICES_PSI_SOCKET_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/psi_socket_bits.h>

BEGIN_SERVICE_DEFINITION(psi_socket_v1)
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
END_SERVICE_DEFINITION(psi_socket_v1)

#endif /* COMPONENTS_SERVICES_PSI_SOCKET_SERVICE_H */
