/* Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/event_scheduler.h"

#include <stdio.h>
#include <string.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_command.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/thread_type.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/dd/dd_schema.h"  // dd::Schema_MDL_locker
#include "sql/dd/types/event.h"
#include "sql/event_data_objects.h"
#include "sql/event_db_repository.h"
#include "sql/event_queue.h"
#include "sql/events.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"              // my_localhost slave_net_timeout
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol_classic.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"  // Sql_condition
#include "sql/system_variables.h"
#include "sql_string.h"
#include "thr_mutex.h"

/**
  @addtogroup Event_Scheduler
  @{
*/

#define LOCK_DATA() lock_data(__func__, __LINE__)
#define UNLOCK_DATA() unlock_data(__func__, __LINE__)
#define COND_STATE_WAIT(mythd, abstime, stage) \
  cond_wait(mythd, abstime, stage, __func__, __FILE__, __LINE__)

extern my_thread_attr_t connection_attrib;

static const LEX_CSTRING scheduler_states_names[] = {
    {STRING_WITH_LEN("INITIALIZED")},
    {STRING_WITH_LEN("RUNNING")},
    {STRING_WITH_LEN("STOPPING")}};

struct scheduler_param {
  THD *thd;
  Event_scheduler *scheduler;
};

/*
  Prints the stack of infos, warnings, errors from thd to
  the console so it can be fetched by the logs-into-tables and
  checked later.

  SYNOPSIS
    evex_print_warnings
      thd  Thread used during the execution of the event
      et   The event itself
*/

void Event_worker_thread::print_warnings(THD *thd, Event_job_data *et) {
  const Sql_condition *err;
  enum loglevel ll;

  DBUG_TRACE;
  if (thd->get_stmt_da()->cond_count() == 0) return;

  char msg_buf[10 * STRING_BUFFER_USUAL_SIZE];
  char prefix_buf[5 * STRING_BUFFER_USUAL_SIZE];
  String prefix(prefix_buf, sizeof(prefix_buf), system_charset_info);
  prefix.length(0);
  prefix.append("Event Scheduler: [");

  prefix.append(et->m_definer.str, et->m_definer.length, system_charset_info);
  prefix.append("][", 2);
  prefix.append(et->m_schema_name.str, et->m_schema_name.length,
                system_charset_info);
  prefix.append('.');
  prefix.append(et->m_event_name.str, et->m_event_name.length,
                system_charset_info);
  prefix.append("] ", 2);

  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  while ((err = it++)) {
    String err_msg(msg_buf, sizeof(msg_buf), system_charset_info);
    /* set it to 0 or we start adding at the end. That's the trick ;) */
    err_msg.length(0);
    err_msg.append(prefix);
    err_msg.append(err->message_text(), err->message_octet_length(),
                   system_charset_info);
    switch (err->severity()) {
      case Sql_condition::SL_ERROR:
        ll = ERROR_LEVEL;
        break;
      case Sql_condition::SL_WARNING:
        ll = WARNING_LEVEL;
        break;
      case Sql_condition::SL_NOTE:
        ll = INFORMATION_LEVEL;
        break;
      default:
        ll = ERROR_LEVEL;
        DBUG_ASSERT(false);
    }
    LogErr(ll, ER_EVENT_MESSAGE_STACK, static_cast<int>(err_msg.length()),
           err_msg.c_ptr());
  }
}

/*
  Performs post initialization of structures in a new thread.

  SYNOPSIS
    post_init_event_thread()
      thd  Thread

  NOTES
      Before this is called, one should not do any DBUG_XXX() calls.

*/

bool post_init_event_thread(THD *thd) {
  if (my_thread_init()) {
    return true;
  }
  thd->store_globals();

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->add_thd(thd);
  thd_manager->inc_thread_running();
  return false;
}

/*
  Cleans up the THD and the threaded environment of the thread.

  SYNOPSIS
    deinit_event_thread()
      thd  Thread
*/

void deinit_event_thread(THD *thd) {
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();

  thd->proc_info = "Clearing";
  thd->get_protocol_classic()->end_net();
  DBUG_PRINT("exit", ("Event thread finishing"));
  thd->release_resources();
  thd_manager->remove_thd(thd);
  thd_manager->dec_thread_running();
  delete thd;
}

