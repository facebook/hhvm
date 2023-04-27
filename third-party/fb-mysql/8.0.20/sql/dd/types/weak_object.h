/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__WEAK_OBJECT_INCLUDED
#define DD__WEAK_OBJECT_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type XXX: temporary, debug-only

namespace dd {

///////////////////////////////////////////////////////////////////////////

/**
  Base class for all data dictionary objects.

  @note This class may be inherited along different paths
        for some subclasses due to the diamond shaped
        inheritance hierarchy; thus, direct subclasses
        must inherit this class virtually.
*/

class Weak_object {
 public:
  // XXX: temporary, debug-only.
  virtual void debug_print(String_type &outb) const = 0;

 public:
  Weak_object() {}

  Weak_object(const Weak_object &) = default;

  virtual ~Weak_object() {}
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__WEAK_OBJECT_INCLUDED
