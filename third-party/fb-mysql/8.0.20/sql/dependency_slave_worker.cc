#include "dependency_slave_worker.h"
#include "debug_sync.h"
#include "log_event_wrapper.h"
#include "rpl_slave_commit_order_manager.h"

bool append_item_to_jobs(slave_job_item *job_item, Slave_worker *w,
                         Relay_log_info *rli);

int slave_worker_exec_single_job(Slave_worker *worker, Relay_log_info *rli,
                                 std::shared_ptr<Log_event_wrapper> &ev_wrap,
                                 uint start_relay_number,
                                 my_off_t start_relay_pos);

std::shared_ptr<Log_event_wrapper> Dependency_slave_worker::get_begin_event(
    Commit_order_manager *co_mngr) {
  return scheduler->dequeue(c_rli, this, co_mngr);
}

// Pulls and executes events single group
// Returns true if the group executed successfully
bool Dependency_slave_worker::execute_group() {
  uint start_relay_number = 0;
  my_off_t start_relay_pos = 0;
  int err = 0;
  Commit_order_manager *commit_order_mngr = get_commit_order_manager();

  auto begin_event = get_begin_event(commit_order_mngr);
  auto ev = begin_event;

  if (begin_event) {
    start_relay_number = begin_event->get_event_relay_log_number();
    start_relay_pos = begin_event->get_event_start_pos();
  }

  while (ev) {
    if ((err = execute_event(ev, start_relay_number, start_relay_pos))) {
      scheduler->dependency_worker_error = true;
      break;
    }
    finalize_event(ev);
    ev = ev->next();
  }

  // case: in case of error rollback if commit ordering is enabled
  if (unlikely(err)) {
    Commit_order_manager::wait_and_finish(info_thd, true);
  }

  scheduler->signal_trx_done(begin_event);

  return err == 0 && !info_thd->killed && running_status == RUNNING;
}

int Dependency_slave_worker::execute_event(
    std::shared_ptr<Log_event_wrapper> &ev, uint start_relay_number,
    my_off_t start_relay_pos) {
  // wait for all dependencies to be satisfied
  if (unlikely(!ev->wait(this))) return 1;

  DBUG_EXECUTE_IF("dbug.dep_wait_before_update_execution", {
    if (ev->raw_event()->get_type_code() == binary_log::UPDATE_ROWS_EVENT) {
      const char act[] = "now signal signal.reached wait_for signal.done";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(info_thd, STRING_WITH_LEN(act)));
    }
  };);

  // case: there was an error in one of the workers, so let's skip execution of
  // events immediately
  if (unlikely(scheduler->dependency_worker_error)) return 1;

  return slave_worker_exec_single_job(this, c_rli, ev, start_relay_number,
                                      start_relay_pos) == 0
             ? 0
             : -1;
}

void Dependency_slave_worker::finalize_event(
    std::shared_ptr<Log_event_wrapper> &ev) {
  scheduler->unregister_keys(ev);
}

Dependency_slave_worker::Dependency_slave_worker(
    Relay_log_info *rli
#ifdef HAVE_PSI_INTERFACE
    ,
    PSI_mutex_key *param_key_info_run_lock,
    PSI_mutex_key *param_key_info_data_lock,
    PSI_mutex_key *param_key_info_sleep_lock,
    PSI_mutex_key *param_key_info_thd_lock,
    PSI_mutex_key *param_key_info_data_cond,
    PSI_mutex_key *param_key_info_start_cond,
    PSI_mutex_key *param_key_info_stop_cond,
    PSI_mutex_key *param_key_info_sleep_cond
#endif
    ,
    uint param_id, const char *param_channel)
    : Slave_worker(rli
#ifdef HAVE_PSI_INTERFACE
                   ,
                   param_key_info_run_lock, param_key_info_data_lock,
                   param_key_info_sleep_lock, param_key_info_thd_lock,
                   param_key_info_data_cond, param_key_info_start_cond,
                   param_key_info_stop_cond, param_key_info_sleep_cond
#endif
                   ,
                   param_id, param_channel) {
  scheduler = static_cast<Mts_submode_dependency *>(c_rli->current_mts_submode);
}

void Dependency_slave_worker::start() {
  DBUG_ASSERT(scheduler->dep_queue.empty());
  DBUG_ASSERT(scheduler->dep_key_lookup.empty());
  DBUG_ASSERT(scheduler->keys_accessed_by_group.empty());
  DBUG_ASSERT(scheduler->dbs_accessed_by_group.empty());

  while (execute_group())
    ;

  // case: cleanup if stopped abruptly
  if (running_status != STOP_ACCEPTED) {
    // tagging as exiting so Coordinator won't be able synchronize with it
    mysql_mutex_lock(&jobs_lock);
    running_status = ERROR_LEAVING;
    mysql_mutex_unlock(&jobs_lock);

    // Killing Coordinator to indicate eventual consistency error
    mysql_mutex_lock(&c_rli->info_thd->LOCK_thd_data);
    c_rli->info_thd->awake(THD::KILL_QUERY);
    mysql_mutex_unlock(&c_rli->info_thd->LOCK_thd_data);
  }
}
