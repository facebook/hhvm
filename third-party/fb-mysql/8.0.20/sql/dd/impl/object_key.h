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

#ifndef DD__OBJECT_KEY_INCLUDED
#define DD__OBJECT_KEY_INCLUDED

#include "sql/dd/string_type.h"  // dd::String_type

namespace dd {

///////////////////////////////////////////////////////////////////////////

struct Raw_key;
class Raw_table;

///////////////////////////////////////////////////////////////////////////

class Object_key {
 public:
  virtual Raw_key *create_access_key(Raw_table *t) const = 0;

  virtual String_type str() const = 0;

 public:
  virtual ~Object_key() {}

  Object_key() = default;
  Object_key(const Object_key &) = default;
  Object_key(Object_key &&) = default;
  Object_key &operator=(const Object_key &) = default;
  Object_key &operator=(Object_key &&) = default;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__OBJECT_KEY_INCLUDED
