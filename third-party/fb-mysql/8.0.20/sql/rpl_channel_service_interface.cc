/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_channel_service_interface.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <atomic>
#include <map>
#include <sstream>
#include <utility>

#include "mutex_lock.h"  // MUTEX_LOCK
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"
#include "sql/current_thd.h"
#include "sql/log.h"
#include "sql/log_event.h"
#include "sql/mysqld.h"              // opt_mts_slave_parallel_workers
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol_classic.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_info_factory.h"
#include "sql/rpl_info_handler.h"
#include "sql/rpl_mi.h"
#include "sql/rpl_msr.h" /* Multisource replication */
#include "sql/rpl_mts_submode.h"
#include "sql/rpl_rli.h"
#include "sql/rpl_rli_pdb.h"
#include "sql/rpl_slave.h"
#include "sql/rpl_trx_boundary_parser.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"

/**
  Auxiliary function to stop all the running channel threads according to the
  given mask.

  @note: The caller shall possess channel_map lock before calling this function,
         and unlock after returning from this function.

  @param mi                   The pointer to Master_info instance
  @param threads_to_stop      The types of threads to be stopped
  @param timeout              The expected time in which the thread should stop

  @return the operation status
    @retval 0      OK
    @retval !=0    Error
*/
int channel_stop(Master_info *mi, int threads_to_stop, long timeout);

int initialize_channel_service_interface() {
  DBUG_TRACE;

  // master info and relay log repositories must be TABLE
  if (opt_mi_repository_id != INFO_REPOSITORY_TABLE ||
      opt_rli_repository_id != INFO_REPOSITORY_TABLE) {
    LogErr(ERROR_LEVEL, ER_RPL_CHANNELS_REQUIRE_TABLES_AS_INFO_REPOSITORIES);
    return 1;
  }

  // server id must be different from 0
  if (server_id == 0) {
    LogErr(ERROR_LEVEL, ER_RPL_CHANNELS_REQUIRE_NON_ZERO_SERVER_ID);
    return 1;
  }

  return 0;
}

