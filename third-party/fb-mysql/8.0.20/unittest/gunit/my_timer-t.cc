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

#include <gtest/gtest.h>
#include <string.h>

#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "my_timer.h"
#include "unittest/gunit/thr_template.cc"
#include "unittest/gunit/thread_utils.h"

#ifdef HAVE_PSI_INTERFACE
PSI_mutex_key key_thd_timer_mutex = PSI_NOT_INSTRUMENTED;
#endif

/**
  Cast a member of a structure to the structure that contains it.

  @param  ptr     Pointer to the member.
  @param  type    Type of the structure that contains the member.
  @param  member  Name of the member within the structure.
*/
#define my_container_of(ptr, type, member) \
  ((type *)((char *)ptr - offsetof(type, member)))

namespace my_timer_unittest {

typedef struct {
  my_timer_t timer;
  unsigned int fired;
  native_mutex_t mutex;
  native_cond_t cond;
} test_timer_t;

static void timer_notify_function(my_timer_t *timer) {
  test_timer_t *test = my_container_of(timer, test_timer_t, timer);

  native_mutex_lock(&test->mutex);
  test->fired++;
  native_cond_signal(&test->cond);
  native_mutex_unlock(&test->mutex);
}

static void test_timer_create(test_timer_t *test) {
  memset(test, 0, sizeof(test_timer_t));
  native_mutex_init(&test->mutex, nullptr);
  native_cond_init(&test->cond);
  test->timer.notify_function = timer_notify_function;
  EXPECT_EQ(my_timer_create(&test->timer), 0);
}

static void test_timer_destroy(test_timer_t *test) {
  native_mutex_destroy(&test->mutex);
  native_cond_destroy(&test->cond);
  my_timer_delete(&test->timer);
}

static void timer_set_and_wait(test_timer_t *test, unsigned int fired_count) {
  int rc, state;

  rc = my_timer_set(&test->timer, 5);
  EXPECT_EQ(rc, 0);

  // timer not fired yet
  EXPECT_TRUE((test->fired != fired_count));

  while (test->fired != fired_count)
    native_cond_wait(&test->cond, &test->mutex);

  // timer fired
  EXPECT_TRUE(test->fired == fired_count);

  rc = my_timer_cancel(&test->timer, &state);
  EXPECT_EQ(rc, 0);

  // timer state was signaled
  EXPECT_EQ(state, 0);
}

static void test_timer(void) {
  int rc;
  test_timer_t test;

  test_timer_create(&test);

  native_mutex_lock(&test.mutex);

  rc = my_timer_set(&test.timer, 5);
  EXPECT_EQ(rc, 0);

  /* not fired yet */
  EXPECT_EQ(test.fired, (unsigned int)0);

  while (!test.fired) native_cond_wait(&test.cond, &test.mutex);

  /* timer fired once */
  EXPECT_EQ(test.fired, (unsigned int)1);

  native_mutex_unlock(&test.mutex);

  test_timer_destroy(&test);
}

extern "C" void *test_timer_per_thread(void *arg) {
  int iter = *(int *)arg;

  while (iter--) test_timer();

  return nullptr;
}

/* Test timer creation and deletion. */
TEST(Mysys, TimerCreateAndDelete) {
  int rc;
  my_timer_t timer;

  EXPECT_EQ(my_timer_initialize(), 0);

  memset(&timer, 0, sizeof(timer));

  rc = my_timer_create(&timer);
  EXPECT_EQ(rc, 0);

  my_timer_delete(&timer);

  my_timer_deinitialize();
}

/* Test single timer in one thread */
TEST(Mysys, TestTimer) {
  int rc;
  test_timer_t test;

  EXPECT_EQ(my_timer_initialize(), 0);

  test_timer_create(&test);

  native_mutex_lock(&test.mutex);

  rc = my_timer_set(&test.timer, 5);
  EXPECT_EQ(rc, 0);

  // timer not fired yet
  EXPECT_EQ(test.fired, (unsigned int)0);

  while (!test.fired) native_cond_wait(&test.cond, &test.mutex);

  // timer fired once
  EXPECT_EQ(test.fired, (unsigned int)1);

  native_mutex_unlock(&test.mutex);

  test_timer_destroy(&test);

  my_timer_deinitialize();
}

/* Test reset function of timer */
TEST(Mysys, TestTimerReset) {
  int rc, state;
  test_timer_t test;

  EXPECT_EQ(my_timer_initialize(), 0);

  test_timer_create(&test);

  native_mutex_lock(&test.mutex);

  rc = my_timer_set(&test.timer, 600000);
  EXPECT_EQ(rc, 0);

  // timer not fired yet
  EXPECT_EQ(test.fired, (unsigned int)0);

  // reset timer
  rc = my_timer_cancel(&test.timer, &state);
  EXPECT_EQ(rc, 0);

  native_mutex_unlock(&test.mutex);

  test_timer_destroy(&test);

  my_timer_deinitialize();
}

/* Test multiple timers in single thread */
TEST(Mysys, TestMultipleTimers) {
  int rc, state;
  test_timer_t test1, test2, test3;

  EXPECT_EQ(my_timer_initialize(), 0);

  // Timer "test1"
  test_timer_create(&test1);
  rc = my_timer_set(&test1.timer, 3);
  EXPECT_EQ(rc, 0);

  // Timer "test2"
  test_timer_create(&test2);
  rc = my_timer_set(&test2.timer, 6);
  EXPECT_EQ(rc, 0);

  // Timer "test3"
  test_timer_create(&test3);
  rc = my_timer_set(&test3.timer, 600000);
  EXPECT_EQ(rc, 0);

  // Wait till test1 timer fired.
  native_mutex_lock(&test1.mutex);
  while (!test1.fired) native_cond_wait(&test1.cond, &test1.mutex);
  native_mutex_unlock(&test1.mutex);

  // Wait till test2 timer fired.
  native_mutex_lock(&test2.mutex);
  while (!test2.fired) native_cond_wait(&test2.cond, &test2.mutex);
  native_mutex_unlock(&test2.mutex);

  // timer test1 fired
  EXPECT_EQ(test1.fired, (unsigned int)1);

  // timer test2 fired
  EXPECT_EQ(test2.fired, (unsigned int)1);

  // timer test3 not fired yet
  EXPECT_EQ(test3.fired, (unsigned int)0);

  // reset timer test3
  rc = my_timer_cancel(&test3.timer, &state);
  EXPECT_EQ(rc, 0);

  test_timer_destroy(&test1);
  test_timer_destroy(&test2);
  test_timer_destroy(&test3);

  my_timer_deinitialize();
}

/* Test timer in multiple threads */
TEST(Mysys, TestTimerPerThread) {
  my_thread_attr_init(&thr_attr);
  my_thread_attr_setdetachstate(&thr_attr, MY_THREAD_CREATE_DETACHED);

  EXPECT_EQ(my_timer_initialize(), 0);

  test_concurrently("per-thread", test_timer_per_thread, THREADS, 5);

  my_timer_deinitialize();
  my_thread_attr_destroy(&thr_attr);
}

/* Test timer reuse functionality */
TEST(Mysys, TestTimerReuse) {
  test_timer_t test;

  EXPECT_EQ(my_timer_initialize(), 0);

  test_timer_create(&test);

  native_mutex_lock(&test.mutex);

  timer_set_and_wait(&test, 1);
  timer_set_and_wait(&test, 2);
  timer_set_and_wait(&test, 3);

  native_mutex_unlock(&test.mutex);

  test_timer_destroy(&test);

  my_timer_deinitialize();
}

/* Test timer module reinitialization */
TEST(Mysys, TestReinitialization) {
  EXPECT_EQ(my_timer_initialize(), 0);
  test_timer();
  my_timer_deinitialize();

  // Reinitialization
  EXPECT_EQ(my_timer_initialize(), 0);
  test_timer();
  my_timer_deinitialize();
}
}  // namespace my_timer_unittest
