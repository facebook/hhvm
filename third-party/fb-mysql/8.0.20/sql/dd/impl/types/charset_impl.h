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

#ifndef DD__CHARSET_IMPL_INCLUDED
#define DD__CHARSET_IMPL_INCLUDED

#include <stdio.h>
#include <sys/types.h>
#include <new>

#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_imp
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/charset.h"  // dd::Charset

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;
class Raw_record;
class Weak_object;
class Object_table;

///////////////////////////////////////////////////////////////////////////

class Charset_impl : public Entity_object_impl, public Charset {
 public:
  Charset_impl()
      : m_mb_max_length(0), m_default_collation_id(INVALID_OBJECT_ID) {}

 public:
  virtual const Object_table &object_table() const;

  static void register_tables(Open_dictionary_tables_ctx *otx);

  virtual bool validate() const;

  virtual bool restore_attributes(const Raw_record &r);

  virtual bool store_attributes(Raw_record *r);

 public:
  /////////////////////////////////////////////////////////////////////////
  // Default collation.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id default_collation_id() const {
    return m_default_collation_id;
  }

  virtual void set_default_collation_id(Object_id collation_id) {
    m_default_collation_id = collation_id;
  }

  /////////////////////////////////////////////////////////////////////////
  // mb_max_length
  /////////////////////////////////////////////////////////////////////////

  virtual uint mb_max_length() const { return m_mb_max_length; }

  virtual void set_mb_max_length(uint mb_max_length) {
    m_mb_max_length = mb_max_length;
  }

  /////////////////////////////////////////////////////////////////////////
  // comment
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

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
            "CHARSET OBJECT: {OID: %lld}, name= %s, "
            "collation_id= {OID: %lld}, mb_max_length= %u, "
            "comment= %s",
            id(), name().c_str(), m_default_collation_id, m_mb_max_length,
            m_comment.c_str());
    outb = String_type(outbuf);
  }

 private:
  uint m_mb_max_length;
  String_type m_comment;

  Object_id m_default_collation_id;

  Charset *clone() const { return new Charset_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__CHARSET_IMPL_INCLUDED