static void set_mi_settings(Master_info *mi,
                            Channel_creation_info *channel_info) {
  mysql_mutex_lock(mi->rli->relay_log.get_log_lock());
  mysql_mutex_lock(&mi->data_lock);

  mi->rli->set_thd_tx_priority(channel_info->thd_tx_priority);

  mi->rli->replicate_same_server_id =
      (channel_info->replicate_same_server_id == RPL_SERVICE_SERVER_DEFAULT)
          ? replicate_same_server_id
          : channel_info->replicate_same_server_id;

  mi->rli->opt_slave_parallel_workers =
      (channel_info->channel_mts_parallel_workers == RPL_SERVICE_SERVER_DEFAULT)
          ? opt_mts_slave_parallel_workers
          : channel_info->channel_mts_parallel_workers;

  if (channel_info->channel_mts_parallel_type == RPL_SERVICE_SERVER_DEFAULT) {
    const auto parallel_option = get_mts_parallel_option();
    if (parallel_option == MTS_PARALLEL_TYPE_DB_NAME)
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DB_NAME;
    else if (parallel_option == MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
    else
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DEPENDENCY;
  } else {
    if (channel_info->channel_mts_parallel_type ==
        CHANNEL_MTS_PARALLEL_TYPE_DB_NAME)
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DB_NAME;
    else if (channel_info->channel_mts_parallel_type ==
             MTS_PARALLEL_TYPE_LOGICAL_CLOCK)
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
    else
      mi->rli->channel_mts_submode = MTS_PARALLEL_TYPE_DEPENDENCY;
  }

  mi->rli->checkpoint_group =
      (channel_info->channel_mts_checkpoint_group == RPL_SERVICE_SERVER_DEFAULT)
          ? opt_mts_checkpoint_group
          : channel_info->channel_mts_checkpoint_group;

  mi->set_mi_description_event(new Format_description_log_event());

  mysql_mutex_unlock(&mi->data_lock);
  mysql_mutex_unlock(mi->rli->relay_log.get_log_lock());
}

static bool init_thread_context() { return my_thread_init(); }

static void clean_thread_context() { my_thread_end(); }

static THD *create_surrogate_thread() {
  THD *thd = nullptr;
  thd = new THD;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();
  thd->security_context()->skip_grants();

  return (thd);
}

static void delete_surrogate_thread(THD *thd) {
  thd->release_resources();
  delete thd;
  current_thd = nullptr;
}

void initialize_channel_creation_info(Channel_creation_info *channel_info) {
  channel_info->type = SLAVE_REPLICATION_CHANNEL;
  channel_info->hostname = nullptr;
  channel_info->port = 0;
  channel_info->user = nullptr;
  channel_info->password = nullptr;
  channel_info->ssl_info = nullptr;
  channel_info->auto_position = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->channel_mts_parallel_type = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->channel_mts_parallel_workers = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->channel_mts_checkpoint_group = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->replicate_same_server_id = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->thd_tx_priority = 0;
  channel_info->sql_delay = RPL_SERVICE_SERVER_DEFAULT;
  channel_info->preserve_relay_logs = false;
  channel_info->retry_count = 0;
  channel_info->connect_retry = 0;
  channel_info->public_key_path = nullptr;
  channel_info->get_public_key = 0;
  channel_info->compression_algorithm = nullptr;
  channel_info->zstd_compression_level = 0;
}

void initialize_channel_ssl_info(Channel_ssl_info *channel_ssl_info) {
  channel_ssl_info->use_ssl = 0;
  channel_ssl_info->ssl_ca_file_name = nullptr;
  channel_ssl_info->ssl_ca_directory = nullptr;
  channel_ssl_info->ssl_cert_file_name = nullptr;
  channel_ssl_info->ssl_crl_file_name = nullptr;
  channel_ssl_info->ssl_crl_directory = nullptr;
  channel_ssl_info->ssl_key = nullptr;
  channel_ssl_info->ssl_cipher = nullptr;
  channel_ssl_info->tls_version = nullptr;
  channel_ssl_info->ssl_verify_server_cert = 0;
  channel_ssl_info->tls_ciphersuites = nullptr;
}

void initialize_channel_connection_info(Channel_connection_info *channel_info) {
  channel_info->until_condition = CHANNEL_NO_UNTIL_CONDITION;
  channel_info->gtid = nullptr;
  channel_info->view_id = nullptr;
}

static void set_mi_ssl_options(LEX_MASTER_INFO *lex_mi,
                               Channel_ssl_info *channel_ssl_info) {
  lex_mi->ssl = (channel_ssl_info->use_ssl) ? LEX_MASTER_INFO::LEX_MI_ENABLE
                                            : LEX_MASTER_INFO::LEX_MI_DISABLE;

  if (channel_ssl_info->ssl_ca_file_name != nullptr) {
    lex_mi->ssl_ca = channel_ssl_info->ssl_ca_file_name;
  }

  if (channel_ssl_info->ssl_ca_directory != nullptr) {
    lex_mi->ssl_capath = channel_ssl_info->ssl_ca_directory;
  }

  if (channel_ssl_info->tls_version != nullptr) {
    lex_mi->tls_version = channel_ssl_info->tls_version;
  }

  if (channel_ssl_info->ssl_cert_file_name != nullptr) {
    lex_mi->ssl_cert = channel_ssl_info->ssl_cert_file_name;
  }

  if (channel_ssl_info->ssl_crl_file_name != nullptr) {
    lex_mi->ssl_crl = channel_ssl_info->ssl_crl_file_name;
  }

  if (channel_ssl_info->ssl_crl_directory != nullptr) {
    lex_mi->ssl_crlpath = channel_ssl_info->ssl_crl_directory;
  }

  if (channel_ssl_info->ssl_key != nullptr) {
    lex_mi->ssl_key = channel_ssl_info->ssl_key;
  }

  if (channel_ssl_info->ssl_cipher != nullptr) {
    lex_mi->ssl_cipher = channel_ssl_info->ssl_cipher;
  }

  if (channel_ssl_info->tls_ciphersuites != nullptr) {
    lex_mi->tls_ciphersuites = LEX_MASTER_INFO::SPECIFIED_STRING;
    lex_mi->tls_ciphersuites_string = channel_ssl_info->tls_ciphersuites;
  } else {
    lex_mi->tls_ciphersuites = LEX_MASTER_INFO::SPECIFIED_NULL;
  }

  lex_mi->ssl_verify_server_cert = (channel_ssl_info->ssl_verify_server_cert)
                                       ? LEX_MASTER_INFO::LEX_MI_ENABLE
                                       : LEX_MASTER_INFO::LEX_MI_DISABLE;
}

int channel_create(const char *channel, Channel_creation_info *channel_info) {
  DBUG_TRACE;

  Master_info *mi = nullptr;
  int error = 0;
  LEX_MASTER_INFO *lex_mi = nullptr;

  bool thd_created = false;
  THD *thd = current_thd;

  // Don't create default channels
  if (!strcmp(channel_map.get_default_channel(), channel))
    return RPL_CHANNEL_SERVICE_DEFAULT_CHANNEL_CREATION_ERROR;

  /* Service channels are not supposed to use sql_slave_skip_counter */
  mysql_mutex_lock(&LOCK_sql_slave_skip_counter);
  if (sql_slave_skip_counter > 0)
    error = RPL_CHANNEL_SERVICE_SLAVE_SKIP_COUNTER_ACTIVE;
  mysql_mutex_unlock(&LOCK_sql_slave_skip_counter);
  if (error) return error;

  channel_map.wrlock();

  /* Get the Master_info of the channel */
  mi = channel_map.get_mi(channel);

  /* create a new channel if doesn't exist */
  if (!mi) {
    if ((error = add_new_channel(&mi, channel))) goto err;
  }

  lex_mi = new LEX_MASTER_INFO();
  lex_mi->channel = channel;
  lex_mi->host = channel_info->hostname;
  /*
    'group_replication_recovery' channel (*after recovery is done*)
    or 'group_replication_applier' channel wants to set the port number
    to '0' as there is no actual network usage on these channels.
  */
  lex_mi->port_opt = LEX_MASTER_INFO::LEX_MI_ENABLE;
  lex_mi->port = channel_info->port;
  lex_mi->user = channel_info->user;
  lex_mi->password = channel_info->password;
  lex_mi->sql_delay = channel_info->sql_delay;
  lex_mi->connect_retry = channel_info->connect_retry;
  if (channel_info->retry_count) {
    lex_mi->retry_count_opt = LEX_MASTER_INFO::LEX_MI_ENABLE;
    lex_mi->retry_count = channel_info->retry_count;
  }

  if (channel_info->auto_position) {
    lex_mi->auto_position = LEX_MASTER_INFO::LEX_MI_ENABLE;
    if ((mi && mi->is_auto_position()) ||
        channel_info->auto_position == RPL_SERVICE_SERVER_DEFAULT) {
      // So change master allows new configurations with a running SQL thread
      lex_mi->auto_position = LEX_MASTER_INFO::LEX_MI_UNCHANGED;
    }
  }

  if (channel_info->public_key_path) {
    lex_mi->public_key_path = channel_info->public_key_path;
  }

  if (channel_info->get_public_key) {
    lex_mi->get_public_key = LEX_MASTER_INFO::LEX_MI_ENABLE;
    if (mi && mi->get_public_key) {
      // So change master allows new configurations with a running SQL thread
      lex_mi->get_public_key = LEX_MASTER_INFO::LEX_MI_UNCHANGED;
    }
  } else {
    lex_mi->get_public_key = LEX_MASTER_INFO::LEX_MI_DISABLE;
    if (mi && !mi->get_public_key) {
      // So change master allows new configurations with a running SQL thread
      lex_mi->get_public_key = LEX_MASTER_INFO::LEX_MI_UNCHANGED;
    }
  }

  if (channel_info->compression_algorithm != nullptr) {
    lex_mi->compression_algorithm = channel_info->compression_algorithm;
  }
  if (channel_info->zstd_compression_level) {
    lex_mi->zstd_compression_level = channel_info->zstd_compression_level;
  }

  if (channel_info->ssl_info != nullptr) {
    set_mi_ssl_options(lex_mi, channel_info->ssl_info);
  }

  if (mi) {
    if (!thd) {
      thd_created = true;
      thd = create_surrogate_thread();
    }

    if ((error = change_master(thd, mi, lex_mi,
                               channel_info->preserve_relay_logs))) {
      goto err;
    }
  }

  set_mi_settings(mi, channel_info);

  if (channel_map.is_group_replication_channel_name(mi->get_channel())) {
    thd->variables.max_allowed_packet = slave_max_allowed_packet;
    thd->get_protocol_classic()->set_max_packet_size(slave_max_allowed_packet +
                                                     MAX_LOG_EVENT_HEADER);
  }

err:
  channel_map.unlock();

  if (thd_created) {
    delete_surrogate_thread(thd);
  }

  delete lex_mi;

  return error;
}

int channel_start(const char *channel, Channel_connection_info *connection_info,
                  int threads_to_start, int wait_for_connection) {
  DBUG_TRACE;
  int error = 0;
  int thread_mask = 0;
  LEX_MASTER_INFO lex_mi;
  ulong thread_start_id = 0;
  bool thd_created = false;
  THD *thd = current_thd;

  /* Service channels are not supposed to use sql_slave_skip_counter */
  mysql_mutex_lock(&LOCK_sql_slave_skip_counter);
  if (sql_slave_skip_counter > 0)
    error = RPL_CHANNEL_SERVICE_SLAVE_SKIP_COUNTER_ACTIVE;
  mysql_mutex_unlock(&LOCK_sql_slave_skip_counter);
  if (error) return error;

  channel_map.wrlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    error = RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
    goto err;
  }

  if (threads_to_start & CHANNEL_APPLIER_THREAD) {
    thread_mask |= SLAVE_SQL;
  }
  if (threads_to_start & CHANNEL_RECEIVER_THREAD) {
    thread_mask |= SLAVE_IO;
  }

  // Nothing to be done here
  if (!thread_mask) goto err;

  LEX_SLAVE_CONNECTION lex_connection;
  lex_connection.reset();

  if (connection_info->until_condition != CHANNEL_NO_UNTIL_CONDITION) {
    switch (connection_info->until_condition) {
      case CHANNEL_UNTIL_APPLIER_AFTER_GTIDS:
        lex_mi.gtid_until_condition = LEX_MASTER_INFO::UNTIL_SQL_AFTER_GTIDS;
        lex_mi.gtid = connection_info->gtid;
        break;
      case CHANNEL_UNTIL_APPLIER_BEFORE_GTIDS:
        lex_mi.gtid_until_condition = LEX_MASTER_INFO::UNTIL_SQL_BEFORE_GTIDS;
        lex_mi.gtid = connection_info->gtid;
        break;
      case CHANNEL_UNTIL_APPLIER_AFTER_GAPS:
        lex_mi.until_after_gaps = true;
        break;
      case CHANNEL_UNTIL_VIEW_ID:
        DBUG_ASSERT((thread_mask & SLAVE_SQL) && connection_info->view_id);
        lex_mi.view_id = connection_info->view_id;
        break;
      default:
        DBUG_ASSERT(0);
    }
  }

  if (wait_for_connection && (thread_mask & SLAVE_IO))
    thread_start_id = mi->slave_run_id;

  if (!thd) {
    thd_created = true;
    thd = create_surrogate_thread();
  }

  error = start_slave(thd, &lex_connection, &lex_mi, thread_mask, mi, false);

  if (wait_for_connection && (thread_mask & SLAVE_IO) && !error) {
    mysql_mutex_lock(&mi->run_lock);
    /*
      If the ids are still equal this means the start thread method did not
      wait for the thread to start
    */
    while (thread_start_id == mi->slave_run_id) {
      mysql_cond_wait(&mi->start_cond, &mi->run_lock);
    }
    mysql_mutex_unlock(&mi->run_lock);

    while (mi->slave_running != MYSQL_SLAVE_RUN_CONNECT) {
      // If there is such a state change then there was an error on connection
      if (mi->slave_running == MYSQL_SLAVE_NOT_RUN) {
        error = RPL_CHANNEL_SERVICE_RECEIVER_CONNECTION_ERROR;
        break;
      }
      my_sleep(100);
    }
  }

err:
  channel_map.unlock();

  if (thd_created) {
    delete_surrogate_thread(thd);
  }

  return error;
}

