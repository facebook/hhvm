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

#ifndef COMPONENTS_SERVICES_MYSQL_MUTEX_H
#define COMPONENTS_SERVICES_MYSQL_MUTEX_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/mysql_mutex_service.h>

#define REQUIRES_MYSQL_MUTEX_SERVICE REQUIRES_SERVICE(mysql_mutex_v1)
#define REQUIRES_MYSQL_MUTEX_SERVICE_PLACEHOLDER \
  REQUIRES_SERVICE_PLACEHOLDER(mysql_mutex_v1)

extern REQUIRES_MYSQL_MUTEX_SERVICE_PLACEHOLDER;

#define MYSQL_MUTEX_CALL(M) mysql_service_mysql_mutex_v1->M

#define mysql_mutex_register(P1, P2, P3) \
  MYSQL_MUTEX_CALL(register_info)(P1, P2, P3)

#define mysql_mutex_init(K, M, A) \
  mysql_mutex_init_with_src(K, M, A, __FILE__, __LINE__)
#define mysql_mutex_init_with_src(K, M, A, F, L) \
  MYSQL_MUTEX_CALL(init)(K, M, A, F, L)

#define mysql_mutex_destroy(M) \
  mysql_mutex_destroy_with_src(M, __FILE__, __LINE__)
#define mysql_mutex_destroy_with_src(M, F, L) MYSQL_MUTEX_CALL(destroy)(M, F, L)

#define mysql_mutex_lock(M) mysql_mutex_lock_with_src(M, __FILE__, __LINE__)
#define mysql_mutex_lock_with_src(M, F, L) MYSQL_MUTEX_CALL(lock)(M, F, L)

#define mysql_mutex_trylock(M) \
  mysql_mutex_trylock_with_src(M, __FILE__, __LINE__)
#define mysql_mutex_trylock_with_src(M, F, L) MYSQL_MUTEX_CALL(trylock)(M, F, L)

#define mysql_mutex_unlock(M) mysql_mutex_unlock_with_src(M, __FILE__, __LINE__)
#define mysql_mutex_unlock_with_src(M, F, L) MYSQL_MUTEX_CALL(unlock)(M, F, L)

#endif /* COMPONENTS_SERVICES_MYSQL_MUTEX_H */
