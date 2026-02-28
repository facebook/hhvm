/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_mi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "include/compression.h"
#include "include/mutex_lock.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql_version.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/debug_sync.h"
#include "sql/dynamic_ids.h"  // Server_ids
#include "sql/log.h"
#include "sql/mysqld.h"  // sync_masterinfo_period
#include "sql/rpl_info_handler.h"
#include "sql/rpl_msr.h"    // channel_map
#include "sql/rpl_slave.h"  // master_retry_count
#include "sql/sql_class.h"

enum {
  LINES_IN_MASTER_INFO_WITH_SSL = 14,

  /* 5.1.16 added value of master_ssl_verify_server_cert */
  LINE_FOR_MASTER_SSL_VERIFY_SERVER_CERT = 15,

  /* 5.5 added value of master_heartbeat_period */
  LINE_FOR_MASTER_HEARTBEAT_PERIOD = 16,

  /* MySQL Cluster 6.3 added master_bind */
  LINE_FOR_MASTER_BIND = 17,

  /* 6.0 added value of master_ignore_server_id */
  LINE_FOR_REPLICATE_IGNORE_SERVER_IDS = 18,

  /* 6.0 added value of master_uuid */
  LINE_FOR_MASTER_UUID = 19,

  /* line for master_retry_count */
  LINE_FOR_MASTER_RETRY_COUNT = 20,

  /* line for ssl_crl */
  LINE_FOR_SSL_CRL = 21,

  /* line for ssl_crl */
  LINE_FOR_SSL_CRLPATH = 22,

  /* line for auto_position */
  LINE_FOR_AUTO_POSITION = 23,

  /* line for channel */
  LINE_FOR_CHANNEL = 24,

  /* line for tls_version */
  LINE_FOR_TLS_VERSION = 25,

  /* line for master_public_key_path */
  LINE_FOR_PUBLIC_KEY_PATH = 26,

  /* line for get_master_public_key */
  LINE_FOR_GET_PUBLIC_KEY = 27,

  /* line for network_namespace */
  LINE_FOR_NETWORK_NAMESPACE = 28,

  /* line for master_compression_algorithm */
  LINE_FOR_MASTER_COMPRESSION_ALGORITHM = 29,

  /* line for master_zstd_compression_level */
  LINE_FOR_MASTER_ZSTD_COMPRESSION_LEVEL = 30,

  /* line for tls_ciphersuites */
  LINE_FOR_TLS_CIPHERSUITES = 31,

  /* Number of lines currently used when saving master info file */
  LINES_IN_MASTER_INFO = LINE_FOR_TLS_CIPHERSUITES

};

/*
  Please every time you add a new field to the mater info, update
  what follows. For now, this is just used to get the number of
  fields.
*/
const char *info_mi_fields[] = {"number_of_lines",
                                "master_log_name",
                                "master_log_pos",
                                "host",
                                "user",
                                "password",
                                "port",
                                "connect_retry",
                                "ssl",
                                "ssl_ca",
                                "ssl_capath",
                                "ssl_cert",
                                "ssl_cipher",
                                "ssl_key",
                                "ssl_verify_server_cert",
                                "heartbeat_period",
                                "bind",
                                "ignore_server_ids",
                                "uuid",
                                "retry_count",
                                "ssl_crl",
                                "ssl_crlpath",
                                "auto_position",
                                "channel_name",
                                "tls_version",
                                "public_key_path",
                                "get_public_key",
                                "network_namespace",
                                "master_compression_algorithm",
                                "master_zstd_compression_level",
                                "tls_ciphersuites"};

const uint info_mi_table_pk_field_indexes[] = {
    LINE_FOR_CHANNEL - 1,
};