int channel_stop(Master_info *mi, int threads_to_stop, long timeout) {
  channel_map.assert_some_lock();

  if (mi == nullptr) {
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  int thread_mask = 0;
  int server_thd_mask = 0;
  int error = 0;
  bool thd_init = false;

  mi->channel_wrlock();
  lock_slave_threads(mi);

  init_thread_mask(&server_thd_mask, mi, false /* not inverse*/);

  if ((threads_to_stop & CHANNEL_APPLIER_THREAD) &&
      (server_thd_mask & SLAVE_SQL)) {
    thread_mask |= SLAVE_SQL;
  }
  if ((threads_to_stop & CHANNEL_RECEIVER_THREAD) &&
      (server_thd_mask & SLAVE_IO)) {
    thread_mask |= SLAVE_IO;
  }

  if (thread_mask == 0) {
    goto end;
  }

  thd_init = init_thread_context();

  if (current_thd) current_thd->set_skip_readonly_check();

  error = terminate_slave_threads(mi, thread_mask, timeout, false);

  if (current_thd) current_thd->reset_skip_readonly_check();

end:
  unlock_slave_threads(mi);
  mi->channel_unlock();

  if (thd_init) {
    clean_thread_context();
  }

  return error;
}

int channel_stop(const char *channel, int threads_to_stop, long timeout) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  int error = channel_stop(mi, threads_to_stop, timeout);

  channel_map.unlock();

  return error;
}

