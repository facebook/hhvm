#ifndef BINLOG_H_INCLUDED
/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#define BINLOG_H_INCLUDED

#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <atomic>
#include <limits>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "libbinlogevents/include/binlog_event.h"  // enum_binlog_checksum_alg
#include "m_string.h"                              // llstr
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sharedlib.h"
#include "my_sys.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_cond_bits.h"
#include "mysql/components/services/psi_file_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"  // Item_result
#include "rpl_gtid.h"
#include "sql/mysqld.h"  // key_hlc_wait_cond + key_hlc_wait_mutex
#include "sql/rpl_commit_stage_manager.h"
#include "sql/rpl_trx_tracking.h"
#include "sql/sql_class.h"         // THD
#include "sql/tc_log.h"            // TC_LOG
#include "sql/transaction_info.h"  // Transaction_ctx
#include "thr_mutex.h"

class Format_description_log_event;
class Gtid_monitoring_info;
class Gtid_set;
class Ha_trx_info;
class Incident_log_event;
class Log_event;
class Master_info;
class Relay_log_info;
class Rows_log_event;
class Metadata_log_event;
class Sid_map;
class THD;
class Transaction_boundary_parser;
class binlog_cache_data;
class user_var_entry;
class Binlog_cache_storage;

struct Gtid;
struct snapshot_info_st;
struct RaftRotateInfo;

typedef int64 query_id_t;

using log_file_name_container = std::list<std::string>;
using database_hlc_container =
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>>;

/*
  Maximum unique log filename extension.
  Note: setting to 0x7FFFFFFF due to atol windows
        overflow/truncate.
 */
#define MAX_LOG_UNIQUE_FN_EXT 0x7FFFFFFF

/*
  Maximum allowed unique log filename extension for
  RESET MASTER TO command - 2 Billion
 */
#define MAX_ALLOWED_FN_EXT_RESET_MASTER 2000000000

struct Binlog_user_var_event {
  user_var_entry *user_var_event;
  char *value;
  ulong length;
  Item_result type;
  uint charset_number;
  bool unsigned_flag;
};

extern latency_histogram histogram_raft_trx_wait;

extern char *histogram_step_size_binlog_fsync;
extern int opt_histogram_step_size_binlog_group_commit;
extern latency_histogram histogram_binlog_fsync;
extern counter_histogram histogram_binlog_group_commit;

/* The enum defining the server's action when a trx fails inside ordered commit
 * due to an error related to consensus (raft plugin) */
enum enum_commit_consensus_error_actions {
  /* Transactions that fail in ordered commit will be rolled back.
   * Currently all trxs in the group will be rolled back when the leader thread
   * of the group fails. An optimization to just rollback the failing trx is
   * left as a TODO */
  ROLLBACK_TRXS_IN_GROUP = 0,
  /* Ignore consensus errors and proceed as usual. Might be useful as an
   * override in some cases like consensus plugin bugs, easier rollouts, full
   * region failures etc */
  IGNORE_COMMIT_CONSENSUS_ERROR = 1,
  INVALID_COMMIT_CONSENSUS_ERROR_ACTION
};

enum enum_raft_signal_async_dump_threads_options {
  AFTER_CONSENSUS = 0,
  AFTER_ENGINE_COMMIT = 1,
  INVALID_OPTION
};

/* log info errors */
#define LOG_INFO_EOF -1
#define LOG_INFO_IO -2
#define LOG_INFO_INVALID -3
#define LOG_INFO_SEEK -4
#define LOG_INFO_MEM -6
#define LOG_INFO_FATAL -7
#define LOG_INFO_IN_USE -8
#define LOG_INFO_EMFILE -9

/* bitmap to MYSQL_BIN_LOG::close() */
#define LOG_CLOSE_INDEX 1
#define LOG_CLOSE_TO_BE_OPENED 2
#define LOG_CLOSE_STOP_EVENT 4

bool block_all_dump_threads();
void unblock_all_dump_threads();

/*
  Note that we destroy the lock mutex in the destructor here.
  This means that object instances cannot be destroyed/go out of scope
  until we have reset thd->current_linfo to NULL;
 */
struct LOG_INFO {
  char log_file_name[FN_REFLEN] = {0};
  my_off_t index_file_offset, index_file_start_offset;
  my_off_t pos;
  bool fatal;       // if the purge happens to give us a negative offset
  int entry_index;  // used in purge_logs(), calculatd in find_log_pos().
  int encrypted_header_size;
  bool is_relay_log;         // is this info pointing to a relay log?
  bool is_used_by_dump_thd;  // is this info being used by a dump thread?
  LOG_INFO(bool relay_log = false, bool used_by_dump_thd = false)
      : index_file_offset(0),
        index_file_start_offset(0),
        pos(0),
        fatal(false),
        entry_index(0),
        encrypted_header_size(0),
        is_relay_log(relay_log),
        is_used_by_dump_thd(used_by_dump_thd) {
    memset(log_file_name, 0, FN_REFLEN);
  }
};

/**
 * A class abstracting a hybrid logical clock. Some important aspects of this
 * clock:
 * 1. 64 bit unsigned integer is used to track nanosecond precision
 *    ticks (this should suffice well into 2500 AD)
 * 2. There is no explicit 'Logical' component. The rationale is that it is not
 *    common to generate 'events' frequently at a rate of greater than 1 per
 *    nanosecond. If we do hit this ocassionally, then this clock simply
 *    increments the current wall clock by 1 and sets that as the current
 *    nanosecond time. The wall clock should eventually catch up with the
 *    internal nanosecond clock.
 */
class HybridLogicalClock {
 public:
  explicit HybridLogicalClock(uint64_t start) { current_ = start; }

  HybridLogicalClock() { current_ = 0; }

  ~HybridLogicalClock() = default;

  /**
   * Get the next hlc value and update the internal clock to match the next hlc
   * value. By definition of this clock (see above), the next value is
   * max(current+1, wall-clock). This is called when we want to assign a HLC
   * timestamp to a event.
   *
   * @return   The next HLC
   */
  uint64_t get_next();

  /**
   * Get the current value of the HLC
   *
   * @return   The current value of internal nanosecond clock
   */
  uint64_t get_current();

  /**
   * Update the internal clock to a value greater than or equal to minimum_hlc
   * and return the updated value. This method is used to synchronize HLC
   * clocks across different instances OR to set a minimum bound on the next
   * hlc by an external entity. Returns the value of the updated internal hlc
   *
   * @param    minimum_hlc - The minimum/lower bound on the HLC
   *
   * @return   The hlc after setting the minimum bound
   */
  uint64_t update(uint64_t minimum_hlc);

  /**
   * Update the applied HLC for specified databases
   *
   * @param  databases - the databases for which hlc needs to be updated
   * @param  applied_hlc - Applied HLC
   */
  void update_database_hlc(const database_container &databases,
                           uint64_t applied_hlc);

  /**
   * Get the applied hlc for all the database that is being tracked by this
   * clock
   *
   * @param  [out] A map of database->applied_hlc
   */
  database_hlc_container get_database_hlc() const;

  /**
   * Get the applied hlc for the specified database
   *
   * @param  [in] A database name to select HLC for
   *
   * @return The HLC for the specified database, or 0 if not found
   */
  uint64_t get_selected_database_hlc(const std::string &database);

  /**
   * Verify if the given HLC value is 'valid', by which it isn't 0 or intmax
   *
   * @param [in] The HLC value to validate
   */
  static bool is_valid_hlc(uint64_t hlc) {
    return hlc != 0 && hlc != ULLONG_MAX;
  }

  /**
   * Clear database HLC map
   */
  void clear_database_hlc();

  /**
   * Block the THD if the query attribute specified HLC isn't
   * present in the engine according to database_applied_hlc_
   */
  bool wait_for_hlc_applied(THD *thd);

  /**
   * Check that lower HLC bound requirements are satisfied for
   * insert/update/delete queries.
   */
  bool check_hlc_bound(THD *thd);

 private:
  // nanosecond precision internal clock
  std::atomic<uint64_t> current_;

