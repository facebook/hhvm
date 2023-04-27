/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/clone_handler.h"

#include <string.h>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/plugin.h"
#include "mysql/plugin_clone.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#include "sql/dd/upgrade_57/upgrade.h"  // dd::upgrade_57::in_progress
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_parse.h"
#include "sql/sql_plugin.h"  // plugin_unlock
#include "sql/srv_session.h"
#include "sql_string.h"  // to_lex_cstring
#include "violite.h"

class THD;

/* To get current session thread default THD */
THD *thd_get_current_thd();

/** Clone handler global */
Clone_handler *clone_handle = nullptr;

/** Clone plugin name */
const char *clone_plugin_nm = "clone";

bool Clone_handler::get_donor_error(Srv_session *session, int &error,
                                    const char *&message) {
  error = 0;
  message = nullptr;
  THD *thd = nullptr;

  if (session != nullptr) {
    thd = session->get_thd();
  }

  /* Check if THD exists. */
  if (thd == nullptr) {
    /* Try to get current THD handle. */
    thd = thd_get_current_thd();
    if (thd == nullptr) {
      return (false);
    }
  }

  /* Check if DA exists. */
  auto da = thd->get_stmt_da();
  if (da == nullptr || !da->is_error()) {
    return (false);
  }

  if (da->mysql_errno() != ER_CLONE_DONOR) {
    return (false);
  }

  /* Assign current error from DA */
  error = da->mysql_errno();
  message = da->message_text();

  /* Parse and find out donor error and message. */
  size_t err_pos = 0;
  std::string msg_string(message);

  while (!std::isdigit(message[err_pos])) {
    /* Find position of next ":". */
    err_pos = msg_string.find(": ", err_pos);

    /* No more separator, return. */
    if (err_pos == std::string::npos) {
      DBUG_ASSERT(false);
      return (false);
    }
    /* Skip ":" and space. */
    err_pos += 2;
  }

  error = std::atoi(message + err_pos);

  if (error != 0) {
    err_pos = msg_string.find(": ", err_pos);
    /* Should find the error message following the error code. */
    if (err_pos == std::string::npos) {
      DBUG_ASSERT(false);
      return (false);
    }
    /* Skip ":" and space. */
    err_pos += 2;
    message = message + err_pos;
  }
  return (true);
}

int Clone_handler::clone_local(THD *thd, const char *data_dir) {
  int error;
  char dir_name[FN_REFLEN];

  error = validate_dir(data_dir, dir_name);

  if (error == 0) {
    error = m_plugin_handle->clone_local(thd, dir_name);
  }

  return error;
}

int Clone_handler::clone_remote_client(THD *thd, const char *remote_host,
                                       uint remote_port,
                                       const char *remote_user,
                                       const char *remote_passwd,
                                       const char *data_dir,
                                       enum mysql_ssl_mode ssl_mode) {
  int error = 0;
  char dir_name[FN_REFLEN];
  char *dir_ptr = nullptr;

  /* NULL when clone replaces current data directory. */
  if (data_dir != nullptr) {
    error = validate_dir(data_dir, dir_name);
    dir_ptr = &dir_name[0];
  }

  if (error != 0) {
    return error;
  }

  /* NULL data directory implies we are replacing current data directory
  for provisioning this node. We never set it back to false only in case
  of error otherwise the server would shutdown or restart at the end of
  operation. */
  bool provisioning = (data_dir == nullptr);
  if (provisioning) {
    ++s_provision_in_progress;
  }

  int mode = static_cast<int>(ssl_mode);

  error = m_plugin_handle->clone_client(
      thd, remote_host, remote_port, remote_user, remote_passwd, dir_ptr, mode);

  if (error != 0 && provisioning) {
    --s_provision_in_progress;
  }
  return error;
}

int Clone_handler::clone_remote_server(THD *thd, MYSQL_SOCKET socket) {
  auto err = m_plugin_handle->clone_server(thd, socket);
  return err;
}

