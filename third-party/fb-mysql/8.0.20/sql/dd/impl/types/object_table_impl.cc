/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <mysqld_error.h>

#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // DD_bootstrap_ctx
#include "sql/dd/impl/types/object_table_impl.h"  // Object_table_impl
#include "sql/table.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

Object_table_impl::Object_table_impl()
    : m_last_dd_version(0),
      m_target_def(),
      m_actual_present(false),
      m_actual_def(),
      m_hidden(true) {
  m_target_def.add_option(static_cast<int>(Common_option::ENGINE), "ENGINE",
                          "ENGINE=INNODB");
  m_target_def.add_option(static_cast<int>(Common_option::CHARSET), "CHARSET",
                          "DEFAULT CHARSET=utf8");
  m_target_def.add_option(static_cast<int>(Common_option::COLLATION),
                          "COLLATION", "COLLATE=utf8_bin");
  m_target_def.add_option(static_cast<int>(Common_option::ROW_FORMAT),
                          "ROW_FORMAT", "ROW_FORMAT=DYNAMIC");
  m_target_def.add_option(static_cast<int>(Common_option::STATS_PERSISTENT),
                          "STATS_PERSISTENT", "STATS_PERSISTENT=0");
  m_target_def.add_option(
      static_cast<int>(Common_option::TABLESPACE), "TABLESPACE",
      String_type("TABLESPACE=") + String_type(MYSQL_TABLESPACE_NAME.str));
}

bool Object_table_impl::set_actual_table_definition(
    const Properties &table_def_properties) const {
  m_actual_present = true;
  return m_actual_def.restore_from_properties(table_def_properties);
}

int Object_table_impl::field_number(int target_field_number,
                                    const String_type &field_label) const {
  /*
    During upgrade, we must re-interpret the field number using the
    field label. Otherwise, we use the target field number. Note that
    for minor downgrade, we use the target field number directly since
    only extensions are allowed.
  */
  if (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade())
    return m_actual_def.field_number(field_label);
  return target_field_number;
}

int Object_table_impl::field_number(const String_type &field_label) const {
  /*
    During upgrade, we must get the position of the field label from
    the actual definition. Otherwise, we get the position from the
    the target definition.
  */
  if (bootstrap::DD_bootstrap_ctx::instance().is_dd_upgrade())
    return m_actual_def.field_number(field_label);
  return m_target_def.field_number(field_label);
}

///////////////////////////////////////////////////////////////////////////

Object_table *Object_table::create_object_table() {
  return new (std::nothrow) Object_table_impl();
}

///////////////////////////////////////////////////////////////////////////

}  // namespace dd
