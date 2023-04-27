/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_HANDLER_H
#define RPL_HANDLER_H

#include <sys/types.h>

#include <mysql_version.h>
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"  // free_root
#include "mysql/components/services/mysql_rwlock_bits.h"
#include "mysql/components/services/psi_rwlock_bits.h"
#include "mysql/psi/mysql_rwlock.h"
#include "mysql/raft_listener_queue_if.h"
#include "mysql/raft_optype.h"  // RaftReplicateMsgOpType
#include "mysql/raft_replication.h"
#include "replication.h"
#include "sql/sql_list.h"        // List
#include "sql/sql_plugin_ref.h"  // plugin_ref

class Master_info;
class String;
class THD;
struct Binlog_relay_IO_observer;
struct Binlog_relay_IO_param;
struct Binlog_storage_observer;
struct Binlog_transmit_observer;
struct Server_state_observer;
struct Trans_observer;
struct TABLE_LIST;

class Observer_info {
 public:
  void *observer;
  st_plugin_int *plugin_int;
  plugin_ref plugin;

  Observer_info(void *ob, st_plugin_int *p);
};

class Delegate {
 public:
  typedef List<Observer_info> Observer_info_list;
  typedef List_iterator<Observer_info> Observer_info_iterator;

  int add_observer(void *observer, st_plugin_int *plugin) {
    int ret = false;
    if (!inited) return true;
    write_lock();
    Observer_info_iterator iter(observer_info_list);
    Observer_info *info = iter++;
    while (info && info->observer != observer) info = iter++;
    if (!info) {
      info = new Observer_info(observer, plugin);
      if (!info || observer_info_list.push_back(info, &memroot)) ret = true;
    } else
      ret = true;
    unlock();
    return ret;
  }

  int remove_observer(void *observer) {
    int ret = false;
    if (!inited) return true;
    write_lock();
    Observer_info_iterator iter(observer_info_list);
    Observer_info *info = iter++;
    while (info && info->observer != observer) info = iter++;
    if (info) {
      iter.remove();
      delete info;
    } else
      ret = MYSQL_REPLICATION_OBSERVER_NOT_FOUND;
    unlock();
    return ret;
  }

  inline Observer_info_iterator observer_info_iter() {
    return Observer_info_iterator(observer_info_list);
  }

  inline bool is_empty() {
    DBUG_PRINT("debug", ("is_empty: %d", observer_info_list.is_empty()));
    return observer_info_list.is_empty();
  }

  inline int read_lock() {
    if (!inited) return true;
    return mysql_rwlock_rdlock(&lock);
  }

  inline int write_lock() {
    if (!inited) return true;
    return mysql_rwlock_wrlock(&lock);
  }

  inline int unlock() {
    if (!inited) return true;
    return mysql_rwlock_unlock(&lock);
  }

  inline bool is_inited() { return inited; }

  explicit Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
      PSI_rwlock_key key
#endif
  );

  ~Delegate() {
    inited = false;
    mysql_rwlock_destroy(&lock);
    free_root(&memroot, MYF(0));
  }

 private:
  Observer_info_list observer_info_list;
  mysql_rwlock_t lock;
  MEM_ROOT memroot;
  bool inited;
};

#ifdef HAVE_PSI_RWLOCK_INTERFACE
extern PSI_rwlock_key key_rwlock_Trans_delegate_lock;
#endif

class Binlog_cache_storage;

class Trans_delegate : public Delegate {
 public:
  Trans_delegate()
      : Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
            key_rwlock_Trans_delegate_lock
#endif
        ) {
  }

  typedef Trans_observer Observer;

  int before_dml(THD *thd, int &result);
  int before_commit(THD *thd, bool all, Binlog_cache_storage *trx_cache_log,
                    Binlog_cache_storage *stmt_cache_log,
                    ulonglong cache_log_max_size, bool is_atomic_ddl);
  int before_rollback(THD *thd, bool all);
  int after_commit(THD *thd, bool all);
  int after_rollback(THD *thd, bool all);
  int trans_begin(THD *thd, int &result);
};

