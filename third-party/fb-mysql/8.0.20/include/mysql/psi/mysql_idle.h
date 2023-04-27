/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_IDLE_H
#define MYSQL_IDLE_H

/**
  @file include/mysql/psi/mysql_idle.h
  Instrumentation helpers for idle waits.
*/

#include "mysql/psi/psi_idle.h"

#include "pfs_idle_provider.h"

#ifndef PSI_IDLE_CALL
#define PSI_IDLE_CALL(M) psi_idle_service->M
#endif

/**
  @defgroup psi_api_idle Idle Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def MYSQL_START_IDLE_WAIT
  Instrumentation helper for table io_waits.
  This instrumentation marks the start of a wait event.
  @param LOCKER the locker
  @param STATE the locker state
  @sa MYSQL_END_IDLE_WAIT.
*/
#ifdef HAVE_PSI_IDLE_INTERFACE
#define MYSQL_START_IDLE_WAIT(LOCKER, STATE) \
  LOCKER = inline_mysql_start_idle_wait(STATE, __FILE__, __LINE__)
#else
#define MYSQL_START_IDLE_WAIT(LOCKER, STATE) \
  do {                                       \
  } while (0)
#endif

/**
  @def MYSQL_END_IDLE_WAIT
  Instrumentation helper for idle waits.
  This instrumentation marks the end of a wait event.
  @param LOCKER the locker
  @sa MYSQL_START_IDLE_WAIT.
*/
#ifdef HAVE_PSI_IDLE_INTERFACE
#define MYSQL_END_IDLE_WAIT(LOCKER) inline_mysql_end_idle_wait(LOCKER)
#else
#define MYSQL_END_IDLE_WAIT(LOCKER) \
  do {                              \
  } while (0)
#endif

#ifdef HAVE_PSI_IDLE_INTERFACE
/**
  Instrumentation calls for MYSQL_START_IDLE_WAIT.
  @sa MYSQL_END_IDLE_WAIT.
*/
static inline struct PSI_idle_locker *inline_mysql_start_idle_wait(
    PSI_idle_locker_state *state, const char *src_file, int src_line) {
  struct PSI_idle_locker *locker;
  locker = PSI_IDLE_CALL(start_idle_wait)(state, src_file, src_line);
  return locker;
}

/**
  Instrumentation calls for MYSQL_END_IDLE_WAIT.
  @sa MYSQL_START_IDLE_WAIT.
*/
static inline void inline_mysql_end_idle_wait(struct PSI_idle_locker *locker) {
  if (likely(locker != nullptr)) {
    PSI_IDLE_CALL(end_idle_wait)(locker);
  }
}
#endif

/** @} (end of group psi_api_idle) */

#endif
