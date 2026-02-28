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

/**
@file clone_handler.h
Clone handler interface to access clone plugin
*/

#ifndef CLONE_HANDLER_INCLUDED
#define CLONE_HANDLER_INCLUDED

#include <atomic>
#include <string>

#include "my_io.h"
#include "mysql.h"
#include "sql/sql_plugin_ref.h"  // plugin_ref

class THD;
class Srv_session;
struct Mysql_clone;
struct MYSQL_SOCKET;

/**
  Number of PSI_statement_info instruments
  for clone statements.
*/

#define CLONE_PSI_STATEMENT_COUNT 5

/**
  Clone plugin handler to convenient way to. Takes
*/
class Clone_handler {
 public:
  /** Constructor: Initialize plugin name */
  Clone_handler(const char *plugin_name_arg) : m_plugin_handle(nullptr) {
    m_plugin_name.assign(plugin_name_arg);
  }

  /** Initialize plugin handle
  @return error code */
  int init();

  /** Clone handler interface for local clone.
  @param[in]	thd		server thread handle
  @param[in]	data_dir	cloned data directory
  @return error code */
  int clone_local(THD *thd, const char *data_dir);

  /** Clone handler interface for remote clone client.
  @param[in]	thd		server thread handle
  @param[in]	remote_host	remote host IP address
  @param[in]	remote_port	remote server port
  @param[in]	remote_user	remote user name
  @param[in]	remote_passwd	remote user's password
  @param[in]	data_dir	cloned data directory
  @param[in]	ssl_mode	remote connection ssl mode
  @return error code */
  int clone_remote_client(THD *thd, const char *remote_host, uint remote_port,
                          const char *remote_user, const char *remote_passwd,
                          const char *data_dir, enum mysql_ssl_mode ssl_mode);

  /** Clone handler interface for remote clone server.
  @param[in]	thd	server thread handle
  @param[in]	socket	network socket to remote client
  @return error code */
  int clone_remote_server(THD *thd, MYSQL_SOCKET socket);

  /** Get donor error and message for ER_CLONE_DONOR error.
  @param[in]	session	server session
  @param[out]	error	donor error number
  @param[out]	message	error message
  @return true, iff successful. */
  static bool get_donor_error(Srv_session *session, int &error,
                              const char *&message);

  /** @return false only if no user data is dropped yet. */
  static bool is_data_dropped() { return (s_is_data_dropped); }

  /** Must set before dropping any user data. */
  static void set_drop_data() { s_is_data_dropped.store(true); }

  /** @return true, if clone provisioning in progress. */
  static bool is_provisioning() { return (s_provision_in_progress > 0); }

  /** @return true, if ordered commit should be forced. Currently
  clone would force ordered commit at end while blocking XA operations */
  static bool need_commit_order() { return (s_xa_block_op.load()); }

  /* Initialize XA counters and mutex. */
  static void init_xa();

  /* Destroy XA mutex. */
  static void uninit_xa();

  /* RAII guard for XA operation synchronization with clone. */
  struct XA_Operation {
    /** Constructor: mark XA operation begin.
    @param[in]	thd	session thread */
    explicit XA_Operation(THD *thd);

    /** Destructor: mark XA operation end. */
    ~XA_Operation();

    /** Disable copy construction */
    XA_Operation(XA_Operation const &) = delete;

    /** Disable assignment */
    XA_Operation &operator=(XA_Operation const &) = delete;

   private:
    /** Session thread holding the guard. */
    THD *m_thd;
  };

  /* RAII guard for blocking and unblocking XA operations. */
  struct XA_Block {
    /** Constructor: Block all XA operations.
    @param[in]	thd	session thread */
    explicit XA_Block(THD *thd);

    /** Destructor: unblock XA operations. */
    ~XA_Block();

    /** Disable copy construction */
    XA_Block(XA_Block const &) = delete;

    /** Disable assignment */
    XA_Block &operator=(XA_Block const &) = delete;

    /** @return true, if XA blocking is unsuccessful. */
    bool failed() const;

   private:
    /** If blocking is successful and there is no XA operations. */
    bool m_success;
  };

 private:
  /** Block new active XA operations and wait for existing ones to complete.
  @param[in]	thd	session thread */
  static bool block_xa_operation(THD *thd);

  /** Unblock waiting XA operations. */
  static void unblock_xa_operation();

  /** Increment XA operation count and wait if blocked by clone.
  @param[in]	thd	session thread */
  static void begin_xa_operation(THD *thd);

  /** Decrement XA operation count. */
  static void end_xa_operation();

  /** Validate clone data directory and convert to os format
  @param[in]	in_dir	user specified clone directory
  @param[out]	out_dir	data directory in native os format
  @return error code */
  int validate_dir(const char *in_dir, char *out_dir);

 private:
  /** Number of XA operations (prepare/commit/rollback) in progress. */
  static std::atomic<int> s_xa_counter;

  /** Set when clone blocks XA operations. XA operations currently are
  not ordered between binlog and SE and needs to be synchronized for clone. */
  static std::atomic<bool> s_xa_block_op;

  /** True if clone provisioning in progress. */
  static std::atomic<int> s_provision_in_progress;

  /** True, if any user data is dropped by clone. */
  static std::atomic<bool> s_is_data_dropped;

  /** Mutex to synchronize blocking XA operation. */
  static mysql_mutex_t s_xa_mutex;

  /** Clone plugin name */
  std::string m_plugin_name;

  /** Clone plugin handle */
  Mysql_clone *m_plugin_handle;
};

/** Check if the clone plugin is installed and lock. If the plugin is ready,
return the handler to caller.
@param[in]	thd	server thread handle
@param[out]	plugin	plugin reference
@return clone handler on success otherwise NULL */
Clone_handler *clone_plugin_lock(THD *thd, plugin_ref *plugin);

/** Unlock the clone plugin.
@param[in]	thd	server thread handle
@param[out]	plugin	plugin reference */
void clone_plugin_unlock(THD *thd, plugin_ref plugin);

#endif /* CLONE_HANDLER_INCLUDED */
