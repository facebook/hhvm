/*
  Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef THREAD_POOL_PRIV_INCLUDED
#define THREAD_POOL_PRIV_INCLUDED

/**
  @file include/mysql/thread_pool_priv.h
  All accesses to THD variables and functions are defined in this header file.
*/
#include <mysqld_error.h> /* To get ER_ERROR_ON_READ */

#include "sql/conn_handler/channel_info.h"
#include "sql/conn_handler/connection_handler_manager.h"

/**
  Called by the server when a new client connects.

  @param channel_info  Pointer to object containing information
                       about the new connection.

  @retval true  failure
  @retval false success
*/
typedef bool (*add_connection_t)(Channel_info *channel_info);

/**
  Called by the server when the connection handler is destroyed.
*/
typedef void (*end_t)(void);

/**
   This structure must be populated by plugins which implement connection
   handlers and passed as an argument to my_connection_handler_set() in
   order to activate the connection handler.

   The structure contains pointers to plugin functions which the server
   will call when a new client connects or when the connection handler is
   unloaded. It also contains the maximum number of threads the connection
   handler will create.
*/
struct Connection_handler_functions {
  /**
     The maximum number of threads this connection handler will create.
  */
  uint max_threads;

  add_connection_t add_connection;
  end_t end;
};

/* create thd from channel_info object */
THD *create_thd(Channel_info *channel_info);
/* destroy channel_info object */
void destroy_channel_info(Channel_info *channel_info);
/* Decrement connection counter */
void dec_connection_count();
/*
  thread_created is maintained by thread pool when activated since
  user threads are created by the thread pool (and also special
  threads to maintain the thread pool). This is done through
  inc_thread_created.
*/
void inc_thread_created();

void thd_lock_thread_count();
void thd_unlock_thread_count();

/*
  Interface to global thread list iterator functions.
  Executes a function with signature 'void f(THD*, uint64)' for all THDs.
*/
typedef void(do_thd_impl_uint64)(THD *, uint64);
void do_for_all_thd(do_thd_impl_uint64, uint64);

/* Needed to get access to scheduler variables */
void *thd_get_scheduler_data(THD *thd);
void thd_set_scheduler_data(THD *thd, void *data);
PSI_thread *thd_get_psi(THD *thd);
void thd_set_psi(THD *thd, PSI_thread *psi);

/* Interface to THD variables and functions */
void thd_set_killed(THD *thd);
void thd_clear_errors(THD *thd);
void thd_close_connection(THD *thd);
THD *thd_get_current_thd();
void thd_lock_data(THD *thd);
void thd_unlock_data(THD *thd);
bool thd_is_transaction_active(THD *thd);
int thd_connection_has_data(THD *thd);
void thd_set_net_read_write(THD *thd, uint val);
uint thd_get_net_read_write(THD *thd);
void thd_set_not_killable(THD *thd);
ulong thd_get_net_wait_timeout(THD *thd);
my_socket thd_get_fd(THD *thd);
void thd_store_globals(THD *thd);

/*
  The thread pool must be able to execute statements using the connection
  state in THD object. This is the main objective of the thread pool to
  schedule the start of these commands.
*/
bool do_command(THD *thd);

/*
  The thread pool requires an interface to the connection logic in the
  MySQL Server since the thread pool will maintain the event logic on
  the TCP connection of the MySQL Server. Thus new connections, dropped
  connections will be discovered by the thread pool and it needs to
  ensure that the proper MySQL Server logic attached to these events is
  executed.
*/
/* Prepare connection as part of connection set-up */
bool thd_prepare_connection(THD *thd);
/* Release auditing before executing statement */
void mysql_audit_release(THD *thd);
/* Check if connection is still alive */
bool thd_connection_alive(THD *thd);
/* Close connection with possible error code */
void close_connection(THD *thd, uint sql_errno, bool server_shutdown,
                      bool generate_event);
/* End the connection before closing it */
void end_connection(THD *thd);
/* Reset thread globals */
void reset_thread_globals(THD *thd);

/*
  max_connections is needed to calculate the maximum number of threads
  that is allowed to be started by the thread pool. The method
  get_max_connections() gets reference to this variable.
*/
ulong get_max_connections(void);
/*
  connection_attrib is the thread attributes for connection threads,
  the method get_connection_attrib provides a reference to these
  attributes.
*/
my_thread_attr_t *get_connection_attrib(void);

/* Increment the status variable 'Aborted_connects'. */
void increment_aborted_connects();

/**
  Set current PSI thread.
*/
void psi_set_thread(PSI_thread *psi MY_ATTRIBUTE((unused)));

/**
  Set current PSI thread as owner of THD socket.
  @param thd THD of socket to set owner on.
*/
void psi_set_socket_thread_owner(THD *thd);

/**
  Send connection timeout error.

  @param thd THD of timed out connection.
*/
void net_send_conn_timeout_error(THD *thd);

#endif  // THREAD_POOL_PRIV_INCLUDED
