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

#ifndef MYSQL_MUTEX_H
#define MYSQL_MUTEX_H

/**
  @file include/mysql/psi/mysql_mutex.h
  Instrumentation helpers for mutexes.
  This header file provides the necessary declarations
  to use the mutex API with the performance schema instrumentation.
  In some compilers (SunStudio), 'static inline' functions, when declared
  but not used, are not optimized away (because they are unused) by default,
  so that including a static inline function from a header file does
  create unwanted dependencies, causing unresolved symbols at link time.
  Other compilers, like gcc, optimize these dependencies by default.
*/
/*
  Note: there are several orthogonal dimensions here.

  Dimension 1: Instrumentation
  HAVE_PSI_MUTEX_INTERFACE is defined when the instrumentation is compiled in.
  This may happen both in debug or production builds.

  Dimension 2: Debug
  SAFE_MUTEX is defined when debug is compiled in.
  This may happen both with and without instrumentation.

  Dimension 3: Platform
  Mutexes are implemented with one of:
  - the pthread library
  - fast mutexes
  - window apis
  This is implemented by various macro definitions in my_thread.h

  This causes complexity with '#ifdef'-ery that can't be avoided.
*/

#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/psi_mutex.h"
#include "thr_mutex.h"

#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#include "pfs_mutex_provider.h"
#endif
#endif

#ifndef PSI_MUTEX_CALL
#define PSI_MUTEX_CALL(M) psi_mutex_service->M
#endif

/**
  @defgroup psi_api_mutex Mutex Instrumentation (API)
  @ingroup psi_api
  @{
*/

/*
  Consider the following code:
    static inline void foo() { bar(); }
  when foo() is never called.

  With gcc, foo() is a local static function, so the dependencies
  are optimized away at compile time, and there is no dependency on bar().
  With other compilers (HP, Sun Studio), the function foo() implementation
  is compiled, and bar() needs to be present to link.

  Due to the existing header dependencies in MySQL code, this header file
  is sometime used when it is not needed, which in turn cause link failures
  on some platforms.
  The proper fix would be to cut these extra dependencies in the calling code.
  DISABLE_MYSQL_THREAD_H is a work around to limit dependencies.
  DISABLE_MYSQL_PRLOCK_H is similar, and is used to disable specifically
  the prlock wrappers.
*/
#ifndef DISABLE_MYSQL_THREAD_H

/**
  @def mysql_mutex_assert_owner(M)
  Wrapper, to use safe_mutex_assert_owner with instrumented mutexes.
  @c mysql_mutex_assert_owner is a drop-in replacement
  for @c safe_mutex_assert_owner.
*/
#ifdef SAFE_MUTEX
#define mysql_mutex_assert_owner(M) \
  safe_mutex_assert_owner((M)->m_mutex.m_u.m_safe_ptr);
#else
#define mysql_mutex_assert_owner(M) \
  {}
#endif

/**
  @def mysql_mutex_assert_not_owner(M)
  Wrapper, to use safe_mutex_assert_not_owner with instrumented mutexes.
  @c mysql_mutex_assert_not_owner is a drop-in replacement
  for @c safe_mutex_assert_not_owner.
*/
#ifdef SAFE_MUTEX
#define mysql_mutex_assert_not_owner(M) \
  safe_mutex_assert_not_owner((M)->m_mutex.m_u.m_safe_ptr);
#else
#define mysql_mutex_assert_not_owner(M) \
  {}
#endif

/**
  @def mysql_mutex_register(P1, P2, P3)
  Mutex registration.
*/
#define mysql_mutex_register(P1, P2, P3) inline_mysql_mutex_register(P1, P2, P3)

/**
  @def mysql_mutex_init(K, M, A)
  Instrumented mutex_init.
  @c mysql_mutex_init is a replacement for @c pthread_mutex_init.
  @param K The PSI_mutex_key for this instrumented mutex
  @param M The mutex to initialize
  @param A Mutex attributes
*/

#define mysql_mutex_init(K, M, A) \
  mysql_mutex_init_with_src(K, M, A, __FILE__, __LINE__)

#define mysql_mutex_init_with_src(K, M, A, F, L) \
  inline_mysql_mutex_init(K, M, A, F, L)

/**
  @def mysql_mutex_destroy(M)
  Instrumented mutex_destroy.
  @c mysql_mutex_destroy is a drop-in replacement
  for @c pthread_mutex_destroy.
*/
#define mysql_mutex_destroy(M) \
  mysql_mutex_destroy_with_src(M, __FILE__, __LINE__)

#define mysql_mutex_destroy_with_src(M, F, L) \
  inline_mysql_mutex_destroy(M, F, L)

