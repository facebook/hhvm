/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_ADMIN_SESSION_H
#define MYSQL_ADMIN_SESSION_H

#include <mysql/components/service.h>
#include <mysql/service_srv_session_bits.h> /* MYSQL_SESSION,
                                               srv_session_error_cb */

/**
  @ingroup group_components_services_inventory

  A service to create an "admin" session that's not a subject to max connection
  count limits.

  This service is an extension to the sessions plugin service suite.
  The @ref MYSQL_SESSION references are compatible with the plugin service.

  @sa @ref mysql_component_mysql_admin_session_imp
*/
BEGIN_SERVICE_DEFINITION(mysql_admin_session)
/**
  Creates a new session without checking the max_connection limits

  The function is the same as @ref srv_session_open plugin service API, but
  in addition to it it also marks the newly created @ref MYSQL_SESSION as
  "immune" to max_connections count check. The @ref MYSQL_SESSION created
  through this function can be operated as any other @ref MYSQL_SESSION via the
  srv_session_* functions from the @ref srv_session_service_st set.
  @note The @ref MYSQL_SESSION created by this function need to be freed by
  calling
  @ref srv_session_close.

  @sa @ref mysql_component_mysql_admin_session_imp, @ref srv_session_close,
  @ref srv_session_open

  @param error_cb the function to call on error
  @param ctx the context to pass to error_cb
  @return a newly created MYSQL_SESSION
*/
DECLARE_METHOD(MYSQL_SESSION, open, (srv_session_error_cb error_cb, void *ctx));

END_SERVICE_DEFINITION(mysql_admin_session)

#endif /* MYSQL_ADMIN_SESSION_H */
