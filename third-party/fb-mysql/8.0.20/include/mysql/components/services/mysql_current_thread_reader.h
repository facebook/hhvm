/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_CURRENT_THREAD_READER_H
#define MYSQL_CURRENT_THREAD_READER_H

#include <mysql/components/service.h>

#ifdef __cplusplus
class THD;
#define MYSQL_THD THD *
#else
#define MYSQL_THD void *
#endif

/**
  @ingroup group_components_services_inventory

  A service to fetch the current thread id

  Use in conjuntion with all the related services that operate on thread ids
  @sa mysql_component_mysql_current_thread_reader_imp
*/
BEGIN_SERVICE_DEFINITION(mysql_current_thread_reader)
/**
  get the current THD.

  @param[out] thread id
  @return Status of performed operation
  @retval false success (valid password)
  @retval true failure (invalid password)
*/
DECLARE_BOOL_METHOD(get, (MYSQL_THD * thd));

END_SERVICE_DEFINITION(mysql_current_thread_reader)

#endif /* MYSQL_CURRENT_THREAD_READER_H */
