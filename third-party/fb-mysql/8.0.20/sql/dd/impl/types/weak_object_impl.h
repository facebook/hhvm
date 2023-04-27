/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__WEAK_OBJECT_IMPL_INCLUDED
#define DD__WEAK_OBJECT_IMPL_INCLUDED

#include "sql/dd/object_id.h"          // Object_id
#include "sql/dd/types/weak_object.h"  // dd::Weak_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Entity_object;
class Entity_object_impl;
class Object_key;
class Object_table;
class Open_dictionary_tables_ctx;
class Raw_new_record;
class Raw_record;

///////////////////////////////////////////////////////////////////////////

class Weak_object_impl : virtual public Weak_object {
 public:
  Weak_object_impl() {}

  virtual ~Weak_object_impl() {}

 public:
  virtual const Object_table &object_table() const = 0;

  virtual bool validate() const = 0;

  // NOTE: the store() operation can not be made constant as
  // the object state is modified when storing a newly created object
  // (object id is assigned using auto-increment).
  virtual bool store(Open_dictionary_tables_ctx *otx);

  bool drop(Open_dictionary_tables_ctx *otx) const;

 public:
  virtual bool restore_attributes(const Raw_record &r) = 0;

  virtual bool store_attributes(Raw_record *r) = 0;

 public:
  // Restore's all the related collections.
  // There are 2 scenarions when collection is filled.
  // 1) Parent object is retrieved using restore()
  //    and then restore collections.
  //    Eg: Tablespace (Parent object) invoked restore()
  //        and then call restore_children() to fetch
  //        Tablespace_file objects.
  //
  // 2) Parent object is fetched using Raw_record_set->next()
  //    and then restore collections is called for each
  //    parent object fetched.
  //    Eg. Indexes (Parent object) that belong to a Table object
  //        is fetched and then Index_element collections per
  //        index is restored using restore_children().
  //
  virtual bool restore_children(Open_dictionary_tables_ctx *) { return false; }

  virtual bool store_children(Open_dictionary_tables_ctx *) { return false; }

  virtual bool drop_children(Open_dictionary_tables_ctx *) const {
    return false;
  }

  /**
    Indicates that object is guaranteed to have primary key value which
    doesn't exist in database (e.g. because it only will be generated
    using auto-increment at store() time). So it is ok for store() method
    to skip lookup of existing object with the same primary key and simply
    try to insert new object into the table.
  */
  virtual bool has_new_primary_key() const = 0;

 protected:
  virtual Object_key *create_primary_key() const = 0;

  // set_primary_key_value() is called after new object has been inserted into
  // the table, giving the chance to get inserted values of AUTO_INCREMENT
  // columns. It gives a chance for Entity_object to override it.
  virtual void set_primary_key_value(const Raw_new_record &) {}

  /*
    Called by store() method to allow resetting of has_new_primary_key()
    property after we completed loading of object and its children.
  */
  virtual void fix_has_new_primary_key() {}

 protected:
  // Check if the parent object id matches with this object.
  bool check_parent_consistency(Entity_object_impl *parent,
                                Object_id parent_id) const;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__WEAK_OBJECT_IMPL_INCLUDED