Master_info::Master_info(
#ifdef HAVE_PSI_INTERFACE
    PSI_mutex_key *param_key_info_run_lock,
    PSI_mutex_key *param_key_info_data_lock,
    PSI_mutex_key *param_key_info_sleep_lock,
    PSI_mutex_key *param_key_info_thd_lock,
    PSI_mutex_key *param_key_info_rotate_lock,
    PSI_mutex_key *param_key_info_data_cond,
    PSI_mutex_key *param_key_info_start_cond,
    PSI_mutex_key *param_key_info_stop_cond,
    PSI_mutex_key *param_key_info_sleep_cond,
    PSI_mutex_key *param_key_info_rotate_cond,
#endif
    uint param_id, const char *param_channel)
    : Rpl_info("I/O",
#ifdef HAVE_PSI_INTERFACE
               param_key_info_run_lock, param_key_info_data_lock,
               param_key_info_sleep_lock, param_key_info_thd_lock,
               param_key_info_data_cond, param_key_info_start_cond,
               param_key_info_stop_cond, param_key_info_sleep_cond,
#endif
               param_id, param_channel),
      start_user_configured(false),
#ifdef HAVE_PSI_INTERFACE
      key_info_rotate_lock(param_key_info_rotate_lock),
      key_info_rotate_cond(param_key_info_rotate_cond),
#endif
      ssl(false),
      ssl_verify_server_cert(false),
      get_public_key(false),
      port(MYSQL_PORT),
      connect_retry(DEFAULT_CONNECT_RETRY),
      clock_diff_with_master(0),
      heartbeat_period(0),
      received_heartbeats(0),
      last_heartbeat(0),
      master_id(0),
      checksum_alg_before_fd(binary_log::BINLOG_CHECKSUM_ALG_UNDEF),
      retry_count(master_retry_count),
      mi_description_event(nullptr),
      auto_position(false),
      transaction_parser(
          Transaction_boundary_parser::TRX_BOUNDARY_PARSER_RECEIVER),
      reset(false) {
  host[0] = 0;
  user[0] = 0;
  bind_addr[0] = 0;
  network_namespace[0] = 0;
  password[0] = 0;
  start_password[0] = 0;
  ssl_ca[0] = 0;
  ssl_capath[0] = 0;
  ssl_cert[0] = 0;
  ssl_cipher[0] = 0;
  ssl_key[0] = 0;
  tls_version[0] = 0;
  ssl_crl[0] = 0;
  ssl_crlpath[0] = 0;
  master_uuid[0] = 0;
  start_plugin_auth[0] = 0;
  start_plugin_dir[0] = 0;
  start_user[0] = 0;
  public_key_path[0] = 0;
  ignore_server_ids = new Server_ids;
  strcpy(compression_algorithm, COMPRESSION_ALGORITHM_UNCOMPRESSED);
  zstd_compression_level = default_zstd_compression_level;
  server_extn.m_user_data = nullptr;
  server_extn.m_before_header = nullptr;
  server_extn.m_after_header = nullptr;
  server_extn.compress_ctx.algorithm = MYSQL_UNCOMPRESSED;
  gtid_monitoring_info = new Gtid_monitoring_info(&data_lock);

  mysql_mutex_init(*key_info_rotate_lock, &this->rotate_lock,
                   MY_MUTEX_INIT_FAST);
  mysql_cond_init(*key_info_rotate_cond, &this->rotate_cond);

  /*channel is set in base class, rpl_info.cc*/
  snprintf(for_channel_str, sizeof(for_channel_str) - 1, " for channel '%s'",
           channel);
  snprintf(for_channel_uppercase_str, sizeof(for_channel_uppercase_str) - 1,
           " FOR CHANNEL '%s'", channel);

  m_channel_lock = new Checkable_rwlock(
#ifdef HAVE_PSI_INTERFACE
      key_rwlock_channel_lock
#endif
  );
}

Master_info::~Master_info() {
  /* No one else is using this master_info */
  m_channel_lock->assert_some_wrlock();
  /* No other administrative task is able to get this master_info */
  channel_map.assert_some_wrlock();
  m_channel_lock->unlock();

  this->clear_rotate_requests();
  mysql_mutex_destroy(&rotate_lock);
  mysql_cond_destroy(&rotate_cond);

  delete m_channel_lock;
  delete ignore_server_ids;
  delete mi_description_event;
  delete gtid_monitoring_info;
}

