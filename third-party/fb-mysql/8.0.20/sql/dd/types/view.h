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

#ifndef DD__VIEW_INCLUDED
#define DD__VIEW_INCLUDED

#include "sql/dd/types/abstract_table.h"  // dd::Abstract_table

namespace dd {

///////////////////////////////////////////////////////////////////////////

class View_impl;
class View_table;
class View_routine;

///////////////////////////////////////////////////////////////////////////

class View : virtual public Abstract_table {
 public:
  typedef Collection<View_table *> View_tables;
  typedef Collection<View_routine *> View_routines;
  typedef View_impl Impl;

 public:
  enum enum_check_option  // VIEW_CHECK_NONE, VIEW_CHECK_LOCAL,
                          // VIEW_CHECK_CASCADED
  { CO_NONE = 1,
    CO_LOCAL,
    CO_CASCADED };

  enum enum_algorithm  // VIEW_ALGORITHM_UNDEFINED, VIEW_ALGORITHM_TMPTABLE,
                       // VIEW_ALGORITHM_MERGE
  { VA_UNDEFINED = 1,
    VA_TEMPORARY_TABLE,
    VA_MERGE };

  enum enum_security_type { ST_DEFAULT = 1, ST_INVOKER, ST_DEFINER };

 public:
  virtual ~View() {}

  /////////////////////////////////////////////////////////////////////////
  // regular/system view flag.
  /////////////////////////////////////////////////////////////////////////

  /* non-virtual */ bool is_system_view() const {
    return type() == enum_table_type::SYSTEM_VIEW;
  }

  virtual void set_system_view(bool system_view) = 0;

  /////////////////////////////////////////////////////////////////////////
  // collations.
  /////////////////////////////////////////////////////////////////////////

  virtual Object_id client_collation_id() const = 0;
  virtual void set_client_collation_id(Object_id client_collation_id) = 0;

  virtual Object_id connection_collation_id() const = 0;
  virtual void set_connection_collation_id(
      Object_id connection_collation_id) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definition/utf8.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definition() const = 0;
  virtual void set_definition(const String_type &definition) = 0;

  virtual const String_type &definition_utf8() const = 0;
  virtual void set_definition_utf8(const String_type &definition_utf8) = 0;

  /////////////////////////////////////////////////////////////////////////
  // check_option.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_check_option check_option() const = 0;
  virtual void set_check_option(enum_check_option check_option) = 0;

  /////////////////////////////////////////////////////////////////////////
  // is_updatable.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_updatable() const = 0;
  virtual void set_updatable(bool updatable) = 0;

  /////////////////////////////////////////////////////////////////////////
  // algorithm.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_algorithm algorithm() const = 0;
  virtual void set_algorithm(enum_algorithm algorithm) = 0;

  /////////////////////////////////////////////////////////////////////////
  // security_type.
  /////////////////////////////////////////////////////////////////////////

  virtual enum_security_type security_type() const = 0;
  virtual void set_security_type(enum_security_type security_type) = 0;

  /////////////////////////////////////////////////////////////////////////
  // definer.
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &definer_user() const = 0;
  virtual const String_type &definer_host() const = 0;
  virtual void set_definer(const String_type &username,
                           const String_type &hostname) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Explicit list of column names.
  // It is a dictionary of string=>string, where the key is the number of the
  // column ("1", "2", etc) and the value is the column's name.
  /////////////////////////////////////////////////////////////////////////

  virtual const Properties &column_names() const = 0;
  virtual Properties &column_names() = 0;

  /////////////////////////////////////////////////////////////////////////
  // View-table collection.
  /////////////////////////////////////////////////////////////////////////

  virtual View_table *add_table() = 0;

  virtual const View_tables &tables() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // View_routine collection.
  /////////////////////////////////////////////////////////////////////////

  virtual View_routine *add_routine() = 0;

  virtual const View_routines &routines() const = 0;

  /**
    Allocate a new object graph and invoke the copy contructor for
    each object.

    @return pointer to dynamically allocated copy
  */
  virtual View *clone() const = 0;

  /**
    Clear View columns, View_tables and View_routines collections.
  */
  virtual void remove_children() = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__VIEW_INCLUDED
