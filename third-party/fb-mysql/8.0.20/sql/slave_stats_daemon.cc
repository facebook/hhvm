/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <my_systime.h>
#include <my_time.h>
#include <mysql.h>
#include <mysql/psi/mysql_thread.h>
#include <mysql_com.h>
#include <sql_common.h>
#include <sql_string.h>

#include "sql/log.h"
#include "sql/protocol_classic.h"
#include "sql/rpl_mi.h"
#include "sql/rpl_msr.h"  // Multisource_info
#include "sql/rpl_slave.h"
#include "sql/slave_stats_daemon.h"
#include "sql/sql_base.h"
#include "sql/sql_show.h"
#include "sql/srv_session.h"

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
// Function removed after OpenSSL 1.1.0
#define ERR_remove_state(x)
#endif

/*
 * The Slave stats daemon thread is responsible for
 * continuously sending lag statistics from slaves to masters
 */

my_thread_t slave_stats_daemon_thread;
mysql_mutex_t LOCK_slave_stats_daemon;
mysql_cond_t COND_slave_stats_daemon;

/* connection/read timeout in seconds*/
const int REPLICA_STATS_NET_TIMEOUT = 5;

static bool abort_slave_stats_daemon = false;

static bool connected_to_master = false;

/**
  Create and initialize the mysql object, and connect to the
  master.

  @retval true if connection successful
  @retval false otherwise.
*/
static bool safe_connect_slave_stats_thread_to_master(MYSQL *&mysql,
                                                      Master_info *active_mi,
                                                      NET_SERVER *server_extn) {
  if (mysql != nullptr) {
    mysql_close(mysql);
  }
  mysql = mysql_init(nullptr);
  if (!mysql) {
    return false;
  }
  mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT,
                (const char *)&REPLICA_STATS_NET_TIMEOUT);
  mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT,
                (const char *)&REPLICA_STATS_NET_TIMEOUT);
  configure_master_connection_options(mysql, active_mi);

  char pass[MAX_PASSWORD_LENGTH + 1];
  size_t password_size = sizeof(pass);
  if (active_mi->get_password(pass, &password_size)) {
    return false;
  }
  mysql_extension_set_server_extn(mysql, server_extn);
  if (!mysql_real_connect(mysql, active_mi->host, active_mi->get_user(), pass,
                          0, active_mi->port, 0, 0)) {
    return false;
  }
  return true;
}

extern "C" {
static void *handle_slave_stats_daemon(void *arg MY_ATTRIBUTE((unused))) {
  THD *thd = nullptr;
  int error = 0;
  struct timespec abstime;

  DBUG_ENTER("handle_slave_stats_daemon");

  slave_stats_daemon_thread = my_thread_self();

  MYSQL *mysql = nullptr;
  // allocate server extension structure
  NET_SERVER server_extn;
  server_extn.m_user_data = nullptr;
  server_extn.m_before_header = nullptr;
  server_extn.m_after_header = nullptr;
  server_extn.compress_ctx.algorithm = MYSQL_UNCOMPRESSED;

  Master_info *active_mi;
  while (true) {
    mysql_mutex_lock(&LOCK_slave_stats_daemon);
    set_timespec(&abstime, write_stats_frequency);
    while ((!error || error == EINTR) && !abort_slave_stats_daemon) {
      /*
       write_stats_frequency is set to 0. Do not send stats to master.
       Wait until a signal is received either for aborting the thread or for
       updating write_stats_frequency.
      */
      if (write_stats_frequency == 0) {
        error =
            mysql_cond_wait(&COND_slave_stats_daemon, &LOCK_slave_stats_daemon);
      } else {
        /*
        wait for write_stats_frequency seconds before sending next set
        of slave lag statistics
        */
        error = mysql_cond_timedwait(&COND_slave_stats_daemon,
                                     &LOCK_slave_stats_daemon, &abstime);
      }
    }

    mysql_mutex_unlock(&LOCK_slave_stats_daemon);

    if (abort_slave_stats_daemon) break;

    if (error == ETIMEDOUT) {
      // Initialize connection thd, if not already done.
      if (thd == nullptr) {
        my_thread_init();
        thd = new THD;
        thd->set_new_thread_id();
        THD_CHECK_SENTRY(thd);
        thd->thread_stack = (char *)&thd;
        my_net_init(thd->get_protocol_classic()->get_net(), 0);
        thd->store_globals();
      }

      channel_map.rdlock();
      active_mi = channel_map.get_default_channel_mi();
      channel_map.unlock();
      // If not connected to current master, try connection. If not
      // successful, try again in next cycle

      if (!connected_to_master) {
        connected_to_master = safe_connect_slave_stats_thread_to_master(
            mysql, active_mi, &server_extn);
        if (connected_to_master) {
          DBUG_PRINT("info",
                     ("Slave Stats Daemon: connected to master '%s@%s:%d'",
                      active_mi->get_user(), active_mi->host, active_mi->port));
        } else {
          DBUG_PRINT(
              "info",
              ("Slave Stats Daemon: Couldn't connect to master '%s@%s:%d', "
               "will try again during next cycle, (Error: %s)",
               active_mi->get_user(), active_mi->host, active_mi->port,
               mysql_error(mysql)));
        }
      }
      if (connected_to_master &&
          active_mi->slave_running == MYSQL_SLAVE_RUN_CONNECT) {
        if (send_replica_statistics_to_master(mysql, active_mi)) {
          DBUG_PRINT("info", ("Slave Stats Daemon: Failed to send lag "
                              "statistics, resetting connection, (Error: %s)",
                              mysql_error(mysql)));
          connected_to_master = false;
        }
      }
      error = 0;
    }
  }
  mysql_close(mysql);
  mysql = nullptr;
  connected_to_master = false;
  if (thd != nullptr) {
    net_end(thd->get_protocol_classic()->get_net());
    thd->release_resources();
    delete (thd);
  }
  ERR_remove_state(0);
  DBUG_ASSERT(slave_stats_daemon_thread_counter > 0);
  slave_stats_daemon_thread_counter--;
  my_thread_end();
  return (nullptr);
}
}  // extern "C"

