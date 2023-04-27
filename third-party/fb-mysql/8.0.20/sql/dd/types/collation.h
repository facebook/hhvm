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

#ifndef DD__COLLATION_INCLUDED
#define DD__COLLATION_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Charset;
class Collation_impl;
class Primary_id_key;
class Global_name_key;
class Void_key;

namespace tables {
class Collations;
}

///////////////////////////////////////////////////////////////////////////

class Collation : virtual public Entity_object {
 public:
  typedef Collation_impl Impl;
  typedef Collation Cache_partition;
  typedef tables::Collations DD_table;
  typedef Primary_id_key Id_key;
  typedef Global_name_key Name_key;
  typedef Void_key Aux_key;

  // Persisted pad attribute, mapped from Pad_attribute enum defined
  // in include/m_ctype.h. The setter is not part of the public API,
  // and there is no getter, since this attribute is only exposed
  // throught the I_S.
  enum enum_pad_attribute { PA_UNDEFINED, PA_PAD_SPACE, PA_NO_PAD };

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
  // Character set.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id charset_id() const = 0;
  virtual void set_charset_id(Object_id charset_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // compiled
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_compiled() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // sort_length
  /////////////////////////////////////////////////////////////////////////

  virtual uint sort_length() const = 0;

  /**
    Allocate a new object and invoke the copy contructor.

    @return pointer to dynamically allocated copy
  */
  virtual Collation *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLLATION_INCLUDED