  // Per-database entry to track the list of waiting queries
  class DatabaseEntry {
   public:
    DatabaseEntry() {
#ifdef HAVE_PSI_INTERFACE
      mysql_mutex_init(key_hlc_wait_mutex, &mutex_, nullptr);
      mysql_cond_init(key_hlc_wait_cond, &cond_);
#else
      mysql_mutex_init(0, &mutex_, nullptr);
      mysql_cond_init(0, &cond_);
#endif
    }

    ~DatabaseEntry() {
      mysql_mutex_destroy(&mutex_);
      mysql_cond_destroy(&cond_);
    }

    void update_hlc(uint64_t applied_hlc);

    bool wait_for_hlc(THD *thd, uint64_t requested_hlc, uint64_t timeout_ms);

    uint64_t max_applied_hlc() const { return max_applied_hlc_; }
    uint64_t num_out_of_order_hlc() const { return num_out_of_order_hlc_; }

   private:
    mysql_mutex_t mutex_;
    mysql_cond_t cond_;
    std::atomic<uint64_t> max_applied_hlc_{0};
    std::atomic<uint64_t> num_out_of_order_hlc_{0};
  };

  std::shared_ptr<DatabaseEntry> getEntry(const std::string &database) {
    std::shared_ptr<DatabaseEntry> entry;
    auto entryIt = database_map_.find(database);
    if (entryIt == database_map_.end()) {
      entry = std::make_shared<DatabaseEntry>();
      database_map_.emplace(database, entry);
    } else {
      entry = entryIt->second;
    }
    return entry;
  }

  /**
   * A map of applied HLC for each database. The key is the name of the database
   * and the value is the applied_hlc for that database. Applied HLC is the HLC
   * of the last known trx that was applied (committed) to the engine
   */
  std::unordered_map<std::string, std::shared_ptr<DatabaseEntry>> database_map_;
  mutable std::mutex database_map_lock_;
};

struct st_filenum_pos {
  uint file_num = 0;
  uint pos = 0;

  static const uint max_pos = std::numeric_limits<uint>::max();

  st_filenum_pos() = default;

  st_filenum_pos(uint file_num, uint pos) {
    this->file_num = file_num;
    this->pos = pos;
  }

  int cmp(const st_filenum_pos &other) const {
    if (file_num == other.file_num && pos == other.pos) return 0;
    if (file_num == other.file_num) return pos < other.pos ? -1 : 1;
    return file_num < other.file_num ? -1 : 1;
  }

  bool operator==(const st_filenum_pos &other) const { return cmp(other) == 0; }
  bool operator<(const st_filenum_pos &other) const { return cmp(other) < 0; }
  bool operator>(const st_filenum_pos &other) const { return cmp(other) > 0; }
  bool operator<=(const st_filenum_pos &other) const { return cmp(other) <= 0; }
  bool operator>=(const st_filenum_pos &other) const { return cmp(other) >= 0; }
};

/*
  TODO use mmap instead of IO_CACHE for binlog
  (mmap+fsync is two times faster than write+fsync)
*/
class MYSQL_BIN_LOG : public TC_LOG {
 public:
  class Binlog_ofile;
  enum enum_read_gtids_from_binlog_status {
    GOT_GTIDS,
    GOT_PREVIOUS_GTIDS,
    NO_GTIDS,
    ERROR,
    TRUNCATED
  };

 private:
  enum enum_log_state { LOG_OPENED, LOG_CLOSED, LOG_TO_BE_OPENED };

  /* LOCK_log is inited by init_pthread_objects() */
  mysql_mutex_t LOCK_log;
  char *name;
  char log_file_name[FN_REFLEN];
  char db[NAME_LEN + 1];
  bool write_error, inited;
  Binlog_ofile *m_binlog_file;

  /** Instrumentation key to use for file io in @c log_file */
  PSI_file_key m_log_file_key;
  /** The instrumentation key to use for @ LOCK_log. */
  PSI_mutex_key m_key_LOCK_log;
  /** The instrumentation key to use for @ LOCK_index. */
  PSI_mutex_key m_key_LOCK_index;
  /** The instrumentation key to use for @ LOCK_binlog_end_pos. */
  PSI_mutex_key m_key_LOCK_binlog_end_pos;

  PSI_mutex_key m_key_COND_done;

  PSI_mutex_key m_key_LOCK_commit_queue;
  PSI_mutex_key m_key_LOCK_done;
  PSI_mutex_key m_key_LOCK_flush_queue;
  PSI_mutex_key m_key_LOCK_sync_queue;
  /** The instrumentation key to use for @ LOCK_commit. */
  PSI_mutex_key m_key_LOCK_commit;
  /** The instrumentation key to use for @ LOCK_sync. */
  PSI_mutex_key m_key_LOCK_sync;
  /** The instrumentation key to use for @ LOCK_xids. */
  PSI_mutex_key m_key_LOCK_xids;
  PSI_mutex_key m_key_LOCK_non_xid_trxs;
  PSI_mutex_key m_key_LOCK_lost_gtids_for_tailing;
  /** The instrumentation key to use for @ update_cond. */
  PSI_cond_key m_key_update_cond;
  /** The instrumentation key to use for @ prep_xids_cond. */
  PSI_cond_key m_key_prep_xids_cond;
  /** The instrumentation key to use for @ non_xid_trxs_cond. */
  PSI_cond_key m_key_non_xid_trxs_cond;
  /** The instrumentation key to use for opening the log file. */
  PSI_file_key m_key_file_log;
  /** The instrumentation key to use for opening the log index file. */
  PSI_file_key m_key_file_log_index;
  /** The instrumentation key to use for opening a log cache file. */
  PSI_file_key m_key_file_log_cache;
  /** The instrumentation key to use for opening a log index cache file. */
  PSI_file_key m_key_file_log_index_cache;

  /* POSIX thread objects are inited by init_pthread_objects() */
  mysql_mutex_t LOCK_index;
  mysql_mutex_t LOCK_commit;
  mysql_mutex_t LOCK_sync;
  mysql_mutex_t LOCK_binlog_end_pos;
  mysql_mutex_t LOCK_xids;
  mysql_mutex_t LOCK_non_xid_trxs;
  mysql_mutex_t LOCK_lost_gtids_for_tailing;
  mysql_cond_t update_cond;

  std::atomic<my_off_t> atomic_binlog_end_pos;

  // binlog_file_name/binlog_encrypted_header_size are updated under
  // LOCK_binlog_end_pos mutex to match the latest log_file_name contents. This
  // variable is used in the execution of commands SHOW MASTER STATUS / SHOW
  // BINARY LOGS to avoid taking LOCK_log mutex.
  //
  // binlog_file_name/binlog_encrypted_header_size are protected by
  // LOCK_binlog_end_pos mutex where as log_file_name is protected by LOCK_log
  // mutex.
  char binlog_file_name[FN_REFLEN];
  int binlog_encrypted_header_size;
  std::atomic<st_filenum_pos> last_acked;

  ulonglong bytes_written;
  IO_CACHE index_file;
  char index_file_name[FN_REFLEN];
  /*
     Mapping from binlog file name to the previous gtid set in
     encoded form which is found at the top of the binlog as
     Previous_gtids_log_event. This structure is protected by LOCK_index
     mutex. A new mapping is added in add_log_to_index() function,
     and this is totally rebuilt in init_gtid_sets() function.

     The filenames in this map match those stored in the binlog index file.
     They must be normalized via normalize_binlog_name before they are
     passed to functions that binlog events. Otherwise, filenames that are
     relative paths can result in file access failure when the binlog
     directories change.
  */
  Gtid_set_map previous_gtid_set_map;
  /*
    Used by sys_var gtid_purged_for_tailing
  */
  std::string lost_gtid_for_tailing;
  /*
    crash_safe_index_file is temp file used for guaranteeing
    index file crash safe when master server restarts.
  */
  IO_CACHE crash_safe_index_file;
  char crash_safe_index_file_name[FN_REFLEN];
  /*
    purge_file is a temp file used in purge_logs so that the index file
    can be updated before deleting files from disk, yielding better crash
    recovery. It is created on demand the first time purge_logs is called
    and then reused for subsequent calls. It is cleaned up in cleanup().
  */
  IO_CACHE purge_index_file;
  char purge_index_file_name[FN_REFLEN];
  /*
     The max size before rotation (usable only if log_type == LOG_BIN: binary
     logs and relay logs).
     For a binlog, max_size should be max_binlog_size.
     For a relay log, it should be max_relay_log_size if this is non-zero,
     max_binlog_size otherwise.
     max_size is set in init(), and dynamically changed (when one does SET
     GLOBAL MAX_BINLOG_SIZE|MAX_RELAY_LOG_SIZE) by fix_max_binlog_size and
     fix_max_relay_log_size).
  */
  ulong max_size;

