/* Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "unittest/gunit/thread_utils.h"

#include <errno.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <ostream>

#include "gtest/gtest-message.h"
#include "mutex_lock.h"
#include "my_inttypes.h"
#include "mysql/psi/mysql_cond.h"
#include "thr_mutex.h"

namespace thread {

namespace {
extern "C" void *thread_start_routine(void *arg) {
  Thread *start_arg = (Thread *)arg;
  Thread::run_wrapper(start_arg);
  return nullptr;
}

// We cannot use ASSERT_FALSE in constructors/destructors,
// so we add a local helper routine.
#define LOCAL_ASSERT_FALSE(arg) assert_false(arg, __LINE__)
void assert_false(int arg, int line) {
  ASSERT_FALSE(arg) << "failed with arg " << arg << " at line " << line;
}

}  // namespace

int Thread::start() {
  const int retval =
      my_thread_create(&m_thread_handle, nullptr, thread_start_routine, this);
  if (retval != 0) {
    ADD_FAILURE() << " could not start thread, errno: " << errno;
    return retval;
  }

  return retval;
}

void Thread::join() {
  const int failed = my_thread_join(&m_thread_handle, nullptr);
  if (failed) {
    ADD_FAILURE() << " could not join thread id " << m_thread_handle.thread
                  << " failed: " << failed << " errno: " << errno;
  }
}

void Thread::run_wrapper(Thread *start_arg) {
  const bool error = my_thread_init();
  ASSERT_FALSE(error);
  start_arg->run();
  my_thread_end();
}

Notification::Notification() : m_notified(false) {
  const int failed1 = mysql_cond_init(0, &m_cond);
  LOCAL_ASSERT_FALSE(failed1);
  const int failed2 = mysql_mutex_init(0, &m_mutex, MY_MUTEX_INIT_FAST);
  LOCAL_ASSERT_FALSE(failed2);
}

Notification::~Notification() {
  mysql_mutex_destroy(&m_mutex);
  mysql_cond_destroy(&m_cond);
}

bool Notification::has_been_notified() {
  MUTEX_LOCK(lock, &m_mutex);
  return m_notified;
}

void Notification::wait_for_notification() {
  MUTEX_LOCK(lock, &m_mutex);
  while (!m_notified) {
    const int failed = mysql_cond_wait(&m_cond, &m_mutex);
    ASSERT_FALSE(failed);
  }
}

void Notification::notify() {
  MUTEX_LOCK(lock, &m_mutex);
  m_notified = true;
  const int failed = mysql_cond_broadcast(&m_cond);
  ASSERT_FALSE(failed);
}

}  // namespace thread
