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

#ifndef MYSQL_RWLOCK_H
#define MYSQL_RWLOCK_H

/**
  @file include/mysql/psi/mysql_rwlock.h
  Instrumentation helpers for rwlock.
*/

#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/psi/psi_rwlock.h"
#include "thr_rwlock.h"
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#include "pfs_rwlock_provider.h"
#endif
#endif

#ifndef PSI_RWLOCK_CALL
#define PSI_RWLOCK_CALL(M) psi_rwlock_service->M
#endif

/**
  @defgroup psi_api_rwlock Rwlock Instrumentation (API)
  @ingroup psi_api
  @{
*/

/**
  @def mysql_prlock_assert_write_owner(M)
  Drop-in replacement
  for @c rw_pr_lock_assert_write_owner.
*/
#ifdef SAFE_MUTEX
#define mysql_prlock_assert_write_owner(M) \
  rw_pr_lock_assert_write_owner(&(M)->m_prlock)
#else
#define mysql_prlock_assert_write_owner(M) \
  {}
#endif

/**
  @def mysql_prlock_assert_not_write_owner(M)
  Drop-in replacement
  for @c rw_pr_lock_assert_not_write_owner.
*/
#ifdef SAFE_MUTEX
#define mysql_prlock_assert_not_write_owner(M) \
  rw_pr_lock_assert_not_write_owner(&(M)->m_prlock)
#else
#define mysql_prlock_assert_not_write_owner(M) \
  {}
#endif

#ifndef DISABLE_MYSQL_THREAD_H

/**
  @def mysql_rwlock_register(P1, P2, P3)
  Rwlock registration.
*/
#define mysql_rwlock_register(P1, P2, P3) \
  inline_mysql_rwlock_register(P1, P2, P3)

/**
  @def mysql_rwlock_init(K, T)
  Instrumented rwlock_init.
  @c mysql_rwlock_init is a replacement for @c pthread_rwlock_init.
  Note that pthread_rwlockattr_t is not supported in MySQL.
  @param K The PSI_rwlock_key for this instrumented rwlock
  @param T The rwlock to initialize
*/

#define mysql_rwlock_init(K, T) \
  mysql_rwlock_init_with_src(K, T, __FILE__, __LINE__)

#define mysql_rwlock_init_with_src(K, T, F, L) \
  inline_mysql_rwlock_init(K, T, F, L)

/**
  @def mysql_prlock_init(K, T)
  Instrumented rw_pr_init.
  @c mysql_prlock_init is a replacement for @c rw_pr_init.
  @param K The PSI_rwlock_key for this instrumented prlock
  @param T The prlock to initialize
*/

#define mysql_prlock_init(K, T) \
  mysql_prlock_init_with_src(K, T, __FILE__, __LINE__)

#define mysql_prlock_init_with_src(K, T, F, L) \
  inline_mysql_prlock_init(K, T, F, L)

/**
  @def mysql_rwlock_destroy(T)
  Instrumented rwlock_destroy.
  @c mysql_rwlock_destroy is a drop-in replacement
  for @c pthread_rwlock_destroy.
*/

#define mysql_rwlock_destroy(T) \
  mysql_rwlock_destroy_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_destroy_with_src(T, F, L) \
  inline_mysql_rwlock_destroy(T, F, L)

/**
  @def mysql_prlock_destroy(T)
  Instrumented rw_pr_destroy.
  @c mysql_prlock_destroy is a drop-in replacement
  for @c rw_pr_destroy.
*/
#define mysql_prlock_destroy(T) \
  mysql_prlock_destroy_with_src(T, __FILE__, __LINE__)

#define mysql_prlock_destroy_with_src(T, F, L) \
  inline_mysql_prlock_destroy(T, F, L)

/**
  @def mysql_rwlock_rdlock(T)
  Instrumented rwlock_rdlock.
  @c mysql_rwlock_rdlock is a drop-in replacement
  for @c pthread_rwlock_rdlock.
*/

#define mysql_rwlock_rdlock(T) \
  mysql_rwlock_rdlock_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_rdlock_with_src(T, F, L) \
  inline_mysql_rwlock_rdlock(T, F, L)