int channel_stop_all(int threads_to_stop, long timeout,
                     std::string *error_message) {
  Master_info *mi = nullptr;

  /* Error related varaiables */
  int error = 0;
  std::stringstream err_msg_ss;
  err_msg_ss << "Error stopping channel(s): ";

  channel_map.rdlock();

  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;

    if (mi) {
      int channel_error = channel_stop(mi, threads_to_stop, timeout);

      DBUG_EXECUTE_IF("group_replication_stop_all_channels_failure",
                      { channel_error = 1; });

      if (channel_error &&
          channel_error != RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR) {
        error = channel_error;

        mi->report(ERROR_LEVEL, error,
                   "Error stopping channel: %s. Got error: %d",
                   mi->get_channel(), error);

        err_msg_ss << " '" << mi->get_channel() << "' [error number: " << error
                   << "],";
      }
    }
  }

  if (error) {
    *error_message = err_msg_ss.str();
    (*error_message)[error_message->length() - 1] = '.';
    error_message->append(
        " Please check the error log for additional details.");
  }

  channel_map.unlock();
  return error;
}

class Kill_binlog_dump : public Do_THD_Impl {
 public:
  Kill_binlog_dump() {}

  virtual void operator()(THD *thd_to_kill) {
    if (thd_to_kill->get_command() == COM_BINLOG_DUMP ||
        thd_to_kill->get_command() == COM_BINLOG_DUMP_GTID) {
      DBUG_ASSERT(thd_to_kill != current_thd);
      MUTEX_LOCK(thd_data_lock, &thd_to_kill->LOCK_thd_data);
      thd_to_kill->duplicate_slave_id = true;
      thd_to_kill->awake(THD::KILL_CONNECTION);
    }
  }
};