  // current file sequence number for load data infile binary logging
  uint file_id;

  /* pointer to the sync period variable, for binlog this will be
     sync_binlog_period, for relay log this will be
     sync_relay_log_period
  */
  uint *sync_period_ptr;
  uint sync_counter;

  mysql_cond_t m_prep_xids_cond;
  std::atomic<int32> m_atomic_prep_xids{0};

  /* binlog offset tracker for the last update done on SE's. */
  my_off_t ha_last_updated_binlog_pos;
  char ha_last_updated_binlog_file; /* Only need the last char */

  /**
    Increment the prepared XID counter.
   */
  void inc_prep_xids(THD *thd);

  /**
    Decrement the prepared XID counter.

    Signal m_prep_xids_cond if the counter reaches zero.
   */
  void dec_prep_xids(THD *thd);

  int32 get_prep_xids() { return m_atomic_prep_xids; }

  uint32_t non_xid_trxs;
  mysql_cond_t non_xid_trxs_cond;

  void inc_non_xid_trxs(THD *thd);
  void dec_non_xid_trxs(THD *thd);

  int32 get_non_xid_trxs() {
    mysql_mutex_assert_owner(&LOCK_non_xid_trxs);
    return non_xid_trxs;
  }

  inline uint get_sync_period() { return *sync_period_ptr; }

 public:
  /*
    This is used to start writing to a new log file. The difference from
    new_file() is locking. new_file_without_locking() does not acquire
    LOCK_log.
  */
  int new_file_without_locking(
      Format_description_log_event *extra_description_event,
      RaftRotateInfo *raft_rotate_info = nullptr);

 private:
  /**
    Checks binlog error action to identify if the server needs to abort on
    non-recoverable errors when writing to binlog

    @return true if server needs to be aborted
   */
  bool should_abort_on_binlog_error();

  /*
   * @param raft_rotate_info
   *   Rotate related information passed in by listener callbacks.
   *   Caters today to relay log rotates, no-op rotates and config
   *   change rotates.
   */
  void drain_committing_trxs(bool wait_non_xid_trxs_always);
  int new_file_impl(bool need_lock,
                    Format_description_log_event *extra_description_event,
                    RaftRotateInfo *raft_rotate_info = nullptr);

  bool open(PSI_file_key log_file_key, const char *log_name,
            const char *new_name, uint32 new_index_number);
  bool init_and_set_log_file_name(const char *log_name, const char *new_name,
                                  uint32 new_index_number);
  int generate_new_name(char *new_name, const char *log_name,
                        uint32 new_index_number = 0);

  // Maximum gtid recovered after recovery from SE and binlog.
  std::string recovery_binlog_max_gtid;

 public:
  const char *generate_name(const char *log_name, const char *suffix,
                            char *buff);
  bool is_open() { return atomic_log_state != LOG_CLOSED; }

  // Set to true if 'open' binlog was found during the trx log recovery
  bool open_binlog_found = false;

  // The starting position of the first gtid event in the trx log file
  my_off_t first_gtid_start_pos = 0;

  /* True if this binlog is an apply-log (in raft mode apply logs are the binlog
   * used as trx log on follower instances)
   *
   * This is set to true, at the end of converting a Raft FOLLOWER to a
   * MySQL Slave. It is set to false, when the Raft Candidate transitions
   * to LEADER, and converts the MySQL Slave to a MySQL Master as a part
   * of Election Decision Callback.
   *
   * @ref - rpl_handler.cc / point_binlog_to_binlog
   *        rpl_handler.cc / point_binlog_to_apply
   */
  bool is_apply_log = false;

  // TODO(pgl) : update init_index_file() to update apply_file_count
  // Number of files that are created and maintained in index files
  std::atomic<uint64_t> apply_file_count;

  /* This is relay log */
  bool is_relay_log;

  ulong signal_cnt;          // update of the counter is checked by heartbeat
  uint8 checksum_alg_reset;  // to contain a new value when binlog is rotated
  /*
    Holds the last seen in Relay-Log FD's checksum alg value.
    The initial value comes from the slave's local FD that heads
    the very first Relay-Log file. In the following the value may change
    with each received master's FD_m.
    Besides to be used in verification events that IO thread receives
    (except the 1st fake Rotate, see @c Master_info:: checksum_alg_before_fd),
    the value specifies if/how to compute checksum for slave's local events
    and the first fake Rotate (R_f^1) coming from the master.
    R_f^1 needs logging checksum-compatibly with the RL's heading FD_s.

    Legends for the checksum related comments:

    FD     - Format-Description event,
    R      - Rotate event
    R_f    - the fake Rotate event
    E      - an arbirary event

    The underscore indexes for any event
    `_s'   indicates the event is generated by Slave
    `_m'   - by Master

    Two special underscore indexes of FD:
    FD_q   - Format Description event for queuing   (relay-logging)
    FD_e   - Format Description event for executing (relay-logging)

    Upper indexes:
    E^n    - n:th event is a sequence

    RL     - Relay Log
    (A)    - checksum algorithm descriptor value
    FD.(A) - the value of (A) in FD
  */
  binary_log::enum_binlog_checksum_alg relay_log_checksum_alg;

  MYSQL_BIN_LOG(uint *sync_period, bool relay_log = false);
  ~MYSQL_BIN_LOG();

  Gtid engine_binlog_max_gtid;
  char engine_binlog_file[FN_REFLEN + 1];
  my_off_t engine_binlog_pos;

  void set_recovery_binlog_max_gtid(const std::string &max_binlog_gtid) {
    recovery_binlog_max_gtid = max_binlog_gtid;
  }

  const std::string &get_recovery_binlog_max_gtid() {
    return recovery_binlog_max_gtid;
  }

  // copy of Relay_log_info::last_master_timestamp
  std::atomic<time_t> last_master_timestamp;

  void set_psi_keys(
      PSI_mutex_key key_LOCK_index, PSI_mutex_key key_LOCK_commit,
      PSI_mutex_key key_LOCK_commit_queue, PSI_mutex_key key_LOCK_done,
      PSI_mutex_key key_LOCK_flush_queue, PSI_mutex_key key_LOCK_log,
      PSI_mutex_key key_LOCK_binlog_end_pos, PSI_mutex_key key_LOCK_sync,
      PSI_mutex_key key_LOCK_sync_queue, PSI_mutex_key key_LOCK_xids,
      PSI_mutex_key key_LOCK_non_xid_trxs,
      PSI_mutex_key key_LOCK_lost_gtids_for_tailing, PSI_cond_key key_COND_done,
      PSI_cond_key key_update_cond, PSI_cond_key key_prep_xids_cond,
      PSI_cond_key key_non_xid_trxs_cond, PSI_file_key key_file_log,
      PSI_file_key key_file_log_index, PSI_file_key key_file_log_cache,
      PSI_file_key key_file_log_index_cache) {
    m_key_COND_done = key_COND_done;

    m_key_LOCK_commit_queue = key_LOCK_commit_queue;
    m_key_LOCK_done = key_LOCK_done;
    m_key_LOCK_flush_queue = key_LOCK_flush_queue;
    m_key_LOCK_sync_queue = key_LOCK_sync_queue;

    m_key_LOCK_index = key_LOCK_index;
    m_key_LOCK_log = key_LOCK_log;
    m_key_LOCK_binlog_end_pos = key_LOCK_binlog_end_pos;
    m_key_LOCK_commit = key_LOCK_commit;
    m_key_LOCK_sync = key_LOCK_sync;
    m_key_LOCK_xids = key_LOCK_xids;
    m_key_LOCK_non_xid_trxs = key_LOCK_non_xid_trxs;
    m_key_LOCK_lost_gtids_for_tailing = key_LOCK_lost_gtids_for_tailing;
    m_key_update_cond = key_update_cond;
    m_key_prep_xids_cond = key_prep_xids_cond;
    m_key_non_xid_trxs_cond = key_non_xid_trxs_cond;
    m_key_file_log = key_file_log;
    m_key_file_log_index = key_file_log_index;
    m_key_file_log_cache = key_file_log_cache;
    m_key_file_log_index_cache = key_file_log_index_cache;
  }

