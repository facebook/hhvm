/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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
  @file mysys/win_timers.cc
*/

#if defined(_WIN32)
#include <errno.h>
#include <windows.h> /* Timer Queue and IO completion port functions */

#include "my_dbug.h"
#include "my_sys.h"    /* my_message_local */
#include "my_thread.h" /* my_thread_init, my_thread_end */
#include "my_timer.h"  /* my_timer_t */
#include "mysql/psi/mysql_thread.h"
#include "mysys_err.h"
#include "mysys_priv.h" /* key_thread_timer_notifier */

enum enum_timer_state { TIMER_SET = false, TIMER_EXPIRED = true };

// Timer notifier thread id.
static my_thread_handle timer_notify_thread;

// IO completion port handle
HANDLE io_compl_port = 0;

// Timer queue handle
HANDLE timer_queue = 0;

/**
  Callback function registered to execute on timer expiration.

  @param  timer_data             timer data passed to function.
  @param  timer_or_wait_fired    flag to represent timer fired or signalled.

  @remark this function is executed in timer owner thread when timer
          expires.
*/
static void CALLBACK timer_callback_function(
    PVOID timer_data, BOOLEAN timer_or_wait_fired MY_ATTRIBUTE((unused))) {
  my_timer_t *timer = (my_timer_t *)timer_data;
  DBUG_ASSERT(timer != NULL);
  timer->id.timer_state = TIMER_EXPIRED;
  PostQueuedCompletionStatus(io_compl_port, 0, (ULONG_PTR)timer, 0);
}

/**
  Timer expiration notification thread.

  @param arg  Unused.
*/
static void *timer_notify_thread_func(void *arg MY_ATTRIBUTE((unused))) {
  DWORD bytes_transferred;
  ULONG_PTR compl_key;
  LPOVERLAPPED overlapped;
  my_timer_t *timer;

  my_thread_init();

  while (1) {
    // Get IO Completion status.
    if (GetQueuedCompletionStatus(io_compl_port, &bytes_transferred, &compl_key,
                                  &overlapped, INFINITE) == 0)
      break;

    timer = (my_timer_t *)compl_key;
    timer->notify_function(timer);
  }

  my_thread_end();

  return NULL;
}

/**
  Delete a timer.

  @param timer    Timer Object.
  @param state    The state of the timer at the time of deletion, either
                  signaled (0) or nonsignaled (1).

  @return  0 On Success
          -1 On error.
*/
static int delete_timer(my_timer_t *timer, int *state) {
  int ret_val;
  int retry_count = 3;

  DBUG_ASSERT(timer != 0);
  DBUG_ASSERT(timer_queue != 0);

  if (state != NULL) *state = 0;

  if (timer->id.timer_handle) {
    do {
      ret_val =
          DeleteTimerQueueTimer(timer_queue, timer->id.timer_handle, NULL);

      if (ret_val != 0) {
        /**
          From MSDN documentation of DeleteTimerQueueTimer:

            ------------------------------------------------------------------

            BOOL WINAPI DeleteTimerQueueTimer(
              _In_opt_ HANDLE TimerQueue,
              _In_     HANDLE Timer,
              _In_opt_ HANDLE CompletionEvent
            );

            ...
            If there are outstanding callback functions and CompletionEvent is
            NULL, the function will fail and set the error code to
            ERROR_IO_PENDING. This indicates that there are outstanding callback
            functions. Those callbacks either will execute or are in the middle
            of executing.
            ...

            ------------------------------------------------------------------

          So we are here only in 2 cases,
             1 When timer is *not* expired yet.
             2 When timer is expired and callback function execution is
               completed.

          So here in case 1 timer.id->timer_state is TIMER_SET and
                  in case 2 timer.id->timer_state is TIMER_EXPIRED
          (From MSDN documentation(pasted above), if timer callback function is
           not yet executed or it is in the middle of execution then
           DeleteTimerQueueTimer() fails. Hence when we are here, we are sure
           that state is either TIMER_SET(case 1) or TIMER_EXPIRED(case 2))

          Note:
            timer.id->timer_state is set to TIMER_EXPIRED in
            timer_callback_function(). This function is executed by the OS
            thread on timer expiration.

            On timer expiration, when callback function is in the middle of
            execution or it is yet to be executed by OS thread the call to
            DeleteTimerQueueTimer() fails with an error "ERROR_IO_PENDING".
            In this case,  timer.id->timer_state is not accessed in the current
            code (please check else if block below).

            Since timer.id->timer_state is not accessed in the current code
            while it is getting modified in timer_callback_function,
            no synchronization mechanism used.

          Setting state to 1(non-signaled) if timer_state is not set to
          "TIMER_EXPIRED"
        */
        if (timer->id.timer_state != TIMER_EXPIRED && state != NULL) *state = 1;

        timer->id.timer_handle = 0;
      } else if (GetLastError() == ERROR_IO_PENDING) {
        /**
          Timer is expired and timer callback function execution is not
          yet completed.

          Note: timer->id.timer_state is modified in callback function.
                Accessing timer->id.timer_state here might result in
                race conditions.
                Currently we are not accessing timer->id.timer_state
                here so not using any synchronization mechanism.
        */
        timer->id.timer_handle = 0;
        ret_val = 1;
      } else {
        /**
          Timer deletion from queue failed and there are no outstanding
          callback functions for this timer.
        */
        if (--retry_count == 0) {
          my_message_local(ERROR_LEVEL, EE_FAILED_TO_DELETE_TIMER, errno);
          return -1;
        }
      }
    } while (ret_val == 0);
  }

  return 0;
}