int binlog_dump_thread_kill() {
  DBUG_TRACE;
  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  Kill_binlog_dump kill_binlog_dump;
  thd_manager->do_for_all_thd(&kill_binlog_dump);
  return 0;
}

int channel_purge_queue(const char *channel, bool reset_all) {
  DBUG_TRACE;

  channel_map.wrlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  bool thd_init = init_thread_context();

  int error = reset_slave(current_thd, mi, reset_all);

  channel_map.unlock();

  if (thd_init) {
    clean_thread_context();
  }

  return error;
}

bool channel_is_active(const char *channel,
                       enum_channel_thread_types thd_type) {
  int thread_mask = 0;
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return false;
  }

  init_thread_mask(&thread_mask, mi, false /* not inverse*/);

  channel_map.unlock();

  switch (thd_type) {
    case CHANNEL_NO_THD:
      return true;  // return true as the channel exists
    case CHANNEL_RECEIVER_THREAD:
      return thread_mask & SLAVE_IO;
    case CHANNEL_APPLIER_THREAD:
      return thread_mask & SLAVE_SQL;
    default:
      DBUG_ASSERT(0);
  }
  return false;
}

int channel_get_thread_id(const char *channel,
                          enum_channel_thread_types thd_type,
                          unsigned long **thread_id) {
  DBUG_TRACE;

  int number_threads = -1;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  switch (thd_type) {
    case CHANNEL_RECEIVER_THREAD:
      mysql_mutex_lock(&mi->info_thd_lock);
      if (mi->info_thd != nullptr) {
        *thread_id = (unsigned long *)my_malloc(
            PSI_NOT_INSTRUMENTED, sizeof(unsigned long), MYF(MY_WME));
        **thread_id = mi->info_thd->thread_id();
        number_threads = 1;
      }
      mysql_mutex_unlock(&mi->info_thd_lock);
      break;
    case CHANNEL_APPLIER_THREAD:
      if (mi->rli != nullptr) {
        mysql_mutex_lock(&mi->rli->run_lock);

        if (mi->rli->slave_parallel_workers > 0) {
          // Parallel applier.
          size_t num_workers = mi->rli->get_worker_count();
          number_threads = 1 + num_workers;
          *thread_id = (unsigned long *)my_malloc(
              PSI_NOT_INSTRUMENTED, number_threads * sizeof(unsigned long),
              MYF(MY_WME));
          unsigned long *thread_id_pointer = *thread_id;

          // Set default values on thread_id array.
          for (int i = 0; i < number_threads; i++, thread_id_pointer++)
            *thread_id_pointer = -1;
          thread_id_pointer = *thread_id;

          // Coordinator thread id.
          if (mi->rli->info_thd != nullptr) {
            mysql_mutex_lock(&mi->rli->info_thd_lock);
            *thread_id_pointer = mi->rli->info_thd->thread_id();
            mysql_mutex_unlock(&mi->rli->info_thd_lock);
            thread_id_pointer++;
          }

          // Workers thread id.
          if (mi->rli->workers_array_initialized) {
            for (size_t i = 0; i < num_workers; i++, thread_id_pointer++) {
              Slave_worker *worker = mi->rli->get_worker(i);
              if (worker != nullptr) {
                mysql_mutex_lock(&worker->jobs_lock);
                if (worker->info_thd != nullptr &&
                    worker->running_status != Slave_worker::NOT_RUNNING) {
                  mysql_mutex_lock(&worker->info_thd_lock);
                  *thread_id_pointer = worker->info_thd->thread_id();
                  mysql_mutex_unlock(&worker->info_thd_lock);
                }
                mysql_mutex_unlock(&worker->jobs_lock);
              }
            }
          }
        } else {
          // Sequential applier.
          if (mi->rli->info_thd != nullptr) {
            *thread_id = (unsigned long *)my_malloc(
                PSI_NOT_INSTRUMENTED, sizeof(unsigned long), MYF(MY_WME));
            mysql_mutex_lock(&mi->rli->info_thd_lock);
            **thread_id = mi->rli->info_thd->thread_id();
            mysql_mutex_unlock(&mi->rli->info_thd_lock);
            number_threads = 1;
          }
        }
        mysql_mutex_unlock(&mi->rli->run_lock);
      }
      break;
    default:
      break;
  }

  channel_map.unlock();

  return number_threads;
}

