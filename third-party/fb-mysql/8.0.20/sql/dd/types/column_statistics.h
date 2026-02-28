/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__COLUMN_STATISTIC_INCLUDED
#define DD__COLUMN_STATISTIC_INCLUDED

#include "my_alloc.h"                    // MEM_ROOT
#include "sql/dd/sdi_fwd.h"              // RJ_Document
#include "sql/dd/types/entity_object.h"  // dd::Entity_object

class THD;
struct MDL_key;

namespace histograms {
class Histogram;
}

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column_statistics_impl;
class Item_name_key;
class Primary_id_key;
class Void_key;

namespace tables {
class Column_statistics;
}

///////////////////////////////////////////////////////////////////////////

class Column_statistics : virtual public Entity_object {
 protected:
  /// MEM_ROOT on which the histogram data is allocated.
  MEM_ROOT m_mem_root;

 public:
  typedef Column_statistics_impl Impl;
  typedef Column_statistics Cache_partition;
  typedef tables::Column_statistics DD_table;
  typedef Primary_id_key Id_key;
  typedef Item_name_key Name_key;
  typedef Void_key Aux_key;

  // We need a set of functions to update a preallocated key.
  bool update_id_key(Id_key *key) const { return update_id_key(key, id()); }

  static bool update_id_key(Id_key *key, Object_id id);

  bool update_name_key(Name_key *key) const {
    return update_name_key(key, name());
  }

  static bool update_name_key(Name_key *key, const String_type &name);

  bool update_aux_key(Aux_key *) const { return true; }

  virtual ~Column_statistics() {}

  virtual const String_type &schema_name() const = 0;
  virtual void set_schema_name(const String_type &schema_name) = 0;

  virtual const String_type &table_name() const = 0;
  virtual void set_table_name(const String_type &table_name) = 0;

  virtual const String_type &column_name() const = 0;
  virtual void set_column_name(const String_type &column_name) = 0;

  virtual const histograms::Histogram *histogram() const = 0;
  virtual void set_histogram(const histograms::Histogram *histogram) = 0;

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

  /*
    Create a unique name for a column statistic object based on the triplet
    SCHEMA_NAME TABLE_NAME COLUMN_NAME separated with the 'Unit Separator'
    character.
  */
  static String_type create_name(const String_type &schema_name,
                                 const String_type &table_name,
                                 const String_type &column_name);

  String_type create_name() const {
    return Column_statistics::create_name(schema_name(), table_name(),
                                          column_name());
  }

  static void create_mdl_key(const String_type &schema_name,
                             const String_type &table_name,
                             const String_type &column_name, MDL_key *key);

  void create_mdl_key(MDL_key *key) const {
    Column_statistics::create_mdl_key(schema_name(), table_name(),
                                      column_name(), key);
  }

  virtual Column_statistics *clone() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLUMN_STATISTIC_INCLUDED
