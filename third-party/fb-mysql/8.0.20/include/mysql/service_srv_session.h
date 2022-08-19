/*  Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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
#ifndef MYSQL_SRV_SESSION_SERVICE_INCLUDED
#define MYSQL_SRV_SESSION_SERVICE_INCLUDED

/**
  @file include/mysql/service_srv_session.h
  Header file for the Server session service. This service is to provide
  of creating sessions with the server. These sessions can be furtherly used
  together with the Command service to execute commands in the server.
*/

#include "mysql/service_srv_session_bits.h" /* MYSQL_SESSION, srv_session_error_cb */

#ifndef MYSQL_ABI_CHECK
#include "mysql/plugin.h" /* MYSQL_THD */
#endif

extern "C" struct srv_session_service_st {
  int (*init_session_thread)(const void *plugin);

  void (*deinit_session_thread)();

  MYSQL_SESSION(*open_session)
  (srv_session_error_cb error_cb, void *plugix_ctx);

  int (*detach_session)(MYSQL_SESSION session);

  int (*close_session)(MYSQL_SESSION session);

  int (*server_is_available)();

  int (*attach_session)(MYSQL_SESSION session, MYSQL_THD *ret_previous_thd);
} * srv_session_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define srv_session_init_thread(plugin) \
  srv_session_service->init_session_thread((plugin))

#define srv_session_deinit_thread() srv_session_service->deinit_session_thread()

#define srv_session_open(cb, ctx) srv_session_service->open_session((cb), (ctx))

#define srv_session_detach(session) \
  srv_session_service->detach_session((session))

#define srv_session_close(session) srv_session_service->close_session((session))

#define srv_session_server_is_available() \
  srv_session_service->server_is_available()

#define srv_session_attach(session, thd) \
  srv_session_service->attach_session((session), (thd))

#else

/**
  Initializes the current physical thread to use with session service.

  Call this function ONLY in physical threads which are not initialized in
  any way by the server.

  @param plugin  Pointer to the plugin structure, passed to the plugin over
                 the plugin init function.

  @return
    0  success
    1  failure
*/
int srv_session_init_thread(const void *plugin);

/**
  Deinitializes the current physical thread to use with session service.


  Call this function ONLY in physical threads which were initialized using
  srv_session_init_thread().
*/
void srv_session_deinit_thread();

/**
  Opens a server session.

  In a thread not initialized by the server itself, this function should be
  called only after srv_session_init_thread() has already been called.

  @param error_cb    session error callback
  @param plugin_ctx  Plugin's context, opaque pointer that would
                     be provided to callbacks. Might be NULL.
  @return
    session   on success
    NULL      on failure
*/
MYSQL_SESSION srv_session_open(srv_session_error_cb error_cb, void *plugin_ctx);

/**
  Detaches a session from current physical thread.

  Detaches a previously session. Which can only occur when the MYSQL_SESSION
  was manually attached by "srv_session_attach".
  Other srv_session calls automatically attached/detached the THD when the
  MYSQL_SESSION is used (for example command_service_run_command()).

  @param session  Session to detach

  @returns
    0  success
    1  failure
*/
int srv_session_detach(MYSQL_SESSION session);

/**
  Closes a previously opened session.

  @param session  Session to close

  @return
    0  success
    1  failure
*/
int srv_session_close(MYSQL_SESSION session);

/**
  Returns if the server is available (not booting or shutting down)

  @return
    0  not available
    1  available
*/
int srv_session_server_is_available();

/**
  Attaches a session to current physical thread.

  Previously attached THD is detached and returned through ret_previous_thd.
  THD associated with session is attached.

  @param session  Session to attach

  @returns
    0  success
    1  failure
*/
int srv_session_attach(MYSQL_SESSION session, MYSQL_THD *ret_previous_thd);

#endif

#endif /* MYSQL_SRV_SESSION_SERVICE_INCLUDED */
