/* Copyright (c) 2017, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/triggers.h"

namespace dd {
namespace system_views {

const Triggers &Triggers::instance() {
  static Triggers *s_instance = new Triggers();
  return *s_instance;
}

Triggers::Triggers() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_TRIGGER_CATALOG, "TRIGGER_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TRIGGER_SCHEMA, "TRIGGER_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_TRIGGER_NAME, "TRIGGER_NAME", "trg.name");
  m_target_def.add_field(FIELD_EVENT_MANIPULATION, "EVENT_MANIPULATION",
                         "trg.event_type");
  m_target_def.add_field(FIELD_EVENT_OBJECT_CATALOG, "EVENT_OBJECT_CATALOG",
                         "cat.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_EVENT_OBJECT_SCHEMA, "EVENT_OBJECT_SCHEMA",
                         "sch.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_EVENT_OBJECT_TABLE, "EVENT_OBJECT_TABLE",
                         "tbl.name" + m_target_def.fs_name_collation());
  m_target_def.add_field(FIELD_ACTION_ORDER, "ACTION_ORDER",
                         "trg.action_order");
  m_target_def.add_field(FIELD_ACTION_CONDITION, "ACTION_CONDITION", "NULL");
  m_target_def.add_field(FIELD_ACTION_STATEMENT, "ACTION_STATEMENT",
                         "trg.action_statement_utf8");
  m_target_def.add_field(FIELD_ACTION_ORIENTATION, "ACTION_ORIENTATION",
                         "'ROW'");
  m_target_def.add_field(FIELD_ACTION_TIMING, "ACTION_TIMING",
                         "trg.action_timing");
  m_target_def.add_field(FIELD_ACTION_REFERENCE_OLD_TABLE,
                         "ACTION_REFERENCE_OLD_TABLE", "NULL");
  m_target_def.add_field(FIELD_ACTION_REFERENCE_NEW_TABLE,
                         "ACTION_REFERENCE_NEW_TABLE", "NULL");
  m_target_def.add_field(FIELD_ACTION_REFERENCE_OLD_ROW,
                         "ACTION_REFERENCE_OLD_ROW", "'OLD'");
  m_target_def.add_field(FIELD_ACTION_REFERENCE_NEW_ROW,
                         "ACTION_REFERENCE_NEW_ROW", "'NEW'");
  m_target_def.add_field(FIELD_CREATED, "CREATED", "trg.created");
  m_target_def.add_field(FIELD_SQL_MODE, "SQL_MODE", "trg.sql_mode");
  m_target_def.add_field(FIELD_DEFINER, "DEFINER", "trg.definer");
  m_target_def.add_field(FIELD_CHARACTER_SET_CLIENT, "CHARACTER_SET_CLIENT",
                         "cs_client.name");
  m_target_def.add_field(FIELD_COLLATION_CONNECTION, "COLLATION_CONNECTION",
                         "coll_conn.name");
  m_target_def.add_field(FIELD_DATABASE_COLLATION, "DATABASE_COLLATION",
                         "coll_db.name");

  m_target_def.add_from(
      "mysql.triggers trg JOIN mysql.tables tbl "
      "ON tbl.id=trg.table_id");
  m_target_def.add_from("JOIN mysql.schemata sch ON tbl.schema_id=sch.id");
  m_target_def.add_from("JOIN mysql.catalogs cat ON cat.id=sch.catalog_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_client "
      "ON coll_client.id=trg.client_collation_id");
  m_target_def.add_from(
      "JOIN mysql.character_sets cs_client "
      "ON cs_client.id=coll_client.character_set_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_conn "
      "ON coll_conn.id=trg.connection_collation_id");
  m_target_def.add_from(
      "JOIN mysql.collations coll_db "
      "ON coll_db.id=trg.schema_collation_id");

  m_target_def.add_where("tbl.type != 'VIEW'");
  m_target_def.add_where("AND CAN_ACCESS_TRIGGER(sch.name, tbl.name)");
  m_target_def.add_where("AND IS_VISIBLE_DD_OBJECT(tbl.hidden)");
}

}  // namespace system_views
}  // namespace dd