 public:
  /** Manage the MTS dependency tracking */
  Transaction_dependency_tracker m_dependency_tracker;

  /**
    Find the oldest binary log that contains any GTID that
    is not in the given gtid set. This is done by scanning the map
    structure previous_gtid_set_map in reverse order.

    @param[out] binlog_file_name the file name of oldest binary log found
    @param[in]  gtid_set the given gtid set
    @param[out] first_gtid the first GTID information from the binary log
                file returned at binlog_file_name
    @param[out] errmsg the error message outputted, which is left untouched
                if the function returns false
    @return false on success, true on error.
  */
  bool find_first_log_not_in_gtid_set(char *binlog_file_name,
                                      const Gtid_set *gtid_set,
                                      Gtid *first_gtid, const char **errmsg);

  /**
    Builds the set of all GTIDs in the binary log, and the set of all
    lost GTIDs in the binary log, and stores each set in respective
    argument. This scans the index file from the beginning and builds
    previous_gtid_set_map. Since index file contains the previous gtid
    set in binary string format, this function doesn't open every
    binary log file.

    @param gtid_set Will be filled with all GTIDs in this binary/relay
    log.
    @param lost_groups Will be filled with all GTIDs in the
    Previous_gtids_log_event of the first binary log that has a
    Previous_gtids_log_event. This is requested to binary logs but not
    to relay logs.
    @param verify_checksum If true, checksums will be checked.
    @param need_lock If true, LOCK_log, LOCK_index, and
    global_sid_lock->wrlock are acquired; otherwise they are asserted
    to be taken already.
    @param [out] trx_parser  This will be used to return the actual
    relaylog transaction parser state because of the possibility
    of partial transactions.
    @param [out] partial_trx If a transaction was left incomplete
    on the relaylog, its GTID information should be returned to be
    used in the case of the rest of the transaction be added to the
    relaylog.
    @param is_server_starting True if the server is starting.
    @param max_prev_hlc max hlc in all previous binlogs (out param)
    @param startup True if the server is starting up.
    @return false on success, true on error.
  */
  bool init_gtid_sets(Gtid_set *gtid_set, Gtid_set *lost_groups,
                      bool verify_checksum, bool need_lock,
                      Transaction_boundary_parser *trx_parser,
                      Gtid_monitoring_info *partial_trx,
                      uint64_t *max_prev_hlc = NULL, bool startup = false);

  /**
   * This function is used by binlog_change_to_apply to update
   * the previous gtid set map, since we don't call init_gtid_sets
   * which would have initialized it from disk
   */
  bool init_prev_gtid_sets_map();

  void get_lost_gtids(Gtid_set *gtids) {
    gtids->clear();
    mysql_mutex_lock(&LOCK_index);
    const auto it = previous_gtid_set_map.begin();
    if (it != previous_gtid_set_map.end() && !it->second.empty())
      gtids->add_gtid_encoding((const uchar *)it->second.c_str(),
                               it->second.length());
    mysql_mutex_unlock(&LOCK_index);
  }

  void update_lost_gtid_for_tailing() {
    mysql_mutex_assert_owner(&LOCK_index);
    mysql_mutex_lock(&LOCK_lost_gtids_for_tailing);
    lost_gtid_for_tailing = "";
    const auto it = previous_gtid_set_map.begin();
    if (it != previous_gtid_set_map.end()) lost_gtid_for_tailing = it->second;
    mysql_mutex_unlock(&LOCK_lost_gtids_for_tailing);
  }

  enum_read_gtids_from_binlog_status read_gtids_from_binlog(
      const char *filename, Gtid_set *all_gtids, Gtid_set *prev_gtids,
      Gtid *first_gtid, Sid_map *sid_map, bool verify_checksum,
      bool is_relay_log, my_off_t max_pos = ULLONG_MAX,
      uint64_t *max_prev_hlc = NULL);

  void set_previous_gtid_set_relaylog(Gtid_set *previous_gtid_set_param) {
    DBUG_ASSERT(is_apply_log || is_relay_log);
    previous_gtid_set_relaylog = previous_gtid_set_param;
  }
  /**
    If the thread owns a GTID, this function generates an empty
    transaction and releases ownership of the GTID.

    - If the binary log is disabled for this thread, the GTID is
      inserted directly into the mysql.gtid_executed table and the
      GTID is included in @@global.gtid_executed.  (This only happens
      for DDL, since DML will save the GTID into table and release
      ownership inside ha_commit_trans.)

    - If the binary log is enabled for this thread, an empty
      transaction consisting of GTID, BEGIN, COMMIT is written to the
      binary log, the GTID is included in @@global.gtid_executed, and
      the GTID is added to the mysql.gtid_executed table on the next
      binlog rotation.

    This function must be called by any committing statement (COMMIT,
    implicitly committing statements, or Xid_log_event), after the
    statement has completed execution, regardless of whether the
    statement updated the database.

    This logic ensures that an empty transaction is generated for the
    following cases:

    - Explicit empty transaction:
      SET GTID_NEXT = 'UUID:NUMBER'; BEGIN; COMMIT;

    - Transaction or DDL that gets completely filtered out in the
      slave thread.

    @param thd The committing thread

    @retval 0 Success
    @retval nonzero Error
  */
  int gtid_end_transaction(THD *thd);
  /**
    Re-encrypt previous existent binary/relay logs as below.
      Starting from the next to last entry on the index file, iterating
      down to the first one:
        - If the file is encrypted, re-encrypt it. Otherwise, skip it.
        - If failed to open the file, report an error.

    @retval False Success
    @retval True  Error
  */
  bool reencrypt_logs();

  // Extract HLC time (either prev_hlc or regular hlc) from Metadata_log_event
  uint64_t extract_hlc(Metadata_log_event *metadata_ev);

  /* Return the HLC timestamp for the caller (next txn) */
  uint64_t get_next_hlc() { return hlc.get_next(); }

  uint64_t get_current_hlc() { return hlc.get_current(); }

  /* Update the minimum HLC value. This is used to set the lower bound on the
     HLC for this instance. This is achieved by exposing a global system var
     'minimum hlc' - updates on which will call this function. This can also
     be used to synchronize HLC across different communicating instances */
  uint64_t update_hlc(uint64_t minimum_hlc) { return hlc.update(minimum_hlc); }

  /* get the applied HLC for all known databases in this instance */
  database_hlc_container get_database_hlc() const {
    return hlc.get_database_hlc();
  }

  /* get the applied HLC for a specific database in this instance */
  uint64_t get_selected_database_hlc(const std::string &database) {
    return hlc.get_selected_database_hlc(database);
  }

  void clear_database_hlc() { hlc.clear_database_hlc(); }

  bool wait_for_hlc_applied(THD *thd) { return hlc.wait_for_hlc_applied(thd); }

  bool check_hlc_bound(THD *thd) { return hlc.check_hlc_bound(thd); }

 private:
  std::atomic<enum_log_state> atomic_log_state{LOG_CLOSED};

  /* The previous gtid set in relay log. */
  Gtid_set *previous_gtid_set_relaylog;

  /* Hybrid logical clock for this instance. The logical clock is tracked per
   * instance today. It should be relatively easy to convert this to per-shard
   * by having a map of such  clocks
   */
  HybridLogicalClock hlc;

  // Used by raft log only
  // Log file name: binary-logs-{port}.####
  // Log file ext: ####
  ulong raft_cur_log_ext;
  /*
     This is set when we have registered log entities with raft plugin
     during ordered commit, after we have become master on step up.
     Protected by LOCK_log. To prevent repeated re-registrations.
   */
  bool setup_flush_done;

  int open(const char *opt_name) { return open_binlog(opt_name); }

