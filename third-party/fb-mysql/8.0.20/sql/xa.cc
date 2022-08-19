/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/xa.h"

#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "mysql/components/services/psi_mutex_bits.h"
#include "mysql/plugin.h"  // MYSQL_XIDDATASIZE
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_transaction.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"  // is_transaction_empty
#include "sql/clone_handler.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/handler.h"     // handlerton
#include "sql/item.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mdl_context_backup.h"  // MDL_context_backup_manager
#include "sql/mysqld.h"              // server_id
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"  // key_memory_XID
#include "sql/query_options.h"
#include "sql/rpl_context.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/sql_class.h"                       // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_list.h"
#include "sql/sql_plugin.h"  // plugin_foreach
#include "sql/sql_table.h"   // filename_to_tablename
#include "sql/system_variables.h"
#include "sql/tc_log.h"       // tc_log
#include "sql/transaction.h"  // trans_begin, trans_rollback
#include "sql/transaction_info.h"
#include "sql_string.h"
#include "template_utils.h"
#include "thr_mutex.h"

const char *XID_STATE::xa_state_names[] = {"NON-EXISTING", "ACTIVE", "IDLE",
                                           "PREPARED", "ROLLBACK ONLY"};

/* for recover() handlerton call */
static const int MIN_XID_LIST_SIZE = 128;
static const int MAX_XID_LIST_SIZE = 1024 * 128;

struct transaction_free_hash {
  void operator()(Transaction_ctx *) const;
};

static bool inited = false;
static mysql_mutex_t LOCK_transaction_cache;
static malloc_unordered_map<std::string, std::shared_ptr<Transaction_ctx>>
    transaction_cache{key_memory_XID};

static const uint MYSQL_XID_PREFIX_LEN = 8;  // must be a multiple of 8
static const uint MYSQL_XID_OFFSET = MYSQL_XID_PREFIX_LEN + sizeof(server_id);
static const uint MYSQL_XID_GTRID_LEN = MYSQL_XID_OFFSET + sizeof(my_xid);

static void attach_native_trx(THD *thd);
static std::shared_ptr<Transaction_ctx> transaction_cache_search(XID *xid);
static bool transaction_cache_insert(XID *xid, Transaction_ctx *transaction);
static bool transaction_cache_insert_recovery(XID *xid);

my_xid xid_t::get_my_xid() const {
  static_assert(XIDDATASIZE == MYSQL_XIDDATASIZE,
                "Our #define needs to match the one in plugin.h.");

  if (gtrid_length == static_cast<long>(MYSQL_XID_GTRID_LEN) &&
      bqual_length == 0 &&
      !memcmp(data, MYSQL_XID_PREFIX, MYSQL_XID_PREFIX_LEN)) {
    my_xid tmp;
    memcpy(&tmp, data + MYSQL_XID_OFFSET, sizeof(tmp));
    return tmp;
  }
  return 0;
}

void xid_t::set(my_xid xid) {
  formatID = 1;
  memcpy(data, MYSQL_XID_PREFIX, MYSQL_XID_PREFIX_LEN);
  memcpy(data + MYSQL_XID_PREFIX_LEN, &server_id, sizeof(server_id));
  memcpy(data + MYSQL_XID_OFFSET, &xid, sizeof(xid));
  gtrid_length = MYSQL_XID_GTRID_LEN;
  bqual_length = 0;
}

static bool xacommit_handlerton(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (hton->state == SHOW_OPTION_YES && hton->recover) {
    xa_status_code ret = hton->commit_by_xid(hton, (XID *)arg);

    /*
      Consider XAER_NOTA as success since not every storage should be
      involved into XA transaction, therefore absence of transaction
      specified by xid in storage engine doesn't mean that a real error
      happened. To illustrate it, lets consider the corner case
      when no one storage engine is involved into XA transaction:
      XA START 'xid1';
      XA END 'xid1';
      XA PREPARE 'xid1';
      XA COMMIT 'xid1';
      For this use case, handing of the statement XA COMMIT leads to
      returning XAER_NOTA by ha_innodb::commit_by_xid because there isn't
      a real transaction managed by innodb. So, there is no XA transaction
      with specified xid in resource manager represented by InnoDB storage
      engine although such transaction exists in transaction manager
      represented by mysql server runtime.
    */
    if (ret != XA_OK && ret != XAER_NOTA) {
      my_error(ER_XAER_RMERR, MYF(0));
      return true;
    }
    return false;
  }

  return false;
}

static bool xarollback_handlerton(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (hton->state == SHOW_OPTION_YES && hton->recover) {
    xa_status_code ret = hton->rollback_by_xid(hton, (XID *)arg);

    /*
      Consider XAER_NOTA as success since not every storage should be
      involved into XA transaction, therefore absence of transaction
      specified by xid in storage engine doesn't mean that a real error
      happened. To illustrate it, lets consider the corner case
      when no one storage engine is involved into XA transaction:
      XA START 'xid1';
      XA END 'xid1';
      XA PREPARE 'xid1';
      XA COMMIT 'xid1';
      For this use case, handing of the statement XA COMMIT leads to
      returning XAER_NOTA by ha_innodb::commit_by_xid because there isn't
      a real transaction managed by innodb. So, there is no XA transaction
      with specified xid in resource manager represented by InnoDB storage
      engine although such transaction exists in transaction manager
      represented by mysql server runtime.
    */
    if (ret != XA_OK && ret != XAER_NOTA) {
      my_error(ER_XAER_RMERR, MYF(0));
      return true;
    }
    return false;
  }
  return false;
}

static bool ha_commit_or_rollback_by_xid(THD *, XID *xid, bool commit) {
  return plugin_foreach(nullptr,
                        commit ? xacommit_handlerton : xarollback_handlerton,
                        MYSQL_STORAGE_ENGINE_PLUGIN, xid);
}

Recovered_xa_transactions *Recovered_xa_transactions::m_instance = nullptr;

Recovered_xa_transactions::Recovered_xa_transactions()
    : m_prepared_xa_trans(Malloc_allocator<XA_recover_txn *>(
          key_memory_Recovered_xa_transactions)),
      m_mem_root_inited(false) {}

Recovered_xa_transactions &Recovered_xa_transactions::instance() {
  return *m_instance;
}

bool Recovered_xa_transactions::init() {
  m_instance = new (std::nothrow) Recovered_xa_transactions();
  return m_instance == nullptr;
}

void Recovered_xa_transactions::destroy() {
  delete m_instance;
  m_instance = nullptr;
}

bool Recovered_xa_transactions::add_prepared_xa_transaction(
    XA_recover_txn *prepared_xa_trn_arg) {
  XA_recover_txn *prepared_xa_trn = new (&m_mem_root) XA_recover_txn();

  if (prepared_xa_trn == nullptr) {
    LogErr(ERROR_LEVEL, ER_SERVER_OUTOFMEMORY,
           static_cast<int>(sizeof(XA_recover_txn)));
    return true;
  }

  prepared_xa_trn->id = prepared_xa_trn_arg->id;
  prepared_xa_trn->mod_tables = prepared_xa_trn_arg->mod_tables;

  m_prepared_xa_trans.push_back(prepared_xa_trn);

  return false;
}

MEM_ROOT *Recovered_xa_transactions::get_allocated_memroot() {
  if (!m_mem_root_inited) {
    init_sql_alloc(key_memory_XID, &m_mem_root, TABLE_ALLOC_BLOCK_SIZE, 0);
    m_mem_root_inited = true;
  }
  return &m_mem_root;
}

struct xarecover_st {
  int len, found_foreign_xids, found_my_xids;
  XA_recover_txn *list;
  const xid_to_gtid_container *commit_list;
  bool dry_run;
  Gtid *binlog_max_gtid; /* max gtid across all engines */
  char *binlog_max_gtid_buf;
  Gtid *binlog_smallest_max_gtid; /* smallest max gtid for engine */
  char *binlog_file;
  my_off_t *binlog_pos;
};

static bool xarecover_create_mdl_backup(XA_recover_txn &txn,
                                        MEM_ROOT *mem_root) {
  MDL_request_list mdl_requests;
  List_iterator<st_handler_tablename> table_list_it(*txn.mod_tables);
  st_handler_tablename *tbl_name;

  while ((tbl_name = table_list_it++)) {
    MDL_request *table_mdl_request = new (mem_root) MDL_request;
    if (table_mdl_request == nullptr) {
      /* Out of memory: Abort() */
      return true;
    }

    char db_buff[NAME_CHAR_LEN * FILENAME_CHARSET_MBMAXLEN + 1];
    int len = filename_to_tablename(tbl_name->db, db_buff, sizeof(db_buff));
    db_buff[len] = '\0';

    char name_buff[NAME_CHAR_LEN * FILENAME_CHARSET_MBMAXLEN + 1];
    len = filename_to_tablename(tbl_name->tablename, name_buff,
                                sizeof(name_buff));
    name_buff[len] = '\0';

    /*
      We do not have information about the actual lock taken
      during the transaction. Hence we are going with a strong
      lock to be safe.
    */
    MDL_REQUEST_INIT(table_mdl_request, MDL_key::TABLE, db_buff, name_buff,
                     MDL_SHARED_WRITE, MDL_TRANSACTION);
    mdl_requests.push_front(table_mdl_request);
  }

  return MDL_context_backup_manager::instance().create_backup(
      &mdl_requests, txn.id.key(), txn.id.key_length());
}

