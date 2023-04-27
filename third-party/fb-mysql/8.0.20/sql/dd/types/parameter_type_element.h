/* Copyright (c) 2016, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__PARAMETER_TYPE_ELEMENT_INCLUDED
#define DD__PARAMETER_TYPE_ELEMENT_INCLUDED

#include "my_inttypes.h"
#include "sql/dd/types/weak_object.h"  // dd::Weak_object

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Parameter;
class Parameter_type_element_impl;

namespace tables {
class Parameter_type_elements;
}

///////////////////////////////////////////////////////////////////////////

class Parameter_type_element : virtual public Weak_object {
 public:
  typedef Parameter_type_element_impl Impl;
  typedef tables::Parameter_type_elements DD_table;

 public:
  virtual ~Parameter_type_element() {}

  /////////////////////////////////////////////////////////////////////////
  // Name
  /////////////////////////////////////////////////////////////////////////

  virtual const String_type &name() const = 0;
  virtual void set_name(const String_type &name) = 0;

  /////////////////////////////////////////////////////////////////////////
  // Parameter
  /////////////////////////////////////////////////////////////////////////

  virtual const Parameter &parameter() const = 0;

  /////////////////////////////////////////////////////////////////////////
  // Index
  /////////////////////////////////////////////////////////////////////////

  virtual uint index() const = 0;
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__PARAMETER_TYPE_ELEMENT_INCLUDED
