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

#ifndef DD__INDEX_INCLUDED
#define DD__INDEX_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/collection.h"           // dd::Collection
#include "sql/dd/sdi_fwd.h"              // dd::Sdi_rcontext
#include "sql/dd/sdi_fwd.h"              // dd::Sdi_wcontext
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Index_impl;
class Index_element;
class Object_table;
class Properties;
class Table;

namespace tables {
class Indexes;
}

///////////////////////////////////////////////////////////////////////////

class Index : virtual public Entity_object {
 public:
  typedef Collection<Index_element *> Index_elements;
  typedef Index_impl Impl;
  typedef tables::Indexes DD_table;

 public:
  enum enum_index_type  // similar to Keytype in sql_class.h but w/o FOREIGN_KEY
  { IT_PRIMARY = 1,
    IT_UNIQUE,
    IT_MULTIPLE,
    IT_FULLTEXT,
    IT_SPATIAL };

  enum enum_index_algorithm  // similar to ha_key_alg
  { IA_SE_SPECIFIC = 1,
    IA_BTREE,
    IA_RTREE,
    IA_HASH,
    IA_FULLTEXT };

 public:
  virtual ~Index() {}

  /**
    Dummy method to be able to use Partition_index and Index interchangeably
    in templates.
  */
  const Index &index() const { return *this; }

  Index &index() { return *this; }

  /////////////////////////////////////////////////////////////////////////
  // Table.
  /////////////////////////////////////////////////////////////////////////

  virtual const Table &table() const = 0;

  virtual Table &table() = 0;

  /////////////////////////////////////////////////////////////////////////
  // is_generated
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_generated() const = 0;
  virtual void set_generated(bool generated) = 0;

  /////////////////////////////////////////////////////////////////////////
  // hidden.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_hidden() const = 0;
  virtual void set_hidden(bool hidden) = 0;

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;
  virtual void set_comment(const String_type &comment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const = 0;

  virtual Properties &options() = 0;
  virtual bool set_options(const Properties &options) = 0;
  virtual bool set_options(const String_type &options_raw) = 0;

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const = 0;

  virtual Properties &se_private_data() = 0;
  virtual bool set_se_private_data(const String_type &se_private_data_raw) = 0;
  virtual bool set_se_private_data(const Properties &se_private_data) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Tablespace.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id tablespace_id() const = 0;
  virtual void set_tablespace_id(Object_id tablespace_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const = 0;
  virtual void set_engine(const String_type &engine) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Index type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_index_type type() const = 0;
  virtual void set_type(enum_index_type type) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Index algorithm.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_index_algorithm algorithm() const = 0;
  virtual void set_algorithm(enum_index_algorithm algorithm) = 0;

  virtual bool is_algorithm_explicit() const = 0;
  virtual void set_algorithm_explicit(bool alg_expl) = 0;

  virtual bool is_visible() const = 0;
  virtual void set_visible(bool is_visible) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Index-element collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Index_element *add_element(Column *c) = 0;

  virtual const Index_elements &elements() const = 0;

  virtual void set_ordinal_position(uint ordinal_position) = 0;

  virtual uint ordinal_position() const = 0;

  /**
    Converts *this into json.

    Converts all member variables that are to be included in the sdi
    into json by transforming them appropriately and passing them to
    the rapidjson writer provided.

    @param wctx opaque context for data needed by serialization
    @param w rapidjson writer which will perform conversion to json

  */

  virtual void serialize(Sdi_wcontext *wctx, Sdi_writer *w) const = 0;

  /**
    Re-establishes the state of *this by reading sdi information from
    the rapidjson DOM subobject provided.

    Cross-references encountered within this object are tracked in
    sdictx, so that they can be updated when the entire object graph
    has been established.

    @param rctx stores book-keeping information for the
    deserialization process
    @param val subobject of rapidjson DOM containing json
    representation of this object
  */

  virtual bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) = 0;

  /**
    Check if index represents candidate key.
  */
  virtual bool is_candidate_key() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__INDEX_INCLUDED
