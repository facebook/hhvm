/*
   Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "client/base/bool_option.h"

using namespace Mysql::Tools::Base::Options;
using std::string;

Bool_option::Bool_option(bool *value, string name, string description)
    : Abstract_option<Bool_option>(value, GET_BOOL, name, description, false) {
  this->m_option_structure.arg_type = NO_ARG;
  *value = false;
}

Bool_option *Bool_option::set_value(bool value) {
  *(bool *)this->m_option_structure.value = value;
  this->m_option_structure.def_value = (longlong)value;
  return this;
}