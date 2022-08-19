/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @addtogroup Replication
  @{

  @file sql/rpl_slave.cc

  @brief Code to run the io thread and the sql thread on the
  replication slave.
*/

#include "sql/rpl_slave.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/compression.h"
#include "include/mutex_lock.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/status_var.h"
#include "sql/rpl_channel_service_interface.h"
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <algorithm>
#include <atomic>
#include <chrono>
#include <deque>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <signal.h>
#include "dependency_slave_worker.h"

#include "errmsg.h"  // CR_*
#include "lex_string.h"
#include "libbinlogevents/include/binlog_event.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "libbinlogevents/include/control_events.h"
#include "libbinlogevents/include/debug_vars.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_bitmap.h"  // MY_BITMAP
#include "my_byteorder.h"
#include "my_command.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread_local.h"  // thread_local_key_t
#include "mysql.h"            // MYSQL
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/thread_type.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "mysys_err.h"
#include "pfs_thread_provider.h"
#include "prealloced_array.h"
#include "sql-common/net_ns.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"
#include "sql/binlog_reader.h"
#include "sql/clone_handler.h"  // is_provisioning
#include "sql/current_thd.h"
#include "sql/debug_sync.h"   // DEBUG_SYNC
#include "sql/derror.h"       // ER_THD
#include "sql/dynamic_ids.h"  // Server_ids
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/log.h"
#include "sql/log_event.h"  // Rotate_log_event
#include "sql/mdl.h"
#include "sql/mysqld.h"              // ER
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol.h"
#include "sql/protocol_classic.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/rpl_applier_reader.h"
#include "sql/rpl_filter.h"
#include "sql/rpl_group_replication.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_handler.h"  // RUN_HOOK
#include "sql/rpl_info.h"
#include "sql/rpl_info_factory.h"  // Rpl_info_factory
#include "sql/rpl_info_handler.h"
#include "sql/rpl_mi.h"
#include "sql/rpl_msr.h"  // Multisource_info
#include "sql/rpl_mts_submode.h"
#include "sql/rpl_reporting.h"
#include "sql/rpl_rli.h"                         // Relay_log_info
#include "sql/rpl_rli_pdb.h"                     // Slave_worker
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/rpl_slave_until_options.h"
#include "sql/rpl_trx_boundary_parser.h"
#include "sql/rpl_utility.h"
#include "sql/slave_stats_daemon.h"  // stop_handle_slave_stats_daemon, start_handle_slave_stats_daemon
#include "sql/sql_backup_lock.h"  // is_instance_backup_locked
#include "sql/sql_class.h"        // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"   // execute_init_command
#include "sql/sql_plugin.h"  // opt_plugin_dir_ptr
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/transaction.h"  // trans_begin
#include "sql/transaction_info.h"
#include "sql_common.h"  // end_server
#include "sql_string.h"
#include "typelib.h"
#ifndef DBUG_OFF
#include "rpl_debug_points.h"
#endif

struct mysql_cond_t;
struct mysql_mutex_t;

using binary_log::checksum_crc32;
using binary_log::Log_event_header;
using std::max;
using std::min;
using namespace std::chrono;

#define FLAGSTR(V, F) ((V) & (F) ? #F " " : "")

/*
  a parameter of sql_slave_killed() to defer the killed status
*/
#define SLAVE_WAIT_GROUP_DONE 60
bool use_slave_mask = false;
MY_BITMAP slave_error_mask;
char slave_skip_error_names[SHOW_VAR_FUNC_BUFF_SIZE];

char *slave_load_tmpdir = nullptr;
bool replicate_same_server_id;
ulonglong relay_log_space_limit = 0;
uint rpl_receive_buffer_size = 0;
bool reset_seconds_behind_master = true;

const char *relay_log_index = nullptr;
const char *relay_log_basename = nullptr;

/* When raft has done a TermAdvancement, it starts
 * the SQL thread. During the receive of No-Ops it
 * does the log repointing and then starts the SQL
 * thread. During this phase, no external actor should
 * be able to start the SQL thread. This boolean is
 * set to true when raft has stopped the SQL thread.
 */
std::atomic<bool> sql_thread_stopped_by_raft(false);

std::weak_ptr<Rpl_applier_reader> global_applier_reader;
/*
  MTS load-ballancing parameter.
  Max length of one MTS Worker queue. The value also determines the size
  of Relay_log_info::gaq (see @c slave_start_workers()).
  It can be set to any value in [1, ULONG_MAX - 1] range.
*/
const ulong mts_slave_worker_queue_len_max = 16384;

/*
  Statistics go to the error log every # of seconds when
  --log_error_verbosity > 2
*/
const long mts_online_stat_period = 60 * 2;

/*
  MTS load-ballancing parameter.
  Time unit in microsecs to sleep by MTS Coordinator to avoid extra thread
  signalling in the case of Worker queues are close to be filled up.
*/
const ulong mts_coordinator_basic_nap = 5;

/*
  MTS load-ballancing parameter.
  Percent of Worker queue size at which Worker is considered to become
  hungry.

  C enqueues --+                   . underrun level
               V                   "
   +----------+-+------------------+--------------+
   | empty    |.|::::::::::::::::::|xxxxxxxxxxxxxx| ---> Worker dequeues
   +----------+-+------------------+--------------+

   Like in the above diagram enqueuing to the x-d area would indicate
   actual underrruning by Worker.
*/
const ulong mts_worker_underrun_level = 10;

/*
  When slave thread exits, we need to remember the temporary tables so we
  can re-use them on slave start.

  TODO: move the vars below under Master_info
*/

int disconnect_slave_event_count = 0, abort_slave_event_count = 0;

static thread_local Master_info *RPL_MASTER_INFO = nullptr;

enum enum_slave_reconnect_actions {
  SLAVE_RECON_ACT_REG = 0,
  SLAVE_RECON_ACT_DUMP = 1,
  SLAVE_RECON_ACT_EVENT = 2,
  SLAVE_RECON_ACT_MAX
};

enum enum_slave_reconnect_messages {
  SLAVE_RECON_MSG_WAIT = 0,
  SLAVE_RECON_MSG_KILLED_WAITING = 1,
  SLAVE_RECON_MSG_AFTER = 2,
  SLAVE_RECON_MSG_FAILED = 3,
  SLAVE_RECON_MSG_COMMAND = 4,
  SLAVE_RECON_MSG_KILLED_AFTER = 5,
  SLAVE_RECON_MSG_MAX
};

static const char *reconnect_messages[SLAVE_RECON_ACT_MAX][SLAVE_RECON_MSG_MAX] =
    {{"Waiting to reconnect after a failed registration on master",
      "Slave I/O thread killed while waiting to reconnect after a failed \
registration on master",
      "Reconnecting after a failed registration on master",
      "failed registering on master, reconnecting to try again, \
log '%s' at position %s",
      "COM_REGISTER_SLAVE",
      "Slave I/O thread killed during or after reconnect"},
     {"Waiting to reconnect after a failed binlog dump request",
      "Slave I/O thread killed while retrying master dump",
      "Reconnecting after a failed binlog dump request",
      "failed dump request, reconnecting to try again, log '%s' at position %s",
      "COM_BINLOG_DUMP", "Slave I/O thread killed during or after reconnect"},
     {"Waiting to reconnect after a failed master event read",
      "Slave I/O thread killed while waiting to reconnect after a failed read",
      "Reconnecting after a failed master event read",
      "Slave I/O thread: Failed reading log event, reconnecting to retry, \
log '%s' at position %s",
      "",
      "Slave I/O thread killed during or after a reconnect done to recover from \
failed read"}};

enum enum_slave_apply_event_and_update_pos_retval {
  SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK = 0,
  SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR = 1,
  SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR = 2,
  SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR = 3,
  SLAVE_APPLY_EVENT_RETRY = 4,
  SLAVE_APPLY_EVENT_UNTIL_REACHED = 5,
  SLAVE_APPLY_EVENT_AND_UPDATE_POS_MAX
};

static int process_io_rotate(Master_info *mi, Rotate_log_event *rev);
static bool wait_for_relay_log_space(Relay_log_info *rli);
static inline bool io_slave_killed(THD *thd, Master_info *mi);
static inline bool is_autocommit_off_and_infotables(THD *thd);
static int init_slave_thread(THD *thd, SLAVE_THD_TYPE thd_type);
static void print_slave_skip_errors(void);
static int safe_connect(THD *thd, MYSQL *mysql, Master_info *mi);
static int safe_reconnect(THD *thd, MYSQL *mysql, Master_info *mi,
                          bool suppress_warnings);
static int connect_to_master(THD *thd, MYSQL *mysql, Master_info *mi,
                             bool reconnect, bool suppress_warnings);
static int get_master_version_and_clock(MYSQL *mysql, Master_info *mi);
static int get_master_uuid(MYSQL *mysql, Master_info *mi);
int io_thread_init_commands(MYSQL *mysql, Master_info *mi);
static int terminate_slave_thread(THD *thd, mysql_mutex_t *term_lock,
                                  mysql_cond_t *term_cond,
                                  std::atomic<uint> *slave_running,
                                  ulong *stop_wait_timeout, bool need_lock_term,
                                  bool force = false);
static bool check_io_slave_killed(THD *thd, Master_info *mi, const char *info);
static int mts_event_coord_cmp(LOG_POS_COORD *id1, LOG_POS_COORD *id2);

static int check_slave_sql_config_conflict(const Relay_log_info *rli);

/*
  Applier thread InnoDB priority.
  When two transactions conflict inside InnoDB, the one with
  greater priority wins.

  @param thd       Thread handler for slave
  @param priority  Thread priority
*/
static void set_thd_tx_priority(THD *thd, int priority) {
  DBUG_TRACE;
  DBUG_ASSERT(thd->system_thread == SYSTEM_THREAD_SLAVE_SQL ||
              thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER);

  thd->thd_tx_priority = priority;
  DBUG_EXECUTE_IF("dbug_set_high_prio_sql_thread",
                  { thd->thd_tx_priority = 1; });
}

static void log_slave_command(THD *thd) {
  Security_context *sctx = thd->security_context();
  if (!sctx) return;
  sql_print_information("Executing slave command '%s' by user %s from host %s",
                        thd->query().str, sctx->user().str, sctx->host().str);
}

/*
  Function to set the slave's max_allowed_packet based on the value
  of slave_max_allowed_packet.

    @in_param    thd    Thread handler for slave
    @in_param    mysql  MySQL connection handle
*/

static void set_slave_max_allowed_packet(THD *thd, MYSQL *mysql) {
  DBUG_TRACE;
  // thd and mysql must be valid
  DBUG_ASSERT(thd && mysql);

  thd->variables.max_allowed_packet = slave_max_allowed_packet;
  /*
    Adding MAX_LOG_EVENT_HEADER_LEN to the max_packet_size on the I/O
    thread and the mysql->option max_allowed_packet, since a
    replication event can become this much  larger than
    the corresponding packet (query) sent from client to master.
  */
  thd->get_protocol_classic()->set_max_packet_size(slave_max_allowed_packet +
                                                   MAX_LOG_EVENT_HEADER);
  /*
    Skipping the setting of mysql->net.max_packet size to slave
    max_allowed_packet since this is done during mysql_real_connect.
  */
  mysql->options.max_allowed_packet =
      slave_max_allowed_packet + MAX_LOG_EVENT_HEADER;
}

/*
  Find out which replications threads are running

  SYNOPSIS
    init_thread_mask()
    mask                Return value here
    mi                  master_info for slave
    inverse             If set, returns which threads are not running

  IMPLEMENTATION
    Get a bit mask for which threads are running so that we can later restart
    these threads.

  RETURN
    mask        If inverse == 0, running threads
                If inverse == 1, stopped threads
*/

void init_thread_mask(int *mask, Master_info *mi, bool inverse) {
  bool set_io = mi->slave_running, set_sql = mi->rli->slave_running;
  int tmp_mask = 0;
  DBUG_TRACE;

  if (set_io) tmp_mask |= SLAVE_IO;
  if (set_sql) tmp_mask |= SLAVE_SQL;
  if (inverse) tmp_mask ^= (SLAVE_IO | SLAVE_SQL);
  *mask = tmp_mask;
}

/*
  lock_slave_threads()
*/

void lock_slave_threads(Master_info *mi) {
  DBUG_TRACE;

  // protection against mixed locking order (see header)
  mi->channel_assert_some_wrlock();

  // TODO: see if we can do this without dual mutex
  mysql_mutex_lock(&mi->run_lock);
  mysql_mutex_lock(&mi->rli->run_lock);
}

/*
  unlock_slave_threads()
*/

void unlock_slave_threads(Master_info *mi) {
  DBUG_TRACE;

  // TODO: see if we can do this without dual mutex
  mysql_mutex_unlock(&mi->rli->run_lock);
  mysql_mutex_unlock(&mi->run_lock);
}

#ifdef HAVE_PSI_INTERFACE

static PSI_memory_key key_memory_rli_mts_coor;

static PSI_thread_key key_thread_slave_io, key_thread_slave_sql,
    key_thread_slave_worker;

static PSI_thread_info all_slave_threads[] = {
    {&key_thread_slave_io, "slave_io", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_thread_slave_sql, "slave_sql", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_thread_slave_worker, "slave_worker", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME}};

static PSI_memory_info all_slave_memory[] = {{&key_memory_rli_mts_coor,
                                              "Relay_log_info::mts_coor", 0, 0,
                                              PSI_DOCUMENT_ME}};

static void init_slave_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_slave_threads));
  mysql_thread_register(category, all_slave_threads, count);

  count = static_cast<int>(array_elements(all_slave_memory));
  mysql_memory_register(category, all_slave_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

static bool configured_as_slave() {
  channel_map.assert_some_lock();

  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    if (Master_info::is_configured(it->second)) {
      return true;
    }
  }
  return false;
}

/* Initialize slave structures */

int init_slave() {
  DBUG_TRACE;
  int error = 0;
  int thread_mask = SLAVE_SQL;

  // No IO thread in raft mode
  if (!enable_raft_plugin) thread_mask |= SLAVE_IO;

#ifdef HAVE_PSI_INTERFACE
  init_slave_psi_keys();
#endif

  /*
    This is called when mysqld starts. Before client connections are
    accepted. However bootstrap may conflict with us if it does START SLAVE.
    So it's safer to take the lock.
  */
  channel_map.wrlock();

  RPL_MASTER_INFO = nullptr;

  /*
    Create slave info objects by reading repositories of individual
    channels and add them into channel_map
  */
  if ((error = Rpl_info_factory::create_slave_info_objects(
           opt_mi_repository_id, opt_rli_repository_id, thread_mask,
           &channel_map)))
    LogErr(ERROR_LEVEL,
           ER_RPL_SLAVE_FAILED_TO_CREATE_OR_RECOVER_INFO_REPOSITORIES);

#ifndef DBUG_OFF
  /* @todo: Print it for all the channels */
  {
    Master_info *default_mi;
    default_mi = channel_map.get_default_channel_mi();
    if (default_mi && default_mi->rli) {
      DBUG_PRINT("info",
                 ("init group master %s %lu  group relay %s %lu event %s %lu\n",
                  default_mi->rli->get_group_master_log_name(),
                  (ulong)default_mi->rli->get_group_master_log_pos(),
                  default_mi->rli->get_group_relay_log_name(),
                  (ulong)default_mi->rli->get_group_relay_log_pos(),
                  default_mi->rli->get_event_relay_log_name(),
                  (ulong)default_mi->rli->get_event_relay_log_pos()));
    }
  }
#endif

  is_slave = configured_as_slave();

  /**
    If engine binlog max gtid is set, then update recovery_max_engine_gtid.
    recovery_max_engine_gtid is used later during slave's idempotent
    recovery/apply of binnlog events.
    engine_binlog_max_gtid is set during storage engine recovery using
    global_sid_map. However, idempotent recovery/apply uses
    rli->recovery_sid_map. Hence rli->recovery_max_engine_gtid needs to be
    initialized by hashing into rli->recovery_sid_map
    NOTE: idempotent recovery requires tables have unique key to work correctly
   */
  if (!mysql_bin_log.engine_binlog_max_gtid.is_empty()) {
    char buf[Gtid::MAX_TEXT_LENGTH + 1];

    /* Extract engine_binlog_max_gtid using global_sid_map */
    mysql_bin_log.engine_binlog_max_gtid.to_string(global_sid_map, buf,
                                                   /* need_lock */ true);
    mysql_bin_log.set_recovery_binlog_max_gtid(buf);

    /* Now set rli->recovery_max_engine_gtid (and optionally add
       it into rli->recovery_sid_map */
    for (const auto &channel : channel_map) {
      channel.second->rli->populate_recovery_binlog_max_gtid();
    }

    LogErr(SYSTEM_LEVEL, ER_RPL_SLAVE_MAX_GTID_RECOVERED, buf);
  }

  if (is_slave && mysql_bin_log.engine_binlog_pos != ULLONG_MAX &&
      mysql_bin_log.engine_binlog_file[0] &&
      get_gtid_mode(GTID_MODE_LOCK_CHANNEL_MAP) != GTID_MODE_OFF &&
      !enable_raft_plugin) {
    /*
      With less durable settins (sync_binlog !=1 and
      innodb_flush_log_at_trx_commit !=1), a slave with GTIDs/MTS
      may be inconsistent due to the two possible scenarios below
      1) slave's binary log is behind innodb transaction log.
      2) slave's binlary log is ahead of innodb transaction log.
      The engine_binlog_max_gtid in innodb trx log stores
      max gtid executed in engine and handles scenario 1. But in case
      of scenario 2, slave will skip executing some transactions if it's
      GTID is logged in the binlog even though it is not commiited in
      innodb.
      Scenario 2 is handled by changing gtid_executed when a
      slave is initialized based on the binlog file and binlog position
      which are logged inside innodb trx log. When gtid_executed is set
      to an old value which is consistent with innodb, slave doesn't
      miss any transactions.

     This entire block is skipped in raft mode since the executed gtid set is
     calculated correctly based on engine position and filename during
     transaction log (binlog or apply-log) recovery and gtid initialization
    */
    mysql_mutex_t *log_lock = mysql_bin_log.get_log_lock();
    mysql_mutex_lock(log_lock);
    global_sid_lock->wrlock();
    char file_name[FN_REFLEN + 1];
    mysql_bin_log.make_log_name(file_name, mysql_bin_log.engine_binlog_file);

    char *binlog_gtid_set_buffer = nullptr;
    gtid_state->get_executed_gtids()->to_string(&binlog_gtid_set_buffer);

    const_cast<Gtid_set *>(gtid_state->get_executed_gtids())->clear();
    MYSQL_BIN_LOG::enum_read_gtids_from_binlog_status ret =
        mysql_bin_log.read_gtids_from_binlog(
            file_name, const_cast<Gtid_set *>(gtid_state->get_executed_gtids()),
            NULL, NULL, global_sid_map, opt_master_verify_checksum, false,
            mysql_bin_log.engine_binlog_pos);

    if (ret == MYSQL_BIN_LOG::ERROR || ret == MYSQL_BIN_LOG::TRUNCATED) {
      global_sid_lock->unlock();
      mysql_mutex_unlock(log_lock);
      my_free(binlog_gtid_set_buffer);
      sql_print_error("Fail to read binlog");
      error = 1;
      goto err;
    }

    char *trx_gtid_set_buffer = nullptr;
    gtid_state->get_executed_gtids()->to_string(&trx_gtid_set_buffer);
    sql_print_information("Resetting GTID_EXECUTED: old : %s, new: %s",
                          binlog_gtid_set_buffer, trx_gtid_set_buffer);
    my_free(binlog_gtid_set_buffer);
    my_free(trx_gtid_set_buffer);

    global_sid_lock->unlock();
    // rotate writes the consistent gtid_executed as previous_gtid_log_event
    // in next binlog. This is done to avoid situations where there is a
    // slave crash immediately after executing some relay log events.
    // Those slave crashes are not safe if binlog is not rotated since the
    // gtid_executed set after crash recovery will be inconsistent with InnoDB.
    // A crash before this rotate is safe because of valid binlog file and
    // position values inside innodb trx header which will not be updated
    // until sql_thread is ready.
    bool check_purge;
    mysql_bin_log.rotate(true, &check_purge);
    mysql_mutex_unlock(log_lock);
    if (ret == MYSQL_BIN_LOG::ERROR || ret == MYSQL_BIN_LOG::TRUNCATED) {
      sql_print_error(
          "Failed to read log %s up to pos %llu "
          "to find out crash safe gtid_executed "
          "Replication will not be setup due to "
          "possible data inconsistency with master. ",
          mysql_bin_log.engine_binlog_file, mysql_bin_log.engine_binlog_pos);
      error = 1;
      goto err;
    }
  }

  if (get_gtid_mode(GTID_MODE_LOCK_CHANNEL_MAP) == GTID_MODE_OFF) {
    for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
         it++) {
      Master_info *mi = it->second;
      if (mi != nullptr && mi->is_auto_position()) {
        LogErr(WARNING_LEVEL,
               ER_RPL_SLAVE_AUTO_POSITION_IS_1_AND_GTID_MODE_IS_OFF,
               mi->get_channel(), mi->get_channel());
      }
    }
  }

  if (check_slave_sql_config_conflict(nullptr)) {
    error = 1;
    goto err;
  }

  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    Master_info *mi = it->second;
    mi->rli->relay_log.raft_log_recover();
  }

  /*
    Loop through the channel_map and start slave threads for each channel.
  */
  if (!opt_skip_slave_start) {
    for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
         it++) {
      Master_info *mi = it->second;

      /* If server id is not set, start_slave_thread() will say it */
      if (Master_info::is_configured(mi) && mi->rli->inited) {
        /* same as in start_slave() cache the global var values into rli's
         * members */
        mi->rli->opt_slave_parallel_workers = opt_mts_slave_parallel_workers;
        mi->rli->checkpoint_group = opt_mts_checkpoint_group;
        const auto parallel_option = get_mts_parallel_option();
        if (parallel_option == MTS_PARALLEL_TYPE_DB_NAME)
          mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DB_NAME;
        else if (parallel_option == MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
          mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
        else
          mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DEPENDENCY;
        if (start_slave_threads(true /*need_lock_slave=true*/,
                                false /*wait_for_start=false*/, mi,
                                thread_mask)) {
          LogErr(ERROR_LEVEL, ER_FAILED_TO_START_SLAVE_THREAD,
                 mi->get_channel());
        }
      } else {
        LogErr(INFORMATION_LEVEL, ER_FAILED_TO_START_SLAVE_THREAD,
               mi->get_channel());
      }
    }
  }

err:

  channel_map.unlock();
  if (error) LogErr(INFORMATION_LEVEL, ER_SLAVE_NOT_STARTED_ON_SOME_CHANNELS);

  return error;
}

/**
   Function to start a slave for all channels.
   Used in Multisource replication.
   @param[in]        thd           THD object of the client.

   @retval false success
   @retval true error

    @todo  It is good to continue to start other channels
           when a slave start failed for other channels.

    @todo  The problem with the below code is if the slave is already
           stared, we would multple warnings called
           "Slave was already running" for each channel.
           A nice warning message  would be to add
           "Slave for channel '%s" was already running"
           but error messages are in different languages and cannot be tampered
           with so, we have to handle it case by case basis, whether
           only default channel exists or not and properly continue with
           starting other channels if one channel fails clearly giving
           an error message by displaying failed channels.
*/
bool start_slave(THD *thd) {
  DBUG_TRACE;
  Master_info *mi;
  bool channel_configured, error = false;

  if (channel_map.get_num_instances() == 1) {
    mi = channel_map.get_default_channel_mi();
    DBUG_ASSERT(mi);
    if (start_slave(thd, &thd->lex->slave_connection, &thd->lex->mi,
                    thd->lex->slave_thd_opt, mi, true))
      return true;
  } else {
    /*
      Users cannot start more than one channel's applier thread
      if sql_slave_skip_counter > 0. It throws an error to the session.
    */
    mysql_mutex_lock(&LOCK_sql_slave_skip_counter);
    /* sql_slave_skip_counter > 0 && !(START SLAVE IO_THREAD) */
    if (sql_slave_skip_counter > 0 && !(thd->lex->slave_thd_opt & SLAVE_IO)) {
      my_error(ER_SLAVE_CHANNEL_SQL_SKIP_COUNTER, MYF(0));
      mysql_mutex_unlock(&LOCK_sql_slave_skip_counter);
      return true;
    }
    mysql_mutex_unlock(&LOCK_sql_slave_skip_counter);

    for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
         it++) {
      mi = it->second;

      channel_configured =
          mi &&                      // Master_info exists
          (mi->inited || mi->reset)  // It is inited or was reset
          && mi->host[0];            // host is set

      if (channel_configured) {
        if (start_slave(thd, &thd->lex->slave_connection, &thd->lex->mi,
                        thd->lex->slave_thd_opt, mi, true)) {
          LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_START_SLAVE_FOR_CHANNEL,
                 mi->get_channel());
          error = true;
        }
      }
    }
  }
  if (!error) {
    /* no error */
    my_ok(thd);
  }
  return error;
}

/**
   Function to stop a slave for all channels.
   Used in Multisource replication.
   @param[in]        thd           THD object of the client.

   @retval           0             success
   @retval           1             error

    @todo  It is good to continue to stop other channels
           when a slave start failed for other channels.
*/
int stop_slave(THD *thd) {
  DBUG_TRACE;
  bool push_temp_table_warning = true;
  Master_info *mi = nullptr;
  int error = 0;

  if (channel_map.get_num_instances() == 1) {
    mi = channel_map.get_default_channel_mi();

    DBUG_ASSERT(!strcmp(mi->get_channel(), channel_map.get_default_channel()));

    error = stop_slave(thd, mi, true, false /*for_one_channel*/,
                       &push_temp_table_warning);
  } else {
    for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
         it++) {
      mi = it->second;

      if (Master_info::is_configured(mi)) {
        if (stop_slave(thd, mi, true, false /*for_one_channel*/,
                       &push_temp_table_warning)) {
          LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_STOP_SLAVE_FOR_CHANNEL,
                 mi->get_channel());
          error = 1;
        }
      }
    }
  }

  if (!error) {
    /* no error */
    my_ok(thd);
  }

  return error;
}

/**
  Entry point to the START SLAVE command. The function
  decides to start replication threads on several channels
  or a single given channel.

  @param[in]   thd        the client thread carrying the command.

  @retval      false      ok
  @retval      true       not ok.
*/
bool start_slave_cmd(THD *thd) {
  DBUG_TRACE;

  Master_info *mi;
  LEX *lex = thd->lex;
  bool res = true; /* default, an error */

  DEBUG_SYNC(thd, "begin_start_slave");

  channel_map.wrlock();

  DEBUG_SYNC(thd, "after_locking_channel_map_in_start_slave");

  if (!is_slave_configured()) {
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
    goto err;
  }

  if (!lex->mi.for_channel) {
    /*
      If slave_until options are provided when multiple channels exist
      without explicitly providing FOR CHANNEL clause, error out.
    */
    if (lex->mi.slave_until && channel_map.get_num_instances() > 1) {
      my_error(ER_SLAVE_MULTIPLE_CHANNELS_CMD, MYF(0));
      goto err;
    }

    res = start_slave(thd);
  } else {
    mi = channel_map.get_mi(lex->mi.channel);

    /*
      If the channel being used is a group replication channel we need to
      disable this command here as, in some cases, group replication does not
      support them.

      For channel group_replication_applier we disable START SLAVE [IO_THREAD]
      command.

      For channel group_replication_recovery we disable START SLAVE command
      and its two thread variants.
    */
    if (mi &&
        channel_map.is_group_replication_channel_name(mi->get_channel()) &&
        ((!thd->lex->slave_thd_opt || (thd->lex->slave_thd_opt & SLAVE_IO)) ||
         (!(channel_map.is_group_replication_channel_name(mi->get_channel(),
                                                          true)) &&
          (thd->lex->slave_thd_opt & SLAVE_SQL)))) {
      const char *command = "START SLAVE FOR CHANNEL";
      if (thd->lex->slave_thd_opt & SLAVE_IO)
        command = "START SLAVE IO_THREAD FOR CHANNEL";
      else if (thd->lex->slave_thd_opt & SLAVE_SQL)
        command = "START SLAVE SQL_THREAD FOR CHANNEL";

      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0), command,
               mi->get_channel(), command);

      goto err;
    }

    if (mi)
      res = start_slave(thd, &thd->lex->slave_connection, &thd->lex->mi,
                        thd->lex->slave_thd_opt, mi, true);
    else if (strcmp(channel_map.get_default_channel(), lex->mi.channel))
      my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), lex->mi.channel);

    if (!res) my_ok(thd);
  }
err:
  channel_map.unlock();
  return res;
}

/**
  Entry point for the STOP SLAVE command. This function stops replication
  threads for all channels or a single channel based on the  command
  options supplied.

  @param[in]     thd         the client thread.

  @retval        false       ok
  @retval        true        not ok.
*/
bool stop_slave_cmd(THD *thd) {
  DBUG_TRACE;

  Master_info *mi;
  bool push_temp_table_warning = true;
  LEX *lex = thd->lex;
  bool res = true; /*default, an error */

  channel_map.rdlock();

  if (!is_slave_configured()) {
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
    channel_map.unlock();
    return true;
  }

  MDL_lock_guard backup_sentry{thd};
  /* During provisioning we stop slave after acquiring backup lock. */
  if (!Clone_handler::is_provisioning() &&
      (!thd->lex->slave_thd_opt || (thd->lex->slave_thd_opt & SLAVE_SQL))) {
    if (backup_sentry.lock(MDL_key::BACKUP_LOCK, MDL_INTENTION_EXCLUSIVE)) {
      my_error(ER_RPL_CANT_STOP_SLAVE_WHILE_LOCKED_BACKUP, MYF(0));
      channel_map.unlock();
      return true;
    }
  }

  if (!lex->mi.for_channel)
    res = stop_slave(thd);
  else {
    mi = channel_map.get_mi(lex->mi.channel);

    /*
      If the channel being used is a group replication channel we need to
      disable this command here as, in some cases, group replication does not
      support them.

      For channel group_replication_applier we disable STOP SLAVE [IO_THREAD]
      command.

      For channel group_replication_recovery we disable STOP SLAVE command
      and its two thread variants.
    */
    if (mi &&
        channel_map.is_group_replication_channel_name(mi->get_channel()) &&
        ((!thd->lex->slave_thd_opt || (thd->lex->slave_thd_opt & SLAVE_IO)) ||
         (!(channel_map.is_group_replication_channel_name(mi->get_channel(),
                                                          true)) &&
          (thd->lex->slave_thd_opt & SLAVE_SQL)))) {
      const char *command = "STOP SLAVE FOR CHANNEL";
      if (thd->lex->slave_thd_opt & SLAVE_IO)
        command = "STOP SLAVE IO_THREAD FOR CHANNEL";
      else if (thd->lex->slave_thd_opt & SLAVE_SQL)
        command = "STOP SLAVE SQL_THREAD FOR CHANNEL";

      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0), command,
               mi->get_channel(), command);

      channel_map.unlock();
      return true;
    }

    if (mi)
      res = stop_slave(thd, mi, true /*net report */, true /*for_one_channel*/,
                       &push_temp_table_warning);
    else if (strcmp(channel_map.get_default_channel(), lex->mi.channel))
      my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), lex->mi.channel);
  }

  channel_map.unlock();

  DBUG_EXECUTE_IF("stop_slave_dont_release_backup_lock", {
    rpl_slave_debug_point(DBUG_RPL_S_STOP_SLAVE_BACKUP_LOCK, thd);
  });

  return res;
}

enum enum_read_rotate_from_relay_log_status {
  FOUND_ROTATE,
  NOT_FOUND_ROTATE,
  ERROR
};

/**
   Parse the given relay log and identify the rotate event from the master.
   Ignore the Format description event, Previous_gtid log event, ignorable
   event and Stop event within the relay log as they are generated by slave.
   When a rotate event is found check if it is a rotate that is originated from
   the master based on the server_id. Ignore the event if the rotate is from
   slave or if it is a fake rotate event. If any other events are encountered
   apart from the above events generate an error. From the rotate event
   extract the master's binary log name and position.

   @param filename
          Relay log name which needs to be parsed.

   @param[out] master_log_file
          Set the master_log_file to the log file name that is extracted from
          rotate event. The master_log_file should contain string of len
          FN_REFLEN.

   @param[out] master_log_pos
          Set the master_log_pos to the log position extracted from rotate
          event.

   @retval FOUND_ROTATE: When rotate event is found in the relay log
   @retval NOT_FOUND_ROTATE: When rotate event is not found in the relay log
   @retval ERROR: On error
 */
static enum_read_rotate_from_relay_log_status read_rotate_from_relay_log(
    char *filename, char *master_log_file, my_off_t *master_log_pos) {
  DBUG_TRACE;

  Relaylog_file_reader relaylog_file_reader(opt_slave_sql_verify_checksum);
  if (relaylog_file_reader.open(filename)) {
    LogErr(ERROR_LEVEL, ER_RPL_RECOVERY_ERROR,
           relaylog_file_reader.get_error_str());
    return ERROR;
  }

  Log_event *ev = nullptr;
  bool done = false;
  enum_read_rotate_from_relay_log_status ret = NOT_FOUND_ROTATE;
  while (!done && (ev = relaylog_file_reader.read_event_object()) != nullptr) {
    DBUG_PRINT("info", ("Read event of type %s", ev->get_type_str()));
    switch (ev->get_type_code()) {
      case binary_log::FORMAT_DESCRIPTION_EVENT:
        break;
      case binary_log::ROTATE_EVENT:
        /*
          Check for rotate event from the master. Ignore the ROTATE event if it
          is a fake rotate event with server_id=0.
         */
        if (ev->server_id && ev->server_id != ::server_id) {
          Rotate_log_event *rotate_ev = (Rotate_log_event *)ev;
          DBUG_ASSERT(FN_REFLEN >= rotate_ev->ident_len + 1);
          memcpy(master_log_file, rotate_ev->new_log_ident,
                 rotate_ev->ident_len + 1);
          *master_log_pos = rotate_ev->pos;
          ret = FOUND_ROTATE;
          done = true;
        }
        break;
      case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
        break;
      case binary_log::IGNORABLE_LOG_EVENT:
        break;
      case binary_log::STOP_EVENT:
        break;
      default:
        LogErr(ERROR_LEVEL, ER_RPL_RECOVERY_NO_ROTATE_EVENT_FROM_MASTER);
        ret = ERROR;
        done = true;
        break;
    }
    delete ev;
  }
  if (relaylog_file_reader.has_fatal_error()) {
    LogErr(ERROR_LEVEL, ER_RPL_RECOVERY_ERROR_READ_RELAY_LOG, -1);
    return ERROR;
  }
  return ret;
}

/**
   Reads relay logs one by one starting from the first relay log. Looks for
   the first rotate event from the master. If rotate is not found in the relay
   log search continues to next relay log. If rotate event from master is
   found then the extracted master_log_file and master_log_pos are used to set
   rli->group_master_log_name and rli->group_master_log_pos. If an error has
   occurred the error code is retuned back.

   @param rli
          Relay_log_info object to read relay log files and to set
          group_master_log_name and group_master_log_pos.

   @retval 0 Success - Rotate event was found
   @retval 1 Failure - Found some events replicated but no rotate event was
   found
   @retval 2 When no rotate event from master was found. This can happen when
             slave server was restarted immediately after executing CHANGE
   MASTER
 */
static int find_first_relay_log_with_rotate_from_master(Relay_log_info *rli) {
  DBUG_TRACE;
  int error = 0;
  LOG_INFO linfo;
  bool got_rotate_from_master = false;
  int pos;
  char master_log_file[FN_REFLEN];
  my_off_t master_log_pos = 0;

  if (channel_map.is_group_replication_channel_name(rli->get_channel())) {
    LogErr(INFORMATION_LEVEL,
           ER_RPL_RECOVERY_SKIPPED_GROUP_REPLICATION_CHANNEL);
    goto err;
  }

  for (pos = rli->relay_log.find_log_pos(&linfo, nullptr, true); !pos;
       pos = rli->relay_log.find_next_log(&linfo, true)) {
    switch (read_rotate_from_relay_log(linfo.log_file_name, master_log_file,
                                       &master_log_pos)) {
      case ERROR:
        error = 1;
        break;
      case FOUND_ROTATE:
        got_rotate_from_master = true;
        break;
      case NOT_FOUND_ROTATE:
        break;
    }
    if (error || got_rotate_from_master) break;
  }
  if (pos == LOG_INFO_IO) {
    error = 1;
    LogErr(ERROR_LEVEL, ER_RPL_RECOVERY_IO_ERROR_READING_RELAY_LOG_INDEX);
    goto err;
  }
  if (pos == LOG_INFO_EOF) {
    error = 2;
    LogErr(WARNING_LEVEL, ER_RPL_RECOVERY_NO_ROTATE_EVENT_FROM_MASTER);
    LogErr(WARNING_LEVEL, ER_WARN_RPL_RECOVERY_NO_ROTATE_EVENT_FROM_MASTER_EOF,
           rli->mi->get_channel());
    goto err;
  }
  if (!error && got_rotate_from_master) {
    rli->set_group_master_log_name(master_log_file);
    rli->set_group_master_log_pos(master_log_pos);
  }
err:
  return error;
}

/*
  Updates the master info based on the information stored in the
  relay info and ignores relay logs previously retrieved by the IO
  thread, which thus starts fetching again based on to the
  master_log_pos and master_log_name. Eventually, the old
  relay logs will be purged by the normal purge mechanism.

  When GTID's are enabled the "Retrieved GTID" set should be cleared
  so that partial read events are discarded and they are
  fetched once again

  @param mi    pointer to Master_info instance
*/
static void recover_relay_log(Master_info *mi) {
  Relay_log_info *rli = mi->rli;
  // Set Receiver Thread's positions as per the recovered Applier Thread.
  mi->set_master_log_pos(
      max<ulonglong>(BIN_LOG_HEADER_SIZE, rli->get_group_master_log_pos()));
  mi->set_master_log_name(rli->get_group_master_log_name());

  LogErr(WARNING_LEVEL, ER_RPL_RECOVERY_FILE_MASTER_POS_INFO,
         (ulong)mi->get_master_log_pos(), mi->get_master_log_name(),
         mi->get_for_channel_str(), rli->get_group_relay_log_pos(),
         rli->get_group_relay_log_name());

  // Start with a fresh relay log.
  rli->set_group_relay_log_name(rli->relay_log.get_log_fname());
  rli->set_group_relay_log_pos(BIN_LOG_HEADER_SIZE);
  /*
    Clear the retrieved GTID set so that events that are written partially
    will be fetched again.
  */
  if (mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE) == GTID_MODE_ON &&
      !channel_map.is_group_replication_channel_name(rli->get_channel())) {
    rli->get_sid_lock()->wrlock();
    (const_cast<Gtid_set *>(rli->get_gtid_set()))->clear_set_and_sid_map();
    rli->get_sid_lock()->unlock();
  }
}

/*
  Updates the master info based on the information stored in the
  relay info and ignores relay logs previously retrieved by the IO
  thread, which thus starts fetching again based on to the
  master_log_pos and master_log_name. Eventually, the old
  relay logs will be purged by the normal purge mechanism.

  There can be a special case where rli->group_master_log_name and
  rli->group_master_log_pos are not intialized, as the sql thread was never
  started at all. In those cases all the existing relay logs are parsed
  starting from the first one and the initial rotate event that was received
  from the master is identified. From the rotate event master_log_name and
  master_log_pos are extracted and they are set to rli->group_master_log_name
  and rli->group_master_log_pos.

  In the feature, we should improve this routine in order to avoid throwing
  away logs that are safely stored in the disk. Note also that this recovery
  routine relies on the correctness of the relay-log.info and only tolerates
  coordinate problems in master.info.

  In this function, there is no need for a mutex as the caller
  (i.e. init_slave) already has one acquired.

  Specifically, the following structures are updated:

  1 - mi->master_log_pos  <-- rli->group_master_log_pos
  2 - mi->master_log_name <-- rli->group_master_log_name
  3 - It moves the relay log to the new relay log file, by
      rli->group_relay_log_pos  <-- BIN_LOG_HEADER_SIZE;
      rli->event_relay_log_pos  <-- BIN_LOG_HEADER_SIZE;
      rli->group_relay_log_name <-- rli->relay_log.get_log_fname();
      rli->event_relay_log_name <-- rli->relay_log.get_log_fname();

   If there is an error, it returns (1), otherwise returns (0).
 */
int init_recovery(Master_info *mi) {
  DBUG_TRACE;

  int error = 0;
  Relay_log_info *rli = mi->rli;
  char *group_master_log_name = nullptr;

  /* Set the recovery_parallel_workers to 0 if Auto Position is enabled. */
  bool is_gtid_with_autopos_on =
      (((mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE) == GTID_MODE_ON) &&
        mi->is_auto_position())
           ? true
           : false);
  if (is_gtid_with_autopos_on) rli->recovery_parallel_workers = 0;

  if (rli->recovery_parallel_workers) {
    /*
      This is not idempotent and a crash after this function and before
      the recovery is actually done may lead the system to an inconsistent
      state.

      This may happen because the gap is not persitent stored anywhere
      and eventually old relay log files will be removed and further
      calculations on the gaps will be impossible.

      We need to improve this. /Alfranio.
    */
    error = mts_recovery_groups(rli);
    if (rli->mts_recovery_group_cnt) return error;
  }

  group_master_log_name = const_cast<char *>(rli->get_group_master_log_name());
  if (!error) {
    bool run_relay_log_recovery = true;
    if (!group_master_log_name[0]) {
      if (rli->replicate_same_server_id) {
        error = 1;
        LogErr(ERROR_LEVEL,
               ER_RPL_RECOVERY_REPLICATE_SAME_SERVER_ID_REQUIRES_POSITION);
        return error;
      }
      error = find_first_relay_log_with_rotate_from_master(rli);
      if (error == 2) {
        // No events from the master on relay log - skip relay log recovery
        run_relay_log_recovery = false;
        error = 0;
      } else if (error)
        return error;
    }
    if (run_relay_log_recovery) recover_relay_log(mi);
  }
  return error;
}

/*
  Relay log recovery in the case of MTS, is handled by the following function.
  Gaps in MTS execution are filled using implicit execution of
  START SLAVE UNTIL SQL_AFTER_MTS_GAPS call. Once slave reaches a consistent
  gapless state receiver thread's positions are initialized to applier thread's
  positions and the old relay logs are discarded. This completes the recovery
  process.

  @param mi    pointer to Master_info instance.

  @retval 0 success
  @retval 1 error
*/
static inline int fill_mts_gaps_and_recover(Master_info *mi) {
  DBUG_TRACE;
  Relay_log_info *rli = mi->rli;
  int recovery_error = 0;
  rli->is_relay_log_recovery = false;
  Until_mts_gap *until_mg = new Until_mts_gap(rli);
  rli->set_until_option(until_mg);
  rli->until_condition = Relay_log_info::UNTIL_SQL_AFTER_MTS_GAPS;
  until_mg->init();
  const auto parallel_option = get_mts_parallel_option();
  if (parallel_option == MTS_PARALLEL_TYPE_DB_NAME)
    rli->channel_mts_submode = MTS_PARALLEL_TYPE_DB_NAME;
  else if (parallel_option == MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
    rli->channel_mts_submode = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
  else
    rli->channel_mts_submode = MTS_PARALLEL_TYPE_DEPENDENCY;
  LogErr(INFORMATION_LEVEL, ER_RPL_MTS_RECOVERY_STARTING_COORDINATOR);
  recovery_error = start_slave_thread(
#ifdef HAVE_PSI_THREAD_INTERFACE
      key_thread_slave_sql,
#endif
      handle_slave_sql, &rli->run_lock, &rli->run_lock, &rli->start_cond,
      &rli->slave_running, &rli->slave_run_id, mi);

  if (recovery_error) {
    LogErr(WARNING_LEVEL, ER_RPL_MTS_RECOVERY_FAILED_TO_START_COORDINATOR);
    goto err;
  }
  mysql_mutex_lock(&rli->run_lock);
  mysql_cond_wait(&rli->stop_cond, &rli->run_lock);
  mysql_mutex_unlock(&rli->run_lock);
  if (rli->until_condition != Relay_log_info::UNTIL_DONE) {
    LogErr(WARNING_LEVEL, ER_RPL_MTS_AUTOMATIC_RECOVERY_FAILED);
    goto err;
  }
  rli->clear_until_option();
  /*
    We need a mutex while we are changing master info parameters to
    keep other threads from reading bogus info
  */
  mysql_mutex_lock(&mi->data_lock);
  mysql_mutex_lock(&rli->data_lock);
  recover_relay_log(mi);

  if (mi->flush_info(true) || rli->flush_info(true)) {
    recovery_error = 1;
    mysql_mutex_unlock(&mi->data_lock);
    mysql_mutex_unlock(&rli->data_lock);
    goto err;
  }
  rli->inited = true;
  rli->error_on_rli_init_info = false;
  mysql_mutex_unlock(&mi->data_lock);
  mysql_mutex_unlock(&rli->data_lock);
  LogErr(INFORMATION_LEVEL, ER_RPL_MTS_RECOVERY_SUCCESSFUL);
  return recovery_error;
err:
  /*
    If recovery failed means we failed to initialize rli object in the case
    of MTS. We should not allow the START SLAVE command to work as we do in
    the case of STS. i.e if init_recovery call fails then we set inited=0.
  */
  rli->end_info();
  rli->inited = false;
  rli->error_on_rli_init_info = true;
  rli->clear_until_option();
  return recovery_error;
}

static Master_info *raft_get_default_mi();

/**
 * This changes the name of the raft relay log
 * to binlog name
 */
int rli_relay_log_raft_reset(
    std::pair<std::string, uint64_t> raft_log_applied_upto_pos, THD *thd) {
  DBUG_ENTER("rli_relay_log_raft_reset");
  Master_info *mi = nullptr;
  Relay_log_info *rli = nullptr;
  Gtid_set *all_gtid_set = nullptr;
  int error = 0;
  std::string normalized_log_name;

  if (disable_raft_log_repointing) DBUG_RETURN(0);

  channel_map.rdlock();

  mi = raft_get_default_mi();
  if (!mi) {
    channel_map.unlock();
    sql_print_error("Could not get default mi in rli_relay_log_raft_reset");
    error = 1;
    return error;
  }

  rli = mi->rli;
  normalized_log_name =
      std::string(binlog_file_basedir_ptr) +
      std::string(raft_log_applied_upto_pos.first.c_str() +
                  dirname_length(raft_log_applied_upto_pos.first.c_str()));

  /*
    We need a mutex while we are changing master info parameters to
    keep other threads from reading bogus info
  */
  mysql_mutex_lock(&mi->data_lock);
  mysql_mutex_lock(&mi->rli->data_lock);

  enum_return_check check_return_mi = mi->check_info();
  enum_return_check check_return_rli = mi->rli->check_info();

  // If the master.info file does not exist, or if it exists,
  // but the inited has never happened (most likely due to an
  // error), try mi_init_info
  if (check_return_mi == REPOSITORY_DOES_NOT_EXIST || !mi->inited) {
    // NO_LINT_DEBUG
    sql_print_information(
        "rli_relay_log_raft_reset: Master info "
        "repository doesn't exist or not inited."
        " Calling mi_init_info");
    if (mi->mi_init_info()) {
      // NO_LINT_DEBUG
      sql_print_error(
          "rli_relay_log_raft_reset: Failed to initialize "
          "the master info structure");
      error = 1;
      goto end;
    }
  }

  if (check_return_rli == REPOSITORY_DOES_NOT_EXIST) {
    // NO_LINT_DEBUG
    // NO_LINT_DEBUG
    sql_print_information(
        "rli_relay_log_raft_reset: Relay log info repository"
        " doesn't exist or not inited. Calling"
        " load_mi_and_rli_from_repositories ");
    // TODO: Check these additional params (skip_received_gtid_set_recovery)
    if (load_mi_and_rli_from_repositories(
            mi,
            /*ignore_if_no_info=*/false,
            /*thread_mask=*/SLAVE_SQL | SLAVE_IO,
            /*skip_received_gtid_set_recovery=*/false,
            /*need_lock=*/false)) {
      // NO_LINT_DEBUG
      sql_print_error("Failed to initialize the master info structure");
      error = 1;
      goto end;
    }
  }

  mysql_mutex_lock(mi->rli->relay_log.get_log_lock());
  mi->rli->relay_log.lock_index();

  mi->rli->relay_log.close(LOG_CLOSE_INDEX, /*need_lock_log=*/false,
                           /*need_lock_index=*/false);

  // open_index_file() will calculate index file full path(using 2nd argument)
  // if 1st argument is nullptr. Otherwise, it will treat 1st arugment as index
  // file full path.
  if (mi->rli->relay_log.open_index_file(nullptr, opt_bin_logname,
                                         /*need_lock_index=*/false)) {
    // NO_LINT_DEBUG
    sql_print_error("rli_relay_log_raft_reset::failed to open index file");
    error = 1;
    mi->rli->relay_log.unlock_index();
    mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());
    goto end;
  }

  all_gtid_set = const_cast<Gtid_set *>(mi->rli->get_gtid_set());
  if (all_gtid_set == nullptr) {
    error = 1;
    mi->rli->relay_log.unlock_index();
    mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());
    goto end;
  }

  all_gtid_set->get_sid_map()->get_sid_lock()->wrlock();

  // At this point the gtid set in the RLI and the last retrieved
  // GTID are up to date with all the gtids in the last raft log
  // file
  mi->rli->relay_log.init_gtid_sets(
      all_gtid_set, NULL, opt_slave_sql_verify_checksum,
      /*need_lock=*/false, &mi->transaction_parser,
      mi->get_gtid_monitoring_info());

  all_gtid_set->get_sid_map()->get_sid_lock()->unlock();

  mi->rli->relay_log.set_previous_gtid_set_relaylog(all_gtid_set);

  // At the end of this
  // cur_log_ext, log_file_name, name and IO_CACHE(log_file) should all be
  // up to date
  if (mi->rli->relay_log.open_existing_binlog(opt_bin_logname,
                                              max_binlog_size)) {
    // NO_LINT_DEBUG
    sql_print_error("rli_relay_log_raft_reset::failed to open binlog file");
    error = 1;
    mi->rli->relay_log.unlock_index();
    mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());
    goto end;
  }

  // Setup the positions correctly for sql appliers
  mi->rli->set_group_relay_log_name(normalized_log_name.c_str());
  mi->rli->set_group_relay_log_pos(raft_log_applied_upto_pos.second);

  mi->rli->set_event_relay_log_pos(rli->get_group_relay_log_pos());
  mi->rli->set_event_relay_log_name(rli->get_group_relay_log_name());

  // Register log to raft
  // Previous mi->rli->relay_log.close(LOG_CLOSE_INDEX) will also close
  // binlog and its IO_CACHE.
  mi->rli->relay_log.register_log_entities(thd, /*context=*/0,
                                           /*need_lock=*/false,
                                           /*is_relay_log=*/true);
  mi->rli->relay_log.unlock_index();
  mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());

  // NO_LINT_DEBUG
  sql_print_information("Relay log cursor set to: %s:%llu",
                        mi->rli->get_group_relay_log_name(),
                        mi->rli->get_group_relay_log_pos());

  mi->rli->inited = true;

end:

  mysql_mutex_unlock(&mi->rli->data_lock);
  mysql_mutex_unlock(&mi->data_lock);
  channel_map.unlock();
  DBUG_RETURN(error);
}

int load_mi_and_rli_from_repositories(Master_info *mi, bool ignore_if_no_info,
                                      int thread_mask,
                                      bool skip_received_gtid_set_recovery,
                                      bool need_lock) {
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);
  int init_error = 0;
  enum_return_check check_return = ERROR_CHECKING_REPOSITORY;
  THD *thd = current_thd;

  /*
    We need a mutex while we are changing master info parameters to
    keep other threads from reading bogus info
  */
  if (need_lock) {
    mysql_mutex_lock(&mi->data_lock);
    mysql_mutex_lock(&mi->rli->data_lock);
  }

  /*
    When info tables are used and autocommit= 0 we force a new
    transaction start to avoid table access deadlocks when START SLAVE
    is executed after RESET SLAVE.
  */
  if (is_autocommit_off_and_infotables(thd)) {
    if (trans_begin(thd)) {
      init_error = 1;
      goto end;
    }
  }

  /*
    This takes care of the startup dependency between the master_info
    and relay_info. It initializes the master info if the SLAVE_IO
    thread is being started and the relay log info if either the
    SLAVE_SQL thread is being started or was not initialized as it is
    required by the SLAVE_IO thread.
  */
  check_return = mi->check_info();
  if (check_return == ERROR_CHECKING_REPOSITORY) {
    if (enable_raft_plugin) {
      // NO_LINT_DEBUG
      sql_print_error(
          "load_mi_and_rli_from_repositories: mi repository "
          "check returns ERROR_CHECKING_REPOSITORY");
    }
    init_error = 1;
    goto end;
  }

  if (!(ignore_if_no_info && check_return == REPOSITORY_DOES_NOT_EXIST)) {
    if ((thread_mask & SLAVE_IO) != 0) {
      if (enable_raft_plugin) {
        // NO_LINT_DEBUG
        sql_print_information(
            "load_mi_and_rli_from_repositories: mi_init_info called");
      }
      if (mi->mi_init_info()) {
        if (enable_raft_plugin) {
          // NO_LINT_DEBUG
          sql_print_error(
              "load_mi_and_rli_from_repositories: mi_init_info returned error");
        }
        init_error = 1;
      }
    }
  }

  check_return = mi->rli->check_info();
  if (check_return == ERROR_CHECKING_REPOSITORY) {
    if (enable_raft_plugin) {
      // NO_LINT_DEBUG
      sql_print_error(
          "load_mi_and_rli_from_repositories: rli repository check returns"
          " ERROR_CHECKING_REPOSITORY");
    }
    init_error = 1;
    goto end;
  }
  if (!(ignore_if_no_info && check_return == REPOSITORY_DOES_NOT_EXIST)) {
    if (((thread_mask & SLAVE_SQL) != 0 || !(mi->rli->inited))) {
      if (enable_raft_plugin) {
        // NO_LINT_DEBUG
        sql_print_information(
            "load_mi_and_rli_from_repositories: rli_init_info called");
      }
      if (mi->rli->rli_init_info(skip_received_gtid_set_recovery)) {
        if (enable_raft_plugin) {
          // NO_LINT_DEBUG
          sql_print_error(
              "load_mi_and_rli_from_repositories: rli_init_info returned "
              "error");
        }
        init_error = 1;
      }
    } else {
      /*
        During rli_init_info() above, the relay log is opened (if rli was not
        initialized yet). The function below expects the relay log to be opened
        to get its coordinates and store as the last flushed relay log
        coordinates from I/O thread point of view.
      */
      mi->update_flushed_relay_log_info();
    }
  }

  DBUG_EXECUTE_IF("enable_mts_worker_failure_init",
                  { DBUG_SET("+d,mts_worker_thread_init_fails"); });
end:
  /*
    When info tables are used and autocommit= 0 we force transaction
    commit to avoid table access deadlocks when START SLAVE is executed
    after RESET SLAVE.
  */
  if (is_autocommit_off_and_infotables(thd))
    if (trans_commit(thd)) init_error = 1;

  if (need_lock) {
    mysql_mutex_unlock(&mi->rli->data_lock);
    mysql_mutex_unlock(&mi->data_lock);
  }

  /*
    Handling MTS Relay-log recovery after successful initialization of mi and
    rli objects.

    MTS Relay-log recovery is handled by SSUG command. In order to start the
    slave applier thread rli needs to be inited and mi->rli->data_lock should
    be in released state. Hence we do the MTS recovery at this point of time
    where both conditions are satisfied.
  */
  if (!init_error && mi->rli->is_relay_log_recovery &&
      mi->rli->mts_recovery_group_cnt)
    init_error = fill_mts_gaps_and_recover(mi);
  return init_error;
}

// TODO: currently we're only setting host port
int raft_reset_slave(THD *) {
  DBUG_ENTER("raft_reset_slave");
  int error = 0;
  channel_map.rdlock();
  Master_info *mi = channel_map.get_default_channel_mi();

  if (!mi) {
    channel_map.unlock();
    DBUG_RETURN(error);
  }
  mysql_mutex_lock(&mi->data_lock);
  strmake(mi->host, "\0", sizeof(mi->host) - 1);
  mi->port = 0;
  mi->inited = false;
  mysql_mutex_lock(&mi->rli->data_lock);
  mi->rli->inited = false;
  /**
    Clear the retrieved gtid set for this channel.
  */
  mi->rli->get_sid_lock()->wrlock();
  (const_cast<Gtid_set *>(mi->rli->get_gtid_set()))->clear_set_and_sid_map();
  mi->rli->get_sid_lock()->unlock();

  mysql_mutex_unlock(&mi->rli->data_lock);
  mysql_mutex_unlock(&mi->data_lock);
  remove_info(mi);
  // no longer a slave. will be set again during change master
  is_slave = false;
  channel_map.unlock();
  DBUG_RETURN(error);
}

// TODO: currently we're only setting host port
int raft_change_master(
    THD *, const std::pair<const std::string, uint> &master_instance,
    const std::string &master_uuid) {
  DBUG_ENTER("raft_change_master");
  int error = 0;

  channel_map.rdlock();
  Master_info *mi = channel_map.get_default_channel_mi();

  if (!mi) {
    channel_map.unlock();
    DBUG_RETURN(error);
  }

  mysql_mutex_lock(&mi->data_lock);
  strmake(mi->host, const_cast<char *>(master_instance.first.c_str()),
          sizeof(mi->host) - 1);
  mi->port = master_instance.second;
  DBUG_ASSERT(master_uuid.length() == UUID_LENGTH);
  strncpy(mi->master_uuid, master_uuid.c_str(), UUID_LENGTH);
  mi->master_uuid[UUID_LENGTH] = 0;
  mi->set_auto_position(true);
  mi->init_master_log_pos();

  int thread_mask_stopped_threads;
  /*
    Before load_mi_and_rli_from_repositories() call, get a bit mask to indicate
    stopped threads in thread_mask_stopped_threads. Since the third argguement
    is 1, thread_mask when the function returns stands for stopped threads.
  */
  init_thread_mask(&thread_mask_stopped_threads, mi, 1);
  mysql_mutex_lock(&mi->rli->data_lock);
  // Call mi->init_info() and/or mi->rli->init_info() if itn't configured
  if (load_mi_and_rli_from_repositories(mi, false, thread_mask_stopped_threads,
                                        false, /*need_lock*/ false)) {
    error = ER_MASTER_INFO;
    my_error(ER_MASTER_INFO, MYF(0));
    goto end;
  }
  mi->inited = true;
  mi->flush_info(true);

  // changing to a slave. set the is_slave flag
  is_slave = true;
end:
  mysql_mutex_unlock(&mi->rli->data_lock);
  mysql_mutex_unlock(&mi->data_lock);
  channel_map.unlock();
  DBUG_RETURN(error);
}

void end_info(Master_info *mi) {
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);

  /*
    The previous implementation was not acquiring locks.  We do the same here.
    However, this is quite strange.
  */
  mi->end_info();
  mi->rli->end_info();
}

void clear_info(Master_info *mi) {
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);

  /*
    Reset errors (the idea is that we forget about the
    old master).
  */
  mi->clear_error();
  mi->rli->clear_error();
  if (mi->rli->workers_array_initialized) {
    for (size_t i = 0; i < mi->rli->get_worker_count(); i++) {
      mi->rli->get_worker(i)->clear_error();
    }
  }
  mi->rli->clear_sql_delay();

  end_info(mi);
}

int remove_info(Master_info *mi) {
  int error = 1;
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);

  /*
    The previous implementation was not acquiring locks.
    We do the same here. However, this is quite strange.
  */
  clear_info(mi);

  if (mi->remove_info() || Rpl_info_factory::reset_workers(mi->rli) ||
      mi->rli->remove_info())
    goto err;

  error = 0;

err:
  return error;
}

bool reset_info(Master_info *mi) {
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);

  clear_info(mi);

  if (mi->remove_info() || Rpl_info_factory::reset_workers(mi->rli))
    return true;

  MUTEX_LOCK(mi_lock, &mi->data_lock);
  MUTEX_LOCK(rli_lock, &mi->rli->data_lock);

  mi->init_master_log_pos();
  mi->master_uuid[0] = 0;

  if (mi->reset && opt_mi_repository_id == INFO_REPOSITORY_TABLE &&
      opt_rli_repository_id == INFO_REPOSITORY_TABLE && mi->flush_info(true)) {
    my_error(ER_MASTER_INFO, MYF(0));
    return true;
  }

  bool have_relay_log_data_to_persist =              // Only want to keep
      (!mi->rli->is_privilege_checks_user_null() ||  // if PCU is not null
       mi->rli->is_row_format_required() ||          // or RRF is 1
       Relay_log_info::PK_CHECK_STREAM !=            // or RTPKC != STREAM
           mi->rli->get_require_table_primary_key_check()) &&
      opt_rli_repository_id == INFO_REPOSITORY_TABLE &&  // in TABLE repository.
      opt_mi_repository_id == INFO_REPOSITORY_TABLE;

  if ((have_relay_log_data_to_persist && mi->rli->clear_info()) ||
      (!have_relay_log_data_to_persist && mi->rli->remove_info())) {
    my_error(ER_MASTER_INFO, MYF(0));
    return true;
  }

  return false;
}

int flush_master_info(Master_info *mi, bool force, bool need_lock,
                      bool do_flush_relay_log) {
  DBUG_TRACE;
  DBUG_ASSERT(mi != nullptr && mi->rli != nullptr);
  DBUG_EXECUTE_IF("fail_to_flush_master_info", { return 1; });
  /*
    With the appropriate recovery process, we will not need to flush
    the content of the current log.

    For now, we flush the relay log BEFORE the master.info file, because
    if we crash, we will get a duplicate event in the relay log at restart.
    If we change the order, there might be missing events.

    If we don't do this and the slave server dies when the relay log has
    some parts (its last kilobytes) in memory only, with, say, from master's
    position 100 to 150 in memory only (not on disk), and with position 150
    in master.info, there will be missing information. When the slave restarts,
    the I/O thread will fetch binlogs from 150, so in the relay log we will
    have "[0, 100] U [150, infinity[" and nobody will notice it, so the SQL
    thread will jump from 100 to 150, and replication will silently break.
  */
  mysql_mutex_t *log_lock = mi->rli->relay_log.get_log_lock();
  mysql_mutex_t *data_lock = &mi->data_lock;

  if (need_lock) {
    mysql_mutex_lock(log_lock);
    mysql_mutex_lock(data_lock);
  } else {
    mysql_mutex_assert_owner(log_lock);
    mysql_mutex_assert_owner(&mi->data_lock);
  }

  int err = 0;
  /*
    We can skip flushing the relay log when this function is called from
    queue_event(), as after_write_to_relay_log() will already flush it.
  */
  if (do_flush_relay_log) err |= mi->rli->flush_current_log();

  err |= mi->flush_info(force);

  if (need_lock) {
    mysql_mutex_unlock(data_lock);
    mysql_mutex_unlock(log_lock);
  }

  return err;
}

/**
  Convert slave skip errors bitmap into a printable string.
*/

static void print_slave_skip_errors(void) {
  /*
    To be safe, we want 10 characters of room in the buffer for a number
    plus terminators. Also, we need some space for constant strings.
    10 characters must be sufficient for a number plus {',' | '...'}
    plus a NUL terminator. That is a max 6 digit number.
  */
  const size_t MIN_ROOM = 10;
  DBUG_TRACE;
  DBUG_ASSERT(sizeof(slave_skip_error_names) > MIN_ROOM);
  DBUG_ASSERT(MAX_SLAVE_ERROR <= 999999);  // 6 digits

  if (!use_slave_mask || bitmap_is_clear_all(&slave_error_mask)) {
    /* purecov: begin tested */
    memcpy(slave_skip_error_names, STRING_WITH_LEN("OFF"));
    /* purecov: end */
  } else if (bitmap_is_set_all(&slave_error_mask)) {
    /* purecov: begin tested */
    memcpy(slave_skip_error_names, STRING_WITH_LEN("ALL"));
    /* purecov: end */
  } else {
    char *buff = slave_skip_error_names;
    char *bend = buff + sizeof(slave_skip_error_names);
    int errnum;

    for (errnum = 0; errnum < MAX_SLAVE_ERROR; errnum++) {
      if (bitmap_is_set(&slave_error_mask, errnum)) {
        if (buff + MIN_ROOM >= bend) break; /* purecov: tested */
        buff = longlong10_to_str(errnum, buff, -10);
        *buff++ = ',';
      }
    }
    if (buff != slave_skip_error_names) buff--;  // Remove last ','
    /*
      The range for client side error is [2000-2999]
      so if the errnum doesn't lie in that and if less
      than MAX_SLAVE_ERROR[10000] we enter the if loop.
    */
    if (errnum < MAX_SLAVE_ERROR &&
        (errnum < CR_MIN_ERROR || errnum > CR_MAX_ERROR)) {
      /* Couldn't show all errors */
      buff = my_stpcpy(buff, "..."); /* purecov: tested */
    }
    *buff = 0;
  }
  DBUG_PRINT("init", ("error_names: '%s'", slave_skip_error_names));
}

/**
 Change arg to the string with the nice, human-readable skip error values.
   @param slave_skip_errors_ptr
          The pointer to be changed
*/
void set_slave_skip_errors(char **slave_skip_errors_ptr) {
  DBUG_TRACE;
  print_slave_skip_errors();
  *slave_skip_errors_ptr = slave_skip_error_names;
}

/**
  Init function to set up array for errors that should be skipped for slave
*/
static void init_slave_skip_errors() {
  DBUG_TRACE;
  DBUG_ASSERT(!use_slave_mask);  // not already initialized

  if (bitmap_init(&slave_error_mask, nullptr, MAX_SLAVE_ERROR)) {
    fprintf(stderr, "Badly out of memory, please check your system status\n");
    exit(MYSQLD_ABORT_EXIT);
  }
  use_slave_mask = true;
}

static void add_slave_skip_errors(const uint *errors, uint n_errors) {
  DBUG_TRACE;
  DBUG_ASSERT(errors);
  DBUG_ASSERT(use_slave_mask);

  for (uint i = 0; i < n_errors; i++) {
    const uint err_code = errors[i];
    /*
      The range for client side error is [2000-2999]
      so if the err_code doesn't lie in that and if less
      than MAX_SLAVE_ERROR[14000] we enter the if loop.
    */
    if (err_code < MAX_SLAVE_ERROR &&
        (err_code < CR_MIN_ERROR || err_code > CR_MAX_ERROR))
      bitmap_set_bit(&slave_error_mask, err_code);
  }
}

/*
  Add errors that should be skipped for slave

  SYNOPSIS
    add_slave_skip_errors()
    arg         List of errors numbers to be added to skip, separated with ','

  NOTES
    Called from get_options() in mysqld.cc on start-up
*/

void add_slave_skip_errors(const char *arg) {
  const char *p = nullptr;
  /*
    ALL is only valid when nothing else is provided.
  */
  const uchar SKIP_ALL[] = "all";
  size_t SIZE_SKIP_ALL = strlen((const char *)SKIP_ALL) + 1;
  /*
    IGNORE_DDL_ERRORS can be combined with other parameters
    but must be the first one provided.
  */
  const uchar SKIP_DDL_ERRORS[] = "ddl_exist_errors";
  size_t SIZE_SKIP_DDL_ERRORS = strlen((const char *)SKIP_DDL_ERRORS);
  DBUG_TRACE;

  // initialize mask if not done yet
  if (!use_slave_mask) init_slave_skip_errors();

  for (; my_isspace(system_charset_info, *arg); ++arg) /* empty */
    ;
  if (!my_strnncoll(system_charset_info, pointer_cast<const uchar *>(arg),
                    SIZE_SKIP_ALL, SKIP_ALL, SIZE_SKIP_ALL)) {
    bitmap_set_all(&slave_error_mask);
    return;
  }
  if (!my_strnncoll(system_charset_info, pointer_cast<const uchar *>(arg),
                    SIZE_SKIP_DDL_ERRORS, SKIP_DDL_ERRORS,
                    SIZE_SKIP_DDL_ERRORS)) {
    // DDL errors to be skipped for relaxed 'exist' handling
    const uint ddl_errors[] = {
        // error codes with create/add <schema object>
        ER_DB_CREATE_EXISTS, ER_TABLE_EXISTS_ERROR, ER_DUP_KEYNAME,
        ER_MULTIPLE_PRI_KEY,
        // error codes with change/rename <schema object>
        ER_BAD_FIELD_ERROR, ER_NO_SUCH_TABLE, ER_DUP_FIELDNAME,
        // error codes with drop <schema object>
        ER_DB_DROP_EXISTS, ER_BAD_TABLE_ERROR, ER_CANT_DROP_FIELD_OR_KEY};

    add_slave_skip_errors(ddl_errors,
                          sizeof(ddl_errors) / sizeof(ddl_errors[0]));
    /*
      After processing the SKIP_DDL_ERRORS, the pointer is
      increased to the position after the comma.
    */
    if (strlen(arg) > SIZE_SKIP_DDL_ERRORS + 1) arg += SIZE_SKIP_DDL_ERRORS + 1;
  }
  for (p = arg; *p;) {
    long err_code;
    if (!(p = str2int(p, 10, 0, LONG_MAX, &err_code))) break;
    if (err_code < MAX_SLAVE_ERROR)
      bitmap_set_bit(&slave_error_mask, (uint)err_code);
    while (!my_isdigit(system_charset_info, *p) && *p) p++;
  }
}

static void set_thd_in_use_temporary_tables(Relay_log_info *rli) {
  TABLE *table;

  for (table = rli->save_temporary_tables; table; table = table->next) {
    table->in_use = rli->info_thd;
    if (table->file != nullptr) {
      /*
        Since we are stealing opened temporary tables from one thread to
        another, we need to let the performance schema know that, for aggregates
        per thread to work properly.
      */
      table->file->unbind_psi();
      table->file->rebind_psi();
    }
  }
}

int terminate_slave_threads(Master_info *mi, int thread_mask,
                            ulong stop_wait_timeout, bool need_lock_term) {
  DBUG_TRACE;

  if (!mi->inited) return 0; /* successfully do nothing */
  int error, force_all = (thread_mask & SLAVE_FORCE_ALL);
  mysql_mutex_t *sql_lock = &mi->rli->run_lock, *io_lock = &mi->run_lock;
  mysql_mutex_t *log_lock = mi->rli->relay_log.get_log_lock();
  /*
    Set it to a variable, so the value is shared by both stop methods.
    This guarantees that the user defined value for the timeout value is for
    the time the 2 threads take to shutdown, and not the time of each thread
    stop operation.
  */
  ulong total_stop_wait_timeout = stop_wait_timeout;

  if (thread_mask & (SLAVE_SQL | SLAVE_FORCE_ALL)) {
    DBUG_PRINT("info", ("Terminating SQL thread"));
    mi->rli->abort_slave = true;
    if ((error = terminate_slave_thread(
             mi->rli->info_thd, sql_lock, &mi->rli->stop_cond,
             &mi->rli->slave_running, &total_stop_wait_timeout,
             need_lock_term)) &&
        !force_all) {
      if (error == 1) {
        return ER_STOP_SLAVE_SQL_THREAD_TIMEOUT;
      }
      return error;
    }

    DBUG_PRINT("info", ("Flushing relay-log info file."));
    if (current_thd)
      THD_STAGE_INFO(current_thd, stage_flushing_relay_log_info_file);

    /*
      Flushes the relay log info regardles of the sync_relay_log_info option.
    */
    if (mi->rli->flush_info(true)) {
      return ER_ERROR_DURING_FLUSH_LOGS;
    }
  }
  if (thread_mask & (SLAVE_IO | SLAVE_FORCE_ALL)) {
    DBUG_PRINT("info", ("Terminating IO thread"));
    mi->abort_slave = true;
    DBUG_EXECUTE_IF("pause_after_queue_event",
                    { rpl_slave_debug_point(DBUG_RPL_S_PAUSE_QUEUE_EV); });
    /*
      If the I/O thread is running and waiting for disk space,
      the signal above will not make it to stop.
    */
    bool io_waiting_disk_space =
        mi->slave_running && mi->info_thd->is_waiting_for_disk_space();

    /*
      If we are shutting down the server and the I/O thread is waiting for
      disk space, tell the terminate_slave_thread to forcefully kill the I/O
      thread by sending a KILL_CONNECTION signal that will be listened by
      my_write function.
    */
    bool force_io_stop =
        io_waiting_disk_space && (thread_mask & SLAVE_FORCE_ALL);

    // If not shutting down, let the user to decide to abort I/O thread or wait
    if (io_waiting_disk_space && !force_io_stop) {
      LogErr(WARNING_LEVEL, ER_STOP_SLAVE_IO_THREAD_DISK_SPACE,
             mi->get_channel());
      DBUG_EXECUTE_IF("simulate_io_thd_wait_for_disk_space",
                      { rpl_slave_debug_point(DBUG_RPL_S_IO_WAIT_FOR_SPACE); });
    }

    if ((error = terminate_slave_thread(
             mi->info_thd, io_lock, &mi->stop_cond, &mi->slave_running,
             &total_stop_wait_timeout, need_lock_term, force_io_stop)) &&
        !force_all) {
      if (error == 1) {
        return ER_STOP_SLAVE_IO_THREAD_TIMEOUT;
      }
      return error;
    }

#ifndef DBUG_OFF
    if (force_io_stop) {
      if (DBUG_EVALUATE_IF("simulate_io_thd_wait_for_disk_space", 1, 0)) {
        DBUG_SET("-d,simulate_io_thd_wait_for_disk_space");
      }
    }
#endif

    mysql_mutex_lock(log_lock);

    DBUG_PRINT("info", ("Flushing relay log and master info repository."));
    if (current_thd)
      THD_STAGE_INFO(current_thd,
                     stage_flushing_relay_log_and_master_info_repository);

    /*
      Flushes the master info regardles of the sync_master_info option.
    */
    mysql_mutex_lock(&mi->data_lock);
    if (mi->flush_info(true)) {
      mysql_mutex_unlock(&mi->data_lock);
      mysql_mutex_unlock(log_lock);
      return ER_ERROR_DURING_FLUSH_LOGS;
    }
    mysql_mutex_unlock(&mi->data_lock);

    /*
      Flushes the relay log regardles of the sync_relay_log option.
    */
    if (mi->rli->relay_log.is_open() &&
        mi->rli->relay_log.flush_and_sync(true)) {
      mysql_mutex_unlock(log_lock);
      return ER_ERROR_DURING_FLUSH_LOGS;
    }

    mysql_mutex_unlock(log_lock);
  }
  return 0;
}

/**
   Wait for a slave thread to terminate.

   This function is called after requesting the thread to terminate
   (by setting @c abort_slave member of @c Relay_log_info or @c
   Master_info structure to 1). Termination of the thread is
   controlled with the the predicate <code>*slave_running</code>.

   Function will acquire @c term_lock before waiting on the condition
   unless @c need_lock_term is false in which case the mutex should be
   owned by the caller of this function and will remain acquired after
   return from the function.

   @param thd
          Current session.
   @param term_lock
          Associated lock to use when waiting for @c term_cond

   @param term_cond
          Condition that is signalled when the thread has terminated

   @param slave_running
          Pointer to predicate to check for slave thread termination

   @param stop_wait_timeout
          A pointer to a variable that denotes the time the thread has
          to stop before we time out and throw an error.

   @param need_lock_term
          If @c false the lock will not be acquired before waiting on
          the condition. In this case, it is assumed that the calling
          function acquires the lock before calling this function.

   @param force
          Force the slave thread to stop by sending a KILL_CONNECTION
          signal to it. This is used to forcefully stop the I/O thread
          when it is waiting for disk space and the server is shutting
          down.

   @retval 0 All OK, 1 on "STOP SLAVE" command timeout,
   ER_SLAVE_CHANNEL_NOT_RUNNING otherwise.

   @note  If the executing thread has to acquire term_lock
          (need_lock_term is true, the negative running status does not
          represent any issue therefore no error is reported.

 */
static int terminate_slave_thread(THD *thd, mysql_mutex_t *term_lock,
                                  mysql_cond_t *term_cond,
                                  std::atomic<uint> *slave_running,
                                  ulong *stop_wait_timeout, bool need_lock_term,
                                  bool force) {
  DBUG_TRACE;
  if (need_lock_term) {
    mysql_mutex_lock(term_lock);
  } else {
    mysql_mutex_assert_owner(term_lock);
  }
  if (!*slave_running) {
    if (need_lock_term) {
      /*
        if run_lock (term_lock) is acquired locally then either
        slave_running status is fine
      */
      mysql_mutex_unlock(term_lock);
      return 0;
    } else {
      return ER_SLAVE_CHANNEL_NOT_RUNNING;
    }
  }
  DBUG_ASSERT(thd != nullptr);
  THD_CHECK_SENTRY(thd);

  /*
    Is is critical to test if the slave is running. Otherwise, we might
    be referening freed memory trying to kick it
  */

  while (*slave_running)  // Should always be true
  {
    DBUG_PRINT("loop", ("killing slave thread"));

    mysql_mutex_lock(&thd->LOCK_thd_data);
    /*
      Error codes from pthread_kill are:
      EINVAL: invalid signal number (can't happen)
      ESRCH: thread already killed (can happen, should be ignored)
    */
#ifndef _WIN32
    int err MY_ATTRIBUTE((unused)) = pthread_kill(thd->real_id, SIGALRM);
    DBUG_ASSERT(err != EINVAL);
#endif
    if (force)
      thd->awake(THD::KILL_CONNECTION);
    else
      thd->awake(THD::NOT_KILLED);
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    /*
      There is a small chance that slave thread might miss the first
      alarm. To protect againts it, resend the signal until it reacts
    */
    struct timespec abstime;
    set_timespec(&abstime, 2);
#ifndef DBUG_OFF
    int error =
#endif
        mysql_cond_timedwait(term_cond, term_lock, &abstime);
    if ((*stop_wait_timeout) >= 2)
      (*stop_wait_timeout) = (*stop_wait_timeout) - 2;
    else if (*slave_running) {
      if (need_lock_term) mysql_mutex_unlock(term_lock);
      return 1;
    }
    DBUG_ASSERT(error == ETIMEDOUT || error == 0);
  }

  DBUG_ASSERT(*slave_running == 0);

  if (need_lock_term) mysql_mutex_unlock(term_lock);
  return 0;
}

bool start_slave_thread(
#ifdef HAVE_PSI_THREAD_INTERFACE
    PSI_thread_key thread_key,
#endif
    my_start_routine h_func, mysql_mutex_t *start_lock,
    mysql_mutex_t *cond_lock, mysql_cond_t *start_cond,
    std::atomic<uint> *slave_running, std::atomic<ulong> *slave_run_id,
    Master_info *mi) {
  bool is_error = false;
  my_thread_handle th;
  ulong start_id;
  DBUG_TRACE;

  if (start_lock) mysql_mutex_lock(start_lock);
  if (!server_id) {
    if (start_cond) mysql_cond_broadcast(start_cond);
    LogErr(ERROR_LEVEL, ER_RPL_SERVER_ID_MISSING, mi->get_for_channel_str());
    my_error(ER_BAD_SLAVE, MYF(0));
    goto err;
  }

  if (*slave_running) {
    if (start_cond) mysql_cond_broadcast(start_cond);
    my_error(ER_SLAVE_CHANNEL_MUST_STOP, MYF(0), mi->get_channel());
    goto err;
  }
  start_id = *slave_run_id;
  DBUG_PRINT("info", ("Creating new slave thread"));
  if (mysql_thread_create(thread_key, &th, &connection_attrib, h_func,
                          (void *)mi)) {
    LogErr(ERROR_LEVEL, ER_RPL_CANT_CREATE_SLAVE_THREAD,
           mi->get_for_channel_str());
    my_error(ER_SLAVE_THREAD, MYF(0));
    goto err;
  }
  if (start_cond && cond_lock)  // caller has cond_lock
  {
    THD *thd = current_thd;
    while (start_id == *slave_run_id && thd != nullptr) {
      DBUG_PRINT("sleep", ("Waiting for slave thread to start"));
      PSI_stage_info saved_stage = {0, "", 0, ""};
      thd->ENTER_COND(start_cond, cond_lock,
                      &stage_waiting_for_slave_thread_to_start, &saved_stage);
      /*
        It is not sufficient to test this at loop bottom. We must test
        it after registering the mutex in enter_cond(). If the kill
        happens after testing of thd->killed and before the mutex is
        registered, we could otherwise go waiting though thd->killed is
        set.
      */
      if (!thd->killed) mysql_cond_wait(start_cond, cond_lock);
      mysql_mutex_unlock(cond_lock);
      thd->EXIT_COND(&saved_stage);
      mysql_mutex_lock(cond_lock);  // re-acquire it
      if (thd->killed) {
        my_error(thd->killed, MYF(0));
        goto err;
      }
    }
  }

  goto end;
err:
  is_error = true;
end:

  if (start_lock) mysql_mutex_unlock(start_lock);
  return is_error;
}

/*
  start_slave_threads()

  NOTES
    SLAVE_FORCE_ALL is not implemented here on purpose since it does not make
    sense to do that for starting a slave--we always care if it actually
    started the threads that were not previously running
*/

bool start_slave_threads(bool need_lock_slave, bool wait_for_start,
                         Master_info *mi, int thread_mask) {
  mysql_mutex_t *lock_io = nullptr, *lock_sql = nullptr,
                *lock_cond_io = nullptr, *lock_cond_sql = nullptr;
  mysql_cond_t *cond_io = nullptr, *cond_sql = nullptr;
  bool is_error = false;
  DBUG_TRACE;
  DBUG_EXECUTE_IF("uninitialized_master-info_structure", mi->inited = false;);

  if (!mi->inited || !mi->rli->inited) {
    int error = (!mi->inited ? ER_SLAVE_MI_INIT_REPOSITORY
                             : ER_SLAVE_RLI_INIT_REPOSITORY);

    if (enable_raft_plugin) {
      // NO_LINT_DEBUG
      sql_print_error("start_slave_threads: error: %d mi_inited: %d", error,
                      mi->inited);
    }
    Rpl_info *info = (!mi->inited ? mi : static_cast<Rpl_info *>(mi->rli));
    const char *prefix = current_thd ? ER_THD_NONCONST(current_thd, error)
                                     : ER_DEFAULT_NONCONST(error);
    info->report(ERROR_LEVEL,
                 (!mi->inited ? ER_SERVER_SLAVE_MI_INIT_REPOSITORY
                              : ER_SERVER_SLAVE_RLI_INIT_REPOSITORY),
                 prefix, nullptr);
    my_error(error, MYF(0));
    return true;
  }

  if (mi->is_auto_position() && (thread_mask & SLAVE_IO) &&
      mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE) == GTID_MODE_OFF) {
    my_error(ER_CANT_USE_AUTO_POSITION_WITH_GTID_MODE_OFF, MYF(0),
             mi->get_for_channel_str());
    return true;
  }

  if (need_lock_slave) {
    lock_io = &mi->run_lock;
    lock_sql = &mi->rli->run_lock;
  }
  if (wait_for_start) {
    cond_io = &mi->start_cond;
    cond_sql = &mi->rli->start_cond;
    lock_cond_io = &mi->run_lock;
    lock_cond_sql = &mi->rli->run_lock;
  }

  if ((thread_mask & SLAVE_IO) && !enable_raft_plugin)
    is_error = start_slave_thread(
#ifdef HAVE_PSI_THREAD_INTERFACE
        key_thread_slave_io,
#endif
        handle_slave_io, lock_io, lock_cond_io, cond_io, &mi->slave_running,
        &mi->slave_run_id, mi);
  if (!is_error && (thread_mask & SLAVE_SQL)) {
    /*
      MTS-recovery gaps gathering is placed onto common execution path
      for either START-SLAVE and --skip-start-slave= 0
    */
    if (mi->rli->recovery_parallel_workers != 0) {
      if (mts_recovery_groups(mi->rli)) {
        is_error = true;
        my_error(ER_MTS_RECOVERY_FAILURE, MYF(0));
      }
    }
    if (!is_error)
      is_error = start_slave_thread(
#ifdef HAVE_PSI_THREAD_INTERFACE
          key_thread_slave_sql,
#endif
          handle_slave_sql, lock_sql, lock_cond_sql, cond_sql,
          &mi->rli->slave_running, &mi->rli->slave_run_id, mi);
    if (is_error)
      terminate_slave_threads(mi, thread_mask & SLAVE_IO,
                              rpl_stop_slave_timeout, need_lock_slave);
  }
  return is_error;
}

/*
  Release slave threads at time of executing shutdown.

  SYNOPSIS
    end_slave()
*/

void end_slave() {
  DBUG_TRACE;

  Master_info *mi = nullptr;

  /*
    This is called when the server terminates, in close_connections().
    It terminates slave threads. However, some CHANGE MASTER etc may still be
    running presently. If a START SLAVE was in progress, the mutex lock below
    will make us wait until slave threads have started, and START SLAVE
    returns, then we terminate them here.
  */
  channel_map.wrlock();

  /* traverse through the map and terminate the threads */
  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;

    if (mi)
      terminate_slave_threads(mi, SLAVE_FORCE_ALL, rpl_stop_slave_timeout);
  }
  channel_map.unlock();
}

/**
   Free all resources used by slave threads at time of executing shutdown.
   The routine must be called after all possible users of channel_map
   have left.

*/
void delete_slave_info_objects() {
  DBUG_TRACE;

  Master_info *mi = nullptr;

  channel_map.wrlock();

  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;

    if (mi) {
      mi->channel_wrlock();
      end_info(mi);
      if (mi->rli) delete mi->rli;
      delete mi;
      it->second = 0;
    }
  }

  // Clean other types of channel
  for (mi_map::iterator it = channel_map.begin(GROUP_REPLICATION_CHANNEL);
       it != channel_map.end(GROUP_REPLICATION_CHANNEL); it++) {
    mi = it->second;

    if (mi) {
      mi->channel_wrlock();
      end_info(mi);
      if (mi->rli) delete mi->rli;
      delete mi;
      it->second = 0;
    }
  }

  channel_map.unlock();
}

/**
   Check if multi-statement transaction mode and master and slave info
   repositories are set to table.

   @param thd    THD object

   @retval true  Success
   @retval false Failure
*/
static bool is_autocommit_off_and_infotables(THD *thd) {
  DBUG_TRACE;
  return (thd && thd->in_multi_stmt_transaction_mode() &&
          (opt_mi_repository_id == INFO_REPOSITORY_TABLE ||
           opt_rli_repository_id == INFO_REPOSITORY_TABLE))
             ? true
             : false;
}

static bool io_slave_killed(THD *thd, Master_info *mi) {
  DBUG_TRACE;

  DBUG_ASSERT(mi->info_thd == thd);
  DBUG_ASSERT(mi->slave_running);  // tracking buffer overrun
  return mi->abort_slave || connection_events_loop_aborted() || thd->killed;
}

/**
   The function analyzes a possible killed status and makes
   a decision whether to accept it or not.
   Normally upon accepting the sql thread goes to shutdown.
   In the event of deferring decision @c rli->last_event_start_time waiting
   timer is set to force the killed status be accepted upon its expiration.

   Notice Multi-Threaded-Slave behaves similarly in that when it's being
   stopped and the current group of assigned events has not yet scheduled
   completely, Coordinator defers to accept to leave its read-distribute
   state. The above timeout ensures waiting won't last endlessly, and in
   such case an error is reported.

   @param thd   pointer to a THD instance
   @param rli   pointer to Relay_log_info instance

   @return true the killed status is recognized, false a possible killed
           status is deferred.
*/
bool sql_slave_killed(THD *thd, Relay_log_info *rli) {
  bool is_parallel_warn = false;

  DBUG_TRACE;

  DBUG_ASSERT(rli->info_thd == thd);
  DBUG_ASSERT(rli->slave_running == 1);
  if (rli->sql_thread_kill_accepted) return true;
  DBUG_EXECUTE_IF("stop_when_mts_in_group", rli->abort_slave = 1;
                  DBUG_SET("-d,stop_when_mts_in_group");
                  DBUG_SET("-d,simulate_stop_when_mts_in_group");
                  return false;);
  if (connection_events_loop_aborted() || thd->killed || rli->abort_slave) {
    rli->sql_thread_kill_accepted = true;
    /* NOTE: In MTS mode if all workers are done and if the partial trx
       (if any) can be rollbacked safely we can accept the kill */
    const bool can_rollback =
        rli->abort_slave &&
        (!rli->is_mts_in_group() ||
         (rli->mts_workers_queue_empty() && !rli->cannot_safely_rollback()));
    is_parallel_warn =
        (rli->is_parallel_exec() && (!can_rollback || thd->killed));
    /*
      Slave can execute stop being in one of two MTS or Single-Threaded mode.
      The modes define different criteria to accept the stop.
      In particular that relates to the concept of groupping.
      Killed Coordinator thread expects the worst so it warns on
      possible consistency issue.
    */
    if (is_parallel_warn || (!rli->is_parallel_exec() &&
                             thd->get_transaction()->cannot_safely_rollback(
                                 Transaction_ctx::SESSION) &&
                             rli->is_in_group())) {
      char msg_stopped[] =
          "... Slave SQL Thread stopped with incomplete event group "
          "having non-transactional changes. "
          "If the group consists solely of row-based events, you can try "
          "to restart the slave with --slave-exec-mode=IDEMPOTENT, which "
          "ignores duplicate key, key not found, and similar errors (see "
          "documentation for details).";
      char msg_stopped_mts[] =
          "... The slave coordinator and worker threads are stopped, possibly "
          "leaving data in inconsistent state. A restart should "
          "restore consistency automatically, although using non-transactional "
          "storage for data or info tables or DDL queries could lead to "
          "problems. "
          "In such cases you have to examine your data (see documentation for "
          "details).";

      if (rli->abort_slave) {
        DBUG_PRINT("info",
                   ("Request to stop slave SQL Thread received while "
                    "applying an MTS group or a group that "
                    "has non-transactional "
                    "changes; waiting for completion of the group ... "));

        /*
          Slave sql thread shutdown in face of unfinished group modified
          Non-trans table is handled via a timer. The slave may eventually
          give out to complete the current group and in that case there
          might be issues at consequent slave restart, see the error message.
          WL#2975 offers a robust solution requiring to store the last exectuted
          event's coordinates along with the group's coordianates
          instead of waiting with @c last_event_start_time the timer.
        */

        if (rli->last_event_start_time == 0)
          rli->last_event_start_time = my_time(0);
        rli->sql_thread_kill_accepted =
            difftime(my_time(0), rli->last_event_start_time) <=
                    SLAVE_WAIT_GROUP_DONE
                ? false
                : true;

        DBUG_EXECUTE_IF("stop_slave_middle_group",
                        DBUG_EXECUTE_IF("incomplete_group_in_relay_log",
                                        rli->sql_thread_kill_accepted =
                                            true;););  // time is over

        if (!rli->sql_thread_kill_accepted && !rli->reported_unsafe_warning) {
          rli->report(
              WARNING_LEVEL, 0,
              !is_parallel_warn
                  ? "Request to stop slave SQL Thread received while "
                    "applying a group that has non-transactional "
                    "changes; waiting for completion of the group ... "
                  : "Coordinator thread of multi-threaded slave is being "
                    "stopped in the middle of assigning a group of events; "
                    "deferring to exit until the group completion ... ");
          rli->reported_unsafe_warning = true;
        }
      }
      if (rli->sql_thread_kill_accepted) {
        if (rli->mts_group_status == Relay_log_info::MTS_IN_GROUP) {
          rli->mts_group_status = Relay_log_info::MTS_KILLED_GROUP;
        }
        if (is_parallel_warn)
          rli->report(!rli->is_error()
                          ? ERROR_LEVEL
                          : WARNING_LEVEL,  // an error was reported by Worker
                      ER_MTS_INCONSISTENT_DATA,
                      ER_THD(thd, ER_MTS_INCONSISTENT_DATA), msg_stopped_mts);
        else
          rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                      ER_THD(thd, ER_SLAVE_FATAL_ERROR), msg_stopped);
      }
    }
  }

  if (rli->sql_thread_kill_accepted) rli->last_event_start_time = 0;

  return rli->sql_thread_kill_accepted;
}

bool net_request_file(NET *net, const char *fname) {
  DBUG_TRACE;
  return net_write_command(net, 251, pointer_cast<const uchar *>(fname),
                           strlen(fname), pointer_cast<const uchar *>(""), 0);
}

/*
  From other comments and tests in code, it looks like
  sometimes Query_log_event and Load_log_event can have db == 0
  (see rewrite_db() above for example)
  (cases where this happens are unclear; it may be when the master is 3.23).
*/

const char *print_slave_db_safe(const char *db) {
  DBUG_TRACE;

  return (db ? db : "");
}

/*
  Check if the error is caused by network.
  @param[in]   errorno   Number of the error.
  RETURNS:
  true         network error
  false        not network error
*/

static bool is_network_error(uint errorno) {
  return errorno == CR_CONNECTION_ERROR || errorno == CR_CONN_HOST_ERROR ||
         errorno == CR_SERVER_GONE_ERROR || errorno == CR_SERVER_LOST ||
         errorno == ER_CON_COUNT_ERROR || errorno == ER_SERVER_SHUTDOWN ||
         errorno == ER_NET_READ_INTERRUPTED ||
         errorno == ER_NET_WRITE_INTERRUPTED;
}

enum enum_command_status {
  COMMAND_STATUS_OK,
  COMMAND_STATUS_ERROR,
  COMMAND_STATUS_ALLOWED_ERROR
};
/**
  Execute an initialization query for the IO thread.

  If there is an error, then this function calls mysql_free_result;
  otherwise the MYSQL object holds the result after this call.  If
  there is an error other than allowed_error, then this function
  prints a message and returns -1.

  @param mi Master_info object.
  @param query Query string.
  @param allowed_error Allowed error code, or 0 if no errors are allowed.
  @param[out] master_res If this is not NULL and there is no error, then
  mysql_store_result() will be called and the result stored in this pointer.
  @param[out] master_row If this is not NULL and there is no error, then
  mysql_fetch_row() will be called and the result stored in this pointer.

  @retval COMMAND_STATUS_OK No error.
  @retval COMMAND_STATUS_ALLOWED_ERROR There was an error and the
  error code was 'allowed_error'.
  @retval COMMAND_STATUS_ERROR There was an error and the error code
  was not 'allowed_error'.
*/
static enum_command_status io_thread_init_command(
    Master_info *mi, const char *query, int allowed_error,
    MYSQL_RES **master_res = nullptr, MYSQL_ROW *master_row = nullptr) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("IO thread initialization command: '%s'", query));
  MYSQL *mysql = mi->mysql;
  int ret = mysql_real_query(mysql, query, static_cast<ulong>(strlen(query)));
  if (io_slave_killed(mi->info_thd, mi)) {
    LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_IO_THREAD_WAS_KILLED,
           mi->get_for_channel_str(), query);
    mysql_free_result(mysql_store_result(mysql));
    return COMMAND_STATUS_ERROR;
  }
  if (ret != 0) {
    int err = mysql_errno(mysql);
    mysql_free_result(mysql_store_result(mysql));
    if (!err || err != allowed_error) {
      mi->report(is_network_error(err) ? WARNING_LEVEL : ERROR_LEVEL, err,
                 "The slave IO thread stops because the initialization query "
                 "'%s' failed with error '%s'.",
                 query, mysql_error(mysql));
      return COMMAND_STATUS_ERROR;
    }
    return COMMAND_STATUS_ALLOWED_ERROR;
  }
  if (master_res != nullptr) {
    if ((*master_res = mysql_store_result(mysql)) == nullptr) {
      mi->report(WARNING_LEVEL, mysql_errno(mysql),
                 "The slave IO thread stops because the initialization query "
                 "'%s' did not return any result.",
                 query);
      return COMMAND_STATUS_ERROR;
    }
    if (master_row != nullptr) {
      if ((*master_row = mysql_fetch_row(*master_res)) == nullptr) {
        mysql_free_result(*master_res);
        mi->report(WARNING_LEVEL, mysql_errno(mysql),
                   "The slave IO thread stops because the initialization query "
                   "'%s' did not return any row.",
                   query);
        return COMMAND_STATUS_ERROR;
      }
    }
  } else
    DBUG_ASSERT(master_row == nullptr);
  return COMMAND_STATUS_OK;
}

/**
  Set user variables after connecting to the master.

  @param  mysql MYSQL to request uuid from master.
  @param  mi    Master_info to set master_uuid

  @return 0: Success, 1: Fatal error, 2: Transient network error.
 */
int io_thread_init_commands(MYSQL *mysql, Master_info *mi) {
  char query[256];
  int ret = 0;
  DBUG_EXECUTE_IF("fake_5_5_version_slave", return ret;);

  snprintf(query, sizeof(query),
           "SET "
           "@slave_uuid= '%s',"
           "@dump_thread_wait_sleep_usec= %llu",
           server_uuid, opt_slave_dump_thread_wait_sleep_usec);
  if (mysql_real_query(mysql, query, static_cast<ulong>(strlen(query))) &&
      !check_io_slave_killed(mi->info_thd, mi, nullptr))
    goto err;

  mysql_free_result(mysql_store_result(mysql));
  return ret;

err:
  if (mysql_errno(mysql) && is_network_error(mysql_errno(mysql))) {
    mi->report(WARNING_LEVEL, mysql_errno(mysql),
               "The initialization command '%s' failed with the following"
               " error: '%s'.",
               query, mysql_error(mysql));
    ret = 2;
  } else {
    char errmsg[512];
    const char *errmsg_fmt =
        "The slave I/O thread stops because a fatal error is encountered "
        "when it tries to send query to master(query: %s).";

    sprintf(errmsg, errmsg_fmt, query);
    mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
               ER_THD(current_thd, ER_SLAVE_FATAL_ERROR), errmsg);
    ret = 1;
  }
  mysql_free_result(mysql_store_result(mysql));
  return ret;
}

/**
  Get master's uuid on connecting.

  @param  mysql MYSQL to request uuid from master.
  @param  mi    Master_info to set master_uuid

  @return 0: Success, 1: Fatal error, 2: Transient network error.
*/
static int get_master_uuid(MYSQL *mysql, Master_info *mi) {
  const char *errmsg;
  MYSQL_RES *master_res = nullptr;
  MYSQL_ROW master_row = nullptr;
  int ret = 0;
  char query_buf[] = "SELECT @@GLOBAL.SERVER_UUID";

  DBUG_EXECUTE_IF("dbug.return_null_MASTER_UUID", {
    mi->master_uuid[0] = 0;
    return 0;
  };);

  DBUG_EXECUTE_IF("dbug.before_get_MASTER_UUID",
                  { rpl_slave_debug_point(DBUG_RPL_S_BEFORE_MASTER_UUID); };);

  DBUG_EXECUTE_IF("dbug.simulate_busy_io",
                  { rpl_slave_debug_point(DBUG_RPL_S_SIMULATE_BUSY_IO); };);
#ifndef DBUG_OFF
  DBUG_EXECUTE_IF("dbug.simulate_no_such_var_server_uuid", {
    query_buf[strlen(query_buf) - 1] = '_';  // currupt the last char
  });
#endif
  if (!mysql_real_query(mysql, STRING_WITH_LEN(query_buf)) &&
      (master_res = mysql_store_result(mysql)) &&
      (master_row = mysql_fetch_row(master_res))) {
    if (!strcmp(::server_uuid, master_row[0]) &&
        !mi->rli->replicate_same_server_id) {
      errmsg =
          "The slave I/O thread stops because master and slave have equal "
          "MySQL server UUIDs; these UUIDs must be different for "
          "replication to work.";
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 ER_THD(current_thd, ER_SLAVE_FATAL_ERROR), errmsg);
      // Fatal error
      ret = 1;
    } else {
      if (mi->master_uuid[0] != 0 && strcmp(mi->master_uuid, master_row[0]))
        LogErr(WARNING_LEVEL, ER_RPL_SLAVE_MASTER_UUID_HAS_CHANGED,
               mi->master_uuid);
      strncpy(mi->master_uuid, master_row[0], UUID_LENGTH);
      mi->master_uuid[UUID_LENGTH] = 0;
    }
  } else if (mysql_errno(mysql) != ER_UNKNOWN_SYSTEM_VARIABLE) {
    if (is_network_error(mysql_errno(mysql))) {
      mi->report(WARNING_LEVEL, mysql_errno(mysql),
                 "Get master SERVER_UUID failed with error: %s",
                 mysql_error(mysql));
      ret = 2;
    } else {
      /* Fatal error */
      errmsg =
          "The slave I/O thread stops because a fatal error is encountered "
          "when it tries to get the value of SERVER_UUID variable from master.";
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 ER_THD(current_thd, ER_SLAVE_FATAL_ERROR), errmsg);
      ret = 1;
    }
  } else {
    mi->master_uuid[0] = 0;
    mi->report(
        WARNING_LEVEL, ER_UNKNOWN_SYSTEM_VARIABLE,
        "Unknown system variable 'SERVER_UUID' on master. "
        "A probable cause is that the variable is not supported on the "
        "master (version: %s), even though it is on the slave (version: %s)",
        mysql->server_version, server_version);
  }

  if (master_res) mysql_free_result(master_res);
  return ret;
}

/**
  Determine, case-sensitively, if short_string is equal to
  long_string, or a true prefix of long_string, or not a prefix.

  @retval 0 short_string is not a prefix of long_string.
  @retval 1 short_string is a true prefix of long_string (not equal).
  @retval 2 short_string is equal to long_string.
*/
static int is_str_prefix_case(const char *short_string,
                              const char *long_string) {
  int i;
  for (i = 0; short_string[i]; i++)
    if (my_toupper(system_charset_info, short_string[i]) !=
        my_toupper(system_charset_info, long_string[i]))
      return 0;
  return long_string[i] ? 1 : 2;
}

/*
  Note that we rely on the master's version (3.23, 4.0.14 etc) instead of
  relying on the binlog's version. This is not perfect: imagine an upgrade
  of the master without waiting that all slaves are in sync with the master;
  then a slave could be fooled about the binlog's format. This is what happens
  when people upgrade a 3.23 master to 4.0 without doing RESET MASTER: 4.0
  slaves are fooled. So we do this only to distinguish between 3.23 and more
  recent masters (it's too late to change things for 3.23).

  RETURNS
  0       ok
  1       error
  2       transient network problem, the caller should try to reconnect
*/

static int get_master_version_and_clock(MYSQL *mysql, Master_info *mi) {
  char err_buff[MAX_SLAVE_ERRMSG];
  const char *errmsg = nullptr;
  int err_code = 0;
  int version_number = 0;
  version_number = atoi(mysql->server_version);

  MYSQL_RES *master_res = nullptr;
  MYSQL_ROW master_row;
  DBUG_TRACE;

  DBUG_EXECUTE_IF("unrecognized_master_version", { version_number = 1; };);

  if (!my_isdigit(&my_charset_bin, *mysql->server_version) ||
      version_number < 5) {
    errmsg = "Master reported unrecognized MySQL version";
    err_code = ER_SLAVE_FATAL_ERROR;
    sprintf(err_buff, ER_THD_NONCONST(current_thd, err_code), errmsg);
    goto err;
  }

  mysql_mutex_lock(mi->rli->relay_log.get_log_lock());
  mysql_mutex_lock(&mi->data_lock);
  mi->set_mi_description_event(new Format_description_log_event());
  /* as we are here, we tried to allocate the event */
  if (mi->get_mi_description_event() == nullptr) {
    mysql_mutex_unlock(&mi->data_lock);
    mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());
    errmsg = "default Format_description_log_event";
    err_code = ER_SLAVE_CREATE_EVENT_FAILURE;
    sprintf(err_buff, ER_THD_NONCONST(current_thd, err_code), errmsg);
    goto err;
  }

  /*
    FD_q's (A) is set initially from RL's (A): FD_q.(A) := RL.(A).
    It's necessary to adjust FD_q.(A) at this point because in the following
    course FD_q is going to be dumped to RL.
    Generally FD_q is derived from a received FD_m (roughly FD_q := FD_m)
    in queue_event and the master's (A) is installed.
    At one step with the assignment the Relay-Log's checksum alg is set to
    a new value: RL.(A) := FD_q.(A). If the slave service is stopped
    the last time assigned RL.(A) will be passed over to the restarting
    service (to the current execution point).
    RL.A is a "codec" to verify checksum in queue_event() almost all the time
    the first fake Rotate event.
    Starting from this point IO thread will executes the following checksum
    warmup sequence  of actions:

    FD_q.A := RL.A,
    A_m^0 := master.@@global.binlog_checksum,
    {queue_event(R_f): verifies(R_f, A_m^0)},
    {queue_event(FD_m): verifies(FD_m, FD_m.A), dump(FD_q), rotate(RL),
                        FD_q := FD_m, RL.A := FD_q.A)}

    See legends definition on MYSQL_BIN_LOG::relay_log_checksum_alg
    docs lines (binlog.h).
    In above A_m^0 - the value of master's
    @@binlog_checksum determined in the upcoming handshake (stored in
    mi->checksum_alg_before_fd).


    After the warm-up sequence IO gets to "normal" checksum verification mode
    to use RL.A in

    {queue_event(E_m): verifies(E_m, RL.A)}

    until it has received a new FD_m.
  */
  mi->get_mi_description_event()->common_footer->checksum_alg =
      mi->rli->relay_log.relay_log_checksum_alg;

  DBUG_ASSERT(mi->get_mi_description_event()->common_footer->checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_UNDEF);
  DBUG_ASSERT(mi->rli->relay_log.relay_log_checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_UNDEF);

  mi->clock_diff_with_master = 0; /* The "most sensible" value */
  mysql_mutex_unlock(&mi->data_lock);
  mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());

  /*
    Check that the master's server id and ours are different. Because if they
    are equal (which can result from a simple copy of master's datadir to slave,
    thus copying some my.cnf), replication will work but all events will be
    skipped.
    Do not die if SELECT @@SERVER_ID fails on master (very old master?).
    Note: we could have put a @@SERVER_ID in the previous SELECT
    UNIX_TIMESTAMP() instead, but this would not have worked on 3.23 masters.
  */
  DBUG_EXECUTE_IF("dbug.before_get_SERVER_ID",
                  { rpl_slave_debug_point(DBUG_RPL_S_BEFORE_SERVER_ID); };);
  master_res = nullptr;
  master_row = nullptr;
  DBUG_EXECUTE_IF("get_master_server_id.ER_NET_READ_INTERRUPTED", {
    DBUG_SET("+d,inject_ER_NET_READ_INTERRUPTED");
    DBUG_SET(
        "-d,get_master_server_id."
        "ER_NET_READ_INTERRUPTED");
  });
  if (!mysql_real_query(mysql, STRING_WITH_LEN("SELECT @@GLOBAL.SERVER_ID")) &&
      (master_res = mysql_store_result(mysql)) &&
      (master_row = mysql_fetch_row(master_res))) {
    if ((::server_id ==
         (mi->master_id = strtoul(master_row[0], nullptr, 10))) &&
        !mi->rli->replicate_same_server_id) {
      errmsg =
          "The slave I/O thread stops because master and slave have equal \
MySQL server ids; these ids must be different for replication to work (or \
the --replicate-same-server-id option must be used on slave but this does \
not always make sense; please check the manual before using it).";
      err_code = ER_SLAVE_FATAL_ERROR;
      sprintf(err_buff, ER_THD(current_thd, ER_SLAVE_FATAL_ERROR), errmsg);
      goto err;
    }
  } else if (mysql_errno(mysql) != ER_UNKNOWN_SYSTEM_VARIABLE) {
    if (check_io_slave_killed(mi->info_thd, mi, nullptr))
      goto slave_killed_err;
    else if (is_network_error(mysql_errno(mysql))) {
      mi->report(WARNING_LEVEL, mysql_errno(mysql),
                 "Get master SERVER_ID failed with error: %s",
                 mysql_error(mysql));
      goto network_err;
    }
    /* Fatal error */
    errmsg =
        "The slave I/O thread stops because a fatal error is encountered \
when it try to get the value of SERVER_ID variable from master.";
    err_code = mysql_errno(mysql);
    sprintf(err_buff, "%s Error: %s", errmsg, mysql_error(mysql));
    goto err;
  } else {
    mi->report(WARNING_LEVEL, ER_SERVER_UNKNOWN_SYSTEM_VARIABLE,
               "Unknown system variable 'SERVER_ID' on master, \
maybe it is a *VERY OLD MASTER*.");
  }
  if (master_res) {
    mysql_free_result(master_res);
    master_res = nullptr;
  }
  if (mi->master_id == 0 && mi->ignore_server_ids->dynamic_ids.size() > 0) {
    errmsg =
        "Slave configured with server id filtering could not detect the master "
        "server id.";
    err_code = ER_SLAVE_FATAL_ERROR;
    sprintf(err_buff, ER_THD(current_thd, ER_SLAVE_FATAL_ERROR), errmsg);
    goto err;
  }

  if (mi->heartbeat_period != 0.0) {
    char llbuf[22];
    const char query_format[] = "SET @master_heartbeat_period= %s";
    char query[sizeof(query_format) - 2 + sizeof(llbuf)];
    /*
       the period is an ulonglong of nano-secs.
    */
    llstr((ulonglong)(mi->heartbeat_period * 1000000000UL), llbuf);
    sprintf(query, query_format, llbuf);

    if (mysql_real_query(mysql, query, static_cast<ulong>(strlen(query)))) {
      if (check_io_slave_killed(mi->info_thd, mi, nullptr))
        goto slave_killed_err;

      if (is_network_error(mysql_errno(mysql))) {
        mi->report(
            WARNING_LEVEL, mysql_errno(mysql),
            "SET @master_heartbeat_period to master failed with error: %s",
            mysql_error(mysql));
        mysql_free_result(mysql_store_result(mysql));
        goto network_err;
      } else {
        /* Fatal error */
        errmsg =
            "The slave I/O thread stops because a fatal error is encountered "
            " when it tries to SET @master_heartbeat_period on master.";
        err_code = ER_SLAVE_FATAL_ERROR;
        sprintf(err_buff, "%s Error: %s", errmsg, mysql_error(mysql));
        mysql_free_result(mysql_store_result(mysql));
        goto err;
      }
    }
    mysql_free_result(mysql_store_result(mysql));
  }

  /*
    Querying if master is capable to checksum and notifying it about own
    CRC-awareness. The master's side instant value of @@global.binlog_checksum
    is stored in the dump thread's uservar area as well as cached locally
    to become known in consensus by master and slave.
  */
  if (DBUG_EVALUATE_IF("simulate_slave_unaware_checksum", 0, 1)) {
    int rc;
    const char query[] =
        "SET @master_binlog_checksum= @@global.binlog_checksum";
    master_res = nullptr;
    // initially undefined
    mi->checksum_alg_before_fd = binary_log::BINLOG_CHECKSUM_ALG_UNDEF;
    /*
      @c checksum_alg_before_fd is queried from master in this block.
      If master is old checksum-unaware the value stays undefined.
      Once the first FD will be received its alg descriptor will replace
      the being queried one.
    */
    rc = mysql_real_query(mysql, query, static_cast<ulong>(strlen(query)));
    if (rc != 0) {
      mi->checksum_alg_before_fd = binary_log::BINLOG_CHECKSUM_ALG_OFF;
      if (check_io_slave_killed(mi->info_thd, mi, nullptr))
        goto slave_killed_err;

      if (mysql_errno(mysql) == ER_UNKNOWN_SYSTEM_VARIABLE) {
        // this is tolerable as OM -> NS is supported
        mi->report(WARNING_LEVEL, mysql_errno(mysql),
                   "Notifying master by %s failed with "
                   "error: %s",
                   query, mysql_error(mysql));
      } else {
        if (is_network_error(mysql_errno(mysql))) {
          mi->report(WARNING_LEVEL, mysql_errno(mysql),
                     "Notifying master by %s failed with "
                     "error: %s",
                     query, mysql_error(mysql));
          mysql_free_result(mysql_store_result(mysql));
          goto network_err;
        } else {
          errmsg =
              "The slave I/O thread stops because a fatal error is encountered "
              "when it tried to SET @master_binlog_checksum on master.";
          err_code = ER_SLAVE_FATAL_ERROR;
          sprintf(err_buff, "%s Error: %s", errmsg, mysql_error(mysql));
          mysql_free_result(mysql_store_result(mysql));
          goto err;
        }
      }
    } else {
      mysql_free_result(mysql_store_result(mysql));
      if (!mysql_real_query(
              mysql, STRING_WITH_LEN("SELECT @master_binlog_checksum")) &&
          (master_res = mysql_store_result(mysql)) &&
          (master_row = mysql_fetch_row(master_res)) &&
          (master_row[0] != nullptr)) {
        mi->checksum_alg_before_fd = static_cast<enum_binlog_checksum_alg>(
            find_type(master_row[0], &binlog_checksum_typelib, 1) - 1);

        DBUG_EXECUTE_IF("undefined_algorithm_on_slave",
                        mi->checksum_alg_before_fd =
                            binary_log::BINLOG_CHECKSUM_ALG_UNDEF;);
        if (mi->checksum_alg_before_fd ==
            binary_log::BINLOG_CHECKSUM_ALG_UNDEF) {
          errmsg =
              "The slave I/O thread was stopped because a fatal error is "
              "encountered "
              "The checksum algorithm used by master is unknown to slave.";
          err_code = ER_SLAVE_FATAL_ERROR;
          sprintf(err_buff, "%s Error: %s", errmsg, mysql_error(mysql));
          mysql_free_result(mysql_store_result(mysql));
          goto err;
        }

        // valid outcome is either of
        DBUG_ASSERT(mi->checksum_alg_before_fd ==
                        binary_log::BINLOG_CHECKSUM_ALG_OFF ||
                    mi->checksum_alg_before_fd ==
                        binary_log::BINLOG_CHECKSUM_ALG_CRC32);
      } else if (check_io_slave_killed(mi->info_thd, mi, nullptr))
        goto slave_killed_err;
      else if (is_network_error(mysql_errno(mysql))) {
        mi->report(WARNING_LEVEL, mysql_errno(mysql),
                   "Get master BINLOG_CHECKSUM failed with error: %s",
                   mysql_error(mysql));
        goto network_err;
      } else {
        errmsg =
            "The slave I/O thread stops because a fatal error is encountered "
            "when it tried to SELECT @master_binlog_checksum.";
        err_code = ER_SLAVE_FATAL_ERROR;
        sprintf(err_buff, "%s Error: %s", errmsg, mysql_error(mysql));
        mysql_free_result(mysql_store_result(mysql));
        goto err;
      }
    }
    if (master_res) {
      mysql_free_result(master_res);
      master_res = nullptr;
    }
  } else
    mi->checksum_alg_before_fd = binary_log::BINLOG_CHECKSUM_ALG_OFF;

  if (DBUG_EVALUATE_IF("simulate_slave_unaware_gtid", 0, 1)) {
    enum_gtid_mode master_gtid_mode = GTID_MODE_OFF;
    enum_gtid_mode slave_gtid_mode =
        mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE);
    switch (io_thread_init_command(mi, "SELECT @@GLOBAL.GTID_MODE",
                                   ER_UNKNOWN_SYSTEM_VARIABLE, &master_res,
                                   &master_row)) {
      case COMMAND_STATUS_ERROR:
        return 2;
      case COMMAND_STATUS_ALLOWED_ERROR:
        // master is old and does not have @@GLOBAL.GTID_MODE
        master_gtid_mode = GTID_MODE_OFF;
        break;
      case COMMAND_STATUS_OK: {
        bool error = false;
        const char *master_gtid_mode_string = master_row[0];
        DBUG_EXECUTE_IF("simulate_master_has_gtid_mode_on_something",
                        { master_gtid_mode_string = "on_something"; });
        DBUG_EXECUTE_IF("simulate_master_has_gtid_mode_off_something",
                        { master_gtid_mode_string = "off_something"; });
        DBUG_EXECUTE_IF("simulate_master_has_unknown_gtid_mode",
                        { master_gtid_mode_string = "Krakel Spektakel"; });
        master_gtid_mode = get_gtid_mode(master_gtid_mode_string, &error);
        if (error) {
          // For potential future compatibility, allow unknown
          // GTID_MODEs that begin with ON/OFF (treating them as ON/OFF
          // respectively).
          enum_gtid_mode mode = GTID_MODE_OFF;
          for (int i = 0; i < 2; i++) {
            switch (is_str_prefix_case(get_gtid_mode_string(mode),
                                       master_gtid_mode_string)) {
              case 0:  // is not a prefix; continue loop
                break;
              case 1:  // is a true prefix, i.e. not equal
                mi->report(WARNING_LEVEL, ER_UNKNOWN_ERROR,
                           "The master uses an unknown GTID_MODE '%s'. "
                           "Treating it as '%s'.",
                           master_gtid_mode_string, get_gtid_mode_string(mode));
                // fall through
              case 2:  // is equal
                error = false;
                master_gtid_mode = mode;
                break;
            }
            mode = GTID_MODE_ON;
          }
        }
        if (error) {
          mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                     "The slave IO thread stops because the master has "
                     "an unknown @@GLOBAL.GTID_MODE '%s'.",
                     master_gtid_mode_string);
          mysql_free_result(master_res);
          return 1;
        }
        mysql_free_result(master_res);
        break;
      }
    }
    if ((slave_gtid_mode == GTID_MODE_OFF &&
         master_gtid_mode >= GTID_MODE_ON_PERMISSIVE) ||
        (slave_gtid_mode == GTID_MODE_ON &&
         master_gtid_mode <= GTID_MODE_OFF_PERMISSIVE)) {
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 "The replication receiver thread cannot start because "
                 "the master has GTID_MODE = %.192s and this server has "
                 "GTID_MODE = %.192s.",
                 get_gtid_mode_string(master_gtid_mode),
                 get_gtid_mode_string(slave_gtid_mode));
      return 1;
    }
    if (mi->is_auto_position() && master_gtid_mode != GTID_MODE_ON) {
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 "The replication receiver thread cannot start in "
                 "AUTO_POSITION mode: the master has GTID_MODE = %.192s "
                 "instead of ON.",
                 get_gtid_mode_string(master_gtid_mode));
      return 1;
    }
  }

err:
  if (errmsg) {
    if (master_res) mysql_free_result(master_res);
    DBUG_ASSERT(err_code != 0);
    mi->report(ERROR_LEVEL, err_code, "%s", err_buff);
    return 1;
  }

  return 0;

network_err:
  if (master_res) mysql_free_result(master_res);
  return 2;

slave_killed_err:
  if (master_res) mysql_free_result(master_res);
  return 2;
}

static bool wait_for_relay_log_space(Relay_log_info *rli) {
  bool slave_killed = false;
  Master_info *mi = rli->mi;
  PSI_stage_info old_stage;
  THD *thd = mi->info_thd;
  DBUG_TRACE;

  mysql_mutex_lock(&rli->log_space_lock);
  thd->ENTER_COND(&rli->log_space_cond, &rli->log_space_lock,
                  &stage_waiting_for_relay_log_space, &old_stage);
  while (rli->log_space_limit < rli->log_space_total &&
         !(slave_killed = io_slave_killed(thd, mi)) &&
         !rli->ignore_log_space_limit)
    mysql_cond_wait(&rli->log_space_cond, &rli->log_space_lock);

  /*
    Makes the IO thread read only one event at a time
    until the SQL thread is able to purge the relay
    logs, freeing some space.

    Therefore, once the SQL thread processes this next
    event, it goes to sleep (no more events in the queue),
    sets ignore_log_space_limit=true and wakes the IO thread.
    However, this event may have been enough already for
    the SQL thread to purge some log files, freeing
    rli->log_space_total .

    This guarantees that the SQL and IO thread move
    forward only one event at a time (to avoid deadlocks),
    when the relay space limit is reached. It also
    guarantees that when the SQL thread is prepared to
    rotate (to be able to purge some logs), the IO thread
    will know about it and will rotate.

    NOTE: The ignore_log_space_limit is only set when the SQL
          thread sleeps waiting for events.

   */
  if (rli->ignore_log_space_limit) {
#ifndef DBUG_OFF
    {
      char llbuf1[22], llbuf2[22];
      DBUG_PRINT("info", ("log_space_limit=%s "
                          "log_space_total=%s "
                          "ignore_log_space_limit=%d "
                          "sql_force_rotate_relay=%d",
                          llstr(rli->log_space_limit, llbuf1),
                          llstr(rli->log_space_total, llbuf2),
                          (int)rli->ignore_log_space_limit,
                          (int)rli->sql_force_rotate_relay));
    }
#endif
    if (rli->sql_force_rotate_relay) {
      DBUG_EXECUTE_IF("rpl_before_forced_rotate", {
        rpl_slave_debug_point(DBUG_RPL_S_BEFORE_FORCED_ROTATE);
      });
      rotate_relay_log(mi, true, true, false);
      rli->sql_force_rotate_relay = false;
    }

    rli->ignore_log_space_limit = false;
  }

  mysql_mutex_unlock(&rli->log_space_lock);
  thd->EXIT_COND(&old_stage);
  return slave_killed;
}

/*
  Builds a Rotate and writes it to relay log.

  The caller must hold mi->data_lock.

  @param thd pointer to I/O Thread's Thd.
  @param mi  point to I/O Thread metadata class.

  @param force_flush_mi_info when true, do not respect sync period and flush
                             information.
                             when false, flush will only happen if it is time to
                             flush.

  @return 0 if everything went fine, 1 otherwise.
*/
static int write_rotate_to_master_pos_into_relay_log(THD *thd, Master_info *mi,
                                                     bool force_flush_mi_info) {
  Relay_log_info *rli = mi->rli;
  int error = 0;
  DBUG_TRACE;

  DBUG_ASSERT(thd == mi->info_thd);
  mysql_mutex_assert_owner(rli->relay_log.get_log_lock());

  DBUG_PRINT("info", ("writing a Rotate event to the relay log"));
  Rotate_log_event *ev = new Rotate_log_event(mi->get_master_log_name(), 0,
                                              mi->get_master_log_pos(),
                                              Rotate_log_event::DUP_NAME);

  DBUG_EXECUTE_IF("fail_generating_rotate_event_on_write_rotate_to_master_pos",
                  {
                    if (likely((bool)ev)) {
                      delete ev;
                      ev = nullptr;
                    }
                  });

  if (likely((bool)ev)) {
    if (mi->get_mi_description_event() != nullptr)
      ev->common_footer->checksum_alg =
          mi->get_mi_description_event()->common_footer->checksum_alg;

    ev->server_id = 0;  // don't be ignored by slave SQL thread
    if (unlikely(rli->relay_log.write_event(ev, mi) != 0))
      mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
                 ER_THD(thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
                 "failed to write a Rotate event"
                 " to the relay log, SHOW SLAVE STATUS may be"
                 " inaccurate");
    mysql_mutex_lock(&mi->data_lock);
    if (flush_master_info(mi, force_flush_mi_info, false, false)) {
      error = 1;
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_FLUSH_MASTER_INFO_FILE);
    }
    mysql_mutex_unlock(&mi->data_lock);
    delete ev;
  } else {
    error = 1;
    mi->report(ERROR_LEVEL, ER_SLAVE_CREATE_EVENT_FAILURE,
               ER_THD(thd, ER_SLAVE_CREATE_EVENT_FAILURE),
               "Rotate_event (out of memory?),"
               " SHOW SLAVE STATUS may be inaccurate");
  }

  return error;
}

/*
  Builds a Rotate from the ignored events' info and writes it to relay log.

  @param thd pointer to I/O Thread's Thd.
  @param mi  point to I/O Thread metadata class.

  @return 0 if everything went fine, 1 otherwise.
*/
static int write_ignored_events_info_to_relay_log(THD *thd, Master_info *mi) {
  Relay_log_info *rli = mi->rli;
  mysql_mutex_t *end_pos_lock = rli->relay_log.get_binlog_end_pos_lock();
  int error = 0;
  DBUG_TRACE;

  DBUG_ASSERT(thd == mi->info_thd);
  mysql_mutex_lock(rli->relay_log.get_log_lock());
  mysql_mutex_lock(end_pos_lock);

  if (rli->ign_master_log_name_end[0]) {
    DBUG_PRINT("info", ("writing a Rotate event to track down ignored events"));
    /*
      If the ignored events' info still hold, they should have same info as
      the mi->get_master_log_[name|pos].
    */
    DBUG_ASSERT(
        strcmp(rli->ign_master_log_name_end, mi->get_master_log_name()) == 0);
    DBUG_ASSERT(rli->ign_master_log_pos_end == mi->get_master_log_pos());

    /* Avoid the applier to get the ignored event' info by rli->ign* */
    rli->ign_master_log_name_end[0] = 0;
    /* can unlock before writing as the relay log will soon have our Rotate */
    mysql_mutex_unlock(end_pos_lock);

    /* Generate the rotate based on mi position */
    error = write_rotate_to_master_pos_into_relay_log(
        thd, mi, false /* force_flush_mi_info */);
  } else
    mysql_mutex_unlock(end_pos_lock);

  mysql_mutex_unlock(rli->relay_log.get_log_lock());
  return error;
}

/**
  Returns slave lag duration relative to master.
  @param mi Pointer to Master_info object for the IO thread.

  @retval pair(sec_behind_master, milli_second_behind_master)
  This function sets above values to -1 to represent nulls
*/
std::pair<longlong, longlong> get_time_lag_behind_master(Master_info *mi) {
  longlong sec_behind_master = -1;
  longlong milli_sec_behind_master = -1;
  /*
     The pseudo code to compute Seconds_Behind_Master:
     if (SQL thread is running)
     {
       if (SQL thread processed all the available relay log)
       {
         if (IO thread is running)
            print 0;
         else
            print NULL;
       }
        else
          compute Seconds_Behind_Master;
      }
      else
       print NULL;
  */

  bool sbm_is_null = false;
  bool sbm_is_zero = false;
  if (mi->rli->slave_running) {
    time_t now = time(0);
    /*
       Check if SQL thread is at the end of relay log
       Checking should be done using two conditions
       condition1: compare the log positions and
       condition2: compare the file names (to handle rotation case)
    */
    if (reset_seconds_behind_master &&
        (mi->get_master_log_pos() == mi->rli->get_group_master_log_pos()) &&
        (!strcmp(mi->get_master_log_name(),
                 mi->rli->get_group_master_log_name()))) {
      if (mi->slave_running == MYSQL_SLAVE_RUN_CONNECT)
        sec_behind_master = 0LL;
      else
        sec_behind_master = -1;
      sbm_is_zero = mi->slave_running == MYSQL_SLAVE_RUN_CONNECT;
      sbm_is_null = !sbm_is_zero;
    } else {
      long time_diff = ((long)(now - mi->rli->last_master_timestamp) -
                        mi->clock_diff_with_master);
      /*
        Apparently on some systems time_diff can be <0. Here are possible
        reasons related to MySQL:
        - the master is itself a slave of another master whose time is ahead.
        - somebody used an explicit SET TIMESTAMP on the master.
        Possible reason related to granularity-to-second of time functions
        (nothing to do with MySQL), which can explain a value of -1:
        assume the master's and slave's time are perfectly synchronized, and
        that at slave's connection time, when the master's timestamp is read,
        it is at the very end of second 1, and (a very short time later) when
        the slave's timestamp is read it is at the very beginning of second
        2. Then the recorded value for master is 1 and the recorded value for
        slave is 2. At SHOW SLAVE STATUS time, assume that the difference
        between timestamp of slave and rli->last_master_timestamp is 0
        (i.e. they are in the same second), then we get 0-(2-1)=-1 as a result.
        This confuses users, so we don't go below 0: hence the max().

        last_master_timestamp == 0 (an "impossible" timestamp 1970) is a
        special marker to say "consider we have caught up".
      */
      if (mi->rli->last_master_timestamp == 0) {
        /*
          If the I/O thread is encountering problems during initailization,
          then display NULL instead of 0.
        */
        sbm_is_zero = mi->slave_running == MYSQL_SLAVE_RUN_CONNECT;
        sbm_is_null = !sbm_is_zero;
      }
      if (sbm_is_null) {
        sec_behind_master = -1;
      } else {
        sec_behind_master =
            (longlong)(mi->rli->last_master_timestamp ? max(0L, time_diff) : 0);
      }
    }
  } else {
    sec_behind_master = -1;
    sbm_is_null = true;
  }

  // Milli_Seconds_Behind_Master
  if (opt_binlog_trx_meta_data) {
    if (sbm_is_null)
      milli_sec_behind_master = -1;
    else if (sbm_is_zero)
      milli_sec_behind_master = 0LL;
    else {
      ulonglong now_millis =
          duration_cast<milliseconds>(system_clock::now().time_since_epoch())
              .count();
      // adjust for clock mismatch
      now_millis -= mi->clock_diff_with_master * 1000;
      milli_sec_behind_master =
          now_millis - mi->rli->last_master_timestamp_millis;
    }
  }
  return std::make_pair(sec_behind_master, milli_sec_behind_master);
}

/**
  Send milli_second_behind_master statistic to primary using
  COM_SEND_REPLICA_STATISTICS
*/
int send_replica_statistics_to_master(MYSQL *mysql, Master_info *mi) {
  uchar buf[1024];
  uchar *pos = buf;
  DBUG_ENTER("send_replica_statistics_to_master");

  int timestamp = my_time(0);
  std::pair<longlong, longlong> time_lag_behind_master =
      get_time_lag_behind_master(mi);
  longlong milli_sec_behind_master =
      max((longlong)time_lag_behind_master.second, (longlong)0);

  int4store(pos, server_id);
  pos += 4;
  int4store(pos, timestamp);
  pos += 4;
  int4store(pos, milli_sec_behind_master);
  pos += 4;

  if (simple_command(mysql, COM_SEND_REPLICA_STATISTICS, buf,
                     (size_t)(pos - buf), 0)) {
    DBUG_RETURN(1);
  }
  DBUG_RETURN(0);
}

static int register_slave_on_master(MYSQL *mysql, Master_info *mi,
                                    bool *suppress_warnings) {
  uchar buf[1024], *pos = buf;
  size_t report_host_len = 0, report_user_len = 0, report_password_len = 0;
  DBUG_TRACE;

  *suppress_warnings = false;
  if (report_host) report_host_len = strlen(report_host);
  if (report_host_len > HOSTNAME_LENGTH) {
    LogErr(WARNING_LEVEL, ER_RPL_SLAVE_REPORT_HOST_TOO_LONG, report_host_len,
           HOSTNAME_LENGTH, mi->get_for_channel_str());
    return 0;
  }

  if (report_user) report_user_len = strlen(report_user);
  if (report_user_len > USERNAME_LENGTH) {
    LogErr(WARNING_LEVEL, ER_RPL_SLAVE_REPORT_USER_TOO_LONG, report_user_len,
           USERNAME_LENGTH, mi->get_for_channel_str());
    return 0;
  }

  if (report_password) report_password_len = strlen(report_password);
  if (report_password_len > MAX_PASSWORD_LENGTH) {
    LogErr(WARNING_LEVEL, ER_RPL_SLAVE_REPORT_PASSWORD_TOO_LONG,
           report_password_len, MAX_PASSWORD_LENGTH, mi->get_for_channel_str());
    return 0;
  }

  int4store(pos, server_id);
  pos += 4;
  pos = net_store_data(pos, (uchar *)report_host, report_host_len);
  pos = net_store_data(pos, (uchar *)report_user, report_user_len);
  pos = net_store_data(pos, (uchar *)report_password, report_password_len);
  int2store(pos, (uint16)report_port);
  pos += 2;
  /*
    Fake rpl_recovery_rank, which was removed in BUG#13963,
    so that this server can register itself on old servers,
    see BUG#49259.
   */
  int4store(pos, /* rpl_recovery_rank */ 0);
  pos += 4;
  /* The master will fill in master_id */
  int4store(pos, 0);
  pos += 4;

  if (simple_command(mysql, COM_REGISTER_SLAVE, buf, (size_t)(pos - buf), 0)) {
    if (mysql_errno(mysql) == ER_NET_READ_INTERRUPTED) {
      *suppress_warnings = true;  // Suppress reconnect warning
    } else if (!check_io_slave_killed(mi->info_thd, mi, nullptr)) {
      char err_buf[256];
      snprintf(err_buf, sizeof(err_buf), "%s (Errno: %d)", mysql_error(mysql),
               mysql_errno(mysql));
      mi->report(ERROR_LEVEL, ER_SLAVE_MASTER_COM_FAILURE,
                 ER_THD(current_thd, ER_SLAVE_MASTER_COM_FAILURE),
                 "COM_REGISTER_SLAVE", err_buf);
    }
    return 1;
  }

  DBUG_EXECUTE_IF("simulate_register_slave_killed", {
    mi->abort_slave = 1;
    return 1;
  };);
  return 0;
}

/**
    Function that fills the metadata required for SHOW SLAVE STATUS.
    This function shall be used in two cases:
     1) SHOW SLAVE STATUS FOR ALL CHANNELS
     2) SHOW SLAVE STATUS for a channel

     @param[in,out]  field_list        field_list to fill the metadata
     @param[in]      io_gtid_set_size  the size to be allocated to store
                                       the retrieved gtid set
     @param[in]      sql_gtid_set_size the size to be allocated to store
                                       the executed gtid set

     @todo  return a bool after adding catching the exceptions to the
            push_back() methods for field_list.
*/

static void show_slave_status_metadata(List<Item> &field_list,
                                       int io_gtid_set_size,
                                       int sql_gtid_set_size) {
  field_list.push_back(new Item_empty_string("Slave_IO_State", 14));
  field_list.push_back(
      new Item_empty_string("Master_Host", HOSTNAME_LENGTH + 1));
  field_list.push_back(
      new Item_empty_string("Master_User", USERNAME_LENGTH + 1));
  field_list.push_back(new Item_return_int("Master_Port", 7, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("Connect_Retry", 10, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Master_Log_File", FN_REFLEN));
  field_list.push_back(
      new Item_return_int("Read_Master_Log_Pos", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_empty_string("Relay_Log_File", FN_REFLEN));
  field_list.push_back(
      new Item_return_int("Relay_Log_Pos", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_empty_string("Relay_Master_Log_File", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Slave_IO_Running", 3));
  field_list.push_back(new Item_empty_string("Slave_SQL_Running", 3));
  field_list.push_back(new Item_empty_string("Replicate_Do_DB", 20));
  field_list.push_back(new Item_empty_string("Replicate_Ignore_DB", 20));
  field_list.push_back(new Item_empty_string("Replicate_Do_Table", 20));
  field_list.push_back(new Item_empty_string("Replicate_Ignore_Table", 23));
  field_list.push_back(new Item_empty_string("Replicate_Wild_Do_Table", 24));
  field_list.push_back(
      new Item_empty_string("Replicate_Wild_Ignore_Table", 28));
  field_list.push_back(new Item_return_int("Last_Errno", 4, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Last_Symbolic_Errno", 20));
  field_list.push_back(new Item_empty_string("Last_Error", 20));
  field_list.push_back(
      new Item_return_int("Skip_Counter", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("Exec_Master_Log_Pos", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_return_int("Relay_Log_Space", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_empty_string("Until_Condition", 6));
  field_list.push_back(new Item_empty_string("Until_Log_File", FN_REFLEN));
  field_list.push_back(
      new Item_return_int("Until_Log_Pos", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_empty_string("Master_SSL_Allowed", 7));
  field_list.push_back(new Item_empty_string("Master_SSL_CA_File", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Master_SSL_CA_Path", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Master_SSL_Cert", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Master_SSL_Cipher", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Master_SSL_Key", FN_REFLEN));
  field_list.push_back(
      new Item_return_int("Seconds_Behind_Master", 10, MYSQL_TYPE_LONGLONG));
  if (opt_binlog_trx_meta_data)
    field_list.push_back(new Item_return_int("Milli_Seconds_Behind_Master", 10,
                                             MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_empty_string("Master_SSL_Verify_Server_Cert", 3));
  field_list.push_back(
      new Item_return_int("Last_IO_Errno", 4, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Last_IO_Error", 20));
  field_list.push_back(
      new Item_return_int("Last_SQL_Errno", 4, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Last_SQL_Error", 20));
  field_list.push_back(
      new Item_empty_string("Replicate_Ignore_Server_Ids", FN_REFLEN));
  field_list.push_back(
      new Item_return_int("Master_Server_Id", sizeof(ulong), MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Master_UUID", UUID_LENGTH));
  field_list.push_back(
      new Item_empty_string("Master_Info_File", 2 * FN_REFLEN));
  field_list.push_back(new Item_return_int("SQL_Delay", 10, MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_return_int("SQL_Remaining_Delay", 8, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Slave_SQL_Running_State", 20));
  field_list.push_back(
      new Item_return_int("Master_Retry_Count", 10, MYSQL_TYPE_LONGLONG));
  field_list.push_back(
      new Item_empty_string("Master_Bind", HOSTNAME_LENGTH + 1));
  field_list.push_back(new Item_empty_string("Last_IO_Error_Timestamp", 20));
  field_list.push_back(new Item_empty_string("Last_SQL_Error_Timestamp", 20));
  field_list.push_back(new Item_empty_string("Master_SSL_Crl", FN_REFLEN));
  field_list.push_back(new Item_empty_string("Master_SSL_Crlpath", FN_REFLEN));
  field_list.push_back(
      new Item_empty_string("Retrieved_Gtid_Set", io_gtid_set_size));
  field_list.push_back(
      new Item_empty_string("Executed_Gtid_Set", sql_gtid_set_size));
  field_list.push_back(
      new Item_return_int("Auto_Position", sizeof(ulong), MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Replicate_Rewrite_DB", 24));
  field_list.push_back(
      new Item_empty_string("Channel_Name", CHANNEL_NAME_LENGTH));
  field_list.push_back(new Item_empty_string("Master_TLS_Version", FN_REFLEN));
  field_list.push_back(
      new Item_empty_string("Master_public_key_path", FN_REFLEN));
  field_list.push_back(new Item_return_int("Get_master_public_key",
                                           sizeof(ulong), MYSQL_TYPE_LONG));
  field_list.push_back(
      new Item_empty_string("Network_Namespace", NAME_LEN + 1));
  field_list.push_back(
      new Item_empty_string("Slave_Lag_Stats_Thread_Running", 3));
}

/**
    Send the data to the client of a Master_info during show_slave_status()
    This function has to be called after calling show_slave_status_metadata().
    Just before sending the data, thd->get_protocol() is prepared to (re)send;

    @param[in]     thd         client thread
    @param[in]     mi          the master info. In the case of multisource
                               replication, this master info corresponds to a
                                channel.

    @param[in]     io_gtid_set_buffer    buffer related to Retrieved GTID set
                                          for each channel.
    @param[in]     sql_gtid_set_buffer   buffer related to Executed GTID set
                                           for each channel.
    @retval        0     success
    @retval        1     Error
*/

static bool show_slave_status_send_data(THD *thd, Master_info *mi,
                                        char *io_gtid_set_buffer,
                                        char *sql_gtid_set_buffer) {
  DBUG_TRACE;

  Protocol *protocol = thd->get_protocol();
  char *slave_sql_running_state = nullptr;
  Rpl_filter *rpl_filter = mi->rli->rpl_filter;

  DBUG_PRINT("info", ("host is set: '%s'", mi->host));

  protocol->start_row();

  /*
    slave_running can be accessed without run_lock but not other
    non-volatile members like mi->info_thd or rli->info_thd, for
    them either info_thd_lock or run_lock hold is required.
  */
  mysql_mutex_lock(&mi->info_thd_lock);
  protocol->store(mi->info_thd ? mi->info_thd->get_proc_info() : "",
                  &my_charset_bin);
  mysql_mutex_unlock(&mi->info_thd_lock);

  mysql_mutex_lock(&mi->rli->info_thd_lock);
  slave_sql_running_state = const_cast<char *>(
      mi->rli->info_thd ? mi->rli->info_thd->get_proc_info() : "");
  mysql_mutex_unlock(&mi->rli->info_thd_lock);

  mysql_mutex_lock(&mi->data_lock);
  mysql_mutex_lock(&mi->rli->data_lock);
  mysql_mutex_lock(&mi->err_lock);
  mysql_mutex_lock(&mi->rli->err_lock);

  DEBUG_SYNC(thd, "wait_after_lock_active_mi_and_rli_data_lock_is_acquired");
  protocol->store(mi->host, &my_charset_bin);
  protocol->store(mi->get_user(), &my_charset_bin);
  protocol->store((uint32)mi->port);
  protocol->store((uint32)mi->connect_retry);
  protocol->store(mi->get_master_log_name(), &my_charset_bin);
  protocol->store((ulonglong)mi->get_master_log_pos());
  protocol->store(mi->rli->get_group_relay_log_name() +
                      dirname_length(mi->rli->get_group_relay_log_name()),
                  &my_charset_bin);
  protocol->store((ulonglong)mi->rli->get_group_relay_log_pos());
  protocol->store(mi->rli->get_group_master_log_name(), &my_charset_bin);
  protocol->store(
      mi->slave_running == MYSQL_SLAVE_RUN_CONNECT
          ? "Yes"
          : (mi->slave_running == MYSQL_SLAVE_RUN_NOT_CONNECT ? "Connecting"
                                                              : "No"),
      &my_charset_bin);
  protocol->store(mi->rli->slave_running ? "Yes" : "No", &my_charset_bin);

  /*
    Acquire the read lock, because the filter may be modified by
    CHANGE REPLICATION FILTER when slave is not running.
  */
  rpl_filter->rdlock();
  store(protocol, rpl_filter->get_do_db());
  store(protocol, rpl_filter->get_ignore_db());

  char buf[256];
  String tmp(buf, sizeof(buf), &my_charset_bin);
  rpl_filter->get_do_table(&tmp);
  protocol->store(&tmp);
  rpl_filter->get_ignore_table(&tmp);
  protocol->store(&tmp);
  rpl_filter->get_wild_do_table(&tmp);
  protocol->store(&tmp);
  rpl_filter->get_wild_ignore_table(&tmp);
  protocol->store(&tmp);

  protocol->store(mi->rli->last_error().number);

  if (mi->rli->last_error().number == 0) {
    protocol->store("", &my_charset_bin);
  } else if (mi->rli->last_error().number >= EE_ERROR_FIRST &&
             mi->rli->last_error().number <= EE_ERROR_LAST) {
    protocol->store(get_global_errname(mi->rli->last_error().number),
                    &my_charset_bin);
  } else {
    protocol->store("regular sql errno", &my_charset_bin);
  }

  protocol->store(mi->rli->last_error().message, &my_charset_bin);
  protocol->store((uint32)mi->rli->slave_skip_counter);
  protocol->store((ulonglong)mi->rli->get_group_master_log_pos());
  protocol->store((ulonglong)mi->rli->log_space_total);

  const char *until_type = "";

  switch (mi->rli->until_condition) {
    case Relay_log_info::UNTIL_NONE:
      until_type = "None";
      break;
    case Relay_log_info::UNTIL_MASTER_POS:
      until_type = "Master";
      break;
    case Relay_log_info::UNTIL_RELAY_POS:
      until_type = "Relay";
      break;
    case Relay_log_info::UNTIL_SQL_BEFORE_GTIDS:
      until_type = "SQL_BEFORE_GTIDS";
      break;
    case Relay_log_info::UNTIL_SQL_AFTER_GTIDS:
      until_type = "SQL_AFTER_GTIDS";
      break;
    case Relay_log_info::UNTIL_SQL_VIEW_ID:
      until_type = "SQL_VIEW_ID";
      break;
    case Relay_log_info::UNTIL_SQL_AFTER_MTS_GAPS:
      until_type = "SQL_AFTER_MTS_GAPS";
      break;
    case Relay_log_info::UNTIL_DONE:
      until_type = "DONE";
      break;
    default:
      DBUG_ASSERT(0);
  }
  protocol->store(until_type, &my_charset_bin);
  protocol->store(mi->rli->get_until_log_name(), &my_charset_bin);
  protocol->store((ulonglong)mi->rli->get_until_log_pos());

  protocol->store(mi->ssl ? "Yes" : "No", &my_charset_bin);
  protocol->store(mi->ssl_ca, &my_charset_bin);
  protocol->store(mi->ssl_capath, &my_charset_bin);
  protocol->store(mi->ssl_cert, &my_charset_bin);
  protocol->store(mi->ssl_cipher, &my_charset_bin);
  protocol->store(mi->ssl_key, &my_charset_bin);

  std::pair<longlong, longlong> time_lag_behind_master =
      get_time_lag_behind_master(mi);

  // Seconds_Behind_Master
  if (time_lag_behind_master.first == -1) {
    protocol->store_null();
  } else {
    protocol->store(time_lag_behind_master.first);
  }

  // Milli_Seconds_Behind_Master
  if (opt_binlog_trx_meta_data) {
    if (time_lag_behind_master.second == -1) {
      protocol->store_null();
    } else {
      protocol->store(time_lag_behind_master.second);
    }
  }

  protocol->store(mi->ssl_verify_server_cert ? "Yes" : "No", &my_charset_bin);

  // Last_IO_Errno
  protocol->store(mi->last_error().number);
  // Last_IO_Error
  protocol->store(mi->last_error().message, &my_charset_bin);
  // Last_SQL_Errno
  protocol->store(mi->rli->last_error().number);
  // Last_SQL_Error
  protocol->store(mi->rli->last_error().message, &my_charset_bin);
  // Replicate_Ignore_Server_Ids
  {
    char buff[FN_REFLEN];
    ulong i, cur_len;
    for (i = 0, buff[0] = 0, cur_len = 0;
         i < mi->ignore_server_ids->dynamic_ids.size(); i++) {
      ulong s_id, slen;
      char sbuff[FN_REFLEN];
      s_id = mi->ignore_server_ids->dynamic_ids[i];
      slen = sprintf(sbuff, (i == 0 ? "%lu" : ", %lu"), s_id);
      if (cur_len + slen + 4 > FN_REFLEN) {
        /*
          break the loop whenever remained space could not fit
          ellipses on the next cycle
        */
        sprintf(buff + cur_len, "...");
        break;
      }
      cur_len += sprintf(buff + cur_len, "%s", sbuff);
    }
    protocol->store(buff, &my_charset_bin);
  }
  // Master_Server_id
  protocol->store((uint32)mi->master_id);
  protocol->store(mi->master_uuid, &my_charset_bin);
  // Master_Info_File
  protocol->store(mi->get_description_info(), &my_charset_bin);
  // SQL_Delay
  protocol->store((uint32)mi->rli->get_sql_delay());
  // SQL_Remaining_Delay
  if (slave_sql_running_state == stage_sql_thd_waiting_until_delay.m_name) {
    time_t t = my_time(0), sql_delay_end = mi->rli->get_sql_delay_end();
    protocol->store((uint32)(t < sql_delay_end ? sql_delay_end - t : 0));
  } else
    protocol->store_null();
  // Slave_SQL_Running_State
  protocol->store(slave_sql_running_state, &my_charset_bin);
  // Master_Retry_Count
  protocol->store((ulonglong)mi->retry_count);
  // Master_Bind
  protocol->store(mi->bind_addr, &my_charset_bin);
  // Last_IO_Error_Timestamp
  protocol->store(mi->last_error().timestamp, &my_charset_bin);
  // Last_SQL_Error_Timestamp
  protocol->store(mi->rli->last_error().timestamp, &my_charset_bin);
  // Master_Ssl_Crl
  protocol->store(mi->ssl_crl, &my_charset_bin);
  // Master_Ssl_Crlpath
  protocol->store(mi->ssl_crlpath, &my_charset_bin);
  // Retrieved_Gtid_Set
  protocol->store(io_gtid_set_buffer, &my_charset_bin);
  // Executed_Gtid_Set
  protocol->store(sql_gtid_set_buffer, &my_charset_bin);
  // Auto_Position
  protocol->store(mi->is_auto_position() ? 1 : 0);
  // Replicate_Rewrite_DB
  rpl_filter->get_rewrite_db(&tmp);
  protocol->store(&tmp);
  // channel_name
  protocol->store(mi->get_channel(), &my_charset_bin);
  // Master_TLS_Version
  protocol->store(mi->tls_version, &my_charset_bin);
  // Master_public_key_path
  protocol->store(mi->public_key_path, &my_charset_bin);
  // Get_master_public_key
  protocol->store(mi->get_public_key ? 1 : 0);

  protocol->store(mi->network_namespace_str(), &my_charset_bin);
  // slave lag stats daemon running status
  protocol->store(slave_stats_daemon_thread_counter > 0 ? "Yes" : "No",
                  &my_charset_bin);

  rpl_filter->unlock();
  mysql_mutex_unlock(&mi->rli->err_lock);
  mysql_mutex_unlock(&mi->err_lock);
  mysql_mutex_unlock(&mi->rli->data_lock);
  mysql_mutex_unlock(&mi->data_lock);

  return false;
}

/**
   Method to the show the replication status in all channels.

   @param[in]       thd        the client thread

   @retval          0           success
   @retval          1           Error

*/
bool show_slave_status(THD *thd) {
  List<Item> field_list;
  Protocol *protocol = thd->get_protocol();
  int sql_gtid_set_size = 0, io_gtid_set_size = 0;
  Master_info *mi = nullptr;
  char *sql_gtid_set_buffer = nullptr;
  char **io_gtid_set_buffer_array;
  /*
    We need the maximum size of the retrieved gtid set (i.e io_gtid_set_size).
    This size is needed to reserve the place in show_slave_status_metadata().
    So, we travel all the mi's and find out the maximum size of io_gtid_set_size
    and pass it through show_slave_status_metadata()
  */
  int max_io_gtid_set_size = io_gtid_set_size;
  uint idx;
  uint num_io_gtid_sets;
  bool ret = true;

  DBUG_TRACE;

  channel_map.assert_some_lock();

  num_io_gtid_sets = channel_map.get_num_instances();

  io_gtid_set_buffer_array =
      (char **)my_malloc(key_memory_show_slave_status_io_gtid_set,
                         num_io_gtid_sets * sizeof(char *), MYF(MY_WME));

  if (io_gtid_set_buffer_array == nullptr) return true;

  global_sid_lock->wrlock();

  const Gtid_set *sql_gtid_set = gtid_state->get_executed_gtids();
  sql_gtid_set_size = sql_gtid_set->to_string(&sql_gtid_set_buffer);

  global_sid_lock->unlock();

  idx = 0;
  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;
    /*
      The following statement is needed because, when mi->host[0]=0
      we don't alloc memory for retried_gtid_set. However, we try
      to free it at the end, causing a crash. To be on safeside,
      we initialize it to NULL, so that my_free() takes care of it.
    */
    io_gtid_set_buffer_array[idx] = nullptr;

    if (Master_info::is_configured(mi)) {
      const Gtid_set *io_gtid_set = mi->rli->get_gtid_set();
      mi->rli->get_sid_lock()->wrlock();

      /*
         @todo: a single memory allocation improves speed,
         instead of doing it for each loop
      */

      if ((io_gtid_set_size =
               io_gtid_set->to_string(&io_gtid_set_buffer_array[idx])) < 0) {
        my_eof(thd);
        my_free(sql_gtid_set_buffer);

        for (uint i = 0; i < idx - 1; i++) {
          my_free(io_gtid_set_buffer_array[i]);
        }
        my_free(io_gtid_set_buffer_array);

        mi->rli->get_sid_lock()->unlock();
        return true;
      } else
        max_io_gtid_set_size = max_io_gtid_set_size > io_gtid_set_size
                                   ? max_io_gtid_set_size
                                   : io_gtid_set_size;

      mi->rli->get_sid_lock()->unlock();
    }
    idx++;
  }

  show_slave_status_metadata(field_list, max_io_gtid_set_size,
                             sql_gtid_set_size);

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    goto err;
  }

  /* Run through each mi */

  idx = 0;
  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;

    if (Master_info::is_configured(mi)) {
      if (show_slave_status_send_data(thd, mi, io_gtid_set_buffer_array[idx],
                                      sql_gtid_set_buffer))
        goto err;

      if (protocol->end_row()) goto err;
    }
    idx++;
  }

  ret = false;
err:
  my_eof(thd);
  for (uint i = 0; i < num_io_gtid_sets; i++) {
    my_free(io_gtid_set_buffer_array[i]);
  }
  my_free(io_gtid_set_buffer_array);
  my_free(sql_gtid_set_buffer);

  return ret;
}

/**
  Execute a SHOW SLAVE STATUS statement.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @param mi Pointer to Master_info object for the IO thread.

  @retval false success
  @retval true failure

  Currently, show slave status works for a channel too, in multisource
  replication. But using performance schema tables is better.

*/
bool show_slave_status(THD *thd, Master_info *mi) {
  List<Item> field_list;
  Protocol *protocol = thd->get_protocol();
  char *sql_gtid_set_buffer = nullptr, *io_gtid_set_buffer = nullptr;
  int sql_gtid_set_size = 0, io_gtid_set_size = 0;
  DBUG_TRACE;

  if (mi != nullptr) {
    global_sid_lock->wrlock();
    const Gtid_set *sql_gtid_set = gtid_state->get_executed_gtids();
    sql_gtid_set_size = sql_gtid_set->to_string(&sql_gtid_set_buffer);
    global_sid_lock->unlock();

    mi->rli->get_sid_lock()->wrlock();
    const Gtid_set *io_gtid_set = mi->rli->get_gtid_set();
    io_gtid_set_size = io_gtid_set->to_string(&io_gtid_set_buffer);
    mi->rli->get_sid_lock()->unlock();

    if (sql_gtid_set_size < 0 || io_gtid_set_size < 0) {
      my_eof(thd);
      my_free(sql_gtid_set_buffer);
      my_free(io_gtid_set_buffer);
      return true;
    }
  }

  /* Fill the metadata required for show slave status. */

  show_slave_status_metadata(field_list, io_gtid_set_size, sql_gtid_set_size);

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    my_free(sql_gtid_set_buffer);
    my_free(io_gtid_set_buffer);
    return true;
  }

  if (Master_info::is_configured(mi)) {
    if (show_slave_status_send_data(thd, mi, io_gtid_set_buffer,
                                    sql_gtid_set_buffer))
      return true;

    if (protocol->end_row()) {
      my_free(sql_gtid_set_buffer);
      my_free(io_gtid_set_buffer);
      return true;
    }
  }
  my_eof(thd);
  my_free(sql_gtid_set_buffer);
  my_free(io_gtid_set_buffer);
  return false;
}

/**
  Entry point for SHOW SLAVE STATUS command. Function displayes
  the slave status for all channels or for a single channel
  based on the FOR CHANNEL  clause.

  @param[in]       thd          the client thread.

  @retval          false        ok
  @retval          true         not ok
*/
bool show_slave_status_cmd(THD *thd) {
  Master_info *mi = nullptr;
  LEX *lex = thd->lex;
  bool res;

  DBUG_TRACE;

  channel_map.rdlock();

  if (!lex->mi.for_channel)
    res = show_slave_status(thd);
  else {
    mi = channel_map.get_mi(lex->mi.channel);

    /*
      When mi is NULL, that means the channel doesn't exist, SSS
      will throw an error.
    */
    if (mi == nullptr) {
      my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), lex->mi.channel);
      channel_map.unlock();
      return true;
    }

    /*
      If the channel being used is a group replication applier channel we
      need to disable the SHOW SLAVE STATUS commannd as its output is not
      compatible with this command.
    */
    if (channel_map.is_group_replication_channel_name(mi->get_channel(),
                                                      true)) {
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "SHOW SLAVE STATUS", mi->get_channel());
      channel_map.unlock();
      return true;
    }

    res = show_slave_status(thd, mi);
  }

  channel_map.unlock();

  return res;
}

void set_slave_thread_options(THD *thd) {
  DBUG_TRACE;
  /*
     It's nonsense to constrain the slave threads with max_join_size; if a
     query succeeded on master, we HAVE to execute it. So set
     OPTION_BIG_SELECTS. Setting max_join_size to HA_POS_ERROR is not enough
     (and it's not needed if we have OPTION_BIG_SELECTS) because an INSERT
     SELECT examining more than 4 billion rows would still fail (yes, because
     when max_join_size is 4G, OPTION_BIG_SELECTS is automatically set, but
     only for client threads.
  */
  ulonglong options = thd->variables.option_bits | OPTION_BIG_SELECTS;
  if (opt_log_slave_updates)
    options |= OPTION_BIN_LOG;
  else
    options &= ~OPTION_BIN_LOG;
  thd->variables.option_bits = options;
  thd->variables.completion_type = 0;

  /*
    Set autocommit= 1 when info tables are used and autocommit == 0 to
    avoid trigger asserts on mysql_execute_command(THD *thd) caused by
    info tables updates which do not commit, like Rotate, Stop and
    skipped events handling.
  */
  if ((thd->variables.option_bits & OPTION_NOT_AUTOCOMMIT) &&
      (opt_mi_repository_id == INFO_REPOSITORY_TABLE ||
       opt_rli_repository_id == INFO_REPOSITORY_TABLE)) {
    thd->variables.option_bits |= OPTION_AUTOCOMMIT;
    thd->variables.option_bits &= ~OPTION_NOT_AUTOCOMMIT;
    thd->server_status |= SERVER_STATUS_AUTOCOMMIT;
  }

  /*
    Set thread InnoDB high priority.
  */
  DBUG_EXECUTE_IF("dbug_set_high_prio_sql_thread", {
    if (thd->system_thread == SYSTEM_THREAD_SLAVE_SQL ||
        thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER)
      thd->thd_tx_priority = 1;
  });
}

void set_slave_thread_default_charset(THD *thd, Relay_log_info const *rli) {
  DBUG_TRACE;

  thd->variables.character_set_client =
      global_system_variables.character_set_client;
  thd->variables.collation_connection =
      global_system_variables.collation_connection;
  thd->variables.collation_server = global_system_variables.collation_server;
  thd->update_charset();

  /*
    We use a const cast here since the conceptual (and externally
    visible) behavior of the function is to set the default charset of
    the thread.  That the cache has to be invalidated is a secondary
    effect.
   */
  const_cast<Relay_log_info *>(rli)->cached_charset_invalidate();
}

/*
  init_slave_thread()
*/

static int init_slave_thread(THD *thd, SLAVE_THD_TYPE thd_type) {
  DBUG_TRACE;
#if !defined(DBUG_OFF)
  int simulate_error = 0;
#endif
  thd->system_thread = (thd_type == SLAVE_THD_WORKER)
                           ? SYSTEM_THREAD_SLAVE_WORKER
                           : (thd_type == SLAVE_THD_SQL)
                                 ? SYSTEM_THREAD_SLAVE_SQL
                                 : SYSTEM_THREAD_SLAVE_IO;
  thd->get_protocol_classic()->init_net(nullptr);
  thd->slave_thread = true;
  thd->enable_slow_log = opt_log_slow_slave_statements;
  set_slave_thread_options(thd);

  /*
    Replication threads are:
    - background threads in the server, not user sessions,
    - yet still assigned a PROCESSLIST_ID,
      for historical reasons (displayed in SHOW PROCESSLIST).
  */
  thd->set_new_thread_id();

#ifdef HAVE_PSI_THREAD_INTERFACE
  /*
    Populate the PROCESSLIST_ID in the instrumentation.
  */
  struct PSI_thread *psi = PSI_THREAD_CALL(get_thread)();
  PSI_THREAD_CALL(set_thread_id)(psi, thd->thread_id());
#endif /* HAVE_PSI_THREAD_INTERFACE */

  DBUG_EXECUTE_IF("simulate_io_slave_error_on_init",
                  simulate_error |= (1 << SLAVE_THD_IO););
  DBUG_EXECUTE_IF("simulate_sql_slave_error_on_init",
                  simulate_error |= (1 << SLAVE_THD_SQL););
  thd->store_globals();
#if !defined(DBUG_OFF)
  if (simulate_error & (1 << thd_type)) {
    return -1;
  }
#endif

  if (thd_type == SLAVE_THD_SQL) {
    THD_STAGE_INFO(thd, stage_waiting_for_the_next_event_in_relay_log);
    thd->set_command(
        COM_QUERY);  // the SQL thread does not use the server protocol
  } else {
    THD_STAGE_INFO(thd, stage_waiting_for_master_update);
  }
  thd->set_time();
  /* Do not use user-supplied timeout value for system threads. */
  thd->variables.lock_wait_timeout_nsec = LONG_TIMEOUT_NSEC;
  return 0;
}

/**
  Sleep for a given amount of time or until killed.

  @param thd        Thread context of the current thread.
  @param seconds    The number of seconds to sleep.
  @param func       Function object to check if the thread has been killed.
  @param info       The Rpl_info object associated with this sleep.

  @retval True if the thread has been killed, false otherwise.
*/
template <typename killed_func, typename rpl_info>
static inline bool slave_sleep(THD *thd, time_t seconds, killed_func func,
                               rpl_info info) {
  bool ret;
  struct timespec abstime;
  mysql_mutex_t *lock = &info->sleep_lock;
  mysql_cond_t *cond = &info->sleep_cond;

  /* Absolute system time at which the sleep time expires. */
  set_timespec(&abstime, seconds);

  mysql_mutex_lock(lock);
  thd->ENTER_COND(cond, lock, nullptr, nullptr);

  while (!(ret = func(thd, info))) {
    int error = mysql_cond_timedwait(cond, lock, &abstime);
    if (is_timeout(error)) break;
  }

  mysql_mutex_unlock(lock);
  thd->EXIT_COND(nullptr);

  return ret;
}

/**
  Callback function for mysql_binlog_open().

  Sets gtid data in the command packet.

  @param rpl              Replication stream information.
  @param packet_gtid_set  Pointer to command packet where gtid
                          data should be stored.
*/
static void fix_gtid_set(MYSQL_RPL *rpl, uchar *packet_gtid_set) {
  Gtid_set *gtid_set = (Gtid_set *)rpl->gtid_set_arg;

  gtid_set->encode(packet_gtid_set);
}

static int request_dump(THD *thd, MYSQL *mysql, MYSQL_RPL *rpl, Master_info *mi,
                        bool *suppress_warnings) {
  DBUG_TRACE;
  enum_server_command command =
      mi->is_auto_position() ? COM_BINLOG_DUMP_GTID : COM_BINLOG_DUMP;
  /*
    Note: binlog_flags is always 0.  However, in versions up to 5.6
    RC, the master would check the lowest bit and do something
    unexpected if it was set; in early versions of 5.6 it would also
    use the two next bits.  Therefore, for backward compatibility,
    if we ever start to use the flags, we should leave the three
    lowest bits unused.
  */
  uint binlog_flags = 0;

  *suppress_warnings = false;
  if (RUN_HOOK(binlog_relay_io, before_request_transmit,
               (thd, mi, binlog_flags)))
    return 1;

  rpl->server_id = server_id;
  rpl->flags = binlog_flags;

  Sid_map sid_map(nullptr); /* No lock needed */
  /*
    Note: should be declared at the same level as the mysql_binlog_open() call,
    as the latter might call fix_gtid_set() which in turns calls
    gtid_executed->encode().
  */
  Gtid_set gtid_executed(&sid_map);

  if (command == COM_BINLOG_DUMP_GTID) {
    // get set of GTIDs
    mi->rli->get_sid_lock()->wrlock();

    if (gtid_executed.add_gtid_set(mi->rli->get_gtid_set()) !=
        RETURN_STATUS_OK) {
      mi->rli->get_sid_lock()->unlock();
      return 1;
    }
    mi->rli->get_sid_lock()->unlock();

    global_sid_lock->wrlock();
    gtid_state->dbug_print();

    if (gtid_executed.add_gtid_set(gtid_state->get_executed_gtids()) !=
        RETURN_STATUS_OK) {
      global_sid_lock->unlock();
      return 1;
    }
    global_sid_lock->unlock();

    rpl->file_name = nullptr; /* No need to set rpl.file_name_length */
    rpl->start_position = 4;
    rpl->flags |= MYSQL_RPL_GTID;
    rpl->gtid_set_encoded_size = gtid_executed.get_encoded_length();
    rpl->fix_gtid_set = fix_gtid_set;
    rpl->gtid_set_arg = (void *)&gtid_executed;
  } else {
    rpl->file_name_length = 0;
    rpl->file_name = mi->get_master_log_name();
    rpl->start_position = DBUG_EVALUATE_IF("request_master_log_pos_3", 3,
                                           mi->get_master_log_pos());
  }
  if (mysql_binlog_open(mysql, rpl)) {
    /*
      Something went wrong, so we will just reconnect and retry later
      in the future, we should do a better error analysis, but for
      now we just fill up the error log :-)
    */
    if (mysql_errno(mysql) == ER_NET_READ_INTERRUPTED)
      *suppress_warnings = true;  // Suppress reconnect warning
    else
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_ERROR_RETRYING,
             command_name[command].str, mysql_errno(mysql), mysql_error(mysql),
             mi->connect_retry);
    return 1;
  }

  return 0;
}

/**
  Read one event from the master.

  @param mysql               MySQL connection.
  @param rpl                 Replication stream information.
  @param mi                  Master connection information.
  @param suppress_warnings   true when a normal net read timeout has caused us
                             to try a reconnect. We do not want to print
                             anything to the error log in this case because
                             this an abnormal event in an idle server.

  @retval 'packet_error'     Error.
  @retval  number            Length of packet.
*/

static ulong read_event(MYSQL *mysql, MYSQL_RPL *rpl, Master_info *mi,
                        bool *suppress_warnings) {
  DBUG_TRACE;

  *suppress_warnings = false;
  /*
    my_real_read() will time us out
    We check if we were told to die, and if not, try reading again
  */
#ifndef DBUG_OFF
  if (disconnect_slave_event_count && !(mi->events_until_exit--))
    return packet_error;
#endif

  if (mysql_binlog_fetch(mysql, rpl)) {
    if (mysql_errno(mysql) == ER_NET_READ_INTERRUPTED) {
      /*
        We are trying a normal reconnect after a read timeout;
        we suppress prints to .err file as long as the reconnect
        happens without problems
      */
      *suppress_warnings = true;
    } else if (!mi->abort_slave) {
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_ERROR_READING_FROM_SERVER,
             mi->get_for_channel_str(), mysql_error(mysql), mysql_errno(mysql));
    }
    return packet_error;
  }

  /* Check if eof packet */
  if (rpl->size == 0) {
    LogErr(SYSTEM_LEVEL, ER_RPL_SLAVE_DUMP_THREAD_KILLED_BY_MASTER,
           mi->get_for_channel_str(), ::server_uuid, mysql_error(mysql));
    return packet_error;
  }

  DBUG_PRINT("exit", ("len: %lu  net->read_pos[4]: %d", rpl->size,
                      mysql->net.read_pos[4]));
  return rpl->size - 1;
}

/**
  If this is a lagging slave (specified with CHANGE MASTER TO MASTER_DELAY = X),
  delays accordingly. Also unlocks rli->data_lock.

  Design note: this is the place to unlock rli->data_lock. The lock
  must be held when reading delay info from rli, but it should not be
  held while sleeping.

  @param ev Event that is about to be executed.

  @param thd The sql thread's THD object.

  @param rli The sql thread's Relay_log_info structure.

  @retval 0 If the delay timed out and the event shall be executed.

  @retval nonzero If the delay was interrupted and the event shall be skipped.
*/
static int sql_delay_event(Log_event *ev, THD *thd, Relay_log_info *rli) {
  time_t sql_delay = rli->get_sql_delay();

  DBUG_TRACE;
  mysql_mutex_assert_owner(&rli->data_lock);
  DBUG_ASSERT(!rli->belongs_to_client());

  if (sql_delay) {
    int type = ev->get_type_code();
    time_t sql_delay_end = 0;

    if (rli->commit_timestamps_status == Relay_log_info::COMMIT_TS_UNKNOWN &&
        (type == binary_log::GTID_LOG_EVENT ||
         type == binary_log::ANONYMOUS_GTID_LOG_EVENT)) {
      if (static_cast<Gtid_log_event *>(ev)->has_commit_timestamps &&
          DBUG_EVALUATE_IF("sql_delay_without_timestamps", 0, 1)) {
        rli->commit_timestamps_status = Relay_log_info::COMMIT_TS_FOUND;
      } else {
        rli->commit_timestamps_status = Relay_log_info::COMMIT_TS_NOT_FOUND;
      }
    }

    if (rli->commit_timestamps_status == Relay_log_info::COMMIT_TS_FOUND) {
      if (type == binary_log::GTID_LOG_EVENT ||
          type == binary_log::ANONYMOUS_GTID_LOG_EVENT) {
        /*
          Calculate when we should execute the event.
          The immediate master timestamp is expressed in microseconds.
          Delayed replication is defined in seconds.
          Hence convert immediate_commit_timestamp to seconds here.
        */
        sql_delay_end = ceil((static_cast<Gtid_log_event *>(ev)
                                  ->immediate_commit_timestamp) /
                             1000000.00) +
                        sql_delay;
      }
    } else {
      /*
        the immediate master does not support commit timestamps
        in Gtid_log_events
      */
      if (type != binary_log::ROTATE_EVENT &&
          type != binary_log::FORMAT_DESCRIPTION_EVENT &&
          type != binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
        // Calculate when we should execute the event.
        sql_delay_end = ev->common_header->when.tv_sec +
                        rli->mi->clock_diff_with_master + sql_delay;
      }
    }
    if (sql_delay_end != 0) {
      // The current time.
      time_t now = my_time(0);
      // The amount of time we will have to sleep before executing the event.
      time_t nap_time = 0;

      if (sql_delay_end > now) {
        nap_time = sql_delay_end - now;

        DBUG_PRINT("info",
                   ("sql_delay= %lu "
                    "now= %ld "
                    "sql_delay_end= %ld "
                    "nap_time= %ld",
                    sql_delay, (long)now, (long)sql_delay_end, (long)nap_time));
        DBUG_PRINT("info", ("delaying replication event %lu secs", nap_time));
        rli->start_sql_delay(sql_delay_end);
        mysql_mutex_unlock(&rli->data_lock);
        return slave_sleep(thd, nap_time, sql_slave_killed, rli);
      } else {
        DBUG_PRINT("info", ("sql_delay= %lu "
                            "now= %ld "
                            "sql_delay_end= %ld ",
                            sql_delay, (long)now, (long)sql_delay_end));
      }
    }
  }
  mysql_mutex_unlock(&rli->data_lock);
  return 0;
}

/**
  Applies the given event and advances the relay log position.

  This is needed by the sql thread to execute events from the binlog,
  and by clients executing BINLOG statements.  Conceptually, this
  function does:

  @code
    ev->apply_event(rli);
    ev->update_pos(rli);
  @endcode

  It also does the following maintainance:

   - Initializes the thread's server_id and time; and the event's
     thread.

   - If !rli->belongs_to_client() (i.e., if it belongs to the slave
     sql thread instead of being used for executing BINLOG
     statements), it does the following things: (1) skips events if it
     is needed according to the server id or slave_skip_counter; (2)
     unlocks rli->data_lock; (3) sleeps if required by 'CHANGE MASTER
     TO MASTER_DELAY=X'; (4) maintains the running state of the sql
     thread (rli->thread_state).

   - Reports errors as needed.

  @param ptr_ev a pointer to a reference to the event to apply.

  @param thd The client thread that executes the event (i.e., the
  slave sql thread if called from a replication slave, or the client
  thread if called to execute a BINLOG statement).

  @param rli The relay log info (i.e., the slave's rli if called from
  a replication slave, or the client's thd->rli_fake if called to
  execute a BINLOG statement).

  @note MTS can store NULL to @c ptr_ev location to indicate
        the event is taken over by a Worker.

  @retval SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK
          OK.

  @retval SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR
          Error calling ev->apply_event().

  @retval SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR
          No error calling ev->apply_event(), but error calling
          ev->update_pos().

  @retval SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR
          append_item_to_jobs() failed, thread was killed while waiting
          for successful enqueue on worker.
*/
static enum enum_slave_apply_event_and_update_pos_retval
apply_event_and_update_pos(Log_event **ptr_ev, THD *thd, Relay_log_info *rli) {
  int exec_res = 0;
  bool skip_event = false;
  Log_event *ev = *ptr_ev;
  Log_event::enum_skip_reason reason = Log_event::EVENT_SKIP_NOT;

  DBUG_TRACE;

  DBUG_PRINT("exec_event",
             ("%s(type_code: %d; server_id: %d)", ev->get_type_str(),
              ev->get_type_code(), ev->server_id));
  DBUG_PRINT("info",
             ("thd->options: %s%s; rli->last_event_start_time: %lu",
              FLAGSTR(thd->variables.option_bits, OPTION_NOT_AUTOCOMMIT),
              FLAGSTR(thd->variables.option_bits, OPTION_BEGIN),
              (ulong)rli->last_event_start_time));

  /*
    Execute the event to change the database and update the binary
    log coordinates, but first we set some data that is needed for
    the thread.

    The event will be executed unless it is supposed to be skipped.

    Queries originating from this server must be skipped.  Low-level
    events (Format_description_log_event, Rotate_log_event,
    Stop_log_event) from this server must also be skipped. But for
    those we don't want to modify 'group_master_log_pos', because
    these events did not exist on the master.
    Format_description_log_event is not completely skipped.

    Skip queries specified by the user in 'slave_skip_counter'.  We
    can't however skip events that has something to do with the log
    files themselves.

    Filtering on own server id is extremely important, to ignore
    execution of events created by the creation/rotation of the relay
    log (remember that now the relay log starts with its Format_desc,
    has a Rotate etc).
  */
  /*
     Set the unmasked and actual server ids from the event
   */
  thd->server_id = ev->server_id;  // use the original server id for logging
  thd->unmasked_server_id = ev->common_header->unmasked_server_id;
  thd->set_time();  // time the query
  thd->lex->set_current_select(nullptr);
  if (!ev->common_header->when.tv_sec)
    my_micro_time_to_timeval(my_micro_time(), &ev->common_header->when);
  ev->thd = thd;  // because up to this point, ev->thd == 0

  if (!(rli->is_mts_recovery() &&
        bitmap_is_set(&rli->recovery_groups, rli->mts_recovery_index))) {
    reason = ev->shall_skip(rli);
  }
#ifndef DBUG_OFF
  if (rli->is_mts_recovery()) {
    DBUG_PRINT("mts",
               ("Mts is recovering %d, number of bits set %d, "
                "bitmap is set %d, index %lu.\n",
                rli->is_mts_recovery(), bitmap_bits_set(&rli->recovery_groups),
                bitmap_is_set(&rli->recovery_groups, rli->mts_recovery_index),
                rli->mts_recovery_index));
  }
#endif
  if (reason == Log_event::EVENT_SKIP_COUNT) {
    --rli->slave_skip_counter;
    skip_event = true;
  }
  set_timespec_nsec(&rli->ts_exec[0], 0);
  rli->stats_read_time += diff_timespec(&rli->ts_exec[0], &rli->ts_exec[1]);

  if (reason == Log_event::EVENT_SKIP_NOT) {
    // Sleeps if needed, and unlocks rli->data_lock.
    if (sql_delay_event(ev, thd, rli))
      return SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK;

    exec_res = ev->apply_event(rli);

    DBUG_EXECUTE_IF("simulate_stop_when_mts_in_group",
                    if (rli->mts_group_status == Relay_log_info::MTS_IN_GROUP &&
                        rli->curr_group_seen_begin)
                        DBUG_SET("+d,stop_when_mts_in_group"););

    if (!exec_res && (ev->worker != rli)) {
      if (is_mts_parallel_type_dependency(rli)) {
        DBUG_ASSERT(ev->worker == nullptr);
        if (ev->m_mts_dep_allowed &&
            !static_cast<Mts_submode_dependency *>(rli->current_mts_submode)
                 ->schedule_dep(rli, ev)) {
          *ptr_ev = nullptr;
          return SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR;
        }
      } else if (ev->worker) {
        Slave_job_item item = {ev, rli->get_event_relay_log_number(),
                               rli->get_event_start_pos()};
        Slave_job_item *job_item = &item;
        Slave_worker *w = (Slave_worker *)ev->worker;
        // specially marked group typically with OVER_MAX_DBS_IN_EVENT_MTS db:s
        bool need_sync = ev->is_mts_group_isolated();

        // all events except BEGIN-query must be marked with a non-NULL Worker
        DBUG_ASSERT(((Slave_worker *)ev->worker) == rli->last_assigned_worker);

        DBUG_PRINT("Log_event::apply_event:",
                   ("-> job item data %p to W_%lu", job_item->data, w->id));

        // Reset mts in-group state
        if (rli->mts_group_status == Relay_log_info::MTS_END_GROUP) {
          // CGAP cleanup
          rli->curr_group_assigned_parts.clear();
          // reset the B-group and Gtid-group marker
          rli->curr_group_seen_begin = rli->curr_group_seen_gtid = false;
          rli->curr_group_seen_metadata = false;
          rli->last_assigned_worker = nullptr;
        }
        /*
           Stroring GAQ index of the group that the event belongs to
           in the event. Deferred events are handled similarly below.
        */
        ev->mts_group_idx = rli->gaq->assigned_group_index;

        bool append_item_to_jobs_error = false;
        if (rli->curr_group_da.size() > 0) {
          /*
            the current event sorted out which partion the current group
            belongs to. It's time now to processed deferred array events.
          */
          for (uint i = 0; i < rli->curr_group_da.size(); i++) {
            Slave_job_item da_item = rli->curr_group_da[i];
            DBUG_PRINT("mts", ("Assigning job %llu to worker %lu",
                               (da_item.data)->common_header->log_pos, w->id));
            da_item.data->mts_group_idx =
                rli->gaq->assigned_group_index;  // similarly to above
            if (!append_item_to_jobs_error)
              append_item_to_jobs_error = append_item_to_jobs(&da_item, w, rli);
            if (append_item_to_jobs_error) delete da_item.data;
          }
          rli->curr_group_da.clear();
        }
        if (append_item_to_jobs_error)
          return SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR;

        DBUG_PRINT("mts", ("Assigning job %llu to worker %lu\n",
                           job_item->data->common_header->log_pos, w->id));

        /* Notice `ev' instance can be destoyed after `append()' */
        if (append_item_to_jobs(job_item, w, rli))
          return SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR;
        if (need_sync) {
          /*
            combination of over-max db:s and end of the current group
            forces to wait for the assigned groups completion by assigned
            to the event worker.
            Indeed MTS group status could be safely set to MTS_NOT_IN_GROUP
            after wait_() returns.
            No need to know a possible error out of synchronization call.
          */
          (void)rli->current_mts_submode->wait_for_workers_to_finish(rli);
        }
      }
      *ptr_ev = nullptr;  // announcing the event is passed to w-worker

      if (rli->is_parallel_exec() && rli->mts_events_assigned % 1024 == 1) {
        time_t my_now = my_time(0);

        if ((my_now - rli->mts_last_online_stat) >= mts_online_stat_period) {
          LogErr(INFORMATION_LEVEL, ER_RPL_MTS_STATISTICS,
                 rli->get_for_channel_str(),
                 static_cast<unsigned long>(my_now - rli->mts_last_online_stat),
                 rli->mts_events_assigned, rli->mts_wq_overrun_cnt,
                 rli->mts_wq_overfill_cnt, rli->wq_size_waits_cnt,
                 rli->mts_total_wait_overlap.load(),
                 rli->mts_wq_no_underrun_cnt, rli->mts_total_wait_worker_avail);
          rli->mts_last_online_stat = my_now;
        }
      }
    }
  } else
    mysql_mutex_unlock(&rli->data_lock);

  set_timespec_nsec(&rli->ts_exec[1], 0);
  rli->stats_exec_time += diff_timespec(&rli->ts_exec[1], &rli->ts_exec[0]);

  DBUG_PRINT("info", ("apply_event error = %d", exec_res));
  if (exec_res == 0) {
    /*
      Positions are not updated here when an XID is processed. To make
      a slave crash-safe, positions must be updated while processing a
      XID event and as such do not need to be updated here again.

      However, if the event needs to be skipped, this means that it
      will not be processed and then positions need to be updated here.

      DDL:s that are not yet committed, as indicated by
      @c has_ddl_committed flag, visit the block.

      See sql/rpl_rli.h for further details.
    */
    int error = 0;
    if (*ptr_ev &&
        ((ev->get_type_code() != binary_log::XID_EVENT &&
          !is_committed_ddl(*ptr_ev)) ||
         skip_event ||
         (rli->is_mts_recovery() && !is_gtid_event(ev) &&
          (ev->ends_group() || !rli->mts_recovery_group_seen_begin) &&
          bitmap_is_set(&rli->recovery_groups, rli->mts_recovery_index)))) {
#ifndef DBUG_OFF
      /*
        This only prints information to the debug trace.

        TODO: Print an informational message to the error log?
      */
      static const char *const explain[] = {
          // EVENT_SKIP_NOT,
          "not skipped",
          // EVENT_SKIP_IGNORE,
          "skipped because event should be ignored",
          // EVENT_SKIP_COUNT
          "skipped because event skip counter was non-zero"};
      DBUG_PRINT("info",
                 ("OPTION_BEGIN: %d; IN_STMT: %d",
                  static_cast<bool>(thd->variables.option_bits & OPTION_BEGIN),
                  rli->get_flag(Relay_log_info::IN_STMT)));
      DBUG_PRINT("skip_event",
                 ("%s event was %s", ev->get_type_str(), explain[reason]));
#endif

      error = ev->update_pos(rli);
      /*
        Slave skips an event if the slave_skip_counter is greater than zero.
        We have to free thd's mem_root here after we update the positions
        in the repository table if the event is a skipped event.
        Otherwise, imagine a situation where slave_skip_counter is big number
        and slave is skipping the events and updating the repository.
        All the memory used while these operations are going on is never
        freed unless slave starts executing the events (after slave_skip_counter
        becomes zero).

        Hence we free thd's mem_root here if it is a skipped event.
        (freeing mem_root generally happens from Query_log_event::do_apply_event
        or Rows_log_event::do_apply_event when they find the end of
        the group event).
      */
      if (skip_event) free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));

#ifndef DBUG_OFF
      DBUG_PRINT("info", ("update_pos error = %d", error));
      if (!rli->belongs_to_client()) {
        char buf[22];
        DBUG_PRINT("info",
                   ("group %s %s", llstr(rli->get_group_relay_log_pos(), buf),
                    rli->get_group_relay_log_name()));
        DBUG_PRINT("info",
                   ("event %s %s", llstr(rli->get_event_relay_log_pos(), buf),
                    rli->get_event_relay_log_name()));
      }
#endif
    } else {
      /*
        INTVAR_EVENT, RAND_EVENT, USER_VAR_EVENT and ROWS_QUERY_LOG_EVENT are
        deferred event. It means ev->worker is NULL.
      */
      DBUG_ASSERT(*ptr_ev == ev || rli->is_parallel_exec() ||
                  (!ev->worker &&
                   (ev->get_type_code() == binary_log::INTVAR_EVENT ||
                    ev->get_type_code() == binary_log::RAND_EVENT ||
                    ev->get_type_code() == binary_log::USER_VAR_EVENT ||
                    ev->get_type_code() == binary_log::ROWS_QUERY_LOG_EVENT)));

      rli->inc_event_relay_log_pos();
    }

    if (!error && rli->is_mts_recovery() &&
        ev->get_type_code() != binary_log::ROTATE_EVENT &&
        ev->get_type_code() != binary_log::FORMAT_DESCRIPTION_EVENT &&
        ev->get_type_code() != binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
      if (ev->starts_group()) {
        rli->mts_recovery_group_seen_begin = true;
      } else if ((ev->ends_group() || !rli->mts_recovery_group_seen_begin) &&
                 !is_gtid_event(ev)) {
        rli->mts_recovery_index++;
        if (--rli->mts_recovery_group_cnt == 0) {
          rli->mts_recovery_index = 0;
          LogErr(INFORMATION_LEVEL, ER_RPL_MTS_RECOVERY_COMPLETE,
                 rli->get_for_channel_str(), rli->get_group_relay_log_name(),
                 rli->get_group_relay_log_pos(),
                 rli->get_group_master_log_name(),
                 rli->get_group_master_log_pos());
          /*
             Few tests wait for UNTIL_SQL_AFTER_MTS_GAPS completion.
             Due to exisiting convention the status won't change
             prior to slave restarts.
             So making of UNTIL_SQL_AFTER_MTS_GAPS completion isdone here,
             and only in the debug build to make the test to catch the change
             despite a faulty design of UNTIL checking before execution.
          */
          if (rli->until_condition ==
              Relay_log_info::UNTIL_SQL_AFTER_MTS_GAPS) {
            rli->until_condition = Relay_log_info::UNTIL_DONE;
          }
          // reset the Worker tables to remove last slave session time info
          if ((error = rli->mts_finalize_recovery())) {
            (void)Rpl_info_factory::reset_workers(rli);
          }
        }
        rli->mts_recovery_group_seen_begin = false;
        if (!error) error = rli->flush_info(true);
      }
    }

    if (error) {
      /*
        The update should not fail, so print an error message and
        return an error code.

        TODO: Replace this with a decent error message when merged
        with BUG#24954 (which adds several new error message).
      */
      char buf[22];
      rli->report(ERROR_LEVEL, ER_UNKNOWN_ERROR,
                  "It was not possible to update the positions"
                  " of the relay log information: the slave may"
                  " be in an inconsistent state."
                  " Stopped in %s position %s",
                  rli->get_group_relay_log_name(),
                  llstr(rli->get_group_relay_log_pos(), buf));
      return SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR;
    }
  }

  return exec_res ? SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR
                  : SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK;
}

/**
  Let the worker applying the current group to rollback and gracefully
  finish its work before.

  @param rli The slave's relay log info.

  @param ev a pointer to the event on hold before applying this rollback
  procedure.

  @retval false The rollback succeeded.

  @retval true  There was an error while injecting events.
*/
static bool coord_handle_partial_binlogged_transaction(Relay_log_info *rli,
                                                       const Log_event *ev) {
  DBUG_TRACE;
  /*
    This function is called holding the rli->data_lock.
    We must return it still holding this lock, except in the case of returning
    error.
  */
  mysql_mutex_assert_owner(&rli->data_lock);
  THD *thd = rli->info_thd;

  if (!rli->curr_group_seen_begin) {
    DBUG_PRINT("info", ("Injecting QUERY(BEGIN) to rollback worker"));
    Log_event *begin_event = new Query_log_event(thd, STRING_WITH_LEN("BEGIN"),
                                                 true,  /* using_trans */
                                                 false, /* immediate */
                                                 true,  /* suppress_use */
                                                 0,     /* error */
                                                 true /* ignore_command */);
    ((Query_log_event *)begin_event)->db = "";
    begin_event->common_header->data_written = 0;
    begin_event->server_id = ev->server_id;
    /*
      We must be careful to avoid SQL thread increasing its position
      farther than the event that triggered this QUERY(BEGIN).
    */
    begin_event->common_header->log_pos = ev->common_header->log_pos;
    begin_event->future_event_relay_log_pos = ev->future_event_relay_log_pos;

    /*
      The timestamp of this event needs to be adjusted so that it does not
      interfere with the SBM calculated based on the timestamps from the
      primary's binlog events.  Setting to 0 will have the event inherit the
      current time, so use the current last_master_timestamp, or 1, if
      last_master_timestamp is not yet set.
    */
    begin_event->common_header->when.tv_sec =
        rli->last_master_timestamp > 0 ? rli->last_master_timestamp : 1;

    if (apply_event_and_update_pos(&begin_event, thd, rli) !=
        SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK) {
      delete begin_event;
      return true;
    }
    mysql_mutex_lock(&rli->data_lock);
  }

  DBUG_PRINT("info", ("Injecting QUERY(ROLLBACK) to rollback worker"));
  Log_event *rollback_event = new Query_log_event(
      thd, STRING_WITH_LEN("ROLLBACK"), true, /* using_trans */
      false,                                  /* immediate */
      true,                                   /* suppress_use */
      0,                                      /* error */
      true /* ignore_command */);
  ((Query_log_event *)rollback_event)->db = "";
  rollback_event->common_header->data_written = 0;
  rollback_event->server_id = ev->server_id;
  /*
    We must be careful to avoid SQL thread increasing its position
    farther than the event that triggered this QUERY(ROLLBACK).
  */
  rollback_event->common_header->log_pos = ev->common_header->log_pos;
  rollback_event->future_event_relay_log_pos = ev->future_event_relay_log_pos;

  /*
    The timestamp of this event needs to be adjusted so that it does not
    interfere with the SBM calculated based on the timestamps from the
    primary's binlog events.  Setting to 0 will have the event inherit the
    current time, so use the current last_master_timestamp, or 1, if
    last_master_timestamp is not yet set.
  */
  rollback_event->common_header->when.tv_sec =
      rli->last_master_timestamp > 0 ? rli->last_master_timestamp : 1;

  ((Query_log_event *)rollback_event)->rollback_injected_by_coord = true;

  if (apply_event_and_update_pos(&rollback_event, thd, rli) !=
      SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK) {
    delete rollback_event;
    return true;
  }
  mysql_mutex_lock(&rli->data_lock);

  return false;
}

/**
  Top-level function for executing the next event in the relay log.
  This is called from the SQL thread.

  This function reads the event from the relay log, executes it, and
  advances the relay log position.  It also handles errors, etc.

  This function may fail to apply the event for the following reasons:

   - The position specfied by the UNTIL condition of the START SLAVE
     command is reached.

   - It was not possible to read the event from the log.

   - The slave is killed.

   - An error occurred when applying the event, and the event has been
     tried slave_trans_retries times.  If the event has been retried
     fewer times, 0 is returned.

   - init_info or init_relay_log_pos failed. (These are called
     if a failure occurs when applying the event.)

   - An error occurred when updating the binlog position.

  @retval 0 The event was applied.

  @retval 1 The event was not applied.
*/
static int exec_relay_log_event(THD *thd, Relay_log_info *rli,
                                Rpl_applier_reader *applier_reader,
                                Log_event *in) {
  DBUG_TRACE;

  /*
     We acquire this mutex since we need it for all operations except
     event execution. But we will release it in places where we will
     wait for something for example inside of next_event().
   */
  mysql_mutex_lock(&rli->data_lock);

  Log_event *ev = nullptr;
#ifndef DBUG_OFF
  if (!abort_slave_event_count || rli->events_until_exit--)
#endif
    ev = in;

  Log_event **ptr_ev = nullptr;
  RLI_current_event_raii rli_c_ev(rli, ev);

  if (ev != nullptr) {
    /*
      To avoid assigned event groups exceeding rli->checkpoint_group, it
      need force to compute checkpoint.
    */
    bool force = rli->rli_checkpoint_seqno >= rli->checkpoint_group;
    if (force || rli->is_time_for_mts_checkpoint()) {
      mysql_mutex_unlock(&rli->data_lock);
      if (mts_checkpoint_routine(rli, force)) {
        delete ev;
        return 1;
      }
      mysql_mutex_lock(&rli->data_lock);
    }
  }

  /*
    It should be checked after calling mts_checkpoint_routine(), because that
    function could be interrupted by kill while 'force' is true.
  */
  if (sql_slave_killed(thd, rli)) {
    mysql_mutex_unlock(&rli->data_lock);
    delete ev;

    LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_ERROR_READING_RELAY_LOG_EVENTS,
           rli->get_for_channel_str(), "slave SQL thread was killed");
    return 1;
  }

  if (ev) {
    if (enable_raft_plugin &&
        ev->get_type_code() == binary_log::METADATA_EVENT) {
      Metadata_log_event *mev = static_cast<Metadata_log_event *>(ev);
      if (mev->does_exist(binary_log::Metadata_event::Metadata_event_types::
                              RAFT_TERM_INDEX_TYPE)) {
        const int64_t term = mev->get_raft_term();
        const int64_t index = mev->get_raft_index();
        if (rli->last_opid.first != -1 && rli->last_opid.second != -1 &&
            (index != rli->last_opid.second + 1 ||
             term < rli->last_opid.first)) {
          char msg[1024];
          snprintf(
              msg, sizeof(msg),
              "Out of order opid found last opid=%ld:%ld, current opid=%ld:%ld",
              rli->last_opid.first, rli->last_opid.second, term, index);
          rli->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_READ_FAILURE,
                      ER_THD(thd, ER_SLAVE_RELAY_LOG_READ_FAILURE), msg);
          rli->abort_slave = 1;
          mysql_mutex_unlock(&rli->data_lock);
          delete ev;
          return 1;
        }
        rli->last_opid = std::make_pair(term, index);
      }
    }

    if (rli->is_row_format_required()) {
      bool info_error{false};
      binary_log::Log_event_basic_info log_event_info;
      std::tie(info_error, log_event_info) = extract_log_event_basic_info(ev);

      if (info_error ||
          rli->transaction_parser.feed_event(log_event_info, true)) {
        /* purecov: begin inspected */
        LogErr(WARNING_LEVEL,
               ER_RPL_SLAVE_SQL_THREAD_DETECTED_UNEXPECTED_EVENT_SEQUENCE);
        /* purecov: end */
      }

      if (info_error || rli->transaction_parser.check_row_logging_constraints(
                            log_event_info)) {
        rli->report(
            ERROR_LEVEL,
            ER_RPL_SLAVE_APPLY_LOG_EVENT_FAILED_INVALID_NON_ROW_FORMAT,
            ER_THD(thd,
                   ER_RPL_SLAVE_APPLY_LOG_EVENT_FAILED_INVALID_NON_ROW_FORMAT),
            rli->mi->get_channel());
        rli->abort_slave = 1;
        mysql_mutex_unlock(&rli->data_lock);
        delete ev;
        return 1;
      }
    }

    enum enum_slave_apply_event_and_update_pos_retval exec_res;

    ptr_ev = &ev;
    /*
      Even if we don't execute this event, we keep the master timestamp,
      so that seconds behind master shows correct delta (there are events
      that are not replayed, so we keep falling behind).

      If it is an artificial event, or a relay log event (IO thread generated
      event) or ev->when is set to 0, or a FD from master, or a heartbeat
      event with server_id '0' then  we don't update the last_master_timestamp.

      In case of parallel execution last_master_timestamp is only updated when
      a job is taken out of GAQ. Thus when last_master_timestamp is 0 (which
      indicates that GAQ is empty, all slave workers are waiting for events from
      the Coordinator), we need to initialize it with a timestamp from the first
      event to be executed in parallel.
    */
    if ((!rli->is_parallel_exec() || rli->last_master_timestamp == 0) &&
        !(ev->is_artificial_event() || ev->is_relay_log_event() ||
          ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT ||
          ev->server_id == 0)) {
      const auto v = ev->common_header->when.tv_sec + (time_t)ev->exec_time;
      rli->set_last_master_timestamp(v, v * 1000);
      DBUG_ASSERT(rli->last_master_timestamp >= 0);
    }

    if (rli->is_until_satisfied_before_dispatching_event(ev)) {
      /*
        Setting abort_slave flag because we do not want additional message about
        error in query execution to be printed.
      */
      rli->abort_slave = true;
      mysql_mutex_unlock(&rli->data_lock);
      delete ev;
      return SLAVE_APPLY_EVENT_UNTIL_REACHED;
    }

    { /**
               The following failure injecion works in cooperation with tests
               setting @@global.debug= 'd,incomplete_group_in_relay_log'.
               Xid or Commit events are not executed to force the slave sql
               read hanging if the realy log does not have any more events.
            */
      DBUG_EXECUTE_IF(
          "incomplete_group_in_relay_log",
          if ((ev->get_type_code() == binary_log::XID_EVENT) ||
              ((ev->get_type_code() == binary_log::QUERY_EVENT) &&
               strcmp("COMMIT", ((Query_log_event *)ev)->query) == 0)) {
            DBUG_ASSERT(thd->get_transaction()->cannot_safely_rollback(
                Transaction_ctx::SESSION));
            rli->abort_slave = 1;
            mysql_mutex_unlock(&rli->data_lock);
            delete ev;
            rli->inc_event_relay_log_pos();
            return 0;
          };);
    }

    /*
      GTID protocol will put a FORMAT_DESCRIPTION_EVENT from the master with
      log_pos != 0 after each (re)connection if auto positioning is enabled.
      This means that the SQL thread might have already started to apply the
      current group but, as the IO thread had to reconnect, it left this
      group incomplete and will start it again from the beginning.
      So, before applying this FORMAT_DESCRIPTION_EVENT, we must let the
      worker roll back the current group and gracefully finish its work,
      before starting to apply the new (complete) copy of the group.
    */
    if (ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT &&
        ev->server_id != ::server_id && ev->common_header->log_pos != 0 &&
        rli->is_parallel_exec() && rli->curr_group_seen_gtid) {
      if (coord_handle_partial_binlogged_transaction(rli, ev))
        /*
          In the case of an error, coord_handle_partial_binlogged_transaction
          will not try to get the rli->data_lock again.
        */
        return 1;
    }

    /* ptr_ev can change to NULL indicating MTS coorinator passed to a Worker */
    exec_res = apply_event_and_update_pos(ptr_ev, thd, rli);
    /*
      Note: the above call to apply_event_and_update_pos executes
      mysql_mutex_unlock(&rli->data_lock);
    */

    /* For deferred events, the ptr_ev is set to NULL
        in Deferred_log_events::add() function.
        Hence deferred events wont be deleted here.
        They will be deleted in Deferred_log_events::rewind() funciton.
    */
    if (*ptr_ev) {
      DBUG_ASSERT(*ptr_ev == ev);  // event remains to belong to Coordinator

      DBUG_EXECUTE_IF("dbug.calculate_sbm_after_previous_gtid_log_event", {
        if (ev->get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
          rpl_slave_debug_point(DBUG_RPL_S_SBM_AFTER_PREVIOUS_GTID_EV, thd);
        }
      };);
      DBUG_EXECUTE_IF("dbug.calculate_sbm_after_fake_rotate_log_event", {
        if (ev->get_type_code() == binary_log::ROTATE_EVENT &&
            ev->is_artificial_event()) {
          rpl_slave_debug_point(DBUG_RPL_S_SBM_AFTER_FAKE_ROTATE_EV, thd);
        }
      };);
      /*
        Format_description_log_event should not be deleted because it will be
        used to read info about the relay log's format; it will be deleted when
        the SQL thread does not need it, i.e. when this thread terminates.
        ROWS_QUERY_LOG_EVENT is destroyed at the end of the current statement
        clean-up routine but ones with trx meta data are deleted here.
      */
      if (ev->get_type_code() != binary_log::FORMAT_DESCRIPTION_EVENT &&
          ev->get_type_code() != binary_log::ROWS_QUERY_LOG_EVENT) {
        DBUG_PRINT("info", ("Deleting the event after it has been executed"));
        delete ev;
        /*
          Raii guard is explicitly instructed to invalidate
          otherwise bogus association of the execution context with the being
          destroyed above event.
        */
        ev = rli->current_event = nullptr;
      }
    }

    /*
      exec_res == SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR
                  update_log_pos failed: this should not happen, so we
                  don't retry.
      exec_res == SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR
                  append_item_to_jobs() failed, this happened because
                  thread was killed while waiting for enqueue on worker.
    */
    if (exec_res >= SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR) {
      delete ev;
      return 1;
    }

    if (slave_trans_retries) {
      int temp_err = 0;
      bool silent = false;
      if (exec_res && !is_mts_worker(thd) /* no reexecution in MTS mode */ &&
          (temp_err = rli->has_temporary_error(thd, 0, &silent)) &&
          !thd->get_transaction()->cannot_safely_rollback(
              Transaction_ctx::SESSION)) {
        const char *errmsg;
        /*
          We were in a transaction which has been rolled back because of a
          temporary error;
          let's seek back to BEGIN log event and retry it all again.
          Note, if lock wait timeout (innodb_lock_wait_timeout exceeded)
          there is no rollback since 5.0.13 (ref: manual).
          We have to not only seek but also
          a) init_info(), to seek back to hot relay log's start for later
          (for when we will come back to this hot log after re-processing the
          possibly existing old logs where BEGIN is: applier_reader will
          then need the cache to be at position 0 (see comments at beginning of
          init_info()).
          b) init_relay_log_pos(), because the BEGIN may be an older relay log.
        */
        if (rli->trans_retries < slave_trans_retries) {
          /*
            The transactions has to be rolled back before
            load_mi_and_rli_from_repositories is called. Because
            load_mi_and_rli_from_repositories will starts a new
            transaction if master_info_repository is TABLE.
          */
          rli->cleanup_context(thd, true);
          /*
            Temporary error status is both unneeded and harmful for following
            open-and-lock slave system tables but store its number first for
            monitoring purposes.
          */
          uint temp_trans_errno = thd->get_stmt_da()->mysql_errno();
          thd->clear_error();
          applier_reader->close();
          /*
             We need to figure out if there is a test case that covers
             this part. \Alfranio.
          */
          if (load_mi_and_rli_from_repositories(rli->mi, false, SLAVE_SQL))
            LogErr(ERROR_LEVEL,
                   ER_RPL_SLAVE_FAILED_TO_INIT_MASTER_INFO_STRUCTURE,
                   rli->get_for_channel_str());
          else if (applier_reader->open(&errmsg))
            LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_INIT_RELAY_LOG_POSITION,
                   rli->get_for_channel_str(), errmsg);
          else {
            exec_res = SLAVE_APPLY_EVENT_RETRY;
            /* chance for concurrent connection to get more locks */
            slave_sleep(thd,
                        min<ulong>(rli->trans_retries, MAX_SLAVE_RETRY_PAUSE),
                        sql_slave_killed, rli);
            mysql_mutex_lock(&rli->data_lock);  // because of SHOW STATUS
            if (!silent) {
              rli->trans_retries++;
              if (rli->is_processing_trx()) {
                rli->retried_processing(temp_trans_errno,
                                        ER_THD_NONCONST(thd, temp_trans_errno),
                                        rli->trans_retries);
              }
            }
            rli->retried_trans++;
            mysql_mutex_unlock(&rli->data_lock);
#ifndef DBUG_OFF
            if (rli->trans_retries == 2 || rli->trans_retries == 6)
              DBUG_EXECUTE_IF("rpl_ps_tables_worker_retry", {
                rpl_slave_debug_point(DBUG_RPL_S_PS_TABLE_WORKER_RETRY);
              };);

#endif
            DBUG_PRINT("info", ("Slave retries transaction "
                                "rli->trans_retries: %lu",
                                rli->trans_retries));
          }
        } else {
          thd->fatal_error();
          rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                      "Slave SQL thread retried transaction %lu time(s) "
                      "in vain, giving up. Consider raising the value of "
                      "the slave_transaction_retries variable.",
                      rli->trans_retries);
        }
      } else if ((exec_res && !temp_err) ||
                 (opt_using_transactions &&
                  rli->get_group_relay_log_pos() ==
                      rli->get_event_relay_log_pos())) {
        /*
          Only reset the retry counter if the entire group succeeded
          or failed with a non-transient error.  On a successful
          event, the execution will proceed as usual; in the case of a
          non-transient error, the slave will stop with an error.
         */
        rli->trans_retries = 0;  // restart from fresh
        DBUG_PRINT("info", ("Resetting retry counter, rli->trans_retries: %lu",
                            rli->trans_retries));
      }
    }
    if (exec_res) {
      delete ev;
      /* Raii object is explicitly updated 'cos this branch doesn't end func */
      rli->current_event = nullptr;
    } else if (rli->is_until_satisfied_after_dispatching_event()) {
      mysql_mutex_lock(&rli->data_lock);
      rli->abort_slave = true;
      mysql_mutex_unlock(&rli->data_lock);
      return SLAVE_APPLY_EVENT_UNTIL_REACHED;
    }
    return exec_res;
  }

  /*
    It is impossible to read next event to finish the event group whenever a
    read event error happens. So MTS group status is set to MTS_KILLED_GROUP to
    force stop.
  */
  if (rli->mts_group_status == Relay_log_info::MTS_IN_GROUP)
    rli->mts_group_status = Relay_log_info::MTS_KILLED_GROUP;

  mysql_mutex_unlock(&rli->data_lock);
  rli->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_READ_FAILURE,
              ER_THD(thd, ER_SLAVE_RELAY_LOG_READ_FAILURE),
              "\
Could not parse relay log event entry. The possible reasons are: the master's \
binary log is corrupted (you can check this by running 'mysqlbinlog' on the \
binary log), the slave's relay log is corrupted (you can check this by running \
'mysqlbinlog' on the relay log), a network problem, the server was unable to \
fetch a keyring key required to open an encrypted relay log file, or a bug in \
the master's or slave's MySQL code. If you want to check the master's binary \
log or slave's relay log, you will be able to know their names by issuing \
'SHOW SLAVE STATUS' on this slave.\
");
  return SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR;
}

static bool check_io_slave_killed(THD *thd, Master_info *mi, const char *info) {
  if (io_slave_killed(thd, mi)) {
    if (info)
      LogErr(INFORMATION_LEVEL, ER_RPL_IO_THREAD_KILLED, info,
             mi->get_for_channel_str());
    return true;
  }
  return false;
}

/**
  @brief Try to reconnect slave IO thread.

  @details Terminates current connection to master, sleeps for
  @c mi->connect_retry msecs and initiates new connection with
  @c safe_reconnect(). Variable pointed by @c retry_count is increased -
  if it exceeds @c mi->retry_count then connection is not re-established
  and function signals error.
  Unless @c suppres_warnings is true, a warning is put in the server error log
  when reconnecting. The warning message and messages used to report errors
  are taken from @c messages array. In case @c mi->retry_count is exceeded,
  no messages are added to the log.

  @param[in]     thd                 Thread context.
  @param[in]     mysql               MySQL connection.
  @param[in]     mi                  Master connection information.
  @param[in,out] retry_count         Number of attempts to reconnect.
  @param[in]     suppress_warnings   true when a normal net read timeout
                                     has caused to reconnecting.
  @param[in]     messages            Messages to print/log, see
                                     reconnect_messages[] array.

  @retval        0                   OK.
  @retval        1                   There was an error.
*/

static int try_to_reconnect(THD *thd, MYSQL *mysql, Master_info *mi,
                            uint *retry_count, bool suppress_warnings,
                            const char *messages[SLAVE_RECON_MSG_MAX]) {
  mi->slave_running = MYSQL_SLAVE_RUN_NOT_CONNECT;
  thd->proc_info = messages[SLAVE_RECON_MSG_WAIT];
  thd->clear_active_vio();
  end_server(mysql);
  if ((*retry_count)++) {
    if (*retry_count > mi->retry_count) return 1;  // Don't retry forever
    slave_sleep(thd, mi->connect_retry, io_slave_killed, mi);
  }
  if (check_io_slave_killed(thd, mi, messages[SLAVE_RECON_MSG_KILLED_WAITING]))
    return 1;
  thd->proc_info = messages[SLAVE_RECON_MSG_AFTER];
  if (!suppress_warnings) {
    char llbuff[22];
    /*
      Raise a warining during registering on master/requesting dump.
      Log a message reading event.
    */
    if (messages[SLAVE_RECON_MSG_COMMAND][0]) {
      char buf[256];
      snprintf(buf, sizeof(buf), messages[SLAVE_RECON_MSG_FAILED],
               mi->get_io_rpl_log_name(),
               llstr(mi->get_master_log_pos(), llbuff));

      mi->report(WARNING_LEVEL, ER_SLAVE_MASTER_COM_FAILURE,
                 ER_THD(thd, ER_SLAVE_MASTER_COM_FAILURE),
                 messages[SLAVE_RECON_MSG_COMMAND], buf);
    } else {
      LogErr(INFORMATION_LEVEL, ER_SLAVE_RECONNECT_FAILED,
             mi->get_io_rpl_log_name(), llstr(mi->get_master_log_pos(), llbuff),
             mi->get_for_channel_str());
    }
  }
  if (safe_reconnect(thd, mysql, mi, true) || io_slave_killed(thd, mi)) {
    LogErr(INFORMATION_LEVEL, ER_SLAVE_KILLED_AFTER_RECONNECT);
    return 1;
  }
  return 0;
}

/**
  Slave IO thread entry point.

  @param arg Pointer to Master_info struct that holds information for
  the IO thread.

  @return Always 0.
*/
extern "C" void *handle_slave_io(void *arg) {
  THD *thd = nullptr;  // needs to be first for thread_stack
  bool thd_added = false;
  MYSQL *mysql;
  Master_info *mi = (Master_info *)arg;
  Relay_log_info *rli = mi->rli;
  char llbuff[22];
  uint retry_count;
  bool suppress_warnings;
  int ret;
  bool successfully_connected;
  bool slave_stats_daemon_created = false;
#ifndef DBUG_OFF
  uint retry_count_reg = 0, retry_count_dump = 0, retry_count_event = 0;
#endif
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  // needs to call my_thread_init(), otherwise we get a coredump in DBUG_ stuff
  my_thread_init();
  {
    DBUG_TRACE;
    // Raft don't use slave IO thread
    DBUG_ASSERT(!enable_raft_plugin);
    DBUG_ASSERT(mi->inited);
    mysql = nullptr;
    retry_count = 0;

    mysql_mutex_lock(&mi->run_lock);
    /* Inform waiting threads that slave has started */
    mi->slave_run_id++;

#ifndef DBUG_OFF
    mi->events_until_exit = disconnect_slave_event_count;
#endif

    thd = new THD;  // note that contructor of THD uses DBUG_ !
    THD_CHECK_SENTRY(thd);
    mi->info_thd = thd;

#ifdef HAVE_PSI_THREAD_INTERFACE
    // save the instrumentation for IO thread in mi->info_thd
    struct PSI_thread *psi = PSI_THREAD_CALL(get_thread)();
    thd_set_psi(mi->info_thd, psi);
#endif
    mysql_thread_set_psi_id(thd->thread_id());
    mysql_thread_set_psi_THD(thd);

    thd->thread_stack = (char *)&thd;  // remember where our stack is
    mi->clear_error();
    mi->slave_running = 1;
    if (init_slave_thread(thd, SLAVE_THD_IO)) {
      mysql_cond_broadcast(&mi->start_cond);
      mysql_mutex_unlock(&mi->run_lock);
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                 "Failed during slave I/O thread initialization ");
      goto err;
    }

    thd_manager->add_thd(thd);
    thd_added = true;

    mi->abort_slave = false;
    mysql_mutex_unlock(&mi->run_lock);
    mysql_cond_broadcast(&mi->start_cond);

    DBUG_PRINT("master_info",
               ("log_file_name: '%s'  position: %s", mi->get_master_log_name(),
                llstr(mi->get_master_log_pos(), llbuff)));

    /* This must be called before run any binlog_relay_io hooks */
    RPL_MASTER_INFO = mi;

    if (RUN_HOOK(binlog_relay_io, thread_start, (thd, mi))) {
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                 "Failed to run 'thread_start' hook");
      goto err;
    }

    if (!(mi->mysql = mysql = mysql_init(nullptr))) {
      mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                 ER_THD(thd, ER_SLAVE_FATAL_ERROR), "error in mysql_init()");
      goto err;
    }
    mysql_extension_set_server_extn(mysql, &mi->server_extn);

    THD_STAGE_INFO(thd, stage_connecting_to_master);

    if (mi->is_set_network_namespace()) {
#ifdef HAVE_SETNS
      if (set_network_namespace(mi->network_namespace)) goto err;
#else
      // Network namespace not supported by the platform. Report error.
      LogErr(ERROR_LEVEL, ER_NETWORK_NAMESPACES_NOT_SUPPORTED);
      goto err;
#endif
      // Save default value of network namespace
      // Set network namespace before sockets be created
    }
    successfully_connected = !safe_connect(thd, mysql, mi);
    // we can get killed during safe_connect
#ifdef HAVE_SETNS
    if (mi->is_set_network_namespace()) {
      // Restore original network namespace used to be before connection has
      // been created
      successfully_connected =
          restore_original_network_namespace() | successfully_connected;
    }
#endif

    if (successfully_connected) {
      LogErr(SYSTEM_LEVEL, ER_RPL_SLAVE_CONNECTED_TO_MASTER_REPLICATION_STARTED,
             mi->get_for_channel_str(), mi->get_user(), mi->host, mi->port,
             mi->get_io_rpl_log_name(),
             llstr(mi->get_master_log_pos(), llbuff));
    } else {
      LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_IO_THREAD_KILLED,
             mi->get_for_channel_str());
      goto err;
    }

  connected:

    /*
      When using auto positioning, the slave IO thread will always start reading
      a transaction from the beginning of the transaction (transaction's first
      event). So, we have to reset the transaction boundary parser after
      (re)connecting.
      If not using auto positioning, the Relay_log_info::rli_init_info() took
      care of putting the mi->transaction_parser in the correct state when
      initializing Received_gtid_set from relay log during slave server starts,
      as the IO thread might had stopped in the middle of a transaction.
    */
    if (mi->is_auto_position()) {
      mi->transaction_parser.reset();
      mi->clear_queueing_trx(true /* need_lock*/);
    }

    ++relay_io_connected;
    DBUG_EXECUTE_IF("dbug.before_get_running_status_yes", {
      rpl_slave_debug_point(DBUG_RPL_S_BEFORE_RUNNING_STATUS, thd);
    };);
    DBUG_EXECUTE_IF("dbug.calculate_sbm_after_previous_gtid_log_event", {
      /* Fake that thread started 3 minutes ago */
      thd->start_time.tv_sec -= 180;
    };);
    DBUG_EXECUTE_IF("dbug.calculate_sbm_after_fake_rotate_log_event", {
      /* Fake that thread started 3 minutes ago */
      thd->start_time.tv_sec -= 180;
    };);
    mysql_mutex_lock(&mi->run_lock);
    mi->slave_running = MYSQL_SLAVE_RUN_CONNECT;
    mysql_mutex_unlock(&mi->run_lock);

    THD_STAGE_INFO(thd, stage_checking_master_version);
    ret = get_master_version_and_clock(mysql, mi);
    if (!ret) ret = get_master_uuid(mysql, mi);
    if (!ret) ret = io_thread_init_commands(mysql, mi);

    if (ret == 1) /* Fatal error */
      goto err;

    if (ret == 2) {
      if (check_io_slave_killed(
              mi->info_thd, mi,
              "Slave I/O thread killed "
              "while calling get_master_version_and_clock(...)"))
        goto err;
      suppress_warnings = false;
      /* Try to reconnect because the error was caused by a transient network
       * problem */
      if (try_to_reconnect(thd, mysql, mi, &retry_count, suppress_warnings,
                           reconnect_messages[SLAVE_RECON_ACT_REG]))
        goto err;
      goto connected;
    }

    /*
      Register ourselves with the master.
    */
    THD_STAGE_INFO(thd, stage_registering_slave_on_master);
    if (register_slave_on_master(mysql, mi, &suppress_warnings)) {
      if (!check_io_slave_killed(thd, mi,
                                 "Slave I/O thread killed "
                                 "while registering slave on master")) {
        LogErr(ERROR_LEVEL, ER_RPL_SLAVE_IO_THREAD_CANT_REGISTER_ON_MASTER);
        if (try_to_reconnect(thd, mysql, mi, &retry_count, suppress_warnings,
                             reconnect_messages[SLAVE_RECON_ACT_REG]))
          goto err;
      } else
        goto err;
      goto connected;
    }

    DBUG_EXECUTE_IF(
        "FORCE_SLAVE_TO_RECONNECT_REG", if (!retry_count_reg) {
          retry_count_reg++;
          LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_FORCING_TO_RECONNECT_IO_THREAD,
                 mi->get_for_channel_str());
          if (try_to_reconnect(thd, mysql, mi, &retry_count, suppress_warnings,
                               reconnect_messages[SLAVE_RECON_ACT_REG]))
            goto err;
          goto connected;
        });

    if (!slave_stats_daemon_created) {
      // start sending secondary lag stats to primary
      slave_stats_daemon_created = start_handle_slave_stats_daemon();
    }
    DBUG_PRINT("info", ("Starting reading binary log from master"));
    while (!io_slave_killed(thd, mi)) {
      MYSQL_RPL rpl;

      THD_STAGE_INFO(thd, stage_requesting_binlog_dump);
      if (request_dump(thd, mysql, &rpl, mi, &suppress_warnings)) {
        LogErr(ERROR_LEVEL, ER_RPL_SLAVE_ERROR_REQUESTING_BINLOG_DUMP,
               mi->get_for_channel_str());
        if (check_io_slave_killed(thd, mi,
                                  "Slave I/O thread killed while \
requesting master dump") ||
            try_to_reconnect(thd, mysql, mi, &retry_count, suppress_warnings,
                             reconnect_messages[SLAVE_RECON_ACT_DUMP]))
          goto err;
        goto connected;
      }
      DBUG_EXECUTE_IF(
          "FORCE_SLAVE_TO_RECONNECT_DUMP", if (!retry_count_dump) {
            retry_count_dump++;
            LogErr(INFORMATION_LEVEL,
                   ER_RPL_SLAVE_FORCING_TO_RECONNECT_IO_THREAD,
                   mi->get_for_channel_str());
            if (try_to_reconnect(thd, mysql, mi, &retry_count,
                                 suppress_warnings,
                                 reconnect_messages[SLAVE_RECON_ACT_DUMP]))
              goto err;
            goto connected;
          });
      const char *event_buf;

      DBUG_ASSERT(mi->last_error().number == 0);
      while (!io_slave_killed(thd, mi)) {
        ulong event_len;
        /*
           We say "waiting" because read_event() will wait if there's nothing to
           read. But if there's something to read, it will not wait. The
           important thing is to not confuse users by saying "reading" whereas
           we're in fact receiving nothing.
        */
        THD_STAGE_INFO(thd, stage_waiting_for_master_to_send_event);
        event_len = read_event(mysql, &rpl, mi, &suppress_warnings);
        if (check_io_slave_killed(thd, mi,
                                  "Slave I/O thread killed while \
reading event"))
          goto err;
        DBUG_EXECUTE_IF(
            "FORCE_SLAVE_TO_RECONNECT_EVENT", if (!retry_count_event) {
              retry_count_event++;
              LogErr(INFORMATION_LEVEL,
                     ER_RPL_SLAVE_FORCING_TO_RECONNECT_IO_THREAD,
                     mi->get_for_channel_str());
              if (try_to_reconnect(thd, mysql, mi, &retry_count,
                                   suppress_warnings,
                                   reconnect_messages[SLAVE_RECON_ACT_EVENT]))
                goto err;
              goto connected;
            });

        if (event_len == packet_error) {
          uint mysql_error_number = mysql_errno(mysql);
          switch (mysql_error_number) {
            case CR_NET_PACKET_TOO_LARGE:
              LogErr(ERROR_LEVEL,
                     ER_RPL_LOG_ENTRY_EXCEEDS_SLAVE_MAX_ALLOWED_PACKET,
                     slave_max_allowed_packet);
              mi->report(
                  ERROR_LEVEL, ER_SERVER_NET_PACKET_TOO_LARGE, "%s",
                  "Got a packet bigger than 'slave_max_allowed_packet' bytes");
              goto err;
            case ER_MASTER_FATAL_ERROR_READING_BINLOG:
              mi->report(ERROR_LEVEL,
                         ER_SERVER_MASTER_FATAL_ERROR_READING_BINLOG,
                         ER_THD(thd, ER_MASTER_FATAL_ERROR_READING_BINLOG),
                         mysql_error_number, mysql_error(mysql));
              goto err;
            case ER_OUT_OF_RESOURCES:
              LogErr(ERROR_LEVEL, ER_RPL_SLAVE_STOPPING_AS_MASTER_OOM);
              mi->report(ERROR_LEVEL, ER_SERVER_OUT_OF_RESOURCES, "%s",
                         ER_THD(thd, ER_SERVER_OUT_OF_RESOURCES));
              goto err;
          }
          if (try_to_reconnect(thd, mysql, mi, &retry_count, suppress_warnings,
                               reconnect_messages[SLAVE_RECON_ACT_EVENT]))
            goto err;
          goto connected;
        }  // if (event_len == packet_error)

        relay_io_events++;
        relay_io_bytes += event_len;

        retry_count = 0;  // ok event, reset retry counter
        THD_STAGE_INFO(thd, stage_queueing_master_event_to_the_relay_log);
        event_buf = (const char *)mysql->net.read_pos + 1;
        DBUG_PRINT("info", ("IO thread received event of type %s",
                            Log_event::get_type_str(
                                (Log_event_type)event_buf[EVENT_TYPE_OFFSET])));
        if (RUN_HOOK(binlog_relay_io, after_read_event,
                     (thd, mi, (const char *)mysql->net.read_pos + 1, event_len,
                      &event_buf, &event_len))) {
          mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                     ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                     "Failed to run 'after_read_event' hook");
          goto err;
        }

        /* XXX: 'synced' should be updated by queue_event to indicate
           whether event has been synced to disk */
        bool synced = false;
#ifndef DBUG_OFF
        bool was_in_trx = false;
        if (mi->is_queueing_trx()) {
          was_in_trx = true;
          DBUG_EXECUTE_IF("rpl_ps_tables_queue", {
            rpl_slave_debug_point(DBUG_RPL_S_PS_TABLE_QUEUE);
          };);
        }
#endif
        QUEUE_EVENT_RESULT queue_res = queue_event(mi, event_buf, event_len);
        if (queue_res == QUEUE_EVENT_ERROR_QUEUING) {
          mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
                     ER_THD(thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
                     "could not queue event from master");
          goto err;
        }
#ifndef DBUG_OFF
        if (was_in_trx && !mi->is_queueing_trx()) {
          DBUG_EXECUTE_IF("rpl_ps_tables",
                          { rpl_slave_debug_point(DBUG_RPL_S_PS_TABLES); };);
        }
#endif
        DBUG_EXECUTE_IF("error_before_semi_sync_reply", goto err;);

        if (RUN_HOOK(binlog_relay_io, after_queue_event,
                     (thd, mi, event_buf, event_len, synced))) {
          mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                     ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                     "Failed to run 'after_queue_event' hook");
          goto err;
        }

        /* The event was queued, but there was a failure flushing master info */
        if (queue_res == QUEUE_EVENT_ERROR_FLUSHING_INFO) {
          mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                     ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                     "Failed to flush master info.");
          goto err;
        }

        DBUG_ASSERT(queue_res == QUEUE_EVENT_OK);
        /*
          Pause the IO thread execution and wait for
          'continue_after_queue_event' signal to continue IO thread
          execution.
        */
        DBUG_EXECUTE_IF("pause_after_queue_event", {
          rpl_slave_debug_point(DBUG_RPL_S_PAUSE_AFTER_QUEUE_EV);
        };);

        /*
          See if the relay logs take too much space.
          We don't lock mi->rli->log_space_lock here; this dirty read saves time
          and does not introduce any problem:
          - if mi->rli->ignore_log_space_limit is 1 but becomes 0 just after (so
          the clean value is 0), then we are reading only one more event as we
          should, and we'll block only at the next event. No big deal.
          - if mi->rli->ignore_log_space_limit is 0 but becomes 1 just after (so
          the clean value is 1), then we are going into
          wait_for_relay_log_space() for no reason, but this function will do a
          clean read, notice the clean value and exit immediately.
        */
#ifndef DBUG_OFF
        {
          char llbuf1[22], llbuf2[22];
          DBUG_PRINT("info", ("log_space_limit=%s log_space_total=%s \
ignore_log_space_limit=%d",
                              llstr(rli->log_space_limit, llbuf1),
                              llstr(rli->log_space_total, llbuf2),
                              (int)rli->ignore_log_space_limit));
        }
#endif

        DBUG_EXECUTE_IF("rpl_set_relay_log_limits", {
          rli->log_space_limit = 10;
          rli->log_space_total = 20;
        };);

        if (rli->log_space_limit &&
            rli->log_space_limit < rli->log_space_total &&
            !rli->ignore_log_space_limit)
          if (wait_for_relay_log_space(rli)) {
            LogErr(ERROR_LEVEL,
                   ER_RPL_SLAVE_IO_THREAD_ABORTED_WAITING_FOR_RELAY_LOG_SPACE);
            goto err;
          }
        DBUG_EXECUTE_IF("flush_after_reading_user_var_event", {
          if (event_buf[EVENT_TYPE_OFFSET] == binary_log::USER_VAR_EVENT)
            rpl_slave_debug_point(DBUG_RPL_S_FLUSH_AFTER_USERV_EV);
        });
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_gtid_log_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::GTID_LOG_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_query_log_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::QUERY_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_user_var_log_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::USER_VAR_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_table_map_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::TABLE_MAP_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_xid_log_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::XID_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_write_rows_log_event",
            if (event_buf[EVENT_TYPE_OFFSET] == binary_log::WRITE_ROWS_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF(
            "stop_io_after_reading_unknown_event",
            if (event_buf[EVENT_TYPE_OFFSET] >= binary_log::ENUM_END_EVENT)
                thd->killed = THD::KILLED_NO_VALUE;);
        DBUG_EXECUTE_IF("stop_io_after_queuing_event",
                        thd->killed = THD::KILLED_NO_VALUE;);
        /*
          After event is flushed to relay log file, memory used
          by thread's mem_root is not required any more.
          Hence adding free_root(thd->mem_root,...) to do the
          cleanup, otherwise a long running IO thread can
          cause OOM error.
        */
        free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
      }
    }

    // error = 0;
  err:
    if (slave_stats_daemon_created) {
      // stop sending secondary lag stats to primary
      stop_handle_slave_stats_daemon();
    }
    // print the current replication position
    LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_IO_THREAD_EXITING,
           mi->get_for_channel_str(), mi->get_io_rpl_log_name(),
           llstr(mi->get_master_log_pos(), llbuff));
    /* At this point the I/O thread will not try to reconnect anymore. */
    mi->atomic_is_stopping = true;
    (void)RUN_HOOK(binlog_relay_io, thread_stop, (thd, mi));
    /*
      Pause the IO thread and wait for 'continue_to_stop_io_thread'
      signal to continue to shutdown the IO thread.
    */
    DBUG_EXECUTE_IF("pause_after_io_thread_stop_hook", {
      rpl_slave_debug_point(DBUG_RPL_S_PAUSE_AFTER_IO_STOP, thd);
    };);
    thd->reset_query();
    thd->reset_db(NULL_CSTR);
    if (mysql) {
      /*
        Here we need to clear the active VIO before closing the
        connection with the master.  The reason is that THD::awake()
        might be called from terminate_slave_thread() because somebody
        issued a STOP SLAVE.  If that happends, the shutdown_active_vio()
        can be called in the middle of closing the VIO associated with
        the 'mysql' object, causing a crash.
      */
      thd->clear_active_vio();
      mysql_close(mysql);
      mi->mysql = nullptr;
    }
    write_ignored_events_info_to_relay_log(thd, mi);
    THD_STAGE_INFO(thd, stage_waiting_for_slave_mutex_on_exit);
    mysql_mutex_lock(&mi->run_lock);
    /*
      Clean information used to start slave in order to avoid
      security issues.
    */
    mi->reset_start_info();
    /* Forget the relay log's format */
    mysql_mutex_lock(rli->relay_log.get_log_lock());
    mi->set_mi_description_event(nullptr);
    mysql_mutex_unlock(rli->relay_log.get_log_lock());

    // destructor will not free it, because net.vio is 0
    thd->get_protocol_classic()->end_net();

    thd->release_resources();
    THD_CHECK_SENTRY(thd);
    if (thd_added) thd_manager->remove_thd(thd);

    mi->abort_slave = false;
    mi->slave_running = 0;
    mi->atomic_is_stopping = false;
    mysql_mutex_lock(&mi->info_thd_lock);
    mi->info_thd = nullptr;
    mysql_mutex_unlock(&mi->info_thd_lock);

    /*
      The thd can only be destructed after indirect references
      through mi->info_thd are cleared: mi->info_thd= NULL.

      For instance, user thread might be issuing show_slave_status
      and attempting to read mi->info_thd->get_proc_info().
      Therefore thd must only be deleted after info_thd is set
      to NULL.
    */
    mysql_thread_set_psi_THD(nullptr);
    delete thd;

    /*
      Note: the order of the two following calls (first broadcast, then unlock)
      is important. Otherwise a killer_thread can execute between the calls and
      delete the mi structure leading to a crash! (see BUG#25306 for details)
     */
    mysql_cond_broadcast(&mi->stop_cond);  // tell the world we are done
    DBUG_EXECUTE_IF("simulate_slave_delay_at_terminate_bug38694", sleep(5););
    mysql_mutex_unlock(&mi->run_lock);
  }
  my_thread_end();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  my_thread_exit(nullptr);
  return (nullptr);  // Avoid compiler warnings
}

/*
  Check the temporary directory used by commands like
  LOAD DATA INFILE.
 */
static int check_temp_dir(char *tmp_file, const char *channel_name) {
  int fd;
  MY_DIR *dirp;
  char tmp_dir[FN_REFLEN];
  size_t tmp_dir_size;

  DBUG_TRACE;

  /*
    Get the directory from the temporary file.
  */
  dirname_part(tmp_dir, tmp_file, &tmp_dir_size);

  /*
    Check if the directory exists.
   */
  if (!(dirp = my_dir(tmp_dir, MYF(MY_WME)))) return 1;
  my_dirend(dirp);

  /*
    Check permissions to create a file.
   */
  // append the server UUID to the temp file name.
  constexpr uint size_of_tmp_file_name = 768;
  static_assert(size_of_tmp_file_name >= FN_REFLEN + TEMP_FILE_MAX_LEN, "");
  char *unique_tmp_file_name = (char *)my_malloc(
      key_memory_rpl_slave_check_temp_dir, size_of_tmp_file_name, MYF(0));
  /*
    In the case of Multisource replication, the file create
    sometimes fail because of there is a race that a second SQL
    thread might create the same file and the creation fails.
    TO overcome this, we add a channel name to get a unique file name.
  */

  /* @TODO: dangerous. Prevent this buffer flow */
  snprintf(unique_tmp_file_name, size_of_tmp_file_name, "%s%s%s", tmp_file,
           channel_name, server_uuid);
  if ((fd = mysql_file_create(key_file_misc, unique_tmp_file_name, CREATE_MODE,
                              O_WRONLY | O_EXCL | O_NOFOLLOW, MYF(MY_WME))) < 0)
    return 1;

  /*
    Clean up.
   */
  mysql_file_close(fd, MYF(0));

  mysql_file_delete(key_file_misc, unique_tmp_file_name, MYF(0));
  my_free(unique_tmp_file_name);
  return 0;
}

/*
  Worker thread for the parallel execution of the replication events.
*/
extern "C" {
static void *handle_slave_worker(void *arg) {
  THD *thd; /* needs to be first for thread_stack */
  bool thd_added = false;
  int error = 0;
  Slave_worker *w = (Slave_worker *)arg;
  Relay_log_info *rli = w->c_rli;
  ulong purge_cnt = 0;
  ulonglong purge_size = 0;
  struct slave_job_item _item, *job_item = &_item;
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
#ifdef HAVE_PSI_THREAD_INTERFACE
  struct PSI_thread *psi;
#endif

  my_thread_init();
  DBUG_TRACE;

  thd = new THD(/* enable_plugin = */ true, /* is_slave = */ true);
  if (!thd) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_INITIALIZE_SLAVE_WORKER,
           rli->get_for_channel_str());
    goto err;
  }
  mysql_mutex_lock(&w->info_thd_lock);
  w->info_thd = thd;
  mysql_mutex_unlock(&w->info_thd_lock);
  thd->thread_stack = (char *)&thd;

#ifdef HAVE_PSI_THREAD_INTERFACE
  // save the instrumentation for worker thread in w->info_thd
  psi = PSI_THREAD_CALL(get_thread)();
  thd_set_psi(w->info_thd, psi);
#endif
  mysql_thread_set_psi_id(thd->thread_id());
  mysql_thread_set_psi_THD(thd);

  if (init_slave_thread(thd, SLAVE_THD_WORKER)) {
    // todo make SQL thread killed
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_CANT_INITIALIZE_SLAVE_WORKER,
           rli->get_for_channel_str());
    goto err;
  }
  thd->rli_slave = w;
  thd->init_query_mem_roots();

  if (channel_map.is_group_replication_channel_name(rli->get_channel())) {
    if (channel_map.is_group_replication_channel_name(rli->get_channel(),
                                                      true)) {
      thd->rpl_thd_ctx.set_rpl_channel_type(GR_APPLIER_CHANNEL);
    } else {
      thd->rpl_thd_ctx.set_rpl_channel_type(GR_RECOVERY_CHANNEL);
    }
  } else {
    thd->rpl_thd_ctx.set_rpl_channel_type(RPL_STANDARD_CHANNEL);
  }

  w->set_filter(rli->rpl_filter);

  if ((w->deferred_events_collecting = w->rpl_filter->is_on()))
    w->deferred_events = new Deferred_log_events();
  DBUG_ASSERT(thd->rli_slave->info_thd == thd);

  /* Set applier thread InnoDB priority */
  set_thd_tx_priority(thd, rli->get_thd_tx_priority());

  thd->variables.require_row_format = rli->is_row_format_required();

  if (Relay_log_info::PK_CHECK_STREAM !=
      rli->get_require_table_primary_key_check())
    thd->variables.sql_require_primary_key =
        (rli->get_require_table_primary_key_check() ==
         Relay_log_info::PK_CHECK_ON);
  w->set_require_table_primary_key_check(
      rli->get_require_table_primary_key_check());

  thd_manager->add_thd(thd);
  thd_added = true;

  if (w->update_is_transactional()) {
    rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                "Error checking if the worker repository is transactional.");
    goto err;
  }

  mysql_mutex_lock(&w->jobs_lock);
  w->running_status = Slave_worker::RUNNING;
  mysql_cond_signal(&w->jobs_cond);

  mysql_mutex_unlock(&w->jobs_lock);

  DBUG_ASSERT(thd->is_slave_error == 0);

  w->stats_exec_time = w->stats_read_time = 0;
  set_timespec_nsec(&w->ts_exec[0], 0);
  set_timespec_nsec(&w->ts_exec[1], 0);
  set_timespec_nsec(&w->stats_begin, 0);

  // No need to report anything, all error handling will be performed in the
  // slave SQL thread.
  if (!rli->check_privilege_checks_user())
    rli->initialize_security_context(w->info_thd);  // Worker security context
                                                    // initialization with
                                                    // `PRIVILEGE_CHECKS_USER`

  if (is_mts_parallel_type_dependency(rli)) {
    static_cast<Dependency_slave_worker *>(w)->start();
  } else {
    while (!error) {
      error = slave_worker_exec_job_group(w, rli);
    }
  }
  /*
     Cleanup after an error requires clear_error() go first.
     Otherwise assert(!all) in binlog_rollback()
  */
  thd->clear_error();
  w->cleanup_context(thd, error);

  mysql_mutex_lock(&w->jobs_lock);

  while (de_queue(&w->jobs, job_item)) {
    purge_cnt++;
    purge_size += job_item->data->common_header->data_written;
    DBUG_ASSERT(job_item->data);
    delete job_item->data;
  }

  DBUG_ASSERT(w->jobs.len == 0);

  mysql_mutex_unlock(&w->jobs_lock);

  mysql_mutex_lock(&rli->pending_jobs_lock);
  rli->pending_jobs -= purge_cnt;
  rli->mts_pending_jobs_size -= purge_size;
  DBUG_ASSERT(rli->mts_pending_jobs_size < rli->mts_pending_jobs_size_max);

  mysql_mutex_unlock(&rli->pending_jobs_lock);

  /*
     In MTS case cleanup_after_session() has be called explicitly.
     TODO: to make worker thd be deleted before Slave_worker instance.
  */
  if (thd->rli_slave) {
    w->cleanup_after_session();
    thd->rli_slave = nullptr;
  }
  mysql_mutex_lock(&w->jobs_lock);

  struct timespec stats_end;
  set_timespec_nsec(&stats_end, 0);
  DBUG_PRINT("info",
             ("Worker %lu statistics: "
              "events processed = %lu "
              "online time = %llu "
              "events exec time = %llu "
              "events read time = %llu "
              "hungry waits = %lu "
              "priv queue overfills = %llu ",
              w->id, w->events_done, diff_timespec(&stats_end, &w->stats_begin),
              w->stats_exec_time, w->stats_read_time, w->wq_empty_waits,
              w->jobs.waited_overfill));

  w->running_status = Slave_worker::NOT_RUNNING;
  mysql_cond_signal(&w->jobs_cond);  // famous last goodbye

  mysql_mutex_unlock(&w->jobs_lock);

err:

  if (thd) {
    /*
       The slave code is very bad. Notice that it is missing
       several clean up calls here. I've just added what was
       necessary to avoid valgrind errors.

       /Alfranio
    */
    thd->get_protocol_classic()->end_net();

    /*
      to avoid close_temporary_tables() closing temp tables as those
      are Coordinator's burden.
    */
    thd->system_thread = NON_SYSTEM_THREAD;
    thd->release_resources();

    THD_CHECK_SENTRY(thd);
    if (thd_added) thd_manager->remove_thd(thd);
    mysql_thread_set_psi_THD(nullptr);
    delete thd;
  }

  my_thread_end();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  my_thread_exit(nullptr);
  return nullptr;
}
}  // extern "C"

/**
   Orders jobs by comparing relay log information.
*/

int mts_event_coord_cmp(LOG_POS_COORD *id1, LOG_POS_COORD *id2) {
  longlong filecmp = strcmp(id1->file_name, id2->file_name);
  longlong poscmp = id1->pos - id2->pos;
  return (filecmp < 0
              ? -1
              : (filecmp > 0 ? 1 : (poscmp < 0 ? -1 : (poscmp > 0 ? 1 : 0))));
}

bool mts_recovery_groups(Relay_log_info *rli) {
  Log_event *ev = nullptr;
  bool is_error = false;
  bool flag_group_seen_begin = false;
  uint recovery_group_cnt = 0;
  bool not_reached_commit = true;

  // Value-initialization, to avoid compiler warnings on push_back.
  Slave_job_group job_worker = Slave_job_group();

  LOG_INFO linfo;
  my_off_t offset = 0;
  MY_BITMAP *groups = &rli->recovery_groups;
  THD *thd = current_thd;

  DBUG_TRACE;

  DBUG_ASSERT(rli->slave_parallel_workers == 0);

  /*
     Although mts_recovery_groups() is reentrant it returns
     early if the previous invocation raised any bit in
     recovery_groups bitmap.
  */
  if (rli->is_mts_recovery()) return false;

  /*
    Parallel applier recovery is based on master log name and
    position, on Group Replication we have several masters what
    makes impossible to recover parallel applier from that information.
    Since we always have GTID_MODE=ON on Group Replication, we can
    ignore the positions completely, seek the current relay log to the
    beginning and start from there. Already applied transactions will be
    skipped due to GTIDs auto skip feature and applier will resume from
    the last applied transaction.
  */
  if (channel_map.is_group_replication_channel_name(rli->get_channel(), true)) {
    rli->recovery_parallel_workers = 0;
    rli->mts_recovery_group_cnt = 0;
    rli->set_group_relay_log_pos(BIN_LOG_HEADER_SIZE);
    return false;
  }

  // raft replication always have GTID_MODE=ON, thus ignore positions
  if (enable_raft_plugin) {
    return false;
  }
  /*
    Save relay log position to compare with worker's position.
  */
  LOG_POS_COORD cp = {const_cast<char *>(rli->get_group_master_log_name()),
                      rli->get_group_master_log_pos()};

  /*
    Gathers information on valuable workers and stores it in
    above_lwm_jobs in asc ordered by the master binlog coordinates.
  */
  Prealloced_array<Slave_job_group, 16> above_lwm_jobs(PSI_NOT_INSTRUMENTED);
  above_lwm_jobs.reserve(rli->recovery_parallel_workers);

  /*
    When info tables are used and autocommit= 0 we force a new
    transaction start to avoid table access deadlocks when START SLAVE
    is executed after STOP SLAVE with MTS enabled.
  */
  if (is_autocommit_off_and_infotables(thd))
    if (trans_begin(thd)) goto err;

  for (uint id = 0; id < rli->recovery_parallel_workers; id++) {
    Slave_worker *worker =
        Rpl_info_factory::create_worker(opt_rli_repository_id, id, rli, true);

    if (!worker) {
      if (is_autocommit_off_and_infotables(thd)) trans_rollback(thd);
      goto err;
    }

    LOG_POS_COORD w_last = {
        const_cast<char *>(worker->get_group_master_log_name()),
        worker->get_group_master_log_pos()};
    if (mts_event_coord_cmp(&w_last, &cp) > 0) {
      /*
        Inserts information into a dynamic array for further processing.
        The jobs/workers are ordered by the last checkpoint positions
        workers have seen.
      */
      job_worker.worker = worker;
      job_worker.checkpoint_log_pos = worker->checkpoint_master_log_pos;
      job_worker.checkpoint_log_name = worker->checkpoint_master_log_name;

      above_lwm_jobs.push_back(job_worker);
    } else {
      /*
        Deletes the worker because its jobs are included in the latest
        checkpoint.
      */
      delete worker;
    }
  }

  /*
    When info tables are used and autocommit= 0 we force transaction
    commit to avoid table access deadlocks when START SLAVE is executed
    after STOP SLAVE with MTS enabled.
  */
  if (is_autocommit_off_and_infotables(thd))
    if (trans_commit(thd)) goto err;

  /*
    In what follows, the group Recovery Bitmap is constructed.

     seek(lwm);

     while(w= next(above_lwm_w))
       do
         read G
         if G == w->last_comm
           w.B << group_cnt++;
           RB |= w.B;
            break;
         else
           group_cnt++;
        while(!eof);
        continue;
  */
  DBUG_ASSERT(!rli->recovery_groups_inited);

  if (!above_lwm_jobs.empty()) {
    bitmap_init(groups, nullptr, MTS_MAX_BITS_IN_GROUP);
    rli->recovery_groups_inited = true;
    bitmap_clear_all(groups);
  }
  rli->mts_recovery_group_cnt = 0;
  for (Slave_job_group *jg = above_lwm_jobs.begin(); jg != above_lwm_jobs.end();
       ++jg) {
    Slave_worker *w = jg->worker;
    LOG_POS_COORD w_last = {const_cast<char *>(w->get_group_master_log_name()),
                            w->get_group_master_log_pos()};

    LogErr(DEBUG_LEVEL, ER_RPL_MTS_GROUP_RECOVERY_RELAY_LOG_INFO_FOR_WORKER,
           w->id, w->get_group_relay_log_name(), w->get_group_relay_log_pos(),
           w->get_group_master_log_name(), w->get_group_master_log_pos());

    recovery_group_cnt = 0;
    not_reached_commit = true;
    if (rli->relay_log.find_log_pos(&linfo, rli->get_group_relay_log_name(),
                                    true)) {
      LogErr(ERROR_LEVEL, ER_RPL_ERROR_LOOKING_FOR_LOG,
             rli->get_group_relay_log_name());
      goto err;
    }
    offset = rli->get_group_relay_log_pos();

    Relaylog_file_reader relaylog_file_reader(opt_slave_sql_verify_checksum);

    for (int checking = 0; not_reached_commit; checking++) {
      if (relaylog_file_reader.open(linfo.log_file_name, offset)) {
        LogErr(ERROR_LEVEL, ER_BINLOG_FILE_OPEN_FAILED,
               relaylog_file_reader.get_error_str());
        goto err;
      }

      while (not_reached_commit &&
             (ev = relaylog_file_reader.read_event_object())) {
        DBUG_ASSERT(ev->is_valid());

        if (ev->get_type_code() == binary_log::ROTATE_EVENT ||
            ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT ||
            ev->get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
          delete ev;
          ev = nullptr;
          continue;
        }

        DBUG_PRINT(
            "mts",
            ("Event Recoverying relay log info "
             "group_mster_log_name %s, event_master_log_pos %llu type code %u.",
             linfo.log_file_name, ev->common_header->log_pos,
             ev->get_type_code()));

        if (ev->starts_group()) {
          flag_group_seen_begin = true;
        } else if ((ev->ends_group() || !flag_group_seen_begin) &&
                   !is_gtid_event(ev)) {
          int ret = 0;
          LOG_POS_COORD ev_coord = {
              const_cast<char *>(rli->get_group_master_log_name()),
              ev->common_header->log_pos};
          flag_group_seen_begin = false;
          recovery_group_cnt++;

          LogErr(INFORMATION_LEVEL, ER_RPL_MTS_GROUP_RECOVERY_RELAY_LOG_INFO,
                 rli->get_group_master_log_name(), ev->common_header->log_pos);
          if ((ret = mts_event_coord_cmp(&ev_coord, &w_last)) == 0) {
#ifndef DBUG_OFF
            for (uint i = 0; i <= w->worker_checkpoint_seqno; i++) {
              if (bitmap_is_set(&w->group_executed, i))
                DBUG_PRINT("mts", ("Bit %u is set.", i));
              else
                DBUG_PRINT("mts", ("Bit %u is not set.", i));
            }
#endif
            DBUG_PRINT("mts",
                       ("Doing a shift ini(%lu) end(%lu).",
                        (w->worker_checkpoint_seqno + 1) - recovery_group_cnt,
                        w->worker_checkpoint_seqno));

            for (uint i = (w->worker_checkpoint_seqno + 1) - recovery_group_cnt,
                      j = 0;
                 i <= w->worker_checkpoint_seqno; i++, j++) {
              if (bitmap_is_set(&w->group_executed, i)) {
                DBUG_PRINT("mts", ("Setting bit %u.", j));
                bitmap_test_and_set(groups, j);
              }
            }
            not_reached_commit = false;
          } else
            DBUG_ASSERT(ret < 0);
        }
        delete ev;
        ev = nullptr;
      }

      relaylog_file_reader.close();
      offset = BIN_LOG_HEADER_SIZE;
      if (not_reached_commit && rli->relay_log.find_next_log(&linfo, true)) {
        LogErr(ERROR_LEVEL, ER_RPL_CANT_FIND_FOLLOWUP_FILE,
               linfo.log_file_name);
        goto err;
      }
    }

    rli->mts_recovery_group_cnt =
        (rli->mts_recovery_group_cnt < recovery_group_cnt
             ? recovery_group_cnt
             : rli->mts_recovery_group_cnt);
  }

  DBUG_ASSERT(!rli->recovery_groups_inited ||
              rli->mts_recovery_group_cnt <= groups->n_bits);

  goto end;
err:
  is_error = true;
end:

  for (Slave_job_group *jg = above_lwm_jobs.begin(); jg != above_lwm_jobs.end();
       ++jg) {
    delete jg->worker;
  }

  if (rli->mts_recovery_group_cnt == 0) rli->clear_mts_recovery_groups();

  return is_error;
}

bool mts_checkpoint_routine(Relay_log_info *rli, bool force) {
  ulong cnt;
  bool error = false;
  time_t ts = 0;
  ulonglong ts_millis = 0;

  DBUG_TRACE;

#ifndef DBUG_OFF
  if (DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0)) {
    if (!rli->gaq->count_done(rli)) return false;
  }
  DBUG_EXECUTE_IF("mts_checkpoint", {
    rpl_slave_debug_point(DBUG_RPL_S_MTS_CHECKPOINT_START, rli->info_thd);
  };);
#endif

  /*
    rli->checkpoint_group can have two possible values due to
    two possible status of the last (being scheduled) group.
  */
  DBUG_ASSERT(!rli->gaq->full() ||
              ((rli->rli_checkpoint_seqno == rli->checkpoint_group - 1 &&
                (rli->mts_group_status == Relay_log_info::MTS_IN_GROUP ||
                 rli->mts_group_status == Relay_log_info::MTS_KILLED_GROUP)) ||
               rli->rli_checkpoint_seqno == rli->checkpoint_group));

  do {
    if (is_mts_parallel_type_logical_clock(rli))
      mysql_mutex_lock(&rli->mts_gaq_LOCK);

    cnt = rli->gaq->move_queue_head(&rli->workers);

    if (is_mts_parallel_type_logical_clock(rli))
      mysql_mutex_unlock(&rli->mts_gaq_LOCK);
#ifndef DBUG_OFF
    if (DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0) &&
        cnt != opt_mts_checkpoint_period)
      LogErr(ERROR_LEVEL, ER_RPL_MTS_CHECKPOINT_PERIOD_DIFFERS_FROM_CNT);
#endif
  } while (!sql_slave_killed(rli->info_thd, rli) && cnt == 0 && force &&
           !DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0) &&
           (my_sleep(rli->mts_coordinator_basic_nap), 1));
  /*
    This checks how many consecutive jobs where processed.
    If this value is different than zero the checkpoint
    routine can proceed. Otherwise, there is nothing to be
    done.
  */
  if (cnt == 0) goto end;

  /*
     The workers have completed  cnt jobs from the gaq. This means that we
     should increment C->jobs_done by cnt.
   */
  if (!is_mts_worker(rli->info_thd) &&
      rli->current_mts_submode->get_type() == MTS_PARALLEL_TYPE_LOGICAL_CLOCK) {
    DBUG_PRINT("info", ("jobs_done this itr=%ld", cnt));
    static_cast<Mts_submode_logical_clock *>(rli->current_mts_submode)
        ->jobs_done += cnt;
  }

  // case: rebalance workers should be called only when the current event
  // in the coordinator is a begin or gtid event
  if (!force && opt_mts_dynamic_rebalance && !rli->curr_group_seen_begin &&
      !is_mts_parallel_type_dependency(rli) && !rli->curr_group_seen_gtid &&
      !rli->sql_thread_kill_accepted) {
    rebalance_workers(rli);
  }
  if (!is_mts_parallel_type_dependency(rli)) {
    /* TODO:
       to turn the least occupied selection in terms of jobs pieces
    */
    for (Slave_worker **it = rli->workers.begin(); it != rli->workers.begin();
         ++it) {
      Slave_worker *w_i = *it;
      rli->least_occupied_workers[w_i->id] = w_i->jobs.len;
    };
    std::sort(rli->least_occupied_workers.begin(),
              rli->least_occupied_workers.end());
  }

  if (DBUG_EVALUATE_IF("skip_checkpoint_load_reset", 0, 1)) {
    // reset the database load
    mysql_mutex_lock(&rli->slave_worker_hash_lock);
    for (auto &item : rli->mapping_db_to_worker) {
      db_worker_hash_entry *entry = item.second.get();
      entry->load = 0;
    }
    mysql_mutex_unlock(&rli->slave_worker_hash_lock);
  }

  mysql_mutex_lock(&rli->data_lock);

  /*
    "Coordinator::commit_positions"

    rli->gaq->lwm has been updated in move_queue_head() and
    to contain all but rli->group_master_log_name which
    is altered solely by Coordinator at special checkpoints.
  */
  rli->set_group_master_log_pos(rli->gaq->lwm.group_master_log_pos);
  rli->set_group_relay_log_pos(rli->gaq->lwm.group_relay_log_pos);
  DBUG_PRINT(
      "mts",
      ("New checkpoint %llu %llu %s", rli->gaq->lwm.group_master_log_pos,
       rli->gaq->lwm.group_relay_log_pos, rli->gaq->lwm.group_relay_log_name));

  if (rli->gaq->lwm.group_relay_log_name[0] != 0)
    rli->set_group_relay_log_name(rli->gaq->lwm.group_relay_log_name);

  /*
     todo: uncomment notifies when UNTIL will be supported

     rli->notify_group_master_log_name_update();
     rli->notify_group_relay_log_name_update();

     Todo: optimize with if (wait_flag) broadcast
         waiter: set wait_flag; waits....; drops wait_flag;
  */

  if (get_gtid_mode(GTID_MODE_LOCK_NONE) != GTID_MODE_ON)
    error = rli->flush_info(true);

  mysql_cond_broadcast(&rli->data_cond);
  mysql_mutex_unlock(&rli->data_lock);

  /*
    We need to ensure that this is never called at this point when
    cnt is zero. This value means that the checkpoint information
    will be completely reset.
  */

  /*
    Update the rli->last_master_timestamp for reporting correct
    Seconds_behind_master.

    Note (herman) original comment "If GAQ is empty, set it to zero."
    Noticed that SBM keeps rising despite processing slave transactions
    Changed zero to the timestamp of the lwm similar to the original code.

    Else, update it with the timestamp of the first job of the Slave_job_queue
    which was assigned in the Log_event::get_slave_worker() function.
  */
  if (!rli->gaq->empty()) {
    auto sjg = reinterpret_cast<Slave_job_group *>(rli->gaq->head_queue());
    ts = sjg->ts;
    ts_millis = sjg->ts_millis;
  } else {
    ts = rli->gaq->lwm.ts;
    ts_millis = rli->gaq->lwm.ts_millis;
  }
  rli->reset_notified_checkpoint(cnt, ts, ts_millis, true);
  /* end-of "Coordinator::"commit_positions" */

end:
  error = error || rli->info_thd->killed != THD::NOT_KILLED;
#ifndef DBUG_OFF
  if (DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0)) DBUG_SUICIDE();
  DBUG_EXECUTE_IF("mts_checkpoint", {
    rpl_slave_debug_point(DBUG_RPL_S_MTS_CHECKPOINT_END, rli->info_thd);
  };);
#endif
  set_timespec_nsec(&rli->last_clock, 0);

  return error;
}

/**
   Instantiation of a Slave_worker and forking out a single Worker thread.

   @param  rli  Coordinator's Relay_log_info pointer
   @param  i    identifier of the Worker

   @return 0 suppress or 1 if fails
*/
static int slave_start_single_worker(Relay_log_info *rli, ulong i) {
  int error = 0;
  my_thread_handle th;
  Slave_worker *w = nullptr;

  mysql_mutex_assert_owner(&rli->run_lock);

  if (!(w = Rpl_info_factory::create_worker(opt_rli_repository_id, i, rli,
                                            false))) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_WORKER_THREAD_CREATION_FAILED,
           rli->get_for_channel_str());
    error = 1;
    goto err;
  }

  if (w->init_worker(rli, i)) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_WORKER_THREAD_CREATION_FAILED,
           rli->get_for_channel_str());
    error = 1;
    goto err;
  }

  // We assume that workers are added in sequential order here.
  DBUG_ASSERT(i == rli->workers.size());
  if (i >= rli->workers.size()) rli->workers.resize(i + 1);
  rli->workers[i] = w;

  if (DBUG_EVALUATE_IF("mts_worker_thread_fails", i == 1, 0) ||
      (error =
           mysql_thread_create(key_thread_slave_worker, &th, &connection_attrib,
                               handle_slave_worker, (void *)w))) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_WORKER_THREAD_CREATION_FAILED_WITH_ERRNO,
           rli->get_for_channel_str(), error);
    error = 1;
    goto err;
  }

  mysql_mutex_lock(&w->jobs_lock);
  if (w->running_status == Slave_worker::NOT_RUNNING)
    mysql_cond_wait(&w->jobs_cond, &w->jobs_lock);
  mysql_mutex_unlock(&w->jobs_lock);
  // Least occupied inited with zero
  {
    ulong jobs_len = w->jobs.len;
    rli->least_occupied_workers.push_back(jobs_len);
  }
err:
  if (error && w) {
    // Free the current submode object
    delete w->current_mts_submode;
    w->current_mts_submode = nullptr;
    delete w;
    /*
      Any failure after array inserted must follow with deletion
      of just created item.
    */
    if (rli->workers.size() == i + 1) rli->workers.erase(i);
  }
  return error;
}

/**
   Initialization of the central rli members for Coordinator's role,
   communication channels such as Assigned Partition Hash (APH),
   and starting the Worker pool.

   @param rli             Pointer to Coordinator's Relay_log_info instance.
   @param n               Number of configured Workers in the upcoming session.
   @param[out] mts_inited If the initialization processed was started.

   @return 0         success
           non-zero  as failure
*/
static int slave_start_workers(Relay_log_info *rli, ulong n, bool *mts_inited) {
  int error = 0;
  /**
    gtid_monitoring_info must be cleared when MTS is enabled or
    workers_copy_pfs has elements
  */
  bool clear_gtid_monitoring_info = false;

  mysql_mutex_assert_owner(&rli->run_lock);

  if (n == 0 && rli->mts_recovery_group_cnt == 0) {
    rli->workers.clear();
    rli->clear_processing_trx();
    goto end;
  }

  *mts_inited = true;

  /*
    The requested through argument number of Workers can be different
     from the previous time which ended with an error. Thereby
     the effective number of configured Workers is max of the two.
  */
  rli->init_workers(max(n, rli->recovery_parallel_workers));

  rli->last_assigned_worker = nullptr;  // associated with curr_group_assigned
  // Least_occupied_workers array to hold items size of Slave_jobs_queue::len
  rli->least_occupied_workers.resize(n);

  /*
     GAQ  queue holds seqno:s of scheduled groups. C polls workers in
     @c opt_mts_checkpoint_period to update GAQ (see @c next_event())
     The length of GAQ is set to be equal to checkpoint_group.
     Notice, the size matters for mts_checkpoint_routine's progress loop.
  */

  rli->gaq = new Slave_committed_queue(rli->checkpoint_group, n);
  if (!rli->gaq->inited) return 1;

  // length of WQ is actually constant though can be made configurable
  rli->mts_slave_worker_queue_len_max = mts_slave_worker_queue_len_max;
  rli->mts_pending_jobs_size = 0;
  rli->mts_pending_jobs_size_max = ::opt_mts_pending_jobs_size_max;
  rli->mts_wq_underrun_w_id = MTS_WORKER_UNDEF;
  rli->mts_wq_excess_cnt = 0;
  rli->mts_wq_overrun_cnt = 0;
  rli->mts_wq_oversize = false;
  rli->mts_coordinator_basic_nap = mts_coordinator_basic_nap;
  rli->mts_worker_underrun_level = mts_worker_underrun_level;
  rli->curr_group_seen_begin = rli->curr_group_seen_gtid = false;
  rli->curr_group_isolated = false;
  rli->rli_checkpoint_seqno = 0;
  rli->mts_last_online_stat = my_time(0);
  rli->mts_group_status = Relay_log_info::MTS_NOT_IN_GROUP;
  clear_gtid_monitoring_info = true;

  if (init_hash_workers(rli))  // MTS: mapping_db_to_worker
  {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_FAILED_TO_INIT_PARTITIONS_HASH);
    error = 1;
    goto err;
  }

  for (uint i = 0; i < n; i++) {
    if ((error = slave_start_single_worker(rli, i))) goto err;
    rli->slave_parallel_workers++;
  }

end:
  /*
    Free the buffer that was being used to report worker's status through
    the table performance_schema.table_replication_applier_status_by_worker
    between stop slave and next start slave.
  */
  for (int i = static_cast<int>(rli->workers_copy_pfs.size()) - 1; i >= 0;
       i--) {
    delete rli->workers_copy_pfs[i];
    if (!clear_gtid_monitoring_info) clear_gtid_monitoring_info = true;
  }
  rli->workers_copy_pfs.clear();

  // Effective end of the recovery right now when there is no gaps
  if (!error && rli->mts_recovery_group_cnt == 0) {
    if ((error = rli->mts_finalize_recovery()))
      (void)Rpl_info_factory::reset_workers(rli);
    if (!error) error = rli->flush_info(true);
  }

err:
  if (clear_gtid_monitoring_info) rli->clear_gtid_monitoring_info();
  return error;
}

/*
   Ending Worker threads.

   Not in case Coordinator is killed itself, it first waits for
   Workers have finished their assignements, and then updates checkpoint.
   Workers are notified with setting KILLED status
   and waited for their acknowledgment as specified by
   worker's running_status.
   Coordinator finalizes with its MTS running status to reset few objects.
*/
static void slave_stop_workers(Relay_log_info *rli, bool *mts_inited) {
  THD *thd = rli->info_thd;

  if (!*mts_inited)
    return;
  else if (rli->slave_parallel_workers == 0)
    goto end;

  /*
    If request for stop slave is received notify worker
    to stop.
  */
  // Initialize worker exit count and max_updated_index to 0 during each stop.
  rli->exit_counter = 0;
  rli->max_updated_index = (rli->until_condition != Relay_log_info::UNTIL_NONE)
                               ? rli->mts_groups_assigned
                               : 0;

  if (is_mts_parallel_type_dependency(rli)) {
    static_cast<Mts_submode_dependency *>(rli->current_mts_submode)
        ->stop_dependency_workers(rli);
  } else {
    if (!rli->workers.empty()) {
      for (int i = static_cast<int>(rli->workers.size()) - 1; i >= 0; i--) {
        Slave_worker *w = rli->workers[i];
        struct slave_job_item item = {nullptr, 0, 0};
        struct slave_job_item *job_item = &item;
        mysql_mutex_lock(&w->jobs_lock);

        if (w->running_status != Slave_worker::RUNNING) {
          mysql_mutex_unlock(&w->jobs_lock);
          continue;
        }

        w->running_status = Slave_worker::STOP;
        (void)set_max_updated_index_on_stop(w, job_item);
        mysql_cond_signal(&w->jobs_cond);

        mysql_mutex_unlock(&w->jobs_lock);

        DBUG_PRINT("info", ("Notifying worker %lu%s to exit, thd %p", w->id,
                            w->get_for_channel_str(), w->info_thd));
      }
    }
    thd_proc_info(thd, "Waiting for workers to exit");

    for (Slave_worker **it = rli->workers.begin(); it != rli->workers.end();
         ++it) {
      Slave_worker *w = *it;
      mysql_mutex_lock(&w->jobs_lock);
      while (w->running_status != Slave_worker::NOT_RUNNING) {
        PSI_stage_info old_stage;
        DBUG_ASSERT(w->running_status == Slave_worker::ERROR_LEAVING ||
                    w->running_status == Slave_worker::STOP ||
                    w->running_status == Slave_worker::STOP_ACCEPTED);

        thd->ENTER_COND(&w->jobs_cond, &w->jobs_lock,
                        &stage_slave_waiting_workers_to_exit, &old_stage);
        mysql_cond_wait(&w->jobs_cond, &w->jobs_lock);
        mysql_mutex_unlock(&w->jobs_lock);
        thd->EXIT_COND(&old_stage);
        mysql_mutex_lock(&w->jobs_lock);
      }
      mysql_mutex_unlock(&w->jobs_lock);
    }
  }

  for (Slave_worker **it = rli->workers.begin(); it != rli->workers.end();
       ++it) {
    Slave_worker *w = *it;

    /*
      Make copies for reporting through the performance schema tables.
      This is preserved until the next START SLAVE.
    */
    Slave_worker *worker_copy = new Slave_worker(
        nullptr,
#ifdef HAVE_PSI_INTERFACE
        &key_relay_log_info_run_lock, &key_relay_log_info_data_lock,
        &key_relay_log_info_sleep_lock, &key_relay_log_info_thd_lock,
        &key_relay_log_info_data_cond, &key_relay_log_info_start_cond,
        &key_relay_log_info_stop_cond, &key_relay_log_info_sleep_cond,
#endif
        w->id, rli->get_channel());
    worker_copy->copy_values_for_PFS(w->id, w->running_status, w->info_thd,
                                     w->last_error(),
                                     w->get_gtid_monitoring_info());
    rli->workers_copy_pfs.push_back(worker_copy);
  }

  /// @todo: consider to propagate an error out of the function
  if (thd->killed == THD::NOT_KILLED) (void)mts_checkpoint_routine(rli, false);

  while (!rli->workers.empty()) {
    Slave_worker *w = rli->workers.back();
    // Free the current submode object
    delete w->current_mts_submode;
    w->current_mts_submode = nullptr;
    rli->workers.pop_back();
    delete w;
  }
  struct timespec stats_end;
  set_timespec_nsec(&stats_end, 0);

  DBUG_PRINT(
      "info",
      ("Total MTS session statistics: "
       "events processed = %llu; "
       "online time = %llu "
       "worker queues filled over overrun level = %lu "
       "waited due a Worker queue full = %lu "
       "waited due the total size = %lu "
       "total wait at clock conflicts = %llu "
       "found (count) workers occupied = %lu "
       "waited when workers occupied = %llu",
       rli->mts_events_assigned, diff_timespec(&stats_end, &rli->stats_begin),
       rli->mts_wq_overrun_cnt, rli->mts_wq_overfill_cnt,
       rli->wq_size_waits_cnt, rli->mts_total_wait_overlap.load(),
       rli->mts_wq_no_underrun_cnt, rli->mts_total_wait_worker_avail));

  DBUG_ASSERT(rli->pending_jobs == 0);
  DBUG_ASSERT(rli->mts_pending_jobs_size == 0);

end:
  rli->mts_group_status = Relay_log_info::MTS_NOT_IN_GROUP;
  destroy_hash_workers(rli);
  delete rli->gaq;
  rli->least_occupied_workers.clear();

  // Destroy buffered events of the current group prior to exit.
  for (uint i = 0; i < rli->curr_group_da.size(); i++)
    delete rli->curr_group_da[i].data;
  rli->curr_group_da.clear();  // GCDA

  rli->curr_group_assigned_parts.clear();  // GCAP
  rli->deinit_workers();
  rli->workers_array_initialized = false;
  rli->slave_parallel_workers = 0;

  *mts_inited = false;
}

/**
  Processes the outcome of applying an event, logs it properly if it's an error
  and return the proper error code to trigger.

  @return the error code to bubble up in the execution stack.
 */
static int report_apply_event_error(THD *thd, Relay_log_info *rli) {
  DBUG_TRACE;
  longlong slave_errno = 0;

  /*
    retrieve as much info as possible from the thd and, error
    codes and warnings and print this to the error log as to
    allow the user to locate the error
  */
  uint32 const last_errno = rli->last_error().number;

  if (thd->is_error()) {
    char const *const errmsg = thd->get_stmt_da()->message_text();

    DBUG_PRINT("info", ("thd->get_stmt_da()->get_mysql_errno()=%d; "
                        "rli->last_error.number=%d",
                        thd->get_stmt_da()->mysql_errno(), last_errno));
    if (last_errno == 0) {
      /*
        This function is reporting an error which was not reported
        while executing exec_relay_log_event().
      */
      rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(), "%s", errmsg);
    } else if (last_errno != thd->get_stmt_da()->mysql_errno()) {
      /*
       * An error was reported while executing exec_relay_log_event()
       * however the error code differs from what is in the thread.
       * This function prints out more information to help finding
       * what caused the problem.
       */
      LogErr(ERROR_LEVEL, ER_RPL_SLAVE_ADDITIONAL_ERROR_INFO_FROM_DA, errmsg,
             thd->get_stmt_da()->mysql_errno());
    }
  }

  /* Print any warnings issued */
  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  const Sql_condition *err;
  /*
    Added controlled slave thread cancel for replication
    of user-defined variables.
  */
  bool udf_error = false;
  while ((err = it++)) {
    if (err->mysql_errno() == ER_CANT_OPEN_LIBRARY) udf_error = true;
    LogErr(WARNING_LEVEL, ER_RPL_SLAVE_ERROR_INFO_FROM_DA, err->message_text(),
           err->mysql_errno());
  }
  if (udf_error)
    slave_errno = ER_RPL_SLAVE_ERROR_LOADING_USER_DEFINED_LIBRARY;
  else
    slave_errno = ER_RPL_SLAVE_ERROR_RUNNING_QUERY;

  return slave_errno;
}

/**
  Slave SQL thread entry point.

  @param arg Pointer to Relay_log_info object that holds information
  for the SQL thread.

  @return Always 0.
*/
extern "C" void *handle_slave_sql(void *arg) {
  THD *thd; /* needs to be first for thread_stack */
  bool thd_added = false;
  bool main_loop_error = false;
  char llbuff[22], llbuff1[22];
  char saved_log_name[FN_REFLEN];
  char saved_master_log_name[FN_REFLEN];
  my_off_t saved_log_pos = 0;
  my_off_t saved_master_log_pos = 0;
  my_off_t saved_skip = 0;

  Relay_log_info *rli = ((Master_info *)arg)->rli;
  const char *errmsg;
  longlong slave_errno = 0;
  bool mts_inited = false;
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  Commit_order_manager *commit_order_mngr = nullptr;
  auto applier_reader = std::make_shared<Rpl_applier_reader>(rli);
  global_applier_reader = applier_reader;
  Relay_log_info::enum_priv_checks_status priv_check_status =
      Relay_log_info::enum_priv_checks_status::SUCCESS;

  // needs to call my_thread_init(), otherwise we get a coredump in DBUG_ stuff
  my_thread_init();
  {
    DBUG_TRACE;

    DBUG_ASSERT(rli->inited);
    mysql_mutex_lock(&rli->run_lock);
    DBUG_ASSERT(!rli->slave_running);
    errmsg = nullptr;
#ifndef DBUG_OFF
    rli->events_until_exit = abort_slave_event_count;
#endif

    // note that contructor of THD uses DBUG_ !
    thd = new THD(/* enable_plugin = */ true, /* is_slave = */ true);
    thd->thread_stack = (char *)&thd;  // remember where our stack is
    mysql_mutex_lock(&rli->info_thd_lock);
    rli->info_thd = thd;

#ifdef HAVE_PSI_THREAD_INTERFACE
    // save the instrumentation for SQL thread in rli->info_thd
    struct PSI_thread *psi = PSI_THREAD_CALL(get_thread)();
    thd_set_psi(rli->info_thd, psi);
#endif
    mysql_thread_set_psi_id(thd->thread_id());
    mysql_thread_set_psi_THD(thd);

    if (rli->channel_mts_submode == MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
      rli->current_mts_submode = new Mts_submode_logical_clock();
    else if (rli->channel_mts_submode == MTS_PARALLEL_TYPE_DB_NAME)
      rli->current_mts_submode = new Mts_submode_database();
    else
      rli->current_mts_submode = new Mts_submode_dependency();

    rli->last_opid = std::make_pair(-1, -1);

    const auto slave_preserve_commit_order = get_slave_preserve_commit_order();
    if (slave_preserve_commit_order && !rli->is_parallel_exec())
      commit_order_mngr =
          new Commit_order_manager(rli->opt_slave_parallel_workers);

    rli->set_commit_order_manager(commit_order_mngr);

    if (channel_map.is_group_replication_channel_name(rli->get_channel())) {
      if (channel_map.is_group_replication_channel_name(rli->get_channel(),
                                                        true)) {
        thd->rpl_thd_ctx.set_rpl_channel_type(GR_APPLIER_CHANNEL);
      } else {
        thd->rpl_thd_ctx.set_rpl_channel_type(GR_RECOVERY_CHANNEL);
      }
    } else {
      thd->rpl_thd_ctx.set_rpl_channel_type(RPL_STANDARD_CHANNEL);
    }

    mysql_mutex_unlock(&rli->info_thd_lock);

    rli->mts_dependency_replication = opt_mts_dependency_replication;
    rli->mts_dependency_size = opt_mts_dependency_size;
    rli->mts_dependency_refill_threshold = opt_mts_dependency_refill_threshold;
    rli->mts_dependency_max_keys = opt_mts_dependency_max_keys;
    rli->slave_preserve_commit_order = slave_preserve_commit_order;
    rli->mts_dependency_order_commits = opt_mts_dependency_order_commits;
    rli->mts_dependency_cond_wait_timeout =
        opt_mts_dependency_cond_wait_timeout;

    if (is_mts_parallel_type_dependency(rli) &&
        !slave_use_idempotent_for_recovery_options) {
      sql_print_error(
          "mts_dependency_replication is enabled but "
          "slave_use_idempotent_for_recovery is disabled. The slave is not "
          "crash "
          "safe! Please enable slave_use_idempotent_for_recovery for crash "
          "safety.");
    }
    /* Inform waiting threads that slave has started */
    rli->slave_run_id++;
    rli->slave_running = 1;
    rli->reported_unsafe_warning = false;
    rli->sql_thread_kill_accepted = false;

    if (init_slave_thread(thd, SLAVE_THD_SQL)) {
      /*
        TODO: this is currently broken - slave start and change master
        will be stuck if we fail here
      */
      mysql_cond_broadcast(&rli->start_cond);
      mysql_mutex_unlock(&rli->run_lock);
      rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                  ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                  "Failed during slave thread initialization");
      goto err;
    }
    thd->init_query_mem_roots();

    if (opt_rbr_column_type_mismatch_whitelist) {
      const auto &list =
          split_into_set(opt_rbr_column_type_mismatch_whitelist, ',');
      rli->set_rbr_column_type_mismatch_whitelist(list);
    } else
      rli->set_rbr_column_type_mismatch_whitelist(
          std::unordered_set<std::string>());

    if ((rli->deferred_events_collecting = rli->rpl_filter->is_on()))
      rli->deferred_events = new Deferred_log_events();
    thd->rli_slave = rli;
    DBUG_ASSERT(thd->rli_slave->info_thd == thd);

    thd->temporary_tables = rli->save_temporary_tables;  // restore temp tables
    set_thd_in_use_temporary_tables(
        rli);  // (re)set sql_thd in use for saved temp tables
    /* Set applier thread InnoDB priority */
    set_thd_tx_priority(thd, rli->get_thd_tx_priority());
    thd->variables.require_row_format = rli->is_row_format_required();

    if (Relay_log_info::PK_CHECK_STREAM !=
        rli->get_require_table_primary_key_check())
      thd->variables.sql_require_primary_key =
          (rli->get_require_table_primary_key_check() ==
           Relay_log_info::PK_CHECK_ON);

    rli->transaction_parser.reset();

    thd_manager->add_thd(thd);
    thd_added = true;

    rli->stats_exec_time = rli->stats_read_time = 0;
    set_timespec_nsec(&rli->ts_exec[0], 0);
    set_timespec_nsec(&rli->ts_exec[1], 0);
    set_timespec_nsec(&rli->stats_begin, 0);

    if (RUN_HOOK(binlog_relay_io, applier_start, (thd, rli->mi))) {
      mysql_cond_broadcast(&rli->start_cond);
      mysql_mutex_unlock(&rli->run_lock);
      rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                  ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                  "Failed to run 'applier_start' hook");
      goto err;
    }

    /* MTS: starting the worker pool */
    if (slave_start_workers(rli, rli->opt_slave_parallel_workers,
                            &mts_inited) != 0) {
      mysql_cond_broadcast(&rli->start_cond);
      mysql_mutex_unlock(&rli->run_lock);
      rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                  ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                  "Failed during slave workers initialization");
      goto err;
    }
    /*
      We are going to set slave_running to 1. Assuming slave I/O thread is
      alive and connected, this is going to make Seconds_Behind_Master be 0
      i.e. "caught up". Even if we're just at start of thread. Well it's ok, at
      the moment we start we can think we are caught up, and the next second we
      start receiving data so we realize we are not caught up and
      Seconds_Behind_Master grows. No big deal.
    */
    rli->abort_slave = false;

    /*
      Reset errors for a clean start (otherwise, if the master is idle, the SQL
      thread may execute no Query_log_event, so the error will remain even
      though there's no problem anymore). Do not reset the master timestamp
      (imagine the slave has caught everything, the STOP SLAVE and START SLAVE:
      as we are not sure that we are going to receive a query, we want to
      remember the last master timestamp (to say how many seconds behind we are
      now.
      But the master timestamp is reset by RESET SLAVE & CHANGE MASTER.
    */
    rli->clear_error();
    if (rli->workers_array_initialized) {
      for (size_t i = 0; i < rli->get_worker_count(); i++) {
        rli->get_worker(i)->clear_error();
      }
    }

    if (rli->update_is_transactional()) {
      mysql_cond_broadcast(&rli->start_cond);
      mysql_mutex_unlock(&rli->run_lock);
      rli->report(
          ERROR_LEVEL, ER_SLAVE_FATAL_ERROR, ER_THD(thd, ER_SLAVE_FATAL_ERROR),
          "Error checking if the relay log repository is transactional.");
      goto err;
    }

    if (!rli->is_transactional())
      rli->report(
          WARNING_LEVEL, 0,
          "If a crash happens this configuration does not guarantee that "
          "the relay "
          "log info will be consistent");

    mysql_cond_broadcast(&rli->start_cond);
    mysql_mutex_unlock(&rli->run_lock);

    DEBUG_SYNC(thd, "after_start_slave");

    // tell the I/O thread to take relay_log_space_limit into account from now
    // on
    mysql_mutex_lock(&rli->log_space_lock);
    rli->ignore_log_space_limit = false;
    mysql_mutex_unlock(&rli->log_space_lock);
    rli->trans_retries = 0;  // start from "no error"
    DBUG_PRINT("info", ("rli->trans_retries: %lu", rli->trans_retries));

    if (applier_reader->open(&errmsg)) {
      rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR, "%s", errmsg);
      goto err;
    }

    THD_CHECK_SENTRY(thd);
    DBUG_ASSERT(rli->info_thd == thd);

    DBUG_PRINT("master_info", ("log_file_name: %s  position: %s",
                               rli->get_group_master_log_name(),
                               llstr(rli->get_group_master_log_pos(), llbuff)));

    if (check_temp_dir(rli->slave_patternload_file, rli->get_channel())) {
      rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                  "Unable to use slave's temporary directory %s - %s",
                  slave_load_tmpdir, thd->get_stmt_da()->message_text());
      goto err;
    }

    priv_check_status = rli->check_privilege_checks_user();
    if (!!priv_check_status) {
      rli->report_privilege_check_error(ERROR_LEVEL, priv_check_status,
                                        false /* to client*/);
      rli->set_privilege_checks_user_corrupted(true);
      goto err;
    }
    priv_check_status =
        rli->initialize_applier_security_context();  // Applier security context
                                                     // initialization with
                                                     // `PRIVILEGE_CHECKS_USER`
    if (!!priv_check_status) {
      rli->report_privilege_check_error(ERROR_LEVEL, priv_check_status,
                                        false /* to client*/);
      goto err;
    }

    if (rli->is_privilege_checks_user_null())
      LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_SQL_THREAD_STARTING,
             rli->get_for_channel_str(), rli->get_rpl_log_name(),
             llstr(rli->get_group_master_log_pos(), llbuff),
             rli->get_group_relay_log_name(),
             llstr(rli->get_group_relay_log_pos(), llbuff1));
    else
      LogErr(INFORMATION_LEVEL,
             ER_RPL_SLAVE_SQL_THREAD_STARTING_WITH_PRIVILEGE_CHECKS,
             rli->get_for_channel_str(), rli->get_rpl_log_name(),
             llstr(rli->get_group_master_log_pos(), llbuff),
             rli->get_group_relay_log_name(),
             llstr(rli->get_group_relay_log_pos(), llbuff1),
             rli->get_privilege_checks_username().c_str(),
             rli->get_privilege_checks_hostname().c_str(),
             opt_always_activate_granted_roles == 0 ? "DEFAULT" : "ALL");

    /* execute init_slave variable */
    if (opt_init_slave.length) {
      execute_init_command(thd, &opt_init_slave, &LOCK_sys_init_slave);
      if (thd->is_slave_error) {
        rli->report(ERROR_LEVEL, ER_SERVER_SLAVE_INIT_QUERY_FAILED,
                    ER_THD(current_thd, ER_SERVER_SLAVE_INIT_QUERY_FAILED),
                    thd->get_stmt_da()->mysql_errno(),
                    thd->get_stmt_da()->message_text());
        goto err;
      }
    }

    /*
      First check until condition - probably there is nothing to execute. We
      do not want to wait for next event in this case.
    */
    mysql_mutex_lock(&rli->data_lock);
    if (rli->slave_skip_counter) {
      strmake(saved_log_name, rli->get_group_relay_log_name(), FN_REFLEN - 1);
      strmake(saved_master_log_name, rli->get_group_master_log_name(),
              FN_REFLEN - 1);
      saved_log_pos = rli->get_group_relay_log_pos();
      saved_master_log_pos = rli->get_group_master_log_pos();
      saved_skip = rli->slave_skip_counter;
    }
    if (rli->is_until_satisfied_at_start_slave()) {
      mysql_mutex_unlock(&rli->data_lock);
      goto err;
    }
    mysql_mutex_unlock(&rli->data_lock);

    /* Read queries from the IO/THREAD until this thread is killed */

    while (!main_loop_error && !sql_slave_killed(thd, rli)) {
      Log_event *ev = nullptr;
      THD_STAGE_INFO(thd, stage_reading_event_from_the_relay_log);
      DBUG_ASSERT(rli->info_thd == thd);
      THD_CHECK_SENTRY(thd);
      if (saved_skip && rli->slave_skip_counter == 0) {
        LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_SKIP_COUNTER_EXECUTED,
               (ulong)saved_skip, saved_log_name, (ulong)saved_log_pos,
               saved_master_log_name, (ulong)saved_master_log_pos,
               rli->get_group_relay_log_name(),
               (ulong)rli->get_group_relay_log_pos(),
               rli->get_group_master_log_name(),
               (ulong)rli->get_group_master_log_pos());
        saved_skip = 0;
      }

      // read next event
      mysql_mutex_lock(&rli->data_lock);
      ev = applier_reader->read_next_event();
      mysql_mutex_unlock(&rli->data_lock);

      // set additional context as needed by the scheduler before execution
      // takes place
      if (ev != nullptr && rli->is_parallel_exec() &&
          rli->current_mts_submode != nullptr)
        rli->current_mts_submode->set_multi_threaded_applier_context(*rli, *ev);

      // try to execute the event
      switch (exec_relay_log_event(thd, rli, applier_reader.get(), ev)) {
        case SLAVE_APPLY_EVENT_AND_UPDATE_POS_OK:
          /** success, we read the next event. */
          /** fall through */
        case SLAVE_APPLY_EVENT_UNTIL_REACHED:
          /** this will make the main loop abort in the next iteration */
          /** fall through */
        case SLAVE_APPLY_EVENT_RETRY:
          /** single threaded applier has to retry.
              Next iteration reads the same event. */
          break;

        case SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPLY_ERROR:
          /** fall through */
        case SLAVE_APPLY_EVENT_AND_UPDATE_POS_UPDATE_POS_ERROR:
          /** fall through */
        case SLAVE_APPLY_EVENT_AND_UPDATE_POS_APPEND_JOB_ERROR:
          main_loop_error = true;
          break;

        default:
          /* This shall never happen. */
          DBUG_ASSERT(0); /* purecov: inspected */
          break;
      }
    }
  err:

    // report error
    if (main_loop_error == true && !sql_slave_killed(thd, rli))
      slave_errno = report_apply_event_error(thd, rli);

    /* At this point the SQL thread will not try to work anymore. */
    rli->atomic_is_stopping = true;
    (void)RUN_HOOK(
        binlog_relay_io, applier_stop,
        (thd, rli->mi, rli->is_error() || !rli->sql_thread_kill_accepted));

    slave_stop_workers(rli, &mts_inited);  // stopping worker pool
    /* Thread stopped. Print the current replication position to the log */
    if (slave_errno)
      LogErr(ERROR_LEVEL, slave_errno, rli->get_rpl_log_name(),
             llstr(rli->get_group_master_log_pos(), llbuff));
    else
      LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_SQL_THREAD_EXITING,
             rli->get_for_channel_str(), rli->get_rpl_log_name(),
             llstr(rli->get_group_master_log_pos(), llbuff));

    delete rli->current_mts_submode;
    rli->current_mts_submode = nullptr;
    rli->clear_mts_recovery_groups();

    /*
      Some events set some playgrounds, which won't be cleared because thread
      stops. Stopping of this thread may not be known to these events ("stop"
      request is detected only by the present function, not by events), so we
      must "proactively" clear playgrounds:
    */
    thd->clear_error();
    rli->cleanup_context(thd, true);
    /*
      Some extra safety, which should not been needed (normally, event deletion
      should already have done these assignments (each event which sets these
      variables is supposed to set them to 0 before terminating)).
    */
    thd->set_catalog(NULL_CSTR);
    thd->reset_query();
    thd->reset_db(NULL_CSTR);

    /*
      Pause the SQL thread and wait for 'continue_to_stop_sql_thread'
      signal to continue to shutdown the SQL thread.
    */
    DBUG_EXECUTE_IF("pause_after_sql_thread_stop_hook", {
      rpl_slave_debug_point(DBUG_RPL_S_AFTER_SQL_STOP, thd);
    };);

    THD_STAGE_INFO(thd, stage_waiting_for_slave_mutex_on_exit);
    mysql_mutex_lock(&rli->run_lock);
    /* We need data_lock, at least to wake up any waiting master_pos_wait() */
    mysql_mutex_lock(&rli->data_lock);
    applier_reader->close();
    DBUG_ASSERT(rli->slave_running == 1);  // tracking buffer overrun
    /* When master_pos_wait() wakes up it will check this and terminate */
    rli->slave_running = 0;
    rli->atomic_is_stopping = false;
    /* Forget the relay log's format */
    if (rli->set_rli_description_event(nullptr)) {
#ifndef DBUG_OFF
      bool set_rli_description_event_failed = false;
#endif
      DBUG_ASSERT(set_rli_description_event_failed);
    }
    /* Wake up master_pos_wait() */
    DBUG_PRINT("info",
               ("Signaling possibly waiting master_pos_wait() functions"));
    mysql_cond_broadcast(&rli->data_cond);
    mysql_mutex_unlock(&rli->data_lock);
    rli->ignore_log_space_limit = false; /* don't need any lock */
    rli->sql_force_rotate_relay = false;
    /* we die so won't remember charset - re-update them on next thread start */
    rli->cached_charset_invalidate();
    rli->save_temporary_tables = thd->temporary_tables;

    /*
      TODO: see if we can do this conditionally in next_event() instead
      to avoid unneeded position re-init
    */
    thd->temporary_tables =
        nullptr;  // remove tempation from destructor to close them
    // destructor will not free it, because we are weird
    thd->get_protocol_classic()->end_net();
    DBUG_ASSERT(rli->info_thd == thd);
    THD_CHECK_SENTRY(thd);
    mysql_mutex_lock(&rli->info_thd_lock);
    rli->info_thd = nullptr;
    if (commit_order_mngr) {
      delete commit_order_mngr;
      rli->set_commit_order_manager(nullptr);
    }

    mysql_mutex_unlock(&rli->info_thd_lock);
    set_thd_in_use_temporary_tables(
        rli);  // (re)set info_thd in use for saved temp tables

    thd->release_resources();
    THD_CHECK_SENTRY(thd);
    if (thd_added) thd_manager->remove_thd(thd);

    /*
      The thd can only be destructed after indirect references
      through mi->rli->info_thd are cleared: mi->rli->info_thd= NULL.

      For instance, user thread might be issuing show_slave_status
      and attempting to read mi->rli->info_thd->get_proc_info().
      Therefore thd must only be deleted after info_thd is set
      to NULL.
    */
    mysql_thread_set_psi_THD(nullptr);
    delete thd;

    /*
     Note: the order of the broadcast and unlock calls below (first broadcast,
     then unlock) is important. Otherwise a killer_thread can execute between
     the calls and delete the mi structure leading to a crash! (see BUG#25306
     for details)
    */
    mysql_cond_broadcast(&rli->stop_cond);
    DBUG_EXECUTE_IF("simulate_slave_delay_at_terminate_bug38694", sleep(5););
    mysql_mutex_unlock(&rli->run_lock);  // tell the world we are done
  }
  my_thread_end();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  ERR_remove_thread_state(0);
#endif /* OPENSSL_VERSION_NUMBER < 0x10100000L */
  my_thread_exit(nullptr);
  return nullptr;  // Avoid compiler warnings
}

/**
  Used by the slave IO thread when it receives a rotate event from the
  master.

  Updates the master info with the place in the next binary log where
  we should start reading.  Rotate the relay log to avoid mixed-format
  relay logs.

  @param mi master_info for the slave
  @param rev The rotate log event read from the master

  @note The caller must hold mi->data_lock before invoking this function.

  @retval 0 ok
  @retval 1 error
*/
static int process_io_rotate(Master_info *mi, Rotate_log_event *rev) {
  DBUG_TRACE;
  mysql_mutex_assert_owner(mi->rli->relay_log.get_log_lock());

  if (unlikely(!rev->is_valid())) return 1;

#ifndef DBUG_OFF
  /*
    If we do not do this, we will be getting the first
    rotate event forever, so we need to not disconnect after one.
  */
  if (disconnect_slave_event_count) mi->events_until_exit++;
#endif

  /*
    Master will send a FD event immediately after the Roate event, so don't log
    the current FD event.
  */
  int ret = rotate_relay_log(mi, false, false, true);

  mysql_mutex_lock(&mi->data_lock);
  /* Safe copy as 'rev' has been "sanitized" in Rotate_log_event's ctor */
  memcpy(const_cast<char *>(mi->get_master_log_name()), rev->new_log_ident,
         rev->ident_len + 1);
  mi->set_master_log_pos(rev->pos);
  DBUG_PRINT("info",
             ("new (master_log_name, master_log_pos): ('%s', %lu)",
              mi->get_master_log_name(), (ulong)mi->get_master_log_pos()));
  mysql_mutex_unlock(&mi->data_lock);

  return ret;
}

/**
 * Mark this GTID as logged in the rli and set the master_log_file_name and
 * master_log_file_pos in mi. Finally,  flushes the master.info file
 *
 * @retval 0 Success
 * @retval 1 Some failure
 */
int update_rli_and_mi(
    const std::string &gtid_s,
    const std::pair<const std::string, unsigned long long> &master_log_pos) {
  channel_map.rdlock();
  DBUG_ASSERT(channel_map.get_num_instances() == 1);
  Master_info *mi = channel_map.get_default_channel_mi();
  DBUG_ASSERT(mi != nullptr);
  Relay_log_info *rli = mi->rli;
  DBUG_ASSERT(rli != nullptr);

  mysql_mutex_lock(&mi->data_lock);
  // Update the master log file name in mi, if provided
  if (!master_log_pos.first.empty()) {
    mi->set_master_log_name(master_log_pos.first.c_str());
  }
  // Update the master log file pos in mi
  mi->set_master_log_pos(master_log_pos.second);
  // Flush the master.info file
  mi->flush_info(/*force*/ false);
  // init description event
  if (mi->get_mi_description_event() == nullptr) {
    auto fdle = new Format_description_log_event();
    fdle->common_footer->checksum_alg =
        mi->rli->relay_log.relay_log_checksum_alg;
    mysql_mutex_lock(rli->relay_log.get_log_lock());
    mi->set_mi_description_event(fdle);
    mysql_mutex_unlock(rli->relay_log.get_log_lock());
  }
  mysql_mutex_unlock(&mi->data_lock);

  // It is possible that this call was only done to update the master_log_pos
  // in which case an empty gtid would have been passed
  if (!gtid_s.length()) {
    channel_map.unlock();
    return 0;
  }

  mysql_mutex_t *log_lock = mi->rli->relay_log.get_log_lock();
  mysql_mutex_assert_not_owner(log_lock);
  mysql_mutex_lock(log_lock);
  const char *buf = gtid_s.c_str();
  Gtid_log_event gtid_ev(buf, mi->get_mi_description_event());
  mysql_mutex_unlock(log_lock);

  Gtid gtid = {0, 0};
  rli->get_sid_lock()->rdlock();
  gtid.sidno = gtid_ev.get_sidno(rli->get_gtid_set()->get_sid_map());
  rli->get_sid_lock()->unlock();
  if (gtid.sidno < 0) {
    sql_print_information("could not get proper sid: %s", buf);
    channel_map.unlock();
    return 1;
  }

  gtid.gno = gtid_ev.get_gno();
  DBUG_PRINT("info", ("update_rli_and_mi: Found Gtid : Gtid(%d, %lld).",
                      gtid.sidno, gtid.gno));

  rli->get_sid_lock()->rdlock();
  rli->add_logged_gtid(gtid.sidno, gtid.gno);
  rli->get_sid_lock()->unlock();
  channel_map.unlock();

  return 0;
}

/**
  Store an event received from the master connection into the relay
  log.

  @param mi The Master_info object representing this connection.
  @param buf Pointer to the event data.
  @param event_len Length of event data.
  @param do_flush_mi True to flush master info after successfully queuing the
                     event.

  @retval QUEUE_EVENT_OK                  on success.
  @retval QUEUE_EVENT_ERROR_QUEUING       if there was an error while queuing.
  @retval QUEUE_EVENT_ERROR_FLUSHING_INFO if there was an error while
                                          flushing master info.

  @todo Make this a member of Master_info.
*/
QUEUE_EVENT_RESULT queue_event(Master_info *mi, const char *buf,
                               ulong event_len, bool do_flush_mi) {
  QUEUE_EVENT_RESULT res = QUEUE_EVENT_OK;
  ulong inc_pos = 0;
  Relay_log_info *rli = mi->rli;
  mysql_mutex_t *log_lock = rli->relay_log.get_log_lock();
  ulong s_id;
  int lock_count = 0;

  DBUG_EXECUTE_IF("wait_in_the_middle_of_trx", {
    /*
      See `gr_flush_relay_log_no_split_trx.test`
      1) Add a debug sync point that holds and makes the applier thread to
         wait, in the middle of a transaction -
         `signal.rpl_requested_for_a_flush`.
    */
    DBUG_SET("-d,wait_in_the_middle_of_trx");
    const char dbug_wait[] = "now WAIT_FOR signal.rpl_requested_for_a_flush";
    DBUG_ASSERT(
        !debug_sync_set_action(current_thd, STRING_WITH_LEN(dbug_wait)));
  });

  /*
    FD_q must have been prepared for the first R_a event
    inside get_master_version_and_clock()
    Show-up of FD:s affects checksum_alg at once because
    that changes FD_queue.
  */
  enum_binlog_checksum_alg checksum_alg =
      mi->checksum_alg_before_fd != binary_log::BINLOG_CHECKSUM_ALG_UNDEF
          ? mi->checksum_alg_before_fd
          : mi->rli->relay_log.relay_log_checksum_alg;

  const char *save_buf =
      nullptr;  // needed for checksumming the fake Rotate event
  char rot_buf[LOG_EVENT_HEADER_LEN + Binary_log_event::ROTATE_HEADER_LEN +
               FN_REFLEN];
  Gtid gtid = {0, 0};
  ulonglong immediate_commit_timestamp = 0;
  ulonglong original_commit_timestamp = 0;
  bool info_error{false};
  binary_log::Log_event_basic_info log_event_info;
  ulonglong compressed_transaction_bytes = 0;
  ulonglong uncompressed_transaction_bytes = 0;
  auto compression_type = binary_log::transaction::compression::type::NONE;
  Log_event_type event_type = (Log_event_type)buf[EVENT_TYPE_OFFSET];

  DBUG_ASSERT(checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_OFF ||
              checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_UNDEF ||
              checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_CRC32);

  DBUG_TRACE;

  /*
    Pause the IO thread execution and wait for 'continue_queuing_event'
    signal to continue IO thread execution.
  */
  DBUG_EXECUTE_IF("pause_on_queuing_event",
                  { rpl_slave_debug_point(DBUG_RPL_S_PAUSE_QUEUING); };);

  /*
    FD_queue checksum alg description does not apply in a case of
    FD itself. The one carries both parts of the checksum data.
  */
  if (event_type == binary_log::FORMAT_DESCRIPTION_EVENT) {
    checksum_alg = Log_event_footer::get_checksum_alg(buf, event_len);
  }

  // does not hold always because of old binlog can work with NM
  // DBUG_ASSERT(checksum_alg != BINLOG_CHECKSUM_ALG_UNDEF);

  // should hold unless manipulations with RL. Tests that do that
  // will have to refine the clause.
  DBUG_ASSERT(mi->rli->relay_log.relay_log_checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_UNDEF);

  // Emulate the network corruption
  DBUG_EXECUTE_IF(
      "corrupt_queue_event",
      if (event_type != binary_log::FORMAT_DESCRIPTION_EVENT) {
        char *debug_event_buf_c = const_cast<char *>(buf);
        int debug_cor_pos = rand() % (event_len - BINLOG_CHECKSUM_LEN);
        debug_event_buf_c[debug_cor_pos] = ~debug_event_buf_c[debug_cor_pos];
        DBUG_PRINT("info",
                   ("Corrupt the event at queue_event: byte on position %d",
                    debug_cor_pos));
        DBUG_SET("");
      });
  binary_log_debug::debug_checksum_test =
      DBUG_EVALUATE_IF("simulate_checksum_test_failure", true, false);
  binary_log_debug::debug_checksum_test =
      DBUG_EVALUATE_IF("gr_simulate_checksum_test_failure", true,
                       binary_log_debug::debug_checksum_test);
  if (Log_event_footer::event_checksum_test(
          const_cast<uchar *>(pointer_cast<const uchar *>(buf)), event_len,
          checksum_alg)) {
    mi->report(ERROR_LEVEL, ER_NETWORK_READ_EVENT_CHECKSUM_FAILURE, "%s",
               ER_THD(current_thd, ER_NETWORK_READ_EVENT_CHECKSUM_FAILURE));
    goto err;
  }

  /*
    From now, and up to finishing queuing the event, no other thread is allowed
    to write to the relay log, or to rotate it.
  */
  mysql_mutex_lock(log_lock);
  DBUG_ASSERT(lock_count == 0);
  lock_count = 1;

  if (mi->get_mi_description_event() == nullptr) {
    LogErr(ERROR_LEVEL, ER_RPL_SLAVE_QUEUE_EVENT_FAILED_INVALID_CONFIGURATION,
           mi->get_channel());
    goto err;
  }

  /*
    Simulate an unknown ignorable log event by rewriting a Xid
    log event before queuing it into relay log.
  */
  DBUG_EXECUTE_IF(
      "simulate_unknown_ignorable_log_event_with_xid",
      if (event_type == binary_log::XID_EVENT) {
        uchar *ev_buf = const_cast<uchar *>(pointer_cast<const uchar *>(buf));
        /* Overwrite the log event type with an unknown type. */
        ev_buf[EVENT_TYPE_OFFSET] = binary_log::ENUM_END_EVENT + 1;
        /* Set LOG_EVENT_IGNORABLE_F for the log event. */
        int2store(ev_buf + FLAGS_OFFSET,
                  uint2korr(ev_buf + FLAGS_OFFSET) | LOG_EVENT_IGNORABLE_F);
        /* Recalc event's CRC */
        ha_checksum ev_crc = checksum_crc32(0L, nullptr, 0);
        ev_crc = checksum_crc32(ev_crc, (const uchar *)ev_buf,
                                event_len - BINLOG_CHECKSUM_LEN);
        int4store(&ev_buf[event_len - BINLOG_CHECKSUM_LEN], ev_crc);
        /*
          We will skip writing this event to the relay log in order to let
          the startup procedure to not finding it and assuming this transaction
          is incomplete.
          But we have to keep the unknown ignorable error to let the
          "stop_io_after_reading_unknown_event" debug point to work after
          "queuing" this event.
        */
        mysql_mutex_lock(&mi->data_lock);
        mi->set_master_log_pos(mi->get_master_log_pos() + event_len);
        lock_count = 2;
        goto end;
      });

  /*
    This transaction parser is used to ensure that the GTID of the transaction
    (if it has one) will only be added to the Retrieved_Gtid_Set after the
    last event of the transaction be queued.
    It will also be used to avoid rotating the relay log in the middle of
    a transaction.
  */
  std::tie(info_error, log_event_info) = extract_log_event_basic_info(
      buf, event_len, mi->get_mi_description_event());
  if (info_error || mi->transaction_parser.feed_event(log_event_info, true)) {
    /*
      The transaction parser detected a problem while changing state and threw
      a warning message. We are taking care of avoiding transaction boundary
      issues, but it can happen.

      Transaction boundary errors might happen mostly because of bad master
      positioning in 'CHANGE MASTER TO' (or bad manipulation of master.info)
      when GTID auto positioning is off. Errors can also happen when using
      cross-version replication, replicating from a master that supports more
      event types than this slave.

      The IO thread will keep working and queuing events regardless of the
      transaction parser error, but we will throw another warning message to
      log the relay log file and position of the parser error to help
      forensics.
    */
    LogErr(WARNING_LEVEL,
           ER_RPL_SLAVE_IO_THREAD_DETECTED_UNEXPECTED_EVENT_SEQUENCE,
           mi->get_master_log_name(), mi->get_master_log_pos());
  }

  if (rli->is_row_format_required()) {
    if (info_error ||
        mi->transaction_parser.check_row_logging_constraints(log_event_info)) {
      mi->report(ERROR_LEVEL,
                 ER_RPL_SLAVE_QUEUE_EVENT_FAILED_INVALID_NON_ROW_FORMAT,
                 ER_THD(current_thd,
                        ER_RPL_SLAVE_QUEUE_EVENT_FAILED_INVALID_NON_ROW_FORMAT),
                 mi->get_channel());
      goto err;
    }
  }

  switch (event_type) {
    case binary_log::STOP_EVENT:
      /*
        We needn't write this event to the relay log. Indeed, it just indicates
        a master server shutdown. The only thing this does is cleaning. But
        cleaning is already done on a per-master-thread basis (as the master
        server is shutting down cleanly, it has written all DROP TEMPORARY TABLE
        prepared statements' deletion are TODO only when we binlog prep stmts).

        We don't even increment mi->get_master_log_pos(), because we may be just
        after a Rotate event. Btw, in a few milliseconds we are going to have a
        Start event from the next binlog (unless the master is presently running
        without --log-bin).
      */
      do_flush_mi = false;
      goto end;
    case binary_log::ROTATE_EVENT: {
      Format_description_log_event *fde = mi->get_mi_description_event();
      enum_binlog_checksum_alg fde_checksum_alg = fde->footer()->checksum_alg;
      if (fde_checksum_alg != checksum_alg)
        fde->footer()->checksum_alg = checksum_alg;
      Rotate_log_event rev(buf, fde);
      fde->footer()->checksum_alg = fde_checksum_alg;

      if (unlikely(process_io_rotate(mi, &rev))) {
        // This error will be reported later at handle_slave_io().
        goto err;
      }
      /*
         Checksum special cases for the fake Rotate (R_f) event caused by the
         protocol of events generation and serialization in RL where Rotate of
         master is queued right next to FD of slave. Since it's only FD that
         carries the alg desc of FD_s has to apply to R_m. Two special rules
         apply only to the first R_f which comes in before any FD_m. The 2nd R_f
         should be compatible with the FD_s that must have taken over the last
         seen FD_m's (A).

         RSC_1: If OM \and fake Rotate \and slave is configured to
                to compute checksum for its first FD event for RL
                the fake Rotate gets checksummed here.
      */
      if (uint4korr(&buf[0]) == 0 &&
          checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_OFF &&
          mi->rli->relay_log.relay_log_checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_OFF) {
        ha_checksum rot_crc = checksum_crc32(0L, nullptr, 0);
        event_len += BINLOG_CHECKSUM_LEN;
        memcpy(rot_buf, buf, event_len - BINLOG_CHECKSUM_LEN);
        int4store(&rot_buf[EVENT_LEN_OFFSET],
                  uint4korr(rot_buf + EVENT_LEN_OFFSET) + BINLOG_CHECKSUM_LEN);
        rot_crc = checksum_crc32(rot_crc, (const uchar *)rot_buf,
                                 event_len - BINLOG_CHECKSUM_LEN);
        int4store(&rot_buf[event_len - BINLOG_CHECKSUM_LEN], rot_crc);
        DBUG_ASSERT(event_len == uint4korr(&rot_buf[EVENT_LEN_OFFSET]));
        DBUG_ASSERT(
            mi->get_mi_description_event()->common_footer->checksum_alg ==
            mi->rli->relay_log.relay_log_checksum_alg);
        /* the first one */
        DBUG_ASSERT(mi->checksum_alg_before_fd !=
                    binary_log::BINLOG_CHECKSUM_ALG_UNDEF);
        save_buf = buf;
        buf = rot_buf;
      } else
          /*
            RSC_2: If NM \and fake Rotate \and slave does not compute checksum
            the fake Rotate's checksum is stripped off before relay-logging.
          */
          if (uint4korr(&buf[0]) == 0 &&
              checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF &&
              mi->rli->relay_log.relay_log_checksum_alg ==
                  binary_log::BINLOG_CHECKSUM_ALG_OFF) {
        event_len -= BINLOG_CHECKSUM_LEN;
        memcpy(rot_buf, buf, event_len);
        int4store(&rot_buf[EVENT_LEN_OFFSET],
                  uint4korr(rot_buf + EVENT_LEN_OFFSET) - BINLOG_CHECKSUM_LEN);
        DBUG_ASSERT(event_len == uint4korr(&rot_buf[EVENT_LEN_OFFSET]));
        DBUG_ASSERT(
            mi->get_mi_description_event()->common_footer->checksum_alg ==
            mi->rli->relay_log.relay_log_checksum_alg);
        /* the first one */
        DBUG_ASSERT(mi->checksum_alg_before_fd !=
                    binary_log::BINLOG_CHECKSUM_ALG_UNDEF);
        save_buf = buf;
        buf = rot_buf;
      }
      /*
        Now the I/O thread has just changed its mi->get_master_log_name(), so
        incrementing mi->get_master_log_pos() is nonsense.
      */
      inc_pos = 0;
      break;
    }
    case binary_log::FORMAT_DESCRIPTION_EVENT: {
      /*
        Create an event, and save it (when we rotate the relay log, we will have
        to write this event again).
      */
      /*
        We are the only thread which reads/writes mi_description_event.
        The relay_log struct does not move (though some members of it can
        change), so we needn't any lock (no rli->data_lock, no log lock).
      */
      // mark it as undefined that is irrelevant anymore
      mi->checksum_alg_before_fd = binary_log::BINLOG_CHECKSUM_ALG_UNDEF;
      Format_description_log_event *new_fdle;
      Log_event *ev = nullptr;
      if (binlog_event_deserialize(reinterpret_cast<const unsigned char *>(buf),
                                   event_len, mi->get_mi_description_event(),
                                   true, &ev) != Binlog_read_error::SUCCESS) {
        // This error will be reported later at handle_slave_io().
        goto err;
      }

      new_fdle = dynamic_cast<Format_description_log_event *>(ev);
      if (new_fdle->common_footer->checksum_alg ==
          binary_log::BINLOG_CHECKSUM_ALG_UNDEF)
        new_fdle->common_footer->checksum_alg =
            binary_log::BINLOG_CHECKSUM_ALG_OFF;

      mi->set_mi_description_event(new_fdle);

      /* installing new value of checksum Alg for relay log */
      mi->rli->relay_log.relay_log_checksum_alg =
          new_fdle->common_footer->checksum_alg;

      /*
         Though this does some conversion to the slave's format, this will
         preserve the master's binlog format version, and number of event types.
      */
      /*
         If the event was not requested by the slave (the slave did not ask for
         it), i.e. has end_log_pos=0, we do not increment
         mi->get_master_log_pos()
      */
      inc_pos = uint4korr(buf + LOG_POS_OFFSET) ? event_len : 0;
      DBUG_PRINT("info", ("binlog format is now %d",
                          mi->get_mi_description_event()->binlog_version));

    } break;

    case binary_log::HEARTBEAT_LOG_EVENT: {
      /*
        HB (heartbeat) cannot come before RL (Relay)
      */
      Heartbeat_log_event hb(buf, mi->get_mi_description_event());
      if (!hb.is_valid()) {
        char errbuf[1024];
        char llbuf[22];
        sprintf(errbuf,
                "inconsistent heartbeat event content; the event's data: "
                "log_file_name %-.512s log_pos %s",
                hb.get_log_ident(), llstr(hb.common_header->log_pos, llbuf));
        mi->report(ERROR_LEVEL, ER_SLAVE_HEARTBEAT_FAILURE,
                   ER_THD(current_thd, ER_SLAVE_HEARTBEAT_FAILURE), errbuf);
        goto err;
      }
      mysql_mutex_lock(&mi->data_lock);
      mi->received_heartbeats++;
      mi->last_heartbeat = my_getsystime() / 10;

      /*
        Update the last_master_timestamp if the heartbeat from the master
        has a greater timestamp value, this makes sure last_master_timestamp
        is always monotonically increasing
      */
      mysql_mutex_lock(&rli->data_lock);
      auto io_thread_file = mi->get_master_log_name();
      auto io_thread_pos = mi->get_master_log_pos();
      auto sql_thread_file = mi->rli->get_group_master_log_name();
      auto sql_thread_pos = mi->rli->get_group_master_log_pos();

      // find out if the SQL thread has caught up with the IO thread by
      // comparing their coordinates
      bool caughtup = (sql_thread_pos == io_thread_pos) &&
                      (!strcmp(sql_thread_file, io_thread_file));
      // case: update last master ts only if we've caughtup
      if (caughtup)
        rli->set_last_master_timestamp(hb.common_header->when.tv_sec,
                                       hb.common_header->when.tv_sec * 1000);
      mysql_mutex_unlock(&rli->data_lock);

      /*
        During GTID protocol, if the master skips transactions,
        a heartbeat event is sent to the slave at the end of last
        skipped transaction to update coordinates.

        I/O thread receives the heartbeat event and updates mi
        only if the received heartbeat position is greater than
        mi->get_master_log_pos(). This event is written to the
        relay log as an ignored Rotate event. SQL thread reads
        the rotate event only to update the coordinates corresponding
        to the last skipped transaction. Note that,
        we update only the positions and not the file names, as a ROTATE
        EVENT from the master prior to this will update the file name.
      */
      if (mi->is_auto_position() &&
          mi->get_master_log_pos() < hb.common_header->log_pos &&
          mi->get_master_log_name() != nullptr) {
        DBUG_ASSERT(memcmp(const_cast<char *>(mi->get_master_log_name()),
                           hb.get_log_ident(), hb.get_ident_len()) == 0);

        DBUG_EXECUTE_IF("reached_heart_beat_queue_event",
                        { rpl_slave_debug_point(DBUG_RPL_S_HEARTBEAT_EV); };);
        mi->set_master_log_pos(hb.common_header->log_pos);

        /*
           Put this heartbeat event in the relay log as a Rotate Event.
        */
        inc_pos = 0;
        mysql_mutex_unlock(&mi->data_lock);
        if (write_rotate_to_master_pos_into_relay_log(
                mi->info_thd, mi, false
                /* force_flush_mi_info */))
          goto end;
        do_flush_mi = false; /* write_rotate_... above flushed master info */
      } else
        mysql_mutex_unlock(&mi->data_lock);

      /*
         compare local and event's versions of log_file, log_pos.

         Heartbeat is sent only after an event corresponding to the corrdinates
         the heartbeat carries.
         Slave can not have a difference in coordinates except in the only
         special case when mi->get_master_log_name(), mi->get_master_log_pos()
         have never been updated by Rotate event i.e when slave does not have
         any history with the master (and thereafter mi->get_master_log_pos() is
         NULL).

         TODO: handling `when' for SHOW SLAVE STATUS' snds behind
      */
      if (memcmp(const_cast<char *>(mi->get_master_log_name()),
                 hb.get_log_ident(), hb.get_ident_len()) ||
          (mi->get_master_log_pos() > hb.common_header->log_pos)) {
        /* missed events of heartbeat from the past */
        char errbuf[1024];
        char llbuf[22];
        sprintf(errbuf,
                "heartbeat is not compatible with local info; "
                "the event's data: log_file_name %-.512s log_pos %s",
                hb.get_log_ident(), llstr(hb.common_header->log_pos, llbuf));
        mi->report(ERROR_LEVEL, ER_SLAVE_HEARTBEAT_FAILURE,
                   ER_THD(current_thd, ER_SLAVE_HEARTBEAT_FAILURE), errbuf);
        goto err;
      }
      goto end;
    } break;

    case binary_log::PREVIOUS_GTIDS_LOG_EVENT: {
      /*
        This event does not have any meaning for the slave and
        was just sent to show the slave the master is making
        progress and avoid possible deadlocks.
        So at this point, the event is replaced by a rotate
        event what will make the slave to update what it knows
        about the master's coordinates.
      */
      inc_pos = 0;
      mysql_mutex_lock(&mi->data_lock);
      mi->set_master_log_pos(mi->get_master_log_pos() + event_len);
      mysql_mutex_unlock(&mi->data_lock);

      if (write_rotate_to_master_pos_into_relay_log(
              mi->info_thd, mi, true /* force_flush_mi_info */))
        goto err;

      do_flush_mi = false; /* write_rotate_... above flushed master info */
      goto end;
    } break;

    case binary_log::TRANSACTION_PAYLOAD_EVENT: {
      binary_log::Transaction_payload_event tpe(buf,
                                                mi->get_mi_description_event());
      compression_type = tpe.get_compression_type();
      compressed_transaction_bytes = tpe.get_payload_size();
      uncompressed_transaction_bytes = tpe.get_uncompressed_size();
      auto gtid_monitoring_info = mi->get_gtid_monitoring_info();
      gtid_monitoring_info->update(compression_type,
                                   compressed_transaction_bytes,
                                   uncompressed_transaction_bytes);
      inc_pos = event_len;
      break;
    }

    case binary_log::GTID_LOG_EVENT: {
      /*
        This can happen if the master uses GTID_MODE=OFF_PERMISSIVE, and
        sends GTID events to the slave. A possible scenario is that user
        does not follow the upgrade procedure for GTIDs, and creates a
        topology like A->B->C, where A uses GTID_MODE=ON_PERMISSIVE, B
        uses GTID_MODE=OFF_PERMISSIVE, and C uses GTID_MODE=OFF.  Each
        connection is allowed, but the master A will generate GTID
        transactions which will be sent through B to C.  Then C will hit
        this error.
      */
      if (mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE) == GTID_MODE_OFF) {
        mi->report(
            ERROR_LEVEL, ER_CANT_REPLICATE_GTID_WITH_GTID_MODE_OFF,
            ER_THD(current_thd, ER_CANT_REPLICATE_GTID_WITH_GTID_MODE_OFF),
            mi->get_master_log_name(), mi->get_master_log_pos());
        goto err;
      }
      Gtid_log_event gtid_ev(buf, mi->get_mi_description_event());
      rli->get_sid_lock()->rdlock();
      gtid.sidno = gtid_ev.get_sidno(rli->get_gtid_set()->get_sid_map());
      rli->get_sid_lock()->unlock();
      if (gtid.sidno < 0) goto err;
      gtid.gno = gtid_ev.get_gno();
      original_commit_timestamp = gtid_ev.original_commit_timestamp;
      immediate_commit_timestamp = gtid_ev.immediate_commit_timestamp;
      compressed_transaction_bytes = uncompressed_transaction_bytes =
          gtid_ev.transaction_length - gtid_ev.get_event_length();

      inc_pos = event_len;
    } break;

    case binary_log::ANONYMOUS_GTID_LOG_EVENT: {
      /*
        This cannot normally happen, because the master has a check that
        prevents it from sending anonymous events when auto_position is
        enabled.  However, the master could be something else than
        mysqld, which could contain bugs that we have no control over.
        So we need this check on the slave to be sure that whoever is on
        the other side of the protocol does not break the protocol.
      */
      if (mi->is_auto_position()) {
        mi->report(
            ERROR_LEVEL, ER_CANT_REPLICATE_ANONYMOUS_WITH_AUTO_POSITION,
            ER_THD(current_thd, ER_CANT_REPLICATE_ANONYMOUS_WITH_AUTO_POSITION),
            mi->get_master_log_name(), mi->get_master_log_pos());
        goto err;
      }
      /*
        This can happen if the master uses GTID_MODE=ON_PERMISSIVE, and
        sends an anonymous event to the slave. A possible scenario is
        that user does not follow the upgrade procedure for GTIDs, and
        creates a topology like A->B->C, where A uses
        GTID_MODE=OFF_PERMISSIVE, B uses GTID_MODE=ON_PERMISSIVE, and C
        uses GTID_MODE=ON.  Each connection is allowed, but the master A
        will generate anonymous transactions which will be sent through
        B to C.  Then C will hit this error.
      */
      else {
        if (mi->get_gtid_mode_from_copy(GTID_MODE_LOCK_NONE) == GTID_MODE_ON) {
          mi->report(ERROR_LEVEL, ER_CANT_REPLICATE_ANONYMOUS_WITH_GTID_MODE_ON,
                     ER_THD(current_thd,
                            ER_CANT_REPLICATE_ANONYMOUS_WITH_GTID_MODE_ON),
                     mi->get_master_log_name(), mi->get_master_log_pos());
          goto err;
        }
      }
      /*
       save the original_commit_timestamp and the immediate_commit_timestamp to
       be later used for monitoring
      */
      Gtid_log_event anon_gtid_ev(buf, mi->get_mi_description_event());
      original_commit_timestamp = anon_gtid_ev.original_commit_timestamp;
      immediate_commit_timestamp = anon_gtid_ev.immediate_commit_timestamp;
      compressed_transaction_bytes = uncompressed_transaction_bytes =
          anon_gtid_ev.transaction_length - anon_gtid_ev.get_event_length();
    }
    /* fall through */
    default:
      inc_pos = event_len;
      break;
  }

  /*
    Simulate an unknown ignorable log event by rewriting the write_rows log
    event and previous_gtids log event before writing them in relay log.
  */
  DBUG_EXECUTE_IF(
      "simulate_unknown_ignorable_log_event",
      if (event_type == binary_log::WRITE_ROWS_EVENT ||
          event_type == binary_log::PREVIOUS_GTIDS_LOG_EVENT) {
        char *event_buf = const_cast<char *>(buf);
        /* Overwrite the log event type with an unknown type. */
        event_buf[EVENT_TYPE_OFFSET] = binary_log::ENUM_END_EVENT + 1;
        /* Set LOG_EVENT_IGNORABLE_F for the log event. */
        int2store(event_buf + FLAGS_OFFSET,
                  uint2korr(event_buf + FLAGS_OFFSET) | LOG_EVENT_IGNORABLE_F);
      });

  /*
     If this event is originating from this server, don't queue it.
     We don't check this for 3.23 events because it's simpler like this; 3.23
     will be filtered anyway by the SQL slave thread which also tests the
     server id (we must also keep this test in the SQL thread, in case somebody
     upgrades a 4.0 slave which has a not-filtered relay log).

     ANY event coming from ourselves can be ignored: it is obvious for queries;
     for STOP_EVENT/ROTATE_EVENT/START_EVENT: these cannot come from ourselves
     (--log-slave-updates would not log that) unless this slave is also its
     direct master (an unsupported, useless setup!).
  */

  s_id = uint4korr(buf + SERVER_ID_OFFSET);

  /*
    If server_id_bits option is set we need to mask out irrelevant bits
    when checking server_id, but we still put the full unmasked server_id
    into the Relay log so that it can be accessed when applying the event
  */
  s_id &= opt_server_id_mask;

  if ((s_id == ::server_id && !mi->rli->replicate_same_server_id) ||
      /*
        the following conjunction deals with IGNORE_SERVER_IDS, if set
        If the master is on the ignore list, execution of
        format description log events and rotate events is necessary.
      */
      (mi->ignore_server_ids->dynamic_ids.size() > 0 &&
       mi->shall_ignore_server_id(s_id) &&
       /* everything is filtered out from non-master */
       (s_id != mi->master_id ||
        /* for the master meta information is necessary */
        (event_type != binary_log::FORMAT_DESCRIPTION_EVENT &&
         event_type != binary_log::ROTATE_EVENT)))) {
    /*
      Do not write it to the relay log.
      a) We still want to increment mi->get_master_log_pos(), so that we won't
      re-read this event from the master if the slave IO thread is now
      stopped/restarted (more efficient if the events we are ignoring are big
      LOAD DATA INFILE).
      b) We want to record that we are skipping events, for the information of
      the slave SQL thread, otherwise that thread may let
      rli->group_relay_log_pos stay too small if the last binlog's event is
      ignored.
      But events which were generated by this slave and which do not exist in
      the master's binlog (i.e. Format_desc, Rotate & Stop) should not increment
      mi->get_master_log_pos().
      If the event is originated remotely and is being filtered out by
      IGNORE_SERVER_IDS it increments mi->get_master_log_pos()
      as well as rli->group_relay_log_pos.
    */
    if (!(s_id == ::server_id && !mi->rli->replicate_same_server_id) ||
        (event_type != binary_log::FORMAT_DESCRIPTION_EVENT &&
         event_type != binary_log::ROTATE_EVENT &&
         event_type != binary_log::STOP_EVENT)) {
      rli->relay_log.lock_binlog_end_pos();
      mi->set_master_log_pos(mi->get_master_log_pos() + inc_pos);
      memcpy(rli->ign_master_log_name_end, mi->get_master_log_name(),
             FN_REFLEN);
      DBUG_ASSERT(rli->ign_master_log_name_end[0]);
      rli->ign_master_log_pos_end = mi->get_master_log_pos();
      // the slave SQL thread needs to re-check
      rli->relay_log.update_binlog_end_pos(false /*need_lock*/);
      rli->relay_log.unlock_binlog_end_pos();
    }
    DBUG_PRINT(
        "info",
        ("master_log_pos: %lu, event originating from %u server, ignored",
         (ulong)mi->get_master_log_pos(), uint4korr(buf + SERVER_ID_OFFSET)));
  } else {
    bool is_error = false;
    /* write the event to the relay log */
    if (likely(rli->relay_log.write_buffer(buf, event_len, mi) == 0)) {
      DBUG_SIGNAL_WAIT_FOR(current_thd,
                           "pause_on_queue_event_after_write_buffer",
                           "receiver_reached_pause_on_queue_event",
                           "receiver_continue_queuing_event");
      mysql_mutex_lock(&mi->data_lock);
      lock_count = 2;
      mi->set_master_log_pos(mi->get_master_log_pos() + inc_pos);
      DBUG_PRINT("info",
                 ("master_log_pos: %lu", (ulong)mi->get_master_log_pos()));

      /*
        If we are starting an anonymous transaction, we will discard
        the GTID of the partial transaction that was not finished (if
        there is one) when calling mi->started_queueing().
      */
#ifndef DBUG_OFF
      if (event_type == binary_log::ANONYMOUS_GTID_LOG_EVENT) {
        if (!mi->get_queueing_trx_gtid()->is_empty()) {
          DBUG_PRINT("info", ("Discarding Gtid(%d, %lld) as the transaction "
                              "wasn't complete and we found an "
                              "ANONYMOUS_GTID_LOG_EVENT.",
                              mi->get_queueing_trx_gtid()->sidno,
                              mi->get_queueing_trx_gtid()->gno));
        }
      }
#endif

      /*
        We have to mark this GTID (either anonymous or not) as started
        to be queued.

        Also, if this event is a GTID_LOG_EVENT, we have to store its GTID to
        add to the Retrieved_Gtid_Set later, when the last event of the
        transaction be queued. The call to mi->started_queueing() will save
        the GTID to be used later.
      */
      if (event_type == binary_log::GTID_LOG_EVENT ||
          event_type == binary_log::ANONYMOUS_GTID_LOG_EVENT) {
        // set the timestamp for the start time of queueing this transaction
        mi->started_queueing(gtid, original_commit_timestamp,
                             immediate_commit_timestamp);

        auto gtid_monitoring_info = mi->get_gtid_monitoring_info();
        gtid_monitoring_info->update(
            binary_log::transaction::compression::type::NONE,
            compressed_transaction_bytes, uncompressed_transaction_bytes);
      }
    } else {
      /*
        We failed to write the event and didn't updated slave positions.

        We have to "rollback" the transaction parser state, or else, when
        restarting the I/O thread without GTID auto positing the parser
        would assume the failed event as queued.
      */
      mi->transaction_parser.rollback();
      is_error = true;
    }

    if (save_buf != nullptr) buf = save_buf;
    if (is_error) {
      // This error will be reported later at handle_slave_io().
      goto err;
    }
  }
  goto end;

err:
  res = QUEUE_EVENT_ERROR_QUEUING;

end:
  if (res == QUEUE_EVENT_OK && do_flush_mi) {
    /*
      Take a ride in the already locked LOCK_log to flush master info.

      JAG: TODO: Notice that we could only flush master info if we are
                 not in the middle of a transaction. Having a proper
                 relay log recovery can allow us to do this.
    */
    if (lock_count == 1) {
      mysql_mutex_lock(&mi->data_lock);
      lock_count = 2;
    }

    if (flush_master_info(mi, false /*force*/, lock_count == 0 /*need_lock*/,
                          false /*flush_relay_log*/))
      res = QUEUE_EVENT_ERROR_FLUSHING_INFO;
  }
  if (lock_count >= 2) mysql_mutex_unlock(&mi->data_lock);
  if (lock_count >= 1) mysql_mutex_unlock(log_lock);
  DBUG_PRINT("info", ("queue result: %d", res));
  return res;
}

/**
  Hook to detach the active VIO before closing a connection handle.

  The client API might close the connection (and associated data)
  in case it encounters a unrecoverable (network) error. This hook
  is called from the client code before the VIO handle is deleted
  allows the thread to detach the active vio so it does not point
  to freed memory.

  Other calls to THD::clear_active_vio throughout this module are
  redundant due to the hook but are left in place for illustrative
  purposes.
*/

void slave_io_thread_detach_vio() {
  THD *thd = current_thd;
  if (thd && thd->slave_thread) thd->clear_active_vio();
}

/*
  method to configure some common mysql options for connection to master
*/
void configure_master_connection_options(MYSQL *mysql, Master_info *mi) {
  if (mi->bind_addr[0]) {
    DBUG_PRINT("info", ("bind_addr: %s", mi->bind_addr));
    mysql_options(mysql, MYSQL_OPT_BIND, mi->bind_addr);
  }

  /* By default the channel is not configured to use SSL */
  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;
  if (mi->ssl) {
    /* The channel is configured to use SSL */
    mysql_ssl_set(mysql, mi->ssl_key[0] ? mi->ssl_key : nullptr,
                  mi->ssl_cert[0] ? mi->ssl_cert : nullptr,
                  mi->ssl_ca[0] ? mi->ssl_ca : nullptr,
                  mi->ssl_capath[0] ? mi->ssl_capath : nullptr,
                  mi->ssl_cipher[0] ? mi->ssl_cipher : nullptr);
    mysql_options(mysql, MYSQL_OPT_SSL_CRL,
                  mi->ssl_crl[0] ? mi->ssl_crl : nullptr);
    mysql_options(mysql, MYSQL_OPT_TLS_VERSION,
                  mi->tls_version[0] ? mi->tls_version : nullptr);
    mysql_options(mysql, MYSQL_OPT_TLS_CIPHERSUITES,
                  mi->tls_ciphersuites.first
                      ? nullptr
                      : mi->tls_ciphersuites.second.c_str());
    mysql_options(mysql, MYSQL_OPT_SSL_CRLPATH,
                  mi->ssl_crlpath[0] ? mi->ssl_crlpath : nullptr);
    if (mi->ssl_verify_server_cert)
      ssl_mode = SSL_MODE_VERIFY_IDENTITY;
    else if (mi->ssl_ca[0] || mi->ssl_capath[0])
      ssl_mode = SSL_MODE_VERIFY_CA;
    else
      ssl_mode = SSL_MODE_REQUIRED;
  }
  mysql_options(mysql, MYSQL_OPT_SSL_MODE, &ssl_mode);

  {
    char *algorithm;
    char buf[64];
    if (opt_slave_compressed_protocol) {
#ifndef DBUG_OFF
      int ret =
#endif /* DBUG_OFF */
          /*
            Allow negotiating for uncompressed protocol
            when using slave_compressed_protocol
          */
          snprintf(buf, sizeof(buf), "%s,%s",
                   mysql_compression_lib_names[opt_slave_compression_lib],
                   COMPRESSION_ALGORITHM_UNCOMPRESSED);
      DBUG_ASSERT(ret >= 0 && static_cast<unsigned int>(ret) < sizeof(buf));
      algorithm = buf;
    } else {
      algorithm = mi->compression_algorithm;
    }
    mysql_options(mysql, MYSQL_OPT_COMPRESSION_ALGORITHMS, algorithm);
  }
  mysql_options(mysql, MYSQL_OPT_ZSTD_COMPRESSION_LEVEL,
                opt_slave_compressed_protocol
                    ? (int *)&zstd_net_compression_level
                    : &mi->zstd_compression_level);
  /*
    If server's default charset is not supported (like utf16, utf32) as client
    charset, then set client charset to 'latin1' (default client charset).
  */
  if (is_supported_parser_charset(default_charset_info))
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, default_charset_info->csname);
  else {
    LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_CANT_USE_CHARSET,
           default_charset_info->csname, default_client_charset_info->csname);
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME,
                  default_client_charset_info->csname);
  }

  if (mi->is_start_plugin_auth_configured()) {
    DBUG_PRINT("info", ("Slaving is using MYSQL_DEFAULT_AUTH %s",
                        mi->get_start_plugin_auth()));
    mysql_options(mysql, MYSQL_DEFAULT_AUTH, mi->get_start_plugin_auth());
  }

  if (mi->is_start_plugin_dir_configured()) {
    DBUG_PRINT("info", ("Slaving is using MYSQL_PLUGIN_DIR %s",
                        mi->get_start_plugin_dir()));
    mysql_options(mysql, MYSQL_PLUGIN_DIR, mi->get_start_plugin_dir());
  }
  /* Set MYSQL_PLUGIN_DIR in case master asks for an external authentication
     plugin */
  else if (opt_plugin_dir_ptr && *opt_plugin_dir_ptr)
    mysql_options(mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir_ptr);

  if (mi->public_key_path[0]) {
    /* Set public key path */
    DBUG_PRINT("info", ("Set master's public key path"));
    mysql_options(mysql, MYSQL_SERVER_PUBLIC_KEY, mi->public_key_path);
  }

  /* Get public key from master */
  DBUG_PRINT("info", ("Set preference to get public key from master"));
  mysql_options(mysql, MYSQL_OPT_GET_SERVER_PUBLIC_KEY, &mi->get_public_key);
}

/*
  Try to connect until successful or slave killed

  SYNPOSIS
    safe_connect()
    thd                 Thread handler for slave
    mysql               MySQL connection handle
    mi                  Replication handle

  RETURN
    0   ok
    #   Error
*/

static int safe_connect(THD *thd, MYSQL *mysql, Master_info *mi) {
  DBUG_TRACE;

  return connect_to_master(thd, mysql, mi, false, false);
}

/*
  SYNPOSIS
    connect_to_master()

  IMPLEMENTATION
    Try to connect until successful or slave killed or we have retried
    mi->retry_count times
*/

static int connect_to_master(THD *thd, MYSQL *mysql, Master_info *mi,
                             bool reconnect, bool suppress_warnings) {
  int slave_was_killed = 0;
  int last_errno = -2;  // impossible error
  ulong err_count = 0;
  char llbuff[22];
  char password[MAX_PASSWORD_LENGTH + 1];
  size_t password_size = sizeof(password);
  DBUG_TRACE;
  set_slave_max_allowed_packet(thd, mysql);
#ifndef DBUG_OFF
  mi->events_until_exit = disconnect_slave_event_count;
#endif
  ulong client_flag = CLIENT_REMEMBER_OPTIONS;

  /* Always reset public key to remove cached copy */
  mysql_reset_server_public_key();

  mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&slave_net_timeout);
  mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, (char *)&slave_net_timeout);

  configure_master_connection_options(mysql, mi);

  if (!mi->is_start_user_configured())
    LogErr(WARNING_LEVEL, ER_RPL_SLAVE_INSECURE_CHANGE_MASTER);

  if (mi->get_password(password, &password_size)) {
    mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
               ER_THD(thd, ER_SLAVE_FATAL_ERROR),
               "Unable to configure password when attempting to "
               "connect to the master server. Connection attempt "
               "terminated.");
    return 1;
  }

  const char *user = mi->get_user();
  if (user == nullptr || user[0] == 0) {
    mi->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
               ER_THD(thd, ER_SLAVE_FATAL_ERROR),
               "Invalid (empty) username when attempting to "
               "connect to the master server. Connection attempt "
               "terminated.");
    return 1;
  }

  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "program_name", "mysqld");
  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD, "_client_role",
                 "binary_log_listener");
  mysql_options4(mysql, MYSQL_OPT_CONNECT_ATTR_ADD,
                 "_client_replication_channel_name", mi->get_channel());
  mysql_options(mysql, MYSQL_OPT_NET_RECEIVE_BUFFER_SIZE,
                &rpl_receive_buffer_size);

  mysql_extension_set_server_extn(mysql, &mi->server_extn);
  while (!(slave_was_killed = io_slave_killed(thd, mi)) &&
         (reconnect ? mysql_reconnect(mysql) != 0
                    : mysql_real_connect(mysql, mi->host, user, password,
                                         nullptr, mi->port, nullptr,
                                         client_flag) == nullptr)) {
    /*
       SHOW SLAVE STATUS will display the number of retries which
       would be real retry counts instead of mi->retry_count for
       each connection attempt by 'Last_IO_Error' entry.
    */
    last_errno = mysql_errno(mysql);
    suppress_warnings = false;
    mi->report(ERROR_LEVEL, last_errno,
               "error %s to master '%s@%s:%d'"
               " - retry-time: %d retries: %lu message: %s",
               (reconnect ? "reconnecting" : "connecting"), mi->get_user(),
               mi->host, mi->port, mi->connect_retry, err_count + 1,
               mysql_error(mysql));
    /*
      By default we try forever. The reason is that failure will trigger
      master election, so if the user did not set mi->retry_count we
      do not want to have election triggered on the first failure to
      connect
    */
    if (++err_count == mi->retry_count) {
      slave_was_killed = 1;
      break;
    }
    slave_sleep(thd, mi->connect_retry, io_slave_killed, mi);
    mysql_extension_set_server_extn(mysql, &mi->server_extn);
  }

  if (!slave_was_killed) {
    mi->clear_error();  // clear possible left over reconnect error
    if (reconnect) {
      if (!suppress_warnings)
        LogErr(
            SYSTEM_LEVEL, ER_RPL_SLAVE_CONNECTED_TO_MASTER_REPLICATION_RESUMED,
            mi->get_for_channel_str(), mi->get_user(), mi->host, mi->port,
            mi->get_io_rpl_log_name(), llstr(mi->get_master_log_pos(), llbuff));
    } else {
      query_logger.general_log_print(thd, COM_CONNECT_OUT, "%s@%s:%d",
                                     mi->get_user(), mi->host, mi->port);
    }

    thd->set_active_vio(mysql->net.vio);
  }
  mysql->reconnect = true;
  DBUG_PRINT("exit", ("slave_was_killed: %d", slave_was_killed));
  return slave_was_killed;
}

/*
  safe_reconnect()

  IMPLEMENTATION
    Try to connect until successful or slave killed or we have retried
    mi->retry_count times
*/

static int safe_reconnect(THD *thd, MYSQL *mysql, Master_info *mi,
                          bool suppress_warnings) {
  DBUG_TRACE;
  return connect_to_master(thd, mysql, mi, true, suppress_warnings);
}

/*
  Rotate a relay log (this is used only by FLUSH LOGS; the automatic rotation
  because of size is simpler because when we do it we already have all relevant
  locks; here we don't, so this function is mainly taking locks).
  Returns nothing as we cannot catch any error (MYSQL_BIN_LOG::new_file()
  is void).
*/

int rotate_relay_log(Master_info *mi, bool log_master_fd, bool need_lock,
                     bool need_log_space_lock,
                     RaftRotateInfo *raft_rotate_info) {
  DBUG_TRACE;

  Relay_log_info *rli = mi->rli;

  if (need_lock)
    mysql_mutex_lock(rli->relay_log.get_log_lock());
  else
    mysql_mutex_assert_owner(rli->relay_log.get_log_lock());
  DBUG_EXECUTE_IF("crash_before_rotate_relaylog", DBUG_SUICIDE(););

  int error = 0;

  /*
     We need to test inited because otherwise, new_file() will attempt to lock
     LOCK_log, which may not be inited (if we're not a slave).
  */
  if (!rli->inited) {
    DBUG_PRINT("info", ("rli->inited == 0"));
    goto end;
  }

  /* If the relay log is closed, new_file() will do nothing. */
  if (log_master_fd)
    error = rli->relay_log.new_file_without_locking(
        mi->get_mi_description_event(), raft_rotate_info);
  else
    error = rli->relay_log.new_file_without_locking(nullptr, raft_rotate_info);

  if (error != 0) goto end;

  /*
    We harvest now, because otherwise BIN_LOG_HEADER_SIZE will not immediately
    be counted, so imagine a succession of FLUSH LOGS  and assume the slave
    threads are started:
    relay_log_space decreases by the size of the deleted relay log, but does
    not increase, so flush-after-flush we may become negative, which is wrong.
    Even if this will be corrected as soon as a query is replicated on the
    slave (because the I/O thread will then call harvest_bytes_written() which
    will harvest all these BIN_LOG_HEADER_SIZE we forgot), it may give strange
    output in SHOW SLAVE STATUS meanwhile. So we harvest now.
    If the log is closed, then this will just harvest the last writes, probably
    0 as they probably have been harvested.
  */
  rli->relay_log.harvest_bytes_written(rli, need_log_space_lock);
end:
  if (need_lock) mysql_mutex_unlock(rli->relay_log.get_log_lock());
  return error;
}

int rotate_relay_log_for_raft(RaftRotateInfo *rotate_info) {
  DBUG_ENTER("rotate_relay_log_for_raft");
  int error = 0;
  Master_info *mi = nullptr;

  channel_map.rdlock();

  if (channel_map.get_num_instances() > 1) {
    error = 1;
    goto end;
  }

  mi = channel_map.get_default_channel_mi();
  DBUG_ASSERT(mi);

  // TODO  Check if this is really needed when integrating with plugin
  // m_channel_lock seems to protect things like 'change master' and may not be
  // needed for file rotation
  mi->channel_wrlock();

  /* in case of no_op we would be starting the file name from the master
     so new_log_ident and pos wont be used */
  if (!rotate_info->noop) {
    mysql_mutex_lock(&mi->data_lock);
    memcpy(const_cast<char *>(mi->get_master_log_name()),
           rotate_info->new_log_ident.c_str(),
           rotate_info->new_log_ident.length() + 1);
    mi->set_master_log_pos(rotate_info->pos);
    mysql_mutex_unlock(&mi->data_lock);
  }

  error = rotate_relay_log(mi,
                           /*log_master_fd=*/true,
                           /*need_lock=*/true,
                           /*need_log_space_lock=*/true, rotate_info);

  mi->channel_unlock();

end:
  channel_map.unlock();
  DBUG_RETURN(error);
}

/**
  flushes the relay logs of a replication channel.

  @param[in]         mi      Master_info corresponding to the
                             channel.
  @param[in]         thd     the client thread carrying the command.

  @retval            1       fail
  @retval            0       ok
  @retval            -1      deferred flush
*/
int flush_relay_logs(Master_info *mi, THD *thd) {
  DBUG_TRACE;
  int error = 0;

  if (mi) {
    Relay_log_info *rli = mi->rli;
    if (rli->inited) {
      // Rotate immediately if one is true:
      if ((!is_group_replication_plugin_loaded() ||  // GR is disabled
           !mi->transaction_parser
                .is_inside_transaction() ||  // not inside a transaction
           !channel_map.is_group_replication_channel_name(
               mi->get_channel(), true) ||  // channel isn't GR applier channel
           !mi->slave_running) &&           // the I/O thread isn't running
          DBUG_EVALUATE_IF("deferred_flush_relay_log",
                           !channel_map.is_group_replication_channel_name(
                               mi->get_channel(), true),
                           true)) {
        if (rotate_relay_log(mi)) error = 1;
      }
      // Postpone the rotate action, delegating it to the I/O thread
      else {
        channel_map.unlock();
        mi->request_rotate(thd);
        channel_map.rdlock();
        error = -1;
      }
    }
  }
  return error;
}

/**
   Entry point for FLUSH RELAYLOGS command or to flush relaylogs for
   the FLUSH LOGS command.
   FLUSH LOGS or FLUSH RELAYLOGS needs to flush the relaylogs of all
   the replciaiton channels in multisource replication.
   FLUSH RELAYLOGS FOR CHANNEL flushes only the relaylogs pertaining to
   a channel.

   @param[in]         thd              the client thread carrying the command.

   @retval            true             fail
   @retval            false            success
*/
bool flush_relay_logs_cmd(THD *thd) {
  DBUG_TRACE;
  Master_info *mi = nullptr;
  LEX *lex = thd->lex;
  bool error = false;

  channel_map.rdlock();

  /*
     lex->mi.channel is NULL, for FLUSH LOGS or when the client thread
     is not present. (See tmp_thd in  the caller).
     When channel is not provided, lex->mi.for_channel is false.
  */
  if (!lex->mi.channel || !lex->mi.for_channel) {
    bool flush_was_deferred{false};
    enum_channel_type channel_types[] = {SLAVE_REPLICATION_CHANNEL,
                                         GROUP_REPLICATION_CHANNEL};

    for (auto channel_type : channel_types) {
      mi_map already_processed;

      do {
        flush_was_deferred = false;

        for (mi_map::iterator it = channel_map.begin(channel_type);
             it != channel_map.end(channel_type); it++) {
          if (already_processed.find(it->first) != already_processed.end())
            continue;

          mi = it->second;
          already_processed.insert(std::make_pair(it->first, mi));

          int flush_status = flush_relay_logs(mi, thd);
          flush_was_deferred = (flush_status == -1);
          error = (flush_status == 1);

          if (flush_status != 0) break;
        }
      } while (flush_was_deferred);
    }
  } else {
    mi = channel_map.get_mi(lex->mi.channel);

    if (mi) {
      error = (flush_relay_logs(mi, thd) == 1);
    } else {
      if (thd->system_thread == SYSTEM_THREAD_SLAVE_SQL ||
          thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER) {
        /*
          Log warning on SQL or worker threads.
        */
        LogErr(WARNING_LEVEL, ER_RPL_SLAVE_INCORRECT_CHANNEL, lex->mi.channel);
      } else {
        /*
          Return error on client sessions.
        */
        error = true;
        my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), lex->mi.channel);
      }
    }
  }

  channel_map.unlock();

  return error;
}

bool reencrypt_relay_logs() {
  DBUG_TRACE;

  Master_info *mi;
  channel_map.rdlock();

  enum_channel_type channel_types[] = {SLAVE_REPLICATION_CHANNEL,
                                       GROUP_REPLICATION_CHANNEL};
  for (auto channel_type : channel_types) {
    for (mi_map::iterator it = channel_map.begin(channel_type);
         it != channel_map.end(channel_type); it++) {
      mi = it->second;
      if (mi != nullptr) {
        Relay_log_info *rli = mi->rli;
        if (rli != nullptr && rli->inited && rli->relay_log.reencrypt_logs()) {
          channel_map.unlock();
          return true;
        }
      }
    }
  }

  channel_map.unlock();

  return false;
}

/**
   Detects, based on master's version (as found in the relay log), if master
   has a certain bug.
   @param rli Relay_log_info which tells the master's version
   @param bug_id Number of the bug as found in bugs.mysql.com
   @param report bool report error message, default true

   @param pred Predicate function that will be called with @c param to
   check for the bug. If the function return @c true, the bug is present,
   otherwise, it is not.

   @param param  State passed to @c pred function.

   @return true if master has the bug, false if it does not.
*/
bool rpl_master_has_bug(const Relay_log_info *rli, uint bug_id, bool report,
                        bool (*pred)(const void *), const void *param) {
  struct st_version_range_for_one_bug {
    uint bug_id;
    const uchar introduced_in[3];  // first version with bug
    const uchar fixed_in[3];       // first version with fix
  };
  static struct st_version_range_for_one_bug versions_for_all_bugs[] = {
      {24432, {5, 0, 24}, {5, 0, 38}}, {24432, {5, 1, 12}, {5, 1, 17}},
      {33029, {5, 0, 0}, {5, 0, 58}},  {33029, {5, 1, 0}, {5, 1, 12}},
      {37426, {5, 1, 0}, {5, 1, 26}},
  };
  const uchar *master_ver =
      rli->get_rli_description_event()->server_version_split;

  DBUG_ASSERT(sizeof(rli->get_rli_description_event()->server_version_split) ==
              3);

  for (uint i = 0;
       i < sizeof(versions_for_all_bugs) / sizeof(*versions_for_all_bugs);
       i++) {
    const uchar *introduced_in = versions_for_all_bugs[i].introduced_in,
                *fixed_in = versions_for_all_bugs[i].fixed_in;
    if ((versions_for_all_bugs[i].bug_id == bug_id) &&
        (memcmp(introduced_in, master_ver, 3) <= 0) &&
        (memcmp(fixed_in, master_ver, 3) > 0) &&
        (pred == nullptr || (*pred)(param))) {
      if (!report) return true;
      // a short message for SHOW SLAVE STATUS (message length constraints)
      my_printf_error(ER_UNKNOWN_ERROR,
                      "master may suffer from"
                      " http://bugs.mysql.com/bug.php?id=%u"
                      " so slave stops; check error log on slave"
                      " for more info",
                      MYF(0), bug_id);
      // a verbose message for the error log
      enum loglevel report_level = INFORMATION_LEVEL;
      if (!ignored_error_code(ER_UNKNOWN_ERROR)) {
        report_level = ERROR_LEVEL;
        current_thd->is_slave_error = true;
      } else if (log_error_verbosity >= 2)
        report_level = WARNING_LEVEL;

      if (report_level != INFORMATION_LEVEL)
        rli->report(report_level, ER_SERVER_UNKNOWN_ERROR,
                    "According to the master's version ('%s'),"
                    " it is probable that master suffers from this bug:"
                    " http://bugs.mysql.com/bug.php?id=%u"
                    " and thus replicating the current binary log event"
                    " may make the slave's data become different from the"
                    " master's data."
                    " To take no risk, slave refuses to replicate"
                    " this event and stops."
                    " We recommend that all updates be stopped on the"
                    " master and slave, that the data of both be"
                    " manually synchronized,"
                    " that master's binary logs be deleted,"
                    " that master be upgraded to a version at least"
                    " equal to '%d.%d.%d'. Then replication can be"
                    " restarted.",
                    rli->get_rli_description_event()->server_version, bug_id,
                    fixed_in[0], fixed_in[1], fixed_in[2]);
      return true;
    }
  }
  return false;
}

/**
   BUG#33029, For all 5.0 up to 5.0.58 exclusive, and 5.1 up to 5.1.12
   exclusive, if one statement in a SP generated AUTO_INCREMENT value
   by the top statement, all statements after it would be considered
   generated AUTO_INCREMENT value by the top statement, and a
   erroneous INSERT_ID value might be associated with these statement,
   which could cause duplicate entry error and stop the slave.

   Detect buggy master to work around.
 */
bool rpl_master_erroneous_autoinc(THD *thd) {
  if (thd->rli_slave && thd->rli_slave->info_thd == thd) {
    Relay_log_info *c_rli = thd->rli_slave->get_c_rli();

    DBUG_EXECUTE_IF("simulate_bug33029", return true;);
    return rpl_master_has_bug(c_rli, 33029, false, nullptr, nullptr);
  }
  return false;
}

/**
  a copy of active_mi->rli->slave_skip_counter, for showing in SHOW GLOBAL
  VARIABLES, INFORMATION_SCHEMA.GLOBAL_VARIABLES and @@sql_slave_skip_counter
  without taking all the mutexes needed to access
  active_mi->rli->slave_skip_counter properly.
*/
uint sql_slave_skip_counter;

/**
   Executes a START SLAVE statement.

  @param thd                 Pointer to THD object for the client thread
                             executing the statement.

   @param connection_param   Connection parameters for starting threads

   @param master_param       Master parameters used for starting threads

   @param thread_mask_input  The thread mask that identifies which threads to
                             start. If 0 is passed (start no thread) then this
                             parameter is ignored and all stopped threads are
                             started

   @param mi                 Pointer to Master_info object for the slave's IO
                             thread.

   @param set_mts_settings   If true, the channel uses the server MTS
                             configured settings when starting the applier
                             thread.

   @retval false success
   @retval true error
*/
bool start_slave(THD *thd, LEX_SLAVE_CONNECTION *connection_param,
                 LEX_MASTER_INFO *master_param, int thread_mask_input,
                 Master_info *mi, bool set_mts_settings, bool invoked_by_raft) {
  bool is_error = false;
  int thread_mask;

  DBUG_TRACE;

  /*
    START SLAVE command should ignore 'read-only' and 'super_read_only'
    options so that it can update 'mysql.slave_master_info' and
    'mysql.slave_relay_log_info' replication repository tables.
  */
  thd->set_skip_readonly_check();
  Security_context *sctx = thd->security_context();
  if (!sctx->check_access(SUPER_ACL) &&
      !sctx->has_global_grant(STRING_WITH_LEN("REPLICATION_SLAVE_ADMIN"))
           .first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER or REPLICATION_SLAVE_ADMIN");
    return true;
  }

  mi->channel_wrlock();

  if (connection_param->user || connection_param->password) {
    if (!thd->get_ssl()) {
      push_warning(thd, Sql_condition::SL_NOTE, ER_INSECURE_PLAIN_TEXT,
                   ER_THD(thd, ER_INSECURE_PLAIN_TEXT));
    }
  }

  log_slave_command(thd);

  lock_slave_threads(mi);  // this allows us to cleanly read slave_running
  // Get a mask of _stopped_ threads
  init_thread_mask(&thread_mask, mi, true /* inverse */);
  /*
    Below we will start all stopped threads.  But if the user wants to
    start only one thread, do as if the other thread was running (as we
    don't wan't to touch the other thread), so set the bit to 0 for the
    other thread
  */
  if (thread_mask_input) {
    thread_mask &= thread_mask_input;
  }

  if (thread_mask)  // some threads are stopped, start them
  {
    // If raft is doing some critical operations to block out threads,
    // we disallow slave sql start till raft has restarted the slave
    // thread.
    if (enable_raft_plugin && !invoked_by_raft && sql_thread_stopped_by_raft) {
      unlock_slave_threads(mi);

      mi->channel_unlock();

      // NO_LINT_DEBUG
      sql_print_information(
          "Did not allow start_slave as raft has stopped SQL threads");
      my_error(ER_RAFT_OPERATION_INCOMPATIBLE, MYF(0),
               "start slave not allowed when raft has stopped SQL threads");
      return true;
    }

    (void)invoked_by_raft;
    if (load_mi_and_rli_from_repositories(mi, false, thread_mask)) {
      is_error = true;
      my_error(ER_MASTER_INFO, MYF(0));

      if (enable_raft_plugin) {
        // NO_LINT_DEBUG
        sql_print_error(
            "start_slave: error as load_mi_and_rli_from_repositories failed");
      }
    } else if (*mi->host || !(thread_mask & SLAVE_IO)) {
      /*
        If we will start IO thread we need to take care of possible
        options provided through the START SLAVE if there is any.
      */
      if (thread_mask & SLAVE_IO) {
        if (connection_param->user) {
          mi->set_start_user_configured(true);
          mi->set_user(connection_param->user);
        }
        if (connection_param->password) {
          mi->set_start_user_configured(true);
          mi->set_password(connection_param->password);
        }
        if (connection_param->plugin_auth)
          mi->set_plugin_auth(connection_param->plugin_auth);
        if (connection_param->plugin_dir)
          mi->set_plugin_dir(connection_param->plugin_dir);
      }

      /*
        If we will start SQL thread we will care about UNTIL options If
        not and they are specified we will ignore them and warn user
        about this fact.
      */
      if (thread_mask & SLAVE_SQL) {
        /*
          sql_slave_skip_counter only effects the applier thread which is
          first started. So after sql_slave_skip_counter is copied to
          rli->slave_skip_counter, it is reset to 0.
        */
        mysql_mutex_lock(&LOCK_sql_slave_skip_counter);
        mi->rli->slave_skip_counter = sql_slave_skip_counter;
        sql_slave_skip_counter = 0;
        mysql_mutex_unlock(&LOCK_sql_slave_skip_counter);
        /*
          To cache the MTS system var values and used them in the following
          runtime. The system vars can change meanwhile but having no other
          effects.
          It also allows the per channel definition of this variables.
        */
        if (set_mts_settings) {
          mi->rli->opt_slave_parallel_workers = opt_mts_slave_parallel_workers;
          const auto parallel_option = get_mts_parallel_option();
          if (parallel_option == MTS_PARALLEL_TYPE_DB_NAME)
            mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DB_NAME;
          else if (parallel_option == MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
            mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
          else
            mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DEPENDENCY;

#ifndef DBUG_OFF
          if (!DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0))
#endif
            mi->rli->checkpoint_group = opt_mts_checkpoint_group;
        }

        int slave_errno = mi->rli->init_until_option(thd, master_param);
        if (slave_errno) {
          my_error(slave_errno, MYF(0));
          is_error = true;
        }

        if (!is_error) is_error = check_slave_sql_config_conflict(mi->rli);
      } else if (master_param->pos || master_param->relay_log_pos ||
                 master_param->gtid)
        push_warning(thd, Sql_condition::SL_NOTE, ER_UNTIL_COND_IGNORED,
                     ER_THD(thd, ER_UNTIL_COND_IGNORED));

      if (!is_error)
        is_error =
            start_slave_threads(false /*need_lock_slave=false*/,
                                true /*wait_for_start=true*/, mi, thread_mask);
    } else {
      is_error = true;
      my_error(ER_BAD_SLAVE, MYF(0));
    }
  } else {
    /* no error if all threads are already started, only a warning */
    push_warning_printf(
        thd, Sql_condition::SL_NOTE, ER_SLAVE_CHANNEL_WAS_RUNNING,
        ER_THD(thd, ER_SLAVE_CHANNEL_WAS_RUNNING), mi->get_channel());
  }

  /*
    Clean up start information if there was an attempt to start
    the IO thread to avoid any security issue.
  */
  if (is_error && (thread_mask & SLAVE_IO) == SLAVE_IO) mi->reset_start_info();

  unlock_slave_threads(mi);

  mi->channel_unlock();

  return is_error;
}

/**
 * Grab the master-info structure corresponding to the default channel
 * The caller should hold the requisite locks.
 *
 * @retval A pointer to default channel's mi. nullptr if the mi is not
 * configured correctly or if there are more than one channel
 */
static Master_info *raft_get_default_mi() {
  Master_info *mi = nullptr;

  if (channel_map.get_num_instances() != 1) {
    // NO_LINT_DEBUG
    sql_print_error(
        "Number of channels = %lu. There should be only one channel"
        " with raft.",
        channel_map.get_num_instances());
    goto end;
  }

  mi = channel_map.get_default_channel_mi();
  if (!mi) {
    // NO_LINT_DEBUG
    sql_print_information("Default channel's master-info is not configured");
  }

end:
  return mi;
}

int raft_stop_io_thread(THD *thd) {
  int res = 0;
  Master_info *mi = nullptr;
  bool push_temp_table_warning = false;

  thd->lex->slave_thd_opt = SLAVE_IO;

  channel_map.rdlock();
  mi = raft_get_default_mi();

  if (!mi) {
    // NO_LINT_DEBUG
    sql_print_information(
        "Defaut channel not configured in raft_stop_io_thread");
    goto end;
  }

  res = stop_slave(thd, mi,
                   /*net_report=*/0,
                   /*for_one_channel=*/true, &push_temp_table_warning);

end:
  channel_map.unlock();
  return res;
}

int raft_stop_sql_thread(THD *thd) {
  int res = 0;
  Master_info *mi = nullptr;
  bool push_temp_table_warning = false;

  thd->lex->slave_thd_opt = SLAVE_SQL;

  channel_map.rdlock();
  mi = raft_get_default_mi();
  if (!mi) {
    // NO_LINT_DEBUG
    sql_print_information(
        "Defaut channel not configured in raft_stop_sql_thread");
    goto end;
  }

  res = stop_slave(thd, mi,
                   /*net_report=*/0,
                   /*for_one_channel=*/true, &push_temp_table_warning);
  if (!res) {
    // set this flag to prevent other non-raft actors from
    // starting sql thread during critical raft operations
    sql_thread_stopped_by_raft = true;
  }

end:
  channel_map.unlock();
  return res;
}

int raft_start_sql_thread(THD *thd) {
  int res = 0;
  Master_info *mi = nullptr;
  LEX_SLAVE_CONNECTION lex_connection;
  LEX_MASTER_INFO lex_mi;

  lex_connection.reset();
  thd->lex->slave_thd_opt = SLAVE_SQL;

  channel_map.wrlock();
  mi = raft_get_default_mi();
  if (!mi) {
    // NO_LINT_DEBUG
    sql_print_error("Defaut channel not configured in raft_start_sql_thread");
    res = 1;
    goto end;
  }

  res = start_slave(thd, &lex_connection, &lex_mi, thd->lex->slave_thd_opt, mi,
                    /*set_mts_settings=*/true, true /*invoked_by_raft*/);
  if (!res) {
    // reset this flag to let other non-raft actors
    // to stop and start sql threads.
    sql_thread_stopped_by_raft = false;
  }

end:
  channel_map.unlock();
  return res;
}

/**
  Execute a STOP SLAVE statement.

  @param thd              Pointer to THD object for the client thread executing
                          the statement.

  @param mi               Pointer to Master_info object for the slave's IO
                          thread.

  @param net_report       If true, saves the exit status into Diagnostics_area.

  @param for_one_channel  If the method is being invoked only for one channel

  @param push_temp_tables_warning  If it should push a "have temp tables
                                   warning" once having open temp tables. This
                                   avoids multiple warnings when there is more
                                   than one channel with open temp tables.
                                   This parameter can be removed when the
                                   warning is issued with per-channel
                                   information.

  @retval 0 success
  @retval 1 error
*/
int stop_slave(THD *thd, Master_info *mi, bool net_report, bool for_one_channel,
               bool *push_temp_tables_warning) {
  DBUG_TRACE;

  int slave_errno = 0;
  if (!thd) thd = current_thd;

  /*
    STOP SLAVE command should ignore 'read-only' and 'super_read_only'
    options so that it can update 'mysql.slave_master_info' and
    'mysql.slave_relay_log_info' replication repository tables.
  */
  thd->set_skip_readonly_check();

  Security_context *sctx = thd->security_context();
  if (!sctx->check_access(SUPER_ACL) &&
      !sctx->has_global_grant(STRING_WITH_LEN("REPLICATION_SLAVE_ADMIN"))
           .first) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
             "SUPER or REPLICATION_SLAVE_ADMIN");
    return 1;
  }

  log_slave_command(thd);

  mi->channel_wrlock();

  THD_STAGE_INFO(thd, stage_killing_slave);
  int thread_mask;
  lock_slave_threads(mi);

  DBUG_EXECUTE_IF("simulate_hold_run_locks_on_stop_slave", my_sleep(10000000););

  // Get a mask of _running_ threads
  init_thread_mask(&thread_mask, mi, false /* not inverse*/);

  /*
    Below we will stop all running threads.
    But if the user wants to stop only one thread, do as if the other thread
    was stopped (as we don't wan't to touch the other thread), so set the
    bit to 0 for the other thread
  */
  if (thd->lex->slave_thd_opt) thread_mask &= thd->lex->slave_thd_opt;

  if (thread_mask) {
    slave_errno =
        terminate_slave_threads(mi, thread_mask, rpl_stop_slave_timeout,
                                false /*need_lock_term=false*/);
  } else if (!enable_raft_plugin) {
    // raft plugin doesn't start IO thread
    // no error if both threads are already stopped, only a warning
    slave_errno = 0;
    push_warning_printf(
        thd, Sql_condition::SL_NOTE, ER_SLAVE_CHANNEL_WAS_NOT_RUNNING,
        ER_THD(thd, ER_SLAVE_CHANNEL_WAS_NOT_RUNNING), mi->get_channel());
  }

  /*
    If the slave has open temp tables and there is a following CHANGE MASTER
    there is a possibility that the temporary tables are left open forever.
    Though we dont restrict failover here, we do warn users. In future, we
    should have a command to delete open temp tables the slave has replicated.
    See WL#7441 regarding this command.
  */

  if (mi->rli->atomic_channel_open_temp_tables && *push_temp_tables_warning) {
    push_warning(thd, Sql_condition::SL_WARNING,
                 ER_WARN_OPEN_TEMP_TABLES_MUST_BE_ZERO,
                 ER_THD(thd, ER_WARN_OPEN_TEMP_TABLES_MUST_BE_ZERO));
    *push_temp_tables_warning = false;
  }

  unlock_slave_threads(mi);

  mi->channel_unlock();

  if (slave_errno) {
    if ((slave_errno == ER_STOP_SLAVE_SQL_THREAD_TIMEOUT) ||
        (slave_errno == ER_STOP_SLAVE_IO_THREAD_TIMEOUT)) {
      push_warning(thd, Sql_condition::SL_NOTE, slave_errno,
                   ER_THD_NONCONST(thd, slave_errno));

      /*
        If new slave_errno is added in the if() condition above then make sure
        that there are no % in the error message or change the logging API
        to use verbatim() to avoid % substitutions.
      */
      longlong log_errno = (slave_errno == ER_STOP_SLAVE_SQL_THREAD_TIMEOUT)
                               ? ER_RPL_SLAVE_SQL_THREAD_STOP_CMD_EXEC_TIMEOUT
                               : ER_RPL_SLAVE_IO_THREAD_STOP_CMD_EXEC_TIMEOUT;
      LogErr(WARNING_LEVEL, log_errno);
    }
    if (net_report) my_error(slave_errno, MYF(0));
    return 1;
  } else if (net_report && for_one_channel)
    my_ok(thd);

  return 0;
}

/**
  Execute a RESET SLAVE (for all channels), used in Multisource replication.
  If resetting of a particular channel fails, it exits out.

  @param[in]  thd  THD object of the client.

  @retval     0    success
  @retval     1    error
 */

int reset_slave(THD *thd) {
  DBUG_TRACE;

  channel_map.assert_some_wrlock();

  Master_info *mi = nullptr;
  int result = 0;
  mi_map::iterator it, gr_channel_map_it;
  log_slave_command(thd);
  if (thd->lex->reset_slave_info.all) {
    /* First do reset_slave for default channel */
    mi = channel_map.get_default_channel_mi();
    if (mi && reset_slave(thd, mi, thd->lex->reset_slave_info.all)) return 1;
    /* Do while iteration for rest of the channels */
    it = channel_map.begin();
    while (it != channel_map.end()) {
      if (!it->first.compare(channel_map.get_default_channel())) {
        it++;
        continue;
      }
      mi = it->second;
      DBUG_ASSERT(mi);
      if ((result = reset_slave(thd, mi, thd->lex->reset_slave_info.all)))
        break;
      it = channel_map.begin();
    }
    /* RESET group replication specific channels */
    gr_channel_map_it = channel_map.begin(GROUP_REPLICATION_CHANNEL);
    while (gr_channel_map_it != channel_map.end(GROUP_REPLICATION_CHANNEL)) {
      mi = gr_channel_map_it->second;
      DBUG_ASSERT(mi);
      /*
        We cannot RESET a group replication channel while the group
        replication is running.
      */
      if (is_group_replication_running()) {
        my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
                 "RESET SLAVE ALL FOR CHANNEL", mi->get_channel());
        return 1;
      }
      if ((result = reset_slave(thd, mi, thd->lex->reset_slave_info.all)))
        break;
      gr_channel_map_it = channel_map.begin(GROUP_REPLICATION_CHANNEL);
    }
  } else {
    it = channel_map.begin();
    while (it != channel_map.end()) {
      mi = it->second;
      DBUG_ASSERT(mi);
      if ((result = reset_slave(thd, mi, thd->lex->reset_slave_info.all)))
        break;
      it++;
    }
    /*
      RESET group replication specific channels.

      We cannot RESET a group replication channel while the group
      replication is running.
    */
    gr_channel_map_it = channel_map.begin(GROUP_REPLICATION_CHANNEL);
    while (gr_channel_map_it != channel_map.end(GROUP_REPLICATION_CHANNEL)) {
      mi = gr_channel_map_it->second;
      DBUG_ASSERT(mi);
      if (is_group_replication_running()) {
        my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
                 "RESET SLAVE FOR CHANNEL", mi->get_channel());
        return 1;
      }
      if ((result = reset_slave(thd, mi, thd->lex->reset_slave_info.all)))
        break;
      gr_channel_map_it++;
    }
  }
  return result;
}

/**
  Execute a RESET SLAVE statement.
  Locks slave threads and unlocks the slave threads after executing
  reset slave.

  @param thd        Pointer to THD object of the client thread executing the
                    statement.

  @param mi         Pointer to Master_info object for the slave.

  @param reset_all  Do a full reset or only clean master info structures

  @retval 0   success
  @retval !=0 error
*/
int reset_slave(THD *thd, Master_info *mi, bool reset_all) {
  int thread_mask = 0, error = 0;
  const char *errmsg = "Unknown error occurred while reseting slave";
  DBUG_TRACE;

  if (enable_raft_plugin && !override_enable_raft_check) {
    // NO_LINT_DEBUG
    sql_print_information(
        "Did not allow reset_slave as enable_raft_plugin is ON");
    my_error(ER_RAFT_OPERATION_INCOMPATIBLE, MYF(0),
             "reset slave not allowed when enable_raft_plugin is ON");
    return 1;
  }

  bool is_default_channel =
      strcmp(mi->get_channel(), channel_map.get_default_channel()) == 0;

  /*
    RESET SLAVE command should ignore 'read-only' and 'super_read_only'
    options so that it can update 'mysql.slave_master_info' and
    'mysql.slave_relay_log_info' replication repository tables.
  */
  thd->set_skip_readonly_check();
  mi->channel_wrlock();

  lock_slave_threads(mi);
  init_thread_mask(&thread_mask, mi, false /* not inverse */);
  if (thread_mask)  // We refuse if any slave thread is running
  {
    my_error(ER_SLAVE_CHANNEL_MUST_STOP, MYF(0), mi->get_channel());
    error = ER_SLAVE_CHANNEL_MUST_STOP;
    unlock_slave_threads(mi);
    mi->channel_unlock();
    goto err;
  }

  ha_reset_slave(thd);

  // delete relay logs, clear relay log coordinates
  if ((error = mi->rli->purge_relay_logs(thd, &errmsg,
                                         reset_all && !is_default_channel))) {
    my_error(ER_RELAY_LOG_FAIL, MYF(0), errmsg);
    error = ER_RELAY_LOG_FAIL;
    unlock_slave_threads(mi);
    mi->channel_unlock();
    goto err;
  }

  mysql_bin_log.last_master_timestamp.store(0);

  DBUG_ASSERT(!mi->rli || !mi->rli->slave_running);  // none writes in rli table
  if ((reset_all && remove_info(mi)) ||  // Removes all repository information.
      (!reset_all && reset_info(mi))) {  // Resets log names, positions, etc,
                                         // but keeps configuration information
                                         // needed for a re-connection.
    error = ER_UNKNOWN_ERROR;
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    unlock_slave_threads(mi);
    mi->channel_unlock();
    goto err;
  }
  unlock_slave_threads(mi);

  (void)RUN_HOOK(binlog_relay_io, after_reset_slave, (thd, mi));

  /*
     RESET SLAVE ALL deletes the channels(except default channel), so their mi
     and rli objects are removed. For default channel, its mi and rli are
     deleted and recreated to keep in clear status.
  */
  if (reset_all) {
    bool is_default =
        !strcmp(mi->get_channel(), channel_map.get_default_channel());

    channel_map.delete_mi(mi->get_channel());

    if (is_default) {
      if (!Rpl_info_factory::create_mi_and_rli_objects(
              opt_mi_repository_id, opt_rli_repository_id,
              channel_map.get_default_channel(), true, &channel_map)) {
        error = ER_MASTER_INFO;
        my_message(ER_MASTER_INFO, ER_THD(thd, ER_MASTER_INFO), MYF(0));
      }
    }
  } else {
    mi->channel_unlock();
  }

  is_slave = configured_as_slave();

err:
  return error;
}

/**
  Entry function for RESET SLAVE command. Function either resets
  the slave for all channels or for a single channel.
  When RESET SLAVE ALL is given, the slave_info_objects (mi, rli & workers)
  are destroyed.

  @param[in]           thd          the client thread with the command.

  @retval              false        OK
  @retval              true         not OK
*/
bool reset_slave_cmd(THD *thd) {
  DBUG_TRACE;

  Master_info *mi;
  LEX *lex = thd->lex;
  bool res = true;  // default, an error

  channel_map.wrlock();

  if (!is_slave_configured()) {
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
    channel_map.unlock();
    return res = true;
  }

  if (!lex->mi.for_channel)
    res = reset_slave(thd);
  else {
    mi = channel_map.get_mi(lex->mi.channel);
    /*
      If the channel being used is a group replication channel and
      group_replication is still running we need to disable RESET SLAVE [ALL]
      command.
    */
    if (mi &&
        channel_map.is_group_replication_channel_name(mi->get_channel(),
                                                      true) &&
        is_group_replication_running()) {
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "RESET SLAVE [ALL] FOR CHANNEL", mi->get_channel());
      channel_map.unlock();
      return true;
    }

    if (mi)
      res = reset_slave(thd, mi, thd->lex->reset_slave_info.all);
    else if (strcmp(channel_map.get_default_channel(), lex->mi.channel))
      my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), lex->mi.channel);
  }

  channel_map.unlock();

  return res;
}

/**
   This function checks if the given CHANGE MASTER command has any receive
   option being set or changed.

   - used in change_master().

  @param  lex_mi structure that holds all change master options given on the
          change master command.

  @retval false No change master receive option.
  @retval true  At least one receive option was there.
*/

static bool have_change_master_receive_option(const LEX_MASTER_INFO *lex_mi) {
  bool have_receive_option = false;

  DBUG_TRACE;

  /* Check if *at least one* receive option is given on change master command*/
  if (lex_mi->host || lex_mi->user || lex_mi->password ||
      lex_mi->log_file_name || lex_mi->pos || lex_mi->bind_addr ||
      lex_mi->network_namespace || lex_mi->port || lex_mi->connect_retry ||
      lex_mi->server_id || lex_mi->ssl != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_verify_server_cert != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->heartbeat_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->retry_count_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_key || lex_mi->ssl_cert || lex_mi->ssl_ca ||
      lex_mi->ssl_capath || lex_mi->tls_version ||
      lex_mi->tls_ciphersuites != LEX_MASTER_INFO::UNSPECIFIED ||
      lex_mi->ssl_cipher || lex_mi->ssl_crl || lex_mi->ssl_crlpath ||
      lex_mi->repl_ignore_server_ids_opt == LEX_MASTER_INFO::LEX_MI_ENABLE ||
      lex_mi->public_key_path ||
      lex_mi->get_public_key != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->zstd_compression_level || lex_mi->compression_algorithm ||
      lex_mi->require_row_format != -1)
    have_receive_option = true;

  return have_receive_option;
}

/**
   This function checks all possible cases in which compression algorithm,
   compression level can be configured for a channel.

   - used in change_receive_options

   @param  lex_mi      pointer to structure holding all options specified
                       as part of change master to statement
   @param  mi          pointer to structure holding all options specified
                       as part of change master to statement after performing
                       necessary checks

   @retval false    in case of success
   @retval true     in case of failures
*/
static bool change_master_set_compression(THD *, const LEX_MASTER_INFO *lex_mi,
                                          Master_info *mi) {
  DBUG_TRACE;

  if (lex_mi->compression_algorithm) {
    if (validate_compression_attributes(lex_mi->compression_algorithm,
                                        lex_mi->channel, false))
      return true;
    DBUG_ASSERT(sizeof(mi->compression_algorithm) >
                strlen(lex_mi->compression_algorithm));
    strcpy(mi->compression_algorithm, lex_mi->compression_algorithm);
  }
  /* level specified */
  if (lex_mi->zstd_compression_level) {
    /* vaildate compression level */
    if (!is_zstd_compression_level_valid(lex_mi->zstd_compression_level)) {
      my_error(ER_CHANGE_MASTER_WRONG_COMPRESSION_LEVEL_CLIENT, MYF(0),
               lex_mi->zstd_compression_level, lex_mi->channel);
      return true;
    }
    mi->zstd_compression_level = lex_mi->zstd_compression_level;
  }
  return false;
}

/**
   This function checks if the given CHANGE MASTER command has any execute
   option being set or changed.

   - used in change_master().

  @param  lex_mi structure that holds all change master options given on the
          change master command.

  @param[out] need_relay_log_purge
              - If relay_log_file/relay_log_pos options are used,
                we wont delete relaylogs. We set this boolean flag to false.
              - If relay_log_file/relay_log_pos options are NOT used,
                we return the boolean flag UNCHANGED.
              - Used in change_receive_options() and change_master().

  @retval false No change master execute option.
  @retval true  At least one execute option was there.
*/

static bool have_change_master_execute_option(const LEX_MASTER_INFO *lex_mi,
                                              bool *need_relay_log_purge) {
  bool have_execute_option = false;

  DBUG_TRACE;

  /* Check if *at least one* execute option is given on change master command*/
  if (lex_mi->relay_log_name || lex_mi->relay_log_pos ||
      lex_mi->sql_delay != -1 || lex_mi->privilege_checks_username != nullptr ||
      lex_mi->privilege_checks_none || lex_mi->require_row_format != -1 ||
      lex_mi->require_table_primary_key_check !=
          LEX_MASTER_INFO::LEX_MI_PK_CHECK_UNCHANGED)
    have_execute_option = true;

  if (lex_mi->relay_log_name || lex_mi->relay_log_pos)
    *need_relay_log_purge = false;

  return have_execute_option;
}

/**
   This function is called if the change master command had at least one
   receive option. This function then sets or alters the receive option(s)
   given in the command. The execute options are handled in the function
   change_execute_options()

   - used in change_master().
   - Receiver threads should be stopped when this function is called.

  @param thd    Pointer to THD object for the client thread executing the
                statement.

  @param lex_mi structure that holds all change master options given on the
                change master command.
                Coming from the an executing statement or set directly this
                shall contain connection settings like hostname, user, password
                and other settings like the number of connection retries.

  @param mi     Pointer to Master_info object belonging to the slave's IO
                thread.

  @retval 0    no error i.e., success.
  @retval !=0  error.
*/

static int change_receive_options(THD *thd, LEX_MASTER_INFO *lex_mi,
                                  Master_info *mi) {
  int ret = 0; /* return value. Set if there is an error. */

  DBUG_TRACE;

  /*
    If the user specified host or port without binlog or position,
    reset binlog's name to FIRST and position to 4.
  */

  if ((lex_mi->host && strcmp(lex_mi->host, mi->host)) ||
      (lex_mi->port && lex_mi->port != mi->port)) {
    /*
      This is necessary because the primary key, i.e. host or port, has
      changed.

      The repository does not support direct changes on the primary key,
      so the row is dropped and re-inserted with a new primary key. If we
      don't do that, the master info repository we will end up with several
      rows.
    */
    if (mi->clean_info()) {
      ret = 1;
      goto err;
    }
    mi->master_uuid[0] = 0;
    mi->master_id = 0;
  }

  if ((lex_mi->host || lex_mi->port) && !lex_mi->log_file_name &&
      !lex_mi->pos) {
    char *var_master_log_name = nullptr;
    var_master_log_name = const_cast<char *>(mi->get_master_log_name());
    var_master_log_name[0] = '\0';
    mi->set_master_log_pos(BIN_LOG_HEADER_SIZE);
  }

  if (lex_mi->log_file_name) mi->set_master_log_name(lex_mi->log_file_name);
  if (lex_mi->pos) {
    mi->set_master_log_pos(lex_mi->pos);
  }

  if (lex_mi->log_file_name && !lex_mi->pos)
    push_warning(thd, Sql_condition::SL_WARNING,
                 ER_WARN_ONLY_MASTER_LOG_FILE_NO_POS,
                 ER_THD(thd, ER_WARN_ONLY_MASTER_LOG_FILE_NO_POS));

  DBUG_PRINT("info", ("master_log_pos: %lu", (ulong)mi->get_master_log_pos()));

  if (lex_mi->user || lex_mi->password) {
    if (!thd->get_ssl()) {
      push_warning(thd, Sql_condition::SL_NOTE, ER_INSECURE_PLAIN_TEXT,
                   ER_THD(thd, ER_INSECURE_PLAIN_TEXT));
    }
    push_warning(thd, Sql_condition::SL_NOTE, ER_INSECURE_CHANGE_MASTER,
                 ER_THD(thd, ER_INSECURE_CHANGE_MASTER));
  }

  if (lex_mi->user) mi->set_user(lex_mi->user);
  if (lex_mi->password) mi->set_password(lex_mi->password);
  if (lex_mi->host) strmake(mi->host, lex_mi->host, sizeof(mi->host) - 1);
  if (lex_mi->bind_addr)
    strmake(mi->bind_addr, lex_mi->bind_addr, sizeof(mi->bind_addr) - 1);

  if (lex_mi->network_namespace)
    strmake(mi->network_namespace, lex_mi->network_namespace,
            sizeof(mi->network_namespace) - 1);
  /*
    Setting channel's port number explicitly to '0' should be allowed.
    Eg: 'group_replication_recovery' channel (*after recovery is done*)
    or 'group_replication_applier' channel wants to set the port number
    to '0' as there is no actual network usage on these channels.
  */
  if (lex_mi->port || lex_mi->port_opt == LEX_MASTER_INFO::LEX_MI_ENABLE)
    mi->port = lex_mi->port;
  if (lex_mi->connect_retry) mi->connect_retry = lex_mi->connect_retry;
  if (lex_mi->retry_count_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->retry_count = lex_mi->retry_count;

  if (lex_mi->heartbeat_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->heartbeat_period = lex_mi->heartbeat_period;
  else if (lex_mi->host || lex_mi->port) {
    /*
      If the user specified host or port or both without heartbeat_period,
      we use default value for heartbeat_period. By default, We want to always
      have heartbeat enabled when we switch master unless
      master_heartbeat_period is explicitly set to zero (heartbeat disabled).

      Here is the default value for heartbeat period if CHANGE MASTER did not
      specify it.  (no data loss in conversion as hb period has a max)
    */
    mi->heartbeat_period =
        min<float>(SLAVE_MAX_HEARTBEAT_PERIOD, (slave_net_timeout / 2.0f));
    DBUG_ASSERT(mi->heartbeat_period > (float)0.001 ||
                mi->heartbeat_period == 0);

    // counter is cleared if master is CHANGED.
    mi->received_heartbeats = 0;
    // clear timestamp of last heartbeat as well.
    mi->last_heartbeat = 0;
  }

  /*
    reset the last time server_id list if the current CHANGE MASTER
    is mentioning IGNORE_SERVER_IDS= (...)
  */
  if (lex_mi->repl_ignore_server_ids_opt == LEX_MASTER_INFO::LEX_MI_ENABLE)
    mi->ignore_server_ids->dynamic_ids.clear();
  for (size_t i = 0; i < lex_mi->repl_ignore_server_ids.size(); i++) {
    ulong s_id = lex_mi->repl_ignore_server_ids[i];
    if (s_id == ::server_id && replicate_same_server_id) {
      ret = ER_SLAVE_IGNORE_SERVER_IDS;
      my_error(ER_SLAVE_IGNORE_SERVER_IDS, MYF(0), static_cast<int>(s_id));
      goto err;
    } else {
      // Keep the array sorted, ignore duplicates.
      mi->ignore_server_ids->dynamic_ids.insert_unique(s_id);
    }
  }

  if (lex_mi->ssl != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->ssl = (lex_mi->ssl == LEX_MASTER_INFO::LEX_MI_ENABLE);

  if (lex_mi->ssl_verify_server_cert != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->ssl_verify_server_cert =
        (lex_mi->ssl_verify_server_cert == LEX_MASTER_INFO::LEX_MI_ENABLE);

  if (lex_mi->public_key_path)
    strmake(mi->public_key_path, lex_mi->public_key_path,
            sizeof(mi->public_key_path) - 1);

  if (lex_mi->get_public_key != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->get_public_key =
        (lex_mi->get_public_key == LEX_MASTER_INFO::LEX_MI_ENABLE);

  if (lex_mi->ssl_ca)
    strmake(mi->ssl_ca, lex_mi->ssl_ca, sizeof(mi->ssl_ca) - 1);
  if (lex_mi->ssl_capath)
    strmake(mi->ssl_capath, lex_mi->ssl_capath, sizeof(mi->ssl_capath) - 1);
  if (lex_mi->tls_version)
    strmake(mi->tls_version, lex_mi->tls_version, sizeof(mi->tls_version) - 1);

  if (LEX_MASTER_INFO::SPECIFIED_NULL == lex_mi->tls_ciphersuites) {
    mi->tls_ciphersuites.first = true;
    mi->tls_ciphersuites.second.clear();
  } else if (LEX_MASTER_INFO::SPECIFIED_STRING == lex_mi->tls_ciphersuites) {
    mi->tls_ciphersuites.first = false;
    mi->tls_ciphersuites.second.assign(lex_mi->tls_ciphersuites_string);
  }

  if (lex_mi->ssl_cert)
    strmake(mi->ssl_cert, lex_mi->ssl_cert, sizeof(mi->ssl_cert) - 1);
  if (lex_mi->ssl_cipher)
    strmake(mi->ssl_cipher, lex_mi->ssl_cipher, sizeof(mi->ssl_cipher) - 1);
  if (lex_mi->ssl_key)
    strmake(mi->ssl_key, lex_mi->ssl_key, sizeof(mi->ssl_key) - 1);
  if (lex_mi->ssl_crl)
    strmake(mi->ssl_crl, lex_mi->ssl_crl, sizeof(mi->ssl_crl) - 1);
  if (lex_mi->ssl_crlpath)
    strmake(mi->ssl_crlpath, lex_mi->ssl_crlpath, sizeof(mi->ssl_crlpath) - 1);

  ret = change_master_set_compression(thd, lex_mi, mi);
  if (ret) goto err;

err:
  return ret;
}

/**
   This function is called if the change master command had at least one
   execute option. This function then sets or alters the execute option(s)
   given in the command. The receive options are handled in the function
   change_receive_options()

   - used in change_master().
   - Execute threads should be stopped before this function is called.

  @param lex_mi structure that holds all change master options given on the
                change master command.

  @param mi     Pointer to Master_info object belonging to the slave's IO
                thread.

  @return       false if the execute options were successfully set and true,
                otherwise.
*/

static bool change_execute_options(LEX_MASTER_INFO *lex_mi, Master_info *mi) {
  DBUG_TRACE;

  if (lex_mi->privilege_checks_username != nullptr ||
      lex_mi->privilege_checks_none) {
    Relay_log_info::enum_priv_checks_status error{
        mi->rli->set_privilege_checks_user(
            lex_mi->privilege_checks_username,
            lex_mi->privilege_checks_none ? nullptr
                                          : lex_mi->privilege_checks_hostname)};
    if (!!error) {
      mi->rli->report_privilege_check_error(
          ERROR_LEVEL, error, true /* to client*/, mi->rli->get_channel(),
          lex_mi->privilege_checks_username, lex_mi->privilege_checks_hostname);
      return true;
    }
  }

  if (lex_mi->require_row_format != -1) {  // Is included in CHM statement
    mi->rli->set_require_row_format(lex_mi->require_row_format);
  }

  if (lex_mi->require_table_primary_key_check !=
      LEX_MASTER_INFO::LEX_MI_PK_CHECK_UNCHANGED) {
    switch (lex_mi->require_table_primary_key_check) {
      case (LEX_MASTER_INFO::LEX_MI_PK_CHECK_STREAM):
        mi->rli->set_require_table_primary_key_check(
            Relay_log_info::PK_CHECK_STREAM);
        break;
      case (LEX_MASTER_INFO::LEX_MI_PK_CHECK_ON):
        mi->rli->set_require_table_primary_key_check(
            Relay_log_info::PK_CHECK_ON);
        break;
      case (LEX_MASTER_INFO::LEX_MI_PK_CHECK_OFF):
        mi->rli->set_require_table_primary_key_check(
            Relay_log_info::PK_CHECK_OFF);
        break;

      default:          /* purecov: tested */
        DBUG_ASSERT(0); /* purecov: tested */
        break;
    }
  }

  if (lex_mi->relay_log_name) {
    char relay_log_name[FN_REFLEN];
    mi->rli->relay_log.make_log_name(relay_log_name, lex_mi->relay_log_name);
    mi->rli->set_group_relay_log_name(relay_log_name);
    mi->rli->is_group_master_log_pos_invalid = true;
  }

  if (lex_mi->relay_log_pos) {
    mi->rli->set_group_relay_log_pos(lex_mi->relay_log_pos);
    mi->rli->is_group_master_log_pos_invalid = true;
  }

  if (lex_mi->sql_delay != -1) mi->rli->set_sql_delay(lex_mi->sql_delay);

  return false;
}

/**
  This function shall issue a deprecation warning if
  there are server ids tokenized from the CHANGE MASTER
  TO command while @@global.gtid_mode=ON.
 */
static void issue_deprecation_warnings_for_channel(THD *thd) {
  LEX_MASTER_INFO *lex_mi = &thd->lex->mi;

  /*
    Deprecation of GTID_MODE + IGNORE_SERVER_IDS

    Generate deprecation warning when user executes CHANGE
    MASTER TO IGNORE_SERVER_IDS if GTID_MODE=ON.
  */
  enum_gtid_mode gtid_mode = get_gtid_mode(GTID_MODE_LOCK_CHANNEL_MAP);
  if (lex_mi->repl_ignore_server_ids.size() > 0 && gtid_mode == GTID_MODE_ON) {
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_WARN_DEPRECATED_SYNTAX,
                        ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT),
                        "CHANGE MASTER TO ... IGNORE_SERVER_IDS='...' "
                        "(when @@GLOBAL.GTID_MODE = ON)");
  }
}

/**
  Execute a CHANGE MASTER statement.

  Apart from changing the receive/execute configurations/positions,
  this function also does the following:
  - May leave replicated open temporary table after warning.
  - Purges relay logs if no threads running and no relay log file/pos options.
  - Delete worker info in mysql.slave_worker_info table if applier not running.

  @param thd            Pointer to THD object for the client thread executing
                        the statement.

  @param mi             Pointer to Master_info object belonging to the slave's
                        IO thread.

  @param lex_mi         Lex information with master connection data.
                        Coming from the an executing statement or set directly
                        this shall contain connection settings like hostname,
                        user, password and other settings like the number of
                        connection retries.

  @param preserve_logs  If the decision of purging the logs should be always be
                        false even if no relay log name/position is given to
                        the method. The preserve_logs parameter will not be
                        respected when the relay log info repository is not
                        initialized.

  @retval 0   success
  @retval !=0 error
*/
int change_master(THD *thd, Master_info *mi, LEX_MASTER_INFO *lex_mi,
                  bool preserve_logs) {
  int error = 0;

  /* Do we have at least one receive related (IO thread) option? */
  bool have_receive_option = false;
  /* Do we have at least one execute related (SQL/coord/worker) option? */
  bool have_execute_option = false;
  /* If there are no mts gaps, we delete the rows in this table. */
  bool mts_remove_worker_info = false;
  /* used as a bit mask to indicate running slave threads. */
  int thread_mask;
  /*
    Relay logs are purged only if both receive and execute threads are
    stopped before executing CHANGE MASTER and relay_log_file/relay_log_pos
    options are not used.
  */
  bool need_relay_log_purge = true;

  /*
    We want to save the old receive configurations so that we can use them to
    print the changes in these configurations (from-to form). This is used in
    LogErr() later.
  */
  char saved_host[HOSTNAME_LENGTH + 1], saved_bind_addr[HOSTNAME_LENGTH + 1];
  uint saved_port = 0;
  char saved_log_name[FN_REFLEN];
  my_off_t saved_log_pos = 0;

  DBUG_TRACE;

  if (enable_raft_plugin && !override_enable_raft_check) {
    // NO_LINT_DEBUG
    sql_print_information(
        "Did not allow change_master as enable_raft_plugin is ON");
    my_error(ER_RAFT_OPERATION_INCOMPATIBLE, MYF(0),
             "change master not allowed when enable_raft_plugin is ON");
    return 1;
  }

  log_slave_command(thd);

  /*
    CHANGE MASTER command should ignore 'read-only' and 'super_read_only'
    options so that it can update 'mysql.slave_master_info' replication
    repository tables.
  */
  thd->set_skip_readonly_check();
  mi->channel_wrlock();
  /*
    When we change master, we first decide which thread is running and
    which is not. We dont want this assumption to break while we change master.

    Suppose we decide that receiver thread is running and thus it is
    safe to change receive related options in mi. By this time if
    the receive thread is started, we may have a race condition between
    the client thread and receiver thread.
  */
  lock_slave_threads(mi);

  /*
    Get a bit mask for the slave threads that are running.
    Since the third argument is 0, thread_mask after the function
    returns stands for running threads.
  */
  init_thread_mask(&thread_mask, mi, false);

  /*
    change master with master_auto_position=1 requires stopping both
    receiver and applier threads. If any slave thread is running,
    we report an error.
  */
  if (thread_mask) /* If any thread is running */
  {
    if (lex_mi->auto_position != LEX_MASTER_INFO::LEX_MI_UNCHANGED) {
      error = ER_SLAVE_CHANNEL_MUST_STOP;
      my_error(ER_SLAVE_CHANNEL_MUST_STOP, MYF(0), mi->get_channel());
      goto err;
    }
    /*
      Prior to WL#6120, we imposed the condition that STOP SLAVE is required
      before CHANGE MASTER. Since the slave threads die on STOP SLAVE, it was
      fine if we purged relay logs.

      Now that we do allow CHANGE MASTER with a running receiver/applier thread,
      we need to make sure that the relay logs are purged only if both
      receiver and applier threads are stopped otherwise we could lose events.

      The idea behind purging relay logs if both the threads are stopped is to
      keep consistency with the old behavior. If the user/application is doing
      a CHANGE MASTER without stopping any one thread, the relay log purge
      should be controlled via the 'relay_log_purge' option.
    */
    need_relay_log_purge = false;
  }

  /*
    We cannot specify auto position and set either the coordinates
    on master or slave. If we try to do so, an error message is
    printed out.
  */
  if (lex_mi->log_file_name != nullptr || lex_mi->pos != 0 ||
      lex_mi->relay_log_name != nullptr || lex_mi->relay_log_pos != 0) {
    if (lex_mi->auto_position == LEX_MASTER_INFO::LEX_MI_ENABLE ||
        (lex_mi->auto_position != LEX_MASTER_INFO::LEX_MI_DISABLE &&
         mi->is_auto_position())) {
      error = ER_BAD_SLAVE_AUTO_POSITION;
      my_error(ER_BAD_SLAVE_AUTO_POSITION, MYF(0));
      goto err;
    }
  }

  /* CHANGE MASTER TO MASTER_AUTO_POSITION = 1 requires GTID_MODE != OFF */
  if (lex_mi->auto_position == LEX_MASTER_INFO::LEX_MI_ENABLE &&
      /*
        We hold channel_map lock for the duration of the CHANGE MASTER.
        This is important since it prevents that a concurrent
        connection changes to GTID_MODE=OFF between this check and the
        point where AUTO_POSITION is stored in the table and in mi.
      */
      get_gtid_mode(GTID_MODE_LOCK_CHANNEL_MAP) == GTID_MODE_OFF) {
    error = ER_AUTO_POSITION_REQUIRES_GTID_MODE_NOT_OFF;
    my_error(ER_AUTO_POSITION_REQUIRES_GTID_MODE_NOT_OFF, MYF(0));
    goto err;
  }

  /* Check if at least one receive option is given on change master */
  have_receive_option = have_change_master_receive_option(lex_mi);

  /* Check if at least one execute option is given on change master */
  have_execute_option =
      have_change_master_execute_option(lex_mi, &need_relay_log_purge);

  if (need_relay_log_purge && /* If we should purge the logs for this channel */
      preserve_logs &&        /* And we were asked to keep them */
      mi->rli->inited)        /* And the channel was initialized properly */
  {
    need_relay_log_purge = false;
  }

  /*
    With both threads running, we dont allow changing either receive or execute
    options.
   */
  if (have_receive_option && have_execute_option && (thread_mask & SLAVE_IO) &&
      (thread_mask & SLAVE_SQL)) {
    error = ER_SLAVE_CHANNEL_MUST_STOP;
    my_error(ER_SLAVE_CHANNEL_MUST_STOP, MYF(0), mi->get_channel());
    goto err;
  }

  /* With receiver thread running, we dont allow changing receive options. */
  if (have_receive_option && (thread_mask & SLAVE_IO)) {
    error = ER_SLAVE_CHANNEL_IO_THREAD_MUST_STOP;
    my_error(ER_SLAVE_CHANNEL_IO_THREAD_MUST_STOP, MYF(0), mi->get_channel());
    goto err;
  }

  /* With an execute thread running, we don't allow changing execute options. */
  if (have_execute_option && (thread_mask & SLAVE_SQL)) {
    error = ER_SLAVE_CHANNEL_SQL_THREAD_MUST_STOP;
    my_error(ER_SLAVE_CHANNEL_SQL_THREAD_MUST_STOP, MYF(0), mi->get_channel());
    goto err;
  }

  /*
    We need to check if there is an empty master_host. Otherwise
    change master succeeds, a master.info file is created containing
    empty master_host string and when issuing: start slave; an error
    is thrown stating that the server is not configured as slave.
    (See BUG#28796).
  */
  if (lex_mi->host && !*lex_mi->host) {
    error = ER_WRONG_ARGUMENTS;
    my_error(ER_WRONG_ARGUMENTS, MYF(0), "MASTER_HOST");
    goto err;
  }

  THD_STAGE_INFO(thd, stage_changing_master);

  int thread_mask_stopped_threads;

  /*
    Before load_mi_and_rli_from_repositories() call, get a bit mask to indicate
    stopped threads in thread_mask_stopped_threads. Since the third argguement
    is 1, thread_mask when the function returns stands for stopped threads.
  */

  init_thread_mask(&thread_mask_stopped_threads, mi, true);

  if (load_mi_and_rli_from_repositories(mi, false, thread_mask_stopped_threads,
                                        need_relay_log_purge)) {
    error = ER_MASTER_INFO;
    my_error(ER_MASTER_INFO, MYF(0));
    goto err;
  }

  if (channel_map.is_group_replication_channel_name(lex_mi->channel)) {
    mi->rli->set_require_row_format(true);
  }

  if (have_execute_option && (error = change_execute_options(lex_mi, mi)))
    goto err;

  if ((thread_mask & SLAVE_SQL) == 0)  // If execute threads are stopped
  {
    if (mi->rli->mts_recovery_group_cnt) {
      /*
        Change-Master can't be done if there is a mts group gap.
        That requires mts-recovery which START SLAVE provides.
      */
      DBUG_ASSERT(mi->rli->recovery_parallel_workers);

      error = ER_MTS_CHANGE_MASTER_CANT_RUN_WITH_GAPS;
      my_error(ER_MTS_CHANGE_MASTER_CANT_RUN_WITH_GAPS, MYF(0));
      goto err;
    } else {
      /*
        Lack of mts group gaps makes Workers info stale regardless of
        need_relay_log_purge computation. We set the mts_remove_worker_info
        flag here and call reset_workers() later to delete the worker info
        in mysql.slave_worker_info table.
      */
      if (mi->rli->recovery_parallel_workers) mts_remove_worker_info = true;
    }
  }

  /*
    When give a warning?
    CHANGE MASTER command is used in three ways:
    a) To change a connection configuration but remain connected to
       the same master.
    b) To change positions in binary or relay log(eg: master_log_pos).
    c) To change the master you are replicating from.
    We give a warning in cases b and c.
  */
  if ((lex_mi->host || lex_mi->port || lex_mi->log_file_name || lex_mi->pos ||
       lex_mi->relay_log_name || lex_mi->relay_log_pos) &&
      (mi->rli->atomic_channel_open_temp_tables > 0))
    push_warning(thd, Sql_condition::SL_WARNING,
                 ER_WARN_OPEN_TEMP_TABLES_MUST_BE_ZERO,
                 ER_THD(thd, ER_WARN_OPEN_TEMP_TABLES_MUST_BE_ZERO));

  /*
    auto_position is the only option that affects both receive
    and execute sections of replication. So, this code is kept
    outside both if (have_receive_option) and if (have_execute_option)

    Here, we check if the auto_position option was used and set the flag
    if the slave should connect to the master and look for GTIDs.
  */
  if (lex_mi->auto_position != LEX_MASTER_INFO::LEX_MI_UNCHANGED)
    mi->set_auto_position(
        (lex_mi->auto_position == LEX_MASTER_INFO::LEX_MI_ENABLE));

  if (have_receive_option) {
    strmake(saved_host, mi->host, HOSTNAME_LENGTH);
    strmake(saved_bind_addr, mi->bind_addr, HOSTNAME_LENGTH);
    saved_port = mi->port;
    strmake(saved_log_name, mi->get_master_log_name(), FN_REFLEN - 1);
    saved_log_pos = mi->get_master_log_pos();

    if ((error = change_receive_options(thd, lex_mi, mi))) {
      goto err;
    }
  }

  /*
    If user didn't specify neither host nor port nor any log name nor any log
    pos, i.e. he specified only user/password/master_connect_retry,
    master_delay, he probably  wants replication to resume from where it had
    left, i.e. from the coordinates of the **SQL** thread (imagine the case
    where the I/O is ahead of the SQL; restarting from the coordinates of the
    I/O would lose some events which is probably unwanted when you are just
    doing minor changes like changing master_connect_retry). Note: coordinates
    of the SQL thread must be read before the block which resets them.
  */
  if (need_relay_log_purge) {
    /*
      A side-effect is that if only the I/O thread was started, this thread may
      restart from ''/4 after the CHANGE MASTER. That's a minor problem (it is a
      much more unlikely situation than the one we are fixing here).
    */
    if (!lex_mi->host && !lex_mi->port && !lex_mi->log_file_name &&
        !lex_mi->pos) {
      /*
        Sometimes mi->rli->master_log_pos == 0 (it happens when the SQL thread
        is not initialized), so we use a max(). What happens to
        mi->rli->master_log_pos during the initialization stages of replication
        is not 100% clear, so we guard against problems using max().
      */
      mi->set_master_log_pos(max<ulonglong>(
          BIN_LOG_HEADER_SIZE, mi->rli->get_group_master_log_pos()));
      mi->set_master_log_name(mi->rli->get_group_master_log_name());
    }
  }

  if (have_receive_option)
    LogErr(SYSTEM_LEVEL, ER_SLAVE_CHANGE_MASTER_TO_EXECUTED,
           mi->get_for_channel_str(true), saved_host, saved_port,
           saved_log_name, (ulong)saved_log_pos, saved_bind_addr, mi->host,
           mi->port, mi->get_master_log_name(), (ulong)mi->get_master_log_pos(),
           mi->bind_addr);

  /* If the receiver is stopped, flush master_info to disk. */
  if ((thread_mask & SLAVE_IO) == 0 && flush_master_info(mi, true)) {
    error = ER_RELAY_LOG_INIT;
    my_error(ER_RELAY_LOG_INIT, MYF(0), "Failed to flush master info file");
    goto err;
  }

  if ((thread_mask & SLAVE_SQL) == 0) /* Applier module is not executing */
  {
    if (need_relay_log_purge) {
      /*
        'if (need_relay_log_purge)' implicitly means that all slave threads are
        stopped and there is no use of relay_log_file/relay_log_pos options.
        We need not check these here again.
      */

      /* purge_relay_log() returns pointer to an error message here. */
      const char *errmsg = nullptr;
      /*
        purge_relay_log() assumes that we have run_lock and no slave threads
        are running.
      */
      THD_STAGE_INFO(thd, stage_purging_old_relay_logs);
      if (mi->rli->purge_relay_logs(thd, &errmsg)) {
        error = ER_RELAY_LOG_FAIL;
        my_error(ER_RELAY_LOG_FAIL, MYF(0), errmsg);
        goto err;
      }

      /*
        Coordinates in rli were spoilt by purge_relay_logs(),
        so restore them to good values. If we left them to ''/0, that would
        work. But that would fail in the case of 2 successive CHANGE MASTER
        (without a START SLAVE in between): because first one would set the
        coords in mi to the good values of those in rli, then set those i>n rli
        to ''/0, then second CHANGE MASTER would set the coords in mi to those
        of rli, i.e. to ''/0: we have lost all copies of the original good
        coordinates. That's why we always save good coords in rli.
*/
      mi->rli->set_group_master_log_pos(mi->get_master_log_pos());
      mi->rli->set_group_master_log_name(mi->get_master_log_name());
      DBUG_PRINT("info", ("master_log_pos: %llu", mi->get_master_log_pos()));
    } else {
      const char *errmsg = nullptr;
      if (mi->rli->is_group_relay_log_name_invalid(&errmsg)) {
        error = ER_RELAY_LOG_INIT;
        my_error(ER_RELAY_LOG_INIT, MYF(0), errmsg);
        goto err;
      }
    }

    char *var_group_master_log_name =
        const_cast<char *>(mi->rli->get_group_master_log_name());

    if (!var_group_master_log_name[0])  // uninitialized case
      mi->rli->set_group_master_log_pos(0);

    mi->rli->abort_pos_wait++; /* for MASTER_POS_WAIT() to abort */

    /* Clear the errors, for a clean start */
    mi->rli->clear_error();
    if (mi->rli->workers_array_initialized) {
      for (size_t i = 0; i < mi->rli->get_worker_count(); i++) {
        mi->rli->get_worker(i)->clear_error();
      }
    }

    is_slave = configured_as_slave();
    /*
      If we don't write new coordinates to disk now, then old will remain in
      relay-log.info until START SLAVE is issued; but if mysqld is shutdown
      before START SLAVE, then old will remain in relay-log.info, and will be
      the in-memory value at restart (thus causing errors, as the old relay log
      does not exist anymore).

      Notice that the rli table is available exclusively as slave is not
      running.
    */
    if (mi->rli->flush_info(true)) {
      error = ER_RELAY_LOG_INIT;
      my_error(ER_RELAY_LOG_INIT, MYF(0), "Failed to flush relay info file.");
      goto err;
    }

  } /* end 'if (thread_mask & SLAVE_SQL == 0)' */

  if (mts_remove_worker_info)
    if (Rpl_info_factory::reset_workers(mi->rli)) {
      error = ER_MTS_RESET_WORKERS;
      my_error(ER_MTS_RESET_WORKERS, MYF(0));
      goto err;
    }
err:

  unlock_slave_threads(mi);
  mi->channel_unlock();
  return error;
}

/**
   This function is first called when the Master_info object
   corresponding to a channel in a multisourced slave does not
   exist. But before a new channel is created, certain
   conditions have to be met. The below function apriorily
   checks if all such conditions are met. If all the
   conditions are met then it creates a channel i.e
   mi<->rli

   @param[in,out]  mi                When new {mi,rli} are created,
                                     the reference is stored in *mi
   @param[in]      channel           The channel on which the change
                                     master was introduced.
*/
int add_new_channel(Master_info **mi, const char *channel) {
  DBUG_TRACE;

  int error = 0;
  Ident_name_check ident_check_status;

  /*
    Refuse to create a new channel if the repositories does not support this.
  */

  if (opt_mi_repository_id == INFO_REPOSITORY_FILE ||
      opt_rli_repository_id == INFO_REPOSITORY_FILE) {
    LogErr(ERROR_LEVEL,
           ER_RPL_SLAVE_NEW_MASTER_INFO_NEEDS_REPOS_TYPE_OTHER_THAN_FILE);
    error = ER_SLAVE_NEW_CHANNEL_WRONG_REPOSITORY;
    my_error(ER_SLAVE_NEW_CHANNEL_WRONG_REPOSITORY, MYF(0));
    goto err;
  }

  /*
    Return if max num of replication channels exceeded already.
  */

  if (!channel_map.is_valid_channel_count()) {
    error = ER_SLAVE_MAX_CHANNELS_EXCEEDED;
    my_error(ER_SLAVE_MAX_CHANNELS_EXCEEDED, MYF(0));
    goto err;
  }

  /*
    Now check the sanity of the channel name. It's length etc. The channel
    identifier is similar to table names. So, use  check_table_function.
  */
  if (channel) {
    ident_check_status = check_table_name(channel, strlen(channel));
  } else
    ident_check_status = Ident_name_check::WRONG;

  if (ident_check_status != Ident_name_check::OK) {
    error = ER_SLAVE_CHANNEL_NAME_INVALID_OR_TOO_LONG;
    my_error(ER_SLAVE_CHANNEL_NAME_INVALID_OR_TOO_LONG, MYF(0));
    goto err;
  }

  if (!((*mi) = Rpl_info_factory::create_mi_and_rli_objects(
            opt_mi_repository_id, opt_rli_repository_id, channel, false,
            &channel_map))) {
    error = ER_MASTER_INFO;
    my_error(ER_MASTER_INFO, MYF(0));
    goto err;
  }

err:

  return error;
}

/**
   Method used to check if the user is trying to update any other option for
   the change master apart from the MASTER_USER and MASTER_PASSWORD.
   In case user tries to update any other parameter apart from these two,
   this method will return error.

   @param  lex_mi structure that holds all change master options given on
           the change master command.

   @retval true - The CHANGE MASTER is updating a unsupported parameter for the
                  recovery channel.

   @retval false - Everything is fine. The CHANGE MASTER can execute with the
                   given option(s) for the recovery channel.
*/
static bool is_invalid_change_master_for_group_replication_recovery(
    const LEX_MASTER_INFO *lex_mi) {
  DBUG_TRACE;
  bool have_extra_option_received = false;

  /* Check if *at least one* receive/execute option is given on change master
   * command*/
  if (lex_mi->host || lex_mi->log_file_name || lex_mi->pos ||
      lex_mi->bind_addr || lex_mi->port || lex_mi->connect_retry ||
      lex_mi->server_id ||
      lex_mi->auto_position != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_verify_server_cert != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->heartbeat_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->retry_count_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_key || lex_mi->ssl_cert || lex_mi->ssl_ca ||
      lex_mi->ssl_capath || lex_mi->tls_version ||
      lex_mi->tls_ciphersuites != LEX_MASTER_INFO::UNSPECIFIED ||
      lex_mi->ssl_cipher || lex_mi->ssl_crl || lex_mi->ssl_crlpath ||
      lex_mi->repl_ignore_server_ids_opt == LEX_MASTER_INFO::LEX_MI_ENABLE ||
      lex_mi->relay_log_name || lex_mi->relay_log_pos ||
      lex_mi->sql_delay != -1 || lex_mi->public_key_path ||
      lex_mi->get_public_key != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->zstd_compression_level || lex_mi->compression_algorithm ||
      lex_mi->require_row_format != -1)
    have_extra_option_received = true;

  return have_extra_option_received;
}

/**
   Method used to check if the user is trying to update any other option for
   the change master apart from the PRIVILEGE_CHECKS_USER.
   In case user tries to update any other parameter apart from this one, this
   method will return error.

   @param  lex_mi structure that holds all change master options given on
           the change master command.

   @retval true - The CHANGE MASTER is updating a unsupported parameter for the
                  recovery channel.

   @retval false - Everything is fine. The CHANGE MASTER can execute with the
                   given option(s) for the recovery channel.
*/
static bool is_invalid_change_master_for_group_replication_applier(
    const LEX_MASTER_INFO *lex_mi) {
  DBUG_TRACE;
  bool have_extra_option_received = false;

  /* Check if *at least one* receive/execute option is given on change master
   * command*/
  if (lex_mi->host || lex_mi->user || lex_mi->password ||
      lex_mi->log_file_name || lex_mi->pos || lex_mi->bind_addr ||
      lex_mi->port || lex_mi->connect_retry || lex_mi->server_id ||
      lex_mi->auto_position != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_verify_server_cert != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->heartbeat_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->retry_count_opt != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->ssl_key || lex_mi->ssl_cert || lex_mi->ssl_ca ||
      lex_mi->ssl_capath || lex_mi->tls_version || lex_mi->ssl_cipher ||
      lex_mi->ssl_crl || lex_mi->ssl_crlpath ||
      lex_mi->repl_ignore_server_ids_opt == LEX_MASTER_INFO::LEX_MI_ENABLE ||
      lex_mi->relay_log_name || lex_mi->relay_log_pos ||
      lex_mi->sql_delay != -1 || lex_mi->public_key_path ||
      lex_mi->get_public_key != LEX_MASTER_INFO::LEX_MI_UNCHANGED ||
      lex_mi->zstd_compression_level || lex_mi->compression_algorithm ||
      lex_mi->require_row_format != -1)
    have_extra_option_received = true;

  return have_extra_option_received;
}

/**
  Entry point for the CHANGE MASTER command. Function
  decides to create a new channel or create an existing one.

  @param[in]        thd        the client thread that issued the command.

  @retval           true       fail
  @retval           false      success.
*/
bool change_master_cmd(THD *thd) {
  DBUG_TRACE;

  Master_info *mi = nullptr;
  LEX *lex = thd->lex;
  bool res = false;

  channel_map.wrlock();

  /* The slave must have been initialized to allow CHANGE MASTER statements */
  if (!is_slave_configured()) {
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
    res = true;
    goto err;
  }

  if (channel_map.is_group_replication_channel_name(lex->mi.channel, true)) {
    /*
      If the chosen name is for group_replication_applier channel we allow the
      channel creation based on the check as to which field is being updated.
    */
    LEX_MASTER_INFO *lex_mi = &thd->lex->mi;
    if (is_invalid_change_master_for_group_replication_applier(lex_mi)) {
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "CHANGE MASTER with the given parameters", lex->mi.channel);
      res = true;
      goto err;
    }

    /*
      group_replication_applier channel only has the SQL thread, the IO thread
      job is done by GR pipeline, which queues events into the relay log after
      going through certification.
      Thence for CHANGE MASTER execution pre-conditions we need to check if
      the full GR stack is stopped.
    */
    if (is_group_replication_running()) {
      my_error(ER_GRP_OPERATION_NOT_ALLOWED_GR_MUST_STOP, MYF(0));
      res = true;
      goto err;
    }
  }

  // If the channel being used is group_replication_recovery we allow the
  // channel creation based on the check as to which field is being updated.
  if (channel_map.is_group_replication_channel_name(lex->mi.channel) &&
      !channel_map.is_group_replication_channel_name(lex->mi.channel, true)) {
    LEX_MASTER_INFO *lex_mi = &thd->lex->mi;
    if (is_invalid_change_master_for_group_replication_recovery(lex_mi)) {
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "CHANGE MASTER with the given parameters", lex->mi.channel);
      res = true;
      goto err;
    }
  }

  /*
    Error out if number of replication channels are > 1 if FOR CHANNEL
    clause is not provided in the CHANGE MASTER command.
  */
  if (!lex->mi.for_channel && channel_map.get_num_instances() > 1) {
    my_error(ER_SLAVE_MULTIPLE_CHANNELS_CMD, MYF(0));
    res = true;
    goto err;
  }

  /* Get the Master_info of the channel */
  mi = channel_map.get_mi(lex->mi.channel);

  /* create a new channel if doesn't exist */
  if (!mi && strcmp(lex->mi.channel, channel_map.get_default_channel())) {
    /* The mi will be returned holding mi->channel_lock for writing */
    if (add_new_channel(&mi, lex->mi.channel)) goto err;
  }

  if (mi) {
    bool configure_filters = !Master_info::is_configured(mi);

    if (!(res = change_master(thd, mi, &thd->lex->mi))) {
      /*
        If the channel was just created or not configured before this
        "CHANGE MASTER", we need to configure rpl_filter for it.
      */
      if (configure_filters) {
        if ((res = Rpl_info_factory::configure_channel_replication_filters(
                 mi->rli, lex->mi.channel)))
          goto err;
      }

      /*
        Issuing deprecation warnings after the change (we make
        sure that we don't issue warning if there is an error).
      */
      issue_deprecation_warnings_for_channel(thd);

      my_ok(thd);
    }
  } else {
    /*
       Even default channel does not exist. So issue a previous
       backward compatible  error message (till 5.6).
       @TODO: This error message shall be improved.
    */
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
  }

err:
  channel_map.unlock();

  return res;
}

/**
  Check if there is any slave SQL config conflict.

  @param[in] rli The slave's rli object.

  @return 0 is returned if there is no conflict, otherwise 1 is returned.
 */
static int check_slave_sql_config_conflict(const Relay_log_info *rli) {
  int channel_mts_submode, slave_parallel_workers;

  if (rli) {
    channel_mts_submode = rli->channel_mts_submode;
    slave_parallel_workers = rli->opt_slave_parallel_workers;
  } else {
    /*
      When the slave is first initialized, we collect the values from the
      command line options
    */
    channel_mts_submode = get_mts_parallel_option();
    slave_parallel_workers = opt_mts_slave_parallel_workers;
  }

  const auto slave_preserve_commit_order = get_slave_preserve_commit_order();
  if (slave_preserve_commit_order && slave_parallel_workers > 0) {
    if (channel_mts_submode == MTS_PARALLEL_TYPE_DB_NAME) {
      my_error(ER_DONT_SUPPORT_SLAVE_PRESERVE_COMMIT_ORDER, MYF(0),
               "when slave_parallel_type is DATABASE");
      return ER_DONT_SUPPORT_SLAVE_PRESERVE_COMMIT_ORDER;
    }
  }

  if (rli) {
    const char *channel = const_cast<Relay_log_info *>(rli)->get_channel();
    if (slave_parallel_workers > 0 &&
        (channel_mts_submode != MTS_PARALLEL_TYPE_LOGICAL_CLOCK ||
         (channel_mts_submode == MTS_PARALLEL_TYPE_LOGICAL_CLOCK &&
          !slave_preserve_commit_order)) &&
        channel_map.is_group_replication_channel_name(channel, true)) {
      my_error(ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED, MYF(0),
               "START SLAVE SQL_THREAD when SLAVE_PARALLEL_WORKERS > 0 "
               "and SLAVE_PARALLEL_TYPE != LOGICAL_CLOCK "
               "or SLAVE_PRESERVE_COMMIT_ORDER != ON",
               channel);
      return ER_SLAVE_CHANNEL_OPERATION_NOT_ALLOWED;
    }
  }

  return 0;
}

// This function will generate the string containing current master host and
// port info if available.
std::string get_active_master_info() {
  std::string str_ptr;

  channel_map.rdlock();
  Master_info *mi = channel_map.get_default_channel_mi();

  if (Master_info::is_configured(mi)) {
    str_ptr = ". Current master_host: ";
    str_ptr += mi->host;
    str_ptr += ", master_port: ";
    str_ptr += std::to_string(mi->port);
  }

  if ((Master_info::is_configured(mi) ||
       skip_master_info_check_for_read_only_error_msg_extra) &&
      opt_read_only_error_msg_extra && opt_read_only_error_msg_extra[0]) {
    str_ptr += ". ";
    str_ptr += opt_read_only_error_msg_extra;
  }

  channel_map.unlock();
  return str_ptr;
}

/* counter for the number of BI inconsistencies found */
ulong before_image_inconsistencies = 0;
/* table_name -> last gtid for BI inconsistencies */
std::unordered_map<std::string, before_image_mismatch> bi_inconsistencies;
/* mutex for counter and map */
std::mutex bi_inconsistency_lock;

void update_before_image_inconsistencies(
    const before_image_mismatch &mismatch) {
  const std::lock_guard<std::mutex> lock(bi_inconsistency_lock);
  ++before_image_inconsistencies;
  bi_inconsistencies[mismatch.table] = mismatch;
}

ulong get_num_before_image_inconsistencies() {
  const std::lock_guard<std::mutex> lock(bi_inconsistency_lock);
  return before_image_inconsistencies;
}

/**
  @} (end of group Replication)
*/
