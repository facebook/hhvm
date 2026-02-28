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

#ifndef COMPONENTS_SERVICES_MYSQL_RWLOCK_SERVICE_H
#define COMPONENTS_SERVICES_MYSQL_RWLOCK_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/mysql_rwlock_bits.h>
#include <mysql/components/services/psi_rwlock_bits.h>

typedef void (*mysql_rwlock_register_t)(const char *category,
                                        PSI_rwlock_info *info, int count);

typedef int (*mysql_rwlock_init_t)(PSI_rwlock_key key, mysql_rwlock_t *that,
                                   const char *src_file, unsigned int src_line);

typedef int (*mysql_prlock_init_t)(PSI_rwlock_key key, mysql_prlock_t *that,
                                   const char *src_file, unsigned int src_line);

typedef int (*mysql_rwlock_destroy_t)(mysql_rwlock_t *that,
                                      const char *src_file,
                                      unsigned int src_line);

typedef int (*mysql_prlock_destroy_t)(mysql_prlock_t *that,
                                      const char *src_file,
                                      unsigned int src_line);

typedef int (*mysql_rwlock_rdlock_t)(mysql_rwlock_t *that, const char *src_file,
                                     unsigned int src_line);

typedef int (*mysql_prlock_rdlock_t)(mysql_prlock_t *that, const char *src_file,
                                     unsigned int src_line);

typedef int (*mysql_rwlock_wrlock_t)(mysql_rwlock_t *that, const char *src_file,
                                     unsigned int src_line);

typedef int (*mysql_prlock_wrlock_t)(mysql_prlock_t *that, const char *src_file,
                                     unsigned int src_line);

typedef int (*mysql_rwlock_tryrdlock_t)(mysql_rwlock_t *that,
                                        const char *src_file,
                                        unsigned int src_line);

/* No mysql_prlock_tryrdlock_t */

typedef int (*mysql_rwlock_trywrlock_t)(mysql_rwlock_t *that,
                                        const char *src_file,
                                        unsigned int src_line);

/* No mysql_prlock_trywrlock_t */

typedef int (*mysql_rwlock_unlock_t)(mysql_rwlock_t *that, const char *src_file,
                                     unsigned int src_line);

typedef int (*mysql_prlock_unlock_t)(mysql_prlock_t *that, const char *src_file,
                                     unsigned int src_line);

BEGIN_SERVICE_DEFINITION(mysql_rwlock_v1)
mysql_rwlock_register_t register_info;
mysql_rwlock_init_t rwlock_init;
mysql_prlock_init_t prlock_init;
mysql_rwlock_destroy_t rwlock_destroy;
mysql_prlock_destroy_t prlock_destroy;
mysql_rwlock_rdlock_t rwlock_rdlock;
mysql_prlock_rdlock_t prlock_rdlock;
mysql_rwlock_wrlock_t rwlock_wrlock;
mysql_prlock_wrlock_t prlock_wrlock;
mysql_rwlock_tryrdlock_t rwlock_tryrdlock;
mysql_rwlock_trywrlock_t rwlock_trywrlock;
mysql_rwlock_unlock_t rwlock_unlock;
mysql_prlock_unlock_t prlock_unlock;
END_SERVICE_DEFINITION(mysql_rwlock_v1)

#define REQUIRES_MYSQL_RWLOCK_SERVICE REQUIRES_SERVICE(mysql_rwlock_v1)

#endif /* COMPONENTS_SERVICES_MYSQL_RWLOCK_SERVICE_H */
