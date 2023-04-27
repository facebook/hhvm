/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/event_queue.h"

#include <stdio.h>
#include <atomic>
#include <memory>

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_systime.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // Auto_releaser
#include "sql/event_db_repository.h"         // Event_db_repository
#include "sql/events.h"                      // Events
#include "sql/lock.h"                        // lock_object_name
#include "sql/log.h"                         // log_*()
#include "sql/malloc_allocator.h"
#include "sql/mdl.h"
#include "sql/psi_memory_key.h"  // key_memory_Event_scheduler_scheduler_param
#include "sql/sql_audit.h"       // mysql_audit_release
#include "sql/sql_class.h"       // THD
#include "sql/sql_lex.h"
#include "sql/sql_table.h"  // write_bin_log
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit*, trans_rollback*
#include "sql/tztime.h"       // my_tz_OFFSET0
#include "sql_string.h"
#include "thr_mutex.h"

/**
  @addtogroup Event_Scheduler
  @{
*/

#define EVENT_QUEUE_INITIAL_SIZE 30

#define LOCK_QUEUE_DATA() lock_data(__func__, __LINE__)
#define UNLOCK_QUEUE_DATA() unlock_data(__func__, __LINE__)

/*
  Constructor of class Event_queue.

  SYNOPSIS
    Event_queue::Event_queue()
*/

Event_queue::Event_queue()
    : queue(Event_queue_less(),
            Malloc_allocator<Event_queue_element *>(
                key_memory_Event_scheduler_scheduler_param)),
      next_activation_at(0),
      mutex_last_locked_at_line(0),
      mutex_last_unlocked_at_line(0),
      mutex_last_attempted_lock_at_line(0),
      mutex_last_locked_in_func("n/a"),
      mutex_last_unlocked_in_func("n/a"),
      mutex_last_attempted_lock_in_func("n/a"),
      mutex_queue_data_locked(false),
      mutex_queue_data_attempting_lock(false),
      waiting_on_cond(false) {
  mysql_mutex_init(key_LOCK_event_queue, &LOCK_event_queue, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_queue_state, &COND_queue_state);
}

Event_queue::~Event_queue() {
  deinit_queue();
  mysql_mutex_destroy(&LOCK_event_queue);
  mysql_cond_destroy(&COND_queue_state);
}

/*
  This is a queue's constructor. Until this method is called, the
  queue is unusable.  We don't use a C++ constructor instead in
  order to be able to check the return value. The queue is
  initialized once at server startup.  Initialization can fail in
  case of a failure reading events from the database or out of
  memory.

  SYNOPSIS
    Event_queue::init()

  RETURN VALUE
    false  OK
    true   Error
*/

bool Event_queue::init_queue() {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("this: %p", this));

  LOCK_QUEUE_DATA();

  if (queue.reserve(EVENT_QUEUE_INITIAL_SIZE)) {
    LogErr(ERROR_LEVEL, ER_EVENT_CANT_INIT_QUEUE);
    goto err;
  }

  UNLOCK_QUEUE_DATA();
  return false;

err:
  UNLOCK_QUEUE_DATA();
  return true;
}

/*
  Deinits the queue. Remove all elements from it and destroys them
  too.

  SYNOPSIS
    Event_queue::deinit_queue()
*/

void Event_queue::deinit_queue() {
  DBUG_TRACE;

  LOCK_QUEUE_DATA();
  empty_queue();
  UNLOCK_QUEUE_DATA();
}

/**
  Adds an event to the queue.

  Compute the next execution time for an event, and if it is still
  active, add it to the queue.
  The object is left intact in case of an error. Otherwise
  the queue container assumes ownership of it.

  @param[in]  thd      thread handle
  @param[in]  new_element a new element to add to the queue
  @param[out] created  set to true if no error and the element is
                       added to the queue, false otherwise

  @retval true  an error occurred. The value of created is undefined,
                the element was not deleted.
  @retval false success
*/

