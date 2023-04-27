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

#ifndef DD__CHARSET_INCLUDED
#define DD__CHARSET_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Charset_impl;
class Collation;
class Primary_id_key;
class Global_name_key;
class Void_key;

namespace tables {
class Character_sets;
}

///////////////////////////////////////////////////////////////////////////

class Charset : virtual public Entity_object {
 public:
  typedef Charset_impl Impl;
  typedef Charset Cache_partition;
  typedef tables::Character_sets DD_table;
  typedef Primary_id_key Id_key;
  typedef Global_name_key Name_key;
  typedef Void_key Aux_key;

  // We need a set of functions to update a preallocated key.
  virtual bool update_id_key(Id_key *key) const {
    return update_id_key(key, id());
  }

  static bool update_id_key(Id_key *key, Object_id id);

  virtual bool update_name_key(Name_key *key) const {
    return update_name_key(key, name());
  }

  static bool update_name_key(Name_key *key, const String_type &name);

  virtual bool update_aux_key(Aux_key *) const { return true; }

 public:
  /////////////////////////////////////////////////////////////////////////
  // Default collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id default_collation_id() const = 0;
  virtual void set_default_collation_id(Object_id default_collation_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // mb_max_length
  /////////////////////////////////////////////////////////////////////////

  virtual uint mb_max_length() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // comment
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;

  /**
    Allocate a new object and invoke the copy contructor.

    @return pointer to dynamically allocated copy
  */
  virtual Charset *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__CHARSET_INCLUDED