/**
  @def mysql_mutex_lock(M)
  Instrumented mutex_lock.
  @c mysql_mutex_lock is a drop-in replacement for @c pthread_mutex_lock.
  @param M The mutex to lock
*/

#define mysql_mutex_lock(M) mysql_mutex_lock_with_src(M, __FILE__, __LINE__)

#define mysql_mutex_lock_with_src(M, F, L) inline_mysql_mutex_lock(M, F, L)

/**
  @def mysql_mutex_trylock(M)
  Instrumented mutex_lock.
  @c mysql_mutex_trylock is a drop-in replacement
  for @c my_mutex_trylock.
*/

#define mysql_mutex_trylock(M) \
  mysql_mutex_trylock_with_src(M, __FILE__, __LINE__)

#define mysql_mutex_trylock_with_src(M, F, L) \
  inline_mysql_mutex_trylock(M, F, L)

/**
  @def mysql_mutex_unlock(M)
  Instrumented mutex_unlock.
  @c mysql_mutex_unlock is a drop-in replacement for @c pthread_mutex_unlock.
*/
#define mysql_mutex_unlock(M) mysql_mutex_unlock_with_src(M, __FILE__, __LINE__)

#define mysql_mutex_unlock_with_src(M, F, L) inline_mysql_mutex_unlock(M, F, L)

static inline void inline_mysql_mutex_register(
    const char *category MY_ATTRIBUTE((unused)),
    PSI_mutex_info *info MY_ATTRIBUTE((unused)),
    int count MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_MUTEX_INTERFACE
  PSI_MUTEX_CALL(register_mutex)(category, info, count);
#endif
}

static inline int inline_mysql_mutex_init(
    PSI_mutex_key key MY_ATTRIBUTE((unused)), mysql_mutex_t *that,
    const native_mutexattr_t *attr, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_MUTEX_INTERFACE
  that->m_psi = PSI_MUTEX_CALL(init_mutex)(key, &that->m_mutex);
#else
  that->m_psi = nullptr;
#endif
  return my_mutex_init(&that->m_mutex, attr
#ifdef SAFE_MUTEX
                       ,
                       src_file, src_line
#endif
  );
}

static inline int inline_mysql_mutex_destroy(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_MUTEX_CALL(destroy_mutex)(that->m_psi);
    that->m_psi = nullptr;
  }
#endif
  return my_mutex_destroy(&that->m_mutex
#ifdef SAFE_MUTEX
                          ,
                          src_file, src_line
#endif
  );
}

static inline int inline_mysql_mutex_lock(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_mutex_locker *locker;
    PSI_mutex_locker_state state;
    locker = PSI_MUTEX_CALL(start_mutex_wait)(
        &state, that->m_psi, PSI_MUTEX_LOCK, src_file, src_line);

    /* Instrumented code */
    result = my_mutex_lock(&that->m_mutex
#ifdef SAFE_MUTEX
                           ,
                           src_file, src_line
#endif
    );

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_MUTEX_CALL(end_mutex_wait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = my_mutex_lock(&that->m_mutex
#ifdef SAFE_MUTEX
                         ,
                         src_file, src_line
#endif
  );

  return result;
}

static inline int inline_mysql_mutex_trylock(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    /* Instrumentation start */
    PSI_mutex_locker *locker;
    PSI_mutex_locker_state state;
    locker = PSI_MUTEX_CALL(start_mutex_wait)(
        &state, that->m_psi, PSI_MUTEX_TRYLOCK, src_file, src_line);

    /* Instrumented code */
    result = my_mutex_trylock(&that->m_mutex
#ifdef SAFE_MUTEX
                              ,
                              src_file, src_line
#endif
    );

    /* Instrumentation end */
    if (locker != nullptr) {
      PSI_MUTEX_CALL(end_mutex_wait)(locker, result);
    }

    return result;
  }
#endif

  /* Non instrumented code */
  result = my_mutex_trylock(&that->m_mutex
#ifdef SAFE_MUTEX
                            ,
                            src_file, src_line
#endif
  );

  return result;
}

static inline int inline_mysql_mutex_unlock(
    mysql_mutex_t *that, const char *src_file MY_ATTRIBUTE((unused)),
    uint src_line MY_ATTRIBUTE((unused))) {
  int result;

#ifdef HAVE_PSI_MUTEX_INTERFACE
  if (that->m_psi != nullptr) {
    PSI_MUTEX_CALL(unlock_mutex)(that->m_psi);
  }
#endif

  result = my_mutex_unlock(&that->m_mutex
#ifdef SAFE_MUTEX
                           ,
                           src_file, src_line
#endif
  );

  return result;
}

#endif /* DISABLE_MYSQL_THREAD_H */

/** @} (end of group psi_api_mutex) */

#endif