void Master_info::request_rotate(THD *thd) {
  DBUG_TRACE;
  MUTEX_LOCK(lock, &this->rotate_lock);
  this->rotate_requested.store(true);

  DBUG_EXECUTE_IF("deferred_flush_relay_log", {
    /*
      See `gr_flush_relay_log_no_split_trx.test`
      4) Make the `FLUSH RELAY LOG` execution path to emit
         `signal.rpl_requested_for_a_flush` right before waiting on the
         transaction to end.
    */
    const char dbug_signal[] = "now SIGNAL signal.rpl_requested_for_a_flush";
    DBUG_ASSERT(
        !debug_sync_set_action(current_thd, STRING_WITH_LEN(dbug_signal)));
  });

  while (this->rotate_requested.load() && !thd->killed)
    mysql_cond_wait(&this->rotate_cond, &this->rotate_lock);
}

void Master_info::clear_rotate_requests() {
  DBUG_TRACE;
  MUTEX_LOCK(lock, &this->rotate_lock);

  if (this->rotate_requested.load()) {
    this->rotate_requested.store(false);
    mysql_cond_broadcast(&this->rotate_cond);

    DBUG_EXECUTE_IF("deferred_flush_relay_log", {
      /*
        See `gr_flush_relay_log_no_split_trx.test`
        7) Make the applier execution path to emit
           `signal.rpl_broadcasted_rotate_end` just after finishing processing
           the deferred flushing of the relay log.
      */
      const char dbug_signal[] = "now SIGNAL signal.rpl_broadcasted_rotate_end";
      DBUG_ASSERT(
          !debug_sync_set_action(current_thd, STRING_WITH_LEN(dbug_signal)));
    });
  }
}

bool Master_info::is_rotate_requested() {
  return this->rotate_requested.load();
}

/**
   Reports if the s_id server has been configured to ignore events
   it generates with

      CHANGE MASTER IGNORE_SERVER_IDS= ( list of server ids )

   Method is called from the io thread event receiver filtering.

   @param      s_id    the master server identifier

   @retval   true    if s_id is in the list of ignored master  servers,
   @retval   false   otherwise.
 */
bool Master_info::shall_ignore_server_id(ulong s_id) {
  return std::binary_search(ignore_server_ids->dynamic_ids.begin(),
                            ignore_server_ids->dynamic_ids.end(), s_id);
}

void Master_info::init_master_log_pos() {
  DBUG_TRACE;

  master_log_name[0] = 0;
  master_log_pos = BIN_LOG_HEADER_SIZE;  // skip magic number
  flushed_relay_log_info.log_file_name[0] = 0;
  flushed_relay_log_info.pos = 0;
}

void Master_info::end_info() {
  DBUG_TRACE;

  if (!inited) return;

  handler->end_info();

  inited = false;
  reset = true;
}

/**
  Store the master file and position where the slave's I/O thread are in the
  relay log.

  This function should be called either from the slave I/O thread, or when the
  slave thread is not running.

  It can also be called by any function changing the relay log, regardless
  of changing master positions (i.e. a FLUSH RELAY LOGS that rotates the relay
  log without changing master positions).

  Error can happen if writing to repository fails or if flushing the repository
  fails.

  @param force when true, do not respect sync period and flush information.
               when false, flush will only happen if it is time to flush.
*/

int Master_info::flush_info(bool force) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("master_pos: %lu", (ulong)master_log_pos));

  bool skip_flushing = !inited;
  /*
    A Master_info of a channel that was inited and then reset must be flushed
    into the repository or else its connection configuration will be lost in
    case the server restarts before starting the channel again.
  */
  if (force && reset) skip_flushing = false;

  if (skip_flushing) return 0;

  /*
    We update the sync_period at this point because only here we
    now that we are handling a master info. This needs to be
    update every time we call flush because the option maybe
    dynamically set.
  */
  if (inited) handler->set_sync_period(sync_masterinfo_period);

  handler->inc_sync_counter();

  bool do_flush = sync_masterinfo_period &&
                  handler->get_sync_counter() >= sync_masterinfo_period;

  /*
    Check whether a write is actually necessary. If not checked,
    write_info() causes unnecessary code path which copies (sprintf),
    writes to file cache and flush_info() causes unnecessary flush of the
    file cache which are anyway completely useless in recovery since
    they are not transactional if we are using FILE based repository.
  */
  if (skip_flush_master_info && !(force || do_flush)) return 0;

  if (write_info(handler)) goto err;

  if (handler->flush_info(force)) goto err;

  update_flushed_relay_log_info();

  return 0;

