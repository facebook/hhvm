/*
   Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TABLE_TRIGGER_FIELD_SUPPORT_H_INCLUDED
#define TABLE_TRIGGER_FIELD_SUPPORT_H_INCLUDED

///////////////////////////////////////////////////////////////////////////

#include "sql/trigger_def.h"  // enum_trigger_variable_type

///////////////////////////////////////////////////////////////////////////

struct TABLE;
class Field;

///////////////////////////////////////////////////////////////////////////

/**
  This is an interface to be used from Item_trigger_field to access information
  about table trigger fields (NEW/OLD rows).
*/

class Table_trigger_field_support {
 public:
  virtual TABLE *get_subject_table() = 0;

  virtual Field *get_trigger_variable_field(enum_trigger_variable_type v,
                                            int field_index) = 0;

  virtual ~Table_trigger_field_support() {}
};

///////////////////////////////////////////////////////////////////////////

#endif  // TABLE_TRIGGER_FIELD_SUPPORT_H_INCLUDED