bool Event_queue::create_event(THD *thd, Event_queue_element *new_element,
                               bool *created) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p et=%s.%s", thd, new_element->m_schema_name.str,
                       new_element->m_event_name.str));

  /* Will do nothing if the event is disabled */
  new_element->compute_next_execution_time(thd);
  if (new_element->m_status != Event_parse_data::ENABLED) {
    *created = false;
    return false;
  }

  DBUG_PRINT("info", ("new event in the queue: %p", new_element));

  LOCK_QUEUE_DATA();
  *created = (queue.push(new_element) == false);
  dbug_dump_queue(thd->query_start_in_secs());
  mysql_cond_broadcast(&COND_queue_state);
  UNLOCK_QUEUE_DATA();

  return !*created;
}

/*
  Updates an event from the scheduler queue

  SYNOPSIS
    Event_queue::update_event()
      thd        Thread
      dbname     Schema of the event
      name       Name of the event
      new_schema New schema, in case of RENAME TO, otherwise NULL
      new_name   New name, in case of RENAME TO, otherwise NULL
*/

void Event_queue::update_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name,
                               Event_queue_element *new_element) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p  et=[%s.%s]", thd, dbname.str, name.str));

  if ((new_element->m_status == Event_parse_data::DISABLED) ||
      (new_element->m_status == Event_parse_data::SLAVESIDE_DISABLED)) {
    DBUG_PRINT("info", ("The event is disabled."));
    /*
      Destroy the object but don't skip to end: because we may have to remove
      object from the cache.
    */
    delete new_element;
    new_element = nullptr;
  } else
    new_element->compute_next_execution_time(thd);

  LOCK_QUEUE_DATA();
  find_n_remove_event(dbname, name);

  /* If not disabled event */
  if (new_element) {
    DBUG_PRINT("info", ("new event in the queue: %p", new_element));
    queue.push(new_element);
    mysql_cond_broadcast(&COND_queue_state);
  }

  dbug_dump_queue(thd->query_start_in_secs());
  UNLOCK_QUEUE_DATA();
}

/*
  Drops an event from the queue

  SYNOPSIS
    Event_queue::drop_event()
      thd     Thread
      dbname  Schema of the event to drop
      name    Name of the event to drop
*/

void Event_queue::drop_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p  db :%s  name: %s", thd, dbname.str, name.str));

  LOCK_QUEUE_DATA();
  find_n_remove_event(dbname, name);
  dbug_dump_queue(thd->query_start_in_secs());
  UNLOCK_QUEUE_DATA();

  /*
    We don't signal here because the scheduler will catch the change
    next time it wakes up.
  */
}

/*
  Drops all events from the in-memory queue and disk that match
  certain pattern evaluated by a comparator function

  SYNOPSIS
    Event_queue::drop_matching_events()
      pattern        A pattern string
      comparator     The function to use for comparing

  RETURN VALUE
    >=0  Number of dropped events

  NOTE
    Expected is the caller to acquire lock on LOCK_event_queue
*/

void Event_queue::drop_matching_events(LEX_CSTRING pattern,
                                       bool (*comparator)(LEX_CSTRING,
                                                          Event_basic *)) {
  size_t i = 0;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("pattern=%s", pattern.str));

  while (i < queue.size()) {
    Event_queue_element *et = queue[i];
    DBUG_PRINT("info",
               ("[%s.%s]?", et->m_schema_name.str, et->m_event_name.str));
    if (comparator(pattern, et)) {
      /*
        The queue is ordered. If we remove an element, then all elements
        after it will shift one position to the left, if we imagine it as
        an array from left to the right. In this case we should not
        increment the counter and the (i < queue.elements) condition is ok.
      */
      queue.remove(i);
#ifdef HAVE_PSI_SP_INTERFACE
      /* Drop statistics for this stored program from performance schema. */
      MYSQL_DROP_SP(to_uint(enum_sp_type::EVENT), et->m_schema_name.str,
                    et->m_schema_name.length, et->m_event_name.str,
                    et->m_event_name.length);
#endif
      delete et;
    } else
      i++;
  }
  /*
    We don't call mysql_cond_broadcast(&COND_queue_state);
    If we remove the top event:
    1. The queue is empty. The scheduler will wake up at some time and
       realize that the queue is empty. If create_event() comes inbetween
       it will signal the scheduler
    2. The queue is not empty, but the next event after the previous top,
       won't be executed any time sooner than the element we removed. Hence,
       we may not notify the scheduler and it will realize the change when it
       wakes up from timedwait.
  */
}

