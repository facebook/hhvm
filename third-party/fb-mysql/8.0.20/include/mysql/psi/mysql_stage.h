/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_STAGE_H
#define MYSQL_STAGE_H

/**
  @file include/mysql/psi/mysql_stage.h
  Instrumentation helpers for stages.
*/

#include "mysql/psi/psi_stage.h"

#include "my_inttypes.h"
#include "pfs_stage_provider.h"

#ifndef PSI_STAGE_CALL
#define PSI_STAGE_CALL(M) psi_stage_service->M
#endif

/**
  @defgroup psi_api_stage Stage Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_stage_register(P1, P2, P3)
  Stage registration.
*/
#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_stage_register(P1, P2, P3) inline_mysql_stage_register(P1, P2, P3)
#else
#define mysql_stage_register(P1, P2, P3) \
  do {                                   \
  } while (0)
#endif

/**
  @def MYSQL_SET_STAGE
  Set the current stage.
  Use this API when the file and line
  is passed from the caller.
  @param K the stage key
  @param F the source file name
  @param L the source file line
  @return the current stage progress
*/
#ifdef HAVE_PSI_STAGE_INTERFACE
#define MYSQL_SET_STAGE(K, F, L) inline_mysql_set_stage(K, F, L)
#else
#define MYSQL_SET_STAGE(K, F, L) NULL
#endif

/**
  @def mysql_set_stage
  Set the current stage.
  @param K the stage key
  @return the current stage progress
*/
#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_set_stage(K) inline_mysql_set_stage(K, __FILE__, __LINE__)
#else
#define mysql_set_stage(K) NULL
#endif

/**
  @def mysql_end_stage
  End the last stage
*/
#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_end_stage inline_mysql_end_stage
#else
#define mysql_end_stage() \
  do {                    \
  } while (0)
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline void inline_mysql_stage_register(const char *category,
                                               PSI_stage_info **info,
                                               int count) {
  PSI_STAGE_CALL(register_stage)(category, info, count);
}
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline PSI_stage_progress *inline_mysql_set_stage(PSI_stage_key key,
                                                         const char *src_file,
                                                         int src_line) {
  return PSI_STAGE_CALL(start_stage)(key, src_file, src_line);
}
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline void inline_mysql_end_stage() { PSI_STAGE_CALL(end_stage)(); }
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_stage_set_work_completed(P1, P2) \
  inline_mysql_stage_set_work_completed(P1, P2)

#define mysql_stage_get_work_completed(P1) \
  inline_mysql_stage_get_work_completed(P1)
#else
#define mysql_stage_set_work_completed(P1, P2) \
  do {                                         \
  } while (0)

#define mysql_stage_get_work_completed(P1) \
  do {                                     \
  } while (0)
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_stage_inc_work_completed(P1, P2) \
  inline_mysql_stage_inc_work_completed(P1, P2)
#else
#define mysql_stage_inc_work_completed(P1, P2) \
  do {                                         \
  } while (0)
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
#define mysql_stage_set_work_estimated(P1, P2) \
  inline_mysql_stage_set_work_estimated(P1, P2)

#define mysql_stage_get_work_estimated(P1) \
  inline_mysql_stage_get_work_estimated(P1)
#else
#define mysql_stage_set_work_estimated(P1, P2) \
  do {                                         \
  } while (0)

#define mysql_stage_get_work_estimated(P1) \
  do {                                     \
  } while (0)
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline void inline_mysql_stage_set_work_completed(
    PSI_stage_progress *progress, ulonglong val) {
  if (progress != nullptr) {
    progress->m_work_completed = val;
  }
}

static inline ulonglong inline_mysql_stage_get_work_completed(
    PSI_stage_progress *progress) {
  return progress->m_work_completed;
}
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline void inline_mysql_stage_inc_work_completed(
    PSI_stage_progress *progress, ulonglong val) {
  if (progress != nullptr) {
    progress->m_work_completed += val;
  }
}
#endif

#ifdef HAVE_PSI_STAGE_INTERFACE
static inline void inline_mysql_stage_set_work_estimated(
    PSI_stage_progress *progress, ulonglong val) {
  if (progress != nullptr) {
    progress->m_work_estimated = val;
  }
}

static inline ulonglong inline_mysql_stage_get_work_estimated(
    PSI_stage_progress *progress) {
  return progress->m_work_estimated;
}
#endif

/** @} (end of group psi_api_stage) */

#endif
