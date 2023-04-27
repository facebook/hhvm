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
#include <mysql/components/services/mysql_rwlock_service.h>
#include <mysql/psi/mysql_rwlock.h>

void impl_mysql_rwlock_register(const char *category, PSI_rwlock_info *info,
                                int count) {
  mysql_rwlock_register(category, info, count);
}

int impl_mysql_rwlock_init(PSI_rwlock_key key, mysql_rwlock_t *that,
                           const char *src_file, unsigned int src_line) {
  return mysql_rwlock_init_with_src(key, that, src_file, src_line);
}

int impl_mysql_prlock_init(PSI_rwlock_key key, mysql_prlock_t *that,
                           const char *src_file, unsigned int src_line) {
  return mysql_prlock_init_with_src(key, that, src_file, src_line);
}

int impl_mysql_rwlock_destroy(mysql_rwlock_t *that, const char *src_file,
                              unsigned int src_line) {
  return mysql_rwlock_destroy_with_src(that, src_file, src_line);
}

int impl_mysql_prlock_destroy(mysql_prlock_t *that, const char *src_file,
                              unsigned int src_line) {
  return mysql_prlock_destroy_with_src(that, src_file, src_line);
}

int impl_mysql_rwlock_rdlock(mysql_rwlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_rwlock_rdlock_with_src(that, src_file, src_line);
}

int impl_mysql_prlock_rdlock(mysql_prlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_prlock_rdlock_with_src(that, src_file, src_line);
}

int impl_mysql_rwlock_wrlock(mysql_rwlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_rwlock_wrlock_with_src(that, src_file, src_line);
}

int impl_mysql_prlock_wrlock(mysql_prlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_prlock_wrlock_with_src(that, src_file, src_line);
}

int impl_mysql_rwlock_tryrdlock(mysql_rwlock_t *that, const char *src_file,
                                unsigned int src_line) {
  return mysql_rwlock_tryrdlock_with_src(that, src_file, src_line);
}

int impl_mysql_rwlock_trywrlock(mysql_rwlock_t *that, const char *src_file,
                                unsigned int src_line) {
  return mysql_rwlock_trywrlock_with_src(that, src_file, src_line);
}

int impl_mysql_rwlock_unlock(mysql_rwlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_rwlock_unlock_with_src(that, src_file, src_line);
}

int impl_mysql_prlock_unlock(mysql_prlock_t *that, const char *src_file,
                             unsigned int src_line) {
  return mysql_prlock_unlock_with_src(that, src_file, src_line);
}

extern SERVICE_TYPE(mysql_rwlock_v1)
    SERVICE_IMPLEMENTATION(mysql_server, mysql_rwlock_v1);

SERVICE_TYPE(mysql_rwlock_v1)
SERVICE_IMPLEMENTATION(mysql_server, mysql_rwlock_v1) = {
    impl_mysql_rwlock_register,  impl_mysql_rwlock_init,
    impl_mysql_prlock_init,      impl_mysql_rwlock_destroy,
    impl_mysql_prlock_destroy,   impl_mysql_rwlock_rdlock,
    impl_mysql_prlock_rdlock,    impl_mysql_rwlock_wrlock,
    impl_mysql_prlock_wrlock,    impl_mysql_rwlock_tryrdlock,
    impl_mysql_rwlock_trywrlock, impl_mysql_rwlock_unlock,
    impl_mysql_prlock_unlock};