bool Recovered_xa_transactions::recover_prepared_xa_transactions() {
  bool ret = false;

  if (m_mem_root_inited) {
    while (!m_prepared_xa_trans.empty()) {
      auto prepared_xa_trn = m_prepared_xa_trans.front();
      transaction_cache_insert_recovery(&prepared_xa_trn->id);

      if (xarecover_create_mdl_backup(*prepared_xa_trn, &m_mem_root)) {
        ret = true;
        break;
      }

      m_prepared_xa_trans.pop_front();
    }
    free_root(&m_mem_root, MYF(0));
    m_mem_root_inited = false;
  }

  return ret;
}

/* Read the current binlog position from SE and synchronize
 * it with the latest.
 */
static bool update_binlog_pos(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  xarecover_st *info = (struct xarecover_st *)arg;

  DBUG_ASSERT(info->binlog_file && info->binlog_max_gtid_buf);

  if (hton->state != SHOW_OPTION_YES || !hton->update_binlog_pos ||
      !hton->recover_binlog_pos) {
    return false;
  }

  /*
    Update SE's binlog position.
  */
  bool status = hton->update_binlog_pos(
      hton, info->binlog_file, info->binlog_pos, info->binlog_max_gtid_buf);

  if (status) {
    /* SE is expected to do final check on updating binlog positions - it needs
       to verify the binlog position hasn't already been updated by someone else
       and it skips the updates in this case. Thus, we can simply log the
       failure here */
    DBUG_PRINT("info",
               ("Plugin '%s': skipped binlog position (%s,%llu), max gtid %s",
                plugin_name(plugin)->str, info->binlog_file, *info->binlog_pos,
                info->binlog_max_gtid_buf));
    return false;
  }

  DBUG_PRINT("info",
             ("Plugin '%s': updated binlog position (%s,%llu), max gtid %s",
              plugin_name(plugin)->str, info->binlog_file, *info->binlog_pos,
              info->binlog_max_gtid_buf));

  return false;
}

/* Keeps all SE's binlog positions in sync.
 * This function is to be serialized during MYSQL_BIN_LOG
 * group commit phase where LOCK_commit is taken.
 */
int ha_update_binlog_pos(const char *binlog_file, my_off_t binlog_pos,
                         Gtid *max_gtid) {
  xarecover_st info;
  char max_gtid_buf[Gtid::MAX_TEXT_LENGTH + 1] = {0};
  DBUG_ENTER("ha_update_binlog_pos");

  if (binlog_file == nullptr || binlog_file[0] == '\0') {
    DBUG_RETURN(1);
  }

  info.binlog_file =
      const_cast<char *>(binlog_file) + dirname_length(binlog_file);
  info.binlog_pos = &binlog_pos;
  info.binlog_max_gtid = max_gtid;
  info.binlog_max_gtid_buf = max_gtid_buf;

  /*
    Populate the gtid buffer in string format that will
    be passed along to underlying SE's
   */
  if (max_gtid && !max_gtid->is_empty()) {
    global_sid_lock->rdlock();
    max_gtid->to_string(global_sid_map, info.binlog_max_gtid_buf);
    global_sid_lock->unlock();
  }

  /*
     Update binlog position of all SE that supports 2pc
   */
  if (plugin_foreach(nullptr, update_binlog_pos, MYSQL_STORAGE_ENGINE_PLUGIN,
                     &info)) {
    DBUG_RETURN(1);
  }

  DBUG_RETURN(0);
}

/*
 * Recover engine binlog position for plugins that have 2pc enabled.
 * In order for multiple 2pc engines to recover properly, the binlog
 * position should be the minimum of what each plugin has.
 * Warning: if an engine has an empty binlog position, i.e., slave
 * crashed earlier than the first binlog position update, that slave
 * would lose those transactions and be inconsistent with master.
 * Rather than replaying all binlogs from the very beginning, which
 * may be infeasible, this slave needs to be reconstructed operationally.
 * Periodically syncing binlog positions from these engines should reduce
 * the chance of this problem.
 */
static void recover_binlog_pos(const char *plugin_name, handlerton *hton,
                               xarecover_st *info) {
  char binlog_file[FN_REFLEN + 1] = {0};
  my_off_t binlog_pos = ULLONG_MAX;
  Gtid max_gtid{0, 0};

  DBUG_ASSERT(info->binlog_file && info->binlog_max_gtid &&
              info->binlog_smallest_max_gtid);

  hton->recover_binlog_pos(hton, &max_gtid, binlog_file, &binlog_pos);

  if (binlog_file[0] == 0) {
    DBUG_ASSERT(max_gtid.is_empty());

    return;
  }

  char gtid_buf[Gtid::MAX_TEXT_LENGTH + 1] = {0};
  if (!max_gtid.is_empty()) {
    global_sid_lock->rdlock();
    max_gtid.to_string(global_sid_map, gtid_buf);
    global_sid_lock->unlock();
  }

  sql_print_information("Plugin '%s': binlog position (%s,%llu), max gtid %s",
                        plugin_name, binlog_file, binlog_pos, gtid_buf);

  if (info->binlog_file[0] == 0) {
    DBUG_ASSERT(info->binlog_max_gtid->is_empty());

    *(info->binlog_max_gtid) = max_gtid;
    *(info->binlog_smallest_max_gtid) = max_gtid;
    memcpy(info->binlog_file, binlog_file, FN_REFLEN + 1);
    *info->binlog_pos = binlog_pos;

    return;
  }

  // Compute the max gtid based on the max of that of each engine
  // to make sure that idempotent recovery works
  // Idempotent should be disabled if sidno (uuid) doesn't match
  // in case of slave recovery after master switches. Also,
  // different engines could have different sidno. This is
  // not handled and slave would error out (duplicated key, key
  // not found, etc) - however, it should be rare if we proactively
  // keep all engines in sync.
  if (max_gtid.greater_than(*info->binlog_max_gtid)) {
    *(info->binlog_max_gtid) = max_gtid;

    // TODO (luqun): When raft is enabled, the plugin writes directly into
    // the base io-cache and these positions may not be updated correctly.
    // Explore the feasibility of plugin using the stream directly OR updating
    // the positions correctly in the stream
    // binlog positions should monotonically increase with max gtid
    DBUG_ASSERT(enable_raft_plugin ||
                is_binlog_advanced(info->binlog_file, *info->binlog_pos,
                                   binlog_file, binlog_pos));
  }

  // Compute mysql binlog position based on the min of that of
  // each engine
  if (is_binlog_advanced(binlog_file, binlog_pos, info->binlog_file,
                         *info->binlog_pos)) {
    memcpy(info->binlog_file, binlog_file, FN_REFLEN + 1);
    *info->binlog_pos = binlog_pos;

    // Track the smallest max gtid found so far.
    //
    // The engine's actual committed max_gtid might be higher than this because
    // prepared transactions may be rolled forward. However, idempotent
    // recovery allows the transactions that are being rolled forward to be
    // replayed. In the case where the binlog is trimmed on a primary instance,
    // nothing is rolled forward, so this value will be the smallest max_gtid
    // committed to an engine, and can be used as the executed_gtid base value.
    *(info->binlog_smallest_max_gtid) = max_gtid;
  }
}