long long channel_get_last_delivered_gno(const char *channel, int sidno) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  rpl_gno last_gno = 0;

  Checkable_rwlock *sid_lock = mi->rli->get_sid_lock();
  sid_lock->rdlock();
  last_gno = mi->rli->get_gtid_set()->get_last_gno(sidno);
  sid_lock->unlock();

#if !defined(DBUG_OFF)
  const Gtid_set *retrieved_gtid_set = mi->rli->get_gtid_set();
  char *retrieved_gtid_set_string = nullptr;
  sid_lock->wrlock();
  retrieved_gtid_set->to_string(&retrieved_gtid_set_string);
  sid_lock->unlock();
  DBUG_PRINT("info", ("get_last_delivered_gno retrieved_set_string: %s",
                      retrieved_gtid_set_string));
  my_free(retrieved_gtid_set_string);
#endif

  channel_map.unlock();

  return last_gno;
}

int channel_add_executed_gtids_to_received_gtids(const char *channel) {
  DBUG_TRACE;

  channel_map.rdlock();
  Master_info *mi = channel_map.get_mi(channel);
  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  global_sid_lock->wrlock();

  enum_return_status return_status =
      mi->rli->add_gtid_set(gtid_state->get_executed_gtids());

  global_sid_lock->unlock();
  channel_map.unlock();

  return return_status != RETURN_STATUS_OK;
}

int channel_queue_packet(const char *channel, const char *buf,
                         unsigned long event_len) {
  int result;
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }
  channel_map.unlock();

  result = queue_event(mi, buf, event_len, false /*flush_master_info*/);

  return result;
}