/*
  Drops all events from the in-memory queue and disk that are from
  certain schema.

  SYNOPSIS
    Event_queue::drop_schema_events()
      schema    The schema name
*/

void Event_queue::drop_schema_events(LEX_CSTRING schema) {
  DBUG_TRACE;
  LOCK_QUEUE_DATA();
  drop_matching_events(schema, event_basic_db_equal);
  UNLOCK_QUEUE_DATA();
}

/*
  Searches for an event in the queue

  SYNOPSIS
    Event_queue::find_n_remove_event()
      db    The schema of the event to find
      name  The event to find

  NOTE
    The caller should do the locking also the caller is responsible for
    actual signalling in case an event is removed from the queue.
*/

void Event_queue::find_n_remove_event(LEX_CSTRING db, LEX_CSTRING name) {
  DBUG_TRACE;

  for (size_t i = 0; i < queue.size(); ++i) {
    Event_queue_element *et = queue[i];
    DBUG_PRINT("info", ("[%s.%s]==[%s.%s]?", db.str, name.str,
                        et->m_schema_name.str, et->m_event_name.str));
    if (event_basic_identifier_equal(db, name, et)) {
      queue.remove(i);
      delete et;
      break;
    }
  }
}

/*
  Recalculates activation times in the queue. There is one reason for
  that. Because the values (execute_at) by which the queue is ordered are
  changed by calls to compute_next_execution_time() on a request from the
  scheduler thread, if it is not running then the values won't be updated.
  Once the scheduler is started again the values has to be recalculated
  so they are right for the current time.

  SYNOPSIS
    Event_queue::recalculate_activation_times()
      thd  Thread
*/

void Event_queue::recalculate_activation_times(THD *thd) {
  DBUG_TRACE;

  LOCK_QUEUE_DATA();
  DBUG_PRINT("info", ("%u loaded events to be recalculated",
                      static_cast<unsigned>(queue.size())));
  for (size_t i = 0; i < queue.size(); i++) {
    queue[i]->compute_next_execution_time(thd);
  }
  queue.build_heap();

  /*
    Prevent InnoDB from automatically committing the InnoDB transaction after
    updating the data-dictionary table.
  */
  Disable_autocommit_guard autocommit_guard(thd);

  /*
    The disabled elements are moved to the end during the `fix`.
    Start from the end and remove all of the elements which are
    disabled. When we find the first non-disabled one we break, as we
    have removed all. The queue has been ordered in a way the disabled
    events are at the end.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  Implicit_substatement_state_guard substatement_guard(thd);
  std::vector<std::unique_ptr<Event_queue_element>> events_to_drop;
  for (size_t i = queue.size(); i > 0; i--) {
    Event_queue_element *element = queue[i - 1];
    if (element->m_status != Event_parse_data::DISABLED) break;
    /*
      This won't cause queue re-order, because we remove
      always the last element.
    */
    queue.remove(i - 1);
    /*
      Dropping the event from Data Dictionary.
    */
    if (element->m_dropped)
      events_to_drop.push_back(std::unique_ptr<Event_queue_element>(element));
    else
      delete element;
  }
  UNLOCK_QUEUE_DATA();

  for (const auto &element : events_to_drop) {
    bool ret;
    bool event_exists;

    if (lock_object_name(thd, MDL_key::EVENT, element->m_schema_name.str,
                         element->m_event_name.str)) {
      continue;
    }
    if (!(ret = Event_db_repository::drop_event(thd, element->m_schema_name,
                                                element->m_event_name, false,
                                                &event_exists))) {
      String sp_sql;
      if ((ret = construct_drop_event_sql(thd, &sp_sql, element->m_schema_name,
                                          element->m_event_name))) {
        LogErr(WARNING_LEVEL, ER_FAILED_TO_CONSTRUCT_DROP_EVENT_QUERY);
      } else {
        // Write drop event to bin log.
        thd->add_to_binlog_accessed_dbs(element->m_schema_name.str);
        if ((ret = write_bin_log(thd, true, sp_sql.c_ptr_safe(),
                                 sp_sql.length(), event_exists))) {
          LogErr(WARNING_LEVEL, ER_FAILED_TO_BINLOG_DROP_EVENT,
                 element->m_schema_name.str, element->m_event_name.str);
        }
      }
    }

    if (!ret)
      ret = trans_commit_stmt(thd) || trans_commit(thd);
    else {
      trans_rollback_stmt(thd);
      trans_rollback(thd);
    }
  }
  // Release locks taken before drop_event()
  thd->mdl_context.release_transactional_locks();

  /*
    XXX: The events are dropped only from memory and not from disk
         even if `drop_list[j]->dropped` is true. There will be still on the
         disk till next server restart.
         Please add code here to do it.
  */
}