int Clone_handler::init() {
  /* Don't allow loading clone plugin during upgrade. */
  if (dd::upgrade_57::in_progress()) {
    LogErr(ERROR_LEVEL, ER_PLUGIN_INSTALL_ERROR, "clone",
           "Cannot install during upgrade.");
    return 1;
  }

  plugin_ref plugin;

  plugin = my_plugin_lock_by_name(
      nullptr, to_lex_cstring(m_plugin_name.c_str()), MYSQL_CLONE_PLUGIN);
  if (plugin == nullptr) {
    m_plugin_handle = nullptr;
    LogErr(ERROR_LEVEL, ER_CLONE_PLUGIN_NOT_LOADED_TRACE);
    return 1;
  }

  m_plugin_handle = (Mysql_clone *)plugin_decl(plugin)->info;
  plugin_unlock(nullptr, plugin);

  if (opt_initialize) {
    /* Inform that database initialization in progress. */
    return ER_INIT_BOOTSTRAP_COMPLETE;
  }
  return 0;
}

int Clone_handler::validate_dir(const char *in_dir, char *out_dir) {
  MY_STAT stat_info;

  /* Verify that it is absolute path. */
  if (test_if_hard_path(in_dir) == 0) {
    my_error(ER_WRONG_VALUE, MYF(0), "path", in_dir);
    return ER_WRONG_VALUE;
  }

  /* Verify that the length is not too long. */
  if (strlen(in_dir) >= FN_REFLEN - 1) {
    my_error(ER_PATH_LENGTH, MYF(0), "DATA DIRECTORY");
    return ER_PATH_LENGTH;
  }

  /* Convert the path to native os format. */
  convert_dirname(out_dir, in_dir, nullptr);

  /* Check if the data directory exists already. */
  if (mysql_file_stat(key_file_misc, out_dir, &stat_info, MYF(0)) != nullptr) {
    my_error(ER_DB_CREATE_EXISTS, MYF(0), in_dir);
    return ER_DB_CREATE_EXISTS;
  }

  /* Check if path is within current data directory */
  char tmp_dir[FN_REFLEN + 1];
  size_t length;

  strncpy(tmp_dir, out_dir, FN_REFLEN);
  length = strlen(out_dir);

  /* Loop and remove all non-existent directories from the tail */
  while (length != 0) {
    /* Check if directory exists. */
    if (mysql_file_stat(key_file_misc, tmp_dir, &stat_info, MYF(0)) !=
        nullptr) {
      /* Check if the path is not within data directory. */
      if (test_if_data_home_dir(tmp_dir)) {
        my_error(ER_PATH_IN_DATADIR, MYF(0), in_dir);
        return ER_PATH_IN_DATADIR;
      }

      break;
    }

    size_t new_length;

    /* Remove the last directory separator from string */
    tmp_dir[length - 1] = '\0';
    dirname_part(tmp_dir, tmp_dir, &new_length);

    /* length must always decrease for the loop to terminate */
    if (length <= new_length) {
      DBUG_ASSERT(false);
      break;
    }

    length = new_length;
  }

  return 0;
}

int clone_handle_create(const char *plugin_name) {
  if (clone_handle != nullptr) {
    LogErr(ERROR_LEVEL, ER_CLONE_HANDLER_EXIST_TRACE);
    return 1;
  }

  clone_handle = new Clone_handler(plugin_name);

  if (clone_handle == nullptr) {
    LogErr(ERROR_LEVEL, ER_CLONE_CREATE_HANDLER_FAIL_TRACE);
    return 1;
  }

  return clone_handle->init();
}

int clone_handle_drop() {
  if (clone_handle == nullptr) {
    return 1;
  }

  delete clone_handle;

  clone_handle = nullptr;

  if (opt_initialize) {
    /* Inform that database initialization in progress. */
    return ER_INIT_BOOTSTRAP_COMPLETE;
  }
  return 0;
}

Clone_handler *clone_plugin_lock(THD *thd, plugin_ref *plugin) {
  *plugin = my_plugin_lock_by_name(thd, to_lex_cstring(clone_plugin_nm),
                                   MYSQL_CLONE_PLUGIN);

  mysql_mutex_lock(&LOCK_plugin);

  /* Return handler only if the plugin is ready. We might successfully
  lock the plugin when initialization is progress. */
  if (*plugin != nullptr && plugin_state(*plugin) == PLUGIN_IS_READY) {
    mysql_mutex_unlock(&LOCK_plugin);

    DBUG_ASSERT(clone_handle != nullptr);
    return clone_handle;
  }

  mysql_mutex_unlock(&LOCK_plugin);

  return nullptr;
}

void clone_plugin_unlock(THD *thd, plugin_ref plugin) {
  plugin_unlock(thd, plugin);
}

