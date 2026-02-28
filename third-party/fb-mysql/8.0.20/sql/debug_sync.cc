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

/**
  == Debug Sync Facility ==

  The Debug Sync Facility allows placement of synchronization points in
  the server code by using the DEBUG_SYNC macro:

      open_tables(...)

      DEBUG_SYNC(thd, "after_open_tables");

      lock_tables(...)

  When activated, a sync point can

    - Emit a signal(s) and/or
    - Wait for a signal

  Nomenclature:

    - signal:             An event identified by a name that a signal
                          thread uses to notify the wait thread that
                          waits on this event. When the signal  thread
                          notifies the wait thread, the signal name
                          is copied into global list and the wait thread
                          is signalled to wake up and proceed with further
                          processing.

    - emit a signal:      Signal thread wakes up wait thread or multiple
                          wait threads that shall wait for the signal identified
                          by a signal name. This signal thread copies the signal
                          name into a global list and broadcasts the event which
                          wakes the threads that wait for this event.

    - wait for a signal:  Wait on a event indentified by the signal name until
                          the signal thread signals the event.

  By default, all sync points are inactive. They do nothing (except to
  burn a couple of CPU cycles for checking if they are active).

  A sync point becomes active when an action is requested for it.
  To do so, put a line like this in the test case file:

      SET DEBUG_SYNC= 'after_open_tables SIGNAL opened WAIT_FOR flushed';

  This activates the sync point 'after_open_tables'. It requests it to
  emit the signal 'opened' and wait for another thread to emit the signal
  'flushed' when the thread's execution runs through the sync point.

  For every sync point there can be one action per thread only. Every
  thread can request multiple actions, but only one per sync point. In
  other words, a thread can activate multiple sync points.

  However a single action can emit several signals, example given:

      SET DEBUG_SYNC= 'after_open_tables SIGNAL a,b,c WAIT_FOR flushed';

  Suppose we had several connections, and each one could possibly emit
  signal 'after_latch'. Let assume there is another connection, which
  waits for the signal being emitted. If the waiting connection wanted
  to recognize, which connection emitted 'after_latch', then we could
  decide to always emit two signals: 'after_latch' and 'con$id', where
  con$id would describe uniquely each connection (con1, con2, ...).
  Then the waiting connection could simply perform SELECT @@DEBUG_SYNC,
  and search for con* there. To remove such con$id from @@DEBUG_SYNC,
  one could then simply perform SET DEBUG_SYNC= 'now WAIT_FOR con$id'.

  Here is an example how to activate and use the sync points:

      --connection conn1
      SET DEBUG_SYNC= 'after_open_tables SIGNAL opened WAIT_FOR flushed';
      send INSERT INTO t1 VALUES(1);
          --connection conn2
          SET DEBUG_SYNC= 'now WAIT_FOR opened';
          SET DEBUG_SYNC= 'after_abort_locks SIGNAL flushed';
          FLUSH TABLE t1;

  When conn1 runs through the INSERT statement, it hits the sync point
  'after_open_tables'. It notices that it is active and executes its
  action. It emits the signal 'opened' and waits for another thread to
  emit the signal 'flushed'.

  conn2 waits immediately at the special sync point 'now' for another
  thread to emit the 'opened' signal.

  If conn1 signals 'opened' before conn2 reaches 'now', conn2 will find
  the 'opened' signal. The wait thread shall not wait in this case.

  When conn2 reaches 'after_abort_locks', it signals 'flushed', which lets
  conn1 awake and clears the 'flushed' signal from the global list. In case
  the 'flushed' signal is to be notified to multiple wait threads, an attribute
  NO_CLEAR_EVENT need to be specified with the WAIT_FOR in addition to signal
  the name as:
      SET DEBUG_SYNC= 'WAIT_FOR flushed NO_CLEAR_EVENT';
  It is up to the user to ensure once when all the wait threads have processed
  the 'flushed' signal to clear/deactivate the signal using the RESET action
  of DEBUG_SYNC accordingly.


  Normally the activation of a sync point is cleared when it has been
  executed. Sometimes it is necessary to keep the sync point active for
  another execution. You can add an execute count to the action:

      SET DEBUG_SYNC= 'name SIGNAL sig EXECUTE 3';

  This sets the signal point's activation counter to 3. Each execution
  decrements the counter. After the third execution the sync point
  becomes inactive.

  One of the primary goals of this facility is to eliminate sleeps from
  the test suite. In most cases it should be possible to rewrite test
  cases so that they do not need to sleep. (But this facility cannot
  synchronize multiple processes.) However, to support test development,
  and as a last resort, sync point waiting times out. There is a default
  timeout, but it can be overridden:

      SET DEBUG_SYNC= 'name WAIT_FOR sig TIMEOUT 10 EXECUTE 2';

  TIMEOUT 0 is special: If the signal is not present, the wait times out
  immediately.

  When a wait timed out (even on TIMEOUT 0), a warning is generated so
  that it shows up in the test result.

  You can throw an error message and kill the query when a synchronization
  point is hit a certain number of times:

      SET DEBUG_SYNC= 'name HIT_LIMIT 3';

  Or combine it with signal and/or wait:

      SET DEBUG_SYNC= 'name SIGNAL sig EXECUTE 2 HIT_LIMIT 3';

  Here the first two hits emit the signal, the third hit returns the error
  message and kills the query.

  For cases where you are not sure that an action is taken and thus
  cleared in any case, you can force to clear (deactivate) a sync point:

      SET DEBUG_SYNC= 'name CLEAR';

  If you want to clear all actions and clear the global signal, use:

      SET DEBUG_SYNC= 'RESET';

  This is the only way to reset the global signal to an empty string.

  For testing of the facility itself you can execute a sync point just
  as if it had been hit:

      SET DEBUG_SYNC= 'name TEST';


  === Formal Syntax ===

  The string to "assign" to the DEBUG_SYNC variable can contain:

      {RESET |
       <sync point name> TEST |
       <sync point name> CLEAR |
       <sync point name> {{SIGNAL <signal name>[, <signal name>]* |
                           WAIT_FOR <signal name> [TIMEOUT <seconds>]
                           [NO_CLEAR_EVENT]}
                          [EXECUTE <count>] &| HIT_LIMIT <count>}}

  Here '&|' means 'and/or'. This means that one of the sections
  separated by '&|' must be present or both of them.


  === Activation/Deactivation ===

  The facility is an optional part of the MySQL server.
  It is enabled in a debug server by default.

  The Debug Sync Facility, when compiled in, is disabled by default. It
  can be enabled by a mysqld command line option:

      --debug-sync-timeout[=default_wait_timeout_value_in_seconds]

  'default_wait_timeout_value_in_seconds' is the default timeout for the
  WAIT_FOR action. If set to zero, the facility stays disabled.

  The facility is enabled by default in the test suite, but can be
  disabled with:

      mysql-test-run.pl ... --debug-sync-timeout=0 ...

  Likewise the default wait timeout can be set:

      mysql-test-run.pl ... --debug-sync-timeout=10 ...

  The command line option influences the readable value of the system
  variable 'debug_sync'.

  * If the facility is not compiled in, the system variable does not exist.

  * If --debug-sync-timeout=0 the value of the variable reads as "OFF".

  * Otherwise the value reads as "ON - current signal: " followed by the
    current signal string, which can be empty.

  The readable variable value is the same, regardless if read as global
  or session value.

  Setting the 'debug-sync' system variable requires 'SUPER' privilege.
  You can never read back the string that you assigned to the variable,
  unless you assign the value that the variable does already have. But
  that would give a parse error. A syntactically correct string is
  parsed into a debug sync action and stored apart from the variable value.


  === Implementation ===

  Pseudo code for a sync point:

      #define DEBUG_SYNC(thd, sync_point_name)
                if (unlikely(opt_debug_sync_timeout))
                  debug_sync(thd, STRING_WITH_LEN(sync_point_name))

  The sync point performs a binary search in a sorted array of actions
  for this thread.

  The SET DEBUG_SYNC statement adds a requested action to the array or
  overwrites an existing action for the same sync point. When it adds a
  new action, the array is sorted again.


  === A typical synchronization pattern ===

  There are quite a few places in MySQL, where we use a synchronization
  pattern like this:

  mysql_mutex_lock(&mutex);
  thd->enter_cond(&condition_variable, &mutex, new_message);
  # if defined(ENABLE_DEBUG_SYNC)
  if (!thd->killed && !end_of_wait_condition)
     DEBUG_SYNC(thd, "sync_point_name");
  # endif
  while (!thd->killed && !end_of_wait_condition)
    mysql_cond_wait(&condition_variable, &mutex);
  mysql_mutex_unlock(&mutex);
  thd->exit_cond(old_message);

  Here some explanations:

  thd->enter_cond() is used to register the condition variable and the
  mutex in THD::current_cond/current_mutex. This is done to allow the
  thread to be interrupted (killed) from its sleep. Another thread can
  find the condition variable to signal and mutex to use for synchronization
  in this thread's THD.

  thd->enter_cond() requires the mutex to be acquired in advance.

  thd->exit_cond() unregisters the condition variable and mutex. Requires
  the mutex to be released in advance.

  If you want to have a Debug Sync point with the wait, please place it
  behind enter_cond(). Only then you can safely decide, if the wait will
  be taken. Also you will have THD::proc_info correct when the sync
  point emits a signal. DEBUG_SYNC sets its own proc_info, but restores
  the previous one before releasing its internal mutex. As soon as
  another thread sees the signal, it does also see the proc_info from
  before entering the sync point. In this case it will be "new_message",
  which is associated with the wait that is to be synchronized.

  In the example above, the wait condition is repeated before the sync
  point. This is done to skip the sync point, if no wait takes place.
  The sync point is before the loop (not inside the loop) to have it hit
  once only. It is possible that the condition variable is signaled
  multiple times without the wait condition to be true.

  A bit off-topic: At some places, the loop is taken around the whole
  synchronization pattern:

  while (!thd->killed && !end_of_wait_condition)
  {
    mysql_mutex_lock(&mutex);
    thd->enter_cond(&condition_variable, &mutex, new_message);
    if (!thd->killed [&& !end_of_wait_condition])
    {
      [DEBUG_SYNC(thd, "sync_point_name");]
      mysql_cond_wait(&condition_variable, &mutex);
    }
    mysql_mutex_unlock(&mutex);
    thd->exit_cond(old_message);
  }

  Note that it is important to repeat the test for thd->killed after
  enter_cond(). Otherwise the killing thread may kill this thread after
  it tested thd->killed in the loop condition and before it registered
  the condition variable and mutex in enter_cond(). In this case, the
  killing thread does not know that this thread is going to wait on a
  condition variable. It would just set THD::killed. But if we would not
  test it again, we would go asleep though we are killed. If the killing
  thread would kill us when we are after the second test, but still
  before sleeping, we hold the mutex, which is registered in THD.
  The killing thread would try to acquire the mutex before signaling
  the condition variable. Since the mutex is only released implicitly in
  mysql_cond_wait(), the signaling happens at the right place. We
  have a safe synchronization.

  === Co-work with the DBUG facility ===

  When running the MySQL test suite with the --debug command line
  option, the Debug Sync Facility writes trace messages to the DBUG
  trace. The following shell commands proved very useful in extracting
  relevant information:

  egrep 'query:|debug_sync_exec:' mysql-test/var/log/mysqld.1.trace

  It shows all executed SQL statements and all actions executed by
  synchronization points.

  Sometimes it is also useful to see, which synchronization points have
  been run through (hit) with or without executing actions. Then add
  "|debug_sync_point:" to the egrep pattern.

  === Further reading ===

  For a discussion of other methods to synchronize threads see
  http://forge.mysql.com/wiki/MySQL_Internals_Test_Synchronization

  For complete syntax tests, functional tests, and examples see the test
  case debug_sync.test.


  See also worklog entry WL#4259 - Test Synchronization Facility
*/

