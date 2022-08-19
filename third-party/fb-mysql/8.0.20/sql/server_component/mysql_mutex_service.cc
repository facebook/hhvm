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

#include <mysql/components/component_implementation.h>
#include <mysql/components/service.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/mysql_mutex_service.h>
#include <mysql/psi/mysql_mutex.h>

void impl_mysql_mutex_register(const char *category, PSI_mutex_info *info,
                               int count) {
  mysql_mutex_register(category, info, count);
}

int impl_mysql_mutex_init(PSI_mutex_key key, mysql_mutex_t *that,
                          const native_mutexattr_t *attr, const char *src_file,
                          unsigned int src_line) {
  return mysql_mutex_init_with_src(key, that, attr, src_file, src_line);
}

int impl_mysql_mutex_destroy(mysql_mutex_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_mutex_destroy_with_src(that, src_file, src_line);
}

int impl_mysql_mutex_lock(mysql_mutex_t *that, const char *src_file,
                          unsigned int src_line) {
  return mysql_mutex_lock_with_src(that, src_file, src_line);
}

int impl_mysql_mutex_trylock(mysql_mutex_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_mutex_trylock_with_src(that, src_file, src_line);
}

int impl_mysql_mutex_unlock(mysql_mutex_t *that, const char *src_file,
                            unsigned int src_line) {
  return mysql_mutex_unlock_with_src(that, src_file, src_line);
}

extern SERVICE_TYPE(mysql_mutex_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_mutex_v1);

SERVICE_TYPE(mysql_mutex_v1)
SERVICE_IMPLEMENTATION(mysql_server, mysql_mutex_v1) = {
    impl_mysql_mutex_register, impl_mysql_mutex_init,
    impl_mysql_mutex_destroy,  impl_mysql_mutex_lock,
    impl_mysql_mutex_trylock,  impl_mysql_mutex_unlock};