/*
  Performs pre- mysql_thread_create() initialisation of THD. Do this
  in the thread that will pass THD to the child thread. In the
  child thread call post_init_event_thread().

  SYNOPSIS
    pre_init_event_thread()
      thd  The THD of the thread. Has to be allocated by the caller.

  NOTES
    1. The host of the thead is my_localhost
    2. thd->net is initted with NULL - no communication.
*/

void pre_init_event_thread(THD *thd) {
  DBUG_TRACE;
  thd->security_context()->set_master_access(0);
  thd->security_context()->cache_current_db_access(0);
  thd->security_context()->set_host_or_ip_ptr(my_localhost,
                                              strlen(my_localhost));
  thd->get_protocol_classic()->init_net(nullptr);
  thd->security_context()->set_user_ptr(STRING_WITH_LEN("event_scheduler"));
  thd->get_protocol_classic()->get_net()->read_timeout =
      timeout_from_seconds(slave_net_timeout);
  thd->slave_thread = false;
  thd->variables.option_bits |= OPTION_AUTO_IS_NULL;
  thd->get_protocol_classic()->set_client_capabilities(CLIENT_MULTI_RESULTS);

  thd->set_new_thread_id();
  /*
    Guarantees that we will see the thread in SHOW PROCESSLIST though its
    vio is NULL.
  */

  thd->proc_info = "Initialized";
  thd->set_time();

  /* Do not use user-supplied timeout value for system threads. */
  thd->variables.lock_wait_timeout_nsec = LONG_TIMEOUT_NSEC;
}

/*
  Function that executes the scheduler,

  SYNOPSIS
    event_scheduler_thread()
      arg  Pointer to `struct scheduler_param`

  RETURN VALUE
    0  OK
*/

extern "C" {
static void *event_scheduler_thread(void *arg) {
  /* needs to be first for thread_stack */
  THD *thd = ((struct scheduler_param *)arg)->thd;
  Event_scheduler *scheduler = ((struct scheduler_param *)arg)->scheduler;
  bool res;

  thd->thread_stack = (char *)&thd;  // remember where our stack is

  mysql_thread_set_psi_id(thd->thread_id());

  res = post_init_event_thread(thd);

  {
    DBUG_TRACE;
    my_claim(arg);
    thd->claim_memory_ownership();
    my_free(arg);
    if (!res)
      scheduler->run(thd);
    else {
      thd->proc_info = "Clearing";
      thd->get_protocol_classic()->end_net();
      delete thd;
    }

  }  // Against gcc warnings
  my_thread_end();
  return nullptr;
}

/**
  Function that executes an event in a child thread. Setups the
  environment for the event execution and cleans after that.

  SYNOPSIS
    event_worker_thread()
      arg  The Event_job_data object to be processed

  RETURN VALUE
    0  OK
*/

static void *event_worker_thread(void *arg) {
  THD *thd;
  Event_queue_element_for_exec *event = (Event_queue_element_for_exec *)arg;

  event->claim_memory_ownership();

  thd = event->thd;

  thd->claim_memory_ownership();

  mysql_thread_set_psi_id(thd->thread_id());

  Event_worker_thread worker_thread;
  worker_thread.run(thd, event);

  my_thread_end();
  return nullptr;  // Can't return anything here
}
}  // extern "C"

/**
  Function that executes an event in a child thread. Setups the
  environment for the event execution and cleans after that.

  SYNOPSIS
    Event_worker_thread::run()
      thd    Thread context
      event  The Event_queue_element_for_exec object to be processed
*/

