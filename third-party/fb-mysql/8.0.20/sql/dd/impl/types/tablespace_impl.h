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

#ifndef DD__TABLESPACE_IMPL_INCLUDED
#define DD__TABLESPACE_IMPL_INCLUDED

#include <memory>  // std::unique_ptr
#include <new>
#include <string>

#include "sql/dd/impl/properties_impl.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/types/entity_object_impl.h"  // dd::Entity_object_impl
#include "sql/dd/impl/types/weak_object_impl.h"
#include "sql/dd/object_id.h"
#include "sql/dd/sdi_fwd.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/tablespace.h"       // dd::Tablespace
#include "sql/dd/types/tablespace_file.h"  // dd::Tablespace_file

class THD;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_table;
class Open_dictionary_tables_ctx;
class Properties;
class Sdi_rcontext;
class Sdi_wcontext;
class Tablespace_file;
class Weak_object;
class Object_table;

class Tablespace_impl : public Entity_object_impl, public Tablespace {
 public:
  Tablespace_impl();

  virtual ~Tablespace_impl();

 public:
  virtual const Object_table &object_table() const;

  virtual bool validate() const;

  virtual bool restore_children(Open_dictionary_tables_ctx *otx);

  virtual bool store_children(Open_dictionary_tables_ctx *otx);

  virtual bool drop_children(Open_dictionary_tables_ctx *otx) const;

  virtual bool store_attributes(Raw_record *r);

  virtual bool restore_attributes(const Raw_record &r);

  void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const;

  bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val);

  virtual void debug_print(String_type &outb) const;

  virtual bool is_empty(THD *thd, bool *empty) const;

 public:
  static void register_tables(Open_dictionary_tables_ctx *otx);

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const { return m_comment; }

  virtual void set_comment(const String_type &comment) { m_comment = comment; }

  /////////////////////////////////////////////////////////////////////////
  // options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const { return m_options; }

  virtual Properties &options() { return m_options; }

  virtual bool set_options(const String_type &options_raw) {
    return m_options.insert_values(options_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const {
    return m_se_private_data;
  }

  virtual Properties &se_private_data() { return m_se_private_data; }

  virtual bool set_se_private_data(const String_type &se_private_data_raw) {
    return m_se_private_data.insert_values(se_private_data_raw);
  }

  /////////////////////////////////////////////////////////////////////////
  // m_engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const { return m_engine; }

  virtual void set_engine(const String_type &engine) { m_engine = engine; }

  /////////////////////////////////////////////////////////////////////////
  // Tablespace file collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Tablespace_file *add_file();

  virtual bool remove_file(String_type data_file);

  virtual const Tablespace_file_collection &files() const { return m_files; }

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

 private:
  // Fields

  String_type m_comment;
  Properties_impl m_options;
  Properties_impl m_se_private_data;
  String_type m_engine;

  // Collections.

  Tablespace_file_collection m_files;

  Tablespace_impl(const Tablespace_impl &src);

  Tablespace *clone() const { return new Tablespace_impl(*this); }
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__TABLESPACE_IMPL_INCLUDED