int channel_wait_until_apply_queue_applied(const char *channel,
                                           double timeout) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  mi->inc_reference();
  channel_map.unlock();

  /*
    The retrieved_gtid_set (rli->get_gtid_set) has its own sid_map/sid_lock
    and do not use global_sid_map/global_sid_lock. Instead of blocking both
    sid locks on each wait iteration at rli->wait_for_gtid_set(Gtid_set), it
    would be better to use rli->wait_for_gtid_set(char *) that will create a
    new Gtid_set based on global_sid_map.
  */
  char *retrieved_gtid_set_buf;
  mi->rli->get_sid_lock()->wrlock();
  mi->rli->get_gtid_set()->to_string(&retrieved_gtid_set_buf);
  mi->rli->get_sid_lock()->unlock();

  int error = mi->rli->wait_for_gtid_set(current_thd, retrieved_gtid_set_buf,
                                         timeout, false);
  my_free(retrieved_gtid_set_buf);
  mi->dec_reference();

  if (error == -1) return REPLICATION_THREAD_WAIT_TIMEOUT_ERROR;
  if (error == -2) return REPLICATION_THREAD_WAIT_NO_INFO_ERROR;

  return error;
}

int channel_wait_until_transactions_applied(const char *channel,
                                            const char *gtid_set,
                                            double timeout,
                                            bool update_THD_status) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock(); /* purecov: inspected */
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR; /* purecov:
                                                                  inspected */
  }

  mi->inc_reference();
  channel_map.unlock();

  int error = mi->rli->wait_for_gtid_set(current_thd, gtid_set, timeout,
                                         update_THD_status);
  mi->dec_reference();

  if (error == -1) return REPLICATION_THREAD_WAIT_TIMEOUT_ERROR;
  if (error == -2) return REPLICATION_THREAD_WAIT_NO_INFO_ERROR;

  return error;
}

int channel_is_applier_waiting(const char *channel) {
  DBUG_TRACE;
  int result = RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return result;
  }

  unsigned long *thread_ids = nullptr;
  int number_appliers =
      channel_get_thread_id(channel, CHANNEL_APPLIER_THREAD, &thread_ids);

  if (number_appliers <= 0) {
    goto end;
  }

  if (number_appliers == 1) {
    result = channel_is_applier_thread_waiting(*thread_ids);
  } else if (number_appliers > 1) {
    int waiting = 0;

    // Check if coordinator is waiting.
    waiting += channel_is_applier_thread_waiting(thread_ids[0]);

    // Check if workers are waiting.
    for (int i = 1; i < number_appliers; i++)
      waiting += channel_is_applier_thread_waiting(thread_ids[i], true);

    // Check if all are waiting.
    if (waiting == number_appliers)
      result = 1;
    else
      result = 0;
  }

end:
  channel_map.unlock();
  my_free(thread_ids);

  return result;
}

int channel_is_applier_thread_waiting(unsigned long thread_id, bool worker) {
  DBUG_TRACE;
  int result = -1;

  Find_thd_with_id find_thd_with_id(thread_id);
  THD *thd = Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
  if (thd) {
    result = 0;

    const char *proc_info = thd->get_proc_info();
    if (proc_info) {
      const char *stage_name = stage_slave_has_read_all_relay_log.m_name;
      if (worker)
        stage_name = stage_slave_waiting_event_from_coordinator.m_name;

      if (!strcmp(proc_info, stage_name)) result = 1;
    }
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

  return result;
}

int channel_flush(const char *channel) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  bool error = (flush_relay_logs(mi, mi->info_thd) == 1);

  channel_map.unlock();

  return error ? 1 : 0;
}

int channel_get_retrieved_gtid_set(const char *channel, char **retrieved_set) {
  DBUG_TRACE;

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    return RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR;
  }

  mi->inc_reference();
  channel_map.unlock();

  int error = 0;
  const Gtid_set *receiver_gtid_set = mi->rli->get_gtid_set();
  if (receiver_gtid_set->to_string(retrieved_set, true /*need_lock*/) == -1)
    error = ER_OUTOFMEMORY;

  mi->dec_reference();

  return error;
}

