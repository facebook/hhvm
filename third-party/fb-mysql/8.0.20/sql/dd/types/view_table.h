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

#ifndef DD__VIEW_TABLE_INCLUDED
#define DD__VIEW_TABLE_INCLUDED

#include "sql/dd/types/weak_object.h"  // dd::Weak_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class View;
class View_table_impl;

namespace tables {
class View_table_usage;
}

///////////////////////////////////////////////////////////////////////////

class View_table : virtual public Weak_object {
 public:
  typedef View_table_impl Impl;
  typedef tables::View_table_usage DD_table;

 public:
  virtual ~View_table() {}

  /////////////////////////////////////////////////////////////////////////
  // View table catalog name.
  // XXX: do we need it now?
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_catalog() const = 0;
  virtual void set_table_catalog(const String_type &table_catalog) = 0;

  /////////////////////////////////////////////////////////////////////////
  // View table schema name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_schema() const = 0;
  virtual void set_table_schema(const String_type &table_schema) = 0;

  /////////////////////////////////////////////////////////////////////////
  // View table name.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &table_name() const = 0;
  virtual void set_table_name(const String_type &table_name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Parent view.
  /////////////////////////////////////////////////////////////////////////

  virtual const View &view() const = 0;

  virtual View &view() = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__VIEW_TABLE_INCLUDED
