/* Copyright (c) 2009, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/binlog.h"

#include "my_config.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <exception>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "lex_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_systime.h"
#include "my_thread.h"
#include "sql/check_stack.h"
#include "sql/clone_handler.h"
#include "sql_string.h"
#include "template_utils.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <algorithm>
#include <list>
#include <map>
#include <new>
#include <queue>
#include <sstream>
#include <vector>

#include "dur_prop.h"
#include "libbinlogevents/include/compression/base.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "libbinlogevents/include/control_events.h"
#include "libbinlogevents/include/debug_vars.h"
#include "libbinlogevents/include/rows_event.h"
#include "libbinlogevents/include/statement_events.h"
#include "libbinlogevents/include/table_id.h"
#include "mf_wcomp.h"    // wild_one, wild_many
#include "mutex_lock.h"  // Mutex_lock
#include "my_base.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_sqlcommand.h"
#include "my_stacktrace.h"  // my_safe_print_system_time
#include "my_thread_local.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/thread_type.h"
#include "mysqld_error.h"
#include "partition_info.h"
#include "prealloced_array.h"
#include "sql/binlog/global.h"
#include "sql/binlog/tools/iterators.h"
#include "sql/binlog_ostream.h"
#include "sql/binlog_reader.h"
#include "sql/create_field.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/derror.h"      // ER_THD
#include "sql/discrete_interval.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item_func.h"  // user_var_entry
#include "sql/key.h"
#include "sql/log.h"
#include "sql/log_event.h"           // Rows_log_event
#include "sql/mysqld.h"              // sync_binlog_period ...
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/query_options.h"
#include "sql/replication.h"
#include "sql/rpl_binlog_sender.h"
#include "sql/rpl_filter.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_handler.h"  // RUN_HOOK
#include "sql/rpl_mi.h"       // Master_info
#include "sql/rpl_msr.h"      // channel_map
#include "sql/rpl_record.h"
#include "sql/rpl_rli.h"      // Relay_log_info
#include "sql/rpl_rli_pdb.h"  // Slave_worker
#include "sql/rpl_slave.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/rpl_transaction_ctx.h"
#include "sql/rpl_trx_boundary_parser.h"  // Transaction_boundary_parser
#include "sql/rpl_utility.h"
#include "sql/sql_backup_lock.h"  // is_instance_backup_locked
#include "sql/sql_base.h"         // find_temporary_table
#include "sql/sql_bitmap.h"
#include "sql/sql_const.h"
#include "sql/sql_data_change.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // sqlcom_can_generate_row_events
#include "sql/sql_show.h"   // append_identifier
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "sql_partition.h"
#include "thr_lock.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

class Item;

using binary_log::checksum_crc32;
using std::list;
using std::max;
using std::min;
using std::string;

static bool enable_raft_plugin_save = false;

#define FLAGSTR(V, F) ((V) & (F) ? #F " " : "")
#define YESNO(X) ((X) ? "yes" : "no")

/**
  @defgroup Binary_Log Binary Log
  @{
 */

#define MY_OFF_T_UNDEF (~(my_off_t)0UL)

/*
  Constants required for the limit unsafe warnings suppression
 */
// seconds after which the limit unsafe warnings suppression will be activated
#define LIMIT_UNSAFE_WARNING_ACTIVATION_TIMEOUT 50
// number of limit unsafe warnings after which the suppression will be activated
#define LIMIT_UNSAFE_WARNING_ACTIVATION_THRESHOLD_COUNT 50

static ulonglong limit_unsafe_suppression_start_time = 0;
static bool unsafe_warning_suppression_is_activated = false;
static int limit_unsafe_warning_count = 0;

static handlerton *binlog_hton;
bool opt_binlog_order_commits = true;

const char *log_bin_index = nullptr;
const char *log_bin_basename = nullptr;
// const char *opt_relaylog_index_name = nullptr;

const char *hlc_ts_lower_bound = "hlc_ts_lower_bound";
const char *hlc_ts_upper_bound = "hlc_ts_upper_bound";
const char *hlc_wait_timeout_ms = "hlc_wait_timeout_ms";

/* Size for IO_CACHE buffer for binlog & relay log */
ulong rpl_read_size;
bool rpl_semi_sync_master_enabled = false;

latency_histogram histogram_raft_trx_wait;

char *histogram_step_size_binlog_fsync = NULL;
int opt_histogram_step_size_binlog_group_commit = 1;
latency_histogram histogram_binlog_fsync;
counter_histogram histogram_binlog_group_commit;

MYSQL_BIN_LOG mysql_bin_log(&sync_binlog_period);
Dump_log dump_log;

static int binlog_init(void *p);
static int binlog_start_trans_and_stmt(THD *thd, Log_event *start_event);
static int binlog_close_connection(handlerton *hton, THD *thd);
static int binlog_savepoint_set(handlerton *hton, THD *thd, void *sv);
static int binlog_savepoint_rollback(handlerton *hton, THD *thd, void *sv);
static bool binlog_savepoint_rollback_can_release_mdl(handlerton *hton,
                                                      THD *thd);
static int binlog_commit(handlerton *hton, THD *thd, bool all);
static int binlog_rollback(handlerton *hton, THD *thd, bool all);
static int binlog_prepare(handlerton *hton, THD *thd, bool all);
static xa_status_code binlog_xa_commit(handlerton *hton, XID *xid);
static xa_status_code binlog_xa_rollback(handlerton *hton, XID *xid);
static void exec_binlog_error_action_abort(const char *err_string);
static bool binlog_recover(Binlog_file_reader *binlog_file_reader,
                           my_off_t *valid_pos, Gtid *binlog_max_gtid,
                           char *engine_binlog_file,
                           my_off_t *engine_binlog_pos,
                           const std::string &cur_binlog_file,
                           my_off_t *first_gtid_start_pos);
static void binlog_prepare_row_images(const THD *thd, TABLE *table,
                                      bool is_update);
static bool is_loggable_xa_prepare(THD *thd);
static std::pair<std::string, uint> extract_file_index(
    const std::string &file_name);
extern int ha_update_binlog_pos(const char *, my_off_t, Gtid *);

bool normalize_binlog_name(char *to, const char *from, bool is_relay_log) {
  DBUG_TRACE;
  bool error = false;
  char buff[FN_REFLEN];
  char *ptr = const_cast<char *>(from);
  char *opt_name = is_relay_log ? opt_relay_logname : opt_bin_logname;

  DBUG_ASSERT(from);

  /* opt_name is not null and not empty and from is a relative path */
  if (opt_name && opt_name[0] && from && !test_if_hard_path(from)) {
    // take the path from opt_name
    // take the filename from from
    char log_dirpart[FN_REFLEN], log_dirname[FN_REFLEN];
    size_t log_dirpart_len, log_dirname_len;
    dirname_part(log_dirpart, opt_name, &log_dirpart_len);
    dirname_part(log_dirname, from, &log_dirname_len);

    /* log may be empty => relay-log or log-bin did not
        hold paths, just filename pattern */
    if (log_dirpart_len > 0) {
      /* create the new path name */
      if (fn_format(buff, from + log_dirname_len, log_dirpart, "",
                    MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr) {
        error = true;
        goto end;
      }

      ptr = buff;
    }
  }

  DBUG_ASSERT(ptr);
  if (ptr) {
    size_t length = strlen(ptr);

    // Strips the CR+LF at the end of log name and \0-terminates it.
    if (length && ptr[length - 1] == '\n') {
      ptr[length - 1] = 0;
      length--;
      if (length && ptr[length - 1] == '\r') {
        ptr[length - 1] = 0;
        length--;
      }
    }
    if (!length) {
      error = true;
      goto end;
    }
    strmake(to, ptr, length);
  }
end:
  return error;
}

/**
   Logical binlog file which wraps and hides the detail of lower layer storage
   implementation. Binlog code just use this class to control real storage
 */
class MYSQL_BIN_LOG::Binlog_ofile : public Basic_ostream {
 public:
  ~Binlog_ofile() override {
    DBUG_TRACE;
    close();
    return;
  }

  /**
     Opens the binlog file. It opens the lower layer storage.

     @param[in] log_file_key  The PSI_file_key for this stream
     @param[in] binlog_name  The file to be opened
     @param[in] flags  The flags used by IO_CACHE.
     @param[in] existing True if opening the file, false if creating a new one.

     @retval false  Success
     @retval true  Error
  */
  bool open(
#ifdef HAVE_PSI_INTERFACE
      PSI_file_key log_file_key,
#endif
      const char *binlog_name, myf flags, bool existing = false) {
    DBUG_TRACE;
    DBUG_ASSERT(m_pipeline_head == nullptr);

#ifndef DBUG_OFF
    {
#ifndef HAVE_PSI_INTERFACE
      PSI_file_key log_file_key = PSI_NOT_INSTRUMENTED;
#endif
      MY_STAT info;
      if (!mysql_file_stat(log_file_key, binlog_name, &info, MYF(0))) {
        DBUG_ASSERT(existing == !(my_errno() == ENOENT));
        set_my_errno(0);
      }
    }
#endif

    std::unique_ptr<IO_CACHE_ostream> file_ostream(new IO_CACHE_ostream);
    if (file_ostream->open(log_file_key, binlog_name, flags)) return true;

    // Get the underlying IO_CACHE for the file stream
    m_io_cache = file_ostream->get_io_cache();

    m_pipeline_head = std::move(file_ostream);

    /* Setup encryption for new files if needed */
    if (!existing && rpl_encryption.is_enabled()) {
      std::unique_ptr<Binlog_encryption_ostream> encrypted_ostream(
          new Binlog_encryption_ostream());
      if (encrypted_ostream->open(std::move(m_pipeline_head))) return true;
      m_encrypted_header_size = encrypted_ostream->get_header_size();
      m_pipeline_head = std::move(encrypted_ostream);
    }

    return false;
  }

  /**
    Opens an existing binlog file. It opens the lower layer storage reusing the
    existing file password if needed.

    @param[in] log_file_key The PSI_file_key for this stream
    @param[in] binlog_name The file to be opened
    @param[in] flags The flags used by IO_CACHE.

    @retval std::unique_ptr A Binlog_ofile object pointer.
    @retval nullptr Error.
  */
  static std::unique_ptr<Binlog_ofile> open_existing(
#ifdef HAVE_PSI_INTERFACE
      PSI_file_key log_file_key,
#endif
      const char *binlog_name, myf flags) {
    DBUG_TRACE;
    std::unique_ptr<Rpl_encryption_header> header;
    unsigned char magic[BINLOG_MAGIC_SIZE];

    /* Open a simple istream to read the magic from the file */
    IO_CACHE_istream istream;
    if (istream.open(key_file_binlog, key_file_binlog_cache, binlog_name,
                     MYF(MY_WME | MY_DONT_CHECK_FILESIZE), rpl_read_size))
      return nullptr;
    if (istream.read(magic, BINLOG_MAGIC_SIZE) != BINLOG_MAGIC_SIZE)
      return nullptr;

    DBUG_ASSERT(Rpl_encryption_header::ENCRYPTION_MAGIC_SIZE ==
                BINLOG_MAGIC_SIZE);
    /* Identify the file type by the magic to get the encryption header */
    if (memcmp(magic, Rpl_encryption_header::ENCRYPTION_MAGIC,
               BINLOG_MAGIC_SIZE) == 0) {
      header = Rpl_encryption_header::get_header(&istream);
      if (header == nullptr) return nullptr;
    } else if (memcmp(magic, BINLOG_MAGIC, BINLOG_MAGIC_SIZE) != 0) {
      return nullptr;
    }

    /* Open the binlog_ofile */
    std::unique_ptr<Binlog_ofile> ret_ofile(new Binlog_ofile);
    if (ret_ofile->open(
#ifdef HAVE_PSI_INTERFACE
            log_file_key,
#endif
            binlog_name, flags, true)) {
      return nullptr;
    }

    if (header != nullptr) {
      /* Add the encryption stream on top of IO_CACHE */
      std::unique_ptr<Binlog_encryption_ostream> encrypted_ostream(
          new Binlog_encryption_ostream);
      ret_ofile->m_encrypted_header_size = header->get_header_size();
      encrypted_ostream->open(std::move(ret_ofile->m_pipeline_head),
                              std::move(header));
      ret_ofile->m_pipeline_head = std::move(encrypted_ostream);
      ret_ofile->set_encrypted();
    }
    return ret_ofile;
  }

  void close() {
    m_pipeline_head.reset(nullptr);
    m_position = 0;
    m_encrypted_header_size = 0;
  }

  /**
     Writes data into storage and maintains binlog position.

     @param[in] buffer  the data will be written
     @param[in] length  the length of the data

     @retval false  Success
     @retval true  Error
  */
  bool write(const unsigned char *buffer, my_off_t length) override {
    DBUG_ASSERT(m_pipeline_head != nullptr);

    if (m_pipeline_head->write(buffer, length)) return true;

    m_position += length;
    return false;
  }

  /**
     Updates some bytes in the binlog file. If is only used for clearing
     LOG_EVENT_BINLOG_IN_USE_F.

     @param[in] buffer  the data will be written
     @param[in] length  the length of the data
     @param[in] offset  the offset of the bytes will be updated

     @retval false  Success
     @retval true  Error
  */
  bool update(const unsigned char *buffer, my_off_t length, my_off_t offset) {
    DBUG_ASSERT(m_pipeline_head != nullptr);
    return m_pipeline_head->seek(offset) ||
           m_pipeline_head->write(buffer, length);
  }

  /**
     Truncates some data at the end of the binlog file.

     @param[in] offset  where the binlog file will be truncated to.

     @retval false  Success
     @retval true  Error
  */
  bool truncate(my_off_t offset) {
    DBUG_ASSERT(m_pipeline_head != nullptr);

    if (m_pipeline_head->truncate(offset)) return true;
    m_position = offset;
    return false;
  }

  bool flush() { return m_pipeline_head->flush(); }
  bool sync() { return m_pipeline_head->sync(); }
  bool flush_and_sync() { return flush() || sync(); }
  my_off_t position() { return m_position; }
  bool is_empty() { return position() == 0; }
  bool is_open() { return m_pipeline_head != nullptr; }
  /**
    Returns the encrypted header size of the binary log file.

    @retval 0 The file is not encrypted.
    @retval >0 The encryption header size.
  */
  int get_encrypted_header_size() { return m_encrypted_header_size; }
  /**
    Returns the real file size.

    While position() returns the "file size" from the plain binary log events
    stream point of view, this function considers the encryption header when it
    exists.

    @return The real file size considering the encryption header.
  */
  my_off_t get_real_file_size() { return m_position + m_encrypted_header_size; }
  /**
    Get the pipeline head.

    @retval  Returns the pipeline head or nullptr.
  */
  std::unique_ptr<Truncatable_ostream> get_pipeline_head() {
    return std::move(m_pipeline_head);
  }
  /**
    Check if the log file is encrypted.

    @retval  True if the log file is encrypted.
    @retval  False if the log file is not encrypted.
  */
  bool is_encrypted() { return m_encrypted; }
  /**
    Set that the log file is encrypted.
  */
  void set_encrypted() { m_encrypted = true; }

  my_off_t get_my_b_tell() { return m_pipeline_head->get_my_b_tell(); }

  /**
     Return the underlying io_cache for this stream object
     @retval A pointer to the underlying IO_CACHE
  */
  IO_CACHE *get_io_cache() const { return m_io_cache; }

  /**
     Seek to the specified offset in the stream. Also sets up the internal
     state correctly.

     @param[in] offset  offset in the stream to seek to

     @retval false  Success
     @retval true  Error
  */
  bool seek(my_off_t offset) {
    if (m_pipeline_head->seek(offset)) return true;  // error

    m_position = 0;
    if (m_encrypted && m_encrypted_header_size > 0 &&
        m_encrypted_header_size <= (int)offset)
      m_position = offset - m_encrypted_header_size;
    else if (!m_encrypted)
      m_position = offset;

    return false;  // success
  }

 private:
  my_off_t m_position = 0;
  int m_encrypted_header_size = 0;
  std::unique_ptr<Truncatable_ostream> m_pipeline_head;
  bool m_encrypted = false;
  IO_CACHE *m_io_cache = nullptr;
};

/**
  Helper class to switch to a new thread and then go back to the previous one,
  when the object is destroyed using RAII.

  This class is used to temporarily switch to another session (THD
  structure). It will set up thread specific "globals" correctly
  so that the POSIX thread looks exactly like the session attached to.
  However, PSI_thread info is not touched as it is required to show
  the actual physial view in PFS instrumentation i.e., it should
  depict as the real thread doing the work instead of thread it switched
  to.

  On destruction, the original session (which is supplied to the
  constructor) will be re-attached automatically. For example, with
  this code, the value of @c current_thd will be the same before and
  after execution of the code.

  @code
  {
    for (int i = 0 ; i < count ; ++i)
    {
      // here we are attached to current_thd
      // [...]
      Thd_backup_and_restore switch_thd(current_thd, other_thd[i]);
      // [...]
      // here we are attached to other_thd[i]
      // [...]
    }
    // here we are attached to current_thd
  }
  @endcode

  @warning The class is not designed to be inherited from.
 */

class Thd_backup_and_restore {
 public:
  /**
    Try to attach the POSIX thread to a session.

    @param[in] backup_thd    The thd to restore to when object is destructed.
    @param[in] new_thd       The thd to attach to.
   */

  Thd_backup_and_restore(THD *backup_thd, THD *new_thd)
      : m_backup_thd(backup_thd),
        m_new_thd(new_thd),
        m_new_thd_old_real_id(new_thd->real_id),
        m_new_thd_old_thread_stack(new_thd->thread_stack) {
    DBUG_ASSERT(m_backup_thd != nullptr && m_new_thd != nullptr);
    // Reset the state of the current thd.
    m_backup_thd->restore_globals();

    m_new_thd->thread_stack = m_backup_thd->thread_stack;
    m_new_thd->store_globals();
  }

  /**
      Restores to previous thd.
   */
  ~Thd_backup_and_restore() {
    /*
      Restore the global variables of the thd we previously attached to,
      to its original state. In other words, detach the m_new_thd.
    */
    m_new_thd->restore_globals();
    m_new_thd->real_id = m_new_thd_old_real_id;
    m_new_thd->thread_stack = m_new_thd_old_thread_stack;

    // Reset the global variables to the original state.
    m_backup_thd->store_globals();
  }

 private:
  THD *m_backup_thd;
  THD *m_new_thd;
  my_thread_t m_new_thd_old_real_id;
  const char *m_new_thd_old_thread_stack;
};

/**
  Caches for non-transactional and transactional data before writing
  it to the binary log.

  @todo All the access functions for the flags suggest that the
  encapsuling is not done correctly, so try to move any logic that
  requires access to the flags into the cache.
*/
class binlog_cache_data {
 public:
  binlog_cache_data(bool trx_cache_arg, ulong *ptr_binlog_cache_use_arg,
                    ulong *ptr_binlog_cache_disk_use_arg)
      : m_pending(nullptr),
        ptr_binlog_cache_use(ptr_binlog_cache_use_arg),
        ptr_binlog_cache_disk_use(ptr_binlog_cache_disk_use_arg) {
    flags.transactional = trx_cache_arg;
  }

  bool open(my_off_t cache_size, my_off_t max_cache_size) {
    return m_cache.open(cache_size, max_cache_size);
  }

  Binlog_cache_storage *get_cache() { return &m_cache; }
  int finalize(THD *thd, Log_event *end_event);
  int finalize(THD *thd, Log_event *end_event, XID_STATE *xs);
  int flush(THD *thd, my_off_t *bytes, bool *wrote_xid);
  int write_event(THD *thd, Log_event *event,
                  bool write_meta_data_event = false);
  size_t get_event_counter() { return event_counter; }
  size_t get_compressed_size() { return m_compressed_size; }
  size_t get_decompressed_size() { return m_decompressed_size; }
  binary_log::transaction::compression::type get_compression_type() {
    return m_compression_type;
  }

  void set_compressed_size(size_t s) { m_compressed_size = s; }
  void set_decompressed_size(size_t s) { m_decompressed_size = s; }
  void set_compression_type(binary_log::transaction::compression::type t) {
    m_compression_type = t;
  }

  virtual ~binlog_cache_data() {
    DBUG_ASSERT(is_binlog_empty());
    m_cache.close();
  }

  bool is_binlog_empty() const {
    DBUG_PRINT("debug", ("%s_cache - pending: 0x%llx, bytes: %llu",
                         (flags.transactional ? "trx" : "stmt"),
                         (ulonglong)pending(), (ulonglong)m_cache.length()));
    return pending() == nullptr && m_cache.is_empty();
  }

  bool is_finalized() const { return flags.finalized; }

  bool is_transactional() const { return flags.transactional; }

  Rows_log_event *pending() const { return m_pending; }

  void set_pending(Rows_log_event *const pending) { m_pending = pending; }

  void set_incident(void) { flags.incident = true; }

  bool has_incident(void) const { return flags.incident; }

  bool has_xid() const {
    // There should only be an XID event if we are transactional
    DBUG_ASSERT((flags.transactional && flags.with_xid) || !flags.with_xid);
    return flags.with_xid;
  }

  bool is_trx_cache() const { return flags.transactional; }

  my_off_t get_byte_position() const { return m_cache.length(); }

  void cache_state_checkpoint(my_off_t pos_to_checkpoint) {
    // We only need to store the cache state for pos > 0
    if (pos_to_checkpoint) {
      cache_state state;
      state.with_rbr = flags.with_rbr;
      state.with_sbr = flags.with_sbr;
      state.with_start = flags.with_start;
      state.with_end = flags.with_end;
      state.with_content = flags.with_content;
      state.event_counter = event_counter;
      cache_state_map[pos_to_checkpoint] = state;
    }
  }

  void cache_state_rollback(my_off_t pos_to_rollback) {
    if (pos_to_rollback) {
      std::map<my_off_t, cache_state>::iterator it;
      it = cache_state_map.find(pos_to_rollback);
      if (it != cache_state_map.end()) {
        flags.with_rbr = it->second.with_rbr;
        flags.with_sbr = it->second.with_sbr;
        flags.with_start = it->second.with_start;
        flags.with_end = it->second.with_end;
        flags.with_content = it->second.with_content;
        event_counter = it->second.event_counter;
      } else
        DBUG_ASSERT(it == cache_state_map.end());
    }
    // Rolling back to pos == 0 means cleaning up the cache.
    else {
      flags.with_rbr = false;
      flags.with_sbr = false;
      flags.with_start = false;
      flags.with_end = false;
      flags.with_content = false;
      event_counter = 0;
    }
  }

  /**
     Reset the cache to unused state when the transaction is finished. It
     drops all data in the cache and clears the flags of the transaction state.
  */
  virtual void reset() {
    compute_statistics();
    remove_pending_event();

    if (m_cache.reset()) {
      LogErr(WARNING_LEVEL, ER_BINLOG_CANT_RESIZE_CACHE);
    }

    flags.incident = false;
    flags.with_xid = false;
    flags.immediate = false;
    flags.finalized = false;
    flags.with_sbr = false;
    flags.with_rbr = false;
    flags.with_start = false;
    flags.with_end = false;
    flags.with_content = false;

    /*
      The truncate function calls reinit_io_cache that calls my_b_flush_io_cache
      which may increase disk_writes. This breaks the disk_writes use by the
      binary log which aims to compute the ratio between in-memory cache usage
      and disk cache usage. To avoid this undesirable behavior, we reset the
      variable after truncating the cache.
    */
    cache_state_map.clear();
    event_counter = 0;
    m_compressed_size = 0;
    m_decompressed_size = 0;
    m_compression_type = binary_log::transaction::compression::NONE;
    DBUG_ASSERT(is_binlog_empty());
  }

  /**
    Returns information about the cache content with respect to
    the binlog_format of the events.

    This will be used to set a flag on GTID_LOG_EVENT stating that the
    transaction may have SBR statements or not, but the binlog dump
    will show this flag as "rbr_only" when it is not set. That's why
    an empty transaction should return true below, or else an empty
    transaction would be assumed as "rbr_only" even not having RBR
    events.

    When dumping a binary log content using mysqlbinlog client program,
    for any transaction assumed as "rbr_only" it will be printed a
    statement changing the transaction isolation level to READ COMMITTED.
    It doesn't make sense to have an empty transaction "requiring" this
    isolation level change.

    @return true  The cache have SBR events or is empty.
    @return false The cache contains a transaction with no SBR events.
   */
  bool may_have_sbr_stmts() { return flags.with_sbr || !flags.with_rbr; }

  /**
    Check if the binlog cache contains an empty transaction, which has
    two binlog events "BEGIN" and "COMMIT".

    @return true  The binlog cache contains an empty transaction.
    @return false Otherwise.
  */
  bool has_empty_transaction() {
    /*
      The empty transaction has two events in trx/stmt binlog cache
      and no changes: one is a transaction start and other is a transaction
      end (there should be no SBR changing content and no RBR events).
    */
    if (flags.with_start &&   // Has transaction start statement
        flags.with_end &&     // Has transaction end statement
        !flags.with_content)  // Has no other content than START/END
    {
      DBUG_ASSERT(event_counter == 2);  // Two events in the cache only
      DBUG_ASSERT(!flags.with_sbr);     // No statements changing content
      DBUG_ASSERT(!flags.with_rbr);     // No rows changing content
      DBUG_ASSERT(!flags.immediate);    // Not a DDL
      DBUG_ASSERT(
          !flags.with_xid);  // Not a XID trx and not an atomic DDL Query
      return true;
    }
    return false;
  }

  /**
    Check if the binlog cache is empty or contains an empty transaction,
    which has two binlog events "BEGIN" and "COMMIT".

    @return true  The binlog cache is empty or contains an empty transaction.
    @return false Otherwise.
  */
  bool is_empty_or_has_empty_transaction() {
    return is_binlog_empty() || has_empty_transaction();
  }

 protected:
  /*
    This structure should have all cache variables/flags that should be restored
    when a ROLLBACK TO SAVEPOINT statement be executed.
  */
  struct cache_state {
    bool with_sbr;
    bool with_rbr;
    bool with_start;
    bool with_end;
    bool with_content;
    size_t event_counter;
  };
  /*
    For every SAVEPOINT used, we will store a cache_state for the current
    binlog cache position. So, if a ROLLBACK TO SAVEPOINT is used, we can
    restore the cache_state values after truncating the binlog cache.
  */
  std::map<my_off_t, cache_state> cache_state_map;
  /*
    In order to compute the transaction size (because of possible extra checksum
    bytes), we need to keep track of how many events are in the binlog cache.
  */
  size_t event_counter = 0;

  size_t m_compressed_size = 0;
  size_t m_decompressed_size = 0;
  binary_log::transaction::compression::type m_compression_type =
      binary_log::transaction::compression::type::NONE;
  /*
    It truncates the cache to a certain position. This includes deleting the
    pending event. It corresponds to rollback statement or rollback to
    a savepoint. It doesn't change transaction state.
   */
  void truncate(my_off_t pos) {
    DBUG_PRINT("info", ("truncating to position %lu", (ulong)pos));
    remove_pending_event();

    // TODO: check the return value.
    (void)m_cache.truncate(pos);
  }

  /**
     Flush pending event to the cache buffer.
   */
  int flush_pending_event(THD *thd) {
    if (m_pending) {
      m_pending->set_flags(Rows_log_event::STMT_END_F);
      if (int error = write_event(thd, m_pending)) return error;
      thd->clear_binlog_table_maps();
    }
    return 0;
  }

 public:
  /**
    Remove the pending event.
   */
  int remove_pending_event() {
    delete m_pending;
    m_pending = nullptr;
    return 0;
  }

 protected:
  struct Flags {
    /*
      Defines if this is either a trx-cache or stmt-cache, respectively, a
      transactional or non-transactional cache.
    */
    bool transactional : 1;

    /*
      This indicates that some events did not get into the cache and most likely
      it is corrupted.
    */
    bool incident : 1;

    /*
      This indicates that the cache should be written without BEGIN/END.
    */
    bool immediate : 1;

    /*
      This flag indicates that the buffer was finalized and has to be
      flushed to disk.
     */
    bool finalized : 1;

    /*
      This indicates that either the cache contain an XID event, or it's
      an atomic DDL Query-log-event. In the latter case the flag is set up
      on the statement level, namely when the Query-log-event is cached
      at time the DDL transaction is not committing.
      The flag therefore gets reset when the cache is cleaned due to
      the statement rollback, e.g in case of a DDL post-caching execution
      error.
      Any statement scope flag among other things must consider its
      reset policy when the statement is rolled back.
    */
    bool with_xid : 1;

    /*
      This indicates that the cache contain statements changing content.
    */
    bool with_sbr : 1;

    /*
      This indicates that the cache contain RBR event changing content.
    */
    bool with_rbr : 1;

    /*
      This indicates that the cache contain s transaction start statement.
    */
    bool with_start : 1;

    /*
      This indicates that the cache contain a transaction end event.
    */
    bool with_end : 1;

    /*
      This indicates that the cache contain content other than START/END.
    */
    bool with_content : 1;
  } flags;

  virtual bool compress(THD *);

 private:
  /*
    Storage for byte data. This binlog_cache_data will serialize
    events into bytes and put them into m_cache.
  */
  Binlog_cache_storage m_cache;

  /*
    Pending binrows event. This event is the event where the rows are currently
    written.
   */
  Rows_log_event *m_pending;

  /**
    This function computes binlog cache and disk usage.
  */
  void compute_statistics() {
    if (!is_binlog_empty()) {
      (*ptr_binlog_cache_use)++;
      if (m_cache.disk_writes() != 0) (*ptr_binlog_cache_disk_use)++;
    }
  }

  /*
    Stores a pointer to the status variable that keeps track of the in-memory
    cache usage. This corresponds to either
      . binlog_cache_use or binlog_stmt_cache_use.
  */
  ulong *ptr_binlog_cache_use;

  /*
    Stores a pointer to the status variable that keeps track of the disk
    cache usage. This corresponds to either
      . binlog_cache_disk_use or binlog_stmt_cache_disk_use.
  */
  ulong *ptr_binlog_cache_disk_use;

  binlog_cache_data &operator=(const binlog_cache_data &info);
  binlog_cache_data(const binlog_cache_data &info);
};

class binlog_stmt_cache_data : public binlog_cache_data {
 public:
  binlog_stmt_cache_data(bool trx_cache_arg, ulong *ptr_binlog_cache_use_arg,
                         ulong *ptr_binlog_cache_disk_use_arg)
      : binlog_cache_data(trx_cache_arg, ptr_binlog_cache_use_arg,
                          ptr_binlog_cache_disk_use_arg) {}

  using binlog_cache_data::finalize;

  int finalize(THD *thd);
};

int binlog_stmt_cache_data::finalize(THD *thd) {
  if (flags.immediate) {
    if (int error = finalize(thd, nullptr)) return error;
  } else {
    Query_log_event end_evt(thd, STRING_WITH_LEN("COMMIT"), false, false, true,
                            0, true);
    if (int error = finalize(thd, &end_evt)) return error;
  }
  return 0;
}

class binlog_trx_cache_data : public binlog_cache_data {
 public:
  binlog_trx_cache_data(bool trx_cache_arg, ulong *ptr_binlog_cache_use_arg,
                        ulong *ptr_binlog_cache_disk_use_arg)
      : binlog_cache_data(trx_cache_arg, ptr_binlog_cache_use_arg,
                          ptr_binlog_cache_disk_use_arg),
        m_cannot_rollback(false),
        before_stmt_pos(MY_OFF_T_UNDEF) {}

  void reset() {
    DBUG_TRACE;
    DBUG_PRINT("enter", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    m_cannot_rollback = false;
    before_stmt_pos = MY_OFF_T_UNDEF;
    binlog_cache_data::reset();
    DBUG_PRINT("return", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    return;
  }

  bool cannot_rollback() const { return m_cannot_rollback; }

  void set_cannot_rollback() { m_cannot_rollback = true; }

  my_off_t get_prev_position() const { return before_stmt_pos; }

  void set_prev_position(my_off_t pos) {
    DBUG_TRACE;
    DBUG_PRINT("enter", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    before_stmt_pos = pos;
    cache_state_checkpoint(before_stmt_pos);
    DBUG_PRINT("return", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    return;
  }

  void restore_prev_position() {
    DBUG_TRACE;
    DBUG_PRINT("enter", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    binlog_cache_data::truncate(before_stmt_pos);
    cache_state_rollback(before_stmt_pos);
    before_stmt_pos = MY_OFF_T_UNDEF;
    /*
      Binlog statement rollback clears with_xid now as the atomic DDL statement
      marker which can be set as early as at event creation and caching.
    */
    flags.with_xid = false;
    DBUG_PRINT("return", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    return;
  }

  void restore_savepoint(my_off_t pos) {
    DBUG_TRACE;
    DBUG_PRINT("enter", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    binlog_cache_data::truncate(pos);
    if (pos <= before_stmt_pos) before_stmt_pos = MY_OFF_T_UNDEF;
    cache_state_rollback(pos);
    DBUG_PRINT("return", ("before_stmt_pos: %llu", (ulonglong)before_stmt_pos));
    return;
  }

  using binlog_cache_data::truncate;

  int truncate(THD *thd, bool all);

 private:
  /*
    It will be set true if any statement which cannot be rolled back safely
    is put in trx_cache.
  */
  bool m_cannot_rollback;

  /*
    Binlog position before the start of the current statement.
  */
  my_off_t before_stmt_pos;

  binlog_trx_cache_data &operator=(const binlog_trx_cache_data &info);
  binlog_trx_cache_data(const binlog_trx_cache_data &info);
};

class binlog_cache_mngr {
 public:
  binlog_cache_mngr(ulong *ptr_binlog_stmt_cache_use_arg,
                    ulong *ptr_binlog_stmt_cache_disk_use_arg,
                    ulong *ptr_binlog_cache_use_arg,
                    ulong *ptr_binlog_cache_disk_use_arg)
      : stmt_cache(false, ptr_binlog_stmt_cache_use_arg,
                   ptr_binlog_stmt_cache_disk_use_arg),
        trx_cache(true, ptr_binlog_cache_use_arg,
                  ptr_binlog_cache_disk_use_arg),
        has_logged_xid(false) {}

  bool init() {
    return stmt_cache.open(binlog_stmt_cache_size,
                           max_binlog_stmt_cache_size) ||
           trx_cache.open(binlog_cache_size, max_binlog_cache_size);
  }

  binlog_cache_data *get_binlog_cache_data(bool is_transactional) {
    if (is_transactional)
      return &trx_cache;
    else
      return &stmt_cache;
  }

  Binlog_cache_storage *get_stmt_cache() { return stmt_cache.get_cache(); }
  Binlog_cache_storage *get_trx_cache() { return trx_cache.get_cache(); }
  /**
    Convenience method to check if both caches are empty.
   */
  bool is_binlog_empty() const {
    return stmt_cache.is_binlog_empty() && trx_cache.is_binlog_empty();
  }

  /*
    clear stmt_cache and trx_cache if they are not empty
  */
  void reset() {
    if (!stmt_cache.is_binlog_empty()) stmt_cache.reset();
    if (!trx_cache.is_binlog_empty()) trx_cache.reset();
  }

#ifndef DBUG_OFF
  bool dbug_any_finalized() const {
    return stmt_cache.is_finalized() || trx_cache.is_finalized();
  }
#endif

  /*
    Convenience method to flush both caches to the binary log.

    @param bytes_written Pointer to variable that will be set to the
                         number of bytes written for the flush.
    @param wrote_xid     Pointer to variable that will be set to @c
                         true if any XID event was written to the
                         binary log. Otherwise, the variable will not
                         be touched.
    @return Error code on error, zero if no error.
   */
  int flush(THD *thd, my_off_t *bytes_written, bool *wrote_xid) {
    my_off_t stmt_bytes = 0;
    my_off_t trx_bytes = 0;
    DBUG_ASSERT(stmt_cache.has_xid() == 0);
    int error = stmt_cache.flush(thd, &stmt_bytes, wrote_xid);
    if (error) return error;
    DEBUG_SYNC(thd, "after_flush_stm_cache_before_flush_trx_cache");
    error = trx_cache.flush(thd, &trx_bytes, wrote_xid);
    if (error) return error;
    *bytes_written = stmt_bytes + trx_bytes;
    return 0;
  }

  /**
    Check if at least one of transacaction and statement binlog caches
    contains an empty transaction, other one is empty or contains an
    empty transaction.

    @return true  At least one of transacaction and statement binlog
                  caches an empty transaction, other one is emptry
                  or contains an empty transaction.
    @return false Otherwise.
  */
  bool has_empty_transaction() {
    return (trx_cache.is_empty_or_has_empty_transaction() &&
            stmt_cache.is_empty_or_has_empty_transaction() &&
            !is_binlog_empty());
  }

  binlog_stmt_cache_data stmt_cache;
  binlog_trx_cache_data trx_cache;
  /*
    The bool flag is for preventing do_binlog_xa_commit_rollback()
    execution twice which can happen for "external" xa commit/rollback.
  */
  bool has_logged_xid;

 private:
  binlog_cache_mngr &operator=(const binlog_cache_mngr &info);
  binlog_cache_mngr(const binlog_cache_mngr &info);
};

static binlog_cache_mngr *thd_get_cache_mngr(const THD *thd) {
  /*
    If opt_bin_log is not set, binlog_hton->slot == -1 and hence
    thd_get_ha_data(thd, hton) segfaults.
  */
  DBUG_ASSERT(opt_bin_log);
  return (binlog_cache_mngr *)thd_get_ha_data(thd, binlog_hton);
}

/**
  Checks if the BINLOG_CACHE_SIZE's value is greater than MAX_BINLOG_CACHE_SIZE.
  If this happens, the BINLOG_CACHE_SIZE is set to MAX_BINLOG_CACHE_SIZE.
*/
void check_binlog_cache_size(THD *thd) {
  if (binlog_cache_size > max_binlog_cache_size) {
    if (thd) {
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_BINLOG_CACHE_SIZE_GREATER_THAN_MAX,
          ER_THD(thd, ER_BINLOG_CACHE_SIZE_GREATER_THAN_MAX),
          (ulong)binlog_cache_size, (ulong)max_binlog_cache_size);
    } else {
      LogErr(WARNING_LEVEL, ER_BINLOG_CACHE_SIZE_TOO_LARGE, binlog_cache_size,
             (ulong)max_binlog_cache_size);
    }
    binlog_cache_size = static_cast<ulong>(max_binlog_cache_size);
  }
}

/**
  Checks if the BINLOG_STMT_CACHE_SIZE's value is greater than
  MAX_BINLOG_STMT_CACHE_SIZE. If this happens, the BINLOG_STMT_CACHE_SIZE is set
  to MAX_BINLOG_STMT_CACHE_SIZE.
*/
void check_binlog_stmt_cache_size(THD *thd) {
  if (binlog_stmt_cache_size > max_binlog_stmt_cache_size) {
    if (thd) {
      push_warning_printf(
          thd, Sql_condition::SL_WARNING,
          ER_BINLOG_STMT_CACHE_SIZE_GREATER_THAN_MAX,
          ER_THD(thd, ER_BINLOG_STMT_CACHE_SIZE_GREATER_THAN_MAX),
          (ulong)binlog_stmt_cache_size, (ulong)max_binlog_stmt_cache_size);
    } else {
      LogErr(WARNING_LEVEL, ER_BINLOG_STMT_CACHE_SIZE_TOO_LARGE,
             binlog_stmt_cache_size, (ulong)max_binlog_stmt_cache_size);
    }
    binlog_stmt_cache_size = static_cast<ulong>(max_binlog_stmt_cache_size);
  }
}

/**
  Updates the HLC tracked by the binlog to a value greater than or equal to the
  one specified in minimum_hlc_ns global system variable
  */
void update_binlog_hlc() {
  // Update HLC
  mysql_bin_log.update_hlc(minimum_hlc_ns);
}

/**
 Check whether binlog_hton has valid slot and enabled
*/
bool binlog_enabled() {
  return (binlog_hton && binlog_hton->slot != HA_SLOT_UNDEF);
}

/*
 Save position of binary log transaction cache.

 SYNPOSIS
   binlog_trans_log_savepos()

   thd      The thread to take the binlog data from
   pos      Pointer to variable where the position will be stored

 DESCRIPTION

   Save the current position in the binary log transaction cache into
   the variable pointed to by 'pos'
*/

static void binlog_trans_log_savepos(THD *thd, my_off_t *pos) {
  DBUG_TRACE;
  DBUG_ASSERT(pos != nullptr);
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);
  DBUG_ASSERT(mysql_bin_log.is_open());
  *pos = cache_mngr->trx_cache.get_byte_position();
  DBUG_PRINT("return", ("position: %lu", (ulong)*pos));
  cache_mngr->trx_cache.cache_state_checkpoint(*pos);
}

static void binlog_dummy_recover_binlog_pos(handlerton *, Gtid *, char *,
                                            my_off_t *) {}

static int binlog_dummy_recover(handlerton *, XA_recover_txn *, uint,
                                MEM_ROOT *) {
  return 0;
}

/**
  Auxiliary class to copy serialized events to the binary log and
  correct some of the fields that are not known until just before
  writing the event.

  This class allows feeding events in parts, so it is practical to use
  in do_write_cache() which reads events from an IO_CACHE where events
  may span mutiple cache pages.

  The following fields are fixed before writing the event:
  - end_log_pos is set
  - the checksum is computed if checksums are enabled
  - the length is incremented by the checksum size if checksums are enabled
*/
class Binlog_event_writer : public Basic_ostream {
  MYSQL_BIN_LOG::Binlog_ofile *m_binlog_file;
  bool have_checksum;
  ha_checksum initial_checksum;
  ha_checksum checksum;
  uint32 end_log_pos;
  uchar header[LOG_EVENT_HEADER_LEN];
  my_off_t header_len = 0;
  uint32 event_len = 0;

 public:
  /**
    Constructs a new Binlog_event_writer. Should be called once before
    starting to flush the transaction or statement cache to the
    binlog.

    @param binlog_file to write to.
  */
  Binlog_event_writer(MYSQL_BIN_LOG::Binlog_ofile *binlog_file)
      : m_binlog_file(binlog_file),
        have_checksum(binlog_checksum_options !=
                      binary_log::BINLOG_CHECKSUM_ALG_OFF),
        initial_checksum(my_checksum(0L, nullptr, 0)),
        checksum(initial_checksum),
        end_log_pos(binlog_file->position()) {
    // Simulate checksum error
    if (DBUG_EVALUATE_IF("fault_injection_crc_value", 1, 0)) checksum--;
  }

  void update_header() {
    event_len = uint4korr(header + EVENT_LEN_OFFSET);

    // Increase end_log_pos
    end_log_pos += event_len;

    // Update event length if it has checksum
    if (have_checksum) {
      int4store(header + EVENT_LEN_OFFSET, event_len + BINLOG_CHECKSUM_LEN);
      end_log_pos += BINLOG_CHECKSUM_LEN;
    }

    // Store end_log_pos
    int4store(header + LOG_POS_OFFSET, end_log_pos);
    // update the checksum
    if (have_checksum) checksum = my_checksum(checksum, header, header_len);
  }

  bool write(const unsigned char *buffer, my_off_t length) {
    DBUG_TRACE;

    while (length > 0) {
      /* Write event header into binlog */
      if (event_len == 0) {
        /* data in the buf may be smaller than header size.*/
        uint32 header_incr =
            std::min<uint32>(LOG_EVENT_HEADER_LEN - header_len, length);

        memcpy(header + header_len, buffer, header_incr);
        header_len += header_incr;
        buffer += header_incr;
        length -= header_incr;

        if (header_len == LOG_EVENT_HEADER_LEN) {
          update_header();
          if (m_binlog_file->write(header, header_len)) return true;

          event_len -= header_len;
          header_len = 0;
        }
      } else {
        my_off_t write_bytes = std::min<uint64>(length, event_len);

        if (m_binlog_file->write(buffer, write_bytes)) return true;

        // update the checksum
        if (have_checksum)
          checksum = my_checksum(checksum, buffer, write_bytes);

        event_len -= write_bytes;
        length -= write_bytes;
        buffer += write_bytes;

        // The whole event is copied, now add the checksum
        if (have_checksum && event_len == 0) {
          uchar checksum_buf[BINLOG_CHECKSUM_LEN];

          int4store(checksum_buf, checksum);
          if (m_binlog_file->write(checksum_buf, BINLOG_CHECKSUM_LEN))
            return true;
          checksum = initial_checksum;
        }
      }
    }
    return false;
  }
  /**
    Returns true if per event checksum is enabled.
  */
  bool is_checksum_enabled() { return have_checksum; }
};

/*
  this function is mostly a placeholder.
  conceptually, binlog initialization (now mostly done in MYSQL_BIN_LOG::open)
  should be moved here.
*/

static int binlog_init(void *p) {
  binlog_hton = (handlerton *)p;
  binlog_hton->state = opt_bin_log ? SHOW_OPTION_YES : SHOW_OPTION_NO;
  binlog_hton->db_type = DB_TYPE_BINLOG;
  binlog_hton->savepoint_offset = sizeof(my_off_t);
  binlog_hton->close_connection = binlog_close_connection;
  binlog_hton->savepoint_set = binlog_savepoint_set;
  binlog_hton->savepoint_rollback = binlog_savepoint_rollback;
  binlog_hton->savepoint_rollback_can_release_mdl =
      binlog_savepoint_rollback_can_release_mdl;
  binlog_hton->commit = binlog_commit;
  binlog_hton->commit_by_xid = binlog_xa_commit;
  binlog_hton->rollback = binlog_rollback;
  binlog_hton->rollback_by_xid = binlog_xa_rollback;
  binlog_hton->prepare = binlog_prepare;
  binlog_hton->recover_binlog_pos = binlog_dummy_recover_binlog_pos;
  binlog_hton->recover = binlog_dummy_recover;
  binlog_hton->flags = HTON_NOT_USER_SELECTABLE | HTON_HIDDEN;

  latency_histogram_init(&histogram_raft_trx_wait, "125us");

  latency_histogram_init(&histogram_binlog_fsync,
                         histogram_step_size_binlog_fsync);
  counter_histogram_init(&histogram_binlog_group_commit,
                         opt_histogram_step_size_binlog_group_commit);

  return 0;
}

static int binlog_deinit(void *) {
  /* Using binlog as TC after the binlog has been unloaded, won't work */
  if (tc_log == &mysql_bin_log) tc_log = nullptr;
  binlog_hton = nullptr;
  return 0;
}

static int binlog_close_connection(handlerton *, THD *thd) {
  DBUG_TRACE;
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);
  DBUG_ASSERT(cache_mngr->is_binlog_empty());
  DBUG_PRINT("debug", ("Set ha_data slot %d to 0x%llx", binlog_hton->slot,
                       (ulonglong) nullptr));
  thd_set_ha_data(thd, binlog_hton, nullptr);
  cache_mngr->~binlog_cache_mngr();
  my_free(cache_mngr);
  return 0;
}

int binlog_cache_data::write_event(THD *thd, Log_event *ev,
                                   bool write_meta_data_event) {
  DBUG_TRACE;

  if (ev != nullptr) {
    DBUG_EXECUTE_IF("simulate_disk_full_at_binlog_cache_write",
                    { DBUG_SET("+d,simulate_no_free_space_error"); });
    // case: write meta data event before the real event
    // see @opt_binlog_trx_meta_data
    if (write_meta_data_event) {
      const std::string metadata = thd->gen_trx_metadata();
      Rows_query_log_event metadata_ev(thd, metadata.c_str(),
                                       metadata.length());
      if (binary_event_serialize(&metadata_ev, &m_cache) != 0) return 1;
    }

    if (binary_event_serialize(ev, &m_cache)) {
      return 1;
    }

    DBUG_EXECUTE_IF("simulate_disk_full_at_binlog_cache_write",
                    // this flag is cleared by my_write.cc but we clear it
                    // explicitly in case if the even didn't hit my_write.cc
                    // so the flag won't affect not targeted calls
                    { DBUG_SET("-d,simulate_no_free_space_error"); });

    if (ev->get_type_code() == binary_log::XID_EVENT) flags.with_xid = true;
    if (ev->is_using_immediate_logging()) flags.immediate = true;
    /* DDL gets marked as xid-requiring at its caching. */
    if (is_atomic_ddl_event(ev)) flags.with_xid = true;
    /* With respect to the event type being written */
    if (ev->is_sbr_logging_format()) flags.with_sbr = true;
    if (ev->is_rbr_logging_format()) flags.with_rbr = true;
    /* With respect to empty transactions */
    if (ev->starts_group()) flags.with_start = true;
    if (ev->ends_group()) flags.with_end = true;
    if (!ev->starts_group() && !ev->ends_group()) flags.with_content = true;
    event_counter++;
    DBUG_PRINT("debug",
               ("event_counter= %lu", static_cast<ulong>(event_counter)));
  }
  return 0;
}

/**
 * Assign HLC timestamp to a thd in group commit
 *
 * @param thd - the THD in group commit
 *
 * @return false on success, true on failure
 */
bool MYSQL_BIN_LOG::assign_hlc(THD *thd) {
  if (!enable_binlog_hlc) {
    /* HLC not enabled, return */
    thd->should_write_hlc = false;
    return false;
  }

  /* Get next HLC timestamp on master instance. On slave instance
   * HLC timestamp would be the same as seen in the binlog stream i.e slave
   * retains master's HLC timestamp */
  if (!thd->slave_thread && !thd->rli_fake)
    thd->hlc_time_ns_next = mysql_bin_log.get_next_hlc();

  thd->should_write_hlc = true;

  return false;
}

/**
 * Write HLC timestamp of a thd in group commit to binlog
 *
 * @param thd - the THD in group commit
 * @param cache_data - The cache that is being written dusring flush stage
 * @param writer - Binlog writer (metadata event with HLC will be written here)
 * @param obuffer - if not null, metadata event will be written here (instead
 *                  of writing to writer)
 * @param wrote_hlc - Will be set to true if HLC was written to the log file
 *
 * @return false on success, true on failure
 */
bool MYSQL_BIN_LOG::write_hlc(THD *thd, binlog_cache_data *cache_data,
                              Binlog_event_writer *writer,
                              Binlog_cache_storage *obuffer, bool *wrote_hlc) {
  if (!thd->should_write_hlc) {
    /* HLC was not assigned to this thd */
    thd->hlc_time_ns_next = 0;
    return false;
  }

  /* HLC written, clear state */
  thd->should_write_hlc = false;
  if (wrote_hlc) *wrote_hlc = true;

  Metadata_log_event metadata_ev(thd, cache_data->is_trx_cache(),
                                 thd->hlc_time_ns_next);

  if (thd->rli_slave || thd->rli_fake) {
    // When a Metadata event with Raft OpId is picked up from
    // relay log and applied, ev->apply_event in rpl_slave.cc stashes
    // the raft term and index from the event into the THD. Here we
    // pick it up to pass the Raft term and index through to the metadata
    // event of binlog/apply side. Although confusing (as to why we are
    // adding raft metadata even in non-raft cases), this is required
    // to align with current approach of how a new non-raft instance
    // is added to existing raft ring. OpId can only be present in
    // raft rings, hence the exposure of this code is to instances
    // which are tailing raft rings or raft members which are now passing
    // OpId to apply log as well. In all other cases Raft term and index
    // is expected to be -1,-1
    int64_t raft_term, raft_index;
    thd->get_trans_marker(&raft_term, &raft_index);
    if (raft_term != -1 && raft_index != -1)
      metadata_ev.set_raft_term_and_index(raft_term, raft_index);
  }

  bool result = false;
  if (obuffer) {
    (void)metadata_ev.write(obuffer);
  } else {
    result = metadata_ev.write(writer);
  }

  /* Update session tracker with hlc timestamp of this trx */
  auto tracker = thd->session_tracker.get_tracker(SESSION_RESP_ATTR_TRACKER);
  if (!result && thd->variables.response_attrs_contain_hlc &&
      tracker->is_enabled()) {
    LEX_CSTRING key = {STRING_WITH_LEN("hlc_ts")};
    std::string value_str = std::to_string(thd->hlc_time_ns_next);
    const LEX_CSTRING value = {value_str.c_str(), value_str.length()};
    tracker->mark_as_changed(thd, &key, &value);
  }

  return result;
}

bool MYSQL_BIN_LOG::assign_automatic_gtids_to_flush_group(THD *first_seen) {
  DBUG_TRACE;
  bool error = false;
  bool is_global_sid_locked = false;
  rpl_sidno locked_sidno = 0;

  for (THD *head = first_seen; head; head = head->next_to_commit) {
    DBUG_ASSERT(head->variables.gtid_next.type != UNDEFINED_GTID);

    /* Generate GTID */
    if (head->variables.gtid_next.type == AUTOMATIC_GTID) {
      if (!is_global_sid_locked) {
        global_sid_lock->rdlock();
        is_global_sid_locked = true;
      }
      if (gtid_state->generate_automatic_gtid(
              head,
              head->get_transaction()->get_rpl_transaction_ctx()->get_sidno(),
              head->get_transaction()->get_rpl_transaction_ctx()->get_gno(),
              &locked_sidno) != RETURN_STATUS_OK) {
        head->commit_error = THD::CE_FLUSH_ERROR;
        error = true;
      }
    } else {
      DBUG_PRINT("info",
                 ("thd->variables.gtid_next.type=%d "
                  "thd->owned_gtid.sidno=%d",
                  head->variables.gtid_next.type, head->owned_gtid.sidno));
      if (head->variables.gtid_next.type == ASSIGNED_GTID)
        DBUG_ASSERT(head->owned_gtid.sidno > 0);
      else {
        DBUG_ASSERT(head->variables.gtid_next.type == ANONYMOUS_GTID);
        DBUG_ASSERT(head->owned_gtid.sidno == THD::OWNED_SIDNO_ANONYMOUS);
      }
    }

    // Assign HLC timestamp to the thd's in the group
    assign_hlc(head);
  }

  if (locked_sidno > 0) gtid_state->unlock_sidno(locked_sidno);

  if (is_global_sid_locked) global_sid_lock->unlock();

  return error;
}

/**
  Write the Gtid_log_event to the binary log (prior to writing the
  statement or transaction cache).

  @param thd Thread that is committing.
  @param cache_data The cache that is flushing.
  @param writer The event will be written to this Binlog_event_writer object.

  @retval false Success.
  @retval true Error.
*/
bool MYSQL_BIN_LOG::write_transaction(THD *thd, binlog_cache_data *cache_data,
                                      Binlog_event_writer *writer) {
  DBUG_TRACE;

  /*
    The GTID for the THD was assigned at
    assign_automatic_gtids_to_flush_group()
  */
  DBUG_ASSERT(thd->owned_gtid.sidno == THD::OWNED_SIDNO_ANONYMOUS ||
              thd->owned_gtid.sidno > 0);

  int64 sequence_number, last_committed;
  /* Generate logical timestamps for MTS */
  m_dependency_tracker.get_dependency(thd, sequence_number, last_committed);

  /*
    In case both the transaction cache and the statement cache are
    non-empty, both will be flushed in sequence and logged as
    different transactions. Then the second transaction must only
    be executed after the first one has committed. Therefore, we
    need to set last_committed for the second transaction equal to
    last_committed for the first transaction. This is done in
    binlog_cache_data::flush. binlog_cache_data::flush uses the
    condition trn_ctx->last_committed==SEQ_UNINIT to detect this
    situation, hence the need to set it here.
  */
  thd->get_transaction()->last_committed = SEQ_UNINIT;

  /*
    For delayed replication and also for the purpose of lag monitoring,
    we assume that the commit timestamp of the transaction is the time of
    executing this code (the time of writing the Gtid_log_event to the binary
    log).
  */
  ulonglong immediate_commit_timestamp = my_micro_time();

  /*
    When the original_commit_timestamp session variable is set to a value
    other than UNDEFINED_COMMIT_TIMESTAMP, it means that either the timestamp
    is known ( > 0 ) or the timestamp is not known ( == 0 ).
  */
  ulonglong original_commit_timestamp =
      thd->variables.original_commit_timestamp;
  /*
    When original_commit_timestamp == UNDEFINED_COMMIT_TIMESTAMP, we assume
    that:
    a) it is not known if this thread is a slave applier ( = 0 );
    b) this is a new transaction ( = immediate_commit_timestamp);
  */
  if (original_commit_timestamp == UNDEFINED_COMMIT_TIMESTAMP) {
    /*
      When applying a transaction using replication, assume that the
      original commit timestamp is not known (the transaction wasn't
      originated on the current server).
    */
    if (thd->slave_thread || thd->is_binlog_applier()) {
      original_commit_timestamp = 0;
    } else
    /* Assume that this transaction is original from this server */
    {
      DBUG_EXECUTE_IF("rpl_invalid_gtid_timestamp",
                      // add one our to the commit timestamps
                      immediate_commit_timestamp += 3600000000;);
      original_commit_timestamp = immediate_commit_timestamp;
    }
  } else {
    // Clear the session variable to have cleared states for next transaction.
    thd->variables.original_commit_timestamp = UNDEFINED_COMMIT_TIMESTAMP;
  }

  if (thd->slave_thread) {
    // log warning if the replication timestamps are invalid
    if (original_commit_timestamp > immediate_commit_timestamp &&
        !thd->rli_slave->get_c_rli()->gtid_timestamps_warning_logged) {
      LogErr(WARNING_LEVEL, ER_INVALID_REPLICATION_TIMESTAMPS);
      thd->rli_slave->get_c_rli()->gtid_timestamps_warning_logged = true;
    } else {
      if (thd->rli_slave->get_c_rli()->gtid_timestamps_warning_logged &&
          original_commit_timestamp <= immediate_commit_timestamp) {
        LogErr(WARNING_LEVEL, ER_RPL_TIMESTAMPS_RETURNED_TO_NORMAL);
        thd->rli_slave->get_c_rli()->gtid_timestamps_warning_logged = false;
      }
    }
  }

  uint32_t trx_immediate_server_version =
      do_server_version_int(::server_version);
  // Clear the session variable to have cleared states for next transaction.
  thd->variables.immediate_server_version = UNDEFINED_SERVER_VERSION;
  DBUG_EXECUTE_IF("fixed_server_version",
                  trx_immediate_server_version = 888888;);
  DBUG_EXECUTE_IF("gr_fixed_server_version",
                  trx_immediate_server_version = 777777;);

  /*
    When the original_server_version session variable is set to a value
    other than UNDEFINED_SERVER_VERSION, it means that either the
    server version is known or the server_version is not known
    (UNKNOWN_SERVER_VERSION).
  */
  uint32_t trx_original_server_version = thd->variables.original_server_version;

  /*
    When original_server_version == UNDEFINED_SERVER_VERSION, we assume
    that:
    a) it is not known if this thread is a slave applier ( = 0 );
    b) this is a new transaction ( = ::server_version);
  */
  if (trx_original_server_version == UNDEFINED_SERVER_VERSION) {
    /*
      When applying a transaction using replication, assume that the
      original server version is not known (the transaction wasn't
      originated on the current server).
    */
    if (thd->slave_thread || thd->is_binlog_applier()) {
      trx_original_server_version = UNKNOWN_SERVER_VERSION;
    } else
    /* Assume that this transaction is original from this server */
    {
      trx_original_server_version = trx_immediate_server_version;
    }
  } else {
    // Clear the session variable to have cleared states for next transaction.
    thd->variables.original_server_version = UNDEFINED_SERVER_VERSION;
  }
  Gtid_log_event gtid_event(
      thd, cache_data->is_trx_cache(), last_committed, sequence_number,
      cache_data->may_have_sbr_stmts(), original_commit_timestamp,
      immediate_commit_timestamp, trx_original_server_version,
      trx_immediate_server_version);

  // Set the transaction length, based on cache info
  gtid_event.set_trx_length_by_cache_size(cache_data->get_byte_position(),
                                          writer->is_checksum_enabled(),
                                          cache_data->get_event_counter());

  DBUG_PRINT("debug", ("cache_data->get_byte_position()= %llu",
                       cache_data->get_byte_position()));
  DBUG_PRINT("debug", ("cache_data->get_event_counter()= %lu",
                       static_cast<ulong>(cache_data->get_event_counter())));
  DBUG_PRINT("debug", ("writer->is_checksum_enabled()= %s",
                       YESNO(writer->is_checksum_enabled())));
  DBUG_PRINT("debug", ("gtid_event.get_event_length()= %lu",
                       static_cast<ulong>(gtid_event.get_event_length())));
  DBUG_PRINT("info",
             ("transaction_length= %llu", gtid_event.transaction_length));

  bool ret = false;
  if (!enable_raft_plugin || mysql_bin_log.is_apply_log) {
    ret = gtid_event.write(writer);
    if (ret) goto end;

    ret = write_hlc(thd, cache_data, writer, nullptr);
    if (ret) goto end;

    /*
      finally write the transaction data, if it was not compressed
      and written as part of the gtid event already
    */
    ret = mysql_bin_log.write_cache(thd, cache_data, writer);
  } else {
    std::unique_ptr<Binlog_cache_storage> temp_binlog_cache =
        std::make_unique<Binlog_cache_storage>();

    temp_binlog_cache->open(
        cache_data->is_transactional() ? binlog_cache_size
                                       : binlog_stmt_cache_size,
        cache_data->is_transactional() ? max_binlog_cache_size
                                       : max_binlog_stmt_cache_size);

    (void)gtid_event.write(temp_binlog_cache.get());

    bool wrote_hlc = false;
    ret =
        write_hlc(thd, cache_data, writer, temp_binlog_cache.get(), &wrote_hlc);
    DBUG_ASSERT(!ret);

    thd->commit_consensus_error = false;
    // TODO(luqun): perf concern? merge in plugin?
    cache_data->get_cache()->copy_to(temp_binlog_cache.get());
    ret = RUN_HOOK_STRICT(raft_replication, before_flush,
                          (thd, temp_binlog_cache->get_io_cache(),
                           RaftReplicateMsgOpType::OP_TYPE_TRX));

    DBUG_EXECUTE_IF("fail_binlog_flush_raft", { ret = 1; });

    /*
     * before_flush hook failing is a guarantee by raft that any subsequent
     * replicate message sent to raft (through before_flush hook) fails (in
     * this group and in subsequent groups). In other words, raft will
     * initiate a step down and will not take any more writes. This
     * is necessary condition to avoid having holes or duplicates in
     * executed_gtid
     */
    if (ret) {
      // Calling into mysql_raft plugin failed. Set commit consensus error.
      // This will ensure that if this THD's trx is allowed to proceed to
      // commit stage, then we rollback the trx
      thd->commit_consensus_error = true;
      thd->commit_error = THD::CE_FLUSH_ERROR;
    }

    mysql_bin_log.post_write(thd, cache_data, ret);
  }

  if (!ret) {
    // update stats if monitoring is active
    binlog::global_context.monitoring_context()
        .transaction_compression()
        .update(binlog::monitoring::log_type::BINARY,
                cache_data->get_compression_type(), thd->owned_gtid,
                gtid_event.immediate_commit_timestamp,
                cache_data->get_compressed_size(),
                cache_data->get_decompressed_size());
  }

end:
  return ret;
}

int MYSQL_BIN_LOG::gtid_end_transaction(THD *thd) {
  DBUG_TRACE;

  DBUG_PRINT("info", ("query=%s", thd->query().str));

  if (thd->owned_gtid.sidno > 0) {
    DBUG_ASSERT(thd->variables.gtid_next.type == ASSIGNED_GTID);

    if (!opt_bin_log || (thd->slave_thread && !opt_log_slave_updates)) {
      /*
        If the binary log is disabled for this thread (either by
        log_bin=0 or sql_log_bin=0 or by log_slave_updates=0 for a
        slave thread), then the statement must not be written to the
        binary log.  In this case, we just save the GTID into the
        table directly.

        (This only happens for DDL, since DML will save the GTID into
        table and release ownership inside ha_commit_trans.)
      */
      if (gtid_state->save(thd) != 0) {
        gtid_state->update_on_rollback(thd);
        return 1;
      } else if (!has_commit_order_manager(thd)) {
        /*
          The gtid_state->save implicitly performs the commit, in the following
          stack:
            Gtid_state::save ->
            Gtid_table_persistor::save ->
            Gtid_table_access_context::deinit ->
            System_table_access::close_table ->
            ha_commit_trans ->
            Relay_log_info::pre_commit ->
            Slave_worker::commit_positions(THD*) ->
            Slave_worker::commit_positions(THD*,Log_event*,...) ->
            Slave_worker::flush_info ->
            Rpl_info_handler::flush_info ->
            Rpl_info_table::do_flush_info ->
            Rpl_info_table_access::close_table ->
            System_table_access::close_table ->
            ha_commit_trans ->
            MYSQL_BIN_LOG::commit ->
            ha_commit_low

          If slave-preserve-commit-order is disabled, it does not call
          update_on_commit from this stack. The reason is as follows:

          In the normal case of MYSQL_BIN_LOG::commit, where the transaction is
          going to be written to the binary log, it invokes
          MYSQL_BIN_LOG::ordered_commit, which updates the GTID state (the call
          gtid_state->update_commit_group(first) in process_commit_stage_queue).
          However, when MYSQL_BIN_LOG::commit is invoked from this stack, it is
          because the transaction is not going to be written to the binary log,
          and then MYSQL_BIN_LOG::commit has a special case that calls
          ha_commit_low directly, skipping ordered_commit. Therefore, the GTID
          state is not updated in this stack.

          On the other hand, if slave-preserve-commit-order is enabled, the
          logic that orders commit carries out a subset of the binlog group
          commit from within ha_commit_low, and this includes updating the GTID
          state. In particular, there is the following call stack under
          ha_commit_low:

            ha_commit_low ->
            Commit_order_manager::wait_and_finish ->
            Commit_order_manager::finish ->
            Commit_order_manager::flush_engine_and_signal_threads ->
            Gtid_state::update_commit_group

          Therefore, it is necessary to call update_on_commit only in case we
          are not using slave-preserve-commit-order here.
        */
        gtid_state->update_on_commit(thd);
      }
    } else {
      /*
        If statement is supposed to be written to binlog, we write it
        to the binary log.  Inserting into table and releasing
        ownership will be done in the binlog commit handler.
      */

      /*
        thd->cache_mngr may be uninitialized if the first transaction
        executed by the client is empty.
      */
      if (thd->binlog_setup_trx_data()) return 1;
      binlog_cache_data *cache_data = &thd_get_cache_mngr(thd)->trx_cache;

      // Generate BEGIN event
      Query_log_event qinfo(thd, STRING_WITH_LEN("BEGIN"), true, false, true, 0,
                            true);
      DBUG_ASSERT(!qinfo.is_using_immediate_logging());

      /*
        Write BEGIN event and then commit (which will generate commit
        event and Gtid_log_event)
      */
      DBUG_PRINT("debug", ("Writing to trx_cache"));
      if (cache_data->write_event(thd, &qinfo) ||
          mysql_bin_log.commit(thd, true))
        return 1;
    }
  } else if (thd->owned_gtid.sidno == THD::OWNED_SIDNO_ANONYMOUS ||
             /*
               A transaction with an empty owned gtid should call
               end_gtid_violating_transaction(...) to clear the
               flag thd->has_gtid_consistency_violatoin in case
               it is set. It missed the clear in ordered_commit,
               because its binlog transaction cache is empty.
             */
             thd->has_gtid_consistency_violation)

  {
    gtid_state->update_on_commit(thd);
  } else if (thd->variables.gtid_next.type == ASSIGNED_GTID &&
             thd->owned_gtid_is_empty()) {
    DBUG_ASSERT(thd->has_gtid_consistency_violation == false);
    gtid_state->update_on_commit(thd);
  }

  return 0;
}

bool MYSQL_BIN_LOG::reencrypt_logs() {
  DBUG_TRACE;

  if (!is_open()) return false;

  std::string error_message;
  /* Gather the set of files to be accessed. */
  list<string> filename_list;
  LOG_INFO linfo;
  int error = 0;
  list<string>::reverse_iterator rit;

  /* Read binary/relay log file names from index file. */
  mysql_mutex_lock(&LOCK_index);
  for (error = find_log_pos(&linfo, nullptr, false); !error;
       error = find_next_log(&linfo, false)) {
    filename_list.push_back(string(linfo.log_file_name));
  }
  mysql_mutex_unlock(&LOCK_index);
  if (error != LOG_INFO_EOF ||
      DBUG_EVALUATE_IF("fail_to_open_index_file", true, false)) {
    error_message.assign("I/O error reading index file '");
    error_message.append(index_file_name);
    error_message.append("'");
    goto err;
  }

  rit = filename_list.rbegin();
  /* Skip the last binary/relay log. */
  if (rit != filename_list.rend()) rit++;
  /* Iterate backwards through binary/relay logs. */
  while (rit != filename_list.rend()) {
    const char *filename = rit->c_str();
    DBUG_EXECUTE_IF("purge_logs_during_reencryption", {
      purge_logs(filename, true, true /*need_lock_index=true*/,
                 true /*need_update_threads=true*/, nullptr, false);
    });
    MUTEX_LOCK(lock, &LOCK_index);
    std::unique_ptr<Binlog_ofile> ofile(
        Binlog_ofile::open_existing(key_file_binlog, filename, MYF(MY_WME)));

    if (ofile == nullptr ||
        DBUG_EVALUATE_IF("fail_to_open_log_file", true, false) ||
        DBUG_EVALUATE_IF("fail_to_read_index_file", true, false)) {
      /* If we can not open the log file, check if it exists in index file. */
      error = find_log_pos(&linfo, filename, false);
      DBUG_EXECUTE_IF("fail_to_read_index_file", error = LOG_INFO_IO;);
      if (error == LOG_INFO_EOF) {
        /* If it does not exist in index file, re-encryption has finished. */
        if (current_thd->is_error()) current_thd->clear_error();
        break;
      } else if (error == 0) {
        /* If it exists in index file, failed to open the log file. */
        error_message.assign("Failed to open log file '");
        error_message.append(filename);
        error_message.append("'");
        goto err;
      } else if (error == LOG_INFO_IO) {
        /* Failed to read index file. */
        error_message.assign("I/O error reading index file '");
        error_message.append(index_file_name);
        error_message.append("'");
        goto err;
      }
    }

    if (ofile->is_encrypted()) {
      std::unique_ptr<Truncatable_ostream> pipeline_head =
          ofile->get_pipeline_head();
      std::unique_ptr<Binlog_encryption_ostream> binlog_encryption_ostream(
          down_cast<Binlog_encryption_ostream *>(pipeline_head.release()));

      auto ret_value = binlog_encryption_ostream->reencrypt();
      if (ret_value.first) {
        error_message.assign("Failed to re-encrypt log file '");
        error_message.append(filename);
        error_message.append("': ");
        error_message.append(ret_value.second.c_str());
        goto err;
      }
    }

    rit++;
  }

  filename_list.clear();

  return false;

err:
  if (current_thd->is_error()) current_thd->clear_error();
  my_error(ER_BINLOG_MASTER_KEY_ROTATION_FAIL_TO_REENCRYPT_LOG, MYF(0),
           error_message.c_str());
  filename_list.clear();

  return true;
}

bool binlog_cache_data::compress(THD *thd) {
  DBUG_TRACE;
  auto error{false};
  auto ctype{binary_log::transaction::compression::type::NONE};
  auto uncompressed_size{m_cache.length()};
  auto size{uncompressed_size};
  auto &cctx{thd->rpl_thd_ctx.transaction_compression_ctx()};
  binary_log::transaction::compression::Compressor *compressor{nullptr};

  // no compression enabled (ctype == NONE at this point)
  if (thd->variables.binlog_trx_compression == false) goto end;

  // do not compress if there are incident events
  DBUG_EXECUTE_IF("binlog_compression_inject_incident", set_incident(););
  if (has_incident()) goto end;

  // do not compress if there are non-transactional changes
  if (thd->get_transaction()->has_modified_non_trans_table(
          Transaction_ctx::STMT) ||
      thd->get_transaction()->has_modified_non_trans_table(
          Transaction_ctx::SESSION))
    goto end;

  // do not compress if has SBR
  if (may_have_sbr_stmts()) goto end;

  // Unable to get a reference to a compressor, fallback to
  // non compressed
  if ((compressor = cctx.get_compressor(thd)) == nullptr) goto end;

  // compression is enabled and all pre-conditions checked.
  // now compress
  else {
    std::size_t old_capacity{0};
    unsigned char *buffer{nullptr};
    unsigned char *old_buffer{nullptr};
    Transaction_payload_log_event tple{thd};
    Compressed_ostream stream;
    PSI_stage_info old_stage;

    // set the thread stage to compressing transaction
    thd->enter_stage(&stage_binlog_transaction_compress, &old_stage, __func__,
                     __FILE__, __LINE__);
    // do we have enough compression buffer ? If not swap with a larger one
    std::tie(buffer, std::ignore, old_capacity) = compressor->get_buffer();
    if (old_capacity < size) {
      old_buffer = buffer;
      auto new_buffer = (unsigned char *)malloc(size);
      if (new_buffer)
        compressor->set_buffer(new_buffer, size);
      else {
        /* purecov: begin inspected */
        // OOM
        error = true;
        goto compression_end;
        /* purecov: end */
      }
    }

    ctype = compressor->compression_type_code();

    compressor->open();

    // inject the compressor in the output stream
    stream.set_compressor(compressor);

    // FIXME: innefficient, we should not copy caches around
    //        This should be fixed when we revamp the capture
    //        cache handling (and make this more geared towards
    //        possible enhancements, such as streaming the changes)
    //        Also, if the cache actually spills to disk, this may
    //        the impact may be amplified, since reiniting the
    //        causes a flush to disk
    if ((error = m_cache.copy_to(&stream))) goto compression_end;

    compressor->close();

    if ((error = m_cache.truncate(0))) goto compression_end;
    // Since we deleted all events from the cache, we also need to
    // reset event_counter.
    event_counter = 0;

    // fill in the new transport event
    std::tie(buffer, size, std::ignore) = compressor->get_buffer();
    tple.set_payload((const char *)buffer);
    tple.set_payload_size(size);
    tple.set_compression_type(ctype);
    tple.set_uncompressed_size(uncompressed_size);

    // write back the new cache contents
    error = write_event(thd, &tple);

  compression_end:
    // revert back to the default buffer, so that we don't overuse memory
    if (old_buffer) {
      std::tie(buffer, std::ignore, std::ignore) = compressor->get_buffer();
      compressor->set_buffer(old_buffer, old_capacity);
      free(buffer);
    }

    // revert the stage if needed
    if (old_stage.m_key != 0) THD_STAGE_INFO(thd, old_stage);
  }

end:
  if (!error) {
    set_compression_type(ctype);
    set_compressed_size(m_cache.length());
    set_decompressed_size(uncompressed_size);
  }
  return error;
}

/**
  This function finalizes the cache preparing for commit or rollback.

  The function just writes all the necessary events to the cache but
  does not flush the data to the binary log file. That is the role of
  the binlog_cache_data::flush function.

  @see binlog_cache_data::flush

  @param thd                The thread whose transaction should be flushed
  @param end_event          The end event either commit/rollback

  @return
    nonzero if an error pops up when flushing the cache.
*/
int binlog_cache_data::finalize(THD *thd, Log_event *end_event) {
  DBUG_TRACE;
  if (!is_binlog_empty()) {
    DBUG_ASSERT(!flags.finalized);
    if (int error = flush_pending_event(thd)) return error;
    if (int error = write_event(thd, end_event)) return error;
    if (int error = this->compress(thd)) return error;
    DBUG_PRINT("debug", ("flags.finalized: %s", YESNO(flags.finalized)));
    flags.finalized = true;
  }
  return 0;
}

/**
   The method writes XA END query to XA-prepared transaction's cache
   and calls the "basic" finalize().

   @return error code, 0 success
*/

int binlog_cache_data::finalize(THD *thd, Log_event *end_event, XID_STATE *xs) {
  int error = 0;
  char buf[XID::ser_buf_size];
  char query[sizeof("XA END") + 1 + sizeof(buf)];
  int qlen = sprintf(query, "XA END %s", xs->get_xid()->serialize(buf));
  Query_log_event qev(thd, query, qlen, true, false, true, 0);

  if ((error = write_event(thd, &qev))) return error;

  return finalize(thd, end_event);
}

void MYSQL_BIN_LOG::handle_write_error(THD *thd) {
  DBUG_ENTER("MYSQL_BIN_LOG::handle_write_error(THD *)");

  report_binlog_write_error();

  thd->commit_error = THD::CE_FLUSH_ERROR;

  DBUG_VOID_RETURN;
}

bool MYSQL_BIN_LOG::post_write(THD *thd, binlog_cache_data *cache_data,
                               int error) {
  DBUG_ENTER("MYSQL_BIN_LOG::post_write(THD *, binlog_cache_data *, int)");

  mysql_mutex_assert_owner(&LOCK_log);
  DBUG_ASSERT(is_open());

  if (unlikely(!is_open())) DBUG_RETURN(false);

  Binlog_cache_storage *cache = cache_data->get_cache();
  if (cache->length() == 0) DBUG_RETURN(false);

  if (write_error || error) {
    handle_write_error(thd);
    DBUG_RETURN(true);
  }

  binlog_bytes_written += cache->length();
  update_thd_next_event_pos(thd);

  DBUG_RETURN(false);
}

/**
  Flush caches to the binary log.

  If the cache is finalized, the cache will be flushed to the binary
  log file. If the cache is not finalized, nothing will be done.

  If flushing fails for any reason, an error will be reported and the
  cache will be reset. Flushing can fail in two circumstances:

  - It was not possible to write the cache to the file. In this case,
    it does not make sense to keep the cache.

  - The cache was successfully written to disk but post-flush actions
    (such as binary log rotation) failed. In this case, the cache is
    already written to disk and there is no reason to keep it.

  @see binlog_cache_data::finalize
 */
int binlog_cache_data::flush(THD *thd, my_off_t *bytes_written,
                             bool *wrote_xid) {
  /*
    Doing a commit or a rollback including non-transactional tables,
    i.e., ending a transaction where we might write the transaction
    cache to the binary log.

    We can always end the statement when ending a transaction since
    transactions are not allowed inside stored functions. If they
    were, we would have to ensure that we're not ending a statement
    inside a stored function.
  */
  DBUG_TRACE;
  DBUG_PRINT("debug", ("flags.finalized: %s", YESNO(flags.finalized)));
  int error = 0;
  if (flags.finalized) {
    my_off_t bytes_in_cache = m_cache.length();
    Transaction_ctx *trn_ctx = thd->get_transaction();

    DBUG_PRINT("debug", ("bytes_in_cache: %llu", bytes_in_cache));

    trn_ctx->sequence_number = mysql_bin_log.m_dependency_tracker.step();

    /*
      In case of two caches the transaction is split into two groups.
      The 2nd group is considered to be a successor of the 1st rather
      than to have a common commit parent with it.
      Notice that due to a simple method of detection that the current is
      the 2nd cache being flushed, the very first few transactions may be logged
      sequentially (a next one is tagged as if a preceding one is its
      commit parent).
    */
    if (trn_ctx->last_committed == SEQ_UNINIT)
      trn_ctx->last_committed = trn_ctx->sequence_number - 1;

    /*
      The GTID is written prior to flushing the statement cache, if
      the transaction has written to the statement cache; and prior to
      flushing the transaction cache if the transaction has written to
      the transaction cache.  If GTIDs are enabled, then transactional
      and non-transactional updates cannot be mixed, so at most one of
      the caches can be non-empty, so just one GTID will be
      generated. If GTIDs are disabled, then no GTID is generated at
      all; if both the transactional cache and the statement cache are
      non-empty then we get two Anonymous_gtid_log_events, which is
      correct.
    */
    Binlog_event_writer writer(mysql_bin_log.get_binlog_file());

    /* The GTID ownership process might set the commit_error */
    error = (thd->commit_error == THD::CE_FLUSH_ERROR);

    DBUG_EXECUTE_IF("simulate_binlog_flush_error", {
      if (rand() % 3 == 0) {
        thd->commit_error = THD::CE_FLUSH_ERROR;
      }
    };);

    DBUG_EXECUTE_IF("fault_injection_reinit_io_cache_while_flushing_to_file",
                    { DBUG_SET("+d,fault_injection_reinit_io_cache"); });

    if (!error)
      if ((error = mysql_bin_log.write_transaction(thd, this, &writer)))
        thd->commit_error = THD::CE_FLUSH_ERROR;

    DBUG_EXECUTE_IF("fault_injection_reinit_io_cache_while_flushing_to_file",
                    { DBUG_SET("-d,fault_injection_reinit_io_cache"); });

    if (flags.with_xid && error == 0) *wrote_xid = true;

    /*
      Reset have to be after the if above, since it clears the
      with_xid flag
    */
    reset();
    if (bytes_written) *bytes_written = bytes_in_cache;
  }
  DBUG_ASSERT(!flags.finalized);
  return error;
}

/**
  This function truncates the transactional cache upon committing or rolling
  back either a transaction or a statement.

  @param thd        The thread whose transaction should be flushed
  @param all        @c true means truncate the transaction, otherwise the
                    statement must be truncated.

  @return
    nonzero if an error pops up when truncating the transactional cache.
*/
int binlog_trx_cache_data::truncate(THD *thd, bool all) {
  DBUG_TRACE;
  int error = 0;

  DBUG_PRINT("info",
             ("thd->options={ %s %s}, transaction: %s",
              FLAGSTR(thd->variables.option_bits, OPTION_NOT_AUTOCOMMIT),
              FLAGSTR(thd->variables.option_bits, OPTION_BEGIN),
              all ? "all" : "stmt"));

  remove_pending_event();

  /*
    If rolling back an entire transaction or a single statement not
    inside a transaction, we reset the transaction cache.
    Even though formally the atomic DDL statement may not end multi-statement
    transaction the cache needs full resetting as there must
    be no other data in it but belonging to the DDL.
  */
  if (ending_trans(thd, all)) {
    if (has_incident()) {
      const char *err_msg =
          "Error happend while resetting the transaction "
          "cache for a rolled back transaction or a single "
          "statement not inside a transaction.";
      error = mysql_bin_log.write_incident(thd, true /*need_lock_log=true*/,
                                           err_msg);
    }
    reset();
  }
  /*
    If rolling back a statement in a transaction, we truncate the
    transaction cache to remove the statement.
  */
  else if (get_prev_position() != MY_OFF_T_UNDEF)
    restore_prev_position();

  thd->clear_binlog_table_maps();

  return error;
}

inline enum xa_option_words get_xa_opt(THD *thd) {
  enum xa_option_words xa_opt = XA_NONE;
  switch (thd->lex->sql_command) {
    case SQLCOM_XA_COMMIT:
      xa_opt =
          static_cast<Sql_cmd_xa_commit *>(thd->lex->m_sql_cmd)->get_xa_opt();
      break;
    default:
      break;
  }

  return xa_opt;
}

/**
   Predicate function yields true when XA transaction is
   being logged having a proper state ready for prepare or
   commit in one phase.

   @param thd    THD pointer of running transaction
   @return true  When the being prepared transaction should be binlogged,
           false otherwise.
*/

inline bool is_loggable_xa_prepare(THD *thd) {
  /*
    simulate_commit_failure is doing a trick with XID_STATE while
    the ongoing transaction is not XA, and therefore to be errored out,
    asserted below. In that case because of the
    latter fact the function returns @c false.
  */
  DBUG_EXECUTE_IF("simulate_commit_failure", {
    XID_STATE *xs = thd->get_transaction()->xid_state();
    DBUG_ASSERT((thd->is_error() && xs->get_state() == XID_STATE::XA_IDLE) ||
                xs->get_state() == XID_STATE::XA_NOTR);
  });

  return DBUG_EVALUATE_IF(
      "simulate_commit_failure", false,
      thd->get_transaction()->xid_state()->has_state(XID_STATE::XA_IDLE));
}

static int binlog_prepare(handlerton *, THD *thd, bool all) {
  DBUG_TRACE;
  if (!all) {
    thd->get_transaction()->store_commit_parent(
        mysql_bin_log.m_dependency_tracker.get_max_committed_timestamp());
  }

  return all && is_loggable_xa_prepare(thd) ? mysql_bin_log.commit(thd, true)
                                            : 0;
}

/**
   Logging XA commit/rollback of a prepared transaction.

   The function is called at XA-commit or XA-rollback logging via
   two paths: the recovered-or-slave-applier or immediately through
   the  XA-prepared transaction connection itself.
   It fills in appropiate event in the statement cache whenever
   xid state is marked with is_binlogged() flag that indicates
   the prepared part of the transaction must've been logged.

   About early returns from the function.
   In the recovered-or-slave-applier case the function may be called
   for the 2nd time, which has_logged_xid monitors.
   ONE_PHASE option to XA-COMMIT is handled to skip
   writing XA-commit event now.
   And the final early return check is for the read-only XA that is
   not to be logged.

   @param thd          THD handle
   @param xid          a pointer to XID object that is serialized
   @param commit       when @c true XA-COMMIT is to be logged,
                       and @c false when it's XA-ROLLBACK.
   @return error code, 0 success
*/

inline int do_binlog_xa_commit_rollback(THD *thd, XID *xid, bool commit) {
  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_XA_COMMIT ||
              thd->lex->sql_command == SQLCOM_XA_ROLLBACK);

  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);

  if (cache_mngr != nullptr && cache_mngr->has_logged_xid) return 0;

  if (get_xa_opt(thd) == XA_ONE_PHASE) return 0;
  if (!xid_state->is_binlogged())
    return 0;  // nothing was really logged at prepare
  if (thd->is_error() && DBUG_EVALUATE_IF("simulate_xa_rm_error", 0, 1))
    return 0;  // don't binlog if there are some errors.

  DBUG_ASSERT(!xid->is_null() ||
              !(thd->variables.option_bits & OPTION_BIN_LOG));

  char buf[XID::ser_buf_size];
  char query[(sizeof("XA ROLLBACK")) + 1 + sizeof(buf)];
  int qlen = sprintf(query, "XA %s %s", commit ? "COMMIT" : "ROLLBACK",
                     xid->serialize(buf));
  Query_log_event qinfo(thd, query, qlen, false, true, true, 0, false);
  return mysql_bin_log.write_event(&qinfo);
}

/**
   Logging XA commit/rollback of a prepared transaction in the case
   it was disconnected and resumed (recovered), or executed by a slave applier.

   @param thd         THD handle
   @param xid         a pointer to XID object
   @param commit      when @c true XA-COMMIT is logged, otherwise XA-ROLLBACK

   @return error code, 0 success
*/

inline xa_status_code binlog_xa_commit_or_rollback(THD *thd, XID *xid,
                                                   bool commit) {
  int error = 0;

#ifndef DBUG_OFF
  {
    binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
    DBUG_ASSERT(!cache_mngr || !cache_mngr->has_logged_xid);
  }
#endif
  if (!(error = do_binlog_xa_commit_rollback(thd, xid, commit))) {
    /*
      Error can't be propagated naturally via result.
      A grand-caller has to access to it through thd's da.
      todo:
      Bug #20488921 ERROR PROPAGATION DOES FULLY WORK IN XA
      stands in the way of implementing a failure simulation
      for XA PREPARE/COMMIT/ROLLBACK.
    */
    binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);

    if (cache_mngr) cache_mngr->has_logged_xid = true;
    if (commit)
      error = mysql_bin_log.commit(thd, true);
    else
      error = mysql_bin_log.rollback(thd, true);
    if (cache_mngr) cache_mngr->has_logged_xid = false;
  }

  return error == TC_LOG::RESULT_SUCCESS ? XA_OK : XAER_RMERR;
}

static xa_status_code binlog_xa_commit(handlerton *, XID *xid) {
  return binlog_xa_commit_or_rollback(current_thd, xid, true);
}

static xa_status_code binlog_xa_rollback(handlerton *, XID *xid) {
  return binlog_xa_commit_or_rollback(current_thd, xid, false);
}

/**
  When a fatal error occurs due to which binary logging becomes impossible and
  the user specified binlog_error_action= ABORT_SERVER the following function is
  invoked. This function pushes the appropriate error message to client and logs
  the same to server error log and then aborts the server.

  @param err_string          Error string which specifies the exact error
                             message from the caller.

  @retval
    none
*/
static void exec_binlog_error_action_abort(const char *err_string) {
  THD *thd = current_thd;
  /*
    When the code enters here it means that there was an error at higher layer
    and my_error function could have been invoked to let the client know what
    went wrong during the execution.

    But these errors will not let the client know that the server is going to
    abort. Even if we add an additional my_error function call at this point
    client will be able to see only the first error message that was set
    during the very first invocation of my_error function call.

    The advantage of having multiple my_error function calls are visible when
    the server is up and running and user issues SHOW WARNINGS or SHOW ERROR
    calls. In this special scenario server will be immediately aborted and
    user will not be able execute the above SHOW commands.

    Hence we clear the previous errors and push one critical error message to
    clients.
   */
  if (thd) {
    if (thd->is_error()) thd->clear_error();
    /*
      Send error to both client and to the server error log.
    */
    my_error(ER_BINLOG_LOGGING_IMPOSSIBLE, MYF(ME_FATALERROR), err_string);
  }

  LogErr(ERROR_LEVEL, ER_BINLOG_LOGGING_NOT_POSSIBLE, err_string);
  flush_error_log_messages();

  if (thd) thd->send_statement_status();
  abort();
}

/**
  This function is called once after each statement.

  @todo This function is currently not used any more and will
  eventually be eliminated. The real commit job is done in the
  MYSQL_BIN_LOG::commit function.

  @see MYSQL_BIN_LOG::commit

  @see handlerton::commit
*/
static int binlog_commit(handlerton *, THD *, bool) {
  DBUG_TRACE;
  /*
    Nothing to do (any more) on commit.
   */
  return 0;
}

/**
  This function is called when a transaction or a statement is rolled back.

  @internal It is necessary to execute a rollback here if the
  transaction was rolled back because of executing a ROLLBACK TO
  SAVEPOINT command, but it is not used for normal rollback since
  MYSQL_BIN_LOG::rollback is called in that case.

  @todo Refactor code to introduce a <code>MYSQL_BIN_LOG::rollback(THD
  *thd, SAVEPOINT *sv)</code> function in @c TC_LOG and have that
  function execute the necessary work to rollback to a savepoint.

  @param thd   The client thread that executes the transaction.
  @param all   This is @c true if this is a real transaction rollback, and
               @false otherwise.

  @see handlerton::rollback
*/
static int binlog_rollback(handlerton *, THD *thd, bool all) {
  DBUG_TRACE;
  int error = 0;
  if (thd->lex->sql_command == SQLCOM_ROLLBACK_TO_SAVEPOINT)
    error = mysql_bin_log.rollback(thd, all);
  return error;
}

uint64_t HybridLogicalClock::get_next() {
  uint64_t current_hlc, next_hlc = 0;
  bool done = false;

  while (!done) {
    // Get the current wall clock in nanosecond precision
    uint64_t current_wall_clock =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();

    // get the 'current' internal HLC
    current_hlc = current_.load();

    // Next HLC timestamp is max of current-hlc and current-wall-clock
    next_hlc = std::max(current_hlc + 1, current_wall_clock);

    // Conditionally update the internal hlc
    done = current_.compare_exchange_strong(current_hlc, next_hlc);
  }

  return next_hlc;
}

uint64_t HybridLogicalClock::get_current() { return current_.load(); }

uint64_t HybridLogicalClock::update(uint64_t minimum_hlc) {
  uint64_t current_hlc, new_min_hlc = 0;
  bool done = false;

  while (!done) {
    // get the 'current' internal HLC
    current_hlc = current_.load();

    // Next HLC timestamp is max of current-hlc, minimum-hlc
    new_min_hlc = std::max(minimum_hlc, current_hlc);
    // only used for MTR to reset HLC
    DBUG_EXECUTE_IF("reset_hlc_for_tests", { new_min_hlc = minimum_hlc; });

    // Conditionally update the internal hlc
    done = current_.compare_exchange_strong(current_hlc, new_min_hlc);
  }

  return new_min_hlc;
}

void HybridLogicalClock::update_database_hlc(
    const database_container &databases, uint64_t applied_hlc) {
  std::vector<std::shared_ptr<DatabaseEntry>> entries;
  {
    std::unique_lock<std::mutex> lock(database_map_lock_);
    for (const auto &database : databases) {
      auto entry = getEntry(database);
      DBUG_ASSERT(entry);
      entries.push_back(std::move(entry));
    }
  }

  for (const auto &entry : entries) {
    entry->update_hlc(applied_hlc);
  }
}

database_hlc_container HybridLogicalClock::get_database_hlc() const {
  database_hlc_container container;
  std::unique_lock<std::mutex> lock(database_map_lock_);
  for (const auto &record : database_map_) {
    container.emplace(record.first,
                      std::make_pair(record.second->max_applied_hlc(),
                                     record.second->num_out_of_order_hlc()));
  }
  return container;
}

uint64_t HybridLogicalClock::get_selected_database_hlc(
    const std::string &database) {
  std::unique_lock<std::mutex> lock(database_map_lock_);

  const auto it = database_map_.find(database);
  return it != database_map_.end() ? it->second->max_applied_hlc() : 0;
}

void HybridLogicalClock::clear_database_hlc() {
  std::unique_lock<std::mutex> lock(database_map_lock_);
  database_map_.clear();
}

bool HybridLogicalClock::wait_for_hlc_applied(THD *thd) {
  if (!(thd->variables.enable_block_stale_hlc_read && thd->db().str &&
        !thd->slave_thread)) {
    return false;
  }

  const char *hlc_ts_str = nullptr;
  const char *hlc_wait_timeout_str = nullptr;
  for (const auto &p : thd->query_attrs_list) {
    if (p.first == hlc_ts_lower_bound) {
      hlc_ts_str = p.second.c_str();
    } else if (p.first == hlc_wait_timeout_ms) {
      hlc_wait_timeout_str = p.second.c_str();
    }
    // Bailout once both strings are found
    if (hlc_wait_timeout_str && hlc_ts_str) {
      break;
    }
  }

  // No lower bound HLC ts specified, skip early
  if (!hlc_ts_str) {
    return false;
  }

  // Behavior of this feature on reads inside of a transaction is complex
  // and not supported at this point in time.
  if (thd->in_active_multi_stmt_transaction()) {
    my_error(ER_HLC_READ_BOUND_IN_TRANSACTION, MYF(0));
    return true;
  }

  // The implementation makes the assumption that
  // allow_noncurrent_db_rw = OFF and only reads to the current database
  // are allowed
  if (thd->variables.allow_noncurrent_db_rw != 3 /* OFF */) {
    my_error(ER_INVALID_NONCURRENT_DB_RW_FOR_HLC_READ_BOUND, MYF(0),
             thd->variables.allow_noncurrent_db_rw);
    return true;
  }

  char *endptr = nullptr;
  uint64_t requested_hlc = strtoull(hlc_ts_str, &endptr, 10);
  if (!endptr || *endptr != '\0' ||
      !HybridLogicalClock::is_valid_hlc(requested_hlc)) {
    my_error(ER_INVALID_HLC, MYF(0), hlc_ts_str);
    return true;
  }

  endptr = nullptr;
  uint64_t timeout_ms = wait_for_hlc_timeout_ms;
  if (hlc_wait_timeout_str) {
    timeout_ms = strtoull(hlc_wait_timeout_str, &endptr, 10);
    if (!endptr || *endptr != '\0') {
      my_error(ER_INVALID_HLC_WAIT_TIMEOUT, MYF(0), hlc_wait_timeout_str);
      return true;
    }
  }

  const auto &db_lex_str = thd->db();
  std::string db(db_lex_str.str, db_lex_str.length);

  uint64_t applied_hlc = mysql_bin_log.get_selected_database_hlc(db);
  if (requested_hlc > applied_hlc &&
      (timeout_ms == 0 || !wait_for_hlc_timeout_ms)) {
    my_error(ER_STALE_HLC_READ, MYF(0), requested_hlc, applied_hlc, db.c_str());
    return true;
  }

  // Return early if the waiting feature isn't enabled
  if (!wait_for_hlc_timeout_ms) return false;

  std::shared_ptr<DatabaseEntry> entry = nullptr;
  {
    std::unique_lock<std::mutex> lock(database_map_lock_);
    entry = getEntry(db);
  }

  return entry->wait_for_hlc(thd, requested_hlc, timeout_ms);
}

bool HybridLogicalClock::check_hlc_bound(THD *thd) {
  if (!thd->variables.enable_hlc_bound || thd->slave_thread) {
    return false;
  }

  const char *hlc_lower_bound_ts_str = nullptr;
  const char *hlc_upper_bound_ts_str = nullptr;
  for (const auto &p : thd->query_attrs_list) {
    if (p.first == hlc_ts_lower_bound) {
      hlc_lower_bound_ts_str = p.second.c_str();
    } else if (p.first == hlc_ts_upper_bound) {
      hlc_upper_bound_ts_str = p.second.c_str();
    }
    if (hlc_lower_bound_ts_str && hlc_upper_bound_ts_str) {
      break;
    }
  }

  if (hlc_lower_bound_ts_str) {
    char *endptr = nullptr;
    uint64_t requested_hlc = strtoull(hlc_lower_bound_ts_str, &endptr, 10);
    if (!endptr || *endptr != '\0' ||
        !HybridLogicalClock::is_valid_hlc(requested_hlc)) {
      my_error(ER_INVALID_HLC, MYF(0), hlc_lower_bound_ts_str);
      return true;
    }

    // There is a limit on how far we can jump HLC value. In case if the
    // requested HLC value is too far ahead of the current wallclock time
    // we fail the request with ER_HLC_ABOVE_MAX_DRIFT error.

    // Get current wall clock
    uint64_t cur_wall_clock_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();

    // Updating the HLC cannot be allowed if HLC drifts forward by more than
    // 'maximum_hlc_drift_ns' as compared to wall clock
    if (requested_hlc > cur_wall_clock_ns &&
        (requested_hlc - cur_wall_clock_ns) > maximum_hlc_drift_ns) {
      my_error(ER_HLC_ABOVE_MAX_DRIFT, MYF(0), hlc_lower_bound_ts_str);
      return true;
    }
    update(requested_hlc);
  }

  if (hlc_upper_bound_ts_str) {
    char *endptr = nullptr;
    uint64_t requested_hlc = strtoull(hlc_upper_bound_ts_str, &endptr, 10);
    if (!endptr || *endptr != '\0' ||
        !HybridLogicalClock::is_valid_hlc(requested_hlc)) {
      my_error(ER_INVALID_HLC, MYF(0), hlc_upper_bound_ts_str);
      return true;
    }

    uint64_t current_hlc = get_current();
    if (requested_hlc <= current_hlc + hlc_upper_bound_delta) {
      my_error(ER_HLC_STALE_UPPER_BOUND, MYF(0), current_hlc);
      return true;
    }
  }

  return false;
}

void HybridLogicalClock::DatabaseEntry::update_hlc(uint64_t applied_hlc) {
  // CAS loop to update max_applied_hlc if less than the new applied HLC
  uint64_t original = max_applied_hlc_;
  // Track num of out of order hlc value
  if (original > applied_hlc) {
    num_out_of_order_hlc_++;
  }
  while (original < applied_hlc &&
         !max_applied_hlc_.compare_exchange_strong(original, applied_hlc)) {
  }

  // Signal the list of waiters with requested HLCs close to the current applied
  // value
  if (wait_for_hlc_timeout_ms) {
    mysql_cond_broadcast(&cond_);
  }
}

/**
 * Returns true for cases where the wait failed for any reason
 */
bool HybridLogicalClock::DatabaseEntry::wait_for_hlc(THD *thd,
                                                     uint64_t requested_hlc,
                                                     uint64_t timeout_ms) {
  auto start_time = std::chrono::steady_clock::now();
  while (max_applied_hlc_ < requested_hlc) {
    // HLC is nano-second granularity, scale down to millis
    auto delta_ms = (requested_hlc - max_applied_hlc_) / 1000000ULL;

    int64_t remaining_timeout_ms = 0;
    bool sleeping = true;

    uint64_t total_duration_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time)
            .count();

    // Look at the delta between the current applied HLC and the requested
    // HLC. If there is a large gap, go ahead and block (sleep) on the local
    // mutex/condvar combination to stall until the database has almost caught
    // up. Once the database is almost caught up, then go ahead and block on
    // the wait queue that is released on binlog application.
    //
    // We use the delta in requested_hlc - applied_hlc as an approximation
    // for how long to wait. Seconds behind master is difficult because its
    // unclear in the sequence of lagged binlog records the unapplied HLC exists
    // and thus how long to wait
    if (delta_ms > wait_for_hlc_sleep_threshold_ms) {
      // For large deltas, go ahead and send the thread to sleep for a bit
      // Use std::min() for pathological cases where there is a low write rate
      // and thus a large gap in HLC values
      remaining_timeout_ms =
          std::min(delta_ms * wait_for_hlc_sleep_scaling_factor, 100.0);
      sleeping = true;
    } else {
      // Once the HLC is close to being applied, block on the list of waiters
      // to be released when new binlog events are applied to the database
      remaining_timeout_ms = timeout_ms - total_duration_ms;
      sleeping = false;
    }

    if (remaining_timeout_ms <= 0 || total_duration_ms > timeout_ms) {
      my_error(ER_HLC_WAIT_TIMEDOUT, MYF(0), requested_hlc);
      return true;
    }

    if (sleeping) {
      const char *save_proc_info =
          thd_proc_info(thd, "Waiting for database applied HLC");
      std::this_thread::sleep_for(
          std::chrono::milliseconds{remaining_timeout_ms});
      thd_proc_info(thd, save_proc_info);
    } else {
      struct timespec timeout;
      set_timespec_nsec(&timeout, remaining_timeout_ms * 1000000ULL);

      mysql_mutex_lock(&mutex_);
      thd->ENTER_COND(&cond_, &mutex_, &stage_waiting_for_hlc, NULL);
      thd_wait_begin(thd, THD_WAIT_FOR_HLC);

      int error = mysql_cond_timedwait(&cond_, &mutex_, &timeout);

      mysql_mutex_unlock(&mutex_);
      thd->EXIT_COND(NULL);
      thd_wait_end(thd);

      // When intentionally sleeping to stall, we expect a timeout
      if (is_timeout(error)) {
        my_error(ER_HLC_WAIT_TIMEDOUT, MYF(0), requested_hlc);
        return true;
      }
    }

    if (thd_killed(thd)) {
      my_error(ER_QUERY_INTERRUPTED, MYF(0));
      return true;
    }
  }

  // Return the total wait time back to the client (for instrumentation
  // purposes)
  auto tracker = thd->session_tracker.get_tracker(SESSION_RESP_ATTR_TRACKER);
  if (thd->variables.response_attrs_contain_hlc && tracker->is_enabled()) {
    LEX_CSTRING key = {STRING_WITH_LEN("hlc_wait_duration_ms")};
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time);
    std::string value_str = std::to_string(time.count());
    const LEX_CSTRING value = {value_str.c_str(), value_str.length()};
    tracker->mark_as_changed(thd, &key, &value);
  }

  return false;
}

/**
  Write a rollback record of the transaction to the binary log.

  For binary log group commit, the rollback is separated into three
  parts:

  1. First part consists of filling the necessary caches and
     finalizing them (if they need to be finalized). After a cache is
     finalized, nothing can be added to the cache.

  2. Second part execute an ordered flush and commit. This will be
     done using the group commit functionality in @c ordered_commit.

     Since we roll back the transaction early, we call @c
     ordered_commit with the @c skip_commit flag set. The @c
     ha_commit_low call inside @c ordered_commit will then not be
     called.

  3. Third part checks any errors resulting from the flush and handles
     them appropriately.

  @see MYSQL_BIN_LOG::ordered_commit
  @see ha_commit_low
  @see ha_rollback_low

  @param thd Session to commit
  @param all This is @c true if this is a real transaction rollback, and
             @c false otherwise.

  @return Error code, or zero if there were no error.
 */

int MYSQL_BIN_LOG::rollback(THD *thd, bool all) {
  int error = 0;
  bool stuff_logged = false;
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
  bool is_empty = false;

  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("all: %s, cache_mngr: 0x%llx, thd->is_error: %s", YESNO(all),
              (ulonglong)cache_mngr, YESNO(thd->is_error())));
  /*
    Defer XA-transaction rollback until its XA-rollback event is recorded.
    When we are executing a ROLLBACK TO SAVEPOINT, we
    should only clear the caches since this function is called as part
    of the engine rollback.
    In other cases we roll back the transaction in the engines early
    since this will release locks and allow other transactions to
    start executing.
  */
  if (thd->lex->sql_command == SQLCOM_XA_ROLLBACK) {
    XID_STATE *xs = thd->get_transaction()->xid_state();

    DBUG_ASSERT(all || !xs->is_binlogged() ||
                (!xs->is_in_recovery() && thd->is_error()));
    /*
      Whenever cache_mngr is not initialized, the xa prepared
      transaction's binary logging status must not be set, unless the
      transaction is rolled back through an external connection which
      has binlogging switched off.
    */
    DBUG_ASSERT(cache_mngr || !xs->is_binlogged() ||
                !(is_open() && thd->variables.option_bits & OPTION_BIN_LOG));

    is_empty = !xs->is_binlogged();
    if ((error = do_binlog_xa_commit_rollback(thd, xs->get_xid(), false)))
      goto end;
    cache_mngr = thd_get_cache_mngr(thd);
  } else if (thd->lex->sql_command != SQLCOM_ROLLBACK_TO_SAVEPOINT)
    if ((error = ha_rollback_low(thd, all))) goto end;

  /*
    If there is no cache manager, or if there is nothing in the
    caches, there are no caches to roll back, so we're trivially done
    unless XA-ROLLBACK that yet to run rollback_low().
  */
  if (cache_mngr == nullptr || cache_mngr->is_binlog_empty()) {
    goto end;
  }

  DBUG_PRINT("debug", ("all.cannot_safely_rollback(): %s, trx_cache_empty: %s",
                       YESNO(thd->get_transaction()->cannot_safely_rollback(
                           Transaction_ctx::SESSION)),
                       YESNO(cache_mngr->trx_cache.is_binlog_empty())));
  DBUG_PRINT("debug",
             ("stmt.cannot_safely_rollback(): %s, stmt_cache_empty: %s",
              YESNO(thd->get_transaction()->cannot_safely_rollback(
                  Transaction_ctx::STMT)),
              YESNO(cache_mngr->stmt_cache.is_binlog_empty())));

  /*
    If an incident event is set we do not flush the content of the statement
    cache because it may be corrupted.
  */
  if (cache_mngr->stmt_cache.has_incident()) {
    const char *err_msg =
        "The content of the statement cache is corrupted "
        "while writing a rollback record of the transaction "
        "to the binary log.";
    error = write_incident(thd, true /*need_lock_log=true*/, err_msg);
    cache_mngr->stmt_cache.reset();
  } else if (!cache_mngr->stmt_cache.is_binlog_empty()) {
    if (thd->lex->sql_command == SQLCOM_CREATE_TABLE &&
        thd->lex->select_lex->item_list.elements && /* With select */
        !(thd->lex->create_info->options & HA_LEX_CREATE_TMP_TABLE) &&
        thd->is_current_stmt_binlog_format_row()) {
      /*
        In row based binlog format, we reset the binlog statement cache
        when rolling back a single statement 'CREATE...SELECT' transaction,
        since the 'CREATE TABLE' event was put in the binlog statement cache.
      */
      cache_mngr->stmt_cache.reset();
    } else {
      if ((error = cache_mngr->stmt_cache.finalize(thd))) goto end;
      stuff_logged = true;
    }
  }

  if (ending_trans(thd, all)) {
    if (trans_cannot_safely_rollback(thd)) {
      const char xa_rollback_str[] = "XA ROLLBACK";
      /*
        sizeof(xa_rollback_str) and XID::ser_buf_size both allocate `\0',
        so one of the two is used for necessary in the xa case `space' char
      */
      char query[sizeof(xa_rollback_str) + XID::ser_buf_size] = "ROLLBACK";
      XID_STATE *xs = thd->get_transaction()->xid_state();

      if (thd->lex->sql_command == SQLCOM_XA_ROLLBACK) {
        /* this block is relevant only for not prepared yet and "local" xa trx
         */
        DBUG_ASSERT(
            thd->get_transaction()->xid_state()->has_state(XID_STATE::XA_IDLE));
        DBUG_ASSERT(!cache_mngr->has_logged_xid);

        sprintf(query, "%s ", xa_rollback_str);
        xs->get_xid()->serialize(query + sizeof(xa_rollback_str));
      }
      /*
        If the transaction is being rolled back and contains changes that
        cannot be rolled back, the trx-cache's content is flushed.
      */
      Query_log_event end_evt(thd, query, strlen(query), true, false, true, 0,
                              true);
      error = thd->lex->sql_command != SQLCOM_XA_ROLLBACK
                  ? cache_mngr->trx_cache.finalize(thd, &end_evt)
                  : cache_mngr->trx_cache.finalize(thd, &end_evt, xs);
      stuff_logged = true;
    } else {
      /*
        If the transaction is being rolled back and its changes can be
        rolled back, the trx-cache's content is truncated.
      */
      error = cache_mngr->trx_cache.truncate(thd, all);

      DBUG_EXECUTE_IF("ensure_binlog_cache_is_reset", {
        /* Assert that binlog cache is reset at rollback time. */
        DBUG_ASSERT(binlog_cache_is_reset);
        binlog_cache_is_reset = false;
      };);
    }
  } else {
    /*
      If a statement is being rolled back, it is necessary to know
      exactly why a statement may not be safely rolled back as in
      some specific situations the trx-cache can be truncated.

      If a temporary table is created or dropped, the trx-cache is not
      truncated. Note that if the stmt-cache is used, there is nothing
      to truncate in the trx-cache.

      If a non-transactional table is updated and the binlog format is
      statement, the trx-cache is not truncated. The trx-cache is used
      when the direct option is off and a transactional table has been
      updated before the current statement in the context of the
      current transaction. Note that if the stmt-cache is used there is
      nothing to truncate in the trx-cache.

      If other binlog formats are used, updates to non-transactional
      tables are written to the stmt-cache and trx-cache can be safely
      truncated, if necessary.
    */
    if (thd->get_transaction()->has_dropped_temp_table(Transaction_ctx::STMT) ||
        thd->get_transaction()->has_created_temp_table(Transaction_ctx::STMT) ||
        (thd->get_transaction()->has_modified_non_trans_table(
             Transaction_ctx::STMT) &&
         thd->variables.binlog_format == BINLOG_FORMAT_STMT)) {
      /*
        If the statement is being rolled back and dropped or created a
        temporary table or modified a non-transactional table and the
        statement-based replication is in use, the statement's changes
        in the trx-cache are preserved.
      */
      cache_mngr->trx_cache.set_prev_position(MY_OFF_T_UNDEF);
    } else {
      /*
        Otherwise, the statement's changes in the trx-cache are
        truncated.
      */
      error = cache_mngr->trx_cache.truncate(thd, all);
    }
  }
  if (stuff_logged) {
    Transaction_ctx *trn_ctx = thd->get_transaction();
    trn_ctx->store_commit_parent(
        m_dependency_tracker.get_max_committed_timestamp());
  }

  DBUG_PRINT("debug", ("error: %d", error));
  if (error == 0 && stuff_logged) {
    if (RUN_HOOK(
            transaction, before_commit,
            (thd, all, thd_get_cache_mngr(thd)->get_trx_cache(),
             thd_get_cache_mngr(thd)->get_stmt_cache(),
             max<my_off_t>(max_binlog_cache_size, max_binlog_stmt_cache_size),
             false))) {
      // Reset the thread OK status before changing the outcome.
      if (thd->get_stmt_da()->is_ok())
        thd->get_stmt_da()->reset_diagnostics_area();
      my_error(ER_RUN_HOOK_ERROR, MYF(0), "before_commit");
      return RESULT_ABORTED;
    }

#ifndef DBUG_OFF
    /*
      XA rollback is always accepted.
    */
    if (thd->get_transaction()
            ->get_rpl_transaction_ctx()
            ->is_transaction_rollback())
      DBUG_ASSERT(0);
#endif

    error = ordered_commit(thd, all, /* skip_commit */ true);
  }

  if (check_write_error(thd)) {
    /*
      "all == true" means that a "rollback statement" triggered the error and
      this function was called. However, this must not happen as a rollback
      is written directly to the binary log. And in auto-commit mode, a single
      statement that is rolled back has the flag all == false.
    */
    DBUG_ASSERT(!all);
    /*
      We reach this point if the effect of a statement did not properly get into
      a cache and need to be rolled back.
    */
    error |= cache_mngr->trx_cache.truncate(thd, all);
  }

end:
  /* Deferred xa rollback to engines */
  if (!error && thd->lex->sql_command == SQLCOM_XA_ROLLBACK) {
    error = ha_rollback_low(thd, all);
    if (!error && !thd->is_error()) {
      /*
        XA-rollback ignores the gtid_state, if the transaciton
        is empty.
      */
      if (is_empty && !thd->slave_thread) gtid_state->update_on_rollback(thd);
      /*
        XA-rollback commits the new gtid_state, if transaction
        is not empty.
      */
      else {
        gtid_state->update_on_commit(thd);
        /*
          Inform hook listeners that a XA ROLLBACK did commit, that
          is, did log a transaction to the binary log.
        */
        // semi-sync plugin only called when raft is not enabled
        if (!enable_raft_plugin)
          (void)RUN_HOOK(transaction, after_commit, (thd, all));
      }
    }
  }
  /*
    When a statement errors out on auto-commit mode it is rollback
    implicitly, so the same should happen to its GTID.
  */
  if (!thd->in_active_multi_stmt_transaction())
    gtid_state->update_on_rollback(thd);

  /*
    TODO: some errors are overwritten, which may cause problem,
    fix it later.
  */
  DBUG_PRINT("return", ("error: %d", error));
  return error;
}

/**
  @note
  How do we handle this (unlikely but legal) case:
  @verbatim
    [transaction] + [update to non-trans table] + [rollback to savepoint] ?
  @endverbatim
  The problem occurs when a savepoint is before the update to the
  non-transactional table. Then when there's a rollback to the savepoint, if we
  simply truncate the binlog cache, we lose the part of the binlog cache where
  the update is. If we want to not lose it, we need to write the SAVEPOINT
  command and the ROLLBACK TO SAVEPOINT command to the binlog cache. The latter
  is easy: it's just write at the end of the binlog cache, but the former
  should be *inserted* to the place where the user called SAVEPOINT. The
  solution is that when the user calls SAVEPOINT, we write it to the binlog
  cache (so no need to later insert it). As transactions are never intermixed
  in the binary log (i.e. they are serialized), we won't have conflicts with
  savepoint names when using mysqlbinlog or in the slave SQL thread.
  Then when ROLLBACK TO SAVEPOINT is called, if we updated some
  non-transactional table, we don't truncate the binlog cache but instead write
  ROLLBACK TO SAVEPOINT to it; otherwise we truncate the binlog cache (which
  will chop the SAVEPOINT command from the binlog cache, which is good as in
  that case there is no need to have it in the binlog).
*/

static int binlog_savepoint_set(handlerton *, THD *thd, void *sv) {
  DBUG_TRACE;
  int error = 1;

  String log_query;
  if (log_query.append(STRING_WITH_LEN("SAVEPOINT ")))
    return error;
  else
    append_identifier(thd, &log_query, thd->lex->ident.str,
                      thd->lex->ident.length);

  int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
  Query_log_event qinfo(thd, log_query.c_ptr_safe(), log_query.length(), true,
                        false, true, errcode);
  /*
    We cannot record the position before writing the statement
    because a rollback to a savepoint (.e.g. consider it "S") would
    prevent the savepoint statement (i.e. "SAVEPOINT S") from being
    written to the binary log despite the fact that the server could
    still issue other rollback statements to the same savepoint (i.e.
    "S").
    Given that the savepoint is valid until the server releases it,
    ie, until the transaction commits or it is released explicitly,
    we need to log it anyway so that we don't have "ROLLBACK TO S"
    or "RELEASE S" without the preceding "SAVEPOINT S" in the binary
    log.
  */
  if (!(error = mysql_bin_log.write_event(&qinfo)))
    binlog_trans_log_savepos(thd, (my_off_t *)sv);

  return error;
}

static int binlog_savepoint_rollback(handlerton *, THD *thd, void *sv) {
  DBUG_TRACE;
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);
  my_off_t pos = *(my_off_t *)sv;
  DBUG_ASSERT(pos != ~(my_off_t)0);

  /*
    Write ROLLBACK TO SAVEPOINT to the binlog cache if we have updated some
    non-transactional table. Otherwise, truncate the binlog cache starting
    from the SAVEPOINT command.
  */
  if (trans_cannot_safely_rollback(thd)) {
    String log_query;
    if (log_query.append(STRING_WITH_LEN("ROLLBACK TO ")))
      return 1;
    else {
      /*
        Before writing identifier to the binlog, make sure to
        quote the identifier properly so as to prevent any SQL
        injection on the slave.
      */
      append_identifier(thd, &log_query, thd->lex->ident.str,
                        thd->lex->ident.length);
    }

    int errcode = query_error_code(thd, thd->killed == THD::NOT_KILLED);
    Query_log_event qinfo(thd, log_query.c_ptr_safe(), log_query.length(), true,
                          false, true, errcode);
    return mysql_bin_log.write_event(&qinfo);
  }
  // Otherwise, we truncate the cache
  cache_mngr->trx_cache.restore_savepoint(pos);
  /*
    When a SAVEPOINT is executed inside a stored function/trigger we force the
    pending event to be flushed with a STMT_END_F flag and clear the table maps
    as well to ensure that following DMLs will have a clean state to start
    with. ROLLBACK inside a stored routine has to finalize possibly existing
    current row-based pending event with cleaning up table maps. That ensures
    that following DMLs will have a clean state to start with.
   */
  if (thd->in_sub_stmt) thd->clear_binlog_table_maps();
  return 0;
}

/**
   purge logs, master and slave sides both, related error code
   convertor.
   Called from @c purge_error_message(), @c MYSQL_BIN_LOG::reset_logs()

   @param  res  an error code as used by purging routines

   @return the user level error code ER_*
*/
static uint purge_log_get_error_code(int res) {
  uint errcode = 0;

  switch (res) {
    case 0:
      break;
    case LOG_INFO_EOF:
      errcode = ER_UNKNOWN_TARGET_BINLOG;
      break;
    case LOG_INFO_IO:
      errcode = ER_IO_ERR_LOG_INDEX_READ;
      break;
    case LOG_INFO_INVALID:
      errcode = ER_BINLOG_PURGE_PROHIBITED;
      break;
    case LOG_INFO_SEEK:
      errcode = ER_FSEEK_FAIL;
      break;
    case LOG_INFO_MEM:
      errcode = ER_OUT_OF_RESOURCES;
      break;
    case LOG_INFO_FATAL:
      errcode = ER_BINLOG_PURGE_FATAL_ERR;
      break;
    case LOG_INFO_IN_USE:
      errcode = ER_LOG_IN_USE;
      break;
    case LOG_INFO_EMFILE:
      errcode = ER_BINLOG_PURGE_EMFILE;
      break;
    default:
      errcode = ER_LOG_PURGE_UNKNOWN_ERR;
      break;
  }

  return errcode;
}

/**
  Check whether binlog state allows to safely release MDL locks after
  rollback to savepoint.

  @param thd   The client thread that executes the transaction.

  @return true  - It is safe to release MDL locks.
          false - If it is not.
*/
static bool binlog_savepoint_rollback_can_release_mdl(handlerton *, THD *thd) {
  DBUG_TRACE;
  /**
    If we have not updated any non-transactional tables rollback
    to savepoint will simply truncate binlog cache starting from
    SAVEPOINT command. So it should be safe to release MDL acquired
    after SAVEPOINT command in this case.
  */
  return !trans_cannot_safely_rollback(thd);
}

/**
  Adjust log offset in the binary log file for all running slaves
  This class implements call back function for do_for_all_thd().
  It is called for each thd in thd list to adjust offset.
*/
class Adjust_offset : public Do_THD_Impl {
 public:
  Adjust_offset(my_off_t value, bool is_relay_log)
      : m_purge_offset(value), m_relay_log(is_relay_log) {}
  virtual void operator()(THD *thd) {
    LOG_INFO *linfo = thd->current_linfo;
    mysql_mutex_lock(&thd->LOCK_thd_data);
    if (linfo && (!enable_raft_plugin || linfo->is_relay_log == m_relay_log)) {
      /*
        Index file offset can be less that purge offset only if
        we just started reading the index file. In that case
        we have nothing to adjust.
      */
      if (linfo->index_file_offset < m_purge_offset)
        linfo->fatal = (linfo->index_file_offset != 0);
      else
        linfo->index_file_offset -= m_purge_offset;
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

 private:
  my_off_t m_purge_offset;
  bool m_relay_log;
};

class Adjust_linfo_in_dump_thread : public Do_THD_Impl {
 public:
  explicit Adjust_linfo_in_dump_thread(bool is_relay_log) {
    m_relay_log = is_relay_log;
  }
  virtual void operator()(THD *thd) {
    LOG_INFO *linfo = thd->current_linfo;
    if (linfo && linfo->is_used_by_dump_thd) {
      mysql_mutex_lock(&thd->LOCK_thd_data);
      linfo->is_relay_log = m_relay_log;
      mysql_mutex_unlock(&thd->LOCK_thd_data);
    }
  }

 private:
  bool m_relay_log;
};

/*
  Adjust the position pointer in the binary log file for all running slaves.

  SYNOPSIS
    adjust_linfo_offsets()
    purge_offset Number of bytes removed from start of log index file

  NOTES
    - This is called when doing a PURGE when we delete lines from the
      index log file.

  REQUIREMENTS
    - Before calling this function, we have to ensure that no threads are
      using any binary log file before purge_offset.

  TODO
    - Inform the slave threads that they should sync the position
      in the binary log file with flush_relay_log_info.
      Now they sync is done for next read.
*/
static void adjust_linfo_offsets(my_off_t purge_offset, bool is_relay_log) {
  Adjust_offset adjust_offset(purge_offset, is_relay_log);
  Global_THD_manager::get_instance()->do_for_all_thd(&adjust_offset);
}

static void adjust_linfo_in_dump_threads(bool is_relay_log) {
  Adjust_linfo_in_dump_thread adjust_linfo_in_dump_thread(is_relay_log);
  Global_THD_manager::get_instance()->do_for_all_thd(
      &adjust_linfo_in_dump_thread);
}

/**
  This class implements Call back function for do_for_all_thd().
  It is called for each thd in thd list to count
  threads using bin log file
*/

class Log_in_use : public Do_THD_Impl {
 public:
  Log_in_use(const char *value) : m_log_name(value), m_count(0) {
    m_log_name_len = strlen(m_log_name) + 1;
  }
  virtual void operator()(THD *thd) {
    LOG_INFO *linfo;
    mysql_mutex_lock(&thd->LOCK_thd_data);
    if ((linfo = thd->current_linfo)) {
      if (!strncmp(m_log_name, linfo->log_file_name, m_log_name_len)) {
        LogErr(WARNING_LEVEL, ER_BINLOG_FILE_BEING_READ_NOT_PURGED, m_log_name,
               thd->thread_id());
        m_count++;
      }
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }
  int get_count() { return m_count; }

 private:
  const char *m_log_name;
  size_t m_log_name_len;
  int m_count;
};

static int log_in_use(const char *log_name) {
  Log_in_use log_in_use(log_name);
#ifndef DBUG_OFF
  if (current_thd)
    DEBUG_SYNC(current_thd, "purge_logs_after_lock_index_before_thread_count");
#endif
  Global_THD_manager::get_instance()->do_for_all_thd(&log_in_use);
  return log_in_use.get_count();
}

static bool purge_error_message(THD *thd, int res) {
  uint errcode;

  if ((errcode = purge_log_get_error_code(res)) != 0) {
    my_error(errcode, MYF(0));
    return true;
  }
  my_ok(thd);
  return false;
}

bool is_transaction_empty(THD *thd) {
  DBUG_TRACE;
  int rw_ha_count = check_trx_rw_engines(thd, Transaction_ctx::SESSION);
  rw_ha_count += check_trx_rw_engines(thd, Transaction_ctx::STMT);
  return rw_ha_count == 0;
}

int check_trx_rw_engines(THD *thd, Transaction_ctx::enum_trx_scope trx_scope) {
  DBUG_TRACE;

  int rw_ha_count = 0;
  Ha_trx_info *ha_list =
      (Ha_trx_info *)thd->get_transaction()->ha_trx_info(trx_scope);

  for (Ha_trx_info *ha_info = ha_list; ha_info; ha_info = ha_info->next()) {
    if (ha_info->is_trx_read_write()) ++rw_ha_count;
  }
  return rw_ha_count;
}

bool is_empty_transaction_in_binlog_cache(const THD *thd) {
  DBUG_TRACE;

  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);
  if (cache_mngr != nullptr && cache_mngr->has_empty_transaction()) {
    return true;
  }

  return false;
}

/**
  This function checks if a transactional table was updated by the
  current transaction.

  @param thd The client thread that executed the current statement.
  @return
    @c true if a transactional table was updated, @c false otherwise.
*/
bool trans_has_updated_trans_table(const THD *thd) {
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);

  return (cache_mngr ? !cache_mngr->trx_cache.is_binlog_empty() : 0);
}

/**
  This function checks if a transactional table was updated by the
  current statement.

  @param ha_list Registered storage engine handler list.
  @return
    @c true if a transactional table was updated, @c false otherwise.
*/
bool stmt_has_updated_trans_table(Ha_trx_info *ha_list) {
  const Ha_trx_info *ha_info;
  for (ha_info = ha_list; ha_info; ha_info = ha_info->next()) {
    if (ha_info->is_trx_read_write() && ha_info->ht() != binlog_hton)
      return (true);
  }
  return (false);
}

/**
  This function checks if a transaction, either a multi-statement
  or a single statement transaction is about to commit or not.

  @param thd The client thread that executed the current statement.
  @param all Committing a transaction (i.e. true) or a statement
             (i.e. false).
  @return
    @c true if committing a transaction, otherwise @c false.
*/
bool ending_trans(THD *thd, const bool all) {
  return (all || ending_single_stmt_trans(thd, all));
}

/**
  This function checks if a single statement transaction is about
  to commit or not.

  @param thd The client thread that executed the current statement.
  @param all Committing a transaction (i.e. true) or a statement
             (i.e. false).
  @return
    @c true if committing a single statement transaction, otherwise
    @c false.
*/
bool ending_single_stmt_trans(THD *thd, const bool all) {
  return (!all && !thd->in_multi_stmt_transaction_mode());
}

/**
  This function checks if a transaction cannot be rolled back safely.

  @param thd The client thread that executed the current statement.
  @return
    @c true if cannot be safely rolled back, @c false otherwise.
*/
bool trans_cannot_safely_rollback(const THD *thd) {
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);

  return cache_mngr->trx_cache.cannot_rollback();
}

/**
  This function checks if current statement cannot be rollded back safely.

  @param thd The client thread that executed the current statement.
  @return
    @c true if cannot be safely rolled back, @c false otherwise.
*/
bool stmt_cannot_safely_rollback(const THD *thd) {
  return thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT);
}

/**
  Execute a PURGE BINARY LOGS TO @<log@> command.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @param to_log Name of the last log to purge.

  @retval false success
  @retval true failure
*/
bool purge_master_logs(THD *thd, const char *to_log) {
  char search_file_name[FN_REFLEN];
  if (!mysql_bin_log.is_open()) {
    my_ok(thd);
    return false;
  }

  mysql_bin_log.make_log_name(search_file_name, to_log);
  return purge_error_message(
      thd, mysql_bin_log.purge_logs(
               search_file_name, false, true /*need_lock_index=true*/,
               true /*need_update_threads=true*/, nullptr, false));
}

/**
  Execute a PURGE BINARY LOGS BEFORE @<date@> command.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @param purge_time Date before which logs should be purged.

  @retval false success
  @retval true failure
*/
bool purge_master_logs_before_date(THD *thd, time_t purge_time) {
  if (!mysql_bin_log.is_open()) {
    my_ok(thd);
    return false;
  }
  return purge_error_message(
      thd, mysql_bin_log.purge_logs_before_date(purge_time, false));
}

/**
  Execute a PURGE RAFT LOGS TO <log-name> command.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @param to_log Name of the last log to purge.

  @retval false success
  @retval true failure
*/
bool purge_raft_logs(THD *thd, const char *to_log) {
  // This is no-op when raft is not enabled
  if (!enable_raft_plugin) return false;

  // If mysql_bin_log is not an apply log, then it represents the 'raft logs' on
  // leader. Call purge_master_logs() to handle the purge correctly
  if (!mysql_bin_log.is_apply_log) return purge_master_logs(thd, to_log);

  Master_info *active_mi;
  if (!get_and_lock_master_info(&active_mi)) {
    return false;
  }

  Relay_log_info *rli = active_mi->rli;

  // Lock requirement and ordering based on SQL appliers next_event loop
  mysql_mutex_lock(&rli->data_lock);
  rli->relay_log.lock_index();
  char search_file_name[FN_REFLEN];
  mysql_bin_log.make_log_name(search_file_name, to_log);

  // Note that we pass max_log as group_relay_log_name. This is because we
  // should not purge anything that is still needed by sql appliers.
  // group_relay_log_name should be captured by 'in_use' check in
  // purge_logs(). However when sql_threads are stopped and a purge command is
  // issued, then 'in_use' check will not be sufficient and we might end up
  // deleting raft logs which are not yet applied. Hence we explicitly pass
  // 'max_log' asking purge_logs() to not purge anything at or beyond 'max_log'
  bool error = purge_error_message(
      thd, rli->relay_log.purge_logs(search_file_name,
                                     /*included=*/false,
                                     /*need_lock_index=*/false,
                                     /*need_update_threads=*/true,
                                     /*decrease_log_space=*/nullptr,
                                     /*auto_purge=*/false,
                                     rli->get_group_relay_log_name()));

  if (!error) {
    error = update_relay_log_cordinates(rli);
  }

  rli->relay_log.unlock_index();
  mysql_mutex_unlock(&rli->data_lock);
  unlock_master_info(active_mi);

  return error;
}

/**
  Execute a PURGE RAFT LOGS BEFORE <date> command.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @param purge_time Date before which logs should be purged.

  @retval false success
  @retval true failure
*/
bool purge_raft_logs_before_date(THD *thd, time_t purge_time) {
  // This is no-op when raft is not enabled
  if (!enable_raft_plugin) return false;

  // If mysql_bin_log is not an apply log, then it represents the 'raft logs' on
  // leader. Call purge_master_logs_before_date() to handle the purge
  // correctly
  if (!mysql_bin_log.is_apply_log)
    return purge_master_logs_before_date(thd, purge_time);

  Master_info *active_mi;
  if (!get_and_lock_master_info(&active_mi)) {
    return false;
  }

  Relay_log_info *rli = active_mi->rli;

  // Lock requirement and ordering based SQL appliers next_event loop
  mysql_mutex_lock(&rli->data_lock);
  rli->relay_log.lock_index();

  // Note that we pass max_log as group_relay_log_name. This is because we
  // should not purge anything that is still needed by sql appliers.
  // group_relay_log_name should be captured by 'in_use' check in
  // purge_logs(). However, when sql_threads are stopped and a purge command is
  // issued, then 'in_use' check will not be sufficient and we might end up
  // deleting raft logs which are not yet applied. Hence, we explicitly pass
  // 'max_log' asking purge_logs() to not purge anything at or beyond 'max_log'
  auto error = purge_error_message(
      thd, rli->relay_log.purge_logs_before_date(
               purge_time,
               /*auto_purge=*/false,
               /*stop_purge=*/false,
               /*need_lock_index=*/false, rli->get_group_relay_log_name()));

  if (!error) {
    error = update_relay_log_cordinates(rli);
  }
  rli->relay_log.unlock_index();
  mysql_mutex_unlock(&rli->data_lock);
  unlock_master_info(active_mi);
  return error;
}

/**
  Updates the index file cordinates in relay log info. All required locks need
  to be acquired by the caller

  @param rli Relay log info that needs to be updated

  @retval FALSE success
  @retval TRUE failure
*/
bool update_relay_log_cordinates(Relay_log_info *rli) {
  auto applier_reader = global_applier_reader.lock();
  if (!applier_reader) return false;
  int error = applier_reader->update_relay_log_coordinates(rli);

  if (error) {
    if (binlog_error_action == ABORT_SERVER ||
        binlog_error_action == ROLLBACK_TRX) {
      exec_binlog_error_action_abort(
          "Could not update relay log position "
          "for applier threads. Aborting server");
    }
  }
  return error;
}

/**
  Implement 'show raft logs' sql command
  @param thd Thread descriptor

  @retval false success
  @retval true failure
*/
bool show_raft_logs(THD *thd) {
  uint length;
  char file_name_and_gtid_set_length[FN_REFLEN + 22];
  File file;
  LOG_INFO cur;
  bool exit_loop = false;
  const char *errmsg = 0;

  // Redirect to show_binlog() on leader instances
  if (!mysql_bin_log.is_apply_log) return show_binlogs(thd);

  Master_info *active_mi;
  if (!get_and_lock_master_info(&active_mi)) {
    my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW RAFT LOGS",
             "No master info or relay log info present");
    return true;
  }

  Relay_log_info *rli = active_mi->rli;

  // Capture the current log
  rli->relay_log.get_current_log(&cur, /*need_lock_log=*/true);

  // Prevent new files from sneaking ino the index beyond this point. We only
  // read in the index till cur.log_file_name
  rli->relay_log.lock_index();

  IO_CACHE *index_file = rli->relay_log.get_index_file();
  Protocol *protocol = thd->get_protocol();

  List<Item> field_list;
  field_list.push_back(new Item_empty_string("Log_name", 255));
  field_list.push_back(
      new Item_return_int("File_size", 20, MYSQL_TYPE_LONGLONG));

  int error = 0;
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    error = 1;
    errmsg = "Protocol failed to send metadata";
    goto err;
  }

  reinit_io_cache(index_file, READ_CACHE, (my_off_t)0, 0, 0);

  /* The file ends with EOF or empty line */
  while ((length = my_b_gets(index_file, file_name_and_gtid_set_length,
                             FN_REFLEN + 22)) > 1 &&
         !exit_loop) {
    int dir_len;
    ulonglong file_length = 0;  // Length if open fails

    file_name_and_gtid_set_length[length - 1] = 0;
    uint gtid_set_length =
        split_file_name_and_gtid_set_length(file_name_and_gtid_set_length);
    if (gtid_set_length) {
      my_b_seek(index_file, my_b_tell(index_file) + gtid_set_length + 1);
    }

    char *fname = file_name_and_gtid_set_length;
    length = strlen(fname);
    dir_len = dirname_length(fname);
    length -= dir_len;

    protocol->start_row();
    protocol->store_string(fname + dir_len, length, &my_charset_bin);

    if (!(strncmp(fname + dir_len, cur.log_file_name + dir_len, length))) {
      /* Reached the position of the current file in the index. State the size
         of this file as cur.pos and exit the loop */
      file_length = cur.pos;
      exit_loop = true;
    } else {
      /* this is an old log, open it and find the size */
      if ((file = mysql_file_open(key_file_relaylog, fname, O_RDONLY,
                                  MYF(0))) >= 0) {
        file_length = (ulonglong)mysql_file_seek(file, 0L, MY_SEEK_END, MYF(0));
        mysql_file_close(file, MYF(0));
      }
    }
    protocol->store(file_length);
    if (protocol->end_row()) {
      error = 1;
      errmsg = "Failure in protocol write";
      goto err;
    }
  }

  if (index_file->error == -1) {
    error = 1;
    errmsg = "Index file error";
    goto err;
  }

  my_eof(thd);

err:
  rli->relay_log.unlock_index();
  unlock_master_info(active_mi);
  if (errmsg) {
    my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW RAFT LOGS", errmsg);
  }

  return error;
}

bool get_and_lock_master_info(Master_info **master_info) {
  channel_map.rdlock();
  Master_info *active_mi = channel_map.get_default_channel_mi();

  if (active_mi == NULL || active_mi->rli == NULL) {
    channel_map.unlock();
    return false;
  }
  // TODO(pgl): Verify during integration. m_channel_lock might not be needed
  // here
  active_mi->channel_rdlock();
  if (active_mi->rli == NULL) {
    active_mi->channel_unlock();
    channel_map.unlock();
    return false;
  }
  *master_info = active_mi;
  return true;
}

void unlock_master_info(Master_info *master_info) {
  master_info->channel_unlock();
  channel_map.unlock();
}

/*
  Helper function to get the error code of the query to be binlogged.
 */
int query_error_code(const THD *thd, bool not_killed) {
  int error;

  if (not_killed) {
    error = thd->is_error() ? thd->get_stmt_da()->mysql_errno() : 0;

    /* thd->get_stmt_da()->sql_errno() might be ER_SERVER_SHUTDOWN or
       ER_QUERY_INTERRUPTED, So here we need to make sure that error
       is not set to these errors when specified not_killed by the
       caller.
    */
    if (error == ER_SERVER_SHUTDOWN || error == ER_QUERY_INTERRUPTED) error = 0;
  } else
    error = thd->killed;

  return error;
}

/**
  Copy content of 'from' file from offset to 'to' file.

  - We do the copy outside of the IO_CACHE as the cache
  buffers would just make things slower and more complicated.
  In most cases the copy loop should only do one read.

  @param from          File to copy.
  @param to            File to copy to.
  @param offset        Offset in 'from' file.


  @retval
    0    ok
  @retval
    -1    error
*/
static bool copy_file(IO_CACHE *from, IO_CACHE *to, my_off_t offset) {
  int bytes_read;
  uchar io_buf[IO_SIZE * 2];
  DBUG_TRACE;

  mysql_file_seek(from->file, offset, MY_SEEK_SET, MYF(0));
  while (true) {
    if ((bytes_read = (int)mysql_file_read(from->file, io_buf, sizeof(io_buf),
                                           MYF(MY_WME))) < 0)
      goto err;
    if (DBUG_EVALUATE_IF("fault_injection_copy_part_file", 1, 0))
      bytes_read = bytes_read / 2;
    if (!bytes_read) break;  // end of file
    if (mysql_file_write(to->file, io_buf, bytes_read, MYF(MY_WME | MY_NABP)))
      goto err;
  }

  return false;

err:
  return true;
}

/**
   Load data's io cache specific hook to be executed
   before a chunk of data is being read into the cache's buffer
   The fuction instantianates and writes into the binlog
   replication events along LOAD DATA processing.

   @param file  pointer to io-cache
   @retval 0 success
   @retval 1 failure
*/
int log_loaded_block(IO_CACHE *file) {
  DBUG_TRACE;
  LOAD_FILE_INFO *lf_info;
  uint block_len;
  /* buffer contains position where we started last read */
  uchar *buffer = (uchar *)my_b_get_buffer_start(file);
  uint max_event_size = current_thd->variables.max_allowed_packet;
  lf_info = (LOAD_FILE_INFO *)file->arg;
  if (lf_info->thd->is_current_stmt_binlog_format_row()) return 0;
  if (lf_info->last_pos_in_file != HA_POS_ERROR &&
      lf_info->last_pos_in_file >= my_b_get_pos_in_file(file))
    return 0;

  for (block_len = (uint)(my_b_get_bytes_in_buffer(file)); block_len > 0;
       buffer += min(block_len, max_event_size),
      block_len -= min(block_len, max_event_size)) {
    lf_info->last_pos_in_file = my_b_get_pos_in_file(file);
    if (lf_info->logged_data_file) {
      Append_block_log_event a(lf_info->thd, lf_info->thd->db().str, buffer,
                               min(block_len, max_event_size),
                               lf_info->log_delayed);
      if (mysql_bin_log.write_event(&a)) return 1;
    } else {
      Begin_load_query_log_event b(lf_info->thd, lf_info->thd->db().str, buffer,
                                   min(block_len, max_event_size),
                                   lf_info->log_delayed);
      if (mysql_bin_log.write_event(&b)) return 1;
      lf_info->logged_data_file = true;
    }
  }
  return 0;
}

/* Helper function for SHOW BINLOG/RELAYLOG EVENTS */
template <class BINLOG_FILE_READER>
bool show_binlog_events(THD *thd, MYSQL_BIN_LOG *binary_log) {
  Protocol *protocol = thd->get_protocol();
  List<Item> field_list;
  std::string errmsg;
  LOG_INFO linfo(binary_log->is_relay_log);

  DBUG_TRACE;

  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_SHOW_BINLOG_EVENTS ||
              thd->lex->sql_command == SQLCOM_SHOW_RELAYLOG_EVENTS);

  if (binary_log->is_open()) {
    LEX_MASTER_INFO *lex_mi = &thd->lex->mi;
    SELECT_LEX_UNIT *unit = thd->lex->unit;
    ha_rows event_count, limit_start, limit_end;
    my_off_t pos =
        max<my_off_t>(BIN_LOG_HEADER_SIZE, lex_mi->pos);  // user-friendly
    char search_file_name[FN_REFLEN], *name;
    const char *log_file_name = lex_mi->log_file_name;
    Log_event *ev = nullptr;

    unit->set_limit(thd, thd->lex->current_select());
    limit_start = unit->offset_limit_cnt;
    limit_end = unit->select_limit_cnt;

    name = search_file_name;
    if (log_file_name)
      binary_log->make_log_name(search_file_name, log_file_name);
    else
      name = nullptr;  // Find first log

    linfo.index_file_offset = 0;

    if (binary_log->find_log_pos(&linfo, name, true /*need_lock_index=true*/)) {
      errmsg = "Could not find target log";
      goto err;
    }

    mysql_mutex_lock(&thd->LOCK_thd_data);
    thd->current_linfo = &linfo;
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    BINLOG_FILE_READER binlog_file_reader(
        opt_master_verify_checksum,
        std::max(thd->variables.max_allowed_packet,
                 binlog_row_event_max_size + MAX_LOG_EVENT_HEADER));

    if (binlog_file_reader.open(linfo.log_file_name, pos)) {
      errmsg = binlog_file_reader.get_error_str();
      goto err;
    }

    /*
      Adjust the pos to the correct starting offset of an event after the
      specified position if it is an invalid starting offset.
    */
    pos = binlog_file_reader.position();

    /*
      For 'in-active' binlog file, it is safe to read all events in it. But
      for 'active' binlog file, it is only safe to read the events before
      get_binlog_end_pos().

      Binlog rotation may happen after calling is_active(). In this case,
      end_pos will NOT be set to 0 while the file is actually not 'active'.
      It is safe, since 'end_pos' still expresses a correct position.
    */
    my_off_t end_pos = binary_log->get_binlog_end_pos();
    if (!binary_log->is_active(linfo.log_file_name)) end_pos = 0;

    DEBUG_SYNC(thd, "after_show_binlog_event_found_file");

    /**
      Relaylog_file_reader and Binlog_file_reader are typedefs to
      Basic_binlog_file_reader whereas Relaylog_file_reader uses
      a Relaylog_ifile in the template instantiation and
      Binlog_file_reader uses a Binlog_ifile in the template
      instantiation.

      Binlog_ifile and Relaylog_ifile differ only in the open()
      member function and they both derive from Basic_binlog_ifile.

      Therefore, it is OK to cast to Binlog_file_reader here.

      TODO: in the future investigate if some refactoring is needed
            here. Perhaps make the Iterator itself templated.
     */
    binlog::tools::Iterator it(
        reinterpret_cast<Binlog_file_reader *>(&binlog_file_reader));

    /*
      Unpacked events shall copy their part of the buffer from uncompressed
      buffer (the cointainer, i.e., the buffer iterator goes out of scope
      once the events are inflated and put in a vector). However, it is
      unclear if the *buffer* from which events are deserialized is still
      needed for the porposes of displaying events in SHOW BINLOG/RELAYLOG
      EVENTS.
    */
    my_off_t last_log_pos = 0;
    for (event_count = 0, ev = it.begin(); ev != it.end();) {
      DEBUG_SYNC(thd, "wait_in_show_binlog_events_loop");
      if (event_count >= limit_start &&
          ev->net_send(protocol, linfo.log_file_name, pos)) {
        /* purecov: begin inspected */
        errmsg = "Net error";
        delete ev;
        ev = nullptr;
        goto err;
        /* purecov: end */
      }
      last_log_pos = ev->common_header->log_pos;
      delete ev;
      ev = nullptr;
      pos = binlog_file_reader.position();

      if (++event_count == limit_end) break;
      if ((ev = it.next()) == it.end()) break;
      if (it.has_error()) break;
      if (end_pos > 0 && pos >= end_pos &&
          (ev->common_header->log_pos != last_log_pos)) {
        delete ev;
        ev = nullptr;
        break;
      }
    }

    if (binlog_file_reader.has_fatal_error())
      errmsg = binlog_file_reader.get_error_str();
    else if (it.has_error())
      errmsg = it.get_error_message(); /* purecov: inspected */
    else
      errmsg = "";
  }
  // Check that linfo is still on the function scope.
  DEBUG_SYNC(thd, "after_show_binlog_events");

err:
  if (!errmsg.empty()) {
    if (thd->lex->sql_command == SQLCOM_SHOW_RELAYLOG_EVENTS)
      my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW RELAYLOG EVENTS",
               errmsg.c_str());
    else
      my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW BINLOG EVENTS",
               errmsg.c_str());
  } else
    my_eof(thd);

  mysql_mutex_lock(&thd->LOCK_thd_data);
  thd->current_linfo = nullptr;
  mysql_mutex_unlock(&thd->LOCK_thd_data);
  return !errmsg.empty();
}

bool show_binlog_events(THD *thd, MYSQL_BIN_LOG *binary_log) {
  if (binary_log->is_relay_log)
    return show_binlog_events<Relaylog_file_reader>(thd, binary_log);
  return show_binlog_events<Binlog_file_reader>(thd, binary_log);
}

/**
  Execute a SHOW BINLOG EVENTS statement.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @retval false success
  @retval true failure
*/
bool mysql_show_binlog_events(THD *thd) {
  List<Item> field_list;
  DBUG_TRACE;

  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_SHOW_BINLOG_EVENTS);

  Log_event::init_show_field_list(&field_list);
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  /*
    Wait for handlers to insert any pending information
    into the binlog.  For e.g. ndb which updates the binlog asynchronously
    this is needed so that the uses sees all its own commands in the binlog
  */
  ha_binlog_wait(thd);

  return show_binlog_events(thd, &mysql_bin_log);
}

/**
  Executes SHOW GTID_EXECUTED IN 'log_name' FROM 'log_pos' statement.
  Scans the binlog 'log_name' to build Gtid_set by adding
  previous GTIDs and all the GTIDs upto the position 'log_pos'.

  @paarm thd Pointer to the THD object for the client thread executing the
             statement.
  @retval false Success
  @retval true  Failure
*/
bool show_gtid_executed(THD *thd) {
  DBUG_ENTER("show_gtid_executed");
  LEX *lex = thd->lex;

  DBUG_ASSERT(lex->sql_command == SQLCOM_GTID_EXECUTED);

  // Handle empty file name
  if (!lex->mi.log_file_name) {
    my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW GTID_EXECUTED",
             "binlog file name is not specified");
    DBUG_RETURN(true);
  }
  Protocol *protocol = thd->get_protocol();
  Sid_map sid_map(NULL);
  Gtid_set gtid_executed(&sid_map);
  char file_name[FN_REFLEN];
  mysql_bin_log.make_log_name(file_name, lex->mi.log_file_name);

  MYSQL_BIN_LOG::enum_read_gtids_from_binlog_status ret =
      mysql_bin_log.read_gtids_from_binlog(file_name, &gtid_executed, NULL,
                                           NULL, &sid_map, false, false,
                                           lex->mi.pos);
  if (ret == MYSQL_BIN_LOG::ERROR || ret == MYSQL_BIN_LOG::TRUNCATED) {
    DBUG_RETURN(true);
  }
  char *gtid_executed_string;
  gtid_executed.to_string(&gtid_executed_string);
  uint gtid_executed_string_length = gtid_executed.get_string_length();

  List<Item> field_list;
  field_list.push_back(
      new Item_empty_string("Gtid_executed", gtid_executed_string_length));
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    my_free(gtid_executed_string);
    DBUG_RETURN(true);
  }
  protocol->start_row();
  protocol->store(gtid_executed_string, &my_charset_bin);
  if (protocol->end_row()) DBUG_RETURN(true);

  my_free(gtid_executed_string);
  my_eof(thd);
  DBUG_RETURN(false);
}

MYSQL_BIN_LOG::MYSQL_BIN_LOG(uint *sync_period, bool relay_log)
    : name(nullptr),
      write_error(false),
      inited(false),
      m_binlog_file(new Binlog_ofile()),
      m_key_LOCK_log(key_LOG_LOCK_log),
      bytes_written(0),
      lost_gtid_for_tailing(""),
      file_id(1),
      sync_period_ptr(sync_period),
      sync_counter(0),
      ha_last_updated_binlog_pos(0),
      ha_last_updated_binlog_file(0),
      non_xid_trxs(0),
      is_relay_log(relay_log),
      signal_cnt(0),
      checksum_alg_reset(binary_log::BINLOG_CHECKSUM_ALG_UNDEF),
      relay_log_checksum_alg(binary_log::BINLOG_CHECKSUM_ALG_UNDEF),
      engine_binlog_pos(ULLONG_MAX),
      previous_gtid_set_relaylog(nullptr),
      raft_cur_log_ext(0),
      setup_flush_done(false),
      is_rotating_caused_by_incident(false) {
  /*
    We don't want to initialize locks here as such initialization depends on
    safe_mutex (when using safe_mutex) which depends on MY_INIT(), which is
    called only in main(). Doing initialization here would make it happen
    before main().
  */
  index_file_name[0] = 0;
  engine_binlog_file[0] = 0;
  engine_binlog_max_gtid.clear();
  apply_file_count.store(0);
}

MYSQL_BIN_LOG::~MYSQL_BIN_LOG() { delete m_binlog_file; }

/* this is called only once */

void MYSQL_BIN_LOG::cleanup() {
  DBUG_TRACE;
  if (inited) {
    inited = false;
    close(LOG_CLOSE_INDEX | LOG_CLOSE_STOP_EVENT, true /*need_lock_log=true*/,
          true /*need_lock_index=true*/);
    mysql_mutex_destroy(&LOCK_log);
    mysql_mutex_destroy(&LOCK_index);
    mysql_mutex_destroy(&LOCK_commit);
    mysql_mutex_destroy(&LOCK_sync);
    mysql_mutex_destroy(&LOCK_binlog_end_pos);
    mysql_mutex_destroy(&LOCK_xids);
    mysql_mutex_destroy(&LOCK_non_xid_trxs);
    mysql_mutex_destroy(&LOCK_lost_gtids_for_tailing);
    mysql_cond_destroy(&update_cond);
    mysql_cond_destroy(&m_prep_xids_cond);
    mysql_cond_destroy(&non_xid_trxs_cond);
    if (!is_relay_log) {
      Commit_stage_manager::get_instance().deinit();
    }
    // Clear the HLC map, forcing mutexes and condvars to be cleaned up
    hlc.clear_database_hlc();
  }

  delete m_binlog_file;
  m_binlog_file = nullptr;
}

void MYSQL_BIN_LOG::init_pthread_objects() {
  DBUG_ASSERT(inited == 0);
  inited = true;

  mysql_mutex_init(m_key_LOCK_log, &LOCK_log, MY_MUTEX_INIT_SLOW);
  mysql_mutex_init(m_key_LOCK_index, &LOCK_index, MY_MUTEX_INIT_SLOW);
  mysql_mutex_init(m_key_LOCK_commit, &LOCK_commit, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(m_key_LOCK_sync, &LOCK_sync, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(m_key_LOCK_non_xid_trxs, &LOCK_non_xid_trxs,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(m_key_LOCK_binlog_end_pos, &LOCK_binlog_end_pos,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(m_key_LOCK_xids, &LOCK_xids, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(m_key_LOCK_lost_gtids_for_tailing,
                   &LOCK_lost_gtids_for_tailing, MY_MUTEX_INIT_FAST);
  mysql_cond_init(m_key_update_cond, &update_cond);
  mysql_cond_init(m_key_prep_xids_cond, &m_prep_xids_cond);
  mysql_cond_init(m_key_non_xid_trxs_cond, &non_xid_trxs_cond);
  if (!is_relay_log) {
    Commit_stage_manager::get_instance().init(
        m_key_LOCK_flush_queue, m_key_LOCK_sync_queue, m_key_LOCK_commit_queue,
        m_key_LOCK_done, m_key_COND_done);
  }
}

/**
  Check if a string is a valid number.

  @param str			String to test
  @param res			Store value here
  @param allow_wildcards	Set to 1 if we should ignore '%' and '_'

  @note
    For the moment the allow_wildcards argument is not used
    Should be moved to some other file.

  @retval
    1	String is a number
  @retval
    0	String is not a number
*/

static bool is_number(const char *str, ulong *res, bool allow_wildcards) {
  int flag;
  const char *start;
  DBUG_TRACE;

  flag = 0;
  start = str;
  while (*str++ == ' ')
    ;
  if (*--str == '-' || *str == '+') str++;
  while (my_isdigit(files_charset_info, *str) ||
         (allow_wildcards && (*str == wild_many || *str == wild_one))) {
    flag = 1;
    str++;
  }
  if (*str == '.') {
    for (str++; my_isdigit(files_charset_info, *str) ||
                (allow_wildcards && (*str == wild_many || *str == wild_one));
         str++, flag = 1)
      ;
  }
  if (*str != 0 || flag == 0) return false;
  if (res) *res = atol(start);
  return true; /* Number ok */
} /* is_number */

/**
  Find a unique filename for 'filename.#'.

  Set '#' to the highest existing log file extension plus one.

  This function will return nonzero if: (i) the generated name
  exceeds FN_REFLEN; (ii) if the number of extensions is exhausted;
  or (iii) some other error happened while examining the filesystem.

  @return
    nonzero if not possible to get unique filename.
*/

static int find_uniq_filename(char *name, uint32 new_index_number,
                              bool need_next = true,
                              ulong *cur_log_ext = nullptr) {
  uint i;
  char buff[FN_REFLEN], ext_buf[FN_REFLEN];
  MY_DIR *dir_info = nullptr;
  struct fileinfo *file_info;
  ulong max_found = 0, next = 0, number = 0;
  size_t buf_length, length;
  char *start, *end;
  int error = 0;
  DBUG_TRACE;

  length = dirname_part(buff, name, &buf_length);
  start = name + length;
  end = strend(start);

  *end = '.';
  length = (size_t)(end - start + 1);

  if ((DBUG_EVALUATE_IF(
          "error_unique_log_filename", 1,
          !(dir_info =
                my_dir(buff, MYF(MY_DONT_SORT)))))) {  // This shouldn't happen
    my_stpcpy(end, ".1");                              // use name+1
    return 1;
  }
  file_info = dir_info->dir_entry;
  for (i = dir_info->number_off_files; i--; file_info++) {
    if (strncmp(file_info->name, start, length) == 0 &&
        is_number(file_info->name + length, &number, false)) {
      max_found = std::max(max_found, number);
    }
  }
  my_dirend(dir_info);

  /* check if reached the maximum possible extension number */
  if (max_found >= MAX_LOG_UNIQUE_FN_EXT) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FILE_EXTENSION_NUMBER_EXHAUSTED, max_found);
    error = 1;
    goto end;
  }

  if (new_index_number > 0) {
    /*
      If "new_index_number" was specified, this means we are handling a
      "RESET MASTER TO" command and the binary log was already purged
      so max_found should be 0.
    */
    DBUG_ASSERT(max_found == 0);
    next = new_index_number;
  } else
    next = (need_next || max_found == 0) ? (max_found + 1) : max_found;

  if (cur_log_ext != nullptr) *cur_log_ext = next;

  if (sprintf(ext_buf, "%06lu", next) < 0) {
    error = 1;
    goto end;
  }
  *end++ = '.';

  /*
    Check if the generated extension size + the file name exceeds the
    buffer size used. If one did not check this, then the filename might be
    truncated, resulting in error.
   */
  if (((strlen(ext_buf) + (end - name)) >= FN_REFLEN)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FILE_NAME_TOO_LONG, name, ext_buf,
           (strlen(ext_buf) + (end - name)));
    error = 1;
    goto end;
  }

  if (sprintf(end, "%06lu", next) < 0) {
    error = 1;
    goto end;
  }

  /* print warning if reaching the end of available extensions. */
  if (next > MAX_ALLOWED_FN_EXT_RESET_MASTER)
    LogErr(WARNING_LEVEL, ER_BINLOG_FILE_EXTENSION_NUMBER_RUNNING_LOW, next,
           (MAX_LOG_UNIQUE_FN_EXT - next));

end:
  return error;
}

/**
  Mimic what generate_new_name does but do not increment the file ext
  after finding the last file, thus returning the current latest file

  @param new_name [out] new_name to use for log-file will be stored here
  @param log_name base name to use for the log-file. Used to generate new_name

  @retval 1 on error, 0 on success
*/
static int find_existing_last_file(char *new_name, const char *log_name) {
  fn_format(new_name, log_name, mysql_data_home, "", 4);
  if (fn_ext(log_name)[0]) return 0;

  if (find_uniq_filename(new_name, /*new_index_number=*/0,
                         /*need_next=*/false)) {
    my_printf_error(ER_NO_UNIQUE_LOGFILE,
                    ER_THD(current_thd, ER_NO_UNIQUE_LOGFILE),
                    MYF(ME_FATALERROR), log_name);
    // NO_LINT_DEBUG
    sql_print_error("Error trying to find existing last file for %s", log_name);
    return 1;
  }

  return 0;
}

int MYSQL_BIN_LOG::generate_new_name(char *new_name, const char *log_name,
                                     uint32 new_index_number) {
  fn_format(new_name, log_name, mysql_data_home, "", 4);
  if (!fn_ext(log_name)[0]) {
    if (find_uniq_filename(new_name, new_index_number, /*need_next=*/true,
                           &raft_cur_log_ext)) {
      if (current_thd != nullptr)
        my_printf_error(ER_NO_UNIQUE_LOGFILE,
                        ER_THD(current_thd, ER_NO_UNIQUE_LOGFILE),
                        MYF(ME_FATALERROR), log_name);
      LogErr(ERROR_LEVEL, ER_FAILED_TO_GENERATE_UNIQUE_LOGFILE, log_name);
      return 1;
    }
  }
  return 0;
}

/**
  @todo
  The following should be using fn_format();  We just need to
  first change fn_format() to cut the file name if it's too long.
*/
const char *MYSQL_BIN_LOG::generate_name(const char *log_name,
                                         const char *suffix, char *buff) {
  if (!log_name || !log_name[0]) {
    if (is_relay_log || log_bin_supplied)
      strmake(buff, default_logfile_name, FN_REFLEN - strlen(suffix) - 1);
    else
      strmake(buff, default_binlogfile_name, FN_REFLEN - strlen(suffix) - 1);

    return (const char *)fn_format(buff, buff, "", suffix,
                                   MYF(MY_REPLACE_EXT | MY_REPLACE_DIR));
  }
  // get rid of extension to avoid problems

  const char *p = fn_ext(log_name);
  uint length = (uint)(p - log_name);
  strmake(buff, log_name, min<size_t>(length, FN_REFLEN - 1));
  return (const char *)buff;
}

bool MYSQL_BIN_LOG::init_and_set_log_file_name(const char *log_name,
                                               const char *new_name,
                                               uint32 new_index_number) {
  if (new_name && !my_stpcpy(log_file_name, new_name))
    return true;
  else if (!new_name &&
           generate_new_name(log_file_name, log_name, new_index_number))
    return true;

  return false;
}

/**
  Open the logfile and init IO_CACHE.

  @param log_file_key        The file instrumentation key for this file
  @param log_name            The name of the log to open
  @param new_name            The new name for the logfile.
                             NULL forces generate_new_name() to be called.
  @param new_index_number    The binary log file index number to start from
                             after the RESET MASTER TO command is called.

  @return true if error, false otherwise.
*/

bool MYSQL_BIN_LOG::open(PSI_file_key log_file_key, const char *log_name,
                         const char *new_name, uint32 new_index_number) {
  DBUG_TRACE;
  bool ret = false;

  write_error = false;
  myf flags = MY_WME | MY_NABP | MY_WAIT_IF_FULL;
  if (is_relay_log) flags = flags | MY_REPORT_WAITING_IF_FULL;

  if (!(name = my_strdup(key_memory_MYSQL_LOG_name, log_name, MYF(MY_WME)))) {
    goto err;
  }

  if (init_and_set_log_file_name(name, new_name, new_index_number) ||
      DBUG_EVALUATE_IF("fault_injection_init_name", 1, 0))
    goto err;

  db[0] = 0;

  /* Keep the key for reopen */
  m_log_file_key = log_file_key;

  /*
    LOCK_sync guarantees that no thread is calling m_binlog_file to sync data
    to disk when another thread is opening the new file
    (FLUSH LOG or RESET MASTER).
  */
  if (!is_relay_log) mysql_mutex_lock(&LOCK_sync);

  ret = m_binlog_file->open(log_file_key, log_file_name, flags);

  if (!is_relay_log) mysql_mutex_unlock(&LOCK_sync);

  if (ret) goto err;

  atomic_log_state = LOG_OPENED;
  return false;

err:
  if (should_abort_on_binlog_error()) {
    exec_binlog_error_action_abort(
        "Either disk is full, file system is read only or "
        "there was an encryption error while opening the binlog. "
        "Aborting the server.");
  } else {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_OPEN_FOR_LOGGING, log_name, errno);
  }

  my_free(name);
  name = nullptr;
  atomic_log_state = LOG_CLOSED;
  return true;
}

bool MYSQL_BIN_LOG::open_index_file(const char *index_file_name_arg,
                                    const char *log_name,
                                    bool need_lock_index) {
  bool error = false;
  File index_file_nr = -1;
  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  /*
    First open of this class instance
    Create an index file that will hold all file names uses for logging.
    Add new entries to the end of it.
  */
  myf opt = MY_UNPACK_FILENAME;

  if (!enable_raft_plugin && my_b_inited(&index_file)) goto end;

  if (enable_raft_plugin && my_b_inited(&index_file)) end_io_cache(&index_file);

  if (!index_file_name_arg) {
    index_file_name_arg = log_name;  // Use same basename for index file
    opt = MY_UNPACK_FILENAME | MY_REPLACE_EXT;
  }
  fn_format(index_file_name, index_file_name_arg, mysql_data_home, ".index",
            opt);

  if (set_crash_safe_index_file_name(index_file_name_arg)) {
    error = true;
    goto end;
  }

  // case: crash_safe_index_file exists
  if (!my_access(crash_safe_index_file_name, F_OK)) {
    /*
      We need to move crash_safe_index_file to index_file if the index_file
      does not exist or delete it if the index_file exists when mysqld server
      restarts.
    */

    // case: index_file does not exist
    if (my_access(index_file_name, F_OK)) {
      if (my_rename(crash_safe_index_file_name, index_file_name, MYF(MY_WME))) {
        LogErr(ERROR_LEVEL, ER_BINLOG_CANT_MOVE_TMP_TO_INDEX,
               "MYSQL_BIN_LOG::open_index_file");
        error = true;
        goto end;
      }
    } else {
      if (close_crash_safe_index_file() ||
          my_delete(crash_safe_index_file_name, MYF(MY_WME))) {
        LogErr(ERROR_LEVEL, ER_BINLOG_CANT_DELETE_TMP_INDEX,
               "MYSQL_BIN_LOG::open_index_file");
        error = true;
        goto end;
      }
    }
  }

  if ((index_file_nr = mysql_file_open(m_key_file_log_index, index_file_name,
                                       O_RDWR | O_CREAT, MYF(MY_WME))) < 0 ||
      mysql_file_sync(index_file_nr, MYF(MY_WME)) ||
      init_io_cache_ext(&index_file, index_file_nr, IO_SIZE, READ_CACHE,
                        mysql_file_seek(index_file_nr, 0L, MY_SEEK_END, MYF(0)),
                        false, MYF(MY_WME | MY_WAIT_IF_FULL),
                        m_key_file_log_index_cache) ||
      DBUG_EVALUATE_IF("fault_injection_openning_index", 1, 0)) {
    /*
      TODO: all operations creating/deleting the index file or a log, should
      call my_sync_dir() or my_sync_dir_by_file() to be durable.
      TODO: file creation should be done with mysql_file_create()
      not mysql_file_open().
    */
    if (index_file_nr >= 0) mysql_file_close(index_file_nr, MYF(0));
    error = true;
    goto end;
  }

  /*
    Sync the index by purging any binary log file that is not registered.
    In other words, purge any binary log file that was created but not
    register in the index due to a crash.
  */
  if (set_purge_index_file_name(index_file_name_arg) ||
      open_purge_index_file(false) ||
      purge_index_entry(nullptr, nullptr, false) || close_purge_index_file() ||
      DBUG_EVALUATE_IF("fault_injection_recovering_index", 1, 0)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_SYNC_INDEX_FILE);
    error = true;
    goto end;
  }

end:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);
  return error;
}

int MYSQL_BIN_LOG::init_index_file() {
  char *index_file_name = NULL, *log_file_name = NULL;
  int error = 0;

  DBUG_ASSERT(enable_raft_plugin);

  myf opt = MY_UNPACK_FILENAME;
  char *apply_index_file_name_base = opt_applylog_index_name;
  if (!apply_index_file_name_base) {
    // Use the same base as the apply binlog file name
    apply_index_file_name_base = opt_apply_logname;
    opt = MY_UNPACK_FILENAME | MY_REPLACE_EXT;
  }

  // Create the apply index file name
  DBUG_ASSERT(apply_index_file_name_base != NULL);
  char apply_index_file[FN_REFLEN + 1];
  fn_format(apply_index_file, apply_index_file_name_base, mysql_data_home,
            ".index", opt);

  if (!my_access(apply_index_file, F_OK)) {
    // NO_LINT_DEBUG
    sql_print_information(
        "Binlog apply index file exists. Recovering mysqld "
        "based on binlog apply index file: %s",
        opt_applylog_index_name);
    index_file_name = opt_applylog_index_name;
    log_file_name = opt_apply_logname;
    is_apply_log = true;
  } else {
    // NO_LINT_DEBUG
    sql_print_information(
        "Binlog apply index file does not exist. Recovering "
        "mysqld based on binlog index file: %s",
        opt_binlog_index_name);
    index_file_name = opt_binlog_index_name;
    log_file_name = opt_bin_logname;
  }

  if (mysql_bin_log.open_index_file(index_file_name, log_file_name, true)) {
    // NO_LINT_DEBUG
    sql_print_error("Failed while opening index file");
    error = 1;
  }

  return error;
}

/**
  Remove logs from index that are not present on disk
  NOTE: this method will not update index with arbitrarily
  deleted logs. It will only remove entries of logs which
  are deleted from the beginning of the sequence
  @param need_lock_index        Need to lock index?
  @param need_update_threads    If we want to update the log coordinates
                                of all threads. False for relay logs,
                                true otherwise.
  @retval
   0    ok
  @retval
    LOG_INFO_IO    Got IO error while reading/writing file
    LOG_INFO_EOF   log-index-file is empty
*/
int MYSQL_BIN_LOG::remove_deleted_logs_from_index(bool need_lock_index,
                                                  bool need_update_threads) {
  int error;
  uint64_t no_of_log_files_purged = 0;
  uint64_t num_apply_files = 0;
  LOG_INFO log_info;

  DBUG_ENTER("remove_deleted_logs_from_index");

  if (need_lock_index) mysql_mutex_lock(&LOCK_index);

  if ((error = find_log_pos(&log_info, NullS, false /*need_lock_index=false*/)))
    goto err;

  while (true) {
    if (my_access(log_info.log_file_name, F_OK) == 0) break;

    int ret = find_next_log(&log_info, false /*need_lock_index=false*/);
    if (ret == LOG_INFO_EOF) {
      break;
    } else if (ret == LOG_INFO_IO) {
      LogErr(ERROR_LEVEL, ER_BINLOG_CANT_READ_INDEX,
             "MYSQL_BIN_LOG::remove_deleted_logs_from_index");
      goto err;
    }

    ++no_of_log_files_purged;
  }

  if (no_of_log_files_purged) {
    error = remove_logs_from_index(&log_info, need_update_threads);
    if (is_apply_log) {
      // Fix number of apply log file count
      if (apply_file_count >= no_of_log_files_purged) {
        apply_file_count -= no_of_log_files_purged;
        DBUG_ASSERT(enable_raft_plugin);
      } else {
        error = get_total_log_files(need_lock_index, &num_apply_files);
        apply_file_count.store(num_apply_files);

        // NO_LINT_DEBUG
        sql_print_information(
            "Fixed apply file count (%lu) by reading from "
            "index file.",
            apply_file_count.load());
      }
    }
  }
  DBUG_PRINT("info", ("num binlogs deleted = %lu", no_of_log_files_purged));

err:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);
  DBUG_RETURN(error);
}

/**
  Add the GTIDs from the given relaylog file and also
  update the IO thread transaction parser.

  @param filename Relaylog file to read from.
  @param retrieved_gtids Gtid_set to store the GTIDs found on the relaylog file.
  @param verify_checksum Set to true to verify event checksums.
  @param trx_parser The transaction boundary parser to be used in order to
  only add a GTID to the gtid_set after ensuring the transaction is fully
  stored on the relay log.
  @param partial_trx The trx_monitoring_info of the last incomplete transaction
  found in the relay log.

  @retval false The file was successfully read and all GTIDs from
  Previous_gtids and Gtid_log_event from complete transactions were added to
  the retrieved_set.
  @retval true There was an error during the procedure.
*/
static bool read_gtids_and_update_trx_parser_from_relaylog(
    const char *filename, Gtid_set *retrieved_gtids, bool verify_checksum,
    Transaction_boundary_parser *trx_parser,
    Gtid_monitoring_info *partial_trx) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("Opening file %s", filename));

  DBUG_ASSERT(retrieved_gtids != nullptr);
  DBUG_ASSERT(trx_parser != nullptr);
#ifndef DBUG_OFF
  unsigned long event_counter = 0;
#endif
  bool error = false;

  Relaylog_file_reader relaylog_file_reader(verify_checksum);
  if (relaylog_file_reader.open(filename)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FILE_OPEN_FAILED,
           relaylog_file_reader.get_error_str());

    /*
      As read_gtids_from_binlog() will not throw error on truncated
      relaylog files, we should do the same here in order to keep the
      current behavior.
    */
    if (relaylog_file_reader.get_error_type() ==
        Binlog_read_error::CANNOT_GET_FILE_PASSWORD)
      error = true;
    return error;
  }

  Log_event *ev = nullptr;
  bool seen_prev_gtids = false;
  ulong data_len = 0;

  while (!error && (ev = relaylog_file_reader.read_event_object()) != nullptr) {
    DBUG_PRINT("info", ("Read event of type %s", ev->get_type_str()));
#ifndef DBUG_OFF
    event_counter++;
#endif

    data_len = uint4korr(ev->temp_buf + EVENT_LEN_OFFSET);

    bool info_error{false};
    binary_log::Log_event_basic_info log_event_info;
    std::tie(info_error, log_event_info) = extract_log_event_basic_info(
        ev->temp_buf, data_len,
        relaylog_file_reader.format_description_event());

    if (info_error || trx_parser->feed_event(log_event_info, false)) {
      /*
        The transaction boundary parser found an error while parsing a
        sequence of events from the relaylog. As we don't know if the
        parsing has started from a reliable point (it might started in
        a relay log file that begins with the rest of a transaction
        that started in a previous relay log file), it is better to do
        nothing in this case. The boundary parser will fix itself once
        finding an event that represent a transaction boundary.

        Suppose the following relaylog:

         rl-bin.000011 | rl-bin.000012 | rl-bin.000013 | rl-bin-000014
        ---------------+---------------+---------------+---------------
         PREV_GTIDS    | PREV_GTIDS    | PREV_GTIDS    | PREV_GTIDS
         (empty)       | (UUID:1-2)    | (UUID:1-2)    | (UUID:1-2)
        ---------------+---------------+---------------+---------------
         XID           | QUERY(INSERT) | QUERY(INSERT) | XID
        ---------------+---------------+---------------+---------------
         GTID(UUID:2)  |
        ---------------+
         QUERY(CREATE  |
         TABLE t1 ...) |
        ---------------+
         GTID(UUID:3)  |
        ---------------+
         QUERY(BEGIN)  |
        ---------------+

        As it is impossible to determine the current Retrieved_Gtid_Set by only
        looking to the PREVIOUS_GTIDS on the last relay log file, and scanning
        events on it, we tried to find a relay log file that contains at least
        one GTID event during the backwards search.

        In the example, we will find a GTID only in rl-bin.000011, as the
        UUID:3 transaction was spanned across 4 relay log files.

        The transaction spanning can be caused by "FLUSH RELAY LOGS" commands
        on slave while it is queuing the transaction.

        So, in order to correctly add UUID:3 into Retrieved_Gtid_Set, we need
        to parse the relay log starting on the file we found the last GTID
        queued to know if the transaction was fully retrieved or not.

        Start scanning rl-bin.000011 after resetting the transaction parser
        will generate an error, as XID event is only expected inside a DML,
        but in this case, we can ignore this error and reset the parser.
      */
      trx_parser->reset();
      /*
        We also have to discard the GTID of the partial transaction that was
        not finished if there is one. This is needed supposing that an
        incomplete transaction was replicated with a GTID.

        GTID(1), QUERY(BEGIN), QUERY(INSERT), ANONYMOUS_GTID, QUERY(DROP ...)

        In the example above, without cleaning the partial_trx,
        the GTID(1) would be added to the Retrieved_Gtid_Set after the
        QUERY(DROP ...) event.

        GTID(1), QUERY(BEGIN), QUERY(INSERT), GTID(2), QUERY(DROP ...)

        In the example above the GTID(1) will also be discarded as the
        GTID(1) transaction is not complete.
      */
      if (partial_trx->is_processing_trx_set()) {
        DBUG_PRINT("info", ("Discarding Gtid(%d, %lld) as the transaction "
                            "wasn't complete and we found an error in the"
                            "transaction boundary parser.",
                            partial_trx->get_processing_trx_gtid()->sidno,
                            partial_trx->get_processing_trx_gtid()->gno));
        partial_trx->clear_processing_trx();
      }
    }

    switch (ev->get_type_code()) {
      case binary_log::FORMAT_DESCRIPTION_EVENT:
      case binary_log::ROTATE_EVENT:
        // do nothing; just accept this event and go to next
        break;
      case binary_log::PREVIOUS_GTIDS_LOG_EVENT: {
        seen_prev_gtids = true;
        // add events to sets
        Previous_gtids_log_event *prev_gtids_ev =
            (Previous_gtids_log_event *)ev;
        if (prev_gtids_ev->add_to_set(retrieved_gtids) != 0) {
          error = true;
          break;
        }
#ifndef DBUG_OFF
        char *prev_buffer = prev_gtids_ev->get_str(nullptr, nullptr);
        DBUG_PRINT("info", ("Got Previous_gtids from file '%s': Gtid_set='%s'.",
                            filename, prev_buffer));
        my_free(prev_buffer);
#endif
        break;
      }
      case binary_log::GTID_LOG_EVENT: {
        /* If we didn't find any PREVIOUS_GTIDS in this file */
        if (!seen_prev_gtids) {
          my_error(ER_BINLOG_LOGICAL_CORRUPTION, MYF(0), filename,
                   "The first global transaction identifier was read, but "
                   "no other information regarding identifiers existing "
                   "on the previous log files was found.");
          error = true;
          break;
        }

        Gtid_log_event *gtid_ev = (Gtid_log_event *)ev;
        rpl_sidno sidno = gtid_ev->get_sidno(retrieved_gtids->get_sid_map());
        ulonglong immediate_commit_timestamp =
            gtid_ev->immediate_commit_timestamp;
        longlong original_commit_timestamp = gtid_ev->original_commit_timestamp;

        if (sidno < 0) {
          error = true;
          break;
        } else {
          if (retrieved_gtids->ensure_sidno(sidno) != RETURN_STATUS_OK) {
            error = true;
            break;
          } else {
            Gtid gtid = {sidno, gtid_ev->get_gno()};
            /*
              As are updating the transaction boundary parser while reading
              GTIDs from relay log files to fill the Retrieved_Gtid_Set, we
              should not add the GTID here as we don't know if the transaction
              is complete on the relay log yet.
            */
            partial_trx->start(gtid, original_commit_timestamp,
                               immediate_commit_timestamp);
          }
          DBUG_PRINT("info",
                     ("Found Gtid in relaylog file '%s': Gtid(%d, %lld).",
                      filename, sidno, gtid_ev->get_gno()));
        }
        break;
      }
      case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      default:
        /*
          If we reached the end of a transaction after storing it's GTID
          in partial_trx structure, it is time to add this GTID to the
          retrieved_gtids set because the transaction is complete and there is
          no need for asking this transaction again.
        */
        if (trx_parser->is_not_inside_transaction()) {
          if (partial_trx->is_processing_trx_set()) {
            const Gtid *fully_retrieved_gtid;
            fully_retrieved_gtid = partial_trx->get_processing_trx_gtid();
            DBUG_PRINT("info", ("Adding Gtid to Retrieved_Gtid_Set as the "
                                "transaction was completed at "
                                "relaylog file '%s': Gtid(%d, %lld).",
                                filename, fully_retrieved_gtid->sidno,
                                fully_retrieved_gtid->gno));
            retrieved_gtids->_add_gtid(*fully_retrieved_gtid);
            /*
             We don't need to update the last queued structure here. We just
             want to have the information about the partial transaction left in
             the relay log.
            */
            partial_trx->clear();
          }
        }
        break;
    }
    delete ev;
  }

  if (relaylog_file_reader.has_fatal_error()) {
    // This is not a fatal error; the log may just be truncated.
    // @todo but what other errors could happen? IO error?
    LogErr(WARNING_LEVEL, ER_BINLOG_ERROR_READING_GTIDS_FROM_RELAY_LOG, -1);
  }

#ifndef DBUG_OFF
  LogErr(INFORMATION_LEVEL, ER_BINLOG_EVENTS_READ_FROM_RELAY_LOG_INFO,
         event_counter, filename);
#endif

  return error;
}

enum enum_read_gtids_from_binlog_status {
  GOT_GTIDS,
  GOT_PREVIOUS_GTIDS,
  NO_GTIDS,
  ERROR,
  TRUNCATED
};
/**
  Reads GTIDs from the given binlog file.

  @param filename File to read from.
  @param all_gtids If not NULL, then the GTIDs from the
  Previous_gtids_log_event and from all Gtid_log_events are stored in
  this object.
  @param prev_gtids If not NULL, then the GTIDs from the
  Previous_gtids_log_events are stored in this object.
  @param first_gtid If not NULL, then the first GTID information from the
  file will be stored in this object.
  @param sid_map The sid_map object to use in the rpl_sidno generation
  of the Gtid_log_event. If lock is needed in the sid_map, the caller
  must hold it.
  @param verify_checksum Set to true to verify event checksums.
  @param is_relay_log
  @param max_pos Read the binlog file upto max_pos offset.
  @param max_prev_hlc max hlc in all previous binlogs (out param)

  @retval GOT_GTIDS The file was successfully read and it contains
  both Gtid_log_events and Previous_gtids_log_events.
  This is only possible if either all_gtids or first_gtid are not null.
  @retval GOT_PREVIOUS_GTIDS The file was successfully read and it
  contains Previous_gtids_log_events but no Gtid_log_events.
  For binary logs, if no all_gtids and no first_gtid are specified,
  this function will be done right after reading the PREVIOUS_GTIDS
  regardless of the rest of the content of the binary log file.
  @retval NO_GTIDS The file was successfully read and it does not
  contain GTID events.
  @retval ERROR Out of memory, or IO error, or malformed event
  structure, or the file is malformed (e.g., contains Gtid_log_events
  but no Previous_gtids_log_event).
  @retval TRUNCATED The file was truncated before the end of the
  first Previous_gtids_log_event.
*/
MYSQL_BIN_LOG::enum_read_gtids_from_binlog_status
MYSQL_BIN_LOG::read_gtids_from_binlog(const char *filename, Gtid_set *all_gtids,
                                      Gtid_set *prev_gtids, Gtid *first_gtid,
                                      Sid_map *sid_map, bool verify_checksum,
                                      bool is_relay_log, my_off_t max_pos,
                                      uint64_t *max_prev_hlc) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("Opening file %s", filename));

#ifndef DBUG_OFF
  unsigned long event_counter = 0;
  /*
    We assert here that both all_gtids and prev_gtids, if specified,
    uses the same sid_map as the one passed as a parameter. This is just
    to ensure that, if the sid_map needed some lock and was locked by
    the caller, the lock applies to all the GTID sets this function is
    dealing with.
  */
  if (all_gtids) DBUG_ASSERT(all_gtids->get_sid_map() == sid_map);
  if (prev_gtids) DBUG_ASSERT(prev_gtids->get_sid_map() == sid_map);
#endif

  Binlog_file_reader binlog_file_reader(verify_checksum);
  if (binlog_file_reader.open(filename)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FILE_OPEN_FAILED,
           binlog_file_reader.get_error_str());
    /*
      We need to revisit the recovery procedure for relay log
      files. Currently, it is called after this routine.
      /Alfranio
    */
    if (binlog_file_reader.get_error_type() ==
        Binlog_read_error::CANNOT_GET_FILE_PASSWORD)
      return ERROR;
    return TRUNCATED;
  }

  Log_event *ev = nullptr;
  enum_read_gtids_from_binlog_status ret = NO_GTIDS;
  bool done = false;
  bool seen_first_gtid = false;
  uint64_t prev_hlc = 0;
  while (!done && (ev = binlog_file_reader.read_event_object()) != nullptr) {
#ifndef DBUG_OFF
    event_counter++;
#endif
    if (ev->common_header->log_pos > max_pos) {
      delete ev;
      break;
    }
    DBUG_PRINT("info", ("Read event of type %s", ev->get_type_str()));
    switch (ev->get_type_code()) {
      case binary_log::FORMAT_DESCRIPTION_EVENT:
      case binary_log::ROTATE_EVENT:
        // do nothing; just accept this event and go to next
        break;
      case binary_log::PREVIOUS_GTIDS_LOG_EVENT: {
        ret = GOT_PREVIOUS_GTIDS;
        // add events to sets
        Previous_gtids_log_event *prev_gtids_ev =
            (Previous_gtids_log_event *)ev;
        if (all_gtids != nullptr && prev_gtids_ev->add_to_set(all_gtids) != 0)
          ret = ERROR, done = true;
        else if (prev_gtids != nullptr &&
                 prev_gtids_ev->add_to_set(prev_gtids) != 0)
          ret = ERROR, done = true;
#ifndef DBUG_OFF
        char *prev_buffer = prev_gtids_ev->get_str(nullptr, nullptr);
        DBUG_PRINT("info", ("Got Previous_gtids from file '%s': Gtid_set='%s'.",
                            filename, prev_buffer));
        my_free(prev_buffer);
#endif
        /*
          If this is not a relay log, the previous_gtids were asked and no
          all_gtids neither first_gtid were asked, it is fine to consider the
          job as done.
        */
        if (!is_relay_log && prev_gtids != nullptr && all_gtids == nullptr &&
            first_gtid == nullptr)
          done = true;
        DBUG_EXECUTE_IF("inject_fault_bug16502579", {
          DBUG_PRINT("debug", ("PREVIOUS_GTIDS_LOG_EVENT found. "
                               "Injected ret=NO_GTIDS."));
          if (ret == GOT_PREVIOUS_GTIDS) {
            ret = NO_GTIDS;
            done = false;
          }
        });
        break;
      }
      case binary_log::GTID_LOG_EVENT: {
        if (ret != GOT_GTIDS) {
          if (ret != GOT_PREVIOUS_GTIDS) {
            /*
              Since this routine is run on startup, there may not be a
              THD instance. Therefore, ER(X) cannot be used.
             */
            const char *msg_fmt =
                (current_thd != nullptr)
                    ? ER_THD(current_thd, ER_BINLOG_LOGICAL_CORRUPTION)
                    : ER_DEFAULT(ER_BINLOG_LOGICAL_CORRUPTION);
            my_printf_error(
                ER_BINLOG_LOGICAL_CORRUPTION, msg_fmt, MYF(0), filename,
                "The first global transaction identifier was read, but "
                "no other information regarding identifiers existing "
                "on the previous log files was found.");
            ret = ERROR, done = true;
            break;
          } else
            ret = GOT_GTIDS;
        }
        /*
          When this is a relaylog, we just check if the relay log contains at
          least one Gtid_log_event, so that we can distinguish the return values
          GOT_GTID and GOT_PREVIOUS_GTIDS. We don't need to read anything else
          from the relay log.
          When this is a binary log, if all_gtids is requested (i.e., NOT NULL),
          we should continue to read all gtids. If just first_gtid was
          requested, we will be done after storing this Gtid_log_event info on
          it.
        */
        if (is_relay_log) {
          ret = GOT_GTIDS, done = true;
        } else {
          Gtid_log_event *gtid_ev = (Gtid_log_event *)ev;
          rpl_sidno sidno = gtid_ev->get_sidno(sid_map);
          if (sidno < 0)
            ret = ERROR, done = true;
          else {
            if (all_gtids) {
              if (all_gtids->ensure_sidno(sidno) != RETURN_STATUS_OK)
                ret = ERROR, done = true;
              all_gtids->_add_gtid(sidno, gtid_ev->get_gno());
              DBUG_PRINT("info", ("Got Gtid from file '%s': Gtid(%d, %lld).",
                                  filename, sidno, gtid_ev->get_gno()));
            }

            /* If the first GTID was requested, stores it */
            if (first_gtid && !seen_first_gtid) {
              first_gtid->set(sidno, gtid_ev->get_gno());
              seen_first_gtid = true;
              /* If the first_gtid was the only thing requested, we are done */
              if (all_gtids == nullptr) ret = GOT_GTIDS, done = true;
            }
          }
        }
        break;
      }
      case binary_log::METADATA_EVENT: {
        if (unlikely(max_prev_hlc)) {
          prev_hlc = std::max(
              prev_hlc, extract_hlc(static_cast<Metadata_log_event *>(ev)));
        }
        break;
      }
      case binary_log::ANONYMOUS_GTID_LOG_EVENT: {
        /*
          When this is a relaylog, we just check if it contains
          at least one Anonymous_gtid_log_event after initialization
          (FDs, Rotates and PREVIOUS_GTIDS), so that we can distinguish the
          return values GOT_GTID and GOT_PREVIOUS_GTIDS.
          We don't need to read anything else from the relay log.
        */
        if (is_relay_log) {
          ret = GOT_GTIDS;
          done = true;
          break;
        }
        DBUG_ASSERT(prev_gtids == nullptr
                        ? true
                        : all_gtids != nullptr || first_gtid != nullptr);
      }
      // Fall through.
      default:
        // if we found any other event type without finding a
        // previous_gtids_log_event, then the rest of this binlog
        // cannot contain gtids
        if (ret != GOT_GTIDS && ret != GOT_PREVIOUS_GTIDS) done = true;
        /*
          The GTIDs of the relaylog files will be handled later
          because of the possibility of transactions be spanned
          along distinct relaylog files.
          So, if we found an ordinary event without finding the
          GTID but we already found the PREVIOUS_GTIDS, this probably
          means that the event is from a transaction that started on
          previous relaylog file.
        */
        if (ret == GOT_PREVIOUS_GTIDS && is_relay_log) done = true;
        break;
    }
    delete ev;
    DBUG_PRINT("info", ("done=%d", done));
  }

  if (binlog_file_reader.has_fatal_error()) {
    // This is not a fatal error; the log may just be truncated.

    // @todo but what other errors could happen? IO error?
    LogErr(WARNING_LEVEL, ER_BINLOG_ERROR_READING_GTIDS_FROM_BINARY_LOG, -1);
  }

  // Set max HLC timestamp in all previous binlogs
  if (unlikely(max_prev_hlc)) {
    *max_prev_hlc = prev_hlc;
  }

  if (all_gtids)
    all_gtids->dbug_print("all_gtids");
  else
    DBUG_PRINT("info", ("all_gtids==NULL"));
  if (prev_gtids)
    prev_gtids->dbug_print("prev_gtids");
  else
    DBUG_PRINT("info", ("prev_gtids==NULL"));
  if (first_gtid == nullptr)
    DBUG_PRINT("info", ("first_gtid==NULL"));
  else if (first_gtid->sidno == 0)
    DBUG_PRINT("info", ("first_gtid.sidno==0"));
  else
    first_gtid->dbug_print(sid_map, "first_gtid");

  DBUG_PRINT("info", ("returning %d", ret));
#ifndef DBUG_OFF
  if (!is_relay_log && prev_gtids != nullptr && all_gtids == nullptr &&
      first_gtid == nullptr)
    LogErr(INFORMATION_LEVEL, ER_BINLOG_EVENTS_READ_FROM_BINLOG_INFO,
           event_counter, filename);
#endif
  return ret;
}

uint64_t MYSQL_BIN_LOG::extract_hlc(Metadata_log_event *metadata_ev) {
  return std::max(metadata_ev->get_hlc_time(),
                  metadata_ev->get_prev_hlc_time());
}

bool MYSQL_BIN_LOG::find_first_log_not_in_gtid_set(char *binlog_file_name,
                                                   const Gtid_set *gtid_set,
                                                   Gtid *first_gtid,
                                                   const char **errmsg) {
  DBUG_TRACE;

  Gtid_set binlog_previous_gtid_set(gtid_set->get_sid_map());

  mysql_mutex_lock(&LOCK_index);
  for (auto rit = previous_gtid_set_map.rbegin();
       rit != previous_gtid_set_map.rend(); ++rit) {
    binlog_previous_gtid_set.clear();
    binlog_previous_gtid_set.add_gtid_encoding(
        (const uchar *)rit->second.c_str(), rit->second.length());

    if (binlog_previous_gtid_set.is_subset(gtid_set)) {
      char full_log_name[FN_REFLEN];
      strcpy(binlog_file_name, rit->first.c_str());
      full_log_name[0] = 0;
      if (normalize_binlog_name(full_log_name, binlog_file_name,
                                is_relay_log)) {
        *errmsg = "Error normalizing log file name";
        mysql_mutex_unlock(&LOCK_index);
        return true;
      }
      enum_read_gtids_from_binlog_status ret = read_gtids_from_binlog(
          full_log_name, NULL, NULL, first_gtid, gtid_set->get_sid_map(),
          opt_master_verify_checksum, is_relay_log);
      // some rpl tests injects the error skip_writing_previous_gtids_log_event
      // intentionally. Don't pass this error to dump thread which causes
      // slave io_thread error failing the tests.
      if (ret != GOT_GTIDS && ret != GOT_PREVIOUS_GTIDS &&
          DBUG_EVALUATE_IF("skip_writing_previous_gtids_log_event", 0, 1)) {
        *errmsg = "Error finding first GTID in the log file";
        mysql_mutex_unlock(&LOCK_index);
        return true;
      }
      mysql_mutex_unlock(&LOCK_index);
      return false;
    }
  }

  char *missing_gtids = NULL;
  Gtid_set gtid_missing(gtid_set->get_sid_map());
  gtid_missing.add_gtid_set(gtid_set);
  gtid_missing.remove_gtid_set(&binlog_previous_gtid_set);
  gtid_missing.to_string(&missing_gtids, false, NULL);

  String tmp_uuid;
  mysql_mutex_lock(&current_thd->LOCK_thd_data);
  const auto it = current_thd->user_vars.find("slave_uuid");
  if (it != current_thd->user_vars.end() && it->second->length() > 0) {
    tmp_uuid.copy(it->second->ptr(), it->second->length(), NULL);
  }
  mysql_mutex_unlock(&current_thd->LOCK_thd_data);
  LogErr(WARNING_LEVEL, ER_FOUND_MISSING_GTIDS, tmp_uuid.ptr(), missing_gtids);
  my_free(missing_gtids);

  *errmsg = ER_THD(current_thd, ER_MASTER_HAS_PURGED_REQUIRED_GTIDS);
  DBUG_PRINT("error", ("'%s'", *errmsg));
  mysql_mutex_unlock(&LOCK_index);
  return true;
}

/*
 * This is a limited version of init_gtid_sets which is only
 * called from binlog_change_to_apply.
 * Needs to be called LOCK_index held.
 * The previous_gtid_set_map is cleared and reinitialized from
 * the index file contents.
 */
bool MYSQL_BIN_LOG::init_prev_gtid_sets_map() {
  char file_name_and_gtid_set_length[FILE_AND_GTID_SET_LENGTH];
  uchar *previous_gtid_set_in_file = NULL;
  int error = 0, length;
  std::pair<Gtid_set_map::iterator, bool> iterator;
  DBUG_ENTER("MYSQL_BIN_LOG::init_prev_gtid_sets_map");

  mysql_mutex_assert_owner(&LOCK_index);

  // clear the map as it is being reset
  previous_gtid_set_map.clear();

  my_b_seek(&index_file, 0);
  while ((length = my_b_gets(&index_file, file_name_and_gtid_set_length,
                             sizeof(file_name_and_gtid_set_length))) >= 1) {
    file_name_and_gtid_set_length[length - 1] = 0;
    uint gtid_string_length =
        split_file_name_and_gtid_set_length(file_name_and_gtid_set_length);
    if (gtid_string_length > 0) {
      // Allocate gtid_string_length + 1 to include the '\n' also.
      previous_gtid_set_in_file = reinterpret_cast<uchar *>(
          my_malloc(PSI_NOT_INSTRUMENTED, gtid_string_length + 1, MYF(MY_WME)));
      if (previous_gtid_set_in_file == NULL) {
        // NO_LINT_DEBUG
        sql_print_error(
            "MYSQL_BIN_LOG::init_prev_gtid_sets_map failed allocating "
            "%u bytes",
            gtid_string_length + 1);
        error = 2;
        goto end;
      }
      if (my_b_read(&index_file, previous_gtid_set_in_file,
                    gtid_string_length + 1)) {
        // NO_LINT_DEBUG
        sql_print_error(
            "MYSQL_BINLOG::init_prev_gtid_sets_map failed because "
            "previous gtid set of binlog %s is corrupted in "
            "the index file",
            file_name_and_gtid_set_length);
        error = !index_file.error ? LOG_INFO_EOF : LOG_INFO_IO;
        my_free(previous_gtid_set_in_file);
        goto end;
      }
    }

    iterator = previous_gtid_set_map.insert(std::pair<string, string>(
        string(file_name_and_gtid_set_length),
        string(reinterpret_cast<char *>(previous_gtid_set_in_file),
               gtid_string_length)));

    my_free(previous_gtid_set_in_file);
    previous_gtid_set_in_file = NULL;
  }

end:
  update_lost_gtid_for_tailing();
  DBUG_PRINT("info", ("returning %d", error));
  DBUG_RETURN(error != 0 ? true : false);
}

bool MYSQL_BIN_LOG::init_gtid_sets(Gtid_set *all_gtids, Gtid_set *lost_gtids,
                                   bool verify_checksum, bool need_lock,
                                   Transaction_boundary_parser *trx_parser,
                                   Gtid_monitoring_info *partial_trx,
                                   uint64_t *max_prev_hlc, bool startup) {
  char file_name_and_gtid_set_length[FILE_AND_GTID_SET_LENGTH];
  uchar *previous_gtid_set_in_file = NULL;
  bool found_lost_gtids = false;
  uint last_previous_gtid_encoded_length = 0;
  int error = 0, length;
  std::pair<Gtid_set_map::iterator, bool> iterator, save_iterator;
  char full_log_name[FN_REFLEN];
  std::string log_file_to_read;

  full_log_name[0] = 0;
  log_file_to_read.clear();

  /* Initialize the sid_map to be used in read_gtids_from_binlog */
  Sid_map *sid_map = NULL;
  if (all_gtids)
    sid_map = all_gtids->get_sid_map();
  else if (lost_gtids)
    sid_map = lost_gtids->get_sid_map();

  DBUG_TRACE;
  DBUG_PRINT(
      "info",
      ("lost_gtids=%p; so we are recovering a %s log; is_relay_log=%d",
       lost_gtids, lost_gtids == nullptr ? "relay" : "binary", is_relay_log));

  Checkable_rwlock *sid_lock =
      is_relay_log ? all_gtids->get_sid_map()->get_sid_lock() : global_sid_lock;
  /*
    If this is a relay log, we must have the IO thread Master_info trx_parser
    in order to correctly feed it with relay log events.
  */
#ifndef DBUG_OFF
  if (is_relay_log) {
    DBUG_ASSERT(trx_parser != nullptr);
    DBUG_ASSERT(lost_gtids == nullptr);
  }
#endif

  /*
    Acquires the necessary locks to ensure that logs are not either
    removed or updated when we are reading from it.
  */
  if (need_lock) {
    // We don't need LOCK_log if we are only going to read the initial
    // Prevoius_gtids_log_event and ignore the Gtid_log_events.
    if (all_gtids != nullptr) mysql_mutex_lock(&LOCK_log);
    mysql_mutex_lock(&LOCK_index);
    sid_lock->wrlock();
  } else {
    if (all_gtids != nullptr) mysql_mutex_assert_owner(&LOCK_log);
    mysql_mutex_assert_owner(&LOCK_index);
    sid_lock->assert_some_wrlock();
  }

  previous_gtid_set_map.clear();

  // Gather the set of files to be accessed for relay log.
  list<string> filename_list;
  int num_prev_gtids_found = 0;

  my_b_seek(&index_file, 0);
  while ((length = my_b_gets(&index_file, file_name_and_gtid_set_length,
                             sizeof(file_name_and_gtid_set_length))) >= 1) {
    file_name_and_gtid_set_length[length - 1] = 0;
    uint gtid_string_length =
        split_file_name_and_gtid_set_length(file_name_and_gtid_set_length);
    if (gtid_string_length > 0) {
      // Allocate gtid_string_length + 1 to include the '\n' also.
      previous_gtid_set_in_file =
          (uchar *)my_malloc(key_memory_binlog_previous_gtid_set,
                             gtid_string_length + 1, MYF(MY_WME));
      if (previous_gtid_set_in_file == NULL) {
        LogErr(ERROR_LEVEL, ER_BINLOG_INDEX_PREV_GTID_OUTOFMEMORY,
               gtid_string_length + 1);
        error = 1;
        goto end;
      }
      if (my_b_read(&index_file, previous_gtid_set_in_file,
                    gtid_string_length + 1)) {
        LogErr(ERROR_LEVEL, ER_BINLOG_INDEX_PREV_GTID_CORRUPT,
               file_name_and_gtid_set_length);
        error = !index_file.error ? LOG_INFO_EOF : LOG_INFO_IO;
        my_free(previous_gtid_set_in_file);
        goto end;
      }

      if (lost_gtids != NULL && !found_lost_gtids) {
        DBUG_PRINT("info", ("first binlog with gtids %s\n",
                            file_name_and_gtid_set_length));

#ifndef DBUG_OFF
        Gtid_set unused_lost_gtid(lost_gtids->get_sid_map(),
                                  lost_gtids->get_sid_map()->get_sid_lock());

        /*
          In the original implementation:

          Mysql server iterates forwards through binary logs, looking for the
          first binary log that contains both Previous_gtids_log_event and
          gtid_log_event for gathering the set of gtid_purged on server start.
          It also iterates forwards through binary logs, looking for the first
          binary log that contains both Previous_gtids_log_event and
          gtid_log_event for gathering the set of gtid_purged when purging
          binary logs. This may take very long time if it has many binary logs
          and almost all of them are out of filesystem cache. So if the
          binlog_gtid_simple_recovery is enabled, we just initialize
          GLOBAL.GTID_PURGED from the first binary log, do not read any more
          binary logs.

          In ours:

          Reading the binlog for gtids is needed for the test
          binlog.binlog_purge_binary_logs_stall. Our implementation also
          prevents the large binlog read stall in MySQL since we never
          need to read the MySQL binlog directly.
        */
        if (normalize_binlog_name(full_log_name, file_name_and_gtid_set_length,
                                  is_relay_log) ||
            read_gtids_from_binlog(full_log_name, NULL, &unused_lost_gtid, NULL,
                                   sid_map, verify_checksum, is_relay_log,
                                   ULLONG_MAX, max_prev_hlc) == ERROR) {
          error = 1;
          goto end;
        }
#endif /* DBUG_OFF */

        lost_gtids->add_gtid_encoding(previous_gtid_set_in_file,
                                      gtid_string_length);
        found_lost_gtids = true;
      }
      num_prev_gtids_found++;
    } else {
      /*
        TODO -- need to support old filename entries where
        the prev_gtid set is not stored in the index file.
        This is likely required before Oracle will accept
        this feature
      */
    }
    iterator = previous_gtid_set_map.insert(std::pair<string, string>(
        string(file_name_and_gtid_set_length),
        string((char *)previous_gtid_set_in_file, gtid_string_length)));

    if (enable_raft_plugin) {
      if (log_file_to_read.empty()) {
        const std::string cur_log_file =
            file_name_and_gtid_set_length +
            dirname_length(file_name_and_gtid_set_length);

        if (strcmp(cur_log_file.c_str(), this->engine_binlog_file) == 0)
          log_file_to_read.assign(file_name_and_gtid_set_length);
      } else if (startup) {
        // NO_LINT_DEBUG
        sql_print_warning(
            "Engine has seen trxs till file %s, but found "
            "additional file %s in the index file",
            log_file_to_read.c_str(), file_name_and_gtid_set_length);
      }
    }

    /* filename_list is only used for relay_log checking later */
    if (is_relay_log)
      filename_list.push_back(string(file_name_and_gtid_set_length));

    if (all_gtids != NULL && gtid_string_length > 0) {
      last_previous_gtid_encoded_length = gtid_string_length;
      save_iterator = iterator;
    }
    my_free(previous_gtid_set_in_file);
    previous_gtid_set_in_file = NULL;
  }

  if (all_gtids != NULL && last_previous_gtid_encoded_length) {
    const char *last_binlog_file_with_gtids =
        save_iterator.first->first.c_str();
    DBUG_PRINT("info",
               ("last binlog with gtids %s\n", last_binlog_file_with_gtids));
    if (normalize_binlog_name(full_log_name, last_binlog_file_with_gtids,
                              is_relay_log)) {
      error = 1;
      goto end;
    }

    my_off_t max_pos = ULLONG_MAX;

    // In raft mode, when the server is starting up, we update gtid_executed
    // only till the file position that is stored in the engine. This is because
    // anything that is not committed in the engine could get trimmed when this
    // instance rejoins the raft ring
    if (enable_raft_plugin && startup) {
      if (log_file_to_read.empty()) {
        // NO_LINT_DEBUG
        sql_print_information(
            "Could not get the transaction log file name from the engine. "
            "Using the latest for initializing mysqld state");

        // In innodb, engine_binlog_file is populated _only_ when there are
        // trxs to recover (i.e trxs are in prepared state) during startup
        // and engine recovery. Hence, engine_binlog_file being empty indicates
        // that engine's state is up-to-date with the latest binlog file and we
        // can include all gtids in the latest binlog file for calculating
        // executed-gtid-set
        if (strlen(engine_binlog_file) == 0)
          max_pos = ULLONG_MAX;
        else
          max_pos = this->first_gtid_start_pos;
      } else {
        // Initializing gtid_sets based on engine binlog position is fine since
        // idempotent recovery will fill in the holes
        max_pos = this->engine_binlog_pos;
      }

      // NO_LINT_DEBUG
      sql_print_information(
          "Reading all gtids till file position %llu "
          "in file %s",
          max_pos, log_file_to_read.c_str());
    } else {
      log_file_to_read.assign(last_binlog_file_with_gtids);
      max_pos = ULLONG_MAX;
    }
    if (read_gtids_from_binlog(full_log_name, all_gtids, NULL, NULL, sid_map,
                               verify_checksum, is_relay_log, max_pos,
                               max_prev_hlc) == ERROR) {
      error = 1;
      goto end;
    }
    /*
      Even though the previous gtid encoding is not null in index file, it may
      happen that the binlog is corrupted and doesn't contain previous gtid log
      event. In these cases, the encoding in the index file is considered as
      true and used to initialize the all_gtids(GLOBAL.GTID_EXECUTED).
    */
    if (all_gtids->is_empty()) {
      all_gtids->add_gtid_encoding(
          (const uchar *)save_iterator.first->second.c_str(),
          last_previous_gtid_encoded_length);
    }
  }

  /*
    If we use GTIDs and have partial transactions on the relay log,
    must check if it ends on next relay log files.
    We also need to feed the boundary parser with the rest of the
    relay log to put it in the correct state before receiving new
    events from the master in the case of GTID auto positioning be
    disabled.

    If no previous gtids were found, then skip this.
  */
  if (is_relay_log && filename_list.size() > 0 && num_prev_gtids_found) {
    /*
      Suppose the following relaylog:

       rl-bin.000001 | rl-bin.000002 | rl-bin.000003 | rl-bin-000004
      ---------------+---------------+---------------+---------------
       PREV_GTIDS    | PREV_GTIDS    | PREV_GTIDS    | PREV_GTIDS
       (empty)       | (UUID:1)      | (UUID:1)      | (UUID:1)
      ---------------+---------------+---------------+---------------
       GTID(UUID:1)  | QUERY(INSERT) | QUERY(INSERT) | XID
      ---------------+---------------+---------------+---------------
       QUERY(CREATE  |
       TABLE t1 ...) |
      ---------------+
       GTID(UUID:2)  |
      ---------------+
       QUERY(BEGIN)  |
      ---------------+

      As it is impossible to determine the current Retrieved_Gtid_Set by only
      looking to the PREVIOUS_GTIDS on the last relay log file, and scanning
      events on it, we tried to find a relay log file that contains at least
      one GTID event during the backwards search.

      In the example, we will find a GTID only in rl-bin.000001, as the
      UUID:2 transaction was spanned across 4 relay log files.

      The transaction spanning can be caused by "FLUSH RELAY LOGS" commands
      on slave while it is queuing the transaction.

      So, in order to correctly add UUID:2 into Retrieved_Gtid_Set, we need
      to parse the relay log starting on the file we found the last GTID
      queued to know if the transaction was fully retrieved or not.
    */

    // Iterate over all files in reverse order.
    // Find the first log file where there is a difference in PREV_GTIDS.
    // This should be the log file that contains the last GTID_EVENT.
    list<string>::reverse_iterator rit;
    std::map<std::string, std::string>::iterator pg_it;

    rit = filename_list.rbegin();
    pg_it = previous_gtid_set_map.find(*rit);
    if (pg_it != previous_gtid_set_map.end()) {
      /*
        Record the prev gtid found. This should be guaranteed to be non-empty
        since gtids being enabled means previous gtid is always populated.
        Only in the case of gtids never being enabled should previous
        gtids not exist, but then the num_prev_gtids_found should be 0.
      */
      std::string last_file_prev_gtid = pg_it->second;
      DBUG_ASSERT(last_file_prev_gtid.length() != 0);

      rit++;
      while (rit != filename_list.rend()) {
        pg_it = previous_gtid_set_map.find(*rit);
        if (pg_it == previous_gtid_set_map.end() ||
            last_file_prev_gtid.length() != pg_it->second.length() ||
            memcmp(last_file_prev_gtid.c_str(), pg_it->second.c_str(),
                   last_file_prev_gtid.length())) {
          /* Found file with prev gtid different from the last file */
          break;
        }
        rit++;
      }

      /*
        Adjust the reverse iterator to point to the relaylog file we
        need to start parsing, as it was incremented after generating
        the relay log file name.
      */
      DBUG_ASSERT(rit != filename_list.rbegin());
      if (rit == filename_list.rend()) rit--;
      /* Reset the transaction parser before feeding it with events */
      trx_parser->reset();
      partial_trx->clear();

      DBUG_PRINT("info", ("Iterating forwards through relay logs, "
                          "updating the Retrieved_Gtid_Set and updating "
                          "IO thread trx parser before start."));
      list<string>::iterator it;
      for (it = find(filename_list.begin(), filename_list.end(), *rit);
           it != filename_list.end(); it++) {
        const char *filename = it->c_str();
        DBUG_PRINT("info", ("filename='%s'", filename));
        if (normalize_binlog_name(full_log_name, filename, is_relay_log) ||
            read_gtids_and_update_trx_parser_from_relaylog(
                full_log_name, all_gtids, true, trx_parser, partial_trx)) {
          error = 1;
          goto end;
        }
      }
    }
  }

end:
  update_lost_gtid_for_tailing();
  if (all_gtids) all_gtids->dbug_print("all_gtids");
  if (lost_gtids) lost_gtids->dbug_print("lost_gtids");
  if (need_lock) {
    sid_lock->unlock();
    mysql_mutex_unlock(&LOCK_index);
    if (all_gtids != nullptr) mysql_mutex_unlock(&LOCK_log);
  }
  filename_list.clear();
  DBUG_PRINT("info", ("returning %d", error));
  return error != 0 ? true : false;
}

/**
  Open a (new) binlog file.

  - Open the log file and the index file. Register the new
  file name in it
  - When calling this when the file is in use, you must have a locks
  on LOCK_log and LOCK_index.

  @retval
    0	ok
  @retval
    1	error
*/

bool MYSQL_BIN_LOG::open_binlog(
    const char *log_name, const char *new_name, ulong max_size_arg,
    bool null_created_arg, bool need_lock_index, bool need_sid_lock,
    Format_description_log_event *extra_description_event,
    uint32 new_index_number, RaftRotateInfo *raft_rotate_info,
    bool need_end_log_pos_lock) {
  // lock_index must be acquired *before* sid_lock.
  DBUG_ASSERT(need_sid_lock || !need_lock_index);
  DBUG_TRACE;
  DBUG_PRINT("enter", ("base filename: %s", log_name));

  uchar *previous_gtid_set_buffer = NULL;
  uint gtid_set_length = 0;

  mysql_mutex_assert_owner(get_log_lock());

  if (init_and_set_log_file_name(log_name, new_name, new_index_number)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_GENERATE_NEW_FILE_NAME);
    return true;
  }

  DBUG_PRINT("info", ("generated filename: %s", log_file_name));

  DEBUG_SYNC(current_thd, "after_log_file_name_initialized");

  if (open_purge_index_file(true) ||
      register_create_index_entry(log_file_name) || sync_purge_index_file() ||
      DBUG_EVALUATE_IF("fault_injection_registering_index", 1, 0)) {
    /**
      @todo: although this was introduced to appease valgrind
      when injecting emulated faults using fault_injection_registering_index
      it may be good to consider what actually happens when
      open_purge_index_file succeeds but register or sync fails.

      Perhaps we might need the code below in MYSQL_BIN_LOG::cleanup
      for "real life" purposes as well?
    */
    DBUG_EXECUTE_IF("fault_injection_registering_index", {
      if (my_b_inited(&purge_index_file)) {
        end_io_cache(&purge_index_file);
        my_close(purge_index_file.file, MYF(0));
      }
    });

    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_SYNC_INDEX_FILE_IN_OPEN);
    return true;
  }
  DBUG_EXECUTE_IF("crash_create_non_critical_before_update_index",
                  DBUG_SUICIDE(););

  write_error = false;

  /* open the main log file */
  if (open(m_key_file_log, log_name, new_name, new_index_number)) {
    close_purge_index_file();
    return true; /* all warnings issued */
  }

  max_size = max_size_arg;

  bool write_file_name_to_index_file = false;
  bool raft_noop = raft_rotate_info && raft_rotate_info->noop;
  /*

   * Which rotates are initiated by the plugin and happen
   * in the raft listener queue thread?
   *
   * 1. Any rotate which is a post_append (i.e. normal rotate)
   * 2. Any rotate which the plugin asks the mysql to initiate
   * via injecting an operation in the listener queue.
   * i.e. 2.1 - No-Op messages
   *      2.2 - Config Change messages.
  */
  bool rotate_in_listener_context =
      raft_rotate_info &&
      (raft_rotate_info->post_append || raft_rotate_info->noop ||
       raft_rotate_info->config_change_rotate);
  /* This must be before goto err. */
#ifndef DBUG_OFF
  binary_log_debug::debug_pretend_version_50034_in_binlog =
      DBUG_EVALUATE_IF("pretend_version_50034_in_binlog", true, false);
#endif
  Format_description_log_event s;

  if (m_binlog_file->is_empty()) {
    /*
      The binary log file was empty (probably newly created)
      This is the normal case and happens when the user doesn't specify
      an extension for the binary log files.
      In this case we write a standard header to it.
    */
    if (m_binlog_file->write(pointer_cast<const uchar *>(BINLOG_MAGIC),
                             BIN_LOG_HEADER_SIZE))
      goto err;
    bytes_written += BIN_LOG_HEADER_SIZE;
    write_file_name_to_index_file = true;
  }

  /*
    don't set LOG_EVENT_BINLOG_IN_USE_F for the relay log
  */
  if (!is_relay_log || raft_noop) {
    s.common_header->flags |= LOG_EVENT_BINLOG_IN_USE_F;
  }

  if (is_relay_log) {
    /* relay-log */
    if (relay_log_checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_UNDEF) {
      /* inherit master's A descriptor if one has been received */
      if (opt_slave_sql_verify_checksum == 0)
        /* otherwise use slave's local preference of RL events verification */
        relay_log_checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
      else
        relay_log_checksum_alg =
            static_cast<enum_binlog_checksum_alg>(binlog_checksum_options);
    }
  }

  if (!s.is_valid()) goto err;
  s.dont_set_created = null_created_arg;
  // Since start time of listener thread is not correct,
  // we need to explicitly set event timestamp otherwise,
  // the mysqlbinlog output will show UTC-0 for FD/PGTID/MD event
  if (rotate_in_listener_context) s.common_header->when.tv_sec = my_time(0);
  /* Set LOG_EVENT_RELAY_LOG_F flag for relay log's FD */
  if (is_relay_log && !raft_noop) s.set_relay_log_event();
  if (write_event_to_binlog(&s)) goto err;
  /*
    We need to revisit this code and improve it.
    See further comments in the mysqld.
    /Alfranio
  */

  /*
    Prior to including to the prev_gtid information in the index file,
    the previous gtid event is written by open_binlog for all new relay/binlogs
    created except in the case where the binary log is created during server
    startup.

    ----

    If the slave was configured before server restart, the server will
    generate a new relay log file without having current_thd, but this
    new relay log file must have a PREVIOUS_GTIDS event as we now
    generate the PREVIOUS_GTIDS event always.

    This is only needed for relay log files because the server will add
    the PREVIOUS_GTIDS of binary logs (when current_thd==NULL) after
    server's GTID initialization.

    During server's startup at mysqld_main(), from the binary/relay log
    initialization point of view, it will:
    1) Call init_server_components() that will generate a new binary log
       file but won't write the PREVIOUS_GTIDS event yet;
    2) Initialize server's GTIDs;
    3) Write the binary log PREVIOUS_GTIDS;
    4) Call init_slave() in where the new relay log file will be created
       after initializing relay log's Retrieved_Gtid_Set;

    ----

    However, since open_binlog needs to store the gtid set in the index file,
    all cases need to write out the prev_gtid information. Server start up is
    expected to 'initialize' gtid_sets within init_server_components() first so
    that previous gtids can be generated correctly.
  */
  {
    Checkable_rwlock *sid_lock = nullptr;
    Gtid_set logged_gtids_binlog(global_sid_map, global_sid_lock);
    Gtid_set *previous_logged_gtids;

    if (is_relay_log) {
      previous_logged_gtids = previous_gtid_set_relaylog;
      sid_lock = previous_gtid_set_relaylog->get_sid_map()->get_sid_lock();
    } else {
      previous_logged_gtids = &logged_gtids_binlog;
      sid_lock = global_sid_lock;
    }

    if (need_sid_lock)
      sid_lock->wrlock();
    else
      sid_lock->assert_some_wrlock();

    if (!is_relay_log) {
      const Gtid_set *executed_gtids = gtid_state->get_executed_gtids();
      /* logged_gtids_binlog= executed_gtids
         Always store full executed_gtids set into Previous_gtids_log_event to
         make binlogs self-contained
         NOTE:
         This store full gtids set behavior is different than official MySQL.
         offical mysql behavior: store delta gtids set in Previous_gtids
                                 its value: executed_gtids - gtids_only_in_table
              fb mysql behavior: store full gtids set in Previous_gtids
                                 its value: executed_gtids
         The reason for this change is that infra depends on correct
         Previous_gtids value(full GTIDS SET) and uses "set GTID_PURGED +
         FLUSH LOGS" to control Previous_gtids value.
         *When call set global GTID_PURGED="UUID:ID1-ID2", it will
            store "UUID:ID1:ID2" into lost_gtids, executed_gtids,
            gtids_only_in_table variables and also store "UUID:ID1:ID2" into
            mysql.executed_gtid table.
         *When call flush logs, it will store
            (executed_gtids - gtids_only_in_table) or (executed_gtids) as
            Previous_gtid_set.
         Example:
         1. set global GTID_PURGED="UUID:ID1-ID2";flush logs;
           executed_gtids ==  UUID:ID1-ID2
           executed_gtids - gtids_only_in_table ==  <empty>
         2. set global GTID_PURGED="UUID:ID1-ID2"; sync another trans;flush logs
           executed_gtids ==  UUID:ID1-ID3
           executed_gtids - gtids_only_in_table == UUID:ID3
         3. set global GTID_PURGED="UUID:ID1-ID2"; restart slave;flush logs
           executed_gtids ==  UUID:ID1-ID2
           executed_gtids - gtids_only_in_table ==  <empty>
         For all of these above example, FB MySQL will store executed_gtids as
         Previous_gtid_set.
      */
      if (logged_gtids_binlog.add_gtid_set(executed_gtids) !=
          RETURN_STATUS_OK) {
        if (need_sid_lock) sid_lock->unlock();
        goto err;
      }
    }

    /* Generate the encoded gtid set for storing in the index file */
    previous_gtid_set_buffer = previous_logged_gtids->encode(&gtid_set_length);

    DBUG_PRINT("info", ("Generating PREVIOUS_GTIDS for %s file.",
                        is_relay_log ? "relaylog" : "binlog"));
    Previous_gtids_log_event prev_gtids_ev(previous_logged_gtids);

    if (rotate_in_listener_context)
      prev_gtids_ev.common_header->when.tv_sec = my_time(0);
    // TODO(pgl) : confirm if raft_noop check required here.
    if (!raft_noop && is_relay_log) prev_gtids_ev.set_relay_log_event();
    if (need_sid_lock) sid_lock->unlock();
    if (write_event_to_binlog(&prev_gtids_ev)) goto err;

    /* If HLC is enabled, then write the current HLC value into binlog. This is
     * used during server restart to intialize the HLC clock of the instance.
     * This is a guarantee that all trx's in all of the previous binlog have a
     * HLC timestamp lower than or equal to the value seen here. Note that this
     * function (open_binlog()) should be called during server restart only
     * after initializing the local instance's HLC clock (by reading the
     * previous binlog file) */
    if (enable_binlog_hlc && (!is_relay_log || raft_rotate_info)) {
      uint64_t current_hlc = 0;
      if (!is_relay_log) {
        current_hlc = mysql_bin_log.get_current_hlc();
      }
      Metadata_log_event metadata_ev(current_hlc);
      if (rotate_in_listener_context)
        metadata_ev.common_header->when.tv_sec = my_time(0);
      if (raft_rotate_info) {
        metadata_ev.set_raft_prev_opid(raft_rotate_info->rotate_opid.first,
                                       raft_rotate_info->rotate_opid.second);
      }
      if (write_event_to_binlog(&metadata_ev)) goto err;
    }
  }

  // in raft, it isn't necessary to write these extra description event
  // in raft, if these extra description event is required, need to write them
  // together with descrption event
  if (!enable_raft_plugin && extra_description_event) {
    /*
      This is a relay log written to by the I/O slave thread.
      Write the event so that others can later know the format of this relay
      log.
      Note that this event is very close to the original event from the
      master (it has binlog version of the master, event types of the
      master), so this is suitable to parse the next relay log's event. It
      has been produced by
      Format_description_log_event::Format_description_log_event(char* buf,).
      Why don't we want to write the mi_description_event if this
      event is for format<4 (3.23 or 4.x): this is because in that case, the
      mi_description_event describes the data received from the
      master, but not the data written to the relay log (*conversion*),
      which is in format 4 (slave's).
    */
    /*
      Set 'created' to 0, so that in next relay logs this event does not
      trigger cleaning actions on the slave in
      Format_description_log_event::apply_event_impl().
    */
    extra_description_event->created = 0;
    /* Don't set log_pos in event header */
    extra_description_event->set_artificial_event();

    if (binary_event_serialize(extra_description_event, m_binlog_file))
      goto err;
    bytes_written += extra_description_event->common_header->data_written;
  }
  if (m_binlog_file->flush_and_sync()) goto err;

  if (write_file_name_to_index_file) {
    DBUG_EXECUTE_IF("crash_create_critical_before_update_index",
                    DBUG_SUICIDE(););
    DBUG_ASSERT(my_b_inited(&index_file) != 0);

    /*
      The new log file name is appended into crash safe index file after
      all the content of index file is copyed into the crash safe index
      file. Then move the crash safe index file to index file.
    */
    DBUG_EXECUTE_IF("simulate_disk_full_on_open_binlog",
                    { DBUG_SET("+d,simulate_no_free_space_error"); });
    if (DBUG_EVALUATE_IF("fault_injection_updating_index", 1, 0) ||
        add_log_to_index((uchar *)log_file_name, strlen(log_file_name),
                         need_lock_index, previous_gtid_set_buffer,
                         gtid_set_length)) {
      DBUG_EXECUTE_IF("simulate_disk_full_on_open_binlog", {
        DBUG_SET("-d,simulate_file_write_error");
        DBUG_SET("-d,simulate_no_free_space_error");
        DBUG_SET("-d,simulate_disk_full_on_open_binlog");
      });
      goto err;
    }

    DBUG_EXECUTE_IF("crash_create_after_update_index", DBUG_SUICIDE(););
  }

  atomic_log_state = LOG_OPENED;
  /*
    At every rotate memorize the last transaction counter state to use it as
    offset at logging the transaction logical timestamps.
  */
  m_dependency_tracker.rotate();

  close_purge_index_file();

  update_binlog_end_pos(need_end_log_pos_lock);
  my_free(previous_gtid_set_buffer);
  return false;

err:
  if (is_inited_purge_index_file())
    purge_index_entry(nullptr, nullptr, need_lock_index);
  close_purge_index_file();
  if (should_abort_on_binlog_error()) {
    exec_binlog_error_action_abort(
        "Either disk is full, file system is read only or "
        "there was an encryption error while opening the binlog. "
        "Aborting the server.");
  } else {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_USE_FOR_LOGGING,
           (new_name) ? new_name : name, errno);
    close(LOG_CLOSE_INDEX, false, need_lock_index);
  }
  my_free(previous_gtid_set_buffer);
  return true;
}

bool MYSQL_BIN_LOG::open_existing_binlog(const char *log_name,
                                         ulong max_size_arg,
                                         bool need_end_log_pos_lock) {
  DBUG_ENTER("MYSQL_BIN_LOG::open_existing_binlog(const char *, ...)");
  DBUG_PRINT("enter", ("name: %s", log_name));

  my_off_t offset = 0;
  File file = -1;

  // This sets the cur_log_ext telling the plugin that
  // RLI initialization has happened.
  // i.e. cur_log_ext != (ulong)-1
  char existing_file[FN_REFLEN];
  if (find_existing_last_file(existing_file, log_name)) {
    // NO_LINT_DEBUG
    sql_print_error(
        "MYSQL_BIN_LOG::open_existing_binlog failed to locate last file");
    DBUG_RETURN(1);
  }

  // At the end of this "log_file_name" is set to existing_file
  if (init_and_set_log_file_name(log_name, existing_file,
                                 /*new_index_number=*/0)) {
    // NO_LINT_DEBUG
    sql_print_error(
        "MYSQL_BIN_LOG::open_existing_binlog failed to generate "
        "new file name.");
    DBUG_RETURN(1);
  }

  my_free(name);
  if (!(name = my_strdup(key_memory_MYSQL_LOG_name, log_name, MYF(MY_WME)))) {
    // NO_LINT_DEBUG
    sql_print_error("Could not allocate name %s (error %d)", log_name, errno);
    DBUG_RETURN(1);
  }

  auto binlog_file =
      Binlog_ofile::open_existing(m_key_file_log, existing_file, MYF(MY_WME));
  if (!binlog_file) goto err;

  // release current point before assign
  delete m_binlog_file;
  m_binlog_file = binlog_file.release();

  file = mysql_file_open(m_key_file_log, existing_file, O_CREAT | O_WRONLY,
                         MYF(MY_WME));

  if (file < 0) goto err;

  // seek to end of file
  mysql_file_seek(file, 0L, MY_SEEK_END, MYF(0));
  if ((offset = mysql_file_tell(file, MYF(MY_WME))) == MY_FILEPOS_ERROR) {
    if (my_errno() == ESPIPE)
      offset = 0;
    else
      goto err;
  }

  if (file >= 0) mysql_file_close(file, MYF(0));

  // Now setup the m_binlog_file correctly by seeking to the end position in the
  // file
  if (offset > 0) m_binlog_file->seek(offset);

  max_size = max_size_arg;
  update_binlog_end_pos(need_end_log_pos_lock);
  atomic_log_state = LOG_OPENED;

  DBUG_RETURN(0);  // Success

err:
  if (should_abort_on_binlog_error()) {
    exec_binlog_error_action_abort(
        "Either disk is full or file system is read "
        "only while opening the binlog. Aborting the"
        " server.");
  } else
    // NO_LINT_DEBUG
    sql_print_error(
        "Could not open %s for logging (error %d). "
        "Turning logging off for the whole duration "
        "of the MySQL server process. To turn it on "
        "again: fix the cause, shutdown the MySQL "
        "server and restart it.",
        name, my_errno());

  if (file >= 0) mysql_file_close(file, MYF(0));

  my_free(name);
  name = NULL;
  atomic_log_state = LOG_CLOSED;

  DBUG_RETURN(1);  // Error
}

/**
  Move crash safe index file to index file.

  @param need_lock_index If true, LOCK_index will be acquired;
  otherwise it should already be held.

  @retval 0 ok
  @retval -1 error
*/
int MYSQL_BIN_LOG::move_crash_safe_index_file_to_index_file(
    bool need_lock_index) {
  int error = 0;
  File fd = -1;
  DBUG_TRACE;
  int failure_trials = MYSQL_BIN_LOG::MAX_RETRIES_FOR_DELETE_RENAME_FAILURE;
  bool file_rename_status = false, file_delete_status = false;
  THD *thd = current_thd;

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  if (my_b_inited(&index_file)) {
    end_io_cache(&index_file);
    if (mysql_file_close(index_file.file, MYF(0)) < 0) {
      error = -1;
      LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_CLOSE_INDEX_FILE_WHILE_REBUILDING,
             index_file_name);
      /*
        Delete Crash safe index file here and recover the binlog.index
        state(index_file io_cache) from old binlog.index content.
       */
      mysql_file_delete(key_file_binlog_index, crash_safe_index_file_name,
                        MYF(0));

      goto recoverable_err;
    }

    /*
      Sometimes an outsider can lock index files for temporary viewing
      purpose. For eg: MEB locks binlog.index/relaylog.index to view
      the content of the file. During that small period of time, deletion
      of the file is not possible on some platforms(Eg: Windows)
      Server should retry the delete operation for few times instead of
      panicking immediately.
    */
    while ((file_delete_status == false) && (failure_trials > 0)) {
      if (DBUG_EVALUATE_IF("force_index_file_delete_failure", 1, 0)) break;

      DBUG_EXECUTE_IF("simulate_index_file_delete_failure", {
        /* This simulation causes the delete to fail */
        static char first_char = index_file_name[0];
        index_file_name[0] = 0;
        sql_print_information("Retrying delete");
        if (failure_trials == 1) index_file_name[0] = first_char;
      };);
      file_delete_status = !(mysql_file_delete(key_file_binlog_index,
                                               index_file_name, MYF(MY_WME)));
      --failure_trials;
      if (!file_delete_status) {
        my_sleep(1000);
        /* Clear the error before retrying. */
        if (failure_trials > 0) thd->clear_error();
      }
    }

    if (!file_delete_status) {
      error = -1;
      LogErr(ERROR_LEVEL,
             ER_BINLOG_FAILED_TO_DELETE_INDEX_FILE_WHILE_REBUILDING,
             index_file_name);
      /*
        Delete Crash safe file index file here and recover the binlog.index
        state(index_file io_cache) from old binlog.index content.
       */
      mysql_file_delete(key_file_binlog_index, crash_safe_index_file_name,
                        MYF(0));

      goto recoverable_err;
    }
  }

  DBUG_EXECUTE_IF("crash_create_before_rename_index_file", DBUG_SUICIDE(););
  /*
    Sometimes an outsider can lock index files for temporary viewing
    purpose. For eg: MEB locks binlog.index/relaylog.index to view
    the content of the file. During that small period of time, rename
    of the file is not possible on some platforms(Eg: Windows)
    Server should retry the rename operation for few times instead of panicking
    immediately.
  */
  failure_trials = MYSQL_BIN_LOG::MAX_RETRIES_FOR_DELETE_RENAME_FAILURE;
  while ((file_rename_status == false) && (failure_trials > 0)) {
    DBUG_EXECUTE_IF("simulate_crash_safe_index_file_rename_failure", {
      /* This simulation causes the rename to fail */
      static char first_char = index_file_name[0];
      index_file_name[0] = 0;
      sql_print_information("Retrying rename");
      if (failure_trials == 1) index_file_name[0] = first_char;
    };);
    file_rename_status =
        !(my_rename(crash_safe_index_file_name, index_file_name, MYF(MY_WME)));
    --failure_trials;
    if (!file_rename_status) {
      my_sleep(1000);
      /* Clear the error before retrying. */
      if (failure_trials > 0) thd->clear_error();
    }
  }
  if (!file_rename_status) {
    error = -1;
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_RENAME_INDEX_FILE_WHILE_REBUILDING,
           index_file_name);
    goto fatal_err;
  }
  DBUG_EXECUTE_IF("crash_create_after_rename_index_file", DBUG_SUICIDE(););

recoverable_err:
  if ((fd = mysql_file_open(key_file_binlog_index, index_file_name,
                            O_RDWR | O_CREAT, MYF(MY_WME))) < 0 ||
      mysql_file_sync(fd, MYF(MY_WME)) ||
      init_io_cache_ext(&index_file, fd, IO_SIZE, READ_CACHE,
                        mysql_file_seek(fd, 0L, MY_SEEK_END, MYF(0)), false,
                        MYF(MY_WME | MY_WAIT_IF_FULL),
                        key_file_binlog_index_cache)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_OPEN_INDEX_FILE_AFTER_REBUILDING,
           index_file_name);
    goto fatal_err;
  }

  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);
  return error;

fatal_err:
  /*
    This situation is very very rare to happen (unless there is some serious
    memory related issues like OOM) and should be treated as fatal error.
    Hence it is better to bring down the server without respecting
    'binlog_error_action' value here.
  */
  exec_binlog_error_action_abort(
      "MySQL server failed to update the "
      "binlog.index file's content properly. "
      "It might not be in sync with available "
      "binlogs and the binlog.index file state is in "
      "unrecoverable state. Aborting the server.");
  /*
    Server is aborted in the above function.
    This is dead code to make compiler happy.
   */
  return error;
}

/**
  Append log file name to index file.

  - To make crash safe, we copy all the content of index file
  to crash safe index file firstly and then append the log
  file name to the crash safe index file. Finally move the
  crash safe index file to index file.

  @retval
    0   ok
  @retval
    -1   error
*/
int MYSQL_BIN_LOG::add_log_to_index(uchar *log_name, size_t log_name_len,
                                    bool need_lock_index,
                                    uchar *previous_gtid_set_buffer,
                                    uint gtid_set_length) {
  char gtid_set_length_buffer[11];
  DBUG_TRACE;

  DBUG_EXECUTE_IF("simulate_disk_full_add_log_to_index",
                  { DBUG_SET("+d,simulate_no_free_space_error"); });

  if (open_crash_safe_index_file()) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_OPEN_TMP_INDEX,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  if (copy_file(&index_file, &crash_safe_index_file, 0)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_COPY_INDEX_TO_TMP,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  if (my_b_write(&crash_safe_index_file, log_name, log_name_len)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_APPEND_LOG_TO_TMP_INDEX, log_name);
    goto err;
  }

  if (gtid_set_length > 0) {
    DBUG_ASSERT(previous_gtid_set_buffer != nullptr);
    longlong10_to_str(gtid_set_length, gtid_set_length_buffer, 10);

    DBUG_PRINT("info", ("file_name and gtid_set %s\n %s\n", log_name,
                        previous_gtid_set_buffer));
    if (my_b_write(&crash_safe_index_file, (const uchar *)" ", 1) ||
        my_b_write(&crash_safe_index_file, (uchar *)gtid_set_length_buffer,
                   strlen(gtid_set_length_buffer)) ||
        my_b_write(&crash_safe_index_file, (const uchar *)"\n", 1) ||
        my_b_write(&crash_safe_index_file,
                   (const uchar *)previous_gtid_set_buffer, gtid_set_length)) {
      LogErr(ERROR_LEVEL, ER_BINLOG_CANT_APPEND_LOG_TO_TMP_INDEX, log_name);
      goto err;
    }
  }

  if (my_b_write(&crash_safe_index_file, (const uchar *)"\n", 1)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_CLOSE_TMP_INDEX,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  if (need_lock_index) mysql_mutex_lock(&LOCK_index);
  previous_gtid_set_map.insert(std::pair<string, string>(
      string((char *)log_name, log_name_len),
      string((char *)previous_gtid_set_buffer, gtid_set_length)));
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  if (flush_io_cache(&crash_safe_index_file) ||
      mysql_file_sync(crash_safe_index_file.file, MYF(MY_WME))) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_CLOSE_TMP_INDEX,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  if (close_crash_safe_index_file()) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_CLOSE_TMP_INDEX,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  if (move_crash_safe_index_file_to_index_file(need_lock_index)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_MOVE_TMP_TO_INDEX,
           "MYSQL_BIN_LOG::add_log_to_index");
    goto err;
  }

  DBUG_EXECUTE_IF("simulate_disk_full_add_log_to_index",
                  { DBUG_SET("-d,simulate_no_free_space_error"); });

  if (enable_raft_plugin && is_apply_log) apply_file_count++;

  return 0;

err:
  DBUG_EXECUTE_IF("simulate_disk_full_add_log_to_index", {
    DBUG_SET("-d,simulate_no_free_space_error");
    DBUG_SET("-d,simulate_file_write_error");
  });
  return -1;
}

int MYSQL_BIN_LOG::get_current_log(LOG_INFO *linfo,
                                   bool need_lock_log /*true*/) {
  if (need_lock_log) mysql_mutex_lock(&LOCK_log);
  int ret = raw_get_current_log(linfo);
  if (need_lock_log) mysql_mutex_unlock(&LOCK_log);
  return ret;
}

void MYSQL_BIN_LOG::get_current_log_without_lock_log(LOG_INFO *linfo) {
  mysql_mutex_assert_owner(&LOCK_binlog_end_pos);
  strmake(linfo->log_file_name, binlog_file_name,
          sizeof(linfo->log_file_name) - 1);
  linfo->pos = atomic_binlog_end_pos.load(std::memory_order_relaxed);
  linfo->encrypted_header_size = binlog_encrypted_header_size;
}

int MYSQL_BIN_LOG::raw_get_current_log(LOG_INFO *linfo) {
  strmake(linfo->log_file_name, log_file_name,
          sizeof(linfo->log_file_name) - 1);
  // raft write data directly into IO_CACHE, thus
  // Binlog_ofile's m_position member isn't updated.
  if (enable_raft_plugin && !is_apply_log)
    linfo->pos = atomic_binlog_end_pos.load();
  else
    linfo->pos = m_binlog_file->position();

  linfo->encrypted_header_size = m_binlog_file->get_encrypted_header_size();
  return 0;
}

bool MYSQL_BIN_LOG::check_write_error(const THD *thd) {
  DBUG_TRACE;

  bool checked = false;

  if (!thd->is_error()) return checked;

  // Check all conditions for one that matches the expected error
  const Sql_condition *err;
  auto it = thd->get_stmt_da()->sql_conditions();
  while ((err = it++) != nullptr && !checked) {
    switch (err->mysql_errno()) {
      case ER_TRANS_CACHE_FULL:
      case ER_STMT_CACHE_FULL:
      case ER_ERROR_ON_WRITE:
      case ER_BINLOG_LOGGING_IMPOSSIBLE:
        checked = true;
        break;
    }
  }

  DBUG_PRINT("return", ("checked: %s", YESNO(checked)));
  return checked;
}

void MYSQL_BIN_LOG::report_cache_write_error(THD *thd, bool is_transactional) {
  DBUG_TRACE;

  write_error = true;

  if (check_write_error(thd)) return;

  if (my_errno() == EFBIG) {
    if (is_transactional) {
      my_error(ER_TRANS_CACHE_FULL, MYF(MY_WME));
    } else {
      my_error(ER_STMT_CACHE_FULL, MYF(MY_WME));
    }
  } else {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_error(ER_ERROR_ON_WRITE, MYF(MY_WME), name, errno,
             my_strerror(errbuf, sizeof(errbuf), errno));
  }
}

static int compare_log_name(const char *log_1, const char *log_2) {
  const char *log_1_basename = log_1 + dirname_length(log_1);
  const char *log_2_basename = log_2 + dirname_length(log_2);

  return strcmp(log_1_basename, log_2_basename);
}

uint split_file_name_and_gtid_set_length(char *file_name_and_gtid_set_length) {
  char *save_ptr = NULL;
  save_ptr = strchr(file_name_and_gtid_set_length, ' ');
  if (save_ptr != NULL) {
    *save_ptr = 0;  // replace ' ' with '\0'
    save_ptr++;
    return atol(save_ptr);
  }
  return 0;
}

int MYSQL_BIN_LOG::get_total_log_files(bool need_lock_index,
                                       uint64_t *num_log_files) {
  int error = 0;
  LOG_INFO temp_log_info;
  *num_log_files = 0;

  if (need_lock_index) mysql_mutex_lock(&LOCK_index);

  if ((error = find_log_pos(&temp_log_info, /*log_name=*/NullS,
                            /*need_lock_index=*/false)))
    goto err;

  *num_log_files = *num_log_files + 1;
  while (!(error = find_next_log(&temp_log_info, /*need_lock_index=*/false)))
    *num_log_files = *num_log_files + 1;

err:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  if (error == LOG_INFO_EOF) error = 0;  // EOF is not an error

  return error;
}

bool is_binlog_advanced(const char *b1, my_off_t p1, const char *b2,
                        my_off_t p2) {
  if (!b1 || !b1[0]) {
    return (b2 && b2[0] && p2 != ULLONG_MAX);
  }

  if (!b2 || !b2[0]) {
    return false;
  }

  DBUG_ASSERT(p1 != ULLONG_MAX && p2 != ULLONG_MAX);

  int cmp = strlen(b1) - strlen(b2);

  if (cmp) {
    return (cmp < 0);
  }

  return (strcmp(b1, b2) < 0 || (strcmp(b1, b2) == 0 && p1 < p2));
}

/**
  Find the position in the log-index-file for the given log name.

  @param[out] linfo The found log file name will be stored here, along
  with the byte offset of the next log file name in the index file.
  @param log_name Filename to find in the index file, or NULL if we
  want to read the first entry.
  @param need_lock_index If false, this function acquires LOCK_index;
  otherwise the lock should already be held by the caller.

  @note
    On systems without the truncate function the file will end with one or
    more empty lines.  These will be ignored when reading the file.

  @retval
    0			ok
  @retval
    LOG_INFO_EOF	        End of log-index-file found
  @retval
    LOG_INFO_IO		Got IO error while reading file
*/

int MYSQL_BIN_LOG::find_log_pos(LOG_INFO *linfo, const char *log_name,
                                bool need_lock_index) {
  int error = 0;
  char *full_fname = linfo->log_file_name;
  char full_log_name[FN_REFLEN];
  char file_name_and_gtid_set_length[FILE_AND_GTID_SET_LENGTH];
  DBUG_TRACE;
  full_log_name[0] = full_fname[0] = 0;

  /*
    Mutex needed because we need to make sure the file pointer does not
    move from under our feet
  */
  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  if (!my_b_inited(&index_file)) {
    error = LOG_INFO_IO;
    goto end;
  }

  // extend relative paths for log_name to be searched
  if (log_name) {
    if (normalize_binlog_name(full_log_name, log_name, is_relay_log)) {
      error = LOG_INFO_EOF;
      goto end;
    }
  }

  DBUG_PRINT("enter", ("log_name: %s, full_log_name: %s",
                       log_name ? log_name : "NULL", full_log_name));

  /* As the file is flushed, we can't get an error here */
  my_b_seek(&index_file, (my_off_t)0);

  for (;;) {
    size_t length;
    my_off_t offset = my_b_tell(&index_file);

    DBUG_EXECUTE_IF("simulate_find_log_pos_error", error = LOG_INFO_EOF;
                    break;);
    /* If we get 0 or 1 characters, this is the end of the file */
    if ((length = my_b_gets(&index_file, file_name_and_gtid_set_length,
                            FILE_AND_GTID_SET_LENGTH)) <= 1) {
      /* Did not find the given entry; Return not found or error */
      error = !index_file.error ? LOG_INFO_EOF : LOG_INFO_IO;
      break;
    }

    file_name_and_gtid_set_length[length - 1] = 0;
    uint gtid_string_length =
        split_file_name_and_gtid_set_length(file_name_and_gtid_set_length);
    if (gtid_string_length > 0) {
      my_b_seek(&index_file, my_b_tell(&index_file) + gtid_string_length + 1);
    }

    // extend relative paths and match against full path
    if (normalize_binlog_name(full_fname, file_name_and_gtid_set_length,
                              is_relay_log)) {
      error = LOG_INFO_EOF;
      break;
    }
    // if the log entry matches, null string matching anything
    if (!log_name || !compare_log_name(full_fname, full_log_name)) {
      DBUG_PRINT("info", ("Found log file entry"));
      linfo->index_file_start_offset = offset;
      linfo->index_file_offset = my_b_tell(&index_file);
      break;
    }
    linfo->entry_index++;
  }

end:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);
  return error;
}

/**
  Find the position in the log-index-file for the given log name.

  @param[out] linfo The filename will be stored here, along with the
  byte offset of the next filename in the index file.

  @param need_lock_index If true, LOCK_index will be acquired;
  otherwise it should already be held by the caller.

  @note
    - Before calling this function, one has to call find_log_pos()
    to set up 'linfo'
    - Mutex needed because we need to make sure the file pointer does not move
    from under our feet

  @retval 0 ok
  @retval LOG_INFO_EOF End of log-index-file found
  @retval LOG_INFO_IO Got IO error while reading file
*/
int MYSQL_BIN_LOG::find_next_log(LOG_INFO *linfo, bool need_lock_index) {
  int error = 0;
  uint length, gtid_string_length;
  char file_name_and_gtid_set_length[FILE_AND_GTID_SET_LENGTH];
  char *full_fname = linfo->log_file_name;
  DBUG_ENTER("find_next_log");

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  DBUG_PRINT("enter", ("index_file_offset: %llu", linfo->index_file_offset));
  if (!my_b_inited(&index_file)) {
    error = LOG_INFO_IO;
    goto err;
  }
  /* As the file is flushed, we can't get an error here */
  my_b_seek(&index_file, linfo->index_file_offset);

  linfo->index_file_start_offset = linfo->index_file_offset;
  if ((length = my_b_gets(&index_file, file_name_and_gtid_set_length,
                          FILE_AND_GTID_SET_LENGTH)) <= 1) {
    error = !index_file.error ? LOG_INFO_EOF : LOG_INFO_IO;
    goto err;
  }

  if (file_name_and_gtid_set_length[0] != 0) {
    file_name_and_gtid_set_length[length - 1] = 0;
    gtid_string_length =
        split_file_name_and_gtid_set_length(file_name_and_gtid_set_length);
    if (gtid_string_length > 0) {
      my_b_seek(&index_file, my_b_tell(&index_file) + gtid_string_length + 1);
    }

    if (normalize_binlog_name(full_fname, file_name_and_gtid_set_length,
                              is_relay_log)) {
      error = LOG_INFO_EOF;
      goto err;
    }
  }

  linfo->index_file_offset = my_b_tell(&index_file);

err:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);
  DBUG_RETURN(error);
}

/**
  Find the relay log name following the given name from relay log index file.

  @param[in,out] log_name  The name is full path name.

  @return return 0 if it finds next relay log. Otherwise return the error code.
*/
int MYSQL_BIN_LOG::find_next_relay_log(char log_name[FN_REFLEN + 1]) {
  LOG_INFO info;
  int error;
  char relative_path_name[FN_REFLEN + 1];

  if (fn_format(relative_path_name, log_name + dirname_length(log_name),
                mysql_data_home, "", 0) == NullS)
    return 1;

  mysql_mutex_lock(&LOCK_index);

  error = find_log_pos(&info, relative_path_name, false);
  if (error == 0) {
    error = find_next_log(&info, false);
    if (error == 0) strcpy(log_name, info.log_file_name);
  }

  mysql_mutex_unlock(&LOCK_index);
  return error;
}

std::pair<int, std::list<std::string>> MYSQL_BIN_LOG::get_log_index(
    bool need_lock_index) {
  DBUG_TRACE;
  LOG_INFO log_info;

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  std::list<std::string> filename_list;
  int error = 0;
  for (error =
           this->find_log_pos(&log_info, nullptr, false /*need_lock_index*/);
       error == 0;
       error = this->find_next_log(&log_info, false /*need_lock_index*/)) {
    filename_list.push_back(std::string(log_info.log_file_name));
  }

  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  return std::make_pair(error, filename_list);
}

/**
  Removes files, as part of a RESET MASTER or RESET SLAVE statement,
  by deleting all logs referred to in the index file and the index
  file. Then, it creates a new index file and a new log file.

  The new index file will only contain the new log file.

  @param thd Thread
  @param delete_only If true, do not create a new index file and
  a new log file.

  @note
    If not called from slave thread, write start event to new log

  @retval
    0	ok
  @retval
    1   error
*/
bool MYSQL_BIN_LOG::reset_logs(THD *thd, bool delete_only) {
  LOG_INFO linfo;
  bool error = false;
  int err;
  const char *save_name = nullptr;
  Checkable_rwlock *sid_lock = nullptr;
  DBUG_TRACE;

  /*
    Flush logs for storage engines, so that the last transaction
    is persisted inside storage engines.
  */
  DBUG_ASSERT(!thd->is_log_reset());
  thd->set_log_reset();
  if (ha_flush_logs()) {
    thd->clear_log_reset();
    return true;
  }
  thd->clear_log_reset();

  ha_reset_logs(thd);

  /*
    We need to get both locks to be sure that no one is trying to
    write to the index log file.
  */
  mysql_mutex_lock(&LOCK_log);
  mysql_mutex_lock(&LOCK_index);

  if (is_relay_log)
    sid_lock = previous_gtid_set_relaylog->get_sid_map()->get_sid_lock();
  else
    sid_lock = global_sid_lock;
  sid_lock->wrlock();

  /* Save variables so that we can reopen the log */
  save_name = name;
  name = nullptr;  // Protect against free
  close(LOG_CLOSE_TO_BE_OPENED, false /*need_lock_log=false*/,
        false /*need_lock_index=false*/);

  /*
    First delete all old log files and then update the index file.
    As we first delete the log files and do not use sort of logging,
    a crash may lead to an inconsistent state where the index has
    references to non-existent files.

    We need to invert the steps and use the purge_index_file methods
    in order to make the operation safe.
  */

  previous_gtid_set_map.clear();
  if ((err = find_log_pos(&linfo, NullS, false /*need_lock_index=false*/)) !=
      0) {
    uint errcode = purge_log_get_error_code(err);
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_LOCATE_OLD_BINLOG_OR_RELAY_LOG_FILES);
    my_error(errcode, MYF(0));
    error = true;
    goto err;
  }

  for (;;) {
    if ((error = my_delete_allow_opened(linfo.log_file_name, MYF(0))) != 0) {
      if (my_errno() == ENOENT) {
        push_warning_printf(
            current_thd, Sql_condition::SL_WARNING, ER_LOG_PURGE_NO_FILE,
            ER_THD(current_thd, ER_LOG_PURGE_NO_FILE), linfo.log_file_name);
        LogErr(INFORMATION_LEVEL, ER_BINLOG_CANT_DELETE_FILE,
               linfo.log_file_name);
        set_my_errno(0);
        error = false;
      } else {
        push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                            ER_BINLOG_PURGE_FATAL_ERR,
                            "a problem with deleting %s; "
                            "consider examining correspondence "
                            "of your binlog index file "
                            "to the actual binlog files",
                            linfo.log_file_name);
        error = true;
        goto err;
      }
    }
    if (find_next_log(&linfo, false /*need_lock_index=false*/)) break;
  }

  /* Start logging with a new file */
  close(LOG_CLOSE_INDEX | LOG_CLOSE_TO_BE_OPENED, false /*need_lock_log=false*/,
        false /*need_lock_index=false*/);
  if ((error = my_delete_allow_opened(index_file_name,
                                      MYF(0))))  // Reset (open will update)
  {
    if (my_errno() == ENOENT) {
      push_warning_printf(
          current_thd, Sql_condition::SL_WARNING, ER_LOG_PURGE_NO_FILE,
          ER_THD(current_thd, ER_LOG_PURGE_NO_FILE), index_file_name);
      LogErr(INFORMATION_LEVEL, ER_BINLOG_CANT_DELETE_FILE, index_file_name);
      set_my_errno(0);
      error = false;
    } else {
      push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                          ER_BINLOG_PURGE_FATAL_ERR,
                          "a problem with deleting %s; "
                          "consider examining correspondence "
                          "of your binlog index file "
                          "to the actual binlog files",
                          index_file_name);
      error = true;
      goto err;
    }
  }
  DBUG_EXECUTE_IF("wait_for_kill_gtid_state_clear", {
    const char action[] = "now WAIT_FOR kill_gtid_state_clear";
    DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(action)));
  };);

  /*
    For relay logs we clear the gtid state associated per channel(i.e rli)
    in the purge_relay_logs()
  */
  if (!is_relay_log) {
    if (gtid_state->clear(thd)) {
      error = true;
    }
    /*
      Don't clear global_sid_map because gtid_state->clear() above didn't
      touched owned_gtids GTID set.
    */
    error = error || gtid_state->init();
  }

  if (!delete_only) {
    if (!open_index_file(index_file_name, nullptr,
                         false /*need_lock_index=false*/))
      error = open_binlog(save_name, nullptr, max_size, false,
                          false /*need_lock_index=false*/,
                          false /*need_sid_lock=false*/, nullptr,
                          thd->lex->next_binlog_file_nr) ||
              error;
  }
  /* String has been duplicated, free old file-name */
  if (name != nullptr) {
    my_free(const_cast<char *>(save_name));
    save_name = nullptr;
  }

err:
  update_lost_gtid_for_tailing();
  if (name == nullptr)
    name = const_cast<char *>(save_name);  // restore old file-name
  sid_lock->unlock();
  mysql_mutex_unlock(&LOCK_index);
  mysql_mutex_unlock(&LOCK_log);
  return error;
}

/**
  Set the name of crash safe index file.

  @retval
    0   ok
  @retval
    1   error
*/
int MYSQL_BIN_LOG::set_crash_safe_index_file_name(const char *base_file_name) {
  int error = 0;
  DBUG_TRACE;
  if (fn_format(crash_safe_index_file_name, base_file_name, mysql_data_home,
                ".index_crash_safe",
                MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH | MY_REPLACE_EXT)) ==
      nullptr) {
    error = 1;
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_SET_TMP_INDEX_NAME);
  }
  return error;
}

/**
  Open a (new) crash safe index file.

  @note
    The crash safe index file is a special file
    used for guaranteeing index file crash safe.
  @retval
    0   ok
  @retval
    1   error
*/
int MYSQL_BIN_LOG::open_crash_safe_index_file() {
  int error = 0;
  File file = -1;

  DBUG_TRACE;

  if (!my_b_inited(&crash_safe_index_file)) {
    myf flags = MY_WME | MY_NABP | MY_WAIT_IF_FULL;
    if (is_relay_log) flags = flags | MY_REPORT_WAITING_IF_FULL;

    if ((file = my_open(crash_safe_index_file_name, O_RDWR | O_CREAT,
                        MYF(MY_WME))) < 0 ||
        init_io_cache(&crash_safe_index_file, file, IO_SIZE, WRITE_CACHE, 0,
                      false, flags)) {
      error = 1;
      LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_OPEN_TEMPORARY_INDEX_FILE);
    }
  }
  return error;
}

/**
  Close the crash safe index file.

  @note
    The crash safe file is just closed, is not deleted.
    Because it is moved to index file later on.
  @retval
    0   ok
  @retval
    1   error
*/
int MYSQL_BIN_LOG::close_crash_safe_index_file() {
  int error = 0;

  DBUG_TRACE;

  if (my_b_inited(&crash_safe_index_file)) {
    end_io_cache(&crash_safe_index_file);
    error = my_close(crash_safe_index_file.file, MYF(0));
  }
  crash_safe_index_file = IO_CACHE();

  return error;
}

/**
  Remove logs from index file.

  - To make crash safe, we copy the content of index file
  from index_file_start_offset recored in log_info to
  crash safe index file firstly and then move the crash
  safe index file to index file.

  @param log_info               Store here the found log file name and
                                position to the NEXT log file name in
                                the index file.

  @param need_update_threads    If we want to update the log coordinates
                                of all threads. False for relay logs,
                                true otherwise.

  @retval
    0    ok
  @retval
    LOG_INFO_IO    Got IO error while reading/writing file
*/
int MYSQL_BIN_LOG::remove_logs_from_index(LOG_INFO *log_info,
                                          bool need_update_threads) {
  DBUG_EXECUTE_IF("simulate_disk_full_remove_logs_from_index",
                  { DBUG_SET("+d,simulate_no_free_space_error"); });
  if (open_crash_safe_index_file()) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_OPEN_TMP_INDEX,
           "MYSQL_BIN_LOG::remove_logs_from_index");
    goto err;
  }

  if (copy_file(&index_file, &crash_safe_index_file,
                log_info->index_file_start_offset)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_COPY_INDEX_TO_TMP,
           "MYSQL_BIN_LOG::remove_logs_from_index");
    goto err;
  }

  if (close_crash_safe_index_file()) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_CLOSE_TMP_INDEX,
           "MYSQL_BIN_LOG::remove_logs_from_index");
    goto err;
  }
  DBUG_EXECUTE_IF("fault_injection_copy_part_file", DBUG_SUICIDE(););

  if (move_crash_safe_index_file_to_index_file(
          false /*need_lock_index=false*/)) {
    LogErr(ERROR_LEVEL, ER_BINLOG_CANT_MOVE_TMP_TO_INDEX,
           "MYSQL_BIN_LOG::remove_logs_from_index");
    goto err;
  }

  DBUG_EXECUTE_IF("simulate_disk_full_remove_logs_from_index",
                  { DBUG_SET("-d,simulate_no_free_space_error"); });

  // now update offsets in index file for running threads
  if (need_update_threads)
    adjust_linfo_offsets(log_info->index_file_start_offset, is_relay_log);
  return 0;

err:
  DBUG_EXECUTE_IF("simulate_disk_full_remove_logs_from_index", {
    DBUG_SET("-d,simulate_no_free_space_error");
    DBUG_SET("-d,simulate_file_write_error");
  });
  return LOG_INFO_IO;
}

void MYSQL_BIN_LOG::purge_apply_logs() {
  if (!is_apply_log) return;

  // No need to trigger purge if number of apply log files in the system
  // currently is lower than apply_log_retention_num
  if (apply_file_count <= apply_log_retention_num) return;

  time_t purge_time = my_time(0) - apply_log_retention_duration /* mins */ * 60;
  if (purge_time > 0) {
    ha_flush_logs(NULL);
    purge_logs_before_date(purge_time, /*auto_purge=*/true,
                           /*stop_purge=*/true);
  }

  return;
}

/**
  Remove all logs before the given log from disk and from the index file.

  @param to_log	      Delete all log file name before this file.
  @param included            If true, to_log is deleted too.
  @param need_lock_index
  @param need_update_threads If we want to update the log coordinates of
                             all threads. False for relay logs, true otherwise.
  @param decrease_log_space  If not null, decrement this variable of
                             the amount of log space freed
  @param auto_purge          True if this is an automatic purge.
  @param max_log             If max_log is found in the index before to_log,
                             then do not purge anything beyond that point

  @note
    If any of the logs before the deleted one is in use,
    only purge logs up to this one.

  @retval
    0			ok
  @retval
    LOG_INFO_EOF          to_log not found
    LOG_INFO_EMFILE       Too many files opened
    LOG_INFO_IO           Got IO error while reading/writing file
    LOG_INFO_FATAL        If any other than ENOENT error from
                                mysql_file_stat() or mysql_file_delete()
*/

int MYSQL_BIN_LOG::purge_logs(const char *to_log, bool included,
                              bool need_lock_index, bool need_update_threads,
                              ulonglong *decrease_log_space, bool auto_purge,
                              const char *max_log) {
  int error = 0, error_index = 0, no_of_log_files_to_purge = 0;
  uint64_t num_log_files = 0;
  int no_of_threads_locking_log = 0;
  bool exit_loop = false;
  bool found_last_safe_file = false;
  LOG_INFO log_info;
  THD *thd = current_thd;
  log_file_name_container delete_list;
  std::pair<std::string, uint> file_index_pair;
  std::string safe_purge_file;
  int raft_plugin_error = 0;

  DBUG_TRACE;
  DBUG_PRINT("info", ("to_log= %s", to_log));

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);
  if ((error =
           find_log_pos(&log_info, to_log, false /*need_lock_index=false*/))) {
    LogErr(ERROR_LEVEL, ER_BINLOG_PURGE_LOGS_CALLED_WITH_FILE_NOT_IN_INDEX,
           to_log);
    goto err;
  }

  no_of_log_files_to_purge = log_info.entry_index;

  /*
    File name exists in index file; delete until we find this file
    or a file that is used.
  */
  if ((error = find_log_pos(&log_info, NullS, false /*need_lock_index=false*/)))
    goto err;

  if (enable_raft_plugin && !is_apply_log) {
    thd->clear_safe_purge_file();
    // Consult the plugin for file that could be deleted safely.
    // This will also allow plugin to clean up its index files and other states
    // (if any)
    file_index_pair = extract_file_index(std::string(to_log));
    if (!included && file_index_pair.second > 0) file_index_pair.second -= 1;

    // Nothing to purge if file index is 0
    if (!included && file_index_pair.second == 0) goto err;

    error = RUN_HOOK_STRICT(raft_replication, purge_logs,
                            (current_thd, file_index_pair.second));

    DBUG_EXECUTE_IF("simulate_raft_plugin_purge_error", error = 1;);

    if (error) {
      // NO_LINT_DEBUG
      sql_print_error(
          "MYSQL_BIN_LOG::purge_logs raft plugin failed in "
          "purge_logs(). file-name: %s",
          to_log);
      raft_plugin_error = 1;
      goto err;
    }

    safe_purge_file = thd->get_safe_purge_file();
  }

  while ((compare_log_name(to_log, log_info.log_file_name) ||
          (exit_loop = included))) {
    if (enable_raft_plugin && found_last_safe_file) {
      // It is not safe to delete any more files since raft needs it for
      // durability or there are peers trying to read from this file
      if (!auto_purge)
        // TODO: Converge on raft specific error codes here
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_WARN_PURGE_LOG_IS_ACTIVE,
            ER_THD(thd, ER_WARN_PURGE_LOG_IS_ACTIVE), log_info.log_file_name);

      break;
    }

    if (is_active(log_info.log_file_name) ||
        (enable_raft_plugin && max_log &&
         strcmp(max_log, log_info.log_file_name) == 0)) {
      // Either the file is active or in raft mode found 'max_log' in the index.
      // Should not delete anything at or beyond 'max_log'
      if (!auto_purge)
        push_warning_printf(
            thd, Sql_condition::SL_WARNING, ER_WARN_PURGE_LOG_IS_ACTIVE,
            ER_THD(thd, ER_WARN_PURGE_LOG_IS_ACTIVE), log_info.log_file_name);
      break;
    }

    if ((no_of_threads_locking_log = log_in_use(log_info.log_file_name))) {
      if (!auto_purge)
        push_warning_printf(thd, Sql_condition::SL_WARNING,
                            ER_WARN_PURGE_LOG_IN_USE,
                            ER_THD(thd, ER_WARN_PURGE_LOG_IN_USE),
                            log_info.log_file_name, no_of_threads_locking_log,
                            delete_list.size(), no_of_log_files_to_purge);
      break;
    }

    delete_list.emplace_back(log_info.log_file_name);

    if (enable_raft_plugin && !is_apply_log && safe_purge_file.length() > 0 &&
        !strcmp(safe_purge_file.c_str(), log_info.log_file_name))
      found_last_safe_file = true;

    if (find_next_log(&log_info, false /*need_lock_index=false*/) || exit_loop)
      break;
  }

  DBUG_EXECUTE_IF("crash_purge_before_update_index", DBUG_SUICIDE(););

  /* Read each entry from the list and delete the file. */
  if ((error_index = purge_logs_in_list(delete_list, thd, decrease_log_space,
                                        false /*need_lock_index=false*/)))
    LogErr(ERROR_LEVEL, ER_BINLOG_PURGE_LOGS_FAILED_TO_PURGE_LOG);

  DBUG_EXECUTE_IF("crash_purge_critical_before_update_index", DBUG_SUICIDE(););

  /* We know how many files to delete. Update index file. */
  if (delete_list.size() &&
      (error = remove_logs_from_index(&log_info, need_update_threads))) {
    LogErr(ERROR_LEVEL, ER_BINLOG_PURGE_LOGS_CANT_UPDATE_INDEX_FILE);
    goto err;
  }

  if (enable_raft_plugin && is_apply_log) {
    if (apply_file_count > delete_list.size()) {
      apply_file_count -= delete_list.size();
    } else {
      // NO_LINT_DEBUG
      sql_print_information(
          "Apply file count needs to be fixed. "
          "apply_file_count = %lu, number of deleted apply files = %lu",
          apply_file_count.load(), delete_list.size());

      error = get_total_log_files(/*need_lock_index=*/false, &num_log_files);

      apply_file_count.store(num_log_files);
      // NO_LINT_DEBUG
      sql_print_information(
          "Fixed apply file count (%lu) by reading from "
          "index file.",
          apply_file_count.load());
    }
  }

  DBUG_EXECUTE_IF("crash_purge_non_critical_after_update_index",
                  DBUG_SUICIDE(););

  // Update gtid_state->lost_gtids
  if (!is_relay_log) {
    global_sid_lock->wrlock();
    error = init_gtid_sets(
        nullptr, const_cast<Gtid_set *>(gtid_state->get_lost_gtids()),
        opt_master_verify_checksum, false /*false=don't need lock*/,
        nullptr /*trx_parser*/, nullptr /*partial_trx*/);
    global_sid_lock->unlock();
  }

  if (enable_raft_plugin && is_relay_log) {
    error = init_prev_gtid_sets_map();
  }

err:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  /*
    Error codes from purge logs take precedence.
    Then error codes from purging the index entry.
  */
  error = error ? error : error_index;
  if (error && should_abort_on_binlog_error()) {
    if (!raft_plugin_error || abort_on_raft_purge_error) {
      exec_binlog_error_action_abort(
          "Either disk is full, file system is read only or "
          "there was an encryption error while opening the binlog. "
          "Aborting the server.");
    }
  }
  return error;
}

int MYSQL_BIN_LOG::set_purge_index_file_name(const char *base_file_name) {
  int error = 0;
  DBUG_TRACE;
  if (fn_format(
          purge_index_file_name, base_file_name, mysql_data_home, ".~rec~",
          MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH | MY_REPLACE_EXT)) == nullptr) {
    error = 1;
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_SET_PURGE_INDEX_FILE_NAME);
  }
  return error;
}

int MYSQL_BIN_LOG::open_purge_index_file(bool destroy) {
  int error = 0;
  File file = -1;

  DBUG_TRACE;

  if (destroy) close_purge_index_file();

  if (!my_b_inited(&purge_index_file)) {
    myf flags = MY_WME | MY_NABP | MY_WAIT_IF_FULL;
    if (is_relay_log) flags = flags | MY_REPORT_WAITING_IF_FULL;

    if ((file = my_open(purge_index_file_name, O_RDWR | O_CREAT, MYF(MY_WME))) <
            0 ||
        init_io_cache(&purge_index_file, file, IO_SIZE,
                      (destroy ? WRITE_CACHE : READ_CACHE), 0, false, flags)) {
      error = 1;
      LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_OPEN_REGISTER_FILE);
    }
  }
  return error;
}

int MYSQL_BIN_LOG::close_purge_index_file() {
  int error = 0;

  DBUG_TRACE;

  if (my_b_inited(&purge_index_file)) {
    end_io_cache(&purge_index_file);
    error = my_close(purge_index_file.file, MYF(0));
  }
  my_delete(purge_index_file_name, MYF(0));
  new (&purge_index_file) IO_CACHE();

  return error;
}

bool MYSQL_BIN_LOG::is_inited_purge_index_file() {
  DBUG_TRACE;
  return my_b_inited(&purge_index_file);
}

int MYSQL_BIN_LOG::sync_purge_index_file() {
  int error = 0;
  DBUG_TRACE;

  if ((error = flush_io_cache(&purge_index_file)) ||
      (error = my_sync(purge_index_file.file, MYF(MY_WME))))
    return error;

  return error;
}

int MYSQL_BIN_LOG::register_purge_index_entry(const char *entry) {
  int error = 0;
  DBUG_TRACE;

  if ((error = my_b_write(&purge_index_file, (const uchar *)entry,
                          strlen(entry))) ||
      (error = my_b_write(&purge_index_file, (const uchar *)"\n", 1)))
    return error;

  return error;
}

int MYSQL_BIN_LOG::register_create_index_entry(const char *entry) {
  DBUG_TRACE;
  return register_purge_index_entry(entry);
}

int MYSQL_BIN_LOG::purge_index_entry(THD *thd, ulonglong *decrease_log_space,
                                     bool need_lock_index) {
  int error = 0;
  LOG_INFO log_info;
  LOG_INFO check_log_info;
  log_file_name_container delete_list;

  DBUG_TRACE;

  DBUG_ASSERT(my_b_inited(&purge_index_file));

  if ((error =
           reinit_io_cache(&purge_index_file, READ_CACHE, 0, false, false))) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_REINIT_REGISTER_FILE);
    goto err;
  }

  for (;;) {
    size_t length;

    if ((length = my_b_gets(&purge_index_file, log_info.log_file_name,
                            FN_REFLEN)) <= 1) {
      if (purge_index_file.error) {
        error = purge_index_file.error;
        LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_READ_REGISTER_FILE, error);
        goto err;
      }

      /* Reached EOF */
      break;
    }

    /* Get rid of the trailing '\n' */
    log_info.log_file_name[length - 1] = 0;

    if ((error = find_log_pos(&check_log_info, log_info.log_file_name,
                              need_lock_index))) {
      if (error != LOG_INFO_EOF) {
        if (thd) {
          push_warning_printf(thd, Sql_condition::SL_WARNING,
                              ER_BINLOG_PURGE_FATAL_ERR,
                              "a problem with deleting %s and "
                              "reading the binlog index file",
                              log_info.log_file_name);
        } else {
          LogErr(INFORMATION_LEVEL,
                 ER_BINLOG_CANT_DELETE_FILE_AND_READ_BINLOG_INDEX,
                 log_info.log_file_name);
        }
        break;
      }

      error = 0;
      if (!need_lock_index) {
        /*
          This is to avoid triggering an error in NDB.

          @todo: This is weird, what does NDB errors have to do with
          need_lock_index? Explain better or refactor /Sven
        */
        ha_binlog_index_purge_file(current_thd, log_info.log_file_name);
      }

      delete_list.emplace_back(log_info.log_file_name);
    }
  }

  if (!error)
    error = purge_logs_in_list(delete_list, thd, decrease_log_space,
                               need_lock_index);

err:
  return error;
}

/**
  Deletes logs sepecified in a list if they exist on the file system
  @param delete_list         The list of log files to delete
  @param thd                 Pointer to the THD object
  @param decrease_log_space  Amount of space freed
  @param need_lock_index     Need to lock the index?
  @retval
    0                        ok
  @retval
    LOG_INFO_EMFILE          Too many files opened
    LOG_INFO_FATAL           If any other than ENOENT error from
                             mysql_file_stat() or mysql_file_delete()
*/
int MYSQL_BIN_LOG::purge_logs_in_list(
    const log_file_name_container &delete_list, THD *thd,
    ulonglong *decrease_log_space,
    bool need_lock_index MY_ATTRIBUTE((unused))) {
  MY_STAT s;
  int error = 0;

  DBUG_TRACE;

  for (const auto &log_file_name : delete_list) {
    if (!mysql_file_stat(m_key_file_log, log_file_name.c_str(), &s, MYF(0))) {
      if (my_errno() == ENOENT) {
        /*
          It's not fatal if we can't stat a log file that does not exist;
          If we could not stat, we won't delete.
        */
        if (thd) {
          push_warning_printf(
              thd, Sql_condition::SL_WARNING, ER_LOG_PURGE_NO_FILE,
              ER_THD(thd, ER_LOG_PURGE_NO_FILE), log_file_name.c_str());
        }
        LogErr(INFORMATION_LEVEL, ER_CANT_STAT_FILE, log_file_name.c_str());
        set_my_errno(0);
      } else {
        /*
          Other than ENOENT are fatal
        */
        if (thd) {
          push_warning_printf(thd, Sql_condition::SL_WARNING,
                              ER_BINLOG_PURGE_FATAL_ERR,
                              "a problem with getting info on being purged %s; "
                              "consider examining correspondence "
                              "of your binlog index file "
                              "to the actual binlog files",
                              log_file_name.c_str());
        } else {
          LogErr(INFORMATION_LEVEL,
                 ER_BINLOG_CANT_DELETE_LOG_FILE_DOES_INDEX_MATCH_FILES,
                 log_file_name.c_str());
        }
        error = LOG_INFO_FATAL;
        break;
      }
    } else {
      DBUG_PRINT("info", ("purging %s", log_file_name.c_str()));
      if (!mysql_file_delete(key_file_binlog, log_file_name.c_str(), MYF(0))) {
        DBUG_EXECUTE_IF("wait_in_purge_index_entry", {
          const char action[] =
              "now SIGNAL in_purge_index_entry WAIT_FOR go_ahead_sql";
          DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(action)));
          DBUG_SET("-d,wait_in_purge_index_entry");
        };);
        if (decrease_log_space) *decrease_log_space -= s.st_size;
      } else {
        if (my_errno() == ENOENT) {
          if (thd) {
            push_warning_printf(
                thd, Sql_condition::SL_WARNING, ER_LOG_PURGE_NO_FILE,
                ER_THD(thd, ER_LOG_PURGE_NO_FILE), log_file_name.c_str());
          }
          LogErr(INFORMATION_LEVEL, ER_BINLOG_CANT_DELETE_FILE,
                 log_file_name.c_str());
          set_my_errno(0);
        } else {
          if (thd) {
            push_warning_printf(thd, Sql_condition::SL_WARNING,
                                ER_BINLOG_PURGE_FATAL_ERR,
                                "a problem with deleting %s; "
                                "consider examining correspondence "
                                "of your binlog index file "
                                "to the actual binlog files",
                                log_file_name.c_str());
          } else {
            LogErr(INFORMATION_LEVEL,
                   ER_BINLOG_CANT_DELETE_LOG_FILE_DOES_INDEX_MATCH_FILES,
                   log_file_name.c_str());
          }
          if (my_errno() == EMFILE) {
            DBUG_PRINT("info",
                       ("my_errno: %d, set ret = LOG_INFO_EMFILE", my_errno()));
            error = LOG_INFO_EMFILE;
            break;
          }
          error = LOG_INFO_FATAL;
          break;
        }
      }
    }
  }
  return error;
}

/**
  Remove all logs before the given file date from disk and from the
  index file.

  @param purge_time	Delete all log files before given date.
  @param auto_purge     True if this is an automatic purge.
  @param stop_purge     True if this is purge of apply logs and we have to stop
                        purging files based on apply_log_retention_num
  @param need_lock_index  True if LOCK_index need to be acquired
  @param max_log          If max_log is found in the index before to_log,
                          then do not purge anything beyond that point

  @note
    If any of the logs before the deleted one is in use,
    only purge logs up to this one.

  @retval
    0				ok
  @retval
    LOG_INFO_PURGE_NO_ROTATE	Binary file that can't be rotated
    LOG_INFO_FATAL              if any other than ENOENT error from
                                mysql_file_stat() or mysql_file_delete()
*/

int MYSQL_BIN_LOG::purge_logs_before_date(time_t purge_time, bool auto_purge,
                                          bool stop_purge, bool need_lock_index,
                                          const char *max_log) {
  int error = 0;
  int no_of_threads_locking_log = 0;
  uint64_t no_of_log_files_purged = 0;
  bool log_is_active = false, log_is_in_use = false;
  char to_log[FN_REFLEN], copy_log_in_use[FN_REFLEN];
  LOG_INFO log_info;
  MY_STAT stat_area;
  THD *thd = current_thd;
  uint64_t max_files_to_purge = ULONG_MAX;

  DBUG_TRACE;

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  if (enable_raft_plugin && is_apply_log && stop_purge) {
    if (apply_file_count <= apply_log_retention_num) goto err;

    max_files_to_purge = apply_file_count - apply_log_retention_num;
  }

  to_log[0] = 0;

  if ((error = find_log_pos(&log_info, NullS, false /*need_lock_index=false*/)))
    goto err;

  while (!(log_is_active = is_active(log_info.log_file_name))) {
    if (enable_raft_plugin && max_log &&
        strcmp(max_log, log_info.log_file_name) == 0)
      break;

    if (!mysql_file_stat(m_key_file_log, log_info.log_file_name, &stat_area,
                         MYF(0))) {
      if (my_errno() == ENOENT) {
        /*
          It's not fatal if we can't stat a log file that does not exist.
        */
        set_my_errno(0);
      } else {
        /*
          Other than ENOENT are fatal
        */
        if (thd) {
          push_warning_printf(thd, Sql_condition::SL_WARNING,
                              ER_BINLOG_PURGE_FATAL_ERR,
                              "a problem with getting info on being purged %s; "
                              "consider examining correspondence "
                              "of your binlog index file "
                              "to the actual binlog files",
                              log_info.log_file_name);
        } else {
          LogErr(INFORMATION_LEVEL, ER_BINLOG_FAILED_TO_DELETE_LOG_FILE,
                 log_info.log_file_name);
        }
        error = LOG_INFO_FATAL;
        goto err;
      }
    }
    /* check if the binary log file is older than the purge_time
       if yes check if it is in use, if not in use then add
       it in the list of binary log files to be purged.
    */
    else if (stat_area.st_mtime < purge_time) {
      if ((no_of_threads_locking_log = log_in_use(log_info.log_file_name))) {
        if (!auto_purge) {
          log_is_in_use = true;
          strcpy(copy_log_in_use, log_info.log_file_name);
        }
        break;
      }
      strmake(to_log, log_info.log_file_name,
              sizeof(log_info.log_file_name) - 1);
      no_of_log_files_purged++;
    } else
      break;

    if (enable_raft_plugin && is_apply_log && stop_purge &&
        (no_of_log_files_purged >= max_files_to_purge))
      break;
    if (find_next_log(&log_info, false /*need_lock_index=false*/)) break;
  }

  if (log_is_active) {
    if (!auto_purge)
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WARN_PURGE_LOG_IS_ACTIVE,
          ER_THD(thd, ER_WARN_PURGE_LOG_IS_ACTIVE), log_info.log_file_name);
  }

  if (log_is_in_use) {
    int no_of_log_files_to_purge = no_of_log_files_purged + 1;
    while (strcmp(log_file_name, log_info.log_file_name)) {
      if (mysql_file_stat(m_key_file_log, log_info.log_file_name, &stat_area,
                          MYF(0))) {
        if (stat_area.st_mtime < purge_time)
          no_of_log_files_to_purge++;
        else
          break;
      }
      if (find_next_log(&log_info, false /*need_lock_index=false*/)) {
        no_of_log_files_to_purge++;
        break;
      }
    }

    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_WARN_PURGE_LOG_IN_USE,
                        ER_THD(thd, ER_WARN_PURGE_LOG_IN_USE), copy_log_in_use,
                        no_of_threads_locking_log, no_of_log_files_purged,
                        no_of_log_files_to_purge);
  }

  error = (to_log[0] ? purge_logs(to_log, true, false /*need_lock_index=false*/,
                                  true /*need_update_threads=true*/,
                                  reinterpret_cast<ulonglong *>(0), auto_purge,
                                  max_log)
                     : 0);

err:
  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  return error;
}

/**
  Create a new log file name.

  @param[out] buf       Buffer allocated with at least FN_REFLEN bytes where
                        new name is stored.
  @param      log_ident Identity of the binary/relay log.

  @note
    If file name will be longer then FN_REFLEN it will be truncated
*/

void MYSQL_BIN_LOG::make_log_name(char *buf, const char *log_ident) {
  size_t dir_len = dirname_length(log_file_name);
  if (dir_len >= FN_REFLEN) dir_len = FN_REFLEN - 1;
  my_stpnmov(buf, log_file_name, dir_len);
  strmake(buf + dir_len, log_ident, FN_REFLEN - dir_len - 1);
}

/**
  Check if we are writing/reading to the given log file.
*/

bool MYSQL_BIN_LOG::is_active(const char *log_file_name_arg) {
  return !compare_log_name(log_file_name, log_file_name_arg);
}

void MYSQL_BIN_LOG::inc_prep_xids(THD *thd) {
  DBUG_TRACE;
#ifndef DBUG_OFF
  int result = ++m_atomic_prep_xids;
  DBUG_PRINT("debug", ("m_atomic_prep_xids: %d", result));
#else
  m_atomic_prep_xids++;
#endif
  thd->get_transaction()->m_flags.xid_written = true;
}

void MYSQL_BIN_LOG::dec_prep_xids(THD *thd) {
  DBUG_TRACE;
  int32 result = --m_atomic_prep_xids;
  DBUG_PRINT("debug", ("m_atomic_prep_xids: %d", result));
  thd->get_transaction()->m_flags.xid_written = false;
  if (result == 0) {
    mysql_mutex_lock(&LOCK_xids);
    mysql_cond_signal(&m_prep_xids_cond);
    mysql_mutex_unlock(&LOCK_xids);
  }
}

void MYSQL_BIN_LOG::inc_non_xid_trxs(THD *thd) {
  DBUG_ENTER("MYSQL_BIN_LOG::inc_non_xid_trxs");
  mysql_mutex_lock(&LOCK_non_xid_trxs);
  ++non_xid_trxs;
  thd->non_xid_trx = true;
  mysql_mutex_unlock(&LOCK_non_xid_trxs);
  DBUG_VOID_RETURN;
}

void MYSQL_BIN_LOG::dec_non_xid_trxs(THD *thd) {
  DBUG_ENTER("MYSQL_BIN_LOG::dec_non_xid_trxs");

  mysql_mutex_lock(&LOCK_non_xid_trxs);
  DBUG_ASSERT(non_xid_trxs > 0);

  if (non_xid_trxs > 0) --non_xid_trxs;

  thd->non_xid_trx = false;

  DBUG_PRINT("debug", ("non_xid_trxs: %d", non_xid_trxs));

  /* Signal the threads that could be blocked in binlog rotation if the
   * non_xid_trxs is zero*/
  if (non_xid_trxs == 0) mysql_cond_signal(&non_xid_trxs_cond);

  mysql_mutex_unlock(&LOCK_non_xid_trxs);
  DBUG_VOID_RETURN;
}

/*
  Wrappers around new_file_impl to avoid using argument
  to control locking. The argument 1) less readable 2) breaks
  incapsulation 3) allows external access to the class without
  a lock (which is not possible with private new_file_without_locking
  method).

  @retval
    nonzero - error

*/

int MYSQL_BIN_LOG::new_file(
    Format_description_log_event *extra_description_event,
    RaftRotateInfo *raft_rotate_info) {
  return new_file_impl(true /*need_lock_log=true*/, extra_description_event,
                       raft_rotate_info);
}

/*
  @retval
    nonzero - error
*/
int MYSQL_BIN_LOG::new_file_without_locking(
    Format_description_log_event *extra_description_event,
    RaftRotateInfo *raft_rotate_info) {
  return new_file_impl(false /*need_lock_log=false*/, extra_description_event,
                       raft_rotate_info);
}

/*
  Checks binlog error action to identify if the server needs to abort on
  non-recoverable errors when writing to binlog

  @return
    true if server needs to be aborted, false otherwise
*/
bool MYSQL_BIN_LOG::should_abort_on_binlog_error() {
  return (binlog_error_action == ABORT_SERVER ||
          binlog_error_action == ROLLBACK_TRX);
}

/**
  Wait for transactions in committing state to complete.
  Assumes that LOCK_log is held for the entire duration
  which prevents new transactions from sneaking in.
 */
void MYSQL_BIN_LOG::drain_committing_trxs(bool wait_non_xid_trxs_always) {
  mysql_mutex_assert_owner(&LOCK_log);

  mysql_mutex_lock(&LOCK_xids);
  /*
    We need to ensure that the number of prepared XIDs are 0.

    If m_atomic_prep_xids is not zero:
    - We wait for storage engine commit, hence decrease m_atomic_prep_xids
    - We keep the LOCK_log to block new transactions from being
      written to the binary log.
   */
  while (get_prep_xids() > 0) {
    mysql_cond_wait(&m_prep_xids_cond, &LOCK_xids);
  }
  mysql_mutex_unlock(&LOCK_xids);

  if (opt_trim_binlog || wait_non_xid_trxs_always) {
    /* Wait for all non-xid trxs to finish */
    mysql_mutex_lock(&LOCK_non_xid_trxs);
    while (get_non_xid_trxs() > 0)
      mysql_cond_wait(&non_xid_trxs_cond, &LOCK_non_xid_trxs);
    mysql_mutex_unlock(&LOCK_non_xid_trxs);
  }
}

/**
  Start writing to a new log file or reopen the old file.

  @param need_lock_log If true, this function acquires LOCK_log;
  otherwise the caller should already have acquired it.

  @param extra_description_event The master's FDE to be written by the I/O
  thread while creating a new relay log file. This should be NULL for
  binary log files.

  @param raft_rotate_info indicates how the rotate behaves when initiated by
  raft plugin. In non-raft world, this defaults to nullptr

  @retval 0 success
  @retval nonzero - error

  @note The new file name is stored last in the index file
*/
int MYSQL_BIN_LOG::new_file_impl(
    bool need_lock_log, Format_description_log_event *extra_description_event,
    RaftRotateInfo *raft_rotate_info) {
  int error = 0;
  bool close_on_error = false;
  char new_name[FN_REFLEN], *new_name_ptr = nullptr, *old_name, *file_to_open;
  const size_t ERR_CLOSE_MSG_LEN = 1024;
  char close_on_error_msg[ERR_CLOSE_MSG_LEN];
  // Indicates if an error occured inside raft plugin
  int consensus_error = 0;

  // Temporary cache for Rotate Event
  std::unique_ptr<Binlog_cache_storage> temp_binlog_cache;

  memset(close_on_error_msg, 0, sizeof close_on_error_msg);

  DBUG_TRACE;
  if (!is_open()) {
    DBUG_PRINT("info", ("log is closed"));
    return error;
  }

  // If this rotation is initiated from raft plugin, we
  // expect raft to be enabled, otherwise we fail early
  if (raft_rotate_info && !enable_raft_plugin) {
    DBUG_PRINT("info", ("enable_raft_plugin is off"));
    return 1;
  }

  if (need_lock_log) mysql_mutex_lock(&LOCK_log);

  DBUG_EXECUTE_IF("semi_sync_3-way_deadlock",
                  DEBUG_SYNC(current_thd, "before_rotate_binlog"););

  drain_committing_trxs(false);

  bool no_op = raft_rotate_info && raft_rotate_info->noop;

  // skip rotate event append implies rotate event has already
  // been appended to relay log by plugin
  bool skip_re_append = raft_rotate_info && raft_rotate_info->post_append;
  bool rotate_in_listener_context =
      raft_rotate_info &&
      (raft_rotate_info->post_append || raft_rotate_info->noop ||
       raft_rotate_info->config_change_rotate);
  if (skip_re_append) {
    DBUG_ASSERT(is_relay_log);
  }

  // Convenience struct to pass down call tree, e.g. open_binlog
  RaftRotateInfo raft_rotate_info_tmp;

  // rotate events need to go through consensus so that we don't have
  // to trim previous a rotate event, i.e. into a rotated file.
  // if we are rotating an is_apply_log = true, then we are a slave
  // trying to do FLUSH BINARY LOGS, which should not have to go
  // through consensus
  bool rotate_via_raft =
      enable_raft_plugin && (no_op || !is_relay_log) && (!is_apply_log);

  bool config_change_rotate =
      raft_rotate_info && raft_rotate_info->config_change_rotate;
  if (rotate_via_raft) {
    if (!raft_rotate_info) raft_rotate_info = &raft_rotate_info_tmp;
    raft_rotate_info->rotate_via_raft = rotate_via_raft; /* true */

    // post append rotates - no flush and before commit stages required
    // rotate_via_raft - flush and before commit of rotate event needs to happen
    DBUG_ASSERT(!skip_re_append);
  }

  mysql_mutex_lock(&LOCK_index);

  mysql_mutex_assert_owner(&LOCK_log);
  mysql_mutex_assert_owner(&LOCK_index);

  if (DBUG_EVALUATE_IF("expire_logs_always", 0, 1) &&
      (error = ha_flush_logs())) {
    goto end;
  }

  if (!is_relay_log) {
    /* Save set of GTIDs of the last binlog into table on binlog rotation */
    if ((error = gtid_state->save_gtids_of_last_binlog_into_table())) {
      if (error == ER_RPL_GTID_TABLE_CANNOT_OPEN) {
        close_on_error =
            m_binlog_file->get_real_file_size() >=
                static_cast<my_off_t>(max_size) ||
            DBUG_EVALUATE_IF("simulate_max_binlog_size", true, false);

        if (!close_on_error) {
          LogErr(ERROR_LEVEL, ER_BINLOG_UNABLE_TO_ROTATE_GTID_TABLE_READONLY,
                 "Current binlog file was flushed to disk and will be kept in "
                 "use.");
        } else {
          snprintf(close_on_error_msg, sizeof close_on_error_msg,
                   ER_THD(current_thd, ER_RPL_GTID_TABLE_CANNOT_OPEN), "mysql",
                   "gtid_executed");

          if (binlog_error_action != ABORT_SERVER &&
              binlog_error_action != ROLLBACK_TRX)
            LogErr(WARNING_LEVEL,
                   ER_BINLOG_UNABLE_TO_ROTATE_GTID_TABLE_READONLY,
                   "Binary logging going to be disabled.");
        }

        DBUG_EXECUTE_IF("gtid_executed_readonly",
                        { DBUG_SET("-d,gtid_executed_readonly"); });
        DBUG_EXECUTE_IF("simulate_max_binlog_size",
                        { DBUG_SET("-d,simulate_max_binlog_size"); });
      } else {
        close_on_error = true;
        snprintf(close_on_error_msg, sizeof close_on_error_msg, "%s",
                 ER_THD(current_thd, ER_OOM_SAVE_GTIDS));
      }
      goto end;
    }
  }

  /*
    If user hasn't specified an extension, generate a new log name
    We have to do this here and not in open as we want to store the
    new file name in the current binary log file.
  */
  new_name_ptr = new_name;
  if ((error = generate_new_name(new_name, name))) {
    // Use the old name if generation of new name fails.
    strcpy(new_name, name);
    close_on_error = true;
    snprintf(close_on_error_msg, sizeof close_on_error_msg,
             ER_THD(current_thd, ER_NO_UNIQUE_LOGFILE), name);
    if (strlen(close_on_error_msg)) {
      close_on_error_msg[strlen(close_on_error_msg) - 1] = '\0';
    }
    goto end;
  }

  // we have moved the cur log ext and in raft
  // this will mess with the index
  if (rotate_via_raft) raft_cur_log_ext--;
  /*
    Make sure that the log_file is initialized before writing
    Rotate_log_event into it.
  */
  if (m_binlog_file->is_open() && !skip_re_append) {
    /*
      We log the whole file name for log file as the user may decide
      to change base names at some point.
      In raft, no_op rotates should not be tagged as relay log rotates because
      it gets replicated and written to the binlog on the followers
    */
    Rotate_log_event r(
        new_name + dirname_length(new_name), 0, LOG_EVENT_OFFSET,
        is_relay_log && !no_op ? Rotate_log_event::RELAY_LOG : 0);
    if (rotate_in_listener_context) r.common_header->when.tv_sec = my_time(0);

    if (rotate_via_raft) {
      temp_binlog_cache = std::make_unique<Binlog_cache_storage>();
      if (!temp_binlog_cache) {
        error = 1;
        goto end;
      }

      /* 4000 buffer for Metadata and rest of payload */
      my_off_t rotate_cache_size = 4000;
      if (config_change_rotate) {
        rotate_cache_size += raft_rotate_info->config_change.size();
      }
      if ((error =
               temp_binlog_cache->open(rotate_cache_size, rotate_cache_size)))
        goto end;

      Metadata_log_event me;
      if (no_op) {
        me.set_raft_rotate_tag(Metadata_log_event::RRET_NOOP);
      } else if (config_change_rotate) {
        me.set_raft_rotate_tag(Metadata_log_event::RRET_CONFIG_CHANGE);
        me.set_raft_str(raft_rotate_info->config_change);
      } else {
        me.set_raft_rotate_tag(Metadata_log_event::RRET_SIMPLE_ROTATE);
      }

      if (rotate_in_listener_context)
        me.common_header->when.tv_sec = my_time(0);
      // checksum has to be turned off, because the raft plugin
      // will patch the events and generate the final checksum.
      me.common_footer->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
      if ((error = me.write(temp_binlog_cache.get()))) goto end;
      add_bytes_written(me.common_header->data_written);

      // checksum has to be turned off, because the raft plugin
      // will patch the events and generate the final checksum.
      r.common_footer->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
      if ((error = r.write(temp_binlog_cache.get()))) goto end;

      add_bytes_written(r.common_header->data_written);
    } else if (DBUG_EVALUATE_IF("fault_injection_new_file_rotate_event",
                                (error = 1), false) ||
               (error = write_event_to_binlog(&r))) {
      char errbuf[MYSYS_STRERROR_SIZE];
      DBUG_EXECUTE_IF("fault_injection_new_file_rotate_event", errno = 2;);
      close_on_error = true;
      snprintf(close_on_error_msg, sizeof close_on_error_msg,
               ER_THD(current_thd, ER_ERROR_ON_WRITE), name, errno,
               my_strerror(errbuf, sizeof(errbuf), errno));
      my_printf_error(ER_ERROR_ON_WRITE, ER_THD(current_thd, ER_ERROR_ON_WRITE),
                      MYF(ME_FATALERROR), name, errno,
                      my_strerror(errbuf, sizeof(errbuf), errno));
      goto end;
    }

    // AT THIS POINT we should block in Raft mode to replicate the Rotate event.
    if (rotate_via_raft) {
      RaftReplicateMsgOpType op_type = RaftReplicateMsgOpType::OP_TYPE_ROTATE;

      if (no_op)
        op_type = RaftReplicateMsgOpType::OP_TYPE_NOOP;
      else if (config_change_rotate)
        op_type = RaftReplicateMsgOpType::OP_TYPE_CHANGE_CONFIG;

      DBUG_EXECUTE_IF("simulate_before_flush_error_on_new_file", error = 1;);

      if (!error)
        error = RUN_HOOK_STRICT(
            raft_replication, before_flush,
            (current_thd, temp_binlog_cache->get_io_cache(), op_type));

      // time to safely readjust the cur_log_ext back to expected value
      if (!error) {
        error = RUN_HOOK_STRICT(raft_replication, before_commit, (current_thd));

        if (!error) {
          // If there was no error, there is a guarantee that this rotate
          // event has reached consensus. Hence this file extension can now
          // be extended. If we don't check for this error, before_commit
          // hook could have failed and then a truncation request could arrive
          // on the same rotate event/same file which would violate a raft
          // plugin invariant. Lets say the rotate event was the last event in
          // file 3. By incrementing the ext to 4, we would give the signal to
          // MySQL Raft plugin that file 3 has been rotated and therefore can
          // never be a candidate for trimming. However since we did not reach
          // consensus the rotate message in file 3 could get a TruncateOpsAfter
          // call which would trigger an assert
          raft_cur_log_ext++;
        }
      }

      if (error) {
        // NO_LINT_DEBUG
        sql_print_error("Failed to rotate binary log");
        consensus_error = 1;
        current_thd->clear_error();  // Clear previous errors first
        my_error(ER_RAFT_FILE_ROTATION_FAILED, MYF(0), 1);
        goto end;
      }

      if (!error) {
        // Store the rotate op_id in the raft_rotate_info for
        // open_binlog to use
        int64_t r_term, r_index;
        current_thd->get_trans_marker(&r_term, &r_index);
        raft_rotate_info->rotate_opid = std::make_pair(r_term, r_index);
      }
    }

    if ((error = m_binlog_file->flush())) {
      close_on_error = true;
      snprintf(close_on_error_msg, sizeof close_on_error_msg, "%s",
               "Either disk is full or file system is read only");
      goto end;
    }
  }

  DEBUG_SYNC(current_thd, "after_rotate_event_appended");

  old_name = name;
  name = nullptr;  // Don't free name
  close(LOG_CLOSE_TO_BE_OPENED | LOG_CLOSE_INDEX, false /*need_lock_log=false*/,
        false /*need_lock_index=false*/);

  if (checksum_alg_reset != binary_log::BINLOG_CHECKSUM_ALG_UNDEF) {
    DBUG_ASSERT(!is_relay_log);
    DBUG_ASSERT(binlog_checksum_options != checksum_alg_reset);
    binlog_checksum_options = checksum_alg_reset;
  }
  /*
    Note that at this point, atomic_log_state != LOG_CLOSED
    (important for is_open()).
  */

  DEBUG_SYNC(current_thd, "binlog_rotate_between_close_and_open");
  /*
    new_file() is only used for rotation (in FLUSH LOGS or because size >
    max_binlog_size or max_relay_log_size).
    If this is a binary log, the Format_description_log_event at the beginning
    of the new file should have created=0 (to distinguish with the
    Format_description_log_event written at server startup, which should
    trigger temp tables deletion on slaves.
  */

  /* reopen index binlog file, BUG#34582 */
  file_to_open = index_file_name;
  error = open_index_file(index_file_name, nullptr,
                          false /*need_lock_index=false*/);
  if (!error) {
    /* reopen the binary log file. */
    file_to_open = new_name_ptr;
    error = open_binlog(
        old_name, new_name_ptr, max_size, true /*null_created_arg=true*/,
        false /*need_lock_index=false*/, true /*need_sid_lock=true*/,
        extra_description_event, 0 /*new_index_number = 0*/,
        raft_rotate_info /*raft_specific_handling*/);
  }

  /* handle reopening errors */
  if (error) {
    char errbuf[MYSYS_STRERROR_SIZE];
    my_printf_error(ER_CANT_OPEN_FILE, ER_THD(current_thd, ER_CANT_OPEN_FILE),
                    MYF(ME_FATALERROR), file_to_open, error,
                    my_strerror(errbuf, sizeof(errbuf), error));
    close_on_error = true;
    snprintf(close_on_error_msg, sizeof close_on_error_msg,
             ER_THD(current_thd, ER_CANT_OPEN_FILE), file_to_open, error,
             my_strerror(errbuf, sizeof(errbuf), error));
  }
  my_free(old_name);

  // We do after_commit hook only for regular rotates and never for NO-OP
  // rotates.
  // rotate_via_raft = (no_op || !is_relay) && (!is_apply)
  // therefore the condition below is equivalent to
  // (!is_relay) && (!is_apply) i.e this rotate is
  // on a binlog in the master
  // Why do we skip AFTER COMMIT for this rotate?
  // This is to prevent the same after commit notification coming
  // twice, once for this call and later when the apply thread
  // processes this NO-OP event. The double notification can
  // confuse LWM and outOfOrderTrxs computation in the plugin
  if (!error && rotate_via_raft && !no_op) {
    // not trapping return code, because this is the existing
    // pattern in most places of after_commit hook (TODO)
    (void)RUN_HOOK_STRICT(raft_replication, after_commit, (current_thd));
  }
  if (!error && enable_raft_plugin && !no_op &&
      (is_relay_log || rotate_via_raft)) {
    // Register new log to raft
    // Previous close(LOG_CLOSE_TO_BE_OPENED | LOG_CLOSE_INDEX,) will close
    // binlog and its IO_CACHE.
    register_log_entities(current_thd, /*context=*/0,
                          /*need_lock=*/false, is_relay_log);
  }
end:
  if (error && close_on_error /* rotate, flush or reopen failed */) {
    /*
      Close whatever was left opened.

      We are keeping the behavior as it exists today, ie,
      we disable logging and move on (see: BUG#51014).

      TODO: as part of WL#1790 consider other approaches:
       - kill mysql (safety);
       - try multiple locations for opening a log file;
       - switch server to protected/readonly mode
       - ...
    */
    if (should_abort_on_binlog_error()) {
      // Abort the server only if this is not a consensus error. Aborting the
      // server for consensus error is not good since it might lead to crashing
      // all instances in the ring on failure to propagate
      // rotate/no-op/config-change events
      if (!consensus_error) {
        char abort_msg[ERR_CLOSE_MSG_LEN + 48];
        memset(abort_msg, 0, sizeof abort_msg);
        snprintf(abort_msg, sizeof abort_msg,
                 "%s, while rotating the binlog. "
                 "Aborting the server",
                 close_on_error_msg);
        exec_binlog_error_action_abort(abort_msg);
      }
    } else
      LogErr(ERROR_LEVEL, ER_BINLOG_CANT_OPEN_FOR_LOGGING,
             new_name_ptr != nullptr ? new_name_ptr : "new file", errno);

    close(LOG_CLOSE_INDEX, false /*need_lock_log=false*/,
          false /*need_lock_index=false*/);
  }

  mysql_mutex_unlock(&LOCK_index);
  if (need_lock_log) mysql_mutex_unlock(&LOCK_log);

  DEBUG_SYNC(current_thd, "after_disable_binlog");
  return error;
}

/**
  Called after an event has been written to the relay log by the IO
  thread.  This flushes and possibly syncs the file (according to the
  sync options), rotates the file if it has grown over the limit, and
  finally calls signal_update().

  @note The caller must hold LOCK_log before invoking this function.

  @param mi Master_info for the IO thread.

  @retval false success
  @retval true error
*/
bool MYSQL_BIN_LOG::after_write_to_relay_log(Master_info *mi) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("max_size: %lu", max_size));

  // Check pre-conditions
  mysql_mutex_assert_owner(&LOCK_log);
  DBUG_ASSERT(is_relay_log);

  /*
    We allow the relay log rotation by relay log size
    only if the trx parser is not inside a transaction.
  */
  bool can_rotate = mi->transaction_parser.is_not_inside_transaction();

#ifndef DBUG_OFF
  if (m_binlog_file->get_real_file_size() >
          DBUG_EVALUATE_IF("rotate_slave_debug_group", 500, max_size) &&
      !can_rotate) {
    DBUG_PRINT("info", ("Postponing the rotation by size waiting for "
                        "the end of the current transaction."));
  }
#endif

  // Flush and sync
  bool error = flush_and_sync(false);
  if (error) {
    mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
               ER_THD(current_thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
               "failed to flush event to relay log file");
    truncate_relaylog_file(mi, atomic_binlog_end_pos);
  } else {
    if (can_rotate) {
      mysql_mutex_lock(&mi->data_lock);
      /*
        If the last event of the transaction has been flushed, we can add
        the GTID (if it is not empty) to the logged set, or else it will
        not be available in the Previous GTIDs of the next relay log file
        if we are going to rotate the relay log.
      */
      const Gtid *last_gtid_queued = mi->get_queueing_trx_gtid();
      if (!last_gtid_queued->is_empty()) {
        mi->rli->get_sid_lock()->rdlock();
        DBUG_SIGNAL_WAIT_FOR(current_thd, "updating_received_transaction_set",
                             "reached_updating_received_transaction_set",
                             "continue_updating_received_transaction_set");
        mi->rli->add_logged_gtid(last_gtid_queued->sidno,
                                 last_gtid_queued->gno);
        mi->rli->get_sid_lock()->unlock();
      }

      if (mi->is_queueing_trx()) {
        mi->finished_queueing();

        Trx_monitoring_info processing;
        Trx_monitoring_info last;
        mi->get_gtid_monitoring_info()->copy_info_to(&processing, &last);

        // update the compression information
        binlog::global_context.monitoring_context()
            .transaction_compression()
            .update(binlog::monitoring::log_type::RELAY, last.compression_type,
                    last.gtid, last.end_time, last.compressed_bytes,
                    last.uncompressed_bytes,
                    mi->rli->get_gtid_set()->get_sid_map());
      }
      mysql_mutex_unlock(&mi->data_lock);

      /*
        If relay log is too big, rotate. But only if not in the middle of a
        transaction when GTIDs are enabled.

        Also rotate, if a deffered flush request has been placed.

        We now try to mimic the following master binlog behavior: "A transaction
        is written in one chunk to the binary log, so it is never split between
        several binary logs. Therefore, if you have big transactions, you might
        see binary log files larger than max_binlog_size."
      */
      if (m_binlog_file->get_real_file_size() >
              DBUG_EVALUATE_IF("rotate_slave_debug_group", 500, max_size) ||
          mi->is_rotate_requested()) {
        error = new_file_without_locking(mi->get_mi_description_event());
        mi->clear_rotate_requests();
      }
    }
  }

  lock_binlog_end_pos();
  mi->rli->ign_master_log_name_end[0] = 0;
  update_binlog_end_pos(false /*need_lock*/);
  harvest_bytes_written(mi->rli, true /*need_log_space_lock=true*/);
  unlock_binlog_end_pos();

  return error;
}

bool MYSQL_BIN_LOG::write_event(Log_event *ev, Master_info *mi) {
  DBUG_TRACE;

  DBUG_EXECUTE_IF("fail_to_write_ignored_event_to_relay_log", { return true; });
  // check preconditions
  DBUG_ASSERT(is_relay_log);

  mysql_mutex_assert_owner(&LOCK_log);

  // write data
  bool error = false;
  if (!binary_event_serialize(ev, m_binlog_file)) {
    bytes_written += ev->common_header->data_written;
    if (!is_relay_log) {
      binlog_bytes_written += ev->common_header->data_written;
    }
    error = after_write_to_relay_log(mi);
  } else {
    mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
               ER_THD(current_thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
               "failed to write event to the relay log file");
    truncate_relaylog_file(mi, atomic_binlog_end_pos);
    error = true;
  }

  return error;
}

bool MYSQL_BIN_LOG::write_buffer(const char *buf, uint len, Master_info *mi) {
  DBUG_TRACE;

  // check preconditions
  DBUG_ASSERT(is_relay_log);
  mysql_mutex_assert_owner(&LOCK_log);

  // write data
  bool error = false;
  if (m_binlog_file->write(pointer_cast<const uchar *>(buf), len) == 0) {
    bytes_written += len;
    error = after_write_to_relay_log(mi);
  } else {
    mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
               ER_THD(current_thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
               "failed to write event to the relay log file");
    truncate_relaylog_file(mi, atomic_binlog_end_pos);
    error = true;
  }

  return error;
}

bool MYSQL_BIN_LOG::flush() {
  return m_binlog_file->is_open() && m_binlog_file->flush();
}

bool MYSQL_BIN_LOG::flush_and_sync(const bool force) {
  mysql_mutex_assert_owner(&LOCK_log);

  if (m_binlog_file->flush()) return true;

  std::pair<bool, bool> result = sync_binlog_file(force);

  return result.first;
}

void MYSQL_BIN_LOG::start_union_events(THD *thd, query_id_t query_id_param) {
  DBUG_ASSERT(!thd->binlog_evt_union.do_union);
  thd->binlog_evt_union.do_union = true;
  thd->binlog_evt_union.unioned_events = false;
  thd->binlog_evt_union.unioned_events_trans = false;
  thd->binlog_evt_union.first_query_id = query_id_param;
}

void MYSQL_BIN_LOG::stop_union_events(THD *thd) {
  DBUG_ASSERT(thd->binlog_evt_union.do_union);
  thd->binlog_evt_union.do_union = false;
}

bool MYSQL_BIN_LOG::is_query_in_union(THD *thd, query_id_t query_id_param) {
  return (thd->binlog_evt_union.do_union &&
          query_id_param >= thd->binlog_evt_union.first_query_id);
}

/*
  Updates thd's position-of-next-event variables
  after a *real* write a file.
 */
void MYSQL_BIN_LOG::update_thd_next_event_pos(THD *thd) {
  if (likely(thd != nullptr)) {
    thd->set_next_event_pos(log_file_name, m_binlog_file->position());
  }
}

/*
  Moves the last bunch of rows from the pending Rows event to a cache (either
  transactional cache if is_transaction is @c true, or the non-transactional
  cache otherwise. Sets a new pending event.

  @param thd               a pointer to the user thread.
  @param evt               a pointer to the row event.
  @param is_transactional  @c true indicates a transactional cache,
                           otherwise @c false a non-transactional.
*/
int MYSQL_BIN_LOG::flush_and_set_pending_rows_event(THD *thd,
                                                    Rows_log_event *event,
                                                    bool is_transactional) {
  DBUG_TRACE;
  DBUG_ASSERT(mysql_bin_log.is_open());
  DBUG_PRINT("enter", ("event: %p", event));

  int error = 0;
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(thd);

  DBUG_ASSERT(cache_mngr);

  binlog_cache_data *cache_data =
      cache_mngr->get_binlog_cache_data(is_transactional);

  DBUG_PRINT("info", ("cache_mngr->pending(): %p", cache_data->pending()));

  if (Rows_log_event *pending = cache_data->pending()) {
    /*
      Write pending event to the cache.
    */
    if (cache_data->write_event(thd, pending)) {
      report_cache_write_error(thd, is_transactional);
      if (check_write_error(thd) && cache_data &&
          stmt_cannot_safely_rollback(thd))
        cache_data->set_incident();
      delete pending;
      cache_data->set_pending(nullptr);
      return 1;
    }

    delete pending;
  }

  cache_data->set_pending(event);

  return error;
}

/**
  Write an event to the binary log cache.
*/

bool MYSQL_BIN_LOG::write_event(Log_event *event_info,
                                bool write_meta_data_event,
                                force_cache_type force_cache) {
  THD *thd = event_info->thd;
  bool error = true;
  DBUG_TRACE;

  if (thd->binlog_evt_union.do_union) {
    /*
      In Stored function; Remember that function call caused an update.
      We will log the function call to the binary log on function exit
    */
    thd->binlog_evt_union.unioned_events = true;
    thd->binlog_evt_union.unioned_events_trans |=
        event_info->is_using_trans_cache();
    return false;
  }

  /*
    We only end the statement if we are in a top-level statement.  If
    we are inside a stored function, we do not end the statement since
    this will close all tables on the slave. But there can be a special case
    where we are inside a stored function/trigger and a SAVEPOINT is being
    set in side the stored function/trigger. This SAVEPOINT execution will
    force the pending event to be flushed without an STMT_END_F flag. This
    will result in a case where following DMLs will be considered as part of
    same statement and result in data loss on slave. Hence in this case we
    force the end_stmt to be true.
  */
  bool const end_stmt =
      (thd->in_sub_stmt && thd->lex->sql_command == SQLCOM_SAVEPOINT)
          ? true
          : (thd->locked_tables_mode && thd->lex->requires_prelocking());
  if (thd->binlog_flush_pending_rows_event(end_stmt,
                                           event_info->is_using_trans_cache()))
    return error;

  /*
     In most cases this is only called if 'is_open()' is true; in fact this is
     mostly called if is_open() *was* true a few instructions before, but it
     could have changed since.
  */
  if (likely(is_open())) {
    /*
      In the future we need to add to the following if tests like
      "do the involved tables match (to be implemented)
      binlog_[wild_]{do|ignore}_table?" (WL#1049)"
    */
    const char *local_db = event_info->get_db();
    if ((thd && !(thd->variables.option_bits & OPTION_BIN_LOG)) ||
        (thd->lex->sql_command != SQLCOM_ROLLBACK_TO_SAVEPOINT &&
         thd->lex->sql_command != SQLCOM_SAVEPOINT &&
         (!event_info->is_no_filter_event() &&
          !binlog_filter->db_ok(local_db))))
      return false;

    if (force_cache == FORCE_CACHE_STATEMENT) {
      event_info->set_using_stmt_cache();
      event_info->set_immediate_logging();
    } else if (force_cache == FORCE_CACHE_TRANSACTIONAL) {
      event_info->set_using_trans_cache();
    } else {
      DBUG_ASSERT(force_cache == FORCE_CACHE_DEFAULT);
    }

    DBUG_ASSERT(event_info->is_using_trans_cache() ||
                event_info->is_using_stmt_cache());

    if (binlog_start_trans_and_stmt(thd, event_info)) return error;

    bool is_trans_cache = event_info->is_using_trans_cache();
    binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
    binlog_cache_data *cache_data =
        cache_mngr->get_binlog_cache_data(is_trans_cache);

    DBUG_PRINT("info", ("event type: %d", event_info->get_type_code()));

    /*
       No check for auto events flag here - this write method should
       never be called if auto-events are enabled.

       Write first log events which describe the 'run environment'
       of the SQL command. If row-based binlogging, Insert_id, Rand
       and other kind of "setting context" events are not needed.
    */
    if (thd) {
      if (!thd->is_current_stmt_binlog_format_row()) {
        if (thd->stmt_depends_on_first_successful_insert_id_in_prev_stmt) {
          Intvar_log_event e(
              thd, (uchar)binary_log::Intvar_event::LAST_INSERT_ID_EVENT,
              thd->first_successful_insert_id_in_prev_stmt_for_binlog,
              event_info->event_cache_type, event_info->event_logging_type);
          if (cache_data->write_event(thd, &e)) goto err;
        }
        if (thd->auto_inc_intervals_in_cur_stmt_for_binlog.nb_elements() > 0) {
          DBUG_PRINT(
              "info",
              ("number of auto_inc intervals: %u",
               thd->auto_inc_intervals_in_cur_stmt_for_binlog.nb_elements()));
          Intvar_log_event e(
              thd, (uchar)binary_log::Intvar_event::INSERT_ID_EVENT,
              thd->auto_inc_intervals_in_cur_stmt_for_binlog.minimum(),
              event_info->event_cache_type, event_info->event_logging_type);
          if (cache_data->write_event(thd, &e)) goto err;
        }
        if (thd->rand_used) {
          Rand_log_event e(thd, thd->rand_saved_seed1, thd->rand_saved_seed2,
                           event_info->event_cache_type,
                           event_info->event_logging_type);
          if (cache_data->write_event(thd, &e)) goto err;
        }
        if (!thd->user_var_events.empty()) {
          for (size_t i = 0; i < thd->user_var_events.size(); i++) {
            Binlog_user_var_event *user_var_event = thd->user_var_events[i];

            /* setting flags for user var log event */
            uchar flags = User_var_log_event::UNDEF_F;
            if (user_var_event->unsigned_flag)
              flags |= User_var_log_event::UNSIGNED_F;

            User_var_log_event e(
                thd, user_var_event->user_var_event->entry_name.ptr(),
                user_var_event->user_var_event->entry_name.length(),
                user_var_event->value, user_var_event->length,
                user_var_event->type, user_var_event->charset_number, flags,
                event_info->event_cache_type, event_info->event_logging_type);
            if (cache_data->write_event(thd, &e)) goto err;
          }
        }
      }
    }

    /*
      Write the event.
    */
    if (cache_data->write_event(thd, event_info, write_meta_data_event))
      goto err;

    if (DBUG_EVALUATE_IF("injecting_fault_writing", 1, 0)) goto err;

    /*
      After writing the event, if the trx-cache was used and any unsafe
      change was written into it, the cache is marked as cannot safely
      roll back.
    */
    if (is_trans_cache && stmt_cannot_safely_rollback(thd))
      cache_mngr->trx_cache.set_cannot_rollback();

    error = false;

  err:
    if (error) {
      report_cache_write_error(thd, is_trans_cache);
      if (check_write_error(thd) && cache_data &&
          stmt_cannot_safely_rollback(thd))
        cache_data->set_incident();
    }
  }

  return error;
}

/**
  The method executes rotation when LOCK_log is already acquired
  by the caller.

  @param force_rotate  caller can request the log rotation
  @param check_purge   is set to true if rotation took place

  @note
    If rotation fails, for instance the server was unable
    to create a new log file, we still try to write an
    incident event to the current log.

  @note The caller must hold LOCK_log when invoking this function.

  @retval
    nonzero - error in rotating routine.
*/
int MYSQL_BIN_LOG::rotate(bool force_rotate, bool *check_purge) {
  int error = 0;
  DBUG_TRACE;

  DBUG_ASSERT(!is_relay_log);
  mysql_mutex_assert_owner(&LOCK_log);

  *check_purge = false;

  if (DBUG_EVALUATE_IF("force_rotate", 1, 0) || force_rotate ||
      (m_binlog_file->get_real_file_size() >= (my_off_t)max_size) ||
      DBUG_EVALUATE_IF("simulate_max_binlog_size", true, false)) {
    error = new_file_without_locking(nullptr);
    *check_purge = true;
  }
  return error;
}

/**
  The method executes logs purging routine.
*/
void MYSQL_BIN_LOG::purge() {
  if (expire_logs_days || binlog_expire_logs_seconds) {
    DEBUG_SYNC(current_thd, "at_purge_logs_before_date");
    time_t purge_time = 0;

    if (binlog_expire_logs_seconds) {
      purge_time = my_time(0) - binlog_expire_logs_seconds;
    } else
      purge_time = my_time(0) - expire_logs_days * 24 * 60 * 60;

    DBUG_EXECUTE_IF("expire_logs_always", { purge_time = my_time(0); });
    if (purge_time >= 0) {
      Is_instance_backup_locked_result is_instance_locked =
          is_instance_backup_locked(current_thd);

      if (is_instance_locked == Is_instance_backup_locked_result::OOM) {
        exec_binlog_error_action_abort(
            "Out of memory happened while checking if "
            "instance was locked for backup");
      }
      if (is_instance_locked == Is_instance_backup_locked_result::NOT_LOCKED) {
        /*
          Flush logs for storage engines, so that the last transaction
          is persisted inside storage engines.
        */
        ha_flush_logs();
        purge_logs_before_date(purge_time, true);
      }
    }
  }
  // Auto purge apply logs based on retention parameters
  if (is_apply_log) purge_apply_logs();
}

/**
  Execute a FLUSH LOGS statement.

  The method is a shortcut of @c rotate() and @c purge().
  LOCK_log is acquired prior to rotate and is released after it.

  @param thd           Current session.
  @param force_rotate  caller can request the log rotation

  @retval
    nonzero - error in rotating routine.
*/
int MYSQL_BIN_LOG::rotate_and_purge(THD *thd, bool force_rotate) {
  int error = 0;
  DBUG_TRACE;
  bool check_purge = false;

  /*
    FLUSH BINARY LOGS command should ignore 'read-only' and 'super_read_only'
    options so that it can update 'mysql.gtid_executed' replication repository
    table.
  */
  thd->set_skip_readonly_check();
  /*
    Wait for handlerton to insert any pending information into the binlog.
    For e.g. ha_ndbcluster which updates the binlog asynchronously this is
    needed so that the user see its own commands in the binlog.
  */
  ha_binlog_wait(thd);

  DBUG_ASSERT(!is_relay_log);
  mysql_mutex_lock(&LOCK_log);
  error = rotate(force_rotate, &check_purge);
  /*
    NOTE: Run purge_logs wo/ holding LOCK_log because it does not need
          the mutex. Otherwise causes various deadlocks.
  */
  mysql_mutex_unlock(&LOCK_log);

  if (!error && check_purge) purge();

  return error;
}

uint MYSQL_BIN_LOG::next_file_id() {
  uint res;
  mysql_mutex_lock(&LOCK_log);
  res = file_id++;
  mysql_mutex_unlock(&LOCK_log);
  return res;
}

extern "C" char mysql_bin_log_is_open(void) { return mysql_bin_log.is_open(); }

extern "C" void mysql_bin_log_lock_commits(struct snapshot_info_st *ss_info) {
  mysql_bin_log.lock_commits(ss_info);
}

extern "C" void mysql_bin_log_unlock_commits(struct snapshot_info_st *ss_info) {
  mysql_bin_log.unlock_commits(ss_info);
}

void MYSQL_BIN_LOG::lock_commits(snapshot_info_st *ss_info) {
  mysql_mutex_lock(&LOCK_log);
  drain_committing_trxs(true);

  global_sid_lock->wrlock();
  char *gtid_buff = nullptr;
  gtid_state->get_executed_gtids()->to_string(&gtid_buff);

  if (gtid_buff != nullptr) {
    if (enable_binlog_hlc) {
      ss_info->snapshot_hlc = get_current_hlc();
    }

    ss_info->binlog_file = log_file_name;
    ss_info->binlog_pos = m_binlog_file->get_my_b_tell();
    ss_info->gtid_executed = gtid_buff;
    my_free(gtid_buff);
  }

  global_sid_lock->unlock();
}

void MYSQL_BIN_LOG::unlock_commits(snapshot_info_st *ss_info
#ifdef DBUG_OFF
                                       MY_ATTRIBUTE((unused))
#endif
) {
#ifndef DBUG_OFF
  global_sid_lock->wrlock();
  char *gtid_buff = nullptr;
  gtid_state->get_executed_gtids()->to_string(&gtid_buff);

  assert(ss_info != nullptr &&
         ss_info->binlog_file == std::string(log_file_name) &&
         ss_info->binlog_pos == m_binlog_file->get_my_b_tell() &&
         gtid_buff != nullptr &&
         ss_info->gtid_executed == std::string(gtid_buff));

  my_free(gtid_buff);
  global_sid_lock->unlock();
#endif

  mysql_mutex_unlock(&LOCK_log);
}

int MYSQL_BIN_LOG::get_gtid_executed(Sid_map *sid_map, Gtid_set *gtid_set) {
  DBUG_TRACE;
  int error = 0;

  mysql_mutex_lock(&mysql_bin_log.LOCK_commit);
  global_sid_lock->wrlock();

  enum_return_status return_status = global_sid_map->copy(sid_map);
  if (return_status != RETURN_STATUS_OK) {
    error = 1;
    goto end;
  }

  return_status = gtid_set->add_gtid_set(gtid_state->get_executed_gtids());
  if (return_status != RETURN_STATUS_OK) error = 1;

end:
  global_sid_lock->unlock();
  mysql_mutex_unlock(&mysql_bin_log.LOCK_commit);

  return error;
}

/**
  Write the contents of the given IO_CACHE to the binary log.

  The cache will be reset as a READ_CACHE to be able to read the
  contents from it.

  The data will be post-processed: see class Binlog_event_writer for
  details.

  @param cache Events will be read from this IO_CACHE.
  @param writer Events will be written to this Binlog_event_writer.

  @retval true IO error.
  @retval false Success.

  @see MYSQL_BIN_LOG::write_cache
*/
bool MYSQL_BIN_LOG::do_write_cache(Binlog_cache_storage *cache,
                                   Binlog_event_writer *writer) {
  DBUG_TRACE;

  DBUG_EXECUTE_IF("simulate_do_write_cache_failure", {
    DBUG_SET("-d,simulate_do_write_cache_failure");
    return true;
  });

#ifndef DBUG_OFF
  uint64 expected_total_len = cache->length();
  DBUG_PRINT("info", ("bytes in cache= %" PRIu64, expected_total_len));
#endif

  bool error = false;
  if (cache->copy_to(writer, &error)) {
    if (error) report_binlog_write_error();
    return true;
  }
  return false;
}

/**
  Writes an incident event to stmt_cache.

  @param ev Incident event to be written
  @param thd Thread variable
  @param need_lock_log If true, will acquire LOCK_log; otherwise the
  caller should already have acquired LOCK_log.
  @param err_msg Error message written to log file for the incident.
  @param do_flush_and_sync If true, will call flush_and_sync(), rotate() and
  purge().

  @retval false error
  @retval true success
*/
bool MYSQL_BIN_LOG::write_incident(Incident_log_event *ev, THD *thd,
                                   bool need_lock_log, const char *err_msg,
                                   bool do_flush_and_sync) {
  uint error = 0;
  DBUG_TRACE;
  DBUG_ASSERT(err_msg);

  if (!is_open()) return error;

  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);

  /*
    thd->cache_mngr may be uninitialized when first transaction resulted in an
    incident. If there is no cache manager exists for the session, then we
    create one, so that a GTID is generated and is written prior to flushing
    the stmt_cache.
  */
  if (cache_mngr == NULL ||
      DBUG_EVALUATE_IF("simulate_cache_creation_failure", 1, 0)) {
    if (thd->binlog_setup_trx_data() ||
        DBUG_EVALUATE_IF("simulate_cache_creation_failure", 1, 0)) {
      enum_gtid_mode gtid_mode = get_gtid_mode(GTID_MODE_LOCK_NONE);
      if (gtid_mode == GTID_MODE_ON || gtid_mode == GTID_MODE_ON_PERMISSIVE) {
        const char *mode = gtid_mode == GTID_MODE_ON ? "ON" : "ON_PERMISSIVE";
        std::ostringstream message;

        message << "Could not create IO cache while writing an incident event "
                   "to the binary log for query: '"
                << thd->query().str << "'. Since GTID_MODE= " << mode
                << ", server is unable to proceed with logging.";
        handle_binlog_flush_or_sync_error(thd, true, message.str().c_str());
        return true;
      }
    } else
      cache_mngr = thd_get_cache_mngr(thd);
  }

#ifndef DBUG_OFF
  if (DBUG_EVALUATE_IF("simulate_write_incident_event_into_binlog_directly", 1,
                       0) &&
      !cache_mngr->stmt_cache.is_binlog_empty()) {
    /* The stmt_cache contains corruption data, so we can reset it. */
    cache_mngr->stmt_cache.reset();
  }
#endif

  /*
    If there is no binlog cache then we write incidents directly
    into the binlog. If caller needs GTIDs it has to setup the
    binlog cache (for the injector thread).
  */
  if (cache_mngr == nullptr ||
      DBUG_EVALUATE_IF("simulate_write_incident_event_into_binlog_directly", 1,
                       0)) {
    if (need_lock_log)
      mysql_mutex_lock(&LOCK_log);
    else
      mysql_mutex_assert_owner(&LOCK_log);
    /* Write an incident event into binlog directly. */
    error = write_event_to_binlog(ev);
    /*
      Write an error to log. So that user might have a chance
      to be alerted and explore incident details.
    */
    if (!error)
      LogErr(ERROR_LEVEL, ER_BINLOG_LOGGING_INCIDENT_TO_STOP_SLAVES, err_msg);
  } else  // (cache_mngr != NULL)
  {
    if (!cache_mngr->stmt_cache.is_binlog_empty()) {
      /* The stmt_cache contains corruption data, so we can reset it. */
      cache_mngr->stmt_cache.reset();
    }
    if (!cache_mngr->trx_cache.is_binlog_empty()) {
      /* The trx_cache contains corruption data, so we can reset it. */
      cache_mngr->trx_cache.reset();
    }
    /*
      Write the incident event into stmt_cache, so that a GTID is generated and
      written for it prior to flushing the stmt_cache.
    */
    binlog_cache_data *cache_data = cache_mngr->get_binlog_cache_data(false);
    if ((error = cache_data->write_event(thd, ev))) {
      LogErr(ERROR_LEVEL, ER_BINLOG_EVENT_WRITE_TO_STMT_CACHE_FAILED);
      cache_mngr->stmt_cache.reset();
      return error;
    }

    if (need_lock_log)
      mysql_mutex_lock(&LOCK_log);
    else
      mysql_mutex_assert_owner(&LOCK_log);
  }
  if (!is_relay_log) {
    binlog_bytes_written += ev->common_header->data_written;
  }

  if (do_flush_and_sync) {
    if (!error && !(error = flush_and_sync())) {
      bool check_purge = false;
      update_binlog_end_pos();
      is_rotating_caused_by_incident = true;
      error = rotate(true, &check_purge);
      is_rotating_caused_by_incident = false;
      if (!error && check_purge) purge();
    }
  }

  if (need_lock_log) mysql_mutex_unlock(&LOCK_log);

  /*
    Write an error to log. So that user might have a chance
    to be alerted and explore incident details.
  */
  if (!error && cache_mngr != nullptr)
    LogErr(ERROR_LEVEL, ER_BINLOG_LOGGING_INCIDENT_TO_STOP_SLAVES, err_msg);

  return error;
}

bool MYSQL_BIN_LOG::write_dml_directly(THD *thd, const char *stmt,
                                       size_t stmt_len) {
  bool ret = false;
  /* backup the original command */
  enum_sql_command save_sql_command = thd->lex->sql_command;

  /* Fake it as a DELETE statement, so it can be binlogged correctly */
  thd->lex->sql_command = SQLCOM_DELETE;

  if (thd->binlog_query(THD::STMT_QUERY_TYPE, stmt, stmt_len, false, false,
                        false, 0) ||
      commit(thd, false) != TC_LOG::RESULT_SUCCESS) {
    ret = true;
  }

  thd->lex->sql_command = save_sql_command;
  return ret;
}

/**
  Creates an incident event and writes it to the binary log.

  @param thd  Thread variable
  @param need_lock_log If the binary lock should be locked or not
  @param err_msg Error message written to log file for the incident.
  @param do_flush_and_sync If true, will call flush_and_sync(), rotate() and
  purge().

  @retval
    0    error
  @retval
    1    success
*/
bool MYSQL_BIN_LOG::write_incident(THD *thd, bool need_lock_log,
                                   const char *err_msg,
                                   bool do_flush_and_sync) {
  DBUG_TRACE;

  if (!is_open()) return false;

  LEX_CSTRING write_error_msg = {err_msg, strlen(err_msg)};
  binary_log::Incident_event::enum_incident incident =
      binary_log::Incident_event::INCIDENT_LOST_EVENTS;
  Incident_log_event ev(thd, incident, write_error_msg);

  return write_incident(&ev, thd, need_lock_log, err_msg, do_flush_and_sync);
}

/*
  Write the event into current binlog directly without going though a session
  binlog cache. It will update the event's log_pos and set checksum accordingly.
  binary_event_serialize can be called directly if log_pos should not be
  updated.
*/
inline bool MYSQL_BIN_LOG::write_event_to_binlog(Log_event *ev) {
  ev->common_footer->checksum_alg =
      is_relay_log
          ? relay_log_checksum_alg
          : static_cast<enum_binlog_checksum_alg>(binlog_checksum_options);
  DBUG_ASSERT(ev->common_footer->checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_UNDEF);

  /*
    Stores current position into log_pos, it is used to calculate correcty
    end_log_pos by adding data_written in Log_event::write_header().
  */
  ev->common_header->log_pos = m_binlog_file->position();

  if (binary_event_serialize(ev, m_binlog_file)) return true;

  add_bytes_written(ev->common_header->data_written);
  return false;
}

/* Write the event into current binlog and flush and sync */
bool MYSQL_BIN_LOG::write_event_to_binlog_and_sync(Log_event *ev) {
  if (write_event_to_binlog(ev) || m_binlog_file->flush() ||
      m_binlog_file->sync())
    return true;

  update_binlog_end_pos();
  return false;
}

/**
  Write the contents of the statement or transaction cache to the binary log.

  Comparison with do_write_cache:

  - do_write_cache is a lower-level function that only performs the
    actual write.

  - write_cache is a higher-level function that calls do_write_cache
    and additionally performs some maintenance tasks, including:
    - report any errors that occurred
    - write incident event if needed
    - update gtid_state
    - update thd.binlog_next_event_pos

  @param thd Thread variable

  @param cache_data Events will be read from the IO_CACHE of this
  cache_data object.

  @param writer Events will be written to this Binlog_event_writer.

  @retval true IO error.
  @retval false Success.

  @note We only come here if there is something in the cache.
  @note Whatever is in the cache is always a complete transaction.
  @note 'cache' needs to be reinitialized after this functions returns.
*/
bool MYSQL_BIN_LOG::write_cache(THD *thd, binlog_cache_data *cache_data,
                                Binlog_event_writer *writer) {
  DBUG_TRACE;

  Binlog_cache_storage *cache = cache_data->get_cache();
  bool incident = cache_data->has_incident();

  mysql_mutex_assert_owner(&LOCK_log);

  DBUG_ASSERT(is_open());
  if (likely(is_open()))  // Should always be true
  {
    /*
      We only bother to write to the binary log if there is anything
      to write.

      @todo Is this check redundant? Probably this is only called if
      there is anything in the cache (see @note in comment above this
      function). Check if we can replace this by an assertion. /Sven
    */
    if (!cache->is_empty()) {
      DBUG_EXECUTE_IF("crash_before_writing_xid", {
        if (do_write_cache(cache, writer))
          DBUG_PRINT("info", ("error writing binlog cache: %d", write_error));
        flush_and_sync(true);
        DBUG_PRINT("info", ("crashing before writing xid"));
        DBUG_SUICIDE();
      });

      DBUG_EXECUTE_IF("fail_binlog_flush_raft", {
        thd->commit_error = THD::CE_FLUSH_ERROR;
        thd->commit_consensus_error = true;
        goto err;
      });

      if (do_write_cache(cache, writer)) goto err;

      binlog_bytes_written += cache->length();
      const char *err_msg =
          "Non-transactional changes did not get into "
          "the binlog.";
      if (incident &&
          write_incident(thd, false /*need_lock_log=false*/, err_msg,
                         false /*do_flush_and_sync==false*/)) {
        report_binlog_write_error();
        goto err;
      }
      DBUG_EXECUTE_IF("half_binlogged_transaction", DBUG_SUICIDE(););
    }
    update_thd_next_event_pos(thd);
  }

  return false;

err:
  thd->commit_error = THD::CE_FLUSH_ERROR;

  return true;
}

void MYSQL_BIN_LOG::report_binlog_write_error() {
  char errbuf[MYSYS_STRERROR_SIZE];

  write_error = true;
  LogErr(ERROR_LEVEL, ER_FAILED_TO_WRITE_TO_FILE, name, errno,
         my_strerror(errbuf, sizeof(errbuf), errno));
}

/**
  Wait until we get a signal that the binary log has been updated.
  Applies to master only.

  NOTES
  @param[in] timeout    a pointer to a timespec;
                        NULL means to wait w/o timeout.
  @retval    0          if got signalled on update
  @retval    non-0      if wait timeout elapsed
  @note
    LOCK_binlog_end_pos must be taken before calling this function.
    LOCK_binlog_end_pos is being released while the thread is waiting.
    LOCK_binlog_end_pos is released by the caller.
*/

int MYSQL_BIN_LOG::wait_for_update(const struct timespec *timeout) {
  int ret = 0;
  DBUG_TRACE;

  if (!timeout)
    mysql_cond_wait(&update_cond, &LOCK_binlog_end_pos);
  else
    ret = mysql_cond_timedwait(&update_cond, &LOCK_binlog_end_pos,
                               const_cast<struct timespec *>(timeout));
  return ret;
}

/**
  Close the log file.

  @param exiting     Bitmask for one or more of the following bits:
          - LOG_CLOSE_INDEX : if we should close the index file
          - LOG_CLOSE_TO_BE_OPENED : if we intend to call open
                                     at once after close.
          - LOG_CLOSE_STOP_EVENT : write a 'stop' event to the log

  @param need_lock_log If true, this function acquires LOCK_log;
  otherwise the caller should already have acquired it.

  @param need_lock_index If true, this function acquires LOCK_index;
  otherwise the caller should already have acquired it.

  @note
    One can do an open on the object at once after doing a close.
    The internal structures are not freed until cleanup() is called
*/

void MYSQL_BIN_LOG::close(
    uint exiting, bool need_lock_log,
    bool need_lock_index) {  // One can't set log_type here!
  DBUG_TRACE;
  DBUG_PRINT("enter", ("exiting: %d", (int)exiting));
  if (need_lock_log)
    mysql_mutex_lock(&LOCK_log);
  else
    mysql_mutex_assert_owner(&LOCK_log);

  if (atomic_log_state == LOG_OPENED) {
    // In raft mode, we are disabling all STOP_EVENT addition.
    // There are primarily 3 reasons.
    // 1. T65968945 - during shutdown of the server,
    // relay logs and binlogs add STOP EVENTS. In Raft, the
    // Master still has an RLI which holds onto a STALE relay log
    // and adding a STOP EVENT during close will violate the append
    // only rule to the file, because the same relay log has been
    // usurped as a binlog by the master and appended to, changing its size
    // 2. Comments in code have shown us that STOP EVENTs are not
    // critical for relay logs and best effort in general.
    // A server can crash with kill -9 and there wont be any stop event.
    // Raft recovery code handles these scenarios and so STOP_EVENT is
    // still best effort
    // 3. In Raft on crash recovery we open_existing_binlog, which
    // will have issues because the STOP_EVENT will be in the middle of
    // the file and can confuse appliers, when we have to still keep
    // appending new entries beyond it.
    if ((exiting & LOG_CLOSE_STOP_EVENT) != 0 && !enable_raft_plugin) {
      /**
        TODO(WL#7546): Change the implementation to Stop_event after write() is
        moved into libbinlogevents
      */
      Stop_log_event s;
      // the checksumming rule for relay-log case is similar to Rotate
      s.common_footer->checksum_alg =
          is_relay_log
              ? relay_log_checksum_alg
              : static_cast<enum_binlog_checksum_alg>(binlog_checksum_options);
      DBUG_ASSERT(!is_relay_log || relay_log_checksum_alg !=
                                       binary_log::BINLOG_CHECKSUM_ALG_UNDEF);
      if (!write_event_to_binlog(&s) && !m_binlog_file->flush())
        update_binlog_end_pos();
    }

    /* The following update should not be done in relay log files */
    if (!is_relay_log) {
      my_off_t offset = BIN_LOG_HEADER_SIZE + FLAGS_OFFSET;
      uchar flags = 0;  // clearing LOG_EVENT_BINLOG_IN_USE_F
      (void)m_binlog_file->update(&flags, 1, offset);
    }

    if (m_binlog_file->flush_and_sync() && !write_error) {
      report_binlog_write_error();
    }

    /*
      LOCK_sync to guarantee that no thread is calling m_binlog_file
      to sync data to disk when another thread is closing m_binlog_file.
    */
    if (!is_relay_log) mysql_mutex_lock(&LOCK_sync);
    m_binlog_file->close();
    if (!is_relay_log) mysql_mutex_unlock(&LOCK_sync);

    atomic_log_state =
        (exiting & LOG_CLOSE_TO_BE_OPENED) ? LOG_TO_BE_OPENED : LOG_CLOSED;
    my_free(name);
    name = nullptr;
  }

  /*
    The following test is needed even if is_open() is not set, as we may have
    called a not complete close earlier and the index file is still open.
  */

  if (need_lock_index)
    mysql_mutex_lock(&LOCK_index);
  else
    mysql_mutex_assert_owner(&LOCK_index);

  if ((exiting & LOG_CLOSE_INDEX) && my_b_inited(&index_file)) {
    end_io_cache(&index_file);
    if (mysql_file_close(index_file.file, MYF(0)) < 0 && !write_error) {
      report_binlog_write_error();
    }
  }

  if (need_lock_index) mysql_mutex_unlock(&LOCK_index);

  atomic_log_state =
      (exiting & LOG_CLOSE_TO_BE_OPENED) ? LOG_TO_BE_OPENED : LOG_CLOSED;
  my_free(name);
  name = nullptr;

  if (need_lock_log) mysql_mutex_unlock(&LOCK_log);
}

void MYSQL_BIN_LOG::harvest_bytes_written(Relay_log_info *rli,
                                          bool need_log_space_lock) {
#ifndef DBUG_OFF
  char buf1[22], buf2[22];
#endif

  DBUG_TRACE;
  if (need_log_space_lock)
    mysql_mutex_lock(&rli->log_space_lock);
  else
    mysql_mutex_assert_owner(&rli->log_space_lock);
  rli->log_space_total += bytes_written;
  DBUG_PRINT("info",
             ("relay_log_space: %s  bytes_written: %s",
              llstr(rli->log_space_total, buf1), llstr(bytes_written, buf2)));
  bytes_written = 0;
  if (need_log_space_lock) mysql_mutex_unlock(&rli->log_space_lock);
}

void MYSQL_BIN_LOG::set_max_size(ulong max_size_arg) {
  /*
    We need to take locks, otherwise this may happen:
    new_file() is called, calls open(old_max_size), then before open() starts,
    set_max_size() sets max_size to max_size_arg, then open() starts and
    uses the old_max_size argument, so max_size_arg has been overwritten and
    it's like if the SET command was never run.
  */
  DBUG_TRACE;
  mysql_mutex_lock(&LOCK_log);
  if (is_open()) max_size = max_size_arg;
  mysql_mutex_unlock(&LOCK_log);
}

/****** transaction coordinator log for 2pc - binlog() based solution ******/

/**
  @todo
  keep in-memory list of prepared transactions
  (add to list in log(), remove on unlog())
  and copy it to the new binlog if rotated
  but let's check the behaviour of tc_log_page_waits first!
*/

int MYSQL_BIN_LOG::open_binlog(const char *opt_name) {
  LOG_INFO log_info;
  int error = 1;

  /*
    This function is used for 2pc transaction coordination.  Hence, it
    is never used for relay logs.
  */
  DBUG_ASSERT(!is_relay_log);
  DBUG_ASSERT(total_ha_2pc > 1 || (1 == total_ha_2pc && opt_bin_log));
  DBUG_ASSERT(opt_name && opt_name[0]);

  if (!my_b_inited(&index_file)) {
    /* There was a failure to open the index file, can't open the binlog */
    cleanup();
    return 1;
  }

  if (using_heuristic_recover()) {
    /* generate a new binlog to mask a corrupted one */
    mysql_mutex_lock(&LOCK_log);
    open_binlog(opt_name, nullptr, max_binlog_size, false,
                true /*need_lock_index=true*/, true /*need_sid_lock=true*/,
                nullptr);
    mysql_mutex_unlock(&LOCK_log);
    cleanup();
    return 1;
  }

  if ((error = find_log_pos(&log_info, NullS, true /*need_lock_index=true*/))) {
    if (error != LOG_INFO_EOF)
      LogErr(ERROR_LEVEL, ER_BINLOG_CANT_FIND_LOG_IN_INDEX, error);
    else

    {
      open_binlog_found = false;
      error = 0;
    }
    goto err;
  }

  {
    Log_event *ev = nullptr;
    char log_name[FN_REFLEN];
    my_off_t valid_pos = 0;
    my_off_t binlog_size = 0;

    do {
      strmake(log_name, log_info.log_file_name, sizeof(log_name) - 1);
    } while (
        !(error = find_next_log(&log_info, true /*need_lock_index=true*/)));

    if (error != LOG_INFO_EOF) {
      LogErr(ERROR_LEVEL, ER_BINLOG_CANT_FIND_LOG_IN_INDEX, error);
      goto err;
    }

    Binlog_file_reader binlog_file_reader(opt_master_verify_checksum);
    if (binlog_file_reader.open(log_name)) {
      LogErr(ERROR_LEVEL, ER_BINLOG_FILE_OPEN_FAILED,
             binlog_file_reader.get_error_str());
      goto err;
    }

    // We found atleast one binlog file in the binlog index file
    // Note that this is useful only when raft is enabled - even during clean
    // shutdown, raft plugin can rollback last batch of trxs from the engine
    // (after writing to binlog). hence on restart, this binlog file can have
    // uncommitted trxs and should not be marked as 'cleanly closed'. In other
    // words, 'LOG_EVENT_BINLOG_IN_USE_F' is not a reliable indicator anymore
    // that a binlog file does not contain uncommitted trxs.
    open_binlog_found = true;

    /*
      If the binary log was not properly closed it means that the server
      may have crashed. In that case, we need to call
      MYSQL_BIN_LOG::binlog_recover
      to:

        a) collect logged XIDs;
        b) complete the 2PC of the pending XIDs;
        c) collect the last valid position.

      Therefore, we do need to iterate over the binary log, even if
      total_ha_2pc == 1, to find the last valid group of events written.
      Later we will take this value and truncate the log if need be.
    */
    if ((ev = binlog_file_reader.read_event_object()) &&
        ev->get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT &&
        (ev->common_header->flags & LOG_EVENT_BINLOG_IN_USE_F ||
         DBUG_EVALUATE_IF("eval_force_bin_log_recovery", true, false))) {
      LogErr(INFORMATION_LEVEL, ER_BINLOG_RECOVERING_AFTER_CRASH_USING,
             opt_name);
      valid_pos = binlog_file_reader.position();
      // Get the raw filename without dirname
      const std::string cur_log_file = log_name + dirname_length(log_name);
      error = binlog_recover(&binlog_file_reader, &valid_pos,
                             &engine_binlog_max_gtid, engine_binlog_file,
                             &engine_binlog_pos, cur_log_file,
                             &this->first_gtid_start_pos);
      binlog_size = binlog_file_reader.ifile()->length();
    } else {
      /*
       * If we are here, it implies either mysqld was shutdown cleanly or
       * it was killed during binlog rotation where old binlog file was
       * closed cleanly but new binlog file was not created. In the later case,
       * the storage engine recovery must be triggered so that engine's binlog
       * coordinates (engine_binlog_file and engine_binlog_pos) are updated
       * properly.
       *
       * Note we don't need binlog recovery here since it was closed cleanly.
       * Since recovery in fb-mysql works assuming storage engine as source
       * of truth, it doesn't need the list of xids to recover.
       * We will update binlog state (GTID_SET) based on the storage engine
       * coordinates in init_slave().
       */
      int memory_page_size = my_getpagesize();
      char tmp_binlog_file[FN_REFLEN + 1] = {0};
      my_off_t tmp_binlog_pos = 0;
      MEM_ROOT mem_root(key_memory_binlog_recover_exec, memory_page_size);
      xid_to_gtid_container xids(&mem_root);
      /*
        Temp variables help trigger fetching the file/offset info even
        during a clean recovery.
      */
      error = ha_recover(&xids, &engine_binlog_max_gtid, tmp_binlog_file,
                         &tmp_binlog_pos);
    }

    delete ev;

    if (error) goto err;

    /* Trim the crashed binlog file to last valid transaction
      or event (non-transaction) base on valid_pos. */
    if (valid_pos > 0) {
      std::unique_ptr<Binlog_ofile> ofile(
          Binlog_ofile::open_existing(key_file_binlog, log_name, MYF(MY_WME)));

      if (!ofile) {
        LogErr(ERROR_LEVEL, ER_BINLOG_CANT_OPEN_CRASHED_BINLOG);
        return -1;
      }

      /* Change binlog file size to valid_pos */
      if (valid_pos < binlog_size) {
        if (opt_trim_binlog) {
          char backup_file[FN_REFLEN];
          myf opt = MY_REPLACE_DIR | MY_UNPACK_FILENAME | MY_APPEND_EXT;
          fn_format(backup_file, "binlog_backup", opt_mysql_tmpdir, ".trunc",
                    opt);

          // NO_LINT_DEBUG
          sql_print_error("Taking backup from %s to %s\n", log_name,
                          backup_file);
          /* MY_HOLD_ORIGINAL_MODES prevents attempts to chown the file */
          if (my_copy(log_name, backup_file,
                      MYF(MY_WME | MY_HOLD_ORIGINAL_MODES))) {
            // NO_LINT_DEBUG
            sql_print_error(
                "Could not take backup of the truncated binlog file %s",
                log_name);
          }
        }

        if (ofile->truncate(valid_pos)) {
          LogErr(ERROR_LEVEL, ER_BINLOG_CANT_TRIM_CRASHED_BINLOG);
          return -1;
        }
        LogErr(INFORMATION_LEVEL, ER_BINLOG_CRASHED_BINLOG_TRIMMED, log_name,
               binlog_size, valid_pos, valid_pos);
      }

      /* If raft plugin is not enabled, then clear the 'file-in-use' flag.
       * If raft plugin is in use, then the file gets recovered only later when
       * the node joins the ring at which point it might do additional trimming
       * of the last binlog file in use. */
      if (!enable_raft_plugin) {
        /* Clear LOG_EVENT_BINLOG_IN_USE_F */
        uchar flags = 0;
        if (ofile->update(&flags, 1, BIN_LOG_HEADER_SIZE + FLAGS_OFFSET)) {
          LogErr(ERROR_LEVEL,
                 ER_BINLOG_CANT_CLEAR_IN_USE_FLAG_FOR_CRASHED_BINLOG);
          return -1;
        }
      }
    }  // end if (valid_pos > 0)
  }

err:
  return error;
}

/**
 Truncate the active relay log file in the specified position.

  @param mi Master_info of the channel going to truncate the relay log file.
  @param truncate_pos The position to truncate the active relay log file.
  @return False on success and true on failure.
*/
bool MYSQL_BIN_LOG::truncate_relaylog_file(Master_info *mi,
                                           my_off_t truncate_pos) {
  DBUG_TRACE;
  DBUG_ASSERT(is_relay_log);
  mysql_mutex_assert_owner(&LOCK_log);
  Relay_log_info *rli = mi->rli;
  bool error = false;

  /*
    If the relay log was closed by an error (binlog_error_action=IGNORE_ERROR)
    this truncate function should produce no result as the relay log is already
    in really bad shape.
  */
  if (!is_open()) {
    return false;
  }

  my_off_t relaylog_file_size = m_binlog_file->position();

  if (truncate_pos > 0 && truncate_pos < relaylog_file_size) {
    if (m_binlog_file->truncate(truncate_pos)) {
      mi->report(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_WRITE_FAILURE,
                 ER_THD(current_thd, ER_SLAVE_RELAY_LOG_WRITE_FAILURE),
                 "failed to truncate relay log file");
      error = true;
    } else {
      LogErr(INFORMATION_LEVEL, ER_SLAVE_RELAY_LOG_TRUNCATE_INFO, log_file_name,
             relaylog_file_size, truncate_pos);

      // Re-init the SQL thread IO_CACHE
      DBUG_ASSERT(strcmp(rli->get_event_relay_log_name(), log_file_name) ||
                  rli->get_event_relay_log_pos() <= truncate_pos);
      rli->notify_relay_log_truncated();
    }
  }
  return error;
}

/** This is called on shutdown, after ha_panic. */
void MYSQL_BIN_LOG::close() {}

/*
  Prepare the transaction in the transaction coordinator.

  This function will prepare the transaction in the storage engines
  (by calling @c ha_prepare_low) what will write a prepare record
  to the log buffers.

  @retval 0    success
  @retval 1    error
*/
int MYSQL_BIN_LOG::prepare(THD *thd, bool all) {
  DBUG_TRACE;

  DBUG_ASSERT(opt_bin_log);
  /*
    The applier thread explicitly overrides the value of sql_log_bin
    with the value of log_slave_updates.
  */
  DBUG_ASSERT(thd->slave_thread ? opt_log_slave_updates
                                : thd->variables.sql_log_bin);

  /*
    Set HA_IGNORE_DURABILITY to not flush the prepared record of the
    transaction to the log of storage engine (for example, InnoDB
    redo log) during the prepare phase. So that we can flush prepared
    records of transactions to the log of storage engine in a group
    right before flushing them to binary log during binlog group
    commit flush stage. Reset to HA_REGULAR_DURABILITY at the
    beginning of parsing next command.
  */
  thd->durability_property = HA_IGNORE_DURABILITY;

  int error = ha_prepare_low(thd, all);

  return error;
}

/**
  Commit the transaction in the transaction coordinator.

  This function will commit the sessions transaction in the binary log
  and in the storage engines (by calling @c ha_commit_low). If the
  transaction was successfully logged (or not successfully unlogged)
  but the commit in the engines did not succed, there is a risk of
  inconsistency between the engines and the binary log.

  For binary log group commit, the commit is separated into three
  parts:

  1. First part consists of filling the necessary caches and
     finalizing them (if they need to be finalized). After this,
     nothing is added to any of the caches.

  2. Second part execute an ordered flush and commit. This will be
     done using the group commit functionality in ordered_commit.

  3. Third part checks any errors resulting from the ordered commit
     and handles them appropriately.

  @retval RESULT_SUCCESS   success
  @retval RESULT_ABORTED   error, transaction was neither logged nor committed
  @retval RESULT_INCONSISTENT  error, transaction was logged but not committed
*/
TC_LOG::enum_result MYSQL_BIN_LOG::commit(THD *thd, bool all) {
  DBUG_TRACE;
  DBUG_PRINT("info",
             ("query='%s'", thd == current_thd ? thd->query().str : nullptr));
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
  Transaction_ctx *trn_ctx = thd->get_transaction();
  my_xid xid = trn_ctx->xid_state()->get_xid()->get_my_xid();
  bool stmt_stuff_logged = false;
  bool trx_stuff_logged = false;
  bool skip_commit = is_loggable_xa_prepare(thd);
  bool is_atomic_ddl = false;

  DBUG_PRINT("enter", ("thd: 0x%llx, all: %s, xid: %llu, cache_mngr: 0x%llx",
                       (ulonglong)thd, YESNO(all), (ulonglong)xid,
                       (ulonglong)cache_mngr));

  /*
    No cache manager means nothing to log, but we still have to commit
    the transaction.
   */
  if (cache_mngr == nullptr) {
    if (!skip_commit && ha_commit_low(thd, all)) return RESULT_ABORTED;
    return RESULT_SUCCESS;
  }

  Transaction_ctx::enum_trx_scope trx_scope =
      all ? Transaction_ctx::SESSION : Transaction_ctx::STMT;

  DBUG_PRINT("debug", ("in_transaction: %s, no_2pc: %s, rw_ha_count: %d",
                       YESNO(thd->in_multi_stmt_transaction_mode()),
                       YESNO(trn_ctx->no_2pc(trx_scope)),
                       trn_ctx->rw_ha_count(trx_scope)));
  DBUG_PRINT("debug",
             ("all.cannot_safely_rollback(): %s, trx_cache_empty: %s",
              YESNO(trn_ctx->cannot_safely_rollback(Transaction_ctx::SESSION)),
              YESNO(cache_mngr->trx_cache.is_binlog_empty())));
  DBUG_PRINT("debug",
             ("stmt.cannot_safely_rollback(): %s, stmt_cache_empty: %s",
              YESNO(trn_ctx->cannot_safely_rollback(Transaction_ctx::STMT)),
              YESNO(cache_mngr->stmt_cache.is_binlog_empty())));

  /*
    If there are no handlertons registered, there is nothing to
    commit. Note that DDLs are written earlier in this case (inside
    binlog_query).

    TODO: This can be a problem in those cases that there are no
    handlertons registered. DDLs are one example, but the other case
    is MyISAM. In this case, we could register a dummy handlerton to
    trigger the commit.

    Any statement that requires logging will call binlog_query before
    trans_commit_stmt, so an alternative is to use the condition
    "binlog_query called or stmt.ha_list != 0".
   */
  if (!all && !trn_ctx->is_active(trx_scope) &&
      cache_mngr->stmt_cache.is_binlog_empty())
    return RESULT_SUCCESS;

  if (thd->lex->sql_command == SQLCOM_XA_COMMIT) {
    /* The Commit phase of the XA two phase logging. */

#ifndef DBUG_OFF
    bool one_phase = get_xa_opt(thd) == XA_ONE_PHASE;
    DBUG_ASSERT(all || (thd->slave_thread && one_phase));
    DBUG_ASSERT(!skip_commit || one_phase);
#endif

    XID_STATE *xs = thd->get_transaction()->xid_state();
    if (DBUG_EVALUATE_IF(
            "simulate_xa_commit_log_failure", true,
            do_binlog_xa_commit_rollback(thd, xs->get_xid(), true)))
      return RESULT_ABORTED;
  }

  if (!cache_mngr->stmt_cache.is_binlog_empty()) {
    /*
      Commit parent identification of non-transactional query has
      been deferred until now, except for the mixed transaction case.
    */
    trn_ctx->store_commit_parent(
        m_dependency_tracker.get_max_committed_timestamp());
    if (cache_mngr->stmt_cache.finalize(thd)) return RESULT_ABORTED;
    stmt_stuff_logged = true;
  }

  /*
    We commit the transaction if:
     - We are not in a transaction and committing a statement, or
     - We are in a transaction and a full transaction is committed.
    Otherwise, we accumulate the changes.
  */
  if (!cache_mngr->trx_cache.is_binlog_empty() && ending_trans(thd, all) &&
      !trx_stuff_logged) {
    const bool real_trans =
        (all || !trn_ctx->is_active(Transaction_ctx::SESSION));

    bool one_phase = get_xa_opt(thd) == XA_ONE_PHASE;
    bool is_loggable_xa = is_loggable_xa_prepare(thd);
    XID_STATE *xs = thd->get_transaction()->xid_state();

    /*
      Log and finalize transaction cache regarding XA PREPARE/XA COMMIT ONE
      PHASE if one of the following statements is true:
      - If it is a loggable XA transaction in prepare state;
      - If it is a transaction being commited with 'XA COMMIT ONE PHASE',
      statement and is not an empty transaction when GTID_NEXT is set to a
      manual GTID.

      For other XA COMMIT ONE PHASE statements that already have been finalized
      or are finalizing empty transactions when GTID_NEXT is set to a manual
      GTID, just let the execution flow get into the final 'else' branch and log
      a final 'COMMIT;' statement.
    */
    if (is_loggable_xa ||  // XA transaction in prepare state
        (thd->lex->sql_command == SQLCOM_XA_COMMIT &&  // Is a 'XA COMMIT
         one_phase &&                                  // ONE PHASE'
         xs != nullptr &&                              // and it has not yet
         !xs->is_binlogged() &&                        // been logged
         (thd->owned_gtid.sidno <= 0 ||  // and GTID_NEXT is NOT set to a
                                         // manual GTID
          !xs->has_state(XID_STATE::XA_NOTR))))  // and the transaction is NOT
                                                 // empty and NOT finalized in
                                                 // 'trans_xa_commit'
    {
      /* The prepare phase of XA transaction two phase logging. */
      int err = 0;

      DBUG_ASSERT(thd->lex->sql_command != SQLCOM_XA_COMMIT || one_phase);

      XA_prepare_log_event end_evt(thd, xs->get_xid(), one_phase);

      DBUG_ASSERT(!is_loggable_xa || skip_commit);

      err = cache_mngr->trx_cache.finalize(thd, &end_evt, xs);
      if (err) return RESULT_ABORTED;
      if (is_loggable_xa)
        if (DBUG_EVALUATE_IF("simulate_xa_prepare_failure_in_cache_finalize",
                             true, false))
          return RESULT_ABORTED;
    }
    /*
      If is atomic DDL, finalize cache for DDL and no further logging is needed.
    */
    else if ((is_atomic_ddl = cache_mngr->trx_cache.has_xid())) {
      if (cache_mngr->trx_cache.finalize(thd, nullptr)) return RESULT_ABORTED;
    }
    /*
      We are committing a 2PC transaction if it is a "real" transaction
      and has an XID assigned (because some handlerton registered). A
      transaction is "real" if either 'all' is true or
      'trn_ctx->is_active(Transaction_ctx::SESSION)' is not true.

      Note: This is kind of strange since registering the binlog
      handlerton will then make the transaction 2PC, which is not really
      true. This occurs for example if a MyISAM statement is executed
      with row-based replication on.
    */
    else if (real_trans && xid && trn_ctx->rw_ha_count(trx_scope) > 1 &&
             !trn_ctx->no_2pc(trx_scope)) {
      Xid_log_event end_evt(thd, xid);
      if (cache_mngr->trx_cache.finalize(thd, &end_evt)) return RESULT_ABORTED;
    }
    /*
      No further action needed and no special case applies, log a final
      'COMMIT' statement and finalize the transaction cache.

      Empty transactions finalized with 'XA COMMIT ONE PHASE' will be covered
      by this branch.
     */
    else {
      Query_log_event end_evt(thd, STRING_WITH_LEN("COMMIT"), true, false, true,
                              0, true);
      if (cache_mngr->trx_cache.finalize(thd, &end_evt)) return RESULT_ABORTED;
    }
    trx_stuff_logged = true;
  }

  /*
    This is part of the stmt rollback.
  */
  if (!all) cache_mngr->trx_cache.set_prev_position(MY_OFF_T_UNDEF);

  /*
    Now all the events are written to the caches, so we will commit
    the transaction in the engines. This is done using the group
    commit logic in ordered_commit, which will return when the
    transaction is committed.

    If the commit in the engines fail, we still have something logged
    to the binary log so we have to report this as a "bad" failure
    (failed to commit, but logged something).
  */
  if (stmt_stuff_logged || trx_stuff_logged) {
    if (RUN_HOOK(
            transaction, before_commit,
            (thd, all, thd_get_cache_mngr(thd)->get_trx_cache(),
             thd_get_cache_mngr(thd)->get_stmt_cache(),
             max<my_off_t>(max_binlog_cache_size, max_binlog_stmt_cache_size),
             is_atomic_ddl)) ||
        DBUG_EVALUATE_IF("simulate_failure_in_before_commit_hook", true,
                         false)) {
      ha_rollback_low(thd, all);
      gtid_state->update_on_rollback(thd);
      thd_get_cache_mngr(thd)->reset();
      // Reset the thread OK status before changing the outcome.
      if (thd->get_stmt_da()->is_ok())
        thd->get_stmt_da()->reset_diagnostics_area();
      my_error(ER_RUN_HOOK_ERROR, MYF(0), "before_commit");
      return RESULT_ABORTED;
    }
    /*
      Check whether the transaction should commit or abort given the
      plugin feedback.
    */
    if (thd->get_transaction()
            ->get_rpl_transaction_ctx()
            ->is_transaction_rollback() ||
        (DBUG_EVALUATE_IF("simulate_transaction_rollback_request", true,
                          false))) {
      ha_rollback_low(thd, all);
      gtid_state->update_on_rollback(thd);
      thd_get_cache_mngr(thd)->reset();
      if (thd->get_stmt_da()->is_ok())
        thd->get_stmt_da()->reset_diagnostics_area();
      my_error(ER_TRANSACTION_ROLLBACK_DURING_COMMIT, MYF(0));
      return RESULT_ABORTED;
    }

    if (check_hlc_bound(thd)) {
      return RESULT_ABORTED;
    }

    if (ordered_commit(thd, all, skip_commit)) return RESULT_INCONSISTENT;

    DBUG_EXECUTE_IF("ensure_binlog_cache_is_reset", {
      /* Assert that binlog cache is reset at commit time. */
      DBUG_ASSERT(binlog_cache_is_reset);
      binlog_cache_is_reset = false;
    };);

    /*
      Mark the flag m_is_binlogged to true only after we are done
      with checking all the error cases.
    */
    if (is_loggable_xa_prepare(thd)) {
      thd->get_transaction()->xid_state()->set_binlogged();
      /*
        Inform hook listeners that a XA PREPARE did commit, that
        is, did log a transaction to the binary log.
      */
      // semi-sync plugin only called when raft is not enabled
      if (!enable_raft_plugin)
        (void)RUN_HOOK(transaction, after_commit, (thd, all));
    }
  } else if (!skip_commit) {
    /*
      We only set engine binlog position in ordered_commit path flush phase
      and not all transactions go through them (such as table copy in DDL).
      So in cases where a DDL statement implicitly commits earlier transaction
      and starting a new one, the new transaction could be "leaking" the
      engine binlog pos. In order to avoid that and accidentally overwrite
      binlog position with previous location, we reset it here.
    */
    thd->set_trans_pos(NULL, 0);
    if (ha_commit_low(thd, all)) return RESULT_INCONSISTENT;
  }

  return RESULT_SUCCESS;
}

/**
   Flush caches for session.

   @note @c set_trans_pos is called with a pointer to the file name
   that the binary log currently use and a rotation will change the
   contents of the variable.

   The position is used when calling the after_flush, after_commit,
   and after_rollback hooks, but these have been placed so that they
   occur before a rotation is executed.

   It is the responsibility of any plugin that use this position to
   copy it if they need it after the hook has returned.

   The current "global" transaction_counter is stepped and its new value
   is assigned to the transaction.
 */
std::pair<int, my_off_t> MYSQL_BIN_LOG::flush_thread_caches(THD *thd) {
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
  my_off_t bytes = 0;
  bool wrote_xid = false;
  int error = cache_mngr->flush(thd, &bytes, &wrote_xid);
  if (!error && bytes > 0) {
    /*
      Note that set_trans_pos does not copy the file name. See
      this function documentation for more info.
    */
    if (enable_raft_plugin && !is_apply_log)
      thd->set_trans_pos(log_file_name, atomic_binlog_end_pos.load());
    else
      thd->set_trans_pos(log_file_name, m_binlog_file->position());
    if (wrote_xid)
      inc_prep_xids(thd);
    else
      inc_non_xid_trxs(thd);
  }
  DBUG_PRINT("debug", ("bytes: %llu", bytes));
  return std::make_pair(error, bytes);
}

void MYSQL_BIN_LOG::init_thd_variables(THD *thd, bool all, bool skip_commit) {
  /*
    These values are used while committing a transaction, so clear
    everything.

    Notes:

    - It would be good if we could keep transaction coordinator
      log-specific data out of the THD structure, but that is not the
      case right now.

    - Everything in the transaction structure is reset when calling
      ha_commit_low since that calls Transaction_ctx::cleanup.
  */
  thd->tx_commit_pending = true;
  thd->commit_error = THD::CE_NONE;
  thd->next_to_commit = nullptr;
  thd->durability_property = HA_IGNORE_DURABILITY;
  thd->get_transaction()->m_flags.real_commit = all;
  thd->get_transaction()->m_flags.xid_written = false;
  thd->get_transaction()->m_flags.commit_low = !skip_commit;
  thd->get_transaction()->m_flags.run_hooks = !skip_commit;
#ifndef DBUG_OFF
  /*
     The group commit Leader may have to wait for follower whose transaction
     is not ready to be preempted. Initially the status is pessimistic.
     Preemption guarding logics is necessary only when !DBUG_OFF is set.
     It won't be required for the dbug-off case as long as the follower won't
     execute any thread-specific write access code in this method, which is
     the case as of current.
  */
  thd->get_transaction()->m_flags.ready_preempt = 0;
#endif
}

THD *MYSQL_BIN_LOG::fetch_and_process_flush_stage_queue(
    const bool check_and_skip_flush_logs) {
  /*
    Fetch the entire flush queue and empty it, so that the next batch
    has a leader. We must do this before invoking ha_flush_logs(...)
    for guaranteeing to flush prepared records of transactions before
    flushing them to binary log, which is required by crash recovery.
  */
  Commit_stage_manager::get_instance().lock_queue(
      Commit_stage_manager::BINLOG_FLUSH_STAGE);

  THD *first_seen =
      Commit_stage_manager::get_instance().fetch_queue_skip_acquire_lock(
          Commit_stage_manager::BINLOG_FLUSH_STAGE);
  DBUG_ASSERT(first_seen != nullptr);

  THD *commit_order_thd =
      Commit_stage_manager::get_instance().fetch_queue_skip_acquire_lock(
          Commit_stage_manager::COMMIT_ORDER_FLUSH_STAGE);

  Commit_stage_manager::get_instance().unlock_queue(
      Commit_stage_manager::BINLOG_FLUSH_STAGE);

  if (!check_and_skip_flush_logs ||
      (check_and_skip_flush_logs && commit_order_thd != nullptr)) {
    /*
      We flush prepared records of transactions to the log of storage
      engine (for example, InnoDB redo log) in a group right before
      flushing them to binary log.
    */
    ha_flush_logs(true);
  }

  /*
    The transactions are flushed to the disk and so threads
    executing slave preserve commit order can be unblocked.
  */
  Commit_stage_manager::get_instance()
      .process_final_stage_for_ordered_commit_group(commit_order_thd);
  return first_seen;
}

int MYSQL_BIN_LOG::process_flush_stage_queue(my_off_t *total_bytes_var,
                                             bool *rotate_var,
                                             THD **out_queue_var) {
  DBUG_TRACE;
#ifndef DBUG_OFF
  // number of flushes per group.
  int no_flushes = 0;
#endif
  DBUG_ASSERT(total_bytes_var && rotate_var && out_queue_var);
  my_off_t total_bytes = 0;
  int flush_error = 1;
  int commit_consensus_error = 0;
  mysql_mutex_assert_owner(&LOCK_log);

  THD *first_seen = fetch_and_process_flush_stage_queue();
  DBUG_EXECUTE_IF("crash_after_flush_engine_log", DBUG_SUICIDE(););
  assign_automatic_gtids_to_flush_group(first_seen);

  ulonglong thd_count = 0;
  /* Flush thread caches to binary log. */
  for (THD *head = first_seen; head; head = head->next_to_commit) {
    head->commit_consensus_error = 0;
    std::pair<int, my_off_t> result = flush_thread_caches(head);

    total_bytes += result.second;
    if (flush_error == 1) flush_error = result.first;
#ifndef DBUG_OFF
    no_flushes++;
#endif

    /* There is a weird check above that if first thread in the group could
     * flush successfully, then every thread in the group will also flush
     * successfully. This does not look right - so for the time being, we will
     * use commit_consensus_error flag to identify if there was a error in
     * before_flush hook of raft plugin and set commit_consensus_error here.
     * This will subsequently fail the entire group
     */
    if (head->commit_consensus_error) commit_consensus_error = 1;

    ++thd_count;
  }

  DBUG_ASSERT(thd_count > 0);
  DBUG_PRINT("info", ("Number of threads in group commit %llu", thd_count));
  counter_histogram_increment(&histogram_binlog_group_commit, thd_count);

  *out_queue_var = first_seen;
  *total_bytes_var = total_bytes;
  if (total_bytes > 0 &&
      (m_binlog_file->get_real_file_size() >= (my_off_t)max_size ||
       DBUG_EVALUATE_IF("simulate_max_binlog_size", true, false)))
    *rotate_var = true;

  if (commit_consensus_error && enable_raft_plugin) flush_error = 1;

#ifndef DBUG_OFF
  DBUG_PRINT("info", ("no_flushes:= %d", no_flushes));
  no_flushes = 0;
#endif
  return flush_error;
}

/**
  Waits for consensus commit (before proceeding to engine commit). Invokes
  raft replication plugin's before_commit hook

  @param queue_head  Head of the commit stage queue.
*/
void MYSQL_BIN_LOG::process_consensus_queue(THD *queue_head) {
  DBUG_EXECUTE_IF("simulate_before_commit_error", {
    set_commit_consensus_error(queue_head);
    return;
  });

  if (!enable_raft_plugin) return;

  DBUG_EXECUTE_IF("before_before_commit", {
    const char act[] =
        "now signal reached "
        "wait_for continue";
    DBUG_ASSERT(opt_debug_sync_timeout > 0);
    DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
  });

  THD *last_thd = NULL;
  int error = 0;
  for (THD *thd = queue_head; thd != NULL; thd = thd->next_to_commit)
    if (thd->commit_error == THD::CE_NONE) last_thd = thd;

  if (last_thd) {
    auto start_time = my_timer_now();

    error = RUN_HOOK_STRICT(raft_replication, before_commit, (last_thd));

    if (!this->is_apply_log) {
      auto wait_time = my_timer_since(start_time);
      latency_histogram_increment(&histogram_raft_trx_wait, wait_time, 1);
    }

    if (error) set_commit_consensus_error(queue_head);
    if (!error && opt_raft_signal_async_dump_threads == AFTER_CONSENSUS &&
        enable_raft_plugin && rpl_wait_for_semi_sync_ack) {
      const char *log_file = nullptr;
      my_off_t log_pos = 0;
      if (mysql_bin_log.is_apply_log) {
        last_thd->get_trans_relay_log_pos((const char **)&log_file, &log_pos);
      } else {
        last_thd->get_trans_fixed_pos((const char **)&log_file, &log_pos);
      }
      dump_log.signal_semi_sync_ack(log_file, log_pos);
    }
  }
}

/**
  Commit a sequence of sessions.

  This function commit an entire queue of sessions starting with the
  session in @c first. If there were an error in the flushing part of
  the ordered commit, the error code is passed in and all the threads
  are marked accordingly (but not committed).

  It will also add the GTIDs of the transactions to gtid_executed.

  @see MYSQL_BIN_LOG::ordered_commit

  @param thd The "master" thread
  @param first First thread in the queue of threads to commit
 */

void MYSQL_BIN_LOG::process_commit_stage_queue(THD *thd, THD *first) {
  mysql_mutex_assert_owner(&LOCK_commit);
#ifndef DBUG_OFF
  thd->get_transaction()->m_flags.ready_preempt =
      true;  // formality by the leader
#endif
  THD *candidate = nullptr;
  for (THD *head = first; head; head = head->next_to_commit) {
    DBUG_PRINT("debug", ("Thread ID: %u, commit_error: %d, commit_pending: %s",
                         head->thread_id(), head->commit_error,
                         YESNO(head->tx_commit_pending)));
    DBUG_EXECUTE_IF(
        "block_leader_after_delete",
        if (thd != head) { DBUG_SET("+d,after_delete_wait"); };);
    /*
      If flushing failed, set commit_error for the session, skip the
      transaction and proceed with the next transaction instead. This
      will mark all threads as failed, since the flush failed.

      If flush succeeded, attach to the session and commit it in the
      engines.
    */
#ifndef DBUG_OFF
    Commit_stage_manager::get_instance().clear_preempt_status(head);
#endif
    if (head->get_transaction()->sequence_number != SEQ_UNINIT) {
      mysql_mutex_lock(&LOCK_slave_trans_dep_tracker);
      m_dependency_tracker.update_max_committed(head);
      mysql_mutex_unlock(&LOCK_slave_trans_dep_tracker);
    }
    /*
      Flush/Sync error should be ignored and continue
      to commit phase. And thd->commit_error cannot be
      COMMIT_ERROR at this moment.
    */
    DBUG_ASSERT(head->commit_error != THD::CE_COMMIT_ERROR);
    Thd_backup_and_restore switch_thd(thd, head);
    bool all = head->get_transaction()->m_flags.real_commit;
    if (head->get_transaction()->m_flags.commit_low) {
      /* head is parked to have exited append() */
      DBUG_ASSERT(head->get_transaction()->m_flags.ready_preempt);
      /*
        storage engine commit
       */
      bool error = false;
      if (head->commit_consensus_error) {
        error = true;
        handle_commit_consensus_error(head);
      } else if (ha_commit_low(head, all, false)) {
        error = true;
        head->commit_error = THD::CE_COMMIT_ERROR;
      }

      if (!error) {
        candidate = head;
      }
    }
    DBUG_PRINT("debug", ("commit_error: %d, commit_pending: %s",
                         head->commit_error, YESNO(head->tx_commit_pending)));
  }

  /*
     We only need to use binlog position of the last commit. This is
     because it flushes last and hence will point to last entry in the
     binlog wrt this group commit.
  */
  if (candidate) {
    /*
      Keep SE's binlog positions in sync.
    */
    const char *binlog_file;
    my_off_t binlog_pos;
    const char *max_gtid_var;
    char last_char = ha_last_updated_binlog_file;

    /*
      Here we need to use thd local position info since the global one isn't
      safe - LOCK_log is not held at this point and its possible other threads
      are updating global binlog position by 'FLUSH LOGS' at the same time
    */
    thd_binlog_pos(candidate, &binlog_file, &binlog_pos, nullptr,
                   &max_gtid_var);

    /* Compare the last character of the filename to detect log rotation */
    if (binlog_file) {
      int len = strlen(binlog_file);
      if (len > 0) last_char = binlog_file[len - 1];
    }

    /*
     This is just a partial check (offset comparison without
     checking the log file name) mainly as an optimization to
     avoid the file name comparison. Two cases are handled here:
     Case 1. When binlog rotates, the new position can be smaller.
     Case 2. Normal case when the new offset has passed the threshold.
     NOTE that the check here is not accurate - we could miss the case
     where the file gets rotated and the new offset is larger than the
     last pushed offset (based on the previous file), which could happen
     for long running/large transactions. Since this case is rare and
     it doesn't have to be updated exactly per 'threshold', we're ok with
     ignoring this case.
    */
    if (binlog_pos >=
            (ha_last_updated_binlog_pos + update_binlog_pos_threshold) ||
        binlog_pos < ha_last_updated_binlog_pos ||
        ha_last_updated_binlog_file != last_char) {
      Gtid max_gtid{0, 0};
      if (max_gtid_var != nullptr) {
        global_sid_lock->rdlock();
        max_gtid.parse(global_sid_map, max_gtid_var);
        global_sid_lock->unlock();
      }

      if (!ha_update_binlog_pos(binlog_file, binlog_pos, &max_gtid)) {
        ha_last_updated_binlog_pos = binlog_pos;
        ha_last_updated_binlog_file = last_char;
      }
    }
  }

  /*
    Handle the GTID of the threads.
    gtid_executed table is kept updated even though transactions fail to be
    logged. That's required by slave auto positioning.
  */
  gtid_state->update_commit_group(first);

  for (THD *head = first; head; head = head->next_to_commit) {
    /*
      Decrement the prepared XID counter after storage engine commit.
      We also need decrement the prepared XID when encountering a
      flush error or session attach error for avoiding 3-way deadlock
      among user thread, rotate thread and dump thread.
    */
    if (head->get_transaction()->m_flags.xid_written)
      dec_prep_xids(head);
    else if (head->non_xid_trx)
      dec_non_xid_trxs(head);
  }
}

/**
  Process after commit for a sequence of sessions.

  @param thd The "master" thread
  @param first First thread in the queue of threads to commit
 */

void MYSQL_BIN_LOG::process_after_commit_stage_queue(THD *thd, THD *first) {
  int error = 0;
  THD *last_thd = nullptr;
  for (THD *head = first; head; head = head->next_to_commit) {
    if (enable_binlog_hlc && maintain_database_hlc && head->hlc_time_ns_next) {
      if (enable_binlog_hlc && maintain_database_hlc &&
          head->hlc_time_ns_next) {
        if (likely(!head->databases.empty())) {
          // Successfully committed the trx to engine. Update applied hlc for
          // all databases that this trx touches
          hlc.update_database_hlc(head->databases, head->hlc_time_ns_next);
        } else if (log_error_verbosity >= 4) {
          // Log a error line if databases are empty. This could happen in SBR
          // and for blackhole engine statements
          // NO_LINT_DEBUG
          sql_print_error("Databases were empty for this trx. HLC= %lu",
                          head->hlc_time_ns_next);
        }
      }
    }
    head->databases.clear();

    if (head->get_transaction()->m_flags.run_hooks &&
        head->commit_error != THD::CE_COMMIT_ERROR &&
        !head->commit_consensus_error) {
      /*
        TODO: This hook here should probably move outside/below this
              if and be the only after_commit invocation left in the
              code.
      */
      Thd_backup_and_restore switch_thd(thd, head);
      bool all = head->get_transaction()->m_flags.real_commit;

      // Call semi-sync plugin only when raft is not enabled
      if (!enable_raft_plugin)
        error = error || RUN_HOOK(transaction, after_commit, (head, all));
      else
        error =
            error || RUN_HOOK_STRICT(raft_replication, after_commit, (head));

      if (!enable_raft_plugin) {
        my_off_t pos;
        head->get_trans_pos(nullptr, &pos, nullptr, nullptr);
        signal_semi_sync_ack(head->get_trans_fixed_log_path(), pos);
      }
      /*
        When after_commit finished for the transaction, clear the run_hooks
        flag. This allow other parts of the system to check if after_commit was
        called.
      */
      head->get_transaction()->m_flags.run_hooks = false;
      last_thd = thd;
    }
  }

  if (enable_raft_plugin && !error && last_thd &&
      opt_raft_signal_async_dump_threads == AFTER_ENGINE_COMMIT &&
      rpl_wait_for_semi_sync_ack) {
    my_off_t log_pos;
    const char *log_file = nullptr;
    if (mysql_bin_log.is_apply_log) {
      thd->get_trans_relay_log_pos((const char **)&log_file, &log_pos);
    } else {
      thd->get_trans_fixed_pos((const char **)&log_file, &log_pos);
    }
    dump_log.signal_semi_sync_ack(log_file, log_pos);
  }
}

/**
  Sets the commit_consensus_error flag in all thd's of this group if there was
  an error in the before_commit hook

  @param queue_head  Head of the queue
*/
void MYSQL_BIN_LOG::set_commit_consensus_error(THD *queue_head) {
  for (THD *thd = queue_head; thd != NULL; thd = thd->next_to_commit) {
    thd->commit_consensus_error = true;
  }
}

/**
  Handles commit_consensus_error by consulting commit_consensus_error_action

  @param thd Thd for which conecnsus error needs to be handled
*/
void MYSQL_BIN_LOG::handle_commit_consensus_error(THD *thd) {
  DBUG_ENTER("MYSQL_BIN_LOG::handle_commit_consensus_error");
  bool all = thd->get_transaction()->m_flags.real_commit;

  /* Handle commit consensus error appropriately */
  switch (opt_commit_consensus_error_action) {
    case ROLLBACK_TRXS_IN_GROUP:
      /* Rollbak the trx and set commit_error in thd->commit_error
       * Also clear commit_low flag to prevent commit getting
       * triggered when the session ends. ha_rollback_low() could fail,
       * but there is nothing much we can do */
      ha_rollback_low(thd, all);
      gtid_state->update_on_rollback(thd);

      thd->commit_error = THD::CE_COMMIT_ERROR;
      thd->get_transaction()->m_flags.commit_low = false;

      // Clear hlc_time since we did not commit this trx
      thd->hlc_time_ns_next = 0;
      thd->clear_raft_opid();

      thd->clear_error();  // Clear previous errors first
      char errbuf[MYSQL_ERRMSG_SIZE];
      my_error(ER_ERROR_DURING_COMMIT, MYF(0), 1,
               my_strerror(errbuf, MYSQL_ERRMSG_SIZE, 1));

      break;
    case IGNORE_COMMIT_CONSENSUS_ERROR:
      /* Ignore commit consensus error and commit to engine as usual */
      if (ha_commit_low(thd, all)) thd->commit_error = THD::CE_COMMIT_ERROR;

      break;
    default:
      // Should not happen. This is here to placate the compiler
      DBUG_ASSERT(false);
  }

  DBUG_VOID_RETURN;
}

#ifndef DBUG_OFF
/** Names for the stages. */
static const char *g_stage_name[] = {
    "FLUSH",
    "SYNC",
    "COMMIT",
};
#endif

bool MYSQL_BIN_LOG::change_stage(THD *thd MY_ATTRIBUTE((unused)),
                                 Commit_stage_manager::StageID stage,
                                 THD *queue, mysql_mutex_t *leave_mutex,
                                 mysql_mutex_t *enter_mutex) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: 0x%llx, stage: %s, queue: 0x%llx", (ulonglong)thd,
                       g_stage_name[stage], (ulonglong)queue));
  DBUG_ASSERT(0 <= stage && stage < Commit_stage_manager::STAGE_COUNTER);
  DBUG_ASSERT(enter_mutex);
  DBUG_ASSERT(queue);
  /*
    enroll_for will release the leave_mutex once the sessions are
    queued.
  */
  if (!Commit_stage_manager::get_instance().enroll_for(
          stage, queue, leave_mutex, enter_mutex)) {
    DBUG_ASSERT(!thd_get_cache_mngr(thd)->dbug_any_finalized());
    return true;
  }

  return false;
}

/**
  Flush the I/O cache to file.

  Flush the binary log to the binlog file if any byte where written
  and signal that the binary log file has been updated if the flush
  succeeds.
*/

int MYSQL_BIN_LOG::flush_cache_to_file(my_off_t *end_pos_var) {
  if (m_binlog_file->flush()) {
    THD *thd = current_thd;
    thd->commit_error = THD::CE_FLUSH_ERROR;
    return ER_ERROR_ON_WRITE;
  }

  // raft write data directly into IO_CACHE, thus
  // Binlog_ofile's m_position member isn't updated.
  if (enable_raft_plugin && !is_apply_log)
    *end_pos_var = atomic_binlog_end_pos.load();
  else
    *end_pos_var = m_binlog_file->position();
  return 0;
}

/**
  Call fsync() to sync the file to disk.
*/
std::pair<bool, bool> MYSQL_BIN_LOG::sync_binlog_file(bool force) {
  bool synced = false;
  ulonglong start_time, binlog_fsync_time;
  unsigned int sync_period = get_sync_period();
  if (force || (sync_period && ++sync_counter >= sync_period)) {
    sync_counter = 0;

    /*
      There is a chance that binlog file could be closed by 'RESET MASTER' or
      or 'FLUSH LOGS' just after the leader releases LOCK_log and before it
      acquires LOCK_sync log. So it should check if m_binlog_file is opened.
    */
    start_time = my_timer_now();
    int ret =
        DBUG_EVALUATE_IF("simulate_error_during_sync_binlog_file", 1,
                         m_binlog_file->is_open() && m_binlog_file->sync());

    binlog_fsync_time = my_timer_since(start_time);
    if (histogram_step_size_binlog_fsync)
      latency_histogram_increment(&histogram_binlog_fsync, binlog_fsync_time,
                                  1);

    if (ret) {
      THD *thd = current_thd;
      thd->commit_error = THD::CE_SYNC_ERROR;
      return std::make_pair(true, synced);
    }
    synced = true;
  }
  return std::make_pair(false, synced);
}

/**
   Helper function executed when leaving @c ordered_commit.

   This function contain the necessary code for fetching the error
   code, doing post-commit checks, and wrapping up the commit if
   necessary.

   It is typically called when enter_stage indicates that the thread
   should bail out, and also when the ultimate leader thread finishes
   executing @c ordered_commit.

   It is typically used in this manner:
   @code
   if (enter_stage(thd, Thread_queue::BINLOG_FLUSH_STAGE, thd, &LOCK_log))
     return finish_commit(thd);
   @endcode

   @return Error code if the session commit failed, or zero on
   success.
 */
int MYSQL_BIN_LOG::finish_commit(THD *thd) {
  DBUG_TRACE;
  DEBUG_SYNC(thd, "reached_finish_commit");
  /*
    In some unlikely situations, it can happen that binary
    log is closed before the thread flushes it's cache.
    In that case, clear the caches before doing commit.
  */
  if (unlikely(!is_open())) {
    binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
    if (cache_mngr) cache_mngr->reset();
  }

  if (thd->get_transaction()->sequence_number != SEQ_UNINIT) {
    mysql_mutex_lock(&LOCK_slave_trans_dep_tracker);
    m_dependency_tracker.update_max_committed(thd);
    mysql_mutex_unlock(&LOCK_slave_trans_dep_tracker);
  }
  if (thd->get_transaction()->m_flags.commit_low) {
    const bool all = thd->get_transaction()->m_flags.real_commit;
    /*
      Now flush error and sync erros are ignored and we are continuing and
      committing. And at this time, commit_error cannot be COMMIT_ERROR.
    */
    DBUG_ASSERT(thd->commit_error != THD::CE_COMMIT_ERROR);
    /*
      storage engine commit
    */
    if (thd->commit_consensus_error)
      handle_commit_consensus_error(thd);
    else if (ha_commit_low(thd, all, false))
      thd->commit_error = THD::CE_COMMIT_ERROR;

    /*
      Decrement the prepared XID counter after storage engine commit
    */
    if (thd->get_transaction()->m_flags.xid_written)
      dec_prep_xids(thd);
    else if (thd->non_xid_trx)
      dec_non_xid_trxs(thd);
    /*
      If commit succeeded, we call the after_commit hook

      TODO: This hook here should probably move outside/below this
            if and be the only after_commit invocation left in the
            code.
    */
    if ((thd->commit_error != THD::CE_COMMIT_ERROR) &&
        thd->get_transaction()->m_flags.run_hooks) {
      // semi-sync plugin only called when raft is not enabled
      if (!enable_raft_plugin)
        (void)RUN_HOOK(transaction, after_commit, (thd, all));
      else
        (void)RUN_HOOK_STRICT(raft_replication, after_commit, (thd));
      thd->get_transaction()->m_flags.run_hooks = false;
    }

    if (enable_raft_plugin && thd->commit_error == THD::CE_NONE) {
      int error = RUN_HOOK_STRICT(raft_replication, after_commit, (thd));
      if (!error && opt_raft_signal_async_dump_threads == AFTER_ENGINE_COMMIT &&
          enable_raft_plugin && rpl_wait_for_semi_sync_ack) {
        const char *log_file = nullptr;
        my_off_t log_pos = 0;
        if (mysql_bin_log.is_apply_log) {
          thd->get_trans_relay_log_pos((const char **)&log_file, &log_pos);
        } else {
          thd->get_trans_fixed_pos((const char **)&log_file, &log_pos);
        }
        dump_log.signal_semi_sync_ack(log_file, log_pos);
      }
      thd->get_transaction()->m_flags.run_hooks = false;
    }
  } else if (thd->get_transaction()->m_flags.xid_written)
    dec_prep_xids(thd);
  else if (thd->non_xid_trx)
    dec_non_xid_trxs(thd);

  /*
    If the ordered commit didn't updated the GTIDs for this thd yet
    at process_commit_stage_queue (i.e. --binlog-order-commits=0)
    the thd still has the ownership of a GTID and we must handle it.
  */
  if (!thd->owned_gtid_is_empty()) {
    /*
      Gtid is added to gtid_state.executed_gtids and removed from owned_gtids
      on update_on_commit().
    */
    if (thd->commit_error == THD::CE_NONE) {
      gtid_state->update_on_commit(thd);
    } else
      gtid_state->update_on_rollback(thd);
  }

  /* If this is a slave, then update the local HLC to reflect the HLC of this
   * trx (as generated by master) */
  if (thd->hlc_time_ns_next != 0 && enable_binlog_hlc &&
      (thd->rli_slave || thd->rli_fake)) {
    update_hlc(thd->hlc_time_ns_next);
  }
  thd->hlc_time_ns_next = 0;

  // Clear the raft opid that is stashed, so that if the thread
  // is reused, it does not have stale terms and indexes
  thd->clear_raft_opid();

  DBUG_EXECUTE_IF("leaving_finish_commit", {
    const char act[] = "now SIGNAL signal_leaving_finish_commit";
    DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
  };);

  DBUG_ASSERT(thd->commit_error || !thd->get_transaction()->m_flags.run_hooks);
  DBUG_ASSERT(!thd_get_cache_mngr(thd)->dbug_any_finalized());
  DBUG_PRINT("return", ("Thread ID: %u, commit_error: %d", thd->thread_id(),
                        thd->commit_error));
  /*
    During shutdown, forcibly disconnect thd connections for
    transactions that are in the commit pipeline
  */
  DEBUG_SYNC(thd, "commit_wait_for_shutdown");
  if (!thd->slave_thread && thd->killed == THD::KILL_CONNECTION) {
    thd->disconnect();
  }

  /*
    flush or sync errors are handled by the leader of the group
    (using binlog_error_action). Hence treat only COMMIT_ERRORs as errors.
  */
  return thd->commit_error == THD::CE_COMMIT_ERROR;
}

/**
   Auxiliary function used in ordered_commit.
*/
static inline int call_after_sync_hook(THD *queue_head) {
  const char *log_file = nullptr;
  my_off_t pos = 0;

  if (NO_HOOK(binlog_storage)) return 0;

  DBUG_ASSERT(queue_head != nullptr);
  for (THD *thd = queue_head; thd != nullptr; thd = thd->next_to_commit)
    if (likely(thd->commit_error == THD::CE_NONE))
      thd->get_trans_fixed_pos(&log_file, &pos);

  if (DBUG_EVALUATE_IF("simulate_after_sync_hook_error", 1, 0) ||
      (!enable_raft_plugin &&
       RUN_HOOK(binlog_storage, after_sync, (queue_head, log_file, pos)))) {
    LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_RUN_AFTER_SYNC_HOOK);
    return ER_ERROR_ON_WRITE;
  }
  return 0;
}

/**
  Helper function to handle flush or sync stage errors.
  If binlog_error_action= ABORT_SERVER, server will be aborted
  after reporting the error to the client.
  If binlog_error_action= IGNORE_ERROR, binlog will be closed
  for the reset of the life time of the server. close() call is protected
  with LOCK_log to avoid any parallel operations on binary log.

  @param thd Thread object that faced flush/sync error
  @param need_lock_log
                       > Indicates true if LOCk_log is needed before closing
                         binlog (happens when we are handling sync error)
                       > Indicates false if LOCK_log is already acquired
                         by the thread (happens when we are handling flush
                         error)
  @param message Message stating the reason of the failure
*/
void MYSQL_BIN_LOG::handle_binlog_flush_or_sync_error(THD *thd,
                                                      bool need_lock_log,
                                                      const char *message) {
  bool commit_consensus_error = false;

  if (enable_raft_plugin) {
    for (THD *head = thd; head; head = head->next_to_commit) {
      if (head->commit_consensus_error &&
          head->commit_error == THD::CE_FLUSH_ERROR) {
        commit_consensus_error = true;
        break;
      }
    }

    /* This trx (and the group) will be rolled back in the engine if:
     * (1) binlog_error_action is set to rollback_trx
     * (2) There was a flush error because of a commit_consensus_error (i.e
     * flush error was due to a error inside the before_flush hook of raft
     * plugin)
     *
     * If (2) is not true, then we cannot safely rollbak the trx (either it
     * is too late OR safety of the raft consensus plugin will be violated.
     * Hence we proceed and abort the server
     */
    if (binlog_error_action == ROLLBACK_TRX && commit_consensus_error) {
      // Set commit consensus error for the entire group
      set_commit_consensus_error(thd);
      return;
    }
  }

  char errmsg[MYSQL_ERRMSG_SIZE] = {0};
  if (message == nullptr)
    sprintf(
        errmsg,
        "An error occurred during %s stage of the commit. "
        "'binlog_error_action' is set to '%s'.",
        thd->commit_error == THD::CE_FLUSH_ERROR ? "flush" : "sync",
        binlog_error_action == ABORT_SERVER ? "ABORT_SERVER" : "IGNORE_ERROR");
  else
    strncpy(errmsg, message, MYSQL_ERRMSG_SIZE - 1);

  if (should_abort_on_binlog_error()) {
    /* At this stage the error is either due to
     * (1) sync stage error
     * (2) flush stage error, but consensus error was not set - indicating that
     * the error did not happen inside raft plugin
     *
     * In both these cases we abort the server even when error_action is set to
     * rollback_trx. This is because sync happens periodically and the trx has
     * already committed to engine (so cannot rollback). We cannot safely
     * rollback flush stage errors happening outside of raft plugin.
     *
     * TODO: revisit if and when we have the ability to step down from within
     * mysql server
     */
    char err_buff[MYSQL_ERRMSG_SIZE + 27];
    sprintf(err_buff, "%s Hence aborting the server.", errmsg);
    exec_binlog_error_action_abort(err_buff);
  } else {
    DEBUG_SYNC(thd, "before_binlog_closed_due_to_error");
    if (need_lock_log)
      mysql_mutex_lock(&LOCK_log);
    else
      mysql_mutex_assert_owner(&LOCK_log);
    /*
      It can happen that other group leader encountered
      error and already closed the binary log. So print
      error only if it is in open state. But we should
      call close() always just in case if the previous
      close did not close index file.
    */
    if (is_open()) {
      LogErr(ERROR_LEVEL, ER_TURNING_LOGGING_OFF_FOR_THE_DURATION, errmsg);
    }
    close(LOG_CLOSE_INDEX | LOG_CLOSE_STOP_EVENT, false /*need_lock_log=false*/,
          true /*need_lock_index=true*/);
    /*
      If there is a write error (flush/sync stage) and if
      binlog_error_action=IGNORE_ERROR, clear the error
      and allow the commit to happen in storage engine.
    */
    if (check_write_error(thd) &&
        DBUG_EVALUATE_IF("simulate_cache_creation_failure", false, true))
      thd->clear_error();

    if (need_lock_log) mysql_mutex_unlock(&LOCK_log);
    DEBUG_SYNC(thd, "after_binlog_closed_due_to_error");
  }
}

int MYSQL_BIN_LOG::register_log_entities(THD *thd, int context, bool need_lock,
                                         bool is_relay_log) {
  if (need_lock)
    mysql_mutex_lock(&LOCK_log);
  else
    mysql_mutex_assert_owner(&LOCK_log);

  Raft_replication_observer::st_setup_flush_arg arg;
  arg.log_file_cache = m_binlog_file->get_io_cache();
  arg.log_prefix = name;
  arg.log_name = log_file_name;
  arg.cur_log_ext = &raft_cur_log_ext;
  arg.endpos_log_name = log_file_name;
  arg.endpos = (ulonglong *)&atomic_binlog_end_pos;
  arg.signal_cnt = &signal_cnt;
  arg.lock_log = &LOCK_log;
  arg.lock_index = &LOCK_index;
  arg.lock_end_pos = &LOCK_binlog_end_pos;
  arg.update_cond = &update_cond;
  arg.context = context;
  arg.is_relay_log = is_relay_log;

  int err = RUN_HOOK_STRICT(raft_replication, setup_flush, (thd, &arg));

  if (need_lock) mysql_mutex_unlock(&LOCK_log);

  return err;
}

void MYSQL_BIN_LOG::check_and_register_log_entities(THD *thd) {
  mysql_mutex_assert_owner(&LOCK_log);
  if (!enable_raft_plugin_save) return;

  // If this is a master and raft is turned ON, register the IO_cache and the
  // appropriate locks with the plugin.
  if (!mysql_bin_log.is_apply_log) {
    // only register once
    if (setup_flush_done) return;

    int err = register_log_entities(thd, /*context=*/1, /*need_lock=*/false,
                                    /*is_relay_log=*/false);

    setup_flush_done = (err == 0);
    if (err) {
      // TODO. This will fatal right now as the flush stage will fail
      // NO_LINT_DEBUG
      sql_print_error("Failed to register log entities with plugin.");
    }
    return;
  }

  // We are no more a master. We should reset the variable for a future step up
  setup_flush_done = false;
}

static int register_entities_with_raft() {
  THD *thd = current_thd;

  // First register the binlog for all servers
  // both masters and slaves
  // a Slave's mysql_bin_log will point to apply log
  // however when the slave becomes the master, its registered binlog
  // entities will be used by binlog wrapper
  int err = mysql_bin_log.register_log_entities(
      thd, /*context=*/0, /*need_lock=*/true, /*is_relay_log=*/false);

  if (err) {
    // NO_LINT_DEBUG
    sql_print_error(
        "Failed to register binlog file entities with "
        "raft replication observer");
    return err;
  }

  channel_map.rdlock();
  if (channel_map.get_num_instances() > 1) {
    // NO_LINT_DEBUG
    sql_print_error(
        "Number of slave channels connected: %d. Cannot register "
        "with raft",
        channel_map.get_num_instances());
    channel_map.unlock();
    return 1;  // error
  }
  channel_map.unlock();
  Master_info *active_mi;
  if (!get_and_lock_master_info(&active_mi)) {
    return 0;
  }

  // On a slave server, also register the relaylogs
  // Plugin will make that the default file to write to
  err = active_mi->rli->relay_log.register_log_entities(
      thd, /*context=*/0, /*need_lock=*/true, /*is_relay_log=*/true);
  unlock_master_info(active_mi);
  if (err) {
    // NO_LINT_DEBUG
    sql_print_error("Failed to register relaylog file entities");
  }

  return err;
}

// This function is called by plugin after BinlogWrapper creation.
// It asks the server to register the binlog & relaylog file
// immediately
int ask_server_to_register_with_raft(Raft_Registration_Item item) {
  int err = 0;
  switch (item) {
    case RAFT_REGISTER_LOCKS:
      return register_entities_with_raft();
    case RAFT_REGISTER_PATHS: {
      size_t llen;

      std::string s_wal_dir;
      char wal_dir[FN_REFLEN];
      if (!dirname_part(wal_dir, log_bin_basename, &llen)) {
        // NO_LINT_DEBUG
        sql_print_information(
            "dirname_part for log_file_basename fails. "
            "Falling back to datadir");
        s_wal_dir.assign(mysql_real_data_home_ptr);
      } else {
        s_wal_dir.assign(wal_dir);
      }

      std::string s_log_dir;
      char log_dir[FN_REFLEN];
      if (!dirname_part(log_dir, log_error_dest, &llen)) {
        // NO_LINT_DEBUG
        sql_print_information(
            "dirname_part for log_error_dst fails. "
            "Falling back to datadir");
        s_log_dir.assign(mysql_real_data_home_ptr);
      } else {
        s_log_dir.assign(log_dir);
      }

      THD *thd = current_thd;
      err = RUN_HOOK_STRICT(
          raft_replication, register_paths,
          (thd, server_uuid, ::server_id, s_wal_dir, s_log_dir,
           log_bin_basename, glob_hostname, (uint64_t)mysqld_port));
      break;
    }
    default:
      return -1;
  };

  return err;
}

int MYSQL_BIN_LOG::ordered_commit(THD *thd, bool all, bool skip_commit) {
  DBUG_TRACE;
  int flush_error = 0, sync_error = 0;
  my_off_t total_bytes = 0;
  bool do_rotate = false;

  DBUG_EXECUTE_IF("crash_commit_before_log", DBUG_SUICIDE(););
  init_thd_variables(thd, all, skip_commit);
  DBUG_PRINT("enter", ("commit_pending: %s, commit_error: %d, thread_id: %u",
                       YESNO(thd->tx_commit_pending), thd->commit_error,
                       thd->thread_id()));

  DEBUG_SYNC(thd, "bgc_before_flush_stage");

  /*
    Stage #0: ensure slave threads commit order as they appear in the slave's
              relay log for transactions flushing to binary log.

    This will make thread wait until its turn to commit.
    Commit_order_manager maintains it own queue and its own order for the
    commit. So Stage#0 doesn't maintain separate StageID.
  */
  if (Commit_order_manager::wait_for_its_turn_before_flush_stage(thd) ||
      ending_trans(thd, all) ||
      Commit_order_manager::get_rollback_status(thd)) {
    if (Commit_order_manager::wait(thd)) {
      return thd->commit_error;
    }
  }

  /*
    Stage #1: flushing transactions to binary log

    While flushing, we allow new threads to enter and will process
    them in due time. Once the queue was empty, we cannot reap
    anything more since it is possible that a thread entered and
    appointed itself leader for the flush phase.
  */

  if (change_stage(thd, Commit_stage_manager::BINLOG_FLUSH_STAGE, thd, nullptr,
                   &LOCK_log)) {
    DBUG_PRINT("return", ("Thread ID: %u, commit_error: %d", thd->thread_id(),
                          thd->commit_error));
    return finish_commit(thd);
  }

  if (enable_raft_plugin) {
    enable_raft_plugin_save = enable_raft_plugin;
    check_and_register_log_entities(thd);
  }

  THD *wait_queue = nullptr, *final_queue = nullptr;
  mysql_mutex_t *leave_mutex_before_commit_stage = nullptr;
  my_off_t flush_end_pos = 0;
  bool update_binlog_end_pos_after_sync;
  if (unlikely(!is_open())) {
    final_queue = fetch_and_process_flush_stage_queue(true);
    leave_mutex_before_commit_stage = &LOCK_log;
    /*
      binary log is closed, flush stage and sync stage should be
      ignored. Binlog cache should be cleared, but instead of doing
      it here, do that work in 'finish_commit' function so that
      leader and followers thread caches will be cleared.
    */
    goto commit_stage;
  }
  DEBUG_SYNC(thd, "waiting_in_the_middle_of_flush_stage");
  flush_error =
      process_flush_stage_queue(&total_bytes, &do_rotate, &wait_queue);

  if (flush_error == 0 && total_bytes > 0)
    flush_error = flush_cache_to_file(&flush_end_pos);
  DBUG_EXECUTE_IF("crash_after_flush_binlog", DBUG_SUICIDE(););

  update_binlog_end_pos_after_sync = (get_sync_period() == 1);

  /*
    If the flush finished successfully, we can call the after_flush
    hook. Being invoked here, we have the guarantee that the hook is
    executed before the before/after_send_hooks on the dump thread
    preventing race conditions among these plug-ins.
  */
  if (flush_error == 0) {
    const char *file_name_ptr = log_file_name + dirname_length(log_file_name);
    DBUG_ASSERT(flush_end_pos != 0);
    // semi-sync to be called only when raft is not enabled
    if (!enable_raft_plugin && RUN_HOOK(binlog_storage, after_flush,
                                        (thd, file_name_ptr, flush_end_pos))) {
      LogErr(ERROR_LEVEL, ER_BINLOG_FAILED_TO_RUN_AFTER_FLUSH_HOOK);
      flush_error = ER_ERROR_ON_WRITE;
    }

    if (!update_binlog_end_pos_after_sync) update_binlog_end_pos();

    DBUG_EXECUTE_IF("crash_commit_after_log", DBUG_SUICIDE(););
  }

  /* simulate a write failure during commit - needed for unit test */
  DBUG_EXECUTE_IF("abort_with_io_write_error",
                  flush_error = ER_ERROR_ON_WRITE;);

  /* skip dumping core if write failed and we are allowed to do so */
  if (flush_error == ER_ERROR_ON_WRITE && skip_core_dump_on_error)
    opt_core_file = false;

  if (flush_error) {
    /*
      Handle flush error (if any) after leader finishes it's flush stage.
    */
    handle_binlog_flush_or_sync_error(thd, false /* need_lock_log */, nullptr);
  }

  DEBUG_SYNC(thd, "bgc_after_flush_stage_before_sync_stage");

  /*
    Stage #2: Syncing binary log file to disk
  */

  if (change_stage(thd, Commit_stage_manager::SYNC_STAGE, wait_queue, &LOCK_log,
                   &LOCK_sync)) {
    DBUG_PRINT("return", ("Thread ID: %u, commit_error: %d", thd->thread_id(),
                          thd->commit_error));
    return finish_commit(thd);
  }

  /*
    Shall introduce a delay only if it is going to do sync
    in this ongoing SYNC stage. The "+1" used below in the
    if condition is to count the ongoing sync stage.
    When sync_binlog=0 (where we never do sync in BGC group),
    it is considered as a special case and delay will be executed
    for every group just like how it is done when sync_binlog= 1.
  */
  if (!flush_error && (sync_counter + 1 >= get_sync_period()))
    Commit_stage_manager::get_instance().wait_count_or_timeout(
        opt_binlog_group_commit_sync_no_delay_count,
        opt_binlog_group_commit_sync_delay, Commit_stage_manager::SYNC_STAGE);

  final_queue = Commit_stage_manager::get_instance().fetch_queue_acquire_lock(
      Commit_stage_manager::SYNC_STAGE);

  if (flush_error == 0 && total_bytes > 0) {
    DEBUG_SYNC(thd, "before_sync_binlog_file");
    std::pair<bool, bool> result = sync_binlog_file(false);
    sync_error = result.first;
  }

  if (update_binlog_end_pos_after_sync) {
    THD *tmp_thd = final_queue;
    const char *binlog_file = nullptr;
    my_off_t pos = 0;
    while (tmp_thd->next_to_commit != nullptr)
      tmp_thd = tmp_thd->next_to_commit;
    if (flush_error == 0 && sync_error == 0) {
      tmp_thd->get_trans_fixed_pos(&binlog_file, &pos);
      update_binlog_end_pos(binlog_file, pos);
    }
  }

  DEBUG_SYNC(thd, "bgc_after_sync_stage_before_commit_stage");

  leave_mutex_before_commit_stage = &LOCK_sync;
  /*
    Stage #3: Commit all transactions in order.

    This stage is skipped if we do not need to order the commits and
    each thread have to execute the handlerton commit instead.

    Howver, since we are keeping the lock from the previous stage, we
    need to unlock it if we skip the stage.

    We must also step commit_clock before the ha_commit_low() is called
    either in ordered fashion(by the leader of this stage) or by the tread
    themselves.

    We are delaying the handling of sync error until
    all locks are released but we should not enter into
    commit stage if binlog_error_action is ABORT_SERVER.
  */
commit_stage:
  /* Clone needs binlog commit order. */
  if ((opt_binlog_order_commits || Clone_handler::need_commit_order()) &&
      (sync_error == 0 || binlog_error_action != ABORT_SERVER)) {
    if (change_stage(thd, Commit_stage_manager::COMMIT_STAGE, final_queue,
                     leave_mutex_before_commit_stage, &LOCK_commit)) {
      DBUG_PRINT("return", ("Thread ID: %u, commit_error: %d", thd->thread_id(),
                            thd->commit_error));
      return finish_commit(thd);
    }
    THD *commit_queue =
        Commit_stage_manager::get_instance().fetch_queue_acquire_lock(
            Commit_stage_manager::COMMIT_STAGE);
    DBUG_EXECUTE_IF("semi_sync_3-way_deadlock",
                    DEBUG_SYNC(thd, "before_process_commit_stage_queue"););

    process_consensus_queue(commit_queue);

    ulonglong start_time;
    if (flush_error == 0 && sync_error == 0) {
      start_time = my_timer_now();
      sync_error = call_after_sync_hook(commit_queue);
      thd->semisync_ack_time = my_timer_since(start_time);
    }

    /*
      process_commit_stage_queue will call update_commit_group for the GTID
      owned by each thd in the queue.

      This will be done this way to guarantee that GTIDs are added to
      gtid_executed in order, to avoid creating unnecessary temporary
      gaps and keep gtid_executed as a single interval at all times.

      If we allow each thread to call update_on_commit only when they
      are at finish_commit, the GTID order cannot be guaranteed and
      temporary gaps may appear in gtid_executed. When this happen,
      the server would have to add and remove intervals from the
      Gtid_set, and adding and removing intervals requires a mutex,
      which would reduce performance.
    */
    start_time = my_timer_now();
    process_commit_stage_queue(thd, commit_queue);
    thd->engine_commit_time = my_timer_since(start_time);
    mysql_mutex_unlock(&LOCK_commit);
    /*
      Process after_commit after LOCK_commit is released for avoiding
      3-way deadlock among user thread, rotate thread and dump thread.
    */
    process_after_commit_stage_queue(thd, commit_queue);
    final_queue = commit_queue;
  } else {
    if (leave_mutex_before_commit_stage)
      mysql_mutex_unlock(leave_mutex_before_commit_stage);
    if (flush_error == 0 && sync_error == 0)
      sync_error = call_after_sync_hook(final_queue);
  }

  /*
    Handle sync error after we release all locks in order to avoid deadlocks
  */
  if (sync_error)
    handle_binlog_flush_or_sync_error(thd, true /* need_lock_log */, nullptr);

  DEBUG_SYNC(thd, "before_signal_done");
  /* Commit done so signal all waiting threads */
  Commit_stage_manager::get_instance().signal_done(final_queue);
  DBUG_EXECUTE_IF("block_leader_after_delete", {
    const char action[] = "now SIGNAL leader_proceed";
    DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(action)));
  };);

  /*
    Finish the commit before executing a rotate, or run the risk of a
    deadlock. We don't need the return value here since it is in
    thd->commit_error, which is returned below.
  */
  (void)finish_commit(thd);
  DEBUG_SYNC(thd, "bgc_after_commit_stage_before_rotation");

  /*
    If we need to rotate, we do it without commit error.
    Otherwise the thd->commit_error will be possibly reset.
   */
  if (DBUG_EVALUATE_IF("force_rotate", 1, 0) ||
      (do_rotate && thd->commit_error == THD::CE_NONE &&
       !is_rotating_caused_by_incident)) {
    /*
      Do not force the rotate as several consecutive groups may
      request unnecessary rotations.

      NOTE: Run purge_logs wo/ holding LOCK_log because it does not
      need the mutex. Otherwise causes various deadlocks.
    */

    DEBUG_SYNC(thd, "ready_to_do_rotation");
    bool check_purge = false;
    mysql_mutex_lock(&LOCK_log);
    /*
      If rotate fails then depends on binlog_error_action variable
      appropriate action will be taken inside rotate call.
    */
    int error = rotate(false, &check_purge);
    mysql_mutex_unlock(&LOCK_log);

    if (error)
      thd->commit_error = THD::CE_COMMIT_ERROR;
    else if (check_purge)
      purge();
  }
  /*
    flush or sync errors are handled above (using binlog_error_action).
    Hence treat only COMMIT_ERRORs as errors.
  */
  return thd->commit_error == THD::CE_COMMIT_ERROR;
}

int MYSQL_BIN_LOG::config_change_rotate(std::string config_change) {
  int error = 0;
  DBUG_ENTER("MYSQL_BIN_LOG::config_change_rotate");

  // config change can only be initiated on master/mysql_bin_log
  DBUG_ASSERT(!is_relay_log);
  RaftRotateInfo raft_rotate_info;
  raft_rotate_info.config_change = std::move(config_change);
  raft_rotate_info.config_change_rotate = true;
  error = new_file_impl(true /*need lock log*/, nullptr, &raft_rotate_info);

  DBUG_RETURN(error);
}

int raft_config_change(std::string config_change) {
  int error = 0;
  DBUG_ENTER("raft_config_change");
  if (mysql_bin_log.is_open()) {
    error = mysql_bin_log.config_change_rotate(std::move(config_change));
  } else {
    // TODO(luqun) - add throttled messaging here if present in 8.0
    error = 1;
  }
  DBUG_RETURN(error);
}

int raft_update_follower_info(
    const std::unordered_map<std::string, std::string> &follower_info,
    bool is_leader, bool is_shutdown) {
  int error = 0;
  DBUG_ENTER("raft_update_follower_info");
  error = register_raft_followers(follower_info, is_leader, is_shutdown);
  DBUG_RETURN(error);
}

int rotate_binlog_file(THD *thd) {
  int error = 0;
  DBUG_ENTER("rotate_binlog_file(THD *)");

  if (mysql_bin_log.is_open()) {
    error = mysql_bin_log.rotate_and_purge(thd, true);
  }

  DBUG_RETURN(error);
}

bool block_all_dump_threads() {
  block_dump_threads = true;
  kill_all_dump_threads();

  uint count = 50;
  while (count-- &&
         Global_THD_manager::get_instance()->get_num_thread_binlog_client())
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

  if (Global_THD_manager::get_instance()->get_num_thread_binlog_client()) {
    // NO_LINT_DEBUG
    sql_print_error("Dump thread count did not reach 0 after 5 secs!");
    block_dump_threads = false;
    return false;
  }

  return true;
}

void unblock_all_dump_threads() { block_dump_threads = false; }

int handle_dump_threads(bool block) {
  DBUG_TRACE;
  int err = 0;
  if (block)
    err = block_all_dump_threads() ? 0 : 1;
  else
    unblock_all_dump_threads();
  return err;
}

int binlog_change_to_apply() {
  DBUG_ENTER("binlog_change_to_apply");

  // disable_raft_log_repointing for MTR integration tests
  if (disable_raft_log_repointing) {
    mysql_bin_log.is_apply_log = true;
    DBUG_RETURN(0);
  }

  int error = 0;
  LOG_INFO linfo;
  mysql_mutex_lock(mysql_bin_log.get_log_lock());
  const bool is_locked = dump_log.lock();
  mysql_bin_log.lock_index();
  mysql_bin_log.lock_binlog_end_pos();
  mysql_bin_log.close(LOG_CLOSE_INDEX, /*need_lock_log=*/false,
                      /*need_lock_index=*/false);

  if (mysql_bin_log.open_index_file(opt_applylog_index_name, opt_apply_logname,
                                    /*need_lock_index=*/false)) {
    error = 1;
    goto err;
  }

  mysql_bin_log.is_apply_log = true;

  // set prevoius gtid set for relay log
  mysql_bin_log.set_previous_gtid_set_relaylog(
      const_cast<Gtid_set *>(gtid_state->get_executed_gtids()));

  // HLC is TBD
  if (mysql_bin_log.open_binlog(opt_apply_logname,
                                /*new_name=*/NULL, max_binlog_size,
                                /*null_created_arg=*/false,
                                /*need_lock_index=*/false,
                                /*need_sid_lock=*/true,
                                /*extra_description_event=*/NULL,
                                /*new_index_number =*/0,
                                /*raft_rotate_info =*/nullptr,
                                /*need_end_log_pos_lock =*/false)) {
    error = 1;
    goto err;
  }
  dump_log.switch_log(/* relay_log= */ true, /* should_lock= */ false);

  // Purge all apply logs before the last log, because they
  // are from the previous epoch of being a FOLLOWER, and they
  // don't have proper Rotate events at the end.
  mysql_bin_log.raw_get_current_log(&linfo);

  if (mysql_bin_log.purge_logs(
          linfo.log_file_name, false /* included */,
          false /*need_lock_index=false*/, true /*need_update_threads=true*/,
          NULL /* decrease space */, true /* auto purge */)) {
    error = 1;
    goto err;
  }

  if (mysql_bin_log.init_prev_gtid_sets_map()) {
    error = 1;
    goto err;
  }

err:
  mysql_bin_log.unlock_binlog_end_pos();
  mysql_bin_log.unlock_index();
  dump_log.unlock(is_locked);
  mysql_mutex_unlock(mysql_bin_log.get_log_lock());

  DBUG_RETURN(error);
}

int binlog_change_to_binlog(THD *thd) {
  DBUG_ENTER("binlog_change_to_binlog");

  // disable_raft_log_repointing for MTR integration tests
  if (disable_raft_log_repointing) {
    mysql_bin_log.is_apply_log = false;
    DBUG_RETURN(0);
  }

  int error = 0;
  uint64_t prev_hlc = 0;
  std::vector<std::string> lognames;

  // Flush logs to ensure that storage engine has flushed and fsynced the last
  // batch of transactions. This is important because the act of switching trx
  // logs from "apply-logs-*" to "binary-logs-*" looks like a rotation to other
  // parts of the system and rotation is always a 'sync' point
  ha_flush_logs(NULL);

  mysql_mutex_lock(mysql_bin_log.get_log_lock());
  const bool is_locked = dump_log.lock();
  mysql_bin_log.lock_index();

  Master_info *active_mi;
  if (!get_and_lock_master_info(&active_mi) || !active_mi || !active_mi->rli) {
    error = 1;
    // NO_LINT_DEBUG
    sql_print_error("active_mi or rli is not set");
    mysql_bin_log.unlock_index();
    dump_log.unlock(is_locked);
    mysql_mutex_unlock(mysql_bin_log.get_log_lock());
    return error;
  }
  active_mi->rli->relay_log.lock_binlog_end_pos();

  // Get the index file name
  std::string indexfn = mysql_bin_log.get_index_fname();

  bool delete_apply_logs = false;
  if (indexfn.find(opt_applylog_index_name) != std::string::npos) {
    // This is a apply-binlog index file. Get a list of apply-binlog names from
    // the index file
    error = mysql_bin_log.get_lognames_from_index(false, &lognames);
    if (error) {
      // NO_LINT_DEBUG
      sql_print_error(
          "Failed to get apply binlog filenames from the index file");
      error = 1;
      goto err;
    }
    delete_apply_logs = true;
  }

  mysql_bin_log.close(LOG_CLOSE_INDEX, /*need_lock_log=*/false,
                      /*need_lock_index=*/false);

  // Use opt_bin_logname to calculate index file full path
  if (mysql_bin_log.open_index_file(/*index_file_name_arg*/ nullptr,
                                    opt_bin_logname,
                                    /*need_lock_index=*/false)) {
    error = 1;
    goto err;
  }

  global_sid_lock->wrlock();
  if (mysql_bin_log.init_gtid_sets(
          const_cast<Gtid_set *>(gtid_state->get_executed_gtids()),
          const_cast<Gtid_set *>(gtid_state->get_lost_gtids()),
          opt_master_verify_checksum,
          /*need_lock=*/false,
          /*trx_parser=*/nullptr,
          /*partial_trx=*/nullptr, &prev_hlc)) {
    global_sid_lock->unlock();
    error = 1;
    goto err;
  }
  global_sid_lock->unlock();

  /*
   * TODO: 5.6 myraft sets previous gtid set here using
   * mysql_bin_log.set_previous_gtid_set(gtid_state->get_logged_gtids).
   * However, this is not available in 8.0. open_binlog() determines the
   * previous_gtid_set that needs to be written into the new file. Hence the
   * change is dropped in 8.0.
   */

  // Update the instance's HLC clock to be greater than or equal to the HLC
  // times of trx's in all previous binlog
  mysql_bin_log.update_hlc(prev_hlc);

  if (mysql_bin_log.open_existing_binlog(opt_bin_logname, max_binlog_size)) {
    error = 1;
    goto err;
  }

  if (!delete_apply_logs) goto err;

  // 1. Now delete all the apply binlogs
  // 2. Once the apply binlogs are deleted, then proceed to delete the apply
  // binlog index file
  // TODO: This needs better safety - if mysqld crashes between 1 and 2, it will
  // not be able to startup without manual intervention
  for (const auto &name : lognames) {
    if (my_delete(name.c_str(), MYF(MY_WME))) {  // #1
      // NO_LINT_DEBUG
      sql_print_error("Could not delete the apply binlog file %s",
                      name.c_str());
      error = 1;
      goto err;
    }
  }

  // NO_LINT_DEBUG
  sql_print_information("Deleting the apply index file %s", indexfn.c_str());
  if (my_delete(indexfn.c_str(), MYF(MY_WME))) {  // #2
    // NO_LINT_DEBUG
    sql_print_error("Failed to delete apply index file %s", indexfn.c_str());
    error = 1;
    goto err;
  }

  // unset apply log, so that masters
  // ordered_commit understands this
  mysql_bin_log.is_apply_log = false;
  mysql_bin_log.apply_file_count.store(0);

  dump_log.switch_log(/* relay_log= */ false, /* should_lock= */ false);

  // Register new log to raft
  // Previous mysql_bin_log.close(LOG_CLOSE_INDEX) will also close binlog and
  // its IO_CACHE.
  mysql_bin_log.register_log_entities(thd, /*context=*/0, /*need_lock=*/false,
                                      /*is_relay_log=*/false);
err:
  active_mi->rli->relay_log.unlock_binlog_end_pos();
  unlock_master_info(active_mi);
  mysql_bin_log.unlock_index();
  dump_log.unlock(is_locked);
  mysql_mutex_unlock(mysql_bin_log.get_log_lock());

  DBUG_EXECUTE_IF("crash_after_point_binlog_to_binlog", DBUG_SUICIDE(););

  DBUG_RETURN(error);
}

int MYSQL_BIN_LOG::get_lognames_from_index(bool need_lock,
                                           std::vector<std::string> *lognames) {
  LOG_INFO log_info;
  int error = 0;

  if (need_lock) mysql_mutex_lock(&LOCK_index);

  if ((error = find_log_pos(&log_info, NullS, false /*need_lock_index=false*/)))
    goto err;

  while (true) {
    lognames->emplace_back(log_info.log_file_name);

    int ret = find_next_log(&log_info, false /*need_lock_index=false*/);
    if (ret == LOG_INFO_EOF) {
      break;
    } else if (ret == LOG_INFO_IO) {
      // NO_LINT_DEBUG
      sql_print_error("Could not read from log index file ");
      error = 1;
      goto err;
    }
  }

err:
  if (need_lock) mysql_mutex_unlock(&LOCK_index);

  return error;
}

Dump_log::Dump_log() {
  if (enable_raft_plugin && mysql_bin_log.is_apply_log) {
    Master_info *active_mi = nullptr;
    if (!get_and_lock_master_info(&active_mi)) {
      // NO_LINT_DEBUG
      sql_print_error("active_mi or rli is not set");
    }
    DBUG_ASSERT(active_mi && active_mi->rli);
    log_ = &active_mi->rli->relay_log;
    unlock_master_info(active_mi);
  } else {
    log_ = &mysql_bin_log;
  }
}

void Dump_log::switch_log(bool relay_log, bool should_lock) {
  bool is_locked = false;
  if (should_lock) is_locked = lock();
  mysql_mutex_assert_owner(log_->get_binlog_end_pos_lock());
  log_->update_binlog_end_pos(/* need_lock= */ false);
  Master_info *active_mi = nullptr;
  if (!get_and_lock_master_info(&active_mi)) {
    // NO_LINT_DEBUG
    sql_print_error("active_mi or rli is not set");
  }
  DBUG_ASSERT(active_mi && active_mi->rli);
  log_ = relay_log ? &active_mi->rli->relay_log : &mysql_bin_log;
  unlock_master_info(active_mi);
  // Now let's update the dump thread's linfos
  log_->reset_semi_sync_last_acked();
  adjust_linfo_in_dump_threads(relay_log);
  if (should_lock) unlock(is_locked);
}

// Given a file name of the form 'binlog-file-name.index', it extracts the
// <binlog-file-name> and <index> and returns it as a pair
// Example:
// master-bin-3306.0001 ==> Returns (master-bin-3306, 1)
// master-bin-3306.9999 ==> Returns (master-bin-3306, 9999)
static std::pair<std::string, uint> extract_file_index(
    const std::string &file_name) {
  char *end;
  size_t pos = file_name.find_last_of('.');
  if (pos == string::npos) {
    DBUG_ASSERT(0);  // never should happened
    return std::make_pair(file_name, 1);
  }
  std::string prefix = file_name.substr(0, pos);
  uint index = std::strtoul(file_name.substr(pos + 1).c_str(), &end, 10);

  return std::make_pair(std::move(prefix), index);
}

/*
 * Sets the valid position in the binlog file based on engine position (i.e
 * engine binlog filename and file position)
 *
 * @param - valid_pos[out] - Valid position to set
 * @param - cur_binlog_file - The current binlog file that is being recovered
 * @param - first_gtid_start - The starting position of the first gtid event in
 *                             cur_binlog_file
 * @param - engine_binlog_file - The engine binlog file name
 * @param - engine_binlog_pos  - The engine binlog file position
 */
static void set_valid_pos(my_off_t *valid_pos,
                          const std::string &cur_binlog_file,
                          my_off_t first_gtid_start, char *engine_binlog_file,
                          my_off_t engine_binlog_pos) {
  std::string position = "Engine pos: " + std::to_string(engine_binlog_pos) +
                         ", Current binlog pos: " + std::to_string(*valid_pos) +
                         ", Engine binlog file: " + engine_binlog_file +
                         ", Current binlog file: " + cur_binlog_file;
  // NO_LINT_DEBUG
  sql_print_information("%s", position.c_str());

  if (cur_binlog_file.compare(engine_binlog_file) == 0) {
    // Case 1: Engine binlog file and current binlog files are the same.
    // Compare based only on file position
    if (*valid_pos > engine_binlog_pos) {
      // Binlog will be truncated to this position
      *valid_pos = engine_binlog_pos;
    } else if (*valid_pos < engine_binlog_pos) {
      // Engine is found to be ahead of the current binlog
      // NO_LINT_DEBUG
      sql_print_information(
          "Engine is ahead of binlog. "
          "Binlog will not be truncated to match engine.");
    }
  } else {
    // Case 2: Engine and binlog file names are different. Compare based on file
    // indexes.
    const auto engine_file_pair = extract_file_index(engine_binlog_file);
    const auto cur_file_pair = extract_file_index(cur_binlog_file);

    if (engine_file_pair.first.compare(cur_file_pair.first) != 0) {
      // The file prefix stored in engine is different than the current file
      // prefix. We cannot trim. So give up. Note that server will fail to start
      // in this case
      // NO_LINT_DEBUG
      sql_print_information(
          "The file prefix in engine does not match "
          "the file prefix of the recovering binlog. There "
          "will be no special trimming of the file");
    } else if (engine_file_pair.second < cur_file_pair.second) {
      // Engine file is lower than current binlog file. Truncate to the begin
      // position of the first gtid in the current binlog file
      *valid_pos = first_gtid_start;
    } else {
      // Engine is found to be ahead of the current binlog
      // NO_LINT_DEBUG
      sql_print_information(
          "Engine is ahead of binlog. "
          "Binlog will not be truncated to match engine.");
    }
  }
}

/**
  MYSQLD server recovers from last crashed binlog.

  @param[in] binlog_file_reader Binlog_file_reader of the crashed binlog.
  @param[out] valid_pos The position of the last valid transaction or
                        event(non-transaction) of the crashed binlog.
                        valid_pos must be non-NULL.
  @param[in] cur_binlog_file The current binlog file that is being recovered

  After a crash, storage engines may contain transactions that are
  prepared but not committed (in theory any engine, in practice
  InnoDB).  This function uses the binary log as the source of truth
  to determine which of these transactions should be committed and
  which should be rolled back.

  The function collects the XIDs of all transactions that are
  completely written to the binary log into a hash, and passes this
  hash to the storage engines through the ha_recover function in the
  handler interface.  This tells the storage engines to commit all
  prepared transactions that are in the set, and to roll back all
  prepared transactions that are not in the set.

  To compute the hash, this function iterates over the last binary log
  only (i.e. it assumes that 'log' is the last binary log).  It
  instantiates each event.  For XID-events (i.e. commit to InnoDB), it
  extracts the xid from the event and stores it in the hash.

  It is enough to iterate over only the last binary log because when
  the binary log is rotated we force engines to commit (and we fsync
  the old binary log).

  @retval false Success
  @retval true Out of memory, or storage engine returns error.
*/
static bool binlog_recover(Binlog_file_reader *binlog_file_reader,
                           my_off_t *valid_pos, Gtid *binlog_max_gtid,
                           char *engine_binlog_file,
                           my_off_t *engine_binlog_pos,
                           const std::string &cur_binlog_file,
                           my_off_t *first_gtid_start_pos) {
  bool res = false;
  binlog::tools::Iterator it(binlog_file_reader);
  it.set_copy_event_buffer();

  /*
    Prepared transactions are committed by XID during recovery but we need
    to track the max GTID so we maintain a map from XID to GTID and update
    the max GTID after committing by XID
  */
  Gtid current_gtid;
  current_gtid.clear();
  my_off_t first_gtid_start = 0;
  /*
    The flag is used for handling the case that a transaction
    is partially written to the binlog.
  */
  bool in_transaction = false;
  /*
    Flag to indicate if we have seen a gtid which is pending i.e the trx
    represented by this gtid has not yet ended
  */
  bool pending_gtid = false;
  int memory_page_size = my_getpagesize();
  {
    MEM_ROOT mem_root(key_memory_binlog_recover_exec, memory_page_size);
    xid_to_gtid_container xids(&mem_root);

    /*
      now process events in the queue. Queue is dynamically changed
      everytime we process an event. This may be a bit suboptimal
      since it adds an indirection, but it helps to generalize the
      usage of the transaction payload event (which unfolds into
      several events into the queue when it is processed).
    */
    for (Log_event *ev = it.begin(); !res && (ev != it.end()); ev = it.next()) {
      switch (ev->get_type_code()) {
        // may be begin, middle or end of a transaction
        case binary_log::QUERY_EVENT: {
          // starts a transaction
          if (!strcmp(((Query_log_event *)ev)->query, "BEGIN")) {
            in_transaction = true;
          } else if (!strcmp(((Query_log_event *)ev)->query, "COMMIT")) {
            DBUG_ASSERT(in_transaction == true);
            in_transaction = false;
          }
          // starts and ends a transaction
          if (is_atomic_ddl_event(ev)) {
            DBUG_ASSERT(in_transaction == false);
            auto qev = dynamic_cast<Query_log_event *>(ev);
            DBUG_ASSERT(qev != nullptr);
            res = (qev == nullptr ||
                   !xids.emplace(qev->ddl_xid, current_gtid).second);
          }
          break;
        }
        // ends a transaction
        case binary_log::XID_EVENT: {
          DBUG_ASSERT(in_transaction == true);
          in_transaction = false;
          Xid_log_event *xev = dynamic_cast<Xid_log_event *>(ev);
          DBUG_ASSERT(xev != nullptr);
          res =
              (xev == nullptr || !xids.emplace(xev->xid, current_gtid).second);
          break;
        }
        case binary_log::ANONYMOUS_GTID_LOG_EVENT:
        case binary_log::GTID_LOG_EVENT: {
          pending_gtid = true;
          auto gev = static_cast<Gtid_log_event *>(ev);
          if (gev->get_type() != ANONYMOUS_GTID)
            current_gtid.set(gev->get_sidno(true), gev->get_gno());
          else
            current_gtid.clear();
          if (first_gtid_start == 0) {
            first_gtid_start =
                ev->common_header->log_pos - ev->common_header->data_written;
            *first_gtid_start_pos = first_gtid_start;
          }
        }
        default: {
          break;
        }
      }

      /*
        Recorded valid position for the crashed binlog file
        which did not contain incorrect events. The following
        positions increase the variable valid_pos:

        1 -
          ...
          <---> HERE IS VALID <--->
          GTID
          BEGIN
          ...
          COMMIT
          ...

        2 -
          ...
          <---> HERE IS VALID <--->
          GTID
          DDL/UTILITY
          ...

        In other words, the following positions do not increase
        the variable valid_pos:

        1 -
          GTID
          <---> HERE IS VALID <--->
          ...

        2 -
          GTID
          BEGIN
          <---> HERE IS VALID <--->
          ...
      */
      if (!(ev->get_type_code() == binary_log::METADATA_EVENT &&
            pending_gtid)) {
        if (!in_transaction && !is_gtid_event(ev)) {
          *valid_pos = binlog_file_reader->position();
          pending_gtid = false;
        }
      }

      delete ev;
      ev = nullptr;
      res = it.has_error();
    }

    /*
      Call ha_recover if and only if there is a registered engine that
      does 2PC, otherwise in DBUG builds calling ha_recover directly
      will result in an assert. (Production builds would be safe since
      ha_recover returns right away if total_ha_2pc <= opt_log_bin.)
     */
    if (!res && total_ha_2pc > 1) {
      res = ha_recover(&xids, binlog_max_gtid, engine_binlog_file,
                       engine_binlog_pos);
      if (res) goto fin1;
      /*
        If trim binlog on recover option is set, then we essentially trim
        binlog to the position that the engine thinks it has committed. Note
        that if opt_trim_binlog option is set, then engine recovery (called
        through ha_recover() above) ensures that all prepared txns are rolled
        back. There are a few things which need to be kept in mind:
        1. txns never span across two binlogs, hence it is safe to recover only
           the latest binlog file.
        2. A binlog rotation ensures that the previous binlogs and engine's
           transaction logs are flushed and made durable. Hence all previous
           transactions are made durable.

         If enable_raft_plugin is set, then we skip trimming binlog. This is
         because the trim is handled inside the raft plugin when this node
         rejoins the raft ring
       */
      if (opt_trim_binlog && !enable_raft_plugin) {
        set_valid_pos(valid_pos, cur_binlog_file, first_gtid_start,
                      engine_binlog_file, *engine_binlog_pos);
      }
    }
  }

fin1:
  if (res) LogErr(ERROR_LEVEL, ER_BINLOG_CRASH_RECOVERY_FAILED);
  return res;
}

void MYSQL_BIN_LOG::report_missing_purged_gtids(
    const Gtid_set *lost_gtid_set, const Gtid_set *slave_executed_gtid_set,
    const char **errmsg) {
  DBUG_TRACE;
  THD *thd = current_thd;
  Gtid_set gtid_missing(lost_gtid_set->get_sid_map());
  gtid_missing.add_gtid_set(gtid_state->get_lost_gtids());
  gtid_missing.remove_gtid_set(slave_executed_gtid_set);

  String tmp_uuid;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&current_thd->LOCK_thd_data);
  const auto it = current_thd->user_vars.find("slave_uuid");
  if (it != current_thd->user_vars.end() && it->second->length() > 0) {
    tmp_uuid.copy(it->second->ptr(), it->second->length(), NULL);
  }
  mysql_mutex_unlock(&current_thd->LOCK_thd_data);

  char *missing_gtids = NULL;
  char *slave_executed_gtids = NULL;
  gtid_missing.to_string(&missing_gtids, NULL);
  slave_executed_gtid_set->to_string(&slave_executed_gtids, NULL);

  /*
     Log the information about the missing purged GTIDs to the error log.
  */
  std::ostringstream log_info;
  log_info << "The missing transactions are '" << missing_gtids << "'";

  LogErr(WARNING_LEVEL, ER_FOUND_MISSING_GTIDS, tmp_uuid.ptr(),
         log_info.str().c_str());

  /*
     Send the information about the slave executed GTIDs and missing
     purged GTIDs to slave if the message is less than MYSQL_ERRMSG_SIZE.
  */
  std::ostringstream gtid_info;
  gtid_info << "The GTID set sent by the slave is '" << slave_executed_gtids
            << "', and the missing transactions are '" << missing_gtids << "'";
  *errmsg = ER_THD(thd, ER_MASTER_HAS_PURGED_REQUIRED_GTIDS);

  /* Don't consider the "%s" in the format string. Subtract 2 from the
     total length */
  int total_length = (strlen(*errmsg) - 2 + gtid_info.str().length());

  DBUG_EXECUTE_IF("simulate_long_missing_gtids",
                  { total_length = MYSQL_ERRMSG_SIZE + 1; });

  if (total_length > MYSQL_ERRMSG_SIZE)
    gtid_info.str(
        "The GTID sets and the missing purged transactions are too"
        " long to print in this message. For more information,"
        " please see the master's error log or the manual for"
        " GTID_SUBTRACT");

  /* Buffer for formatting the message about the missing GTIDs. */
  static char buff[MYSQL_ERRMSG_SIZE];
  snprintf(buff, MYSQL_ERRMSG_SIZE, *errmsg, gtid_info.str().c_str());
  *errmsg = const_cast<const char *>(buff);

  my_free(missing_gtids);
  my_free(slave_executed_gtids);
}

void MYSQL_BIN_LOG::report_missing_gtids(
    const Gtid_set *previous_gtid_set, const Gtid_set *slave_executed_gtid_set,
    const char **errmsg) {
  DBUG_TRACE;
  THD *thd = current_thd;
  char *missing_gtids = NULL;
  char *slave_executed_gtids = NULL;
  Gtid_set gtid_missing(slave_executed_gtid_set->get_sid_map());
  gtid_missing.add_gtid_set(slave_executed_gtid_set);
  gtid_missing.remove_gtid_set(previous_gtid_set);
  gtid_missing.to_string(&missing_gtids, NULL);
  slave_executed_gtid_set->to_string(&slave_executed_gtids, NULL);

  String tmp_uuid;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&current_thd->LOCK_thd_data);
  const auto it = current_thd->user_vars.find("slave_uuid");
  if (it != current_thd->user_vars.end() && it->second->length() > 0) {
    tmp_uuid.copy(it->second->ptr(), it->second->length(), NULL);
  }
  mysql_mutex_unlock(&current_thd->LOCK_thd_data);

  /*
     Log the information about the missing purged GTIDs to the error log.
  */
  std::ostringstream log_info;
  log_info << "If the binary log files have been deleted from disk,"
              " check the consistency of 'GTID_PURGED' variable."
              " The missing transactions are '"
           << missing_gtids << "'";
  LogErr(WARNING_LEVEL, ER_FOUND_MISSING_GTIDS, tmp_uuid.ptr(),
         log_info.str().c_str());
  /*
     Send the information about the slave executed GTIDs and missing
     purged GTIDs to slave if the message is less than MYSQL_ERRMSG_SIZE.
  */
  std::ostringstream gtid_info;
  gtid_info << "The GTID set sent by the slave is '" << slave_executed_gtids
            << "', and the missing transactions are '" << missing_gtids << "'";
  *errmsg = ER_THD(thd, ER_MASTER_HAS_PURGED_REQUIRED_GTIDS);

  /* Don't consider the "%s" in the format string. Subtract 2 from the
     total length */
  if ((strlen(*errmsg) - 2 + gtid_info.str().length()) > MYSQL_ERRMSG_SIZE)
    gtid_info.str(
        "The GTID sets and the missing purged transactions are too"
        " long to print in this message. For more information,"
        " please see the master's error log or the manual for"
        " GTID_SUBTRACT");
  /* Buffer for formatting the message about the missing GTIDs. */
  static char buff[MYSQL_ERRMSG_SIZE];
  snprintf(buff, MYSQL_ERRMSG_SIZE, *errmsg, gtid_info.str().c_str());
  *errmsg = const_cast<const char *>(buff);
  my_free(missing_gtids);
  my_free(slave_executed_gtids);
}

/**
 * Recover raft log. This is primarily for relay logs in the raft world since
 * trx logs (binary logs or apply logs) are already recovered by mysqld as part
 * of trx log recovery. This method tries to get rid of partial trxs in the tal
 * of the raft log. Much has been borrowed from
 * MYSQL_BIN_LOG::open_binlog(const char *opt_name) and
 * binlog_recover(). Refactoring the components is rather hard and
 * adds unnecessary complexity with additional params and if() {} else {}
 * branches. Hence a separate method.
 */
int MYSQL_BIN_LOG::raft_log_recover() {
  int error = 0;
  Log_event *ev = 0;
  char log_name[FN_REFLEN];
  my_off_t valid_pos = 0;
  my_off_t binlog_size = 0;
  LOG_INFO log_info;
  bool pending_gtid = false;
  std::string error_message;
  int status = 0;
  bool in_transaction = false;
  if (!mysql_bin_log.is_apply_log)
    goto err;  // raft log already recovered as part of trx log recovery

  if (!my_b_inited(&index_file)) {
    error_message = "Index file is not inited in recover_raft_log";
    error = 1;
    goto err;
  }

  if ((status =
           find_log_pos(&log_info, NullS, true /*need_lock_index=true*/))) {
    if (status != LOG_INFO_EOF) {
      error_message = "find_log_pos() failed in recover_raft_log with error: " +
                      std::to_string(error);
      error = 1;
    }
    goto err;
  }

  do {
    strmake(log_name, log_info.log_file_name, sizeof(log_name) - 1);
  } while (!(status = find_next_log(&log_info, true /*need_lock_index=true*/)));

  if (status != LOG_INFO_EOF) {
    error_message = "find_log_pos() failed in recover_raft_log with error: " +
                    std::to_string(error);
    error = 1;
    goto err;
  }

  {
    Binlog_file_reader binlog_file_reader(opt_master_verify_checksum);
    if (binlog_file_reader.open(log_name)) {
      error = 1;
      error_message = "open_binlog_file() failed in recover_raft_log with ";
      goto err;
    }
    binlog_size = binlog_file_reader.ifile()->length();
    // This logic is borrowed from binlog_recover() which has to do
    // additional things and refactoring it will simply add more branches. Hence
    // the code duplication
    while ((ev = binlog_file_reader.read_event_object())) {
      if (ev->get_type_code() == binary_log::QUERY_EVENT &&
          !strcmp(static_cast<Query_log_event *>(ev)->query, "BEGIN")) {
        in_transaction = true;
      } else if (ev->get_type_code() == binary_log::QUERY_EVENT &&
                 !strcmp(static_cast<Query_log_event *>(ev)->query, "COMMIT")) {
        DBUG_ASSERT(in_transaction == true);
        in_transaction = false;
      } else if (is_gtid_event(ev)) {
        pending_gtid = true;
      } else if (ev->get_type_code() == binary_log::XID_EVENT ||
                 (ev->get_type_code() == binary_log::QUERY_EVENT &&
                  !strcmp(static_cast<Query_log_event *>(ev)->query,
                          "COMMIT"))) {
        if (!in_transaction) {
          // When we see a commit message, we should already be parsing a valid
          // transaction
          error_message =
              "Saw a XID/COMMIT event without a begin. Corrupted log: " +
              std::string(log_name);
          error = 1;
          delete ev;
          break;
        }
        in_transaction = false;
      }
      if (!(ev->get_type_code() == binary_log::METADATA_EVENT &&
            pending_gtid)) {
        if (!in_transaction && !is_gtid_event(ev)) {
          valid_pos = binlog_file_reader.position();
          pending_gtid = false;
        }
      }

      delete ev;
    }
  }

  // No partial trxs found in the raft log or error parsing the log
  if (error || (valid_pos == 0 || valid_pos >= binlog_size)) goto err;

  // NO_LINT_DEBUG
  sql_print_information(
      "Raft log %s with a size of %llu will be trimmed to "
      "%llu bytes based on valid transactions in the file",
      log_name, binlog_size, valid_pos);

  {
    std::unique_ptr<Binlog_ofile> ofile(
        Binlog_ofile::open_existing(key_file_binlog, log_name, MYF(MY_WME)));
    if (!ofile) {
      error_message =
          "Failed to remove partial transactions from raft log file ";
      error = 1;
      goto err;
    }
    if (ofile->truncate(valid_pos)) {
      error_message =
          "Failed to remove partial transactions from raft log file " +
          std::string(log_name);
      error = 1;
      goto err;
    }
  }

err:
  if (error && !error_message.empty())
    // NO_LINT_DEBUG
    sql_print_error("%s", error_message.c_str());

  return error;
}

void MYSQL_BIN_LOG::update_binlog_end_pos(bool need_lock) {
  if (need_lock)
    lock_binlog_end_pos();
  else
    mysql_mutex_assert_owner(&LOCK_binlog_end_pos);
  strmake(binlog_file_name, log_file_name, sizeof(binlog_file_name) - 1);
  atomic_binlog_end_pos = m_binlog_file->position();
  binlog_encrypted_header_size = m_binlog_file->get_encrypted_header_size();
  signal_update();
  if (need_lock) unlock_binlog_end_pos();
}

inline void MYSQL_BIN_LOG::update_binlog_end_pos(const char *file, my_off_t pos,
                                                 bool need_lock) {
  if (need_lock)
    lock_binlog_end_pos();
  else
    mysql_mutex_assert_owner(&LOCK_binlog_end_pos);
  if (is_active(file) && (pos > atomic_binlog_end_pos)) {
    atomic_binlog_end_pos = pos;
  }
  signal_update();
  if (need_lock) unlock_binlog_end_pos();
}

my_off_t MYSQL_BIN_LOG::get_binlog_end_pos() const {
  mysql_mutex_assert_not_owner(&LOCK_log);
  return atomic_binlog_end_pos;
}

/* wait_for_ack can be modified by this function */
my_off_t MYSQL_BIN_LOG::get_last_acked_pos(bool *wait_for_ack,
                                           const char *sender_log_name) {
  *wait_for_ack = *wait_for_ack && rpl_wait_for_semi_sync_ack &&
                  (rpl_semi_sync_master_enabled || enable_raft_plugin);

  if (!*wait_for_ack) return atomic_binlog_end_pos;

  const char *file_name = sender_log_name + dirname_length(sender_log_name);
  const uint file_num = extract_file_index(file_name).second;

  // get a copy of last acked pos atomically
  const st_filenum_pos local_last_acked = last_acked.load();

  const int res = local_last_acked.file_num - file_num;
  const my_off_t last_acked_pos = local_last_acked.pos;

  if (res == 0) return last_acked_pos;
  if (res < 0) return 0;  // wait for ack

  *wait_for_ack = false;
  return atomic_binlog_end_pos;
}

/* Use by raft plugin */
void signal_semi_sync_ack(const std::string &file, uint pos) {
  dump_log.signal_semi_sync_ack(file.c_str(), pos);
}

void MYSQL_BIN_LOG::signal_semi_sync_ack(const char *const log_file,
                                         const my_off_t log_pos) {
  if (!log_file || !log_file[0]) return;

  const char *file_name = log_file + dirname_length(log_file);
  const st_filenum_pos acked = {
      extract_file_index(file_name).second,
      // NOTE: If the acked pos cannot fit in st_filenum_pos::pos then we store
      // uint_max, this way we'll never send unacked trxs because the last acked
      // pos will be stuck at position uint_max in the current binlog file until
      // a rotation happens. This can only happen when a very large trx is
      // written to the binlog, max_binlog_size is capped at 1G but that's a
      // soft limit as one could still write more that 1G of binlogs in a single
      // trx, so when we hit this the log will immediately be rotated and things
      // will be back to normal.
      static_cast<uint>(std::min<ulonglong>(st_filenum_pos::max_pos, log_pos))};

  // case: nothing to update so no signal needed, let's exit
  if (acked <= last_acked.load()) {
    return;
  }

  lock_binlog_end_pos();
  if (acked > last_acked.load()) {
    last_acked = acked;
    update_binlog_end_pos(log_file, log_pos, false);
  }
  unlock_binlog_end_pos();
}

void MYSQL_BIN_LOG::reset_semi_sync_last_acked() {
  lock_binlog_end_pos();
  /* binary log is rotated and all trxs in previous binlog are already committed
   * to the storage engine.
   * Note: when in raft mode we cannot init the coords without consulting the
   * plugin, so we reset the coords
   */
  if (strlen(log_file_name)) {
    last_acked = {extract_file_index(log_file_name).second, 0};
  } else {
    last_acked = {0, 0};
  }
  signal_update();
  unlock_binlog_end_pos();
}

void MYSQL_BIN_LOG::get_semi_sync_last_acked(std::string &log_file,
                                             my_off_t &log_pos) {
  const st_filenum_pos local_last_acked = last_acked.load();
  if (local_last_acked.file_num) {
    char full_name[FN_REFLEN + 1];
    snprintf(full_name, FN_REFLEN, "%s.%06u", opt_bin_logname,
             local_last_acked.file_num);
    log_file = std::string(full_name);
  }
  log_pos = local_last_acked.pos;
}

bool THD::is_binlog_cache_empty(bool is_transactional) const {
  DBUG_TRACE;

  // If opt_bin_log==0, it is not safe to call thd_get_cache_mngr
  // because binlog_hton has not been completely set up.
  DBUG_ASSERT(opt_bin_log);
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(this);

  // cache_mngr is NULL until we call thd->binlog_setup_trx_data, so
  // we assert that this has been done.
  DBUG_ASSERT(cache_mngr != nullptr);

  binlog_cache_data *cache_data =
      cache_mngr->get_binlog_cache_data(is_transactional);
  DBUG_ASSERT(cache_data != nullptr);

  return cache_data->is_binlog_empty();
}

/*
  These functions are placed in this file since they need access to
  binlog_hton, which has internal linkage.
*/

int THD::binlog_setup_trx_data() {
  DBUG_TRACE;
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(this);

  if (cache_mngr) return 0;  // Already set up

  cache_mngr = (binlog_cache_mngr *)my_malloc(key_memory_binlog_cache_mngr,
                                              sizeof(binlog_cache_mngr),
                                              MYF(MY_ZEROFILL));
  if (!cache_mngr) {
    return 1;  // Didn't manage to set it up
  }

  cache_mngr = new (cache_mngr)
      binlog_cache_mngr(&binlog_stmt_cache_use, &binlog_stmt_cache_disk_use,
                        &binlog_cache_use, &binlog_cache_disk_use);
  if (cache_mngr->init()) {
    cache_mngr->~binlog_cache_mngr();
    my_free(cache_mngr);
    return 1;
  }

  DBUG_PRINT("debug", ("Set ha_data slot %d to 0x%llx", binlog_hton->slot,
                       (ulonglong)cache_mngr));
  thd_set_ha_data(this, binlog_hton, cache_mngr);

  return 0;
}

/**

*/
void register_binlog_handler(THD *thd, bool trx) {
  DBUG_TRACE;
  /*
    If this is the first call to this function while processing a statement,
    the transactional cache does not have a savepoint defined. So, in what
    follows:
      . an implicit savepoint is defined;
      . callbacks are registered;
      . binary log is set as read/write.

    The savepoint allows for truncating the trx-cache transactional changes
    fail. Callbacks are necessary to flush caches upon committing or rolling
    back a statement or a transaction. However, notifications do not happen
    if the binary log is set as read/write.
  */
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
  if (cache_mngr->trx_cache.get_prev_position() == MY_OFF_T_UNDEF) {
    /*
      Set an implicit savepoint in order to be able to truncate a trx-cache.
    */
    my_off_t pos = 0;
    binlog_trans_log_savepos(thd, &pos);
    cache_mngr->trx_cache.set_prev_position(pos);

    /*
      Set callbacks in order to be able to call commmit or rollback.
    */
    if (trx) trans_register_ha(thd, true, binlog_hton, nullptr);
    trans_register_ha(thd, false, binlog_hton, nullptr);

    /*
      Set the binary log as read/write otherwise callbacks are not called.
    */
    thd->get_ha_data(binlog_hton->slot)->ha_info[0].set_trx_read_write();
  }
}

bool show_raft_status(THD *thd) {
  Protocol *protocol = thd->get_protocol();
  List<Item> field_list;
  size_t max_var = 0;
  size_t max_value = 0;
  const char *errmsg = 0;
  std::vector<std::pair<std::string, std::string>> var_value_pairs;
  std::vector<std::pair<std::string, std::string>>::const_iterator itr;

  mysql_mutex_lock(&LOCK_status);
  int error = RUN_HOOK_STRICT(raft_replication, show_raft_status,
                              (current_thd, &var_value_pairs));
  mysql_mutex_unlock(&LOCK_status);
  if (error) {
    errmsg = "Failure to run plugin hook";
    goto err;
  }

  for (itr = var_value_pairs.begin(); itr != var_value_pairs.end(); ++itr) {
    max_var = std::max(max_var, itr->first.length() + 10);
    max_value = std::max(max_value, itr->second.length() + 10);
  }

  field_list.push_back(new Item_empty_string("VARIABLE_NAME", max_var));
  field_list.push_back(new Item_empty_string("VARIABLE_VALUE", max_value));
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    errmsg = "Failure during protocol send metadata";
    goto err;
  }

  for (itr = var_value_pairs.begin(); itr != var_value_pairs.end(); ++itr) {
    protocol->start_row();
    protocol->store_string(itr->first.c_str(), itr->first.length(),
                           &my_charset_bin);
    protocol->store_string(itr->second.c_str(), itr->second.length(),
                           &my_charset_bin);
    if (protocol->end_row()) {
      errmsg = "Failure during protocol write";
      goto err;
    }
  }

  my_eof(thd);
  return 0;

err:
  my_error(ER_ERROR_WHEN_EXECUTING_COMMAND, MYF(0), "SHOW RAFT STATUS", errmsg);
  return 1;
}

/**
  Function to start a statement and optionally a transaction for the
  binary log.

  This function does three things:
    - Starts a transaction if not in autocommit mode or if a BEGIN
      statement has been seen.

    - Start a statement transaction to allow us to truncate the cache.

    - Save the currrent binlog position so that we can roll back the
      statement by truncating the cache.

      We only update the saved position if the old one was undefined,
      the reason is that there are some cases (e.g., for CREATE-SELECT)
      where the position is saved twice (e.g., both in
      Query_result_create::prepare() and THD::binlog_write_table_map()), but
      we should use the first. This means that calls to this function
      can be used to start the statement before the first table map
      event, to include some extra events.

  Note however that IMMEDIATE_LOGGING implies that the statement is
  written without BEGIN/COMMIT.

  @param thd         Thread variable
  @param start_event The first event requested to be written into the
                     binary log
 */
static int binlog_start_trans_and_stmt(THD *thd, Log_event *start_event) {
  DBUG_TRACE;

  /*
    Initialize the cache manager if this was not done yet.
  */
  if (thd->binlog_setup_trx_data()) return 1;

  /*
    Retrieve the appropriated cache.
  */
  bool is_transactional = start_event->is_using_trans_cache();
  binlog_cache_mngr *cache_mngr = thd_get_cache_mngr(thd);
  binlog_cache_data *cache_data =
      cache_mngr->get_binlog_cache_data(is_transactional);

  /*
    If the event is requesting immediatly logging, there is no need to go
    further down and set savepoint and register callbacks.
  */
  if (start_event->is_using_immediate_logging()) return 0;

  register_binlog_handler(thd, thd->in_multi_stmt_transaction_mode());

  /* Transactional DDL is logged traditionally without BEGIN. */
  if (is_atomic_ddl_event(start_event)) return 0;

  /*
    If the cache is empty log "BEGIN" at the beginning of every transaction.
    Here, a transaction is either a BEGIN..COMMIT/ROLLBACK block or a single
    statement in autocommit mode.
  */
  if (cache_data->is_binlog_empty()) {
    static const char begin[] = "BEGIN";
    const char *query = nullptr;
    char buf[XID::ser_buf_size];
    char xa_start[sizeof("XA START") + 1 + sizeof(buf)];
    XID_STATE *xs = thd->get_transaction()->xid_state();
    int qlen = sizeof(begin) - 1;

    if (is_transactional && xs->has_state(XID_STATE::XA_ACTIVE)) {
      /*
        XA-prepare logging case.
      */
      qlen = sprintf(xa_start, "XA START %s", xs->get_xid()->serialize(buf));
      query = xa_start;
    } else {
      /*
        Regular transaction case.
      */
      query = begin;
    }

    Query_log_event qinfo(thd, query, qlen, is_transactional, false, true, 0,
                          true);
    if (cache_data->write_event(thd, &qinfo)) return 1;
  }

  return 0;
}

/**
  This function generated meta data in JSON format as a comment in a rows query
  event.
  @see binlog_trx_meta_data
  @return JSON if all good, null string otherwise
*/
std::string THD::gen_trx_metadata() {
  DBUG_ENTER("THD::gen_trx_metadata");
  DBUG_ASSERT(opt_binlog_trx_meta_data);

  rapidjson::Document doc;
  doc.SetObject();

  // case: read existing meta data received from the master
  if (rli_slave && !rli_slave->trx_meta_data_json.empty()) {
    if (doc.Parse(rli_slave->trx_meta_data_json.c_str()).HasParseError()) {
      // NO_LINT_DEBUG
      sql_print_error("Exception while reading meta data: %s",
                      rli_slave->trx_meta_data_json.c_str());
      DBUG_RETURN("");
    }
    // clear existing data
    rli_slave->trx_meta_data_json.clear();
  }

  // add things to the meta data
  if (!add_db_metadata(doc) || !add_time_metadata(doc)) {
    // NO_LINT_DEBUG
    sql_print_error("Exception while adding meta data");
    DBUG_RETURN("");
  }

  // write meta data with new stuff in the binlog
  rapidjson::StringBuffer buf;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
  if (!doc.Accept(writer)) {
    // NO_LINT_DEBUG
    sql_print_error("Error while writing meta data");
    DBUG_RETURN("");
  }
  std::string json = buf.GetString();
  boost::trim(json);

  // verify doc json document
  if (!doc.IsObject()) {
    // NO_LINT_DEBUG
    sql_print_error("Bad JSON format after adding meta data: %s", json.c_str());
    DBUG_RETURN("");
  }

  std::string comment_str =
      std::string("/*").append(TRX_META_DATA_HEADER).append(json).append("*/");

  DBUG_RETURN(comment_str);
}

/**
  This function adds timing information in meta data JSON of rows query event.
  @see THD::write_trx_metadata
  @param meta_data_root Property tree object which represents the JSON
  @return true if all good, false if error
*/
bool THD::add_time_metadata(rapidjson::Document &meta_data_root) {
  DBUG_ENTER("THD::add_time_metadata");
  DBUG_ASSERT(opt_binlog_trx_meta_data);

  rapidjson::Document::AllocatorType &allocator = meta_data_root.GetAllocator();

  // get existing timestamps
  auto times = meta_data_root.FindMember("ts");
  if (times == meta_data_root.MemberEnd()) {
    meta_data_root.AddMember("ts", rapidjson::Value().SetArray(), allocator);
    times = meta_data_root.FindMember("ts");
  }

  // add our timestamp to the array
  std::string millis =
      std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count());
  times->value.PushBack(
      rapidjson::Value().SetString(millis.c_str(), millis.size(), allocator),
      allocator);

  DBUG_RETURN(true);
}

bool THD::add_db_metadata(rapidjson::Document &meta_data_root) {
  DBUG_ENTER("THD::add_db_meta_data");
  DBUG_ASSERT(opt_binlog_trx_meta_data);

  mysql_mutex_lock(&LOCK_thd_db_context);
  std::string local_db_metadata =
      m_current_db_context ? m_current_db_context->db_metadata : "";
  mysql_mutex_unlock(&LOCK_thd_db_context);

  if (!local_db_metadata.empty()) {
    rapidjson::Document db_metadata_root;
    // rapidjson doesn't like calling GetObject() on json non-object value
    // The local_db_metadata format should similar to the following example:
    // {"shard":"<shard_name>", "replicaset":"<replicaset_id>"}
    if (db_metadata_root.Parse(local_db_metadata.c_str()).HasParseError() ||
        !db_metadata_root.IsObject()) {
      // NO_LINT_DEBUG
      sql_print_error("Exception while reading meta data: %s",
                      local_db_metadata.c_str());
      DBUG_RETURN(false);
    }

    // flatten DB metadata into trx metadata
    auto &allocator = meta_data_root.GetAllocator();
    for (auto &node : db_metadata_root.GetObject()) {
      rapidjson::Value val(node.value, allocator);
      if (!meta_data_root.HasMember(node.name))
        meta_data_root.AddMember(node.name, val, allocator);
    }
  }

  DBUG_RETURN(true);
}

/**
  This function writes a table map to the binary log.
  Note that in order to keep the signature uniform with related methods,
  we use a redundant parameter to indicate whether a transactional table
  was changed or not.
  Sometimes it will write a Rows_query_log_event into binary log before
  the table map too.

  @param table             a pointer to the table.
  @param is_transactional  @c true indicates a transactional table,
                           otherwise @c false a non-transactional.
  @param binlog_rows_query @c true indicates a Rows_query log event
                           will be binlogged before table map,
                           otherwise @c false indicates it will not
                           be binlogged.
  @return
    nonzero if an error pops up when writing the table map event
    or the Rows_query log event.
*/
int THD::binlog_write_table_map(TABLE *table, bool is_transactional,
                                bool binlog_rows_query) {
  int error;
  DBUG_TRACE;
  DBUG_PRINT("enter", ("table: %p (%s: #%llu)", table, table->s->table_name.str,
                       table->s->table_map_id.id()));

  /* Pre-conditions */
  DBUG_ASSERT(is_current_stmt_binlog_format_row() && mysql_bin_log.is_open());
  DBUG_ASSERT(table->s->table_map_id.is_valid());

  Table_map_log_event the_event(this, table, table->s->table_map_id,
                                is_transactional);

  binlog_start_trans_and_stmt(this, &the_event);

  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(this);

  binlog_cache_data *cache_data =
      cache_mngr->get_binlog_cache_data(is_transactional);

  if (binlog_rows_query) {
    std::string query;
    if (opt_binlog_trx_meta_data) query.append(gen_trx_metadata());
    if (variables.binlog_rows_query_log_events && this->query().str)
      query.append(this->query().str, this->query().length);
    /* Write the Rows_query_log_event into binlog before the table map */
    if (!query.empty()) {
      Rows_query_log_event rows_query_ev(this, query.c_str(), query.length());
      if ((error = cache_data->write_event(this, &rows_query_ev))) return error;
    }
  }

  if ((error = cache_data->write_event(this, &the_event))) return error;

  binlog_table_maps++;
  return 0;
}

/**
  This function retrieves a pending row event from a cache which is
  specified through the parameter @c is_transactional. Respectively, when it
  is @c true, the pending event is returned from the transactional cache.
  Otherwise from the non-transactional cache.

  @param is_transactional  @c true indicates a transactional cache,
                           otherwise @c false a non-transactional.
  @return
    The row event if any.
*/
Rows_log_event *THD::binlog_get_pending_rows_event(
    bool is_transactional) const {
  Rows_log_event *rows = nullptr;
  binlog_cache_mngr *const cache_mngr = thd_get_cache_mngr(this);

  /*
    This is less than ideal, but here's the story: If there is no cache_mngr,
    prepare_pending_rows_event() has never been called (since the cache_mngr
    is set up there). In that case, we just return NULL.
   */
  if (cache_mngr) {
    binlog_cache_data *cache_data =
        cache_mngr->get_binlog_cache_data(is_transactional);

    rows = cache_data->pending();
  }
  return (rows);
}

/**
   @param db_param    db name c-string to be inserted into alphabetically sorted
                THD::binlog_accessed_db_names list.

                Note, that space for both the data and the node
                struct are allocated in THD::main_mem_root.
                The list lasts for the top-level query time and is reset
                in @c THD::cleanup_after_query().
*/
void THD::add_to_binlog_accessed_dbs(const char *db_param) {
  char *after_db;
  /*
    binlog_accessed_db_names list is to maintain the database
    names which are referenced in a given command.
    Prior to bug 17806014 fix, 'main_mem_root' memory root used
    to store this list. The 'main_mem_root' scope is till the end
    of the query. Hence it caused increasing memory consumption
    problem in big procedures like the ones mentioned below.
    Eg: CALL p1() where p1 is having 1,00,000 create and drop tables.
    'main_mem_root' is freed only at the end of the command CALL p1()'s
    execution. But binlog_accessed_db_names list scope is only till the
    individual statements specified the procedure(create/drop statements).
    Hence the memory allocated in 'main_mem_root' was left uncleared
    until the p1's completion, even though it is not required after
    completion of individual statements.

    Instead of using 'main_mem_root' whose scope is complete query execution,
    now the memroot is changed to use 'thd->mem_root' whose scope is until the
    individual statement in CALL p1(). 'thd->mem_root' is set to
    'execute_mem_root' in the context of procedure and it's scope is till the
    individual statement in CALL p1() and thd->memroot is equal to
    'main_mem_root' in the context of a normal 'top level query'.

    Eg: a) create table t1(i int); => If this function is called while
           processing this statement, thd->memroot is equal to &main_mem_root
           which will be freed immediately after executing this statement.
        b) CALL p1() -> p1 contains create table t1(i int); => If this function
           is called while processing create table statement which is inside
           a stored procedure, then thd->memroot is equal to 'execute_mem_root'
           which will be freed immediately after executing this statement.
    In both a and b case, thd->memroot will be freed immediately and will not
    increase memory consumption.

    A special case(stored functions/triggers):
    Consider the following example:
    create function f1(i int) returns int
    begin
      insert into db1.t1 values (1);
      insert into db2.t1 values (2);
    end;
    When we are processing SELECT f1(), the list should contain db1, db2 names.
    Since thd->mem_root contains 'execute_mem_root' in the context of
    stored function, the mem root will be freed after adding db1 in
    the list and when we are processing the second statement and when we try
    to add 'db2' in the db1's list, it will lead to crash as db1's memory
    is already freed. To handle this special case, if in_sub_stmt is set
    (which is true incase of stored functions/triggers), we use &main_mem_root,
    if not set we will use thd->memroot which changes it's value to
    'execute_mem_root' or '&main_mem_root' depends on the context.
   */
  MEM_ROOT *db_mem_root = in_sub_stmt ? &main_mem_root : mem_root;

  if (!binlog_accessed_db_names)
    binlog_accessed_db_names = new (db_mem_root) List<char>;

  if (binlog_accessed_db_names->elements > MAX_DBS_IN_EVENT_MTS) {
    push_warning_printf(
        this, Sql_condition::SL_WARNING, ER_MTS_UPDATED_DBS_GREATER_MAX,
        ER_THD(this, ER_MTS_UPDATED_DBS_GREATER_MAX), MAX_DBS_IN_EVENT_MTS);
    return;
  }

  after_db = strdup_root(db_mem_root, db_param);

  /*
     sorted insertion is implemented with first rearranging data
     (pointer to char*) of the links and final appending of the least
     ordered data to create a new link in the list.
  */
  if (binlog_accessed_db_names->elements != 0) {
    List_iterator<char> it(*get_binlog_accessed_db_names());

    while (it++) {
      char *swap = nullptr;
      char **ref_cur_db = it.ref();
      int cmp = strcmp(after_db, *ref_cur_db);

      DBUG_ASSERT(!swap || cmp < 0);

      if (cmp == 0) {
        after_db = nullptr; /* dup to ignore */
        break;
      } else if (swap || cmp > 0) {
        swap = *ref_cur_db;
        *ref_cur_db = after_db;
        after_db = swap;
      }
    }
  }
  if (after_db) binlog_accessed_db_names->push_back(after_db, db_mem_root);
}

/*
  Tells if two (or more) tables have auto_increment columns and we want to
  lock those tables with a write lock.

  SYNOPSIS
    has_two_write_locked_tables_with_auto_increment
      tables        Table list

  NOTES:
    Call this function only when you have established the list of all tables
    which you'll want to update (including stored functions, triggers, views
    inside your statement).
*/

static bool has_write_table_with_auto_increment(TABLE_LIST *tables) {
  for (TABLE_LIST *table = tables; table; table = table->next_global) {
    /* we must do preliminary checks as table->table may be NULL */
    if (!table->is_placeholder() && table->table->found_next_number_field &&
        (table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE))
      return true;
  }

  return false;
}

/*
   checks if we have select tables in the table list and write tables
   with auto-increment column.

  SYNOPSIS
   has_two_write_locked_tables_with_auto_increment_and_select
      tables        Table list

  RETURN VALUES

   -true if the table list has atleast one table with auto-increment column


         and atleast one table to select from.
   -false otherwise
*/

static bool has_write_table_with_auto_increment_and_select(TABLE_LIST *tables) {
  bool has_select = false;
  bool has_auto_increment_tables = has_write_table_with_auto_increment(tables);
  for (TABLE_LIST *table = tables; table; table = table->next_global) {
    if (!table->is_placeholder() &&
        (table->lock_descriptor().type <= TL_READ_NO_INSERT)) {
      has_select = true;
      break;
    }
  }
  return (has_select && has_auto_increment_tables);
}

/*
  Tells if there is a table whose auto_increment column is a part
  of a compound primary key while is not the first column in
  the table definition.

  @param tables Table list

  @return true if the table exists, fais if does not.
*/

static bool has_write_table_auto_increment_not_first_in_pk(TABLE_LIST *tables) {
  for (TABLE_LIST *table = tables; table; table = table->next_global) {
    /* we must do preliminary checks as table->table may be NULL */
    if (!table->is_placeholder() && table->table->found_next_number_field &&
        (table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) &&
        table->table->s->next_number_keypart != 0)
      return true;
  }

  return false;
}

/**
  Checks if a table has a column with a non-deterministic DEFAULT expression.
*/
static bool has_nondeterministic_default(const TABLE *table) {
  return std::any_of(
      table->field, table->field + table->s->fields, [](const Field *field) {
        return field->m_default_val_expr != nullptr &&
               field->m_default_val_expr->get_stmt_unsafe_flags() != 0;
      });
}

/**
  Checks if a TABLE_LIST contains a table that has been opened for writing, and
  that has a column with a non-deterministic DEFAULT expression.
*/
static bool has_write_table_with_nondeterministic_default(
    const TABLE_LIST *tables) {
  for (const TABLE_LIST *table = tables; table != nullptr;
       table = table->next_global) {
    /* we must do preliminary checks as table->table may be NULL */
    if (!table->is_placeholder() &&
        table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE &&
        has_nondeterministic_default(table->table))
      return true;
  }
  return false;
}

/*
  Function to check whether the table in query uses a fulltext parser
  plugin or not.

  @param s - table share pointer.

  @retval true - The table uses fulltext parser plugin.
  @retval false - Otherwise.
*/
static bool inline fulltext_unsafe_set(TABLE_SHARE *s) {
  for (unsigned int i = 0; i < s->keys; i++) {
    if ((s->key_info[i].flags & HA_USES_PARSER) && s->keys_in_use.is_set(i))
      return true;
  }
  return false;
}
#ifndef DBUG_OFF
const char *get_locked_tables_mode_name(
    enum_locked_tables_mode locked_tables_mode) {
  switch (locked_tables_mode) {
    case LTM_NONE:
      return "LTM_NONE";
    case LTM_LOCK_TABLES:
      return "LTM_LOCK_TABLES";
    case LTM_PRELOCKED:
      return "LTM_PRELOCKED";
    case LTM_PRELOCKED_UNDER_LOCK_TABLES:
      return "LTM_PRELOCKED_UNDER_LOCK_TABLES";
    default:
      return "Unknown table lock mode";
  }
}
#endif

/**
  Decide on logging format to use for the statement and issue errors
  or warnings as needed.  The decision depends on the following
  parameters:

  - The logging mode, i.e., the value of binlog_format.  Can be
    statement, mixed, or row.

  - The type of statement.  There are three types of statements:
    "normal" safe statements; unsafe statements; and row injections.
    An unsafe statement is one that, if logged in statement format,
    might produce different results when replayed on the slave (e.g.,
    queries with a LIMIT clause).  A row injection is either a BINLOG
    statement, or a row event executed by the slave's SQL thread.

  - The capabilities of tables modified by the statement.  The
    *capabilities vector* for a table is a set of flags associated
    with the table.  Currently, it only includes two flags: *row
    capability flag* and *statement capability flag*.

    The row capability flag is set if and only if the engine can
    handle row-based logging. The statement capability flag is set if
    and only if the table can handle statement-based logging.

  Decision table for logging format
  ---------------------------------

  The following table summarizes how the format and generated
  warning/error depends on the tables' capabilities, the statement
  type, and the current binlog_format.

     Row capable        N NNNNNNNNN YYYYYYYYY YYYYYYYYY
     Statement capable  N YYYYYYYYY NNNNNNNNN YYYYYYYYY

     Statement type     * SSSUUUIII SSSUUUIII SSSUUUIII

     binlog_format      * SMRSMRSMR SMRSMRSMR SMRSMRSMR

     Logged format      - SS-S----- -RR-RR-RR SRRSRR-RR
     Warning/Error      1 --2732444 5--5--6-- ---7--6--

  Legend
  ------

  Row capable:    N - Some table not row-capable, Y - All tables row-capable
  Stmt capable:   N - Some table not stmt-capable, Y - All tables stmt-capable
  Statement type: (S)afe, (U)nsafe, or Row (I)njection
  binlog_format:  (S)TATEMENT, (M)IXED, or (R)OW
  Logged format:  (S)tatement or (R)ow
  Warning/Error:  Warnings and error messages are as follows:

  1. Error: Cannot execute statement: binlogging impossible since both
     row-incapable engines and statement-incapable engines are
     involved.

  2. Error: Cannot execute statement: binlogging impossible since
     BINLOG_FORMAT = ROW and at least one table uses a storage engine
     limited to statement-logging.

  3. Error: Cannot execute statement: binlogging of unsafe statement
     is impossible when storage engine is limited to statement-logging
     and BINLOG_FORMAT = MIXED.

  4. Error: Cannot execute row injection: binlogging impossible since
     at least one table uses a storage engine limited to
     statement-logging.

  5. Error: Cannot execute statement: binlogging impossible since
     BINLOG_FORMAT = STATEMENT and at least one table uses a storage
     engine limited to row-logging.

  6. Error: Cannot execute row injection: binlogging impossible since
     BINLOG_FORMAT = STATEMENT.

  7. Warning: Unsafe statement binlogged in statement format since
     BINLOG_FORMAT = STATEMENT.

  In addition, we can produce the following error (not depending on
  the variables of the decision diagram):

  8. Error: Cannot execute statement: binlogging impossible since more
     than one engine is involved and at least one engine is
     self-logging.

  9. Error: Do not allow users to modify a gtid_executed table
     explicitly by a XA transaction.

  For each error case above, the statement is prevented from being
  logged, we report an error, and roll back the statement.  For
  warnings, we set the thd->binlog_flags variable: the warning will be
  printed only if the statement is successfully logged.

  @see THD::binlog_query

  @param[in] tables Tables involved in the query

  @retval 0 No error; statement can be logged.
  @retval -1 One of the error conditions above applies (1, 2, 4, 5, 6 or 9).
*/

int THD::decide_logging_format(TABLE_LIST *tables) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("query: %s", query().str));
  DBUG_PRINT("info", ("variables.binlog_format: %lu", variables.binlog_format));
  DBUG_PRINT("info", ("lex->get_stmt_unsafe_flags(): 0x%x",
                      lex->get_stmt_unsafe_flags()));

#if defined(ENABLED_DEBUG_SYNC)
  if (!is_attachable_ro_transaction_active())
    DEBUG_SYNC(this, "begin_decide_logging_format");
#endif

  reset_binlog_local_stmt_filter();

  /*
    We should not decide logging format if the binlog is closed or
    binlogging is off, or if the statement is filtered out from the
    binlog by filtering rules.
  */
  if (mysql_bin_log.is_open() && (variables.option_bits & OPTION_BIN_LOG) &&
      !(variables.binlog_format == BINLOG_FORMAT_STMT &&
        !binlog_filter->db_ok(m_db.str))) {
    /*
      Compute one bit field with the union of all the engine
      capabilities, and one with the intersection of all the engine
      capabilities.
    */
    handler::Table_flags flags_write_some_set = 0;
    handler::Table_flags flags_access_some_set = 0;
    handler::Table_flags flags_write_all_set =
        HA_BINLOG_ROW_CAPABLE | HA_BINLOG_STMT_CAPABLE;

    /*
       If different types of engines are about to be updated.
       For example: Innodb and Falcon; Innodb and MyIsam.
    */
    bool multi_write_engine = false;
    /*
       If different types of engines are about to be accessed
       and any of them is about to be updated. For example:
       Innodb and Falcon; Innodb and MyIsam.
    */
    bool multi_access_engine = false;
    /*
      Track if statement creates or drops a temporary table
      and log in ROW if it does.
*/
    bool is_create_drop_temp_table = false;
    /*
       Identifies if a table is changed.
    */
    bool is_write = false;
    /*
       A pointer to a previous table that was changed.
    */
    TABLE *prev_write_table = nullptr;
    /*
       A pointer to a previous table that was accessed.
    */
    TABLE *prev_access_table = nullptr;
    /*
      True if at least one table is transactional.
    */
    bool write_to_some_transactional_table = false;
    /*
      True if at least one table is non-transactional.
    */
    bool write_to_some_non_transactional_table = false;
    /*
       True if all non-transactional tables that has been updated
       are temporary.
    */
    bool write_all_non_transactional_are_tmp_tables = true;
    /**
      The number of tables used in the current statement,
      that should be replicated.
    */
    uint replicated_tables_count = 0;
    /**
      The number of tables written to in the current statement,
      that should not be replicated.
      A table should not be replicated when it is considered
      'local' to a MySQL instance.
      Currently, these tables are:
      - mysql.slow_log
      - mysql.general_log
      - mysql.slave_relay_log_info
      - mysql.slave_master_info
      - mysql.slave_worker_info
      - performance_schema.*
      - TODO: information_schema.*
      In practice, from this list, only performance_schema.* tables
      are written to by user queries.
    */
    uint non_replicated_tables_count = 0;
    /**
      Indicate whether we alreadly reported a warning
      on modifying gtid_executed table.
    */
    int warned_gtid_executed_table = 0;
#ifndef DBUG_OFF
    {
      DBUG_PRINT("debug", ("prelocked_mode: %s",
                           get_locked_tables_mode_name(locked_tables_mode)));
    }
#endif

    if (variables.binlog_format != BINLOG_FORMAT_ROW && tables) {
      /*
        DML statements that modify a table with an auto_increment column based
        on rows selected from a table are unsafe as the order in which the rows
        are fetched fron the select tables cannot be determined and may differ
        on master and slave.
       */
      if (has_write_table_with_auto_increment_and_select(tables))
        lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_WRITE_AUTOINC_SELECT);

      if (has_write_table_auto_increment_not_first_in_pk(tables))
        lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_AUTOINC_NOT_FIRST);

      /*
        A query that modifies autoinc column in sub-statement can make the
        master and slave inconsistent.
        We can solve these problems in mixed mode by switching to binlogging
        if at least one updated table is used by sub-statement
       */
      if (lex->requires_prelocking() &&
          has_write_table_with_auto_increment(lex->first_not_own_table()))
        lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_AUTOINC_COLUMNS);

      /*
        A query that modifies a table with a non-deterministic column default
        expression in a substatement, can make the master and the slave
        inconsistent. Switch to row logging in mixed mode, and raise a warning
        in statement mode.
      */
      if (lex->requires_prelocking() &&
          has_write_table_with_nondeterministic_default(
              lex->first_not_own_table()))
        lex->set_stmt_unsafe(
            LEX::BINLOG_STMT_UNSAFE_DEFAULT_EXPRESSION_IN_SUBSTATEMENT);
    }

    /*
      Get the capabilities vector for all involved storage engines and
      mask out the flags for the binary log.
    */
    for (TABLE_LIST *table = tables; table; table = table->next_global) {
      if (table->is_placeholder()) {
        /*
          Detect if this is a CREATE TEMPORARY or DROP of a
          temporary table. This will be used later in determining whether to
          log in ROW or STMT if MIXED replication is being used.
        */
        if (!is_create_drop_temp_table && !table->table &&
            ((lex->sql_command == SQLCOM_CREATE_TABLE &&
              (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)) ||
             ((lex->sql_command == SQLCOM_DROP_TABLE ||
               lex->sql_command == SQLCOM_TRUNCATE) &&
              find_temporary_table(this, table)))) {
          is_create_drop_temp_table = true;
        }
        continue;
      }
      handler::Table_flags const flags = table->table->file->ha_table_flags();

      DBUG_PRINT("info", ("table: %s; ha_table_flags: 0x%llx",
                          table->table_name, flags));

      if (table->table->no_replicate) {
        if (!warned_gtid_executed_table) {
          warned_gtid_executed_table =
              gtid_state->warn_or_err_on_modify_gtid_table(this, table);
          /*
            Do not allow users to modify the gtid_executed table
            explicitly by a XA transaction.
          */
          if (warned_gtid_executed_table == 2) return -1;
        }
        /*
          The statement uses a table that is not replicated.
          The following properties about the table:
          - persistent / transient
          - transactional / non transactional
          - temporary / permanent
          - read or write
          - multiple engines involved because of this table
          are not relevant, as this table is completely ignored.
          Because the statement uses a non replicated table,
          using STATEMENT format in the binlog is impossible.
          Either this statement will be discarded entirely,
          or it will be logged (possibly partially) in ROW format.
        */
        lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_SYSTEM_TABLE);

        if (table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) {
          non_replicated_tables_count++;
          continue;
        }
      }

      replicated_tables_count++;

      bool trans = table->table->file->has_transactions();

      if (table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) {
        write_to_some_transactional_table =
            write_to_some_transactional_table || trans;

        write_to_some_non_transactional_table =
            write_to_some_non_transactional_table || !trans;

        if (prev_write_table &&
            prev_write_table->file->ht != table->table->file->ht)
          multi_write_engine = true;

        if (table->table->s->tmp_table)
          lex->set_stmt_accessed_table(
              trans ? LEX::STMT_WRITES_TEMP_TRANS_TABLE
                    : LEX::STMT_WRITES_TEMP_NON_TRANS_TABLE);
        else
          lex->set_stmt_accessed_table(trans
                                           ? LEX::STMT_WRITES_TRANS_TABLE
                                           : LEX::STMT_WRITES_NON_TRANS_TABLE);

        /*
         Non-transactional updates are allowed when row binlog format is
         used and all non-transactional tables are temporary.
         Binlog format is checked on THD::is_dml_gtid_compatible() method.
        */
        if (!trans)
          write_all_non_transactional_are_tmp_tables =
              write_all_non_transactional_are_tmp_tables &&
              table->table->s->tmp_table;

        flags_write_all_set &= flags;
        flags_write_some_set |= flags;
        is_write = true;

        prev_write_table = table->table;

        /*
          It should be marked unsafe if a table which uses a fulltext parser
          plugin is modified. See also bug#48183.
        */
        if (!lex->is_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_FULLTEXT_PLUGIN)) {
          if (fulltext_unsafe_set(table->table->s))
            lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_FULLTEXT_PLUGIN);
        }
        /*
          INSERT...ON DUPLICATE KEY UPDATE on a table with more than one unique
          keys can be unsafe. Check for it if the flag is already not marked for
          the given statement.
        */
        if (!lex->is_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_INSERT_TWO_KEYS) &&
            lex->sql_command == SQLCOM_INSERT &&
            lex->duplicates == DUP_UPDATE) {
          uint keys = table->table->s->keys, i = 0, unique_keys = 0;
          for (KEY *keyinfo = table->table->s->key_info;
               i < keys && unique_keys <= 1; i++, keyinfo++) {
            if (keyinfo->flags & HA_NOSAME) unique_keys++;
          }
          if (unique_keys > 1)
            lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_INSERT_TWO_KEYS);
        }
      }
      if (lex->get_using_match()) {
        if (fulltext_unsafe_set(table->table->s))
          lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_FULLTEXT_PLUGIN);
      }

      flags_access_some_set |= flags;

      if (lex->sql_command != SQLCOM_CREATE_TABLE ||
          (lex->sql_command == SQLCOM_CREATE_TABLE &&
           ((lex->create_info->options & HA_LEX_CREATE_TMP_TABLE) ||
            (table->lock_descriptor().type < TL_WRITE_ALLOW_WRITE)))) {
        if (table->table->s->tmp_table)
          lex->set_stmt_accessed_table(
              trans ? LEX::STMT_READS_TEMP_TRANS_TABLE
                    : LEX::STMT_READS_TEMP_NON_TRANS_TABLE);
        else
          lex->set_stmt_accessed_table(trans ? LEX::STMT_READS_TRANS_TABLE
                                             : LEX::STMT_READS_NON_TRANS_TABLE);
      }

      if (prev_access_table &&
          prev_access_table->file->ht != table->table->file->ht)
        multi_access_engine = true;

      prev_access_table = table->table;
    }
    DBUG_ASSERT(!is_write || write_to_some_transactional_table ||
                write_to_some_non_transactional_table);
    /*
      write_all_non_transactional_are_tmp_tables may be true if any
      non-transactional table was not updated, so we fix its value here.
    */
    write_all_non_transactional_are_tmp_tables =
        write_all_non_transactional_are_tmp_tables &&
        write_to_some_non_transactional_table;

    DBUG_PRINT("info", ("flags_write_all_set: 0x%llx", flags_write_all_set));
    DBUG_PRINT("info", ("flags_write_some_set: 0x%llx", flags_write_some_set));
    DBUG_PRINT("info",
               ("flags_access_some_set: 0x%llx", flags_access_some_set));
    DBUG_PRINT("info", ("multi_write_engine: %d", multi_write_engine));
    DBUG_PRINT("info", ("multi_access_engine: %d", multi_access_engine));

    int error = 0;
    int unsafe_flags;

    /*
      With transactional data dictionary, CREATE TABLE runs as one statement
      in a multi-statement transaction internally. Revert this for the
      purposes of determining mixed statement safety.
    */
    const bool multi_stmt_trans = lex->sql_command != SQLCOM_CREATE_TABLE &&
                                  in_multi_stmt_transaction_mode();
    bool trans_table = trans_has_updated_trans_table(this);
    bool binlog_direct = variables.binlog_direct_non_trans_update;

    if (lex->is_mixed_stmt_unsafe(multi_stmt_trans, binlog_direct, trans_table,
                                  tx_isolation))
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_MIXED_STATEMENT);
    else if (multi_stmt_trans && trans_table && !binlog_direct &&
             lex->stmt_accessed_table(LEX::STMT_WRITES_NON_TRANS_TABLE))
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_NONTRANS_AFTER_TRANS);

    /*
      If more than one engine is involved in the statement and at
      least one is doing it's own logging (is *self-logging*), the
      statement cannot be logged atomically, so we generate an error
      rather than allowing the binlog to become corrupt.
    */
    if (multi_write_engine && (flags_write_some_set & HA_HAS_OWN_BINLOGGING))
      my_error((error = ER_BINLOG_MULTIPLE_ENGINES_AND_SELF_LOGGING_ENGINE),
               MYF(0));
    else if (multi_access_engine &&
             flags_access_some_set & HA_HAS_OWN_BINLOGGING)
      lex->set_stmt_unsafe(
          LEX::BINLOG_STMT_UNSAFE_MULTIPLE_ENGINES_AND_SELF_LOGGING_ENGINE);

    /* XA is unsafe for statements */
    if (is_write &&
        !get_transaction()->xid_state()->has_state(XID_STATE::XA_NOTR))
      lex->set_stmt_unsafe(LEX::BINLOG_STMT_UNSAFE_XA);

    DBUG_EXECUTE_IF("make_stmt_only_engines",
                    { flags_write_all_set = HA_BINLOG_STMT_CAPABLE; };);

    /* both statement-only and row-only engines involved */
    if ((flags_write_all_set &
         (HA_BINLOG_STMT_CAPABLE | HA_BINLOG_ROW_CAPABLE)) == 0) {
      /*
        1. Error: Binary logging impossible since both row-incapable
           engines and statement-incapable engines are involved
      */
      my_error((error = ER_BINLOG_ROW_ENGINE_AND_STMT_ENGINE), MYF(0));
    }
    /* statement-only engines involved */
    else if ((flags_write_all_set & HA_BINLOG_ROW_CAPABLE) == 0) {
      if (lex->is_stmt_row_injection()) {
        /*
          4. Error: Cannot execute row injection since table uses
             storage engine limited to statement-logging
        */
        my_error((error = ER_BINLOG_ROW_INJECTION_AND_STMT_ENGINE), MYF(0));
      } else if (variables.binlog_format == BINLOG_FORMAT_ROW &&
                 sqlcom_can_generate_row_events(this->lex->sql_command)) {
        /*
          2. Error: Cannot modify table that uses a storage engine
             limited to statement-logging when BINLOG_FORMAT = ROW
        */
        my_error((error = ER_BINLOG_ROW_MODE_AND_STMT_ENGINE), MYF(0));
      } else if (variables.binlog_format == BINLOG_FORMAT_MIXED &&
                 ((unsafe_flags = lex->get_stmt_unsafe_flags()) != 0)) {
        /*
          3. Error: Cannot execute statement: binlogging of unsafe
             statement is impossible when storage engine is limited to
             statement-logging and BINLOG_FORMAT = MIXED.
        */
        for (int unsafe_type = 0; unsafe_type < LEX::BINLOG_STMT_UNSAFE_COUNT;
             unsafe_type++)
          if (unsafe_flags & (1 << unsafe_type))
            my_error(
                (error = ER_BINLOG_UNSAFE_AND_STMT_ENGINE), MYF(0),
                ER_THD_NONCONST(current_thd,
                                LEX::binlog_stmt_unsafe_errcode[unsafe_type]));
      } else if (is_write &&
                 ((unsafe_flags = lex->get_stmt_unsafe_flags()) != 0)) {
        /*
          7. Warning: Unsafe statement logged as statement due to
             binlog_format = STATEMENT
        */
        binlog_unsafe_warning_flags |= unsafe_flags;
        DBUG_PRINT("info", ("Scheduling warning to be issued by "
                            "binlog_query: '%s'",
                            ER_THD(current_thd, ER_BINLOG_UNSAFE_STATEMENT)));
        DBUG_PRINT("info", ("binlog_unsafe_warning_flags: 0x%x",
                            binlog_unsafe_warning_flags));
      }
      /* log in statement format! */
    }
    /* no statement-only engines */
    else {
      /* binlog_format = STATEMENT */
      if (variables.binlog_format == BINLOG_FORMAT_STMT) {
        if (lex->is_stmt_row_injection()) {
          /*
            6. Error: Cannot execute row injection since
               BINLOG_FORMAT = STATEMENT
          */
          my_error((error = ER_BINLOG_ROW_INJECTION_AND_STMT_MODE), MYF(0));
        } else if ((flags_write_all_set & HA_BINLOG_STMT_CAPABLE) == 0 &&
                   sqlcom_can_generate_row_events(this->lex->sql_command)) {
          /*
            5. Error: Cannot modify table that uses a storage engine
               limited to row-logging when binlog_format = STATEMENT
          */
          my_error((error = ER_BINLOG_STMT_MODE_AND_ROW_ENGINE), MYF(0), "");
        } else if (is_write &&
                   (unsafe_flags = lex->get_stmt_unsafe_flags()) != 0) {
          /*
            7. Warning: Unsafe statement logged as statement due to
               binlog_format = STATEMENT
          */
          binlog_unsafe_warning_flags |= unsafe_flags;
          DBUG_PRINT("info", ("Scheduling warning to be issued by "
                              "binlog_query: '%s'",
                              ER_THD(current_thd, ER_BINLOG_UNSAFE_STATEMENT)));
          DBUG_PRINT("info", ("binlog_unsafe_warning_flags: 0x%x",
                              binlog_unsafe_warning_flags));
        }
        /* log in statement format! */
      }
      /* No statement-only engines and binlog_format != STATEMENT.
         I.e., nothing prevents us from row logging if needed. */
      else {
        if (lex->is_stmt_unsafe() || lex->is_stmt_row_injection() ||
            (flags_write_all_set & HA_BINLOG_STMT_CAPABLE) == 0 ||
            lex->stmt_accessed_table(LEX::STMT_READS_TEMP_TRANS_TABLE) ||
            lex->stmt_accessed_table(LEX::STMT_READS_TEMP_NON_TRANS_TABLE) ||
            is_create_drop_temp_table) {
#ifndef DBUG_OFF
          int flags = lex->get_stmt_unsafe_flags();
          DBUG_PRINT("info", ("setting row format for unsafe statement"));
          for (int i = 0; i < Query_tables_list::BINLOG_STMT_UNSAFE_COUNT;
               i++) {
            if (flags & (1 << i))
              DBUG_PRINT(
                  "info",
                  ("unsafe reason: %s",
                   ER_THD_NONCONST(
                       current_thd,
                       Query_tables_list::binlog_stmt_unsafe_errcode[i])));
          }
          DBUG_PRINT("info",
                     ("is_row_injection=%d", lex->is_stmt_row_injection()));
          DBUG_PRINT("info", ("stmt_capable=%llu",
                              (flags_write_all_set & HA_BINLOG_STMT_CAPABLE)));
#endif
          /* log in row format! */
          set_current_stmt_binlog_format_row_if_mixed();
        }
      }
    }

    if (non_replicated_tables_count > 0) {
      if ((replicated_tables_count == 0) || !is_write) {
        DBUG_PRINT("info",
                   ("decision: no logging, no replicated table affected"));
        set_binlog_local_stmt_filter();
      } else {
        if (!is_current_stmt_binlog_format_row()) {
          my_error((error = ER_BINLOG_STMT_MODE_AND_NO_REPL_TABLES), MYF(0));
        } else {
          clear_binlog_local_stmt_filter();
        }
      }
    } else {
      clear_binlog_local_stmt_filter();
    }

    if (!error &&
        !is_dml_gtid_compatible(write_to_some_transactional_table,
                                write_to_some_non_transactional_table,
                                write_all_non_transactional_are_tmp_tables))
      error = 1;

    if (error) {
      DBUG_PRINT("info", ("decision: no logging since an error was generated"));
      return -1;
    }

    if (is_write &&
        lex->sql_command != SQLCOM_END /* rows-event applying by slave */) {
      /*
        Master side of DML in the STMT format events parallelization.
        All involving table db:s are stored in a abc-ordered name list.
        In case the number of databases exceeds MAX_DBS_IN_EVENT_MTS maximum
        the list gathering breaks since it won't be sent to the slave.
      */
      for (TABLE_LIST *table = tables; table; table = table->next_global) {
        if (table->is_placeholder()) continue;

        DBUG_ASSERT(table->table);

        if (table->table->s->is_referenced_by_foreign_key()) {
          /*
             FK-referenced dbs can't be gathered currently. The following
             event will be marked for sequential execution on slave.
          */
          binlog_accessed_db_names = nullptr;
          add_to_binlog_accessed_dbs("");
          break;
        }
        if (!is_current_stmt_binlog_format_row())
          add_to_binlog_accessed_dbs(table->db);
      }
    }
    DBUG_PRINT("info",
               ("decision: logging in %s format",
                is_current_stmt_binlog_format_row() ? "ROW" : "STATEMENT"));

    if (variables.binlog_format == BINLOG_FORMAT_ROW &&
        (lex->sql_command == SQLCOM_UPDATE ||
         lex->sql_command == SQLCOM_UPDATE_MULTI ||
         lex->sql_command == SQLCOM_DELETE ||
         lex->sql_command == SQLCOM_DELETE_MULTI)) {
      String table_names;
      /*
        Generate a warning for UPDATE/DELETE statements that modify a
        BLACKHOLE table, as row events are not logged in row format.
      */
      for (TABLE_LIST *table = tables; table; table = table->next_global) {
        if (table->is_placeholder()) continue;
        if (table->table->file->ht->db_type == DB_TYPE_BLACKHOLE_DB &&
            table->lock_descriptor().type >= TL_WRITE_ALLOW_WRITE) {
          table_names.append(table->table_name);
          table_names.append(",");
        }
      }
      if (!table_names.is_empty()) {
        bool is_update = (lex->sql_command == SQLCOM_UPDATE ||
                          lex->sql_command == SQLCOM_UPDATE_MULTI);
        /*
          Replace the last ',' with '.' for table_names
        */
        table_names.replace(table_names.length() - 1, 1, ".", 1);
        push_warning_printf(
            this, Sql_condition::SL_WARNING, WARN_ON_BLOCKHOLE_IN_RBR,
            ER_THD(this, WARN_ON_BLOCKHOLE_IN_RBR),
            is_update ? "UPDATE" : "DELETE", table_names.c_ptr());
      }
    }
  } else {
    DBUG_PRINT(
        "info",
        ("decision: no logging since "
         "mysql_bin_log.is_open() = %d "
         "and (options & OPTION_BIN_LOG) = 0x%llx "
         "and binlog_format = %lu "
         "and binlog_filter->db_ok(db) = %d",
         mysql_bin_log.is_open(), (variables.option_bits & OPTION_BIN_LOG),
         variables.binlog_format, binlog_filter->db_ok(m_db.str)));

    for (TABLE_LIST *table = tables; table; table = table->next_global) {
      if (!table->is_placeholder() && table->table->no_replicate &&
          gtid_state->warn_or_err_on_modify_gtid_table(this, table))
        break;
    }
  }

#if defined(ENABLED_DEBUG_SYNC)
  if (!is_attachable_ro_transaction_active())
    DEBUG_SYNC(this, "end_decide_logging_format");
#endif

  return 0;
}

/**
  Given that a possible violation of gtid consistency has happened,
  checks if gtid-inconsistencies are forbidden by the current value of
  ENFORCE_GTID_CONSISTENCY and GTID_MODE. If forbidden, generates
  error or warning accordingly.

  @param thd The thread that has issued the GTID-violating statement.

  @param error_code The error code to use, if error or warning is to
  be generated.

  @param log_error_code The error code to use, if error message is to
  be logged.

  @retval false Error was generated.
  @retval true No error was generated (possibly a warning was generated).
*/
static bool handle_gtid_consistency_violation(THD *thd, int error_code,
                                              int log_error_code) {
  DBUG_TRACE;

  enum_gtid_type gtid_next_type = thd->variables.gtid_next.type;
  global_sid_lock->rdlock();
  enum_gtid_consistency_mode gtid_consistency_mode =
      get_gtid_consistency_mode();
  enum_gtid_mode gtid_mode = get_gtid_mode(GTID_MODE_LOCK_SID);

  DBUG_PRINT("info", ("gtid_next.type=%d gtid_mode=%s "
                      "gtid_consistency_mode=%d error=%d query=%s",
                      gtid_next_type, get_gtid_mode_string(gtid_mode),
                      gtid_consistency_mode, error_code, thd->query().str));

  /*
    GTID violations should generate error if:
    - GTID_MODE=ON or ON_PERMISSIVE and GTID_NEXT='AUTOMATIC' (since the
      transaction is expected to commit using a GTID), or
    - GTID_NEXT='UUID:NUMBER' (since the transaction is expected to
      commit usinga GTID), or
    - ENFORCE_GTID_CONSISTENCY=ON.
  */
  if ((gtid_next_type == AUTOMATIC_GTID &&
       gtid_mode >= GTID_MODE_ON_PERMISSIVE) ||
      gtid_next_type == ASSIGNED_GTID ||
      gtid_consistency_mode == GTID_CONSISTENCY_MODE_ON) {
    global_sid_lock->unlock();
    my_error(error_code, MYF(0));
    return false;
  } else {
    /*
      If we are not generating an error, we must increase the counter
      of GTID-violating transactions.  This will prevent a concurrent
      client from executing a SET GTID_MODE or SET
      ENFORCE_GTID_CONSISTENCY statement that would be incompatible
      with this transaction.

      If the transaction had already been accounted as a gtid violating
      transaction, then don't increment the counters, just issue the
      warning below. This prevents calling
      begin_automatic_gtid_violating_transaction or
      begin_anonymous_gtid_violating_transaction multiple times for the
      same transaction, which would make the counter go out of sync.
    */
    if (!thd->has_gtid_consistency_violation) {
      if (gtid_next_type == AUTOMATIC_GTID)
        gtid_state->begin_automatic_gtid_violating_transaction();
      else {
        DBUG_ASSERT(gtid_next_type == ANONYMOUS_GTID);
        gtid_state->begin_anonymous_gtid_violating_transaction();
      }

      /*
        If a transaction generates multiple GTID violation conditions,
        it must still only update the counters once.  Hence we use
        this per-thread flag to keep track of whether the thread has a
        consistency or not.  This function must only be called if the
        transaction does not already have a GTID violation.
      */
      thd->has_gtid_consistency_violation = true;
    }

    global_sid_lock->unlock();

    // Generate warning if ENFORCE_GTID_CONSISTENCY = WARN.
    if (gtid_consistency_mode == GTID_CONSISTENCY_MODE_WARN) {
      // Need to print to log so that replication admin knows when users
      // have adjusted their workloads.
      LogErr(WARNING_LEVEL, log_error_code);
      // Need to print to client so that users can adjust their workload.
      push_warning(thd, Sql_condition::SL_WARNING, error_code,
                   ER_THD_NONCONST(thd, error_code));
    }
    return true;
  }
}

bool THD::is_ddl_gtid_compatible() {
  DBUG_TRACE;

  // If @@session.sql_log_bin has been manually turned off (only
  // doable by SUPER), then no problem, we can execute any statement.
  if ((variables.option_bits & OPTION_BIN_LOG) == 0 ||
      mysql_bin_log.is_open() == false)
    return true;

  DBUG_PRINT("info",
             ("SQLCOM_CREATE:%d CREATE-TMP:%d SELECT:%d SQLCOM_DROP:%d "
              "DROP-TMP:%d trx:%d",
              lex->sql_command == SQLCOM_CREATE_TABLE,
              (lex->sql_command == SQLCOM_CREATE_TABLE &&
               (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)),
              lex->select_lex->item_list.elements,
              lex->sql_command == SQLCOM_DROP_TABLE,
              (lex->sql_command == SQLCOM_DROP_TABLE && lex->drop_temporary),
              in_multi_stmt_transaction_mode()));

  if (lex->sql_command == SQLCOM_CREATE_TABLE &&
      !(lex->create_info->options & HA_LEX_CREATE_TMP_TABLE) &&
      lex->select_lex->item_list.elements) {
    /*
      CREATE ... SELECT (without TEMPORARY) is unsafe because if
      binlog_format=row it will be logged as a CREATE TABLE followed
      by row events, re-executed non-atomically as two transactions,
      and then written to the slave's binary log as two separate
      transactions with the same GTID.
    */
    bool ret = handle_gtid_consistency_violation(
        this, ER_GTID_UNSAFE_CREATE_SELECT,
        ER_RPL_GTID_UNSAFE_STMT_CREATE_SELECT);
    return ret;
  } else if ((lex->sql_command == SQLCOM_CREATE_TABLE &&
              (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE) != 0) ||
             (lex->sql_command == SQLCOM_DROP_TABLE && lex->drop_temporary)) {
    /*
      When @@session.binlog_format=statement, [CREATE|DROP] TEMPORARY TABLE
      is unsafe to execute inside a transaction or Procedure, because the
      [CREATE|DROP] statement on the temporary table will be executed and
      written into binary log with a GTID even if the transaction or
      Procedure is rolled back.
    */
    if (variables.binlog_format == BINLOG_FORMAT_STMT &&
        (in_multi_stmt_transaction_mode() || in_sub_stmt)) {
      bool ret = handle_gtid_consistency_violation(
          this, ER_CLIENT_GTID_UNSAFE_CREATE_DROP_TEMP_TABLE_IN_TRX_IN_SBR,
          ER_SERVER_GTID_UNSAFE_CREATE_DROP_TEMP_TABLE_IN_TRX_IN_SBR);
      return ret;
    }
  }
  return true;
}

bool THD::is_dml_gtid_compatible(bool some_transactional_table,
                                 bool some_non_transactional_table,
                                 bool non_transactional_tables_are_tmp) {
  DBUG_TRACE;

  // If @@session.sql_log_bin has been manually turned off (only
  // doable by SUPER), then no problem, we can execute any statement.
  if ((variables.option_bits & OPTION_BIN_LOG) == 0 ||
      mysql_bin_log.is_open() == false)
    return true;

  /*
    Single non-transactional updates are allowed when not mixed
    together with transactional statements within a transaction.
    Furthermore, writing to transactional and non-transactional
    engines in a single statement is also disallowed.
    Multi-statement transactions on non-transactional tables are
    split into single-statement transactions when
    GTID_NEXT = "AUTOMATIC".

    Non-transactional updates are allowed when row binlog format is
    used and all non-transactional tables are temporary.

    The debug symbol "allow_gtid_unsafe_non_transactional_updates"
    disables the error.  This is useful because it allows us to run
    old tests that were not written with the restrictions of GTIDs in
    mind.
  */
  DBUG_PRINT("info", ("some_non_transactional_table=%d "
                      "some_transactional_table=%d "
                      "trans_has_updated_trans_table=%d "
                      "non_transactional_tables_are_tmp=%d "
                      "is_current_stmt_binlog_format_row=%d",
                      some_non_transactional_table, some_transactional_table,
                      trans_has_updated_trans_table(this),
                      non_transactional_tables_are_tmp,
                      is_current_stmt_binlog_format_row()));
  if (some_non_transactional_table &&
      (some_transactional_table || trans_has_updated_trans_table(this)) &&
      !(non_transactional_tables_are_tmp &&
        is_current_stmt_binlog_format_row()) &&
      !DBUG_EVALUATE_IF("allow_gtid_unsafe_non_transactional_updates", 1, 0)) {
    return handle_gtid_consistency_violation(
        this, ER_GTID_UNSAFE_NON_TRANSACTIONAL_TABLE,
        ER_RPL_GTID_UNSAFE_STMT_ON_NON_TRANS_TABLE);
  }

  return true;
}

/*
  Implementation of interface to write rows to the binary log through the
  thread.  The thread is responsible for writing the rows it has
  inserted/updated/deleted.
*/

/*
  Template member function for ensuring that there is an rows log
  event of the apropriate type before proceeding.

  PRE CONDITION:
    - Events of type 'RowEventT' have the type code 'type_code'.

  POST CONDITION:
    If a non-NULL pointer is returned, the pending event for thread 'thd' will
    be an event of type 'RowEventT' (which have the type code 'type_code')
    will either empty or have enough space to hold 'needed' bytes.  In
    addition, the columns bitmap will be correct for the row, meaning that
    the pending event will be flushed if the columns in the event differ from
    the columns suppled to the function.

  RETURNS
    If no error, a non-NULL pending event (either one which already existed or
    the newly created one).
    If error, NULL.
 */

template <class RowsEventT>
Rows_log_event *THD::binlog_prepare_pending_rows_event(
    TABLE *table, uint32 serv_id, size_t needed, bool is_transactional,
    const unsigned char *extra_row_info, uint32 source_part_id) {
  DBUG_TRACE;

  DBUG_EXECUTE_IF("simulate_null_pending_rows_event", { return nullptr; });

  /* Fetch the type code for the RowsEventT template parameter */
  int const general_type_code = RowsEventT::TYPE_CODE;

  partition_info *part_info = table->part_info;
  auto part_id = get_rpl_part_id(part_info);

  Rows_log_event *pending = binlog_get_pending_rows_event(is_transactional);

  if (unlikely(pending && !pending->is_valid())) return nullptr;

  /*
    Check if the current event is non-NULL and a write-rows
    event. Also check if the table provided is mapped: if it is not,
    then we have switched to writing to a new table.
    If there is no pending event, we need to create one. If there is a pending
    event, but it's not about the same table id, or not of the same type
    (between Write, Update and Delete), or not the same affected columns, or
    going to be too big, flush this event to disk and create a new pending
    event.

    We do not need to check that the pending event and the new event
    have the same setting for partial json updates, because
    partialness of json can only be changed outside transactions.
  */
  if (!pending || pending->server_id != serv_id ||
      pending->get_table_id() != table->s->table_map_id ||
      pending->get_general_type_code() != general_type_code ||
      pending->get_data_size() + needed > binlog_row_event_max_size ||
      pending->m_row_count >= opt_binlog_rows_event_max_rows ||
      pending->read_write_bitmaps_cmp(table) == false ||
      !(pending->m_extra_row_info.compare_extra_row_info(
          extra_row_info, part_id, source_part_id))) {
    /* Create a new RowsEventT... */
    Rows_log_event *const ev = new RowsEventT(
        this, table, table->s->table_map_id, is_transactional, extra_row_info);
    if (unlikely(!ev)) return nullptr;
    ev->server_id = serv_id;  // I don't like this, it's too easy to forget.
    /*
      flush the pending event and replace it with the newly created
      event...
    */
    if (unlikely(mysql_bin_log.flush_and_set_pending_rows_event(
            this, ev, is_transactional))) {
      delete ev;
      return nullptr;
    }

    return ev; /* This is the new pending event */
  }
  return pending; /* This is the current pending event */
}

/* Declare in unnamed namespace. */
namespace {

/**
   Class to handle temporary allocation of memory for row data.

   The responsibilities of the class is to provide memory for
   packing one or two rows of packed data (depending on what
   constructor is called).

   In order to make the allocation more efficient for rows without blobs,
   a pointer to the allocated memory is stored in the table structure
   for such rows.  If memory for a table containing a blob field
   is requested, only memory for that is allocated, and subsequently
   released when the object is destroyed.

 */
class Row_data_memory {
 public:
  /**
    Build an object to keep track of a block-local piece of memory
    for storing a row of data.

    @param table
    Table where the pre-allocated memory is stored.

    @param data
    Pointer to the table record.
   */
  Row_data_memory(TABLE *table, const uchar *data) : m_memory(nullptr) {
#ifndef DBUG_OFF
    m_alloc_checked = false;
#endif
    allocate_memory(table, max_row_length(table, data));
    m_ptr[0] = has_memory() ? m_memory : nullptr;
    m_ptr[1] = nullptr;
  }

  Row_data_memory(TABLE *table, const uchar *data1, const uchar *data2,
                  ulonglong value_options = 0)
      : m_memory(nullptr) {
#ifndef DBUG_OFF
    m_alloc_checked = false;
#endif
    size_t len1 = max_row_length(table, data1);
    size_t len2 = max_row_length(table, data2, value_options);
    allocate_memory(table, len1 + len2);
    m_ptr[0] = has_memory() ? m_memory : nullptr;
    m_ptr[1] = has_memory() ? m_memory + len1 : nullptr;
  }

  ~Row_data_memory() {
    if (m_memory != nullptr && m_release_memory_on_destruction)
      my_free(m_memory);
  }

  /**
     Is there memory allocated?

     @retval true There is memory allocated
     @retval false Memory allocation failed
   */
  bool has_memory() const {
#ifndef DBUG_OFF
    m_alloc_checked = true;
#endif
    return m_memory != nullptr;
  }

  uchar *slot(uint s) {
    DBUG_ASSERT(s < sizeof(m_ptr) / sizeof(*m_ptr));
    DBUG_ASSERT(m_ptr[s] != nullptr);
    DBUG_ASSERT(m_alloc_checked == true);
    return m_ptr[s];
  }

 private:
  /**
    Compute an upper bound on the amount of memory needed.

    This may return an over-approximation.

    @param table The table
    @param data The server's row record.
    @param value_options The value of @@global.binlog_row_value_options
  */
  size_t max_row_length(TABLE *table, const uchar *data,
                        ulonglong value_options = 0) {
    TABLE_SHARE *table_s = table->s;
    Replicated_columns_view fields{table, Replicated_columns_view::OUTBOUND};
    /*
      The server stores rows using "records".  A record is a sequence of bytes
      which contains values or pointers to values for all fields (columns).  The
      server uses table_s->reclength bytes for a row record.

      The layout of a record is roughly:

      - N+1+B bits, packed into CEIL((N+1+B)/8) bytes, where N is the number of
        nullable columns in the table, and B is the sum of the number of bits of
        all BIT columns.

      - A sequence of serialized fields, each corresponding to a non-BIT,
        non-NULL column in the table.

        For variable-length columns, the first component of the serialized field
        is a length, stored using 1, 2, 3, or 4 bytes depending on the maximum
        length for the data type.

        For most data types, the next component of the serialized field is the
        actual data.  But for for VARCHAR, VARBINARY, TEXT, BLOB, and JSON, the
        next component of the serialized field is a serialized pointer,
        i.e. sizeof(pointer) bytes, which point to another memory area where the
        actual data is stored.

      The layout of a row image in the binary log is roughly:

      - If this is an after-image and partial JSON is enabled, 1 byte containing
        value_options.  If the PARTIAL_JSON bit of value_options is set, this is
        followed by P bits (the "partial_bits"), packed into CEIL(P) bytes,
        where P is the number of JSON columns in the table.

      - M bits (the "null_bits"), packed into CEIL(M) bytes, where M is the
        number of columns in the image.

      - A sequence of serialized fields, each corresponding to a non-NULL column
        in the row image.

        For variable-length columns, the first component of the serialized field
        is a length, stored using 1, 2, 3, or 4 bytes depending on the maximum
        length for the data type.

        For most data types, the next component of the serialized field is the
        actual field data.  But for JSON fields where the corresponding bit of
        the partial_bits is 1, this is a sequence of diffs instead.

      Now we try to use table_s->reclength to estimate how much memory to
      allocate for a row image in the binlog.  Due to the differences this will
      only be an upper bound.  Notice the differences:

      - The binlog may only include a subset of the fields (the row image),
        whereas reclength contains space for all fields.

      - BIT columns are not packed together with NULL bits in the binlog, so up
        to 1 more byte per BIT column may be needed.

      - The binlog has a null bit even for non-nullable fields, whereas the
        reclength only contains space nullable fields, so the binlog may need up
        to CEIL(table_s->fields/8) more bytes.

      - The binlog only has a null bit for fields in the image, whereas the
        reclength contains space for all fields.

      - The binlog contains the full blob whereas the record only contains
        sizeof(pointer) bytes.

      - The binlog contains value_options and partial_bits.  So this may use up
        to 1+CEIL(table_s->fields/8) more bytes.

      - The binlog may contain partial JSON.  This is guaranteed to be smaller
        than the size of the full value.

      - There may exist columns that, due to their nature, are not replicated,
        for instance, hidden generated columns used for functional indexes.

      For those data types that are not stored using a pointer, the size of the
      field in the binary log is at most 2 bytes more than what the field
      contributes to in table_s->reclength, because those data types use at most
      1 byte for the length and waste less than a byte on extra padding and
      extra bits in null_bits or BIT columns.

      For those data types that are stored using a pointer, the size of the
      field in the binary log is at most 2 bytes more than what the field
      contributes to in table_s->reclength, plus the size of the data.  The size
      of the pointer is at least 4 on all supported platforms, so it is bigger
      than what is used by partial_bits, value_format, or any waste due to extra
      padding and extra bits in null_bits.
    */
    size_t length = table_s->reclength + 2 * (fields.filtered_size());

    for (uint i = 0; i < table_s->blob_fields; i++) {
      if (fields.is_excluded(table_s->blob_field[i])) continue;

      Field *field = table->field[table_s->blob_field[i]];
      Field_blob *field_blob = down_cast<Field_blob *>(field);

      if (field_blob->type() == MYSQL_TYPE_JSON &&
          (value_options & PARTIAL_JSON_UPDATES) != 0) {
        Field_json *field_json = down_cast<Field_json *>(field_blob);
        length += field_json->get_diff_vector_and_length(value_options);
      } else
        length +=
            field_blob->get_length(data + field_blob->offset(table->record[0]));
    }
    return length;
  }

  void allocate_memory(TABLE *const table, const size_t total_length) {
    if (table->s->blob_fields == 0) {
      /*
        The maximum length of a packed record is less than this
        length. We use this value instead of the supplied length
        when allocating memory for records, since we don't know how
        the memory will be used in future allocations.

        Since table->s->reclength is for unpacked records, we have
        to add two bytes for each field, which can potentially be
        added to hold the length of a packed field.
      */
      size_t const maxlen = table->s->reclength + 2 * table->s->fields;

      /*
        Allocate memory for two records if memory hasn't been
        allocated. We allocate memory for two records so that it can
        be used when processing update rows as well.
      */
      if (table->write_row_record == nullptr)
        table->write_row_record = (uchar *)table->mem_root.Alloc(2 * maxlen);
      m_memory = table->write_row_record;
      m_release_memory_on_destruction = false;
    } else {
      m_memory = (uchar *)my_malloc(key_memory_Row_data_memory_memory,
                                    total_length, MYF(MY_WME));
      m_release_memory_on_destruction = true;
    }
  }

#ifndef DBUG_OFF
  mutable bool m_alloc_checked;
#endif
  bool m_release_memory_on_destruction;
  uchar *m_memory;
  uchar *m_ptr[2];
};

}  // namespace

int THD::binlog_write_row(TABLE *table, bool is_trans, uchar const *record,
                          const unsigned char *extra_row_info) {
  DBUG_ASSERT(is_current_stmt_binlog_format_row() && mysql_bin_log.is_open());

  /*
    Pack records into format for transfer. We are allocating more
    memory than needed, but that doesn't matter.
  */
  Row_data_memory memory(table, record);
  if (!memory.has_memory()) return HA_ERR_OUT_OF_MEM;

  uchar *row_data = memory.slot(0);

  size_t const len = pack_row(table, table->write_set, row_data, record,
                              enum_row_image_type::WRITE_AI);

  Rows_log_event *const ev =
      binlog_prepare_pending_rows_event<Write_rows_log_event>(
          table, server_id, len, is_trans, extra_row_info);

  if (unlikely(ev == nullptr)) return HA_ERR_OUT_OF_MEM;

  return ev->add_row_data(row_data, len);
}

int THD::binlog_update_row(TABLE *table, bool is_trans,
                           const uchar *before_record,
                           const uchar *after_record,
                           const unsigned char *extra_row_info) {
  DBUG_ASSERT(is_current_stmt_binlog_format_row() && mysql_bin_log.is_open());
  int error = 0;

  /**
    Save a reference to the original read and write set bitmaps.
    We will need this to restore the bitmaps at the end.
   */
  MY_BITMAP *old_read_set = table->read_set;
  MY_BITMAP *old_write_set = table->write_set;

  /**
     This will remove spurious fields required during execution but
     not needed for binlogging. This is done according to the:
     binlog-row-image option.
   */
  binlog_prepare_row_images(this, table, true);

  Row_data_memory row_data(table, before_record, after_record,
                           variables.binlog_row_value_options);
  if (!row_data.has_memory()) return HA_ERR_OUT_OF_MEM;

  uchar *before_row = row_data.slot(0);
  uchar *after_row = row_data.slot(1);

  size_t const before_size =
      pack_row(table, table->read_set, before_row, before_record,
               enum_row_image_type::UPDATE_BI);
  size_t const after_size = pack_row(
      table, table->write_set, after_row, after_record,
      enum_row_image_type::UPDATE_AI, variables.binlog_row_value_options);

  DBUG_DUMP("before_record", before_record, table->s->reclength);
  DBUG_DUMP("after_record", after_record, table->s->reclength);
  DBUG_DUMP("before_row", before_row, before_size);
  DBUG_DUMP("after_row", after_row, after_size);

  partition_info *part_info = table->part_info;
  uint32 source_part_id = binary_log::Rows_event::Extra_row_info::UNDEFINED;
  if (part_info) {
    uint32 new_part_id = binary_log::Rows_event::Extra_row_info::UNDEFINED;
    longlong func_value = 0;
    get_parts_for_update(before_record, after_record, table->record[0],
                         part_info, &source_part_id, &new_part_id, &func_value);
  }

  Rows_log_event *const ev =
      binlog_prepare_pending_rows_event<Update_rows_log_event>(
          table, server_id, before_size + after_size, is_trans, extra_row_info,
          source_part_id);

  if (unlikely(ev == nullptr)) return HA_ERR_OUT_OF_MEM;

  if (part_info) {
    ev->m_extra_row_info.set_source_partition_id(source_part_id);
  }

  error = ev->add_row_data(before_row, before_size) ||
          ev->add_row_data(after_row, after_size);

  /* restore read/write set for the rest of execution */
  table->column_bitmaps_set_no_signal(old_read_set, old_write_set);

  bitmap_clear_all(&table->tmp_set);

  return error;
}

int THD::binlog_delete_row(TABLE *table, bool is_trans, uchar const *record,
                           const unsigned char *extra_row_info) {
  DBUG_ASSERT(is_current_stmt_binlog_format_row() && mysql_bin_log.is_open());
  int error = 0;

  /**
    Save a reference to the original read and write set bitmaps.
    We will need this to restore the bitmaps at the end.
   */
  MY_BITMAP *old_read_set = table->read_set;
  MY_BITMAP *old_write_set = table->write_set;

  /**
     This will remove spurious fields required during execution but
     not needed for binlogging. This is done according to the:
     binlog-row-image option.
   */
  binlog_prepare_row_images(this, table, false);

  /*
     Pack records into format for transfer. We are allocating more
     memory than needed, but that doesn't matter.
  */
  Row_data_memory memory(table, record);
  if (unlikely(!memory.has_memory())) return HA_ERR_OUT_OF_MEM;

  uchar *row_data = memory.slot(0);

  DBUG_DUMP("table->read_set", (uchar *)table->read_set->bitmap,
            (table->s->fields + 7) / 8);
  size_t const len = pack_row(table, table->read_set, row_data, record,
                              enum_row_image_type::DELETE_BI);

  Rows_log_event *const ev =
      binlog_prepare_pending_rows_event<Delete_rows_log_event>(
          table, server_id, len, is_trans, extra_row_info);

  if (unlikely(ev == nullptr)) return HA_ERR_OUT_OF_MEM;

  error = ev->add_row_data(row_data, len);

  /* restore read/write set for the rest of execution */
  table->column_bitmaps_set_no_signal(old_read_set, old_write_set);

  bitmap_clear_all(&table->tmp_set);
  return error;
}

void binlog_prepare_row_images(const THD *thd, TABLE *table, bool is_update) {
  DBUG_TRACE;
  /**
    Remove spurious columns. The write_set has been partially
    handled before in table->mark_columns_needed_for_update.
   */

  DBUG_PRINT_BITSET("debug", "table->read_set (before preparing): %s",
                    table->read_set);

  /* Handle the read set */
  /**
    if there is a primary key in the table (ie, user declared PK or a
    non-null unique index) and we dont want to ship the entire image,
    and the handler involved supports this.
   */
  if (table->s->primary_key < MAX_KEY &&
      (thd->variables.binlog_row_image < BINLOG_ROW_IMAGE_FULL) &&
      !ha_check_storage_engine_flag(table->s->db_type(),
                                    HTON_NO_BINLOG_ROW_OPT)) {
    /**
      Just to be sure that tmp_set is currently not in use as
      the read_set already.
    */
    DBUG_ASSERT(table->read_set != &table->tmp_set);
    // Verify it's not used
    DBUG_ASSERT(bitmap_is_clear_all(&table->tmp_set));

    switch (thd->variables.binlog_row_image) {
      case BINLOG_ROW_IMAGE_MINIMAL:
        /* MINIMAL: Mark only PK */
        table->mark_columns_used_by_index_no_reset(table->s->primary_key,
                                                   &table->tmp_set);
        break;
      case BINLOG_ROW_IMAGE_NOBLOB:
        /**
          NOBLOB: Remove unnecessary BLOB fields from read_set
                  (the ones that are not part of PK).
         */
        bitmap_union(&table->tmp_set, table->read_set);
        for (Field **ptr = table->field; *ptr; ptr++) {
          Field *field = (*ptr);
          if ((field->type() == MYSQL_TYPE_BLOB) &&
              !(field->flags & PRI_KEY_FLAG))
            bitmap_clear_bit(&table->tmp_set, field->field_index);
        }
        break;
      default:
        DBUG_ASSERT(0);  // impossible.
    }

    /* set the temporary read_set */
    table->column_bitmaps_set_no_signal(&table->tmp_set, table->write_set);
  }

  /* Now, handle the write set */
  if (is_update && thd->variables.binlog_row_image != BINLOG_ROW_IMAGE_FULL &&
      !ha_check_storage_engine_flag(table->s->db_type(),
                                    HTON_NO_BINLOG_ROW_OPT)) {
    /**
      Just to be sure that tmp_write_set is currently not in use as
      the write_set already.
    */
    DBUG_ASSERT(table->write_set != &table->tmp_write_set);

    bitmap_copy(&table->tmp_write_set, table->write_set);

    for (Field **ptr = table->field; *ptr; ptr++) {
      Field *field = (*ptr);
      if (bitmap_is_set(&table->tmp_write_set, field->field_index)) {
        /* When image type is NOBLOB, we prune only BLOB fields */
        if (thd->variables.binlog_row_image == BINLOG_ROW_IMAGE_NOBLOB &&
            field->type() != MYSQL_TYPE_BLOB)
          continue;

        /* compare null bit */
        if (field->is_null() && field->is_null_in_record(table->record[1]))
          bitmap_clear_bit(&table->tmp_write_set, field->field_index);

        /* compare content, only if fields are not set to NULL */
        else if (!field->is_null() &&
                 !field->is_null_in_record(table->record[1]) &&
                 !field->cmp_binary_offset(table->s->rec_buff_length))
          bitmap_clear_bit(&table->tmp_write_set, field->field_index);
      }
    }
    table->column_bitmaps_set_no_signal(table->read_set, &table->tmp_write_set);
  }

  DBUG_PRINT_BITSET("debug", "table->read_set (after preparing): %s",
                    table->read_set);
}

int THD::binlog_flush_pending_rows_event(bool stmt_end, bool is_transactional) {
  DBUG_TRACE;
  /*
    We shall flush the pending event even if we are not in row-based
    mode: it might be the case that we left row-based mode before
    flushing anything (e.g., if we have explicitly locked tables).
   */
  if (!mysql_bin_log.is_open()) return 0;

  /*
    Mark the event as the last event of a statement if the stmt_end
    flag is set.
  */
  int error = 0;
  if (Rows_log_event *pending =
          binlog_get_pending_rows_event(is_transactional)) {
    if (stmt_end) {
      pending->set_flags(Rows_log_event::STMT_END_F);
      binlog_table_maps = 0;
    }

    error = mysql_bin_log.flush_and_set_pending_rows_event(this, nullptr,
                                                           is_transactional);
  }

  return error;
}

void THD::binlog_reset_pending_rows_event(bool is_transactional) {
  DBUG_ENTER("THD::binlog_reset_pending_rows_event");
  auto cache_mngr = thd_get_cache_mngr(this);
  if (!cache_mngr) DBUG_VOID_RETURN;

  auto cache_data = cache_mngr->get_binlog_cache_data(is_transactional);
  DBUG_ASSERT(cache_data != nullptr);
  cache_data->remove_pending_event();
  DBUG_ASSERT(binlog_get_pending_rows_event(is_transactional) == nullptr);
  DBUG_VOID_RETURN;
}

#if !defined(DBUG_OFF)
static const char *show_query_type(THD::enum_binlog_query_type qtype) {
  switch (qtype) {
    case THD::ROW_QUERY_TYPE:
      return "ROW";
    case THD::STMT_QUERY_TYPE:
      return "STMT";
    case THD::QUERY_TYPE_COUNT:
    default:
      DBUG_ASSERT(0 <= qtype && qtype < THD::QUERY_TYPE_COUNT);
  }
  static char buf[64];
  sprintf(buf, "UNKNOWN#%d", qtype);
  return buf;
}
#endif

/**
  Auxiliary function to reset the limit unsafety warning suppression.
*/
static void reset_binlog_unsafe_suppression() {
  DBUG_TRACE;
  unsafe_warning_suppression_is_activated = false;
  limit_unsafe_warning_count = 0;
  limit_unsafe_suppression_start_time = my_getsystime() / 10000000;
}

/**
  Auxiliary function to print warning in the error log.
*/
static void print_unsafe_warning_to_log(int unsafe_type, char *buf,
                                        const char *query) {
  DBUG_TRACE;
  sprintf(buf, ER_DEFAULT(ER_BINLOG_UNSAFE_STATEMENT),
          ER_DEFAULT_NONCONST(LEX::binlog_stmt_unsafe_errcode[unsafe_type]));
  LogErr(WARNING_LEVEL, ER_BINLOG_UNSAFE_MESSAGE_AND_STATEMENT, buf, query);
}

/**
  Auxiliary function to check if the warning for limit unsafety should be
  thrown or suppressed. Details of the implementation can be found in the
  comments inline.

  @param buf         Buffer to hold the warning message text
  @param unsafe_type The type of unsafety.
  @param query       The actual query statement.

  TODO: Remove this function and implement a general service for all warnings
  that would prevent flooding the error log. => switch to log_throttle class?
*/
static void do_unsafe_limit_checkout(char *buf, int unsafe_type,
                                     const char *query) {
  ulonglong now;
  DBUG_TRACE;
  DBUG_ASSERT(unsafe_type == LEX::BINLOG_STMT_UNSAFE_LIMIT);
  limit_unsafe_warning_count++;
  /*
    INITIALIZING:
    If this is the first time this function is called with log warning
    enabled, the monitoring the unsafe warnings should start.
  */
  if (limit_unsafe_suppression_start_time == 0) {
    limit_unsafe_suppression_start_time = my_getsystime() / 10000000;
    print_unsafe_warning_to_log(unsafe_type, buf, query);
  } else {
    if (!unsafe_warning_suppression_is_activated)
      print_unsafe_warning_to_log(unsafe_type, buf, query);

    if (limit_unsafe_warning_count >=
        LIMIT_UNSAFE_WARNING_ACTIVATION_THRESHOLD_COUNT) {
      now = my_getsystime() / 10000000;
      if (!unsafe_warning_suppression_is_activated) {
        /*
          ACTIVATION:
          We got LIMIT_UNSAFE_WARNING_ACTIVATION_THRESHOLD_COUNT warnings in
          less than LIMIT_UNSAFE_WARNING_ACTIVATION_TIMEOUT we activate the
          suppression.
        */
        if ((now - limit_unsafe_suppression_start_time) <=
            LIMIT_UNSAFE_WARNING_ACTIVATION_TIMEOUT) {
          unsafe_warning_suppression_is_activated = true;
          DBUG_PRINT("info", ("A warning flood has been detected and the limit \
unsafety warning suppression has been activated."));
        } else {
          /*
           there is no flooding till now, therefore we restart the monitoring
          */
          limit_unsafe_suppression_start_time = my_getsystime() / 10000000;
          limit_unsafe_warning_count = 0;
        }
      } else {
        /*
          Print the suppression note and the unsafe warning.
        */
        LogErr(INFORMATION_LEVEL, ER_BINLOG_WARNING_SUPPRESSED,
               limit_unsafe_warning_count,
               (int)(now - limit_unsafe_suppression_start_time));
        print_unsafe_warning_to_log(unsafe_type, buf, query);
        /*
          DEACTIVATION: We got LIMIT_UNSAFE_WARNING_ACTIVATION_THRESHOLD_COUNT
          warnings in more than  LIMIT_UNSAFE_WARNING_ACTIVATION_TIMEOUT, the
          suppression should be deactivated.
        */
        if ((now - limit_unsafe_suppression_start_time) >
            LIMIT_UNSAFE_WARNING_ACTIVATION_TIMEOUT) {
          reset_binlog_unsafe_suppression();
          DBUG_PRINT("info", ("The limit unsafety warning supression has been \
deactivated"));
        }
      }
      limit_unsafe_warning_count = 0;
    }
  }
}

/**
  Auxiliary method used by @c binlog_query() to raise warnings.

  The type of warning and the type of unsafeness is stored in
  THD::binlog_unsafe_warning_flags.
*/
void THD::issue_unsafe_warnings() {
  char buf[MYSQL_ERRMSG_SIZE * 2];
  DBUG_TRACE;
  /*
    Ensure that binlog_unsafe_warning_flags is big enough to hold all
    bits.  This is actually a constant expression.
  */
  DBUG_ASSERT(LEX::BINLOG_STMT_UNSAFE_COUNT <=
              sizeof(binlog_unsafe_warning_flags) * CHAR_BIT);

  uint32 unsafe_type_flags = binlog_unsafe_warning_flags;

  /*
    For each unsafe_type, check if the statement is unsafe in this way
    and issue a warning.
  */
  for (int unsafe_type = 0; unsafe_type < LEX::BINLOG_STMT_UNSAFE_COUNT;
       unsafe_type++) {
    if ((unsafe_type_flags & (1 << unsafe_type)) != 0) {
      push_warning_printf(
          this, Sql_condition::SL_NOTE, ER_BINLOG_UNSAFE_STATEMENT,
          ER_THD(this, ER_BINLOG_UNSAFE_STATEMENT),
          ER_THD_NONCONST(this, LEX::binlog_stmt_unsafe_errcode[unsafe_type]));
      if (log_error_verbosity > 1 && opt_log_unsafe_statements) {
        if (unsafe_type == LEX::BINLOG_STMT_UNSAFE_LIMIT)
          do_unsafe_limit_checkout(buf, unsafe_type, query().str);
        else  // cases other than LIMIT unsafety
          print_unsafe_warning_to_log(unsafe_type, buf, query().str);
      }
    }
  }
}

/**
  Log the current query.

  The query will be logged in either row format or statement format
  depending on the value of @c current_stmt_binlog_format_row field and
  the value of the @c qtype parameter.

  This function must be called:

  - After the all calls to ha_*_row() functions have been issued.

  - After any writes to system tables. Rationale: if system tables
    were written after a call to this function, and the master crashes
    after the call to this function and before writing the system
    tables, then the master and slave get out of sync.

  - Before tables are unlocked and closed.

  @see decide_logging_format

  @retval 0 Success

  @retval nonzero If there is a failure when writing the query (e.g.,
  write failure), then the error code is returned.
*/
int THD::binlog_query(THD::enum_binlog_query_type qtype, const char *query_arg,
                      size_t query_len, bool is_trans, bool direct,
                      bool suppress_use, int errcode) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("qtype: %s  query: '%s'", show_query_type(qtype), query_arg));
  DBUG_ASSERT(query_arg && mysql_bin_log.is_open());

  if (get_binlog_local_stmt_filter() == BINLOG_FILTER_SET) {
    /*
      The current statement is to be ignored, and not written to
      the binlog. Do not call issue_unsafe_warnings().
    */
    return 0;
  }

  /*
    If we are not in prelocked mode, mysql_unlock_tables() will be
    called after this binlog_query(), so we have to flush the pending
    rows event with the STMT_END_F set to unlock all tables at the
    slave side as well.

    If we are in prelocked mode, the flushing will be done inside the
    top-most close_thread_tables().
  */
  if (this->locked_tables_mode <= LTM_LOCK_TABLES)
    if (int error = binlog_flush_pending_rows_event(true, is_trans))
      return error;

  /*
    Warnings for unsafe statements logged in statement format are
    printed in three places instead of in decide_logging_format().
    This is because the warnings should be printed only if the statement
    is actually logged. When executing decide_logging_format(), we cannot
    know for sure if the statement will be logged:

    1 - sp_head::execute_procedure which prints out warnings for calls to
    stored procedures.

    2 - sp_head::execute_function which prints out warnings for calls
    involving functions.

    3 - THD::binlog_query (here) which prints warning for top level
    statements not covered by the two cases above: i.e., if not insided a
    procedure and a function.

    Besides, we should not try to print these warnings if it is not
    possible to write statements to the binary log as it happens when
    the execution is inside a function, or generaly speaking, when
    the variables.option_bits & OPTION_BIN_LOG is false.
  */
  if ((variables.option_bits & OPTION_BIN_LOG) && sp_runtime_ctx == nullptr &&
      !binlog_evt_union.do_union)
    issue_unsafe_warnings();

  switch (qtype) {
      /*
        ROW_QUERY_TYPE means that the statement may be logged either in
        row format or in statement format.  If
        current_stmt_binlog_format is row, it means that the
        statement has already been logged in row format and hence shall
        not be logged again.
      */
    case THD::ROW_QUERY_TYPE:
      DBUG_PRINT("debug", ("is_current_stmt_binlog_format_row: %d",
                           is_current_stmt_binlog_format_row()));
      if (is_current_stmt_binlog_format_row()) return 0;
      /* Fall through */

      /*
        STMT_QUERY_TYPE means that the query must be logged in statement
        format; it cannot be logged in row format.  This is typically
        used by DDL statements.  It is an error to use this query type
        if current_stmt_binlog_format_row is row.

        @todo Currently there are places that call this method with
        STMT_QUERY_TYPE and current_stmt_binlog_format is row.  Fix those
        places and add assert to ensure correct behavior. /Sven
      */
    case THD::STMT_QUERY_TYPE:
      /*
        The MYSQL_BIN_LOG::write() function will set the STMT_END_F flag and
        flush the pending rows event if necessary.
      */
      {
        Query_log_event qinfo(this, query_arg, query_len, is_trans, direct,
                              suppress_use, errcode);
        /*
          Binlog table maps will be irrelevant after a Query_log_event
          (they are just removed on the slave side) so after the query
          log event is written to the binary log, we pretend that no
          table maps were written.
         */
        int error = mysql_bin_log.write_event(&qinfo, opt_binlog_trx_meta_data);
        binlog_table_maps = 0;
        return error;
      }
      break;

    case THD::QUERY_TYPE_COUNT:
    default:
      DBUG_ASSERT(0 <= qtype && qtype < QUERY_TYPE_COUNT);
  }
  return 0;
}

int trim_logged_gtid(const std::vector<std::string> &trimmed_gtids) {
  if (trimmed_gtids.empty()) return 0;

  global_sid_lock->rdlock();
  int error = gtid_state->remove_logged_gtid_on_trim(trimmed_gtids);
  Master_info *active_mi = nullptr;
  if (!get_and_lock_master_info(&active_mi)) {
    // NO_LINT_DEBUG
    sql_print_information(
        "active_mi or rli is not set. Hence not trimming "
        "logged gtids from rli");
  }
  if (active_mi && active_mi->rli) {
    // Remove rli logged gtids. Note that retrieved gtid is not cleared here
    // since it is going to be updated when the next gtid is fetched
    error = active_mi->rli->remove_logged_gtids(trimmed_gtids);
    unlock_master_info(active_mi);
  }
  global_sid_lock->unlock();

  return error;
}

int get_committed_gtids(const std::vector<std::string> &gtids,
                        std::vector<std::string> *committed_gtids) {
  global_sid_lock->rdlock();

  for (const auto &gtid_s : gtids) {
    if (gtid_s.empty()) continue;

    Gtid gtid;
    enum_return_status st = gtid.parse(global_sid_map, gtid_s.c_str());
    if (st != RETURN_STATUS_OK) {
      global_sid_lock->unlock();
      return st;
    }

    if (gtid_state->get_executed_gtids()->contains_gtid(gtid))
      committed_gtids->push_back(gtid_s);
  }
  global_sid_lock->unlock();

  return 0;
}

struct st_mysql_storage_engine binlog_storage_engine = {
    MYSQL_HANDLERTON_INTERFACE_VERSION};

/** @} */

mysql_declare_plugin(binlog){
    MYSQL_STORAGE_ENGINE_PLUGIN,
    &binlog_storage_engine,
    "binlog",
    PLUGIN_AUTHOR_ORACLE,
    "This is a pseudo storage engine to represent the binlog in a transaction",
    PLUGIN_LICENSE_GPL,
    binlog_init,   /* Plugin Init */
    nullptr,       /* Plugin Check uninstall */
    binlog_deinit, /* Plugin Deinit */
    0x0100 /* 1.0 */,
    nullptr, /* status variables                */
    nullptr, /* system variables                */
    nullptr, /* config options                  */
    0,
} mysql_declare_plugin_end;