void Event_worker_thread::run(THD *thd, Event_queue_element_for_exec *event) {
  /* needs to be first for thread_stack */
  char my_stack;
  Event_job_data job_data;
  bool res;

  DBUG_ASSERT(thd->m_digest == nullptr);

  thd->thread_stack = &my_stack;  // remember where our stack is
  res = post_init_event_thread(thd);

  DBUG_TRACE;
  DBUG_PRINT("info", ("Time is %ld, THD: %p", (long)my_time(0), thd));

  if (res) {
    delete event;
    deinit_event_thread(thd);
    return;
  }

#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_statement_locker_state state;
  DBUG_ASSERT(thd->m_statement_psi == nullptr);
  thd->m_statement_psi = MYSQL_START_STATEMENT(
      &state, event->get_psi_info()->m_key, event->dbname.str,
      event->dbname.length, thd->charset(), nullptr);
#endif
  /*
    We must make sure the schema is released and unlocked in the right
    order. Fail if we are unable to get a meta data lock on the schema
    name. Separate scope so that the Schema_MDL_locker dtor is run before
    thd is deleted.
  */
  {
    dd::Schema_MDL_locker mdl_handler(thd);
    if (mdl_handler.ensure_locked(event->dbname.str)) goto end;

    MDL_key mdl_key;
    dd::Event::create_mdl_key(event->dbname.str, event->name.str, &mdl_key);

    MDL_request event_mdl_request;
    MDL_REQUEST_INIT_BY_KEY(&event_mdl_request, &mdl_key, MDL_SHARED,
                            MDL_EXPLICIT);
    if (thd->mdl_context.acquire_lock_nsec(
            &event_mdl_request, thd->variables.lock_wait_timeout_nsec)) {
      DBUG_PRINT("error", ("Got error in getting MDL locks"));
      goto end;
    }

    if ((res = Event_db_repository::load_named_event(thd, event->dbname,
                                                     event->name, &job_data))) {
      DBUG_PRINT("error", ("Got error from load_named_event"));
      thd->mdl_context.release_lock(event_mdl_request.ticket);
      goto end;
    }
    thd->mdl_context.release_lock(event_mdl_request.ticket);
  }  // End scope so that schema metadata lock is released.

  thd->enable_slow_log = true;

  res = job_data.execute(thd, event->dropped);

  print_warnings(thd, &job_data);

  if (res)
    LogErr(INFORMATION_LEVEL, ER_EVENT_EXECUTION_FAILED, job_data.m_definer.str,
           job_data.m_schema_name.str, job_data.m_event_name.str);

end:
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  MYSQL_END_STATEMENT(thd->m_statement_psi, thd->get_stmt_da());
  thd->m_statement_psi = nullptr;
#endif

  DBUG_ASSERT(thd->m_digest == nullptr);

  DBUG_PRINT("info",
             ("Done with Event %s.%s", event->dbname.str, event->name.str));

  delete event;
  deinit_event_thread(thd);
}

Event_scheduler::Event_scheduler(Event_queue *queue_arg)
    : state(INITIALIZED),
      scheduler_thd(nullptr),
      queue(queue_arg),
      mutex_last_locked_at_line(0),
      mutex_last_unlocked_at_line(0),
      mutex_last_locked_in_func("n/a"),
      mutex_last_unlocked_in_func("n/a"),
      mutex_scheduler_data_locked(false),
      waiting_on_cond(false),
      started_events(0) {
  mysql_mutex_init(key_event_scheduler_LOCK_scheduler_state,
                   &LOCK_scheduler_state, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_event_scheduler_COND_state, &COND_state);
}

Event_scheduler::~Event_scheduler() {
  stop(); /* does nothing if not running */
  mysql_mutex_destroy(&LOCK_scheduler_state);
  mysql_cond_destroy(&COND_state);
}

/**
  Starts the scheduler (again). Creates a new THD and passes it to
  a forked thread. Does not wait for acknowledgement from the new
  thread that it has started. Asynchronous starting. Most of the
  needed initializations are done in the current thread to minimize
  the chance of failure in the spawned thread.

  @param[out] err_no - errno indicating type of error which caused
                       failure to start scheduler thread.

  @retval false Success.
  @retval true  Error.
*/

