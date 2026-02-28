/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

/*****************************************************************************
Replication debug wait points

This file contains methods used to decrease pollution in replication code
Instead of inserting a entire block

   DBUG_EXECUTE_IF(rpl_slave_debug_point

    now SIGNAL this

    WAIT_FOR that

you can simply do

  DBUG_EXECUTE_IF("rpl_slave_debug_point", {rpl_slave_debug();});

The ideas is to have a method per file/context, like slave, master, gtid, etc

*****************************************************************************/

#ifndef RPL_DEBUG_POINTS_H
#define RPL_DEBUG_POINTS_H

#ifndef DBUG_OFF

#include "my_dbug.h"

enum enum_rpl_slave_debug_point {
  /** stop_slave_dont_release_backup_lock */
  DBUG_RPL_S_STOP_SLAVE_BACKUP_LOCK = 0,

  /** pause_after_queue_event */
  DBUG_RPL_S_PAUSE_QUEUE_EV,

  /** simulate_io_thd_wait_for_disk_space */
  DBUG_RPL_S_IO_WAIT_FOR_SPACE,

  /** dbug.before_get_MASTER_UUID */
  DBUG_RPL_S_BEFORE_MASTER_UUID,

  /** dbug.simulate_busy_io */
  DBUG_RPL_S_SIMULATE_BUSY_IO,

  /** dbug.before_get_UNIX_TIMESTAMP */
  DBUG_RPL_S_BEFORE_UNIX_TIMESTAMP,

  /** dbug.before_get_SERVER_ID */
  DBUG_RPL_S_BEFORE_SERVER_ID,

  /** rpl_before_forced_rotate */
  DBUG_RPL_S_BEFORE_FORCED_ROTATE,

  /** calculate_sbm_after_previous_gtid_log_event */
  DBUG_RPL_S_SBM_AFTER_PREVIOUS_GTID_EV,

  /** calculate_sbm_after_fake_rotate_log_event */
  DBUG_RPL_S_SBM_AFTER_FAKE_ROTATE_EV,

  /** rpl_ps_tables_worker_retry */
  DBUG_RPL_S_PS_TABLE_WORKER_RETRY,

  /** before_get_running_status_yes */
  DBUG_RPL_S_BEFORE_RUNNING_STATUS,

  /** rpl_ps_tables_queue */
  DBUG_RPL_S_PS_TABLE_QUEUE,

  /** rpl_ps_tables */
  DBUG_RPL_S_PS_TABLES,

  /** pause_after_queue_event */
  DBUG_RPL_S_PAUSE_AFTER_QUEUE_EV,

  /** flush_after_reading_user_var_event */
  DBUG_RPL_S_FLUSH_AFTER_USERV_EV,

  /** pause_after_io_thread_stop_hook */
  DBUG_RPL_S_PAUSE_AFTER_IO_STOP,

  /** mts_checkpoint - start */
  DBUG_RPL_S_MTS_CHECKPOINT_START,

  /** mts_checkpoint - end */
  DBUG_RPL_S_MTS_CHECKPOINT_END,

  /** pause_after_sql_thread_stop_hook */
  DBUG_RPL_S_AFTER_SQL_STOP,

  /** pause_on_queuing_event */
  DBUG_RPL_S_PAUSE_QUEUING,

  /** reached_heart_beat_queue_event */
  DBUG_RPL_S_HEARTBEAT_EV,
};

