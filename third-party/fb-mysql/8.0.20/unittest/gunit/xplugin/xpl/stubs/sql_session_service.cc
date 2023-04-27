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

#include "my_dbug.h"
#include "mysql/service_srv_session.h"

int srv_session_init_thread(const void *) {
  DBUG_ASSERT(0);
  return 0;
}

void srv_session_deinit_thread() { DBUG_ASSERT(0); }

MYSQL_THD srv_session_info_get_thd(MYSQL_SESSION) {
  DBUG_ASSERT(0);
  return nullptr;
}

MYSQL_SESSION srv_session_open(srv_session_error_cb, void *) {
  DBUG_ASSERT(0);
  return nullptr;
}

int srv_session_close(MYSQL_SESSION) {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_detach(MYSQL_SESSION) {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_attach(MYSQL_SESSION, MYSQL_THD *) {
  DBUG_ASSERT(0);
  return 0;
}

my_thread_id srv_session_info_get_session_id(MYSQL_SESSION) {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_server_is_available() {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_info_set_client_port(Srv_session *, uint16_t) {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_info_set_connection_type(Srv_session *, enum_vio_type) {
  DBUG_ASSERT(0);
  return 0;
}

int srv_session_info_killed(MYSQL_SESSION) {
  DBUG_ASSERT(0);
  return 0;
}
