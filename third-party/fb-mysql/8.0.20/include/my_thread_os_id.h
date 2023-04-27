/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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
  @file include/my_thread_os_id.h
  Portable wrapper for gettid().
*/

#ifndef MY_THREAD_OS_ID_INCLUDED
#define MY_THREAD_OS_ID_INCLUDED

#include "my_config.h"
#include "my_macros.h"
#include "my_thread.h"
#ifndef _WIN32
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef HAVE_PTHREAD_GETTHREADID_NP
#include <pthread_np.h> /* pthread_getthreadid_np() */
#endif                  /* HAVE_PTHREAD_GETTHREADID_NP */

#ifdef HAVE_PTHREAD_THREADID_NP
#include <pthread.h>
#endif /* HAVE_PTHREAD_THREADID_NP */

typedef unsigned long long my_thread_os_id_t;

/**
  Return the operating system thread id.
  With Linux, threads have:
  - an internal id, @c pthread_self(), visible in process
  - an external id, @c gettid(), visible in the operating system,
    for example with perf in linux.
  This helper returns the underling operating system thread id.
*/
static inline my_thread_os_id_t my_thread_os_id() {
#ifdef HAVE_PTHREAD_THREADID_NP
  /*
    macOS.

    Be careful to use this version first, and to not use SYS_gettid on macOS,
    as SYS_gettid has a different meaning compared to linux gettid().
  */
  uint64_t tid64;
  pthread_threadid_np(nullptr, &tid64);
  return (pid_t)tid64;
#else
#ifdef HAVE_SYS_GETTID
  /*
    Linux.
    See man gettid
    See GLIBC Bug 6399 - gettid() should have a wrapper
    https://sourceware.org/bugzilla/show_bug.cgi?id=6399
  */
  return syscall(SYS_gettid);
#else
#ifdef _WIN32
  /* Windows */
  return GetCurrentThreadId();
#else
#ifdef HAVE_PTHREAD_GETTHREADID_NP
  /* FreeBSD 10.2 */
  return pthread_getthreadid_np();
#else
#ifdef HAVE_INTEGER_PTHREAD_SELF
  /* Unknown platform, fallback. */
  return pthread_self();
#else
  /* Feature not available. */
  return 0;
#endif /* HAVE_INTEGER_PTHREAD_SELF */
#endif /* HAVE_PTHREAD_GETTHREADID_NP */
#endif /* _WIN32 */
#endif /* HAVE_SYS_GETTID */
#endif /* HAVE_SYS_THREAD_SELFID */
}

#endif /* MY_THREAD_OS_ID_INCLUDED */
