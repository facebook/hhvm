/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef COMPONENTS_SERVICES_THR_MUTEX_BITS_H
#define COMPONENTS_SERVICES_THR_MUTEX_BITS_H

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>  // IWYU pragma: export
#include <sched.h>    // IWYU pragma: export
#endif

/**
  @file
  ABI for thd_mutex

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

#ifdef _WIN32
typedef CRITICAL_SECTION native_mutex_t;
typedef int native_mutexattr_t;
#else
typedef pthread_mutex_t native_mutex_t;
typedef pthread_mutexattr_t native_mutexattr_t;
#endif

struct safe_mutex_t;

struct my_mutex_t {
  union u {
    native_mutex_t m_native;
    safe_mutex_t *m_safe_ptr;
  } m_u;
};
typedef struct my_mutex_t my_mutex_t;

#endif /* COMPONENTS_SERVICES_THR_MUTEX_BITS_H */