#ifdef HAVE_PSI_RWLOCK_INTERFACE
extern PSI_rwlock_key key_rwlock_Server_state_delegate_lock;
#endif

class Server_state_delegate : public Delegate {
 public:
  Server_state_delegate()
      : Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
            key_rwlock_Server_state_delegate_lock
#endif
        ) {
  }

  typedef Server_state_observer Observer;
  int before_handle_connection(THD *thd);
  int before_recovery(THD *thd);
  int after_engine_recovery(THD *thd);
  int after_recovery(THD *thd);
  int before_server_shutdown(THD *thd);
  int after_server_shutdown(THD *thd);
  int after_dd_upgrade_from_57(THD *thd);
};

#ifdef HAVE_PSI_RWLOCK_INTERFACE
extern PSI_rwlock_key key_rwlock_Binlog_storage_delegate_lock;
#endif

class Binlog_storage_delegate : public Delegate {
 public:
  Binlog_storage_delegate()
      : Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
            key_rwlock_Binlog_storage_delegate_lock
#endif
        ) {
  }

  typedef Binlog_storage_observer Observer;
  int after_flush(THD *thd, const char *log_file, my_off_t log_pos);
  int after_sync(THD *thd, const char *log_file, my_off_t log_pos);
};

#ifdef HAVE_PSI_RWLOCK_INTERFACE
extern PSI_rwlock_key key_rwlock_Binlog_transmit_delegate_lock;
#endif

class Binlog_transmit_delegate : public Delegate {
 public:
  Binlog_transmit_delegate()
      : Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
            key_rwlock_Binlog_transmit_delegate_lock
#endif
        ) {
  }

  typedef Binlog_transmit_observer Observer;
  int transmit_start(THD *thd, ushort flags, const char *log_file,
                     my_off_t log_pos, bool *observe_transmission);
  int transmit_stop(THD *thd, ushort flags);
  int reserve_header(THD *thd, ushort flags, String *packet);
  int before_send_event(THD *thd, ushort flags, String *packet,
                        const char *log_file, my_off_t log_pos);
  int after_send_event(THD *thd, ushort flags, String *packet,
                       const char *skipped_log_file, my_off_t skipped_log_pos);
  int after_reset_master(THD *thd, ushort flags);
};

#ifdef HAVE_PSI_RWLOCK_INTERFACE
extern PSI_rwlock_key key_rwlock_Binlog_relay_IO_delegate_lock;
#endif

class Binlog_relay_IO_delegate : public Delegate {
 public:
  Binlog_relay_IO_delegate()
      : Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
            key_rwlock_Binlog_relay_IO_delegate_lock
#endif
        ) {
  }

  typedef Binlog_relay_IO_observer Observer;
  int thread_start(THD *thd, Master_info *mi);
  int thread_stop(THD *thd, Master_info *mi);
  int applier_start(THD *thd, Master_info *mi);
  int applier_stop(THD *thd, Master_info *mi, bool aborted);
  int before_request_transmit(THD *thd, Master_info *mi, ushort flags);
  int after_read_event(THD *thd, Master_info *mi, const char *packet, ulong len,
                       const char **event_buf, ulong *event_len);
  int after_queue_event(THD *thd, Master_info *mi, const char *event_buf,
                        ulong event_len, bool synced);
  int after_reset_slave(THD *thd, Master_info *mi);
  int applier_log_event(THD *thd, int &out);

 private:
  void init_param(Binlog_relay_IO_param *param, Master_info *mi);
};

#ifdef HAVE_PSI_INTERFACE
extern PSI_rwlock_key key_rwlock_Raft_replication_delegate_lock;
#endif

class Raft_replication_delegate : public Delegate {
 public:
  Raft_replication_delegate()
      : Delegate(
#ifdef HAVE_PSI_INTERFACE
            key_rwlock_Raft_replication_delegate_lock
#endif
        ) {
  }

  typedef Raft_replication_observer Observer;
  int before_flush(THD *thd, IO_CACHE *io_cache,
                   RaftReplicateMsgOpType op_type);

