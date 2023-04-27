/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/system_views/innodb_datafiles.h"

namespace dd {
namespace system_views {

const Innodb_datafiles &Innodb_datafiles::instance() {
  static Innodb_datafiles *s_instance = new Innodb_datafiles();
  return *s_instance;
}

Innodb_datafiles::Innodb_datafiles() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(
      FIELD_SPACE, "SPACE",
      "GET_DD_TABLESPACE_PRIVATE_DATA(ts.se_private_data, 'id')");
  m_target_def.add_field(FIELD_PATH, "PATH", "ts_files.file_name");

  m_target_def.add_from("mysql.tablespace_files ts_files");
  m_target_def.add_from(
      "JOIN mysql.tablespaces ts ON "
      "ts.id=ts_files.tablespace_id");

  m_target_def.add_where("ts.se_private_data IS NOT NULL");
  m_target_def.add_where("AND ts.engine='InnoDB'");
  m_target_def.add_where("AND ts.name<>'mysql'");
  m_target_def.add_where("AND ts.name<>'innodb_temporary'");
}

}  // namespace system_views
}  // namespace dd
