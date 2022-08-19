/* Copyright (c) 2008, 2014, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   51 Franklin Street, Suite 500, Boston, MA 02110-1335 USA */

#ifndef RAFT_REPLICATION_H
#define RAFT_REPLICATION_H

// For RaftReplicateMsgOpType
#include <string>
#include "raft_optype.h"

#ifdef INCL_DEFINED_IN_MYSQL_SERVER
extern bool enable_raft_plugin;
#endif
class RaftListenerQueueIf;

#if MYSQL_VERSION_ID >= 80004
struct IO_CACHE;
struct mysql_cond_t;
struct mysql_mutex_t;
#else
struct st_io_cache;
typedef struct st_io_cache IO_CACHE;
struct st_mysql_cond;
typedef struct st_mysql_cond mysql_cond_t;
struct st_mysql_mutex;
typedef struct st_mysql_mutex mysql_mutex_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
  Raft replication observer parameter
*/
typedef struct Raft_replication_param {
  uint32_t server_id = 0;
  const char *host_or_ip = nullptr;

  // Raft term and index. Set by the before_flush hook and used in subsequent
  // hooks for ordered_commit or file rotation
  int64_t term = -1;
  int64_t index = -1;

  // The max file extension that is to be deleted. Passed as a parameter to
  // purge_logs hook
  uint64_t purge_file_ext = 0;

  // The file that is safe to be deleted. Plugin will set it in purge_logs hook
  std::string purge_file;
} Raft_replication_param;

/**
   Observe special events for Raft replication to work
*/
typedef struct Raft_replication_observer {
  uint32_t len;

  // Arguments passed to setup_flush hooks, these will either correspond to
  // binlog or relay log
  struct st_setup_flush_arg {
    // IO_CACHE for the log
    IO_CACHE *log_file_cache = nullptr;

    // Log prefix e.g. binary-log, apply-log
    const char *log_prefix = nullptr;
    // Log name
    const char *log_name = nullptr;
    // Log number
    ulong *cur_log_ext = nullptr;

    // Log name for the last log
    char *endpos_log_name = nullptr;
    // Last position of in the last log
    unsigned long long *endpos = nullptr;
    // Number of times changes to endpos was signalled
    unsigned long *signal_cnt = nullptr;

    // High level mutex protecting the log access
    mysql_mutex_t *lock_log = nullptr;
    // Mutex protecting index file access
    mysql_mutex_t *lock_index = nullptr;
    // Mutex protecting end pos
    mysql_mutex_t *lock_end_pos = nullptr;
    // Condition variable for signalling end pos update
    mysql_cond_t *update_cond = nullptr;

    // Is this a relay log?
    bool is_relay_log = false;
    // Extra context
    int context = 0;
  };

  /**
     This callback is called before transaction commit
     and after binlog sync.

     For both non-transactional tables and transactional
     tables this is called after binlog sync.

     @param param The parameter for transaction observers

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*before_commit)(Raft_replication_param *param);

  /**
     This callback is called before events of a txn are written to binlog file

     @param param Observer common parameter
     @param cache IO_CACHE containing binlog events for the txn
     @param op_type The type of operation for which before_flush is called

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*before_flush)(Raft_replication_param *param, IO_CACHE *cache,
                      RaftReplicateMsgOpType op_type);

  /**
     This callback is called once upfront to setup the appropriate
     binlog file, io_cache and its mutexes

     @param arg st_setup_flush_arg structure contains log_file_cache,
     log_prefix, log_namem etc

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*setup_flush)(Raft_replication_observer::st_setup_flush_arg *arg);

  /**
   * This callback is invoked by the server to gracefully shutdown the
   * Raft threads
   */
  int (*before_shutdown)();

  /**
   * @param raft_listener_queue - the listener queue in which to add requests
   * @param s_uuid - the uuid of the server to be used as the INSTANCE UUID
   *                 in Raft
   * @param server_id - mysqld server id
   * @param wal_dir_parent - the parent directory under which raft will create
   * config metadata
   * @param log_dir_parent - the parent directory under which raft will create
   * metric logs
   * @param raft_log_path_prefix - the prefix with the dirname path which tells
   * @param s_hostname - the proper hostname of server which can be used in
   * plugin
   * @param port - the port of the server
   * raft where to find raft binlogs.
   */
  int (*register_paths)(RaftListenerQueueIf *raft_listener_queue,
                        const std::string &s_uuid, uint32_t server_id,
                        const std::string &wal_dir_parent,
                        const std::string &log_dir_parent,
                        const std::string &raft_log_path_prefix,
                        const std::string &s_hostname, uint64_t port);

  /**
     This callback is called after transaction commit to engine

     @param param The parameter for the observers

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*after_commit)(Raft_replication_param *param);

  /**
     This callback is called before purging binary logs. This is a way for
     raft plugin to identify the file that could be safely deleted based on
     its state for duarbility and peers that are still catching up. Server
     provides the max extension of the file that it wants to purge. The
     plugin returns the max file name that is safe to be deleted

     @param param The parameter for the observers

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*purge_logs)(Raft_replication_param *param);

  /**
     This callback is called to get a comprehensive status of RAFT
     @param var_value_pairs vector of status vars value pairs

     @retval 0 Sucess
     @retval 1 Failure
  */
  int (*show_raft_status)(
      std::vector<std::pair<std::string, std::string>> *var_value_pairs);
} Raft_replication_observer;

// Finer grained error code during deregister of observer
// Observer was not present in delegate (i.e. not
// previously added to delegate ), and should be safe
// to ignore.
#define MYSQL_REPLICATION_OBSERVER_NOT_FOUND 2

/**
   Register a Raft replication observer

   @param observer The raft replication observer to register
   @param p pointer to the internal plugin structure

   @retval 0 Sucess
   @retval 1 Observer already exists
*/
int register_raft_replication_observer(Raft_replication_observer *observer,
                                       void *p);

/**
   Unregister a raft replication observer

   @param observer The raft observer to unregister
   @param p pointer to the internal plugin structure

   @retval 0 Sucess
   @retval 1 Observer not exists
*/
int unregister_raft_replication_observer(Raft_replication_observer *observer,
                                         void *p);

/**
 * Mark this GTID as logged in the rli and sets the master_log_file and
 * master_log_pos in mi. Also, flushes the master.info file.
 * @retval 0 Success
 * @retval 1 Some failure
 */
int update_rli_and_mi(
    const std::string &gtid_s,
    const std::pair<const std::string, unsigned long long> &master_log_pos);

void signal_semi_sync_ack(const std::string &file_num, uint file_pos);

/*
 * An enum to control what kind of registrations the
 * plugin needs from server.
 * Currently only 2 exist.
 * RAFT_REGISTER_LOCKS - BinlogWrapper related
 * RAFT_REGISTER_PATHS - paths and ports for initial raft setup
 */
enum Raft_Registration_Item {
  RAFT_REGISTER_LOCKS = 0,
  RAFT_REGISTER_PATHS = 1
};

/**
    Ask the mysqld server to immediately register the binlog and relay
    log files.

    Eventually instead of setup_flush in both these observers we will have
    a Raft specific delegate and observer
    @param item whether to register locks and io_caches for binlog wrapper
           or register paths for initial setup
*/
int ask_server_to_register_with_raft(Raft_Registration_Item item);

#ifdef __cplusplus
}
#endif
#endif /* RAFT_REPLICATION_H */
