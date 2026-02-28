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

#ifndef DD__SYSTEM_VIEW_INCLUDED
#define DD__SYSTEM_VIEW_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type

namespace dd {
namespace system_views {

class System_view_definition;

/**
  This class represents base class for all INFORMATION_SCHEMA system views
  defined in sql/dd/impl/system_views/ headers.
*/

class System_view {
 public:
  virtual ~System_view() {}

  /*
    Get name of system view.

    @return String containing name of view.
  */
  virtual const String_type &name() const = 0;

  /*
    Get SQL string containing CREATE VIEW definition to create the system view.

    @return SQL String.
  */
  virtual const System_view_definition *view_definition() const = 0;

  /*
    Check if the system view is to be hidden from users.

    @return true  If system view is hidden.
            false If system view is not hidden.

  */
  virtual bool hidden() const = 0;
};

}  // namespace system_views
}  // namespace dd

#endif  // DD__SYSTEM_VIEW_INCLUDED