  /**
    Enter a stage of the ordered commit procedure.

    Entering is stage is done by:

    - Atomically entering a queue of THD objects (which is just one for
      the first phase).

    - If the queue was empty, the thread is the leader for that stage
      and it should process the entire queue for that stage.

    - If the queue was not empty, the thread is a follower and can go
      waiting for the commit to finish.

    The function will lock the stage mutex if the calling thread was designated
    leader for the phase.

    @param[in] thd    Session structure
    @param[in] stage  The stage to enter
    @param[in] queue  Thread queue for the stage
    @param[in] leave_mutex  Mutex that will be released when changing stage
    @param[in] enter_mutex  Mutex that will be taken when changing stage

    @retval true  In case this thread did not become leader, the function
                  returns true *after* the leader has completed the commit
                  on its behalf, so the thread should continue doing the
                  thread-local processing after the commit
                  (i.e. call finish_commit).

    @retval false The thread is the leader for the stage and should do
                  the processing.
  */
  bool change_stage(THD *thd, Commit_stage_manager::StageID stage, THD *queue,
                    mysql_mutex_t *leave_mutex, mysql_mutex_t *enter_mutex);
  std::pair<int, my_off_t> flush_thread_caches(THD *thd);
  int flush_cache_to_file(my_off_t *flush_end_pos);
  int finish_commit(THD *thd);
  std::pair<bool, bool> sync_binlog_file(bool force);
  void process_commit_stage_queue(THD *thd, THD *queue);
  void process_after_commit_stage_queue(THD *thd, THD *first);

  /**
    Set thread variables used while flushing a transaction.

    @param[in] thd  thread whose variables need to be set
    @param[in] all   This is @c true if this is a real transaction commit, and
                 @c false otherwise.
    @param[in] skip_commit
                 This is @c true if the call to @c ha_commit_low should
                 be skipped (it is handled by the caller somehow) and @c
                 false otherwise (the normal case).
  */
  void init_thd_variables(THD *thd, bool all, bool skip_commit);

  /**
    Fetch and empty BINLOG_FLUSH_STAGE and COMMIT_ORDER_FLUSH_STAGE flush queues
    and flush transactions to the disk, and unblock threads executing slave
    preserve commit order.

    @param[in] check_and_skip_flush_logs
                 if false then flush prepared records of transactions to the log
                 of storage engine.
                 if true then flush prepared records of transactions to the log
                 of storage engine only if COMMIT_ORDER_FLUSH_STAGE queue is
                 non-empty.

    @return Pointer to the first session of the BINLOG_FLUSH_STAGE stage queue.
  */
  THD *fetch_and_process_flush_stage_queue(
      const bool check_and_skip_flush_logs = false);

  /**
    Execute the flush stage.

    @param[out] total_bytes_var Pointer to variable that will be set to total
                                number of bytes flushed, or NULL.

    @param[out] rotate_var Pointer to variable that will be set to true if
                           binlog rotation should be performed after releasing
                           locks. If rotate is not necessary, the variable will
                           not be touched.

    @param[out] out_queue_var  Pointer to the sessions queue in flush stage.

    @return Error code on error, zero on success
  */
  int process_flush_stage_queue(my_off_t *total_bytes_var, bool *rotate_var,
                                THD **out_queue_var);

  /**
    Flush and commit the transaction.

    This will execute an ordered flush and commit of all outstanding
    transactions and is the main function for the binary log group
    commit logic. The function performs the ordered commit in four stages.

    Pre-condition: transactions should have called ha_prepare_low, using
                   HA_IGNORE_DURABILITY, before entering here.

    Stage#0 implements slave-preserve-commit-order for applier threads that
    write the binary log. i.e. it forces threads to enter the queue in the
    correct commit order.

    The stage#1 flushes the caches to the binary log and under
    LOCK_log and marks all threads that were flushed as not pending.

    The stage#2 syncs the binary log for all transactions in the group.

    The stage#3 executes under LOCK_commit and commits all transactions in
    order.

    There are three queues of THD objects: one for each stage.
    The Commit_order_manager maintains it own queue and its own order for the
    commit. So Stage#0 doesn't maintain separate StageID.

    When a transaction enters a stage, it adds itself to a queue. If the queue
    was empty so that this becomes the first transaction in the queue, the
    thread is the *leader* of the queue. Otherwise it is a *follower*. The
    leader will do all work for all threads in the queue, and the followers
    will wait until the last stage is finished.

    Stage 0 (SLAVE COMMIT ORDER):
    1. If slave-preserve-commit-order and is slave applier worker thread, then
       waits until its turn to commit i.e. till it is on the top of the queue.
    2. When it reaches top of the queue, it signals next worker in the commit
       order queue to awake.

    Stage 1 (FLUSH):
    1. Sync the engines (ha_flush_logs), since they prepared using non-durable
       settings (HA_IGNORE_DURABILITY).
    2. Generate GTIDs for all transactions in the queue.
    3. Write the session caches for all transactions in the queue to the binary
       log.
    4. Increment the counter of prepared XIDs.

    Stage 2 (SYNC):
    1. If it is time to sync, based on the sync_binlog option, sync the binlog.
    2. If sync_binlog==1, signal dump threads that they can read up to the
       position after the last transaction in the queue

    Stage 3 (COMMIT):
    This is performed by each thread separately, if binlog_order_commits=0.
    Otherwise by the leader does it for all threads.
    1. Call the after_sync hook.
    2. update the max_committed counter in the dependency_tracker
    3. call ha_commit_low
    4. Call the after_commit hook
    5. Update gtids
    6. Decrement the counter of prepared transactions

    If the binary log needs to be rotated, it is done after this. During
    rotation, it takes a lock that prevents new commit groups from executing the
    flush stage, and waits until the counter of prepared transactions becomes 0,
    before it creates the new file.

    @param[in] thd Session to commit transaction for
    @param[in] all This is @c true if this is a real transaction commit, and
                   @c false otherwise.
    @param[in] skip_commit
                   This is @c true if the call to @c ha_commit_low should
                   be skipped and @c false otherwise (the normal case).
  */
  int ordered_commit(THD *thd, bool all, bool skip_commit = false);

  /* Uses the commit stage queue of ordered commit to call raft replication's
   * before_commit hook to block until consensus-commit of trxs
   * (before committing to engine)
   *
   * @param queue_head - The head of the commit stage queue
   *
   */
  void process_consensus_queue(THD *queue_head);

  /* Handles commit consensus error. Commit consensus errors are failures that
   * happen inside raft replication when the leader fails to achieve consensus
   * (majority votes) on trxs. This function either commit's the trx to the
   * engine OR aborts the trx (rollback) based on commit_consensus_error_action
   *
   * @param thd The THD for the session that encountered commit-consensus error
   */
  void handle_commit_consensus_error(THD *thd);

  /* Sets the commit consensus error for all threads in the group */
  void set_commit_consensus_error(THD *queue_head);

  void handle_binlog_flush_or_sync_error(THD *thd, bool need_lock_log,
                                         const char *message);
  bool do_write_cache(Binlog_cache_storage *cache,
                      class Binlog_event_writer *writer);
  void report_binlog_write_error();

 public:
  int open_binlog(const char *opt_name);
  void close();
  enum_result commit(THD *thd, bool all);
  int rollback(THD *thd, bool all);
  bool truncate_relaylog_file(Master_info *mi, my_off_t valid_pos);
  int prepare(THD *thd, bool all);
#if defined(MYSQL_SERVER)

  void update_thd_next_event_pos(THD *thd);
  int flush_and_set_pending_rows_event(THD *thd, Rows_log_event *event,
                                       bool is_transactional);

#endif /* defined(MYSQL_SERVER) */
  void add_bytes_written(ulonglong inc) { bytes_written += inc; }
  void reset_bytes_written() { bytes_written = 0; }
  void harvest_bytes_written(Relay_log_info *rli, bool need_log_space_lock);
  void set_max_size(ulong max_size_arg);
  void signal_update() {
    DBUG_TRACE;
    DBUG_EXECUTE_IF("simulate_delay_in_binlog_signal_update", sleep(1););
    signal_cnt++;
    mysql_cond_broadcast(&update_cond);
    return;
  }

  void update_binlog_end_pos(bool need_lock = true);
  void update_binlog_end_pos(const char *file, my_off_t pos,
                             bool need_lock = true);

  int wait_for_update(const struct timespec *timeout);

  int raft_log_recover();