err:
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_WRITING_MASTER_CONFIGURATION);
  return 1;
}

void Master_info::set_relay_log_info(Relay_log_info *info) { rli = info; }

/**
  Creates or reads information from the repository, initializing the
  Master_info.
*/
int Master_info::mi_init_info() {
  DBUG_TRACE;
  enum_return_check check_return = ERROR_CHECKING_REPOSITORY;

  if (inited) return 0;

  mysql = nullptr;
  file_id = 1;
  if ((check_return = check_info()) == ERROR_CHECKING_REPOSITORY) goto err;

  if (handler->init_info()) goto err;

  if (check_return == REPOSITORY_DOES_NOT_EXIST) {
    init_master_log_pos();
  } else {
    if (read_info(handler)) goto err;
  }

  inited = true;
  reset = false;
  if (flush_info(true)) goto err;

  return 0;

err:
  handler->end_info();
  inited = false;
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_READING_MASTER_CONFIGURATION);
  return 1;
}

size_t Master_info::get_number_info_mi_fields() {
  return sizeof(info_mi_fields) / sizeof(info_mi_fields[0]);
}

uint Master_info::get_channel_field_num() {
  uint channel_field = LINE_FOR_CHANNEL;
  return channel_field;
}

const uint *Master_info::get_table_pk_field_indexes() {
  return info_mi_table_pk_field_indexes;
}

void Master_info::set_nullable_fields(MY_BITMAP *nullable_fields) {
  bitmap_init(nullable_fields, nullptr,
              Master_info::get_number_info_mi_fields());
  bitmap_clear_all(nullable_fields);
  /* Identify which fields can store a NULL value. */
  bitmap_set_bit(nullable_fields, LINE_FOR_TLS_CIPHERSUITES - 1);
}