/**
  @def mysql_prlock_rdlock(T)
  Instrumented rw_pr_rdlock.
  @c mysql_prlock_rdlock is a drop-in replacement
  for @c rw_pr_rdlock.
*/

#define mysql_prlock_rdlock(T) \
  mysql_prlock_rdlock_with_src(T, __FILE__, __LINE__)

#define mysql_prlock_rdlock_with_src(T, F, L) \
  inline_mysql_prlock_rdlock(T, F, L)

/**
  @def mysql_rwlock_wrlock(T)
  Instrumented rwlock_wrlock.
  @c mysql_rwlock_wrlock is a drop-in replacement
  for @c pthread_rwlock_wrlock.
*/

#define mysql_rwlock_wrlock(T) \
  mysql_rwlock_wrlock_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_wrlock_with_src(T, F, L) \
  inline_mysql_rwlock_wrlock(T, F, L)

/**
  @def mysql_prlock_wrlock(T)
  Instrumented rw_pr_wrlock.
  @c mysql_prlock_wrlock is a drop-in replacement
  for @c rw_pr_wrlock.
*/

#define mysql_prlock_wrlock(T) \
  mysql_prlock_wrlock_with_src(T, __FILE__, __LINE__)

#define mysql_prlock_wrlock_with_src(T, F, L) \
  inline_mysql_prlock_wrlock(T, F, L)

/**
  @def mysql_rwlock_tryrdlock(T)
  Instrumented rwlock_tryrdlock.
  @c mysql_rwlock_tryrdlock is a drop-in replacement
  for @c pthread_rwlock_tryrdlock.
*/

#define mysql_rwlock_tryrdlock(T) \
  mysql_rwlock_tryrdlock_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_tryrdlock_with_src(T, F, L) \
  inline_mysql_rwlock_tryrdlock(T, F, L)

/**
  @def mysql_rwlock_trywrlock(T)
  Instrumented rwlock_trywrlock.
  @c mysql_rwlock_trywrlock is a drop-in replacement
  for @c pthread_rwlock_trywrlock.
*/

#define mysql_rwlock_trywrlock(T) \
  mysql_rwlock_trywrlock_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_trywrlock_with_src(T, F, L) \
  inline_mysql_rwlock_trywrlock(T, F, L)

/**
  @def mysql_rwlock_unlock(T)
  Instrumented rwlock_unlock.
  @c mysql_rwlock_unlock is a drop-in replacement
  for @c pthread_rwlock_unlock.
*/
#define mysql_rwlock_unlock(T) \
  mysql_rwlock_unlock_with_src(T, __FILE__, __LINE__)

#define mysql_rwlock_unlock_with_src(T, F, L) \
  inline_mysql_rwlock_unlock(T, F, L)

/**
  @def mysql_prlock_unlock(T)
  Instrumented rw_pr_unlock.
  @c mysql_prlock_unlock is a drop-in replacement
  for @c rw_pr_unlock.
*/

#define mysql_prlock_unlock(T) \
  mysql_prlock_unlock_with_src(T, __FILE__, __LINE__)

#define mysql_prlock_unlock_with_src(T, F, L) \
  inline_mysql_prlock_unlock(T, F, L)

static inline void inline_mysql_rwlock_register(
    const char *category MY_ATTRIBUTE((unused)),
    PSI_rwlock_info *info MY_ATTRIBUTE((unused)),
    int count MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  PSI_RWLOCK_CALL(register_rwlock)(category, info, count);
#endif
}

static inline int inline_mysql_rwlock_init(
    PSI_rwlock_key key MY_ATTRIBUTE((unused)), mysql_rwlock_t *that,
    const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  that->m_psi = PSI_RWLOCK_CALL(init_rwlock)(key, &that->m_rwlock);
#else
  that->m_psi = nullptr;
#endif
  return native_rw_init(&that->m_rwlock);
}

#ifndef DISABLE_MYSQL_PRLOCK_H
static inline int inline_mysql_prlock_init(
    PSI_rwlock_key key MY_ATTRIBUTE((unused)), mysql_prlock_t *that,
    const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  that->m_psi = PSI_RWLOCK_CALL(init_rwlock)(key, &that->m_prlock);
#else
  that->m_psi = nullptr;
#endif
  return rw_pr_init(&that->m_prlock);
}
#endif

