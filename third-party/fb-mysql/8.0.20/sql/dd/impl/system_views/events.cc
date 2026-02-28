/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/events.h"

namespace dd {
namespace system_views {

const Events &Events::instance() {
  static Events *s_instance = new Events();
  return *s_instance;
}

Events::Events() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_EVENT_CATALOG, "EVENT_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_EVENT_SCHEMA, "EVENT_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_EVENT_NAME, "EVENT_NAME", "evt.name");
  m_target_def.add_field(FIELD_DEFINER, "DEFINER", "evt.definer");
  m_target_def.add_field(FIELD_TIME_ZONE, "TIME_ZONE", "evt.time_zone");
  m_target_def.add_field(FIELD_EVENT_BODY, "EVENT_BODY", "'SQL'");
  m_target_def.add_field(FIELD_EVENT_DEFINITION, "EVENT_DEFINITION",
                         "evt.definition_utf8");
  m_target_def.add_field(
      FIELD_EVENT_TYPE, "EVENT_TYPE",
      "IF (ISNULL(evt.interval_value),'ONE TIME','RECURRING')");
  m_target_def.add_field(FIELD_EXECUTE_AT, "EXECUTE_AT",
                         "CONVERT_TZ(evt.execute_at,'+00:00', evt.time_zone)");
  m_target_def.add_field(FIELD_INTERVAL_VALUE, "INTERVAL_VALUE",
                         "CONVERT_INTERVAL_TO_USER_INTERVAL(evt.interval_value,"
                         "evt.interval_field)");
  m_target_def.add_field(FIELD_INTERVAL_FIELD, "INTERVAL_FIELD",
                         "evt.interval_field");
  m_target_def.add_field(FIELD_SQL_MODE, "SQL_MODE", "evt.sql_mode");
  m_target_def.add_field(FIELD_STARTS, "STARTS",
                         "CONVERT_TZ(evt.starts,'+00:00', evt.time_zone)");
  m_target_def.add_field(FIELD_ENDS, "ENDS",
                         "CONVERT_TZ(evt.ends,'+00:00', evt.time_zone)");
  m_target_def.add_field(FIELD_STATUS, "STATUS", "evt.status");
  m_target_def.add_field(
      FIELD_ON_COMPLETION, "ON_COMPLETION",
      "IF (evt.on_completion='DROP', 'NOT PRESERVE', 'PRESERVE')");
  m_target_def.add_field(FIELD_CREATED, "CREATED", "evt.created");
  m_target_def.add_field(FIELD_LAST_ALTERED, "LAST_ALTERED",
                         "evt.last_altered");
  m_target_def.add_field(
      FIELD_LAST_EXECUTED, "LAST_EXECUTED",
      "CONVERT_TZ(evt.last_executed,'+00:00', evt.time_zone)");
  m_target_def.add_field(FIELD_EVENT_COMMENT, "EVENT_COMMENT", "evt.comment");
  m_target_def.add_field(FIELD_ORIGINATOR, "ORIGINATOR", "evt.originator");
  m_target_def.add_field(FIELD_CHARACTER_SET_CLIENT, "CHARACTER_SET_CLIENT",
                         "cs_client.name");
  m_target_def.add_field(FIELD_COLLATION_CONNECTION, "COLLATION_CONNECTION",
                         "coll_conn.name");
  m_target_def.add_field(FIELD_DATABASE_COLLATION, "DATABASE_COLLATION",
                         "coll_db.name");

  m_target_def.add_from("mysql.events evt");
  m_target_def.add_from("JOIN mysql.schemata sch ON evt.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_client "
      "ON coll_client.id=evt.client_collation_id");
  m_target_def.add_from(
      "JOIN mysql.character_sets cs_client "
      "ON cs_client.id=coll_client.character_set_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_conn "
      "ON coll_conn.id=evt.connection_collation_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_db "
      "ON coll_db.id=evt.schema_collation_id");

  m_target_def.add_where("CAN_ACCESS_EVENT(sch.name)");
}

}  // namespace system_views
}  // namespace dd