int channel_get_credentials(const char *channel, const char **user, char **pass,
                            size_t *pass_size) {
  DBUG_ENTER("channel_get_credentials(channel,user,password, pass_size)");

  channel_map.rdlock();

  Master_info *mi = channel_map.get_mi(channel);

  if (mi == nullptr) {
    channel_map.unlock();
    DBUG_RETURN(RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR);
  }

  mi->inc_reference();
  channel_map.unlock();

  *user = mi->get_user();
  mi->get_password(*pass, pass_size);

  mi->dec_reference();

  DBUG_RETURN(0);
}

bool channel_is_stopping(const char *channel,
                         enum_channel_thread_types thd_type) {
  bool is_stopping = false;
  DBUG_TRACE;

  channel_map.rdlock();
  Master_info *mi = channel_map.get_mi(channel);
  if (mi == nullptr) {
    channel_map.unlock();
    return false;
  }

  switch (thd_type) {
    case CHANNEL_NO_THD:
      break;
    case CHANNEL_RECEIVER_THREAD:
      is_stopping = likely(mi->atomic_is_stopping);
      break;
    case CHANNEL_APPLIER_THREAD:
      is_stopping = likely(mi->rli->atomic_is_stopping);
      break;
    default:
      DBUG_ASSERT(0);
  }

  channel_map.unlock();

  return is_stopping;
}

bool is_partial_transaction_on_channel_relay_log(const char *channel) {
  DBUG_TRACE;
  channel_map.rdlock();
  Master_info *mi = channel_map.get_mi(channel);
  if (mi == nullptr) {
    channel_map.unlock();
    return false;
  }
  bool ret = mi->transaction_parser.is_inside_transaction();
  channel_map.unlock();
  return ret;
}

bool is_any_slave_channel_running(int thread_mask) {
  DBUG_TRACE;
  Master_info *mi = nullptr;
  bool is_running;

  channel_map.rdlock();

  for (mi_map::iterator it = channel_map.begin(); it != channel_map.end();
       it++) {
    mi = it->second;

    if (mi) {
      if ((thread_mask & SLAVE_IO) != 0) {
        mysql_mutex_lock(&mi->run_lock);
        is_running = mi->slave_running;
        mysql_mutex_unlock(&mi->run_lock);
        if (is_running) {
          channel_map.unlock();
          return true;
        }
      }

      if ((thread_mask & SLAVE_SQL) != 0) {
        mysql_mutex_lock(&mi->rli->run_lock);
        is_running = mi->rli->slave_running;
        mysql_mutex_unlock(&mi->rli->run_lock);
        if (is_running) {
          channel_map.unlock();
          return true;
        }
      }
    }
  }

  channel_map.unlock();
  return false;
}

enum_slave_channel_status
has_any_slave_channel_open_temp_table_or_is_its_applier_running() {
  DBUG_TRACE;
  Master_info *mi = nullptr;
  bool is_applier_running = false;
  bool has_open_temp_tables = false;
  mi_map::iterator it;

  channel_map.rdlock();

  mi_map::iterator it_end = channel_map.end();
  for (it = channel_map.begin(); it != channel_map.end(); ++it) {
    mi = it->second;

    if (Master_info::is_configured(mi)) {
      mysql_mutex_lock(&mi->rli->run_lock);
      is_applier_running = mi->rli->slave_running;
      if (mi->rli->atomic_channel_open_temp_tables > 0)
        has_open_temp_tables = true;
      if (is_applier_running || has_open_temp_tables) {
        /*
          Stop acquiring more run_locks and start to release the held
          run_locks once finding that a slave channel applier thread
          is running or a slave channel has open temporary table(s),
          and record the stop position.
        */
        it_end = ++it;
        break;
      }
    }
  }

  /*
    Release the held run_locks until the stop position recorded in above
    or the end of the channel_map.
  */
  for (it = channel_map.begin(); it != it_end; ++it) {
    mi = it->second;
    if (Master_info::is_configured(mi)) mysql_mutex_unlock(&mi->rli->run_lock);
  }

  channel_map.unlock();

  if (has_open_temp_tables)
    return SLAVE_CHANNEL_HAS_OPEN_TEMPORARY_TABLE;
  else if (is_applier_running)
    return SLAVE_CHANNEL_APPLIER_IS_RUNNING;

  return SLAVE_CHANNEL_NO_APPLIER_RUNNING_AND_NO_OPEN_TEMPORARY_TABLE;
}
