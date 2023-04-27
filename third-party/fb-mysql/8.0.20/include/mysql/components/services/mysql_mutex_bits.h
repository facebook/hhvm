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

#ifndef COMPONENTS_SERVICES_MYSQL_MUTEX_BITS_H
#define COMPONENTS_SERVICES_MYSQL_MUTEX_BITS_H

/**
  @file
  ABI for instrumented mutexes.
*/

#include <mysql/components/services/thr_mutex_bits.h>

/**
  @defgroup psi_api_mutex Mutex Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  An instrumented mutex structure.
  @c mysql_mutex_t is a drop-in replacement for @c my_mutex_t.
  @sa mysql_mutex_assert_owner
  @sa mysql_mutex_assert_not_owner
  @sa mysql_mutex_init
  @sa mysql_mutex_lock
  @sa mysql_mutex_unlock
  @sa mysql_mutex_destroy
*/
struct mysql_mutex_t {
  /** The real mutex. */
  my_mutex_t m_mutex;
  /**
    The instrumentation hook.
    Note that this hook is not conditionally defined,
    for binary compatibility of the @c mysql_mutex_t interface.
  */
  struct PSI_mutex *m_psi{nullptr};
};

/** @} (end of group psi_api_mutex) */

#endif /* COMPONENTS_SERVICES_MYSQL_MUTEX_BITS_H */