static inline int inline_mysql_rwlock_destroy(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_RWLOCK_CALL(destroy_rwlock)(that->m_psi);
    that->m_psi = nullptr;
  }
#endif
  return native_rw_destroy(&that->m_rwlock);
}

#ifndef DISABLE_MYSQL_PRLOCK_H
static inline int inline_mysql_prlock_destroy(
    mysql_prlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_RWLOCK_CALL(destroy_rwlock)(that->m_psi);
    that->m_psi = nullptr;
  }
#endif
  return rw_pr_destroy(&that->m_prlock);
}
#endif

static inline int inline_mysql_rwlock_rdlock(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_rdwait)(
        &state, that->m_psi, PSI_RWLOCK_READLOCK, src_file, src_line);

    /* Instrumented code */
    result = native_rw_rdlock(&that->m_rwlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_rdwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = native_rw_rdlock(&that->m_rwlock);

  return result;
}

#ifndef DISABLE_MYSQL_PRLOCK_H
static inline int inline_mysql_prlock_rdlock(
    mysql_prlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_rdwait)(
        &state, that->m_psi, PSI_RWLOCK_READLOCK, src_file, src_line);

    /* Instrumented code */
    result = rw_pr_rdlock(&that->m_prlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_rdwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = rw_pr_rdlock(&that->m_prlock);

  return result;
}
#endif

static inline int inline_mysql_rwlock_wrlock(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_wrwait)(
        &state, that->m_psi, PSI_RWLOCK_WRITELOCK, src_file, src_line);

    /* Instrumented code */
    result = native_rw_wrlock(&that->m_rwlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_wrwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = native_rw_wrlock(&that->m_rwlock);

  return result;
}

#ifndef DISABLE_MYSQL_PRLOCK_H
static inline int inline_mysql_prlock_wrlock(
    mysql_prlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_wrwait)(
        &state, that->m_psi, PSI_RWLOCK_WRITELOCK, src_file, src_line);

    /* Instrumented code */
    result = rw_pr_wrlock(&that->m_prlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_wrwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = rw_pr_wrlock(&that->m_prlock);

  return result;
}
#endif

static inline int inline_mysql_rwlock_tryrdlock(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_rdwait)(
        &state, that->m_psi, PSI_RWLOCK_TRYREADLOCK, src_file, src_line);

    /* Instrumented code */
    result = native_rw_tryrdlock(&that->m_rwlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_rdwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = native_rw_tryrdlock(&that->m_rwlock);

  return result;
}

static inline int inline_mysql_rwlock_trywrlock(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_rwlock_locker *locker;
    PSI_rwlock_locker_state state;
    locker = PSI_RWLOCK_CALL(start_rwlock_wrwait)(
        &state, that->m_psi, PSI_RWLOCK_TRYWRITELOCK, src_file, src_line);

    /* Instrumented code */
    result = native_rw_trywrlock(&that->m_rwlock);

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_RWLOCK_CALL(end_rwlock_wrwait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = native_rw_trywrlock(&that->m_rwlock);

  return result;
}

static inline int inline_mysql_rwlock_unlock(
    mysql_rwlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_RWLOCK_CALL(unlock_rwlock)(that->m_psi, PSI_RWLOCK_UNLOCK);
  }
#endif
  result = native_rw_unlock(&that->m_rwlock);
  return result;
}

#ifndef DISABLE_MYSQL_PRLOCK_H
static inline int inline_mysql_prlock_unlock(
    mysql_prlock_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    int src_line MY_ATTRIBUTE((unused))) {
  int result;
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_RWLOCK_CALL(unlock_rwlock)(that->m_psi, PSI_RWLOCK_UNLOCK);
  }
#endif
  result = rw_pr_unlock(&that->m_prlock);
  return result;
}
#endif

#endif /* DISABLE_MYSQL_THREAD_H */

/** @} (end of group psi_api_rwlock) */

#endif