bool Event_scheduler::start(int *err_no) {
  THD *new_thd = nullptr;
  bool ret = false;
  my_thread_handle th;
  struct scheduler_param *scheduler_param_value;
  DBUG_TRACE;

  LOCK_DATA();
  DBUG_PRINT("info",
             ("state before action %s", scheduler_states_names[state].str));
  if (state > INITIALIZED) goto end;

  DBUG_EXECUTE_IF("event_scheduler_thread_create_failure", {
    *err_no = 11;
    Events::opt_event_scheduler = Events::EVENTS_OFF;
    ret = true;
    goto end;
  });

  if (!(new_thd = new THD)) {
    LogErr(ERROR_LEVEL, ER_CANT_INIT_SCHEDULER_THREAD);
    ret = true;
    goto end;
  }
  pre_init_event_thread(new_thd);
  new_thd->system_thread = SYSTEM_THREAD_EVENT_SCHEDULER;
  new_thd->set_command(COM_DAEMON);

  /*
    We should run the event scheduler thread under the super-user privileges.
    In particular, this is needed to be able to lock the mysql.events table
    for writing when the server is running in the read-only mode.
    Same goes for transaction access mode. Set it to read-write for this thd.

    In case the definer has SYSTEM_USER privileges then event scheduler thread
    can drop the event only if the same privilege is granted to it.
    Therefore, assign all privileges to this thread.
  */
  new_thd->security_context()->skip_grants();
  new_thd->security_context()->set_host_or_ip_ptr(my_localhost,
                                                  strlen(my_localhost));
  new_thd->variables.transaction_read_only = false;
  new_thd->tx_read_only = false;

  scheduler_param_value = (struct scheduler_param *)my_malloc(
      key_memory_Event_scheduler_scheduler_param,
      sizeof(struct scheduler_param), MYF(0));
  scheduler_param_value->thd = new_thd;
  scheduler_param_value->scheduler = this;

  scheduler_thd = new_thd;
  DBUG_PRINT("info", ("Setting state go RUNNING"));
  state = RUNNING;
  DBUG_PRINT("info", ("Forking new thread for scheduler. THD: %p", new_thd));
  if ((*err_no = mysql_thread_create(key_thread_event_scheduler, &th,
                                     &connection_attrib, event_scheduler_thread,
                                     (void *)scheduler_param_value))) {
    DBUG_PRINT("error", ("cannot create a new thread"));
    LogErr(ERROR_LEVEL, ER_CANT_CREATE_SCHEDULER_THREAD, *err_no)
        .os_errno(*err_no);

    new_thd->proc_info = "Clearing";
    new_thd->get_protocol_classic()->end_net();

    state = INITIALIZED;
    scheduler_thd = nullptr;
    delete new_thd;

    my_free(scheduler_param_value);
    ret = true;
  }

end:
  UNLOCK_DATA();
  return ret;
}

/*
  The main loop of the scheduler.

  SYNOPSIS
    Event_scheduler::run()
      thd  Thread

  RETURN VALUE
    false  OK
    true   Error (Serious error)
*/

bool Event_scheduler::run(THD *thd) {
  bool res = false;
  DBUG_TRACE;

  LogErr(INFORMATION_LEVEL, ER_SCHEDULER_STARTED, thd->thread_id());
  /*
    Recalculate the values in the queue because there could have been stops
    in executions of the scheduler and some times could have passed by.
  */
  queue->recalculate_activation_times(thd);

  while (is_running()) {
    Event_queue_element_for_exec *event_name = nullptr;

    /* Gets a minimized version */
    if (queue->get_top_for_execution_if_time(thd, &event_name)) {
      LogErr(INFORMATION_LEVEL, ER_SCHEDULER_STOPPING_FAILED_TO_GET_EVENT);
      if (event_name != nullptr) delete event_name;
      break;
    }

    DBUG_PRINT("info", ("get_top_for_execution_if_time returned "
                        "event_name=%p",
                        event_name));
    if (event_name) {
      if ((res = execute_top(event_name))) break;
    } else {
      DBUG_ASSERT(thd->killed);
      DBUG_PRINT("info", ("job_data is NULL, the thread was killed"));
    }
    DBUG_PRINT("info", ("state=%s", scheduler_states_names[state].str));
    thd->mem_root->Clear();
  }

  LOCK_DATA();
  deinit_event_thread(thd);
  scheduler_thd = nullptr;
  state = INITIALIZED;
  DBUG_PRINT("info", ("Broadcasting COND_state back to the stoppers"));
  mysql_cond_broadcast(&COND_state);
  UNLOCK_DATA();

  return res;
}

/*
  Creates a new THD instance and then forks a new thread, while passing
  the THD pointer and job_data to it.

  SYNOPSIS
    Event_scheduler::execute_top()

  RETURN VALUE
    false  OK
    true   Error (Serious error)
*/