static bool xarecover_handlerton(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  xarecover_st *info = (struct xarecover_st *)arg;
  int got;

  /**
  This variable will track the max gtid for executed transactions
  hton->recover will give us the max gtid stored in the engine prior to
  the restart but we need to update the max gtid if there are any
  prepared transactions that are rolled forward
  */
  Gtid recovered_max_gtid;
  recovered_max_gtid.clear();

  if (info->binlog_max_gtid) recovered_max_gtid = *(info->binlog_max_gtid);

  if (hton->state == SHOW_OPTION_YES && hton->recover) {
    if (info->binlog_file && hton->recover_binlog_pos) {
      recover_binlog_pos(plugin_name(plugin)->str, hton, info);

      char gtid_buf[Gtid::MAX_TEXT_LENGTH + 1] = {0};
      if (info->binlog_max_gtid && !info->binlog_max_gtid->is_empty()) {
        global_sid_lock->rdlock();
        info->binlog_max_gtid->to_string(global_sid_map, gtid_buf);
        global_sid_lock->unlock();
      }

      if (info->binlog_file[0]) {
        sql_print_information(
            "Plugin '%s': Current chosen binlog position (%s,%llu), max gtid "
            "%s",
            plugin_name(plugin)->str, info->binlog_file, *info->binlog_pos,
            gtid_buf);
      }
    }

    while (
        (got = hton->recover(
             hton, info->list, info->len,
             Recovered_xa_transactions::instance().get_allocated_memroot())) >
        0) {
      LogErr(INFORMATION_LEVEL, ER_XA_RECOVER_FOUND_TRX_IN_SE, got,
             ha_resolve_storage_engine_name(hton));
      for (int i = 0; i < got; i++) {
        char buf[XIDDATASIZE * 4 + 6];  // see xid_to_str
        my_xid x = info->list[i].id.get_my_xid();
        if (!x)  // not "mine" - that is generated by external TM
        {
          XID *xid = &info->list[i].id;
          LogErr(INFORMATION_LEVEL, ER_XA_IGNORING_XID, xid->xid_to_str(buf));

          if (Recovered_xa_transactions::instance().add_prepared_xa_transaction(
                  &info->list[i])) {
            return true;
          }
          info->found_foreign_xids++;
          continue;
        }
        if (info->dry_run) {
          info->found_my_xids++;
          continue;
        }

        const Gtid *current_gtid = nullptr;
        bool recovery_mode_condition;
        if (info->commit_list != nullptr) {
          auto it = info->commit_list->find(x);
          if (it != info->commit_list->end()) {
            recovery_mode_condition = true;
            current_gtid = &(it->second);
          } else
            recovery_mode_condition = false;
        } else {
          recovery_mode_condition =
              tc_heuristic_recover == TC_HEURISTIC_RECOVER_COMMIT;
        }
        // recovery mode
        // We roll-forward the txn only if the current prepared txn is present
        // in the binlog's commit list and we have not been explicitly asked
        // to trim binlog during recovery. If we are asked to trim binlogs
        // during recovery (i.e if opt_trim_binlog is set), then we have to
        // rollback all prepared txns in the engine.
        // If raft is enabled, then rollback all prepared txns
        if (recovery_mode_condition && !opt_trim_binlog &&
            !enable_raft_plugin) {
          // case: check if this prepared transaction's gtid is greater than
          // what we recovered before
          if (current_gtid != nullptr &&
              (current_gtid->greater_than(recovered_max_gtid) ||
               recovered_max_gtid.sidno != current_gtid->sidno)) {
            recovered_max_gtid = *current_gtid;
          }

          XID *xid = &info->list[i].id;
          LogErr(INFORMATION_LEVEL, ER_XA_COMMITTING_XID, xid->xid_to_str(buf));
          hton->commit_by_xid(hton, &info->list[i].id);
        } else {
          XID *xid = &info->list[i].id;
          LogErr(INFORMATION_LEVEL, ER_XA_ROLLING_BACK_XID,
                 xid->xid_to_str(buf));
          hton->rollback_by_xid(hton, &info->list[i].id);
        }
      }
      if (got < info->len) break;
    }
    // case: this is required because there might not be even a single
    // iteration of the while loop
    if (info->binlog_max_gtid &&
        (info->binlog_max_gtid->greater_than(recovered_max_gtid) ||
         info->binlog_max_gtid->sidno != recovered_max_gtid.sidno)) {
      recovered_max_gtid = *(info->binlog_max_gtid);
    }
  }

  if (info->binlog_max_gtid) {
    // set the max gtid in info so that it can be carried up the stack
    *(info->binlog_max_gtid) = recovered_max_gtid;
    if (!recovered_max_gtid.is_empty()) {
      char buf[Gtid::MAX_TEXT_LENGTH + 1];
      global_sid_lock->rdlock();
      recovered_max_gtid.to_string(global_sid_map, buf);
      global_sid_lock->unlock();
      // NO_LINT_DEBUG
      sql_print_information("Recovered Max Gtid is %s", buf);
    }
  }
  return false;
}

int ha_recover(const xid_to_gtid_container *commit_list, Gtid *binlog_max_gtid,
               char *binlog_file, my_off_t *binlog_pos) {
  xarecover_st info;
  DBUG_TRACE;
  info.found_foreign_xids = info.found_my_xids = 0;
  info.commit_list = commit_list;
  info.dry_run = (info.commit_list == nullptr &&
                  tc_heuristic_recover == TC_HEURISTIC_NOT_USED);
  info.list = nullptr;
  info.binlog_max_gtid = binlog_max_gtid;
  info.binlog_file = binlog_file;
  info.binlog_pos = binlog_pos;

  Gtid binlog_smallest_max_gtid;
  binlog_smallest_max_gtid.clear();
  info.binlog_smallest_max_gtid = &binlog_smallest_max_gtid;

  /* commit_list and tc_heuristic_recover cannot be set both */
  DBUG_ASSERT(info.commit_list == nullptr ||
              tc_heuristic_recover == TC_HEURISTIC_NOT_USED);
  /* if either is set, total_ha_2pc must be set too */
  DBUG_ASSERT(info.dry_run || total_ha_2pc > (ulong)opt_bin_log);

  if (total_ha_2pc <= (ulong)opt_bin_log) return 0;

  if (info.commit_list) LogErr(SYSTEM_LEVEL, ER_XA_STARTING_RECOVERY);

  /*
    FB - we can always rollback prepared transactions safely because of
    idempotent recovery and RBR. In the case where the binlog is not involved,
    the two storage engines could be inconsistent, but it is up to the
    application to resolve these problems.

    There is never a case where we want to roll-forward prepared transactions
    based on tc_heuristic_recovery, so there is no need to allow that behavior.
  */
  if (false && (total_ha_2pc > (ulong)opt_bin_log + 1)) {
    if (tc_heuristic_recover == TC_HEURISTIC_RECOVER_ROLLBACK) {
      LogErr(ERROR_LEVEL, ER_XA_NO_MULTI_2PC_HEURISTIC_RECOVER);
      return 1;
    }
  } else {
    /*
      If there is only one 2pc capable storage engine it is always safe
      to rollback. This setting will be ignored if we are in automatic
      recovery mode.
    */
    tc_heuristic_recover = TC_HEURISTIC_RECOVER_ROLLBACK;  // forcing ROLLBACK
    info.dry_run = false;
  }

  for (info.len = MAX_XID_LIST_SIZE;
       info.list == nullptr && info.len > MIN_XID_LIST_SIZE; info.len /= 2) {
    info.list = new (std::nothrow) XA_recover_txn[info.len];
  }
  if (!info.list) {
    LogErr(ERROR_LEVEL, ER_SERVER_OUTOFMEMORY,
           static_cast<int>(info.len * sizeof(XID)));
    return 1;
  }

  if (plugin_foreach(nullptr, xarecover_handlerton, MYSQL_STORAGE_ENGINE_PLUGIN,
                     &info)) {
    return 1;
  }

  /* Print the max gtid and smallest max gtid found for recovery */
  if (info.binlog_max_gtid && !info.binlog_max_gtid->is_empty()) {
    char max_gtid_buf[Gtid::MAX_TEXT_LENGTH + 1] = {0};
    char smallest_max_gtid_buf[Gtid::MAX_TEXT_LENGTH + 1] = {0};

    global_sid_lock->rdlock();
    info.binlog_max_gtid->to_string(global_sid_map, max_gtid_buf);
    info.binlog_smallest_max_gtid->to_string(global_sid_map,
                                             smallest_max_gtid_buf);
    global_sid_lock->unlock();

    /*
      The output of this line is parsed by other applications, so should
      not be changed.
    */
    fprintf(stderr,
            "Gtid recovery info: Largest MySQL Gtid %s, "
            "Smallest MySQL Gtid %s\n",
            max_gtid_buf, smallest_max_gtid_buf);
  }

  info.binlog_smallest_max_gtid = nullptr;

  delete[] info.list;

  if (info.found_foreign_xids)
    LogErr(WARNING_LEVEL, ER_XA_RECOVER_FOUND_XA_TRX, info.found_foreign_xids);
  if (info.dry_run && info.found_my_xids) {
    LogErr(ERROR_LEVEL, ER_XA_RECOVER_EXPLANATION, info.found_my_xids,
           opt_tc_log_file);
    return 1;
  }
  if (info.commit_list) LogErr(SYSTEM_LEVEL, ER_XA_RECOVERY_DONE);
  return 0;
}

bool xa_trans_force_rollback(THD *thd) {
  /*
    We must reset rm_error before calling ha_rollback(),
    so thd->transaction.xid structure gets reset
    by ha_rollback()/THD::transaction::cleanup().
  */
  thd->get_transaction()->xid_state()->reset_error();
  if (ha_rollback_trans(thd, true)) {
    my_error(ER_XAER_RMERR, MYF(0));
    return true;
  }
  return false;
}