/**
  Initialize internal components.

  @return 0 On success
          -1 On error.
*/
int my_timer_initialize(void) {
  // Create timer queue.
  timer_queue = CreateTimerQueue();
  if (!timer_queue) {
    my_message_local(ERROR_LEVEL, EE_FAILED_TO_CREATE_TIMER_QUEUE, errno);
    goto err;
  }

  // Create IO completion port.
  io_compl_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
  if (!io_compl_port) {
    my_message_local(ERROR_LEVEL, EE_FAILED_TO_CREATE_IO_COMPLETION_PORT,
                     errno);
    goto err;
  }

  if (mysql_thread_create(key_thread_timer_notifier, &timer_notify_thread, 0,
                          timer_notify_thread_func, 0)) {
    my_message_local(ERROR_LEVEL, EE_FAILED_TO_START_TIMER_NOTIFY_THREAD,
                     errno);
    goto err;
  }

  return 0;

err:
  if (timer_queue) {
    DeleteTimerQueueEx(timer_queue, NULL);
    timer_queue = 0;
  }

  if (io_compl_port) {
    CloseHandle(io_compl_port);
    io_compl_port = 0;
  }

  return -1;
}

/**
  Release any resources that were allocated as part of initialization.
*/
void my_timer_deinitialize() {
  if (timer_queue) {
    DeleteTimerQueueEx(timer_queue, NULL);
    timer_queue = 0;
  }

  if (io_compl_port) {
    CloseHandle(io_compl_port);
    io_compl_port = 0;
  }

  my_thread_join(&timer_notify_thread, NULL);
}

/**
  Create a timer object.

  @param  timer   Timer object.

  @return On success, 0.
*/
int my_timer_create(my_timer_t *timer) {
  DBUG_ASSERT(timer_queue != 0);
  timer->id.timer_handle = 0;
  return 0;
}

/**
  Set the time until the next expiration of the timer.

  @param  timer   Timer object.
  @param  time    Amount of time (in milliseconds) before the timer expires.

  @return On success, 0.
          On error, -1.
*/
int my_timer_set(my_timer_t *timer, unsigned long time) {
  DBUG_ASSERT(timer != NULL);
  DBUG_ASSERT(timer_queue != 0);

  /**
    If timer set previously is expired then it will not be
    removed from the timer queue. Removing it before creating
    a new timer queue timer.
  */
  if (timer->id.timer_handle != 0) my_timer_delete(timer);

  timer->id.timer_state = TIMER_SET;

  if (CreateTimerQueueTimer(&timer->id.timer_handle, timer_queue,
                            timer_callback_function, timer, time, 0,
                            WT_EXECUTEONLYONCE) == 0)
    return -1;

  return 0;
}

/**
  Cancel the timer.

  @param  timer   Timer object.
  @param  state   The state of the timer at the time of cancellation, either
                  signaled (0) or nonsignaled (1).

  @return On success, 0.
          On error,  -1.
*/
int my_timer_cancel(my_timer_t *timer, int *state) {
  DBUG_ASSERT(state != NULL);

  return delete_timer(timer, state);
}

/**
  Delete a timer object.

  @param  timer   Timer Object.
*/
void my_timer_delete(my_timer_t *timer) { delete_timer(timer, NULL); }
#endif