 public:
  /** register binlog/relay (its IO_CACHE) and mutexes to plugin.
      Sharing the pointers with the plugin enables the plugin to
      flush transactions to the appropriate file when the Raft engine
      calls back the log Wrapper.

      @param thd - Thread descriptor
      @param context - 0 for initial time, 1 for each time
         When we pass in 1 for re-registration, we also validate on
         the plugin side that the cached pointers have not shifted.
         @param If true, take LOCK_log
      @param is_relay_log register the relay log if true, otherwise binlog
                          Different observers are used for different logs
   */
  int register_log_entities(THD *thd, int context, bool need_lock,
                            bool is_relay_log);
  void check_and_register_log_entities(THD *thd);
  void init_pthread_objects();
  void cleanup();
  /**
    Create a new binary log.
    @param log_name Name of binlog
    @param new_name Name of binlog, too. todo: what's the difference
    between new_name and log_name?
    @param max_size_arg The size at which this binlog will be rotated.
    @param null_created_arg If false, and a Format_description_log_event
    is written, then the Format_description_log_event will have the
    timestamp 0. Otherwise, it the timestamp will be the time when the
    event was written to the log.
    @param need_lock_index If true, LOCK_index is acquired; otherwise
    LOCK_index must be taken by the caller.
    @param need_sid_lock If true, the read lock on global_sid_lock
    will be acquired.  Otherwise, the caller must hold the read lock
    on global_sid_lock.
    @param extra_description_event The master's FDE to be written by the I/O
    thread while creating a new relay log file. This should be NULL for
    binary log files.
    @param new_index_number The binary log file index number to start from
    after the RESET MASTER TO command is called.
    @param raft_rotate_info rotate related information passed in by
    listener callbacks
    @param need_end_log_pos_lock If true, LOCK_binlog_end_pos is acquired;
    otherwise LOCK_binlog_end_pos must be taken by the caller.
  */
  bool open_binlog(const char *log_name, const char *new_name,
                   ulong max_size_arg, bool null_created_arg,
                   bool need_lock_index, bool need_sid_lock,
                   Format_description_log_event *extra_description_event,
                   uint32 new_index_number = 0,
                   RaftRotateInfo *raft_rotate_info = nullptr,
                   bool need_end_log_pos_lock = true);

  /**
    Open an existing binlog/relaylog file

    @param log_name Name of binlog
    @param max_size The size at which this binlog will be rotated.
    @param need_end_log_pos_lock If true, LOCK_binlog_end_pos is acquired;
    otherwise LOCK_binlog_end_pos must be taken by the caller.

    @retval false on success, true on error
  */
  bool open_existing_binlog(const char *log_name, ulong max_size_arg,
                            bool need_end_log_pos_lock = true);

  bool open_index_file(const char *index_file_name_arg, const char *log_name,
                       bool need_lock_index);

  /*
   * Opens the index file for the transaction log. If a binlog apply index file
   * is found, then it opens the apply index file. Otherwise it opens the binlog
   * index file
   *
   * @return 0 on success, 1 on error
   */
  int init_index_file();

  /**
    Use this to start writing a new log file
    @param raft_rotate_info - Used by raft to optionally control
     how file rotation happens. Caters to relay log rotates,
     no-op rotates and config change rotates.
  */
  int new_file(Format_description_log_event *extra_description_event,
               RaftRotateInfo *raft_rotate_info = nullptr);

  enum force_cache_type {
    FORCE_CACHE_DEFAULT,
    FORCE_CACHE_STATEMENT,
    FORCE_CACHE_TRANSACTIONAL
  };
  bool write_event(Log_event *event_info, bool write_meta_data_event = false,
                   force_cache_type force_cache = FORCE_CACHE_DEFAULT);
  bool write_cache(THD *thd, class binlog_cache_data *cache_data,
                   class Binlog_event_writer *writer);

  /**
   * Called after a THD's iocache is written to binlog (i.e binlog's cache)
   * during ordered commit. Updates all internal state maintained by mysql.
   * Used only when raft based replication is enabled
   *
   * @param thd Thread variable
   * @param cache_data The cache which was written to binlog
   * @param error will be 1 on errors which needs to be handled in post_write
   *
   * @returns true on error, false on success
   */
  bool post_write(THD *thd, binlog_cache_data *cache_data, int error);

  /**
   * Handles error that occured when flushing the cache to binlog file. Used
   * only when raft based replication is enabled
   *
   * @param thd Thread variable
   */
  void handle_write_error(THD *thd);

  /**
   * Assign HLC timestamp to a thd in group commit
   *
   * @param thd - the THD in group commit
   *
   * @return false on success, true on failure
   */
  bool assign_hlc(THD *thd);

  /**
   * Write HLC timestamp of a thd in group commit to binlog
   *
   * @param thd - the THD in group commit
   * @param cache_data - The cache that is being written dusring flush stage
   * @param writer - Binlog writer
   * @param obuffer - The metadata event will be written to the buffer
   *                  (if not null)
   * @param wrote_hlc - Will be set to true if HLC was written to the log file
   *
   * @return false on success, true on failure
   */
  bool write_hlc(THD *thd, binlog_cache_data *cache_data,
                 Binlog_event_writer *writer, Binlog_cache_storage *obuffer,
                 bool *wrote_hlc = nullptr);

  /**
    Assign automatic generated GTIDs for all commit group threads in the flush
    stage having gtid_next.type == AUTOMATIC_GTID.

    @param first_seen The first thread seen entering the flush stage.
    @return Returns false if succeeds, otherwise true is returned.
  */
  bool assign_automatic_gtids_to_flush_group(THD *first_seen);
  bool write_transaction(THD *thd, binlog_cache_data *cache_data,
                         Binlog_event_writer *writer);

  /**
     Write a dml into statement cache and then flush it into binlog. It writes
     Gtid_log_event and BEGIN, COMMIT automatically.

     It is aimed to handle cases of "background" logging where a statement is
     logged indirectly, like "DELETE FROM a_memory_table". So don't use it on
     any normal statement.

     @param[in] thd  the THD object of current thread.
     @param[in] stmt the DELETE statement.
     @param[in] stmt_len the length of DELETE statement.

     @return Returns false if succeeds, otherwise true is returned.
  */
  bool write_dml_directly(THD *thd, const char *stmt, size_t stmt_len);

  void report_cache_write_error(THD *thd, bool is_transactional);
  bool check_write_error(const THD *thd);
  bool write_incident(THD *thd, bool need_lock_log, const char *err_msg,
                      bool do_flush_and_sync = true);
  bool write_incident(Incident_log_event *ev, THD *thd, bool need_lock_log,
                      const char *err_msg, bool do_flush_and_sync = true);
  bool write_event_to_binlog(Log_event *ev);
  bool write_event_to_binlog_and_flush(Log_event *ev);
  bool write_event_to_binlog_and_sync(Log_event *ev);
  void start_union_events(THD *thd, query_id_t query_id_param);
  void stop_union_events(THD *thd);
  bool is_query_in_union(THD *thd, query_id_t query_id_param);

  bool write_buffer(const char *buf, uint len, Master_info *mi);
  bool write_event(Log_event *ev, Master_info *mi);

 private:
  bool after_write_to_relay_log(Master_info *mi);

 public:
  void make_log_name(char *buf, const char *log_ident);
  bool is_active(const char *log_file_name);
  int remove_logs_from_index(LOG_INFO *linfo, bool need_update_threads);
  int remove_deleted_logs_from_index(bool need_lock_index,
                                     bool need_update_threads);
  int rotate(bool force_rotate, bool *check_purge);
  void purge();
  int rotate_and_purge(THD *thd, bool force_rotate);

  /**
   * Take the config change payload and create a before_flush call into the
   * plugin after taking LOCK_log.
   * We do a rotation immediately after config change, because it enables
   * us to keep the invariant that we don't have free floating metadata
   * events due to Raft, except the rotate event at the end of a file.
   * @config_change - a description of the config change understood by Raft
   *                  (move semantics)
   */
  int config_change_rotate(std::string config_change);

  /*
   * Reads the current index file and returns a list of all file names found in
   * the binlog file
   *
   * @param need_lock - Should LOCK_index be taken?
   * @param lognames [out] - vector of filenames in the index
   *
   * @return 1 on error, 0 on success
   */
  int get_lognames_from_index(bool need_lock,
                              std::vector<std::string> *lognames);