void cleanup_trans_state(THD *thd) {
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->server_status &=
      ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);
  DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
  transaction_cache_delete(thd->get_transaction());
}

/**
  Find XA transaction in cache by its xid value.

  @param thd                     Thread context
  @param xid_for_trn_in_recover  xid value to look for in transaction cache
  @param xid_state               State of XA transaction in current session

  @return Pointer to an instance of Transaction_ctx corresponding to a
          xid in argument. If XA transaction not found returns nullptr and
          sets an error in DA to specify a reason of search failure.
*/

static std::shared_ptr<Transaction_ctx>
find_trn_for_recover_and_check_its_state(THD *thd,
                                         xid_t *xid_for_trn_in_recover,
                                         XID_STATE *xid_state) {
  if (!xid_state->has_state(XID_STATE::XA_NOTR)) {
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
    return nullptr;
  }

  /*
    Note, that there is no race condition here between
    transaction_cache_search and transaction_cache_delete,
    since we always delete our own XID
    (m_xid == thd->transaction().xid_state().m_xid).
    The only case when m_xid != thd->transaction.xid_state.m_xid
    and xid_state->in_thd == 0 is in the function
    transaction_cache_insert_recovery(XID), which is called before starting
    client connections, and thus is always single-threaded.
  */
  std::shared_ptr<Transaction_ctx> transaction =
      transaction_cache_search(xid_for_trn_in_recover);

  XID_STATE *xs = (transaction ? transaction->xid_state() : nullptr);
  if (!xs || !xs->is_in_recovery()) {
    my_error(ER_XAER_NOTA, MYF(0));
    return nullptr;
  } else if (thd->in_active_multi_stmt_transaction()) {
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
    return nullptr;
  }

  DBUG_ASSERT(xs->is_in_recovery());

  return transaction;
}

/**
  Commit and terminate a XA transaction.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure
*/

bool Sql_cmd_xa_commit::trans_xa_commit(THD *thd) {
  if (!enable_xa_transaction) {
    my_error(ER_XAER_INVAL, MYF(0));
    return true;
  }

  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  bool res = true;

  DBUG_ASSERT(!thd->slave_thread || xid_state->get_xid()->is_null() ||
              m_xa_opt == XA_ONE_PHASE);

  /* Inform clone handler of XA operation. */
  Clone_handler::XA_Operation xa_guard(thd);
  if (!xid_state->has_same_xid(m_xid)) {
    res = process_external_xa_commit(thd, m_xid, xid_state);
  } else {
    res = process_internal_xa_commit(thd, xid_state);
  }
  return (res);
}

/**
  Acquire Commit metadata lock and all locks acquired by a prepared XA
  transaction before server was shutdown or terminated.

  @param thd           Thread context
  @param external_xid  XID value specified by XA COMMIT or XA ROLLBACK that
                       corresponds to a XA transaction generated outside
                       current session context.

  @retval false        Success
  @retval true         Failure
*/
static bool acquire_mandatory_metadata_locks(THD *thd, xid_t *external_xid) {
  /*
    Acquire metadata lock which will ensure that XA ROLLBACK is blocked
    by active FLUSH TABLES WITH READ LOCK (and vice versa ROLLBACK in
    progress blocks FTWRL). This is to avoid binlog and redo entries
    while a backup is in progress.
  */
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::COMMIT, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
  if (thd->mdl_context.acquire_lock_nsec(
          &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
    return true;
  }

  /*
    Like in the commit case a failure to store gtid is regarded
    as the resource manager issue.
  */

  if (MDL_context_backup_manager::instance().restore_backup(
          &thd->mdl_context, external_xid->key(), external_xid->key_length())) {
    return true;
  }

  return false;
}

/**
  Handle the statement XA COMMIT for the case when xid corresponds to
  an external XA transaction, that it a transaction generated outside
  current session context.

  @param thd           Thread context
  @param external_xid  XID value specified by XA COMMIT that corresponds to
                       a XA transaction generated outside current session
                       context. In fact, it means that XA COMMIT is run
                       against a XA transaction recovered after server restart.
  @param xid_state     State of XA transaction corresponding to the current
                       session that expected to have the value
                       XID_STATE::XA_NOTR

  @return  operation result
    @retval false  Success
    @retval true   Failure
*/
bool Sql_cmd_xa_commit::process_external_xa_commit(THD *thd,
                                                   xid_t *external_xid,
                                                   XID_STATE *xid_state) {
  std::shared_ptr<Transaction_ctx> transaction =
      find_trn_for_recover_and_check_its_state(thd, external_xid, xid_state);

  if (!transaction) return true;

  XID_STATE *xs = transaction->xid_state();

  DBUG_ASSERT(xs->get_xid()->eq(external_xid));

  /*
    Resumed transaction XA-commit.
    The case deals with the "external" XA-commit by either a slave applier
    or a different than XA-prepared transaction session.
  */
  bool res = xs->xa_trans_rolled_back();

  /*
    Metadata locks taken during XA COMMIT should be released when
    there is error in commit order execution, so we take a savepoint
    and rollback to it in case of error. The error during commit order
    execution can be temporary like commit order deadlock (check header
    comment for check_and_report_deadlock()) and can be recovered after
    retrying unlike other commit errors. And to do so we need to restore
    status of metadata locks i.e. rollback to savepoint, before the retry
    attempt to ensure order for applier threads.
  */
  MDL_savepoint mdl_savepoint = thd->mdl_context.mdl_savepoint();

  DEBUG_SYNC(thd, "external_xa_commit_before_acquire_xa_lock");
  /*
    Acquire XID_STATE::m_xa_lock to prevent concurrent running of two
    XA COMMIT/XA ROLLBACK statements. Without acquiring this lock an attempt
    to run two XA COMMIT/XA ROLLBACK statement for the same xid value may lead
    to writing two events for the same xid into the binlog (e.g. twice
    XA COMMIT event, that is an event for XA COMMIT some_xid_value
    followed by an another event XA COMMIT with the same xid value).
    As a consequences, presence of two XA COMMIT/XA ROLLACK statements for
    the same xid value in binlog would break replication.
  */
  std::lock_guard<std::mutex> lk(xs->get_xa_lock());
  /*
    Double check that the XA transaction still does exist since the transaction
    could be removed from the cache by another XA COMMIT/XA ROLLBACK statement
    being executed concurrently from parallel session with the same xid value.
  */
  if (!find_trn_for_recover_and_check_its_state(thd, external_xid, xid_state))
    return true;

  if (acquire_mandatory_metadata_locks(thd, external_xid)) {
    /*
      We can't rollback an XA transaction on lock failure due to
      Innodb redo log and bin log update is involved in rollback.
      Return error to user for a retry.
    */
    my_error(ER_XA_RETRY, MYF(0));
    return true;
  }

  DEBUG_SYNC(thd, "external_xa_commit_after_acquire_commit_lock");

  /* Do not execute gtid wrapper whenever 'res' is true (rm error) */
  bool gtid_error = false;
  bool need_clear_owned_gtid = false;
  std::tie(gtid_error, need_clear_owned_gtid) = commit_owned_gtids(thd, true);
  if (gtid_error) my_error(ER_XA_RBROLLBACK, MYF(0));
  res = res || gtid_error;

  /*
    xs' is_binlogged() is passed through xid_state's member to low-level
    logging routines for deciding how to log.  The same applies to
    Rollback case.
  */
  if (xs->is_binlogged())
    xid_state->set_binlogged();
  else
    xid_state->unset_binlogged();

  /*
    Ensure externalization order for applier threads.

    Note: the calls to Commit_order_manager::wait/wait_and_finish() will be
          no-op for threads other than replication applier threads.
  */
  if (Commit_order_manager::wait(thd)) {
    /*
      In case of error, wait_and_finish() checks if transaction can be
      retried and if it can be retried then only it remove itself from
      waiting (Commit Order Queue) and allow next applier thread to be
      ordered.
    */
    Commit_order_manager::wait_and_finish(thd, true);

    need_clear_owned_gtid = true;
    gtid_error = true;
    gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);
    thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
    return true;
  }

  res = ha_commit_or_rollback_by_xid(thd, external_xid, !res) || res;

  /*
    After ensuring externalization order for applier thread, remove it
    from waiting (Commit Order Queue) and allow next applier thread to
    be ordered.

    Note: the calls to Commit_order_manager::wait_and_finish() will be
          no-op for threads other than replication applier threads.
  */
  Commit_order_manager::wait_and_finish(thd, res);

  xid_state->unset_binlogged();

  MDL_context_backup_manager::instance().delete_backup(
      external_xid->key(), external_xid->key_length());

  transaction_cache_delete(transaction.get());
  gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);

  return res;
}

