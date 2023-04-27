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

#ifndef COMPONENTS_SERVICES_MYSQL_COND_SERVICE_H
#define COMPONENTS_SERVICES_MYSQL_COND_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/mysql_cond_bits.h>
#include <mysql/components/services/mysql_mutex_bits.h>
#include <mysql/components/services/psi_cond_bits.h>

typedef void (*mysql_cond_register_t)(const char *category, PSI_cond_info *info,
                                      int count);

typedef int (*mysql_cond_init_t)(PSI_cond_key key, mysql_cond_t *that,
                                 const char *src_file, unsigned int src_line);

typedef int (*mysql_cond_destroy_t)(mysql_cond_t *that, const char *src_file,
                                    unsigned int src_line);

typedef int (*mysql_cond_wait_t)(mysql_cond_t *that, mysql_mutex_t *mutex,
                                 const char *src_file, unsigned int src_line);

typedef int (*mysql_cond_timedwait_t)(mysql_cond_t *that, mysql_mutex_t *mutex,
                                      const struct timespec *abstime,
                                      const char *src_file,
                                      unsigned int src_line);

typedef int (*mysql_cond_signal_t)(mysql_cond_t *that, const char *src_file,
                                   unsigned int src_line);

typedef int (*mysql_cond_broadcast)(mysql_cond_t *that, const char *src_file,
                                    unsigned int src_line);

BEGIN_SERVICE_DEFINITION(mysql_cond_v1)
mysql_cond_register_t register_info;
mysql_cond_init_t init;
mysql_cond_destroy_t destroy;
mysql_cond_wait_t wait;
mysql_cond_timedwait_t timedwait;
mysql_cond_signal_t signal;
mysql_cond_broadcast broadcast;
END_SERVICE_DEFINITION(mysql_cond_v1)

#define REQUIRES_MYSQL_COND_SERVICE REQUIRES_SERVICE(mysql_cond_v1)

#endif /* COMPONENTS_SERVICES_MYSQL_COND_SERVICE_H */