  int before_commit(THD *thd);

  int setup_flush(THD *thd, Observer::st_setup_flush_arg *arg);

  int before_shutdown(THD *thd);
  int register_paths(THD *thd, const std::string &s_uuid, uint32_t server_id,
                     const std::string &wal_dir_parent,
                     const std::string &log_dir_parent,
                     const std::string &raft_log_path_prefix,
                     const std::string &s_hostname, uint64_t port);
  int after_commit(THD *thd);
  int purge_logs(THD *thd, uint64_t file_ext);
  int show_raft_status(
      THD *thd,
      std::vector<std::pair<std::string, std::string>> *var_value_pairs);
};

int delegates_init();
void delegates_destroy();

extern Trans_delegate *transaction_delegate;
extern Binlog_storage_delegate *binlog_storage_delegate;
extern Server_state_delegate *server_state_delegate;
extern Binlog_transmit_delegate *binlog_transmit_delegate;
extern Binlog_relay_IO_delegate *binlog_relay_io_delegate;
extern Raft_replication_delegate *raft_replication_delegate;

/*
  if there is no observers in the delegate, we can return 0
  immediately.
*/
#define RUN_HOOK(group, hook, args) \
  (group##_delegate->is_empty() ? 0 : group##_delegate->hook args)

/*
  This is same as RUN_HOOK, but return 1 if there are no observers
*/
#define RUN_HOOK_STRICT(group, hook, args) \
  (group##_delegate->is_empty() ? 1 : group##_delegate->hook args)

#define NO_HOOK(group) (group##_delegate->is_empty())

int launch_hook_trans_begin(THD *thd, TABLE_LIST *table);

#endif /* RPL_HANDLER_H */

class RaftListenerQueue : public RaftListenerQueueIf {
 public:
  explicit RaftListenerQueue() { inited_.store(false); }

  ~RaftListenerQueue();

  /* Init the queue, this will create a listening thread for this queue
   *
   * @return 0 on success, 1 on error
   */
  int init();

  /* Deinit the queue. This will add an exit event into the queue which will
   * be picked up by any listening thread and it will stop listening */
  void deinit();

  /* Add an element to the queue. This will signal any listening threads
   * after adding the element to the queue
   *
   * @param element QueueElement to add to queue
   *
   * @return 0 on success, 1 on error
   */
  int add(QueueElement element);

  /* Get an element from the queue. This will block if there are no elements
   * in the queue to be processed
   *
   * @return QueueElement to be processed next
   */
  QueueElement get();

 private:
  std::mutex queue_mutex_;            // Lock guarding the queue
  std::condition_variable queue_cv_;  // CV to wait and signal
  std::queue<QueueElement> queue_;    // The queue of events to be processed

  std::mutex init_mutex_;    // Mutex to guard against init and deinit races
  std::atomic_bool inited_;  // Has this been inited?
};

// A container to help passing raft related rotation info through
// the code. This will be created by the handler and then passed along.
struct RaftRotateInfo {
  // The payload of config change that contains before configuration
  // and after configuration. Generated with help from Raft by plugin.
  std::string config_change;
  // The name of the new log file after rotation. In Raft case, it
  // contains the name of the leader's next log.
  std::string new_log_ident;
  // The starting position of the next file. Typically 4.
  ulonglong pos = 0;
  // Is this rotation to get consensus on a NO-OP event after winning
  // election.
  bool noop = false;
  // if true, the rotate event has already been appended to relay log
  bool post_append = false;
  // If true, the server will need to replicate and get consensus on
  // rotate event.
  bool rotate_via_raft = false;
  // goes together with config_change above, if true, this is a
  // rotation to be initiated by server to get consensus on a
  // config change (add/remove/modify of the ring)
  bool config_change_rotate = false;
  // This is the opid of the rotate event which is either
  // passed in by plugin or obtained from before_flush.
  // During rotation of raft logs, this is put into Metadata event
  // as previous opid
  std::pair<int64_t, int64_t> rotate_opid = std::make_pair(0, 0);
};