/**
  Handle the statement XA COMMIT for the case when xid corresponds to
  an internal XA transaction, that is a transaction generated by
  current session context.

  @param thd           Thread context
  @param xid_state     State of XA transaction corresponding to the current
                       session.

  @return  operation result
    @retval false  Success
    @retval true   Failure
*/
bool Sql_cmd_xa_commit::process_internal_xa_commit(THD *thd,
                                                   XID_STATE *xid_state) {
  DBUG_TRACE;
  bool res = false;
  bool gtid_error = false, need_clear_owned_gtid = false;

  if (xid_state->xa_trans_rolled_back()) {
    xa_trans_force_rollback(thd);
    res = thd->is_error();
  } else if (xid_state->has_state(XID_STATE::XA_IDLE) &&
             m_xa_opt == XA_ONE_PHASE) {
    int r = ha_commit_trans(thd, true);
    if ((res = r)) my_error(r == 1 ? ER_XA_RBROLLBACK : ER_XAER_RMERR, MYF(0));
  } else if (xid_state->has_state(XID_STATE::XA_PREPARED) &&
             m_xa_opt == XA_NONE) {
    MDL_request mdl_request;

    /*
      Acquire metadata lock which will ensure that COMMIT is blocked
      by active FLUSH TABLES WITH READ LOCK (and vice versa COMMIT in
      progress blocks FTWRL).

      We allow FLUSHer to COMMIT; we assume FLUSHer knows what it does.
    */
    MDL_REQUEST_INIT(&mdl_request, MDL_key::COMMIT, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
    if (thd->mdl_context.acquire_lock_nsec(
            &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
      /*
        We can't rollback an XA transaction on lock failure due to
        Innodb redo log and bin log update are involved in rollback.
        Return error to user for a retry.
      */
      my_error(ER_XA_RETRY, MYF(0));
      return true;
    }

    std::tie(gtid_error, need_clear_owned_gtid) = commit_owned_gtids(thd, true);
    if (gtid_error) {
      res = true;
      /*
        Failure to store gtid is regarded as a unilateral one of the
        resource manager therefore the transaction is to be rolled back.
        The specified error is the same as @c xa_trans_force_rollback.
        The prepared XA will be rolled back along and so will do Gtid state,
        see ha_rollback_trans().

        Todo/fixme: fix binlogging, "XA rollback" event could be missed out.
        Todo/fixme: as to XAER_RMERR, should not it be XA_RBROLLBACK?
                    Rationale: there's no consistency concern after rollback,
                    unlike what XAER_RMERR suggests.
      */
      ha_rollback_trans(thd, true);
      my_error(ER_XAER_RMERR, MYF(0));
    } else {
      DBUG_EXECUTE_IF("simulate_crash_on_commit_xa_trx", DBUG_SUICIDE(););
      DEBUG_SYNC(thd, "trans_xa_commit_after_acquire_commit_lock");

      if (tc_log)
        res = tc_log->commit(thd, /* all */ true);
      else
        res = ha_commit_low(thd, /* all */ true);

      DBUG_EXECUTE_IF("simulate_xa_commit_log_failure", { res = true; });

      if (res)
        my_error(ER_XAER_RMERR, MYF(0));  // todo/fixme: consider to rollback it
#ifdef HAVE_PSI_TRANSACTION_INTERFACE
      else {
        /*
          Since we don't call ha_commit_trans() for prepared transactions,
          we need to explicitly mark the transaction as committed.
        */
        MYSQL_COMMIT_TRANSACTION(thd->m_transaction_psi);
      }

      thd->m_transaction_psi = nullptr;
#endif
    }
  } else {
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
    return true;
  }

  gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);
  cleanup_trans_state(thd);

  xid_state->set_state(XID_STATE::XA_NOTR);
  xid_state->unset_binlogged();
  trans_track_end_trx(thd);
  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr || res);
  return res;
}

bool Sql_cmd_xa_commit::execute(THD *thd) {
  bool st = trans_xa_commit(thd);

  if (!st) {
    thd->mdl_context.release_transactional_locks();
    /*
        We've just done a commit, reset transaction
        isolation level and access mode to the session default.
    */
    trans_reset_one_shot_chistics(thd);

    my_ok(thd);
  }
  return st;
}

/**
  Roll back and terminate a XA transaction.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure
*/

bool Sql_cmd_xa_rollback::trans_xa_rollback(THD *thd) {
  if (!enable_xa_transaction) {
    my_error(ER_XAER_INVAL, MYF(0));
    return true;
  }

  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  bool res = true;

  /* Inform clone handler of XA operation. */
  Clone_handler::XA_Operation xa_guard(thd);
  if (!xid_state->has_same_xid(m_xid)) {
    res = process_external_xa_rollback(thd, m_xid, xid_state);
  } else {
    res = process_internal_xa_rollback(thd, xid_state);
  }
  return (res);
}

/**
  Handle the statement XA ROLLBACK for the case when xid corresponds to
  an external XA transaction, that it a transaction generated outside
  current session context.

  @param thd           Thread context
  @param external_xid  XID value specified by XA ROLLBACK that corresponds to
                       a XA transaction generated outside current session
                       context. In fact, it means that XA ROLLBACK is run
                       against a XA transaction recovered after server restart.
  @param xid_state     State of XA transaction corresponding to the current
                       session that expected to have the value
                       XID_STATE::XA_NOTR

  @return  operation result
    @retval false  Success
    @retval true   Failure
*/
bool Sql_cmd_xa_rollback::process_external_xa_rollback(THD *thd,
                                                       xid_t *external_xid,
                                                       XID_STATE *xid_state) {
  DBUG_TRACE;

  std::shared_ptr<Transaction_ctx> transaction =
      find_trn_for_recover_and_check_its_state(thd, external_xid, xid_state);

  if (!transaction) return true;

  XID_STATE *xs = transaction->xid_state();

  DBUG_ASSERT(xs->get_xid()->eq(external_xid));

  /*
    Metadata locks taken during XA ROLLBACK should be released when
    there is error in commit order execution, so we take a savepoint
    and rollback to it in case of error.
  */
  MDL_savepoint mdl_savepoint = thd->mdl_context.mdl_savepoint();

  /*
    Acquire XID_STATE::m_xa_lock to prevent concurrent running of two
    XA COMMIT/XA ROLLBACK statements. Without acquiring this lock an attempt
    to run two XA COMMIT/XA ROLLBACK statement for the same xid value may lead
    to writing two events for the same xid into the binlog (e.g. twice
    XA ROLLBACK event, that is an event for XA ROLLBACK some_xid_value
    followed by an another event XA ROLLBACK with the same xid value).
    As a consequences, presence of two XA COMMIT/XA ROLLACK statements for
    the same xid value in binlog would break replication.
  */
  std::lock_guard<std::mutex> lk(xs->get_xa_lock());
  /*
    Double check that the XA transaction still does exist since the transaction
    could be removed from the cache by another XA COMMIT/XA ROLLBACK statement
    being executed concurrently from parallel session with the same xid value.
  */
  if (!find_trn_for_recover_and_check_its_state(thd, external_xid, xid_state))
    return true;

  if (acquire_mandatory_metadata_locks(thd, external_xid)) {
    /*
      We can't rollback an XA transaction on lock failure due to
      Innodb redo log and bin log update is involved in rollback.
      Return error to user for a retry.
    */
    my_error(ER_XAER_RMERR, MYF(0));
    return true;
  }

  bool gtid_error = false;
  bool need_clear_owned_gtid = false;
  std::tie(gtid_error, need_clear_owned_gtid) = commit_owned_gtids(thd, true);
  if (gtid_error) my_error(ER_XA_RBROLLBACK, MYF(0));
  bool res = xs->xa_trans_rolled_back();

  if (xs->is_binlogged())
    xid_state->set_binlogged();
  else
    xid_state->unset_binlogged();

  /*
    Ensure externalization order for applier threads.

    Note: the calls to Commit_order_manager::wait/wait_and_finish() will be
          no-op for threads other than replication applier threads.
  */
  if (Commit_order_manager::wait(thd)) {
    /*
      In case of error, wait_and_finish() checks if transaction can be
      retried and if it can be retried then only it remove itself from
      waiting (Commit Order Queue) and allow next applier thread to be
      ordered.
    */
    Commit_order_manager::wait_and_finish(thd, true);

    need_clear_owned_gtid = true;
    gtid_error = true;
    gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);
    thd->mdl_context.rollback_to_savepoint(mdl_savepoint);
    return true;
  }

  res = ha_commit_or_rollback_by_xid(thd, external_xid, false) || res;

  /*
    After ensuring externalization order for applier thread, remove it
    from waiting (Commit Order Queue) and allow next applier thread to
    be ordered.

    Note: the calls to Commit_order_manager::wait_and_finish() will be
          no-op for threads other than replication applier threads.
  */
  Commit_order_manager::wait_and_finish(thd, res);
  xid_state->unset_binlogged();

  MDL_context_backup_manager::instance().delete_backup(
      external_xid->key(), external_xid->key_length());
  transaction_cache_delete(transaction.get());
  gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);
  return res || gtid_error;
}