bool Event_scheduler::execute_top(Event_queue_element_for_exec *event_name) {
  THD *new_thd;
  my_thread_handle th;
  int res = 0;
  DBUG_TRACE;
  if (!(new_thd = new THD())) goto error;

  pre_init_event_thread(new_thd);
  new_thd->system_thread = SYSTEM_THREAD_EVENT_WORKER;
  event_name->thd = new_thd;
  DBUG_PRINT("info", ("Event %s@%s ready for start", event_name->dbname.str,
                      event_name->name.str));

  /*
    TODO: should use thread pool here, preferably with an upper limit
    on number of threads: if too many events are scheduled for the
    same time, starting all of them at once won't help them run truly
    in parallel (because of the great amount of synchronization), so
    we may as well execute them in sequence, keeping concurrency at a
    reasonable level.
  */
  /* Major failure */
  if ((res =
           mysql_thread_create(key_thread_event_worker, &th, &connection_attrib,
                               event_worker_thread, event_name))) {
    mysql_mutex_lock(&LOCK_global_system_variables);
    Events::opt_event_scheduler = Events::EVENTS_OFF;
    mysql_mutex_unlock(&LOCK_global_system_variables);

    LogErr(ERROR_LEVEL, ER_SCHEDULER_STOPPING_FAILED_TO_CREATE_WORKER, res);

    new_thd->proc_info = "Clearing";
    new_thd->get_protocol_classic()->end_net();

    goto error;
  }

  ++started_events;

  DBUG_PRINT("info", ("Event is in THD: %p", new_thd));
  return false;

error:
  DBUG_PRINT("error", ("Event_scheduler::execute_top() res: %d", res));
  if (new_thd) delete new_thd;

  delete event_name;
  return true;
}

/*
  Checks whether the state of the scheduler is RUNNING

  SYNOPSIS
    Event_scheduler::is_running()

  RETURN VALUE
    true   RUNNING
    false  Not RUNNING
*/

bool Event_scheduler::is_running() {
  LOCK_DATA();
  bool ret = (state == RUNNING);
  UNLOCK_DATA();
  return ret;
}

/**
  Stops the scheduler (again). Waits for acknowledgement from the
  scheduler that it has stopped - synchronous stopping.

  Already running events will not be stopped. If the user needs
  them stopped manual intervention is needed.

  SYNOPSIS
    Event_scheduler::stop()

  RETURN VALUE
    false  OK
    true   Error (not reported)
*/

bool Event_scheduler::stop() {
  THD *thd = current_thd;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p", thd));

  LOCK_DATA();
  DBUG_PRINT("info",
             ("state before action %s", scheduler_states_names[state].str));
  if (state != RUNNING) {
    /* Synchronously wait until the scheduler stops. */
    while (state != INITIALIZED)
      COND_STATE_WAIT(thd, nullptr, &stage_waiting_for_scheduler_to_stop);
    goto end;
  }

  /* Guarantee we don't catch spurious signals */
  do {
    DBUG_PRINT("info", ("Waiting for COND_started_or_stopped from "
                        "the scheduler thread.  Current value of state is %s . "
                        "workers count=%d",
                        scheduler_states_names[state].str, workers_count()));
    /*
      NOTE: We don't use kill_one_thread() because it can't kill COM_DEAMON
      threads. In addition, kill_one_thread() requires THD but during shutdown
      current_thd is NULL. Hence, if kill_one_thread should be used it has to
      be modified to kill also daemons, by adding a flag, and also we have to
      create artificial THD here. To save all this work, we just do what
      kill_one_thread() does to kill a thread. See also sql_repl.cc for similar
      usage.
    */

    state = STOPPING;
    DBUG_PRINT("info",
               ("Scheduler thread has id %u", scheduler_thd->thread_id()));
    /* Lock from delete */
    mysql_mutex_lock(&scheduler_thd->LOCK_thd_data);
    /* This will wake up the thread if it waits on Queue's conditional */
    LogErr(INFORMATION_LEVEL, ER_SCHEDULER_KILLING, scheduler_thd->thread_id());
    scheduler_thd->awake(THD::KILL_CONNECTION);
    mysql_mutex_unlock(&scheduler_thd->LOCK_thd_data);

    /* thd could be 0x0, when shutting down */
    LogErr(INFORMATION_LEVEL, ER_SCHEDULER_WAITING);
    COND_STATE_WAIT(thd, nullptr, &stage_waiting_for_scheduler_to_stop);
  } while (state == STOPPING);
  DBUG_PRINT("info", ("Scheduler thread has cleaned up. Set state to INIT"));
  LogErr(INFORMATION_LEVEL, ER_SCHEDULER_STOPPED);
end:
  UNLOCK_DATA();
  return false;
}

/**
  This class implements callback for do_for_all_thd().
  It counts the total number of living event worker threads
  from global thread list.
*/

class Is_worker : public Do_THD_Impl {
 public:
  Is_worker() : m_count(0) {}
  virtual void operator()(THD *thd) {
    if (thd->system_thread == SYSTEM_THREAD_EVENT_WORKER) m_count++;
    return;
  }
  int get_count() { return m_count; }