bool Master_info::read_info(Rpl_info_handler *from) {
  int lines = 0;
  char *first_non_digit = nullptr;
  ulong temp_master_log_pos = 0;
  int temp_ssl = 0;
  int temp_ssl_verify_server_cert = 0;
  int temp_auto_position = 0;
  int temp_get_public_key = 0;

  DBUG_TRACE;

  /*
     Starting from 4.1.x master.info has new format. Now its
     first line contains number of lines in file. By reading this
     number we will be always distinguish to which version our
     master.info corresponds to. We can't simply count lines in
     file since versions before 4.1.x could generate files with more
     lines than needed.
     If first line doesn't contain a number or contain number less than
     LINES_IN_MASTER_INFO_WITH_SSL then such file is treated like file
     from pre 4.1.1 version.
     There is no ambiguity when reading an old master.info, as before
     4.1.1, the first line contained the binlog's name, which is either
     empty or has an extension (contains a '.'), so can't be confused
     with an integer.

     So we're just reading first line and trying to figure which version
     is this.
  */

  if (from->prepare_info_for_read() ||
      !!from->get_info(master_log_name, sizeof(master_log_name), ""))
    return true;

  lines = strtoul(master_log_name, &first_non_digit, 10);

  if (master_log_name[0] != '\0' && *first_non_digit == '\0' &&
      lines >= LINES_IN_MASTER_INFO_WITH_SSL) {
    /* Seems to be new format => read master log name */
    if (!!from->get_info(master_log_name, sizeof(master_log_name), ""))
      return true;
  } else
    lines = 7;

  if (!!from->get_info(&temp_master_log_pos, (ulong)BIN_LOG_HEADER_SIZE) ||
      !!from->get_info(host, sizeof(host), (char *)nullptr) ||
      !!from->get_info(user, sizeof(user), "test") ||
      !!from->get_info(password, sizeof(password), (char *)nullptr) ||
      !!from->get_info((int *)&port, (int)MYSQL_PORT) ||
      !!from->get_info((int *)&connect_retry, (int)DEFAULT_CONNECT_RETRY))
    return true;

  /*
    If file has ssl part use it even if we have server without
    SSL support. But these options will be ignored later when
    slave will try connect to master, so in this case warning
    is printed.
  */
  if (lines >= LINES_IN_MASTER_INFO_WITH_SSL) {
    if (!!from->get_info(&temp_ssl, 0) ||
        !!from->get_info(ssl_ca, sizeof(ssl_ca), (char *)nullptr) ||
        !!from->get_info(ssl_capath, sizeof(ssl_capath), (char *)nullptr) ||
        !!from->get_info(ssl_cert, sizeof(ssl_cert), (char *)nullptr) ||
        !!from->get_info(ssl_cipher, sizeof(ssl_cipher), (char *)nullptr) ||
        !!from->get_info(ssl_key, sizeof(ssl_key), (char *)nullptr))
      return true;
  }

  /*
    Starting from 5.1.16 ssl_verify_server_cert might be
    in the file
  */
  if (lines >= LINE_FOR_MASTER_SSL_VERIFY_SERVER_CERT) {
    if (!!from->get_info(&temp_ssl_verify_server_cert, 0)) return true;
  }

  /*
    Starting from 5.5 master_heartbeat_period might be
    in the file
  */
  if (lines >= LINE_FOR_MASTER_HEARTBEAT_PERIOD) {
    if (!!from->get_info(&heartbeat_period, (float)0.0)) return true;
  }

  /*
    Starting from 5.5 master_bind might be in the file
  */
  if (lines >= LINE_FOR_MASTER_BIND) {
    if (!!from->get_info(bind_addr, sizeof(bind_addr), "")) return true;
  }

  /*
    Starting from 5.5 list of server_id of ignorable servers might be
    in the file
  */
  if (lines >= LINE_FOR_REPLICATE_IGNORE_SERVER_IDS) {
    if (!!from->get_info(ignore_server_ids, (Server_ids *)nullptr)) return true;
  }

  /* Starting from 5.5 the master_uuid may be in the repository. */
  if (lines >= LINE_FOR_MASTER_UUID) {
    if (!!from->get_info(master_uuid, sizeof(master_uuid), (char *)nullptr))
      return true;
  }

  /* Starting from 5.5 the master_retry_count may be in the repository. */
  retry_count = master_retry_count;
  if (lines >= LINE_FOR_MASTER_RETRY_COUNT) {
    if (!!from->get_info(&retry_count, master_retry_count)) return true;
  }

  if (lines >= LINE_FOR_SSL_CRLPATH) {
    if (!!from->get_info(ssl_crl, sizeof(ssl_crl), (char *)nullptr) ||
        !!from->get_info(ssl_crlpath, sizeof(ssl_crlpath), (char *)nullptr))
      return true;
  }

  if (lines >= LINE_FOR_AUTO_POSITION) {
    if (!!from->get_info(&temp_auto_position, 0)) return true;
  }

  if (lines >= LINE_FOR_CHANNEL) {
    if (!!from->get_info(channel, sizeof(channel), (char *)nullptr))
      return true;
  }

  if (lines >= LINE_FOR_TLS_VERSION) {
    if (!!from->get_info(tls_version, sizeof(tls_version), (char *)nullptr))
      return true;
  }

  if (lines >= LINE_FOR_PUBLIC_KEY_PATH) {
    if (!!from->get_info(public_key_path, sizeof(public_key_path),
                         (char *)nullptr))
      return true;
  }

  if (lines >= LINE_FOR_GET_PUBLIC_KEY) {
    if (!!from->get_info(&temp_get_public_key, 0)) return true;
  }

  if (lines >= LINE_FOR_NETWORK_NAMESPACE) {
    if (!!from->get_info(network_namespace, sizeof(network_namespace),
                         (char *)nullptr))
      return true;
  }

  ssl = (bool)temp_ssl;
  ssl_verify_server_cert = (bool)temp_ssl_verify_server_cert;
  master_log_pos = (my_off_t)temp_master_log_pos;
  auto_position = temp_auto_position;
  get_public_key = (bool)temp_get_public_key;

  if (lines >= LINE_FOR_MASTER_COMPRESSION_ALGORITHM) {
    char algorithm_name[COMPRESSION_ALGORITHM_NAME_BUFFER_SIZE];
    if (!!from->get_info(algorithm_name, sizeof(algorithm_name), nullptr))
      return true;
    else {
      if (validate_compression_attributes(algorithm_name, channel, true)) {
        LogErr(WARNING_LEVEL, ER_WARN_WRONG_COMPRESSION_ALGORITHM_LOG,
               algorithm_name, channel);
        strcpy(compression_algorithm, COMPRESSION_ALGORITHM_UNCOMPRESSED);
      } else {
        DBUG_ASSERT(strlen(algorithm_name) < sizeof(compression_algorithm));
        strcpy(compression_algorithm, algorithm_name);
      }
    }
  }

  if (lines >= LINE_FOR_MASTER_ZSTD_COMPRESSION_LEVEL) {
    int level;
    if (!!from->get_info(&level, (int)0)) return true;
    if (is_zstd_compression_level_valid(level))
      zstd_compression_level = level;
    else {
      int default_level = default_zstd_compression_level;
      LogErr(WARNING_LEVEL, ER_WARN_WRONG_COMPRESSION_LEVEL_LOG, channel,
             default_level);
      zstd_compression_level = default_level;
    }
  }

  if (lines >= LINE_FOR_TLS_CIPHERSUITES) {
    char buffer[FN_REFLEN_SE] = {0};

    Rpl_info_handler::enum_field_get_status status =
        from->get_info(buffer, sizeof(buffer), nullptr);

    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;

    if (status ==
        Rpl_info_handler::enum_field_get_status::FIELD_VALUE_IS_NULL) {
      tls_ciphersuites.first = true;
      tls_ciphersuites.second.clear();
    } else {
      DBUG_ASSERT(
          status ==
          Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL);
      tls_ciphersuites.first = false;
      tls_ciphersuites.second.assign(buffer);
    }
  }

  return false;
}