/*
  Empties the queue and destroys the Event_queue_element objects in the
  queue.

  SYNOPSIS
    Event_queue::empty_queue()

  NOTE
    Should be called with LOCK_event_queue locked
*/

void Event_queue::empty_queue() {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("Purging the queue. %u element(s)",
                       static_cast<unsigned>(queue.size())));
  LogErr(INFORMATION_LEVEL, ER_EVENT_PURGING_QUEUE,
         static_cast<unsigned>(queue.size()));
  /* empty the queue */
  queue.delete_elements();
}

/*
  Dumps the queue to the trace log.

  SYNOPSIS
    Event_queue::dbug_dump_queue()
      now  Current timestamp
*/

void Event_queue::dbug_dump_queue(time_t now MY_ATTRIBUTE((unused))) {
#ifndef DBUG_OFF
  DBUG_TRACE;
  DBUG_PRINT("info", ("Dumping queue . Elements=%u",
                      static_cast<unsigned>(queue.size())));
  for (size_t i = 0; i < queue.size(); i++) {
    Event_queue_element *et = queue[i];
    DBUG_PRINT("info", ("et: %p  name: %s.%s", et, et->m_schema_name.str,
                        et->m_event_name.str));
    DBUG_PRINT(
        "info",
        ("exec_at: %lu  starts: %lu  ends: %lu  execs_so_far: %u  "
         "expr: %ld  et.exec_at: %ld  now: %ld  "
         "(et.exec_at - now): %d  if: %d",
         (long)et->m_execute_at, (long)et->m_starts, (long)et->m_ends,
         et->m_execution_count, (long)et->m_expression, (long)et->m_execute_at,
         (long)now, (int)(et->m_execute_at - now), et->m_execute_at <= now));
  }
  return;
#endif
}

/*
  Checks whether the top of the queue is elligible for execution and
  returns an Event_job_data instance in case it should be executed.
  `now` is compared against `execute_at` of the top element in the queue.

  SYNOPSIS
    Event_queue::get_top_for_execution_if_time()
      thd        [in]  Thread
      event_name [out] The object to execute

  RETURN VALUE
    false  No error. event_name != NULL
    true   Serious error
*/

