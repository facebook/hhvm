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

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/posix_timers.cc
*/

#include <errno.h>
#include <signal.h>
#include <string.h> /* memset */
#include <sys/time.h>

#include "my_timer.h" /* my_timer_t */

/**
  Timer expiration notification thread.

  @param  arg   Event info.
*/

static void timer_notify_thread_func(sigval arg) {
  my_timer_t *timer = static_cast<my_timer_t *>(arg.sival_ptr);
  timer->notify_function(timer);
}

int my_timer_initialize() { return 0; }

void my_timer_deinitialize() {}

/**
  Create a timer object.

  @param  timer   Location where the timer ID is returned.

  @return On success, 0.
          On error, -1 is returned, and errno is set to indicate the error.
*/

int my_timer_create(my_timer_t *timer) {
  struct sigevent sigev;

  memset(&sigev, 0, sizeof(sigev));

  sigev.sigev_notify = SIGEV_THREAD;
  sigev.sigev_value.sival_ptr = timer;
  sigev.sigev_notify_function = &timer_notify_thread_func;

#ifdef __sun  // CLOCK_MONOTONIC not supported on Solaris even if it compiles.
  return timer_create(CLOCK_REALTIME, &sigev, &timer->id);
#else
  return timer_create(CLOCK_MONOTONIC, &sigev, &timer->id);
#endif
}

/**
  Set the time until the next expiration of the timer.

  @param  timer   Timer object.
  @param  time    Amount of time (in milliseconds) before the timer expires.

  @return On success, 0.
          On error, -1 is returned, and errno is set to indicate the error.
*/

int my_timer_set(my_timer_t *timer, unsigned long time) {
  struct itimerspec spec;
  spec.it_interval.tv_sec = 0;
  spec.it_interval.tv_nsec = 0;
  spec.it_value.tv_sec = static_cast<time_t>(time / 1000);
  spec.it_value.tv_nsec = static_cast<long>((time % 1000) * 1000000);
  return timer_settime(timer->id, 0, &spec, nullptr);
}

/**
  Cancel the timer.

  @param  timer   Timer object.
  @param  state   The state of the timer at the time of cancellation, either
                  signaled (false) or nonsignaled (true).

  @return On success, 0.
          On error, -1 is returned, and errno is set to indicate the error.
*/

int my_timer_cancel(my_timer_t *timer, int *state) {
  int status;
  struct itimerspec old_spec;

  /* A zeroed initial expiration value disarms the timer. */
  struct itimerspec zero_spec;
  zero_spec.it_interval.tv_sec = 0;
  zero_spec.it_interval.tv_nsec = 0;
  zero_spec.it_value.tv_sec = 0;
  zero_spec.it_value.tv_nsec = 0;

  /*
    timer_settime returns the amount of time before the timer
    would have expired or zero if the timer was disarmed.
  */
  if (!(status = timer_settime(timer->id, 0, &zero_spec, &old_spec)))
    *state = (old_spec.it_value.tv_sec || old_spec.it_value.tv_nsec);

  return status;
}

/**
  Delete a timer object.

  @param  timer   Timer object.
*/

void my_timer_delete(my_timer_t *timer) { timer_delete(timer->id); }
