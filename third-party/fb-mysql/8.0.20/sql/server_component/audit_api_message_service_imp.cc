/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "audit_api_message_service_imp.h"

#include "sql/current_thd.h"
#include "sql/sql_audit.h"

DEFINE_BOOL_METHOD(mysql_audit_api_message_imp::emit,
                   (mysql_event_message_subclass_t type, const char *component,
                    size_t component_length, const char *producer,
                    size_t producer_length, const char *message,
                    size_t message_length,
                    mysql_event_message_key_value_t *key_value_map,
                    size_t key_value_map_length)) {
  if (type == MYSQL_AUDIT_MESSAGE_INTERNAL)
    mysql_audit_notify(current_thd, AUDIT_EVENT(MYSQL_AUDIT_MESSAGE_INTERNAL),
                       component, component_length, producer, producer_length,
                       message, message_length, key_value_map,
                       key_value_map_length);
  else
    mysql_audit_notify(current_thd, AUDIT_EVENT(MYSQL_AUDIT_MESSAGE_USER),
                       component, component_length, producer, producer_length,
                       message, message_length, key_value_map,
                       key_value_map_length);

  return false;
}