/**
  Method used to decrease code pollution in slave methods
*/
void rpl_slave_debug_point(enum_rpl_slave_debug_point point_id,
                           THD *thd = nullptr) {
  if (!thd) thd = current_thd;

  DBUG_ASSERT(opt_debug_sync_timeout > 0);

  std::string debug_point_string{""};

  switch (point_id) {
    /* stop_slave_cmd */
    case DBUG_RPL_S_STOP_SLAVE_BACKUP_LOCK: {
      debug_point_string.assign(
          "now SIGNAL slave_acquired_backup_lock WAIT_FOR "
          "tried_to_lock_instance_for_backup");
      break;
    }
    /* terminate_slave_threads */
    case DBUG_RPL_S_PAUSE_QUEUE_EV: {
      debug_point_string.assign("now SIGNAL reached_stopping_io_thread");
      break;
    }
    /* terminate_slave_threads */
    case DBUG_RPL_S_IO_WAIT_FOR_SPACE: {
      debug_point_string.assign("now SIGNAL reached_stopping_io_thread");
      break;
    }
    /* get_master_uuid */
    case DBUG_RPL_S_BEFORE_MASTER_UUID: {
      debug_point_string.assign("now wait_for signal.get_master_uuid");
      break;
    }
    /* get_master_uuid */
    case DBUG_RPL_S_SIMULATE_BUSY_IO: {
      debug_point_string.assign(
          "now signal Reached wait_for signal.got_stop_slave");
      break;
    }
    /* get_master_version_and_clock */
    case DBUG_RPL_S_BEFORE_UNIX_TIMESTAMP: {
      debug_point_string.assign("now wait_for signal.get_unix_timestamp");
      break;
    }
    /* get_master_version_and_clock */
    case DBUG_RPL_S_BEFORE_SERVER_ID: {
      debug_point_string.assign("now wait_for signal.get_server_id");
      break;
    }
    /* wait_for_relay_log_space */
    case DBUG_RPL_S_BEFORE_FORCED_ROTATE: {
      debug_point_string.assign(
          "now SIGNAL signal.rpl_before_forced_rotate_reached WAIT_FOR "
          "signal.rpl_before_forced_rotate_continue");
      break;
    }
    /* exec_relay_log_event */
    case DBUG_RPL_S_SBM_AFTER_PREVIOUS_GTID_EV: {
      debug_point_string.assign(
          "now signal signal.reached wait_for signal.done_sbm_calculation");
      break;
    }
    /* exec_relay_log_event */
    case DBUG_RPL_S_SBM_AFTER_FAKE_ROTATE_EV: {
      debug_point_string.assign(
          "now signal signal.reached wait_for signal.done_sbm_calculation");
      break;
    }
    /* exec_relay_log_event */
    case DBUG_RPL_S_PS_TABLE_WORKER_RETRY: {
      debug_point_string.assign(
          "now SIGNAL signal.rpl_ps_tables_worker_retry_pause WAIT_FOR "
          "signal.rpl_ps_tables_worker_retry_continue");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_BEFORE_RUNNING_STATUS: {
      debug_point_string.assign("now wait_for signal.io_thread_let_running");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_PS_TABLE_QUEUE: {
      debug_point_string.assign(
          "now SIGNAL signal.rpl_ps_tables_queue_before WAIT_FOR "
          "signal.rpl_ps_tables_queue_finish");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_PS_TABLES: {
      debug_point_string.assign(
          "now SIGNAL signal.rpl_ps_tables_queue_after_finish WAIT_FOR "
          "signal.rpl_ps_tables_queue_continue");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_PAUSE_AFTER_QUEUE_EV: {
      debug_point_string.assign(
          "now SIGNAL reached_after_queue_event WAIT_FOR "
          "continue_after_queue_event");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_FLUSH_AFTER_USERV_EV: {
      debug_point_string.assign(
          "now signal Reached wait_for signal.flush_complete_continue");
      break;
    }
    /* handle_slave_io */
    case DBUG_RPL_S_PAUSE_AFTER_IO_STOP: {
      debug_point_string.assign(
          "now SIGNAL reached_stopping_io_thread WAIT_FOR "
          "continue_to_stop_io_thread");
      break;
    }
    /* mts_checkpoint_routine */
    case DBUG_RPL_S_MTS_CHECKPOINT_START: {
      debug_point_string.assign("now signal mts_checkpoint_start");
      break;
    }
    /* mts_checkpoint_routine */
    case DBUG_RPL_S_MTS_CHECKPOINT_END: {
      debug_point_string.assign("now signal mts_checkpoint_end");
      break;
    }
    /* handle_slave_sql */
    case DBUG_RPL_S_AFTER_SQL_STOP: {
      debug_point_string.assign(
          "now SIGNAL reached_stopping_sql_thread WAIT_FOR "
          "continue_to_stop_sql_thread");
      break;
    }
    /* queue_event */
    case DBUG_RPL_S_PAUSE_QUEUING: {
      debug_point_string.assign(
          "now SIGNAL reached_queuing_event WAIT_FOR continue_queuing_event");
      break;
    }
    /* queue_event */
    case DBUG_RPL_S_HEARTBEAT_EV: {
      debug_point_string.assign(
          "now SIGNAL check_slave_master_info WAIT_FOR proceed_write_rotate");
      break;
    }
  }

  DBUG_ASSERT(!debug_point_string.empty());
  DBUG_ASSERT(!debug_sync_set_action(thd, debug_point_string.c_str(),
                                     debug_point_string.length()));
}

#endif /* DBUG_OFF */

#endif /* RPL_DEBUG_POINTS_H */
