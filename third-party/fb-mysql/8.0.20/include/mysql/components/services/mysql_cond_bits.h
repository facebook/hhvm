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

#ifndef COMPONENTS_SERVICES_MYSQL_COND_BITS_H
#define COMPONENTS_SERVICES_MYSQL_COND_BITS_H

/**
  @file
  Instrumentation helpers for conditions.
*/

#include "mysql/components/services/thr_cond_bits.h"

/**
  @defgroup psi_api_cond Cond Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  An instrumented cond structure.
  @c mysql_cond_t is a drop-in replacement for @c native_cond_t.
  @sa mysql_cond_init
  @sa mysql_cond_wait
  @sa mysql_cond_timedwait
  @sa mysql_cond_signal
  @sa mysql_cond_broadcast
  @sa mysql_cond_destroy
*/
struct mysql_cond_t {
  /** The real condition */
  native_cond_t m_cond;
  /**
    The instrumentation hook.
    Note that this hook is not conditionally defined,
    for binary compatibility of the @c mysql_cond_t interface.
  */
  struct PSI_cond *m_psi;
};

/** @} (end of group psi_api_cond) */

#endif /* COMPONENTS_SERVICES_MYSQL_COND_BITS_H */