/**
  Handle the statement XA ROLLBACK for the case when xid corresponds to
  an internal XA transaction, that is a transaction generated by
  current session context.

  @param thd           Thread context
  @param xid_state     State of XA transaction corresponding to the current
                       session.

  @return  operation result
    @retval false  Success
    @retval true   Failure
*/
bool Sql_cmd_xa_rollback::process_internal_xa_rollback(THD *thd,
                                                       XID_STATE *xid_state) {
  DBUG_TRACE;

  if (xid_state->has_state(XID_STATE::XA_NOTR) ||
      xid_state->has_state(XID_STATE::XA_ACTIVE)) {
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
    return true;
  }

  /*
    Acquire metadata lock which will ensure that XA ROLLBACK is blocked
    by active FLUSH TABLES WITH READ LOCK (and vice versa ROLLBACK in
    progress blocks FTWRL). This is to avoid binlog and redo entries
    while a backup is in progress.
  */
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::COMMIT, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
  if (thd->mdl_context.acquire_lock_nsec(
          &mdl_request, thd->variables.lock_wait_timeout_nsec)) {
    /*
      We can't rollback an XA transaction on lock failure due to
      Innodb redo log and bin log update is involved in rollback.
      Return error to user for a retry.
    */
    my_error(ER_XAER_RMERR, MYF(0));
    return true;
  }

  bool gtid_error = false;
  bool need_clear_owned_gtid = false;
  std::tie(gtid_error, need_clear_owned_gtid) = commit_owned_gtids(thd, true);
  bool res = xa_trans_force_rollback(thd) || gtid_error;
  gtid_state_commit_or_rollback(thd, need_clear_owned_gtid, !gtid_error);
  // todo: report a bug in that the raised rm_error in this branch
  //       is masked unlike the "external" rollback branch above.
  DBUG_EXECUTE_IF("simulate_xa_rm_error", {
    my_error(ER_XA_RBROLLBACK, MYF(0));
    res = true;
  });

  cleanup_trans_state(thd);

  xid_state->set_state(XID_STATE::XA_NOTR);
  xid_state->unset_binlogged();
  trans_track_end_trx(thd);
  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr);
  return res;
}

bool Sql_cmd_xa_rollback::execute(THD *thd) {
  bool st = trans_xa_rollback(thd);

  if (!st) {
    thd->mdl_context.release_transactional_locks();
    /*
      We've just done a rollback, reset transaction
      isolation level and access mode to the session default.
    */
    trans_reset_one_shot_chistics(thd);
    my_ok(thd);
  }

  DBUG_EXECUTE_IF("crash_after_xa_rollback", DBUG_SUICIDE(););

  return st;
}

/**
  Start a XA transaction with the given xid value.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure
*/

bool Sql_cmd_xa_start::trans_xa_start(THD *thd) {
  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  DBUG_TRACE;

  if (!enable_xa_transaction) {
    my_error(ER_XAER_INVAL, MYF(0));
    return true;
  }

  if (xid_state->has_state(XID_STATE::XA_IDLE) && m_xa_opt == XA_RESUME) {
    bool not_equal = !xid_state->has_same_xid(m_xid);
    if (not_equal)
      my_error(ER_XAER_NOTA, MYF(0));
    else {
      xid_state->set_state(XID_STATE::XA_ACTIVE);
      MYSQL_SET_TRANSACTION_XA_STATE(
          thd->m_transaction_psi,
          (int)thd->get_transaction()->xid_state()->get_state());
    }
    return not_equal;
  }

  /* TODO: JOIN is not supported yet. */
  if (m_xa_opt != XA_NONE)
    my_error(ER_XAER_INVAL, MYF(0));
  else if (!xid_state->has_state(XID_STATE::XA_NOTR))
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
  else if (thd->locked_tables_mode || thd->in_active_multi_stmt_transaction())
    my_error(ER_XAER_OUTSIDE, MYF(0));
  else if (!trans_begin(thd)) {
    xid_state->start_normal_xa(m_xid);
    MYSQL_SET_TRANSACTION_XID(thd->m_transaction_psi,
                              (const void *)xid_state->get_xid(),
                              (int)xid_state->get_state());
    if (transaction_cache_insert(m_xid, thd->get_transaction())) {
      xid_state->reset();
      trans_rollback(thd);
    }
  }

  return thd->is_error() || !xid_state->has_state(XID_STATE::XA_ACTIVE);
}

bool Sql_cmd_xa_start::execute(THD *thd) {
  bool st = trans_xa_start(thd);

  if (!st) {
    thd->rpl_detach_engine_ha_data();
    my_ok(thd);
  }

  return st;
}

/**
  Put a XA transaction in the IDLE state.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure
*/

bool Sql_cmd_xa_end::trans_xa_end(THD *thd) {
  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  DBUG_TRACE;

  if (!enable_xa_transaction) {
    my_error(ER_XAER_INVAL, MYF(0));
    return true;
  }

  /* TODO: SUSPEND and FOR MIGRATE are not supported yet. */
  if (m_xa_opt != XA_NONE)
    my_error(ER_XAER_INVAL, MYF(0));
  else if (!xid_state->has_state(XID_STATE::XA_ACTIVE))
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
  else if (!xid_state->has_same_xid(m_xid))
    my_error(ER_XAER_NOTA, MYF(0));
  else if (!xid_state->xa_trans_rolled_back()) {
    xid_state->set_state(XID_STATE::XA_IDLE);
    MYSQL_SET_TRANSACTION_XA_STATE(thd->m_transaction_psi,
                                   (int)xid_state->get_state());
  } else {
    MYSQL_SET_TRANSACTION_XA_STATE(thd->m_transaction_psi,
                                   (int)xid_state->get_state());
  }

  return thd->is_error() || !xid_state->has_state(XID_STATE::XA_IDLE);
}

bool Sql_cmd_xa_end::execute(THD *thd) {
  bool st = trans_xa_end(thd);

  if (!st) my_ok(thd);

  return st;
}

/**
  Put a XA transaction in the PREPARED state.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure
*/