std::atomic<int> Clone_handler::s_xa_counter{0};
std::atomic<bool> Clone_handler::s_xa_block_op{false};
std::atomic<int> Clone_handler::s_provision_in_progress{0};
std::atomic<bool> Clone_handler::s_is_data_dropped{false};

mysql_mutex_t Clone_handler::s_xa_mutex;

void Clone_handler::init_xa() {
  s_xa_block_op.store(false);
  s_xa_counter.store(0);
  s_provision_in_progress.store(0);
  mysql_mutex_init(PSI_NOT_INSTRUMENTED, &s_xa_mutex, MY_MUTEX_INIT_FAST);
}

void Clone_handler::uninit_xa() { mysql_mutex_destroy(&s_xa_mutex); }

Clone_handler::XA_Operation::XA_Operation(THD *thd) : m_thd(thd) {
  begin_xa_operation(thd);
  DEBUG_SYNC_C("xa_block_clone");
  /* Allow GTID to be read by SE for XA Commit. */
  thd->pin_gtid();
}

Clone_handler::XA_Operation::~XA_Operation() {
  m_thd->unpin_gtid();
  end_xa_operation();
}

void Clone_handler::begin_xa_operation(THD *thd) {
  /* Quick sequential consistent check and return if not blocked by clone. This
  ensures minimum impact during normal operations. */
  s_xa_counter.fetch_add(1);
  if (!s_xa_block_op.load()) {
    return;
  }
  /* Clone has requested to block xa operation. Synchronize with mutex. */
  s_xa_counter.fetch_sub(1);
  mysql_mutex_lock(&s_xa_mutex);

  int count = 0;
  while (s_xa_block_op.load()) {
    /* Release mutex and sleep. We don't bother about starvation here as Clone
    is not a frequent operation. */
    mysql_mutex_unlock(&s_xa_mutex);
    std::chrono::milliseconds sleep_time(10);
    std::this_thread::sleep_for(sleep_time);
    mysql_mutex_lock(&s_xa_mutex);

#if defined(ENABLED_DEBUG_SYNC)
    if (count == 0) {
      DEBUG_SYNC_C("xa_wait_clone");
    }
#endif

    /* Timeout in 60 seconds */
    if (++count > 60 * 100) {
      LogErr(ERROR_LEVEL, ER_CLONE_CLIENT_TRACE,
             "Clone blocked XA operation too long: 1 minute(Timeout)");
      break;
    }
    /* Check for kill/shutdown. */
    if (thd_killed(thd)) {
      break;
    }
  }
  s_xa_counter.fetch_add(1);
  mysql_mutex_unlock(&s_xa_mutex);
}

void Clone_handler::end_xa_operation() { s_xa_counter.fetch_sub(1); }

Clone_handler::XA_Block::XA_Block(THD *thd) {
  m_success = block_xa_operation(thd);
  DEBUG_SYNC_C("clone_block_xa");
}

Clone_handler::XA_Block::~XA_Block() { unblock_xa_operation(); }

bool Clone_handler::XA_Block::failed() const { return (!m_success); }

bool Clone_handler::block_xa_operation(THD *thd) {
  bool ret = true;
  DBUG_ASSERT(!s_xa_block_op.load());

  mysql_mutex_lock(&s_xa_mutex);
  /* Block new xa prepare/commit/rollback. No new XA operation can start after
  this point and existing operations will eventually finish. */
  s_xa_block_op.store(true);

  int count = 0;
  /* Wait for existing XA operations to complete. */
  while (s_xa_counter.load() != 0) {
    /* Release mutex and sleep. */
    mysql_mutex_unlock(&s_xa_mutex);
    std::chrono::milliseconds sleep_time(10);
    std::this_thread::sleep_for(sleep_time);
    mysql_mutex_lock(&s_xa_mutex);

#if defined(ENABLED_DEBUG_SYNC)
    if (count == 0) {
      DEBUG_SYNC_C("clone_wait_xa");
    }
#endif

    /* Timeout in 60 seconds */
    if (++count > 60 * 100) {
      LogErr(INFORMATION_LEVEL, ER_CLONE_CLIENT_TRACE,
             "Clone wait for XA operation too long: 1 minute(Timeout)");
      ret = false;
      break;
    }
    /* Check for kill/shutdown. */
    if (thd_killed(thd)) {
      ret = false;
      break;
    }
  }
  mysql_mutex_unlock(&s_xa_mutex);
  return (ret);
}

void Clone_handler::unblock_xa_operation() { s_xa_block_op.store(false); }
