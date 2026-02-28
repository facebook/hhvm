#include "log_event_wrapper.h"
#include "rpl_rli.h"
#include "rpl_rli_pdb.h"

void Log_event_wrapper::put_next(std::shared_ptr<Log_event_wrapper> &ev) {
  mysql_mutex_lock(&mutex);
  DBUG_ASSERT(!next_ev &&
              (ev->begin_event() == begin_ev.lock() || is_begin_event));
  next_ev = ev;
  mysql_cond_signal(&next_event_cond);
  mysql_mutex_unlock(&mutex);
}

bool Log_event_wrapper::wait(Slave_worker *worker) {
  DBUG_ASSERT(worker);
  mysql_mutex_lock(&mutex);
  auto info_thd = worker->info_thd;
  PSI_stage_info old_stage;
  info_thd->ENTER_COND(&cond, &mutex, &stage_slave_waiting_for_dependencies,
                       &old_stage);
  while (!info_thd->killed && worker->running_status == Slave_worker::RUNNING &&
         dependencies && !worker->found_commit_order_deadlock()) {
    const auto timeout_nsec =
        worker->c_rli->mts_dependency_cond_wait_timeout * 1000000;
    struct timespec abstime;
    set_timespec_nsec(&abstime, timeout_nsec);
    mysql_cond_timedwait(&cond, &mutex, &abstime);
  }
  mysql_mutex_unlock(&mutex);
  info_thd->EXIT_COND(&old_stage);
  return !info_thd->killed && worker->running_status == Slave_worker::RUNNING;
}

void Log_event_wrapper::set_slave_worker(Slave_worker *worker) {
  slave_worker = worker;
}

std::shared_ptr<Log_event_wrapper> Log_event_wrapper::next() {
  if (unlikely(is_end_event)) return nullptr;

  mysql_mutex_lock(&mutex);
  auto worker = slave_worker;
  DBUG_ASSERT(worker);
  auto info_thd = worker->info_thd;
  PSI_stage_info old_stage;
  info_thd->ENTER_COND(&next_event_cond, &mutex,
                       &stage_slave_waiting_event_from_coordinator, &old_stage);
  while (!info_thd->killed && worker->running_status == Slave_worker::RUNNING &&
         !next_ev) {
    ++static_cast<Mts_submode_dependency *>(worker->c_rli->current_mts_submode)
          ->next_event_waits;
    const auto timeout_nsec =
        worker->c_rli->mts_dependency_cond_wait_timeout * 1000000;
    struct timespec abstime;
    set_timespec_nsec(&abstime, timeout_nsec);
    mysql_cond_timedwait(&next_event_cond, &mutex, &abstime);
  }
  mysql_mutex_unlock(&mutex);
  info_thd->EXIT_COND(&old_stage);
  return next_ev;
}

bool Log_event_wrapper::path_exists(
    const std::shared_ptr<Log_event_wrapper> &ev) const {
  auto tmp = next_ev;
  while (tmp) {
    if (tmp == ev) {
      return true;
    }
    tmp = tmp->next_ev;
  }
  return false;
}
