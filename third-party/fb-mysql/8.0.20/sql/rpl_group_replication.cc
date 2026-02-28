/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_group_replication.h"

#include <stdlib.h>
#include <sys/types.h>

#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin.h"
#include "mysql/plugin_group_replication.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"       // ER_*
#include "sql/clone_handler.h"  // is_data_dropped
#include "sql/log.h"
#include "sql/log_event.h"           // MAX_MAX_ALLOWED_PACKET
#include "sql/mysqld.h"              // mysqld_port
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/replication.h"         // Trans_context_info
#include "sql/rpl_channel_service_interface.h"
#include "sql/rpl_gtid.h"    // gtid_mode_lock
#include "sql/rpl_slave.h"   // report_host
#include "sql/sql_plugin.h"  // plugin_unlock
#include "sql/sql_plugin_ref.h"
#include "sql/ssl_acceptor_context.h"
#include "sql/system_variables.h"  // System_variables

class THD;

extern ulong opt_mi_repository_id;
extern ulong opt_rli_repository_id;

/*
  Struct to share server ssl variables
*/
void st_server_ssl_variables::init() {
  ssl_ca = nullptr;
  ssl_capath = nullptr;
  tls_version = nullptr;
  tls_ciphersuites = nullptr;
  ssl_cert = nullptr;
  ssl_cipher = nullptr;
  ssl_key = nullptr;
  ssl_crl = nullptr;
  ssl_crlpath = nullptr;
  ssl_fips_mode = 0;
}

void st_server_ssl_variables::deinit() {
  my_free(ssl_ca);
  my_free(ssl_capath);
  my_free(tls_version);
  my_free(tls_ciphersuites);
  my_free(ssl_cert);
  my_free(ssl_cipher);
  my_free(ssl_key);
  my_free(ssl_crl);
  my_free(ssl_crlpath);
  init();
}

namespace {
/**
  Static name of Group Replication plugin.
*/
LEX_CSTRING group_replication_plugin_name_str = {
    STRING_WITH_LEN("group_replication")};
}  // namespace

/*
  Group Replication plugin handler function accessors.
*/
int group_replication_init() { return initialize_channel_service_interface(); }

bool is_group_replication_plugin_loaded() {
  bool result = false;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    plugin_unlock(nullptr, plugin);
    result = true;
  }

  return result;
}

int group_replication_start(char **error_message) {
  int result = 1;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    /*
      We need to take global_sid_lock because
      group_replication_handler->start function will (among other
      things) do the following:

       1. Call get_server_startup_prerequirements, which calls get_gtid_mode.
       2. Set plugin-internal state that ensures that
          is_group_replication_running() returns true.

      In order to prevent a concurrent client from executing SET
      GTID_MODE=ON_PERMISSIVE between 1 and 2, we must hold
      gtid_mode_lock.
    */
    gtid_mode_lock->rdlock();
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->start(error_message);
    gtid_mode_lock->unlock();

    plugin_unlock(nullptr, plugin);
  } else {
    LogErr(ERROR_LEVEL, ER_GROUP_REPLICATION_PLUGIN_NOT_INSTALLED);
  }

  return result;
}

int group_replication_stop(char **error_message) {
  int result = 1;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->stop(error_message);
    plugin_unlock(nullptr, plugin);
  } else {
    LogErr(ERROR_LEVEL, ER_GROUP_REPLICATION_PLUGIN_NOT_INSTALLED);
  }

  return result;
}

