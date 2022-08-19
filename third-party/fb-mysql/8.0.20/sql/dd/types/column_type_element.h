/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__COLUMN_TYPE_ELEMENT_INCLUDED
#define DD__COLUMN_TYPE_ELEMENT_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/sdi_fwd.h"            // dd::Sdi_rcontext
#include "sql/dd/types/weak_object.h"  // dd::Weak_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Column;
class Column_type_element_impl;

namespace tables {
class Column_type_elements;
}

///////////////////////////////////////////////////////////////////////////

class Column_type_element : virtual public Weak_object {
 public:
  typedef Column_type_element_impl Impl;
  typedef tables::Column_type_elements DD_table;

 public:
  virtual ~Column_type_element() {}

  /////////////////////////////////////////////////////////////////////////
  // Name
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &name() const = 0;
  virtual void set_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Column
  /////////////////////////////////////////////////////////////////////////

  virtual const Column &column() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // Index
  /////////////////////////////////////////////////////////////////////////

  virtual uint index() const = 0;

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
    @retval false success
    @retval true  failure
  */

  virtual bool deserialize(Sdi_rcontext *rctx, const RJ_Value &val) = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLUMN_TYPE_ELEMENT_INCLUDED