  bool flush();
  /**
     Flush binlog cache and synchronize to disk.

     This function flushes events in binlog cache to binary log file,
     it will do synchronizing according to the setting of system
     variable 'sync_binlog'. If file is synchronized, @c synced will
     be set to 1, otherwise 0.

     @param[in] force if true, ignores the 'sync_binlog' and synchronizes the
     file.

     @retval 0 Success
     @retval other Failure
  */
  bool flush_and_sync(const bool force = false);
  void purge_apply_logs();
  int purge_logs(const char *to_log, bool included, bool need_lock_index,
                 bool need_update_threads, ulonglong *decrease_log_space,
                 bool auto_purge, const char *max_log = nullptr);
  int purge_logs_before_date(time_t purge_time, bool auto_purge,
                             bool stop_purge = 0, bool need_lock_index = true,
                             const char *max_log = nullptr);
  int set_crash_safe_index_file_name(const char *base_file_name);
  int open_crash_safe_index_file();
  int close_crash_safe_index_file();
  int add_log_to_index(uchar *log_file_name, size_t name_len,
                       bool need_lock_index, uchar *previous_gtid_set_buffer,
                       uint gtid_set_length);
  int move_crash_safe_index_file_to_index_file(bool need_lock_index);
  int set_purge_index_file_name(const char *base_file_name);
  int open_purge_index_file(bool destroy);
  bool is_inited_purge_index_file();
  int close_purge_index_file();
  int sync_purge_index_file();
  int register_purge_index_entry(const char *entry);
  int register_create_index_entry(const char *entry);
  int purge_index_entry(THD *thd, ulonglong *decrease_log_space,
                        bool need_lock_index);
  int purge_logs_in_list(const log_file_name_container &delete_list, THD *thd,
                         ulonglong *decrease_log_space, bool need_lock_index);
  bool reset_logs(THD *thd, bool delete_only = false);
  void close(uint exiting, bool need_lock_log, bool need_lock_index);

  // iterating through the log index file
  int find_log_pos(LOG_INFO *linfo, const char *log_name, bool need_lock_index);
  int find_next_log(LOG_INFO *linfo, bool need_lock_index);

  /**
   *  Get the total number of log file entries in the index file
   *
   *  @param need_lock_index - should we aquire LOCK_index
   *  @param num_log_files (out) - number of log file entries in the index file
   *
   *  @return 0 on success, non-zero on failure
   *
   */
  int get_total_log_files(bool need_lock_index, uint64_t *num_log_files);
  int find_next_relay_log(char log_name[FN_REFLEN + 1]);
  int get_current_log(LOG_INFO *linfo, bool need_lock_log = true);
  /*
    This is called to find out the most recent binlog file
    coordinates without LOCK_log protection but with
    LOCK_binlog_end_pos protection.

    get_current_log() is called to find out the most
    recent binlog file coordinates with LOCK_log protection.

    raw_get_current_log() is a helper function to get_current_log().
  */
  void get_current_log_without_lock_log(LOG_INFO *linfo);
  int raw_get_current_log(LOG_INFO *linfo);
  uint next_file_id();
  /**
    Retrieves the contents of the index file associated with this log object
    into an `std::list<std::string>` object. The order held by the index file is
    kept.

    @param need_lock_index whether or not the lock over the index file should be
                           acquired inside the function.

    @return a pair: a function status code; a list of `std::string` objects with
            the content of the log index file.
  */
  std::pair<int, std::list<std::string>> get_log_index(
      bool need_lock_index = true);
  void lock_commits(snapshot_info_st *ss_info);
  void unlock_commits(snapshot_info_st *ss_info);
  inline char *get_index_fname() { return index_file_name; }
  inline char *get_log_fname() { return log_file_name; }
  const char *get_name() const { return name; }
  inline mysql_mutex_t *get_log_lock() { return &LOCK_log; }
  inline mysql_mutex_t *get_commit_lock() { return &LOCK_commit; }
  inline mysql_cond_t *get_log_cond() { return &update_cond; }
  inline Binlog_ofile *get_binlog_file() { return m_binlog_file; }

  inline void lock_index() { mysql_mutex_lock(&LOCK_index); }
  inline void unlock_index() { mysql_mutex_unlock(&LOCK_index); }
  inline IO_CACHE *get_index_file() { return &index_file; }

  /**
    Function to report the missing GTIDs.

    This function logs the missing transactions on master to its error log
    as a warning. If the missing GTIDs are too long to print in a message,
    it suggests the steps to extract the missing transactions.

    This function also informs slave about the GTID set sent by the slave,
    transactions missing on the master and few suggestions to recover from
    the error. This message shall be wrapped by
    ER_MASTER_FATAL_ERROR_READING_BINLOG on slave and will be logged as an
    error.

    This function will be called from mysql_binlog_send() function.

    @param lost_gtid_set               GTID set of missing gtids
    @param slave_executed_gtid_set     GTID set executed by slave
    @param errmsg                      Pointer to the error message
  */
  void report_missing_purged_gtids(const Gtid_set *lost_gtid_set,
                                   const Gtid_set *slave_executed_gtid_set,
                                   const char **errmsg);

  /**
    Function to report the missing GTIDs.

    This function logs the missing transactions on master to its error log
    as a warning. If the missing GTIDs are too long to print in a message,
    it suggests the steps to extract the missing transactions.

    This function also informs slave about the GTID set sent by the slave,
    transactions missing on the master and few suggestions to recover from
    the error. This message shall be wrapped by
    ER_MASTER_FATAL_ERROR_READING_BINLOG on slave and will be logged as an
    error.

    This function will be called from find_first_log_not_in_gtid_set()
    function.

    @param previous_gtid_set           Previous GTID set found
    @param slave_executed_gtid_set     GTID set executed by slave
    @param errmsg                      Pointer to the error message
  */
  void report_missing_gtids(const Gtid_set *previous_gtid_set,
                            const Gtid_set *slave_executed_gtid_set,
                            const char **errmsg);
  static const int MAX_RETRIES_FOR_DELETE_RENAME_FAILURE = 5;
  inline const Gtid_set_map *get_previous_gtid_set_map() const {
    return &previous_gtid_set_map;
  }

  void get_lost_gtid_for_tailing(Gtid_set *gtids) {
    gtids->clear();
    mysql_mutex_lock(&LOCK_lost_gtids_for_tailing);
    if (!lost_gtid_for_tailing.empty())
      gtids->add_gtid_encoding((const uchar *)lost_gtid_for_tailing.c_str(),
                               lost_gtid_for_tailing.length());
    mysql_mutex_unlock(&LOCK_lost_gtids_for_tailing);
  }

  /*
    It is called by the threads (e.g. dump thread, applier thread) which want
    to read hot log without LOCK_log protection.
  */
  my_off_t get_binlog_end_pos() const;
  my_off_t get_last_acked_pos(bool *wait_for_ack, const char *sender_log_name);
  void signal_semi_sync_ack(const char *const log_file, const my_off_t log_pos);
  void reset_semi_sync_last_acked();
  void get_semi_sync_last_acked(std::string &log_file, my_off_t &log_pos);

  mysql_mutex_t *get_binlog_end_pos_lock() { return &LOCK_binlog_end_pos; }
  void lock_binlog_end_pos() { mysql_mutex_lock(&LOCK_binlog_end_pos); }
  void unlock_binlog_end_pos() { mysql_mutex_unlock(&LOCK_binlog_end_pos); }
  inline void update_binlog_group_commit_step() {
    mysql_mutex_lock(&LOCK_log);
    counter_histogram_init(&histogram_binlog_group_commit,
                           opt_histogram_step_size_binlog_group_commit);
    mysql_mutex_unlock(&LOCK_log);
  }

  /**
    Deep copy global_sid_map and gtid_executed.
    Both operations are done under LOCK_commit and global_sid_lock
    protection.

    @param[out] sid_map  The Sid_map to which global_sid_map will
                         be copied.
    @param[out] gtid_set The Gtid_set to which gtid_executed will
                         be copied.

    @return the operation status
      @retval 0      OK
      @retval !=0    Error
  */
  int get_gtid_executed(Sid_map *sid_map, Gtid_set *gtid_set);

  /*
    True while rotating binlog, which is caused by logging Incident_log_event.
  */
  bool is_rotating_caused_by_incident;
};

