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

#ifndef DD__SYSTEM_VIEW_DEFINITION_INCLUDED
#define DD__SYSTEM_VIEW_DEFINITION_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type

namespace dd {
namespace system_views {

/*
  The purpose of this interface is to prepare the DDL statements
  necessary to create a I_S table.
*/

class System_view_definition {
 public:
  virtual ~System_view_definition() {}

  /**
    Build CREATE VIEW DDL statement for the system view.

    @return String_type containing the DDL statement for the target view.
  */
  virtual String_type build_ddl_create_view() const = 0;
};

}  // namespace system_views
}  // namespace dd

#endif  // DD__SYSTEM_VIEW_DEFINITION_INCLUDED