bool Event_queue::get_top_for_execution_if_time(
    THD *thd, Event_queue_element_for_exec **event_name) {
  bool ret = false;
  *event_name = nullptr;
  my_time_t last_executed = 0;
  int status = 0;
  DBUG_TRACE;

  LOCK_QUEUE_DATA();
  for (;;) {
    Event_queue_element *top = nullptr;

    /* Break loop if thd has been killed */
    if (thd->killed) {
      DBUG_PRINT("info", ("thd->killed=%d", thd->killed.load()));
      goto end;
    }

    if (queue.empty()) {
      /* There are no events in the queue */
      next_activation_at = 0;

      /* Release any held audit resources before waiting */
      mysql_audit_release(thd);

      /* Wait on condition until signaled. Release LOCK_queue while waiting. */
      cond_wait(thd, nullptr, &stage_waiting_on_empty_queue, __func__, __FILE__,
                __LINE__);

      continue;
    }

    top = queue.top();

    thd->set_time(); /* Get current time */

    next_activation_at = top->m_execute_at;
    if (next_activation_at > thd->query_start_in_secs()) {
      /*
        Not yet time for top event, wait on condition with
        time or until signaled. Release LOCK_queue while waiting.
      */
      struct timespec top_time;
      set_timespec(&top_time, next_activation_at - thd->query_start_in_secs());

      /* Release any held audit resources before waiting */
      mysql_audit_release(thd);

      cond_wait(thd, &top_time, &stage_waiting_for_next_activation, __func__,
                __FILE__, __LINE__);

      continue;
    }
    if (!(*event_name = new Event_queue_element_for_exec()) ||
        (*event_name)->init(top->m_schema_name, top->m_event_name)) {
      ret = true;
      break;
    }

    DBUG_PRINT("info", ("Ready for execution"));
    top->mark_last_executed(thd);
    if (top->compute_next_execution_time(thd))
      top->m_status = Event_parse_data::DISABLED;
    DBUG_PRINT("info",
               ("event %s status is %d", top->m_event_name.str, top->m_status));

    top->m_execution_count++;
    (*event_name)->dropped = top->m_dropped;
    /*
      Save new values of last_executed timestamp and event status on stack
      in order to be able to update event description in system table once
      QUEUE_DATA lock is released.
    */
    last_executed = top->m_last_executed;
    status = top->m_status;

    if (top->m_status == Event_parse_data::DISABLED) {
      DBUG_PRINT("info", ("removing from the queue"));
      LogErr(INFORMATION_LEVEL, ER_EVENT_LAST_EXECUTION, top->m_schema_name.str,
             top->m_event_name.str, top->m_dropped ? "Dropping." : "");
      delete top;
      queue.pop();
      /*
       This event will get dropped from mysql.events table in
       Event_job_data::execute() function eventually.
       So no need add check to drop it from mysql.events table here.
      */
    } else
      queue.update_top();

    dbug_dump_queue(thd->query_start_in_secs());
    break;
  }
end:
  UNLOCK_QUEUE_DATA();

  DBUG_PRINT("info", ("returning %d  et_new: %p ", ret, *event_name));

  if (*event_name) {
    DBUG_PRINT("info", ("db: %s  name: %s", (*event_name)->dbname.str,
                        (*event_name)->name.str));

    // Acquire exclusive MDL lock on the event and it's parent schema.
    if (lock_object_name(thd, MDL_key::EVENT, (*event_name)->dbname.str,
                         (*event_name)->name.str))
      return true;

    (void)Event_db_repository::update_timing_fields_for_event(
        thd, (*event_name)->dbname, (*event_name)->name, last_executed,
        (ulonglong)status);

    thd->mdl_context.release_transactional_locks();
  }

  return ret;
}

/*
  Auxiliary function for locking LOCK_event_queue. Used by the
  LOCK_QUEUE_DATA macro

  SYNOPSIS
    Event_queue::lock_data()
      func  Which function is requesting mutex lock
      line  On which line mutex lock is requested
*/

