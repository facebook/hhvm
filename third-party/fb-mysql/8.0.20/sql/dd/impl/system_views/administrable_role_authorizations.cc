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

#include "sql/dd/impl/system_views/administrable_role_authorizations.h"

namespace dd {
namespace system_views {

const Administrable_role_authorizations &
Administrable_role_authorizations::instance() {
  static Administrable_role_authorizations *s_instance =
      new Administrable_role_authorizations();
  return *s_instance;
}

Administrable_role_authorizations::Administrable_role_authorizations() {
  m_target_def.set_view_name(view_name());

  m_target_def.add_star();

  m_target_def.add_from("INFORMATION_SCHEMA.APPLICABLE_ROLES");

  m_target_def.add_where("IS_GRANTABLE='YES'");
}

}  // namespace system_views
}  // namespace dd
