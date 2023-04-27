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

#ifndef DD__COLLATION_IMPL_INCLUDED
#define DD__COLLATION_IMPL_INCLUDED

#include <stdio.h>
#include <sys/types.h>
#include <new>

#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/collation.h"  // dd::Collation

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;

///////////////////////////////////////////////////////////////////////////

class Collation_impl : public Entity_object_impl, public Collation {
 public:
  Collation_impl()
      : m_is_compiled(false),
        m_sort_length(0),
        m_charset_id(INVALID_OBJECT_ID) {}

 public:
  virtual const Object_table &object_table() const;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

 public:
  /////////////////////////////////////////////////////////////////////////
  // Character set.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id charset_id() const { return m_charset_id; }

  virtual void set_charset_id(Object_id charset_id) {
    m_charset_id = charset_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // compiled
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_compiled() const { return m_is_compiled; }

  virtual void set_is_compiled(bool is_compiled) {
    m_is_compiled = is_compiled;
  }

  /////////////////////////////////////////////////////////////////////////
  // sort_length
  /////////////////////////////////////////////////////////////////////////

  virtual uint sort_length() const { return m_sort_length; }

  virtual void set_sort_length(uint sort_length) {
    m_sort_length = sort_length;
  }

  /////////////////////////////////////////////////////////////////////////
  // pad_attribute
  /////////////////////////////////////////////////////////////////////////

  virtual void set_pad_attribute(enum_pad_attribute pad_attribute) {
    DBUG_ASSERT(pad_attribute != PA_UNDEFINED);
    m_pad_attribute = pad_attribute;
  }

  // Fix "inherits ... via dominance" warnings
  virtual Entity_object_impl *impl() { return Entity_object_impl::impl(); }
  virtual const Entity_object_impl *impl() const {
    return Entity_object_impl::impl();
  }
  virtual Object_id id() const { return Entity_object_impl::id(); }
  virtual bool is_persistent() const {
    return Entity_object_impl::is_persistent();
  }
  virtual const String_type &name() const { return Entity_object_impl::name(); }
  virtual void set_name(const String_type &name) {
    Entity_object_impl::set_name(name);
  }

 public:
  virtual void debug_print(String_type &outb) const {
    char outbuf[1024];
    sprintf(outbuf,
            "COLLATION OBJECT: id= {OID: %lld}, name= %s,"
            "charset_id= {OID: %lld}, is_compiled= %d, sort_length= %u",
            id(), name().c_str(), m_charset_id, m_is_compiled, m_sort_length);
    outb = String_type(outbuf);
  }

 private:
  // Fields
  bool m_is_compiled;
  uint m_sort_length;
  enum_pad_attribute m_pad_attribute;

  // References to other objects
  Object_id m_charset_id;

  Collation *clone() const { return new Collation_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLLATION_IMPL_INCLUDED