bool Sql_cmd_xa_prepare::trans_xa_prepare(THD *thd) {
  XID_STATE *xid_state = thd->get_transaction()->xid_state();
  DBUG_TRACE;

  if (!enable_xa_transaction) {
    my_error(ER_XAER_INVAL, MYF(0));
    return true;
  }

  if (!xid_state->has_state(XID_STATE::XA_IDLE))
    my_error(ER_XAER_RMFAIL, MYF(0), xid_state->state_name());
  else if (!xid_state->has_same_xid(m_xid))
    my_error(ER_XAER_NOTA, MYF(0));
  else if (thd->slave_thread &&
           is_transaction_empty(
               thd))  // No changes in none of the storage engine
                      // means, filtered statements in the slave
    my_error(ER_XA_REPLICATION_FILTERS,
             MYF(0));  // Empty XA transactions not allowed
  else {
    /*
      Acquire metadata lock which will ensure that XA PREPARE is blocked
      by active FLUSH TABLES WITH READ LOCK (and vice versa PREPARE in
      progress blocks FTWRL). This is to avoid binlog and redo entries
      while a backup is in progress.
    */
    MDL_request mdl_request;
    MDL_REQUEST_INIT(&mdl_request, MDL_key::COMMIT, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
    if (thd->mdl_context.acquire_lock_nsec(
            &mdl_request, thd->variables.lock_wait_timeout_nsec) ||
        ha_xa_prepare(thd)) {
      /*
        Rollback the transaction if lock failed. For ha_xa_prepare() failure
        scenarios, transaction is already rolled back by ha_xa_prepare().
      */
      if (!mdl_request.ticket) ha_rollback_trans(thd, true);

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
      DBUG_ASSERT(thd->m_transaction_psi == nullptr);
#endif

      /*
        Reset rm_error in case ha_xa_prepare() returned error,
        so thd->transaction.xid structure gets reset
        by THD::transaction::cleanup().
      */
      thd->get_transaction()->xid_state()->reset_error();
      cleanup_trans_state(thd);
      xid_state->set_state(XID_STATE::XA_NOTR);
      thd->get_transaction()->cleanup();
      my_error(ER_XA_RBROLLBACK, MYF(0));
    } else {
      xid_state->set_state(XID_STATE::XA_PREPARED);
      MYSQL_SET_TRANSACTION_XA_STATE(thd->m_transaction_psi,
                                     (int)xid_state->get_state());
      if (thd->rpl_thd_ctx.session_gtids_ctx().notify_after_xa_prepare(thd))
        LogErr(WARNING_LEVEL, ER_TRX_GTID_COLLECT_REJECT);
    }
  }

  return thd->is_error() || !xid_state->has_state(XID_STATE::XA_PREPARED);
}

bool Sql_cmd_xa_prepare::execute(THD *thd) {
  bool st = trans_xa_prepare(thd);

  if (!st) {
    if (!thd->rpl_unflag_detached_engine_ha_data() ||
        !(st = applier_reset_xa_trans(thd)))
      my_ok(thd);
  }

  return st;
}

/**
  Return the list of XID's to a client, the same way SHOW commands do.

  @param thd    Current thread

  @retval false  Success
  @retval true   Failure

  @note
    I didn't find in XA specs that an RM cannot return the same XID twice,
    so trans_xa_recover does not filter XID's to ensure uniqueness.
    It can be easily fixed later, if necessary.
*/

bool Sql_cmd_xa_recover::trans_xa_recover(THD *thd) {
  List<Item> field_list;
  Protocol *protocol = thd->get_protocol();

  DBUG_TRACE;

  field_list.push_back(
      new Item_int(NAME_STRING("formatID"), 0, MY_INT32_NUM_DECIMAL_DIGITS));
  field_list.push_back(new Item_int(NAME_STRING("gtrid_length"), 0,
                                    MY_INT32_NUM_DECIMAL_DIGITS));
  field_list.push_back(new Item_int(NAME_STRING("bqual_length"), 0,
                                    MY_INT32_NUM_DECIMAL_DIGITS));
  field_list.push_back(new Item_empty_string("data", XIDDATASIZE * 2 + 2));

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  mysql_mutex_lock(&LOCK_transaction_cache);

  for (const auto &key_and_value : transaction_cache) {
    Transaction_ctx *transaction = key_and_value.second.get();
    XID_STATE *xs = transaction->xid_state();
    if (xs->has_state(XID_STATE::XA_PREPARED)) {
      protocol->start_row();
      xs->store_xid_info(protocol, m_print_xid_as_hex);

      if (protocol->end_row()) {
        mysql_mutex_unlock(&LOCK_transaction_cache);
        return true;
      }
    }
  }

  mysql_mutex_unlock(&LOCK_transaction_cache);
  my_eof(thd);
  return false;
}

/**
  Check if the current user has a privilege to perform XA RECOVER.

  @param thd    Current thread

  @retval false  A user has a privilege to perform XA RECOVER
  @retval true   A user doesn't have a privilege to perform XA RECOVER
*/

bool Sql_cmd_xa_recover::check_xa_recover_privilege(THD *thd) const {
  Security_context *sctx = thd->security_context();

  if (!sctx->has_global_grant(STRING_WITH_LEN("XA_RECOVER_ADMIN")).first) {
    /*
      Report an error ER_XAER_RMERR. A supplementary error
      ER_SPECIFIC_ACCESS_DENIED_ERROR is also reported when
      SHOW WARNINGS is issued. This provides more information
      about the reason for failure.
    */
    my_error(ER_XAER_RMERR, MYF(0));
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "XA_RECOVER_ADMIN");
    return true;
  }

  return false;
}

bool Sql_cmd_xa_recover::execute(THD *thd) {
  bool st = check_xa_recover_privilege(thd) || trans_xa_recover(thd);

  DBUG_EXECUTE_IF("crash_after_xa_recover", { DBUG_SUICIDE(); });

  return st;
}

bool XID_STATE::xa_trans_rolled_back() {
  DBUG_EXECUTE_IF("simulate_xa_rm_error", rm_error = true;);
  if (rm_error) {
    switch (rm_error) {
      case ER_LOCK_WAIT_TIMEOUT:
        my_error(ER_XA_RBTIMEOUT, MYF(0));
        break;
      case ER_LOCK_DEADLOCK:
        my_error(ER_XA_RBDEADLOCK, MYF(0));
        break;
      default:
        my_error(ER_XA_RBROLLBACK, MYF(0));
    }
    xa_state = XID_STATE::XA_ROLLBACK_ONLY;
  }

  return (xa_state == XID_STATE::XA_ROLLBACK_ONLY);
}

bool XID_STATE::check_xa_idle_or_prepared(bool report_error) const {
  if (xa_state == XA_IDLE || xa_state == XA_PREPARED) {
    if (report_error)
      my_error(ER_XAER_RMFAIL, MYF(0), xa_state_names[xa_state]);

    return true;
  }

  return false;
}

bool XID_STATE::check_has_uncommitted_xa() const {
  if (xa_state == XA_IDLE || xa_state == XA_PREPARED ||
      xa_state == XA_ROLLBACK_ONLY) {
    my_error(ER_XAER_RMFAIL, MYF(0), xa_state_names[xa_state]);
    return true;
  }

  return false;
}

bool XID_STATE::check_in_xa(bool report_error) const {
  if (xa_state != XA_NOTR) {
    if (report_error)
      my_error(ER_XAER_RMFAIL, MYF(0), xa_state_names[xa_state]);
    return true;
  }

  return false;
}

void XID_STATE::set_error(THD *thd) {
  if (xa_state != XA_NOTR) rm_error = thd->get_stmt_da()->mysql_errno();
}

void XID_STATE::store_xid_info(Protocol *protocol,
                               bool print_xid_as_hex) const {
  protocol->store_longlong(static_cast<longlong>(m_xid.formatID), false);
  protocol->store_longlong(static_cast<longlong>(m_xid.gtrid_length), false);
  protocol->store_longlong(static_cast<longlong>(m_xid.bqual_length), false);

  if (print_xid_as_hex) {
    /*
      xid_buf contains enough space for 0x followed by HEX representation
      of the binary XID data and one null termination character.
    */
    char xid_buf[XIDDATASIZE * 2 + 2 + 1];

    xid_buf[0] = '0';
    xid_buf[1] = 'x';

    size_t xid_str_len =
        bin_to_hex_str(xid_buf + 2, sizeof(xid_buf) - 2, m_xid.data,
                       m_xid.gtrid_length + m_xid.bqual_length) +
        2;
    protocol->store_string(xid_buf, xid_str_len, &my_charset_bin);
  } else {
    protocol->store_string(m_xid.data, m_xid.gtrid_length + m_xid.bqual_length,
                           &my_charset_bin);
  }
}

char *XID::xid_to_str(char *buf) const {
  char *s = buf;
  *s++ = '\'';

  for (int i = 0; i < gtrid_length + bqual_length; i++) {
    /* is_next_dig is set if next character is a number */
    bool is_next_dig = false;
    if (i < XIDDATASIZE) {
      char ch = data[i + 1];
      is_next_dig = (ch >= '0' && ch <= '9');
    }
    if (i == gtrid_length) {
      *s++ = '\'';
      if (bqual_length) {
        *s++ = '.';
        *s++ = '\'';
      }
    }
    uchar c = static_cast<uchar>(data[i]);
    if (c < 32 || c > 126) {
      *s++ = '\\';
      /*
        If next character is a number, write current character with
        3 octal numbers to ensure that the next number is not seen
        as part of the octal number
      */
      if (c > 077 || is_next_dig) *s++ = _dig_vec_lower[c >> 6];
      if (c > 007 || is_next_dig) *s++ = _dig_vec_lower[(c >> 3) & 7];
      *s++ = _dig_vec_lower[c & 7];
    } else {
      if (c == '\'' || c == '\\') *s++ = '\\';
      *s++ = c;
    }
  }
  *s++ = '\'';
  *s = 0;
  return buf;
}

static inline std::string to_string(const XID &xid) {
  return std::string(pointer_cast<const char *>(xid.key()), xid.key_length());
}

/**
  Callback that is called to do cleanup.

  @param transaction  pointer to free
*/

void transaction_free_hash::operator()(Transaction_ctx *transaction) const {
  // Only time it's allocated is during recovery process.
  if (transaction->xid_state()->is_in_recovery()) delete transaction;
}

#ifdef HAVE_PSI_INTERFACE
static PSI_mutex_key key_LOCK_transaction_cache;

static PSI_mutex_info transaction_cache_mutexes[] = {
    {&key_LOCK_transaction_cache, "LOCK_transaction_cache", PSI_FLAG_SINGLETON,
     0, PSI_DOCUMENT_ME}};

static void init_transaction_cache_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(transaction_cache_mutexes));
  mysql_mutex_register(category, transaction_cache_mutexes, count);
}
#endif /* HAVE_PSI_INTERFACE */

bool transaction_cache_init() {
#ifdef HAVE_PSI_INTERFACE
  init_transaction_cache_psi_keys();
#endif

  mysql_mutex_init(key_LOCK_transaction_cache, &LOCK_transaction_cache,
                   MY_MUTEX_INIT_FAST);
  inited = true;
  return false;
}

void transaction_cache_free() {
  if (inited) {
    transaction_cache.clear();
    mysql_mutex_destroy(&LOCK_transaction_cache);
  }
}