 private:
  int m_count;
};

/*
  Returns the number of living event worker threads.

  SYNOPSIS
    Event_scheduler::workers_count()
*/

int Event_scheduler::workers_count() {
  int count = 0;
  Is_worker is_worker;
  DBUG_TRACE;
  Global_THD_manager::get_instance()->do_for_all_thd(&is_worker);
  count = is_worker.get_count();
  DBUG_PRINT("exit", ("%d", count));
  return count;
}

/*
  Auxiliary function for locking LOCK_scheduler_state. Used
  by the LOCK_DATA macro.

  SYNOPSIS
    Event_scheduler::lock_data()
      func  Which function is requesting mutex lock
      line  On which line mutex lock is requested
*/

void Event_scheduler::lock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mysql_mutex_lock(&LOCK_scheduler_state);
  mutex_last_locked_in_func = func;
  mutex_last_locked_at_line = line;
  mutex_scheduler_data_locked = true;
}

/*
  Auxiliary function for unlocking LOCK_scheduler_state. Used
  by the UNLOCK_DATA macro.

  SYNOPSIS
    Event_scheduler::unlock_data()
      func  Which function is requesting mutex unlock
      line  On which line mutex unlock is requested
*/

void Event_scheduler::unlock_data(const char *func, uint line) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("func=%s line=%u", func, line));
  mutex_last_unlocked_at_line = line;
  mutex_scheduler_data_locked = false;
  mutex_last_unlocked_in_func = func;
  mysql_mutex_unlock(&LOCK_scheduler_state);
}

/*
  Wrapper for mysql_cond_wait/timedwait

  SYNOPSIS
    Event_scheduler::cond_wait()
      thd     Thread (Could be NULL during shutdown procedure)
      abstime If not null then call mysql_cond_timedwait()
      msg     Message for thd->proc_info
      func    Which function is requesting cond_wait
      line    On which line cond_wait is requested
*/

void Event_scheduler::cond_wait(THD *thd, struct timespec *abstime,
                                const PSI_stage_info *stage,
                                const char *src_func, const char *src_file,
                                uint src_line) {
  DBUG_TRACE;
  waiting_on_cond = true;
  mutex_last_unlocked_at_line = src_line;
  mutex_scheduler_data_locked = false;
  mutex_last_unlocked_in_func = src_func;
  if (thd)
    thd->enter_cond(&COND_state, &LOCK_scheduler_state, stage, nullptr,
                    src_func, src_file, src_line);

  DBUG_PRINT("info", ("mysql_cond_%swait", abstime ? "timed" : ""));
  if (!abstime)
    mysql_cond_wait(&COND_state, &LOCK_scheduler_state);
  else
    mysql_cond_timedwait(&COND_state, &LOCK_scheduler_state, abstime);
  if (thd) {
    /*
      Need to unlock before exit_cond, so we need to relock.
      Not the best thing to do but we need to obey cond_wait()
    */
    UNLOCK_DATA();
    thd->exit_cond(nullptr, src_func, src_file, src_line);
    LOCK_DATA();
  }
  mutex_last_locked_in_func = src_func;
  mutex_last_locked_at_line = src_line;
  mutex_scheduler_data_locked = true;
  waiting_on_cond = false;
}

/*
  Dumps the internal status of the scheduler

  SYNOPSIS
    Event_scheduler::dump_internal_status()
*/

void Event_scheduler::dump_internal_status() {
  DBUG_TRACE;

  puts("");
  puts("Event scheduler status:");
  printf("State      : %s\n", scheduler_states_names[state].str);
  printf("Thread id  : %u\n", scheduler_thd ? scheduler_thd->thread_id() : 0);
  printf("LLA        : %s:%u\n", mutex_last_locked_in_func,
         mutex_last_locked_at_line);
  printf("LUA        : %s:%u\n", mutex_last_unlocked_in_func,
         mutex_last_unlocked_at_line);
  printf("WOC        : %s\n", waiting_on_cond ? "YES" : "NO");
  printf("Workers    : %d\n", workers_count());
  printf("Executed   : %lu\n", (ulong)started_events);
  printf("Data locked: %s\n", mutex_scheduler_data_locked ? "YES" : "NO");
}

/**
  @} (End of group Event_Scheduler)
*/