void Event_queue::lock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mutex_last_attempted_lock_in_func = func;
  mutex_last_attempted_lock_at_line = line;
  mutex_queue_data_attempting_lock = true;
  mysql_mutex_lock(&LOCK_event_queue);
  mutex_last_attempted_lock_in_func = "";
  mutex_last_attempted_lock_at_line = 0;
  mutex_queue_data_attempting_lock = false;

  mutex_last_locked_in_func = func;
  mutex_last_locked_at_line = line;
  mutex_queue_data_locked = true;
}

/*
  Auxiliary function for unlocking LOCK_event_queue. Used by the
  UNLOCK_QUEUE_DATA macro

  SYNOPSIS
    Event_queue::unlock_data()
      func  Which function is requesting mutex unlock
      line  On which line mutex unlock is requested
*/

void Event_queue::unlock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mutex_last_unlocked_at_line = line;
  mutex_queue_data_locked = false;
  mutex_last_unlocked_in_func = func;
  mysql_mutex_unlock(&LOCK_event_queue);
}

/*
  Wrapper for mysql_cond_wait/timedwait

  SYNOPSIS
    Event_queue::cond_wait()
      thd     Thread (Could be NULL during shutdown procedure)
      msg     Message for thd->proc_info
      abstime If not null then call mysql_cond_timedwait()
      func    Which function is requesting cond_wait
      line    On which line cond_wait is requested
*/

void Event_queue::cond_wait(THD *thd, struct timespec *abstime,
                            const PSI_stage_info *stage, const char *src_func,
                            const char *src_file, uint src_line) {
  DBUG_TRACE;
  waiting_on_cond = true;
  mutex_last_unlocked_at_line = src_line;
  mutex_queue_data_locked = false;
  mutex_last_unlocked_in_func = src_func;

  thd->enter_cond(&COND_queue_state, &LOCK_event_queue, stage, nullptr,
                  src_func, src_file, src_line);

  if (!thd->killed) {
    if (!abstime)
      mysql_cond_wait(&COND_queue_state, &LOCK_event_queue);
    else
      mysql_cond_timedwait(&COND_queue_state, &LOCK_event_queue, abstime);
  }

  mutex_last_locked_in_func = src_func;
  mutex_last_locked_at_line = src_line;
  mutex_queue_data_locked = true;
  waiting_on_cond = false;

  /*
    Need to unlock before exit_cond, so we need to relock.
    Not the best thing to do but we need to obey cond_wait()
  */
  unlock_data(src_func, src_line);
  thd->exit_cond(nullptr, src_func, src_file, src_line);
  lock_data(src_func, src_line);
}

/*
  Dumps the internal status of the queue

  SYNOPSIS
    Event_queue::dump_internal_status()
*/

void Event_queue::dump_internal_status() {
  DBUG_TRACE;

  /* element count */
  puts("");
  puts("Event queue status:");
  printf("Element count   : %u\n", static_cast<unsigned>(queue.size()));
  printf("Data locked     : %s\n", mutex_queue_data_locked ? "YES" : "NO");
  printf("Attempting lock : %s\n",
         mutex_queue_data_attempting_lock ? "YES" : "NO");
  printf("LLA             : %s:%u\n", mutex_last_locked_in_func,
         mutex_last_locked_at_line);
  printf("LUA             : %s:%u\n", mutex_last_unlocked_in_func,
         mutex_last_unlocked_at_line);
  if (mutex_last_attempted_lock_at_line)
    printf("Last lock attempt at: %s:%u\n",
           mutex_last_attempted_lock_in_func.load(),
           mutex_last_attempted_lock_at_line.load());
  printf("WOC             : %s\n", waiting_on_cond ? "YES" : "NO");

  MYSQL_TIME time;
  my_tz_OFFSET0->gmt_sec_to_TIME(&time, next_activation_at);
  if (time.year != 1970)
    printf("Next activation : %04d-%02d-%02d %02d:%02d:%02d\n", time.year,
           time.month, time.day, time.hour, time.minute, time.second);
  else
    printf("Next activation : never");
}

/**
  @} (End of group Event_Scheduler)
*/