/* Start handle Slave Stats Daemon thread */
bool start_handle_slave_stats_daemon() {
  DBUG_ENTER("start_handle_slave_stats_daemon");

  channel_map.rdlock();
  if (channel_map.get_num_instances() != 1) {
    // more than one channels exists for this slave. We only support
    // single source slave topologies for now. Skip creating the thread.
    sql_print_information(
        "Number of channels = %lu. There should be only one channel"
        " with slave_stats_daemon. Not creating the thread.",
        channel_map.get_num_instances());
    channel_map.unlock();
    DBUG_RETURN(false);
  }
  channel_map.unlock();

  my_thread_handle thread_handle;
  slave_stats_daemon_thread_counter++;
  int error =
      mysql_thread_create(key_thread_handle_slave_stats_daemon, &thread_handle,
                          &connection_attrib, handle_slave_stats_daemon, 0);
  if (error) {
    sql_print_warning("Can't create Slave Stats Daemon thread (errno= %d)",
                      error);
    DBUG_ASSERT(slave_stats_daemon_thread_counter > 0);
    slave_stats_daemon_thread_counter--;
    DBUG_RETURN(false);
  }
  sql_print_information("Successfully created Slave Stats Daemon thread: 0x%lx",
                        (ulong)slave_stats_daemon_thread);
  DBUG_RETURN(true);
}

/* Initiate shutdown of handle Slave Stats Daemon thread */
void stop_handle_slave_stats_daemon() {
  DBUG_ENTER("stop_handle_slave_stats_daemon");
  abort_slave_stats_daemon = true;
  mysql_mutex_lock(&LOCK_slave_stats_daemon);
  sql_print_information("Shutting down Slave Stats Daemon thread: 0x%lx",
                        (ulong)slave_stats_daemon_thread);
  // there must be at most one slave_stats_daemon thread to stop
  DBUG_ASSERT(slave_stats_daemon_thread_counter <= 1);
  mysql_cond_broadcast(&COND_slave_stats_daemon);
  mysql_mutex_unlock(&LOCK_slave_stats_daemon);
  while (slave_stats_daemon_thread_counter > 0) {
    // wait for the thread to finish, sleep for 10ms
    my_sleep(10000);
  }
  // Reset abort_slave_stats_daemon so slave_stats_daemon can be spawned in
  // future
  abort_slave_stats_daemon = false;
  DBUG_VOID_RETURN;
}
