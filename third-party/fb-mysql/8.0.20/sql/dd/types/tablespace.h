/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__TABLESPACE_INCLUDED
#define DD__TABLESPACE_INCLUDED

#include <vector>

#include "my_inttypes.h"
#include "sql/dd/collection.h"            // dd::Collection
#include "sql/dd/impl/raw/object_keys.h"  // IWYU pragma: keep
#include "sql/dd/sdi_fwd.h"               // RJ_Document
#include "sql/dd/types/entity_object.h"   // dd::Entity_object
#include "sql/mdl.h"                      // enum enum_mdl_type

class THD;
class MDL_request;

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Global_name_key;
class Properties;
class Tablespace_impl;
class Tablespace_file;
class Void_key;

namespace tables {
class Tablespaces;
}

///////////////////////////////////////////////////////////////////////////

class Tablespace : virtual public Entity_object {
 public:
  typedef Tablespace_impl Impl;
  typedef Tablespace Cache_partition;
  typedef tables::Tablespaces DD_table;
  typedef Primary_id_key Id_key;
  typedef Global_name_key Name_key;
  typedef Void_key Aux_key;
  typedef Collection<Tablespace_file *> Tablespace_file_collection;

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
  virtual ~Tablespace() {}

  /**
    Check if the tablespace is empty, i.e., whether it has any tables.

    @param       thd      Thread context.
    @param [out] empty    Whether the tablespace is empty.

    @return true if error, false if success.
  */

  virtual bool is_empty(THD *thd, bool *empty) const = 0;

  /////////////////////////////////////////////////////////////////////////
  // comment.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &comment() const = 0;
  virtual void set_comment(const String_type &comment) = 0;

  /////////////////////////////////////////////////////////////////////////
  // options.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &options() const = 0;

  virtual Properties &options() = 0;
  virtual bool set_options(const String_type &options_raw) = 0;

  /////////////////////////////////////////////////////////////////////////
  // se_private_data.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &se_private_data() const = 0;

  virtual Properties &se_private_data() = 0;
  virtual bool set_se_private_data(const String_type &se_private_data_raw) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Engine.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &engine() const = 0;
  virtual void set_engine(const String_type &engine) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Tablespace file collection.
  /////////////////////////////////////////////////////////////////////////

  virtual Tablespace_file *add_file() = 0;

  virtual bool remove_file(String_type data_file) = 0;

  virtual const Tablespace_file_collection &files() const = 0;

  /**
    Allocate a new object graph and invoke the copy contructor for
    each object.

    @return pointer to dynamically allocated copy
  */
  virtual Tablespace *clone() const = 0;

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
};

///////////////////////////////////////////////////////////////////////////

/**
  Represents tables with their id, name, schema id and schema name.
  Needed to keep track of information when querying the dd to find
  tables in a tablespace.
 */
struct Tablespace_table_ref {
  Object_id m_id;
  String_type m_name;
  Object_id m_schema_id;
  String_type m_schema_name;
  bool m_schema_encryption;
  Tablespace_table_ref() = default; /* purecov: inspected */
  Tablespace_table_ref(Object_id id, const String_type &&name,
                       Object_id schema_id)
      : m_id{id},
        m_name{std::move(name)},
        m_schema_id{schema_id},
        m_schema_encryption{false} {}
};

bool operator==(const Tablespace_table_ref &a, const Tablespace_table_ref &b);

bool operator<(const Tablespace_table_ref &a, const Tablespace_table_ref &b);

typedef std::vector<Tablespace_table_ref> Tablespace_table_ref_vec;

/**
  Fetch (by inserting into tblref vector) Tablespace_table_ref objects
  which describe tables in a given tablespace.

  @param thd thread context
  @param tso dd object
  @param tblrefs [OUT] Tablespace_table_ref objects for tables in tablespace
  @retval true if error occurred
  @retval false otherwise
 */
bool fetch_tablespace_table_refs(THD *thd, const Tablespace &tso,
                                 Tablespace_table_ref_vec *tblrefs);

/**
  Create am MDL_request for a the table identified by a Tablespace_table_ref.
  @param thd thread context
  @param tref table to create request for
  @param mdl_type The lock type requested.
  @retval MDL_request (allocated on thd->memroot)
 */
MDL_request *mdl_req(THD *thd, const Tablespace_table_ref &tref,
                     enum enum_mdl_type mdl_type);

/**
  Create am MDL_request for a the schema name provided.
  @param thd thread context
  @param schema_name on which to create request for
  @retval MDL_request (allocated on thd->memroot)
 */
MDL_request *mdl_schema_req(THD *thd, const dd::String_type &schema_name);

}  // namespace dd

#endif  // DD__TABLESPACE_INCLUDED