bool Master_info::set_info_search_keys(Rpl_info_handler *to) {
  DBUG_TRACE;

  if (to->set_info(LINE_FOR_CHANNEL - 1, channel)) return true;

  return false;
}

bool Master_info::write_info(Rpl_info_handler *to) {
  DBUG_TRACE;

  /*
     In certain cases this code may create master.info files that seems
     corrupted, because of extra lines filled with garbage in the end
     file (this happens if new contents take less space than previous
     contents of file). But because of number of lines in the first line
     of file we don't care about this garbage.
  */
  if (to->prepare_info_for_write()) return true;
  if (to->get_rpl_info_type() != INFO_REPOSITORY_FILE) {
    if (to->set_info((int)LINES_IN_MASTER_INFO) ||
        to->set_info(master_log_name) || to->set_info((ulong)master_log_pos) ||
        to->set_info(host) || to->set_info(user) || to->set_info(password) ||
        to->set_info((int)port) || to->set_info((int)connect_retry) ||
        to->set_info((int)ssl) || to->set_info(ssl_ca) ||
        to->set_info(ssl_capath) || to->set_info(ssl_cert) ||
        to->set_info(ssl_cipher) || to->set_info(ssl_key) ||
        to->set_info((int)ssl_verify_server_cert) ||
        to->set_info(heartbeat_period) || to->set_info(bind_addr) ||
        to->set_info(ignore_server_ids) || to->set_info(master_uuid) ||
        to->set_info(retry_count) || to->set_info(ssl_crl) ||
        to->set_info(ssl_crlpath) || to->set_info((int)auto_position) ||
        to->set_info(channel) || to->set_info(tls_version) ||
        to->set_info(public_key_path) || to->set_info(get_public_key) ||
        to->set_info(network_namespace) ||
        to->set_info(compression_algorithm) ||
        to->set_info((int)zstd_compression_level) ||
        to->set_info(tls_ciphersuites.first ? nullptr
                                            : tls_ciphersuites.second.c_str()))
      return true;
  } else {
    String buffer;
    char hb_period[64];
    sprintf(hb_period, "%.3f", heartbeat_period);

    if (ignore_server_ids->pack_dynamic_ids(&buffer)) return true;
    if (to->set_info(
            LINES_IN_MASTER_INFO, /* 31 params after the format string */
            "%d\n%s\n%lu\n%s\n%s\n%s\n%u\n%u\n"
            "%d\n%s\n%s\n%s\n%s\n%s\n%d\n%s\n%s\n%s\n%s\n%lu\n%s\n"
            "%s\n%d\n%s\n%s\n%s\n%d\n%s\n%s\n%d\n%s\n",
            (int)LINES_IN_MASTER_INFO, master_log_name, (ulong)master_log_pos,
            host, user, password, port, connect_retry, ssl, ssl_ca, ssl_capath,
            ssl_cert, ssl_cipher, ssl_key, (int)ssl_verify_server_cert,
            hb_period, bind_addr, buffer.c_ptr_safe(), master_uuid, retry_count,
            ssl_crl, ssl_crlpath, (int)auto_position, channel, tls_version,
            public_key_path, get_public_key, network_namespace,
            compression_algorithm, (int)zstd_compression_level,
            tls_ciphersuites.first ? "" : tls_ciphersuites.second.c_str()))
      return true;
  }
  return false;
}