/**
  Search information about XA transaction by a XID value.

  @param xid    Pointer to a XID structure that identifies a XA transaction.

  @return  pointer to a Transaction_ctx that describes the whole transaction
           including XA-specific information (XID_STATE).
    @retval  NULL     failure
    @retval  != NULL  success
*/

static std::shared_ptr<Transaction_ctx> transaction_cache_search(XID *xid) {
  std::shared_ptr<Transaction_ctx> res{nullptr};
  mysql_mutex_lock(&LOCK_transaction_cache);

  const auto it = transaction_cache.find(to_string(*xid));
  if (it != transaction_cache.end()) res = it->second;

  mysql_mutex_unlock(&LOCK_transaction_cache);
  return res;
}

/**
  Insert information about XA transaction into a cache indexed by XID.

  @param xid     Pointer to a XID structure that identifies a XA transaction.
  @param transaction
                 Pointer to Transaction object that is inserted.

  @return  operation result
    @retval  false   success or a cache already contains XID_STATE
                     for this XID value
    @retval  true    failure
*/

bool transaction_cache_insert(XID *xid, Transaction_ctx *transaction) {
  mysql_mutex_lock(&LOCK_transaction_cache);
  std::shared_ptr<Transaction_ctx> ptr(transaction, transaction_free_hash());
  bool res = !transaction_cache.emplace(to_string(*xid), std::move(ptr)).second;
  mysql_mutex_unlock(&LOCK_transaction_cache);
  if (res) {
    my_error(ER_XAER_DUPID, MYF(0));
  }
  return res;
}

inline bool create_and_insert_new_transaction(XID *xid, bool is_binlogged_arg) {
  Transaction_ctx *transaction = new (std::nothrow) Transaction_ctx();
  XID_STATE *xs;

  if (!transaction) {
    my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(Transaction_ctx));
    return true;
  }
  xs = transaction->xid_state();
  xs->start_recovery_xa(xid, is_binlogged_arg);

  return !transaction_cache
              .emplace(to_string(*xs->get_xid()),
                       std::shared_ptr<Transaction_ctx>(
                           transaction, transaction_free_hash()))
              .second;
}

bool transaction_cache_detach(Transaction_ctx *transaction) {
  bool res = false;
  XID_STATE *xs = transaction->xid_state();
  XID xid = *(xs->get_xid());
  bool was_logged = xs->is_binlogged();

  DBUG_ASSERT(xs->has_state(XID_STATE::XA_PREPARED));

  mysql_mutex_lock(&LOCK_transaction_cache);

  DBUG_ASSERT(transaction_cache.count(to_string(xid)) != 0);
  transaction_cache.erase(to_string(xid));
  res = create_and_insert_new_transaction(&xid, was_logged);

  mysql_mutex_unlock(&LOCK_transaction_cache);

  return res;
}

/**
  Insert information about XA transaction being recovered into a cache
  indexed by XID.

  @param xid     Pointer to a XID structure that identifies a XA transaction.

  @return  operation result
    @retval  false   success or a cache already contains Transaction_ctx
                     for this XID value
    @retval  true    failure
*/

bool transaction_cache_insert_recovery(XID *xid) {
  mysql_mutex_lock(&LOCK_transaction_cache);

  if (transaction_cache.count(to_string(*xid))) {
    mysql_mutex_unlock(&LOCK_transaction_cache);
    return false;
  }

  /*
    It's assumed that XA transaction was binlogged before the server
    shutdown. If --log-bin has changed since that from OFF to ON, XA
    COMMIT or XA ROLLBACK of this transaction may be logged alone into
    the binary log.
  */
  bool res = create_and_insert_new_transaction(xid, true);

  mysql_mutex_unlock(&LOCK_transaction_cache);

  return res;
}

void transaction_cache_delete(Transaction_ctx *transaction) {
  mysql_mutex_lock(&LOCK_transaction_cache);
  const auto it =
      transaction_cache.find(to_string(*transaction->xid_state()->get_xid()));
  if (it != transaction_cache.end() && it->second.get() == transaction)
    transaction_cache.erase(it);
  mysql_mutex_unlock(&LOCK_transaction_cache);
}

/**
  The function restores previously saved storage engine transaction context.

  @param     thd     Thread context
*/
static void attach_native_trx(THD *thd) {
  Ha_trx_info *ha_info =
      thd->get_transaction()->ha_trx_info(Transaction_ctx::SESSION);
  Ha_trx_info *ha_info_next;

  if (ha_info) {
    for (; ha_info; ha_info = ha_info_next) {
      handlerton *hton = ha_info->ht();
      reattach_engine_ha_data_to_thd(thd, hton);
      ha_info_next = ha_info->next();
      ha_info->reset();
    }
  } else {
    /*
      Although the current `Ha_trx_info` object is null, we need to make sure
      that the data engine plugins have the oportunity to attach their internal
      transactions and clean up the session.
     */
    thd->rpl_reattach_engine_ha_data();
  }
}

/**
  This is a specific to "slave" applier collection of standard cleanup
  actions to reset XA transaction states at the end of XA prepare rather than
  to do it at the transaction commit, see @c ha_commit_one_phase.
  THD of the slave applier is dissociated from a transaction object in engine
  that continues to exist there.

  @param  thd current thread
  @return the value of is_error()
*/

bool applier_reset_xa_trans(THD *thd) {
  DBUG_TRACE;
  Transaction_ctx *trn_ctx = thd->get_transaction();
  XID_STATE *xid_state = trn_ctx->xid_state();

  /*
    Return error is not an option as XA is in prepared state and
    connection is gone. Log the error and continue.
  */
  if (MDL_context_backup_manager::instance().create_backup(
          &thd->mdl_context, xid_state->get_xid()->key(),
          xid_state->get_xid()->key_length())) {
    LogErr(ERROR_LEVEL, ER_XA_CANT_CREATE_MDL_BACKUP);
  }
  /*
    In the following the server transaction state gets reset for
    a slave applier thread similarly to xa_commit logics
    except commit does not run.
  */
  thd->variables.option_bits &= ~OPTION_BEGIN;
  trn_ctx->reset_unsafe_rollback_flags(Transaction_ctx::STMT);
  thd->server_status &= ~SERVER_STATUS_IN_TRANS;
  /* Server transaction ctx is detached from THD */
  transaction_cache_detach(trn_ctx);
  xid_state->reset();
  /*
     The current engine transactions is detached from THD, and
     previously saved is restored.
  */
  attach_native_trx(thd);
  trn_ctx->set_ha_trx_info(Transaction_ctx::SESSION, nullptr);
  trn_ctx->set_no_2pc(Transaction_ctx::SESSION, false);
  trn_ctx->cleanup();
#ifdef HAVE_PSI_TRANSACTION_INTERFACE
  thd->m_transaction_psi = nullptr;
#endif
  thd->mdl_context.release_transactional_locks();
  /*
    On client sessions a XA PREPARE will always be followed by a XA COMMIT
    or a XA ROLLBACK, and both statements will reset the tx isolation level
    and access mode when the statement is finishing a transaction.

    For replicated workload it is possible to have other transactions between
    the XA PREPARE and the XA [COMMIT|ROLLBACK].

    So, if the slave applier changed the current transaction isolation level,
    it needs to be restored to the session default value after having the
    XA transaction prepared.
  */
  trans_reset_one_shot_chistics(thd);

  return thd->is_error();
}

/**
  The function detaches existing storage engines transaction
  context from thd. Backup area to save it is provided to low level
  storage engine function.

  is invoked by plugin_foreach() after
  trans_xa_start() for each storage engine.

  @param[in,out]     thd     Thread context
  @param             plugin  Reference to handlerton

  @return    false   on success, true otherwise.
*/

bool detach_native_trx(THD *thd, plugin_ref plugin, void *) {
  DBUG_TRACE;
  handlerton *hton = plugin_data<handlerton *>(plugin);

  if (hton->replace_native_transaction_in_thd) {
    /* Ensure any active backup engine ha_data won't be overwritten */
    DBUG_ASSERT(!thd->get_ha_data(hton->slot)->ha_ptr_backup);

    hton->replace_native_transaction_in_thd(
        thd, nullptr, &thd->get_ha_data(hton->slot)->ha_ptr_backup);
  }

  return false;
}

bool reattach_native_trx(THD *thd, plugin_ref plugin, void *) {
  DBUG_TRACE;
  handlerton *hton = plugin_data<handlerton *>(plugin);

  if (hton->replace_native_transaction_in_thd) {
    /* restore the saved original engine transaction's link with thd */
    void **trx_backup = &thd->get_ha_data(hton->slot)->ha_ptr_backup;

    hton->replace_native_transaction_in_thd(thd, *trx_backup, nullptr);
    *trx_backup = nullptr;
  }
  return false;
}