#include "sql/debug_sync.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <atomic>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/concept/usage.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_index/type_index_facade.hpp>
#include <memory>
#include <vector>

#include "boost/algorithm/string/detail/classification.hpp"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "sql/sql_error.h"
#include "sql/thr_malloc.h"
#include "sql_string.h"
#include "thr_mutex.h"

#if defined(ENABLED_DEBUG_SYNC)

#include <set>
#include <string>

#include "mysql/psi/mysql_memory.h"
#include "sql/current_thd.h"
#include "sql/derror.h"
#include "sql/log.h"
#include "sql/sql_class.h"

using std::max;
using std::min;

/*
  Action to perform at a synchronization point.
  NOTE: This structure is moved around in memory by realloc(), qsort(),
        and memmove(). Do not add objects with non-trivial constuctors
        or destructors, which might prevent moving of this structure
        with these functions.
*/
struct st_debug_sync_action {
  ulong activation_count = 0; /* max(hit_limit, execute) */
  ulong hit_limit = 0;        /* hits before kill query */
  ulong execute = 0;          /* executes before self-clear */
  ulong timeout = 0;          /* wait_for timeout */
  String signal;              /* signal to emit */
  String wait_for;            /* signal to wait for */
  String sync_point;          /* sync point name */
  bool need_sort = false;     /* if new action, array needs sort */
  bool clear_event = false;   /* do not clear signal if false */
};

/* Debug sync control. Referenced by THD. */
struct st_debug_sync_control {
  st_debug_sync_action *ds_action; /* array of actions */
  uint ds_active;                  /* # active actions */
  uint ds_allocated;               /* # allocated actions */
  ulonglong dsp_hits;              /* statistics */
  ulonglong dsp_executed;          /* statistics */
  ulonglong dsp_max_active;        /* statistics */
  /*
    thd->proc_info points at unsynchronized memory.
    It must not go away as long as the thread exists.
  */
  char ds_proc_info[80]; /* proc_info string */
};

typedef std::set<std::string> signal_event_set;

/**
  Definitions for the debug sync facility.
  1. Global set of signal names which are signalled.
  2. Global condition variable for signaling and waiting.
  3. Global mutex to synchronize access to the above.
*/
struct st_debug_sync_globals {
  signal_event_set ds_signal_set; /* list of signals signalled */
  mysql_cond_t ds_cond;           /* condition variable */
  mysql_mutex_t ds_mutex;         /* mutex variable */
  ulonglong dsp_hits;             /* statistics */
  ulonglong dsp_executed;         /* statistics */
  ulonglong dsp_max_active;       /* statistics */

  st_debug_sync_globals() : dsp_hits(0), dsp_executed(0), dsp_max_active(0) {}

 private:
  // Not implemented:
  st_debug_sync_globals(const st_debug_sync_globals &);
  st_debug_sync_globals &operator=(const st_debug_sync_globals &);
};
static st_debug_sync_globals debug_sync_global; /* All globals in one object */

/**
  Callback pointer for C files.
*/
extern DebugSyncCallbackFp debug_sync_C_callback_ptr;

/**
  Callbacks from C files.
*/
static void debug_sync_C_callback(const char *, size_t);

/**
  Callback for debug sync, to be used by C files. See thr_lock.c for example.

  @description

    We cannot place a sync point directly in C files (like those in mysys or
    certain storage engines written mostly in C like MyISAM or Maria). Because
    they are C code and do not know the
    macro DEBUG_SYNC(thd, sync_point_name). The macro needs a 'thd' argument.
    Hence it cannot be used in files outside of the sql/ directory.

    The workaround is to call back simple functions like this one from
    non-sql/ files.

    We want to allow modules like thr_lock to be used without sql/ and
    especially without Debug Sync. So we cannot just do a simple call
    of the callback function. Instead we provide a global pointer in
    the other file, which is to be set to the callback by Debug Sync.
    If the pointer is not set, no call back will be done. If Debug
    Sync sets the pointer to a callback function like this one, it will
    be called. That way thr_lock.c does not have an undefined reference
    to Debug Sync and can be used without it. Debug Sync, in contrast,
    has an undefined reference to that pointer and thus requires
    thr_lock to be linked too. But this is not a problem as it is part
    of the MySQL server anyway.

  @note
    The callback pointer in C files is set only if debug sync is
    initialized. And this is done only if opt_debug_sync_timeout is set.
*/

