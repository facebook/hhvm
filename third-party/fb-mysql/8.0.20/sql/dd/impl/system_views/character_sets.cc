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

#include "sql/dd/impl/system_views/character_sets.h"

namespace dd {
namespace system_views {

const Character_sets &Character_sets::instance() {
  static Character_sets *s_instance = new Character_sets();
  return *s_instance;
}

Character_sets::Character_sets() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_field(FIELD_CHARACTER_SET_NAME, "CHARACTER_SET_NAME",
                         "cs.name");
  m_target_def.add_field(FIELD_DEFAULT_COLLATE_NAME, "DEFAULT_COLLATE_NAME",
                         "col.name");
  m_target_def.add_field(FIELD_DESCRIPTION, "DESCRIPTION", "cs.comment");
  m_target_def.add_field(FIELD_MAXLEN, "MAXLEN", "cs.mb_max_length");

  m_target_def.add_from("mysql.character_sets cs");
  m_target_def.add_from(
      "JOIN mysql.collations col ON "
      "cs.default_collation_id=col.id ");
}

}  // namespace system_views
}  // namespace dd