void Master_info::set_password(const char *password_arg) {
  DBUG_TRACE;

  DBUG_ASSERT(password_arg);

  if (password_arg && start_user_configured)
    strmake(start_password, password_arg, sizeof(start_password) - 1);
  else if (password_arg)
    strmake(password, password_arg, sizeof(password) - 1);
}

bool Master_info::get_password(char *password_arg, size_t *password_arg_size) {
  bool ret = true;
  DBUG_TRACE;

  if (password_arg && start_user_configured) {
    *password_arg_size = strlen(start_password);
    strmake(password_arg, start_password, sizeof(start_password) - 1);
    ret = false;
  } else if (password_arg) {
    *password_arg_size = strlen(password);
    strmake(password_arg, password, sizeof(password) - 1);
    ret = false;
  }
  return ret;
}

void Master_info::reset_start_info() {
  DBUG_TRACE;
  start_plugin_auth[0] = 0;
  start_plugin_dir[0] = 0;
  start_user_configured = false;
  start_user[0] = 0;
  start_password[0] = 0;
}

void Master_info::channel_rdlock() {
  channel_map.assert_some_lock();
  m_channel_lock->rdlock();
}

void Master_info::channel_wrlock() {
  channel_map.assert_some_lock();
  m_channel_lock->wrlock();
}

void Master_info::wait_until_no_reference(THD *thd) {
  PSI_stage_info *old_stage = nullptr;

  thd->enter_stage(&stage_waiting_for_no_channel_reference, old_stage, __func__,
                   __FILE__, __LINE__);

  while (atomic_references != 0) my_sleep(10000);

  THD_STAGE_INFO(thd, *old_stage);
}

bool Master_info::is_ignore_server_ids_configured() {
  return ignore_server_ids->dynamic_ids.size() > 0;
}

void Master_info::update_flushed_relay_log_info() {
  MYSQL_BIN_LOG *relay_log = &rli->relay_log;
  mysql_mutex_assert_owner(&data_lock);
  if (rli->inited)
    relay_log->get_current_log(&flushed_relay_log_info, false);
  else {
    flushed_relay_log_info.log_file_name[0] = 0;
    flushed_relay_log_info.pos = 0;
  }
}

void Master_info::get_flushed_relay_log_info(LOG_INFO *linfo) {
  strmake(linfo->log_file_name, flushed_relay_log_info.log_file_name,
          sizeof(linfo->log_file_name) - 1);
  linfo->pos = flushed_relay_log_info.pos;
}
