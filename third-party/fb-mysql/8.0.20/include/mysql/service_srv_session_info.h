#ifndef MYSQL_SERVICE_SRV_SESSION_INFO_INCLUDED
#define MYSQL_SERVICE_SRV_SESSION_INFO_INCLUDED
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

/**
  @file include/mysql/service_srv_session_info.h
  Service providing setters and getters for some properties of a session
*/

#include "mysql/service_srv_session.h"
#ifndef MYSQL_ABI_CHECK
#include <stdint.h>    /* uint16_t */
#include "m_string.h"  /* LEX_CSTRING */
#include "my_thread.h" /* my_thread_id */
#include "my_thread_local.h"
#include "mysql_com.h" /* Vio for violite.h */
#include "plugin.h"    /* MYSQL_THD */

enum enum_vio_type : int;
#endif

extern "C" struct srv_session_info_service_st {
  MYSQL_THD (*get_thd)(MYSQL_SESSION session);

  my_thread_id (*get_session_id)(MYSQL_SESSION session);

  LEX_CSTRING (*get_current_db)(MYSQL_SESSION session);

  uint16_t (*get_client_port)(MYSQL_SESSION session);
  int (*set_client_port)(MYSQL_SESSION session, uint16_t port);

  int (*set_connection_type)(MYSQL_SESSION session, enum enum_vio_type type);

  int (*killed)(MYSQL_SESSION session);

  unsigned int (*session_count)();
  unsigned int (*thread_count)(const void *plugin);
} * srv_session_info_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define srv_session_info_get_thd(session) \
  srv_session_info_service->get_thd((session))
#define srv_session_info_get_session_id(sess) \
  srv_session_info_service->get_session_id((sess))
#define srv_session_info_get_current_db(sess) \
  srv_session_info_service->get_current_db((sess))
#define srv_session_info_get_client_port(sess) \
  srv_session_info_service->get_client_port((sess))
#define srv_session_info_set_client_port(sess, port) \
  srv_session_info_service->set_client_port((sess), (port))
#define srv_session_info_set_connection_type(sess, type) \
  srv_session_info_service->set_connection_type((sess), (type))
#define srv_session_info_killed(sess) srv_session_info_service->killed((sess))
#define srv_session_info_session_count(sess) \
  srv_session_info_service->session_count(sess)
#define srv_session_info_thread_count(plugin) \
  srv_session_info_service->thread_count(plugin)

#else

/**
  Returns the THD of a session.

  @param session  Session

  @returns
    address of the THD
*/
MYSQL_THD srv_session_info_get_thd(MYSQL_SESSION session);

/**
  Returns the ID of a session.

  @param session Session
*/
my_thread_id srv_session_info_get_session_id(MYSQL_SESSION session);

/**
  Returns the current database of a session.

  @note {NULL, 0} is returned case of no current database or session is NULL


  @param session Session
*/
LEX_CSTRING srv_session_info_get_current_db(MYSQL_SESSION session);

/**
  Returns the client port of a session.

  @note The client port in SHOW PROCESSLIST, INFORMATION_SCHEMA.PROCESSLIST.
  This port is NOT shown in PERFORMANCE_SCHEMA.THREADS.

  @param session Session
*/
uint16_t srv_session_info_get_client_port(MYSQL_SESSION session);

/**
  Sets the client port of a session.

  @note The client port in SHOW PROCESSLIST, INFORMATION_SCHEMA.PROCESSLIST.
  This port is NOT shown in PERFORMANCE_SCHEMA.THREADS.

  @param session  Session
  @param port     Port number

  @return
    0 success
    1 failure
*/
int srv_session_info_set_client_port(MYSQL_SESSION session, uint16_t port);

/**
  Sets the connection type of a session.

  @see enum_vio_type

  @note The type is shown in PERFORMANCE_SCHEMA.THREADS. The value is translated
        from the enum to a string according to @see vio_type_names array
        in vio/vio.c

  @note If NO_VIO_TYPE passed as type the call will fail.

  @return
    0  success
    1  failure
*/
int srv_session_info_set_connection_type(MYSQL_SESSION session,
                                         enum enum_vio_type type);

/**
  Returns whether the session was killed

  @param session  Session

  @return
    0  not killed
    1  killed
*/
int srv_session_info_killed(MYSQL_SESSION session);

/**
  Returns the number opened sessions in thread initialized by srv_session
  service.
*/
unsigned int srv_session_info_session_count();

/**
  Returns the number opened sessions in thread initialized by srv_session
  service.

  @param plugin Pointer to the plugin structure, passed to the plugin over
                the plugin init function.
*/
unsigned int srv_session_info_thread_count(const void *plugin);

#endif /* MYSQL_DYNAMIC_PLUGIN */

#endif /* MYSQL_SERVICE_SRV_SESSION_INFO_INCLUDED */
