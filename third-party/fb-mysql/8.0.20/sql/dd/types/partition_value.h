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

#ifndef DD__PARTITION_VALUE_INCLUDED
#define DD__PARTITION_VALUE_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/sdi_fwd.h"            // dd::Sdi_wcontext
#include "sql/dd/types/weak_object.h"  // dd::Weak_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Partition;
class Partition_value_impl;

namespace tables {
class Table_partition_values;
}

///////////////////////////////////////////////////////////////////////////

class Partition_value : virtual public Weak_object {
 public:
  typedef Partition_value_impl Impl;
  typedef tables::Table_partition_values DD_table;

 public:
  virtual ~Partition_value() {}

  /////////////////////////////////////////////////////////////////////////
  // Partition.
  /////////////////////////////////////////////////////////////////////////

  virtual const Partition &partition() const = 0;

  virtual Partition &partition() = 0;

  /////////////////////////////////////////////////////////////////////////
  // list_num.
  /////////////////////////////////////////////////////////////////////////

  virtual uint list_num() const = 0;
  virtual void set_list_num(uint list_num) = 0;

  /////////////////////////////////////////////////////////////////////////
  // column_num.
  /////////////////////////////////////////////////////////////////////////

  virtual uint column_num() const = 0;
  virtual void set_column_num(uint column_num) = 0;

  /////////////////////////////////////////////////////////////////////////
  // value.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &value_utf8() const = 0;
  virtual void set_value_utf8(const String_type &value) = 0;

  /////////////////////////////////////////////////////////////////////////
  // max_value.
  /////////////////////////////////////////////////////////////////////////

  virtual bool max_value() const = 0;
  virtual void set_max_value(bool max_value) = 0;

  /////////////////////////////////////////////////////////////////////////
  // null_value.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_value_null() const = 0;
  virtual void set_value_null(bool is_null) = 0;

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

}  // namespace dd

#endif  // DD__PARTITION_VALUE_INCLUDED
