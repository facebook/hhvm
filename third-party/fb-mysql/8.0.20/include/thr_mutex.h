#ifndef THR_MUTEX_INCLUDED
#define THR_MUTEX_INCLUDED

/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file include/thr_mutex.h
  MySQL mutex implementation.

  There are three "layers":
  1) native_mutex_*()
       Functions that map directly down to OS primitives.
       Windows    - CriticalSection
       Other OSes - pthread
  2) my_mutex_*()
       Functions that implement SAFE_MUTEX (default for debug),
       Otherwise native_mutex_*() is used.
  3) mysql_mutex_*()
       Functions that include Performance Schema instrumentation.
       See include/mysql/psi/mysql_thread.h
*/

#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_thread.h"

/*
  The following are part of the services ABI:
  - native_mutex_t
  - my_mutex_t
*/
#include "mysql/components/services/thr_mutex_bits.h"

/* Define mutex types, see my_thr_init.c */
#define MY_MUTEX_INIT_SLOW NULL

/* Can be set in /usr/include/pthread.h */
#ifdef PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP
extern native_mutexattr_t my_fast_mutexattr;
#define MY_MUTEX_INIT_FAST &my_fast_mutexattr
#else
#define MY_MUTEX_INIT_FAST NULL
#endif

/* Can be set in /usr/include/pthread.h */
#ifdef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
extern native_mutexattr_t my_errorcheck_mutexattr;
#define MY_MUTEX_INIT_ERRCHK &my_errorcheck_mutexattr
#else
#define MY_MUTEX_INIT_ERRCHK NULL
#endif

static inline int native_mutex_init(native_mutex_t *mutex,
                                    const native_mutexattr_t *attr
                                        MY_ATTRIBUTE((unused))) {
#ifdef _WIN32
  InitializeCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_init(mutex, attr);
#endif
}

static inline int native_mutex_lock(native_mutex_t *mutex) {
#ifdef _WIN32
  EnterCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_lock(mutex);
#endif
}

static inline int native_mutex_trylock(native_mutex_t *mutex) {
#ifdef _WIN32
  if (TryEnterCriticalSection(mutex)) {
    /* Don't allow recursive lock */
    if (mutex->RecursionCount > 1) {
      LeaveCriticalSection(mutex);
      return EBUSY;
    }
    return 0;
  }
  return EBUSY;
#else
  return pthread_mutex_trylock(mutex);
#endif
}

static inline int native_mutex_unlock(native_mutex_t *mutex) {
#ifdef _WIN32
  LeaveCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_unlock(mutex);
#endif
}

static inline int native_mutex_destroy(native_mutex_t *mutex) {
#ifdef _WIN32
  DeleteCriticalSection(mutex);
  return 0;
#else
  return pthread_mutex_destroy(mutex);
#endif
}

#ifdef SAFE_MUTEX
/* safe_mutex adds checking to mutex for easier debugging */
struct safe_mutex_t {
  native_mutex_t global, mutex;
  const char *file;
  uint line, count;
  my_thread_t thread;
};

void safe_mutex_global_init();
int safe_mutex_init(safe_mutex_t *mp, const native_mutexattr_t *attr,
                    const char *file, uint line);
int safe_mutex_lock(safe_mutex_t *mp, bool try_lock, const char *file,
                    uint line);
int safe_mutex_unlock(safe_mutex_t *mp, const char *file, uint line);
int safe_mutex_destroy(safe_mutex_t *mp, const char *file, uint line);

static inline void safe_mutex_assert_owner(safe_mutex_t *mp) {
  DBUG_ASSERT(mp != nullptr);
  native_mutex_lock(&mp->global);
  DBUG_ASSERT(mp->count > 0 && my_thread_equal(my_thread_self(), mp->thread));
  native_mutex_unlock(&mp->global);
}

static inline void safe_mutex_assert_not_owner(safe_mutex_t *mp) {
  DBUG_ASSERT(mp != nullptr);
  native_mutex_lock(&mp->global);
  DBUG_ASSERT(!mp->count || !my_thread_equal(my_thread_self(), mp->thread));
  native_mutex_unlock(&mp->global);
}
#endif /* SAFE_MUTEX */

static inline int my_mutex_init(my_mutex_t *mp, const native_mutexattr_t *attr
#ifdef SAFE_MUTEX
                                ,
                                const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  DBUG_ASSERT(mp != nullptr);
  mp->m_u.m_safe_ptr = (safe_mutex_t *)malloc(sizeof(safe_mutex_t));
  return safe_mutex_init(mp->m_u.m_safe_ptr, attr, file, line);
#else
  return native_mutex_init(&mp->m_u.m_native, attr);
#endif
}

static inline int my_mutex_lock(my_mutex_t *mp
#ifdef SAFE_MUTEX
                                ,
                                const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  DBUG_ASSERT(mp != nullptr);
  DBUG_ASSERT(mp->m_u.m_safe_ptr != nullptr);
  return safe_mutex_lock(mp->m_u.m_safe_ptr, false, file, line);
#else
  return native_mutex_lock(&mp->m_u.m_native);
#endif
}

static inline int my_mutex_trylock(my_mutex_t *mp
#ifdef SAFE_MUTEX
                                   ,
                                   const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  DBUG_ASSERT(mp != nullptr);
  DBUG_ASSERT(mp->m_u.m_safe_ptr != nullptr);
  return safe_mutex_lock(mp->m_u.m_safe_ptr, true, file, line);
#else
  return native_mutex_trylock(&mp->m_u.m_native);
#endif
}

static inline int my_mutex_unlock(my_mutex_t *mp
#ifdef SAFE_MUTEX
                                  ,
                                  const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  DBUG_ASSERT(mp != nullptr);
  DBUG_ASSERT(mp->m_u.m_safe_ptr != nullptr);
  return safe_mutex_unlock(mp->m_u.m_safe_ptr, file, line);
#else
  return native_mutex_unlock(&mp->m_u.m_native);
#endif
}

static inline int my_mutex_destroy(my_mutex_t *mp
#ifdef SAFE_MUTEX
                                   ,
                                   const char *file, uint line
#endif
) {
#ifdef SAFE_MUTEX
  DBUG_ASSERT(mp != nullptr);
  DBUG_ASSERT(mp->m_u.m_safe_ptr != nullptr);
  int rc = safe_mutex_destroy(mp->m_u.m_safe_ptr, file, line);
  free(mp->m_u.m_safe_ptr);
  mp->m_u.m_safe_ptr = nullptr;
  return rc;
#else
  return native_mutex_destroy(&mp->m_u.m_native);
#endif
}

#endif /* THR_MUTEX_INCLUDED */
