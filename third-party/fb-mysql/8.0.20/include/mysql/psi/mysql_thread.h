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

#ifndef MYSQL_THREAD_H
#define MYSQL_THREAD_H

/**
  @file include/mysql/psi/mysql_thread.h
  Instrumentation helpers for mysys threads.
  This header file provides the necessary declarations
  to use the mysys thread API with the performance schema instrumentation.
  In some compilers (SunStudio), 'static inline' functions, when declared
  but not used, are not optimized away (because they are unused) by default,
  so that including a static inline function from a header file does
  create unwanted dependencies, causing unresolved symbols at link time.
  Other compilers, like gcc, optimize these dependencies by default.

  Since the instrumented APIs declared here are wrapper on top
  of my_thread / safemutex / etc APIs,
  including mysql/psi/mysql_thread.h assumes that
  the dependency on my_thread and safemutex already exists.
*/

#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_thread.h"
#include "my_thread_local.h"
#include "mysql/psi/psi_thread.h"
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#include "pfs_thread_provider.h"
#endif
#endif

#ifndef PSI_THREAD_CALL
#define PSI_THREAD_CALL(M) psi_thread_service->M
#endif

/**
  @defgroup psi_api_thread Thread Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_thread_register(P1, P2, P3)
  Thread registration.
*/
#define mysql_thread_register(P1, P2, P3) \
  inline_mysql_thread_register(P1, P2, P3)

/**
  @def mysql_thread_create(K, P1, P2, P3, P4)
  Instrumented my_thread_create.
  This function creates both the thread instrumentation and a thread.
  @c mysql_thread_create is a replacement for @c my_thread_create.
  The parameter P4 (or, if it is NULL, P1) will be used as the
  instrumented thread "indentity".
  Providing a P1 / P4 parameter with a different value for each call
  will on average improve performances, since this thread identity value
  is used internally to randomize access to data and prevent contention.
  This is optional, and the improvement is not guaranteed, only statistical.
  @param K The PSI_thread_key for this instrumented thread
  @param P1 my_thread_create parameter 1
  @param P2 my_thread_create parameter 2
  @param P3 my_thread_create parameter 3
  @param P4 my_thread_create parameter 4
*/
#ifdef HAVE_PSI_THREAD_INTERFACE
#define mysql_thread_create(K, P1, P2, P3, P4) \
  inline_mysql_thread_create(K, P1, P2, P3, P4)
#else
#define mysql_thread_create(K, P1, P2, P3, P4) my_thread_create(P1, P2, P3, P4)
#endif

/**
  @def mysql_thread_set_psi_id(I)
  Set the thread identifier for the instrumentation.
  @param I The thread identifier
*/
#ifdef HAVE_PSI_THREAD_INTERFACE
#define mysql_thread_set_psi_id(I) inline_mysql_thread_set_psi_id(I)
#else
#define mysql_thread_set_psi_id(I) \
  do {                             \
  } while (0)
#endif

/**
  @def mysql_thread_set_psi_THD(T)
  Set the thread sql session for the instrumentation.
  @param T The thread sql session
*/
#ifdef HAVE_PSI_THREAD_INTERFACE
#define mysql_thread_set_psi_THD(T) inline_mysql_thread_set_psi_THD(T)
#else
#define mysql_thread_set_psi_THD(T) \
  do {                              \
  } while (0)
#endif

static inline void inline_mysql_thread_register(
#ifdef HAVE_PSI_THREAD_INTERFACE
    const char *category, PSI_thread_info *info, int count
#else
    const char *category MY_ATTRIBUTE((unused)),
    void *info MY_ATTRIBUTE((unused)), int count MY_ATTRIBUTE((unused))
#endif
) {
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(register_thread)(category, info, count);
#endif
}

#ifdef HAVE_PSI_THREAD_INTERFACE
static inline int inline_mysql_thread_create(PSI_thread_key key,
                                             my_thread_handle *thread,
                                             const my_thread_attr_t *attr,
                                             my_start_routine start_routine,
                                             void *arg) {
  int result;
  result = PSI_THREAD_CALL(spawn_thread)(key, thread, attr, start_routine, arg);
  return result;
}

static inline void inline_mysql_thread_set_psi_id(my_thread_id id) {
  struct PSI_thread *psi = PSI_THREAD_CALL(get_thread)();
  PSI_THREAD_CALL(set_thread_id)(psi, id);
}

#ifdef __cplusplus
class THD;
static inline void inline_mysql_thread_set_psi_THD(THD *thd) {
  struct PSI_thread *psi = PSI_THREAD_CALL(get_thread)();
  PSI_THREAD_CALL(set_thread_THD)(psi, thd);
}
#endif /* __cplusplus */

#endif

/** @} (end of group psi_api_thread) */

#endif
