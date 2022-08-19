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

#ifndef COMPONENTS_SERVICES_MYSQL_COND_H
#define COMPONENTS_SERVICES_MYSQL_COND_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/mysql_cond_service.h>
#include <mysql/components/services/mysql_mutex_bits.h>

#define REQUIRES_MYSQL_COND_SERVICE REQUIRES_SERVICE(mysql_cond_v1)
#define REQUIRES_MYSQL_COND_SERVICE_PLACEHOLDER \
  REQUIRES_SERVICE_PLACEHOLDER(mysql_cond_v1)

extern REQUIRES_MYSQL_COND_SERVICE_PLACEHOLDER;

#define MYSQL_COND_CALL(M) mysql_service_mysql_cond_v1->M

#define mysql_cond_register(P1, P2, P3) \
  MYSQL_COND_CALL(register_info)(P1, P2, P3)

#define mysql_cond_init(K, C) mysql_cond_init_with_src(K, C, __FILE__, __LINE__)
#define mysql_cond_init_with_src(K, C, F, L) MYSQL_COND_CALL(init)(K, C, F, L)

#define mysql_cond_destroy(C) mysql_cond_destroy_with_src(C, __FILE__, __LINE__)
#define mysql_cond_destroy_with_src(C, F, L) MYSQL_COND_CALL(destroy)(C, F, L)

#define mysql_cond_wait(C, M) mysql_cond_wait_with_src(C, M, __FILE__, __LINE__)
#define mysql_cond_wait_with_src(C, M, F, L) MYSQL_COND_CALL(wait)(C, M, F, L)

#define mysql_cond_timedwait(C, M, T) \
  mysql_cond_timedwait_with_src(C, M, T, __FILE__, __LINE__)
#define mysql_cond_timedwait_with_src(C, M, T, F, L) \
  MYSQL_COND_CALL(timedwait)(C, M, T, F, L)

#define mysql_cond_signal(C) mysql_cond_signal_with_src(C, __FILE__, __LINE__)
#define mysql_cond_signal_with_src(C, F, L) MYSQL_COND_CALL(signal)(C, F, L)

#define mysql_cond_broadcast(C) \
  mysql_cond_broadcast_with_src(C, __FILE__, __LINE__)
#define mysql_cond_broadcast_with_src(C, F, L) \
  MYSQL_COND_CALL(broadcast)(C, F, L)

#endif /* COMPONENTS_SERVICES_MYSQL_COND_H */