static void debug_sync_C_callback(const char *sync_point_name,
                                  size_t name_len) {
  if (unlikely(opt_debug_sync_timeout))
    debug_sync(current_thd, sync_point_name, name_len);
}

static PSI_memory_key key_debug_THD_debug_sync_control;
static PSI_memory_key key_debug_sync_action;

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_debug_sync_globals_ds_mutex;

static PSI_mutex_info all_debug_sync_mutexes[] = {
    {&key_debug_sync_globals_ds_mutex, "DEBUG_SYNC::mutex", PSI_FLAG_SINGLETON,
     0, PSI_DOCUMENT_ME}};

static PSI_cond_key key_debug_sync_globals_ds_cond;

static PSI_cond_info all_debug_sync_conds[] = {
    {&key_debug_sync_globals_ds_cond, "DEBUG_SYNC::cond", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static PSI_memory_info all_debug_sync_memory[] = {
    {&key_debug_THD_debug_sync_control, "THD::debug_sync_control", 0, 0,
     PSI_DOCUMENT_ME},
    {&key_debug_sync_action, "debug_sync_control::debug_sync_action", 0, 0,
     PSI_DOCUMENT_ME}};

static void init_debug_sync_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_debug_sync_mutexes));
  mysql_mutex_register(category, all_debug_sync_mutexes, count);

  count = static_cast<int>(array_elements(all_debug_sync_conds));
  mysql_cond_register(category, all_debug_sync_conds, count);

  count = static_cast<int>(array_elements(all_debug_sync_memory));
  mysql_memory_register(category, all_debug_sync_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

/**
  Set the THD::proc_info without instrumentation.
  This method is private to DEBUG_SYNC,
  and on purpose avoid any use of:
  - the SHOW PROFILE instrumentation
  - the PERFORMANCE_SCHEMA instrumentation
  so that using DEBUG_SYNC() in the server code
  does not cause the instrumentations to record
  spurious data.
*/
static const char *debug_sync_thd_proc_info(THD *thd, const char *info) {
  const char *old_proc_info = thd->proc_info;
  thd->proc_info = info;
  return old_proc_info;
}

/**
  Initialize the debug sync facility at server start.

  @return status
    @retval     0       ok
    @retval     != 0    error
*/

int debug_sync_init(void) {
  DBUG_TRACE;

#ifdef HAVE_PSI_INTERFACE
  init_debug_sync_psi_keys();
#endif

  if (opt_debug_sync_timeout) {
    int rc;

    /* Initialize the global variables. */
    if ((rc = mysql_cond_init(key_debug_sync_globals_ds_cond,
                              &debug_sync_global.ds_cond)) ||
        (rc =
             mysql_mutex_init(key_debug_sync_globals_ds_mutex,
                              &debug_sync_global.ds_mutex, MY_MUTEX_INIT_FAST)))
      return rc; /* purecov: inspected */

    /* Set the call back pointer in C files. */
    debug_sync_C_callback_ptr = debug_sync_C_callback;
  }

  return 0;
}

/**
  End the debug sync facility.

  @description
    This is called at server shutdown or after a thread initialization error.
*/

void debug_sync_end(void) {
  DBUG_TRACE;

  /* End the facility only if it had been initialized. */
  if (debug_sync_C_callback_ptr) {
    /* Clear the call back pointer in C files. */
    debug_sync_C_callback_ptr = nullptr;

    /* Destroy the global variables. */
    debug_sync_global.ds_signal_set.clear();
    mysql_cond_destroy(&debug_sync_global.ds_cond);
    mysql_mutex_destroy(&debug_sync_global.ds_mutex);

    /* Print statistics. */
    {
      char llbuff[22];
      LogErr(INFORMATION_LEVEL, ER_DEBUG_SYNC_HIT,
             llstr(debug_sync_global.dsp_hits, llbuff));
      LogErr(INFORMATION_LEVEL, ER_DEBUG_SYNC_EXECUTED,
             llstr(debug_sync_global.dsp_executed, llbuff));
      LogErr(INFORMATION_LEVEL, ER_DEBUG_SYNC_THREAD_MAX,
             llstr(debug_sync_global.dsp_max_active, llbuff));
    }
  }
}

/* purecov: begin tested */

/**
  Disable the facility after lack of memory if no error can be returned.

  @note
    Do not end the facility here because the global variables can
    be in use by other threads.
*/

static void debug_sync_emergency_disable(void) {
  DBUG_TRACE;

  opt_debug_sync_timeout = 0;

  DBUG_PRINT("debug_sync",
             ("Debug Sync Facility disabled due to lack of memory."));
  LogErr(ERROR_LEVEL, ER_DEBUG_SYNC_OOM);
}

/* purecov: end */

/**
  Initialize the debug sync facility at thread start.

  @param[in]    thd             thread handle
*/

void debug_sync_init_thread(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  if (opt_debug_sync_timeout) {
    thd->debug_sync_control = (st_debug_sync_control *)my_malloc(
        key_debug_THD_debug_sync_control, sizeof(st_debug_sync_control),
        MYF(MY_WME | MY_ZEROFILL));
    if (!thd->debug_sync_control) {
      /*
        Error is reported by my_malloc().
        We must disable the facility. We have no way to return an error.
      */
      debug_sync_emergency_disable(); /* purecov: tested */
    }
  }
}

void debug_sync_claim_memory_ownership(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  st_debug_sync_control *ds_control = thd->debug_sync_control;

  if (ds_control != nullptr) {
    if (ds_control->ds_action) {
      st_debug_sync_action *action = ds_control->ds_action;
      st_debug_sync_action *action_end = action + ds_control->ds_allocated;
      for (; action < action_end; action++) {
        action->signal.mem_claim();
        action->wait_for.mem_claim();
        action->sync_point.mem_claim();
      }
      my_claim(ds_control->ds_action);
    }

    my_claim(ds_control);
  }
}

/**
  End the debug sync facility at thread end.

  @param[in]    thd             thread handle
*/

void debug_sync_end_thread(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  if (thd->debug_sync_control) {
    st_debug_sync_control *ds_control = thd->debug_sync_control;

    /*
      This synchronization point can be used to synchronize on thread end.
      This is the latest point in a THD's life, where this can be done.
    */
    DEBUG_SYNC(thd, "thread_end");

    if (ds_control->ds_action) {
      st_debug_sync_action *action = ds_control->ds_action;
      st_debug_sync_action *action_end = action + ds_control->ds_allocated;
      for (; action < action_end; action++) {
        action->signal.mem_free();
        action->wait_for.mem_free();
        action->sync_point.mem_free();
      }
      my_free(ds_control->ds_action);
    }

    /* Statistics. */
    mysql_mutex_lock(&debug_sync_global.ds_mutex);
    debug_sync_global.dsp_hits += ds_control->dsp_hits;
    debug_sync_global.dsp_executed += ds_control->dsp_executed;
    if (debug_sync_global.dsp_max_active < ds_control->dsp_max_active)
      debug_sync_global.dsp_max_active = ds_control->dsp_max_active;
    mysql_mutex_unlock(&debug_sync_global.ds_mutex);

    my_free(ds_control);
    thd->debug_sync_control = nullptr;
  }
}

/**
  Move a string by length.

  @param[out]   to              buffer for the resulting string
  @param[in]    to_end          end of buffer
  @param[in]    from            source string
  @param[in]    length          number of bytes to copy

  @return       pointer to end of copied string
*/

static char *debug_sync_bmove_len(char *to, char *to_end, const char *from,
                                  size_t length) {
  DBUG_ASSERT(to);
  DBUG_ASSERT(to_end);
  DBUG_ASSERT(!length || from);
  length = std::min(length, size_t(to_end - to));
  memcpy(to, from, length);
  return (to + length);
}

#if !defined(DBUG_OFF)

/**
  Create a string that describes an action.

  @param[out]   result          buffer for the resulting string
  @param[in]    size            size of result buffer
  @param[in]    action          action to describe
*/

static void debug_sync_action_string(char *result, uint size,
                                     st_debug_sync_action *action) {
  char *wtxt = result;
  char *wend = wtxt + size - 1; /* Allow emergency '\0'. */
  DBUG_ASSERT(result);
  DBUG_ASSERT(action);

  /* If an execute count is present, signal or wait_for are needed too. */
  DBUG_ASSERT(!action->execute || action->signal.length() ||
              action->wait_for.length());

  if (action->execute) {
    if (action->signal.length()) {
      wtxt = debug_sync_bmove_len(wtxt, wend, STRING_WITH_LEN("SIGNAL "));
      wtxt = debug_sync_bmove_len(wtxt, wend, action->signal.ptr(),
                                  action->signal.length());
    }
    if (action->wait_for.length()) {
      if ((wtxt == result) && (wtxt < wend)) *(wtxt++) = ' ';
      wtxt = debug_sync_bmove_len(wtxt, wend, STRING_WITH_LEN(" WAIT_FOR "));
      wtxt = debug_sync_bmove_len(wtxt, wend, action->wait_for.ptr(),
                                  action->wait_for.length());

      if (action->timeout != opt_debug_sync_timeout) {
        wtxt += snprintf(wtxt, wend - wtxt, " TIMEOUT %lu", action->timeout);
      }
    }
    if (action->execute != 1) {
      wtxt += snprintf(wtxt, wend - wtxt, " EXECUTE %lu", action->execute);
    }
  }
  if (action->hit_limit) {
    wtxt += snprintf(wtxt, wend - wtxt, "%sHIT_LIMIT %lu",
                     (wtxt == result) ? "" : " ", action->hit_limit);
  }

  /*
    If (wtxt == wend) string may not be terminated.
    There is one byte left for an emergency termination.
  */
  *wtxt = '\0';
}

/**
  Print actions.

  @param[in]    thd             thread handle
*/

static void debug_sync_print_actions(THD *thd) {
  st_debug_sync_control *ds_control = thd->debug_sync_control;
  uint idx;
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  if (!ds_control) return;

  for (idx = 0; idx < ds_control->ds_active; idx++) {
    const char *dsp_name = ds_control->ds_action[idx].sync_point.c_ptr();
    char action_string[256];

    debug_sync_action_string(action_string, sizeof(action_string),
                             ds_control->ds_action + idx);
    DBUG_PRINT("debug_sync_list", ("%s %s", dsp_name, action_string));
  }
}

#endif /* !defined(DBUG_OFF) */

/**
  Find a debug sync action.

  @param[in]    actionarr       array of debug sync actions
  @param[in]    quantity        number of actions in array
  @param[in]    dsp_name        name of debug sync point to find
  @param[in]    name_len        length of name of debug sync point

  @return       action
    @retval     != NULL         found sync point in array
    @retval     NULL            not found

  @description
    Binary search. Array needs to be sorted by length, sync point name.
*/

static st_debug_sync_action *debug_sync_find(st_debug_sync_action *actionarr,
                                             int quantity, const char *dsp_name,
                                             size_t name_len) {
  st_debug_sync_action *action;
  int low;
  int high;
  int mid;
  int diff;
  DBUG_ASSERT(actionarr);
  DBUG_ASSERT(dsp_name);
  DBUG_ASSERT(name_len);

  low = 0;
  high = quantity;

  while (low < high) {
    mid = (low + high) / 2;
    action = actionarr + mid;
    if (!(diff = static_cast<int>(name_len - action->sync_point.length())) &&
        !(diff = memcmp(dsp_name, action->sync_point.ptr(), name_len)))
      return action;
    if (diff > 0)
      low = mid + 1;
    else
      high = mid - 1;
  }

  if (low < quantity) {
    action = actionarr + low;
    if ((name_len == action->sync_point.length()) &&
        !memcmp(dsp_name, action->sync_point.ptr(), name_len))
      return action;
  }

  return nullptr;
}

/**
  Reset the debug sync facility.

  @param[in]    thd             thread handle

  @description
    Remove all actions of this thread.
    Clear the global signal.
*/

static void debug_sync_reset(THD *thd) {
  st_debug_sync_control *ds_control = thd->debug_sync_control;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(ds_control);

  /* Remove all actions of this thread. */
  ds_control->ds_active = 0;

  /* Clear the signals. */
  mysql_mutex_lock(&debug_sync_global.ds_mutex);
  debug_sync_global.ds_signal_set.clear();
  mysql_mutex_unlock(&debug_sync_global.ds_mutex);
}

/**
  Remove a debug sync action.

  @param[in]    ds_control      control object
  @param[in]    action          action to be removed

  @description
    Removing an action mainly means to decrement the ds_active counter.
    But if the action is between other active action in the array, then
    the array needs to be shrinked. The active actions above the one to
    be removed have to be moved down by one slot.
*/

static void debug_sync_remove_action(st_debug_sync_control *ds_control,
                                     st_debug_sync_action *action) {
  uint dsp_idx = static_cast<uint>(action - ds_control->ds_action);
  DBUG_TRACE;
  DBUG_ASSERT(ds_control);
  DBUG_ASSERT(ds_control == current_thd->debug_sync_control);
  DBUG_ASSERT(action);
  DBUG_ASSERT(dsp_idx < ds_control->ds_active);

  /* Decrement the number of currently active actions. */
  ds_control->ds_active--;

  /*
    If this was not the last active action in the array, we need to
    shift remaining active actions down to keep the array gap-free.
    Otherwise binary search might fail or take longer than necessary at
    least. Also new actions are always put to the end of the array.
  */
  if (ds_control->ds_active > dsp_idx) {
    /*
      Copy the to-be-removed action object to temporary storage before
      the left-shift below.
    */
    st_debug_sync_action save_action = std::move(*action);

    /* Move actions down. */
    st_debug_sync_action *dest_action = ds_control->ds_action + dsp_idx;
    st_debug_sync_action *src_action = ds_control->ds_action + dsp_idx + 1;
    uint num_actions = ds_control->ds_active - dsp_idx;

    std::move(src_action, src_action + num_actions, dest_action);

    /*
      Copy back the saved action object to the now free array slot.
    */
    dest_action = ds_control->ds_action + ds_control->ds_active;
    *dest_action = std::move(save_action);
  }
}

/**
  Get a debug sync action.

  @param[in]    thd             thread handle
  @param[in]    dsp_name        debug sync point name
  @param[in]    name_len        length of sync point name

  @return       action
    @retval     != NULL         ok
    @retval     NULL            error

  @description
    Find the debug sync action for a debug sync point or make a new one.
*/

static st_debug_sync_action *debug_sync_get_action(THD *thd,
                                                   const char *dsp_name,
                                                   size_t name_len) {
  st_debug_sync_control *ds_control = thd->debug_sync_control;
  st_debug_sync_action *action;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(dsp_name);
  DBUG_ASSERT(name_len);
  DBUG_ASSERT(ds_control);
  DBUG_PRINT("debug_sync", ("sync_point: '%.*s'", (int)name_len, dsp_name));
  DBUG_PRINT("debug_sync", ("active: %u  allocated: %u", ds_control->ds_active,
                            ds_control->ds_allocated));

  /* There cannot be more active actions than allocated. */
  DBUG_ASSERT(ds_control->ds_active <= ds_control->ds_allocated);
  /* If there are active actions, the action array must be present. */
  DBUG_ASSERT(!ds_control->ds_active || ds_control->ds_action);

  /* Try to reuse existing action if there is one for this sync point. */
  if (ds_control->ds_active &&
      (action = debug_sync_find(ds_control->ds_action, ds_control->ds_active,
                                dsp_name, name_len))) {
    /* Reuse an already active sync point action. */
    DBUG_ASSERT((uint)(action - ds_control->ds_action) < ds_control->ds_active);
    DBUG_PRINT("debug_sync", ("reuse action idx: %ld",
                              (long)(action - ds_control->ds_action)));
  } else {
    /* Create a new action. */
    int dsp_idx = ds_control->ds_active++;
    ds_control->dsp_max_active =
        std::max(ds_control->dsp_max_active, ulonglong(ds_control->ds_active));
    if (ds_control->ds_active > ds_control->ds_allocated) {
      uint new_alloc = ds_control->ds_active + 3;
      void *new_action =
          my_malloc(key_debug_sync_action,
                    new_alloc * sizeof(st_debug_sync_action), MYF(MY_WME));
      if (!new_action) {
        /* Error is reported by my_malloc(). */
        return nullptr; /* purecov: tested */
      }
      // Move objects into newly allocated memory.
      // TODO: use std::uninitialized_move in C++17
      if (ds_control->ds_action != nullptr) {
        st_debug_sync_action *d_first =
            static_cast<st_debug_sync_action *>(new_action);
        for (int ix = 0; ix < dsp_idx; ++ix) {
          st_debug_sync_action *src = ds_control->ds_action + ix;
          st_debug_sync_action *dst = d_first + ix;
          new (dst) st_debug_sync_action(std::move(*src));
        }
        my_free(ds_control->ds_action);
      }

      ds_control->ds_action = (st_debug_sync_action *)new_action;
      ds_control->ds_allocated = new_alloc;
      /* Clear new entries. */
      st_debug_sync_action *dest_action = ds_control->ds_action + dsp_idx;
      std::uninitialized_fill_n(dest_action, (new_alloc - dsp_idx),
                                st_debug_sync_action());
    }
    DBUG_PRINT("debug_sync", ("added action idx: %u", dsp_idx));
    action = ds_control->ds_action + dsp_idx;
    if (action->sync_point.copy(dsp_name, name_len, system_charset_info)) {
      /* Error is reported by my_malloc(). */
      return nullptr; /* purecov: tested */
    }
    action->need_sort = true;
  }
  DBUG_ASSERT(action >= ds_control->ds_action);
  DBUG_ASSERT(action < ds_control->ds_action + ds_control->ds_active);
  DBUG_PRINT("debug_sync", ("action: %p  array: %p  count: %u", action,
                            ds_control->ds_action, ds_control->ds_active));

  return action;
}

/**
  Set a debug sync action.

  @param[in]    thd             thread handle
  @param[in]    action          synchronization action

  @return       status
    @retval     false           ok
    @retval     true            error

  @description
    This is called from the debug sync parser. It arms the action for
    the requested sync point. If the action parsed into an empty action,
    it is removed instead.

    Setting an action for a sync point means to make the sync point
    active. When it is hit it will execute this action.

    Before parsing, we "get" an action object. This is placed at the
    end of the thread's action array unless the requested sync point
    has an action already.

    Then the parser fills the action object from the request string.

    Finally the action is "set" for the sync point. If it was parsed
    to be empty, it is removed from the array. If it did belong to a
    sync point before, the sync point becomes inactive. If the action
    became non-empty and it did not belong to a sync point before (it
    was added at the end of the action array), the action array needs
    to be sorted by sync point.

    If the sync point name is "now", it is executed immediately.
*/

static bool debug_sync_set_action(THD *thd, st_debug_sync_action *action) {
  st_debug_sync_control *ds_control = thd->debug_sync_control;
  bool is_dsp_now = false;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(action);
  DBUG_ASSERT(ds_control);

  action->activation_count = max(action->hit_limit, action->execute);
  if (!action->activation_count) {
    debug_sync_remove_action(ds_control, action);
    DBUG_PRINT("debug_sync", ("action cleared"));
  } else {
    const char *dsp_name = action->sync_point.c_ptr();
    DBUG_EXECUTE("debug_sync", {
      /* Functions as DBUG_PRINT args can change keyword and line nr. */
      const char *sig_emit = action->signal.c_ptr();
      const char *sig_wait = action->wait_for.c_ptr();
      DBUG_PRINT("debug_sync",
                 ("sync_point: '%s'  activation_count: %lu  hit_limit: %lu  "
                  "execute: %lu  timeout: %lu  signal: '%s'  wait_for: '%s'",
                  dsp_name, action->activation_count, action->hit_limit,
                  action->execute, action->timeout, sig_emit, sig_wait));
    });

    /* Check this before sorting the array. action may move. */
    is_dsp_now = !my_strcasecmp(system_charset_info, dsp_name, "now");

    if (action->need_sort) {
      action->need_sort = false;
      /* Sort actions by (name_len, name). */
      std::sort(
          ds_control->ds_action, ds_control->ds_action + ds_control->ds_active,
          [](const st_debug_sync_action &a, const st_debug_sync_action &b) {
            if (a.sync_point.length() != b.sync_point.length())
              return a.sync_point.length() < b.sync_point.length();
            return memcmp(a.sync_point.ptr(), b.sync_point.ptr(),
                          a.sync_point.length()) < 0;
          });
    }
  }
  DBUG_EXECUTE("debug_sync_list", debug_sync_print_actions(thd););

  /* Execute the special sync point 'now' if activated above. */
  if (is_dsp_now) {
    DEBUG_SYNC(thd, "now");
    /*
      If HIT_LIMIT for sync point "now" was 1, the execution of the sync
      point decremented it to 0. In this case the following happened:

      - an error message was reported with my_error() and
      - the statement was killed with thd->killed= THD::KILL_QUERY.

      If a statement reports an error, it must not call send_ok().
      The calling functions will not call send_ok(), if we return true
      from this function.

      thd->killed is also set if the wait is interrupted from a
      KILL or KILL QUERY statement. In this case, no error is reported
      and shall not be reported as a result of SET DEBUG_SYNC.
      Hence, we check for the first condition above.
    */
    if (thd->is_error()) return true;
  }

  return false;
}

/*
  Advance the pointer by length of multi-byte character.

    @param    ptr   pointer to multibyte character.

    @return   NULL or pointer after advancing pointer by the
              length of multi-byte character pointed to.
*/

static inline const char *advance_mbchar_ptr(const char *ptr) {
  uint clen = my_mbcharlen(system_charset_info, (uchar)*ptr);

  return (clen != 0) ? ptr + clen : nullptr;
}

/*
  Skip whitespace characters from the beginning of the multi-byte string.

  @param    ptr     pointer to the multi-byte string.

  @return   a pointer to the first non-whitespace character or NULL if the
            string consists from whitespace characters only.
*/

static inline const char *skip_whitespace(const char *ptr) {
  while (ptr != nullptr && *ptr && my_isspace(system_charset_info, *ptr))
    ptr = advance_mbchar_ptr(ptr);

  return ptr;
}

/*
  Get pointer to end of token.

  @param    ptr  pointer to start of token

  @return   NULL or pointer to end of token.
*/

static inline const char *get_token_end_ptr(const char *ptr) {
  while (ptr != nullptr && *ptr && !my_isspace(system_charset_info, *ptr))
    ptr = advance_mbchar_ptr(ptr);

  return ptr;
}

/**
  Extract a token from a string.

  @param[out]     token_p         returns start of token
  @param[out]     token_length_p  returns length of token
  @param[in,out]  ptr             current string pointer, adds '\0' terminators

  @return       string pointer or NULL
    @retval     != NULL         ptr behind token terminator or at string end
    @retval     NULL            no token found in remainder of string

  @note
    This function assumes that the string is in system_charset_info,
    that this charset is single byte for ASCII NUL ('\0'), that no
    character except of ASCII NUL ('\0') contains a byte with value 0,
    and that ASCII NUL ('\0') is used as the string terminator.

    This function needs to return tokens that are terminated with ASCII
    NUL ('\0'). The tokens are used in my_strcasecmp(). Unfortunately
    there is no my_strncasecmp().

    To return the last token without copying it, we require the input
    string to be nul terminated.

  @description
    This function skips space characters at string begin.

    It returns a pointer to the first non-space character in *token_p.

    If no non-space character is found before the string terminator
    ASCII NUL ('\0'), the function returns NULL. *token_p and
    *token_length_p remain unchanged in this case (they are not set).

    The function takes a space character or an ASCII NUL ('\0') as a
    terminator of the token. The space character could be multi-byte.

    It returns the length of the token in bytes, excluding the
    terminator, in *token_length_p.

    If the terminator of the token is ASCII NUL ('\0'), it returns a
    pointer to the terminator (string end).

    If the terminator is a space character, it replaces the the first
    byte of the terminator character by ASCII NUL ('\0'), skips the (now
    corrupted) terminator character, and skips all following space
    characters. It returns a pointer to the next non-space character or
    to the string terminator ASCII NUL ('\0').
*/

static char *debug_sync_token(char **token_p, size_t *token_length_p,
                              char *ptr) {
  DBUG_ASSERT(token_p);
  DBUG_ASSERT(token_length_p);
  DBUG_ASSERT(ptr);

  /* Skip leading space */
  ptr = const_cast<char *>(skip_whitespace(ptr));

  if (ptr == nullptr || !*ptr) return nullptr;

  /* Get token start. */
  *token_p = ptr;

  /* Find token end. */
  ptr = const_cast<char *>(get_token_end_ptr(ptr));

  if (ptr == nullptr) return nullptr;

  /* Get token length. */
  *token_length_p = ptr - *token_p;

  /* If necessary, terminate token. */
  if (*ptr) {
    char *tmp = ptr;

    /* Advance by terminator character length. */
    ptr = const_cast<char *>(advance_mbchar_ptr(ptr));
    if (ptr != nullptr) {
      /* Terminate token. */
      *tmp = '\0';

      /* Skip trailing space */
      ptr = const_cast<char *>(skip_whitespace(ptr));
    }
  }
  return ptr;
}

/**
  Extract a number from a string.

  @param[out]   number_p        returns number
  @param[in]    actstrptr       current pointer in action string

  @return       string pointer or NULL
    @retval     != NULL         ptr behind token terminator or at string end
    @retval     NULL            no token found or token is not valid number

  @note
    The same assumptions about charset apply as for debug_sync_token().

  @description
    This function fetches a token from the string and converts it
    into a number.

    If there is no token left in the string, or the token is not a valid
    decimal number, NULL is returned. The result in *number_p is
    undefined in this case.
*/

static char *debug_sync_number(ulong *number_p, char *actstrptr) {
  char *ptr;
  char *ept;
  char *token;
  size_t token_length;
  DBUG_ASSERT(number_p);
  DBUG_ASSERT(actstrptr);

  /* Get token from string. */
  if (!(ptr = debug_sync_token(&token, &token_length, actstrptr))) goto end;

  *number_p = strtoul(token, &ept, 10);
  if (*ept) ptr = nullptr;

end:
  return ptr;
}

/**
  Evaluate a debug sync action string.

  @param[in]        thd             thread handle
  @param[in,out]    action_str      action string to receive '\0' terminators

  @return           status
    @retval         false           ok
    @retval         true            error

  @description
    This is called when the DEBUG_SYNC system variable is set.
    Parse action string, build a debug sync action, activate it.

    Before parsing, we "get" an action object. This is placed at the
    end of the thread's action array unless the requested sync point
    has an action already.

    Then the parser fills the action object from the request string.

    Finally the action is "set" for the sync point. This means that the
    sync point becomes active or inactive, depending on the action
    values.

  @note
    The input string needs to be ASCII NUL ('\0') terminated. We split
    nul-terminated tokens in it without copy.

  @see the function comment of debug_sync_token() for more constraints
    for the string.
*/

static bool debug_sync_eval_action(THD *thd, char *action_str) {
  st_debug_sync_action *action = nullptr;
  const char *errmsg;
  char *ptr;
  char *token;
  size_t token_length = 0;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(action_str);

  /*
    Get debug sync point name. Or a special command.
  */
  if (!(ptr = debug_sync_token(&token, &token_length, action_str))) {
    errmsg = "Missing synchronization point name";
    goto err;
  }

  /*
    If there is a second token, the first one is the sync point name.
  */
  if (*ptr) {
    /* Get an action object to collect the requested action parameters. */
    action = debug_sync_get_action(thd, token, token_length);
    if (!action) {
      /* Error message is sent. */
      return true; /* purecov: tested */
    }
  }

  /*
    Get kind of action to be taken at sync point.
  */
  if (!(ptr = debug_sync_token(&token, &token_length, ptr))) {
    /* No action present. Try special commands. Token unchanged. */

    /*
      Try RESET.
    */
    if (!my_strcasecmp(system_charset_info, token, "RESET")) {
      /* It is RESET. Reset all actions and global signal. */
      debug_sync_reset(thd);
      goto end;
    }

    /* Token unchanged. It still contains sync point name. */
    errmsg = "Missing action after synchronization point name '%.*s'";
    goto err;
  }

  /*
    Check for pseudo actions first. Start with actions that work on
    an existing action.
  */
  DBUG_ASSERT(action);

  /*
    Try TEST.
  */
  if (!my_strcasecmp(system_charset_info, token, "TEST")) {
    /* It is TEST. Nothing must follow it. */
    if (*ptr) {
      errmsg = "Nothing must follow action TEST";
      goto err;
    }

    /* Execute sync point. */
    debug_sync(thd, action->sync_point.ptr(), action->sync_point.length());
    /* Fix statistics. This was not a real hit of the sync point. */
    thd->debug_sync_control->dsp_hits--;
    goto end;
  }

  /*
    Now check for actions that define a new action.
    Initialize action. Do not use memset(). Strings may have malloced.
  */
  action->activation_count = 0;
  action->hit_limit = 0;
  action->execute = 0;
  action->timeout = 0;
  action->signal.length(0);
  action->wait_for.length(0);

  /*
    Try CLEAR.
  */
  if (!my_strcasecmp(system_charset_info, token, "CLEAR")) {
    /* It is CLEAR. Nothing must follow it. */
    if (*ptr) {
      errmsg = "Nothing must follow action CLEAR";
      goto err;
    }

    /* Set (clear/remove) action. */
    goto set_action;
  }

  /*
    Now check for real sync point actions.
  */

  /*
    Try SIGNAL.
  */
  if (!my_strcasecmp(system_charset_info, token, "SIGNAL")) {
    /* It is SIGNAL. Signal name must follow. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) {
      errmsg = "Missing signal name after action SIGNAL";
      goto err;
    }
    if (action->signal.copy(token, token_length, system_charset_info)) {
      /* Error is reported by my_malloc(). */
      /* purecov: begin tested */
      errmsg = nullptr;
      goto err;
      /* purecov: end */
    }

    /* Set default for EXECUTE option. */
    action->execute = 1;

    /* Get next token. If none follows, set action. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) goto set_action;
  }

  /*
    Try WAIT_FOR.
  */
  if (!my_strcasecmp(system_charset_info, token, "WAIT_FOR")) {
    /* It is WAIT_FOR. Wait_for signal name must follow. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) {
      errmsg = "Missing signal name after action WAIT_FOR";
      goto err;
    }
    if (action->wait_for.copy(token, token_length, system_charset_info)) {
      /* Error is reported by my_malloc(). */
      /* purecov: begin tested */
      errmsg = nullptr;
      goto err;
      /* purecov: end */
    }

    /* Set default for EXECUTE and TIMEOUT options. */
    action->execute = 1;
    action->timeout = opt_debug_sync_timeout;
    action->clear_event = true;

    /* Get next token. If none follows, set action. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) goto set_action;

    /*
      Try TIMEOUT.
    */
    if (!my_strcasecmp(system_charset_info, token, "TIMEOUT")) {
      /* It is TIMEOUT. Number must follow. */
      if (!(ptr = debug_sync_number(&action->timeout, ptr))) {
        errmsg = "Missing valid number after TIMEOUT";
        goto err;
      }

      /* Get next token. If none follows, set action. */
      if (!(ptr = debug_sync_token(&token, &token_length, ptr)))
        goto set_action;
    }
  }

  /*
    Try EXECUTE.
  */
  if (!my_strcasecmp(system_charset_info, token, "EXECUTE")) {
    /*
      EXECUTE requires either SIGNAL and/or WAIT_FOR to be present.
      In this case action->execute has been preset to 1.
    */
    if (!action->execute) {
      errmsg = "Missing action before EXECUTE";
      goto err;
    }

    /* Number must follow. */
    if (!(ptr = debug_sync_number(&action->execute, ptr))) {
      errmsg = "Missing valid number after EXECUTE";
      goto err;
    }

    /* Get next token. If none follows, set action. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) goto set_action;
  }

  /*
    Try NO_CLEAR_EVENT.
  */
  if (!my_strcasecmp(system_charset_info, token, "NO_CLEAR_EVENT")) {
    action->clear_event = false;
    /* Get next token. If none follows, set action. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) goto set_action;
  }

  /*
    Try HIT_LIMIT.
  */
  if (!my_strcasecmp(system_charset_info, token, "HIT_LIMIT")) {
    /* Number must follow. */
    if (!(ptr = debug_sync_number(&action->hit_limit, ptr))) {
      errmsg = "Missing valid number after HIT_LIMIT";
      goto err;
    }

    /* Get next token. If none follows, set action. */
    if (!(ptr = debug_sync_token(&token, &token_length, ptr))) goto set_action;
  }

  errmsg = "Illegal or out of order stuff: '%.*s'";

err:
  if (errmsg) {
    /*
      NOTE: errmsg must either have %.*s or none % at all.
      It can be NULL if an error message is already reported
      (e.g. by my_malloc()).
    */
    token_length =
        std::min(token_length, size_t(64)); /* Limit error message length. */
    my_printf_error(ER_PARSE_ERROR, errmsg, MYF(0), token_length, token);
  }
  if (action) debug_sync_remove_action(thd->debug_sync_control, action);
  return true;

set_action:
  return debug_sync_set_action(thd, action);

end:
  return false;
}

/**
  Set the system variable 'debug_sync'.

  @param[in]    thd             thread handle
  @param[in]    val_str         set variable request

  @return       status
    @retval     false           ok, variable is set
    @retval     true            error, variable could not be set

  @note
    "Setting" of the system variable 'debug_sync' does not mean to
    assign a value to it as usual. Instead a debug sync action is parsed
    from the input string and stored apart from the variable value.

  @note
    For efficiency reasons, the action string parser places '\0'
    terminators in the string. So we need to take a copy here.
*/

bool debug_sync_update(THD *thd, char *val_str) {
  DBUG_TRACE;
  DBUG_PRINT("debug_sync", ("set action: '%s'", val_str));

  /*
    debug_sync_eval_action() places '\0' in the string, which itself
    must be '\0' terminated.
  */
  return opt_debug_sync_timeout ? debug_sync_eval_action(thd, val_str) : false;
}

/**
  Retrieve the value of the system variable 'debug_sync'.

  @param[in]    thd             thread handle

  @return       string
    @retval     != NULL         ok, string pointer
    @retval     NULL            memory allocation error

  @note
    The value of the system variable 'debug_sync' reflects if
    the facility is enabled ("ON") or disabled (default, "OFF").

    When "ON", the list of signals signalled are added separated by comma.
*/

uchar *debug_sync_value_ptr(THD *thd) {
  char *value;
  DBUG_TRACE;

  if (opt_debug_sync_timeout) {
    std::string signals_on("ON - signals: '");
    static char sep[] = ",";

    // Ensure exclusive access to debug_sync_global.ds_signal_set
    mysql_mutex_lock(&debug_sync_global.ds_mutex);

    signal_event_set::const_iterator iter;
    for (iter = debug_sync_global.ds_signal_set.begin();
         iter != debug_sync_global.ds_signal_set.end();) {
      signals_on.append(*iter);
      if ((++iter) != debug_sync_global.ds_signal_set.end())
        signals_on.append(sep);
    }
    signals_on.append("'");

    const char *c_str = signals_on.c_str();
    const size_t lgt = strlen(c_str) + 1;

    if ((value = (char *)thd->mem_root->Alloc(lgt))) memcpy(value, c_str, lgt);

    mysql_mutex_unlock(&debug_sync_global.ds_mutex);
  } else {
    /* purecov: begin tested */
    value = const_cast<char *>("OFF");
    /* purecov: end */
  }

  return (uchar *)value;
}

/**
  Return true if the signal is found in global signal list.

  @param signal_name Signal name identifying the signal.

  @note
    If signal is found in the global signal set, it means that the
    signal thread has signalled to the waiting thread. This method
    must be called with the debug_sync_global.ds_mutex held.

  @retval true  if signal is found in the global signal list.
  @retval false otherwise.
*/

static inline bool is_signalled(const std::string *signal_name) {
  return (debug_sync_global.ds_signal_set.find(*signal_name) !=
          debug_sync_global.ds_signal_set.end());
}

/**
  Return false if signal has been added to global signal list.

  @param signal_name signal name that is to be added to the global signal
         list.

  @note
    This method add signal name to the global signal list and signals
    the waiting thread that this signal has been emitted. This method
    must be called with the debug_sync_global.ds_mutex held.
*/

static inline void add_signal_event(const std::string *signal_name) {
  debug_sync_global.ds_signal_set.insert(*signal_name);
}

/**
  Remove the signal from the global signal list.

  @param signal_name signal name to be removed from the global signal list.

  @note
    This method erases the signal from the signal list.  This happens
    when the wait thread has processed the signal event from the
    signalling thread. This method should be called with the
    debug_sync_global.ds_mutex held.
*/
static inline void clear_signal_event(const std::string *signal_name) {
  debug_sync_global.ds_signal_set.erase(*signal_name);
}

/**
  Execute requested action at a synchronization point.

  @param[in]    thd                 thread handle
  @param[in]    action              action to be executed

  @note
    This is to be called only if activation count > 0.
*/

static void debug_sync_execute(THD *thd, st_debug_sync_action *action) {
#ifndef DBUG_OFF
  const char *dsp_name = action->sync_point.c_ptr();
  const char *sig_emit = action->signal.c_ptr();
  const char *sig_wait = action->wait_for.c_ptr();
#endif
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(action);
  DBUG_PRINT("debug_sync",
             ("sync_point: '%s'  activation_count: %lu  hit_limit: %lu  "
              "execute: %lu  timeout: %lu  signal: '%s'  wait_for: '%s'",
              dsp_name, action->activation_count, action->hit_limit,
              action->execute, action->timeout, sig_emit, sig_wait));

  DBUG_ASSERT(action->activation_count);
  action->activation_count--;

  if (action->execute) {
    const char *old_proc_info = nullptr;

    action->execute--;

    /*
      If we will be going to wait, set proc_info for the PROCESSLIST table.
      Do this before emitting the signal, so other threads can see it
      if they awake before we enter_cond() below.
    */
    if (action->wait_for.length()) {
      st_debug_sync_control *ds_control = thd->debug_sync_control;
      strxnmov(ds_control->ds_proc_info, sizeof(ds_control->ds_proc_info) - 1,
               "debug sync point: ", action->sync_point.c_ptr(), NullS);
      old_proc_info = thd->proc_info;
      debug_sync_thd_proc_info(thd, ds_control->ds_proc_info);
    }

    /*
      Take mutex to ensure that only one thread access
      debug_sync_global.ds_signal_set at a time.  Need to take mutex for
      read access too, to create a memory barrier in order to avoid that
      threads just reads an old cached version of the signal.
    */
    mysql_mutex_lock(&debug_sync_global.ds_mutex);

    if (action->signal.length()) {
      std::string signal = action->signal.ptr();
      std::vector<std::string> signals;
      boost::split(signals, signal, boost::is_any_of(","));
      for (std::vector<std::string>::const_iterator it = signals.begin();
           it != signals.end(); ++it) {
        /* Copy the signal to the global set. */
        std::string s = *it;
        boost::trim(s);
        if (!s.empty()) add_signal_event(&s);
      }
      /* Wake threads waiting in a sync point. */
      mysql_cond_broadcast(&debug_sync_global.ds_cond);
      DBUG_PRINT("debug_sync_exec",
                 ("signal '%s'  at: '%s'", sig_emit, dsp_name));
    } /* end if (action->signal.length()) */

    if (action->wait_for.length()) {
      mysql_mutex_t *old_mutex;
      mysql_cond_t *old_cond = nullptr;
      int error = 0;
      struct timespec abstime;
      std::string wait_for = action->wait_for.ptr();

      /*
        We don't use enter_cond()/exit_cond(). They do not save old
        mutex and cond. This would prohibit the use of DEBUG_SYNC
        between other places of enter_cond() and exit_cond().

        Note that we cannot lock LOCK_current_cond here. See comment
        in THD::enter_cond().
      */
      old_mutex = thd->current_mutex;
      old_cond = thd->current_cond;
      thd->current_mutex = &debug_sync_global.ds_mutex;
      thd->current_cond = &debug_sync_global.ds_cond;

      set_timespec(&abstime, action->timeout);
      DBUG_EXECUTE("debug_sync_exec", {
        DBUG_PRINT("debug_sync_exec",
                   ("wait for '%s'  at: '%s'", sig_wait, dsp_name));
      });
      /*
        Wait until global signal string matches the wait_for string.
        Interrupt when thread or query is killed or facility disabled.
        The facility can become disabled when some thread cannot get
        the required dynamic memory allocated.
      */
      while (!is_signalled(&wait_for) && !thd->killed &&
             opt_debug_sync_timeout) {
        error = mysql_cond_timedwait(&debug_sync_global.ds_cond,
                                     &debug_sync_global.ds_mutex, &abstime);

        DBUG_EXECUTE("debug_sync", {
          /* Functions as DBUG_PRINT args can change keyword and line nr. */
          DBUG_PRINT("debug_sync",
                     ("awoke from %s error: %d", sig_wait, error));
        });

        if (is_timeout(error)) {
          // We should not make the statement fail, even if in strict mode.
          push_warning(thd, Sql_condition::SL_WARNING, ER_DEBUG_SYNC_TIMEOUT,
                       ER_THD(thd, ER_DEBUG_SYNC_TIMEOUT));
          DBUG_EXECUTE_IF("debug_sync_abort_on_timeout", DBUG_ABORT(););
          break;
        }
        error = 0;
      }
      if (action->clear_event) clear_signal_event(&wait_for);

      DBUG_EXECUTE(
          "debug_sync_exec",
          if (thd->killed) DBUG_PRINT("debug_sync_exec",
                                      ("killed %d from '%s'  at: '%s'",
                                       thd->killed.load(), sig_wait, dsp_name));
          else DBUG_PRINT("debug_sync_exec",
                          ("%s from '%s'  at: '%s'",
                           error ? "timeout" : "resume", sig_wait, dsp_name)););

      /*
        Restore current mutex/cond information to how it was before DEBUG_SYNC.
        We don't use enter_cond()/exit_cond(). They do not save old
        mutex and cond. This would prohibit the use of DEBUG_SYNC
        between other places of enter_cond() and exit_cond(). The
        protected mutex must always unlocked _before_ mysys_var->mutex
        is locked. (See comment in THD::exit_cond().)
      */
      mysql_mutex_unlock(&debug_sync_global.ds_mutex);
      mysql_mutex_lock(&thd->LOCK_current_cond);
      thd->current_mutex = old_mutex;
      thd->current_cond = old_cond;
      mysql_mutex_unlock(&thd->LOCK_current_cond);
      debug_sync_thd_proc_info(thd, old_proc_info);
    } else {
      /* In case we don't wait, we just release the mutex. */
      mysql_mutex_unlock(&debug_sync_global.ds_mutex);
    } /* end if (action->wait_for.length()) */

  } /* end if (action->execute) */

  /* hit_limit is zero for infinite. Don't decrement unconditionally. */
  if (action->hit_limit) {
    if (!--action->hit_limit) {
      thd->killed = THD::KILL_QUERY;
      my_error(ER_DEBUG_SYNC_HIT_LIMIT, MYF(0));
    }
    DBUG_PRINT("debug_sync_exec",
               ("hit_limit: %lu  at: '%s'", action->hit_limit, dsp_name));
  }
}

/**
  Execute requested action at a synchronization point.

  @param[in]     thd                thread handle
  @param[in]     sync_point_name    name of synchronization point
  @param[in]     name_len           length of sync point name
*/

void debug_sync(THD *thd, const char *sync_point_name, size_t name_len) {
  if (!thd) {
    return;
  }

  st_debug_sync_control *ds_control = thd->debug_sync_control;
  st_debug_sync_action *action;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(sync_point_name);
  DBUG_ASSERT(name_len);
  DBUG_ASSERT(ds_control);
  DBUG_PRINT("debug_sync_point", ("hit: '%s'", sync_point_name));

  /* Statistics. */
  ds_control->dsp_hits++;

  if (ds_control->ds_active &&
      (action = debug_sync_find(ds_control->ds_action, ds_control->ds_active,
                                sync_point_name, name_len)) &&
      action->activation_count) {
    /* Sync point is active (action exists). */
    debug_sync_execute(thd, action);

    /* Statistics. */
    ds_control->dsp_executed++;

    /* If action became inactive, remove it to shrink the search array. */
    if (!action->activation_count) debug_sync_remove_action(ds_control, action);
  }
}

/**
  Define debug sync action.

  @param[in]        thd             thread handle
  @param[in]        action_str      action string

  @return           status
    @retval         false           ok
    @retval         true            error

  @description
    The function is similar to @c debug_sync_eval_action but is
    to be called immediately from the server code rather than
    to be triggered by setting a value to DEBUG_SYNC system variable.

  @note
    The input string is copied prior to be fed to
    @c debug_sync_eval_action to let the latter modify it.

    Caution.
    The function allocates in THD::mem_root and therefore
    is not recommended to be deployed inside big loops.
*/

bool debug_sync_set_action(THD *thd, const char *action_str, size_t len) {
  bool rc;
  char *value;
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  DBUG_ASSERT(action_str);

  value = strmake_root(thd->mem_root, action_str, len);
  rc = debug_sync_eval_action(thd, value);
  return rc;
}

void conditional_sync_point(std::string name) {
  DBUG_EXECUTE_IF(("syncpoint_" + name).c_str(), {
    std::string act =
        "now SIGNAL reached_" + name + " WAIT_FOR continue_" + name;
    DBUG_ASSERT(!debug_sync_set_action(current_thd, act.c_str(), act.length()));
  });
}

void conditional_sync_point_for_timestamp(std::string name) {
  conditional_sync_point(name + "_" +
                         std::to_string(current_thd->start_time.tv_sec));
}

#endif /* defined(ENABLED_DEBUG_SYNC) */