struct LOAD_FILE_INFO {
  THD *thd;
  my_off_t last_pos_in_file;
  bool logged_data_file, log_delayed;
};

extern MYSQL_PLUGIN_IMPORT MYSQL_BIN_LOG mysql_bin_log;

/**
 * Encapsulation over binlog or relay log for dumping raft logs during
 * COM_BINLOG_DUMP and COM_BINLOG_DUMP_GTID.
 */
class Dump_log {
 public:
  // RAII class to handle locking for Dump_log
  class Locker {
   public:
    explicit Locker(Dump_log *dump_log) {
      dump_log_ = dump_log;
      should_lock_ = dump_log_->lock();
    }

    ~Locker() {
      if (should_lock_) dump_log_->unlock(should_lock_);
    }

   private:
    bool should_lock_ = false;
    Dump_log *dump_log_ = nullptr;
  };

  Dump_log();

  void switch_log(bool relay_log, bool should_lock = true);

  MYSQL_BIN_LOG *get_log(bool should_lock = true) {
    bool is_locked = false;
    if (should_lock) is_locked = lock();
    auto ret = log_;
    if (should_lock) unlock(is_locked);
    return ret;
  }

  void signal_semi_sync_ack(const char *const log_file,
                            const my_off_t log_pos) {
    Locker lock(this);
    log_->signal_semi_sync_ack(log_file, log_pos);
  }

  void reset_semi_sync_last_acked() {
    Locker lock(this);
    log_->reset_semi_sync_last_acked();
  }

  void get_lost_gtids(Gtid_set *gtid_set) {
    Locker lock(this);
    log_->get_lost_gtid_for_tailing(gtid_set);
  }

  // Avoid using this and try to use Dump_log::Locker class instead
  bool lock() {
    // NOTE: we lock only when we're in raft mode. That's why we're returning a
    // bool to indicate whether we locked or not. We pass this bool to unlock
    // method to unlock only then the mutex was actually locked.
    const bool should_lock = enable_raft_plugin;
    if (should_lock) log_mutex_.lock();
    return should_lock;
  }

  // Avoid using this and try to use Dump_log::Locker class instead
  void unlock(bool is_locked) {
    if (is_locked) log_mutex_.unlock();
  }

 private:
  MYSQL_BIN_LOG *log_;
  std::mutex log_mutex_;
};
extern MYSQL_PLUGIN_IMPORT Dump_log dump_log;

/**
  Check if the the transaction is empty.

  @param thd The client thread that executed the current statement.

  @retval true No changes found in any storage engine
  @retval false Otherwise.

**/
bool is_transaction_empty(THD *thd);
/**
  Check if the transaction has no rw flag set for any of the storage engines.

  @param thd The client thread that executed the current statement.
  @param trx_scope The transaction scope to look into.

  @retval the number of engines which have actual changes.
 */
int check_trx_rw_engines(THD *thd, Transaction_ctx::enum_trx_scope trx_scope);

/**
  Check if at least one of transacaction and statement binlog caches contains
  an empty transaction, other one is empty or contains an empty transaction,
  which has two binlog events "BEGIN" and "COMMIT".

  @param thd The client thread that executed the current statement.

  @retval true  At least one of transacaction and statement binlog caches
                contains an empty transaction, other one is empty or
                contains an empty transaction.
  @retval false Otherwise.
*/
bool is_empty_transaction_in_binlog_cache(const THD *thd);
bool trans_has_updated_trans_table(const THD *thd);
bool stmt_has_updated_trans_table(Ha_trx_info *ha_list);
bool ending_trans(THD *thd, const bool all);
bool ending_single_stmt_trans(THD *thd, const bool all);
bool trans_cannot_safely_rollback(const THD *thd);
bool stmt_cannot_safely_rollback(const THD *thd);

int log_loaded_block(IO_CACHE *file);

bool purge_master_logs(THD *thd, const char *to_log);
bool purge_raft_logs(THD *thd, const char *to_log);
bool purge_raft_logs_before_date(THD *thd, time_t purge_time);
bool update_relay_log_cordinates(Relay_log_info *rli);
bool show_raft_logs(THD *thd);
bool purge_master_logs_before_date(THD *thd, time_t purge_time);
bool show_binlog_events(THD *thd, MYSQL_BIN_LOG *binary_log);
bool mysql_show_binlog_events(THD *thd);
bool show_gtid_executed(THD *thd);
void check_binlog_cache_size(THD *thd);
void check_binlog_stmt_cache_size(THD *thd);
void update_binlog_hlc();
bool binlog_enabled();
void register_binlog_handler(THD *thd, bool trx);
int query_error_code(const THD *thd, bool not_killed);
bool show_raft_status(THD *thd);
bool get_and_lock_master_info(Master_info **master_info);
void unlock_master_info(Master_info *master_info);
int trim_logged_gtid(const std::vector<std::string> &trimmed_gtids);
int get_committed_gtids(const std::vector<std::string> &gtids,
                        std::vector<std::string> *committed_gtids);

extern const char *log_bin_index;
extern const char *log_bin_basename;
extern bool opt_binlog_order_commits;
extern ulong rpl_read_size;
extern bool rpl_semi_sync_master_enabled;

/**
 * Start a raft configuration change on the binlog with the provided
 * config change payload
 * @param config_change - Has the committed Config and New Config
 */
int raft_config_change(std::string config_change);

/**
 * Block/unblock dump threads
 */
int handle_dump_threads(bool block);

/**
 * Updates slave_list datastructure with raft follower information
 */
int raft_update_follower_info(
    const std::unordered_map<std::string, std::string> &follower_info,
    bool is_leader, bool is_shutdown);

/**
  Rotates the binary log file. Helper method invoked by raft plugin through
  raft listener queue.

  @param thd  The current thread doing the rotate

  @returns true if a problem occurs, false otherwise.
 */
int rotate_binlog_file(THD *thd);

/**
  Rotates the relay log file. Helper method invoked by raft plugin through
  raft listener queue.
  @param raft_rotate_info raft flags and log info

  @returns true if a problem occurs, false otherwise.
 */
int rotate_relay_log_for_raft(RaftRotateInfo *raft_rotate_info);

/**
  This is used to change the mysql_bin_log global MYSQL_BIN_LOG file
  to point to the apply binlog/reopen new one. Apply binlogs are binlog
  files used by FOLLOWERS/SLAVES in Raft. They are only on the State
  Machine side

  @returns true if a problem occurs, false otherwise.
 */
int binlog_change_to_apply();

/**
  This is used to change the mysql_bin_log global MYSQL_BIN_LOG file
  to point to latest binlog-330*.# (Raft LOG). This has to be done
  before a Raft LEADER can become a MySQL Master and start proposing
  transactions via ORDERED COMMIT

  @returns true if a problem occurs, false otherwise.
 */
int binlog_change_to_binlog(THD *thd);

/**
  Turns a relative log binary log path into a full path, based on the
  opt_bin_logname or opt_relay_logname. Also trims the cr-lf at the
  end of the full_path before return to avoid any server startup
  problem on windows.

  @param from         The log name we want to make into an absolute path.
  @param to           The buffer where to put the results of the
                      normalization.
  @param is_relay_log Switch that makes is used inside to choose which
                      option (opt_bin_logname or opt_relay_logname) to
                      use when calculating the base path.

  @returns true if a problem occurs, false otherwise.
 */

bool normalize_binlog_name(char *to, const char *from, bool is_relay_log);

/*
  Splits the first argument into two parts using the delimiter ' '.
  The second part is converted into an integer and the space is
  modified to '\0' in the first argument.

  @param file_name_and_gtid_set_length  binlog file_name and gtid_set length
                                        in binary form separated by ' '.

  @return previous gtid_set length by converting the second string in to an
                            integer.
*/
uint split_file_name_and_gtid_set_length(char *file_name_and_gtid_set_length);

/*
  Compare two binlog files:positions
  @param b1    binlog file1
  @param p1    binlog pos2
  @param b2    binlog file2
  @param p2    binlog pos2
  @return true if (b2,p2) is larger than (b1,p1)
  */
bool is_binlog_advanced(const char *b1, my_off_t p1, const char *b2,
                        my_off_t p2);
#endif /* BINLOG_H_INCLUDED */