bool is_group_replication_running() {
  bool result = false;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->is_running();
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

bool is_group_replication_cloning() {
  bool result = false;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->is_cloning();
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

int set_group_replication_retrieved_certification_info(
    View_change_log_event *view_change_event) {
  int result = 1;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->set_retrieved_certification_info(view_change_event);
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

bool get_group_replication_connection_status_info(
    const GROUP_REPLICATION_CONNECTION_STATUS_CALLBACKS &callbacks) {
  bool result = true;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->get_connection_status_info(callbacks);
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

bool get_group_replication_group_members_info(
    unsigned int index,
    const GROUP_REPLICATION_GROUP_MEMBERS_CALLBACKS &callbacks) {
  bool result = true;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->get_group_members_info(index, callbacks);
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

bool get_group_replication_group_member_stats_info(
    unsigned int index,
    const GROUP_REPLICATION_GROUP_MEMBER_STATS_CALLBACKS &callbacks) {
  bool result = true;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->get_group_member_stats_info(index, callbacks);
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

unsigned int get_group_replication_members_number_info() {
  unsigned int result = 0;

  plugin_ref plugin =
      my_plugin_lock_by_name(nullptr, group_replication_plugin_name_str,
                             MYSQL_GROUP_REPLICATION_PLUGIN);
  if (plugin != nullptr) {
    st_mysql_group_replication *plugin_handle =
        (st_mysql_group_replication *)plugin_decl(plugin)->info;
    result = plugin_handle->get_members_number_info();
    plugin_unlock(nullptr, plugin);
  }

  return result;
}

/** helper function to @ref get_server_parameters */
inline char *my_strdup_nullable(OptionalString from) {
  return from.c_str() == nullptr
             ? nullptr
             : my_strdup(PSI_INSTRUMENT_ME, from.c_str(), MYF(0));
}

/*
  Server methods exported to plugin through
  include/mysql/group_replication_priv.h
*/
void get_server_parameters(char **hostname, uint *port, char **uuid,
                           unsigned int *out_server_version) {
  /*
    use startup option report-host and report-port when provided,
    as value provided by glob_hostname, which used gethostname() function
    internally to determine hostname, will not always provide correct
    network interface, especially in case of multiple network interfaces.
  */
  if (report_host)
    *hostname = report_host;
  else
    *hostname = glob_hostname;

  if (report_port)
    *port = report_port;
  else
    *port = mysqld_port;

  *uuid = server_uuid;

  // Convert server version to hex

  ulong major = 0, minor = 0, patch = 0;
  char *pos = server_version, *end_pos;
  // extract each server decimal number, e.g., for 5.9.30 -> 5, 9 and 30
  major = strtoul(pos, &end_pos, 10);
  pos = end_pos + 1;
  minor = strtoul(pos, &end_pos, 10);
  pos = end_pos + 1;
  patch = strtoul(pos, &end_pos, 10);

  /*
    Convert to a equivalent hex representation.
    5.9.30 -> 0x050930
    version= 0 x 16^5 + 5 x 16^4 + 0 x 16^3 + 9 x 16^2 + 3 x 16^1 + 0 x 16^0
  */
  int v1 = patch / 10;
  int v0 = patch - v1 * 10;
  int v3 = minor / 10;
  int v2 = minor - v3 * 10;
  int v5 = major / 10;
  int v4 = major - v5 * 10;

  *out_server_version =
      v0 + v1 * 16 + v2 * 256 + v3 * 4096 + v4 * 65536 + v5 * 1048576;

  return;
}

void get_server_ssl_parameters(st_server_ssl_variables *server_ssl_variables) {
  OptionalString ca, capath, cert, cipher, ciphersuites, key, crl, crlpath,
      version;

  SslAcceptorContext::read_parameters(&ca, &capath, &version, &cert, &cipher,
                                      &ciphersuites, &key, &crl, &crlpath);

  server_ssl_variables->ssl_ca = my_strdup_nullable(ca);
  server_ssl_variables->ssl_capath = my_strdup_nullable(capath);
  server_ssl_variables->tls_version = my_strdup_nullable(version);
  server_ssl_variables->tls_ciphersuites = my_strdup_nullable(ciphersuites);
  server_ssl_variables->ssl_cert = my_strdup_nullable(cert);
  server_ssl_variables->ssl_cipher = my_strdup_nullable(cipher);
  server_ssl_variables->ssl_key = my_strdup_nullable(key);
  server_ssl_variables->ssl_crl = my_strdup_nullable(crl);
  server_ssl_variables->ssl_crlpath = my_strdup_nullable(crlpath);
  server_ssl_variables->ssl_fips_mode = opt_ssl_fips_mode;

  return;
}

ulong get_server_id() { return server_id; }

ulong get_auto_increment_increment() {
  return global_system_variables.auto_increment_increment;
}

ulong get_auto_increment_offset() {
  return global_system_variables.auto_increment_offset;
}

void set_auto_increment_increment(ulong auto_increment_increment) {
  global_system_variables.auto_increment_increment = auto_increment_increment;
}

void set_auto_increment_offset(ulong auto_increment_offset) {
  global_system_variables.auto_increment_offset = auto_increment_offset;
}

void get_server_startup_prerequirements(Trans_context_info &requirements,
                                        bool has_lock) {
  requirements.binlog_enabled = opt_bin_log;
  requirements.binlog_format = global_system_variables.binlog_format;
  requirements.binlog_checksum_options = binlog_checksum_options;
  requirements.gtid_mode =
      get_gtid_mode(has_lock ? GTID_MODE_LOCK_GTID_MODE : GTID_MODE_LOCK_NONE);
  requirements.log_slave_updates = opt_log_slave_updates;
  requirements.transaction_write_set_extraction =
      global_system_variables.transaction_write_set_extraction;
  requirements.mi_repository_type = opt_mi_repository_id;
  requirements.rli_repository_type = opt_rli_repository_id;
  requirements.parallel_applier_type = get_mts_parallel_option();
  requirements.parallel_applier_workers = opt_mts_slave_parallel_workers;
  requirements.parallel_applier_preserve_commit_order =
      get_slave_preserve_commit_order();
  requirements.lower_case_table_names = lower_case_table_names;
  requirements.default_table_encryption =
      global_system_variables.default_table_encryption;
}

bool get_server_encoded_gtid_executed(uchar **encoded_gtid_executed,
                                      size_t *length) {
  global_sid_lock->wrlock();

  DBUG_ASSERT(get_gtid_mode(GTID_MODE_LOCK_SID) > 0);

  const Gtid_set *executed_gtids = gtid_state->get_executed_gtids();
  *length = executed_gtids->get_encoded_length();
  *encoded_gtid_executed =
      (uchar *)my_malloc(key_memory_Gtid_set_to_string, *length, MYF(MY_WME));
  if (*encoded_gtid_executed == nullptr) {
    global_sid_lock->unlock();
    return true;
  }

  executed_gtids->encode(*encoded_gtid_executed);
  global_sid_lock->unlock();
  return false;
}

#if !defined(DBUG_OFF)
char *encoded_gtid_set_to_string(uchar *encoded_gtid_set, size_t length) {
  /* No sid_lock because this is a completely local object. */
  Sid_map sid_map(nullptr);
  Gtid_set set(&sid_map);

  if (set.add_gtid_encoding(encoded_gtid_set, length) != RETURN_STATUS_OK)
    return nullptr;

  char *buf;
  set.to_string(&buf);
  return buf;
}
#endif

void global_thd_manager_add_thd(THD *thd) {
  Global_THD_manager::get_instance()->add_thd(thd);
}

void global_thd_manager_remove_thd(THD *thd) {
  Global_THD_manager::get_instance()->remove_thd(thd);
}

bool is_gtid_committed(const Gtid &gtid) {
  bool result = false;
  global_sid_lock->rdlock();

  DBUG_ASSERT(get_gtid_mode(GTID_MODE_LOCK_SID) > 0);

  result = gtid_state->is_executed(gtid);

  global_sid_lock->unlock();
  return result;
}

unsigned long get_slave_max_allowed_packet() {
  return slave_max_allowed_packet;
}

unsigned long get_max_slave_max_allowed_packet() {
  return MAX_MAX_ALLOWED_PACKET;
}

bool is_server_restarting_after_clone() { return clone_startup; }

bool is_server_data_dropped() { return Clone_handler::is_data_dropped(); }
